We've implemented various methods to issue `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExtractConfig()` call (aka HTTP GET request) to query the HII subsystem. Let's try to explore how we can issue change commands (aka HTTP POST requests) to the HII subsystem.

For this we would need the `RouteConfig()` function from the `EFI_HII_CONFIG_ROUTING_PROTOCOL`:
```
EFI_HII_CONFIG_ROUTING_PROTOCOL.RouteConfig()

Summary:
This function processes the results of processing forms and routes it to the appropriate handlers or storage.

Prototype:

typedef
EFI_STATUS
 (EFIAPI * EFI_HII_ROUTE_CONFIG ) (
 IN CONST EFI_HII_CONFIG_ROUTING_PROTOCOL *This,
 IN CONST EFI_STRING Configuration,
 OUT EFI_STRING *Progress
 );

Parameters:
This		Points to the EFI_HII_CONFIG_ROUTING_PROTOCOL instance.
Configuration	A null-terminated string in <MultiConfigResp> format.
Progress	A pointer to a string filled in with the offset of the most recent ‘&’ before the first
		failing name / value pair (or the beginning of the string if the failure is in the first
		name / value pair) or the terminating NULL if all was successful.

Description:
This function routes the results of processing forms to the appropriate targets.
```

In our `HIIConfig` application we would add two methods to issue change (=route) requests:
- by providing `<ConfigStr>`
- by providing `<Guid> <Name> <Path> <Offset> <Width> <Value>` combination

```cpp
VOID Usage()
{
  Print(L"Usage:\n");
  ...
  Print(L"HIIConfig.efi route <ConfigStr>\n");
  Print(L"HIIConfig.efi route <Guid> <Name> <Path> <Offset> <Width> <Value>\n");
}
```
This is similar to the inteface that we've added for our `export` command.

# `HIIConfig.efi route <ConfigStr>`

Here is a code that implements `HIIConfig.efi route <ConfigStr>` functionality:
```cpp
EFI_STRING Request;
EFI_STRING Progress;
EFI_STRING Result;
if (!StrCmp(Argv[1], L"dump")) {
  ...
} else if (!StrCmp(Argv[1], L"extract")) {
  ...
} else if (!StrCmp(Argv[1], L"route")) {
  if (Argc == 3) {
    Request = Argv[2];
  } else {
    Print(L"Error! Wrong arguments\n");
    Usage();
    return EFI_INVALID_PARAMETER;
  }
  Print(L"Request: %s\n", Request);
  Status = gHiiConfigRouting->RouteConfig(gHiiConfigRouting,
                                          Request,
                                          &Progress);
  if (StrCmp(Progress, L'\0')) {
    Print(L"Part of string was unparsed: %s\n", Progress);
    if (StrCmp(Progress, Request)) {
      Print(L"IMPORTANT: part of the data was written!\n");
    }
  }
  if (EFI_ERROR(Status)) {
    Print(L"Error! RouteConfig returned %r\n", Status);
    return Status;
  }
  FreePool(Result);
} else {
  Print(L"Error! Wrong arguments\n");
  Usage();
  return EFI_INVALID_PARAMETER;
}
```

The code is similar to the `HIIConfig.efi extract <ConfigStr>` call handling code. Except in this case we don't have any response string to print.

Also like before we check if the `Progress` string is empty after the call. But in case of `route` it is very important, as if the `Progress` is not empty and is not equal to the initial `Request` string it would mean that the request was processed partitially. In this case even if there would be an error for the overall `gHiiConfigRouting->RouteConfig` call, part of the data still was written to the HII subsystem! So in this case we inform a user on this subject.

Let's test our application on the `HIIFormCheckbox.efi` form. Load it's driver into the shell:
```
FS0:\> load HIIFormCheckbox.efi
```

Before we've used the form browser to check the actual form data, but as you know in case of a `efistorage` we can simply use the `dmpstore` command to get the necessary data not leaving the CLI:
```
FS0:\> dmpstore -guid ef2acc91-7b50-4ab9-ab67-2b04f8bc135e
Variable NV+BS 'EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E:CheckboxValue' DataSize = 0x01
  00000000: 01                                               *.*
```

Now let's try to change it to 0 with the `...&OFFSET=0000&WIDTH=0001&VALUE=00` command. The header of the data you can get from the `HIIConfig.efi extract ef2acc91-7b50-4ab9-ab67-2b04f8bc135e CheckboxValue VenHw(ef2acc91-7b50-4ab9-ab67-2b04f8bc135e)` output calls that we've made before. The route command call would look like this:
```
FS0:\> HIIConfig.efi route GUID=91cc2aef507bb94aab672b04f8bc135e&NAME=0043006800650063006b0062006f007800560061006c00750065&PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400&OFFSET=0000&WIDTH=0001&VALUE=00

Request: GUID=91cc2aef507bb94aab672b04f8bc135e&NAME=0043006800650063006b0062006f007800560061006c00750065&PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400&OFFSET=0000&WIDTH=0001&VALUE=00
```

After that you can verify that the data indeed was changed:
```
FS0:\> dmpstore -guid ef2acc91-7b50-4ab9-ab67-2b04f8bc135e
Variable NV+BS 'EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E:CheckboxValue' DataSize = 0x01
  00000000: 00                                               *.*
```

If you want to, you can check that the Form browser also displays the updated value.

Now you can set the checkbox element back with the `...&OFFSET=0000&WIDTH=0001&VALUE=01` call:
```
FS0:\> HIIConfig.efi route GUID=91cc2aef507bb94aab672b04f8bc135e&NAME=0043006800650063006b0062006f007800560061006c00750065&PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400&OFFSET=0000&WIDTH=0001&VALUE=01

Request: GUID=91cc2aef507bb94aab672b04f8bc135e&NAME=0043006800650063006b0062006f007800560061006c00750065&PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400&OFFSET=0000&WIDTH=0001&VALUE=01

FS0:\> dmpstore -guid ef2acc91-7b50-4ab9-ab67-2b04f8bc135e
Variable NV+BS 'EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E:CheckboxValue' DataSize = 0x01
  00000000: 01                                               *.*
```

