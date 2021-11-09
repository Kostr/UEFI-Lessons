We know how to add font for the new language, and we now how to populate strings for some of the existing languages in the system dynamically.
Let's see if it is possible to create another language option dynamically.

`Select Language` menu gets all possible language options from the value of the `PlatformLangCodes` EFI variable. And the current language option is reflected by the `PlatformLang` EFI option.

Initially the values for these options are set with a help of PCD in the https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec:
```
  ## Default platform supported RFC 4646 languages: (American) English & French.
  # @Prompt Default Value of PlatformLangCodes Variable.
  gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultPlatformLangCodes|"en;fr;en-US;fr-FR"|VOID*|0x0000001e

  ## Default current RFC 4646 language: (American) English.
  # @Prompt Default Value of PlatformLang Variable.
  gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultPlatformLang|"en-US"|VOID*|0x0000001f
```
You can look at the actual code in the `UiCreateLanguageMenu` function from the https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Application/UiApp/FrontPageCustomizedUiSupport.c.

Let's see if it is possible to modify `PlatformLangCodes` and add another language into it.

First create an app that would print the value for the `PlatformLangCodes` option:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;

  CHAR8* LanguageString;
  Status = GetEfiGlobalVariable2(L"PlatformLangCodes", (VOID**)&LanguageString, NULL);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't perform GetEfiGlobalVariable2, status=%r\n", Status);
    return Status;
  }
  Print(L"Current value of the 'PlatformLangCodes' variable is '%a'\n", LanguageString);

  return EFI_SUCCESS;
}
```
We've already used `gRT->GetVariable` protocol function directly before, so here we use `GetEfiGlobalVariable2` library function to the simplify code. This function is defined in the https://github.com/tianocore/edk2/blob/master/MdePkg/Library/UefiLib/UefiLib.c:
```
**
  Returns a pointer to an allocated buffer that contains the contents of a
  variable retrieved through the UEFI Runtime Service GetVariable().  This
  function always uses the EFI_GLOBAL_VARIABLE GUID to retrieve variables.
  The returned buffer is allocated using AllocatePool().  The caller is
  responsible for freeing this buffer with FreePool().
  If Name is NULL, then ASSERT().
  If Value is NULL, then ASSERT().
  @param[in]  Name  The pointer to a Null-terminated Unicode string.
  @param[out] Value The buffer point saved the variable info.
  @param[out] Size  The buffer size of the variable.
  @return EFI_OUT_OF_RESOURCES      Allocate buffer failed.
  @return EFI_SUCCESS               Find the specified variable.
  @return Others Errors             Return errors from call to gRT->GetVariable.
**/
EFI_STATUS
EFIAPI
GetEfiGlobalVariable2 (
  IN CONST CHAR16    *Name,
  OUT VOID           **Value,
  OUT UINTN          *Size OPTIONAL
  )
```

If you build and run our application now, you would get the value from the PCD:
```
FS0:\> AddNewLanguage.efi
Current value of the 'PlatformLangCodes' variable is 'en;fr;en-US;fr-FR'
```

Now let's try to add `;ru-RU` to the end of the variable and write it back.

First construct new string and fill it with the necessary data:
```
CHAR8* NewLanguageString = AllocatePool(AsciiStrLen(LanguageString) + AsciiStrSize(";ru-RU"));
if (NewLanguageString == NULL) {
  Print(L"Error! Can't allocate size for new PlatformLangCodes variable\n");
  FreePool(LanguageString);
  return EFI_OUT_OF_RESOURCES;
}

CopyMem(NewLanguageString, LanguageString, AsciiStrLen(LanguageString));
CopyMem(&NewLanguageString[AsciiStrLen(LanguageString)], ";ru-RU", AsciiStrSize(";ru-RU"));

Print(L"Set 'PlatformLangCodes' variable to '%a'\n", NewLanguageString);
```
Just in case ASCII string functions are defined in the https://github.com/tianocore/edk2/blob/master/MdePkg/Library/BaseLib/String.c.
Also don't forget to include `Library/MemoryAllocationLib.h` header for the `AllocatePool` function and `Library/BaseMemoryLib.h` header for the `CopyMem` function.

Now use `SetVariable` call to update variable:
```
Status = gRT->SetVariable (
              L"PlatformLangCodes",
              &gEfiGlobalVariableGuid,
              EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
              AsciiStrSize(NewLanguageString),
              NewLanguageString
              );
if (EFI_ERROR(Status)) {
  Print(L"Error! Can't set 'PlatformLangCodes' variable, status=%r\n", Status);
}
```
`SetVariable` is a Runtime Service, you can find its definition in the UEFI specification:
```
SetVariable()

Summary:
Sets the value of a variable.

Prototype:
typedef
EFI_STATUS
SetVariable (
 IN CHAR16 *VariableName,
 IN EFI_GUID *VendorGuid,
 IN UINT32 Attributes,
 IN UINTN DataSize,
 IN VOID *Data
 );

