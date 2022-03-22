# `oneof` element

`oneof` element allows to select an option from the predefined set (https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.6.6.2-vfr-oneof-statement-definition)

Add the folowing code to the `UefiLessonsPkg/HIIFormDataElements/Form.vfr`
```
oneof
  varid = FormData.OneOfValue,
  prompt = STRING_TOKEN(ONEOF_PROMPT),
  help = STRING_TOKEN(ONEOF_HELP),
  option text = STRING_TOKEN(ONEOF_OPTION1), value = 0x00, flags = DEFAULT;
  option text = STRING_TOKEN(ONEOF_OPTION2), value = 0x33, flags = 0;
  option text = STRING_TOKEN(ONEOF_OPTION3), value = 0x55, flags = 0;
endoneof;
```

Add new string tokens to the `UefiLessonsPkg/HIIFormDataElements/Strings.uni`:
```
#string ONEOF_PROMPT           #language en-US  "OneOf list prompt"
#string ONEOF_HELP             #language en-US  "OneOf list help"
#string ONEOF_OPTION1          #language en-US  "OneOf list option 1"
#string ONEOF_OPTION2          #language en-US  "OneOf list option 2"
#string ONEOF_OPTION3          #language en-US  "OneOf list option 3"
```

As for data, add this to the `UefiLessonsPkg/HIIFormDataElements/Data.h`:
```
typedef struct {
  ...
  UINT8 OneOf;
} UEFI_VARIABLE_STRUCTURE;
```

Once you load the form, you would get:

![OneOf1](OneOf1.png?raw=true "OneOf1")

You can select one of the available options:

![OneOf2](OneOf2.png?raw=true "OneOf2")

If you select the option 2:

![OneOf3](OneOf3.png?raw=true "OneOf3")

The variable field would get the value `0x33`. If you want to, you can verify it with the `dmpstore` command.

## IFR

IFR code would look like this (`Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIFormDataElements/HIIFormDataElements/DEBUG/Form.lst`):
```
    oneof
>000000B4: 05 91 0F 00 10 00 06 00 01 00 20 00 00 10 00 55 00
      varid = FormData.OneOfValue,
      prompt = STRING_TOKEN(0x000F),
      help = STRING_TOKEN(0x0010),
      option text = STRING_TOKEN(0x0011), value = 0x00, flags = DEFAULT;
>000000C5: 09 07 11 00 10 00 00
      option text = STRING_TOKEN(0x0012), value = 0x33, flags = 0;
>000000CC: 09 07 12 00 00 00 33
      option text = STRING_TOKEN(0x0013), value = 0x55, flags = 0;
>000000D3: 09 07 13 00 00 00 55
    endoneof;
>000000DA: 29 02
```

The first opcodes are:
```
EFI_IFR_ONE_OF

Summary:
Creates a select-one-of question.

Prototype:

#define EFI_IFR_ONE_OF_OP 0x05

typedef struct _EFI_IFR_ONE_OF {
 EFI_IFR_OP_HEADER Header;
 EFI_IFR_QUESTION_HEADER Question;
 UINT8 Flags;
 union {
 struct {
 UINT8 MinValue;
 UINT8 MaxValue;
 UINT8 Step;
 } u8;
 struct {
 UINT16 MinValue;
 UINT16 MaxValue;
 UINT16 Step;
 } u16;
 struct {
 UINT32 MinValue;
 UINT32 MaxValue;
 UINT32 Step;
 } u32;
 struct {
 UINT64 MinValue;
 UINT64 MaxValue;
 UINT64 Step;
 } u64;
 } data;
} EFI_IFR_ONE_OF;

Members:
Header 		The sequence that defines the type of opcode as well as the length of the opcode being defined.
		Header.OpCode = EFI_IFR_ONE_OF_OP.
Question 	The standard question header.
Flags 		Specifies flags related to the numeric question.
MinValue 	The minimum value to be accepted by the browser for this opcode.
		The size of the data field may vary from 8 to 64 bits, depending on the size specified in Flags
MaxValue 	The maximum value to be accepted by the browser for this opcode.
		The size of the data field may vary from 8 to 64 bits, depending on the size specified in Flags
Step 		Defines the amount to increment or decrement the value each time a user requests a value change.
		If the step value is 0, then the input mechanism for the numeric value is to be free-form
		and require the user to type in the actual value.
		The size of the data field may vary from 8 to 64 bits, depending on the size specified in Flags

Description:
This opcode creates a select-on-of object, where the user must select from one of the nested options.
This is identical to EFI_IFR_NUMERIC.
```

