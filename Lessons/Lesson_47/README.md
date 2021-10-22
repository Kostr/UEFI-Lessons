Now we can finally register our Package list in the HII database. Here is a description for the `NewPackageList` one more time:
```
EFI_HII_DATABASE_PROTOCOL.NewPackageList()

Summary:
Adds the packages in the package list to the HII database.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_HII_DATABASE_NEW_PACK) (
 IN CONST EFI_HII_DATABASE_PROTOCOL *This,
 IN CONST EFI_HII_PACKAGE_LIST_HEADER *PackageList,
 IN CONST EFI_HANDLE DriverHandle, OPTIONAL
 OUT EFI_HII_HANDLE *Handle
 );

Parameters:
This		A pointer to the EFI_HII_DATABASE_PROTOCOL instance
PackageList	A pointer to an EFI_HII_PACKAGE_LIST_HEADER structure
DriverHandle	Associate the package list with this EFI handle
Handle		A pointer to the EFI_HII_HANDLE instance
Description	This function adds the packages in the package list to the database and returns a handle. If there is a
		EFI_DEVICE_PATH_PROTOCOL associated with the DriverHandle, then this function will create a
		package of type EFI_PACKAGE_TYPE_DEVICE_PATH and add it to the package list.
```

We've already filled PackageList array, so we can use this function like this:
```
  EFI_HII_HANDLE Handle;
  EFI_STATUS Status = gHiiDatabase->NewPackageList(gHiiDatabase, PackageListHdr, NULL, &Handle);
  if (EFI_ERROR(Status))
  {
    Print(L"Can't register HII Package list %g, status = %r\n", gHIIStringsCGuid, Status);
  }
```

Now when we have `EFI_HII_HANDLE Handle` for our package list it is time to try to get strings from the HII Database. For this we can utilize `GetString` from another of HII Database protocols - `EFI_HII_STRING_PROTOCOL`:
```
EFI_HII_STRING_PROTOCOL.GetString()

Summary:
Returns information about a string in a specific language, associated with a package list.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_HII_GET_STRING) (
 IN CONST EFI_HII_STRING_PROTOCOL *This,
 IN CONST CHAR8 *Language,
 IN EFI_HII_HANDLE PackageList,
 IN EFI_STRING_ID StringId,
 OUT EFI_STRING String,
 IN OUT UINTN *StringSize,
 OUT EFI_FONT_INFO **StringFontInfo OPTIONAL
 );
 
Parameters:
This			A pointer to the EFI_HII_STRING_PROTOCOL instance.
PackageList		The package list in the HII database to search for the specified string.
Language		Points to the language for the retrieved string. Callers of interfaces that require RFC
				4646 language codes to retrieve a Unicode string must use the RFC 4647 algorithm to
				lookup the Unicode string with the closest matching RFC 4646 language code.
StringId		The string’s id, which is unique within PackageList.
String			Points to the new null-terminated string.
StringSize		On entry, points to the size of the buffer pointed to by String, in bytes. On return,
				points to the length of the string, in bytes.
StringFontInfo	Points to a buffer that will be callee allocated and will have the string's font
				information into this buffer. The caller is responsible for freeing this buffer. If the
				parameter is NULL a buffer will not be allocated and the string font information will
				not be returned.

Description:
This function retrieves the string specified by StringId which is associated with the specified
PackageList in the language Language and copies it into the buffer specified by String.
```

Once again here is a standard UEFI scheme for getting resources of unknown size. First we call `GetString` with a `StringSize=0`. The function returns `EFI_BUFFER_TOO_SMALL`, but fills the `StringSize` with a necessary value. Then we call `GetString` one more time, this time with a correct `StringSize`.
As we've included `UefiHiiServicesLib` in our application we don't need to perform `LocateProtocol` to find `EFI_HII_STRING_PROTOCOL` but simply can use `gHiiString` variable.
Here is a function `PrintStringFromHiiHandle` for printing strings from Package list with a `EFI_HII_HANDLE` by simply providing string (Id, Language) combination:
```
EFI_STATUS PrintStringFromHiiHandle(EFI_HII_HANDLE* Handle, CHAR8* Language, UINTN StringId)
{
  EFI_STRING String = NULL;
  UINTN StringSize = 0;
  EFI_STATUS Status = gHiiString->GetString(gHiiString, Language, *Handle, StringId, String, &StringSize, NULL);

  if (Status != EFI_BUFFER_TOO_SMALL) {
    return Status;
  }

  String = AllocateZeroPool(StringSize);
  if (String == NULL) {
    return Status;
  }

  Status = gHiiString->GetString(gHiiString, Language, *Handle, StringId, String, &StringSize, NULL);

  Print(L"Status = %r, %s\n", Status, String);

  FreePool(String);

  return EFI_SUCCESS;
}
```

After we've registered our package we can use it as simple as this:
```
  PrintStringFromHiiHandle(&Handle, "en-US", 1);
  PrintStringFromHiiHandle(&Handle, "en-US", 2);
  PrintStringFromHiiHandle(&Handle, "fr-FR", 1);
  PrintStringFromHiiHandle(&Handle, "fr-FR", 2);
```

Now it is time to combine everything together and build our application.

If we execute it under OVMF we would get:
```
FS0:\> HIIStringsC.efi
Status = Success, English
Status = Success, Hello
Status = Success, French
Status = Success, Bonjour
```

So everything works correctly!

We can even see our newly created package if we execute our `ShowHII` application:
```
FS0:\> ShowHII.efi
...
PackageList[20]: GUID=8E0B8ED3-14F7-499D-A224-AEE89DC97FA3; size=0xC0
        Package[0]: type=STRINGS; size=0x53
        Package[1]: type=STRINGS; size=0x55
        Package[2]: type=END; size=0x4
```

Keep in mind that if we will try to execute the `HIIStringsC` app again we would get an errror:
```
FS0:\> HIIStringsC.efi
Can't register HII Package list 8E0B8ED3-14F7-499D-A224-AEE89DC97FA3, status = Invalid Parameter
```

It is happening, because it is not possible to register two Package lists with the same GUID.


