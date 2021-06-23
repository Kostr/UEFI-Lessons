In this lesson we'll try to print all the runtime variables available in our system.

To do it, we need to use `GetNextVariableName()` API function from EFI Runtime Services:
```
GetNextVariableName()

Summary:
Enumerates the current variable names.

Prototype:
typedef
EFI_STATUS
GetNextVariableName (
 IN OUT UINTN *VariableNameSize,
 IN OUT CHAR16 *VariableName,
 IN OUT EFI_GUID *VendorGuid
 );

Parameters:
VariableNameSize 	The size of the VariableName buffer. The size must be large
			enough to fit input string supplied in VariableName buffer.
VariableName 		On input, supplies the last VariableName that was returned by
			GetNextVariableName(). On output, returns the Nullterminated string
			of the current variable.
VendorGuid 		On input, supplies the last VendorGuid that was returned by
			GetNextVariableName(). On output, returns the VendorGuid
			of the current variable.

Description
GetNextVariableName() is called multiple times to retrieve the VariableName and VendorGuid of all
variables currently available in the system. On each call to GetNextVariableName() the previous
results are passed into the interface, and on output the interface returns the next variable name data.
When the entire variable list has been returned, the error EFI_NOT_FOUND is returned.

Note that if EFI_BUFFER_TOO_SMALL is returned, the VariableName buffer was too small for the next
variable. When such an error occurs, the VariableNameSize is updated to reflect the size of buffer
needed. In all cases when calling GetNextVariableName() the VariableNameSize must not exceed the
actual buffer size that was allocated for VariableName. The VariableNameSize must not be smaller the size
of the variable name string passed to GetNextVariableName() on input in the VariableName buffer.

To start the search, a Null-terminated string is passed in VariableName; that is, VariableName is a pointer
to a Null character. This is always done on the initial call to GetNextVariableName(). When
VariableName is a pointer to a Null character, VendorGuid is ignored.


Status Codes Returned:
EFI_SUCCESS 		The function completed successfully.
EFI_NOT_FOUND 		The next variable was not found.
EFI_BUFFER_TOO_SMALL 	The VariableNameSize is too small for the result.
			VariableNameSize has been updated with the size needed to complete the request.
...
```


