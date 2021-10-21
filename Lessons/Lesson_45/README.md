Let's look one more time at the output of our `ShowHII` application:
```
FS0:\> ShowHII.efi
PackageList[0]: GUID=A487A478-51EF-48AA-8794-7BEE2A0562F1; size=0x1ADC
        Package[0]: type=STRINGS; size=0x1AC4
        Package[1]: type=END; size=0x4
PackageList[1]: GUID=19618BCE-55AE-09C6-37E9-4CE04084C7A1; size=0x21E4
        Package[0]: type=STRINGS; size=0x21CC
        Package[1]: type=END; size=0x4
PackageList[2]: GUID=2F30DA26-F51B-4B6F-85C4-31873C281BCA; size=0xA93
        Package[0]: type=STRINGS; size=0xA7B
        Package[1]: type=END; size=0x4
PackageList[3]: GUID=F74D20EE-37E7-48FC-97F7-9B1047749C69; size=0x2EE9
        Package[0]: type=IMAGES; size=0x2ED1
        Package[1]: type=END; size=0x4
PackageList[4]: GUID=EBF8ED7C-0DD1-4787-84F1-F48D537DCACF; size=0x46C
        Package[0]: type=FORMS; size=0x82
        Package[1]: type=FORMS; size=0x82
        Package[2]: type=STRINGS; size=0x199
        Package[3]: type=STRINGS; size=0x19B
        Package[4]: type=DEVICE_PATH; size=0x1C
        Package[5]: type=END; size=0x4
PackageList[5]: GUID=FE561596-E6BF-41A6-8376-C72B719874D0; size=0x93F
        Package[0]: type=FORMS; size=0xF5
        Package[1]: type=STRINGS; size=0x40A
        Package[2]: type=STRINGS; size=0x40C
        Package[3]: type=DEVICE_PATH; size=0x1C
        Package[4]: type=END; size=0x4
PackageList[6]: GUID=2A46715F-3581-4A55-8E73-2B769AAA30C5; size=0x6B0
        Package[0]: type=FORMS; size=0x143
        Package[1]: type=STRINGS; size=0x539
        Package[2]: type=DEVICE_PATH; size=0x1C
        Package[3]: type=END; size=0x4
PackageList[7]: GUID=99FDC8FD-849B-4EBA-AD13-FB9699C90A4D; size=0x6FE
        Package[0]: type=STRINGS; size=0x340
        Package[1]: type=STRINGS; size=0x3A6
        Package[2]: type=END; size=0x4
PackageList[8]: GUID=E38C1029-E38F-45B9-8F0D-E2E60BC9B262; size=0x15DA
        Package[0]: type=STRINGS; size=0xA88
        Package[1]: type=STRINGS; size=0xB3A
        Package[2]: type=END; size=0x4
PackageList[9]: GUID=D9DCC5DF-4007-435E-9098-8970935504B2; size=0x855
        Package[0]: type=FORMS; size=0x1F6
        Package[1]: type=STRINGS; size=0x62B
        Package[2]: type=DEVICE_PATH; size=0x1C
        Package[3]: type=END; size=0x4
PackageList[10]: GUID=F5F219D3-7006-4648-AC8D-D61DFB7BC6AD; size=0x14EC
        Package[0]: type=SIMPLE_FONTS; size=0x14D4
        Package[1]: type=END; size=0x4
PackageList[11]: GUID=4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9; size=0x6AC8
        Package[0]: type=FORMS; size=0x1030
        Package[1]: type=STRINGS; size=0x3C99
        Package[2]: type=STRINGS; size=0x1DCB
        Package[3]: type=DEVICE_PATH; size=0x1C
        Package[4]: type=END; size=0x4
PackageList[12]: GUID=F95A7CCC-4C55-4426-A7B4-DC8961950BAE; size=0x13909
        Package[0]: type=STRINGS; size=0x138F1
        Package[1]: type=END; size=0x4
PackageList[13]: GUID=DEC5DAA4-6781-4820-9C63-A7B0E4F1DB31; size=0x8677
        Package[0]: type=STRINGS; size=0x865F
        Package[1]: type=END; size=0x4
PackageList[14]: GUID=4344558D-4EF9-4725-B1E4-3376E8D6974F; size=0x83BD
        Package[0]: type=STRINGS; size=0x83A5
        Package[1]: type=END; size=0x4
PackageList[15]: GUID=0AF0B742-63EC-45BD-8DB6-71AD7F2FE8E8; size=0xCB04
        Package[0]: type=STRINGS; size=0xCAEC
        Package[1]: type=END; size=0x4
PackageList[16]: GUID=25F200AA-D3CB-470A-BF51-E7D162D22E6F; size=0x1D3D7
        Package[0]: type=STRINGS; size=0x1D3BF
        Package[1]: type=END; size=0x4
PackageList[17]: GUID=5F5F605D-1583-4A2D-A6B2-EB12DAB4A2B6; size=0x3048
        Package[0]: type=STRINGS; size=0x3030
        Package[1]: type=END; size=0x4
PackageList[18]: GUID=F3D301BB-F4A5-45A8-B0B7-FA999C6237AE; size=0x26B5
        Package[0]: type=STRINGS; size=0x269D
        Package[1]: type=END; size=0x4
PackageList[19]: GUID=7C04A583-9E3E-4F1C-AD65-E05268D0B4D1; size=0x5CB
        Package[0]: type=STRINGS; size=0x5B3
        Package[1]: type=END; size=0x4
```

