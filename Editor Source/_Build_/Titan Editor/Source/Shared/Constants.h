/******************************************************************************/
extern const Str ClientServerString;
extern const cchar8 *WorldVerSuffix,
    *ProjectsBuildPath,   // !! this will     get deleted in 'CleanAll' !!
    *ProjectsPublishPath, // !! this will     get deleted in 'CleanAll' !!
    *ProjectsCodePath,    // !! this will NOT get deleted in 'CleanAll' !!
    *ProjectPackageExt,
    *CodeExt, // cpp was selected, because unlike other ideas: "code", "txt" only "cpp" gets correct coloring when opened in Visual Studio
    *CodeSyncExt;
extern const Str NullName,
    UnknownName,
    MultipleName,
    EsenthelStoreURL;
extern const flt StateFadeTime,
    SkelSlotSize,
    WaypointRadius,
    FlushWaypointsDelay,
    FlushWaterDelay,
    BuyFullVersionTime, // 10 min
    AutoSaveTime,       // 5 min
    MaxGameViewRange,
    PropElmNameWidth, // value width of property having element name
    SendAreasDelay,
    VtxDupPosEps,
    DefaultFOV,
    PreviewFOV;
extern const MESH_FLAG MeshJoinAllTestVtxFlag;
extern const Edit::Material::TEX_QUALITY MinMtrlTexQualityBase0, // minimum texture compression quality for Material Base0  Texture (RGBA              ) #MaterialTextureLayout      , set to LOW    because can be maximized based on 'ElmMaterial.tex_quality/EditMaterial.tex_quality'
    MinMtrlTexQualityBase1,                                      // minimum texture compression quality for Material Base1  Texture (NxNy              ) #MaterialTextureLayout      , set to HIGH   because normals need this (without this, they get very blocky due to low quality)
    MinMtrlTexQualityBase2,                                      // minimum texture compression quality for Material Base2  Texture (MetalRoughBumpGlow) #MaterialTextureLayout      , set to MEDIUM because can't be changed otherwise
    MinMtrlTexQualityDetail,                                     // minimum texture compression quality for Material Detail Texture (NxNyRoughCol      ) #MaterialTextureLayoutDetail, set to HIGH   because normals need this (without this, they get very blocky due to low quality)
    MinMtrlTexQualityMacro,                                      // minimum texture compression quality for Material Macro  Texture (RGB               ) #MaterialTextureLayout      , set to LOW    because can be maximized based on 'ElmMaterial.tex_quality/EditMaterial.tex_quality'
    MinMtrlTexQualityLight;
extern const COMPRESS_TYPE ServerNetworkCompression, ClientNetworkCompression, ProjectPackageCompression;
extern const cchar8 *AppName,
    *ServerAppName,
    *InstallerName;
extern const cchar8 *ObjAccessNames[];
extern int ObjAccessNamesElms;
extern const cchar8 *ElmNameMesh,
    *ElmNameSkel,
    *ElmNamePhys;
extern const MESH_FLAG EditMeshFlagAnd, // TanBin are not needed in Edit because they're always re-created if needed
    GameMeshFlagAnd;
extern const MaterialPtr MaterialNull;
extern bool IsServer;
extern ReadWriteSync WorldAreaSync;
extern ListColumn NameDescListColumn[1];
/******************************************************************************/
Str SDKPath();
Str BinPath();
bool LoadOK(LOAD_RESULT result);
bool CanRead(USER_ACCESS access);
bool CanWrite(USER_ACCESS access);
bool CanWriteCode(USER_ACCESS access);
int Compare(C TimeStamp &a, C TimeStamp &b);
TimeStamp Min(C TimeStamp &a, C TimeStamp &b);
TimeStamp Max(C TimeStamp &a, C TimeStamp &b);
/******************************************************************************/
