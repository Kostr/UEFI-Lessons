In this lesson we would create a driver that would register callbacks for hot key combinations.

In UEFI in is possible to do with a help of `RegisterKeyNotify` function in `EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL`:
```
EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL.RegisterKeyNotify()

Summary:
Register a notification function for a particular keystroke for the input device.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_REGISTER_KEYSTROKE_NOTIFY) (
 IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
 IN EFI_KEY_DATA *KeyData,
 IN EFI_KEY_NOTIFY_FUNCTION KeyNotificationFunction,
 OUT VOID **NotifyHandle
 );

Parameters:
This 				A pointer to the EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL instance.
KeyData 			A pointer to a buffer that is filled in with the keystroke information
				for the key that was pressed. If KeyData.Key, KeyData.KeyState.KeyToggleState and
				KeyData.KeyState.KeyShiftState are 0, then any incomplete keystroke will trigger a
				notification of the KeyNotificationFunction.
KeyNotificationFunction		Points to the function to be called when the key sequence is typed
				specified by KeyData.
NotifyHandle 			Points to the unique handle assigned to the registered notification.

Description:
The RegisterKeystrokeNotify() function registers a function which will be called when a specified
keystroke will occur. The keystroke being specified can be for any combination of KeyData.Key or
KeyData.KeyState information.
```

The prototype for the callback function defined as:
```
typedef
EFI_STATUS
(EFIAPI *EFI_KEY_NOTIFY_FUNCTION) (
 IN EFI_KEY_DATA *KeyData
 );
```

As for `EFI_KEY_DATA` it is a structure with two fields:
```
typedef struct {
 EFI_INPUT_KEY Key;            // The EFI scan code and Unicode value returned from the input device.
 EFI_KEY_STATE KeyState;       // The current state of various toggled attributes as well as input modifier values.
} EFI_KEY_DATA
```

The `EFI_INPUT_KEY` type is defined in the file https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/SimpleTextIn.h:
```
typedef struct {
  UINT16  ScanCode;
  CHAR16  UnicodeChar;
} EFI_INPUT_KEY;
```
We've already used it in our `InteractiveApp` application.

`EFI_KEY_STATE` type is new, you can find in the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/SimpleTextInEx.h:
```
///
/// EFI_KEY_TOGGLE_STATE. The toggle states are defined.
/// They are: EFI_TOGGLE_STATE_VALID, EFI_SCROLL_LOCK_ACTIVE
/// EFI_NUM_LOCK_ACTIVE, EFI_CAPS_LOCK_ACTIVE
///
typedef UINT8 EFI_KEY_TOGGLE_STATE;

typedef struct _EFI_KEY_STATE {
  ///
  /// Reflects the currently pressed shift
  /// modifiers for the input device. The
  /// returned value is valid only if the high
  /// order bit has been set.
  ///
  UINT32                KeyShiftState;
  ///
  /// Reflects the current internal state of
  /// various toggled attributes. The returned
  /// value is valid only if the high order
  /// bit has been set.
  ///
  EFI_KEY_TOGGLE_STATE  KeyToggleState;
} EFI_KEY_STATE;
```

To get an understanding of what information is coded in these `KeyShiftState`/`KeyToggleState` fields take a look at these defines from the same file:
```
//
// Any Shift or Toggle State that is valid should have
// high order bit set.
//
// Shift state
//
#define EFI_SHIFT_STATE_VALID     0x80000000
#define EFI_RIGHT_SHIFT_PRESSED   0x00000001
#define EFI_LEFT_SHIFT_PRESSED    0x00000002
#define EFI_RIGHT_CONTROL_PRESSED 0x00000004
#define EFI_LEFT_CONTROL_PRESSED  0x00000008
#define EFI_RIGHT_ALT_PRESSED     0x00000010
#define EFI_LEFT_ALT_PRESSED      0x00000020
#define EFI_RIGHT_LOGO_PRESSED    0x00000040
#define EFI_LEFT_LOGO_PRESSED     0x00000080
#define EFI_MENU_KEY_PRESSED      0x00000100
#define EFI_SYS_REQ_PRESSED       0x00000200

//
// Toggle state
//
#define EFI_TOGGLE_STATE_VALID    0x80
#define EFI_KEY_STATE_EXPOSED     0x40
#define EFI_SCROLL_LOCK_ACTIVE    0x01
#define EFI_NUM_LOCK_ACTIVE       0x02
#define EFI_CAPS_LOCK_ACTIVE      0x04
```

