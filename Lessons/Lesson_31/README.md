In this lesson we would modify our `ListPCI` utility, so it would show us information about PCI Vendor and Device. Even `pci` command in UEFI shell doesn't show this information. It only shows us information about PCI class/subclass code. So our utility can be really usefull. Just in case you can check out sources for the `pci` command here:
- https://github.com/tianocore/edk2/blob/master/ShellPkg/Library/UefiShellDebug1CommandsLib/Pci.c
- https://github.com/tianocore/edk2/blob/master/ShellPkg/Library/UefiShellDebug1CommandsLib/Pci.h

This lesson was inspired by `ShowPCIx` application by fpmurphy https://github.com/fpmurphy/UEFI-Utilities-2019/tree/master/MyApps/ShowPCIx
Although this utility was taking too long time to do the parsing, so I've decided to rewrite it with a performance in mind.

In our app we would create a function `FindPCIDevDescription` that can fill Vendor/Device description strings based on its codes.
```
EFI_STATUS FindPCIDevDescription(IN UINT16 VendorId,
                                 IN UINT16 DeviceId,
                                 OUT CHAR16* VendorDesc,
                                 OUT CHAR16* DeviceDesc,
                                 IN UINTN DescBufferSize)
```

We integrate this function in a main PCI loop like this:
```
if (PCIConfHdr.VendorId != 0xffff) {
  Print(L"  %02x:%02x.%02x - Vendor:%04x, Device:%04x",
                                                          Bus,
                                                          Device,
                                                          Func,
                                                          PCIConfHdr.VendorId,
                                                          PCIConfHdr.DeviceId);

  CHAR16 VendorDesc[DESCRIPTOR_STR_MAX_SIZE];
  CHAR16 DeviceDesc[DESCRIPTOR_STR_MAX_SIZE];
  Status = FindPCIDevDescription(PCIConfHdr.VendorId,
                                 PCIConfHdr.DeviceId,
                                 VendorDesc,
                                 DeviceDesc,
                                 DESCRIPTOR_STR_MAX_SIZE);
  if (!EFI_ERROR(Status)) {
    Print(L":    %s, %s\n", VendorDesc, DeviceDesc);
  } else {
    Print(L"\n");
  }
}
```

In this code `DESCRIPTOR_STR_MAX_SIZE` is just a max size for the Vendor/Device description string. We declare `VendorDesc`/`DeviceDesc` as static arrays for simplicity and pick array size large enough to contain all descriptions.
```
#define DESCRIPTOR_STR_MAX_SIZE 200
```

Now it is time to write a function `FindPCIDevDescription`.

We can get PCI Vendor/Device information from a public PCI ID Repository https://pci-ids.ucw.cz/. This site hosts a file with publically known PCI Vendor/Device combinations https://pci-ids.ucw.cz/v2.2/pci.ids.

In the header of a file there are some comments about how the information is presented:
```
# Syntax:
# vendor  vendor_name
#	device  device_name				<-- single tab
#		subvendor subdevice  subsystem_name	<-- two tabs
```

Download this file to you shared QEMU directory:
```
$ cd ~/UEFI_disk
$ wget https://pci-ids.ucw.cz/v2.2/pci.ids
```

Now let's write our function.

First we need to check if PCI database file really exists:

For this task we can utilize a function from the `ShellLib` https://github.com/tianocore/edk2/blob/master/ShellPkg/Include/Library/ShellLib.h
```
/**
  Function to determine if a given filename exists.
  @param[in] Name         Path to test.
  @retval EFI_SUCCESS     The Path represents a file.
  @retval EFI_NOT_FOUND   The Path does not represent a file.
  @retval other           The path failed to open.
**/
EFI_STATUS
EFIAPI
ShellFileExists(
  IN CONST CHAR16 *Name
  );
```

So our check would be as simple as:
```
EFI_STATUS Status = ShellFileExists(L"pci.ids");
if (EFI_ERROR(Status))
{
  Print(L"No file pci.ids: %r\n", Status);
  return Status;
}
```

Next we need to open a file for read:
```
SHELL_FILE_HANDLE FileHandle;
Status = ShellOpenFileByName(L"pci.ids",
                             &FileHandle,
                             EFI_FILE_MODE_READ,
                             0);
if (EFI_ERROR(Status))
{
  Print(L"Can't open file pci.ids: %r\n", Status);
  return Status;
}
```

