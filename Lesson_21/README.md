Let's create a another variable `PcdMyVar32_1` the same way we did in the previous lesson.

Add a PCD definition to the `UefiLessonsPkg/UefiLessonsPkg.dec`:
```
[PcdsFixedAtBuild]
  ...
  gUefiLessonsPkgTokenSpaceGuid.PcdMyVar32_1|42|UINT32|0x00000002
```

Add print statement to `UefiLessonsPkg/PCDLesson/PCDLesson.c`:
```
Print(L"PcdMyVar32_1=%d\n", FixedPcdGet32(PcdMyVar32_1));
```

As for the `PCDLesson.inf` this time add an override for a value this time:
```
[FixedPcd]
  ...
  gUefiLessonsPkgTokenSpaceGuid.PcdMyVar32_1|43
```

If you build execute our app under OVMF you would get:
```
FS0:\> PCDLesson.efi
PcdMyVar32=42
PcdMyVar32_1=43
```

So it means that every App/Driver can override a PCD declared in *.dec file differently.

______________

Now let's create a third variable `PcdMyVar32_2` the same way as `PcdMyVar32_1`.
`UefiLessonsPkg/UefiLessonsPkg.dec`
 ```
[PcdsFixedAtBuild]
  ...
  gUefiLessonsPkgTokenSpaceGuid.PcdMyVar32_2|42|UINT32|0x00000003
```
UefiLessonsPkg/PCDLesson/PCDLesson.c
```
Print(L"PcdMyVar32_2=%d\n", FixedPcdGet32(PcdMyVar32_2));
```
UefiLessonsPkg/PCDLesson/PCDLesson.inf
```
[FixedPcd]
  ...
  gUefiLessonsPkgTokenSpaceGuid.PcdMyVar32_2|43
```
Only this time add an override for the variable to our `UefiLessonsPkg/UefiLessonsPkg.dsc` file:
```
[PcdsFixedAtBuild]
  gUefiLessonsPkgTokenSpaceGuid.PcdMyVar32_2|44
```

If you build our app and execute in under OVMF now you would get:
```
FS0:\> PCDLesson.efi
PcdMyVar32=42
PcdMyVar32_1=43
PcdMyVar32_2=44
```

So override order would be:
```
DEC < INF < DSC
```
- Declaration file (DEC) declares PCD with its default value
- Every App/Driver Information file (INF) can override the value of this PCD differently
- However a package description file that contains all these Apps/Drivers (DSC) can override this PCD for all of them

