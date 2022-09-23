/*
 * Copyright (c) 2022, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Pi/PiFirmwareFile.h>
#include <Pi/PiFirmwareVolume.h>
#include <Protocol/FirmwareVolume2.h>
#include <Library/MemoryAllocationLib.h>


VOID PrintBuffer(UINT8* Buffer, UINTN BufferSize)
{
  UINTN i=0;
  while (i<BufferSize) {
    if (!(i%16)) {
      Print(L"0x%08x: ", i);
    }
    Print(L"%02x", Buffer[i]);
    i++;
    if (i%16) {
      if (i != BufferSize) {
        Print(L" ");
      }
    } else {
      Print(L"  |");
      for (UINT8 j=16; j>0; j--) {
        if ((Buffer[i-j]>0x20) && (Buffer[i-j]<0x7E)) {
          Print(L"%c", Buffer[i-j]);
        } else {
          Print(L".");
        }
      }
      Print(L"|\n");
    }
  }
  if (i%16) {
    while (i%16) {
      Print(L"   ");
      i++;
    }
    Print(L"  |");
    for (UINT8 j=16; j>0; j--) {
      if ((i-j) < BufferSize) {
        if ((Buffer[i-j]>0x20) && (Buffer[i-j]<0x7E)) {
          Print(L"%c", Buffer[i-j]);
        } else {
          Print(L".");
        }
      }
    }
    Print(L"|\n");
  }
  Print(L"\n");
}


CHAR16* SectionTypeString(EFI_SECTION_TYPE SectionType)
{
  if (SectionType == EFI_SECTION_ALL)
    return L"ALL";
  else if (SectionType == EFI_SECTION_COMPRESSION)
    return L"COMPRESSION";
  else if (SectionType == EFI_SECTION_GUID_DEFINED)
    return L"GUID_DEFINED";
  else if (SectionType == EFI_SECTION_DISPOSABLE)
    return L"DISPOSABLE";
  else if (SectionType == EFI_SECTION_PE32)
    return L"PE32";
  else if (SectionType == EFI_SECTION_PIC)
    return L"PIC";
  else if (SectionType == EFI_SECTION_TE)
    return L"TE";
  else if (SectionType == EFI_SECTION_DXE_DEPEX)
    return L"DXE_DEPEX";
  else if (SectionType == EFI_SECTION_VERSION)
    return L"VERSION";
  else if (SectionType == EFI_SECTION_USER_INTERFACE)
    return L"USER_INTERFACE";
  else if (SectionType == EFI_SECTION_COMPATIBILITY16)
    return L"COMPATIBILITY16";
  else if (SectionType == EFI_SECTION_FIRMWARE_VOLUME_IMAGE)
    return L"FV_IMAGE";
  else if (SectionType == EFI_SECTION_FREEFORM_SUBTYPE_GUID)
    return L"SUBTYPE_GUID";
  else if (SectionType == EFI_SECTION_RAW)
    return L"RAW";
  else if (SectionType == EFI_SECTION_PEI_DEPEX)
    return L"PEI_DEPEX";
  else if (SectionType == EFI_SECTION_MM_DEPEX)
    return L"MM_DEPEX";
  else
    return L"UNKNOWN";
}


CHAR16* FileTypeString(EFI_FV_FILETYPE FileType)
{
  if (FileType == EFI_FV_FILETYPE_RAW)
    return L"RAW";
  else if (FileType == EFI_FV_FILETYPE_FREEFORM)
    return L"FREEFORM";
  else if (FileType == EFI_FV_FILETYPE_SECURITY_CORE)
    return L"SEC_CORE";
  else if (FileType == EFI_FV_FILETYPE_PEI_CORE)
    return L"PEI_CORE";
  else if (FileType == EFI_FV_FILETYPE_DXE_CORE)
    return L"DXE_CORE";
  else if (FileType == EFI_FV_FILETYPE_PEIM)
    return L"PEIM";
  else if (FileType == EFI_FV_FILETYPE_DRIVER)
    return L"DRIVER";
  else if (FileType == EFI_FV_FILETYPE_COMBINED_PEIM_DRIVER)
    return L"COMBINED_PEIM_DRIVER";
  else if (FileType == EFI_FV_FILETYPE_APPLICATION)
    return L"APPLICATION";
  else if (FileType == EFI_FV_FILETYPE_MM)
    return L"MM";
  else if (FileType == EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE)
    return L"FV_IMAGE";
  else if (FileType == EFI_FV_FILETYPE_COMBINED_MM_DXE)
    return L"COMBINED_MM_DXE";
  else if (FileType == EFI_FV_FILETYPE_MM_CORE)
    return L"MM_CORE";
  else if (FileType == EFI_FV_FILETYPE_MM_STANDALONE)
    return L"MM_STANDALONE";
  else if (FileType == EFI_FV_FILETYPE_MM_CORE_STANDALONE)
    return L"MM_CORE_STANDALONE";
  else if ((FileType >= 0xC0) && (FileType <= 0xDF))
    return L"OEM";
  else if ((FileType >= 0xE0) && (FileType <= 0xEF))
    return L"DEBUG";
  else if (FileType == EFI_FV_FILETYPE_FFS_PAD)
    return L"FFS_PAD";
  else if ((FileType >= 0xF0) && (FileType <= 0xFF))
    return L"FFS";
  else
    return L"UNKNOWN";
}


EFI_STATUS
PrintFiles(
  IN EFI_FIRMWARE_VOLUME2_PROTOCOL* FV2Protocol
  )
{
  EFI_STATUS Status;
  EFI_GUID FileGuid;
  EFI_FV_FILE_ATTRIBUTES Attributes;
  UINTN Size;
  EFI_FV_FILETYPE FileType;
  UINTN Key = 0;

  do {
    FileType = EFI_FV_FILETYPE_ALL;
    Status = FV2Protocol->GetNextFile(
                            FV2Protocol,
                            (VOID**)&Key,
                            &FileType,
                            &FileGuid,
                            &Attributes,
                            &Size
                          );
    if (!EFI_ERROR(Status))
      Print(L"%g - %s - %d bytes\n", FileGuid, FileTypeString(FileType), Size);
  } while (Status == EFI_SUCCESS);

  return EFI_SUCCESS;
}


EFI_STATUS
ReadFile(
  IN EFI_FIRMWARE_VOLUME2_PROTOCOL* FV2Protocol,
  IN EFI_GUID* FileGuid
  )
{
  EFI_STATUS Status;
  UINT8* Buffer = NULL;
  UINTN BufferSize;
  EFI_FV_FILETYPE FileType;
  EFI_FV_FILE_ATTRIBUTES FileAttributes;
  UINT32 AuthenticationStatus;
  Status = FV2Protocol->ReadFile(
                          FV2Protocol,
                          FileGuid,
                          (VOID**)&Buffer,
                          &BufferSize,
                          &FileType,
                          &FileAttributes,
                          &AuthenticationStatus
                        );
  if (EFI_ERROR(Status)) {
    Print(L"Error! ReadFile returned error: %r\n", Status);
    return Status;
  }

  Print(L"FileType=%s\n", FileTypeString(FileType));
  Print(L"FileAttributes=0x%08x\n", FileAttributes);
  Print(L"AuthenticationStatus=0x%08x\n", AuthenticationStatus);
  Print(L"\n");

  Print(L"Raw Data:\n");
  PrintBuffer(Buffer, BufferSize);
  Print(L"-------------------------------------\n");
  if (FileType == EFI_FV_FILETYPE_RAW)
    return EFI_SUCCESS;
  Print(L"Parsed Data:\n\n");
  UINTN i=0;
  while (i < BufferSize) {
    EFI_COMMON_SECTION_HEADER* SectionHeader = (EFI_COMMON_SECTION_HEADER*) &Buffer[i];
    UINTN SectionSize = SectionHeader->Size[0] + (SectionHeader->Size[1] << 8) + (SectionHeader->Size[2] << 16);
    Print(L"Section %s, size 0x%08x\n", SectionTypeString(SectionHeader->Type), SectionSize);
    Print(L"Data:\n");
    PrintBuffer(&Buffer[i] + sizeof(EFI_COMMON_SECTION_HEADER), SectionSize - sizeof(EFI_COMMON_SECTION_HEADER));
    i += SectionSize;
    if (i%4)
      i += (4-(i%4));
  }
  FreePool(Buffer);
  return EFI_SUCCESS;
}


EFI_STATUS
ReadSection(
  IN EFI_FIRMWARE_VOLUME2_PROTOCOL* FV2Protocol,
  IN EFI_GUID* FileGuid,
  IN EFI_SECTION_TYPE SectionType,
  IN UINTN SectionInstance
  )
{
  EFI_STATUS Status;
  UINT8* Buffer = NULL;
  UINTN BufferSize;
  UINT32 AuthenticationStatus;
  Print(L"Section %s %d\n", SectionTypeString(SectionType), SectionInstance);
  Status = FV2Protocol->ReadSection(
                          FV2Protocol,
                          FileGuid,
                          SectionType,
                          SectionInstance,
                          (VOID**)&Buffer,
                          &BufferSize,
                          &AuthenticationStatus
                        );
  if (!EFI_ERROR(Status)) {
    PrintBuffer(Buffer, BufferSize);
  } else {
    Print(L"Error! ReadSection returned error: %r\n", Status);
  }
  FreePool(Buffer);
  return Status;
}


VOID Usage()
{
  Print(L"Usage:\n");
  Print(L"  FfsFile [<FileGUID> [<SectionType> <SectionInstance>]]\n");
  Print(L"\n");
  Print(L"<FileGUID>:\n");
  Print(L"GUID name of the File in FFS\n");
  Print(L"\n");
  Print(L"<SectionType>:\n");
  Print(L"ALL|COMPRESS|GUIDED|DISPOSABLE|PE32|PIC|TE|DXE_DEPEX|PEI_DEPEX|MM_DEPEX\n");
  Print(L"VERSION|UI|COMPAT16|FV_IMAGE|SUBTYPE_GUID|RAW\n");
  Print(L"\n");
  Print(L"<SectionInstance>: section instance number in a target file\n");
}


INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS Status;

  if ((Argc != 1) && (Argc != 2) && (Argc != 4)) {
    Usage();
    return EFI_INVALID_PARAMETER;
  }

  EFI_GUID FileGuid;
  if (Argc > 1) {
    Status = StrToGuid(Argv[1], &FileGuid);
    if (Status != RETURN_SUCCESS) {
      Print(L"Error! Can't convert <File GUID> argument to GUID\n");
      return EFI_INVALID_PARAMETER;
    }
  }

  EFI_SECTION_TYPE SectionType;
  UINTN SectionInstance;
  if (Argc == 4) {
    if (!StrCmp(Argv[2], L"ALL"))
      SectionType = EFI_SECTION_ALL;
    else if (!StrCmp(Argv[2], L"COMPRESS"))
      SectionType = EFI_SECTION_COMPRESSION;
    else if (!StrCmp(Argv[2], L"GUIDED"))
      SectionType = EFI_SECTION_GUID_DEFINED;
    else if (!StrCmp(Argv[2], L"DISPOSABLE"))
      SectionType = EFI_SECTION_DISPOSABLE;
    else if (!StrCmp(Argv[2], L"PE32"))
      SectionType = EFI_SECTION_PE32;
    else if (!StrCmp(Argv[2], L"PIC"))
      SectionType = EFI_SECTION_PIC;
    else if (!StrCmp(Argv[2], L"TE"))
      SectionType = EFI_SECTION_TE;
    else if (!StrCmp(Argv[2], L"DXE_DEPEX"))
      SectionType = EFI_SECTION_DXE_DEPEX;
    else if (!StrCmp(Argv[2], L"VERSION"))
      SectionType = EFI_SECTION_VERSION;
    else if (!StrCmp(Argv[2], L"UI"))
      SectionType = EFI_SECTION_USER_INTERFACE;
    else if (!StrCmp(Argv[2], L"COMPAT16"))
      SectionType = EFI_SECTION_COMPATIBILITY16;
    else if (!StrCmp(Argv[2], L"FV_IMAGE"))
      SectionType = EFI_SECTION_FIRMWARE_VOLUME_IMAGE;
    else if (!StrCmp(Argv[2], L"SUBTYPE_GUID"))
      SectionType = EFI_SECTION_FREEFORM_SUBTYPE_GUID;
    else if (!StrCmp(Argv[2], L"RAW"))
      SectionType = EFI_SECTION_RAW;
    else if (!StrCmp(Argv[2], L"PEI_DEPEX"))
      SectionType = EFI_SECTION_PEI_DEPEX;
    else if (!StrCmp(Argv[2], L"MM_DEPEX"))
      SectionType = EFI_SECTION_MM_DEPEX;
    else {
      Print(L"Error! Wrong <SectionType>\n");
      return EFI_INVALID_PARAMETER;
    }

    SectionInstance = StrDecimalToUintn(Argv[3]);
  }


  EFI_FIRMWARE_VOLUME2_PROTOCOL* FV2Protocol;
  Status = gBS->LocateProtocol(
                  &gEfiFirmwareVolume2ProtocolGuid,
                  NULL,
                  (VOID**)&FV2Protocol
                );
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't locate EFI_FIRMWARE_VOLUME2_PROTOCOL: %r\n", Status);
    return Status;
  }

  if (Argc == 1)
    return PrintFiles(FV2Protocol);

  if (Argc == 2)
    return ReadFile(FV2Protocol, &FileGuid);

  if (Argc == 4)
    return ReadSection(FV2Protocol, &FileGuid, SectionType, SectionInstance);

  return EFI_SUCCESS;
}
