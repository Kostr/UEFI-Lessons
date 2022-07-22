We've covered two types of files:
- `EFI_FV_FILETYPE_RAW` - Binary data (`FILE RAW`)
- `EFI_FV_FILETYPE_FREEFORM` - Sectioned data (`FILE FREEFORM`)

Let's investigate other file types. The main difference is that other file types has limitations for the sections that must/can be inside the files.

# `EFI_FV_FILETYPE_SECURITY_CORE`

`EFI_FV_FILETYPE_SECURITY_CORE=0x03` file is for platform core code used during the SEC phase.

Specification defines this type like this:
```
EFI_FV_FILETYPE_SECURITY_CODE

The file type EFI_FV_FILETYPE_SECURITY_CORE denotes code and data that comprise the first part of PI Architecture firmware to execute. Its format is undefined with respect to the PI Architecture, as differing platform architectures may have varied requirements.
```

Although it says that the format is undefined, this code:
```
FILE SEC = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION RAW = $(WORKDIR)/hello.txt
}
```

Would result in a build failure:
```
GenFfs: ERROR 2000: Invalid parameter
  Fv File type EFI_FV_FILETYPE_SECURITY_CORE must have one and only one Pe or Te section, but 0 Pe/Te section are input
```

The build checks even a file data itself. If we try this code:
```
FILE SEC = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/hello.txt
}
```

We'll get an error
```
GenFv: ERROR 3000: Invalid PeImage
```

So let's use real "*.efi" image for the section data:
```
FILE SEC = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
}
```

This code will finally build.

The important part of all of these experments is that EDKII has many check for the limitations of file types.

# `EFI_FV_FILETYPE_PEI_CORE`

`EFI_FV_FILETYPE_PEI_CORE=0x04` is for PEI Foundation:
```
EFI_FV_FILETYPE_PEI_CORE

The file type EFI_FV_FILETYPE_PEI_CORE denotes the PEI Foundation file. This image is entered upon completion of the SEC phase of a PI Architecture-compliant boot cycle.

This file type is a sectioned file that must be constructed in accordance with the following rules:
• The file must contain one and only one executable section. This section must have one of the following types:
—EFI_SECTION_PE32
—EFI_SECTION_PIC
—EFI_SECTION_TE
• The file must contain no more than one EFI_SECTION_VERSION section.

As long as the above rules are followed, the file may contain other leaf and encapsulations as required/enabled by the platform design.
```

Example:
```
FILE PEI_CORE = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
}
```

And here is we add other sections:
```
FILE PEI_CORE = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
  SECTION BUILD_NUM = 42 VERSION = "MyVersion"
  SECTION UI = "MyUI"
}
```

Although edk2 controls many things, there are many things that currently it doesn't control or do incorrectly. For example currently it is not possible to use `PIC` section instead of `PE32` although specification allows it. And it is possible to provide many `VERSION` sections, although specification forbids it.

# `EFI_FV_FILETYPE_DXE_CORE`

`EFI_FV_FILETYPE_DXE_CORE=0x05` is for DXE Foundation:
```
EFI_FV_FILETYPE_DXE_CORE

The file type EFI_FV_FILETYPE_DXE_CORE denotes the DXE Foundation file. This image is the one entered upon completion of the PEI phase of a UEFI boot cycle.

This file type is a sectioned file that must be constructed in accordance with the following rules:
• The file must contain at least one and only one executable section, which must have a type of EFI_SECTION_PE32. 
• The file must contain no more than one EFI_SECTION_VERSION section.

The sections that are described in the rules above may be optionally encapsulated in compression and/or additional GUIDed sections as required by the platform design.
As long as the above rules are followed, the file may contain other leaf and encapsulation sections as required or enabled by the platform design.
```

Minimal example:
```
FILE DXE_CORE = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
}
```

We can add other sections:
```
FILE DXE_CORE = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
  SECTION BUILD_NUM = 42 VERSION = "MyVersion"
  SECTION UI = "MyUI"
}
```

