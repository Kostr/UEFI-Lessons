Up until now we've only used protocols.

Let's write a driver that would actually produce a protocol.

As a protocol a similar to the class, we'll call our simple protocol `SimpleClass`.

Our protocol would have 2 functions - simple getter and setter for internal class variable `mNumber`. `m` in `mNumber` in this case denotes that this variable is local to a module.

UefiLessonsPkg/SimpleClassProtocol/SimpleClassProtocol.c:
```
UINTN mNumber = 0;

EFI_STATUS
EFIAPI
SimpleClassProtocolSetNumber (
  UINTN Number
  )
{
  mNumber = Number;

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
SimpleClassProtocolGetNumber (
  UINTN* Number
  )
{
  if (Number == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Number = mNumber;

  return EFI_SUCCESS;
}

SIMPLE_CLASS_PROTOCOL  mSimpleClass = {
  SimpleClassProtocolGetNumber,
  SimpleClassProtocolSetNumber
};
```
In C syntax there is no such keyword as a `class`, but still it is possible to create something similar. We use `struct` keyword and class methods would be just fields with pointers to functions.

Now we need to create a header file that would contain the `SIMPLE_CLASS_PROTOCOL` type definition.

Usually packages contain headers for protocols in a folder:
```
<pkg>/Include/Protocol/
```

Therefore create a header UefiLessonsPkg/Include/Protocol/SimpleClass.h:
```
#ifndef __SIMPLE_CLASS_PROTOCOL_H__
#define __SIMPLE_CLASS_PROTOCOL_H__


typedef struct _SIMPLE_CLASS_PROTOCOL  SIMPLE_CLASS_PROTOCOL;

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


#endif
```

Include this file in our *.c file, so it would itself also know what is the `SIMPLE_CLASS_PROTOCOL` type:
```
#include <Protocol/SimpleClass.h>
```

To make protocol actually usable by other modules we need to install it in the system (protocol database).

For this task we can use `InstallProtocolInterface` function:
```
EFI_BOOT_SERVICES.InstallProtocolInterface()

Summary
Installs a protocol interface on a device handle. If the handle does not exist, it is created and added to the
list of handles in the system.

Prototype
typedef
EFI_STATUS
(EFIAPI *EFI_INSTALL_PROTOCOL_INTERFACE) (
 IN OUT EFI_HANDLE *Handle,
 IN EFI_GUID *Protocol,
 IN EFI_INTERFACE_TYPE InterfaceType,
 IN VOID *Interface
 );

Parameters
Handle 		A pointer to the EFI_HANDLE on which the interface is to be installed. If *Handle is NULL on input,
		a new handle is created and returned on output. If *Handle is not NULL on input, the protocol is
		added to the handle, and the handle is returned unmodified.
		If *Handle is not a valid handle, then EFI_INVALID_PARAMETER is returned.
Protocol 	The numeric ID of the protocol interface.
InterfaceType 	Indicates whether Interface is supplied in native form.
Interface 	A pointer to the protocol interface. The Interface must adhere to the structure defined by Protocol.
		NULL can be used if a structure is not associated with Protocol.

Description
The InstallProtocolInterface() function installs a protocol interface (a GUID/Protocol Interface structure pair) on a device handle.
The same GUID cannot be installed more than once onto the same handle.
```

You can find `InstallProtocolInterface` all over in the edk2 codebase, but actually according to the same UEFI spec this API is outdated.
The suggested API is:
```
EFI_BOOT_SERVICES.InstallMultipleProtocolInterfaces()

Summary:
Installs one or more protocol interfaces into the boot services environment.

Prototype:
typedef
EFI_STATUS
EFIAPI *EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES) (
 IN OUT EFI_HANDLE *Handle,
 ...
 );

Parameters:
Handle 		The pointer to a handle to install the new protocol interfaces on, or a pointer to NULL if a new handle is to be
		allocated.
...		A variable argument list containing pairs of protocol GUIDs and protocol interfaces.

Description:
This function installs a set of protocol interfaces into the boot services environment. It removes
arguments from the variable argument list in pairs. The first item is always a pointer to the protocol’s
GUID, and the second item is always a pointer to the protocol’s interface. These pairs are used to call the
boot service EFI_BOOT_SERVICES.InstallProtocolInterface() to add a protocol interface to
Handle. If Handle is NULL on entry, then a new handle will be allocated. The pairs of arguments are
removed in order from the variable argument list until a NULL protocol GUID value is found.
```


