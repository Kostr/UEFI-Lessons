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

In the nutshell each Sku override is represented as an array of `PCD_DATA_DELTA` structures:
```
typedef struct {
  UINT32    Offset : 24;
  UINT32    Value  : 8;
} PCD_DATA_DELTA;
```
Each of these structures mean "change PCD Database byte at the `PCD_DATA_DELTA.Offset` to the `PCD_DATA_DELTA.Value`". So it is 4 bytes record to change 1 byte of the PCD Database.

# Changing SKU at runtime

The whole point of this part of SKU functionality is to change group of PCDs at runtime. So let's investigate how SKU change affect PCD values.

We need to write an application that could change current SKU value.

For this we would need a function from the `EFI_PCD_PROTOCOL` defined in the PI specification:
```
EFI_PCD_PROTOCOL

Summary:
A platform database that contains a variety of current platform settings or directives that can be accessed by a driver or application.

GUID:

#define EFI_PCD_PROTOCOL_GUID \
 { 0x13a3f0f6, 0x264a, 0x3ef0, \
 { 0xf2, 0xe0, 0xde, 0xc5, 0x12, 0x34, 0x2f, 0x34 } }


Protocol Interface Structure:

typedef struct _EFI_PCD_PROTOCOL {
 EFI_PCD_PROTOCOL_SET_SKU SetSku;
 EFI_PCD_PROTOCOL_GET_8 Get8;
 EFI_PCD_PROTOCOL_GET_16 Get16;
 EFI_PCD_PROTOCOL_GET_32 Get32;
 EFI_PCD_PROTOCOL_GET_64 Get64;
 EFI_PCD_PROTOCOL_GET_POINTER GetPtr;
 EFI_PCD_PROTOCOL_GET_BOOLEAN GetBool;
 EFI_PCD_PROTOCOL_GET_SIZE GetSize;
 EFI_PCD_PROTOCOL_SET_8 Set8;
 EFI_PCD_PROTOCOL_SET_16 Set16;
 EFI_PCD_PROTOCOL_SET_32 Set32;
 EFI_PCD_PROTOCOL_SET_64 Set64;
 EFI_PCD_PROTOCOL_SET_POINTER SetPtr;
 EFI_PCD_PROTOCOL_SET_BOOLEAN SetBool;
 EFI_PCD_PROTOCOL_CALLBACK_ON_SET CallbackOnSet;
 EFI_PCD_PROTOCOL_CANCEL_CALLBACK CancelCallback;
 EFI_PCD_PROTOCOL_GET_NEXT_TOKEN GetNextToken;
 EFI_PCD_PROTOCOL_GET_NEXT_TOKEN_SPACE GetNextTokenSpace;
} EFI_PCD_PROTOCOL;
```

This is the same protocol that is implicitly used in library calls when you access Dynamic and DynamicEx PCDs.

But right now we are interested in the `SetSku` function:
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

To get the current SkuId value we would use another protocol - `EFI_GET_PCD_INFO_PROTOCOL`:
```
EFI_GET_PCD_INFO_PROTOCOL

Summary:
The protocol that provides additional information about items that reside in the PCD database.

GUID:
#define EFI_GET_PCD_INFO_PROTOCOL_GUID \
 { 0xfd0f4478, 0xefd, 0x461d, \
 { 0xba, 0x2d, 0xe5, 0x8c, 0x45, 0xfd, 0x5f, 0x5e } }


Protocol Interface Structure:

typedef struct _EFI_GET_PCD_INFO_PROTOCOL {
 EFI_GET_PCD_INFO_PROTOCOL_GET_INFO GetInfo;
 EFI_GET_PCD_INFO_PROTOCOL_GET_SKU GetSku;
} EFI_GET_PCD_INFO_PROTOCOL;
```

Particularly its function `GetSku`:
```
EFI_GET_PCD_INFO_PROTOCOL.GetSku ()

Summary:
Retrieve the currently set SKU Id.

Prototype:
typedef
UINTN
(EFIAPI *EFI_GET_PCD_INFO_PROTOCOL_GET_SKU) (
 VOID
 );

Description:
GetSku() returns the currently set SKU Id. If the platform has not set at a SKU Id, then the default SKU Id value of 0 is returned. If the platform has set a SKU Id, then the currently set SKU Id is returned.
```