In our parsing process we would need a file size for a `pci.ids` file. Again we can use a function from the `ShellLib` for this task:
```
/**
  Retrieve the size of a file.
  This function extracts the file size info from the FileHandle's EFI_FILE_INFO
  data.
  @param[in] FileHandle         The file handle from which size is retrieved.
  @param[out] Size              The pointer to size.
  @retval EFI_SUCCESS           The operation was completed sucessfully.
  @retval EFI_DEVICE_ERROR      Cannot access the file.
**/
EFI_STATUS
EFIAPI
ShellGetFileSize (
  IN SHELL_FILE_HANDLE          FileHandle,
  OUT UINT64                    *Size
  );
```

There are a lot of things that can go wrong in our parsing process, but no matter what we should close an opened file handle. The easiest way to go from any point of the function to a specific place is a `goto` statement. We were taught that `goto` is always wrong, but actually in can be very handfull in some situations. It is used a lot in a Linux Kernel code for similar cleanup purposes, so don't argue and accept it:
```
EFI_STATUS FindPCIDevDescription(IN UINT16 VendorId,
                                 IN UINT16 DeviceId,
                                 OUT CHAR16* VendorDesc,
                                 OUT CHAR16* DeviceDesc,
                                 IN UINTN DescBufferSize)
{
  BOOLEAN Vendor_found = FALSE;
  BOOLEAN Device_found = FALSE;

  ...

  UINT64 FileSize;
  Status = ShellGetFileSize(FileHandle, &FileSize);
  if (EFI_ERROR(Status))
  {
    Print(L"Can't get file size for file pci.ids: %r\n", Status);
    goto end;
  }

  ...

end:
  if (!Vendor_found) {
    UnicodeSPrint(VendorDesc, DescBufferSize, L"Undefined");
  }
  if (!Device_found) {
    UnicodeSPrint(DeviceDesc, DescBufferSize, L"Undefined");
  }
  ShellCloseFile(&FileHandle);

  return Status;
}
```

When we would parse a database file we would be comparing char symbols, so we need to convert our `UINT16` value codes for Vendor and Device to hex strings.

For this task we can utilize `AsciiValueToStringS` function from the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PrintLib.h
```
/**
  Converts a decimal value to a Null-terminated Ascii string.
  Converts the decimal number specified by Value to a Null-terminated Ascii
  string specified by Buffer containing at most Width characters. No padding of
  spaces is ever performed. 
  ...
  If RADIX_HEX is set in Flags, then the output buffer will be formatted in
  hexadecimal format.
  ...
  If PREFIX_ZERO is set in Flags and PREFIX_ZERO is not being ignored, then
  Buffer is padded with '0' characters so the combination of the optional '-'
  sign character, '0' characters, digit characters for Value, and the
  Null-terminator add up to Width characters.
  If an error would be returned, then the function will ASSERT().
  @param  Buffer      The pointer to the output buffer for the produced
                      Null-terminated Ascii string.
  @param  BufferSize  The size of Buffer in bytes, including the
                      Null-terminator.
  @param  Flags       The bitmask of flags that specify left justification,
                      zero pad, and commas.
  @param  Value       The 64-bit signed value to convert to a string.
  @param  Width       The maximum number of Ascii characters to place in
                      Buffer, not including the Null-terminator.
  @retval RETURN_SUCCESS           The decimal value is converted.
  @retval RETURN_BUFFER_TOO_SMALL  If BufferSize cannot hold the converted
                                   value.
  @retval RETURN_INVALID_PARAMETER If Buffer is NULL.
                                   If PcdMaximumAsciiStringLength is not
                                   zero, and BufferSize is greater than
                                   PcdMaximumAsciiStringLength.
                                   If unsupported bits are set in Flags.
                                   If both COMMA_TYPE and RADIX_HEX are set in
                                   Flags.
                                   If Width >= MAXIMUM_VALUE_CHARACTERS.
**/
RETURN_STATUS
EFIAPI
AsciiValueToStringS (
  IN OUT CHAR8   *Buffer,
  IN UINTN       BufferSize,
  IN UINTN       Flags,
  IN INT64       Value,
  IN UINTN       Width
  );
```

We just need to create arrays large enough and use the correct flags (`RADIX_HEX | PREFIX_ZERO`):
```
CHAR8 VendorStr[5];
CHAR8 DeviceStr[5];
AsciiValueToStringS(VendorStr,
                    5,
                    RADIX_HEX | PREFIX_ZERO,
                    VendorId,
                    4);
AsciiValueToStringS(DeviceStr,
                    5,
                    RADIX_HEX | PREFIX_ZERO,
                    DeviceId,
                    4);
```

