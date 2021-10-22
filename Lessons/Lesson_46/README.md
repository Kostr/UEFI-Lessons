# String package

String package is a set of strings in one language. Each of the strings in package can be referred by ID. IDs to these strings are not written anywhere but it is supposed that numbering scheme simply starts from ID=1 in every string package and increases by 1 on every string [there are some exceptions, but it is not important now].

The general idea is that when you want to provide translations for your piece of interface you create String packages for each language you want to support. In every of these language string packages the same words should have the same ID. For example:
```
      String package (ENG)        String package (FR)
      ___________________         ___________________

ID=1   ...                            ...

       ...                            ...

ID=5   Hello!                        Bonjour!

       ...                            ...
```

In the code strings from the Package list are received from the `(ID, language)` combination. This helps to create UIs that could switch translations easily.

# String package content

String package strarts with a header `EFI_HII_STRING_PACKAGE_HDR` and then contains so called `String blocks`, which can be of different types. And as with the package list, the last element (=String block) has special `END` type.

![String_package](String_package.png?raw=true "String package")

The most important and most common type of String block is UCS2 String block. It contains header which points String block type and a CHAR16 string as a block data. Basically String package is a set of UCS2 String blocks each of which contains one string.

![UCS2_String_block](UCS2_String_block.png?raw=true "UCS2 String block")


# Create String package

Let's look at the header of the String package:

```
Prototype:
typedef struct _EFI_HII_STRING_PACKAGE_HDR {
 EFI_HII_PACKAGE_HEADER Header;
 UINT32 HdrSize;
 UINT32 StringInfoOffset;
 CHAR16 LanguageWindow[16];
 EFI_STRING_ID LanguageName;
 CHAR8 Language[ … ];
} EFI_HII_STRING_PACKAGE_HDR;

Members:
Header			The standard package header, where Header.Type = EFI_HII_PACKAGE_STRINGS.
HdrSize			Size of this header.
StringInfoOffset	Offset, relative to the start of this header, of the string information.
LanguageWindow		Specifies the default values placed in the static and dynamic windows
			before processing each SCSU-encoded string.
LanguageName		String identifier within the current string package of the full name of the
			language specified by Language.
Language		The null-terminated ASCII string that specifies the language of the strings in the package.
```

`CHAR8 Language` is a string like `en-EN`, `en-US`, `en`, `ru-RU`, ...
As the `Language` size not fixed, the `EFI_HII_STRING_PACKAGE_HDR` has a `HdrSize` field to indicate overall package header size.
Besides this short form, every string package contains a string with a full language name (e.g. `English`), and `LanguageName` field is an ID of this string within a package. Usually it is a first string with an ID=1.

As we've already know the String package would be filled with String blocks. They can be of many types, but for now we'll investigate the most important ones - the `UCS2 String block` and the `End block`.

MdePkg/Include/Uefi/UefiInternalFormRepresentation.h
```
typedef struct {
  UINT8                   BlockType;
} EFI_HII_STRING_BLOCK;

//
// Value of different string information block types
//
#define EFI_HII_SIBT_END                     0x00
...
#define EFI_HII_SIBT_STRING_UCS2             0x14
...

typedef struct _EFI_HII_SIBT_STRING_UCS2_BLOCK {
  EFI_HII_STRING_BLOCK    Header;
  CHAR16                  StringText[1];                // <--- String size is not fixed, but to point a fact that this type of block has
} EFI_HII_SIBT_STRING_UCS2_BLOCK;                       //      a string in itself, the header contains one element array

...

typedef struct _EFI_HII_SIBT_END_BLOCK {
  EFI_HII_STRING_BLOCK    Header;
} EFI_HII_SIBT_END_BLOCK;
```

Now it is time to write a function that can fill package data array from strings:
```
UINT32 InitStringPackage(CHAR8* Ptr, CHAR8* Language, CHAR16** Strings, UINTN StringsLen):

// CHAR8* Ptr	 - start of the array where we want our package data
// CHAR8* Language  - short language form for the `EFI_HII_STRING_PACKAGE_HDR.Language` field
// CHAR16** Strings - array of CHAR16* Strings that would be written to the package
// UINTN StringsLen - how many string there are in the Strings array
//
// Return UINT32    - function return the result package size
```

