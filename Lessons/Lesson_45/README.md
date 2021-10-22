In the last lesson we've discovered that internally HII Database stores Package lists and its packages not in a continious data array, but in a complex data structure with many double linked lists.

But when we've used `ExportPackageLists` from the `EFI_HII_DATABASE_PROTOCOL`, we received continious data array of Package lists and its packages. It is a handy interface to hide/abstract inernals of the HII Database and provide data to the user in a form that is easy to parse.

The same goes when we want to add Package list to the database via `NewPackageList` from the `EFI_HII_DATABASE_PROTOCOL`. This functions expects incoming Package list in a continious data array in the same form.

```
EFI_HII_DATABASE_PROTOCOL.NewPackageList()

Summary:
Adds the packages in the package list to the HII database.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_HII_DATABASE_NEW_PACK) (
 IN CONST EFI_HII_DATABASE_PROTOCOL *This,
 IN CONST EFI_HII_PACKAGE_LIST_HEADER *PackageList,
 IN CONST EFI_HANDLE DriverHandle, OPTIONAL
 OUT EFI_HII_HANDLE *Handle
 );

Parameters:
This		A pointer to the EFI_HII_DATABASE_PROTOCOL instance
PackageList	A pointer to an EFI_HII_PACKAGE_LIST_HEADER structure
DriverHandle	Associate the package list with this EFI handle
Handle		A pointer to the EFI_HII_HANDLE instance
Description	This function adds the packages in the package list to the database and returns a handle. If there is a
		EFI_DEVICE_PATH_PROTOCOL associated with the DriverHandle, then this function will create a
		package of type EFI_PACKAGE_TYPE_DEVICE_PATH and add it to the package list.
```

Let's inspect one more time the output of our `ShowHII` application:
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

From this output you can see that each Package list contains one or more data packages and ends with a special `END` package.

Ordinary package contains `EFI_HII_PACKAGE_HEADER` and data content. But the `END` package is simply a `EFI_HII_PACKAGE_HEADER` with a `Type` field set to `EFI_HII_PACKAGE_END`.

Just in case here are prototypes for the header structures once again:
```
typedef struct {
 EFI_GUID PackageListGuid;
 UINT32 PackagLength;
} EFI_HII_PACKAGE_LIST_HEADER;

typedef struct {
 UINT32 Length:24;
 UINT32 Type:8;
 UINT8 Data[ … ];
} EFI_HII_PACKAGE_HEADER;
```

So basically package list data here looks something like this:

![Package_list](Package_list.png?raw=true "Package list")


Ordinary packages can be of different types. For the examples take a look at the possible defines for the `EFI_HII_PACKAGE_HEADER.type` field:
```
//
// Value of HII package type
//
#define EFI_HII_PACKAGE_TYPE_ALL             0x00
#define EFI_HII_PACKAGE_TYPE_GUID            0x01
#define EFI_HII_PACKAGE_FORMS                0x02
#define EFI_HII_PACKAGE_STRINGS              0x04
#define EFI_HII_PACKAGE_FONTS                0x05
#define EFI_HII_PACKAGE_IMAGES               0x06
#define EFI_HII_PACKAGE_SIMPLE_FONTS         0x07
#define EFI_HII_PACKAGE_DEVICE_PATH          0x08
#define EFI_HII_PACKAGE_KEYBOARD_LAYOUT      0x09
#define EFI_HII_PACKAGE_ANIMATIONS           0x0A
#define EFI_HII_PACKAGE_END                  0xDF
#define EFI_HII_PACKAGE_TYPE_SYSTEM_BEGIN    0xE0
#define EFI_HII_PACKAGE_TYPE_SYSTEM_END      0xFF
```

In the next lessons we would try to add a Package list with Strings packages.
```
EFI_HII_PACKAGE_HEADER.type = EFI_HII_PACKAGE_STRINGS
```

# Create app from template

Initialize new app from our template script:
```
./createNewApp.sh HIIStringsC
```

And add new app to the `UefiLessonsPkg/UefiLessonsPkg.dsc`:
```
[Components]
  ...
  UefiLessonsPkg/HIIStringsC/HIIStringsC.inf
```

