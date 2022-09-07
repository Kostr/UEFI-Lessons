In this lesson we are going to look how non-volatile variables are stored in the flash image.

Let's remember the `[FD.OVMF]` image structure from the [https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkgX64.fdf](https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkgX64.fdf):
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

The variable storage is declared inside the `VarStore.fdf.inc` file, so let's look into it [https://github.com/tianocore/edk2/blob/master/OvmfPkg/VarStore.fdf.inc](https://github.com/tianocore/edk2/blob/master/OvmfPkg/VarStore.fdf.inc).

As in our case the flash size is 4MB, the define `FD_SIZE_IN_KB` is equal to 4096. Therefore the file content can be simplified to:
```
0x00000000|0x00040000

#NV_VARIABLE_STORE
DATA = {
  ## This is the EFI_FIRMWARE_VOLUME_HEADER                             <----- EFI_FIRMWARE_VOLUME_HEADER
  # ZeroVector []
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  # FileSystemGuid: gEfiSystemNvDataFvGuid         =
  #   { 0xFFF12B8D, 0x7696, 0x4C8B,
  #     { 0xA9, 0x85, 0x27, 0x47, 0x07, 0x5B, 0x4F, 0x50 }}
  0x8D, 0x2B, 0xF1, 0xFF, 0x96, 0x76, 0x8B, 0x4C,
  0xA9, 0x85, 0x27, 0x47, 0x07, 0x5B, 0x4F, 0x50,
  # FvLength: 0x84000							<----- All Content: EFI_FIRMWARE_VOLUME_HEADER + NV_VARIABLE_STORE + NV_EVENT_LOG + NV_FTW_WORKING + NV_FTW_SPARE
  0x00, 0x40, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
  # Signature "_FVH"       # Attributes
  0x5f, 0x46, 0x56, 0x48, 0xff, 0xfe, 0x04, 0x00,
  # HeaderLength
  0x48, 0x00,
  # CheckSum
  0xAF, 0xB8,
  # ExtHeaderOffset #Reserved #Revision
  0x00, 0x00, 0x00, 0x02,
  # Blockmap[0]: 0x84 Blocks * 0x1000 Bytes / Block			<------ 0x84 Blocks * 0x1000 Bytes / Block
  0x84, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
  # Blockmap[1]: End
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  ## This is the VARIABLE_STORE_HEADER					<------ VARIABLE_STORE_HEADER
  # It is compatible with SECURE_BOOT_ENABLE == FALSE as well.
  # Signature: gEfiAuthenticatedVariableGuid =
  #   { 0xaaf32c78, 0x947b, 0x439a,
  #     { 0xa1, 0x80, 0x2e, 0x14, 0x4e, 0xc3, 0x77, 0x92 }}
  0x78, 0x2c, 0xf3, 0xaa, 0x7b, 0x94, 0x9a, 0x43,
  0xa1, 0x80, 0x2e, 0x14, 0x4e, 0xc3, 0x77, 0x92,
  # Size: 0x40000 (gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableSize) -
  #          0x48 (size of EFI_FIRMWARE_VOLUME_HEADER) = 0x3ffb8
  # This can speed up the Variable Dispatch a bit.
  0xB8, 0xFF, 0x03, 0x00,
  # FORMATTED: 0x5A #HEALTHY: 0xFE #Reserved: UINT16 #Reserved1: UINT32
  0x5A, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
}

0x00040000|0x00001000
#NV_EVENT_LOG

0x00041000|0x00001000
#NV_FTW_WORKING
DATA = {
  # EFI_FAULT_TOLERANT_WORKING_BLOCK_HEADER->Signature = gEdkiiWorkingBlockSignatureGuid         =
  #  { 0x9e58292b, 0x7c68, 0x497d, { 0xa0, 0xce, 0x65,  0x0, 0xfd, 0x9f, 0x1b, 0x95 }}
  0x2b, 0x29, 0x58, 0x9e, 0x68, 0x7c, 0x7d, 0x49,
  0xa0, 0xce, 0x65,  0x0, 0xfd, 0x9f, 0x1b, 0x95,
  # Crc:UINT32            #WorkingBlockValid:1, WorkingBlockInvalid:1, Reserved
  0x2c, 0xaf, 0x2c, 0x64, 0xFE, 0xFF, 0xFF, 0xFF,
  # WriteQueueSize: UINT64
  0xE0, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
}

0x00042000|0x00042000
#NV_FTW_SPARE
```

On a high level this file declares 4 regions:
```
0x00000000..0x00040000 # NV_VARIABLE_STORE
0x00040000..0x00041000 # NV_EVENT_LOG
0x00041000..0x00042000 # NV_FTW_WORKING
0x00042000..0x00084000 # NV_FTW_SPARE
```

In this lesson we are interested in the first region `NV_VARIABLE_STORE` as this region is used as a variable storage in our image.

But as you can see the `NV_VARIABLE_STORE` region data actually starts not from the variable storage itself. At the beggining there is a Firmware Volume header (`EFI_FIRMWARE_VOLUME_HEADER`) for all 4 regions which is filled manually via the hardcoded `DATA` array.

In case you don't remember here is a description for the `EFI_FIRMWARE_VOLUME_HEADER` [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Pi/PiFirmwareVolume.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Pi/PiFirmwareVolume.h):
```
typedef struct {
  UINT8                     ZeroVector[16];
  EFI_GUID                  FileSystemGuid;	// Declares the file system with which the firmware volume is formatted
  UINT64                    FvLength;		// Length in bytes of the complete firmware volume, including the header
  UINT32                    Signature;		// [#define EFI_FVH_SIGNATURE  SIGNATURE_32 ('_', 'F', 'V', 'H')]
  EFI_FVB_ATTRIBUTES_2      Attributes;		// Declares capabilities and power-on defaults for the firmware volume
  UINT16                    HeaderLength;	// Length in bytes of the complete firmware volume header
  UINT16                    Checksum;		// A 16-bit checksum of the firmware volume header. A valid header sums to zero
  UINT16                    ExtHeaderOffset;	// Offset to the extended header (EFI_FIRMWARE_VOLUME_EXT_HEADER) or zero if there is no extended header
  UINT8                     Reserved[1];	// This field must always be set to zero
  UINT8                     Revision;		// [#define EFI_FVH_REVISION  0x02]
  EFI_FV_BLOCK_MAP_ENTRY    BlockMap[1];	// An array of run-length encoded FvBlockMapEntry structures. The array is terminated with an entry of {0,0}
} EFI_FIRMWARE_VOLUME_HEADER;
```

