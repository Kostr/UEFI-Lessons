In this lesson we would try to list OptionROMs that are present in the PCI devices in our system.

# Theory

Some PCI devices like graphic cards require additional 'drivers' to work at the BIOS stage.

But BIOS is not an OS, it can't have all the drivers for all the possible PCI devices that you can plug into your board. To solve this problem such complex PCI devices store its 'driver' it its own firmware. This is a concept known as a PCI Option ROM https://en.wikipedia.org/wiki/Option_ROM

BIOS queries PCI configuration space in every PCI device and determines if it has an option ROM. If it has BIOS can execute code from the option ROM depending on the BIOS settings.

PCI Option ROM can have several code images in itself. The common case is that option ROM contains two images:
- driver for the legacy BIOS (Legacy OpROM)
- driver for the UEFI BIOS (UEFI OpROM)

Keep in mind that older cards can contain only driver for the legacy BIOS, newer/future ones can contain only driver for the UEFI BIOS.

Currently the world is in the transition process from Legacy BIOS interface to UEFI. UEFI firmware doesn't work with Legacy OpROMs. But for now vendors still leave a possibility to work with such legacy interfaces via CSM mode.

But you wouldn't see any output, if you set a BIOS policy to load only UEFI OpROMs, but your graphic card only contains Legacy OpROM.

Also OptionROM can have various code images for different CPU architectures, so this PCI device could work both with ARM CPU host and x86 CPU host.

As you can see it might be very interesting to know which OptionROMs your PCI device contain. So let's write the UEFI Shell utility that can show us that.

# `EFI_PCI_IO_PROTOCOL`

To access PCI devices we would utilize `EFI_PCI_IO_PROTOCOL` https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/PciRootBridgeIo.h
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

As you can see it is pretty similar to the `EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL` we've used in the last lesson. Here is a comparision of these two:
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

The main difference is that `EFI_PCI_IO_PROTOCOL` is attached to every PCI device in the system and the `EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL` is attached only to PCI Root Bridges.

To access PCI OptionROM we would need to utilize these parameters from the `EFI_PCI_IO_PROTOCOL`:
```
RomSize 	The size, in bytes, of the ROM image.
RomImage 	A pointer to the in memory copy of the ROM image. The PCI Bus Driver is responsible
		for allocating memory for the ROM image, and copying the contents of the ROM to memory.
		The contents of this buffer are either from the PCI option ROM that can be accessed
		through the ROM BAR of the PCI controller, or it is from a platformspecific location.
		The Attributes() function can be used to determine from which of these two sources 
		the RomImage buffer was initialized
```

First let's try to enumerate PCI devices in our system with this `EFI_PCI_IO_PROTOCOL`. The logic is similar to the one, that we've used in the previous lesson. First we find all handles that have `EFI_PCI_IO_PROTOCOL` protocol via `LocateHandleBuffer` API and then we actually get the each protocol with a `OpenProtocol` call. On every protocol we would execute our `PrintPCI` function that we would write next:
```
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS             Status;
  UINTN                  HandleCount;
  EFI_HANDLE             *HandleBuffer;
  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                );
  if (EFI_ERROR (Status)) {
    Print(L"Can't locate EFI_PCI_IO_PROTOCOL: %r\n", Status);
    return Status;
  }

  //Print(L"Number of PCI devices in the system: %d\n", HandleCount);
  EFI_PCI_IO_PROTOCOL* PciIo;
  for (UINTN Index = 0; Index < HandleCount; Index++) {
    Status = gBS->OpenProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo,
                    ImageHandle,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (EFI_ERROR(Status)) {
      Print(L"Can't open protocol: %r\n", Status);
      return Status;
    }
    Status = PrintPCI(PciIo);
    if (EFI_ERROR(Status)) {
      Print(L"Error in PCI printing\n");
    }

  }
  FreePool(HandleBuffer);

  return EFI_SUCCESS;
}
```

When we have a `EFI_PCI_IO_PROTOCOL` we can execute `GetLocation()` to know the current PCI controller’s address:
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

