It is almost impossible to find something in the raw output of the `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExportConfig()` function, so let's try prettify it. For this we will write a custom function `PrintConfigString`:
```cpp
- gST->ConOut->OutputString(gST->ConOut, Result);
+ PrintConfigString(Result);
```

In our `PrintConfigString` function we would split the configuration string by the keyword separator `&` and use another `PrintConfigSubString` function to print each `<key>=<data>` pair:
```cpp
VOID PrintConfigString(
  IN EFI_STRING ConfigString
  )
{
  UINTN StartIndex=0;
  for (UINTN i=0; ConfigString[i] != 0; i++) {
    if (ConfigString[i] == L'&') {
      ConfigString[i] = 0;                                         <--- we can modify the string as we wouldn't need it after this function
      PrintConfigSubString(&ConfigString[StartIndex]);
      StartIndex = i+1;
    }
  }
  PrintConfigSubString(&ConfigString[StartIndex]);
}
```

Here is how our `PrintConfigSubString` function would look like:
```cpp
VOID PrintConfigSubString(
  IN EFI_STRING ConfigString
  )
{
  EFI_STATUS Status;
  if (StrStr(ConfigString, L"GUID=")) {
    <...>						// display the full string and actual guid
  } else if (StrStr(ConfigString, L"NAME=")) {
    <...>                                               // display the full string and actual name
  } else if (StrStr(ConfigString, L"PATH=")) {
    <...>
  } else if (StrStr(ConfigString, L"VALUE=")) {
    <...>                                               // display the full string and actual value
  } else if (StrStr(ConfigString, L"OFFSET=") || StrStr(ConfigString, L"WIDTH=")) {
    Print(L"%s  ", ConfigString);                       // don't print '\n', so we could see the "OFFSET=<...>  WIDTH=<...>  VALUE=<...>" on the same string
  } else {
    Print(L"%s\n", ConfigString);
  }
}
```

Here you can see, that for each key we would need a special code to print it.
If the string starts with a `GUID=`/`NAME=`/`PATH=` we will parse the data and display actual GUID/Name/DevicePath in a readable standard format. If the string starts with `VALUE=` we would display its data as a hex buffer similar to hexdump. If the string starts with `OFFSET=` or `WIDTH=` we would display data as-is and without any carriage return, so the `OFFSET`/`WIDTH`/`VALUE` data would be placed at the same string.

## `GUID=`

Let's start with the `GUID=` string handling. First of all we print a new line `Print(L"\n")` as `GUID=` signifies a start of a new configuration header. Next we pass the string key data to our special `GuidFromCfgString` function that creates `EFI_GUID` value on the heap from the string content:
```cpp
Print(L"\n");
EFI_GUID* Guid;
Status = GuidFromCfgString(&ConfigString[StrLen(L"GUID=")], StrLen(ConfigString) - StrLen(L"GUID="), &Guid);
if (!EFI_ERROR(Status))
  Print(L"%s (%g)\n", ConfigString, Guid);
else
  Print(L"%s\n", ConfigString);
FreePool(Guid);
```

The `GuidFromCfgString` function in turn uses `ByteCfgStringToBuffer` helper to get `EFI_GUID` data as an array:
```cpp
EFI_STATUS GuidFromCfgString(CHAR16* CfgString, UINTN Size, EFI_GUID** Guid)
{
  UINTN GuidSize;
  ByteCfgStringToBuffer(CfgString, Size, (UINT8**)Guid, &GuidSize);
  if (GuidSize != sizeof(EFI_GUID))
    return EFI_NOT_FOUND;
  return EFI_SUCCESS;
}
```

And here is the `ByteCfgStringToBuffer` function. It receives config string and its length as arguments and returns bytes buffer with its size:
```cpp
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
```

What does this function do? Let's some part from the output as an example:
```
GUID=1cc53572800cab4c87ac3b084a6304b1
```
In this case `ByteCfgStringToBuffer` function will only receive the string data portion:
```
1cc53572800cab4c87ac3b084a6304b1
```
and construct byte array like this:
```cpp
{0x1c, 0xc5, 0x35, 0x72, 0x80, 0x0c, 0xab, 0x4c, 0x87, 0xac, 0x3b, 0x08, 0x4a, 0x63, 0x04, 0xb1}
```
Which is the same as:
```cpp
{{0x7235c51c}, {0x0c80}, {0x4cab}, {0x87, 0xac, 0x3b, 0x08, 0x4a, 0x63, 0x04, 0xb1}}
```
And this is the GUID `gOvmfPlatformConfigGuid` from the [https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkg.dec](https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkg.dec):
```
[Guids]
  ...
  gOvmfPlatformConfigGuid               = {0x7235c51c, 0x0c80, 0x4cab, {0x87, 0xac, 0x3b, 0x08, 0x4a, 0x63, 0x04, 0xb1}}
```
So you just need to cast the returned buffer to `EFI_GUID` and you can print it as usual with `%g`. As the `EFI_GUID` was allocated on heap we use the `FreePool` function after the print statement.

