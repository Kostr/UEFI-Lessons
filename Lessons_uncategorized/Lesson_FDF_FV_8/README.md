Sometimes we might want to put to the FFS files that aren't the result of the edk2 build. Off course we can do it manually via the `FILE = <...>` statement, but it is also possible to do it via the INF file. As with compiled modules this keeps code more structured and clean.

The files that we want to push to the FFS need to be listed in the INF module `[Binaries]` section.

Let's try to create INF file for the `Shell.efi` image. First build the `*.efi` file:
```
build --platform=ShellPkg/ShellPkg.dsc --module=ShellPkg/Application/Shell/Shell.inf --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

Now create a folder `BinaryModule` for our new module and put newly generated file into that folder:
```
mkdir UefiLessonsPkg/BinaryModule/
cp Build/Shell/RELEASE_GCC5/X64/ShellPkg/Application/Shell/Shell/OUTPUT/Shell.efi UefiLessonsPkg/BinaryModule/
```

Create INF file:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = BinaryModule
  FILE_GUID                      = a580fb82-9d1f-480b-ba42-17273d861c95
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0

[Binaries]
  PE32|Shell.efi
```

Here we don't need the `ENTRY_POINT` define that we've always used in our modules. And `[Packages]` and `[LibraryClasses]` sections are also no longer necessary.
However you might need `[Depex]` section in a you binary module. But right now we leave things as they are.

The format of the statements inside the `[Binaries]` section is:
```
<SectionFileType>|<FilePath>[|Target]
```
- `<SectionFileType>` - remember how there are usually two words for sections inside the rule statements? This is the second word
- `<FilePath>` - path to the file to include
- `<Target>` - you can use `DEBUG/RELEASE/NOOPT/*` as a taget, or you can omit this part alltogether

To build the binary module along with the ordinary rule section:
```
[Rule.<ARCH>.<MODULE_TYPE>]
```
You need to define another rule section:
```
[Rule.<ARCH>.<MODULE_TYPE>.BINARY]
```

So in our case we need to add `[Rule.Common.UEFI_APPLICATION.BINARY]` section along with the old one `[Rule.Common.UEFI_APPLICATION]`.

`UefiLessonsPkg/UefiLessonsPkg.fdf`:
```
[FV.SimpleVolume]
FvAlignment        = 16

INF UefiLessonsPkg/BinaryModule/BinaryModule.inf

[Rule.Common.UEFI_APPLICATION]
  FILE APPLICATION = $(NAMED_GUID) {
    PE32     PE32                    $(INF_OUTPUT)/$(MODULE_NAME).efi
    UI       STRING="$(MODULE_NAME)"
    VERSION  STRING="$(INF_VERSION)" BUILD_NUM=$(BUILD_NUMBER)
  }

[Rule.Common.UEFI_APPLICATION.BINARY]
  FILE FREEFORM = $(NAMED_GUID) {
    PE32     PE32                    |.efi
    UI       STRING="$(MODULE_NAME)"
    VERSION  STRING="$(INF_VERSION)" BUILD_NUM=$(BUILD_NUMBER)
  }
```

To be certain that the correct rule is used in the `BINARY` rule we create `FILE FREEFORM` opposed to the `FILE APPLICATION` in the standard rule.

In the binary rule statements everything is the same except the `PE32` statement. Here instead of an actual file we need to write file extension in a form `|.<extension>`

As we don't need to compile our module, we don't need to add it to the `[Components]` section of our DSC file. And there wouldn't be any folder like `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/BinaryModule/` after the compilation. Only the `Ffs` folder would be created:
```
$ find Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/a580fb82-9d1f-480b-ba42-17273d861c95BinaryModule/
Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/a580fb82-9d1f-480b-ba42-17273d861c95BinaryModule/
Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/a580fb82-9d1f-480b-ba42-17273d861c95BinaryModule/a580fb82-9d1f-480b-ba42-17273d861c95SEC2.ui
Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/a580fb82-9d1f-480b-ba42-17273d861c95BinaryModule/a580fb82-9d1f-480b-ba42-17273d861c95SEC3.ver.txt
Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/a580fb82-9d1f-480b-ba42-17273d861c95BinaryModule/a580fb82-9d1f-480b-ba42-17273d861c95.ffs
Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/a580fb82-9d1f-480b-ba42-17273d861c95BinaryModule/a580fb82-9d1f-480b-ba42-17273d861c95SEC1.1.pe32.txt
Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/a580fb82-9d1f-480b-ba42-17273d861c95BinaryModule/a580fb82-9d1f-480b-ba42-17273d861c95SEC3.ver
Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/a580fb82-9d1f-480b-ba42-17273d861c95BinaryModule/a580fb82-9d1f-480b-ba42-17273d861c95.ffs.txt
Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/a580fb82-9d1f-480b-ba42-17273d861c95BinaryModule/a580fb82-9d1f-480b-ba42-17273d861c95SEC1.1.pe32
```
And here is the output of `VolInfo`:
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
Number of Blocks:      0x000E2ED8
Block Length:          0x00000001
Total Volume Size:     0x000E2ED8
============================================================
File Name:        A580FB82-9D1F-480B-BA42-17273D861C95  BinaryModule
File Offset:      0x00000048
File Length:      0x000E2E8A
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x000E2E44
------------------------------------------------------------
  Type:  EFI_SECTION_USER_INTERFACE
  Size:  0x0000001E
  String: BinaryModule