As we are writing driver in our example, let's use `InstallMultipleProtocolInterfaces` in a function `SimpleClassProtocolDriverEntryPoint` that we will declare as entry point in the driver INF file:
```
EFI_HANDLE  mSimpleClassHandle = NULL;

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
Here I've also added `Print` statements that shows `mSimpleClassHandle` address.

`mSimpleClassHandle` and `mSimpleClass` are declared in the same `*.c` file (therefore the `m` prefix), and `gSimpleClassProtocolGuid` (`g` stands for global) we would declare in a package DEC file so any others modules could include this file and use our protocol.

UefiLessonsPkg/UefiLessonsPkg.dec:
```
[Protocols]
  gSimpleClassProtocolGuid = { 0xb5510eea, 0x6f11, 0x4e4b, { 0xad, 0x0f, 0x35, 0xce, 0x17, 0xbd, 0x7a, 0x67 }}
```

Finally it is time to write INF file:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = SimpleClassProtocol
  FILE_GUID                      = 51d6a90a-c021-4472-b2c1-5fdd1b7f2196
  MODULE_TYPE                    = UEFI_DRIVER
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = SimpleClassProtocolDriverEntryPoint      <---- entry point that actually calls 'InstallMultipleProtocolInterfaces'

[Sources]
  SimpleClassProtocol.c

[Packages]
  MdePkg/MdePkg.dec
  UefiLessonsPkg/UefiLessonsPkg.dec     <---- we need to add this to get access to the GUID value 'gSimpleClassProtocolGuid'

[Protocols]
  gSimpleClassProtocolGuid     <---- protocols that are used in module

[LibraryClasses]
  UefiDriverEntryPoint
  UefiLib
```

# SimpleClassUser

Now it time to write an app that would use our protocol. As this protocol is not installed to the app image handle, we first need to find all handles with such protocol with a help of `LocateHandleBuffer` API, and then call `OpenProtocol` on every one of these handles.

This is no different as it was for some other protocols from the other edk2 packages.

UefiLessonsPkg/SimpleClassUser/SimpleClassUser.c:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/SimpleClass.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN                  HandleCount;
  EFI_HANDLE*            HandleBuffer;
  UINTN Index;
  SIMPLE_CLASS_PROTOCOL* SimpleClass;

  EFI_STATUS Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gSimpleClassProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    Print(L"Error! Can't find any handle with gSimpleClassProtocolGuid: %r\n", Status);
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Print(L"Handle = %p\n", HandleBuffer[Index]);
    Status = gBS->OpenProtocol(
      HandleBuffer[Index],
      &gSimpleClassProtocolGuid,
      (VOID **)&SimpleClass,
      ImageHandle,
      NULL,
      EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );

    if (!EFI_ERROR(Status)) {
      <...>
    } else {
      Print(L"Error! Can't open SimpleClass protocol: %r\n", Status);
    }
  }

  return Status;
}
```

We would use our protocol this way:
- get and print current number value
- add 5 to the number value and set it
- get and print current number value
```
UINTN Number;

Status = SimpleClass->GetNumber(&Number);
if (!EFI_ERROR(Status)) {
  Print(L"Number before=%d\n", Number);
} else {
  Print(L"Error! Can't get number: %r\n", Status);
}

Status = SimpleClass->SetNumber(Number+5);
if (EFI_ERROR(Status))
  Print(L"Error! Can't set number: %r\n", Status);