Now let's write a shell application `SetSku` that sets SkuId based on the input argument:
```cpp
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Pi/PiMultiPhase.h>
#include <Protocol/PiPcd.h>
#include <Protocol/PiPcdInfo.h>


VOID Usage()
{
  Print(L"Program to set Sku number\n\n");
  Print(L"  Usage: SetSku <SkuNumber in hex>\n");
}

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS Status;

  if (Argc != 2) {
    Usage();
    return EFI_INVALID_PARAMETER;
  }

  UINTN SkuNumber;
  RETURN_STATUS ReturnStatus;
  ReturnStatus = StrHexToUintnS(Argv[1], NULL, &SkuNumber);
  if (RETURN_ERROR(ReturnStatus)) {
    Print(L"Error! Can't convert SkuId string to number\n");
    return EFI_INVALID_PARAMETER;
  }

  EFI_PCD_PROTOCOL* pcdProtocol;
  Status = gBS->LocateProtocol(&gEfiPcdProtocolGuid,
                               NULL,
                               (VOID **)&pcdProtocol);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't find EFI_PCD_PROTOCOL\n");
    return EFI_UNSUPPORTED;
  }

  EFI_GET_PCD_INFO_PROTOCOL* getPcdInfoProtocol;
  Status = gBS->LocateProtocol(&gEfiGetPcdInfoProtocolGuid,
                               NULL,
                               (VOID **)&getPcdInfoProtocol);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't find EFI_GET_PCD_INFO_PROTOCOL\n");
    return EFI_UNSUPPORTED;
  }


  pcdProtocol->SetSku(SkuNumber);

  UINTN SkuId = getPcdInfoProtocol->GetSku();
  if (SkuId != SkuNumber) {
    Print(L"Failed to change SkuId to 0x%lx, SkuId is still 0x%lx\n", SkuNumber, SkuId);
    return EFI_UNSUPPORTED;
  }

  Print(L"Sku is changed successfully to 0x%lx\n", SkuNumber);

  return EFI_SUCCESS;
}
```

The program is very simple, so I hope it doesn't need any comments.

Let's build it and use under UEFI shell.

First with a help of `DumpDynPcd` application check initial value for our PCD:
```
FS0:\> DumpDynPcd.efi
...
150CAB53-AD47-4385-B5DD-BCFC76BACAF0
  Token = 0xAF35F3B2 - Type = UINT32:DYNAMICEX  - Size = 0x4 - Value = 0xBABEBABE
```
Now use `SetSku` to change SkuId:
```
FS0:\> SetSku.efi 0x55
Sku is changed successfully to 0x55
```
And verify PCD value again:
```
FS0:\> DumpDynPcd.efi
...
150CAB53-AD47-4385-B5DD-BCFC76BACAF0
  Token = 0xAF35F3B2 - Type = UINT32:DYNAMICEX  - Size = 0x4 - Value = 0x11111111
```
You can see that the PCD value was affected by the SkuId set operation. This is the main point of SkuId change.

Usually the firmware on boot checks some hardware configuration which defines different board variants (for example hardware configuration can be encoded as the input value on some GPIO pins). Based on the decoded hardware configuration firmware sets SkuId, thus loading a whole new set of PCDs that belong to this particular board variant. This way it is possible to have common firmware for different boards.

# SkuId can be set only once

Let's get back to the SkuId mechanics investigation. First point is that you can set SkuId only once at boot. If you try to execute `SetSku` application again it would fail:
```
FS0:\> SetSku.efi 0x66
Failed to change SkuId to 0x66, SkuId is still 0x55
```
And the PCD value wouldn't change to the new Sku override:
```
FS0:\> DumpDynPcd.efi
...
150CAB53-AD47-4385-B5DD-BCFC76BACAF0
  Token = 0xAF35F3B2 - Type = UINT32:DYNAMICEX  - Size = 0x4 - Value = 0x11111111
```
Initial call to our application have worked just because OVMF firmware didn't perform `EFI_PCD_PROTOCOL->SetSku` on boot.

If you want to see how this check is done in EDKII code check [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Pcd.c](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Pcd.c):
```cpp
VOID
EFIAPI
DxePcdSetSku (
  IN  UINTN  SkuId
  )
{
...
  if (mPcdDatabase.DxeDb->SystemSkuId != (SKU_ID)0) {
    DEBUG ((DEBUG_ERROR, "PcdDxe - The SKU Id could be changed only once."));
    DEBUG ((
      DEBUG_ERROR,
      "PcdDxe - The SKU Id was set to 0x%lx already, it could not be set to 0x%lx any more.",
      mPcdDatabase.DxeDb->SystemSkuId,
      (SKU_ID)SkuId
      ));
    ASSERT (FALSE);
    return;
  }
...
}
```

