Now let's explore `PatchableInModule` PCD class.

Add new PCD to DEC file `UefiLessonsPkg/UefiLessonsPkg.dec` under the `[PcdsPatchableInModule]` section:
```
[PcdsPatchableInModule]
  gUefiLessonsPkgTokenSpaceGuid.PcdPatchableInt32|0x31313131|UINT32|0xFCDA11B5
```

Populate it to INF `UefiLessonsPkg/PCDLesson/PCDLesson.inf`:
```
[PatchPcd]
  gUefiLessonsPkgTokenSpaceGuid.PcdPatchableInt32
```

To get a value of PCD in a *.c file either `PatchPcdGet` or generic `PcdGet` should be used.

Let's test both methods:
```
Print(L"PcdPatchableInt32=0x%x\n", PatchPcdGet32(PcdPatchableInt32));
Print(L"PcdPatchableInt32=0x%x\n", PcdGet32(PcdPatchableInt32));
```

Now build and look to AutoGen files.

`Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h`:
```
#define _PCD_TOKEN_PcdPatchableInt32  0U
#define _PCD_PATCHABLE_VALUE_PcdPatchableInt32  ((UINT32)0x31313131U)
extern volatile   UINT32  _gPcd_BinaryPatch_PcdPatchableInt32;
#define _PCD_GET_MODE_32_PcdPatchableInt32  _gPcd_BinaryPatch_PcdPatchableInt32
#define _PCD_PATCHABLE_PcdPatchableInt32_SIZE 4
#define _PCD_GET_MODE_SIZE_PcdPatchableInt32  _gPcd_BinaryPatch_Size_PcdPatchableInt32
extern UINTN _gPcd_BinaryPatch_Size_PcdPatchableInt32;
#define _PCD_SET_MODE_32_PcdPatchableInt32(Value)  (_gPcd_BinaryPatch_PcdPatchableInt32 = (Value))
#define _PCD_SET_MODE_32_S_PcdPatchableInt32(Value)  ((_gPcd_BinaryPatch_PcdPatchableInt32 = (Value)), RETURN_SUCCESS)
```

`Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.c`:
```
volatile  UINT32 _gPcd_BinaryPatch_PcdPatchableInt32 = _PCD_PATCHABLE_VALUE_PcdPatchableInt32;
GLOBAL_REMOVE_IF_UNREFERENCED UINTN _gPcd_BinaryPatch_Size_PcdPatchableInt32 = 4;
```

According to the [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h):
```
#define PatchPcdGet32(TokenName)  _gPcd_BinaryPatch_##TokenName
...
#define PcdGet32(TokenName)  _PCD_GET_MODE_32_##TokenName
```

If you unravel preprocessor code, you'll see that both calls translate to the same variable:
```
PatchPcdGet32(PcdPatchableInt32) -> _gPcd_BinaryPatch_PcdPatchableInt32

PcdGet32(PcdPatchableInt32) -> _PCD_GET_MODE_32_PcdPatchableInt32 ->  _gPcd_BinaryPatch_PcdPatchableInt32
```

This is a `volatile` variable that is assigned in the `AutoGen.c`:
```
volatile UINT32  _gPcd_BinaryPatch_PcdPatchableInt32 =  _PCD_PATCHABLE_VALUE_PcdPatchableInt32 // = ((UINT32)0x31313131U)
```

So the main difference from the `FixedAtBuild` and `FeatureFlag` PCDs is that the variable defined as `volatile` and the set functions aren't blocked.

# PCD value modification at run-time

Let's try to set our PCD then. As with get, there are two possibilities. You can use either `PatchPcdSet<Type>` or generic `PcdSet<Type>S` API [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h):
```
#define PatchPcdSet32(TokenName, Value)  (_gPcd_BinaryPatch_##TokenName = (Value))
...
#define PcdSet32S(TokenName, Value)         _PCD_SET_MODE_32_S_##TokenName    ((Value))
```

Keep in mind that `PcdSet32S` unravels to a macro returning a value, for example:
```
PcdSet32S(PcdPatchableInt32, 44) -->  _PCD_SET_MODE_32_S_PcdPatchableInt32 ((44)) --> ((_gPcd_BinaryPatch_PcdPatchableInt32 = (44)), RETURN_SUCCESS)
```

So if you don't want such error:
```
/<...>/Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h:108:106: error: right-hand operand of comma expression has no effect [-Werror=unused-value]
  108 | #define _PCD_SET_MODE_32_S_PcdPatchableInt32(Value)  ((_gPcd_BinaryPatch_PcdPatchableInt32 = (Value)), RETURN_SUCCESS)
      |                                                        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~.
```
You should use something like this:
```
EFI_STATUS Status = PcdSet32S(PcdPatchableInt32, 44);
Print(L"Status=%r\n", Status);
```

The native `PatchPcdSet32` API function doesn't require such thing, so you can use it simply as this:
```
PatchPcdSet32(PcdPatchableInt32, 43);
```

Test both set methods in app code:
```
Print(L"PcdPatchableInt32=0x%x\n", PatchPcdGet32(PcdPatchableInt32));
Print(L"PcdPatchableInt32=0x%x\n", PcdGet32(PcdPatchableInt32));
PatchPcdSet32(PcdPatchableInt32, 43);
Print(L"PcdPatchableInt32=%d\n", PatchPcdGet32(PcdPatchableInt32));
EFI_STATUS Status = PcdSet32S(PcdPatchableInt32, 44);
Print(L"Status=%r\n", Status);
Print(L"PcdPatchableInt32=%d\n", PatchPcdGet32(PcdPatchableInt32));
```

Now if you build and execute our app under OVMF you would get the following output:
```
FS0:\> PCDLesson.efi
...
PcdPatchableInt32=0x31313131
PcdPatchableInt32=0x31313131
PcdPatchableInt32=43
Status=Success
PcdPatchableInt32=44
```

# PCD Patching

In case you've wondered why I've assigned a hex value for the PCD default value or why this PCD type is named `PatchableInModule` this section is for you.

This PCD type is named like that because the value of this PCD can be changed in a binary PE/COFF image (i.e. the final `*.efi` file). To do this two utilities are used:
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

For the `*.efi` file we can use one of:
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
$ grep PatchableInt32 Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.map -A2
 .data._gPcd_BinaryPatch_PcdPatchableInt32
                0x00000000000055a0        0x4 /tmp/PCDLesson.dll.96BX4a.ltrans0.ltrans.o
 *fill*         0x00000000000055a4        0xc
```
So as you can see the default value for our PCD is placed under the 0x55a0 offset.

Now let's execute `GenPatchPcdTable` and create a file `PCDLessonPatchPcdTable` in the app Build `DEBUG` folder:
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
PcdPatchableInt32              0x55A0     .data
```
As you can see it has the same offset that we've seen in a map file.

Now let's get a final look at our default PCD value in an *.elf file:
```
hexdump -s 0x55A0 Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.efi -n 4
00055a0 3131 3131
00055a4
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
  --offset=0x55a0 \
  --value=0xDEADDEAD \
  --type=UINT32 \
  Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.efi
```
Look at hexdump again:
```
$ hexdump -s 0x55A0 Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.efi -n 4
00055a0 dead dead
00055a4
```
We've successfully changed our PCD in a binary!

Now copy the modified `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.efi` file to the UEFI shared disk and execute our app under OVMF:
```
FS0:\> PCDLesson.efi
...
PcdPatchableInt32=0xDEADDEAD
PcdPatchableInt32=0xDEADDEAD
PcdPatchableInt32=43
Status=Success
PcdPatchableInt32=44
```

