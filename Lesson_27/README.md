Let's try to get information from one of the system tables that we've discovered.

You can find the most recent version of the "System Management BIOS (SMBIOS) Reference Specification" on the DMTF site https://www.dmtf.org/dsp/DSP0134
For example now 3.4.0 is the most recent version https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.4.0.pdf

From the previous lesson we now, that SMBIOS table is declared under `gEfiSmbiosTableGuid`.

Let's create an app, that would print the address of this table.
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/BaseMemoryLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  for (UINTN i=0; i<SystemTable->NumberOfTableEntries; i++) {
    if (CompareGuid(&(SystemTable->ConfigurationTable[i].VendorGuid), &gEfiSmbiosTableGuid)) {
      Print(L"SMBIOS table is placed at %p\n", SystemTable->ConfigurationTable[i].VendorTable);
    }
  }
  return EFI_SUCCESS;
}
```

In this code we've used `CompareGuid` function from the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/BaseMemoryLib.h:
```
/**
  Compares two GUIDs.
  This function compares Guid1 to Guid2.  If the GUIDs are identical then TRUE is returned.
  If there are any bit differences in the two GUIDs, then FALSE is returned.
  If Guid1 is NULL, then ASSERT().
  If Guid2 is NULL, then ASSERT().
  @param  Guid1       A pointer to a 128 bit GUID.
  @param  Guid2       A pointer to a 128 bit GUID.
  @retval TRUE        Guid1 and Guid2 are identical.
  @retval FALSE       Guid1 and Guid2 are not identical.
**/
BOOLEAN
EFIAPI
CompareGuid (
  IN CONST GUID  *Guid1,
  IN CONST GUID  *Guid2
  );
```

Don't forget to add:
```
[Guids]
  gEfiSmbiosTableGuid
```
to app *inf file.

Let's build our app and execute it under OVMF:
```
FS0:\> SmbiosInfo.efi
SMBIOS table is placed at 7941000
```

# Get SMBIOS tables with `dmem`

UEFI shell has a command `dmem` for memory dump:
```
FS0:\> dmem -? -b
Displays the contents of system or device memory.

DMEM [-b] [address] [size] [-MMIO]

  -b      - Displays one screen at a time.
  -MMIO   - Forces address cycles to the PCI bus.
  address - Specifies a starting address in hexadecimal format.
  size    - Specifies the number of bytes to display in hexadecimal format.

NOTES:
  1. This command displays the contents of system memory or device memory.
  2. Enter address and size in hexadecimal format.
  3. If address is not specified, the contents of the UEFI System Table
     are displayed. Otherwise, memory starting at the specified address is displayed.
  4. Size specifies the number of bytes to display. If size is not specified,
     512 bytes are displayed.
  5. If MMIO is not specified, main system memory is displayed. Otherwise,
     device memory is displayed through the use of the
     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL.

EXAMPLES:
  * To display the UEFI system table pointer entries:
    fs0:\> dmem

  * To display memory contents from 1af3088 with size of 16 bytes:
    Shell> dmem 1af3088 16

  * To display memory mapped IO contents from 1af3088 with a size of 16 bytes:
    Shell> dmem 1af3088 16 -MMIO
