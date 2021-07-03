
In this lesson we try to show all PCI devices available in a system.

For this task we'll need to utilize `EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL` from the UEFI specification. This protocol is installed to every PCI Root bridge in the system.
It provides various functions to access PCI devices under this root bridge. For example with its help it is possible to read PCI device memory, I/O and configuration spaces for every PCI device:

You can look at a protocol structure to get a hint on what it can do:
```
typedef struct _EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL {
 EFI_HANDLE ParentHandle;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_POLL_IO_MEM PollMem;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_POLL_IO_MEM PollIo;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS Mem;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS Io;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS Pci;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_COPY_MEM CopyMem;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_MAP Map;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_UNMAP Unmap;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_ALLOCATE_BUFFER AllocateBuffer;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_FREE_BUFFER FreeBuffer;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_FLUSH Flush;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_GET_ATTRIBUTES GetAttributes;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_SET_ATTRIBUTES SetAttributes;
 EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_CONFIGURATION Configuration;
 UINT32 SegmentNumber;
} EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL;
```

As in the system can be many PCI root bridges and therefore many `EFI_PCI_ROOT_BRIDGE_IO_PROTOCOLs`, we need to use `LocateHandleBuffer` to get all handles that have this protocol and then loop through these handles using `OpenProtocol` on every one of them.

```
EFI_BOOT_SERVICES.LocateHandleBuffer()

Summary:
Returns an array of handles that support the requested protocol in a buffer allocated from pool.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_HANDLE_BUFFER) (
 IN EFI_LOCATE_SEARCH_TYPE SearchType,
 IN EFI_GUID *Protocol OPTIONAL,
 IN VOID *SearchKey OPTIONAL,
 OUT UINTN *NoHandles,
 OUT EFI_HANDLE **Buffer
 );

Parameters:
SearchType 	Specifies which handle(s) are to be returned.
Protocol 	Provides the protocol to search by. This parameter is only valid for a SearchType of ByProtocol.
SearchKey 	Supplies the search key depending on the SearchType.
NoHandles 	The number of handles returned in Buffer.
Buffer 		A pointer to the buffer to return the requested array of handles that support Protocol.
		This buffer is allocated with a call to the Boot Service EFI_BOOT_SERVICES.AllocatePool().
		It is the caller's responsibility to call the Boot Service EFI_BOOT_SERVICES.FreePool() when the caller no longer
		requires the contents of Buffer.

Description:
The LocateHandleBuffer() function returns one or more handles that match the SearchType request. Buffer is allocated from pool, and the number of entries in Buffer is returned in NoHandles. Each
SearchType is described below:

AllHandles 		Protocol and SearchKey are ignored and the function returns an array of every handle in the system.
ByRegisterNotify 	SearchKey supplies the Registration returned by EFI_BOOT_SERVICES.RegisterProtocolNotify(). 
			The function returns the next handle that is new for the Registration.
			Only one handle is returned at a time, and the caller must loop until 
			no more handles are returned. Protocol is ignored for this search type.
ByProtocol 		All handles that support Protocol are returned. SearchKey is ignored for this search type.
```

