# `EFI_SECTION_COMPRESSION`

The `EFI_SECTION_COMPRESSION` is encapsulation section in a sense that it is a section that have sections in itself. Here is full definition from the PI specification:
```
EFI_SECTION_COMPRESSION

Summary:
An encapsulation section type in which the section data is compressed.

Prototype:
typedef struct {
 EFI_COMMON_SECTION_HEADER CommonHeader;
 UINT32 UncompressedLength;
 UINT8 CompressionType;
} EFI_COMPRESSION_SECTION;

Parameters:
CommonHeader		Usual common section header. CommonHeader.Type = EFI_SECTION_COMPRESSION.
UncompressedLength	UINT32 that indicates the size of the section data after decompression.
CompressionType		Indicates which compression algorithm is used.

Description:
A compression section is an encapsulation section in which the section data is compressed. To process the contents and extract the enclosed section stream, the section data must be decompressed using the decompressor indicated by the CompressionType parameter. The decompressed image is then interpreted as a section stream.
```

The example usage is this:
```
FILE FREEFORM = f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 {
  SECTION COMPRESS {
    SECTION RAW = $(WORKDIR)/hello.txt
    SECTION RAW = $(WORKDIR)/hello.txt
    SECTION RAW = $(WORKDIR)/hello.txt
  }
}
```

`VolInfo` would parse it like this:
```
$ VolInfo Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv
VolInfo Version 1.0 Build Developer Build based on Revision: Unknown
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
File Name:        F6ED9CF0-CDC1-40A1-9909-AC6A2F435E23
File Offset:      0x00000048
File Length:      0x00000042
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_COMPRESSION
  Size:  0x0000002A
  Uncompressed Length:  0x00000023
  Compression Type:  EFI_STANDARD_COMPRESSION
/------------ Encapsulation section start -----------------\
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
\------------ Encapsulation section end -------------------/
There are a total of 1 files in this FV
```

It is possible to set compression type in the section syntax. By default compress uses standard `PI_STD` compression.

If you want to you could point it explicitly. The output would be the same:
```
FILE FREEFORM = f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 {
  SECTION COMPRESS PI_STD {
    SECTION RAW = $(WORKDIR)/hello.txt
    SECTION RAW = $(WORKDIR)/hello.txt
    SECTION RAW = $(WORKDIR)/hello.txt
  }
}
```

If you don't want any compression for your data you can use `PI_NONE` compression type.
```
FILE FREEFORM = f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 {
  SECTION COMPRESS PI_NONE {
    SECTION RAW = $(WORKDIR)/hello.txt
    SECTION RAW = $(WORKDIR)/hello.txt
    SECTION RAW = $(WORKDIR)/hello.txt
  }
}
```

In this case section would just work as a section aggregator.

For the proof compare the `hexdump` output from the compression section with `PI_STD` argument (or without any arguments):
```
$ hexdump -C Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.com
00000000  2a 00 00 01 23 00 00 00  01 19 00 00 00 23 00 00  |*...#........#..|
00000010  00 00 0d 3a 51 8d 45 7d  6a 6a 52 e1 7e 0c 86 0b  |...:Q.E}jjR.~...|
00000020  92 10 25 86 35 27 6d 1e  c0 00                    |..%.5'm...|
0000002a
```
And with the `PI_NONE` argument:
```
$ hexdump -C Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.com
00000000  2c 00 00 01 23 00 00 00  00 0b 00 00 19 68 65 6c  |,...#........hel|
00000010  6c 6f 21 0a 00 0b 00 00  19 68 65 6c 6c 6f 21 0a  |lo!......hello!.|
00000020  00 0b 00 00 19 68 65 6c  6c 6f 21 0a              |.....hello!.|
0000002c
```
In the second case you can clearly see the `hello!` strings from our `hello.txt` file.

As you might have guessed already the extenstion of a file for this section is "*.com".

# `EFI_SECTION_GUID_DEFINED`

