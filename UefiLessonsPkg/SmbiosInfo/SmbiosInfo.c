/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/BaseMemoryLib.h>
#include <Protocol/Smbios.h>

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

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  for (UINTN i=0; i<SystemTable->NumberOfTableEntries; i++) {
    if (CompareGuid(&(SystemTable->ConfigurationTable[i].VendorGuid), &gEfiSmbiosTableGuid)) {
      Print(L"SMBIOS table is placed at %p\n\n", SystemTable->ConfigurationTable[i].VendorTable);
    }
  } 

  EFI_SMBIOS_PROTOCOL* SmbiosProtocol;
  EFI_STATUS Status = gBS->LocateProtocol (
                  &gEfiSmbiosProtocolGuid,
                  NULL,
                  (VOID**)&SmbiosProtocol
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  EFI_SMBIOS_HANDLE SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  EFI_SMBIOS_TABLE_HEADER* Record;
  Status = SmbiosProtocol->GetNext(SmbiosProtocol,
                                   &SmbiosHandle,
                                   NULL,
                                   &Record,
                                   NULL);
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
  
  return EFI_SUCCESS;
}
