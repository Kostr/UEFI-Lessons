In this lesson we would investigate the `EFI_BOOT_SERVICES.RegisterProtocolNotify()` function. This API can be usefull to create callbacks that are executed when certain protocols are installed in the system.

For our investigation let's create a simple driver `ProtocolEventDriver`:
```
./createNewDriver.sh ProtocolEventDriver
```

Add the following code to create a callback function `NotifyFunc` executed when the local `EFI_EVENT Event` is signaled:
```cpp
EFI_EVENT Event;
UINTN NotifyData = 0;

VOID EFIAPI NotifyFunc(EFI_EVENT Event, VOID* Context)
{
  ...
}


EFI_STATUS
EFIAPI
ProtocolEventDriverUnload (
  EFI_HANDLE ImageHandle
  )
{
  gBS->CloseEvent(Event);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ProtocolEventDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  Status = gBS->CreateEvent(EVT_NOTIFY_SIGNAL,
                            TPL_NOTIFY,
                            &NotifyFunc,
                            &NotifyData,
                            &Event);
  if (EFI_ERROR(Status)) {
    Print(L"Error! CreateEvent returned: %r\n", Status);
    return Status;
  }

  return EFI_SUCCESS;
}
```

Here we pass `UINTN NotifyData` to the callback function. Let's increment it inside the callback code, so we can understand how many times the callback was called:
```cpp
  if (Context == NULL)
    return;

  Print(L"\nEvent is signaled! Context = %d\n", *(UINTN*)Context);
  *(UINTN*)Context += 1;
```

Right now no one can signal our event, therefore the callback code would never be executed. Let's register the protocol install notifier. Here we would monitor installation of our own `SIMPLE_CLASS_PROTOCOL` which we've created in the earlier lesson.

In case you don't remember check out the protocol API in the `UefiLessonsPkg/Include/Protocol/SimpleClass.h`
```cpp
typedef
EFI_STATUS
(EFIAPI* SIMPLE_CLASS_GET_NUMBER)(
  UINTN* Number
  );


typedef
EFI_STATUS
(EFIAPI* SIMPLE_CLASS_SET_NUMBER)(
  UINTN Number
  );


struct _SIMPLE_CLASS_PROTOCOL {
  SIMPLE_CLASS_GET_NUMBER GetNumber;
  SIMPLE_CLASS_SET_NUMBER SetNumber;
};
```

In the nutshell this protocol is a simple abstraction to the `UINTN Number` field.

Now to the `RegisterProtocolNotify()`. Here is its description from the UEFI specification:
```
EFI_BOOT_SERVICES.RegisterProtocolNotify()

Summary:
Creates an event that is to be signaled whenever an interface is installed for a specified protocol.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_REGISTER_PROTOCOL_NOTIFY) (
  IN EFI_GUID *Protocol,
  IN EFI_EVENT Event,
  OUT VOID **Registration
);

Parameters:
Protocol     - The numeric ID of the protocol for which the event is to be registered
Event        - Event that is to be signaled whenever a protocol interface is registered for Protocol.
               The same EFI_EVENT may be used for multiple protocol notify registrations
Registration - A pointer to a memory location to receive the registration value. This value must be saved and used by the notification function
               of Event to retrieve the list of handles that have added a protocol interface of type Protocol

Description:
The RegisterProtocolNotify() function creates an event that is to be signaled whenever a protocol interface is installed for Protocol by InstallProtocolInterface() or EFI_BOOT_SERVICES.ReinstallProtocolInterface().

Once Event has been signaled, the EFI_BOOT_SERVICES.LocateHandle() function can be called to identify the newly installed, or reinstalled, handles that support Protocol. The Registration parameter in EFI_BOOT_SERVICES.RegisterProtocolNotify() corresponds to the SearchKey parameter in LocateHandle().
```

And this is how we would use it:
```cpp
#include <Protocol/SimpleClass.h>

STATIC VOID  *mRegistrationTracker;

EFI_STATUS
EFIAPI
ProtocolEventDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  ...

  Status = gBS->RegisterProtocolNotify(&gSimpleClassProtocolGuid,
                                       Event,
                                       &mRegistrationTracker);
  if (EFI_ERROR(Status)) {
    Print(L"Error! RegisterProtocolNotify returned: %r\n", Status);
    return Status;
  }

  return EFI_SUCCESS;
}
```
Don't mind the `Registration` parameter for now, we'll investigate it later.

