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

In this lesson we would work with a sectioned file without any such limitations - `FREEFORM` file. In this lesson we want to investigate sections, so this is the perfect file type for this task.

According to the PI documentation the file type `EFI_FV_FILETYPE_FREEFORM = 0x02` denotes a sectioned file that may contain any combination of sections.

Here is an example of how we add such file. Let's start with one `RAW` section in the file.

First here is a section description from the specification:
```
EFI_SECTION_RAW

Summary:
A leaf section type that contains an array of zero or more bytes.

Prototype:
typedef EFI_COMMON_SECTION_HEADER EFI_RAW_SECTION;

Description:
A raw section is a leaf section that contains an array of zero or more bytes. No particular formatting of these bytes is implied by this section type
```

Just in case, I want to point out, that like the rest of sections, this section has a slightly different format in case if section size is above 16MB. Here and after we would inspect only "small" sections. For the "large" section format consult PI specification.

Now here is a code:
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

`hexdump` output of the resulting Firmware Volume would look like this:
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

`hexdump`:
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

If you parse it, it looks like this:

![FREEFORM](FREEFORM.png?raw=true)


`VolInfo`:
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

Look at the output after the string:
```
Generating SIMPLEVOLUME FV
```

## `GenSec`

The first string after the output above is:
```
['GenSec', '-s', 'EFI_SECTION_RAW', '-o', '<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.raw', '<...>/$(WORKDIR)/hello.txt', '-v']
```
This means that `GenSec` utility from the BaseTools was called and shows its arguments. This is the EDKII utility to construct sections from the data.
```
GenSec -s EFI_SECTION_RAW -o <...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.raw' <...>/$(WORKDIR)/hello.txt -v
```
You can look at the `GenSec` help for the options description. After the command `. edksetup.sh` this utility would be in your path. But basically this call means:
```
GenSec --sectiontype EFI_SECTION_RAW \
       --outputfile <output_file.raw> \
       <input_file> \
       --verbose
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

After we have all of the 3 sections we can generate a file. In the output this corresponds to this call:
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
Here you can see how the `.ffs` file is constructed from the several sections that were created earlier.

You can look at the output file and see how our sections are combined and prepended with a file header:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f4
35e23.ffs -C
00000000  f0 9c ed f6 c1 cd a1 40  99 09 ac 6a 2f 43 5e 23  |.......@...j/C^#|
00000010  3b aa 02 00 3a 00 00 07  0a 00 00 19 68 65 6c 6c  |;...:.......hell|
00000020  6f 0a 00 00 0a 00 00 19  68 65 6c 6c 6f 0a 00 00  |o.......hello...|
00000030  0a 00 00 19 68 65 6c 6c  6f 0a                    |....hello.|
0000003a
```

Like with the sections, right to the `<...>.ffs` file there would be `<...>.ffs.txt` file that would contain the string of the call to the `GenFfs` utility

## `GenFv`

Finally `GenFv` utility is called for the Firmware Volume (`FV`) generation:
```
['GenFv', '-a', '/<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/SIMPLEVOLUME.inf', '-o', '/<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv', '-i', '/<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.inf', '-v']
```

Here is it in a more pleasant display:
```
GenFv --addrfile /<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/SIMPLEVOLUME.inf \
      --outputfile /<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv \
      --inputfile /<...>/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.inf \
      --verbose
```

If you look at the content of the input file created in the build process `Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.inf`, you would see how it references the `*.ffs` file that was created earlier:
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

The output file `SIMPLEVOLUME.Fv` is the one that we usually investigate:
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

Now you know how modular is flash image build. `build` tool that we use actually delegates many tasks to the simple utilities like `GenSec`/`GenFfs`/`GenFv` and many intermediate files are created along the build process.

