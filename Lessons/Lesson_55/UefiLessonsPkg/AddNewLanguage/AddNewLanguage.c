/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/VariablePolicy.h>


EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;

  CHAR8* LanguageString;
  Status = GetEfiGlobalVariable2(L"PlatformLangCodes", (VOID**)&LanguageString, NULL);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't perform GetEfiGlobalVariable2, status=%r\n", Status);
    return Status;
  }
  Print(L"Current value of the 'PlatformLangCodes' variable is '%a'\n", LanguageString);

  CHAR8* NewLanguageString = AllocatePool(AsciiStrLen(LanguageString) + AsciiStrSize(";ru-RU"));
  if (NewLanguageString == NULL) {
    Print(L"Error! Can't allocate size for new PlatformLangCodes variable\n");
    FreePool(LanguageString);
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem(NewLanguageString, LanguageString, AsciiStrLen(LanguageString));
  CopyMem(&NewLanguageString[AsciiStrLen(LanguageString)], ";ru-RU", AsciiStrSize(";ru-RU"));

  Print(L"Set 'PlatformLangCodes' variable to '%a'\n", NewLanguageString);

  Status = gRT->SetVariable (
                L"PlatformLangCodes",
                &gEfiGlobalVariableGuid,
                EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                AsciiStrSize(NewLanguageString),
                NewLanguageString
                );
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't set PlatformLangCodes variable, status=%r\n", Status);
  }

  EDKII_VARIABLE_POLICY_PROTOCOL* VariablePolicyProtocol;
  Status = gBS->LocateProtocol(&gEdkiiVariablePolicyProtocolGuid,
                               NULL,
                               (VOID**)&VariablePolicyProtocol);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Could not find Variable Policy protocol: %r\n", Status);
    return Status;
  }
  Status = VariablePolicyProtocol->DisableVariablePolicy();
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't disable VariablePolicy: %r\n", Status);
    return Status;
  }

  return EFI_SUCCESS;
}
