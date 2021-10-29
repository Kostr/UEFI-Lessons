/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/HiiLib.h>
#include <Library/UefiHiiServicesLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  EFI_HII_PACKAGE_LIST_HEADER *PackageList;
  //
  // Retrieve HII package list from ImageHandle.
  //
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    Print(L"Error! Can't open EFI_HII_PACKAGE_LIST_PROTOCOL\n");
    return Status;
  }

  EFI_HII_HANDLE Handle;
  Status = gHiiDatabase->NewPackageList(gHiiDatabase, PackageList, NULL, &Handle);
  if (EFI_ERROR(Status))
  {
    Print(L"Can't register HII Package list %g, status = %r\n", gHIIStringsUNIRCGuid, Status);
    return Status;
  }

  Print(L"en-US ID=1: %s\n", HiiGetString(Handle, 1, "en-US"));
  Print(L"en-US ID=2: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_HELLO), "en-US"));
  Print(L"en-US ID=3: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_BYE), "en-US"));
  Print(L"fr-FR ID=1: %s\n", HiiGetString(Handle, 1, "fr-FR"));
  Print(L"fr-FR ID=2: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_HELLO), "fr-FR"));
  Print(L"fr-FR ID=3: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_BYE), "fr-FR"));

  Print(L"Best language ID=1: %s\n", HiiGetString(Handle, 1, NULL));
  Print(L"Best language ID=2: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_HELLO), NULL));
  Print(L"Best language ID=3: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_BYE), NULL));

  Print(L"fr ID=3: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_BYE), "fr"));

  return EFI_SUCCESS;
}
