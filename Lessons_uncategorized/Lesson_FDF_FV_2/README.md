In the last lesson we've encoded our data to FFS in the files of type `RAW` (`EFI_FFS_FILE_HEADER.Type = EFI_FV_FILETYPE_RAW`). It is the most simple type, in this case all of our data is written as is right after the file header.

Other file types consist of sections, with each section prepended with a header `EFI_COMMON_SECTION_HEADER` ([https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Pi/PiFirmwareFile.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Pi/PiFirmwareFile.h)):
```
EFI_COMMON_SECTION_HEADER

Summary:
Defines the common header for all the section types

Prototype:
typedef struct {
 UINT8 Size[3];
 EFI_SECTION_TYPE Type;
} EFI_COMMON_SECTION_HEADER;

Parameters:
Size	A 24-bit unsigned integer that contains the total size of the section in bytes, including the EFI_COMMON_SECTION_HEADER
Type	Declares the section type

Description:
The type EFI_COMMON_SECTION_HEADER defines the common header for all the section types
```

Specification defines following sections:
| Name | Value | Description |
| ---- | ----- | ----------- |
| EFI_SECTION_COMPRESSION | 0x01 | Encapsulation section where other sections are compressed |
| EFI_SECTION_GUID_DEFINED | 0x02 | Encapsulation section where other sections have format defined by a GUID |
| EFI_SECTION_DISPOSABLE | 0x03 | Encapsulation section used during the build process but not required for execution |
| EFI_SECTION_PE32 | 0x10 | PE32+ Executable image |
| EFI_SECTION_PIC | 0x11 | Position-Independent Code |
| EFI_SECTION_TE | 0x12 | Terse Executable image |
| EFI_SECTION_DXE_DEPEX | 0x13 | DXE Dependency Expression |
| EFI_SECTION_VERSION | 0x14 | Version, Text and Numeric |
| EFI_SECTION_USER_INTERFACE | 0x15 | User-Friendly name of the driver |
| EFI_SECTION_COMPATIBILITY16 | 0x16 | DOS-style 16-bit EXE |
| EFI_SECTION_FIRMWARE_VOLUME_IMAGE | 0x17 | PI Firmware Volume image |
| EFI_SECTION_FREEFORM_SUBTYPE_GUID | 0x18 | Raw data with GUID in header to define format |
| EFI_SECTION_RAW | 0x19 | Raw data |
| EFI_SECTION_PEI_DEPEX | 0x1b | PEI Dependency Expression |
| EFI_SECTION_MM_DEPEX | 0x1c | Leaf section type for determining the dispatch order for an MM Traditional driver in MM Traditional Mode or MM Standaline driver in MM Standalone Mode |

PI specification defines which sections each type of file must have. For example the file of type `EFI_FV_FILETYPE_APPLICATION=0x09` must have at least one `EFI_SECTION_PE32=0x10` section.

In this lesson we would work with a sectioned file without any such limitations - `FREEFORM` file.

According to the PI documentation the file type `EFI_FV_FILETYPE_FREEFORM = 0x02` denotes a sectioned file that may contain any combination of sections.

Here is an example of how we add such file. Let's start with one `RAW` section in the file:
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
  SECTION RAW = $(WORKDIR)/hello.txt
}
```

`hexdump` output would look like this:
```
$ hexdump /<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -C
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  78 e5 8c 8c 3d 8a 1c 4f  99 35 89 61 85 c3 2d d3  |x...=..O.5.a..-.|
00000020  00 05 00 00 00 00 00 00  5f 46 56 48 00 08 04 00  |........_FVH....|
00000030  48 00 cd e3 00 00 00 02  00 05 00 00 01 00 00 00  |H...............|
00000040  00 00 00 00 00 00 00 00  f0 9c ed f6 c1 cd a1 40  |...............@|
00000050  99 09 ac 6a 2f 43 5e 23  52 aa 02 00 23 00 00 f8  |...j/C^#R...#...|
00000060  0b 00 00 19 68 65 6c 6c  6f 21 0a ff ff ff ff ff  |....hello!......|
00000070  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00000500
```

And here is `VolInfo` output. Everything is exactly as it is supposed to be. Firmware Volume contains one `EFI_FV_FILETYPE_FREEFORM` file with one `EFI_SECTION_RAW`:
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
File Name:        F6ED9CF0-CDC1-40A1-9909-AC6A2F435E23  /<...>/$(WORKDIR)/hello.txt
File Offset:      0x00000048
File Length:      0x00000023
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
There are a total of 1 files in this FV
```

