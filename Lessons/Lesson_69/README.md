In this lesson we will look at several built-in VFR conditionals.

Before we go deep into this new keywords, let's look one more time at our form:

![Before](Before.png?raw=true "Before")

Remember it as a reference as we would change the look of our form in this lesson.

Also each of the built-in`s in this lesson work with a conditional statement. But we want to show how the element works, so we would simply use a constant `TRUE` in the place of a conditional.

# suppressif

The suppress tag causes the nested objects to be hidden from the user if the expression appearing as the
first nested object evaluates to TRUE.

Opcode for this operation is:
```
#define EFI_IFR_SUPPRESS_IF_OP 0x0a
```

VFR token definition can be found under a link https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.7.3-vfr-statement-suppressif-definition

As an example let's suppress our `orderedlist` element:
```
suppressif TRUE;
    orderedlist
      ...
    endlist;
endif;
```

After that the form would look like this:

![SuppressIf](SuppressIf.png?raw=true "SuppressIf")

One more use of `suppressif` is in the fact that you can hide options inside the question.

For example currently our `oneof` question looks like:

![SuppressIf_oneof1](SuppressIf_oneof1.png?raw=true "SuppressIf_oneof1")

But adding `suppressif` like this:
```
oneof
  varid = FormData.OneOfValue,
  prompt = STRING_TOKEN(ONEOF_PROMPT),
  help = STRING_TOKEN(ONEOF_HELP),
  option text = STRING_TOKEN(ONEOF_OPTION1), value = 0x00, flags = 0;
  option text = STRING_TOKEN(ONEOF_OPTION2), value = 0x33, flags = 0;
  suppressif TRUE;
  option text = STRING_TOKEN(ONEOF_OPTION3), value = 0x55, flags = 0;
  endif;
endoneof;
```

Would hide the option 3 for the user:

![SuppressIf_oneof2](SuppressIf_oneof2.png?raw=true "SuppressIf_oneof2")

# grayoutif

All nested statements or questions will be grayed out (not selectable and visually distinct) if the expression appearing as the first nested object evaluates to TRUE.

Opcode for this operation is:
```
#define EFI_IFR_GRAY_OUT_IF_OP 0x19
```

VFR token definition can be found under a link https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.7.4-vfr-statement-grayoutif-definition


As an example let's grayout our `orderedlist` element:
```
grayoutif TRUE;
    orderedlist
      ...
    endlist;
endif;
```

As a result the element becomes unselectable like subtitle:

![GrayoutIf](GrayoutIf.png?raw=true "GrayoutIf")

# disableif

All nested statements, questions, options or expressions will not be processed if the expression appearing as the first nested object evaluates to TRUE.

Opcode for this operation is:
```
#define EFI_IFR_DISABLE_IF_OP 0x1E
```

VFR token definition can be found under a link https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.7.2-vfr-statement-disalbeif-definition

As an example let's disable our `orderedlist` element:
```
disableif TRUE;
    orderedlist
      ...
    endlist;
endif
```

With this condition our form looks like in a `suppressif` example, the element is simply not present in the view:

![DisableIf](DisableIf.png?raw=true "DisableIf")

# warningif

Creates a validation expression and warning message for a question.

Opcode for this operation is:
```
#define EFI_IFR_WARNING_IF_OP 0x63
```

VFR token definition can be found under a link https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.6.1.7-vfr-question-tag-warningif-definition-vfrstatement


This built-in is supposed to be used inside a question. Let's add it to the `orderedlist`:
```
orderedlist
  varid = FormData.OrderedListValue,
  prompt = STRING_TOKEN(ORDERED_LIST_PROMPT),
  help = STRING_TOKEN(ORDERED_LIST_HELP),
  option text = STRING_TOKEN(ORDERED_LIST_OPTION1), value = 0x0A, flags = 0;
  option text = STRING_TOKEN(ORDERED_LIST_OPTION2), value = 0x0B, flags = 0;
  option text = STRING_TOKEN(ORDERED_LIST_OPTION3), value = 0x0C, flags = 0;
  warningif
    prompt = STRING_TOKEN(WARNING_IF_PROMPT),
    TRUE
  endif;
