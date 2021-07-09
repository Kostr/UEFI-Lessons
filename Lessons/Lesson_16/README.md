In previous lesson we've developed an app that showed us current boot options:
```
FS0:\> ShowBootVariables.efi
Boot0000
UiApp
Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(462CAA21-7614-4503-836E-8AB6F4662331)

Boot0001
UEFI QEMU DVD-ROM QM00003
PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)

Boot0002
UEFI QEMU HARDDISK QM00001
PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)

Boot0003*
EFI Internal Shell
Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)

Boot0004
UEFI PXEv4 (MAC:525400123456)
PciRoot(0x0)/Pci(0x3,0x0)/MAC(525400123456,0x1)/IPv4(0.0.0.0)
```

If you want to know information about the printed GUIDs here it is:
- `FILE_GUID = 7C04A583-9E3E-4f1c-AD65-E05268D0B4D1 # gUefiShellFileGuid`
https://github.com/tianocore/edk2/blob/master/ShellPkg/Application/Shell/Shell.inf
- `FILE_GUID = 462CAA21-7614-4503-836E-8AB6F4662331` - UiApp module is driver for BDS phase
https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Application/UiApp/UiApp.inf

As for the `7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1` it is a GUID for the firmware volume in our OVMF image.
https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkgX64.fdf

This *.fdf file lists what drivers/apps are placed in every volume in the image (this includes volumes for SEC, PEI or DXE stages): 
```
...

[FV.SECFV]
FvNameGuid         = 763BED0D-DE9F-48F5-81F1-3E90E1B1A015
BlockSize          = 0x1000
FvAlignment        = 16
...
INF <...>
...

[FV.PEIFV]
FvNameGuid         = 6938079B-B503-4E3D-9D24-B28337A25806
BlockSize          = 0x10000
FvAlignment        = 16
...
INF <...>
...

[FV.DXEFV]
FvForceRebase      = FALSE
FvNameGuid         = 7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1        ### <---- this is a GUID from our output
BlockSize          = 0x10000
FvAlignment        = 16
...
INF  ShellPkg/Application/Shell/Shell.inf          <--- this apps are placed in the FV.DXEFV firmware volume
...
INF  MdeModulePkg/Application/UiApp/UiApp.inf
```

For the complete edk2 FDF file specification look at the https://edk2-docs.gitbook.io/edk-ii-fdf-specification/

If you look at the https://github.com/tianocore/edk2/blob/master/ShellPkg/Application/Shell/Shell.inf you'll see that EFI Shell is a simple `UEFI_APPLICATION`. We've developed a lot of those in these lessons, so let's try to add one of our apps to boot options.

We will try to add our `HelloWorld` application to the boot options. We will look at how Shell app is included and try to do the same.

First we need to add our app to OVMF image. So let's add `UefiLessonsPkg/HelloWorld/HelloWorld.inf` to the `[FV.DXEFV]` section next to `INF  ShellPkg/Application/Shell/Shell.inf` in a `OvmfPkg/OvmfPkgX64.fdf b/OvmfPkg/OvmfPkgX64.fdf` we've already discussed.

```
[FV.DXEFV]
FvForceRebase      = FALSE
FvNameGuid         = 7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1
BlockSize          = 0x10000
FvAlignment        = 16
...

  INF  ShellPkg/Application/Shell/Shell.inf
+ INF  UefiLessonsPkg/HelloWorld/HelloWorld.inf
...
```

If we try to build OVMF now we would get:
```
$ build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
...
build.py...
 : error F001: Module /home/kostr/tiano/edk2/UefiLessonsPkg/HelloWorld/HelloWorld.inf NOT found in DSC file; Is it really a binary module?
...
```

Ok, so we need to add information to the `OvmfPkg/OvmfPkgX64.dsc` file. If you look at this file, you'll see that `ShellPkg/Application/Shell/Shell.inf` is listed under `[Components]` section. We've already used it in our first lesson when we've tried to compile our app without package. Add `UefiLessonsPkg/HelloWorld/HelloWorld.inf` next to the Shell INF file.
```
[Components]
  ...
  ShellPkg/Application/Shell/Shell.inf {
   ...
  }
+ UefiLessonsPkg/HelloWorld/HelloWorld.inf
  ...
```
Shell.inf file has some information inside the brackets `{...}`. Don't mind it, we don't need such info for our simple `HelloWorld.inf`.