# Create more `RAW` sections

Let's try to add more `RAW` sections to our file. We can use the same file for the section content, it is absolutely legit:
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
  SECTION RAW = $(WORKDIR)/hello.txt
  SECTION RAW = $(WORKDIR)/hello.txt
  SECTION RAW = $(WORKDIR)/hello.txt
}
```

hexdump:
```
$ hexdump /<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -C
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  78 e5 8c 8c 3d 8a 1c 4f  99 35 89 61 85 c3 2d d3  |x...=..O.5.a..-.|
00000020  00 05 00 00 00 00 00 00  5f 46 56 48 00 08 04 00  |........_FVH....|
00000030  48 00 cd e3 00 00 00 02  00 05 00 00 01 00 00 00  |H...............|
00000040  00 00 00 00 00 00 00 00  f0 9c ed f6 c1 cd a1 40  |...............@|
00000050  99 09 ac 6a 2f 43 5e 23  3a aa 02 00 3b 00 00 f8  |...j/C^#:...;...|
00000060  0b 00 00 19 68 65 6c 6c  6f 21 0a 00 0b 00 00 19  |....hello!......|
00000070  68 65 6c 6c 6f 21 0a 00  0b 00 00 19 68 65 6c 6c  |hello!......hell|
00000080  6f 21 0a ff ff ff ff ff  ff ff ff ff ff ff ff ff  |o!..............|
00000090  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00000500
```

If you parse it, it look like this:
![FREEFORM](FREEFORM.png?raw=true)


VolInfo:
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
File Name:        F6ED9CF0-CDC1-40A1-9909-AC6A2F435E23  /<...>/$(WORKDIR)/hello.txt
File Offset:      0x00000048
File Length:      0x0000003B
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
There are a total of 1 files in this FV
```

# FV build process

Before we go any further in our investigation of section types it would be good to know a little about EDKII build process and utilities.

If you execute `build` with `-v` (`--verbose`) argument, the EDKII build system would output a lot information about the build. In the end of the output you could see how EDKII uses its tools for image generation.

Look at the output after the:
```
Generating SIMPLEVOLUME FV
```

## `GenSec`

The first string is:
```
['GenSec', '-s', 'EFI_SECTION_RAW', '-o', '<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.raw', '<...>/$(WORKDIR)/hello.txt', '-v']
```
This means that `GenSec` utility from the BaseTools was called and shows its arguments. This is the EDKII utility to construct sections from the data.
```
GenSec -s EFI_SECTION_RAW -o <...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.raw' <...>/$(WORKDIR)/hello.txt -v
```
You can look at the `GenSec` help for the options description, but basically this call means:
```
GenSec --sectiontype EFI_SECTION_RAW --outputfile <output_file.raw> <input_file> --verbose
```

You can look at the output file content:
```
$ hexdump /<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f4
35e23SEC1.raw -C
00000000  0a 00 00 19 68 65 6c 6c  6f 0a                    |....hello.|
0000000a
```
Here you can see how the data was prepended with a section header for the RAW section.

The long path that is used here is just a:
```
FV/Ffs/<FILE_GUID><FV_NAME>/<FILE_GUID>SEC<SECTION NUMBER>.raw
```
You can see how the next section files are created with increased `<SECTION NUMBER>`.

Build even saves the command, that was used to generate each section.

Right to each of the `<...>.raw` files you can find `<...>.raw.txt` file which includes `GenSec` command that was used for the file generation.

```
$ cat Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.raw.txt
GenSec -s EFI_SECTION_RAW -o /<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.raw /<...>/$(WORKDIR)/hello.txt
```

