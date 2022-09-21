Now let's try to dump the configuration settings for the applications that we've created earlier.

# `HIIFormCheckbox.efi`

Let's take `HIIFormCheckbox.efi` as initial example. This is our minimal HII Form application.

Load its driver in UEFI shell:
```
FS0:\> load HIIFormCheckbox.efi
```
And check `HIIConfig.efi dump` output:
```
FS0:\> HIIConfig.efi dump
<...>
```
It can be a surprise, but the `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExportConfig()` call doesn't return settings for the storage created by the `HIIFormCheckbox.efi` driver. But at the same time it is possible to return this settings via the direct `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExtractConfig()` call. This is the behaviour for storages created with a help of a `efivarstore` helper.

So let's try to construct request for our storage. This is the example from the `UefiLessonsPkg/HIIFormCheckbox/Form.vfr` code:
```
#define FORMSET_GUID  {0xef2acc91, 0x7b50, 0x4ab9, {0xab, 0x67, 0x2b, 0x4, 0xf8, 0xbc, 0x13, 0x5e}}

formset
  guid     = FORMSET_GUID,
  title    = STRING_TOKEN(FORMSET_TITLE),
  help     = STRING_TOKEN(FORMSET_HELP),

  efivarstore UINT8,
    attribute = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
    name  = CheckboxValue,
    guid  = FORMSET_GUID;

  form
    formid = 1,
    title = STRING_TOKEN(FORMID1_TITLE);

    checkbox
      varid = CheckboxValue,
      prompt = STRING_TOKEN(CHECKBOX_PROMPT),
      help = STRING_TOKEN(CHECKBOX_HELP),
    endcheckbox;
  endform;
endformset;
```
And here is how we encoded DevicePath in the `UefiLessonsPkg/HIIFormCheckbox/HIIFormCheckbox.c` file:
```
HII_VENDOR_DEVICE_PATH  mHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    FORMSET_GUID
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

Putting all of this together our request would look like this:
```
HIIConfig.efi extract ef2acc91-7b50-4ab9-ab67-2b04f8bc135e CheckboxValue VenHw(ef2acc91-7b50-4ab9-ab67-2b04f8bc135e)
```

So let's try to use it:
```
FS0:\> HIIConfig.efi extract ef2acc91-7b50-4ab9-ab67-2b04f8bc135e CheckboxValue VenHw(ef2acc91-7b50-4ab9-ab67-2b04f8bc135e)

Request: GUID=91cc2aef507bb94aab672b04f8bc135e&NAME=0043006800650063006b0062006f007800560061006c00750065&PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400
Response: GUID=91cc2aef507bb94aab672b04f8bc135e&NAME=0043006800650063006b0062006f007800560061006c00750065&PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400&OFFSET=0000&WIDTH=0001&VALUE=00&GUID=91cc2aef507bb94aab672b04f8bc135e&NAME=0043006800650063006b0062006f007800560061006c00750065&PATH=0104140091cc2aef507bb94aab672b0<...>


GUID=91cc2aef507bb94aab672b04f8bc135e (EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E)
NAME=0043006800650063006b0062006f007800560061006c00750065 (CheckboxValue)
PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400 (VenHw(EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E))
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .

GUID=91cc2aef507bb94aab672b04f8bc135e (EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E)
NAME=0043006800650063006b0062006f007800560061006c00750065 (CheckboxValue)
PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400 (VenHw(EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E))
ALTCFG=0000
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .

GUID=91cc2aef507bb94aab672b04f8bc135e (EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E)
NAME=0043006800650063006b0062006f007800560061006c00750065 (CheckboxValue)
PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400 (VenHw(EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E))
ALTCFG=0001
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
```

The output is broken in 3 groups. The first group corresponds to the current data setting. Currently the UINT8 value for the Checkbox is equal to 0x00.

The next 2 groups describe alternative configurations. These configurations under the `ALTCFG=0000` and `ALTCFG=0001` correspond to defaults:
- `ALTCFG=0000` - Standard default
- `ALTCFG=0001` - Manufacturing default

We don't have any code for the defaults in our VFR, therefore HII subsystem have returned zero values in them.

Now let's try to go to the form browser and manually change the checkbox value.

After that issue the extract call again:
```
FS0:\> HIIConfig.efi extract ef2acc91-7b50-4ab9-ab67-2b04f8bc135e CheckboxValue VenHw(ef2acc91-7b50-4ab9-ab67-2b04f8bc135e)

