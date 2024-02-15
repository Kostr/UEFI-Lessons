In this lesson we would try to add UNI translations dynamically to the system.

For example we have a `PlatformDxe` which has strings only for the English language https://github.com/tianocore/edk2/blob/master/OvmfPkg/PlatformDxe/Platform.uni:
```
#langdef en-US "English"

#string STR_FORMSET_TITLE        #language en-US "OVMF Platform Configuration"
#string STR_FORMSET_HELP         #language en-US "Change various OVMF platform settings."
#string STR_MAIN_FORM_TITLE      #language en-US "OVMF Settings"
#string STR_RES_CUR              #language en-US "Preferred Resolution at Next Boot"
#string STR_RES_CUR_HELP         #language en-US "The preferred resolution of the Graphics Console at next boot. It might be unset, or even invalid (hence ignored) wrt. the video RAM size."
#string STR_RES_NEXT             #language en-US "Change Preferred Resolution for Next Boot"
#string STR_RES_NEXT_HELP        #language en-US "You can specify a new preference for the Graphics Console here. The list is filtered against the video RAM size."
#string STR_SAVE_EXIT            #language en-US "Commit Changes and Exit"
#string STR_DISCARD_EXIT         #language en-US "Discard Changes and Exit"
```

This means that even if we change our language preference to French in the BIOS menu, these strings still would be printed in English. You can verify it.

With a help of a `Select Language` option change language to `Francais`

![Translations1](Translations1.png?raw=true "Translations1")

Then look at the `Device Manager->OVMF Platform Configuration`:

![Translations2](Translations2.png?raw=true "Translations2")

As you remember one string package has translation strings only for one language. If HII element supports multiple languages it would have multiple STRING packages in its package list. As for the `PlatformDxe` it has only one string package in its package list: 
https://github.com/tianocore/edk2/blob/master/OvmfPkg/PlatformDxe/Platform.inf
```
PackageList[9]: GUID=D9DCC5DF-4007-435E-9098-8970935504B2; size=0x855
        Package[0]: type=FORMS; size=0x1F6
        Package[1]: type=STRINGS; size=0x62B
        Package[2]: type=DEVICE_PATH; size=0x1C
        Package[3]: type=END; size=0x4
```

First let's print all possible strings from the package list. We saw the UNI source, but it is better to check this, as some strings can be added dynamilly in the system.

To print strings from the package list we need `EFI_HII_HANDLE` of the package list. For this we can utilize `ListPackageLists` function from the `EFI_HII_DATABASE_PROTOCOL`:

```
EFI_HII_DATABASE_PROTOCOL.ListPackageLists()

Summary:
Determines the handles that are currently active in the database.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_HII_DATABASE_LIST_PACKS) (
 IN CONST EFI_HII_DATABASE_PROTOCOL *This,
 IN UINT8 PackageType,
 IN CONST EFI_GUID *PackageGuid,
 IN OUT UINTN *HandleBufferLength,
 OUT EFI_HII_HANDLE *Handle
 );

Parameters:
This			A pointer to the EFI_HII_DATABASE_PROTOCOL instance.
PackageType		Specifies the package type of the packages to list or EFI_HII_PACKAGE_TYPE_ALL
			for all packages to be listed.
PackageGuid		If PackageType is EFI_HII_PACKAGE_TYPE_GUID, then this is the pointer to the
			GUID which must match the Guid field of EFI_HII_GUID_PACKAGE_HDR. Otherwise, it must be NULL.
HandleBufferLength	On input, a pointer to the length of the handle buffer. On output, the length of the
			handle buffer that is required for the handles found.
Handle			An array of EFI_HII_HANDLE instances returned. Type EFI_HII_HANDLE is
			defined in EFI_HII_DATABASE_PROTOCOL.NewPackageList() in the Packages section.

Description:
This function returns a list of the package handles of the specified type that are currently active in the
database. The pseudo-type EFI_HII_PACKAGE_TYPE_ALL will cause all package handles to be listed
```

With this function we need to call it once with a `HandleBufferLength=0`, receive `EFI_BUFFER_TOO_SMALL` error, but get the value for the `HandleBufferLength`. Allocate necessary size and call this function again. Standard UEFI mechanics which is already tedious.

To make things easier let's utilize `HiiGetHiiHandles` function from the `HiiLib`:

