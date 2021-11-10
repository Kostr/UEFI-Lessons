The main purpose of HII is to present to the user configuration menus to control UEFI settings.

We've already covered HII strings and HII fonts. Now it is time to talk about HII forms - the final element, that glues everything together.

The data in HII form packages is encoded in a special IFR format where IFR stands for the Internal Form Representation. It is not easy to construct forms packages by hand, as IFR is not very human readable. It is a series of operational codes (opcodes), and its parsing process can be very tedious.

To ease things EDKII offers a way to write HII forms in a special human friendly language called VFR. Here VFR stands for Visual Form Representation (opposed to Internal Form Representation) and its specification can be found under the https://edk2-docs.gitbook.io/edk-ii-vfr-specification/
EDKII has a special utility called `VfrCompile` to transorm VFR code to C arrays with IFR opcodes https://github.com/tianocore/edk2/tree/master/BaseTools/Source/C/VfrCompile

Let's try to create an application that would show us a simple form.

Create new `HIISimpleForm` application with a HiiLib.
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = HIISimpleForm
  FILE_GUID                      = df2f1465-2bf1-492c-af6c-232ac40bdf82
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = UefiMain

[Sources]
  HIISimpleForm.c

[Packages]
  MdePkg/MdePkg.dec
  MdeModulePkg/MdeModulePkg.dec

[LibraryClasses]
  UefiApplicationEntryPoint
  UefiLib
  HiiLib
```
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/HiiLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}
```
Add it to the DSC file UefiLessonsPkg/UefiLessonsPkg.dsc:
```
[Components]
  ...
  UefiLessonsPkg/HIISimpleForm/HIISimpleForm.inf
```

And this is our first VFR `UefiLessonsPkg/HIISimpleForm/Form.vfr`:
```
#define HIISIMPLEFORM_FORMSET_GUID  {0xef2acc91, 0x7b50, 0x4ab9, {0xab, 0x67, 0x2b, 0x4, 0xf8, 0xbc, 0x13, 0x5e}}

formset
  guid     = HIISIMPLEFORM_FORMSET_GUID,
  title    = STRING_TOKEN(HIISIMPLEFORM_FORMSET_TITLE),
  help     = STRING_TOKEN(HIISIMPLEFORM_FORMSET_HELP),
endformset;
```

Everything in VFR must be encoded inside the `formset` component. This component starts with the `formset` keyword and ends with the `endformset` keyword. The `formset` component must have the 3 mandatory fields: `guid`, `title` and `help`.

We can encode `guid` either in place:
```
guid     = {0xef2acc91, 0x7b50, 0x4ab9, {0xab, 0x67, 0x2b, 0x4, 0xf8, 0xbc, 0x13, 0x5e}}
```
Or with a help of a define statement which is similar to C syntax like we did above.

The `title` and `help` fields should contain string IDs of the strings. Therefore to get them we use `STRING_TOKEN(...)`, and the strings itself should be encoded in the UNI file (in our case `UefiLessonsPkg/HIISimpleForm/Strings.uni`):
```
#langdef en-US "English"

#string HIISIMPLEFORM_FORMSET_TITLE          #language en-US  "Simple Formset"
#string HIISIMPLEFORM_FORMSET_HELP           #language en-US  "This is a very simple formset"
```

The `title` and `help` fields would be visible if our formset would be included into another formset. For example look at the https://github.com/tianocore/edk2/blob/master/OvmfPkg/PlatformDxe/PlatformForms.vfr:
```
formset
  guid  = OVMF_PLATFORM_CONFIG_GUID,
  title = STRING_TOKEN(STR_FORMSET_TITLE),
  help  = STRING_TOKEN(STR_FORMSET_HELP),
  ...
endformset;
```
And https://github.com/tianocore/edk2/blob/master/OvmfPkg/PlatformDxe/Platform.uni
```
#langdef en-US "English"

#string STR_FORMSET_TITLE        #language en-US "OVMF Platform Configuration"
#string STR_FORMSET_HELP         #language en-US "Change various OVMF platform settings."
```
This would produce this output in the BIOS menu:
![Formset_title_help](Formset_title_help.png?raw=true "Formset_title_help")


Okay, now that we have VFR and UNI files it is time to publish our form to the HII. Add VFR and UNI files to the `Sources` section in the `UefiLessonsPkg/HIISimpleForm/HIISimpleForm.inf`:
```
[Sources]
  ...
  Strings.uni
  Form.vfr
```

Here is a code that would populate our form and its strings to the HII database `UefiLessonsPkg/HIISimpleForm/HIISimpleForm.c`:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/HiiLib.h>

extern UINT8 FormBin[];

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_HII_HANDLE Handle = HiiAddPackages(
                             &gEfiCallerIdGuid,
                             NULL,
                             HIISimpleFormStrings,
                             FormBin,
                             NULL
                             );
  if (Handle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}
