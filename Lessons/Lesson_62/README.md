In this lesson we will talk about `DevicePath` concept in UEFI.

A Device Path is used to define the programmatic path to a device in UEFI environment.

Internally `Device path` consists from the so called `Device Path nodes` which directly follow each other in memory. At the very end of every `Device path` there is a `Device Path node` of a special `End` type.

Basically it looks something like this:
```
Device path = 
  <Device Path node>
  ...
  <Device Path node>
  <Device Path node with the End type>
```
There is a possibility to encode several so called `Device Path instances` in the Device path. In this case Device Path would look a little bit more complicated:
```
Device path = 
  <Device Path node>
  ...
  <Device Path node>
  <Device Path node with the End instance type>
  <Device Path node>
  ...
  <Device Path node>
  <Device Path node with the End instance type>
  <Device Path node>
  ...
  <Device Path node>
  <Device Path node with the End type>
```

Now let's look at the definition of the `<Device Path node>`. Each `<Device Path node>` has a `EFI_DEVICE_PATH_PROTOCOL` header. The size of this header is only 4 bytes. And after it immediately follows data in a format that is dictated by the values of the header Type/SubType's fields.

https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/DevicePath.h
```
#pragma pack(1)

typedef struct {
  UINT8    Type;    ///< 0x01 Hardware Device Path.
                    ///< 0x02 ACPI Device Path.
                    ///< 0x03 Messaging Device Path.
                    ///< 0x04 Media Device Path.
                    ///< 0x05 BIOS Boot Specification Device Path.
                    ///< 0x7F End of Hardware Device Path.

  UINT8    SubType; ///< Varies by Type

  UINT8    Length[2]; ///< Specific Device Path data. Type and Sub-Type define
                      ///< type of data. Size of data is included in Length.
} EFI_DEVICE_PATH_PROTOCOL;
```

Currently there are 6 main types of Device Paths. You can see them in the Type field comment above. Each of these types has several subtypes. And each of these subtypes has a unique well defined format for its data. And all of them are defined in the UEFI specification.

For the glance you could look at the structures in the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/DevicePath.h
Most of the structures in this file corresponds to different data formatting based on the Type/SybType fields.

The `Length` field defines full size of the `Device Node` including the header. `Length[0]` defines the lower byte and `Length[1]` defines higher byte of the length.

For example PCI device has:
- a type `#define HARDWARE_DEVICE_PATH  0x01` 
- a subtype `#define HW_PCI_DP  0x01`

And the full path is encoded as this:
```
///
/// PCI Device Path.
///
typedef struct {
  EFI_DEVICE_PATH_PROTOCOL    Header;
  ///
  /// PCI Function Number.
  ///
  UINT8                       Function;
  ///
  /// PCI Device Number.
  ///
  UINT8                       Device;
} PCI_DEVICE_PATH;
```
If we'll unwind the `EFI_DEVICE_PATH_PROTOCOL` structure it would look like this:
```
{
  UINT8    Type      = HARDWARE_DEVICE_PATH = 0x01
  UINT8    SubType   = HW_PCI_DP = 0x01
  UINT8    Length[0] = 0x06
  UINT8    Length[1] = 0x00
  UINT8    Function;
  UINT8    Device;
}
```

# Create Device Path/Device Node statically and dynamically

Let's try to write an application to investigate how it is possible to work with `Device Path` in UEFI.

Create an application, and include `<Library/DevicePathLib.h>` header to the source code. This is the header for the UEFI Device Path library (https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/DevicePathLib.h).

Let's assume that we want to create a device path for the PCI device with just a single node of a type `PCI_DEVICE_PATH`. Our theoretical path would refer to the PCI device `Function=5`/`Device=3`.


The code for the static device node with such characteristics can look like this:
```
#define EXAMPLE_PCI_FUNCTION 5
#define EXAMPLE_PCI_DEVICE 3

PCI_DEVICE_PATH PciDevicePathNodeStatic;
PciDevicePathNodeStatic.Header.Type = HARDWARE_DEVICE_PATH;
PciDevicePathNodeStatic.Header.SubType = HW_PCI_DP;
PciDevicePathNodeStatic.Header.Length[0] = sizeof(PCI_DEVICE_PATH);
PciDevicePathNodeStatic.Header.Length[1] = 0;
PciDevicePathNodeStatic.Function = EXAMPLE_PCI_FUNCTION;
PciDevicePathNodeStatic.Device = EXAMPLE_PCI_DEVICE;
```
or like this (which is effectively the same):
```
#define EXAMPLE_PCI_FUNCTION 5
#define EXAMPLE_PCI_DEVICE 3

PCI_DEVICE_PATH PciDevicePathNodeStatic = {
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
  EXAMPLE_PCI_DEVICE;
};
```

