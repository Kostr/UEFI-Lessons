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
  BUILD_TARGETS                  = RELEASE|DEBUG


[LibraryClasses]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  #DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
  DebugLib|OvmfPkg/Library/PlatformDebugLibIoPort/PlatformDebugLibIoPort.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
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
  #UefiLessonsPkg/PCDLesson/PCDLesson.inf
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
  UefiLessonsPkg/HIIStringsUNI/HIIStringsUNI.inf
  UefiLessonsPkg/HIIStringsUNIRC/HIIStringsUNIRC.inf
  UefiLessonsPkg/HIIStringsMan/HIIStringsMan.inf
  UefiLessonsPkg/HIIAddRussianFont/HIIAddRussianFont.inf
  UefiLessonsPkg/HIIAddLocalization/HIIAddLocalization.inf
  UefiLessonsPkg/AddNewLanguage/AddNewLanguage.inf
  UefiLessonsPkg/HIISimpleForm/HIISimpleForm.inf
  UefiLessonsPkg/HIIStaticForm/HIIStaticForm.inf
  UefiLessonsPkg/HIIStaticFormDriver/HIIStaticFormDriver.inf
  UefiLessonsPkg/DisplayHIIByGuid/DisplayHIIByGuid.inf
  UefiLessonsPkg/SetVariableExample/SetVariableExample.inf
  UefiLessonsPkg/UpdateDmpstoreDump/UpdateDmpstoreDump.inf
  UefiLessonsPkg/DevicePath/DevicePath.inf
  UefiLessonsPkg/HIIFormCheckbox/HIIFormCheckbox.inf
  UefiLessonsPkg/HIIFormDataElements/HIIFormDataElements.inf
  UefiLessonsPkg/HIIFormLabel/HIIFormLabel.inf
  UefiLessonsPkg/HIIConfig/HIIConfig.inf
  UefiLessonsPkg/HIIKeyword/HIIKeyword.inf
  UefiLessonsPkg/HIIFormDataElementsWithKeywords/HIIFormDataElementsWithKeywords.inf
  UefiLessonsPkg/HIIFormDataElementsWithDefaultsSet/HIIFormDataElementsWithDefaultsSet.inf
  UefiLessonsPkg/HIIFormDataElementsVarstore/HIIFormDataElementsVarstore.inf
  UefiLessonsPkg/FfsFile/FfsFile.inf
  UefiLessonsPkg/SetSku/SetSku.inf
  UefiLessonsPkg/HiddenSettings/HiddenSettings.inf
  UefiLessonsPkg/ShowHIIext/ShowHIIext.inf
  UefiLessonsPkg/ProtocolEventDriver/ProtocolEventDriver.inf
  UefiLessonsPkg/ShowStrings/ShowStrings.inf
  UefiLessonsPkg/PasswordForm/PasswordForm.inf

#[PcdsFixedAtBuild]
#  gUefiLessonsPkgTokenSpaceGuid.PcdInt8|0x88|UINT8|0x3B81CDF1
#  gUefiLessonsPkgTokenSpaceGuid.PcdInt16|0x1616|UINT16|0x77DFB6E6
#  gUefiLessonsPkgTokenSpaceGuid.PcdInt32|0x32323232|UINT32|0xF2A48130
#  gUefiLessonsPkgTokenSpaceGuid.PcdInt64|0x6464646464646464|UINT64|0x652F4E29
#  gUefiLessonsPkgTokenSpaceGuid.PcdBool|TRUE|BOOLEAN|0x69E88A63
#  gUefiLessonsPkgTokenSpaceGuid.PcdExpression|0xFF000000 + 0x00FFFFFF|UINT32|0x9C405222
#  gUefiLessonsPkgTokenSpaceGuid.PcdExpression_1|((0xFFFFFFFF & 0x000000FF) << 8) + 0x33|UINT32|0x5911C44B
#  gUefiLessonsPkgTokenSpaceGuid.PcdExpression_2|(0x00000000 | 0x00100000)|UINT32|0xAD880207
#  gUefiLessonsPkgTokenSpaceGuid.PcdExpression_3|((56 < 78) || !(23 > 44))|BOOLEAN|0x45EDE955
#  gUefiLessonsPkgTokenSpaceGuid.PcdAsciiStr|"hello"|VOID*|0xB29914B5
#  gUefiLessonsPkgTokenSpaceGuid.PcdUCS2Str|L"hello"|VOID*|0xF22124E5
#  gUefiLessonsPkgTokenSpaceGuid.PcdArray|{0xa5, 0xA6, 0xA7}|VOID*|0xD5DB9A27
#  gUefiLessonsPkgTokenSpaceGuid.PcdGuidInBytes|{0x07, 0x07, 0x74, 0xF1, 0x1D, 0x69, 0x03, 0x42, 0xBF, 0xAB, 0x99, 0xE1, 0x32, 0xFA, 0x41, 0x66}|VOID*|0xB9E0CDC0
#  gUefiLessonsPkgTokenSpaceGuid.PcdGuid|{GUID("f1740707-691d-4203-bfab-99e132fa4166")}|VOID*|0x7F2066F7
#  gUefiLessonsPkgTokenSpaceGuid.PcdGuidByPCD|{GUID(gUefiLessonsPkgTokenSpaceGuid)}|VOID*|0x0860CCD5
#  gUefiLessonsPkgTokenSpaceGuid.PcdGuidByEfiGuid|{GUID({0x150cab53, 0xad47, 0x4385, {0xb5, 0xdd, 0xbc, 0xfc, 0x76, 0xba, 0xca, 0xf0}})}|VOID*|0x613506D5
#  gUefiLessonsPkgTokenSpaceGuid.PcdDevicePath|{DEVICE_PATH("PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)")}|VOID*|0xC56EE1E2
#  gUefiLessonsPkgTokenSpaceGuid.PcdIntCasts|{UINT16(0x1122), UINT32(0x33445566), UINT8(0x77), UINT64(0x8899aabbccddeeff)}|VOID*|0x647456A6
#  gUefiLessonsPkgTokenSpaceGuid.PcdWithLabels|{ 0x0A, 0x0B, OFFSET_OF(End), 0x0C, LABEL(Start) 0x0D, LABEL(End) 0x0E, 0x0F, OFFSET_OF(Start) }|VOID*|0xD91A8BF6
#  gUefiLessonsPkgTokenSpaceGuid.PcdArrayExt|{0x11, UINT16(0x2233), UINT32(0x44556677), L"hello", "world!", GUID("09b9b358-70bd-421e-bafb-4f97e2ac7d44")}|VOID*|0x7200C5DF
#  gUefiLessonsPkgTokenSpaceGuid.PcdCustomStruct|{0}|CustomStruct|0x535D4CB5 {
#    <Packages>
#      UefiLessonsPkg/UefiLessonsPkg.dec
#    <HeaderFiles>
#      CustomPcdTypes.h
#  }
#  gUefiLessonsPkgTokenSpaceGuid.PcdCustomStruct.Val8|0x11
#  gUefiLessonsPkgTokenSpaceGuid.PcdCustomStruct.Val32[0]|0x22334455
#  gUefiLessonsPkgTokenSpaceGuid.PcdCustomStruct.Val32[1]|0x66778899
#  gUefiLessonsPkgTokenSpaceGuid.PcdCustomStruct.ValStruct.Guid|{GUID("f1740707-691d-4203-bfab-99e132fa4166")}
#  gUefiLessonsPkgTokenSpaceGuid.PcdCustomStruct.ValStruct.Name|L'Hello'
#  gUefiLessonsPkgTokenSpaceGuid.PcdCustomStruct.ValUnion.BitFields.Field2|0xF

