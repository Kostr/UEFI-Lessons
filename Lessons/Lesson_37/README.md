In this lesson we'll learn about library constructor and destructor.

Create a new library module `UefiLessonsPkg/Library/SimpleLibraryWithConstructor/SimpleLibraryWithConstructor.inf`:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = SimpleLibraryWithConstructor
  FILE_GUID                      = 96952c1e-86a6-4700-96b0-e7303ac3f92d
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  LIBRARY_CLASS                  = SimpleLibrary | UEFI_APPLICATION
  CONSTRUCTOR                    = SimpleLibraryConstructor                  <-------- 

[Sources]
  SimpleLibraryWithConstructor.c

[Packages]
  MdePkg/MdePkg.dec
  UefiLessonsPkg/UefiLessonsPkg.dec
```
Here we've added `CONSTRUCTOR` statement with a name of constructor function. Let's add `Print` statement to it, to know when it is executed.

`UefiLessonsPkg/Library/SimpleLibraryWithConstructor/SimpleLibraryWithConstructor.c`:
```
#include <Library/UefiLib.h>
#include <Library/SimpleLibrary.h>

UINTN Plus2(UINTN number) {
  return number+2;
}

EFI_STATUS
EFIAPI
SimpleLibraryConstructor(
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  Print(L"Hello from library constructor!\n");
  return EFI_SUCCESS;
}
```

Now we don't need to create another app that would use our new lib, we can simply change library implementation in the `UefiLessonsPkg/UefiLessonsPkg.dsc` and our `SimpleLibraryUser` would be recompiled with our new library version:
```
[LibraryClasses]
  ...
  #SimpleLibrary|UefiLessonsPkg/Library/SimpleLibrary/SimpleLibrary.inf
  SimpleLibrary|UefiLessonsPkg/Library/SimpleLibraryWithConstructor/SimpleLibraryWithConstructor.inf
```

If you build our app and execute it under OVMF now you would get:
```
FS0:\> SimpleLibraryUser.efi
Hello from library constructor!
5
```

An example of a library that uses constructor would be `UefiBootServicesTableLib` library https://github.com/tianocore/edk2/blob/master/MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf. We've used it all over in our lessons, so let's take a look at it.

As a matter of fact, constructor is the only thing that this library has.

To understand how this library works take a look at its *.c and *.h files:

https://github.com/tianocore/edk2/blob/master/MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.c
```
EFI_HANDLE         gImageHandle = NULL;
EFI_SYSTEM_TABLE   *gST         = NULL;
EFI_BOOT_SERVICES  *gBS         = NULL;

EFI_STATUS
EFIAPI
UefiBootServicesTableLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  gImageHandle = ImageHandle;
  gST = SystemTable;
  gBS = SystemTable->BootServices;

  return EFI_SUCCESS;
}
```
https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/UefiBootServicesTableLib.h:
```
extern EFI_HANDLE         gImageHandle;
extern EFI_SYSTEM_TABLE   *gST;
extern EFI_BOOT_SERVICES  *gBS;
```

So as you can see this library just sets some global variables - shortcuts for the UEFI main parts. As the library constructors execute before the main app code, with this library you can access `gImageHandle`/`gST`/`gBS` anywhere in your app code from the very start.

# DESTRUCTOR

Similar we can create another version of a `SimpleLibrary` that would have both constructor and destructor.

`UefiLessonsPkg/Library/SimpleLibraryWithConstructorAndDestructor/SimpleLibraryWithConstructorAndDestructor.inf`:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = SimpleLibraryWithConstructorAndDestructor
  FILE_GUID                      = 96952c1e-86a6-4700-96b0-e7303ac3f92d
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  LIBRARY_CLASS                  = SimpleLibrary | UEFI_APPLICATION
  CONSTRUCTOR                    = SimpleLibraryConstructor
  DESTRUCTOR                     = SimpleLibraryDestructor                      <-----------

[Sources]
  SimpleLibraryWithConstructorAndDestructor.c

[Packages]
  MdePkg/MdePkg.dec
  UefiLessonsPkg/UefiLessonsPkg.dec
```
`UefiLessonsPkg/Library/SimpleLibraryWithConstructorAndDestructor/SimpleLibraryWithConstructorAndDestructor.c`:
```
#include <Library/UefiLib.h>
#include <Library/SimpleLibrary.h>

UINTN Plus2(UINTN number) {
  return number+2;
}

EFI_STATUS
EFIAPI
SimpleLibraryConstructor(
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  Print(L"Hello from library constructor!\n");
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SimpleLibraryDestructor(
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  Print(L"Hello from library destructor!\n");
  return EFI_SUCCESS;
}
```

