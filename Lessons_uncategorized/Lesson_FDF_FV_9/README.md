We've investigated many methods how we can store data in flash. Now let's look at how we can access this data in our applications.

First we try to do it in a most simple way. The flash image is mapped to the processor memory. So let's just try to work with this memory region via pointers.

# Base address of the flash in memory

Final OVMF image has a size of 4MB:
```
$ du -sh Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd
4.0M    Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd
```
In case of `qemu-system-x86_64` it is mapped to the end of 32-bit address space. In this case it means that is mapped to the `0xFFC00000` address:
```
2^32 = 4Gb = 0x100000000
4MB = 4*1024*1024 = 0x400000

0x100000000 - 0x400000 = 0xFFC00000
```
If you look in the `OvmfPkg/OvmfPkgDefines.fdf.inc` file, you'll see:
```
!if $(FD_SIZE_IN_KB) == 4096
...
DEFINE FW_BASE_ADDRESS   = 0xFFC00000
...
!endif
```
This is the value that is used for the FD `BaseAddress` in the `OvmfPkg/OvmfPkgX64.fdf`:
```
[FD.OVMF]
BaseAddress   = $(FW_BASE_ADDRESS)
...
```

You can check `dmem` output at this address in UEFI shell:
```
Shell> dmem 0xFFC00000 0x100
Memory Address 00000000FFC00000 100 Bytes
  FFC00000: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  FFC00010: 8D 2B F1 FF 96 76 8B 4C-A9 85 27 47 07 5B 4F 50  *.+...v.L..'G.[OP*
  FFC00020: 00 40 08 00 00 00 00 00-5F 46 56 48 FF FE 04 00  *.@......_FVH....*
  FFC00030: 48 00 AF B8 00 00 00 02-84 00 00 00 00 10 00 00  *H...............*
  FFC00040: 00 00 00 00 00 00 00 00-78 2C F3 AA 7B 94 9A 43  *........x,..{..C*
  FFC00050: A1 80 2E 14 4E C3 77 92-B8 FF 03 00 5A FE 00 00  *....N.w.....Z...*
  FFC00060: 00 00 00 00 AA 55 3C 00-07 00 00 00 00 00 00 00  *.....U<.........*
  FFC00070: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  FFC00080: 00 00 00 00 00 00 00 00-08 00 00 00 04 00 00 00  *................*
  FFC00090: 11 40 70 EB 02 14 D3 11-8E 77 00 A0 C9 69 72 3B  *.@p......w...ir;*
  FFC000A0: 4D 00 54 00 43 00 00 00-01 00 00 00 AA 55 3C 00  *M.T.C........U<.*
  FFC000B0: 03 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  FFC000C0: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  FFC000D0: 28 00 00 00 01 00 00 00-16 D6 47 4B D6 A8 52 45  *(.........GK..RE*
  FFC000E0: 9D 44 CC AD 2E 0F 4C F9-49 00 6E 00 69 00 74 00  *.D....L.I.n.i.t.*
  FFC000F0: 69 00 61 00 6C 00 41 00-74 00 74 00 65 00 6D 00  *i.a.l.A.t.t.e.m.*
```

And verify that it is indeed `OVMF.fd` image:
```
$ hexdump -n 256 Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd -C
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  8d 2b f1 ff 96 76 8b 4c  a9 85 27 47 07 5b 4f 50  |.+...v.L..'G.[OP|
00000020  00 40 08 00 00 00 00 00  5f 46 56 48 ff fe 04 00  |.@......_FVH....|
00000030  48 00 af b8 00 00 00 02  84 00 00 00 00 10 00 00  |H...............|
00000040  00 00 00 00 00 00 00 00  78 2c f3 aa 7b 94 9a 43  |........x,..{..C|
00000050  a1 80 2e 14 4e c3 77 92  b8 ff 03 00 5a fe 00 00  |....N.w.....Z...|
00000060  00 00 00 00 aa 55 3c 00  07 00 00 00 00 00 00 00  |.....U<.........|
00000070  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000080  00 00 00 00 00 00 00 00  08 00 00 00 04 00 00 00  |................|
00000090  11 40 70 eb 02 14 d3 11  8e 77 00 a0 c9 69 72 3b  |.@p......w...ir;|
000000a0  4d 00 54 00 43 00 00 00  01 00 00 00 aa 55 3c 00  |M.T.C........U<.|
000000b0  03 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000000c0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000000d0  28 00 00 00 01 00 00 00  16 d6 47 4b d6 a8 52 45  |(.........GK..RE|
000000e0  9d 44 cc ad 2e 0f 4c f9  49 00 6e 00 69 00 74 00  |.D....L.I.n.i.t.|
000000f0  69 00 61 00 6c 00 41 00  74 00 74 00 65 00 6d 00  |i.a.l.A.t.t.e.m.|
00000100
```