Also these edk2 files (https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/SimpleTextIn.h and https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/SimpleTextInEx.h) contain defines for the scan codes (up, down, right, left keys, home, end, delete, f1-f24, etc.). So if you'll ever what to use scan code for some exotic button, take a look in those files.

Finally it is time to write code for our driver.

Our driver would be called `HotKeyDriver`. It would register hot key combinations in its entry point function and unregister them in the unload function.

UefiLessonsPkg/HotKeyDriver/HotKeyDriver.inf:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = HotKeyDriver
  FILE_GUID                      = da316635-c66f-477e-9df6-880d2d729f1b
  MODULE_TYPE                    = UEFI_DRIVER
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = HotKeyDriverEntryPoint
  UNLOAD_IMAGE                   = HotKeyDriverUnload
  
[Sources]
  HotKeyDriver.c

[Packages]
  MdePkg/MdePkg.dec

[Protocols]
  gEfiSimpleTextInputExProtocolGuid            # <----- guid for EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL

[LibraryClasses]
  UefiDriverEntryPoint
  UefiLib
```

First in our driver entry point function we need to locate `EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL` by its guid `gEfiSimpleTextInputExProtocolGuid` (UefiLessonsPkg/HotKeyDriver/HotKeyDriver.c):
```
#include <Protocol/SimpleTextInEx.h>

EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* InputEx = NULL;

EFI_STATUS
EFIAPI
HotKeyDriverEntryPoint(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status = gBS->LocateProtocol(
			 &gEfiSimpleTextInputExProtocolGuid,
			 NULL,
			 (VOID**)&InputEx
			 );
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't locate EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL: %r", Status);
    return Status;
  }

  ...

  return EFI_SUCCESS;
}
```

As we would need `EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL` in the unload function as well, we declare variable for it as global.

Now it is time to use `RegisterKeyNotify` API. Let's create a callback that would be fired at `left ctrl + left alt + z` combination:
```
  EFI_KEY_DATA HotKey;
  HotKey.Key.ScanCode = 0;
  HotKey.Key.UnicodeChar = L'z';
  HotKey.KeyState.KeyShiftState = EFI_LEFT_CONTROL_PRESSED | EFI_LEFT_ALT_PRESSED | EFI_SHIFT_STATE_VALID;
  HotKey.KeyState.KeyToggleState = 0;

  Status = InputEx->RegisterKeyNotify(InputEx,
			&HotKey,
			MyKeyNotificationFunction,
                        (VOID**)&NotifyHandle);

  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't perform RegisterKeyNotify: %r", Status);
    return Status;
  }
```

In the begining of our file we add a global variable for `NotifyHandle` (again, we need it global as it would be used in driver unload function) and define our `MyKeyNotificationFunction` callback function:
```
EFI_HANDLE NotifyHandle;

EFI_STATUS EFIAPI MyKeyNotificationFunction(EFI_KEY_DATA* KeyData)
{
	Print(L"\nHot Key 1 is pressed\n");         // we add '\n' in the begining, so the output wouldn't interfere with the UEFI shell prompt
	return EFI_SUCCESS;
}
```

Let's write another callback that would be fired on any `z` press if numlock is active:
```
  HotKey.KeyState.KeyShiftState = 0;
  HotKey.KeyState.KeyToggleState = EFI_TOGGLE_STATE_VALID | EFI_NUM_LOCK_ACTIVE | EFI_KEY_STATE_EXPOSED;
  Status = InputEx->RegisterKeyNotify(InputEx,
			&HotKey,
			MyKeyNotificationFunction1,
                        (VOID**)&NotifyHandle1);

  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't perform RegisterKeyNotify: %r", Status);
    return Status;
  }