## `NAME=`

To print `NAME=` data we use the `NameFromCfgString` helper function:
```cpp
CHAR16* Name;
NameFromCfgString(&ConfigString[StrLen(L"NAME=")], StrLen(ConfigString) - StrLen(L"NAME="), &Name);
Print(L"%s (%s)\n", ConfigString, Name);
FreePool(Name);
```

This function looks like this:
```cpp
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
```

Once again let's see the things on the example from the output:
```
NAME=004d00610069006e0046006f0072006d00530074006100740065
```
`NameFromCfgString` will receive in this case just the string data part:
```
004d00610069006e0046006f0072006d00530074006100740065
```
And convert it to the `CHAR16` array:
```cpp
{ 0x004d, 0x0061, 0x0069, 0x006e, 0x0046, 0x006f, 0x0072, 0x006d, 0x0053, 0x0074, 0x0061, 0x0074, 0x0065 }
```
Which essentially is:
```cpp
{ L'M', L'a', L'i', L'n', L'F', L'o', L'r', L'm', L'S', L't', L'a', L't', L'e' }
```
That corresponds to the string:
```cpp
MainFormState
```

## `PATH=`

The `PATH=` data contains device path encoded in bytes. String to device path conversion is done with the help of the `DevicePathFromCfgString` helper function:
```cpp
EFI_DEVICE_PATH_PROTOCOL* DevicePath;
Status = DevicePathFromCfgString(&ConfigString[StrLen(L"PATH=")], StrLen(ConfigString) - StrLen(L"PATH="), &DevicePath);
if (!EFI_ERROR(Status))
  Print(L"%s (%s)\n", ConfigString, ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*) DevicePath, FALSE, FALSE));
else
  Print(L"%s\n", ConfigString);
FreePool(DevicePath);
```

Here is how it looks like. It uses the same `ByteCfgStringToBuffer` function that we've used in `GUID=` case to convert string data to a buffer. In the end we verify if the final buffer is really a correct device path:
```cpp
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
```

As here we use device path library utilities don't forget to add `DevicePathLib` to the `[LibraryClasses]` in the `UefiLessonsPkg/HIIConfig/HIIConfig.inf` file and add its header `#include <Library/DevicePathLib.h>` to the `UefiLessonsPkg/HIIConfig/HIIConfig.c` file.

## `VALUE=`

Handling code for `VALUE=` key looks like this:
```cpp
PrintLongString(ConfigString);
Print(L"\n");
UINT8* Buffer;
UINTN BufferSize;
ByteCfgStringToBufferReversed(&ConfigString[StrLen(L"VALUE=")], StrLen(&ConfigString[StrLen(L"VALUE=")]), &Buffer, &BufferSize);
PrintBuffer(Buffer, BufferSize);
FreePool(Buffer);
```
In this case we first print data as a string. But because it can be longer than `gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize`, we can't just use ordinary `Print` statement. We use our custom `PrintLongString` function that prints `<...>` at the end of a string that will be truncated on an ordinary print. This way we have a clear way to see if the output is complete or not:
```cpp
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
```
Next we want to get the actual data in the array. The data after the `VALUE=` key is encoded in bytes, but the order is reversed. That means that data like this:
```
VALUE=112233445566
```
actually corresponds to a byte array:
```cpp
{0x66, 0x55, 0x44, 0x33, 0x22, 0x11}
```
That is why we use different function to get the byte array than in `GUID=` or `PATH=` cases. `ByteCfgStringToBufferReversed` is a little bit different than `ByteCfgStringToBuffer`:
```cpp
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
```

Finally we display the value buffer similar to the Linux `hexdump -C` utility output:
```cpp
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
```

# Putting it all together

