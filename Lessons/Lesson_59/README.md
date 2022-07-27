Up until now we've displayed our forms in the same app that populated it to the HII Database. But this is not a standard approach. Usually UEFI drivers populate HII Forms and another application loads them. As we are going to create more complicated forms that would have actual non-volatile settings, it is time to separate these two functionalities.

For this we need to:
- convert our HII Form UEFI application to the UEFI driver form
- create an application that would be able to load populated HII Forms from the HII database

# `DisplayHIIByGuid`

Let's start with the second task. Create `DisplayHIIByGuid` UEFI Shell application.

`UefiLessonsPkg/DisplayHIIByGuid/DisplayHIIByGuid.inf`
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = DisplayHIIByGuid
  FILE_GUID                      = 1597e1d0-7f62-4631-a166-703f03bd7223
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = ShellCEntryLib

[Sources]
  DisplayHIIByGuid.c

[Packages]
  MdePkg/MdePkg.dec

[LibraryClasses]
  UefiLib
  ShellCEntryLib
```

`UefiLessonsPkg/DisplayHIIByGuid/DisplayHIIByGuid.c`
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  return EFI_SUCCESS;
}
```

Add new app to the `UefiLessonsPkg/UefiLessonsPkg.dsc`
```
[Components]
  ...
  UefiLessonsPkg/DisplayHIIByGuid/DisplayHIIByGuid.inf
```

Our application would take HII Package List GUID from the command argument and display HII Forms from that package.

First we need to parse argument string to GUID value. For this task we can utilize `StrToGuid` function from the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/BaseLib.h

```
/**
  Convert a Null-terminated Unicode GUID string to a value of type
  EFI_GUID.
  ...
  If String is not aligned in a 16-bit boundary, then ASSERT().
  @param  String                   Pointer to a Null-terminated Unicode string.
  @param  Guid                     Pointer to the converted GUID.
  @retval RETURN_SUCCESS           Guid is translated from String.
  @retval RETURN_INVALID_PARAMETER If String is NULL.
                                   If Data is NULL.
  @retval RETURN_UNSUPPORTED       If String is not as the above format.
**/
RETURN_STATUS
EFIAPI
StrToGuid (
  IN  CONST CHAR16  *String,
  OUT GUID          *Guid
  );
```

Use it like this:
```
if (Argc != 2) {
  Print(L"Usage:\n");
  Print(L"  DisplayHIIByGuid <Package list GUID>\n");
  return EFI_INVALID_PARAMETER;
}

EFI_GUID PackageListGuid;
EFI_STATUS Status = StrToGuid(Argv[1], &PackageListGuid);
if (Status != RETURN_SUCCESS) {
  Print(L"Error! Can't convert <Package list GUID> argument to GUID\n");
  return EFI_INVALID_PARAMETER;
}
```

Now when we have Package List GUID we can get HII Handles for this GUID with the help of the `HiiGetHiiHandles` function

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Library/HiiLib.h

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiHiiLib/HiiLib.c
```
/**
  Retrieves the array of all the HII Handles or the HII handles of a specific
  package list GUID in the HII Database.
  This array is terminated with a NULL HII Handle.
  This function allocates the returned array using AllocatePool().
  The caller is responsible for freeing the array with FreePool().
  @param[in]  PackageListGuid  An optional parameter that is used to request
                               HII Handles associated with a specific
                               Package List GUID.  If this parameter is NULL,
                               then all the HII Handles in the HII Database
                               are returned.  If this parameter is not NULL,
                               then zero or more HII Handles associated with
                               PackageListGuid are returned.
  @retval NULL   No HII handles were found in the HII database
  @retval NULL   The array of HII Handles could not be retrieved
  @retval Other  A pointer to the NULL terminated array of HII Handles
**/
EFI_HII_HANDLE *
EFIAPI
HiiGetHiiHandles (
  IN CONST EFI_GUID  *PackageListGuid  OPTIONAL
  )
;
```

In our code we can use it as simple as:
```
EFI_HII_HANDLE* HiiHandles = HiiGetHiiHandles(&PackageListGuid);

...

FreePool(HiiHandles);
```


Don't forget to include `#include <Library/HiiLib.h>` to our `*.c` file and add this library to our `*.inf` file:
```
[Packages]
  ...
  MdeModulePkg/MdeModulePkg.dec

[LibraryClasses]
  ...
  HiiLib
```