You can even try to group the necessary section in some form of encapsulation as specification permits it and edk2 supports. For example this syntax would build:
```
FILE DXE_CORE = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION COMPRESS {
    SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
    SECTION BUILD_NUM = 42 VERSION = "MyVersion"
    SECTION UI = "MyUI"
  }
}
```

# `EFI_FV_FILETYPE_PEIM`

`EFI_FV_FILETYPE_PEIM=0x06` is for PEI modules (PEIMs):
```
EFI_FV_FILETYPE_PEIM

The file type EFI_FV_FILETYPE_PEIM denotes a file that is a PEI module (PEIM). A PEI module is dispatched by the PEI Foundation based on its dependencies during execution of the PEI phase. See the Platform Initialization Pre-EFI Initialization Core Interface Specification for details on PEI operation.

This file type is a sectioned file that must be constructed in accordance with the following rules:
• The file must contain one and only one executable section. This section must have one of the following types: 
—EFI_SECTION_PE32
—EFI_SECTION_PIC
—EFI_SECTION_TE
• The file must contain no more than one EFI_SECTION_VERSION section. 
• The file must contain no more than one EFI_SECTION_PEI_DEPEX section.

As long as the above rules are followed, the file may contain other leaf and encapsulation sections as required or enabled by the platform design. Care must be taken to ensure that additional encapsulations do not render the file inaccessible due to execute-in-place requirements.
```
Minimal example:
```
FILE PEIM = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
}
```

# `EFI_FV_FILETYPE_DRIVER`

`EFI_FV_FILETYPE_DRIVER=0x07` is for DXE drivers:
```
EFI_FV_FILETYPE_DRIVER

The file type EFI_FV_FILETYPE_DRIVER denotes a file that contains a PE32 image that can be dispatched by the DXE Dispatcher.

This file type is a sectioned file that must be constructed in accordance with the following rules:
• The file must contain at least one EFI_SECTION_PE32 section. There are no restrictions on encapsulation of this section.
• The file must contain no more than one EFI_SECTION_VERSION section.
• The file must contain no more than one EFI_SECTION_DXE_DEPEX section.

There are no restrictions on the encapsulation of the leaf sections. 

In the event that more than one EFI_SECTION_PE32 section is present in the file, the selection algorithm for choosing which one represents the DXE driver that will be dispatched is defined by the 
LoadImage() boot service, which is used by the DXE Dispatcher.
The file may contain other leaf and encapsulation sections as required or enabled by the platform design.
```

Minimal example:
```
FILE DRIVER = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
}
```

As specification permits it is possible to provide several PE32 sections in this file:
```
FILE DRIVER = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
}
```

# `EFI_FV_FILETYPE_COMBINED_PEIM_DRIVER`

`EFI_FV_FILETYPE_COMBINED_PEIM_DRIVER=0x08` is for combined PEIM/DXE drivers:
```
EFI_FV_FILETYPE_COMBINED_PEIM_DRIVER

The file type EFI_FV_FILETYPE_COMBINED_PEIM_DRIVER denotes a file that contains code suitable for dispatch by the PEI Dispatcher, as well as a PE32 image that can be dispatched by the DXE Dispatcher. It has two uses:
• Enables sharing code between PEI and DXE to reduce firmware storage requirements.
• Enables bundling coupled PEIM/driver pairs in the same file.

This file type is a sectioned file and must follow the intersection of all rules defined for both EFI_FV_FILETYPE_PEIM and EFI_FV_FILETYPE_DRIVER files. This intersection is listed below:
• The file must contain one and only one EFI_SECTION_PE32 section. There are no restrictions on encapsulation of this section; however, care must be taken to ensure any execute-in-place requirements are satisfied.
• The file must not contain more than one EFI_SECTION_DXE_DEPEX section.
• The file must not contain more than one EFI_SECTION_PEI_DEPEX section.
• The file must contain no more than one EFI_SECTION_VERSION section.
The file may contain other leaf and encapsulation sections as required or enabled by the platform design.
```