Also don't forget to add the necessary code to the `INF` file to be able to use our `SIMPLE_CLASS_PROTOCOL` in the driver code:
```
[Packages]
  ...
  UefiLessonsPkg/UefiLessonsPkg.dec

[Protocols]
  gSimpleClassProtocolGuid
```

Now when we load `SimpleClassProtocol.efi` driver (created in the earlier lesson) that installs the `SIMPLE_CLASS_PROTOCOL` protocol we would see that our callback code is executed:
```
FS0:\> load ProtocolEventDriver.efi
Image 'FS0:\ProtocolEventDriver.efi' loaded at 6415000 - Success
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver
Event is signaled! Context = 0
, handle=640FB98
Image 'FS0:\SimpleClassProtocol.efi' loaded at 640C000 - Success
```

The strings
```
Hello from SimpleClassProtocol driver
, handle=640FB98
```

Are printed from the `SimpleClassProtocol` driver. Here is its code:
```cpp
EFI_STATUS
EFIAPI
SimpleClassProtocolDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print(L"Hello from SimpleClassProtocol driver");

  EFI_STATUS Status = gBS->InstallMultipleProtocolInterfaces(
                             &mSimpleClassHandle,
                             &gSimpleClassProtocolGuid,
                             &mSimpleClass,
                             NULL
                             );
  if (!EFI_ERROR(Status))
    Print(L", handle=%p\n", mSimpleClassHandle);
  else
    Print(L"\n", mSimpleClassHandle);

  return Status;
}
```

You can see how the `\nEvent is signaled! Context = 0\n` from the `ProtocolEventDriver` was printed in between the prints of `SimpleClassProtocol` driver. That means that as soon as the `SIMPLE_CLASS_PROTOCOL` was installed, our callback was executed interrupting the `SimpleClassProtocol` driver code.

You can execute the `load SimpleClassProtocol.efi` again and see that our callback would be called on each protocol install:
```
FS0:\> load ProtocolEventDriver.efi
Image 'FS0:\ProtocolEventDriver.efi' loaded at 6415000 - Success
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver
Event is signaled! Context = 0
, handle=640FB98
Image 'FS0:\SimpleClassProtocol.efi' loaded at 640C000 - Success
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver
Event is signaled! Context = 1
, handle=641AB18
Image 'FS0:\SimpleClassProtocol.efi' loaded at 6408000 - Success
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver
Event is signaled! Context = 2
, handle=6419918
Image 'FS0:\SimpleClassProtocol.efi' loaded at 63E3000 - Success
```

You can also see that as we've coded earlier, the `UINTN NotifyData` (aka `Context`) is incremented on each callback execution.

Now let's check the `dh` output:
```
FS0:\> dh
...
A4: ImageDevicePath(..C1)/\ProtocolEventDriver.efi) LoadedImage(\ProtocolEventDriver.efi)
A5: ImageDevicePath(..C1)/\SimpleClassProtocol.efi) LoadedImage(\SimpleClassProtocol.efi)
A6: B5510EEA-6F11-4E4B-AD0F-35CE17BD7A67
A7: ImageDevicePath(..C1)/\SimpleClassProtocol.efi) LoadedImage(\SimpleClassProtocol.efi)
A8: B5510EEA-6F11-4E4B-AD0F-35CE17BD7A67
A9: ImageDevicePath(..C1)/\SimpleClassProtocol.efi) LoadedImage(\SimpleClassProtocol.efi)
AA: B5510EEA-6F11-4E4B-AD0F-35CE17BD7A67
```
You can see that each `load SimpleClassProtocol.efi` command installs a separate `SIMPLE_CLASS_PROTOCOL` protocol to the system.