You can verify that it is still possible to change SkuId to the 0x66 value if you do it first. Restart QEMU and execute these operations:
```
FS0:\> DumpDynPcd.efi
...
150CAB53-AD47-4385-B5DD-BCFC76BACAF0
  Token = 0xAF35F3B2 - Type = UINT32:DYNAMICEX  - Size = 0x4 - Value = 0xBABEBABE

FS0:\> SetSku.efi 0x66
Sku is changed successfully to 0x66

FS0:\> DumpDynPcd.efi
...
150CAB53-AD47-4385-B5DD-BCFC76BACAF0
  Token = 0xAF35F3B2 - Type = UINT32:DYNAMICEX  - Size = 0x4 - Value = 0x22222222
```

So as you can see, the PCD value was successfully changed to the override of the SkuId 0x66.

# Incorrect SkuId

Restart QEMU and try to set SkuId outside of the available ones:
```
FS0:\> SetSku 0x99
Failed to change SkuId to 0x99, SkuId is still 0x0
```
Here you can see that the SkuId wouldn't be changed if you supply unknown SkuId value.


# Changing SkuId after PCD change

What will happen if we will change PCD value from its default and then change SkuId?

We can verify this with our `PCDLesson.efi` application as it changes the value for the PCD at runtime:
```
FS0:\> DumpDynPcd.efi
...
150CAB53-AD47-4385-B5DD-BCFC76BACAF0
  Token = 0xAF35F3B2 - Type = UINT32:DYNAMICEX  - Size = 0x4 - Value = 0xBABEBABE

FS0:\> PCDLesson.efi

FS0:\> DumpDynPcd.efi
...
150CAB53-AD47-4385-B5DD-BCFC76BACAF0
  Token = 0xAF35F3B2 - Type = UINT32:DYNAMICEX  - Size = 0x4 - Value = 0x88888888

FS0:\> SetSku.efi 0x55
Sku is changed successfully to 0x55

FS0:\> DumpDynPcd.efi
...
150CAB53-AD47-4385-B5DD-BCFC76BACAF0
  Token = 0xAF35F3B2 - Type = UINT32:DYNAMICEX  - Size = 0x4 - Value = 0x11111111
```

As you can see the SkuId change sets any affected PCDs, even if they were changed before.

# PEI stage

Our `SetSku` application is supposed to be launched in the DXE stage, so we were using the PCD database protocols for the DXE stage.

But the PCD database is also used in the PEI stage. So there is another PCD database protocol for the PEI stage - `EFI_PEI_PCD_PPI`:
```
#define EFI_PEI_PCD_PPI_GUID \
 { 0x1f34d25, 0x4de2, 0x23ad, \
 { 0x3f, 0xf3, 0x36, 0x35, 0x3f, 0xf3, 0x23, 0xf1 } }

typedef struct {
 EFI_PEI_PCD_PPI_SET_SKU SetSku;
 EFI_PEI_PCD_PPI_GET_8 Get8;
 EFI_PEI_PCD_PPI_GET_16 Get16;
 EFI_PEI_PCD_PPI_GET_32 Get32;
 EFI_PEI_PCD_PPI_GET_64 Get64;
 EFI_PEI_PCD_PPI_GET_POINTER GetPtr;
 EFI_PEI_PCD_PPI_GET_BOOLEAN GetBool;
 EFI_PEI_PCD_PPI_GET_SIZE GetSize;
 EFI_PEI_PCD_PPI_SET_8 Set8;
 EFI_PEI_PCD_PPI_SET_16 Set16;
 EFI_PEI_PCD_PPI_SET_32 Set32;
 EFI_PEI_PCD_PPI_SET_64 Set64;
 EFI_PEI_PCD_PPI_SET_POINTER SetPtr;
 EFI_PEI_PCD_PPI_SET_BOOLEAN SetBool;
 EFI_PEI_PCD_PPI_CALLBACK_ON_SET CallbackOnSet;
 EFI_PEI_PCD_PPI_CANCEL_CALLBACK CancelCallback;
 EFI_PEI_PCD_PPI_GET_NEXT_TOKEN GetNextToken;
 EFI_PEI_PCD_PPI_GET_NEXT_TOKEN_SPACE GetNextTokenSpace;
} EFI_PEI_PCD_PPI;
```

