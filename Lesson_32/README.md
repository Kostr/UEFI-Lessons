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



```
$ git clone git://git.ipxe.org/ipxe.git
$ cd ipxe/src
$ make bin-x86_64-efi/ipxe.efi 			# EFI app with all devices
$ make bin-x86_64-efi/808610de.efirom		# EFI ROM vendev: 8086:10de
$ make bin/808610de.rom                         # Legacy ROM vendev: 8086:10de
```

# EfiRom

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
Complete version of the manual is placed at https://github.com/tianocore/edk2/blob/master/BaseTools/UserManuals/EfiRom_Utility_Man_Page.rtf

If you are interested in the source code: https://github.com/tianocore/edk2/tree/master/BaseTools/Source/C/EfiRom

With this tool it is possible to:
- dump Option ROM images
- create ROM images from a EFI drivers and/or Legacy binary images

https://edk2-docs.gitbook.io/edk-ii-uefi-driver-writer-s-guide/18_pci_driver_design_guidelines/readme.7
https://edk2-docs.gitbook.io/edk-ii-basetools-user-guides/efirom



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

