Now it is time to look at Firmware Volumes (`FV`) and how they are described in the FDF file.

`Firmware Volume` is one of the region types in the `Flash Device Image (FD)`. If you declare some region as FV, you must provide its name `<FVname>` and define a separate section `[FV.<FVname>]`:
```
[FD.<FDname>]
...

0xXXXX|0xYYYY
FV = <FVname>


[FV.<FVname>]
...
```

Firmware volume is a region with a special formatting which is defined by the `UEFI Platform Initialization (PI) specification (Volume 3: Shared Architectural Elements)`.

At the start of each FV is a special header `EFI_FIRMWARE_VOLUME_HEADER` ([https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Pi/PiFirmwareVolume.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Pi/PiFirmwareVolume.h)):
```
EFI_FIRMWARE_VOLUME_HEADER

Summary:
Describes the features and layout of the firmware volume.

Prototype:
typedef struct {
 UINT8 ZeroVector[16];
 EFI_GUID FileSystemGuid;
 UINT64 FvLength;
 UINT32 Signature;
 EFI_FVB_ATTRIBUTES_2 Attributes;
 UINT16 HeaderLength;
 UINT16 Checksum;
 UINT16 ExtHeaderOffset;
 UINT8 Reserved[1];
 UINT8 Revision;
 EFI_FV_BLOCK_MAP BlockMap[];
} EFI_FIRMWARE_VOLUME_HEADER

Parameters:
ZeroVector		The first 16 bytes are reserved to allow for the reset vector of processors whose reset vector is at address 0
FileSystemGuid		Declares the file system with which the firmware volume is formatted
FvLength		Length in bytes of the complete firmware volume, including the header
Signature		Set to {'_','F','V','H'}
Attributes		Declares capabilities and power-on defaults for the firmware volume
HeaderLength		Length in bytes of the complete firmware volume header
Checksum		A 16-bit checksum of the firmware volume header. A valid header sums to zero
ExtHeaderOffset		Offset, relative to the start of the header, of the extended header (EFI_FIRMWARE_VOLUME_EXT_HEADER) or zero if there is no extended header
Reserved		In this version of the specification, this field must always be set to zero
Revision		Set to 2. Future versions of this specification may define new header fields and will increment the Revision field accordingly
FvBlockMap[]		An array of run-length encoded FvBlockMapEntry structures. The array is terminated with an entry of {0,0}

FvBlockMapEntry.NumBlocks 	The number of blocks in the run.
FvBlockMapEntry.BlockLength 	The length of each block in the run

Description:
A firmware volume based on a block device begins with a header that describes the features and layout of the firmware volume. This header includes a description of the capabilities, state, and block map of the device.
```

The rest of the data in the FV region is organized via files in a filesystem. The filesystem in this case is called a `firmware file system (FFS)`. The FFS defines how files are stored in flash. The type of the FFS that is used is defined by the the header GUID field `EFI_FIRMWARE_VOLUME_HEADER.FileSystemGuid`.

Currently the UEFI Platform Initialization (PI) specification defines two filesystems:
```
#define EFI_FIRMWARE_FILE_SYSTEM2_GUID \
 { 0x8c8ce578, 0x8a3d, 0x4f1c, { 0x99, 0x35, 0x89, 0x61, 0x85, 0xc3, 0x2d, 0xd3 } }

#define EFI_FIRMWARE_FILE_SYSTEM3_GUID \
 { 0x5473c07a, 0x3dcb, 0x4dca, { 0xbd, 0x6f, 0x1e, 0x96, 0x89, 0xe7, 0x34, 0x9a } }
```

The main difference between them is that `EFI_FIRMWARE_FILE_SYSTEM3` supports files with a size `>16MB`. For the rest of the article we would describe `EFI_FIRMWARE_FILE_SYSTEM2`.

