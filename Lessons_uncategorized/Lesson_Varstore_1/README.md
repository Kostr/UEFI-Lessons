Up until now we've used `EFI Variable Storage` to store our HII form settings. In VFR this type of storage is encoded with the `efivarstore` object. UEFI specification defines `EFI Variable Storage` like this:
```
EFI Variable Storage
This is a specialized form of Buffer Storage, which uses the EFI runtime services GetVariable() and SetVariable() to access the entire buffer defined for the Variable Store as a single binary object.
```
So in this case form just uses the global UEFI variable storage to store/retrieve the settings. But sometimes it is not a desired behaviour.

For example, imagine that you have an external PCIe device that via it's Option ROM adds some forms to the HII subsystem. Would it be right to store these form settings in the global UEFI variable storage? The answer is no. Imagine that some time later you decide that this device is not for you and remove it. But if its settings were stored in the global UEFI variable storage, they will be left hanging there forever. Therefore it is more right to store device settings on the device itself. Moreover in this case you can configure the device once and move it between systems without any need to reconfigure. But in the case of a `efivarstore` it is not possible as it can only use the global UEFI variable storage for the form settings.

Another example when `efivarstore` is not enough would be the case where we need to perform some non-trivial operations to get/set form data. For the simple example imagine that you have a LED controlled by GPIO on your board and you want to create a checkbox that would allow to see the current state of a LED and enable/disable it. You'll need something more than `efivarstore` to create such form.

Luckily the UEFI specification offers us another type of storage - `Buffer Storage`:
```
Buffer Storage
With buffer storage, the application, platform or driver provides the definition of a buffer which contains the values for one or more questions. The size of the entire
buffer is defined in the EFI_IFR_VARSTORE definition. Each question defines a field in the buffer by providing an offset within the buffer and the size of the required
storage. These variable stores are exposed by the app/driver using the EFI_HII_CONFIG_ACCESS_PROTOCOL, which is installed on the same handle as the package list.
Question values are retrieved via EFI_HII_CONFIG_ACCESS_PROTOCOL.ExtractConfig() and updated via EFI_HII_CONFIG_ACCESS_PROTOCOL.RouteConfig(). Rather than access the
buffer as a whole, Buffer Storage Variable Stores access each field independently, via a list of one or more (field offset, value) pairs encoded as variable length text
strings as defined for the EFI_HII_CONFIG_ACCESS_PROTOCOL
```
Let me try to explain this definition.

Remember how we accessed manually HII forms from CLI with our `HIIConfig.efi` application?
- to get form data we've used `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExtractConfig()` function. This function sends configuration request string and returns configuration response and progress strings
- to set form data we've used `EFI_HII_CONFIG_ROUTING_PROTOCOL.RouteConfig()` function. This function sends configuration request string and returns progress string

In case of a `EFI Variable Storage` these functions implicitly use the `GetVariable()`/`SetVariable()` API to get/set the necessary data.

But in case of a `varstore` you have all the control of the underlying behaviour because you write the handling code for the `ExtractConfig()`/`RouteConfig()` cases yourself!

How is it happening? You publish a special protocol with your custom handling functions. This protocol is called `EFI_HII_CONFIG_ACCESS_PROTOCOL`:
```
EFI_HII_CONFIG_ACCESS_PROTOCOL

Summary:
The EFI HII configuration routing protocol invokes this type of protocol when it needs to forward requests to a driver's configuration handler. This protocol is published by drivers providing and receiving configuration data from HII. The ExtractConfig() and RouteConfig() functions are typically invoked by the driver which implements the HII Configuration Routing Protocol. The Callback() function is typically invoked by the Forms Browser.

GUID:
#define EFI_HII_CONFIG_ACCESS_PROTOCOL_GUID \
 { 0x330d4706, 0xf2a0, 0x4e4f,\
 {0xa3,0x69, 0xb6, 0x6f,0xa8, 0xd5, 0x43, 0x85}}

Protocol Interface Structure:
typedef struct {
 EFI_HII_ACCESS_EXTRACT_CONFIG ExtractConfig;
 EFI_HII_ACCESS_ROUTE_CONFIG RouteConfig;
 EFI_HII_ACCESS_FORM_CALLBACK Callback;
} EFI_HII_CONFIG_ACCESS_PROTOCOL;

Parameters:
ExtractConfig	This function breaks apart the request strings routing them to the appropriate drivers. This function is analogous to the similarly named function in the HII Routing Protocol
RouteConfig	This function breaks apart the results strings and returns configuration information as specified by the request
Callback	This function is called from the configuration browser to communicate certain activities that were initiated by a user

Description:
This protocol provides a callable interface between the HII and drivers. Only drivers which provide IFR data to HII are required to publish this protocol.
```
Basically it works like this. When you call `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExtractConfig()` to get the form settings, under the hood the `EFI_HII_CONFIG_ROUTING_PROTOCOL` will redirect the call to your `EFI_HII_CONFIG_ACCESS_PROTOCOL.ExtractConfig()` function. The same is for the `RouteConfig()` case.
```
EFI_HII_CONFIG_ROUTING_PROTOCOL.ExtractConfig() <-> EFI_HII_CONFIG_ACCESS_PROTOCOL.ExtractConfig()
EFI_HII_CONFIG_ROUTING_PROTOCOL.RouteConfig()   <-> EFI_HII_CONFIG_ACCESS_PROTOCOL.RouteConfig()
```

