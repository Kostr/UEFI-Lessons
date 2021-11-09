Now it is time to add our font data to the HII Database.

Initialize new application:
```
./createNewApp.sh HIIAddRussianFont
```
Add it to the package DSC file (UefiLessonsPkg/UefiLessonsPkg.dsc):
```
[Components]
  UefiLessonsPkg/HIIAddRussianFont/HIIAddRussianFont.inf
```

In our application we would populate package list to the HII database, so we need to declare GUID for this package list in the `UefiLessonsPkg/UefiLessonsPkg.dec`:
```
[Guids]
  ...
  gHIIAddRussianFontGuid = { 0x9fe2f616, 0x323c, 0x45a7, { 0x87, 0xa2, 0xdf, 0xef, 0xf5, 0x17, 0xcc, 0x66 }}
```
And declare that our app would need it in the `UefiLessonsPkg/HIIAddRussianFont/HIIAddRussianFont.inf`:
```
[Packages]
  ...
  UefiLessonsPkg/UefiLessonsPkg.dec

[Guids]
  gHIIAddRussianFontGuid
```
Also we would be using library for HII services (for example for `HiiAddPackages` function), so let's include it from the start:
```
[Packages]
  ...
  MdeModulePkg/MdeModulePkg.dec

[LibraryClasses]
  ...
  HiiLib
```

# String.uni file

Add strings UNI file to our application sources `UefiLessonsPkg/HIIAddRussianFont/HIIAddRussianFont.inf`:
```
[Sources]
  ...
  Strings.uni
```

Then create this `Strings.uni` file with these strings:
- "Hello!"
- "Bye!"
- Language alphabet in uppercase
- Language alphabet in lowercase
```
#langdef en-US "English"
#langdef fr-FR "Francais"
#langdef ru-RU "Russian"

#string STR_HELLO               #language en-US  "Hello!"
                                #language fr-FR  "Bonjour!"
                                #language ru-RU  "Привет!"

#string STR_BYE                 #language en-US  "Bye!"
                                #language fr-FR  "Au revoir!"
                                #language ru-RU  "До свидания!"

#string STR_ALPHABET_UPPERCASE  #language en-US  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                #language fr-FR  "ABCDEFGHIJKLMNOPQRSTUVWXYZÀÈÙÉÂÊÎÔÛËÏÜŸÆŒÇ"
                                #language ru-RU  "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЬЪЭЮЯ"

#string STR_ALPHABET_LOWERCASE  #language en-US  "abcdefghijklmnopqrstuvwxyz"
                                #language fr-FR  "abcdefghijklmnopqrstuvwxyzàèùéâêîôûëïüÿæœç"
                                #language ru-RU  "абвгдеёжзийклмнопрстуфхцчшщьъэюя"
```
The alphabet strings would help us to check if all the letters in the language are printed correctly.

# RussianFont.c

Add our file with russian font glyphs that we've recieved in the last lesson to our `UefiLessonsPkg/HIIAddRussianFont/HIIAddRussianFont.inf`:
```
[Sources]
  ...
  RussianFont.c
```