The `EFI_SECTION_GUID_DEFINED` is another encapsulation section. But in this case the data inside is processed with a specific tool, and the GUID of this tool is written to the section header, so you would know how exactly you need to interpret/decompress the data. In a way it is a more advanced `COMPRESS` section. The complete definition from the specification is this:
```
EFI_SECTION_GUID_DEFINED

Summary:
An encapsulation section type in which the method of encapsulation is defined by an identifying GUID. 

Prototype:
typedef struct {
 EFI_COMMON_SECTION_HEADER CommonHeader;
 EFI_GUID SectionDefinitionGuid;
 UINT16 DataOffset;
 UINT16 Attributes;
 // GuidSpecificHeaderFields;
} EFI_GUID_DEFINED_SECTION;

Parameters:
CommonHeader			Common section header. CommonHeader.Type = EFI_SECTION_GUID_DEFINED.
SectionDefinitionGuid		GUID that defines the format of the data that follows. It is a vendor-defined section type. 
DataOffset			Contains the offset in bytes from the beginning of the common header to the first byte of the data.
Attributes			Bit field that declares some specific characteristics of the section contents.
GuidSpecificHeaderFields	Zero or more bytes of data that are defined by the section’s GUID. An example of this data would be a digital signature and manifest. 
Data				Zero or more bytes of arbitrary data. The format of the data is defined by SectionDefinitionGuid.

Description:
A GUID-defined section contains a section-type-specific header that contains an identifying GUID, followed by an arbitrary amount of data. It is an encapsulation section in which the method of encapsulation is defined by the GUID. A matching instance of EFI_GUIDED_SECTION_EXTRACTION_PROTOCOL (DXE) or EFI_GUIDED_SECTION_EXTRACTION_PPI (PEI) is required to extract the contents of this encapsulation section. 
The GUID-defined section enables custom encapsulation section types for any purpose. One commonly expected use is creating an encapsulation section to enable a cryptographic authentication of the section contents.
```

The syntax for this section look like this:
```
SECTION GUIDED <Guid> {
  <...>
}
```

What GUIDs you can use? The ones that you have in you `Conf/tools_def.txt`. For example:
```
##################
# GenCrc32 tool definitions
##################
*_*_*_CRC32_PATH          = GenCrc32
*_*_*_CRC32_GUID          = FC1BCDB0-7D31-49AA-936A-A4600D9DD083

##################
# BrotliCompress tool definitions
##################
*_*_*_BROTLI_PATH        = BrotliCompress
*_*_*_BROTLI_GUID        = 3D532050-5CDA-4FD0-879E-0F7F630D5AFB

##################
# LzmaCompress tool definitions
##################
*_*_*_LZMA_PATH          = LzmaCompress
*_*_*_LZMA_GUID          = EE4E5898-3914-4259-9D6E-DC7BD79403CF

##################
# LzmaF86Compress tool definitions with converter for x86 code.
# It can improve the compression ratio if the input file is IA32 or X64 PE image.
##################
*_*_*_LZMAF86_PATH         = LzmaF86Compress
*_*_*_LZMAF86_GUID         = D42AE6BD-1352-4bfb-909A-CA72A6EAE889

##################
# TianoCompress tool definitions
##################
*_*_*_TIANO_PATH         = TianoCompress
*_*_*_TIANO_GUID         = A31280AD-481E-41B6-95E8-127F4C984779
```

If you'll want to write your own utility someday, this is the typical call for the utility:
```
<tool> -e -o <output_file> <input_file>
```
`-e` here means `encode` and `-o` the output file.


Now let's write an example that uses GUIDED section. Here we use `TianoCompress` utility GUID for the processing tool:
```
FILE FREEFORM = f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 {
  SECTION GUIDED A31280AD-481E-41B6-95E8-127F4C984779 {
    SECTION RAW = $(WORKDIR)/hello.txt
    SECTION RAW = $(WORKDIR)/hello.txt
    SECTION RAW = $(WORKDIR)/hello.txt
  }
}
```

