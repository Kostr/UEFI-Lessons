Now let's explore `PatchableInModule` PCD.

Add PCD to DEC file UefiLessonsPkg/UefiLessonsPkg.dec:
```
[PcdsPatchableInModule]
  gUefiLessonsPkgTokenSpaceGuid.PcdMyPatchableVar32|0x31313131|UINT32|0x00000004
```
Populate it to INF UefiLessonsPkg/PCDLesson/PCDLesson.inf:
```
[PatchPcd]
  gUefiLessonsPkgTokenSpaceGuid.PcdMyPatchableVar32
```

To get a value of PCD in a *.c file either `PatchPcdGet` or generic `PcdGet` should be used.

Let's test both methods:
```
Print(L"PcdMyPatchableVar32=%d\n", PatchPcdGet32(PcdMyPatchableVar32));
Print(L"PcdMyPatchableVar32=%d\n", PcdGet32(PcdMyPatchableVar32));
```

Now build and look to AutoGen files:

Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
```
#define _PCD_TOKEN_PcdMyPatchableVar32  0U
#define _PCD_PATCHABLE_VALUE_PcdMyPatchableVar32  ((UINT32)0x31313131U)
extern volatile   UINT32  _gPcd_BinaryPatch_PcdMyPatchableVar32;
#define _PCD_GET_MODE_32_PcdMyPatchableVar32  _gPcd_BinaryPatch_PcdMyPatchableVar32
#define _PCD_PATCHABLE_PcdMyPatchableVar32_SIZE 4
#define _PCD_GET_MODE_SIZE_PcdMyPatchableVar32  _gPcd_BinaryPatch_Size_PcdMyPatchableVar32
extern UINTN _gPcd_BinaryPatch_Size_PcdMyPatchableVar32;
#define _PCD_SET_MODE_32_PcdMyPatchableVar32(Value)  (_gPcd_BinaryPatch_PcdMyPatchableVar32 = (Value))
#define _PCD_SET_MODE_32_S_PcdMyPatchableVar32(Value)  ((_gPcd_BinaryPatch_PcdMyPatchableVar32 = (Value)), RETURN_SUCCESS)
```
Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.c
```
volatile  UINT32 _gPcd_BinaryPatch_PcdMyPatchableVar32 = _PCD_PATCHABLE_VALUE_PcdMyPatchableVar32;
GLOBAL_REMOVE_IF_UNREFERENCED UINTN _gPcd_BinaryPatch_Size_PcdMyPatchableVar32 = 4;
```

So in this case our call to `PcdGet32(PcdMyPatchableVar32)` would be expanded to:
```
PcdGet32(PcdMyPatchableVar32) --> _PCD_GET_MODE_32_PcdMyPatchableVar32 --> _gPcd_BinaryPatch_PcdMyPatchableVar32
```
This variable would be assigned in `AutoGen.c`:
```
volatile  UINT32 _gPcd_BinaryPatch_PcdMyPatchableVar32 = ((UINT32)0x31313131U);
```
So the main difference from the FixedAtBuild and FeatureFlag PCDs is that variable defined as `volatile` and set functions aren't blocked.

Let's try to set our PCD then. As with get, there are two possibilities. You can use either `PatchPcdSet<Type>` or generic `PcdSet<Type>S`.

Keep in mind that `PcdSet32S` unravels to a macro returning a value.
https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h:
```
#define PcdSet32S(TokenName, Value)         _PCD_SET_MODE_32_S_##TokenName    ((Value))
```
Therefore:
```
PcdSet32S(PcdMyPatchableVar32, 44) -->  _PCD_SET_MODE_32_S_PcdMyPatchableVar32 ((Value)) --> ((_gPcd_BinaryPatch_PcdMyPatchableVar32 = (Value)), RETURN_SUCCESS)
```

So if you don't want such error:
```
/home/kostr/tiano/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h:108:106: error: right-hand operand of comma expression has no effect [-Werror=unused-value]
  108 | #define _PCD_SET_MODE_32_S_PcdMyPatchableVar32(Value)  ((_gPcd_BinaryPatch_PcdMyPatchableVar32 = (Value)), RETURN_SUCCESS)
      |                                                        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~.
```
You should use something like this:
```
RETURN_STATUS PcdStatus = PcdSet32S(PcdMyPatchableVar32, 44);
Print(L"PcdStatus=%r\n", PcdStatus);
```

Alternatively you can use `PatchPcdSet32` which don't require such thing as it is defined as follows:
```
#define PatchPcdSet32(TokenName, Value)     (_gPcd_BinaryPatch_##TokenName = (Value))
```

Test both set methods in app code:
```
Print(L"PcdMyPatchableVar32=0x%x\n", PcdGet32(PcdMyPatchableVar32));
RETURN_STATUS PcdStatus = PcdSet32S(PcdMyPatchableVar32, 44);
Print(L"PcdStatus=%r\n", PcdStatus);
Print(L"PcdMyPatchableVar32=%d\n", PcdGet32(PcdMyPatchableVar32));
PatchPcdSet32(PcdMyPatchableVar32, 45);
Print(L"PcdMyPatchableVar32=%d\n", PatchPcdGet32(PcdMyPatchableVar32));
```


