In one of the previous lessons we've looked at the HII resources from the `PlatformDxe` module https://github.com/tianocore/edk2/blob/master/OvmfPkg/PlatformDxe. You could notice that this resources were registered with a GUID number that is not referenced neither in the package DEC file, nor in any *.c file:
```
PackageList[9]: GUID=D9DCC5DF-4007-435E-9098-8970935504B2; size=0x855
        Package[0]: type=FORMS; size=0x1F6
        Package[1]: type=STRINGS; size=0x62B
        Package[2]: type=DEVICE_PATH; size=0x1C
        Package[3]: type=END; size=0x4
```

The GUID value is identical to the value of the `FILE_GUID` in the module INF file https://github.com/tianocore/edk2/blob/master/OvmfPkg/PlatformDxe/Platform.inf
```
[Defines]
  ...
  BASE_NAME                      = PlatformDxe
  FILE_GUID                      = D9DCC5DF-4007-435E-9098-8970935504B2
  ...
  
```

How to get this GUID to the C code? With the EDKII build system is pretty simple. The GUID from the INF file `FILE_GUID` key automatically goes to the Autoconf headers along with the couple of other values https://github.com/tianocore/edk2/blob/master/BaseTools/Source/Python/AutoGen/GenC.py:
```
def CreateHeaderCode(Info, AutoGenC, AutoGenH):
    ...
    #
    # Publish the CallerId Guid
    #
    AutoGenC.Append('\nGLOBAL_REMOVE_IF_UNREFERENCED GUID gEfiCallerIdGuid = %s;\n' % GuidStringToGuidStructureString(Info.Guid))
    AutoGenC.Append('\nGLOBAL_REMOVE_IF_UNREFERENCED GUID gEdkiiDscPlatformGuid = %s;\n' % GuidStringToGuidStructureString(Info.PlatformInfo.Guid))
    AutoGenC.Append('\nGLOBAL_REMOVE_IF_UNREFERENCED CHAR8 *gEfiCallerBaseName = "%s";\n' % Info.Name)
```

You could see these values in the `Build/OvmfX64/RELEASE_GCC5/X64/OvmfPkg/PlatformDxe/Platform/DEBUG/AutoGen.h`:
```
extern GUID  gEfiCallerIdGuid;
extern GUID  gEdkiiDscPlatformGuid;
extern CHAR8 *gEfiCallerBaseName;

#define EFI_CALLER_ID_GUID \
  {0xD9DCC5DF, 0x4007, 0x435E, {0x90, 0x98, 0x89, 0x70, 0x93, 0x55, 0x04, 0xB2}}
#define EDKII_DSC_PLATFORM_GUID \
  {0x5a9e7754, 0xd81b, 0x49ea, {0x85, 0xad, 0x69, 0xea, 0xa7, 0xb1, 0x53, 0x9b}}
```

And in the `Build/OvmfX64/RELEASE_GCC5/X64/OvmfPkg/PlatformDxe/Platform/DEBUG/AutoGen.c`:
```
GLOBAL_REMOVE_IF_UNREFERENCED GUID gEfiCallerIdGuid = {0xD9DCC5DF, 0x4007, 0x435E, {0x90, 0x98, 0x89, 0x70, 0x93, 0x55, 0x04, 0xB2}};

GLOBAL_REMOVE_IF_UNREFERENCED GUID gEdkiiDscPlatformGuid = {0x5a9e7754, 0xd81b, 0x49ea, {0x85, 0xad, 0x69, 0xea, 0xa7, 0xb1, 0x53, 0x9b}};

GLOBAL_REMOVE_IF_UNREFERENCED CHAR8 *gEfiCallerBaseName = "PlatformDxe";
```

These 3 variables are present in Autoconf files for every module, including ours:
- `gEfiCallerIdGuid` - value of the `FILE_GUID` key in the module INF file,
- `gEdkiiDscPlatformGuid` - value of the `PLATFORM_GUID` key in the module package DSC file,
- `gEfiCallerBaseName` - value of the `BASE_NAME` key in the module INF file, encoded in ASCII string

You can see that the `gEfiCallerIdGuid` is really used in the https://github.com/tianocore/edk2/blob/master/OvmfPkg/PlatformDxe/Platform.c:
```
EFI_STATUS
EFIAPI
PlatformInit (
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
  ...
  mInstalledPackages = HiiAddPackages (
                         &gEfiCallerIdGuid,  // PackageListGuid
                         ImageHandle,        // associated DeviceHandle
                         PlatformDxeStrings, // 1st package
                         PlatformFormsBin,   // 2nd package
                         NULL                // terminator
                         );
  ...
```

It doesn't look like `gEdkiiDscPlatformGuid` is used anywhere in the EDKII codebase, but you could use it if you want in the same way like any other GUID.

As for the `gEfiCallerBaseName` it is often used in the debug prints:
```
DEBUG ((DEBUG_ERROR, "%a: %a: MyFunction(): %r\n",  gEfiCallerBaseName, __FUNCTION__, Status));
```
As the `gEfiCallerBaseName` string is encoded in ASCII (`=CHAR8*`), to print it it is necessary to use `%a` formatting.

