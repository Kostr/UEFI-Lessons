As you remember the entry point for our UEFI programs is:

```
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
```

The passed structure `EFI_SYSTEM_TABLE` looks like this according to UEFI specification:

```
typedef struct {
 EFI_TABLE_HEADER Hdr;
 CHAR16 *FirmwareVendor;
 UINT32 FirmwareRevision;
 EFI_HANDLE ConsoleInHandle;
 EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
 EFI_HANDLE ConsoleOutHandle;
 EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
 EFI_HANDLE StandardErrorHandle;
 EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
 EFI_RUNTIME_SERVICES *RuntimeServices;
 EFI_BOOT_SERVICES *BootServices;
 UINTN NumberOfTableEntries;
 EFI_CONFIGURATION_TABLE *ConfigurationTable;
} EFI_SYSTEM_TABLE;
```

We've already used most of the fields of this structure. Now it is time to peek inside these fields:
```
UINTN NumberOfTableEntries;
EFI_CONFIGURATION_TABLE *ConfigurationTable
```
According to the UEFI spec:
```
NumberOfTableEntries	The number of system configuration tables in the buffer ConfigurationTable.
ConfigurationTable	A pointer to the system configuration tables. The number of entries in the table is NumberOfTableEntries.
```

As for EFI_CONFIGURATION_TABLE type:
```
EFI_CONFIGURATION_TABLE

Summary:
Contains a set of GUID/pointer pairs comprised of the ConfigurationTable field in the EFI System
Table.
Related Definitions
typedef struct{
 EFI_GUID VendorGuid;
 VOID *VendorTable;
} EFI_CONFIGURATION_TABLE;

```

Let's create a simple program `ShowTables` to look at tables available at OVMF:
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
  for (UINTN i=0; i<SystemTable->NumberOfTableEntries; i++) {
    Print(L"%g, %p\n", SystemTable->ConfigurationTable[i].VendorGuid,
                       SystemTable->ConfigurationTable[i].VendorTable);
  }
  return EFI_SUCCESS;
}
```

Build and execute it under OVMF:
```
FS0:\> ShowTables.efi
EE4E5898-3914-4259-9D6E-DC7BD79403CF, 78EDF98
05AD34BA-6F02-4214-952E-4DA0398E2BB9, 7ED2AC0
7739F24C-93D7-11D4-9A3A-0090273FC14D, 78EA018
4C19049F-4137-4DD3-9C10-8B97A83FFDFA, 7ED3AA0
49152E77-1ADA-4764-B7A2-7AFEFED95E8B, 7ED5F10
060CC026-4C0D-4DDA-8F41-595FEF00A502, 7942018
EB9D2D31-2D88-11D3-9A16-0090273FC14D, 7941000
EB9D2D30-2D88-11D3-9A16-0090273FC14D, 7B7E000
8868E871-E4F1-11D3-BC22-0080C73C8881, 7B7E014
DCFA911D-26EB-469F-A220-38B7DC461220, 6E86018
```

_______

Let's search edk2 codebase for GUIDs:
```
EE4E5898-3914-4259-9D6E-DC7BD79403CF, 78EDF98
```
`gLzmaCustomDecompressGuid`

LZMA_CUSTOM_DECOMPRESS_GUID

https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Include/Guid/LzmaDecompress.h

GUID indicates the LZMA custom compress/decompress algorithm.  
The Global ID used to identify a section of an FFS file of type EFI_SECTION_GUID_DEFINED, whose contents have been compressed using LZMA. 
```
05AD34BA-6F02-4214-952E-4DA0398E2BB9, 7ED2AC0
```
`gEfiDxeServicesTableGuid`

DXE_SERVICES_TABLE_GUID 

https://github.com/tianocore/edk2/tree/master/MdePkg/Include/Guid/DxeServices.h (DXE Services Table)

```
7739F24C-93D7-11D4-9A3A-0090273FC14D, 78EA018
```
`gEfiHobListGuid`

https://github.com/tianocore/edk2/tree/master/MdePkg/Include/Guid/HobList.h

HOB List passed from PEI to DXE

```
4C19049F-4137-4DD3-9C10-8B97A83FFDFA, 7ED3AA0
```
`gEfiMemoryTypeInformationGuid` 

https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Include/Guid/MemoryTypeInformation.h

The memory type information HOB

```
49152E77-1ADA-4764-B7A2-7AFEFED95E8B, 7ED5F10
```

`gEfiDebugImageInfoTableGuid`

https://github.com/tianocore/edk2/tree/master/MdePkg/Include/Guid/Debug ImageInfoTable.h

Debug Image Info Table

```
060CC026-4C0D-4DDA-8F41-595FEF00A502, 7942018
```
`gMemoryStatusCodeRecordGuid`

https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Include/Guid/MemoryStatusCodeRecord.h

Status code records HOB that originate from the PEI status code

```
EB9D2D31-2D88-11D3-9A16-0090273FC14D, 7941000
```
`gEfiSmbiosTableGuid` 

https://github.com/tianocore/edk2/tree/master/MdePkg/Include/Guid/SmBios.h

SMBIOS tables

```
EB9D2D30-2D88-11D3-9A16-0090273FC14D, 7B7E000
```

`gEfiAcpi10TableGuid` 

ACPI_10_TABLE_GUID     ACPI_TABLE_GUID

https://github.com/tianocore/edk2/tree/master/MdePkg/Include/Guid/Acpi.h

ACPI tables for old specifications

```
8868E871-E4F1-11D3-BC22-0080C73C8881, 7B7E014
```
gEfiAcpiTableGuid/gEfiAcpi20TableGuid

EFI_ACPI_20_TABLE_GUID EFI_ACPI_TABLE_GUID

https://github.com/tianocore/edk2/tree/master/MdePkg/Include/Guid/Acpi.h

ACPI tables for modern specifications

```
DCFA911D-26EB-469F-A220-38B7DC461220, 6E77018
```

gEfiMemoryAttributesTableGuid

https://github.com/tianocore/edk2/tree/master/MdePkg/Include/Guid/MemoryAttributesTable.h

UEFI Memory Attributes Table [Defined in UEFI spec]


