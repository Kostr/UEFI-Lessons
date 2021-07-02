In the last lesson we've discovered that our system has BGRT ACPI table.

According to the ACPI specification:
```
The Boot Graphics Resource Table (BGRT) is an optional table that provides a mechanism to indicate that
an image was drawn on the screen during boot, and some information about the image.
The table is written when the image is drawn on the screen. This should be done after it is expected that
any firmware components that may write to the screen are done doing so and it is known that the image
is the only thing on the screen. If the boot path is interrupted (e.g., by a key press), the valid bit within the
status field should be changed to 0 to indicate to the OS that the current image is invalidated
```
This table actually have a pointer to image data, check structure definition under https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Acpi63.h:
```
///
/// Boot Graphics Resource Table definition.
///
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;
  ///
  /// 2-bytes (16 bit) version ID. This value must be 1.
  ///
  UINT16                      Version;
  ///
  /// 1-byte status field indicating current status about the table.
  ///     Bits[7:1] = Reserved (must be zero)
  ///     Bit [0] = Valid. A one indicates the boot image graphic is valid.
  ///
  UINT8                       Status;
  ///
  /// 1-byte enumerated type field indicating format of the image.
  ///     0 = Bitmap
  ///     1 - 255  Reserved (for future use)
  ///
  UINT8                       ImageType;
  ///
  /// 8-byte (64 bit) physical address pointing to the firmware's in-memory copy
  /// of the image bitmap.
  ///
  UINT64                      ImageAddress;
  ///
  /// A 4-byte (32-bit) unsigned long describing the display X-offset of the boot image.
  /// (X, Y) display offset of the top left corner of the boot image.
  /// The top left corner of the display is at offset (0, 0).
  ///
  UINT32                      ImageOffsetX;
  ///
  /// A 4-byte (32-bit) unsigned long describing the display Y-offset of the boot image.
  /// (X, Y) display offset of the top left corner of the boot image.
  /// The top left corner of the display is at offset (0, 0).
  ///
  UINT32                      ImageOffsetY;
} EFI_ACPI_6_3_BOOT_GRAPHICS_RESOURCE_TABLE;
```


Let's create an app that would save an image from BGRT.

This time to get BGRT table we would utilize `EFI_ACPI_SDT_PROTOCOL` protocol.

To get ACPI table data we would use `GetAcpiTable()` function from this protocol:
```
EFI_ACPI_SDT_PROTOCOL.GetAcpiTable()

Summary:
Returns a requested ACPI table.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_ACPI_GET_ACPI_TABLE) (
 IN UINTN Index,
 OUT EFI_ACPI_SDT_HEADER **Table,
 OUT EFI_ACPI_TABLE_VERSION *Version,
 OUT UINTN *TableKey
 );

Parameters:
Index		The zero-based index of the table to retrieve.
Table		Pointer for returning the table buffer.
Version		On return, updated with the ACPI versions to which this table belongs.
TableKey	On return, points to the table key for the specified ACPI system definition table.

Description:
The GetAcpiTable() function returns a pointer to a buffer containing the ACPI table associated with the Index that was input. The following structures are not considered elements in the list of ACPI tables:
- Root System Description Pointer (RSD_PTR)
- Root System Description Table (RSDT)
- Extended System Description Table (XSDT)
```
In edk2 it is defined here: https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/AcpiSystemDescriptionTable.h

To get all tables we need to call `GetAcpiTable` with incrementing values for `Index` starting with 0, while function returns `EFI_SUCCESS`.

On every success call we would get a pointer to a common header for a ACPI table:
```
typedef struct {
 UINT32 Signature;
 UINT32 Length;
 UINT8 Revision;
 UINT8 Checksum;
 CHAR8 OemId[6];
 CHAR8 OemTableId[8];
 UINT32 OemRevision;
 UINT32 CreatorId;
 UINT32 CreatorRevision;
} EFI_ACPI_SDT_HEADER;
```

To use `EFI_ACPI_SDT_PROTOCOL` we need to add include to our file:
```
#include <Protocol/AcpiSystemDescriptionTable.h>
```
And add protocol to the *.inf file:
```
[Protocols]
  gEfiAcpiSdtProtocolGuid
```