Now we have a pointer to the HII Handle array associated with our GUID. But before we call `SendForm` we need to calculate how many element this array has. For this copy `HiiHandles` data and loop over its elements until encounter NULL:
```
EFI_HII_HANDLE* HiiHandle = HiiHandles;
UINTN HandleCount=0;
while (*HiiHandle != NULL) {
  HiiHandle++;
  HandleCount++;
}
```

Now we have everything we need. It is time to call `SendForm` function from the `EFI_FORM_BROWSER2_PROTOCOL`. Add this include to our `*.c` file `#include <Protocol/FormBrowser2.h>` and this code to our `*.inf` file:
```
[Protocols]
  gEfiFormBrowser2ProtocolGuid
```

Actual code to call the protocol:
```
EFI_FORM_BROWSER2_PROTOCOL* FormBrowser2;
Status = gBS->LocateProtocol(&gEfiFormBrowser2ProtocolGuid, NULL, (VOID**)&FormBrowser2);
if (EFI_ERROR(Status)) {
  Print(L"Error! Can't locate gEfiFormBrowser2Protocol\n");
  FreePool(HiiHandles);
  return Status;
}

Status = FormBrowser2->SendForm (
                         FormBrowser2,
                         HiiHandles,
                         HandleCount,
                         NULL,
                         0,
                         NULL,
                         NULL
                         );

if (EFI_ERROR(Status)) {
  Print(L"Error! SendForm returned %r\n", Status);
}
```

Compile our application and copy it to the shared disk folder.

Earlier we've created `ShowHII` program that shows all the package lists from HII database:
```
FS0:\> ShowHII.efi
PackageList[0]: GUID=A487A478-51EF-48AA-8794-7BEE2A0562F1; size=0x1ADC
        Package[0]: type=STRINGS; size=0x1AC4
        Package[1]: type=END; size=0x4
PackageList[1]: GUID=19618BCE-55AE-09C6-37E9-4CE04084C7A1; size=0x21E4
        Package[0]: type=STRINGS; size=0x21CC
        Package[1]: type=END; size=0x4
PackageList[2]: GUID=2F30DA26-F51B-4B6F-85C4-31873C281BCA; size=0xA93
        Package[0]: type=STRINGS; size=0xA7B
        Package[1]: type=END; size=0x4
PackageList[3]: GUID=F74D20EE-37E7-48FC-97F7-9B1047749C69; size=0x2EE9
        Package[0]: type=IMAGES; size=0x2ED1
        Package[1]: type=END; size=0x4
PackageList[4]: GUID=EBF8ED7C-0DD1-4787-84F1-F48D537DCACF; size=0x46C
        Package[0]: type=FORMS; size=0x82
        Package[1]: type=FORMS; size=0x82
        Package[2]: type=STRINGS; size=0x199
        Package[3]: type=STRINGS; size=0x19B
        Package[4]: type=DEVICE_PATH; size=0x1C
        Package[5]: type=END; size=0x4
PackageList[5]: GUID=FE561596-E6BF-41A6-8376-C72B719874D0; size=0x93F
        Package[0]: type=FORMS; size=0xF5
        Package[1]: type=STRINGS; size=0x40A
        Package[2]: type=STRINGS; size=0x40C
        Package[3]: type=DEVICE_PATH; size=0x1C
        Package[4]: type=END; size=0x4
PackageList[6]: GUID=2A46715F-3581-4A55-8E73-2B769AAA30C5; size=0x6B0
        Package[0]: type=FORMS; size=0x143
        Package[1]: type=STRINGS; size=0x539
        Package[2]: type=DEVICE_PATH; size=0x1C
        Package[3]: type=END; size=0x4
PackageList[7]: GUID=99FDC8FD-849B-4EBA-AD13-FB9699C90A4D; size=0x6FE
        Package[0]: type=STRINGS; size=0x340
        Package[1]: type=STRINGS; size=0x3A6
        Package[2]: type=END; size=0x4
PackageList[8]: GUID=E38C1029-E38F-45B9-8F0D-E2E60BC9B262; size=0x15DA
        Package[0]: type=STRINGS; size=0xA88
        Package[1]: type=STRINGS; size=0xB3A
        Package[2]: type=END; size=0x4
PackageList[9]: GUID=D9DCC5DF-4007-435E-9098-8970935504B2; size=0x855
        Package[0]: type=FORMS; size=0x1F6
        Package[1]: type=STRINGS; size=0x62B
        Package[2]: type=DEVICE_PATH; size=0x1C
        Package[3]: type=END; size=0x4
PackageList[10]: GUID=F5F219D3-7006-4648-AC8D-D61DFB7BC6AD; size=0x14EC
        Package[0]: type=SIMPLE_FONTS; size=0x14D4
        Package[1]: type=END; size=0x4
PackageList[11]: GUID=4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9; size=0x6AC8
        Package[0]: type=FORMS; size=0x1030
        Package[1]: type=STRINGS; size=0x3C99
        Package[2]: type=STRINGS; size=0x1DCB
        Package[3]: type=DEVICE_PATH; size=0x1C
        Package[4]: type=END; size=0x4
PackageList[12]: GUID=F95A7CCC-4C55-4426-A7B4-DC8961950BAE; size=0x13909
        Package[0]: type=STRINGS; size=0x138F1
        Package[1]: type=END; size=0x4
PackageList[13]: GUID=DEC5DAA4-6781-4820-9C63-A7B0E4F1DB31; size=0x8677
        Package[0]: type=STRINGS; size=0x865F
        Package[1]: type=END; size=0x4
PackageList[14]: GUID=4344558D-4EF9-4725-B1E4-3376E8D6974F; size=0x83BD
        Package[0]: type=STRINGS; size=0x83A5
        Package[1]: type=END; size=0x4
PackageList[15]: GUID=0AF0B742-63EC-45BD-8DB6-71AD7F2FE8E8; size=0xCB04
        Package[0]: type=STRINGS; size=0xCAEC
        Package[1]: type=END; size=0x4
PackageList[16]: GUID=25F200AA-D3CB-470A-BF51-E7D162D22E6F; size=0x1D3D7
        Package[0]: type=STRINGS; size=0x1D3BF
        Package[1]: type=END; size=0x4
PackageList[17]: GUID=5F5F605D-1583-4A2D-A6B2-EB12DAB4A2B6; size=0x3048
        Package[0]: type=STRINGS; size=0x3030
        Package[1]: type=END; size=0x4
PackageList[18]: GUID=F3D301BB-F4A5-45A8-B0B7-FA999C6237AE; size=0x26B5
        Package[0]: type=STRINGS; size=0x269D
        Package[1]: type=END; size=0x4
PackageList[19]: GUID=7C04A583-9E3E-4F1C-AD65-E05268D0B4D1; size=0x5CB
        Package[0]: type=STRINGS; size=0x5B3
        Package[1]: type=END; size=0x4
```