So let's write our `PrintPCI` function:
```
EFI_STATUS PrintPCI(EFI_PCI_IO_PROTOCOL* PciIo)
{
  UINTN SegmentNumber;
  UINTN BusNumber;
  UINTN DeviceNumber;
  UINTN FunctionNumber;
  EFI_STATUS Status = PciIo->GetLocation(PciIo,
                                         &SegmentNumber,
                                         &BusNumber,
                                         &DeviceNumber,
                                         &FunctionNumber);
  if (EFI_ERROR(Status)) {
    Print(L"Error in getting PCI location: %r\n", Status);
    return Status;
  }

  PCI_DEVICE_INDEPENDENT_REGION PCIConfHdr;
  Status = PciIo->Pci.Read(PciIo,
                           EfiPciIoWidthUint8,
                           0,
                           sizeof(PCI_DEVICE_INDEPENDENT_REGION),
                           &PCIConfHdr);

  if (EFI_ERROR(Status)) {
    Print(L"Error in reading PCI conf space: %r\n", Status);
    return Status;
  }

  Print(L"%02x:%02x.%02x - Vendor:%04x, Device:%04x", BusNumber,
                                                      DeviceNumber,
                                                      FunctionNumber,
                                                      PCIConfHdr.VendorId,
                                                      PCIConfHdr.DeviceId);
    
  return Status;
}

```
After using `GetLocation` API we've used `Pci.Read` call to read PCI configuration space like we did in earlier lesson.

If you'll build and execute our app now you would get the same list of PCI devices, that we've received with a `EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL`.
```
FS0:\> DumpPCIroms.efi
00:00.00 - Vendor:8086, Device:1237
00:01.00 - Vendor:8086, Device:7000
00:01.01 - Vendor:8086, Device:7010
00:01.03 - Vendor:8086, Device:7113
00:02.00 - Vendor:1234, Device:1111
```

But right now we are interested only in devices that have PCI Option ROM, so replace the `Print` statement to:
```
if (PciIo->RomSize) {
  Print(L"%02x:%02x.%02x - Vendor:%04x, Device:%04x", BusNumber,
                                                      DeviceNumber,
                                                      FunctionNumber,
                                                      PCIConfHdr.VendorId,
                                                      PCIConfHdr.DeviceId);

  CHAR16 VendorDesc[DESCRIPTOR_STR_MAX_SIZE];
  CHAR16 DeviceDesc[DESCRIPTOR_STR_MAX_SIZE];
  Status = FindPCIDevDescription(PCIConfHdr.VendorId,
                                 PCIConfHdr.DeviceId,
                                 VendorDesc,
                                 DeviceDesc,
                                 DESCRIPTOR_STR_MAX_SIZE);
  if (!EFI_ERROR(Status)) {
    Print(L":    %s, %s\n", VendorDesc, DeviceDesc);
  } else {
    Print(L"\n");
  }
  PrintOptionROM(PciIo->RomImage, PciIo->RomSize);
}
```
Here I've used a `FindPCIDevDescription` function, you need to copy it from the last lesson. Also the main OptionROM printing would be happening in a `PrintOptionROM` that we'll write next.

But first some theory.

OptionROM image in memory would have a special header. edk2 definition for the header structure is `PCI_EXPANSION_ROM_HEADER`. This header contains signature field that is equal to `0xAA55`. You can check out definitions in a file https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Pci22.h:
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
This header has offset (`PcirOffset`) to another header `PCI_DATA_STRUCTURE`. It also has a `signature` field which is equal to `PCIR` in this case:
```
///
/// PCI Data Structure Format
/// Section 6.3.1.2, PCI Local Bus Specification, 2.2
///
typedef struct {
  UINT32  Signature;    ///< "PCIR"
  UINT16  VendorId;
  UINT16  DeviceId;
  UINT16  Reserved0;
  UINT16  Length;
  UINT8   Revision;
  UINT8   ClassCode[3];
  UINT16  ImageLength;
  UINT16  CodeRevision;
  UINT8   CodeType;
  UINT8   Indicator;
  UINT16  Reserved1;
} PCI_DATA_STRUCTURE;
```
These two headers prepends every code image in an Option ROM. As you remember there can be several code images in the Option ROM. To find all of them we need to loop through the header structures.
Two fields will help us to do it:
- `PCI_DATA_STRUCTURE.ImageLength` - describes the total code image size in 512 byte increments,
- `PCI_DATA_STRUCTURE.Indicator` - the most significant bit of this byte tells if this is the last code image in the option ROM