Now we can compile OVMF without errors:
```
$ build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```
Our `HelloWorld` app would be compiled and embedded in the OVMF image but unfortunately in wouldn't be listed in a boot options.

When we used our `ShowBootVariables.efi` app we saw that UEFI shell boot option had a description `EFI Internal Shell`. Let's try search for that in a edk2 codebase:
```
$ grep "EFI Internal Shell" -r ./ --exclude=Build
./ArmVirtPkg/Library/PlatformBootManagerLib/PlatformBm.c:    &gUefiShellFileGuid, L"EFI Internal Shell", LOAD_OPTION_ACTIVE
./OvmfPkg/Library/PlatformBootManagerLib/BdsPlatform.c:    &gUefiShellFileGuid, L"EFI Internal Shell", LOAD_OPTION_ACTIVE
./OvmfPkg/Library/PlatformBootManagerLibBhyve/BdsPlatform.c:    &gUefiShellFileGuid, L"EFI Internal Shell", LOAD_OPTION_ACTIVE
./OvmfPkg/Library/PlatformBootManagerLibGrub/BdsPlatform.c:    &gUefiShellFileGuid, L"EFI Internal Shell", LOAD_OPTION_ACTIVE
```

For our case we're interested in a `./OvmfPkg/Library/PlatformBootManagerLib/BdsPlatform.c` file:
```
VOID
EFIAPI
PlatformBootManagerAfterConsole (
  VOID
  )
{
  ...
  //
  // Register UEFI Shell
  //
  PlatformRegisterFvBootOption (
    &gUefiShellFileGuid, L"EFI Internal Shell", LOAD_OPTION_ACTIVE
    );

  ...
}
```
If you grep for a `gUefiShellFileGuid`:
```
./OvmfPkg/Library/PlatformBootManagerLib/BdsPlatform.c:    &gUefiShellFileGuid, L"EFI Internal Shell", LOAD_OPTION_ACTIVE
./OvmfPkg/Library/PlatformBootManagerLib/PlatformBootManagerLib.inf:  gUefiShellFileGuid
./ShellPkg/Application/Shell/Shell.inf:  FILE_GUID                      = 7C04A583-9E3E-4f1c-AD65-E05268D0B4D1 # gUefiShellFileGuid
./ShellPkg/ShellPkg.dec:  gUefiShellFileGuid              = {0x7c04a583, 0x9e3e, 0x4f1c, {0xad, 0x65, 0xe0, 0x52, 0x68, 0xd0, 0xb4, 0xd1}}
```

Ok it looks like we need to create a `UefiLessonsPkg/UefiLessonsPkg.dec` file with something like `gHelloWorldFileGuid` with a value equal to the one that is listed in a `UefiLessonsPkg/HelloWorld/HelloWorld.inf` file.

Let's do it:
```
[Defines]
  DEC_SPECIFICATION              = 0x00010005
  PACKAGE_NAME                   = UefiLessonsPkg
  PACKAGE_GUID                   = 7e7edbba-ca2c-4177-a3f0-d3371358773a
  PACKAGE_VERSION                = 1.01

[Guids]
  # FILE_GUID as defined in UefiLessonsPkg/HelloWorld/HelloWorld.inf
  gHelloWorldFileGuid            = {0x2e55fa38, 0xf148, 0x42d3, {0xaf, 0x90, 0x1b, 0xe2, 0x47, 0x32, 0x3e, 0x30}}

```
EDK2 Package Declaration (DEC) File Format Specification can be found at link https://edk2-docs.gitbook.io/edk-ii-dec-specification/