Now we can use our newly created `DisplayHIIByGuid` application to show forms from the HII Database.

Try to execute the following commands:
```
FS0:\> DisplayHIIByGuid.efi EBF8ED7C-0DD1-4787-84F1-F48D537DCACF
FS0:\> DisplayHIIByGuid.efi FE561596-E6BF-41A6-8376-C72B719874D0
FS0:\> DisplayHIIByGuid.efi 2A46715F-3581-4A55-8E73-2B769AAA30C5
FS0:\> DisplayHIIByGuid.efi D9DCC5DF-4007-435E-9098-8970935504B2
FS0:\> DisplayHIIByGuid.efi 4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9
```

These GUIDs corresponds to:
- `EBF8ED7C-0DD1-4787-84F1-F48D537DCACF`
https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/DriverHealthManagerDxe/DriverHealthManagerDxe.inf

- `FE561596-E6BF-41A6-8376-C72B719874D0`
`EFI_FILE_EXPLORE_FORMSET_GUID` https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/FileExplorerLib/FormGuid.h

- `2A46715F-3581-4A55-8E73-2B769AAA30C5`
`RAM_DISK_FORM_SET_GUID` `gRamDiskFormSetGuid` https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Guid/RamDiskHii.h https://github.com/tianocore/edk2/blob/master/MdeModulePkg/MdeModulePkg.dec

- `D9DCC5DF-4007-435E-9098-8970935504B2`
https://github.com/tianocore/edk2/blob/master/OvmfPkg/PlatformDxe/Platform.inf

- `4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9`
`gIScsiConfigGuid` `ISCSI_CONFIG_GUID` https://github.com/tianocore/edk2/blob/master/NetworkPkg/Include/Guid/IScsiConfigHii.h https://github.com/tianocore/edk2/blob/master/NetworkPkg/NetworkPkg.dec


