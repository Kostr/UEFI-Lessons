In the PCD section naming the second level of a name override is a SKU override.

SKU means `stock-keeping unit`. It is an identifier for the particular product. Not to the individual uniq product item but to the product as a class. For example your company develops two board variations - one with 4 PCI connectors, and another one with 3 PCIe connectors. In this case each of these board variations would have its own SKU number.

In this lesson we'll investigate how EDKII can be customized for the different SKUs.

First of all in the platfrom every supported SKU must be declared in the DSC file in the `[SkuIds]` section. Each declaration goes as a `value|name` pair. Implicitly the `0|DEFAULT` SKU is always present. That is why we didn't declare this section in our `UefiLessonsPkg/UefiLessonsPkg.dsc` file. But if you want to, you can declare it explicitly:
```
[SkuIds]
  0|DEFAULT
```

The target SKU is chosen by the `SKUID_IDENTIFIER` in the `[Defines]` section. Currently it is set to the `DEFAULT` SKU:
```
 [Defines]
   ...
   SKUID_IDENTIFIER = DEFAULT
   ...
```

But if you want to, you can override it via an argument to the `build` command:
```
$ build --help
...
  -x SKUID, --sku-id=SKUID
                        Using this name of SKU ID to build the platform,
                        overriding SKUID_IDENTIFIER in DSC file.
...
```

Now let's add couple of other custom SKUs:
```
[SkuIds]
  0|DEFAULT
  0x55|MySKU1
  0x66|MySKU2
```

And specify the PCD override section to the `MySKU1`:
```
[PcdsFixedAtBuild.common.MySKU1]
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|44
```

Now if you execute our ordinary build, the PCD wouldn't be overriden:
```
$ build
...
$ grep _PCD_VALUE_PcdInt32Override Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
#define _PCD_VALUE_PcdInt32Override  43U
```

But if you specify the correct SKU, the override would be applied. You can do by changing the `SKUID_IDENTIFIER` define value or by providing an argument to the `build` utility. Let's take the second approach:
```
$ build --sku-id=MySKU1
...
$ grep _PCD_VALUE_PcdInt32Override Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
#define _PCD_VALUE_PcdInt32Override  44U
```

Like with the ARCH override the `common` specifier can be used to indicate any SKU:
```
[PcdsFixedAtBuild.common.common]
```

# Multiple SKUs build

It is possible to set many SKUs in the `SKUID_IDENTIFIER` like this:
```
SKUID_IDENTIFIER = MySKU1|MySKU2
```
Unfortunately currently it is not possible to set multiple SKUs by `build` command arguments:
```
$ build -x MySKU1 -x MySKU2
```
We will see how the multiple SKUs build can be usefull next when we would talk about DynamicPCDs, but as for the `[PcdsFixedAtBuild]`/`[FeatureFlag]`/`[PatchableInModule]` sections, setting multiple SKUs in the `SKUID_IDENTIFIER` would cancel all specific SKU overrides.

If you want to set multiple SKUs build to all the SKUs in the `[SkuIds]` section, it is possible to use the `ALL` keyword:
```
SKUID_IDENTIFIER = ALL
```

# SKU deltas for DynamicPCD

As we would investigate the PCD Database, let's look at the `OvmfPkg/OvmfPkgX64.dsc`:
```
[Defines]
  ...
  SKUID_IDENTIFIER = DEFAULT
  ...

[SkuIds]
  0|DEFAULT
```

As you see currectly it has only one SKU, let's add couple more:
```
[Defines]
  ...
  SKUID_IDENTIFIER = ALL
  ...

[SkuIds]
  0|DEFAULT
  0x55|MySKU1
  0x66|MySKU2
```

Rebuild OVMF:
```
$ build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

If you check the `parse_pcd_db` output you could see that information about all possible SKUs is encoded in the PCD database files:
```
$ parse_pcd_db \
    --peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
    --dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw"

PEI PCD DB

Available SKUs: 0x0 0x55 0x66
_____

...

_____

DXE PCD DB

Available SKUs: 0x0 0x55 0x66
____

