Let's explore when `ExtractConfig`/`RouteConfig`/`Callback` functions of our driver are called and with what arguments.

# Add support for the `DEBUG(( ... ))` statements

We can't use `Print` to output debug informaton from these functions as it would interfere with the Form Browser output. Therefore we would use `DEBUG (( ... ))` statements that would print directly to log.

To use it perform the following modifications in the `UefiLessonsPkg/UefiLessonsPkg.dsc`:
```
  [LibraryClasses]
    ...
-   DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
+   DebugLib|OvmfPkg/Library/PlatformDebugLibIoPort/PlatformDebugLibIoPort.inf
+   IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsicSev.inf

+ [PcdsFixedAtBuild]
+   gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xFF
+   gEfiMdePkgTokenSpaceGuid.PcdFixedDebugPrintErrorLevel|0xFFFFFFFF
+   gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x8000004F
```

As you remember the configurations strings can be very long, so you'll probably need to increase debug print buffer size in the [https://github.com/tianocore/edk2/blob/master/OvmfPkg/Library/PlatformDebugLibIoPort/DebugLib.c](https://github.com/tianocore/edk2/blob/master/OvmfPkg/Library/PlatformDebugLibIoPort/DebugLib.c):
```
- #define MAX_DEBUG_MESSAGE_LENGTH  0x200
+ #define MAX_DEBUG_MESSAGE_LENGTH  0x2000
```

# Investigation of `ExtractConfig`/`RouteConfig`/`Callback` behaviour

Now let's create placeholders for our functions that would only print their incoming configuration requests.

The `ExtractConfig` and `RouteConfig` functions will need some additional code. It is because their API expects that if some input was unhandled, it should be reflected in the `EFI_STRING *Progress` variable. Also in these functions we need to check incoming pointers for `NULL`, so we wouldn't assign `NULL` to something in any case.

With all of that information in mind let's create our fake protocol functions:
```cpp
STATIC
EFI_STATUS
EFIAPI
ExtractConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Request,
  OUT       EFI_STRING                      *Progress,
  OUT       EFI_STRING                      *Results
)
{
  DEBUG ((EFI_D_INFO, "ExtractConfig: Request=%s\n", Request));

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Progress = Request;

  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
RouteConfig (
  IN CONST  EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN CONST  EFI_STRING                      Configuration,
  OUT       EFI_STRING                      *Progress
)
{
  DEBUG ((EFI_D_INFO, "RouteConfig: Configuration=%s\n", Configuration));

  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Progress = Configuration;

  return EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
Callback (
  IN     CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN     EFI_BROWSER_ACTION                     Action,
  IN     EFI_QUESTION_ID                        QuestionId,
  IN     UINT8                                  Type,
  IN OUT EFI_IFR_TYPE_VALUE                     *Value,
  OUT    EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  DEBUG ((EFI_D_INFO, "Callback\n"));

  return EFI_UNSUPPORTED;
}
```

Now let's build and test our application.

To get our debug log you need to run QEMU with `debugcon` arguments:
```
qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly,file=Build/OvmfX64/DEBUG_GCC5/FV/OVMF.fd \
  -drive format=raw,file=fat:rw:~/UEFI_disk \
  -net none \
  -vnc :1 \
  -global isa-debugcon.iobase=0x402 \
  -debugcon file:debug.log
```
This would print debug output to the `debug.log` file.

Or you could just use the first stage of the `run_gdb_ovmf.sh` script. It uses `tmux` and runs the QEMU command in one pane and `tail -f debug.log` in another one:
```
./scripts/run_gdb_ovmf.sh -1
```

Wait for the UEFI shell to boot and load our driver:
```
FS0:\> load HIIFormDataElementsVarstore.efi
```

Exit to the form browser. When you enter the driver form, you'll get the following string in the output:
```
ExtractConfig: Request=GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=0&WIDTH=0024
```
Which is basically:
```
GUID=927580373a731b4f9557f22af743e8c2
NAME=0046006f0072006d0044006100740061
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400
OFFSET=0
WIDTH=0024
```
So in the `ExtractConfig` configuration string argument the Form browser requests the whole storage in one `OFFSET=/WIDTH=` pair. With the output argument `EFI_STRING* Progress` and function return code (`EFI_UNSUPPORTED`) the driver says that incoming request wasn't parsed, therefore the form browser just fills the form with the default values from the VFR:

