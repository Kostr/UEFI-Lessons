Let's investigate final protocol that was attached to our `ImageHandle` - `EFI_SHELL_PARAMETERS_PROTOCOL`

https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/ShellParameters.h
```
typedef struct _EFI_SHELL_PARAMETERS_PROTOCOL {
  ///
  /// Points to an Argc-element array of points to NULL-terminated strings containing
  /// the command-line parameters. The first entry in the array is always the full file
  /// path of the executable. Any quotation marks that were used to preserve
  /// whitespace have been removed.
  ///
  CHAR16 **Argv;

  ///
  /// The number of elements in the Argv array.
  ///
  UINTN Argc;

  ///
  /// The file handle for the standard input for this executable. This may be different
  /// from the ConInHandle in EFI_SYSTEM_TABLE.
  ///
  SHELL_FILE_HANDLE StdIn;

  ///
  /// The file handle for the standard output for this executable. This may be different
  /// from the ConOutHandle in EFI_SYSTEM_TABLE.
  ///
  SHELL_FILE_HANDLE StdOut;

  ///
  /// The file handle for the standard error output for this executable. This may be
  /// different from the StdErrHandle in EFI_SYSTEM_TABLE.
  ///
  SHELL_FILE_HANDLE StdErr;
} EFI_SHELL_PARAMETERS_PROTOCOL;
```

As we see, we can access command line arguments that was passed to our program through this protocol.
Let's use it in our `MemoryInfo` program.

In the last lesson we've printed our EFI memory map. It had >100 entries.
When you boot Linux kernel, you can see some info about the current memory map, but this table is much shorter. It happens because of two facts:
- Kernel differentiate EFI memory types much less granular. Instead of `EfiReservedMemoryType`/`EfiLoaderCode`/`EfiLoaderData`/...` it simply has only 4 types: `usable`/`ACPI NVS`/`ACPI data`/`reserved`
- Kernel glues adjacent regions together


I've generated kernel image for EFI x86-64 with buildroot:
```
cd ~
git clone https://github.com/buildroot/buildroot.git
cd buildroot
make pc_x86_64_efi_defconfig
make
```

If we try to boot this kernel with:
```
qemu-system-x86_64 -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
                   -drive format=raw,file=fat:rw:~/UEFI_disk \
                   -nographic \
                   -kernel ~/buildroot/output/images/bzImage \
                   -append "console=ttyS0"
```

In kernel boot log we can see:
```
BIOS-provided physical RAM map:
BIOS-e820: [mem 0x0000000000000000-0x000000000009ffff] usable
BIOS-e820: [mem 0x0000000000100000-0x00000000007fffff] usable
BIOS-e820: [mem 0x0000000000800000-0x0000000000807fff] ACPI NVS
BIOS-e820: [mem 0x0000000000808000-0x000000000080ffff] usable
BIOS-e820: [mem 0x0000000000810000-0x00000000008fffff] ACPI NVS
BIOS-e820: [mem 0x0000000000900000-0x00000000078eefff] usable
BIOS-e820: [mem 0x00000000078ef000-0x0000000007b6efff] reserved
BIOS-e820: [mem 0x0000000007b6f000-0x0000000007b7efff] ACPI data
BIOS-e820: [mem 0x0000000007b7f000-0x0000000007bfefff] ACPI NVS
BIOS-e820: [mem 0x0000000007bff000-0x0000000007ef3fff] usable
BIOS-e820: [mem 0x0000000007ef4000-0x0000000007f77fff] reserved
BIOS-e820: [mem 0x0000000007f78000-0x0000000007ffffff] ACPI NVS
BIOS-e820: [mem 0x00000000ffc00000-0x00000000ffffffff] reserved
```

Let's modify our `MemoryInfo` program:
- if `full` option is passed, we print memory map as we do now
- if no option is passed, we print memory map in a "Linux kernel way"

First, add `full` boolean flag. If argument "full" is passed to our program, we'll set this flag, else it would be equal to `false`.
```
EFI_SHELL_PARAMETERS_PROTOCOL* ShellParameters;

Status = gBS->HandleProtocol(
  ImageHandle,
  &gEfiShellParametersProtocolGuid,
  (VOID **) &ShellParameters
);

BOOLEAN full=FALSE;
if (Status == EFI_SUCCESS) {
  if (ShellParameters->Argc == 2) {
    if (!StrCmp(ShellParameters->Argv[1], L"full")) {
      full=TRUE;
    }
  }
}
```

To use `EFI_SHELL_PARAMETERS_PROTOCOL` we need to add include file:
```
#include <Protocol/ShellParameters.h>
```
And add GUID to the application *.inf file:
```
[Protocols]
  gEfiShellParametersProtocolGuid
```

Now to the next problem. Create a function for OS memory type mapping:
```
const CHAR16 *memory_types_OS_view[] = {
    L"reserved", // L"EfiReservedMemoryType",
    L"usable",   // L"EfiLoaderCode",
    L"usable",   // L"EfiLoaderData",
    L"usable",   // L"EfiBootServicesCode",
    L"usable",   // L"EfiBootServicesData",
    L"reserved", // L"EfiRuntimeServicesCode",
    L"reserved", // L"EfiRuntimeServicesData",
    L"usable",   // L"EfiConventionalMemory",
    L"reserved", // L"EfiUnusableMemory",
    L"ACPI data",// L"EfiACPIReclaimMemory",
    L"ACPI NVS", // L"EfiACPIMemoryNVS",
    L"reserved", // L"EfiMemoryMappedIO",
    L"reserved", // L"EfiMemoryMappedIOPortSpace",
    L"reserved", // L"EfiPalCode",
    L"usable",   // L"EfiPersistentMemory",
    L"usable",   // L"EfiMaxMemoryType"
};

