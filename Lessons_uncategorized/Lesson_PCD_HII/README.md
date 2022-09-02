In our DEC file we have Dynamic and DynamicEx PCDs:
```
[PcdsDynamic]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32|0xCAFECAFE|UINT32|0x4F9259A3

[PcdsDynamicEx]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|0xBABEBABE|UINT32|0xAF35F3B2
```
When we were investigating PCD override via DSC file we've used `PcdsDynamicDefault` and `PcdsDynamicExDefault` section names:
```
[PcdsDynamicDefault]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32|0x11111111

[PcdsDynamicExDefault]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|0x22222222
```
The section name in this case plays important role. It sets the PCD storage method. The EDKII supports 3 storage methods for Dynamic/DynamicEx PCDs:
- `Default` - PCDs of this kind are stored in temporary memory (`[PcdsDynamicDefault]/[PcdsDynamicExDefault]`)
- `Hii` - PCDs of this kind are stored in EFI variables (`[PcdsDynamicHii]/[PcdsDynamicExHii]`)
- `Vpd` - PCDs of this kind are stored in read-only VPD data (`[PcdsDynamicVpd]/[PcdsDynamicExVpd]`)

So far we've worked only with `Default` PCDs. Initial values for these PCDs are always the same on each boot, it is the values encoded in the PCD database. All the PCD value changes happen in the temporary memory, so on the next boot you start from defaults.

Now let's try to investigate `Hii` PCDs.

For this storage method PCD are declared this way:
```
<TokenGuid>.<TokenName>|<EFI Var Name>|<EFI Var GUID>|<Offset in EFI Var>[|<Override value>[|<EFI Var attributes>]]
```

For example let's use this code in the DSC file
```
[PcdsDynamicHii]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32|L"MyHiiVar"|gUefiLessonsPkgTokenSpaceGuid|0x5|0x11111111|BS,RT

[PcdsDynamicExHii]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|L"MyExHiiVar"|gUefiLessonsPkgTokenSpaceGuid|0x7|0x22222222|BS
```
Here we've used the same GUID for the EFI var as we use for the PCD token space, but this is not mondatory.

Now additionaly correct PCDLesson application, so it would only get these PCDs, without any modifications:
```
if (PcdToken(PcdDynamicInt32)) {
  Print(L"PcdDynamicInt32=0x%x\n", PcdGet32(PcdDynamicInt32));
} else {
  Print(L"PcdDynamicInt32 token is unassigned\n");
}
Print(L"PcdDynamicExInt32=0x%x\n", PcdGetEx32(&gUefiLessonsPkgTokenSpaceGuid, PcdDynamicExInt32));
```

Rebuild OVMF and copy updated `PCDLesson.efi` application to the shared folder:
```
build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5

$ cp Build/OvmfX64/RELEASE_GCC5/X64/PCDLesson.efi ~/UEFI_disk/
```

Check the `parse_pcd_db` output:
```
$ parse_pcd_db \
--peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
--dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw"

...

38:
Token type = HII
Datum type = UINT32
HII VARIABLE
Guid:
150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
Name:
4d 00 79 00 48 00 69 00 69 00 56 00 61 00 72 00  | M.y.H.i.i.V.a.r.
00 00                                            | ..
Attributes:
RT+BS
Offset:
0x0005
Value:
0x11111111 (=286331153)

...

42:
Token type = HII
Datum type = UINT32
DynamicEx Token = 0xaf35f3b2
DynamicEx GUID  = 150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
HII VARIABLE
Guid:
150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
Name:
4d 00 79 00 45 00 78 00 48 00 69 00 69 00 56 00  | M.y.E.x.H.i.i.V.
61 00 72 00 00 00                                | a.r...
Attributes:
BS
Offset:
0x0007
Value:
0x22222222 (=572662306)
```

Now launch OVMF. You can check `DumpDynPcd.efi` output, but it doesn't show anything specific about the PCD storage methood:
```
FS0:\> DumpDynPcd.efi
...
Default Token Space
  Token = 0x00000026 - Type = UINT32:DYNAMIC    - Size = 0x4 - Value = 0x11111111
...
150CAB53-AD47-4385-B5DD-BCFC76BACAF0
  Token = 0xAF35F3B2 - Type = UINT32:DYNAMICEX  - Size = 0x4 - Value = 0x22222222
```

Check the EFI Variables under the `gUefiLessonsPkgTokenSpaceGuid`:
```
FS0:\> dmpstore -guid 150cab53-ad47-4385-b5dd-bcfc76bacaf0
dmpstore: No matching variables found. Guid 150CAB53-AD47-4385-B5DD-BCFC76BACAF0
```

So out PCDs didn't create any variables.

