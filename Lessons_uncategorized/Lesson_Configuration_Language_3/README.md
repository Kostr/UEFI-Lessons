The `ExportConfig` function dumps configuration for the entirety of the current HII database. We've prettified that output, but it is still cumbersome to use it if we just want to check a single configuration value.

For this task we can utilize the `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExtractConfig()` function:
```
EFI_HII_CONFIG_ROUTING_PROTOCOL.ExtractConfig()

Summary:
This function allows a caller to extract the current configuration for one or more named elements from one or more drivers.

Prototype:

typedef
EFI_STATUS
 (EFIAPI * EFI_HII_EXTRACT_CONFIG ) (
 IN CONST EFI_HII_CONFIG_ROUTING_PROTOCOL *This,
 IN CONST EFI_STRING Request,
 OUT EFI_STRING *Progress,
 OUT EFI_STRING *Results
 );

Parameters:
This		Points to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance
Request		A null-terminated string in <MultiConfigRequest> format
Progress	On return, points to a character in the Request string. Points to the string’s null terminator if request was successful
		Points to the most recent ‘&’ before the first failing name / value pair (or the beginning of the string if the failure is in the first name / value pair)
		if the request was not successful
Results		A null-terminated string in <MultiConfigAltResp> format which has all values filled in for the names in the Request string

Description:
This function allows the caller to request the current configuration for one or more named elements from one or more drivers. The resulting string is in the standard HII configuration string format.
```

Our goal in this lesson is to add 3 methods to get a configuration setting:
- by providing a full config request string
- by providing config header values (GUID/NAME/PATH) in a readable format to get all settings for the corresponding storage
- by providing config header values (GUID/NAME/PATH) in a readable format and OFFSET and WIDTH data to get some individual setting from the corresponding storage

So our help message in this lesson would be updated to this:
```cpp
VOID Usage()
{
  Print(L"Usage:\n");
  Print(L"HIIConfig.efi dump\n");
  Print(L"HIIConfig.efi extract <ConfigStr>\n");
  Print(L"HIIConfig.efi extract <Guid> <Name> <Path>\n");
  Print(L"HIIConfig.efi extract <Guid> <Name> <Path> <Offset> <Width>\n");
}
```

# `HIIConfig.efi extract <ConfigStr>`

Let's stars with the `HIIConfig.efi extract <ConfigStr>` handler. It can be implemented like this:
```cpp
EFI_STRING Request;
EFI_STRING Progress;
EFI_STRING Result;
if (!StrCmp(Argv[1], L"dump")) {
  <...>
} else if (!StrCmp(Argv[1], L"extract")) {
  if (Argc == 3) {                                // <--- HIIConfig.efi extract <ConfigStr>
    Request = Argv[2];
  } else {
    Print(L"Error! Wrong arguments\n");
    Usage();
    return EFI_INVALID_PARAMETER;
  }
  Status = gHiiConfigRouting->ExtractConfig(gHiiConfigRouting,
                                            Request,
                                            &Progress,
                                            &Result);

  if (StrCmp(Progress, L'\0')) {
    Print(L"Part of string was unparsed %s\n", Progress);
  }
  if (EFI_ERROR(Status)) {
    Print(L"Error! ExtractConfig returned %r\n", Status);
    return Status;
  }
  Print(L"Response: ");
  PrintLongString(Result);
  Print(L"\n\n");
  PrintConfigString(Result);
  FreePool(Result);
}
```

The `ExtractConfig` function parses incoming request string sequentially, that is why there is a real possibility, that only the part of the string would be parsed. This is why we need to check the `Progress` data after the call. If some part of string wouldn't be parsed, this part would be returned in the `Progress`.

The result is encoded in the configuration string, so we first print it as a long string via our `PrintLongString` function and then parse the output via our `PrintConfigString` function.

In the end we need to free `Result` string as it was allocated on heap by the `ExtractConfig` call. We don't need to free the `Progress` string as it is just a pointer to the data in the `Result` string.