Each file in the filesystem would have a header `EFI_FFS_FILE_HEADER` ([https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Pi/PiFirmwareFile.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Pi/PiFirmwareFile.h)):
```
EFI_FFS_FILE_HEADER

Summary:
Each file begins with a header that describes the state and contents of the file. The header is 8-byte aligned with respect to the beginning of the firmware volume

Prototype:
typedef struct {
 EFI_GUID Name;
 EFI_FFS_INTEGRITY_CHECK IntegrityCheck;
 EFI_FV_FILETYPE Type;
 EFI_FFS_FILE_ATTRIBUTES Attributes;
 UINT8 Size[3];
 EFI_FFS_FILE_STATE State;
} EFI_FFS_FILE_HEADER;

Parameters:
Name		This GUID is the file name. It is used to uniquely identify the file. There may be only one instance of a file with the file name GUID of Name
		in any given firmware volume, except if the file type is EFI_FV_FILETYPE_FFS_PAD
IntegrityCheck	Used to verify the integrity of the file
Type		Identifies the type of file
Attributes	Declares various file attribute bits
Size		The length of the file in bytes, including the FFS header
State		Used to track the state of the file throughout the life of the file from creation to deletion
```

The data inside the file is formatted with respect to the `EFI_FFS_FILE_HEADER.Type` field. Specification defines these file types:

| Name | Value | Description |
| ---- | ----- | ----------- |
| EFI_FV_FILETYPE_RAW | 0x01 | Binary data |
| EFI_FV_FILETYPE_FREEFORM | 0x02 | Sectioned data |
| EFI_FV_FILETYPE_SECURITY_CORE | 0x03 | Platform core code used during the SEC phase |
| EFI_FV_FILETYPE_PEI_CORE | 0x04 | PEI Foundation |
| EFI_FV_FILETYPE_DXE_CORE | 0x05 | DXE Foundation |
| EFI_FV_FILETYPE_PEIM | 0x06 | PEI module (PEIM) |
| EFI_FV_FILETYPE_DRIVER | 0x07 | DXE driver |
| EFI_FV_FILETYPE_COMBINED_PEIM_DRIVER | 0x08 | Combined PEIM/DXE driver |
| EFI_FV_FILETYPE_APPLICATION | 0x09 | Application |
| EFI_FV_FILETYPE_MM | 0x0A | Contains a PE32+ image that will be loaded into MMRAM in MM Traditional Mode |
| EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE | 0x0B | Firmware volume image |
| EFI_FV_FILETYPE_COMBINED_MM_DXE | 0x0C | Contains PE32+ image that will be dispatched by the DXE Dispatcher and will also be loaded into MMRAM in MM Tradition Mode |
| EFI_FV_FILETYPE_MM_CORE | 0x0D | MM Foundation that support MM Traditional Mode |
| EFI_FV_FILETYPE_MM_STANDALONE | 0x0E | Contains a PE32+ image that will be loaded into MMRAM in MM Standalone Mode |
| EFI_FV_FILETYPE_MM_CORE_STANDALONE | 0x0F | MM Foundation that support MM Tradition Mode and MM Standalone Mode |
| EFI_FV_FILETYPE_OEM_MIN…EFI_FV_FILETYPE_OEM_MAX | 0xC0-0xDF | OEM File Types |
| EFI_FV_FILETYPE_DEBUG_MIN…EFI_FV_FILETYPE_DEBUG_MAX | 0xE0-0xEF | Debug/Test File Types |
| EFI_FV_FILETYPE_FFS_MIN…EFI_FV_FILETYPE_FFS_MAX | 0xF0-0xFF | Firmware File System Specific File Types |
| EFI_FV_FILETYPE_FFS_PAD | 0xF0 | Pad File For FFS |


# Create simple Firmware Volume

Now let's try to create the most simple firmware volume which would contain one binary file. Here is code for this structure (`UefiLessonsPkg/UefiLessonsPkg.fdf`):
```
[FD.SimpleImage]
BaseAddress   = 0x0
Size          = 0x1000
ErasePolarity = 1

0x100|0x500
FV = SimpleVolume

[FV.SimpleVolume]
FvAlignment   = 16

FILE RAW = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  $(WORKDIR)/hello.txt
}
```

Like in the FD case, we can set some characteristics of FV via predefined tokens.
In this example we have only one token setting `FvAlignment = 16`, which is placed rigth after the `[FV.SimpleVolume]`. It is the only mandatory token for the `Firmware Volume`. We will talk about FV tokens later.

