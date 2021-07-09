Let's print GUID for all the protocols that are exist in our `IMAGE_HANDLE`.

First understand what `EFI_GUID` internally means in the edk2 codebase:

https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiBaseType.h
```
///
/// 128-bit buffer containing a unique identifier value.
///
typedef GUID                      EFI_GUID;
```
`GUID` structure is defined in https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Base.h :
```
///
/// 128 bit buffer containing a unique identifier value.
/// Unless otherwise specified, aligned on a 64 bit boundary.
///
typedef struct {
  UINT32  Data1;
  UINT16  Data2;
  UINT16  Data3;
  UINT8   Data4[8];
} GUID;
```
Fortunately we don't have to manually print all these fields by hand. `Print` function has a format option `%g` to print GUIDs, so we could simply print GUIDs with a code like this:
```
Print("GUID=%g\n", myGUID);
```
More information about `Print` formating options can be found at https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PrintLib.h


We want to print `EFI_GUID` field from the `PROTOCOL_ENTRY` structure which are referred from the `PROTOCOL_INTERFACE` structures. So we need to define both of these structures in our file for the same reason we've defined `IHANDLE` earlier.

```
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

`PROTOCOL_INTERFACE` structures that are connected to any HANDLE are interlinked with each other with a help of a `LIST_ENTRY Link` field that connects them to a double linked list.

As you may remember `LIST_ENTRY` is defined like this:
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
Each of these fields inside these structure points to another `LIST_ENTRY` structure that is placed in another `PROTOCOL_INTERFACE`.

So this connection looks like this:

```
typedef struct {                                         typedef struct {
  UINTN                       Signature;                   UINTN                       Signature;
  struct LIST_ENTRY {                          |---------> struct LIST_ENTRY {
    LIST_ENTRY  *ForwardLink; -----------------|             LIST_ENTRY  *ForwardLink;
    LIST_ENTRY  *BackLink;                                   LIST_ENTRY  *BackLink;
  } Link;                                                  } Link;
  IHANDLE                     *Handle;                     IHANDLE                     *Handle;
  LIST_ENTRY                  ByProtocol;                  LIST_ENTRY                  ByProtocol;
  PROTOCOL_ENTRY              *Protocol;                   PROTOCOL_ENTRY              *Protocol;
  VOID                        *Interface;                  VOID                        *Interface;
  LIST_ENTRY                  OpenList;                    LIST_ENTRY                  OpenList;
  UINTN                       OpenListCount;               UINTN                       OpenListCount;
} PROTOCOL_INTERFACE;                                    } PROTOCOL_INTERFACE;
```

But in reality we want a pointer not to `Link` field of another `PROTOCOL_INTERFACE` structure, we want a pointer to another `PROTCOL_INTERFACE` structure inself.

Therefore one more thing that we would need is a couple of macros:
```
#define offsetof(a,b) ((INTN)(&(((a*)(0))->b)))

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
```
These macros can be familiar to you if you've investigated linux kernel programming. This is where I got them from anyway.

`contianer_of` macro helps to get a pointer to a structure if you have a pointer to one of its fields.

If you want to understand how it works internally in C language I suggest you to look at the https://stackoverflow.com/questions/15832301/understanding-container-of-macro-in-the-linux-kernel

With all of this information final code for our `UefiMain` function would look like this:
```
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

  Print(L"Back Protocol Interface Link: %p\n", MyHandle->Protocols.BackLink);
  Print(L"Forward Protocol Interface Link: %p\n", MyHandle->Protocols.ForwardLink);

  LIST_ENTRY *FirstLink = MyHandle->Protocols.ForwardLink;
  LIST_ENTRY *CurrentLink = FirstLink;
  do {
        PROTOCOL_INTERFACE* MyProtocolInterface = container_of(CurrentLink, PROTOCOL_INTERFACE, Link);

        Print(L"\n");
        Print(L"Current Link: %p\n", CurrentLink);
        Print(L"Signature: %c %c %c %c\n", (MyProtocolInterface->Signature >>  0) & 0xff,
                                           (MyProtocolInterface->Signature >>  8) & 0xff,
                                           (MyProtocolInterface->Signature >> 16) & 0xff,
                                           (MyProtocolInterface->Signature >> 24) & 0xff);

        Print(L"Back Link: %p\n", MyProtocolInterface->Link.BackLink);
        Print(L"Forward Link: %p\n", MyProtocolInterface->Link.ForwardLink);
        Print(L"GUID=%g\n", MyProtocolInterface->Protocol->ProtocolID);
        CurrentLink = MyProtocolInterface->Link.ForwardLink;
  } while (CurrentLink != FirstLink);

  return EFI_SUCCESS;
}
```

If we compile and run our app in OVMF (I hope at this time I don't need to repeat how to do it):
```
FS0:\> ImageHandle.efi
h n d l
Back Protocol Interface Link: 68D4320
Forward Protocol Interface Link: 6891520

