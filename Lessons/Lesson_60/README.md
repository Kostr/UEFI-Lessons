Before we start creating HII forms that would work with non-volatile UEFI variables (i.e. persistent across reboots), we need to check UEFI variable services one more time.

Up until now we've only checked already existing variables with the help of `gRT->GetVariable` and `gRT->GetNextVariableName` services.

Let's try to create an application that would create, change and delete our own custom UEFI variables.

For this task we will need `gRT->SetVariable` function:
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
VariableName 	A Null-terminated string that is the name of the vendor’s variable.
		Each VariableName is unique for each VendorGuid.
		VariableName must contain 1 or more characters. If VariableName is an empty string, then
		EFI_INVALID_PARAMETER is returned
VendorGuid 	A unique identifier for the vendor
Attributes 	Attributes bitmask to set for the variable
DataSize 	The size in bytes of the Data buffer ... A size of zero causes the variable to be deleted.
Data 		The contents for the variable
```

Now let's write a code for our application. It would give a user a possibility to create or delete custom UEFI variables. For the simplicity the variable value would be just a string supplied by user. Here is a help message for our application:

```
VOID Usage()
{
  Print(L"Delete variable\n");
  Print(L"   SetVariableExample <variable name>\n");
  Print(L"\n");
  Print(L"Set variable\n");
  Print(L"   SetVariableExample <variable name> <attributes> <value>\n");
  Print(L"\n");
  Print(L"<attributes> can be <n|b|r>\n");
  Print(L"n - NON_VOLATILE\n");
  Print(L"b - BOOTSERVICE_ACCESS\n");
  Print(L"r - RUNTIME_ACCESS\n");
}
```

You can see that when variable is created it is necessary to provide variable attributes. They can be a combination of the following flags:
- `EFI_VARIABLE_NON_VOLATILE` - variable is persistent across reboots
- `EFI_VARIABLE_BOOTSERVICE_ACCESS` - variable is available in the UEFI stage
- `EFI_VARIABLE_RUNTIME_ACCESS` - variable is available in the OS stage (after the successfull `EFI_BOOT_SERVICES.ExitBootServices()` call)

These flags are defined in the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiMultiPhase.h with the several other flags that we wouldn't cover right now.

With these 3 flags only the following combinations are allowed:

| NV | BS | RT |
|---|---|---|
| + | + | + |
| + | + |   |
|   | + |   |
|   | + | + |


Sometimes in the code you could encounter variable attribute combinations like these (https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Guid/VariableFormat.h):
```
///
/// Variable Attribute combinations.
///
#define VARIABLE_ATTRIBUTE_NV_BS           (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS)
#define VARIABLE_ATTRIBUTE_BS_RT           (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS)
#define VARIABLE_ATTRIBUTE_NV_BS_RT        (VARIABLE_ATTRIBUTE_BS_RT | EFI_VARIABLE_NON_VOLATILE)
...
```

But let's get back to our application. We would need to handle command arguments so it is better to create a shell application.

The code is pretty simple.

`UefiLessonsPkg/SetVariableExample/SetVariableExample.c`
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>


VOID Usage()
{
  ...
}

INTN EFIAPI ShellAppMain(IN UINTN Argc, IN CHAR16 **Argv)
{
  EFI_STATUS Status;

  if (Argc==2) {
    CHAR16* VariableName = Argv[1];
    Status = gRT->SetVariable (
                VariableName,
                &gEfiCallerIdGuid,
                0,
                0,
                NULL
                );
    if (EFI_ERROR(Status)) {
      Print(L"%r\n", Status);
    } else {
      Print(L"Variable %s was successfully deleted\n", VariableName);
    }
    return Status;
  } else if (Argc==4) {
    CHAR16* VariableName = Argv[1];
    CHAR16* AttributesStr = Argv[2];
    CHAR16* Value = Argv[3];
    UINT32  Attributes = 0;
    for (UINTN i=0; i<StrLen(AttributesStr); i++) {
      switch(AttributesStr[i]) {
        case L'n':
          Attributes |= EFI_VARIABLE_NON_VOLATILE;
          break;
        case L'b':
          Attributes |= EFI_VARIABLE_BOOTSERVICE_ACCESS;
          break;
        case L'r':
          Attributes |= EFI_VARIABLE_RUNTIME_ACCESS;
          break;
        default:
          Print(L"Error! Unknown attribute!");
          return EFI_INVALID_PARAMETER;
      }
    }
    Status = gRT->SetVariable (
                VariableName,
                &gEfiCallerIdGuid,
                Attributes,
                StrSize(Value),
                Value
                );
    if (EFI_ERROR(Status)) {
      Print(L"%r\n", Status);
    } else {
      Print(L"Variable %s was successfully changed\n", VariableName);
    }
    return Status;
  } else {
    Usage();
  }

  return EFI_SUCCESS;
}
```
`UefiLessonsPkg/SetVariableExample/SetVariableExample.inf`:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = SetVariableExample
  FILE_GUID                      = bb2a829f-7943-4691-a03a-f1f48519d7e6
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = ShellCEntryLib

[Sources]
  SetVariableExample.c

[Packages]
  MdePkg/MdePkg.dec

[LibraryClasses]
  UefiLib
  ShellCEntryLib
