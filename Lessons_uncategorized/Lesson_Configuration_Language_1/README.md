The communication between the `FormBrowser` application and the HII subsystem is based on a special interface language defined in the UEFI specification: `UEFI Configuration Language`.

In a way it is similar to how a real internet browser uses HTTP GET/POST requests "language" to accesses a web server.

And as you might know in case of the HTTP you could just use something like `curl` to issue requests directly from CLI instead of using a window browser application.
There is no predefined command in the UEFI shell that would allow you to issue `UEFI Configuration Language` requests directly without launching a form browser, so let's create such application.

In other words our application would use this `UEFI Configuration Language` to talk with the HII subsystem directly. Our goal is to read and write HII form elements directly from the UEFI Shell CLI.

The protocol that is used for communication with the HII subsystem is `EFI_HII_CONFIG_ROUTING_PROTOCOL`:
```
EFI_HII_CONFIG_ROUTING_PROTOCOL

Summary:
The EFI HII Configuration Routing Protocol manages the movement of configuration data from drivers to configuration applications. It then serves as the single point to receive configuration information from configuration applications, routing the results to the appropriate drivers.


GUID:

#define EFI_HII_CONFIG_ROUTING_PROTOCOL_GUID \
 { 0x587e72d7, 0xcc50, 0x4f79,\
 { 0x82, 0x09, 0xca, 0x29, 0x1f, 0xc1, 0xa1, 0x0f }}


Protocol Interface Structure:

typedef struct {
 EFI_HII_EXTRACT_CONFIG ExtractConfig;
 EFI_HII_EXPORT_CONFIG ExportConfig
 EFI_HII_ROUTE_CONFIG RouteConfig;
 EFI_HII_BLOCK_TO_CONFIG BlockToConfig;
 EFI_HII_CONFIG_TO_BLOCK ConfigToBlock;
 EFI_HII_GET_ALT_CFG GetAltConfig;
} EFI_HII_CONFIG_ROUTING_PROTOCOL;

Description:
This protocol defines the configuration routing interfaces between external applications and the HII. There may only be one instance of this protocol in the system.
```

# `HIIConfig.efi dump`

First let's try to use the protocol `ExportConfig` function to dump the current HII configuration in the `UEFI Configuration Language` format:
```
EFI_HII_CONFIG_ROUTING_PROTOCOL.ExportConfig()

Summary:
This function allows the caller to request the current configuration for the entirety of the current HII database and returns the data in a null-terminated string.

Prototype:

typedef
EFI_STATUS
 (EFIAPI * EFI_HII_EXPORT_CONFIG ) (
 IN CONST EFI_HII_CONFIG_ROUTING_PROTOCOL *This,
 OUT EFI_STRING *Results
 );

Parameters:
This		Points to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance
Results		A null-terminated string in <MultiConfigAltResp> format which has all values filled in for the entirety of the current HII database

Description:
This function allows the caller to request the current configuration for all of the current HII database. The results include both the current and alternate configurations
```

Create a Shell application `HIIConfig`. In the INF file add the `gEfiHiiConfigRoutingProtocolGuid` to the application protocols `UefiLessonsPkg/HIIConfig/HIIConfig.inf`:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = HIIConfig
  FILE_GUID                      = 417bf8bf-05b6-4cb5-b6e1-ed41c1fc971d
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = ShellCEntryLib

[Sources]
  HIIConfig.c

[Packages]
  MdePkg/MdePkg.dec
  MdeModulePkg/MdeModulePkg.dec

[LibraryClasses]
  ShellCEntryLib
  UefiLib

[Protocols]
  gEfiHiiConfigRoutingProtocolGuid
```

In the `UefiLessonsPkg/HIIConfig/HIIConfig.c` locate this protocol and use its `ExportConfig` function if the application is called with a `dump` argument:
```cpp
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/HiiConfigRouting.h>