Status = SimpleClass->GetNumber(&Number);
if (!EFI_ERROR(Status)) {
  Print(L"Number after=%d\n", Number);
} else {
  Print(L"Error! Can't get number: %r\n", Status);
}
```

INF file for the protocol user app would be pretty simple UefiLessonsPkg/SimpleClassUser/SimpleClassUser.inf:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = SimpleClassUser
  FILE_GUID                      = 466eed70-8def-44ea-9fb4-9012b266ec8c
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = UefiMain

[Sources]
  SimpleClassUser.c

[Packages]
  MdePkg/MdePkg.dec
  UefiLessonsPkg/UefiLessonsPkg.dec         <---- we need to add it to get access to the 'gSimpleClassProtocolGuid'

[Protocols]
  gSimpleClassProtocolGuid        <----- add used protocol guids

[LibraryClasses]
  UefiApplicationEntryPoint
  UefiLib
```

Add both `SimpleClassProtocol` and `SimpleClassUser` to the `[Components]` section of our package:
```
[Components]
  ...
  UefiLessonsPkg/SimpleClassProtocol/SimpleClassProtocol.inf
  UefiLessonsPkg/SimpleClassUser/SimpleClassUser.inf
```

# Testing

Now build, copy results to QEMU shared folder and run OVMF.

If we execute our `SimpleClassUser` first, we would get an error:
```
FS0:\> SimpleClassUser.efi
Error! Can't find any handle with gSimpleClassProtocolGuid: Not Found
```

It is understandable, our protocol is not yet installed in the system. Let's install it and use our app.
```
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver, handle=6695318
Image 'FS0:\SimpleClassProtocol.efi' loaded at 6645000 - Success
FS0:\> SimpleClassUser.efi
Handle = 6695318
Number before=0
Number after=5
```

We can see handles for our driver and its protocol with `dh`:
```
FS0:\> dh
...
C6: ImageDevicePath(..C1)/\SimpleClassProtocol.efi) LoadedImage(\SimpleClassProtocol.efi)
C7: B5510EEA-6F11-4E4B-AD0F-35CE17BD7A67
```

If we execute `SimpleClassUser.efi` again, the number would increase from 5 to 10. It means that the `number` value is really stored outside `SimpleClassUser.efi`. As you can guess, it is stored in a protocol with a handle `C7`.
```
FS0:\> SimpleClassUser.efi
Handle = 6695318
Number before=5
Number after=10
```

We can load another copy of our `SimpleClassProtocol.efi` driver:
```
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver, handle=6635498
Image 'FS0:\SimpleClassProtocol.efi' loaded at 6631000 - Success
```
Now there would be two protocol handles in the system C7 and C9:
```
FS0:\> dh
...
C6: ImageDevicePath(..C1)/\SimpleClassProtocol.efi) LoadedImage(\SimpleClassProtocol.efi)
C7: B5510EEA-6F11-4E4B-AD0F-35CE17BD7A67
C8: ImageDevicePath(..C1)/\SimpleClassProtocol.efi) LoadedImage(\SimpleClassProtocol.efi)
C9: B5510EEA-6F11-4E4B-AD0F-35CE17BD7A67
```

If we execute our `SimpleClassUser.efi` application again, we'll see, that 2 separate version of `number` exist in the system:
```
FS0:\> SimpleClassUser.efi
Handle = 6695318
Number before=10
Number after=15
Handle = 6635498
Number before=0
Number after=5
```

We didn't implement unload function in our driver yet, so it is not possible to unload them.

# IMAGE_UNLOAD

Let's implemet unload function in our protocol driver (UefiLessonsPkg/SimpleClassProtocol/SimpleClassProtocol.inf):
```
[Defines]
   ...
  UNLOAD_IMAGE                   = SimpleClassProtocolDriverUnload
```

For now write it like this (UefiLessonsPkg/SimpleClassProtocol/SimpleClassProtocol.c):
```
EFI_STATUS
EFIAPI
SimpleClassProtocolDriverUnload (
  IN EFI_HANDLE        ImageHandle
  )
{
  Print(L"Bye-bye from SimpleClassProtocol driver, handle=%p\n", mSimpleClassHandle);
  return EFI_SUCCESS;
}
```

