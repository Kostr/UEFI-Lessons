/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/HiiLib.h>

#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>

#define FORMSET_GUID  {0xef2acc91, 0x7b50, 0x4ab9, {0xab, 0x67, 0x2b, 0x4, 0xf8, 0xbc, 0x13, 0x5e}}

extern UINT8 FormBin[];

#pragma pack(1)
///
/// HII specific Vendor Device Path definition.
///
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} HII_VENDOR_DEVICE_PATH;
#pragma pack()

HII_VENDOR_DEVICE_PATH  mHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    FORMSET_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};


EFI_HII_HANDLE  mHiiHandle = NULL;
EFI_HANDLE      mDriverHandle = NULL;


EFI_STATUS
EFIAPI
HIIFormCheckboxUnload (
  EFI_HANDLE ImageHandle
  )
{
  if (mHiiHandle != NULL)
    HiiRemovePackages(mHiiHandle);

  EFI_STATUS Status;
  UINTN BufferSize;
  UINT8 EfiVarstore;

  BufferSize = sizeof(UINT8);
  Status = gRT->GetVariable(
                L"CheckboxValue",
                &mHiiVendorDevicePath.VendorDevicePath.Guid,
                NULL,
                &BufferSize,
                &EfiVarstore);
  if (!EFI_ERROR(Status)) {
    Status = gRT->SetVariable(
                  L"CheckboxValue",
                  &mHiiVendorDevicePath.VendorDevicePath.Guid,
                  0,
                  0,
                  NULL);
    if (EFI_ERROR(Status)) {
      Print(L"Error! Can't delete variable! %r\n", Status);
    }
  }

  Status = gBS->UninstallMultipleProtocolInterfaces(
                mDriverHandle,
                &gEfiDevicePathProtocolGuid,
                &mHiiVendorDevicePath,
                NULL
                );

  return Status;
}

EFI_STATUS
EFIAPI
HIIFormCheckboxEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  Status = gBS->InstallMultipleProtocolInterfaces(
                  &mDriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }


  UINTN BufferSize;
  UINT8 EfiVarstore;
  BufferSize = sizeof(UINT8);
  Status = gRT->GetVariable (
                L"CheckboxValue",
                &mHiiVendorDevicePath.VendorDevicePath.Guid,
                NULL,
                &BufferSize,
                &EfiVarstore);
  if (EFI_ERROR(Status)) {
    ZeroMem(&EfiVarstore, sizeof(EfiVarstore));
    Status = gRT->SetVariable(
                  L"CheckboxValue",
                  &mHiiVendorDevicePath.VendorDevicePath.Guid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof(EfiVarstore),
                  &EfiVarstore);
    if (EFI_ERROR(Status)) {
      Print(L"Error! Can't create variable! %r\n", Status);
    }
  }
 
  mHiiHandle = HiiAddPackages(
                 &gEfiCallerIdGuid,
                 mDriverHandle,
                 HIIFormCheckboxStrings,
                 FormBin,
                 NULL
                 );
  if (mHiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
 
  return EFI_SUCCESS;
}