Next we define what goes into the FFS of the FV. Here we have one FILE of type `RAW`, which means that the file type `EFI_FFS_FILE_HEADER.Type` is equal to `EFI_FV_FILETYPE_RAW`.
And specification defines this type like this:
```
EFI_FV_FILETYPE_RAW
The file type EFI_FV_FILETYPE_RAW denotes a file that does not contain sections and is treated as a raw data file
```
The GUID value `15c658f6-eb5c-4b8f-b232-d6bd7368a73e` I've generated via `uuidgen` utility. It defines a unique name for our file in the FFS and will be written to the `EFI_FFS_FILE_HEADER.Name` field.
Inside the brackets we define content for the file. In our case it is our `hello.txt` generated via `echo "hello!" > hello.txt` command.

Let's build and check our FD image:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEIMAGE.fd -C
00000000  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00000100  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000110  78 e5 8c 8c 3d 8a 1c 4f  99 35 89 61 85 c3 2d d3  |x...=..O.5.a..-.|
00000120  00 05 00 00 00 00 00 00  5f 46 56 48 00 08 04 00  |........_FVH....|
00000130  48 00 cd e3 00 00 00 02  00 05 00 00 01 00 00 00  |H...............|
00000140  00 00 00 00 00 00 00 00  f6 58 c6 15 5c eb 8f 4b  |.........X..\..K|
00000150  b2 32 d6 bd 73 68 a7 3e  5f aa 01 00 1f 00 00 f8  |.2..sh.>_.......|
00000160  68 65 6c 6c 6f 21 0a ff  ff ff ff ff ff ff ff ff  |hello!..........|
00000170  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00000600
```
Besides FD, EDKII build system also generates images for Firmware Volumes. We've generated our FV with `Offset|Size = 0x100|0x500`, therefore you can see how `SIMPLEIMAGE.fd` has 0x100 bytes of 0xff's at the start of an image, and how `SIMPLEVOLUME.Fv` starts right from its data.
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -C
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  78 e5 8c 8c 3d 8a 1c 4f  99 35 89 61 85 c3 2d d3  |x...=..O.5.a..-.|
00000020  00 05 00 00 00 00 00 00  5f 46 56 48 00 08 04 00  |........_FVH....|
00000030  48 00 cd e3 00 00 00 02  00 05 00 00 01 00 00 00  |H...............|
00000040  00 00 00 00 00 00 00 00  f6 58 c6 15 5c eb 8f 4b  |.........X..\..K|
00000050  b2 32 d6 bd 73 68 a7 3e  5f aa 01 00 1f 00 00 f8  |.2..sh.>_.......|
00000060  68 65 6c 6c 6f 21 0a ff  ff ff ff ff ff ff ff ff  |hello!..........|
00000070  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00000500
```

Firmware Volume data starts with a header. In our case:
```cpp
typedef struct {
  UINT8                     ZeroVector[16];			= { 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 }
  EFI_GUID                  FileSystemGuid;			= { 0x8c8ce578, 0x8a3d, 0x4f1c, { 0x99, 0x35, 0x89, 0x61, 0x85, 0xc3, 0x2d, 0xd3 } } = EFI_FIRMWARE_FILE_SYSTEM2_GUID
  UINT64                    FvLength;				= 0x0000000000000500
  UINT32                    Signature;				= "_FVH"
  EFI_FVB_ATTRIBUTES_2      Attributes;				= 0x00040800 = (EFI_FVB2_ERASE_POLARITY | EFI_FVB2_ALIGNMENT_16)
  UINT16                    HeaderLength;			= 0x0048
  UINT16                    Checksum;				= 0xe3cd
  UINT16                    ExtHeaderOffset;			= 0x0000
  UINT8                     Reserved[1];			= 0x00
  UINT8                     Revision;				= 0x02
  EFI_FV_BLOCK_MAP_ENTRY    BlockMap[1];			= [{0x00000500, 0x00000001}, {0x00000000, 0x00000000}]
} EFI_FIRMWARE_VOLUME_HEADER;
```

