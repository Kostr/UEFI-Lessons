In this lesson we would explore some internals behind the HII database.

First of all let's look at the main structure of the HII database:

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabase.h
```
typedef struct _HII_DATABASE_PRIVATE_DATA {
  UINTN                                 Signature;
  LIST_ENTRY                            DatabaseList;
  LIST_ENTRY                            DatabaseNotifyList;
  EFI_HII_FONT_PROTOCOL                 HiiFont;
  EFI_HII_IMAGE_PROTOCOL                HiiImage;
  EFI_HII_IMAGE_EX_PROTOCOL             HiiImageEx;
  EFI_HII_STRING_PROTOCOL               HiiString;
  EFI_HII_DATABASE_PROTOCOL             HiiDatabase;
  EFI_HII_CONFIG_ROUTING_PROTOCOL       ConfigRouting;
  EFI_CONFIG_KEYWORD_HANDLER_PROTOCOL   ConfigKeywordHandler;
  LIST_ENTRY                            HiiHandleList;
  INTN                                  HiiHandleCount;
  LIST_ENTRY                            FontInfoList;
  UINTN                                 Attribute;
  EFI_GUID                              CurrentLayoutGuid;
  EFI_HII_KEYBOARD_LAYOUT               *CurrentLayout;
} HII_DATABASE_PRIVATE_DATA;
```

This structure contains pointers to the main HII protocols. Each of these protocols is responsible for interactons with different parts of HII. For example one is responsible for interaction with fonts in the HII database (`EFI_HII_FONT_PROTOCOL`), another one for interaction with images (`EFI_HII_IMAGE_PROTOCOL`/`EFI_HII_IMAGE_EX_PROTOCOL`) and another one for interaction with strings (`EFI_HII_STRING_PROTOCOL`).
We've already glimpsed at the `EFI_HII_DATABASE_PROTOCOL` that is responsible for adding/removing HII packages to/from the databse.
We will investigate possibilities of this protocols more as we would go through different HII elements.

Besides the protocols this structure maintains a set of double linked list to different elements.
https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Base.h:
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
Among these lists there is a double linked list to database records ```LIST_ENTRY DatabaseList```.
You can look at the definition of a database record at the https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabase.h:
```
#define HII_DATABASE_RECORD_SIGNATURE   SIGNATURE_32 ('h','i','d','r')

typedef struct _HII_DATABASE_RECORD {
  UINTN                                 Signature;
  HII_DATABASE_PACKAGE_LIST_INSTANCE    *PackageList;
  EFI_HANDLE                            DriverHandle;
  EFI_HII_HANDLE                        Handle;
  LIST_ENTRY                            DatabaseEntry;
} HII_DATABASE_RECORD;
```
`LIST_ENTRY DatabaseList` points to the `DatabaseEntry` field in the first `HII_DATABASE_RECORD`. `DatabaseEntry` in this structure in turn points to the `DatabaseEntry` field in the next `HII_DATABASE_RECORD` and so on.

Each database record have a `HII_DATABASE_PACKAGE_LIST_INSTANCE *PackageList` field, let's look at this definition (https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabase.h):
```
typedef struct _HII_DATABASE_PACKAGE_LIST_INSTANCE {
  EFI_HII_PACKAGE_LIST_HEADER           PackageListHdr;
  LIST_ENTRY                            GuidPkgHdr;
  LIST_ENTRY                            FormPkgHdr;
  LIST_ENTRY                            KeyboardLayoutHdr;
  LIST_ENTRY                            StringPkgHdr;
  LIST_ENTRY                            FontPkgHdr;
  HII_IMAGE_PACKAGE_INSTANCE            *ImagePkg;
  LIST_ENTRY                            SimpleFontPkgHdr;
  UINT8                                 *DevicePathPkg;
} HII_DATABASE_PACKAGE_LIST_INSTANCE;
```
Each of these double linked list contains pointers to packages of corresponding type present in this package list.
In the previous lesson we've recieved all the package lists and its packages as a continious data array, but this was only a handy feature of the `ExportPackageLists` function from the `EFI_HII_DATABASE_PROTOCOL`. As you can see now in a reality HII data is represented in a double linked lists that could be sparsed all over the platform memory.

