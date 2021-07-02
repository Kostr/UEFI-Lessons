The latest ACPI specification can be found under UEFI specifications page https://uefi.org/specifications

The current latest specification is "ACPI Specification Version 6.4 (released January 2021)" (https://uefi.org/specs/ACPI/6.4/)


Use the same tactic we used for SMBIOS tables:
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
    if (CompareGuid(&(SystemTable->ConfigurationTable[i].VendorGuid), &gEfiAcpi20TableGuid)) {
      Print(L"ACPI table is placed at %p\n\n", SystemTable->ConfigurationTable[i].VendorTable);
    }
  }
  return EFI_SUCCESS;
}
```

dmem peak inside memory:
```
FS0:\> AcpiInfo.efi
ACPI table is placed at 7B7E014

FS0:\> dmem 7B7E014 30
Memory Address 0000000007B7E014 30 Bytes
  07B7E014: 52 53 44 20 50 54 52 20-4E 42 4F 43 48 53 20 02  *RSD PTR NBOCHS .*
  07B7E024: 74 D0 B7 07 24 00 00 00-E8 D0 B7 07 00 00 00 00  *t...$...........*
  07B7E034: 66 00 00 00 AF AF AF AF-AF AF AF AF AF AF AF AF  *f...............*
FS0:\>
```

The signature `RSP PTR` stands for Root System Description Pointer (RSDP) Structure (https://uefi.org/specs/ACPI/6.4/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html#root-system-description-pointer-rsdp-structure).

It contains addresses for RSDT and XSDT tables. If you calculate offsets, you'll get these addresses from our memory dump:
```
XSDT=0x07B7D0E8
RSDT=0x07B7D074
```
These tables in turn would cointain pointers to other ACPI tables that actualy contain data useful to OS.

According to the spec "platforms provide the RSDT to enable compatibility with ACPI 1.0 operating systems. The XSDT, described in the next section, supersedes RSDT functionality". If you peak these addresses with `dmem` table contents would be pretty much the same except table signatures.


Ok, it's time to write some code. ACPI structures are defined in following header files:
```
$ ls -1 MdePkg/Include/IndustryStandard/Acpi*
MdePkg/Include/IndustryStandard/Acpi.h
MdePkg/Include/IndustryStandard/Acpi10.h
MdePkg/Include/IndustryStandard/Acpi20.h
MdePkg/Include/IndustryStandard/Acpi30.h
MdePkg/Include/IndustryStandard/Acpi40.h
MdePkg/Include/IndustryStandard/Acpi50.h
MdePkg/Include/IndustryStandard/Acpi51.h
MdePkg/Include/IndustryStandard/Acpi60.h
MdePkg/Include/IndustryStandard/Acpi61.h
MdePkg/Include/IndustryStandard/Acpi62.h
MdePkg/Include/IndustryStandard/Acpi63.h
MdePkg/Include/IndustryStandard/AcpiAml.h
```

Keep in mind that headers for latter standards include headers for earlier standards in itself.
```
Acpi.h > Acpi63.h > Acpi62.h > ... > Acpi10.h > AcpiAml.h
```

Let's look at RSDP structure definition at the most latest ACPI standard header file
https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Acpi63.h
```
///
/// Root System Description Pointer Structure
///
typedef struct {
  UINT64  Signature;
  UINT8   Checksum;
  UINT8   OemId[6];
  UINT8   Revision;
  UINT32  RsdtAddress;
  UINT32  Length;
  UINT64  XsdtAddress;
  UINT8   ExtendedChecksum;
  UINT8   Reserved[3];
} EFI_ACPI_6_3_ROOT_SYSTEM_DESCRIPTION_POINTER;
```

We can use it to print addresses of RSDT/XSDT tables.
```
EFI_ACPI_6_3_ROOT_SYSTEM_DESCRIPTION_POINTER* RSDP = NULL;

for (UINTN i=0; i<SystemTable->NumberOfTableEntries; i++) {
  if (CompareGuid(&(SystemTable->ConfigurationTable[i].VendorGuid), &gEfiAcpi20TableGuid)) {
    Print(L"RSDP table is placed at %p\n\n", SystemTable->ConfigurationTable[i].VendorTable);
    RSDP = SystemTable->ConfigurationTable[i].VendorTable;
  }
}

if (!RSDP) {
  Print(L"No ACPI2.0 table was found in the system\n");
  return EFI_SUCCESS;
}

