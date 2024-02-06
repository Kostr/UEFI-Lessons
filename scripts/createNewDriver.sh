#!/bin/bash
##
# Copyright (c) 2024, Konstantin Aladyshev <aladyshev22@gmail.com>
#
# SPDX-License-Identifier: MIT
##

# This is a simple script that creates a basic structure for your new UEFI driver
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

[Packages]
  MdePkg/MdePkg.dec

[LibraryClasses]
  UefiDriverEntryPoint
  UefiLib
EOF

cat << EOF > UefiLessonsPkg/${DRIVER_NAME}/${DRIVER_NAME}.c
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>


EFI_STATUS
EFIAPI
${DRIVER_NAME}Unload (
  EFI_HANDLE ImageHandle
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
${DRIVER_NAME}EntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}
EOF

