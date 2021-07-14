When we were investigating `GetNextVariableName()` UEFI service and all the NVRAM variables in the system, we've found out that under `EFI_GLOBAL_VARIABLE GUID` (`gEfiGlobalVariableGuid`)
 these variables are present: 
```
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: Key0000
8BE4DF61-93CA-11D2-AA0D-00E098032B8C: Key0001
```

You can read UEFI specification or edk2 file https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Guid/GlobalVariable.h to find help for these options:
```
// L"Key####"       - Describes hot key relationship with a Boot#### load option
```

OVMF sets these options in a file https://github.com/tianocore/edk2/blob/master/OvmfPkg/Library/PlatformBootManagerLib/BdsPlatform.c:
```
VOID
PlatformRegisterOptionsAndKeys (
  VOID
  )
{
  ...

  //
  // Map F2 to Boot Manager Menu
  //
  F2.ScanCode     = SCAN_F2;
  F2.UnicodeChar  = CHAR_NULL;
  Esc.ScanCode    = SCAN_ESC;
  Esc.UnicodeChar = CHAR_NULL;
  Status = EfiBootManagerGetBootManagerMenu (&BootOption);
  ASSERT_EFI_ERROR (Status);
  Status = EfiBootManagerAddKeyOptionVariable (
             NULL, (UINT16) BootOption.OptionNumber, 0, &F2, NULL
             );
  ASSERT (Status == EFI_SUCCESS || Status == EFI_ALREADY_STARTED);
  Status = EfiBootManagerAddKeyOptionVariable (
             NULL, (UINT16) BootOption.OptionNumber, 0, &Esc, NULL
             );
  ...
}
```
`EfiBootManagerAddKeyOptionVariable` code sets `F2` and `ESC` keystrokes as hotkeys for the boot manager menu entry. And adds NVRAM variables `KeyXXXX` with all the necessary info.

https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiBootManagerLib/BmHotkey.c:
```
/**
  Add the key option.
  It adds the key option variable and the key option takes affect immediately.
  @param AddedOption      Return the added key option.
  @param BootOptionNumber The boot option number for the key option.
  @param Modifier         Key shift state.
  @param ...              Parameter list of pointer of EFI_INPUT_KEY.
  @retval EFI_SUCCESS         The key option is added.
  @retval EFI_ALREADY_STARTED The hot key is already used by certain key option.
**/
EFI_STATUS
EFIAPI
EfiBootManagerAddKeyOptionVariable (
  OUT EFI_BOOT_MANAGER_KEY_OPTION *AddedOption,   OPTIONAL
  IN UINT16                       BootOptionNumber,
  IN UINT32                       Modifier,
  ...
  )
{
  EFI_BOOT_MANAGER_KEY_OPTION    KeyOption;
  ...
  UnicodeSPrint (KeyOptionName, sizeof (KeyOptionName), L"Key%04x", KeyOptionNumber);

  Status = gRT->SetVariable (                                                              //  <------ this call sets 'KeyXXXX' variable
                  KeyOptionName,
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  BmSizeOfKeyOption (&KeyOption),
                  &KeyOption
                  );
  if (!EFI_ERROR (Status)) {
    ...
    if (mBmHotkeyServiceStarted) {
      BmProcessKeyOption (&KeyOption);			                                   // <---- this function calls 'RegisterKeyNotify'
    }
  }

  return Status;
}
```

To get an understanding about how `EFI_BOOT_MANAGER_KEY_OPTION` is coded, take a look at its definition in https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Include/Library/UefiBootManagerLib.h
```
#pragma pack(1)
///
/// EFI Key Option.
///
typedef struct {
  ///
  /// Specifies options about how the key will be processed.
  ///
  EFI_BOOT_KEY_DATA  KeyData;
  ///
  /// The CRC-32 which should match the CRC-32 of the entire EFI_LOAD_OPTION to
  /// which BootOption refers. If the CRC-32s do not match this value, then this key
  /// option is ignored.
  ///
  UINT32             BootOptionCrc;
  ///
  /// The Boot#### option which will be invoked if this key is pressed and the boot option
  /// is active (LOAD_OPTION_ACTIVE is set).
  ///
  UINT16             BootOption;
  ///
  /// The key codes to compare against those returned by the
  /// EFI_SIMPLE_TEXT_INPUT and EFI_SIMPLE_TEXT_INPUT_EX protocols.
  /// The number of key codes (0-3) is specified by the EFI_KEY_CODE_COUNT field in KeyOptions.
  ///
  EFI_INPUT_KEY      Keys[3];
  UINT16             OptionNumber;
} EFI_BOOT_MANAGER_KEY_OPTION;
#pragma pack()
```