![1](1.png?raw=true "1")

Now let's try to unset checkbox and submit the form. This would print the following string to the log:
```
RouteConfig: Configuration=GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=0000&WIDTH=0001&VALUE=00&OFFSET=0001&WIDTH=0002&VALUE=0007&OFFSET=0003&WIDTH=0014&VALUE=00660065006400200067006e0069007200740053&OFFSET=0019&WIDTH=0004&VALUE=160507e5&OFFSET=001d&WIDTH=0003&VALUE=213717&OFFSET=0020&WIDTH=0001&VALUE=55&OFFSET=0021&WIDTH=0003&VALUE=0a0b0c
```
Which is basically:
```
GUID=927580373a731b4f9557f22af743e8c2
NAME=0046006f0072006d0044006100740061
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400
OFFSET=0000 WIDTH=0001 VALUE=00
OFFSET=0001 WIDTH=0002 VALUE=0007
OFFSET=0003 WIDTH=0014 VALUE=00660065006400200067006e0069007200740053
OFFSET=0019 WIDTH=0004 VALUE=160507e5
OFFSET=001d WIDTH=0003 VALUE=213717
OFFSET=0020 WIDTH=0001 VALUE=55
OFFSET=0021 WIDTH=0003 VALUE=0a0b0c
```
So in this case the Form Browser provides values for all of the form elements in the `RouteConfig` argument. Once again with `EFI_STRING* Progress` and a function return code (`EFI_UNSUPPORTED`) we say that incoming request wasn't parsed. In this case the Form Browser prints that form submit operation failed:

![2](2.png?raw=true "2")

Although you now know the actual requests that the Form Browser makes, you shouldn't completely rely on this behaviour as it can change in the next EDKII version. Moreover, different Form Browser or another application like our `HIIConfig.efi` could use any combination of `OFFSET=/WIDTH=` pairs in it's requests. Therefore the driver must handle all the possible `OFFSET=/WIDTH=` combinations.

To finish our investigation let's check the request from the `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExportConfig()` function. To execute it we can use our `HIIConfig.efi` application with the `dump` argument:
```
FS0:\> HIIConfig.efi dump
```
This wouldn't produce any output in the shell regarding our form, but in the debug log it would print the following messages:
```
ExtractConfig: Request=<null string>
ExtractConfig: Request=GUID=927580373a731b4f9557f22af743e8c2&NAME=0046006f0072006d0044006100740061&PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400&OFFSET=0000&WIDTH=0001&OFFSET=0001&WIDTH=0002&OFFSET=0003&WIDTH=0014&OFFSET=0019&WIDTH=0004&OFFSET=001d&WIDTH=0003&OFFSET=0020&WIDTH=0001&OFFSET=0021&WIDTH=0003
```
As you see here first the `ExportConfig()` function tries to execute `ExtractConfig()` with `NULL` in place of a request configuration string. If this request fails (and in our case all requests fail), it checks IFR code in the HII database and tries to use `ExtractConfig()` with the request containing `OFFSET=/WIDTH=` pairs to all of the form elements:
```
GUID=927580373a731b4f9557f22af743e8c2
NAME=0046006f0072006d0044006100740061
PATH=01041400641982fbb4ac7b439fe666aa7ad7c5d87fff0400
OFFSET=0000 WIDTH=0001
OFFSET=0001 WIDTH=0002
OFFSET=0003 WIDTH=0014
OFFSET=0019 WIDTH=0004
OFFSET=001d WIDTH=0003
OFFSET=0020 WIDTH=0001
OFFSET=0021 WIDTH=0003
```
If you want, you can check the implementation of the `EFI_HII_CONFIG_ROUTING_PROTOCOL.ExportConfig()` function in the [https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/HiiDatabaseDxe/ConfigRouting.c](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Universal/HiiDatabaseDxe/ConfigRouting.c)

Final thing that I want to point out is that our `Callback()` function wasn't called in any of the test cases that we've performed.

