/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/HiiLib.h>
#include <Protocol/FormBrowser2.h>

#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>

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

#define FORMSET_GUID  {0xef2acc91, 0x7b50, 0x4ab9, {0xab, 0x67, 0x2b, 0x4, 0xf8, 0xbc, 0x13, 0x5e}}

EFI_HII_HANDLE  mHiiHandle = NULL;
EFI_HANDLE      mDriverHandle = NULL;

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


EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  //
  // Publish sample Fromset
  //
  Status = gBS->InstallProtocolInterface (
                  &mDriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mHiiVendorDevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }


  UINTN BufferSize;
  UINT8 EfiVarstore;
  BufferSize = sizeof(UINT8);
  Status = gRT->GetVariable (
                L"HIIFormCheckboxEfiVarstore",
                &mHiiVendorDevicePath.VendorDevicePath.Guid,
                NULL,
                &BufferSize,
                &EfiVarstore);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't find variable! %r\n", Status);
    ZeroMem(&EfiVarstore, sizeof(EfiVarstore));
    Status = gRT->SetVariable(
                  L"HIIFormCheckboxEfiVarstore",
                  &mHiiVendorDevicePath.VendorDevicePath.Guid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof(EfiVarstore),
                  &EfiVarstore);
    if (EFI_ERROR(Status)) {
      Print(L"Error! Can't create variable! %r\n", Status);
    }
  }
 

  EFI_HII_HANDLE Handle = HiiAddPackages(
                             &gEfiCallerIdGuid,
                             mDriverHandle,
                             HIIFormCheckboxStrings,
                             FormBin,
                             NULL
                             );
  if (Handle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
 
  EFI_FORM_BROWSER2_PROTOCOL* FormBrowser2;
  Status = gBS->LocateProtocol(&gEfiFormBrowser2ProtocolGuid, NULL, (VOID**)&FormBrowser2);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = FormBrowser2->SendForm (
                           FormBrowser2,
                           &Handle,
                           1,
                           NULL,
                           0,
                           NULL,
                           NULL
                           );

  HiiRemovePackages(Handle);

  return EFI_SUCCESS;
}
