Let's continue our investigation of the `EFI_HII_CONFIG_ACCESS_PROTOCOL.Callback` function.

In the last lesson we've researched all the input parameters of the function and understood when the function is called.

Now let's check the output parameter of the function - `OUT EFI_BROWSER_ACTION_REQUEST *ActionRequest`. According to the UEFI specification `Upon return, the callback function may specify the desired browser action`.

All the available values for the `ActionRequest` are listed in the file [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/FormBrowser2.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/FormBrowser2.h):
```
typedef UINTN EFI_BROWSER_ACTION_REQUEST;

#define EFI_BROWSER_ACTION_REQUEST_NONE               0
#define EFI_BROWSER_ACTION_REQUEST_RESET              1
#define EFI_BROWSER_ACTION_REQUEST_SUBMIT             2
#define EFI_BROWSER_ACTION_REQUEST_EXIT               3
#define EFI_BROWSER_ACTION_REQUEST_FORM_SUBMIT_EXIT   4
#define EFI_BROWSER_ACTION_REQUEST_FORM_DISCARD_EXIT  5
#define EFI_BROWSER_ACTION_REQUEST_FORM_APPLY         6
#define EFI_BROWSER_ACTION_REQUEST_FORM_DISCARD       7
#define EFI_BROWSER_ACTION_REQUEST_RECONNECT          8
#define EFI_BROWSER_ACTION_REQUEST_QUESTION_APPLY     9
```

According to the UEFI specification:
```
If the callback function returns with the ActionRequest set to:
 _NONE, then the Forms Browser will take no special behavior,
 _RESET, then the Forms Browser will exit and request the platform to reset,
 _SUBMIT, then the Forms Browser will save all modified question values to storage and exit,
 _EXIT, then the Forms Browser will discard all modified question values and exit,
 _FORM_SUBMIT_EXIT, then the Forms Browser will write all modified question values on the selected form to storage and then exit the selected form,
 _FORM_DISCARD_EXIT, then the Forms Browser will discard the modified question values on the selected form and then exit the selected form,
 _FORM_APPLY, then the Forms Browser will write all modified current question values on the selected form to storage,
 _FORM_DISCARD, then the Forms Browser will discard the current question values on the selected form and replace them with the original question values,
 _RECONNECT, a hardware and/or software configuration change was performed by the user, and the controller needs to be reconnected for the driver to recognize the change. The Forms Browser is required to call the EFI Boot Service DisconnectController() followed by the EFI Boot Service ConnectController() to reconnect the controller, and then exit. The controller handle passed to DisconnectController() and ConnectController() is the handle on which this EFI_HII_CONFIG_ACCESS_PROTOCOL is installed,
 _QUESTION_APPLY, then the Forms Browser will write the current modified question value on the selected form to storage.
```

In a short form this means:
```
_NONE              - nothing
_RESET             - exit and reset platform
_SUBMIT            - save all modifications and exit
_EXIT              - discard all modifications and exit
_FORM_SUBMIT_EXIT  - save current form modifications and exit form
_FORM_DISCARD_EXIT - discard current form modifications and exit form
_FORM_APPLY        - save current form modifications
_FORM_DISCARD      - discard current form modifications
_RECONNECT         - reconnect the controller and exit
_QUESTION_APPLY    - save current question modification
```

Let's try to see them in action. For that we would create `HIIFormCallbackDebug2` based on our recent `HIIFormCallbackDebug` driver.

Since the actual `ActionRequest` values are numbers between 0 and 9 we can test them via our `numeric` input. You already know that when the user changes the element value in the Form Browser, the following callbacks are called:
- `EFI_BROWSER_ACTION_CHANGING`
- `EFI_BROWSER_ACTION_CHANGED`