Parameters:
VariableName 	A Null-terminated string that is the name of the vendor’s variable. Each VariableName is unique for each VendorGuid.
VendorGuid 	A unique identifier for the vendor.
Attributes 	Attributes bitmask to set for the variable.
DataSize 	The size in bytes of the Data buffer.
Data 		The contents for the variable.
```
In case you forgot `gRT` is a shortcut for the `SystemTable->RuntimeServices` from the `UefiRuntimeServicesTableLib` library. So don't forget to include its library header `<Library/UefiRuntimeServicesTableLib.h>`.

If you build and run our application now you would get:
```
FS0:\> AddNewLanguage.efi
Current value of the 'PlatformLangCodes' variable is 'en;fr;en-US;fr-FR'
Set 'PlatformLangCodes' variable to 'en;fr;en-US;fr-FR;ru-RU'
Error! Can't set 'PlatformLangCodes' variable, status=Write Protected
```

Unfortunately it is not possible to add new language at runtime as 'PlatformLangCodes' EFI variable is write protected. Therefore it is not possible to add another localization language at runtime.

If you look at the UEFI spec you'll see:
```
The PlatformLangCodes variable contains a null- terminated ASCII string representing the language
codes that the firmware can support. At initialization time the firmware computes the supported
languages and creates this data variable. Since the firmware creates this value on each initialization, its
contents are not stored in nonvolatile memory. This value is considered read-only. 
```

# EDKII_VARIABLE_POLICY_PROTOCOL

The `PlatformLangCodes` is locked for modifications with a help of a `gEdkiiVariablePolicyProtocolGuid` protocol. This is a custom EDKII protocol for setting different policies on variables.

You can read more about the UEFI Variable Policy protocol in the https://github.com/tianocore/edk2/tree/master/MdeModulePkg/Library/VariablePolicyLib/ReadMe.md

The header file is placed under https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Protocol/VariablePolicy.h

The policy for the `PlatformLangCodes` EFI variable is set in the https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/BdsDxe/BdsEntry.c along with couple of other variables:
```
///
/// The read-only variables defined in UEFI Spec.
///
CHAR16  *mReadOnlyVariables[] = {
  EFI_PLATFORM_LANG_CODES_VARIABLE_NAME,             // L"PlatformLangCodes"  The language codes that the firmware supports
  EFI_LANG_CODES_VARIABLE_NAME,
  EFI_BOOT_OPTION_SUPPORT_VARIABLE_NAME,
  EFI_HW_ERR_REC_SUPPORT_VARIABLE_NAME,
  EFI_OS_INDICATIONS_SUPPORT_VARIABLE_NAME
  };

...

  // Mark the read-only variables if the Variable Lock protocol exists
  //
  Status = gBS->LocateProtocol(&gEdkiiVariablePolicyProtocolGuid,
NULL, (VOID**)&VariablePolicy);
  DEBUG((DEBUG_INFO, "[BdsDxe] Locate Variable Policy protocol -
%r\n", Status));
  if (!EFI_ERROR (Status)) {
    for (Index = 0; Index < ARRAY_SIZE (mReadOnlyVariables); Index++) {
      Status = RegisterBasicVariablePolicy(
                 VariablePolicy,
                 &gEfiGlobalVariableGuid,
                 mReadOnlyVariables[Index],
                 VARIABLE_POLICY_NO_MIN_SIZE,
                 VARIABLE_POLICY_NO_MAX_SIZE,
                 VARIABLE_POLICY_NO_MUST_ATTR,
                 VARIABLE_POLICY_NO_CANT_ATTR,
                 VARIABLE_POLICY_TYPE_LOCK_NOW
                 );
      ASSERT_EFI_ERROR(Status);
    }
  }
```

# Try to execute DisableVariablePolicy()

We can try to perform `DisableVariablePolicy()` to disable `VariablePolicyProtocol`.

UefiLessonsPkg/AddNewLanguage/AddNewLanguage.c
```
...
#include <Protocol/VariablePolicy.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  ...

  EDKII_VARIABLE_POLICY_PROTOCOL* VariablePolicyProtocol;
  Status = gBS->LocateProtocol(&gEdkiiVariablePolicyProtocolGuid,
                               NULL, 
                               (VOID**)&VariablePolicyProtocol);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Could not find Variable Policy protocol: %r\n", Status);
    return Status;
  }  
  Status = VariablePolicyProtocol->DisableVariablePolicy();
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't disable VariablePolicy: %r\n", Status);
    return Status;
  }
}
```

UefiLessonsPkg/AddNewLanguage/AddNewLanguage.inf:
```
....
[Packages]
  ...
  MdeModulePkg/MdeModulePkg.dec
...

[Protocols]
  gEdkiiVariablePolicyProtocolGuid
```

But unfortunately this call would fail:
```
FS0:\> AddNewLanguage.efi
Current value of the 'PlatformLangCodes' variable is 'en;fr;en-US;fr-FR'
Set 'PlatformLangCodes' variable to 'en;fr;en-US;fr-FR;ru-RU'
Error! Can't set PlatformLangCodes variable, status=Write Protected
Error! Can't disable VariablePolicy: Write Protected
```

This happend because in the end of the DXE UEFI stage variable policy is locked in the MdeModulePkg/Universal/Variable/RuntimeDxe/VariableDxe.c:
```
VOID
EFIAPI
OnEndOfDxe (
  EFI_EVENT Event,
  VOID *Context
)
{
  EFI_STATUS Status;
  DEBUG ((EFI_D_INFO, "[Variable]END_OF_DXE is signaled\n"));
   ...
  Status = LockVariablePolicy ();
   ...
}
```
Locking means that is no longer possible to change or disable policy for variables. Therefore there is no way to change `PlatformLangCodes` at runtime.