We need to make one more simple thing. `AsciiValueToStringS` would save hex value in upper-case, but in our database file it is written in lower-case. So we need to write a simple function for case conversion:
```
VOID ToLowerASCII(CHAR8* Str, UINTN Size)
{
  for (UINT8 i=0; i<Size; i++) {
    if ((Str[i]>='A')&&(Str[i]<='Z')) {
      Str[i]+=32;
    }
  }
}
```
And use it like this:
```
ToLowerASCII(VendorStr, 4);
ToLowerASCII(DeviceStr, 4);
```

Now the hard part, the main parsing loop:
```
CHAR8 Buffer[BLOCK_READ_SIZE];
UINTN Size;
UINT64 FilePos = 0;
while (TRUE)
{
  Size = BLOCK_READ_SIZE;
  Status = ShellReadFile(FileHandle, &Size, Buffer);
  if (EFI_ERROR(Status))
  {
    Print(L"Can't read file pci.ids: %r\n", Status);
    goto end;
  }
  UINTN StrStart = 0;
  UINTN StrEnd = 0;
  for (UINTN i=0; i<Size; i++) {
    if (Buffer[i]=='\n') {
      StrEnd=i;
      <...>
      StrStart=StrEnd;
    }
  }

  if (FilePos+Size >= FileSize) {
    break;
  }
  FilePos += StrEnd;
  Status = ShellSetFilePosition(FileHandle, FilePos);
  if (EFI_ERROR(Status))
  {
    Print(L"Can't set file position pci.ids: %r\n", Status);
    goto end;
  }
}
```
Couple of hints of what is happening here in the code:
- We read file by blocks (`#define BLOCK_READ_SIZE (1024*4)`),
- In each block we search for `\n` symbols, to fill variables `StrStart` and `StrEnd`, the actual search would be happening for the data between every two `\n` symbols,
- After the end of each block parsing we try to set a file pointer to the place of a last found `\n` (`=StrEnd`). For this task we utilize another function from the `ShellLib`:
```
EFI_STATUS
EFIAPI
ShellSetFilePosition (
  IN SHELL_FILE_HANDLE  FileHandle,
  IN UINT64             Position
  );
```
- If we've reached the end of a file and this was the last possible read, we end our search:
```
if (FilePos+Size >= FileSize) {
  break;
}
```

Now the thing that was hided behind `<...>`:
```
      if (!Vendor_found){
        // 0123456         7
        //\nVVVV  |<desc>|\n
        if ((StrEnd - StrStart) > 7) {
          if ((Buffer[StrStart+1]==VendorStr[0]) &&
              (Buffer[StrStart+2]==VendorStr[1]) &&
              (Buffer[StrStart+3]==VendorStr[2]) &&
              (Buffer[StrStart+4]==VendorStr[3])) {
            Buffer[StrEnd] = 0;
            UnicodeSPrintAsciiFormat(VendorDesc, DescBufferSize, "%a", &Buffer[StrStart+1+4+2]);
            Vendor_found = TRUE;
          }
        }
      } else {
        // 0 1234567         8
        //\n\tDDDD  |<desc>|\n
        if ((StrEnd - StrStart) > 8) {
          if ((Buffer[StrStart+1]=='\t') &&
              (Buffer[StrStart+2]==DeviceStr[0]) &&
              (Buffer[StrStart+3]==DeviceStr[1]) &&
              (Buffer[StrStart+4]==DeviceStr[2]) &&
              (Buffer[StrStart+5]==DeviceStr[3])) {
            Buffer[StrEnd] = 0;
            UnicodeSPrintAsciiFormat(DeviceDesc, DescBufferSize, "%a", &Buffer[StrStart+1+1+4+2]);
            Device_found = TRUE;
            goto end;
          }
        }
      }
```
Here is some explanation as well:
- if the vendor string wasn't found, search for its pattern, if it was, search for the device pattern,
- both `StrStart` and `StrEnd` point to different `\n` symbols, and we try to understand if information between them is what we need,
- I put some comments for the minimal format that we are looking for, for example:
```
// 0123456         7
//\nVVVV  |<desc>|\n
```
This means that if a `StrStart` is pointing to `\n` at (`i+0`), `StrEnd` should be at least poining to `\n` at 7 (`i+7`) as vendor description is take place in symbols (`i+1`-`i+4`), and after it there have to be exactly two spaces. So even if an actual description is empty, there have to be at least 8 symbols in our string,
- if we found our string we use `UnicodeSPrintAsciiFormat` to transform it to `CHAR16` string. This is another function from the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PrintLib.h
```
/**
  Produces a Null-terminated Unicode string in an output buffer based on a Null-terminated
  ASCII format string and  variable argument list.
  This function is similar as snprintf_s defined in C11.
  Produces a Null-terminated Unicode string in the output buffer specified by StartOfBuffer
  and BufferSize.
  The Unicode string is produced by parsing the format string specified by FormatString.
  Arguments are pulled from the variable argument list based on the contents of the
  format string.
  ...
  @param  StartOfBuffer   A pointer to the output buffer for the produced Null-terminated
                          Unicode string.
  @param  BufferSize      The size, in bytes, of the output buffer specified by StartOfBuffer.
  @param  FormatString    A Null-terminated ASCII format string.
  @param  ...             Variable argument list whose contents are accessed based on the
                          format string specified by FormatString.
  @return The number of Unicode characters in the produced output buffer not including the
          Null-terminator.
**/
UINTN
EFIAPI
UnicodeSPrintAsciiFormat (
  OUT CHAR16       *StartOfBuffer,
  IN  UINTN        BufferSize,
  IN  CONST CHAR8  *FormatString,
  ...
  );
```