------------------------------------------------------------
  Type:  EFI_SECTION_VERSION
  Size:  0x0000000E
  Build Number:  0x0000
  Version String:  1.0
There are a total of 1 files in this FV
```

# Adding other sections

Let's try to write other sections to the file. Create file "hello.txt":
```
echo "hello" > UefiLessonsPkg/BinaryModule/hello.txt
```
And add it as `RAW` to the `[Binaries]` section:
```
[Binaries]
  PE32|Shell.efi
  RAW|hello.txt
```

If we perform build right now, nothing would change.

We need to modify our rule to include `.txt` files:
```
[Rule.Common.UEFI_APPLICATION.BINARY]
  FILE FREEFORM = $(NAMED_GUID) {
    PE32     PE32                    |.efi
    UI       STRING="$(MODULE_NAME)"
    VERSION  STRING="$(INF_VERSION)" BUILD_NUM=$(BUILD_NUMBER)
    RAW RAW |.txt
  }
```
With this rule the new section would be added to the file:
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
Number of Blocks:      0x000E2EE0
Block Length:          0x00000001
Total Volume Size:     0x000E2EE0
============================================================
File Name:        A580FB82-9D1F-480B-BA42-17273D861C95  BinaryModule
File Offset:      0x00000048
File Length:      0x000E2E96
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x000E2E44
------------------------------------------------------------
  Type:  EFI_SECTION_USER_INTERFACE
  Size:  0x0000001E
  String: BinaryModule
------------------------------------------------------------
  Type:  EFI_SECTION_VERSION
  Size:  0x0000000E
  Build Number:  0x0000
  Version String:  1.0
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000A
There are a total of 1 files in this FV
```
Let's try to add another `.txt` file to our module. Create file:
```
$ echo "world!" > UefiLessonsPkg/BinaryModule/world.txt
```
And include it to the `[Binaries]` section:
```
[Binaries]
  PE32|Shell.efi
  RAW|hello.txt
  RAW|world.txt
```
In this case two `RAW` sections would be at the end of our file:
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
Number of Blocks:      0x000E2EF0
Block Length:          0x00000001
Total Volume Size:     0x000E2EF0
============================================================
File Name:        A580FB82-9D1F-480B-BA42-17273D861C95  BinaryModule
File Offset:      0x00000048
File Length:      0x000E2EA3
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x000E2E44
------------------------------------------------------------
  Type:  EFI_SECTION_USER_INTERFACE
  Size:  0x0000001E
  String: BinaryModule
------------------------------------------------------------
  Type:  EFI_SECTION_VERSION
  Size:  0x0000000E
  Build Number:  0x0000
  Version String:  1.0
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000A
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
There are a total of 1 files in this FV
```

The `hello` section is the one that has a size 0x0000000A, and `world!` section is the one that has a size `0x0000000B` as it is one symbol longer.

You can see both strings if you look at the ffs file with `hexdump`:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/a580fb82-9d1f-480b-ba42-17273d861c95BinaryModule/a580fb82-9d1f-480b-ba42-17273d861c95.ffs -C | tail
000e2e20  d8 ab e8 ab 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000e2e30  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
000e2e50  00 00 00 00 00 00 00 00  00 00 00 00 1e 00 00 15  |................|
000e2e60  42 00 69 00 6e 00 61 00  72 00 79 00 4d 00 6f 00  |B.i.n.a.r.y.M.o.|
000e2e70  64 00 75 00 6c 00 65 00  00 00 00 00 0e 00 00 14  |d.u.l.e.........|
000e2e80  00 00 31 00 2e 00 30 00  00 00 00 00 0a 00 00 19  |..1...0.........|
000e2e90  68 65 6c 6c 6f 0a 00 00  0b 00 00 19 77 6f 72 6c  |hello.......worl|
000e2ea0  64 21 0a                                          |d!.|
000e2ea3
```

