UEFI shell has a `dmpstore` command that helps to see content of UEFI variables.

If you'll look at the help of the `dmpstore` command you could see one more useful feature that this command presents. With this command it is possible to save UEFI variables to a file and load them back from such files:
```
FS0:\> dmpstore -?
...
DMPSTORE [-all | ([variable] [-guid guid])] [-s file]
DMPSTORE [-all | ([variable] [-guid guid])] [-l file]
...
  -s       - Saves variables to a file.
  -l       - Loads and sets variables from a file.
...
```
Let's try to use this mechanics to modify content of an existing UEFI variable. It can be a useful feature for the debug.

In some earlier lesson we've created the `ShowBootVariables.efi` application that displays boot sources based on the content of UEFU boot variables:
```
FS0:\> ShowBootVariables.efi
Boot0000
UiApp
Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(462CAA21-7614-4503-836E-8AB6F4662331)

Boot0001
UEFI QEMU DVD-ROM QM00003
PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)

Boot0002*
EFI Internal Shell
Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)

Boot0003
UEFI QEMU HARDDISK QM00001
PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
```

One of the variables that were parsed in this application is a `BootOrder` variable. Just in case you forgot:
```
The BootOrder variable contains an array of UINT16’s that make up an ordered list of the Boot####
options. The first element in the array is the value for the first logical boot option, the second element is
the value for the second logical boot option, etc. The BootOrder order list is used by the firmware’s
boot manager as the default boot order.
```

Print the content of the BootOrder variable
```
FS0:\> dmpstore BootOrder
Variable NV+RT+BS 'EFIGlobalVariable:BootOrder' DataSize = 0x08
  00000000: 00 00 01 00 02 00 03 00-                         *........*
```
This means that the order is:
```
Boot0000
Boot0001
Boot0002
Boot0003
```
Everything is like our `ShowBootVariables` application shows.

With a help of the `dmpstore` command we can dump the content of a `BootOrder` variable to a file:
```
FS0:\> dmpstore BootOrder -s BootOrder.bin
Save variable to file: BootOrder.bin.
Variable NV+RT+BS '8BE4DF61-93CA-11D2-AA0D-00E098032B8C:BootOrder' DataSize = 0x08
```

UEFI shell contains `hexedit` command in itself. With it we can see the content of a created file:

![hexediti_1.png](hexedit_1.png?raw=true "BootOrder.bin before modifications")

`hexedit` is a hex editor, you can see its help message with a `Ctrl+E` command:

![hexedit_help.png](hexedit_help.png?raw=true "hexedit help")

Exit help with `Ctrl-W`.

`dmpstore` command represents each variable with a following structure in a file:
```
{
  UINT32 NameSize;           // Size of the variable name in bytes
  UINT32 DataSize;           // Size of the variable data in bytes
  CHAR16 Name[NameSize/2];   // Variable name in CHAR16
  EFI_GUID Guid;             // Variable GUID
  UINT32 Attributes;         // Variable attributes
  UINT8 Data[DataSize];      // Variable data
  UINT32 Crc;                // CRC32 checksum for the record
}
```

Here is a file content with a highlight for the structure fileds:

![bootorder.png](bootorder.png?raw=true "bootorder")

Let's try to modify the file content changing the boot order to:
```
Boot0001
Boot0000
Boot0002
Boot0003
```

![hexedit_2.png](hexedit_2.png?raw=true "hexedit after modification")

Type `Ctrl+Q` to quit and enter `y` to save our modifications.

If you'll try to load the changed settings you would get an error:
```
FS0:\> dmpstore -l BootOrder.bin
Load and set variables from file: BootOrder.bin.
dmpstore: Incorrect file format.
dmpstore: No matching variables found. Guid 8BE4DF61-93CA-11D2-AA0D-00E098032B8C
```
This is happening because `UINT32 Crc` field of the record is not longer valid for the current record content.

Let's create an application `UpdateDmpstoreDump` to update CRC fields in the `dmpstore` dumps.

Once again as we would parse command shell arguments it is better to create a shell application. We would read and write files, therefore include `ShellLib` to the `LibraryClasses`:
`UefiLessonsPkg/UpdateDmpstoreDump/UpdateDmpstoreDump.inf`:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = UpdateDmpstoreDump
  FILE_GUID                      = d14fe21b-7dbf-40ff-96cb-5d6f5b63cda6
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = ShellCEntryLib

[Sources]
  UpdateDmpstoreDump.c

[Packages]
  MdePkg/MdePkg.dec
  ShellPkg/ShellPkg.dec

[LibraryClasses]
  UefiLib
  ShellCEntryLib
  ShellLib
