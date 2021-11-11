Let's start to investigate form elements.

Create new app `HIIStaticForm` with a content similar to our `HIISimpleForm` app. I won't repeat all the steps, as in this lesson we would only change form content.

So we start with this VFR:
```
#define HIISTATICFORM_FORMSET_GUID  {0x32783cc5, 0xe551, 0x4b61, {0xb7, 0xbd, 0x41, 0xba, 0x71, 0x7f, 0xba, 0x81}}

formset
  guid     = HIISTATICFORM_FORMSET_GUID,
  title    = STRING_TOKEN(HIISTATICFORM_FORMSET_TITLE),
  help     = STRING_TOKEN(HIISTATICFORM_FORMSET_HELP),
  form
    formid = 1,
    title = STRING_TOKEN(HIISTATICFORM_FORMID1_TITLE);
  endform;
endformset;
```
And it produces the folowing code:
```
formset
>00000000: 0E A7 C5 3C 78 32 51 E5 61 4B B7 BD 41 BA 71 7F BA 81 02 00 03 00 01 71 99 03 93 45 85 04 4B B4 5E 32 EB 83 26 04 0E
>00000027: 5C 06 00 00 00 00
>0000002D: 5C 06 00 00 01 00
  guid = {0x32783cc5, 0xe551, 0x4b61, {0xb7, 0xbd, 0x41, 0xba, 0x71, 0x7f, 0xba, 0x81}},
  title = STRING_TOKEN(0x0002),
  help = STRING_TOKEN(0x0003),
  form
>00000033: 01 86 01 00 04 00
    formid = 1,
    title = STRING_TOKEN(0x0004);
  endform;
>00000039: 29 02
endformset;
>0000003B: 29 02
```
And as you remember this creates an empty form with a title.

# Subtitle

The most simple form element is subtitle (https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.5.1-vfr-subtitle-definition). It is a non-interactive text to display some information to the user. Is is not possible to select this element. In EDKII form browser it is also displayed in a blue color (opposed to the interactive elements, that would be displayed in black color).

You can add subtitle with this code:
```
subtitle
  text = STRING_TOKEN(SUBTITLE1),
endsubtitle;
```
But it is most common to use a short form:
```
subtitle text = STRING_TOKEN(SUBTITLE1);
```

Simply embed this string inside our form code:
```
...
form
  formid = 1,
  title = STRING_TOKEN(HIISTATICFORM_FORMID1_TITLE);

  subtitle text = STRING_TOKEN(SUBTITLE1);
endform;
...
```

Off course you should declare this string in the UNI file:
```
#string SUBTITLE1                            #language en-US  "Subtitle1"
```

This VFR would produce following picture:

![Subtitle1](Subtitle1.png?raw=true "Subtitle1")

# IFR

```
formset
>00000000: 0E A7 C5 3C 78 32 51 E5 61 4B B7 BD 41 BA 71 7F BA 81 02 00 03 00 01 71 99 03 93 45 85 04 4B B4 5E 32 EB 83 26 04 0E
>00000027: 5C 06 00 00 00 00
>0000002D: 5C 06 00 00 01 00
  guid = {0x32783cc5, 0xe551, 0x4b61, {0xb7, 0xbd, 0x41, 0xba, 0x71, 0x7f, 0xba, 0x81}},
  title = STRING_TOKEN(0x0002),
  help = STRING_TOKEN(0x0003),
  form
>00000033: 01 86 01 00 04 00
    formid = 1,
    title = STRING_TOKEN(0x0004);
    subtitle text = STRING_TOKEN(0x0005);
>00000039: 02 87 05 00 00 00 00
>00000040: 29 02
  endform;
>00000042: 29 02
endformset;
>00000044: 29 02
```

If you compare output before and after you could see that the difference is:
```
    subtitle text = STRING_TOKEN(0x0005);
>00000039: 02 87 05 00 00 00 00
>00000040: 29 02
```

It is two opcodes: `EFI_IFR_SUBTITLE` and our `EFI_IFR_END`. We've already seen the structure for the `EFI_IFR_END`, so let's look at the `EFI_IFR_SUBTITLE`:
```
EFI_IFR_SUBTITLE

Summary:
Creates a sub-title in the current form.

Prototype:

#define EFI_IFR_SUBTITLE_OP 0x02

typedef struct _EFI_IFR_SUBTITLE {
 EFI_IFR_OP_HEADER Header;
 EFI_IFR_STATEMENT_HEADER Statement;
 UINT8 Flags;
} EFI_IFR_SUBTITLE;

Members:
Header 		The sequence that defines the type of opcode as well as the length of the opcode being defined.
		For this tag, Header.OpCode = EFI_IFR_SUBTITLE_OP.
Flags 		Identifies specific behavior for the sub-title.
```


# Another subtitle

If you add another subtitle similar to like we did it above:
```
...
form
  formid = 1,
  title = STRING_TOKEN(HIISTATICFORM_FORMID1_TITLE);

  subtitle text = STRING_TOKEN(SUBTITLE1);
  subtitle text = STRING_TOKEN(SUBTITLE2);
endform;
...
```

