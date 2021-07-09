One of the main BIOS/UEFI task is to present OS memory map. Operating system needs to know how much RAM is available in the systems and what regions of it OS can use and what regions it can't use.


For this purpose UEFI specification defines `EFI_BOOT_SERVICES.GetMemoryMap()` function:
```
EFI_BOOT_SERVICES.GetMemoryMap()

Summary:
Returns the current memory map.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_GET_MEMORY_MAP) (
 IN OUT UINTN *MemoryMapSize,
 IN OUT EFI_MEMORY_DESCRIPTOR *MemoryMap,
 OUT UINTN *MapKey,
 OUT UINTN *DescriptorSize,
 OUT UINT32 *DescriptorVersion
 );

Parameters:
MemoryMapSize 		A pointer to the size, in bytes, of the MemoryMap buffer. On input,
			this is the size of the buffer allocated by the caller. On output, it is
			the size of the buffer returned by the firmware if the buffer was
			large enough, or the size of the buffer needed to contain the map if
			the buffer was too small.
MemoryMap		A pointer to the buffer in which firmware places the current memory
			map. The map is an array of EFI_MEMORY_DESCRIPTORs.
MapKey 			A pointer to the location in which firmware returns the key for the
			current memory map.
DescriptorSize 		A pointer to the location in which firmware returns the size, in bytes,
			of an individual EFI_MEMORY_DESCRIPTOR.
DescriptorVersion 	A pointer to the location in which firmware returns the version
			number associated with the EFI_MEMORY_DESCRIPTOR.


Status Codes Returned:
EFI_SUCCESS 		The memory map was returned in the MemoryMap buffer.
EFI_BUFFER_TOO_SMALL 	The MemoryMap buffer was too small. The current buffer size needed to
			hold the memory map is returned in MemoryMapSize.
EFI_INVALID_PARAMETER 	MemoryMapSize is NULL.
EFI_INVALID_PARAMETER 	The MemoryMap buffer is not too small and MemoryMap is NULL.
```

```
//*******************************************************
//EFI_MEMORY_DESCRIPTOR
//*******************************************************
typedef struct {
 UINT32 Type;
 EFI_PHYSICAL_ADDRESS PhysicalStart;
 EFI_VIRTUAL_ADDRESS VirtualStart;
 UINT64 NumberOfPages;
 UINT64 Attribute;
} EFI_MEMORY_DESCRIPTOR;

Type 		Type of the memory region.
		Type EFI_MEMORY_TYPE is defined in the AllocatePages()
		function description.
PhysicalStart 	Physical address of the first byte in the memory region.
		PhysicalStart must be aligned on a 4 KiB boundary, and must
		not be above 0xfffffffffffff000. Type EFI_PHYSICAL_ADDRESS is
		defined in the AllocatePages() function description.
VirtualStart 	Virtual address of the first byte in the memory region.
		VirtualStart must be aligned on a 4 KiB boundary, and must not
		be above 0xfffffffffffff000.
NumberOfPages 	Number of 4 KiB pages in the memory region.
		NumberOfPages must not be 0, and must not be any value that
		would represent a memory page with a start address, either physical
		or virtual, above 0xfffffffffffff000
Attribute 	Attributes of the memory region that describe the bit mask of
		capabilities for that memory region, and not necessarily the current
		settings for that memory region.
```

EFI_BOOT_SERVICES.AllocatePages() allocates 4KB pages according to spec



Code for our app:
```
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
      int i = 0;
      while ((UINT8 *)desc <  (UINT8 *)MemoryMap + MemoryMapSize) {
        UINTN PAGE_SIZE = 4096;
        UINTN mapping_size =(UINTN) desc->NumberOfPages * PAGE_SIZE;


        CHAR16 str[ATTRIBUTE_STR_SIZE];
        Print(L"[#%02d] Type: %s  Attr: %s\n", i++, memory_type_to_str(desc->Type), memory_attrs_to_str(str, desc->Attribute));
        Print(L"      Phys: %016llx-%016llx\n", desc->PhysicalStart, desc->PhysicalStart + mapping_size - 1);

        desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)desc + DescriptorSize);
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
```

