https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/PciRootBridgeIo.h
```
typedef struct _EFI_PCI_IO_PROTOCOL {
EFI_PCI_IO_PROTOCOL_POLL_IO_MEM PollMem;
 EFI_PCI_IO_PROTOCOL_POLL_IO_MEM PollIo;
 EFI_PCI_IO_PROTOCOL_ACCESS Mem;
 EFI_PCI_IO_PROTOCOL_ACCESS Io;
 EFI_PCI_IO_PROTOCOL_CONFIG_ACCESS Pci;
 EFI_PCI_IO_PROTOCOL_COPY_MEM CopyMem;
 EFI_PCI_IO_PROTOCOL_MAP Map;
 EFI_PCI_IO_PROTOCOL_UNMAP Unmap;
 EFI_PCI_IO_PROTOCOL_ALLOCATE_BUFFER AllocateBuffer;
 EFI_PCI_IO_PROTOCOL_FREE_BUFFER FreeBuffer;
 EFI_PCI_IO_PROTOCOL_FLUSH Flush;
 EFI_PCI_IO_PROTOCOL_GET_LOCATION GetLocation;
 EFI_PCI_IO_PROTOCOL_ATTRIBUTES Attributes;
 EFI_PCI_IO_PROTOCOL_GET_BAR_ATTRIBUTES GetBarAttributes;
 EFI_PCI_IO_PROTOCOL_SET_BAR_ATTRIBUTES SetBarAttributes;
 UINT64 RomSize;
 VOID *RomImage;
} EFI_PCI_IO_PROTOCOL;
```

As you can see it is pretty simiilar to `EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL`. Here is a comparision of these two:
```
typedef struct _EFI_PCI_IO_PROTOCOL {                        typedef struct _EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL {
                                                              EFI_HANDLE ParentHandle;
 EFI_PCI_IO_PROTOCOL_POLL_IO_MEM PollMem;                     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_POLL_IO_MEM PollMem;
 EFI_PCI_IO_PROTOCOL_POLL_IO_MEM PollIo;                      EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_POLL_IO_MEM PollIo;
 EFI_PCI_IO_PROTOCOL_ACCESS Mem;                              EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS Mem;
 EFI_PCI_IO_PROTOCOL_ACCESS Io;                               EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS Io;
 EFI_PCI_IO_PROTOCOL_CONFIG_ACCESS Pci;                       EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS Pci;
 EFI_PCI_IO_PROTOCOL_COPY_MEM CopyMem;                        EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_COPY_MEM CopyMem;
 EFI_PCI_IO_PROTOCOL_MAP Map;                                 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_MAP Map;
 EFI_PCI_IO_PROTOCOL_UNMAP Unmap;                             EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_UNMAP Unmap;
 EFI_PCI_IO_PROTOCOL_ALLOCATE_BUFFER AllocateBuffer;          EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ALLOCATE_BUFFER AllocateBuffer;
 EFI_PCI_IO_PROTOCOL_FREE_BUFFER FreeBuffer;                  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_FREE_BUFFER FreeBuffer;
 EFI_PCI_IO_PROTOCOL_FLUSH Flush;                             EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_FLUSH Flush;
 EFI_PCI_IO_PROTOCOL_GET_LOCATION GetLocation;
 EFI_PCI_IO_PROTOCOL_ATTRIBUTES Attributes;
 EFI_PCI_IO_PROTOCOL_GET_BAR_ATTRIBUTES GetBarAttributes;     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_GET_ATTRIBUTES GetAttributes;
 EFI_PCI_IO_PROTOCOL_SET_BAR_ATTRIBUTES SetBarAttributes;     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_SET_ATTRIBUTES SetAttributes;
 UINT64 RomSize;
 VOID *RomImage;
                                                              EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_CONFIGURATION Configuration;
                                                              UINT32 SegmentNumber;
} EFI_PCI_IO_PROTOCOL;                                       } EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL;
```




```
RomSize 	The size, in bytes, of the ROM image.
RomImage 	A pointer to the in memory copy of the ROM image. The PCI Bus Driver is responsible
		for allocating memory for the ROM image, and copying the contents of the ROM to memory.
		The contents of this buffer are either from the PCI option ROM that can be accessed
		through the ROM BAR of the PCI controller, or it is from a platformspecific location.
		The Attributes() function can be used to determine from which of these two sources 
		the RomImage buffer was initialized
```

