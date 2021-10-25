In the last lesson we've declared our UNI strings file under the `Sources` section of the application INF file:
```
[Sources]
  ...
  Strings.uni
```
But within the EDKII codebase you could encounter couple of other methods for including UNI files:
- as the value for the `MODULE_UNI_FILE` key within the INF file,
- under the `[UserExtensions.TianoCore."ExtraFiles"]` section of the INF file,
- as the value for the `PACKAGE_UNI_FILE` key within the DEC file,
- under the `[UserExtensions.TianoCore."ExtraFiles"]` section of the DEC file.

The most important thing that UNI files included with these methods aren't processed by EDKII build tools! It is the files associated with the UEFI Packaging Specification (https://uefi.org/sites/default/files/resources/Dist_Package_Spec_1_1.pdf). They are used only by the `The Intel(R) UEFI Packaging Tool`. If you don't use it to distribute your packages, you can ignore all these files.

As I don't use it myself, I won't go into the details, but here are the examples how these files look like.

# MODULE_UNI_FILE/[UserExtensions.TianoCore."ExtraFiles"] within the INF file:

Let's look at the https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/ResetSystemPei/ResetSystemPei.inf
```
[Defines]
 ...
 MODULE_UNI_FILE                = ResetSystemPei.uni

...

[UserExtensions.TianoCore."ExtraFiles"]
  ResetSystemPeiExtra.uni
```
Usually UNI file under the `MODULE_UNI_FILE` key has the same name as the INF file.

And such file contains only 2 strings: `STR_MODULE_ABSTRACT` and `STR_MODULE_DESCRIPTION`.
Example: https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/ResetSystemPei/ResetSystemPei.uni
```
#string STR_MODULE_ABSTRACT             #language en-US "Implements Reset2, ResetFilter and ResetHandler PPIs"

#string STR_MODULE_DESCRIPTION          #language en-US "This driver implements Reset2, ResetFilter and ResetHandler PPIs."
```

UNI file under the `[UserExtensions.TianoCore."ExtraFiles"]` section usually has the same name as the INF file plus `Extra` postfix.

And such file contains only 1 string: `STR_PROPERTIES_MODULE_NAME`.
Example: https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/ResetSystemPei/ResetSystemPeiExtra.uni
```
#string STR_PROPERTIES_MODULE_NAME
#language en-US
"Reset System PEIM"
```

# PACKAGE_UNI_FILE/[UserExtensions.TianoCore."ExtraFiles"] within the DEC file:

For the example you could look at the https://github.com/tianocore/edk2/blob/master/MdeModulePkg/MdeModulePkg.dec:
```
[Defines]
  PACKAGE_UNI_FILE = MdeModulePkg.uni

...

[UserExtensions.TianoCore."ExtraFiles"]
  MdeModulePkgExtra.uni
```
Here you can see that the naming scheme is similar to the one that was used in the INF files:
- the file declared as the value of the `PACKAGE_UNI_FILE` key is named exactly like the DEC file,
- the file declared under the `[UserExtensions.TianoCore."ExtraFiles"]` is named the same plus the `Extra` postfix

Content of these files is also similar to the INF files. The only difference that `<MODULE>` is changed to `<PACKAGE>` in all the string token names:

- MdeModulePkg.uni (https://github.com/tianocore/edk2/blob/master/MdeModulePkg/MdeModulePkg.uni)
```
#string STR_PACKAGE_ABSTRACT            #language en-US "Provides the modules that conform to UEFI/PI Industry standards"

#string STR_PACKAGE_DESCRIPTION         #language en-US "It also provides the definitions (including PPIs/PROTOCOLs/GUIDs and library classes) and libraries instances, which are used for those modules."
```
- MdeModulePkgExtra.uni (https://github.com/tianocore/edk2/blob/master/MdeModulePkg/MdeModulePkgExtra.uni)
```
#string STR_PROPERTIES_PACKAGE_NAME
#language en-US
"MdeModule package"
```

Besides of that you can observe some doc strings for the PCDs in the MdeModulePkg.uni:
```
#string STR_gEfiMdeModulePkgTokenSpaceGuid_PcdProgressCodeOsLoaderLoad_PROMPT  #language en-US "Progress Code for OS Loader LoadImage start."

#string STR_gEfiMdeModulePkgTokenSpaceGuid_PcdProgressCodeOsLoaderLoad_HELP  #language en-US "Progress Code for OS Loader LoadImage start.<BR><BR>\n"
                                                                                             "PROGRESS_CODE_OS_LOADER_LOAD   = (EFI_SOFTWARE_DXE_BS_DRIVER | (EFI_OEM_SPECIFIC | 0x00000000)) = 0x03058000<BR>"

#string STR_gEfiMdeModulePkgTokenSpaceGuid_ERR_80000003 #language en-US "Incorrect progress code provided."
```

# Conclusion

Again, all these UNI files are irrelevant to the build tools. But these files are present all over the EDKII codebase, so you should at least be familiar with these concepts.
