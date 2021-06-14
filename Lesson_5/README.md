To build modules in our package we've used something like this:
```
$ build --platform=UefiLessonsPkg/UefiLessonsPkg.dsc \
        --module=UefiLessonsPkg/HelloWorld/HelloWorld.inf \
        --arch=X64 \
        --buildtarget=RELEASE \
        --tagname=GCC5
```

This is kinda long string. Let's simplify it.

First we can omit `module` option from the command and always compile our package entirely with all its modules:

```
$ build --platform=UefiLessonsPkg/UefiLessonsPkg.dsc \
        --arch=X64 \
        --buildtarget=RELEASE \
        --tagname=GCC5
```

When we initiate our environment with:
```
. edk2setup.sh
```
`Conf` directory is created in the edk2 folder. This directory is populated with a bunch of files.
Also the `CONF_PATH` environment variable is created which points to the newly created `Conf` folder.

Right now we are interested in the `Conf/target.txt` file.
By default it has many comments (lines that start with `#` symbol) and is prepopulated with these values:
```
ACTIVE_PLATFORM       = EmulatorPkg/EmulatorPkg.dsc
TARGET                = DEBUG
TARGET_ARCH           = IA32
TOOL_CHAIN_CONF       = Conf/tools_def.txt
TOOL_CHAIN_TAG        = VS2015x86
BUILD_RULE_CONF = Conf/build_rule.txt
```

We can put all our build command options in this file to simplify our everyday life.
```
ACTIVE_PLATFORM       = UefiLessonsPkg/UefiLessonsPkg.dsc
TARGET                = RELEASE
TARGET_ARCH           = X64
TOOL_CHAIN_CONF       = Conf/tools_def.txt
TOOL_CHAIN_TAG        = GCC5
BUILD_RULE_CONF = Conf/build_rule.txt
```

After that we could build our package with all its modules with the one single command:
```
build
```

