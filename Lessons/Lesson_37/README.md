If you'll search through ShellPkg library (https://github.com/tianocore/edk2/tree/master/ShellPkg/Library) you can notice that there is a folder `UefiShellAcpiViewCommandLib` (https://github.com/tianocore/edk2/tree/master/ShellPkg/Library/UefiShellAcpiViewCommandLib).
This folder provides a library for the support of in-shell `acpiview` command. If you check the INF file, you'll see
https://github.com/tianocore/edk2/blob/master/ShellPkg/Library/UefiShellAcpiViewCommandLib/UefiShellAcpiViewCommandLib.inf:
```
# Provides Shell 'acpiview' command functions
```
But if you try to execute `acpiview` in our current OVMF build, you'll notice that this command is not recognized:
```
FS0:\> acpiview
'acpiview' is not recognized as an internal or external command, operable program, or script file.
```
We have 3 ways to use this 'acpiview' command functionality:
- compile `acpiview` as a separate app and run it as an ordinary UEFI shell application
- compile shell with 'acpiview' command in itself and run it under OVMF
- update OVMF image with a shell that actually includes 'acpiview' command in itself

# Compile `acpiview` as a separate app

I guess it is the most easy way.

It is possible to perform such thing with a help of https://github.com/tianocore/edk2/tree/master/ShellPkg/Application/AcpiViewApp

If you look at the source file, you'll see that it is pretty simple, the main function just makes a library call to `ShellCommandRunAcpiView` function:
```
EFI_STATUS
EFIAPI
AcpiViewAppMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return ShellCommandRunAcpiView (gImageHandle, SystemTable);
}
```

To build this application issue:
```
build --platform=ShellPkg/ShellPkg.dsc --module=ShellPkg/Application/AcpiViewApp/AcpiViewApp.inf --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

Copy image to the QEMU shared folder:
```
cp Build/Shell/RELEASE_GCC5/X64/AcpiViewApp.efi ~/UEFI_disk/
```

You can see application help with a:
```
FS0:\> AcpiViewApp.efi -?
Display ACPI Table information.

ACPIVIEWAPP.EFI [[-?] | [[[[-l] | [-s AcpiTable [-d]]] [-q] [-h]] [-r Spec]]]


  -l - Display list of installed ACPI Tables.
  -s - Display only the specified AcpiTable type and only support single
       invocation option.
         AcpiTable    : The required ACPI Table type.
  -d - Generate a binary file dump of the specified AcpiTable.
  -q - Quiet. Suppress errors and warnings. Disables consistency checks.
  -h - Enable colour highlighting.
  -r - Validate that all required ACPI tables are installed
         Spec  : Specification to validate against.
                 For Arm, the possible values are:
                   0 - Server Base Boot Requirements v1.0
                   1 - Server Base Boot Requirements v1.1
                   2 - Server Base Boot Requirements v1.2
  -? - Show help.


  This program is provided to allow examination of ACPI table values from the
  UEFI Shell.  This can help with investigations, especially at that stage
  where the tables are not enabling an OS to boot.
  The program is not exhaustive, and only encapsulates detailed knowledge of a
  limited number of table types.

  Default behaviour is to display the content of all tables installed.
  'Known' table types (listed in NOTES below) will be parsed and displayed
  with descriptions and field values.  Where appropriate a degree of
  consistency checking is done and errors may be reported in the output.
  Other table types will be displayed as an array of Hexadecimal bytes.

  To facilitate debugging, the -s and -d options can be used to generate a
  binary file image of a table that can be copied elsewhere for investigation
  using tools such as those provided by acpica.org.  This is especially
  relevant for AML type tables like DSDT and SSDT.

NOTES:
  1. The AcpiTable parameter can match any installed table type.
     Tables without specific handling will be displayed as a raw hex dump (or
     dumped to a file if -d is used).
  2. -s option supports to display the specified AcpiTable type that is present
     in the system. For normal type AcpiTable, it would display the data of the
     AcpiTable and AcpiTable header. The following type may contain header type
     other than AcpiTable header. The actual header can refer to the ACPI spec
     6.3
     Extra A. Particular types:
       APIC  - Multiple APIC Description Table (MADT)
       BGRT  - Boot Graphics Resource Table
       DBG2  - Debug Port Table 2
       DSDT  - Differentiated System Description Table
       FACP  - Fixed ACPI Description Table (FADT)
       GTDT  - Generic Timer Description Table
       IORT  - IO Remapping Table
       MCFG  - Memory Mapped Config Space Base Address Description Table
       PPTT  - Processor Properties Topology Table
       RSDP  - Root System Description Pointer
       SLIT  - System Locality Information Table
       SPCR  - Serial Port Console Redirection Table
       SRAT  - System Resource Affinity Table
       SSDT  - Secondary SystemDescription Table
       XSDT  - Extended System Description Table



