/*
 * Copyright (c) 2024, Konstantin Aladyshev <aladyshev22@gmail.com>
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
#include <Library/DebugLib.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiPopup.h>
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
  EFI_INPUT_KEY Key;  
  do {
    CreatePopUp(EFI_TEXT_ATTR(EFI_RED, EFI_BLACK), &Key, L"Extract: ", Request, NULL);
  } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);

  DEBUG ((EFI_D_INFO, "ExtractConfig: Request=%s\n", Request));

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
  EFI_INPUT_KEY Key;  
  do {
    CreatePopUp(EFI_TEXT_ATTR(EFI_RED, EFI_BLACK), &Key, L"Route: ", Configuration, NULL);
  } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);

  DEBUG ((EFI_D_INFO, "RouteConfig: Configuration=%s\n", Configuration));

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

EFI_STRING ActionToStr(EFI_BROWSER_ACTION Action)
{
  switch (Action) {
    case EFI_BROWSER_ACTION_CHANGING:
      return L"EFI_BROWSER_ACTION_CHANGING";
    case EFI_BROWSER_ACTION_CHANGED:
      return L"EFI_BROWSER_ACTION_CHANGED";
    case EFI_BROWSER_ACTION_RETRIEVE:
      return L"EFI_BROWSER_ACTION_RETRIEVE";
    case EFI_BROWSER_ACTION_FORM_OPEN:
      return L"EFI_BROWSER_ACTION_FORM_OPEN";
    case EFI_BROWSER_ACTION_FORM_CLOSE:
      return L"EFI_BROWSER_ACTION_FORM_CLOSE";
    case EFI_BROWSER_ACTION_SUBMITTED:
      return L"EFI_BROWSER_ACTION_SUBMITTED";
    case EFI_BROWSER_ACTION_DEFAULT_STANDARD:
      return L"EFI_BROWSER_ACTION_DEFAULT_STANDARD";
    case EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING:
      return L"EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING";
    case EFI_BROWSER_ACTION_DEFAULT_SAFE:
      return L"EFI_BROWSER_ACTION_DEFAULT_SAFE";
    case EFI_BROWSER_ACTION_DEFAULT_PLATFORM:
      return L"EFI_BROWSER_ACTION_DEFAULT_PLATFORM";
    case EFI_BROWSER_ACTION_DEFAULT_HARDWARE:
      return L"EFI_BROWSER_ACTION_DEFAULT_HARDWARE";
    case EFI_BROWSER_ACTION_DEFAULT_FIRMWARE:
      return L"EFI_BROWSER_ACTION_DEFAULT_FIRMWARE";
    default:
      return L"Unknown";
  }
}

EFI_STRING TypeToStr(UINT8 Type)
{
  switch (Type) {
    case EFI_IFR_TYPE_NUM_SIZE_8:
      return L"EFI_IFR_TYPE_NUM_SIZE_8";
    case EFI_IFR_TYPE_NUM_SIZE_16:
      return L"EFI_IFR_TYPE_NUM_SIZE_16";
    case EFI_IFR_TYPE_NUM_SIZE_32:
      return L"EFI_IFR_TYPE_NUM_SIZE_32";
    case EFI_IFR_TYPE_NUM_SIZE_64:
      return L"EFI_IFR_TYPE_NUM_SIZE_64";
    case EFI_IFR_TYPE_BOOLEAN:
      return L"EFI_IFR_TYPE_BOOLEAN";
    case EFI_IFR_TYPE_TIME:
      return L"EFI_IFR_TYPE_TIME";
    case EFI_IFR_TYPE_DATE:
      return L"EFI_IFR_TYPE_DATE";
    case EFI_IFR_TYPE_STRING:
      return L"EFI_IFR_TYPE_STRING";
    case EFI_IFR_TYPE_OTHER:
      return L"EFI_IFR_TYPE_OTHER";
    case EFI_IFR_TYPE_UNDEFINED:
      return L"EFI_IFR_TYPE_UNDEFINED";
    case EFI_IFR_TYPE_ACTION:
      return L"EFI_IFR_TYPE_ACTION";
    case EFI_IFR_TYPE_BUFFER:
      return L"EFI_IFR_TYPE_BUFFER";
    case EFI_IFR_TYPE_REF:
      return L"EFI_IFR_TYPE_REF";
    default:
      return L"Unknown";
  }
}

VOID CallbackValueToStr(UINT8 Type, EFI_IFR_TYPE_VALUE *Value, EFI_STRING* ValueStr, UINTN ValueStrSize)
{
  switch (Type) {
    case EFI_IFR_TYPE_NUM_SIZE_8:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%d", Value->u8);
      break;
    case EFI_IFR_TYPE_NUM_SIZE_16:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%d", Value->u16);
      break;
    case EFI_IFR_TYPE_NUM_SIZE_32:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%d", Value->u32);
      break;
    case EFI_IFR_TYPE_NUM_SIZE_64:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%ld", Value->u64);
      break;
    case EFI_IFR_TYPE_BOOLEAN:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%d", Value->b);
      break;
    case EFI_IFR_TYPE_TIME:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%02d:%02d:%02d", Value->time.Hour, Value->time.Minute, Value->time.Second);
      break;
    case EFI_IFR_TYPE_DATE:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%04d/%02d/%02d", Value->date.Year, Value->date.Month, Value->date.Day);
      break;
    case EFI_IFR_TYPE_STRING:
      if (Value->string)
        UnicodeSPrint(*ValueStr, ValueStrSize, L"%s", HiiGetString(mHiiHandle, Value->string, "en-US"));
      else
        UnicodeSPrint(*ValueStr, ValueStrSize, L"NO STRING!");
      break;
    default:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"Unknown");
      break;
  }
}

VOID DebugCallbackValue(UINT8 Type, EFI_IFR_TYPE_VALUE *Value)
{
  switch (Type) {
    case EFI_IFR_TYPE_NUM_SIZE_8:
      DEBUG ((EFI_D_INFO, "%d\n", Value->u8));
      break;
    case EFI_IFR_TYPE_NUM_SIZE_16:
      DEBUG ((EFI_D_INFO, "%d\n", Value->u16));
      break;
    case EFI_IFR_TYPE_NUM_SIZE_32:
      DEBUG ((EFI_D_INFO, "%d\n", Value->u32));
      break;
    case EFI_IFR_TYPE_NUM_SIZE_64:
      DEBUG ((EFI_D_INFO, "%ld\n", Value->u64));
      break;
    case EFI_IFR_TYPE_BOOLEAN:
      DEBUG ((EFI_D_INFO, "%d\n", Value->b));
      break;
    case EFI_IFR_TYPE_TIME:
      DEBUG ((EFI_D_INFO, "%02d:%02d:%02d\n", Value->time.Hour, Value->time.Minute, Value->time.Second));
      break;
    case EFI_IFR_TYPE_DATE:
      DEBUG ((EFI_D_INFO, "%04d/%02d/%02d\n", Value->date.Year, Value->date.Month, Value->date.Day));
      break;
    case EFI_IFR_TYPE_STRING:
      if (Value->string)
        DEBUG ((EFI_D_INFO, "%s\n", HiiGetString(mHiiHandle, Value->string, "en-US") ));
      else
        DEBUG ((EFI_D_INFO, "NO STRING!\n" ));
      break;
    default:
      DEBUG ((EFI_D_INFO, "Unknown\n" ));
      break;
  }
}

VOID HIIPopupCallbackInfo(EFI_BROWSER_ACTION Action, EFI_QUESTION_ID QuestionId, UINT8 Type, EFI_IFR_TYPE_VALUE* Value)
{
  EFI_STATUS Status;
  EFI_HII_POPUP_PROTOCOL* HiiPopup;
  EFI_HII_POPUP_SELECTION UserSelection;
  Status = gBS->LocateProtocol(&gEfiHiiPopupProtocolGuid,
                               NULL,
                               (VOID **)&HiiPopup);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "Error! Can't find EFI_HII_POPUP_PROTOCOL\n"));
    return;
  }

  UINTN Size = 300;
  EFI_STRING ValueStr = AllocateZeroPool(Size);
  CallbackValueToStr(Type, Value, &ValueStr, Size);
  EFI_STRING PopupStr = AllocateZeroPool(Size);
  UnicodeSPrint(PopupStr, Size, L"Callback:\nAction=%s\nQuestionId=0x%04x\nType=%s\nValue=%s", ActionToStr(Action), QuestionId, TypeToStr(Type), ValueStr);
  FreePool(ValueStr);
  HiiSetString(mHiiHandle, STRING_TOKEN(POPUP_MESSAGE), PopupStr, NULL);
  Status = HiiPopup->CreatePopup(HiiPopup,
                                 EfiHiiPopupStyleInfo,
                                 EfiHiiPopupTypeOk,
                                 mHiiHandle,
                                 STRING_TOKEN(POPUP_MESSAGE),
                                 &UserSelection
                                );
  FreePool(PopupStr);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "Error! Can't create popup, %r\n", Status));
    return;
  } 
}

VOID PopupCallbackInfo(EFI_BROWSER_ACTION Action, EFI_QUESTION_ID QuestionId, UINT8 Type, EFI_IFR_TYPE_VALUE* Value)
{
  EFI_INPUT_KEY Key;  

  UINTN Size = 100;
  EFI_STRING ActionStr = AllocateZeroPool(Size);
  EFI_STRING QuestionIdStr = AllocateZeroPool(Size);
  EFI_STRING TypeStr = AllocateZeroPool(Size);
  EFI_STRING ValueStr = AllocateZeroPool(Size);
  EFI_STRING ValStr = AllocateZeroPool(Size);
  CallbackValueToStr(Type, Value, &ValStr, Size);
  UnicodeSPrint(ActionStr, Size, L"Action=%s", ActionToStr(Action));
  UnicodeSPrint(QuestionIdStr, Size, L"QuestionId=0x%04x", QuestionId);
  UnicodeSPrint(TypeStr, Size, L"Type=%s", TypeToStr(Type));
  UnicodeSPrint(ValueStr, Size, L"Value=%s", ValStr);
  do {
    //CreatePopUp(EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLUE), &Key, L"Callback:", ActionStr, QuestionIdStr, TypeStr, ValueStr, NULL);
    CreatePopUp(EFI_TEXT_ATTR(EFI_RED, EFI_BLACK), &Key, L"Callback:", ActionStr, QuestionIdStr, TypeStr, ValueStr, NULL);
  } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);

  FreePool(ActionStr);
  FreePool(QuestionIdStr);
  FreePool(TypeStr);
  FreePool(ValueStr);
  FreePool(ValStr);
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
  //DEBUG ((EFI_D_INFO, "Callback: Action=%s, QuestionId=0x%04x, Type=%s, Value=", ActionToStr(Action), QuestionId, TypeToStr(Type)));
  //DebugCallbackValue(Type, Value);

  HIIPopupCallbackInfo(Action, QuestionId, Type, Value);
  //PopupCallbackInfo(Action, QuestionId, Type, Value);

  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI
HIIFormCallbackDebugUnload (
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
HIIFormCallbackDebugEntryPoint (
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
                 HIIFormCallbackDebugStrings,
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