Now if you execute our app under OVMF prints for patchable PCDs would output:
```
PcdMyPatchableVar32=0x31313131
PcdStatus=Success
PcdMyPatchableVar32=44
PcdMyPatchableVar32=45
```

_________________________________

In case you've wondered why I've assigned a hex value for PCD default value or why this PCD type is named `PatchableInModule` this section is for you.

This PCD type is named like that because the value of this PCD can be changed in a binary PE/COFF image (*.efi file). To do this two utilities are used:
- `GenPatchPcdTable` - this tool is used to get the patchable PCD offset for the EFI image by parsing the map file
- `PatchPcdValue` - this tool is used to actually patch PCD value

You can download rtf manuals for these utilities from edk2 repo:
-https://github.com/tianocore/edk2/blob/master/BaseTools/UserManuals/GenPatchPcdTable_Utility_Man_Page.rtf
-https://github.com/tianocore/edk2/blob/master/BaseTools/UserManuals/PatchPcdValue_Utility_Man_Page.rtf

Let's start with `GenPatchPcdTable`. First check out help for this tool:
```
$ ./BaseTools/BinWrappers/PosixLike/GenPatchPcdTable -h
Usage: GenPatchPcdTable.py -m <MapFile> -e <EfiFile> -o <OutFile>

Copyright (c) 2008 - 2018, Intel Corporation. All rights reserved.

Options:
  --version             show program's version number and exit
  -h, --help            show this help message and exit
  -m MAPFILE, --mapfile=MAPFILE
                        Absolute path of module map file.
  -e EFIFILE, --efifile=EFIFILE
                        Absolute path of EFI binary file.
  -o OUTFILE, --outputfile=OUTFILE
                        Absolute path of output file to store the got
                        patchable PCD table.
```
Now let's create a `PatchPcdTable` for our app:

For the *.efi file we can use one of:
```
Build/UefiLessonsPkg/RELEASE_GCC5/X64/PCDLesson.efi 
Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/OUTPUT/PCDLesson.efi
Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.efi
```
For the *.map file we can use one of:
```
Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.map
Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/OUTPUT/PCDLesson.map
```
In case you wonder how our PCD can be found in a map file, execute:
```
$ grep MyPatchableVar32 Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.map -A2
 .data._gPcd_BinaryPatch_PcdMyPatchableVar32
                0x0000000000001920        0x4 /tmp/PCDLesson.dll.m6t8YL.ltrans0.ltrans.o
 *fill*         0x0000000000001924        0xc
```
So as you can see the default value for our PCD is placed under 0x1920 offset.

Now let's execute `GenPatchPcdTable` and create a file `PCDLessonPatchPcdTable` in app Build DEBUG folder:
```
./BaseTools/BinWrappers/PosixLike/GenPatchPcdTable \
  -m Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.map \
  -e Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.efi \
  -o Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLessonPatchPcdTable
```

Checkout the created `PCDLessonPatchPcdTable` file.
```
$ cat Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLessonPatchPcdTable
PCD Name                       Offset    Section Name
PcdMyPatchableVar32            0x1920     .data
```
As you can see it has the same offset that we've seen in a map file.

Now let's get a final look at our default PCD value in an *.elf file:
```
hexdump -s 0x1920 Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.efi -n 4
0001920 3131 3131
0001924
```

Checkout help for the `PatchPcdValue` tool:
```
$ ./BaseTools/BinWrappers/PosixLike/PatchPcdValue -h
Usage: PatchPcdValue.py -f Offset -u Value -t Type [-s MaxSize] <input_file>

Copyright (c) 2010 - 2018, Intel Corporation. All rights reserved.

Options:
  -f PCDOFFSET, --offset=PCDOFFSET
                        Start offset to the image is used to store PCD value.
  -u PCDVALUE, --value=PCDVALUE
                        PCD value will be updated into the image.
  -t PCDTYPENAME, --type=PCDTYPENAME
                        The name of PCD data type may be one of VOID*,BOOLEAN,
                        UINT8, UINT16, UINT32, UINT64.
  -s PCDMAXSIZE, --maxsize=PCDMAXSIZE
                        Max size of data buffer is taken by PCD value.It must
                        be set when PCD type is VOID*.
  -v, --verbose         Run verbosely
  -d LOGLEVEL, --debug=LOGLEVEL
                        Run with debug information
  -q, --quiet           Run quietly
  -?                    show this help message and exit
  --version             show program's version number and exit
  -h, --help            show this help message and exit
```

Now use it to patch our PCD in an *.elf file:
```
./BaseTools/BinWrappers/PosixLike/PatchPcdValue \
  --offset=0x1920 \
  --value=0xDEADDEAD \
  --type=UINT32 \
  Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.efi
```
Look at hexdump again:
```
$ hexdump -s 0x1920 Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.efi -n 4
0001920 dead dead
0001924
```
We've successfully changed our PCD in a binary!

Now if you execute our app under OVMF prints for patchable PCDs would output:
```
PcdMyPatchableVar32=0xDEADDEAD
PcdStatus=Success
PcdMyPatchableVar32=44
PcdMyPatchableVar32=45
```

