#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/PcdLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print(L"PcdMyVar32=%d\n", FixedPcdGet32(PcdMyVar32));
  return EFI_SUCCESS;
}