```
GetLocation Retrieves this PCI controller’s current PCI bus number, device
number, and function number
```


```
EFI_PCI_IO_PROTOCOL.GetLocation()

Summary:
Retrieves this PCI controller’s current PCI bus number, device number, and function number.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_PCI_IO_PROTOCOL_GET_LOCATION) (
 IN EFI_PCI_IO_PROTOCOL *This,
 OUT UINTN *SegmentNumber,
 OUT UINTN *BusNumber,
 OUT UINTN *DeviceNumber,
 OUT UINTN *FunctionNumber
 );

Parameters:
This 		A pointer to the EFI_PCI_IO_PROTOCOL instance.
SegmentNumber 	The PCI controller’s current PCI segment number.
BusNumber 	The PCI controller’s current PCI bus number.
DeviceNumber 	The PCI controller’s current PCI device number.
FunctionNumber 	The PCI controller’s current PCI function number.

Description:
The GetLocation() function retrieves a PCI controller’s current location on a PCI Host Bridge. This is
specified by a PCI segment number, PCI bus number, PCI device number, and PCI function number. These
values can be used with the PCI Root Bridge I/O Protocol to perform PCI configuration cycles on the PCI
controller, or any of its peer PCI controller’s on the same PCI Host Bridge.
```

```
FS0:\> DumpPCIroms.efi
Number of PCI devices in the system: 5
00:00.00 - Vendor:8086, Device:1237
00:01.00 - Vendor:8086, Device:7000
00:01.01 - Vendor:8086, Device:7010
00:01.03 - Vendor:8086, Device:7113
00:02.00 - Vendor:1234, Device:1111
  Has OptionROM: address=6E91018, size=39424
```

```
FS0:\> dmem 6E91018 30
Memory Address 0000000006E91018 30 Bytes
  06E91018: 55 AA 4D E9 AE 55 B4 00-00 00 00 00 00 00 00 00  *U.M..U..........*
  06E91028: 00 00 00 00 00 00 00 00-3C 98 00 00 00 00 49 42  *........<.....IB*
  06E91038: 4D 00 2E 8B 16 C6 98 85-D2 74 01 EE C2 02 00 66  *M........t.....f*
```

https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Pci22.h
```
#define PCI_EXPANSION_ROM_HEADER_SIGNATURE              0xaa55

...

///
/// Standard PCI Expansion ROM Header
/// Section 13.4.2, Unified Extensible Firmware Interface Specification, Version 2.1
///
typedef struct {
  UINT16  Signature;    ///< 0xaa55
  UINT8   Reserved[0x16];
  UINT16  PcirOffset;
} PCI_EXPANSION_ROM_HEADER;
```



https://edk2-docs.gitbook.io/edk-ii-uefi-driver-writer-s-guide/18_pci_driver_design_guidelines/readme.7

# EfiRom

In the next sections will be investigating OptionROM images with the help of BaseTools utility `EfiRom`. It is available in your path once you'll execute `. edksetup.sh` in edk2 folder.

First take a look at `EfiRom` help:
```
$ EfiRom -h
Usage: EfiRom -f VendorId -i DeviceId [options] [file name<s>]

Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.

Options:
  -o FileName, --output FileName
            File will be created to store the output content.
  -e EfiFileName
            EFI PE32 image files.
  -ec EfiFileName
            EFI PE32 image files and will be compressed.
  -b BinFileName
            Legacy binary files.
  -l ClassCode
            Hex ClassCode in the PCI data structure header.
  -r Rev    Hex Revision in the PCI data structure header.
  -n        Not to automatically set the LAST bit in the last file.
  -f VendorId
            Hex PCI Vendor ID for the device OpROM, must be specified
  -i DeviceId
            One or more hex PCI Device IDs for the device OpROM, must be specified
  -p, --pci23
            Default layout meets PCI 3.0 specifications
            specifying this flag will for a PCI 2.3 layout.
  -d, --dump
            Dump the headers of an existing option ROM image.
  -v, --verbose
            Turn on verbose output with informational messages.
  --version Show program's version number and exit.
  -h, --help
            Show this help message and exit.
  -q, --quiet
            Disable all messages except FATAL ERRORS.
  --debug [#,0-9]
            Enable debug messages at level #.
```
Complete version of the manual for `EfiRom` is placed at https://github.com/tianocore/edk2/blob/master/BaseTools/UserManuals/EfiRom_Utility_Man_Page.rtf

