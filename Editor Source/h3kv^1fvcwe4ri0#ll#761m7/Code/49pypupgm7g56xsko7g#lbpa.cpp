/******************************************************************************

   Publishing is best done in separate state so:
      -we don't allow editing   project elements during paking
      -we don't allow receiving project elements from the server during paking

   Windows:
      Engine.pak
      Project.pak
      EngineEmbed.pak (for engine data embedded into EXE, used when 'appEmbedEngineData')
      App.pak         (for app specific data embedded into EXE, used always             )

   Android:
      Engine.pak
      Project.pak (will contain only app specific data when "!appPublishProjData")

/******************************************************************************/
class ImageGenerate
{
   Str      src_mtrl, dest_base_0;
   DateTime time;

   void set(C Str &src_mtrl, C Str &dest_base_0, C DateTime &time) {T.src_mtrl=src_mtrl; T.dest_base_0=dest_base_0; T.time=time;}

   void process()
   {
      Image       base_0;
      MaterialPtr mtrl=src_mtrl;
      if(mtrl && MergeBaseTextures(base_0, *mtrl)){File f; if(base_0.save(f.writeMem())){f.pos(0); SafeOverwrite(f, dest_base_0, &time);}}
   }
}
class ImageConvert : SrcDest // src and dest files
{
   enum FAMILY
   {
      ELM_IMAGE      ,
      ELM_IMAGE_ATLAS,
      ELM_FONT       ,
      ELM_PANEL_IMAGE,
      IMAGE          ,
   }
   bool       pow2     =false,
              clamp    =true ,
              alpha_lum=false,
              has_color=true ,
              has_alpha=true ,
           ignore_alpha=false, 
                    env=false;
   byte        downsize=    0;
   int         mip_maps=   -1,
               max_size=    0;
   VecI2           size=    0;
   FAMILY      family  =ELM_IMAGE;
   DateTime   time            ; // date time to set after converting
   int        type=-1, mode=-1;

   static SyncLock Lock;

   void set(C Str &src, C Str &dest, C DateTime &time, int type, bool ignore_alpha, bool clamp, byte downsize=0, int max_size=0)
   {
      T.family=IMAGE; T.src=src; T.dest=dest; T.time=time; T.type=type; T.ignore_alpha=ignore_alpha; T.clamp=clamp; T.downsize=downsize; T.max_size=max_size;
   }
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmImage &data, IMAGE_TYPE type)
   {
      T.family=ELM_IMAGE; T.src=src; T.dest=dest; T.time=time; T.pow2=data.pow2(); T.mip_maps=(data.mipMapsActual() ? 0 : 1); T.alpha_lum=data.alphaLum(); T.has_color=data.hasColor(); T.has_alpha=data.hasAlpha3(); T.ignore_alpha=data.ignoreAlpha(); T.env=data.envActual(); T.type=type; T.mode=data.mode; T.size=data.size;
   }
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmIcon &data, IMAGE_TYPE type)
   {
      T.family=ELM_IMAGE; T.src=src; T.dest=dest; T.time=time; T.pow2=false; T.has_color=data.hasColor(); T.has_alpha=data.hasAlpha(); T.type=type;
   }
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmImageAtlas &data, IMAGE_TYPE type)
   {
      T.family=ELM_IMAGE_ATLAS; T.src=src; T.dest=dest; T.time=time; T.type=type;
   }
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmFont &data, IMAGE_TYPE type)
   {
      T.family=ELM_FONT; T.src=src; T.dest=dest; T.time=time; T.type=type;
   }
   void set(C Str &src, C Str &dest, C DateTime &time, C ElmPanelImage &data, IMAGE_TYPE type)
   {
      T.family=ELM_PANEL_IMAGE; T.src=src; T.dest=dest; T.time=time; T.type=type;
   }

   static bool SkipOptimize(int &type, DateTime &time) // skip formats which are slow to convert
   {
      if(PublishSkipOptimize())
         if(InRange(type, IMAGE_TYPES) && ImageTI[type].compressed) // if compressed
            if(type<IMAGE_BC1 || type>IMAGE_BC5_SIGN) // BC1..BC5 compression is fast, so no need to skip
      {
         type=-1; time.decDay(); return true; // use original type and set previous date, so the file will be regenerated next time
      }
      return false;
   }

   void process()C
   {
      DateTime time=T.time; if(!time.valid())time=FileInfo(src).modify_time_utc; // 'time' could've been empty if the file didn't exist yet at the moment of setting up this object (this can happen for dynamically generated textures)
      int type=T.type; bool skip=SkipOptimize(type, time), pvrtc=(type==IMAGE_PVRTC1_2 || type==IMAGE_PVRTC1_4 || type==IMAGE_PVRTC1_2_SRGB || type==IMAGE_PVRTC1_4_SRGB);
      switch(family)
      {
         case ELM_IMAGE:
         {
            Image image; if(image.ImportTry(src))if(EditToGameImage(image, image, pow2, true, alpha_lum, ElmImage.COMPRESSED, mode, mip_maps, has_color, has_alpha, ignore_alpha, env, size, &type)) // sRGB is ignored here because we force custom type
            {
               File f; if(image.save(f.writeMem())){f.pos(0); SafeOverwrite(f, dest, &time);} // save using specified time
            }
         }break;

         case ELM_IMAGE_ATLAS:
         {
            ImageAtlas atlas; if(atlas.load(src))
            {
               FREPA(atlas.images)
               {
                  Image &image=atlas.images[i];
                  VecI2   size=image.size(), old=size; if(pvrtc)size=CeilPow2(size.max()); // image must be square for PVRTC. Unlike for regular textures, for atlases it's better to use 'CeilPow2', to make sure we don't make textures much smaller than original.
                  if(image.copyTry(image, size.x, size.y, -1, type, mode, mip_maps, FILTER_BEST, (clamp?IC_CLAMP:IC_WRAP)|IC_ALPHA_WEIGHT))
                  {
                  #if 0 // don't do any adjustments because we may want to detect image proportions based on 'trimmed_size' (Esenthel RTS does that)
                     if(size!=old) // if size is different then adjust the parts
                        REPAD(p, atlas.parts) // iterate all parts
                     {
                        ImageAtlas.Part &part=atlas.parts[p];
                        if(part.image_index==i) // if this part is stored in converted image
                        {
                           part.original_size=Round(Vec2(part.original_size*size)/old);
                           part. trimmed_size=Round(Vec2(part. trimmed_size*size)/old);
                           part. trim_pos    =Round(Vec2(part. trim_pos    *size)/old);
                           part.center_offset.set(part.trim_pos.x-part.original_size.x*0.5, -part.trim_pos.y+part.original_size.y*0.5);
                        }
                     }
                  #endif
                  }
               }
               File f; if(atlas.save(f.writeMem())){f.pos(0); SafeOverwrite(f, dest, &time);} // save using specified time
            }
         }break;

         case ELM_FONT:
         {
            Font font; if(font.load(src))
            {
               font.imageType(IMAGE_TYPE(type));
               File f; if(font.save(f.writeMem())){f.pos(0); SafeOverwrite(f, dest, &time);} // save using specified time
            }
         }break;

         case ELM_PANEL_IMAGE:
         {
            PanelImage panel_image; if(panel_image.load(src))
            {
               VecI2 size=panel_image.image.size(); if(pvrtc)size=CeilPow2(size.max()); // image must be square for PVRTC. Unlike for regular textures, for panel images it's better to use 'CeilPow2', to make sure we don't make textures much smaller than original.
               panel_image.image.copyTry(panel_image.image, size.x, size.y, -1, type, -1, -1, FILTER_BEST, IC_CLAMP|IC_ALPHA_WEIGHT);
               File f; if(panel_image.save(f.writeMem())){f.pos(0); SafeOverwrite(f, dest, &time);} // save using specified time
            }
         }break;

         case IMAGE:
         {
            if(skip) // just copy
            {
               File f; if(f.readTry(src))SafeOverwrite(f, dest, &time);
            }else
            if(C ImagePtr &image=src)
            {
               Image temp, *s=image();
               VecI  size=s.size3(); if(pvrtc)size.xy=NearestPow2(size.xy.avgI()); // image must be square for PVRTC
               if(downsize) // downsize
               {
                  size.x=Max(1, size.x>>downsize);
                  size.y=Max(1, size.y>>downsize);
                  size.z=Max(1, size.z>>downsize);
               }
               if(max_size>0) // limit size
               {
                  MIN(size.x, max_size);
                  MIN(size.y, max_size);
                  MIN(size.z, max_size);
               }
               int mip_maps=T.mip_maps, mode=T.mode;
               if(ignore_alpha && NeedFullAlpha(*s, type)) // if we won't need alpha
               {
                  if(mip_maps<0)mip_maps=((s.mipMaps()==1) ? 1 : 0); // source will have now only one mip-map so we can't use "-1", auto-detect instead
                  if(mode    <0)mode    =s.mode();                   // source will now be as IMAGE_SOFT      so we can't use "-1", auto-detect instead
                  if(s.copyTry(temp, -1, -1, -1, ImageTypeExcludeAlpha(ImageTypeUncompressed(s.type())), IMAGE_SOFT, 1))s=&temp;
               }
               if(s.copyTry(temp, size.x, size.y, size.z, type, mode, mip_maps, FILTER_BEST, (clamp?IC_CLAMP:IC_WRAP)))
               {
                  File f; if(temp.save(f.writeMem())){f.pos(0); SafeOverwrite(f, dest, &time);} // save using specified time
               }
            }
         }break;
      }
   }
}
/******************************************************************************/
class PublishResult : ClosableWindow
{
   TextNoTest text;
   Button     ok, size_stats;
   Str        path;

