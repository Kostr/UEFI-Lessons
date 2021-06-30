Let's explore another PCD type - feature flag PCD.

Add PCD to DEC file UefiLessonsPkg/UefiLessonsPkg.dec:
```
[PcdsFeatureFlag]
  gUefiLessonsPkgTokenSpaceGuid.PcdMyFeatureFlagVar|FALSE|BOOLEAN|0x20000001
```

Populate it to INF UefiLessonsPkg/PCDLesson/PCDLesson.inf:
```
[FeaturePcd]
  gUefiLessonsPkgTokenSpaceGuid.PcdMyFeatureFlagVar
```

To get PCD value in a *.c file either `FeaturePcdGet` or `PcdGetBool` can be used which are practically the same https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h:
```
#define FeaturePcdGet(TokenName)            _PCD_GET_MODE_BOOL_##TokenName
#define PcdGetBool(TokenName)               _PCD_GET_MODE_BOOL_##TokenName
```
Let's add both to our UefiLessonsPkg/PCDLesson/PCDLesson.c
```
Print(L"PcdMyFeatureFlagVar=%d\n", FeaturePcdGet(PcdMyFeatureFlagVar));
Print(L"PcdMyFeatureFlagVar=%d\n", PcdGetBool(PcdMyFeatureFlagVar));
```
Ok, we are ready to build. Build your module and check out AutoGen files.

Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
```
#define _PCD_TOKEN_PcdMyFeatureFlagVar  0U
#define _PCD_SIZE_PcdMyFeatureFlagVar 1
#define _PCD_GET_MODE_SIZE_PcdMyFeatureFlagVar  _PCD_SIZE_PcdMyFeatureFlagVar
#define _PCD_VALUE_PcdMyFeatureFlagVar  ((BOOLEAN)0U)
extern const  BOOLEAN  _gPcd_FixedAtBuild_PcdMyFeatureFlagVar;
#define _PCD_GET_MODE_BOOL_PcdMyFeatureFlagVar  _gPcd_FixedAtBuild_PcdMyFeatureFlagVar
//#define _PCD_SET_MODE_BOOL_PcdMyFeatureFlagVar  ASSERT(FALSE)  // It is not allowed to set value for a FIXED_AT_BUILD PCD
```

Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.c
```
GLOBAL_REMOVE_IF_UNREFERENCED const BOOLEAN _gPcd_FixedAtBuild_PcdMyFeatureFlagVar = _PCD_VALUE_PcdMyFeatureFlagVar;
```

So both calls for our PCD would translate to:
```
FeaturePcdGet/PcdGetBool(PcdMyFeatureFlagVar) --> _PCD_GET_MODE_BOOL_PcdMyFeatureFlagVar --> _gPcd_FixedAtBuild_PcdMyFeatureFlagVar
```
And this variable is declared in AutoGen.c file:
```
GLOBAL_REMOVE_IF_UNREFERENCED const BOOLEAN _gPcd_FixedAtBuild_PcdMyFeatureFlagVar = ((BOOLEAN)0U);
```

If you execute it under OVMF these print statements would get you:
```
PcdMyFeatureFlagVar=0
PcdMyFeatureFlagVar=0
```

________________________

So as you can see a feature PCD is similar to a BOOLEAN PCD defined under `PcdsFixedAtBuild` section

Let's create a fixed bool PCD and compare results:

UefiLessonsPkg/UefiLessonsPkg.dec:
```
[PcdsFixedAtBuild]
  ...
  gUefiLessonsPkgTokenSpaceGuid.PcdMyVarBool|FALSE|BOOLEAN|0x00000004

[PcdsFeatureFlag]
  gUefiLessonsPkgTokenSpaceGuid.PcdMyFeatureFlagVar|FALSE|BOOLEAN|0x20000001
```
UefiLessonsPkg/PCDLesson/PCDLesson.inf:
```
[FixedPcd]
  ...
  gUefiLessonsPkgTokenSpaceGuid.PcdMyVarBool

[FeaturePcd]
  gUefiLessonsPkgTokenSpaceGuid.PcdMyFeatureFlagVar
```
UefiLessonsPkg/PCDLesson/PCDLesson.c:
```
Print(L"PcdMyFeatureFlagVar=%d\n", FeaturePcdGet(PcdMyFeatureFlagVar));
Print(L"PcdMyFeatureFlagVar=%d\n", PcdGetBool(PcdMyFeatureFlagVar));
Print(L"PcdMyVarBool=%d\n", FixedPcdGetBool(PcdMyVarBool));
Print(L"PcdMyVarBool=%d\n", PcdGetBool(PcdMyVarBool));
```

Ok, now let's build and look at AutoGen files:

AutoGen.c
```
GLOBAL_REMOVE_IF_UNREFERENCED const BOOLEAN _gPcd_FixedAtBuild_PcdMyVarBool = _PCD_VALUE_PcdMyVarBool;
GLOBAL_REMOVE_IF_UNREFERENCED const BOOLEAN _gPcd_FixedAtBuild_PcdMyFeatureFlagVar = _PCD_VALUE_PcdMyFeatureFlagVar;
```
AutoGen.h
```
#define _PCD_TOKEN_PcdMyVarBool  0U
#define _PCD_SIZE_PcdMyVarBool 1
#define _PCD_GET_MODE_SIZE_PcdMyVarBool  _PCD_SIZE_PcdMyVarBool
#define _PCD_VALUE_PcdMyVarBool  0U
extern const  BOOLEAN  _gPcd_FixedAtBuild_PcdMyVarBool;
#define _PCD_GET_MODE_BOOL_PcdMyVarBool  _gPcd_FixedAtBuild_PcdMyVarBool
//#define _PCD_SET_MODE_BOOL_PcdMyVarBool  ASSERT(FALSE)  // It is not allowed to set value for a FIXED_AT_BUILD PCD

#define _PCD_TOKEN_PcdMyFeatureFlagVar  0U
#define _PCD_SIZE_PcdMyFeatureFlagVar 1
#define _PCD_GET_MODE_SIZE_PcdMyFeatureFlagVar  _PCD_SIZE_PcdMyFeatureFlagVar
#define _PCD_VALUE_PcdMyFeatureFlagVar  ((BOOLEAN)0U)
extern const  BOOLEAN  _gPcd_FixedAtBuild_PcdMyFeatureFlagVar;
#define _PCD_GET_MODE_BOOL_PcdMyFeatureFlagVar  _gPcd_FixedAtBuild_PcdMyFeatureFlagVar
//#define _PCD_SET_MODE_BOOL_PcdMyFeatureFlagVar  ASSERT(FALSE)  // It is not allowed to set value for a FIXED_AT_BUILD PCD
```

As you see the differences are subtle. `FeaturePcd` is simply a syntactic sugar for BOOLEAN `FixedAtBuild` PCD.

