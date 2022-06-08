In this lesson we would talk about the `DEBUG` macro in edk2 codebase. Example of a `DEBUG` macro use:
```
DEBUG ((EFI_D_ERROR, "Hello Debug! Check this variable: %d\n", MyVar));
```
So the string formatting is similar to the `Print` function, but there are couple of benefits:
- log levels (`EFI_D_*`) help to categorize log messages. With the configuration through PCD we can easily turn on/off different types of debug messages
- there can be several implementations of the `DEBUG` functionality. Different UEFI stages/modules can have different `DEBUG` implementations

All predefined log message categories are defined in the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/DebugLib.h
```
#define DEBUG_INIT      0x00000001  // Initialization
#define DEBUG_WARN      0x00000002  // Warnings
#define DEBUG_LOAD      0x00000004  // Load events
#define DEBUG_FS        0x00000008  // EFI File system
#define DEBUG_POOL      0x00000010  // Alloc & Free (pool)
#define DEBUG_PAGE      0x00000020  // Alloc & Free (page)
#define DEBUG_INFO      0x00000040  // Informational debug messages
#define DEBUG_DISPATCH  0x00000080  // PEI/DXE/SMM Dispatchers
#define DEBUG_VARIABLE  0x00000100  // Variable
#define DEBUG_BM        0x00000400  // Boot Manager
#define DEBUG_BLKIO     0x00001000  // BlkIo Driver
#define DEBUG_NET       0x00004000  // Network Io Driver
#define DEBUG_UNDI      0x00010000  // UNDI Driver
#define DEBUG_LOADFILE  0x00020000  // LoadFile
#define DEBUG_EVENT     0x00080000  // Event messages
#define DEBUG_GCD       0x00100000  // Global Coherency Database changes
#define DEBUG_CACHE     0x00200000  // Memory range cachability changes
#define DEBUG_VERBOSE   0x00400000  // Detailed debug messages that may
                                    // significantly impact boot performance
#define DEBUG_ERROR     0x80000000  // Error
```
But generally these alias values are used:
```
//
// Aliases of debug message mask bits
//
#define EFI_D_INIT      DEBUG_INIT
#define EFI_D_WARN      DEBUG_WARN
#define EFI_D_LOAD      DEBUG_LOAD
#define EFI_D_FS        DEBUG_FS
#define EFI_D_POOL      DEBUG_POOL
#define EFI_D_PAGE      DEBUG_PAGE
#define EFI_D_INFO      DEBUG_INFO
#define EFI_D_DISPATCH  DEBUG_DISPATCH
#define EFI_D_VARIABLE  DEBUG_VARIABLE
#define EFI_D_BM        DEBUG_BM
#define EFI_D_BLKIO     DEBUG_BLKIO
#define EFI_D_NET       DEBUG_NET
#define EFI_D_UNDI      DEBUG_UNDI
#define EFI_D_LOADFILE  DEBUG_LOADFILE
#define EFI_D_EVENT     DEBUG_EVENT
#define EFI_D_VERBOSE   DEBUG_VERBOSE
#define EFI_D_ERROR     DEBUG_ERROR
```

The DEBUG macro itself is also defined in the `DebugLib` library header (https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/DebugLib.h).

If you'll split all the preprocessing hell it generally looks like this when it is used:
```
if (DebugPrintEnabled ()) {
  if (DebugPrintLevelEnabled (PrintLevel)) {
    DebugPrint (PrintLevel, ##__VA_ARGS__);
  }
}
```

The `DEBUG` macro interface is defined in a `DebugLib` header. But for the actual implementation we need to look at a particular library instance that is used.

If you look at the https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkgX64.dsc you'll see that:
- `DebugLib` has different implementations in different UEFI stages
- `DebugLib` has different implementations based on a `DEBUG_ON_SERIAL_PORT` definition
```
[LibraryClasses.common.SEC]
!ifdef $(DEBUG_ON_SERIAL_PORT)
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
!else
  DebugLib|OvmfPkg/Library/PlatformDebugLibIoPort/PlatformRomDebugLibIoPort.inf
!endif
```
```
[LibraryClasses.common.PEI_CORE]
# [LibraryClasses.common.PEIM]
# [LibraryClasses.common.DXE_CORE]
# [LibraryClasses.common.DXE_RUNTIME_DRIVER]
# [LibraryClasses.common.UEFI_DRIVER]
# [LibraryClasses.common.DXE_DRIVER]
# [LibraryClasses.common.UEFI_APPLICATION]
# [LibraryClasses.common.DXE_SMM_DRIVER]
# [LibraryClasses.common.SMM_CORE]
!ifdef $(DEBUG_ON_SERIAL_PORT)
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
!else
  DebugLib|OvmfPkg/Library/PlatformDebugLibIoPort/PlatformDebugLibIoPort.inf
!endif
```

