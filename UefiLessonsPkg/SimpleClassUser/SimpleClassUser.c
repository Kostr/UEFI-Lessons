#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/SimpleClass.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN                  HandleCount;
  EFI_HANDLE*            HandleBuffer;
  UINTN Index;
  SIMPLE_CLASS_PROTOCOL* SimpleClass;

  EFI_STATUS Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gSimpleClassProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    Print(L"Error! Can't find any handle with gSimpleClassProtocolGuid: %r\n", Status);
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Print(L"Handle = %p\n", HandleBuffer[Index]);
    Status = gBS->OpenProtocol(
      HandleBuffer[Index],
      &gSimpleClassProtocolGuid,
      (VOID **)&SimpleClass,
      ImageHandle,
      NULL,
      EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );

    if (!EFI_ERROR(Status)) {
      UINTN Number;

      Status = SimpleClass->GetNumber(&Number);
      if (!EFI_ERROR(Status)) {
        Print(L"Number before=%d\n", Number);
      } else {
        Print(L"Error! Can't get number: %r\n", Status);
      }

      Status = SimpleClass->SetNumber(Number+5);
      if (EFI_ERROR(Status))
        Print(L"Error! Can't set number: %r\n", Status);

      Status = SimpleClass->GetNumber(&Number);
      if (!EFI_ERROR(Status)) {
        Print(L"Number after=%d\n", Number);
      } else {
        Print(L"Error! Can't get number: %r\n", Status);
      }
    } else {
      Print(L"Error! Can't open SimpleClass protocol: %r\n", Status);
    }
  }

  return Status;
}