```
EFI_STATUS             Status;
UINTN                  HandleCount;
EFI_HANDLE             *HandleBuffer;
Status = gBS->LocateHandleBuffer(
                ByProtocol,
                &gEfiPciRootBridgeIoProtocolGuid,
                NULL,
                &HandleCount,
                &HandleBuffer
              );
if (EFI_ERROR (Status)) {
  Print(L"Can't locate EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL: %r\n", Status);
  return Status;
}

Print(L"Number of PCI root bridges in the system: %d\n", HandleCount);
EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL* PciRootBridgeIo;
for (UINTN Index = 0; Index < HandleCount; Index++) {
  ...
}
FreePool(HandleBuffer);
```
Don't forget to include `<Protocol/PciRootBridgeIo.h>` for the `EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL` and `<Library/MemoryAllocationLib.h>` for the `FreePool`. And offcourse protocol should be included in the app `*.inf` file:
```
[Protocols]
  gEfiPciRootBridgeIoProtocolGuid
```
To get a protocol for particaular handle you can use `OpenProtocol` function:
```
EFI_BOOT_SERVICES.OpenProtocol()

Summary:
Queries a handle to determine if it supports a specified protocol. If the protocol is supported by the
handle, it opens the protocol on behalf of the calling agent. This is an extended version of the EFI boot
service EFI_BOOT_SERVICES.HandleProtocol(). 

Prototype
typedef
EFI_STATUS
(EFIAPI *EFI_OPEN_PROTOCOL) (
 IN EFI_HANDLE Handle,
 IN EFI_GUID *Protocol,
 OUT VOID **Interface OPTIONAL,
 IN EFI_HANDLE AgentHandle,
 IN EFI_HANDLE ControllerHandle,
 IN UINT32 Attributes
 );

Parameters:
Handle 			The handle for the protocol interface that is being opened.
Protocol 		The published unique identifier of the protocol.
Interface 		Supplies the address where a pointer to the corresponding Protocol Interface is returned. NULL will be returned in *Interface if a
			structure is not associated with Protocol. This parameter is optional, and will be ignored if Attributes is EFI_OPEN_PROTOCOL_TEST_PROTOCOL.
AgentHandle 		The handle of the agent that is opening the protocol interface specified by Protocol and Interface. For agents that follow the UEFI
			Driver Model, this parameter is the handle that contains the EFI_DRIVER_BINDING_PROTOCOL instance that is produced by
			the UEFI driver that is opening the protocol interface. For UEFI applications, this is the image handle of the UEFI application that is
			opening the protocol interface. For applications that use HandleProtocol() to open a protocol interface, this parameter is
			the image handle of the EFI firmware.
ControllerHandle 	If the agent that is opening a protocol is a driver that follows the
			UEFI Driver Model, then this parameter is the controller handle that
			requires the protocol interface. If the agent does not follow the UEFI
			Driver Model, then this parameter is optional and may be NULL.
Attributes 		The open mode of the protocol interface specified by Handle and
			Protocol.

Description:
This function opens a protocol interface on the handle specified by Handle for the protocol specified by Protocol.
The first three parameters are the same as EFI_BOOT_SERVICES.HandleProtocol(). The only difference is that the agent that is opening a protocol interface is tracked in an EFI's internal handle
database
```

There are various `Attributes` (last parameter):
```
#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL 0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL 0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL 0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER 0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE 0x00000020
```

We will need `EFI_OPEN_PROTOCOL_GET_PROTOCOL`:
```
GET_PROTOCOL - Used by a driver to get a protocol interface from a handle
```
You can read more about other values in the UEFI specification.


Use `OpenProtocol` call in our loop, and call our custom `EFI_STATUS PrintRootBridge(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL* PciRootBridgeIo)` function for every found protocol:
```
for (UINTN Index = 0; Index < HandleCount; Index++) {
  Status = gBS->OpenProtocol (
                  HandleBuffer[Index],
                  &gEfiPciRootBridgeIoProtocolGuid,
                  (VOID **)&PciRootBridgeIo,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (EFI_ERROR(Status)) {
    Print(L"Can't open protocol: %r\n", Status);
    return Status;
  }
  Print(L"\nPCI Root Bridge %d\n", Index);
  Status = PrintRootBridge(PciRootBridgeIo);
  if (EFI_ERROR(Status)) {
    Print(L"Error in PCI Root Bridge printing\n");
  }
}
```

Now let's write this `PrintRootBridge` function.

First we need to get all available buses for the PCI Root Bridge. To do this we can use `Configuration()` function from the `EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL`:
```
EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL.Configuration()

Summary:

Retrieves the current resource settings of this PCI root bridge in the form of a set of ACPI resource descriptors.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_CONFIGURATION) (
 IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *This,
 OUT VOID **Resources
 );

Parameters:
This 		A pointer to the EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL. 
Resources 	A pointer to the resource descriptors that describe the current configuration of this PCI root bridge.
		The storage for the resource descriptors is allocated by this function. The caller must treat the return
		buffer as read-only data, and the buffer must not be freed by the caller.

Description:
The Configuration() function retrieves a set of resource descriptors that contains the current
configuration of this PCI root bridge.
```

Also here is important information about `ACPI resource descriptors` - the data that we would get from excuting this function:
```
There are only two resource descriptor types from the ACPI Specification that may be used to describe
the current resources allocated to a PCI root bridge. These are the QWORD Address Space Descriptor,
and the End Tag. The QWORD Address Space Descriptor can describe memory, I/O, and bus number
ranges for dynamic or fixed resources. The configuration of a PCI root bridge is described with one or
more QWORD Address Space Descriptors followed by an End Tag
```