For example we can get this output on the command `DisplayHIIByGuid.efi D9DCC5DF-4007-435E-9098-8970935504B2`:

![OVMF_Settings_Form.png](OVMF_Settings_Form.png?raw=true "OVMF Settings Form")

# Formset `classguid`

If you've tried `DisplayHIIByGuid.efi` on all of the package lists with forms, you could notice that one GUID doesn't work:
```
FS0:\> DisplayHIIByGuid.efi FE561596-E6BF-41A6-8376-C72B719874D0
Error! SendForm returned Not found
```

Once again this is the GUID:
- `FE561596-E6BF-41A6-8376-C72B719874D0`
`EFI_FILE_EXPLORE_FORMSET_GUID` https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/FileExplorerLib/FormGuid.h

Let's look again at the `EFI_FORM_BROWSER2_PROTOCOL.SendForm()` description:
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
Up until now we've used `FormsetGuid = NULL` in all of the function calls. Which according to the descriptio means that `EFI_HII_PLATFORM_SETUP_FORMSET_GUID` is used.

Also earlier we've investigated that if don't explicitly declare `classguid` for our formset, IFR will still has default value which is equal to the same `EFI_HII_PLATFORM_SETUP_FORMSET_GUID`. This is the reason why all of our `SendForm` calls have worked up until now.

But if you look at the VFR code of the form from the problematic package list (https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/FileExplorerLib/FileExplorerVfr.vfr) you would see that this form uses different `classguid`:
```
formset
  guid = EFI_FILE_EXPLORE_FORMSET_GUID,
  title = STRING_TOKEN(STR_FILE_EXPLORER_TITLE),
  help = STRING_TOKEN(STR_NULL_STRING),
  classguid = EFI_FILE_EXPLORE_FORMSET_GUID,
```
In fact it is the same GUID that package list has.

To correctly display formsets with `classguid` different from the default value (`EFI_HII_PLATFORM_SETUP_FORMSET_GUID`) we need to add a possibility to get this GUID from user.

First we need to modify program description a little bit:
```
-  if (Argc != 2) {
+  if ((Argc < 2) || (Argc > 3)) {
     Print(L"Usage:\n");
-    Print(L"  DisplayHIIByGuid <Package list GUID>\n");
+    Print(L"  DisplayHIIByGuid <Package list GUID> [<Formset classguid>]\n");
     return EFI_INVALID_PARAMETER;
   }
```
Then add new argument handling:
```
EFI_GUID FormsetClassGuid = EFI_HII_PLATFORM_SETUP_FORMSET_GUID;
if (Argc == 3) {
  Status = StrToGuid(Argv[2], &FormsetClassGuid);
  if (Status != RETURN_SUCCESS) {
    Print(L"Error! Can't convert <Formset classguid> argument to GUID\n");
    return EFI_INVALID_PARAMETER;
  }
}
```
And finally `FormsetClassGuid` in our `SendForm` call:
```
   Status = FormBrowser2->SendForm (
                            FormBrowser2,
                            HiiHandles,
                            HandleCount,
-                           NULL,
+                           &FormsetClassGuid,
                            0,
                            NULL,
                            NULL
                            );
```

Now you can verify that with a:
```
FS0:\> DisplayHIIByGuid.efi FE561596-E6BF-41A6-8376-C72B719874D0 FE561596-E6BF-41A6-8376-C72B719874D0
```
We can correctly display the File Explorer form:

![FileExplorer](FileExplorer.png?raw=true "FileExplorer")

# `HIIStaticFormDriver`

Now let's create `HIIStaticFormDriver` UEFI driver that would be a copy of a `HIIStaticForm` UEFI application that we've created earlier, except that it would be a driver:
```
$ ./createNewDriver.sh HIIStaticFormDriver
$ cp UefiLessonsPkg/HIIStaticForm/Form.vfr UefiLessonsPkg/HIIStaticFormDriver/
$ cp UefiLessonsPkg/HIIStaticForm/Strings.uni UefiLessonsPkg/HIIStaticFormDriver/
```

Here is full code for the `HIIStaticFormDriver` `*.c` and `*.inf` files. You can compare it with a `HIIStaticForm` application code.