The first call to `GetMemoryMap` function should fail with `EFI_BUFFER_TOO_SMALL` as `MemoryMapSize=0` is obviously not enough to store all the memory descriptors. But this first call will fill the same `MemoryMapSize` with an actual size that we need to allocate.

We allocate the necessary size with the `gBS->AllocatePool` call
```
EFI_BOOT_SERVICES.AllocatePool()

Summary:
Allocates pool memory

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_ALLOCATE_POOL) (
 IN EFI_MEMORY_TYPE PoolType,
 IN UINTN Size,
 OUT VOID **Buffer
 );

Parameters:
PoolType 	The type of pool to allocate. Type EFI_MEMORY_TYPE is defined in
		the EFI_BOOT_SERVICES.AllocatePages() function description.
Size 		The number of bytes to allocate from the pool.
Buffer 		A pointer to a pointer to the allocated buffer if the call succeeds;
		undefined otherwise.

Description:
The AllocatePool() function allocates a memory region of Size bytes from memory of type PoolType
and returns the address of the allocated memory in the location referenced by Buffer.

Status Codes Returned:
EFI_SUCCESS 		The requested number of bytes was allocated.
EFI_OUT_OF_RESOURCES 	The pool requested could not be allocated.
EFI_INVALID_PARAMETER 	PoolType is in the range EfiMaxMemoryType..0x6FFFFFFF.
EFI_INVALID_PARAMETER 	PoolType is EfiPersistentMemory.
EFI_INVALID_PARAMETER 	Buffer is NULL
```

As a `EFI_MEMORY_TYPE` we pass `EfiBootServicesData` which signifies the data that is connected to the BootServices and would be freed after OS starts execution.
Other possible values for `EFI_MEMORY_TYPE` can be found in a spec or here https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiMultiPhase.h


If the memory was successfully allocated we need to dealocate it with the `gBS->FreePool` call in the end:
```
EFI_BOOT_SERVICES.FreePool()

Summary:
Returns pool memory to the system.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_FREE_POOL) (
IN VOID *Buffer
);

Parameters:
Buffer 	Pointer to the buffer to free.

Description:
The FreePool() function returns the memory specified by Buffer to the system. The Buffer that is freed must have been allocated by AllocatePool().

Status Codes Returned:
EFI_SUCCESS 		The memory was returned to the system.
EFI_INVALID_PARAMETER 	Buffer was invalid. 
```

If the second call to the `GetMemoryMap` function was successful we would get array of `EFI_MEMORY_DESCRIPTOR` objects starting at `MemoryMap` address.

We walk through that array and print information in each descriptor.

To convert memory type value to a string we create `memory_type_to_str` helper function:
```
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

const CHAR16 *
memory_type_to_str(UINT32 type)
{
    if (type > sizeof(memory_types)/sizeof(CHAR16 *))
        return L"Unknown";

    return memory_types[type];
}
```

To convert memory attrributes to a string we create `memory_attrs_to_str` helper function.
I'm not proud of this macro solution, as macros generally considered error prone design, but this gives us an easy way to concatenate all possible attriburtes which are defined as `EFI_MEMORY_XXX` in a edk2 codebase.
```
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
```