```

When `Argc==4`, we treat incoming arguments as inputs for the variable creation. We parse arguments and call `gRT->SetVariable` service.

When `Argc==2`, it means that the user have supplied only the variable name and according to our help message we have to delete this variable. To do it we call `gRT->SetVariable` with a `DataSize` equal to 0.

All the variables would be created under the `gEfiCallerIdGuid` GUID, which in our case means `bb2a829f-7943-4691-a03a-f1f48519d7e6`.

Build our application and copy it to the UEFI shared disk folder.

Don't forget to add `-drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd` to the QEMU arguments:
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk \
  -net none \
  -nographic
```

To look at the currently present UEFI variables we can use `dmpstore` command from the UEFI shell. This command has an option to print only variables associated with a particular guid. If we try to execute it right from the UEFI shell start we wouldn't get anything associated with our GUID:
```
FS0:\> dmpstore -guid bb2a829f-7943-4691-a03a-f1f48519d7e6
dmpstore: No matching variables found. Guid BB2A829F-7943-4691-A03A-F1F48519D7E6
```

Let's try to create a `MyVar` variable with a `Hello` content:
```
FS0:\> SetVariableExample.efi MyVar b "Hello"
Variable MyVar was successfully changed
FS0:\> dmpstore -guid bb2a829f-7943-4691-a03a-f1f48519d7e6
Variable BS 'BB2A829F-7943-4691-A03A-F1F48519D7E6:MyVar' DataSize = 0x0C
  00000000: 48 00 65 00 6C 00 6C 00-6F 00 00 00              *H.e.l.l.o...*
```
You can indeed see that the variable was created.

You can try to change it:
```
FS0:\> SetVariableExample.efi MyVar b "Hello World"
Variable MyVar was successfully changed
FS0:\> dmpstore -guid bb2a829f-7943-4691-a03a-f1f48519d7e6
Variable BS 'BB2A829F-7943-4691-A03A-F1F48519D7E6:MyVar' DataSize = 0x18
  00000000: 48 00 65 00 6C 00 6C 00-6F 00 20 00 57 00 6F 00  *H.e.l.l.o. .W.o.*
  00000010: 72 00 6C 00 64 00 00 00-                         *r.l.d...*
```
Or delete it:
```
FS0:\> SetVariableExample.efi MyVar
Variable MyVar was successfully deleted
FS0:\> dmpstore -guid bb2a829f-7943-4691-a03a-f1f48519d7e6
dmpstore: No matching variables found. Guid BB2A829F-7943-4691-A03A-F1F48519D7E6
```

You can see the proof that not all the combination of attributes are supported:
```
FS0:\> SetVariableExample.efi MyVar nr "Hello"
Invalid Parameter
```


Now let's create two variables with the different attributes: "NV+BS" and "BS"
```
FS0:\> SetVariableExample.efi MyPersistentVar nb "Persistent variable"
Variable MyPersistentVar was successfully changed
FS0:\> SetVariableExample.efi MyVar b "Memory variable"
Variable MyVar was successfully changed
FS0:\> dmpstore -guid bb2a829f-7943-4691-a03a-f1f48519d7e6
Variable NV+BS 'BB2A829F-7943-4691-A03A-F1F48519D7E6:MyPersistentVar' DataSize = 0x28
  00000000: 50 00 65 00 72 00 73 00-69 00 73 00 74 00 65 00  *P.e.r.s.i.s.t.e.*
  00000010: 6E 00 74 00 20 00 76 00-61 00 72 00 69 00 61 00  *n.t. .v.a.r.i.a.*
  00000020: 62 00 6C 00 65 00 00 00-                         *b.l.e...*
Variable BS 'BB2A829F-7943-4691-A03A-F1F48519D7E6:MyVar' DataSize = 0x20
  00000000: 4D 00 65 00 6D 00 6F 00-72 00 79 00 20 00 76 00  *M.e.m.o.r.y. .v.*
  00000010: 61 00 72 00 69 00 61 00-62 00 6C 00 65 00 00 00  *a.r.i.a.b.l.e...*
```
Restart QEMU and issue `dmpstore` command:
```
FS0:\> dmpstore -guid bb2a829f-7943-4691-a03a-f1f48519d7e6
Variable NV+BS 'BB2A829F-7943-4691-A03A-F1F48519D7E6:MyPersistentVar' DataSize = 0x28
  00000000: 50 00 65 00 72 00 73 00-69 00 73 00 74 00 65 00  *P.e.r.s.i.s.t.e.*
  00000010: 6E 00 74 00 20 00 76 00-61 00 72 00 69 00 61 00  *n.t. .v.a.r.i.a.*
  00000020: 62 00 6C 00 65 00 00 00-                         *b.l.e...*
```
As you can see only the `MyPersistentVar` is present.

Also your can check that is not possible to change attributes of an already created variable.
```
FS0:\> SetVariableExample.efi MyPersistentVar b "Persistent variable"
Invalid Parameter
```

`dmpstore` command also can be used to delete variable
```
FS0:\> dmpstore -d -guid bb2a829f-7943-4691-a03a-f1f48519d7e6 MyPersistentVar
Delete variable 'BB2A829F-7943-4691-A03A-F1F48519D7E6:MyPersistentVar': Success
```

You can delete all the variables behind the GUID with a `dmpstore -d <GUID>` command.

In our application the variable content is a simple string. But keep in mind that this is just an example. Often variables have complicated structures stored in them.


