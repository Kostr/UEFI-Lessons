Let's look a the process how UEFI loads modues from flash.

The Foundation checks its Firmware Volume (FV) and loads modules with specific types. For example, one of the module types for the DXE Foundation is `EFI_FV_FILETYPE_DRIVER=0x07` (DXE driver). After the module load, the DXE Foundation would transfer execution to the module section `EFI_SECTION_PE32=0x10` (PE32+ Executable image). As you might remember the section of this type is mandatory for such type of file by the specification.

How we can add such files to our flash image? For example we can write something like this:
```
FILE DRIVER = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(WORKDIR)/Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
}
```
To remove hardcoded paths we can use various build defines:
```
FILE DRIVER = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION PE32 = $(OUTPUT_DIRECTORY)/$(TARGET)_$(TOOL_CHAIN_TAG)/$(ARCH)/SimplestApp.efi
}
```
But this also is not ideal, if our flash image would need a lot of modules, we would have a lot of this boilerplate code. Which is obviously not good.

Also here we need to provide file type and file sections, and they should correspond to the actual module type. But to know the actual module type we need to consult the module itself. And if we change it one day, we would need to not forget to sync sections in FDF. This interdependency of code is not good.

Finally here we need to provide file GUIDs. This information would pollute every FDF that would include the module.

Luckily there is another method in edk2!

Instead of working with the product of `*.inf` file build (i.e. its `*.efi` file), we can use the `*.inf` file itself:
```
[FV.SimpleVolume]
FvAlignment        = 16

INF UefiLessonsPkg/SimplestApp/SimplestApp.inf
```
This makes perfect sense since the `*.inf` file knows how to compile the module, the type of the module (`MODULE_TYPE`) and its file guid (`FILE_GUID`):
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = SimplestApp
  FILE_GUID                      = 4a298956-fbe0-47fb-ae3a-2d5a0a959a26
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = UefiMain

[Sources]
  SimplestApp.c

[Packages]
  MdePkg/MdePkg.dec

[LibraryClasses]
  UefiApplicationEntryPoint
```

If you try to build flash image with the FDF code we've written above you would get an error:
```
build.py...
 : error F003: Don't Find common rule RULE.COMMON.UEFI_APPLICATION for INF UefiLessonsPkg/SimplestApp/SimplestApp.inf
```

It is because edk2 uses `rule` system to construct FFS files from the module INF's. The error says that our FDF lack a rule for our module (which has a type `UEFI_APPLICATION`).

Let's try to write such rule:
```
[Rule.Common.UEFI_APPLICATION]
  FILE APPLICATION = $(NAMED_GUID) {
    PE32     PE32                    $(INF_OUTPUT)/$(MODULE_NAME).efi
  }
```
Here we say that for every included `*.inf` file with a `MODULE_TYPE = UEFI_APPLICATION` we add `FILE` of type `APPLICATION` to the FFS.

This file would be added with a name `FILE_GUID` from the `*.inf` defines (this is what `$(NAMED_GUID)` means).

The file would include one section `PE32` with a content `$(INF_OUTPUT)/$(MODULE_NAME).efi`, which is a product of a module build. `MODULE_NAME` is derived from the `BASE_NAME` defined in the `*inf` file and the `INF_OUTPUT` is derived from the build method that we use. In this case this would expand to:
```
Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/SimplestApp/SimplestApp/OUTPUT/SimplestApp.efi
```
The section syntax is similar to the way how we've defined sections in a file, except here we don't use the `SECTION` keyword and have written `PE32` two times. This syntax means:
```
<SectionType> <SectionFileType> <File>
```
- `<SectionType>` - this is a standard section type that we've used
- `<SectionFileType>` - this one is tricky, see explanation below
- `<File>` - this is the file path for the section data

`<SectionFileType>` is a parameter for the edk2 build system. Most often it is equal to the `<SectionType>`. Except for the `RAW` section. In this case it can also be `BIN`, `ASL` or `ACPI`. We'll get back to this parameter when we would talk about binary modules. Right now just use the same value as you use for the `<SectionType>`.

With the rule above the image would build and you can check the resulting `FV` with `VolInfo`:
```
$ VolInfo Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -x Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
VolInfo Version 1.0 Build Developer Build based on Revision: Unknown
%36s %511s
ParseGuidBaseNameFile: Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
Signature:        _FVH (4856465F)
Attributes:       40800
       EFI_FVB2_ERASE_POLARITY
       EFI_FVB2_ALIGNMENT_16
