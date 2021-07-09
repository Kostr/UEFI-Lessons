#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello World!\n");
  gST->ConOut->OutputString(gST->ConOut, L"Hello again!\n");
  Print(L"Bye!\n");

  UINTN Index;
  gBS->WaitForEvent(1, &(gST->ConIn->WaitForKey), &Index);
  gST->ConIn->Reset(gST->ConIn, FALSE);
  return EFI_SUCCESS;
}
