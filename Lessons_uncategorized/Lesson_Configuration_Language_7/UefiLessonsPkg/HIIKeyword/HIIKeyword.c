/*
 * Copyright (c) 2022, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/HiiConfigKeyword.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>

VOID PrintBuffer(UINT8* Buffer, UINTN Size)
{
  UINTN i = 0;
  while (i < Size) {
    Print(L"%02x ", Buffer[i]);
    i++;
    if (!(i%16)) {
      Print(L" | ");
      for (UINTN j=16; j>0; j--)
        if ((Buffer[i-j] >= 0x20) && (Buffer[i-j] < 0x7E))
          Print(L"%c", Buffer[i-j]);
        else
          Print(L".");
      Print(L"\n");
    }
  }

  if (i%16) {
    for (UINTN j=0; j<=15; j++) {
      if ((i+j)%16)
        Print(L"   ");
      else
        break;
    }
    Print(L" | ");

    for (UINTN j=(i%16); j>0; j--) {
      if ((Buffer[i-j] >= 0x20) && (Buffer[i-j] < 0x7E))
        Print(L"%c", Buffer[i-j]);
      else
        Print(L".");
    }
    Print(L"\n");
  }
}

VOID ByteCfgStringToBuffer(CHAR16* CfgString, UINTN CfgStringLen, UINT8** Buffer, UINTN* BufferSize)
{
  *BufferSize = (CfgStringLen + 1) / 2;
  *Buffer = (UINT8*)AllocateZeroPool(*BufferSize);
  UINT8  DigitUint8;
  CHAR16 TempStr[2] = {0};
  for (UINTN Index = 0; Index < CfgStringLen; Index++) {
    TempStr[0] = CfgString[Index];
    DigitUint8 = (UINT8)StrHexToUint64(TempStr);
    if ((Index & 1) == 0) {
      (*Buffer)[Index/2] = DigitUint8;
    } else {
      (*Buffer)[Index/2] = (UINT8)(((*Buffer)[Index/2] << 4) + DigitUint8);
    }
  }
}

VOID ByteCfgStringToBufferReversed(CHAR16* CfgString, UINTN CfgStringLen, UINT8** Buffer, UINTN* BufferSize)
{
  *BufferSize = (CfgStringLen + 1) / 2;
  *Buffer = (UINT8*)AllocateZeroPool(*BufferSize);
  UINT8  DigitUint8;
  CHAR16 TempStr[2] = {0};
  for (INTN Index = (CfgStringLen-1); Index >= 0; Index--) {
    TempStr[0] = CfgString[Index];
    DigitUint8 = (UINT8)StrHexToUint64(TempStr);
    if (((CfgStringLen-1-Index) & 1) == 0) {
      (*Buffer)[(CfgStringLen-1-Index)/2] = DigitUint8;
    } else {
      (*Buffer)[(CfgStringLen-1-Index)/2] = (UINT8)((DigitUint8 << 4) + (*Buffer)[(CfgStringLen-1-Index)/2]);
    }
  }
}

EFI_STATUS DevicePathFromCfgString(CHAR16* CfgString, UINTN Size, EFI_DEVICE_PATH_PROTOCOL** DevicePath)
{
  UINTN DevicePathSize;
  ByteCfgStringToBuffer(CfgString, Size, (UINT8**)DevicePath, &DevicePathSize);

  EFI_DEVICE_PATH_PROTOCOL* DevicePathTest = *DevicePath;
  while (!IsDevicePathEnd(DevicePathTest)) {
    if ((DevicePathTest->Type == 0) || (DevicePathTest->SubType == 0) || (DevicePathNodeLength(DevicePathTest) < sizeof(EFI_DEVICE_PATH_PROTOCOL)))
      return EFI_NOT_FOUND;
    DevicePathTest = NextDevicePathNode (DevicePathTest);
  }

  return EFI_SUCCESS;
}

VOID PrintLongString(CHAR16* Str)
{
  UINT32 MaxPrintBufferSize = PcdGet32(PcdUefiLibMaxPrintBufferSize);
  if (StrLen(Str) > MaxPrintBufferSize) {
    EFI_STRING TempStr = (EFI_STRING)AllocateZeroPool(MaxPrintBufferSize * sizeof (CHAR16));
    CopyMem(TempStr, Str, MaxPrintBufferSize * sizeof (CHAR16));
    TempStr[MaxPrintBufferSize-1]=0;
    TempStr[MaxPrintBufferSize-2]=L'>';
    TempStr[MaxPrintBufferSize-3]=L'.';
    TempStr[MaxPrintBufferSize-4]=L'.';
    TempStr[MaxPrintBufferSize-5]=L'.';
    TempStr[MaxPrintBufferSize-6]=L'<';
    Print(L"%s", TempStr);
    FreePool(TempStr);
  } else {
    Print(L"%s", Str);
  }
}

VOID PrintConfigSubString(
  IN EFI_STRING ConfigString
  )
{
  EFI_STATUS Status;
  if (StrStr(ConfigString, L"NAMESPACE=")) {
    Print(L"\n");
    Print(L"%s\n", ConfigString);
  } else if (StrStr(ConfigString, L"PATH=")) {
    EFI_DEVICE_PATH_PROTOCOL* DevicePath;
    Status = DevicePathFromCfgString(&ConfigString[StrLen(L"PATH=")], StrLen(ConfigString) - StrLen(L"PATH="), &DevicePath);
    if (!EFI_ERROR(Status))
      Print(L"%s (%s)\n", ConfigString, ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*) DevicePath, FALSE, FALSE));
    else
      Print(L"%s\n", ConfigString);
    FreePool(DevicePath);
  } else if (StrStr(ConfigString, L"VALUE=")) {
    PrintLongString(ConfigString);
    Print(L"\n");
    UINT8* Buffer;
    UINTN BufferSize;
    ByteCfgStringToBufferReversed(&ConfigString[StrLen(L"VALUE=")], StrLen(&ConfigString[StrLen(L"VALUE=")]), &Buffer, &BufferSize);
    PrintBuffer(Buffer, BufferSize);
    FreePool(Buffer);
  } else {
    Print(L"%s\n", ConfigString);
  }
}

VOID PrintConfigString(
  IN EFI_STRING ConfigString
  )
{
  UINTN StartIndex=0;
  for (UINTN i=0; ConfigString[i] != 0; i++) {
    if (ConfigString[i] == L'&') {
      ConfigString[i] = 0;
      PrintConfigSubString(&ConfigString[StartIndex]);
      StartIndex = i+1;
    }
  }
  PrintConfigSubString(&ConfigString[StartIndex]);
}

EFI_STRING ProgressErrorStr(UINT32 ProgressErr)
{
  switch (ProgressErr) {
    case KEYWORD_HANDLER_NAMESPACE_ID_NOT_FOUND:
      return L"NamespaceId not found\n";
    case KEYWORD_HANDLER_MALFORMED_STRING:
      return L"Malformed string\n";
    case KEYWORD_HANDLER_KEYWORD_NOT_FOUND:
      return L"Keyword not found\n";
    case KEYWORD_HANDLER_INCOMPATIBLE_VALUE_DETECTED:
      return L"Incompatible value detected\n";
    case KEYWORD_HANDLER_ACCESS_NOT_PERMITTED:
      return L"Access not permitted\n";
    default:
      return L"Unknown error\n";
  }
}

VOID Usage()
{
  Print(L"Usage:\n");
  Print(L"HIIKeyword get <NamespaceStr> <KeywordStr>\n");
  Print(L"HIIKeyword set <KeywordStr>\n");
}

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_CONFIG_KEYWORD_HANDLER_PROTOCOL  *gHiiConfigKeywordHandler = NULL;
  EFI_STATUS Status = gBS->LocateProtocol(&gEfiConfigKeywordHandlerProtocolGuid,
                                          NULL,
                                          (VOID **)&gHiiConfigKeywordHandler);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't locate gEfiConfigKeywordHandlerProtocolGuid: %r", Status);
    return Status;
  }

  if (Argc==1) {
    Usage();
    return EFI_SUCCESS;
  }

  EFI_STRING Progress;
  UINT32 ProgressErr;
  if (!StrCmp(Argv[1], L"get")) {
    if (Argc != 4) {
      Print(L"Wrong argument!\n");
      Usage();
      return EFI_INVALID_PARAMETER;
    }
    EFI_STRING NameSpaceId = Argv[2];
    EFI_STRING KeywordString = Argv[3];
    if (!StrCmp(NameSpaceId, L'\0')) {
      NameSpaceId = NULL;
    }
    if (!StrCmp(KeywordString, L'\0')) {
      KeywordString = NULL;
    }

    EFI_STRING Results;
    Status = gHiiConfigKeywordHandler->GetData(gHiiConfigKeywordHandler,
                                               NameSpaceId,
                                               KeywordString,
                                               &Progress,
                                               &ProgressErr,
                                               &Results);
    if (StrCmp(Progress, L'\0')) {
      Print(L"Part of string was unparsed %s\n", Progress);
    }
    if (ProgressErr) {
      Print(L"Error! ProgressErr=%s\n", ProgressErrorStr(ProgressErr));
    }
    if (EFI_ERROR(Status)) {
      Print(L"Error! GetData returned %r\n", Status);
      return Status;
    }
    Print(L"Response: ");
    PrintLongString(Results);
    Print(L"\n\n");
    PrintConfigString(Results);
    FreePool(Results);
  } else if (!StrCmp(Argv[1], L"set")) {
    if (Argc != 3) {
      Print(L"Wrong argument!\n");
      Usage();
      return EFI_INVALID_PARAMETER;
    }
    Status = gHiiConfigKeywordHandler->SetData(gHiiConfigKeywordHandler,
                                               Argv[2],
                                               &Progress,
                                               &ProgressErr);
    if (StrCmp(Progress, L'\0')) {
      Print(L"Part of string was unparsed %s\n", Progress);
    }
    if (ProgressErr) {
      Print(L"Error! ProgressErr=%s\n", ProgressErrorStr(ProgressErr));
    }
    if (EFI_ERROR(Status)) {
      Print(L"Error! SetData returned %r\n", Status);
      return Status;
    }
  } else {
    Print(L"Wrong argument!\n");
    Usage();
    return EFI_INVALID_PARAMETER;
  }
  return EFI_SUCCESS;
}
