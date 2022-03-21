#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HiiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include "Data.h"

extern UINT8 FormBin[];

#pragma pack(1)
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
EFI_STRING      UEFIVariableName = UEFI_VARIABLE_STRUCTURE_NAME;


EFI_STATUS
EFIAPI
HIIFormDataElementsUnload (
  EFI_HANDLE ImageHandle
  )
{
  if (mHiiHandle != NULL)
    HiiRemovePackages(mHiiHandle);

  EFI_STATUS Status;
  UINTN BufferSize;
  UEFI_VARIABLE_STRUCTURE EfiVarstore;

  BufferSize = sizeof(UEFI_VARIABLE_STRUCTURE);
  Status = gRT->GetVariable(
                UEFIVariableName,
                &mHiiVendorDevicePath.VendorDevicePath.Guid,
                NULL,
                &BufferSize,
                &EfiVarstore);
  if (!EFI_ERROR(Status)) {
    Status = gRT->SetVariable(
                  UEFIVariableName,
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
HIIFormDataElementsEntryPoint (
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
  UEFI_VARIABLE_STRUCTURE EfiVarstore;
  BufferSize = sizeof(UEFI_VARIABLE_STRUCTURE);
  Status = gRT->GetVariable (
                UEFIVariableName,
                &mHiiVendorDevicePath.VendorDevicePath.Guid,
                NULL,
                &BufferSize,
                &EfiVarstore);
  if (EFI_ERROR(Status)) {
    ZeroMem(&EfiVarstore, sizeof(EfiVarstore));
    Status = gRT->SetVariable(
                  UEFIVariableName,
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
                 HIIFormDataElementsStrings,
                 FormBin,
                 NULL
                 );
  if (mHiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}