# `OVMF` image structure

Let's investigate OVMF image structure. We've already know that is basically consists of `OVMF_VARS` and `OVMF_CODE` images concatenated together:
```
[FD.OVMF]
BaseAddress   = $(FW_BASE_ADDRESS)
Size          = $(FW_SIZE)
ErasePolarity = 1
BlockSize     = $(BLOCK_SIZE)
NumBlocks     = $(FW_BLOCKS)

!include VarStore.fdf.inc          #  = [FD.OVMF_VARS]

$(VARS_SIZE)|$(FVMAIN_SIZE)        #
FV = FVMAIN_COMPACT                #
                                   #  = [FD.OVMF_CODE]
$(SECFV_OFFSET)|$(SECFV_SIZE)      #
FV = SECFV                         #
```

`OVMF_CODE` part consists of two Firmware Volumes: `SECFV` and `FVMAIN_COMPACT`. `SECFV` FV is pretty simple and consists only of two modules:
```
[FV.SECFV]
...
#
# SEC Phase modules
#
# The code in this FV handles the initial firmware startup, and
# decompresses the PEI and DXE FVs which handles the rest of the boot sequence.
#
INF  OvmfPkg/Sec/SecMain.inf
INF  RuleOverride=RESET_VECTOR OvmfPkg/ResetVector/ResetVector.in
```
While the `FVMAIN_COMPACT` volume is a Firmware Volume file, that has one Lzma compressed section (`*_*_*_LZMA_GUID = EE4E5898-3914-4259-9D6E-DC7BD79403CF`), which has 2 Firmware Volume subsections - images for PEI and DXE stages:
```
[FV.FVMAIN_COMPACT]
...
FILE FV_IMAGE = 9E21FD93-9C72-4c15-8C4B-E77F1DB2D792 {
   SECTION GUIDED EE4E5898-3914-4259-9D6E-DC7BD79403CF PROCESSING_REQUIRED = TRUE {
     #
     # These firmware volumes will have files placed in them uncompressed,
     # and then both firmware volumes will be compressed in a single
     # compression operation in order to achieve better overall compression.
     #
     SECTION FV_IMAGE = PEIFV
     SECTION FV_IMAGE = DXEFV
   }
 }
```

Here is a picture of an image structure from the OVMF package [README.md](https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkgX64.fdf):
```
+--------------------------------------- base + 0x400000 (4GB/0x100000000)
| VTF0 (16-bit reset code) and OVMF SEC
| (SECFV, 208KB/0x34000)
+--------------------------------------- base + 0x3cc000
|
| Compressed main firmware image
| (FVMAIN_COMPACT, 3360KB/0x348000)
|
+--------------------------------------- base + 0x84000
| Fault-tolerant write (FTW)
| Spare blocks (264KB/0x42000)
+--------------------------------------- base + 0x42000
| FTW Work block (4KB/0x1000)
+--------------------------------------- base + 0x41000
| Event log area (4KB/0x1000)
+--------------------------------------- base + 0x40000
| Non-volatile variable storage
| area (256KB/0x40000)
+--------------------------------------- base address (0xffc00000)
```

In case you wonder how the OVMF firmware works with the Lzma compressed FV: the code in `SECFV` locates `FVMAIN_COMPACT` Firmware Volume, and decompresses its content into RAM memory. The addresses of `PEIFV` and `DXEFV` Firmware Volumes after the decompression are defined by the following PCDs:
```
[FD.MEMFD]
BaseAddress   = $(MEMFD_BASE_ADDRESS)		# =0x800000 (OvmfPkg/OvmfPkgDefines.fdf.inc)
...

0x020000|0x0E0000
gUefiOvmfPkgTokenSpaceGuid.PcdOvmfPeiMemFvBase|gUefiOvmfPkgTokenSpaceGuid.PcdOvmfPeiMemFvSize
FV = PEIFV

0x100000|0xC00000
gUefiOvmfPkgTokenSpaceGuid.PcdOvmfDxeMemFvBase|gUefiOvmfPkgTokenSpaceGuid.PcdOvmfDxeMemFvSize
FV = DXEFV
```

If you'll calculate PCD values, you'll get that `PEIFV` would be placed at addresses `0x820000..0x900000` and `DXEFV` would be placed at addresses `0x900000..0x1500000`.

