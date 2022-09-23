#ifndef _DATA_H_
#define _DATA_H_

#define FORMSET_GUID  {0x531bc507, 0x9191, 0x4fa2, {0x94, 0x46, 0xb8, 0x44, 0xe3, 0x5d, 0xd1, 0x2a}}
#define DATAPATH_GUID {0xc299f575, 0xf1dd, 0x4d7d, {0xb7, 0xaa, 0xe5, 0x06, 0x4b, 0x3e, 0xcb, 0xd7}}
#define STORAGE_GUID  {0xd2ae39c7, 0xe8dd, 0x4ab8, {0xad, 0x83, 0x7a, 0x57, 0xfe, 0x0e, 0x7e, 0xa8}}

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

#define LABEL_START 0x1111
#define LABEL_END   0x2222

#endif
