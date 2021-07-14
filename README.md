These series of lessons are intendend to get you started with UEFI programming in Linux with the help of TianoCore.

Lessons description:

- Lesson 00: Getting started guide for TianoCore. Compile OVMF and run it in QEMU
- Lesson 01: Create a simplest app and run it in OVMF
- Lesson 02: Create a simplest package
- Lesson 03: Create `HelloWorld` app with the help of SystemTable services
- Lesson 04: Use edk2 libraries to simplify our `HelloWorld` app
- Lesson 05: Simplify build command with the help of files in the `Conf` folder
- Lesson 06: Handle/Protocol databases internals - Part 1: Theory and raw access to an app ImageHandle
- Lesson 07: Handle/Protocol databases internals - Part 2: Raw access to app protocols
- Lesson 08: `HandleProtocol` API function. Information from the `ImageHandle` protocols
- Lesson 09: Get ImageHandle protocols with the `ProtocolsPerHandle` API function
- Lesson 10: `EFI_STATUS` type and `EFI_ERROR` macros
- Lesson 11: Get EFI memory map information
- Lesson 12: `EFI_SHELL_PARAMETERS_PROTOCOL`. Transform our EFI memory map to the Linux kernel style
- Lesson 13: `ShellAppMain` entry point
- Lesson 14: Get all variable names and GUIDs with the `gRT->GetNextVariableName` API
- Lesson 15: Get and parse boot variables (BootOrder/BootCurrent/BootXXXX) with the `gRT->GetVariable` API
- Lesson 16: Build our own boot option inside OVMF image similar to the UEFI shell app
- Lesson 17: Add `WaitForEvent` function to our boot option app to see its output on booting
- Lesson 18: Handle input from user with the `ReadKeyStroke` function
- Lesson 19: Boot option modification with the help of `bcfg` command
- Lesson 20: Intro to Platfrom Configuration Database (PCD). Declare and get simple fixed PCD
- Lesson 21: Override order for PCD variables
- Lesson 22: Feature flag PCD and its comparision to BOOLEAN fixed at build PCD
- Lesson 23: PatchableInModule PCDs and how they can be changed via `GenPatchPcdTable`/`PatchPcdValue` utilities 
- Lesson 24: Dynamic/DynamicEx PCDs
- Lesson 25: More on PCDs
- Lesson 26: Tables referenced in `EFI_CONFIGURATION_TABLE`
- Lesson 27: Get SMBIOS information with `dmem`/`EFI_SMBIOS_PROTOCOL`/`smbiosview`
- Lesson 28: Get ACPI tables and save them to files with a help of `EFI_SHELL_PROTOCOL`
- Lesson 29: Use `EFI_ACPI_SDT_PROTOCOL` and `ShellLib` to save a BMP image from the ACPI BGRT table
- Lesson 30: Find all PCI root bridges in the system with a help of `LocateHandleBuffer`/`OpenProtocol` functions and use
`EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL` to get all PCI functions in the system.
- Lesson 31: Search `pci.ids` database to get PCI Vendor/Device information with a help of `ShellLib`/`PrintLib` functions
- Lesson 32: Show PCI Option ROM images with the help of `EFI_PCI_IO_PROTOCOL` protocol
- Lesson 33: Use `EfiRom` utility for parsing and creation of PCI Option ROM images 
- Lesson 34: Create a simple UEFI driver. Use `load`/`unload` UEFI shell commands to work with a driver image 
- Lesson 35: Create a simple library and use it in an app
- Lesson 36: Library constructor and destructor. `NULL` libraries
- Lesson 37: Investigate ways how to add `acpiview` command functionality to your shell
- Lesson 38: Create and use your custom protocol. `InstallMultipleProtocolInterfaces` and `UninstallMultipleProtocolInterfaces` functions
- Lesson 39: Create a driver that adds hot key functionality with a help of `RegisterKeyNotify`/`UnregisterKeyNotify` functions

______

Usefull links:
1) Beyond BIOS: Developing with the Unified Extensible Firmware Interface, Third Edition by Vincent Zimmer and Michael Rothman (https://www.amazon.com/Beyond-BIOS-Developing-Extensible-Interface/dp/1501514784)

2) Harnessing the Uefi Shell: Moving The Platform Beyond Dos, Second Edition by Michael Rothman and Vincent Zimmer (https://www.amazon.com/Harnessing-UEFI-Shell-Moving-Platform/dp/1501514806)

3) https://github.com/tianocore-training

https://github.com/Laurie0131

4) https://blog.fpmurphy.com/

https://github.com/fpmurphy/UEFI-Utilities-2019

5) https://github.com/andreiw/UefiToolsPkg

_______

Check these if you don't afraid of the Chinese language:

6) Principles and Programming of UEFI, Dai Zhenghua
(https://www.amazon.com/Combat-UEFI-Principles-Programming-Chinese/dp/B07W3GFLRM)

https://github.com/zhenghuadai/uefi-programming

7) http://www.lab-z.com/iof/

8) https://blog.csdn.net/luobing4365

https://gitee.com/luobing4365/uefi-exolorer

_______

And for the Russian speakers:

9) https://habr.com/ru/users/CodeRush/posts/

# Specifications

UEFI specifications (https://uefi.org/specifications):
- [UEFI Specification](https://uefi.org/sites/default/files/resources/UEFI_Spec_2_9_2021_03_18.pdf)
- [UEFI Platform Initialization Specification](https://uefi.org/sites/default/files/resources/PI_Spec_1_7_A_final_May1.pdf)
- [UEFI Shell Specification](http://www.uefi.org/sites/default/files/resources/UEFI_Shell_2_2.pdf)

EDK2 specifications (https://github.com/tianocore/tianocore.github.io/wiki/EDK-II-Documentation#specifications):
- [Build Specification](https://edk2-docs.gitbook.io/edk-ii-build-specification/)
- [Platform Description (DSC) File Specification](https://edk2-docs.gitbook.io/edk-ii-dsc-specification/)
- [Package Declaration (DEC) File Format Specification](https://edk2-docs.gitbook.io/edk-ii-dec-specification/)
- [Module Information (INF) File Specification](https://edk2-docs.gitbook.io/edk-ii-inf-specification/)
- [Flash Description (FDF) File Specification](https://edk2-docs.gitbook.io/edk-ii-fdf-specification/)
- [Platform Configuration Database Infrastructure Description](https://edk2-docs.gitbook.io/edk-ii-pcd-specification/)
- [Multi-String .UNI File Format Specification](https://edk2-docs.gitbook.io/edk-ii-uni-specification/)
- [VFR Programming Language Specification](https://edk2-docs.gitbook.io/edk-ii-vfr-specification/)
- [Driver Writer's Guide](https://edk2-docs.gitbook.io/edk-ii-uefi-driver-writer-s-guide/)
- [Module Writer's Guide](https://edk2-docs.gitbook.io/edk-ii-module-writer-s-guide/)
- [EDKII User Manual](https://github.com/tianocore-docs/Docs/raw/master/User_Docs/EDK_II_UserManual_0_7.pdf)