The API of the ExtractConfig() and RouteConfig() functions for these two protocols is completely identical:
```
typedef                                                     typedef
EFI_STATUS                                                  EFI_STATUS
 (EFIAPI * EFI_HII_EXTRACT_CONFIG ) (                        (EFIAPI * EFI_HII_ACCESS_EXTRACT_CONFIG ) (
 IN CONST EFI_HII_CONFIG_ROUTING_PROTOCOL *This,             IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This,
 IN CONST EFI_STRING Request,                                IN CONST EFI_STRING Request,
 OUT EFI_STRING *Progress,                                   OUT EFI_STRING *Progress,
 OUT EFI_STRING *Results                                     OUT EFI_STRING *Results
 );                                                          );



typedef                                                     typedef
EFI_STATUS                                                  EFI_STATUS
 (EFIAPI * EFI_HII_ROUTE_CONFIG ) (                         (EFIAPI * EFI_HII_ACCESS_ROUTE_CONFIG ) (
 IN CONST EFI_HII_CONFIG_ROUTING_PROTOCOL *This,             IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This,
 IN CONST EFI_STRING Configuration,                          IN CONST EFI_STRING Configuration,
 OUT EFI_STRING *Progress                                    OUT EFI_STRING *Progress
 );                                                          );
```

We've already familiar with the `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExtractConfig()`/`EFI_HII_CONFIG_ROUTING_PROTOCOL.RouteConfig()` functions. And we know what configuration strings we can send and what answer should we expect from that. This is a functionality that our `EFI_HII_CONFIG_ACCESS_PROTOCOL.ExtractConfig()`/`EFI_HII_CONFIG_ACCESS_PROTOCOL.RouteConfig()` functions need to implement. We will get to the details later when we would actually write them.

But generally this is what you should do in these functions:

`EFI_HII_CONFIG_ACCESS_PROTOCOL.ExtractConfig()`:
- parse configuration request string (`EFI_STRING Request`)
- read necessary storage settings
- return appropriate configuration response (`EFI_STRING *Results`) and progress strings (`EFI_STRING *Progress`)
`EFI_HII_CONFIG_ACCESS_PROTOCOL.RouteConfig()`:
- parse configuration request string (`EFI_STRING Configuration`)
- write necessary storage settings
- return progress string (`EFI_STRING *Progress`)

As you see, in the driver's `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExtractConfig()`/`EFI_HII_CONFIG_ROUTING_PROTOCOL.RouteConfig()` functions you have a complete freedom about how you read/write storage settings. You just need to fill the output parameters correctly. This additional freedom can solve both problem raised at the beginning of the lesson. Now nothing stops you from storing the settings on the external device or creating a form that would get/set hardware dynamically.

# `varstore`

Let's create a driver that would use the `Buffer Storage` for it's form data.

To make things familiar, our example would provide the same form as in the `HIIFormDataElements` driver case, but just use the `Buffer Storage` instead of the `EFI Variable Storage`.

As in the VFR code the buffer storage is defined with a help of a `varstore` keyword, we would name our new application `HIIFormDataElementsVarstore`.

There would be many similarities between the `HIIFormDataElementsVarstore` and `HIIFormDataElements`, so I would only describe the things that need changes.

For example VFR code will only need to change `efivarstore` to `varstore`, leaving the rest the same:
```
- efivarstore UEFI_VARIABLE_STRUCTURE,
-   attribute = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
-   name  = FormData,
-   guid  = STORAGE_GUID;

+ varstore VARIABLE_STRUCTURE,
+   name  = FormData,
+   guid  = STORAGE_GUID;
```
As you see `varstore` definition doesn't define variable attributes, because the storage is not limited now to the global UEFI variable storage. Because of that we also change the underlying structure name to the `VARIABLE_STRUCTURE`. Don't forget to replace this name in other places.

# `EFI_IFR_VARSTORE`

Let's look at the new code generated from our updated VFR `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIFormDataElementsVarstore/HIIFormDataElementsVarstore/DEBUG/Form.lst`:
```
  varstore VARIABLE_STRUCTURE,
>00000033: 24 1F 92 75 80 37 3A 73 1B 4F 95 57 F2 2A F7 43 E8 C2 01 00 24 00 46 6F 72 6D 44 61 74 61 00
    name = FormData,
    guid = {0x37807592, 0x733a, 0x4f1b, {0x95, 0x57, 0xf2, 0x2a, 0xf7, 0x43, 0xe8, 0xc2}};
```