Just in case it is the file with this content:
```
EFI_WIDE_GLYPH gSimpleFontWideGlyphData[] = {
{ 0x00, 0x00, { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
};
UINT32 gSimpleFontWideBytes = sizeof(gSimpleFontWideGlyphData);

EFI_NARROW_GLYPH gSimpleFontNarrowGlyphData[] = {
{ 0x400, 0x00, { 0x60,0x30,0x00,0xfe,0x66,0x62,0x60,0x68,0x78,0x68,0x60,0x60,0x62,0x66,0xfe,0x00,0x00,0x00,0x00}},
{ 0x401, 0x00, { 0x66,0x66,0x00,0xfe,0x66,0x62,0x60,0x68,0x78,0x68,0x60,0x60,0x62,0x66,0xfe,0x00,0x00,0x00,0x00}},
{ 0x402, 0x00, { 0x00,0x00,0x00,0xfc,0x64,0x60,0x60,0x6c,0x76,0x66,0x66,0x66,0x66,0x66,0xe6,0x0c,0x00,0x00,0x00}},
{ 0x403, 0x00, { 0x0c,0x18,0x00,0xfe,0x66,0x62,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0xf0,0x00,0x00,0x00,0x00}},
{ 0x404, 0x00, { 0x00,0x00,0x00,0x3c,0x66,0xc2,0xc0,0xc8,0xf8,0xc8,0xc0,0xc0,0xc2,0x66,0x3c,0x00,0x00,0x00,0x00}},
...
{ 0x45c, 0x00, { 0x00,0x00,0x00,0x0c,0x18,0x30,0x00,0xe6,0x66,0x6c,0x78,0x78,0x6c,0x66,0xe6,0x00,0x00,0x00,0x00}},
{ 0x45d, 0x00, { 0x00,0x00,0x00,0x60,0x30,0x18,0x00,0xc6,0xc6,0xce,0xde,0xf6,0xe6,0xc6,0xc6,0x00,0x00,0x00,0x00}},
{ 0x45e, 0x00, { 0x00,0x00,0x00,0x00,0x6c,0x38,0x00,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x7e,0x06,0x06,0x0c,0xf8,0x00}},
{ 0x45f, 0x00, { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xee,0x6c,0x6c,0x6c,0x6c,0x6c,0x6c,0xfe,0x10,0x10,0x00,0x00}},
};
UINT32 gSimpleFontNarrowBytes = sizeof(gSimpleFontNarrowGlyphData);
```

# HIIAddRussianFont.c

Now it is time for the main code:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>

extern EFI_WIDE_GLYPH gSimpleFontWideGlyphData[];
extern UINT32 gSimpleFontWideBytes;
extern EFI_NARROW_GLYPH gSimpleFontNarrowGlyphData[];
extern UINT32 gSimpleFontNarrowBytes;


EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT8* FontPackage = CreateSimpleFontPkg(gSimpleFontWideGlyphData,
                                          gSimpleFontWideBytes,
                                          gSimpleFontNarrowGlyphData,
                                          gSimpleFontNarrowBytes);

  EFI_HII_HANDLE Handle = HiiAddPackages(&gHIIAddRussianFontGuid,
                                         NULL,
                                         FontPackage,
                                         HIIAddRussianFontStrings,
                                         NULL);

  FreePool(FontPackage);

  if (Handle == NULL)
  {
    Print(L"Error! Can't perform HiiAddPackages\n");
    return EFI_INVALID_PARAMETER;
  }

  Print(L"en-US ID=1: %s\n", HiiGetString(Handle, 1, "en-US"));
  Print(L"en-US ID=2: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_HELLO), "en-US"));
  Print(L"en-US ID=3: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_BYE), "en-US"));
  Print(L"en-US ID=4: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_ALPHABET_UPPERCASE), "en-US"));
  Print(L"en-US ID=5: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_ALPHABET_LOWERCASE), "en-US"));
  Print(L"fr-FR ID=1: %s\n", HiiGetString(Handle, 1, "fr-FR"));
  Print(L"fr-FR ID=2: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_HELLO), "fr-FR"));
  Print(L"fr-FR ID=3: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_BYE), "fr-FR"));
  Print(L"fr-FR ID=4: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_ALPHABET_UPPERCASE), "fr-FR"));
  Print(L"fr-FR ID=5: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_ALPHABET_LOWERCASE), "fr-FR"));
  Print(L"ru-RU ID=1: %s\n", HiiGetString(Handle, 1, "ru-RU"));
  Print(L"ru-RU ID=2: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_HELLO), "ru-RU"));
  Print(L"ru-RU ID=3: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_BYE), "ru-RU"));
  Print(L"ru-RU ID=4: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_ALPHABET_UPPERCASE), "ru-RU"));
  Print(L"ru-RU ID=5: %s\n", HiiGetString(Handle, STRING_TOKEN(STR_ALPHABET_LOWERCASE), "ru-RU"));

  return EFI_SUCCESS;
}
```
Here we:
- construct font package data from the Glyph arrays from the `RussianFont.c` file. We will define this `CreateSimpleFontPkg` function next,
- use `HiiAddPackages` to add both font package and application strings package (this function is variadic, it can take variable number of packages and push them all to create the new package list [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiHiiLib/HiiLib.c]),
- once we've added packages we no longer need `FontPackage`, so don't forget to free it with a `FreePool` function
- if everything is ok, use `HiiGetString` to print all the strings in all languages,

Now it is time to define our `CreateSimpleFontPkg` function. But first here are the necessary defines and types for the font package creation https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiInternalFormRepresentation.h:
```
///
/// A simplified font package consists of a font header
/// followed by a series of glyph structures.
///
typedef struct _EFI_HII_SIMPLE_FONT_PACKAGE_HDR {
  EFI_HII_PACKAGE_HEADER Header;
  UINT16                 NumberOfNarrowGlyphs;
  UINT16                 NumberOfWideGlyphs;
  // EFI_NARROW_GLYPH       NarrowGlyphs[];
  // EFI_WIDE_GLYPH         WideGlyphs[];
} EFI_HII_SIMPLE_FONT_PACKAGE_HDR;

///
/// The header found at the start of each package.
///
typedef struct {
  UINT32  Length:24;
  UINT32  Type:8;
  // UINT8  Data[...];
} EFI_HII_PACKAGE_HEADER;


//
// Value of HII package type
//
...
#define EFI_HII_PACKAGE_SIMPLE_FONTS         0x07
...
```

In our function we need to allocate necessary array for the font package. Besides allocating space for the package itself we need to add 4 bytes for the package size that would prepend all the data. As you remember this is the necessary data format for the `HiiAddPackages` function:
```
UINT8* CreateSimpleFontPkg(EFI_WIDE_GLYPH* WideGlyph,
                           UINT32 WideGlyphSizeInBytes,
                           EFI_NARROW_GLYPH* NarrowGlyph,
                           UINT32 NarrowGlyphSizeInBytes)
{
  UINT32 PackageLen = sizeof(EFI_HII_SIMPLE_FONT_PACKAGE_HDR) + WideGlyphSizeInBytes + NarrowGlyphSizeInBytes + 4;
  UINT8* FontPackage = (UINT8*)AllocateZeroPool (PackageLen);

  *(UINT32*)FontPackage = PackageLen;

  ...

  return FontPackage;
}
```

Now fill the package data. First fill the header:
```
EFI_HII_SIMPLE_FONT_PACKAGE_HDR *SimpleFont;
SimpleFont = (EFI_HII_SIMPLE_FONT_PACKAGE_HDR*)(FontPackage + 4);
SimpleFont->Header.Length = (UINT32)(PackageLen - 4);
SimpleFont->Header.Type = EFI_HII_PACKAGE_SIMPLE_FONTS;
SimpleFont->NumberOfNarrowGlyphs = (UINT16)(NarrowGlyphSizeInBytes / sizeof(EFI_NARROW_GLYPH));
SimpleFont->NumberOfWideGlyphs = (UINT16)(WideGlyphSizeInBytes / sizeof(EFI_WIDE_GLYPH));
```
And then copy the Glyphs data (for the `CopyMem` function you need to include `<Library/BaseMemoryLib.h>` header):
```
UINT8* Location = (UINT8*)(&SimpleFont->NumberOfWideGlyphs + 1);
CopyMem(Location, NarrowGlyph, NarrowGlyphSizeInBytes);
CopyMem(Location + NarrowGlyphSizeInBytes, WideGlyph, WideGlyphSizeInBytes);
```

If you build and run our application now, you would see that russian strings are printed correctly:

![AddFont1](AddFont1.png?raw=true "Add font 1")


Now let's try to print some string in russian at the beginning and at the end of our application:
```
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print(L"Привет!\n");

  ...

  Print(L"Привет!\n");

  return EFI_SUCCESS;
}
```
First print would produce only `!` as there are no information in the system about how to display russian unicode symbol glyphs. But at the end after we've added our font package, print would produce expected result:

![AddFont2](AddFont2.png?raw=true "Add font 2")
