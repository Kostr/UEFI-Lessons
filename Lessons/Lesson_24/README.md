All the PCD types that we've investigated so far are local to the module. `PcdsFixedAtBuild` and `PcdsFeatureFlag` are simple defines and `PcdsPatchableInModule` is just a local variable.
It is not possible to pass information BETWEEN modules with these PCDs. I'm talking about situations where one module updates PCD and another module reads its value. It would be very usefull to have such functionality in a complete UEFI firmware image.
Fortunately there is another class of PCDs that can help to solve this issue. This class is called `PcdsDynamic`.
Let's think for a minute. Where would we store these PCD values? They belong to the platform (all modules), but not one particular module. Thus we need some common database in a platfrom with a known interface, that all the modules can access. Also we need to construct initial values for this database by reading all the dynamic PCD settings in the DEC/INF/DSC files.

The common PCD database modules are presented here [https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Universal/PCD](https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Universal/PCD). 
As we need to have an access to the PCD database both in PEI and DXE stages there are two of them: one for PEI stage [https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Universal/PCD/Pei](https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Universal/PCD/Pei) and one for the DXE stage [https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Universal/PCD/Dxe](https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Universal/PCD/Dxe).

To get the functionality of the dynamic PCDs platfrom intergator needs to include these modules in the final image and load them at the very start of their stages. As usually we would use OVMF platfrom in our experiments, and it has these modules included.

Now the fun part - let's look at the PCD database files, that are created as a result of parsing of all DEC/INF/DSC in a platfrom.

You can find these files under the paths:
- `Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw`
- `Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw`

They are created because the modules have these defines in their INF files:
[https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Pei/Pcd.inf](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Pei/Pcd.inf)
```
[Defines]
  ...
  PCD_IS_DRIVER                  = PEI_PCD_DRIVER
```
[https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Pcd.inf](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Pcd.inf)
```
[Defines]
  ...
  PCD_IS_DRIVER                  = DXE_PCD_DRIVER
```

The PEI PCD database file includes PCDs that are used only in PEI modules or both in PEI and DXE modules. The DXE PCD database file includes PCDs that are used only in DXE modules. But the PEI database module can access only its own database, and the DXE database module is able to access both databases. This way we have only the necessary PCDs in the PEI stage and all of them in the DXE stage without any duplication. Keeping only the necessary PCDs in the PEI stage is done for the optimization. As in the PEI stage we need to reduce memory footprint as much as possible.

# [PEI|DXE]PcdDataBase.raw format