Minimal example:
```
FILE PEI_DXE_COMBO = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
}
```

# `EFI_FV_FILETYPE_APPLICATION`

`EFI_FV_FILETYPE_APPLICATION=0x09` is for UEFI applications:
```
EFI_FV_FILETYPE_APPLICATION:

The file type EFI_FV_FILETYPE_APPLICATION denotes a file that contains a PE32 image that can be loaded using the UEFI Boot Service LoadImage(). Files of type EFI_FV_FILETYPE_APPLICATION are not dispatched by the DXE Dispatcher.

This file type is a sectioned file that must be constructed in accordance with the following rule:
• The file must contain at least one EFI_SECTION_PE32 section. There are no restrictions on encapsulation of this section.

There are no restrictions on the encapsulation of the leaf section.

In the event that more than one EFI_SECTION_PE32 section is present in the file, the selection algorithm for choosing which one represents the PE32 for the application in question is defined by the LoadImage() boot service. See the Platform Initialization Driver Execution Environment Core Interface Specification for details. 

The file may contain other leaf and encapsulation sections as required or enabled by the platform design.
```

Minimal example:
```
FILE APPLICATION = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
}
```

# `EFI_FV_FILETYPE_MM`

`EFI_FV_FILETYPE_MM=0x0a` files contain a PE32+ image that will be loaded into MMRAM in MM Traditional Mode:
```
EFI_FV_FILETYPE_MM

The file type EFI_FV_FILETYPE_MM denotes a file that contains a PE32+ image that will be loaded into MMRAM in MM Tradition Mode.

This file type is a sectioned file that must be constructed in accordance with the following rules:
• The file must contain at least one EFI_SECTION_PE32 section. There are no restrictions on encapsulation of this section.
• The file must contain no more than one EFI_SECTION_VERSION section.
• The file must contain no more than one EFI_SECTION_MM_DEPEX section.

There are no restrictions on the encapsulation of the leaf sections.

The file may contain other leaf and encapsulation sections as required or enabled by the platform design
```

Minimal example:
```
FILE SMM = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
}
```

# `EFI_FV_FILETYPE_MM_CORE`

`EFI_FV_FILETYPE_MM_CORE=0x0d` file contains MM Foundation that support MM Traditional Mode.
```
EFI_FV_FILETYPE_MM_CORE

The file type EFI_FV_FILETYPE_MM_CORE denotes the MM Foundation file that only supports MM Traditional Mode. This image will be loaded by MM IPL into MMRAM.

This file type is a sectioned file that must be constructed in accordance with the following rules:
• The file must contain at least one and only one executable section, which must have a type of EFI_SECTION_PE32.
• The file must contain no more than one EFI_SECTION_VERSION section.

The sections that are described in the rules above may be optionally encapsulated in compression and/or additional GUIDed sections as required by the platform design.

As long as the above rules are followed, the file may contain other leaf and encapsulation sections as required or enabled by the platform design.
```

Minimal example:
```
FILE SMM_CORE = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
}
```

# `EFI_FV_FILETYPE_MM_STANDALONE`

`EFI_FV_FILETYPE_MM_STANDALONE=0x0E` file contains a PE32+ image that will be loaded into MMRAM in MM Standalone Mode. 
```
EFI_FV_FILETYPE_MM_STANDALONE

The file type EFI_FV_FILETYPE_MM_STANDALONE denotes a file that contains a PE32+ image that will be loaded into SMRAM in SMM Standalone Mode.

This file type is a sectioned file that must be constructed in accordance with the following rules:
• The file must contain at least one EFI_SECTION_PE32 section. There are no restrictions on encapsulation of this section.
• The file must contain no more than one EFI_SECTION_VERSION section.
• The file must contain no more than one EFI_SECTION_MM_DEPEX section.

There are no restrictions on the encapsulation of the leaf sections. In the event that more than one EFI_SECTION_PE32 section is present in the file, the selection algorithm for choosing which one represents the MM driver that will be dispatched is defined by MM Foundation Dispatcher.The file may contain other leaf and encapsulation sections as required or enabled by the platform design.
```