With this in mind let's write the main loop for the Option ROM parsing:
```
VOID PrintOptionROM(VOID *RomImage, UINT64 RomSize)
{
  Print(L"Has OptionROM at memory %p-%p\n", RomImage, (UINT8*)RomImage + RomSize);
  PCI_EXPANSION_ROM_HEADER* RomHeader = (PCI_EXPANSION_ROM_HEADER*) RomImage;
  UINTN RomImageIndex = 1;
  while (TRUE)
  {
    if (RomHeader->Signature != PCI_EXPANSION_ROM_HEADER_SIGNATURE) {
      Print(L"Error! OptionROM has a wrong signature\n");
      return;
    }
    PCI_DATA_STRUCTURE* RomImage = (PCI_DATA_STRUCTURE*)((UINT8*)RomHeader + RomHeader->PcirOffset);
    if ((((CHAR8)((RomImage->Signature >>  0) & 0xFF)) != 'P') &&
        (((CHAR8)((RomImage->Signature >>  8) & 0xFF)) != 'C') &&
        (((CHAR8)((RomImage->Signature >> 16) & 0xFF)) != 'I') &&
        (((CHAR8)((RomImage->Signature >> 24) & 0xFF)) != 'R')) {
      Print(L"Error! OptionROM image has wrong signature\n");
      return;
    }

    <...>

    if ((RomImage->Indicator) & 0x80) {
      break;
    }
    RomHeader = (PCI_EXPANSION_ROM_HEADER*)((UINT8*) RomHeader + (RomImage->ImageLength)*512);
    RomImageIndex++;
  }
  Print(L"------------------\n");
}
```

In the `<...>` we should write an actual parsing of the code image.

UEFI specification was trying to adapt to the current Option ROM design that was already present when UEFI is originated. It appears that original Option ROM design wasn't thought with extendability in mind. Therefore UEFI extended it as it could, and now the parsing process may look a little bit strange. 

First we need to look at the field `PCI_DATA_STRUCTURE.CodeType` in the second header. And by this field interpret the first `PCI_EXPANSION_ROM_HEADER`.

That is why there is a strange `union` in the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Pci22.h:
```
typedef union {
  UINT8                           *Raw;
  PCI_EXPANSION_ROM_HEADER        *Generic;
  EFI_PCI_EXPANSION_ROM_HEADER    *Efi;
  EFI_LEGACY_EXPANSION_ROM_HEADER *PcAt;
} EFI_PCI_ROM_HEADER;
```
It means that:
- if an image is Legacy (`PCI_DATA_STRUCTURE.CodeType = 0x00`) then you should treat `EFI_PCI_ROM_HEADER` as a `EFI_LEGACY_EXPANSION_ROM_HEADER`
- if an image is UEFI (`PCI_DATA_STRUCTURE.CodeType = 0x03`) then you should treat `EFI_PCI_ROM_HEADER` as a `EFI_PCI_EXPANSION_ROM_HEADER`
- if you don't know image type yet, you can only treat `EFI_PCI_ROM_HEADER` as a `PCI_EXPANSION_ROM_HEADER`


There is nothing useful for us in the `EFI_LEGACY_EXPANSION_ROM_HEADER`, so we would only print additional information if the image is EFI:
```
Print(L"---Code Image %d---\n", RomImageIndex);
Print(L"Address: %p-%p\n", RomHeader, (UINT8*)RomHeader + (RomImage->ImageLength)*512);
Print(L"VendorId: %04x, DeviceId: %04x\n", RomImage->VendorId, RomImage->DeviceId);
Print(L"Type: ");
switch (RomImage->CodeType) {
  case 0x00:
    Print(L"IA-32, PC-AT compatible\n");
    break;
  case 0x01:
    Print(L"Open Firmware standard for PCI\n");
    break;
  case 0x02:
    Print(L"Hewlett-Packard PA RISC\n");
    break;
  case 0x03:
    Print(L"EFI Image\n");
    break;
  default:
    Print(L"Unknown\n");
    break;
}
if (RomImage->CodeType == 0x03) {
  EFI_PCI_EXPANSION_ROM_HEADER* EfiRomHeader =  (EFI_PCI_EXPANSION_ROM_HEADER*) RomHeader;
  if (EfiRomHeader->EfiSignature == EFI_PCI_EXPANSION_ROM_HEADER_EFISIGNATURE) {
    <print additional EFI image info>
  } else {
    Print(L"EFI signature is incorrect!\n");
  }
}
```

