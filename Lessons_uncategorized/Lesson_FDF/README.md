When we build OVMF image the final result is a flash image.

Last messages in the build log provide some information about the image generation process:
```
$ build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5

<...>

Fd File Name:OVMF (/<...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd)

Generate Region at Offset 0x0
   Region Size = 0x40000
   Region Name = DATA

Generate Region at Offset 0x40000
   Region Size = 0x1000
   Region Name = None

Generate Region at Offset 0x41000
   Region Size = 0x1000
   Region Name = DATA

Generate Region at Offset 0x42000
   Region Size = 0x42000
   Region Name = None

Generate Region at Offset 0x84000
   Region Size = 0x348000
   Region Name = FV

Generating FVMAIN_COMPACT FV

Generating PEIFV FV
###
Generating DXEFV FV
########
Generate Region at Offset 0x3CC000
   Region Size = 0x34000
   Region Name = FV

Generating SECFV FV
#
Fd File Name:OVMF_VARS (/<...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd)

Generate Region at Offset 0x0
   Region Size = 0x40000
   Region Name = DATA

Generate Region at Offset 0x40000
   Region Size = 0x1000
   Region Name = None

Generate Region at Offset 0x41000
   Region Size = 0x1000
   Region Name = DATA

Generate Region at Offset 0x42000
   Region Size = 0x42000
   Region Name = None

Fd File Name:OVMF_CODE (/<...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd)

Generate Region at Offset 0x0
   Region Size = 0x348000
   Region Name = FV

Generate Region at Offset 0x348000
   Region Size = 0x34000
   Region Name = FV

Fd File Name:MEMFD (/<...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/MEMFD.fd)

Generate Region at Offset 0x0
   Region Size = 0x6000
   Region Name = None

Generate Region at Offset 0x6000
   Region Size = 0x1000
   Region Name = None

Generate Region at Offset 0x7000
   Region Size = 0x1000
   Region Name = None

Generate Region at Offset 0x8000
   Region Size = 0x1000
   Region Name = None

Generate Region at Offset 0x9000
   Region Size = 0x2000
   Region Name = None

Generate Region at Offset 0xB000
   Region Size = 0x1000
   Region Name = None

Generate Region at Offset 0xC000
   Region Size = 0x1000
   Region Name = None
Padding region starting from offset 0xD000, with size 0x3000

Generate Region at Offset 0xD000
   Region Size = 0x3000
   Region Name = None

Generate Region at Offset 0x10000
   Region Size = 0x10000
   Region Name = None

Generate Region at Offset 0x20000
   Region Size = 0xE0000
   Region Name = FV

Generate Region at Offset 0x100000
   Region Size = 0xC00000
   Region Name = FV

GUID cross reference file can be found at /<...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/Guid.xref

FV Space Information
SECFV [13%Full] 212992 (0x34000) total, 27904 (0x6d00) used, 185088 (0x2d300) free
PEIFV [11%Full] 917504 (0xe0000) total, 102824 (0x191a8) used, 814680 (0xc6e58) free
DXEFV [22%Full] 12582912 (0xc00000) total, 2879400 (0x2befa8) used, 9703512 (0x941058) free
FVMAIN_COMPACT [27%Full] 3440640 (0x348000) total, 935208 (0xe4528) used, 2505432 (0x263ad8) free

- Done -
```

This image process is initiated because the package DSC file `OvmfPkg/OvmfPkgX64.dsc` has `FLASH_DEFINITION` identifier defined:
```
[Defines]
  ...
  FLASH_DEFINITION               = OvmfPkg/OvmfPkgX64.fdf
```

