Let's talk about the `Human Interface Infrastructure (HII)` in the UEFI.

Every BIOS has some form of interaction with a user. For example it can have an `image` that you see on boot. If you press a special key on boot you would go to the BIOS menu, which would have some `forms` to control BIOS settings. Text in those forms can be written with custom `fonts`. If you change language setting in BIOS, all the `strings` in the interface should be translated to the new language.

The main goal of HII is to provide a standartizied interface to retreive and extend all these different parts of human interface. With HII external driver/application has an easy way to install new elements (such as fonts, strings, images and forms) to the platfrom or get the ones that are currently available for its internal use.

This is possible because all the HII data is getting stored in a special database - HII Database, which acts as a central repository for the entire platform.

In the end a so called `Form Browser` would use HII Database data to display user interface and interact with a user.

Let's create an application that would explore the HII Database content.

We've created new apps many times already, so I've decided to create a template script to ease this repetative task. Put script `createNewApp.sh` in the root of `edk2` folder and initialize new app `ShowHII`:
```
./createNewApp.sh ShowHII
```
This would create `UefiLessonsPkg/ShowHII` app folder with an INF and *.c file.
Add newly created app to the `UefiLessonsPkg/UefiLessonsPkg.dsc`
```
[Components]
  ...
  UefiLessonsPkg/ShowHII/ShowHII.inf
```
To access HII database we need to get `EFI_HII_DATABASE_PROTOCOL`:
```
EFI_HII_DATABASE_PROTOCOL

Summary:
Database manager for HII-related data structures.

Protocol:
typedef struct _EFI_HII_DATABASE_PROTOCOL {
 EFI_HII_DATABASE_NEW_PACK NewPackageList;
 EFI_HII_DATABASE_REMOVE_PACK RemovePackageList;
 EFI_HII_DATABASE_UPDATE_PACK UpdatePackageList;
 EFI_HII_DATABASE_LIST_PACKS ListPackageLists;
 EFI_HII_DATABASE_EXPORT_PACKS ExportPackageLists;
 EFI_HII_DATABASE_REGISTER_NOTIFY RegisterPackageNotify;
 EFI_HII_DATABASE_UNREGISTER_NOTIFY UnregisterPackageNotify;
 EFI_HII_FIND_KEYBOARD_LAYOUTS FindKeyboardLayouts;
 EFI_HII_GET_KEYBOARD_LAYOUT GetKeyboardLayout;
 EFI_HII_SET_KEYBOARD_LAYOUT SetKeyboardLayout;
 EFI_HII_DATABASE_GET_PACK_HANDLE GetPackageListHandle;
} EFI_HII_DATABASE_PROTOCOL;
```

Right now we are interested in the function `ExportPackageLists`:
```
EFI_HII_DATABASE_PROTOCOL.ExportPackageLists()

Summary:
Exports the contents of one or all package lists in the HII database into a buffer.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_HII_DATABASE_EXPORT_PACKS) (
 IN CONST EFI_HII_DATABASE_PROTOCOL *This,
 IN EFI_HII_HANDLE Handle,
 IN OUT UINTN *BufferSize,
 OUT EFI_HII_PACKAGE_LIST_HEADER *Buffer
 );
 
Parameters:
This		A pointer to the EFI_HII_DATABASE_PROTOCOL instance.
Handle		An EFI_HII_HANDLE that corresponds to the desired package list in the HII
		database to export or NULL to indicate all package lists should be exported.
BufferSize	On input, a pointer to the length of the buffer. On output, the length of the buffer
		that is required for the exported data.
Buffer		A pointer to a buffer that will contain the results of the export function. 
```
As you can see as a result of this function we would get `package lists`. That is because data in the HII database is organized in these package lists identified by GUID. In turn each package list can have several packages of different types (form/font/strings/...).