## `GenFfs`

After we have all of the 3 section we can generate a file. In the output this corresponds to this call:
```
['GenFfs', '-t', 'EFI_FV_FILETYPE_FREEFORM', '-g', 'f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23', '-o', '/home/aladyshev/edk2_patches/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23.ffs', '-i', '/home/aladyshev/edk2_patches/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.raw', '-i', '/home/aladyshev/edk2_patches/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC2.raw', '-i', '/home/aladyshev/edk2_patches/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC3.raw', '-v']
```

`GenFfs` is another utility from the BaseTools. Here is the usage above written in a more descriptive way:
```
GenFfs --filetype EFI_FV_FILETYPE_FREEFORM \
       --fileguid f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 \
       --outputfile /<...>/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23.ffs \
       --sectionfile /<...>/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.raw \
       --sectionfile /<...>/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC2.raw \
       --sectionfile /<...>/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC3.raw \
       --verbose
```
Here you can see how the `.ffs` file is constructed from the several sections that we've created earlier.

You can look at the file and see how our sections are combined and prepended with a file header:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f4
35e23.ffs -C
00000000  f0 9c ed f6 c1 cd a1 40  99 09 ac 6a 2f 43 5e 23  |.......@...j/C^#|
00000010  3b aa 02 00 3a 00 00 07  0a 00 00 19 68 65 6c 6c  |;...:.......hell|
00000020  6f 0a 00 00 0a 00 00 19  68 65 6c 6c 6f 0a 00 00  |o.......hello...|
00000030  0a 00 00 19 68 65 6c 6c  6f 0a                    |....hello.|
0000003a
```

Like with the sections right to the `<...>.ffs` file there would be `<...>.ffs.txt` file that would contain the string of the call to the `GenFfs` utility`

## `GenFv`

Finally `GenFv` utility is called for the Firmware Volume (`FV`) generation:
```
['GenFv', '-a', '/home/aladyshev/edk2_patches/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/SIMPLEVOLUME.inf', '-o', '/home/aladyshev/edk2_patches/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv', '-i', '/home/aladyshev/edk2_patches/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.inf', '-v']
```

Here is it in a more pleasant display:
```
GenFv --addrfile /<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/SIMPLEVOLUME.inf \
      --outputfile /<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv \
      --inputfile /<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.inf \
      --verbose
```

If you look at the content of the inpur file created in the build `Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.inf`, you would see how it references the `*.ffs` file that we've created earlier:
```
[options]
EFI_BASE_ADDRESS = 0x100
EFI_BLOCK_SIZE  = 0x1
EFI_NUM_BLOCKS   =  0x500
[attributes]
EFI_ERASE_POLARITY   =  1
EFI_FVB2_ALIGNMENT_16 = TRUE
[files]
EFI_FILE_NAME = /<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23.ffs
```

The output file `SIMPLEVOLUME.Fv` is the one that we've usually investigated before:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -C
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  78 e5 8c 8c 3d 8a 1c 4f  99 35 89 61 85 c3 2d d3  |x...=..O.5.a..-.|
00000020  00 05 00 00 00 00 00 00  5f 46 56 48 00 08 04 00  |........_FVH....|
00000030  48 00 cd e3 00 00 00 02  00 05 00 00 01 00 00 00  |H...............|
00000040  00 00 00 00 00 00 00 00  f0 9c ed f6 c1 cd a1 40  |...............@|
00000050  99 09 ac 6a 2f 43 5e 23  3b aa 02 00 3a 00 00 f8  |...j/C^#;...:...|
00000060  0a 00 00 19 68 65 6c 6c  6f 0a 00 00 0a 00 00 19  |....hello.......|
00000070  68 65 6c 6c 6f 0a 00 00  0a 00 00 19 68 65 6c 6c  |hello.......hell|
00000080  6f 0a ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |o...............|
00000090  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00000500
```

Now you know how modular is flash image build. `build` tool that we use actually delegates many tasks to the simple utilities like `GenSec`/`GenFfs`/`GenFv` and many sintermediate files are created along the build process.

# `EFI_SECTION_USER_INTERFACE`

Let's investigate `EFI_SECTION_USER_INTERFACE=0x15` section type:
```
EFI_SECTION_USER_INTERFACE

