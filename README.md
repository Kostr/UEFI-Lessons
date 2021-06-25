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

