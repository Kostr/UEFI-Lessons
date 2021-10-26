In this lesson we would look at another method how your application can publish HII String packages. This time we would talk about embedding HII data in the resulting EFI file PE/COFF resources.

# Create application

As usual create new application:
```
./createNewApp.sh HIIStringsUNIRC
```

Add it to our DSC package file UefiLessonsPkg/UefiLessonsPkg.dsc:
```
[Components]
  ...
  UefiLessonsPkg/HIIStringsUNI/HIIStringsUNI.inf
```

As last time, we would need a GUID for our package list, declare it in package DEC file UefiLessonsPkg/UefiLessonsPkg.dec:
```
[Guids]
  ...
  gHIIStringsUNIRCGuid = { 0x785693b4, 0x623e, 0x40fa, { 0x9a, 0x45, 0x68, 0xda, 0x38, 0x30, 0x89, 0xdd }}
```

Now modify application files to be similar to the `HIIStringsUNI` app. You'll need to create `Strings.uni` file and modify application INF and *.c file:
- `UefiLessonsPkg/HIIStringsUNIRC/HIIStringsUNIRC.inf`
- `UefiLessonsPkg/HIIStringsUNIRC/HIIStringsUNIRC.c`
- `UefiLessonsPkg/HIIStringsUNIRC/Strings.uni`

In the end you should have the same result as the `HIIStringsUNI` app:
```
FS0:\> HIIStringsUNIRC.efi
en-US ID=1: English
en-US ID=2: Hello!
en-US ID=3: Bye!
fr-FR ID=1: Francais
fr-FR ID=2: Bonjour!
fr-FR ID=3: Au revoir!
Best language ID=1: English
Best language ID=2: Hello!
Best language ID=3: Bye!
fr ID=3: Au revoir!
```

Now we'are ready for modifications.

# UEFI_HII_RESOURCE_SECTION

Add this to the `UefiLessonsPkg/HIIStringsUNIRC/HIIStringsUNIRC.inf`:
```
[Defines]
  ...
  UEFI_HII_RESOURCE_SECTION      = TRUE
```

`UEFI_HII_RESOURCE_SECTION` flag specifies whether HII resource section is generated into PE image.

Now you wouldn't be able to build our application because of the error:
```
UefiLessonsPkg/HIIStringsUNIRC/HIIStringsUNIRC.c:15:42: error: ‘HIIStringsUNIRCStrings’ undeclared (first use in this function);
```

If you'll look at the `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIStringsUNIRC/HIIStringsUNIRC/DEBUG/HIIStringsUNIRCStrDefs.h` you'll see that this file still contains string tokens, but the `HIIStringsUNIRCStrings` is no longer here:

And `AutoGen.c` file no longer contains `HIIStringsUNIRCStrings` array initialization code (`Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIStringsUNIRC/HIIStringsUNIRC/DEBUG/AutoGen.c`).

Now our strings directly encoded into the special section of the resulting *.efi image.

To get them we need to search for a protocol `EFI_HII_PACKAGE_LIST_PROTOCOL` in the application `EFI_HANDLE ImageHandle`. 

Here is relevant content from UEFI specification for the `EFI_BOOT_SERVICES.LoadImage()` function which shell uses to load every program:
```
Once the image is loaded, LoadImage() installs EFI_HII_PACKAGE_LIST_PROTOCOL on the handle if
the image contains a custom PE/COFF resource with the type 'HII'. The protocol's interface pointer points
to the HII package list which is contained in the resource's data
```

The `EFI_HII_PACKAGE_LIST_PROTOCOL` is identified by the `gEfiHiiPackageListProtocolGuid` from the https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec:
```
[Protocols]
  ...
  ## Include/Protocol/HiiPackageList.h
  gEfiHiiPackageListProtocolGuid  = { 0x6a1ee763, 0xd47a, 0x43b4, {0xaa, 0xbe, 0xef, 0x1d, 0xe2, 0xab, 0x56, 0xfc}}
```

As we already have `MdeModulePkg/MdeModulePkg.dec` in the `[Packages]` section of our INF file, all we need to do is add this GUID to the `[Protocols]` section:
```
[Protocols]
  gEfiHiiPackageListProtocolGuid
```

In the code we could aquire `PackageList` protocol from the application `ImageHandle` with a help of `OpenProtocol` UEFI boot service:
```
...

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  EFI_HII_PACKAGE_LIST_HEADER *PackageList;
  //
  // Retrieve HII package list from ImageHandle.
  //
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    Print(L"Error! Can't open EFI_HII_PACKAGE_LIST_PROTOCOL\n");
    return Status;
  }
  ...
```

The resulting `PackageList` is not only String packages with some size header from the `StrGather` script like in was in the case of `HIIStringsUNI` application. It is an ordinary Package list like the one that we've manually constructed in our `HIIStringsC` application.
So instead of using `HiiAddPackages` library function we need to use `EFI_HII_DATABASE_PROTOCOL.NewPackageList()` directly:
```
EFI_HII_HANDLE Handle;
Status = gHiiDatabase->NewPackageList(gHiiDatabase, PackageList, NULL, &Handle);
if (EFI_ERROR(Status))
{
  Print(L"Can't register HII Package list %g, status = %r\n", gHIIStringsUNIRCGuid, Status);
  return Status;
}
```

