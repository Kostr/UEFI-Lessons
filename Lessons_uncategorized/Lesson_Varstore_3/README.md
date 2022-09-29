# `BlockToConfig()`/`ConfigToBlock()`

With the `Buffer Storage` you have a freedom about what you do on store/update operations, but still your form storage is just some data packed in a structure.

So add it as a global variable to our `UefiLessonsPkg/HIIFormDataElementsVarstore/HIIFormDataElementsVarstore.c` file:
```cpp
VARIABLE_STRUCTURE FormStorage;
```

Now let's understand our task in the `Extractonfig()` function: we need to parse incoming request, get some parts of our structure and return them in a configuration response.

In other words we need to correspond `OFFSET=<...>&WIDTH=<...>` combinations from the request to the data in the structure. But you've already know that there can be many such pairs in the request, and parsing all of them manually can be a real burden. I'm not even speaking about creating appropriate response string.

Therefore UEFI specification offers us a helper function to ease our life:
```
EFI_HII_CONFIG_ROUTING_PROTOCOL.BlockToConfig()

Summary:
This helper function is to be called by drivers to map configuration data stored in byte array (“block”) formats such as UEFI Variables into current configuration strings.

Prototype:

typedef
EFI_STATUS
 (EFIAPI * EFI_HII_BLOCK_TO_CONFIG ) (
 IN CONST EFI_HII_CONFIG_ROUTING_PROTOCOL *This,
 IN CONST EFI_STRING ConfigRequest,
 IN CONST UINT8 *Block,
 IN CONST UINTN BlockSize,
 OUT EFI_STRING *Config,
 OUT EFI_STRING *Progress
 );

Parameters:
This		Points to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance
ConfigRequest	A null-terminated string in <ConfigRequest> format
Block		Array of bytes defining the block’s configuration
BlockSize	Length in bytes of Block
Config		Filled-in configuration string. String allocated by the function. Returned only if call is
		successful. The null-terminated string will be in <ConfigResp> format
Progress	A pointer to a string filled in with the offset of the most recent ‘&’ before the first
		failing name / value pair (or the beginning of the string if the failure is in the first
		name / value pair) or the terminating NULL if all was successful

Description:
This function extracts the current configuration from a block of bytes. To do so, it requires that the ConfigRequest string consists of a list of <BlockName> formatted names. It uses the offset in the
name to determine the index into the Block to start the extraction and the width of each name to determine the number of bytes to extract. These are mapped to a string using the equivalent of the C
“%x” format (with optional leading spaces). The call fails if, for any (offset, width) pair in ConfigRequest, offset+value >= BlockSize
```

With it you can finish our `ExtractConfig` function like this:
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
To use `BlockToConfig` we need to get `EFI_HII_CONFIG_ROUTING_PROTOCOL` pointer. You can receive it via the `LocateProtocol` call in the driver's entry point function. After that just save the pointer as a global variable, so you could use it in other places like `ExtractConfig` function. Although here I've decided to take another approach. Here I just use `UefiHiiServicesLib` that locates several HII related protocols for us and provides them in pointers like `gHiiConfigRouting` for our usage. I've mentioned it in the earlier lessons. To use it add `UefiHiiServicesLib` to the `[LibraryClasses]` in the `INF` file and `#include <Library/UefiHiiServicesLib.h>` to the start of the `*.c` file.

In the code above I've marked a place where can be additional custom operations to get you storage data structure. In our simple driver we don't have anything like that, but speaking in terms of our previous examples, this is the place where you would read you PCIe device memory, get state of GPIO LEDs, etc. We can even mimic `efivarstore` behaviour if here we would load our structure from some variable in the global UEFI variable storage.


