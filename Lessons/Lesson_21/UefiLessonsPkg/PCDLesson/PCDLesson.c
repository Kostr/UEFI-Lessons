/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/PcdLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print(L"PcdMyVar32=%d\n", FixedPcdGet32(PcdMyVar32));
  Print(L"PcdMyVar32_1=%d\n", FixedPcdGet32(PcdMyVar32_1));
  Print(L"PcdMyVar32_2=%d\n", FixedPcdGet32(PcdMyVar32_2));
  return EFI_SUCCESS;
}