Header Length:         0x00000048
File System ID:        8c8ce578-8a3d-4f1c-9935-896185c32dd3
Revision:              0x0002
Number of Blocks:      0x00000428
Block Length:          0x00000001
Total Volume Size:     0x00000428
============================================================
File Name:        4A298956-FBE0-47FB-AE3A-2D5A0A959A26  SimplestApp
File Offset:      0x00000048
File Length:      0x000003DC
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x09  EFI_FV_FILETYPE_APPLICATION
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x000003C4
There are a total of 1 files in this FV
```
Everything is exactly as expected.

If you want to include another `UEFI_APPLICATION` module, you just add another `*.inf` to the `FV`:
```
[FV.SimpleVolume]
FvAlignment        = 16

INF UefiLessonsPkg/SimplestApp/SimplestApp.inf
INF UefiLessonsPkg/HelloWorld/HelloWorld.inf
```

If you check `VolInfo` for this configuration, you would see that now our FV has two files, and each of the files has one PE32 section.
```
$ VolInfo Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -x Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
VolInfo Version 1.0 Build Developer Build based on Revision: Unknown
%36s %511s
ParseGuidBaseNameFile: Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
Signature:        _FVH (4856465F)
Attributes:       40800
       EFI_FVB2_ERASE_POLARITY
       EFI_FVB2_ALIGNMENT_16
Header Length:         0x00000048
File System ID:        8c8ce578-8a3d-4f1c-9935-896185c32dd3
Revision:              0x0002
Number of Blocks:      0x00001B88
Block Length:          0x00000001
Total Volume Size:     0x00001B88
============================================================
File Name:        4A298956-FBE0-47FB-AE3A-2D5A0A959A26  SimplestApp
File Offset:      0x00000048
File Length:      0x000003DC
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x09  EFI_FV_FILETYPE_APPLICATION
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x000003C4
============================================================
File Name:        2E55FA38-F148-42D3-AF90-1BE247323E30  HelloWorld
File Offset:      0x00000428
File Length:      0x0000175C
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x09  EFI_FV_FILETYPE_APPLICATION
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x00001744
There are a total of 2 files in this FV
```

With the rule system we can easily add other sections to the resulting files. For example let's add `UI` section with a `MODULE_NAME` define content to each file resulting from the `UEFI_APPLICATION` module compilation. Here in the rule statements it is also possible to initialize `UI` section in-place as we've used to:
```
[Rule.Common.UEFI_APPLICATION]
  FILE APPLICATION = $(NAMED_GUID) {
    PE32     PE32                    $(INF_OUTPUT)/$(MODULE_NAME).efi
    UI       STRING="$(MODULE_NAME)"
  }
```
In the `VolInfo` output you can see how each of the files now have `EFI_SECTION_USER_INTERFACE` section initialized with the apropriate string:
```
$ VolInfo Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -x Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
VolInfo Version 1.0 Build Developer Build based on Revision: Unknown
%36s %511s
ParseGuidBaseNameFile: Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
Signature:        _FVH (4856465F)
Attributes:       40800
       EFI_FVB2_ERASE_POLARITY
       EFI_FVB2_ALIGNMENT_16
Header Length:         0x00000048
File System ID:        8c8ce578-8a3d-4f1c-9935-896185c32dd3
Revision:              0x0002
Number of Blocks:      0x00001BB8
Block Length:          0x00000001
Total Volume Size:     0x00001BB8
============================================================
File Name:        4A298956-FBE0-47FB-AE3A-2D5A0A959A26  SimplestApp
File Offset:      0x00000048
File Length:      0x000003F8
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x09  EFI_FV_FILETYPE_APPLICATION
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x000003C4
------------------------------------------------------------
  Type:  EFI_SECTION_USER_INTERFACE
  Size:  0x0000001C
  String: SimplestApp
============================================================
File Name:        2E55FA38-F148-42D3-AF90-1BE247323E30  HelloWorld
File Offset:      0x00000440
File Length:      0x00001776
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x09  EFI_FV_FILETYPE_APPLICATION
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x00001744
------------------------------------------------------------
  Type:  EFI_SECTION_USER_INTERFACE
  Size:  0x0000001A
  String: HelloWorld
