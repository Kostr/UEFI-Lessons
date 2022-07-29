When we put file in the `FD` directly we set file address/size explicitly. We can assign PCDs to these values and use them in the code to access data.

But when you put files in the `FV`, you don't have anything like that. Files are placed one after another in the FFS. But lukily UEFI offers us a protocol to access files from the `FV` - `EFI_FIRMWARE_VOLUME2_PROTOCOL`.

In this definition from the specification you can see what capabilities it offers:
```
EFI_FIRMWARE_VOLUME2_PROTOCOL

Summary:
The Firmware Volume Protocol provides file-level access to the firmware volume. Each firmware volume driver must produce an instance of the Firmware Volume Protocol if the firmware volume is to be visible to the system during the DXE phase. The Firmware Volume Protocol also provides mechanisms for determining and modifying some attributes of the firmware volume.

GUID:
#define EFI_FIRMWARE_VOLUME2_PROTOCOL_GUID \
 { 0x220e73b6, 0x6bdb, 0x4413, 0x84, 0x5, 0xb9, 0x74, \
 0xb1, 0x8, 0x61, 0x9a }

Protocol Interface Structure:
typedef struct_EFI_FIRMWARE_VOLUME_PROTOCOL {
 EFI_FV_GET_ATTRIBUTES GetVolumeAttributes;
 EFI_FV_SET_ATTRIBUTES SetVolumeAttributes;
 EFI_FV_READ_FILE ReadFile;
 EFI_FV_READ_SECTION ReadSection;
 EFI_FV_WRITE_FILE WriteFile;
 EFI_FV_GET_NEXT_FILE GetNextFile;
 UINT32 KeySize;
 EFI_HANDLE ParentHandle;
 EFI_FV_GET_INFO GetInfo;
 EFI_FV_SET_INFO SetInfo;
} EFI_FIRMWARE_VOLUME2_PROTOCOL;

Parameters:
GetVolumeAttributes	Retrieves volume capabilities and current settings
SetVolumeAttributes	Modifies the current settings of the firmware volume
ReadFile		Reads an entire file from the firmware volume
ReadSection		Reads a single section from a file into a buffer
WriteFile		Writes an entire file into the firmware volume
GetNextFile		Provides service to allow searching the firmware volume
KeySize			Data field that indicates the size in bytes of the Key input buffer for the GetNextFile() API.
ParentHandle		Handle of the parent firmware volume
GetInfo			Gets the requested file or volume information
SetInfo			Sets the requested file information

Description:
The Firmware Volume Protocol contains the file-level abstraction to the firmware volume as well as 
some firmware volume attribute reporting and configuration services.
```

Let's write an application that would utilize this protocol to read files from the current FV.

We name our application `FfsFile`. Here is its help message:
```
VOID Usage()
{
  Print(L"Usage:\n");
  Print(L"  FfsFile [<FileGUID> [<SectionType> <SectionInstance>]]\n");
  Print(L"\n");
  Print(L"<FileGUID>:\n");
  Print(L"GUID name of the File in FFS\n");
  Print(L"\n");
  Print(L"<SectionType>:\n");
  Print(L"ALL|COMPRESS|GUIDED|DISPOSABLE|PE32|PIC|TE|DXE_DEPEX|PEI_DEPEX|MM_DEPEX\n");
  Print(L"VERSION|UI|COMPAT16|FV_IMAGE|SUBTYPE_GUID|RAW\n");
  Print(L"\n");
  Print(L"<SectionInstance>: section instance number in a target file\n");
}
```
Our application allows 3 methods of launching:
- `FfsFile` - list all available files,
- `FfsFile <FileGUID>` - output information about specific file,
- `FfsFile <FileGUID> <SectionType> <SectionInstance>` - output information about specific section in the specific file.

As you see, we need to parse incoming arguments, therefore it is better to construct our application with `ShellCEntryLib`. Also we would use the aforementioned `EFI_FIRMWARE_VOLUME_PROTOCOL`, so we need to include this as a protocol to the INF file `UefiLessonsPkg/FfsFile/FfsFile.inf`:
```
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = FfsFile
  FILE_GUID                      = a60ea21e-f679-4896-b4b0-85f77e150b83
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = ShellCEntryLib

[Sources]
  FfsFile.c

[Packages]
  MdePkg/MdePkg.dec

[LibraryClasses]
  ShellCEntryLib
  UefiLib

[Protocols]
  gEfiFirmwareVolume2ProtocolGuid
```

