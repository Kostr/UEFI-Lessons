# Dynamic PCD

Now let's try to add Dynamic PCD to our module. In the DEC file PCDs of this type go under the `[PcdsDynamic]` section (`UefiLessonsPkg/UefiLessonsPkg.dec`):
```
[PcdsDynamic]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32|0xCAFECAFE|UINT32|0x4F9259A3
```
Now populate it to the INF. This time you should type it under the `[Pcd]` section:
```
[Pcd]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32
```

In the *.c code use the `PcdGet32`/`PcdSet32S` API to work with the variable:
```
Print(L"PcdDynamicInt32=0x%x\n", PcdGet32(PcdDynamicInt32));
PcdSet32S(PcdDynamicInt32, 0xBEEFBEEF);
Print(L"PcdDynamicInt32=0x%x\n", PcdGet32(PcdDynamicInt32));
```

Now let's look at the AutoGen code. Our PCD wouldn't bring anything to the `AutoGen.c`, but in the `AutoGen.h` file it would give you this:
```
#define _PCD_TOKEN_PcdDynamicInt32  0U
#define _PCD_GET_MODE_32_PcdDynamicInt32  LibPcdGet32(_PCD_TOKEN_PcdDynamicInt32)
#define _PCD_GET_MODE_SIZE_PcdDynamicInt32  LibPcdGetSize(_PCD_TOKEN_PcdDynamicInt32)
#define _PCD_SET_MODE_32_PcdDynamicInt32(Value)  LibPcdSet32(_PCD_TOKEN_PcdDynamicInt32, (Value))
#define _PCD_SET_MODE_32_S_PcdDynamicInt32(Value)  LibPcdSet32S(_PCD_TOKEN_PcdDynamicInt32, (Value))
```

So if you remember that [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h):
```
#define PcdGet32(TokenName)  _PCD_GET_MODE_32_##TokenName
#define PcdSet32S(TokenName, Value)  _PCD_SET_MODE_32_S_##TokenName    ((Value))
```
our API calls would translate like this:
```
PcdGet32(PcdDynamicInt32) -> _PCD_GET_MODE_32_PcdDynamicInt32 -> LibPcdGet32(_PCD_TOKEN_PcdDynamicInt32) -> LibPcdGet32(0U)
PcdSet32S(PcdDynamicInt32, 0xBEEFBEEF) -> _PCD_SET_MODE_32_S_PcdDynamicInt32 ((0xBEEFBEEF)) -> LibPcdSet32S(_PCD_TOKEN_PcdDynamicInt32, (0xBEEFBEEF)) -> LibPcdSet32S(0U, (0xBEEFBEEF))
```

So in the end we have:
```
PcdGet32(PcdDynamicInt32)              -> LibPcdGet32(0U)
PcdSet32S(PcdDynamicInt32, 0xBEEFBEEF) -> LibPcdSet32S(0U, (0xBEEFBEEF))
```

This doesn't sound right. As we already know in the PCD Database the local token numbering starts from 1. So 0 is invalid token number.
If you build and execute our aplication now you would probably get a system fault.

What is wrong?

`LibPcdGet`/`LibPcdSet` API is intendend to access the current PCD Database. So when we execute our application under the OVMF the `LibPcdGet`/`LibPcdSet` would access the OVMF PCD Database.

But as you remember the PCD DB is static in a way, that all of its PCDs must be gathered at the image build time. It is not possible to add more Dynamic PCDs to the Database at run-time. Which Dynamic PCDs go the PCD Database? Only the Dynamic PCDs of the modules that would go to the final flash image.

The general mechanics is this:
- For all of the modules that go to the final flash image assign `LocalToken` numbers to all their `[DynamicPcds]`. This is a global platfrom level assignment which starts from 1
- Using all of the `[DynamicPcds]` above generate PCD DB images for the `PCD_IS_DRIVER = PEI_PCD_DRIVER/DXE_PCD_DRIVER` modules

Our application it not a part of a OVMF image, therefore there is no notion of our `PcdDynamicInt32` in the OVMF PCD Database. And our package doesn't create any flash image itself. Therefore the build system can't assign any local token number to our PCD.

So our error has happend because our module with its Dynamic PCD was compiled separately from the main firmare with a PCD Database.

To fix this we need to include our application to the OVMF flash image.

Add our image to the end of the `[FV.DXEFV]` section in the `OvmfPkg/OvmfPkgX64.fdf`:
```
[FV.DXEFV]
...
INF  UefiLessonsPkg/PCDLesson/PCDLesson.inf
```
And for the correct compilation also add it to `[Components]` section in the `OvmfPkg/OvmfPkgX64.dsc`:
```
[Components]
...
UefiLessonsPkg/PCDLesson/PCDLesson.inf
```
Now we need to rebuild the OVMF image:
```
$ build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

If you will execute the PCD Database parser program now, you would notice that the max local token number is now one number more. It is 41 instead of 40 that was before. We can also find our PCD by its magic number value:
```
$ ./parse_pcd_db \
  --peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
  --dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw"

...

DXE PCD DB
LocalTokenNumberTable:

...

38:
Token type = Data
Datum type = UINT32
Value:
0xcafecafe (=3405695742)