I hope I've explained everything. Don't forget to add necessary includes for the libraries that we've used:
```
#include <Library/ShellLib.h>
#include <Library/PrintLib.h>
```
As well as put `ShellLib` to our `*.inf` file:
```
[Packages]
  ...
  ShellPkg/ShellPkg.dec

[LibraryClasses]
  ...
  ShellLib
```

If you build and execute our app under OVMF now, you would get:
```
FS0:\> ListPCI.efi
Number of PCI root bridges in the system: 1

PCI Root Bridge 0
  00:00.00 - Vendor:8086, Device:1237:    Intel Corporation, 440FX - 82441FX PMC [Natoma]
  00:01.00 - Vendor:8086, Device:7000:    Intel Corporation, 82371SB PIIX3 ISA [Natoma/Triton II]
  00:01.01 - Vendor:8086, Device:7010:    Intel Corporation, 82371SB PIIX3 IDE [Natoma/Triton II]
  00:01.03 - Vendor:8086, Device:7113:    Intel Corporation, 82371AB/EB/MB PIIX4 ACPI
  00:02.00 - Vendor:1234, Device:1111:    Undefined, Undefined
```

In case you wonder what is that misterious 1234/1111 device it is a QEMU VGA controller https://github.com/qemu/qemu/blob/master/docs/specs/standard-vga.txt

# Add PCI expander bridges and PCI root bridges to QEMU machine

We've used this command to run QEMU:
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=../OVMF_VARS.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk \
  -nographic \
  -net none
```
But you can actually provide various PCI expander bridges and PCI root bridges in this command:
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=../OVMF_VARS.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk \
  -nographic \
  -net none \
  -device pci-bridge,id=bridge0,chassis_nr=1 \
  -device virtio-scsi-pci,id=scsi0,bus=bridge0,addr=0x3 \
  -device pci-bridge,id=bridge1,chassis_nr=2 \
  -device virtio-scsi-pci,id=scsi1,bus=bridge1,addr=0x3 \
  -device virtio-scsi-pci,id=scsi2,bus=bridge1,addr=0x4 \
  -device pxb,id=bridge2,bus=pci.0,bus_nr=3 \
  -device virtio-scsi-pci,bus=bridge2,addr=0x3 \
  -device pxb,id=bridge3,bus=pci.0,bus_nr=8 \
  -device virtio-scsi-pci,bus=bridge3,addr=0x3 \
  -device virtio-scsi-pci,bus=bridge3,addr=0x4
```
On this system our command will produce this output:
```
FS0:\> ListPCI.efi
Number of PCI root bridges in the system: 3

PCI Root Bridge 0
  00:00.00 - Vendor:8086, Device:1237:    Intel Corporation, 440FX - 82441FX PMC [Natoma]
  00:01.00 - Vendor:8086, Device:7000:    Intel Corporation, 82371SB PIIX3 ISA [Natoma/Triton II]
  00:01.01 - Vendor:8086, Device:7010:    Intel Corporation, 82371SB PIIX3 IDE [Natoma/Triton II]
  00:01.03 - Vendor:8086, Device:7113:    Intel Corporation, 82371AB/EB/MB PIIX4 ACPI
  00:02.00 - Vendor:1234, Device:1111:    Undefined, Undefined
  00:03.00 - Vendor:1B36, Device:0001:    Red Hat, Inc., QEMU PCI-PCI bridge
  00:04.00 - Vendor:1B36, Device:0001:    Red Hat, Inc., QEMU PCI-PCI bridge
  00:05.00 - Vendor:1B36, Device:0009:    Red Hat, Inc., QEMU PCI Expander bridge
  00:06.00 - Vendor:1B36, Device:0009:    Red Hat, Inc., QEMU PCI Expander bridge
  01:03.00 - Vendor:1AF4, Device:1004:    Red Hat, Inc., Virtio SCSI
  02:03.00 - Vendor:1AF4, Device:1004:    Red Hat, Inc., Virtio SCSI
  02:04.00 - Vendor:1AF4, Device:1004:    Red Hat, Inc., Virtio SCSI

PCI Root Bridge 1
  03:00.00 - Vendor:1B36, Device:0001:    Red Hat, Inc., QEMU PCI-PCI bridge
  04:03.00 - Vendor:1AF4, Device:1004:    Red Hat, Inc., Virtio SCSI

PCI Root Bridge 2
  08:00.00 - Vendor:1B36, Device:0001:    Red Hat, Inc., QEMU PCI-PCI bridge
  09:03.00 - Vendor:1AF4, Device:1004:    Red Hat, Inc., Virtio SCSI
  09:04.00 - Vendor:1AF4, Device:1004:    Red Hat, Inc., Virtio SCSI
```

