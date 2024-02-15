In all our experments the driver `Callback()` function was never called.

This has happend because none of the driver form elements had `EFI_IFR_FLAG_CALLBACK` flag set.

The `EFI_IFR_FLAG_CALLBACK` is one of the flags for the `EFI_IFR_QUESTION_HEADER.flags` field [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiInternalFormRepresentation.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiInternalFormRepresentation.h):
```cpp
//
// Flag values of EFI_IFR_QUESTION_HEADER
//
#define EFI_IFR_FLAG_READ_ONLY           0x01
#define EFI_IFR_FLAG_CALLBACK            0x04
#define EFI_IFR_FLAG_RESET_REQUIRED      0x10
#define EFI_IFR_FLAG_REST_STYLE          0x20
#define EFI_IFR_FLAG_RECONNECT_REQUIRED  0x40
#define EFI_IFR_FLAG_OPTIONS_ONLY        0x80
```
The `EFI_IFR_QUESTION_HEADER` in turn is a structure present in the IFR code of all our data elements:
```
EFI_IFR_CHECKBOX     - checkbox
EFI_IFR_NUMERIC      - numeric
EFI_IFR_STRING       - string
EFI_IFR_DATE         - date
EFI_IFR_TIME         - time
EFI_IFR_ONE_OF       - oneof
EFI_IFR_ORDERED_LIST - orderedlist
```

So let's add `EFI_IFR_FLAG_CALLBACK` to one of our form elements. For example let's add it to the `numeric` element. In the VFR code the necessary flag is indicated by the `INTERACTIVE` keyword:
```
 numeric
   name = NumericQuestion,
   varid = FormData.NumericValue,
   prompt = STRING_TOKEN(NUMERIC_PROMPT),
   help = STRING_TOKEN(NUMERIC_HELP),
-  flags = NUMERIC_SIZE_2 | DISPLAY_UINT_HEX,
+  flags = NUMERIC_SIZE_2 | DISPLAY_UINT_HEX | INTERACTIVE,
   minimum = 0,
   maximum = 10,
   step = 1,
   default = 7, defaultstore = StandardDefault,
   default = 8, defaultstore = ManufactureDefault,
endnumeric;
```

# `EFI_HII_CONFIG_ACCESS_PROTOCOL.CallBack()`

Here is a description for the `EFI_HII_CONFIG_ACCESS_PROTOCOL.CallBack()` function from the UEFI specification:
```
EFI_HII_CONFIG_ACCESS_PROTOCOL.CallBack()

Summary:
This function is called to provide results data to the driver.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_HII_ACCESS_FORM_CALLBACK) (
 IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This,
 IN EFI_BROWSER_ACTION Action,
 IN EFI_QUESTION_ID QuestionId,
 IN UINT8 Type
 IN OUT EFI_IFR_TYPE_VALUE *Value,
 OUT EFI_BROWSER_ACTION_REQUEST *ActionRequest,
);

Parameters:
This		Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL
Action		Specifies the type of action taken by the browser
QuestionId	A unique value which is sent to the original exporting driver so that it can identify the
		type of data to expect. The format of the data tends to vary based on the opcode that generated the callback
Type		The type of value for the question
Value		A pointer to the data being sent to the original exporting driver. The type is specified by Type
ActionRequest	On return, points to the action requested by the callback function

Description
This function is called by the forms browser in response to a user action on a question which has the
EFI_IFR_FLAG_CALLBACK bit set in the EFI_IFR_QUESTION_HEADER. The user action is specified by
Action. Depending on the action, the browser may also pass the question value using Type and Value.
Upon return, the callback function may specify the desired browser action.
```

To better understand when and how this function is called let's add a debug statement that would print incoming arguments like we did with `RouteConfig()` and `ExtractConfig()`. But first let's try to understand what all these arguments mean.

## `EFI_BROWSER_ACTION Action`