In case you don't remember the first two bytes is `EFI_IFR_OP_HEADER`:
```cpp
typedef struct _EFI_IFR_OP_HEADER {
 UINT8 OpCode;
 UINT8 Length:7;
 UINT8 Scope:1;
} EFI_IFR_OP_HEADER;
```

In our case `OpCode` is equal to `0x24`, which corresponds to the `EFI_IFR_VARSTORE`:
```
EFI_IFR_VARSTORE

Summary:
Creates a variable storage short-cut for linear buffer storage

Prototype:

#define EFI_IFR_VARSTORE_OP 0x24

typedef struct {
 EFI_IFR_OP_HEADER Header;
 EFI_GUID Guid;
 EFI_VARSTORE_ID VarStoreId;
 UINT16 Size;
 //UINT8 Name[];
} EFI_IFR_VARSTORE;

Members:
Header 		The byte sequence that defines the type of opcode as well as the length of the opcode being defined. For this tag, Header.OpCode = EFI_IFR_VARSTORE_OP
Guid 		The variable’s GUID definition. This field comprises one half of the variable name, with the other half being the humanreadable aspect of the name, which is represented by the
		string immediately following the Size field.
VarStoreId 	The variable store identifier, which is unique within the current form set. This field is the value that uniquely identify this instance from others. Question headers refer to this value
		to designate which is the active variable that is being used. A value of zero is invalid.
Size 		The size of the variable store.
Name 		A null-terminated ASCII string that specifies the name associated with the variable store. The field is not actually included in the structure but is included here to help illustrate
		the encoding of the opcode. The size of the string, including the null termination, is included in the opcode's header size.

Description:
This opcode describes a Buffer Storage Variable Store within a form set. A question can select this variable store by setting the VarStoreId field in its opcode header. An EFI_IFR_VARSTORE with a specified VarStoreId must appear in the IFR before it can be referenced by a question.
```

So the next bytes is the `EFI_IFR_VARSTORE` structure filled with the following values:
```
typedef struct {
 EFI_IFR_OP_HEADER Header;	// 24 1F
 EFI_GUID Guid;			// 92 75 80 37 3A 73 1B 4F 95 57 F2 2A F7 43 E8 C2 (={0x37807592, 0x733a, 0x4f1b, {0x95, 0x57, 0xf2, 0x2a, 0xf7, 0x43, 0xe8, 0xc2}})
 EFI_VARSTORE_ID VarStoreId;    // 01 00 (=0x0001)
 UINT16 Size;                   // 24 00
 //UINT8 Name[];                // 46 6F 72 6D 44 61 74 61 00  ("FormData")
} EFI_IFR_VARSTORE;
```

# `EntryPoint`/`Unload`

Now let's get to the main C module of our driver. In our driver we would need to create and publish `EFI_HII_CONFIG_ACCESS_PROTOCOL`. As we would use it in multiple places we define it as a global variable at the start of the file:
```cpp
EFI_HII_CONFIG_ACCESS_PROTOCOL mConfigAccess;
```
In the driver's EntryPoint we need to fill this protocol functions and publish it to the system. Also like before we need to install DevicePath protocol and add necessary HII packages. So our code would look like this:
```cpp
EFI_STATUS
EFIAPI
HIIFormDataElementsVarstoreEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  mConfigAccess.ExtractConfig = &ExtractConfig;			// <--- reference ExtractConfig/RouteConfig/Callback functions that we would define later
  mConfigAccess.RouteConfig   = &RouteConfig;
  mConfigAccess.Callback      = &Callback;

  EFI_STATUS Status;
  Status = gBS->InstallMultipleProtocolInterfaces(
                  &mDriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,             // <--- add created mConfigAccess protocol
                  &mConfigAccess,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mHiiHandle = HiiAddPackages(
                 &gEfiCallerIdGuid,
                 mDriverHandle,
                 HIIFormDataElementsVarstoreStrings,
                 FormBin,
                 NULL
                 );
  if (mHiiHandle == NULL) {
    gBS->UninstallMultipleProtocolInterfaces(
           mDriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mHiiVendorDevicePath,
           &gEfiHiiConfigAccessProtocolGuid,
           &mConfigAccess,
           NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}
```

Don't forget to add `#include <Protocol/HiiConfigAccess.h>` to the start of the file and this code to the INF file:
```
[Protocols]
  gEfiHiiConfigAccessProtocolGuid
```

In the driver's unload function you need to remove HII packages and uninstall all the added protocols:
```cpp
EFI_STATUS
EFIAPI
HIIFormDataElementsVarstoreUnload (
  EFI_HANDLE ImageHandle
  )
{
  if (mHiiHandle != NULL)
    HiiRemovePackages(mHiiHandle);

  EFI_STATUS Status = gBS->UninstallMultipleProtocolInterfaces(
                             mDriverHandle,
                             &gEfiDevicePathProtocolGuid,
                             &mHiiVendorDevicePath,
                             &gEfiHiiConfigAccessProtocolGuid,
                             &mConfigAccess,
                             NULL);

  return Status;
}
```

Now all we left to do is to write `ExtractConfig`/`RouteConfig`/`Callback` functions.