To verify this, check FV headers with `hexdump`:
```
$ hexdump Build/OvmfX64/RELEASE_GCC5/FV/PEIFV.Fv -C | head
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  78 e5 8c 8c 3d 8a 1c 4f  99 35 89 61 85 c3 2d d3  |x...=..O.5.a..-.|
00000020  00 00 0e 00 00 00 00 00  5f 46 56 48 ff fe 07 00  |........_FVH....|
00000030  48 00 4f f6 60 00 00 02  0e 00 00 00 00 00 01 00  |H.O.`...........|
00000040  00 00 00 00 00 00 00 00  ff ff ff ff ff ff ff ff  |................|
00000050  ff ff ff ff ff ff ff ff  f4 aa f0 00 2c 00 00 f8  |............,...|
00000060  9b 07 38 69 03 b5 3d 4e  9d 24 b2 83 37 a2 58 06  |..8i..=N.$..7.X.|
00000070  14 00 00 00 ff ff ff ff  0a cc 45 1b 6a 15 8a 42  |..........E.j..B|
00000080  af 62 49 86 4d a0 e6 e6  b8 aa 02 00 2c 00 00 f8  |.bI.M.......,...|
00000090  14 00 00 19 4f da 3a 9b  56 ae 24 4c 8d ea f0 3b  |....O.:.V.$L...;|
$ hexdump Build/OvmfX64/RELEASE_GCC5/FV/DXEFV.Fv -C | head
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  78 e5 8c 8c 3d 8a 1c 4f  99 35 89 61 85 c3 2d d3  |x...=..O.5.a..-.|
00000020  00 00 c0 00 00 00 00 00  5f 46 56 48 ff fe 04 00  |........_FVH....|
00000030  48 00 ee f4 60 00 00 02  c0 00 00 00 00 00 01 00  |H...`...........|
00000040  00 00 00 00 00 00 00 00  ff ff ff ff ff ff ff ff  |................|
00000050  ff ff ff ff ff ff ff ff  f4 aa f0 00 2c 00 00 f8  |............,...|
00000060  c9 bd b8 7c eb f8 34 4f  aa ea 3e e4 af 65 16 a1  |...|..4O..>..e..|
00000070  14 00 00 00 ff ff ff ff  e7 0e 51 fc dc ff d4 11  |..........Q.....|
00000080  bd 41 00 80 c7 3c 88 81  16 aa 02 00 5c 00 00 f8  |.A...<......\...|
00000090  44 00 00 19 ce 0f 68 9b  6b ad 3a 4f b6 0b f5 98  |D.....h.k.:O....|
```
And dump OVMF memory with `dmem` from the UEFI shell:
```
Shell> dmem 820000 a0
Memory Address 0000000000820000 A0 Bytes
  00820000: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  00820010: 78 E5 8C 8C 3D 8A 1C 4F-99 35 89 61 85 C3 2D D3  *x...=..O.5.a..-.*
  00820020: 00 00 0E 00 00 00 00 00-5F 46 56 48 FF FE 07 00  *........_FVH....*
  00820030: 48 00 4F F6 60 00 00 02-0E 00 00 00 00 00 01 00  *H.O.`...........*
  00820040: 00 00 00 00 00 00 00 00-FF FF FF FF FF FF FF FF  *................*
  00820050: FF FF FF FF FF FF FF FF-F4 AA F0 00 2C 00 00 F8  *............,...*
  00820060: 9B 07 38 69 03 B5 3D 4E-9D 24 B2 83 37 A2 58 06  *..8i..=N.$..7.X.*
  00820070: 14 00 00 00 FF FF FF FF-0A CC 45 1B 6A 15 8A 42  *..........E.j..B*
  00820080: AF 62 49 86 4D A0 E6 E6-B8 AA 02 00 2C 00 00 F8  *.bI.M.......,...*
  00820090: 14 00 00 19 4F DA 3A 9B-56 AE 24 4C 8D EA F0 3B  *....O.:.V.$L...;*
Shell> Address 0000000000900000 A0 Bytes
  00900000: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  00900010: 78 E5 8C 8C 3D 8A 1C 4F-99 35 89 61 85 C3 2D D3  *x...=..O.5.a..-.*
  00900020: 00 00 C0 00 00 00 00 00-5F 46 56 48 FF FE 04 00  *........_FVH....*
  00900030: 48 00 EE F4 60 00 00 02-C0 00 00 00 00 00 01 00  *H...`...........*
  00900040: 00 00 00 00 00 00 00 00-FF FF FF FF FF FF FF FF  *................*
  00900050: FF FF FF FF FF FF FF FF-F4 AA F0 00 2C 00 00 F8  *............,...*
  00900060: C9 BD B8 7C EB F8 34 4F-AA EA 3E E4 AF 65 16 A1  *...|..4O..>..e..*
  00900070: 14 00 00 00 FF FF FF FF-E7 0E 51 FC DC FF D4 11  *..........Q.....*
  00900080: BD 41 00 80 C7 3C 88 81-16 AA 02 00 5C 00 00 F8  *.A...<......\...*
  00900090: 44 00 00 19 CE 0F 68 9B-6B AD 3A 4F B6 0B F5 98  *D.....h.k.:O....*
```

# Create a custom region in OVMF image