For the test let's constuct a requst string from the data in the output of the `dump` command. If you remember at the beginning it displays `MainFormState` configuration:
```
FS0:\> HIIConfig.efi dump
Full configuration for the HII Database (Size = 42018):

GUID=1cc53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
OFFSET=0000  WIDTH=0020  VALUE=00000000000000000000000000000000000000000000007400650073006e0055
55 00 6E 00 73 00 65 00 74 00 00 00 00 00 00 00  | U.n.s.e.t.......
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
OFFSET=0020  WIDTH=0004  VALUE=00000000
00 00 00 00                                      | ....

GUID=1cc53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
ALTCFG=0000
OFFSET=0020  WIDTH=0004  VALUE=00000000
00 00 00 00                                      | ....

GUID=1cc53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
ALTCFG=0001
OFFSET=0020  WIDTH=0004  VALUE=00000000
00 00 00 00                                      | ....

<...>
```

So let's use the data from the output to construct our request string:
```
FS0:\> HIIConfig.efi extract GUID=1cc53572800cab4c87ac3b084a6304b1&NAME=004d00610069006e0046006f0072006d00530074006100740065&PATH=01041400dfc5dcd907405e4390988970935504b27fff0400

Response: GUID=1cc53572800cab4c87ac3b084a6304b1&NAME=004d00610069006e0046006f0072006d00530074006100740065&PATH=01041400dfc5dcd907405e4390988970935504b27fff0400&OFFSET=0000&WIDTH=0020&VALUE=00000000000000000000000000000000000000000000007400650073006e0055&OFFSET=0020&WIDTH=0004&VALUE=00000000&GUID=1cc53572800cab4c87ac3b084a6<...>


GUID=1cc53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
OFFSET=0000  WIDTH=0020  VALUE=00000000000000000000000000000000000000000000007400650073006e0055
55 00 6E 00 73 00 65 00 74 00 00 00 00 00 00 00  | U.n.s.e.t.......
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
OFFSET=0020  WIDTH=0004  VALUE=00000000
00 00 00 00                                      | ....

GUID=1cc53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
ALTCFG=0000
OFFSET=0020  WIDTH=0004  VALUE=00000000
00 00 00 00                                      | ....

FS0:\> c53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
ALTCFG=0001
OFFSET=0020  WIDTH=0004  VALUE=00000000
00 00 00 00                                      | ....
```

Here you can see that the `MainFormState` output is the same that was in the case of a `dump` command, but now the output string contains ONLY the `MainFormState` output.

Before diving into the various configuration requests let's add more methods to call our program that would simplify our life.

# `HIIConfig.efi extract <Guid> <Name> <Path>`

Usually you know `Guid`/`Name`/`DevicePath` values in their standard format, and it is not easy to convert them to their respective configuration string format. So let's write a code that would do it programmatically.

This would allow us to call our program with something like this:
```
FS0:\> HIIConfig.efi extract 7235C51C-0C80-4CAB-87AC-3B084A6304B1 MainFormState VenHw(D9DCC5DF-4007-435E-9098-8970935504B2)
```

Let's update our application to support such call. Here I've marked places that were updated:
```cpp
EFI_STRING Request;
EFI_STRING Progress;
EFI_STRING Result;
if (!StrCmp(Argv[1], L"dump")) {
  <...>
} else if (!StrCmp(Argv[1], L"extract")) {
  if (Argc == 3) {
    Request = Argv[2];
  } else if (Argc == 5) {                                           // <--- HIIConfig.efi extract <Guid> <Name> <Path>
    Status = CreateCfgHeader(Argv[2], Argv[3], Argv[4], &Request);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    Print(L"Request: %s\n", Request);
  } else {
    Print(L"Error! Wrong arguments\n");
    Usage();
    return EFI_INVALID_PARAMETER;
  }
  Status = gHiiConfigRouting->ExtractConfig(gHiiConfigRouting,
                                            Request,
                                            &Progress,
                                            &Result);

  if (StrCmp(Progress, L'\0')) {
    Print(L"Part of string was unparsed %s\n", Progress);
  }

  if (Argc == 5)                                                    // <--- if Request string was allocated on heap we need to free it
    FreePool(Request);

  if (EFI_ERROR(Status)) {
    Print(L"Error! ExtractConfig returned %r\n", Status);
    return Status;
  }
  Print(L"Response: ");
  PrintLongString(Result);
  Print(L"\n\n");
  PrintConfigString(Result);
  FreePool(Result);
}
```

