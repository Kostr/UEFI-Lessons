Let's create a simple UEFI driver.

Up until now we've created only UEFI applications. The main difference between application and a driver is a fact that application is unloaded from the memory after its execution. But the driver is a thing that stays in memory. And while it stays there it can provide usefull protocols for other applications to use.


Let's create a simplest driver UefiLessonsPkg/SimpleDriver/SimpleDriver.inf
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = SimpleDriver
  FILE_GUID                      = 384aeb18-105d-4af1-bf17-5e349e8f4d4c
  MODULE_TYPE                    = UEFI_DRIVER
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = SimpleDriverEntryPoint

[Sources]
  SimpleDriver.c

[Packages]
  MdePkg/MdePkg.dec

[LibraryClasses]
  UefiDriverEntryPoint
  UefiLib
```
Things that have changed from our usual INF file:
- The `MODULE_TYPE` is `UEFI_DRIVER` (earlier we've always used `UEFI_APPLICATION`),
- The `ENTRY_POINT` is `SimpleDriverEntryPoint` (earlier we've always used `UefiMain`, but the driver operates with Entry/Unload functions, so it is better to start to give them proper names),
- `UefiDriverEntryPoint` library class is used (earlier we've always used `UefiApplicationEntryPoint`). In case you wonder about `UefiDriverEntryPoint` library internals take a look into https://github.com/tianocore/edk2/tree/master/MdePkg/Library/UefiDriverEntryPoint

Now let's write *.c source file UefiLessonsPkg/SimpleDriver/SimpleDriver.c

The only function that we need to implement is our entry function `SimpleDriverEntryPoint` that we've declared:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

EFI_STATUS
EFIAPI
SimpleDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print(L"Hello from driver!\n");

  return EFI_SUCCESS;
}
```

Include this driver in the components section of our `UefiLessonsPkg/UefiLessonsPkg.dsc`:
```
[Components]
  ...
  UefiLessonsPkg/SimpleDriver/SimpleDriver.inf
```
If you try to build, the process would fail:
```
build.py...
/home/aladyshev/tiano/edk2/UefiLessonsPkg/UefiLessonsPkg.dsc(...): error 4000: Instance of library class [UefiDriverEntryPoint] is not found
        in [/home/aladyshev/tiano/edk2/UefiLessonsPkg/SimpleDriver/SimpleDriver.inf] [X64]
        consumed by module [/home/aladyshev/tiano/edk2/UefiLessonsPkg/SimpleDriver/SimpleDriver.inf]
```
As usually we need to find proper library implementation:
```
$ grep UefiDriverEntryPoint -r ./ --exclude-dir=Build | grep LIBRARY_CLASS
./MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf:  LIBRARY_CLASS                  = UefiDriverEntryPoint|DXE_DRIVER DXE_RUNTIME_DRIVER UEFI_DRIVER SMM_CORE DXE_SMM_DRIVER
```
And place it in our DSC:
```
[LibraryClasses]
  ...
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
```

Build, copy file to our QEMU shared folder and run OVMF.

First let's try to execute it as an app:
```
FS0:\> SimpleDriver.efi
The image is not an application.
```
As you see this action is not possible.

To load driver we need to use `load` shell command:
```
FS0:\> load -? -b
Loads a UEFI driver into memory.

LOAD [-nc] file [file...]

  -nc  - Loads the driver, but does not connect the driver.
  File - Specifies a file that contains the image of the UEFI driver (wildcards are
         permitted).

NOTES:
  1. This command loads a driver into memory. It can load multiple files at
     one time. The file name supports wildcards.
  2. If the -nc flag is not specified, this command attempts to connect the
     driver to a proper device. It might also cause previously loaded drivers
     to be connected to their corresponding devices.
  3. Use the 'UNLOAD' command to unload a driver.

EXAMPLES:
  * To load a driver:
    fs0:\> load Isabus.efi

  * To load multiple drivers:
    fs0:\> load Isabus.efi IsaSerial.efi

  * To load multiple drivers using file name wildcards:
    fs0:\> load Isa*.efi

  * To load a driver without connecting it to a device:
    fs0:\> load -nc IsaBus.efi
```

Use this command to load our driver:
```
FS0:\> load SimpleDriver.efi
Hello from driver!
Image 'FS0:\SimpleDriver.efi' loaded at 6646000 - Success
```

