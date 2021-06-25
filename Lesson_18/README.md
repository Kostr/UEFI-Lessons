
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

With this app you can also look at how special keys are handled.
This is the output from "PageUp" key:
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