# Package list GUID

As every Package list has its own GUID we need to create GUID and add it to our DEC file (`UefiLessonsPkg/UefiLessonsPkg.dec`):
```
[Guids]
  ...
  gHIIStringsCGuid = { 0x8e0b8ed3, 0x14f7, 0x499d, { 0xa2, 0x24, 0xae, 0xe8, 0x9d, 0xc9, 0x7f, 0xa3 }}
```
To reference it in our code we should declare it in the application INF file as well. Add this to our `UefiLessonsPkg/HIIStringsC/HIIStringsC.inf` file:
```
[Guids]
  gHIIStringsCGuid
```

For this GUID to be included we also need to add `UefiLessonsPkg/UefiLessonsPkg.dec` in the `Packages` section:
```
[Packages]
  ...
  UefiLessonsPkg/UefiLessonsPkg.dec
```

# UefiHiiServicesLib

As each of the HII protocols can have only one instance in the system, there is a library that abstracts all the `LocateProtocol` logic in its constructor and fills global variables for protocols (https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Library/UefiHiiServicesLib):
```
EFI_HII_STRING_PROTOCOL  		*gHiiString		// UEFI HII String Protocol
EFI_HII_DATABASE_PROTOCOL  		*gHiiDatabase		// UEFI HII Database Protocol
EFI_HII_CONFIG_ROUTING_PROTOCOL 	*gHiiConfigRouting	// UEFI HII Config Routing Protocol
EFI_HII_FONT_PROTOCOL 			*gHiiFont		// UEFI HII Font Protocol
EFI_HII_IMAGE_PROTOCOL  		*gHiiImage		// UEFI HII Image Protocol
```

So instead of using this in our last application:
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
We could simply include `UefiHiiServicesLib` to the app INF file and use `gHiiDatabase` instead as a `EFI_HII_DATABASE_PROTOCOL*`.

Let's use `UefiHiiServicesLib` in our current app. For this add `UefiHiiServicesLib` to the Library classes in the `UefiLessonsPkg/HIIStringsC/HIIStringsC.inf`:
```
[LibraryClasses]
  ...
  UefiHiiServicesLib
```
Also we need to include this library package DEC file in the application INF:
```
[Packages]
  ...
  MdeModulePkg/MdeModulePkg.dec
```

Finally add necessary include to our *.c file `UefiLessonsPkg/HIIStringsC/HIIStringsC.c`:
```
#include <Library/UefiHiiServicesLib.h>
```

# Application code

Here is a starting template for our application. We cheat a little bit here as we don't calculate size for our Package list, but use some number bigger that we would actually need in this example. This lesson is splitted in many parts and is hard enough as it is, I don't want to complicate things even more, so take my word on it that this size would be enough for the thing we are about to do:
```
  CHAR8* Data = (CHAR8*) AllocateZeroPool(200);          // CHEAT! NEEDS CORRECTION FOR YOUR OWN PACKAGES!
  UINT32 offset = 0;
  EFI_HII_PACKAGE_LIST_HEADER* PackageListHdr = (EFI_HII_PACKAGE_LIST_HEADER*)&Data[offset];
  PackageListHdr->PackageListGuid = gHIIStringsCGuid;
  offset += sizeof(EFI_HII_PACKAGE_LIST_HEADER);

  <...> 		// Fill String Packages in the memory starting from &Data[offset]
  offset += <...>	// Add packages size to the 'offset' variable

  EFI_HII_PACKAGE_HEADER* HIIEndPackageHdr = (EFI_HII_PACKAGE_HEADER*)&Data[offset];
  HIIEndPackageHdr->Type = EFI_HII_PACKAGE_END;
  HIIEndPackageHdr->Length = sizeof(EFI_HII_PACKAGE_HEADER);
  offset += sizeof(EFI_HII_PACKAGE_HEADER);

  PackageListHdr->PackageLength = offset;

  <...>			// Add new package to the HII Database

  FreePool(Data);
```

Off course don't forget to add include for using memory allocation functions:
```
#include <Library/MemoryAllocationLib.h>
```