```
/**
  Retrieves the array of all the HII Handles or the HII handles of a specific
  package list GUID in the HII Database.
  This array is terminated with a NULL HII Handle.
  This function allocates the returned array using AllocatePool().
  The caller is responsible for freeing the array with FreePool().
  @param[in]  PackageListGuid  An optional parameter that is used to request
                               HII Handles associated with a specific
                               Package List GUID.  If this parameter is NULL,
                               then all the HII Handles in the HII Database
                               are returned.  If this parameter is not NULL,
                               then zero or more HII Handles associated with
                               PackageListGuid are returned.
  @retval NULL   No HII handles were found in the HII database
  @retval NULL   The array of HII Handles could not be retrieved
  @retval Other  A pointer to the NULL terminated array of HII Handles
**/
EFI_HII_HANDLE *
EFIAPI
HiiGetHiiHandles (
  IN CONST EFI_GUID  *PackageListGuid  OPTIONAL
  )
```

With it our application can be as simple as this:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_GUID PackageGuid = {0xD9DCC5DF, 0x4007, 0x435E, {0x90, 0x98, 0x89, 0x70, 0x93, 0x55, 0x04, 0xB2 }};
  EFI_HII_HANDLE* Handle = HiiGetHiiHandles(&PackageGuid);

  for (UINTN i=0; i<0xFFFF; i++) {
    EFI_STRING String = HiiGetString(*Handle, i, "en-US");
    if (String != NULL) {
      Print(L"ID=%d, %s\n", i, String);
      FreePool(String);
    }
  }
  return EFI_SUCCESS;
}
```
Here you can see that to get strings we simply loop over all possible string tokens and call `HiiGetString` function.

If you build and execute this application now, you would get this:
```
FS0:\> HIIAddLocalization.efi
ID=1, English
ID=2, OVMF Platform Configuration
ID=3, Change various OVMF platform settings.
ID=4, OVMF Settings
ID=5, Preferred Resolution at Next Boot
ID=6, The preferred resolution of the Graphics Console at next boot. It might be unset, or even invalid (hence ignored) wrt. the video RAM size.
ID=7, Change Preferred Resolution for Next Boot
ID=8, You can specify a new preference for the Graphics Console here. The list is filtered against the video RAM size.
ID=9, Commit Changes and Exit
ID=10, Discard Changes and Exit
ID=11, 640x480
ID=12, 800x480
ID=13, 800x600
ID=14, 832x624
ID=15, 960x640
ID=16, 1024x600
ID=17, 1024x768
ID=18, 1152x864
ID=19, 1152x870
ID=20, 1280x720
ID=21, 1280x760
ID=22, 1280x768
ID=23, 1280x800
ID=24, 1280x960
ID=25, 1280x1024
ID=26, 1360x768
ID=27, 1366x768
ID=28, 1400x1050
ID=29, 1440x900
ID=30, 1600x900
ID=31, 1600x1200
ID=32, 1680x1050
ID=33, 1920x1080
ID=34, 1920x1200
ID=35, 1920x1440
ID=36, 2000x2000
ID=37, 2048x1536
ID=38, 2048x2048
ID=39, 2560x1440
ID=40, 2560x1600
```

Let's create an array with translations to the French language:
```
  CHAR16* FrenchStrings[] = {
    L"Configuration de la OVMF plateforme",
    L"Modifier divers paramètres de la plateforme OVMF",
    L"Paramètres OVMF",
    L"Résolution préférée au prochain démarrage",
    L"La résolution préférée de la console graphique au prochain démarrage. Il peut être non défini, ou même invalide (donc ignoré) wrt. la taille de la RAM vidéo.",
    L"Modifier la résolution préférée pour le prochain démarrage",
    L"Vous pouvez spécifier ici une nouvelle préférence pour la console graphique. La liste est filtrée en fonction de la taille de la RAM vidéo.",
    L"Valider les modifications et quitter",
    L"Annuler les changements et quitter",
    L"640x480",
    L"800x480",
    L"800x600",
    L"832x624",
    L"960x640",
    L"1024x600",
    L"1024x768",
    L"1152x864",
    L"1152x870",
    L"1280x720",
    L"1280x760",
    L"1280x768",
    L"1280x800",
    L"1280x960",
    L"1280x1024",
    L"1360x768",
    L"1366x768",
    L"1400x1050",
    L"1440x900",
    L"1600x900",
    L"1600x1200",
    L"1680x1050",
    L"1920x1080",
    L"1920x1200",
    L"1920x1440",
    L"2000x2000",
    L"2048x1536",
    L"2048x2048",
    L"2560x1440",
    L"2560x1600",
  };