There are a total of 2 files in this FV
```

Another commonly added section is `VERSION`, so let's add this one too:
```
[Rule.Common.UEFI_APPLICATION]
  FILE APPLICATION = $(NAMED_GUID) {
    PE32     PE32                    $(INF_OUTPUT)/$(MODULE_NAME).efi
    UI       STRING="$(MODULE_NAME)"
    VERSION  STRING="$(INF_VERSION)" BUILD_NUM=$(BUILD_NUMBER)
  }
```

`VolInfo` output in this case:
```
$ VolInfo Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -x Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
VolInfo Version 1.0 Build Developer Build based on Revision: Unknown
%36s %511s
ParseGuidBaseNameFile: Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
Signature:        _FVH (4856465F)
Attributes:       40800
       EFI_FVB2_ERASE_POLARITY
       EFI_FVB2_ALIGNMENT_16
Header Length:         0x00000048
File System ID:        8c8ce578-8a3d-4f1c-9935-896185c32dd3
Revision:              0x0002
Number of Blocks:      0x00001BD8
Block Length:          0x00000001
Total Volume Size:     0x00001BD8
============================================================
File Name:        4A298956-FBE0-47FB-AE3A-2D5A0A959A26  SimplestApp
File Offset:      0x00000048
File Length:      0x00000406
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x09  EFI_FV_FILETYPE_APPLICATION
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x000003C4
------------------------------------------------------------
  Type:  EFI_SECTION_USER_INTERFACE
  Size:  0x0000001C
  String: SimplestApp
------------------------------------------------------------
  Type:  EFI_SECTION_VERSION
  Size:  0x0000000E
  Build Number:  0x0000
  Version String:  1.0
============================================================
File Name:        2E55FA38-F148-42D3-AF90-1BE247323E30  HelloWorld
File Offset:      0x00000450
File Length:      0x00001786
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x09  EFI_FV_FILETYPE_APPLICATION
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x00001744
------------------------------------------------------------
  Type:  EFI_SECTION_USER_INTERFACE
  Size:  0x0000001A
  String: HelloWorld
------------------------------------------------------------
  Type:  EFI_SECTION_VERSION
  Size:  0x0000000E
  Build Number:  0x0000
  Version String:  1.0
There are a total of 2 files in this FV
```

# Arch parameter in the rule section name

We've defined our rule as
```
[Rule.Common.UEFI_APPLICATION]
```
The word after the first period means target architecture. In this case we defined it as `Common` which means any architecture.

You can verify that our build would be fine with such rule:
```
[Rule.X64.UEFI_APPLICATION]
```
If you want to you could provide completely separate rule for `X32` architecture.

# Overriding rules

It is possible to override rules for particular modules. In this case you need to provide an overriden rule. If you remember earlier `SimplestApp.inf` and `HelloWorld.inf` modules have generated similarly constructed files. Let's provide overriden rule and use it for one of these modules:
```
[FV.SimpleVolume]
FvAlignment        = 16

INF RuleOverride=MyCompressed UefiLessonsPkg/SimplestApp/SimplestApp.inf
INF UefiLessonsPkg/HelloWorld/HelloWorld.inf

[Rule.Common.UEFI_APPLICATION]
  FILE APPLICATION = $(NAMED_GUID) {
    PE32     PE32                    $(INF_OUTPUT)/$(MODULE_NAME).efi
    UI       STRING="$(MODULE_NAME)"
    VERSION  STRING="$(INF_VERSION)" BUILD_NUM=$(BUILD_NUMBER)
  }

[Rule.Common.UEFI_APPLICATION.MyCompressed]
  FILE APPLICATION = $(NAMED_GUID) {
    GUIDED EE4E5898-3914-4259-9D6E-DC7BD79403CF {                        # LzmaCompress
      PE32     PE32                    $(INF_OUTPUT)/$(MODULE_NAME).efi
      VERSION  STRING="$(INF_VERSION)" BUILD_NUM=$(BUILD_NUMBER)
    }
    UI       STRING="$(MODULE_NAME)"
  }
```

Now the modules generate differently structured files:
```
$ VolInfo Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -x Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
VolInfo Version 1.0 Build Developer Build based on Revision: Unknown
%36s %511s
ParseGuidBaseNameFile: Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
Signature:        _FVH (4856465F)
Attributes:       40800
       EFI_FVB2_ERASE_POLARITY
       EFI_FVB2_ALIGNMENT_16