# Many sections of the same type in one rule

What whould be if you put another `RAW` statement to the file?
```
[Rule.Common.UEFI_APPLICATION.BINARY]
  FILE FREEFORM = $(NAMED_GUID) {
    RAW RAW |.txt
    PE32     PE32                    |.efi
    UI       STRING="$(MODULE_NAME)"
    VERSION  STRING="$(INF_VERSION)" BUILD_NUM=$(BUILD_NUMBER)
    RAW RAW |.txt
  }
```

Each statement would generate 2 `RAW` sections (`hello` and `world!`) in this case:
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
Number of Blocks:      0x000E2F08
Block Length:          0x00000001
Total Volume Size:     0x000E2F08
============================================================
File Name:        A580FB82-9D1F-480B-BA42-17273D861C95  BinaryModule
File Offset:      0x00000048
File Length:      0x000E2EBB
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000A
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x000E2E44
------------------------------------------------------------
  Type:  EFI_SECTION_USER_INTERFACE
  Size:  0x0000001E
  String: BinaryModule
------------------------------------------------------------
  Type:  EFI_SECTION_VERSION
  Size:  0x0000000E
  Build Number:  0x0000
  Version String:  1.0
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000A
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000B
There are a total of 1 files in this FV
```

# `SectionFileType` for `RAW` section

Remember how I told you that `RAW` section can have several `SectionFileType's`: `RAW`/`BIN`/`ASL`/`ACPI`. You can try to use them for our files:
```
[Binaries]
  PE32|Shell.efi
  ASL|hello.txt
  ACPI|world.txt
```
Off course our `*.txt` aren't connected to the ACPI in any way, but let's just do it as an experiment.

Modify rule statement like this:
```
[Rule.Common.UEFI_APPLICATION.BINARY]
  FILE FREEFORM = $(NAMED_GUID) {
    PE32     PE32                    |.efi
    UI       STRING="$(MODULE_NAME)"
    VERSION  STRING="$(INF_VERSION)" BUILD_NUM=$(BUILD_NUMBER)
    RAW ASL |.txt
  }
```
Now build system would include only the file `ASL|hello.txt`, but not `ACPI|world.txt` as the rule works only with `ASL` `SectionFileType's`. For the proof look at the `VolInfo` output:
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
Number of Blocks:      0x000E2EE0
Block Length:          0x00000001
Total Volume Size:     0x000E2EE0
============================================================
File Name:        A580FB82-9D1F-480B-BA42-17273D861C95  BinaryModule
File Offset:      0x00000048
File Length:      0x000E2E96
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x02  EFI_FV_FILETYPE_FREEFORM
------------------------------------------------------------
  Type:  EFI_SECTION_PE32
  Size:  0x000E2E44
------------------------------------------------------------
  Type:  EFI_SECTION_USER_INTERFACE
  Size:  0x0000001E
  String: BinaryModule
------------------------------------------------------------
  Type:  EFI_SECTION_VERSION
  Size:  0x0000000E
  Build Number:  0x0000
  Version String:  1.0
------------------------------------------------------------
  Type:  EFI_SECTION_RAW
  Size:  0x0000000A
There are a total of 1 files in this FV
```

One final thing to note: there is no difference in a flash content from an actual `SectionFileType` that you use in `[Binaries]` section `RAW`/`BIN`/`ASL`/`ACPI`. There is no difference between `ASL|hello.txt` or `BIN|hello.txt` as long as the file is included in the rule.

# `FILE RAW =` in the rule

`BINARY` rules also allow to add RAW files to the filesystem. Here is an example:
```
[Rule.Common.UEFI_APPLICATION.BINARY]
  FILE RAW = $(NAMED_GUID) {
                          |.efi
  }
```
This would produce following `VolInfo` output:
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
Number of Blocks:      0x000E2EA0
Block Length:          0x00000001
Total Volume Size:     0x000E2EA0
============================================================
File Name:        A580FB82-9D1F-480B-BA42-17273D861C95  BinaryModule
File Offset:      0x00000048
File Length:      0x000E2E58
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x01  EFI_FV_FILETYPE_RAW
There are a total of 1 files in this FV
```