Request: GUID=91cc2aef507bb94aab672b04f8bc135e&NAME=0043006800650063006b0062006f007800560061006c00750065&PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400
Response: GUID=91cc2aef507bb94aab672b04f8bc135e&NAME=0043006800650063006b0062006f007800560061006c00750065&PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400&OFFSET=0000&WIDTH=0001&VALUE=01&GUID=91cc2aef507bb94aab672b04f8bc135e&NAME=0043006800650063006b0062006f007800560061006c00750065&PATH=0104140091cc2aef507bb94aab672b0<...>


GUID=91cc2aef507bb94aab672b04f8bc135e (EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E)
NAME=0043006800650063006b0062006f007800560061006c00750065 (CheckboxValue)
PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400 (VenHw(EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E))
OFFSET=0000  WIDTH=0001  VALUE=01
01                                               | .

GUID=91cc2aef507bb94aab672b04f8bc135e (EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E)
NAME=0043006800650063006b0062006f007800560061006c00750065 (CheckboxValue)
PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400 (VenHw(EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E))
ALTCFG=0000
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .

GUID=91cc2aef507bb94aab672b04f8bc135e (EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E)
NAME=0043006800650063006b0062006f007800560061006c00750065 (CheckboxValue)
PATH=0104140091cc2aef507bb94aab672b04f8bc135e7fff0400 (VenHw(EF2ACC91-7B50-4AB9-AB67-2B04F8BC135E))
ALTCFG=0001
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
```

You can see how the returned data in the first group (current setting) has changed to the 0x01.

# `HIIFormDataElements.efi`

Now let's try to work with more advanced application: `HIIFormDataElements.efi`.

It encodes data in a custom `UEFI_VARIABLE_STRUCTURE` (`UefiLessonsPkg/HIIFormDataElements/Data.h`):
```
#define FORMSET_GUID  {0x531bc507, 0x9191, 0x4fa2, {0x94, 0x46, 0xb8, 0x44, 0xe3, 0x5d, 0xd1, 0x2a}}

#define UEFI_VARIABLE_STRUCTURE_NAME L"FormData"

