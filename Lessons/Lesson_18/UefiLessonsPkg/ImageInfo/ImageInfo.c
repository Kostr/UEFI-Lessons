/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

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
  EFI_STATUS Status;
  EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;

  Status = gBS->HandleProtocol(
    ImageHandle,
    &gEfiLoadedImageProtocolGuid,
    (VOID **) &LoadedImage
  );

  if (Status == EFI_SUCCESS) {
    EFI_DEVICE_PATH_PROTOCOL* DevicePath;

    Status = gBS->HandleProtocol(
      ImageHandle,
      &gEfiLoadedImageDevicePathProtocolGuid,
      (VOID**) &DevicePath
    );

    if (Status == EFI_SUCCESS) {
      Print(L"Image device: %s\n", ConvertDevicePathToText(DevicePath, FALSE, TRUE));
      Print(L"Image file: %s\n",  ConvertDevicePathToText(LoadedImage->FilePath, FALSE, TRUE));	// EFI_DEVICE_PATH_PROTOCOL *FilePath
      Print(L"Image Base: %X\n", LoadedImage->ImageBase);
      Print(L"Image Size: %X\n", LoadedImage->ImageSize);
    } else {
      Print(L"Can't get EFI_LOADED_IMAGE_PROTOCOL, Status=%r\n", Status);
    }
  } else {
    Print(L"Can't get EFI_DEVICE_PATH_PROTOCOL, Status=%r\n", Status);
  }
  return EFI_SUCCESS;
}