Right after the `EFI_FIRMWARE_VOLUME_HEADER` we have a header for our only file:
```cpp
typedef struct {
 EFI_GUID Name;							= 15c658f6-eb5c-4b8f-b232-d6bd7368a73e
 EFI_FFS_INTEGRITY_CHECK IntegrityCheck;			= 0x5faa
 EFI_FV_FILETYPE Type;						= 0x01 (=EFI_FV_FILETYPE_RAW)
 EFI_FFS_FILE_ATTRIBUTES Attributes;				= 0x00
 UINT8 Size[3];							= 0x00001f
 EFI_FFS_FILE_STATE State;					= 0xf8
} EFI_FFS_FILE_HEADER;
```

Right after that we have a content of our file `hello.txt`:
```
$ hexdump hello.txt -C
00000000  68 65 6c 6c 6f 21 0a                              |hello!.|
0000000
```

The rest of the FV is filled with `0xff`.

# VolInfo

There is an utility `BaseTools/BinWrappers/PosixLike/VolInfo` that you can use to dump information about Firmware Volumes. Here is an example how we can use it to dump information about our Formware Volume:
```
$ VolInfo Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -x Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
VolInfo Version 1.0 Build Developer Build based on Revision: Unknown
ParseGuidBaseNameFile: Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
Signature:        _FVH (4856465F)
Attributes:       40800
       EFI_FVB2_ERASE_POLARITY
       EFI_FVB2_ALIGNMENT_16
Header Length:         0x00000048
File System ID:        8c8ce578-8a3d-4f1c-9935-896185c32dd3
Revision:              0x0002
Number of Blocks:      0x00000500
Block Length:          0x00000001
Total Volume Size:     0x00000500
============================================================
File Name:        15C658F6-EB5C-4B8F-B232-D6BD7368A73E  /<...>/edk2/$(WORKDIR)/hello.txt
File Offset:      0x00000048
File Length:      0x0000001F
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x01  EFI_FV_FILETYPE_RAW
There are a total of 1 files in this FV
```

As you can see this utility gives us the same information that we've parsed ourselves.

# Another file

Let's add another file to our FFS. For a change let's initialize our next file with binary content:
```
$ echo -n -e \\xDE\\xAD\\xBE\\xEF > DEADBEEF.txt
$ hexdump DEADBEEF.txt -C
00000000  de ad be ef                                       |....|
00000004
```
Now let's add it to our FFS:
```
[FD.SimpleImage]
BaseAddress   = 0x0
Size          = 0x1000
ErasePolarity = 1

0x100|0x500
FV = SimpleVolume

[FV.SimpleVolume]
FvAlignment   = 16

FILE RAW = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  $(WORKDIR)/hello.txt
}

FILE RAW = dd77425e-d338-43e7-8e94-1a755e0c217d {
  $(WORKDIR)/DEADBEEF.txt
}
```

Build image and look at the Firmware Volume content:
```
$ hexdump /home/aladyshev/tiano/2021/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -C
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  78 e5 8c 8c 3d 8a 1c 4f  99 35 89 61 85 c3 2d d3  |x...=..O.5.a..-.|
00000020  00 05 00 00 00 00 00 00  5f 46 56 48 00 08 04 00  |........_FVH....|
00000030  48 00 cd e3 00 00 00 02  00 05 00 00 01 00 00 00  |H...............|
00000040  00 00 00 00 00 00 00 00  f6 58 c6 15 5c eb 8f 4b  |.........X..\..K|
00000050  b2 32 d6 bd 73 68 a7 3e  5f aa 01 00 1f 00 00 f8  |.2..sh.>_.......|
00000060  68 65 6c 6c 6f 21 0a ff  5e 42 77 dd 38 d3 e7 43  |hello!..^Bw.8..C|
00000070  8e 94 1a 75 5e 0c 21 7d  01 aa 01 00 1c 00 00 f8  |...u^.!}........|
00000080  de ad be ef ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
00000090  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00000500
```

In this picture I've tried to provide visual parsing of data:
![FV](FV.png?raw=true)

Here you can see how our files follow each other in the FFS. Each of the files has its own `EFI_FFS_FILE_HEADER` with its unique Name (=GUID). The important thing to note that the filesystem is flat, files just follow one another. Therefore to find some file by GUID, we need to traverse FFS from the start.
Also here you can see that the padding byte 0xff was inserted between the files. It was inserted because according to the specification each file must start at 8 byte boundary.

