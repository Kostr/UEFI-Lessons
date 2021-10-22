First install necessary packages to your distro.
For the EDKII compilation you'll need:
- nasm - The Netwide Assembler (NASM) is an assembler and disassembler for the Intel x86 architecture. It can be used to write 16-bit, 32-bit (IA-32) and 64-bit (x86-64) programs. NASM is considered to be one of the most popular assemblers for Linux,
- iasl - Intel ACPI compiler/decompiler. Advanced Configuration and Power Interface (ACPI) provides an open standard that operating systems can use to discover and configure computer hardware components, to perform power management and to perform status monitoring. UEFI firmware provides ACPI configuration tables to OS. ACPI specification can be found at https://uefi.org/specifications,
- uuid-dev - Universally Unique ID library. A universally unique identifier (UUID) is a 128-bit label used for identification of various components in UEFI. The term globally unique identifier (GUID) is also used,
- python3 - Python 3 interpreter is needed as build infrastructure includes python scripts.

This is how you install these packages in Ubuntu:
```
$ sudo apt-get install -y nasm iasl uuid-dev python3
```

Also it might be necessary to install `python3-distutils` package:
```
$ sudo apt-get install -y python3-distutils
```

Then it is necessary to clone edk2 repo and update all its submodules:
```
$ git clone https://github.com/tianocore/edk2
$ cd edk2
$ git submodule update --init
```

Just in case the size of the edk2 folder after that would be ~867M (as for 12-06-2021).

Next we need to compile EDK2 build tools:
```
make -C BaseTools
```

To initiate a build we need to source `edksetup.sh` script to get necessary variables into our environment:
```
$ . edksetup.sh
Loading previous configuration from /<...>/edk2/Conf/BuildEnv.sh
Using EDK2 in-source Basetools
WORKSPACE: /<...>/edk2
EDK_TOOLS_PATH: /<...>/edk2/BaseTools
CONF_PATH: /<...>/edk2/Conf
```

After that we could use `build` command to build EDK2 packages.
Try to execute help command to see all available options:
```
build --help
```

Next we want to build the Open Virtual Machine Firmware (OVMF). OVMF is a EDKII-based firmware that is possible to run under the qemu x86-64 virtual machine. This allows easy debugging and experimentation with UEFI firmware; either for testing OS booting or using the (included) EFI shell. We would be writing various EFI programs, and OVMF is an easy way to test them. We could install it with the package manager with something like `sudo apt-get install ovmf`, but it is not fun.

To build OVMF execute:
```
build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

If build is successful, result would be in the folder `Build/{Platform Name}/{TARGET}_{TOOL_CHAIN_TAG}/FV`. So in our case it would be:
```
$ ls -lh Build/OvmfX64/RELEASE_GCC5/FV/OVMF*
-rw-r--r-- 1 kostr kostr 4.0M Jun 12 21:03 Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd
-rw-r--r-- 1 kostr kostr 3.5M Jun 12 21:03 Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd
-rw-r--r-- 1 kostr kostr 528K Jun 12 21:03 Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd
```

The OVMF firmware (a UEFI implementation for QEMU) is split into two files:
- OVMF_CODE.fd contains the actual UEFI firmware,
- OVMF_VARS.fd is a "template" used to emulate persistent NVRAM storage.

All VM instances can share the same system-wide, read-only OVMF_CODE.fd file from the ovmf package, but each instance needs a private, writable copy of OVMF_VARS.fd.

Let's install QEMU to test our build artifacts:
```
sudo apt-get install qemu-system-x86_64
```

Lets execute QEMU to run our binaries:
```
$ qemu-system-x86_64 -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF_CODE.fd \
                     -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF_VARS.fd \
                     -nographic \
                     -net none
```

Hopefully you'll see UEFI Interactive Shell:
```
ïI Interactive Shell v2.2
EDK II
UEFI v2.70 (EDK II, 0x00010000)
Mapping table
     BLK0: Alias(s):
          PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
Press ESC in 1 seconds to skip startup.nsh or any other key to continue.
Shell>
```

To exit QEMU run `CTRL+A - X`

The QEMU run script can be simplified to:
```
$ qemu-system-x86_64 -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
                     -nographic \
                     -net none
```

More info about OVMF can be found at https://github.com/tianocore/edk2/blob/master/OvmfPkg/README