This argument describes an operation that is currently performed by the browser. UEFI specification defines a list of possible browser operations with the `EFI_BROWSER_ACTION` type. The `EFI_BROWSER_ACTION` is `UINTN` with possible values described in the [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/HiiConfigAccess.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/HiiConfigAccess.h):
```cpp
typedef UINTN EFI_BROWSER_ACTION;

#define EFI_BROWSER_ACTION_CHANGING               0		// Called after the user have changed the element value, but before the browser updated display
#define EFI_BROWSER_ACTION_CHANGED                1		// Called after the user have changed the element value, after the browser updated display
#define EFI_BROWSER_ACTION_RETRIEVE               2		// Called after the browser has read the value, but before displayed it
#define EFI_BROWSER_ACTION_FORM_OPEN              3		// Called on form open before retrieve
#define EFI_BROWSER_ACTION_FORM_CLOSE             4		// Called on form exit
#define EFI_BROWSER_ACTION_SUBMITTED              5		// Called after submit
#define EFI_BROWSER_ACTION_DEFAULT_STANDARD       0x1000	// Called on setting standard default (default value is equal 0x0000)
#define EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING  0x1001	// Called on setting manufacture default (default value is equal 0x0001)
#define EFI_BROWSER_ACTION_DEFAULT_SAFE           0x1002	// Called on setting safe defaults (default value is equal 0x0002)
#define EFI_BROWSER_ACTION_DEFAULT_PLATFORM       0x2000	// Called on setting platform defaults (default values in range 0x4000-0x7fff)
#define EFI_BROWSER_ACTION_DEFAULT_HARDWARE       0x3000	// Called on setting hardware defaults (default values in range 0x8000-0xbfff)
#define EFI_BROWSER_ACTION_DEFAULT_FIRMWARE       0x4000	// Called on setting firmware defaults (default values in range 0xc000-0xffff)
```
On each of these operations the Form Browser calls `Callback()` function for each of the form elements with the `EFI_IFR_FLAG_CALLBACK` flag.

To print `EFI_BROWSER_ACTION` value as a string we'll define this simple function:
```cpp
EFI_STRING ActionToStr(EFI_BROWSER_ACTION Action)
{
  switch (Action) {
    case EFI_BROWSER_ACTION_CHANGING:
      return L"EFI_BROWSER_ACTION_CHANGING";
    case EFI_BROWSER_ACTION_CHANGED:
      return L"EFI_BROWSER_ACTION_CHANGED";
    case EFI_BROWSER_ACTION_RETRIEVE:
      return L"EFI_BROWSER_ACTION_RETRIEVE";
    case EFI_BROWSER_ACTION_FORM_OPEN:
      return L"EFI_BROWSER_ACTION_FORM_OPEN";
    case EFI_BROWSER_ACTION_FORM_CLOSE:
      return L"EFI_BROWSER_ACTION_FORM_CLOSE";
    case EFI_BROWSER_ACTION_SUBMITTED:
      return L"EFI_BROWSER_ACTION_SUBMITTED";
    case EFI_BROWSER_ACTION_DEFAULT_STANDARD:
      return L"EFI_BROWSER_ACTION_DEFAULT_STANDARD";
    case EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING:
      return L"EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING";
    case EFI_BROWSER_ACTION_DEFAULT_SAFE:
      return L"EFI_BROWSER_ACTION_DEFAULT_SAFE";
    case EFI_BROWSER_ACTION_DEFAULT_PLATFORM:
      return L"EFI_BROWSER_ACTION_DEFAULT_PLATFORM";
    case EFI_BROWSER_ACTION_DEFAULT_HARDWARE:
      return L"EFI_BROWSER_ACTION_DEFAULT_HARDWARE";
    case EFI_BROWSER_ACTION_DEFAULT_FIRMWARE:
      return L"EFI_BROWSER_ACTION_DEFAULT_FIRMWARE";
    default:
      return L"Unknown";
  }
}
```

## `EFI_QUESTION_ID QuestionId`

The `EFI_QUESTION_ID` type is defined in the [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiInternalFormRepresentation.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiInternalFormRepresentation.h)
```
typedef UINT16  EFI_QUESTION_ID;
```
It is just a `UINT16` value identificator to indicate for which of the form elements the `Callback()` function is called.

It is possible to explicitly set values for question id's of your form elements with the VFR `questionid` keyword. If you don't do it, the `VfrCompiler` will implicitly assign unique question identifiers to all the form elements without it. The numbering in this case starts from `0x0001`.

## `UINT8 Type` and `EFI_IFR_TYPE_VALUE *Value`

The Form Browser also sends an actual form element data to the `Callback()` function.

