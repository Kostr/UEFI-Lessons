#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Pi/PiMultiPhase.h>
#include <Protocol/PiPcd.h>
#include <Protocol/PiPcdInfo.h>

//#include <Library/PcdLib.h>

VOID Usage()
{
  Print(L"Program to set Sku number\n\n");
  Print(L"  Usage: SetSku <SkuNumber in hex>\n");
}

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS Status;

  if (Argc != 2) {
    Usage();
    return EFI_INVALID_PARAMETER;
  }

  UINTN SkuNumber;
  RETURN_STATUS ReturnStatus;
  ReturnStatus = StrHexToUintnS(Argv[1], NULL, &SkuNumber);
  if (RETURN_ERROR(ReturnStatus)) {
    Print(L"Error! Can't convert SkuId string to number\n");
    return EFI_INVALID_PARAMETER;
  }

  EFI_PCD_PROTOCOL* pcdProtocol;
  Status = gBS->LocateProtocol(&gEfiPcdProtocolGuid,
                               NULL,
                               (VOID **)&pcdProtocol);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't find EFI_PCD_PROTOCOL\n");
    return EFI_UNSUPPORTED;
  }

  EFI_GET_PCD_INFO_PROTOCOL* getPcdInfoProtocol;
  Status = gBS->LocateProtocol(&gEfiGetPcdInfoProtocolGuid,
                               NULL,
                               (VOID **)&getPcdInfoProtocol);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't find EFI_GET_PCD_INFO_PROTOCOL\n");
    return EFI_UNSUPPORTED;
  }


  pcdProtocol->SetSku(SkuNumber);

  UINTN SkuId = getPcdInfoProtocol->GetSku();
/*
  LibPcdSetSku(SkuNumber);
  UINTN SkuId = LibPcdGetSku();
*/
  if (SkuId != SkuNumber) {
    Print(L"Failed to change SkuId to 0x%lx, SkuId is still 0x%lx\n", SkuNumber, SkuId);
    return EFI_UNSUPPORTED;
  }

  Print(L"Sku is changed successfully to 0x%lx\n", SkuNumber);

  return EFI_SUCCESS;
}