   static void OK       (PublishResult &pr) {pr.del();}
   static void SizeStats(PublishResult &pr) {pr.del(); .SizeStats.display(pr.path, Publish.cipher());}

   void display(C Str &text)
   {
      path=(PublishProjectDataPath.is() ? PublishProjectDataPath : PublishBinPath);

      Gui+=super.create(Rect_C(0, 0, 1, 0.34), "Publishing succeeded"); button[2].show();
      T+=ok        .create(Rect_D(clientWidth()*(PublishProjectPackage ? 2 : 1)/4, -clientHeight()+0.04, 0.3, 0.06), "OK"        ).func(OK       , T);
      T+=size_stats.create(Rect_D(clientWidth()*                             3 /4, -clientHeight()+0.04, 0.3, 0.06), "Size Stats").func(SizeStats, T).hidden(PublishProjectPackage);
      T+=T.text    .create(Rect(0.02, ok.rect().max.y, clientWidth()-0.02, 0), text);
      activate();
   }
}
enum PUBLISH_STAGE
{
   PUBLISH_MTRL_SIMPLIFY,
   PUBLISH_TEX_OPTIMIZE ,
   PUBLISH_PUBLISH      ,
}
/******************************************************************************/
class PublishClass
{
   ProjectCipher cipher;
   PakProgress   progress;

   WindowIO      export_data;
   Edit.EXE_TYPE export_data_exe=Edit.EXE_EXE;

   static void ExportData(C Str &name, PublishClass &publish) {StartPublish(S, publish.export_data_exe, Edit.BUILD_PUBLISH, true, name);}

   void create()
   {
      export_data.create("pak", S, S, ExportData, ExportData, T);
   }
   void exportData(Edit.EXE_TYPE exe=Edit.EXE_EXE) {export_data_exe=exe; export_data.save();}
}
PublishClass Publish;

State               StatePublish(UpdatePublish, DrawPublish, InitPublish, ShutPublish);
Memb<PakFileData>   PublishFiles;
Memc<ImageGenerate> PublishGenerate;
Memc<ImageConvert>  PublishConvert;
Memc<Mems<byte>>    PublishFileData; // for file data allocated dynamically
SyncLock            PublishLock;
bool                PublishOk, PublishNoCompile, PublishOpenIDE, PublishDataAsPak, PublishDataOnly, PublishProjectPackage;
int                 PublishAreasLeft;
PUBLISH_STAGE       PublishStage;
Str                 PublishPath,
                    PublishBinPath, // "Bin/" path (must include tail slash)
                    PublishProjectDataPath, // "Project.pak" path
                    PublishExePath, 
                    PublishErrorMessage;