Summary:
A leaf section type that contains a Unicode string that contains a human-readable file name.

Prototype:
typedef struct {
 EFI_COMMON_SECTION_HEADER CommonHeader;
 CHAR16 FileNameString[];
} EFI_USER_INTERFACE_SECTION;

Description:
The user interface file name section is a leaf section that contains a Unicode string that contains a human-readable file name.
This section is optional and is not required for any file types. There must never be more than one user interface file name section contained within a file.
```

As you can see this section is really simple. Its data is just a `CHAR16` string. EDKII has syntax to initialize this section in-place:
```
FILE FREEFORM = f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 {
  SECTION UI = "MyUI"
}
```
`VolInfo` can display string of this section:
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
File Name:        F6ED9CF0-CDC1-40A1-9909-AC6A2F435E23  MyUI
File Offset:      0x00000048
File Length:      0x00000026
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_USER_INTERFACE
  Size:  0x0000000E
  String: MyUI
There are a total of 1 files in this FV
```

Just in case, I want to point out, that like the rest of section, this section has a slightly different format in case of a section size above 16MB. Here and after we would inspect only "small" sections. For the "large" section format consult PI specification.

# `EFI_SECTION_VERSION`

Another simple section that you might come across is `EFI_SECTION_VERSION`:
```
Summary:
A leaf section type that contains a numeric build number and an optional Unicode string that represents the file revision.

Prototype:
typedef struct {
 EFI_COMMON_SECTION_HEADER CommonHeader;
 UINT16 BuildNumber;
 CHAR16 VersionString[];
} EFI_VERSION_SECTION;

Parameters:
CommonHeader	Common section header. CommonHeader.Type = EFI_SECTION_VERSION.
BuildNumber	A UINT16 that represents a particular build. Subsequent builds have monotonically 
		increasing build numbers relative to earlier builds.
VersionString	A null-terminated Unicode string that contains a text representation of the version. If 
		there is no text representation of the version, then an empty string must be provided.

Description:
A version section is a leaf section that contains a numeric build number and an optional Unicode string that represents the file revision.
To facilitate versioning of PEIMs, DXE drivers, and other files, a version section may be included in a file.
There must never be more than one version section contained within a file.
```

The syntax for adding this section is this:
```
FILE FREEFORM = f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 {
  SECTION BUILD_NUM = 42 VERSION = "MyVersion"
}
```

Once again `VolInfo` is able to display the content of this section:
```
$ VolInfo Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -x Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
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
File Length:      0x00000032
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_VERSION
  Size:  0x0000001A
  Build Number:  0x002A
  Version String:  MyVersion
There are a total of 1 files in this FV
```
# `EFI_SECTION_FREEFORM_SUBTYPE_GUID`

`EFI_SECTION_FREEFORM_SUBTYPE_GUID` section is very similar to raw, except that its header contains GUID value to describe the data inside the section.

Here is definition from the PI specification:
```
EFI_SECTION_FREEFORM_SUBTYPE_GUID

Summary:
A leaf section type that contains a single EFI_GUID in the header to describe the raw data.

Prototype:
typedef struct {
 EFI_COMMON_SECTION_HEADER CommonHeader;
 EFI_GUID SubTypeGuid;
} EFI_FREEFORM_SUBTYPE_GUID_SECTION;

Parameters:
CommonHeader	Common section header. CommonHeader.Type = EFI_SECTION_FREEFORM_SUBTYPE_GUID.
SubtypeGuid	This GUID is defined by the creator of the file. It is a vendor-defined file type.

Description:
A free-form subtype GUID section is a leaf section that contains a single EFI_GUIDin the header to describe the raw data
```

The syntax for adding this section is this:
```
SECTION SUBTYPE_GUID <Guid> = <File>
```

Let's try to use it:
```
FILE FREEFORM = f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 {
  SECTION SUBTYPE_GUID cf1fce30-b181-4d6a-b860-354c922e5c3e = $(WORKDIR)/hello.txt
}
```

