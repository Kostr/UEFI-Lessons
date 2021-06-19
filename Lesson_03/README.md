Finally we are ready to write our "Hello World" app.

First we create a new edk2 module in our package directory similar to the our `SimplestApp` module:
```
$ mkdir UefiLessonsPkg/HelloWorld
$ vi UefiLessonsPkg/HelloWorld/HelloWorld.inf
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = HelloWorld
  FILE_GUID                      = 2e55fa38-f148-42d3-af90-1be247323e30
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = UefiMain

[Sources]
  HelloWorld.c

[Packages]
  MdePkg/MdePkg.dec

[LibraryClasses]
  UefiApplicationEntryPoint
```
Don't forget to add our newly created app to the `Components` section of the package DSC file
```
[Components]
  UefiLessonsPkg/SimplestApp/SimplestApp.inf
+ UefiLessonsPkg/HelloWorld/HelloWorld.inf
```
Next we need to write the source code file. Let's remember the code for our SimplestApp:
```
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}
```
To print something to the console ("Hello World" message in our case) we need to use services from the `EFI_SYSTEM_TABLE` that is passed to the entry point of our app.

The description of the `EFI_SYSTEM_TABLE` can be found in the UEFI specification (https://uefi.org/sites/default/files/resources/UEFI_Spec_2_8_final.pdf).

EFI_SYSTEM_TABLE is a struct that was populated by the UEFI firmware and contains pointers to the runtime and boot services tables.
```
typedef struct {
 EFI_TABLE_HEADER Hdr;
 CHAR16 *FirmwareVendor;
 UINT32 FirmwareRevision;
 EFI_HANDLE ConsoleInHandle;
 EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
 EFI_HANDLE ConsoleOutHandle;
 EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
 EFI_HANDLE StandardErrorHandle;
 EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
 EFI_RUNTIME_SERVICES *RuntimeServices;
 EFI_BOOT_SERVICES *BootServices;
 UINTN NumberOfTableEntries;
 EFI_CONFIGURATION_TABLE *ConfigurationTable;
} EFI_SYSTEM_TABLE;
```
We are interested in the `ConOut` field. `ConOut` is abbreviaton for "Console Output" and according to the UEFI spec it is a pointer to the `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL` interface that is associated with `ConsoleOutHandle`.

If we keep digging into UEFI spec we can find description of the `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL`.
According to the spec `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL` defines the minimum requirements for a text-based ConsoleOut device.

As everything in UEFI it has GUID:
```
#define EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID \
 {0x387477c2,0x69c7,0x11d2,\
 {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}}
```

And the interface description is:
```
typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
 EFI_TEXT_RESET Reset;
 EFI_TEXT_STRING OutputString;
 EFI_TEXT_TEST_STRING TestString;
 EFI_TEXT_QUERY_MODE QueryMode;
 EFI_TEXT_SET_MODE SetMode;
 EFI_TEXT_SET_ATTRIBUTE SetAttribute;
 EFI_TEXT_CLEAR_SCREEN ClearScreen;
 EFI_TEXT_SET_CURSOR_POSITION SetCursorPosition;
 EFI_TEXT_ENABLE_CURSOR EnableCursor;
 SIMPLE_TEXT_OUTPUT_MODE *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
```
Right now we are interested in a `OutputString` method:
```
OutputString     Displays the string on the device at the current cursor location.
```
This is what we need. Let's look at the function description:
```
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString()

Summary
Writes a string to the output device.

Prototype
typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_STRING) (
 IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
 IN CHAR16 *String
 );

Parameters
This    A pointer to the EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL instance.
String  The Null-terminated string to be displayed on the output device(s).
```

In edk2 `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL` is defined in the header file:
https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/SimpleTextOut.h


With all this knowledge we can write our source code file `UefiLessonsPkg/HelloWorld/HelloWorld.c`:
```
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello World!\n");
  return EFI_SUCCESS;
}
```

The `L""` signifies that the string is composed from CHAR16 symbols, as was required in spec.

Let's compile our edk2 module:
```
$ build --platform=UefiLessonsPkg/UefiLessonsPkg.dsc \
        --module=UefiLessonsPkg/HelloWorld/HelloWorld.inf \
        --arch=X64 \
        --buildtarget=RELEASE --tagname=GCC5
```

Copy the app to our `UEFI_disk` folder and run OVMF:
```
$ cp Build/UefiLessonsPkg/RELEASE_GCC5/X64/HelloWorld.efi ~/UEFI_disk/
$ qemu-system-x86_64 -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
                     -drive format=raw,file=fat:rw:~/UEFI_disk \
                     -nographic \
                     -net none
```

```
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
Shell> fs0:
FS0:\> HelloWorld.efi
Hello World!
FS0:\>
```