First we need to locate the `EFI_HII_DATABASE_PROTOCOL` in our application:
```
  EFI_STATUS Status;
  EFI_HII_DATABASE_PROTOCOL* HiiDbProtocol;
  Status = gBS->LocateProtocol(&gEfiHiiDatabaseProtocolGuid,
                               NULL, 
                               (VOID**)&HiiDbProtocol);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Could not find HII Database protocol: %r\n", Status);
    return Status;
  }
```
For correct build don't forget to add `gEfiHiiDatabaseProtocolGuid` to the `ShowHII.inf` file:
```
[Protocols]
  gEfiHiiDatabaseProtocolGuid
```
And add necessary include in the `ShowHII.c` file:
```
#include <Protocol/HiiDatabase.h>
```

Now let's use the protocol `ExportPackageLists` function. Couple of things to mention:
- we would use `NULL` for the `Handle` parameter to get not some package list from the database, but all the package lists from the database,
- we don't know the size of the output array beforehand. To get it `ExportPackageLists` use the standard UEFI mechanics:
1) first we call `ExportPackageLists` with a `BufferSize=0`. As a result we should get `EFI_BUFFER_TOO_SMALL` as a return value, but the `BufferSize` would be filled with a necessary size value,
2) we allocate buffer of necessary size with a `gBS->AllocatePool` call,
3) we call `ExportPackageLists` one more time, now with correct buffer and size.
- don't forget to free allocated buffer when it is necessay. For this you can utilize `FreePool` function from the `Library/MemoryAllocationLib.h`
```
  UINTN PackageListSize = 0;
  EFI_HII_PACKAGE_LIST_HEADER* PackageList = NULL;

  Status = HiiDbProtocol->ExportPackageLists(HiiDbProtocol,
                                             NULL,             // All package lists
                                             &PackageListSize,
                                             PackageList);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    Print(L"ERROR: Could not obtain package list size\n");
    return Status;
  }

  Status = gBS->AllocatePool(EfiBootServicesData, 
                             PackageListSize, 
                             (VOID**)&PackageList);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Could not allocate sufficient memory for package list: %r\n", Status);
    return Status;
  }


  Status = HiiDbProtocol->ExportPackageLists(HiiDbProtocol,
                                             NULL,
                                             &PackageListSize,
                                             PackageList);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Could not retrieve the package list: %r\n", Status);
    FreePool(PackageList);
    return Status;
  }

  // <Process data>
  
  FreePool(PackageList);
```

Now let's process data that we've acquired. Our data is a set of package lists, each of which has a special header `EFI_HII_PACKAGE_LIST_HEADER`:
```
EFI_HII_PACKAGE_LIST_HEADER
Summary:
The header found at the start of each package list. 

Prototype:
typedef struct {
 EFI_GUID PackageListGuid;
 UINT32 PackagLength;
} EFI_HII_PACKAGE_LIST_HEADER;

Members:
PackageListGuid		The unique identifier applied to the list of packages which follows.
PackageLength 		The size of the package list (in bytes), including the header. 
```

Let's create a function with a loop that would parse the data and print all the info from these package list headers:
```
VOID ParseHiiPackageLists(EFI_HII_PACKAGE_LIST_HEADER* HiiDatabase, UINTN HiiDatabaseSize)
{
  EFI_HII_PACKAGE_LIST_HEADER* HiiPackageListHeader;
  HiiPackageListHeader = (EFI_HII_PACKAGE_LIST_HEADER*) HiiDatabase;

  UINTN i=0;
  while ((UINTN) HiiPackageListHeader < ((UINTN) HiiDatabase + HiiDatabaseSize)) {
    UINTN HiiPackageListSize = HiiPackageListHeader->PackageLength;
    if (HiiPackageListSize == 0)
      break;
    Print(L"PackageList[%d]: GUID=%g; size=0x%X\n", i++, HiiPackageListHeader->PackageListGuid, HiiPackageListHeader->PackageLength);

    <...>	// Parse PackageList

    // Go to next PackageList
    HiiPackageListHeader = (EFI_HII_PACKAGE_LIST_HEADER*)((UINTN) HiiPackageListHeader + HiiPackageListSize);
  }
}
```

Call this function with a:
```
ParseHiiPackageLists(PackageList, PackageListSize);
```

