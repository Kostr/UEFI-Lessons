/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/MemoryAllocationLib.h>
#include <Protocol/FormBrowser2.h>
#include <Library/HiiLib.h>

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  if (Argc <=1) {
    Print(L"Usage:\n");
    Print(L"  DisplayHIIByGuid <GUID> [<GUID> [<GUID> [...]]]\n");
    return EFI_INVALID_PARAMETER;
  }

  GUID* Guids = (GUID*)AllocatePool(sizeof(GUID)*(Argc-1));

  for (UINTN i=1; i<Argc; i++) {
    RETURN_STATUS Status = StrToGuid(Argv[i], &Guids[i-1]);
    if (Status != RETURN_SUCCESS) {
      Print(L"Error! Can't convert one of the GUIDs to string\n");
      FreePool(Guids);
      return EFI_INVALID_PARAMETER;
    }
    Print(L"%g\n", Guids[i-1]);
  }


  EFI_HII_HANDLE* Handle = HiiGetHiiHandles(&Guids[0]);

  UINTN HandleCount=0;
  while (*Handle != NULL) {
    Handle++;
    HandleCount++;
    Print(L"Total HandleCount=%d\n", HandleCount);
  }
  Print(L"Total HandleCount=%d\n", HandleCount);

/*
  return EFI_SUCCESS;

  EFI_STATUS Status;
  EFI_FORM_BROWSER2_PROTOCOL* FormBrowser2;
  Status = gBS->LocateProtocol(&gEfiFormBrowser2ProtocolGuid, NULL, (VOID**)&FormBrowser2);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = FormBrowser2->SendForm (
                           FormBrowser2,
                           Handle,
                           1,
                           NULL,
                           0,
                           NULL,
                           NULL
                           );  

*/
  FreePool(Handle);
  FreePool(Guids);

  return EFI_SUCCESS;
}