Here we use the custom `CreateCfgHeader` function to create `GUID=<Guid>&NAME=<Name>&PATH=<DevicePath>` request. To do it our function would utilize `HiiConstructConfigHdr` function from the `HiiLib` [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiHiiLib/HiiLib.c](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiHiiLib/HiiLib.c):
```cpp
/**
  Allocates and returns a Null-terminated Unicode <ConfigHdr> string using routing
  information that includes a GUID, an optional Unicode string name, and a device
  path.  The string returned is allocated with AllocatePool().  The caller is
  responsible for freeing the allocated string with FreePool().
  The format of a <ConfigHdr> is as follows:
    GUID=<HexCh>32&NAME=<Char>NameLength&PATH=<HexChar>DevicePathSize<Null>
  @param[in]  Guid          Pointer to an EFI_GUID that is the routing information
                            GUID.  Each of the 16 bytes in Guid is converted to
                            a 2 Unicode character hexadecimal string.  This is
                            an optional parameter that may be NULL.
  @param[in]  Name          Pointer to a Null-terminated Unicode string that is
                            the routing information NAME.  This is an optional
                            parameter that may be NULL.  Each 16-bit Unicode
                            character in Name is converted to a 4 character Unicode
                            hexadecimal string.
  @param[in]  DriverHandle  The driver handle which supports a Device Path Protocol
                            that is the routing information PATH.  Each byte of
                            the Device Path associated with DriverHandle is converted
                            to a 2 Unicode character hexadecimal string.
  @retval NULL   DriverHandle does not support the Device Path Protocol.
  @retval Other  A pointer to the Null-terminate Unicode <ConfigHdr> string
**/
EFI_STRING
EFIAPI
HiiConstructConfigHdr (
  IN CONST EFI_GUID  *Guid   OPTIONAL,
  IN CONST CHAR16    *Name   OPTIONAL,
  IN EFI_HANDLE      DriverHandle
  )
```

As we would use HII library, don't forget to add `HiiLib` to the `[LibraryClasses]` in the `UefiLessonsPkg/HIIConfig/HIIConfig.inf` file and add the `#include <Library/HiiLib.h>` statement to the `UefiLessonsPkg/HIIConfig/HIIConfig.c` file.

To call the `HiiConstructConfigHdr` function we need to have 3 parameters: GUID, Name and a DriverHandle. Here is how we get them:
- to get `Guid` we can utilize `StrToGuid` converter function,
- we can get `Name` as-is from the `Argv`,
- to get `DriverHandle` we construct `EFI_DEVICE_PATH_PROTOCOL` structure from the argumat via the `ConvertTextToDevicePath` function and than use this device path in the `LocateDevicePath` boot service:
```
EFI_BOOT_SERVICES.LocateDevicePath()

Summary:
Locates the handle to a device on the device path that supports the specified protocol.

Prototype:

typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_DEVICE_PATH) (
 IN EFI_GUID *Protocol,
 IN OUT EFI_DEVICE_PATH_PROTOCOL **DevicePath,
 OUT EFI_HANDLE *Device
 );

Parameters:
Protocol 	The protocol to search for
DevicePath 	On input, a pointer to a pointer to the device path. On output, the device path pointer is modified to point to the remaining part of the
		device path—that is, when the function finds the closest handle, it splits the device path into two parts, stripping off the front part, and
		returning the remaining portion
Device 		A pointer to the returned device handle

Description:
The LocateDevicePath() function locates all devices on DevicePath that support Protocol and returns the handle to the device that is closest to DevicePath. DevicePath is advanced over the device path nodes that were matched.
```

