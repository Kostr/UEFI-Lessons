In this lesson we would investigate how to modify hidden BIOS settings. "Hidden" in this context means settings from the menu that are not selectable for modification or settings that are even not shown in the menu. In other words settings that are closed by `grayoutif` or `suppressif` in the VFR code.

For this lesson I've created a `HiddenSettings` driver that populates VFR elements hidden with both these methods. To show that we can modify settings regardless of their underlying storage type, the driver populates both `varstore` elements and `efivarstore` elements.

You can find the source code for the driver at the end of this lesson. But right now let's imagine that we now nothing about it.

When the driver is loaded:
```
FS0:\> load HiddenSettings.efi
Image 'FS0:\HiddenSettings.efi' loaded at 61F9000 - Success
```

It populates the following formset:

![FormsetInTheMenu](FormsetInTheMenu.png?raw=true "FormsetInTheMenu")

If you enter this form, you would see this:

![Form1](Form1.png?raw=true "Form1")

As you can see no settings are available for modification. So what can we do in such case?

# General plan

- Dump package lists with form packages,
- "Disifr" (analogy with "disassemble", since readable VFR code is compiled to IFR machine code) package lists with form packages and find necessary hidden settings,
- Use `HIIConfig.efi` application to modify settings

# Dump package lists with form packages

We already have a `ShowHII` application that allows us to see all package lists in the HII database. Let's modify it a little bit to give us an opportunity to save them. To not interfere with the previous lesson I've created a new `ShowHIIext` (extended) application with the necessary modifications. I won't go into the details, since we already know all the building blocks for such changes.

Here is a final help for the `ShowHIIext` application:
```
FS0:\> ShowHIIext.efi -?
Shows packages installed into the HII database.

SHOWHIIEXT [-b] [save [<index>]]

  -b            - Display one screen at a time
  save          - Save all package lists as files
  save <index>  - Save package list with index <index> as a file
```

We can first execute it without arguments, find the package lists in the output which have packages of `FORMS` type and then use the application again with `save <index>` options to save only the necessary package lists.

But to make things simplier and faster let's save all the available package lists and deal with them later. Don't forget to load our `HiddenSettings.efi` driver first!
```
FS0:\> load HiddenSettings.efi
Image 'FS0:\HiddenSettings.efi' loaded at 640A000 - Success
FS0:\> ShowHIIext.efi save
Save file as 0000_A487A478-51EF-48AA-8794-7BEE2A0562F1
Save file as 0001_19618BCE-55AE-09C6-37E9-4CE04084C7A1
Save file as 0002_2F30DA26-F51B-4B6F-85C4-31873C281BCA
Save file as 0003_F74D20EE-37E7-48FC-97F7-9B1047749C69
Save file as 0004_EBF8ED7C-0DD1-4787-84F1-F48D537DCACF
Save file as 0005_FE561596-E6BF-41A6-8376-C72B719874D0
Save file as 0006_2A46715F-3581-4A55-8E73-2B769AAA30C5
Save file as 0007_99FDC8FD-849B-4EBA-AD13-FB9699C90A4D
Save file as 0008_E38C1029-E38F-45B9-8F0D-E2E60BC9B262
Save file as 0009_D9DCC5DF-4007-435E-9098-8970935504B2
Save file as 0010_F5F219D3-7006-4648-AC8D-D61DFB7BC6AD
Save file as 0011_4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9
Save file as 0012_F95A7CCC-4C55-4426-A7B4-DC8961950BAE
Save file as 0013_DEC5DAA4-6781-4820-9C63-A7B0E4F1DB31
Save file as 0014_4344558D-4EF9-4725-B1E4-3376E8D6974F
Save file as 0015_0AF0B742-63EC-45BD-8DB6-71AD7F2FE8E8
Save file as 0016_25F200AA-D3CB-470A-BF51-E7D162D22E6F
Save file as 0017_5F5F605D-1583-4A2D-A6B2-EB12DAB4A2B6
Save file as 0018_F3D301BB-F4A5-45A8-B0B7-FA999C6237AE
Save file as 0019_7C04A583-9E3E-4F1C-AD65-E05268D0B4D1
Save file as 0020_269D7962-BADC-44DA-815A-4AF4F293F3E0
```