Now let's get to our `RouteConfig()` function. In this function we need to parse incoming request and modify some parts of our storage structure based on the request. Once again a process of parsing `OFFSET=<...>&WIDTH=<...>&VALUE=<...>` combinations from the configuration request and cooresponding them to the real data is not easy. Luckily UEFI specification offers us a helper function for that case too:
```
EFI_HII_CONFIG_ROUTING_PROTOCOL.ConfigToBlock()

Summary:
This helper function is to be called by drivers to map configuration strings to configurations stored in byte array (“block”) formats such as UEFI Variables.

Prototype:

typedef
EFI_STATUS
 (EFIAPI * EFI_HII_CONFIG_TO_BLOCK ) (
 IN CONST EFI_HII_CONFIG_ROUTING_PROTOCOL *This,
 IN CONST EFI_STRING *ConfigResp,
 IN OUT CONST UINT8 *Block,
 IN OUT UINTN *BlockSize,
 OUT EFI_STRING *Progress
 );

Parameters:
This		Points to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.
ConfigResp	A null-terminated string in <ConfigResp> format.
Block		A possibly null array of bytes representing the current block. Only bytes referenced in the ConfigResp string in the block are modified. If this parameter is null or if the
		*BlockSize parameter is (on input) shorter than required by the Configuration string, only the BlockSize parameter is updated and an appropriate status (see below) is returned.
BlockSize	The length of the Block in units of UINT8. On input, this is the size of the Block. On output, if successful, contains the largest index of the modified byte in the Block, or
		the required buffer size if the Block is not large enough.
Progress	On return, points to an element of the ConfigResp string filled in with the offset of the most recent ‘&’ before the first failing name / value pair (or the beginning of the
		string if the failure is in the first name / value pair) or the terminating NULL if all was successful.

Description:
This function maps a configuration containing a series of <BlockConfig> formatted name value pairs in ConfigResp into a Block so it may be stored in a linear mapped storage such as a UEFI Variable. If
present, the function skips GUID, NAME, and PATH in <ConfigResp>. It stops when it finds a non-<BlockConfig> name / value pair (after skipping the routing header) or when it reaches the end of the string.
```

So let's use it in our `RouteConfig()` function:
```cpp
STATIC
EFI_STATUS
EFIAPI
RouteConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Configuration,
  OUT       EFI_STRING                      *Progress
)
{
  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  UINTN BlockSize = sizeof(VARIABLE_STRUCTURE);
  EFI_STATUS Status = gHiiConfigRouting->ConfigToBlock(gHiiConfigRouting,
                                                       Configuration,
                                                       (UINT8*)&FormStorage,
                                                       &BlockSize,
                                                       Progress);

  // Here can be additional custom operations to update you storage data structure

  return Status;
}
```
Once again I've marked a place where can be additional functionality for your storage. Speaking in terms of our previous examples, this is the place where you would update you PCIe device memory, set state of GPIO LEDs, etc. If you want to mimic `efivarstore` behaviour here you would update the variable in the global UEFI variable storage.

Now we are ready to build and test our driver.

Load it in the UEFI shell:
```
FS0:\> load HIIFormDataElementsVarstore.efi
```

Check the form browser. Now our form is filled with zeros rather than default values. We didn't give any data to our `VARIABLE_STRUCTURE FormStorage` variable, so it is expected output:
![1](1.png?raw=true "1")

And now if you'll try to set the checkbox element and submit a form, you'll see that submit also works fine:
![2](2.png?raw=true "2")

As in our case the `VARIABLE_STRUCTURE FormStorage` is a simple local driver variable the changes are pesistent only in the context of a current boot. If you'll reboot QEMU, you'll start from 0 values in the storage again.

# Setting defaults for the form elements

As now our `ExtractConfig`/`RouteConfig` functions are working we can use `HiiSetToDefaults` helper to set defaults to our form elements non-interactively, like we did it in the `efivarstore` case earlier. For example let's initialize our form from the manufacture defaults:
```cpp
EFI_GUID StorageGuid = STORAGE_GUID;
EFI_STRING StorageName = L"FormData";

<...>

EFI_STATUS
EFIAPI
HIIFormDataElementsVarstoreEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  <...>

  EFI_STRING ConfigStr = HiiConstructConfigHdr(&StorageGuid, StorageName, mDriverHandle);
  UINT16 DefaultId = 1;
  if (!HiiSetToDefaults(ConfigStr, DefaultId)) {
    Print(L"Error! Can't set default configuration #%d\n", DefaultId);
  }

  return EFI_SUCCESS;
}
```

With that our form will start with the manufacture default values:
![5](5.png?raw=true "5")


# Checking the form with `HIIConfig.efi`

As another test let's try to use our `HIIConfig.efi` program to work with our `varstorage` driver.