The `hexdump` of this section would look like this:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.guided -C
00000000  39 00 00 02 ad 80 12 a3  1e 48 b6 41 95 e8 12 7f  |9........H.A....|
00000010  4c 98 47 79 18 00 01 00  19 00 00 00 23 00 00 00  |L.Gy........#...|
00000020  00 0d 3a 51 8d 45 7d 6a  6a 52 e1 7e 0c 86 0b 92  |..:Q.E}jjR.~....|
00000030  10 09 61 8d 49 db 47 b0  00                       |..a.I.G..|
00000039
```

And the `VolInfo` output for the FV would look like this:
```
$ VolInfo Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv
VolInfo Version 1.0 Build Developer Build based on Revision: Unknown
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
File Name:        F6ED9CF0-CDC1-40A1-9909-AC6A2F435E23
File Offset:      0x00000048
File Length:      0x00000051
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_GUID_DEFINED
  Size:  0x00000039
  SectionDefinitionGuid:  a31280ad-481e-41b6-95e8-127f4c984779

  DataOffset:             0x0018
  Attributes:             0x0001
/------------ Encapsulation section start -----------------\
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
\------------ Encapsulation section end -------------------/
There are a total of 1 files in this FV
```

One more notice. If you woudn't supply any GUID like this:
```
FILE FREEFORM = f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 {
  SECTION GUIDED {
    SECTION RAW = $(WORKDIR)/hello.txt
    SECTION RAW = $(WORKDIR)/hello.txt
    SECTION RAW = $(WORKDIR)/hello.txt
  }
}
```
The section would be processed by the `fc1bcdb0-7d31-49aa-936a-a4600d9dd083` tool, which correspond to the `GenCrc32` tool. For the proof look at hexdump:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.guided -C
00000000  3f 00 00 02 b0 cd 1b fc  31 7d aa 49 93 6a a4 60  |?.......1}.I.j.`|
00000010  0d 9d d0 83 1c 00 02 00  84 3e 2b 76 0b 00 00 19  |.........>+v....|
00000020  68 65 6c 6c 6f 21 0a 00  0b 00 00 19 68 65 6c 6c  |hello!......hell|
00000030  6f 21 0a 00 0b 00 00 19  68 65 6c 6c 6f 21 0a     |o!......hello!.|
0000003f
```

Or `VolInfo` output:
```
$ VolInfo Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv
VolInfo Version 1.0 Build Developer Build based on Revision: Unknown
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
File Name:        F6ED9CF0-CDC1-40A1-9909-AC6A2F435E23
File Offset:      0x00000048
File Length:      0x00000057
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_GUID_DEFINED
  Size:  0x0000003F
  SectionDefinitionGuid:  fc1bcdb0-7d31-49aa-936a-a4600d9dd083

  DataOffset:             0x001C
  Attributes:             0x0002
/------------ Encapsulation section start -----------------\
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
\------------ Encapsulation section end -------------------/
There are a total of 1 files in this FV
```

## `EFI_GUID_DEFINED_SECTION.Attributes`

```
//*****************************************************
// Bit values for GuidedSectionHeader.Attributes
//*****************************************************
#define EFI_GUIDED_SECTION_PROCESSING_REQUIRED 0x01     // Set to 1 if the section requires processing to obtain meaningful data from the section contents
#define EFI_GUIDED_SECTION_AUTH_STATUS_VALID 0x02       // Set to 1 if the section contains authentication data that is reported
```

Look at the `DataOffset` and `Attributes`
- for the `TianoCompress` section
```
DataOffset:             0x0018
Attributes:             0x0001
```
- for the `GenCrc32` section:
```
DataOffset:             0x001C
Attributes:             0x0002
```
Here you can see that `TianoCompress` section sets `EFI_GUIDED_SECTION_PROCESSING_REQUIRED` bit. Indeed, in this case the content of the section is compressed, and we need to decompress it before usage. `DataOffset` in this case points to the start of the data right after the header.

In case of `GenCrc32` section the  `EFI_GUIDED_SECTION_PROCESSING_REQUIRED` bit is unset, but in turn `EFI_GUIDED_SECTION_AUTH_STATUS_VALID` bit is set. `GenCrc32` just prepends the data with the CRC32 value. This is why DataOffset is increased to 4. And as we have some checksum, the auth bit is set.


If you want to, you can explicitly set these parameters. For example here is how you can invert these bits for our `TianoCompress` section:
```
SECTION GUIDED A31280AD-481E-41B6-95E8-127F4C984779 PROCESSING_REQUIRED = FALSE AUTH_STATUS_VALID = TRUE {
  ...
}
```
This doesn't change `DataOffset`, but changes `Attributes`. If you wull look to the image with VolInfo you would see:
```
Attributes:             0x0002
```

As you might have guessed already the extenstion of a file for this section is "*.guided".

# `EFI_SECTION_FIRMWARE_VOLUME_IMAGE`

With the help of `EFI_SECTION_FIRMWARE_VOLUME_IMAGE` section we can include full Firmware Volumes (FV's) into the sections:
```
EFI_SECTION_FIRMWARE_VOLUME_IMAGE

