# Create UEFI variable

In our `efivarstore` we refer to a particular UEFI variable. HII subsystem can manage access to this variable, but it is necessary that this variable already exists in the system.

So let's create this variable in the driver entry point. To keep this variable between reboots we need to first check if the variable already exists with a help of a `gRT->GetVariable` call. And only if the variable is not present create one with a default value. This mechanics is used many times in the edk2 codebase.
```
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#define FORMSET_GUID  {0xef2acc91, 0x7b50, 0x4ab9, {0xab, 0x67, 0x2b, 0x4, 0xf8, 0xbc, 0x13, 0x5e}}

EFI_STATUS
EFIAPI
HIIFormCheckboxEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;

  EFI_GUID Guid = FORMSET_GUID;

  UINTN BufferSize;
  UINT8 EfiVarstore;
  BufferSize = sizeof(UINT8);
  Status = gRT->GetVariable(
                L"CheckboxValue",
                &Guid,
                NULL,
                &BufferSize,
                &EfiVarstore);
  if (EFI_ERROR(Status)) {
    ZeroMem(&EfiVarstore, sizeof(EfiVarstore));
    Status = gRT->SetVariable(
                  L"CheckboxValue",
                  &Guid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof(EfiVarstore),
                  &EfiVarstore);
    if (EFI_ERROR(Status)) {
      Print(L"Error! Can't create variable! %r\n", Status);
    }
  }

  ...

}
```

Check how variable is now created on driver load:
```
FS0:\> dmpstore -guid ef2acc91-7b50-4ab9-ab67-2b04f8bc135e
dmpstore: No matching variables found. Guid EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E
FS0:\> load HIIFormCheckbox.efi
Image 'FS0:\HIIFormCheckbox.efi' loaded at 687C000 - Success
FS0:\> dmpstore -guid ef2acc91-7b50-4ab9-ab67-2b04f8bc135e
Variable NV+BS 'EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E:CheckboxValue' DataSize = 0x01
  00000000: 00                                               *.*
```

You can restart OVMF and check that now the variable present in the system even without a loaded driver:
```
FS0:\> dmpstore -guid ef2acc91-7b50-4ab9-ab67-2b04f8bc135e
Variable NV+BS 'EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E:CheckboxValue' DataSize = 0x01
  00000000: 00                                               *.*
```

But our driver is not included in the boot process, it is an optional driver. So if the user truly wants to remove all the things connected to the driver we can add code to the `HIIFormCheckboxUnload` for the UEFI variable deletion:
```
EFI_STATUS
EFIAPI
HIIFormCheckboxUnload (
  EFI_HANDLE ImageHandle
  )
{
  if (mHiiHandle != NULL)
    HiiRemovePackages(mHiiHandle);

  EFI_STATUS Status;
  EFI_GUID Guid = FORMSET_GUID;
  UINTN BufferSize;
  UINT8 EfiVarstore;

  BufferSize = sizeof(UINT8);
  Status = gRT->GetVariable(
                L"CheckboxValue",
                &Guid,
                NULL,
                &BufferSize,
                &EfiVarstore);
  if (!EFI_ERROR(Status)) {
    Status = gRT->SetVariable(
                  L"CheckboxValue",
                  &Guid,
                  0,
                  0,
                  NULL);
    if (EFI_ERROR(Status)) {
      Print(L"Error! Can't delete variable! %r\n", Status);
    }
  }

  return EFI_SUCCESS;
}
```

Let's verify our code. For the driver unload we can use shell `unload` command.
```
FS0:\> unload -?
Unloads a driver image that was already loaded.

UNLOAD [-n] [-v|-verbose] Handle

  -n           - Skips all prompts during unloading, so that it can be used
                 in a script file.
  -v, -verbose - Dumps verbose status information before the image is unloaded.
  Handle       - Specifies the handle of driver to unload, always taken as hexadecimal number.

NOTES:
  1. The '-n' option can be used to skip all prompts during unloading.
  2. If the '-v' option is specified, verbose image information will be
     displayed before the image is unloaded.
  3. Only drivers that support unloading can be successfully unloaded.
  4. Use the 'LOAD' command to load a driver.

EXAMPLES:
  * To find the handle for the UEFI driver image to unload:
    Shell> dh -b

  * To unload the UEFI driver image with handle 27:
    Shell> unload 27
```

