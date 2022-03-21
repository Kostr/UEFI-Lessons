# `date` element

`date` input element is used to store date (https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.6.9-vfr-date-statement-definition)

Add this code to `UefiLessonsPkg/HIIFormDataElements/Form.vfr`:
```
date
  varid = FormData.DateValue,
  prompt = STRING_TOKEN(DATE_PROMPT),
  help = STRING_TOKEN(DATE_HELP),
enddate;
```

Add strings to `UefiLessonsPkg/HIIFormDataElements/Strings.uni`
```
#string DATE_PROMPT            #language en-US  "Date prompt"
#string DATE_HELP              #language en-US  "Date help"
```

Date is encoded in special `EFI_HII_DATE` type (https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiInternalFormRepresentation.h):
```
typedef struct {
  UINT16    Year;
  UINT8     Month;
  UINT8     Day;
} EFI_HII_DATE;
```

Therefore add it to our structure in `UefiLessonsPkg/HIIFormDataElements/Data.h`:
```
typedef struct {
  ...
  EFI_HII_DATE DateValue;
} UEFI_VARIABLE_STRUCTURE;
```

This will result to this element:
 
![Date1](Date1.png?raw=true "Date1")

The value is displayed in `MM/DD/YYYY` format. So it is possible to store `02/20/2022`, but not `20/02/2022`.

As for checks, even the leap year is checked. You can enter `02/29/2020`, but not `02/29/2021`.

# `time` element

`time` VFR input element is used to store time (https://edk2-docs.gitbook.io/edk-ii-vfr-specification/2_vfr_description_in_bnf/211_vfr_form_definition#2.11.6.10-vfr-time-statement-definition)

Add this code to `UefiLessonsPkg/HIIFormDataElements/Form.vfr`:
```
time
  varid = FormData.TimeValue,
  prompt = STRING_TOKEN(TIME_PROMPT),
  help = STRING_TOKEN(TIME_HELP),
endtime;
```

Add strings to `UefiLessonsPkg/HIIFormDataElements/Strings.uni`
```
#string TIME_PROMPT            #language en-US  "Time prompt"
#string TIME_HELP              #language en-US  "Time help"
```

Time is encoded in special `EFI_HII_TIME` type (https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Uefi/UefiInternalFormRepresentation.h):
```
typedef struct {
  UINT8    Hour;
  UINT8    Minute;
  UINT8    Second;
} EFI_HII_TIME;
```

Therefore add it to our structure in `UefiLessonsPkg/HIIFormDataElements/Data.h`:
```
typedef struct {
  ...
  EFI_HII_TIME TimeValue;
} UEFI_VARIABLE_STRUCTURE;
```

This will result to this element:

![Time1](Time1.png?raw=true "Time1")

As with `date` element HII Form Browser doesn't allow to set invalid time values.