# "Disifr" package lists with form packages and find necessary hidden settings

Now when we have all the package lists, let's try to extract UEFI IFR data from the binaries and transform it into human-readable text.

Luckily for us such tool already exists.

Initially it was developed by @donovan6000 and called [Universal-IFR-Extractor](https://github.com/donovan6000/Universal-IFR-Extractor). The development was discontinued and picked up in the [fork](https://github.com/LongSoft/Universal-IFR-Extractor) by @NikolajSchlej. As a final iteration @NikolajSchlej has decided to rewrite the utility to Rust language, and at the time of this writing it can be considered as a most up-to-date version of the utility - [IFRExtractor-RS](https://github.com/LongSoft/IFRExtractor-RS). So this is a program that we would be using to find our hidden settings.

Since the application is written in Rust don't forget to install cargo (the Rust package manager):
```
$ sudo apt install cargo
```

Now let's clone and build `IFRExtractor-RS` application:
```
$ git clone https://github.com/LongSoft/IFRExtractor-RS
$ cd IFRExtractor-RS
$ cargo build
    Updating crates.io index
  Downloaded version_check v0.1.5
  Downloaded memchr v2.5.0
  Downloaded nom v4.2.3
  Downloaded 3 crates (189.3 KB) in 0.81s
   Compiling version_check v0.1.5
   Compiling memchr v2.5.0
   Compiling nom v4.2.3
   Compiling ifrextractor v1.1.0 (/home/<...>/IFRExtractor-RS)
    Finished dev [unoptimized + debuginfo] target(s) in 1m 02s
```

The final binary would be created in the `target/debug` folder, and here is a help of the generated `ifrextractor` utility:
```
$ ./target/debug/ifrextractor
IFRExtractor RS v1.1.0 - extracts HII database from binary files into human-readable text
Usage: ifrextractor file.bin
```

If you want to, you can test `ifrextractor` on any of our previously generated drivers that populate forms to better understand its output. But I think we can just use it now in our investigation and understand its output along the way.

Let's launch `ifrextractor` on every generated package list file:
```
$ find ~/disk/ -name "00*_*" -type f -exec ./target/debug/ifrextractor {} \;
Extracting all UEFI HII form packages using en-US UEFI HII string packages
No IFR data found
No IFR data found
No IFR data found
No IFR data found
No IFR data found
No IFR data found
Extracting all UEFI HII form packages using en-US UEFI HII string packages
No IFR data found
Extracting all UEFI HII form packages using en-US UEFI HII string packages
No IFR data found
No IFR data found
No IFR data found
No IFR data found
No IFR data found
No IFR data found
No IFR data found
Extracting all UEFI HII form packages using en-US UEFI HII string packages
Extracting all UEFI HII form packages using en-US UEFI HII string packages
No IFR data found
Extracting all UEFI HII form packages using en-US UEFI HII string packages
```

As you can see for every file with the IFR data inside the `ifrextractor` has created a `*.txt` file with the decoded form data.
```
$ ls ~disk/*.txt
disk/0004_EBF8ED7C-0DD1-4787-84F1-F48D537DCACF.0.0.en-US.ifr.txt
disk/0004_EBF8ED7C-0DD1-4787-84F1-F48D537DCACF.1.0.en-US.ifr.txt
disk/0005_FE561596-E6BF-41A6-8376-C72B719874D0.0.0.en-US.ifr.txt
disk/0006_2A46715F-3581-4A55-8E73-2B769AAA30C5.0.0.en-US.ifr.txt
disk/0009_D9DCC5DF-4007-435E-9098-8970935504B2.0.0.en-US.ifr.txt
disk/0011_4B47D616-A8D6-4552-9D44-CCAD2E0F4CF9.0.0.en-US.ifr.txt
disk/0020_269D7962-BADC-44DA-815A-4AF4F293F3E0.0.0.en-US.ifr.txt
```

Let's grep the name of our target formset that we saw in the menu in these files:
```
$ grep "Form with hidden settings" ~/disk/*.txt
../20_269D7962-BADC-44DA-815A-4AF4F293F3E0.0.0.en-US.ifr.txt:   Form FormId: 0x1, Title: "Form with hidden settings"
```

Now let's investigate this file:
```
$ cat disk/0020_269D7962-BADC-44DA-815A-4AF4F293F3E0.0.0.en-US.ifr.txt
Program version: 1.5.1, Extraction mode: UEFI
FormSet Guid: 98259761-6B0B-4531-A74B-67AF5D8A8153, Title: "Formset with hidden settings", Help: "Formset with hidden settings"
        DefaultStore DefaultId: 0x0, Name: "Standard default"
        DefaultStore DefaultId: 0x1, Name: ""
        VarStore Guid: 3F5996A7-8B56-472C-9538-475F77157C84, VarStoreId: 0x1, Size: 0x4, Name: "FormData"
        VarStoreEfi Guid: 01539E15-2A73-4AAF-9D3E-437BFABB4666, VarStoreId: 0x2, Attributes: 0x3, Size: 0x4, Name: "FormEfiData"
        Form FormId: 0x1, Title: "Form with hidden settings"
                SuppressIf
                        True
                        CheckBox Prompt: "Show varstore settings", Help: "Show varstore settings", QuestionFlags: 0x0, QuestionId: 0x1, VarStoreId: 0x1, VarOffset: 0x0, Flags: 0x0, Default: Disabled, MfgDefault: Disabled
                                Default DefaultId: 0x0 Value: 0
                        End
                End
                SuppressIf
                        EqIdVal QuestionId: 0x1, Value: 0x0
                        Numeric Prompt: "Numeric prompt", Help: "Numeric help", QuestionFlags: 0x0, QuestionId: 0x2, VarStoreId: 0x1, VarOffset: 0x1, Flags: 0x21, Size: 16, Min: 0x0, Max: 0xA, Step: 0x1
                                Default DefaultId: 0x0 Value: 5
                        End
                        OneOf Prompt: "OneOf list prompt", Help: "OneOf list help", QuestionFlags: 0x0, QuestionId: 0x3, VarStoreId: 0x1, VarOffset: 0x3, Flags: 0x10, Size: 8, Min: 0x0, Max: 0x55, Step: 0x0
                                OneOfOption Option: "OneOf list option 1" Value: 0
                                OneOfOption Option: "OneOf list option 2" Value: 51, Default
                                OneOfOption Option: "OneOf list option 3" Value: 85
                        End
                End
                GrayOutIf
                        True
                        CheckBox Prompt: "Enable efivarstorage settings", Help: "Enable efivarstorage settings", QuestionFlags: 0x0, QuestionId: 0x4, VarStoreId: 0x2, VarOffset: 0x0, Flags: 0x0, Default: Disabled, MfgDefault: Disabled
                                Default DefaultId: 0x0 Value: 0
                        End
                End
                GrayOutIf
                        EqIdVal QuestionId: 0x4, Value: 0x0
                        Numeric Prompt: "Numeric EFI prompt", Help: "Numeric EFI help", QuestionFlags: 0x0, QuestionId: 0x5, VarStoreId: 0x2, VarOffset: 0x1, Flags: 0x21, Size: 16, Min: 0x0, Max: 0xA, Step: 0x1
                                Default DefaultId: 0x0 Value: 6
                        End
                        OneOf Prompt: "OneOf list EFI prompt", Help: "OneOf list EFI help", QuestionFlags: 0x0, QuestionId: 0x6, VarStoreId: 0x2, VarOffset: 0x3, Flags: 0x10, Size: 8, Min: 0x0, Max: 0x55, Step: 0x0
                                OneOfOption Option: "OneOf list option 1" Value: 0
                                OneOfOption Option: "OneOf list option 2" Value: 51
                                OneOfOption Option: "OneOf list option 3" Value: 85, Default
                        End
                End
        End
End
```

Here we can see 2 groups of settings:
- suppressed TRUE checkbox [varstore] that unlocks the 2 settings below:
  - numeric option [varstore]
  - oneof option [varstore]

- grayoutif TRUE checkbox [efivarstore] that unlocks the 2 settings below:
  - numeric option [efivarstore]
  - oneof option [efivarstore]

Now when we know everything about out target form, let's try to modify its hidden settings.

# Use `HIIConfig.efi` application to modify settings

As you can see from the `20_269D7962-BADC-44DA-815A-4AF4F293F3E0.0.0.en-US.ifr.txt` output, the hidden settings are stored in these `varstore`/`efivarstore` elements:
- `VarStore Guid: 3F5996A7-8B56-472C-9538-475F77157C84, VarStoreId: 0x1, Size: 0x4, Name: "FormData"`
- `VarStoreEfi Guid: 01539E15-2A73-4AAF-9D3E-437BFABB4666, VarStoreId: 0x2, Attributes: 0x3, Size: 0x4, Name: "FormEfiData"`

Now if you look at `VarStoreId`/`VarOffset` fields in form elements description, you can recreate the storages internal structure:

| Element  | Menu string                     | VarStoreId | VarOffset (in bytes) | Size (in bytes)                              |
| -------- | ------------------------------- | ---------- | -------------------- | -------------------------------------------- |
| Checkbox | "Show varstore settings"        | 1          | 0                    | 1 byte  (implicitly since it is a checkbox)  |
| Numeric  | "Numeric prompt"                | 1          | 1                    | 2 bytes (since element has `Size: 16`)       |
| OneOf    | "OneOf list prompt"             | 1          | 3                    | 1 bytes (since element has `Size: 8`)        |
| Checkbox | "Enable efivarstorage settings" | 2          | 0                    | 1 byte  (implicitly since it is a checkbox)  |
| Numeric  | "Numeric EFI prompt"            | 2          | 1                    | 2 bytes (since element has `Size: 16`)       |
| OneOf    | "OneOf list EFI prompt"         | 2          | 3                    | 1 bytes (since element has `Size: 8`)        |


You can use `HIIConfig.efi dump` to print all the HII database settings. If you'll search for the `varstore` GUID, you'll find the following settings:
```
GUID=a796593f568b2c479538475f77157c84 (3F5996A7-8B56-472C-9538-475F77157C84)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400 (VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB))
OFFSET=0  WIDTH=0000000000000004  VALUE=33000500
00 05 00 33                                      | ...3

GUID=a796593f568b2c479538475f77157c84 (3F5996A7-8B56-472C-9538-475F77157C84)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400 (VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB))
ALTCFG=0000
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0005
05 00                                            | ..
OFFSET=0003  WIDTH=0001  VALUE=33
33                                               | 3

GUID=a796593f568b2c479538475f77157c84 (3F5996A7-8B56-472C-9538-475F77157C84)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400 (VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB))
ALTCFG=0001
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0005
05 00                                            | ..
OFFSET=0003  WIDTH=0001  VALUE=33
33                                               | 3
```

Here you can see that the current `FormData` varstorage values are currently equal to their defaults:
- FormData.Checkbox["Show varstore settings"] = 0
- FormData.Numeric["Numeric prompt"] = 0005
- FromData.OneOf["OneOf list prompt"] = 33

Also we can grab the `PATH` value from the output (`VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB)`) and use it in `HIIConfig.efi route`/`HIIConfig.efi extract` command calls.

Unfortunately the `efivarstore` settings are not present in the output, since the implementation of the `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExportConfig()` is buggy in the current edk2 code ([https://bugzilla.tianocore.org/show_bug.cgi?id=4639](https://bugzilla.tianocore.org/show_bug.cgi?id=4639)). But we'll deal with it later. Now let's focus on the `varstorage` elements.

Let's show the current settings for the "Show varstore settings" checkbox:
```
FS0:\> HIIConfig.efi extract 3F5996A7-8B56-472C-9538-475F77157C84 FormData VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB)

Request: GUID=a796593f568b2c479538475f77157c84&NAME=0046006f0072006d0044006100740061&PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400
Response: GUID=a796593f568b2c479538475f77157c84&NAME=0046006f0072006d0044006100740061&PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400&OFFSET=0000&WIDTH=0001&VALUE=00&OFFSET=0001&WIDTH=0002&VALUE=0005&OFFSET=0003&WIDTH=0001&VALUE=33&GUID=a796593f568b2c479538475f77157c84&NAME=0046006f0072006d0044006100740061&PATH=01041<...>


GUID=a796593f568b2c479538475f77157c84 (3F5996A7-8B56-472C-9538-475F77157C84)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400 (VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB))
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0005
05 00                                            | ..
OFFSET=0003  WIDTH=0001  VALUE=33
33                                               | 3

GUID=a796593f568b2c479538475f77157c84 (3F5996A7-8B56-472C-9538-475F77157C84)
NAME=0046006f0072006d0044006100740061 (FormData)
PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400 (VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB))
ALTCFG=0000
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0005
05 00                                            | ..
OFFSET=0003  WIDTH=0001  VALUE=33
33                                               | 3

GUID=a796593f568b2c479538475f77157c84 (3F5996A7-8B56-472C-9538-475F77157C84)
NAME=0046006f0072006d0044006100740061 (FormData)
FS0:\> 041400975b3d3fe8484a4f851e82e650930aeb7fff0400 (VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB))
ALTCFG=0001
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0005
05 00                                            | ..
OFFSET=0003  WIDTH=0001  VALUE=33
33                                               | 3
```
The output for the setting is practically the same as in the `HIIConfig.efi dump` output, but with that command we've verified that we can successfully get individual setting.

Now let's enable the checkbox:
```
FS0:\> HIIConfig.efi route 3F5996A7-8B56-472C-9538-475F77157C84 FormData VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB) 0000 0001 01

Request: GUID=a796593f568b2c479538475f77157c84&NAME=0046006f0072006d0044006100740061&PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400&OFFSET=0000&WIDTH=0001&VALUE=01
```

You can verify that the suppressed settings are now shown in the driver form!

![Form2](Form2.png?raw=true "Form2")

Now let's think what we can do about `efivarstore` settings.

Because of the aforementioned bug we can't export `evivarstore` settings with the `HIIConfig.efi dump`, but we still can try to extract/route them. The problem is that to use `HIIConfig extract/route` we need to know `DevicePath` of the storage. Since our `varstore` and `efivarstore` are created by the same driver, let's try to use the same `VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB)` to extract "Enable efivarstorage settings" checkbox:
```
FS0:\> HIIConfig.efi extract 01539E15-2A73-4AAF-9D3E-437BFABB4666 FormEfiData VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB)

Request: GUID=159e5301732aaf4a9d3e437bfabb4666&NAME=0046006f0072006d0045006600690044006100740061&PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400
Response: GUID=159e5301732aaf4a9d3e437bfabb4666&NAME=0046006f0072006d0045006600690044006100740061&PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400&OFFSET=0000&WIDTH=0001&VALUE=01&OFFSET=0001&WIDTH=0002&VALUE=0002&OFFSET=0003&WIDTH=0001&VALUE=00&GUID=159e5301732aaf4a9d3e437bfabb4666&NAME=0046006f0072006d004500660069004<...>


GUID=159e5301732aaf4a9d3e437bfabb4666 (01539E15-2A73-4AAF-9D3E-437BFABB4666)
NAME=0046006f0072006d0045006600690044006100740061 (FormEfiData)
PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400 (VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB))
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0006
06 00                                            | ..
OFFSET=0003  WIDTH=0001  VALUE=55
55                                               | U

GUID=159e5301732aaf4a9d3e437bfabb4666 (01539E15-2A73-4AAF-9D3E-437BFABB4666)
NAME=0046006f0072006d0045006600690044006100740061 (FormEfiData)
PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400 (VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB))
ALTCFG=0000
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0006
06 00                                            | ..
OFFSET=0003  WIDTH=0001  VALUE=55
55                                               | U

GUID=159e5301732aaf4a9d3e437bfabb4666 (01539E15-2A73-4AAF-9D3E-437BFABB4666)
NAME=0046006f0072006d0045006600690044006100740061 (FormEfiData)
FS0:\> 041400975b3d3fe8484a4f851e82e650930aeb7fff0400 (VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB))
ALTCFG=0001
OFFSET=0000  WIDTH=0001  VALUE=00
00                                               | .
OFFSET=0001  WIDTH=0002  VALUE=0006
06 00                                            | ..
OFFSET=0003  WIDTH=0001  VALUE=55
55                                               | U
```
Luckilly in this particular case we were able to find `efivarstore` settings. If it is not the case we'll discuss alternative way in the `Another way to change 'efivarstore' settings` paragraph below.

Let's try to change "Enable efivarstorage settings" checkbox value:
```
FS0:\> HIIConfig.efi route 01539E15-2A73-4AAF-9D3E-437BFABB4666 FormEfiData VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB) 0000 0001 01

Request: GUID=159e5301732aaf4a9d3e437bfabb4666&NAME=0046006f0072006d0045006600690044006100740061&PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400&OFFSET=0000&WIDTH=0001&VALUE=01
```

Now exit to the menu and see our changes:

![Form3](Form3.png?raw=true "Form3")

As you can see we now able to see both `grayoutif` and `suppressif` hidden settings. Now you can successfully modify the necessary settings via the menu!

# What if the settings can't be shown in the menu?

In our example by setting the checkboxes we've opened the hidden settings for modification from the menu directly. But that is not always the case, sometimes there is no such checkboxes, and the hidden settings are always suppressed/grayout via `suppressif TRUE`/`grayoutif TRUE` statements.

In such cases you can use `HIIConfig.efi route` to modify the necessary settings directly and verify the changes via the `HIIConfig.efi extract`.

For example let's modify `FromData.OneOf["OneOf list prompt"]` from option2 to option3 and `FromData.Numeric["Numeric EFI prompt"]` from 6 to 3.
```
FS0:\> HIIConfig.efi route 3F5996A7-8B56-472C-9538-475F77157C84 FormData VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB) 0003 0001 55
Request: GUID=a796593f568b2c479538475f77157c84&NAME=0046006f0072006d0044006100740061&PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400&OFFSET=0003&WIDTH=0001&VALUE=55
FS0:\> HIIConfig.efi route 01539E15-2A73-4AAF-9D3E-437BFABB4666 FormEfiData VenHw(3F3D5B97-48E8-4F4A-851E-82E650930AEB) 0001 0002 0003
Request: GUID=159e5301732aaf4a9d3e437bfabb4666&NAME=0046006f0072006d0045006600690044006100740061&PATH=01041400975b3d3fe8484a4f851e82e650930aeb7fff0400&OFFSET=0001&WIDTH=0002&VALUE=0003
```

If you'll enter the form now, you'll see that the values were successfully updated:

![Form4](Form4.png?raw=true "Form4")

# Another way to change `efivarstore` settings

Because of the bug in the edk2 code of the `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExportConfig()` implementation we can't dump `efivarstore` storages and therefore get their device path. In the example above we've got lucky and were able to just use devicepath from the neighboor `varstore` storage. But what we can do if it is not the case and `efivarstore` has unknown device path?

In this case we can utilize shell command `dmpstore` to dump and modify `efivarstore` underlying EFI variable directly. From the earlier lessons we already know how to change EFI variables like that, but for completeness of this lesson let's repeat the process step by step.

To show the current values inside the storage we can use:
```
FS0:\> dmpstore -guid 01539E15-2A73-4AAF-9D3E-437BFABB4666
Variable NV+BS '01539E15-2A73-4AAF-9D3E-437BFABB4666:FormEfiData' DataSize = 0x04
  00000000: 01 03 00 55                                      *...U*
```

Now let's save the dump
```
FS0:\> dmpstore -guid 01539E15-2A73-4AAF-9D3E-437BFABB4666 -s FormEfiData
Save variable to file: FormEfiData.
Variable NV+BS '01539E15-2A73-4AAF-9D3E-437BFABB4666:FormEfiData' DataSize = 0x04
```

And update it via the shell `hexedit` command:
```
FS0:\> hexedit FormEfiData
```

The storage settings are near the end of the file:
![dmpstore1](dmpstore1.png?raw=true "dmpstore1")

Modify `0300` to `0700` to change the value of the numeric option:

![dmpstore2](dmpstore2.png?raw=true "dmpstore2")

Now use `Ctrl-S` to save the updated buffer and `Ctrl-Q` to exit the editor.

As you might remember from earlier lessons to correctly load the updated variable we need to recalculate the checksum inside the file. For that we can use our `UpdateDmpstoreDump.efi` application that we have written earlier.
```
FS0:\> UpdateDmpstoreDump.efi FormEfiData
```

If you check the file now, you can see that the 4-byte checksum at the end was updated:

![dmpstorer3](dmpstore3.png?raw=true "dmpstore3")

Now load the updated file to modify the EFI variable content:
```
FS0:\> dmpstore -guid 01539E15-2A73-4AAF-9D3E-437BFABB4666 -l FormEfiData
Load and set variables from file: FormEfiData.
Variable NV+BS '01539E15-2A73-4AAF-9D3E-437BFABB4666:FormEfiData' DataSize = 0x04
```

Exit to the form and verify that the changes are indeed present in the menu:

![Form5](Form5.png?raw=true "Form5")

# Updating hidden settings in the real world scenario

In this lesson we've investigated how we can find and modify hidden settings in the HII database. But I want to point out that when you find something like that in the real system there is no guarantee that the found settings are actually used in the UEFI code.

For example you've found an option "PCIe speed" and change it to "Gen1". When you reboot, the setting is persistent, so everything seems correct and you expect that PCIe now runs at Gen1 speed. But the problem is that you don't see any changes in the PCIe functionality. Unfortunately such scenario is possible. You are changing some options in the settings storage, but no one says that these settings are used in any way in the driver code.

But still worth a try!

# Alternative tools for BIOS settings modification

In this lesson we've used `ShowHIIext.efi/HIIConfig.efi`/`UpdateDmpstoreDump.efi` applications to modify BIOS settings. These applications we've developed earlier from scratch.

But we are not the first ones who tries to tweak hidden BIOS settings.

Most know solution is using patched version of the GRUB bootloader. This patched version provides additional command `setup_var` and its variations that can be used to modify BIOS settings.

You can check the original solution by @datasone [grub-mod-setup_var](https://github.com/datasone/grub-mod-setup_var).

But right now it is adviced to use [setup_var.efi](https://github.com/datasone/setup_var.efi) instead. This is an advanced version of the program rewritten in Rust.

# `HiddenSettings` driver code

As I promised here is a form code for the `HiddenSettings` driver.

`UefiLessonsPkg/HiddenSettings/Form.vfr`:
```
#include <Uefi/UefiMultiPhase.h>
#include "Data.h"

formset
  guid     = FORMSET_GUID,
  title    = STRING_TOKEN(FORMSET_TITLE),
  help     = STRING_TOKEN(FORMSET_HELP),

  varstore VARIABLE_STRUCTURE,
    name  = FormData,
    guid  = STORAGE_GUID;

  efivarstore VARIABLE_STRUCTURE,
    attribute = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
    name  = FormEfiData,
    guid  = STORAGE_EFI_GUID;

  defaultstore StandardDefault,
    prompt      = STRING_TOKEN(STANDARD_DEFAULT_PROMPT),
    attribute   = 0x0000;

  form
    formid = 1,
    title = STRING_TOKEN(FORMID1_TITLE);

suppressif TRUE;
    checkbox
      varid = FormData.CheckboxValue,
      prompt = STRING_TOKEN(CHECKBOX_PROMPT),
      help = STRING_TOKEN(CHECKBOX_HELP),
      default = FALSE, defaultstore = StandardDefault,
    endcheckbox;
endif;

suppressif ideqval FormData.CheckboxValue == 0;
    numeric
      name = NumericQuestion,
      varid = FormData.NumericValue,
      prompt = STRING_TOKEN(NUMERIC_PROMPT),
      help = STRING_TOKEN(NUMERIC_HELP),
      flags = NUMERIC_SIZE_2 | DISPLAY_UINT_HEX,
      minimum = 0,
      maximum = 10,
      step = 1,
      default = 5, defaultstore = StandardDefault,
    endnumeric;

    oneof
      name = OneOfQuestion,
      varid = FormData.OneOfValue,
      prompt = STRING_TOKEN(ONEOF_PROMPT),
      help = STRING_TOKEN(ONEOF_HELP),
      option text = STRING_TOKEN(ONEOF_OPTION1), value = 0x00, flags = 0;
      option text = STRING_TOKEN(ONEOF_OPTION2), value = 0x33, flags = DEFAULT;
      option text = STRING_TOKEN(ONEOF_OPTION3), value = 0x55, flags = 0;
    endoneof;
endif;

grayoutif TRUE;
    checkbox
      varid = FormEfiData.CheckboxValue,
      prompt = STRING_TOKEN(CHECKBOX_EFI_PROMPT),
      help = STRING_TOKEN(CHECKBOX_EFI_HELP),
      default = FALSE, defaultstore = StandardDefault,
    endcheckbox;
endif;

grayoutif ideqval FormEfiData.CheckboxValue == 0;
    numeric
      name = NumericEfiQuestion,
      varid = FormEfiData.NumericValue,
      prompt = STRING_TOKEN(NUMERIC_EFI_PROMPT),
      help = STRING_TOKEN(NUMERIC_EFI_HELP),
      flags = NUMERIC_SIZE_2 | DISPLAY_UINT_HEX,
      minimum = 0,
      maximum = 10,
      step = 1,
      default = 6, defaultstore = StandardDefault,
    endnumeric;

    oneof
      name = OneOfEfiQuestion,
      varid = FormEfiData.OneOfValue,
      prompt = STRING_TOKEN(ONEOF_EFI_PROMPT),
      help = STRING_TOKEN(ONEOF_EFI_HELP),
      option text = STRING_TOKEN(ONEOF_OPTION1), value = 0x00, flags = 0;
      option text = STRING_TOKEN(ONEOF_OPTION2), value = 0x33, flags = 0;
      option text = STRING_TOKEN(ONEOF_OPTION3), value = 0x55, flags = DEFAULT;
    endoneof;
endif;

  endform;
endformset;
```

`UefiLessonsPkg/HiddenSettings/Data.h`:
```
#ifndef _DATA_H_
#define _DATA_H_

#define FORMSET_GUID  {0x98259761, 0x6b0b, 0x4531, {0xa7, 0x4b, 0x67, 0xaf, 0x5d, 0x8a, 0x81, 0x53}}
#define DATAPATH_GUID {0x3f3d5b97, 0x48e8, 0x4f4a, {0x85, 0x1e, 0x82, 0xe6, 0x50, 0x93, 0x0a, 0xeb}}
#define STORAGE_GUID  {0x3f5996a7, 0x8b56, 0x472c, {0x95, 0x38, 0x47, 0x5f, 0x77, 0x15, 0x7c, 0x84}}
#define STORAGE_EFI_GUID  {0x01539e15, 0x2a73, 0x4aaf, {0x9d, 0x3e, 0x43, 0x7b, 0xfa, 0xbb, 0x46, 0x66}}

#pragma pack(1)
typedef struct {
  UINT8 CheckboxValue;
  UINT16 NumericValue;
  UINT8 OneOfValue;
} VARIABLE_STRUCTURE;
#pragma pack()

#endif
```

You can compare the original VFR with the decoded file from the `ifrextractor`. There is nothing unusual in the C source code file. But if you want to you can check the source code [here](/UefiLessonsPkg/HiddenSettings).