```
```
EFI_HANDLE NotifyHandle1;

EFI_STATUS EFIAPI MyKeyNotificationFunction1(EFI_KEY_DATA* KeyData)
{
	Print(L"\nHot Key 2 is pressed\n");     // we add '\n' in the begining, so the output wouldn't interfere with the UEFI shell prompt
	return EFI_SUCCESS;                       // or another our callback function
}
```

I've already mentioned couple of times, that we need to unregister callback function on driver unload. Why do we need that?

Because these functions `MyKeyNotificationFunction` and `MyKeyNotificationFunction1` are defined in the `HotKeyDriver` memory. And as soon as we unload `HotKeyDriver` from memory, this memory would become freed/invalid. And on a hot key combination press system would try to dereference a callback function from the pointer to this invalid region. This would lead to an exception.

For the unregister operation EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL defines `UnregisterKeyNotify` function:
```
EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL.UnregisterKeyNotify()

Summary:
Remove the notification that was previously registered.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_UNREGISTER_KEYSTROKE_NOTIFY) (
 IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
 IN VOID *NotificationHandle
 );

Parameters:
This 			A pointer to the EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL instance.
NotificationHandle	The handle of the notification function being unregistered.

Description:
The UnregisterKeystrokeNotify() function removes the notification which was previously registered. 
```

Let's use it to unregister our callbacks:
```
EFI_STATUS
EFIAPI
HotKeyDriverUnload(
  IN EFI_HANDLE        ImageHandle
  )
{
  if (!InputEx)
    return EFI_SUCCESS;

  EFI_STATUS Status;

  if (!NotifyHandle) {
    Status = InputEx->UnregisterKeyNotify(InputEx,
                                   (VOID*)NotifyHandle);

    if (EFI_ERROR(Status)) {
      Print(L"Error! Can't perform RegisterKeyNotify: %r", Status);
      return Status;
    }
  }

  if (!NotifyHandle1) {
    Status = InputEx->UnregisterKeyNotify(InputEx,
                       (VOID*)NotifyHandle1);

    if (EFI_ERROR(Status)) {
      Print(L"Error! Can't perform RegisterKeyNotify: %r", Status);
      return Status;
    }
  }

  return EFI_SUCCESS;
}
```

Now it is time to build and test our driver.

To avoid terminal translations of non-ordinary keys to escape sequences we shouldn't run our QEMU with `-nographic` option as we usually do.

We can either boot it with native graphics, or with graphics through the vnc server:
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk
```
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk
  -vnc :1
```

Once we boot QEMU, load our driver into OVMF:
```
FS0:\> load HotKeyDriver.efi
Image 'FS0:\HotKeyDriver.efi' loaded at 6646000 - Success
```

If numlock is disabled only one callback is possible:
- `left ctrl + left alt + z` produces
```
FS0:\>
Hot Key 1 is pressed
```
If numlock is enabled:
- `z` produces `Hot Key 2 is pressed`
```
FS0:\>
Hot Key 2 is pressed
```
- `left ctrl + left alt + z` produces both prints:
```
FS0:\>
Hot Key 1 is pressed
Hot Key 2 is pressed
```

Don't forget to flush your screen with `cls` command in UEFI shell between different keystroke combinations, so you woudn't miss any new output.

Also be aware that if QEMU is run in `-nographic` mode all the modifires are not parsed. So every `z` keystroke would always result with both callbacks:
```
FS0:\> z
Hot Key 1 is pressed
Hot Key 2 is pressed
```