In the last snippet we've used some edk2 string manipulation functions:
- StrCpyS:
https://github.com/tianocore/edk2/blob/master/MdePkg/Library/BaseLib/SafeString.c
(According to https://edk2-docs.gitbook.io/edk-ii-secure-coding-guide/secure_coding_guidelines_general StrCpy is a deprecated API and should be replaced with StrCpyS)
- StrLen: https://github.com/tianocore/edk2/blob/master/MdePkg/Library/BaseLib/String.c
This functions are similar to their analogs from the standard C library.

For these string functions we need to add one more include:
```
#include <Library/BaseMemoryLib.h>
```


If we build and execute our app under OVMF it will give us something like this:
```
FS0:\> MemoryInfo.efi
[#00] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000000000000-0000000000000FFF
[#01] Type: EfiConventionalMemory  Attr:  UC WC WT WB
      Phys: 0000000000001000-000000000009FFFF
[#02] Type: EfiConventionalMemory  Attr:  UC WC WT WB
      Phys: 0000000000100000-00000000007FFFFF
[#03] Type: EfiACPIMemoryNVS  Attr:  UC WC WT WB
      Phys: 0000000000800000-0000000000807FFF
[#04] Type: EfiConventionalMemory  Attr:  UC WC WT WB
      Phys: 0000000000808000-000000000080FFFF
[#05] Type: EfiACPIMemoryNVS  Attr:  UC WC WT WB
      Phys: 0000000000810000-00000000008FFFFF
[#06] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000000900000-00000000014FFFFF
[#07] Type: EfiConventionalMemory  Attr:  UC WC WT WB
      Phys: 0000000001500000-0000000003F35FFF
[#08] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000003F36000-0000000003F55FFF
[#09] Type: EfiConventionalMemory  Attr:  UC WC WT WB
      Phys: 0000000003F56000-000000000673AFFF
[#10] Type: EfiLoaderCode  Attr:  UC WC WT WB
      Phys: 000000000673B000-000000000680BFFF
[#11] Type: EfiConventionalMemory  Attr:  UC WC WT WB
      Phys: 000000000680C000-0000000006855FFF
[#12] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006856000-0000000006873FFF
[#13] Type: EfiConventionalMemory  Attr:  UC WC WT WB
      Phys: 0000000006874000-000000000688AFFF
[#14] Type: EfiLoaderCode  Attr:  UC WC WT WB
      Phys: 000000000688B000-000000000688DFFF
[#15] Type: EfiConventionalMemory  Attr:  UC WC WT WB
      Phys: 000000000688E000-000000000688EFFF
[#16] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 000000000688F000-0000000006EAAFFF
[#17] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006EAB000-0000000006ECCFFF
[#18] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006ECD000-0000000006ED2FFF
[#19] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006ED3000-0000000006ED9FFF
[#20] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006EDA000-0000000006EDBFFF
[#21] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006EDC000-0000000006EF3FFF
[#22] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006EF4000-0000000006EF5FFF
[#23] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006EF6000-0000000006EFDFFF
[#24] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006EFE000-0000000006EFEFFF
[#25] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006EFF000-0000000006F0CFFF
[#26] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006F0D000-0000000006F0DFFF
[#27] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006F0E000-0000000006F10FFF
[#28] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006F11000-0000000006F17FFF
[#29] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006F18000-0000000006F26FFF
[#30] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006F27000-0000000006F28FFF
[#31] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006F29000-0000000006F3CFFF
[#32] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006F3D000-0000000006F43FFF
[#33] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006F44000-0000000006F66FFF
[#34] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006F67000-0000000006F69FFF
[#35] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006F6A000-0000000006F6EFFF
[#36] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006F6F000-0000000006F70FFF
[#37] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006F71000-0000000006F80FFF
[#38] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006F81000-0000000006F82FFF
[#39] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006F83000-0000000006F85FFF
[#40] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006F86000-0000000006F87FFF
[#41] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006F88000-0000000006F93FFF
[#42] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006F94000-0000000006F95FFF
[#43] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006F96000-0000000006FA0FFF
[#44] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006FA1000-0000000006FA2FFF
[#45] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006FA3000-0000000006FA7FFF
[#46] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006FA8000-0000000006FA9FFF
[#47] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006FAA000-0000000006FB0FFF
[#48] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006FB1000-0000000006FB6FFF
[#49] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006FB7000-0000000006FBAFFF
[#50] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006FBB000-0000000006FBBFFF
[#51] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006FBC000-0000000006FE8FFF
[#52] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000006FE9000-0000000006FEAFFF
[#53] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000006FEB000-0000000006FFFFFF
[#54] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007000000-0000000007200FFF
[#55] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007201000-0000000007201FFF
[#56] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007202000-0000000007205FFF
[#57] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007206000-0000000007206FFF
[#58] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007207000-0000000007207FFF
[#59] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007208000-000000000720EFFF
[#60] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 000000000720F000-0000000007215FFF
[#61] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007216000-0000000007219FFF
[#62] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 000000000721A000-000000000721EFFF
[#63] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 000000000721F000-000000000722CFFF
[#64] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 000000000722D000-0000000007230FFF
[#65] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007231000-0000000007242FFF
[#66] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007243000-0000000007246FFF
[#67] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007247000-0000000007257FFF
[#68] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007258000-0000000007258FFF
[#69] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007259000-000000000725CFFF
[#70] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 000000000725D000-0000000007260FFF
[#71] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007261000-0000000007263FFF
[#72] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007264000-0000000007266FFF
[#73] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007267000-000000000727EFFF
[#74] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 000000000727F000-000000000727FFFF
[#75] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007280000-0000000007283FFF
[#76] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007284000-0000000007287FFF
[#77] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007288000-0000000007288FFF
[#78] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007289000-0000000007289FFF
[#79] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 000000000728A000-000000000728FFFF
[#80] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007290000-0000000007291FFF
[#81] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007292000-0000000007292FFF
[#82] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007293000-0000000007293FFF
[#83] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007294000-0000000007296FFF
[#84] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007297000-0000000007298FFF
[#85] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007299000-000000000729BFFF
[#86] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 000000000729C000-000000000729DFFF
[#87] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 000000000729E000-000000000729EFFF
[#88] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 000000000729F000-00000000072A8FFF
[#89] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 00000000072A9000-00000000072B4FFF
[#90] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 00000000072B5000-00000000072B6FFF
[#91] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 00000000072B7000-00000000072B7FFF
[#92] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 00000000072B8000-00000000072BAFFF
[#93] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 00000000072BB000-00000000072C0FFF
[#94] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 00000000072C1000-00000000076C0FFF
[#95] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 00000000076C1000-00000000076C6FFF
[#96] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 00000000076C7000-00000000076C8FFF
[#97] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 00000000076C9000-00000000076D3FFF
[#98] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 00000000076D4000-00000000076D6FFF
[#99] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 00000000076D7000-00000000076D9FFF
[#100] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 00000000076DA000-00000000078EEFFF
[#101] Type: EfiRuntimeServicesData  Attr:  UC WC WT WB RUNTIME
      Phys: 00000000078EF000-00000000079EEFFF
[#102] Type: EfiRuntimeServicesCode  Attr:  UC WC WT WB RUNTIME
      Phys: 00000000079EF000-0000000007AEEFFF
[#103] Type: EfiReservedMemoryType  Attr:  UC WC WT WB
      Phys: 0000000007AEF000-0000000007B6EFFF
[#104] Type: EfiACPIReclaimMemory  Attr:  UC WC WT WB
      Phys: 0000000007B6F000-0000000007B7EFFF
[#105] Type: EfiACPIMemoryNVS  Attr:  UC WC WT WB
      Phys: 0000000007B7F000-0000000007BFEFFF
[#106] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007BFF000-0000000007DFFFFF
[#107] Type: EfiConventionalMemory  Attr:  UC WC WT WB
      Phys: 0000000007E00000-0000000007E9CFFF
[#108] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007E9D000-0000000007EBCFFF
[#109] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007EBD000-0000000007ED6FFF
[#110] Type: EfiBootServicesData  Attr:  UC WC WT WB
      Phys: 0000000007ED7000-0000000007EDFFFF
[#111] Type: EfiBootServicesCode  Attr:  UC WC WT WB
      Phys: 0000000007EE0000-0000000007EF3FFF
[#112] Type: EfiRuntimeServicesData  Attr:  UC WC WT WB RUNTIME
      Phys: 0000000007EF4000-0000000007F77FFF
[#113] Type: EfiACPIMemoryNVS  Attr:  UC WC WT WB
      Phys: 0000000007F78000-0000000007FFFFFF
[#114] Type: EfiMemoryMappedIO  Attr:  UC RUNTIME
      Phys: 00000000FFC00000-00000000FFFFFFFF
```


You could verify our result if you execute `memmap` shell command:
```
Shell> memmap
Type       Start            End              # Pages          Attributes
BS_Code    0000000000000000-0000000000000FFF 0000000000000001 000000000000000F
Available  0000000000001000-000000000009FFFF 000000000000009F 000000000000000F
Available  0000000000100000-00000000007FFFFF 0000000000000700 000000000000000F
ACPI_NVS   0000000000800000-0000000000807FFF 0000000000000008 000000000000000F
Available  0000000000808000-000000000080FFFF 0000000000000008 000000000000000F
ACPI_NVS   0000000000810000-00000000008FFFFF 00000000000000F0 000000000000000F
BS_Data    0000000000900000-00000000014FFFFF 0000000000000C00 000000000000000F
Available  0000000001500000-0000000003F35FFF 0000000000002A36 000000000000000F
BS_Data    0000000003F36000-0000000003F55FFF 0000000000000020 000000000000000F
Available  0000000003F56000-0000000006509FFF 00000000000025B4 000000000000000F
LoaderCode 000000000650A000-00000000065DAFFF 00000000000000D1 000000000000000F
Available  00000000065DB000-0000000006624FFF 000000000000004A 000000000000000F
BS_Data    0000000006625000-0000000006642FFF 000000000000001E 000000000000000F
Available  0000000006643000-000000000665AFFF 0000000000000018 000000000000000F
BS_Data    000000000665B000-000000000666DFFF 0000000000000013 000000000000000F
Available  000000000666E000-000000000666FFFF 0000000000000002 000000000000000F
BS_Data    0000000006670000-0000000006D68FFF 00000000000006F9 000000000000000F
BS_Code    0000000006D69000-0000000006E1FFFF 00000000000000B7 000000000000000F
BS_Data    0000000006E20000-0000000006EAAFFF 000000000000008B 000000000000000F
BS_Code    0000000006EAB000-0000000006ECCFFF 0000000000000022 000000000000000F
BS_Data    0000000006ECD000-0000000006ED2FFF 0000000000000006 000000000000000F
BS_Code    0000000006ED3000-0000000006ED9FFF 0000000000000007 000000000000000F
BS_Data    0000000006EDA000-0000000006EDBFFF 0000000000000002 000000000000000F
BS_Code    0000000006EDC000-0000000006EF3FFF 0000000000000018 000000000000000F
BS_Data    0000000006EF4000-0000000006EF5FFF 0000000000000002 000000000000000F
BS_Code    0000000006EF6000-0000000006EFDFFF 0000000000000008 000000000000000F
BS_Data    0000000006EFE000-0000000006EFEFFF 0000000000000001 000000000000000F
BS_Code    0000000006EFF000-0000000006F0CFFF 000000000000000E 000000000000000F
BS_Data    0000000006F0D000-0000000006F0DFFF 0000000000000001 000000000000000F
BS_Code    0000000006F0E000-0000000006F10FFF 0000000000000003 000000000000000F
BS_Data    0000000006F11000-0000000006F17FFF 0000000000000007 000000000000000F
BS_Code    0000000006F18000-0000000006F26FFF 000000000000000F 000000000000000F
BS_Data    0000000006F27000-0000000006F28FFF 0000000000000002 000000000000000F
BS_Code    0000000006F29000-0000000006F3CFFF 0000000000000014 000000000000000F
BS_Data    0000000006F3D000-0000000006F43FFF 0000000000000007 000000000000000F
BS_Code    0000000006F44000-0000000006F66FFF 0000000000000023 000000000000000F
BS_Data    0000000006F67000-0000000006F69FFF 0000000000000003 000000000000000F
BS_Code    0000000006F6A000-0000000006F6EFFF 0000000000000005 000000000000000F
BS_Data    0000000006F6F000-0000000006F70FFF 0000000000000002 000000000000000F
BS_Code    0000000006F71000-0000000006F80FFF 0000000000000010 000000000000000F
BS_Data    0000000006F81000-0000000006F82FFF 0000000000000002 000000000000000F
BS_Code    0000000006F83000-0000000006F85FFF 0000000000000003 000000000000000F
BS_Data    0000000006F86000-0000000006F87FFF 0000000000000002 000000000000000F
BS_Code    0000000006F88000-0000000006F93FFF 000000000000000C 000000000000000F
BS_Data    0000000006F94000-0000000006F95FFF 0000000000000002 000000000000000F
BS_Code    0000000006F96000-0000000006FA0FFF 000000000000000B 000000000000000F
BS_Data    0000000006FA1000-0000000006FA2FFF 0000000000000002 000000000000000F
BS_Code    0000000006FA3000-0000000006FA7FFF 0000000000000005 000000000000000F
BS_Data    0000000006FA8000-0000000006FA9FFF 0000000000000002 000000000000000F
BS_Code    0000000006FAA000-0000000006FB0FFF 0000000000000007 000000000000000F
BS_Data    0000000006FB1000-0000000006FB6FFF 0000000000000006 000000000000000F
BS_Code    0000000006FB7000-0000000006FBAFFF 0000000000000004 000000000000000F
BS_Data    0000000006FBB000-0000000006FBBFFF 0000000000000001 000000000000000F
BS_Code    0000000006FBC000-0000000006FE8FFF 000000000000002D 000000000000000F
BS_Data    0000000006FE9000-0000000006FEAFFF 0000000000000002 000000000000000F
BS_Code    0000000006FEB000-0000000006FFFFFF 0000000000000015 000000000000000F
BS_Data    0000000007000000-0000000007200FFF 0000000000000201 000000000000000F
BS_Code    0000000007201000-0000000007201FFF 0000000000000001 000000000000000F
BS_Data    0000000007202000-0000000007205FFF 0000000000000004 000000000000000F
BS_Code    0000000007206000-0000000007206FFF 0000000000000001 000000000000000F
BS_Data    0000000007207000-0000000007207FFF 0000000000000001 000000000000000F
BS_Code    0000000007208000-000000000720EFFF 0000000000000007 000000000000000F
BS_Data    000000000720F000-0000000007215FFF 0000000000000007 000000000000000F
BS_Code    0000000007216000-0000000007219FFF 0000000000000004 000000000000000F
BS_Data    000000000721A000-000000000721EFFF 0000000000000005 000000000000000F
BS_Code    000000000721F000-000000000722CFFF 000000000000000E 000000000000000F
BS_Data    000000000722D000-0000000007230FFF 0000000000000004 000000000000000F
BS_Code    0000000007231000-0000000007242FFF 0000000000000012 000000000000000F
BS_Data    0000000007243000-0000000007246FFF 0000000000000004 000000000000000F
BS_Code    0000000007247000-0000000007257FFF 0000000000000011 000000000000000F
BS_Data    0000000007258000-0000000007258FFF 0000000000000001 000000000000000F
BS_Code    0000000007259000-000000000725CFFF 0000000000000004 000000000000000F
BS_Data    000000000725D000-0000000007260FFF 0000000000000004 000000000000000F
BS_Code    0000000007261000-0000000007263FFF 0000000000000003 000000000000000F
BS_Data    0000000007264000-0000000007266FFF 0000000000000003 000000000000000F
BS_Code    0000000007267000-000000000727EFFF 0000000000000018 000000000000000F
BS_Data    000000000727F000-000000000727FFFF 0000000000000001 000000000000000F
BS_Code    0000000007280000-0000000007283FFF 0000000000000004 000000000000000F
BS_Data    0000000007284000-0000000007287FFF 0000000000000004 000000000000000F
BS_Code    0000000007288000-0000000007288FFF 0000000000000001 000000000000000F
BS_Data    0000000007289000-0000000007289FFF 0000000000000001 000000000000000F
BS_Code    000000000728A000-000000000728FFFF 0000000000000006 000000000000000F
BS_Data    0000000007290000-0000000007291FFF 0000000000000002 000000000000000F
BS_Code    0000000007292000-0000000007292FFF 0000000000000001 000000000000000F
BS_Data    0000000007293000-0000000007293FFF 0000000000000001 000000000000000F
BS_Code    0000000007294000-0000000007296FFF 0000000000000003 000000000000000F
BS_Data    0000000007297000-0000000007298FFF 0000000000000002 000000000000000F
BS_Code    0000000007299000-000000000729BFFF 0000000000000003 000000000000000F
BS_Data    000000000729C000-000000000729DFFF 0000000000000002 000000000000000F
BS_Code    000000000729E000-000000000729EFFF 0000000000000001 000000000000000F
BS_Data    000000000729F000-00000000072A8FFF 000000000000000A 000000000000000F
BS_Code    00000000072A9000-00000000072B4FFF 000000000000000C 000000000000000F
BS_Data    00000000072B5000-00000000072B6FFF 0000000000000002 000000000000000F
BS_Code    00000000072B7000-00000000072B7FFF 0000000000000001 000000000000000F
BS_Data    00000000072B8000-00000000072BAFFF 0000000000000003 000000000000000F
BS_Code    00000000072BB000-00000000072C0FFF 0000000000000006 000000000000000F
BS_Data    00000000072C1000-00000000076C0FFF 0000000000000400 000000000000000F
BS_Code    00000000076C1000-00000000076C6FFF 0000000000000006 000000000000000F
BS_Data    00000000076C7000-00000000076C8FFF 0000000000000002 000000000000000F
BS_Code    00000000076C9000-00000000076D3FFF 000000000000000B 000000000000000F
BS_Data    00000000076D4000-00000000076D6FFF 0000000000000003 000000000000000F
BS_Code    00000000076D7000-00000000076D9FFF 0000000000000003 000000000000000F
BS_Data    00000000076DA000-00000000078EEFFF 0000000000000215 000000000000000F
RT_Data    00000000078EF000-00000000079EEFFF 0000000000000100 800000000000000F
RT_Code    00000000079EF000-0000000007AEEFFF 0000000000000100 800000000000000F
Reserved   0000000007AEF000-0000000007B6EFFF 0000000000000080 000000000000000F
ACPI_Recl  0000000007B6F000-0000000007B7EFFF 0000000000000010 000000000000000F
ACPI_NVS   0000000007B7F000-0000000007BFEFFF 0000000000000080 000000000000000F
BS_Data    0000000007BFF000-0000000007DFFFFF 0000000000000201 000000000000000F
Available  0000000007E00000-0000000007E9CFFF 000000000000009D 000000000000000F
BS_Data    0000000007E9D000-0000000007EBCFFF 0000000000000020 000000000000000F
BS_Code    0000000007EBD000-0000000007ED6FFF 000000000000001A 000000000000000F
BS_Data    0000000007ED7000-0000000007EDFFFF 0000000000000009 000000000000000F
BS_Code    0000000007EE0000-0000000007EF3FFF 0000000000000014 000000000000000F
RT_Data    0000000007EF4000-0000000007F77FFF 0000000000000084 800000000000000F
ACPI_NVS   0000000007F78000-0000000007FFFFFF 0000000000000088 000000000000000F
MMIO       00000000FFC00000-00000000FFFFFFFF 0000000000000400 8000000000000001

  Reserved  :            128 Pages (524,288 Bytes)
  LoaderCode:            209 Pages (856,064 Bytes)
  LoaderData:              0 Pages (0 Bytes)
  BS_Code   :            670 Pages (2,744,320 Bytes)
  BS_Data   :          7,819 Pages (32,026,624 Bytes)
  RT_Code   :            256 Pages (1,048,576 Bytes)
  RT_Data   :            388 Pages (1,589,248 Bytes)
  ACPI_Recl :             16 Pages (65,536 Bytes)
  ACPI_NVS  :            512 Pages (2,097,152 Bytes)
  MMIO      :          1,024 Pages (4,194,304 Bytes)
  MMIO_Port :              0 Pages (0 Bytes)
  PalCode   :              0 Pages (0 Bytes)
  Available :         22,674 Pages (92,872,704 Bytes)
  Persistent:              0 Pages (0 Bytes)
              --------------
Total Memory:            127 MB (133,300,224 Bytes)
```

As you can see, the regions are the same that we've got with our program.
