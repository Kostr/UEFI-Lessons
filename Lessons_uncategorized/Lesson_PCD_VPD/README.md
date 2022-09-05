In this lesson we are going to look at the last Dynamic PCD storage method - `VPD`.

VPD stands for "Vital Product Data". In this context it is a read-only file in the firmware image which contains VPD PCDs.

To mark dynamic PCDs as VPD PCDs, you have to list them under `[PcdsDynamicVpd]`/`[PcdsDynamicExVpd]` sections in the DSC file. The format of the PCDs of this kind looks like this:
```
<TokenGuid>.<TokenName>|<VPD offset>[|<Override value>]
```

For example add this override for the PCDs from the `PCDLesson` application to the `OvmfPkg/OvmfPkgX64.dsc` file:
```
[PcdsDynamicVpd]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32|7|0x44444444

[PcdsDynamicExVpd]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|16|0x55555555
```

If you'll try to build OVMF as-is, you would encounter build failure:
```
$ build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5

...

build.py...
 : error 000E: Fail to find third-party BPDG tool to process VPD PCDs. BPDG Guid tool need to be defined in tools_def.txt and VPD_TOOL_GUID need to be provided in DSC file.
```

This is happening because the tool for read-only VPD file creation is not set. The EDKII has one such tool included in itself - it is BPDG tool. It is already defined in the `Conf/tools_def.txt` file:
```
##################
# BPDG tool definitions
##################
*_*_*_VPDTOOL_PATH         = BPDG
*_*_*_VPDTOOL_GUID         = 8C3D856A-9BE6-468E-850A-24F7A8D38E08
```
The tool `VPDTOOL` is a Python module from the folder [`BaseTools/Source/Python/BPDG/`](https://github.com/tianocore/edk2/blob/master/BaseTools/Source/Python/BPDG/) which is used via [`BaseTools/BinWrappers/PosixLike/BPDG`](https://github.com/tianocore/edk2/blob/master/BaseTools/BinWrappers/PosixLike/BPDG) wrapper on Linux. As a wrapper folder `BaseTools/BinWrappers/PosixLike` gets added to the environment `PATH` variable after the `. edksetup.sh` command, we can simply declare path for `VPDTOOL` as `BPDG`. You can check this application in your build environment:
```
$ BPDG
Please specify the filename.txt file which contain the VPD pcd info!
BPDG options -o Filename.bin -m Filename.map Filename.txt
Copyright (c) 2010 - 2018, Intel Corporation All Rights Reserved.

  Intel(r) Binary Product Data Generation Tool (Intel(r) BPDG)

Required Flags:
  -o BIN_FILENAME, --vpd-filename=BIN_FILENAME
            Specify the file name for the VPD binary file
  -m FILENAME, --map-filename=FILENAME
            Generate file name for consumption during the build that contains
            the mapping of Pcd name, offset, datum size and value derived
            from the input file and any automatic calculations.
```

If we want to create VPD file we need to add a special define `VPD_TOOL_GUID` to the DSC `[Defines]` section. This define determines a tool which would be used for the VPD generation. Let's use the aforemetioned `BPDG` tool for this. Add this code to the `OvmfPkg/OvmfPkgX64.dsc` file:
```
[Defines]
  ...
  VPD_TOOL_GUID                  = 8C3D856A-9BE6-468E-850A-24F7A8D38E08
```

The used GUID corresponds to the GUID of the tool `VPDTOOL` defined in the `Conf/tools_def.txt`

Now the OVMF build should succeed.

Check out PCD Database files with the `parse_pcd_db` application:
```
$ parse_pcd_db \
  --peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
  --dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw" 

...

38:
Token type = VPD
Datum type = UINT32
VPD offset = 0x00000007 (=7)
Size = 4
Provide VPD file to print actual data

...

42:
Token type = VPD
Datum type = UINT32
DynamicEx Token = 0xaf35f3b2
DynamicEx GUID  = 150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
VPD offset = 0x00000010 (=16)
Size = 4
Provide VPD file to print actual data
```

As you can see the actual PCD values are not encoded in the PCD Database itself. The database PCD records contain only the offsets inside the VPD file.

The VPD itself created as a separate binary in a build folder. You can check it with the `hexdump` and see that it contains our PCD values:
```
$ hexdump -C Build/OvmfX64/RELEASE_GCC5/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08.bin
00000000  00 00 00 00 00 00 00 44  44 44 44 00 00 00 00 00  |.......DDDD.....|
00000010  55 55 55 55                                       |UUUU|
00000014
```

`parse_pcd_db` application supports setting the VPD file via the command line, so you could see the actual PCD values:
```
$ parse_pcd_db \
  --peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
  --dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw" \
  --vpd Build/OvmfX64/RELEASE_GCC5/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08.bin

...

38:
Token type = VPD
Datum type = UINT32
VPD offset = 0x00000007 (=7)
Value:
0x44444444 (=1145324612)

...

42:
Token type = VPD
Datum type = UINT32
DynamicEx Token = 0xaf35f3b2
DynamicEx GUID  = 150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
VPD offset = 0x00000010 (=16)
Value:
0x55555555 (=1431655765)
```

# PCD Database access to the VPD data

Let's see how PCD Database driver gets VPD PCD data. Here is a code snippet from the [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Service.c](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Service.c) file:
```cpp
VOID *
GetWorker (
  IN UINTN  TokenNumber,
  IN UINTN  GetSize
  )
{
  ...
  switch (LocalTokenNumber & PCD_TYPE_ALL_SET) {
    case PCD_TYPE_VPD:
      ...
      RetPtr = (VOID *)(mVpdBaseAddress + VpdHead->Offset);
      break;

  ...
}
```

In this case `VpdHead->Offset` is an offset that is acquired from the PCD token. So it is a value from the PCD Database. The other variable `mVpdBaseAddress` is a start address of the VPD file in memory. This variable is set in the module initialization code [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Pcd.c](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Dxe/Pcd.c):
```cpp
EFI_STATUS
EFIAPI
PcdDxeInit (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  ...

  //
  // Cache VpdBaseAddress in entry point for the following usage.
  //

  //
  // PcdVpdBaseAddress64 is DynamicEx PCD only. So, DxePcdGet64Ex() is used to get its value.
  //
  mVpdBaseAddress = (UINTN)DxePcdGet64Ex (&gEfiMdeModulePkgTokenSpaceGuid, PcdToken (PcdVpdBaseAddress64));
  if (mVpdBaseAddress == 0) {
    //
    // PcdVpdBaseAddress64 is not set, get value from PcdVpdBaseAddress.
    //
    mVpdBaseAddress = (UINTN)PcdGet32 (PcdVpdBaseAddress);
  }

  ...
}
```

As you see, the start address of the VPD file in memory is set via PCDs: `PcdVpdBaseAddress` or `PcdVpdBaseAddress64`. These PCDs are declared in the `MdeModulePkg/MdeModulePkg.dec`:
```
[PcdsFixedAtBuild, PcdsPatchableInModule]
  ## VPD type PCD  allows a developer to point to an absolute physical address PcdVpdBaseAddress
  #  to store PCD value.
  # @Prompt VPD base address.
  gEfiMdeModulePkgTokenSpaceGuid.PcdVpdBaseAddress|0x0|UINT32|0x00010010

  ...

[PcdsDynamicEx]
    ## VPD type PCD allows a developer to point to an absolute physical address PcdVpdBaseAddress64
  #  to store PCD value. It will be DynamicExDefault only.
  #  It is used to set VPD region base address. So, it can't be DynamicExVpd PCD. Its value is
  #  required to be accessed in PcdDxe driver entry point. So, its value must be set in PEI phase.
  #  It can't depend on EFI variable service, and can't be DynamicExHii PCD.
  # @Prompt 64bit VPD base address.
  gEfiMdeModulePkgTokenSpaceGuid.PcdVpdBaseAddress64|0x0|UINT64|0x00030006
```

So if we want our VPD PCDs to work correctly we need to embed VPD file in the flash image and set any of these PCDs for its address.

# Embed VPD file to the flash image

Let's look how `OvmfPkg/OvmfPkgX64.fdf` file describes `OVMF.fd` image:
```
[FD.OVMF]
BaseAddress   = $(FW_BASE_ADDRESS)
Size          = $(FW_SIZE)
ErasePolarity = 1
BlockSize     = $(BLOCK_SIZE)
NumBlocks     = $(FW_BLOCKS)

!include VarStore.fdf.inc

$(VARS_SIZE)|$(FVMAIN_SIZE)
FV = FVMAIN_COMPACT

$(SECFV_OFFSET)|$(SECFV_SIZE)
FV = SECFV
```

You can look up the size/offset defines in the `OvmfPkg/OvmfPkgDefines.fdf.inc` file. Here are all 3 regions of the `OVMF.fd` image with their respective sizes:
```
VARS           region     (0x000000..0x084000)
FVMAIN_COMPACT region     (0x084000..0x3CC000)
SECFV          region     (0x3CC000..0x400000)
```

Let's substract 4KB (4096=0x1000) region from the `FVMAIN_COMPACT` part and dedicate it for the VPD file:
```
VARS           region     (0x000000..0x084000)
VPD            region     (0x084000..0x085000)
FVMAIN_COMPACT region     (0x085000..0x3CC000)
SECFV          region     (0x3CC000..0x400000)
```

For this make the following modifications to the `OvmfPkg/OvmfPkgDefines.fdf.inc` file:
```
!if $(FD_SIZE_IN_KB) == 4096
...
-DEFINE FVMAIN_SIZE       = 0x00348000
+DEFINE FVMAIN_SIZE       = 0x00347000
+DEFINE VPD_SIZE          = 0x1000
...
!endif
```

And modify `OvmfPkg/OvmfPkgX64.fdf` code like this:
```

[FD.OVMF]
BaseAddress   = $(FW_BASE_ADDRESS)
Size          = $(FW_SIZE)
ErasePolarity = 1
BlockSize     = $(BLOCK_SIZE)
NumBlocks     = $(FW_BLOCKS)

!include VarStore.fdf.inc

$(VARS_SIZE)|$(VPD_SIZE)
gEfiMdeModulePkgTokenSpaceGuid.PcdVpdBaseAddress
FILE = $(OUTPUT_DIRECTORY)/$(TARGET)_$(TOOL_CHAIN_TAG)/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08.bin

($(VARS_SIZE)+$(VPD_SIZE))|$(FVMAIN_SIZE)
FV = FVMAIN_COMPACT

$(SECFV_OFFSET)|$(SECFV_SIZE)
FV = SECFV
```

As you already know the build system will automatically set the `gEfiMdeModulePkgTokenSpaceGuid.PcdVpdBaseAddress` PCD for the PCD Database. With it the VPD PCDs should work correctly.

# `PCDLesson`

For the test let's again use our `PCDLesson` application.

It has the following code for the Dynamic/DynamicEx PCDs test:
```cpp
if (PcdToken(PcdDynamicInt32)) {
  Print(L"PcdDynamicInt32=0x%x\n", PcdGet32(PcdDynamicInt32));
  Status = PcdSet32S(PcdDynamicInt32, 0xBEEFBEEF);
  Print(L"Status=%r\n", Status);
  Print(L"PcdDynamicInt32=0x%x\n", PcdGet32(PcdDynamicInt32));
} else {
  Print(L"PcdDynamicInt32 token is unassigned\n");
}

Print(L"PcdDynamicExInt32=0x%x\n", PcdGetEx32(&gUefiLessonsPkgTokenSpaceGuid, PcdDynamicExInt32));

PcdSetEx32S(&gUefiLessonsPkgTokenSpaceGuid, PcdDynamicExInt32, 0x77777777);
Print(L"Status=%r\n", Status);
Print(L"PcdDynamicExInt32=0x%x\n", PcdGetEx32(&gUefiLessonsPkgTokenSpaceGuid, PcdDynamicExInt32));
```

As we've modified `OVMF.fd` FDF code, rebuild OVMF and launch QEMU with the updated `OVMF.fd` image:
```
$ qemu-system-x86_64 \
    -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
    -drive format=raw,file=fat:rw:~/UEFI_disk \
    -net none \
    -nographic
```

Check the `PCDLesson` output:
```
FS0:\> PCDLesson.efi
...
PcdDynamicInt32=0x44444444
Status=Invalid Parameter
PcdDynamicInt32=0x44444444
PcdDynamicExInt32=0x55555555
Status=Invalid Parameter
PcdDynamicExInt32=0x55555555
```

So as you can see the values from the VPD file are getted correctly. And read-only mechanics also works. It is not possible to set the VPD PCDs.

# Additional syntax for PCD statements

It is possible to use `*` in place of a `VPD offset`. This way the PCD would start just after the one before it. For example check this code:
```
[PcdsDynamicVpd]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32|7|0x44444444

[PcdsDynamicExVpd]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|*|0x55555555
```
Rebuild OVMF and use `hexdump` on the VPD file:
```
$ hexdump -C Build/OvmfX64/RELEASE_GCC5/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08.bin
00000000  00 00 00 00 00 00 00 44  44 44 44 55 55 55 55     |.......DDDDUUUU|
0000000f
```

Another possibility is to not override the PCD value in the DSC. This way the PCD would get its value from the DEC/INF file. For example:
```
[PcdsDynamicVpd]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32|7

[PcdsDynamicExVpd]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|*|0x55555555
```

Once again rebuild OVMF and use `hexdump` on the VPD file to verify the functionality:
```
$ hexdump -C Build/OvmfX64/RELEASE_GCC5/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08.bin
00000000  00 00 00 00 00 00 00 fe  ca fe ca 55 55 55 55     |...........UUUU|
0000000f
```

# `*.txt` and `*.map` files

Besides the VPD `*.bin` file the build system also creates `*.txt` and `*.map` files:
```
$ find Build/OvmfX64/RELEASE_GCC5/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08*
Build/OvmfX64/RELEASE_GCC5/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08.bin
Build/OvmfX64/RELEASE_GCC5/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08.map
Build/OvmfX64/RELEASE_GCC5/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08.txt
```

Here is the content of these files for the last example.

The `*.txt` contains the final values for the PCDs (notice `0xCAFECAFE` for the `PcdDynamicInt32`):
```
$ cat Build/OvmfX64/RELEASE_GCC5/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08.txt
...
#  This file lists all VPD informations for a platform collected by build.exe.
...
gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|DEFAULT|*|4|0x55555555
gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32|DEFAULT|7|4|0xCAFECAFE
```

And the `*.map` also contains calculated offset for all the PCDs:
```
$ cat Build/OvmfX64/RELEASE_GCC5/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08.map
...
#  This file lists all VPD informations for a platform fixed/adjusted by BPDG tool.
...
gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32 | DEFAULT | 7 | 4 | 0xCAFECAFE
gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32 | DEFAULT | 0xb | 4 | 0x55555555
```

# Links

[EDK II Build System Output File Format](https://edk2-docs.gitbook.io/edk-ii-dsc-specification/appendix_d_vpd_data_files/d1_edk_ii_build_system_output_file_format)

[GUIDed Tools and PCD VPD Data](https://edk2-docs.gitbook.io/edk-ii-build-specification/7_build_environment/73_guided_tools)

