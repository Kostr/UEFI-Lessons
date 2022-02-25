/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <Library/MemoryAllocationLib.h>

VOID Usage()
{
  Print(L"Recalculate CRCs for dmpstore command dump\n");
  Print(L"\n");
  Print(L"  UpdateDmpstoreDump <filename>\n");
}

INTN EFIAPI ShellAppMain(IN UINTN Argc, IN CHAR16 **Argv)
{
  if (Argc!=2) {
    Usage();
    return EFI_INVALID_PARAMETER;
  }

  SHELL_FILE_HANDLE FileHandle;

  CHAR16* Filename = Argv[1];
  EFI_STATUS Status = ShellOpenFileByName(
    Filename,
    &FileHandle,
    EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ,
    0
  );
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't open file %s\n", Filename);
    return Status;
  }

  UINT64 FileSize;
  Status = ShellGetFileSize(FileHandle, &FileSize);
  if (EFI_ERROR(Status)) {
    Status = ShellCloseFile(&FileHandle);
    return SHELL_DEVICE_ERROR;
  }

  UINT64 FilePos = 0;
  while (FilePos < FileSize) {
    UINTN ToReadSize;
    UINT32 NameSize;
    ToReadSize = sizeof(NameSize);
    Status = ShellReadFile(FileHandle, &ToReadSize, &NameSize);
    if (EFI_ERROR(Status) || (ToReadSize != sizeof(NameSize))) {
      Status = SHELL_VOLUME_CORRUPTED;
      break;
    }
    FilePos += ToReadSize;

    UINT32 DataSize;
    ToReadSize = sizeof(DataSize);
    Status = ShellReadFile(FileHandle, &ToReadSize, &DataSize);
    if (EFI_ERROR(Status) || (ToReadSize != sizeof(DataSize))) {
      Status = SHELL_VOLUME_CORRUPTED;
      break;
    }
    FilePos += ToReadSize;
    
    UINTN RemainingSize = NameSize +
                          sizeof(EFI_GUID) +
                          sizeof(UINT32) +
                          DataSize;
    UINT8* Buffer = AllocatePool(sizeof(NameSize) + sizeof(DataSize) + RemainingSize);
    if (Buffer == NULL) {
      Status = SHELL_OUT_OF_RESOURCES;
      break;
    }

    *(UINT32*)Buffer = NameSize;
    *((UINT32*)Buffer + 1) = DataSize;
    
    ToReadSize = RemainingSize;
    Status = ShellReadFile(FileHandle, &ToReadSize, (UINT32*)Buffer + 2);
    if (EFI_ERROR(Status) || (ToReadSize != RemainingSize)) {
      Status = SHELL_VOLUME_CORRUPTED;
      FreePool (Buffer);
      break;
    }
    FilePos += ToReadSize;
    
    
    UINT32 Crc32;
    gBS->CalculateCrc32 (
       Buffer,
       sizeof(NameSize) + sizeof(DataSize) + RemainingSize,
       &Crc32
    );

    UINTN ToWriteSize = sizeof(Crc32);
    Status = ShellWriteFile(
      FileHandle,
      &ToWriteSize,
      &Crc32
    );
    if (EFI_ERROR(Status) || (ToWriteSize != sizeof(Crc32))) {
      Print(L"Error! Not all data was written\n");
      FreePool(Buffer);
      break;
    }
    FilePos += ToWriteSize;
    FreePool(Buffer);
  }

  if (EFI_ERROR(Status)) {
    Print(L"Error! %r\n", Status);
  }

  Status = ShellCloseFile(&FileHandle);
  if (EFI_ERROR(Status)) {
    Print(L"Can't close file: %r\n", Status);
  }

  return EFI_SUCCESS;
}
