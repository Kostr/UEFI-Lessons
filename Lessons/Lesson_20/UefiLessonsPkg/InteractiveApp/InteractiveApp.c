#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN Index;
  EFI_INPUT_KEY Key;

  Print(L"Try to guess the secret symbol!\n");
  Print(L"To quit press 'q'\n");

  while(TRUE) {
    gBS->WaitForEvent(1, &(gST->ConIn->WaitForKey), &Index);
    gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    Print(L"ScanCode = %04x, UnicodeChar = %04x (%c)\n", Key.ScanCode, Key.UnicodeChar, Key.UnicodeChar);

    if (Key.UnicodeChar == 'k') {
      Print(L"Correct!\n");
      break;
    } else if (Key.UnicodeChar == 'q') {
      Print(L"Bye!\n");
      break;
    } else {
      Print(L"Wrong!\n");
    }
  }
  gST->ConIn->Reset(gST->ConIn, FALSE);
  return EFI_SUCCESS;
}