A DevicePathLib has a function to print DeviceNodes in a human readable text form:
```
/**
  Converts a device node to its string representation.
  @param DeviceNode        A Pointer to the device node to be converted.
  @param DisplayOnly       If DisplayOnly is TRUE, then the shorter text representation
                           of the display node is used, where applicable. If DisplayOnly
                           is FALSE, then the longer text representation of the display node
                           is used.
  @param AllowShortcuts    If AllowShortcuts is TRUE, then the shortcut forms of text
                           representation for a device node can be used, where applicable.
  @return A pointer to the allocated text representation of the device node or NULL if DeviceNode
          is NULL or there was insufficient memory.
**/
CHAR16 *
EFIAPI
ConvertDeviceNodeToText (
  IN CONST EFI_DEVICE_PATH_PROTOCOL  *DeviceNode,
  IN BOOLEAN                         DisplayOnly,
  IN BOOLEAN                         AllowShortcuts
  );
```
If we would use it to print our Device node like this:
```
Print(L"PciDevicePathNodeStatic: %s\n", ConvertDeviceNodeToText((EFI_DEVICE_PATH_PROTOCOL*) &PciDevicePathNodeStatic, FALSE, FALSE));
```
We would get:
```
FS0:\> DevicePath.efi
PciDevicePathNodeStatic: Pci(0x3,0x5)
```

Now let's try to create Device Path node dynamically. For this we can utilize `CreateDeviceNode` function from the library:
```
/**
  Creates a device node.
  This function creates a new device node in a newly allocated buffer of size
  NodeLength and initializes the device path node header with NodeType and NodeSubType.
  The new device path node is returned.
  If NodeLength is smaller than a device path header, then NULL is returned.
  If there is not enough memory to allocate space for the new device path, then
  NULL is returned.
  The memory is allocated from EFI boot services memory. It is the responsibility
  of the caller to free the memory allocated.
  @param  NodeType                   The device node type for the new device node.
  @param  NodeSubType                The device node sub-type for the new device node.
  @param  NodeLength                 The length of the new device node.
  @return The new device path.
**/
EFI_DEVICE_PATH_PROTOCOL *
CreateDeviceNode (
   UINT8                           NodeType,
   UINT8                           NodeSubType,
   UINT16                          NodeLength
  )
```

The code would look like this:
```
EFI_DEVICE_PATH_PROTOCOL* PciDevicePathNodeDynamic = CreateDeviceNode(HARDWARE_DEVICE_PATH, HW_PCI_DP, sizeof(PCI_DEVICE_PATH));
((PCI_DEVICE_PATH*)PciDevicePathNodeDynamic)->Function = EXAMPLE_PCI_FUNCTION;
((PCI_DEVICE_PATH*)PciDevicePathNodeDynamic)->Device = EXAMPLE_PCI_DEVICE;
Print(L"PciDevicePathNodeDynamic: %s\n", ConvertDeviceNodeToText((EFI_DEVICE_PATH_PROTOCOL*) PciDevicePathNodeDynamic, FALSE, FALSE));
```
And it would gave us exactly the same result:
```
FS0:\> DevicePath.efi
PciDevicePathNodeStatic: Pci(0x3,0x5)
PciDevicePathNodeDynamic: Pci(0x3,0x5)
```

Now let's try to create complete Device Path. For this we need to add Device Node with the End type right after the current one that we've created.

Once again we start with a static declaration method. We need to create our own structure type which would include both `PCI_DEVICE_PATH` node and the `End` node:
```
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
```

