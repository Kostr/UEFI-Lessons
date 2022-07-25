# APRIORI

The two main stages of the UEFI boot process is `PEI` and `DXE`. In these stages modules are loaded by the `PEI Foundation` and `DXE Foundation` accordingly. Overall the dispatch process is happening with the respect to the dependency expressions listed in the modules `[Depex]` sections. These expressions put some constrains on the module loading order. But it is important to note that these constrains are not complete.
For example, the dependency expressions can state that some module `A` should be loaded before some module `B`. The module dispatcher would always satisfy this rule, but it is completely random when it would load these two modules, before or after some module `C`. Therefore the dispatch order is not completely determenistic and can differ between boots.

But in some cases determenistic order is required. For example when it is necessary to load some critical modules as soon as possible. It would be tedious to put them in all modules dependency expressions. To help with this issue UEFI defines a special `a priory` file. `PEI Foundation` and `DXE Foundation` are allowed to have one such file each. The data in this file is the array of module (=file) GUIDs.

When the Foundation starts the dispatch process it would first try to load its `a priory` file. The `a priory` files have predefined GUID, so Foundations just checks all file names in the FV consecutively.


You can find defines for these GUIDs in the files:

[https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Guid/AprioriFileName.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Guid/AprioriFileName.h)
```cpp
#define PEI_APRIORI_FILE_NAME_GUID \
  { 0x1b45cc0a, 0x156a, 0x428a, { 0x62, 0XAF, 0x49, 0x86, 0x4d, 0xa0, 0xe6, 0xe6 } }
```

[https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Guid/Apriori.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Guid/Apriori.h)
```cpp
#define EFI_APRIORI_GUID \
 {0xfc510ee7,0xffdc,0x11d4,0xbd,0x41,0x0,0x80,
 0xc7,0x3c,0x88,0x81}
```
Or respective PCDs in the [https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec](https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec)
```
gPeiAprioriFileNameGuid        = { 0x1b45cc0a, 0x156a, 0x428a, { 0XAF, 0x62,  0x49, 0x86, 0x4d, 0xa0, 0xe6, 0xe6 }}
gAprioriGuid                   = { 0xFC510EE7, 0xFFDC, 0x11D4, { 0xBD, 0x41, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81 }}
```

If the `a priory` file is present, the Foundation would start module loading process with the modules corresponding to GUIDs from this file in the order they are written, with no respect to these modules dependency expression itself.

The edk2 has a mechanism to initialize `a priory` files in FDF.

For example let's try to define PEI `a priory` file in our Firmware Volume:
```
[FV.SimpleVolume]
FCFvAlignment        = 16

APRIORI PEI {
  FILE RAW = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
    $(WORKDIR)/hello.txt
  }
}
```
Because of this code the build system would create a file data with `.Apri` extenstion, which would list all necessary GUIDs. In our case it is just one GUID right now:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/1B45CC0A-156A-428A-AF62-49864DA0E6E6SIMPLEVOLUME/1B45CC0A-156A-428A-AF62-49864DA0E6E6SIMPLEVOLUME.Apri -C
00000000  f6 58 c6 15 5c eb 8f 4b  b2 32 d6 bd 73 68 a7 3e  |.X..\..K.2..sh.>|
00000010
```
As the specifciation states the data in the `a priory` file should be in the `RAW` section of the `FREEFORM` file. Therefore next to the `.Apri` file there are `RAW` section file:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/1B45CC0A-156A-428A-AF62-49864DA0E6E6SIMPLEVOLUME/1B45CC0A-156A-428A-AF
62-49864DA0E6E6SIMPLEVOLUME.raw -C
00000000  14 00 00 19 f6 58 c6 15  5c eb 8f 4b b2 32 d6 bd  |.....X..\..K.2..|
00000010  73 68 a7 3e                                       |sh.>|
00000014
```
And a `FREEFORM` file:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/1B45CC0A-156A-428A-AF62-49864DA0E6E6SIMPLEVOLUME/1B45CC0A-156A-428A-AF62-49864DA0E6E6SIMPLEVOLUME.Ffs -C
00000000  0a cc 45 1b 6a 15 8a 42  af 62 49 86 4d a0 e6 e6  |..E.j..B.bI.M...|
00000010  b8 aa 02 00 2c 00 00 07  14 00 00 19 f6 58 c6 15  |....,........X..|
00000020  5c eb 8f 4b b2 32 d6 bd  73 68 a7 3e              |\..K.2..sh.>|
0000002c
```
Here you can see that the `APRIORI PEI` have created a file with the `PEI_APRIORI_FILE_NAME_GUID`.

`VolInfo` utility is able to parse the data inside the `a priori` file:
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
Number of Blocks:      0x00000078
Block Length:          0x00000001
Total Volume Size:     0x00000078
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
15C658F6-EB5C-4B8F-B232-D6BD7368A73E
There are a total of 1 files in this FV
```

Here you can see that the syntax we've used above just creates the `a priory` file, it doesn't add the files itself to the FFS. If we want to do it, we need to perform this explicitly:
```
[FV.SimpleVolume]
FvAlignment        = 16

APRIORI PEI {
  FILE RAW = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
    $(WORKDIR)/hello.txt
  }
}

FILE RAW = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  $(WORKDIR)/hello.txt
}
```