Now let's try to use `dh` command to look at our driver handle:
```
FS0:\> dh -? -b
Displays the device handles in the UEFI environment.

DH [-l <lang>] [handle | -p <prot_id>] [-d] [-v]

  -p     - Dumps all handles of a protocol specified by the GUID.
  -d     - Dumps UEFI Driver Model-related information.
  -l     - Dumps information using the language codes (e.g. ISO 639-2).
  -sfo   - Displays information as described in Standard-Format Output.
  -v     - Dumps verbose information about a specific handle.
  handle - Specifies a handle to dump information about (a hexadecimal number).
           If not present, then all information will be dumped.

NOTES:
  1. When neither 'handle' nor 'prot_id' is specified, a list of all the
     device handles in the UEFI environment is displayed.
  2. The '-d' option displays UEFI Driver Model related information including
     parent handles, child handles, all drivers installed on the handle, etc.
  3. The '-v' option displays verbose information for the specified handle
     including all the protocols on the handle and their details.
  4. If the '-p' option is specified, all handles containing the specified
     protocol will be displayed. Otherwise, the 'handle' parameter has to be
     specified for display. In this case, the '-d' option will be enabled
     automatically if the '-v' option is not specified.

EXAMPLES:
  * To display all handles and display one screen at a time:
    Shell> dh -b

  * To display the detailed information on handle 0x30:
    Shell> dh 30

  * To display all handles with 'diskio' protocol:
    Shell> dh -p diskio

  * To display all handles with 'LoadedImage' protocol and break when the screen is
    full:
    Shell> dh -p LoadedImage -b
```

If you'll execute it without any parameters, you'll get all handles in the system. And our driver would be the last one:
```
FS0:\> dh
...
C6: ImageDevicePath(..F,0xFBFC1)/\SimpleDriver.efi) LoadedImage(\SimpleDriver.efi)
```
You can print more verbose output for our handle:
```
FS0:\> dh -d -v c6
C6: 664C998
ImageDevicePath(664A018)
  PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)/HD(1,MBR,0xBE1AFDFA,0x3F,0xFBFC1)/\SimpleDriver.efi
LoadedImage(664A440)
  Revision......: 0x00001000
  ParentHandle..: 6EE5D18
  SystemTable...: 79EE018
  DeviceHandle..: 6E36798
  FilePath......: \SimpleDriver.efi
  PdbFileName...: /home/aladyshev/tiano/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/SimpleDriver/SimpleDriver/DEBUG/SimpleDriver.dll
  OptionsSize...: 0
  LoadOptions...: 0
  ImageBase.....: 6646000
  ImageSize.....: 16C0
  CodeType......: EfiBootServicesCode
  DataType......: EfiBootServicesData
  Unload........: 0
```


You can load more instanses of our driver, this is not a problem:
```
FS0:\> load SimpleDriver.efi
Hello from driver!
Image 'FS0:\SimpleDriver.efi' loaded at 6619000 - Success
FS0:\> load SimpleDriver.efi
Hello from driver!
Image 'FS0:\SimpleDriver.efi' loaded at 6617000 - Success
FS0:\> load SimpleDriver.efi
Hello from driver!
Image 'FS0:\SimpleDriver.efi' loaded at 6613000 - Success
```

If you look at dh now, there would be multiple handles from our driver:
```
FS0:\> dh
...
C6: ImageDevicePath(..F,0xFBFC1)/\SimpleDriver.efi) LoadedImage(\SimpleDriver.efi)
C7: ImageDevicePath(..F,0xFBFC1)/\SimpleDriver.efi) LoadedImage(\SimpleDriver.efi)
C8: ImageDevicePath(..F,0xFBFC1)/\SimpleDriver.efi) LoadedImage(\SimpleDriver.efi)
C9: ImageDevicePath(..F,0xFBFC1)/\SimpleDriver.efi) LoadedImage(\SimpleDriver.efi)
```

To unload driver from memory you can utilize `unload` command:
```
FS0:\> unload -?
Unloads a driver image that was already loaded.

UNLOAD [-n] [-v|-verbose] Handle

  -n           - Skips all prompts during unloading, so that it can be used
                 in a script file.
  -v, -verbose - Dumps verbose status information before the image is unloaded.
  Handle       - Specifies the handle of driver to unload, always taken as hexadecimal number.

NOTES:
  1. The '-n' option can be used to skip all prompts during unloading.
  2. If the '-v' option is specified, verbose image information will be
     displayed before the image is unloaded.
  3. Only drivers that support unloading can be successfully unloaded.
  4. Use the 'LOAD' command to load a driver.

EXAMPLES:
  * To find the handle for the UEFI driver image to unload:
    Shell> dh -b

  * To unload the UEFI driver image with handle 27:
    Shell> unload 27
```