Output for the request for all of the driver configuration data looks like this:
```
FS0:\> HIIConfig.efi extract 37807592-733A-4F1B-9557-F22AF743E8C2 FormData VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8)

Request: GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400
Response: GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=0000&WIDTH=0001&VALUE=00&OFFSET=0001&WIDTH=0002&VALUE=0008&OFFSET=0003&WIDTH=0014&VALUE=006f0072007000200067006e0069007200740053&OFFSET=0019&WIDTH=0004&VALUE=160507e5&OFFSET=001<...>


GUID=927580373a731b4f9557f22af743e8c2 (37807592-733A-4F1B-9557-F22AF743E8C2)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400 (VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8))
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0008
08 00                                            | ..
OFFSET=0003  WIDTH=0014  VALUE=006f0072007000200067006e0069007200740053
53 00 74 00 72 00 69 00 6E 00 67 00 20 00 70 00  | S.t.r.i.n.g. .p.
72 00 6F 00                                      | r.o.
OFFSET=0019  WIDTH=0004  VALUE=160507e5
E5 07 05 16                                      | ....
OFFSET=001d  WIDTH=0003  VALUE=213717
17 37 21                                         | .7!
OFFSET=0020  WIDTH=0001  VALUE=33
33                                               | 3
OFFSET=0021  WIDTH=0003  VALUE=0a0b0c
0C 0B 0A                                         | ...

GUID=927580373a731b4f9557f22af743e8c2 (37807592-733A-4F1B-9557-F22AF743E8C2)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400 (VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8))
ALTCFG=0000
OFFSET=0000  WIDTH=0001  VALUE=01
01                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0007
07 00                                            | ..
OFFSET=0003  WIDTH=0014  VALUE=00660065006400200067006e0069007200740053
53 00 74 00 72 00 69 00 6E 00 67 00 20 00 64 00  | S.t.r.i.n.g. .d.
65 00 66 00                                      | e.f.
OFFSET=0019  WIDTH=0004  VALUE=160507e5
E5 07 05 16                                      | ....
OFFSET=001d  WIDTH=0003  VALUE=213717
17 37 21                                         | .7!
OFFSET=0020  WIDTH=0001  VALUE=55
55                                               | U
OFFSET=0021  WIDTH=0003  VALUE=0a0b0c
0C 0B 0A                                         | ...

GUID=927580373a731b4f9557f22af743e8c2 (37807592-733A-4F1B-9557-F22AF743E8C2)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400 (VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8))
ALTCFG=0001
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0008
08 00                                            | ..
OFFSET=0003  WIDTH=0014  VALUE=006f0072007000200067006e0069007200740053
53 00 74 00 72 00 69 00 6E 00 67 00 20 00 70 00  | S.t.r.i.n.g. .p.
72 00 6F 00                                      | r.o.
OFFSET=0019  WIDTH=0004  VALUE=160507e5
E5 07 05 16                                      | ....
OFFSET=001d  WIDTH=0003  VALUE=213717
17 37 21                                         | .7!
OFFSET=0020  WIDTH=0001  VALUE=33
33                                               | 3
OFFSET=0021  WIDTH=0003  VALUE=0a0b0c
0C 0B 0A                                         | ...
```

Here is example that accesses individual element (`numeric` in this case):
```
FS0:\> HIIConfig.efi extract 37807592-733A-4F1B-9557-F22AF743E8C2 FormData VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8) 3 14

Request: GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=3&WIDTH=14
Response: GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=3&WIDTH=14&VALUE=006f0072007000200067006e0069007200740053&GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5<...>


GUID=927580373a731b4f9557f22af743e8c2 (37807592-733A-4F1B-9557-F22AF743E8C2)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400 (VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8))
OFFSET=3  WIDTH=14  VALUE=006f0072007000200067006e0069007200740053
53 00 74 00 72 00 69 00 6E 00 67 00 20 00 70 00  | S.t.r.i.n.g. .p.
72 00 6F 00                                      | r.o.

GUID=927580373a731b4f9557f22af743e8c2 (37807592-733A-4F1B-9557-F22AF743E8C2)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400 (VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8))
ALTCFG=0000
OFFSET=0003  WIDTH=0014  VALUE=00660065006400200067006e0069007200740053
53 00 74 00 72 00 69 00 6E 00 67 00 20 00 64 00  | S.t.r.i.n.g. .d.
65 00 66 00                                      | e.f.

GUID=927580373a731b4f9557f22af743e8c2 (37807592-733A-4F1B-9557-F22AF743E8C2)
NAME=0046006f0072006d0044006100740061 (FormData)
NAME=0041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400 (VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8))
ALTCFG=0001
OFFSET=0003  WIDTH=0014  VALUE=006f0072007000200067006e0069007200740053
53 00 74 00 72 00 69 00 6E 00 67 00 20 00 70 00  | S.t.r.i.n.g. .p.
72 00 6F 00                                      | r.o.
```

