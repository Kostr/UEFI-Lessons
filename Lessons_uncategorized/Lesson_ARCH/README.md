After you've written you application code in C you need to compile it for the target CPU architecture.

EDKII supports compilation for several CPU architectures. You can see the complete list if you look at the output of the `build --help` command:
```
$ build --help
...

Options:

...

  -a TARGETARCH, --arch=TARGETARCH
                        ARCHS is one of list: IA32, X64, ARM, AARCH64, RISCV64
                        or EBC, which overrides target.txt's TARGET_ARCH
                        definition. To specify more archs, please repeat this
                        option.

...
```

The abbreviations above mean:
```
IA32    - x86 32bit
X64     - x86 64bit
ARM     - ARM 32bit
AARCH64 - ARM 64bit
RISCV64 - RISCV 64bit
EBC     - EFI Byte Code
```

As we use `qemu-system-x86_64` virualization tool, we've compiled our applications for the `X64` architecture.

In our first application examples we did it via an argument to the `build` command:
```
build ... --arch=X64 ...
```
Later we've constructed the `Conf/target.txt` file, so we could call the `build` command without any arguments. In this file we've indicated the target arch by setting the `TARGET_ARCH` define:
```
`TARGET_ARCH = X64`
```

Let's try to build our code for the x86 32bit (`IA32`) architecture.

To do this we can override default argument (which comes from the `Conf/target.txt`) via a command argument:
```
build --arch=IA32
```

If you try to do it right now, you'll get the error:
```
build.py...
 : error 2000: Invalid parameter
        Invalid ARCH specified. [Valid ARCH: X64]

- Failed -
```

This is happening because `IA32` architecture is not present in the architecture whitelist in our DSC file `UefiLessonsPkg/UefiLessonsPkg.dsc`:
```
[Defines]
 ...
 SUPPORTED_ARCHITECTURES        = X64
```

To correct this modify `SUPPORTED_ARCHITECTURES` to the following value:
```
[Defines]
 ...
 SUPPORTED_ARCHITECTURES        = IA32|X64
```

Now the build via `build --arch=IA32` should succeed. The results will be in the same `Build/UefiLessonsPkg/RELEASE_GCC5/` folder. But now along with the `X64` subfolder there would be the `IA32` subfolder.

You can verify that the generated application indeed are different via the Linux `file` command:
```
$ file Build/UefiLessonsPkg/RELEASE_GCC5/X64/PCDLesson.efi
Build/UefiLessonsPkg/RELEASE_GCC5/X64/PCDLesson.efi: MS-DOS executable PE32+ executable (EFI application) x86-64, for MS Windows
$ file Build/UefiLessonsPkg/RELEASE_GCC5/IA32/PCDLesson.efi
Build/UefiLessonsPkg/RELEASE_GCC5/IA32/PCDLesson.efi: MS-DOS executable PE32 executable (EFI application) Intel 80386, for MS Windows
```

# Architecture specifier for sections names

In our `UefiLessonsPkg/UefiLessonsPkg.dsc` file we have an override for the `PcdInt32Override` PCD:
```
[PcdsFixedAtBuild]
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|44
```

This override works for both `X64` and `IA32` builds:
```
$ grep _PCD_VALUE_PcdInt32Override Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
#define _PCD_VALUE_PcdInt32Override  44U
$ grep _PCD_VALUE_PcdInt32Override Build/UefiLessonsPkg/RELEASE_GCC5/IA32/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
#define _PCD_VALUE_PcdInt32Override  44U
```

It is possible to set override for the particular architecture. For example change the section name to the:
```
[PcdsFixedAtBuild.IA32]
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|44
```

Regenerate both builds and verify the PCD values:
```
$ build && build --arch=IA32
...
$ grep _PCD_VALUE_PcdInt32Override Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
#define _PCD_VALUE_PcdInt32Override  43U
$ grep _PCD_VALUE_PcdInt32Override Build/UefiLessonsPkg/RELEASE_GCC5/IA32/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
#define _PCD_VALUE_PcdInt32Override  44U
```

It is possible to have both the generic arch section and the qualified one:
```
[PcdsFixedAtBuild.IA32]
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|45

[PcdsFixedAtBuild]
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|44
```

In this case `X64` arch PCD would get its value from the generic override, and `IA32` arch PCD would get its value from the qualified section. The order of sections is not important in this case:
```
$ build && build --arch=IA32
...
$ grep _PCD_VALUE_PcdInt32Override Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
#define _PCD_VALUE_PcdInt32Override  44U
$ grep _PCD_VALUE_PcdInt32Override Build/UefiLessonsPkg/RELEASE_GCC5/IA32/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
#define _PCD_VALUE_PcdInt32Override  45U
```

It is worth to notice that it is possible to provide different section names as a list, for example:
```
[PcdsFixedAtBuild.IA32, PcdsFixedAtBuild.X64]
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|45
```
In this case the override would be applied for the `X64` and `IA32` builds, but not for the `ARM` build for example (if we would ever want to permit one).

# `common`

Keep and mind that currently you can write any name in the arch section name qualifier. EDKII doesn't perform any checks on this matter:
```
[PcdsFixedAtBuild.IA32]
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|45

[PcdsFixedAtBuild.blablabla]
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|44
```

Off course in this case the `X64` PCD wouldn't get its override to the 44 value:
```
$ build && build --arch=IA32
...
$ grep _PCD_VALUE_PcdInt32Override Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
#define _PCD_VALUE_PcdInt32Override  43U
$ grep _PCD_VALUE_PcdInt32Override Build/UefiLessonsPkg/RELEASE_GCC5/IA32/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
#define _PCD_VALUE_PcdInt32Override  45U
```
So don't confuse `IA32` with `X32` or something like that. You woudn't see any error, but you'll get incorrect values.

As we will know next, there are many layers to the section name override (e.g. `[PcdsFixedAtBuild.XXX.YYY.ZZZ]`). If you want to override second layer but want to keep the first one as generic, you use `common` for indication that the override goes to any architecture. For example this override:
```
[PcdsFixedAtBuild.IA32]
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|45

[PcdsFixedAtBuild.common]
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|44
```

Will give you the same results as with the `[PcdsFixedAtBuild.IA32]/[PcdsFixedAtBuild]` sections:
```
$ build && build --arch=IA32
...
$ grep _PCD_VALUE_PcdInt32Override Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
#define _PCD_VALUE_PcdInt32Override  44U
$ grep _PCD_VALUE_PcdInt32Override Build/UefiLessonsPkg/RELEASE_GCC5/IA32/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h
#define _PCD_VALUE_PcdInt32Override  45U
```

But with a help of a `common` syntax you can construct overrides for other build aspects like `[PcdsFixedAtBuild.common.XXX]`.

# Other sections

In the example above we've used arch section name override for the `[PcdsFixedAtBuild]` section in the DSC file. But it is also to possible to override other sections in other EDKII metafiles (DEC/INF/FDF), for example:
```
[PcdsPatchableInModule.X64]
[PcdsDynamic.X64]
[Protocols.X64]
[LibraryClasses.X64]
[Components.X64]
[BuildOptions.X64]
[Guids.X64]
[Sources.X64]
[Includes.X64]
[Defines.X64]
[Depex.X64]
[Pcd.X64]
```

You should consult EDKII documentation for the particular metafile (DEC/INF/DSC/FDF), but generally almost every section name can be overriden with the ARCH qualifier. And the ARCH override would always be the first qualifier one in section naming.

