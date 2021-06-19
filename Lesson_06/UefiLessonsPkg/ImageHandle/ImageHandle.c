#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>


typedef struct {
  UINTN               Signature;
  /// All handles list of IHANDLE
  LIST_ENTRY          AllHandles;
  /// List of PROTOCOL_INTERFACE's for this handle
  LIST_ENTRY          Protocols;
  UINTN               LocateRequest;
  /// The Handle Database Key value when this handle was last created or modified
  UINT64              Key;
} IHANDLE;

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  IHANDLE* MyHandle = ImageHandle;
  Print(L"Signature: %c %c %c %c\n", (MyHandle->Signature >>  0) & 0xff,
                                     (MyHandle->Signature >>  8) & 0xff,
                                     (MyHandle->Signature >> 16) & 0xff,
                                     (MyHandle->Signature >> 24) & 0xff);

  return EFI_SUCCESS;
}
