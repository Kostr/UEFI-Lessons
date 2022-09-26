#ifndef _DATA_H_
#define _DATA_H_

#define FORMSET_GUID  {0xdfdb4e02, 0x32ca, 0x4f50, {0xa1, 0xf1, 0x07, 0xdc, 0xfb, 0xf5, 0xb4, 0x5a}}
#define DATAPATH_GUID {0xec8952af, 0x0779, 0x4e39, {0xb1, 0x6c, 0x69, 0x7c, 0xb3, 0xc7, 0x89, 0x0d}}
#define STORAGE_GUID  {0x430d7be9, 0x60bd, 0x4081, {0x99, 0xc1, 0x01, 0xf4, 0xf7, 0x1f, 0xdf, 0x80}}

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