And here is a definition for the options opcodes:
```
EFI_IFR_ONE_OF_OPTION

Summary:
Creates a pre-defined option for a question.

Prototype:

#define EFI_IFR_ONE_OF_OPTION_OP 0x09

typedef struct _EFI_IFR_ONE_OF_OPTION {
 EFI_IFR_OP_HEADER Header;
 EFI_STRING_ID Option;
 UINT8 Flags;
 UINT8 Type;
 EFI_IFR_TYPE_VALUE Value;
} EFI_IFR_ONE_OF_OPTION;

Members:
Header	 The sequence that defines the type of opcode as well as the length of the opcode being defined.
	 Header.OpCode = EFI_IFR_ONE_OF_OPTION_OP.
Option   The string token reference to the option description string for this particular opcode.
Flags    Specifies the flags associated with the current option (EFI_IFR_OPTION_x)
Type 	 Specifies the type of the option’s value (See EFI_IFR_TYPE)
Value 	 The union of all of the different possible values. The actual contents (and size)
	 of the field depends on Type.
```

Description says that the content of the `Value` field is dependent on the value of the `Type` field. Here are all the possible value types with comments of their type flags:
```
typedef union {
 UINT8 u8;             // EFI_IFR_TYPE_NUM_SIZE_8
 UINT16 u16;           // EFI_IFR_TYPE_NUM_SIZE_16
 UINT32 u32;           // EFI_IFR_TYPE_NUM_SIZE_32
 UINT64 u64;           // EFI_IFR_TYPE_NUM_SIZE_64
 BOOLEAN b;            // EFI_IFR_TYPE_BOOLEAN
 EFI_HII_TIME time;    // EFI_IFR_TYPE_TIME
 EFI_HII_DATE date;    // EFI_IFR_TYPE_DATE
 EFI_STRING_ID string; // EFI_IFR_TYPE_STRING, EFI_IFR_TYPE_ACTION
 EFI_HII_REF ref;      // EFI_IFR_TYPE_REF
 // UINT8 buffer[];    // EFI_IFR_TYPE_BUFFER
} EFI_IFR_TYPE_VALUE;
```

And here are the actual values for the `EFI_IFR_TYPE_x` defines:
```
#define EFI_IFR_TYPE_NUM_SIZE_8 	0x00
#define EFI_IFR_TYPE_NUM_SIZE_16 	0x01
#define EFI_IFR_TYPE_NUM_SIZE_32 	0x02
#define EFI_IFR_TYPE_NUM_SIZE_64 	0x03
#define EFI_IFR_TYPE_BOOLEAN 		0x04
#define EFI_IFR_TYPE_TIME 		0x05
#define EFI_IFR_TYPE_DATE 		0x06
#define EFI_IFR_TYPE_STRING 		0x07
#define EFI_IFR_TYPE_OTHER 		0x08
#define EFI_IFR_TYPE_UNDEFINED 		0x09
#define EFI_IFR_TYPE_ACTION 		0x0A
#define EFI_IFR_TYPE_BUFFER 		0x0B
#define EFI_IFR_TYPE_REF 		0x0C
```

In our structure we've declared data as `UINT8 OneOf`. Therefore compiler automatically have deducted our type flag as `EFI_IFR_TYPE_NUM_SIZE_8`. If we change that to `UINT16 OneOf`, compiler would change the type field value to the `EFI_IFR_TYPE_NUM_SIZE_16`.

