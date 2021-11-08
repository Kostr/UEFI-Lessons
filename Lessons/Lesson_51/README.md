Every command in UEFI Shell has a help message which you can read if you supply command with a `-?` argument. For example:
```
Shell> reset -?
Resets the system.

RESET [-w [string]]
RESET [-s [string]]
RESET [-c [string]]

  -s     - Performs a shutdown.
  -w     - Performs a warm boot.
  -c     - Performs a cold boot.
  string - Describes a reason for the reset.

NOTES:
  1. This command resets the system.
  2. The default is to perform a cold reset unless the -w parameter is
     specified.
  3. If a reset string is specified, it is passed into the Reset()
     function, and the system records the reason for the system reset.
```

In this lesson we would investigate how we can add this help/man functionality to our application.

The Shell module responsible for the man finding and parsing is `ShellManParser`:
- https://github.com/tianocore/edk2/blob/master/ShellPkg/Application/Shell/ShellManParser.h
- https://github.com/tianocore/edk2/blob/master/ShellPkg/Application/Shell/ShellManParser.c

If you look close at the module `ProcessManFile` function you'll see that besides everything it tries to:
- open image protocol by `gEfiHiiPackageListProtocolGuid`
- if found, register recieved Package list with `gHiiDatabase->NewPackageList`
- go through all possible string IDs with a help of `HiiGetString`
- if `ManFileFindTitleSection` function returns true for some string, execute `ManFileFindSections`

`ManFileFindTitleSection` basically searches for a string that has a special man formatting.