```

In the `UefiLessonsPkg/UpdateDmpstoreDump/UpdateDmpstoreDump.c` we start from reading dump file name from the command argument and opening the file with read and write attributes:
```
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

  ...

  Status = ShellCloseFile(&FileHandle);
  if (EFI_ERROR(Status)) {
    Print(L"Can't close file: %r\n", Status);
  }

  return EFI_SUCCESS;
}
```

The dump file can have many records in itself and the size of a record is not a constant, but depends on the record fields. Therefore the only way to fix all record CRCs is to step throught the file records until the file end:
```
  UINT64 FileSize;
  Status = ShellGetFileSize(FileHandle, &FileSize);
  if (EFI_ERROR(Status)) {
    Status = ShellCloseFile(&FileHandle);
    return SHELL_DEVICE_ERROR;
  }

  UINT64 FilePos = 0;
  while (FilePos < FileSize) {
    ...
  }
```

Here is a code to read record data and calculate its CRC32. It is pretty similar to the one that `dmpstore` command has it its `LoadVariablesFromFilefunction` (https://github.com/tianocore/edk2/blob/master/ShellPkg/Library/UefiShellDebug1CommandsLib/DmpStore.c):
```
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

...

FreePool(Buffer);
```

To calculate the CRC32 checksum here we use `EFI_BOOT_SERVICES.CalculateCrc32()` function:
```
EFI_BOOT_SERVICES.CalculateCrc32()

Summary:
Computes and returns a 32-bit CRC for a data buffer.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_CALCULATE_CRC32)
 IN VOID *Data,
 IN UINTN DataSize,
 OUT UINT32 *Crc32
 );

Parameters:
Data 		A pointer to the buffer on which the 32-bit CRC is to be computed.
DataSize 	The number of bytes in the buffer Data.
Crc32 		The 32-bit CRC that was computed for the data buffer specified by Data and DataSize.

Description:
This function computes the 32-bit CRC for the data buffer specified by Data and DataSize. If the 32-bit CRC is computed, then it is returned in Crc32 and EFI_SUCCESS is returned.
```

When we have our CRC32 checksum we can update file content with a help of a `ShellWriteFile` function:
```
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
```

Build our application and use it on the `dmpstore` dump:
```
FS0:\> UpdateDmpstoreDump.efi BootOrder.bin
```

If you look at the file content again you would see that the CRC field has changed.

![hexedit_3.png](hexedit_3.png?raw=true "hexedit after CRC modification")

Now `dmpstore -l` would finish without errors:
```
FS0:\> dmpstore -l BootOrder.bin
Load and set variables from file: BootOrder.bin.
Variable NV+RT+BS '8BE4DF61-93CA-11D2-AA0D-00E098032B8C:BootOrder' DataSize = 0x08
```

You can see that variable content was modified:
```
FS0:\> dmpstore BootOrder
Variable NV+RT+BS 'EFIGlobalVariable:BootOrder' DataSize = 0x08
  00000000: 01 00 00 00 02 00 03 00-                         *........*
```

You can also use our `ShowBootVariables.efi` application to see the changes:
```
FS0:\> ShowBootVariables.efi
Boot0001
UEFI QEMU DVD-ROM QM00003
PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)

Boot0000
UiApp
Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(462CAA21-7614-4503-836E-8AB6F4662331)

Boot0002*
EFI Internal Shell
Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)

Boot0003
UEFI QEMU HARDDISK QM00001
PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
```
__________________________

We can also verify that our program works in case when there are multiple variables in the dump file.

Just in case you have some persistent variables from the previous lesson delete all the variables under our GUID with a `dmpstore -d -guid <GUID>` command:
```
FS0:\> dmpstore -d -guid bb2a829f-7943-4691-a03a-f1f48519d7e6
dmpstore: No matching variables found. Guid 7C04A583-9E3E-4F1C-AD65-E05268D0B4D1

