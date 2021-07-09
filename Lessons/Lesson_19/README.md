In this lesson we would be messing with the NVRAM variables (BIOS settings), i.e. variables that are persistent between boots.

As you remember after OVMF build we have these files:
```
$ ls -l Build/OvmfX64/RELEASE_GCC5/FV/OVMF*
-rw-r--r-- 1 kostr kostr 4194304 Jun 25 14:40 Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd
-rw-r--r-- 1 kostr kostr 3653632 Jun 25 14:40 Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd
-rw-r--r-- 1 kostr kostr  540672 Jun 25 14:38 Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd
```
- `OVMF_CODE.fd` - code image (read-only)
- `OVMF_VARS.fd` - NVRAM variables image (read-write)
- `OVMF.fd` - combined image (`OVMF_CODE.fd` + `OVMF_VARS.fd`)

We can boot QEMU either with:
```
$ qemu-system-x86_64 -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
                     ...
```
or with
```
$ qemu-system-x86_64 -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd \
                     -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd \
                     ...
```

It was fine to use short form earlier, but in this lesson we would modify NVRAM variables, so it is best to use full form with a separate copy of `OVMF_VARS.fd`, so you could always revert things to their initial state.

Let's undo our modifications to the `OvmfPkg` package and rebuild it:
```
$ git restore OvmfPkg
$ build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

After that create a backup copy of `OVMF_VARS.fd` and run QEMU with it:
```
$ cp Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd ../
$ qemu-system-x86_64 -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd \
                     -drive if=pflash,format=raw,file=../OVMF_VARS.fd \
                     -drive format=raw,file=fat:rw:~/UEFI_disk \
                     -nographic \
                     -net none
```

Check boot variables with the help of `bcfg boot dump`:
```
Shell> bcfg boot dump
Option: 00. Variable: Boot0000
  Desc    - UiApp
  DevPath - Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(462CAA21-7614-4503-836E-8AB6F4662331)
  OVMF_VARS.fdiOptional- N
Option: 01. Variable: Boot0001
  Desc    - UEFI QEMU DVD-ROM QM00003
  DevPath - PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
  Optional- Y
Option: 02. Variable: Boot0002
  Desc    - UEFI QEMU HARDDISK QM00001
  DevPath - PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
  Optional- Y
Option: 03. Variable: Boot0003
  Desc    - EFI Internal Shell
  DevPath - Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)
  Optional- N
```

Besides showing boot options `bcfg` command can add/remove boot options or change their order.
You can check out help for `bcfg` via:
```
bcfg -? -b
```

Now let's try to add our `InteractiveApp.efi` to the boot options.

```
Shell> fs0:
FS0:\> bcfg boot add 4 InteractiveApp.efi "Interactive app"
Target = 0004.
bcfg: Add Boot0004 as 4
FS0:\> bcfg boot dump
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
Option: 03. Variable: Boot0003
  Desc    - EFI Internal Shell
  DevPath - Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)
  Optional- N
Option: 04. Variable: Boot0004
  Desc    - Interactive app
  DevPath - PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)/HD(1,MBR,0xBE1AFDFA,0x3F,0xFBFC1)/\InteractiveApp.efi
  Optional- N
```

Now if you navigate to the `Boot Manager` you'll see our app:
![Boot Manager](BootManager.png?raw=true "Boot manager")

As soon as you don't change `OVMF_VARS.fd` this option would be present even between QEMU restarts.

We can go further and even place our app as a first boot source:
```
Shell> bcfg boot mv 4 0
Shell> bcfg boot dump
Option: 00. Variable: Boot0004
  Desc    - Interactive app
  DevPath - PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)/HD(1,MBR,0xBE1AFDFA,0x3F,0xFBFC1)/\InteractiveApp.efi
  Optional- N
Option: 01. Variable: Boot0000
  Desc    - UiApp
  DevPath - Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(462CAA21-7614-4503-836E-8AB6F4662331)
  Optional- N
Option: 02. Variable: Boot0001
  Desc    - UEFI QEMU DVD-ROM QM00003
  DevPath - PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
  Optional- Y
Option: 03. Variable: Boot0002
  Desc    - UEFI QEMU HARDDISK QM00001
  DevPath - PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
  Optional- Y
Option: 04. Variable: Boot0003
  Desc    - EFI Internal Shell
  DevPath - Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)
  Optional- N
```

Now if you reboot UEFI shell with a `reset` command, or rerun QEMU, you will need to pass our app before you can go to the BIOS menu:
```
BdsDxe: loading Boot0004 "Interactive app" from PciRoot(0x0)/Pci(0x1,0x1)/Ata(Primary,Master,0x0)/HD(1,MBR,0xBE1AFDFA,0x3F,0xFBFC1)/\InteractiveApp.efi
BdsDxe: starting Boot0004 "Interactive app" from PciRoot(0x0)/Pci(0x1,0x1)/Ata(Primary,Master,0x0)/HD(1,MBR,0xBE1AFDFA,0x3F,0xFBFC1)/\InteractiveApp.efi
Try to guess the secret symbol!
```

Input correct symbol, go to the `Boot manager` menu and run UEFI shell so we could delete our app from the boot sources:
```
FS0:\> bcfg boot dump
Option: 00. Variable: Boot0004
  Desc    - Interactive app
  DevPath - PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)/HD(1,MBR,0xBE1AFDFA,0x3F,0xFBFC1)/\InteractiveApp.efi
  Optional- N
Option: 01. Variable: Boot0000
  Desc    - UiApp
  DevPath - Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(462CAA21-7614-4503-836E-8AB6F4662331)
  Optional- N
Option: 02. Variable: Boot0001
  Desc    - UEFI QEMU DVD-ROM QM00003
  DevPath - PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
  Optional- Y
Option: 03. Variable: Boot0002
  Desc    - UEFI QEMU HARDDISK QM00001
  DevPath - PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
  Optional- Y
Option: 04. Variable: Boot0003
  Desc    - EFI Internal Shell
  DevPath - Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)
  Optional- N

FS0:\> bcfg boot rm 0

FS0:\> bcfg boot dump
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
Option: 03. Variable: Boot0003
  Desc    - EFI Internal Shell
  DevPath - Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)
  Optional- N
```