#pragma pack(1)
typedef struct {
  UINT8 CheckboxValue;
  UINT16 NumericValue;
  CHAR16 StringValue[11];
  EFI_HII_DATE DateValue;
  EFI_HII_TIME TimeValue;
  UINT8 OneOfValue;
  UINT8 OrderedListValue[3];
} UEFI_VARIABLE_STRUCTURE;
#pragma pack()
```

And the VFR code looks like this (`UefiLessonsPkg/HIIFormDataElements/Form.vfr`):
```
formset
  guid     = FORMSET_GUID,
  title    = STRING_TOKEN(FORMSET_TITLE),
  help     = STRING_TOKEN(FORMSET_HELP),

  efivarstore UEFI_VARIABLE_STRUCTURE,
    attribute = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
    name  = FormData,
    guid  = FORMSET_GUID;

  defaultstore StandardDefault,
    prompt      = STRING_TOKEN(STANDARD_DEFAULT_PROMPT),
    attribute   = 0x0000;

  defaultstore ManufactureDefault,
    prompt      = STRING_TOKEN(MFG_DEFAULT_PROMPT),
    attribute   = 0x0001;

  form
    formid = 1,
    title = STRING_TOKEN(FORMID1_TITLE);

    checkbox
      varid = FormData.CheckboxValue,
      prompt = STRING_TOKEN(CHECKBOX_PROMPT),
      help = STRING_TOKEN(CHECKBOX_HELP),
      default = TRUE, defaultstore = StandardDefault,
      default = FALSE, defaultstore = ManufactureDefault,
    endcheckbox;

    numeric
      name = NumericQuestion,
      varid = FormData.NumericValue,
      prompt = STRING_TOKEN(NUMERIC_PROMPT),
      help = STRING_TOKEN(NUMERIC_HELP),
      flags = NUMERIC_SIZE_2 | DISPLAY_UINT_HEX,
      //minimum = 0x1234,
      //maximum = 0xaa55,
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
      minsize = 5,
      maxsize = 10,
      default = STRING_TOKEN(STRING_DEFAULT), defaultstore = StandardDefault,
      default = STRING_TOKEN(STRING_PROMPT), defaultstore = ManufactureDefault,
    endstring;

    date
      varid = FormData.DateValue,
      prompt = STRING_TOKEN(DATE_PROMPT),
      help = STRING_TOKEN(DATE_HELP),
      default = 2021/05/22,
    enddate;

    time
      varid = FormData.TimeValue,
      prompt = STRING_TOKEN(TIME_PROMPT),
      help = STRING_TOKEN(TIME_HELP),
      default = 23:55:33,
    endtime;

    oneof
      name = OneOfQuestion,
      varid = FormData.OneOfValue,
      prompt = STRING_TOKEN(ONEOF_PROMPT),
      help = STRING_TOKEN(ONEOF_HELP),
      option text = STRING_TOKEN(ONEOF_OPTION1), value = 0x00, flags = 0;
      option text = STRING_TOKEN(ONEOF_OPTION2), value = 0x33, flags = MANUFACTURING;
      option text = STRING_TOKEN(ONEOF_OPTION3), value = 0x55, flags = DEFAULT;
    endoneof;

    orderedlist
      varid = FormData.OrderedListValue,
      prompt = STRING_TOKEN(ORDERED_LIST_PROMPT),
      help = STRING_TOKEN(ORDERED_LIST_HELP),
      option text = STRING_TOKEN(ORDERED_LIST_OPTION1), value = 0x0A, flags = 0;
      option text = STRING_TOKEN(ORDERED_LIST_OPTION2), value = 0x0B, flags = 0;
      option text = STRING_TOKEN(ORDERED_LIST_OPTION3), value = 0x0C, flags = 0;
      default = {0x0c, 0x0b, 0x0a},
    endlist;

    resetbutton
      defaultstore = StandardDefault,
      prompt   = STRING_TOKEN(BTN_STANDARD_DEFAULT_PROMPT),
      help     = STRING_TOKEN(BTN_STANDARD_DEFAULT_HELP),
    endresetbutton;

    resetbutton
      defaultstore = ManufactureDefault,
      prompt   = STRING_TOKEN(BTN_MFG_DEFAULT_PROMPT),
      help     = STRING_TOKEN(BTN_MFG_DEFAULT_HELP),
    endresetbutton;
  endform;
endformset;
```

DevicePath is also encoded like `VenHw(FORMSET_GUID)` (`UefiLessonsPkg/HIIFormDataElements/HIIFormDataElements.c`):
```cpp
HII_VENDOR_DEVICE_PATH  mHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    FORMSET_GUID
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

Let's try to load the driver to the environment:
```
FS0:\> load HIIFormDataElements.efi
```

