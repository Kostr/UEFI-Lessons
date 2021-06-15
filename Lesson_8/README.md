Let's explore protocols associated with our ImageHandle that we've discovered in the last lesson.

If we try to grep GUIDs in the edk2 source code:
```
$ grep -i bc62157e -r ./ --exclude-dir=Build
./MdePkg/Include/Protocol/LoadedImage.h:    0xbc62157e, 0x3e33, 0x4fec, {0x99, 0x20, 0x2d, 0x3b, 0x36, 0xd7, 0x50, 0xdf } \
./MdePkg/MdePkg.dec:  gEfiLoadedImageDevicePathProtocolGuid = { 0xbc62157e, 0x3e33, 0x4fec, {0x99, 0x20, 0x2d, 0x3b, 0x36, 0xd7, 0x50, 0xdf }}
$ grep -i 5B1B31A1 -r ./ --exclude-dir=Build
./MdePkg/Include/Protocol/LoadedImage.h:    0x5B1B31A1, 0x9562, 0x11d2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B } \
./MdePkg/MdePkg.dec:  gEfiLoadedImageProtocolGuid    = { 0x5B1B31A1, 0x9562, 0x11D2, { 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }}
```

Link to the header file: https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/LoadedImage.h



```
///
/// Can be used on any image handle to obtain information about the loaded image.
///
typedef struct {
  UINT32            Revision;       ///< Defines the revision of the EFI_LOADED_IMAGE_PROTOCOL structure.
                                    ///< All future revisions will be backward compatible to the current revision.
  EFI_HANDLE        ParentHandle;   ///< Parent image's image handle. NULL if the image is loaded directly from
                                    ///< the firmware's boot manager.
  EFI_SYSTEM_TABLE  *SystemTable;   ///< the image's EFI system table pointer.

  //
  // Source location of image
  //
  EFI_HANDLE        DeviceHandle;   ///< The device handle that the EFI Image was loaded from.
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;  ///< A pointer to the file path portion specific to DeviceHandle
                                        ///< that the EFI Image was loaded from.
  VOID              *Reserved;      ///< Reserved. DO NOT USE.

  //
  // Images load options
  //
  UINT32            LoadOptionsSize;///< The size in bytes of LoadOptions.
  VOID              *LoadOptions;   ///< A pointer to the image's binary load options.

  //
  // Location of where image was loaded
  //
  VOID              *ImageBase;     ///< The base address at which the image was loaded.
  UINT64            ImageSize;      ///< The size in bytes of the loaded image.
  EFI_MEMORY_TYPE   ImageCodeType;  ///< The memory type that the code sections were loaded as.
  EFI_MEMORY_TYPE   ImageDataType;  ///< The memory type that the data sections were loaded as.
  EFI_IMAGE_UNLOAD  Unload;
} EFI_LOADED_IMAGE_PROTOCOL;
```

EFI_LOADED_IMAGE_DEVICE_PATH_PROTOCOL - When installed, the Loaded Image Device Path Protocol specifies the device path that was used when a PE/COFF image was loaded through the EFI Boot Service LoadImage().
The Loaded Image Device Path Protocol uses the same protocol interface structure as the Device Path Protocol. The only difference between the Device Path Protocol and the Loaded Image Device Path Protocol is the protocol GUID value.
The Loaded Image Device Path Protocol must be installed onto the image handle of a PE/COFF image loaded through the EFI Boot Service LoadImage(). A copy of the device path specified by the DevicePath parameter to the EFI Boot Service LoadImage() is made before it is installed onto the image handle. It is legal to call LoadImage() for a buffer in memory with a NULL DevicePath parameter. In this case, the Loaded Image Device Path Protocol is installed with a NULL interface pointer.

So effectively if you want to look up the definition for this protocol you should search `EFI_DEVICE_PATH_PROTOCOL`.

