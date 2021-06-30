Summary table:

|                       |  Fixed at build    |    Feature Flag    |     Patchable            |      Dynamic         |       DynamicEx        |
| --------------------- | ------------------ |--------------------|--------------------------|----------------------|------------------------|
| .dec section:         | [PcdsFixedAtBuild] |  [PcdsFeatureFlag] |  [PcdsPatchableInModule] |   [PcdsDynamic]      |  [PcdsDynamicEx]       |
| .dsc section:         | [PcdsFixedAtBuild]<br/> | [PcdsFeatureFlag]<br/> | [PcdsPatchableInModule]<br/> | [PcdsDynamicDefault]<br/> | [PcdsDynamicExDefault]<br/> |
| <br/>                 | <br/>                   | <br/>                  | <br/>                        | [PcdsDynamicHii]<br/>     | [PcdsDynamicExHii]<br/>     |
| <br/>                 | <br/>                   | <br/>                  | <br/>                        | [PcdsDynamicVpd]<br/>     | [PcdsDynamicExVpd]<br/>     |
| .inf section:<br/>    | [FixedPcd]<br/>    | [FeaturePcd]<br/> |   [PatchPcd]<br/>        |    [Pcd]<br/>         |  [PcdEx]<br/>          |
|                       | [Pcd]              | [Pcd]             |   [Pcd]                  |                       |                        |
| Get:<br/>             | PcdGet<Type><br/>  | PcdGetBool<br/>   |   PcdGet<Type><br/>      |    PcdGet<Type><br/>  |  PcdGet<Type><br/>     |
|                       | FixedPcdGet<Type>  | FeaturePcdGet     |   PatchPcdGet<Type>      |                       |  PcdGetEx<Type>        |
| Set:<br/>             | -<br/>             | -<br/>            |   PcdSet<Type>S<br/>     |    PcdSet<Type>S<br/> |  PcdSet<Type>S<br/>    |
|                       |                    |                   |   PatchPcdSet<Type>      |                       |  PcdSetEx<Type>S       |
| Scope:<br/>           | Every module have<br/>  | Every module have<br/> | Every module have<br/>       | Global for<br/>           | Global for<br/>             |
|                       | its own copy            | its own copy           | its own copy                 | platfrom                  | platfrom                    |


<Type> = 8|16|32|64|Bool|Ptr