Now let's try to actually use the `SIMPLE_CLASS_PROTOCOL` inside the callback function. We would use the `gBS->LocateProtocol` to find the protocol and perform `+5` operation on its internal number via the protocol `SIMPLE_CLASS_GET_NUMBER/SIMPLE_CLASS_SET_NUMBER` functions.
```cpp
VOID EFIAPI NotifyFunc(EFI_EVENT Event, VOID* Context)
{
  ...

  EFI_STATUS Status;
  SIMPLE_CLASS_PROTOCOL* SimpleClass;
  Status = gBS->LocateProtocol(&gSimpleClassProtocolGuid,
                               NULL,
                               (VOID**)&SimpleClass);
  if (EFI_ERROR(Status)) {
    Print(L"Error! LocateProtocol returned: %r\n", Status);
    return;
  }

  UINTN Number;
  Status = SimpleClass->GetNumber(&Number);
  if (!EFI_ERROR(Status)) {
    Print(L"Current number = %d\n", Number);
  } else {
    Print(L"Error! Can't get number: %r\n", Status);
    return;
  }

  Status = SimpleClass->SetNumber(Number+5);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't set number: %r\n", Status);
    return;
  }
}
```

This is what we would get from this code:
```
FS0:\> load ProtocolEventDriver.efi
Image 'FS0:\ProtocolEventDriver.efi' loaded at 6415000 - Success
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver
Event is signaled! Context = 0
Current number = 0
, handle=640FB98
Image 'FS0:\SimpleClassProtocol.efi' loaded at 640C000 - Success
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver
Event is signaled! Context = 1
Current number = 5
, handle=646A818
Image 'FS0:\SimpleClassProtocol.efi' loaded at 6408000 - Success
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver
Event is signaled! Context = 2
Current number = 10
, handle=6419918
Image 'FS0:\SimpleClassProtocol.efi' loaded at 63E3000 - Success
```

You can see the problem here. Each time we perform `load SimpleClassProtocol.efi` we install additional protocol to the system. We saw that in the `dh` command output.
But `gBS->LocateProtocol` always finds the first installed protocol. So how can we call the callback code on the newly installed protocol that caused the callback in the first place?

To fix that we can utilize `Registration` parameter of the `RegisterProtocolNotify/LocateProtocol` functions.

With it the `LocateProtocol` function would return the next handle that is new for the registration.

Here is a description of the `EFI_BOOT_SERVICES.LocateProtocol()` function from the specification. We've already saw it, and used it many times, but now pay attention to the `Registration` parameter:
```
EFI_BOOT_SERVICES.LocateProtocol()

Summary:
Returns the first protocol instance that matches the given protocol.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_PROTOCOL) (
  IN EFI_GUID *Protocol,
  IN VOID *Registration OPTIONAL,
  OUT VOID **Interface
);

Parameters:
Protocol      - Provides the protocol to search for
Registration  - Optional registration key returned from EFI_BOOT_SERVICES.RegisterProtocolNotify(). If Registration is NULL, then it is ignored
Interface     - On return, a pointer to the first interface that matches Protocol and Registration

Description:
The LocateProtocol() function finds the first device handle that support Protocol, and returns a pointer to the protocol interface from that handle in Interface. If no protocol instances are found, then Interface is set to NULL.

If Registration is not NULL, and there are no new handles for Registration, then EFI_NOT_FOUND is returned.
```

All we need to do now to fix the problem is to use `mRegistrationTracker` instead of `NULL` in the `gBS->LocateProtocol` call. In the nutshell the registration variable is used in 3 places:
```cpp
STATIC VOID  *mRegistrationTracker;                                     // <-----

VOID EFIAPI NotifyFunc(EFI_EVENT Event, VOID* Context)
{
  ...
  Status = gBS->LocateProtocol(&gSimpleClassProtocolGuid,
                               mRegistrationTracker,                    // <-----
                               (VOID**)&SimpleClass);
  ...
}

...

EFI_STATUS
EFIAPI
ProtocolEventDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  ...
  Status = gBS->RegisterProtocolNotify(&gSimpleClassProtocolGuid,
                                       Event,
                                       &mRegistrationTracker);          // <-----
  ...
}
```

