Let's print content of variables that define boot options in UEFI.

To do it, we need to use `GetVariable()` API function from the EFI Runtime Services:
```
GetVariable()

Summary:
Returns the value of a variable.

Prototype:
typedef
EFI_STATUS
GetVariable (
 IN CHAR16 *VariableName,
 IN EFI_GUID *VendorGuid,
 OUT UINT32 *Attributes OPTIONAL,
 IN OUT UINTN *DataSize,
 OUT VOID *Data OPTIONAL
 );

Parameters:
VariableName    A Null-terminated string that is the name of the vendor’s variable.
VendorGuid      A unique identifier for the vendor
Attributes      If not NULL, a pointer to the memory location to return the
                attributes bitmask for the variable.
                If not NULL, then Attributes is set on output both when
                EFI_SUCCESS and when EFI_BUFFER_TOO_SMALL is returned.
DataSize        On input, the size in bytes of the return Data buffer.
                On output the size of data returned in Data.
Data            The buffer to return the contents of the variable. May be NULL
                with a zero DataSize in order to determine the size buffer needed.

Description:
Each vendor may create and manage its own variables without the risk of name conflicts by using a
unique VendorGuid. When a variable is set its Attributes are supplied to indicate how the data variable
should be stored and maintained by the system. The attributes affect when the variable may be accessed
and volatility of the data

If the Data buffer is too small to hold the contents of the variable, the error EFI_BUFFER_TOO_SMALL is
returned and DataSize is set to the required buffer size to obtain the data.

Status Codes Returned:
EFI_SUCCESS 		The function completed successfully.
EFI_NOT_FOUND 		The variable was not found.
EFI_BUFFER_TOO_SMALL 	The DataSize is too small for the result. DataSize has been
			updated with the size needed to complete the request. If
			Attributes is not NULL, then the attributes bitmask for the
			variable has been stored to the memory location pointed-to by
			Attributes.
...

```

In previous lesson we've discovered that these options are present in our environment under GUID `EFI_GLOBAL_VARIABLE` (gEfiGlobalVariableGuid):
```
Boot0000
Boot0001
Boot0002
Boot0003
Boot0004
BootCurrent
BootOrder
```

If we look at the UEFI spec for these options description we would find:
```
Each Boot#### variable contains an EFI_LOAD_OPTION. Each Boot#### variable is the name “Boot”
appended with a unique four digit hexadecimal number. For example, Boot0001, Boot0002, Boot0A02,
etc.

...

The BootOrder variable contains an array of UINT16’s that make up an ordered list of the Boot####
options. The first element in the array is the value for the first logical boot option, the second element is
the value for the second logical boot option, etc. The BootOrder order list is used by the firmware’s
boot manager as the default boot order.

...

The BootCurrent variable is a single UINT16 that defines the Boot#### option that was selected on
the current boot.
```

Create `ShowBootVariables` for this lesson.

First let's try to get simple `UINT16 BootCurrent` option.

We would be getting variables a lot in this program, so it is best to create a function that abstracts all the necessities of the `GetVariable` API calls:
```
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
```

As with many other different UEFI APIs first `GetVariable` call gives us `EFI_BUFFER_TOO_SMALL` error, but fills `Size` variable with the size of the array that we need to allocate and pass to this function for the correct execution.
We allocate necessary size with the `AllocateZeroPool` edk2 library call, like we did it in previous lesson. After that we call `GetVariable` function for the second time, expecting `EFI_SUCCESS` status.


The main function would look like this:
```
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>

EFI_STATUS
GetNvramVariable( CHAR16   *VariableName,
                  EFI_GUID *VariableOwnerGuid,
                  VOID     **Buffer,
                  UINTN    *BufferSize)
{
  ...
}

INTN EFIAPI ShellAppMain(IN UINTN Argc, IN CHAR16 **Argv)
{
  UINTN OptionSize;
  EFI_STATUS Status;

  UINT16* BootCurrent;
  Status = GetNvramVariable(L"BootCurrent", &gEfiGlobalVariableGuid, (VOID**)&BootCurrent, &OptionSize);
  if (Status == EFI_SUCCESS) {
    Print(L"BootCurrent=%d\n", *BootCurrent);
  } else {
    Print(L"Can't get BootCurrent variable\n");
  }

  return EFI_SUCCESS;
}
```

