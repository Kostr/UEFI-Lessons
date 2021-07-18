In this lesson we would talk about how to debug our UEFI application and drivers with GDB.

To perfrom debug with GDB we need to load debug symbols at the correct offset, for it we need to know:
- app offset in memory
- internal app sections offsets

# Getting application offset in memory

Run your QEMU with debug image of OVMF and log functionality enabled:
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk \
  -net none \
  -nographic \
  -global isa-debugcon.iobase=0x402 \
  -debugcon file:debug.log
```
In a separate terminal window execute:
```
tail -f debug.log
```
This way you can follow log runtime.

Wait for UEFI shell boot and execute our `ShowBootVariables.efi` application. After you do it, something similar to these messages would be produced to `debug.log`:
```
Loading driver at 0x00006649000 EntryPoint=0x0000664C80F ShowBootVariables.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 666B898
ProtectUefiImageCommon - 0x666B4C0
  - 0x0000000006649000 - 0x0000000000005540
InstallProtocolInterface: 752F3136-4E16-4FDC-A22A-E5F46812F4CA 7EA36C8
```

Right now we are interested in this string:
```
Loading driver at 0x00006649000 EntryPoint=0x0000664C80F ShowBootVariables.efi
```
Our image is loaded to `0x6649000`.

# Getting application sections offset

Use `objdump` to see what sections are inside `.efi` executable:
```
$ objdump -h Build/UefiLessonsPkg/DEBUG_GCC5/X64/ShowBootVariables.efi

Build/UefiLessonsPkg/DEBUG_GCC5/X64/ShowBootVariables.efi:     file format pei-x86-64

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .text         00004d00  0000000000000240  0000000000000240  00000240  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .data         00000580  0000000000004f40  0000000000004f40  00004f40  2**4
                  CONTENTS, ALLOC, LOAD, DATA
  2 .reloc        00000080  00000000000054c0  00000000000054c0  000054c0  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
```
And what sections are inside `.debug` version:
```
$ objdump -h Build/UefiLessonsPkg/DEBUG_GCC5/X64/ShowBootVariables.debug

Build/UefiLessonsPkg/DEBUG_GCC5/X64/ShowBootVariables.debug:     file format elf64-x86-64

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .text         00004cf5  0000000000000240  0000000000000240  000000c0  2**6
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  1 .data         000004a1  0000000000004f40  0000000000004f40  00004dc0  2**6
                  CONTENTS, ALLOC, LOAD, RELOC, DATA
  2 .eh_frame     00000000  0000000000005400  0000000000005400  00005280  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  3 .rela         00000510  0000000000005400  0000000000005400  00005280  2**3
                  CONTENTS, READONLY
  4 .build-id     00000024  0000000000005910  0000000000005910  00005790  2**2
                  CONTENTS, READONLY
  5 .debug_info   000283a4  0000000000000000  0000000000000000  000057b4  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING, OCTETS
  6 .debug_abbrev 00003635  0000000000000000  0000000000000000  0002db58  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  7 .debug_loc    00009fc9  0000000000000000  0000000000000000  0003118d  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING, OCTETS
  8 .debug_aranges 00000570  0000000000000000  0000000000000000  0003b156  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING, OCTETS
  9 .debug_ranges 000013f0  0000000000000000  0000000000000000  0003b6c6  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING, OCTETS
 10 .debug_line   00007a5b  0000000000000000  0000000000000000  0003cab6  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING, OCTETS
 11 .debug_str    000069e4  0000000000000000  0000000000000000  00044511  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 12 .debug_frame  000015a8  0000000000000000  0000000000000000  0004aef8  2**3
                  CONTENTS, RELOC, READONLY, DEBUGGING, OCTETS
```
As you can see `*.efi` file is a version without any `.debug*` sections.

Right now we are interested in `File off` column of the image that is getting executed - `ShowBootVariables.efi`.

- `.text` section has offset `0x240`
- `.data` section has offset `0x4f40`

# Loading debug symbols to GDB

First of all, to make remote debugging possible we have to provide `-s` parameter for QEMU. This is a shorthand for `tcp::1234`. QEMU will listen for GDB connections on this port.
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk \
  -net none \
  -nographic \
  -global isa-debugcon.iobase=0x402 \
  -debugcon file:debug.log \
  -s
```

Now we need to calculate addresses for `.text` and `.data` sections:
```
0x6649000 +  0x240 = 0x6649240
0x6649000 + 0x4f40 = 0x664df40
```

Run GDB and load symbols into it:
```
$ gdb
(gdb) add-symbol-file Build/UefiLessonsPkg/DEBUG_GCC5/X64/ShowBootVariables.debug 0x6649240 -s .data 0x664df40
add symbol table from file "Build/UefiLessonsPkg/DEBUG_GCC5/X64/ShowBootVariables.debug" at
        .text_addr = 0x6649240
        .data_addr = 0x664df40
(y or n) y
Reading symbols from Build/UefiLessonsPkg/DEBUG_GCC5/X64/ShowBootVariables.debug...
```

The entry point of our application is `ShellAppMain` function. In a separate terminal find the string in a file where it is defined:
```
$ grep -n ShellAppMain UefiLessonsPkg/ShowBootVariables/ShowBootVariables.c
67:INTN EFIAPI ShellAppMain(IN UINTN Argc, IN CHAR16 **Argv)
```

Set breakpint at that place:
```
(gdb) b UefiLessonsPkg/ShowBootVariables/ShowBootVariables.c:67
Breakpoint 1 at 0x664c50f: file /home/kostr/tiano/edk2/UefiLessonsPkg/ShowBootVariables/ShowBootVariables.c, line 68.
```

Connect to the remote:
```
(gdb) target remote :1234
Remote debugging using :1234
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x000000000717d841 in ?? ()
```

This will stop QEMU, so we need to execute `c` (=continue) to make QEMU interactive again:
```
(gdb) c
Continuing.
```
Now GDB is waiting for a breakpoint hit.

Swith to QEMU terminal and run our app
```
FS0:\> ShowBootVariables.efi
```
On GDB you should receive a breakpoint hit:
```
Breakpoint 1, ShellAppMain (Argc=1, Argv=0x666a398) at /home/kostr/tiano/edk2/UefiLessonsPkg/ShowBootVariables/ShowBootVariables.c:73
73        Status = GetNvramVariable(L"BootCurrent", &gEfiGlobalVariableGuid, (VOID**)&BootCurrent, &OptionSize);
(gdb)
```

![gdb](gdb.png?raw=true "gdb")

To ease GDB development I suggest you to try TUI mode (https://sourceware.org/gdb/onlinedocs/gdb/TUI.html):
```
(gdb) tui enable
```
In this mode source code file would be displayed right above the GDB prompt.

![gdb_tui](gdb_tui.png?raw=true "gdb_tui")