Now the C code. First we need to parse input arguments:
```cpp
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS Status;

  if ((Argc != 1) && (Argc != 2) && (Argc != 4)) {
    Usage();
    return EFI_INVALID_PARAMETER;
  }

  EFI_GUID FileGuid;
  if (Argc > 1) {
    Status = StrToGuid(Argv[1], &FileGuid);
    if (Status != RETURN_SUCCESS) {
      Print(L"Error! Can't convert <File GUID> argument to GUID\n");
      return EFI_INVALID_PARAMETER;
    }
  }

  EFI_SECTION_TYPE SectionType;
  UINTN SectionInstance;
  if (Argc == 4) {
    if (!StrCmp(Argv[2], L"ALL"))
      SectionType = EFI_SECTION_ALL;
    else if (!StrCmp(Argv[2], L"COMPRESS"))
      SectionType = EFI_SECTION_COMPRESSION;
    else if (!StrCmp(Argv[2], L"GUIDED"))
      SectionType = EFI_SECTION_GUID_DEFINED;
    else if (!StrCmp(Argv[2], L"DISPOSABLE"))
      SectionType = EFI_SECTION_DISPOSABLE;
    else if (!StrCmp(Argv[2], L"PE32"))
      SectionType = EFI_SECTION_PE32;
    else if (!StrCmp(Argv[2], L"PIC"))
      SectionType = EFI_SECTION_PIC;
    else if (!StrCmp(Argv[2], L"TE"))
      SectionType = EFI_SECTION_TE;
    else if (!StrCmp(Argv[2], L"DXE_DEPEX"))
      SectionType = EFI_SECTION_DXE_DEPEX;
    else if (!StrCmp(Argv[2], L"VERSION"))
      SectionType = EFI_SECTION_VERSION;
    else if (!StrCmp(Argv[2], L"UI"))
      SectionType = EFI_SECTION_USER_INTERFACE;
    else if (!StrCmp(Argv[2], L"COMPAT16"))
      SectionType = EFI_SECTION_COMPATIBILITY16;
    else if (!StrCmp(Argv[2], L"FV_IMAGE"))
      SectionType = EFI_SECTION_FIRMWARE_VOLUME_IMAGE;
    else if (!StrCmp(Argv[2], L"SUBTYPE_GUID"))
      SectionType = EFI_SECTION_FREEFORM_SUBTYPE_GUID;
    else if (!StrCmp(Argv[2], L"RAW"))
      SectionType = EFI_SECTION_RAW;
    else if (!StrCmp(Argv[2], L"PEI_DEPEX"))
      SectionType = EFI_SECTION_PEI_DEPEX;
    else if (!StrCmp(Argv[2], L"MM_DEPEX"))
      SectionType = EFI_SECTION_MM_DEPEX;
    else {
      Print(L"Error! Wrong <SectionType>\n");
      return EFI_INVALID_PARAMETER;
    }

    SectionInstance = StrDecimalToUintn(Argv[3]);
  }

  <...>

  return EFI_SUCCESS;
}
```

Right after the argument parsing we need to get the protocol:
```cpp
EFI_FIRMWARE_VOLUME2_PROTOCOL* FV2Protocol;
Status = gBS->LocateProtocol(
                &gEfiFirmwareVolume2ProtocolGuid,
                NULL,
                (VOID**)&FV2Protocol
              );
if (EFI_ERROR(Status)) {
  Print(L"Error! Can't locate EFI_FIRMWARE_VOLUME2_PROTOCOL: %r\n", Status);
  return Status;
}
```

Don't forget to add these headers for the protocol:
```cpp
#include <Pi/PiFirmwareFile.h>
#include <Pi/PiFirmwareVolume.h>
#include <Protocol/FirmwareVolume2.h>
```

Next we split the program functionality to three functions:
```cpp
if (Argc == 1)
  return PrintFiles(FV2Protocol);

if (Argc == 2)
  return ReadFile(FV2Protocol, &FileGuid);

if (Argc == 4)
  return ReadSection(FV2Protocol, &FileGuid, SectionType, SectionInstance);
```

# `FfsFile`

Let's start with a first one:
```cpp
EFI_STATUS
PrintFiles(
  IN EFI_FIRMWARE_VOLUME2_PROTOCOL* FV2Protocol
  )
{
  EFI_STATUS Status;
  EFI_GUID FileGuid;
  EFI_FV_FILE_ATTRIBUTES Attributes;
  UINTN Size;
  EFI_FV_FILETYPE FileType;
  UINTN Key = 0;

  do {
    FileType = EFI_FV_FILETYPE_ALL;
    Status = FV2Protocol->GetNextFile(
                            FV2Protocol,
                            (VOID**)&Key,
                            &FileType,
                            &FileGuid,
                            &Attributes,
                            &Size
                          );
    if (!EFI_ERROR(Status))
      Print(L"%g - %s - %d bytes\n", FileGuid, FileTypeString(FileType), Size);
  } while (Status == EFI_SUCCESS);

  return EFI_SUCCESS;
}
```

