In this lesson we are going to look how to embed content in HII forms dynamically.

For example it can help to display things like:
- current possible boot sources
- currently present PCI devices
- currently present memory
- ...

All these things are determined in the UEFI boot process, and can differ from boot to boot depending on the current hardware configuration. Therefore if you want to add such information to the HII Forms you need to do it dynamically.

For this operation EDKII uses labels mechanics.

But first let's create `HIIFormLabel` application with an empty form. For simplicity we wouldn't use any storage in this example (as we would embed simple elements that don't use storage at all).

# Initial code

Here is code for the driver that adds an empty form.

`UefiLessonsPkg/HIIFormLabel/HIIFormLabel.inf`:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = HIIFormLabel
  FILE_GUID                      = a869c42c-fd49-469d-b6ab-b37569c0e90d
  MODULE_TYPE                    = UEFI_DRIVER
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = HIIFormLabelEntryPoint
  UNLOAD_IMAGE                   = HIIFormLabelUnload

[Sources]
  HIIFormLabel.c
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

`UefiLessonsPkg/HIIFormLabel/HIIFormLabel.c`:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/HiiLib.h>
#include "Data.h"

#include <Guid/MdeModuleHii.h>

extern UINT8 FormBin[];

EFI_HII_HANDLE  mHiiHandle = NULL;


EFI_STATUS
EFIAPI
HIIFormLabelUnload (
  EFI_HANDLE ImageHandle
  )
{
  if (mHiiHandle != NULL)
    HiiRemovePackages(mHiiHandle);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HIIFormLabelEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  mHiiHandle = HiiAddPackages(
                 &gEfiCallerIdGuid,
                 NULL,
                 HIIFormLabelStrings,
                 FormBin,
                 NULL
                 );
  if (mHiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
```

`UefiLessonsPkg/HIIFormLabel/Data.h`:
```
#ifndef _DATA_H_
#define _DATA_H_

#define FORMSET_GUID  {0x29caf8e0, 0x2788, 0x4e1c, {0xb6, 0x15, 0x5b, 0xf8, 0x2f, 0x06, 0x7a, 0xa5}}

#endif
```

`UefiLessonsPkg/HIIFormLabel/Form.vfr`:
```
#include "Data.h"

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

`UefiLessonsPkg/HIIFormLabel/Strings.uni`:
```
#langdef en-US "English"

#string FORMSET_TITLE          #language en-US  "Simple Formset"
#string FORMSET_HELP           #language en-US  "This is a very simple formset"
#string FORMID1_TITLE          #language en-US  "Simple Form"
```

All this code would give us this form:

![Before](Before.png?raw=true "Before")

# Add label

Now let's try add content dynamically to our form.

Add 2 `label` elements to our form in `UefiLessonsPkg/HIIFormDataElements/Form.vfr`:
```
formset
  guid     = FORMSET_GUID,
  title    = STRING_TOKEN(FORMSET_TITLE),
  help     = STRING_TOKEN(FORMSET_HELP),
  form
    formid = 1,
    title = STRING_TOKEN(FORMID1_TITLE);

    label LABEL_START;
    label LABEL_END;
  endform;
endformset;
```

These two elements define a start and end of the section for the dynamic content.

Values under `LABEL_START` and `LABEL_END` are just `UINT16` numbers. Place some defines for them to the `UefiLessonsPkg/HIIFormDataElements/Data.h`:
```
#define LABEL_START 0x1111
#define LABEL_END   0x2222
```
The actual label numbers are not important. The only limitation is that label numers must be unique in a scope of a target form.

# IFR

Build module and look at the IFR `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIFormDataElements/HIIFormDataElements/DEBUG/Form.lst`:
```
    label 0x1111;
>00000117: 5F 15 35 17 0B 0F A0 87 93 41 B2 66 53 8C 38 AF 48 CE 00 11 11
    label 0x2222;
>0000012C: 5F 15 35 17 0B 0F A0 87 93 41 B2 66 53 8C 38 AF 48 CE 00 22 22
```

Labels are encoded with a help of `EFI_IFR_GUID_OP` opcode. This opcode helps vendors to add new IFR opcodes that are not defined in the UEFI specification. This is a method for UEFI specification extension.
```
EFI_IFR_GUID

Summary:
A GUIDed operation. This op-code serves as an extensible op-code which can be defined by the Guid value to have various functionality. It should be noted that IFR browsers or scripts which cannot interpret the meaning of this GUIDed op-code will skip it.

Prototype:

#define EFI_IFR_GUID_OP 0x5F

typedef struct _EFI_IFR_GUID {
 EFI_IFR_OP_HEADER Header;
 EFI_GUID Guid;
//Optional Data Follows
} EFI_IFR_GUID;

Parameters:
Header	The sequence that defines the type of opcode as well as the length of the opcode being defined.
	For this tag, Header.OpCode = EFI_IFR_GUID_OP
Guid 	The GUID value for this op-code. This field is intended to define a particular type of special-purpose function,
	and the format of the data which immediately follows the Guid field (if any) is defined by that particular GUID.
```

If you parse our binary data you could see that in our case `Guid` is equal to:
```
35 17 0B 0F A0 87 93 41 B2 66 53 8C 38 AF 48 CE
```
This value defines a format of all the rest of the opcode data that follows next until the opcode end (which is determined by `Header.Length`). In this case it is `00 11 11` or `00 22 22`.


The GUID in the IFR is defined in the `./MdeModulePkg/MdeModulePkg.dec`:
```
gEfiIfrTianoGuid      = { 0xf0b1735, 0x87a0, 0x4193, {0xb2, 0x66, 0x53, 0x8c, 0x38, 0xaf, 0x48, 0xce }}
```
Or in the `./MdeModulePkg/Include/Guid/MdeModuleHii.h`:
```
#define EFI_IFR_TIANO_GUID \
  { 0xf0b1735, 0x87a0, 0x4193, {0xb2, 0x66, 0x53, 0x8c, 0x38, 0xaf, 0x48, 0xce} }
```

EDKII has several special IFR opcodes. All of them are defined under the same `EFI_IFR_TIANO_GUID`. To differ them between each other the first byte after the GUID defines an opcode:
```
///
/// EDKII implementation extension opcodes, new extension can be added here later.
///
#define EFI_IFR_EXTEND_OP_LABEL       0x0
#define EFI_IFR_EXTEND_OP_BANNER      0x1
#define EFI_IFR_EXTEND_OP_TIMEOUT     0x2
#define EFI_IFR_EXTEND_OP_CLASS       0x3
#define EFI_IFR_EXTEND_OP_SUBCLASS    0x4
```

In our case the next byte is `0x00` which corresponds to the `EFI_IFR_EXTEND_OP_LABEL`.

In this case the full opcode is defined with this structure:
```
///
/// Label opcode.
///
typedef struct _EFI_IFR_GUID_LABEL {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// EFI_IFR_TIANO_GUID.
  ///
  EFI_GUID            Guid;			// EFI_IFR_TIANO_GUID
  ///
  /// EFI_IFR_EXTEND_OP_LABEL.
  ///
  UINT8               ExtendOpCode;		// EFI_IFR_EXTEND_OP_LABEL = 0x0
  ///
  /// Label Number.
  ///
  UINT16              Number;
} EFI_IFR_GUID_LABEL;
```

# C code

To insert content from the C code into section defined by labels we need to utilize `HiiUpdateForm` function from the `HiiLib`


https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Library/HiiLib.h
```
/**
  This function updates a form that has previously been registered with the HII
  Database.  This function will perform at most one update operation.
  The form to update is specified by Handle, FormSetGuid, and FormId.  Binary
  comparisons of IFR opcodes are performed from the beginning of the form being
  updated until an IFR opcode is found that exactly matches the first IFR opcode
  specified by StartOpCodeHandle.  The following rules are used to determine if
  an insert, replace, or delete operation is performed:
  1) If no matches are found, then NULL is returned.
  2) If a match is found, and EndOpCodeHandle is NULL, then all of the IFR opcodes
     from StartOpCodeHandle except the first opcode are inserted immediately after
     the matching IFR opcode in the form to be updated.
  3) If a match is found, and EndOpCodeHandle is not NULL, then a search is made
     from the matching IFR opcode until an IFR opcode exactly matches the first
     IFR opcode specified by EndOpCodeHandle.  If no match is found for the first
     IFR opcode specified by EndOpCodeHandle, then NULL is returned.  If a match
     is found, then all of the IFR opcodes between the start match and the end
     match are deleted from the form being updated and all of the IFR opcodes
     from StartOpCodeHandle except the first opcode are inserted immediately after
     the matching start IFR opcode.  If StartOpCcodeHandle only contains one
     IFR instruction, then the result of this operation will delete all of the IFR
     opcodes between the start end matches.
  If HiiHandle is NULL, then ASSERT().
  If StartOpCodeHandle is NULL, then ASSERT().
  @param[in]  HiiHandle          The HII Handle of the form to update.
  @param[in]  FormSetGuid        The Formset GUID of the form to update.  This
                                 is an optional parameter that may be NULL.
                                 If it is NULL, all FormSet will be updated.
  @param[in]  FormId             The ID of the form to update.
  @param[in]  StartOpCodeHandle  An OpCode Handle that contains the set of IFR
                                 opcodes to be inserted or replaced in the form.
                                 The first IFR instruction in StartOpCodeHandle
                                 is used to find matching IFR opcode in the
                                 form.
  @param[in]  EndOpCodeHandle    An OpCcode Handle that contains the IFR opcode
                                 that marks the end of a replace operation in
                                 the form.  This is an optional parameter that
                                 may be NULL.  If it is NULL, then the IFR
                                 opcodes specified by StartOpCodeHandle are
                                 inserted into the form.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory resources are allocated.
  @retval EFI_NOT_FOUND          The following cases will return EFI_NOT_FOUND:
                                 1) The form specified by HiiHandle, FormSetGuid,
                                 and FormId could not be found in the HII Database.
                                 2) No IFR opcodes in the target form match the first
                                 IFR opcode in StartOpCodeHandle.
                                 3) EndOpCOde is not NULL, and no IFR opcodes in the
                                 target form following a matching start opcode match
                                 the first IFR opcode in EndOpCodeHandle.
  @retval EFI_SUCCESS            The matched form is updated by StartOpcode.
**/
EFI_STATUS
EFIAPI
HiiUpdateForm (
  IN EFI_HII_HANDLE  HiiHandle,
  IN EFI_GUID        *FormSetGuid         OPTIONAL,
  IN EFI_FORM_ID     FormId,
  IN VOID            *StartOpCodeHandle,
  IN VOID            *EndOpCodeHandle     OPTIONAL
  );
```

As you can see this function needs two opcode handles:
- `StartOpCodeHandle` - handle that contains IFR that marks a start of a replace section + all the opcodes that are needed to be inserted,
- `EndOpCodeHandle` - handle that contains IFR that marks an end of a replace section


How to create these OpCode Handles? `HiiLib` has a special function for that:
```
/**
  Allocates and returns a new OpCode Handle.  OpCode Handles must be freed with
  HiiFreeOpCodeHandle().
  @retval NULL   There are not enough resources to allocate a new OpCode Handle.
  @retval Other  A new OpCode handle.
**/
VOID *
EFIAPI
HiiAllocateOpCodeHandle (
  VOID
  );
```

The important thing to note here, is that this function doesn't accept any size. Because in this operation the buffer is always fixed. `HiiAllocateOpCodeHandle` returns a pointer to the newly allocated structure `HII_LIB_OPCODE_BUFFER` that is defined as follow:
```
#define HII_LIB_OPCODE_ALLOCATION_SIZE  0x200	// (=512)

typedef struct {
  UINT8    *Buffer;		// Pointer to newly allocated buffer of HII_LIB_OPCODE_ALLOCATION_SIZE bytes
  UINTN    BufferSize;
  UINTN    Position;
} HII_LIB_OPCODE_BUFFER;
```

All our dynamically allocated opcodes should fit into the preallocated buffer.

Off course we need to free all the buffers allocated with `HiiAllocateOpCodeHandle`:

```
/**
  Frees an OpCode Handle that was previously allocated with HiiAllocateOpCodeHandle().
  When an OpCode Handle is freed, all of the opcodes associated with the OpCode
  Handle are also freed.
  If OpCodeHandle is NULL, then ASSERT().
  @param[in]  OpCodeHandle   The handle to the buffer of opcodes.
**/
VOID
EFIAPI
HiiFreeOpCodeHandle (
  VOID  *OpCodeHandle
  );
```

So in our driver we should add this code:
```
VOID* StartOpCodeHandle = HiiAllocateOpCodeHandle();
VOID* EndOpCodeHandle = HiiAllocateOpCodeHandle();

<...>  // fill OpCode Handles


EFI_GUID formsetGuid = FORMSET_GUID;
EFI_STATUS Status = HiiUpdateForm(
                      mHiiHandle,
                      &formsetGuid,
                      0x1,
                      StartOpCodeHandle,
                      EndOpCodeHandle
                    );
if (EFI_ERROR(Status)) {
  Print(L"Error! HiiUpdateForm returned = %r\n", Status);
}

HiiFreeOpCodeHandle(StartOpCodeHandle);
HiiFreeOpCodeHandle(EndOpCodeHandle);
return Status;
```

Now we need to create correct data under `StartOpCodeHandle` and `EndOpCodeHandle`. First we need to add `label` IFR to them. For this task we can utilize `HiiCreateGuidOpCode` function:
```
/**
  Create EFI_IFR_GUID opcode.
  If OpCodeHandle is NULL, then ASSERT().
  If Guid is NULL, then ASSERT().
  If OpCodeSize < sizeof (EFI_IFR_GUID), then ASSERT().
  @param[in]  OpCodeHandle  Handle to the buffer of opcodes.
  @param[in]  Guid          Pointer to EFI_GUID of this guided opcode.
  @param[in]  GuidOpCode    Pointer to an EFI_IFR_GUID opcode.  This is an
                            optional parameter that may be NULL.  If this
                            parameter is NULL, then the GUID extension
                            region of the created opcode is filled with zeros.
                            If this parameter is not NULL, then the GUID
                            extension region of GuidData will be copied to
                            the GUID extension region of the created opcode.
  @param[in]  OpCodeSize    The size, in bytes, of created opcode.  This value
                            must be >= sizeof(EFI_IFR_GUID).
  @retval NULL   There is not enough space left in Buffer to add the opcode.
  @retval Other  A pointer to the created opcode.
**/
UINT8 *
EFIAPI
HiiCreateGuidOpCode (
  IN VOID            *OpCodeHandle,
  IN CONST EFI_GUID  *Guid,
  IN CONST VOID      *GuidOpCode     OPTIONAL,
  IN UINTN           OpCodeSize
  )
```

This function returns a pointer to the opcode buffer. In our case it is a buffer for the `EFI_IFR_GUID_LABEL` opcode. So we need to cast the buffer to the `EFI_IFR_GUID_LABEL` structure and fill it with correct data. If `HiiCreateGuidOpCode` returns `NULL` we need to report an error and free all the previously allocated resources:
```
EFI_IFR_GUID_LABEL* StartLabel = (EFI_IFR_GUID_LABEL*) HiiCreateGuidOpCode(StartOpCodeHandle,
                                                                           &gEfiIfrTianoGuid,
                                                                           NULL,
                                                                           sizeof(EFI_IFR_GUID_LABEL)
                                                                           );
if (StartLabel == NULL) {
  Print(L"Error! Can't create StartLabel opcode, not enough space\n");
  HiiRemovePackages(mHiiHandle);
  return EFI_BUFFER_TOO_SMALL;
}
StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
StartLabel->Number = LABEL_START;


EFI_IFR_GUID_LABEL* EndLabel = (EFI_IFR_GUID_LABEL*) HiiCreateGuidOpCode(EndOpCodeHandle,
                                                                         &gEfiIfrTianoGuid,
                                                                         NULL,
                                                                         sizeof(EFI_IFR_GUID_LABEL)
                                                                         );
if (EndLabel == NULL) {
  Print(L"Error! Can't create Text opcode, not enough space\n");
  HiiFreeOpCodeHandle(StartOpCodeHandle);
  HiiRemovePackages(mHiiHandle);
  return EFI_BUFFER_TOO_SMALL;
}
EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
EndLabel->Number = LABEL_END;
```
As we are using `gEfiIfrTianoGuid` here, don't forget to add `#include <Guid/MdeModuleHii.h>` to our code and add proper `Guids` section to the INF file:
```
[Guids]
  gEfiIfrTianoGuid
```

Our final task is to embed new code to the IFR. So besides the markers (=labels) we need to add actually new IFR codes to the buffer refernced by the `StartOpCodeHandle`. Let's add a VFR `text` element with a help of a `HiiCreateTextOpCode` function:
```
/**
  Create EFI_IFR_TEXT_OP opcode.
  If OpCodeHandle is NULL, then ASSERT().
  @param[in]  OpCodeHandle  Handle to the buffer of opcodes.
  @param[in]  Prompt        String ID for Prompt.
  @param[in]  Help          String ID for Help.
  @param[in]  TextTwo       String ID for TextTwo.
  @retval NULL   There is not enough space left in Buffer to add the opcode.
  @retval Other  A pointer to the created opcode.
**/
UINT8 *
EFIAPI
HiiCreateTextOpCode (
  IN VOID           *OpCodeHandle,
  IN EFI_STRING_ID  Prompt,
  IN EFI_STRING_ID  Help,
  IN EFI_STRING_ID  TextTwo
  );
```

As you can see this function needs `EFI_STRING_ID` for the new `text` element. As we are generating our new element dynamically, let's also add new strings dynamically to the HII database:

For this task we can utilize `HiiSetString` function from the `https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiHiiLib/HiiString.c`:
```
/**
  This function create a new string in String Package or updates an existing
  string in a String Package.  If StringId is 0, then a new string is added to
  a String Package.  If StringId is not zero, then a string in String Package is
  updated.  If SupportedLanguages is NULL, then the string is added or updated
  for all the languages that the String Package supports.  If SupportedLanguages
  is not NULL, then the string is added or updated for the set of languages
  specified by SupportedLanguages.
  If HiiHandle is NULL, then ASSERT().
  If String is NULL, then ASSERT().
  @param[in]  HiiHandle           A handle that was previously registered in the
                                  HII Database.
  @param[in]  StringId            If zero, then a new string is created in the
                                  String Package associated with HiiHandle.  If
                                  non-zero, then the string specified by StringId
                                  is updated in the String Package  associated
                                  with HiiHandle.
  @param[in]  String              A pointer to the Null-terminated Unicode string
                                  to add or update in the String Package associated
                                  with HiiHandle.
  @param[in]  SupportedLanguages  A pointer to a Null-terminated ASCII string of
                                  language codes.  If this parameter is NULL, then
                                  String is added or updated in the String Package
                                  associated with HiiHandle for all the languages
                                  that the String Package supports.  If this
                                  parameter is not NULL, then then String is added
                                  or updated in the String Package associated with
                                  HiiHandle for the set oflanguages specified by
                                  SupportedLanguages.  The format of
                                  SupportedLanguages must follow the language
                                  format assumed the HII Database.
  @retval 0      The string could not be added or updated in the String Package.
  @retval Other  The EFI_STRING_ID of the newly added or updated string.
**/

EFI_STRING_ID
EFIAPI
HiiSetString (
  IN EFI_HII_HANDLE    HiiHandle,
  IN EFI_STRING_ID     StringId             OPTIONAL,
  IN CONST EFI_STRING  String,
  IN CONST CHAR8       *SupportedLanguages  OPTIONAL
  )
```

With this function we can create new strings as simple as this:
```
EFI_STRING_ID text_prompt = HiiSetString(mHiiHandle,
                                         0,
                                         L"Text prompt",
                                         NULL);
EFI_STRING_ID text_help = HiiSetString(mHiiHandle,
                                       0,
                                       L"Text help",
                                       NULL);
```

Now we can use `HiiCreateTextOpCode` function to add more opcodes to the opcode buffer refernced by `StartOpCodeHandle`:
```
UINT8* Result = HiiCreateTextOpCode(StartOpCodeHandle,
                                    text_prompt,
                                    text_help,
                                    0);
if (Result == NULL) {
  Print(L"Error! Can't create Text opcode, not enough space\n");
  HiiFreeOpCodeHandle(StartOpCodeHandle);
  HiiFreeOpCodeHandle(EndOpCodeHandle);
  HiiRemovePackages(mHiiHandle);
  return EFI_BUFFER_TOO_SMALL;
}
```

Let's add another `text` element for our example. Once again the `HiiCreateTextOpCode` would use the same `StartOpCodeHandle`:
```
text_prompt = HiiSetString(mHiiHandle,
                           0,
                           L"Another text prompt",
                           NULL);
text_help = HiiSetString(mHiiHandle,
                         0,
                         L"Another text help",
                         NULL);

Result = HiiCreateTextOpCode(StartOpCodeHandle,
                             text_prompt,
                             text_help,
                             0);
if (Result == NULL) {
  Print(L"Error! Can't create Text opcode, not enough space\n");
  HiiFreeOpCodeHandle(StartOpCodeHandle);
  HiiFreeOpCodeHandle(EndOpCodeHandle);
  HiiRemovePackages(mHiiHandle);
  return EFI_BUFFER_TOO_SMALL;
}
```

Now when we have `StartOpCodeHandle` and `EndOpCodeHandle` filled, `HiiUpdateForm` call should succeed. So let's build and load our driver.

If you look at our form, you will see that now that it has 2 `text` elements like we have intended:

![Label](Label.png?raw=true "Label")

# `HIICreate*`

In our example we've used `HiiCreateTextOpCode` function to create `EFI_IFR_TEXT_OP` opcode (i.e. `text` element). But `HiiLib` offers many functions to create all kinds of elements. Most of them we are already know:

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Library/HiiLib.h

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiHiiLib/HiiLib.c

```
HiiCreateSubTitleOpCode		EFI_IFR_SUBTITLE_OP
HiiCreateCheckBoxOpCode		EFI_IFR_CHECKBOX_OP
HiiCreateTextOpCode		EFI_IFR_TEXT_OP
HiiCreateNumericOpCode		EFI_IFR_NUMERIC_OP
HiiCreateStringOpCode		EFI_IFR_STRING_OP
HiiCreateDateOpCode		EFI_IFR_DATE_OP
HiiCreateTimeOpCode		EFI_IFR_TIME_OP
HiiCreateOneOfOpCode		EFI_IFR_ONE_OF_OP
HiiCreateOneOfOptionOpCode	EFI_IFR_ONE_OF_OPTION_OP
HiiCreateOrderedListOpCode	EFI_IFR_ORDERED_LIST_OP

HiiCreateDefaultOpCode		EFI_IFR_DEFAULT_OP
HiiCreateGuidOpCode		EFI_IFR_GUID
HiiCreateActionOpCode		EFI_IFR_ACTION_OP
HiiCreateGotoOpCode		EFI_IFR_REF_OP
HiiCreateGotoExOpCode		EFI_IFR_REF_OP, EFI_IFR_REF2_OP, EFI_IFR_REF3_OP and EFI_IFR_REF4_OP
HiiCreateEndOpCode		EFI_IFR_END_OP
```

And if this is not enough for you `HiiLib` library has a function to add raw opcode buffer:
```
/**
  Append raw opcodes to an OpCodeHandle.
  If OpCodeHandle is NULL, then ASSERT().
  If RawBuffer is NULL, then ASSERT();
  @param[in]  OpCodeHandle   Handle to the buffer of opcodes.
  @param[in]  RawBuffer      Buffer of opcodes to append.
  @param[in]  RawBufferSize  The size, in bytes, of Buffer.
  @retval NULL   There is not enough space left in Buffer to add the opcode.
  @retval Other  A pointer to the appended opcodes.
**/
UINT8 *
EFIAPI
HiiCreateRawOpCodes (
  IN VOID   *OpCodeHandle,
  IN UINT8  *RawBuffer,
  IN UINTN  RawBufferSize
  )
```
