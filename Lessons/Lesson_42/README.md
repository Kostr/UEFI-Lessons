In this lesson we would talk about how to debug our UEFI applications and drivers with GDB.

To perfrom debug with GDB we need to load debug symbols at the correct offset, for it we need to know:
- app offset in memory
- internal app sections offsets

As example in this lesson we would try to debug `ShowBootVariables.efi` application that we've developed earlier.

# Getting application offset in memory

Run your QEMU with the debug image of OVMF and log functionality enabled:
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
$ objdump -h Build/UefiLessonsPkg/RELEASE_GCC5/X64/ShowBootVariables.efi

Build/UefiLessonsPkg/RELEASE_GCC5/X64/ShowBootVariables.efi:     file format pei-x86-64

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
$ objdump -h Build/UefiLessonsPkg/RELEASE_GCC5/X64/ShowBootVariables.debug

Build/UefiLessonsPkg/RELEASE_GCC5/X64/ShowBootVariables.debug:     file format elf64-x86-64

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
  -s
```

Now we need to calculate addresses for `.text` and `.data` sections:
```
0x6649000 +  0x240 = 0x6649240
0x6649000 + 0x4f40 = 0x664df40
```

Run GDB and load symbols into it at calculated offsets:
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


# efi.py

All this proccess of debug setup is very tedious.

To ease a proccess of address calculation we could use [efi.py](efi.py) python script by Artem Nefedov.

This script helps to load debug symbols into GDB (https://github.com/artem-nefedov/uefi-gdb/blob/master/efi.py).

You can load this script into GDB:
```
gdb -ex 'source efi.py'
```
After that you would have custom `efi` command in GDB.

We can use like this:
```
(gdb) efi -64 ShowBootVariables
Turning pagination off
Using pre-defined driver list: ['ShowBootVariables']
The target architecture is assumed to be i386:x86-64:intel
With architecture X64
Looking for addresses in debug.log
EFI file Build/UefiLessonsPkg/DEBUG_GCC5/X64/ShowBootVariables.efi
 Base address 0x00006649000
.text address 0x0000000000000240
.data address 0x0000000000004f40
add symbol table from file "Build/UefiLessonsPkg/DEBUG_GCC5/X64/ShowBootVariables.debug" at
        .text_addr = 0x6649240
        .data_addr = 0x664df40
Restoring pagination

```
This would search `debug.log` file (so it should be already present) for `Loading driver at <...> EntryPoint=<...> ShowBootVariables.efi` string, get address information from it and
load all the necessary symbols into GDB at the correct addresses.

After that all you need to do is run QEMU in a separate terminal:

```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk \
  -net none \
  -nographic \
  -s
```
And in GDB terminal create a breakpoint and connect to QEMU as before:
```
(gdb) target remote :1234
Remote debugging using :1234
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x000000000717d841 in ?? ()
(gdb) b UefiLessonsPkg/ShowBootVariables/ShowBootVariables.c:67
Breakpoint 1 at 0x664c50f: file /home/kostr/tiano/edk2/UefiLessonsPkg/ShowBootVariables/ShowBootVariables.c, line 68.
(gdb) c
Continuing.
```
After that running our app in QEMU would produce breakpoint hit like before:
```
Breakpoint 1, ShellAppMain (Argc=1, Argv=0x666ae18) at /home/kostr/tiano/edk2/UefiLessonsPkg/ShowBootVariables/ShowBootVariables.c:73
73        Status = GetNvramVariable(L"BootCurrent", &gEfiGlobalVariableGuid, (VOID**)&BootCurrent, &OptionSize);
```

# run_gdb.sh

The `efi.py` script is good, but we still have much to do:
- we need to create `debug.log` file that would contain necessary string. For this we need to launch QEMU with a correct parameters, and after boot launch our app in QEMU. We need to do it every time after recompilation of our app,
- we need to run GDB with a correct arguments to load `efi.py`, and then execute `efi` command with a correct arguments,
- we need to search entry point of our function to set a first breakpoint,
- we need to run QEMU in a separate terminal with a correct parameters,
- we need to connect GDB to QEMU,
- we need to run our app in QEMU.

That is too much to do, and we wasn't even talking about a fact, that it would be good to see OVMF log at the same time as we debug.

To automate all these tasks I've created a `run_gdb.sh` script that creates `tmux` session with different panes, and automatically send necessay keys to them.

```
$ ./run_gdb.sh -h
Description:
  run_gdb.sh is a script that helps to debug UEFI shell applications and drivers

Usage: run_gdb.sh -m <module> [-1|-f|-p <package>|-q <dir>]
  -1            This is a first run of this configuration
                (in this case before main gdb launch there would be another QEMU start that will create 'debug.log' file)
  -f            Load all debug symbols
                (this will load all OVMF debug symbols - with this you could step inside OVMF functions)
  -m <module>   UEFI module to debug
  -p <package>  UEFI package to debug
                (by default it is equal to PACKAGE variable in the head of the script)
  -q <dir>      QEMU shared directory
                (by default it is equal to QEMU_SHARED_FOLDER variable in the head of the script)

