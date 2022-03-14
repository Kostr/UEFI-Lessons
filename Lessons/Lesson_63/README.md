Now let's try to actually save some user input with a form.

For the data we will choose the most simple element - checkbox (https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.6.5.1-vfr-checkbox-statement-definition)

# Starting point

Let's create an application `HIIFormCheckbox` which is based on the code of our `HIIStaticFormDriver` application:

`UefiLessonsPkg/HIIFormCheckbox/HIIFormCheckbox.inf`:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = HIIFormCheckbox
  FILE_GUID                      = 771a4631-43ba-4852-9593-919d9de079f1
  MODULE_TYPE                    = UEFI_DRIVER
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = HIIFormCheckboxEntryPoint
  UNLOAD_IMAGE                   = HIIFormCheckboxUnload

[Sources]
  HIIStaticFormDriver.c
  Strings.uni
  Form.vfr

[Packages]
  MdePkg/MdePkg.dec
  MdeModulePkg/MdeModulePkg.dec

[LibraryClasses]
  UefiDriverEntryPoint
  UefiLib
  HiiLib
```

`UefiLessonsPkg/HIIFormCheckbox/HIIFormCheckbox.c`:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/HiiLib.h>


extern UINT8 FormBin[];

EFI_HII_HANDLE  mHiiHandle = NULL;


EFI_STATUS
EFIAPI
HIIFormCheckboxUnload (
  EFI_HANDLE ImageHandle
  )
{
  if (mHiiHandle != NULL)
    HiiRemovePackages(mHiiHandle);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HIIFormCheckboxEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  mHiiHandle = HiiAddPackages(
                 &gEfiCallerIdGuid,
                 NULL,
                 HIIFormCheckboxStrings,
                 FormBin,
                 NULL
                 );
  if (mHiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}
```

`UefiLessonsPkg/HIIFormCheckbox/Strings.uni`
```
#langdef en-US "English"

#string FORMSET_TITLE          #language en-US  "Simple Formset"
#string FORMSET_HELP           #language en-US  "This is a very simple formset"
#string FORMID1_TITLE          #language en-US  "Simple Form"
```

`UefiLessonsPkg/HIIFormCheckbox/Form.vfr`
```
#include <Uefi/UefiMultiPhase.h>

#define FORMSET_GUID  {0xef2acc91, 0x7b50, 0x4ab9, {0xab, 0x67, 0x2b, 0x4, 0xf8, 0xbc, 0x13, 0x5e}}

formset
  guid     = FORMSET_GUID,
  title    = STRING_TOKEN(FORMSET_TITLE),
  help     = STRING_TOKEN(FORMSET_HELP),

  form
    formid = 1,
    title = STRING_TOKEN(FORMID1_TITLE);
  endform;
endformset;
```

Now if you build our driver you can load it with the `load HIIFormCheckbox` command.
```
FS0:\> load HIIFormCheckbox.efi
Image 'FS0:\HIIFormCheckbox.efi' loaded at 688B000 - Success
```

Verify that HII resources were created with a help of our `ShowHII.efi` application:
```
FS0:\> ShowHII.efi
...
PackageList[20]: GUID=771A4631-43BA-4852-9593-919D9DE079F1; size=0x114
        Package[0]: type=FORMS; size=0x41
        Package[1]: type=STRINGS; size=0xBB
        Package[2]: type=END; size=0x4
```

And if you use our `DisplayHIIByGuid.efi` application, you could see the form:
```
FS0:\> DisplayHIIByGuid.efi 771A4631-43BA-4852-9593-919D9DE079F1
```

![Start](Start.png?raw=true "Starting Form")

# Add checkbox element

Now let's start to add new functionality.

Insert the following code inside the form:
```
checkbox
  prompt = STRING_TOKEN(CHECKBOX_PROMPT),
  help = STRING_TOKEN(CHECKBOX_HELP),
endcheckbox;
```

And define the strings in our UNI file:
```
#string CHECKBOX_PROMPT        #language en-US  "Checkbox prompt"
#string CHECKBOX_HELP          #language en-US  "Checkbox help"
```

This will give you the following element on the form

