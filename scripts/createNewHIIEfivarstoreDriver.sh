#!/bin/bash
##
# Copyright (c) 2024, Konstantin Aladyshev <aladyshev22@gmail.com>
#
# SPDX-License-Identifier: MIT
##

# This is a simple script that creates a basic template for your new UEFI driver
# with a HII Form and a EFI Variable Storage.
# Put this script in your edk2 folder and run it with 1 argument - your new driver name

DRIVER_NAME=${1}

UUID=$(uuidgen)

mkdir -p UefiLessonsPkg/${DRIVER_NAME}

cat << EOF > UefiLessonsPkg/${DRIVER_NAME}/${DRIVER_NAME}.inf
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = ${DRIVER_NAME}
  FILE_GUID                      = ${UUID}
  MODULE_TYPE                    = UEFI_DRIVER
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = ${DRIVER_NAME}EntryPoint
  UNLOAD_IMAGE                   = ${DRIVER_NAME}Unload

[Sources]
  ${DRIVER_NAME}.c
  Strings.uni
  Form.vfr

[Packages]
  MdePkg/MdePkg.dec
  MdeModulePkg/MdeModulePkg.dec

[LibraryClasses]
  UefiDriverEntryPoint
  UefiLib
  HiiLib
EOF

cat << EOF > UefiLessonsPkg/${DRIVER_NAME}/${DRIVER_NAME}.c
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HiiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
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
EFI_STRING      UEFIVariableName = UEFI_VARIABLE_STRUCTURE_NAME;
EFI_GUID        UEFIVariableGuid = STORAGE_GUID;

