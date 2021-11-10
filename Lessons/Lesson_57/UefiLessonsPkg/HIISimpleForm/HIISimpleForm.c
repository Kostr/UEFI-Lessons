/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/HiiLib.h>
#include <Protocol/FormBrowser2.h>

extern UINT8 FormBin[];

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_HII_HANDLE Handle = HiiAddPackages(
                             &gEfiCallerIdGuid,
                             NULL,
                             HIISimpleFormStrings,
                             FormBin,
                             NULL
                             );
  if (Handle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  EFI_STATUS Status;
  EFI_FORM_BROWSER2_PROTOCOL* FormBrowser2;
  Status = gBS->LocateProtocol(&gEfiFormBrowser2ProtocolGuid, NULL, (VOID**)&FormBrowser2);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = FormBrowser2->SendForm (
                           FormBrowser2,
                           &Handle,
                           1,
                           NULL,
                           0,
                           NULL,
                           NULL
                           );

  HiiRemovePackages(Handle);

  return EFI_SUCCESS;
}