If we build and execute our application right now, we would get something like this:
```
FS0:\> ShowHII.efi
PackageList[0]: GUID=A487A478-51EF-48AA-8794-7BEE2A0562F1; size=0x1ADC
PackageList[1]: GUID=19618BCE-55AE-09C6-37E9-4CE04084C7A1; size=0x21E4
PackageList[2]: GUID=2F30DA26-F51B-4B6F-85C4-31873C281BCA; size=0xA93
PackageList[3]: GUID=F74D20EE-37E7-48FC-97F7-9B1047749C69; size=0x2EE9
PackageList[4]: GUID=EBF8ED7C-0DD1-4787-84F1-F48D537DCACF; size=0x46C
PackageList[5]: GUID=FE561596-E6BF-41A6-8376-C72B719874D0; size=0x93F
PackageList[6]: GUID=2A46715F-3581-4A55-8E73-2B769AAA30C5; size=0x6B0
PackageList[7]: GUID=99FDC8FD-849B-4EBA-AD13-FB9699C90A4D; size=0x6FE
PackageList[8]: GUID=E38C1029-E38F-45B9-8F0D-E2E60BC9B262; size=0x15DA
PackageList[9]: GUID=D9DCC5DF-4007-435E-9098-8970935504B2; size=0x855
PackageList[10]: GUID=F5F219D3-7006-4648-AC8D-D61DFB7BC6AD; size=0x14EC
PackageList[11]: GUID=4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9; size=0x6AC8
PackageList[12]: GUID=F95A7CCC-4C55-4426-A7B4-DC8961950BAE; size=0x13909
PackageList[13]: GUID=DEC5DAA4-6781-4820-9C63-A7B0E4F1DB31; size=0x8677
PackageList[14]: GUID=4344558D-4EF9-4725-B1E4-3376E8D6974F; size=0x83BD
PackageList[15]: GUID=0AF0B742-63EC-45BD-8DB6-71AD7F2FE8E8; size=0xCB04
PackageList[16]: GUID=25F200AA-D3CB-470A-BF51-E7D162D22E6F; size=0x1D3D7
PackageList[17]: GUID=5F5F605D-1583-4A2D-A6B2-EB12DAB4A2B6; size=0x3048
PackageList[18]: GUID=F3D301BB-F4A5-45A8-B0B7-FA999C6237AE; size=0x26B5
PackageList[19]: GUID=7C04A583-9E3E-4F1C-AD65-E05268D0B4D1; size=0x5CB
```

Now let's investigate our PackageLists. Each `PackageList` consist of several packages which starts right after the `EFI_HII_PACKAGE_LIST_HEADER`. Each package in turn would have its own `EFI_HII_PACKAGE_HEADER`:
```
EFI_HII_PACKAGE_HEADER

Summary:
The header found at the start of each package.

Prototype:
typedef struct {
 UINT32 Length:24;
 UINT32 Type:8;
 UINT8 Data[ … ];
} EFI_HII_PACKAGE_HEADER;

Members:
Length 		The size of the package in bytes.
Type 		The package type. See EFI_HII_PACKAGE_TYPE_x, below.
Data 		The package data, the format of which is determined by Type.
```

Here are descriptions for some of the available package types:
```
Package Type Description:
#define EFI_HII_PACKAGE_TYPE_ALL     0x00 		// Pseudo-package type used when exporting package lists.
#define EFI_HII_PACKAGE_TYPE_GUID    0x01 		// Package type where the format of the data
												// is specified using a GUID immediately
												// following the package header.
#define EFI_HII_PACKAGE_FORMS        0x02 		// Forms package.
#define EFI_HII_PACKAGE_STRINGS      0x04 		// Strings package
#define EFI_HII_PACKAGE_FONTS        0x05 		// Fonts package.
#define EFI_HII_PACKAGE_IMAGES       0x06 		// Images package.
#define EFI_HII_PACKAGE_SIMPLE_FONTS 0x07 		// Simplified (8x19, 16x19) Fonts package
#define EFI_HII_PACKAGE_DEVICE_PATH  0x08 		// Binary-encoded device path.
#define EFI_HII_PACKAGE_ANIMATIONS   0x0A		// Animations package.
#define EFI_HII_PACKAGE_END          0xDF 		// Used to mark the end of a package list.
```

