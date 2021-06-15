Let's create `ImageInfo` app in our `UefiLessonsPkg`.

First generate a random GUID.
```
$ uuidgen
b68d3472-70c7-4928-841b-6566032e0a23
```

And then create *.inf and *.c file based on the code in our `HelloWorld` app:
```
$ cat UefiLessonsPkg/ImageInfo/ImageInfo.inf
[Defines]
  INF_VERSION                    = 1.25
  BASE_NAME                      = ImageInfo
  FILE_GUID                      = b68d3472-70c7-4928-841b-6566032e0a23
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = UefiMain

[Sources]
  ImageInfo.c

[Packages]
  MdePkg/MdePkg.dec

[LibraryClasses]
  UefiApplicationEntryPoint
  UefiLib
```
```
$ cat UefiLessonsPkg/ImageInfo/ImageInfo.c
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}
```
Add a newly created app to our `UefiLessonsPkg/UefiLessonsPkg.dsc` file:
```
[Components]
  ...
+ UefiLessonsPkg/ImageInfo/ImageInfo.inf
```

Now let's get to work.

Our main function in a *.c file has 2 input parameters `EFI_HANDLE ImageHandle` and `EFI_SYSTEM_TABLE *SystemTable`:
```
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
```
We've already started to explore the `EFI_SYSTEM_TABLE`. In this lesson let's look what `ImageHandle` is about.

`ImageHandle` parameter is a handle of an image for the program itself.

You can found a definition for a `EFI_HANDLE` type in a https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiBaseType.h
```
///
/// A collection of related interfaces.
///
typedef VOID                      *EFI_HANDLE;
```
So it is simply a pointer to a void. So it could be a pointer to anything.

In a reality it is a pointer to this type of structure https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Core/Dxe/Hand/Handle.h :
```
#define EFI_HANDLE_SIGNATURE            SIGNATURE_32('h','n','d','l')

///
/// IHANDLE - contains a list of protocol handles
///
typedef struct {
  UINTN               Signature;
  /// All handles list of IHANDLE
  LIST_ENTRY          AllHandles;
  /// List of PROTOCOL_INTERFACE's for this handle
  LIST_ENTRY          Protocols;
  UINTN               LocateRequest;
  /// The Handle Database Key value when this handle was last created or modified
  UINT64              Key;
} IHANDLE;
```
LIST_ENTRY is a double linked list. So with it we can traverse to any handle in the system. Sometimes it is called the Handle database.

In case you wonder how LIST_ENTRY is defined look at the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Base.h :
```
///
/// LIST_ENTRY structure definition.
///
typedef struct _LIST_ENTRY LIST_ENTRY;

///
/// _LIST_ENTRY structure definition.
///
struct _LIST_ENTRY {
  LIST_ENTRY  *ForwardLink;
  LIST_ENTRY  *BackLink;
};
```
Each handle has a list of protocols attached to it. Protocol is like an interface - a set of functions and data fields. To track the list of protocols attached to a handle this field is used:
```
   /// List of PROTOCOL_INTERFACE's for this handle
   LIST_ENTRY          Protocols;
```
It is another double linked list that points to these structures:
```
#define PROTOCOL_INTERFACE_SIGNATURE  SIGNATURE_32('p','i','f','c')

///
/// PROTOCOL_INTERFACE - each protocol installed on a handle is tracked
/// with a protocol interface structure
///
typedef struct {
  UINTN                       Signature;
  /// Link on IHANDLE.Protocols
  LIST_ENTRY                  Link;
  /// Back pointer
  IHANDLE                     *Handle;
  /// Link on PROTOCOL_ENTRY.Protocols
  LIST_ENTRY                  ByProtocol;
  /// The protocol ID
  PROTOCOL_ENTRY              *Protocol;
  /// The interface value
  VOID                        *Interface;
  /// OPEN_PROTOCOL_DATA list
  LIST_ENTRY                  OpenList;
  UINTN                       OpenListCount;

} PROTOCOL_INTERFACE;
```

