#include <Library/UefiLib.h>
#include <Library/SimpleLibrary.h>

UINTN Plus2(UINTN number) {
  return number+2;
}

EFI_STATUS
EFIAPI
SimpleLibraryConstructor(
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  Print(L"Hello from library constructor!\n");
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SimpleLibraryDestructor(
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  Print(L"Hello from library destructor!\n");
  return EFI_SUCCESS;
}