endlist;
```

Add new string to the `UefiLessonsPkg/HIIFormDataElements/Strings.uni`
```
#string WARNING_IF_PROMPT      #language en-US  "WarningIf prompt"
```

The warning message appear when you change the variable and hit `enter`:

![WarningIf](WarningIf.png?raw=true "WarningIf")

But it is important to notice, that you can ignore the warning and still save the updated data with `F10`.

With `warningif` it is possible to set timeout value for the warning message:
```
warningif
  prompt = STRING_TOKEN(WARNING_IF_PROMPT),
  timeout = 5,
  TRUE
endif;
```

In this case warning will dissapear after 5 seconds (or earlier if user presses a kay to acknowledge change). The ticking timer will be displayed on the popup window:

![WarningIf_timeout](WarningIf_timeout.png?raw=true "WarningIf_timeout")

# nosubmitif

Creates a conditional expression which will be evaluated when the form is submitted. If the conditional evaluates to TRUE, then the error message Error will be displayed to the user and the user will be prevented from submitting the form.

Opcode for this operation is:
```
#define EFI_IFR_NO_SUBMIT_IF_OP 0x10
```

VFR token definition can be found under a link https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.6.1.2-vfr-question-tag-nosubmitif-definition

This built-in is supposed to be used inside a question. Let's add it to the `orderedlist`:
```
orderedlist
  varid = FormData.OrderedListValue,
  prompt = STRING_TOKEN(ORDERED_LIST_PROMPT),
  help = STRING_TOKEN(ORDERED_LIST_HELP),
  option text = STRING_TOKEN(ORDERED_LIST_OPTION1), value = 0x0A, flags = 0;
  option text = STRING_TOKEN(ORDERED_LIST_OPTION2), value = 0x0B, flags = 0;
  option text = STRING_TOKEN(ORDERED_LIST_OPTION3), value = 0x0C, flags = 0;
  nosubmitif
    prompt = STRING_TOKEN(NOSUBMIT_IF_PROMPT),
    TRUE
  endif;
endlist;
```

Add new string to the `UefiLessonsPkg/HIIFormDataElements/Strings.uni`
```
#string NOSUBMIT_IF_PROMPT     #language en-US  "NoSubmitIf prompt"
```

With this code you can still change the variable and hit `enter` and the form would display updated value with `Configuration changed` message. But the form submit (`F10`) would fail. It is not possible to save variable if `nosubmitif` condition evaluates to `TRUE`.

![NosubmitIf](NosubmitIf.png?raw=true "NosubmitIf")

# inconsistentif

Creates a validation expression and error message for a question. The user should not be allowed to submit the results of a form inconsistency.

VFR token definition can be found under a link https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.6.1.1-vfr-question-tag-inconsistentif-definition

Opcode for this operation is:
```
#define EFI_IFR_INCONSISTENT_IF_OP 0x011
```

This built-in is supposed to be used inside a question. Let's add it to the `orderedlist`:
```
orderedlist
  varid = FormData.OrderedListValue,
  prompt = STRING_TOKEN(ORDERED_LIST_PROMPT),
  help = STRING_TOKEN(ORDERED_LIST_HELP),
  option text = STRING_TOKEN(ORDERED_LIST_OPTION1), value = 0x0A, flags = 0;
  option text = STRING_TOKEN(ORDERED_LIST_OPTION2), value = 0x0B, flags = 0;
  option text = STRING_TOKEN(ORDERED_LIST_OPTION3), value = 0x0C, flags = 0;
  inconsistentif
    prompt = STRING_TOKEN(INCONSISTENT_IF_PROMPT),
    TRUE
  endif;
endlist;
```

Add new string to the `UefiLessonsPkg/HIIFormDataElements/Strings.uni`
```
#string INCONSISTENT_IF_PROMPT #language en-US  "InconsistentIf prompt"
```

This code won't allow to change a variable to the value when `inconsistentif` evaluates to TRUE. Form will simply show popup message and won't change values.

![InconsistentIf](InconsistentIf.png?raw=true "InconsistentIf")
