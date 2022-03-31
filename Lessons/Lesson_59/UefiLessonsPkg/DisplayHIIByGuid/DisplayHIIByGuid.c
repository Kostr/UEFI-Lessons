/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/MemoryAllocationLib.h>
#include <Library/HiiLib.h>
#include <Protocol/FormBrowser2.h>

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  if ((Argc < 2) || (Argc > 3)) {
    Print(L"Usage:\n");
    Print(L"  DisplayHIIByGuid <Package list GUID> [<Formset classguid>]\n");
    return EFI_INVALID_PARAMETER;
  }

  EFI_GUID PackageListGuid;
  EFI_STATUS Status = StrToGuid(Argv[1], &PackageListGuid);
  if (Status != RETURN_SUCCESS) {
    Print(L"Error! Can't convert <Package list GUID> argument to GUID\n");
    return EFI_INVALID_PARAMETER;
  }

  EFI_GUID FormsetClassGuid = EFI_HII_PLATFORM_SETUP_FORMSET_GUID;
  if (Argc == 3) {
    Status = StrToGuid(Argv[2], &FormsetClassGuid);
    if (Status != RETURN_SUCCESS) {
      Print(L"Error! Can't convert <Formset classguid> argument to GUID\n");
      return EFI_INVALID_PARAMETER;
    }
  }

  EFI_HII_HANDLE* HiiHandles = HiiGetHiiHandles(&PackageListGuid);

  EFI_HII_HANDLE* HiiHandle = HiiHandles;
  UINTN HandleCount=0;
  while (*HiiHandle != NULL) {
    HiiHandle++;
    HandleCount++;
  }

  EFI_FORM_BROWSER2_PROTOCOL* FormBrowser2;
  Status = gBS->LocateProtocol(&gEfiFormBrowser2ProtocolGuid, NULL, (VOID**)&FormBrowser2);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't locate gEfiFormBrowser2Protocol\n");
    FreePool(HiiHandles);
    return Status;
  }

  Status = FormBrowser2->SendForm (
                           FormBrowser2,
                           HiiHandles,
                           HandleCount,
                           &FormsetClassGuid,
                           0,
                           NULL,
                           NULL
                           );  

  if (EFI_ERROR(Status)) {
    Print(L"Error! SendForm returned %r\n", Status);
  }

  FreePool(HiiHandles);

  return EFI_SUCCESS;
}