Here is a code finding BGRT ACPI table:
```
EFI_ACPI_SDT_PROTOCOL* AcpiSdtProtocol;
EFI_STATUS Status = gBS->LocateProtocol (
                &gEfiAcpiSdtProtocolGuid,
                NULL,
                (VOID**)&AcpiSdtProtocol
                );
if (EFI_ERROR (Status)) {
  return Status;
}

BOOLEAN BGRT_found = FALSE;
UINTN Index = 0;
EFI_ACPI_SDT_HEADER* Table;
EFI_ACPI_TABLE_VERSION Version;
UINTN TableKey;
while (TRUE) {
  Status = AcpiSdtProtocol->GetAcpiTable(Index,
    &Table,
    &Version,
    &TableKey
  );
  if (EFI_ERROR(Status)) {
    break;
  }
  if (((CHAR8)((Table->Signature >>  0) & 0xFF) == 'B') &&
      ((CHAR8)((Table->Signature >>  8) & 0xFF) == 'G') &&
      ((CHAR8)((Table->Signature >> 16) & 0xFF) == 'R') &&
      ((CHAR8)((Table->Signature >> 24) & 0xFF) == 'T')) {
    BGRT_found = TRUE;
    break;
  }
  Index++;
}
if (!BGRT_found) {
  Print(L"BGRT table is not present in the system\n");
  return EFI_UNSUPPORTED;
}
```

Now we need to save an image from BGRT table.

Currently ACPI specification support only BMP image type https://uefi.org/specs/ACPI/6.4/05_ACPI_Software_Programming_Model/ACPI_Software_Programming_Model.html#image-type

So first we check if the type is actually BMP:
```
EFI_ACPI_6_3_BOOT_GRAPHICS_RESOURCE_TABLE* BGRT = (EFI_ACPI_6_3_BOOT_GRAPHICS_RESOURCE_TABLE*)Table;
if (BGRT->ImageType == 0) {
  ...
}
```

Now we need to actually save a BMP image. BGRT doesn't contain any size for an image, only offset to data: `ImageAddress`.

To get image size we need to look at BMP header. 

In edk2 it is defined under https://github.com/tianocore/edk2/blob/master/MdePkg/Include/IndustryStandard/Bmp.h:
```
typedef struct {
  CHAR8         CharB;
  CHAR8         CharM;
  UINT32        Size;
  UINT16        Reserved[2];
  UINT32        ImageOffset;
  UINT32        HeaderSize;
  UINT32        PixelWidth;
  UINT32        PixelHeight;
  UINT16        Planes;          ///< Must be 1
  UINT16        BitPerPixel;     ///< 1, 4, 8, or 24
  UINT32        CompressionType;
  UINT32        ImageSize;       ///< Compressed image size in bytes
  UINT32        XPixelsPerMeter;
  UINT32        YPixelsPerMeter;
  UINT32        NumberOfColors;
  UINT32        ImportantColors;
} BMP_IMAGE_HEADER;
```

Don't forget to include this file in our program:
```
#include <IndustryStandard/Bmp.h>
```

When we know that the image is BMP, we can check its signature (`BM`), parse its size and actually write its data to a file. Here we use `EFI_STATUS WriteFile(CHAR16* FileName, VOID* Data, UINTN* Size)` function to write data to a file, we will define it in a minute:
```
BMP_IMAGE_HEADER* BMP = (BMP_IMAGE_HEADER*)(BGRT->ImageAddress);

if ((BMP->CharB != 'B') || (BMP->CharM != 'M')) {
  Print(L"BMP image has wrong signature!\n");
  return EFI_UNSUPPORTED;
}
Print(L"BGRT conatins BMP image with %dx%d resolution\n", BMP->PixelWidth, BMP->PixelHeight);
UINTN Size = BMP->Size;
Status = WriteFile(L"BGRT.bmp", BMP, &Size);
if (EFI_ERROR(Status)) {
  Print(L"Error! Can't write BGRT.bmp file\n");
}
```

Last time we've used `EFI_SHELL_PROTOCOL` to create a file and write data to it. This time we will try to utilize ShelLib:

https://github.com/tianocore/edk2/blob/master/ShellPkg/Include/Library/ShellLib.h

https://github.com/tianocore/edk2/blob/master/ShellPkg/Library/UefiShellLib/UefiShellLib.c

Again we will need 3 functions: for file open, write and close:
```
/**
  This function will open a file or directory referenced by filename.
  If return is EFI_SUCCESS, the Filehandle is the opened file's handle;
  otherwise, the Filehandle is NULL. Attributes is valid only for
  EFI_FILE_MODE_CREATE.
  @param[in] FileName           The pointer to file name.
  @param[out] FileHandle        The pointer to the file handle.
  @param[in] OpenMode           The mode to open the file with.
  @param[in] Attributes         The file's file attributes.
  ...
**/

EFI_STATUS
EFIAPI
ShellOpenFileByName(
  IN CONST CHAR16               *FileName,
  OUT SHELL_FILE_HANDLE         *FileHandle,
  IN UINT64                     OpenMode,
  IN UINT64                     Attributes
  );
```
```
/**
  Write data to a file.
  This function writes the specified number of bytes to the file at the current
  file position. The current file position is advanced the actual number of bytes
  written, which is returned in BufferSize. Partial writes only occur when there
  has been a data error during the write attempt (such as "volume space full").
  The file is automatically grown to hold the data if required. Direct writes to
  opened directories are not supported.
  @param[in] FileHandle          The opened file for writing.
  @param[in, out] BufferSize     On input the number of bytes in Buffer.  On output
                                 the number of bytes written.
  @param[in] Buffer              The buffer containing data to write is stored.
  ...
**/

EFI_STATUS
EFIAPI
ShellWriteFile(
  IN SHELL_FILE_HANDLE          FileHandle,
  IN OUT UINTN                  *BufferSize,
  IN VOID                       *Buffer
  );
```
```
/**
  Close an open file handle.
  This function closes a specified file handle. All "dirty" cached file data is
  flushed to the device, and the file is closed. In all cases the handle is
  closed.
  @param[in] FileHandle           The file handle to close.
**/

EFI_STATUS
EFIAPI
ShellCloseFile (
  IN SHELL_FILE_HANDLE          *FileHandle
  );
```