Memc<Str>           PublishKeep;
Button              PublishSkipOptimize;
Edit.EXE_TYPE       PublishExeType  =Edit.EXE_EXE;
Edit.BUILD_MODE     PublishBuildMode=Edit.BUILD_BUILD;
PublishResult       PublishRes;
WindowIO            PublishProjectPackageIO;
/******************************************************************************/
enum DATA_STATE
{
   DATA_READY ,
   DATA_CREATE, // need to be fully recreated
   DATA_UPDATE, // just update
}
bool PublishDataNeeded(Edit.EXE_TYPE exe) {return exe==Edit.EXE_UWP || exe==Edit.EXE_APK || exe==Edit.EXE_AAB || exe==Edit.EXE_IOS || exe==Edit.EXE_NS;}
bool PublishDataNeedOptimized()
{
   return (PublishExeType==Edit.EXE_APK || PublishExeType==Edit.EXE_AAB || PublishExeType==Edit.EXE_IOS) && PublishBuildMode==Edit.BUILD_PUBLISH; // optimize only for Android/iOS Publishing, because for large games with many GB that would require potentially rewriting all data, if user wants to manually optimize, he would have to delete the publish project before publishing
}
DATA_STATE PublishDataState(MemPtr<PakFileData> files, C Str &pak_name) // state of project data
{
   if(!pak_name.is())return DATA_READY; // if we don't want to create project data pak (no file)
   if(CompareFile(FileInfoSystem(pak_name).modify_time_utc, CodeEdit.appEmbedSettingsTime(PublishExeType))>0) // if existing Pak time is newer than settings (compression/encryption)
   {
      bool need_optimized=PublishDataNeedOptimized(); // test if optimized 
      Memt<DataRangeAbs> used_file_ranges;
      Pak pak; if(pak.loadEx(pak_name, Publish.cipher(), 0, null, null, need_optimized ? &used_file_ranges : null)==PAK_LOAD_OK)
      {
         if(need_optimized && used_file_ranges.elms()!=1)return DATA_CREATE; // needs to be optimized, so fully recreate
         return PakEqual(files, pak) ? DATA_READY : DATA_UPDATE;
      }
   }
   return DATA_CREATE;
}
/******************************************************************************/
void PublishDo()
{
   CodeEdit.publish();
}
void PublishProjectPackageAs(C Str &path, ptr user)
{
   StartPublish(S, Edit.EXE_EXE, Edit.BUILD_PUBLISH, true, path, false, true);
}
void KeepPublish(C Str &name)
{
   PublishKeep.add(SkipStartPath(name, PublishPath));
}
FILE_LIST_MODE CleanPublish(C FileFind &ff, bool &OK)
{
   Str full=ff.pathName();
   if(PublishKeep.binaryHas(SkipStartPath(full, PublishPath), ComparePathCI))return FILE_LIST_CONTINUE;
   bool ok;
   switch(ff.type)
   {
      case FSTD_LINK:
      case FSTD_FILE: ok=FDelFile(full); break;
      case FSTD_DIR : ok=FDelDirs(full); break;
      default       : ok=FDel    (full); break;
   }
   OK&=ok;
   return ok ? FILE_LIST_SKIP : FILE_LIST_CONTINUE; // if deleted OK then we can skip this item (if didn't delete then try to delete files inside)
}
bool CleanPublish(Memt<SrcDest> &files)
{
               KeepPublish(PublishProjectDataPath); // keep because this will be verified manually
   FREPA(files)KeepPublish(files[i].dest);
   PublishKeep.sort(ComparePathCI);
   bool ok=true; FList(PublishPath, CleanPublish, ok);
   if( !ok){Gui.msgBox(S, S+"Can't delete \""+PublishPath+'"'); return false;}
   return true;
}
bool StartPublish(C Str &exe_name, Edit.EXE_TYPE exe_type, Edit.BUILD_MODE build_mode, bool no_compile=false, C Str &custom_project_data_path=S, bool open_ide=false, bool project_package=false)
{
   PublishRes.del();

   if(exe_type==Edit.EXE_AAB && (build_mode==Edit.BUILD_PLAY || build_mode==Edit.BUILD_DEBUG))exe_type=Edit.EXE_APK; // cannot play AAB, only APK

   CodeEdit.clearAuto();
   PublishExePath       =exe_name;
   PublishExeType       =exe_type;
   PublishBuildMode     =build_mode;
   PublishNoCompile     =no_compile;
   PublishOpenIDE       =open_ide;
   PublishProjectPackage=project_package;
   PublishDataOnly      =(no_compile && custom_project_data_path.is());
   PublishPath           .clear();
   PublishBinPath        .clear();
   PublishProjectDataPath.clear();
   PublishKeep           .clear();

   if(PublishProjectPackage)Publish.cipher.clear(); // ProjectPackage is never encrypted
   else                     Publish.cipher.set(Proj);

   if(PublishProjectPackage)CodeEdit.saveChanges();

   if(PublishDataOnly || PublishProjectPackage) // data only
   {
      PublishPath           =custom_project_data_path; FCreateDirs(GetPath(PublishPath));
      PublishProjectDataPath=custom_project_data_path;
      PublishDataAsPak=true;
   }else // executable
   {
      bool physx_dll=false; //(PHYSX_DLL && Physics.engine()==PHYS_ENGINE_PHYSX && CodeEdit.appPublishPhysxDll());
      Str  bin_path=BinPath();
      Memt<SrcDest> files, dirs;
      Memc<Str    > ignore;
      if(exe_type==Edit.EXE_EXE || exe_type==Edit.EXE_DLL)
      {
         PublishPath=ProjectsPath+ProjectsPublishPath+GetBase(CodeEdit.appPath(CodeEdit.appName())).tailSlash(true);
         PublishDataAsPak=CodeEdit.appPublishDataAsPak();
         if(!FCreateDirs(PublishPath)){Gui.msgBox(S, S+"Can't create \""+PublishPath+'"'); return false;}
         if(!CodeEdit.appEmbedEngineData() || CodeEdit.appPublishProjData() || physx_dll) // if want to store something in "Bin" folder
         {
            PublishBinPath=PublishPath+"Bin\\"; if(!FCreateDirs(PublishBinPath)){Gui.msgBox(S, S+"Can't create \""+PublishBinPath+'"'); return false;} KeepPublish(PublishBinPath);
            if(CodeEdit.appPublishProjData() && PublishDataAsPak)PublishProjectDataPath=PublishBinPath+"Project.pak";

            if(!CodeEdit.appEmbedEngineData() && PublishDataAsPak)files.New().set(bin_path, PublishBinPath, "Engine.pak");
            if(physx_dll)
            {
               cchar8 *physx_files32[]=
               {
                  "PhysXDevice.dll",
                  "PhysXGpu_32.dll",
               };
               cchar8 *physx_files64[]=
               {
                  "PhysXDevice64.dll",
                  "PhysXGpu_64.dll",
               };
               if(CodeEdit.config32Bit())FREPA(physx_files32)files.New().set(bin_path, PublishBinPath, physx_files32[i]);
               else                      FREPA(physx_files64)files.New().set(bin_path, PublishBinPath, physx_files64[i]);
            }
         }
         if(CodeEdit.appPublishSteamDll ())files.New().set(  "Code/Windows/"                                                                     , PublishPath, CodeEdit.config32Bit() ? "steam_api.dll" : "steam_api64.dll");
         if(CodeEdit.appPublishOpenVRDll())files.New().set(S+"Code/Windows/"+(CodeEdit.config32Bit() ? "openvr_api.32.dll" : "openvr_api.64.dll"), PublishPath+"openvr_api.dll");
         if(PublishExePath.is()           )files.New().set(PublishExePath, PublishPath+GetBase(PublishExePath));
         if(!CleanPublish(files))return false;
      }else
      if(exe_type==Edit.EXE_WEB)
      {
         if(build_mode==Edit.BUILD_PLAY)
         {
            PublishPath=GetPath(GetPath(PublishExePath)).tailSlash(true); // convert from "../Emscripten/Debug/App.*" to "../Emscripten/" so we can reuse the same "Engine.pak" and "Project.pak" for Debug/Release configurations
         }else
         {
            PublishPath=ProjectsPath+ProjectsPublishPath+GetBase(CodeEdit.appPath(CodeEdit.appName())).tailSlash(true);
            if(!FCreateDirs(PublishPath)){Gui.msgBox(S, S+"Can't create \""+PublishPath+'"'); return false;}
            if(!CleanPublish(files))return false;
         }
         PublishDataAsPak=CodeEdit.appPublishDataAsPak();
         if(1/*!CodeEdit.appEmbedEngineData()*/ || CodeEdit.appPublishProjData()) // if want to store something in "Bin" folder, embed engine data should be ignored (we always need "Engine.pak" for Web)
         {
            PublishBinPath=PublishPath/*+"Bin\\"*/; if(!FCreateDirs(PublishBinPath)){Gui.msgBox(S, S+"Can't create \""+PublishBinPath+'"'); return false;} KeepPublish(PublishBinPath);
            if(CodeEdit.appPublishProjData() && PublishDataAsPak)PublishProjectDataPath=PublishBinPath+"Project.pak";

            if(1/*!CodeEdit.appEmbedEngineData()*/ && PublishDataAsPak)files.New().set(bin_path+"Web/Engine.pak", PublishBinPath+"Engine.pak"); // embed engine data should be ignored (we always need "Engine.pak" for Web)
         }
         if(PublishExePath.is())
         {
            Str exe_src=            GetExtNot   (PublishExePath),
                exe_dst=PublishPath+GetBaseNoExt(PublishExePath);
            files.New().set(exe_src+"." ENGINE_NAME ".html", exe_dst+".html");
            files.New().set(exe_src+".js"                  , exe_dst+".js"  );
            files.New().set(exe_src+".wasm"                , exe_dst+".wasm");
         }
      }else
      if(exe_type==Edit.EXE_MAC)
      {
         PublishPath=ProjectsPath+ProjectsPublishPath+GetBase(CodeEdit.appPath(CodeEdit.appName())).tailSlash(true);
         PublishDataAsPak=CodeEdit.appPublishDataAsPak();
         if(!FCreateDirs(PublishPath)){Gui.msgBox(S, S+"Can't create \""+PublishPath+'"'); return false;}
         if(!CodeEdit.appEmbedEngineData() || CodeEdit.appPublishProjData()) // if want to store something in "Bin" folder
         {
            PublishBinPath=PublishPath+"Bin\\"; if(!FCreateDirs(PublishBinPath)){Gui.msgBox(S, S+"Can't create \""+PublishBinPath+'"'); return false;} KeepPublish(PublishBinPath);
            if(CodeEdit.appPublishProjData() && PublishDataAsPak)PublishProjectDataPath=PublishBinPath+"Project.pak";

            if(!CodeEdit.appEmbedEngineData() && PublishDataAsPak)files.New().set(bin_path, PublishBinPath, "Engine.pak");
         }
         if(PublishExePath.is())switch(FileInfoSystem(PublishExePath).type)
         {
            case FSTD_FILE: files.New().set(PublishExePath, PublishPath+GetBase(PublishExePath)); break;
            case FSTD_DIR :  dirs.New().set(PublishExePath, PublishPath+GetBase(PublishExePath)); break;
            default       : Gui.msgBox(S, S+"Unknown file type \""+PublishExePath+'"'); return false;
         }
         if(!CleanPublish(files))return false;
      }else
      if(exe_type==Edit.EXE_LINUX)
      {
         PublishPath=ProjectsPath+ProjectsPublishPath+GetBase(CodeEdit.appPath(CodeEdit.appName())).tailSlash(true);
         PublishDataAsPak=CodeEdit.appPublishDataAsPak();
         if(!FCreateDirs(PublishPath)){Gui.msgBox(S, S+"Can't create \""+PublishPath+'"'); return false;}
         if(!CodeEdit.appEmbedEngineData() || CodeEdit.appPublishProjData() || physx_dll || CodeEdit.appPublishSteamDll() || CodeEdit.appPublishOpenVRDll()) // if want to store something in "Bin" folder
         {
            PublishBinPath=PublishPath+"Bin\\"; if(!FCreateDirs(PublishBinPath)){Gui.msgBox(S, S+"Can't create \""+PublishBinPath+'"'); return false;} KeepPublish(PublishBinPath);
            if(CodeEdit.appPublishProjData() && PublishDataAsPak)PublishProjectDataPath=PublishBinPath+"Project.pak";

            if(!CodeEdit.appEmbedEngineData() && PublishDataAsPak)files.New().set(bin_path, PublishBinPath, "Engine.pak");
            if(physx_dll)
            {
               cchar8 *physx_files[]=
               {
                  "libPhysX3_x64.so",
                  "libPhysX3Common_x64.so",
                  "libPhysX3Cooking_x64.so",
                  "libPxFoundation_x64.so",
               };
               FREPA(physx_files)files.New().set(bin_path, PublishBinPath, physx_files[i]);
            }
            if(CodeEdit.appPublishSteamDll ())files.New().set(bin_path, PublishBinPath, "libsteam_api.so");
            if(CodeEdit.appPublishOpenVRDll())files.New().set(bin_path, PublishBinPath, "libopenvr_api.so");
         }
         if(PublishExePath.is())files.New().set(PublishExePath, PublishPath+GetBase(PublishExePath));
         if(!CleanPublish(files))return false;
      }else
      if(exe_type==Edit.EXE_UWP)
      {
         PublishDataAsPak=true; // always set to true because files inside exe can't be modified by the app, so there's no point in storing them separately
         //if(CodeEdit.appPublishProjData()) always setup 'PublishProjectDataPath' because even if we don't include Project data, we still include App data
         {
            PublishProjectDataPath=CodeEdit.UWPProjectPakPath(); if(!PublishProjectDataPath.is()){Gui.msgBox(S, "Invalid path for project data file"); return false;}
            FCreateDirs(GetPath(PublishProjectDataPath));
         }
      }else
      if(exe_type==Edit.EXE_APK || exe_type==Edit.EXE_AAB)
      {
         PublishDataAsPak=true; // always set to true because files inside APK (assets) can't be modified by the app, so there's no point in storing them separately
         //if(CodeEdit.appPublishProjData()) always setup 'PublishProjectDataPath' because even if we don't include Project data, we still include App data
         {
            PublishProjectDataPath=CodeEdit.androidProjectPakPath(); if(!PublishProjectDataPath.is()){Gui.msgBox(S, "Invalid path for project data file"); return false;}
            FCreateDirs(GetPath(PublishProjectDataPath));
         }
      }else
      if(exe_type==Edit.EXE_IOS)
      {
         PublishDataAsPak=true; // always set to true because files inside iOS APP can't be modified by the app, so there's no point in storing them separately
         //if(CodeEdit.appPublishProjData()) always setup 'PublishProjectDataPath' because even if we don't include Project data, we still include App data
         {
            PublishProjectDataPath=CodeEdit.iOSProjectPakPath(); if(!PublishProjectDataPath.is()){Gui.msgBox(S, "Invalid path for project data file"); return false;}
            FCreateDirs(GetPath(PublishProjectDataPath));
         }
      }else
      if(exe_type==Edit.EXE_NS)
      {
         PublishDataAsPak=true; // always set to true because files inside iOS APP can't be modified by the app, so there's no point in storing them separately
         //if(CodeEdit.appPublishProjData()) always setup 'PublishProjectDataPath' because even if we don't include Project data, we still include App data
         {
            PublishProjectDataPath=CodeEdit.nintendoProjectPakPath(); if(!PublishProjectDataPath.is()){Gui.msgBox(S, "Invalid path for project data file"); return false;}
            FCreateDirs(GetPath(PublishProjectDataPath));
         }
      }else
      {
         Gui.msgBox(S, "Invalid application type."); return false;
      }
      FILE_OVERWRITE_MODE overwrite=FILE_OVERWRITE_DIFFERENT; // FILE_OVERWRITE_DIFFERENT faster but less safe
      FREPA(files)if(!FCopy      (files[i].src, files[i].dest, overwrite)){Gui.msgBox(S, S+"Can't copy \""+GetBase(files[i].src)+'"'); return false;}
      FREPA( dirs)if(!FReplaceDir( dirs[i].src,  dirs[i].dest, overwrite)){Gui.msgBox(S, S+"Can't copy \""+GetBase( dirs[i].src)+'"'); return false;}
   }

   if(build_mode!=Edit.BUILD_PUBLISH) // if there's no need to wait for fully complete data (for example we want to Play/Debug game on mobile which requires PAK creation)
      if(PublishDataAsPak) // if we're creating a pak
         if(!(PublishExeType==Edit.EXE_AAB && CodeEdit.appGooglePlayAssetDelivery())) // no asset packs
   {
      Proj.flush(); // flush what we've changed
      SetPublishFiles(PublishFiles, PublishGenerate, PublishConvert, PublishFileData); // detect files for packing
      if(!PublishGenerate.elms() && !PublishConvert.elms()) // if there are no elements to generate and convert
         if(PublishDataState(PublishFiles, PublishProjectDataPath)==DATA_READY) // if data already available
            {PublishSuccess(); return true;} // exit already
   }

   StatePublish.set(StateFadeTime);
   return true;
}
/******************************************************************************/
void ImageGenerateProcess(ImageGenerate &generate, ptr user, int thread_index)
{
   if(!Publish.progress.stop)
   {
      ThreadMayUseGPUData();
      generate.process();
      {SyncLocker locker(PublishLock); Publish.progress.progress+=1.0/PublishGenerate.elms();}
   }
}
bool SetPak(MemPtr<PakFileData> files, C Str &pak_name)
{
   DATA_STATE             state=PublishDataState(files, pak_name); if(state==DATA_READY)return true;
   PROJ_CMPR_PLATFORM       pcp=ProjCompres(PublishExeType);
   COMPRESS_TYPE compress      =PublishProjectPackage ? ProjectPackageCompression      : Proj.compress_type [pcp];
   int           compress_level=PublishProjectPackage ? ProjectPackageCompressionLevel : Proj.compress_level[pcp];
   if(state==DATA_UPDATE && PakReplaceInPlace(files, pak_name, PAK_SET_HASH, Publish.cipher(), compress, compress_level, &PublishErrorMessage, &Publish.progress))return true;
                     return PakCreate        (files, pak_name, PAK_SET_HASH, Publish.cipher(), compress, compress_level, &PublishErrorMessage, &Publish.progress);
}
/******************************************************************************/
bool PublishFunc(Thread &thread)
{
   // generate
   PublishStage=PUBLISH_MTRL_SIMPLIFY; Publish.progress.progress=0; WorkerThreads.process1(PublishGenerate, ImageGenerateProcess);

   // convert
   if(PublishConvert.elms()) // image compression is already multi-threaded, so allow only one at a time, this is so we can check for 'stop' after other image finished, and stop immediately, without waiting for this one to be processed
   {
      PublishStage=PUBLISH_TEX_OPTIMIZE; Publish.progress.progress=0;
      ThreadMayUseGPUData();
      FREPA(PublishConvert) // process in order because of progress
      {
         if(Publish.progress.stop)break;
         PublishConvert[i].process();
         Publish.progress.progress=flt(i)/PublishConvert.elms();
      }
      ThreadFinishedUsingGPUData();
   }

   // pak
   PublishStage=PUBLISH_PUBLISH; Publish.progress.progress=0;
   if(PublishDataAsPak)
   {
      if(PublishExeType==Edit.EXE_AAB && CodeEdit.appGooglePlayAssetDelivery()) // asset packs are only for AAB (not APK)
      {
         FDelFile(PublishProjectDataPath); // delete main project data so it won't be included in the APK
         Str android_path=CodeEdit.androidPath(); if(!android_path.is()){PublishErrorMessage="Invalid 'androidPath'"; return false;} android_path.tailSlash(true);
         const int           max_size=500<<20; // 500 MB (there's 512 MB limit for each PAK, so keep a little smaller because of PAK header) - https://developer.android.com/guide/playcore/asset-delivery
               long        split_size=0;
         Memt<PakFileData> split;
         CodeEdit.android_asset_packs=0;
         for(int i=0; ; i++)
         {
          C PakFileData *pfd=PublishFiles.addr(i);
            long pfd_size=(pfd ? pfd.data.size() : 0);
            if(  pfd_size>=max_size){PublishErrorMessage=S+"File too big:\n"+pfd.name; return false;} // a single file cannot be bigger than asset pack
            if(!pfd || split_size+pfd_size>max_size) // if reached the end, or adding this file would break the limit
            {
               if(split.elms()) // have any data
               {
                  Str pack_name=S+"Data"+CodeEdit.android_asset_packs,
                      pack_path=android_path+pack_name+"/src/main/assets/";
                  FCreateDirs(pack_path);
                  if(!SetPak(split, pack_path+pack_name+".pak"))return false; // !! FILE NAMES MUST BE UNIQUE BECAUSE ANDROID GRADLE WILL COMPLAIN IF THERE ARE MULTIPLE FILES WITH SAME NAMES ACROSS DIFFERENT ASSET PACKS !!
                  split.clear(); split_size=0; CodeEdit.android_asset_packs++;
               }
               if(!pfd){PublishOk=true; break;} // finished
            }
            split.add(*pfd); split_size+=pfd_size;
         }
      }else PublishOk=SetPak(PublishFiles, PublishProjectDataPath);
   }else
   {
      Memc<Str> dest_paths;
      FREPA(PublishFiles)
      {
         if(Publish.progress.stop)return false;
       C PakFileData &pfd=PublishFiles[i];
         if(pfd.type!=FSTD_DIR)
         {
            Str dest=PublishBinPath+pfd.name, dest_path=GetPath(dest);
            if(dest_paths.binaryInclude(dest_path, ComparePathCI))FCreateDirs(dest_path); // create path only once
            if(!FCopy(pfd.data.name, dest))
            {
               PublishErrorMessage="Error copying file.";
               if(!FExist(pfd.data.name))PublishErrorMessage.line()+="File not found.";
               PublishErrorMessage.line()+=S+"File: \""+pfd.name+'"';
               if(Elm *elm=Proj.findElm(GetFileNameID(pfd.name)))PublishErrorMessage.line()+=S+"Element: \""+Proj.elmFullName(elm)+'"';
               return false;
            }
         }
         Publish.progress.progress=flt(i)/PublishFiles.elms();
      }
      PublishOk=true;
   }
   return false;
}
/******************************************************************************/
class Texture
{
   enum
   {
      SRGB      =1<<0, // if sRGB gamma
      SIGN      =1<<1, // if signed
      DYNAMIC   =1<<2, // if texture is dynamically generated
      REGENERATE=1<<3, // if this texture needs to be regenerated
   }
   UID                            id; // texture id
   Edit.Material.TEX_QUALITY quality=Edit.Material.LOW;
   byte                     downsize=MaxMaterialDownsize-1, // downsize
                            channels=3, // assume RGB by default (no alpha used)
                               flags=0;

