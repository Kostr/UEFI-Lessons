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

  return EFI_SUCCESS;
}