EXAMPLES:
  * To display a list of the installed table types:
    fs0:\> acpiviewapp.efi -l

  * To parse and display a specific table type:
    fs0:\> acpiviewapp.efi -s GTDT

  * To save a binary dump of the contents of a table to a file
    in the current working directory:
    fs0:\> acpiviewapp.efi -s DSDT -d

  * To display contents of all ACPI tables:
    fs0:\> acpiviewapp.efi

  * To check if all Server Base Boot Requirements (SBBR) v1.2 mandatory
    ACPI tables are installed (Arm only):
    fs0:\> acpiviewapp.efi -r 2
```

With this program you can list ACPI tables in system:
```
FS0:\> AcpiViewApp.efi -l

Installed Table(s):
           1. RSDP
           2. XSDT
           3. FACP
           4. FACS
           5. DSDT
           6. APIC
           7. HPET
           8. BGRT
```

Show the content of any table:
```
FS0:\> AcpiViewApp.efi -s BGRT


 --------------- BGRT Table ---------------

Address  : 0x7B77000
Length   : 56

00000000 : 42 47 52 54 38 00 00 00 - 01 C5 49 4E 54 45 4C 20   BGRT8.....INTEL
00000010 : 45 44 4B 32 20 20 20 20 - 02 00 00 00 20 20 20 20   EDK2    ....
00000020 : 13 00 00 01 01 00 01 00 - 18 30 8B 06 00 00 00 00   .........0......
00000030 : 2F 01 00 00 0F 01 00 00                             /.......

Table Checksum : OK

BGRT                                 :
  Signature                          : BGRT
  Length                             : 56
  Revision                           : 1
  Checksum                           : 0xC5
  Oem ID                             : INTEL
  Oem Table ID                       : EDK2
  Oem Revision                       : 0x2
  Creator ID                         :
  Creator Revision                   : 0x1000013
  Version                            : 0x1
  Status                             : 0x1
  Image Type                         : 0x0
  Image Address                      : 0x68B3018
  Image Offset X                     : 303
  Image Offset Y                     : 271

Table Statistics:
        0 Error(s)
        0 Warning(s)
```

Or dump any ACPI table:
```
FS0:\> acpiview -s APIC -d
Dumping ACPI table to : .\APIC0000.bin ... DONE.
```
You can disassemble this image with `iasl -d <file>` like we did earlier.


# Compile shell with 'acpiview' command in itself and run it under OVMF

This case is a little bit crazy, we would be running a shell applicaion inside the shell application.

I guess this is not the usual case, but it will help you to know how to compile the shell image that you can actually use in your projects.

If you'll look at the https://github.com/tianocore/edk2/blob/master/ShellPkg/ShellPkg.dsc you'll see that if you build `ShellPkg`, you'll actually build two versions of the `Shell.inf`:
- one would have general commands
- another one would have all the commands
```
ShellPkg/Application/Shell/Shell.inf {
  <PcdsFixedAtBuild>
    gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
  <LibraryClasses>
    NULL|ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
fndef $(NO_SHELL_PROFILES)
    NULL|ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellNetwork2CommandsLib/UefiShellNetwork2CommandsLib.inf
ndif #$(NO_SHELL_PROFILES)
}

#
# Build a second version of the shell with all commands integrated
#
ShellPkg/Application/Shell/Shell.inf {
 <Defines>
    FILE_GUID = EA4BB293-2D7F-4456-A681-1F22F42CD0BC
  <PcdsFixedAtBuild>
    gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
  <LibraryClasses>
    NULL|ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellNetwork2CommandsLib/UefiShellNetwork2CommandsLib.inf
    NULL|ShellPkg/Library/UefiShellAcpiViewCommandLib/UefiShellAcpiViewCommandLib.inf         <------- acpiview is present in this Shell version
}
```

In case you wonder how `UefiShellAcpiViewCommandLib.inf` registers new command take a look at its sources:

https://github.com/tianocore/edk2/blob/master/ShellPkg/Library/UefiShellAcpiViewCommandLib/UefiShellAcpiViewCommandLib.inf
```
[Defines]
  INF_VERSION                    = 0x00010019
  BASE_NAME                      = UefiShellAcpiViewCommandLib
  FILE_GUID                      = FB5B305E-84F5-461F-940D-82D345757AFA
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  LIBRARY_CLASS                  = AcpiViewCommandLib|UEFI_APPLICATION UEFI_DRIVER
  CONSTRUCTOR                    = UefiShellAcpiViewCommandLibConstructor
  DESTRUCTOR                     = UefiShellAcpiViewCommandLibDestructor

  ...