Let's execute our `PCDLesson.efi` application that right now only use `PcdGet*` functions:
```
FS0:\> PCDLesson.efi
...
PcdDynamicInt32=0x11111111
PcdDynamicExInt32=0x22222222
```

And check the EFI variables again:
```
FS0:\> dmpstore -guid 150cab53-ad47-4385-b5dd-bcfc76bacaf0
dmpstore: No matching variables found. Guid 150CAB53-AD47-4385-B5DD-BCFC76BACAF0
```

The point of this is that simple `PcdGet*` doesn't work with EFI variable interface if there is no EFI variable beforehand. In this case `PcdGet*` simply returns default value from the PCD Database.

Now let's return the `PcdSet*` code to out `PCDLesson.efi` application:
```
if (PcdToken(PcdDynamicInt32)) {
  Print(L"PcdDynamicInt32=0x%x\n", PcdGet32(PcdDynamicInt32));
  Status = PcdSet32S(PcdDynamicInt32, 0xBEEFBEEF);
  Print(L"Status=%r\n", Status);
  Print(L"PcdDynamicInt32=0x%x\n", PcdGet32(PcdDynamicInt32));
} else {
  Print(L"PcdDynamicInt32 token is unassigned\n");
}

Print(L"PcdDynamicExInt32=0x%x\n", PcdGetEx32(&gUefiLessonsPkgTokenSpaceGuid, PcdDynamicExInt32));

PcdSetEx32S(&gUefiLessonsPkgTokenSpaceGuid, PcdDynamicExInt32, 0x77777777);
Print(L"Status=%r\n", Status);
Print(L"PcdDynamicExInt32=0x%x\n", PcdGetEx32(&gUefiLessonsPkgTokenSpaceGuid, PcdDynamicExInt32));
```

Again rebuild OVMF and copy updated `PCDLesson.efi` application to the shared folder:
```
build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5

$ cp Build/OvmfX64/RELEASE_GCC5/X64/PCDLesson.efi ~/UEFI_disk/
```

Repeat our experiment:
```
FS0:\> dmpstore -guid 150cab53-ad47-4385-b5dd-bcfc76bacaf0
dmpstore: No matching variables found. Guid 150CAB53-AD47-4385-B5DD-BCFC76BACAF0

FS0:\> PCDLesson.efi
...
PcdDynamicInt32=0x11111111
Status=Success
PcdDynamicInt32=0xBEEFBEEF
PcdDynamicExInt32=0x22222222
Status=Success
PcdDynamicExInt32=0x77777777

FS0:\> dmpstore -guid 150cab53-ad47-4385-b5dd-bcfc76bacaf0
Variable BS '150CAB53-AD47-4385-B5DD-BCFC76BACAF0:MyExHiiVar' DataSize = 0x0B
  00000000: 00 00 00 00 00 00 00 77-77 77 77                 *.......wwww*
Variable RT+BS '150CAB53-AD47-4385-B5DD-BCFC76BACAF0:MyHiiVar' DataSize = 0x09
  00000000: 00 00 00 00 00 EF BE EF-BE                       *.........*
```

As you can see the `PcdSet*` statements have created EFI variables that were not present before. The variables were created with a minimal possible size = `Offset in EFI Var` + `sizeof(value)`.
In our case it is `5 + 4 = 9 = 0x09` and `7 + 4 = 11 = 0x0B`.
You can see how all the field from the PCD statement in the DSC are transformed to the EFI variable settings.

# `Non-volatile` variables

As we didn't give our variables the `NV` attributes they wouldn't be present on the next OVMF reboot from the start.
```
FS0:\> dmpstore -guid 150cab53-ad47-4385-b5dd-bcfc76bacaf0
dmpstore: No matching variables found. Guid 150CAB53-AD47-4385-B5DD-BCFC76BACAF0
```
So let's add `NV` attribute to our PCDs:
```
[PcdsDynamicHii]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32|L"MyHiiVar"|gUefiLessonsPkgTokenSpaceGuid|0x5|0x11111111|BS,RT,NV

[PcdsDynamicExHii]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|L"MyExHiiVar"|gUefiLessonsPkgTokenSpaceGuid|0x7|0x22222222|BS,NV
```

Rebuild OVMF and repeat our test:
```
FS0:\> dmpstore -guid 150cab53-ad47-4385-b5dd-bcfc76bacaf0
dmpstore: No matching variables found. Guid 150CAB53-AD47-4385-B5DD-BCFC76BACAF0

FS0:\> PCDLesson.efi
PcdDynamicInt32=0x11111111
Status=Success
PcdDynamicInt32=0xBEEFBEEF
PcdDynamicExInt32=0x22222222
Status=Success
PcdDynamicExInt32=0x77777777

FS0:\> dmpstore -guid 150cab53-ad47-4385-b5dd-bcfc76bacaf0
Variable NV+BS '150CAB53-AD47-4385-B5DD-BCFC76BACAF0:MyExHiiVar' DataSize = 0x0B
  00000000: 00 00 00 00 00 00 00 77-77 77 77                 *.......wwww*
Variable NV+RT+BS '150CAB53-AD47-4385-B5DD-BCFC76BACAF0:MyHiiVar' DataSize = 0x09
  00000000: 00 00 00 00 00 EF BE EF-BE                       *.........*
```