Now on each callback we would work with the newly installed protocol:
```
FS0:\> load ProtocolEventDriver.efi
Image 'FS0:\ProtocolEventDriver.efi' loaded at 6415000 - Success
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver
Event is signaled! Context = 0
Current number = 0
, handle=640FB98
Image 'FS0:\SimpleClassProtocol.efi' loaded at 640C000 - Success
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver
Event is signaled! Context = 1
Current number = 0
, handle=646C218
Image 'FS0:\SimpleClassProtocol.efi' loaded at 6408000 - Success
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver
Event is signaled! Context = 2
Current number = 0
, handle=6419918
Image 'FS0:\SimpleClassProtocol.efi' loaded at 63E3000 - Success
```

# `EfiCreateProtocolNotifyEvent`

To ease creation process of the protocol notification callbacks the [UefiLib](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/UefiLib.h) offers a `EfiCreateProtocolNotifyEvent` function:
```cpp
/**
  Creates and returns a notification event and registers that event with all the protocol
  instances specified by ProtocolGuid.

  This function causes the notification function to be executed for every protocol of type
  ProtocolGuid instance that exists in the system when this function is invoked. If there are
  no instances of ProtocolGuid in the handle database at the time this function is invoked,
  then the notification function is still executed one time. In addition, every time a protocol
  of type ProtocolGuid instance is installed or reinstalled, the notification function is also
  executed. This function returns the notification event that was created.
  If ProtocolGuid is NULL, then ASSERT().
  If NotifyTpl is not a legal TPL value, then ASSERT().
  If NotifyFunction is NULL, then ASSERT().
  If Registration is NULL, then ASSERT().


  @param  ProtocolGuid    Supplies GUID of the protocol upon whose installation the event is fired.
  @param  NotifyTpl       Supplies the task priority level of the event notifications.
  @param  NotifyFunction  Supplies the function to notify when the event is signaled.
  @param  NotifyContext   The context parameter to pass to NotifyFunction.
  @param  Registration    A pointer to a memory location to receive the registration value.
                          This value is passed to LocateHandle() to obtain new handles that
                          have been added that support the ProtocolGuid-specified protocol.

  @return The notification event that was created.

**/
EFI_EVENT
EFIAPI
EfiCreateProtocolNotifyEvent (
  IN  EFI_GUID          *ProtocolGuid,
  IN  EFI_TPL           NotifyTpl,
  IN  EFI_EVENT_NOTIFY  NotifyFunction,
  IN  VOID              *NotifyContext   OPTIONAL,
  OUT VOID              **Registration
  )
```

This API abstracts the calling of `CreateEvent/RegisterProtocolNotify` functions.

Keep in mind that `EfiCreateProtocolNotifyEvent` also immediately signals the callback function `NotifyFunction` manually right after the `RegisterProtocolNotify` call. So your notification function can be executed even if there are no target protocols in the system.

You can see how the `EfiCreateProtocolNotifyEvent` can simplify our main code:
```cpp
EFI_STATUS
EFIAPI
ProtocolEventDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Event = EfiCreateProtocolNotifyEvent(&gSimpleClassProtocolGuid,
                                       TPL_NOTIFY,
                                       &NotifyFunc,
                                       &NotifyData,
                                       &mRegistrationTracker);
  return EFI_SUCCESS;
}
```

If you'll test this application you'll see that opposed to our own code before, here the created event is signaled one time even when there are no `SIMPLE_CLASS_PROTOCOL` protocols in the system.
```
FS0:\> load ProtocolEventDriver.efi

Event is signaled! Context = 0                                      <-------
Error! LocateProtocol returned: Not Found                           <-------
Image 'FS0:\ProtocolEventDriver.efi' loaded at 6415000 - Success
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver
Event is signaled! Context = 1
Current number = 0
, handle=640FB98
Image 'FS0:\SimpleClassProtocol.efi' loaded at 640C000 - Success
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver
Event is signaled! Context = 2
Current number = 0
, handle=641A918
Image 'FS0:\SimpleClassProtocol.efi' loaded at 6408000 - Success
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver
Event is signaled! Context = 3
Current number = 0
, handle=6419918
Image 'FS0:\SimpleClassProtocol.efi' loaded at 63E3000 - Success
```