...
```

Now let's check our application AutoGen files. Keep in mind that our application current build would be not in the `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/` folder, but in the `Build/OvmfX64/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/` folder, as we've build the `OvmfX64` package.

So, if you look at the `Build/OvmfX64/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h`, you'll see that now our PCD has correct token number assigned:
```
#define _PCD_TOKEN_PcdDynamicInt32  38U
#define _PCD_GET_MODE_32_PcdDynamicInt32  LibPcdGet32(_PCD_TOKEN_PcdDynamicInt32)
#define _PCD_GET_MODE_SIZE_PcdDynamicInt32  LibPcdGetSize(_PCD_TOKEN_PcdDynamicInt32)
#define _PCD_SET_MODE_32_PcdDynamicInt32(Value)  LibPcdSet32(_PCD_TOKEN_PcdDynamicInt32, (Value))
#define _PCD_SET_MODE_32_S_PcdDynamicInt32(Value)  LibPcdSet32S(_PCD_TOKEN_PcdDynamicInt32, (Value))
```
The number 38 is exactly the one that we saw in the `parse_pcd_db` output before.

Now copy the correct execatable to the shared folder:
```
$ cp Build/OvmfX64/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.efi ~/UEFI_disk/
```
And execute it under the updated OVMF image:
```
FS0:\> PCDLesson.efi
...
PcdDynamicInt32=0xCAFECAFE
PcdDynamicInt32=0xBEEFBEEF
```
So, now the Get and Set methods are working correctly.

If you'll execute the application one more time, you could notice that the last application call has changed the PCD value:
```
FS0:\> PCDLesson.efi
...
PcdDynamicInt32=0xBEEFBEEF
PcdDynamicInt32=0xBEEFBEEF
```
But this change only exists at this boot. The PCD Database module loads database files from the flash into the RAM, and all the PCD changes modify only the current RAM values, not affecting the flash code.

Therefore if you rerun QEMU, the PCD would start from its initial values from the database:
```
FS0:\> PCDLesson.efi
...
PcdDynamicInt32=0xCAFECAFE
PcdDynamicInt32=0xBEEFBEEF
```

As we don't always use our application as a part of Ovmf package it would be better to guard the `PcdDynamicInt32` access for cases when local tokan is unassigned.

We can do it with a help of a `PcdToken` macro:
```
#define PcdToken(TokenName)  _PCD_TOKEN_##TokenName
```

Code usage as simple as this:
```
if (PcdToken(PcdDynamicInt32)) {
  Print(L"PcdDynamicInt32=0x%x\n", PcdGet32(PcdDynamicInt32));
  PcdSet32S(PcdDynamicInt32, 0xBEEFBEEF);
  Print(L"PcdDynamicInt32=0x%x\n", PcdGet32(PcdDynamicInt32));
} else {
  Print(L"PcdDynamicInt32 token is unassigned\n");
}
```
At least now if you accidently copy `Build/UefiLessonsPkg/RELEASE_GCC5/X64/PCDLesson.efi` instead of `Build/OvmfX64/RELEASE_GCC5/X64/PCDLesson.efi` the application won't break the OVMF.

# Dynamic Ex PCD

Suppose we already have the necessary PCD in the PCD Database. And we want to access it via application from the UEFI Shell.

The only way to do it with Dynamic PCD is to include this application to the image build. In other case, when we compile the application separately, the build system is anaware about the local token numbering system used in the image PCD DB, and would generate 0 as a token number.

But sometimes we don't have a sources for the flash image. So it is not possible to compile our application as a part of image. What can be done in such case?

For this case there is another dynamic PCD type - `Dynamic Ex PCD`. To work with this PCD you need to provide a `Token number` (the one that we write when we declare PCD) and a `Token Guid`.

In the DEC file PCDs of this type go under the `[PcdsDynamicEx]` section (`UefiLessonsPkg/UefiLessonsPkg.dec`):
```
[PcdsDynamicEx]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|0xBABEBABE|UINT32|0xAF35F3B2
```
And in the INF file they go under the `[PcdEx]` section (`UefiLessonsPkg/PCDLesson/PCDLesson.inf`):
```
[PcdEx]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32
```

We still need to include this PCD in the OVMF PCD database.

But we already have our application included in the `OvmfPkg/OvmfPkgX64.fdf`:
```
[FV.DXEFV]
...
INF  UefiLessonsPkg/PCDLesson/PCDLesson.inf
```
And in the `OvmfPkg/OvmfPkgX64.dsc`:
```
[Components]
...
UefiLessonsPkg/PCDLesson/PCDLesson.inf
```

So the only thing that we need to do is to rebuild OVMF image:
```
$ build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

You can verify that our new PCD was added to the updated PCD Database:
```
$ ./parse_pcd_db \
  --peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
  --dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw"

...

DXE PCD DB
LocalTokenNumberTable:

...

42:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0xaf35f3b2
DynamicEx GUID  = 150cab53-ad47-4385-b5ddbcfc76bacaf0
Value:
0xbabebabe (=3133061822)
```

The `DynamicEx Token = 0xaf35f3b2` is the token number that we've used in the DEC file:
```
[PcdsDynamicEx]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|0xBABEBABE|UINT32|0xAF35F3B2
```
And `DynamicEx GUID  = 150cab53-ad47-4385-b5ddbcfc76bacaf0` is a GUID of the token space declared in the same DEC file:
```
[Guids]
  ...
  gUefiLessonsPkgTokenSpaceGuid = {0x150cab53, 0xad47, 0x4385, {0xb5, 0xdd, 0xbc, 0xfc, 0x76, 0xba, 0xca, 0xf0}}
```

And this is the code added to the `Build/OvmfX64/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h` (`2939548594=0xAF35F3B2`):
```
#define _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicExInt32  2939548594U
#define _PCD_TOKEN_PcdDynamicExInt32  _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicExInt32
#define _PCD_GET_MODE_32_PcdDynamicExInt32  LibPcdGetEx32(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32)
#define _PCD_GET_MODE_SIZE_PcdDynamicExInt32 LibPcdGetExSize(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32)
#define _PCD_SET_MODE_32_PcdDynamicExInt32(Value)  LibPcdSetEx32(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32, (Value))
#define _PCD_SET_MODE_32_S_PcdDynamicExInt32(Value)  LibPcdSetEx32S(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32, (Value))

#define COMPAREGUID(Guid1, Guid2) (BOOLEAN)(*(CONST UINT64*)Guid1 == *(CONST UINT64*)Guid2 && *((CONST UINT64*)Guid1 + 1) == *((CONST UINT64*)Guid2 + 1))

#define __PCD_PcdDynamicExInt32_ADDR_CMP(GuidPtr)  (\
  (GuidPtr == &gUefiLessonsPkgTokenSpaceGuid) ? _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicExInt32:0 \
  )

#define __PCD_PcdDynamicExInt32_VAL_CMP(GuidPtr)  (\
  (GuidPtr == NULL) ? 0:\
  COMPAREGUID (GuidPtr, &gUefiLessonsPkgTokenSpaceGuid) ? _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicExInt32:0 \
  )
#define _PCD_TOKEN_EX_PcdDynamicExInt32(GuidPtr)   __PCD_PcdDynamicExInt32_ADDR_CMP(GuidPtr) ? __PCD_PcdDynamicExInt32_ADDR_CMP(GuidPtr) : __PCD_PcdDynamicExInt32_VAL_CMP(GuidPtr)
```

The important part is that if we build our `UefiLessonsPkg` package now:
```
$ build
```
the application would get the same code in its `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h` file. 

So even without any connection to the OVMF source code, the build system generates correct credentials to access the necessary PCD. We can make some other application in our package, and still use it to access the same PCD that we've created with our app.

This is the main difference between the Dynamic and DynamicEx PCDs - we don't need to build the application as a part of a main flash image build.