If you'll look at the `BmSizeOfKeyOption` function (https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiBootManagerLib/BmHotkey.c) that was used in a `gRT->SetVariable` call above:
```
UINTN
BmSizeOfKeyOption (
  IN CONST EFI_BOOT_MANAGER_KEY_OPTION  *KeyOption
  )
{
  return OFFSET_OF (EFI_BOOT_MANAGER_KEY_OPTION, Keys)
    + KeyOption->KeyData.Options.InputKeyCount * sizeof (EFI_INPUT_KEY);
}
```
you will see that `OptionNumber` field of `EFI_BOOT_MANAGER_KEY_OPTION` is not stored in NVRAM and `Keys` array size is variable.


As for other subtypes:
- `EFI_BOOT_KEY_DATA` is defined in https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiSpec.h
```
///
/// EFI Boot Key Data
///
typedef union {
  struct {
    ///
    /// Indicates the revision of the EFI_KEY_OPTION structure. This revision level should be 0.
    ///
    UINT32  Revision        : 8;
    ///
    /// Either the left or right Shift keys must be pressed (1) or must not be pressed (0).
    ///
    UINT32  ShiftPressed    : 1;
    ///
    /// Either the left or right Control keys must be pressed (1) or must not be pressed (0).
    ///
    UINT32  ControlPressed  : 1;
    ///
    /// Either the left or right Alt keys must be pressed (1) or must not be pressed (0).
    ///
    UINT32  AltPressed      : 1;
    ///
    /// Either the left or right Logo keys must be pressed (1) or must not be pressed (0).
    ///
    UINT32  LogoPressed     : 1;
    ///
    /// The Menu key must be pressed (1) or must not be pressed (0).
    ///
    UINT32  MenuPressed     : 1;
    ///
    /// The SysReq key must be pressed (1) or must not be pressed (0).
    ///
    UINT32  SysReqPressed    : 1;
    UINT32  Reserved        : 16;
    ///
    /// Specifies the actual number of entries in EFI_KEY_OPTION.Keys, from 0-3. If
    /// zero, then only the shift state is considered. If more than one, then the boot option will
    /// only be launched if all of the specified keys are pressed with the same shift state.
    ///
    UINT32  InputKeyCount   : 2;
  } Options;
  UINT32  PackedValue;
} EFI_BOOT_KEY_DATA;
```
- `EFI_INPUT_KEY` is defined in https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/SimpleTextIn.h
```
typedef struct {
  UINT16  ScanCode;
  CHAR16  UnicodeChar;
} EFI_INPUT_KEY;
```

With all this information in mind we can call `dmpstore` command in our shell and parse `KeyXXXX` options.

![dmpstore](dmpstore.png?raw=true "dmpstore")

As you can see `Key0000` defines hot key with a `0x000c` scan code for the `Boot0000` option. And `Key0001` defines hot key with a `0x0017` scan code for the same `Boot0000` option.
You can also see that `Boot0000` stands for the `UiApp` which happens to be a boot menu.


And according to the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/SimpleTextIn.h
```
#define SCAN_F2         0x000C
...
#define SCAN_ESC        0x0017
```

So you can use `F2` and `ESC` keys to stop the boot process and go to the boot menu.

If you want to know more about this HotKey functionality implementation take a look at a callback function `BmHotkeyCallback` and a `mBmHotkeyBootOption` variable that it sets (https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Library/UefiBootManagerLib/BmHotkey.c)

One more notice. As you might remember if we launch QEMU with `-nographic` option, every non-standard key is transmitted through the escape sequence, which starts with the `SCAN_ESC` symbol. So don't be surprised, when every non-standard key would act as a HotKey in this case.

