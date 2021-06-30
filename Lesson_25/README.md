Summary table:

|               | Fixed at build                    | Feature Flag                      | Patchable                          | Dynamic                                                      | DynamicEx                                                          |
|---------------|-----------------------------------|-----------------------------------|------------------------------------|--------------------------------------------------------------|--------------------------------------------------------------------|
| .dec section: | [PcdsFixedAtBuild]                | [PcdsFeatureFlag]                 | [PcdsPatchableInModule]            | [PcdsDynamic]                                                | [PcdsDynamicEx]                                                    |
| .dsc section: | [PcdsFixedAtBuild]                | [PcdsFeatureFlag]                 | [PcdsPatchableInModule]            | [PcdsDynamicDefault]<br>[PcdsDynamicHii]<br>[PcdsDynamicVpd] | [PcdsDynamicExDefault]<br>[PcdsDynamicExHii]<br>[PcdsDynamicExVpd] |
| .inf section: | [FixedPcd]<br>[Pcd]               | [FeaturePcd]<br>[Pcd]             | [PatchPcd]<br>[Pcd]                | [Pcd]                                                        | [PcdEx]                                                            |
| Get:          | PcdGet<Type><br>FixedPcdGet<Type> | PcdGetBool<br>FeaturePcdGet       | PcdGet<Type><br>PatchPcdGet<Type>  | PcdGet<Type>                                                 | PcdGet<Type><br>PcdGetEx<Type>                                     |
| Set:          | -                                 | -                                 | PcdSet<Type>S<br>PatchPcdSet<Type> | PcdSet<Type>S                                                | PcdSet<Type>S<br>PcdSetEx<Type>S                                   |
| Scope:        | Every module have its own copy | Every module have its own copy | Every module have its own copy  | Global for<br>platform                                       | Global for<br>platform                                             |


\<Type\> = 8|16|32|64|Bool|Ptr
