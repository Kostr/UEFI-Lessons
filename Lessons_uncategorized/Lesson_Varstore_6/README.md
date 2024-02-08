Earlier we've used `DEBUG` macros to investigate when `EFI_HII_CONFIG_ACCESS_PROTOCOL.CallBack()` function is called. It is a little cumbersome, since we had to build our driver in debug mode and constantly monitor OVMF log for messages. So let's learn how to create popup windows in edk2.

UEFI specification defines the following protocol for that:
```
EFI_HII_POPUP_PROTOCOL

Summary:
This protocol provides services to display a popup window.
The protocol is typically produced by the forms browser and consumed by a driver’s callback handler.

GUID
#define EFI_HII_POPUP_PROTOCOL_GUID  { 0x4311edc0, 0x6054, 0x46d4, { 0x9e, 0x40, 0x89, 0x3e, 0xa9, 0x52, 0xfc, 0xcc } }

Protocol Interface Structure:
typedef struct {
 UINT64 Revision;
 EFI_HII_CREATE_POPUP CreatePopup;
} EFI_HII_POPUP_PROTOCOL;

Parameters:
Revision      Protocol revision
CreatePopup   Displays a popup window
```

The `Revision` is just equal to 1 for now, the function that we need is `CreatePopup()`:
```
EFI_HII_POPUP_PROTOCOL.CreatePopup()

Summary:
Displays a popup window.

Prototype:
typedef
EFI_STATUS
(EFIAPI * EFI_HII_CREATE_POPUP) (
 IN EFI_HII_POPUP_PROTOCOL *This,
 IN EFI_HII_POPUP_STYLE PopupStyle,
 IN EFI_HII_POPUP_TYPE PopupType,
 EFI_HII_HANDLE HiiHandle
 IN EFI_STRING_ID Message,
 OUT EFI_HII_POPUP_SELECTION *UserSelection OPTIONAL,
);

Parameters:
This            A pointer to the EFI_HII_POPUP_PROTOCOL instance.
PopupStyle      Popup style to use
PopupType       Type of the popup to display
HiiHandle       HII handle of the string pack containing Message
Message         A message to display in the popup box.
UserSelection   User selection

Description:
The CreatePopup() function displays a modal message box that contains string specified by Message. Explicit line break characters can be used to specify a multi-line message. A popup window may contain user selectable options. The option selected by a user is returned via an optional UserSelection parameter.
```

In the nutshell this function creates a popup looking like this:
```
  +-------------------------------------------+
  |            ERROR/WARNING/INFO             |
  |-------------------------------------------|
  |              popup messages               |
  |                                           |
  |          user selectable options          |
  +-------------------------------------------+
```

There are 3 possible popup window styles (`PopupStyle` argument) - Info, Warning, Error:
```cpp
typedef enum {
 EfiHiiPopupStyleInfo,
 EfiHiiPopupStyleWarning,
 EfiHiiPopupStyleError
} EFI_HII_POPUP_STYLE;
```

And 4 possible popup window types (`PopupType` argument) - Ok, Ok-Cancel, Yes-No, Yes-No-Cancel:
```cpp
typedef enum {
 EfiHiiPopupTypeOk,
 EfiHiiPopupTypeOkCancel,
 EfiHiiPopupTypeYesNo,
 EfiHiiPopupTypeYesNoCancel
} EFI_HII_POPUP_TYPE;
```

Based on the option that user has selected, the return parameter `UserSelection` would have one of these values:
```cpp
typedef enum {
 EfiHiiPopupSelectionOk,
 EfiHiiPopupSelectionCancel,
 EfiHiiPopupSelectionYes,
 EfiHiiPopupSelectionNo
} EFI_HII_POPUP_SELECTION;
```

If you want to look at the function implementation check out [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/DisplayEngineDxe/Popup.c](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/DisplayEngineDxe/Popup.c).

Now let's transform our driver code to output popup instead of the `DEBUG` macro. We'll abstract all the functionality in the `HIIPopupCallbackInfo` function:
```cpp
STATIC
EFI_STATUS
EFIAPI
Callback (
  IIN     CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN     EFI_BROWSER_ACTION                     Action,
  IN     EFI_QUESTION_ID                        QuestionId,
  IN     UINT8                                  Type,
  IN OUT EFI_IFR_TYPE_VALUE                     *Value,
  OUT    EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  //DEBUG ((EFI_D_INFO, "Callback: Action=%s, QuestionId=0x%04x, Type=%s, Value=", ActionToStr(Action), QuestionId, TypeToStr(Type)));
  //DebugCallbackValue(Type, Value);

  HIIPopupCallbackInfo(Action, QuestionId, Type, Value);

  return EFI_UNSUPPORTED;
}
```