So we need to check ACPI specification about 2 types of ACPI resource descriptors:
- QWORD Address Space Descriptor
- End Tag Descriptor

The QWORD address space descriptor is defined here in ACPI specification https://uefi.org/specs/ACPI/6.4/06_Device_Configuration/Device_Configuration.html?#qword-address-space-descriptor

In edk2 its structure is placed in a file https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Acpi10.h
```
///
/// The common definition of QWORD, DWORD, and WORD
/// Address Space Descriptors.
///
typedef PACKED struct {
  UINT8   Desc;
  UINT16  Len;
  UINT8   ResType;
  UINT8   GenFlag;
  UINT8   SpecificFlag;
  UINT64  AddrSpaceGranularity;
  UINT64  AddrRangeMin;
  UINT64  AddrRangeMax;
  UINT64  AddrTranslationOffset;
  UINT64  AddrLen;
} EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR;
```

The end tag descriptor is defined in ACPI spec under https://uefi.org/specs/ACPI/6.4/06_Device_Configuration/Device_Configuration.html#end-tag

Define for it in edk2 is here https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Acpi10.h:
```
#define ACPI_END_TAG_DESCRIPTOR                   0x79
```

So after we get an array of `EFI_ACPI_ADDRESS_SPACE_DESCRIPTORs` from our `PciRootBridgeIo->Configuration` call, we need to loop through it until we would encounter descriptor `ACPI_END_TAG_DESCRIPTOR`.

QWORD address space descriptor can have one of the several resource types:
```
//
// Resource Type
//
#define ACPI_ADDRESS_SPACE_TYPE_MEM   0x00
#define ACPI_ADDRESS_SPACE_TYPE_IO    0x01
#define ACPI_ADDRESS_SPACE_TYPE_BUS   0x02
```

Right now we are interested in `ACPI_ADDRESS_SPACE_TYPE_BUS` type. We need to know, how many PCI buses has this PCI root bridge.

So the code for our function would look like this:
```
EFI_STATUS PrintRootBridge(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL* PciRootBridgeIo)
{
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR* AddressDescriptor;
  EFI_STATUS Status = PciRootBridgeIo->Configuration(
                                         PciRootBridgeIo,
                                         (VOID**)&AddressDescriptor
                                       );
  if (EFI_ERROR(Status)) {
    Print(L"\tError! Can't get EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR: %r\n", Status);
    return Status;
  }
  while (AddressDescriptor->Desc != ACPI_END_TAG_DESCRIPTOR) {
    if (AddressDescriptor->ResType == ACPI_ADDRESS_SPACE_TYPE_BUS) {
      ...
    }
    AddressDescriptor++;
  }
}
return Status;
```

When we know all available buses for the PCI root bridge we can try to read PCI configuration space for its devices with a help of `EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL.Pci.Read()` function:
```
EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL.Pci.Read()
EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL.Pci.Write()

Summary:
Enables a PCI driver to access PCI controller registers in a PCI root bridge’s configuration space.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_IO_MEM) (
 IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *This,
 IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH Width,
 IN UINT64 Address,
 IN UINTN Count,
 IN OUT VOID *Buffer
 );

Parameters:
This		A pointer to the EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL.
Width 		Signifies the width of the memory operations.
Address		The address within the PCI configuration space for the PCI controller.
Count 		The number of PCI configuration operations to perform. Bytes moved is Width size * Count, starting at Address.
Buffer 		For read operations, the destination buffer to store the results.
		For write operations, the source buffer to write data from.

Description:
The Pci.Read() and Pci.Write() functions enable a driver to access PCI configuration registers for a
PCI controller.
All the PCI transactions generated by this function are guaranteed to be completed before this function
returns.

```

The address in this function is defined as follows:

![PCI_Configuration_Address](PCI_Configuration_Address.png?raw=true "PCI_Configuration_Address")

So we write a simple function to create an Address variable from the Bus/Device/Function/Register value:
```
UINT64 PciConfigurationAddress(UINT8 Bus,
                               UINT8 Device,
                               UINT8 Function,
                               UINT32 Register)
{
  UINT64 Address = (((UINT64)Bus) << 24) + (((UINT64)Device) << 16) + (((UINT64)Function) << 8);
  if (Register & 0xFFFFFF00) {
    Address += (((UINT64)Register) << 32);
  } else {
    Address += (((UINT64)Register) << 0);
  }
  return Address;
}
```


Let's try to loop through all possible PCI functions and for every one of them read its header from PCI cofiguration space.