The complete code for the `CreateCfgHeader` looks like this:
```cpp
EFI_STATUS CreateCfgHeader(EFI_STRING GuidStr, EFI_STRING NameStr, EFI_STRING DevicePathStr, EFI_STRING* Request)
{
  EFI_STATUS Status;
  EFI_GUID FormsetGuid;
  Status = StrToGuid(GuidStr, &FormsetGuid);
  if (Status != RETURN_SUCCESS) {
    Print(L"Error! Can't convert <FormsetGuid> argument to GUID\n");
    return EFI_INVALID_PARAMETER;
  }

  EFI_HANDLE DriverHandle = NULL;
  EFI_DEVICE_PATH_PROTOCOL* DevicePath = ConvertTextToDevicePath(DevicePathStr);

  Status = gBS->LocateDevicePath(&gEfiDevicePathProtocolGuid,
                                 &DevicePath,
                                 &DriverHandle
                                );
  FreePool(DevicePath);
  if (EFI_ERROR(Status) || (DriverHandle == NULL)) {
    Print(L"Error! Can't get DriverHandle\n");
    return EFI_INVALID_PARAMETER;
  }

  *Request = HiiConstructConfigHdr(&FormsetGuid, NameStr, DriverHandle);
  return EFI_SUCCESS;
}
```

Build and test the new command:
```
FS0:\> HIIConfig.efi extract 7235C51C-0C80-4CAB-87AC-3B084A6304B1 MainFormState VenHw(D9DCC5DF-4007-435E-9098-8970935504B2)

Request: GUID=1cc53572800cab4c87ac3b084a6304b1&NAME=004d00610069006e0046006f0072006d00530074006100740065&PATH=01041400dfc5dcd907405e4390988970935504b27fff0400
Response: GUID=1cc53572800cab4c87ac3b084a6304b1&NAME=004d00610069006e0046006f0072006d00530074006100740065&PATH=01041400dfc5dcd907405e4390988970935504b27fff0400&OFFSET=0000&WIDTH=0020&VALUE=00000000000000000000000000000000000000000000007400650073006e0055&OFFSET=0020&WIDTH=0004&VALUE=00000000&GUID=1cc53572800cab4c87ac3b084a6<...>


GUID=1cc53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
OFFSET=0000  WIDTH=0020  VALUE=00000000000000000000000000000000000000000000007400650073006e0055
55 00 6E 00 73 00 65 00 74 00 00 00 00 00 00 00  | U.n.s.e.t.......
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
OFFSET=0020  WIDTH=0004  VALUE=00000000
00 00 00 00                                      | ....

GUID=1cc53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
ALTCFG=0000
OFFSET=0020  WIDTH=0004  VALUE=00000000
00 00 00 00                                      | ....

GUID=1cc53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
ALTCFG=0001
OFFSET=0020  WIDTH=0004  VALUE=00000000
00 00 00 00                                      | ....
```

The output is the same as before, so our code works correctly.

# `HIIConfig.efi extract <Guid> <Name> <Path> <Offset> <Width>`

It is possible to limit the configuration response to specific part of the storage if you supply `...&OFFSET=<Offset>&WIDTH=<Width>` in the request. For example:
```
FS0:\> HIIConfig.efi extract GUID=1cc53572800cab4c87ac3b084a6304b1&NAME=004d00610069006e0046006f0072006d00530074006100740065&PATH=01041400dfc5dcd907405e4390988970935504b27fff0400&OFFSET=0000&WIDTH=0020
Response: GUID=1cc53572800cab4c87ac3b084a6304b1&NAME=004d00610069006e0046006f0072006d00530074006100740065&PATH=01041400dfc5dcd907405e4390988970935504b27fff0400&OFFSET=0000&WIDTH=0020&VALUE=00000000000000000000000000000000000000000000007400650073006e0055


GUID=1cc53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
OFFSET=0000  WIDTH=0020  VALUE=00000000000000000000000000000000000000000000007400650073006e0055
55 00 6E 00 73 00 65 00 74 00 00 00 00 00 00 00  | U.n.s.e.t.......
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
```

Once again it is kinda hard to create configuration string if you know Guid/Name/DevicePath in their usual format. So let's create one more `extract` command call option in our program:
```
HIIConfig.efi extract <Guid> <Name> <Path> <Offset> <Width>
```