Don't forget to change the library backend in the `UefiLessonsPkg/UefiLessonsPkg.dsc`:
```
[LibraryClasses]
  ...
  #SimpleLibrary|UefiLessonsPkg/Library/SimpleLibrary/SimpleLibrary.inf
  #SimpleLibrary|UefiLessonsPkg/Library/SimpleLibraryWithConstructor/SimpleLibraryWithConstructor.inf
  SimpleLibrary|UefiLessonsPkg/Library/SimpleLibraryWithConstructorAndDestructor/SimpleLibraryWithConstructorAndDestructor.inf
```

Now our app would have print strings both at the beginning and in the end:
```
FS0:\> SimpleLibraryUser.efi
Hello from library constructor!
5
Hello from library destructor!
```


As an example of a library with both CONSTRUCTOR and DESTRUCTOR take a look at the https://github.com/tianocore/edk2/blob/master/MdePkg/Library/UefiDebugLibConOut/DebugLibConstructor.c, it uses `CreateEvent` in a constructor, and closes it in a desctrutor with a `CloseEvent`.


# `NULL` library


As you already know OVMF includes `Shell` app in itself. For its compilation OVMF package DSC file (https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkgX64.dsc) contains these strings:
```
[Components]
  ...
  ShellPkg/Application/Shell/Shell.inf {
    <LibraryClasses>
      ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
      NULL|ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
      ...
      HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
      PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
      BcfgCommandLib|ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf

      ... 
  }
```

Here you can notice that some of the library classes have `NULL` class.

`NULL` library classes are conceptually an "anonymous library". It enables one to statically link code into a module even if the module doesn't directly call functions in that library.

It can be useful if we don't need to call library API in our app, but just need library constructor/destructor functions.

If you'll take a look at the file https://github.com/tianocore/edk2/blob/master/ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.c you'll see that this module constructor is used to add additional commands to the Shell:
```
EFI_STATUS
EFIAPI
ShellLevel1CommandsLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
 ...
  ShellCommandRegisterCommandName(L"stall",  ShellCommandRunStall   , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_STALL) ));
  ShellCommandRegisterCommandName(L"for",    ShellCommandRunFor     , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_FOR)   ));
  ShellCommandRegisterCommandName(L"goto",   ShellCommandRunGoto    , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_GOTO)  ));
  ShellCommandRegisterCommandName(L"if",     ShellCommandRunIf      , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_IF)    ));
  ShellCommandRegisterCommandName(L"shift",  ShellCommandRunShift   , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_SHIFT) ));
  ShellCommandRegisterCommandName(L"exit",   ShellCommandRunExit    , ShellCommandGetManFileNameLevel1, 1, L"", TRUE , gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_EXIT)  ));
  ShellCommandRegisterCommandName(L"else",   ShellCommandRunElse    , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_ELSE)  ));
  ShellCommandRegisterCommandName(L"endif",  ShellCommandRunEndIf   , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_ENDIF) ));
  ShellCommandRegisterCommandName(L"endfor", ShellCommandRunEndFor  , ShellCommandGetManFileNameLevel1, 1, L"", FALSE, gShellLevel1HiiHandle, (EFI_STRING_ID)(PcdGet8(PcdShellSupportLevel) < 3 ? 0 : STRING_TOKEN(STR_GET_HELP_ENDFOR)));

  return (EFI_SUCCESS);
```

This is an elegant way to split shell command support to different modules.
With this functionality you can easily compile your image of `Shell` with a necessary commands support.

