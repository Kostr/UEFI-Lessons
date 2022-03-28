In this lesson we are going to talk about default values for the VFR data elements.

# Setting defaults for elements

Let's try to add defaults to our `HIIFormDataElements` application.

![Before](Before.png?raw=true "Before")

We can set default values to the elements with a help of a `default` keyword:
```
checkbox
  ...
  default = TRUE,
endcheckbox;

numeric
  ...
  default = 7,
endnumeric;

string
  ...
  default = STRING_TOKEN(STRING_DEFAULT),
endstring;

date
  ...
  default = 2021/05/22,
enddate;

time
  ...
  default = 23:55:33,
endtime;

oneof
  ...
  default = 0x33,
endoneof;

orderedlist
  ...
  default = {0x0c, 0x0b, 0x0a},
endlist;
```

Don't forget to add new string token to the `Strings.uni` file:
```
#string STRING_DEFAULT         #language en-US  "String default"
```

Immediately after form load, you might see that order of `orderedlist` element was changed to the default value. This looks kinda strage, but I guess it is connected to the fact that standard value `0x000000` doesn't make any sence to the element.

![After1](After1.png?raw=true "After1")

Anyway, you can see that it is possible to set reset values to default with a help of `F9` keystroke:

![After1_1](After1_1.png?raw=true "After1_1")

If you do so, you will get confirmation window:

![After2](After2.png?raw=true "After2")

And if you press `Y`, they form values would be updated:

![After3](After3.png?raw=true "After3")

Off course to them you actually need to submit the form with `F10`.

# IFR

If you look at IFR code (`Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIFormDataElements/HIIFormDataElements/DEBUG/Form.lst`):
```
    checkbox
      ...
      default = TRUE,
>0000006A: 5B 06 00 00 00 01
    endcheckbox;

    numeric
      ...
      default = 7,
>00000086: 5B 07 00 00 01 07 00
    endnumeric;

    string
      ...
      default = STRING_TOKEN(0x001A),
>0000009F: 5B 07 00 00 07 1A 00
    endstring;

    date
      ...
      default = 2021/05/22,
>000000B6: 5B 09 00 00 06 E5 07 05 16
    enddate;

    time
      ...
      default = 23:55:33,
>000000CF: 5B 08 00 00 05 17 37 21
    endtime;

    oneof
      ...
      default = 0x33,
>000000FF: 5B 06 00 00 00 33
    endoneof;

    orderedlist
      ...
      default = {0x0c, 0x0b, 0x0a},
>0000012B: 5B 08 00 00 0B 0C 0B 0A
    endlist;
```

You'll see that all the defaults are presented with a help of `EFI_IFR_DEFAULT` opcode:
```
EFI_IFR_DEFAULT

Summary:
Provides a default value for the current question

Prototype:
#define EFI_IFR_DEFAULT_OP 0x5b

typedef struct _EFI_IFR_DEFAULT {
 EFI_IFR_OP_HEADER Header;
 UINT16 DefaultId;
 UINT8 Type;
 EFI_IFR_TYPE_VALUE Value;
} EFI_IFR_DEFAULT;

typedef struct _EFI_IFR_DEFAULT_2 {
 EFI_IFR_OP_HEADER Header;
 UINT16 DefaultId;
 UINT8 Type;
} EFI_IFR_DEFAULT_2;

Members:
Header    The sequence that defines the type of opcode as well as the length of the opcode being defined.
          For this tag, Header.OpCode = EFI_IFR_DEFAULT_OP.
DefaultId Identifies the default store for this value. The default store must have previously been created using EFI_IFR_DEFAULTSTORE.
Type      The type of data in the Value field. See EFI_IFR_TYPE_x in EFI_IFR_ONE_OF_OPTION.
Value     The default value. The actual size of this field depends on Type. If Type is EFI_IFR_TYPE_OTHER, then the default value
          is provided by a nested EFI_IFR_VALUE.

Description:
This opcode specifies a default value for the current question. There are two forms. The first (EFI_IFR_DEFAULT) assumes that the default value is a constant, embedded directly in the Value member. The second (EFI_IFR_DEFAULT_2) assumes that the default value is specified using a nested EFI_IFR_VALUE opcode.
```

