/*
 * Copyright (c) 2024, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/SimpleClass.h>

EFI_EVENT Event;
STATIC VOID  *mRegistrationTracker;
UINTN NotifyData = 0;

VOID EFIAPI NotifyFunc(EFI_EVENT Event, VOID* Context)
{
  if (Context == NULL)
    return;

  Print(L"\nEvent is signaled! Context = %d\n", *(UINTN*)Context);
  *(UINTN*)Context += 1;

  SIMPLE_CLASS_PROTOCOL* SimpleClass;
  EFI_STATUS Status;
  Status = gBS->LocateProtocol(&gSimpleClassProtocolGuid,
                               // NULL,
                               mRegistrationTracker,
                               (VOID**)&SimpleClass);
  if (EFI_ERROR(Status)) {
    Print(L"Error! LocateProtocol returned: %r\n", Status);
    return;
  }

  UINTN Number;
  Status = SimpleClass->GetNumber(&Number);
  if (!EFI_ERROR(Status)) {
    Print(L"Current number = %d\n", Number);
  } else {
    Print(L"Error! Can't get number: %r\n", Status);
    return;
  }

  Status = SimpleClass->SetNumber(Number+5);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't set number: %r\n", Status);
    return;
  }
}


EFI_STATUS
EFIAPI
ProtocolEventDriverUnload (
  EFI_HANDLE ImageHandle
  )
{
  gBS->CloseEvent(Event);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ProtocolEventDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{

  EFI_STATUS Status;

  Status = gBS->CreateEvent(EVT_NOTIFY_SIGNAL,
                            TPL_NOTIFY,
                            &NotifyFunc,
                            &NotifyData,
                            &Event);
  if (EFI_ERROR(Status)) {
    Print(L"Error! CreateEvent returned: %r\n", Status);
    return Status;
  }

  Status = gBS->RegisterProtocolNotify(&gSimpleClassProtocolGuid,
                                       Event,
                                       &mRegistrationTracker);
  if (EFI_ERROR(Status)) {
    Print(L"Error! RegisterProtocolNotify returned: %r\n", Status);
    return Status;
  }

/*
  Event = EfiCreateProtocolNotifyEvent(&gSimpleClassProtocolGuid,
                                       TPL_NOTIFY,
                                       &NotifyFunc,
                                       &NotifyData,
                                       &mRegistrationTracker);
*/
  return EFI_SUCCESS;
}