Earlier we've included `#include <Library/UefiBootServicesTableLib.h>` in our file to get `gST`/`gBS`
To get shortcut for the EFI Runtime Services Table we need to add similar include:
```
#include <Library/UefiRuntimeServicesTableLib.h>
```
You can look at this file (https://github.com/tianocore/edk2/blob/master/MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.c), but it simply adds `extern EFI_RUNTIME_SERVICES  *gRT;` which is filled as `gRT = SystemTable->RuntimeServices;` in a library constructor call https://github.com/tianocore/edk2/blob/master/MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.c

As spec says to start a search we need to create a string with 1 character that equals Null (=0).
To create such a string we will use `AllocateZeroPool` function.
```
UINTN VariableNameSize = sizeof (CHAR16);
CHAR16* VariableName = AllocateZeroPool(sizeof(CHAR16));
```

`AllocateZeroPool` is not a UEFI spec function but simply is a helper from the edk2 MemoryAllocationLib (https://github.com/tianocore/edk2/blob/master/MdePkg/Library/PeiMemoryAllocationLib/MemoryAllocationLib.c):
```
/ **
  Allocates and zeros a buffer of type EfiBootServicesData.
   ...
  @param  AllocationSize        The number of bytes to allocate and zero.

  @return A pointer to the allocated buffer or NULL if allocation fails.
**/

VOID *
EFIAPI
AllocateZeroPool (
  IN UINTN  AllocationSize
  )
```

This string is obviously wouldn't be enough to store variable names, so we definitely would get `EFI_BUFFER_TOO_SMALL` at least on the first call.
In this case we need to reallocate our buffer. We could do it with the help of `ReallocatePool` function.

`RealocatePool` is another helper function from the edk2 MemoryAllocationLib (https://github.com/tianocore/edk2/blob/master/MdePkg/Library/PeiMemoryAllocationLib/MemoryAllocationLib.c):
```
/**
  Reallocates a buffer of type EfiBootServicesData.
   ...
  @param  OldSize        The size, in bytes, of OldBuffer.
  @param  NewSize        The size, in bytes, of the buffer to reallocate.
  @param  OldBuffer      The buffer to copy to the allocated buffer.  This is an optional
                         parameter that may be NULL.
  @return A pointer to the allocated buffer or NULL if allocation fails.
**/

VOID *
EFIAPI
ReallocatePool (
  IN UINTN  OldSize,
  IN UINTN  NewSize,
  IN VOID   *OldBuffer  OPTIONAL
  )
```

Let's create `ListVariables` app and try to print first variable:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

INTN EFIAPI ShellAppMain(IN UINTN Argc, IN CHAR16 **Argv)
{
  EFI_GUID VendorGuid;
  UINTN VariableNameSize = sizeof (CHAR16);
  CHAR16* VariableName = AllocateZeroPool(sizeof(CHAR16));
  if (VariableName == NULL) {
    Print(L"Error on AllocateZeroPool call\n");
    return EFI_OUT_OF_RESOURCES;
  }

  UINTN VariableNameSizeOld = VariableNameSize;
  EFI_STATUS Status = gRT->GetNextVariableName(&VariableNameSize, VariableName, &VendorGuid);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    VariableName = ReallocatePool(VariableNameSizeOld, VariableNameSize, VariableName);
    if (VariableName == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    Status = gRT->GetNextVariableName(&VariableNameSize, VariableName, &VendorGuid);
    if (Status == EFI_SUCCESS) {
      Print(L"%g: %s\n", VendorGuid, VariableName);
    } else {
      Print(L"Error on 'gRT->GetNextVariableName' call: %s\n", Status);
      return Status;
    }
  } else {
    Print(L"Error on 'gRT->GetNextVariableName' call: %s\n", Status);
    return Status;
  }
  FreePool(VariableName);
  return EFI_SUCCESS;
}
```

If we try to execute it under OVMF we would get:
```
FS0:\> ListVariables.efi
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: OsIndicationsSupported
```

Now we need to create a loop to print all the available variables.
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

INTN EFIAPI ShellAppMain(IN UINTN Argc, IN CHAR16 **Argv)
{
  EFI_GUID VendorGuid;
  UINTN VariableNameSize = sizeof (CHAR16);
  CHAR16* VariableName = AllocateZeroPool(sizeof(CHAR16));
  if (VariableName == NULL) {
    rint(L"Error on AllocateZeroPool call\n");
    return EFI_OUT_OF_RESOURCES;
  }

  while (TRUE)
  {
    UINTN VariableNameSizeOld = VariableNameSize;
    EFI_STATUS Status = gRT->GetNextVariableName(&VariableNameSize, VariableName, &VendorGuid);
    if (Status == EFI_SUCCESS) {
      Print(L"%g: %s\n", VendorGuid, VariableName);
    } else if (Status == EFI_BUFFER_TOO_SMALL) {
      VariableName = ReallocatePool(VariableNameSizeOld, VariableNameSize, VariableName);
      if (VariableName == NULL) {
        Print(L"Error on ReallocatePool call\n");
        return EFI_OUT_OF_RESOURCES;
      }
      Status = gRT->GetNextVariableName(&VariableNameSize, VariableName, &VendorGuid);
      if (Status == EFI_SUCCESS) {
        Print(L"%g: %s\n", VendorGuid, VariableName);
      } else {
        Print(L"Error on 'gRT->GetNextVariableName' call: %s\n", Status);
        return Status;
      }
    } else if (Status == EFI_NOT_FOUND) {
      FreePool(VariableName);
      return EFI_SUCCESS;
    } else {
      Print(L"Error on 'gRT->GetNextVariableName' call: %s\n", Status);
      return Status;
    }
  }
  return EFI_SUCCESS;
}
```

Inside `while(TRUE)` loop we continiously run `gRT->GetNextVariableName` function and look for its returned `EFI_STATUS` code:
- `EFI_SUCCESS` means variable is successfully acquired, so we simply print it,
- `EFI_BUFFER_TOO_SMALL` means we need to grow our array with the `RealocatePool` API and call `gRT->GetNextVariableName` again,
- `EFI_NOT_FOUND` means we've parsed all variables and need to stop,
- on another status we simply return with error.


If we now run our program under OVMF we would get something like this:
```
FS0:\> ListVariables.efi
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: OsIndicationsSupported
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: BootOptionSupport
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: LangCodes
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: PlatformLangCodes
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: PlatformRecovery0000
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: ConOutDev
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: ErrOutDev
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: ConInDev
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: BootCurrent
158DEF5A-F656-419C-B027-7A3192C079D2: path
158DEF5A-F656-419C-B027-7A3192C079D2: nonesting
0053D9D6-2659-4599-A26B-EF4536E631A9: cat
0053D9D6-2659-4599-A26B-EF4536E631A9: cd..
0053D9D6-2659-4599-A26B-EF4536E631A9: cd\
0053D9D6-2659-4599-A26B-EF4536E631A9: copy
0053D9D6-2659-4599-A26B-EF4536E631A9: del
0053D9D6-2659-4599-A26B-EF4536E631A9: dir
0053D9D6-2659-4599-A26B-EF4536E631A9: md
0053D9D6-2659-4599-A26B-EF4536E631A9: mem
0053D9D6-2659-4599-A26B-EF4536E631A9: mount
0053D9D6-2659-4599-A26B-EF4536E631A9: move
0053D9D6-2659-4599-A26B-EF4536E631A9: ren
158DEF5A-F656-419C-B027-7A3192C079D2: profiles
158DEF5A-F656-419C-B027-7A3192C079D2: uefishellsupport
158DEF5A-F656-419C-B027-7A3192C079D2: uefishellversion
158DEF5A-F656-419C-B027-7A3192C079D2: uefiversion
158DEF5A-F656-419C-B027-7A3192C079D2: cwd
158DEF5A-F656-419C-B027-7A3192C079D2: debuglasterror
158DEF5A-F656-419C-B027-7A3192C079D2: lasterror
59324945-EC44-4C0D-B1CD-9DB139DF070C: Attempt 1
59324945-EC44-4C0D-B1CD-9DB139DF070C: Attempt 2
59324945-EC44-4C0D-B1CD-9DB139DF070C: Attempt 3
59324945-EC44-4C0D-B1CD-9DB139DF070C: Attempt 4
59324945-EC44-4C0D-B1CD-9DB139DF070C: Attempt 5
59324945-EC44-4C0D-B1CD-9DB139DF070C: Attempt 6
59324945-EC44-4C0D-B1CD-9DB139DF070C: Attempt 7
4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9: InitialAttemptOrder
59324945-EC44-4C0D-B1CD-9DB139DF070C: Attempt 8
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: Boot0000
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: Timeout
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: PlatformLang
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: Lang
04B37FE8-F6AE-480B-BDD5-37D98C5E89AA: VarErrorFlag
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: Key0000
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: Key0001
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: Boot0001
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: Boot0002
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: Boot0003
4C19049F-4137-4DD3-9C10-8B97A83FFDFA: MemoryTypeInformation
5B446ED1-E30B-4FAA-871A-3654ECA36080: 525400123456
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: BootOrder
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: Boot0004
EB704011-1402-11D3-8E77-00A0C969723B: MTC
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: ConOut
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: ConIn
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: ErrOut
```

The most important group is `8BE4DF61-93CA-11D2-AA0D-00E098032B8C`. It stands for `EFI_GLOBAL_VARIABLE` and is defined in UEFI spec. In edk2 you can find it under https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Guid/GlobalVariable.h:
```
#define EFI_GLOBAL_VARIABLE \
  { \
    0x8BE4DF61, 0x93CA, 0x11d2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C } \
  }

extern EFI_GUID gEfiGlobalVariableGuid;
```
Options such as `Boot####` are defined by UEFI specification, therefore they have this predefined GUID = `EFI_GLOBAL_VARIABLE`. If you'll ever want to have your own options, you shouldn't use `EFI_GLOBAL_VARIABLE` for them, but instead create your own to eliminate a possibility of variable name conflicts.


To look where GUID is defined in a edk2 codebase you could use something like this:
```
$ grep -i 4B37FE8 -r ./ --exclude-dir=Build
```

To sort variables by GUID you can simply save output of our program and perform `sort` on it in Linux shell:
```
$ cat guids.txt | sort
```


All the variables grouped by GUID with links to files with GUID definition:

`8BE4DF61-93CA-11D2-AA0D-00E098032B8C` - `gEfiGlobalVariableGuid`

https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Guid/GlobalVariable.h

https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec
```
Boot0000
Boot0001
Boot0002
Boot0003
Boot0004
BootCurrent
BootOptionSupport
BootOrder
ConIn
ConInDev
ConOut
ConOutDev
ErrOut
ErrOutDev
Key0000
Key0001
Lang
LangCodes
OsIndicationsSupported
PlatformLang
PlatformLangCodes
PlatformRecovery0000
Timeout
```

`04B37FE8-F6AE-480B-BDD5-37D98C5E89AA` - `gEdkiiVarErrorFlagGuid`

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Guid/VarErrorFlag.h 

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/MdeModulePkg.dec
```
VarErrorFlag
```

`4C19049F-4137-4DD3-9C10-8B97A83FFDFA` - `gEfiMemoryTypeInformationGuid`

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Guid/MemoryTypeInformation.h

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/MdeModulePkg.dec
```
MemoryTypeInformation
```

`5B446ED1-E30B-4FAA-871A-3654ECA36080` - `gEfiIp4Config2ProtocolGuid`

https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/Ip4Config2.h

https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec
```
525400123456
```


`EB704011-1402-11D3-8E77-00A0C969723B` - `gMtcVendorGuid`

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Guid/MtcVendor.h

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/MdeModulePkg.dec
```
MTC
```

`59324945-EC44-4C0D-B1CD-9DB139DF070C` - `gEfiIScsiInitiatorNameProtocolGuid`

https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/IScsiInitiatorName.h

https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec
```
Attempt 1
Attempt 2
Attempt 3
Attempt 4
Attempt 5
Attempt 6
Attempt 7
Attempt 8
```

`158DEF5A-F656-419C-B027-7A3192C079D2` - `gShellVariableGuid`

https://github.com/tianocore/edk2/blob/master/ShellPkg/Include/Guid/ShellVariableGuid.h

https://github.com/tianocore/edk2/blob/master/ShellPkg/ShellPkg.dec
```
cwd
debuglasterror
lasterror
nonesting
path
profiles
uefishellsupport
uefishellversion
uefiversion
```

`4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9` - `gIScsiConfigGuid`

https://github.com/tianocore/edk2/blob/master/NetworkPkg/Include/Guid/IScsiConfigHii.h

https://github.com/tianocore/edk2/blob/master/NetworkPkg/NetworkPkg.dec
```
InitialAttemptOrder
```

`0053D9D6-2659-4599-A26B-EF4536E631A9` - `gShellAliasGuid`

https://github.com/tianocore/edk2/blob/master/ShellPkg/Include/Guid/ShellAliasGuid.h

https://github.com/tianocore/edk2/blob/master/ShellPkg/ShellPkg.dec
```
cat
cd..
cd\
copy
del
dir
md
mem
mount
move
ren
```