Here the function utilizes `GetNextFile()` function from the protocol. Simplified description from the specification:
```
EFI_FIRMWARE_VOLUME2_PROTOCOL.GetNextFile()

Summary:
Retrieves information about the next file in the firmware volume store that matches the search criteria.

Prototype:
typedef 
EFI_STATUS
(EFIAPI * EFI_FV_GET_NEXT_FILE) (
 IN CONST EFI_FIRMWARE_VOLUME2_PROTOCOL *This,
 IN OUT VOID *Key,
 IN OUT EFI_FV_FILETYPE *FileType,
 OUT EFI_GUID *NameGuid,
 OUT EFI_FV_FILE_ATTRIBUTES *Attributes,
 OUT UINTN *Size
 );

Parameters:
This		Indicates the EFI_FIRMWARE_VOLUME2_PROTOCOL instance
Key		Pointer to a caller-allocated buffer that contains implementation-specific data that is used to track where to begin the search for the next file
FileType	Pointer to a caller-allocated EFI_FV_FILETYPE. The GetNextFile() API can filter its search for files based on the value of the *FileType input.
		A *FileType input of EFI_FV_FILETYPE_ALL causes GetNextFile() to search for files of all types. If a file is found, the file’s type is returned in *FileType.
NameGuid	Pointer to a caller-allocated EFI_GUID. If a matching file is found, the file’s name is returned in *NameGuid
Attributes	Pointer to a caller-allocated EFI_FV_FILE_ATTRIBUTES. If a matching file is found, the file’s attributes are returned in *Attributes
Size		Pointer to a caller-allocated UINTN. If a matching file is found, the file’s size is returned in *Size

Description:
GetNextFile() is the interface that is used to search a firmware volume for a particular file. It is called successively until the desired file is located or the function returns EFI_NOT_FOUND.
```
To enumerate all the files in the FFS we just need to use this function continiously until it would return an error.

`FileTypeString` that we've here returns a description string for the `EFI_FV_FILETYPE` value:
```cpp
CHAR16* FileTypeString(EFI_FV_FILETYPE FileType)
{
  if (FileType == EFI_FV_FILETYPE_RAW)
    return L"RAW";
  else if (FileType == EFI_FV_FILETYPE_FREEFORM)
    return L"FREEFORM";
  else if (FileType == EFI_FV_FILETYPE_SECURITY_CORE)
    return L"SEC_CORE";
  else if (FileType == EFI_FV_FILETYPE_PEI_CORE)
    return L"PEI_CORE";
  else if (FileType == EFI_FV_FILETYPE_DXE_CORE)
    return L"DXE_CORE";
  else if (FileType == EFI_FV_FILETYPE_PEIM)
    return L"PEIM";
  else if (FileType == EFI_FV_FILETYPE_DRIVER)
    return L"DRIVER";
  else if (FileType == EFI_FV_FILETYPE_COMBINED_PEIM_DRIVER)
    return L"COMBINED_PEIM_DRIVER";
  else if (FileType == EFI_FV_FILETYPE_APPLICATION)
    return L"APPLICATION";
  else if (FileType == EFI_FV_FILETYPE_MM)
    return L"MM";
  else if (FileType == EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE)
    return L"FV_IMAGE";
  else if (FileType == EFI_FV_FILETYPE_COMBINED_MM_DXE)
    return L"COMBINED_MM_DXE";
  else if (FileType == EFI_FV_FILETYPE_MM_CORE)
    return L"MM_CORE";
  else if (FileType == EFI_FV_FILETYPE_MM_STANDALONE)
    return L"MM_STANDALONE";
  else if (FileType == EFI_FV_FILETYPE_MM_CORE_STANDALONE)
    return L"MM_CORE_STANDALONE";
  else if ((FileType >= 0xC0) && (FileType <= 0xDF))
    return L"OEM";
  else if ((FileType >= 0xE0) && (FileType <= 0xEF))
    return L"DEBUG";
  else if (FileType == EFI_FV_FILETYPE_FFS_PAD)
    return L"FFS_PAD";
  else if ((FileType >= 0xF0) && (FileType <= 0xFF))
    return L"FFS";
  else
    return L"UNKNOWN";
}
```