But each element store the data in it's own type. Therefore the Form Browser in one argument sends the type of value [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiInternalFormRepresentation.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiInternalFormRepresentation.h):
```
//
// Types of the option's value.
//
#define EFI_IFR_TYPE_NUM_SIZE_8   0x00
#define EFI_IFR_TYPE_NUM_SIZE_16  0x01
#define EFI_IFR_TYPE_NUM_SIZE_32  0x02
#define EFI_IFR_TYPE_NUM_SIZE_64  0x03
#define EFI_IFR_TYPE_BOOLEAN      0x04
#define EFI_IFR_TYPE_TIME         0x05
#define EFI_IFR_TYPE_DATE         0x06
#define EFI_IFR_TYPE_STRING       0x07
#define EFI_IFR_TYPE_OTHER        0x08
#define EFI_IFR_TYPE_UNDEFINED    0x09
#define EFI_IFR_TYPE_ACTION       0x0A
#define EFI_IFR_TYPE_BUFFER       0x0B
#define EFI_IFR_TYPE_REF          0x0C
```
And in in another argument it sends the actual value as a union type:
```
typedef union {
  UINT8            u8;
  UINT16           u16;
  UINT32           u32;
  UINT64           u64;
  BOOLEAN          b;
  EFI_HII_TIME     time;
  EFI_HII_DATE     date;
  EFI_STRING_ID    string; ///< EFI_IFR_TYPE_STRING, EFI_IFR_TYPE_ACTION
  EFI_HII_REF      ref;    ///< EFI_IFR_TYPE_REF
  // UINT8 buffer[];      ///< EFI_IFR_TYPE_BUFFER
} EFI_IFR_TYPE_VALUE;
```

Therefore to get the actual value we first look at the `UINT8 Type` argument and then use the necessary type on the `EFI_IFR_TYPE_VALUE` union. Below I've written two functions: `TypeToStr` simply prints a type of the callback element, and `DebugCallbackValue` prints an element value based on it's type:
```cpp
EFI_STRING TypeToStr(UINT8 Type)
{
  switch (Type) {
    case EFI_IFR_TYPE_NUM_SIZE_8:
      return L"EFI_IFR_TYPE_NUM_SIZE_8";
    case EFI_IFR_TYPE_NUM_SIZE_16:
      return L"EFI_IFR_TYPE_NUM_SIZE_16";
    case EFI_IFR_TYPE_NUM_SIZE_32:
      return L"EFI_IFR_TYPE_NUM_SIZE_32";
    case EFI_IFR_TYPE_NUM_SIZE_64:
      return L"EFI_IFR_TYPE_NUM_SIZE_64";
    case EFI_IFR_TYPE_BOOLEAN:
      return L"EFI_IFR_TYPE_BOOLEAN";
    case EFI_IFR_TYPE_TIME:
      return L"EFI_IFR_TYPE_TIME";
    case EFI_IFR_TYPE_DATE:
      return L"EFI_IFR_TYPE_DATE";
    case EFI_IFR_TYPE_STRING:
      return L"EFI_IFR_TYPE_STRING";
    case EFI_IFR_TYPE_OTHER:
      return L"EFI_IFR_TYPE_OTHER";
    case EFI_IFR_TYPE_UNDEFINED:
      return L"EFI_IFR_TYPE_UNDEFINED";
    case EFI_IFR_TYPE_ACTION:
      return L"EFI_IFR_TYPE_ACTION";
    case EFI_IFR_TYPE_BUFFER:
      return L"EFI_IFR_TYPE_BUFFER";
    case EFI_IFR_TYPE_REF:
      return L"EFI_IFR_TYPE_REF";
    default:
      return L"Unknown";
  }
}

VOID DebugCallbackValue(UINT8 Type, EFI_IFR_TYPE_VALUE *Value)
{
  switch (Type) {
    case EFI_IFR_TYPE_NUM_SIZE_8:
      DEBUG ((EFI_D_INFO, "%d\n", Value->u8));
      break;
    case EFI_IFR_TYPE_NUM_SIZE_16:
      DEBUG ((EFI_D_INFO, "%d\n", Value->u16));
      break;
    case EFI_IFR_TYPE_NUM_SIZE_32:
      DEBUG ((EFI_D_INFO, "%d\n", Value->u32));
      break;
    case EFI_IFR_TYPE_NUM_SIZE_64:
      DEBUG ((EFI_D_INFO, "%ld\n", Value->u64));
      break;
    case EFI_IFR_TYPE_BOOLEAN:
      DEBUG ((EFI_D_INFO, "%d\n", Value->b));
      break;
    case EFI_IFR_TYPE_TIME:
      DEBUG ((EFI_D_INFO, "%02d:%02d:%02d\n", Value->time.Hour, Value->time.Minute, Value->time.Second));
      break;
    case EFI_IFR_TYPE_DATE:
      DEBUG ((EFI_D_INFO, "%04d/%02d/%02d\n", Value->date.Year, Value->date.Month, Value->date.Day));
      break;
    case EFI_IFR_TYPE_STRING:
      if (Value->string)
        DEBUG ((EFI_D_INFO, "%s\n", HiiGetString(mHiiHandle, Value->string, "en-US") ));
      else
        DEBUG ((EFI_D_INFO, "NO STRING!\n" ));
      break;
    default:
      DEBUG ((EFI_D_INFO, "Unknown\n" ));
      break;
  }
}
```