Summary:
A leaf section type that contains a PI Firmware Volume.

Prototype:
typedef EFI_COMMON_SECTION_HEADER EFI_FIRMWARE_VOLUME_IMAGE_SECTION;

Description:
A firmware volume image section is a leaf section that contains a PI Firmware Volume Image.
```

The syntax look like this:
```
SECTION FV_IMAGE = <FV_name>
```
Off couse `[FV.<FV_name>]` should be defined in this case.

Let's create another Firmware Volume with one simple file and test this type of section:
```
[FD.SimpleImage]
BaseAddress   = 0x0
Size          = 0x1000
ErasePolarity = 1

0x100|0x500
FV = SimpleVolume

[FV.SimpleVolume]
FvAlignment        = 16

FILE FREEFORM = f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 {
  SECTION FV_IMAGE = SimpleSubVolume
}


[FV.SimpleSubVolume]
FvAlignment        = 16

FILE FREEFORM = dc070116-d211-4ab1-a657-e0b6c64b2643 {
  SECTION RAW = $(WORKDIR)/hello.txt
}
```

Here is `VolInfo` output for this case:
```
$ VolInfo Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv
VolInfo Version 1.0 Build Developer Build based on Revision: Unknown
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
File Name:        F6ED9CF0-CDC1-40A1-9909-AC6A2F435E23
File Offset:      0x00000048
File Length:      0x00000098
File Attributes:  0x08
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000C
------------------------------------------------------------
  Type:  EFI_SECTION_FIRMWARE_VOLUME_IMAGE
  Size:  0x00000074
/------------ Firmware Volume section start ---------------\
============================================================
File Name:        DC070116-D211-4AB1-A657-E0B6C64B2643
File Offset:      0x00000048
File Length:      0x00000023
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
There are a total of 1 files in the child FV
\------------ Firmware Volume section end -----------------/
There are a total of 1 files in this FV
```

The section file extension is "*fv.sec":
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1fv.sec -C
00000000  74 00 00 17 00 00 00 00  00 00 00 00 00 00 00 00  |t...............|
00000010  00 00 00 00 78 e5 8c 8c  3d 8a 1c 4f 99 35 89 61  |....x...=..O.5.a|
00000020  85 c3 2d d3 70 00 00 00  00 00 00 00 5f 46 56 48  |..-.p......._FVH|
00000030  00 08 04 00 48 00 ed ec  00 00 00 02 70 00 00 00  |....H.......p...|
00000040  01 00 00 00 00 00 00 00  00 00 00 00 16 01 07 dc  |................|
00000050  11 d2 b1 4a a6 57 e0 b6  c6 4b 26 43 f6 aa 02 00  |...J.W...K&C....|
00000060  23 00 00 f8 0b 00 00 19  68 65 6c 6c 6f 21 0a ff  |#.......hello!..|
00000070  ff ff ff ff                                       |....|
00000074
```

