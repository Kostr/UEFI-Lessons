Now let's try to actually save some user input with a form.

For the data we will choose the most simple element - checkbox (https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.6.5.1-vfr-checkbox-statement-definition)

Crete new application with a form and insert the following code inside:
```
checkbox
  prompt = STRING_TOKEN(CHECKBOX_TITLE),
  help = STRING_TOKEN(CHECKBOX_HELP),
endcheckbox;
```

This will give you the following element on form

![Checkbox1](Checkbox1.png?raw=true "Checkbox1")

Which you can toggle with a spacebar

![Checkbox2](Checkbox2.png?raw=true "Checkbox2")

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
And for its field `EFI_IFR_QUESTION_HEADER`:
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

Description
This is the standard header for questions.
```

# Creating `efivarstore`



FS0:\> dmpstore -all
```
Variable NV+BS 'EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E:HIIFormCheckboxEfiVarstore' DataSize = 0x01
  00000000: 01                                               *.*
```

FS0:\> dmpstore -guid EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E
Variable NV+BS 'EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E:HIIFormCheckboxEfiVarstore' DataSize = 0x01
  00000000: 01                                               *.*