If the image is EFI we can print its type, target CPU architecture and compression:
```
Print(L"Subsystem: ");
switch (EfiRomHeader->EfiSubsystem) {
  case EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION:
    Print(L"EFI Application\n");
    break;
  case EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
    Print(L"EFI Boot Service Driver\n");
    break;
  case EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
    Print(L"EFI Runtime Driver\n");
    break;
  case EFI_IMAGE_SUBSYSTEM_SAL_RUNTIME_DRIVER:
    Print(L"EFI SAL Runtime Driver\n");
    break;
  default:
    Print(L"Unknown\n");
    break;
}
Print(L"Machine type: ");
switch (EfiRomHeader->EfiMachineType) {
  case IMAGE_FILE_MACHINE_I386:
    Print(L"IA-32\n");
    break;
  case IMAGE_FILE_MACHINE_IA64:
    Print(L"Itanium\n");
    break;
  case IMAGE_FILE_MACHINE_EBC:
    Print(L"EFI Byte Code (EBC)\n");
    break;
  case IMAGE_FILE_MACHINE_X64:
    Print(L"X64\n");
    break;
  case IMAGE_FILE_MACHINE_ARMTHUMB_MIXED:
    Print(L"ARM\n");
    break;
  case IMAGE_FILE_MACHINE_ARM64:
    Print(L"ARM 64-bit\n");
    break;
  case IMAGE_FILE_MACHINE_RISCV32:
    Print(L"RISCV32\n");
    break;
  case IMAGE_FILE_MACHINE_RISCV64:
    Print(L"RISCV64\n");
    break;
  case IMAGE_FILE_MACHINE_RISCV128:
    Print(L"RISCV128\n");
    break;
  default:
    Print(L"Unknown\n");
    break;
}
switch (EfiRomHeader->CompressionType) {
  case EFI_PCI_EXPANSION_ROM_HEADER_COMPRESSED:
    Print(L"Compressed following the UEFI Compression Algorithm\n");
    break;
  case 0:
    Print(L"Uncompressed\n");
    break;
  default:
    Print(L"Unknown compression type\n");
    break;
}
```
Check out "PCI Option ROMs" part in the UEFI spec for the explanation of the parsing process.
Also some of these case statements use constants from the PE specification, therefore we have to include PE Image header https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/PeImage.h:
```
#include <IndustryStandard/PeImage.h>
```

Build our program. After that copy its `*.efi` executable to our QEMU shared folder and run OVMF with a command:
```
$ qemu-system-x86_64 -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
                     -nographic \
                     -net none
```

If you execute our program now you would get:
```
00:02.00 - Vendor:1234, Device:1111:    Undefined, Undefined
Has OptionROM at memory 6EA5018-6EAEA18
---Code Image 1---
Address: 6EA5018-6EAEA18
PCIR address: 6EAE854
VendorId: 1234, DeviceId: 1111
Type: IA-32, PC-AT compatible
------------------
```

You can use `dmem` to peak in the Option ROM regions. At the `6EA5018` you can see the first header with its `AA55` signature:
```
FS0:\> dmem 6EA5018 30
Memory Address 0000000006EA5018 30 Bytes
  06EA5018: 55 AA 4D E9 AE 55 B4 00-00 00 00 00 00 00 00 00  *U.M..U..........*
  06EA5028: 00 00 00 00 00 00 00 00-3C 98 00 00 00 00 49 42  *........<.....IB*
  06EA5038: 4D 00 2E 8B 16 C6 98 85-D2 74 01 EE C2 02 00 66  *M........t.....f*
```
And at the `6EAE854` you can observe the second header with its `PCIR` signature. After it immediately follows PCI vendor/device pair (`1234:1111`):
```
FS0:\> dmem 6EAE854 30
Memory Address 0000000006EAE854 30 Bytes
  06EAE854: 50 43 49 52 34 12 11 11-00 00 18 00 00 00 00 03  *PCIR4...........*
  06EAE864: 4D 00 01 00 00 80 00 00-66 90 66 90 66 90 66 90  *M.......f.f.f.f.*
  06EAE874: 66 90 66 90 67 63 63 3A-20 28 55 62 75 6E 74 75  *f.f.gcc: (Ubuntu*
```


