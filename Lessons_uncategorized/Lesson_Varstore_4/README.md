# Make `EFI_HII_CONFIG_ACCESS_PROTOCOL.ExtractConfig()` compatible with UEFI specification

Here is some description from the UEFI specification regarding `EFI_HII_CONFIG_ACCESS_PROTOCOL.ExtractConfig()` function:
```
If a NULL is passed in for the Request field, all of the settings being abstracted by this function will be returned in the Results field. In addition, if a ConfigHdr is
passed in with no request elements, all of the settings being abstracted for that particular ConfigHdr reference will be returned in the Results Field.
```

Currently our `ExtractConfig` implementation doesn't cover this corner cases. It simply uses `BlockToConfig` function, which needs `OFFSET=<...>&WIDTH=<...>` pairs for it to function correctly:
```cpp
STATIC
EFI_STATUS
EFIAPI
ExtractConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Request,
  OUT       EFI_STRING                      *Progress,
  OUT       EFI_STRING                      *Results
)
{
  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Here can be additional custom operations to get you VARIABLE_STRUCTURE data

  EFI_STATUS Status = gHiiConfigRouting->BlockToConfig(gHiiConfigRouting,
                                                       Request,
                                                       (UINT8*)&FormStorage,
                                                       sizeof(VARIABLE_STRUCTURE),
                                                       Results,
                                                       Progress);

  return Status;
}
```

But we've just seen with our `HIIConfig.efi` application that every possible call works correctly. Why is that? Let's explore this.

## No `OFFSET=<...>&WIDTH=<...>` pairs in the request

I've tried to debug the code on the driver side while using `HIIConfig.efi` and the interesting fact is that even if our `HIIConfig` program sends
```
GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400
```
The driver gets:
```
GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=0000&WIDTH=0001&OFFSET=0001&WIDTH=0002&OFFSET=0003&WIDTH=0014&OFFSET=0019&WIDTH=0004&OFFSET=001d&WIDTH=0003&OFFSET=0020&WIDTH=0001&OFFSET=0021&WIDTH=0003
```

This transformation of the request is implicit and is happening inside the `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExtartConfig()` function [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/HiiDatabaseDxe/ConfigRouting.c](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/HiiDatabaseDxe/ConfigRouting.c)

This is a reason why our driver handles requests without `OFFSET=<...>&WIDTH=<...>` pairs without any errors.

## `Request == NULL`

It is not possible to issue `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExtractConfig()` with a `NULL` in place of a configuration request. Function will simply fail.

But you've seen that our driver is called with a `NULL` request when `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExportConfig()` function is executed. As we don't cover this case (`BlockToConfig` with `NULL` request simply returns error), the `ExportConfig()` issues the full request. And that request our driver can handle. So once again we don't see any visible errors if we don't implement `NULL` handling functionality.

Currently `Request == NULL` handling can only be seen as an optimization for the `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExportConfig()` call.

## `ExtractConfig()` changes

Even if we've seen that currently our driver is good with EDKII, we should respect UEFI specification requirements. In other case EDKII can change it's behaviour sometime in the future and our driver will just stop working.

So let's add this functionality. It can be done like this:
```cpp
STATIC
EFI_STATUS
EFIAPI
ExtractConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Request,
  OUT       EFI_STRING                      *Progress,
  OUT       EFI_STRING                      *Results
)
{
  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  BOOLEAN AllocatedRequest = FALSE;
  EFI_STRING ConfigRequest = Request;                                       <--- by default our ConfigRequest that we would use with BlockToConfig function is equal to Request argument

  if ((Request == NULL) || (StrStr (Request, L"OFFSET") == NULL)) {         <--- check if Request argument is not enough for the direct BlockToConfig call
    EFI_STRING ConfigRequestHdr = HiiConstructConfigHdr(&StorageGuid, StorageName, mDriverHandle);
    UINTN Size = (StrLen(ConfigRequestHdr) + StrLen(L"&OFFSET=0&WIDTH=") + sizeof(UINT64)*2 + 1) * sizeof(CHAR16);  <--- sizeof(VARIABLE_STRUCTURE) is UINTN=UINT64, therefore to encode it we need 'sizeof(UINT64)*2' symbols
    ConfigRequest = AllocateZeroPool(Size);
    AllocatedRequest = TRUE;
    UnicodeSPrint(ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, sizeof(VARIABLE_STRUCTURE));  <--- ConfigRequest = "Request + &OFFSET=0&WIDTH=XXXXXXXXXXXXXXXX"
    FreePool(ConfigRequestHdr);
  }

  EFI_STATUS Status = gHiiConfigRouting->BlockToConfig(gHiiConfigRouting,
                                                       ConfigRequest,				<--- here we use ConfigRequest that can be "Request" or "Request + &OFFSET=0&WIDTH=XXXXXXXXXXXXXXXX"
                                                       (UINT8*)&FormStorage,
                                                       sizeof(VARIABLE_STRUCTURE),
                                                       Results,
                                                       Progress);

  if (AllocatedRequest) {                                                   <--- if we've generated ConfigRequest string we need to free it
    FreePool(ConfigRequest);
    if (Request == NULL) {                                                  <--- also in this case BlockToConfig sets the Progress pointer to the ConfigRequest, but we need to sets this pointer to original Request
      *Progress = NULL;
    } else if (StrStr(Request, L"OFFSET") == NULL) {
      *Progress = Request + StrLen(Request);
    }
  }

  return Status;
}
```
The pattern above is widely used in the EDKII. When you would investigate other drivers keep in mind that usually instead of writing `StrLen(L"&OFFSET=0&WIDTH=") + sizeof(UINT64)*2` the driver developers just write its calculated value, which is 32.

