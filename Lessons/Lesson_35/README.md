In this lesson we will try to create the most simple library.

Usually libraries are present in these directories:
```
<Pkg Name>/Library/<Library Name>/               <---- inf and source files for the library (=library implementation)
<Pkg Name>/Include/Library/                      <---- library headers (=library interface)
```

Create folders for our `SimpleLibrary`:
```
$ mkdir -p UefiLessonsPkg/Library/SimpleLibrary/
$ mkdir -p UefiLessonsPkg/Include/Library/
```

First let's implement the header file, the interface for our library. Our `SimpleLibrary` would contain the only function `Plus2` that would receive a `number` and return a `number+2`.
Therefore the content in the header file (`UefiLessonsPkg/Include/Library/SimpleLibrary.h`) would look like this:
```
UINTN Plus2(UINTN number);
```

No harder the library implementation file `UefiLessonsPkg/Library/SimpleLibrary/SimpleLibrary.c`:
```
#include <Library/SimpleLibrary.h>

UINTN Plus2(UINTN number) {
  return number+2;
}
```

This is really a simple library, it stands to its name!

Now we need to create an INF file for the library `UefiLessonsPkg/Library/SimpleLibrary/SimpleLibrary.inf`:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = SimpleLibrary
  FILE_GUID                      = 826c8951-5bd2-4d72-a9d9-f7ab48684117
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  LIBRARY_CLASS                  = SimpleLibrary | UEFI_APPLICATION

[Sources]
  SimpleLibrary.c

[Packages]
  MdePkg/MdePkg.dec
```

The interesting string here is the:
```
LIBRARY_CLASS = SimpleLibrary | UEFI_APPLICATION
```
It says that this library can only be used in modules with a type `UEFI_APPLICATION`. If you would say here `DXE_DRIVER` and try to link it to some of you UEFI applications, build process would fail. The error message would look like this:
```
build.py...
/home/kostr/tiano/edk2/UefiLessonsPkg/UefiLessonsPkg.dsc(...): error 1001: Module type [UEFI_APPLICATION] is not supported by library instance [/home/kostr/tiano/edk2/UefiLessonsPkg/Library/SimpleLibrary/SimpleLibrary.inf]
        consumed by [/home/kostr/tiano/edk2/UefiLessonsPkg/<path to your app inf file>]
```

Now we need to include our library to our package DSC file `UefiLessonsPkg/UefiLessonsPkg.dsc`, so it would get build on a package build:
```
[Components]
  ...
  UefiLessonsPkg/Library/SimpleLibrary/SimpleLibrary.inf
```

But if you try to build our package now build would fail with a message:
```
/home/kostr/tiano/edk2/UefiLessonsPkg/Library/SimpleLibrary/SimpleLibrary.c:1:10: fatal error: Library/SimpleLibrary.h: No such file or directory
    1 | #include <Library/SimpleLibrary.h>
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~
compilation terminated.
```
The reason of that is a fact that our `UefiLessonsPkg/Include/Library/` folder is not recognized by a build system as a place where headers might be.

To fix it we need to add to our `UefiLessonsPkg/UefiLessonsPkg.dec` file `[Includes]` section:
```
[Includes]
  Include
```

And include this `UefiLessonsPkg/UefiLessonsPkg.dec` file to the library module INF file section `[Packages]`:
```
[Packages]
  MdePkg/MdePkg.dec
  UefiLessonsPkg/UefiLessonsPkg.dec
```

Now we are good, build would succeed.

# SimpleLibraryUser

Now let's create an application that would use our library.

UefiLessonsPkg/SimpleLibraryUser/SimpleLibraryUser.c:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/SimpleLibrary.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print(L"%d\n", Plus2(3));

  return EFI_SUCCESS;
}
```

UefiLessonsPkg/SimpleLibraryUser/SimpleLibraryUser.inf
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = SimpleLibraryUser
  FILE_GUID                      = 22a1f57c-21ca-4011-9133-e3df0d01dace
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = UefiMain

[Sources]
  SimpleLibraryUser.c

[Packages]
  MdePkg/MdePkg.dec
  UefiLessonsPkg/UefiLessonsPkg.dec                  <--- we need to include this for the same reason as in library INF file (for the header search)

[LibraryClasses]
  UefiApplicationEntryPoint
  UefiLib
  SimpleLibrary                                      <--- library is included as usual
```

Now add modifications to the `UefiLessonsPkg/UefiLessonsPkg.dsc`:
```
[LibraryClasses]
  ...
  SimpleLibrary|UefiLessonsPkg/Library/SimpleLibrary/SimpleLibrary.inf

[Components]
  ...
  UefiLessonsPkg/SimpleLibraryUser/SimpleLibraryUser.inf
```
Here we've added implementation for our library class and added our new module to the package components.

If you build everything now and execute it under OVMF, you would get:
```
FS0:\> SimpleLibraryUser.efi
5
```

`3+2` is indeed `5`, so our library works correctly!