# `HIIConfig.efi route <Guid> <Name> <Path> <Offset> <Width> <Value>`

Finally let's add a possibility to call the `route` command by supplying the arguments in a human readable format:
```
HIIConfig.efi route <Guid> <Name> <Path> <Offset> <Width> <Value>
```

Here are the necessary modifications to the code:
```cpp
EFI_STRING Request;
EFI_STRING Progress;
EFI_STRING Result;
if (!StrCmp(Argv[1], L"dump")) {
  ...
} else if (!StrCmp(Argv[1], L"extract")) {
  ...
} else if (!StrCmp(Argv[1], L"route")) {
  if (Argc == 3) {
    Request = Argv[2];
  } else if (Argc == 8) {                                                  // <---- HIIConfig.efi route <Guid> <Name> <Path> <Offset> <Width> <Value>
    Status = CreateCfgHeader(Argv[2], Argv[3], Argv[4], &Request);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    EFI_STRING OffsetStr = Argv[5];
    EFI_STRING WidthStr = Argv[6];
    EFI_STRING ValueStr = Argv[7];
    UINTN Size = (StrLen(Request) + StrLen(L"&OFFSET=") + StrLen(OffsetStr) + StrLen(L"&WIDTH=") + StrLen(WidthStr) +
                  StrLen(L"&VALUE=") + StrLen(ValueStr) + 1) * sizeof(CHAR16);
    EFI_STRING TempRequest = AllocateZeroPool(Size);
    UnicodeSPrint(TempRequest, Size, L"%s&OFFSET=%s&WIDTH=%s&VALUE=%s", Request, OffsetStr, WidthStr, ValueStr);
    FreePool(Request);
    Request = TempRequest;
  } else {
    Print(L"Error! Wrong arguments\n");
    Usage();
    return EFI_INVALID_PARAMETER;
  }
  Print(L"Request: %s\n", Request);
  Status = gHiiConfigRouting->RouteConfig(gHiiConfigRouting,
                                          Request,
                                          &Progress);
  if (StrCmp(Progress, L'\0')) {
    Print(L"Part of string was unparsed: %s\n", Progress);
    if (StrCmp(Progress, Request)) {
      Print(L"IMPORTANT: part of the data was written!\n");
    }
  }
  if (Argc == 8) {                                                        // <----- don't forget to free Request if it was allocated dynamically
    FreePool(Request);
  }
  if (EFI_ERROR(Status)) {
    Print(L"Error! RouteConfig returned %r\n", Status);
    return Status;
  }
  FreePool(Result);
} else {
  Print(L"Error! Wrong arguments\n");
  Usage();
  return EFI_INVALID_PARAMETER;
}
```
Like with the similar `extract` case we construct configuration header with the `CreateCfgHeader` function and then append the rest of the elements via the `UnicodeSPrint` call. As in this case we allocate Request string dynamically we need to free it in the end. As the `Progress` is just a pointer to some part of the `Request`, we can do it only after the `Progress` is checked for errors.

And here is a test for the new functionality:
```
FS0:\> load HIIFormCheckbox.efi
Image 'FS0:\HIIFormCheckbox.efi' loaded at 6877000 - Success
FS0:\> dmpstore -guid ef2acc91-7b50-4ab9-ab67-2b04f8bc135e
Variable NV+BS 'EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E:CheckboxValue' DataSize = 0x01
  00000000: 01                                               *.*

FS0:\> HIIConfig.efi route ef2acc91-7b50-4ab9-ab67-2b04f8bc135e CheckboxValue VenHw(ef2acc91-7b50-4ab9-ab67-2b04f8bc135e) 0 1 0
Request: GUID=91cc2aef507bb94aab672b04f8bc135e&NAME=0043006800650063006b0062006f007800560061006c00750065&PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400&OFFSET=0&WIDTH=1&VALUE=0

FS0:\> dmpstore -guid ef2acc91-7b50-4ab9-ab67-2b04f8bc135e
Variable NV+BS 'EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E:CheckboxValue' DataSize = 0x01
  00000000: 00                                               *.*

FS0:\> HIIConfig.efi route ef2acc91-7b50-4ab9-ab67-2b04f8bc135e CheckboxValue VenHw(ef2acc91-7b50-4ab9-ab67-2b04f8bc135e) 0 1 1
Request: GUID=91cc2aef507bb94aab672b04f8bc135e&NAME=0043006800650063006b0062006f007800560061006c00750065&PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400&OFFSET=0&WIDTH=1&VALUE=1

FS0:\> dmpstore -guid ef2acc91-7b50-4ab9-ab67-2b04f8bc135e
Variable NV+BS 'EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E:CheckboxValue' DataSize = 0x01
  00000000: 01                                               *.*

```

If you want to, you can issue route requests to our `HIIFormDataElements.efi` form storage, just remember these things:
- like with the `extract` case it is not mandatory for the OFFSET/WIDTH to be on element boundaries,
- don't forget to pay attention to the byte order in the `VALUE=` string and to the order of bytes in the actual form elemet. The order is reversed. So you will need to pass string element data backbards (i.e `VALUE=<o><l><l><e><H>`).
- changing element via `RouteConfig()` call can bypass some VFR limitations. For example even if numeric element has `minimum` and `maximum` attributes set like this:
```
numeric
  ...
  minimum = 0,
  maximum = 10,
  ...
endnumric
```
it would still possible to set a value outside of these limits.


