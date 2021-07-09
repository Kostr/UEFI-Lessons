#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/SimpleLibrary.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print(L"%d\n", Plus2(3));

  return EFI_SUCCESS;
}
