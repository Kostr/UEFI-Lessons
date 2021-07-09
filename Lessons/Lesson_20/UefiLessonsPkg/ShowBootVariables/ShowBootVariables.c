#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>


EFI_STATUS
GetNvramVariable( CHAR16   *VariableName,
                  EFI_GUID *VariableOwnerGuid, 
                  VOID     **Buffer,
                  UINTN    *BufferSize)
{
    UINTN Size = 0;
    *BufferSize = 0;

    EFI_STATUS Status = gRT->GetVariable(VariableName, VariableOwnerGuid, NULL, &Size, NULL);
    if (Status != EFI_BUFFER_TOO_SMALL) {
        Print(L"Error! 'gRT->GetVariable' call returned %r\n", Status);
        return Status;
    }

    *Buffer = AllocateZeroPool(Size);
    if (!Buffer) {
        Print(L"Error! 'AllocateZeroPool' call returned %r\n", Status);
        return EFI_OUT_OF_RESOURCES;
    }

    Status = gRT->GetVariable(VariableName, VariableOwnerGuid, NULL, &Size, *Buffer);
    if (Status == EFI_SUCCESS) {
        *BufferSize = Size;
    } else {
        FreePool( *Buffer );
        *Buffer = NULL;
    }

    return Status;
}


VOID PrintBootOption(CHAR16* BootOptionName)
{
  UINTN OptionSize;
  UINT8* Buffer;
 
  EFI_STATUS Status = GetNvramVariable(BootOptionName, &gEfiGlobalVariableGuid, (VOID**)&Buffer, &OptionSize);
  if (Status == EFI_SUCCESS) {
      EFI_LOAD_OPTION* LoadOption = (EFI_LOAD_OPTION*) Buffer;
      CHAR16* Description = (CHAR16*)(Buffer + sizeof (EFI_LOAD_OPTION));
      UINTN DescriptionSize = StrSize(Description);

      Print(L"%s\n", Description);
      if (LoadOption->FilePathListLength != 0) {
        VOID* FilePathList = (UINT8 *)Description + DescriptionSize;
        CHAR16* DevPathString = ConvertDevicePathToText(FilePathList, TRUE, FALSE);
        Print(L"%s\n", DevPathString);
      }
  } else {
    Print(L"Can't get %s variable\n", BootOptionName);
  }
}


INTN EFIAPI ShellAppMain(IN UINTN Argc, IN CHAR16 **Argv)
{
  UINTN OptionSize;
  EFI_STATUS Status;

  UINT16* BootCurrent;
  Status = GetNvramVariable(L"BootCurrent", &gEfiGlobalVariableGuid, (VOID**)&BootCurrent, &OptionSize);
  if (Status != EFI_SUCCESS) {
    Print(L"Can't get BootCurrent variable\n");
  }

  UINT16* BootOrderArray;
  Status = GetNvramVariable(L"BootOrder", &gEfiGlobalVariableGuid, (VOID**)&BootOrderArray, &OptionSize);
  if (Status == EFI_SUCCESS) {
    for (UINTN i=0; i<(OptionSize/sizeof(UINT16)); i++) {
      CHAR16 BootOptionStr[sizeof("Boot####")+1];
      UnicodeSPrint(BootOptionStr, (sizeof("Boot####")+1)*sizeof(CHAR16), L"Boot%04x", BootOrderArray[i]);
      Print(L"%s%s\n", BootOptionStr, (BootOrderArray[i] == *BootCurrent)? L"*" : L"" );
      PrintBootOption(BootOptionStr);
      Print(L"\n");
    }
  } else {
    Print(L"Can't get BootOrder variable\n");
  }

  return EFI_SUCCESS;
}