Current Link: 6891520
p i f c
Back Link: 6891430
Forward Link: 6891B20
GUID=752F3136-4E16-4FDC-A22A-E5F46812F4CA

Current Link: 6891B20
p i f c
Back Link: 6891520
Forward Link: 68D4320
GUID=BC62157E-3E33-4FEC-9920-2D3B36D750DF

Current Link: 68D4320
p i f c
Back Link: 6891B20
Forward Link: 6891430
GUID=5B1B31A1-9562-11D2-8E3F-00A0C969723B

Current Link: 6891430
  ? ? ?
Back Link: 68D4320
Forward Link: 6891520
GUID=00000000-0000-0000-0000-000000000000
```

Let's find first GUID by executing grep on the edk2 source:
```
$ grep -i 752F3136 -r ./ --exclude-dir=Build
./MdePkg/Include/Protocol/ShellParameters.h:  0x752f3136, 0x4e16, 0x4fdc, { 0xa2, 0x2a, 0xe5, 0xf4, 0x68, 0x12, 0xf4, 0xca } \
./MdePkg/MdePkg.dec:  gEfiShellParametersProtocolGuid      = { 0x752f3136, 0x4e16, 0x4fdc, {0xa2, 0x2a, 0xe5, 0xf4, 0x68, 0x12, 0xf4, 0xca }}
```

https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/ShellParameters.h
```
#define EFI_SHELL_PARAMETERS_PROTOCOL_GUID \
  { \
  0x752f3136, 0x4e16, 0x4fdc, { 0xa2, 0x2a, 0xe5, 0xf4, 0x68, 0x12, 0xf4, 0xca } \
  }
```
You can also see in in a https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec file:
```
  ## Include/Protocol/ShellParameters.h
  gEfiShellParametersProtocolGuid      = { 0x752f3136, 0x4e16, 0x4fdc, {0xa2, 0x2a, 0xe5, 0xf4, 0x68, 0x12, 0xf4, 0xca }}
```

The next two GUIDs you can find in UEFI the specification.

`EFI_LOADED_IMAGE_DEVICE_PATH_PROTOCOL` - When installed, the Loaded Image Device Path Protocol specifies the device path that was used when a PE/COFF image was loaded through the EFI Boot Service LoadImage().

```
#define EFI_LOADED_IMAGE_DEVICE_PATH_PROTOCOL_GUID \
{0xbc62157e,0x3e33,0x4fec,\
 {0x99,0x20,0x2d,0x3b,0x36,0xd7,0x50,0xdf}}
```

`EFI_LOADED_IMAGE_PROTOCOL` - Can be used on any image handle to obtain information about the loaded image.
```
#define EFI_LOADED_IMAGE_PROTOCOL_GUID\
 {0x5B1B31A1,0x9562,0x11d2,\
 {0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B}}
```

The last `PROTOCOL_INTERFACE` structure doesn't have a valid "p i f c" signature (in my case it is 0x20 0x0E 0xED 0x06), so we don't need to look at its GUID.

