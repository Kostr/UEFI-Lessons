/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/HiiDatabase.h>
#include <Library/MemoryAllocationLib.h>


CHAR16* PackageType(UINTN Type)
{
  switch(Type) {
    case EFI_HII_PACKAGE_TYPE_ALL:
      return L"ALL";
    case EFI_HII_PACKAGE_TYPE_GUID:
      return L"GUID";
    case EFI_HII_PACKAGE_FORMS:
      return L"FORMS";
    case EFI_HII_PACKAGE_STRINGS:
      return L"STRINGS";
    case EFI_HII_PACKAGE_FONTS:
      return L"FONTS";
    case EFI_HII_PACKAGE_IMAGES:
      return L"IMAGES";
    case EFI_HII_PACKAGE_SIMPLE_FONTS:
      return L"SIMPLE_FONTS";
    case EFI_HII_PACKAGE_DEVICE_PATH:
      return L"DEVICE_PATH";
    case EFI_HII_PACKAGE_KEYBOARD_LAYOUT:
      return L"KEYBOARD_LAYOUT";
    case EFI_HII_PACKAGE_ANIMATIONS:
      return L"ANIMATIONS";
    case EFI_HII_PACKAGE_END:
      return L"END";
    case EFI_HII_PACKAGE_TYPE_SYSTEM_BEGIN:
      return L"SYSTEM_BEGIN";
    case EFI_HII_PACKAGE_TYPE_SYSTEM_END:
      return L"SYSTEM_END";
  }
  return L"UNKNOWN";
}

VOID ParseHiiPackageLists(EFI_HII_PACKAGE_LIST_HEADER* HiiDatabase, UINTN HiiDatabaseSize)
{
  EFI_HII_PACKAGE_LIST_HEADER* HiiPackageListHeader;
  HiiPackageListHeader = (EFI_HII_PACKAGE_LIST_HEADER*) HiiDatabase;

  UINTN i=0;
  while ((UINTN) HiiPackageListHeader < ((UINTN) HiiDatabase + HiiDatabaseSize)) {
    UINTN HiiPackageListSize = HiiPackageListHeader->PackageLength;
    if (HiiPackageListSize == 0)
      break;
    Print(L"PackageList[%d]: GUID=%g; size=0x%X\n", i++, HiiPackageListHeader->PackageListGuid, HiiPackageListHeader->PackageLength);

    EFI_HII_PACKAGE_HEADER* HiiPackageHeader = (EFI_HII_PACKAGE_HEADER*)((UINTN) HiiPackageListHeader + sizeof(EFI_HII_PACKAGE_LIST_HEADER));
    UINTN j=0;
    while ((UINTN) HiiPackageHeader < ((UINTN) HiiPackageListHeader + HiiPackageListSize)) {
      Print(L"\tPackage[%d]: type=%s; size=0x%X\n", j++, PackageType(HiiPackageHeader->Type), HiiPackageHeader->Length);

      // Go to next Package
      HiiPackageHeader = (EFI_HII_PACKAGE_HEADER*)((UINTN) HiiPackageHeader + HiiPackageHeader->Length);
    }

    // Go to next PackageList
    HiiPackageListHeader = (EFI_HII_PACKAGE_LIST_HEADER*)((UINTN) HiiPackageListHeader + HiiPackageListSize);
  }
}

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  EFI_HII_DATABASE_PROTOCOL* HiiDbProtocol;
  Status = gBS->LocateProtocol(&gEfiHiiDatabaseProtocolGuid,
                               NULL, 
                               (VOID**)&HiiDbProtocol);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Could not find HII Database protocol: %r\n", Status);
    return Status;
  }

  UINTN PackageListSize = 0;
  EFI_HII_PACKAGE_LIST_HEADER* PackageList = NULL;

  Status = HiiDbProtocol->ExportPackageLists(HiiDbProtocol,
                                             NULL,		// All package lists
                                             &PackageListSize,
                                             PackageList);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    Print(L"ERROR: Could not obtain package list size\n");
    return Status;
  }

  Status = gBS->AllocatePool(EfiBootServicesData, 
                             PackageListSize, 
                             (VOID**)&PackageList);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Could not allocate sufficient memory for package list: %r\n", Status);
    return Status;
  }


  Status = HiiDbProtocol->ExportPackageLists(HiiDbProtocol,
                                             NULL,
                                             &PackageListSize,
                                             PackageList);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Could not retrieve the package list: %r\n", Status);
    FreePool(PackageList);
    return Status;
  }

  ParseHiiPackageLists(PackageList, PackageListSize);

  FreePool(PackageList);

  return EFI_SUCCESS;
}