Now let's rerun our QEMU without `-net none` option:
```
$ qemu-system-x86_64 -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
                     -nographic
```
And execute our program again:
```
00:02.00 - Vendor:1234, Device:1111:    Undefined, Undefined
Has OptionROM at memory 6EA5018-6EAEA18
---Code Image 1---
Address: 6EA5018-6EAEA18
PCIR address: 6EAE854
VendorId: 1234, DeviceId: 1111
Type: IA-32, PC-AT compatible
------------------
00:03.00 - Vendor:8086, Device:100E:    Intel Corporation, 82540EM Gigabit Ethernet Controller
Has OptionROM at memory 6E59018-6EA4218
---Code Image 1---
Address: 6E59018-6E6F418
PCIR address: 6E59034
VendorId: 8086, DeviceId: 100E
Type: IA-32, PC-AT compatible
---Code Image 2---
Address: 6E6F418-6EA4218
PCIR address: 6E6F434
VendorId: 8086, DeviceId: 100E
Type: EFI Image
Subsystem: EFI Boot Service Driver
Machine type: X64
Uncompressed
------------------
```
Now besides the Legacy Option ROM for the `1234:1111` VGA QEMU device there is another Option ROM.

This second Option ROM is for the PCI network card and it has two code images in itself: for legacy BIOS and for UEFI.

If you peak in the second image you'll see some strings that will give you a clue that this is an iPXE image (https://ipxe.org/start):
```
FS0:\> dmem 6E59018 c0
Memory Address 0000000006E59018 C0 Bytes
  06E59018: 55 AA B2 E9 A2 00 22 00-00 00 00 00 00 00 00 00  *U.....".........*
  06E59028: 9C 00 00 00 00 00 84 00-1C 00 40 00 50 43 49 52  *..........@.PCIR*
  06E59038: 86 80 0E 10 BF 04 1C 00-03 02 00 00 B2 00 01 00  *................*
  06E59048: 00 00 07 00 00 00 00 00-8D B4 00 00 8D B4 00 00  *................*
  06E59058: 24 50 6E 50 01 02 00 00-00 7D 00 00 00 00 60 00  *$PnP.....}....`.*
  06E59068: 70 00 02 00 00 F4 00 00-00 00 85 03 00 00 00 00  *p...............*
  06E59078: 68 74 74 70 3A 2F 2F 69-70 78 65 2E 6F 72 67 00  *http://ipxe.org.*
  06E59088: 69 50 58 45 00 28 50 43-49 20 78 78 3A 78 78 2E  *iPXE.(PCI xx:xx.*
  06E59098: 78 29 00 90 55 4E 44 49-16 1D 00 00 01 02 A4 04  *x)..UNDI........*
  06E590A8: 10 2D 10 2D 42 08 50 43-49 52 66 90 69 50 58 45  *.-.-B.PCIRf.iPXE*
  06E590B8: 0C 4E 07 00 FE DB B2 BE-60 1E 06 0F A0 0F A8 FC  *.N......`.......*
  06E590C8: 0E 1F BE 27 03 31 FF E8-4C 04 8E EB 31 FF E8 3D  *...'.1..L...1..=*
```

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

# Code images for different CPU architectures in OptionROM image

In the intro I've mentioned that PCI device can have various code images for different CPU architectures.

Check out this guide as an example https://www.workofard.com/2020/12/aarch64-option-roms-for-amd-gpus/

It explains how to add the AMD AARCH64 Gop Driver as another Code Image to the PCI graphic card OptionROM to make the graphic card work at boot time in AArch64 systems.

It uses `flashrom` utility to download initial PCI card Option ROM firmware and with a help of `EfiRom` adds AMD EFI graphic driver for AArch64 to the list of images in this OptionROM.