Let's test again. Load driver and execute our app once:
```
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver, handle=665B618
Image 'FS0:\SimpleClassProtocol.efi' loaded at 6646000 - Success
FS0:\> SimpleClassUser.efi
Handle = 665B618
Number before=0
Number after=5
FS0:\> dh
...
C6: ImageDevicePath(..C1)/\SimpleClassProtocol.efi) LoadedImage(\SimpleClassProtocol.efi)
C7: B5510EEA-6F11-4E4B-AD0F-35CE17BD7A67
```

It is not possible to perform unload for the protocol handler (C7), but as we've implemented unload function for our driver let's to try unload it:
```
FS0:\> unload c7
Unload - Handle [665B618].  [y/n]?
y
Unload - Handle [665B618] Result Invalid Parameter.
FS0:\> unload c6
Unload - Handle [665FF18].  [y/n]?
y
Bye-bye from SimpleClassProtocol driver, handle=665B618
Unload - Handle [665FF18] Result Success.
```

But there is a problem - C7 is still in the system:
```
FS0:\> dh
...
C7: B5510EEA-6F11-4E4B-AD0F-35CE17BD7A67
```

Now if we try to execute our app we would get an exception. `OpenProtocol` call would give the same address for the protocol, but now this memory is freed, and a call to protocol functions like `SimpleClass->GetNumber` would crash the system:
```
FS0:\> SimpleClassUser.efi
Handle = 665B618
!!!! X64 Exception Type - 0D(#GP - General Protection)  CPU Apic ID - 00000000 !!!!
ExceptionData - 0000000000000000
RIP  - AFAFAFAFAFAFAFAF, CS  - 0000000000000038, RFLAGS - 0000000000000246
RAX  - 0000000006647740, RCX - 0000000007EBC468, RDX - 0000000000000000
RBX  - 00000000079EE018, RSP - 0000000007EBC418, RBP - 0000000007EBC468
RSI  - 0000000000000000, RDI - 000000000665EA18
R8   - 00000000000000AF, R9  - 000000000665EA18, R10 - 000000008005C440
R11  - 0000000000000000, R12 - 0000000000000000, R13 - 00000000066353E4
R14  - 0000000007EBC460, R15 - 0000000006636040
DS   - 0000000000000030, ES  - 0000000000000030, FS  - 0000000000000030
GS   - 0000000000000030, SS  - 0000000000000030
CR0  - 0000000080010033, CR2 - 0000000000000000, CR3 - 0000000007C01000
CR4  - 0000000000000668, CR8 - 0000000000000000
DR0  - 0000000000000000, DR1 - 0000000000000000, DR2 - 0000000000000000
DR3  - 0000000000000000, DR6 - 00000000FFFF0FF0, DR7 - 0000000000000400
GDTR - 00000000079DE000 0000000000000047, LDTR - 0000000000000000
IDTR - 00000000072AD018 0000000000000FFF,   TR - 0000000000000000
FXSAVE_STATE - 0000000007EBC070
```

To fix this we need to uninstall our protocol from the system on the driver unload.

# Uninstall protocol interface

Like with the protocol interface install there are two functions in UEFI API for the protocol uninstallation. The obsolete one `UninstallProtocolInterface` and a new one `UninstallMultipleProtocolInterfaces`:
```
EFI_BOOT_SERVICES.UninstallProtocolInterface()

Summary:
Removes a protocol interface from a device handle. It is recommended that UninstallMultipleProtocolInterfaces() be used in place of
UninstallProtocolInterface().

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_UNINSTALL_PROTOCOL_INTERFACE) (
 IN EFI_HANDLE Handle,
 IN EFI_GUID *Protocol,
 IN VOID *Interface
 );

Parameters:
Handle 		The handle on which the interface was installed. If Handle is not a
		valid handle, then EFI_INVALID_PARAMETER is returned.
Protocol 	The numeric ID of the interface.
Interface 	A pointer to the interface. NULL can be used if a structure is not associated with Protocol.

Description:
The UninstallProtocolInterface() function removes a protocol interface from the handle on 
which it was previously installed. The Protocol and Interface values define the protocol interface to
remove from the handle.
If the last protocol interface is removed from a handle, the handle is freed and is no longer valid.
```