#  gUefiLessonsPkgTokenSpaceGuid.PcdCustomStruct_1|{CODE(
#    {
#      0x11,
#      {0x22334455, 0x66778899},
#      {
#        {0xf1740707, 0x691d, 0x4203, {0xbf, 0xab, 0x99, 0xe1, 0x32, 0xfa, 0x41, 0x66}},
#        {0x0048, 0x0065, 0x006c, 0x006c, 0x006f, 0x0000}
#      },
#      {{0x0, 0xf, 0x0}}
#    }
#   )}|CustomStruct|0xC1D6B9A7 {
#    <Packages>
#      UefiLessonsPkg/UefiLessonsPkg.dec
#    <HeaderFiles>
#      CustomPcdTypes.h
#  }

#  gUefiLessonsPkgTokenSpaceGuid.PcdArrayWithFixedSize|{0x0}|UINT8[8]|0x4C4CB9A3
#  gUefiLessonsPkgTokenSpaceGuid.PcdArrayWithFixedSize_1|{0x0}|UINT32[3]|0x285DAD21
#  gUefiLessonsPkgTokenSpaceGuid.PcdArrayWithFixedSize_2|{UINT32(0x11223344), UINT32(0x55667788), UINT32(0x99aabbcc)}|UINT32[3]|0x25D6ED26
#  gUefiLessonsPkgTokenSpaceGuid.PcdArrayWithFixedSize_3|{CODE({0x11223344, 0x55667788, 0x99aabbcc})}|UINT32[3]|0xE5BC424D
#  gUefiLessonsPkgTokenSpaceGuid.PcdArrayWithFixedSize_4|{0x0}|CustomStruct[2]|0x0D00EE44 {
#    <Packages>
#      UefiLessonsPkg/UefiLessonsPkg.dec
#    <HeaderFiles>
#      CustomPcdTypes.h
#  }

#  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|42|UINT32|0x3CB8ABB8

#  gUefiLessonsPkgTokenSpaceGuid.Region1Offset|0|UINT32|0x00000005
#  gUefiLessonsPkgTokenSpaceGuid.Region1Size|0|UINT32|0x00000006

#[PcdsFeatureFlag]
#  gUefiLessonsPkgTokenSpaceGuid.PcdFeatureFlag|TRUE|BOOLEAN|0x16DD586E

#[PcdsPatchableInModule]
#  gUefiLessonsPkgTokenSpaceGuid.PcdPatchableInt32|0x31313131|UINT32|0xFCDA11B5

#[PcdsDynamic]
#  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32|0xCAFECAFE|UINT32|0x4F9259A3

#[PcdsDynamicEx]
#  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|0xBABEBABE|UINT32|0xAF35F3B2

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF
  gEfiMdePkgTokenSpaceGuid.PcdFixedDebugPrintErrorLevel|0xFFFFFFFF
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x8000004F