Effectively PROTOCOL_INTERFACE structure is only a proxy that points to an actual PROTOCOL_ENTRY structrure which are exist in a separate protocol database.
```
#define PROTOCOL_ENTRY_SIGNATURE        SIGNATURE_32('p','r','t','e')

///
/// PROTOCOL_ENTRY - each different protocol has 1 entry in the protocol
/// database.  Each handler that supports this protocol is listed, along
/// with a list of registered notifies.
///
typedef struct {
  UINTN               Signature;
  /// Link Entry inserted to mProtocolDatabase
  LIST_ENTRY          AllEntries;
  /// ID of the protocol
  EFI_GUID            ProtocolID;
  /// All protocol interfaces
  LIST_ENTRY          Protocols;
  /// Registerd notification handlers
  LIST_ENTRY          Notify;
} PROTOCOL_ENTRY;
```
It all sounds a little bit complicated so it is better to see it in a picture:
![Handle_Protocol_databases_scheme](Handle_Protocol_databases_scheme.jpg?raw=true "Handle/Protocol databases scheme")
Important information from the picture:
- HANDLE 0 has PROTOCOL_INTERFACE 0 (->PROTOCOL_ENTRY 0) and PROTOCOL_INTERFACE 1 (->PROTOCOL_ENTRY 1)
- HANDLE 1 has PROTOCOL_INTERFACE 2 (->PROTOCOL_ENTRY 2) and PROTOCOL_INTERFACE 3 (->PROTOCOL_ENTRY 1)
- HANDLE 2 has PROTOCOL_INTERFACE 4 (->PROTOCOL_ENTRY 1)
Besides that:
- All handles are interconnected with the help of `AllHandles` field in each `IHANDLE` structure
- All protocols are interconnected with the help of `AllEntries` field in each `PROTOCOL_ENTRY` structure
Red lines:
- Each handle has a double linked list of all its PROTOCOL_INTERFACES, which internally connected to each other through the `Link` field
Green lines:
- Each protocol entry effectively knows all the users (=PROTOCOL_INTERFACES) of it.

Let's check that `EFI_HANDLE ImageHandle` effectively is the `IHANDLE` structure.

To check this we will cast `ImageHandle` to `IHANDLE` and print its signature.

```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>


typedef struct {
  UINTN               Signature;
  /// All handles list of IHANDLE
  LIST_ENTRY          AllHandles;
  /// List of PROTOCOL_INTERFACE's for this handle
  LIST_ENTRY          Protocols;
  UINTN               LocateRequest;
  /// The Handle Database Key value when this handle was last created or modified
  UINT64              Key;
} IHANDLE;

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  IHANDLE* MyHandle = ImageHandle;
  Print(L"Signature: %c %c %c %c\n", (MyHandle->Signature >>  0) & 0xff,
                                     (MyHandle->Signature >>  8) & 0xff,
                                     (MyHandle->Signature >> 16) & 0xff,
                                     (MyHandle->Signature >> 24) & 0xff);

  return EFI_SUCCESS;
}


Don't blame me for redefinition of `IHANDLE` structure. These structures (IHANDLE/PROTOCOL_ENTRY/PROTOCOL_INTERFACE) are private and not intended to be used outside of the `MdeModulePkg`. Even https://github.com/tianocore/edk2/blob/master/StandaloneMmPkg/Core/StandaloneMmCore.h redifines IHANDLE/PROTOCOL_ENTRY/PROTOCOL_INTERFACE.

```
$ . edksetup.sh
$ build
$ cp Build/UefiLessonsPkg/RELEASE_GCC5/X64/ImageInfo.efi ~/UEFI_disk/
$ qemu-system-x86_64 -drive if=pflash,format=raw,file=Build/OvmfX64/RELEASE_GCC5/FV/OVMF.fd \
                     -drive format=raw,file=fat:rw:~/UEFI_disk \
                     -nographic \
                     -net none
```

```
UEFI Interactive Shell v2.2
EDK II
UEFI v2.70 (EDK II, 0x00010000)
Mapping table
      FS0: Alias(s):HD0a1:;BLK1:
          PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)/HD(1,MBR,0xBE1AFDFA,0x3F,0xFBFC1)
     BLK0: Alias(s):
          PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
     BLK2: Alias(s):
          PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)
Press ESC in 4 seconds to skip startup.nsh or any other key to continue.
Shell> fs0:
FS0:\> ImageInfo.efi
Signature: h n d l
```



