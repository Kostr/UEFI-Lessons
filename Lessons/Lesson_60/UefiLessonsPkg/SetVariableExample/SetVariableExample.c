/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>


VOID Usage()
{
  Print(L"Delete variable\n");
  Print(L"   SetVariableExample <variable name>\n");
  Print(L"\n");
  Print(L"Set variable\n");
  Print(L"   SetVariableExample <variable name> <attributes> <value>\n");
  Print(L"\n");
  Print(L"<attributes> can be <n|b|r>\n");
  Print(L"n - NON_VOLATILE\n");
  Print(L"b - BOOTSERVICE_ACCESS\n");
  Print(L"r - RUNTIME_ACCESS\n");
}

INTN EFIAPI ShellAppMain(IN UINTN Argc, IN CHAR16 **Argv)
{
  EFI_STATUS Status;

  if (Argc==2) {
    CHAR16* VariableName = Argv[1];
    Status = gRT->SetVariable (
                VariableName,
                &gEfiCallerIdGuid,
                0,
                0,
                NULL
                );
    if (EFI_ERROR (Status)) {
      Print(L"%r\n", Status);
    } else {
      Print(L"Variable %s was successfully deleted\n", VariableName);
    }
    return Status;
  } else if (Argc==4) {
    CHAR16* VariableName = Argv[1];
    CHAR16* AttributesStr = Argv[2];
    CHAR16* Value = Argv[3];
    UINT32  Attributes = 0;
    for (UINTN i=0; i<StrLen(AttributesStr); i++) {
      switch(AttributesStr[i]) {
        case L'n':
          Attributes |= EFI_VARIABLE_NON_VOLATILE;
          break;
        case L'b':
          Attributes |= EFI_VARIABLE_BOOTSERVICE_ACCESS;
          break;
        case L'r':
          Attributes |= EFI_VARIABLE_RUNTIME_ACCESS;
          break;
        default:
          Print(L"Error! Unknown attribute!");
          return EFI_INVALID_PARAMETER;
      }
    }
    Status = gRT->SetVariable (
                VariableName,
                &gEfiCallerIdGuid,
                Attributes,
                StrSize(Value),
                Value
                );
    if (EFI_ERROR (Status)) {
      Print(L"%r\n", Status);
    } else {
      Print(L"Variable %s was successfully changed\n", VariableName);
    }
    return Status;
  } else {
    Usage();
  }

  return EFI_SUCCESS;
}