As `DEBUG_ON_SERIAL_PORT` is not defined by default, the main library in our case would be `PlatformDebugLibIoPort` (https://github.com/tianocore/edk2/tree/master/OvmfPkg/Library/PlatformDebugLibIoPort):
```
DebugLib|OvmfPkg/Library/PlatformDebugLibIoPort/PlatformDebugLibIoPort.inf
```

As you remember `DEBUG` macro gets translated to something like:
```
if (DebugPrintEnabled ()) {
  if (DebugPrintLevelEnabled (PrintLevel)) {
    DebugPrint (PrintLevel, ##__VA_ARGS__);
  }
}
```

Let's look at the https://github.com/tianocore/edk2/blob/master/OvmfPkg/Library/PlatformDebugLibIoPort/DebugLib.c to see how `DebugPrintEnabled`, `DebugPrintLevelEnabled` and `DebugPrint` are implemented.


`DEBUG` macro first checks `DebugPrintEnabled` result. This function checks PCD `PcdDebugPropertyMask` if it has `DEBUG_PROPERTY_DEBUG_PRINT_ENABLED` bit set:
```
BOOLEAN
EFIAPI
DebugPrintEnabled (
  VOID
  )
{
  return (BOOLEAN) ((PcdGet8(PcdDebugPropertyMask) & DEBUG_PROPERTY_DEBUG_PRINT_ENABLED) != 0);
}
```
Then `DebugPrintLevelEnabled` function checks if passed `ErrorLevel` is present in the PCD `PcdFixedDebugPrintErrorLevel`:
```
BOOLEAN
EFIAPI
DebugPrintLevelEnabled (
  IN  CONST UINTN        ErrorLevel
  )
{
  return (BOOLEAN) ((ErrorLevel & PcdGet32(PcdFixedDebugPrintErrorLevel)) != 0);
}
```

Then actual `DebugPrint` function is executed. It simply passes control to `DebugVPrint`:
```
VOID
EFIAPI
DebugPrint (
  IN  UINTN        ErrorLevel,
  IN  CONST CHAR8  *Format,
  ...
  )
{
  VA_LIST         Marker;

  VA_START (Marker, Format);
  DebugVPrint (ErrorLevel, Format, Marker);
  VA_END (Marker);
}
```

And `DebugVPrint` in turn passes control to `DebugPrintMarker`:
```
VOID
EFIAPI
DebugVPrint (
  IN  UINTN         ErrorLevel,
  IN  CONST CHAR8   *Format,
  IN  VA_LIST       VaListMarker
  )
{
  DebugPrintMarker (ErrorLevel, Format, VaListMarker, NULL);
}
```

`DebugPrintMarker` is the main debug function. It check if passed `ErrorLevel` is present in `GetDebugPrintErrorLevel()` output and in the end performs write of the debug string to the I/O port defined by PCD `PcdDebugIoPort`:
```
VOID
DebugPrintMarker (
  IN  UINTN         ErrorLevel,
  IN  CONST CHAR8   *Format,
  IN  VA_LIST       VaListMarker,
  IN  BASE_LIST     BaseListMarker
  )
{
  CHAR8    Buffer[MAX_DEBUG_MESSAGE_LENGTH];
  UINTN    Length;

  //
  // If Format is NULL, then ASSERT().
  //
  ASSERT (Format != NULL);

  //
  // Check if the global mask disables this message or the device is inactive
  //
  if ((ErrorLevel & GetDebugPrintErrorLevel ()) == 0 ||
      !PlatformDebugLibIoPortFound ()) {
    return;
  }

  //
  // Convert the DEBUG() message to an ASCII String
  //
  if (BaseListMarker == NULL) {
    Length = AsciiVSPrint (Buffer, sizeof (Buffer), Format, VaListMarker);
  } else {
    Length = AsciiBSPrint (Buffer, sizeof (Buffer), Format, BaseListMarker);
  }

  //
  // Send the print string to the debug I/O port
  //
  IoWriteFifo8 (PcdGet16 (PcdDebugIoPort), Length, Buffer);
}
```

`GetDebugPrintErrorLevel()` is a function from `DebugPrintErrorLevelLib` library. Our `OvmfPkgX64.dsc` defines this implementation for it:
```
[LibraryClasses]
  ...
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
```
If you'll look at it sources you'll see that it simply checks for another PCD `PcdDebugPrintErrorLevel` (https://github.com/tianocore/edk2/blob/master/MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.c):
```
UINT32
EFIAPI
GetDebugPrintErrorLevel (
  VOID
  )
{
  //
  // Retrieve the current debug print error level mask from PcdDebugPrintErrorLevel.
  //
  return PcdGet32 (PcdDebugPrintErrorLevel);
}
```

## Summary:
The code `DEBUG (( ErrorLevel, String, ... ))` does the following:
- checks if `PcdDebugPropertyMask` has `DEBUG_PROPERTY_DEBUG_PRINT_ENABLED`
- checks if passed `ErrorLevel` is set in `PcdFixedDebugPrintErrorLevel` 
- checks if passed `ErrorLevel` is set in `PcdDebugPrintErrorLevel`
- writes formatted String to I/O port defined by `PcdDebugIoPort`



# Check if `PcdDebugPropertyMask` has `DEBUG_PROPERTY_DEBUG_PRINT_ENABLED`

OVMF DSC file defines this PCD PcdDebugPropertyMask like this https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkgX64.dsc:
```
[PcdsFixedAtBuild]
!if $(SOURCE_DEBUG_ENABLE) == TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x17
!else
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2F
!endif
```
It also redefines this PCD for Shell:
```
[Components]
  ShellPkg/Application/Shell/Shell.inf {
    ...
    <PcdsFixedAtBuild>
    ...
    gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF
  }
```

You can check bit definitions for this PCD in a https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec:
```
## The mask is used to control DebugLib behavior.<BR><BR>
#  BIT0 - Enable Debug Assert.<BR>
#  BIT1 - Enable Debug Print.<BR>
#  BIT2 - Enable Debug Code.<BR>
#  BIT3 - Enable Clear Memory.<BR>
#  BIT4 - Enable BreakPoint as ASSERT.<BR>
#  BIT5 - Enable DeadLoop as ASSERT.<BR>
# @Prompt Debug Property.
# @Expression  0x80000002 | (gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask & 0xC0) == 0
gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0|UINT8|0x00000005
```
So if the `SOURCE_DEBUG_ENABLE` is:
- TRUE: these features are enabled: Debug Assert/Debug Print/Debug Code/BreakPoint as ASSERT.
- FALSE: these features are enabled: Debug Assert/Debug Print/Debug Code/Clear Memory/DeadLoop as ASSERT.

In a `DEBUG` macro we compare this PCD aginst `DEBUG_PROPERTY_DEBUG_PRINT_ENABLED` (https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/DebugLib.h):
```
//
// Declare bits for PcdDebugPropertyMask
//
#define DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED       0x01
#define DEBUG_PROPERTY_DEBUG_PRINT_ENABLED        0x02
#define DEBUG_PROPERTY_DEBUG_CODE_ENABLED         0x04
#define DEBUG_PROPERTY_CLEAR_MEMORY_ENABLED       0x08
#define DEBUG_PROPERTY_ASSERT_BREAKPOINT_ENABLED  0x10
#define DEBUG_PROPERTY_ASSERT_DEADLOOP_ENABLED    0x20
```

As you can see this check would pass no matter of `SOURCE_DEBUG_ENABLE` setting.

# Checks if passed `ErrorLevel` is set in `PcdFixedDebugPrintErrorLevel`

This PCD is declared in https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec:
```
[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdFixedDebugPrintErrorLevel|0xFFFFFFFF|UINT32|0x30001016
```

This PCD is not redefined in the OVMF DSC file, so this is a final value. `0xFFFFFFFF` means that `DEBUG` message would pass this check no matter of its `EFI_D_*` print level.


# Checks if passed `ErrorLevel` is set in `PcdDebugPrintErrorLevel`

This PCD is declared in a https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec
```
[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000000|UINT32|0x00000006
```

By default it is defined in the `MdePkg.dec` and in the `MdePkg.dsc` as `0x80000000` https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dsc:
```
[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000000
```
The `0x80000000` value means that only error messages (`EFI_D_ERROR`) would be printed (ErrorLevel bit definitions again are in https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/DebugLib.h).


Although when we compile OVMF our main DSC is `OvmfPkgX64.dsc` https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkgX64.dsc. And this DSC redefines `PcdDebugPrintErrorLevel` PCD:
```
[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x8000004F
```
The `0x8000004F` value is equal to: `EFI_D_ERROR | EFI_D_INFO | EFI_D_FS | EFI_D_LOAD | EFI_D_WARN | EFI_D_INIT`.

So it means that when DEBUG statement is enabled, only these categories of messages would be printed.


# Writes formatted String to I/O port defined by `PcdDebugIoPort`

The `PcdDebugIoPort` PCD is defined in https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkg.dec
```
[PcdsFixedAtBuild]
  gUefiOvmfPkgTokenSpaceGuid.PcdDebugIoPort|0x402|UINT16|4
```

This PCD controls the destination I/O port for the debug messages in `PlatformDebugLibIoPort`

# Testing

To see debug messages we need to recompile OVMF in debug mode.

To do this use:
```
build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=DEBUG --tagname=GCC5
```
The build results would be in a folder
```
Build/OvmfX64/DEBUG_GCC5
```

Now let's run QEMU with these parameters:
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk \
  -net none \
  -nographic \
  -global isa-debugcon.iobase=0x402 \
  -debugcon file:debug.log
```

This would create `debug.log` file with all log `DEBUG` messages from OVMF. This file would be recreated every time you run this command.

In a separate terminal window you can run:
```
tail -f debug.log
```
to follow this file content runtime.

I've included the produced `debug.log` to the current folder.


# Replace GUIDs in a log file

If you start looking at the `debug.log` file, you'll see that it is full of GUIDs:
```
$ head debug.log
SecCoreStartupWithStack(0xFFFCC000, 0x820000)
Register PPI Notify: DCD0BE23-9586-40F4-B643-06522CED4EDE
Install PPI: 8C8CE578-8A3D-4F1C-9935-896185C32DD3
Install PPI: 5473C07A-3DCB-4DCA-BD6F-1E9689E7349A
The 0th FV start address is 0x00000820000, size is 0x000E0000, handle is 0x820000
Register PPI Notify: 49EDB1C1-BF21-4761-BB12-EB0031AABB39
Register PPI Notify: EA7CA24B-DED5-4DAD-A389-BF827E8F9B38
Install PPI: B9E0ABFE-5979-4914-977F-6DEE78C278A6
Install PPI: DBE23AA9-A345-4B97-85B6-B226F1617389
DiscoverPeimsAndOrderWithApriori(): Found 0x7 PEI FFS files in the 0th FV
```
It would be nice to replace them with meaningfull text for readability.

Luckily edk2 build system creates a file with names for used GUIDs:
```
$ head Build/OvmfX64/DEBUG_GCC5/FV/Guid.xref
8c1a6b71-0c4b-4497-aaad-07404edf142c PCDLesson
1BA0062E-C779-4582-8566-336AE8F78F09 ResetVector
df1ccef6-f301-4a63-9661-fc6030dcc880 SecMain
52C05B14-0B98-496c-BC3B-04B50211D680 PeiCore
9B3ADA4F-AE56-4c24-8DEA-F03B7558AE50 PcdPeim
A3610442-E69F-4DF3-82CA-2360C4031A23 ReportStatusCodeRouterPei
9D225237-FA01-464C-A949-BAABC02D31D0 StatusCodeHandlerPei
86D70125-BAA3-4296-A62F-602BEBBB9081 DxeIpl
222c386d-5abc-4fb4-b124-fbb82488acf4 PlatformPei
89E549B0-7CFE-449d-9BA3-10D8B2312D71 S3Resume2Pei
```

Let's create a python app that would parse this file, create a `GUID:NAME` dictionary and use it to replace all GUIDs with their names in a `debug.log` file. The result would be written to the `debug_parsed.log` file:
```
from shutil import copyfile

GUIDS_FILE_PATH = "Build/OvmfX64/DEBUG_GCC5/FV/Guid.xref"
EXTRA_GUIDS_FILE_PATH = "Guid_extra.xref"
LOG_IN_FILE_PATH = "debug.log"
LOG_OUT_FILE_PATH = "debug_parsed.log"

guids = {}

with open(GUIDS_FILE_PATH) as p:
    for line in p:
        l = line.split(" ")
        if len(l)==2:
            guids[l[0].upper()] = l[1][:-1]

if EXTRA_GUIDS_FILE_PATH:
    with open(EXTRA_GUIDS_FILE_PATH) as p:
        for line in p:
            l = line.split(" ")
            if len(l)==2:
                guids[l[0].upper()] = l[1][:-1]

copyfile(LOG_IN_FILE_PATH, LOG_OUT_FILE_PATH)

f = open(LOG_OUT_FILE_PATH, 'r')
filedata = f.read()
f.close()

for key,val in guids.items():
    filedata = filedata.replace(key, val)

f = open(LOG_OUT_FILE_PATH, 'w')
f.write(filedata)
f.close()
```
I've also added a possibility to include another `GUID:NAME` file with a help of `EXTRA_GUIDS_FILE_PATH` variable. This can be usefull if for some reason any guids are not present in the main `Guid.xref` file.

Put this script in edk2 folder and run it with `python` or `python3`:
```
$ python replace_guids.py
```

Now look at the created `debug_parsed.log` file:
```
$ head debug_parsed.log
SecCoreStartupWithStack(0xFFFCC000, 0x820000)
Register PPI Notify: gEfiPeiSecurity2PpiGuid
Install PPI: gEfiFirmwareFileSystem2Guid
Install PPI: gEfiFirmwareFileSystem3Guid
The 0th FV start address is 0x00000820000, size is 0x000E0000, handle is 0x820000
Register PPI Notify: gEfiPeiFirmwareVolumeInfoPpiGuid
Register PPI Notify: gEfiPeiFirmwareVolumeInfo2PpiGuid
Install PPI: gEfiPeiLoadFilePpiGuid
Install PPI: gEfiTemporaryRamSupportPpiGuid
DiscoverPeimsAndOrderWithApriori(): Found 0x7 PEI FFS files in the 0th FV
```

It is much easier to read now!

As a final step, let's modify our python script to make it more generic, so it would take all variables not from the script defines, but from the command line arguments. Also let's add a possibility to modify log file in place by default:
```
from argparse import ArgumentParser

parser = ArgumentParser(description="Convert GUIDs to text identifiers in UEFI firmware boot log")
parser.add_argument('-g', '--guids', help="Guid.xref file location", required=True)
parser.add_argument('-e', '--guids_extra', help="additional Guid.xref file location")
parser.add_argument('-i', '--log_input', help="input log file location", required=True)
parser.add_argument('-o', '--log_output', help="output log file location (by default input file is changed in place)")
args = parser.parse_args()

GUIDS_FILE_PATH = args.guids
EXTRA_GUIDS_FILE_PATH = args.guids_extra

LOG_IN_FILE_PATH = args.log_input
if args.log_output:
    LOG_OUT_FILE_PATH = args.log_output
else:
    LOG_OUT_FILE_PATH = args.log_input

...

if LOG_IN_FILE_PATH != LOG_OUT_FILE_PATH:
    copyfile(LOG_IN_FILE_PATH, LOG_OUT_FILE_PATH)

...
```

With this version we need to launch our script like this:
```
$ python replace_guids.py -g Build/OvmfX64/DEBUG_GCC5/FV/Guid.xref -e Guid_extra.xref -i debug.log -o debug_parsed.log
```

Also now we have a nice help message:
```
$ python replace_guids.py --help
usage: replace_guids.py [-h] -g GUIDS [-e GUIDS_EXTRA] -i LOG_INPUT [-o LOG_OUTPUT]

Convert GUIDs to text identifiers in UEFI firmware boot log

optional arguments:
  -h, --help            show this help message and exit
  -g GUIDS, --guids GUIDS
                        Guid.xref file location
  -e GUIDS_EXTRA, --guids_extra GUIDS_EXTRA
                        additional Guid.xref file location
  -i LOG_INPUT, --log_input LOG_INPUT
                        input log file location
  -o LOG_OUTPUT, --log_output LOG_OUTPUT
                        output log file location (by default input file is changed in place)
```

# Max string length in `DEBUG` output

There is a limitation to the maximum string length that can be outputed with a `DEBUG` statement.

https://github.com/tianocore/edk2/blob/master/OvmfPkg/Library/PlatformDebugLibIoPort/DebugLib.c:
```
#define MAX_DEBUG_MESSAGE_LENGTH  0x100
```
If the printed sting is longer than this limit, the output would be truncated to the first `MAX_DEBUG_MESSAGE_LENGTH` symbols. Most probably you would notice this by the fact that `\n` symbol at the end of the string would be lost, and the next printed string wouldn't start from the new line.

In this case you might consider increasing the aforementioned limit, for this change the value above and recompile OVMF:
```
build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=DEBUG --tagname=GCC5
```
