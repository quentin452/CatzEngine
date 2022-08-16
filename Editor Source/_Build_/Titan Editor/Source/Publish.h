/******************************************************************************/
extern State               StatePublish;
extern Memb<PakFileData>   PublishFiles;
extern Memc<ImageGenerate> PublishGenerate;
extern Memc<ImageConvert>  PublishConvert;
extern Memc<Mems<byte>>    PublishFileData;
extern SyncLock            PublishLock;
extern bool                PublishOk, PublishNoCompile, PublishOpenIDE, PublishDataAsPak, PublishDataOnly, PublishProjectPackage;
extern int                 PublishAreasLeft;
extern PUBLISH_STAGE       PublishStage;
extern Str                 PublishPath,
                    PublishBinPath, // "Bin/" path (must include tail slash)
                    PublishProjectDataPath, // "Project.pak" path
                    PublishExePath, 
                    PublishErrorMessage;
extern Memc<Str>           PublishKeep;
extern Button              PublishSkipOptimize;
extern Edit::EXE_TYPE       PublishExeType  ;
extern Edit::BUILD_MODE     PublishBuildMode;
extern WindowIO            PublishProjectPackageIO;
/******************************************************************************/
bool PublishDataNeeded(Edit::EXE_TYPE exe);
bool PublishDataNeedOptimized();
DATA_STATE PublishDataState(MemPtr<PakFileData> files, C Str &pak_name);
void PublishDo();
void PublishProjectPackageAs(C Str &path, ptr user);
void KeepPublish(C Str &name);
FILE_LIST_MODE CleanPublish(C FileFind &ff, bool &OK);
bool CleanPublish(Memt<SrcDest> &files);
bool StartPublish(C Str &exe_name, Edit::EXE_TYPE exe_type, Edit::BUILD_MODE build_mode, bool no_compile=false, C Str &custom_project_data_path=S, bool open_ide=false, bool project_package=false);
void ImageGenerateProcess(ImageGenerate &generate, ptr user, int thread_index);
bool SetPak(MemPtr<PakFileData> files, C Str &pak_name);
bool PublishFunc(Thread &thread);
Texture* GetTexture(MemPtr<Texture> textures, C UID &tex_id);
void AddPublishFiles(Memt<Elm*> &elms, MemPtr<PakFileData> files, Memc<ImageGenerate> &generate, Memc<ImageConvert> &convert, Edit::EXE_TYPE exe_type);
void SetPublishFiles(Memb<PakFileData> &files, Memc<ImageGenerate> &generate, Memc<ImageConvert> &convert, Memc<Mems<byte>> &file_data);
void GetPublishFiles(Memb<PakFileData> &files);
bool InitPublish();
void ShutPublish();
void PublishSuccess(C Str &open_path, C Str &text);
void PublishSuccess();
void PublishCancel();
bool UpdatePublish();
void DrawPublish();
/******************************************************************************/