If we execute this code under OVMF we would get:
```
FS0:\> ShowBootVariables.efi
3
```
This means that `Boot0003` is active.


Now let's get `UINT16 BootOrder[]` variable. It is an array of XXXX numbers that describe Boot option priority.
So `{0,2,4,1,3}` for example would mean this order:
```
Boot0000
Boot0002
Boot0004
Boot0001
Boot0003
```

This is the code to get and print this array:
```
UINT16* BootOrderArray;
Status = GetNvramVariable(L"BootOrder", &gEfiGlobalVariableGuid, (VOID**)&BootOrderArray, &OptionSize);
if (Status == EFI_SUCCESS) {
  for (UINTN i=0; i<(OptionSize/sizeof(UINT16)); i++) {
    Print(L"Boot%04d%s\n", BootOrderArray[i], (BootOrderArray[i] == *BootCurrent)? L"*" : L"" );
  }
} else {
  Print(L"Can't get BootOrder variable\n");
}
```

If we execute this code under OVMF we would get:
```
Boot0000
Boot0001
Boot0002
Boot0003*
Boot0004
```


Now let's print information about every `Boot####` option.

According to the UEFI the spec these options are stored in a `EFI_LOAD_OPTION` structure:
```
typedef struct _EFI_LOAD_OPTION {
 UINT32 Attributes;
 UINT16 FilePathListLength;
 // CHAR16 Description[];
 // EFI_DEVICE_PATH_PROTOCOL FilePathList[];
 // UINT8 OptionalData[];
} EFI_LOAD_OPTION;

Parameters
Attributes 		The attributes for this load option entry. All unused bits must be zero
			and are reserved by the UEFI specification for future growth.
FilePathListLength	Length in bytes of the FilePathList. OptionalData starts at
			offset sizeof(UINT32) + sizeof(UINT16) +
			StrSize(Description) + FilePathListLength of the
			EFI_LOAD_OPTION descriptor.
Description 		The user readable description for the load option. This field ends
			with a Null character.
FilePathList 		A packed array of UEFI device paths. The first element of the array is
			a device path that describes the device and location of the Image for
			this load option. The FilePathList[0] is specific to the device
			type. Other device paths may optionally exist in the FilePathList,
			but their usage is OSV specific. Each element in the array is variable
			length, and ends at the device path end structure. Because the size
			of Description is arbitrary, this data structure is not guaranteed
			to be aligned on a natural boundary. This data structure may have to
			be copied to an aligned natural boundary before it is used.
OptionalData 		The remaining bytes in the load option descriptor are a binary data
			buffer that is passed to the loaded image. If the field is zero bytes
			long, a NULL pointer is passed to the loaded image. The number of
			bytes in OptionalData can be computed by subtracting the
			starting offset of OptionalData from total size in bytes of the
			EFI_LOAD_OPTION. 
```
In edk2 it is defined in a file https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiSpec.h

Pay attention to the fact that some fields in this structure are commented:
```
// CHAR16 Description[];
// EFI_DEVICE_PATH_PROTOCOL FilePathList[];
// UINT8 OptionalData[];
```
It is because these fields are variable size arrays, so we need to calculate offsets to them dynamically.

Create a function that accepts names of a Boot variable (such as "Boot0003") and outputs all the needed information about it:
```
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
```

This code uses pointer arithmetics that we've discussed above. To get `Description` field size we simply use `StrSize(Description)` call as `Description` field always ends with Null.