Now let's try to add a custom region to the OVMF flash image and manipuate it with our custom application.

The biggest part in the OVMF image is `FVMAIN_COMPACT` Firmware Volume. In case you forgot the overall structure of the `OVMF.fd` image is this:
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

Let's create a DATA region of size 0x1000 with a predefined array. We move the start of `FVMAIN_COMPACT` a 0x1000 further and put our region in this place:
```
[FD.OVMF]
BaseAddress   = $(FW_BASE_ADDRESS)
Size          = $(FW_SIZE)
ErasePolarity = 1
BlockSize     = $(BLOCK_SIZE)
NumBlocks     = $(FW_BLOCKS)

!include VarStore.fdf.inc

$(VARS_SIZE)|0x1000
gUefiOvmfPkgTokenSpaceGuid.PcdMyRegionBase
DATA = {
  0xDE, 0xAD, 0xBE, 0xEF
}

($(VARS_SIZE)+0x1000)|($(FVMAIN_SIZE)-0x1000)
FV = FVMAIN_COMPACT

$(SECFV_OFFSET)|$(SECFV_SIZE)
FV = SECFV
```
Here I've also added a PCD for the base of our region. It would be equal to `$(FW_BASE_ADDRESS)+$(VARS_SIZE)`. I didn't define a PCD for the region size, as we wouldn't need it. Also you can see that it is possible to use mathematical expressions in region parameters definition.

Don't forget to add this new PCD to the `OvmfPkg/OvmfPkg.dec` file:
```
[PcdsFixedAtBuild]
...
gUefiOvmfPkgTokenSpaceGuid.PcdMyRegionBase|0x55|UINT32|0xa5a5a5a5
```
Here I've used a random token `0xa5a5a5a5` and `0x55` as a default value for the PCD.

# Create application to manipulate custom region data

Now let's construct our application. It would try to read and write a value at the `PcdMyRegionBase` address.

`UefiLessonsPkg/FlashAccessRaw/FlashAccessRaw.c`:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  volatile UINT32* Val = (UINT32*)(FixedPcdGet32(PcdMyRegionBase));
  Print(L"Val = 0x%08x\n", *Val);
  *Val = 0xCAFECAFE;
  Print(L"Val = 0x%08x\n", *Val);
  return EFI_SUCCESS;
}
```

`UefiLessonsPkg/FlashAccessRaw/FlashAccessRaw.inf`:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = FlashAccessRaw
  FILE_GUID                      = 475028f8-4219-4615-9a24-c1ccc66f8fee
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = UefiMain

[Sources]
  FlashAccessRaw.c

[Packages]
  MdePkg/MdePkg.dec
  OvmfPkg/OvmfPkg.dec                           # need to include this to get access to the PCD

[LibraryClasses]
  UefiApplicationEntryPoint
  UefiLib

[Pcd]
  gUefiOvmfPkgTokenSpaceGuid.PcdMyRegionBase   # necessary PCD
```

The important thing is that you should't build this app as a part of `UefiLessonsPkg/UefiLessonsPkg.dsc` via standard `build` command like we've used to:
```
[Components]
...
UefiLessonsPkg/FlashAccessRaw/FlashAccessRaw.inf
```
If you do it, the `gUefiOvmfPkgTokenSpaceGuid.PcdMyRegionBase` wouldn't get its value from the FDF file. You can verify this if you look at the created AutoGen file (`Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/FlashAccessRaw/FlashAccessRaw/DEBUG/AutoGen.h`). The PCD value in this case is getting assigned to its default value:
```
#define _PCD_VALUE_PcdMyRegionBase  0x55U
```
This is why the `FlashAccessRaw` application should be compiled as a part of `OvmfPkg/OvmfPkgX64.dsc`:
```
[Components]
  ...
  UefiLessonsPkg/FlashAccessRaw/FlashAccessRaw.inf
```
via OVMF build command:
```
build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```
In this case PCD would get correct value `Build/OvmfX64/RELEASE_GCC5/X64/UefiLessonsPkg/FlashAccessRaw/FlashAccessRaw/DEBUG/AutoGen.h`:
```
#define _PCD_VALUE_PcdMyRegionBase  0xFFC84000U
```

Now copy correct version to the shared folder:
```
cp Build/OvmfX64/RELEASE_GCC5/X64/FlashAccessRaw.efi ~/UEFI_disk/
```
And check it's output:
```
FS0:\> FlashAccessRaw.efi
Val = 0xEFBEADDE
Val = 0xEFBEADDE
```

You can see that we've correctly read our value from the flash. The `0xEFBEADDE` is just the `0xDEADBEEF` backwards. This is just how little-endian architecture interprets UINT32 numbers in memory.

The second important observation from the output it that the memory-mapped flash region is read-only. It is not possible to rewrite it via pointers.