All the statements above use the first form (`EFI_IFR_DEFAULT` structure) with a `Value` field included. In case you forgot, here are possible value types:
```
#define EFI_IFR_TYPE_NUM_SIZE_8    0x00
#define EFI_IFR_TYPE_NUM_SIZE_16   0x01
#define EFI_IFR_TYPE_NUM_SIZE_32   0x02
#define EFI_IFR_TYPE_NUM_SIZE_64   0x03
#define EFI_IFR_TYPE_BOOLEAN       0x04
#define EFI_IFR_TYPE_TIME          0x05
#define EFI_IFR_TYPE_DATE          0x06
#define EFI_IFR_TYPE_STRING        0x07
#define EFI_IFR_TYPE_OTHER         0x08
#define EFI_IFR_TYPE_UNDEFINED     0x09
#define EFI_IFR_TYPE_ACTION        0x0A
#define EFI_IFR_TYPE_BUFFER        0x0B
#define EFI_IFR_TYPE_REF           0x0C
#define EFI_IFR_OPTION_DEFAULT     0x10
#define EFI_IFR_OPTION_DEFAULT_MFG 0x20
```

Also you might notice that all the statements above have `DefaultId = 0`. So now it is time to talk about what default storage is.

# Default storage

First of all we need to understand that it is possible to define several default storages in UEFI.

If you look at the `Form.lst` file of our `HIIFormDataElements` application (`Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIFormDataElements/HIIFormDataElements/DEBUG/Form.lst`) you'll see that by default (excuse the pun) our formset already has 2 default storages:
```
formset
>00000000: 0E A7 07 C5 1B 53 91 91 A2 4F 94 46 B8 44 E3 5D D1 2A 02 00 03 00 01 71 99 03 93 45 85 04 4B B4 5E 32 EB 83 26 04 0E
>00000027: 5C 06 00 00 00 00
>0000002D: 5C 06 00 00 01 00
```

Opcode `0x5c` corresponds to `EFI_IFR_DEFAULTSTORE`:
```
EFI_IFR_DEFAULTSTORE

Summary:
Provides a declaration for the type of default values that a question can be associated with.

Prototype:

#define EFI_IFR_DEFAULTSTORE_OP 0x5c

typedef struct _EFI_IFR_DEFAULTSTORE {
 EFI_IFR_OP_HEADER Header;
 EFI_STRING_ID DefaultName;
 UINT16 DefaultId;
} EFI_IFR_DEFAULTSTORE;

Members:
Header      The sequence that defines the type of opcode as well as the length of the opcode being defined.
            For this tag, Header.OpCode = EFI_IFR_DEFAULTSTORE_OP
DefaultName A string token reference for the human readable string associated with the type of default being declared.
DefaultId   The default identifier, which is unique within the current form set. The default identifier creates a group of defaults.

Description:
Declares a class of default which can then have question default values associated with. An EFI_IFR_DEFAULTSTORE with a specified DefaultId must appear in the IFR before it can be referenced by an EFI_IFR_DEFAULT.
```

As you can see the code abobe defines two default storages without names with IDs equal to `0x0000` and `0x0001`. 

Generally UEFI specification divides all the possible values for default storage ID (`UINT16`) into 4 classes:
- `0x0000-0x3fff`: reserved by UEFI specification
- `0x4000-0x7fff`: for platform providers usage
- `0x8000-0xbfff`: for hardware vendors usage
- `0xc000-0xffff`: for firmware vendors usage

Within the first group there are 3 predefined classes:
- `0x0000` - "standard defaults" (defaults used to prepare the system/device for normal operation)
- `0x0001` - "manufacturing defaults" (defaults used to prepare the system/device for manufacturing)
- `0x0002` - "safe defaults" (defaults used to boot the system in a "safe" or low-risk mode)

So as you can see in our formset we have 2 default storages: "standard defaults" and "manufacturing defaults". They are always created by the VFR compier for every formset.

You can find all the defines for the values above in the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiInternalFormRepresentation.h:
```
#define EFI_HII_DEFAULT_CLASS_STANDARD       0x0000
#define EFI_HII_DEFAULT_CLASS_MANUFACTURING  0x0001
#define EFI_HII_DEFAULT_CLASS_SAFE           0x0002
#define EFI_HII_DEFAULT_CLASS_PLATFORM_BEGIN 0x4000
#define EFI_HII_DEFAULT_CLASS_PLATFORM_END   0x7fff
#define EFI_HII_DEFAULT_CLASS_HARDWARE_BEGIN 0x8000
#define EFI_HII_DEFAULT_CLASS_HARDWARE_END   0xbfff
#define EFI_HII_DEFAULT_CLASS_FIRMWARE_BEGIN 0xc000
#define EFI_HII_DEFAULT_CLASS_FIRMWARE_END   0xffff
```

# Define default storage in VFR

If you want to support several default storages in VFR you most probably need to define them explicitly.

VFR has for this special `defaultstore` element https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/26_vfr_default_stores_definition

Element syntax looks like this:
```
defaultstore <STORAGE NAME>,
  prompt      = <STRING TOKEN>,
  attribute   = <STORAGE ID>;
```

