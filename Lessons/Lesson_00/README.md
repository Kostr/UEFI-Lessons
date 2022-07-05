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

If you curious what projects EDKII loads as submodules check [https://github.com/tianocore/edk2/blob/master/.gitmodules](https://github.com/tianocore/edk2/blob/master/.gitmodules) file.
Currently it has these submodules:
```
[submodule "CryptoPkg/Library/OpensslLib/openssl"]
	path = CryptoPkg/Library/OpensslLib/openssl
	url = https://github.com/openssl/openssl                            # TLS/SSL and crypto library
[submodule "SoftFloat"]
	path = ArmPkg/Library/ArmSoftFloatLib/berkeley-softfloat-3
	url = https://github.com/ucb-bar/berkeley-softfloat-3.git           # software implementation of binary floating-point
[submodule "UnitTestFrameworkPkg/Library/CmockaLib/cmocka"]
	path = UnitTestFrameworkPkg/Library/CmockaLib/cmocka
	url = https://github.com/tianocore/edk2-cmocka.git                  # unit testing framework for C with support for mock objects
[submodule "MdeModulePkg/Universal/RegularExpressionDxe/oniguruma"]
	path = MdeModulePkg/Universal/RegularExpressionDxe/oniguruma
	url = https://github.com/kkos/oniguruma                             # regular expression library
[submodule "MdeModulePkg/Library/BrotliCustomDecompressLib/brotli"]
	path = MdeModulePkg/Library/BrotliCustomDecompressLib/brotli
	url = https://github.com/google/brotli                              # Brotli compression format
[submodule "BaseTools/Source/C/BrotliCompress/brotli"]
	path = BaseTools/Source/C/BrotliCompress/brotli
	url = https://github.com/google/brotli                              # Brotli compression format
	ignore = untracked
[submodule "RedfishPkg/Library/JsonLib/jansson"]
	path = RedfishPkg/Library/JsonLib/jansson
	url = https://github.com/akheron/jansson                            # C library for encoding, decoding and manipulating JSON data
```

Just in case the size of the edk2 folder after the submodules load would be ~867M (as for 12-06-2021).

Next we need to compile EDK2 build tools. These tools are present in the [Basetools](https://github.com/tianocore/edk2/tree/master/BaseTools) folder. Basically all the tools are either Python scripts or C programs:
- [https://github.com/tianocore/edk2/tree/master/BaseTools/Source/Python](https://github.com/tianocore/edk2/tree/master/BaseTools/Source/Python)
- [https://github.com/tianocore/edk2/tree/master/BaseTools/Source/C](https://github.com/tianocore/edk2/tree/master/BaseTools/Source/C)

We need to compile the later ones. For this we execute:
```
make -C BaseTools
```
This would produce tools and libs under the folders:
- `BaseTools/Source/C/bin`
- `BaseTools/Source/C/libs`

As EDKII can be compiled both for Windows and Linux, the tools are used through wrappers:
For Linux:
- [https://github.com/tianocore/edk2/tree/master/BaseTools/BinPipWrappers/PosixLike](https://github.com/tianocore/edk2/tree/master/BaseTools/BinPipWrappers/PosixLike)
- [https://github.com/tianocore/edk2/tree/master/BaseTools/BinWrappers/PosixLike](https://github.com/tianocore/edk2/tree/master/BaseTools/BinWrappers/PosixLike)

For Windows:
- [https://github.com/tianocore/edk2/tree/master/BaseTools/BinPipWrappers/WindowsLike](https://github.com/tianocore/edk2/tree/master/BaseTools/BinPipWrappers/WindowsLike)
- [https://github.com/tianocore/edk2/tree/master/BaseTools/BinWrappers/WindowsLike](https://github.com/tianocore/edk2/tree/master/BaseTools/BinWrappers/WindowsLike)

Manuals for some of the build tools you can find under the link [https://github.com/tianocore/edk2/tree/master/BaseTools/UserManuals](https://github.com/tianocore/edk2/tree/master/BaseTools/UserManuals)

Ok, we've build all the necessary tools and we're ready to build EDKII.

To initiate a build we need to source `edksetup.sh` script to get necessary variables into our environment:
```
$ . edksetup.sh
Using EDK2 in-source Basetools
WORKSPACE: /<...>/edk2
EDK_TOOLS_PATH: /<...>/edk2/BaseTools
CONF_PATH: /<...>/edk2/Conf
Copying $EDK_TOOLS_PATH/Conf/build_rule.template
     to /<...>/edk2/Conf/build_rule.txt
Copying $EDK_TOOLS_PATH/Conf/tools_def.template
     to /<...>/edk2/Conf/tools_def.txt
Copying $EDK_TOOLS_PATH/Conf/target.template
     to /<...>/edk2/Conf/target.txt
```
Besides the new variables `WORKSPACE`/`EDK_TOOLS_PATH`/`CONF_PATH` this script would also update the standard `PATH` variable prepending it with a wrapper folder `BaseTools/BinWrappers/PosixLike`, so EDKII build tools could be used simply by their names.

As you can see on the first launch this script also initializes necessary for the edk2 build `Conf` folder. This folder will be initialized by copying several files from the [https://github.com/tianocore/edk2/tree/master/BaseTools/Conf](https://github.com/tianocore/edk2/tree/master/BaseTools/Conf) folder.

This happens only once, on the next `edksetup.sh` loads this script would only set environment variables:
```
$ . edksetup.sh
Loading previous configuration from /<...>/edk2/Conf/BuildEnv.sh
Using EDK2 in-source Basetools
WORKSPACE: /<...>/edk2
EDK_TOOLS_PATH: /<...>/edk2/BaseTools
CONF_PATH: /<...>/edk2/Conf
```
The `Conf` directory contains several configuration files for the build. It is implied that EDKII user would configure the build by modifying files in this folder, this is why this folder is not under git control but rather created on the fly via `BaseTools/Conf` templates and the content inside the `Conf` folder is ignored by git [https://github.com/tianocore/edk2/blob/master/Conf/.gitignore](https://github.com/tianocore/edk2/blob/master/Conf/.gitignore).


After that we could use the `build` command to build EDK2 packages.
We can use `build` command directly in our shell, because we've added `BaseTools/BinWrappers/PosixLike` to our path and the `build` command is just a wrapper [https://github.com/tianocore/edk2/blob/master/BaseTools/BinWrappers/PosixLike/build](https://github.com/tianocore/edk2/blob/master/BaseTools/BinWrappers/PosixLike/build) around the actual Python script [https://github.com/tianocore/edk2/blob/master/BaseTools/Source/Python/build/build.py](https://github.com/tianocore/edk2/blob/master/BaseTools/Source/Python/build/build.py).
Try to execute help option to see all the available build options:
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
