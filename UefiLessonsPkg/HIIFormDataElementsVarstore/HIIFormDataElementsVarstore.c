/*
 * Copyright (c) 2022, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HiiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Protocol/HiiConfigAccess.h>
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
    DATAPATH_GUID
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
EFI_HII_CONFIG_ACCESS_PROTOCOL mConfigAccess;
EFI_GUID StorageGuid = STORAGE_GUID;
EFI_STRING StorageName = L"FormData";

VARIABLE_STRUCTURE FormStorage;


STATIC
EFI_STATUS
EFIAPI
ExtractConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Request,
  OUT       EFI_STRING                      *Progress,
  OUT       EFI_STRING                      *Results
)
{
  BOOLEAN AllocatedRequest = FALSE;

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((Request != NULL) && !HiiIsConfigHdrMatch(Request, &StorageGuid, StorageName)) {
    return EFI_NOT_FOUND;
  }

  EFI_STRING ConfigRequest = Request;
  if ((Request == NULL) || (StrStr (Request, L"OFFSET") == NULL)) {
    EFI_STRING ConfigRequestHdr = HiiConstructConfigHdr(&StorageGuid, StorageName, mDriverHandle);
    UINTN Size = (StrLen(ConfigRequestHdr) + StrLen(L"&OFFSET=0&WIDTH=") + sizeof(UINTN)*2 + 1) * sizeof(CHAR16);
    ConfigRequest = AllocateZeroPool(Size);
    AllocatedRequest = TRUE;
    UnicodeSPrint(ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, sizeof(VARIABLE_STRUCTURE));
    FreePool(ConfigRequestHdr);
  }
  EFI_STATUS Status = gHiiConfigRouting->BlockToConfig(gHiiConfigRouting,
                                                       ConfigRequest,
                                                       (UINT8*)&FormStorage,
                                                       sizeof(VARIABLE_STRUCTURE),
                                                       Results,
                                                       Progress);

  if (AllocatedRequest) {
    FreePool(ConfigRequest);
    if (Request == NULL) {
      *Progress = NULL;
    } else if (StrStr(Request, L"OFFSET") == NULL) {
      *Progress = Request + StrLen(Request);
    }
  }

  return Status;
}

STATIC
EFI_STATUS
EFIAPI
RouteConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Configuration,
  OUT       EFI_STRING                      *Progress
)
{
  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  UINTN BlockSize = sizeof(VARIABLE_STRUCTURE);
  EFI_STATUS Status = gHiiConfigRouting->ConfigToBlock(gHiiConfigRouting,
                                                       Configuration,
                                                       (UINT8*)&FormStorage,
                                                       &BlockSize,
                                                       Progress);

  return Status;
}

STATIC
EFI_STATUS
EFIAPI
Callback (
  IN     CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN     EFI_BROWSER_ACTION                     Action,
  IN     EFI_QUESTION_ID                        QuestionId,
  IN     UINT8                                  Type,
  IN OUT EFI_IFR_TYPE_VALUE                     *Value,
  OUT    EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI
HIIFormDataElementsVarstoreUnload (
  EFI_HANDLE ImageHandle
  )
{
  if (mHiiHandle != NULL)
    HiiRemovePackages(mHiiHandle);

  EFI_STATUS Status = gBS->UninstallMultipleProtocolInterfaces(
                             mDriverHandle,
                             &gEfiDevicePathProtocolGuid,
                             &mHiiVendorDevicePath,
                             &gEfiHiiConfigAccessProtocolGuid,
                             &mConfigAccess,
                             NULL);

  return Status;
}

EFI_STATUS
EFIAPI
HIIFormDataElementsVarstoreEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  mConfigAccess.ExtractConfig = &ExtractConfig;
  mConfigAccess.RouteConfig   = &RouteConfig;
  mConfigAccess.Callback      = &Callback;

  EFI_STATUS Status;
  Status = gBS->InstallMultipleProtocolInterfaces(
                  &mDriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mConfigAccess,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mHiiHandle = HiiAddPackages(
                 &gEfiCallerIdGuid,
                 mDriverHandle,
                 HIIFormDataElementsVarstoreStrings,
                 FormBin,
                 NULL
                 );
  if (mHiiHandle == NULL) {
    gBS->UninstallMultipleProtocolInterfaces(
           mDriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mHiiVendorDevicePath,
           &gEfiHiiConfigAccessProtocolGuid,
           &mConfigAccess,
           NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  EFI_STRING ConfigStr = HiiConstructConfigHdr(&StorageGuid, StorageName, mDriverHandle);
  UINT16 DefaultId = 1;
  if (!HiiSetToDefaults(ConfigStr, DefaultId)) {
    Print(L"Error! Can't set default configuration #%d\n", DefaultId);
  }

  return EFI_SUCCESS;
}