The `SetSku` function is similar to the one that we've used:
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

The same goes to the `EFI_GET_PCD_INFO_PROTOCOL` and its `GetSku` function. Here are their analogs for the PEI stage:
```
#define EFI_GET_PCD_INFO_PPI_GUID \
 { 0xa60c6b59, 0xe459, 0x425d, \
 { 0x9c, 0x69, 0xb, 0xcc, 0x9c, 0xb2, 0x7d, 0x81 } }

typedef struct _EFI_GET_PCD_INFO_PPI {
 EFI_GET_PCD_INFO_PPI_GET_INFO GetInfo;
 EFI_GET_PCD_INFO_PPI_GET_SKU GetSku;
} EFI_GET_PCD_INFO_PPI;
```
```
EFI_GET_PCD_INFO_PPI.GetSku ()

Summary:
Retrieve the currently set SKU Id.

Prototype:

typedef
UINTN
(EFIAPI *EFI_GET_PCD_INFO_PPI_GET_SKU) (
 VOID
 );


Description:
GetSku() returns the currently set SKU Id. If the platform has not set at a SKU Id, then the default SKU Id value of 0 is returned. If the platform has set a SKU Id, then the currently set SKU Id is returned.
```

Sometimes it is necessary to get correct PCDs for the target SKU as much early as possible. This is why often the firmware sets the SkuId exactly in the PEI phase using the functions above.

# `LibPcdSetSku`/`LibPcdGetSku`

The PCD library has couple of functions that simplify SKU get/set operations [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h):
```cpp
/**
  This function provides a means by which SKU support can be established in the PCD infrastructure.
  Sets the current SKU in the PCD database to the value specified by SkuId.  SkuId is returned.
  @param  SkuId   The SKU value that will be used when the PCD service retrieves and sets values
                  associated with a PCD token.
  @return  Return the SKU ID that was set.
**/
UINTN
EFIAPI
LibPcdSetSku (
  IN UINTN  SkuId
  );
```
```cpp
/**
  Retrieve the currently set SKU Id.
  @return   The currently set SKU Id. If the platform has not set at a SKU Id, then the
            default SKU Id value of 0 is returned. If the platform has set a SKU Id, then the currently set SKU
            Id is returned.
**/
UINTN
EFIAPI
LibPcdGetSku (
  VOID
  );
```

The functions have different realization in PEI [https://github.com/tianocore/edk2/blob/master/MdePkg/Library/PeiPcdLib/PeiPcdLib.c](https://github.com/tianocore/edk2/blob/master/MdePkg/Library/PeiPcdLib/PeiPcdLib.c) and in DXE [https://github.com/tianocore/edk2/blob/master/MdePkg/Library/DxePcdLib/DxePcdLib.c](https://github.com/tianocore/edk2/blob/master/MdePkg/Library/DxePcdLib/DxePcdLib.c), since the PCD datbase protocol is different in these phases. But basically this library functions call the same PCD protocol functions that we did in our example program.

Using these functions we can simplify our application:
```cpp
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PcdLib.h>

VOID Usage()
{
  Print(L"Program to set Sku number\n\n");
  Print(L"Usage: SetSku <SkuNumber>\n");
}

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  if (Argc != 2) {
    Usage();
    return EFI_INVALID_PARAMETER;
  }

  UINTN SkuNumber;
  RETURN_STATUS ReturnStatus;
  ReturnStatus = StrHexToUintnS(Argv[1], NULL, &SkuNumber);
  if (RETURN_ERROR(ReturnStatus)) {
    Print(L"Error! Can't convert SkuId string to number\n");
    return EFI_INVALID_PARAMETER;
  }

  LibPcdSetSku(SkuNumber);
  UINTN SkuId = LibPcdGetSku();
  if (SkuId != SkuNumber) {
    Print(L"Failed to change SkuId to %ld, SkuId is still %ld\n", SkuNumber, SkuId);
    return EFI_UNSUPPORTED;
  }

  Print(L"Sku is changed successfully to %ld\n", SkuNumber);

  return EFI_SUCCESS;
}
```

# Links

- [https://edk2-docs.gitbook.io/edk-ii-build-specification/2_design_discussion/27_sku_support](https://edk2-docs.gitbook.io/edk-ii-build-specification/2_design_discussion/27_sku_support)