Minimal example:
```
FILE MM_STANDALONE = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
}
```

# `EFI_FV_FILETYPE_MM_CORE_STANDALONE`

`EFI_FV_FILETYPE_MM_CORE_STANDALONE=0x0f` file is for MM Foundation that support MM Tradition Mode and MM Standalone Mode.

Specification defines it like this:
```
EFI_FV_FILETYPE_MM_CORE_STANDALONE

The file type EFI_FV_FILETYPE_SMM_CORE_STANDALONE denotes the MM Foundation file that support MM Traditional Mode and MM Standalone Mode. This image will be loaded by standalone MM IPL into MMRAM.
```

As you can see there no limitations on file sections, therefore something like this will build fine:
```
FILE MM_CORE_STANDALONE = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION RAW = $(WORKDIR)/hello.txt
}
```

# `EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE`

`EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE=0x0B` is for files that contain Firmware volume image (FV)

```
EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE

The file type EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE denotes a file that contains one or more firmware volume images. 

This file type is a sectioned file that must be constructed in accordance with the following rule:
• The file must contain at least one section of type EFI_SECTION_FIRMWARE_VOLUME_IMAGE. There are no restrictions on encapsulation of this section.

The file may contain other leaf and encapsulation sections as required or enabled by the platform design.
```

Here is example:
```
[FD.SimpleImage]
BaseAddress   = 0x0
Size          = 0x1000
ErasePolarity = 1

0x00|0x1000
FV = SimpleVolume


[FV.SimpleVolume]
FvAlignment        = 16

FILE FV_IMAGE = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION FV_IMAGE = SimpleSubVolume
}


[FV.SimpleSubVolume]
FvAlignment        = 16

FILE FREEFORM = dc070116-d211-4ab1-a657-e0b6c64b2643 {
  SECTION RAW = $(WORKDIR)/hello.txt
}
```

Although specification clearly states that the file of such type must contain `SECTION FV_IMAGE` inside, be aware that edk2 doesn't really check it.

# `EFI_FV_FILETYPE_COMBINED_MM_DXE`

`EFI_FV_FILETYPE_COMBINED_MM_DXE=0x0C` is for files that contain PE32+ image that will be dispatched by the DXE Dispatcher and will also be loaded into MMRAM in MM Tradition Mode.
```
EFI_FV_FILETYPE_COMBINED_SMM_DXE

The file type EFI_FV_FILETYPE_COMBINED_MM_DXE denotes a file that contains a PE32+ image that will be dispatched by the DXE Dispatcher and will also be loaded into MMRAM in MM Traditional Mode.

This file type is a sectioned file that must be constructed in accordance with the following rules:
• The file must contain at least one EFI_SECTION_PE32 section. There are no restrictions on encapsulation of this section.
• The file must contain no more than one EFI_SECTION_VERSION section.
• The file must contain no more than one EFI_SECTION_DXE_DEPEX section. This section is ignored when the file is loaded into SMRAM.
• The file must contain no more than one EFI_SECTION_MM_DEPEX section. This section is ignored when the file is dispatched by the DXE Dispatcher.
There are no restrictions on the encapsulation of the leaf sections.

The file may contain other leaf and encapsulation sections as required or enabled by the platform design
```

Currently there is no method in EDK2 to initialize files with such type in FDF.

# `EFI_FV_FILETYPE_OEM_MIN…EFI_FV_FILETYPE_OEM_MAX`

EFI_FV_FILETYPE_OEM_MIN…EFI_FV_FILETYPE_OEM_MAX = 0xC0-0xDF is for OEM File types

Currently there is no method in EDK2 to initialize files with such type in FDF.

