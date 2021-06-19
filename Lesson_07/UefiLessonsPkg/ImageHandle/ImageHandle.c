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


#define offsetof(a,b) ((INTN)(&(((a*)(0))->b)))

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

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
  	Print(L"Signature: %x %x %x %x\n", (MyProtocolInterface->Signature >>  0) & 0xff,
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