This is the output that I've recieved in OVMF with our application:
```
FS0:\> FfsFile.efi
FC510EE7-FFDC-11D4-BD41-0080C73C8881 - FREEFORM - 68 bytes
D6A2CB7F-6A18-4E2F-B43B-9920A733700A - DXE_CORE - 103590 bytes
D93CE3D8-A7EB-4730-8C8E-CC466A9ECC3C - DRIVER - 16514 bytes
6C2004EF-4E0E-4BE4-B14C-340EB4AA5891 - DRIVER - 12426 bytes
80CF7257-87AB-47F9-A3FE-D50B76D89541 - DRIVER - 10338 bytes
B601F8C4-43B7-4784-95B1-F4226CB40CEE - DRIVER - 16470 bytes
F80697E9-7FD6-4665-8646-88E33EF71DFC - DRIVER - 2974 bytes
13AC6DD0-73D0-11D4-B06B-00AA00BD6DE7 - DRIVER - 15742 bytes
245CB4DA-8E15-4A1B-87E3-9878FFA07520 - DRIVER - 2502 bytes
A19B1FE7-C1BC-49F8-875F-54A5D542443F - DRIVER - 2306 bytes
1A1E4886-9517-440E-9FDE-3BE44CEE2136 - DRIVER - 46990 bytes
C190FE35-44AA-41A1-8AEA-4947BC60E09D - DRIVER - 1758 bytes
F6697AC4-A776-4EE1-B643-1FEFF2B615BB - DRIVER - 1710 bytes
11A6EDF6-A9BE-426D-A6CC-B22FE51D9224 - DRIVER - 7394 bytes
128FB770-5E79-4176-9E51-9BB268A17DD1 - DRIVER - 15182 bytes
93B80004-9FB3-11D4-9A3A-0090273FC14D - DRIVER - 40298 bytes
4B28E4C7-FF36-4E10-93CF-A82159E777C5 - DRIVER - 16490 bytes
C8339973-A563-4561-B858-D8476F9DEFC4 - DRIVER - 1474 bytes
378D7B65-8DA9-4773-B6E4-A47826A833E1 - DRIVER - 16494 bytes
83DD3B39-7CAF-4FAC-A542-E050B767E3A7 - DRIVER - 3838 bytes
0170F60C-1D40-4651-956D-F0BD9879D527 - DRIVER - 8234 bytes
11D92DFB-3CA9-4F93-BA2E-4780ED3E03B5 - DRIVER - 5426 bytes
FAB5D4F4-83C0-4AAF-8480-442D11DF6CEA - DRIVER - 6834 bytes
58E26F0D-CBAC-4BBA-B70F-18221415665A - DRIVER - 4402 bytes
30346B14-1580-4781-879D-BA0C55AE9BB2 - DRIVER - 5482 bytes
2B3DB5DD-B315-4961-8454-0AFF3C811B19 - DRIVER - 5038 bytes
F099D67F-71AE-4C36-B2A3-DCEB0EB2B7D8 - DRIVER - 1646 bytes
AD608272-D07F-4964-801E-7BD3B7888652 - DRIVER - 12426 bytes
42857F0A-13F2-4B21-8A23-53D3F714B840 - DRIVER - 12406 bytes
51CCF399-4FDF-4E55-A45B-E123F84D456A - DRIVER - 6518 bytes
408EDCEC-CF6D-477C-A5A8-B4844E3DE281 - DRIVER - 18870 bytes
CCCB0C28-4B24-11D5-9A5A-0090273FC14D - DRIVER - 14654 bytes
9E863906-A40F-4875-977F-5B93FF237FC6 - DRIVER - 18030 bytes
EBF8ED7C-0DD1-4787-84F1-F48D537DCACF - DRIVER - 15554 bytes
6D33944A-EC75-4855-A54D-809C75241F6C - DRIVER - 72034 bytes
462CAA21-7614-4503-836E-8AB6F4662331 - APPLICATION - 111766 bytes
806040CA-DAD9-4978-A3B4-2D2AB0C8A48F - DRIVER - 5346 bytes
9B680FCE-AD6B-4F3A-B60B-F59899003443 - DRIVER - 37114 bytes
79E4A61C-ED73-4312-94FE-E3E7563362A9 - DRIVER - 6786 bytes
6B38F7B4-AD98-40E9-9093-ACA2B5A253C4 - DRIVER - 6954 bytes
1FA1F39E-FEFF-4AAE-BD7B-38A070A3B609 - DRIVER - 13170 bytes
28A03FF4-12B3-4305-A417-BB1A4F94081E - DRIVER - 28948 bytes
CD3BAFB6-50FB-4FE8-8E4E-AB74D2C1A600 - DRIVER - 2990 bytes
0167CCC4-D0F7-4F21-A3EF-9E64B7CDCE8B - DRIVER - 7142 bytes
0A66E322-3740-4CCE-AD62-BD172CECCA35 - DRIVER - 26474 bytes
021722D8-522B-4079-852A-FE44C2C13F49 - DRIVER - 4278 bytes
5E523CB4-D397-4986-87BD-A6DD8B22F455 - DRIVER - 25982 bytes
19DF145A-B1D4-453F-8507-38816676D7F6 - DRIVER - 12330 bytes
5BE3BDF4-53CF-46A3-A6A9-73C34A6E5EE3 - DRIVER - 24626 bytes
348C4D62-BFBD-4882-9ECE-C80BB1C4783B - DRIVER - 86678 bytes
EBF342FE-B1D3-4EF8-957C-8048606FF671 - DRIVER - 65938 bytes
E660EA85-058E-4B55-A54B-F02F83A24707 - DRIVER - 59134 bytes
96B5C032-DF4C-4B6E-8232-438DCF448D0E - DRIVER - 1890 bytes
864E1CA8-85EB-4D63-9DCC-6E0FC90FFD55 - DRIVER - 4714 bytes
E2775B47-D453-4EE3-ADA7-391A1B05AC17 - DRIVER - 16822 bytes
C4D1F932-821F-4744-BF06-6D30F7730F8D - DRIVER - 11574 bytes
F9D88642-0737-49BC-81B5-6889CD57D9EA - DRIVER - 8634 bytes
4110465D-5FF3-4F4B-B580-24ED0D06747A - DRIVER - 3142 bytes
9622E42C-8E38-4A08-9E8F-54F784652F6B - DRIVER - 17306 bytes
17985E6F-E778-4D94-AEFA-C5DD2B77E186 - DRIVER - 9422 bytes
BDCE85BB-FBAA-4F4E-9264-501A2C249581 - DRIVER - 10974 bytes
FA20568B-548B-4B2B-81EF-1BA08D4A3CEC - DRIVER - 35518 bytes
B8E62775-BB0A-43F0-A843-5BE8B14F8CCD - DRIVER - 3154 bytes
961578FE-B6B7-44C3-AF35-6BC705CD2B1F - DRIVER - 23134 bytes
905F13B0-8F91-4B0A-BD76-E1E78F9422E4 - DRIVER - 13350 bytes
7BD9DDF7-8B83-488E-AEC9-24C78610289C - DRIVER - 20974 bytes
A487A478-51EF-48AA-8794-7BEE2A0562F1 - DRIVER - 30174 bytes
19618BCE-55AE-09C6-37E9-4CE04084C7A1 - DRIVER - 38558 bytes
2F30DA26-F51B-4B6F-85C4-31873C281BCA - DRIVER - 20790 bytes
7C04A583-9E3E-4F1C-AD65-E05268D0B4D1 - APPLICATION - 855486 bytes
F74D20EE-37E7-48FC-97F7-9B1047749C69 - DRIVER - 13602 bytes
A210F973-229D-4F4D-AA37-9895E6C9EABA - DRIVER - 2174 bytes
A2F436EA-A127-4EF8-957C-8048606FF670 - DRIVER - 18342 bytes
E4F61863-FE2C-4B56-A8F4-08519BC439DF - DRIVER - 18214 bytes
025BBFC7-E6A9-4B8B-82AD-6815A1AEAF4A - DRIVER - 23910 bytes
529D3F93-E8E9-4E73-B1E1-BDF6A9D50113 - DRIVER - 11494 bytes
94734718-0BBC-47FB-96A5-EE7A5AE6A2AD - DRIVER - 28522 bytes
9FB1A1F3-3B71-4324-B39A-745CBB015FFF - DRIVER - 58138 bytes
6D6963AB-906D-4A65-A7CA-BD40E5D6AF2B - DRIVER - 23910 bytes
DC3641B8-2FA8-4ED3-BC1F-F9962A03454B - DRIVER - 25962 bytes
1A7E4468-2F55-4A56-903C-01265EB7622B - DRIVER - 43878 bytes
B95E9FDA-26DE-48D2-8807-1F9107AC5E3A - DRIVER - 56370 bytes
86CDDF93-4872-4597-8AF9-A35AE4D3725F - DRIVER - 100766 bytes
A92CDB4B-82F1-4E0B-A516-8A655D371524 - DRIVER - 10802 bytes
2FB92EFA-2EE0-4BAE-9EB6-7464125E1EF7 - DRIVER - 14822 bytes
BDFE430E-8F2A-4DB0-9991-6F856594777E - DRIVER - 17446 bytes
B7F50E91-A759-412C-ADE4-DCD03E7F7C28 - DRIVER - 26918 bytes
240612B7-A063-11D4-9A3A-0090273FC14D - DRIVER - 17066 bytes
2D2E62CF-9ECF-43B7-8219-94E7FC713DFE - DRIVER - 14314 bytes
9FB4B4A7-42C0-4BCD-8540-9BCC6711F83E - DRIVER - 11002 bytes
E3752948-B9A1-4770-90C4-DF41C38986BE - DRIVER - 16498 bytes
DCE1B094-7DC6-45D0-9FDD-D7FC3CC3E4EF - DRIVER - 6914 bytes
D6099B94-CD97-4CC5-8714-7F6312701A8A - DRIVER - 13938 bytes
D9DCC5DF-4007-435E-9098-8970935504B2 - DRIVER - 11622 bytes
2EC9DA37-EE35-4DE9-86C5-6D9A81DC38A7 - DRIVER - 3282 bytes
8657015B-EA43-440D-949A-AF3BE365C0FC - DRIVER - 4866 bytes
733CBAC2-B23F-4B92-BC8E-FB01CE5907B7 - DRIVER - 20586 bytes
22DC2B60-FE40-42AC-B01F-3AB1FAD9AAD8 - DRIVER - 16498 bytes
FE5CEA76-4F72-49E8-986F-2CD899DFFE5D - DRIVER - 10190 bytes
CBD2E4D5-7068-4FF5-B462-9822B4AD8D60 - DRIVER - 41062 bytes
```