# Putting it all together

Let's get all of our helper functions and create a debug print statement for the `Callback()` code:
```cpp
STATIC
EFI_STATUS
EFIAPI
Callback (
  IN     CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN     EFI_BROWSER_ACTION                     Action,
  IN     EFI_QUESTION_ID                        QuestionId,
  IN     UINT8                                  Type,
  IN OUT EFI_IFR_TYPE_VALUE                     *Value,
  OUT    EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  DEBUG ((EFI_D_INFO, "Callback: Action=%s, QuestionId=0x%04x, Type=%s, Value=", ActionToStr(Action), QuestionId, TypeToStr(Type)));
  DebugCallbackValue(Type, Value);

  return EFI_UNSUPPORTED;
}
```

Besides the `Callback()` let's add the `DEBUG` statements to the `RouteConfig()` and `ExtractConfig()` functions as well. This way we will know the order of how the Form Browser calls our functions:
```
STATIC
EFI_STATUS
EFIAPI
RouteConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Configuration,
  OUT       EFI_STRING                      *Progress
)
{
  DEBUG ((EFI_D_INFO, "RouteConfig: Configuration=%s\n", Configuration));

  ...
}

STATIC
EFI_STATUS
EFIAPI
ExtractConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Request,
  OUT       EFI_STRING                      *Progress,
  OUT       EFI_STRING                      *Results
)
{
  DEBUG ((EFI_D_INFO, "ExtractConfig: Request=%s\n", Request));

  ...
}
```

Now it is time to test our driver. Build our driver, copy it to the shared folder and run QEMU with debug log. For the last operation you can use the first stage of the `run_gdb_ovmf.sh` script:
```
./run_gdb_ovmf.sh -1
```

Load our driver to the UEFI shell.
```
FS0:\> load HIIFormDataElementsVarstore.efi
```

The `Callback()` code is run only by the Form Browser. Therefore if we would issue requests with our `HIIConfig.efi` application, the `Callback()` would never run. So let's type `exit` to run the Form Browser.

When we enter our form, the following strings would be printed to the debug log (by default the value for the numeric element is 8):
```
ExtractConfig: Request=GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=0&WIDTH=0024
Callback: Action=EFI_BROWSER_ACTION_FORM_OPEN, QuestionId=0x0002, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=0
Callback: Action=EFI_BROWSER_ACTION_RETRIEVE, QuestionId=0x0002, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=8
```
As you see here our element got `QuestionId=0x0002`. This is because we didn't explicitly set any questionId's for our form elements and the `numeric` element is second on the form (the first is the `checkbox` element). We've set `NUMERIC_SIZE_2` flag in the element VFR code, therefore the value is interpreted as `Value->u16`.

Change the numeric element value to 3:
```
Callback: Action=EFI_BROWSER_ACTION_CHANGING, QuestionId=0x0002, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=3
Callback: Action=EFI_BROWSER_ACTION_CHANGED, QuestionId=0x0002, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=3
```
As you know, in this case the Form Browser doesn't call `ExtractConfig`/`RouteConfig` functions. But as you see you still have control in that case since the `Callback()` code is called.