```

Create new variables and save them to a file:
```
FS0:\> SetVariableExample.efi HelloVar nb "Hello World"
Variable HelloVar was successfully changed
FS0:\> SetVariableExample.efi ByeVar nbr "Bye World"
Variable ByeVar was successfully changed
FS0:\> dmpstore -guid bb2a829f-7943-4691-a03a-f1f48519d7e6 -s MyVar.bin
Save variable to file: MyVar.bin.
Variable NV+RT+BS 'BB2A829F-7943-4691-A03A-F1F48519D7E6:ByeVar' DataSize = 0x16
Variable NV+BS 'BB2A829F-7943-4691-A03A-F1F48519D7E6:HelloVar' DataSize = 0x1A
```

Use hexedit to modify `World` string in both records of a dump file. Here I've just increased each letter code with 1.

Before:
```
00000000 0E 00 00 00 14 00 00 00  42 00 79 00 65 00 56 00  ........B.y.e.V.
00000010 61 00 72 00 00 00 9F 82  2A BB 43 79 91 46 A0 3A  a.r...??*?Cy?F?:
00000020 F1 F4 85 19 D7 E6 07 00  00 00 42 00 79 00 65 00  ???.??....B.y.e.
00000030 20 00 57 00 6F 00 72 00  6C 00 64 00 00 00 EC 24   .W.o.r.l.d...?$
00000040 78 CD 12 00 00 00 18 00  00 00 48 00 65 00 6C 00  x?........H.e.l.
00000050 6C 00 6F 00 56 00 61 00  72 00 00 00 9F 82 2A BB  l.o.V.a.r...??*?
00000060 43 79 91 46 A0 3A F1 F4  85 19 D7 E6 03 00 00 00  Cy?F?:???.??....
00000070 48 00 65 00 6C 00 6C 00  6F 00 20 00 57 00 6F 00  H.e.l.l.o. .W.o.
00000080 72 00 6C 00 64 00 00 00  97 82 10 13              r.l.d...??..
```
After:
```
00000000 0E 00 00 00 14 00 00 00  42 00 79 00 65 00 56 00  ........B.y.e.V.
00000010 61 00 72 00 00 00 9F 82  2A BB 43 79 91 46 A0 3A  a.r...??*?Cy?F?:
00000020 F1 F4 85 19 D7 E6 07 00  00 00 42 00 79 00 65 00  ???.??....B.y.e.
00000030 20 00 58 00 70 00 73 00  6D 00 65 00 00 00 EC 24   .X.p.s.m.e...?$
00000040 78 CD 12 00 00 00 18 00  00 00 48 00 65 00 6C 00  x?........H.e.l.
00000050 6C 00 6F 00 56 00 61 00  72 00 00 00 9F 82 2A BB  l.o.V.a.r...??*?
00000060 43 79 91 46 A0 3A F1 F4  85 19 D7 E6 03 00 00 00  Cy?F?:???.??....
00000070 48 00 65 00 6C 00 6C 00  6F 00 20 00 58 00 70 00  H.e.l.l.o. .X.p.
00000080 73 00 6D 00 65 00 00 00  97 82 10 13              s.m.e...??..
```

Use our program to update checksums:
```
FS0:\> UpdateDmpstoreDump.efi MyVar.bin
```

Now you can verify that our new dump indeed have changed both variables content:
```
FS0:\> dmpstore -guid bb2a829f-7943-4691-a03a-f1f48519d7e6
Variable NV+RT+BS 'BB2A829F-7943-4691-A03A-F1F48519D7E6:ByeVar' DataSize = 0x14
  00000000: 42 00 79 00 65 00 20 00-57 00 6F 00 72 00 6C 00  *B.y.e. .W.o.r.l.*
  00000010: 64 00 00 00                                      *d...*
Variable NV+BS 'BB2A829F-7943-4691-A03A-F1F48519D7E6:HelloVar' DataSize = 0x18
  00000000: 48 00 65 00 6C 00 6C 00-6F 00 20 00 57 00 6F 00  *H.e.l.l.o. .W.o.*
  00000010: 72 00 6C 00 64 00 00 00-                         *r.l.d...*

FS0:\> dmpstore -guid bb2a829f-7943-4691-a03a-f1f48519d7e6 -l MyVar.bin
Load and set variables from file: MyVar.bin.
Variable NV+RT+BS 'BB2A829F-7943-4691-A03A-F1F48519D7E6:ByeVar' DataSize = 0x14
Variable NV+BS 'BB2A829F-7943-4691-A03A-F1F48519D7E6:HelloVar' DataSize = 0x18

FS0:\> dmpstore -guid bb2a829f-7943-4691-a03a-f1f48519d7e6
Variable NV+BS 'BB2A829F-7943-4691-A03A-F1F48519D7E6:HelloVar' DataSize = 0x18
  00000000: 48 00 65 00 6C 00 6C 00-6F 00 20 00 58 00 70 00  *H.e.l.l.o. .X.p.*
  00000010: 73 00 6D 00 65 00 00 00-                         *s.m.e...*
Variable NV+RT+BS 'BB2A829F-7943-4691-A03A-F1F48519D7E6:ByeVar' DataSize = 0x14
  00000000: 42 00 79 00 65 00 20 00-58 00 70 00 73 00 6D 00  *B.y.e. .X.p.s.m.*
  00000010: 65 00 00 00                                      *e...*
```

Keep in mind that if you change size of the variable data, you need to change the `UINT32 DataSize` field as well.