Compare this output with the one that we had before:
```
    oneof
>000000B4: 05 94 0F 00 10 00 06 00 01 00 20 00 00 11 00 00 55 00 00 00
      varid = FormData.OneOfValue,
      prompt = STRING_TOKEN(0x000F),
      help = STRING_TOKEN(0x0010),
      option text = STRING_TOKEN(0x0011), value = 0x00, flags = DEFAULT;
>000000C8: 09 08 11 00 11 01 00 00
      option text = STRING_TOKEN(0x0012), value = 0x33, flags = 0;
>000000D0: 09 08 12 00 01 01 33 00
      option text = STRING_TOKEN(0x0013), value = 0x55, flags = 0;
>000000D8: 09 08 13 00 01 01 55 00
    endoneof;
>000000E0: 29 02
```

Although `EFI_IFR_TYPE_VALUE` can take many values besides numeric if you try to set its type to the `EFI_HII_DATE OneOfValue` for example, you would get an error:
```
ERROR 12288: OneOf question only support UINT8, UINT16, UINT32 and UINT64 data type
```

So only subtype of `EFI_IFR_TYPE_VALUE` is supported.

# `orderedlist` element

Another element with options is the `orderedlist` element (https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.6.8-vfr-orderedlist-statement-definition)

Add this to the `UefiLessonsPkg/HIIFormDataElements/Form.vfr`:
```
orderedlist
  varid = FormData.OrderedListValue,
  prompt = STRING_TOKEN(ORDERED_LIST_PROMPT),
  help = STRING_TOKEN(ORDERED_LIST_HELP),
  option text = STRING_TOKEN(ORDERED_LIST_OPTION1), value = 0x0A, flags = 0;
  option text = STRING_TOKEN(ORDERED_LIST_OPTION2), value = 0x0B, flags = 0;
  option text = STRING_TOKEN(ORDERED_LIST_OPTION3), value = 0x0C, flags = 0;
endlist;
```

Add strings to the `UefiLessonsPkg/HIIFormDataElements/Strings.uni`:
```
#string ORDERED_LIST_PROMPT    #language en-US  "Ordered list prompt"
#string ORDERED_LIST_HELP      #language en-US  "Ordered list help"
#string ORDERED_LIST_OPTION1   #language en-US  "Ordered list option 1"
#string ORDERED_LIST_OPTION2   #language en-US  "Ordered list option 2"
#string ORDERED_LIST_OPTION3   #language en-US  "Ordered list option 3"
```

If we have 3 options, we need to declare array of 3 elements. Let's declare them as `UINT8`:
```
typedef struct {
  ...
  UINT8 OrderedListValue[3];
} UEFI_VARIABLE_STRUCTURE;
```

On load our form would look like this:

![Orderedlist1](Orderedlist1.png?raw=true "Orderedlist1")

If you select the element:

![Orderedlist2](Orderedlist2.png?raw=true "Orderedlist2")

You can change its order by moving it up or down with the help of `+` and `-` keys:

![Orderedlist3](Orderedlist3.png?raw=true "Orderedlist3")

Change order to `2 1 3` and save it with `F10`:

![Orderedlist4](Orderedlist4.png?raw=true "Orderedlist4")

This would get the following data in the `OrderedListValue` field of our UEFI variable:
```
0B 0A 0C
```

## IFR

Let's look at the IFR code (`Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIFormDataElements/HIIFormDataElements/DEBUG/Form.lst`):
```
    orderedlist
>000000DC: 23 8F 14 00 15 00 07 00 01 00 21 00 00 03 00
      varid = FormData.OrderedListValue,
      prompt = STRING_TOKEN(0x0014),
      help = STRING_TOKEN(0x0015),
      option text = STRING_TOKEN(0x0016), value = 0x0A, flags = 0;
>000000EB: 09 07 16 00 00 00 0A
      option text = STRING_TOKEN(0x0017), value = 0x0B, flags = 0;
>000000F2: 09 07 17 00 00 00 0B
      option text = STRING_TOKEN(0x0018), value = 0x0C, flags = 0;
>000000F9: 09 07 18 00 00 00 0C
    endlist;
>00000100: 29 02
```