Maximum values for PCI bus, device and function are determined by PCI specification.

In edk2 they are defined in github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Pci22.h:
```
#define PCI_MAX_BUS     255
#define PCI_MAX_DEVICE  31
#define PCI_MAX_FUNC    7
```
As with ACPI newer PCI specifications include the older ones:
```
Pci.h > PciExpress50.h > PciExpress40.h > PciExpress31.h > PciExpress30.h > PciExpress21.h > Pci30.h > Pci23.h > Pci22.h
```
- https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Pci.h
- https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/PciExpress50.h
- https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/PciExpress40.h
- https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/PciExpress31.h
- https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/PciExpress30.h
- https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/PciExpress21.h
- https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Pci30.h
- https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Pci23.h
- https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Pci22.h


For every possible PCI function we would try to read its common PCI configuration space header:
```
///
/// Common header region in PCI Configuration Space
/// Section 6.1, PCI Local Bus Specification, 2.2
///
typedef struct {
  UINT16  VendorId;
  UINT16  DeviceId;
  UINT16  Command;
  UINT16  Status;
  UINT8   RevisionID;
  UINT8   ClassCode[3];
  UINT8   CacheLineSize;
  UINT8   LatencyTimer;
  UINT8   HeaderType;
  UINT8   BIST;
} PCI_DEVICE_INDEPENDENT_REGION;
```

After getting the data we would check if a `VendorId` field is valid. If it is not equal to `0xffff` it is an actual PCI function. In this case we would print some information about it.

Here is a code for this Bus/Device/Func loop:
```
for (UINT8 Bus = AddressDescriptor->AddrRangeMin; Bus <= AddressDescriptor->AddrRangeMax; Bus++) {
  for (UINT8 Device = 0; Device <= PCI_MAX_DEVICE; Device++) {
    for (UINT8 Func = 0; Func <= PCI_MAX_FUNC; Func++) {
      UINT64 Address = PciConfigurationAddress(Bus, Device, Func, 0);
      PCI_DEVICE_INDEPENDENT_REGION PCIConfHdr;
      Status = PciRootBridgeIo->Pci.Read(
        PciRootBridgeIo,
        EfiPciWidthUint8,
        Address,
        sizeof(PCI_DEVICE_INDEPENDENT_REGION),
        &PCIConfHdr
      );
      if (!EFI_ERROR(Status)) {
        if (PCIConfHdr.VendorId != 0xffff) {
          Print(L"\tBus: %02x, Dev: %02x, Func: %02x - Vendor:%04x, Device:%04x\n",
                                                                  Bus,
                                                                  Device,
                                                                  Func,
                                                                  PCIConfHdr.VendorId,
                                                                  PCIConfHdr.DeviceId);
        }
      } else {
        Print(L"\tError in PCI read: %r\n", Status);
      }
    }
  }
}
```

If we build and execute our app under OVMF we would get:
```
FS0:\> ListPCI.efi
Number of PCI root bridges in the system: 1

PCI Root Bridge 0
        Bus: 00, Dev: 00, Func: 00 - Vendor:8086, Device:1237
        Bus: 00, Dev: 01, Func: 00 - Vendor:8086, Device:7000
        Bus: 00, Dev: 01, Func: 01 - Vendor:8086, Device:7010
        Bus: 00, Dev: 01, Func: 03 - Vendor:8086, Device:7113
        Bus: 00, Dev: 02, Func: 00 - Vendor:1234, Device:1111
```


You can verify that our output is correct if you execute UEFI shell `pci` command:

```
FS0:\> pci
   Seg  Bus  Dev  Func
   ---  ---  ---  ----
    00   00   00    00 ==> Bridge Device - Host/PCI bridge
             Vendor 8086 Device 1237 Prog Interface 0
    00   00   01    00 ==> Bridge Device - PCI/ISA bridge
             Vendor 8086 Device 7000 Prog Interface 0
    00   00   01    01 ==> Mass Storage Controller - IDE controller
             Vendor 8086 Device 7010 Prog Interface 80
    00   00   01    03 ==> Bridge Device - Other bridge type
             Vendor 8086 Device 7113 Prog Interface 0
    00   00   02    00 ==> Display Controller - VGA/8514 controller
             Vendor 1234 Device 1111 Prog Interface 0
```

One more thing to end this lesson, you can utilize `PciLib` to access PCI Configuration Space registers. Check out its interface at https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PciLib.h