Now let's try to access the PCD, this is done via `PcdGetEx32`/`PcdSetEx32S` functions from the [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h):
```
#define PcdTokenEx(Guid, TokenName)  _PCD_TOKEN_EX_##TokenName(Guid)
#define PcdGetEx32(Guid, TokenName)  LibPcdGetEx32 ((Guid), PcdTokenEx(Guid,TokenName))
#define PcdSetEx32S(Guid, TokenName, Value)  LibPcdSetEx32S ((Guid), PcdTokenEx(Guid,TokenName), (Value))
```
As you can see, API in this case a little bit different. The `Guid` in these defines is a pointer to the GUID, so we need to use `&gUefiLessonsPkgTokenSpaceGuid` in our case. Here is the code:
```
Print(L"PcdDynamicExInt32=0x%x\n", PcdGetEx32(&gUefiLessonsPkgTokenSpaceGuid, PcdDynamicExInt32));
PcdSet32S(gUefiLessonsPkgTokenSpaceGuid, PcdDynamicExInt32, 0x77777777);
Print(L"PcdDynamicExInt32=0x%x\n", PcdGetEx32(&gUefiLessonsPkgTokenSpaceGuid, PcdDynamicExInt32));
```
We don't need to do anything to import `gUefiLessonsPkgTokenSpaceGuid` as it is already in its `AutoGen.c` file:
```
...
GLOBAL_REMOVE_IF_UNREFERENCED EFI_GUID gUefiLessonsPkgTokenSpaceGuid = {0x150cab53, 0xad47, 0x4385, {0xb5, 0xdd, 0xbc, 0xfc, 0x76, 0xba, 0xca, 0xf0}};
...
```

It is kinda hard to unravel the preprocessor code like we used to, because of all the if/then/else guardian logic, but in the end we would just call `LibPcdGetEx32`/`LibPcdSetEx32S` functions with these arguments:
```
PcdGetEx32(gUefiLessonsPkgTokenSpaceGuid, PcdDynamicExInt32)
-> LibPcdGetEx32 ((gUefiLessonsPkgTokenSpaceGuid), PcdTokenEx(gUefiLessonsPkgTokenSpaceGuid,PcdDynamicExInt32))
-> LibPcdGetEx32 ((gUefiLessonsPkgTokenSpaceGuid), _PCD_TOKEN_EX_PcdDynamicExInt32(gUefiLessonsPkgTokenSpaceGuid))
...
-> LibPcdGetEx32 ( {0x150cab53, 0xad47, 0x4385, {0xb5, 0xdd, 0xbc, 0xfc, 0x76, 0xba, 0xca, 0xf0}}, 2939548594U)

PcdSetEx32S(gUefiLessonsPkgTokenSpaceGuid, PcdDynamicExInt32, 0x77777777)
-> LibPcdSetEx32S ((gUefiLessonsPkgTokenSpaceGuid), PcdTokenEx(gUefiLessonsPkgTokenSpaceGuid,PcdDynamicExInt32), (0x77777777))
...
-> LibPcdSetEx32S ( {0x150cab53, 0xad47, 0x4385, {0xb5, 0xdd, 0xbc, 0xfc, 0x76, 0xba, 0xca, 0xf0}}, 2939548594U, (0x77777777) )
```

Now build our application and copy results to the shared disk. Again, in the case of DynamicEx PCD you can use the result of the package build, i.e. `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/PCDLesson.efi` file.

The behaviour is similar to the Dynamic PCD. For the first time you'll get initial PCD value:
```
FS0:\> PCDLesson.efi
...
PcdDynamicExInt32=0xBABEBABE
PcdDynamicExInt32=0x77777777
```
But since the program updates PCD, at the next launches it would start with the updated value:
```
FS0:\> PCDLesson.efi
...
PcdDynamicExInt32=0x77777777
PcdDynamicExInt32=0x77777777
```
And QEMU re-launch takes things to the start.

# Using Dynamic API functions to access Dynamic Ex PCDs

It is important to notice that the `PcdGet32`/`PcdSet32S` functions are not specific Dynamic API functions. These functons are generic API that can be used with all types of PCD including the Dynamic Ex PCDs. For an example add this code to our application:
```
Print(L"PcdDynamicExInt32=0x%x\n", PcdGet32(PcdDynamicExInt32));
PcdSet32S(PcdDynamicExInt32, 0x88888888);
Print(L"PcdDynamicExInt32=0x%x\n", PcdGet32(PcdDynamicExInt32));
```

If you are interested how it works check `Build/OvmfX64/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h`:
```
#define _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicExInt32  2939548594U
#define _PCD_TOKEN_PcdDynamicExInt32  _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicExInt32
#define _PCD_GET_MODE_32_PcdDynamicExInt32  LibPcdGetEx32(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32)
#define _PCD_GET_MODE_SIZE_PcdDynamicExInt32 LibPcdGetExSize(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32)
#define _PCD_SET_MODE_32_PcdDynamicExInt32(Value)  LibPcdSetEx32(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32, (Value))
#define _PCD_SET_MODE_32_S_PcdDynamicExInt32(Value)  LibPcdSetEx32S(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32, (Value))
```
and remember `PcdGet32`/`PcdSet32S` definitions:
```
#define PcdGet32(TokenName)  _PCD_GET_MODE_32_##TokenName
#define PcdSet32S(TokenName, Value)  _PCD_SET_MODE_32_S_##TokenName    ((Value))
```

The code can be unravelled like this:
```
PcdGet32(PcdDynamicExInt32) -> _PCD_GET_MODE_32_PcdDynamicExInt32 -> LibPcdGetEx32(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32)
PcdSet32S(PcdDynamicExInt32, 0x88888888) -> _PCD_SET_MODE_32_S_PcdDynamicExInt32 ((0x88888888)) -> LibPcdSetEx32S(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32, (0x88888888))
```

You can rebuild our application and verify that it still works:
```
FS0:\> PCDLesson.efi
...
PcdDynamicExInt32=0xBABEBABE
PcdDynamicExInt32=0x77777777
PcdDynamicExInt32=0x77777777
PcdDynamicExInt32=0x88888888
```

# `PCD_DYNAMIC_AS_DYNAMICEX`

EDK2 has an easy way to change all the Dynamic PCDs to Dynamic Ex type. For that all you need to do is to ad a special define to the platform DSC file.

To check it, add this code to the `OvmfPkg/OvmfPkgX64.dsc` file:
```
[Defines]
  ...
  PCD_DYNAMIC_AS_DYNAMICEX = TRUE
```

When you'll rebuild OVMF, you could notice that setting this define will lead to significant ammount of recompilation.