const CHAR16 *
memory_type_to_str_OS_view(UINT32 type)
{
    if (type > sizeof(memory_types_OS_view)/sizeof(CHAR16 *))
        return L"Unknown";

    return memory_types_OS_view[type];
}
```

And finally we need to modify our program to glue adjacent regions with the same type together if the `full` flag is not set:
```
EFI_MEMORY_DESCRIPTOR* desc = MemoryMap;
EFI_MEMORY_DESCRIPTOR* next_desc;
int i = 0;
while ((UINT8 *)desc < (UINT8 *)MemoryMap + MemoryMapSize) {
  UINTN PAGE_SIZE = 4096;
  UINTN mapping_size =(UINTN) desc->NumberOfPages * PAGE_SIZE;

  UINT64 Start = desc->PhysicalStart;

  next_desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)desc + DescriptorSize);
  if (!full) {
    while ((UINT8 *)next_desc < (UINT8 *)MemoryMap + MemoryMapSize) {
      mapping_size =(UINTN) desc->NumberOfPages * PAGE_SIZE;
      if ((desc->PhysicalStart + mapping_size) == (next_desc->PhysicalStart)) {

        if (desc->Type != next_desc->Type) {
          if (StrCmp(memory_type_to_str_OS_view(desc->Type),
                     memory_type_to_str_OS_view(next_desc->Type)))
            break;
          }

          desc=next_desc;
          next_desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)next_desc + DescriptorSize);
      } else {
          break;
      }
    }
  }

  if (full) {
    CHAR16 str[ATTRIBUTE_STR_SIZE];
    Print(L"[#%02d] Type: %s  Attr: %s\n", i++,
      memory_type_to_str(desc->Type), memory_attrs_to_str(str, desc->Attribute));
    Print(L"      Phys: %016llx-%016llx\n", Start, Start + mapping_size - 1);
  }
  else {
    Print(L" [mem: %016llx-%016llx] %s\n", Start, desc->PhysicalStart + mapping_size - 1,
      memory_type_to_str_OS_view(desc->Type) );
  }

  desc = next_desc;
}
```

Build program and copy it to UEFI folder.
If we run in with the `full` option, everything would be like the last time:
```
FS0:\> MemoryInfo.efi full
[#00] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000000000000-0000000000000FFF
[#01] Type: EfiConventionalMemory  Attr:  UC WC WT WB
      Phys: 0000000000001000-000000000009FFFF
[#02] Type: EfiConventionalMemory  Attr:  UC WC WT WB
      Phys: 0000000000100000-00000000007FFFFF
[#03] Type: EfiACPIMemoryNVS  Attr:  UC WC WT WB
      Phys: 0000000000800000-0000000000807FFF
[#04] Type: EfiConventionalMemory  Attr:  UC WC WT WB
      Phys: 0000000000808000-000000000080FFFF
[#05] Type: EfiACPIMemoryNVS  Attr:  UC WC WT WB
      Phys: 0000000000810000-00000000008FFFFF
...
```

But if we run it without the `full` option, we will get a map similar to the that kernel displays in its boot log:
```
FS0:\> MemoryInfo.efi
 [mem: 0000000000000000-000000000009FFFF] usable
 [mem: 0000000000100000-00000000007FFFFF] usable
 [mem: 0000000000800000-0000000000807FFF] ACPI NVS
 [mem: 0000000000808000-000000000080FFFF] usable
 [mem: 0000000000810000-00000000008FFFFF] ACPI NVS
 [mem: 0000000000900000-00000000078EEFFF] usable
 [mem: 00000000078EF000-0000000007B6EFFF] reserved
 [mem: 0000000007B6F000-0000000007B7EFFF] ACPI data
 [mem: 0000000007B7F000-0000000007BFEFFF] ACPI NVS
 [mem: 0000000007BFF000-0000000007EF3FFF] usable
 [mem: 0000000007EF4000-0000000007F77FFF] reserved
 [mem: 0000000007F78000-0000000007FFFFFF] ACPI NVS
 [mem: 00000000FFC00000-00000000FFFFFFFF] reserved
```
Compare it with the actual kernel output:
```
BIOS-provided physical RAM map:
BIOS-e820: [mem 0x0000000000000000-0x000000000009ffff] usable
BIOS-e820: [mem 0x0000000000100000-0x00000000007fffff] usable
BIOS-e820: [mem 0x0000000000800000-0x0000000000807fff] ACPI NVS
BIOS-e820: [mem 0x0000000000808000-0x000000000080ffff] usable
BIOS-e820: [mem 0x0000000000810000-0x00000000008fffff] ACPI NVS
BIOS-e820: [mem 0x0000000000900000-0x00000000078eefff] usable
BIOS-e820: [mem 0x00000000078ef000-0x0000000007b6efff] reserved
BIOS-e820: [mem 0x0000000007b6f000-0x0000000007b7efff] ACPI data
BIOS-e820: [mem 0x0000000007b7f000-0x0000000007bfefff] ACPI NVS
BIOS-e820: [mem 0x0000000007bff000-0x0000000007ef3fff] usable
BIOS-e820: [mem 0x0000000007ef4000-0x0000000007f77fff] reserved
BIOS-e820: [mem 0x0000000007f78000-0x0000000007ffffff] ACPI NVS
BIOS-e820: [mem 0x00000000ffc00000-0x00000000ffffffff] reserved
```