We can even check this header with the `VolInfo` utility. As you know the `OVMF_VARS.fd` image is just a `VarStore.fdf.inc` that we are currently discussing:
```
[FD.OVMF_VARS]
BaseAddress   = $(FW_BASE_ADDRESS)
Size          = $(VARS_SIZE)
ErasePolarity = 1
BlockSize     = $(BLOCK_SIZE)
NumBlocks     = $(VARS_BLOCKS)

!include VarStore.fdf.inc
```

So let's use the `VolInfo` on this image.

First rebuild `OVMF` image to clear all the current NV content:
```
$ build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

Now use `VolInfo` on the `OVMF_VARS.fd` file:
```
$ VolInfo Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd

VolInfo Version 1.0 Build Developer Build based on Revision: Unknown
Signature:        _FVH (4856465F)
Attributes:       4FEFF
       EFI_FVB2_READ_DISABLED_CAP
       EFI_FVB2_READ_ENABLED_CAP
       EFI_FVB2_READ_STATUS
       EFI_FVB2_WRITE_DISABLED_CAP
       EFI_FVB2_WRITE_ENABLED_CAP
       EFI_FVB2_WRITE_STATUS
       EFI_FVB2_LOCK_CAP
       EFI_FVB2_LOCK_STATUS
       EFI_FVB2_STICKY_WRITE
       EFI_FVB2_MEMORY_MAPPED
       EFI_FVB2_ERASE_POLARITY
       EFI_FVB2_READ_LOCK_CAP
       EFI_FVB2_READ_LOCK_STATUS
       EFI_FVB2_WRITE_LOCK_CAP
       EFI_FVB2_WRITE_LOCK_STATUS
       EFI_FVB2_ALIGNMENT_16
Header Length:         0x00000048
File System ID:        fff12b8d-7696-4c8b-a985-2747075b4f50
Revision:              0x0002
Number of Blocks:      0x00000084
Block Length:          0x00001000
Total Volume Size:     0x00084000
VolInfo: ERROR 0003: error parsing FV image
  cannot find the first file in the FV image
```

Here you can see how `VolInfo` was able to parse the Firmware Volume header. `VolInfo` wasn't able to parse the FV content, as it is not the usual FFS files, but a custom data.

So let's invetigate this data ourself. First there is the `VARIABLE_STORE_HEADER` [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Guid/VariableFormat.h](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Guid/VariableFormat.h):
```cpp
typedef struct {
  EFI_GUID    Signature;	// Variable store region signature
  UINT32      Size;		// Size of entire variable store, including size of variable store header but not including the size of FvHeader
  UINT8       Format;		// Variable region format state
  UINT8       State;		// Variable region healthy state
  UINT16      Reserved;
  UINT32      Reserved1;
} VARIABLE_STORE_HEADER;
```

It uses `gEfiAuthenticatedVariableGuid` GUID for the `Signature` field which is declared in the same file:
```cpp
#define EFI_AUTHENTICATED_VARIABLE_GUID \
  { 0xaaf32c78, 0x947b, 0x439a, { 0xa1, 0x80, 0x2e, 0x14, 0x4e, 0xc3, 0x77, 0x92 } }
extern EFI_GUID  gEfiAuthenticatedVariableGuid;
```

Right after the variable storage header there is an array of actual variable records. In our case each record actual variable data is prepended with a `AUTHENTICATED_VARIABLE_HEADER`:
```cpp
///
/// Single Authenticated Variable Data Header Structure.
///
typedef struct {
  UINT16      StartId;			// Variable Data Start Flag
  UINT8       State;			// Variable State defined above
  UINT8       Reserved;
  UINT32      Attributes;		// Attributes of variable defined in UEFI specification
  UINT64      MonotonicCount;		// Associated monotonic count value against replay attack
  EFI_TIME    TimeStamp;		// Associated TimeStamp value against replay attack
  UINT32      PubKeyIndex;		// Index of associated public key in database
  UINT32      NameSize;			// Size of variable null-terminated Unicode string name
  UINT32      DataSize;			// Size of the variable data without this header
  EFI_GUID    VendorGuid;		// A unique identifier for the vendor that produces and consumes this varaible
} AUTHENTICATED_VARIABLE_HEADER;
```

The `StartId` field in this structure contains a marker for the header:
```cpp
///
/// Variable data start flag.
///
#define VARIABLE_DATA  0x55AA
```

So it is easy to see when one variable ends and another one starts. We will need it later.

But if you look at the `OVMF_VARS.fd` right now you wouldn't see any variables:
```
$ hexdump -C Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  8d 2b f1 ff 96 76 8b 4c  a9 85 27 47 07 5b 4f 50  |.+...v.L..'G.[OP|
00000020  00 40 08 00 00 00 00 00  5f 46 56 48 ff fe 04 00  |.@......_FVH....|
00000030  48 00 af b8 00 00 00 02  84 00 00 00 00 10 00 00  |H...............|
00000040  00 00 00 00 00 00 00 00  78 2c f3 aa 7b 94 9a 43  |........x,..{..C|
00000050  a1 80 2e 14 4e c3 77 92  b8 ff 03 00 5a fe 00 00  |....N.w.....Z...|
00000060  00 00 00 00 ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
00000070  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00041000  2b 29 58 9e 68 7c 7d 49  a0 ce 65 00 fd 9f 1b 95  |+)X.h|}I..e.....|
00041010  2c af 2c 64 fe ff ff ff  e0 0f 00 00 00 00 00 00  |,.,d............|
00041020  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00084000
```

The same goes to the `OVMF.fd` image if you have any doubts:
```
$ hexdump -C Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd | head -n 12
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  8d 2b f1 ff 96 76 8b 4c  a9 85 27 47 07 5b 4f 50  |.+...v.L..'G.[OP|
00000020  00 40 08 00 00 00 00 00  5f 46 56 48 ff fe 04 00  |.@......_FVH....|
00000030  48 00 af b8 00 00 00 02  84 00 00 00 00 10 00 00  |H...............|
00000040  00 00 00 00 00 00 00 00  78 2c f3 aa 7b 94 9a 43  |........x,..{..C|
00000050  a1 80 2e 14 4e c3 77 92  b8 ff 03 00 5a fe 00 00  |....N.w.....Z...|
00000060  00 00 00 00 ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
00000070  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00041000  2b 29 58 9e 68 7c 7d 49  a0 ce 65 00 fd 9f 1b 95  |+)X.h|}I..e.....|
00041010  2c af 2c 64 fe ff ff ff  e0 0f 00 00 00 00 00 00  |,.,d............|
00041020  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00084000  ...
```

This is because there is no NVRAM variables right after the re-build.

Now let's run QEMU for the first time with `OVMF.fd` image:
```
$ qemu-system-x86_64  \
  -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk \
  -net none \
  -nographic
