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
