#!/bin/bash

# This is a simple script that creates a basic structure for your new UEFI shell application
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
  ENTRY_POINT                    = ShellCEntryLib

[Sources]
  ${APP_NAME}.c

[Packages]
  MdePkg/MdePkg.dec

[LibraryClasses]
  ShellCEntryLib
  UefiLib
EOF

cat << EOF > UefiLessonsPkg/${APP_NAME}/${APP_NAME}.c
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  return EFI_SUCCESS;
}
EOF