```
Now let's comment the strings print loop. Next we need to create a strings package and populate it to the HII database under the same Package List. Here we can use one trick. Instead of a manual string package creation we can call `gHiiString->NewString` function with the dummy `L""` string in the new language. If `en-US` string package has 40 strings, this would create `fr-FR` string package with only one string `L""` with an ID=41. And this package would be automatically added to the package list.

After the `fr-FR` string package is present in the package list, we can simply call `gHiiString->SetString` function for all of the data strings:
```
  EFI_STATUS Status;
  for (UINTN i=0; i<(sizeof(FrenchStrings)/sizeof(FrenchStrings[0])); i++) {
    EFI_STRING_ID StringId;
    if (i==0) {
      Status = gHiiString->NewString(gHiiString, *Handle, &StringId, "fr-FR", L"French", L"", NULL);
      if (EFI_ERROR(Status)) {
        Print(L"Error! NewString fail\n");
      }
    }
    StringId = i+2;
    Status = gHiiString->SetString(gHiiString, *Handle, StringId, "fr-FR", FrenchStrings[i], NULL);
    if (EFI_ERROR(Status)) {
      Print(L"Error! SetString fail for ID=%d\n", StringId);
    }
  }
```

Just in case here are the API description for the `NewString` and `SetString` functions from the `EFI_HII_STRING_PROTOCOL`:
```
EFI_HII_STRING_PROTOCOL.SetString()

Summary:
Change information about the string.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_HII_SET_STRING) (
 IN CONST EFI_HII_STRING_PROTOCOL *This,
 IN EFI_HII_HANDLE PackageList,
 IN EFI_STRING_ID StringId,
 IN CONST CHAR8 *Language,
 IN CONST EFI_STRING String,
 IN CONST EFI_FONT_INFO *StringFontInfo OPTIONAL
 );

Parameters:
This		A pointer to the EFI_HII_STRING_PROTOCOL instance.
PackageList	The package list containing the strings.
Language	Points to the language for the updated string.
StringId	The string id, which is unique within PackageList.
String		Points to the new null-terminated string.
StringFontInfo	Points to the string’s font information or NULL if the string font information is not changed.

Description:
This function updates the string specified by StringId in the specified PackageList to the text specified by
String and, optionally, the font information specified by StringFontInfo
```
```
EFI_HII_STRING_PROTOCOL.NewString()

Summary:
Creates a new string in a specific language and add it to strings from a specific package list.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_HII_NEW_STRING) (
 IN CONST EFI_HII_STRING_PROTOCOL *This,
 IN EFI_HII_HANDLE PackageList,
 OUT EFI_STRING_ID *StringId
 IN CONST CHAR8 *Language,
 IN CONST CHAR16 *LanguageName OPTIONAL,
 IN CONST EFI_STRING String,
 IN CONST EFI_FONT_INFO *StringFontInfo
 );

Parameters:
This		A pointer to the EFI_HII_STRING_PROTOCOL instance.
PackageList	Handle of the package list where this string will be added.
Language	Points to the language for the new string.
LanguageName	Points to the printable language name to associate with the passed in Language
		field.
String		Points to the new null-terminated string.
StringFontInfo	Points to the new string’s font information or NULL if the string should have the
		default system font, size and style.
StringId	On return, contains the new strings id, which is unique within PackageList.

DescriptionL
This function adds the string String to the group of strings owned by PackageList, with the specified font information StringFontInfo and returns a new string id
```

Don't forget to add `UefiHiiServicesLib` library class and its header `Library/UefiHiiServicesLib.h` for direct using of `gHiiString` protocol.

Now build and run our application now.
```
FS0:\> HIIAddLocalization.efi
FS0:\>
```
After that perform `exit` to go to the BIOS menu.

Use `Select Language` option to change language to `Francais`. Then got to the `DeviceManager->OVMF Platform Configuration`. You can already see that translation are in place:

![Translations4](Translations4.png?raw=true "Translations4")

If you exit the menu and then go to it again you could even see that even the string in the `Device manager` is changed to "Configuration de la OVMF plateforme":

![Translations3](Translations3.png?raw=true "Translations3")

