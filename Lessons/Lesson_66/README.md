Let's look at another data input VFR element - `string` (https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.6.7.1-vfr-string-statement-definition)

This element is intended to store `CHAR16` strings.

Here is minimal code that we need to add to our VFR code. `minsize`/`maxsize` elements are mandatory for the `string` element:
```
string
  varid = FormData.StringValue,
  prompt = STRING_TOKEN(STRING_PROMPT),
  help = STRING_TOKEN(STRING_HELP),
  minsize = 5,
  maxsize = 10,
endstring;
```

Off course add new tokens to the `UefiLessonsPkg/HIIFormDataElements/Strings.uni`:
```
#string STRING_PROMPT          #language en-US  "String prompt"
#string STRING_HELP            #language en-US  "String help"
```

As for our `UEFI_VARIABLE_STRUCTURE`, we need to add array of `UINT16` elements with a size at least of `maxsize`:
```
typedef struct {
  UINT8 CheckboxValue;
  UINT16 NumericValue;
  UINT16 StringValue[10];
} UEFI_VARIABLE_STRUCTURE;
```

If array size is lower than `maxsize` EDKII build system would fail:
```
ERROR 12288: String MaxSize can't be larger than the max number of elements in string array.
```

Don't forget to delete old UEFI Variable before loading our updated driver:
```
Shell> dmpstore -guid 531bc507-9191-4fa2-9446-b844e35dd12a -d
Delete variable '531BC507-9191-4FA2-9446-B844E35DD12A:FormData': Success
Shell> load fs0:HIIFormDataElements.efi
```

By default element looks like this:
![String1](String1.png?raw=true "String1")

As we have `minsize` string limit, it is not possible to enter string smaller than 5 characters:
![String2](String2.png?raw=true "String2")

It is also not possible to enter string longer than 10 characters. HII simply won't allow you to type symbols beyond the limit:
![String3](String3.png?raw=true "String3")

But if everything is in limits, you can successfully save data:
![String4](String4.png?raw=true "String4")

After that you can check our UEFI variable storage:
```
Shell> dmpstore -guid 531bc507-9191-4fa2-9446-b844e35dd12a
Variable NV+BS '531BC507-9191-4FA2-9446-B844E35DD12A:FormData' DataSize = 0x17
  00000000: 00 00 00 55 00 45 00 46-00 49 00 2D 00 4C 00 65  *...U.E.F.I.-.L.e*
  00000010: 00 73 00 73 00 6F 00                             *.s.s.o.*
```

As you can see the string doesn't contain `\0` terminating symbol. So for simplicity maybe it is better to declare array size one element larger than the value in the VFR `maxsize` field.
 field (i.e. `CHAR16 StringValue[11]`).

This way it would be easier to parse string field from our UEFI variable:
```
Shell> dmpstore -guid 531bc507-9191-4fa2-9446-b844e35dd12a
Variable NV+BS '531BC507-9191-4FA2-9446-B844E35DD12A:FormData' DataSize = 0x19
  00000000: 00 00 00 55 00 45 00 46-00 49 00 2D 00 4C 00 65  *...U.E.F.I.-.L.e*
  00000010: 00 73 00 73 00 6F 00 00-00                       *.s.s.o...*
```

# MULTI_LINE

VFR specification defines a `MULTI_LINE` flag for the `string` element:
```
string
  varid = FormData.StringValue,
  prompt = STRING_TOKEN(STRING_PROMPT),
  help = STRING_TOKEN(STRING_HELP),
  flags = MULTI_LINE,
  minsize = 5,
  maxsize = 10,
endstring;
```

This flag should work as a hint for the form browser that multi-line text can be allowed. But currently OVMF form broswer doesn't use this flag.

# IFR

You can look at IFR code `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIFormDataElements/HIIFormDataElements/DEBUG/Form.lst`:
```
    string
>00000082: 1C 90 09 00 0A 00 03 00 01 00 03 00 00 05 0A 00
      varid = FormData.StringValue,
      prompt = STRING_TOKEN(0x0009),
      help = STRING_TOKEN(0x000A),
      minsize = 5,
      maxsize = 10,
    endstring;
>00000092: 29 02
```

The data encodes `EFI_IFR_STRING` structure:
```
Summary:
Defines the string question.

Prototype:
#define EFI_IFR_STRING_OP 0x1C

typedef struct _EFI_IFR_STRING {
 EFI_IFR_OP_HEADER Header;
 EFI_IFR_QUESTION_HEADER Question;
 UINT8 MinSize;
 UINT8 MaxSize;
 UINT8 Flags;
} EFI_IFR_STRING;

Members:
Header 		The sequence that defines the type of opcode as well as the length of the opcode being defined.
		Header.OpCode = EFI_IFR_STRING_OP.
Question 	The standard question header.
MinSize 	The minimum number of characters that can be accepted for this opcode.
MaxSize 	The maximum number of characters that can be accepted for this opcode.
Flags 		Flags which control the string editing behavior.

Description:
This creates a string question. The minimum length is MinSize and the maximum length is MaxSize
```

You can parse the data, but really there is nothing new for us.

# Note

At the time of this writing you can cheat EDKII build system and declare `StringValue` array with the size of 10, but with a type of `UINT8`. The code would compile, but as you can guess, you shouldn't expect anything good from this. Right now because of this error HII system would allow you to "save" form and wouldn't even complain. But as an actual value data still wouldn't be equal to the form data, HII would you suggest to save form data endlessly.
