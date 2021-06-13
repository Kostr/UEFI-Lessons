In last lesson we've built an app that was already included in the TianoCore sources.

Let's work our way through to our own app.
We will be working on UEFI app that can be run in UEFI shell.

Create directory for our app:
```
mkdir SimplestApp
```

Then create *.c source file for our app with the folowing content:
```
$ cat SimplestApp/SimplestApp.c

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}
```

Then we need to create edk2 app configuration file. This configuration in edk2 is represented in the *.inf format.
INF file can have several sections:
- Defines
- Sources
- Packages
- LibraryClasses
- Guids
- Ppis
- Protocols
- FeaturePcd
- Pcd
- ...

But right now for our minimal example we're interested in the 3 of those:
- Defines - this section contatins some basic module description. In this section:
BASE_NAME - our app name
FILE_GUID - as was said earlier UEFI uses GUID numbers for identification of the components. You could use free online GUID generator to get random GUID https://www.guidgenerator.com/ or simply use `uuidgen` command line utility:
```
$ uuidgen
e7218aab-998e-4d88-af9b-9573a5bf90ea
```
MODULE_TYPE - we want to build an application that can be run from the UEFI shell, so we use `UEFI_APPLICATION` here. UEFI application is like a simple program that you can run from shell. It is getting loaded to some memory address, executes and returns something, after that app memory would be freed again. For example other possible value here is UEFI_DRIVER - the difference is when you load a driver it keeps staying in memory even after its execution.
Other values are listed here: https://edk2-docs.gitbook.io/edk-ii-inf-specification/appendix_f_module_types
ENTRY_POINT - name of the main function in our *.c source file. As it was `UefiMain` this is the value that we write here.

- Sources - source files for our edk2 module
- LibraryClasses - we need to include `UefiApplicationEntryPoint` library class for our minimal example
- Packages - `MdePkg/MdePkg.dec` is edk2 package with basic UEFI services. It includes `UefiApplicationEntryPoint` that we need to compile our package.

vi SimplestApp/SimplestApp.inf
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = SimplestApp
  FILE_GUID                      = 4a298956-fbe0-47fb-ae3a-2d5a0a959a26
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = UefiMain

[Sources]
  SimplestApp.c

[Packages]
  MdePkg/MdePkg.dec

[LibraryClasses]
  UefiApplicationEntryPoint
```


After that we need to include our app to some package.
We don't have our own package, so let's include it to `OvmfPkg/OvmfPkgX64.dsc` that we've compiled earlier.
Add a path to our app *.inf file in the components section.
```
 ################################################################################
 [Components]
+  SimplestApp/SimplestApp.inf
   OvmfPkg/ResetVector/ResetVector.inf

```

Then build OVMF:
```
build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

Our compiled app would be in a directory `Build/OvmfX64/RELEASE_GCC5/X64`:
```
$ ls -lh Build/OvmfX64/RELEASE_GCC5/X64/SimplestApp.efi
-rw-r--r-- 1 kostr kostr 832 Jun 13 10:14 Build/OvmfX64/RELEASE_GCC5/X64/SimplestApp.efi
```

Create a folder that we would populate to UEFI shell and copy our app into it:
```
mkdir ~/UEFI_disk
cp Build/OvmfX64/RELEASE_GCC5/X64/SimplestApp.efi ~/UEFI_disk
```

Now lets run OVMF with this folder included:
```
$ qemu-system-x86_64 -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
                     -drive format=raw,file=fat:rw:~/UEFI_disk \
                     -nographic \
                     -net none
```

Hopefully you'll see something like this:
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
Press ESC in 3 seconds to skip startup.nsh or any other key to continue.
```

As you can see we have new rows in our mapping table. Our app would be under `FS0`. Lets `mount` this filesystem and execute our app.
```
Shell> fs0:
FS0:\> SimplestApp.efi
FS0:\>
```


