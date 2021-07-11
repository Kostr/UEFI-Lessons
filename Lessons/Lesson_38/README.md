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

Usually package contain headers for protocols in a folder:
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

Include this file in our *.c file, so it would itself also know that is the `SIMPLE_CLASS_PROTOCOL` type:
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

Now it time to write an app that would use our protocol. As this protocol is not installed to the app image handle, we first need to find all handles with such protocol with a help of `LocateHandleBuffer` API, and then call `OpenProtocol` on every one of this handles.

This is no different as it was some other protocol from the other edk2 packages.

UefiLessonsPkg/SimpleClassUser/SimpleClassUser.c
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

If we execute our
```
FS0:\> SimpleClassUser.efi
Error! Can't find any handle with gSimpleClassProtocolGuid: Not Found
```

