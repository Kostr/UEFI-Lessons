In this lesson we would investigate how to modify form elements that have keyword references.

But rather than change some system variables let's add keyword references to our own application.

# Adding keywords to form elements

For an example let's take our `HIIFormDataElements` driver. Adding keywords is very simple. The only file that we would need to modify is UNI file (`Strings.uni` in this case).

First we need to add a new namespace id. It is added via the new language definition with an identificator in a format `x-UEFI-*`. Let's use `x-UEFI-OEM` in our case. The language text representation is not important, here we use `"OEM_NameSpace"` as a placeholder:
```
#langdef x-UEFI-OEM "OEM_NameSpace"
```

Now we need to add `#language x-UEFI-OEM "<...>"` string translations to the prompts of all form elements that we want to refer by keywords. For example let's add keys to the Checkbox, Numeric and String elements:
```
...
#string CHECKBOX_PROMPT        #language en-US  "Checkbox prompt"
                               #language x-UEFI-OEM  "CheckboxKey"
#string NUMERIC_PROMPT         #language en-US  "Numeric prompt"
                               #language x-UEFI-OEM  "NumericKey"
#string STRING_PROMPT          #language en-US  "String prompt"
                               #language x-UEFI-OEM  "StringKey"
...
```
The provided values inside these translation strings would act as keywords for these elements. Just in case, it is completely allowed to use space `" "` inside a key name.

Now build the updated driver and load it in the UEFI shell. Here I've decided to create a new driver `HIIFormDataElementsWithKeywords` to keep some separation between the lessons, but generally it is just a copy of `HIIFormDataElements.efi` with the modifications described above:
```
FS0:\> load HIIFormDataElementsWithKeywords.efi
Image 'FS0:\HIIFormDataElementsWithKeywords.efi' loaded at 6880000 - Success
```

Now use `HIIKeyword` application to dump all the keys of our new namespace:
```
FS0:\> HIIKeyword.efi get "NAMESPACE=x-UEFI-OEM" ""
Response: NAMESPACE=x-UEFI-OEM&PATH=0104140075f599c2ddf17d4db7aae5064b3ecbd77fff0400&KEYWORD=CheckboxKey&VALUE=00&NAMESPACE=x-UEFI-OEM&PATH=0104140075f599c2ddf17d4db7aae5064b3ecbd77fff0400&KEYWORD=NumericKey&VALUE=0000&NAMESPACE=x-UEFI-OEM&PATH=0104140075f599c2ddf17d4db7aae5064b3ecbd77fff0400&KEYWORD=StringKey&VALUE=000000<...>


NAMESPACE=x-UEFI-OEM
PATH=0104140075f599c2ddf17d4db7aae5064b3ecbd77fff0400 (VenHw(C299F575-F1DD-4D7D-B7AA-E5064B3ECBD7))
KEYWORD=CheckboxKey
VALUE=00
00                                               | .

NAMESPACE=x-UEFI-OEM
PATH=0104140075f599c2ddf17d4db7aae5064b3ecbd77fff0400 (VenHw(C299F575-F1DD-4D7D-B7AA-E5064B3ECBD7))
KEYWORD=NumericKey
VALUE=0000
00 00                                            | ..

NAMESPACE=x-UEFI-OEM
PATH=0104140075f599c2ddf17d4db7aae5064b3ecbd77fff0400 (VenHw(C299F575-F1DD-4D7D-B7AA-E5064B3ECBD7))
KEYWORD=StringKey
VALUE=0000000000000000000000000000000000000000
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
00 00 00 00                                      | ....

```

You can verify that it is possible to access individual keys:
```
FS0:\> HIIKeyword.efi get "NAMESPACE=x-UEFI-OEM" "KEYWORD=NumericKey"
Response: NAMESPACE=x-UEFI-OEM&PATH=0104140075f599c2ddf17d4db7aae5064b3ecbd77fff0400&KEYWORD=NumericKey&VALUE=0000


NAMESPACE=x-UEFI-OEM
PATH=0104140075f599c2ddf17d4db7aae5064b3ecbd77fff0400 (VenHw(C299F575-F1DD-4D7D-B7AA-E5064B3ECBD7))
KEYWORD=NumericKey
VALUE=0000
00 00                                            | ..

```

# Use `EFI_KEYWORD_HANDLER_PROTOCOL.SetData()` to modify keyword data