Advantage of using `ShellLib` is that now we don't need to find `EFI_SHELL_PROTOCOL` and work with it manually.


Our `WriteFile` function would look like this:
```
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
    Print(L"Save it to %s\n", FileName);
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
```

To use ShellLib we need to include a header in our program:
```
#include <Library/ShellLib.h>
```

Also we need to add `ShellPkg.dec` to our packages and add `ShellLib` to our library classes:
```
[Packages]
  MdePkg/MdePkg.dec
+  ShellPkg/ShellPkg.dec

[LibraryClasses]
  UefiApplicationEntryPoint
  UefiLib
+  ShellLib
```
Besides that our package `*.dsc` file needs to include a `ShellLib` library class:
```
[LibraryClasses]
  ...
  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
```

Unfortunately this is not enough, our current build would fail with a message, because `ShellLib` by itself needs another library:
```
build.py...
/home/kostr/tiano/edk2/UefiLessonsPkg/UefiLessonsPkg.dsc(...): error 4000: Instance of library class [FileHandleLib] is not found
```

To find it use our standard tactic:
```
$ grep FileHandleLib -r ./ --include=*.inf | grep LIBRARY_CLASS
```

In the end we had to add several more LibraryClasses to make our build succeed:
```
[LibraryClasses]
  ...
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
```

Build our app and execute it under OVMF:
```
FS0:\> SaveBGRT.efi
BGRT conatins BMP image with 193x58 resolution
Save it to BGRT.bmp7
FS0:\>
```

If you look at the BGRT.bmp picture that are app have produced, it would have the same content as https://raw.githubusercontent.com/tianocore/edk2/master/MdeModulePkg/Logo/Logo.bmp

The file itself wouldn't be the same since BGRT driver don't use an image from flash, but actually grabs a boot screen and transforms it to a BMP image. For the proof checkout how https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.c uses `TranslateGopBltToBmp` function from the https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/BaseBmpSupportLib/BmpSupportLib.c library.
If you find it strange that BGRT grabs a screen instead of using an image from flash, remember how BGRT is defined in ACPI specification:
```
The Boot Graphics Resource Table (BGRT) is an optional table that provides a mechanism to indicate that an image was drawn on the screen during boot
```

The file GUID for binary boot logo image is defined in the file https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Logo/Logo.inf
```
FILE_GUID                      = 7BB28B99-61BB-11D5-9A5D-0090273FC14D
```
It is a GUID that is usually used for the Logo image in BIOS. It is even hardcoded to https://github.com/tianocore/edk2/blob/master/BaseTools/Source/Python/Eot/Report.py
```
## GenerateFfs() method
#
#  Generate FFS information
#
#  @param self: The object pointer
#  @param FfsObj: FFS object after FV image is parsed
#
def GenerateFfs(self, FfsObj):
    self.FfsIndex = self.FfsIndex + 1
    if FfsObj is not None and FfsObj.Type in [0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0xA]:
        FfsGuid = FfsObj.Guid
        FfsOffset = FfsObj._OFF_
        FfsName = 'Unknown-Module'
        FfsPath = FfsGuid
        FfsType = FfsObj._TypeName[FfsObj.Type]

        # Hard code for Binary INF
        if FfsGuid.upper() == '7BB28B99-61BB-11D5-9A5D-0090273FC14D':
            FfsName = 'Logo'

        if FfsGuid.upper() == '7E374E25-8E01-4FEE-87F2-390C23C606CD':
            FfsName = 'AcpiTables'

        if FfsGuid.upper() == '961578FE-B6B7-44C3-AF35-6BC705CD2B1F':
            FfsName = 'Fat'
        ...
```

If you want to know how Logo and BGRT are work in edk2, checkout these drivers:
- https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Library/BootLogoLib/
- https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Logo/
- https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/
- https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/BaseBmpSupportLib/