   Texture& downSize(int size) {MIN(downsize, size); return T;}
   
   Texture& usesAlpha() {channels=4; return T;}
   Texture& normal   () {channels=2; flags|=SIGN; return T;}

   bool sRGB      ()C {return FlagOn(flags, SRGB      );}   Texture& sRGB      (bool on) {FlagSet(flags, SRGB      , on); return T;}
   bool sign      ()C {return FlagOn(flags, SIGN      );}   Texture& sign      (bool on) {FlagSet(flags, SIGN      , on); return T;}
   bool dynamic   ()C {return FlagOn(flags, DYNAMIC   );}   Texture& dynamic   (bool on) {FlagSet(flags, DYNAMIC   , on); return T;}
   bool regenerate()C {return FlagOn(flags, REGENERATE);}   Texture& regenerate(bool on) {FlagSet(flags, REGENERATE, on); return T;}

   static int CompareTex(C Texture &tex, C UID &tex_id) {return Compare(tex.id, tex_id);}
}
Texture* GetTexture(MemPtr<Texture> textures, C UID &tex_id)
{
   if(tex_id.valid())
   {
      int index; if(textures.binarySearch(tex_id, index, Texture.CompareTex))return &textures[index];
      Texture &tex=textures.NewAt(index); tex.id=tex_id;
      return  &tex;
   }
   return null;
}
void AddPublishFiles(Memt<Elm*> &elms, MemPtr<PakFileData> files, Memc<ImageGenerate> &generate, Memc<ImageConvert> &convert, Edit.EXE_TYPE exe_type)
{
   if(!elms.elms())return;

   Memx<Texture> publish_texs; // textures to be published, need to use Memx because below pointers are stored

 C bool android=(exe_type==Edit.EXE_APK || exe_type==Edit.EXE_AAB),
            iOS=(exe_type==Edit.EXE_IOS),
            uwp=(exe_type==Edit.EXE_UWP),
            web=(exe_type==Edit.EXE_WEB),
             ns=(exe_type==Edit.EXE_NS ),
           mtrl_simplify=Proj.materialSimplify(exe_type);
   TEX_SIZE_PLATFORM tsp=ProjTexSize          (exe_type);

   // elements
   FREPA(elms)if(Elm *elm=elms[i])if(ElmPublish(elm.type))
   {
      if(elm.type==ELM_WORLD) // world
      {
         Str world_path=EncodeFileName(elm.id), world_edit_path_src, world_game_path_src;

         PakFileData &pfd=files.New(); pfd.name=world_path; pfd.type=FSTD_DIR; // world folder

         if(Proj.getWorldPaths(elm.id, world_edit_path_src, world_game_path_src))
            if(WorldVer *ver=Proj.worldVerGet(elm.id))
         {
            world_path.tailSlash(true);

            PakFileData &pfd=files.New(); // world settings
            pfd.name    =world_path         +"Settings";
            pfd.data.set(world_game_path_src+"Settings");

            Str     area_path=world_path         +"Area\\",
                src_area_path=world_game_path_src+"Area\\",
                waypoint_path=world_path         +"Waypoint\\",
            src_waypoint_path=world_game_path_src+"Waypoint\\";
            FREPA(ver.areas) // process in order to avoid re-sorting
            {
               Str src=src_area_path+ver.areas.lockedKey(i); if(FExist(src)) // if an area was created, but later its data was removed, then the AreaVer will remain, however the area data file may be removed, because of that, we need to check if it exists
               {
                  PakFileData &pfd=files.New(); // world area
                  pfd.name=area_path+ver.areas.lockedKey(i);
                  pfd.data.set(src);
               }
            }
            FREPA(ver.waypoints) // process in order to avoid re-sorting
            {
               Str file_id=EncodeFileName(ver.waypoints.lockedKey(i));
               if(FExist(src_waypoint_path+file_id)) // waypoint game file can be deleted when it was removed so we need to check if it exists
               {
                  PakFileData &pfd=files.New(); // world waypoint
                  pfd.name    =    waypoint_path+file_id;
                  pfd.data.set(src_waypoint_path+file_id);
               }
            }
         }
      }else
      if(elm.type==ELM_MINI_MAP) // mini map
      {
         Str mini_map_path=EncodeFileName(elm.id), mini_map_game_path_src, mini_map_formats_path;
         PakFileData &pfd=files.New(); pfd.name=mini_map_path; pfd.type=FSTD_DIR; // mini map folder
         if(ElmMiniMap *data=elm.miniMapData())
            if(MiniMapVer *ver=Proj.miniMapVerGet(elm.id))
         {
            mini_map_path.tailSlash(true);
            mini_map_game_path_src=Proj.gamePath(elm.id).tailSlash(true);

            PakFileData &pfd=files.New(); // mini map settings
            pfd.name    =mini_map_path         +"Settings";
            pfd.data.set(mini_map_game_path_src+"Settings");

            IMAGE_TYPE dest_type=IMAGE_NONE;
            if(android || iOS) // convert for android/ios, desktop/uwp/web/switch already have IMAGE_BC1 chosen
            {
               dest_type=IMAGE_ETC2_RGB_SRGB; //(android ? IMAGE_ETC2_RGB_SRGB : IMAGE_PVRTC1_4_SRGB);
               mini_map_formats_path=Proj.formatPath(elm.id, FormatSuffix(dest_type));
               mini_map_formats_path.tailSlash(true);
               FCreateDirs(mini_map_formats_path);
            }

            FREPA(ver.images) // process in order to avoid re-sorting
            {
               PakFileData &pfd=files.New(); // mini map image
               pfd.name    =mini_map_path         +ver.images[i];
               pfd.data.set(mini_map_game_path_src+ver.images[i]);
               if(dest_type) // convert
               {
                  Str        src_name=pfd.data.name,
                            dest_name=mini_map_formats_path+ver.images[i];
                  pfd.data.set(dest_name); // adjust pak file path
                  FileInfo src_fi(src_name);
                  if(CompareFile(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc)) // if different (compare just modify time, because sizes will always be different due to different formats)
                     convert.New().set(src_name, dest_name, src_fi.modify_time_utc, dest_type, true, true); // create new conversion
               }
            }
         }
      }else // regular files
      {
         PakFileData &pfd=files.New();
         pfd.name         =       EncodeFileName(elm.id);
         pfd.data.set(             Proj.gamePath(elm.id));
         pfd.compress_mode=(ElmPublishNoCompress(elm.type) ? COMPRESS_DISABLE : COMPRESS_ENABLE);

         if(elm.type==ELM_MTRL)if(ElmMaterial *data=elm.mtrlData()) // material
         {
            bool uses_tex_bump=data.usesTexBump(), 
                 uses_tex_glow=data.usesTexGlow();
            byte downsize  =(InRange(tsp, data.tex_downsize) ? data.tex_downsize[tsp] : 0);
            uint flags     =0; // used to set 'dynamic, regenerate'
            UID  base_0_tex=data.base_0_tex,
                 base_1_tex=data.base_1_tex,
                 base_2_tex=data.base_2_tex;

            // simplify material
            if(mtrl_simplify && (base_1_tex.valid() || base_2_tex.valid()))
            {
               flags|=Texture.DYNAMIC; // mark it as dynamically generated texture
               uses_tex_bump=uses_tex_glow=false; // these textures are removed when merging

               // adjust base texture ID's
               base_0_tex=MergedBaseTexturesID(base_0_tex, base_1_tex, base_2_tex); // new texture merged from old
               base_1_tex.zero(); // removed
               base_2_tex.zero(); // removed

               // make a copy of the material with adjusted textures
               Str src_name=pfd.data.name,
                  dest_name=Proj.formatPath(elm.id, FormatSuffixSimple());
               pfd.data.set(dest_name); // adjust pak file path
               FileInfo src_fi(src_name);
               if(CompareFile(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc)) // if different (compare just modify time, because sizes will always be different due to different formats)
               {
                  MaterialPtr mtrl=src_name; // use 'MaterialPtr' to access it from cache if available
                  Material    temp=*mtrl;
                  temp.base_0=Proj.texPath(base_0_tex); // the texture will be initially saved to 'texDynamicPath' however in PAK it will be created in 'texPath' so save the reference there
                  temp.base_1=null;
                  temp.base_2=null;
                  // adjust extra Base2 parameters
                  {
                     Vec4 avg; if(mtrl->base_2 && mtrl->base_2->stats(null, null, &avg))
                     {
                        flt avg_rough=avg.y, avg_metal=avg.x; // #MaterialTextureLayout
                        temp.rough_add=avg_rough*temp.  rough_mul+temp.  rough_add ; temp.rough_mul=0; // !! clear 'rough_mul' after calculating 'rough_add' !!
                        temp.reflect  (avg_metal*temp.reflect_mul+temp.reflect_add);
                     }
                  }
                  if(data.usesTexGlow())temp.glow=0; // disable glow if there was a glow map, because now it's removed
                  File f; temp.save(f.writeMem(), Proj.game_path); f.pos(0); SafeOverwrite(f, dest_name, &src_fi.modify_time_utc);
                  flags|=Texture.REGENERATE; // material parameters could have changed, so regenerate texture too as it may depend on them
               }

               // merge textures
               FileInfo  src_base  (Proj.texPath       (data.base_0_tex.valid() ? data.base_0_tex : data.base_1_tex.valid() ? data.base_1_tex : data.base_2_tex)); // get modify time of first available original texture
               Str      dest_base_0=Proj.texDynamicPath(     base_0_tex); // get path where merged texture will be stored
               if((flags&Texture.REGENERATE) // if material was changed
               || CompareFile(src_base.modify_time_utc, FileInfoSystem(dest_base_0).modify_time_utc)) // or texture times are different (compare just modify time, because sizes will always be different due to different formats)
               {
                  flags|=Texture.REGENERATE; // make sure this is enabled
                  generate.New().set(src_name, dest_base_0, src_base.modify_time_utc);
               }
            }

            // !! 'GetTexture' needs to be called always because it adds texture to publish list !!
            // #MaterialTextureLayout
            Texture *t0; if(        t0=GetTexture(publish_texs,        base_0_tex)){t0.sRGB(true); t0.downSize(downsize); MAX(t0.quality, MinMtrlTexQualityBase0 ); MAX(t0.quality, data.tex_quality); t0.flags|=flags;}
            Texture *t1; if(        t1=GetTexture(publish_texs,        base_1_tex)){               t1.downSize(downsize); MAX(t1.quality, MinMtrlTexQualityBase1 ); t1.normal();}
            Texture *t2; if(        t2=GetTexture(publish_texs,        base_2_tex)){               t2.downSize(downsize); MAX(t2.quality, MinMtrlTexQualityBase2 );}
                         if(Texture *t=GetTexture(publish_texs, data.  detail_tex)){               t .downSize(downsize); MAX(t .quality, MinMtrlTexQualityDetail); t.usesAlpha();} // Detail uses Alpha for Color #MaterialTextureLayoutDetail
                         if(Texture *t=GetTexture(publish_texs, data.   macro_tex)){t .sRGB(true); t .downSize(downsize); MAX(t .quality, MinMtrlTexQualityMacro ); MAX(t.quality, data.tex_quality);} // doesn't use Alpha, 'GetTexture' needs to be called
                         if(Texture *t=GetTexture(publish_texs, data.emissive_tex)){t .sRGB(true); t .downSize(downsize); MAX(t .quality, MinMtrlTexQualityLight );} // doesn't use Alpha, 'GetTexture' needs to be called

            // check which base textures use Alpha Channel, #MaterialTextureLayout
            if(t0)
            {
               if(data.usesTexAlpha())t0.usesAlpha(); // t0 Alpha used for opacity
            }
            if(t2)
            {
               if(uses_tex_glow)t2.usesAlpha(); // t2 Alpha used for glow
            }
         }

         if(elm.type==ELM_WATER_MTRL)if(ElmWaterMtrl *data=elm.waterMtrlData()) // water material
         {
            // !! 'GetTexture' needs to be called always because it adds texture to publish list !!
            // #MaterialTextureLayoutWater
            Texture *t0; if(t0=GetTexture(publish_texs, data.base_0_tex)){t0.sRGB(true); t0.downSize(0); MAX(t0.quality, MinMtrlTexQualityBase0); MAX(t0.quality, data.tex_quality);}
            Texture *t1; if(t1=GetTexture(publish_texs, data.base_1_tex)){               t1.downSize(0); MAX(t1.quality, MinMtrlTexQualityBase1); t1.normal();}
            Texture *t2; if(t2=GetTexture(publish_texs, data.base_2_tex)){               t2.downSize(0); MAX(t2.quality, MinMtrlTexQualityBase2); t2.channels=1; t2.sign(true);}

            // check which base textures use Alpha Channel, #MaterialTextureLayoutWater
         }

         // try optimizing images for target platform
         if(elm.type==ELM_IMAGE) // image
            if(android || iOS || uwp || web) // desktop/switch platforms already have the best format chosen through 'EditToGameImage' and 'ImageProps'
               if(ElmImage *data=elm.imageData())
                  if(IMAGE_TYPE dest_type=(android ? data.androidType() : iOS ? data.iOSType() : uwp ? data.uwpType() : data.webType())) // want to use custom type
         {
            Str src_name=pfd.data.name,
               dest_name=Proj.formatPath(elm.id, FormatSuffix(dest_type));
            pfd.data.set(dest_name); // adjust pak file path
            FileInfo src_fi(src_name);
            if(CompareFile(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc)) // if different (compare just modify time, because sizes will always be different due to different formats)
               convert.New().set(Proj.editPath(elm.id), dest_name, src_fi.modify_time_utc, *data, dest_type); // create new conversion (set src from edit path to get better quality)
         }

         // try optimizing icons for target platform
         if(elm.type==ELM_ICON) // icon
            if(android || iOS || uwp || web) // desktop/switch platforms already have the best format chosen through 'EditToGameImage' and 'ImageProps'
               if(ElmIcon *data=elm.iconData())
                  if(IMAGE_TYPE dest_type=(android ? data.androidType(&Proj) : iOS ? data.iOSType(&Proj) : uwp ? data.uwpType(&Proj) : data.webType(&Proj))) // want to use custom type
         {
            Str src_name=pfd.data.name,
               dest_name=Proj.formatPath(elm.id, FormatSuffix(dest_type));
            pfd.data.set(dest_name); // adjust pak file path
            FileInfo src_fi(src_name);
            if(CompareFile(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc)) // if different (compare just modify time, because sizes will always be different due to different formats)
               convert.New().set(Proj.gamePath(elm.id), dest_name, src_fi.modify_time_utc, *data, dest_type); // create new conversion (set src from game path because icons have only game version)
         }

         // try optimizing atlases for target platform
         if(elm.type==ELM_IMAGE_ATLAS) // image atlas
            if(android || iOS || (uwp && !UWPBC7) || (web && !WebBC7)) // desktop/switch platforms already have the best format chosen during image atlas creation
               if(ElmImageAtlas *data=elm.imageAtlasData())
                  if(data.compress())
                     if(IMAGE_TYPE dest_type=((android || iOS) ? IMAGE_ETC2_RGBA_SRGB : iOS ? IMAGE_PVRTC1_4_SRGB : uwp ? IMAGE_BC3_SRGB : IMAGE_BC3_SRGB)) // we assume that atlas images contain transparency
         {
            Str src_name=pfd.data.name,
               dest_name=Proj.formatPath(elm.id, FormatSuffix(dest_type));
            pfd.data.set(dest_name); // adjust pak file path
            FileInfo src_fi(src_name);
            if(CompareFile(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc)) // if different (compare just modify time, because sizes will always be different due to different formats)
               convert.New().set(Proj.gamePath(elm.id), dest_name, src_fi.modify_time_utc, *data, dest_type); // create new conversion (set src from game path because image atlases have only game version)
         }

         // try optimizing fonts for target platform
         if(elm.type==ELM_FONT) // font
            if(android || iOS || web) // desktop/switch platforms already have the best format chosen during font creation
               if(ElmFont *data=elm.fontData())
                  if(IMAGE_TYPE dest_type=IMAGE_ETC2_RG) // #FontImageLayout
         {
            Str src_name=pfd.data.name,
               dest_name=Proj.formatPath(elm.id, FormatSuffix(dest_type));
            pfd.data.set(dest_name); // adjust pak file path
            FileInfo src_fi(src_name);
            if(CompareFile(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc)) // if different (compare just modify time, because sizes will always be different due to different formats)
               convert.New().set(Proj.gamePath(elm.id), dest_name, src_fi.modify_time_utc, *data, dest_type); // create new conversion (set src from game path because fonts have only game version)
         }

         // try optimizing panel images for target platform
         if(elm.type==ELM_PANEL_IMAGE) // panel image
            if(android || iOS || (uwp && !UWPBC7) || (web && !WebBC7)) // desktop/switch platforms already have the best format chosen during image atlas creation
               if(ElmPanelImage *data=elm.panelImageData())
                  if(IMAGE_TYPE dest_type=((android || iOS) ? IMAGE_ETC2_RGBA_SRGB : iOS ? IMAGE_PVRTC1_4_SRGB : uwp ? IMAGE_BC3_SRGB : IMAGE_BC3_SRGB)) // we assume that panel images contain transparency
         {
            Str src_name=pfd.data.name,
               dest_name=Proj.formatPath(elm.id, FormatSuffix(dest_type));
            pfd.data.set(dest_name); // adjust pak file path
            FileInfo src_fi(src_name);
            if(CompareFile(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc)) // if different (compare just modify time, because sizes will always be different due to different formats)
               convert.New().set(Proj.gamePath(elm.id), dest_name, src_fi.modify_time_utc, *data, dest_type); // create new conversion (set src from game path because image atlases have only game version)
         }
      }
   }

   // textures
   int tex_not_found=0;
   FREPA(publish_texs)
   {
    C Texture &tex=publish_texs[i];
      if(Proj.texs.binaryHas(tex.id) || tex.dynamic())
      {
         PakFileData &pfd=files.New();
         pfd.name=S+"Tex/"+EncodeFileName(tex.id); // dest name
         pfd.data.set(tex.dynamic() ? Proj.texDynamicPath(tex.id) : Proj.texPath(tex.id)); // src name

         // change type
         int change_type=-1; // sRGB is set below
         if(tex.quality<Edit.Material.FULL) // compress
         {
            if(android || iOS)change_type=((tex.channels==1) ? (tex.sign() ? IMAGE_ETC2_R_SIGN : IMAGE_ETC2_R) : (tex.channels==2) ? (tex.sign() ? IMAGE_ETC2_RG_SIGN : IMAGE_ETC2_RG) : (tex.channels==3                   ) ? IMAGE_ETC2_RGB : IMAGE_ETC2_RGBA);else
            if(iOS           )change_type=((tex.channels==1) ? (tex.sign() ? IMAGE_ETC2_R_SIGN : IMAGE_ETC2_R) : (tex.channels==2) ? (tex.sign() ? IMAGE_ETC2_RG_SIGN : IMAGE_ETC2_RG) : (tex.quality >=Edit.Material.MEDIUM) ? IMAGE_PVRTC1_4 : IMAGE_PVRTC1_2 );else
            if(uwp           )change_type=((!UWPBC7 && tex.channels>=3) ? ((tex.channels==4) ? IMAGE_BC3 : IMAGE_BC1) : -1);else // here we only want to disable BC7
            if(web           )change_type=((tex.channels<=2) ? -1 : WebBC7 ? ((tex.channels==4 || tex.quality>Edit.Material.MEDIUM) ?        -1 : IMAGE_BC1)  // texture could have alpha, however if we're not using it, then reduce to BC1 because it's only 4-bit per pixel
                                                                           :   tex.channels==4                                      ? IMAGE_BC3 : IMAGE_BC1); // if BC7 not supported for Web, then use BC3
            if(change_type>=0 && tex.sRGB())change_type=ImageTypeIncludeSRGB((IMAGE_TYPE)change_type); // set sRGB
         }

         // change size
         int max_size=INT_MAX; //((tex.max_size>0) ? tex.max_size : INT_MAX);

         if(change_type>=0 || max_size!=INT_MAX || tex.downsize) // if any change is desired
         {
            // detect if we actually need to retype/resize
            if((change_type>=0 || max_size!=INT_MAX) && !tex.downsize) // in this detection we can change only 'change_type' and 'max_size', however if 'downsize' is set, then we will always convert, so no need to check this
            {
               ImageHeader header; if(ImageLoadHeader(pfd.data.name, header)) // this may fail if this is a dynamically generated texture which doesn't exist yet, in that case just proceed with conversion
               {
                  if(header.type      ==change_type)change_type=-1     ; // if image already is of that type , then disable retyping
                  if(header.size.max()<=   max_size)   max_size=INT_MAX; // if image size fits into the limit, then disable resizing
               }
            }

            if(change_type>=0 || max_size!=INT_MAX || tex.downsize)
            {
               Str           src_name=pfd.data.name,
                            dest_name=Proj.texFormatPath(tex.id, FormatSuffix(IMAGE_TYPE(change_type)), tex.downsize);
               pfd.data.set(dest_name); // adjust pak file path
               FileInfo src_fi(src_name);
               if(tex.regenerate() // if texture is going to be regenerated in this process, then always allow converting it
             //|| !src_fi.type // if source was not found (this can happen for dynamically generated textures), then always allow, this is not needed because we always set 'regenerate' in this case
               || CompareFile(src_fi.modify_time_utc, FileInfoSystem(dest_name).modify_time_utc)) // if different (compare just modify time, because sizes will always be different due to different formats)
                  convert.New().set(src_name, dest_name, src_fi.modify_time_utc, change_type, tex.channels<4, false, tex.downsize, max_size); // create new conversion
            }
         }
      }else
      {
         tex_not_found++;
      }
   }
   if(tex_not_found)Gui.msgBox(S, S+tex_not_found+" Texture"+CountS(tex_not_found)+" were not found");
}
/******************************************************************************/
void SetPublishFiles(Memb<PakFileData> &files, Memc<ImageGenerate> &generate, Memc<ImageConvert> &convert, Memc<Mems<byte>> &file_data)
{
   files    .clear();
   generate .clear();
   convert  .clear();
   file_data.clear();

   if(PublishProjectPackage) // publish as *.ProjectPackage
   {
      Project temp; temp=Proj;
      Memc<UID> remove; Proj.floodRemoved(remove, Proj.root); remove.sort();
      REPA(temp.elms)if(remove.binaryHas(temp.elms[i].id))temp.elms.removeValid(i, true);
      temp.getTextures(temp.texs); // keep only used textures

      // project "Data" file
      {
         File file; temp.save(file.writeMem()); file.pos(0);
         Mems<byte>  &data=file_data.New(); data.setNum(file.size()).loadRawData(file);
         PakFileData &pfd =files.New();
         pfd.name="Data";
         pfd.modify_time_utc.getUTC();
         pfd.data.set(data.data(), data.elms());
      }

      // elements
      FREPA(temp.elms)
      {
         Elm &elm=temp.elms[i];
         if(elm.type==ELM_WORLD)
         {
            Str world_edit_path=S+"Edit/"+EncodeFileName(elm.id),
                world_game_path=S+"Game/"+EncodeFileName(elm.id),
                world_edit_path_src, world_game_path_src;

            {PakFileData &pfd=files.New(); pfd.name=world_edit_path; pfd.type=FSTD_DIR;} // world edit folder
            {PakFileData &pfd=files.New(); pfd.name=world_game_path; pfd.type=FSTD_DIR;} // world game folder

            if(Proj.getWorldPaths(elm.id, world_edit_path_src, world_game_path_src))
               if(WorldVer *ver=Proj.worldVerGet(elm.id))
            {
               world_edit_path.tailSlash(true);
               world_game_path.tailSlash(true);

               { // world edit data
                  PakFileData &pfd=files.New();
                  pfd.name    =world_edit_path    +"Data";
                  pfd.data.set(world_edit_path_src+"Data");
               }
               { // world game settings
                  PakFileData &pfd=files.New();
                  pfd.name    =world_game_path    +"Settings";
                  pfd.data.set(world_game_path_src+"Settings");
               }

               // areas, waypoints and water
               Str area_game_path    =world_game_path    +"Area\\",
                   area_edit_path    =world_edit_path    +"Area\\",
                   area_game_path_src=world_game_path_src+"Area\\",
                   area_edit_path_src=world_edit_path_src+"Area\\",
               waypoint_game_path    =world_game_path    +"Waypoint\\",
               waypoint_edit_path    =world_edit_path    +"Waypoint\\",
               waypoint_game_path_src=world_game_path_src+"Waypoint\\",
               waypoint_edit_path_src=world_edit_path_src+"Waypoint\\",
                   lake_edit_path    =world_edit_path    +"Lake\\",
                   lake_edit_path_src=world_edit_path_src+"Lake\\",
                  river_edit_path    =world_edit_path    +"River\\",
                  river_edit_path_src=world_edit_path_src+"River\\";
               FREPA(ver.areas) // areas, process in order to avoid re-sorting
               {
                  Str src;

                  // edit
                  src=area_edit_path_src+ver.areas.lockedKey(i); if(FExist(src)) // if an area was created, but later its data was removed, then the AreaVer will remain, however the area data file may be removed, because of that, we need to check if it exists
                  {
                     PakFileData &pfd=files.New();
                     pfd.name=area_edit_path+ver.areas.lockedKey(i);
                     pfd.data.set(src);
                  }
                  // game
                  src=area_game_path_src+ver.areas.lockedKey(i); if(FExist(src)) // if an area was created, but later its data was removed, then the AreaVer will remain, however the area data file may be removed, because of that, we need to check if it exists
                  {
                     PakFileData &pfd=files.New();
                     pfd.name=area_game_path+ver.areas.lockedKey(i);
                     pfd.data.set(src);
                  }
               }
               FREPA(ver.waypoints) // waypoints, process in order to avoid re-sorting
               {
                  Str file_id=EncodeFileName(ver.waypoints.lockedKey(i));
                  { // edit
                     PakFileData &pfd=files.New();
                     pfd.name    =waypoint_edit_path    +file_id;
                     pfd.data.set(waypoint_edit_path_src+file_id);
                  }
                  if(FExist(waypoint_game_path_src+file_id)) // waypoint game file can be deleted when it was removed so we need to check if it exists
                  {
                     PakFileData &pfd=files.New();
                     pfd.name    =waypoint_game_path    +file_id;
                     pfd.data.set(waypoint_game_path_src+file_id);
                  }
               }
               FREPA(ver.lakes) // lakes, process in order to avoid re-sorting
               {
                  Str      file_id=EncodeFileName(ver.lakes.lockedKey(i));
                  PakFileData &pfd=files.New();
                  pfd.name    =lake_edit_path    +file_id;
                  pfd.data.set(lake_edit_path_src+file_id);
               }
               FREPA(ver.rivers) // rivers, process in order to avoid re-sorting
               {
                  Str      file_id=EncodeFileName(ver.rivers.lockedKey(i));
                  PakFileData &pfd=files.New();
                  pfd.name    =river_edit_path    +file_id;
                  pfd.data.set(river_edit_path_src+file_id);
               }
            }
         }else
         if(elm.type==ELM_MINI_MAP) // mini-map
         {
            { // edit
               PakFileData &pfd=files.New();
               pfd.name    =S+"Edit/"+EncodeFileName(elm.id);
               pfd.data.set(           Proj.editPath(elm.id));
            }
            { // game
               Str mini_map_path=S+"Game/"+EncodeFileName(elm.id), mini_map_game_path_src;
               PakFileData &pfd=files.New(); pfd.name=mini_map_path; pfd.type=FSTD_DIR; // mini map folder
               if(ElmMiniMap *data=elm.miniMapData())
                  if(MiniMapVer *ver=Proj.miniMapVerGet(elm.id))
               {
                  mini_map_path.tailSlash(true);
                  mini_map_game_path_src=Proj.gamePath(elm.id).tailSlash(true);

                  PakFileData &pfd=files.New(); // mini map settings
                  pfd.name    =mini_map_path         +"Settings";
                  pfd.data.set(mini_map_game_path_src+"Settings");

                  FREPA(ver.images) // process in order to avoid re-sorting
                  {
                     PakFileData &pfd=files.New(); // mini map image
                     pfd.name    =mini_map_path         +ver.images[i];
                     pfd.data.set(mini_map_game_path_src+ver.images[i]);
                  }
               }
            }
         }else // regular element
         {
            if(elm.type==ELM_CODE)
            {
               PakFileData &pfd=files.New();
               pfd.name    =S+"Code/"+EncodeFileName(elm.id)+CodeExt; // keep extension so when using copy elms to another project, we have consistency between copying from both *.ProjectPackage and normal projects
               pfd.data.set(           Proj.codePath(elm.id));
               // don't save code base
            }
            if(ElmEdit(elm.type))
            {
               PakFileData &pfd=files.New();
               pfd.name    =S+"Edit/"+EncodeFileName(elm.id);
               pfd.data.set(           Proj.editPath(elm.id));
               pfd.compress_mode=((elm.type==ELM_IMAGE) ? COMPRESS_DISABLE : COMPRESS_ENABLE); // in Edit folder images are stored using JPG/PNG/WEBP
            }
            if(ElmGame(elm.type))
            {
               PakFileData &pfd=files.New();
               pfd.name    =S+"Game/"+EncodeFileName(elm.id);
               pfd.data.set(           Proj.gamePath(elm.id));
               pfd.compress_mode=((elm.type==ELM_VIDEO) ? COMPRESS_DISABLE : COMPRESS_ENABLE); // videos are already compressed, some sounds may be stored as WAV, so try to compress them if possible
            }
         }
      }

      // textures
      FREPA(temp.texs)
      {
       C UID &tex_id=temp.texs[i];
         PakFileData &pfd=files.New();
         pfd.name    =S+"Game/Tex/"+EncodeFileName(tex_id);
         pfd.data.set(                Proj.texPath(tex_id));
      }
   }else
   {
      // Engine.pak files (add this as first to preserve order of loading "Engine.pak" and then "Project.pak", in case some project files overwrite engine files)
      if(!CodeEdit.appEmbedEngineData() && !PublishDataAsPak)
      {
         Pak engine; if(engine.load(BinPath()+"Engine.pak"))FREPA(engine)
         {
          C PakFile     &pf  =engine.file    (i );
          C Str         &name=engine.fullName(pf);
            PakFileData &pfd =files.New();
            pfd.name    =name;
            pfd.data.set(name);
            pfd.type=pf.type();
            pfd.modify_time_utc=pf.modify_time_utc;
            pfd.xxHash64_32=pf.data_xxHash64_32;
         }
      }

      // Project files
      Memt<Elm*> elms;
      if(CodeEdit.appPublishProjData() || PublishDataOnly)
      {
         Proj.getPublishElms(elms, PublishExeType);
      }else
      if(PublishExeType==Edit.EXE_UWP || PublishExeType==Edit.EXE_APK || PublishExeType==Edit.EXE_AAB || PublishExeType==Edit.EXE_IOS || PublishExeType==Edit.EXE_NS) // for Windows New, Android, iOS and Switch, if Project data is not included, then include only App data
      {
         Proj.getActiveAppElms(elms, PublishExeType);
      }
      AddPublishFiles(elms, files, generate, convert, PublishExeType);
   }
}
void GetPublishFiles(Memb<PakFileData> &files) // this is to be called outside of publishing just to get a list of files
{
   Memc<ImageGenerate> generate;
   Memc<ImageConvert > convert;
   Memc<Mems<byte>   > file_data;
   PublishDataAsPak     =true;
   PublishDataOnly      =true;
   PublishExeType       =Edit.EXE_EXE;
   PublishProjectPackage=false;
   SetPublishFiles(files, generate, convert, file_data);
}
/******************************************************************************/
bool InitPublish()
{
   App.stayAwake(AWAKE_SYSTEM);
   PublishOk=false;
   PublishErrorMessage.clear();
   Publish.progress.reset();
   SetKbExclusive();
   Proj.pause(); // this will also flush all unsaved data which is crucial for publishing
   PublishAreasLeft=Proj.worldAreasToRebuild();
   UpdateProgress.create(Rect_C(0, -0.05, 1, 0.045));
   Gui+=PublishSkipOptimize.create(Rect_C(0, -0.20, 0.45, 0.08), "Skip for now").focusable(false); PublishSkipOptimize.mode=BUTTON_TOGGLE;
   return true;
}
void ShutPublish()
{
   PublishCancel();
   UpdateThread       .del(); // delete the thread first
   UpdateProgress     .del();
   PublishFiles       .del();
   PublishGenerate    .del();
   PublishConvert     .del();
   PublishFileData    .del();
   PublishSkipOptimize.del();
   CodeEdit.clearAuto(); // clear needed for next 'makeAuto'
   Proj.resume();
   App.stateNormal().flash();
   if(!PublishOk)Gui.msgBox("Publishing Failed", PublishErrorMessage);
   App.stayAwake(AWAKE_OFF);
}
/******************************************************************************/
void PublishSuccess(C Str &open_path, C Str &text)
{
   PublishRes.display(text);
   Explore(open_path);
}
void PublishSuccess()
{
   if(PublishOpenIDE)
   {
      CodeEdit.CodeEditorInterface.openIDE();
   }else
   if(PublishDataNeeded(PublishExeType) && !PublishNoCompile) // we've published data, now we need to compile the code
   {
      CodeEdit.codeDo(PublishBuildMode);
   }else
   if(PublishExeType==Edit.EXE_WEB && PublishBuildMode==Edit.BUILD_PLAY)
   {
      Run("emrun", S+'"'+PublishPath+GetBase(PublishExePath)+'"');
   }else
   {
      Str text;
      if(PublishProjectDataPath.is())text=S+(PublishProjectPackage ? "Project size: " : "Project data size: ")+SizeBytes(FSize(PublishProjectDataPath));
      else                           text="Publishing succeeded";
      PublishSuccess(PublishPath, text);
   }
}
void PublishCancel() {UpdateThread.stop(); Publish.progress.stop=true; App.stateWorking();}
/******************************************************************************/
bool UpdatePublish()
{
   if(Kb.bp(KB_ESC))PublishCancel();
   if(Publish.progress.stop) // when wanting to stop then wait until thread finishes
   {
      if(!UpdateThread.active())
      {
         SetProjectState();
         PublishOk=false; if(!PublishErrorMessage.is())PublishErrorMessage="Publishing breaked on user request";
      }
   }else
   {
      if(!UpdateThread.created())
      {
         UpdateProgress.set(PublishAreasLeft-Proj.worldAreasToRebuild(), PublishAreasLeft);
         Builder.update(false);
         if(Builder.finished())
         {
            WorldEdit.flush(); // flush any world areas that were built
            SetPublishFiles(PublishFiles, PublishGenerate, PublishConvert, PublishFileData);
            UpdateThread.create(PublishFunc, null, 0, false, "Editor.Publish");
         }
      }else
      if(!UpdateThread.active()) // finished
      {
         SetProjectState();
         if(PublishOk)PublishSuccess();
      }else
      UpdateProgress.set(Publish.progress.progress);
      App.stateProgress(UpdateProgress());
   }
   int sleep=1000/30;
   if(!App.maximized())REPA(MT)if(MT.b(i))
   {
      GuiObj *go=MT.guiObj(i); if(!go || go==Gui.desktop() || go.type()==GO_WINDOW && !(go.asWindow().flag&WIN_MOVABLE)){App.window().move(MT.pixelDelta(i).x, MT.pixelDelta(i).y); sleep=1;}
   }
   Time.wait(sleep);
       Gui.update();
    Server.update(null, true); // it's very important to set 'busy' so no commands are processed during publishing
   if(Ms.bp(MS_MAXIMIZE))App.window().toggle();
   PublishSkipOptimize.visible(PublishStage==PUBLISH_TEX_OPTIMIZE && !Publish.progress.stop);
   return true;
}
/******************************************************************************/
void DrawPublish()
{
   D.clear(BackgroundColor());
   if(Publish.progress.stop)
   {
      D.text(0, 0.05, "Stopping");
   }else
   {
      if(!UpdateThread.created())
      {
         D.text(0, 0.05, S+"Waiting for "+Proj.worldAreasToRebuild()+" world areas to finish building");
      }else
      {
         Str text; switch(PublishStage)
         {
            case PUBLISH_MTRL_SIMPLIFY: text="Simplifying Materials"; break;
            case PUBLISH_TEX_OPTIMIZE : text=(PublishSkipOptimize() ? "Copying Textures"    : "Optimizing Textures"); break;
            default                   : text=(PublishProjectPackage ? "Compressing Project" : "Publishing Project" ); break;
         }
         D.text(0, 0.05, text);
      }
      GuiPC gpc;
      gpc.visible=gpc.enabled=true; 
      gpc.client_rect=gpc.clip=D.rect();
      gpc.offset.zero();
      UpdateProgress.draw(gpc);
      D.clip();
   }
   Gui.draw();
   Draw();
}
/******************************************************************************/