After the build check the PCD DB files:
```
$ ./parse_pcd_db \
  --peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
  --dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw"

PEI PCD DB
LocalTokenNumberTable:

1:
Token type = Data
Datum type = UINT8 (Bool)
DynamicEx Token = 0x01100000
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

2:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x40000008
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

3:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00030007
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

4:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00030008
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

5:
Token type = String
Datum type = Pointer
DynamicEx Token = 0x00030005
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
CurrentSize = 1
MaxSize     = 1
Value:
00                                               | .

6:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x30001047
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

7:
Token type = Data
Datum type = UINT16
DynamicEx Token = 0x00030004
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

8:
Token type = Data
Datum type = UINT8 (Bool)
DynamicEx Token = 0x0001006f
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

9:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00030006
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

10:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0x00000001
DynamicEx GUID  = 0d3fb176-9569-4d51-a3ef7d61c64feaba [gEfiSecurityPkgTokenSpaceGuid]
0 - unitialized

11:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0x00000008
DynamicEx GUID  = ac05bf33-995a-4ed4-aab8ef7ae80f5cb0 [gUefiCpuPkgTokenSpaceGuid]
0 - unitialized

12:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0x00000002
DynamicEx GUID  = ac05bf33-995a-4ed4-aab8ef7ae80f5cb0 [gUefiCpuPkgTokenSpaceGuid]
Value:
0x00000040 (=64)

13:
Token type = Data
Datum type = UINT8 (Bool)
DynamicEx Token = 0x60000016
DynamicEx GUID  = ac05bf33-995a-4ed4-aab8ef7ae80f5cb0 [gUefiCpuPkgTokenSpaceGuid]
0 - unitialized

14:
Token type = Data
Datum type = UINT16
DynamicEx Token = 0x0000001b
DynamicEx GUID  = 93bb96af-b9f2-4eb8-9462e0ba74564236 [gUefiOvmfPkgTokenSpaceGuid]
0 - unitialized

15:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00000022
DynamicEx GUID  = 93bb96af-b9f2-4eb8-9462e0ba74564236 [gUefiOvmfPkgTokenSpaceGuid]
0 - unitialized

16:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00000023
DynamicEx GUID  = 93bb96af-b9f2-4eb8-9462e0ba74564236 [gUefiOvmfPkgTokenSpaceGuid]
0 - unitialized

17:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00000024
DynamicEx GUID  = 93bb96af-b9f2-4eb8-9462e0ba74564236 [gUefiOvmfPkgTokenSpaceGuid]
0 - unitialized

18:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00000025
DynamicEx GUID  = 93bb96af-b9f2-4eb8-9462e0ba74564236 [gUefiOvmfPkgTokenSpaceGuid]
0 - unitialized

19:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00000026
DynamicEx GUID  = 93bb96af-b9f2-4eb8-9462e0ba74564236 [gUefiOvmfPkgTokenSpaceGuid]
0 - unitialized

20:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00000027
DynamicEx GUID  = 93bb96af-b9f2-4eb8-9462e0ba74564236 [gUefiOvmfPkgTokenSpaceGuid]
Value:
0x0000000800000000 (=34359738368)

21:
Token type = Data
Datum type = UINT8 (Bool)
DynamicEx Token = 0x00000034
DynamicEx GUID  = 93bb96af-b9f2-4eb8-9462e0ba74564236 [gUefiOvmfPkgTokenSpaceGuid]
0 - unitialized

22:
Token type = Data
Datum type = UINT16
DynamicEx Token = 0x00000020
DynamicEx GUID  = 93bb96af-b9f2-4eb8-9462e0ba74564236 [gUefiOvmfPkgTokenSpaceGuid]
Value:
0x0008 (=8)
_____

DXE PCD DB
LocalTokenNumberTable:

23:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0x30000013
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

24:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0x30000010
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

25:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x80000001
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

26:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00030000
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

27:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00030001
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

28:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0x4000000b
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
Value:
0x00000280 (=640)

29:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0x4000000c
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
Value:
0x000001e0 (=480)

30:
Token type = Data
Datum type = UINT8
DynamicEx Token = 0x0001006a
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

31:
Token type = Data
Datum type = UINT16
DynamicEx Token = 0x00010055
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
Value:
0x0208 (=520)

32:
Token type = Data
Datum type = UINT8 (Bool)
DynamicEx Token = 0x00030003
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
0 - unitialized

33:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0x40000009
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
Value:
0x00000320 (=800)

34:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0x4000000a
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
Value:
0x00000258 (=600)

35:
Token type = Data
Datum type = UINT16
DynamicEx Token = 0x0000002c
DynamicEx GUID  = 914aebe7-4635-459b-aa1c11e219b03a10 [gEfiMdePkgTokenSpaceGuid]
0 - unitialized

36:
Token type = Data
Datum type = UINT8
DynamicEx Token = 0x10000009
DynamicEx GUID  = 40e064b2-0ae0-48b1-a07df8cf1e1a2310 [gEfiNetworkPkgTokenSpaceGuid]
Value:
0x01 (=1)

37:
Token type = Data
Datum type = UINT8
DynamicEx Token = 0x1000000a
DynamicEx GUID  = 40e064b2-0ae0-48b1-a07df8cf1e1a2310 [gEfiNetworkPkgTokenSpaceGuid]
Value:
0x01 (=1)

38:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0xaf35f3b2
DynamicEx GUID  = 150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
Value:
0xbabebabe (=3133061822)

39:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0x4f9259a3
DynamicEx GUID  = 150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
Value:
0xcafecafe (=3405695742)

40:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00000002
DynamicEx GUID  = 93bb96af-b9f2-4eb8-9462e0ba74564236 [gUefiOvmfPkgTokenSpaceGuid]
0 - unitialized

41:
Token type = Data
Datum type = UINT8 (Bool)
DynamicEx Token = 0x00000010
DynamicEx GUID  = 93bb96af-b9f2-4eb8-9462e0ba74564236 [gUefiOvmfPkgTokenSpaceGuid]
0 - unitialized

42:
Token type = Data
Datum type = UINT8 (Bool)
DynamicEx Token = 0x00000021
DynamicEx GUID  = 93bb96af-b9f2-4eb8-9462e0ba74564236 [gUefiOvmfPkgTokenSpaceGuid]
0 - unitialized
_____
```

As we already now, we can use `PcdGet32`/`PcdSet32S` functions to access Dynamic Ex PCDs. So we don't need to change anything in our modules because of this transition.