Submit the form with updated value:
```
RouteConfig: Configuration=GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=0000&WIDTH=0001&VALUE=00&OFFSET=0001&WIDTH=0002&VALUE=0003&OFFSET=0003&WIDTH=0014&VALUE=006f0072007000200067006e0069007200740053&OFFSET=0019&WIDTH=0004&VALUE=160507e5&OFFSET=001d&WIDTH=0003&VALUE=213717&OFFSET=0020&WIDTH=0001&VALUE=33&OFFSET=0021&WIDTH=0003&VALUE=0a0b0c
Callback: Action=EFI_BROWSER_ACTION_SUBMITTED, QuestionId=0x0002, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=3
```

Press reset to default button (this sets value to 7):
```
ExtractConfig: Request=GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=0000&WIDTH=0001&OFFSET=0001&WIDTH=0002&OFFSET=0003&WIDTH=0014&OFFSET=0019&WIDTH=0004&OFFSET=001d&WIDTH=0003&OFFSET=0020&WIDTH=0001&OFFSET=0021&WIDTH=0003
Callback: Action=EFI_BROWSER_ACTION_DEFAULT_STANDARD, QuestionId=0x0002, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=3
```

Press reset to manufacture default button (this sets value to 8):
```
ExtractConfig: Request=GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=0000&WIDTH=0001&OFFSET=0001&WIDTH=0002&OFFSET=0003&WIDTH=0014&OFFSET=0019&WIDTH=0004&OFFSET=001d&WIDTH=0003&OFFSET=0020&WIDTH=0001&OFFSET=0021&WIDTH=0003
Callback: Action=EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING, QuestionId=0x0002, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=7
```

Exit without submit
```
Callback: Action=EFI_BROWSER_ACTION_CHANGED, QuestionId=0x0002, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=3
Callback: Action=EFI_BROWSER_ACTION_FORM_CLOSE, QuestionId=0x0002, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=3
```
Here you see that 2 callbacks were executed: `EFI_BROWSER_ACTION_CHANGED` and `EFI_BROWSER_ACTION_FORM_CLOSE`. As another experiment enter the form again, set manufacture defaults and submit them. And only then exit our form. This way the exit action would print only the `EFI_BROWSER_ACTION_FORM_CLOSE` callback:
```
Callback: Action=EFI_BROWSER_ACTION_FORM_CLOSE, QuestionId=0x0002, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=8
```
So the `EFI_BROWSER_ACTION_CHANGED` in this case is called only if an element has unsubmitted changes.


# `QuestionId` and the order of callbacks

In the last example the `QuestionId` for our element was set implicitly by the `VfrCompiler`. But as we've mentioned it earlier it is possible to set it yourself with a help of a `quiestionid` keyword.

For another experiment let's add `INTERACTIVE` flag to the `checkbox` and `string` elements. But set the `quiestionid` explicitly only for the `checkbox` and `numeric` elements:
```
...

checkbox
  varid = FormData.CheckboxValue,
  questionid = 0x5555,
  prompt = STRING_TOKEN(CHECKBOX_PROMPT),
  help = STRING_TOKEN(CHECKBOX_HELP),
  flags = INTERACTIVE,
  default = TRUE, defaultstore = StandardDefault,
  default = FALSE, defaultstore = ManufactureDefault,
endcheckbox;

numeric
  name = NumericQuestion,
  varid = FormData.NumericValue,
  questionid = 0x4444,
  prompt = STRING_TOKEN(NUMERIC_PROMPT),
  help = STRING_TOKEN(NUMERIC_HELP),
  flags = NUMERIC_SIZE_2 | DISPLAY_UINT_HEX | INTERACTIVE,
  minimum = 0,
  maximum = 10,
  step = 1,
  default = 7, defaultstore = StandardDefault,
  default = 8, defaultstore = ManufactureDefault,
endnumeric;

string
  name = StringQuestion,
  varid = FormData.StringValue,
  prompt = STRING_TOKEN(STRING_PROMPT),
  help = STRING_TOKEN(STRING_HELP),
  flags = INTERACTIVE,
  minsize = 5,
  maxsize = 10,
  default = STRING_TOKEN(STRING_DEFAULT), defaultstore = StandardDefault,
  default = STRING_TOKEN(STRING_PROMPT), defaultstore = ManufactureDefault,
endstring

...
```

