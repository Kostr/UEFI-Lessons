#!/bin/bash

# This is a simple script that creates a basic structure for your new UEFI application
# Put this script in your edk2 folder and run it with 1 argument - your new application name

APP_NAME=${1}

UUID=$(uuidgen)

mkdir -p UefiLessonsPkg/${APP_NAME}

cat << EOF > UefiLessonsPkg/${APP_NAME}/${APP_NAME}.inf
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = ${APP_NAME}
  FILE_GUID                      = ${UUID}
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = UefiMain

[Sources]
  ${APP_NAME}.c

[Packages]
  MdePkg/MdePkg.dec

[LibraryClasses]
  UefiApplicationEntryPoint
  UefiLib
EOF

cat << EOF > UefiLessonsPkg/${APP_NAME}/${APP_NAME}.c
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}
EOF

