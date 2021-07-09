#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>


EFI_STATUS
EFIAPI
SimpleDriverUnload (
  EFI_HANDLE ImageHandle
  )
{
  Print(L"Bye-bye from driver!\n");

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SimpleDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  Print(L"Hello from driver!\n");

  return EFI_SUCCESS;
}