```
EFI_BOOT_SERVICES.UninstallMultipleProtocolInterfaces()

Summary:
Removes one or more protocol interfaces into the boot services environment.

Prototype:
typedef
EFI_STATUS
EFIAPI *EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES) (
 IN EFI_HANDLE Handle,
 ...
 );

Parameters:
Handle 		The handle to remove the protocol interfaces from.
...		A variable argument list containing pairs of protocol GUIDs and protocol interfaces.

Description:
This function removes a set of protocol interfaces from the boot services environment. It removes
arguments from the variable argument list in pairs. The first item is always a pointer to the protocol’s
GUID, and the second item is always a pointer to the protocol’s interface. These pairs are used to call the
boot service EFI_BOOT_SERVICES.UninstallProtocolInterface() to remove a protocol
interface from Handle. The pairs of arguments are removed in order from the variable argument list until
a NULL protocol GUID value is found
```

Let's call `UninstallMultipleProtocolInterfaces` on the driver unload:
```
EFI_STATUS
EFIAPI
SimpleClassProtocolDriverUnload (
  IN EFI_HANDLE        ImageHandle
  )
{
  Print(L"Bye-bye from SimpleClassProtocol driver, handle=%p\n", mSimpleClassHandle);

  EFI_STATUS Status = gBS->UninstallMultipleProtocolInterfaces(
                             mSimpleClassHandle,
                             &gSimpleClassProtocolGuid,
                             &mSimpleClass,
                             NULL
                             );

  return Status;
}
```

Now build and perform our experiments again.

Load driver and execute our app once:
```
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver, handle=665F118
Image 'FS0:\SimpleClassProtocol.efi' loaded at 6645000 - Success
FS0:\> SimpleClassUser.efi
Handle = 665F118
Number before=0
Number after=5
FS0:\> dh
...
C6: ImageDevicePath(..C1)/\SimpleClassProtocol.efi) LoadedImage(\SimpleClassProtocol.efi)
C7: B5510EEA-6F11-4E4B-AD0F-35CE17BD7A67
```
Load another copy of the driver and execute our app again:
```
FS0:\> load SimpleClassProtocol.efi
Hello from SimpleClassProtocol driver, handle=6636898
Image 'FS0:\SimpleClassProtocol.efi' loaded at 6630000 - Success
FS0:\> SimpleClassUser.efi
Handle = 665F118
Number before=5
Number after=10
Handle = 6636898
Number before=0
Number after=5
FS0:\> dh
...
C6: ImageDevicePath(..C1)/\SimpleClassProtocol.efi) LoadedImage(\SimpleClassProtocol.efi)
C7: B5510EEA-6F11-4E4B-AD0F-35CE17BD7A67
C8: ImageDevicePath(..C1)/\SimpleClassProtocol.efi) LoadedImage(\SimpleClassProtocol.efi)
C9: B5510EEA-6F11-4E4B-AD0F-35CE17BD7A67
```
Now there are two protocols/drivers in the system. Let's try to unload the first one.
```
FS0:\> unload c6
Unload - Handle [665F018].  [y/n]?
y
Bye-bye from SimpleClassProtocol driver, handle=665F118
Unload - Handle [665F018] Result Success.
FS0:\> dh
C8: ImageDevicePath(..C1)/\SimpleClassProtocol.efi) LoadedImage(\SimpleClassProtocol.efi)
C9: B5510EEA-6F11-4E4B-AD0F-35CE17BD7A67
```
If we execute our app again, we would see, that that now there is only one handle in the system with the `SimpleClass` protocol, it's valid, and it was completely undisturbed by another protocol uninstall process:
```
FS0:\> SimpleClassUser.efi
Handle = 6636898
Number before=5
Number after=10
```

