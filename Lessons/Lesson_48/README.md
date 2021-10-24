It was pretty hard to add new string packages using `NewPackageList` function directly.
Keep in mind that we've only added a couple of strings and we didn't even calculate the necessary data array for the Package list dynamically. Also if we would want to add fonts/forms/images/... we would need to investigate format of these packages and write necessary functions for them as well.

Let's check what EDKII can offer us to simplify these tasks. In this lesson particularly we would talk about how we can simplify our string packages creation.

# Create application

As usual create new application:
```
./createNewApp.sh HIIStringsUNI
```

Add it to our DSC package file UefiLessonsPkg/UefiLessonsPkg.dsc:
```
[Components]
  ...
  UefiLessonsPkg/HIIStringsUNI/HIIStringsUNI.inf
```

As last time, we would need a GUID for our package list, declare it in package DEC file UefiLessonsPkg/UefiLessonsPkg.dec:
```
[Guids]
  ...
  gHIIStringsUNIGuid = { 0x6ee19058, 0x0fe2, 0x44ed, { 0x89, 0x1c, 0xa5, 0xd7, 0xe1, 0x08, 0xee, 0x1a }}
```

And add it to the application INF file UefiLessonsPkg/HIIStringsUNI/HIIStringsUNI.inf:
```
[Packages]
  ...
  UefiLessonsPkg/UefiLessonsPkg.dec
  
...

[Guids]
  gHIIStringsUNIGuid
```

# UNI file

In EDKII you can define all translation strings in the files of a special UNI format. EDKII build utilities will parse data in these files and create array with String packages content.

Let's add `Strings.uni` file to the `Sources` section in our application INF file `UefiLessonsPkg/HIIStringsUNI/HIIStringsUNI.inf`. Keep in mind that you can use any name for the *.uni file and you can have as many *.uni as you want in your `Sources`:
```
[Sources]
  ...
  Strings.uni
```

Fill the content of this file `UefiLessonsPkg/HIIStringsUNI/Strings.uni`:
```
#langdef en-US "English"
#langdef fr-FR "Francais"

#string STR_HELLO         #language en-US  "Hello!"
                          #language fr-FR  "Bonjour!"

#string STR_BYE           #language en-US  "Bye!"
                          #language fr-FR  "Au revoir!"
```