The referenced FDF file should be formatted according to the [EDK II Flash Description (FDF) File Specification](https://edk2-docs.gitbook.io/edk-ii-fdf-specification/).
This file defines the flash images generated in the end of the build.

Each image is called `Flash Device Image` and is defined by the `[FD.<name>]` section.

For example `OvmfPkg/OvmfPkgX64.fdf` has 4 such sections:
```
[FD.OVMF]
[FD.OVMF_VARS]
[FD.OVMF_CODE]
[FD.MEMFD]
```
Each of these sections leads to the `Flash Device Image` generation:
```
$ find Build/OvmfX64/RELEASE_GCC5/FV/ -name "*.fd"
Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd
Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd
Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd
Build/OvmfX64/RELEASE_GCC5/FV/MEMFD.fd
```
You can see how they are generated in the log above:
```
$ build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5

<...>

Fd File Name:OVMF (/<...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd)

<...>

Fd File Name:OVMF_VARS (/<...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd)

<...>

Fd File Name:OVMF_CODE (/<...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd)

<...>

Fd File Name:MEMFD (/<...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/MEMFD.fd)

<...>

GUID cross reference file can be found at /<...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/Guid.xref

FV Space Information
SECFV [13%Full] 212992 (0x34000) total, 27904 (0x6d00) used, 185088 (0x2d300) free
PEIFV [11%Full] 917504 (0xe0000) total, 102824 (0x191a8) used, 814680 (0xc6e58) free
DXEFV [22%Full] 12582912 (0xc00000) total, 2879400 (0x2befa8) used, 9703512 (0x941058) free
FVMAIN_COMPACT [27%Full] 3440640 (0x348000) total, 935208 (0xe4528) used, 2505432 (0x263ad8) free

- Done -
```

## `Flash Device Image` flash description tokens

The `Flash Device Image` is intendend for usage on a specific flash device, therefore the following tokens are mandatory for each FD section:
```
BaseAddress   = <...>
Size          = <...>
ErasePolarity = <...>
```
- `BaseAddress` field defines an address at which flash image would be mapped to the CPU memory
- `Size` field defines the size of the flash image
- `ErasePolarity` field defines how to fill not used space in flash image (with 0 or 1)

Most often the following two tokens are also defined:
```
BlockSize = <...>
NumBlocks = <...>
```
These tokens define block structure of a flash chip. If they are present this rule should be satisfied:
```
BlockSize * NumBlocks = Size
```

Let's look at the values for these tokens in the `OvmfPkg/OvmfPkgX64.fdf` file. Most of the tokens there are defined via defines. For their definition we should look in the `[Defines]` section:
```
[Defines]
!include OvmfPkgDefines.fdf.inc
```
As you can see this section uses `!include` directive to abstract all defines in the separate file. If you look at this file, you'll see that it contains some `if...endif` logic depending on the value of `FD_SIZE_IN_KB` variable for various configurations. In our case `OvmfPkg/OvmfPkgX64.dsc` defines `FD_SIZE_IN_KB` as `4096`:
```
[Defines]
  ...
  !ifdef $(FD_SIZE_1MB)
    DEFINE FD_SIZE_IN_KB           = 1024
  !else
  !ifdef $(FD_SIZE_2MB)
    DEFINE FD_SIZE_IN_KB           = 2048
  !else
  !ifdef $(FD_SIZE_4MB)
    DEFINE FD_SIZE_IN_KB           = 4096
  !else
    DEFINE FD_SIZE_IN_KB           = 4096
  !endif
```
Therefore we use the following defines from the `OvmfPkg/OvmfPkgDefines.fdf.inc` file:
```
DEFINE BLOCK_SIZE        = 0x1000

!if $(FD_SIZE_IN_KB) == 4096
DEFINE VARS_SIZE         = 0x84000
DEFINE VARS_BLOCKS       = 0x84
DEFINE VARS_LIVE_SIZE    = 0x40000
DEFINE VARS_SPARE_SIZE   = 0x42000

DEFINE FW_BASE_ADDRESS   = 0xFFC00000
DEFINE FW_SIZE           = 0x00400000
DEFINE FW_BLOCKS         = 0x400
DEFINE CODE_BASE_ADDRESS = 0xFFC84000
DEFINE CODE_SIZE         = 0x0037C000
DEFINE CODE_BLOCKS       = 0x37C
DEFINE FVMAIN_SIZE       = 0x00348000
DEFINE SECFV_OFFSET      = 0x003CC000
DEFINE SECFV_SIZE        = 0x34000
!endif

DEFINE MEMFD_BASE_ADDRESS = 0x800000
```

Now we can calculate the token values for each `Flash Device Image` in the `OvmfPkg/OvmfPkgX64.fdf`:
```
[FD.OVMF]
BaseAddress   = $(FW_BASE_ADDRESS)   # 0xFFC00000
Size          = $(FW_SIZE)           # 0x00400000
ErasePolarity = 1
BlockSize     = $(BLOCK_SIZE)        # 0x1000
NumBlocks     = $(FW_BLOCKS)         # 0x400

...

[FD.OVMF_VARS]
BaseAddress   = $(FW_BASE_ADDRESS)   # 0xFFC00000
Size          = $(VARS_SIZE)         # 0x84000
ErasePolarity = 1
BlockSize     = $(BLOCK_SIZE)        # 0x1000
NumBlocks     = $(VARS_BLOCKS)       # 0x84

...

[FD.OVMF_CODE]
BaseAddress   = $(CODE_BASE_ADDRESS) # 0xFFC84000
Size          = $(CODE_SIZE)         # 0x0037C000
ErasePolarity = 1
BlockSize     = $(BLOCK_SIZE)        # 0x1000
NumBlocks     = $(CODE_BLOCKS)       # 0x37C

...

[FD.MEMFD]
BaseAddress   = $(MEMFD_BASE_ADDRESS) # 0x800000
Size          = 0xD00000
ErasePolarity = 1
BlockSize     = 0x10000
NumBlocks     = 0xD0
```

Here you can see that `BlockSize * NumBlocks = Size` formula is correct for every FD.

And you can verify that size of the files matches the `Size` field values (although it is not mandatory as we will see next):
```
$ printf "%x\n" `stat -c "%s"  Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd`
400000
$ printf "%x\n" `stat -c "%s"  Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd`
37c000
$ printf "%x\n" `stat -c "%s"  Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd`
84000
$ printf "%x\n" `stat -c "%s"  Build/OvmfX64/RELEASE_GCC5/FV/MEMFD.fd`
d00000
```

Couple of words about the `OVMF*.fd` images itself. Remember what commands we've used to launch QEMU:
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
  ...

qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd \
  ...
```
If you look closely to the `OVMF_CODE.fd` and `OVMF_VARS.fd` you could notice that 
```
[FD.OVMF_VARS].BaseAddress + [FD.OVMF_VARS].Size = [FD.OVMF_CODE].BaseAddress
[FD.OVMF_CODE].BaseAddress + [FD.OVMF_CODE].Size = 0x100000000

[FD.OVMF].BaseAddress + [FD.OVMF].Size = 0x100000000

[FD.OVMF].BaseAddress == [FD.OVMF_VARS].BaseAddress
```
Simply speaking the `[FD.OVMF]` image is just the `[FD.OVMF_VARS]` image followed by the `[FD.OVMF_CODE]` image.

# Regions

`Flash Device Image` consists of regions. It is necessary to have at least one region in a `Flash Device Image`.

The most simple definition of region is:
```
Offset|Size
```
For example:
```
0x500|0x600
```
This is a region that starts at offset 0x500 with a size of 0x600. It is important to note that the final `Flash Device Image` size is defined by its regions and not by the `Size` token value. The none of the regions can have addresses above the `Size`.

If we want to, we can assign PCDs to the region offset/size values:
```
Offset|Size
TokenSpaceGuidCName.PcdOffsetCName | TokenSpaceGuidCName.PcdSizeCName
```
For example:
```
0x500|0x600
gEfiMyTokenSpaceGuid.PcdFlashRegionBaseAddress | gEfiMyTokenSpaceGuid.PcdFlashRegionSize
```
This way the build system will automatically override the PCDs with the provided values. Off course these PCDs `gEfiMyTokenSpaceGuid.PcdFlashRegionBaseAddress` and `gEfiMyTokenSpaceGuid.PcdFlashRegionSize` must be defined in the DEC file.
They can be of types `PcdsFixedAtBuild` or `PcdsPatchableInModule`, but not dynamic!

Another thing that we would want to add to our region definition is a region type:
```
Offset|Size
TokenSpaceGuidCName.PcdOffsetCName | TokenSpaceGuidCName.PcdSizeCName
  RegionType
```

If `RegionType` is not present, it is considered `None` and edk2 doesn't touch this region data. It can be usefull, for example, if we define a space in flash reserved for logs.
If `RegionType` is present, it can be one of the following types: `FV`, `DATA`, `FILE`, `INF` or `CAPSULE`.

Here are some explanation for the most common ones: `FV`, `DATA`, `FILE`:

## FV region

The `FV` region type is a pointer to some `Firmware Volume`. `Firmware Volume` (`FV`) is a data block next level lower than `Flash Device Image` (`FD`).

Example:
```
0x000000|0x0C0000
FV = FVMAIN
```
In this case FDF must define the section `[FV.FVMAIN]`.

We will look at the `Firmware Volumes` in the next lesson.

## DATA region

In the case of a `DATA` region we immediately define the data inside the region via initialized array:
```
0x0|0x50
DATA = {
  0xDE, 0xAD, 0xBE, 0xEF
}
```
This would give us a region with such data:
```
00000000  de ad be ef ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
00000010  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00000050
```

If you don't want to pollute FDF file, you can create some file and put data text into it:
```
0x0|0x50
DATA = {
  !include mydata.txt
}
```
```
$ cat mydata.txt
0xDE, 0xAD, 0xBE, 0xEF
```

## FILE region

FILE region is a pointer to a binary file that will be loaded into the flash device.

For example create a simple file with text:
```
$ cat "hello!" > hello.txt
```
And define this region:
```
0x0|0x50
FILE = hello.txt
```
In the final flash image it would look like this:
```
00000000  68 65 6c 6c 6f 21 0a ff  ff ff ff ff ff ff ff ff  |hello!..........|
00000010  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00000050
```

# OVMF `Flash Device Image` regions

Let's look closely to the OVMF `Flash Device Image` regions.

Start with `[FD.OVMF_VARS]` Flash Device Image:
```
[FD.OVMF_VARS]
...
!include VarStore.fdf.inc
```
As you can see all of its regions are defined in the `OvmfPkg/VarStore.fdf.inc` file.

If we remember that `FD_SIZE_IN_KB` is equal to `4096` in our case we can simplify `OvmfPkg/VarStore.fdf.inc` content to this:
```
0x00000000|0x00040000 # NV_VARIABLE_STORE
DATA = {
  ...
}
0x00040000|0x00001000 # NV_EVENT_LOG
0x00041000|0x00001000 # NV_FTW_WORKING
DATA = {
  ...
}
0x00042000|0x00042000 # NV_FTW_SPARE
```

Here we have two empty regions, and two regions initialized with DATA arrays.

Now we can see how FDF content corresponds to build log output:
```
Fd File Name:OVMF_VARS (/<...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd)

Generate Region at Offset 0x0
   Region Size = 0x40000
   Region Name = DATA

Generate Region at Offset 0x40000
   Region Size = 0x1000
   Region Name = None

Generate Region at Offset 0x41000
   Region Size = 0x1000
   Region Name = DATA

Generate Region at Offset 0x42000
   Region Size = 0x42000
   Region Name = None
```

The `[FD.OVMF_CODE]` flash device image consists of two Firmware Volumes (for the defines definition look at the `OvmfPkg/OvmfPkgDefines.fdf.inc` content):
```
[FD.OVMF_CODE]
...
0x00000000|$(FVMAIN_SIZE)        # 0x00000000|0x00348000
FV = FVMAIN_COMPACT

$(FVMAIN_SIZE)|$(SECFV_SIZE)     # 0x00348000|0x34000
FV = SECFV
```
And here is example of a build log output for this FD from the start of the lesson:
```
Fd File Name:OVMF_CODE (/<...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd)

Generate Region at Offset 0x0
   Region Size = 0x348000
   Region Name = FV

Generate Region at Offset 0x348000
   Region Size = 0x34000
   Region Name = FV
```

Finally the `[FD.OVMF]` content:
```
[FD.OVMF]
...
!include VarStore.fdf.inc

$(VARS_SIZE)|$(FVMAIN_SIZE)
FV = FVMAIN_COMPACT

$(SECFV_OFFSET)|$(SECFV_SIZE)
FV = SECFV
```
Which means:
```
0x00000000|0x00040000 # NV_VARIABLE_STORE
DATA = {
  ...
}
0x00040000|0x00001000 # NV_EVENT_LOG
0x00041000|0x00001000 # NV_FTW_WORKING
DATA = {
  ...
}
0x00042000|0x00042000 # NV_FTW_SPARE

$(VARS_SIZE)|$(FVMAIN_SIZE)      # 0x84000 | 0x00348000
FV = FVMAIN_COMPACT

$(SECFV_OFFSET)|$(SECFV_SIZE)    # 0x003CC000 | 0x34000
FV = SECFV
```
Compare it with the build log for this FD:
```
Fd File Name:OVMF (/<...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd)

Generate Region at Offset 0x0
   Region Size = 0x40000
   Region Name = DATA

Generate Region at Offset 0x40000
   Region Size = 0x1000
   Region Name = None

Generate Region at Offset 0x41000
   Region Size = 0x1000
   Region Name = DATA

Generate Region at Offset 0x42000
   Region Size = 0x42000
   Region Name = None

Generate Region at Offset 0x84000
   Region Size = 0x348000
   Region Name = FV

...

Generate Region at Offset 0x3CC000
   Region Size = 0x34000
   Region Name = FV
```

# Creating `UefiLessonsPkg.fdf`

If you want to, you can experiment with FDF in our `UefiLessonsPkg` package.

For this add `FLASH_DEFINITION` define to the `UefiLessonsPkg/UefiLessonsPkg.dsc` file:
```
[Defines]
  ...
  FLASH_DEFINITION               = UefiLessonsPkg/UefiLessonsPkg.fdf

...
```
And create file `UefiLessonsPkg/UefiLessonsPkg.fdf`. For example fill it with this content:
```
[FD.SimpleImage]
BaseAddress   = 0x0
Size          = 0x1000
ErasePolarity = 1

0x00|0x50

```
Important notice: keep in mind that the FDF file must end with an empty string. In other case EDKII build would fail!

The FDF file above would produce the following FD as a build result:
```
Fd File Name:SIMPLEIMAGE (/<...>/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEIMAGE.fd)

Generate Region at Offset 0x0
   Region Size = 0x50
   Region Name = None
```
You can look at its content with a help of a `hexdump` utility:
```
$ hexdump /<...>/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEIMAGE.fd
0000000 ffff ffff ffff ffff ffff ffff ffff ffff
*
0000050
```

# Padding regions

If regions would start not from the 0x0, the build system will automatically create a padding region. For example this config:
```
[FD.SimpleImage]
BaseAddress   = 0x0
Size          = 0x1000
ErasePolarity = 1

0x60|0x300
```
Would produce the following log:
```
Fd File Name:SIMPLEIMAGE (/<...>/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEIMAGE.fd)
Padding region starting from offset 0x0, with size 0x60

Generate Region at Offset 0x0
   Region Size = 0x60
   Region Name = None

Generate Region at Offset 0x60
   Region Size = 0x300
   Region Name = None
```
And hexdump output would look like this:
```
$ hexdump /<...>/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEIMAGE.fd
0000000 ffff ffff ffff ffff ffff ffff ffff ffff
*
0000360
```
The padding region is also created if there is an unused space between two regions. For example this config:
```
[FD.SimpleImage]
BaseAddress   = 0x0
Size          = 0x1000
ErasePolarity = 1
BlockSize = 0x100
NumBlocks = 0x10

0x0|0x50

0x80|0x40
```
would produce the following build log output:
```
Fd File Name:SIMPLEIMAGE (/<...>/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEIMAGE.fd)

Generate Region at Offset 0x0
   Region Size = 0x50
   Region Name = None
Padding region starting from offset 0x50, with size 0x30

Generate Region at Offset 0x50
   Region Size = 0x30
   Region Name = None

Generate Region at Offset 0x80
   Region Size = 0x40
   Region Name = None
```
And hexdump output would look like this:
```
$ hexdump /<...>/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEIMAGE.fd -C
00000000  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
000000c0
```

## Define PCDs from region offset and size

You can define PCDs in the DEC file `UefiLessonsPkg/UefiLessonsPkg.dec`:
```
[PcdsFixedAtBuild]
  ...
  gUefiLessonsPkgTokenSpaceGuid.Region1Offset|0|UINT32|0x00000005
  gUefiLessonsPkgTokenSpaceGuid.Region1Size|0|UINT32|0x00000006
```
And initialize them in FDF:
```
[FD.SimpleImage]
BaseAddress   = 0x0
Size          = 0x1000
ErasePolarity = 1

0x0|0x50
gUefiLessonsPkgTokenSpaceGuid.Region1Offset | gUefiLessonsPkgTokenSpaceGuid.Region1Size
```

For example if we add these PCDs to the `UefiLessonsPkg/PCDLesson/PCDLesson.inf` file:
```
[FixedPcd]
  ...
  gUefiLessonsPkgTokenSpaceGuid.Region1Offset
  gUefiLessonsPkgTokenSpaceGuid.Region1Size
 ```

The following defines would be generated in the `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h` file ready for the usage in your code:
```cpp
#define _PCD_TOKEN_Region1Offset  0U
#define _PCD_SIZE_Region1Offset 4
#define _PCD_GET_MODE_SIZE_Region1Offset  _PCD_SIZE_Region1Offset
#define _PCD_VALUE_Region1Offset  0x00000000U                               <------------
extern const  UINT32  _gPcd_FixedAtBuild_Region1Offset;
#define _PCD_GET_MODE_32_Region1Offset  _gPcd_FixedAtBuild_Region1Offset
//#define _PCD_SET_MODE_32_Region1Offset  ASSERT(FALSE)  // It is not allowed to set value for a FIXED_AT_BUILD PCD

#define _PCD_TOKEN_Region1Size  0U
#define _PCD_SIZE_Region1Size 4
#define _PCD_GET_MODE_SIZE_Region1Size  _PCD_SIZE_Region1Size
#define _PCD_VALUE_Region1Size  0x00000050U                                 <------------
extern const  UINT32  _gPcd_FixedAtBuild_Region1Size;
#define _PCD_GET_MODE_32_Region1Size  _gPcd_FixedAtBuild_Region1Size
//#define _PCD_SET_MODE_32_Region1Size  ASSERT(FALSE)  // It is not allowed to set value for a FIXED_AT_BUILD PCD
```

## Define DATA region

As a final example let's declare the region as DATA and initialize it:
```
[FD.SimpleImage]
BaseAddress   = 0x0
Size          = 0x1000
ErasePolarity = 1

0x0|0x50
DATA = {
  0xDE, 0xAD, 0xBE, 0xEF
}
```

This would give us the following content in the final image:
```
$ hexdump /<...>/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEIMAGE.fd -C
00000000  de ad be ef ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
00000010  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00000050
```
