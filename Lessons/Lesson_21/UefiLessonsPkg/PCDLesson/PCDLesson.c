/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print(L"PcdInt8=0x%x\n", FixedPcdGet8(PcdInt8));
  Print(L"PcdInt16=0x%x\n", FixedPcdGet16(PcdInt16));
  Print(L"PcdInt32=0x%x\n", FixedPcdGet32(PcdInt32));
  Print(L"PcdInt64=0x%x\n", FixedPcdGet64(PcdInt64));
  Print(L"PcdBool=0x%x\n", FixedPcdGetBool(PcdBool));

  Print(L"PcdInt8=0x%x\n", PcdGet8(PcdInt8));
  Print(L"PcdInt16=0x%x\n", PcdGet16(PcdInt16));
  Print(L"PcdInt32=0x%x\n", PcdGet32(PcdInt32));
  Print(L"PcdInt64=0x%x\n", PcdGet64(PcdInt64));
  Print(L"PcdIntBool=0x%x\n", PcdGetBool(PcdBool));

  Print(L"PcdAsciiStr=%a\n", FixedPcdGetPtr(PcdAsciiStr));
  Print(L"PcdAsciiStrSize=%d\n", FixedPcdGetSize(PcdAsciiStr));
  Print(L"PcdUCS2Str=%s\n", PcdGetPtr(PcdUCS2Str));
  Print(L"PcdUCS2StrSize=%d\n", PcdGetSize(PcdUCS2Str));

  for (UINTN i=0; i<FixedPcdGetSize(PcdArray); i++) {
    Print(L"PcdArray[%d]=0x%02x\n", i, ((UINT8*)FixedPcdGetPtr(PcdArray))[i]);
  }

  Print(L"PcdGuidInBytes=%g\n", *(EFI_GUID*)FixedPcdGetPtr(PcdGuidInBytes));
  Print(L"PcdGuid=%g\n", *(EFI_GUID*)FixedPcdGetPtr(PcdGuid));

  Print(L"PcdDevicePath: %s\n", ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*) FixedPcdGetPtr(PcdDevicePath), FALSE, FALSE));

//-------------
  Print(L"PcdInt32Override=%d\n", FixedPcdGet32(PcdInt32Override));
//-------------

  return EFI_SUCCESS;
}