To simplify things we wouldn't perform any processing on the `<Offset>` and `<Width>` arguments, so you need to provide them exactly as they would go to the request. So to mimic request above you should write arguments like this:
```
HIIConfig.efi extract 7235C51C-0C80-4CAB-87AC-3B084A6304B1 MainFormState VenHw(D9DCC5DF-4007-435E-9098-8970935504B2) 0000 0020
```
But in case of the UEFI configuration language the leading zeros are not mandatory, so you can use something like this:
```
HIIConfig.efi extract 7235C51C-0C80-4CAB-87AC-3B084A6304B1 MainFormState VenHw(D9DCC5DF-4007-435E-9098-8970935504B2) 0 20
```
In other way you should provide `<Offset>` and `<Width>` arguments in hex without leading `0x` letters.

To handle our new command we update our code like this:
```cpp
EFI_STRING Request;
EFI_STRING Progress;
EFI_STRING Result;
if (!StrCmp(Argv[1], L"dump")) {
  <...>
} else if (!StrCmp(Argv[1], L"extract")) {
  if (Argc == 3) {
    Request = Argv[2];
  } else if ((Argc == 5) || (Argc == 7)) {                                         <---- create CfgHeader
    Status = CreateCfgHeader(Argv[2], Argv[3], Argv[4], &Request);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    if (Argc == 7) {                                                               <---- append it with `&OFFSET=<Offset>&WIDTH=<Width>` string
      EFI_STRING OffsetStr = Argv[5];
      EFI_STRING WidthStr = Argv[6];
      UINTN Size = (StrLen(Request) + StrLen(L"&OFFSET=") + StrLen(OffsetStr) + StrLen(L"&WIDTH=") + StrLen(WidthStr) + 1) * sizeof(CHAR16);
      EFI_STRING TempRequest = AllocateZeroPool(Size);
      UnicodeSPrint(TempRequest, Size, L"%s&OFFSET=%s&WIDTH=%s", Request, OffsetStr, WidthStr);
      FreePool(Request);
      Request = TempRequest;
    }
    Print(L"Request: %s\n", Request);
  } else {
    Print(L"Error! Wrong arguments\n");
    Usage();
    return EFI_INVALID_PARAMETER;
  }
  Status = gHiiConfigRouting->ExtractConfig(gHiiConfigRouting,
                                            Request,
                                            &Progress,
                                            &Result);

  if (StrCmp(Progress, L'\0')) {
    Print(L"Part of string was unparsed %s\n", Progress);
  }

  if (Argc >= 5)                                                                     <---- don't forget to free dynamically allocated request string
    FreePool(Request);

  if (EFI_ERROR(Status)) {
    Print(L"Error! ExtractConfig returned %r\n", Status);
    return Status;
  }
  Print(L"Response: ");
  PrintLongString(Result);
  Print(L"\n\n");
  PrintConfigString(Result);
  FreePool(Result);
}
```

Let's build our application and test new functionality:
```
FS0:\> HIIConfig.efi extract 7235C51C-0C80-4CAB-87AC-3B084A6304B1 MainFormState VenHw(D9DCC5DF-4007-435E-9098-8970935504B2) 0 20

Request: GUID=1cc53572800cab4c87ac3b084a6304b1&NAME=004d00610069006e0046006f0072006d00530074006100740065&PATH=01041400dfc5dcd907405e4390988970935504b27fff0400&OFFSET=0&WIDTH=20
Response: GUID=1cc53572800cab4c87ac3b084a6304b1&NAME=004d00610069006e0046006f0072006d00530074006100740065&PATH=01041400dfc5dcd907405e4390988970935504b27fff0400&OFFSET=0&WIDTH=20&VALUE=00000000000000000000000000000000000000000000007400650073006e0055


GUID=1cc53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
OFFSET=0  WIDTH=20  VALUE=00000000000000000000000000000000000000000000007400650073006e0055
55 00 6E 00 73 00 65 00 74 00 00 00 00 00 00 00  | U.n.s.e.t.......
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
```

As you see everything works exactly as with the manual configuration string.

