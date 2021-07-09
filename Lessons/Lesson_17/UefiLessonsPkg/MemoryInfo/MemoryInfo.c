#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

// for SetMem
#include <Library/BaseMemoryLib.h>

#include <Protocol/ShellParameters.h> 

const CHAR16 *memory_types[] = { 
    L"EfiReservedMemoryType", 
    L"EfiLoaderCode", 
    L"EfiLoaderData", 
    L"EfiBootServicesCode", 
    L"EfiBootServicesData", 
    L"EfiRuntimeServicesCode", 
    L"EfiRuntimeServicesData", 
    L"EfiConventionalMemory", 
    L"EfiUnusableMemory", 
    L"EfiACPIReclaimMemory", 
    L"EfiACPIMemoryNVS", 
    L"EfiMemoryMappedIO", 
    L"EfiMemoryMappedIOPortSpace", 
    L"EfiPalCode",
    L"EfiPersistentMemory",
    L"EfiMaxMemoryType"
};


const CHAR16 *memory_types_OS_view[] = { 
    L"reserved", // L"EfiReservedMemoryType", 
    L"usable",   // L"EfiLoaderCode", 
    L"usable",   // L"EfiLoaderData", 
    L"usable",   // L"EfiBootServicesCode", 
    L"usable",   // L"EfiBootServicesData", 
    L"reserved", // L"EfiRuntimeServicesCode", 
    L"reserved", // L"EfiRuntimeServicesData", 
    L"usable",   // L"EfiConventionalMemory", 
    L"reserved", // L"EfiUnusableMemory", 
    L"ACPI data",// L"EfiACPIReclaimMemory", 
    L"ACPI NVS", // L"EfiACPIMemoryNVS", 
    L"reserved", // L"EfiMemoryMappedIO", 
    L"reserved", // L"EfiMemoryMappedIOPortSpace", 
    L"reserved", // L"EfiPalCode",
    L"usable",   // L"EfiPersistentMemory",
    L"usable",   // L"EfiMaxMemoryType"
};


const CHAR16 * 
memory_type_to_str(UINT32 type) 
{ 
    if (type > sizeof(memory_types)/sizeof(CHAR16 *)) 
        return L"Unknown"; 

    return memory_types[type]; 
}

const CHAR16 *
memory_type_to_str_OS_view(UINT32 type)
{
    if (type > sizeof(memory_types_OS_view)/sizeof(CHAR16 *)) 
        return L"Unknown"; 

    return memory_types_OS_view[type]; 
}

#define ATTRIBUTE_STR_SIZE 50

#define CHECK_EFI_MEMORY_ATTRIBUTE(attr) if (attrs & EFI_MEMORY_##attr) { \
                                           StrCpyS(&str[i], ATTRIBUTE_STR_SIZE, L" "#attr); \
                                           i+=StrLen(L" "#attr); \
                                         }

const CHAR16 *
memory_attrs_to_str(CHAR16* str, UINT64 attrs)
{
  int i=0;
  SetMem((VOID *)str, sizeof(str), 0);

  CHECK_EFI_MEMORY_ATTRIBUTE(UC)
  CHECK_EFI_MEMORY_ATTRIBUTE(WC)
  CHECK_EFI_MEMORY_ATTRIBUTE(WT)
  CHECK_EFI_MEMORY_ATTRIBUTE(WB)
  CHECK_EFI_MEMORY_ATTRIBUTE(UCE)
  CHECK_EFI_MEMORY_ATTRIBUTE(WP)
  CHECK_EFI_MEMORY_ATTRIBUTE(RP)
  CHECK_EFI_MEMORY_ATTRIBUTE(XP)
  CHECK_EFI_MEMORY_ATTRIBUTE(NV)
  CHECK_EFI_MEMORY_ATTRIBUTE(MORE_RELIABLE)
  CHECK_EFI_MEMORY_ATTRIBUTE(RO)
  CHECK_EFI_MEMORY_ATTRIBUTE(SP)
  CHECK_EFI_MEMORY_ATTRIBUTE(CPU_CRYPTO)
  CHECK_EFI_MEMORY_ATTRIBUTE(RUNTIME)

  return str;
}

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN MemoryMapSize = 0;
  EFI_MEMORY_DESCRIPTOR* MemoryMap = NULL;
  UINTN MapKey;
  UINTN DescriptorSize;
  UINT32 DescriptorVersion;

  EFI_STATUS Status;


  EFI_SHELL_PARAMETERS_PROTOCOL* ShellParameters;

  Status = gBS->HandleProtocol(
    ImageHandle,
    &gEfiShellParametersProtocolGuid,
    (VOID **) &ShellParameters
  );

  BOOLEAN full=FALSE;
  if (Status == EFI_SUCCESS) {
    if (ShellParameters->Argc == 2) {
      if (!StrCmp(ShellParameters->Argv[1], L"full")) {
        full=TRUE;
      }
    }
  }


  Status = gBS->GetMemoryMap(
        &MemoryMapSize,
        MemoryMap,
        &MapKey,
        &DescriptorSize,
        &DescriptorVersion
  );

  if (Status == EFI_BUFFER_TOO_SMALL) {
    Status = gBS->AllocatePool(
          EfiBootServicesData,
          MemoryMapSize,
          (void**)&MemoryMap
    );

    if (EFI_ERROR(Status)) {
      Print(L"AllocatePool error: %r\n", Status);
      return Status;
    }

    Status = gBS->GetMemoryMap(
          &MemoryMapSize,
          MemoryMap,
          &MapKey,
          &DescriptorSize,
          &DescriptorVersion
    );

    if (!EFI_ERROR(Status))
    {
      EFI_MEMORY_DESCRIPTOR* desc = MemoryMap;
      EFI_MEMORY_DESCRIPTOR* next_desc;
      int i = 0;
      while ((UINT8 *)desc < (UINT8 *)MemoryMap + MemoryMapSize) {
        UINTN PAGE_SIZE = 4096;
        UINTN mapping_size =(UINTN) desc->NumberOfPages * PAGE_SIZE;

        UINT64 Start = desc->PhysicalStart;

        next_desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)desc + DescriptorSize);
        if (!full) {
	  while ((UINT8 *)next_desc < (UINT8 *)MemoryMap + MemoryMapSize) {
	    mapping_size =(UINTN) desc->NumberOfPages * PAGE_SIZE;
	    if ((desc->PhysicalStart + mapping_size) == (next_desc->PhysicalStart)) {

              if (desc->Type != next_desc->Type) {
                if (StrCmp(memory_type_to_str_OS_view(desc->Type), 
                           memory_type_to_str_OS_view(next_desc->Type)))
                  break;
                }

                desc=next_desc;
	        next_desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)next_desc + DescriptorSize);
	    } else {
	        break;
	    }
	  }
        }

        if (full) {
          CHAR16 str[ATTRIBUTE_STR_SIZE];
          Print(L"[#%02d] Type: %s  Attr: %s\n", i++, 
            memory_type_to_str(desc->Type), memory_attrs_to_str(str, desc->Attribute));
          Print(L"      Phys: %016llx-%016llx\n", Start, Start + mapping_size - 1);
        }
        else {
          Print(L" [mem: %016llx-%016llx] %s\n", Start, desc->PhysicalStart + mapping_size - 1,
            memory_type_to_str_OS_view(desc->Type) );
        }

        desc = next_desc;
      }

      gBS->FreePool(MemoryMap);
    } else {
      Print(L"GetMemoryMap with buffer error: %r\n", Status);
    }
  } else {
    Print(L"GetMemoryMap without buffer error: %r\n", Status);
  }

  return Status;
}

