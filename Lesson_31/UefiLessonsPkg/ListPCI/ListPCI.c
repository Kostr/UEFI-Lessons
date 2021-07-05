#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/PciRootBridgeIo.h>
#include <Library/MemoryAllocationLib.h>
#include <IndustryStandard/Pci.h>
#include <Library/ShellLib.h>
#include <Library/PrintLib.h>


#define DESCRIPTOR_STR_MAX_SIZE 200
#define BLOCK_READ_SIZE (1024*4)

VOID ToLowerASCII(CHAR8* Str, UINTN Size)
{
  for (UINT8 i=0; i<Size; i++) {
    if ((Str[i]>='A')&&(Str[i]<='Z')) {
      Str[i]+=32;
    }
  }
}

EFI_STATUS FindPCIDevDescription(IN UINT16 VendorId,
                                 IN UINT16 DeviceId,
                                 OUT CHAR16* VendorDesc,
                                 OUT CHAR16* DeviceDesc,
                                 IN UINTN DescBufferSize)
{
  BOOLEAN Vendor_found = FALSE;
  BOOLEAN Device_found = FALSE;

  EFI_STATUS Status = ShellFileExists(L"pci.ids");
  if (EFI_ERROR(Status))
  {
    Print(L"No file pci.ids: %r\n", Status);
    return Status;
  }
  
  SHELL_FILE_HANDLE FileHandle;
  Status = ShellOpenFileByName(L"pci.ids",
                               &FileHandle,
                               EFI_FILE_MODE_READ,
                               0);
  if (EFI_ERROR(Status))
  {
    Print(L"Can't open file pci.ids: %r\n", Status);
    return Status;
  }

  UINT64 FileSize;
  Status = ShellGetFileSize(FileHandle, &FileSize);
  if (EFI_ERROR(Status))
  {
    Print(L"Can't get file size for file pci.ids: %r\n", Status);
    goto end;
  }

  CHAR8 VendorStr[5];
  CHAR8 DeviceStr[5];
  AsciiValueToStringS(VendorStr,
                      5,
                      RADIX_HEX | PREFIX_ZERO,
                      VendorId,
                      4);
  AsciiValueToStringS(DeviceStr,
                      5,
                      RADIX_HEX | PREFIX_ZERO,
                      DeviceId,
                      4);
  ToLowerASCII(VendorStr, 4);
  ToLowerASCII(DeviceStr, 4);

  CHAR8 Buffer[BLOCK_READ_SIZE];
  UINTN Size;
  UINT64 FilePos = 0;
  while (TRUE)
  {
    Size = BLOCK_READ_SIZE;
    Status = ShellReadFile(FileHandle, &Size, Buffer);
    if (EFI_ERROR(Status))
    {
      Print(L"Can't read file pci.ids: %r\n", Status);
      goto end;
    }
    UINTN StrStart = 0;
    UINTN StrEnd = 0;
    for (UINTN i=0; i<Size; i++) {
      if (Buffer[i]=='\n') {
        StrEnd=i;
        if (!Vendor_found){
          // 0123456         7
          //\nVVVV  |<desc>|\n
          if ((StrEnd - StrStart) > 7) {
            if ((Buffer[StrStart+1]==VendorStr[0]) &&
                (Buffer[StrStart+2]==VendorStr[1]) &&
                (Buffer[StrStart+3]==VendorStr[2]) &&
                (Buffer[StrStart+4]==VendorStr[3])) {
              Buffer[StrEnd] = 0;
              UnicodeSPrintAsciiFormat(VendorDesc, DescBufferSize, "%a", &Buffer[StrStart+1+4+2]);
              Vendor_found = TRUE;
            }
          }
        } else {
          // 0 1234567         8
          //\n\tDDDD  |<desc>|\n
          if ((StrEnd - StrStart) > 8) {
            if ((Buffer[StrStart+1]=='\t') &&
                (Buffer[StrStart+2]==DeviceStr[0]) &&
                (Buffer[StrStart+3]==DeviceStr[1]) &&
                (Buffer[StrStart+4]==DeviceStr[2]) &&
                (Buffer[StrStart+5]==DeviceStr[3])) {
              Buffer[StrEnd] = 0;
              UnicodeSPrintAsciiFormat(DeviceDesc, DescBufferSize, "%a", &Buffer[StrStart+1+1+4+2]);
              Device_found = TRUE;
              goto end;
            }
          }
        }
        StrStart = StrEnd;
      }
    }

    if (FilePos+Size >= FileSize) {
      break;
    }
    FilePos += StrEnd;
    Status = ShellSetFilePosition(FileHandle, FilePos);
    if (EFI_ERROR(Status))
    {
      Print(L"Can't set file position pci.ids: %r\n", Status);
      goto end;
    }
  }

end:
  if (!Vendor_found) {
    UnicodeSPrint(VendorDesc, DescBufferSize, L"Undefined");
  }
  if (!Device_found) {
    UnicodeSPrint(DeviceDesc, DescBufferSize, L"Undefined");
  }
  ShellCloseFile(&FileHandle);

  return Status;
}


