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

Intermidiate file for this section would have `*.ui` extension:
```
Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.ui -C
00000000  0e 00 00 15 4d 00 79 00  55 00 49 00 00 00        |....M.y.U.I...|
0000000e
```

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

Intermidiate file for this section would have `*.ver` extension:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.ver -C
00000000  1a 00 00 14 2a 00 4d 00  79 00 56 00 65 00 72 00  |....*.M.y.V.e.r.|
00000010  73 00 69 00 6f 00 6e 00  00 00                    |s.i.o.n...|
0000001a
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

Here is how `VolInfo` output looks like in this case:
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

Intermidiate file for this section would have `*.guid` extension:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.guid -C
00000000  1b 00 00 18 30 ce 1f cf  81 b1 6a 4d b8 60 35 4c  |....0.....jM.`5L|
00000010  92 2e 5c 3e 68 65 6c 6c  6f 21 0a                 |..\>hello!.|
0000001b
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
`VolInfo` display for the section:
```
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x0000000B
```

The section file extension is "*.pe32":
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.pe32 -C
00000000  0b 00 00 10 68 65 6c 6c  6f 21 0a                 |....hello!.|
0000000b
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

`VolInfo` output for the section:
```
------------------------------------------------------------
  Type:  EFI_SECTION_PIC
  Size:  0x0000000B
```

The section file extension is "*.pic":
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.pic -C
00000000  0b 00 00 11 68 65 6c 6c  6f 21 0a                 |....hello!.|
0000000b
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
`VolInfo` output for the section:
```
------------------------------------------------------------
  Type:  EFI_SECTION_COMPATIBILITY16
  Size:  0x0000000B
```

The section file extension is "*.com16":
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SIMPLEVOLUME/f6ed9cf0-cdc1-40a1-9909-ac6a2f435e23SEC1.com16 -C
00000000  0b 00 00 16 68 65 6c 6c  6f 21 0a                 |....hello!.|
0000000b
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
But if you would use correct file, the `VolInfo` would show you `EFI_SECTION_TE` section.

The section file extension is "*.te":

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

The section file extension for these sections is "*.dpx"