```
https://github.com/tianocore/edk2/blob/master/ShellPkg/Library/UefiShellAcpiViewCommandLib/UefiShellAcpiViewCommandLib.c
```
EFI_STATUS
EFIAPI
UefiShellAcpiViewCommandLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
 ...
  // Install our Shell command handler
  ShellCommandRegisterCommandName (
    L"acpiview",
    ShellCommandRunAcpiView,
    ShellCommandGetManFileNameAcpiView,
    0,
    L"acpiview",
    TRUE,
    gShellAcpiViewHiiHandle,
    STRING_TOKEN (STR_GET_HELP_ACPIVIEW)
    );

  return EFI_SUCCESS;
}
```
It doesn't look too scary, so you can even try to add your command to the shell. Maybe will try that in later lessons.

Now execute this command to build the Shell application:
```
build --platform=ShellPkg/ShellPkg.dsc --module=ShellPkg/Application/Shell/Shell.inf --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

After the build there would be two files in the build folder:
```
$ ls Build/Shell/RELEASE_GCC5/X64/Shell*.efi
Build/Shell/RELEASE_GCC5/X64/Shell_7C04A583-9E3E-4f1c-AD65-E05268D0B4D1.efi
Build/Shell/RELEASE_GCC5/X64/Shell_EA4BB293-2D7F-4456-A681-1F22F42CD0BC.efi
```

If you look closely to the code from the `ShellPkg/ShellPkg.dsc` that I've pasted earlier, you can notice that the image that we need is an image with a `EA4BB293-2D7F-4456-A681-1F22F42CD0BC` guid.

Copy it to the QEMU shared folder:
```
$ cp Build/Shell/RELEASE_GCC5/X64/Shell_EA4BB293-2D7F-4456-A681-1F22F42CD0BC.efi ~/UEFI_disk/
```
In your default shell there wouldn't be any `acpiview` command, but when as you'll move to the `Shell_EA4BB293-2D7F-4456-A681-1F22F42CD0BC.efi` this `acpiview` command would became present in the shell.
```
FS0:\> acpiview -l
'acpiview' is not recognized as an internal or external command, operable program, or script file.
FS0:\> Shell_EA4BB293-2D7F-4456-A681-1F22F42CD0BC.efi
UEFI Interactive Shell v2.2
EDK II
UEFI v2.70 (EDK II, 0x00010000)
Mapping table
      FS0: Alias(s):HD0a1:;BLK1:
          PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)/HD(1,MBR,0xBE1AFDFA,0x3F,0xFBFC1)
     BLK0: Alias(s):
          PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
     BLK2: Alias(s):
          PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
Press ESC in 4 seconds to skip startup.nsh or any other key to continue.
FS0:\> acpiview -l

Installed Table(s):
           1. RSDP
           2. XSDT
           3. FACP
           4. FACS
           5. DSDT
           6. APIC
           7. HPET
           8. BGRT
```

# Update OVMF image with a shell that actually includes 'acpiview' command in itself

Correct `OvmfPkg/OvmfPkgX64.dsc`. You'll need to add `UefiShellAcpiViewCommandLib.inf` to the `Shell.inf` library classes:
```
[Components]
  ...
  ShellPkg/Application/Shell/Shell.inf {
    <LibraryClasses>
      ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
      NULL|ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellAcpiViewCommandLib/UefiShellAcpiViewCommandLib.inf          <-----------
!if $(NETWORK_IP6_ENABLE) == TRUE
      NULL|ShellPkg/Library/UefiShellNetwork2CommandsLib/UefiShellNetwork2CommandsLib.inf
!endif
      HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
      PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
      BcfgCommandLib|ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf

    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF
      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
      gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|8000
  }
```

Rebuild OVMF:
```
build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

You can test that this OVMF image has a shell that includes `acpiview` command in itself:
```
FS0:\> acpiview -l

Installed Table(s):
           1. RSDP
           2. XSDT
           3. FACP
           4. FACS
           5. DSDT
           6. APIC
           7. HPET
           8. BGRT
```

