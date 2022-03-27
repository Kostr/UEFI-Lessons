#ifndef _DATA_H_
#define _DATA_H_

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

#endif