Look at the generated file `Build/OvmfX64/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h`:
```
#define _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicInt32  1334991267U
#define _PCD_TOKEN_PcdDynamicInt32  _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicInt32
#define _PCD_GET_MODE_32_PcdDynamicInt32  LibPcdGetEx32(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicInt32)
#define _PCD_GET_MODE_SIZE_PcdDynamicInt32 LibPcdGetExSize(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicInt32)
#define _PCD_SET_MODE_32_PcdDynamicInt32(Value)  LibPcdSetEx32(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicInt32, (Value))
#define _PCD_SET_MODE_32_S_PcdDynamicInt32(Value)  LibPcdSetEx32S(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicInt32, (Value))

#define _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicExInt32  2939548594U
#define _PCD_TOKEN_PcdDynamicExInt32  _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicExInt32
#define _PCD_GET_MODE_32_PcdDynamicExInt32  LibPcdGetEx32(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32)
#define _PCD_GET_MODE_SIZE_PcdDynamicExInt32 LibPcdGetExSize(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32)
#define _PCD_SET_MODE_32_PcdDynamicExInt32(Value)  LibPcdSetEx32(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32, (Value))
#define _PCD_SET_MODE_32_S_PcdDynamicExInt32(Value)  LibPcdSetEx32S(&gUefiLessonsPkgTokenSpaceGuid, _PCD_TOKEN_PcdDynamicExInt32, (Value))

#define COMPAREGUID(Guid1, Guid2) (BOOLEAN)(*(CONST UINT64*)Guid1 == *(CONST UINT64*)Guid2 && *((CONST UINT64*)Guid1 + 1) == *((CONST UINT64*)Guid2 + 1))

#define __PCD_PcdDynamicInt32_ADDR_CMP(GuidPtr)  (\
  (GuidPtr == &gUefiLessonsPkgTokenSpaceGuid) ? _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicInt32:0 \
  )

#define __PCD_PcdDynamicExInt32_ADDR_CMP(GuidPtr)  (\
  (GuidPtr == &gUefiLessonsPkgTokenSpaceGuid) ? _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicExInt32:0 \
  )

#define __PCD_PcdDynamicInt32_VAL_CMP(GuidPtr)  (\
  (GuidPtr == NULL) ? 0:\
  COMPAREGUID (GuidPtr, &gUefiLessonsPkgTokenSpaceGuid) ? _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicInt32:0 \
  )
#define _PCD_TOKEN_EX_PcdDynamicInt32(GuidPtr)   __PCD_PcdDynamicInt32_ADDR_CMP(GuidPtr) ? __PCD_PcdDynamicInt32_ADDR_CMP(GuidPtr) : __PCD_PcdDynamicInt32_VAL_CMP(GuidPtr)

#define __PCD_PcdDynamicExInt32_VAL_CMP(GuidPtr)  (\
  (GuidPtr == NULL) ? 0:\
  COMPAREGUID (GuidPtr, &gUefiLessonsPkgTokenSpaceGuid) ? _PCD_TOKEN_gUefiLessonsPkgTokenSpaceGuid_PcdDynamicExInt32:0 \
  )
#define _PCD_TOKEN_EX_PcdDynamicExInt32(GuidPtr)   __PCD_PcdDynamicExInt32_ADDR_CMP(GuidPtr) ? __PCD_PcdDynamicExInt32_ADDR_CMP(GuidPtr) : __PCD_PcdDynamicExInt32_VAL_CMP(GuidPtr)
```
Here you can see how both PCD defines in the end now call `LibPcdGetEx`/`LibPcdSetEx` functions.

You can verify that OVMF still boots and our application still works as expected:
```
FS0:\> PCDLesson.efi
...
PcdDynamicInt32=0xCAFECAFE
PcdDynamicInt32=0xBEEFBEEF
PcdDynamicExInt32=0xBABEBABE
PcdDynamicExInt32=0x77777777
PcdDynamicExInt32=0x77777777
PcdDynamicExInt32=0x88888888
```

# `PCD_INFO_GENERATION`

There is another interesting platform level PCD define - `PCD_INFO_GENERATION`. Add this setting to the `OvmfPkg/OvmfPkgX64.dsc` (and remove `PCD_DYNAMIC_AS_DYNAMICEX = TRUE`):
```
[Defines]
  ...
  PCD_INFO_GENERATION = TRUE
```