On a QEMU `q35` machine you can even add PCI-express root complexes:
```
qemu-system-x86_64 \
  -machine q35 \
  -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=../OVMF_VARS.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk \
  -nographic \
  -net none \
  -device pxb-pcie,id=pcie.1,bus_nr=2,bus=pcie.0 \
  -device ioh3420,id=pcie_port1,bus=pcie.1,chassis=1 \
  -device virtio-scsi-pci,bus=pcie_port1 \
  -device ioh3420,id=pcie_port2,bus=pcie.1,chassis=2 \
  -device virtio-scsi-pci,bus=pcie_port2 \
  -device pxb-pcie,id=pcie.2,bus_nr=8,bus=pcie.0 \
  -device ioh3420,id=pcie_port3,bus=pcie.2,chassis=3 \
  -device virtio-scsi-pci,bus=pcie_port3
```
```
FS0:\> ListPCI.efi
Number of PCI root bridges in the system: 3

PCI Root Bridge 0
  00:00.00 - Vendor:8086, Device:29C0:    Intel Corporation, 82G33/G31/P35/P31 Express DRAM Controller
  00:01.00 - Vendor:1234, Device:1111:    Undefined, Undefined
  00:02.00 - Vendor:1B36, Device:000B:    Red Hat, Inc., QEMU PCIe Expander bridge
  00:03.00 - Vendor:1B36, Device:000B:    Red Hat, Inc., QEMU PCIe Expander bridge
  00:1F.00 - Vendor:8086, Device:2918:    Intel Corporation, 82801IB (ICH9) LPC Interface Controller
  00:1F.02 - Vendor:8086, Device:2922:    Intel Corporation, 82801IR/IO/IH (ICH9R/DO/DH) 6 port SATA Controller [AHCI mode]
  00:1F.03 - Vendor:8086, Device:2930:    Intel Corporation, 82801I (ICH9 Family) SMBus Controller

PCI Root Bridge 1
  02:00.00 - Vendor:8086, Device:3420:    Intel Corporation, 7500/5520/5500/X58 I/O Hub PCI Express Root Port 0
  02:01.00 - Vendor:8086, Device:3420:    Intel Corporation, 7500/5520/5500/X58 I/O Hub PCI Express Root Port 0
  03:00.00 - Vendor:1AF4, Device:1048:    Red Hat, Inc., Virtio SCSI
  04:00.00 - Vendor:1AF4, Device:1048:    Red Hat, Inc., Virtio SCSI

PCI Root Bridge 2
  08:00.00 - Vendor:8086, Device:3420:    Intel Corporation, 7500/5520/5500/X58 I/O Hub PCI Express Root Port 0
  09:00.00 - Vendor:1AF4, Device:1048:    Red Hat, Inc., Virtio SCSI
```

If you are interested check out this link to know more about all these QEMU parameters https://blogs.oracle.com/linux/post/a-study-of-the-linux-kernel-pci-subsystem-with-qemu

# UTF-8

In this lesson we've parsed `pci.ids` file as an ASCII file, but actually it is encoded in UTF-8.

```
$ file ~/UEFI_disk/pci.ids
/home/kostr/UEFI_disk/pci.ids: UTF-8 Unicode text, with very long lines
```

But as `pci.ids` file consists mostly from ASCII symbols it was fine to treat it as an ASCII.

We've used this simplification because it is hard to parse UTF-8 data in UEFI since it doesn't have any native support for this encoding.

The only way to parse UTF-8 is to deserialize the UTF-8 to Unicode and then serialize that to UCS-2 (CHAR16). If you are really want to do it, you can utilize some conversion code from the terminal driver (https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/Console/TerminalDxe/Vtutf8.c).