Here I've used `gHiiDatabase`, so don't forget to add necessary header ```#include <Library/UefiHiiServicesLib.h>```and add `UefiHiiServicesLib` to the Library classes in the application INF file:
```
[LibraryClasses]
  ...
  UefiHiiServicesLib
```

Now you can build and verify that everything is ok:
```
FS0:\> HIIStringsUNIRC.efi
Status = Success
en-US ID=1: English
en-US ID=2: Hello!
en-US ID=3: Bye!
fr-FR ID=1: Francais
fr-FR ID=2: Bonjour!
fr-FR ID=3: Au revoir!
Best language ID=1: English
Best language ID=2: Hello!
Best language ID=3: Bye!
fr ID=3: Au revoir!
```

# PE/COFF resource with the type 'HII'

Let's use `objdump` utility to look at the application headers. You can output content of all headers with the `-x` option:
```
objdump -x  Build/UefiLessonsPkg/RELEASE_GCC5/X64/HIIStringsUNIRC.efi
```

Pay attention to these things:
```
...

The Data Directory
Entry 0 0000000000000000 00000000 Export Directory [.edata (or where ever we found it)]
Entry 1 0000000000000000 00000000 Import Directory [parts of .idata]
Entry 2 00000000000023c0 00000180 Resource Directory [.rsrc]                <---------- Resource Directory has data
Entry 3 0000000000000000 00000000 Exception Directory [.pdata]
Entry 4 0000000000000000 00000000 Security Directory
Entry 5 0000000000000000 00000000 Base Relocation Directory [.reloc]
Entry 6 00000000000022cc 0000001c Debug Directory
Entry 7 0000000000000000 00000000 Description Directory
Entry 8 0000000000000000 00000000 Special Directory
Entry 9 0000000000000000 00000000 Thread Storage Directory [.tls]
Entry a 0000000000000000 00000000 Load Configuration Directory
Entry b 0000000000000000 00000000 Bound Import Directory
Entry c 0000000000000000 00000000 Import Address Table Directory
Entry d 0000000000000000 00000000 Delay Import Directory
Entry e 0000000000000000 00000000 CLR Runtime Header
Entry f 0000000000000000 00000000 Reserved

...

The .rsrc Resource Directory section:
000  Type Table: Char: 0, Time: 00000000, Ver: 0/0, Num Names: 1, IDs: 0
010   Entry: name: [val: 80000048 len 3]: HII, Value: 0x80000018           <--------- Data has type HII
018    Name Table: Char: 0, Time: 00000000, Ver: 0/0, Num Names: 1, IDs: 0
028     Entry: name: [val: 80000050 len 3]: EFI, Value: 0x80000030
030      Language Table: Char: 0, Time: 00000000, Ver: 0/0, Num Names: 1, IDs: 0
040       Entry: name: [val: 80000058 len 3]: BIN, Value: 0x000060
060        Leaf: Addr: 0x002430, Size: 0x0000ea, Codepage: 0
 String table starts at offset: 0x48
 Resources start at offset: 0x70

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .text         00001fc0  0000000000000240  0000000000000240  00000240  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .data         000001c0  0000000000002200  0000000000002200  00002200  2**4
                  CONTENTS, ALLOC, LOAD, DATA
  2 .rsrc         00000180  00000000000023c0  00000000000023c0  000023c0  2**2       <----- .rsrc is displayed here as well
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
SYMBOL TABLE:
no symbols
```

Now comment `UEFI_HII_RESOURCE_SECTION` in the `UefiLessonsPkg/HIIStringsUNIRC/HIIStringsUNIRC.inf`:
```
[Defines]
  ...
  #UEFI_HII_RESOURCE_SECTION      = TRUE
```
Build application and execute `objdump` once again:
```
...

The Data Directory
Entry 0 0000000000000000 00000000 Export Directory [.edata (or where ever we found it)]
Entry 1 0000000000000000 00000000 Import Directory [parts of .idata]
Entry 2 0000000000000000 00000000 Resource Directory [.rsrc]                 <----- Resource directory is empty
Entry 3 0000000000000000 00000000 Exception Directory [.pdata]
Entry 4 0000000000000000 00000000 Security Directory
Entry 5 0000000000000000 00000000 Base Relocation Directory [.reloc]
Entry 6 00000000000022cc 0000001c Debug Directory
Entry 7 0000000000000000 00000000 Description Directory
Entry 8 0000000000000000 00000000 Special Directory
Entry 9 0000000000000000 00000000 Thread Storage Directory [.tls]
Entry a 0000000000000000 00000000 Load Configuration Directory
Entry b 0000000000000000 00000000 Bound Import Directory
Entry c 0000000000000000 00000000 Import Address Table Directory
Entry d 0000000000000000 00000000 Delay Import Directory
Entry e 0000000000000000 00000000 CLR Runtime Header
Entry f 0000000000000000 00000000 Reserved

...                                                                          <----- No .rsrc Resource Directory section

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .text         00001fc0  0000000000000240  0000000000000240  00000240  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .data         000001c0  0000000000002200  0000000000002200  00002200  2**4   <---- No .rsrc section here as well
                  CONTENTS, ALLOC, LOAD, DATA
SYMBOL TABLE:
no symbols
```

If you try to execute this version of our application you would get:
```
FS0:\> HIIStringsUNIRC.efi
Error! Can't open EFI_HII_PACKAGE_LIST_PROTOCOL
```