Package list can have as many packages as needed. All of them would be concatenated together. After all the data packages in a package list there should be a package of type `EFI_HII_PACKAGE_END` that mark the end of the package list.

With all this info in mind let's add code that would parse all the packages inside the package list:
```
EFI_HII_PACKAGE_HEADER* HiiPackageHeader = (EFI_HII_PACKAGE_HEADER*)((UINTN) HiiPackageListHeader + sizeof(EFI_HII_PACKAGE_LIST_HEADER));
UINTN j=0;
while ((UINTN) HiiPackageHeader < ((UINTN) HiiPackageListHeader + HiiPackageListSize)) {
  Print(L"\tPackage[%d]: type=%s; size=0x%X\n", j++, PackageType(HiiPackageHeader->Type), HiiPackageHeader->Length);

  // Go to next Package
  HiiPackageHeader = (EFI_HII_PACKAGE_HEADER*)((UINTN) HiiPackageHeader + HiiPackageHeader->Length);
}
```
Here I've used `PackageType` function to transform package type value to a printable string. This function can be defined like this:
```
CHAR16* PackageType(UINTN Type)
{
  switch(Type) {
    case EFI_HII_PACKAGE_TYPE_ALL:
      return L"ALL";
    case EFI_HII_PACKAGE_TYPE_GUID:
      return L"GUID";
    case EFI_HII_PACKAGE_FORMS:
      return L"FORMS";
    case EFI_HII_PACKAGE_STRINGS:
      return L"STRINGS";
    case EFI_HII_PACKAGE_FONTS:
      return L"FONTS";
    case EFI_HII_PACKAGE_IMAGES:
      return L"IMAGES";
    case EFI_HII_PACKAGE_SIMPLE_FONTS:
      return L"SIMPLE_FONTS";
    case EFI_HII_PACKAGE_DEVICE_PATH:
      return L"DEVICE_PATH";
    case EFI_HII_PACKAGE_KEYBOARD_LAYOUT:
      return L"KEYBOARD_LAYOUT";
    case EFI_HII_PACKAGE_ANIMATIONS:
      return L"ANIMATIONS";
    case EFI_HII_PACKAGE_END:
      return L"END";
    case EFI_HII_PACKAGE_TYPE_SYSTEM_BEGIN:
      return L"SYSTEM_BEGIN";
    case EFI_HII_PACKAGE_TYPE_SYSTEM_END:
      return L"SYSTEM_END";
  }
  return L"UNKNOWN";
}
```