To print DevicePath as a string we use `ConvertDevicePathToText` call (we've used it earlier in our `ImageInfo` application). To use it we need to add `#include <Library/DevicePathLib.h>` to our program.

Now we need to construct "BootXXXX" variables from a `BootOrder` numbers.

To do so we would use `UnicodeSPrint` function from the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PrintLib.h
```
/**
  Produces a Null-terminated Unicode string in an output buffer based on a Null-terminated
  Unicode format string and variable argument list.
  This function is similar as snprintf_s defined in C11.

  ...

  @param  StartOfBuffer   A pointer to the output buffer for the produced Null-terminated
                          Unicode string.
  @param  BufferSize      The size, in bytes, of the output buffer specified by StartOfBuffer.
  @param  FormatString    A Null-terminated Unicode format string.
  @param  ...             Variable argument list whose contents are accessed based on the
                          format string specified by FormatString.
  @return The number of Unicode characters in the produced output buffer not including the
          Null-terminator.
**/

UINTN
EFIAPI
UnicodeSPrint (
  OUT CHAR16        *StartOfBuffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR16  *FormatString,
  ...
  );
```

Just in case in the same file there is a similar function for ASCII:
```
UINTN
EFIAPI
AsciiSPrint (
  OUT CHAR8        *StartOfBuffer,
  IN  UINTN        BufferSize,
  IN  CONST CHAR8  *FormatString,
  ...
  );
```


So the code inside BootOrder loop now would look like this:
```
CHAR16 BootOptionStr[sizeof("Boot####")+1];
UnicodeSPrint(BootOptionStr, (sizeof("Boot####")+1)*sizeof(CHAR16), L"Boot%04x", BootOrderArray[i]);
Print(L"%s%s\n", BootOptionStr, (BootOrderArray[i] == *BootCurrent)? L"*" : L"" );
PrintBootOption(BootOptionStr);
Print(L"\n");
```

Don't forget to add `#include <Library/PrintLib.h>` to the start of the file.


If we compile our app and execute it under OVMF we would get:
```
FS0:\> ShowBootVariables.efi
Boot0000
UiApp
Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(462CAA21-7614-4503-836E-8AB6F4662331)

Boot0001
UEFI QEMU DVD-ROM QM00003
PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)

Boot0002
UEFI QEMU HARDDISK QM00001
PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)

Boot0003*
EFI Internal Shell
Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)

Boot0004
UEFI PXEv4 (MAC:525400123456)
PciRoot(0x0)/Pci(0x3,0x0)/MAC(525400123456,0x1)/IPv4(0.0.0.0)
```

We are definitely in the UEFI shell, so the `BootCurrent`(="*") is placed correctly.

If you want to know information about the printed GUIDs here it is:
- `FILE_GUID = 7C04A583-9E3E-4f1c-AD65-E05268D0B4D1 # gUefiShellFileGuid`
https://github.com/tianocore/edk2/blob/master/ShellPkg/Application/Shell/Shell.inf
- `FILE_GUID = 462CAA21-7614-4503-836E-8AB6F4662331` - UiApp module is driver for BDS phase
https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Application/UiApp/UiApp.inf


As for the `7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1` it is a GUID for the firmware volume in our OVMF image.
https://github.com/tianocore/edk2/blob/master/OvmfPkg/OvmfPkgX64.fdf

This *.fdf file lists what drivers/apps are placed in every volume in the image (this includes volumes for SEC, PEI or DXE stages): 
```
...

[FV.SECFV]
FvNameGuid         = 763BED0D-DE9F-48F5-81F1-3E90E1B1A015
BlockSize          = 0x1000
FvAlignment        = 16
...
INF <...>
...

[FV.PEIFV]
FvNameGuid         = 6938079B-B503-4E3D-9D24-B28337A25806
BlockSize          = 0x10000
FvAlignment        = 16
...
INF <...>
...

[FV.DXEFV]
FvForceRebase      = FALSE
FvNameGuid         = 7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1        ### <---- this is a GUID from our output
BlockSize          = 0x10000
FvAlignment        = 16
...
INF  ShellPkg/Application/Shell/Shell.inf          <--- this apps are placed in the FV.DXEFV firmware volume
...
INF  MdeModulePkg/Application/UiApp/UiApp.inf
```

For the complete edk2 FDF file specification look at the https://edk2-docs.gitbook.io/edk-ii-fdf-specification/