And request its HII configuration data:
```
FS0:\> HIIConfig.efi extract 531bc507-9191-4fa2-9446-b844e35dd12a FormData VenHw(531bc507-9191-4fa2-9446-b844e35dd12a)

Request: GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400
Response: GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400&OFFSET=0000&WIDTH=0001&VALUE=00&OFFSET=0001&WIDTH=0002&VALUE=0000&OFFSET=0003&WIDTH=0014&VALUE=0000000000000000000000000000000000000000&OFFSET=0019&WIDTH=0004&VALUE=00000000&OFFSET=001<...>


GUID=07c51b539191a24f9446b844e35dd12a (531BC507-9191-4FA2-9446-B844E35DD12A)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400 (VenHw(531BC507-9191-4FA2-9446-B844E35DD12A))
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0000
00 00                                            | ..
OFFSET=0003  WIDTH=0014  VALUE=0000000000000000000000000000000000000000
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
00 00 00 00                                      | ....
OFFSET=0019  WIDTH=0004  VALUE=00000000
00 00 00 00                                      | ....
OFFSET=001d  WIDTH=0003  VALUE=000000
00 00 00                                         | ...
OFFSET=0020  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0021  WIDTH=0003  VALUE=000000
00 00 00                                         | ...

GUID=07c51b539191a24f9446b844e35dd12a (531BC507-9191-4FA2-9446-B844E35DD12A)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400 (VenHw(531BC507-9191-4FA2-9446-B844E35DD12A))
ALTCFG=0000
OFFSET=0000  WIDTH=0001  VALUE=01
01                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0007
07 00                                            | ..
OFFSET=0003  WIDTH=0014  VALUE=00660065006400200067006e0069007200740053
53 00 74 00 72 00 69 00 6E 00 67 00 20 00 64 00  | S.t.r.i.n.g. .d.
65 00 66 00                                      | e.f.
OFFSET=0019  WIDTH=0004  VALUE=160507e5
E5 07 05 16                                      | ....
OFFSET=001d  WIDTH=0003  VALUE=213717
17 37 21                                         | .7!
OFFSET=0020  WIDTH=0001  VALUE=55
55                                               | U
OFFSET=0021  WIDTH=0003  VALUE=0a0b0c
0C 0B 0A                                         | ...

GUID=07c51b539191a24f9446b844e35dd12a (531BC507-9191-4FA2-9446-B844E35DD12A)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400 (VenHw(531BC507-9191-4FA2-9446-B844E35DD12A))
ALTCFG=0001
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0008
08 00                                            | ..
OFFSET=0003  WIDTH=0014  VALUE=006f0072007000200067006e0069007200740053
53 00 74 00 72 00 69 00 6E 00 67 00 20 00 70 00  | S.t.r.i.n.g. .p.
72 00 6F 00                                      | r.o.
OFFSET=0019  WIDTH=0004  VALUE=160507e5
E5 07 05 16                                      | ....
OFFSET=001d  WIDTH=0003  VALUE=213717
17 37 21                                         | .7!
OFFSET=0020  WIDTH=0001  VALUE=33
33                                               | 3
OFFSET=0021  WIDTH=0003  VALUE=0a0b0c
0C 0B 0A                                         | ...
```

Here you can see the current values (by default they are all zeros), and the Standard and Manufacturing defaults. As we've actually filled defaults in the VFR code, you can see different data for the Standard and Manufacturing cases.

You can request individual form elements if you provide offset and width in the request. For example here is how you can get settings for the `string` element:
```
FS0:\> HIIConfig.efi extract 531bc507-9191-4fa2-9446-b844e35dd12a FormData VenHw(531bc507-9191-4fa2-9446-b844e35dd12a) 3 14
Request: GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400&OFFSET=3&WIDTH=14
Response: GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400&OFFSET=3&WIDTH=14&VALUE=0000000000000000000000000000000000000000&GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd1<...>


GUID=07c51b539191a24f9446b844e35dd12a (531BC507-9191-4FA2-9446-B844E35DD12A)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400 (VenHw(531BC507-9191-4FA2-9446-B844E35DD12A))
OFFSET=3  WIDTH=14  VALUE=0000000000000000000000000000000000000000
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  | ................
00 00 00 00                                      | ....

GUID=07c51b539191a24f9446b844e35dd12a (531BC507-9191-4FA2-9446-B844E35DD12A)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400 (VenHw(531BC507-9191-4FA2-9446-B844E35DD12A))
ALTCFG=0000
OFFSET=0003  WIDTH=0014  VALUE=00660065006400200067006e0069007200740053
53 00 74 00 72 00 69 00 6E 00 67 00 20 00 64 00  | S.t.r.i.n.g. .d.
65 00 66 00                                      | e.f.

GUID=07c51b539191a24f9446b844e35dd12a (531BC507-9191-4FA2-9446-B844E35DD12A)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400 (VenHw(531BC507-9191-4FA2-9446-B844E35DD12A))
ALTCFG=0001
OFFSET=0003  WIDTH=0014  VALUE=006f0072007000200067006e0069007200740053
53 00 74 00 72 00 69 00 6E 00 67 00 20 00 70 00  | S.t.r.i.n.g. .p.
72 00 6F 00                                      | r.o.
```