Here is a function implementation:
```
UINT32 InitStringPackage(CHAR8* Ptr, CHAR8* Language, CHAR16** Strings, UINTN StringsLen)
{
  UINT32 Size = 0;
  EFI_HII_STRING_PACKAGE_HDR* HIIStringPackageHdr = (EFI_HII_STRING_PACKAGE_HDR*)&Ptr[0];
  HIIStringPackageHdr->Header.Type = EFI_HII_PACKAGE_STRINGS;
  UINT32 HeaderSize = (UINT32) (AsciiStrSize(Language) - 1 + sizeof (EFI_HII_STRING_PACKAGE_HDR));
  HIIStringPackageHdr->HdrSize = HeaderSize;
  HIIStringPackageHdr->StringInfoOffset = HeaderSize;
  HIIStringPackageHdr->LanguageName = 1;                // <--- the String[0] should be a full language name string!
  AsciiStrCpyS (HIIStringPackageHdr->Language,
                (HeaderSize - OFFSET_OF(EFI_HII_STRING_PACKAGE_HDR,Language)) / sizeof (CHAR8),
                Language);
  Size += HeaderSize;

  for (UINTN i=0; i<StringsLen; i++)
  {
    EFI_HII_SIBT_STRING_UCS2_BLOCK* StrBlock = (EFI_HII_SIBT_STRING_UCS2_BLOCK*)&Ptr[Size];
    StrBlock->Header.BlockType = EFI_HII_SIBT_STRING_UCS2;
    StrCpyS(StrBlock->StringText, StrLen(Strings[i])+1, Strings[i]);
    Size += sizeof(EFI_HII_SIBT_STRING_UCS2_BLOCK) + StrLen(Strings[i])*2;  
  }

  EFI_HII_SIBT_END_BLOCK* EndBlock = (EFI_HII_SIBT_END_BLOCK*)&Ptr[Size];
  EndBlock->Header.BlockType = EFI_HII_SIBT_END;
  Size += sizeof(EFI_HII_SIBT_END_BLOCK);

  HIIStringPackageHdr->Header.Length = Size;

  return Size;
}
```

This function can be called like this:
```
  CHAR16* EnStrings[] = {
    L"English",
    L"Hello",
  };
  CHAR8* Data = (CHAR8*) AllocateZeroPool(200);
  UINT32 PackageSize = InitStringPackage(Data, "en-US", EnStrings, sizeof(EnStrings)/sizeof(EnStrings[0]));
```
Here are we use size `200` for our Data array as a size much bigger that we would need for the package. Ideally we need to perform calculations for the array size, but this lesson is hard enough as it is, so we keep it like that for now.

Now when we have our `InitStringPackage` function that creates string packages we can fill package data in our code.

Here is a complete Package list data generation code:
```
  CHAR8* Data = (CHAR8*) AllocateZeroPool(200);          // CHEAT! NEEDS CORRECTION FOR YOUR OWN PACKAGES!
  UINT32 offset = 0;
  EFI_HII_PACKAGE_LIST_HEADER* PackageListHdr = (EFI_HII_PACKAGE_LIST_HEADER*)&Data[offset];
  PackageListHdr->PackageListGuid = gHIIStringsCGuid;
  offset += sizeof(EFI_HII_PACKAGE_LIST_HEADER);

  CHAR16* EnStrings[] = {
    L"English",
    L"Hello",
  };
  offset += InitStringPackage(&Data[offset], "en-US", EnStrings, sizeof(EnStrings)/sizeof(EnStrings[0]));

  CHAR16* FrStrings[] = {
    L"French",
    L"Bonjour",
  };
  offset += InitStringPackage(&Data[offset], "fr-FR", FrStrings, sizeof(FrStrings)/sizeof(FrStrings[0]));

  EFI_HII_PACKAGE_HEADER* HIIEndPackageHdr = (EFI_HII_PACKAGE_HEADER*)&Data[offset];
  HIIEndPackageHdr->Type = EFI_HII_PACKAGE_END;
  HIIEndPackageHdr->Length = sizeof(EFI_HII_PACKAGE_HEADER);
  offset += sizeof(EFI_HII_PACKAGE_HEADER);

  PackageListHdr->PackageLength = offset;

  <...>			// Add new package to the HII Database

  FreePool(Data);
```