Examples:
 run_gdb.sh -1 -m MyApp      - create 'debug.log' file with the necessary address information for the 'MyApp'
                               and debug it with gdb
 run_gdb.sh -1 -m MyApp -f   - create 'debug.log' file with the necessary address information for the 'MyApp'
                               and debug it with gdb (all debug symbols are included, i.e. you can step into OVMF functions)
 run_gdb.sh -m MyApp         - debug 'MyApp' with gdb ('debug.log' was created in the last run, no need to remake it again)
```

If we running our debug for the first time, we need to create `debug.log` with necessary information. For this run:
```
./run_gdb.sh -1 -m ShowBootVariables
```

This would launch `tmux` window with a OVMF log in the upper pane and QEMU in the lower pane:
![run_gdb_first_time_1](run_gdb_first_time_1.png?raw=true "run_gdb_first_time_1")

You just need to hit `Enter` to load your app:
![run_gdb_first_time_2](run_gdb_first_time_2.png?raw=true "run_gdb_first_time_2")

Now press `Ctrl+b` and then type `:kill-session` to close tmux session.

After that another tmux session will be launched:
- at left side there will be GDB in tui mode
- at upper right corner there will be OVMF log
- at lower right corner there will be QEMU

![run_gdb_1](run_gdb_1.png?raw=true "run_gdb_1")

Again once GDB loads all the necessary info, all you need to do is press `Enter` in QEMU pane.

![run_gdb_2](run_gdb_2.png?raw=true "run_gdb_2")

# Load all symbols

If you want to step in functions, that were defined outside your application/driver (for example `gRT->GetVariable`) you need to load all the symbols into GDB from the `debug.log`.

For this case you need to run `run_gdb.sh` script with `-f` argument:
```
./run_gdb.sh -1 -m ShowBootVariables -f
```
With this you can step inside most of the OVMF functions.
![run_gdb_full](run_gdb_full.png?raw=true "run_gdb_full")

One caution, don't hit `Enter` in QEMU pane, before GDB has loaded all the symbols.

# Debug OVMF itself

To debug OVMF itself we need to stop QEMU at start. For this task there is a `-S` parameter:
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk \
  -net none \
  -nographic \
  -s \
  -S
```

Now we need to run GDB, load all the symbols from previously created `debug.log` file and connect to QEMU.

Again, this is no very different from application debug, but it can be very tedious to setup.

So I've provided similar `run_gdb_ovmf.sh` script to ease this task.

```
$ ./run_gdb_ovmf.sh -h
Description:
  run_gdb_ovmf.sh is a script that helps to debug OVMF

Usage: run_gdb_ovmf.sh [-1] [-q <dir>]
  -1            This is a first run of this configuration
                (in this case before main gdb launch there would be another QEMU start that will create 'debug.log' file)
  -q <dir>      QEMU shared directory
                (by default it is equal to QEMU_SHARED_FOLDER variable in the head of the script)

Examples:
 run_gdb_ovmf.sh -1      - create 'debug.log' file with the necessary address information
                           and debug OVMF it with gdb
 run_gdb_ovmf.sh         - debug OVMF with gdb ('debug.log' was created in the last run, no need to remake it again)
```

The `-1` argument is no different from the `run_gdb.sh` script.

In the end `run_gdb_ovmf.sh` would provide similar `tmux` session. The only difference is that it will not set any breakpoints or run OVMF.

![run_gdb_ovmf](run_gdb_ovmf.png?raw=true "run_gdb_ovmf")


# Minimal GDB cheatsheet

- `s` - step
- `n` - next
- `fin` - step out
- `i loc` - info about local variables
- `i arg` - info about function arguments
- `p <var>` - print value of `<var>`
- `b <number>` - set breakpoint at line `<number>` in the current file
- `b <file>:<number>` - set breakpoint at line `<number>` in the `<file>` file
- `b <func>` - break on function `<func>`
- `i b` - info breakpoints
- `d <number>` - delete breakpoint `<number>`
- `c` - continue
- `q` - quit GDB
- `Ctrl+p` - previous GDB command in history
- `Ctrl+n` - next GDB command in history
- `Ctrl+x` and `o` - change active window in tui mode

To print variable value in hexadecimal use `p/x <variable>`.

To print CHAR16 string you can use `x /sh <addr>` command. Here is the example how can you print device path:
```
(gdb) p ConvertDevicePathToText(DevicePath, 0, 1)
$1 = (CHAR16 *) 0x6d04518
(gdb) x /sh 0x6d04518 
0x6d04518:      u"PciRoot(0x0)/Pci(0x2,0x0)"
```
You can even do it in one command:
```
(gdb) x /sh ConvertDevicePathToText(DevicePath, 0, 1)
0x6d02098:      u"PciRoot(0x0)/Pci(0x2,0x0)"
```

# Minimal Tmux cheatsheet

- `Ctrl+b` and `up/down/left/right` - switch between panes
- `Ctrl+b` and `:kill-session` - close all panes
- `Ctrl+b` and `Ctrl+up/down/left/right` - change pane size