VOID Usage()
{
  Print(L"Usage:\n");
  Print(L"HIIConfig.efi dump\n");
}

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *gHiiConfigRouting = NULL;
  EFI_STATUS Status = gBS->LocateProtocol(&gEfiHiiConfigRoutingProtocolGuid,
                                          NULL,
                                          (VOID **)&gHiiConfigRouting);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't locate gEfiHiiConfigRoutingProtocolGuid: %r", Status);
    return Status;
  }

  if (Argc == 1) {
    Usage();
    return EFI_SUCCESS;
  }

  EFI_STRING Result;
  if (!StrCmp(Argv[1], L"dump")) {
    Status = gHiiConfigRouting->ExportConfig(gHiiConfigRouting, &Result);
    Print(L"Full configuration for the HII Database (Size = %d):\n", StrLen(Result));
    Print(L"%s\n", Result);
    FreePool(Result);
  }
  return EFI_SUCCESS;
}
```
As the `Result` string data is allocated on heap, we need to use `FreePool` to free this memory afterwards.

Let's build and test this application:
```
FS0:\> HIIConfig.efi dump
Full configuration for the HII Database (Size = 42018):
GUID=1cc53572800cab4c87ac3b084a6304b1&NAME=004d00610069006e0046006f0072006d00530074006100740065&PATH=01041400dfc5dcd907405e4390988970935504b27fff0400&OFFSET=0000&WIDTH=0020&VALUE=00000000000000000000000000000000000000000000007400650073006e0055&OFFSET=0020&WIDTH=0004&VALUE=00000000&GUID=1cc53572800cab4c87ac3b084a6304b1&
```
Here is your first peak into the `UEFI Configuration language`.

But something is wrong, can you see what?

The string length of the response above is 42018, but the actual printed string is a way shorter than that. If you'll try to measure it, you'll see that it has exactly 320 symbols. Why is that?

This is happening because the maximum printable number of characters in the `Print()` statement is limited by the `gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize` PCD. Check its definition in the [https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec](https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec):
```
  ## The maximum printable number of characters. UefLib functions: AsciiPrint(), AsciiErrorPrint(),
  #  PrintXY(), AsciiPrintXY(), Print(), ErrorPrint() base on this PCD value to print characters.
  # @Prompt Maximum Printable Number of Characters.
  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|320|UINT32|0x101
```
If you want to see how it is used, check the `InternalPrint` function implementation in the [https://github.com/tianocore/edk2/blob/master/MdePkg/Library/UefiLib/UefiLibPrint.c](https://github.com/tianocore/edk2/blob/master/MdePkg/Library/UefiLib/UefiLibPrint.c).

Anyway, to bypass this limit we can use the `gST->ConOut->OutputString` function directly like this:
```
  Print(L"Full configuration for the HII Database (Size = %d):\n", StrLen(Result));