Header Length:         0x00000048
File System ID:        8c8ce578-8a3d-4f1c-9935-896185c32dd3
Revision:              0x0002
Number of Blocks:      0x00001970
Block Length:          0x00000001
Total Volume Size:     0x00001970
============================================================
File Name:        4A298956-FBE0-47FB-AE3A-2D5A0A959A26  SimplestApp
File Offset:      0x00000048
File Length:      0x000001A0
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x09  EFI_FV_FILETYPE_APPLICATION
------------------------------------------------------------
  Type:  EFI_SECTION_GUID_DEFINED
  Size:  0x0000016B
  SectionDefinitionGuid:  ee4e5898-3914-4259-9d6e-dc7bd79403cf

  DataOffset:             0x0018
  Attributes:             0x0001
Decoding
/------------ Encapsulation section start -----------------\
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x000003C4
------------------------------------------------------------
  Type:  EFI_SECTION_VERSION
  Size:  0x0000000E
  Build Number:  0x0000
  Version String:  1.0
\------------ Encapsulation section end -------------------/
------------------------------------------------------------
  Type:  EFI_SECTION_USER_INTERFACE
  Size:  0x0000001C
  String: SimplestApp
============================================================
File Name:        2E55FA38-F148-42D3-AF90-1BE247323E30  HelloWorld
File Offset:      0x000001E8
File Length:      0x00001786
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x09  EFI_FV_FILETYPE_APPLICATION
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x00001744
------------------------------------------------------------
  Type:  EFI_SECTION_USER_INTERFACE
  Size:  0x0000001A
  String: HelloWorld
------------------------------------------------------------
  Type:  EFI_SECTION_VERSION
  Size:  0x0000000E
  Build Number:  0x0000
  Version String:  1.0
There are a total of 2 files in this FV
```

# APRIORI

If you want to add some modules to the `a priori` file, you just add the module INFs to the `APRIORI` blocks:
```
APRIORI PEI {
  INF UefiLessonsPkg/MemoryInfo/MemoryInfo.inf
}

APRIORI DXE {
  INF UefiLessonsPkg/SimplestApp/SimplestApp.inf
  INF UefiLessonsPkg/HelloWorld/HelloWorld.inf
}
```
`VolInfo`:
```
$ VolInfo Build/UefiLessonsPkg/RELEASE_GCC5/FV/SIMPLEVOLUME.Fv -x Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
VolInfo Version 1.0 Build Developer Build based on Revision: Unknown
%36s %511s
ParseGuidBaseNameFile: Build/UefiLessonsPkg/RELEASE_GCC5/FV/Guid.xref
Signature:        _FVH (4856465F)
Attributes:       40800
       EFI_FVB2_ERASE_POLARITY
       EFI_FVB2_ALIGNMENT_16
Header Length:         0x00000048
File System ID:        8c8ce578-8a3d-4f1c-9935-896185c32dd3
Revision:              0x0002
Number of Blocks:      0x000000B8
Block Length:          0x00000001
Total Volume Size:     0x000000B8
============================================================
File Name:        1B45CC0A-156A-428A-AF62-49864DA0E6E6
File Offset:      0x00000048
File Length:      0x0000002C
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x00000014

PEI APRIORI FILE:
D2CE1D65-603E-4B48-B3E1-FE65D5D0BCA8  MemoryInfo
============================================================
File Name:        FC510EE7-FFDC-11D4-BD41-0080C73C8881
File Offset:      0x00000078
File Length:      0x0000003C
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x00000024

DXE APRIORI FILE:
4A298956-FBE0-47FB-AE3A-2D5A0A959A26  SimplestApp
2E55FA38-F148-42D3-AF90-1BE247323E30  HelloWorld
There are a total of 2 files in this FV
```

Here you can see, that as before, defining files inside the `APRIORI` blocks doesn't add the files itself to the FFS.

Also this syntax doesn't even need any rules defined. But off course you'll need them when you would add the modules itself.

# Links

- [https://edk2-docs.gitbook.io/edk-ii-fdf-specification/3_edk_ii_fdf_file_format/39_-rule-_sections](https://edk2-docs.gitbook.io/edk-ii-fdf-specification/3_edk_ii_fdf_file_format/39_-rule-_sections)