# `EFI_FV_FILETYPE_DEBUG_MIN…EFI_FV_FILETYPE_DEBUG_MAX`

EFI_FV_FILETYPE_DEBUG_MIN…EFI_FV_FILETYPE_DEBUG_MAX = 0xE0-0xEF is for Debug/Test File Types

Currently there is no method in EDK2 to initialize files with such type in FDF.

# `EFI_FV_FILETYPE_FFS_MIN…EFI_FV_FILETYPE_FFS_MAX`

`EFI_FV_FILETYPE_FFS_MIN…EFI_FV_FILETYPE_FFS_MAX = 0xF0-0xFF` is for Firmware File System Specific File Types

Currently there is no method in EDK2 to initialize files with such type in FDF.

# `EFI_FV_FILETYPE_FFS_PAD`

`EFI_FV_FILETYPE_FFS_PAD` = 0xF0 is for Pad File For FFS.

```
EFI_FV_FILETYPE_FFS_PAD

A pad file is an FFS-defined file type that is used to pad the location of the file that follows it in the storage file.
```

Currently there is no method in EDK2 to initialize files with such type in FDF.

Although if you remember we've saw files of such type. If you provide `FvNameGuid` to the `FV` would start with a `EFI_FV_FILETYPE_FFS_PAD` file with the provided GUID:
```
[FV.SimpleVolume]
...
FvNameGuid         = 763BED0D-DE9F-48F5-81F1-3E90E1B1A015
...
```

For the example use this FDF:
```
[FD.SimpleImage]
BaseAddress   = 0x0
Size          = 0x1000
ErasePolarity = 1

0x00|0x1000
FV = SimpleVolume

[FV.SimpleVolume]
FvAlignment        = 16
FvNameGuid         = 763BED0D-DE9F-48F5-81F1-3E90E1B1A015

FILE FREEFORM = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION UI = "MyUI"
}
```

Although `GenFfs` is anaware of `EFI_FV_FILETYPE_FFS_PAD` file type. The file generation happens because of `EFI_FV_EXT_HEADER_FILE_NAME` parameter in the `*.inf` for the `GenFv` utility:
```
$ cat /home/aladyshev/tiano/2021/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.inf
[options]
EFI_BASE_ADDRESS = 0x0
EFI_BLOCK_SIZE  = 0x1
EFI_NUM_BLOCKS   =  0x1000
[attributes]
EFI_ERASE_POLARITY   =  1
EFI_FVB2_ALIGNMENT_16 = TRUE
EFI_FV_EXT_HEADER_FILE_NAME = /home/aladyshev/tiano/2021/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.ext
[files]
EFI_FILE_NAME = /home/aladyshev/tiano/2021/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/15c658f6-eb5c-4b8f-b232-d6bd7368a73eSIMPLEVOLUME/15c658f6-eb5c-4b8f-b232-d6bd7368a73e.ffs
```

The file content (without the header) is in this file:
```
$ hexdump /home/aladyshev/tiano/2021/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.ext -C
00000000  0d ed 3b 76 9f de f5 48  81 f1 3e 90 e1 b1 a0 15  |..;v...H..>.....|
00000010  14 00 00 00                                       |....|
00000014
```
Here you can see the GUID that we've provided in `FvNameGuid`. The last 4 bytes `0x00000014` is the size of the data.

You can check `VolInfo` output:
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
Number of Blocks:      0x00001000
Block Length:          0x00000001
Total Volume Size:     0x00001000
============================================================
File Name:        FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF
File Offset:      0x00000048
File Length:      0x0000002C
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0xF0  EFI_FV_FILETYPE_FFS_PAD
============================================================
File Name:        15C658F6-EB5C-4B8F-B232-D6BD7368A73E
File Offset:      0x00000078
File Length:      0x00000026
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_USER_INTERFACE
  Size:  0x0000000E
  String: MyUI
There are a total of 2 files in this FV
```