So let's set the `*ActionRequest` based on the user input in the `EFI_BROWSER_ACTION_CHANGED` callback for the `numeric`:
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
  //DEBUG ((EFI_D_INFO, "Callback: Action=%s, QuestionId=0x%04x, Type=%s, Value=", ActionToStr(Action), QuestionId, TypeToStr(Type)));
  //DebugCallbackValue(Type, Value);

  HIIPopupCallbackInfo(Action, QuestionId, Type, Value);

  if ((QuestionId == NUMERIC_QUESTION_ID) && (Action == EFI_BROWSER_ACTION_CHANGED)) {
    if ((Value->u16 >= 0) && (Value->u16 <= 9)) {
      *ActionRequest = Value->u16;
      return EFI_SUCCESS;
    }
  }

  return EFI_UNSUPPORTED;
}
```
This way when you set `numeric` to 4 for example, in the `EFI_BROWSER_ACTION_CHANGED` callback you would return `ActionRequest = 4 = EFI_BROWSER_ACTION_REQUEST_FORM_SUBMIT_EXIT`. Neat!

As you can see from the UEFI specification for the `ActionRequest` some actions close form, and some close entire formset. So to fully debug it we need to create a multiform formset.

Let's create 3 forms linked in the following way:
```
Form1 -> Form2 -> Form3
```
And move one of our interactive elements (for example `string`) to the Form2 and the rest of the input element leave in the most nested Form3.

Here is a snippet how it would look in the VFR:
```
formset

  <...>

  form
    formid = 1,
    title = STRING_TOKEN(FORMID1_TITLE);

    goto 2,
      prompt = STRING_TOKEN(GOTO_FORM2_PROMPT),
      help = STRING_TOKEN(GOTO_FORM2_HELP);

  endform;

  form
    formid = 2,
    title = STRING_TOKEN(FORMID2_TITLE);

    goto 3,
      prompt = STRING_TOKEN(GOTO_FORM3_PROMPT),
      help = STRING_TOKEN(GOTO_FORM3_HELP);

    string
      <...>
    endstring;

  endform;

 form
   formid = 3,
   title = STRING_TOKEN(FORMID3_TITLE);

   <... all other elements ... >

 endform;
endformset;
```

Off course don't forget to define the newly added strings:
```
#string FORMID1_TITLE          #language en-US  "Form 1"
#string FORMID2_TITLE          #language en-US  "Form 2"
#string FORMID3_TITLE          #language en-US  "Form 3"
#string GOTO_FORM2_PROMPT      #language en-US  "Enter Form 2"
#string GOTO_FORM2_HELP        #language en-US  "Enter Form 2"
#string GOTO_FORM3_PROMPT      #language en-US  "Enter Form 3"
#string GOTO_FORM3_HELP        #language en-US  "Enter Form 3"
```

Now you can experiment with the driver, by setting different values to the `numeric`.

First of all couple of new facts that we can obtain from the multiform formset:
- `Extract` is called once for all the elements when you enter the first form of the formset (Form1),
- Callbacks `FORM_OPEN`/`FORM_RETRIEVE`/`FORM_CLOSE` are called when you enter the from with the target element. I.e for `string` it will be Form2, for the rest of elements - Form3,
- You can leave the form with a changed value, but not a formset. When you try to leave formset the browser will ask you if you want to submit the changes,
- Even if the elements are on different forms the submit action (F10) would produce the callbacks for all the elements

Now back to the `ActionRequest` investigation. Here I've tried to describe addition actions that you would get from setting the `ActionRequest` (i.e. our `numeric`) to the particular value.
```
Changing to 0 - _NONE  - no additional actions

Changing to 1 - _RESET
 + ACTION_CHANGED    <--- revert uncommited changes for all elements in the formset
 + ACTION_FORM_CLOSE <--- close for all elements on the form
 + Close entire formset
 (Now if you try to exit browser it will prompt that the platform reset is needed)

Changing to 2 - _SUBMIT
 + Route with settings for all elements
 + ACTION_SUBMITTED  <--- submit for all elements in the formset
 + ACTION_FORM_CLOSE <--- close for all elements on the form
 + Close entire formset

Changing to 3 - _EXIT
 + ACTION_CHANGED    <--- revert uncommited changes for all elements in the formset
 + ACTION_FORM_CLOSE <--- close for all elements on the form
 + Close entire formset

Changing to 4 - _FORM_SUBMIT_EXIT
 + Route with settings for all elements
 + ACTION_SUBMITTED  <--- submit for all elements on the form
 + ACTION_FORM_CLOSE <--- close for all elements on the form
 + Close current form

Changing to 5 - _FORM_DISCARD_EXIT
 + ACTION_CHANGED    <--- revert uncommited changes for all elements on the form
 + ACTION_FORM_CLOSE <--- close for all elements on the form
 + Close current form

Changing to 6 - _FORM_APPLY
 + Route with settings for all elements
 + ACTION_SUBMITTED  <--- submit for all form elements

Changing to 7 - _FORM_DISCARD
 + ACTION_CHANGED    <--- revert uncommited changes for all form elements

Changing to 8 - _RECONNECT
 "Reconnect is required, confirm the changes then exit and reconnect"
 If user answers "N"
 + ACTION_CHANGED    <--- revert uncommited changes in thr formset
 + ACTION_FORM_CLOSE <--- close for all elements in the form
 + Close entire formset
 If user answers "Y"
 + Route with settings for all elements
 + ACTION_SUBMITTED  <--- submit for all elements in a formset
 + ACTION_FORM_CLOSE <--- close for all elements in the form
 + Reconnect
 + Close entire formset

Changing to 9 - _QUESTION_APPLY <--- no additional callbacks
 + Route with settings for target question only
```

I hope I didn't mess anything in the above description. Anyway now you have a `HIIFormCallbackDebug2` application to verify how the FormBrowser calls callbacks for the elements.