Here is how VolInfo output looks like in this case:
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
File Length:      0x00000033
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_FREEFORM_SUBTYPE_GUID
  Size:  0x0000001B
  Guid:  cf1fce30-b181-4d6a-b860-354c922e5c3e

There are a total of 1 files in this FV
```

# `EFI_SECTION_PE32`/`EFI_SECTION_PIC`/`EFI_SECTION_COMPATIBILITY16`/`EFI_SECTION_TE`

Next there is a couple of sections for executable images. Most often you would use `EFI_SECTION_PE32` and include `.efi` code as your data. But nevertheless here is some info about all of the sections from the caption.

Here in VolInfo output I will show you just the section part, as the rest doesn't have any interesting changes. Also as the build system doesn't check the content of the section data, I will just use the same `hello.txt` file for the section content that we've created earlier:

## `EFI_SECTION_PE32`
```
EFI_SECTION_PE32
Summary
A leaf section type that contains a complete PE32+ image.
Prototype
typedef EFI_COMMON_SECTION_HEADER EFI_PE32_SECTION;

Description
The PE32+ image section is a leaf section that contains a complete PE32+ image. Normal UEFI 
executables are stored within PE32+ images
```
Example usage:
```
FILE FREEFORM = f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 {
  SECTION PE32 = $(WORKDIR)/hello.txt
}
```
VolInfo display for the section:
```
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x0000000B
```

## `EFI_SECTION_PIC`
```
EFI_SECTION_PIC
Summary
A leaf section type that contains a position-independent-code (PIC) image.
Prototype
typedef EFI_COMMON_SECTION_HEADER EFI_PIC_SECTION;

Description
A PIC image section is a leaf section that contains a position-independent-code (PIC) image.
```

Example usage:
```
FILE FREEFORM = f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 {
  SECTION PIC = $(WORKDIR)/hello.txt
}
```
VolInfo output for the section:
```
------------------------------------------------------------
  Type:  EFI_SECTION_PIC
  Size:  0x0000000B
```

## `EFI_SECTION_COMPATIBILITY16`
```
EFI_SECTION_COMPATIBILITY16

Summary:
A leaf section type that contains an IA-32 16-bit executable image.

Prototype:
typedef EFI_COMMON_SECTION_HEADER EFI_COMPATIBILITY16_SECTION;

Description:
A Compatibility16 image section is a leaf section that contains an IA-32 16-bit executable image. 
IA-32 16-bit legacy code that may be included in PI Architecture firmware is stored in a 16-bit 
executable image.
```

Example usage:
```
FILE FREEFORM = f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 {
  SECTION COMPAT16 = $(WORKDIR)/hello.txt
}
```
VolInfo output for the section:
```
------------------------------------------------------------
  Type:  EFI_SECTION_COMPATIBILITY16
  Size:  0x0000000B
```

## `EFI_SECTION_TE`
```
EFI_SECTION_TE
Summary
A leaf section that contains a Terse Executable (TE) image.
Prototype
typedef EFI_COMMON_SECTION_HEADER EFI_TE_SECTION;