Look at the `VolInfo` output for this build:
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
Number of Blocks:      0x00000098
Block Length:          0x00000001
Total Volume Size:     0x00000098
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
15C658F6-EB5C-4B8F-B232-D6BD7368A73E  /<...>/edk2/$(WORKDIR)/hello.txt
============================================================
File Name:        15C658F6-EB5C-4B8F-B232-D6BD7368A73E  /<...>/$(WORKDIR)/hello.txt
File Offset:      0x00000078
File Length:      0x0000001F
File Attributes:  0x00
File State:       0xF8
        EFI_FILE_DATA_VALID
File Type:        0x01  EFI_FV_FILETYPE_RAW
There are a total of 2 files in this FV
```

Another thing that is important to note, that `a priory` syntax doesn't care about the file contents it just takes the file GUID. Therefore this syntax:
```
APRIORI PEI {
  FILE RAW = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
    $(WORKDIR)/hello.txt
  }
}
```
and this:
```
APRIORI PEI {
  FILE FREEFORM = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
    SECTION COMPRESS PI_STD {
      SECTION RAW = $(WORKDIR)/hello.txt
      SECTION RAW = $(WORKDIR)/hello.txt
      SECTION RAW = $(WORKDIR)/hello.txt
    }
  }
}
```
would produce the same `a priory` file.

Also the information about the file content inside the APRIORI syntax and outside it doesn't even have to match:
```
APRIORI PEI {
  FILE RAW = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
    $(WORKDIR)/hello.txt
  }
}

FILE FREEFORM = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
  SECTION COMPRESS PI_STD {
    SECTION RAW = $(WORKDIR)/hello.txt
    SECTION RAW = $(WORKDIR)/hello.txt
    SECTION RAW = $(WORKDIR)/hello.txt
  }
}
```

Off course if you want to list many GUIDs in the `a priory` file, you just need to add more files inside the `APRIORI` block:
```
APRIORI PEI {
  FILE RAW = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
    $(WORKDIR)/hello.txt
  }
  FILE RAW = d1c69684-8da0-4531-a497-571457fa394a {
    $(WORKDIR)/hello.txt
  }
  FILE RAW = 6bf59546-7fcf-49b3-9993-091737369bd1 {
    $(WORKDIR)/hello.txt
  }
}
```

In this case EDKII just adds more GUIDs to the  `.Apri` file:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/1B45CC0A-156A-428A-AF62-49864DA0E6E6SIMPLEVOLUME/1B45CC0A-156A-428A-AF62-49864DA0E6E6SIMPLEVOLUME.Apri -C
00000000  f6 58 c6 15 5c eb 8f 4b  b2 32 d6 bd 73 68 a7 3e  |.X..\..K.2..sh.>|
00000010  84 96 c6 d1 a0 8d 31 45  a4 97 57 14 57 fa 39 4a  |......1E..W.W.9J|
00000020  46 95 f5 6b cf 7f b3 49  99 93 09 17 37 36 9b d1  |F..k...I....76..|
00000030
```

And `VolInfo` would correctly display the GUIDs in their respective order:
```
PEI APRIORI FILE:
15C658F6-EB5C-4B8F-B232-D6BD7368A73E
D1C69684-8DA0-4531-A497-571457FA394A
6BF59546-7FCF-49B3-9993-091737369BD1
```

# APRIORI DXE

If you want to create DXE `a priory` file, you just need to use `APRIORI DXE {...}` syntax:
```
APRIORI DXE {
  FILE RAW = 15c658f6-eb5c-4b8f-b232-d6bd7368a73e {
    $(WORKDIR)/hello.txt
  }
  FILE RAW = d1c69684-8da0-4531-a497-571457fa394a {
    $(WORKDIR)/hello.txt
  }
  FILE RAW = 6bf59546-7fcf-49b3-9993-091737369bd1 {
    $(WORKDIR)/hello.txt
  }
}
```

This would create the `a priory` "*.Apri" file with the `EFI_APRIORI_GUID` name:
```
$ hexdump Build/UefiLessonsPkg/RELEASE_GCC5/FV/Ffs/FC510EE7-FFDC-11D4-BD41-0080C73C8881SIMPLEVOLUME/FC510EE7-FFDC-11D4-BD41-0080C73C8881SIMPLEVOLUME.Apri -C
00000000  f6 58 c6 15 5c eb 8f 4b  b2 32 d6 bd 73 68 a7 3e  |.X..\..K.2..sh.>|
00000010  84 96 c6 d1 a0 8d 31 45  a4 97 57 14 57 fa 39 4a  |......1E..W.W.9J|
00000020  46 95 f5 6b cf 7f b3 49  99 93 09 17 37 36 9b d1  |F..k...I....76..|
00000030
```

In case you didn't notice the `VolInfo` outputs the type of the `a priori` file along with the GUIDs:
```
DXE APRIORI FILE:
15C658F6-EB5C-4B8F-B232-D6BD7368A73E
D1C69684-8DA0-4531-A497-571457FA394A
6BF59546-7FCF-49B3-9993-091737369BD1
```

