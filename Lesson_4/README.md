In this lesson we keep working on our 'Hello World' app.

It is kinda tedious to write such a long string for a simple printf:
```
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello World!\n");
```

So let's try to simplify it.

First in UEFI/edk2 you would be using a lot this `SystemTable` pointer as well as its filed `SystemTable->BootServices` and another main function parameter `ImageHandle`.

So simplify access to these variables edk2 library create global defines for them gST, gBS, gImageHandle.

You could look at the source of the UefiBootServicesTableLib.c:

https://github.com/tianocore/edk2/blob/master/MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.c

https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/UefiBootServicesTableLib.h

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
  //
  // Cache the Image Handle
  //
  gImageHandle = ImageHandle;
  ASSERT (gImageHandle != NULL);

  //
  // Cache pointer to the EFI System Table
  //
  gST = SystemTable;
  ASSERT (gST != NULL);

  //
  // Cache pointer to the EFI Boot Services Table
  //
  gBS = SystemTable->BootServices;
  ASSERT (gBS != NULL);

  return EFI_SUCCESS;
}
```

Let's try to use them in our function:

```
gST->ConOut->OutputString(gST->ConOut, L"Hello again!\n");
```

And try to recompile:
```
$ build --platform=UefiLessonsPkg/UefiLessonsPkg.dsc \
        --module=UefiLessonsPkg/HelloWorld/HelloWorld.inf \
        --arch=X64 \
        --buildtarget=RELEASE --tagname=GCC5
```

Unfortunately this will not compile:
```
/home/kostr/edk2/UefiLessonsPkg/HelloWorld/HelloWorld.c: In function ‘UefiMain’:
/home/kostr/edk2/UefiLessonsPkg/HelloWorld/HelloWorld.c:12:3: error: ‘gST’ undeclared (first use in this function)
   12 |   gST->ConOut->OutputString(gST->ConOut, L"Hello again!\n");
      |   ^~~
```

This error message is familiar, we'are missing some `#include` in our file, so let's add it:
```
#include <Library/UefiBootServicesTableLib.h>
```

This library is already included in our *.dsc file, so we are good to go:
```
[LibraryClasses]
  ...
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  ...
```

Our source code file at this point is:
```
// for gBS
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello World!\n");
  gST->ConOut->OutputString(gST->ConOut, L"Hello again!\n");
  return EFI_SUCCESS;
}
```

Test for the successful compilation:
```
$ build --platform=UefiLessonsPkg/UefiLessonsPkg.dsc \
        --module=UefiLessonsPkg/HelloWorld/HelloWorld.inf \
        --arch=X64 \
        --buildtarget=RELEASE --tagname=GCC5
```

In the case of printf we can simplify things even more. Print is such a common function, that there is a library function for it in edk2. Add this to our file:
```
Print(L"Bye!\n");
```

If we try to recompile we would get error message about missing `#include` once again:
```
/home/kostr/tiano/edk2/UefiLessonsPkg/HelloWorld/HelloWorld.c: In function ‘UefiMain’:
/home/kostr/tiano/edk2/UefiLessonsPkg/HelloWorld/HelloWorld.c:13:3: error: implicit declaration of function ‘Print’ [-Werror=implicit-function-declaration]
   13 |   Print(L"Bye!\n");
      |   ^~~~~
```

In this case header and source file for the `Print` function are:
- https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/UefiLib.h
- https://github.com/tianocore/edk2/blob/master/MdePkg/Library/UefiLib/UefiLibPrint.c

So let's include `UefiLib.h` in our file:
```
#include <Library/UefiLib.h>
```
And try to recompile.
```
$ build --platform=UefiLessonsPkg/UefiLessonsPkg.dsc \
        --module=UefiLessonsPkg/HelloWorld/HelloWorld.inf \
        --arch=X64 \
        --buildtarget=RELEASE --tagname=GCC5
```

Build would fail with a message:
```
/home/kostr/edk2/MdePkg/Library/BasePrintLib/PrintLibInternal.c:821: undefined reference to `Print'
collect2: error: ld returned 1 exit status
make: *** [GNUmakefile:307: /home/kostr/edk2/Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HelloWorld/HelloWorld/DEBUG/HelloWorld.dll] Error 1
```

Compilation step was successful, but the linker have failed. This usually means that *.h file was in place, but the actual library wasn't linked.

```
$ grep UefiLib -r ./ --include=*.inf | grep LIBRARY_CLASS
./MdePkg/Library/UefiLib/UefiLib.inf:  LIBRARY_CLASS                  = UefiLib|DXE_CORE DXE_DRIVER DXE_RUNTIME_DRIVER DXE_SMM_DRIVER UEFI_APPLICATION UEFI_DRIVER SMM_CORE
```

Add this to our `UefiLessonsPkg/UefiLessonsPkg.dsc` file:
```
[LibraryClasses]
  ...
+ UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
```
And this string to our `UefiLessonsPkg/HelloWorld/HelloWorld.inf` file:
```
[LibraryClasses]
  UefiApplicationEntryPoint
+ UefiLib
```

If we try to build, we will get `Failed` once again:
```
/home/kostr/edk2/UefiLessonsPkg/UefiLessonsPkg.dsc(...): error 4000: Instance of library class [MemoryAllocationLib] is not found
        in [/home/kostr/edk2/MdePkg/Library/UefiLib/UefiLib.inf] [X64]
        consumed by module [/home/kostr/edk2/UefiLessonsPkg/HelloWorld/HelloWorld.inf]
```

Try to find it:
```
$ grep MemoryAllocationLib -r ./ --include=*.inf | grep LIBRARY_CLASS | grep MdePkg
./MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf:  LIBRARY_CLASS                  = MemoryAllocationLib|PEIM PEI_CORE SEC
./MdePkg/Library/SmmMemoryAllocationLib/SmmMemoryAllocationLib.inf:  LIBRARY_CLASS                  = MemoryAllocationLib|DXE_SMM_DRIVER
./MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf:  LIBRARY_CLASS                  = MemoryAllocationLib|DXE_DRIVER DXE_RUNTIME_DRIVER DXE_SMM_DRIVER UEFI_APPLICATION UEFI_DRIVER
```

Add this to our *.dsc file:
```
[LibraryClasses]
  ...
+ MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
```

After that we would get another build failure with the similar message. So we need to repeat this process of finding right library. In the end we would need to add two more libraries:  
```
[LibraryClasses]
  ...
+ DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
+ UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
```

After that finally build would finish successfully.

Test it:
```
cp Build/UefiLessonsPkg/RELEASE_GCC5/X64/HelloWorld.efi ~/UEFI_disk/
$ qemu-system-x86_64 -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
                     -drive format=raw,file=fat:rw:~/UEFI_disk \
                     -nographic \
                     -net none
```
```
UEFI Interactive Shell v2.2
EDK II
UEFI v2.70 (EDK II, 0x00010000)
Mapping table
      FS0: Alias(s):HD0a1:;BLK1:
          PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)/HD(1,MBR,0xBE1AFDFA,0x3F,0xFBFC1)
     BLK0: Alias(s):
          PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
     BLK2: Alias(s):
          PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
Press ESC in 1 seconds to skip startup.nsh or any other key to continue.
Shell> fs0:
FS0:\> HelloWorld.efi
Hello World!
Hello again!
Bye!
FS0:\>
```
