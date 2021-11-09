/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>


extern EFI_WIDE_GLYPH gSimpleFontWideGlyphData[];
extern UINT32 gSimpleFontWideBytes;
extern EFI_NARROW_GLYPH gSimpleFontNarrowGlyphData[];
extern UINT32 gSimpleFontNarrowBytes;


UINT8* CreateSimpleFontPkg(EFI_WIDE_GLYPH* WideGlyph,
                           UINT32 WideGlyphSizeInBytes,
                           EFI_NARROW_GLYPH* NarrowGlyph,
                           UINT32 NarrowGlyphSizeInBytes)
{
  UINT32 PackageLen = sizeof(EFI_HII_SIMPLE_FONT_PACKAGE_HDR) + WideGlyphSizeInBytes + NarrowGlyphSizeInBytes + 4; 
  UINT8* FontPackage = (UINT8*)AllocateZeroPool (PackageLen); 
  *(UINT32*)FontPackage = PackageLen;

  EFI_HII_SIMPLE_FONT_PACKAGE_HDR *SimpleFont; 
  SimpleFont = (EFI_HII_SIMPLE_FONT_PACKAGE_HDR*)(FontPackage + 4); 
  SimpleFont->Header.Length = (UINT32)(PackageLen - 4); 
  SimpleFont->Header.Type = EFI_HII_PACKAGE_SIMPLE_FONTS; 
  SimpleFont->NumberOfNarrowGlyphs = (UINT16)(NarrowGlyphSizeInBytes / sizeof(EFI_NARROW_GLYPH));
  SimpleFont->NumberOfWideGlyphs = (UINT16)(WideGlyphSizeInBytes / sizeof(EFI_WIDE_GLYPH)); 
  
  UINT8* Location = (UINT8*)(&SimpleFont->NumberOfWideGlyphs + 1); 
  CopyMem(Location, NarrowGlyph, NarrowGlyphSizeInBytes);
  CopyMem(Location + NarrowGlyphSizeInBytes, WideGlyph, WideGlyphSizeInBytes);

  return FontPackage;
}


EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print(L"Привет!\n");

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

  Print(L"Привет!\n");

  return EFI_SUCCESS;
}

