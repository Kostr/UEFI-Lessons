/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiLib.h>
#include <Library/SimpleLibrary.h>

UINTN Plus2(UINTN number) {
  return number+2;
}

EFI_STATUS
EFIAPI
SimpleLibraryConstructor(
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  Print(L"Hello from library constructor!\n");
  return EFI_SUCCESS;
}