`UefiLessonsPkg/HIIStaticFormDriver/HIIStaticFormDriver.inf`:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = HIIStaticFormDriver
  FILE_GUID                      = 22514099-ad3b-45ec-b14b-112eb6446db2
  MODULE_TYPE                    = UEFI_DRIVER
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = HIIStaticFormDriverEntryPoint
  UNLOAD_IMAGE                   = HIIStaticFormDriverUnload

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

`UefiLessonsPkg/HIIStaticFormDriver/HIIStaticFormDriver.c`:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/HiiLib.h>


extern UINT8 FormBin[];

EFI_HII_HANDLE Handle;


EFI_STATUS
EFIAPI
HIIStaticFormDriverUnload (
  EFI_HANDLE ImageHandle
  )
{
  if (Handle != NULL)
    HiiRemovePackages(Handle);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HIIStaticFormDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Handle = HiiAddPackages(
             &gEfiCallerIdGuid,
             NULL,
             HIIStaticFormDriverStrings,
             FormBin,
             NULL
           );
  if (Handle == NULL)
    return EFI_OUT_OF_RESOURCES;

  return EFI_SUCCESS;
}
```

Build `HIIStaticFormDriver`, run UEFI shell and load it:
```
FS0:\> load HIIStaticFormDriver.efi
Image 'FS0:\HIIStaticFormDriver.efi' loaded at 688A000 - Success
```
You can see that our driver have populated new data to the HII Database:
```
FS0:\> ShowHII.efi
...
PackageList[20]: GUID=22514099-AD3B-45EC-B14B-112EB6446DB2; size=0x28B
        Package[0]: type=FORMS; size=0x98
        Package[1]: type=STRINGS; size=0x1DB
        Package[2]: type=END; size=0x4
```

And with a help of our `DisplayHIIByGuid.efi` application you can see the populated HII data:
```
FS0:\> DisplayHIIByGuid.efi 22514099-AD3B-45EC-B14B-112EB6446DB2
```

![HII_static_form](HII_static_form.png?raw=true "HII Static Form")

# See your application in the main UEFI menu

If you exit to the main UEFI menu:
```
FS0:\> exit
```

and go the `Device Manager`, you could find our formset as one of the possible menus:

![DeviceManager](DeviceManager.png?raw=true "DeviceManager")

If you check this menu, you'll find out that this is indeed our driver form.

The current EDKII code works in a way, that all the forms with a `classguid = EFI_HII_PLATFORM_SETUP_FORMSET_GUID` will be added to the `Device Manager` menu.

So if you don't use different `classguid` in your forms it is probably easier to check your form at this menu than typing `DisplayHIIByGuid.efi <Package list GUID>.

If you curious how new forms are dynamically added to the Device Manager check https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/DeviceManagerUiLib/DeviceManager.c

Here is some snippet from that file where you can see that code indeed uses `CompareGuid` to compare every formset `classguid` with `EFI_HII_PLATFORM_SETUP_FORMSET_GUID` and proceeds only if `CompareGuid` returns `TRUE`:
```
  //
  // Get all the Hii handles
  //
  HiiHandles = HiiGetHiiHandles (NULL);

  ...

  //
  // Search for formset of each class type
  //
  for (Index = 0; HiiHandles[Index] != NULL; Index++) {
    Status = HiiGetFormSetFromHiiHandle (HiiHandles[Index], &Buffer, &BufferSize);
    if (EFI_ERROR (Status)) {
      continue;
    }

    Ptr = (UINT8 *)Buffer;
    while (TempSize < BufferSize) {
      TempSize += ((EFI_IFR_OP_HEADER *)Ptr)->Length;
      if (((EFI_IFR_OP_HEADER *)Ptr)->Length <= OFFSET_OF (EFI_IFR_FORM_SET, Flags)) {
        Ptr += ((EFI_IFR_OP_HEADER *)Ptr)->Length;
        continue;
      }

      ClassGuidNum = (UINT8)(((EFI_IFR_FORM_SET *)Ptr)->Flags & 0x3);
      ClassGuid    = (EFI_GUID *)(VOID *)(Ptr + sizeof (EFI_IFR_FORM_SET));
      while (ClassGuidNum-- > 0) {
        if (CompareGuid (&gEfiHiiPlatformSetupFormsetGuid, ClassGuid) == 0) {      <------- Add only formsets with
          ClassGuid++;                                                                      EFI_HII_PLATFORM_SETUP_FORMSET_GUID
          continue;
        }
    ...
```