Let's add our GUID to the `OvmfPkg/Library/PlatformBootManagerLib/PlatformBootManagerLib.inf` file
```
[Guids]
 ...
  gUefiShellFileGuid
+ gHelloWorldFileGuid
```
So we could use it in a `OvmfPkg/Library/PlatformBootManagerLib/BdsPlatform.c` file:
```
VOID
EFIAPI
PlatformBootManagerAfterConsole (
  VOID
  )
{
  ...
  //
  // Register UEFI Shell
  //
  PlatformRegisterFvBootOption (
    &gUefiShellFileGuid, L"EFI Internal Shell", LOAD_OPTION_ACTIVE
    );

  //
  // Register HelloWorld app
  //
  PlatformRegisterFvBootOption (
    &gHelloWorldFileGuid, L"Hello World", LOAD_OPTION_ACTIVE
    );
  ...
}
```

If we try to build OVMF now we would get an error:
```
$ build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
...

build.py...
/home/kostr/tiano/edk2/OvmfPkg/Library/PlatformBootManagerLib/PlatformBootManagerLib.inf(87): error 4000: Value of Guid [gHelloWorldFileGuid] is not found under [Guids] section in
        /home/kostr/tiano/edk2/MdePkg/MdePkg.dec
        /home/kostr/tiano/edk2/MdeModulePkg/MdeModulePkg.dec
        /home/kostr/tiano/edk2/SourceLevelDebugPkg/SourceLevelDebugPkg.dec
        /home/kostr/tiano/edk2/OvmfPkg/OvmfPkg.dec
        /home/kostr/tiano/edk2/SecurityPkg/SecurityPkg.dec
        /home/kostr/tiano/edk2/ShellPkg/ShellPkg.dec
...
```
To correct in we need our newly created `UefiLessonsPkg/UefiLessonsPkg.dec` to the `OvmfPkg/Library/PlatformBootManagerLib/PlatformBootManagerLib.inf`:
```
[Packages]
  ...
  ShellPkg/ShellPkg.dec
  UefiLessonsPkg/UefiLessonsPkg.dec
```

Finally we can successfully build OVMF. Let's run it and test our newly created boot option.

Execute QEMU launch as usual. Now if you'll execute our `ShowBootVariables.efi` app you would get:
```
FS0:\> ShowBootVariables.efi
Boot0000
UiApp
Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(462CAA21-7614-4503-836E-8AB6F4662331)

Boot0001
UEFI QEMU DVD-ROM QM00003
PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)

Boot0002
UEFI QEMU HARDDISK QM00001
PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)

Boot0003*
EFI Internal Shell
Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)

Boot0004
Hello World
Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(2E55FA38-F148-42D3-AF90-1BE247323E30)

Boot0005
UEFI PXEv4 (MAC:525400123456)
PciRoot(0x0)/Pci(0x3,0x0)/MAC(525400123456,0x1)/IPv4(0.0.0.0)
```

Hooraay! Our boot option is present in the system.

UEFI shell has a command `bcfg` to print boot variables. You can check that the data from our app is correct, if you execute `bcfg boot dump`:
```
Shell> bcfg boot dump
Option: 00. Variable: Boot0000
  Desc    - UiApp
  DevPath - Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(462CAA21-7614-4503-836E-8AB6F4662331)
  Optional- N
Option: 01. Variable: Boot0001
  Desc    - UEFI QEMU DVD-ROM QM00003
  DevPath - PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
  Optional- Y
Option: 02. Variable: Boot0002
  Desc    - UEFI QEMU HARDDISK QM00001
  DevPath - PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
  Optional- Y
Shell>  03. Variable: Boot0003
  Desc    - EFI Internal Shell
  DevPath - Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)
  Optional- N
Option: 04. Variable: Boot0004
  Desc    - Hello World
  DevPath - Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(2E55FA38-F148-42D3-AF90-1BE247323E30)
  Optional- N
Option: 05. Variable: Boot0005
  Desc    - UEFI PXEv4 (MAC:525400123456)
  DevPath - PciRoot(0x0)/Pci(0x3,0x0)/MAC(525400123456,0x1)/IPv4(0.0.0.0)
  Optional- Y
```

Just in case I've placed `Ovmf.diff` in this lesson folder that shows all the necessary changes.