Now it is time to build our app and execute it under OVMF:
```
FS0:\> ShowHII.efi
PackageList[0]: GUID=A487A478-51EF-48AA-8794-7BEE2A0562F1; size=0x1ADC
        Package[0]: type=STRINGS; size=0x1AC4
        Package[1]: type=END; size=0x4
PackageList[1]: GUID=19618BCE-55AE-09C6-37E9-4CE04084C7A1; size=0x21E4
        Package[0]: type=STRINGS; size=0x21CC
        Package[1]: type=END; size=0x4
PackageList[2]: GUID=2F30DA26-F51B-4B6F-85C4-31873C281BCA; size=0xA93
        Package[0]: type=STRINGS; size=0xA7B
        Package[1]: type=END; size=0x4
PackageList[3]: GUID=F74D20EE-37E7-48FC-97F7-9B1047749C69; size=0x2EE9
        Package[0]: type=IMAGES; size=0x2ED1
        Package[1]: type=END; size=0x4
PackageList[4]: GUID=EBF8ED7C-0DD1-4787-84F1-F48D537DCACF; size=0x46C
        Package[0]: type=FORMS; size=0x82
        Package[1]: type=FORMS; size=0x82
        Package[2]: type=STRINGS; size=0x199
        Package[3]: type=STRINGS; size=0x19B
        Package[4]: type=DEVICE_PATH; size=0x1C
        Package[5]: type=END; size=0x4
PackageList[5]: GUID=FE561596-E6BF-41A6-8376-C72B719874D0; size=0x93F
        Package[0]: type=FORMS; size=0xF5
        Package[1]: type=STRINGS; size=0x40A
        Package[2]: type=STRINGS; size=0x40C
        Package[3]: type=DEVICE_PATH; size=0x1C
        Package[4]: type=END; size=0x4
PackageList[6]: GUID=2A46715F-3581-4A55-8E73-2B769AAA30C5; size=0x6B0
        Package[0]: type=FORMS; size=0x143
        Package[1]: type=STRINGS; size=0x539
        Package[2]: type=DEVICE_PATH; size=0x1C
        Package[3]: type=END; size=0x4
PackageList[7]: GUID=99FDC8FD-849B-4EBA-AD13-FB9699C90A4D; size=0x6FE
        Package[0]: type=STRINGS; size=0x340
        Package[1]: type=STRINGS; size=0x3A6
        Package[2]: type=END; size=0x4
PackageList[8]: GUID=E38C1029-E38F-45B9-8F0D-E2E60BC9B262; size=0x15DA
        Package[0]: type=STRINGS; size=0xA88
        Package[1]: type=STRINGS; size=0xB3A
        Package[2]: type=END; size=0x4
PackageList[9]: GUID=D9DCC5DF-4007-435E-9098-8970935504B2; size=0x855
        Package[0]: type=FORMS; size=0x1F6
        Package[1]: type=STRINGS; size=0x62B
        Package[2]: type=DEVICE_PATH; size=0x1C
        Package[3]: type=END; size=0x4
PackageList[10]: GUID=F5F219D3-7006-4648-AC8D-D61DFB7BC6AD; size=0x14EC
        Package[0]: type=SIMPLE_FONTS; size=0x14D4
        Package[1]: type=END; size=0x4
PackageList[11]: GUID=4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9; size=0x6AC8
        Package[0]: type=FORMS; size=0x1030
        Package[1]: type=STRINGS; size=0x3C99
        Package[2]: type=STRINGS; size=0x1DCB
        Package[3]: type=DEVICE_PATH; size=0x1C
        Package[4]: type=END; size=0x4
PackageList[12]: GUID=F95A7CCC-4C55-4426-A7B4-DC8961950BAE; size=0x13909
        Package[0]: type=STRINGS; size=0x138F1
        Package[1]: type=END; size=0x4
PackageList[13]: GUID=DEC5DAA4-6781-4820-9C63-A7B0E4F1DB31; size=0x8677
        Package[0]: type=STRINGS; size=0x865F
        Package[1]: type=END; size=0x4
PackageList[14]: GUID=4344558D-4EF9-4725-B1E4-3376E8D6974F; size=0x83BD
        Package[0]: type=STRINGS; size=0x83A5
        Package[1]: type=END; size=0x4
PackageList[15]: GUID=0AF0B742-63EC-45BD-8DB6-71AD7F2FE8E8; size=0xCB04
        Package[0]: type=STRINGS; size=0xCAEC
        Package[1]: type=END; size=0x4
PackageList[16]: GUID=25F200AA-D3CB-470A-BF51-E7D162D22E6F; size=0x1D3D7
        Package[0]: type=STRINGS; size=0x1D3BF
        Package[1]: type=END; size=0x4
PackageList[17]: GUID=5F5F605D-1583-4A2D-A6B2-EB12DAB4A2B6; size=0x3048
        Package[0]: type=STRINGS; size=0x3030
        Package[1]: type=END; size=0x4
PackageList[18]: GUID=F3D301BB-F4A5-45A8-B0B7-FA999C6237AE; size=0x26B5
        Package[0]: type=STRINGS; size=0x269D
        Package[1]: type=END; size=0x4
PackageList[19]: GUID=7C04A583-9E3E-4F1C-AD65-E05268D0B4D1; size=0x5CB
        Package[0]: type=STRINGS; size=0x5B3
        Package[1]: type=END; size=0x4
```

________________________

Let's put some context to the GUIDs from the output. We would return to it later, when we investigate more about diferent HII packages.

https://github.com/tianocore/edk2/blob/master/ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf
```
PackageList[0]: GUID=A487A478-51EF-48AA-8794-7BEE2A0562F1; size=0x1ADC
        Package[0]: type=STRINGS; size=0x1AC4
        Package[1]: type=END; size=0x4
```