As you can see from its help message we need to find the driver handle to unload it. And to do this we can use `dh` command. If you'll execute it after the driver load you'll see something like this at the end of the output:
```
FS0:\> dh
...
A9: ImageDevicePath(..xFBFC1)/\HIIFormCheckbox.efi) LoadedImage(\HIIFormCheckbox.efi)
```
So nn this particular case the driver handle number is `A9`:
```
FS0:\> unload A9
Unload - Handle [688E218].  [y/n]?
y
Unload - Handle [688E218] Result Success.
```

After that UEFI variable would be deleted:
```
FS0:\> dmpstore -guid ef2acc91-7b50-4ab9-ab67-2b04f8bc135e
dmpstore: No matching variables found. Guid EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E
```

Ok, all works fine, but what is about our form? Unfortunately this is not enough for the checkbox to function correctly. The form wouldn't let to submit itself with the same error message. We need to do one more thing.

# Add Device Path

If you'll look at the output of our `ShowHII.efi` application you might notice that every `PackageList` that has a `FORMS` package also has a `DEVICE_PATH` package. For example:
```
PackageList[9]: GUID=D9DCC5DF-4007-435E-9098-8970935504B2; size=0x855
        Package[0]: type=FORMS; size=0x1F6
        Package[1]: type=STRINGS; size=0x62B
        Package[2]: type=DEVICE_PATH; size=0x1C
        Package[3]: type=END; size=0x4
```
This is not an accident. It is necessary to provide device path along with a form for the HII subsystem to function correctly. This is the last thing that we need to do for our checkbox form.

As you might remember there are several possible types of the DevicePath nodes:
```
typedef struct {
  UINT8    Type;    ///< 0x01 Hardware Device Path.
                    ///< 0x02 ACPI Device Path.
                    ///< 0x03 Messaging Device Path.
                    ///< 0x04 Media Device Path.
                    ///< 0x05 BIOS Boot Specification Device Path.
                    ///< 0x7F End of Hardware Device Path.

  UINT8    SubType; ///< Varies by Type

  UINT8    Length[2];
} EFI_DEVICE_PATH_PROTOCOL;
```
And every type has several subtypes. A combination of the `type + subtype` defines a structure of an actual data that follows. All the data formatting is strictly defined by the UEFI spec. But some types leave a possibility for the vendor to provide its own custom structures in the Device Path. Off course edk2 lib wouldn't be able to print your special path, but at least it wouldn't fail on it, since the library can simply skip the unknown DeviceNode using the `EFI_DEVICE_PATH_PROTOCOL.Length` field.

All this is possible because of special SubTypes like:
```
#define HW_VENDOR_DP  0x04	// vendor subtype for the Hardware Device Path type

#define MSG_VENDOR_DP  0x0a     // vendor subtype for the Messaging Device Path type

#define MEDIA_VENDOR_DP  0x03   // vendor subtype for the Media Device Path type
```

In every case vendor supplies special GUID that would define a data that follows (https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/DevicePath.h):
```
///
/// The Vendor Device Path allows the creation of vendor-defined Device Paths. A vendor must
/// allocate a Vendor GUID for a Device Path. The Vendor GUID can then be used to define the
/// contents on the n bytes that follow in the Vendor Device Path node.
///
typedef struct {
  EFI_DEVICE_PATH_PROTOCOL    Header;
  ///
  /// Vendor-assigned GUID that defines the data that follows.
  ///
  EFI_GUID                    Guid;
  ///
  /// Vendor-defined variable size data.
  ///
} VENDOR_DEVICE_PATH;
```

Device paths for forms in the edk2 codebase are represented with this method. Usually `HARDWARE_DEVICE_PATH` and `HW_VENDOR_DP` are used as Type and SubType for such vendor node.

