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
  #PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf  
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  #SimpleLibrary|UefiLessonsPkg/Library/SimpleLibrary/SimpleLibrary.inf
  #SimpleLibrary|UefiLessonsPkg/Library/SimpleLibraryWithConstructor/SimpleLibraryWithConstructor.inf
  SimpleLibrary|UefiLessonsPkg/Library/SimpleLibraryWithConstructorAndDestructor/SimpleLibraryWithConstructorAndDestructor.inf

[Components]
  UefiLessonsPkg/SimplestApp/SimplestApp.inf
  UefiLessonsPkg/HelloWorld/HelloWorld.inf
  UefiLessonsPkg/ImageHandle/ImageHandle.inf
  UefiLessonsPkg/ImageInfo/ImageInfo.inf
  UefiLessonsPkg/MemoryInfo/MemoryInfo.inf
  UefiLessonsPkg/SimpleShellApp/SimpleShellApp.inf
  UefiLessonsPkg/ListVariables/ListVariables.inf
  UefiLessonsPkg/ShowBootVariables/ShowBootVariables.inf
  UefiLessonsPkg/InteractiveApp/InteractiveApp.inf
  UefiLessonsPkg/PCDLesson/PCDLesson.inf
  UefiLessonsPkg/SmbiosInfo/SmbiosInfo.inf
  UefiLessonsPkg/ShowTables/ShowTables.inf
  UefiLessonsPkg/AcpiInfo/AcpiInfo.inf
  UefiLessonsPkg/SaveBGRT/SaveBGRT.inf
  UefiLessonsPkg/ListPCI/ListPCI.inf
  UefiLessonsPkg/SimpleDriver/SimpleDriver.inf
  UefiLessonsPkg/PCIRomInfo/PCIRomInfo.inf
  UefiLessonsPkg/Library/SimpleLibrary/SimpleLibrary.inf
  UefiLessonsPkg/Library/SimpleLibraryWithConstructor/SimpleLibraryWithConstructor.inf
  UefiLessonsPkg/SimpleLibraryUser/SimpleLibraryUser.inf
  UefiLessonsPkg/SimpleClassProtocol/SimpleClassProtocol.inf
  UefiLessonsPkg/SimpleClassUser/SimpleClassUser.inf
  UefiLessonsPkg/HotKeyDriver/HotKeyDriver.inf
  UefiLessonsPkg/ShowHII/ShowHII.inf
  UefiLessonsPkg/HIIStringsC/HIIStringsC.inf

[PcdsFixedAtBuild]
  gUefiLessonsPkgTokenSpaceGuid.PcdMyVar32_2|44

