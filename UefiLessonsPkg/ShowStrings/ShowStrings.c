/*
 * Copyright (c) 2024, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>


INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  if (Argc != 2) {
    Print(L"Usage:\n");
    Print(L"  ShowStrings [Package GUID]\n");
    return EFI_INVALID_PARAMETER;
  }

  EFI_GUID PackageGuid;
  EFI_STATUS Status = StrToGuid(Argv[1], &PackageGuid);
  if (Status != RETURN_SUCCESS) {
    Print(L"Error! Can't convert <Package GUID> argument to GUID\n");
    return EFI_INVALID_PARAMETER;
  }

  EFI_HII_HANDLE* Handle = HiiGetHiiHandles(&PackageGuid);

  for (UINTN i=1; i<0xFFFF; i++) {
    EFI_STRING String = HiiGetString(*Handle, i, "en-US");
    if (String != NULL) {
      Print(L"ID=%d, %s\n", i, String);
      FreePool(String);
    }
  }
  return EFI_SUCCESS;
}