UINT64 PciConfigurationAddress(UINT8 Bus,
                               UINT8 Device,
                               UINT8 Function,
                               UINT32 Register)
{
  UINT64 Address = (((UINT64)Bus) << 24) + (((UINT64)Device) << 16) + (((UINT64)Function) << 8);
  if (Register & 0xFFFFFF00) {
    Address += (((UINT64)Register) << 32);
  } else {
    Address += (((UINT64)Register) << 0);
  }
  return Address;
}


EFI_STATUS PrintRootBridge(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL* PciRootBridgeIo)
{
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR* AddressDescriptor;
  EFI_STATUS Status = PciRootBridgeIo->Configuration(
                                         PciRootBridgeIo,
                                         (VOID**)&AddressDescriptor
                                       );
  if (EFI_ERROR(Status)) {
    Print(L"\tError! Can't get EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR: %r\n", Status);
    return Status;
  }
  while (AddressDescriptor->Desc != ACPI_END_TAG_DESCRIPTOR) {
    if (AddressDescriptor->ResType == ACPI_ADDRESS_SPACE_TYPE_BUS) {
      for (UINT8 Bus = AddressDescriptor->AddrRangeMin; Bus <= AddressDescriptor->AddrRangeMax; Bus++) {
        for (UINT8 Device = 0; Device <= PCI_MAX_DEVICE; Device++) {
          for (UINT8 Func = 0; Func <= PCI_MAX_FUNC; Func++) {
            UINT64 Address = PciConfigurationAddress(Bus, Device, Func, 0);
            PCI_DEVICE_INDEPENDENT_REGION PCIConfHdr;
            Status = PciRootBridgeIo->Pci.Read(
              PciRootBridgeIo,
              EfiPciWidthUint8,
              Address,
              sizeof(PCI_DEVICE_INDEPENDENT_REGION),
              &PCIConfHdr
            );
            if (!EFI_ERROR(Status)) {
              if (PCIConfHdr.VendorId != 0xffff) {
                Print(L"  %02x:%02x.%02x - Vendor:%04x, Device:%04x",
                                                                        Bus,
                                                                        Device,
                                                                        Func,
                                                                        PCIConfHdr.VendorId, 
                                                                        PCIConfHdr.DeviceId);

                CHAR16 VendorDesc[DESCRIPTOR_STR_MAX_SIZE];
                CHAR16 DeviceDesc[DESCRIPTOR_STR_MAX_SIZE];
                Status = FindPCIDevDescription(PCIConfHdr.VendorId,
                                               PCIConfHdr.DeviceId,
                                               VendorDesc,
                                               DeviceDesc,
                                               DESCRIPTOR_STR_MAX_SIZE);
                if (!EFI_ERROR(Status)) {
                  Print(L":    %s, %s\n", VendorDesc, DeviceDesc);
                } else {
                  Print(L"\n");
                }
              }
            } else {
              Print(L"  Error in PCI read: %r\n", Status);
            }
          }
        }
      }
    }
    AddressDescriptor++;
  }
  return Status;
}

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS             Status;
  UINTN                  HandleCount;
  EFI_HANDLE             *HandleBuffer;
  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiPciRootBridgeIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                );
  if (EFI_ERROR (Status)) {
    Print(L"Can't locate EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL: %r\n", Status);
    return Status;
  }

  Print(L"Number of PCI root bridges in the system: %d\n", HandleCount);
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL* PciRootBridgeIo;
  for (UINTN Index = 0; Index < HandleCount; Index++) {
    Status = gBS->OpenProtocol (
                    HandleBuffer[Index],
                    &gEfiPciRootBridgeIoProtocolGuid,
                    (VOID **)&PciRootBridgeIo,
                    ImageHandle,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (EFI_ERROR(Status)) {
      Print(L"Can't open protocol: %r\n", Status);
      return Status;
    }
    Print(L"\nPCI Root Bridge %d\n", Index);
    Status = PrintRootBridge(PciRootBridgeIo);
    if (EFI_ERROR(Status)) {
      Print(L"Error in PCI Root Bridge printing\n");
    }
  }
  FreePool(HandleBuffer);

  return EFI_SUCCESS;
}