The most of the files are DXE DRIVERs except these ones:
```
FC510EE7-FFDC-11D4-BD41-0080C73C8881 - FREEFORM - 68 bytes		# DXE APRIORI file
D6A2CB7F-6A18-4E2F-B43B-9920A733700A - DXE_CORE - 103590 bytes          # DXE CORE (MdeModulePkg/Core/Dxe/DxeMain.inf)
...
462CAA21-7614-4503-836E-8AB6F4662331 - APPLICATION - 111766 bytes       # UiApp (MdeModulePkg/Application/UiApp/UiApp.inf)
...
7C04A583-9E3E-4F1C-AD65-E05268D0B4D1 - APPLICATION - 855486 bytes       # Shell (ShellPkg/Application/Shell/Shell.inf)
...

```

# `FfsFile <FileGUID>`

Now to the second part - outputing file information by its GUID name.

For this functionality we would need `ReadFile` function from the protocol:
```
EFI_FIRMWARE_VOLUME2_PROTOCOL.ReadFile()

Summary:
Retrieves a file and/or file information from the firmware volume.

Prototype:
typedef 
EFI_STATUS
(EFIAPI * EFI_FV_READ_FILE) (
 IN CONST EFI_FIRMWARE_VOLUME2_PROTOCOL *This,
 IN CONST EFI_GUID *NameGuid,
 IN OUT VOID **Buffer,
 IN OUT UINTN *BufferSize,
 OUT EFI_FV_FILETYPE *FileType,
 OUT EFI_FV_FILE_ATTRIBUTES *FileAttributes,
 OUT UINT32 *AuthenticationStatus
 );

Parameters:
This			Indicates the EFI_FIRMWARE_VOLUME2_PROTOCOL instance
NameGuid		Pointer to an EFI_GUID, which is the file name
Buffer			Pointer to a pointer to a buffer in which the file contents are returned, not including the file header
BufferSize		Pointer to a caller-allocated UINTN. It indicates the size of the memory represented by *Buffer
FileType		Pointer to a caller-allocated EFI_FV_FILETYPE
FileAttributes		Pointer to a caller-allocated EFI_FV_FILE_ATTRIBUTES
AuthenticationStatus	Pointer to a caller-allocated UINT32 in which the authentication status is returned

Description:
ReadFile() is used to retrieve any file from a firmware volume during the DXE phase.
```

