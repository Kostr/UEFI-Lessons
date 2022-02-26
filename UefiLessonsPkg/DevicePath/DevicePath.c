/*
 * Copyright (c) 2021, Konstantin Aladyshev <aladyshev22@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>


#define EXAMPLE_PCI_FUNCTION 5
#define EXAMPLE_PCI_DEVICE 3

#pragma pack(1)
typedef struct {
  PCI_DEVICE_PATH             PciDevicePath;
  EFI_DEVICE_PATH_PROTOCOL    End;
} FULL_PCI_DEVICE_PATH;
#pragma pack()


FULL_PCI_DEVICE_PATH  PciDevicePathStatic = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_PCI_DP,
      {
        (UINT8) (sizeof (PCI_DEVICE_PATH)),
        (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8)
      }
    },
    EXAMPLE_PCI_FUNCTION,
    EXAMPLE_PCI_DEVICE
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};


EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  PCI_DEVICE_PATH PciDevicePathNodeStatic;
  PciDevicePathNodeStatic.Header.Type = HARDWARE_DEVICE_PATH;
  PciDevicePathNodeStatic.Header.SubType = HW_PCI_DP;
  PciDevicePathNodeStatic.Header.Length[0] = sizeof(PCI_DEVICE_PATH);
  PciDevicePathNodeStatic.Header.Length[1] = 0;
  PciDevicePathNodeStatic.Function = EXAMPLE_PCI_FUNCTION;
  PciDevicePathNodeStatic.Device = EXAMPLE_PCI_DEVICE;
  Print(L"PciDevicePathNodeStatic: %s\n", ConvertDeviceNodeToText((EFI_DEVICE_PATH_PROTOCOL*) &PciDevicePathNodeStatic, FALSE, FALSE));

  EFI_DEVICE_PATH_PROTOCOL* PciDevicePathNodeDynamic = CreateDeviceNode(HARDWARE_DEVICE_PATH, HW_PCI_DP, sizeof(PCI_DEVICE_PATH));
  ((PCI_DEVICE_PATH*)PciDevicePathNodeDynamic)->Function = EXAMPLE_PCI_FUNCTION;
  ((PCI_DEVICE_PATH*)PciDevicePathNodeDynamic)->Device = EXAMPLE_PCI_DEVICE;
  Print(L"PciDevicePathNodeDynamic: %s\n", ConvertDeviceNodeToText((EFI_DEVICE_PATH_PROTOCOL*) PciDevicePathNodeDynamic, FALSE, FALSE));

  Print(L"PciDevicePathStatic: %s\n", ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*) &PciDevicePathStatic, FALSE, FALSE));

  EFI_DEVICE_PATH_PROTOCOL* PciDevicePathDynamic = AppendDevicePathNode((EFI_DEVICE_PATH_PROTOCOL*)NULL, (EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathNodeDynamic);
  Print(L"PciDevicePathDynamic: %s\n", ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*) PciDevicePathDynamic, FALSE, FALSE));


  Print(L"_____________________\n\n");

  EFI_DEVICE_PATH_PROTOCOL* PciDevicePathDynamicMulti;
  EFI_DEVICE_PATH_PROTOCOL* TempPath;
  PciDevicePathDynamicMulti = AppendDevicePathNode((EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathDynamic, (EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathNodeDynamic);
  TempPath = PciDevicePathDynamicMulti;
  PciDevicePathDynamicMulti = AppendDevicePathNode((EFI_DEVICE_PATH_PROTOCOL*)TempPath, (EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathNodeDynamic);
  FreePool(TempPath);
  TempPath = PciDevicePathDynamicMulti;
  PciDevicePathDynamicMulti = AppendDevicePathNode((EFI_DEVICE_PATH_PROTOCOL*)TempPath, (EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathNodeDynamic);
  FreePool(TempPath);
  Print(L"Complicated DevicePath (AppendDevicePathNode): %s\n", ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*) PciDevicePathDynamicMulti, FALSE, FALSE));

  TempPath = PciDevicePathDynamicMulti;
  PciDevicePathDynamicMulti = AppendDevicePath((EFI_DEVICE_PATH_PROTOCOL*)TempPath, (EFI_DEVICE_PATH_PROTOCOL*)TempPath);
  FreePool(TempPath);
  Print(L"Complicated DevicePath (AppendDevicePath): %s\n", ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*) PciDevicePathDynamicMulti, FALSE, FALSE));


  Print(L"_____________________\n\n");

  EFI_DEVICE_PATH_PROTOCOL* TempDevicePathNode = PciDevicePathDynamicMulti;
  UINT8 PciNodeCount = 0;
  while (!IsDevicePathEnd(TempDevicePathNode)) {
    if ( (DevicePathType(TempDevicePathNode) == HARDWARE_DEVICE_PATH) && (DevicePathSubType(TempDevicePathNode) == HW_PCI_DP) )
      PciNodeCount++;
    TempDevicePathNode = NextDevicePathNode(TempDevicePathNode);
  }
  Print(L"Last device path has %d PCI nodes\n", PciNodeCount);

  Print(L"_____________________\n\n");

  EFI_DEVICE_PATH_PROTOCOL*  PciDevicePathNodeFromText = ConvertTextToDeviceNode(L"Pci(0x3,0x5)");
  Print(L"PciDevicePathNodeFromText: %s\n", ConvertDeviceNodeToText((EFI_DEVICE_PATH_PROTOCOL*) PciDevicePathNodeFromText, FALSE, FALSE));
  EFI_DEVICE_PATH_PROTOCOL*  PciDevicePathFromText = ConvertTextToDevicePath(L"Pci(0x3,0x5)");
  Print(L"PciDevicePathFromText: %s\n", ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*) PciDevicePathFromText, FALSE, FALSE));
 
  FreePool(PciDevicePathNodeDynamic);
  FreePool(PciDevicePathDynamic);
  FreePool(PciDevicePathNodeFromText);
  FreePool(PciDevicePathFromText);
  FreePool(PciDevicePathDynamicMulti);

  return EFI_SUCCESS;
}
