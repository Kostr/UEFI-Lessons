/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/HiiLib.h>
#include <Guid/MdeModuleHii.h>
#include "Data.h"

extern UINT8 FormBin[];

EFI_HII_HANDLE  mHiiHandle = NULL;


EFI_STATUS
EFIAPI
HIIFormLabelUnload (
  EFI_HANDLE ImageHandle
  )
{
  if (mHiiHandle != NULL)
    HiiRemovePackages(mHiiHandle);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HIIFormLabelEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  mHiiHandle = HiiAddPackages(
                 &gEfiCallerIdGuid,
                 NULL,
                 HIIFormLabelStrings,
                 FormBin,
                 NULL
                 );
  if (mHiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  VOID* StartOpCodeHandle = HiiAllocateOpCodeHandle();
  EFI_IFR_GUID_LABEL* StartLabel = (EFI_IFR_GUID_LABEL*) HiiCreateGuidOpCode(StartOpCodeHandle,
                                                                             &gEfiIfrTianoGuid,
                                                                             NULL,
                                                                             sizeof(EFI_IFR_GUID_LABEL)
                                                                             );
  if (StartLabel == NULL) {
    Print(L"Error! Can't create StartLabel opcode, not enough space\n");
    HiiRemovePackages(mHiiHandle);
    return EFI_BUFFER_TOO_SMALL;
  }
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number = LABEL_START;


  VOID* EndOpCodeHandle = HiiAllocateOpCodeHandle();
  EFI_IFR_GUID_LABEL* EndLabel = (EFI_IFR_GUID_LABEL*) HiiCreateGuidOpCode(EndOpCodeHandle,
                                                                           &gEfiIfrTianoGuid,
                                                                           NULL,
                                                                           sizeof(EFI_IFR_GUID_LABEL)
                                                                           );
  if (EndLabel == NULL) {
    Print(L"Error! Can't create EndLabel opcode, not enough space\n");
    HiiFreeOpCodeHandle(StartOpCodeHandle);
    HiiFreeOpCodeHandle(EndOpCodeHandle);
    HiiRemovePackages(mHiiHandle);
    return EFI_BUFFER_TOO_SMALL;
  }

  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number = LABEL_END;

  EFI_STRING_ID text_prompt = HiiSetString(mHiiHandle,
                                           0,
                                           L"Text prompt",
                                           NULL);
  EFI_STRING_ID text_help = HiiSetString(mHiiHandle,
                                         0,
                                         L"Text help",
                                         NULL);


  UINT8* Result = HiiCreateTextOpCode(StartOpCodeHandle,
                                      text_prompt,
                                      text_help,
                                      0);
  if (Result == NULL) {
    Print(L"Error! Can't create Text opcode, not enough space\n");
    HiiFreeOpCodeHandle(StartOpCodeHandle);
    HiiFreeOpCodeHandle(EndOpCodeHandle);
    HiiRemovePackages(mHiiHandle);
    return EFI_BUFFER_TOO_SMALL;
  }

  text_prompt = HiiSetString(mHiiHandle,
                             0,
                             L"Another text prompt",
                             NULL);
  text_help = HiiSetString(mHiiHandle,
                           0,
                           L"Another text help",
                           NULL);

  Result = HiiCreateTextOpCode(StartOpCodeHandle,
                               text_prompt,
                               text_help,
                               0);
  if (Result == NULL) {
    Print(L"Error! Can't create Text opcode, not enough space\n");
    HiiFreeOpCodeHandle(StartOpCodeHandle);
    HiiFreeOpCodeHandle(EndOpCodeHandle);
    HiiRemovePackages(mHiiHandle);
    return EFI_BUFFER_TOO_SMALL;
  }


 
  EFI_GUID formsetGuid = FORMSET_GUID;
  EFI_STATUS Status = HiiUpdateForm(
                        mHiiHandle,
                        &formsetGuid,
                        0x1,
                        StartOpCodeHandle,
                        EndOpCodeHandle
                      );
  if (EFI_ERROR(Status)) {
    Print(L"Error! HiiUpdateForm returned = %r\n", Status);
  }

  HiiFreeOpCodeHandle(StartOpCodeHandle);
  HiiFreeOpCodeHandle(EndOpCodeHandle);
  return Status;
}
