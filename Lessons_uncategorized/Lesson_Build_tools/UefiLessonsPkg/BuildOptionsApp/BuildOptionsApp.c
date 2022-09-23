/*
 * Copyright (c) 2022, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  #ifdef MY_DEBUG
    Print(L"MY_DEBUG is defined\n");
  #endif
  #ifdef MY_RELEASE
    Print(L"MY_RELEASE is defined\n");
  #endif
  #ifdef MY_ALL_TARGETS
    Print(L"MY_ALL_TARGETS is defined\n");
  #endif

  return EFI_SUCCESS;
}