But it is now possible to use it now as our driver don't have an Unload function. If you'll look at the earlier output of `dh -d -v c6` command, you can see that `Unload........: 0`.

Therefore if you'll try to unload our driver you'll get an error:
```
FS0:\> unload c6
Unload - Handle [664C998].  [y/n]?
y
Unload - Handle [664C998] Result Unsupported.
```
If you'll execute `dh`, you would still see that our driver handle is still present in the system.

# Add unload function

Now let's try add unload function to our driver. Add it to the INF file UefiLessonsPkg/SimpleDriver/SimpleDriver.inf:
```
[Defines]
  ...
  ENTRY_POINT                    = SimpleDriverEntryPoint
+ UNLOAD_IMAGE                   = SimpleDriverUnload
```
And add some simple implementation to the *.c file UefiLessonsPkg/SimpleDriver/SimpleDriver.c:
```
EFI_STATUS
EFIAPI
SimpleDriverUnload (
  EFI_HANDLE ImageHandle
  )
{
  Print(L"Bye-bye from driver!\n");

  return EFI_SUCCESS;
}
```

If you try to execute `dh -d -v` on the handle from this driver you'll get something like:
```
FS0:\> dh -d -v c6
C6: 664CA98
ImageDevicePath(664C618)
  PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)/HD(1,MBR,0xBE1AFDFA,0x3F,0xFBFC1)/\SimpleDriver.efi
LoadedImage(664A240)
  Revision......: 0x00001000
  ParentHandle..: 6EE5D18
  SystemTable...: 79EE018
  DeviceHandle..: 6E36798
  FilePath......: \SimpleDriver.efi
  PdbFileName...: /home/aladyshev/tiano/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/SimpleDriver/SimpleDriver/DEBUG/SimpleDriver.dll
  OptionsSize...: 0
  LoadOptions...: 0
  ImageBase.....: 6646000
  ImageSize.....: 1780
  CodeType......: EfiBootServicesCode
  DataType......: EfiBootServicesData
  Unload........: 6647047
```
As you can see now `Unload` string is filled with a pointer to the driver unload function.

Before preforming an unload take a look at the ImageBase address with `dmem`:
```
FS0:\> dmem 6646000 A0
Memory Address 0000000006646000 A0 Bytes
  06646000: 4D 5A 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *MZ..............*
  06646010: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  06646020: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  06646030: 00 00 00 00 00 00 00 00-00 00 00 00 80 00 00 00  *................*
  06646040: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  06646050: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  06646060: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  06646070: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  *................*
  06646080: 50 45 00 00 64 86 02 00-00 00 00 00 00 00 00 00  *PE..d...........*
  06646090: 00 00 00 00 F0 00 2E 00-0B 02 00 00 40 14 00 00  *............@...*
```
`MZ` signature signifies the header of a PE/COFF image (*.efi file). So our driver is actually there.

Now perform unload:
```
FS0:\> unload c6
Unload - Handle [664CF18].  [y/n]?
y
Bye-bye from driver!
Unload - Handle [664CF18] Result Success.
```

Look at the memory again:
```
FS0:\> dmem 6646000 A0
Memory Address 0000000006646000 A0 Bytes
  06646000: AF AF AF AF AF AF AF AF-AF AF AF AF AF AF AF AF  *................*
  06646010: AF AF AF AF AF AF AF AF-AF AF AF AF AF AF AF AF  *................*
  06646020: AF AF AF AF AF AF AF AF-AF AF AF AF AF AF AF AF  *................*
  06646030: AF AF AF AF AF AF AF AF-AF AF AF AF AF AF AF AF  *................*
  06646040: AF AF AF AF AF AF AF AF-AF AF AF AF AF AF AF AF  *................*
  06646050: AF AF AF AF AF AF AF AF-AF AF AF AF AF AF AF AF  *................*
  06646060: AF AF AF AF AF AF AF AF-AF AF AF AF AF AF AF AF  *................*
  06646070: AF AF AF AF AF AF AF AF-AF AF AF AF AF AF AF AF  *................*
  06646080: AF AF AF AF AF AF AF AF-AF AF AF AF AF AF AF AF  *................*
  06646090: AF AF AF AF AF AF AF AF-AF AF AF AF AF AF AF AF  *................*
```
As you can see in was automatically freed.


One more notice. If you'll load your image again, it would have a handle with C7 number. Number C6 would be skipped:
```
C5: ...
C7: ImageDevicePath(..F,0xFBFC1)/\SimpleDriver.efi) LoadedImage(\SimpleDriver.efi)
```

