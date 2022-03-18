In last lesson we've used `checkbox` VFR element. Let's investigate other VFR elements that can be used for user input.

# Create a HII application that uses a custom stucture in `efivarstore`

Create `HIIFormDataElements` driver with one checkbox element based on our code from the `HIIFormCheckbox` driver.

Our UEFI variable in this lesson would contain data from different VFR elements on the form, therefore we need to put the `UINT8` storage for the checkbox inside the custom structure type.

Create a header file `UefiLessonsPkg/HIIFormDataElements/Data.h` with a typedef for our custom UEFI variable storage structure. Move variable name and GUID definitions (that in our case we decided would be the same as `FORMSET_GUID`) to this header as well:

`UefiLessonsPkg/HIIFormDataElements/Data.h`
```
#ifndef _DATA_H_
#define _DATA_H_

#define FORMSET_GUID  {0x531bc507, 0x9191, 0x4fa2, {0x94, 0x46, 0xb8, 0x44, 0xe3, 0x5d, 0xd1, 0x2a}}

#define UEFI_VARIABLE_STRUCTURE_NAME L"FormData"

#pragma pack(1)
typedef struct {
  UINT8 CheckboxValue;
} UEFI_VARIABLE_STRUCTURE;
#pragma pack()

#endif
```

Here are modifications that we need to make now to the `UefiLessonsPkg/HIIFormDataTemplate/HIIFormDataElements.c`:
```
...
#include "Data.h"			<--- add include

...

EFI_STRING      UEFIVariableName = UEFI_VARIABLE_STRUCTURE_NAME;   <--- add constant for the UEFI variable name


EFI_STATUS
EFIAPI
HIIFormDataElementsUnload (
  EFI_HANDLE ImageHandle
  )
{
  ...

  UEFI_VARIABLE_STRUCTURE EfiVarstore;		<--- use our custom storage instead of `UINT8`
  BufferSize = sizeof(UEFI_VARIABLE_STRUCTURE); <---

  Status = gRT->GetVariable(
                UEFIVariableName,				<--- use variable with UEFI variable name
                &mHiiVendorDevicePath.VendorDevicePath.Guid,
                NULL,
                &BufferSize,
                &EfiVarstore);
  if (!EFI_ERROR(Status)) {
    Status = gRT->SetVariable(
                  UEFIVariableName,                             <--- use variable with UEFI variable name
                  &mHiiVendorDevicePath.VendorDevicePath.Guid,
                  0,
                  0,
                  NULL);
  ...
}


EFI_STATUS
EFIAPI
HIIFormDataElementsEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  ...

  UEFI_VARIABLE_STRUCTURE EfiVarstore;        	<--- use our custom storage instead of `UINT8`
  BufferSize = sizeof(UEFI_VARIABLE_STRUCTURE); <---

  Status = gRT->GetVariable (
                UEFIVariableName,				<--- use variable with UEFI variable name
                &mHiiVendorDevicePath.VendorDevicePath.Guid,
                NULL,
                &BufferSize,
                &EfiVarstore);
  if (EFI_ERROR(Status)) {
    ZeroMem(&EfiVarstore, sizeof(EfiVarstore));
    Status = gRT->SetVariable(
                  UEFIVariableName,                             <--- use variable with UEFI variable name
                  &mHiiVendorDevicePath.VendorDevicePath.Guid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof(EfiVarstore),
                  &EfiVarstore);
  ...
}
```

And to the `UefiLessonsPkg/HIIFormDataElements/Form.vfr`
```
#include <Uefi/UefiMultiPhase.h>
#include "Data.h"                             <--- add include

formset
  guid     = FORMSET_GUID,
  title    = STRING_TOKEN(FORMSET_TITLE),
  help     = STRING_TOKEN(FORMSET_HELP),

  efivarstore UEFI_VARIABLE_STRUCTURE,                                          <--- use our custom type
    attribute = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
    name  = FormData,                                                           <--- new UEFI variable name
    guid  = FORMSET_GUID;

  form
    formid = 1,
    title = STRING_TOKEN(FORMID1_TITLE);

    checkbox
      varid = FormData.CheckboxValue,                                           <--- access as structure element
      prompt = STRING_TOKEN(CHECKBOX_PROMPT),
      help = STRING_TOKEN(CHECKBOX_HELP),
    endcheckbox;
endformset;
```

Build the driver and verify that everything works as it should.

# Numeric

Let's investigate `numeric` VFR element. With it user can enter a number value from a HII Form. https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.6.6.1-vfr-numeric-statement-definition

Our new element would need a storage, so add a field `UINT8 NumericValue` to our UEFI variable structure:
```
typedef struct {
  UINT8 CheckboxValue;
  UINT8 NumericValue;
} UEFI_VARIABLE_STRUCTURE;
```
Also add couple of strings to the `UefiLessonsPkg/HIIFormDataElements/Strings.uni` that we would need in our HII Form:
```
#string NUMERIC_PROMPT         #language en-US  "Numeric prompt"
#string NUMERIC_HELP           #language en-US  "Numeric help"
```