We can use `VolInfo` to see how it interprets our Firmware Volume:
```
$ VolInfo Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -x Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
VolInfo Version 1.0 Build Developer Build based on Revision: Unknown
ParseGuidBaseNameFile: Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
Signature:        _FVH (4856465F)
Attributes:       40800
       EFI_FVB2_ERASE_POLARITY
       EFI_FVB2_ALIGNMENT_16
Header Length:         0x00000048
File System ID:        8c8ce578-8a3d-4f1c-9935-896185c32dd3
Revision:              0x0002
Number of Blocks:      0x00000500
Block Length:          0x00000001
Total Volume Size:     0x00000500
============================================================
File Name:        15C658F6-EB5C-4B8F-B232-D6BD7368A73E  /home/aladyshev/tiano/2021/edk2/$(WORKDIR)/hello.txt
File Offset:      0x00000048
File Length:      0x0000001F
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x01  EFI_FV_FILETYPE_RAW
============================================================
File Name:        DD77425E-D338-43E7-8E94-1A755E0C217D  /home/aladyshev/tiano/2021/edk2/$(WORKDIR)/DEADBEEF.txt
File Offset:      0x00000068
File Length:      0x0000001C
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x01  EFI_FV_FILETYPE_RAW
There are a total of 2 files in this FV
```
Indeed the FV contains 2 files of type `EFI_FV_FILETYPE_RAW`.


# Firmware Volume attributes

Currently in our `Firmware Volume` we've defined only one attribute `FvAlignment`. Along with these attributes it help to set flags in the `EFI_FIRMWARE_VOLUME_HEADER.Attributes` field.
```
FvAlignment        = <...>
ERASE_POLARITY     = 1|0
MEMORY_MAPPED      = TRUE|FALSE
STICKY_WRITE       = TRUE|FALSE
LOCK_CAP           = TRUE|FALSE
LOCK_STATUS        = TRUE|FALSE
WRITE_DISABLED_CAP = TRUE|FALSE
WRITE_ENABLED_CAP  = TRUE|FALSE
WRITE_STATUS       = TRUE|FALSE
WRITE_LOCK_CAP     = TRUE|FALSE
WRITE_LOCK_STATUS  = TRUE|FALSE
READ_DISABLED_CAP  = TRUE|FALSE
READ_ENABLED_CAP   = TRUE|FALSE
READ_STATUS        = TRUE|FALSE
READ_LOCK_CAP      = TRUE|FALSE
READ_LOCK_STATUS   = TRUE|FALSE
```
The setting of these attributes will set respective `EFI_FVB2_*` flags which are defined in the [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Pi/PiFirmwareVolume.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Pi/PiFirmwareVolume.h).
You can read the meaning of these flags in the `UEFI Platform Initialization (PI) specification (Volume 3: Shared Architectural Elements)`.
If you look to `VolInfo` output, you'll see that out FV has two attributes set:
- `EFI_FVB2_ERASE_POLARITY` (set by default, means that uninitialized data bits in volume are set to 1)
- `EFI_FVB2_ALIGNMENT_16` (set by our `FvAlignment=16` setting)

Similar to FD, the FV has an attribute that defines an address at which flash volume would be mapped to the CPU memory:
```
FvBaseAddress = <...>
```
And the tokens that define flash block structure:
```
BlockSize = <...>
NumBlocks = <...>
```

Other possible attribute for the FV that you can come across is `FvNameGuid`:
```
FvNameGuid           = <GUID>
# Example:
# FvNameGuid         = 763BED0D-DE9F-48F5-81F1-3E90E1B1A015
```
This attribute would lead to the creation of a file of type `EFI_FV_FILETYPE_FFS_PAD` (padding file) with a GUID value in its data. This file would be placed first in the FV. We would investigate this file when we would talk about different file types.

# Links

- [\[FV\] Sections](https://edk2-docs.gitbook.io/edk-ii-fdf-specification/2_fdf_design_discussion/25_-fv-_sections)

- [Create the FV Image File(s)](https://edk2-docs.gitbook.io/edk-ii-build-specification/10_post-build_imagegen_stage_-_flash/104_create_the_fv_image_files)

