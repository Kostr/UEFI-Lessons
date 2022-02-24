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
  if (Argc != 2) {
    Print(L"Usage:\n");
    Print(L"  DisplayHIIByGuid <GUID>\n");
    return EFI_INVALID_PARAMETER;
  }

  GUID Guid;
  EFI_STATUS Status = StrToGuid(Argv[1], &Guid);
  if (Status != RETURN_SUCCESS) {
    Print(L"Error! Can't convert input argument to GUID\n");
    return EFI_INVALID_PARAMETER;
  }


  EFI_HII_HANDLE* HiiHandles = HiiGetHiiHandles(&Guid);

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
                           NULL,
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