This file would be a source for 2 string packages:
```
1) 'en-US' string package
ID 1: "English"
ID 2: "Hello!"
ID 3: "Bye!"
2) 'fr-FR' string package
ID 1: "Francais"
ID 2: "Bonjour!"
ID 3: "Au revoir!"
```
You can read more about UNI file format in the [Multi-String .UNI File Format Specification](https://edk2-docs.gitbook.io/edk-ii-uni-specification/).

If you build our application now, this file would be generated along with the usual build files
`Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIStringsUNI/HIIStringsUNI/DEBUG/HIIStringsUNIStrDefs.h`. The name of this file is formed from the `BASE_NAME` value in our INF file and basically it is `<BASE_NAME>StrDefs.h`.

If you look at this file you'll see:
```
extern unsigned char HIIStringsUNIStrings[];
```
This is the array with String packages data that we need. It is imposed that you would pass it to the `HiiAddPackages` function from the `HiiLib` to add HII packages to the database and essentially create new Package list:

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Library/HiiLib.h

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiHiiLib/HiiLib.c
```
/**
  Registers a list of packages in the HII Database and returns the HII Handle
  associated with that registration.  If an HII Handle has already been registered
  with the same PackageListGuid and DeviceHandle, then NULL is returned.  If there
  are not enough resources to perform the registration, then NULL is returned.
  If an empty list of packages is passed in, then NULL is returned.  If the size of
  the list of package is 0, then NULL is returned.
  The variable arguments are pointers that point to package headers defined
  by UEFI VFR compiler and StringGather tool.
  #pragma pack (push, 1)
  typedef struct {
    UINT32                  BinaryLength;
    EFI_HII_PACKAGE_HEADER  PackageHeader;
  } EDKII_AUTOGEN_PACKAGES_HEADER;
  #pragma pack (pop)
  @param[in]  PackageListGuid  The GUID of the package list.
  @param[in]  DeviceHandle     If not NULL, the Device Handle on which
                               an instance of DEVICE_PATH_PROTOCOL is installed.
                               This Device Handle uniquely defines the device that
                               the added packages are associated with.
  @param[in]  ...              The variable argument list that contains pointers
                               to packages terminated by a NULL.
  @retval NULL   An HII Handle has already been registered in the HII Database with
                 the same PackageListGuid and DeviceHandle.
  @retval NULL   The HII Handle could not be created.
  @retval NULL   An empty list of packages was passed in.
  @retval NULL   All packages are empty.
  @retval Other  The HII Handle associated with the newly registered package list.
**/
EFI_HII_HANDLE
EFIAPI
HiiAddPackages (
  IN CONST EFI_GUID    *PackageListGuid,
  IN       EFI_HANDLE  DeviceHandle  OPTIONAL,
  ...
  )
;
```

With these things package list creation can be as simple as this:
```
...

#include <Library/HiiLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_HII_HANDLE Handle = HiiAddPackages(&gHIIStringsUNIGuid,
                                         NULL,
                                         HIIStringsUNIStrings,
                                         NULL);

  if (Handle == NULL)
  {
    Print(L"Error! Can't perform HiiAddPackages\n");
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}
```

Don't forget to include `HiiLib` in the `LibraryClasses` section of our application INF file:
```
[Packages]
  ...
  MdeModulePkg/MdeModulePkg.dec

...

[LibraryClasses]
  ...
  HiiLib
```

If you build our application and run it under OVMF you coud see that indeed our code creates new Package list with 2 string packages:
```
FS0:\> HIIStringsUNI.efi
FS0:\> ShowHII.efi
...
PackageList[20]: GUID=6EE19058-0FE2-44ED-891C-A5D7E108EE1A; size=0xA6
        Package[0]: type=STRINGS; size=0x46
        Package[1]: type=STRINGS; size=0x48
        Package[2]: type=END; size=0x4
```

# HiiGetString

As we've already included `HiiLib` library, let's use its `HiiGetString` function to print our strings:

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Library/HiiLib.h

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiHiiLib/HiiString.c
```
/**
  Retrieves a string from a string package in a specific language specified in Language
  or in the best lanaguage. See HiiGetStringEx () for the details.
  @param[in]  HiiHandle  A handle that was previously registered in the HII Database.
  @param[in]  StringId   The identifier of the string to retrieved from the string
                         package associated with HiiHandle.
  @param[in]  Language   The language of the string to retrieve.  If this parameter
                         is NULL, then the current platform language is used.  The
                         format of Language must follow the language format assumed in
                         the HII Database.
  @retval NULL   The string specified by StringId is not present in the string package.
  @retval Other  The string was returned.
**/
EFI_STRING
EFIAPI
HiiGetString (
  IN EFI_HII_HANDLE  HiiHandle,
  IN EFI_STRING_ID   StringId,
  IN CONST CHAR8     *Language  OPTIONAL
  );
```

Let's add this code to our application:
```
Print(L"en-US ID=1: %s\n", HiiGetString(Handle, 1, "en-US"));
Print(L"en-US ID=2: %s\n", HiiGetString(Handle, 2, "en-US"));
Print(L"en-US ID=3: %s\n", HiiGetString(Handle, 3, "en-US"));
Print(L"fr-FR ID=1: %s\n", HiiGetString(Handle, 1, "fr-FR"));
Print(L"fr-FR ID=2: %s\n", HiiGetString(Handle, 2, "fr-FR"));
Print(L"fr-FR ID=3: %s\n", HiiGetString(Handle, 3, "fr-FR"));
```

If you build and execute our app now you would get:
```
FS0:\> HIIStringsUNI.efi
en-US ID=1: English
en-US ID=2: <null string>
en-US ID=3: <null string>
fr-FR ID=1: Francais
fr-FR ID=2: <null string>
fr-FR ID=3: <null string>
```

What is wrong? Why only the language strings (ID=1) were populated to String packages?

Let's look at the actual `HIIStringsUNIStrings` array data that is present in the file
Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIStringsUNI/HIIStringsUNI/DEBUG/AutoGen.c
```
//
//Unicode String Pack Definition
//
unsigned char HIIStringsUNIStrings[] = {

// STRGATHER_OUTPUT_HEADER
  0x92,  0x00,  0x00,  0x00,

// PACKAGE HEADER

  0x46,  0x00,  0x00,  0x04,  0x34,  0x00,  0x00,  0x00,  0x34,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x01,  0x00,  0x65,  0x6E,
  0x2D,  0x55,  0x53,  0x00,

// PACKAGE DATA

// 0x0001: $PRINTABLE_LANGUAGE_NAME:0x0001
  0x14,  0x45,  0x00,  0x6E,  0x00,  0x67,  0x00,  0x6C,  0x00,  0x69,  0x00,  0x73,  0x00,  0x68,  0x00,  0x00,
  0x00,
  0x00,
// PACKAGE HEADER

  0x48,  0x00,  0x00,  0x04,  0x34,  0x00,  0x00,  0x00,  0x34,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x01,  0x00,  0x66,  0x72,
  0x2D,  0x46,  0x52,  0x00,

// PACKAGE DATA

// 0x0001: $PRINTABLE_LANGUAGE_NAME:0x0001
  0x14,  0x46,  0x00,  0x72,  0x00,  0x61,  0x00,  0x6E,  0x00,  0x63,  0x00,  0x61,  0x00,  0x69,  0x00,  0x73,
  0x00,  0x00,  0x00,
  0x00,

};
```
Couple of things to notice:
- array contains only String data packages, it doesn't contain neither Package list header, nor End Package
- array has special 4 byte header `STRGATHER_OUTPUT_HEADER` - it contains size of the array including this header
- array indeed has only Language name strings in itself

The first two observations just to point out the format of the incoming argument for the `HiiAddPackages` function. You could look at the function implementation to see how the `STRGATHER_OUTPUT_HEADER` is used to construct Package list and call `NewPackageList` with appropriate data.

Now let's look at the `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIStringsUNI/HIIStringsUNI/DEBUG/HIIStringsUNIStrDefs.h` one more time, this time pay attention to this piece of code:
```
//
//Unicode String ID
//
// #define $LANGUAGE_NAME                                       0x0000 // not referenced
// #define $PRINTABLE_LANGUAGE_NAME                             0x0001 // not referenced
// #define STR_HELLO                                            0x0002 // not referenced
// #define STR_BYE                                              0x0003 // not referenced
```

This is the source of our problem, strings didn't go to the array, because their tokens simply weren't refernced.

The build tool `StrGather.py` that is responsible for the array data creation (https://github.com/tianocore/edk2/blob/master/BaseTools/Source/Python/AutoGen/StrGather.py) simply checks for macros `STRING_TOKEN(...)` in the application code, and populates only strings that are refernced in code:
```
STRING_TOKEN = re.compile('STRING_TOKEN *\(([A-Z0-9_]+) *\)', re.MULTILINE | re.UNICODE)
```
But as full language name is a mandatory field, it always gets populated. That is why we saw only it in our first application run.

Let's change our print code to this:
```
Print(L"en-US ID=1: %s\n", HiiGetString(Handle, 1, "en-US"));
Print(L"en-US ID=2: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_HELLO), "en-US"));
Print(L"en-US ID=3: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_BYE), "en-US"));
Print(L"fr-FR ID=1: %s\n", HiiGetString(Handle, 1, "fr-FR"));
Print(L"fr-FR ID=2: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_HELLO), "fr-FR"));
Print(L"fr-FR ID=3: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_BYE), "fr-FR"));
```

Now build the application and look at the generated files:

Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIStringsUNI/HIIStringsUNI/DEBUG/HIIStringsUNIStrDefs.h
```
//
//Unicode String ID
//
// #define $LANGUAGE_NAME                                       0x0000 // not referenced
// #define $PRINTABLE_LANGUAGE_NAME                             0x0001 // not referenced
#define STR_HELLO                                            0x0002
#define STR_BYE                                              0x0003
```
Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIStringsUNI/HIIStringsUNI/DEBUG/AutoGen.c
```
//
//Unicode String Pack Definition
//
unsigned char HIIStringsUNIStrings[] = {

// STRGATHER_OUTPUT_HEADER
  0xD6,  0x00,  0x00,  0x00,

// PACKAGE HEADER

  0x60,  0x00,  0x00,  0x04,  0x34,  0x00,  0x00,  0x00,  0x34,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x01,  0x00,  0x65,  0x6E,
  0x2D,  0x55,  0x53,  0x00,

// PACKAGE DATA

// 0x0001: $PRINTABLE_LANGUAGE_NAME:0x0001
  0x14,  0x45,  0x00,  0x6E,  0x00,  0x67,  0x00,  0x6C,  0x00,  0x69,  0x00,  0x73,  0x00,  0x68,  0x00,  0x00,
  0x00,
// 0x0002: STR_HELLO:0x0002
  0x14,  0x48,  0x00,  0x65,  0x00,  0x6C,  0x00,  0x6C,  0x00,  0x6F,  0x00,  0x21,  0x00,  0x00,  0x00,
// 0x0003: STR_BYE:0x0003
  0x14,  0x42,  0x00,  0x79,  0x00,  0x65,  0x00,  0x21,  0x00,  0x00,  0x00,
  0x00,
// PACKAGE HEADER

  0x72,  0x00,  0x00,  0x04,  0x34,  0x00,  0x00,  0x00,  0x34,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x01,  0x00,  0x66,  0x72,
  0x2D,  0x46,  0x52,  0x00,

// PACKAGE DATA

// 0x0001: $PRINTABLE_LANGUAGE_NAME:0x0001
  0x14,  0x46,  0x00,  0x72,  0x00,  0x61,  0x00,  0x6E,  0x00,  0x63,  0x00,  0x61,  0x00,  0x69,  0x00,  0x73,
  0x00,  0x00,  0x00,
// 0x0002: STR_HELLO:0x0002
  0x14,  0x42,  0x00,  0x6F,  0x00,  0x6E,  0x00,  0x6A,  0x00,  0x6F,  0x00,  0x75,  0x00,  0x72,  0x00,  0x21,
  0x00,  0x00,  0x00,
// 0x0003: STR_BYE:0x0003
  0x14,  0x41,  0x00,  0x75,  0x00,  0x20,  0x00,  0x72,  0x00,  0x65,  0x00,  0x76,  0x00,  0x6F,  0x00,  0x69,
  0x00,  0x72,  0x00,  0x21,  0x00,  0x00,  0x00,
  0x00,

};
```

As you can see this time our strings got into the `HIIStringsUNIStrings` array.

If you execute our application under OVMF now you would get:
```
FS0:\> HIIStringsUNI.efi
en-US ID=1: English
en-US ID=2: Hello!
en-US ID=3: Bye!
fr-FR ID=1: Francais
fr-FR ID=2: Bonjour!
fr-FR ID=3: Au revoir!
```