![Checkbox1](Checkbox1.png?raw=true "Checkbox1")

Which you can toggle with a spacebar

![Checkbox2](Checkbox2.png?raw=true "Checkbox2")

But unfortunately our Form doesn't save any data. No matter in what state you will leave the checkbox on the form exit, once you launch the form again, the checkbox will be cleared. So right now our form is useless.

# IFR code

Let's investigate IFR code from our new element (`Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIFormCheckbox/HIIFormCheckbox/DEBUG/Form.lst`).

The checkbox element will produce `EFI_IFR_CHECKBOX` and `EFI_IFR_END` opcodes:
```
    checkbox
>00000039: 06 8E 05 00 06 00 01 00 00 00 FF FF 00 00
      prompt = STRING_TOKEN(0x0005),
      help = STRING_TOKEN(0x0006),
    endcheckbox;
>00000047: 29 02
```

Here is a definition for the `EFI_IFR_CHECKBOX`:
```
EFI_IFR_CHECKBOX

Summary:
Creates a boolean checkbox.

Prototype:

#define EFI_IFR_CHECKBOX_OP 0x06

typedef struct _EFI_IFR_CHECKBOX {
 EFI_IFR_OP_HEADER Header;
 EFI_IFR_QUESTION_HEADER Question;
 UINT8 Flags;
} EFI_IFR_CHECKBOX;

Members:
Header 		The standard question header, where Header.OpCode = EFI_IFR_CHECKBOX_OP.
Question 	The standard question header.
Flags 		Flags that describe the behavior of the question. All undefined bits should be zero.

Description:
Creates a Boolean checkbox question and adds it to the current form. The checkbox has two values:
FALSE if the box is not checked and TRUE if it is. 
```

Just in case you've forgot, `EFI_IFR_OP_HEADER` takes 2 bytes, so it would take `06 8E` from the `06 8E 05 00 06 00 01 00 00 00 FF FF 00 00`:
```
typedef struct _EFI_IFR_OP_HEADER {
 UINT8 OpCode;
 UINT8 Length:7;
 UINT8 Scope:1;
} EFI_IFR_OP_HEADER;
```

Now let's start checking the `EFI_IFR_QUESTION_HEADER` field which is represented by the rest `05 00 06 00 01 00 00 00 FF FF 00 00` data:
```
EFI_IFR_QUESTION_HEADER

Summary:
Standard question header.

Prototype:

typedef struct _EFI_IFR_QUESTION_HEADER {
 EFI_IFR_STATEMENT_HEADER Header;
 EFI_QUESTION_ID QuestionId;
 EFI_VARSTORE_ID VarStoreId;
 union {
  EFI_STRING_ID VarName;
  UINT16 VarOffset;
 } VarStoreInfo;
 UINT8 Flags;
} EFI_IFR_QUESTION_HEADER;

Members:
Header 		The standard statement header.
QuestionId 	The unique value that identifies the particular question being defined by the opcode. The value of zero is reserved.
Flags 		A bit-mask that determines which unique settings are active for this question.
VarStoreId 	Specifies the identifier of a previously declared variable store to use when storing the question’s value.
		A value of zero indicates no associated variable store.
VarStoreInfo 	If VarStoreId refers to Buffer Storage (EFI_IFR_VARSTORE or EFI_IFR_VARSTORE_EFI), then VarStoreInfo contains a 16-bit Buffer Storage offset (VarOffset).
		If VarStoreId refers to Name/Value Storage (EFI_IFR_VARSTORE_NAME_VALUE), then VarStoreInfo contains the String ID of the name (VarName) for this name/value pair.

Description:
This is the standard header for questions.
```

