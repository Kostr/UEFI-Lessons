/*
 * Copyright (c) 2022, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HiiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Protocol/HiiConfigRouting.h>


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

EFI_STATUS GuidFromCfgString(CHAR16* CfgString, UINTN Size, EFI_GUID** Guid)
{
  UINTN GuidSize;
  ByteCfgStringToBuffer(CfgString, Size, (UINT8**)Guid, &GuidSize);
  if (GuidSize != sizeof(EFI_GUID))
    return EFI_NOT_FOUND;
  return EFI_SUCCESS;
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

EFI_STATUS NameFromCfgString(CHAR16* CfgString, UINTN Size, CHAR16** Name)
{
  *Name = AllocateZeroPool(Size * sizeof(CHAR16));
  CHAR16 TempStr[4];
  for (UINTN i=0; i<Size; i+=4) {
    StrnCpyS(TempStr, sizeof(TempStr), CfgString+i, 4);
    (*Name)[i/4] = (CHAR16)StrHexToUint64(TempStr);
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
  if (StrStr(ConfigString, L"GUID=")) {
    Print(L"\n");
    EFI_GUID* Guid;
    Status = GuidFromCfgString(&ConfigString[StrLen(L"GUID=")], StrLen(ConfigString) - StrLen(L"GUID="), &Guid);
    if (!EFI_ERROR(Status))
      Print(L"%s (%g)\n", ConfigString, Guid);
    else
      Print(L"%s\n", ConfigString);
    FreePool(Guid);
  } else if (StrStr(ConfigString, L"NAME=")) {
    CHAR16* Name;
    NameFromCfgString(&ConfigString[StrLen(L"NAME=")], StrLen(ConfigString) - StrLen(L"NAME="), &Name);
    Print(L"%s (%s)\n", ConfigString, Name);
    FreePool(Name);
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
  } else if (StrStr(ConfigString, L"OFFSET=") || StrStr(ConfigString, L"WIDTH=")) {
    Print(L"%s  ", ConfigString);
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

EFI_STATUS CreateCfgHeader(EFI_STRING GuidStr, EFI_STRING NameStr, EFI_STRING DevicePathStr, EFI_STRING* Request)
{
  EFI_STATUS Status;
  EFI_GUID FormsetGuid;
  Status = StrToGuid(GuidStr, &FormsetGuid);
  if (Status != RETURN_SUCCESS) {
    Print(L"Error! Can't convert <FormsetGuid> argument to GUID\n");
    return EFI_INVALID_PARAMETER;
  }

  EFI_HANDLE DriverHandle = NULL;
  EFI_DEVICE_PATH_PROTOCOL* DevicePath = ConvertTextToDevicePath(DevicePathStr);

  Status = gBS->LocateDevicePath(&gEfiDevicePathProtocolGuid,
                                 &DevicePath,
                                 &DriverHandle
                                );
  FreePool(DevicePath);
  if (EFI_ERROR(Status) || (DriverHandle == NULL)) {
    Print(L"Error! Can't get DriverHandle\n");
    return EFI_INVALID_PARAMETER;
  }

  *Request = HiiConstructConfigHdr(&FormsetGuid, NameStr, DriverHandle);
  return EFI_SUCCESS;
}

VOID Usage()
{
  Print(L"Usage:\n");
  Print(L"HIIConfig.efi dump\n");
  Print(L"HIIConfig.efi extract <ConfigStr>\n");
  Print(L"HIIConfig.efi extract <Guid> <Name> <Path>\n");
  Print(L"HIIConfig.efi extract <Guid> <Name> <Path> <Offset> <Width>\n");
  Print(L"HIIConfig.efi route <ConfigStr>\n");
  Print(L"HIIConfig.efi route <Guid> <Name> <Path> <Offset> <Width> <Value>\n");
}

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *gHiiConfigRouting = NULL;
  EFI_STATUS Status = gBS->LocateProtocol(&gEfiHiiConfigRoutingProtocolGuid,
                                          NULL,
                                          (VOID **)&gHiiConfigRouting);
  if (EFI_ERROR(Status)) {
    Print(L"Error! Can't locate gEfiHiiConfigRoutingProtocolGuid: %r", Status);
    return Status;
  }

  if (Argc == 1) {
    Usage();
    return EFI_SUCCESS;
  }


  EFI_STRING Request;
  EFI_STRING Progress;
  EFI_STRING Result;
  if (!StrCmp(Argv[1], L"dump")) {
    Status = gHiiConfigRouting->ExportConfig(gHiiConfigRouting, &Result);
    Print(L"Full configuration for the HII Database (Size = %d):\n", StrLen(Result));
    PrintConfigString(Result);
    FreePool(Result);
  } else if (!StrCmp(Argv[1], L"extract")) {
    if (Argc == 3) {
      Request = Argv[2];
    } else if ((Argc == 5) || (Argc == 7)) {
      Status = CreateCfgHeader(Argv[2], Argv[3], Argv[4], &Request);
      if (EFI_ERROR(Status)) {
        return Status;
      }
      if (Argc == 7) {
        EFI_STRING OffsetStr = Argv[5];
        EFI_STRING WidthStr = Argv[6];
        UINTN Size = (StrLen(Request) + StrLen(L"&OFFSET=") + StrLen(OffsetStr) + StrLen(L"&WIDTH=") + StrLen(WidthStr) + 1) * sizeof(CHAR16);
        EFI_STRING TempRequest = AllocateZeroPool(Size);
        UnicodeSPrint(TempRequest, Size, L"%s&OFFSET=%s&WIDTH=%s", Request, OffsetStr, WidthStr);
        FreePool(Request);
        Request = TempRequest;
      }
      Print(L"Request: %s\n", Request);
    } else {
      Print(L"Error! Wrong arguments\n");
      Usage();
      return EFI_INVALID_PARAMETER;
    }

    Status = gHiiConfigRouting->ExtractConfig(gHiiConfigRouting,
                                              Request,
                                              &Progress,
                                              &Result);

    if (StrCmp(Progress, L'\0')) {
      Print(L"Part of string was unparsed %s\n", Progress);
    }

    if (Argc >= 5)
      FreePool(Request);

    if (EFI_ERROR(Status)) {
      Print(L"Error! ExtractConfig returned %r\n", Status);
      return Status;
    }
    Print(L"Response: ");
    PrintLongString(Result);
/*  If you want to get buffer for current value, you can do it like this:

    if (Argc == 7) {
      Print(L"\n\nCurrent value:\n");
      CHAR16* StartPtr = StrStr(Result, L"VALUE=");
      StartPtr += StrLen(L"VALUE=");
      CHAR16* EndPtr = StartPtr;
      while ((*EndPtr != 0) && (*EndPtr != L'&'))
        EndPtr++;

      UINT8* Buffer;
      UINTN BufferSize;
      ByteCfgStringToBufferReversed(StartPtr, EndPtr-StartPtr, &Buffer, &BufferSize);
      PrintBuffer(Buffer, BufferSize);
      FreePool(Buffer);
    }
*/
    Print(L"\n\n");
    PrintConfigString(Result);
    FreePool(Result);
  } else if (!StrCmp(Argv[1], L"route")) {
    if (Argc == 3) {
      Request = Argv[2];
    } else if (Argc == 8) {
      Status = CreateCfgHeader(Argv[2], Argv[3], Argv[4], &Request);
      if (EFI_ERROR(Status)) {
        return Status;
      }
      EFI_STRING OffsetStr = Argv[5];
      EFI_STRING WidthStr = Argv[6];
      EFI_STRING ValueStr = Argv[7];
      UINTN Size = (StrLen(Request) + StrLen(L"&OFFSET=") + StrLen(OffsetStr) + StrLen(L"&WIDTH=") + StrLen(WidthStr) +
                    StrLen(L"&VALUE=") + StrLen(ValueStr) + 1) * sizeof(CHAR16);
      EFI_STRING TempRequest = AllocateZeroPool(Size);
      UnicodeSPrint(TempRequest, Size, L"%s&OFFSET=%s&WIDTH=%s&VALUE=%s", Request, OffsetStr, WidthStr, ValueStr);
      FreePool(Request);
      Request = TempRequest;
    } else {
      Print(L"Error! Wrong arguments\n");
      Usage();
      return EFI_INVALID_PARAMETER;
    }
    Print(L"Request: %s\n", Request);

    Status = gHiiConfigRouting->RouteConfig(gHiiConfigRouting,
                                            Request,
                                            &Progress);
    if (StrCmp(Progress, L'\0')) {
      Print(L"Part of string was unparsed: %s\n", Progress);
      if (StrCmp(Progress, Request)) {
        Print(L"IMPORTANT: part of the data was written!\n");
      }
    }
    if (Argc == 8) {
      FreePool(Request);
    }
    if (EFI_ERROR(Status)) {
      Print(L"Error! RouteConfig returned %r\n", Status);
      return Status;
    }
    FreePool(Result);
  } else {
    Print(L"Error! Wrong arguments\n");
    Usage();
    return EFI_INVALID_PARAMETER;
  }
  return EFI_SUCCESS;
}