So to explicitly define "standard defaults" and "manufacturing defaults" we can use something like this:
```
defaultstore StandardDefault,
  prompt      = STRING_TOKEN(STANDARD_DEFAULT_PROMPT),
  attribute   = 0x0000;

defaultstore ManufactureDefault,
  prompt      = STRING_TOKEN(MFG_DEFAULT_PROMPT),
  attribute   = 0x0001;
```

Off course you need to add new string tokens to the UNI file:
```
#string STANDARD_DEFAULT_PROMPT     #language en-US "Standard default"
#string MFG_DEFAULT_PROMPT          #language en-US "Manufacture default"
```

Now you can supply different defines for the storages:
```
numeric
  ...
  default = 7, defaultstore = StandardDefault,
  default = 8, defaultstore = ManufactureDefault,
endnumeric;
```

# IFR

You can look at IFR code and see for yourself, how our `defaultstore` code has trasformed `EFI_IFR_DEFAULTSTORE` opcodes:
```
formset
>00000000: 0E A7 07 C5 1B 53 91 91 A2 4F 94 46 B8 44 E3 5D D1 2A 02 00 03 00 01 71 99 03 93 45 85 04 4B B4 5E 32 EB 83 26 04 0E
>00000027: 5C 06 1B 00 00 00
>0000002D: 5C 06 1E 00 01 00
```
Practically nothing has changed, only the string token for the `DefaultName` field was updated.

But now you can see how default values referece our storages:
```
    numeric
       ...
       default = 7, defaultstore = StandardDefault,
 >00000086: 5B 07 00 00 01 07 00                            // reference storage 0x0000
       default = 8, defaultstore = ManufactureDefault,
 >0000008D: 5B 07 01 00 01 08 00                            // reference storage 0x0001
     endnumeric;
```

# resetbutton

Earlier to reset values to the defaults we have used `F9` key in the form browser. How do we do it now, when we have several default storages?

For this kind of functionality VFR offers `resetbutton` element. This elements creates a button on the form that on press will set question values to the values from the approprite defaultstorage.

Create two buttons for our two default storages:
```
    resetbutton
      defaultstore = MyStandardDefault,                              // <STORAGE NAME> of the target `defaultstore` element
      prompt   = STRING_TOKEN(STR_STANDARD_DEFAULT_PROMPT),
      help     = STRING_TOKEN(STR_STANDARD_DEFAULT_HELP),
    endresetbutton;

    resetbutton
      defaultstore = MyManufactureDefault,
      prompt   = STRING_TOKEN(STR_MANUFACTURE_DEFAULT_PROMPT),
      help     = STRING_TOKEN(STR_MANUFACTURE_DEFAULT_HELP),
    endresetbutton;
```

Don't forget to add new string tokens to the UNI file:
```
#string BTN_STANDARD_DEFAULT_PROMPT #language en-US "Reset to standard default prompt"
#string BTN_STANDARD_DEFAULT_HELP   #language en-US "Reset to standard default help"
#string BTN_MFG_DEFAULT_PROMPT      #language en-US "Reset to manufacture default prompt"
#string BTN_MFG_DEFAULT_HELP        #language en-US "Reset to manufacture default help"
```

With this code our form would look like this:

![ResetButton1](ResetButton1.png?raw=true "ResetButton1")

You can verify that the first button sets `numeric` value to 7:

![ResetButton2](ResetButton2.png?raw=true "ResetButton2")

And the second one to 8:

![ResetButton3](ResetButton3.png?raw=true "ResetButton3")

# IFR

In IFR `resetbutton` looks like this:
```
    resetbutton
>00000146: 0D 88 1F 00 20 00 01 00
      defaultstore = ManufactureDefault,
      prompt = STRING_TOKEN(0x001F),
      help = STRING_TOKEN(0x0020),
    endresetbutton;
```

This element is defined by the `EFI_IFR_RESET_BUTTON` opcode:
```
EFI_IFR_RESET_BUTTON

Summary:
Create a reset or submit button on the current form.

Prototype:

#define EFI_IFR_RESET_BUTTON_OP 0x0d

typedef struct _EFI_IFR_RESET_BUTTON {
 EFI_IFR_OP_HEADER Header;
 EFI_IFR_STATEMENT_HEADER Statement;
 EFI_DEFAULT_ID DefaultId;
} EFI_IFR_RESET_BUTTON;

typedef UINT16 EFI_DEFAULT_ID;

Members:
Header     The standard header, where Header.OpCode = EFI_IFR_RESET_BUTTON_OP.
Statement  Standard statement header, including the prompt and help text.
DefaultId  Specifies the set of default store to use when restoring the defaults to the questions on this form.

Description:
This opcode creates a user-selectable button that resets the question values for all questions on the current form to the default values specified by DefaultId.
```