...
```

Now let's add our `PCDLesson` application DynamicPCDs to the OVMF build. In case you don't remember for that we need to add the application as a component to the DSC file (`OvmfPkg/OvmfPkgX64.dsc`):
```
[Components]
  UefiLessonsPkg/PCDLesson/PCDLesson.inf
  ...
```

As well as add it to the flash image via the `OvmfPkg/OvmfPkgX64.fdf` file:
```
[FV.DXEFV]
...
INF  UefiLessonsPkg/PCDLesson/PCDLesson.inf
```

In our example we would look at the `PcdDynamicExInt32` DynamicEx PCD from the `UefiLessonsPkg/UefiLessonsPkg.dec`:
```
[PcdsDynamicEx]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|0xBABEBABE|UINT32|0xAF35F3B2
```

Let's define couple of overrides for different SKUs in the platform DSC file `OvmfPkg/OvmfPkgX64.dsc`:
```
[PcdsDynamicExDefault.common.MySKU1]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|0x11111111

[PcdsDynamicExDefault.common.MySKU2]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|0x22222222
```

Rebuild OVMF:
```
$ build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

Now let's investigate PCD Database files. If you execute `parse_pcd_db` like usual, you would see that `PcdDynamicExInt32` has its default value `0xBABEBABE` from the DEC file:
```
$ parse_pcd_db \
    --peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
    --dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw"

...

42:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0xaf35f3b2
DynamicEx GUID  = 150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
Value:
0xbabebabe (=3133061822)
```

But if you supply `--sku=0x55` argument, the value would be shown as the one from the `PcdsDynamicExDefault.common.MySKU1` section override:
```
$ parse_pcd_db \
    --peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
    --dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw" \
    --sku=0x55

...

42:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0xaf35f3b2
DynamicEx GUID  = 150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
Value:
0x11111111 (=286331153)
```

The same way `--sku=0x66` argument will give you the value from the `PcdsDynamicExDefault.common.MySKU2` section override:
```
$ parse_pcd_db \
    --peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
    --dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw" \
    --sku=0x66

...

42:
Token type = Data
Datum type = UINT32
DynamicEx Token = 0xaf35f3b2
DynamicEx GUID  = 150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
Value:
0x22222222 (=572662306)
```

So the point here is that information for every SKU override is encoded in the PCD Database.


Check PI specification:
```
#define EFI_PCD_PROTOCOL_GUID \
 { 0x13a3f0f6, 0x264a, 0x3ef0, \
 { 0xf2, 0xe0, 0xde, 0xc5, 0x12, 0x34, 0x2f, 0x34 } }
```

EFI_PCD_PROTOCOL.SetSku ()

Summary:
Sets the SKU value for subsequent calls to set or get PCD token values.\

Prototype:
typedef
VOID
(EFIAPI *EFI_PCD_PROTOCOL_SET_SKU) (
 IN UINTN SkuId
 );

Parameters:
SkuId		The SKU value to set.

Description:
SetSku() sets the SKU Id to be used for subsequent calls to set or get PCD values. SetSku() is normally called only once by the system.
For each item (token), the database can hold a single value that applies to all SKUs, or multiple values, where each value is associated with a specific SKU Id. Items with multiple, SKU-specific values are called SKU enabled.
The SKU Id of zero is reserved as a default. For tokens that are not SKU enabled, the system ignores any set SKU Id and works with the single value for that token. For SKU-enabled tokens, the system will use the SKU Id set by the last call to SetSku(). If no SKU Id is set or the currently set SKU Id isn’t valid for the specified token, the system uses the default SKU Id. If the system attempts to use the default SKU Id and no value has been set for that Id, the results are unpredictable.
```

```
EFI_PEI_PCD_PPI.SetSku ()

Summary:
Sets the SKU value for subsequent calls to set or get PCD token values.

Prototype:
typedef
VOID
(EFIAPI *EFI_PEI_PCD_PPI_SET_SKU) (
 IN UINTN SkuId
 );

Parameters:
SkuId		The SKU value to set.

Description:
 ...
```


# Links

- [https://edk2-docs.gitbook.io/edk-ii-build-specification/2_design_discussion/27_sku_support](https://edk2-docs.gitbook.io/edk-ii-build-specification/2_design_discussion/27_sku_support)