With this in mind let's create our own device path. Our special device node would have `Guid = FORMSET_GUID`. This is not mandatory, our path can have its own GUID, I just don't want to produce unnecessary amount of GUIDs.

Our special GUID would define a Device Path node with no valuable data, so we can statically define full Device Path structure as this:
```
#pragma pack(1)
///
/// HII specific Vendor Device Path definition.
///
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} HII_VENDOR_DEVICE_PATH;
#pragma pack()

HII_VENDOR_DEVICE_PATH  mHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    FORMSET_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};
```


Now we need to install `EFI_DEVICE_PATH_PROTOCOL` to our driver. As you might remember `InstallProtocolInterface` is now considered obsolete, so let's use `InstallMultipleProtocolInterfaces`.

API remainder:
```
EFI_BOOT_SERVICES.InstallMultipleProtocolInterfaces()

Summary:
Installs one or more protocol interfaces into the boot services environment.

Prototype:
typedef
EFI_STATUS
EFIAPI *EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES) (
 IN OUT EFI_HANDLE *Handle,
 ...
 );

Parameters:
Handle 		The pointer to a handle to install the new protocol interfaces on, or a pointer to NULL if a new handle is to be
		allocated.
...		A variable argument list containing pairs of protocol GUIDs and protocol interfaces.

Description:
This function installs a set of protocol interfaces into the boot services environment. It removes
arguments from the variable argument list in pairs. The first item is always a pointer to the protocol’s
GUID, and the second item is always a pointer to the protocol’s interface. These pairs are used to call the
boot service EFI_BOOT_SERVICES.InstallProtocolInterface() to add a protocol interface to
Handle. If Handle is NULL on entry, then a new handle will be allocated. The pairs of arguments are
removed in order from the variable argument list until a NULL protocol GUID value is found.
```

So let's declare `EFI_HANDLE mDriverHandle` for our driver and install our device path to it. 
```
EFI_HANDLE      mDriverHandle = NULL;

EFI_STATUS
EFIAPI
HIIFormCheckboxEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;

  Status = gBS->InstallMultipleProtocolInterfaces(
                  &mDriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ...

}
```

Don't forget to uninstall protocol in the driver unload:
```
EFI_STATUS
EFIAPI
HIIFormCheckboxUnload (
  EFI_HANDLE ImageHandle
  )
{
  ...

  Status = gBS->UninstallMultipleProtocolInterfaces(
              mDriverHandle,
              &gEfiDevicePathProtocolGuid,
              &mHiiVendorDevicePath,
              NULL
              );

  return Status;
}
```

Let's verify that everything work correctly. With the help of `dh` command we can see that now our driver creates another handle with a `DevicePath` protocol installed:
```
FS0:\> load HIIFormCheckbox.efi
Image 'FS0:\HIIFormCheckbox.efi' loaded at 6887000 - Success
FS0:\> dh
...
A8: Shell ShellParameters SimpleTextOut ImageDevicePath(..9E3E-4F1C-AD65-E05268D0B4D1)) LoadedImage(Shell)
A9: ImageDevicePath(..xFBFC1)/\HIIFormCheckbox.efi) LoadedImage(\HIIFormCheckbox.efi)
AA: DevicePath(..7B50-4AB9-AB67-2B04F8BC135E))
```
And on driver unload both handles uninstalled successfully:
```
FS0:\> unload a9
Unload - Handle [688D818].  [y/n]?
y
Unload - Handle [688D818] Result Success.
FS0:\> dh
...
A8: Shell ShellParameters SimpleTextOut ImageDevicePath(..9E3E-4F1C-AD65-E05268D0B4D1)) LoadedImage(Shell)
```


Now we need to actually add `Device Path Package` to the HII Database. Remember Packages and Package Lists in the HII Database?
```
typedef struct _EFI_HII_DEVICE_PATH_PACKAGE {
 EFI_HII_PACKAGE_HEADER Header;
//EFI_DEVICE_PATH_PROTOCOL DevicePath[];
} EFI_HII_DEVICE_PATH_PACKAGE;

Header 		The standard package header, where Header.Type = EFI_HII_PACKAGE_DEVICE_PATH.
DevicePath	The Device Path description associated with the driver handle that provided the content sent to the HII database.
```