Recompile OVMF and check PCD DB files:
```
$ ./parse_pcd_db \
    --peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
    --dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw"

PEI PCD DB
LocalTokenNumberTable:

1:
Token type = Data
Datum type = UINT8 (Bool)
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdAcpiS3Enable
0 - unitialized

2:
Token type = Data
Datum type = UINT64
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdEmuVariableNvStoreReserved
0 - unitialized

3:
Token type = Data
Datum type = UINT64
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdGhcbBase
0 - unitialized

4:
Token type = Data
Datum type = UINT64
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdGhcbSize
0 - unitialized

5:
Token type = Data
Datum type = UINT64
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdPteMemoryEncryptionAddressOrMask
0 - unitialized

6:
Token type = Data
Datum type = UINT8 (Bool)
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdSetNxForStack
0 - unitialized

7:
Token type = Data
Datum type = UINT32
TokenSpaceName = gEfiSecurityPkgTokenSpaceGuid
PcdName = PcdOptionRomImageVerificationPolicy
0 - unitialized

8:
Token type = Data
Datum type = UINT32
TokenSpaceName = gUefiCpuPkgTokenSpaceGuid
PcdName = PcdCpuBootLogicalProcessorNumber
0 - unitialized

9:
Token type = Data
Datum type = UINT32
TokenSpaceName = gUefiCpuPkgTokenSpaceGuid
PcdName = PcdCpuMaxLogicalProcessorNumber
Value:
0x00000040 (=64)

10:
Token type = Data
Datum type = UINT8 (Bool)
TokenSpaceName = gUefiCpuPkgTokenSpaceGuid
PcdName = PcdSevEsIsEnabled
0 - unitialized

11:
Token type = Data
Datum type = UINT16
TokenSpaceName = gUefiOvmfPkgTokenSpaceGuid
PcdName = PcdOvmfHostBridgePciDevId
0 - unitialized

12:
Token type = Data
Datum type = UINT64
TokenSpaceName = gUefiOvmfPkgTokenSpaceGuid
PcdName = PcdPciIoBase
0 - unitialized

13:
Token type = Data
Datum type = UINT64
TokenSpaceName = gUefiOvmfPkgTokenSpaceGuid
PcdName = PcdPciIoSize
0 - unitialized

14:
Token type = Data
Datum type = UINT64
TokenSpaceName = gUefiOvmfPkgTokenSpaceGuid
PcdName = PcdPciMmio32Base
0 - unitialized

15:
Token type = Data
Datum type = UINT64
TokenSpaceName = gUefiOvmfPkgTokenSpaceGuid
PcdName = PcdPciMmio32Size
0 - unitialized

16:
Token type = Data
Datum type = UINT64
TokenSpaceName = gUefiOvmfPkgTokenSpaceGuid
PcdName = PcdPciMmio64Base
0 - unitialized

17:
Token type = Data
Datum type = UINT64
TokenSpaceName = gUefiOvmfPkgTokenSpaceGuid
PcdName = PcdPciMmio64Size
Value:
0x0000000800000000 (=34359738368)

18:
Token type = Data
Datum type = UINT8 (Bool)
TokenSpaceName = gUefiOvmfPkgTokenSpaceGuid
PcdName = PcdQ35SmramAtDefaultSmbase
0 - unitialized

19:
Token type = Data
Datum type = UINT16
TokenSpaceName = gUefiOvmfPkgTokenSpaceGuid
PcdName = PcdQ35TsegMbytes
Value:
0x0008 (=8)

20:
Token type = String
Datum type = Pointer
DynamicEx Token = 0x00030005
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdNvStoreDefaultValueBuffer
CurrentSize = 1
MaxSize     = 1
Value:
00                                               | .

21:
Token type = Data
Datum type = UINT16
DynamicEx Token = 0x00030004
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdSetNvStoreDefaultId
0 - unitialized

22:
Token type = Data
Datum type = UINT64
DynamicEx Token = 0x00030006
DynamicEx GUID  = a1aff049-fdeb-442a-b32013ab4cb72bbc [gEfiMdeModulePkgTokenSpaceGuid]
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdVpdBaseAddress64
0 - unitialized
_____

DXE PCD DB
LocalTokenNumberTable:

23:
Token type = Data
Datum type = UINT32
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdFlashNvStorageFtwSpareBase
0 - unitialized

24:
Token type = Data
Datum type = UINT32
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdFlashNvStorageFtwWorkingBase
0 - unitialized

25:
Token type = Data
Datum type = UINT64
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdFlashNvStorageVariableBase64
0 - unitialized

26:
Token type = Data
Datum type = UINT64
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdS3BootScriptTablePrivateDataPtr
0 - unitialized

27:
Token type = Data
Datum type = UINT64
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdS3BootScriptTablePrivateSmmDataPtr
0 - unitialized

28:
Token type = Data
Datum type = UINT32
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdSetupVideoHorizontalResolution
Value:
0x00000280 (=640)

29:
Token type = Data
Datum type = UINT32
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdSetupVideoVerticalResolution
Value:
0x000001e0 (=480)

30:
Token type = Data
Datum type = UINT8
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdSmbiosDocRev
0 - unitialized

31:
Token type = Data
Datum type = UINT16
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdSmbiosVersion
Value:
0x0208 (=520)

32:
Token type = Data
Datum type = UINT8 (Bool)
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdTestKeyUsed
0 - unitialized

33:
Token type = Data
Datum type = UINT32
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdVideoHorizontalResolution
Value:
0x00000320 (=800)

34:
Token type = Data
Datum type = UINT32
TokenSpaceName = gEfiMdeModulePkgTokenSpaceGuid
PcdName = PcdVideoVerticalResolution
Value:
0x00000258 (=600)

35:
Token type = Data
Datum type = UINT16
TokenSpaceName = gEfiMdePkgTokenSpaceGuid
PcdName = PcdPlatformBootTimeOut
0 - unitialized

36:
Token type = Data
Datum type = UINT8
TokenSpaceName = gEfiNetworkPkgTokenSpaceGuid
PcdName = PcdIPv4PXESupport
Value:
0x01 (=1)

37:
Token type = Data
Datum type = UINT8
TokenSpaceName = gEfiNetworkPkgTokenSpaceGuid
PcdName = PcdIPv6PXESupport
Value:
0x01 (=1)

38:
Token type = Data
Datum type = UINT32
TokenSpaceName = gUefiLessonsPkgTokenSpaceGuid
PcdName = PcdDynamicInt32
Value:
0xcafecafe (=3405695742)

39:
Token type = Data
Datum type = UINT64
TokenSpaceName = gUefiOvmfPkgTokenSpaceGuid
PcdName = PcdEmuVariableEvent
0 - unitialized

40:
Token type = Data
Datum type = UINT8 (Bool)
TokenSpaceName = gUefiOvmfPkgTokenSpaceGuid
PcdName = PcdOvmfFlashVariablesEnable
0 - unitialized

41:
Token type = Data
Datum type = UINT8 (Bool)
TokenSpaceName = gUefiOvmfPkgTokenSpaceGuid
PcdName = PcdQemuSmbiosValidated
0 - unitialized

42:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0xaf35f3b2
DynamicEx GUID  = 150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
TokenSpaceName = gUefiLessonsPkgTokenSpaceGuid
PcdName = PcdDynamicExInt32
Value:
0xbabebabe (=3133061822)
_____
```

Now every token has `TokenSpaceName`/`PcdName` fields. They are present because PCD Database now fills its `PcdNameTable` part that in the usual build is empty. With this information it is much easier to understand the databse content.

# `DumpDynPcd`

The `MdeModulePkg` contains an application to dump Dynamic PCD information at runtime - [https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Application/DumpDynPcd](https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Application/DumpDynPcd).

To build it use this command:
```
$ build --platform=MdeModulePkg/MdeModulePkg.dsc --module=MdeModulePkg/Application/DumpDynPcd/DumpDynPcd.inf --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

Copy result to the shared disk:
```
$ cp Build/MdeModule/RELEASE_GCC5/X64/DumpDynPcd.efi ~/UEFI_disk/
```

If you'll execute this program under UEFI shell, you'll get the output similar to the one that we get from the `parse_pcd_db` program:
```
FS0:\> DumpDynPcd.efi
Current system SKU ID: 0x0

Default Token Space
  Token = 0x00000001 - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = TRUE

Default Token Space
  Token = 0x00000002 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x7EF4000

Default Token Space
  Token = 0x00000003 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x0

Default Token Space
  Token = 0x00000004 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x0

Default Token Space
  Token = 0x00000005 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x0

Default Token Space
  Token = 0x00000006 - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = FALSE

Default Token Space
  Token = 0x00000007 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x0

Default Token Space
  Token = 0x00000008 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x0

Default Token Space
  Token = 0x00000009 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x1

Default Token Space
  Token = 0x0000000A - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = FALSE

Default Token Space
  Token = 0x0000000B - Type = UINT16:DYNAMIC    - Size = 0x2 - Value = 0x1237

Default Token Space
  Token = 0x0000000C - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0xC000

Default Token Space
  Token = 0x0000000D - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x4000

Default Token Space
  Token = 0x0000000E - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x80000000

Default Token Space
  Token = 0x0000000F - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x7C000000

