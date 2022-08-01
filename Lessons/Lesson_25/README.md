Here is all the PCD functions. You can find their definition in the [https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h](https://github.com/tianocore/edk2/blob/master/MdePkg/Include/Library/PcdLib.h):
```
FixedPcdGetBool (TokenName)
FixedPcdGet8    (TokenName)
FixedPcdGet16   (TokenName)
FixedPcdGet32   (TokenName)
FixedPcdGet64   (TokenName)
FixedPcdGetPtr  (TokenName)
FixedPcdGetSize (TokenName)

----------------------------------------------------------------------------------------------------

FeaturePcdGet (TokenName)

----------------------------------------------------------------------------------------------------

PatchPcdGetBool (TokenName)              PatchPcdSetBool (TokenName, Value)
PatchPcdGet8    (TokenName)              PatchPcdSet8    (TokenName, Value)
PatchPcdGet16   (TokenName)              PatchPcdSet16   (TokenName, Value)
PatchPcdGet32   (TokenName)              PatchPcdSet32   (TokenName, Value)
PatchPcdGet64   (TokenName)              PatchPcdSet64   (TokenName, Value)
PatchPcdGetPtr  (TokenName)              PatchPcdSetPtr  (TokenName, Size, Buffer)
PatchPcdGetSize (TokenName)

----------------------------------------------------------------------------------------------------

PcdGetBool      (TokenName)              PcdSetBoolS     (TokenName, Value)
PcdGet8         (TokenName)              PcdSet8S        (TokenName, Value)
PcdGet16        (TokenName)              PcdSet16S       (TokenName, Value)
PcdGet32        (TokenName)              PcdSet32S       (TokenName, Value)
PcdGet64        (TokenName)              PcdSet64S       (TokenName, Value)
PcdGetPtr       (TokenName)              PcdSetPtrS      (TokenName, SizeOfBuffer, Buffer)
PcdGetSize      (TokenName)

----------------------------------------------------------------------------------------------------

PcdGetExBool    (Guid, TokenName)        PcdSetExBoolS   (Guid, TokenName, Value)
PcdGetEx8       (Guid, TokenName)        PcdSetEx8S      (Guid, TokenName, Value)
PcdGetEx16      (Guid, TokenName)        PcdSetEx16S     (Guid, TokenName, Value)
PcdGetEx32      (Guid, TokenName)        PcdSetEx32S     (Guid, TokenName, Value)
PcdGetEx64      (Guid, TokenName)        PcdSetEx64S     (Guid, TokenName, Value)
PcdGetExPtr     (Guid, TokenName)        PcdSetExPtrS    (Guid, TokenName, SizeOfBuffer, Buffer)
PcdGetExSize    (Guid, TokenName)
```

Summary table:

|               | Fixed at build                    | Feature Flag                      | Patchable                          | Dynamic                                                      | DynamicEx                                                          |
|---------------|-----------------------------------|-----------------------------------|------------------------------------|--------------------------------------------------------------|--------------------------------------------------------------------|
| .dec section: | [PcdsFixedAtBuild]                | [PcdsFeatureFlag]                 | [PcdsPatchableInModule]            | [PcdsDynamic]                                                | [PcdsDynamicEx]                                                    |
| .dsc section: | [PcdsFixedAtBuild]                | [PcdsFeatureFlag]                 | [PcdsPatchableInModule]            | [PcdsDynamicDefault]<br>[PcdsDynamicHii]<br>[PcdsDynamicVpd] | [PcdsDynamicExDefault]<br>[PcdsDynamicExHii]<br>[PcdsDynamicExVpd] |
| .inf section: | [FixedPcd]<br>[Pcd]               | [FeaturePcd]<br>[Pcd]             | [PatchPcd]<br>[Pcd]                | [Pcd]                                                        | [PcdEx]                                                            |
| Get:          | `PcdGet<Type>`<br>`FixedPcdGet<Type>` | `PcdGetBool`<br>`FeaturePcdGet`       | `PcdGet<Type>`<br>`PatchPcdGet<Type>`  | `PcdGet<Type>`                                                 | `PcdGet<Type>`<br>`PcdGetEx<Type>`                                     |
| Set:          | -                                 | -                                 | `PcdSet<Type>S`<br>`PatchPcdSet<Type>` | `PcdSet<Type>S`                                                | `PcdSet<Type>S`<br>`PcdSetEx<Type>S`                                   |
| Scope:        | Every module have its own copy | Every module have its own copy | Every module have its own copy  | Global for<br>platform                                       | Global for<br>platform                                             |


\<Type\> = 8|16|32|64|Bool|Ptr