Description
The terse executable section is a leaf section that contains a Terse Executable (TE) image. A TE 
image is an executable image format specific to the PI Architecture that is used for storing 
executable images in a smaller amount of space than would be required by a full PE32+ image.
```

Example usage:
```
FILE FREEFORM = f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23 {
  SECTION TE = $(WORKDIR)/hello.txt
}
```

You would find that in this case build actually checks the file content. Build fails with the error:
```
DOS header signature was not found in /<...>/hello.txt image.
```
But if you would use correct file, the VolInfo would show you `EFI_SECTION_TE` section.

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

VolInfo would parse it like this:
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

For the proof compare the hexdump output from the compression section with `PI_STD` argument (or without any arguments):
```
$ hexdump /<...>/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -C
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  78 e5 8c 8c 3d 8a 1c 4f  99 35 89 61 85 c3 2d d3  |x...=..O.5.a..-.|
00000020  00 05 00 00 00 00 00 00  5f 46 56 48 00 08 04 00  |........_FVH....|
00000030  48 00 cd e3 00 00 00 02  00 05 00 00 01 00 00 00  |H...............|
00000040  00 00 00 00 00 00 00 00  f0 9c ed f6 c1 cd a1 40  |...............@|
00000050  99 09 ac 6a 2f 43 5e 23  33 aa 02 00 42 00 00 f8  |...j/C^#3...B...|
00000060  2a 00 00 01 23 00 00 00  01 19 00 00 00 23 00 00  |*...#........#..|
00000070  00 00 0d 3a 51 8d 45 7d  6a 6a 52 e1 7e 0c 86 0b  |...:Q.E}jjR.~...|
00000080  92 10 25 86 35 27 6d 1e  c0 00 ff ff ff ff ff ff  |..%.5'm.........|
00000090  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00000500
```
And with the `PI_NONE` argument:
```
$ hexdump /<...>/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -C
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  78 e5 8c 8c 3d 8a 1c 4f  99 35 89 61 85 c3 2d d3  |x...=..O.5.a..-.|
00000020  00 05 00 00 00 00 00 00  5f 46 56 48 00 08 04 00  |........_FVH....|
00000030  48 00 cd e3 00 00 00 02  00 05 00 00 01 00 00 00  |H...............|
00000040  00 00 00 00 00 00 00 00  f0 9c ed f6 c1 cd a1 40  |...............@|
00000050  99 09 ac 6a 2f 43 5e 23  31 aa 02 00 44 00 00 f8  |...j/C^#1...D...|
00000060  2c 00 00 01 23 00 00 00  00 0b 00 00 19 68 65 6c  |,...#........hel|
00000070  6c 6f 21 0a 00 0b 00 00  19 68 65 6c 6c 6f 21 0a  |lo!......hello!.|
00000080  00 0b 00 00 19 68 65 6c  6c 6f 21 0a ff ff ff ff  |.....hello!.....|
00000090  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00000500
```
In the second case you can clearly see the `hello!` strings from out `hello.txt` file.

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
The VolInfo output would look like this:
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
The section would be processed by the `fc1bcdb0-7d31-49aa-936a-a4600d9dd083` tool, which correspond to the `GenCrc32` tool. For the proof look at the VolInfo output:
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

Here is VolInfo output for this case:
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

# `EFI_SECTION_DISPOSABLE`

Creation of this type of section is not supported by edk2. There is no syntax for it. But nevertheless here is definition from the specification:
```
EFI_SECTION_DISPOSABLE

Summary:
An encapsulation section type in which the section data is disposable.

Prototype:
typedef EFI_COMMON_SECTION_HEADER EFI_DISPOSABLE_SECTION;

Description:
A disposable section is an encapsulation section in which the section data may be disposed of during the process of creating or updating a firmware image without significant impact on the usefulness of the file.
```

# `EFI_SECTION_PEI_DEPEX`/`EFI_SECTION_DXE_DEPEX`

There are also sections for the dependency expressions for DXE and PEI stages. You probably wouldn't write them yourself, but here is their description for completeness:

```
EFI_SECTION_PEI_DEPEX

Summary:
A leaf section type that is used to determine dispatch order for a PEIM.

Prototype:
typedef EFI_COMMON_SECTION_HEADER EFI_PEI_DEPEX_SECTION;

Description:
The PEI dependency expression section is a leaf section that contains a dependency expression that is used to determine dispatch order for a PEIM. See the Platform Initialization Pre-EFI Initialization Core Interface Specification for details regarding the format of the dependency expression. 
```

```
EFI_SECTION_DXE_DEPEX

Summary:
A leaf section type that is used to determine the dispatch order for a DXE driver.

Prototype:
typedef EFI_COMMON_SECTION_HEADER EFI_DXE_DEPEX_SECTION;

Description
The DXE dependency expression section is a leaf section that contains a dependency expression that is used to determine the dispatch order for a DXE driver. See the Platform Initialization Driver Execution Environment Core Interface Specification for details regarding the format of the dependency expression.
```