Another important field that is present in the `HII_DATABASE_RECORD` is a `EFI_HII_HANDLE Handle`. Each `HII_DATABASE_RECORD` defines a package list and is identified by this `EFI_HII_HANDLE`. According to the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiInternalFormRepresentation.h it is:
```
typedef VOID*   EFI_HII_HANDLE;
```
But if you look to the implementation of the `GenerateHiiDatabaseRecord` function in the https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/HiiDatabaseDxe/Database.c you could see the actual implementation of the HII handle:
```
EFI_STATUS
GenerateHiiDatabaseRecord (
  IN  HII_DATABASE_PRIVATE_DATA *Private,
  OUT HII_DATABASE_RECORD       **DatabaseNode
  )
{
 ...
 HII_HANDLE                         *HiiHandle;
 HII_DATABASE_RECORD                *DatabaseRecord;
 ...
 DatabaseRecord->Handle = (EFI_HII_HANDLE) HiiHandle;
 ...
} 
```
The type `HII_HANDLE` is defined in the https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabase.h:
```
#define HII_HANDLE_SIGNATURE            SIGNATURE_32 ('h','i','h','l')

typedef struct {
  UINTN               Signature;
  LIST_ENTRY          Handle;
  UINTN               Key;
} HII_HANDLE;
```

The `Key` field here corresponds to the current value of the `HiiHandleCount` field in the main `HII_DATABASE_PRIVATE_DATA`.

And the `LIST_ENTRY Handle` helps to connect all the `HII_HANDLEs` in the system together. The important point to note that the `LIST_ENTRY HiiHandleList` field in the main `HII_DATABASE_PRIVATE_DATA` is the same handle list.

Here is a picture of the HII database structure that we've just covered:
![HII_Database](HII_Database.png?raw=true "HII Database")

# HII database initialization

This HII database structure has static initializiation in the `HiiDatabaseDxe` (https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseEntry.c):
```
HII_DATABASE_PRIVATE_DATA mPrivate = {
  HII_DATABASE_PRIVATE_DATA_SIGNATURE,
  {
    (LIST_ENTRY *) NULL,
    (LIST_ENTRY *) NULL
  },
  {
    (LIST_ENTRY *) NULL,
    (LIST_ENTRY *) NULL
  },
  {
    HiiStringToImage,
    HiiStringIdToImage,
    HiiGetGlyph,
    HiiGetFontInfo
  },
  {
    HiiNewImage,
    HiiGetImage,
    HiiSetImage,
    HiiDrawImage,
    HiiDrawImageId
  },
  {
    HiiNewImageEx,
    HiiGetImageEx,
    HiiSetImageEx,
    HiiDrawImageEx,
    HiiDrawImageIdEx,
    HiiGetImageInfo
  },
  {
    HiiNewString,
    HiiGetString,
    HiiSetString,
    HiiGetLanguages,
    HiiGetSecondaryLanguages
  },
  {
    HiiNewPackageList,
    HiiRemovePackageList,
    HiiUpdatePackageList,
    HiiListPackageLists,
    HiiExportPackageLists,
    HiiRegisterPackageNotify,
    HiiUnregisterPackageNotify,
    HiiFindKeyboardLayouts,
    HiiGetKeyboardLayout,
    HiiSetKeyboardLayout,
    HiiGetPackageListHandle
  },
  {
    HiiConfigRoutingExtractConfig,
    HiiConfigRoutingExportConfig,
    HiiConfigRoutingRouteConfig,
    HiiBlockToConfig,
    HiiConfigToBlock,
    HiiGetAltCfg
  },
  {
    EfiConfigKeywordHandlerSetData,
    EfiConfigKeywordHandlerGetData
  },
  {
    (LIST_ENTRY *) NULL,
    (LIST_ENTRY *) NULL
  },
  0,
  {
    (LIST_ENTRY *) NULL,
    (LIST_ENTRY *) NULL
  },
  EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK),
  {
    0x00000000,
    0x0000,
    0x0000,
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
  },
  NULL
};
```

In the `HiiDatabaseDxe` driver entry point it initializes linked lists (`LIST_ENTRY`) and installs all the protocols from the HII database to the system:

```
EFI_STATUS
EFIAPI
InitializeHiiDatabase (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                             Status;
  EFI_HANDLE                             Handle;
  ...
  InitializeListHead (&mPrivate.DatabaseList);
  InitializeListHead (&mPrivate.DatabaseNotifyList);
  InitializeListHead (&mPrivate.HiiHandleList);
  InitializeListHead (&mPrivate.FontInfoList);
  ...
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiHiiFontProtocolGuid,
                  &mPrivate.HiiFont,
                  &gEfiHiiStringProtocolGuid,
                  &mPrivate.HiiString,
                  &gEfiHiiDatabaseProtocolGuid,
                  &mPrivate.HiiDatabase,
                  &gEfiHiiConfigRoutingProtocolGuid,
                  &mPrivate.ConfigRouting,
                  &gEfiConfigKeywordHandlerProtocolGuid,
                  &mPrivate.ConfigKeywordHandler,
                  NULL
                  );
  ...
  if (FeaturePcdGet (PcdSupportHiiImageProtocol)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Handle,
                    &gEfiHiiImageProtocolGuid, &mPrivate.HiiImage,
                    &gEfiHiiImageExProtocolGuid, &mPrivate.HiiImageEx,
                    NULL
                    );

  }
  ...
}
```
_____________________________________

# Linked lists pointers

Just in case here is some preprocessor magic that helps to get pointer to a structure by a pointer to its field.

It is easy to get pointer to `HII_DATABASE_RECORD` by the pointer to its field `DatabaseEntry` with a help of `CR` macro:
```
LIST_ENTRY* Link;
HII_DATABASE_RECORD* DatabaseRecord = CR (Link, HII_DATABASE_RECORD, DatabaseEntry, HII_DATABASE_RECORD_SIGNATURE);
```
In case you wonder definition for the CR macro can be found
https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/DebugLib.h
```
  @param  Record         The pointer to the field specified by Field within a data
                         structure of type TYPE.

  @param  TYPE           The name of the data structure type to return  This
                         data structure must contain the field specified by Field.

  @param  Field          The name of the field in the data structure specified
                         by TYPE to which Record points.

  @param  TestSignature  The 32-bit signature value to match.

**/
#if !defined(MDEPKG_NDEBUG)
  #define CR(Record, TYPE, Field, TestSignature)                                              \
    (DebugAssertEnabled () && (BASE_CR (Record, TYPE, Field)->Signature != TestSignature)) ?  \
    (TYPE *) (_ASSERT (CR has Bad Signature), Record) :                                       \
    BASE_CR (Record, TYPE, Field)
#else
  #define CR(Record, TYPE, Field, TestSignature)                                              \
    BASE_CR (Record, TYPE, Field)
#endif
```
And the definition for `BASE_CR` is placed under https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Base.h:
```
/**
  Macro that returns a pointer to the data structure that contains a specified field of
  that data structure.  This is a lightweight method to hide information by placing a
  public data structure inside a larger private data structure and using a pointer to
  the public data structure to retrieve a pointer to the private data structure.
  This function computes the offset, in bytes, of field specified by Field from the beginning
  of the  data structure specified by TYPE.  This offset is subtracted from Record, and is
  used to return a pointer to a data structure of the type specified by TYPE. If the data type
  specified by TYPE does not contain the field specified by Field, then the module will not compile.
  @param   Record   Pointer to the field specified by Field within a data structure of type TYPE.
  @param   TYPE     The name of the data structure type to return.  This data structure must
                    contain the field specified by Field.
  @param   Field    The name of the field in the data structure specified by TYPE to which Record points.
  @return  A pointer to the structure from one of it's elements.
**/
#define BASE_CR(Record, TYPE, Field)  ((TYPE *) ((CHAR8 *) (Record) - OFFSET_OF (TYPE, Field)))
```
```
/**
  The macro that returns the byte offset of a field in a data structure.
  This function returns the offset, in bytes, of field specified by Field from the
  beginning of the  data structure specified by TYPE. If TYPE does not contain Field,
  the module will not compile.
  @param   TYPE     The name of the data structure that contains the field specified by Field.
  @param   Field    The name of the field in the data structure.
  @return  Offset, in bytes, of field.
**/
#if (defined(__GNUC__) && __GNUC__ >= 4) || defined(__clang__)
#define OFFSET_OF(TYPE, Field) ((UINTN) __builtin_offsetof(TYPE, Field))
#endif

#ifndef OFFSET_OF
#define OFFSET_OF(TYPE, Field) ((UINTN) &(((TYPE *)0)->Field))
#endif
```