```

As you can see here we use `FormBin` array to publish our form data. EDKII build system generates HII form package from every VFR file, and puts its data into the `<VFR name>Bin` array. Like with the UNI files this array is prependend with a 4-byte packages size header. Therefore it can be use with `HiiAddPackages` library function as-is.
Also as this array would be declared in the autogenerated *.c file (and not in *.h file), we also need to declare it as an `extern` in our file.

But let's see it ourself. Build module and look at the `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIISimpleForm/HIISimpleForm/DEBUG/Form.c`
```
unsigned char FormBin[] = {
  // ARRAY LENGTH

  0x3D,  0x00,  0x00,  0x00,

  // PACKAGE HEADER

  0x39,  0x00,  0x00,  0x02,

  // PACKAGE DATA

  0x0E,  0xA7,  0x91,  0xCC,  0x2A,  0xEF,  0x50,  0x7B,  0xB9,  0x4A,  0xAB,  0x67,  0x2B,  0x04,  0xF8,  0xBC,
  0x13,  0x5E,  0x02,  0x00,  0x03,  0x00,  0x01,  0x71,  0x99,  0x03,  0x93,  0x45,  0x85,  0x04,  0x4B,  0xB4,
  0x5E,  0x32,  0xEB,  0x83,  0x26,  0x04,  0x0E,  0x5C,  0x06,  0x00,  0x00,  0x00,  0x00,  0x5C,  0x06,  0x00,
  0x00,  0x01,  0x00,  0x29,  0x02

};
```

Build system also produces one more interesting file. Look at the content of the `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIISimpleForm/HIISimpleForm/DEBUG/Form.lst`:
```
//
//  VFR compiler version  2.01 (UEFI 2.4) Developer Build based on Revision: Unknown
//
extern unsigned char HIISimpleFormStrings[];
formset
>00000000: 0E A7 91 CC 2A EF 50 7B B9 4A AB 67 2B 04 F8 BC 13 5E 02 00 03 00 01 71 99 03 93 45 85 04 4B B4 5E 32 EB 83 26 04 0E
>00000027: 5C 06 00 00 00 00
>0000002D: 5C 06 00 00 01 00
  guid = {0xef2acc91, 0x7b50, 0x4ab9, {0xab, 0x67, 0x2b, 0x4, 0xf8, 0xbc, 0x13, 0x5e}},
  title = STRING_TOKEN(0x0002),
  help = STRING_TOKEN(0x0003),
endformset;
>00000033: 29 02

//
// All Opcode Record List
//
>00000000: 0E A7 91 CC 2A EF 50 7B B9 4A AB 67 2B 04 F8 BC 13 5E 02 00 03 00 01 71 99 03 93 45 85 04 4B B4 5E 32 EB 83 26 04 0E
>00000027: 5C 06 00 00 00 00
>0000002D: 5C 06 00 00 01 00
>00000033: 29 02

Total Size of all record is 0x00000035
```
Here you can see how the `PACKAGE DATA` part in the `FormBin` array is constructed from the IFR opcodes with some comments about relations to the responsible VFR code.

# IFR data parsing

Our data currently consist of 4 components: `EFI_IFR_FORM_SET`, two `EFI_IFR_DEFAULTSTORE` and `EFI_IFR_END`. Each of these take one string of data in the `Form.lst`.

Like with String packages every possible component has a common header. So let's look at its definition first:
```
EFI_IFR_OP_HEADER:

Summary:
Standard opcode header

Prototype:
typedef struct _EFI_IFR_OP_HEADER {
 UINT8 OpCode;
 UINT8 Length:7;
 UINT8 Scope:1;
} EFI_IFR_OP_HEADER;

Members:
OpCode 		Defines which type of operation is being described by this header.
Length 		Defines the number of bytes in the opcode, including this header.
Scope   	If this bit is set, the opcode begins a new scope, which is ended by an EFI_IFR_END opcode.

Description:
Forms are represented in a binary format roughly similar to processor instructions. Each header contains an opcode, a length and a scope indicator.
If Scope indicator is set, the scope exists until it reaches a corresponding EFI_IFR_END opcode. Scopes may be nested within other scopes.
```

Now here is a definition of a `EFI_IFR_FORM_SET`:
```
EFI_IFR_FORM_SET

Summary:
The form set is a collection of forms that are intended to describe the pages that will be displayed to the user.

Prototype:

#define EFI_IFR_FORM_SET_OP 0x0E

typedef struct _EFI_IFR_FORM_SET {
 EFI_IFR_OP_HEADER Header;
 EFI_GUID Guid;
 EFI_STRING_ID FormSetTitle;
 EFI_STRING_ID Help;
 UINT8 Flags;
//EFI_GUID ClassGuid[…];
} EFI_IFR_FORM_SET;

