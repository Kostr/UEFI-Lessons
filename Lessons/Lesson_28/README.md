The latest ACPI specification can be found under UEFI specifications page https://uefi.org/specifications

The current latest specification is "ACPI Specification Version 6.4 (released January 2021)" (https://uefi.org/specs/ACPI/6.4/)


Use the same tactic we used for SMBIOS tables to print ACPI entry point table address:
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

Use `dmem` to peak inside ACPI table memory:
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

The signature `RSP PTR` stands for `Root System Description Pointer (RSDP) Structure` (https://uefi.org/specs/ACPI/6.4/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html#root-system-description-pointer-rsdp-structure).

It contains addresses for `RSDT` and `XSDT` tables. If you calculate offsets, you'll get these addresses from our memory dump:
```
XSDT=0x07B7D0E8
RSDT=0x07B7D074
```
These tables in turn would cointain pointers to other ACPI tables that actualy contain data useful to OS.

According to the spec "platforms provide the RSDT to enable compatibility with ACPI 1.0 operating systems. The XSDT supersedes RSDT functionality". So if you peak these addresses with `dmem`, table contents would be pretty much the same except table signatures. Therefore in our app code we would be parsing XSDT table data.

Ok, it's time to write some code. ACPI structures are defined in the following header files:
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

There is one more thing that we need to check. Some ACPI tables can contatin pointers to another ACPI tables. For example Fixed ACPI Description Table (`FADT`) can contain pointers to `DSDT` and `FACS` tables.

You can check `FADT` description in ACPI specification (https://uefi.org/specs/ACPI/6.4/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html#fixed-acpi-description-table-fadt).

In edk2 there is a structure for `FADT` in the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Acpi63.h file:

`EFI_ACPI_6_3_FIXED_ACPI_DESCRIPTION_TABtruct {
  EFI_ACPI_DESCRIPTION_HEADER             Header;
  UINT32                                  FirmwareCtrl;
  UINT32                                  Dsdt;
  ...
} EFI_ACPI_6_3_FIXED_ACPI_DESCRIPTION_TABLE;

`FirmwareCtrl` field contains pointer to the `FACS` table and `Dsdt` field contains pointer to the `DSDT` table.

Let's write a `CheckSubtables` function that can check if the ACPI table is FADT and if it is look for its subtables: 
```
VOID CheckSubtables(EFI_ACPI_6_3_COMMON_HEADER* table)
{
  if (((CHAR8)((table->Signature >>  0) & 0xFF) == 'F') &&
      ((CHAR8)((table->Signature >>  8) & 0xFF) == 'A') &&
      ((CHAR8)((table->Signature >> 16) & 0xFF) == 'C') &&
      ((CHAR8)((table->Signature >> 24) & 0xFF) == 'P')) {
    EFI_ACPI_6_3_FIXED_ACPI_DESCRIPTION_TABLE* FADT = (EFI_ACPI_6_3_FIXED_ACPI_DESCRIPTION_TABLE*)table;

    EFI_ACPI_6_3_COMMON_HEADER* DSDT = (EFI_ACPI_6_3_COMMON_HEADER*)(UINT64)(FADT->Dsdt);
    if (((CHAR8)((DSDT->Signature >>  0) & 0xFF) == 'D') &&
        ((CHAR8)((DSDT->Signature >>  8) & 0xFF) == 'S') &&
        ((CHAR8)((DSDT->Signature >> 16) & 0xFF) == 'D') &&
        ((CHAR8)((DSDT->Signature >> 24) & 0xFF) == 'T')) {
      Print(L"\tDSDT table is placed at address %p with length 0x%x\n", DSDT, DSDT->Length);
    } else {
      Print(L"\tError! DSDT signature is not valid!\n");
    }

    EFI_ACPI_6_3_COMMON_HEADER* FACS = (EFI_ACPI_6_3_COMMON_HEADER*)(UINT64)(FADT->FirmwareCtrl);
    if (((CHAR8)((FACS->Signature >>  0) & 0xFF) == 'F') &&
        ((CHAR8)((FACS->Signature >>  8) & 0xFF) == 'A') &&
        ((CHAR8)((FACS->Signature >> 16) & 0xFF) == 'C') &&
        ((CHAR8)((FACS->Signature >> 24) & 0xFF) == 'S')) {
      Print(L"\tFACS table is placed at address %p with length 0x%x\n", FACS, FACS->Length);
    } else {
      Print(L"\tError! FACS signature is not valid!\n");
    }
  }
}
```

Call this funtion in our while loop right after the `Print` statement:
```
CheckSubtables(table);
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
        DSDT table is placed at address 7B7B000 with length 0x140B
        FACS table is placed at address 7BDD000 with length 0x40
        APIC table is placed at address 7B79000 with length 0x78
        HPET table is placed at address 7B78000 with length 0x38
        BGRT table is placed at address 7B77000 with length 0x38
```

Pretty neat, our system has 4 ACPI data tables:
- Fixed ACPI Description Table (`FACP`) - https://uefi.org/specs/ACPI/6.4/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html#fixed-acpi-description-table-fadt
- Differentiated System Description Table (`DSDT`) - https://uefi.org/specs/ACPI/6.4/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html#differentiated-system-description-table-dsdt
- Firmware ACPI Control Structure (`FACS`) - https://uefi.org/specs/ACPI/6.4/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html#firmware-acpi-control-structure-facs
- Multiple APIC Description Table (`MADT`) - https://uefi.org/specs/ACPI/6.4/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html#multiple-apic-description-table-madt
- IA-PC High Precision Event Timer Table (`HPET`) - http://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/software-developers-hpet-spec-1-0a.pdf - This one is not present in ACPI spec, but in a separate document from the page https://uefi.org/acpi
- Boot Graphics Resource Table (`BGRT`) - https://uefi.org/specs/ACPI/6.4/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html#boot-graphics-resource-table-bgrt


Keep in mind that as with SMBIOS tables we could use a protocol to get the same data. `GetAcpiTable()` function of a `EFI_ACPI_SDT_PROTOCOL` can help to get the same information. This protocol also is defined by UEFI PI specification.

In edk2 it is defined under https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/AcpiSystemDescriptionTable.h

# Use `EFI_SHELL_PROTOCOL` to save table data

Now let's try to save ACPI tables from memory to files.

To do this we can utilize `EFI_SHELL_PROTOCOL` that is defined in UEFI Shell specification (https://uefi.org/sites/default/files/resources/UEFI_Shell_2_2.pdf). It has many functions for File I/O.

The necessary header in edk2 is https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/Shell.h
```
typedef struct _EFI_SHELL_PROTOCOL {
  EFI_SHELL_EXECUTE                         Execute;
  EFI_SHELL_GET_ENV                         GetEnv;
  EFI_SHELL_SET_ENV                         SetEnv;
  EFI_SHELL_GET_ALIAS                       GetAlias;
  EFI_SHELL_SET_ALIAS                       SetAlias;
  EFI_SHELL_GET_HELP_TEXT                   GetHelpText;
  EFI_SHELL_GET_DEVICE_PATH_FROM_MAP        GetDevicePathFromMap;
  EFI_SHELL_GET_MAP_FROM_DEVICE_PATH        GetMapFromDevicePath;
  EFI_SHELL_GET_DEVICE_PATH_FROM_FILE_PATH  GetDevicePathFromFilePath;
  EFI_SHELL_GET_FILE_PATH_FROM_DEVICE_PATH  GetFilePathFromDevicePath;
  EFI_SHELL_SET_MAP                         SetMap;
  EFI_SHELL_GET_CUR_DIR                     GetCurDir;
  EFI_SHELL_SET_CUR_DIR                     SetCurDir;
  EFI_SHELL_OPEN_FILE_LIST                  OpenFileList;
  EFI_SHELL_FREE_FILE_LIST                  FreeFileList;
  EFI_SHELL_REMOVE_DUP_IN_FILE_LIST         RemoveDupInFileList;
  EFI_SHELL_BATCH_IS_ACTIVE                 BatchIsActive;
  EFI_SHELL_IS_ROOT_SHELL                   IsRootShell;
  EFI_SHELL_ENABLE_PAGE_BREAK               EnablePageBreak;
  EFI_SHELL_DISABLE_PAGE_BREAK              DisablePageBreak;
  EFI_SHELL_GET_PAGE_BREAK                  GetPageBreak;
  EFI_SHELL_GET_DEVICE_NAME                 GetDeviceName;
  EFI_SHELL_GET_FILE_INFO                   GetFileInfo;
  EFI_SHELL_SET_FILE_INFO                   SetFileInfo;
  EFI_SHELL_OPEN_FILE_BY_NAME               OpenFileByName;
  EFI_SHELL_CLOSE_FILE                      CloseFile;
  EFI_SHELL_CREATE_FILE                     CreateFile;
  EFI_SHELL_READ_FILE                       ReadFile;
  EFI_SHELL_WRITE_FILE                      WriteFile;
  EFI_SHELL_DELETE_FILE                     DeleteFile;
  EFI_SHELL_DELETE_FILE_BY_NAME             DeleteFileByName;
  EFI_SHELL_GET_FILE_POSITION               GetFilePosition;
  EFI_SHELL_SET_FILE_POSITION               SetFilePosition;
  EFI_SHELL_FLUSH_FILE                      FlushFile;
  EFI_SHELL_FIND_FILES                      FindFiles;
  EFI_SHELL_FIND_FILES_IN_DIR               FindFilesInDir;
  EFI_SHELL_GET_FILE_SIZE                   GetFileSize;
  EFI_SHELL_OPEN_ROOT                       OpenRoot;
  EFI_SHELL_OPEN_ROOT_BY_HANDLE             OpenRootByHandle;
  EFI_EVENT                                 ExecutionBreak;
  UINT32                                    MajorVersion;
  UINT32                                    MinorVersion;
  // Added for Shell 2.1
  EFI_SHELL_REGISTER_GUID_NAME              RegisterGuidName;
  EFI_SHELL_GET_GUID_NAME                   GetGuidName;
  EFI_SHELL_GET_GUID_FROM_NAME              GetGuidFromName;
  EFI_SHELL_GET_ENV_EX                      GetEnvEx;
} EFI_SHELL_PROTOCOL;
```

We will use 3 functions from this protocol `OpenFileByName`/`WriteFile`/`CloseFile`:
```
EFI_SHELL_PROTOCOL.OpenFileByName()

Summary:
Opens a file or a directory by file name.

Prototype:
typdef
EFI_STATUS
(EFIAPI *EFI_SHELL_OPEN_FILE_BY_NAME) (
 IN CONST CHAR16 *FileName,
 OUT SHELL_FILE_HANDLE *FileHandle,
 IN UINT64 OpenMode
 );

Parameters:
FileName	Points to the null-terminated UCS-2 encoded file name.
FileHandle	On return, points to the file handle.
OpenMode	File open mode.

Description:
This function opens the specified file in the specified OpenMode and returns a file handle.
```
```
EFI_SHELL_PROTOCOL.WriteFile()

Summary:
Writes data to the file.

Prototype:
typedef
EFI_STATUS
(EFIAPI EFI_SHELL_WRITE_FILE)(
 IN SHELL_FILE_HANDLE FileHandle,
 IN OUT UINTN *BufferSize,
 OUT VOID *Buffer
 );

Parameters:
FileHandle 	The opened file handle for writing.
BufferSize	On input, size of Buffer.
Buffer		The buffer in which data to write.

Description:
This function writes the specified number of bytes to the file at the current file position. The current file position is advanced the actual number of bytes
written, which is returned in BufferSize. Partial writes only occur when there has been a data error during the write attempt (such as “volume space full”).
The file automatically grows to hold the data, if required.
```
```
EFI_SHELL_PROTOCOL.CloseFile()

Summary:
Closes the file handle.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_CLOSE_FILE)(
 IN SHELL_FILE_HANDLE FileHandle
 );

Parameters:
FileHandle	The file handle to be closed
Description	This function closes a specified file handle. All “dirty” cached file data is flushed
		to the device, and the file is closed. In all cases, the handle is closed.

```

Now let's start coding. Add necessary include to our *.c file:
```
#include <Protocol/Shell.h>
```
And necessary protocol guid to our *.inf file:
```
[Protocols]
  gEfiShellProtocolGuid
```

In our program we need to acquire `EFI_SHELL_PROTOCOL`, this can be done via `LocateProtocol` function from the BootServices:
```
EFI_SHELL_PROTOCOL* ShellProtocol;
EFI_STATUS Status = gBS->LocateProtocol(
  &gEfiShellProtocolGuid,
  NULL,
  (VOID **)&ShellProtocol
);

if (EFI_ERROR(Status)) {
  Print(L"Can't open EFI_SHELL_PROTOCOL: %r\n", Status);
  return EFI_SUCCESS;
}
```

We would hide all save file functionality behind our custom `SaveACPITable` function. 
```
EFI_STATUS SaveACPITable(UINT32 Signature,  // table signature
                         VOID* addr,        // table address
                         UINTN size)        // table size
```
With it our main while loop would look like this:
```
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

  SaveACPITable(table->Signature, table, table->Length);

  CheckSubtables(table);

  offset += sizeof(UINT64);
}
```
Also don't forget to add it to our `CheckSubtables` function to save `DSDT` and `FACS` tables as well.

As we would be using `EFI_SHELL_PROTOCOL* ShellProtocol` in every call of our `SaveACPITable` function we can either pass it everywhere as a parameter, or move `ShellProtocol` to global variables. Let's use the second approach in our small program.

Now it time to write this `SaveACPITable` function. It would save ACPI table data to the file "<signature>.aml". We use `.aml` extension for our files because in ACPI language source files usually have *.asl/*.dsl extension (ACPI Source Language), and compiled files have *.aml extension (ACPI Machine Language):
```
EFI_STATUS SaveACPITable(UINT32 Signature, VOID* addr, UINTN size) {
  CHAR16 TableName[5];
  TableName[0] = (CHAR16)((Signature>> 0)&0xFF);
  TableName[1] = (CHAR16)((Signature>> 8)&0xFF);
  TableName[2] = (CHAR16)((Signature>>16)&0xFF);
  TableName[3] = (CHAR16)((Signature>>24)&0xFF);
  TableName[4] = 0;

  CHAR16 FileName[9] = {0};
  StrCpyS(FileName, 9, TableName);
  StrCatS(FileName, 9, L".aml");
  SHELL_FILE_HANDLE FileHandle;
  EFI_STATUS Status = ShellProtocol->OpenFileByName(FileName,
                                                    &FileHandle,
                                                    EFI_FILE_MODE_CREATE |
                                                    EFI_FILE_MODE_WRITE |
                                                    EFI_FILE_MODE_READ);
  if (!EFI_ERROR(Status)) {
    Status = ShellProtocol->WriteFile(FileHandle, &size, addr);
    if (EFI_ERROR(Status)) {
      Print(L"Error in WriteFile: %r\n", Status);
    }
    Status = ShellProtocol->CloseFile(FileHandle);
    if (EFI_ERROR(Status)) {
      Print(L"Error in CloseFile: %r\n", Status);
    }
  } else {
    Print(L"Error in OpenFileByName: %r\n", Status);
  }
  return Status;
}
```

To create a string with a file name we use `StrCatS` and `StrCpyS` functions. They are safe versions of string concatention/string copy functions similar to their C++ analogs `strcat_s`/`strcpy_s`. You can check out them in a library https://github.com/tianocore/edk2/blob/master/MdePkg/Library/BaseLib/SafeString.c

With a help of `EFI_SHELL_PROTOCOL` file operation functions writing data to a file is pretty similar to standard system programming. We open handle, write data to it, and finally close handle.

If you build our app and execute it under OVMF you would get 4 files in our `UEFI_disk` shared folder:
```
$ ls -1 ~/UEFI_disk/*.aml
/home/kostr/UEFI_disk/apic.aml
/home/kostr/UEFI_disk/bgrt.aml
/home/kostr/UEFI_disk/dsdt.aml
/home/kostr/UEFI_disk/facp.aml
/home/kostr/UEFI_disk/facs.aml
/home/kostr/UEFI_disk/hpet.aml
```

You can use `iasl` compiler to disassemle ACPI table data:
```
$ iasl -d ~/UEFI_disk/*.aml

Intel ACPI Component Architecture
ASL+ Optimizing Compiler/Disassembler version 20190509
Copyright (c) 2000 - 2019 Intel Corporation

File appears to be binary: found 81 non-ASCII characters, disassembling
Binary file appears to be a valid ACPI table, disassembling
Input file /home/kostr/UEFI_disk/apic.aml, Length 0x78 (120) bytes
ACPI: APIC 0x0000000000000000 000078 (v01 BOCHS  BXPCAPIC 00000001 BXPC 00000001)
Acpi Data Table [APIC] decoded
Formatted output:  /home/kostr/UEFI_disk/apic.dsl - 4939 bytes
File appears to be binary: found 31 non-ASCII characters, disassembling
Binary file appears to be a valid ACPI table, disassembling
Input file /home/kostr/UEFI_disk/bgrt.aml, Length 0x38 (56) bytes
ACPI: BGRT 0x0000000000000000 000038 (v01 INTEL  EDK2     00000002      01000013)
Acpi Data Table [BGRT] decoded
Formatted output:  /home/kostr/UEFI_disk/bgrt.dsl - 1632 bytes
File appears to be binary: found 1630 non-ASCII characters, disassembling
Binary file appears to be a valid ACPI table, disassembling
Input file /home/kostr/UEFI_disk/dsdt.aml, Length 0x140B (5131) bytes
ACPI: DSDT 0x0000000000000000 00140B (v01 BOCHS  BXPCDSDT 00000001 BXPC 00000001)
Pass 1 parse of [DSDT]
Pass 2 parse of [DSDT]
Parsing Deferred Opcodes (Methods/Buffers/Packages/Regions)

Parsing completed
Disassembly completed
ASL Output:    /home/kostr/UEFI_disk/dsdt.dsl - 43444 bytes
File appears to be binary: found 91 non-ASCII characters, disassembling
Binary file appears to be a valid ACPI table, disassembling
Input file /home/kostr/UEFI_disk/facp.aml, Length 0x74 (116) bytes
ACPI: FACP 0x0000000000000000 000074 (v01 BOCHS  BXPCFACP 00000001 BXPC 00000001)
Acpi Data Table [FACP] decoded
Formatted output:  /home/kostr/UEFI_disk/facp.dsl - 4896 bytes
File appears to be binary: found 59 non-ASCII characters, disassembling
Binary file appears to be a valid ACPI table, disassembling
Input file /home/kostr/UEFI_disk/facs.aml, Length 0x40 (64) bytes
ACPI: FACS 0x0000000000000000 000040
Acpi Data Table [FACS] decoded
Formatted output:  /home/kostr/UEFI_disk/facs.dsl - 1394 bytes
File appears to be binary: found 33 non-ASCII characters, disassembling
Binary file appears to be a valid ACPI table, disassembling
Input file /home/kostr/UEFI_disk/hpet.aml, Length 0x38 (56) bytes
ACPI: HPET 0x0000000000000000 000038 (v01 BOCHS  BXPCHPET 00000001 BXPC 00000001)
Acpi Data Table [HPET] decoded
Formatted output:  /home/kostr/UEFI_disk/hpet.dsl - 1891 bytes
```

Now you have *.dsl files in the same `UEFI_disk` shared folder.

For example here is a content for `APIC` table:
```
$ cat ~/UEFI_disk/apic.dsl
/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20190509 (64-bit version)
 * Copyright (c) 2000 - 2019 Intel Corporation
 *
 * Disassembly of /home/kostr/UEFI_disk/apic.aml, Sat Jul  3 00:09:16 2021
 *
 * ACPI Data Table [APIC]
 *
 * Format: [HexOffset DecimalOffset ByteLength]  FieldName : FieldValue
 */

[000h 0000   4]                    Signature : "APIC"    [Multiple APIC Description Table (MADT)]
[004h 0004   4]                 Table Length : 00000078
[008h 0008   1]                     Revision : 01
[009h 0009   1]                     Checksum : ED
[00Ah 0010   6]                       Oem ID : "BOCHS "
[010h 0016   8]                 Oem Table ID : "BXPCAPIC"
[018h 0024   4]                 Oem Revision : 00000001
[01Ch 0028   4]              Asl Compiler ID : "BXPC"
[020h 0032   4]        Asl Compiler Revision : 00000001

[024h 0036   4]           Local Apic Address : FEE00000
[028h 0040   4]        Flags (decoded below) : 00000001
                         PC-AT Compatibility : 1

[02Ch 0044   1]                Subtable Type : 00 [Processor Local APIC]
[02Dh 0045   1]                       Length : 08
[02Eh 0046   1]                 Processor ID : 00
[02Fh 0047   1]                Local Apic ID : 00
[030h 0048   4]        Flags (decoded below) : 00000001
                           Processor Enabled : 1
                      Runtime Online Capable : 0

[034h 0052   1]                Subtable Type : 01 [I/O APIC]
[035h 0053   1]                       Length : 0C
[036h 0054   1]                  I/O Apic ID : 00
[037h 0055   1]                     Reserved : 00
[038h 0056   4]                      Address : FEC00000
[03Ch 0060   4]                    Interrupt : 00000000

[040h 0064   1]                Subtable Type : 02 [Interrupt Source Override]
[041h 0065   1]                       Length : 0A
[042h 0066   1]                          Bus : 00
[043h 0067   1]                       Source : 00
[044h 0068   4]                    Interrupt : 00000002
[048h 0072   2]        Flags (decoded below) : 0000
                                    Polarity : 0
                                Trigger Mode : 0

[04Ah 0074   1]                Subtable Type : 02 [Interrupt Source Override]
[04Bh 0075   1]                       Length : 0A
[04Ch 0076   1]                          Bus : 00
[04Dh 0077   1]                       Source : 05
[04Eh 0078   4]                    Interrupt : 00000005
[052h 0082   2]        Flags (decoded below) : 000D
                                    Polarity : 1
                                Trigger Mode : 3

[054h 0084   1]                Subtable Type : 02 [Interrupt Source Override]
[055h 0085   1]                       Length : 0A
[056h 0086   1]                          Bus : 00
[057h 0087   1]                       Source : 09
[058h 0088   4]                    Interrupt : 00000009
[05Ch 0092   2]        Flags (decoded below) : 000D
                                    Polarity : 1
                                Trigger Mode : 3

[05Eh 0094   1]                Subtable Type : 02 [Interrupt Source Override]
[05Fh 0095   1]                       Length : 0A
[060h 0096   1]                          Bus : 00
[061h 0097   1]                       Source : 0A
[062h 0098   4]                    Interrupt : 0000000A
[066h 0102   2]        Flags (decoded below) : 000D
                                    Polarity : 1
                                Trigger Mode : 3

[068h 0104   1]                Subtable Type : 02 [Interrupt Source Override]
[069h 0105   1]                       Length : 0A
[06Ah 0106   1]                          Bus : 00
[06Bh 0107   1]                       Source : 0B
[06Ch 0108   4]                    Interrupt : 0000000B
[070h 0112   2]        Flags (decoded below) : 000D
                                    Polarity : 1
                                Trigger Mode : 3

[072h 0114   1]                Subtable Type : 04 [Local APIC NMI]
[073h 0115   1]                       Length : 06
[074h 0116   1]                 Processor ID : FF
[075h 0117   2]        Flags (decoded below) : 0000
                                    Polarity : 0
                                Trigger Mode : 0
[077h 0119   1]         Interrupt Input LINT : 01

Raw Table Data: Length 120 (0x78)

    0000: 41 50 49 43 78 00 00 00 01 ED 42 4F 43 48 53 20  // APICx.....BOCHS
    0010: 42 58 50 43 41 50 49 43 01 00 00 00 42 58 50 43  // BXPCAPIC....BXPC
    0020: 01 00 00 00 00 00 E0 FE 01 00 00 00 00 08 00 00  // ................
    0030: 01 00 00 00 01 0C 00 00 00 00 C0 FE 00 00 00 00  // ................
    0040: 02 0A 00 00 02 00 00 00 00 00 02 0A 00 05 05 00  // ................
    0050: 00 00 0D 00 02 0A 00 09 09 00 00 00 0D 00 02 0A  // ................
    0060: 00 0A 0A 00 00 00 0D 00 02 0A 00 0B 0B 00 00 00  // ................
    0070: 0D 00 04 06 FF 00 00 01                          // ........
```