This would give us this output on the form open:
```
ExtractConfig: Request=GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=0&WIDTH=0024
Callback: Action=EFI_BROWSER_ACTION_FORM_OPEN, QuestionId=0x5555, Type=EFI_IFR_TYPE_BOOLEAN, Value=0
Callback: Action=EFI_BROWSER_ACTION_FORM_OPEN, QuestionId=0x4444, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=0
Callback: Action=EFI_BROWSER_ACTION_FORM_OPEN, QuestionId=0x0001, Type=EFI_IFR_TYPE_STRING, Value=
Callback: Action=EFI_BROWSER_ACTION_RETRIEVE, QuestionId=0x5555, Type=EFI_IFR_TYPE_BOOLEAN, Value=0
Callback: Action=EFI_BROWSER_ACTION_RETRIEVE, QuestionId=0x4444, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=8
Callback: Action=EFI_BROWSER_ACTION_RETRIEVE, QuestionId=0x0001, Type=EFI_IFR_TYPE_STRING, Value=String pro
```
As you see here each action is called on every interactive form element. This is true for all other cases when the action affects all the form elements. This includes all the actions except `EFI_BROWSER_ACTION_CHANGING` and `EFI_BROWSER_ACTION_CHANGED`.

Another thing to point out is that the calls in the output above are happening not by the order of `QuestionId`'s, but in the order of element placement in the VFR. Also as we've said it before, the `VfrCompiler` implicitly assigns `QuestionId`'s to all the form elements which don't have any explicit setting for that. Even to those which don't have any `INTERACTIVE` flag. The implicit assignment starts from the `0x0001` value, and this is the value that our `string` element got.

Now if we change the `numeric` element value to 3, only `numeric` callback will be executed:
```
Callback: Action=EFI_BROWSER_ACTION_CHANGING, QuestionId=0x4444, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=3
Callback: Action=EFI_BROWSER_ACTION_CHANGED, QuestionId=0x4444, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=3
```
And if we exit the form without any submit, the following callbacks will be called:
```
Callback: Action=EFI_BROWSER_ACTION_CHANGED, QuestionId=0x4444, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=8
Callback: Action=EFI_BROWSER_ACTION_FORM_CLOSE, QuestionId=0x5555, Type=EFI_IFR_TYPE_BOOLEAN, Value=0
Callback: Action=EFI_BROWSER_ACTION_FORM_CLOSE, QuestionId=0x4444, Type=EFI_IFR_TYPE_NUM_SIZE_16, Value=8
Callback: Action=EFI_BROWSER_ACTION_FORM_CLOSE, QuestionId=0x0001, Type=EFI_IFR_TYPE_STRING, Value=String pro
```

I hope all of these experiments have got you some understanding about when and how the `Callback()` function is executed.

To finish this lesson here is a logic diagram for an individual numeric element with the current value of 8:
```
  Open form (even before the form is displayed)
EFI_BROWSER_ACTION_FORM_OPEN (Value=0)
EFI_BROWSER_ACTION_RETRIEVE  (Value=8)
    |
    -------------------------------------------------
    |                                               |
  Change value from 8 to 8                        Close form
EFI_BROWSER_ACTION_CHANGING   (Value=8)         EFI_BROWSER_ACTION_FORM_CLOSE (Value=8)
    |
    -------------------------------------------------
    |                                               |
  Change value from 8 to 9                        Close form
EFI_BROWSER_ACTION_CHANGING   (Value=9)         EFI_BROWSER_ACTION_FORM_CLOSE (Value=8)
EFI_BROWSER_ACTION_CHANGED    (Value=9)
    |
    -------------------------------------------------------------------------------------------------
    |                                               |                                               |
  Submit                                          Close form with sumbit                          Close form without submit
EFI_BROWSER_ACTION_SUBMITTED  (Value=9)         EFI_BROWSER_ACTION_SUBMITTED  (Value=9)         EFI_BROWSER_ACTION_CHANGED    (Value=8)
    |                                           EFI_BROWSER_ACTION_FORM_CLOSE (Value=9)         EFI_BROWSER_ACTION_FORM_CLOSE (Value=8)
  Close
EFI_BROWSER_ACTION_FORM_CLOSE (Value=9)
```