Members:
Header 		The sequence that defines the type of opcode as well as the length of the opcode being defined. Header.OpCode = EFI_IFR_FORM_SET_OP.
Guid 		The unique GUID value associated with this particular form set.
FormSetTitle    The string token reference to the title of this particular form set.
Help            The string token reference to the help of this particular form set.
Flags           Flags which describe additional features of the form set. Bits 0:1 = number of members in ClassGuid. Bits 2:7 = Reserved. Should be set to zero.
ClassGuid       Zero to four class identifiers.

Description
The form set consists of a header and zero or more forms.
```
We didn't declare any class guid in our VFR. But nevertheless in the opcode output you can see that this IFR has one `ClassGuid` equal to `93039971-8545-4b04-b45e-32eb8326040e`. It is assigned by default by the build system if we haven't provided any other GUID for the class. https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Guid/HiiPlatformSetupFormset.h
```
#define EFI_HII_PLATFORM_SETUP_FORMSET_GUID \
  { 0x93039971, 0x8545, 0x4b04, { 0xb4, 0x5e, 0x32, 0xeb, 0x83, 0x26, 0x4, 0xe } }
```
If we would want to declare any other GUID for the class, VFR syntax is:
```
classguid = <...>
```

Next there are couple of `EFI_IFR_DEFAULTSTORE` elements:
```
EFI_IFR_DEFAULTSTORE

Summary:
Provides a declaration for the type of default values that a question can be associated with

Prototype:
#define EFI_IFR_DEFAULTSTORE_OP 0x5c

typedef struct _EFI_IFR_DEFAULTSTORE {
 EFI_IFR_OP_HEADER Header;
 EFI_STRING_ID DefaultName;
 UINT16 DefaultId;
} EFI_IFR_DEFAULTSTORE;

Members
Header 		The sequence that defines the type of opcode as well as the length of the opcode being defined.
		For this tag, Header.OpCode = EFI_IFR_DEFAULTSTORE_OP
DefaultName 	A string token reference for the human readable string associated with the type of default being declared.
DefaultId 	The default identifier, which is unique within the current form set. The default identifier creates a group of defaults

Description:
Declares a class of default which can then have question default values associated with. An EFI_IFR_DEFAULTSTORE with a specified DefaultId must appear in the IFR before it can be referenced by an EFI_IFR_DEFAULT.
```

As we've opened a scope in the `EFI_IFR_FORM_SET`, we need to close it with a `EFI_IFR_END`:
```
EFI_IFR_END

Summary:
End of the current scope.

Prototype:

#define EFI_IFR_END_OP 0x29

typedef struct _EFI_IFR_END {
 EFI_IFR_OP_HEADER Header;
} EFI_IFR_END;

Members:
Header Standard opcode header, where OpCode is EFI_IFR_END_OP.

Description:
Marks the end of the current scope.
```

# Show form

To actually show form we need to utilize `EFI_FORM_BROWSER2_PROTOCOL` function `SendForm`:
```
EFI_FORM_BROWSER2_PROTOCOL.SendForm()

Summary:
Initialize the browser to display the specified configuration forms.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_SEND_FORM2) (
 IN CONST EFI_FORM_BROWSER2_PROTOCOL *This,
 IN EFI_HII_HANDLE *Handles,
 IN UINTN HandleCount,
 IN CONST EFI_GUID *FormsetGuid, OPTIONAL
 IN EFI_FORM_ID FormId, OPTIONAL
 IN CONST EFI_SCREEN_DESCRIPTOR *ScreenDimensions, OPTIONAL
 OUT EFI_BROWSER_ACTION_REQUEST *ActionRequest OPTIONAL
 );

Parameters:
This			A pointer to the EFI_FORM_BROWSER2_PROTOCOL instance.
Handles			A pointer to an array of HII handles to display.
HandleCount		The number of handles in the array specified by Handle.
FormsetGuid		This field points to the EFI_GUID which must match the Guid field or one of the
                	elements of the ClassId field in the EFI_IFR_FORM_SET op-code. If FormsetGuid
                	is NULL, then this function will display the form set class
                	EFI_HII_PLATFORM_SETUP_FORMSET_GUID.
FormId			This field specifies the identifier of the form within the form set to render as the first
			displayable page. If this field has a value of 0x0000, then the Forms Browser will
			render the first enabled form in the form set.
ScreenDimensions	Points to recommended form dimensions, including any non-content area, in characters.
ActionRequested		Points to the action recommended by the form.