Minimal VFR code for the element would look like this:
```
numeric
  varid = FormData.NumericValue,
  prompt = STRING_TOKEN(NUMERIC_PROMPT),
  help = STRING_TOKEN(NUMERIC_HELP),
  minimum = 5,
  maximum = 20,
endnumeric;
```
`minimum` and `maximum` fields are mandatory for the `numeric` element. In this example we constrain our value to the range `5..20`.

As we've changed the size of our UEFI variable it is better to remove it before we load our driver:
```
FS0:\> dmpstore -guid 531bc507-9191-4fa2-9446-b844e35dd12a -d
FS0:\> load HIIFormDataElements.efi
```

On load our form would look like this:

![Numeric1](Numeric1.png?raw=true "Numeric1") 

As you see by default the value is equal to 0 (because we use `ZeroMem(&EfiVarstore, sizeof(EfiVarstore))`). This means that the value is out of its constrain range (5..20). The point is that HII form can't control initial value. The maximum and minimal limitation only for the user input.

If you try to enter value out of the range (for example `4`), form won't allow you:

![Numeric2](Numeric2.png?raw=true "Numeric2") 

Form engine even permit you from typing unallowed input. For example if you type `3` as a first symbol, form engine wouldn't permit you to type any other number symbol, as any value would be out of the available range.

But you can successfully enter and save (`F10`) value if it is in the allowed range.

![Numeric3](Numeric3.png?raw=true "Numeric3")

![Numeric4](Numeric4.png?raw=true "Numeric4")

With a help of the `dmpstore` command you can verify that the value was set successfully. In our case the value is in the second byte of the storage (`0x0f = 15`):
```
Shell> dmpstore -guid 531bc507-9191-4fa2-9446-b844e35dd12a
Variable NV+BS '531BC507-9191-4FA2-9446-B844E35DD12A:FormData' DataSize = 0x02
  00000000: 00 0F                                            *..*
```

# step

`numeric` VFR element can contain a `step` field. For example:
```
numeric
  varid = FormData.NumericValue,
  prompt = STRING_TOKEN(NUMERIC_PROMPT),
  help = STRING_TOKEN(NUMERIC_HELP),
  minimum = 5,
  maximum = 20,
  step = 2,
endnumeric;
```

If this field is present, in the bottom left corner of a form there would be an `Adjust Value` help message:
![Numeric5](Numeric5.png?raw=true "Numeric5")

This means that you can increase/decrease element value by typing `+`/`-`. And each time you type `+`/`-` the value would be increased/decreased by the value that we've set in our `step` field. The form engine even has some respect for the range field. For example if you have a value of `19` and enter `+`, the value would be set to `20`. The next `+` would change the value to `5`. And the next `+` to `7` and so on.

# IFR code

As usual let's investigate IFR code (`Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIFormDataElements/HIIFormDataElements/DEBUG/Form.lst`):
```
    numeric
>0000006C: 07 91 07 00 08 00 02 00 01 00 01 00 00 10 05 14 02
      varid = FormData.NumericValue,
      prompt = STRING_TOKEN(0x0007),
      help = STRING_TOKEN(0x0008),
      minimum = 5,
      maximum = 20,
      step = 2,
    endnumeric;
>0000007D: 29 02
```

To interpret it we need to consult UEFI specification:
```
EFI_IFR_NUMERIC

Summary:
Creates a number question.

Prototype:
#define EFI_IFR_NUMERIC_OP 0x07

typedef struct _EFI_IFR_NUMERIC {
 EFI_IFR_OP_HEADER   Header;
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
} EFI_IFR_NUMERIC;

Members:
Header 		The sequence that defines the type of opcode as well as the length of the opcode being defined. Header.OpCode = EFI_IFR_NUMERIC_OP.
Question 	The standard question header.
Flags 		Specifies flags related to the numeric question.
MinValue 	The minimum value to be accepted by the browser for this opcode. The size of the data field may vary from 8 to 64 bits.
MaxValue 	The maximum value to be accepted by the browser for this opcode. The size of the data field may vary from 8 to 64 bits.
Step 		Defines the amount to increment or decrement the value each time a user requests a value change. If the step value is 0, then
		the input mechanism for the numeric value is to be free-form and require the user to type in the actual value. The size of the
		data field may vary from 8 to 64 bits.
```

We've already investigated `EFI_IFR_OP_HEADER` and `EFI_IFR_QUESTION_HEADER` when we've talked about the `checkbox` element. If you split this data, all that is left is `10 05 14 02`.

This means that in our case the new fields are filled like this:
```
typedef struct _EFI_IFR_NUMERIC {
 EFI_IFR_OP_HEADER   Header;
 EFI_IFR_QUESTION_HEADER Question;
 UINT8 Flags;                        = 0x10
 struct {
    UINT8 MinValue;                  = 0x05
    UINT8 MaxValue;                  = 0x14
    UINT8 Step;                      = 0x02
 } u8;
} EFI_IFR_NUMERIC;
```

