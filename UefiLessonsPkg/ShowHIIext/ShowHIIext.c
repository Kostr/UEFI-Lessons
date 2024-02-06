/*
 * Copyright (c) 2024, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/HiiDatabase.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ShellLib.h>
#include <Library/PrintLib.h>
#include <Library/HiiLib.h>

GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID mStringHelpTokenId = STRING_TOKEN(STR_HELP);

BOOLEAN savePackageLists = FALSE;
UINTN savePackageIndex = 0xffffffff; // ALL

EFI_STATUS WriteFile(CHAR16* FileName, VOID* Data, UINTN* Size)
{
  SHELL_FILE_HANDLE FileHandle;
  EFI_STATUS Status = ShellOpenFileByName(
    FileName,
    &FileHandle,
    EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ,
    0
  );
  if (!EFI_ERROR(Status)) {
    Print(L"Save file as %s\n", FileName);
    UINTN ToWrite = *Size;
    Status = ShellWriteFile(
      FileHandle,
      Size,
      Data
    );
    if (EFI_ERROR(Status)) {
      Print(L"Can't write file: %r\n", Status);
    }
    if (*Size != ToWrite) {
      Print(L"Error! Not all data was written\n");
    }
    Status = ShellCloseFile(
      &FileHandle
    );
    if (EFI_ERROR(Status)) {
      Print(L"Can't close file: %r\n", Status);
    }
  } else {
    Print(L"Can't open file: %r\n", Status);
  }
  return Status;
}

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
    if (savePackageLists) {
      if ((savePackageIndex == 0xFFFFFFFF) || (savePackageIndex == i)) {
        CHAR16 FileName[100];
        UnicodeSPrint(FileName, 100, L"%04d_%g", i, HiiPackageListHeader->PackageListGuid);
        UINTN ToWrite = HiiPackageListSize;
        EFI_STATUS Status = WriteFile(FileName, HiiPackageListHeader, &ToWrite);
        if (EFI_ERROR(Status)) {
          Print(L"Error! Failed to write PackageList %d\n", i);
        }
      }
    }
    if (!savePackageLists)
      Print(L"PackageList[%d]: GUID=%g; size=0x%X\n", i, HiiPackageListHeader->PackageListGuid, HiiPackageListHeader->PackageLength);
    i++;

    EFI_HII_PACKAGE_HEADER* HiiPackageHeader = (EFI_HII_PACKAGE_HEADER*)((UINTN) HiiPackageListHeader + sizeof(EFI_HII_PACKAGE_LIST_HEADER));
    UINTN j=0;
    while ((UINTN) HiiPackageHeader < ((UINTN) HiiPackageListHeader + HiiPackageListSize)) {
      if (!savePackageLists)
        Print(L"\tPackage[%d]: type=%s; size=0x%X\n", j++, PackageType(HiiPackageHeader->Type), HiiPackageHeader->Length);

      // Go to next Package
      HiiPackageHeader = (EFI_HII_PACKAGE_HEADER*)((UINTN) HiiPackageHeader + HiiPackageHeader->Length);
    }

    // Go to next PackageList
    HiiPackageListHeader = (EFI_HII_PACKAGE_LIST_HEADER*)((UINTN) HiiPackageListHeader + HiiPackageListSize);
  }
}

VOID Usage()
{
  CHAR16 HelpStr[100];
  UnicodeSPrint(HelpStr, 100, L"%a", gEfiCallerBaseName);
  ShellPrintHelp(HelpStr, NULL, FALSE);
}

INTN EFIAPI ShellAppMain(IN UINTN Argc, IN CHAR16 **Argv)
{
  EFI_STATUS Status;

  UINTN bOptionIndex = Argc;
  UINTN hOptionIndex = Argc;
  for (UINTN i = 1; i < Argc; i++) {
    if (!StrCmp(Argv[i], L"-b")) {
      ShellSetPageBreakMode(TRUE);
      bOptionIndex = i;
    } else if (!StrCmp(Argv[i], L"-?")) {
      hOptionIndex = i;
    }
  }

  for (UINTN i = 1; i < Argc; i++) {
    if ((i == bOptionIndex) || (i == hOptionIndex))
      continue;
    if (!StrCmp(Argv[i], L"save")) {
      savePackageLists = TRUE;
      if (((i + 1) < Argc) && ((i + 1) != bOptionIndex) && ((i + 1) != hOptionIndex)) {
        CHAR16* EndPointer;
        Status = StrDecimalToUintnS(Argv[i + 1], &EndPointer, &savePackageIndex);
        if (EFI_ERROR(Status) || (EndPointer == Argv[i + 1])) {
          Usage();
          return EFI_INVALID_PARAMETER;
        }
        i += 1;
      }
    } else {
      Usage();
      return EFI_INVALID_PARAMETER;
    }
  }

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