# `HiiIsConfigHdrMatch`

The last thing that is usually seen in the driver code is the check that `GUID` and `NAME` in the request inedeed match the current driver. The check is performed with the `HiiIsConfigHdrMatch` function from the `HiiLib` [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiHiiLib/HiiLib.c](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiHiiLib/HiiLib.c)
```cpp
/**
  Determines if the routing data specified by GUID and NAME match a <ConfigHdr>.
  If ConfigHdr is NULL, then ASSERT().
  @param[in] ConfigHdr  Either <ConfigRequest> or <ConfigResp>.
  @param[in] Guid       GUID of the storage.
  @param[in] Name       NAME of the storage.
  @retval TRUE   Routing information matches <ConfigHdr>.
  @retval FALSE  Routing information does not match <ConfigHdr>.
**/
BOOLEAN
EFIAPI
HiiIsConfigHdrMatch (
  IN CONST EFI_STRING  ConfigHdr,
  IN CONST EFI_GUID    *Guid      OPTIONAL,
  IN CONST CHAR16      *Name      OPTIONAL
  )
```
This function expects that incoming configuration string is not equal to `NULL`, therefore the it can be used like this:
```cpp
if ((Request != NULL) && !HiiIsConfigHdrMatch(Request, &StorageGuid, StorageName)) {
  return EFI_NOT_FOUND;
}
```
`HiiIsConfigHdrMatch` is more useful when you driver have several storages and you check which one currently is requested.

Anyway with this check our final version for the `ExportConfig` function looks like this:
```cpp
STATIC
EFI_STATUS
EFIAPI
ExtractConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Request,
  OUT       EFI_STRING                      *Progress,
  OUT       EFI_STRING                      *Results
)
{
  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((Request != NULL) && !HiiIsConfigHdrMatch(Request, &StorageGuid, StorageName)) {
    return EFI_NOT_FOUND;
  }

  BOOLEAN AllocatedRequest = FALSE;
  EFI_STRING ConfigRequest = Request;

  if ((Request == NULL) || (StrStr (Request, L"OFFSET") == NULL)) {
    EFI_STRING ConfigRequestHdr = HiiConstructConfigHdr(&StorageGuid, StorageName, mDriverHandle);
    UINTN Size = (StrLen(ConfigRequestHdr) + StrLen(L"&OFFSET=0&WIDTH=") + sizeof(UINTN)*2 + 1) * sizeof(CHAR16);
    ConfigRequest = AllocateZeroPool(Size);
    AllocatedRequest = TRUE;
    UnicodeSPrint(ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, sizeof(VARIABLE_STRUCTURE));
    FreePool(ConfigRequestHdr);
  }

  EFI_STATUS Status = gHiiConfigRouting->BlockToConfig(gHiiConfigRouting,
                                                       ConfigRequest,
                                                       (UINT8*)&FormStorage,
                                                       sizeof(VARIABLE_STRUCTURE),
                                                       Results,
                                                       Progress);

  if (AllocatedRequest) {
    FreePool(ConfigRequest);
    if (Request == NULL) {
      *Progress = NULL;
    } else if (StrStr(Request, L"OFFSET") == NULL) {
      *Progress = Request + StrLen(Request);
    }
  }

  return Status;
}
```

You can rebuild our driver and check that it still works correctly. As I've pointed earlier you wouldn't see any differences in the driver responses.
But if you'll try to debug the driver calls, you'll see that now `ExportConfig()` call from the `HIIConfig.efi dump` command is issued only once with a `NULL` request string. As now our driver handles such request successfully the `ExportConfig()` doesn't have any need to parse IFR and construct a full request.