```

Let's use it to print 0x30 bytes from the 0x7941000 address that we count as a SMBIOS table pointer.
```
FS0:\> dmem 7941000 30
Memory Address 0000000007941000 30 Bytes
  07941000: 5F 53 4D 5F 26 1F 02 08-53 00 00 00 00 00 00 00  *_SM_&...S.......*
  07941010: 5F 44 4D 49 5F 0A 91 01-00 00 94 07 09 00 28 AF  *_DMI_.........(.*
  07941020: AF AF AF AF AF AF AF AF-AF AF AF AF AF AF AF AF  *................*
```

If you look at SMBIOS specification for SMBIOS Entry Point structure you'll see, that `_SM_` and `_DMI_` are predefined values in this structure.

![SMBIOS_entry_structure](SMBIOS_entry_structure.png?raw=true "SMBIOS_entry_structure")

You can find definition for the structure itself in edk2 under https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/SmBios.h
```
typedef struct {
  UINT8   AnchorString[4];
  UINT8   EntryPointStructureChecksum;
  UINT8   EntryPointLength;
  UINT8   MajorVersion;
  UINT8   MinorVersion;
  UINT16  MaxStructureSize;
  UINT8   EntryPointRevision;
  UINT8   FormattedArea[5];
  UINT8   IntermediateAnchorString[5];
  UINT8   IntermediateChecksum;
  UINT16  TableLength;
  UINT32  TableAddress;
  UINT16  NumberOfSmbiosStructures;
  UINT8   SmbiosBcdRevision;
} SMBIOS_TABLE_ENTRY_POINT;
```

If you calculate offsets for different fields you can parse memory dump:

![SMBIOS_entry_structure_dump](SMBIOS_entry_structure_dump.png?raw=true "SMBIOS_entry_structure_dump")

System has 9 SMBIOS structures placed from 0x07940000 to (0x07940000+0x191).

Now when we know address for SMBIOS structures, we can dump them as well.
```
FS0:\> dmem 07940000 191
Memory Address 0000000007940000 191 Bytes
  07940000: 01 1B 00 01 01 02 03 00-00 00 00 00 00 00 00 00  *................*
  07940010: 00 00 00 00 00 00 00 00-06 00 00 51 45 4D 55 00  *...........QEMU.*
  07940020: 53 74 61 6E 64 61 72 64-20 50 43 20 28 69 34 34  *Standard PC (i44*
  07940030: 30 46 58 20 2B 20 50 49-49 58 2C 20 31 39 39 36  *0FX + PIIX, 1996*
  07940040: 29 00 70 63 2D 69 34 34-30 66 78 2D 66 6F 63 61  *).pc-i440fx-foca*
  07940050: 6C 00 00 03 16 00 03 01-01 02 00 00 03 03 03 02  *l...............*
  07940060: 00 00 00 00 00 00 00 00-00 51 45 4D 55 00 70 63  *.........QEMU.pc*
  07940070: 2D 69 34 34 30 66 78 2D-66 6F 63 61 6C 00 00 04  *-i440fx-focal...*
  07940080: 2A 00 04 01 03 01 02 63-06 00 00 FD FB 8B 07 03  **......c........*
  07940090: 00 00 00 D0 07 D0 07 41-01 FF FF FF FF FF FF 00  *.......A........*
  079400A0: 00 00 01 01 01 02 00 01-00 43 50 55 20 30 00 51  *.........CPU 0.Q*
  079400B0: 45 4D 55 00 70 63 2D 69-34 34 30 66 78 2D 66 6F  *EMU.pc-i440fx-fo*
  079400C0: 63 61 6C 00 00 10 17 00-10 01 03 06 00 00 02 00  *cal.............*
  079400D0: FE FF 01 00 00 00 00 00-00 00 00 00 00 00 11 28  *...............(*
  079400E0: 00 11 00 10 FE FF FF FF-FF FF 80 00 09 00 01 00  *................*
  079400F0: 07 02 00 00 00 02 00 00-00 00 00 00 00 00 00 00  *................*
  07940100: 00 00 00 00 00 00 44 49-4D 4D 20 30 00 51 45 4D  *......DIMM 0.QEM*
  07940110: 55 00 00 13 1F 00 13 00-00 00 00 FF FF 01 00 00  *U...............*
  07940120: 10 01 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  07940130: 00 00 00 00 20 0B 00 20-00 00 00 00 00 00 00 00  *.... .. ........*
  07940140: 00 00 1A 00 00 01 02 00-E8 03 00 08 00 00 00 00  *................*
  07940150: 00 00 00 00 1C 00 00 FF-FF 00 00 45 46 49 20 44  *...........EFI D*
  07940160: 65 76 65 6C 6F 70 6D 65-6E 74 20 4B 69 74 20 49  *evelopment Kit I*
  07940170: 49 20 2F 20 4F 56 4D 46-00 30 2E 30 2E 30 00 30  *I / OVMF.0.0.0.0*
  07940180: 32 2F 30 36 2F 32 30 31-35 00 00 7F 04 FF FE 00  *2/06/2015.......*
  07940190: 00                                               *.*
```

# Use EFI_SMBIOS_PROTOCOL to parse SMBIOS data

We can use direct pointer arithmetics to parse SMBIOS tables, but that would be very tedious.

Luckily UEFI PI specification defines a `EFI_SMBIOS_PROTOCOL`, that we can use to get SMBIOS data.

```
EFI_SMBIOS_PROTOCOL

Summary:
Allows consumers to log SMBIOS data records, and enables the producer to create the SMBIOS tables for a platform.

Protocol Interface Structure:
typedef struct _EFI_SMBIOS_PROTOCOL {
 EFI_SMBIOS_ADD Add;
 EFI_SMBIOS_UPDATE_STRINGUpdateString;
 EFI_SMBIOS_REMOVE Remove;
 EFI_SMBIOS_GET_NEXT GetNext;
 UINT8 MajorVersion;
 UINT8 MinorVersion;
} EFI_SMBIOS_PROTOCOL;

Member Description:
Add		Add an SMBIOS record including the formatted area and the optional strings
		that follow the formatted area.
UpdateString 	Update a string in the SMBIOS record.
Remove		Remove an SMBIOS record.
GetNext		Discover all SMBIOS records.
MajorVersion	The major revision of the SMBIOS specification supported.
MinorVersion	The minor revision of the SMBIOS specification supported.

Description:
This protocol provides an interface to add, remove or discover SMBIOS records. The driver which
produces this protocol is responsible for creating the SMBIOS data tables and installing the pointer
to the tables in the EFI System Configuration Table.
```

Right now we are interested in SMBIOS table parsing, so we need to utilize `GetNext` function:
```
EFI_SMBIOS_PROTOCOL.GetNext()

Summary:
Allow the caller to discover all or some of the SMBIOS records.
Prototype
typedef
EFI_STATUS
(EFIAPI *EFI_SMBIOS_GET_NEXT) (
 IN CONST EFI_SMBIOS_PROTOCOL *This,
 IN OUT EFI_SMBIOS_HANDLE *SmbiosHandle,
 IN EFI_SMBIOS_TYPE *Type, OPTIONAL
 OUT EFI_SMBIOS_TABLE_HEADER **Record,
 OUT EFI_HANDLE *ProducerHandle OPTIONAL
 );

Parameters:
This		The EFI_SMBIOS_PROTOCOL instance.
SmbiosHandle	On entry, points to the previous handle of the SMBIOS record. On exit, points to the
		next SMBIOS record handle. If it is FFFEh on entry, then the first SMBIOS record
		handle will be returned. If it returns FFFEh on exit, then there are no more SMBIOS
		records.
Type		On entry, it points to the type of the next SMBIOS record to return. If NULL, it
		indicates that the next record of any type will be returned. Type is not modified by
		the this function.
Record		On exit, points to a pointer to the the SMBIOS Record consisting of the formatted area
		followed by the unformatted area. The unformatted area optionally contains text
		strings.
ProducerHandle	On exit, points to the ProducerHandle registered by Add(). If no
		ProducerHandle was passed into Add() NULL is returned. If a NULL pointer is
		passed in no data will be returned

Description
This function allows all of the SMBIOS records to be discovered. It's possible to find
only the SMBIOS records that match the optional Type argument.

Status Codes Returned:
EFI_SUCCESS 	.SMBIOS record information was successfully returned in Record.
		SmbiosHandle is the handle of the current SMBIOS record
EFI_NOT_FOUND 	The SMBIOS record with SmbiosHandle was the last available record.
```

First let's get this protocol in our app.
```
EFI_SMBIOS_PROTOCOL* SmbiosProtocol;
EFI_STATUS Status = gBS->LocateProtocol (
                &gEfiSmbiosProtocolGuid,
                NULL,
                (VOID**)&SmbiosProtocol
                );
if (EFI_ERROR (Status)) {
  return Status;
}
```
To use it we need to add include to our app https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/Smbios.h:
```
#include <Protocol/Smbios.h>
```
And add protocol guid to our *.inf file:
```
[Protocols]
  gEfiSmbiosProtocolGuid
```

Now let's try to get SMBIOS tables. We would be using `SMBIOS_HANDLE_PI_RESERVED` as `EFI_SMBIOS_HANDLE` in protocol calls. You can find explanation in https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/SmBios.h:
```
///
/// Reference SMBIOS 2.7, chapter 6.1.2.
/// The UEFI Platform Initialization Specification reserves handle number FFFEh for its
/// EFI_SMBIOS_PROTOCOL.Add() function to mean "assign an unused handle number automatically."
/// This number is not used for any other purpose by the SMBIOS specification.
///
#define SMBIOS_HANDLE_PI_RESERVED 0xFFFE
```

Also before we start writing code look at the definition of `EFI_SMBIOS_TABLE_HEADER` structure, that we would receive on `GetNext` calls.
https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/Smbios.h
```
typedef SMBIOS_STRUCTURE    EFI_SMBIOS_TABLE_HEADER;
```
https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/SmBios.h
```
///
/// The Smbios structure header.
///
typedef struct {
  SMBIOS_TYPE    Type;
  UINT8          Length;
  SMBIOS_HANDLE  Handle;
} SMBIOS_STRUCTURE;
```

Now we are ready to write some code. Write a code to print all types of Smbios tables that are present in the system:
```
EFI_SMBIOS_HANDLE SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
EFI_SMBIOS_TABLE_HEADER* Record;
Status = SmbiosProtocol->GetNext(SmbiosProtocol,
                                 &SmbiosHandle,
                                 NULL,
                                 &Record,
                                 NULL);
while (!EFI_ERROR(Status)) {
  Print (L"SMBIOS Type %d \n", Record->Type);
  Status = SmbiosProtocol->GetNext(SmbiosProtocol,
                                   &SmbiosHandle,
                                   NULL,
                                   &Record,
                                   NULL);
}
```

If you build our app and test it under OVMF you would get:
```
SMBIOS table is placed at 7941000
SMBIOS Type 1
SMBIOS Type 3
SMBIOS Type 4
SMBIOS Type 16
SMBIOS Type 17
SMBIOS Type 19
SMBIOS Type 32
SMBIOS Type 0
```

Ok, now let's write code that can parse information in these tables.

Let's start with Type 0 table. edk2 has a structure description at https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/SmBios.h:
```
///
/// BIOS Information (Type 0).
///
typedef struct {
  SMBIOS_STRUCTURE          Hdr;
  SMBIOS_TABLE_STRING       Vendor;
  SMBIOS_TABLE_STRING       BiosVersion;
  UINT16                    BiosSegment;
  SMBIOS_TABLE_STRING       BiosReleaseDate;
  UINT8                     BiosSize;
  MISC_BIOS_CHARACTERISTICS BiosCharacteristics;
  UINT8                     BIOSCharacteristicsExtensionBytes[2];
  UINT8                     SystemBiosMajorRelease;
  UINT8                     SystemBiosMinorRelease;
  UINT8                     EmbeddedControllerFirmwareMajorRelease;
  UINT8                     EmbeddedControllerFirmwareMinorRelease;
  //
  // Add for smbios 3.1.0
  //
  EXTENDED_BIOS_ROM_SIZE    ExtendedBiosSize;
} SMBIOS_TABLE_TYPE0;
```

In this structure `SMBIOS_STRUCTURE Hdr` is a mandatory field that is the same as `EFI_SMBIOS_TABLE_HEADER` that we receive from our protocol function call.

Also `SMBIOS_TABLE_STRING` is just an UINT8 value that defines a string number in an ASCII string array that is placed directly after the structure.

```
///
/// Text strings associated with a given SMBIOS structure are returned in the dmiStrucBuffer, appended directly after
/// the formatted portion of the structure. This method of returning string information eliminates the need for
/// application software to deal with pointers embedded in the SMBIOS structure. Each string is terminated with a null
/// (00h) BYTE and the set of strings is terminated with an additional null (00h) BYTE. When the formatted portion of
/// a SMBIOS structure references a string, it does so by specifying a non-zero string number within the structure's
/// string-set. For example, if a string field contains 02h, it references the second string following the formatted portion
/// of the SMBIOS structure. If a string field references no string, a null (0) is placed in that string field. If the
/// formatted portion of the structure contains string-reference fields and all the string fields are set to 0 (no string
/// references), the formatted section of the structure is followed by two null (00h) BYTES.
///
typedef UINT8 SMBIOS_TABLE_STRING;
```

So let's write a simple function that returns actual ASCII string from a `EFI_SMBIOS_TABLE_HEADER*` and a string number:
```
CHAR8* GetRecordString(EFI_SMBIOS_TABLE_HEADER* Record, UINTN number)
{
  if (!number)
    return "";

  CHAR8* String = (CHAR8*)Record + Record->Length;
  UINTN i=1;
  while (i < number) {
    String = String + AsciiStrSize(String);
    i++;
  }
  return String;
}
```
Here we've used `AsciiStrSize` function that is defined in https://github.com/tianocore/edk2/blob/master/MdePkg/Library/BaseLib/String.c file.
```
/**
  Returns the size of a Null-terminated ASCII string in bytes, including the
  Null terminator.
  This function returns the size, in bytes, of the Null-terminated ASCII string
  specified by String.
  If String is NULL, then ASSERT().
  If PcdMaximumAsciiStringLength is not zero and String contains more than
  PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
  then ASSERT().
  @param  String  A pointer to a Null-terminated ASCII string.
  @return The size of String.
**/
UINTN
EFIAPI
AsciiStrSize (
  IN      CONST CHAR8               *String
  )
```

Now we have everything to write actual parsing code in our `while` loop:

```
while (!EFI_ERROR(Status)) {
  Print (L"SMBIOS Type %d \n", Record->Type);
  switch (Record->Type) {
    case EFI_SMBIOS_TYPE_BIOS_INFORMATION: {
      SMBIOS_TABLE_TYPE0* Type0Record = (SMBIOS_TABLE_TYPE0*) Record;
      Print(L"\tVendor=%a\n", GetRecordString(Record, Type0Record->Vendor));
      Print(L"\tBiosVersion=%a\n", GetRecordString(Record, Type0Record->BiosVersion));
      Print(L"\tBiosReleaseDate=%a\n", GetRecordString(Record, Type0Record->BiosReleaseDate));
      Print(L"\tBiosSegment=0x%x\n", Type0Record->BiosSegment);
      Print(L"\tSystemBiosMajorRelease=0x%x\n", Type0Record->SystemBiosMajorRelease);
      Print(L"\tSystemBiosMinorRelease=0x%x\n", Type0Record->SystemBiosMinorRelease);
      break;
    }
    default:
      Print(L"\tTODO: Parsing for this table is not ready yet\n");
      break;
  }
  Status = SmbiosProtocol->GetNext(SmbiosProtocol,
                                   &SmbiosHandle,
                                   NULL,
                                   &Record,
                                   NULL);
}
```
To print ASCII strings here we've used `%a` format code (https://github.com/tianocore/edk/blob/master/Foundation/Library/Pei/PeiLib/Print/Print.c).


If you build and execute our app under OVMF you would get:
```
FS0:\> SmbiosInfo.efi
SMBIOS table is placed at 7941000

SMBIOS Type 1
        TODO: Parsing for this table is not ready yet
SMBIOS Type 3
        TODO: Parsing for this table is not ready yet
SMBIOS Type 4
        TODO: Parsing for this table is not ready yet
SMBIOS Type 16
        TODO: Parsing for this table is not ready yet
SMBIOS Type 17
        TODO: Parsing for this table is not ready yet
SMBIOS Type 19
        TODO: Parsing for this table is not ready yet
SMBIOS Type 32
        TODO: Parsing for this table is not ready yet
SMBIOS Type 0
        Vendor=EFI Development Kit II / OVMF
        BiosVersion=0.0.0
        BiosReleaseDate=02/06/2015
        BiosSegment=0xE800
        SystemBiosMajorRelease=0x0
        SystemBiosMinorRelease=0x0
```

If you look closely at `dmem` dump from earlier, you'll see that these are the exact strings that were actually present in memory.

We can easily to add code to parse other SMBIOS tables.

For example here is some parsing code for table type 1 structure:
```
case EFI_SMBIOS_TYPE_SYSTEM_INFORMATION: {
  SMBIOS_TABLE_TYPE1* Type1Record = (SMBIOS_TABLE_TYPE1*) Record;
  Print(L"\tManufacturer=%a\n", GetRecordString(Record, Type1Record->Manufacturer));
  Print(L"\tProductName=%a\n", GetRecordString(Record, Type1Record->ProductName));
  Print(L"\tVersion=%a\n", GetRecordString(Record, Type1Record->Version));
  Print(L"\tSerialNumber=%a\n", GetRecordString(Record, Type1Record->SerialNumber));
  Print(L"\tUUID=%g\n", Type1Record->Uuid);
  Print(L"\tWakeUpType=%d\n", Type1Record->WakeUpType);
  Print(L"\tSKUNumber=%a\n", GetRecordString(Record, Type1Record->SKUNumber));
  Print(L"\tFamily=%a\n", GetRecordString(Record, Type1Record->Family));
  break;
}
```
Structure itself is:
```
typedef struct {
  SMBIOS_STRUCTURE        Hdr;
  SMBIOS_TABLE_STRING     Manufacturer;
  SMBIOS_TABLE_STRING     ProductName;
  SMBIOS_TABLE_STRING     Version;
  SMBIOS_TABLE_STRING     SerialNumber;
  GUID                    Uuid;
  UINT8                   WakeUpType;           ///< The enumeration value from MISC_SYSTEM_WAKEUP_TYPE.
  SMBIOS_TABLE_STRING     SKUNumber;
  SMBIOS_TABLE_STRING     Family;
} SMBIOS_TABLE_TYPE1;
```

Build and execute:
```
FS0:\> SmbiosInfo.efi
SMBIOS table is placed at 7941000

SMBIOS Type 1
        Manufacturer=QEMU
        ProductName=Standard PC (i440FX + PIIX, 1996)
        Version=pc-i440fx-focal
        SerialNumber=
        UUID=00000000-0000-0000-0000-000000000000
        WakeUpType=6
        SKUNumber=
        Family=
SMBIOS Type 3
        TODO: Parsing for this table is not ready yet
SMBIOS Type 4
        TODO: Parsing for this table is not ready yet
SMBIOS Type 16
        TODO: Parsing for this table is not ready yet
SMBIOS Type 17
        TODO: Parsing for this table is not ready yet
SMBIOS Type 19
        TODO: Parsing for this table is not ready yet
SMBIOS Type 32
        TODO: Parsing for this table is not ready yet
SMBIOS Type 0
        Vendor=EFI Development Kit II / OVMF
        BiosVersion=0.0.0
        BiosReleaseDate=02/06/2015
        BiosSegment=0xE800
        SystemBiosMajorRelease=0x0
        SystemBiosMinorRelease=0x0
```

As you remember the same information that is present in these SMBIOS tables is present in the main BIOS menu:

![BIOS_menu](BIOS_menu.png?raw=true "BIOS_menu")

And in case you wonder where OVMF defines all these information for its SMBIOS structures, checkout https://github.com/tianocore/edk2/blob/master/OvmfPkg/SmbiosPlatformDxe/SmbiosPlatformDxe.c and implementation of a `EFI_SMBIOS_PROTOCOL` is placed here https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.c

# `smbiosview` command

We can use `EFI_SMBIOS_PROTOCOL` to parse SMBIOS table information, but actually if you just want to see SMBIOS information there is a better option.

UEFI shell has a `smbiosview` command that does exactly what we need.

You can checkout sources for this command here: https://github.com/tianocore/edk2/tree/master/ShellPkg/Library/UefiShellDebug1CommandsLib/SmbiosView

First checkout help for this command:
```
FS0:\> smbiosview -?
Displays SMBIOS information.

SMBIOSVIEW [-t SmbiosType]|[-h SmbiosHandle]|[-s]|[-a]

  -t            - Displays all structures of SmbiosType.
  -h            - Displays structure of SmbiosHandle.
  -s            - Displays a statistics table.
  -a            - Displays all information.
  SmbiosType    - Specifies a SMBIOS structure type.
  SmbiosHandle  - Specifies a SMBIOS structure unique 16-bit handle.

NOTES:
  1. The SmbiosType parameter supports the following types:
       0  - BIOS Information
       1  - System Information
       2  - Baseboard Information
       3  - System Enclosure
       4  - Processor Information
       5  - Memory Controller Information
       6  - Memory Module Information
       7  - Cache Information
       8  - Port Connector Information
       9  - System Slots
       10 - On Board Devices Information
       11 - OEM Strings
       12 - System Configuration Options
       13 - BIOS Language Information
       14 - Group Associations
       15 - System Event Log
       16 - Physical Memory Array
       17 - Memory Device
       18 - 32-bit Memory Error Information
       19 - Memory Array Mapped Address
       20 - Memory Device Mapped Address
       21 - Built-in Pointing Device
       22 - Portable Battery
       23 - System Reset
       24 - Hardware Security
       25 - System Power Controls
       26 - Voltage Probe
       27 - Cooling Device
       28 - Temperature Probe
       29 - Electrical Current Probe
       30 - Out-Of-Band Remote Access
       31 - Boot Integrity Services (BIS) Entry Point
       32 - System Boot Information
       33 - 64-Bit Memory Error Information
       34 - Management Device
       35 - Management Device Component
       36 - Management Device Threshold Data
       37 - Memory Channel
       38 - IPMI Device Information
       39 - System Power Supply
       40 - Additional Information
       41 - Onboard Devices Extended Information
       42 - Management Controller Host Interface
       43 - TPM Device
       44 - Processor Additional Information
  2. Enter the SmbiosHandle parameter in hexadecimal format.
     Do not use the '0x' prefix format for hexadecimal values.
  3. Internal commands:
       :q --------  quit smbiosview
       :0 --------  Change smbiosview display NONE info
       :1 --------  Change smbiosview display OUTLINE info
       :2 --------  Change smbiosview display NORMAL info
       :3 --------  Change smbiosview display DETAIL info
       /? --------  Show help
```

Try to dump one of the structures that we've tried to parse manually:
```
FS0:\> smbiosview -t 0
SMBIOS Entry Point Structure:
Anchor String:        _SM_
EPS Checksum:         0x26
Entry Point Len:      31
Version:              2.8
Number of Structures: 9
Max Struct size:      83
Table Address:        0x7940000
Table Length:         401
Entry Point revision: 0x0
SMBIOS BCD Revision:  0x28
Inter Anchor:         _DMI_
Inter Checksum:       0xA
Formatted Area:
  00000000: 00 00 00 00 00                                   *.....*

=========================================================
Query Structure, conditions are:
QueryType   = 0
QueryHandle = Random
ShowType    = SHOW_DETAIL


=========================================================
Type=0, Handle=0x0
Dump Structure as:
Index=7,Length=0x4A,Addr=0x7940141
00000000: 00 1A 00 00 01 02 00 E8-03 00 08 00 00 00 00 00  *................*
FS0:\> 0: 00 00 00 1C 00 00 FF FF-00 00 45 46 49 20 44 65  *..........EFI De*
00000020: 76 65 6C 6F 70 6D 65 6E-74 20 4B 69 74 20 49 49  *velopment Kit II*
00000030: 20 2F 20 4F 56 4D 46 00-30 2E 30 2E 30 00 30 32  * / OVMF.0.0.0.02*
00000040: 2F 30 36 2F 32 30 31 35-00 00                    */06/2015..*
Structure Type: BIOS Information
Format part Len : 26
Structure Handle: 0
Vendor: EFI Development Kit II / OVMF
BiosVersion: 0.0.0
BiosSegment: 0xE800
BiosReleaseDate: 02/06/2015
BiosSize:  64 KB
BIOS Characteristics:
BIOS Characteristics Not Supported
 Bits 32:47 are reserved for BIOS Vendor
 Bits 48:64 are reserved for System Vendor
BIOS Characteristics Extension Byte1:
BIOS Characteristics Extension Byte2:
Enable Targeted Content Distribution
UEFI Specification is supported
The SMBIOS table describes a virtual machine
 Bits 5:7 are reserved for future assignment
SystemBiosMajorRelease: 0
SystemBiosMinorRelease: 0
EmbeddedControllerFirmwareMajorRelease: 255
EmbeddedControllerFirmwareMinorRelease: 255
```
As you can see it is all the same info that we've received using `EFI_SMBIOS_PROTOCOL`.

You can use this command to see what is inside all of the SMBIOS structures using:
```
FS0:\> smbiosview -b
...
```

The output is too big to paste here, so check it out yourself!