```

You can verify that there are a lot of UEFI Variables in the system via the `dmpstore` command:
```
Shell> dmpstore -all
...
```

All these variables were actually created on the OVMF execution.

And you can actually see the NV storage with `dmem`, as the flash chip is mapped to the `0xffc00000` address (=(4GB-4MB)). See the `DEFINE FW_BASE_ADDRESS = 0xFFC00000` in the [https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkgDefines.fdf.inc](https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkgDefines.fdf.inc):
```
Shell> dmem 0xffc00000 150
Memory Address 00000000FFC00000 150 Bytes
  FFC00000: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  FFC00010: 8D 2B F1 FF 96 76 8B 4C-A9 85 27 47 07 5B 4F 50  *.+...v.L..'G.[OP*
  FFC00020: 00 40 08 00 00 00 00 00-5F 46 56 48 FF FE 04 00  *.@......_FVH....*     <---- _FVH  (EFI_FVH_SIGNATURE)
  FFC00030: 48 00 AF B8 00 00 00 02-84 00 00 00 00 10 00 00  *H...............*
  FFC00040: 00 00 00 00 00 00 00 00-78 2C F3 AA 7B 94 9A 43  *........x,..{..C*
  FFC00050: A1 80 2E 14 4E C3 77 92-B8 FF 03 00 5A FE 00 00  *....N.w.....Z...*
  FFC00060: 00 00 00 00 AA 55 3F 00-07 00 00 00 00 00 00 00  *.....U?.........*
  FFC00070: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  FFC00080: 00 00 00 00 00 00 00 00-08 00 00 00 04 00 00 00  *................*
  FFC00090: 11 40 70 EB 02 14 D3 11-8E 77 00 A0 C9 69 72 3B  *.@p......w...ir;*
  FFC000A0: 4D 00 54 00 43 00 00 00-01 00 00 00 AA 55 3C 00  *M.T.C........U<.*
  FFC000B0: 03 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  FFC000C0: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  FFC000D0: 28 00 00 00 01 00 00 00-16 D6 47 4B D6 A8 52 45  *(.........GK..RE*
  FFC000E0: 9D 44 CC AD 2E 0F 4C F9-49 00 6E 00 69 00 74 00  *.D....L.I.n.i.t.*
  FFC000F0: 69 00 61 00 6C 00 41 00-74 00 74 00 65 00 6D 00  *i.a.l.A.t.t.e.m.*
  FFC00100: 70 00 74 00 4F 00 72 00-64 00 65 00 72 00 00 00  *p.t.O.r.d.e.r...*
  FFC00110: 01 FF FF FF AA 55 3F 00-03 00 00 00 00 00 00 00  *.....U?.........*
  FFC00120: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  FFC00130: 00 00 00 00 00 00 00 00-14 00 00 00 19 04 00 00  *................*
  FFC00140: 45 49 32 59 44 EC 0D 4C-B1 CD 9D B1 39 DF 07 0C  *EI2YD..L....9...*
```

Now finish QEMU and look at the `OVMF.fd` content:
```
$ hexdump -C Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd | head -n 30
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  8d 2b f1 ff 96 76 8b 4c  a9 85 27 47 07 5b 4f 50  |.+...v.L..'G.[OP|
00000020  00 40 08 00 00 00 00 00  5f 46 56 48 ff fe 04 00  |.@......_FVH....|
00000030  48 00 af b8 00 00 00 02  84 00 00 00 00 10 00 00  |H...............|
00000040  00 00 00 00 00 00 00 00  78 2c f3 aa 7b 94 9a 43  |........x,..{..C|
00000050  a1 80 2e 14 4e c3 77 92  b8 ff 03 00 5a fe 00 00  |....N.w.....Z...|
00000060  00 00 00 00 aa 55 3f 00  07 00 00 00 00 00 00 00  |.....U?.........|
00000070  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000080  00 00 00 00 00 00 00 00  08 00 00 00 04 00 00 00  |................|
00000090  11 40 70 eb 02 14 d3 11  8e 77 00 a0 c9 69 72 3b  |.@p......w...ir;|
000000a0  4d 00 54 00 43 00 00 00  01 00 00 00 aa 55 3c 00  |M.T.C........U<.|
000000b0  03 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000000c0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000000d0  28 00 00 00 01 00 00 00  16 d6 47 4b d6 a8 52 45  |(.........GK..RE|
000000e0  9d 44 cc ad 2e 0f 4c f9  49 00 6e 00 69 00 74 00  |.D....L.I.n.i.t.|
000000f0  69 00 61 00 6c 00 41 00  74 00 74 00 65 00 6d 00  |i.a.l.A.t.t.e.m.|
00000100  70 00 74 00 4f 00 72 00  64 00 65 00 72 00 00 00  |p.t.O.r.d.e.r...|
00000110  01 ff ff ff aa 55 3f 00  03 00 00 00 00 00 00 00  |.....U?.........|
00000120  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000130  00 00 00 00 00 00 00 00  14 00 00 00 19 04 00 00  |................|
00000140  45 49 32 59 44 ec 0d 4c  b1 cd 9d b1 39 df 07 0c  |EI2YD..L....9...|
00000150  41 00 74 00 74 00 65 00  6d 00 70 00 74 00 20 00  |A.t.t.e.m.p.t. .|
00000160  31 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |1...............|
00000170  00 00 00 00 00 01 00 00  00 00 00 00 00 00 41 74  |..............At|
00000180  74 65 6d 70 74 20 31 00  00 00 00 00 00 00 00 00  |tempt 1.........|
00000190  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000210  00 00 00 00 00 00 00 00  00 00 bc 0c 00 00 00 00  |................|
00000220  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
```

You can see that is has changed. Now the variable storage contains NV variables.

`UEFITool` utility is able to parse NV storages [https://github.com/LongSoft/UEFITool](https://github.com/LongSoft/UEFITool). Load the `OVMF.fd` file to the `UEFITool` program. Here you can see that firmware indeed starts from the Firmware Volume with the `gEfiSystemNvDataFvGuid`:
![1](1.png?raw=true "1")

If you unravel the variable storage, you could see that the storage is filled with `Invalid` entries:
![2](2.png?raw=true "2")

What is that?

Let's re-launch QEMU for the second time. This would update `OVMF.fd` image. If you load the updated image to the tool you could see that now `EfiMtcGuid` that was in the first variable record now changed to `Invalid`:
![3](3.png?raw=true "3")

Although you can find it in the end of the storage:
![4](4.png?raw=true "4")

So what is happening here? If you execute `hexdump` on the start of the `OVMF.fd` image again there wouldn't be any much difference. It looks like the record for `MTC` variable is present:
```
$ hexdump -C Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd | head -n 30
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
000000a0  4d 00 54 00 43 00 00 00  01 00 00 00 aa 55 3c 00  |M.T.C........U<.|           <----- MTC
000000b0  03 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000000c0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000000d0  28 00 00 00 01 00 00 00  16 d6 47 4b d6 a8 52 45  |(.........GK..RE|
000000e0  9d 44 cc ad 2e 0f 4c f9  49 00 6e 00 69 00 74 00  |.D....L.I.n.i.t.|
000000f0  69 00 61 00 6c 00 41 00  74 00 74 00 65 00 6d 00  |i.a.l.A.t.t.e.m.|
00000100  70 00 74 00 4f 00 72 00  64 00 65 00 72 00 00 00  |p.t.O.r.d.e.r...|
00000110  01 ff ff ff aa 55 3f 00  03 00 00 00 00 00 00 00  |.....U?.........|
00000120  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000130  00 00 00 00 00 00 00 00  14 00 00 00 19 04 00 00  |................|
00000140  45 49 32 59 44 ec 0d 4c  b1 cd 9d b1 39 df 07 0c  |EI2YD..L....9...|
00000150  41 00 74 00 74 00 65 00  6d 00 70 00 74 00 20 00  |A.t.t.e.m.p.t. .|
00000160  31 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |1...............|
00000170  00 00 00 00 00 01 00 00  00 00 00 00 00 00 41 74  |..............At|
00000180  74 65 6d 70 74 20 31 00  00 00 00 00 00 00 00 00  |tempt 1.........|
00000190  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000210  00 00 00 00 00 00 00 00  00 00 bc 0c 00 00 00 00  |................|
00000220  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
```

Although one bit was changed. Let's see it. As we already know, the variable records start with the `0x55AA` (`VARIABLE_DATA`) marker. So let's cut data for this first record in the NV storage, the record which once was `MTC` variable.

Before (working):
```
00000060              aa 55 3f 00  07 00 00 00 00 00 00 00  |.....U?.........|
00000070  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000080  00 00 00 00 00 00 00 00  08 00 00 00 04 00 00 00  |................|
00000090  11 40 70 eb 02 14 d3 11  8e 77 00 a0 c9 69 72 3b  |.@p......w...ir;|
000000a0  4d 00 54 00 43 00 00 00  01 00 00 00              |M.T.C.......    |
```
After (invalid):
```
00000060              aa 55 3c 00  07 00 00 00 00 00 00 00  |.....U<.........|
00000070  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000080  00 00 00 00 00 00 00 00  08 00 00 00 04 00 00 00  |................|
00000090  11 40 70 eb 02 14 d3 11  8e 77 00 a0 c9 69 72 3b  |.@p......w...ir;|
000000a0  4d 00 54 00 43 00 00 00  01 00 00 00              |M.T.C........U<.|
```

The only difference is a change for the `AUTHENTICATED_VARIABLE_HEADER.State` field from `0x3f` to `0x3c`.

In the [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Guid/VariableFormat.h](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Guid/VariableFormat.h) file there are some descriptions for the State field flags:
```cpp
//
/// Variable State flags.
///
#define VAR_IN_DELETED_TRANSITION  0xfe     ///< Variable is in obsolete transition.
#define VAR_DELETED                0xfd     ///< Variable is obsolete.
#define VAR_HEADER_VALID_ONLY      0x7f     ///< Variable header has been valid.
#define VAR_ADDED                  0x3f     ///< Variable has been completely added.
```

With this terminology the `0x3f` value is (`VAR_ADDED`) and the `0x3c` value is (`VAR_ADDED & VAR_IN_DELETED_TRANSITION & VAR_DELETED)`. So our `MTC` variable was deleted and then created in another record at the end of the table.

Why?

In the real scenario the NV variables are stored in a physical flash chip. And this puts some limitations for the NV variable driver. For example it is not possible to erase random bytes of data, as you are only able to erase data in blocks for a flash chip (and a block can be pretty big, for example 1024 bytes). So to change one variable record you would have to delete and recreate a bunch of data.

Also there is a wear-leveling issue. Flash chips have a limited count of block erase operations. Around that time (or even earlier) the block erase wouldn't be completed successfully, so you can't use this block for your data no more. So if some variable changes often, modifying it in-place would wear-level flash chip pretty quickly.

Important conclusion is that it is better to append new records to the end of a storage, than change current records. And this is what we've observed.

Also after the block erase operation in a flash chip all bytes in that block are set to 0xFF (all 1's). The write operation is able to set bits from 1's to 0's (and not in the opposite way!). This is why it is possible to change `0x3f` value (`VAR_ADDED`) to `0x3c` value (`VAR_ADDED & VAR_IN_DELETED_TRANSITION & VAR_DELETED)` without any block erase operation.

# `SetVariableExample` test

For another test let's use our `SetVariableExample` application that we've created earlier.

As a control method let's use `UEFIExtract` utility from the same UEFITool repo [https://github.com/LongSoft/UEFITool](https://github.com/LongSoft/UEFITool). The `UEFIExtract` is a command-line utility, therefore it is easier to use it from CLI:
```
$ ./UEFIExtract
UEFIExtract NE alpha 60 (Aug 27 2022)

Usage: UEFIExtract imagefile        - generate report and dump only leaf tree items into .dump folder.
       UEFIExtract imagefile all    - generate report and dump all tree items.
       UEFIExtract imagefile unpack - generate report and dump all tree items in one dir.
       UEFIExtract imagefile dump   - only generate dump, no report needed.
       UEFIExtract imagefile report - only generate report, no dump needed.
       UEFIExtract imagefile GUID_1 ... [ -o FILE_1 ... ] [ -m MODE_1 ... ] [ -t TYPE_1 ... ] -
         Dump only FFS file(s) with specific GUID(s), without report.
         Type is section type or FF to ignore. Mode is one of: all, body, header, info, file.
Return value is a bit mask where 0 at position N means that file with GUID_N was found and unpacked, 1 otherwise.
```

In our test we would be checking `report` command output. Copy current `OVMF.fd` image, generate report for it and check the result. In the output below I've included only the NV storage part of the output:
```
$ cp <...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd ./
$ ./UEFIExtract OVMF.fd report
$ cat OVMF.fd.report.txt
      Type       |        Subtype        |   Base   |   Size   |  CRC32   |   Name
 Image           | UEFI                  | 00000000 | 00400000 | 90EAAFFF |  UEFI image
 Volume          | NVRAM                 | 00000000 | 00084000 | 393BD56A | - FFF12B8D-7696-4C8B-A985-2747075B4F50
 VSS2 store      |                       | 00000048 | 0003FFB8 | E9A424DE | -- VSS2 store
 VSS entry       | Invalid               | 00000064 | 00000048 | 0B1B3B2A | --- Invalid
 VSS entry       | Invalid               | 000000AC | 00000065 | 43FDF85A | --- Invalid
 VSS entry       | Auth                  | 00000114 | 00000469 | 0E8A7CD5 | --- 59324945-EC44-4C0D-B1CD-9DB139DF070C | Attempt 1
 VSS entry       | Invalid               | 00000580 | 00000066 | B6ACED62 | --- Invalid
 VSS entry       | Auth                  | 000005E8 | 00000469 | 002D804B | --- 59324945-EC44-4C0D-B1CD-9DB139DF070C | Attempt 2
 VSS entry       | Invalid               | 00000A54 | 00000067 | B8B27872 | --- Invalid
 VSS entry       | Auth                  | 00000ABC | 00000469 | B36029FE | --- 59324945-EC44-4C0D-B1CD-9DB139DF070C | Attempt 3
 VSS entry       | Invalid               | 00000F28 | 00000068 | 70B54595 | --- Invalid
 VSS entry       | Auth                  | 00000F90 | 00000469 | 1D627977 | --- 59324945-EC44-4C0D-B1CD-9DB139DF070C | Attempt 4
 VSS entry       | Invalid               | 000013FC | 00000069 | A2979B6A | --- Invalid
 VSS entry       | Auth                  | 00001468 | 00000469 | AE2FD0C2 | --- 59324945-EC44-4C0D-B1CD-9DB139DF070C | Attempt 5
 VSS entry       | Invalid               | 000018D4 | 0000006A | 4C9EA244 | --- Invalid
 VSS entry       | Auth                  | 00001940 | 00000469 | A0882C5C | --- 59324945-EC44-4C0D-B1CD-9DB139DF070C | Attempt 6
 VSS entry       | Invalid               | 00001DAC | 0000006B | B0DA855D | --- Invalid
 VSS entry       | Auth                  | 00001E18 | 00000469 | 13C585E9 | --- 59324945-EC44-4C0D-B1CD-9DB139DF070C | Attempt 7
 VSS entry       | Auth                  | 00002284 | 0000006C | 27A057BF | --- 4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9 | InitialAttemptOrder
 VSS entry       | Auth                  | 000022F0 | 00000469 | 27FD8B0F | --- 59324945-EC44-4C0D-B1CD-9DB139DF070C | Attempt 8
 VSS entry       | Invalid               | 0000275C | 00000052 | D508D27E | --- Invalid
 VSS entry       | Auth                  | 000027B0 | 0000008C | 3D8F8004 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | Boot0000
 VSS entry       | Auth                  | 0000283C | 0000004E | C451514A | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | Timeout
 VSS entry       | Auth                  | 0000288C | 00000059 | A16D4B75 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | PlatformLang
 VSS entry       | Auth                  | 000028E8 | 0000004A | A8279A0B | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | Lang
 VSS entry       | Auth                  | 00002934 | 00000057 | 66192BDE | --- 04B37FE8-F6AE-480B-BDD5-37D98C5E89AA | VarErrorFlag
 VSS entry       | Invalid               | 0000298C | 0000006A | BEDF0159 | --- Invalid
 VSS entry       | Invalid               | 000029F8 | 00000093 | 133B4E89 | --- Invalid
 VSS entry       | Invalid               | 00002A8C | 000000B3 | 2770C13D | --- Invalid
 VSS entry       | Invalid               | 00002B40 | 00000093 | C5BEF246 | --- Invalid
 VSS entry       | Invalid               | 00002BD4 | 000000DC | 94308B4F | --- Invalid
 VSS entry       | Invalid               | 00002CB0 | 000000FC | 62B4896C | --- Invalid
 VSS entry       | Invalid               | 00002DAC | 000000DC | A2F145DC | --- Invalid
 VSS entry       | Invalid               | 00002E88 | 000000FA | 87389AFC | --- Invalid
 VSS entry       | Invalid               | 00002F84 | 0000013B | 3039AE62 | --- Invalid
 VSS entry       | Invalid               | 000030C0 | 00000139 | 76E1A61F | --- Invalid
 VSS entry       | Invalid               | 000031FC | 0000011B | 82C266DE | --- Invalid
 VSS entry       | Invalid               | 00003318 | 0000014A | FDC1C84F | --- Invalid
 VSS entry       | Invalid               | 00003464 | 00000159 | 5B9AE227 | --- Invalid
 VSS entry       | Auth                  | 000035C0 | 0000005A | 7DB8CD95 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | Key0000
 VSS entry       | Auth                  | 0000361C | 0000005A | 15F2ABEA | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | Key0001
 VSS entry       | Invalid               | 00003678 | 00000110 | 1C10333C | --- Invalid
 VSS entry       | Invalid               | 00003788 | 000000D1 | 40EA30C6 | --- Invalid
 VSS entry       | Invalid               | 0000385C | 000000B1 | 1C806E85 | --- Invalid
 VSS entry       | Invalid               | 00003910 | 00000101 | 7CAC293B | --- Invalid
 VSS entry       | Invalid               | 00003A14 | 000000C2 | B7FD38BC | --- Invalid
 VSS entry       | Invalid               | 00003AD8 | 000000D2 | B03D07E3 | --- Invalid
 VSS entry       | Invalid               | 00003BAC | 00000093 | C5BEF246 | --- Invalid
 VSS entry       | Invalid               | 00003C40 | 00000054 | 7D53807A | --- Invalid
 VSS entry       | Auth                  | 00003C94 | 000000B8 | 7F33223C | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | Boot0001
 VSS entry       | Invalid               | 00003D4C | 00000056 | B4142854 | --- Invalid
 VSS entry       | Auth                  | 00003DA4 | 000000BA | 9C2980E3 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | Boot0002
 VSS entry       | Auth                  | 00003E60 | 00000058 | 4E619F14 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | BootOrder
 VSS entry       | Auth                  | 00003EB8 | 000000A6 | C30DAC1A | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | Boot0003
 VSS entry       | Auth                  | 00003F60 | 00000098 | 0B4D15F8 | --- 4C19049F-4137-4DD3-9C10-8B97A83FFDFA | MemoryTypeInformation
 VSS entry       | Auth                  | 00003FF8 | 00000048 | F204DC79 | --- EB704011-1402-11D3-8E77-00A0C969723B | MTC
 VSS entry       | Invalid               | 00004040 | 000000FA | 7B7FE195 | --- Invalid
 VSS entry       | Invalid               | 0000413C | 0000010B | 44135AC8 | --- Invalid
 VSS entry       | Invalid               | 00004248 | 000000DC | A2F145DC | --- Invalid
 VSS entry       | Invalid               | 00004324 | 0000014A | 5F63563F | --- Invalid
 VSS entry       | Invalid               | 00004470 | 00000139 | 33BCF69E | --- Invalid
 VSS entry       | Invalid               | 000045AC | 0000011B | 82C266DE | --- Invalid
 VSS entry       | Invalid               | 000046C8 | 00000159 | 7E8D1B57 | --- Invalid
 VSS entry       | Invalid               | 00004824 | 00000110 | 1C10333C | --- Invalid
 VSS entry       | Invalid               | 00004934 | 000000D1 | 40EA30C6 | --- Invalid
 VSS entry       | Auth                  | 00004A08 | 000000B1 | 3E31A587 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ConOut
 VSS entry       | Invalid               | 00004ABC | 00000101 | 3EF9B03F | --- Invalid
 VSS entry       | Auth                  | 00004BC0 | 000000C2 | 5C48AE3F | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ConIn
 VSS entry       | Invalid               | 00004C84 | 000000D2 | B03D07E3 | --- Invalid
 VSS entry       | Auth                  | 00004D58 | 00000093 | 2D6DFF61 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ErrOut
 Free space      |                       | 00004DEC | 0003B214 | 8BE00E0B | --- Free space
 Padding         | Empty (0xFF)          | 00040000 | 00001000 | F154670A | -- Padding
 FTW store       |                       | 00041000 | 00001000 | FE3E7280 | -- FTW store
 Free space      |                       | 00042000 | 00042000 | 4BC9AB8D | -- Free space
 Padding         | Non-empty             | 00084000 | 00001000 | 306E7D61 | - Padding
<...>
```

Now let's boot `OVMF.fd` and use our `SetVariableExample.efi` program:
```
FS0:\> SetVariableExample.efi
Delete variable
   SetVariableExample <variable name>

Set variable
   SetVariableExample <variable name> <attributes> <value>

<attributes> can be <n|b|r>
n - NON_VOLATILE
b - BOOTSERVICE_ACCESS
r - RUNTIME_ACCESS
```

Create new non-volatile variable with it:
```
FS0:\> SetVariableExample.efi TestVariable nbr "Test string 1"
Variable TestVariable was successfully changed
```

Not closing QEMU, copy the updated `OVMF.fd` file to the `UEFIExtract` folder and check it for the `TestVariable` string like this:
```
$ cp <...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd ./ && ./UEFIExtract ./OVMF.fd report && grep TestVariable OVMF.fd.report.txt -A 3 -B 8
 VSS entry       | Invalid               | 000054BC | 00000159 | 7E8D1B57 | --- Invalid
 VSS entry       | Invalid               | 00005618 | 00000110 | 1C10333C | --- Invalid
 VSS entry       | Invalid               | 00005728 | 000000D1 | 40EA30C6 | --- Invalid
 VSS entry       | Auth                  | 000057FC | 000000B1 | 3E31A587 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ConOut
 VSS entry       | Invalid               | 000058B0 | 00000101 | 3EF9B03F | --- Invalid
 VSS entry       | Auth                  | 000059B4 | 000000C2 | 5C48AE3F | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ConIn
 VSS entry       | Invalid               | 00005A78 | 000000D2 | B03D07E3 | --- Invalid
 VSS entry       | Auth                  | 00005B4C | 00000093 | 2D6DFF61 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ErrOut
 VSS entry       | Auth                  | 00005BE0 | 00000072 | 8B20882D | --- BB2A829F-7943-4691-A03A-F1F48519D7E6 | TestVariable
 Free space      |                       | 00005C54 | 0003A3AC | D7ACFF21 | --- Free space
 Padding         | Empty (0xFF)          | 00040000 | 00001000 | F154670A | -- Padding
 FTW store       |                       | 00041000 | 00001000 | FE3E7280 | -- FTW store
```

So our variable `TestVariable` was created right at the end of the NV storage.

Now modify variable content with `SetVariableExample` application. In case you don't remember, for that our application simply uses the same `gRT->SetVariable` call:
```
FS0:\> SetVariableExample.efi TestVariable nbr "Test string 2"
Variable TestVariable was successfully changed
```

Look at the NV storage again:
```
$ cp <...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd ./ && ./UEFIExtract ./OVMF.fd report && grep TestVariable OVMF.fd.report.txt -A 3 -B 8
 VSS entry       | Invalid               | 00005618 | 00000110 | 1C10333C | --- Invalid
 VSS entry       | Invalid               | 00005728 | 000000D1 | 40EA30C6 | --- Invalid
 VSS entry       | Auth                  | 000057FC | 000000B1 | 3E31A587 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ConOut
 VSS entry       | Invalid               | 000058B0 | 00000101 | 3EF9B03F | --- Invalid
 VSS entry       | Auth                  | 000059B4 | 000000C2 | 5C48AE3F | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ConIn
 VSS entry       | Invalid               | 00005A78 | 000000D2 | B03D07E3 | --- Invalid
 VSS entry       | Auth                  | 00005B4C | 00000093 | 2D6DFF61 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ErrOut
 VSS entry       | Invalid               | 00005BE0 | 00000072 | D928D86C | --- Invalid
 VSS entry       | Auth                  | 00005C54 | 00000072 | 999527C3 | --- BB2A829F-7943-4691-A03A-F1F48519D7E6 | TestVariable
 Free space      |                       | 00005CC8 | 0003A338 | FF1116DF | --- Free space
 Padding         | Empty (0xFF)          | 00040000 | 00001000 | F154670A | -- Padding
 FTW store       |                       | 00041000 | 00001000 | FE3E7280 | -- FTW store
```

The record before was invalidated, and the new `TestVariable` record was created after it.

You can repeat the process:
```
FS0:\> SetVariableExample.efi TestVariable nbr "Test string 3"
Variable TestVariable was successfully changed
FS0:\> SetVariableExample.efi TestVariable nbr "Test string 4"
Variable TestVariable was successfully changed
FS0:\> SetVariableExample.efi TestVariable nbr "Test string 5"
Variable TestVariable was successfully changed
```

The behaviour would be the same:
```
$ cp <...>/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd ./ && ./UEFIExtract ./OVMF.fd report && grep TestVariable OVMF.fd.report.txt -A 3 -B 8
 VSS entry       | Invalid               | 000058B0 | 00000101 | 3EF9B03F | --- Invalid
 VSS entry       | Auth                  | 000059B4 | 000000C2 | 5C48AE3F | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ConIn
 VSS entry       | Invalid               | 00005A78 | 000000D2 | B03D07E3 | --- Invalid
 VSS entry       | Auth                  | 00005B4C | 00000093 | 2D6DFF61 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ErrOut
 VSS entry       | Invalid               | 00005BE0 | 00000072 | D928D86C | --- Invalid
 VSS entry       | Invalid               | 00005C54 | 00000072 | CB9D7782 | --- Invalid
 VSS entry       | Invalid               | 00005CC8 | 00000072 | 732110E7 | --- Invalid
 VSS entry       | Invalid               | 00005D3C | 00000072 | EEF6285E | --- Invalid
 VSS entry       | Auth                  | 00005DB0 | 00000072 | 04421F7A | --- BB2A829F-7943-4691-A03A-F1F48519D7E6 | TestVariable
 Free space      |                       | 00005E24 | 0003A1DC | B9B6D883 | --- Free space
 Padding         | Empty (0xFF)          | 00040000 | 00001000 | F154670A | -- Padding
 FTW store       |                       | 00041000 | 00001000 | FE3E7280 | -- FTW store
```

Now re-launch QEMU and check the report for the updated `OVMF.fd` file:
```
$ cp ~/tiano/2021/edk2/Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd ./ && ./UEFIExtract ./OVMF.fd report && grep TestVariable OVMF.fd.report.txt -A 20 -B 8
 VSS entry       | Invalid               | 000058B0 | 00000101 | 3EF9B03F | --- Invalid
 VSS entry       | Invalid               | 000059B4 | 000000C2 | B7FD38BC | --- Invalid
 VSS entry       | Invalid               | 00005A78 | 000000D2 | B03D07E3 | --- Invalid
 VSS entry       | Invalid               | 00005B4C | 00000093 | C5BEF246 | --- Invalid
 VSS entry       | Invalid               | 00005BE0 | 00000072 | D928D86C | --- Invalid
 VSS entry       | Invalid               | 00005C54 | 00000072 | CB9D7782 | --- Invalid
 VSS entry       | Invalid               | 00005CC8 | 00000072 | 732110E7 | --- Invalid
 VSS entry       | Invalid               | 00005D3C | 00000072 | EEF6285E | --- Invalid
 VSS entry       | Auth                  | 00005DB0 | 00000072 | 04421F7A | --- BB2A829F-7943-4691-A03A-F1F48519D7E6 | TestVariable
 VSS entry       | Auth                  | 00005E24 | 00000048 | D76F83A5 | --- EB704011-1402-11D3-8E77-00A0C969723B | MTC
 VSS entry       | Invalid               | 00005E6C | 000000FA | 7B7FE195 | --- Invalid
 VSS entry       | Invalid               | 00005F68 | 0000010B | 44135AC8 | --- Invalid
 VSS entry       | Invalid               | 00006074 | 000000DC | A2F145DC | --- Invalid
 VSS entry       | Invalid               | 00006150 | 0000014A | 5F63563F | --- Invalid
 VSS entry       | Invalid               | 0000629C | 00000139 | 33BCF69E | --- Invalid
 VSS entry       | Invalid               | 000063D8 | 0000011B | 82C266DE | --- Invalid
 VSS entry       | Invalid               | 000064F4 | 00000159 | 7E8D1B57 | --- Invalid
 VSS entry       | Invalid               | 00006650 | 00000110 | 1C10333C | --- Invalid
 VSS entry       | Invalid               | 00006760 | 000000D1 | 40EA30C6 | --- Invalid
 VSS entry       | Auth                  | 00006834 | 000000B1 | 3E31A587 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ConOut
 VSS entry       | Invalid               | 000068E8 | 00000101 | 3EF9B03F | --- Invalid
 VSS entry       | Auth                  | 000069EC | 000000C2 | 5C48AE3F | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ConIn
 VSS entry       | Invalid               | 00006AB0 | 000000D2 | B03D07E3 | --- Invalid
 VSS entry       | Auth                  | 00006B84 | 00000093 | 2D6DFF61 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ErrOut
 Free space      |                       | 00006C18 | 000393E8 | 1B524AFE | --- Free space
 Padding         | Empty (0xFF)          | 00040000 | 00001000 | F154670A | -- Padding
 FTW store       |                       | 00041000 | 00001000 | FE3E7280 | -- FTW store
 Free space      |                       | 00042000 | 00042000 | 4BC9AB8D | -- Free space
 Padding         | Non-empty             | 00084000 | 00001000 | 306E7D61 | - Padding
```
From this output you can see that `MTC`/`ConOut`/`ConIn`/`ErrOut` variables are updated on every boot. You can see that new records for them were added to the database. As for the new `Invalid` records they can be:
- some new variables, that were later deleted on boot
- the same `ConOut`/`ConIn`/`ErrOut` variables that were later updated on boot

As `UEFIExtract` tool prints start addresses for the records, we can easily check the `Invalid` records content.

For example to check this record:
```
VSS entry       | Invalid               | 00005E6C | 000000FA | 7B7FE195 | --- Invalid
```
You can use:
```
$ hexdump Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd -s 0x5E6C -n 250 -C
```
(it is not possible to use hex values with the `-n` argument, so I had to convert 0x000000FA to decimal number)

This output would look like this:
```
$ hexdump Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd -s 0x5E6C -n 250 -C
00005e6c  aa 55 3c 00 07 00 00 00  00 00 00 00 00 00 00 00  |.U<.............|
00005e7c  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00005e8c  00 00 00 00 0e 00 00 00  b0 00 00 00 61 df e4 8b  |............a...|
00005e9c  ca 93 d2 11 aa 0d 00 e0  98 03 2b 8c 43 00 6f 00  |..........+.C.o.|         <---- ConOut
00005eac  6e 00 4f 00 75 00 74 00  00 00 02 01 0c 00 d0 41  |n.O.u.t........A|
00005ebc  03 0a 00 00 00 00 01 01  06 00 00 01 02 01 0c 00  |................|
00005ecc  d0 41 01 05 00 00 00 00  03 0e 13 00 00 00 00 00  |.A..............|
00005edc  00 c2 01 00 00 00 00 00  08 01 01 03 0a 14 00 53  |...............S|
00005eec  47 c1 e0 be f9 d2 11 9a  0c 00 90 27 3f c1 4d 7f  |G..........'?.M.|
00005efc  01 04 00 02 01 0c 00 d0  41 03 0a 00 00 00 00 01  |........A.......|
00005f0c  01 06 00 00 02 02 03 08  00 00 01 01 80 7f 01 04  |................|
00005f1c  00 02 01 0c 00 d0 41 03  0a 00 00 00 00 01 01 06  |......A.........|
00005f2c  00 00 01 02 01 0c 00 d0  41 01 05 01 00 00 00 03  |........A.......|
00005f3c  0e 13 00 00 00 00 00 00  c2 01 00 00 00 00 00 08  |................|
00005f4c  01 01 03 0a 14 00 53 47  c1 e0 be f9 d2 11 9a 0c  |......SG........|
00005f5c  00 90 27 3f c1 4d 7f ff  04 00                    |..'?.M....|
00005f66
```
As you can see this record was for the `ConOut` variable.

You can repeat the process for the rest of the `Invalid` records

In my case the situation looked like this:
```
 VSS entry       | Auth                  | 00005DB0 | 00000072 | 04421F7A | --- BB2A829F-7943-4691-A03A-F1F48519D7E6 | TestVariable
 VSS entry       | Auth                  | 00005E24 | 00000048 | D76F83A5 | --- EB704011-1402-11D3-8E77-00A0C969723B | MTC
 VSS entry       | Invalid               | 00005E6C | 000000FA | 7B7FE195 | --- Invalid   <--- ConOut
 VSS entry       | Invalid               | 00005F68 | 0000010B | 44135AC8 | --- Invalid   <--- ConIn
 VSS entry       | Invalid               | 00006074 | 000000DC | A2F145DC | --- Invalid   <--- ErrOut
 VSS entry       | Invalid               | 00006150 | 0000014A | 5F63563F | --- Invalid   <--- ConIn
 VSS entry       | Invalid               | 0000629C | 00000139 | 33BCF69E | --- Invalid   <--- ConOut
 VSS entry       | Invalid               | 000063D8 | 0000011B | 82C266DE | --- Invalid   <--- ErrOut
 VSS entry       | Invalid               | 000064F4 | 00000159 | 7E8D1B57 | --- Invalid   <--- ConOut
 VSS entry       | Invalid               | 00006650 | 00000110 | 1C10333C | --- Invalid   <--- ConOut
 VSS entry       | Invalid               | 00006760 | 000000D1 | 40EA30C6 | --- Invalid   <--- ConOut
 VSS entry       | Auth                  | 00006834 | 000000B1 | 3E31A587 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ConOut
 VSS entry       | Invalid               | 000068E8 | 00000101 | 3EF9B03F | --- Invalid   <--- ConIn
 VSS entry       | Auth                  | 000069EC | 000000C2 | 5C48AE3F | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ConIn
 VSS entry       | Invalid               | 00006AB0 | 000000D2 | B03D07E3 | --- Invalid   <--- ErrOut
 VSS entry       | Auth                  | 00006B84 | 00000093 | 2D6DFF61 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ErrOut
 Free space      |                       | 00006C18 | 000393E8 | 1B524AFE | --- Free space
 Padding         | Empty (0xFF)          | 00040000 | 00001000 | F154670A | -- Padding
 FTW store       |                       | 00041000 | 00001000 | FE3E7280 | -- FTW store
 Free space      |                       | 00042000 | 00042000 | 4BC9AB8D | -- Free space
 Padding         | Non-empty             | 00084000 | 00001000 | 306E7D61 | - Padding
```

So as you see all these `Invalid` records come from 3 variables: `ConIn`/`ConOut`/`ErrOut`. Now think about how many times the NV driver would have to erase the same flash block on every boot if these variables would be modified in-place.

For the final test let's delete our variable
```
FS0:\> SetVariableExample.efi TestVariable
Variable TestVariable was successfully deleted
```

As you can guess this operation simply would mark the variable record as invalid:
```
 VSS entry       | Invalid               | 000058B0 | 00000101 | 3EF9B03F | --- Invalid
 VSS entry       | Invalid               | 000059B4 | 000000C2 | B7FD38BC | --- Invalid
 VSS entry       | Invalid               | 00005A78 | 000000D2 | B03D07E3 | --- Invalid
 VSS entry       | Invalid               | 00005B4C | 00000093 | C5BEF246 | --- Invalid
 VSS entry       | Invalid               | 00005BE0 | 00000072 | D928D86C | --- Invalid
 VSS entry       | Invalid               | 00005C54 | 00000072 | CB9D7782 | --- Invalid
 VSS entry       | Invalid               | 00005CC8 | 00000072 | 732110E7 | --- Invalid
 VSS entry       | Invalid               | 00005D3C | 00000072 | EEF6285E | --- Invalid
 VSS entry       | Invalid               | 00005DB0 | 00000072 | 67B27F04 | --- Invalid                                          <------
 VSS entry       | Auth                  | 00005E24 | 00000048 | D76F83A5 | --- EB704011-1402-11D3-8E77-00A0C969723B | MTC
 VSS entry       | Invalid               | 00005E6C | 000000FA | 7B7FE195 | --- Invalid
 VSS entry       | Invalid               | 00005F68 | 0000010B | 44135AC8 | --- Invalid
 VSS entry       | Invalid               | 00006074 | 000000DC | A2F145DC | --- Invalid
 VSS entry       | Invalid               | 00006150 | 0000014A | 5F63563F | --- Invalid
 VSS entry       | Invalid               | 0000629C | 00000139 | 33BCF69E | --- Invalid
 VSS entry       | Invalid               | 000063D8 | 0000011B | 82C266DE | --- Invalid
 VSS entry       | Invalid               | 000064F4 | 00000159 | 7E8D1B57 | --- Invalid
 VSS entry       | Invalid               | 00006650 | 00000110 | 1C10333C | --- Invalid
 VSS entry       | Invalid               | 00006760 | 000000D1 | 40EA30C6 | --- Invalid
 VSS entry       | Auth                  | 00006834 | 000000B1 | 3E31A587 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ConOut
 VSS entry       | Invalid               | 000068E8 | 00000101 | 3EF9B03F | --- Invalid
 VSS entry       | Auth                  | 000069EC | 000000C2 | 5C48AE3F | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ConIn
 VSS entry       | Invalid               | 00006AB0 | 000000D2 | B03D07E3 | --- Invalid
 VSS entry       | Auth                  | 00006B84 | 00000093 | 2D6DFF61 | --- 8BE4DF61-93CA-11D2-AA0D-00E098032B8C | ErrOut
 Free space      |                       | 00006C18 | 000393E8 | 1B524AFE | --- Free space
 Padding         | Empty (0xFF)          | 00040000 | 00001000 | F154670A | -- Padding
 FTW store       |                       | 00041000 | 00001000 | FE3E7280 | -- FTW store
 Free space      |                       | 00042000 | 00042000 | 4BC9AB8D | -- Free space
 Padding         | Non-empty             | 00084000 | 00001000 | 306E7D61 | - Padding
```