# Other methods for defining default values

`oneof` is the most common element in every HII menu and this element has a possibility to define "standard storage" and "manufacturing storage" values in place of an option with a help of a `flags` field. This improves element readibility:
```
oneof
  varid = FormData.OneOfValue,
  prompt = STRING_TOKEN(ONEOF_PROMPT),
  help = STRING_TOKEN(ONEOF_HELP),
  option text = STRING_TOKEN(ONEOF_OPTION1), value = 0x00, flags = 0;
  option text = STRING_TOKEN(ONEOF_OPTION2), value = 0x33, flags = MANUFACTURING;
  option text = STRING_TOKEN(ONEOF_OPTION3), value = 0x55, flags = DEFAULT;
endoneof;
```
In this case `0x55` value is set for the "standard storage" and `0x33` value is set for the "manufacturing storage". You can verify this functionality with the resetbuttons that we've created.

You can mix defaults with `flags` and the ones defined with the `default` keyword, UEFI specification has a strict preference of order in this case. Although this code would look rather strange, so I wouldn't recommend to do it.

Keep in mind that the `flags` method can help to create references only to "standard storage" and "manufacturing storage", you can't define references to your custom storages with different defaultStorage IDs.

# Non-constant default values

Defaults can be defined as expressions. In this case `defaults value = <...>,` syntax is used. Most often it is used when default is dependent on other question value. For example:

```
oneof
  name = OneOfQuestion,
  varid = FormData.OneOfValue,
  prompt = STRING_TOKEN(ONEOF_PROMPT),
  help = STRING_TOKEN(ONEOF_HELP),
  option text = STRING_TOKEN(ONEOF_OPTION1), value = 0x00, flags = 0;
  option text = STRING_TOKEN(ONEOF_OPTION2), value = 0x33, flags = MANUFACTURING;
  option text = STRING_TOKEN(ONEOF_OPTION3), value = 0x55, flags = DEFAULT;
endoneof;

numeric
  name = NumericQuestion,
  varid = FormData.NumericValue,
  prompt = STRING_TOKEN(NUMERIC_PROMPT),
  help = STRING_TOKEN(NUMERIC_HELP),
  flags = NUMERIC_SIZE_2 | DISPLAY_UINT_HEX,
  minimum = 0,
  maximum = 0xff,
  step = 1,
  default value = questionref(OneOfQuestion),
endnumeric;
```

With this code you can see that reset to "standard storage" will result to `numeric` value change to `0x55` and reset to "manufacturing storage" will result to `numeric` value change to `0x33`.

Don't forget that the reference `name` should be defined before the actual `questionref(<name>)` usage.

If you look at the IFR now, you'll see that it is encoded in a completely different way:
```
    numeric
      ...
      default value = questionref(OneOfQuestion),
>000000AE: 5B 85 00 00 08	// EFI_IFR_DEFAULT_2 (structure form without a `Value` field)
>000000B3: 5A 82                // EFI_IFR_VALUE
>000000B5: 40 04 02 00		// EFI_IFR_QUESTION_REF1
```

You can see that the `Type` field in the `EFI_IFR_DEFAULT_2` structure in this case has `EFI_IFR_TYPE_OTHER` value.

In case you want to check IFR, here are defines for the `EFI_IFR_VALUE` and `EFI_IFR_QUESTION_REF1` opcodes:
```
EFI_IFR_VALUE

Summary:
Provides a value for the current question or default.

Prototype:

#define EFI_IFR_VALUE_OP 0x5a

typedef struct _EFI_IFR_VALUE {
 EFI_IFR_OP_HEADER Header;
} EFI_IFR_VALUE;

Members:
Header 	The sequence that defines the type of opcode as well as the length of the opcode being defined.
	For this tag, Header.OpCode = EFI_IFR_VALUE_OP

Description:
Creates a value for the current question or default with no storage. The value is the result of the expression nested in the scope.
```

```
EFI_IFR_QUESTION_REF1

Summary:
Push a question’s value on the expression stack

Prototype:

#define EFI_IFR_QUESTION_REF1_OP 0x40

typedef struct _EFI_IFR_QUESTION_REF1 {
 EFI_IFR_OP_HEADER Header;
 EFI_QUESTION_ID QuestionId;
} EFI_IFR_QUESTION_REF1;

Members:
Header 		The byte sequence that defines the type of opcode as well as the length of the opcode being defined.
		Header.OpCode = EFI_IFR_QUESTION_REF1_OP.
QuestionId 	The question’s identifier, which must be unique within the form set.

Description:
Push the value of the question specified by QuestionId on to the expression stack
```