It is defined in edk2 sources here https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/DevicePath.h :
```
/**
  This protocol can be used on any device handle to obtain generic path/location
  information concerning the physical device or logical device. If the handle does
  not logically map to a physical device, the handle may not necessarily support
  the device path protocol. The device path describes the location of the device
  the handle is for. The size of the Device Path can be determined from the structures
  that make up the Device Path.
**/
typedef struct {
  UINT8 Type;       ///< 0x01 Hardware Device Path.
                    ///< 0x02 ACPI Device Path.
                    ///< 0x03 Messaging Device Path.
                    ///< 0x04 Media Device Path.
                    ///< 0x05 BIOS Boot Specification Device Path.
                    ///< 0x7F End of Hardware Device Path.

  UINT8 SubType;    ///< Varies by Type
                    ///< 0xFF End Entire Device Path, or
                    ///< 0x01 End This Instance of a Device Path and start a new
                    ///< Device Path.

  UINT8 Length[2];  ///< Specific Device Path data. Type and Sub-Type define
                    ///< type of data. Size of data is included in Length.

} EFI_DEVICE_PATH_PROTOCOL;
```


With all this info let's write source code for our new `ImageInfo` app:
```
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/LoadedImage.h>
#include <Library/DevicePathLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;

  gBS->HandleProtocol(
    ImageHandle,
    &gEfiLoadedImageProtocolGuid,
    (VOID **) &LoadedImage
  );

  EFI_DEVICE_PATH_PROTOCOL* DevicePath;

  gBS->HandleProtocol(
    LoadedImage->DeviceHandle,                  // EFI_HANDLE
    &gEfiLoadedImageDevicePathProtocolGuid,
    (VOID**) &DevicePath
  );

  Print(L"Image device: %s\n", ConvertDevicePathToText(DevicePath, FALSE, TRUE));
  Print(L"Image file: %s\n",  ConvertDevicePathToText(LoadedImage->FilePath, FALSE, TRUE));     // EFI_DEVICE_PATH_PROTOCOL *FilePath
  Print(L"Image Base: %X\n", LoadedImage->ImageBase);
  Print(L"Image Size: %X\n", LoadedImage->ImageSize);
  return EFI_SUCCESS;
}
```


```
/home/kostr/tiano/edk2/UefiLessonsPkg/ImageInfo/ImageInfo.c:16: undefined reference to `gEfiLoadedImageProtocolGuid'
/usr/bin/ld: /tmp/ImageInfo.dll.JKxzWu.ltrans0.ltrans.o:/home/kostr/tiano/edk2/UefiLessonsPkg/ImageInfo/ImageInfo.c:24: undefined reference to `gEfiLoadedImageDevicePathProtocolGuid'
```

To solve this error add `Protocols` section to the `UefiLessonsPkg/ImageInfo/ImageInfo.inf` file:
```
[Protocols]
  gEfiLoadedImageProtocolGuid                   ## CONSUMES
  gEfiLoadedImageDevicePathProtocolGuid         ## CONSUMES
```

The [Protocols] section of the EDK II INF file is a list of the global Protocol C Names that are used by the module developer. These C names are used by the parsing utility to lookup the actual GUID value of the PROTOCOL that is located in the EDK II package DEC files, and then emit a data structure to the module's AutoGen.c file (https://edk2-docs.gitbook.io/edk-ii-inf-specification/2_inf_overview/29_-protocols-_section).


`## CONSUMES` is used to indicate usage of this protocol. It has no value other than informational. You can encounter values like this for example in the edk2 codebase:
```
## CONSUMES
## PRODUCES
## ALWAYS_CONSUMES
## ALWAYS_PRODUCES
## SOMETIMES_PRODUCES
## SOMETIMES_PRODUCES
## NOTIFY
```

After we've added these protocols to the *.inf file, the build will succeed.

In case you wonder our GUID variables will go to a file `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/ImageInfo/ImageInfo/DEBUG/AutoGen.c` which is automatically generated by the build system:
```
// Protocols
GLOBAL_REMOVE_IF_UNREFERENCED EFI_GUID gEfiLoadedImageProtocolGuid = { 0x5B1B31A1, 0x9562, 0x11D2, { 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }};
GLOBAL_REMOVE_IF_UNREFERENCED EFI_GUID gEfiLoadedImageDevicePathProtocolGuid = { 0xbc62157e, 0x3e33, 0x4fec, {0x99, 0x20, 0x2d, 0x3b, 0x36, 0xd7, 0x50, 0xdf }};
```