Description:
This function is the primary interface to the Forms Browser. The Forms Browser displays the forms specified by FormsetGuid and FormId from all of HII handles specified by Handles. If more than one form can be displayed, the Forms Browser will provide some means for the user to navigate between the
forms in addition to that provided by cross-references in the forms themselves.
```

As you can see most of the parameters are optional. So we can call this function as simple as:
```
EFI_STATUS Status;
EFI_FORM_BROWSER2_PROTOCOL* FormBrowser2;
Status = gBS->LocateProtocol(&gEfiFormBrowser2ProtocolGuid, NULL, (VOID**)&FormBrowser2);
if (EFI_ERROR(Status)) {
  return Status;
}

Status = FormBrowser2->SendForm (
                         FormBrowser2,
                         &Handle,
                         1,
                         NULL,
                         0,
                         NULL,
                         NULL
                         );
```

Don't forget to add `gEfiFormBrowser2ProtocolGuid` to the application INF file:
```
[Protocols]
  gEfiFormBrowser2ProtocolGuid
```
And add necessary include:
```
#include <Protocol/FormBrowser2.h>
```

Also at the end of our program we need to remove our HII packages as they are no longer needed:
```
HiiRemovePackages(Handle);
```

If we build and run our application now we would see that Form browser doesn't display anything for us and immediately returns control to the shell.
This is happening because there is nothing to display in our formset. The core element for the formset is form. Therefore let's add the most simple form to our formset `UefiLessonsPkg/HIISimpleForm/Form.vfr`:
```
formset
  guid     = HIISIMPLEFORM_FORMSET_GUID,
  title    = STRING_TOKEN(HIISIMPLEFORM_FORMSET_TITLE),
  help     = STRING_TOKEN(HIISIMPLEFORM_FORMSET_HELP),
  form formid = 1,
    title = STRING_TOKEN(HIISIMPLEFORM_FORMID1_TITLE);
  endform;
endformset;
```
Each form must have at least `formid` and `title`. You can say that they are mandatory fields. `title` would be used for the page title when the form is displayed and `formid` is used to reference form from other code.

Also I want to note that it is possible to write `formid` attribute on a separate string like all the other form attributes:
```
form
  formid = 1,
  title = STRING_TOKEN(HIISIMPLEFORM_FORMID1_TITLE);
endform;
```
But usually `formid` is written at the same string as the `form` keyword. Both syntax are equivalent in VFR.

Don't forget to add new string to the `UefiLessonsPkg/HIISimpleForm/Strings.uni`:
```
...
#string HIISIMPLEFORM_FORMID1_TITLE          #language en-US  "Simple Form"
```

If you build and execute our application now you would get folowing output:
![SimpleForm](SimpleForm.png?raw=true "SimpleForm")

# IFR data parsing

If you look at the `Form.lst` now you would see:
```
//
//  VFR compiler version  2.01 (UEFI 2.4) Developer Build based on Revision: Unknown
//
extern unsigned char HIISimpleFormStrings[];
formset
>00000000: 0E A7 91 CC 2A EF 50 7B B9 4A AB 67 2B 04 F8 BC 13 5E 02 00 03 00 01 71 99 03 93 45 85 04 4B B4 5E 32 EB 83 26 04 0E
>00000027: 5C 06 00 00 00 00
>0000002D: 5C 06 00 00 01 00
  guid = {0xef2acc91, 0x7b50, 0x4ab9, {0xab, 0x67, 0x2b, 0x4, 0xf8, 0xbc, 0x13, 0x5e}},
  title = STRING_TOKEN(0x0002),
  help = STRING_TOKEN(0x0003),
  form
>00000033: 01 86 01 00 04 00
    formid = 1,
    title = STRING_TOKEN(0x0004);
  endform;
>00000039: 29 02
endformset;
>0000003B: 29 02
```

Two IFRs were added. First is `EFI_IFR_FORM`.
```
EFI_IFR_FORM

Summary:
Creates a form.

Prototype:
#define EFI_IFR_FORM_OP 0x01

typedef struct _EFI_IFR_FORM {
 EFI_IFR_OP_HEADER Header;
 EFI_FORM_ID FormId;
 EFI_STRING_ID FormTitle;
} EFI_IFR_FORM;

Members:
Header 		The sequence that defines the type of opcode as well as the length of the opcode being defined. Header.OpCode = EFI_IFR_FORM_OP.
FormId 		The form identifier, which uniquely identifies the form within the form set. The form identifier, along with the device path
		and form set GUID, uniquely identifies a form within a system.
FormTitle 	The string token reference to the title of this particular form.

Description:
A form is the encapsulation of what amounts to a browser page. The header defines a FormId, which is referenced by the form set, among others. It also defines a FormTitle, which is a string to be used as the title for the form.
```
But also as this opcode opens another scope we also now have one more `EFI_IFR_END` opcode.


