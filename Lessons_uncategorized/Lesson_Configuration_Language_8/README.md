When we load our HII form drivers with `efivarstore` for the first time, we need to create a variable for the form storage. Up until now we've used the following pattern in the driver's entry point code:
```cpp
UINTN BufferSize;
UEFI_VARIABLE_STRUCTURE EfiVarstore;
BufferSize = sizeof(UEFI_VARIABLE_STRUCTURE);
Status = gRT->GetVariable (
              UEFIVariableName,
              &UEFIVariableGuid,
              NULL,
              &BufferSize,
              &EfiVarstore);
if (EFI_ERROR(Status)) {
  ZeroMem(&EfiVarstore, sizeof(EfiVarstore));					<---- Data structure initialization
  Status = gRT->SetVariable(
                UEFIVariableName,
                &UEFIVariableGuid,
                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                sizeof(EfiVarstore),
                &EfiVarstore);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't create variable! %r\n", Status);
  }
}
```
Here we create an UEFI variable for the form storage structure if it is not already present in the system. When we create a new variable we use `ZeroMem` for the structure initialization. This is why when we look at our form for the first time we see that checkbox is unset, numeric is equal to 0, string element is empty and so on.

If we want to start not from this, but from some sane defaults, instead of `ZeroMem` we can use some custom function that would initialize the structure fields as we want to. But the thing is that we already have a method to declare default values. VFR syntax allow us to set defaults for the form elements with the `default` keyword. You already know how to set elements to these defaults interactively from the Form Browser. But what if we want to start from these values?

Let's investigate how we can use the `default` data from the VFR to initialize form fields non-interactively.

Basically we already know that. We need to:
- use `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExtractConfig()` to get Form storage configuration,
- strip default configuration part (`ALTCFG=0000&...`) from the response,
- use it to constuct configuration request for the `EFI_HII_CONFIG_ROUTING_PROTOCOL.RouteConfig()` function to set elements to their default values.

Stripping the necessary part from the configuration string can be a burden, this is why UEFI specification offers a helper function for that:
```
EFI_HII_CONFIG_ROUTING_PROTOCOL.GetAltCfg()

Summary:
This helper function is to be called by drivers to extract portions of a larger configuration string.

Prototype:
typedef
EFI_STATUS
 (EFIAPI * EFI_HII_GET_ALT_CFG ) (
 IN CONST EFI_HII_CONFIG_ROUTING_PROTOCOL *This,
 IN CONST EFI_STRING ConfigResp,
 IN CONST EFI_GUID *Guid,
 IN CONST EFI_STRING Name,
 IN CONST EFI_DEVICE_PATH_PROTOCOL *DevicePath,
 IN CONST EFI_STRING AltCfgId,
 OUT EFI_STRING *AltCfgResp
 );

Parameters:
This		Points to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance
ConfigResp	A null-terminated string in <ConfigAltResp> format
Guid		A pointer to the GUID value to search for in the routing portion of the ConfigResp
		string when retrieving the requested data. If Guid is NULL, then all GUID values will be searched for
Name		A pointer to the NAME value to search for in the routing portion of the ConfigResp
		string when retrieving the requested data. If Name is NULL, then all Name values will be searched for
DevicePath	A pointer to the PATH value to search for in the routing portion of the ConfigResp
		string when retrieving the requested data. If DevicePath is NULL, then all DevicePath values will be searched for
AltCfgId	A pointer to the ALTCFG value to search for in the routing portion of the
		ConfigResp string when retrieving the requested data. If this parameter is NULL, then the current setting will be retrieved
AltCfgResp	A pointer to a buffer which will be allocated by the function which contains the
		retrieved string as requested. This buffer is only allocated if the call was successful.
		The null-terminated string will be in <ConfigResp> format

Description:
This function retrieves the requested portion of the configuration string from a larger configuration string. This function will use the Guid, Name, and DevicePath parameters to find the appropriate
section of the ConfigResp string. Upon finding this portion of the string, it will use the AltCfgId parameter to find the appropriate instance of data in the ConfigResp string. Once found, the found
data will be copied to a buffer which is allocated by the function so that it can be returned to the caller. The caller is responsible for freeing this allocated buffer.
```

With that our steps would be:
- use `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExtractConfig()` to get Form storage configuration
- use `EFI_HII_CONFIG_ROUTING_PROTOCOL.GetAltCfg()` to get necessary default configuration
- use `EFI_HII_CONFIG_ROUTING_PROTOCOL.RouteConfig()` to set elements to their default values

But fortunately we don't have to do any of that as `HiiLib` offers us the `HiiSetToDefaults` function [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiHiiLib/HiiLib.c](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiHiiLib/HiiLib.c):
```
/**
  Reset the default value specified by DefaultId to the driver
  configuration got by Request string.
  NULL request string support depends on the ExportConfig interface of
  HiiConfigRouting protocol in UEFI specification.
  @param Request    A null-terminated Unicode string in
                    <MultiConfigRequest> format. It can be NULL.
                    If it is NULL, all configuration for the
                    entirety of the current HII database will be reset.
  @param DefaultId  Specifies the type of defaults to retrieve.
  @retval TRUE    The default value is set successfully.
  @retval FALSE   The default value can't be found and set.
**/
BOOLEAN
EFIAPI
HiiSetToDefaults (
  IN CONST EFI_STRING  Request   OPTIONAL,
  IN UINT16            DefaultId
  )
```
So all we need to do is to construct configuration header for the driver storage and call this function. And practically we already know how to do it.

# `HIIFormDataElementsWithDefaultsSet`

To keep lessons separate I would create a new driver `HIIFormDataElementsWithDefaultsSet`. But basically it based on our `HIIFormDataElements` driver code, so I would explain only things that differ.

We need to add new code only if the respective UEFI variable is not set:
```cpp
Status = gRT->GetVariable(...)
if (EFI_ERROR(Status)) {
  ZeroMem(...);
  Status = gRT->SetVariable(...);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't create variable! %r\n", Status);
  }
  <...>                                                         <------ add code that would set defaults to form elements
}
```
As in this code we would call `EFI_HII_CONFIG_ROUTING_PROTOCOL` functions expecting output from our form, the code above must be after the `HiiAddPackages` call:
```cpp
mHiiHandle = HiiAddPackages(...);

Status = gRT->GetVariable(...)
if (EFI_ERROR(Status)) {
  ZeroMem(...);
  Status = gRT->SetVariable(...);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't create variable! %r\n", Status);
  }
  <...>
}
```
I point out that fact, because earlier we did things in the opposite way.

Now let's get to our initialization code. As in our driver code we already have the `DriverHandle` and storage `GUID` and `Name` we can just call `HiiConstructConfigHdr` function to create necessary confugartion header string. After that we just use it with necessary `DefaultId` number to set defaults. Let's use 0 as `DefaultId` to set standard defaults:
```
EFI_STRING ConfigStr = HiiConstructConfigHdr(&UEFIVariableGuid, UEFIVariableName, mDriverHandle);
UINT16 DefaultId = 0;
if (!HiiSetToDefaults(ConfigStr, DefaultId)) {
  Print(L"Error! Can't set default configuration #%d\n", DefaultId);
}
```
This is all we need to add. And if we want to set `Manufacture defaults`, all we need to do is replace `DefaultId = 0` to `DefaultId = 1`.

If you build our driver and load it in UEFI shell, you'll see that now the respective form is filled with default values even at the first launch:

![1](1.png?raw=true "1")