Fortunately the `HiiAddPackages` from the `HiiLib` that we constantly use to register forms and strings can easily be used to install Device Path as well. All we need to do is to provide `EFI_HANDLE` with the `EFI_DEVICE_PATH_PROTOCOL` installed (which we already have created).

Take a look at `HiiAddPackages` description again and pay attention to the highlighted argument:

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Library/HiiLib.h

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiHiiLib/HiiLib.c

```
/**
  Registers a list of packages in the HII Database and returns the HII Handle
  associated with that registration.  If an HII Handle has already been registered
  with the same PackageListGuid and DeviceHandle, then NULL is returned.  If there
  are not enough resources to perform the registration, then NULL is returned.
  If an empty list of packages is passed in, then NULL is returned.  If the size of
  the list of package is 0, then NULL is returned.
  The variable arguments are pointers that point to package headers defined
  by UEFI VFR compiler and StringGather tool.
  #pragma pack (push, 1)
  typedef struct {
    UINT32                  BinaryLength;
    EFI_HII_PACKAGE_HEADER  PackageHeader;
  } EDKII_AUTOGEN_PACKAGES_HEADER;
  #pragma pack (pop)
  @param[in]  PackageListGuid  The GUID of the package list.
  @param[in]  DeviceHandle     If not NULL, the Device Handle on which				<---------
                               an instance of DEVICE_PATH_PROTOCOL is installed.
                               This Device Handle uniquely defines the device that
                               the added packages are associated with.
  @param[in]  ...              The variable argument list that contains pointers
                               to packages terminated by a NULL.
  @retval NULL   An HII Handle has already been registered in the HII Database with
                 the same PackageListGuid and DeviceHandle.
  @retval NULL   The HII Handle could not be created.
  @retval NULL   An empty list of packages was passed in.
  @retval NULL   All packages are empty.
  @retval Other  The HII Handle associated with the newly registered package list.
**/
EFI_HII_HANDLE
EFIAPI
HiiAddPackages (
  IN CONST EFI_GUID    *PackageListGuid,
  IN       EFI_HANDLE  DeviceHandle  OPTIONAL,
  ...
  )
;
```

Before that we've always used `NULL` in place of the `DeviceHandle` argument. So all we need to do now is to make this patch:
```
mHiiHandle = HiiAddPackages(
               &gEfiCallerIdGuid,
-              NULL,
+              mDriverHandle,
               HIIFormCheckboxStrings,
               FormBin,
               NULL
               );
```

Now our form should work correctly.

But before testing let's do one more thing. Our static device path includes `FORMSET_GUID` in itself. So we can simplify `GetVariable`/`SetVariable` code a little bit. Instead of creating and referencing local `EFI_GUID Guid = FORMSET_GUID` variables we can simple use `&mHiiVendorDevicePath.VendorDevicePath.Guid`:

```
- EFI_GUID Guid = FORMSET_GUID;

  Status = gRT->GetVariable(
              L"CheckboxValue",
-             &Guid,
+             &mHiiVendorDevicePath.VendorDevicePath.Guid,
              NULL,
              &BufferSize,
              &EfiVarstore);
```

Ok, now it is time to build and verify that everything works correctly now.
```
FS0:\> load HIIFormCheckbox.efi
Image 'FS0:\HIIFormCheckbox.efi' loaded at 683A000 - Success
FS0:\> DisplayHIIByGuid.efi 771A4631-43BA-4852-9593-919D9DE079F1
```

By default checkbox would be unchecked. But now you can successfully modify it and save it.
You can verify that your change is persistent. If you reboot OVMF and load our driver once again you'll see that now form starts with the checkbox in a checked state.

![CheckboxWithVarstore5](CheckboxWithVarstore5.png?raw=true "CheckboxWithVarstore5")

But if you unload our driver manually the code in our application would delete UEFI variable. So when you load it again the checkbox would in the default empty state.

![CheckboxWithVarstore1](CheckboxWithVarstore1.png?raw=true "CheckboxWithVarstore1")
