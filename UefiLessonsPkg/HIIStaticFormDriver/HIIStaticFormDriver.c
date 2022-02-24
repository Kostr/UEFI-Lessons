/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/HiiLib.h>


extern UINT8 FormBin[];

EFI_HII_HANDLE Handle;


EFI_STATUS
EFIAPI
HIIStaticFormDriverUnload (
  EFI_HANDLE ImageHandle
  )
{
  if (Handle != NULL)
    HiiRemovePackages(Handle);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HIIStaticFormDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Handle = HiiAddPackages(
             &gEfiCallerIdGuid,
             NULL,
             HIIStaticFormDriverStrings,
             FormBin,
             NULL
           );
  if (Handle == NULL) 
    return EFI_OUT_OF_RESOURCES;

  return EFI_SUCCESS;
}