It is not required that `OFFSET...OFFSET+WIDTH` should be on the element boundaries. You can provide any OFFSET/WIDTH combination that defines a range within the storage limits. For example `0 2` would include the `checkbox` and a part of the `numeric` element:
```
FS0:\> HIIConfig.efi extract 531bc507-9191-4fa2-9446-b844e35dd12a FormData VenHw(531bc507-9191-4fa2-9446-b844e35dd12a) 0 2

Request: GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400&OFFSET=0&WIDTH=2
Response: GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400&OFFSET=0&WIDTH=2&VALUE=0000&GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400&ALTCFG=0000&OFFSET=0000&WI<...>


GUID=07c51b539191a24f9446b844e35dd12a (531BC507-9191-4FA2-9446-B844E35DD12A)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400 (VenHw(531BC507-9191-4FA2-9446-B844E35DD12A))
OFFSET=0  WIDTH=2  VALUE=0000
00 00                                            | ..

GUID=07c51b539191a24f9446b844e35dd12a (531BC507-9191-4FA2-9446-B844E35DD12A)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400 (VenHw(531BC507-9191-4FA2-9446-B844E35DD12A))
ALTCFG=0000
OFFSET=0000  WIDTH=0001  VALUE=01
01                                               | .

GUID=07c51b539191a24f9446b844e35dd12a (531BC507-9191-4FA2-9446-B844E35DD12A)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400 (VenHw(531BC507-9191-4FA2-9446-B844E35DD12A))
ALTCFG=0001
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
```

In such cases `ALTCFG`'s are provided only for the elements that are covered fully by the OFFSET/WIDTH combination. This is why there is `ALTCFG` only for the `checkbox` element, but not for the `numeric` element. If you'll provide `0 3` as OFFSET/WIDTH, both elements would be covered by the range and alternative configurations would be displayed for both of them:
```
FS0:\> HIIConfig.efi extract 531bc507-9191-4fa2-9446-b844e35dd12a FormData VenHw(531bc507-9191-4fa2-9446-b844e35dd12a) 0 3

Request: GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400&OFFSET=0&WIDTH=3
Response: GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400&OFFSET=0&WIDTH=3&VALUE=000000&GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400&ALTCFG=0000&OFFSET=0000&<...>


GUID=07c51b539191a24f9446b844e35dd12a (531BC507-9191-4FA2-9446-B844E35DD12A)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400 (VenHw(531BC507-9191-4FA2-9446-B844E35DD12A))
OFFSET=0  WIDTH=3  VALUE=000000
00 00 00                                         | ...

GUID=07c51b539191a24f9446b844e35dd12a (531BC507-9191-4FA2-9446-B844E35DD12A)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400 (VenHw(531BC507-9191-4FA2-9446-B844E35DD12A))
ALTCFG=0000
OFFSET=0000  WIDTH=0001  VALUE=01
01                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0007
07 00                                            | ..

GUID=07c51b539191a24f9446b844e35dd12a (531BC507-9191-4FA2-9446-B844E35DD12A)
FS0:\> 46006f0072006d0044006100740061 (FormData)
PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400 (VenHw(531BC507-9191-4FA2-9446-B844E35DD12A))
ALTCFG=0001
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0008
08 00                                            | ..
```

And if offset/width doesn't cover any element range, there wouldn't be any `ALTCFG`'s in the output at all:
```
FS0:\> HIIConfig.efi extract 531bc507-9191-4fa2-9446-b844e35dd12a FormData VenHw(531bc507-9191-4fa2-9446-b844e35dd12a) 12 3

Request: GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400&OFFSET=12&WIDTH=3
Response: GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400&OFFSET=12&WIDTH=3&VALUE=000000


GUID=07c51b539191a24f9446b844e35dd12a (531BC507-9191-4FA2-9446-B844E35DD12A)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400 (VenHw(531BC507-9191-4FA2-9446-B844E35DD12A))
OFFSET=12  WIDTH=3  VALUE=000000
00 00 00                                         | ...
```

Finally if you'll provide OFFSET/WIDTH that cover range outside of the storage limit, you would get an error:
```
FS0:\> HIIConfig.efi extract 531bc507-9191-4fa2-9446-b844e35dd12a FormData VenHw(531bc507-9191-4fa2-9446-b844e35dd12a) 21 9

Request: GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400&OFFSET=21&WIDTH=9
Part of string was unparsed GUID=07c51b539191a24f9446b844e35dd12a&NAME=0046006f0072006d0044006100740061&PATH=0104140007c51b539191a24f9446b844e35dd12a7fff0400&OFFSET=21&WIDTH=9
Error! ExtractConfig returned Device Error
```

You can try to modify form element values via the form browser and look how `extract` response correctly displays the new data.