Default Token Space
  Token = 0x00000010 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x800000000

Default Token Space
  Token = 0x00000011 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x800000000

Default Token Space
  Token = 0x00000012 - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = FALSE

Default Token Space
  Token = 0x00000013 - Type = UINT16:DYNAMIC    - Size = 0x2 - Value = 0x8

Default Token Space
  Token = 0x00000017 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0xFFC42000

Default Token Space
  Token = 0x00000018 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0xFFC41000

Default Token Space
  Token = 0x00000019 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0xFFC00000

Default Token Space
  Token = 0x0000001A - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x7B6E000

Default Token Space
  Token = 0x0000001B - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x0

Default Token Space
  Token = 0x0000001C - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x280

Default Token Space
  Token = 0x0000001D - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x1E0

Default Token Space
  Token = 0x0000001E - Type = UINT8:DYNAMIC     - Size = 0x1 - Value = 0x0

Default Token Space
  Token = 0x0000001F - Type = UINT16:DYNAMIC    - Size = 0x2 - Value = 0x208

Default Token Space
  Token = 0x00000020 - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = FALSE

Default Token Space
  Token = 0x00000021 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x320

Default Token Space
  Token = 0x00000022 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x258

Default Token Space
  Token = 0x00000023 - Type = UINT16:DYNAMIC    - Size = 0x2 - Value = 0x0

Default Token Space
  Token = 0x00000024 - Type = UINT8:DYNAMIC     - Size = 0x1 - Value = 0x1

Default Token Space
  Token = 0x00000025 - Type = UINT8:DYNAMIC     - Size = 0x1 - Value = 0x1

Default Token Space
  Token = 0x00000026 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0xCAFECAFE

Default Token Space
  Token = 0x00000027 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x0

Default Token Space
  Token = 0x00000028 - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = TRUE

Default Token Space
  Token = 0x00000029 - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = TRUE

A1AFF049-FDEB-442A-B320-13AB4CB72BBC
  Token = 0x00030005 - Type = POINTER:DYNAMICEX - Size = 0x1
  00000000: 00                                               *.*

A1AFF049-FDEB-442A-B320-13AB4CB72BBC
  Token = 0x00030004 - Type = UINT16:DYNAMICEX  - Size = 0x2 - Value = 0x0

A1AFF049-FDEB-442A-B320-13AB4CB72BBC
  Token = 0x00030006 - Type = UINT64:DYNAMICEX  - Size = 0x8 - Value = 0x0

150CAB53-AD47-4385-B5DD-BCFC76BACAF0
  Token = 0xAF35F3B2 - Type = UINT32:DYNAMICEX  - Size = 0x4 - Value = 0xBABEBABE
```

If the platfrom was generated with the `PCD_INFO_GENERATION = TRUE` define, this program would also display PCD name information:
```
FS0:\> DumpDynPcd.efi
Current system SKU ID: 0x0

gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiS3Enable
  Token = 0x00000001 - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = TRUE

gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvStoreReserved
  Token = 0x00000002 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x7EF4000

gEfiMdeModulePkgTokenSpaceGuid.PcdGhcbBase
  Token = 0x00000003 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x0

gEfiMdeModulePkgTokenSpaceGuid.PcdGhcbSize
  Token = 0x00000004 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x0

gEfiMdeModulePkgTokenSpaceGuid.PcdPteMemoryEncryptionAddressOrMask
  Token = 0x00000005 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x0

gEfiMdeModulePkgTokenSpaceGuid.PcdSetNxForStack
  Token = 0x00000006 - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = FALSE

gEfiSecurityPkgTokenSpaceGuid.PcdOptionRomImageVerificationPolicy
  Token = 0x00000007 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x0

gUefiCpuPkgTokenSpaceGuid.PcdCpuBootLogicalProcessorNumber
  Token = 0x00000008 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x0

gUefiCpuPkgTokenSpaceGuid.PcdCpuMaxLogicalProcessorNumber
  Token = 0x00000009 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x1

gUefiCpuPkgTokenSpaceGuid.PcdSevEsIsEnabled
  Token = 0x0000000A - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = FALSE

gUefiOvmfPkgTokenSpaceGuid.PcdOvmfHostBridgePciDevId
  Token = 0x0000000B - Type = UINT16:DYNAMIC    - Size = 0x2 - Value = 0x1237

gUefiOvmfPkgTokenSpaceGuid.PcdPciIoBase
  Token = 0x0000000C - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0xC000

gUefiOvmfPkgTokenSpaceGuid.PcdPciIoSize
  Token = 0x0000000D - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x4000

gUefiOvmfPkgTokenSpaceGuid.PcdPciMmio32Base
  Token = 0x0000000E - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x80000000

gUefiOvmfPkgTokenSpaceGuid.PcdPciMmio32Size
  Token = 0x0000000F - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x7C000000

gUefiOvmfPkgTokenSpaceGuid.PcdPciMmio64Base
  Token = 0x00000010 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x800000000

gUefiOvmfPkgTokenSpaceGuid.PcdPciMmio64Size
  Token = 0x00000011 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x800000000

gUefiOvmfPkgTokenSpaceGuid.PcdQ35SmramAtDefaultSmbase
  Token = 0x00000012 - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = FALSE

gUefiOvmfPkgTokenSpaceGuid.PcdQ35TsegMbytes
  Token = 0x00000013 - Type = UINT16:DYNAMIC    - Size = 0x2 - Value = 0x8

gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase
  Token = 0x00000017 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0xFFC42000

gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase
  Token = 0x00000018 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0xFFC41000

gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase64
  Token = 0x00000019 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0xFFC00000

gEfiMdeModulePkgTokenSpaceGuid.PcdS3BootScriptTablePrivateDataPtr
  Token = 0x0000001A - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x7B6E000

gEfiMdeModulePkgTokenSpaceGuid.PcdS3BootScriptTablePrivateSmmDataPtr
  Token = 0x0000001B - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x0

gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution
  Token = 0x0000001C - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x280

gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution
  Token = 0x0000001D - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x1E0

gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosDocRev
  Token = 0x0000001E - Type = UINT8:DYNAMIC     - Size = 0x1 - Value = 0x0

gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosVersion
  Token = 0x0000001F - Type = UINT16:DYNAMIC    - Size = 0x2 - Value = 0x208

gEfiMdeModulePkgTokenSpaceGuid.PcdTestKeyUsed
  Token = 0x00000020 - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = FALSE

gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution
  Token = 0x00000021 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x320

gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution
  Token = 0x00000022 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x258

gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut
  Token = 0x00000023 - Type = UINT16:DYNAMIC    - Size = 0x2 - Value = 0x0

