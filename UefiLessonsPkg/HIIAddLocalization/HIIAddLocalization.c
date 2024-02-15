/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/HiiLib.h>
#include <Library/UefiHiiServicesLib.h>
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

/* *** If you add this: ***
  for (UINTN i=0; i<0xFFFF; i++) {
    EFI_STRING String = HiiGetString(*Handle, i, "en-US");
    if (String != NULL) {
      Print(L"ID=%d, %s\n", i, String);
      FreePool(String);
    }
  }
*/
/* *** You would get: ***
FS0:\> HIIAddLocalization.efi
en;fr;en-US;fr-FR
LangLen = 17
en;fr;en-US;fr-FR;ru-RU
Error! Can't set PlatformLangCodes variable, status=Write Protected
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
*/

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

  return EFI_SUCCESS;
}