You could learn more about this formatting from the UEFI Shell specification (https://uefi.org/sites/default/files/resources/UEFI_Shell_Spec_2_0.pdf)

![Manual Page Syntax1](Man1.png?raw=true "Manual page1")
![Manual Page Syntax2](Man2.png?raw=true "Manual page2")

# Create an app with a minimal manual

Let's create an application with a manual.

Use shell script to create app from template:
```
./createNewApp.sh HIIStringsMan
```

Add newly created app to the UefiLessonsPkg/UefiLessonsPkg.dsc:
```
[Components]
  UefiLessonsPkg/HIIStringsMan/HIIStringsMan.inf
```

As you remember `ShellManParser` searched for the manual strings in the `gEfiHiiPackageListProtocolGuid` protocol data. Therefore our manual strings we need to include directly in the image resource section UefiLessonsPkg/HIIStringsMan/HIIStringsMan.inf:
```
[Defines]
  ...
  UEFI_HII_RESOURCE_SECTION      = TRUE

[Sources]
  ...
  Strings.uni
```

Here is a minimal content for our strings file UefiLessonsPkg/HIIStringsMan/Strings.uni:
```
#langdef en-US "English"

#string STR_HELP        #language en-US ""
".TH HIIStringsMan 0 "Simple application with a manual inside."\r\n"
".SH NAME\r\n"
"HIIStringsMan application.\r\n"
```

If you build our application and try to execute it, there wouldn't be any help now:
```
FS0:\> HIIStringsMan.efi -?
No help could be found for command 'HIIStringsMan.efi'.
FS0:\>
```

This is because our program don't reference string tokens and they got optimized in the build process `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIStringsMan/HIIStringsMan/DEBUG/HIIStringsManStrDefs.h`:
```
//
//Unicode String ID
//
// #define $LANGUAGE_NAME                                       0x0000 // not referenced
// #define $PRINTABLE_LANGUAGE_NAME                             0x0001 // not referenced
// #define STR_HELLO_WORLD_HELP_INFORMATION                     0x0002 // not referenced
```

To fix it add this string to our `UefiLessonsPkg/HIIStringsMan/HIIStringsMan.c` file:
```
GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID mStringHelpTokenId = STRING_TOKEN(STR_HELP);
```

You can verify after the build that now string is not optimized `Build/UefiLessonsPkg/RELEASE_GCC5/X64/UefiLessonsPkg/HIIStringsMan/HIIStringsMan/DEBUG/HIIStringsManStrDefs.h`:
```
//
//Unicode String ID
//
// #define $LANGUAGE_NAME                                       0x0000 // not referenced
// #define $PRINTABLE_LANGUAGE_NAME                             0x0001 // not referenced
#define STR_HELP                                             0x0002
```

If you execute our application you would get:
```
FS0:\> HIIStringsMan.efi -?
HIIStringsMan application.
```

# Expand our manual

Let's try to add all possible sections to our manual:
```
#langdef en-US "English"

#string STR_HELP        #language en-US ""
".TH HIIStringsMan 0 "Simple application with a manual inside."\r\n"
".SH NAME\r\n"
"HIIStringsMan application.\r\n"
".SH SYNOPSIS\r\n"
"This is the synopsis section.\r\n"
".SH DESCRIPTION\r\n"
"This is the description section.\r\n"
".SH OPTIONS\r\n"
"This is the options section.\r\n"
".SH RETURN VALUES\r\n"
"This is the return values section.\r\n"
".SH ENVIRONMENT VARIABLES\r\n"
"This is the section for used environment variables\r\n"
".SH FILES\r\n"
"This is the section for files associated with the subject.\r\n"
".SH EXAMPLES\r\n"
"This is the section for examples and suggestions.\r\n"
".SH ERRORS\r\n"
"This is the section for errors reported by the command.\r\n"
".SH STANDARDS\r\n"
"This is the section for conformance to applicable standards.\r\n"
".SH BUGS\r\n"
"This is the section for errors and caveats.\r\n"
".SH CATEGORY\r\n"
"This is the section for categories.\r\n"
".SH CUSTOMSECTION\r\n"
"This is an example of a custom section.\r\n"
```

If you build and execute our app now, you would get:
```
FS0:\> HIIStringsMan.efi -?
HIIStringsMan application.
This is the synopsis section.
This is the description section.
This is the options section.
This is the section for examples and suggestions.
```

As you can see not all section were printed. Let's find out why.

First of all let's investigate what happens when we add `-?` to our command. If you look at the shell sources you'll see that if shell find `-?` as one of the command arguments it redirects command and all the rest arguments to the `help` command https://github.com/tianocore/edk2/blob/master/ShellPkg/Application/Shell/Shell.c:
```
/**
  Reprocess the command line to direct all -? to the help command.
  if found, will add "help" as argv[0], and move the rest later.
  @param[in,out] CmdLine        pointer to the command line to update
**/
EFI_STATUS
DoHelpUpdate(
  IN OUT CHAR16 **CmdLine
  )
{
  ...
      if (StrStr(CurrentParameter, L"-?") == CurrentParameter) {
        CurrentParameter[0] = L' ';
        CurrentParameter[1] = L' ';
        NewCmdLineSize = StrSize(L"help ") + StrSize(*CmdLine);
        NewCommandLine = AllocateZeroPool(NewCmdLineSize);
        if (NewCommandLine == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
          break;
        }

        //
        // We know the space is sufficient since we just calculated it.
        //
        StrnCpyS(NewCommandLine, NewCmdLineSize/sizeof(CHAR16), L"help ", 5);
        StrnCatS(NewCommandLine, NewCmdLineSize/sizeof(CHAR16), *CmdLine, StrLen(*CmdLine));
        SHELL_FREE_NON_NULL(*CmdLine);
        *CmdLine = NewCommandLine;
        break;
      }
  ...
}
```

You can verify that result would be the same if you use `help` command directly to our program:
```
FS0:\> help HIIStringsMan.efi
HIIStringsMan application.
This is the synopsis section.
This is the description section.
This is the options section.
This is the section for examples and suggestions.
```

Now look at the `help` command source code https://github.com/tianocore/edk2/blob/master/ShellPkg/Library/UefiShellLevel3CommandsLib/Help.c:
```
SHELL_STATUS
EFIAPI
ShellCommandRunHelp (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
  ...
        //
        // Get the section name for the given command name
        //
        if (ShellCommandLineGetFlag(Package, L"-section")) {
          StrnCatGrow(&SectionToGetHelpOn, NULL, ShellCommandLineGetValue(Package, L"-section"), 0);
        } else if (ShellCommandLineGetFlag(Package, L"-usage")) {
          StrnCatGrow(&SectionToGetHelpOn, NULL, L"NAME,SYNOPSIS", 0);
        } else if (ShellCommandLineGetFlag(Package, L"-verbose") || ShellCommandLineGetFlag(Package, L"-v")) {
        } else {
          //
          // The output of help <command> will display NAME, SYNOPSIS, OPTIONS, DESCRIPTION, and EXAMPLES sections.
          //
          StrnCatGrow (&SectionToGetHelpOn, NULL, L"NAME,SYNOPSIS,OPTIONS,DESCRIPTION,EXAMPLES", 0);
        }
  ...
}
```

Here you can see how `help` parses its incoming arguments:
- by default the sections NAME, SYNOPSIS, OPTIONS, DESCRIPTION, EXAMPLES are printed,
- it is possible to print particular section with a `-section <SECTION NAME>` argument,
- it is possible to print all sections by supplying `-verbose` or `-v` argument

You can verify this in shell:
```
FS0:\> help HIIStringsMan.efi -v
HIIStringsMan application.
This is the synopsis section.
This is the description section.
This is the options section.
This is the return values section.
This is the section for used environment variables
This is the section for files associated with the subject.
This is the section for examples and suggestions.
This is the section for errors reported by the command.
This is the section for conformance to applicable standards.
This is the section for errors and caveats.
This is the section for categories.
This is an example of a custom section.
FS0:\> help HIIStringsMan.efi -section BUGS
This is the section for errors and caveats.
```
The same goes for the `-?`:
```
FS0:\> HIIStringsMan.efi -? -section "RETURN VALUES"
This is the return values section.
FS0:\> HIIStringsMan.efi -? -verbose
HIIStringsMan application.
This is the synopsis section.
This is the description section.
This is the options section.
This is the return values section.
This is the section for used environment variables
This is the section for files associated with the subject.
This is the section for examples and suggestions.
This is the section for errors reported by the command.
This is the section for conformance to applicable standards.
This is the section for errors and caveats.
This is the section for categories.
This is an example of a custom section.
```

# How the ShellManParser is called

We've started this lesson with the assumption that `ShellManParser` will parse our application strings. Let's investigate how `help` program would end up using this module.


In the end `ShellCommandRunHelp` would call `ShellPrintHelp` function https://github.com/tianocore/edk2/blob/master/ShellPkg/Library/UefiShellLevel3CommandsLib/Help.c:
```
SHELL_STATUS
EFIAPI
ShellCommandRunHelp (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
  ...
  Status = ShellPrintHelp(CommandToGetHelpOn, SectionToGetHelpOn, FALSE);
  ...
}
```

This function is defined https://github.com/tianocore/edk2/blob/master/ShellPkg/Library/UefiShellLib/UefiShellLib.c. It mainly call the `GetHelpText` function from the `EFI_SHELL_PROTOCOL`:
```
EFI_STATUS
EFIAPI
ShellPrintHelp (
  IN CONST CHAR16     *CommandToGetHelpOn,
  IN CONST CHAR16     *SectionToGetHelpOn,
  IN BOOLEAN          PrintCommandText
  )
{
  ...
  Status = gEfiShellProtocol->GetHelpText (CommandToGetHelpOn, SectionToGetHelpOn, &OutText);
  ...
}
```


Here is a description for this function from the UEFI Shell Specification:
```
EFI_SHELL_PROTOCOL.GetHelpText()

Summary:
Return help information about a specific command.

Prototype:
typedef
EFI_STATUS
(EFIAPI *EFI_SHELL_GET_HELP_TEXT) (
 IN CONST CHAR16 *Command,
 IN CONST CHAR16 *Sections,
 OUT CHAR16 **HelpText
 );

Parameters:
Command		Points to the null-terminated UEFI Shell command name.
Sections	Points to the null-terminated comma-delimited section names to return. If NULL, then all sections will be returned.
HelpText	On return, points to a callee-allocated buffer containing all specified help text. 
```

Prototype for the `EFI_SHELL_PROTOCOL` structure in edk2 is in the https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Protocol/Shell.h
```
typedef struct _EFI_SHELL_PROTOCOL {
  ...
  EFI_SHELL_GET_HELP_TEXT                   GetHelpText;
  ...
} EFI_SHELL_PROTOCOL;
```

And initialization for this protocol is in the https://github.com/tianocore/edk2/blob/master/ShellPkg/Application/Shell/ShellProtocol.c
```
EFI_SHELL_PROTOCOL         mShellProtocol = {
  ...
  EfiShellGetHelpText,
  ...
}
```

This function is defined in the same file above, and if you look at its definition you'll see that it is calling function from the `ProcessManFile` module.


