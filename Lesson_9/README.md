The methods that we've used to discover protocols for our image handle were purely educational.

Let's try UEFI API functions to get the same result.

To do this we will need to understand `ProtocolsPerHandle` function:
```
EFI_BOOT_SERVICES.ProtocolsPerHandle()

Summary:
Retrieves the list of protocol interface GUIDs that are installed on a handle in a buffer allocated from pool.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_PROTOCOLS_PER_HANDLE) (
 IN EFI_HANDLE Handle,
 OUT EFI_GUID ***ProtocolBuffer,
 OUT UINTN *ProtocolBufferCount
 );

Parameters:
Handle The handle from which to retrieve the list of protocol interface GUIDs.
ProtocolBuffer A pointer to the list of protocol interface GUID pointers that are installed on Handle. This buffer is allocated with a call to the Boot Service EFI_BOOT_SERVICES.AllocatePool(). It is the caller's responsibility to call the Boot Service EFI_BOOT_SERVICES.FreePool() when the caller no longer requires the contents of ProtocolBuffer.
ProtocolBufferCountA pointer to the number of GUID pointers present in ProtocolBuffer.

Description:
The ProtocolsPerHandle() function retrieves the list of protocol interface GUIDs that are installed on Handle. The list is returned in ProtocolBuffer, and the number of GUID pointers in ProtocolBuffer is returned in ProtocolBufferCount.
If Handle is NULL or Handle is NULL, then EFI_INVALID_PARAMETER is returned.
If ProtocolBuffer is NULL, then EFI_INVALID_PAREMETER is returned.
If ProtocolBufferCount is NULL, then EFI_INVALID_PARAMETER is returned.
If there are not enough resources available to allocate ProtocolBuffer, then EFI_OUT_OF_RESOURCES is
returned.

Status Codes Returned:
EFI_SUCCESS	 	The list of protocol interface GUIDs installed on Handle was returned in
			ProtocolBuffer. The number of protocol interface GUIDs was
			returned in ProtocolBufferCount.
EFI_INVALID_PARAMETER 	Handle is NULL.
EFI_INVALID_PARAMETER 	ProtocolBuffer is NULL.
EFI_INVALID_PARAMETER 	ProtocolBufferCount is NULL.
EFI_OUT_OF_RESOURCES 	There is not enough pool memory to store the results.
```

This description mentions that we need manually deallocate received ProtocolBuffer, so lel's also look to the suggessted `FreePool` function:

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
Buffer Pointer to the buffer to free.

Description:
The FreePool() function returns the memory specified by Buffer to the system. On return, the
memory’s type is EfiConventionalMemory. The Buffer that is freed must have been allocated by
AllocatePool().

Status Codes Returned:
EFI_SUCCESS            The memory was returned to the system.
EFI_INVALID_PARAMETER  Buffer was invalid
```

Now when we know how to use API, let's add this code at the end of our `ImageHandle` app:
```
Print(L"________\n");
EFI_GUID **ProtocolGuidArray;
UINTN ArrayCount;
EFI_STATUS Status = gBS->ProtocolsPerHandle(ImageHandle,
                                            &ProtocolGuidArray,
                                            &ArrayCount);

if (Status == EFI_SUCCESS) {
  for (int i=0; i<ArrayCount; i++) {
    Print(L"%g\n", ProtocolGuidArray[i]);
  }
  FreePool(ProtocolGuidArray);
}

```

Also for the `FreePool` function we need to add one more include file https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/MemoryAllocationLib.h:
```
#include <Library/MemoryAllocationLib.h>
```

Some may have noticed, that we've used `FreePool`, but not a `gBS->FreePool`. It is legit because `FreePool` is defined in the https://github.com/tianocore/edk2/blob/master/MdePkg/Library/UefiMemoryAllocationLib/MemoryAllocationLib.c file:
```
/**
  Frees a buffer that was previously allocated with one of the pool allocation functions in the
  Memory Allocation Library.

  Frees the buffer specified by Buffer.  Buffer must have been allocated on a previous call to the
  pool allocation services of the Memory Allocation Library.  If it is not possible to free pool
  resources, then this function will perform no actions.

  If Buffer was not allocated with a pool allocation function in the Memory Allocation Library,
  then ASSERT().

  @param  Buffer                The pointer to the buffer to free.

**/
VOID
EFIAPI
FreePool (
  IN VOID   *Buffer
  )
{
  EFI_STATUS    Status;

  Status = gBS->FreePool (Buffer);
  ASSERT_EFI_ERROR (Status);
}
```
This is shortcut is neccessary as `FreePool` have different library implementations:
```
MdePkg/Library/PeiMemoryAllocationLib/MemoryAllocationLib.c
MdePkg/Library/SmmMemoryAllocationLib/MemoryAllocationLib.c
MdePkg/Library/UefiMemoryAllocationLib/MemoryAllocationLib.c
```
This is the example of LibraryClass that can have several possible instances. In our case we use `UefiMemoryAllocationLib` instance.

UefiLessonsPkg/UefiLessonsPkg.dsc:
```
[LibraryClasses]
  ...
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ...
```

Now if we build and execute our app under OVMF this code will give us:
```
752F3136-4E16-4FDC-A22A-E5F46812F4CA
BC62157E-3E33-4FEC-9920-2D3B36D750DF
5B1B31A1-9562-11D2-8E3F-00A0C969723B
```
Which are the same GUIDs we've discovered through hardcore raw walkthrough through the handle/protocol databases.