Everything is filled as we would expect except the `Flags` field which contains 0x10 value. What does it mean?

Special numeric flags are:
```
#define EFI_IFR_NUMERIC_SIZE           0x03
#define EFI_IFR_NUMERIC_SIZE_1         0x00
#define EFI_IFR_NUMERIC_SIZE_2         0x01
#define EFI_IFR_NUMERIC_SIZE_4         0x02
#define EFI_IFR_NUMERIC_SIZE_8         0x03

#define EFI_IFR_DISPLAY                0x30
#define EFI_IFR_DISPLAY_INT_DEC        0x00
#define EFI_IFR_DISPLAY_UINT_DEC       0x10
#define EFI_IFR_DISPLAY_UINT_HEX       0x20
```

So in our case our numeric has flags `EFI_IFR_DISPLAY_UINT_DEC` and `EFI_IFR_NUMERIC_SIZE_1`. Which means it is displayed as unsigned decimal and has a size of 1 byte.

# numeric flags

If you want to change the flag value you should use these keywords in VFR:
```
NUMERIC_SIZE_1
NUMERIC_SIZE_2
NUMERIC_SIZE_4
NUMERIC_SIZE_8
DISPLAY_INT_DEC
DISPLAY_UINT_DEC
DISPLAY_UINT_HEX
```

For example with `flags = DISPLAY_UINT_HEX` the value would be rendered as hex:
```
numeric
  varid = FormData.NumericValue,
  prompt = STRING_TOKEN(NUMERIC_PROMPT),
  help = STRING_TOKEN(NUMERIC_HELP),
  flags = DISPLAY_UINT_HEX,
  minimum = 5,
  maximum = 200,
  step = 2,
endnumeric;
```

![Numeric6](Numeric6.png?raw=true "Numeric6")


If you want to add a possibility to set negative integers, add `DISPLAY_INT_DEC` flag:
```
numeric
  varid = FormData.NumericValue,
  prompt = STRING_TOKEN(NUMERIC_PROMPT),
  help = STRING_TOKEN(NUMERIC_HELP),
  flags = DISPLAY_INT_DEC,
  minimum = -5,
  maximum = 20,
  step = 2,
endnumeric;
```

![Numeric7](Numeric7.png?raw=true "Numeric7")


As for the size flag by default it is set automatically based on the size of a our storage variable.

For example if you change the type of the numeric field to `UINT32`:
```
typedef struct {
  UINT8 CheckboxValue;
  UINT32 NumericValue;
} UEFI_VARIABLE_STRUCTURE;
```

This VFR code
```
numeric
  varid = FormData.NumericValue,
  prompt = STRING_TOKEN(NUMERIC_PROMPT),
  help = STRING_TOKEN(NUMERIC_HELP),
  minimum = 0x11223344,
  maximum = 0xAABBCCDD,
  step = 2,
endnumeric;
```

Would produce the following IFR result:
```
    numeric
>0000006C: 07 9A 07 00 08 00 02 00 01 00 01 00 00 12 44 33 22 11 DD CC BB AA 02 00 00 00
      varid = FormData.NumericValue,
      prompt = STRING_TOKEN(0x0007),
      help = STRING_TOKEN(0x0008),
      minimum = 0x11223344,
      maximum = 0xaabbccdd,
      step = 2,
    endnumeric;
>00000086: 29 02
```

If you parse the data you would get:
```
flags   = 0x12       = EFI_IFR_DISPLAY_UINT_DEC | EFI_IFR_NUMERIC_SIZE_4
minimum = 0x11223344
maximum = 0xaabbccdd
step    = 0x00000002
```

So the size flag has changed from the `EFI_IFR_NUMERIC_SIZE_1` to `EFI_IFR_NUMERIC_SIZE_4`.

Build system checks size of the storage in the build process. For example if you revert now it to `UINT8`, but leave `UINT32` values for min/max/step, your build would fail:
```
ERROR 12288: Overflow: Value 0x11223344 is too large to store in a UINT8
```

Also you can't enter a size flag that conflicts with a storage size. In this case you would get:
```
ERROR 12288: Numeric Flag is not same to Numeric VarData type
```

In case of a `efivarstore` the size flag is really a redundancy. It only matters when other types of storages are used. But you can put it in case you want to explicitly show the size of the storage in the VFR.

Also don't forget to delete UEFI variable every time you change UEFI variable size. You should do it before the updated driver load:
```
FS0:\> dmpstore -guid 531bc507-9191-4fa2-9446-b844e35dd12a -d
```

Keep in mind that you can combine several flags with a help of a `|` operation:
```
flags = NUMERIC_SIZE_8 | DISPLAY_UINT_HEX,
```