https://github.com/tianocore/edk2/blob/master/ShellPkg/DynamicCommand/HttpDynamicCommand/HttpDynamicCommand.inf
```
PackageList[1]: GUID=19618BCE-55AE-09C6-37E9-4CE04084C7A1; size=0x21E4
        Package[0]: type=STRINGS; size=0x21CC
        Package[1]: type=END; size=0x4
```

https://github.com/tianocore/edk2/blob/master/OvmfPkg/LinuxInitrdDynamicShellCommand/LinuxInitrdDynamicShellCommand.inf
```
PackageList[2]: GUID=2F30DA26-F51B-4B6F-85C4-31873C281BCA; size=0xA93
        Package[0]: type=STRINGS; size=0xA7B
        Package[1]: type=END; size=0x4
```

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Logo/LogoDxe.inf
```
PackageList[3]: GUID=F74D20EE-37E7-48FC-97F7-9B1047749C69; size=0x2EE9
        Package[0]: type=IMAGES; size=0x2ED1
        Package[1]: type=END; size=0x4
```

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/DriverHealthManagerDxe/DriverHealthManagerDxe.inf
```
PackageList[4]: GUID=EBF8ED7C-0DD1-4787-84F1-F48D537DCACF; size=0x46C
        Package[0]: type=FORMS; size=0x82
        Package[1]: type=FORMS; size=0x82
        Package[2]: type=STRINGS; size=0x199
        Package[3]: type=STRINGS; size=0x19B
        Package[4]: type=DEVICE_PATH; size=0x1C
        Package[5]: type=END; size=0x4
```

`EFI_FILE_EXPLORE_FORMSET_GUID`
https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/FileExplorerLib/FormGuid.h
```
PackageList[5]: GUID=FE561596-E6BF-41A6-8376-C72B719874D0; size=0x93F
        Package[0]: type=FORMS; size=0xF5
        Package[1]: type=STRINGS; size=0x40A
        Package[2]: type=STRINGS; size=0x40C
        Package[3]: type=DEVICE_PATH; size=0x1C
        Package[4]: type=END; size=0x4
```

`RAM_DISK_FORM_SET_GUID`
`gRamDiskFormSetGuid`
https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Guid/RamDiskHii.h
https://github.com/tianocore/edk2/blob/master/MdeModulePkg/MdeModulePkg.dec
```
PackageList[6]: GUID=2A46715F-3581-4A55-8E73-2B769AAA30C5; size=0x6B0
        Package[0]: type=FORMS; size=0x143
        Package[1]: type=STRINGS; size=0x539
        Package[2]: type=DEVICE_PATH; size=0x1C
        Package[3]: type=END; size=0x4
```

`gCustomizedDisplayLibGuid`
https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.c
```
PackageList[7]: GUID=99FDC8FD-849B-4EBA-AD13-FB9699C90A4D; size=0x6FE
        Package[0]: type=STRINGS; size=0x340
        Package[1]: type=STRINGS; size=0x3A6
        Package[2]: type=END; size=0x4
```

`gDisplayEngineGuid`
https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/DisplayEngineDxe/FormDisplay.c
```
PackageList[8]: GUID=E38C1029-E38F-45B9-8F0D-E2E60BC9B262; size=0x15DA
        Package[0]: type=STRINGS; size=0xA88
        Package[1]: type=STRINGS; size=0xB3A
        Package[2]: type=END; size=0x4
```

https://github.com/tianocore/edk2/blob/master/OvmfPkg/PlatformDxe/Platform.inf
```
PackageList[9]: GUID=D9DCC5DF-4007-435E-9098-8970935504B2; size=0x855
        Package[0]: type=FORMS; size=0x1F6
        Package[1]: type=STRINGS; size=0x62B
        Package[2]: type=DEVICE_PATH; size=0x1C
        Package[3]: type=END; size=0x4
```

`mFontPackageListGuid`
https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsole.c
```
PackageList[10]: GUID=F5F219D3-7006-4648-AC8D-D61DFB7BC6AD; size=0x14EC
        Package[0]: type=SIMPLE_FONTS; size=0x14D4
        Package[1]: type=END; size=0x4
```