Here is how we can use it:
```cpp
EFI_STATUS
ReadFile(
  IN EFI_FIRMWARE_VOLUME2_PROTOCOL* FV2Protocol,
  IN EFI_GUID* FileGuid
  )
{
  EFI_STATUS Status;
  UINT8* Buffer = NULL;
  UINTN BufferSize;
  EFI_FV_FILETYPE FileType;
  EFI_FV_FILE_ATTRIBUTES FileAttributes;
  UINT32 AuthenticationStatus;
  Status = FV2Protocol->ReadFile(
                          FV2Protocol,
                          FileGuid,
                          (VOID**)&Buffer,
                          &BufferSize,
                          &FileType,
                          &FileAttributes,
                          &AuthenticationStatus
                        );
  if (EFI_ERROR(Status)) {
    Print(L"Error! ReadFile returned error: %r\n", Status);
    return Status;
  }

  Print(L"FileType=%s\n", FileTypeString(FileType));
  Print(L"FileAttributes=0x%08x\n", FileAttributes);
  Print(L"AuthenticationStatus=0x%08x\n", AuthenticationStatus);
  Print(L"\n");

  Print(L"Raw Data:\n");
  PrintBuffer(Buffer, BufferSize);

  <...>

  FreePool(Buffer);
  return EFI_SUCCESS;
}
```

We use custom `VOID PrintBuffer(UINT8* Buffer, UINTN BufferSize)` function to output retrieved buffer data similar to `hexdump`. You can look at the sources for the actual code.

Besides printing raw buffer let's try to manually parse it. If the file has RAW type, it wouldn't have any sections but in other cases we can try to parse `EFI_COMMON_SECTION_HEADER` data at the start and walk sections one by one. Don't forget that every section would be aligned to 4-byte boundary:
```cpp
  Print(L"-------------------------------------\n");
  if (FileType == EFI_FV_FILETYPE_RAW)
    return EFI_SUCCESS;
  Print(L"Parsed Data:\n\n");
  UINTN i=0;
  while (i < BufferSize) {
    EFI_COMMON_SECTION_HEADER* SectionHeader = (EFI_COMMON_SECTION_HEADER*) &Buffer[i];
    UINTN SectionSize = SectionHeader->Size[0] + (SectionHeader->Size[1] << 8) + (SectionHeader->Size[2] << 16);
    Print(L"Section %s, size 0x%08x\n", SectionTypeString(SectionHeader->Type), SectionSize);
    Print(L"Data:\n");
    PrintBuffer(&Buffer[i] + sizeof(EFI_COMMON_SECTION_HEADER), SectionSize - sizeof(EFI_COMMON_SECTION_HEADER));
    i += SectionSize;
    if (i%4)
      i += (4-(i%4));
  }
```