if (((CHAR8)((RSDP->Signature >>  0) & 0xFF) != 'R') ||
    ((CHAR8)((RSDP->Signature >>  8) & 0xFF) != 'S') ||
    ((CHAR8)((RSDP->Signature >> 16) & 0xFF) != 'D') ||
    ((CHAR8)((RSDP->Signature >> 24) & 0xFF) != ' ') ||
    ((CHAR8)((RSDP->Signature >> 32) & 0xFF) != 'P') ||
    ((CHAR8)((RSDP->Signature >> 40) & 0xFF) != 'T') ||
    ((CHAR8)((RSDP->Signature >> 48) & 0xFF) != 'R') ||
    ((CHAR8)((RSDP->Signature >> 56) & 0xFF) != ' ')) {
  Print(L"Error! RSDP signature is not valid!\n");
  return EFI_SUCCESS;
}

Print(L"System description tables:\n");
Print(L"\tRSDT table is placed at address %p\n", RSDP->RsdtAddress);
Print(L"\tXSDT table is placed at address %p\n", RSDP->XsdtAddress);
Print(L"\n");
```

In the same file (https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Acpi63.h) you can find description for XSDT structure:
```
//
// Extended System Description Table
// No definition needed as it is a common description table header, the same with
// EFI_ACPI_DESCRIPTION_HEADER, followed by a variable number of UINT64 table pointers.
//
```

The definition for `EFI_ACPI_DESCRIPTION_HEADER` can be found here https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Acpi10.h:
```
#pragma pack(1)
///
/// The common ACPI description table header.  This structure prefaces most ACPI tables.
///
typedef struct {
  UINT32  Signature;
  UINT32  Length;
  UINT8   Revision;
  UINT8   Checksum;
  UINT8   OemId[6];
  UINT64  OemTableId;
  UINT32  OemRevision;
  UINT32  CreatorId;
  UINT32  CreatorRevision;
} EFI_ACPI_DESCRIPTION_HEADER;
#pragma pack()
```

Let's check information about other ACPI tables that are present in the system:
```
EFI_ACPI_DESCRIPTION_HEADER* XSDT = (EFI_ACPI_DESCRIPTION_HEADER*)RSDP->XsdtAddress;
if (((CHAR8)((XSDT->Signature >>  0) & 0xFF) != 'X') ||
    ((CHAR8)((XSDT->Signature >>  8) & 0xFF) != 'S') ||
    ((CHAR8)((XSDT->Signature >> 16) & 0xFF) != 'D') ||
    ((CHAR8)((XSDT->Signature >> 24) & 0xFF) != 'T')) {
  Print(L"Error! XSDT signature is not valid!\n");
  return EFI_SUCCESS;
}

Print(L"Main ACPI tables:\n");
UINT64 offset = sizeof(EFI_ACPI_DESCRIPTION_HEADER);
while (offset < XSDT->Length) {
  UINT64* table_address = (UINT64*)((UINT8*)XSDT + offset);
  EFI_ACPI_6_3_COMMON_HEADER* table = (EFI_ACPI_6_3_COMMON_HEADER*)(*table_address);
  Print(L"\t%c%c%c%c table is placed at address %p with length 0x%x\n",
                                           (CHAR8)((table->Signature>> 0)&0xFF),
                                           (CHAR8)((table->Signature>> 8)&0xFF),
                                           (CHAR8)((table->Signature>>16)&0xFF),
                                           (CHAR8)((table->Signature>>24)&0xFF),
                                           table,
                                           table->Length);
  offset += sizeof(UINT64);
}
```

If you build our app and execute it under OVMF now you would get:
```
FS0:\> AcpiInfo.efi
RSDP table is placed at 7B7E014

System description tables:
        RSDT table is placed at address 7B7D074
        XSDT table is placed at address 7B7D0E8

Main ACPI tables:
        FACP table is placed at address 7B7A000 with length 0x74
        APIC table is placed at address 7B79000 with length 0x78
        HPET table is placed at address 7B78000 with length 0x38
        BGRT table is placed at address 7B77000 with length 0x38
```

Pretty neat, our system has 4 ACPI data tables:
- Fixed ACPI Description Table (`FACP`) - https://uefi.org/specs/ACPI/6.4/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html#fixed-acpi-description-table-fadt
- Multiple APIC Description Table (`MADT`) - https://uefi.org/specs/ACPI/6.4/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html#multiple-apic-description-table-madt
- IA-PC High Precision Event Timer Table (`HPET`) - http://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/software-developers-hpet-spec-1-0a.pdf - This one is not present in ACPI spec, but in a separate document from the page https://uefi.org/acpi
- Boot Graphics Resource Table (`BGRT`) - https://uefi.org/specs/ACPI/6.4/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html#boot-graphics-resource-table-bgrt