The trick with a separate `DEBUG` statement (hidden inside the `DebugCallbackValue`) for the `Value` printing will not work here, since we can pass exactly 1 string to the `EFI_HII_POPUP_PROTOCOL.CreatePopup()`. So let's write `CallbackValueToStr` that utilizes `UnicodeSPrint` function to return a string. Let's return it in an argument, so the caller would be responsible for the memory allocation/deallocation:
```cpp
VOID CallbackValueToStr(UINT8 Type, EFI_IFR_TYPE_VALUE *Value, EFI_STRING* ValueStr, UINTN ValueStrSize)
{
  switch (Type) {
    case EFI_IFR_TYPE_NUM_SIZE_8:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%d", Value->u8);
      break;
    case EFI_IFR_TYPE_NUM_SIZE_16:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%d", Value->u16);
      break;
    case EFI_IFR_TYPE_NUM_SIZE_32:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%d", Value->u32);
      break;
    case EFI_IFR_TYPE_NUM_SIZE_64:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%ld", Value->u64);
      break;
    case EFI_IFR_TYPE_BOOLEAN:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%d", Value->b);
      break;
    case EFI_IFR_TYPE_TIME:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%02d:%02d:%02d", Value->time.Hour, Value->time.Minute, Value->time.Second);
      break;
    case EFI_IFR_TYPE_DATE:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%04d/%02d/%02d", Value->date.Year, Value->date.Month, Value->date.Day);
      break;
    case EFI_IFR_TYPE_STRING:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"%s", HiiGetString(mHiiHandle, Value->string, "en-US"));
      break;
    default:
      UnicodeSPrint(*ValueStr, ValueStrSize, L"Unknown");
      break;
  }
}
```

Now to the `HIIPopupCallbackInfo`. First we need to locate the `EFI_HII_POPUP_PROTOCOL` protocol:
```cpp
EFI_STATUS Status;
EFI_HII_POPUP_PROTOCOL* HiiPopup;
Status = gBS->LocateProtocol(&gEfiHiiPopupProtocolGuid,
                             NULL,
                             (VOID **)&HiiPopup);
if (EFI_ERROR(Status)) {
  DEBUG ((EFI_D_ERROR, "Error! Can't find EFI_HII_POPUP_PROTOCOL\n"));
  return;
}
```
Don't forget to add the necessary include statement:
```
#include <Protocol/HiiPopup.h>
```
And add `gEfiHiiPopupProtocolGuid` to the driver INF file:
```
[Protocols]
  ...
  gEfiHiiPopupProtocolGuid
```

Now we need to create a string that would be printed in the popup message. Since the function accepts the argument in a form of `EFI_STRING_ID`, we need to add a string to our `Strings.uni` file:
```
#string POPUP_MESSAGE               #language en-US "Popup message"
```
Let's fill it dynamically with the `HiiSetString`:
```cpp
  UINTN Size = 300;
  EFI_STRING ValueStr = AllocateZeroPool(Size);
  CallbackValueToStr(Type, Value, &ValueStr, Size);
  EFI_STRING PopupStr = AllocateZeroPool(Size);
  UnicodeSPrint(PopupStr, Size, L"Callback:\nAction=%s\nQuestionId=0x%04x\nType=%s\nValue=%s", ActionToStr(Action), QuestionId, TypeToStr(Type), ValueStr);
  FreePool(ValueStr);
  HiiSetString(mHiiHandle, STRING_TOKEN(POPUP_MESSAGE), PopupStr, NULL);

  <...>

  FreePool(PopupStr);
```

All the arguments are in place, so we can call the `CreatePopup` function:
```cpp
  EFI_HII_POPUP_SELECTION UserSelection;
  Status = HiiPopup->CreatePopup(HiiPopup,
                                 EfiHiiPopupStyleInfo,
                                 EfiHiiPopupTypeOkCancel,
                                 mHiiHandle,
                                 STRING_TOKEN(POPUP_MESSAGE),
                                 &UserSelection
                                );
```

