In the last lesson we've added our own boot option.

Let's boot it.

Type `exit` in the UEFI shell to go to the BIOS menu:
```
Shell> exit
```
Navigate to the `Boot Manager` window. Our `HelloWorld` option would be present here:
![Boot Manager](BootManager.png?raw=true "Boot manager")

If you'll try to boot it, app would print its strings and immediately return.

Let's fix it. Let's add wait for a key input from user in our app code.
To do it we will need `WaitForEvent` blocking function:

```
EFI_BOOT_SERVICES.WaitForEvent()

Summary
Stops execution until an event is signaled.

Prototype
typedef

EFI_STATUS
(EFIAPI *EFI_WAIT_FOR_EVENT) (
 IN UINTN NumberOfEvents,
 IN EFI_EVENT *Event,
 OUT UINTN *Index
);

Parameters
NumberOfEvents 	The number of events in the Event array.
Event 		An array of EFI_EVENT.
Index 		Pointer to the index of the event which satisfied the wait condition.
```

To our task we should use `EFI_EVENT WaitForKey` from the `EFI_SIMPLE_TEXT_INPUT_PROTOCOL` (which is placed in a `gST->ConIn`)
```
typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
 EFI_INPUT_RESET Reset;
 EFI_INPUT_READ_KEY ReadKeyStroke;
 EFI_EVENT WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
```

Now when we now all this API add this code to the end of the `HelloWorld` app main function:
```
UINTN Index;
gBS->WaitForEvent(1, &(gST->ConIn->WaitForKey), &Index);
```

Compile OVMF again and try to boot our `HelloWorld` boot option through the `Boot Manager` BIOS menu.
Now output of the app stops and waits for the user keystroke.

Everything works fine except a fact that if we choose `Enter` as our keystroke, `HelloWorld` app would be immediately launched again.

This happens because we didn't read or clear the input buffer. As we don't need keystroke information let's simply reset it with a `Reset` function:
```
EFI_SIMPLE_TEXT_INPUT_PROTOCOL.Reset()

Summary:
Resets the input device hardware.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_INPUT_RESET) (
 IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
 IN BOOLEAN ExtendedVerification
 );

Parameters:
This 			A pointer to the EFI_SIMPLE_TEXT_INPUT_PROTOCOL instance.
ExtendedVerification	Indicates that the driver may perform a more exhaustive verification
			operation of the device during reset.

Description:
The Reset() function resets the input device hardware.
The implementation of Reset is required to clear the contents of any input queues resident in memory
used for buffering keystroke data and put the input stream in a known empty state
```

Add this to the end of our app:
```
gST->ConIn->Reset(gST->ConIn, FALSE);
```

Now if we boot our app from the `Boot Manager` BIOS menu it would work correctly for any keystroke.

