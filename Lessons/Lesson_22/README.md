Let's explore another PCD type - feature flag PCD. This is basically a boolean value.

Add this PCD to DEC file `UefiLessonsPkg/UefiLessonsPkg.dec`:
```
[PcdsFeatureFlag]
  gUefiLessonsPkgTokenSpaceGuid.PcdFeatureFlag|TRUE|BOOLEAN|0x16DD586E
```

And populate it to the INF file `UefiLessonsPkg/PCDLesson/PCDLesson.inf`. In the INF file PCDs of this type goes to `[FeaturePcd]` section:
```
[FeaturePcd]
  gUefiLessonsPkgTokenSpaceGuid.PcdFeatureFlag
```

To get the PCD value in a `*.c` file either `FeaturePcdGet` or `PcdGetBool` can be used which are practically the same [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h):
```
#define FeaturePcdGet(TokenName)            _PCD_GET_MODE_BOOL_##TokenName
...
#define PcdGetBool(TokenName)               _PCD_GET_MODE_BOOL_##TokenName
```
Let's use both methonds in our `UefiLessonsPkg/PCDLesson/PCDLesson.c` to print the flag value:
```
Print(L"PcdFeatureFlag=%d\n", FeaturePcdGet(PcdFeatureFlag));
Print(L"PcdFeatureFlag=%d\n", PcdGetBool(PcdFeatureFlag));
```
Ok, we are ready to build. Build your module and check out `AutoGen.h` file `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h`:
```
#define _PCD_TOKEN_PcdFeatureFlag  0U
#define _PCD_SIZE_PcdFeatureFlag 1
#define _PCD_GET_MODE_SIZE_PcdFeatureFlag  _PCD_SIZE_PcdFeatureFlag
#define _PCD_VALUE_PcdFeatureFlag  ((BOOLEAN)1U)
extern const  BOOLEAN  _gPcd_FixedAtBuild_PcdFeatureFlag;
#define _PCD_GET_MODE_BOOL_PcdFeatureFlag  _gPcd_FixedAtBuild_PcdFeatureFlag
//#define _PCD_SET_MODE_BOOL_PcdFeatureFlag  ASSERT(FALSE)  // It is not allowed to set value for a FIXED_AT_BUILD PCD
```

So both calls for our PCD would translate to:
```
FeaturePcdGet/PcdGetBool(PcdFeatureFlag) --> _PCD_GET_MODE_BOOL_PcdFeatureFlag --> _gPcd_FixedAtBuild_PcdFeatureFlag
```

And this `_gPcd_FixedAtBuild_PcdFeatureFlag` variable is a constant from the `AutoGen.c` file `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.c`:
```
GLOBAL_REMOVE_IF_UNREFERENCED const BOOLEAN _gPcd_FixedAtBuild_PcdFeatureFlag;  // = ((BOOLEAN)1U);
```

If you execute the application under OVMF the print statements would get you:
```
FS0:\> PCDLesson.efi
...
PcdFeatureFlag=1
PcdFeatureFlag=1
```

# Comparision to `[PcdsFixedAtBuild]` BOOLEAN datum type PCD

As you can see a feature PCD is similar to a BOOLEAN PCD defined under `PcdsFixedAtBuild` section.

Compare how they are declared in the DEC file `UefiLessonsPkg/UefiLessonsPkg.dec`:
```
[PcdsFixedAtBuild]
  ...
  gUefiLessonsPkgTokenSpaceGuid.PcdBool|TRUE|BOOLEAN|0x69E88A63

[PcdsFeatureFlag]
  gUefiLessonsPkgTokenSpaceGuid.PcdFeatureFlag|TRUE|BOOLEAN|0x16DD586E
```

And in the INF file `UefiLessonsPkg/PCDLesson/PCDLesson.inf`:
```
[FixedPcd]
  ...
  gUefiLessonsPkgTokenSpaceGuid.PcdBool

[FeaturePcd]
  gUefiLessonsPkgTokenSpaceGuid.PcdFeatureFlag
```

As for the `UefiLessonsPkg/PCDLesson/PCDLesson.c` in case of a FeatureFlag PCD you should use `FeaturePcdGet` instead of `FixedPcdGetBool`. But at the same time in both cases you can use `PcdGetBool` function:
```
Print(L"PcdFeatureFlag=%d\n", FeaturePcdGet(PcdFeatureFlag));
Print(L"PcdFeatureFlag=%d\n", PcdGetBool(PcdFeatureFlag));
Print(L"PcdBool=%d\n", FixedPcdGetBool(PcdBool));
Print(L"PcdBool=%d\n", PcdGetBool(PcdBool));
```

Ok, now let's compare the code for the PCDs in the AutoGen files:

`AutoGen.c`:
```
GLOBAL_REMOVE_IF_UNREFERENCED const BOOLEAN _gPcd_FixedAtBuild_PcdBool = _PCD_VALUE_PcdBool;
GLOBAL_REMOVE_IF_UNREFERENCED const BOOLEAN _gPcd_FixedAtBuild_PcdFeatureFlag = _PCD_VALUE_PcdFeatureFlag;
```
`AutoGen.h`:
```
#define _PCD_TOKEN_PcdBool  0U
#define _PCD_SIZE_PcdBool 1
#define _PCD_GET_MODE_SIZE_PcdBool  _PCD_SIZE_PcdBool
#define _PCD_VALUE_PcdBool  1U
extern const  BOOLEAN  _gPcd_FixedAtBuild_PcdBool;
#define _PCD_GET_MODE_BOOL_PcdBool  _gPcd_FixedAtBuild_PcdBool
//#define _PCD_SET_MODE_BOOL_PcdBool  ASSERT(FALSE)  // It is not allowed to set value for a FIXED_AT_BUILD PCD

#define _PCD_TOKEN_PcdFeatureFlag  0U
#define _PCD_SIZE_PcdFeatureFlag 1
#define _PCD_GET_MODE_SIZE_PcdFeatureFlag  _PCD_SIZE_PcdFeatureFlag
#define _PCD_VALUE_PcdFeatureFlag  ((BOOLEAN)1U)
extern const  BOOLEAN  _gPcd_FixedAtBuild_PcdFeatureFlag;
#define _PCD_GET_MODE_BOOL_PcdFeatureFlag  _gPcd_FixedAtBuild_PcdFeatureFlag
//#define _PCD_SET_MODE_BOOL_PcdFeatureFlag  ASSERT(FALSE)  // It is not allowed to set value for a FIXED_AT_BUILD PCD
```

So as you see the differences are subtle. Therefore the `FeaturePcd` is simply a syntactic sugar for BOOLEAN `FixedAtBuild` PCD.

