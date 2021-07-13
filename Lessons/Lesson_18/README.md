
Let's create an app that can actually handle input from user.

To do this we will need `ReadKeyStroke` function:
```
EFI_SIMPLE_TEXT_INPUT_PROTOCOL.ReadKeyStroke()

Summary:
Reads the next keystroke from the input device.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_INPUT_READ_KEY) (
 IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
 OUT EFI_INPUT_KEY *Key
 );

Parameters:
This 		A pointer to the EFI_SIMPLE_TEXT_INPUT_PROTOCOL instance.
Key 		A pointer to a buffer that is filled in with the keystroke information
		for the key that was pressed.

Description:
The ReadKeyStroke() function reads the next keystroke from the input device.

Status Codes Returned:
EFI_SUCCESS 		The keystroke information was returned.
EFI_NOT_READY 		There was no keystroke data available.
EFI_DEVICE_ERROR 	The keystroke information was not returned due to hardware errors.
```

The `EFI_INPUT_KEY` function is defined like this:
```
typedef struct {
 UINT16 ScanCode;
 CHAR16 UnicodeChar;
} EFI_INPUT_KEY;
```


Create `InteractiveApp` with the following code:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN Index;
  EFI_INPUT_KEY Key;

  Print(L"Try to guess the secret symbol!\n");
  Print(L"To quit press 'q'\n");

  while(TRUE) {
    gBS->WaitForEvent(1, &(gST->ConIn->WaitForKey), &Index);
    gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    Print(L"ScanCode = %04x, UnicodeChar = %04x (%c)\n", Key.ScanCode, Key.UnicodeChar, Key.UnicodeChar);

    if (Key.UnicodeChar == 'k') {
      Print(L"Correct!\n");
      break;
    } else if (Key.UnicodeChar == 'q') {
      Print(L"Bye!\n");
      break;
    } else {
      Print(L"Wrong!\n");
    }
  }
  gST->ConIn->Reset(gST->ConIn, FALSE);
  return EFI_SUCCESS;
}
```

The code is pretty simple, so I hope no detailed explanation is needed.
App asks for the secret symbol and loops until user presses the correct one ('k'), or presses 'q'.

```
FS0:\> InteractiveApp.efi
Try to guess the secret symbol!
To quit press 'q'
ScanCode = 0000, UnicodeChar = 0066 (f)
Wrong!
ScanCode = 0000, UnicodeChar = 0074 (t)
Wrong!
ScanCode = 0000, UnicodeChar = 0073 (s)
Wrong!
ScanCode = 0000, UnicodeChar = 0069 (i)
Wrong!
ScanCode = 0000, UnicodeChar = 0061 (a)
Wrong!
ScanCode = 0000, UnicodeChar = 006E (n)
Wrong!
ScanCode = 0000, UnicodeChar = 006B (k)
Correct!
FS0:\>
```

# Differences in keystroke handling with and without `-nographic` option in QEMU

In our lessons we launch QEMU with a `-nographic` option:
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk \
  -nographic
```

With our app now you can observe how special keys are handled in this mode. For example this is the output from the "PageUp" key:
```
ScanCode = 0017, UnicodeChar = 0000 ( )
Wrong!
ScanCode = 0000, UnicodeChar = 005B ([)
Wrong!
ScanCode = 0000, UnicodeChar = 0035 (5)
Wrong!
ScanCode = 0000, UnicodeChar = 007E (~)
Wrong!
```
But according to the UEFI spec and edk2 header for the `SimpleTextIn` protocol (https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/SimpleTextIn.h) "PageUp" key should be displayed as a diferrent scan code, not as 4 symbols starting with the escape scan code:
```
#define SCAN_PAGE_UP    0x0009
...
#define SCAN_ESC        0x0017
```

The problem is in the `-nographic` QEMU option. With it QEMU transforms all special keys to escape sequences like it happens in any console terminal.

If you want to see 'native' behaviour you should run QEMU in graphic mode:
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk
```
If your system doesn't support native graphics, you can run QEMU with a VNC option. QEMU will create a VNC server on the port that you've provided, and you can connect to that port via VNC client app (like VNC viewer for example https://www.realvnc.com/en/connect/download/viewer/).

This command would produce VNC server on a `127.0.0.1:1` address:
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk \
  -vnc :1
```

If you connect to QEMU launched with graphics you can observe that the "PageUp" press produces expected code:

![PageUp](PageUp.png?raw=true "PageUp scan code on QEMU with graphics")

