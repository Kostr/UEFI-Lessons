#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/PciIo.h>
#include <Library/MemoryAllocationLib.h>
#include <IndustryStandard/Pci.h>
#include <Library/ShellLib.h>
#include <Library/PrintLib.h>

#include <IndustryStandard/PeImage.h>


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

EFI_STATUS FindPCIDevDescription(UINT16 VendorId,
                                 UINT16 DeviceId,
                                 CHAR16* VendorDesc,
                                 CHAR16* DeviceDesc,
                                 UINTN DescBufferSize)
{
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
  BOOLEAN Vendor_found = FALSE;
  BOOLEAN Device_found = FALSE;
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
          //\nXXXX  |<desc>|\n
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
          //\n\tXXXX  |<desc>|\n
          if ((StrEnd - StrStart) > 8) {
            if ((Buffer[StrStart+1]=='\t') &&
                (Buffer[StrStart+2]==DeviceStr[0]) &&
                (Buffer[StrStart+3]==DeviceStr[1]) &&
                (Buffer[StrStart+4]==DeviceStr[2]) &&
                (Buffer[StrStart+5]==DeviceStr[3])) {
              Buffer[StrEnd] = 0;
              UnicodeSPrintAsciiFormat(DeviceDesc, DescBufferSize, "%a", &Buffer[StrStart+1+1+4+2]);
              Device_found = TRUE;
              break;
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


EFI_STATUS PrintPCI(EFI_PCI_IO_PROTOCOL* PciIo)
{
  UINTN SegmentNumber;
  UINTN BusNumber;
  UINTN DeviceNumber;
  UINTN FunctionNumber;
  EFI_STATUS Status = PciIo->GetLocation(PciIo,
                                         &SegmentNumber,
                                         &BusNumber,
                                         &DeviceNumber,
                                         &FunctionNumber);
  if (EFI_ERROR(Status)) {
    Print(L"Error in getting PCI location: %r\n", Status);
    return Status;
  }

  PCI_DEVICE_INDEPENDENT_REGION PCIConfHdr;
  Status = PciIo->Pci.Read(PciIo,
                           EfiPciIoWidthUint8,
                           0,
                           sizeof(PCI_DEVICE_INDEPENDENT_REGION),
                           &PCIConfHdr);

  if (EFI_ERROR(Status)) {
    Print(L"Error in reading PCI conf space: %r\n", Status);
    return Status;
  }

  if (PciIo->RomSize) {

  Print(L"%02x:%02x.%02x - Vendor:%04x, Device:%04x", BusNumber,
                                                        DeviceNumber,
                                                        FunctionNumber,
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
    Print(L"Has OptionROM at memory %p-%p\n", PciIo->RomImage, (UINT8*)PciIo->RomImage + PciIo->RomSize);
    PCI_EXPANSION_ROM_HEADER* RomHeader = (PCI_EXPANSION_ROM_HEADER*) PciIo->RomImage;
    UINTN RomImageIndex = 1;
    while (TRUE)
   {
    if (RomHeader->Signature != PCI_EXPANSION_ROM_HEADER_SIGNATURE) {
      Print(L"Error! OptionROM has a wrong signature\n");
      return Status;
    }
    PCI_DATA_STRUCTURE* RomImage = (PCI_DATA_STRUCTURE*)((UINT8*)RomHeader + RomHeader->PcirOffset);
    if ((((CHAR8)((RomImage->Signature >>  0) & 0xFF)) != 'P') &&
        (((CHAR8)((RomImage->Signature >>  8) & 0xFF)) != 'C') &&
        (((CHAR8)((RomImage->Signature >> 16) & 0xFF)) != 'I') &&
        (((CHAR8)((RomImage->Signature >> 24) & 0xFF)) != 'R')) {
      Print(L"Error! OptionROM image has wrong signature\n");
      return Status;
    }
    Print(L"---Code Image %d---\n", RomImageIndex);
    Print(L"Address: %p-%p\n", RomHeader, (UINT8*)RomHeader + (RomImage->ImageLength)*512);
    Print(L"VendorId: %04x, DeviceId: %04x\n", RomImage->VendorId, RomImage->DeviceId);
    Print(L"Type: ");
    switch (RomImage->CodeType) {
      case 0x00:
        Print(L"IA-32, PC-AT compatible\n");
        break;
      case 0x01:
        Print(L"Open Firmware standard for PCI\n");
        break;
      case 0x02:
        Print(L"Hewlett-Packard PA RISC\n");
        break;
      case 0x03:
        Print(L"EFI Image\n");
        break;
      default:
        Print(L"Unknown\n");
        break;
    }
    if (RomImage->CodeType == 0x03) {
      EFI_PCI_EXPANSION_ROM_HEADER* EfiRomHeader =  (EFI_PCI_EXPANSION_ROM_HEADER*) RomHeader;
     if (EfiRomHeader->EfiSignature == EFI_PCI_EXPANSION_ROM_HEADER_EFISIGNATURE) {
      Print(L"Subsystem: ");
      switch (EfiRomHeader->EfiSubsystem) {
        case EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION:
          Print(L"EFI Application\n");
          break;
        case EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
          Print(L"EFI Boot Service Driver\n");
          break;
        case EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
          Print(L"EFI Runtime Driver\n");
          break;
        case EFI_IMAGE_SUBSYSTEM_SAL_RUNTIME_DRIVER:
          Print(L"EFI SAL Runtime Driver\n");
          break;
        default:
          Print(L"Unknown\n");
          break;
      }
      Print(L"Machine type: ");
      switch (EfiRomHeader->EfiMachineType) {
        case IMAGE_FILE_MACHINE_I386:
          Print(L"IA-32\n");
          break;
        case IMAGE_FILE_MACHINE_IA64:
          Print(L"Itanium\n");
          break;
        case IMAGE_FILE_MACHINE_EBC:
          Print(L"EFI Byte Code (EBC)\n");
          break;
        case IMAGE_FILE_MACHINE_X64:
          Print(L"X64\n");
          break;
        case IMAGE_FILE_MACHINE_ARMTHUMB_MIXED:
          Print(L"ARM\n");
          break;
        case IMAGE_FILE_MACHINE_ARM64:
          Print(L"ARM 64-bit\n");
          break;
        case IMAGE_FILE_MACHINE_RISCV32:
          Print(L"RISCV32\n");
          break;
        case IMAGE_FILE_MACHINE_RISCV64:
          Print(L"RISCV64\n");
          break;
        case IMAGE_FILE_MACHINE_RISCV128:
          Print(L"RISCV128\n");
          break;
        default:
          Print(L"Unknown\n");
          break;
      }
      switch (EfiRomHeader->CompressionType) {
        case EFI_PCI_EXPANSION_ROM_HEADER_COMPRESSED:
          Print(L"Compressed following the UEFI Compression Algorithm\n");
          break;
        case 0:
          Print(L"Uncompressed\n");
          break;
        default:
          Print(L"Unknown compression type\n");
          break;
      }
     } else {
      Print(L"EFI signature is incorrect!\n");
     }
    }
    if ((RomImage->Indicator) & 0x80) {
      break;
    }
    RomHeader = (PCI_EXPANSION_ROM_HEADER*)((UINT8*) RomHeader + (RomImage->ImageLength)*512);
    RomImageIndex++;
   }
   Print(L"------------------\n");
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
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                );
  if (EFI_ERROR (Status)) {
    Print(L"Can't locate EFI_PCI_IO_PROTOCOL: %r\n", Status);
    return Status;
  }

  //Print(L"Number of PCI devices in the system: %d\n", HandleCount);
  EFI_PCI_IO_PROTOCOL* PciIo;
  for (UINTN Index = 0; Index < HandleCount; Index++) {
    Status = gBS->OpenProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo,
                    ImageHandle,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (EFI_ERROR(Status)) {
      Print(L"Can't open protocol: %r\n", Status);
      return Status;
    }
    Status = PrintPCI(PciIo);
    if (EFI_ERROR(Status)) {
      Print(L"Error in PCI printing\n");
    }
    
  }
  FreePool(HandleBuffer);

  return EFI_SUCCESS;
}
