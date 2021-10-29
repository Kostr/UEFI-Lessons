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

  Print(L"PcdMyVar32=%d\n", PcdGet32(PcdMyVar32));

  Print(L"PcdMyPatchableVar32=%d\n", PcdGet32(PcdMyPatchableVar32));
  EFI_STATUS Status = PcdSet32S(PcdMyPatchableVar32, 44);
  Print(L"Status=%r\n", Status);
  Print(L"PcdMyPatchableVar32=%d\n", PcdGet32(PcdMyPatchableVar32));
  PatchPcdSet32(PcdMyPatchableVar32, 45);
  Print(L"PcdMyPatchableVar32=%d\n", PatchPcdGet32(PcdMyPatchableVar32));


  Print(L"PcdMyFeatureFlagVar=%d\n", FeaturePcdGet(PcdMyFeatureFlagVar));
  Print(L"PcdMyFeatureFlagVar=%d\n", PcdGetBool(PcdMyFeatureFlagVar));
  Print(L"PcdMyVarBool=%d\n", FixedPcdGetBool(PcdMyVarBool));
  Print(L"PcdMyVarBool=%d\n", PcdGetBool(PcdMyVarBool));
  



  Print(L"PcdMyDynamicExVar32=%x\n", PcdGet32(PcdMyDynamicExVar32));
  PcdSet32S(PcdMyDynamicExVar32, 52);
  Print(L"PcdMyDynamicExVar32=%x\n", PcdGet32(PcdMyDynamicExVar32));

  Print(L"PcdMyDynamicVar32=%x\n", PcdGet32(PcdMyDynamicVar32));
  PcdSet32S(PcdMyDynamicVar32, 52);
  Print(L"PcdMyDynamicVar32=%x\n", PcdGet32(PcdMyDynamicVar32));
  return EFI_SUCCESS;
}