We can change this element value:
```
FS0:\> HIIConfig.efi route 37807592-733A-4F1B-9557-F22AF743E8C2 FormData VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8) 1 2 1234
Request: GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=1&WIDTH=2&VALUE=1234
```

And verify that it indeed was changed:
```
FS0:\> HIIConfig.efi extract 37807592-733A-4F1B-9557-F22AF743E8C2 FormData VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8)
                                        1 2
Request: GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=1&WIDTH=2
Response: GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=1&WIDTH=2&VALUE=1234&GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&ALTCFG=0000&OFFSET=0001&WI<...>


GUID=927580373a731b4f9557f22af743e8c2 (37807592-733A-4F1B-9557-F22AF743E8C2)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400 (VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8))
OFFSET=1  WIDTH=2  VALUE=1234
34 12                                            | 4.

GUID=927580373a731b4f9557f22af743e8c2 (37807592-733A-4F1B-9557-F22AF743E8C2)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400 (VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8))
ALTCFG=0000
OFFSET=0001  WIDTH=0002  VALUE=0007
07 00                                            | ..

GUID=927580373a731b4f9557f22af743e8c2 (37807592-733A-4F1B-9557-F22AF743E8C2)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400 (VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8))
ALTCFG=0001
OFFSET=0001  WIDTH=0002  VALUE=0008
08 00                                            | ..
```

`ExportConfig` call also works correctly. Yes, the `varstore` configurations is seen in the `ExportConfig` output opposed to the `efivarstore` configurations:
```
FS0:\> HIIConfig.efi dump
Full configuration for the HII Database (Size = 43266):

<...>

GUID=927580373a731b4f9557f22af743e8c2 (37807592-733A-4F1B-9557-F22AF743E8C2)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400 (VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8))
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0008
34 12                                            | ..
OFFSET=0003  WIDTH=0014  VALUE=006f0072007000200067006e0069007200740053
53 00 74 00 72 00 69 00 6E 00 67 00 20 00 70 00  | S.t.r.i.n.g. .p.
72 00 6F 00                                      | r.o.
OFFSET=0019  WIDTH=0004  VALUE=160507e5
E5 07 05 16                                      | ....
OFFSET=001d  WIDTH=0003  VALUE=213717
17 37 21                                         | .7!
OFFSET=0020  WIDTH=0001  VALUE=33
33                                               | 3
OFFSET=0021  WIDTH=0003  VALUE=0a0b0c
0C 0B 0A                                         | ...

GUID=927580373a731b4f9557f22af743e8c2 (37807592-733A-4F1B-9557-F22AF743E8C2)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400 (VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8))
ALTCFG=0000
OFFSET=0000  WIDTH=0001  VALUE=01
01                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0007
07 00                                            | ..
OFFSET=0003  WIDTH=0014  VALUE=00660065006400200067006e0069007200740053
53 00 74 00 72 00 69 00 6E 00 67 00 20 00 64 00  | S.t.r.i.n.g. .d.
65 00 66 00                                      | e.f.
OFFSET=0019  WIDTH=0004  VALUE=160507e5
E5 07 05 16                                      | ....
OFFSET=001d  WIDTH=0003  VALUE=213717
17 37 21                                         | .7!
OFFSET=0020  WIDTH=0001  VALUE=55
55                                               | U
OFFSET=0021  WIDTH=0003  VALUE=0a0b0c
0C 0B 0A                                         | ...

GUID=927580373a731b4f9557f22af743e8c2 (37807592-733A-4F1B-9557-F22AF743E8C2)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400 (VenHw(FB821964-ACB4-437B-9FE6-66AA7AD7C5D8))
ALTCFG=0001
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0008
08 00                                            | ..
OFFSET=0003  WIDTH=0014  VALUE=006f0072007000200067006e0069007200740053
53 00 74 00 72 00 69 00 6E 00 67 00 20 00 70 00  | S.t.r.i.n.g. .p.
72 00 6F 00                                      | r.o.
OFFSET=0019  WIDTH=0004  VALUE=160507e5
E5 07 05 16                                      | ....
OFFSET=001d  WIDTH=0003  VALUE=213717
17 37 21                                         | .7!
OFFSET=0020  WIDTH=0001  VALUE=33
33                                               | 3
OFFSET=0021  WIDTH=0003  VALUE=0a0b0c
0C 0B 0A                                         | ...
```

So as you can see everything works correctly. But there is still some room to improve.