If you are interested in the source code: https://github.com/tianocore/edk2/tree/master/BaseTools/Source/C/EfiRom

With this tool it is possible to:
- dump Option ROM images
- create Option ROM images from EFI drivers and/or Legacy binary images

Also take a look at tianocore docs:
- https://edk2-docs.gitbook.io/edk-ii-basetools-user-guides/efirom
- https://edk2-docs.gitbook.io/edk-ii-uefi-driver-writer-s-guide/18_pci_driver_design_guidelines/readme.7/1871_efirom_utility

# ipxe rom

Preboot eXecution Environment (PXE) is a standard for booting to an image received via network (https://en.wikipedia.org/wiki/Preboot_Execution_Environment).

To know how to use particular PCI network card for PXE boot, BIOS needs to know some network card internals. But BIOS is not an OS, it is not possible to have drivers for every PCI network card in it. Therefore this problem is solved via OptionROM firmware. PCI network card contains all the necessary code for PXE boot in itself. But not every card have such firmware in itself. In this case you can utilize iPXE project.

iPXE is the open source network boot firmware (https://ipxe.org/). It provides a full PXE implementation enhanced with some additional features. It support various PCI network cards https://ipxe.org/appnote/hardware_drivers.

iPXE can be compiled as EFI application or as EFI/Legacy OptionROM. You can read more about build targets at https://ipxe.org/appnote/buildtargets.

Let's download iPXE:
```
git clone git://git.ipxe.org/ipxe.git
cd ipxe/src
```

Now build some images:
```
$ sudo apt-get install liblzma-dev
$ make bin-x86_64-efi/ipxe.efi 			# EFI app with all devices
$ make bin-x86_64-efi/8086100e.efirom		# EFI ROM vendev: 8086:100e
$ make bin/8086100e.rom                         # Legacy ROM vendev: 8086:100e
```

You can execute `ipxe.efi` directly from the UEFI shell. It is a simple UEFI application similar to the ones that we create in this series.

Look at the https://github.com/ipxe/ipxe/blob/master/src/image/efi_image.c for source code investigation.

`8086100e.rom` is a Legacy code image PCI Option ROM for `8086:100e` network card

`8086100e.efirom` is an UEFI code image PCI Option ROM for `8086:100e` network card

If you inspect the OptionROM images with `hexdump`, you'll see standard `AA55` and `PCIR` signatures in them.

```
$ hexdump bin/8086100e.rom -C -n 64
00000000  55 aa 86 e9 a2 00 30 00  00 00 00 00 00 00 00 00  |U.....0.........|
00000010  9c 00 00 00 00 00 84 00  1c 00 40 00 50 43 49 52  |..........@.PCIR|
00000020  86 80 0e 10 bf 04 1c 00  03 00 00 02 86 00 01 00  |................|
00000030  00 80 07 00 00 00 00 00  8d b4 00 00 8d b4 00 00  |................|
00000040
$ hexdump bin-x86_64-efi/8086100e.efirom -C -n 64
00000000  55 aa d0 00 f1 0e 00 00  0b 00 64 86 01 00 00 00  |U.........d.....|
00000010  00 00 00 00 00 00 38 00  1c 00 00 00 50 43 49 52  |......8.....PCIR|
00000020  86 80 0e 10 00 00 18 00  00 00 00 02 d0 00 00 00  |................|
00000030  03 80 00 00 87 00 00 00  0d 9e 01 00 00 d2 02 00  |................|
00000040
```

We can even use `EfiRom` on them:
```
$ EfiRom -d bin/8086100e.rom
Image 1 -- Offset 0x0
  ROM header contents
    Signature              0xAA55
    PCIR offset            0x001C
    Signature               PCIR
    Vendor ID               0x8086
    Device ID               0x100E
    Length                  0x001C
    Revision                0x0003
    DeviceListOffset        0x4BF
    Device list contents
      0x100E
    Class Code              0x020000
    Image size              0x10C00
    Code revision:          0x0001
    MaxRuntimeImageLength   0x07
    ConfigUtilityCodeHeaderOffset 0x00
    DMTFCLPEntryPointOffset 0x00
    Indicator               0x80   (last image)
    Code type               0x00
```
If we execute `EfiRom` on a `bin-x86_64-efi/8086100e.efirom`, we would get an error:
```
$ EfiRom -d bin-x86_64-efi/8086100e.efirom
EfiRom: ERROR 1002: No PciRom input file
  No *.rom input file
```
The problem is that we `EfiRom` understands only files with `.rom` extension, so change it and use `EfiRom` again:
```
$ cp bin-x86_64-efi/8086100e.efirom bin-x86_64-efi/8086100e.rom
$ EfiRom -d bin-x86_64-efi/8086100e.rom
Image 1 -- Offset 0x0
  ROM header contents
    Signature              0xAA55
    PCIR offset            0x001C
    Signature               PCIR
    Vendor ID               0x8086
    Device ID               0x100E
    Length                  0x0018
    Revision                0x0000
    DeviceListOffset        0x00
    Class Code              0x020000
    Image size              0x1A000
    Code revision:          0x0000
    MaxRuntimeImageLength   0x00
    ConfigUtilityCodeHeaderOffset 0x87
    DMTFCLPEntryPointOffset 0x00
    Indicator               0x80   (last image)
    Code type               0x03   (EFI image)
  EFI ROM header contents
    EFI Signature          0x0EF1
    Compression Type       0x0001 (compressed)
    Machine type           0x8664 (X64)
    Subsystem              0x000B (EFI boot service driver)
    EFI image offset       0x0038 (@0x38)
```

EfiRom can't currently combine Option ROM from the EFI ROM images. But it can use EFI PE32 image files as a source for EFI ROM images in the resulting Option ROM. So we have to compile another target:
```
$ make bin-x86_64-efi/8086100e.efidrv
```
Now we can create combined OptionROM image with both Legacy and EFI ROMs.
```
$ EfiRom -f 0x8086 -i 0x100e -b bin/8086100e.rom -ec bin-x86_64-efi/8086100e.efidrv -o bin/8086100e_optionrom.rom
$ EfiRom -d bin/8086100e_optionrom.rom
Image 1 -- Offset 0x0
  ROM header contents
    Signature              0xAA55
    PCIR offset            0x001C
    Signature               PCIR
    Vendor ID               0x8086
    Device ID               0x100E
    Length                  0x001C
    Revision                0x0003
    DeviceListOffset        0x4BF
    Device list contents
      0x100E
    Class Code              0x020000
    Image size              0x10C00
    Code revision:          0x0001
    MaxRuntimeImageLength   0x07
    ConfigUtilityCodeHeaderOffset 0x00
    DMTFCLPEntryPointOffset 0x00
    Indicator               0x00
    Code type               0x00
Image 2 -- Offset 0x10C00
  ROM header contents
    Signature              0xAA55
    PCIR offset            0x001C
    Signature               PCIR
    Vendor ID               0x8086
    Device ID               0x100E
    Length                  0x001C
    Revision                0x0003
    DeviceListOffset        0x00
    Class Code              0x000000
    Image size              0x1A000
    Code revision:          0x0000
    MaxRuntimeImageLength   0x00
    ConfigUtilityCodeHeaderOffset 0x00
    DMTFCLPEntryPointOffset 0x00
    Indicator               0x80   (last image)
    Code type               0x03   (EFI image)
  EFI ROM header contents
    EFI Signature          0x0EF1
    Compression Type       0x0001 (compressed)
    Machine type           0x8664 (X64)
    Subsystem              0x000B (EFI boot service driver)
    EFI image offset       0x0038 (@0x10C38)
```

# VGA rom

SeaBIOS is an open-source legacy BIOS implementation that implements the standard BIOS calling interfaces that a typical x86 proprietary BIOS implements (https://github.com/coreboot/seabios).
SeaVGABIOS is a sub-project of the SeaBIOS project - it is an open source implementation of a 16bit X86 VGA BIOS
(https://github.com/coreboot/seabios/blob/master/docs/SeaVGABIOS.md). In other words it builds a Legacy Option ROM for a PCI graphic device.

SeaBIOS uses Kconfig system for the build configuration. Main options for the VGA BIOS are placed under https://github.com/coreboot/seabios/blob/master/vgasrc/Kconfig In this file you can see that one of the options is a OptionROM with a Vendor/Device pair as `1234:1111`. Exactly the one that we saw at QEMU:
```
    config VGA_VID
        depends on VGA_PCI
        hex
        prompt "PCI Vendor ID" if OVERRIDE_PCI_ID
        default 0x1013 if VGA_CIRRUS
        default 0x1002 if VGA_ATI
        default 0x1234 if VGA_BOCHS_STDVGA
        default 0x15ad if VGA_BOCHS_VMWARE
        default 0x1b36 if VGA_BOCHS_QXL
        default 0x1af4 if VGA_BOCHS_VIRTIO
        default 0x100b if VGA_GEODEGX2
        default 0x1022 if VGA_GEODELX
        default 0x1234 if DISPLAY_BOCHS                 <--------------
        default 0x0000
        help
            Vendor ID for the PCI ROM

    config VGA_DID
        depends on VGA_PCI
        hex
        prompt "PCI Vendor ID" if OVERRIDE_PCI_ID
        default 0x00b8 if VGA_CIRRUS
        default 0x5159 if VGA_ATI
        default 0x1111 if VGA_BOCHS_STDVGA
        default 0x0405 if VGA_BOCHS_VMWARE
        default 0x0100 if VGA_BOCHS_QXL
        default 0x1050 if VGA_BOCHS_VIRTIO
        default 0x0030 if VGA_GEODEGX2
        default 0x2081 if VGA_GEODELX
        default 0x1111 if DISPLAY_BOCHS                 <---------------
        default 0x0000
        help
            Device ID for the PCI ROM
```
If you wonder what is `DISPLAY_BOCHS`, here is a help for this option:
```
        config DISPLAY_BOCHS
            depends on QEMU
            bool "qemu bochs-display support"
            select VGA_EMULATE_TEXT
            help
                Build support for the qemu bochs-display device, which
                is basically qemu stdvga without the legacy vga
                emulation, supporting only 16+32 bpp VESA video modes
                in a linear framebuffer.  So this uses cbvga text mode
                emulation.

                The bochs-display device is available in qemu
                v3.0+. The vgabios works with the qemu stdvga too (use
                "qemu -device VGA,romfile=/path/to/vgabios.bin")".
```

Let's build this SeaBIOS VGA image. Install necessary packages, download SeaBIOS source and perfrorm build configuration:
```
$ sudo apt-get install python
$ git clone https://github.com/coreboot/seabios.git
$ cd seabios
$ make menuconfig
```
In a menuconfig select:
```
VGA ROM ---> VGA Hardware Type (qemu bochs-display support)
```
After that execute:
```
$ make
```

After the build resulting VGA Option ROM would be at path `out/vgabios.bin`. As `EfiRom` expects ROM files to have a `*.rom` extension, create a copy of a file with such extension. After that execute `dump` command on this file:
```
$ cp out/vgabios.bin out/vgabios.rom
$ EfiRom -d out/vgabios.rom
Image 1 -- Offset 0x0
  ROM header contents
    Signature              0xAA55
    PCIR offset            0x6E00
    Signature               PCIR
    Vendor ID               0x1234
    Device ID               0x1111
    Length                  0x0018
    Revision                0x0000
    DeviceListOffset        0x00
    Class Code              0x030000
    Image size              0x7000
    Code revision:          0x0001
    MaxRuntimeImageLength   0x00
    ConfigUtilityCodeHeaderOffset 0x9066
    DMTFCLPEntryPointOffset 0x9066
    Indicator               0x80   (last image)
    Code type               0x00
```
This is the info similar to the one that we saw with our utility on a working QEMU system. You can see that PCI vendor/device pair is set to `1234:1111`.


# How QEMU gets these OptionROMs

The main Makefile in QEMU responsible for OptionROM image creation is https://github.com/qemu/qemu/blob/master/roms/Makefile

In this Makefile you can see how:
- `seavgabios-%` target is used for the creation of a Legacy SeaVGABIOS OptionROM
- `efi-rom-%` target is used for the creation of an OptionROM with both Legacy and UEFI iPXE code images (it even uses `EfiRom` utility from edk2 for this purpose)