Now restart OVMF and check EFI variables content:
```
FS0:\> dmpstore -guid 150cab53-ad47-4385-b5dd-bcfc76bacaf0
Variable NV+BS '150CAB53-AD47-4385-B5DD-BCFC76BACAF0:MyExHiiVar' DataSize = 0x0B
  00000000: 00 00 00 00 00 00 00 77-77 77 77                 *.......wwww*
Variable NV+RT+BS '150CAB53-AD47-4385-B5DD-BCFC76BACAF0:MyHiiVar' DataSize = 0x09
  00000000: 00 00 00 00 00 EF BE EF-BE                       *.........*
```

As you can see now it is present from the start. And `PcdGet*`/`PcdSet*` functions successfully use it:
```
FS0:\> PCDLesson.efi
...
PcdDynamicInt32=0xBEEFBEEF
Status=Success
PcdDynamicInt32=0xBEEFBEEF
PcdDynamicExInt32=0x77777777
Status=Success
PcdDynamicExInt32=0x77777777
```

So the PCD default values are used only if there is no EFI variable. If the according EFI variable is present in the system the `PcdGet*` functions just read its content.


# Simplified override

It is possible to omit EFI variable attributes or value override in the PCD desclaration. For example:
```
[PcdsDynamicHii]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicInt32|L"MyHiiVar"|gUefiLessonsPkgTokenSpaceGuid|0x5

[PcdsDynamicExHii]
  gUefiLessonsPkgTokenSpaceGuid.PcdDynamicExInt32|L"MyExHiiVar"|gUefiLessonsPkgTokenSpaceGuid|0x7|0x22222222
```

Rebuild OVMF and check the `parse_pcd_db` output:
```
$ parse_pcd_db \
--peidb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Pei/Pcd/OUTPUT/PEIPcdDataBase.raw" \
--dxedb "Build/OvmfX64/RELEASE_GCC5/X64/MdeModulePkg/Universal/PCD/Dxe/Pcd/OUTPUT/DXEPcdDataBase.raw"

...

38:
Token type = HII
Datum type = UINT32
HII VARIABLE
Guid:
150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
Name:
4d 00 79 00 48 00 69 00 69 00 56 00 61 00 72 00  | M.y.H.i.i.V.a.r.
00 00                                            | ..
Attributes:
NV+RT+BS
Offset:
0x0005
Value:
0xcafecafe (=3405695742)

...

42:
Token type = HII
Datum type = UINT32
DynamicEx Token = 0xaf35f3b2
DynamicEx GUID  = 150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
HII VARIABLE
Guid:
150cab53-ad47-4385-b5ddbcfc76bacaf0 [gUefiLessonsPkgTokenSpaceGuid]
Name:
4d 00 79 00 45 00 78 00 48 00 69 00 69 00 56 00  | M.y.E.x.H.i.i.V.
61 00 72 00 00 00                                | a.r...
Attributes:
NV+RT+BS
Offset:
0x0007
Value:
0x22222222 (=572662306)
```

As you can see from this example:
- by default EFI Variable gets attributes `NV+RT+BS`
- if the override value is not provided, the EDK2 just uses the value from the INF/DEC

# `Hii` PCDs in PEI and DXE stages

It is important to note that it is forbidden to use `PcdSet*` for `Hii` PCDs in PEI stage. It would cause assert and `EFI_INVALID_PARAMETER` error [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Pei/Service.c](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/PCD/Pei/Service.c)

Generally the `Hii` PCD rules can be simplified to this table:
```
-------------------------------------------------------------------------------------------------------------------------------
|          |                  PEI                                  |                     DXE                                  |
-------------------------------------------------------------------------------------------------------------------------------
| PcdGet*  |  If there is a EFI var, return the content from it    |  If there is a EFI var, return the content from it       |
|          |  If there is no EFI var, return PCD default           |  If there is no EFI var, return PCD default              |
|----------|-------------------------------------------------------|----------------------------------------------------------|
| PcdSet*  |                   X                                   |  If there is a EFI var, modify PCD part                  |
|          |                                                       |  If there is no EFI var, create EFI var of minimal size  |
-------------------------------------------------------------------------------------------------------------------------------
```