Now when we have all the parsing code we can finally test it. Here is the output of the `HIIConfig.efi dump` command. I've just truncated the bunch of zeros in the middle for a better readability:
```
FS0:\> HIIConfig.efi dump
Full configuration for the HII Database (Size = 42018):

GUID=1cc53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
OFFSET=0000  WIDTH=0020  VALUE=00000000000000000000000000000000000000000000007400650073006e0055
55 00 6E 00 73 00 65 00 74 00 00 00 00 00 00 00  | U.n.s.e.t.......
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
OFFSET=0020  WIDTH=0004  VALUE=00000000
00 00 00 00                                      | ....

GUID=1cc53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
ALTCFG=0000
OFFSET=0020  WIDTH=0004  VALUE=00000000
00 00 00 00                                      | ....

GUID=1cc53572800cab4c87ac3b084a6304b1 (7235C51C-0C80-4CAB-87AC-3B084A6304B1)
NAME=004d00610069006e0046006f0072006d00530074006100740065 (MainFormState)
PATH=01041400dfc5dcd907405e4390988970935504b27fff0400 (VenHw(D9DCC5DF-4007-435E-9098-8970935504B2))
ALTCFG=0001
OFFSET=0020  WIDTH=0004  VALUE=00000000
00 00 00 00                                      | ....

GUID=16d6474bd6a852459d44ccad2e0f4cf9 (4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9)
NAME=00490053004300530049005f0043004f004e004600490047005f004900460052005f004e00560044004100540041 (ISCSI_CONFIG_IFR_NVDATA)
PATH=0104140016d6474bd6a852459d44ccad2e0f4cf97fff0400 (VenHw(4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9))
OFFSET=0  WIDTH=000000000000453c  VALUE=00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000<...>
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
...                                                                             <----- truncated output
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
00 00 00 00 00 00 00 00 00 00 00 00              | ............

GUID=16d6474bd6a852459d44ccad2e0f4cf9 (4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9)
NAME=00490053004300530049005f0043004f004e004600490047005f004900460052005f004e00560044004100540041 (ISCSI_CONFIG_IFR_NVDATA)
PATH=0104140016d6474bd6a852459d44ccad2e0f4cf97fff0400 (VenHw(4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9))
ALTCFG=0000
OFFSET=01d8  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=01d9  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=01da  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=01dc  WIDTH=0002  VALUE=03e8
E8 03                                            | ..
OFFSET=01de  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=01df  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=05fe  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=062a  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=062b  WIDTH=0001  VALUE=01
01                                               | .
OFFSET=0fd4  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fd5  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fd6  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fd7  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fd8  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fd9  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fda  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fdb  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fdc  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fdd  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fde  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fdf  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe0  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe1  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe2  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe3  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe4  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe5  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe6  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe7  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe8  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe9  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fea  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0feb  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fec  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0fee  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ff0  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ff2  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ff4  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ff6  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ff8  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ffa  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ffc  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0ffd  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0ffe  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fff  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1001  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1002  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1003  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1004  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1005  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1006  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1007  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1008  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1009  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=100a  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=100b  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=100c  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=100e  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=1010  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=1012  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=1014  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=1016  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=1018  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=101a  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=101c  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=101d  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=101e  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=101f  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1020  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1021  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1022  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1023  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1024  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1025  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1026  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1027  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1028  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1029  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=102a  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=102b  WIDTH=0001  VALUE=00
00                                               | .

GUID=16d6474bd6a852459d44ccad2e0f4cf9 (4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9)
NAME=00490053004300530049005f0043004f004e004600490047005f004900460052005f004e00560044004100540041 (ISCSI_CONFIG_IFR_NVDATA)
PATH=0104140016d6474bd6a852459d44ccad2e0f4cf97fff0400 (VenHw(4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9))
ALTCFG=0001
OFFSET=01d8  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=01d9  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=01da  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=01dc  WIDTH=0002  VALUE=03e8
E8 03                                            | ..
OFFSET=01de  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=01df  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=05fe  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=062a  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=062b  WIDTH=0001  VALUE=01
01                                               | .
OFFSET=0fd4  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fd5  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fd6  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fd7  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fd8  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fd9  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fda  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fdb  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fdc  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fdd  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fde  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fdf  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe0  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe1  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe2  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe3  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe4  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe5  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe6  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe7  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe8  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fe9  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fea  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0feb  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fec  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0fee  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ff0  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ff2  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ff4  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ff6  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ff8  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ffa  WIDTH=0002  VALUE=0064
64 00                                            | d.
OFFSET=0ffc  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0ffd  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0ffe  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0fff  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1001  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1002  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1003  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1004  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1005  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1006  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1007  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1008  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1009  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=100a  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=100b  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=100c  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=100e  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=1010  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=1012  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=1014  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=1016  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=1018  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=101a  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=101c  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=101d  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=101e  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=101f  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1020  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1021  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1022  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1023  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1024  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1025  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1026  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1027  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1028  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=1029  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=102a  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=102b  WIDTH=0001  VALUE=00
00                                               | .
```

