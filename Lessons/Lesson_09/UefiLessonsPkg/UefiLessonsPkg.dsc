##
# Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
#
# SPDX-License-Identifier: MIT
##

[Defines]
  DSC_SPECIFICATION              = 0x0001001C
  PLATFORM_GUID                  = 3db7270f-ffac-4139-90a4-0ae68f3f8167
  PLATFORM_VERSION               = 0.01
  PLATFORM_NAME                  = UefiLessonsPkg
  SKUID_IDENTIFIER               = DEFAULT
  SUPPORTED_ARCHITECTURES        = X64
  BUILD_TARGETS                  = RELEASE


[LibraryClasses]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf

[Components]
  UefiLessonsPkg/SimplestApp/SimplestApp.inf
  UefiLessonsPkg/HelloWorld/HelloWorld.inf
  UefiLessonsPkg/ImageHandle/ImageHandle.inf
  UefiLessonsPkg/ImageInfo/ImageInfo.inf