To print Device path as a string we can use `ConvertDevicePathToText` function. This function looks pretty similar to the `ConvertDeviceNodeToText` but do not confuse them! If you'll try to perform `ConvertDevicePathToText` not on the device path, but on the device node, the system would probably hang.
```
/**
  Converts a device path to its text representation.
  @param DevicePath      A Pointer to the device to be converted.
  @param DisplayOnly     If DisplayOnly is TRUE, then the shorter text representation
                         of the display node is used, where applicable. If DisplayOnly
                         is FALSE, then the longer text representation of the display node
                         is used.
  @param AllowShortcuts  If AllowShortcuts is TRUE, then the shortcut forms of text
                         representation for a device node can be used, where applicable.
  @return A pointer to the allocated text representation of the device path or
          NULL if DeviceNode is NULL or there was insufficient memory.
**/
CHAR16 *
EFIAPI
ConvertDevicePathToText (
  IN CONST EFI_DEVICE_PATH_PROTOCOL  *DevicePath,
  IN BOOLEAN                         DisplayOnly,
  IN BOOLEAN                         AllowShortcuts
  );
```

Usage as simple as before:
```
Print(L"PciDevicePathStatic: %s\n", ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*) &PciDevicePathStatic, FALSE, FALSE));
```
Which would once again give us the same result (as we've only added the `End` node, which has no valuable string representation):
```
PciDevicePathStatic: Pci(0x3,0x5)
```

At last let's create Device Path dynamically. For this task we can utilize `AppendDevicePathNode` function:
```
/**
  Creates a new path by appending the device node to the device path.
  This function creates a new device path by appending a copy of the device node specified by
  DevicePathNode to a copy of the device path specified by DevicePath in an allocated buffer.
  The end-of-device-path device node is moved after the end of the appended device node.
  If DevicePathNode is NULL then a copy of DevicePath is returned.
  If DevicePath is NULL then a copy of DevicePathNode, followed by an end-of-device path device
  node is returned.
  If both DevicePathNode and DevicePath are NULL then a copy of an end-of-device-path device node
  is returned.
  If there is not enough memory to allocate space for the new device path, then NULL is returned.
  The memory is allocated from EFI boot services memory. It is the responsibility of the caller to
  free the memory allocated.
  @param  DevicePath                 A pointer to a device path data structure.
  @param  DevicePathNode             A pointer to a single device path node.
  @retval NULL      There is not enough memory for the new device path.
  @retval Others    A pointer to the new device path if success.
                    A copy of DevicePathNode followed by an end-of-device-path node
                    if both FirstDevicePath and SecondDevicePath are NULL.
                    A copy of an end-of-device-path node if both FirstDevicePath and SecondDevicePath are NULL.
**/
EFI_DEVICE_PATH_PROTOCOL *
EFIAPI
AppendDevicePathNode (
  IN CONST EFI_DEVICE_PATH_PROTOCOL  *DevicePath      OPTIONAL,
  IN CONST EFI_DEVICE_PATH_PROTOCOL  *DevicePathNode  OPTIONAL
  );
```
As the comment says, if we'll supply NULL in place of the first argument, the function can add the necessary Device Node End to our Device Node. Thus effectively creating Device Path from the Device Node.
```
EFI_DEVICE_PATH_PROTOCOL* PciDevicePathDynamic = AppendDevicePathNode((EFI_DEVICE_PATH_PROTOCOL*)NULL, (EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathNodeDynamic);
```
If you'll try to print it:
```
Print(L"PciDevicePathDynamic: %s\n", ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*) PciDevicePathDynamic, FALSE, FALSE));
```
Once again you would get the same result:
```
PciDevicePathDynamic: Pci(0x3,0x5)
```

# Multinode Device Paths

When you boot UEFI shell it prints mapping table which shows Device Paths for mappings:
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
Press ESC in 5 seconds to skip startup.nsh or any other key to continue.
```

In the `PciRoot(0x0)/Pci(0x1,0x1)/Ata(0x0)/HD(1,MBR,0xBE1AFDFA,0x3F,0xFBFC1)` path each text between the `/` corresponds to the Device Path node. For example this particular path consist from 5 Device nodes (don't forget the Device End node).

Sane path for the PCI device would have `PciRoot` node before the `Pci` node. But it is not an UEFI specification task to check these things. UEFI specification only defines a structure for the Device Path that was analyzed in the beginning of the lesson.

Therefore is is possible to create Device Path from any Device Nodes.

Let's use the `AppendDevicePathNode` function that we've already know to add 3 more PCI nodes to our path:
```
PciDevicePathDynamic = AppendDevicePathNode((EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathDynamic, (EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathNodeDynamic);
PciDevicePathDynamic = AppendDevicePathNode((EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathDynamic, (EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathNodeDynamic);
PciDevicePathDynamic = AppendDevicePathNode((EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathDynamic, (EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathNodeDynamic);
Print(L"Complicated DevicePath (AppendDevicePathNode): %s\n", ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*) PciDevicePathDynamic, FALSE, FALSE));
```
As you might guess the `Print` statement would print this:
```
Complicated DevicePath (AppendDevicePathNode): Pci(0x3,0x5)/Pci(0x3,0x5)/Pci(0x3,0x5)/Pci(0x3,0x5)
```

We can even concatenate two Device Paths with a help of the similar `AppendDevicePath` function. This function would correctly concatenate Device Path data nodes, so there would be only one Device Path End node in the final path:
```
/**
  Creates a new device path by appending a second device path to a first device path.
  This function creates a new device path by appending a copy of SecondDevicePath to a copy of
  FirstDevicePath in a newly allocated buffer.  Only the end-of-device-path device node from
  SecondDevicePath is retained. The newly created device path is returned.
  If FirstDevicePath is NULL, then it is ignored, and a duplicate of SecondDevicePath is returned.
  If SecondDevicePath is NULL, then it is ignored, and a duplicate of FirstDevicePath is returned.
  If both FirstDevicePath and SecondDevicePath are NULL, then a copy of an end-of-device-path is
  returned.
  If there is not enough memory for the newly allocated buffer, then NULL is returned.
  The memory for the new device path is allocated from EFI boot services memory. It is the
  responsibility of the caller to free the memory allocated.
  @param  FirstDevicePath            A pointer to a device path data structure.
  @param  SecondDevicePath           A pointer to a device path data structure.
  @retval NULL      If there is not enough memory for the newly allocated buffer.
  @retval NULL      If FirstDevicePath or SecondDevicePath is invalid.
  @retval Others    A pointer to the new device path if success.
                    Or a copy an end-of-device-path if both FirstDevicePath and SecondDevicePath are NULL.
**/
EFI_DEVICE_PATH_PROTOCOL *
EFIAPI
AppendDevicePath (
  IN CONST EFI_DEVICE_PATH_PROTOCOL  *FirstDevicePath   OPTIONAL,
  IN CONST EFI_DEVICE_PATH_PROTOCOL  *SecondDevicePath  OPTIONAL
  );
```
We can use it like this to double the data nodes in our path:
```
PciDevicePathDynamic = AppendDevicePath((EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathDynamic, (EFI_DEVICE_PATH_PROTOCOL*)PciDevicePathDynamic);
Print(L"Complicated DevicePath (AppendDevicePath): %s\n", ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*) PciDevicePathDynamic, FALSE, FALSE));
```
And this would give us 8 PCI nodes instead of 4:
```
Complicated DevicePath (AppendDevicePath): Pci(0x3,0x5)/Pci(0x3,0x5)/Pci(0x3,0x5)/Pci(0x3,0x5)/Pci(0x3,0x5)/Pci(0x3,0x5)/Pci(0x3,0x5)/Pci(0x3,0x5)
```

We should not forget the internals of the `AppendDevicePathNode`/`AppendDevicePath` functions! Every call creates a new buffer, but we shouldn't forget to free the old buffer, that we no longer need. Else we would have a memory leak on every call. So the more correct code would look like this:
```
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
```

# Iterate over Device Path

The library presents several functions to iterate throught the device paths. Since the only way to iterate Device Path is to walk through its Device Nodes one by one until the Device End Node would be found, the library presents two functions: `IsDevicePathEnd` and `NextDevicePathNode`.
```
/**
  Determines if a device path node is an end node of an entire device path.
  Determines if a device path node specified by Node is an end node of an entire device path.
  If Node represents the end of an entire device path, then TRUE is returned.
  Otherwise, FALSE is returned.
  If Node is NULL, then ASSERT().
  @param  Node      A pointer to a device path node data structure.
  @retval TRUE      The device path node specified by Node is the end of an entire device path.
  @retval FALSE     The device path node specified by Node is not the end of an entire device path.
**/
BOOLEAN
EFIAPI
IsDevicePathEnd (
  IN CONST VOID  *Node
  );
```
```
/**
  Returns a pointer to the next node in a device path.
  Returns a pointer to the device path node that follows the device path node specified by Node.
  If Node is NULL, then ASSERT().
  @param  Node      A pointer to a device path node data structure.
  @return a pointer to the device path node that follows the device path node specified by Node.
**/
EFI_DEVICE_PATH_PROTOCOL *
EFIAPI
NextDevicePathNode (
  IN CONST VOID  *Node
  );
```

Let's count how many PCI nodes our current Device Path has:
```
EFI_DEVICE_PATH_PROTOCOL* TempDevicePathNode = PciDevicePathDynamicMulti;
UINT8 PciNodeCount = 0;
while (!IsDevicePathEnd(TempDevicePathNode)) {
  if ( (DevicePathType(TempDevicePathNode) == HARDWARE_DEVICE_PATH) && (DevicePathSubType(TempDevicePathNode) == HW_PCI_DP) )
    PciNodeCount++;
  TempDevicePathNode = NextDevicePathNode(TempDevicePathNode);
}
Print(L"Last device path has %d PCI nodes\n", PciNodeCount);
```
Here we've used `DevicePathType` and `DevicePathSubType` functions which are simple functions to access Node fields.

If you'll try this code you would see the same result that we've calculated earlier:
```
Last device path has 8 PCI nodes
```

# Create DevicePath/DeviceNode from text

All along this lesson we've converted device paths and nodes to text strings with a help of `ConvertDevicePathToText`/`ConvertDeviceNodeToText` functions.

But actually library represent an opposite way of things. We can initialize Device Path/Device node from the text strings. For this task library has `ConvertTextToDevicePath` and `ConvertTextToDeviceNode` functions.
```
/**
  Convert text to the binary representation of a device path.
  @param TextDevicePath  TextDevicePath points to the text representation of a device
                         path. Conversion starts with the first character and continues
                         until the first non-device node character.
  @return A pointer to the allocated device path or NULL if TextDeviceNode is NULL or
          there was insufficient memory.
**/
EFI_DEVICE_PATH_PROTOCOL *
EFIAPI
ConvertTextToDevicePath (
  IN CONST CHAR16  *TextDevicePath
  );
```
```
/**
  Convert text to the binary representation of a device node.
  @param TextDeviceNode  TextDeviceNode points to the text representation of a device
                         node. Conversion starts with the first character and continues
                         until the first non-device node character.
  @return A pointer to the EFI device node or NULL if TextDeviceNode is NULL or there was
          insufficient memory or text unsupported.
**/
EFI_DEVICE_PATH_PROTOCOL *
EFIAPI
ConvertTextToDeviceNode (
  IN CONST CHAR16  *TextDeviceNode
  );
```

We don't need to check UEFI specification, we've already know how our path looks in string representation, so let's use our knowledge:
```
EFI_DEVICE_PATH_PROTOCOL*  PciDevicePathNodeFromText = ConvertTextToDeviceNode(L"Pci(0x3,0x5)");
Print(L"PciDevicePathNodeFromText: %s\n", ConvertDeviceNodeToText((EFI_DEVICE_PATH_PROTOCOL*) PciDevicePathNodeFromText, FALSE, FALSE));
EFI_DEVICE_PATH_PROTOCOL*  PciDevicePathFromText = ConvertTextToDevicePath(L"Pci(0x3,0x5)");
Print(L"PciDevicePathFromText: %s\n", ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*) PciDevicePathFromText, FALSE, FALSE));
```
This would get us the expected result:
```
PciDevicePathNodeFromText: Pci(0x3,0x5)
PciDevicePathFromText: Pci(0x3,0x5)
```

# Don't forget to free allocated memory

This section is just a remainder that you shouldn't forget to free memory that was allocated for dynamically created paths and nodes:
```
FreePool(PciDevicePathNodeDynamic);
FreePool(PciDevicePathDynamic);
FreePool(PciDevicePathNodeFromText);
FreePool(PciDevicePathFromText);
```