Now we are ready to add support for keyword data modification to the `HIIKeyword` application. For this we will need `SetData()` function from the `EFI_KEYWORD_HANDLER_PROTOCOL`:
```cpp
EFI_KEYWORD_HANDLER_PROTOCOL.SetData()

Summary:
Set the data associated with a particular configuration namespace keyword.

Prototype:

typedef
EFI_STATUS
(EFIAPI *EFI_KEYWORD_HANDLER _SET_DATA) (
 IN EFI_KEYWORD_HANDLER_PROTOCOL *This,
 IN CONST EFI_STRING KeywordString,
 OUT EFI_STRING *Progress,
 OUT UINT32 *ProgressErr
 );

Parameters:
This 		Pointer to the EFI_KEYWORD_HANDLER _PROTOCOL instance.
KeywordString 	A null-terminated string in <MultiKeywordResp> format.
Progress 	On return, points to a character in the KeywordString. Points to the string’s NULL terminator if the request was successful.
		Points to the most recent ‘&’ before the first failing name / value pair (or the beginning of the string if the failure is in the first name / value pair) if the request was not successful.
ProgressErr 	If during the processing of the KeywordString there was a failure, this parameter gives additional information about the possible source of the problem.

Description:
This function accepts a <MultiKeywordResp> formatted string, finds the associated keyword owners, creates a <MultiConfigResp> string from it and forwards it to the EFI_HII_ROUTING_PROTOCOL.RouteConfig function.
```

The call signature of our program would be modified like this:
```cpp
VOID Usage()
{
  Print(L"Usage:\n");
  Print(L"HIIKeyword get <NamespaceStr> <KeywordStr>\n");
  Print(L"HIIKeyword set <KeywordStr>\n");
}
```

The support code is even simplier than in the `HIIKeyword get <...>` case:
```cpp
EFI_STRING Progress;
UINT32 ProgressErr;
if (!StrCmp(Argv[1], L"get")) {
  <...>
} else if (!StrCmp(Argv[1], L"set")) {
  if (Argc != 3) {
    Print(L"Wrong argument!\n");
    Usage();
    return EFI_INVALID_PARAMETER;
  }
  Status = gHiiConfigKeywordHandler->SetData(gHiiConfigKeywordHandler,
                                             Argv[2],
                                             &Progress,
                                             &ProgressErr);
  if (StrCmp(Progress, L'\0')) {
    Print(L"Part of string was unparsed %s\n", Progress);
  }
  if (ProgressErr) {
    Print(L"Error! ProgressErr=%s\n", ProgressErrorStr(ProgressErr));
  }
  if (EFI_ERROR(Status)) {
    Print(L"Error! SetData returned %r\n", Status);
    return Status;
  }
} else {
  Print(L"Wrong argument!\n");
  Usage();
  return EFI_INVALID_PARAMETER;
}
```

So let's rebuild `HIIKeyword.efi` and test this new functionality. Once again first load our custom form driver:
```
FS0:\> load HIIFormDataElementsWithKeywords.efi
Image 'FS0:\HIIFormDataElementsWithKeywords.efi' loaded at 6880000 - Success
```
Now with the `HIIKeyword.efi` try to change numeric element value to 7:
```
FS0:\> HIIKeyword.efi set "NAMESPACE=x-UEFI-OEM&KEYWORD=NumericKey&VALUE=0007"
```
Verify that the modification was completed successfully:
```
FS0:\> HIIKeyword.efi get "NAMESPACE=x-UEFI-OEM" "KEYWORD=NumericKey"
Response: NAMESPACE=x-UEFI-OEM&PATH=0104140075f599c2ddf17d4db7aae5064b3ecbd77fff0400&KEYWORD=NumericKey&VALUE=0007


NAMESPACE=x-UEFI-OEM
PATH=0104140075f599c2ddf17d4db7aae5064b3ecbd77fff0400 (VenHw(C299F575-F1DD-4D7D-B7AA-E5064B3ECBD7))
KEYWORD=NumericKey
VALUE=0007
07 00                                            | ..
```

If you want to, you can even check the form browser:

![1](1.png?raw=true "1")

I want to point out that like in the `RouteConfig()` case, modification through the `EFI_KEYWORD_HANDLER_PROTOCOL.SetData()` call bypasses some of the VFR checks. For example even if the current numeric value limits are `0..10`
```
numeric
  ...
  minimum = 0,
  maximum = 10,
  ...
endnumeric;
```
it is still possible to change it to something like `0xABCD` like this:
```
FS0:\> HIIKeyword.efi set "NAMESPACE=x-UEFI-OEM&KEYWORD=NumericKey&VALUE=ABCD"
```
Once again you can verify updated value with our program:
```
FS0:\> HIIKeyword.efi get "NAMESPACE=x-UEFI-OEM" "KEYWORD=NumericKey"
Response: NAMESPACE=x-UEFI-OEM&PATH=0104140075f599c2ddf17d4db7aae5064b3ecbd77fff0400&KEYWORD=NumericKey&VALUE=abcd


NAMESPACE=x-UEFI-OEM
PATH=0104140075f599c2ddf17d4db7aae5064b3ecbd77fff0400 (VenHw(C299F575-F1DD-4D7D-B7AA-E5064B3ECBD7))
KEYWORD=NumericKey
VALUE=abcd
CD AB                                            | ..
```
Or with the form browser:

![2](2.png?raw=true "2")