This subtitle will be simply printed on the next string. This principle is true for all form elements. Next element by default will simply go to another string.

![Subtitle2](Subtitle2.png?raw=true "Subtitle2")

# Utilizing subtitle scope

As we saw above, `EFI_IFR_SUBTITLE` opens a scope which is later closed by the `EFI_IFR_END`.

It is possible to define elements inside subtitle scope which will indent all the elements inside the scope.

Add this construction to our form:
```
subtitle
  text = STRING_TOKEN(SUBTITLE3),

  subtitle text = STRING_TOKEN(SUBTITLE4);
endsubtitle;
```

If you look at the `Form.lst` you would see that 
```
    subtitle
>0000004B: 02 87 07 00 00 00 00
      text = STRING_TOKEN(0x0007),
      subtitle text = STRING_TOKEN(0x0008);
>00000052: 02 87 08 00 00 00 00
>00000059: 29 02
    endsubtitle;
>0000005B: 29 02
```
Here you can see opcodes `EFI_IFR_SUBTITLE`-`EFI_IFR_SUBTITLE`-`EFI_IFR_END`-`EFI_IFR_END`. If we didn't put `SUBTITLE4` in the `SUBTITLE3` scope, the output would be `EFI_IFR_SUBTITLE`-`EFI_IFR_END`-`EFI_IFR_SUBTITLE`-`EFI_IFR_END`.


![Subtitle3](Subtitle3.png?raw=true "Subtitle3")

To get more deeper understanding add another subtitle inside the scope of `SUBTITLE3` and another after the scope:
```
subtitle
  text = STRING_TOKEN(SUBTITLE3),

  subtitle text = STRING_TOKEN(SUBTITLE4);
  subtitle text = STRING_TOKEN(SUBTITLE5);
endsubtitle;

subtitle text = STRING_TOKEN(SUBTITLE6);
```

![Subtitle4](Subtitle4.png?raw=true "Subtitle4")

# Empty string

Subtitle element is an easy way to add an empty string to your form.

Insert this code right before the `SUBTITLE6` definition
```
subtitle text = STRING_TOKEN(STR_NULL);
```
And define `STR_NULL` as:
```
#string STR_NULL                             #language en-US  ""
```

This would give you:

![Subtitle5](Subtitle5.png?raw=true "Subtitle5")

# Text element

The next element that we will discuss is `text` (https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.5.2-vfr-text-definition). Add following code to our form right after the last subtitle:
```
text
  help = STRING_TOKEN(TEXT1_help),
  text = STRING_TOKEN(TEXT1_text);
```

If you would look at the `Form.lst`, you'll see that `text` doesn't open a scope, therefore only one IFR is produced - `EFI_IFR_TEXT`:
```
    text
>00000078: 03 08 0C 00 0D 00 00 00
      help = STRING_TOKEN(0x000D),
      text = STRING_TOKEN(0x000C);
```

```
EFI_IFR_TEXT

Summary:
Creates a static text and image.

Prototype:

#define EFI_IFR_TEXT_OP 0x03

typedef struct _EFI_IFR_TEXT {
 EFI_IFR_OP_HEADER Header;
 EFI_IFR_STATEMENT_HEADER Statement;
 EFI_STRING_ID TextTwo;
} EFI_IFR_TEXT;

Members:
Header 		The sequence that defines the type of opcode as well as the length of the opcode being defined.
		For this tag, Header.OpCode = EFI_IFR_TEXT_OP.
Statement 	Standard statement header.
TextTwo 	The string token reference to the secondary string for this opcode.

Description:
This is a static text/image statement.
```
And here is a definition for the `EFI_IFR_STATEMENT_HEADER` field:
```
EFI_IFR_STATEMENT_HEADER

Summary:
Standard statement header.

Prototype:
typedef struct _EFI_IFR_STATEMENT_HEADER {
 EFI_STRING_ID Prompt;
 EFI_STRING_ID Help;
} EFI_IFR_STATEMENT_HEADER;

Members:
Prompt 	The string identifier of the prompt string for this particular statement. The value 0 indicates no prompt string.
Help 	The string identifier of the help string for this particular statement. The value 0 indicates no help string

Description:
This is the standard header for statements, including questions.
```

On the screen `text` element looks like this:

![Text1](Text1.png?raw=true "Text1")

The main difference of the `text` element from the `subtitle` element is that you can select `text` elements.

This is more obvious if you'll add another text element right after the first one:

![Text2](Text2.png?raw=true "Text2")

Now you can use arrow keys to select either `Text1 title` or `Text2 title`. And when you would do this help text for title would change automatically.

# Another text field

It is possible to add another text field to the text element:
```
text
  help = STRING_TOKEN(TEXT3_HELP),
  text = STRING_TOKEN(TEXT3_TEXT);
  text = STRING_TOKEN(TEXT3_TEXT_TWO);
```
Its string would go to the `EFI_IFR_TEXT.TextTwo` field.

In the browser it would like this. The second string is placed in a choice place for the menu item:

![Text3](Text3.png?raw=true "Text3")


