#ifndef CUSTOM_PCD_TYPES_H
#define CUSTOM_PCD_TYPES_H

typedef struct {
  EFI_GUID Guid;
  CHAR16 Name[6];
} InnerCustomStruct;

typedef struct {
  UINT8 Val8;
  UINT32 Val32[2];
  InnerCustomStruct ValStruct;
  union {
    struct {
      UINT8 Field1:1;
      UINT8 Field2:4;
      UINT8 Filed3:3;
    } BitFields;
    UINT8 Byte;
  } ValUnion;
} CustomStruct;

#endif
