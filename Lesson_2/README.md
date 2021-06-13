In the last lesson to build our app we've included its *.inf file as a component to the other package and have built this package. It is clearly not a good solution and in this lesson we will create our own package.

First lets remove our app from the `OvmfPkg/OvmfPkgX64.dsc` file:
```
 ################################################################################
 [Components]
-  SimplestApp/SimplestApp.inf
   OvmfPkg/ResetVector/ResetVector.inf

```

Then create `UefiLessonsPkg` folder in the edk2 directory and move our app into this folder:
```
mkdir UefiLessonsPkg
mv SimplestApp UefiLessonsPkg/SimplestApp
```

Then we need to create platform description file (DSC) for our newly created package:
```
$ vi UefiLessonsPkg/UefiLessonsPkg.dsc
[Defines]
  PLATFORM_NAME                  = UefiLessonsPkg
  PLATFORM_GUID                  = 3db7270f-ffac-4139-90a4-0ae68f3f8167
  PLATFORM_VERSION               = 0.01
  DSC_SPECIFICATION              = 0x00010006
  OUTPUT_DIRECTORY               = Build/UefiLessonsPkg
  SUPPORTED_ARCHITECTURES        = X64
  BUILD_TARGETS                  = RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[Components]
  UefiLessonsPkg/SimplestApp/SimplestApp.inf
```
Full specification for the Platform Description (DSC) File can be found under https://edk2-docs.gitbook.io/edk-ii-dsc-specification/

Let's try to build our SimplestApp module that is now in our own package:
```
$ . edksetup.sh
$ build --platform=UefiLessonsPkg/UefiLessonsPkg.dsc \
        --module=UefiLessonsPkg/SimplestApp/SimplestApp.inf \
        --arch=X64 \
        --buildtarget=RELEASE \
        --tagname=GCC5
```

Unfortunately the build would fail:
```
build.py...
/home/kostr/edk2/UefiLessonsPkg/UefiLessonsPkg.dsc(...): error 4000: Instance of library class [UefiApplicationEntryPoint] is not found
        in [/home/kostr/edk2/UefiLessonsPkg/SimplestApp/SimplestApp.inf] [X64]
        consumed by module [/home/kostr/edk2/UefiLessonsPkg/SimplestApp/SimplestApp.inf]
```

To fix this we need to add `UefiApplicationEntryPoint` to the `LibraryClasses` section in our `UefiLessonsPkg.dsc` file.
To find necessay include lets search our edk2 codebase:
```
$ grep UefiApplicationEntryPoint -r ./ --include=*.inf | grep LIBRARY_CLASS
./MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf:  LIBRARY_CLASS                  = UefiApplicationEntryPoint|UEFI_APPLICATION
```

Therefore we need to add this strings to our *.dsc file:
```
[LibraryClasses]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
```

Let's try to rebuild:
```
$ build --platform=UefiLessonsPkg/UefiLessonsPkg.dsc \
        --module=UefiLessonsPkg/SimplestApp/SimplestApp.inf \
        --arch=X64 \
        --buildtarget=RELEASE \
        --tagname=GCC5
```

But the build would fail again:
```
build.py...
/home/kostr/tiano/edk2/UefiLessonsPkg/UefiLessonsPkg.dsc(...): error 4000: Instance of library class [UefiBootServicesTableLib] is not found
        in [/home/kostr/tiano/edk2/MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf] [X64]
        consumed by module [/home/kostr/tiano/edk2/UefiLessonsPkg/SimplestApp/SimplestApp.inf]
```

As we see the error message is the same. It seems like `UefiApplicationEntryPoint` module that we've included needs some additional includes. So we search necessary libraries again... and again... and again.
In the end our *.dsc file would be looking like this:
```
[Defines]
  PLATFORM_NAME                  = UefiLessonsPkg
  PLATFORM_GUID                  = 3db7270f-ffac-4139-90a4-0ae68f3f8167
  PLATFORM_VERSION               = 0.01
  DSC_SPECIFICATION              = 0x00010006
  OUTPUT_DIRECTORY               = Build/UefiLessonsPkg
  SUPPORTED_ARCHITECTURES        = X64
  BUILD_TARGETS                  = RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf

[Components]
  UefiLessonsPkg/SimplestApp/SimplestApp.inf
```

After the successful build the result binary would be in a `Build/UefiLessonsPkg/RELEASE_GCC5/X64` folder:
```
$ ls -lh Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
-rw-r--r-- 1 kostr kostr 960 Jun 13 12:47 Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi
```

Let's copy in our `UEFI_disk` folder and run it in OVMF:
```
$ cp Build/UefiLessonsPkg/RELEASE_GCC5/X64/SimplestApp.efi ~/UEFI_disk/
$ qemu-system-x86_64 -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
                     -drive format=raw,file=fat:rw:~/UEFI_disk \
                     -nographic \
                     -net none
```

Hopefully everything would be the same it was earlier:
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
Press ESC in 4 seconds to skip startup.nsh or any other key to continue.
Shell> fs0:
FS0:\> SimplestApp.efi
FS0:\>
```