You can look at the content of these files with a help of a `hexdump` utility:
```
$ hexdump Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw -C
00000000  3c 19 7d 3c 2c 68 14 4c  a6 8f 55 2d ea 4f 43 7e  |<.}<,h.L..U-.OC~|
00000010  07 00 00 00 f8 00 00 00  00 00 00 00 00 00 00 00  |................|
00000020  f8 00 00 00 60 00 00 00  84 00 00 00 6c 00 00 00  |....`.......l...|
00000030  dc 00 00 00 f0 00 00 00  f2 00 00 00 50 00 00 00  |............P...|
00000040  00 00 00 00 16 00 03 00  01 00 da da da da da da  |................|
00000050  01 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000060  00 00 00 00 08 00 00 00  40 00 00 00 05 00 03 00  |........@.......|
00000070  14 00 00 00 04 00 03 00  15 00 00 00 06 00 03 00  |................|
00000080  16 00 00 00 54 01 10 01  f8 00 00 08 00 01 00 08  |....T...........|
00000090  08 01 00 08 10 01 00 08  55 01 10 01 48 01 00 04  |........U...H...|
000000a0  4c 01 00 04 68 00 00 04  56 01 10 01 50 01 00 02  |L...h...V...P...|
000000b0  18 01 00 08 20 01 00 08  28 01 00 08 30 01 00 08  |.... ...(...0...|
000000c0  38 01 00 08 60 00 00 08  57 01 10 01 f6 00 00 02  |8...`...W.......|
000000d0  ec 00 00 10 52 01 00 02  40 01 00 08 49 f0 af a1  |....R...@...I...|
000000e0  eb fd 2a 44 b3 20 13 ab  4c b7 2b bc 00 00 00 00  |..*D. ..L.+.....|
000000f0  00 00 01 00 01 00 08 00                           |........|
000000f8

$ hexdump Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw -C
00000000  3c 19 7d 3c 2c 68 14 4c  a6 8f 55 2d ea 4f 43 7e  |<.}<,h.L..U-.OC~|
00000010  07 00 00 00 e0 00 00 00  00 00 00 00 00 00 00 00  |................|
00000020  e0 00 00 00 2e 00 00 00  78 00 00 00 70 00 00 00  |........x...p...|
00000030  c0 00 00 00 d0 00 00 00  d1 00 00 00 50 00 00 00  |............P...|
00000040  00 00 00 00 12 00 00 00  01 00 da da da da da da  |................|
00000050  01 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000060  80 02 00 00 e0 01 00 00  20 03 00 00 58 02 00 00  |........ ...X...|
00000070  00 00 00 00 00 00 00 00  00 01 00 04 04 01 00 04  |................|
00000080  e0 00 00 08 e8 00 00 08  f0 00 00 08 60 00 00 04  |............`...|
00000090  64 00 00 04 0a 01 00 01  d5 00 00 02 0b 01 10 01  |d...............|
000000a0  68 00 00 04 6c 00 00 04  08 01 00 02 d7 00 00 01  |h...l...........|
000000b0  d8 00 00 01 f8 00 00 08  0c 01 10 01 0d 01 10 01  |................|
000000c0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000000d0  00 00 00 00 00 08 02 01  01 da da da da da da da  |................|
000000e0
```

These databases start with a header struct which is defined in the [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Guid/PcdDataBaseSignatureGuid.h](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Guid/PcdDataBaseSignatureGuid.h)
```cpp
typedef struct {
  GUID            Signature;                    // PcdDataBaseGuid.
  UINT32          BuildVersion;
  UINT32          Length;                       // Length of DEFAULT SKU PCD DB
  SKU_ID          SystemSkuId;                  // Current SkuId value.
  UINT32          LengthForAllSkus;             // Length of all SKU PCD DB
  UINT32          UninitDataBaseSize;           // Total size for PCD those default value with 0.
  TABLE_OFFSET    LocalTokenNumberTableOffset;
  TABLE_OFFSET    ExMapTableOffset;
  TABLE_OFFSET    GuidTableOffset;
  TABLE_OFFSET    StringTableOffset;
  TABLE_OFFSET    SizeTableOffset;
  TABLE_OFFSET    SkuIdTableOffset;
  TABLE_OFFSET    PcdNameTableOffset;
  UINT16          LocalTokenCount;              // LOCAL_TOKEN_NUMBER for all.
  UINT16          ExTokenCount;                 // EX_TOKEN_NUMBER for DynamicEx.
  UINT16          GuidTableCount;               // The Number of Guid in GuidTable.
  UINT8           Pad[6];                       // Pad bytes to satisfy the alignment.
} PCD_DATABASE_INIT;
```

Let's investigate fields of this structure:

## Signature
```
GUID            Signature;
```
The first field is GUID, and it gets assigned to the:
```cpp
#define PCD_DATA_BASE_SIGNATURE_GUID \
{ 0x3c7d193c, 0x682c, 0x4c14, { 0xa6, 0x8f, 0x55, 0x2d, 0xea, 0x4f, 0x43, 0x7e } }
```
If you look at the `hexdump`outputs, you'll see how they start with this value. The PCD database code would check this value to verify that the data is indeed the PCD database file.

## Build version
```
UINT32          BuildVersion;
```
The format of the PCD database can change through edk2 development. If it changes, we need to increment `BuildVersion` value. The current version is 7. Off course the PEI PCD DB module, DXE PCD DB module and the build tools all need to be synchronized at this part.
The DB module code verifies that it is true:

[https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Pei/Service.h](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Pei/Service.h)
```cpp
/
// Please make sure the PCD Serivce PEIM Version is consistent with
// the version of the generated PEIM PCD Database by build tool.
//
#define PCD_SERVICE_PEIM_VERSION  7

//
// PCD_PEI_SERVICE_DRIVER_VERSION is defined in Autogen.h.
//
#if (PCD_SERVICE_PEIM_VERSION != PCD_PEI_SERVICE_DRIVER_VERSION)
  #error "Please make sure the version of PCD PEIM Service and the generated PCD PEI Database match."
#endif
```
[https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Service.h](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Service.h)
```cpp
//
// Please make sure the PCD Serivce DXE Version is consistent with
// the version of the generated DXE PCD Database by build tool.
//
#define PCD_SERVICE_DXE_VERSION  7

//
// PCD_DXE_SERVICE_DRIVER_VERSION is defined in Autogen.h.
//
#if (PCD_SERVICE_DXE_VERSION != PCD_DXE_SERVICE_DRIVER_VERSION)
  #error "Please make sure the version of PCD DXE Service and the generated PCD DXE Database match."
#endif
```

For example of a version change look at this commit [https://github.com/tianocore/edk2/commit/7c73626513238176bdd16dca14fcf3f9e10bcc81](https://github.com/tianocore/edk2/commit/7c73626513238176bdd16dca14fcf3f9e10bcc81) that have updated version from 6 to 7.

# Length
```
UINT32          Length;
UINT32          UninitDataBaseSize;
```
Some PCDs in the end of edk2 build calculation would be equal to 0. There can be a lot of such PCDs and it is not optimal to store them all in a flash image. Therefore in the `*.raw`image we would store only the non-zero PCDs. And `Length` signifies the size of this image:
```
|.........| 0
|.........|
|.........|
___________ Length
```
But when we are actually working with PCDs in PEI/DXE DB modules, we need for all of the PCDs to actually be present in memory, as we can assign values to them. And with a help of `UninitDataBaseSize` field we know how much space do we need to reserve for this:
```
|.........| 0
|.........|
|.........|
___________ Length
|000000000|
|000000000|
|000000000|
___________ Length + UninitDataBaseSize
```

## LocalTokenNumberTable
```
TABLE_OFFSET    LocalTokenNumberTableOffset;
...
UINT16          LocalTokenCount;
```
This is the most important part of the table. `TABLE_OFFSET` is just a `UINT32` value - offset to the LocalToken table which has `LocalTokenCount` records. Each record (=LocalToken) is a `UINT32` value. It can be represented like this:
```
union {
  UINT32 LocalToken;
  struct {
    UINT32 pcd_type : 4;
    UINT32 pcd_datum_type : 8;
    UINT32 offset : 20;
  }
}
```
-`pcd_type` - type of the PCD. Yes, there can be multiple classes of PCD, we'll get to that later. The PCD can be `DATA`, `HII` or `VPD`
-`pcd_datum_type` - type of the data in the PCD (UINT8, UINT16, UINT32, UINT64, VOID\*)
-`offset` - offset to the initial data of this PCD in the Database image

The Local tokens are implicitly numbered from 1. And when some firmware module wants to work with a PCD it communicates with a database saying "Hey, I want to get/set PCD at the local token №xxx". The PCD database reads this local token and then interprets actual data at the `offset` field depending on the values `pcd_type` and `pcd_datum_type`. This is how `[PcdsDynamic]` work.

It is important to note that the numbering is sequential for PEI and DXE databases. That means that local tokens in the PEI database are assumed to have numbers `1 .. (PEI_DB.LocalTokenCount)` and local tokens in the DXE database are assumed to have numbers `(PEI_DB.LocalTokenCount + 1) .. (PEI_DB.LocalTokenCount + 1 + DXE_DB.LocalTokenCount)`.

## `ExMapTable` and `GuidTable`. The necessity of `[PcdsDynamic]` and `[PcdsDynamicEx]`
```
TABLE_OFFSET    ExMapTableOffset;
TABLE_OFFSET    GuidTableOffset;
UINT16          ExTokenCount;
UINT16          GuidTableCount;
```

It is easy to reference PCDs by the local token number. But for that we actually need to know the mapping `PCD<-->LocalToken`. And we do, when we build our module and PCD database at the same time as parts of the flash image (OVMF in our case).
But sometimes flash image is already built and we are writing a separate module that needs to use some of the image PCDs. For this case EDKII offers us `[PcdsDynamicEx]` mechanics. In this case we reference dynamic PCDs by a combination `GUID+ExTokenNumber`. `ExTokenNumber` in this case is just a `UINT32` and both values must be unique for every PCD.
When PCD database receives request for `GUID+ExTokenNumber` it first verifies if the GUID is present in the database. There are totally `GuidTableCount` guids in the database and they are placed starting form the `GuidTableOffset` in the database file. If the requested GUID is equal to some GUID in the database we remember the index of this GUID.
After that the code looks at the records in the `ExMapTable` which is present under `ExMapTableOffset`. There are totally `ExTokenCount` records and each record has the format:
```cpp
typedef struct  {
  UINT32    ExTokenNumber;
  UINT16    TokenNumber;        // local token index in the LocalTokenNumberTable
  UINT16    ExGuidIndex;        // GUID index in the GuidTable
} DYNAMICEX_MAPPING;
```
If some record has the requested `ExTokenNumber` and `ExGuidIndex` that correspond to the requested `GUID`, we've found our LocalToken number. The rest goes exactly like in the `[PcdsDynamic]` case. And as you can deduct `ExTokenCount` always less or equal than `LocalTokenCount`.

## `StringTable` and `SizeTable`
```
TABLE_OFFSET    StringTableOffset;
TABLE_OFFSET    SizeTableOffset;
```
When the Database code needs to return a PCD value, beside the offset it needs to know the PCD size. For the simple data types the size fixed. For example if the PCD has a `pcd_datum_type` type `UINT64` the size of the PCD data is 8 bytes. But PCD data can be a structure or a string. In this cases `offset` in the Local token interpreted as an index to the StringTable with is present under the `StringTableOffset`. This table consists of `UINT32` values - actual offsets to the data. Size of the data in this case is written in the `SizeTable` which is present under the `SizeTableOffset` and consists of an elements like this:
```
struct {
  UINT16 MaxSize;
  UINT16 CurrentSize;
}
```
Once again the numbering is implicit. For our target local token we need to inspect `LocalTokenNumberTable` from the start and calculate how many String/Pointer PCDs were before our target local token, and this value would be an index of the `SizeTable` record.

# AutoGen files

Based on the previous section you can see that it is not easy to parse database content. Fortunately EDKII provide some help for us in the AutoGen files.
For example this content is written in the PEI PCD DB AutoGen files:

`Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/DEBUG/AutoGen.h`:
```
//
// External PCD database debug information
//
#if 0
#define PEI_GUID_TABLE_SIZE                1U
#define PEI_STRING_TABLE_SIZE              2U
#define PEI_SKUID_TABLE_SIZE               1U
#define PEI_LOCAL_TOKEN_NUMBER_TABLE_SIZE  22
#define PEI_LOCAL_TOKEN_NUMBER             22
#define PEI_EXMAPPING_TABLE_SIZE           3U
#define PEI_EX_TOKEN_NUMBER                3U
#define PEI_SIZE_TABLE_SIZE                2U
#define PEI_GUID_TABLE_EMPTY               FALSE
#define PEI_STRING_TABLE_EMPTY             FALSE
#define PEI_SKUID_TABLE_EMPTY              TRUE
#define PEI_DATABASE_EMPTY                 FALSE
#define PEI_EXMAP_TABLE_EMPTY              FALSE

typedef struct {
  ...
} PEI_PCD_DATABASE_INIT;

typedef struct {
  ...
} PEI_PCD_DATABASE_UNINIT;

typedef struct {
  PEI_PCD_DATABASE_INIT    Init;
  PEI_PCD_DATABASE_UNINIT  Uninit;
} PEI_PCD_DATABASE;
#endif
```
`Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/DEBUG/AutoGen.c`:
```
//
// External PCD database debug information
//
#if 0
PEI_PCD_DATABASE_INIT gPEIPcdDbInit = {
  ...
};
#endif
```
You can find the content of your PCDs with comments inside the `PEI_PCD_DATABASE_INIT`/`PEI_PCD_DATABASE_UNINIT` structures. As you can see all the code is not included in the build (in is under the `#if 0 .. #endif`), so all of these is just a help message for the developer.

The similar comment you can find in the DXE PCD DB AutoGen files:
- `Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/DEBUG/AutoGen.h`
- `Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/DEBUG/AutoGen.c`

# PCD_TABLE_parser

Information in `AutoGen` files is fine, but sometimes it is kinda hard to search it. For this case I've created a `PCD_TABLE_parser` utility.

Clone it and build:
```
$ git clone https://github.com/Kostr/PCD_TABLE_parser.git ~/PCD_TABLE_parser
$ cd ~/PCD_TABLE_parser
$ make
cc    -c -o main.o main.c
cc    -c -o guids.o guids.c
cc    -c -o utils.o utils.c
gcc main.o guids.o utils.o -o parse_pcd_db
```

Look at the help message:
```
$ ./parse_pcd_db --help
Usage: parse_pcd_db [--peidb <PEI_PCD_DB.raw>] [--dxedb <DXE_PCD_DB.raw>] [--vpd <VPD.bin>]
Program to parse PCD Database raw files

--peidb <PEI_PCD_DB.raw>     - provide PEI PCD database
--dxedb <DXE_PCD_DB.raw>     - provide DXE PCD database
--vpd   <VPD.bin>            - provide VPD binary

Example:
parse_pcd_db \
 --peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
 --dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw" \
 --vpd   "Build/OvmfX64/RELEASE_GCC5/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08.bin"
```

Don't mind the `--vpd` option right now. Let's launch the program giving it just `--peidb` and `--dxedb` arguments. Also you would probably want to pipe the output to `less`.
```
$ parse_pcd_db \
 --peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
 --dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw" | less
```

Here is an example output. I think it is more illustrative to the PCD table description that we've just covered.
Some comments for the output:
- you can see the sequential numbering between PEI and DXE databases
- the `0 - unitialized` means the PCD is in the `Length .. (Length + UninitDataBaseSize)` area
- only some PCDs have DynamicEx Token/GUID information - these are PCDs that are declared via `PcdsDynamicEx` method
- the PCDs of the pointer type has Size info
```
PEI PCD DB
LocalTokenNumberTable:

1:
Token type = Data
Datum type = UINT8 (Bool)
0 - unitialized

2:
Token type = Data
Datum type = UINT64
0 - unitialized

3:
Token type = Data
Datum type = UINT64
0 - unitialized

4:
Token type = Data
Datum type = UINT64
0 - unitialized

5:
Token type = Data
Datum type = UINT64
0 - unitialized

6:
Token type = Data
Datum type = UINT8 (Bool)
0 - unitialized

7:
Token type = Data
Datum type = UINT32
0 - unitialized

8:
Token type = Data
Datum type = UINT32
0 - unitialized

9:
Token type = Data
Datum type = UINT32
Value:
0x00000040 (=64)

10:
Token type = Data
Datum type = UINT8 (Bool)
0 - unitialized

11:
Token type = Data
Datum type = UINT16
0 - unitialized

12:
Token type = Data
Datum type = UINT64
0 - unitialized

13:
Token type = Data
Datum type = UINT64
0 - unitialized

14:
Token type = Data
Datum type = UINT64
0 - unitialized

15:
Token type = Data
Datum type = UINT64
0 - unitialized

16:
Token type = Data
Datum type = UINT64
0 - unitialized

17:
Token type = Data
Datum type = UINT64
Value:
0x0000000800000000 (=34359738368)

18:
Token type = Data
Datum type = UINT8 (Bool)
0 - unitialized

19:
Token type = Data
Datum type = UINT16
Value:
0x0008 (=8)

20:
Token type = String
Datum type = Pointer
DynamicEx Token = 0x00030005
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
CurrentSize = 1
MaxSize     = 1
Value:
00                                               | .

21:
Token type = Data
Datum type = UINT16
DynamicEx Token = 0x00030004
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

22:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00030006
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized
_____

DXE PCD DB
LocalTokenNumberTable:

23:
Token type = Data
Datum type = UINT32
0 - unitialized

24:
Token type = Data
Datum type = UINT32
0 - unitialized

25:
Token type = Data
Datum type = UINT64
0 - unitialized

26:
Token type = Data
Datum type = UINT64
0 - unitialized

27:
Token type = Data
Datum type = UINT64
0 - unitialized

28:
Token type = Data
Datum type = UINT32
Value:
0x00000280 (=640)

29:
Token type = Data
Datum type = UINT32
Value:
0x000001e0 (=480)

30:
Token type = Data
Datum type = UINT8
0 - unitialized

31:
Token type = Data
Datum type = UINT16
Value:
0x0208 (=520)

32:
Token type = Data
Datum type = UINT8 (Bool)
0 - unitialized

33:
Token type = Data
Datum type = UINT32
Value:
0x00000320 (=800)

34:
Token type = Data
Datum type = UINT32
Value:
0x00000258 (=600)

35:
Token type = Data
Datum type = UINT16
0 - unitialized

36:
Token type = Data
Datum type = UINT8
Value:
0x01 (=1)

37:
Token type = Data
Datum type = UINT8
Value:
0x01 (=1)

38:
Token type = Data
Datum type = UINT64
0 - unitialized

39:
Token type = Data
Datum type = UINT8 (Bool)
0 - unitialized

40:
Token type = Data
Datum type = UINT8 (Bool)
0 - unitialized
_____
```

# UEFITool

Let's imagine a situation where we want to look at the PCD DB content in the binary UEFI image.

[https://github.com/LongSoft/UEFITool](https://github.com/LongSoft/UEFITool). You can download and compile it from the source code, or just download precompiled image from the releases page [https://github.com/LongSoft/UEFITool/releases](https://github.com/LongSoft/UEFITool/releases).

When you load the program it looks like this:
![UEFITool](UEFITool.png?raw=true "UEFITool")

Let's open UEFI firmware file `Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd` via `File->Open image file...` menu option.
![UEFITool_1](UEFITool_1.png?raw=true "UEFITool_1")

Now you can investigate the `OVMF.fd` image content by unfolding/folding image sections. For this you need to click on the '>' symbols.

But right now we need to find PEI/DXE PCD databases in the image. As you might remember each database starts from the signature GUID:
```cpp
#define PCD_DATA_BASE_SIGNATURE_GUID \
{ 0x3c7d193c, 0x682c, 0x4c14, { 0xa6, 0x8f, 0x55, 0x2d, 0xea, 0x4f, 0x43, 0x7e } }
```
Therefore use `Action->Search...` menu option and input this value in the `GUID` tab:
![UEFITool_2](UEFITool_2.png?raw=true "UEFITool_2")

The program should find two `Raw` sections. If you click on the first result, program would show you where this section is placed in the image. You can see that it is placed under the `PcdPeim` module. This is the module name of the [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Pei/Pcd.inf](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Pei/Pcd.inf) file. At the `Information` panel on the right you could see that `Body size: F8h (248)`. This is the actual size of the section which is equal to the size the we saw with `hexdump`.

![UEFITool_3](UEFITool_3.png?raw=true "UEFITool_3")

You can save the section data if you right-click on the section name and select `Extract body...`. By default the program would suggest `Section_Raw_PcdPeim_PcdPeim_body.raw` name for the new file.
![UEFITool_4](UEFITool_4.png?raw=true "UEFITool_4")

Now you can check the second result of our search. This section is inside the `PcdDxe` module [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Pcd.inf](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Pcd.inf). The `Body size: E0h (224)` is once again corresponds to the `hexdump` output size. For the body of this section the program would suggest the `Section_Raw_PcdDxe_PcdDxe_body.raw` name.
![UEFITool_5](UEFITool_5.png?raw=true "UEFITool_5")

Now we have both databases, and we can parse them with our `parse_pcd_db` utility. Save the extracted files to the home folder and execute the program:
```
$ ./parse_pcd_db \
 --peidb ~/Section_Raw_PcdPeim_PcdPeim_body.raw \
 --dxedb ~/Section_Raw_PcdDxe_PcdDxe_body.raw
```

The output should be the same as before.

# Links

- PCD PEI DB documentation [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Pei/Pcd.inf](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Pei/Pcd.inf) 
- PCD DXE DB documentation [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Pcd.inf](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Pcd.inf)