The `SectionTypeString` here is a helper to transform `EFI_SECTION_TYPE` value to the text representation similar to `FileTypeString`:
```cpp
CHAR16* SectionTypeString(EFI_SECTION_TYPE SectionType)
{
  if (SectionType == EFI_SECTION_ALL)
    return L"ALL";
  else if (SectionType == EFI_SECTION_COMPRESSION)
    return L"COMPRESSION";
  else if (SectionType == EFI_SECTION_GUID_DEFINED)
    return L"GUID_DEFINED";
  else if (SectionType == EFI_SECTION_DISPOSABLE)
    return L"DISPOSABLE";
  else if (SectionType == EFI_SECTION_PE32)
    return L"PE32";
  else if (SectionType == EFI_SECTION_PIC)
    return L"PIC";
  else if (SectionType == EFI_SECTION_TE)
    return L"TE";
  else if (SectionType == EFI_SECTION_DXE_DEPEX)
    return L"DXE_DEPEX";
  else if (SectionType == EFI_SECTION_VERSION)
    return L"VERSION";
  else if (SectionType == EFI_SECTION_USER_INTERFACE)
    return L"USER_INTERFACE";
  else if (SectionType == EFI_SECTION_COMPATIBILITY16)
    return L"COMPATIBILITY16";
  else if (SectionType == EFI_SECTION_FIRMWARE_VOLUME_IMAGE)
    return L"FV_IMAGE";
  else if (SectionType == EFI_SECTION_FREEFORM_SUBTYPE_GUID)
    return L"SUBTYPE_GUID";
  else if (SectionType == EFI_SECTION_RAW)
    return L"RAW";
  else if (SectionType == EFI_SECTION_PEI_DEPEX)
    return L"PEI_DEPEX";
  else if (SectionType == EFI_SECTION_MM_DEPEX)
    return L"MM_DEPEX";
  else
    return L"UNKNOWN";
}
```

For the test create 3 text files:
```
$ echo "hello" > "hello.txt"
$ echo "my" > "my.txt"
$ echo "world!" > "world.txt"
```

And add such complicated file to the `FV.DXEFV`:
```
[FV.DXEFV]
...
FILE FREEFORM = 9ccae251-a1e0-4999-af44-780bacaf9a3a {
  SECTION RAW = $(WORKSPACE)/hello.txt
  SECTION GUIDED EE4E5898-3914-4259-9D6E-DC7BD79403CF PROCESSING_REQUIRED = TRUE {
    SECTION RAW = $(WORKSPACE)/my.txt
    SECTION COMPRESS PI_STD {
      SECTION RAW = $(WORKSPACE)/world.txt
      SECTION UI = "MyUI"
    }
  }
}
```

Rebuild OVMF image:
```
build --platform=OvmfPkg/OvmfPkgX64.dsc --arch=X64 --buildtarget=RELEASE --tagname=GCC5
```

Verify that our file indeed is added to the FFS:
```
FS0:\> FfsFile.efi
...
9CCAE251-A1E0-4999-AF44-780BACAF9A3A - FREEFORM - 109 bytes
...
```

Now output information about our file:
```
FS0:\> FfsFile.efi 9ccae251-a1e0-4999-af44-780bacaf9a3a
FileType=FREEFORM
FileAttributes=0x00000200
AuthenticationStatus=0x00000000

Raw Data:
0x00000000: 0A 00 00 19 68 65 6C 6C 6F 0A 00 00 61 00 00 02  |....hello...a...|
0x00000010: 98 58 4E EE 14 39 59 42 9D 6E DC 7B D7 94 03 CF  |.XN..9YB.n.{....|
0x00000020: 18 00 01 00 5D 00 00 00 01 3B 00 00 00 00 00 00  |....]....;......|
0x00000030: 00 00 03 80 32 73 81 53 56 D6 22 35 CA 8A 1A 95  |....2s.SV."5....|
0x00000040: E8 C9 C4 E4 6E DF C6 72 98 8B 8E C5 C4 5D 2B D5  |....n..r.....]+.|
0x00000050: FB 5A 2D E3 44 74 55 04 BE 7A F7 E0 4E 88 80 6D  |.Z-.DtU..z..N..m|
0x00000060: 23 E0 DD 04 03 2C 03 74 2A E3 C4 00 00           |#....,.t*....|

-------------------------------------
Parsed Data:

Section RAW, size 0x0000000A
Data:
0x00000000: 68 65 6C 6C 6F 0A                                |hello.|

Section GUID_DEFINED, size 0x00000061
Data:
0x00000000: 98 58 4E EE 14 39 59 42 9D 6E DC 7B D7 94 03 CF  |.XN..9YB.n.{....|
0x00000010: 18 00 01 00 5D 00 00 00 01 3B 00 00 00 00 00 00  |....]....;......|
0x00000020: 00 00 03 80 32 73 81 53 56 D6 22 35 CA 8A 1A 95  |....2s.SV."5....|
0x00000030: E8 C9 C4 E4 6E DF C6 72 98 8B 8E C5 C4 5D 2B D5  |....n..r.....]+.|
0x00000040: FB 5A 2D E3 44 74 55 04 BE 7A F7 E0 4E 88 80 6D  |.Z-.DtU..z..N..m|
0x00000050: 23 E0 DD 04 03 2C 03 74 2A E3 C4 00 00           |#....,.t*....|
```

As you can see here lies a problem with our manual section parsing. We need to decode all the complicated sections like `COMPRESS/GUIDED/FV_IMAGE`. It is possible, but if you just want to read specific section like `UI` in the center of our file there is a better way.

