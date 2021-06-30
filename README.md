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