`gIScsiConfigGuid`
`ISCSI_CONFIG_GUID`
https://github.com/tianocore/edk2/blob/master/NetworkPkg/Include/Guid/IScsiConfigHii.h
https://github.com/tianocore/edk2/blob/master/NetworkPkg/NetworkPkg.dec
```
PackageList[11]: GUID=4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9; size=0x6AC8
        Package[0]: type=FORMS; size=0x1030
        Package[1]: type=STRINGS; size=0x3C99
        Package[2]: type=STRINGS; size=0x1DCB
        Package[3]: type=DEVICE_PATH; size=0x1C
        Package[4]: type=END; size=0x4
```

`gShellLevel2HiiGuid`
`SHELL_LEVEL2_HII_GUID`
https://github.com/tianocore/edk2/blob/master/ShellPkg/Include/Guid/ShellLibHiiGuid.h
https://github.com/tianocore/edk2/blob/master/ShellPkg/ShellPkg.dec
```
PackageList[12]: GUID=F95A7CCC-4C55-4426-A7B4-DC8961950BAE; size=0x13909
        Package[0]: type=STRINGS; size=0x138F1
        Package[1]: type=END; size=0x4
```

`SHELL_LEVEL1_HII_GUID`
`gShellLevel1HiiGuid`
https://github.com/tianocore/edk2/blob/master/ShellPkg/ShellPkg.dec
```
PackageList[13]: GUID=DEC5DAA4-6781-4820-9C63-A7B0E4F1DB31; size=0x8677
        Package[0]: type=STRINGS; size=0x865F
        Package[1]: type=END; size=0x4
```

`SHELL_LEVEL3_HII_GUID`
`gShellLevel3HiiGuid`
https://github.com/tianocore/edk2/blob/master/ShellPkg/ShellPkg.dec
```
PackageList[14]: GUID=4344558D-4EF9-4725-B1E4-3376E8D6974F; size=0x83BD
        Package[0]: type=STRINGS; size=0x83A5
        Package[1]: type=END; size=0x4
```

`SHELL_DRIVER1_HII_GUID`
`gShellDriver1HiiGuid`
https://github.com/tianocore/edk2/blob/master/ShellPkg/ShellPkg.dec
```
PackageList[15]: GUID=0AF0B742-63EC-45BD-8DB6-71AD7F2FE8E8; size=0xCB04
        Package[0]: type=STRINGS; size=0xCAEC
        Package[1]: type=END; size=0x4
```

`SHELL_DEBUG1_HII_GUID`
`gShellDebug1HiiGuid`
https://github.com/tianocore/edk2/blob/master/ShellPkg/ShellPkg.dec
```
PackageList[16]: GUID=25F200AA-D3CB-470A-BF51-E7D162D22E6F; size=0x1D3D7
        Package[0]: type=STRINGS; size=0x1D3BF
        Package[1]: type=END; size=0x4
```

`SHELL_BCFG_HII_GUID`
`gShellBcfgHiiGuid`
https://github.com/tianocore/edk2/blob/master/ShellPkg/ShellPkg.dec
```
PackageList[17]: GUID=5F5F605D-1583-4A2D-A6B2-EB12DAB4A2B6; size=0x3048
        Package[0]: type=STRINGS; size=0x3030
        Package[1]: type=END; size=0x4
```

`SHELL_NETWORK1_HII_GUID`
`gShellNetwork1HiiGuid`
https://github.com/tianocore/edk2/blob/master/ShellPkg/ShellPkg.dec
```
PackageList[18]: GUID=F3D301BB-F4A5-45A8-B0B7-FA999C6237AE; size=0x26B5
        Package[0]: type=STRINGS; size=0x269D
        Package[1]: type=END; size=0x4
```

`gUefiShellFileGuid`
https://github.com/tianocore/edk2/blob/master/ShellPkg/Application/Shell/Shell.inf
https://github.com/tianocore/edk2/blob/master/ShellPkg/ShellPkg.dec
```
PackageList[19]: GUID=7C04A583-9E3E-4F1C-AD65-E05268D0B4D1; size=0x5CB
        Package[0]: type=STRINGS; size=0x5B3
        Package[1]: type=END; size=0x4
```
