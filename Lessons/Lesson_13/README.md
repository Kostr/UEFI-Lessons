In previous lesson we've used `EFI_SHELL_PARAMETERS_PROTOCOL` to get command line parameters to our app.
It is a valid method but there is a simpler way to do it for the shell apps.

To ease things we can use the entry point in this format:
https://github.com/tianocore/edk2/blob/master/ShellPkg/Include/Library/ShellCEntryLib.h
```
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  );
```
This way `Argc`/`Argv` are passed directly to the app entry point like it is usually happening in C programming.

Let's create a `SimpleShellApp` based on our `HelloWorld` app.

With the necessary modifications the INF file would look like this:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = SimpleShellApp
  FILE_GUID                      = 2afd1202-545e-4f8d-b8fb-bc179e84ddc8
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = ShellCEntryLib

[Sources]
  SimpleShellApp.c

[Packages]
  MdePkg/MdePkg.dec

[LibraryClasses]
  UefiApplicationEntryPoint
  UefiLib
  ShellCEntryLib
```
Main changes:
- `ENTRY_POINT = ShellCEntryLib` is added to the `[Defines]` section
- `ShellCEntryLib` is added to the `[LibraryClasses]` section

In the end it works this way. Shell C library is the main UEFI app with the standard entry point:
```
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
```
It parses incoming parameters with the `EFI_SHELL_PARAMETERS_PROTOCOL` like we did it and then calls our app entry point passing all of the parsed parameteres with the call:
```
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  );
```
You can look at the source code of `ShellAppMain` at the https://github.com/tianocore/edk2/blob/master/ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.c

If you look at the source you could see that istead of a `HandleProtocol` API that we've used:
```
Status = gBS->HandleProtocol(
  ImageHandle,
  &gEfiShellParametersProtocolGuid,
  (VOID **) &ShellParameters
);
```
it uses `OpenProtocol` API: (I've modified a code a little bit to make it comparable to our version)
```
Status = gBS->OpenProtocol(
  ImageHandle,
  &gEfiShellParametersProtocolGuid,
  (VOID **)&ShellParameters,
  ImageHandle,
  NULL,
  EFI_OPEN_PROTOCOL_GET_PROTOCOL
);
```
According to the UEFI spec `HandleProtocol` API is outdated and `OpenProtocol` should be used instead.
`OpenProtocol` API is a more general call that can cover more cases. It all would matter when you start develop UEFI drivers. You can read UEFI spec for more information. Right now just accept a fact that for the UEFI app these two calls are the same.

Let's go back to our code. To find the necessary `ShellCEntryLib` library class search as usual:
```
$ grep ShellCEntryLib -r ./ --include=*.inf | grep LIBRARY_CLASS
./ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf:  LIBRARY_CLASS                  = ShellCEntryLib|UEFI_APPLICATION UEFI_DRIVER
```

Add this library class to our `UefiLessonsPkg/UefiLessonsPkg.dsc`:
```
ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
```
Don't forget also to add our new app under the `[Components]` section:
```
UefiLessonsPkg/SimpleShellApp/SimpleShellApp.inf
```


Now let's look at the `*.c` file.

We can't use our first print method as `SystemTable` now is unavailable, but we can simply use `SystemTable` with the help of `gST` pointer:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

INTN EFIAPI ShellAppMain(IN UINTN Argc, IN CHAR16 **Argv)
{
  Print(L"Argc=%d\n", Argc);
//  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello World!\n");
  gST->ConOut->OutputString(gST->ConOut, L"Hello again!\n");
  Print(L"Bye!\n");
  return 0;
}
```

Let's add some parameters handling code:
```
for (UINTN i=Argc; i>0; i--) {
  Print(L"Arg[%d]=%s\n", Argc-i, Argv[Argc-i]);
}
```

If we test our app under OVMF we would get:
```
FS0:\> SimpleShellApp.efi kkk ggg
Hello again!
Bye!
Arg[0]=FS0:\SimpleShellApp.efi
Arg[1]=kkk
Arg[2]=ggg
FS0:\> SimpleShellApp.efi
Hello again!
Bye!
Arg[0]=FS0:\SimpleShellApp.efi
```