EFI_STATUS
EFIAPI
${DRIVER_NAME}Unload (
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
                &UEFIVariableGuid,
                NULL,
                &BufferSize,
                &EfiVarstore);
  if (!EFI_ERROR(Status)) {
    Status = gRT->SetVariable(
                  UEFIVariableName,
                  &UEFIVariableGuid,
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
${DRIVER_NAME}EntryPoint (
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

  mHiiHandle = HiiAddPackages(
                 &gEfiCallerIdGuid,
                 mDriverHandle,
                 ${DRIVER_NAME}Strings,
                 FormBin,
                 NULL
                 );
  if (mHiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  UINTN BufferSize;
  UEFI_VARIABLE_STRUCTURE EfiVarstore;
  BufferSize = sizeof(UEFI_VARIABLE_STRUCTURE);
  Status = gRT->GetVariable (
                UEFIVariableName,
                &UEFIVariableGuid,
                NULL,
                &BufferSize,
                &EfiVarstore);
  if (EFI_ERROR(Status)) {
    ZeroMem(&EfiVarstore, sizeof(EfiVarstore));
    Status = gRT->SetVariable(
                  UEFIVariableName,
                  &UEFIVariableGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof(EfiVarstore),
                  &EfiVarstore);
    if (EFI_ERROR(Status)) {
      Print(L"Error! Can't create variable! %r\n", Status);
    }
    EFI_STRING ConfigStr = HiiConstructConfigHdr(&UEFIVariableGuid, UEFIVariableName, mDriverHandle);
    UINT16 DefaultId = 0;
    if (!HiiSetToDefaults(ConfigStr, DefaultId)) {
      Print(L"Error! Can't set default configuration #%d\n", DefaultId);
    }
  }

  return EFI_SUCCESS;
}
EOF

function C_UUID()
{
  local UUID=$(uuidgen)
  echo "{0x${UUID:0:8}, 0x${UUID:9:4}, 0x${UUID:14:4}, {0x${UUID:19:2}, 0x${UUID:21:2}, 0x${UUID:24:2}, 0x${UUID:26:2}, 0x${UUID:28:2}, 0x${UUID:30:2}, 0x${UUID:32:2}, 0x${UUID:34:2}}}"
}

FORMSET_GUID=$(C_UUID)
DATAPATH_GUID=$(C_UUID)
STORAGE_GUID=$(C_UUID)

cat << EOF > UefiLessonsPkg/${DRIVER_NAME}/Data.h
#ifndef _DATA_H_
#define _DATA_H_

#define FORMSET_GUID  ${FORMSET_GUID}
#define DATAPATH_GUID ${DATAPATH_GUID}
#define STORAGE_GUID  ${STORAGE_GUID}

#define UEFI_VARIABLE_STRUCTURE_NAME L"FormData"

#pragma pack(1)
typedef struct {
  UINT8 CheckboxValue;
  UINT16 NumericValue;
  CHAR16 StringValue[11];
  EFI_HII_DATE DateValue;
  EFI_HII_TIME TimeValue;
  UINT8 OneOfValue;
  UINT8 OrderedListValue[3];
} UEFI_VARIABLE_STRUCTURE;
#pragma pack()

#endif
EOF

cat << EOF > UefiLessonsPkg/${DRIVER_NAME}/Strings.uni
#langdef en-US "English"

#string FORMSET_TITLE          #language en-US  "Simple Formset"
#string FORMSET_HELP           #language en-US  "This is a very simple formset"
#string FORMID1_TITLE          #language en-US  "Simple Form"
#string CHECKBOX_PROMPT        #language en-US  "Checkbox prompt"
#string CHECKBOX_HELP          #language en-US  "Checkbox help"
#string NUMERIC_PROMPT         #language en-US  "Numeric prompt"
#string NUMERIC_HELP           #language en-US  "Numeric help"
#string STRING_PROMPT          #language en-US  "String prompt"
#string STRING_HELP            #language en-US  "String help"
#string DATE_PROMPT            #language en-US  "Date prompt"
#string DATE_HELP              #language en-US  "Date help"
#string TIME_PROMPT            #language en-US  "Time prompt"
#string TIME_HELP              #language en-US  "Time help"
#string ONEOF_PROMPT           #language en-US  "OneOf list prompt"
#string ONEOF_HELP             #language en-US  "OneOf list help"
#string ONEOF_OPTION1          #language en-US  "OneOf list option 1"
#string ONEOF_OPTION2          #language en-US  "OneOf list option 2"
#string ONEOF_OPTION3          #language en-US  "OneOf list option 3"
#string ORDERED_LIST_PROMPT    #language en-US  "Ordered list prompt"
#string ORDERED_LIST_HELP      #language en-US  "Ordered list help"
#string ORDERED_LIST_OPTION1   #language en-US  "Ordered list option 1"
#string ORDERED_LIST_OPTION2   #language en-US  "Ordered list option 2"
#string ORDERED_LIST_OPTION3   #language en-US  "Ordered list option 3"
#string STRING_DEFAULT         #language en-US  "String default"
#string STANDARD_DEFAULT_PROMPT     #language en-US "Standard default"
#string MFG_DEFAULT_PROMPT          #language en-US "Manufacture default"
#string BTN_STANDARD_DEFAULT_PROMPT #language en-US "Reset to standard default prompt"
#string BTN_STANDARD_DEFAULT_HELP   #language en-US "Reset to standard default help"
#string BTN_MFG_DEFAULT_PROMPT      #language en-US "Reset to manufacture default prompt"
#string BTN_MFG_DEFAULT_HELP        #language en-US "Reset to manufacture default help"
EOF

cat << EOF > UefiLessonsPkg/${DRIVER_NAME}/Form.vfr
#include <Uefi/UefiMultiPhase.h>
#include "Data.h"

formset
  guid     = FORMSET_GUID,
  title    = STRING_TOKEN(FORMSET_TITLE),
  help     = STRING_TOKEN(FORMSET_HELP),

  efivarstore UEFI_VARIABLE_STRUCTURE,
    attribute = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
    name  = FormData,
    guid  = STORAGE_GUID;

  defaultstore StandardDefault,
    prompt      = STRING_TOKEN(STANDARD_DEFAULT_PROMPT),
    attribute   = 0x0000;

  defaultstore ManufactureDefault,
    prompt      = STRING_TOKEN(MFG_DEFAULT_PROMPT),
    attribute   = 0x0001;

  form
    formid = 1,
    title = STRING_TOKEN(FORMID1_TITLE);

    checkbox
      varid = FormData.CheckboxValue,
      prompt = STRING_TOKEN(CHECKBOX_PROMPT),
      help = STRING_TOKEN(CHECKBOX_HELP),
      default = TRUE, defaultstore = StandardDefault,
      default = FALSE, defaultstore = ManufactureDefault,
    endcheckbox;

    numeric
      name = NumericQuestion,
      varid = FormData.NumericValue,
      prompt = STRING_TOKEN(NUMERIC_PROMPT),
      help = STRING_TOKEN(NUMERIC_HELP),
      flags = NUMERIC_SIZE_2 | DISPLAY_UINT_HEX,
      minimum = 0,
      maximum = 10,
      step = 1,
      default = 7, defaultstore = StandardDefault,
      default = 8, defaultstore = ManufactureDefault,
    endnumeric;

    string
      name = StringQuestion,
      varid = FormData.StringValue,
      prompt = STRING_TOKEN(STRING_PROMPT),
      help = STRING_TOKEN(STRING_HELP),
      minsize = 5,
      maxsize = 10,
      default = STRING_TOKEN(STRING_DEFAULT), defaultstore = StandardDefault,
      default = STRING_TOKEN(STRING_PROMPT), defaultstore = ManufactureDefault,
    endstring;

    date
      varid = FormData.DateValue,
      prompt = STRING_TOKEN(DATE_PROMPT),
      help = STRING_TOKEN(DATE_HELP),
      default = 2021/05/22,
    enddate;

    time
      varid = FormData.TimeValue,
      prompt = STRING_TOKEN(TIME_PROMPT),
      help = STRING_TOKEN(TIME_HELP),
      default = 23:55:33,
    endtime;

    oneof
      name = OneOfQuestion,
      varid = FormData.OneOfValue,
      prompt = STRING_TOKEN(ONEOF_PROMPT),
      help = STRING_TOKEN(ONEOF_HELP),
      option text = STRING_TOKEN(ONEOF_OPTION1), value = 0x00, flags = 0;
      option text = STRING_TOKEN(ONEOF_OPTION2), value = 0x33, flags = MANUFACTURING;
      option text = STRING_TOKEN(ONEOF_OPTION3), value = 0x55, flags = DEFAULT;
    endoneof;

    orderedlist
      varid = FormData.OrderedListValue,
      prompt = STRING_TOKEN(ORDERED_LIST_PROMPT),
      help = STRING_TOKEN(ORDERED_LIST_HELP),
      option text = STRING_TOKEN(ORDERED_LIST_OPTION1), value = 0x0A, flags = 0;
      option text = STRING_TOKEN(ORDERED_LIST_OPTION2), value = 0x0B, flags = 0;
      option text = STRING_TOKEN(ORDERED_LIST_OPTION3), value = 0x0C, flags = 0;
      default = {0x0c, 0x0b, 0x0a},
    endlist;

    resetbutton
      defaultstore = StandardDefault,
      prompt   = STRING_TOKEN(BTN_STANDARD_DEFAULT_PROMPT),
      help     = STRING_TOKEN(BTN_STANDARD_DEFAULT_HELP),
    endresetbutton;

    resetbutton
      defaultstore = ManufactureDefault,
      prompt   = STRING_TOKEN(BTN_MFG_DEFAULT_PROMPT),
      help     = STRING_TOKEN(BTN_MFG_DEFAULT_HELP),
    endresetbutton;
  endform;
endformset;
EOF
