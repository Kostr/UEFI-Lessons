In this lesson we will be investigating OptionROM images with the help of BaseTools utility `EfiRom`. It is available in your path once you'll execute `. edksetup.sh` in edk2 folder.

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

# Code images for different CPU architectures in OptionROM image

In the previous lesson I've mentioned that PCI device can have various code images for different CPU architectures.

Check out this guide as an example https://www.workofard.com/2020/12/aarch64-option-roms-for-amd-gpus/

It explains how to add the AMD AARCH64 Gop Driver as another Code Image to the PCI graphic card OptionROM to make the graphic card work at boot time in AArch64 systems.

It uses `flashrom` utility to download initial PCI card Option ROM firmware and with a help of `EfiRom` adds AMD EFI graphic driver for AArch64 to the list of images in this OptionROM.