First code element is `EFI_IFR_ORDERED_LIST`:
```
EFI_IFR_ORDERED_LIST

Summary:
Creates a set question using an ordered list.

#define EFI_IFR_ORDERED_LIST_OP 0x23

typedef struct _EFI_IFR_ORDERED_LIST {
 EFI_IFR_OP_HEADER Header;
 EFI_IFR_QUESTION_HEADER Question;
 UINT8 MaxContainers;
 UINT8 Flags;
} EFI_IFR_ORDERED_LIST;

Members:
Header 		The byte sequence that defines the type of opcode as well as the length of the opcode being defined.
		Header.OpCode = EFI_IFR_ORDERED_LIST_OP.
Question 	The standard question header.
MaxContainers 	The maximum number of entries for which this tag will maintain an order.
		This value also identifies the size of the storage associated with this tag’s ordering array.
Flags		A bit-mask that determines which unique settings are active for this opcode.

Description:
Create an ordered list question in the current form. One thing to note is that valid values for the options
in ordered lists should never be a 0.
```

If you match data to the fields, you would get the following data for the new fields:
```
UINT8 MaxContainers;	// 0x03
UINT8 Flags;		// 0x00
```

Everything is in order, our element has 3 options, and therefore `MaxContainers=0x03`.

If you look at the options IFR code you could see that the options are encoded with the same opcode `EFI_IFR_ONE_OF_OPTION` that was used in the `oneof` element.

Here the principle is the same. If you encode our data as `UINT16` instead of `UINT8`
```
UINT16 OrderedListValue[3];
```
The compiler would encode options as `EFI_IFR_TYPE_NUM_SIZE_16` (=`UINT16`) and in the storage they would look as:
```
0B 00 0A 00 0C 00
```

With the `orderedlist` it is even possible to use non-numeric data types. For example you can encode variable as array of dates:
```
EFI_HII_DATE OrderedListValue[3];
```

For this off course you need to change the code in VFR:
```
orderedlist
  varid = FormData.OrderedListValue,
  prompt = STRING_TOKEN(ORDERED_LIST_PROMPT),
  help = STRING_TOKEN(ORDERED_LIST_HELP),
  option text = STRING_TOKEN(ORDERED_LIST_OPTION1), value = 2021/7/4, flags = 0;
  option text = STRING_TOKEN(ORDERED_LIST_OPTION2), value = 2022/8/5, flags = 0;
  option text = STRING_TOKEN(ORDERED_LIST_OPTION3), value = 2023/9/6, flags = 0;
endlist;
```

If you parse IFR data for such code, you could see that every option is encoded with `#define EFI_IFR_TYPE_DATE 0x06` type:
```
    orderedlist
>000000DC: 23 8F 14 00 15 00 07 00 01 00 21 00 00 0C 00
      varid = FormData.OrderedListValue,
      prompt = STRING_TOKEN(0x0014),
      help = STRING_TOKEN(0x0015),
      option text = STRING_TOKEN(0x0016), value = 2021/7/4, flags = 0;
>000000EB: 09 0A 16 00 06 06 E5 07 07 04
      option text = STRING_TOKEN(0x0017), value = 2022/8/5, flags = 0;
>000000F5: 09 0A 17 00 06 06 E6 07 08 05
      option text = STRING_TOKEN(0x0018), value = 2023/9/6, flags = 0;
>000000FF: 09 0A 18 00 06 06 E7 07 09 06
    endlist;
>00000109: 29 02
```

If the data array has a size lower than amount of available options, everything would compile, but in the HII, you would see only the first `array size` options. For example `UINT8 OrderedListValue[2]` would result to:

![Orderedlistr5](Orderedlist5.png?raw=true "Orderedlist5")
