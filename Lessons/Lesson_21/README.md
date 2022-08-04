In this lesson we are going to look at the PCD override capability.

# INF override

Create new `UINT32` PCD with a name `PcdInt32Override` and a default value equal to 42 (`UefiLessonsPkg/UefiLessonsPkg.dec`):
```
[PcdsFixedAtBuild]
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|42|UINT32|0x3CB8ABB8
```

Now when we would populate the PCD to our application via the INF file (`UefiLessonsPkg/PCDLesson/PCDLesson.inf`), we would use the PCD override syntax to change the default value of our PCD, which looks like this:
```
[FixedPcd]
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|43
```

If you build our application now, and look at the `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/PCDLesson/PCDLesson/DEBUG/AutoGen.h`, you would see that `PcdInt32Override` default value was overriden:
```
...
#define _PCD_VALUE_PcdInt32Override  43U
...
```

You can even add a print statement to the `UefiLessonsPkg/PCDLesson/PCDLesson.c` to verify the result:
```
Print(L"PcdInt32Override=%d\n", FixedPcdGet32(PcdInt32Override));
```

If you rebuild the application and execute it under OVMF you would get:
```
FS0:\> PCDLesson.efi
...
PcdInt32Override=43
```

So it means that every App/Driver can override a PCD declared in `*.dec` file differently.

# DSC override

Besides overwriting PCDs locally in every module it is possible to override PCDs globally in the package via its DSC file.

Don't delete our INF override but another one in the DSC file (`UefiLessonsPkg/UefiLessonsPkg.dsc`). In this case we would override the default value to 44:
```
[PcdsFixedAtBuild]
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|44
```

If you build the app and execute in under OVMF now you would get:
```
FS0:\> PCDLesson.efi
...
PcdInt32Override=44
```

So as you can see there is some order of precedence for the override statments:
```
DEC < INF < DSC
```
- Declaration file (DEC) declares PCD with its default value
- Every App/Driver Information file (INF) can override the value of this PCD differently
- However a package description file that contains all these Apps/Drivers (DSC) can override this PCD for all of them

It is important to note that the DSC file in this case means a DSC file of the package that we build. And it is not necessary the same as the DSC file of the module origin package.

For the proof let's build our module as a part of the `OvmfPkg/OvmfPkgX64.dsc` package. For this add our application to the `[Components]` section in the `OvmfPkg/OvmfPkgX64.dsc` file:
```
[Components]
  ...
  UefiLessonsPkg/PCDLesson/PCDLesson.inf
```

Now build the OVMF image:
```
build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

The image of our application in this case would be not at the usual `Build/UefiLessonsPkg/RELEASE_GCC5/X64/PCDLesson.efi` path, but at the path
```
Build/OvmfX64/RELEASE_GCC5/X64/PCDLesson.efi
```

So if you copy this `.efi` file to our shared disk and execute it under OVMF image, you would get:
```
FS0:\> PCDLesson.efi
...
PcdInt32Override=43
```

As you can see the override from the `UefiLessonsPkg/UefiLessonsPkg.dsc` file doesn't work in this case.

For the DSC override you need to modify the DSC of the package that we build, i.e. `OvmfPkg/OvmfPkgX64.dsc` file:
```
[PcdsFixedAtBuild]
  ...
  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|45
```

Rebuild OVMF image, copy updated application to the shared folder, and verify that now DSC override is working correctly:
```
FS0:\> PCDLesson.efi
...
PcdInt32Override=45
```

# FDF override

When we've added our application to the `[Components]` section in the `OvmfPkg/OvmfPkgX64.dsc` we said that we want to compile our application as part of the package. If we want to include our application to the final OVMF flash image we need to add it to the `FDF` file. We won't go into details of how it is done right now, just add the `INF UefiLessonsPkg/PCDLesson/PCDLesson.inf` statement to the end of `[FV.DXEFV]` setion:
```
[FV.DXEFV]
...
INF  UefiLessonsPkg/PCDLesson/PCDLesson.inf
#################################################
```

Important part for us now is that in the same FDF file we can override our PCD via the `SET` syntax. First put it in the `[Defines]` section:
```
[Defines]
...
SET gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override = 46
...
```

Rebuild and verify the override result:
```
FS0:\> PCDLesson.efi
...
PcdInt32Override=46
```

Now add another `SET` statement to the section of our module (which is `[FV.DXEFV]` in our case):
```
[FV.DXEFV]
...
INF  UefiLessonsPkg/PCDLesson/PCDLesson.inf
SET  gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override = 47
#################################################
```

This override should take precedence over the general one in the `[Defines]` section:
```
FS0:\> PCDLesson.efi
...
PcdInt32Override=47
```

# DSC in-module overrides

The next level of PCD override is the DSC in-module overrides. The override syntax in this case looks like this:
```
[Components]
  ...
  UefiLessonsPkg/PCDLesson/PCDLesson.inf {
    <PcdsFixedAtBuild>
      gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override|48
  }
```

This override would take precedence over the ones in the FDF file:
```
FS0:\> PCDLesson.efi
...
PcdInt32Override=48
```

# Overriding via command-line argument

Finally it is possible to override PCD via a command-line argument to the `build` command, for example:
```
build --platform=OvmfPkg/OvmfPkgX64.dsc \
      --arch=X64 \
      --buildtarget=RELEASE \
      --tagname=GCC5 \
      --pcd="gUefiLessonsPkgTokenSpaceGuid.PcdInt32Override=49"
```

Verify the override via our print statement:
```
FS0:\> PCDLesson.efi
...
PcdInt32Override=49
```

# Final words

In this lesson we've investgated some of the precedence rules of the PCD override.

The complete documentation on the subject can be found at this link [https://edk2-docs.gitbook.io/edk-ii-dsc-specification/2_dsc_overview/27_pcd_section_processing#2.7.3.8-precedence](https://edk2-docs.gitbook.io/edk-ii-dsc-specification/2_dsc_overview/27_pcd_section_processing#2.7.3.8-precedence)