From this output you can see that each Package list contains one or more data packages and ends with a special `END` package.

Ordinary package contains `EFI_HII_PACKAGE_HEADER` and data content. But the `END` package is simply a `EFI_HII_PACKAGE_HEADER` with a `Type` field set to `EFI_HII_PACKAGE_END`.

Just in case here are prototypes for the header structures once again:
```
typedef struct {
 EFI_GUID PackageListGuid;
 UINT32 PackagLength;
} EFI_HII_PACKAGE_LIST_HEADER;

typedef struct {
 UINT32 Length:24;
 UINT32 Type:8;
 UINT8 Data[ … ];
} EFI_HII_PACKAGE_HEADER;
```

So basically package list data here looks something like this:

![Package_list](Package_list.png?raw=true "Package list")


Ordinary packages can be of different types. In this lesson we would look at the string package.

# String package

String package is a set of strings in one language. Each of the strings in package can be referred by ID. IDs to these strings are not written anywhere but it is supposed that numbering scheme simply starts from ID=1 in every string package and increases by 1 on every string [there are some exceptions, but it is not important now].

The general idea is to create String packages for each language you want to support for these piece of interface. In every of these language string packages the same words should have the same ID. For example:
```
      String package (ENG)        String package (FR)
      ___________________         ___________________

ID=1   ...                            ...

       ...                            ...

ID=5   Hello!                        Bonjour!

       ...                            ...
```

In the code strings from the Package list are received from the `(ID, language)` combination. This helps to create UIs that could switch translations easily.

# String package content

String package strarts with a header `EFI_HII_STRING_PACKAGE_HDR` and then contains so called `String blocks`, which can be of different types. And as with the package list, the last element (=String block) has special `END` type.

![String_package](String_package.png?raw=true "String package")

The most important and most common type of String block is UCS2 String block. It contains header which points String block type and a CHAR16 string as a block data. Basically String package is a set of UCS2 String blocks each of which contains one string.

![UCS2_String_block](UCS2_String_block.png?raw=true "UCS2 String block")


# Create String package

```
Prototype:
typedef struct _EFI_HII_STRING_PACKAGE_HDR {
 EFI_HII_PACKAGE_HEADER Header;
 UINT32 HdrSize;
 UINT32 StringInfoOffset;
 CHAR16 LanguageWindow[16];
 EFI_STRING_ID LanguageName;
 CHAR8 Language[ … ];
} EFI_HII_STRING_PACKAGE_HDR;

Members:
Header			The standard package header, where Header.Type = EFI_HII_PACKAGE_STRINGS.
HdrSize			Size of this header.
StringInfoOffset	Offset, relative to the start of this header, of the string information.
LanguageWindow		Specifies the default values placed in the static and dynamic windows
			before processing each SCSU-encoded string.
LanguageName		String identifier within the current string package of the full name of the
			language specified by Language.
Language		The null-terminated ASCII string that specifies the language of the strings in the package.
```
