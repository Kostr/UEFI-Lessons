



```
$ git diff OvmfPkg/OvmfPkgDefines.fdf.inc
diff --git a/OvmfPkg/OvmfPkgDefines.fdf.inc b/OvmfPkg/OvmfPkgDefines.fdf.inc
index 35fd454b97..d12ee97ea5 100644
--- a/OvmfPkg/OvmfPkgDefines.fdf.inc
+++ b/OvmfPkg/OvmfPkgDefines.fdf.inc
@@ -61,7 +61,10 @@ DEFINE FW_BLOCKS         = 0x400
 DEFINE CODE_BASE_ADDRESS = 0xFFC84000
 DEFINE CODE_SIZE         = 0x0037C000
 DEFINE CODE_BLOCKS       = 0x37C
-DEFINE FVMAIN_SIZE       = 0x00348000
+DEFINE VPD_OFFSET        = 0x84000
+DEFINE VPD_SIZE          = 0x01000
+DEFINE FVMAIN_OFFSET     = 0x85000
+DEFINE FVMAIN_SIZE       = 0x00347000
 DEFINE SECFV_OFFSET      = 0x003CC000
 DEFINE SECFV_SIZE        = 0x34000
 !endif
```

If we want to create VPD file we need to add a special define `VPD_TOOL_GUID` to the DSC `[Defines]` section. This define determines a tool which would be used for VPD generation. EDK2 comes with one such tool, so let's try to use it.
 
Add this to the `OvmfPkg/OvmfPkgX64.dsc` file:
```
[Defines]
  ...
  VPD_TOOL_GUID                  = 8C3D856A-9BE6-468E-850A-24F7A8D38E08
```

This GUID corresponds to the GUID of the tool `VPDTOOL` defined in the `Conf/tools_def.txt`:
```
##################
# BPDG tool definitions
##################
*_*_*_VPDTOOL_PATH         = BPDG
*_*_*_VPDTOOL_GUID         = 8C3D856A-9BE6-468E-850A-24F7A8D38E08
```
The tool `VPDTOOL` is a Python module from the folder [`BaseTools/Source/Python/BPDG/`](https://github.com/tianocore/edk2/blob/master/BaseTools/Source/Python/BPDG/) which is used via [`BaseTools/BinWrappers/PosixLike/BPDG`](https://github.com/tianocore/edk2/blob/master/BaseTools/BinWrappers/PosixLike/BPDG) wrapper on Linux. As a wrapper folder `BaseTools/BinWrappers/PosixLike` gets added to the environment `PATH` variable after the `. edksetup.sh` command, we can simply declare path for `VPDTOOL` as `BPDG`.


```
[PcdsFixedAtBuild]
  gEfiMdeModulePkgTokenSpaceGuid.PcdVpdBaseAddress|0x0
```

```
[PcdsDynamicVpd]
   gUefiLessonsPkgTokenSpaceGuid.PcdMyDynamicVar32   | 0x40 | 0x55555555
   gUefiLessonsPkgTokenSpaceGuid.PcdMyDynamicVar32_1 | * | 0x66666666
```

```
$ git diff OvmfPkg/OvmfPkgX64.fdf
diff --git a/OvmfPkg/OvmfPkgX64.fdf b/OvmfPkg/OvmfPkgX64.fdf
index 5fa8c08958..be0f4b52ff 100644
--- a/OvmfPkg/OvmfPkgX64.fdf
+++ b/OvmfPkg/OvmfPkgX64.fdf
@@ -26,7 +26,12 @@ NumBlocks     = $(FW_BLOCKS)

 !include VarStore.fdf.inc

-$(VARS_SIZE)|$(FVMAIN_SIZE)
+$(VPD_OFFSET)|$(VPD_SIZE)
+gEfiMdeModulePkgTokenSpaceGuid.PcdVpdBaseAddress
+FILE = $(OUTPUT_DIRECTORY)/$(TARGET)_$(TOOL_CHAIN_TAG)/FV/8C3D856A-9BE6-468E-850A-24F7A8D38E08.bin
+
+#$(VARS_SIZE)|$(FVMAIN_SIZE)
+$(FVMAIN_OFFSET)|$(FVMAIN_SIZE)
 FV = FVMAIN_COMPACT
```

# Links

[EDK II Build System Output File Format](https://edk2-docs.gitbook.io/edk-ii-dsc-specification/appendix_d_vpd_data_files/d1_edk_ii_build_system_output_file_format)

[GUIDed Tools and PCD VPD Data](https://edk2-docs.gitbook.io/edk-ii-build-specification/7_build_environment/73_guided_tools)
