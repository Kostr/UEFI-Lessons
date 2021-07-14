#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/SimpleTextInEx.h>


EFI_HANDLE NotifyHandle;
EFI_HANDLE NotifyHandle1;

EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* InputEx = NULL;

EFI_STATUS EFIAPI MyKeyNotificationFunction(EFI_KEY_DATA* KeyData)
{
	Print(L"\nHot Key 1 is pressed");
	return EFI_SUCCESS;
}

EFI_STATUS EFIAPI MyKeyNotificationFunction1(EFI_KEY_DATA* KeyData)
{
	Print(L"\nHot Key 2 is pressed");
	return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
HotKeyDriverUnload(
  IN EFI_HANDLE        ImageHandle
  )
{
  if (!InputEx)
    return EFI_SUCCESS;

  EFI_STATUS Status;

  if (!NotifyHandle) {
    Status = InputEx->UnregisterKeyNotify(InputEx,
                                   (VOID*)NotifyHandle);

    if (EFI_ERROR(Status)) {
      Print(L"Error! Can't perform RegisterKeyNotify: %r", Status);
      return Status;
    }
  }

  if (!NotifyHandle1) {
    Status = InputEx->UnregisterKeyNotify(InputEx,
                       (VOID*)NotifyHandle1);

    if (EFI_ERROR(Status)) {
      Print(L"Error! Can't perform RegisterKeyNotify: %r", Status);
      return Status;
    }
  }

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
HotKeyDriverEntryPoint(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status = gBS->LocateProtocol(
			 &gEfiSimpleTextInputExProtocolGuid,
			 NULL,
			 (VOID**)&InputEx
			 );
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't locate EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL: %r", Status);
    return Status;
  }

  EFI_KEY_DATA HotKey;
  HotKey.Key.ScanCode = 0;
  HotKey.Key.UnicodeChar = L'z';
  HotKey.KeyState.KeyShiftState = EFI_LEFT_CONTROL_PRESSED | EFI_LEFT_ALT_PRESSED | EFI_SHIFT_STATE_VALID;
  HotKey.KeyState.KeyToggleState = 0;

  Status = InputEx->RegisterKeyNotify(InputEx,
			&HotKey,
			MyKeyNotificationFunction,
                        (VOID**)&NotifyHandle);

  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't perform RegisterKeyNotify: %r", Status);
    return Status;
  }

  HotKey.KeyState.KeyShiftState = 0;
  HotKey.KeyState.KeyToggleState = EFI_TOGGLE_STATE_VALID | EFI_NUM_LOCK_ACTIVE | EFI_KEY_STATE_EXPOSED;

  Status = InputEx->RegisterKeyNotify(InputEx,
			&HotKey,
			MyKeyNotificationFunction1,
                        (VOID**)&NotifyHandle1); 

  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't perform RegisterKeyNotify: %r", Status);
    return Status;
  }

  return EFI_SUCCESS;
}