gEfiNetworkPkgTokenSpaceGuid.PcdIPv4PXESupport
  Token = 0x00000024 - Type = UINT8:DYNAMIC     - Size = 0x1 - Value = 0x1

gEfiNetworkPkgTokenSpaceGuid.PcdIPv6PXESupport
  Token = 0x00000025 - Type = UINT8:DYNAMIC     - Size = 0x1 - Value = 0x1

gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32
  Token = 0x00000026 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0xCAFECAFE

gUefiOvmfPkgTokenSpaceGuid.PcdEmuVariableEvent
  Token = 0x00000027 - Type = UINT64:DYNAMIC    - Size = 0x8 - Value = 0x0

gUefiOvmfPkgTokenSpaceGuid.PcdOvmfFlashVariablesEnable
  Token = 0x00000028 - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = TRUE

gUefiOvmfPkgTokenSpaceGuid.PcdQemuSmbiosValidated
  Token = 0x00000029 - Type = BOOLEAN:DYNAMIC   - Size = 0x1 - Value = TRUE

gEfiMdeModulePkgTokenSpaceGuid.PcdNvStoreDefaultValueBuffer
  Token = 0x00030005 - Type = POINTER:DYNAMICEX - Size = 0x1
  00000000: 00                                               *.*

gEfiMdeModulePkgTokenSpaceGuid.PcdSetNvStoreDefaultId
  Token = 0x00030004 - Type = UINT16:DYNAMICEX  - Size = 0x2 - Value = 0x0

gEfiMdeModulePkgTokenSpaceGuid.PcdVpdBaseAddress64
  Token = 0x00030006 - Type = UINT64:DYNAMICEX  - Size = 0x8 - Value = 0x0

gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32
  Token = 0xAF35F3B2 - Type = UINT32:DYNAMICEX  - Size = 0x4 - Value = 0xBABEBABE
```

In this case it is also possible to access individual PCDs by name:
```
FS0:\> DumpDynPcd.efi gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32
Current system SKU ID: 0x0

gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32
  Token = 0xAF35F3B2 - Type = UINT32:DYNAMICEX  - Size = 0x4 - Value = 0xBABEBABE
```

# Summary

Here are all the PCD functions. You can find their definition in the [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h):
```
FixedPcdGetBool (TokenName)
FixedPcdGet8    (TokenName)
FixedPcdGet16   (TokenName)
FixedPcdGet32   (TokenName)
FixedPcdGet64   (TokenName)
FixedPcdGetPtr  (TokenName)
FixedPcdGetSize (TokenName)

----------------------------------------------------------------------------------------------------

FeaturePcdGet (TokenName)

----------------------------------------------------------------------------------------------------

PatchPcdGetBool (TokenName)              PatchPcdSetBool (TokenName, Value)
PatchPcdGet8    (TokenName)              PatchPcdSet8    (TokenName, Value)
PatchPcdGet16   (TokenName)              PatchPcdSet16   (TokenName, Value)
PatchPcdGet32   (TokenName)              PatchPcdSet32   (TokenName, Value)
PatchPcdGet64   (TokenName)              PatchPcdSet64   (TokenName, Value)
PatchPcdGetPtr  (TokenName)              PatchPcdSetPtr  (TokenName, Size, Buffer)
PatchPcdGetSize (TokenName)

----------------------------------------------------------------------------------------------------

PcdGetBool      (TokenName)              PcdSetBoolS     (TokenName, Value)
PcdGet8         (TokenName)              PcdSet8S        (TokenName, Value)
PcdGet16        (TokenName)              PcdSet16S       (TokenName, Value)
PcdGet32        (TokenName)              PcdSet32S       (TokenName, Value)
PcdGet64        (TokenName)              PcdSet64S       (TokenName, Value)
PcdGetPtr       (TokenName)              PcdSetPtrS      (TokenName, SizeOfBuffer, Buffer)
PcdGetSize      (TokenName)

----------------------------------------------------------------------------------------------------

PcdGetExBool    (Guid, TokenName)        PcdSetExBoolS   (Guid, TokenName, Value)
PcdGetEx8       (Guid, TokenName)        PcdSetEx8S      (Guid, TokenName, Value)
PcdGetEx16      (Guid, TokenName)        PcdSetEx16S     (Guid, TokenName, Value)
PcdGetEx32      (Guid, TokenName)        PcdSetEx32S     (Guid, TokenName, Value)
PcdGetEx64      (Guid, TokenName)        PcdSetEx64S     (Guid, TokenName, Value)
PcdGetExPtr     (Guid, TokenName)        PcdSetExPtrS    (Guid, TokenName, SizeOfBuffer, Buffer)
PcdGetExSize    (Guid, TokenName)
```

Summary table:

|               | Fixed at build                    | Feature Flag                      | Patchable                          | Dynamic                                                      | DynamicEx                                                          |
|---------------|-----------------------------------|-----------------------------------|------------------------------------|--------------------------------------------------------------|--------------------------------------------------------------------|
| .dec section: | [PcdsFixedAtBuild]                | [PcdsFeatureFlag]                 | [PcdsPatchableInModule]            | [PcdsDynamic]                                                | [PcdsDynamicEx]                                                    |
| .dsc section: | [PcdsFixedAtBuild]                | [PcdsFeatureFlag]                 | [PcdsPatchableInModule]            | [PcdsDynamicDefault]<br>[PcdsDynamicHii]<br>[PcdsDynamicVpd] | [PcdsDynamicExDefault]<br>[PcdsDynamicExHii]<br>[PcdsDynamicExVpd] |
| .inf section: | [FixedPcd]<br>[Pcd]               | [FeaturePcd]<br>[Pcd]             | [PatchPcd]<br>[Pcd]                | [Pcd]                                                        | [PcdEx]                                                            |
| Get:          | `PcdGet<Type>`<br>`FixedPcdGet<Type>` | `PcdGetBool`<br>`FeaturePcdGet`       | `PcdGet<Type>`<br>`PatchPcdGet<Type>`  | `PcdGet<Type>`                                                 | `PcdGet<Type>`<br>`PcdGetEx<Type>`                                     |
| Set:          | -                                 | -                                 | `PcdSet<Type>S`<br>`PatchPcdSet<Type>` | `PcdSet<Type>S`                                                | `PcdSet<Type>S`<br>`PcdSetEx<Type>S`                                   |
| Scope:        | Every module have its own copy | Every module have its own copy | Every module have its own copy  | Global for<br>platform                                       | Global for<br>platform                                             |


\<Type\> = 8|16|32|64|Bool|Ptr