Here is a complete code for the `HIIPopupCallbackInfo`:
```cpp
VOID HIIPopupCallbackInfo(EFI_BROWSER_ACTION Action, EFI_QUESTION_ID QuestionId, UINT8 Type, EFI_IFR_TYPE_VALUE* Value)
{
  EFI_STATUS Status;
  EFI_HII_POPUP_PROTOCOL* HiiPopup;
  Status = gBS->LocateProtocol(&gEfiHiiPopupProtocolGuid,
                               NULL,
                               (VOID **)&HiiPopup);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "Error! Can't find EFI_HII_POPUP_PROTOCOL\n"));
    return;
  }

  UINTN Size = 300;
  EFI_STRING ValueStr = AllocateZeroPool(Size);
  CallbackValueToStr(Type, Value, &ValueStr, Size);
  EFI_STRING PopupStr = AllocateZeroPool(Size);
  UnicodeSPrint(PopupStr, Size, L"Callback:\nAction=%s\nQuestionId=0x%04x\nType=%s\nValue=%s", ActionToStr(Action), QuestionId, TypeToStr(Type), ValueStr);
  FreePool(ValueStr);
  HiiSetString(mHiiHandle, STRING_TOKEN(POPUP_OK_MESSAGE), PopupStr, NULL);

  EFI_HII_POPUP_SELECTION UserSelection;
  Status = HiiPopup->CreatePopup(HiiPopup,
                                 EfiHiiPopupStyleInfo,
                                 EfiHiiPopupTypeOk,
                                 mHiiHandle,
                                 STRING_TOKEN(POPUP_OK_MESSAGE),
                                 &UserSelection
                                );
  FreePool(PopupStr);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "Error! Can't create popup, %r\n", Status));
    return;
  }
}
```

Here we've used `EfiHiiPopupStyleInfo`/`EfiHiiPopupTypeOk` arguments to style our popup.

As you remember the first callback call that is executed in our case was:
```
Callback: Action=EFI_BROWSER_ACTION_FORM_OPEN, QuestionId=0x5555, Type=EFI_IFR_TYPE_BOOLEAN, Value=0
```
Here is how it looks now in a form of a popup message. As you can see it is printed by the browser before even drawing our form:
![popup1](popup1.png?raw=true "popup1")

There is not much different in style between the `EfiHiiPopupStyleInfo`/`EfiHiiPopupStyleWarning`/`EfiHiiPopupStyleError`. This just changes the popup header.

For example here is the same popup with a style `EfiHiiPopupStyleWarning`:
![popup2](popup2.png?raw=true "popup2")

And with the style `EfiHiiPopupStyleError`:
![popup3](popup3.png?raw=true "popup3")

Changing the popup type between the `EfiHiiPopupTypeOk`/`EfiHiiPopupTypeOkCancel`/`EfiHiiPopupTypeYesNo`/`EfiHiiPopupTypeYesNoCancel` modifies the user selection options in the bottom of the window. For example here is how `EfiHiiPopupTypeOkCancel` look like:
![popup4](popup4.png?raw=true "popup4")

I hope you can imagine how `EfiHiiPopupTypeYesNo`/`EfiHiiPopupTypeYesNoCancel` look like. In case you need to check what the option user pressed, you just check the return argument `EFI_HII_POPUP_SELECTION UserSelection`.


# `CreatePopUp` function from the `UefiLib.h`

Before the UEFI specification has introduced the `EFI_HII_POPUP_PROTOCOL.CreatePopup()` function, edk2 had its own out-of-spec function for popup creation defined in the [`UefiLib`](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/UefiLib.h) - `CreatePopUp`:

```cpp
/**
  Draws a dialog box to the console output device specified by
  ConOut defined in the EFI_SYSTEM_TABLE and waits for a keystroke
  from the console input device specified by ConIn defined in the
  EFI_SYSTEM_TABLE.

  If there are no strings in the variable argument list, then ASSERT().
  If all the strings in the variable argument list are empty, then ASSERT().

  @param[in]   Attribute  Specifies the foreground and background color of the popup.
  @param[out]  Key        A pointer to the EFI_KEY value of the key that was
                          pressed.  This is an optional parameter that may be NULL.
                          If it is NULL then no wait for a keypress will be performed.
  @param[in]  ...         The variable argument list that contains pointers to Null-
                          terminated Unicode strings to display in the dialog box.
                          The variable argument list is terminated by a NULL.

**/
VOID
EFIAPI
CreatePopUp (
  IN  UINTN          Attribute,
  OUT EFI_INPUT_KEY  *Key       OPTIONAL,
  ...
  )
```
If you are interested in the implementation you can find it [here](https://github.com/tianocore/edk2/blob/master/MdePkg/Library/UefiLib/Console.c).

So as you can see the function prototype is different. Because of that this function can be usefull in different scenarious. Since the `EFI_HII_POPUP_PROTOCOL.CreatePopup()` function is closely interlinked with the HII infrastructure, you need to have `EFI_STRING_ID` from some `EFI_HII_HANDLE` to call it. It gives you a possibility for multilanguage strings, but makes it hard to use in simple console `UEFI_APPLICATION`s.

On the other case this `CreatePopUp` function gives you different control for the popup window. There no user selector options, but you can monitor the exact key that user presses and have more control on the color style of the window.

For the color style check out the defines form the [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/SimpleTextOut.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/SimpleTextOut.h). There several options for the color:
```cpp
#define EFI_BLACK        0x00
#define EFI_BLUE         0x01
#define EFI_GREEN        0x02
#define EFI_CYAN         0x03
#define EFI_RED          0x04
#define EFI_MAGENTA      0x05
#define EFI_BROWN        0x06
#define EFI_LIGHTGRAY    0x07
#define EFI_BRIGHT       0x08
#define EFI_DARKGRAY     0x08
#define EFI_LIGHTBLUE    0x09
#define EFI_LIGHTGREEN   0x0A
#define EFI_LIGHTCYAN    0x0B
#define EFI_LIGHTRED     0x0C
#define EFI_LIGHTMAGENTA 0x0D
#define EFI_YELLOW       0x0E
#define EFI_WHITE        0x0F
```
You can create different foreground/backgroud color combinations with the:
```
#define EFI_TEXT_ATTR(Foreground, Background)  ((Foreground) | ((Background) << 4))
```

If you don't want to use the `EFI_TEXT_ATTR` macro, for some reason there are also separate defines for the background color:
```cpp
#define EFI_BACKGROUND_BLACK     0x00
#define EFI_BACKGROUND_BLUE      0x10
#define EFI_BACKGROUND_GREEN     0x20
#define EFI_BACKGROUND_CYAN      0x30
#define EFI_BACKGROUND_RED       0x40
#define EFI_BACKGROUND_MAGENTA   0x50
#define EFI_BACKGROUND_BROWN     0x60
#define EFI_BACKGROUND_LIGHTGRAY 0x70
```

So you can use `(EFI_WHITE | EFI_BACKGROUND_BLACK)` or `EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK)` and get the same result. `EFI_TEXT_ATTR` macro is not widely used in the edk2 code, I guess that can be connected to the fact that is not easy to remember where is foreground, and where is backgroud in the text like `EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK)`.

Anyway, let's try to use `CreatePopUp` function. Couple of notices:

- The function returns not when user presses ENTER, but when he presses any key. So if you want to react only to the ENTER key, you need to check returned value of the `EFI_INPUT_KEY` argument similar to this:
```cpp
  EFI_INPUT_KEY Key;
  ...
  do {
    CreatePopUp(..., &Key, ...);
  } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
```
- The function doesn't display correctly multiline strings with `\n` symbol inside. If you want a multiline popup message, you need to pass each string one by one ending with `NULL`.

With that in mind let's write a function `PopupCallbackInfo` that uses `CreatePopUp` call:
```cpp
VOID PopupCallbackInfo(EFI_BROWSER_ACTION Action, EFI_QUESTION_ID QuestionId, UINT8 Type, EFI_IFR_TYPE_VALUE* Value)
{
  EFI_INPUT_KEY Key;

  UINTN Size = 100;
  EFI_STRING ActionStr = AllocateZeroPool(Size);
  EFI_STRING QuestionIdStr = AllocateZeroPool(Size);
  EFI_STRING TypeStr = AllocateZeroPool(Size);
  EFI_STRING ValueStr = AllocateZeroPool(Size);
  EFI_STRING ValStr = AllocateZeroPool(Size);
  CallbackValueToStr(Type, Value, &ValStr, Size);
  UnicodeSPrint(ActionStr, Size, L"Action=%s", ActionToStr(Action));
  UnicodeSPrint(QuestionIdStr, Size, L"QuestionId=0x%04x", QuestionId);
  UnicodeSPrint(TypeStr, Size, L"Type=%s", TypeToStr(Type));
  UnicodeSPrint(ValueStr, Size, L"Value=%s", ValStr);
  do {
    CreatePopUp(EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLUE), &Key, L"Callback:", ActionStr, QuestionIdStr, TypeStr, ValueStr, NULL);
  } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);

  FreePool(ActionStr);
  FreePool(QuestionIdStr);
  FreePool(TypeStr);
  FreePool(ValueStr);
  FreePool(ValStr);
}
```
This is how a popup would look like in this case:
![popup5](popup5.png?raw=true "popup5")

You can compare the style of this popup with the window from the `EFI_HII_POPUP_PROTOCOL.CreatePopup()`.

With the `CreatePopUp` you can get wild with color. Here is window styled with the `EFI_TEXT_ATTR(EFI_RED, EFI_BLACK)`

![popup6](popup6.png?raw=true "popup6")

# Using popups in console applications.

If you really want to, you can use popups in console applications. Just be aware, that you need to clear the console yourself after each call, because on the other case the popup will be left on the screen even when you've exited the popup window.

Here is an example when application has issued two popup messages. The first one was created with the `EFI_HII_POPUP_PROTOCOL.CreatePopup()`, and the second one with the `CreatePopUp` call. As you can see they've overlayered each other, and the screen wasn't cleared even after the application has finished its execution.

![popup7](popup7.png?raw=true "popup7")

To clear the screen in such cases you can use:
```cpp
gST->ConOut->ClearScreen(gST->ConOut);
```