- Print(L"%s\n", Result);
+ gST->ConOut->OutputString(gST->ConOut, Results);
```

Now you can see the full output. To make it more readable here I've truncated a bunch of zeros in the middle of the output. But even with this the output is very long:
```
Full configuration for the HII Database (Size = 42018):
GUID=1cc53572800cab4c87ac3b084a6304b1&NAME=004d00610069006e0046006f0072006d00530074006100740065&PATH=01041400dfc5dcd907405e4390988970935504b27fff0400&OFFSET=0
000&WIDTH=0020&VALUE=00000000000000000000000000000000000000000000007400650073006e0055&OFFSET=0020&WIDTH=0004&VALUE=00000000&GUID=1cc53572800cab4c87ac3b084a630
4b1&NAME=004d00610069006e0046006f0072006d00530074006100740065&PATH=01041400dfc5dcd907405e4390988970935504b27fff0400&ALTCFG=0000&OFFSET=0020&WIDTH=0004&VALUE=0
0000000&GUID=1cc53572800cab4c87ac3b084a6304b1&NAME=004d00610069006e0046006f0072006d00530074006100740065&PATH=01041400dfc5dcd907405e4390988970935504b27fff0400&
ALTCFG=0001&OFFSET=0020&WIDTH=0004&VALUE=00000000&GUID=16d6474bd6a852459d44ccad2e0f4cf9&NAME=00490053004300530049005f0043004f004e004600490047005f0049004600520
05f004e00560044004100540041&PATH=0104140016d6474bd6a852459d44ccad2e0f4cf97fff0400&OFFSET=0&WIDTH=000000000000453c&VALUE=00000000000000000000000000000000000000
...
... ton of zeros
...
000000000000000000&GUID=16d6474bd6a852459d44ccad2e0f4cf9&NAME=00490053004300530049005f0043004f004e004600490047005f004900460052005f004e00560044004100540041&PAT
H=0104140016d6474bd6a852459d44ccad2e0f4cf97fff0400&ALTCFG=0000&OFFSET=01d8&WIDTH=0001&VALUE=00&OFFSET=01d9&WIDTH=0001&VALUE=00&OFFSET=01da&WIDTH=0001&VALUE=00
&OFFSET=01dc&WIDTH=0002&VALUE=03e8&OFFSET=01de&WIDTH=0001&VALUE=00&OFFSET=01df&WIDTH=0001&VALUE=00&OFFSET=05fe&WIDTH=0002&VALUE=0000&OFFSET=062a&WIDTH=0001&VA
LUE=00&OFFSET=062b&WIDTH=0001&VALUE=01&OFFSET=0fd4&WIDTH=0001&VALUE=00&OFFSET=0fd5&WIDTH=0001&VALUE=00&OFFSET=0fd6&WIDTH=0001&VALUE=00&OFFSET=0fd7&WIDTH=0001&
VALUE=00&OFFSET=0fd8&WIDTH=0001&VALUE=00&OFFSET=0fd9&WIDTH=0001&VALUE=00&OFFSET=0fda&WIDTH=0001&VALUE=00&OFFSET=0fdb&WIDTH=0001&VALUE=00&OFFSET=0fdc&WIDTH=000
1&VALUE=00&OFFSET=0fdd&WIDTH=0001&VALUE=00&OFFSET=0fde&WIDTH=0001&VALUE=00&OFFSET=0fdf&WIDTH=0001&VALUE=00&OFFSET=0fe0&WIDTH=0001&VALUE=00&OFFSET=0fe1&WIDTH=0
001&VALUE=00&OFFSET=0fe2&WIDTH=0001&VALUE=00&OFFSET=0fe3&WIDTH=0001&VALUE=00&OFFSET=0fe4&WIDTH=0001&VALUE=00&OFFSET=0fe5&WIDTH=0001&VALUE=00&OFFSET=0fe6&WIDTH
=0001&VALUE=00&OFFSET=0fe7&WIDTH=0001&VALUE=00&OFFSET=0fe8&WIDTH=0001&VALUE=00&OFFSET=0fe9&WIDTH=0001&VALUE=00&OFFSET=0fea&WIDTH=0001&VALUE=00&OFFSET=0feb&WID
TH=0001&VALUE=00&OFFSET=0fec&WIDTH=0002&VALUE=0064&OFFSET=0fee&WIDTH=0002&VALUE=0064&OFFSET=0ff0&WIDTH=0002&VALUE=0064&OFFSET=0ff2&WIDTH=0002&VALUE=0064&OFFSE
T=0ff4&WIDTH=0002&VALUE=0064&OFFSET=0ff6&WIDTH=0002&VALUE=0064&OFFSET=0ff8&WIDTH=0002&VALUE=0064&OFFSET=0ffa&WIDTH=0002&VALUE=0064&OFFSET=0ffc&WIDTH=0001&VALU
E=00&OFFSET=0ffd&WIDTH=0001&VALUE=00&OFFSET=0ffe&WIDTH=0001&VALUE=00&OFFSET=0fff&WIDTH=0001&VALUE=00&OFFSET=1000&WIDTH=0001&VALUE=00&OFFSET=1001&WIDTH=0001&VA
LUE=00&OFFSET=1002&WIDTH=0001&VALUE=00&OFFSET=1003&WIDTH=0001&VALUE=00&OFFSET=1004&WIDTH=0001&VALUE=00&OFFSET=1005&WIDTH=0001&VALUE=00&OFFSET=1006&WIDTH=0001&
VALUE=00&OFFSET=1007&WIDTH=0001&VALUE=00&OFFSET=1008&WIDTH=0001&VALUE=00&OFFSET=1009&WIDTH=0001&VALUE=00&OFFSET=100a&WIDTH=0001&VALUE=00&OFFSET=100b&WIDTH=000
1&VALUE=00&OFFSET=100c&WIDTH=0002&VALUE=0000&OFFSET=100e&WIDTH=0002&VALUE=0000&OFFSET=1010&WIDTH=0002&VALUE=0000&OFFSET=1012&WIDTH=0002&VALUE=0000&OFFSET=1014
&WIDTH=0002&VALUE=0000&OFFSET=1016&WIDTH=0002&VALUE=0000&OFFSET=1018&WIDTH=0002&VALUE=0000&OFFSET=101a&WIDTH=0002&VALUE=0000&OFFSET=101c&WIDTH=0001&VALUE=00&O
FFSET=101d&WIDTH=0001&VALUE=00&OFFSET=101e&WIDTH=0001&VALUE=00&OFFSET=101f&WIDTH=0001&VALUE=00&OFFSET=1020&WIDTH=0001&VALUE=00&OFFSET=1021&WIDTH=0001&VALUE=00
&OFFSET=1022&WIDTH=0001&VALUE=00&OFFSET=1023&WIDTH=0001&VALUE=00&OFFSET=1024&WIDTH=0001&VALUE=00&OFFSET=1025&WIDTH=0001&VALUE=00&OFFSET=1026&WIDTH=0001&VALUE=
00&OFFSET=1027&WIDTH=0001&VALUE=00&OFFSET=1028&WIDTH=0001&VALUE=00&OFFSET=1029&WIDTH=0001&VALUE=00&OFFSET=102a&WIDTH=0001&VALUE=00&OFFSET=102b&WIDTH=0001&VALU
E=00&GUID=16d6474bd6a852459d44ccad2e0f4cf9&NAME=00490053004300530049005f0043004f004e004600490047005f004900460052005f004e00560044004100540041&PATH=0104140016d6
474bd6a852459d44ccad2e0f4cf97fff0400&ALTCFG=0001&OFFSET=01d8&WIDTH=0001&VALUE=00&OFFSET=01d9&WIDTH=0001&VALUE=00&OFFSET=01da&WIDTH=0001&VALUE=00&OFFSET=01dc&W
IDTH=0002&VALUE=03e8&OFFSET=01de&WIDTH=0001&VALUE=00&OFFSET=01df&WIDTH=0001&VALUE=00&OFFSET=05fe&WIDTH=0002&VALUE=0000&OFFSET=062a&WIDTH=0001&VALUE=00&OFFSET=
062b&WIDTH=0001&VALUE=01&OFFSET=0fd4&WIDTH=0001&VALUE=00&OFFSET=0fd5&WIDTH=0001&VALUE=00&OFFSET=0fd6&WIDTH=0001&VALUE=00&OFFSET=0fd7&WIDTH=0001&VALUE=00&OFFSE
T=0fd8&WIDTH=0001&VALUE=00&OFFSET=0fd9&WIDTH=0001&VALUE=00&OFFSET=0fda&WIDTH=0001&VALUE=00&OFFSET=0fdb&WIDTH=0001&VALUE=00&OFFSET=0fdc&WIDTH=0001&VALUE=00&OFF
SET=0fdd&WIDTH=0001&VALUE=00&OFFSET=0fde&WIDTH=0001&VALUE=00&OFFSET=0fdf&WIDTH=0001&VALUE=00&OFFSET=0fe0&WIDTH=0001&VALUE=00&OFFSET=0fe1&WIDTH=0001&VALUE=00&O
FFSET=0fe2&WIDTH=0001&VALUE=00&OFFSET=0fe3&WIDTH=0001&VALUE=00&OFFSET=0fe4&WIDTH=0001&VALUE=00&OFFSET=0fe5&WIDTH=0001&VALUE=00&OFFSET=0fe6&WIDTH=0001&VALUE=00
&OFFSET=0fe7&WIDTH=0001&VALUE=00&OFFSET=0fe8&WIDTH=0001&VALUE=00&OFFSET=0fe9&WIDTH=0001&VALUE=00&OFFSET=0fea&WIDTH=0001&VALUE=00&OFFSET=0feb&WIDTH=0001&VALUE=
00&OFFSET=0fec&WIDTH=0002&VALUE=0064&OFFSET=0fee&WIDTH=0002&VALUE=0064&OFFSET=0ff0&WIDTH=0002&VALUE=0064&OFFSET=0ff2&WIDTH=0002&VALUE=0064&OFFSET=0ff4&WIDTH=0
002&VALUE=0064&OFFSET=0ff6&WIDTH=0002&VALUE=0064&OFFSET=0ff8&WIDTH=0002&VALUE=0064&OFFSET=0ffa&WIDTH=0002&VALUE=0064&OFFSET=0ffc&WIDTH=0001&VALUE=00&OFFSET=0f
fd&WIDTH=0001&VALUE=00&OFFSET=0ffe&WIDTH=0001&VALUE=00&OFFSET=0fff&WIDTH=0001&VALUE=00&OFFSET=1000&WIDTH=0001&VALUE=00&OFFSET=1001&WIDTH=0001&VALUE=00&OFFSET=
1002&WIDTH=0001&VALUE=00&OFFSET=1003&WIDTH=0001&VALUE=00&OFFSET=1004&WIDTH=0001&VALUE=00&OFFSET=1005&WIDTH=0001&VALUE=00&OFFSET=1006&WIDTH=0001&VALUE=00&OFFSE
T=1007&WIDTH=0001&VALUE=00&OFFSET=1008&WIDTH=0001&VALUE=00&OFFSET=1009&WIDTH=0001&VALUE=00&OFFSET=100a&WIDTH=0001&VALUE=00&OFFSET=100b&WIDTH=0001&VALUE=00&OFF
SET=100c&WIDTH=0002&VALUE=0000&OFFSET=100e&WIDTH=0002&VALUE=0000&OFFSET=1010&WIDTH=0002&VALUE=0000&OFFSET=1012&WIDTH=0002&VALUE=0000&OFFSET=1014&WIDTH=0002&VA
LUE=0000&OFFSET=1016&WIDTH=0002&VALUE=0000&OFFSET=1018&WIDTH=0002&VALUE=0000&OFFSET=101a&WIDTH=0002&VALUE=0000&OFFSET=101c&WIDTH=0001&VALUE=00&OFFSET=101d&WID
TH=0001&VALUE=00&OFFSET=101e&WIDTH=0001&VALUE=00&OFFSET=101f&WIDTH=0001&VALUE=00&OFFSET=1020&WIDTH=0001&VALUE=00&OFFSET=1021&WIDTH=0001&VALUE=00&OFFSET=1022&W
IDTH=0001&VALUE=00&OFFSET=1023&WIDTH=0001&VALUE=00&OFFSET=1024&WIDTH=0001&VALUE=00&OFFSET=1025&WIDTH=0001&VALUE=00&OFFSET=1026&WIDTH=0001&VALUE=00&OFFSET=1027
&WIDTH=0001&VALUE=00&OFFSET=1028&WIDTH=0001&VALUE=00&OFFSET=1029&WIDTH=0001&VALUE=00&OFFSET=102a&WIDTH=0001&VALUE=00&OFFSET=102b&WIDTH=0001&VALUE=00
```

Let's investigate the output above. As you can see the resulting string consists of pairs `<key>=<data>` delimited by the `&` character. And there only 7 keys in the whole output. Here are their meaning:
```
GUID   - guid of the variable storage
NAME   - name of the variable storage
PATH   - device path
ALTCFG - alternative configurations
OFFSET - offset of the variable inside the storage
WIDTH  - size of the variable inside the storage
VALUE  - value of the variable inside the storage
```

They are used like this:
- The combination `GUID=<...>&NAME=<...>&PATH=<...>` is like a header for some storage configuration,
- Sometimes the header above can include `ALTCFG` at the end: `GUID=<...>&NAME=<...>&PATH=<...>&ALTCFG=<...>`. If it is included it means that the data after the header corresponds to some `default` configuration. There are 2 predefined defaults: Standard default (=0000) and Manufacture Default (=0001). The output for these defaults is always present, even if they are not declared explicitly in the VFR code. But there can be more ALTCFG's in the ouput if you'll declare alternative defaultstores in VFR,
- After the storage configuration header there are configuration elements. Every element is encoded via the `OFFSET=<...>&WIDTH=<...>&VALUE=<...>` combination, which means that the data in the storage at OFFSET with the byte length WIDTH is equal to VALUE.