First the `EFI_IFR_STATEMENT_HEADER`:
```
EFI_IFR_STATEMENT_HEADER

Summary:
Standard statement header.

Prototype:
typedef struct _EFI_IFR_STATEMENT_HEADER {
 EFI_STRING_ID Prompt;
 EFI_STRING_ID Help;
} EFI_IFR_STATEMENT_HEADER;

Members:
Prompt 	The string identifier of the prompt string for this particular statement. The value 0 indicates no prompt string.
Help 	The string identifier of the help string for this particular statement. The value 0 indicates no help string.

Description:
This is the standard header for statements, including questions
```
According to the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiInternalFormRepresentation.h:
```
typedef UINT16  EFI_STRING_ID;
```
So basically it means this (remember we've left with `05 00 06 00 01 00 00 00 FF FF 00 00` data):
```
EFI_STRING_ID Prompt = 00 05
EFI_STRING_ID Help = 00 06
```
This is just references to the strings in the HII String package.

Now we've left with a `01 00 00 00 FF FF 00 00` code.

According to the same `UefiInternalFormRepresentation.h` file:
```
typedef UINT16  EFI_QUESTION_ID;
typedef UINT16  EFI_VARSTORE_ID;
```

So if we continue to match data to fields we would get:
```
EFI_QUESTION_ID QuestionId;   = 00 01
EFI_VARSTORE_ID VarStoreId;   = 00 00
union {
 EFI_STRING_ID VarName;       = FF FF
 UINT16 VarOffset;
} VarStoreInfo;
UINT8 Flags;                  = 00
```

And the last `00` byte that is left (in case you track it) a `UINT8 Flags` field from the end of the `EFI_IFR_CHECKBOX` structure.

You might guess already the source of our problem. To save something we need to create a link to some variable storage. Because right now `VarStoreId = 0`, which according to the docs above means `no associated variable store`.

There are several possible variable storages. You can even even create you callback logic on form submit and don't use any variable storage at all. 
We need to start with something, so in this lesson we are going to look at most simple storage `efivarstore`.

# Add `efivarstore` to VFR

`efivarstore` is a simple UEFI variable storage. Basically you supply `GUID+name` combination for the UEFI variable and its type and flags (https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/27_vfr_variable_store_definition#2.7.2-vfr-efi-variable-store-definition):

For example if you want to represent UEFI variable with your special `MyCustomStructure` type which exist under `MyVariableGuid` with a name `MyVariableName` in the system and has `EFI_VARIABLE_BOOTSERVICE_ACCESS` flag, you should declare it like this:
```
efivarstore MyCustomStructure
  attribute = EFI_VARIABLE_BOOTSERVICE_ACCESS,
  name      = MyVariableName,
  guid      = MyVariableGuid;
```

We don't need any special structure, simple `UINT8` will do for the checkbox.

Let's declare our variable `EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE` and place it under the `FORMSET_GUID` with a name `CheckboxValue`.

To link checkbox value with this variable we can use `varid = CheckboxValue` syntax 
```
formset
  guid     = FORMSET_GUID,
  title    = STRING_TOKEN(FORMSET_TITLE),
  help     = STRING_TOKEN(FORMSET_HELP),

+ efivarstore UINT8,
+   attribute = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
+   name  = CheckboxValue,
+   guid  = FORMSET_GUID;

  form
    formid = 1,
    title = STRING_TOKEN(FORMID1_TITLE);
    checkbox
+     varid = CheckboxValue,
      prompt = STRING_TOKEN(CHECKBOX_PROMPT),
      help = STRING_TOKEN(CHECKBOX_HELP),
    endcheckbox;
  endform;
endformset;
```

# IFR changes

Let's investigate how this new VFR code have changed our IFR sructures.

First we've added `efivarstore`, it have produced the following code:
```
  efivarstore UINT8,
>00000033: 26 28 01 00 91 CC 2A EF 50 7B B9 4A AB 67 2B 04 F8 BC 13 5E 03 00 00 00 01 00 43 68 65 63 6B 62 6F 78 56 61 6C 75 65 00
    attribute = 0x00000002 | 0x00000001,
    name = CheckboxValue,
    guid = {0xef2acc91, 0x7b50, 0x4ab9, {0xab, 0x67, 0x2b, 0x4, 0xf8, 0xbc, 0x13, 0x5e}};
```

It starts with an opcode 0x26 which is:
```
#define EFI_IFR_VARSTORE_EFI_OP         0x26
```
So the first 2 bytes is the `EFI_IFR_OP_HEADER`.

The rest of data is the `EFI_IFR_VARSTORE_EFI` structure:
```
EFI_IFR_VARSTORE_EFI

Summary:
Creates a variable storage short-cut for EFI variable storage.

Prototype:
#define EFI_IFR_VARSTORE_EFI_OP 0x26

typedef struct _EFI_IFR_VARSTORE_EFI {
 EFI_IFR_OP_HEADER Header;
 EFI_VARSTORE_ID VarStoreId;
 EFI_GUID Guid;
 UINT32 Attributes
 UINT16 Size;
 //UINT8 Name[];
} EFI_IFR_VARSTORE_EFI;

Members:
Header 		The byte sequence that defines the type of opcode as well as the length of the opcode being defined. For this tag,
		Header.OpCode = EFI_IFR_VARSTORE_EFI_OP.
VarStoreId 	The variable store identifier, which is unique within the current form set. This field is the value that uniquely identifies
		this variable store definition instance from others. Question headers refer to this value to designate which is the active
		variable that is being used. A value of zero is invalid.
Guid 		The EFI variable’s GUID definition. This field comprises one half of the EFI variable name, with the other half being the
		human-readable aspect of the name, which is specified in the Name field below.
Attributes	Specifies the flags to use for the variable.
Size		The size of the variable store.
Name		A null-terminated ASCII string that specifies one half of the EFI name for this variable store. The other half is specified in
		the Guid field (above). The Name field is not actually included in the structure but is included here to help illustrate the
		encoding of the opcode. The size of the string, including the null termination, is included in the opcode's header size.

Description:
This opcode describes an EFI Variable Variable Store within a form set. The Guid and Name specified here will be used with GetVariable() and SetVariable().
```

Let's look how binary data fills structure fields:
```
EFI_VARSTORE_ID VarStoreId    // 01 00
EFI_GUID Guid;                // 91 CC 2A EF 50 7B B9 4A AB 67 2B 04 F8 BC 13 5E (= FORMSET_GUID) 
UINT32 Attributes             // 03 00 00 00
UINT16 Size;                  // 01 00
//UINT8 Name[];               // 43 68 65 63 6B 62 6F 78 56 61 6C 75 65 00  (= "C h e c k b o x V a l u e") 
```

As you see the code provides `GUID+name` combination for the UEFI variable and its type and flags like it should.

Now let's look at the reference to this storage. We've added `varid = CheckboxValue` to our form:
```
>00000061: 06 8E 05 00 06 00 01 00 01 00 00 00 00 00
      varid = CheckboxValue,
      prompt = STRING_TOKEN(0x0005),
      help = STRING_TOKEN(0x0006),
    endcheckbox;
```

Basically the data has changed like this:
```
- 06 8E 05 00 06 00 01 00 00 00 FF FF 00 00
+ 06 8E 05 00 06 00 01 00 01 00 00 00 00 00
```

Which means this:
```
                                  -          +                              
EFI_VARSTORE_ID VarStoreId;   = 00 00	// 01 00
union {
 EFI_STRING_ID VarName;       = FF FF	// 00 00
 UINT16 VarOffset;
} VarStoreInfo;
```

So now checkbox code points to the varstore with an id = 1 (which is the id of the storage that we've declared above).

# HII checkbox form with varstore

If you look at the form from our application again, you'll see, that now it has several new help messages in the bottom panel, particularly:

- `F9=Reset to Defaults`

- `F10=Save`

![CheckboxWithVarstore1](CheckboxWithVarstore1.png?raw=true "CheckboxWithVarstore1")

Now when you change configuration there are would be `Configuration changed` message:

![CheckboxWithVarstore2](CheckboxWithVarstore2.png?raw=true "CheckboxWithVarstore2")

To save the settings you can use `F10` like it is described in the bottom panel. It would produce the following confirmation window:

![CheckboxWithVarstore3](CheckboxWithVarstore3.png?raw=true "CheckboxWithVarstore3")

But if you try to press `Y` the form will fail with the following message:

![CheckboxWithVarstore4](CheckboxWithVarstore4.png?raw=true "CheckboxWithVarstore4")

To make things work we need to add some modifications to our driver code.