# `FfsFile <FileGUID> <SectionType> <SectionInstance>`

Now the final part - reading file sections. For this part we would use `ReadSection` API from the protocol:
```
EFI_FIRMWARE_VOLUME2_PROTOCOL.ReadSection()

Summary:
Locates the requested section within a file and returns it in a buffer.

Prototype:
typedef 
EFI_STATUS
(EFIAPI * EFI_FV_READ_SECTION) (
 IN CONST EFI_FIRMWARE_VOLUME2_PROTOCOL *This,
 IN CONST EFI_GUID *NameGuid,
 IN EFI_SECTION_TYPE SectionType,
 IN UINTN SectionInstance,
 IN OUT VOID **Buffer,
 IN OUT UINTN *BufferSize,
 OUT UINT32 *AuthenticationStatus
 );

Parameters:
This			Indicates the EFI_FIRMWARE_VOLUME2_PROTOCOL instance
NameGuid		Pointer to an EFI_GUID, which indicates the file name from which the requested section will be read
SectionType		Indicates the section type to return. SectionType in conjunction with SectionInstance indicates which section to return
SectionInstance		Indicates which instance of sections with a type of SectionType to return. SectionInstance is zero based
Buffer			Pointer to a pointer to a buffer in which the section contents are returned, not including the section header the Buffer parameter
BufferSize		Pointer to a caller-allocated UINTN. It indicates the size of the memory represented by *Buffer
AuthenticationStatus	Pointer to a caller-allocated UINT32 in which the authentication status is returned

Description:
ReadSection() is used to retrieve a specific section from a file within a firmware volume. The section returned is determined using a depth-first, left-to-right search algorithm through all sections found in the specified file.
```

The usage is very simple:
```cpp
EFI_STATUS
ReadSection(
  IN EFI_FIRMWARE_VOLUME2_PROTOCOL* FV2Protocol,
  IN EFI_GUID* FileGuid,
  IN EFI_SECTION_TYPE SectionType,
  IN UINTN SectionInstance
  )
{
  EFI_STATUS Status;
  UINT8* Buffer = NULL;
  UINTN BufferSize;
  UINT32 AuthenticationStatus;
  Print(L"Section %s %d\n", SectionTypeString(SectionType), SectionInstance);
  Status = FV2Protocol->ReadSection(
                          FV2Protocol,
                          FileGuid,
                          SectionType,
                          SectionInstance,
                          (VOID**)&Buffer,
                          &BufferSize,
                          &AuthenticationStatus
                        );
  if (!EFI_ERROR(Status)) {
    PrintBuffer(Buffer, BufferSize);
  } else {
    Print(L"Error! ReadSection returned error: %r\n", Status);
  }
  FreePool(Buffer);
  return Status;
}
```

The `ReadSection` API can easily read section inside the complicated stuctures:
```
FS0:\> FfsFile.efi 9ccae251-a1e0-4999-af44-780bacaf9a3a UI 0
Section USER_INTERFACE 0
0x00000000: 4D 00 79 00 55 00 49 00 00 00                    |M.y.U.I...|
```

If there are several sections of the same type, you just need to provide correct index:
```
FS0:\> FfsFile.efi 9ccae251-a1e0-4999-af44-780bacaf9a3a RAW 0
Section RAW 0
0x00000000: 68 65 6C 6C 6F 0A                                |hello.|
FS0:\> FfsFile.efi 9ccae251-a1e0-4999-af44-780bacaf9a3a RAW 1
Section RAW 1
0x00000000: 6D 79 0A                                         |my.|
FS0:\> FfsFile.efi 9ccae251-a1e0-4999-af44-780bacaf9a3a RAW 2
Section RAW 2
0x00000000: 77 6F 72 6C 64 21 0A                             |world!.|
```

If you use `ALL` section type it is even possible to output file data content completely:
```
FS0:\> FfsFile.efi 9ccae251-a1e0-4999-af44-780bacaf9a3a ALL 9
Section ALL 9
0x00000000: 0A 00 00 19 68 65 6C 6C 6F 0A 00 00 61 00 00 02  |....hello...a...|
0x00000010: 98 58 4E EE 14 39 59 42 9D 6E DC 7B D7 94 03 CF  |.XN..9YB.n.{....|
0x00000020: 18 00 01 00 5D 00 00 00 01 3B 00 00 00 00 00 00  |....]....;......|
0x00000030: 00 00 03 80 32 73 81 53 56 D6 22 35 CA 8A 1A 95  |....2s.SV."5....|
0x00000040: E8 C9 C4 E4 6E DF C6 72 98 8B 8E C5 C4 5D 2B D5  |....n..r.....]+.|
0x00000050: FB 5A 2D E3 44 74 55 04 BE 7A F7 E0 4E 88 80 6D  |.Z-.DtU..z..N..m|
0x00000060: 23 E0 DD 04 03 2C 03 74 2A E3 C4 00 00           |#....,.t*....|
```

