/******************************************************************************/
#include "stdafx.h"
using namespace Edit;
/******************************************************************************/
      Str VSPath=""; // leave empty for auto-detect or type full path like this: "C:/Program Files (x86)/Microsoft Visual Studio 14.0"
      Str VSPath2010;
      Str AndroidNDKPath=""; // leave empty for auto-detect or type full path like this: "C:/Progs/AndroidNDK"
      Str EngineDataPath="Data/"; // this will get updated to full path
      Str EditorDataPath="Editor Data/"; // this will get updated to full path
      Str EnginePath="Engine/"; // this will get updated to full path
      Str EditorPath="Editor/"; // this will get updated to full path
      Str EditorSourcePath="Editor Source/_Build_/" ENGINE_NAME " Editor/"; // this will get updated to full path
      Str ThirdPartyLibsPath="ThirdPartyLibs/"; // this will get updated to full path
      Str RootPath; // this will be set to Engine Root full path
const Str      VSEngineProject="Engine.sln";
const Str      VSEditorProject="Project.sln";
const Str   XcodeEngineProject="Engine.xcodeproj";
const Str   XcodeEditorProject="Project.xcodeproj";
const Str   LinuxEngineProject="Linux/"; // if not empty then should include tail slash
const Str   LinuxEditorProject=""; // if not empty then should include tail slash
const Str AndroidProject="Android/"; // if not empty then should include tail slash
const Str OptionsFileName="Engine Builder.txt";
/******************************************************************************/
struct TaskBase;
MemcThreadSafe<TaskBase*> Todo;

struct TaskBase
{
   CChar8  *name, *desc;
   void   (*func)();
   Bool     on, immediate;

   void call () {if(func)func();}
   void queue() {if(immediate)call();else Todo.include(this);}
};
struct Task
{
   CheckBox  cb;
   Button   _queue;
   TaskBase *task;

   static void Queue(Task &task) {task.queue();}
          void queue(          ) {if(task)task->queue();}

   void create(TaskBase &tb)
   {
      task=&tb;
      Gui+= cb   .create().set(tb.on);
      Gui+=_queue.create(tb.name).func(Queue, T).desc(tb.desc);
   }
   void pos(Flt x, Flt y)
   {
      cb   .rect(Rect_L(x     , y, 0.05, 0.05));
     _queue.rect(Rect_L(x+0.06, y, 0.45, 0.05));
      Flt tw=_queue.textWidth(), rw=_queue.rect().w()-0.01; if(tw>rw)_queue.text_size*=rw/tw;
   }
};
/******************************************************************************/
static CChar8 *Physics_t[]=
{
   "PhysX (Recommended)", // 0
   "Bullet"             , // 1
}; ASSERT(PHYS_ENGINE_PHYSX==0 && PHYS_ENGINE_BULLET==1);

struct OptionsClass : ClosableWindow
{
   Text   t_vs_path, t_ndk_path, t_aac, t_physics;
   TextLine vs_path,   ndk_path;
   CheckBox aac;
   ComboBox physics;

   static void OptionChanged(OptionsClass &options) {options.saveConfig();}

   void create()
   {
      Flt y=-0.05f, h=0.06f;
      Gui+=super::create(Rect_C(0, 0, 1.4f, 3*h+0.11f), "Options").hide(); button[2].show();
      T+=t_vs_path .create(Vec2(0.19, y), "Visual Studio Path"); T+= vs_path.create(Rect_L(0.38, y, 1.0, 0.05)); y-=h;
    //T+=t_ndk_path.create(Vec2(0.19, y),   "Android NDK Path"); T+=ndk_path.create(Rect_L(0.38, y, 1.0, 0.05)); y-=h;
      T+=t_aac     .create(Vec2(0.19, y),   "Use Patented AAC"); T+=aac.create(Rect_L(0.38, y, 0.05, 0.05)).func(OptionChanged, T).desc("If support AAC decoding which is currently covered by patents.\nIf disabled then playback of AAC sounds will not be available."); y-=h;
      T+=t_physics .create(Vec2(0.19, y),     "Physics Engine"); T+=physics .create(Rect_L(0.38, y, 1.0, 0.05), Physics_t, Elms(Physics_t)).func(OptionChanged, T).set(PHYS_ENGINE_PHYSX, QUIET); y-=h;
      load();
      loadConfig();
      saveConfig(); // always save
   }
   Bool any()C
   {
      return vs_path().is() || ndk_path().is();
   }
   void load()
   {
      TextData data; if(data.load(OptionsFileName))
      {
         if(TextParam *param=data.findNode("Visual Studio Path")) vs_path.set(param->asText());
         if(TextParam *param=data.findNode(  "Android NDK Path"))ndk_path.set(param->asText());
      }
   }
   void save()C
   {
      if(any())
      {
         TextData data;
         if( vs_path().is())data.nodes.New().set("Visual Studio Path",  vs_path());
         if(ndk_path().is())data.nodes.New().set(  "Android NDK Path", ndk_path());
         data.save(OptionsFileName);
      }else
      {
         FDelFile(OptionsFileName);
      }
   }
   static Str EngineConfig() {return EnginePath+"Engine Config.h";}
   void loadConfig()
   {
      FileText f; if(f.read(EngineConfig()))
      {
         Str s; f.getAll(s);
         aac    .set(Contains(s, "SUPPORT_AAC 1"), QUIET);
         physics.set(Contains(s, "PHYSX 1") ? PHYS_ENGINE_PHYSX : PHYS_ENGINE_BULLET, QUIET); // remember that PhysX can be enabled only for certain platforms, so check if there's at least one "PHYSX 1" occurrence
      }else
      {
         activate(); // if no config available then show options
      }
   }
   void saveConfig()C
   {
      Str s=S+"// File automatically generated by \""+App.name()+"\"\n";
      s+=S+"#define SUPPORT_AAC "+aac()+'\n';
    //s+="#define PHYSX "+(physics()==PHYS_ENGINE_PHYSX)+'\n';
      if(physics()==PHYS_ENGINE_PHYSX)
      {
         s+="#if !IOS_SIMULATOR && !WEB\n"
            "   #define PHYSX 1\n"
            "#else\n"
            "   #define PHYSX 0\n"
            "#endif\n";
      }else s+="#define PHYSX 0\n";
      FileText f; f.writeMem(ANSI).putText(s);
      Bool changed; if(!OverwriteOnChange(f, EngineConfig(), &changed)){ErrorWrite(EngineConfig()); return;}
      if(  changed)FTimeUTC(EnginePath+"H/_/Engine Config.h", DateTime().getUTC()); // set modification time to the default config as well, to make sure to force rebuild (this is needed for VS and possibly others too)
   }
}Options;
/******************************************************************************/
Str DevEnvPath(C Str &vs_path)
{
   if(vs_path.is())
   {
      Str devenv=Str(vs_path).tailSlash(true)+"Common7\\IDE\\devenv.exe"   ; if(FExistSystem(devenv))return devenv; // pro
          devenv=Str(vs_path).tailSlash(true)+"Common7\\IDE\\VCExpress.exe"; if(FExistSystem(devenv))return devenv; // express
          devenv=Str(vs_path).tailSlash(true)+"Common7\\IDE\\WDExpress.exe"; if(FExistSystem(devenv))return devenv; // express for Windows Desktop
   }
   return S;
}
Str DevEnvPath()
{
   if(Options.vs_path().is()){Str p=DevEnvPath(Options.vs_path()); if(p.is())return p;}
   if(           VSPath.is()){Str p=DevEnvPath(         VSPath  ); if(p.is())return p;}
   return S;
}
Str NDKBuildPath(C Str &ndk_path)
{
   if(ndk_path.is())
   {
      Str ndk_build=Str(ndk_path).tailSlash(true)+PLATFORM("ndk-build.cmd", "ndk-build");
      if(FExistSystem(ndk_build))return ndk_build;
   }
   return S;
}
Str NDKBuildPath()
{
   if(Options.ndk_path().is()){Str p=NDKBuildPath(Options.ndk_path()); if(p.is())return p;}
   if(    AndroidNDKPath.is()){Str p=NDKBuildPath(  AndroidNDKPath  ); if(p.is())return p;}
   return S;
}
Str ARPath()
{
#if WINDOWS
   return ThirdPartyLibsPath+"AR/ar.exe";
#else
   return "ar";
#endif
}
/******************************************************************************/
Bool shut; // if 'Shut' called
Int  failed, // number of failures (like compilation thread failes)
     done;

SyncLock lock;

// BUILD OUTPUT
Memc<Str> data,     // list data
          data_new; // data currently added on a secondary thread, and to be added to the main data on the main thread
List<Str> list;     // list
Region    region;   // region
Button    clear, copy;

struct Build
{
   Str  exe, params, log;
   void (*pre)(), (*func)();
   
   Build& set   (C Str &exe, C Str &params, C Str &log=S) {T.exe=exe; T.params=params; T.log=log; return T;}
   Build& set   (void func()                            ) {T.func=func;                           return T;}
   Build& setPre(void func()                            ) {T.pre =func;                           return T;}

   Build() {pre=func=null;}

   static Str GetOutput(ConsoleProcess &cp, C Str &log, Str &processed)
   {
      Str out=cp.get();
      if(log.is())
      {
         FileText f; if(f.read(log))
         {
            Str all=f.getAll();
            out+=SkipStart(all, processed);
            processed=all;
         }
      }
      return out;
   }
   Bool run()
   {
      Bool ok=true;
      if(pre)pre();
      if(exe.is())
      {
         FDelFile(log);
         ConsoleProcess cp; cp.create(exe, params);
         Str processed, output; Memc<Str> outs;

         if(log.is())output="Compile start\n";

         for(; cp.active(); )
         {
            if(shut){cp.kill(); break;}
            cp.wait(25);
            output+=GetOutput(cp, log, processed);
            Split(outs, output, '\n');
            if(outs.elms()>=2)
            {
               SyncLocker locker(lock);
               for(; outs.elms()>=2; )
               {
                  Swap(data_new.New(), outs[0]); outs.remove(0, true);
               }
               output=outs.last();
            }
         }

         ok=(cp.exitCode()==0);

         output+=GetOutput(cp, log, processed);
         ok|=(Contains(output, "========== Build: 1 succeeded") || Contains(output, "========== Build: 0 succeeded, 0 failed, 1 up-to-date"));
         Split(outs, output, '\n');
         {
            SyncLocker locker(lock);
            FREPA(outs)if(outs[i].is())Swap(data_new.New(), outs[i]);
         }

         cp.del();

         if(!ok)
         {
            // fail
            Gui.msgBox("Error", "Compilation failed.\nTasks aborted.");
            Todo.clear();
         }
         FDelFile(log);
      }
      if(ok && func)func();
      return ok;
   }
};
/******************************************************************************/
Memb<Build> build_requests; // builds requested to be processed on secondary threads
Threads     build_threads;

Memx<Task> Tasks;
CheckBox   All;
Button     DoSel, OptionsButton;
/******************************************************************************/
void OptionsToggle(Ptr) {Options.visibleToggleActivate();}
void ToggleAll(Ptr) {REPAO(Tasks).cb.set(All());}
void CopyOutput(Ptr) {Str s; FREPA(data){s+=data[i]; s+='\n';} ClipSet(s);}
void Clear(Ptr) {list.setData(data.clear());}
void DoSelected(Ptr) {FREPA(Tasks)if(Tasks[i].cb())Tasks[i].queue();}
void BuildRun(Build &build, Ptr user, Int thread_index) {build.run();}
/******************************************************************************/
CChar8 *separator="/******************************************************************************/";
CChar8 *copyright="/******************************************************************************\n"
                  " * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *\n"
        " * " ENGINE_NAME " Engine (https://esenthel.com) header file.                           *\n";
/******************************************************************************/
FILE_LIST_MODE Header(C FileFind &ff, Ptr)
{
   if(ff.type==FSTD_FILE)if(ff.name!="Engine Config.h")
   {
      Str name=ff.pathName();

      // read from source file
      FileText f; f.read(name);
      Meml<Str> src; for(; !f.end(); )src.New()=f.fullLine();

      // remove empty line followed by "#ifdef"
      //SMFREP(src)if(_next_ && !SkipWhiteChars(src[i]).is() && Contains(src[_next_], "#ifdef"))src.remove(i);

      // hide EE_PRIVATE sections
      enum STACK_TYPE
      {
         STACK_NONE       ,
         STACK_PRIVATE    ,
         STACK_NOT_PRIVATE,
      };
      Memb<STACK_TYPE> stack  ; // #if stack
      Int              level=0; // EE_PRIVATE depth level
      for(MemlNode *node=src.first(); node;)
      {
         MemlNode *next=node->next();
         
       C Str &s    =src[node];
         Str  first=SkipWhiteChars(s);
         if(first.first()=='#') // if preprocessor command
         {
            if(Starts(first, "#if")) // #if #ifdef #ifndef
            {
               if(Starts(first, "#if EE_PRIVATE"))
               {
                  stack.add(STACK_PRIVATE);
                  level++;
                  src.remove(node);
               }else
               if(Starts(first, "#if !EE_PRIVATE"))
               {
                  stack.add(STACK_NOT_PRIVATE);
                  src.remove(node);
               }else
               {
                  stack.add(STACK_NONE);
                  if(level)src.remove(node);
               }
            }else
            if(Starts(first, "#el")) // #else #elif
            {
               if(stack.elms() && stack.last()==STACK_PRIVATE)
               {
                  stack.last()=STACK_NOT_PRIVATE; // #if !EE_PRIVATE
                  level--;
                  src.remove(node);
               }else
               if(stack.elms() && stack.last()==STACK_NOT_PRIVATE)
               {
                  stack.last()=STACK_PRIVATE;
                  level++;
                  src.remove(node);
               }else
               {
                  if(level)src.remove(node);
               }
            }else
            if(Starts(first, "#endif"))
            {
               if(stack.elms())
               {
                  if(stack.last() || level)src.remove(node);
                  if(stack.last()==STACK_PRIVATE)
                  {
                     level--;
                     if(!level)
                     {
                        if(next->prev())
                        {
                           Str a=src[next->prev()],
                               b=src[next        ];

                           if(!SkipWhiteChars(a).is() && Contains(b, separator))src.remove(next->prev());else // remove empty line followed  by /**/
                           if(!SkipWhiteChars(b).is() && Contains(a, separator))                              // remove empty line preceeded by /**/
                           {
                              MemlNode *temp=next->next(); src.remove(next); next=temp;
                           }
                        }
                     }
                  }
                  stack.removeLast();
               }else
               {
                  Exit(S+"Invalid #endif at \""+name+'"');
               }
            }else
            {
               if(level)src.remove(node);
            }
         }else
         {
            if(level)src.remove(node);
         }

         node=next;
      }

      // remove double lines /**/
      SMFREP(src)if(_next_ && Contains(src[i], separator) && Equal(src[i], src[_next_]))src.remove(i);

      // remove double empty lines
      SMFREP(src)if(_next_ && !SkipWhiteChars(src[i]).is() && !SkipWhiteChars(src[_next_]).is())src.remove(i);

      // remove empty line followed by "}"
      SMFREP(src)if(_next_ && !SkipWhiteChars(src[i]).is() && Starts(SkipWhiteChars(src[_next_]), "}"))src.remove(i);

      // remove empty line preceeded by "}"
      SMFREP(src)if(!SkipWhiteChars(src[i]).is() && i->prev() && Starts(SkipWhiteChars(src[i->prev()]), "{"))src.remove(i);

      // write to destination file
      Str dest=EditorPath+"Bin/Engine/"+SkipStartPath(name, EnginePath+"H"); FCreateDirs(GetPath(dest));
                f.write  (dest);
                f.putText(copyright);
      MFREP(src)f.putLine(src[i]);
                f.del    ();
      FTimeUTC(dest, ff.modify_time_utc);
   }
   return FILE_LIST_CONTINUE;
}
/******************************************************************************/
static void Copy(C Str &src, C Str &dest, FILE_OVERWRITE_MODE overwrite=FILE_OVERWRITE_DIFFERENT)
{
   if(!FCopy(src, dest, overwrite))
   {
      Gui.msgBox("Error", S+"Can't copy:\n\""+src+"\"\nto \n\""+dest+"\".\nTasks aborted.");
      Todo.clear();
   }
}
static void Del(C Str &name)
{
   if(FExistSystem(name) && !FDel(name))
   {
      Gui.msgBox("Error", S+"Can't delete:\n\""+name+"\".\nTasks aborted.");
      Todo.clear();
   }
}
static void ReplaceDir(C Str &src, C Str &dest, FILE_OVERWRITE_MODE overwrite=FILE_OVERWRITE_DIFFERENT)
{
   if(!FReplaceDir(src, dest, overwrite))
   {
      Gui.msgBox("Error", S+"Can't copy:\n\""+src+"\"\nto \n\""+dest+"\".\nTasks aborted.");
      Todo.clear();
   }
}
void CleanEngineLinuxDo()
{
   FDel(EnginePath+LinuxEngineProject+"build");
   FDel(EnginePath+LinuxEngineProject+"dist");
   FDel(EnginePath+"stdafx.h.gch");
   FDel(EnginePath+"stdafx.h.pch");
}
void CleanEditorLinuxDo()
{
   FDel(EditorSourcePath+LinuxEditorProject+"build");
   FDel(EditorSourcePath+"stdafx.h.gch");
   FDel(EditorSourcePath+"stdafx.h.pch");
}
void CleanLinuxDo()
{
   CleanEngineLinuxDo();
   CleanEditorLinuxDo();
}
void CleanEngineWebDo()
{
   FDel(EnginePath+"Emscripten");
}
void MakeWebLibs()
{
   Str dest=EditorPath+"Bin/Engine.bc";
   FDelFile(dest);

   Str params;
   params.space()+=S+'"'+EnginePath        +"Engine.bc\"";
 //params.space()+=S+'"'+ThirdPartyLibsPath+"Brotli/Windows/Emscripten/Release/Brotli.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Bullet/Windows/Emscripten/Release/BulletCollision.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Bullet/Windows/Emscripten/Release/BulletDynamics.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Bullet/Windows/Emscripten/Release/LinearMath.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"FDK-AAC/Windows/Emscripten/Release/FDK-AAC.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Flac/Windows/Emscripten/Release/FLAC.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"JpegTurbo/Web/Emscripten/Release/turbojpeg-static.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"LZ4/Windows/Emscripten/Release/LZ4.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"LZHAM/Windows/Emscripten/Release/LZHAM.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"LZMA/Windows/Emscripten/Release/Lzma.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Ogg/Windows/Emscripten/Release/Ogg.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Opus/Windows/Emscripten/Release/celt.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Opus/Windows/Emscripten/Release/opus.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Opus/Windows/Emscripten/Release/silk_common.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Opus/Windows/Emscripten/Release/silk_float.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Opus/file/win32/VS2010/Emscripten/Release/opusfile.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Png/Windows/Emscripten/LIB Release/Png.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Recast/Windows/Emscripten/Release/Recast.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Snappy/Windows/Emscripten/Release/snappy.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"SQLite/Windows/Emscripten/Release/SQLite.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Theora/Windows/Emscripten/Release/Theora.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Vorbis/Windows/Emscripten/Release/Vorbis.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Vorbis/Windows/Emscripten/Release/Vorbis File.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Waifu2x/Windows/Emscripten/Release/Waifu2x.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Webp/Windows/Emscripten/Release/WebP.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Zlib/Windows/Emscripten/Release/Zlib.bc\"";
   params.space()+=S+'"'+ThirdPartyLibsPath+"Zstd/Windows/bin/Emscripten/Release/zstdlib.bc\"";
 //if(Options.physics()==PHYS_ENGINE_PHYSX)params.space()+=S+'"'+ThirdPartyLibsPath+"PhysX/../PhysX.bc\""; FIXME

   params.space()+=S+"-o \""+dest+'"';
   Build().set("emcc.bat", params).run();
   if(!FExistSystem(dest))Gui.msgBox("Error", "Can't create Engine Web Lib BitCode");
}
/******************************************************************************/
void TestDirForObjFiles(C Str &name, Str &obj_files)
{
   if(FileFind(name, "o")()) // if it has any *.o files inside
      obj_files.space()+=Replace(UnixPath(name+"/*.o"), " ", "\\ ");
}
FILE_LIST_MODE GatherObjFiles(C FileFind &ff, Str &obj_files)
{
   if(ff.type==FSTD_DIR)TestDirForObjFiles(ff.pathName(), obj_files);
   return FILE_LIST_CONTINUE;
}
void MakeLinuxLibs()
{
   Str engine_lib=EditorPath+"Bin/Engine.a";
   FDelFile(engine_lib);

   // get a list of all possible libraries
   Memc<Str> lib_paths;
   lib_paths.add(EnginePath+LinuxEngineProject+"build/Release"); // Engine
   lib_paths.add(ThirdPartyLibsPath+"PVRTC/PVRTex/Linux_x86_64/Static/build/Release"); // PVRTC (because it's stored inside a separate folder)
   lib_paths.add(ThirdPartyLibsPath+"VP/Linux"); // VP (because it's manually built)
   if(Options.physics()==PHYS_ENGINE_PHYSX)lib_paths.add(ThirdPartyLibsPath+"PhysX/physx/bin/linux.clang/release/obj"); // PhysX (because it's stored inside a separate folder and should be linked only if PhysX is selected as physics engine)
   // iterate all Third Party Libs
   for(FileFind ff(ThirdPartyLibsPath); ff(); )if(ff.type==FSTD_DIR)lib_paths.add(ff.pathName()+"/Linux/build/Release");

   // get all *.o files from those paths
   Str obj_files; FREPA(lib_paths){TestDirForObjFiles(lib_paths[i], obj_files); FList(lib_paths[i], GatherObjFiles, obj_files);}

   // make lib
   Build().set(ARPath(), S+"-q \""+UnixPath(engine_lib)+"\" "+obj_files).run();
   if(!FExistSystem(engine_lib))Gui.msgBox("Error", "Can't create Engine Linux Lib");
}
/*Str EngineAndroidLibName(C Str &abi) {return EditorPath+"Bin/Android/Engine-"+abi+".a";}
void MakeAndroidLibs(C Str &abi)
{
   Str engine_lib=EngineAndroidLibName(abi);
   FDelFile(engine_lib);

   // get a list of all possible libraries
   Memc<Str> lib_paths;
   lib_paths.add(EnginePath+AndroidProject+"obj/local/"+abi); // Engine
   // iterate all Third Party Libs
   for(FileFind ff(ThirdPartyLibsPath); ff(); )if(ff.type==FSTD_DIR)
      if(Options.physics()==PHYS_ENGINE_PHYSX || ff.name!="PhysX") // link PhysX only if we want it
         lib_paths.add(ff.pathName()+"/Android/obj/local/"+abi);

   // get all *.o files from those paths
   Str obj_files; FREPA(lib_paths){TestDirForObjFiles(lib_paths[i], obj_files); FList(lib_paths[i], GatherObjFiles, obj_files);}

   // make lib
   if(FExistSystem(engine_lib))Gui.msgBox("Error", S+"Can't remove Engine Android "+abi+" Lib");else
   {
      Str params=S+"-q \""+UnixPath(engine_lib)+"\" "+obj_files;
      if(WINDOWS && params.length()>=32000)Gui.msgBox("Warning", "Command Line may exceed Windows 32768 character limit");
      if(Build().set(ARPath(), params).run())
      {
         if(!FExistSystem(engine_lib))Gui.msgBox("Error", S+"Can't create Engine Android "+abi+" Lib");
      }else
      {
         Explore(engine_lib); // creating archive can sometimes fail, in that case there's a temp generated file that needs to be deleted manually
      }
   }
}
void MakeAndroidLibs()
{
   // #AndroidArchitecture
   MakeAndroidLibs("arm64-v8a");

#if 0 // Arm-32
   MakeAndroidLibs("armeabi-v7a");
#else
   FDelFile(EngineAndroidLibName("armeabi-v7a"));
#endif

#if 0 // x86
   MakeAndroidLibs("x86");
#else
   FDelFile(EngineAndroidLibName("x86"));
#endif
}
void CleanEngineAndroid()
{
   FDelDirs(EnginePath+AndroidProject+"obj");
}
void CompileEngineAndroid()
{
   Str ndk_build=NDKBuildPath();
   if(!ndk_build.is()){Options.show(); Gui.msgBox("Error", "Android NDK Path unknown");}else
   {
      Int build_threads_num=Cpu.threads();
      build_threads.queue(build_requests.New().set(ndk_build, S+"-j"+build_threads_num+" -C \""+EnginePath+AndroidProject+"\""), BuildRun);
   }
}
void AndroidLibs() {build_threads.queue(build_requests.New().set(MakeAndroidLibs), BuildRun);}
/******************************************************************************/
void UpdateHeaders()
{
   // process engine headers
   FDelDirs(EditorPath+"Bin/Engine"); // delete
   FList(EnginePath+"H", Header); // convert
}
/******************************************************************************/
// ENGINE PAK
/******************************************************************************/
static Bool DesktopEnginePakFilter(C Str &full)
{
   Str name=SkipStartPath(full, EngineDataPath), base=GetBase(name), path=GetPath(name);
   if(!SUPPORT_MLAA && base=="MLAA Area.img")return false;
   return true;
}
static Bool UniversalEnginePakFilter(C Str &full)
{
   Str name=SkipStartPath(full, EngineDataPath), base=GetBase(name), path=GetPath(name);
   if(!SUPPORT_MLAA && base=="MLAA Area.img")return false;
   if(EqualPath(name, "Shader/GL")
   || EqualPath(name, "Shader/GL SPIR-V")
   || base=="World Editor"
   )return false;
   return true;
}
static Bool MobileEnginePakFilter(C Str &full)
{
   Str name=SkipStartPath(full, EngineDataPath), base=GetBase(name), path=GetPath(name);
   if(!SUPPORT_MLAA && base=="MLAA Area.img")return false;
   if(EqualPath(name, "Shader/4")
   || base=="World Editor"
   )return false;
   return true;
}
static COMPRESS_MODE ComressMode(C Str &full)
{
   Str name=SkipStartPath(full, EngineDataPath);
   return (GetBase(name)=="Engine Logo.img") ? COMPRESS_ENABLE : COMPRESS_DISABLE;
}
static DateTime LatestTime(C Pak &pak)
{
   DateTime time; time.zero();
   REPA(pak)
   {
    C PakFile &pf=pak.file(i);
      if(!(pf.flag&PF_STD_DIR)) // not a directory
         if(pf.modify_time_utc>time)time=pf.modify_time_utc;
   }
   return time;
}
void CreateEnginePak()
{
   // Desktop "Engine.pak"
   Str engine_pak_name=EditorPath+"Bin/Engine.pak";
   Pak engine_pak; if(!engine_pak.create(EngineDataPath, engine_pak_name, PAK_SHORTEN, null, null, COMPRESS_ZSTD, 255, DesktopEnginePakFilter, ComressMode))Gui.msgBox("Error", "Can't create Engine.pak");
   FTimeUTC(engine_pak_name, LatestTime(engine_pak)); // adjust pak date to latest date of its files

   // Windows Universal "Engine.pak"
   engine_pak_name=EditorPath+"Bin/Universal/Engine.pak";
   FCreateDir(GetPath(engine_pak_name));
   if(!engine_pak.create(EngineDataPath, engine_pak_name, PAK_SHORTEN, null, null, COMPRESS_ZSTD, 255, UniversalEnginePakFilter, ComressMode))Gui.msgBox("Error", "Can't create Universal Engine.pak");
   FTimeUTC(engine_pak_name, LatestTime(engine_pak)); // adjust pak date to latest date of its files

   // Mobile "Engine.pak"
   engine_pak_name=EditorPath+"Bin/Mobile/Engine.pak";
   FCreateDir(GetPath(engine_pak_name));
   if(!engine_pak.create(EngineDataPath, engine_pak_name, PAK_SHORTEN, null, null, COMPRESS_ZSTD, 255, MobileEnginePakFilter, ComressMode))Gui.msgBox("Error", "Can't create Mobile Engine.pak");
   FTimeUTC(engine_pak_name, LatestTime(engine_pak)); // adjust pak date to latest date of its files

   // Web "Engine.pak"
   engine_pak_name=EditorPath+"Bin/Web/Engine.pak";
   FCreateDir(GetPath(engine_pak_name));
   if(!engine_pak.create(EngineDataPath, engine_pak_name, PAK_SHORTEN, null, null, COMPRESS_LZ4, 255, MobileEnginePakFilter))Gui.msgBox("Error", "Can't create Web Engine.pak");
   FTimeUTC(engine_pak_name, LatestTime(engine_pak)); // adjust pak date to latest date of its files
}
/******************************************************************************/
// EDITOR PAK
/******************************************************************************/
Bool FilterEditorPak(C Str &name)
{
   Str rel=SkipStartPath(name, EditorDataPath), engine=EngineDataPath+rel;
   if(GetBase(rel)=="local.properties" && EqualPath(GetPath(GetPath(rel)), "Code/Android"))return false; // skip all "Code/Android/*/local.properties" as they're created manually by the Editor
   if(FileInfoSystem(engine).type==FSTD_FILE) // if file already exists in the Engine Data (Engine.pak) then skip (skip only files)
   {
      FRecycle(name); // remove to prevent uploading to GitHub
      return false;
   }
   return true;
}
void CreateEditorPak()
{
   Str editor_pak_name=EditorPath+"Bin/Editor.pak";
   Pak editor_pak; if(!editor_pak.create(EditorDataPath, editor_pak_name, PAK_SHORTEN, null, null, COMPRESS_NONE, 9, FilterEditorPak))Gui.msgBox("Error", "Can't create Editor.pak");
   DateTime editor_pak_time; editor_pak_time.zero();
   if(editor_pak.totalFiles()) // adjust pak date to latest date of its files
   {
      REPA(editor_pak)
      {
       C PakFile &pf=editor_pak.file(i);
         if(!(pf.flag&PF_STD_DIR)) // not a directory
         {
            DateTime dt=pf.modify_time_utc;
            if(dt>editor_pak_time)editor_pak_time=dt;
         }
      }
      FTimeUTC(editor_pak_name, editor_pak_time);
   }
}
/******************************************************************************/
void CompileVS(C Str &project, C Str &config, C Str &platform, void func()=null, void pre()=null)
{
   if(Contains(platform, "Web", false, WHOLE_WORD_STRICT)) // WEB requires VS 2010
   {
      Str devenv=DevEnvPath(VSPath2010);
      if(!devenv.is())Gui.msgBox("Error", "Compiling Web requires Visual C++ 2010");else
      {
         Str log=GetPath(App.exe())+"/vs_build_"+Replace(config+platform, ' ', '_')+".txt";
         build_threads.queue(build_requests.New().set(devenv, VSBuildParams(project, config, platform, log), log).set(func).setPre(pre), BuildRun);
      }
   }else
   {
      Str msbuild=MSBuildPath(Options.vs_path().is() ? Options.vs_path() : VSPath); if(FExistSystem(msbuild))
      {
         build_threads.queue(build_requests.New().set(msbuild, MSBuildParams(project, config, platform)).set(func).setPre(pre), BuildRun);
      }else
      {
         Str devenv=DevEnvPath();
         if(!devenv.is()){Options.show(); Gui.msgBox("Error", "Visual Studio Path unknown");}else
         {
            Str log=GetPath(App.exe())+"/vs_build_"+Replace(config+platform, ' ', '_')+".txt";
            build_threads.queue(build_requests.New().set(devenv, VSBuildParams(project, config, platform, log), log).set(func).setPre(pre), BuildRun);
         }
      }
   }
}
void CleanXcode(C Str &project, C Str &config, C Str &platform, C Str &sdk=S)
{
   build_threads.queue(build_requests.New().set("xcodebuild", XcodeBuildCleanParams(project, config, platform, sdk)), BuildRun);
}
void CompileXcode(C Str &project, C Str &config, C Str &platform, C Str &sdk=S, void func()=null)
{
   build_threads.queue(build_requests.New().set("xcodebuild", XcodeBuildParams(project, config, platform, sdk)).set(func), BuildRun);
}
void CompileLinux(C Str &project, C Str &config, void func()=null)
{
   build_threads.queue(build_requests.New().set("make", LinuxBuildParams(project, config)).set(func), BuildRun);
}
/******************************************************************************/
Bool CheckNintendoSwitch()
{
   Str path=RootPath+"NintendoSwitch";
   if(FExistSystem(path))return true;
   Gui.msgBox("Error", S+"Nintendo Switch pack not found:\n"+path); return false;
}
/******************************************************************************/
void CopyEngineWindows64GL              () {Copy(EnginePath+"Engine64GL.lib"  , EditorPath+"Bin/Engine64DX11.lib");} // copy as DX11
void CopyEngineWindows64DX9             () {Copy(EnginePath+"Engine64DX9.lib" , EditorPath+"Bin/Engine64DX9.lib");}
void CopyEngineWindows32DX9             () {Copy(EnginePath+"Engine32DX9.lib" , EditorPath+"Bin/Engine32DX9.lib");}
void CopyEngineWindows64DX11            () {Copy(EnginePath+"Engine64DX11.lib", EditorPath+"Bin/Engine64DX11.lib");}
void CopyEngineWindows32DX11            () {Copy(EnginePath+"Engine32DX11.lib", EditorPath+"Bin/Engine32DX11.lib");}
void CopyEngineWindowsUniversal64DX11   () {Copy(EnginePath+"EngineUniversal64DX11.lib"   , EditorPath+"Bin/EngineUniversal64DX11.lib");}
void CopyEngineWindowsUniversal32DX11   () {Copy(EnginePath+"EngineUniversal32DX11.lib"   , EditorPath+"Bin/EngineUniversal32DX11.lib");}
void CopyEngineWindowsUniversalArm32DX11() {Copy(EnginePath+"EngineUniversalArm32DX11.lib", EditorPath+"Bin/EngineUniversalArm32DX11.lib");}
void CopyEngineWindowsUniversalArm64DX11() {Copy(EnginePath+"EngineUniversalArm64DX11.lib", EditorPath+"Bin/EngineUniversalArm64DX11.lib");}
void CopyEngineAndroid                  () {Copy(EnginePath+"EngineAndroid.a"             , EditorPath+"Bin/EngineAndroid.a");}
void CopyEngineNintendoSwitch           () {Copy(EnginePath+"EngineNintendoSwitch.a"      , EditorPath+"Bin/EngineNintendoSwitch.a");}

void CopyEngineDebugWindows64DX9             () {Copy(EnginePath+"EngineDebug64DX9.lib" , EditorPath+"Bin/Engine64DX9.lib");}
void CopyEngineDebugWindows32DX9             () {Copy(EnginePath+"EngineDebug32DX9.lib" , EditorPath+"Bin/Engine32DX9.lib");}
void CopyEngineDebugWindows64DX11            () {Copy(EnginePath+"EngineDebug64DX11.lib", EditorPath+"Bin/Engine64DX11.lib");}
void CopyEngineDebugWindows32DX11            () {Copy(EnginePath+"EngineDebug32DX11.lib", EditorPath+"Bin/Engine32DX11.lib");}
void CopyEngineDebugWindowsUniversal64DX11   () {Copy(EnginePath+"EngineDebugUniversal64DX11.lib"   , EditorPath+"Bin/EngineUniversal64DX11.lib");}
void CopyEngineDebugWindowsUniversal32DX11   () {Copy(EnginePath+"EngineDebugUniversal32DX11.lib"   , EditorPath+"Bin/EngineUniversal32DX11.lib");}
void CopyEngineDebugWindowsUniversalArm32DX11() {Copy(EnginePath+"EngineDebugUniversalArm32DX11.lib", EditorPath+"Bin/EngineUniversalArm32DX11.lib");}
void CopyEngineDebugWindowsUniversalArm64DX11() {Copy(EnginePath+"EngineDebugUniversalArm64DX11.lib", EditorPath+"Bin/EngineUniversalArm64DX11.lib");}
void CopyEngineDebugNintendoSwitch           () {Copy(EnginePath+"EngineDebugNintendoSwitch.a"      , EditorPath+"Bin/EngineNintendoSwitch.a");}

void CompileEngineWindows64GL              () {CompileVS(EnginePath+VSEngineProject, "Release GL"            , "1) 64 bit" , CopyEngineWindows64GL);}
void CompileEngineWindows64DX9             () {CompileVS(EnginePath+VSEngineProject, "Release DX9"           , "1) 64 bit" , CopyEngineWindows64DX9);}
void CompileEngineWindows32DX9             () {CompileVS(EnginePath+VSEngineProject, "Release DX9"           , "2) 32 bit" , CopyEngineWindows32DX9);}
void CompileEngineWindows64DX11            () {CompileVS(EnginePath+VSEngineProject, "Release DX11"          , "1) 64 bit" , CopyEngineWindows64DX11);}
void CompileEngineWindows32DX11            () {CompileVS(EnginePath+VSEngineProject, "Release DX11"          , "2) 32 bit" , CopyEngineWindows32DX11);}
void CompileEngineWindowsUniversal64DX11   () {CompileVS(EnginePath+VSEngineProject, "Release Universal DX11", "1) 64 bit" , CopyEngineWindowsUniversal64DX11);}
void CompileEngineWindowsUniversal32DX11   () {CompileVS(EnginePath+VSEngineProject, "Release Universal DX11", "2) 32 bit" , CopyEngineWindowsUniversal32DX11);}
void CompileEngineWindowsUniversalArm32DX11() {CompileVS(EnginePath+VSEngineProject, "Release Universal DX11", "3) ARM"    , CopyEngineWindowsUniversalArm32DX11);}
void CompileEngineAndroid                  () {CompileVS(EnginePath+VSEngineProject, "Release DX11"          , "6) Android", CopyEngineAndroid);}
void CompileEngineNintendoSwitch           () {if(CheckNintendoSwitch())CompileVS(EnginePath+VSEngineProject, "Release DX11"          , "5) Nintendo Switch", CopyEngineNintendoSwitch);}
void CompileEngineWeb                      () {CompileVS(EnginePath+VSEngineProject, "Release GL"            , "4) Web");}

void CompileEngineDebugWindows64DX9             () {CompileVS(EnginePath+VSEngineProject, "Debug DX9"           , "1) 64 bit", CopyEngineDebugWindows64DX9);}
void CompileEngineDebugWindows32DX9             () {CompileVS(EnginePath+VSEngineProject, "Debug DX9"           , "2) 32 bit", CopyEngineDebugWindows32DX9);}
void CompileEngineDebugWindows64DX11            () {CompileVS(EnginePath+VSEngineProject, "Debug DX11"          , "1) 64 bit", CopyEngineDebugWindows64DX11);}
void CompileEngineDebugWindows32DX11            () {CompileVS(EnginePath+VSEngineProject, "Debug DX11"          , "2) 32 bit", CopyEngineDebugWindows32DX11);}
void CompileEngineDebugWindowsUniversal64DX11   () {CompileVS(EnginePath+VSEngineProject, "Debug Universal DX11", "1) 64 bit", CopyEngineDebugWindowsUniversal64DX11);}
void CompileEngineDebugWindowsUniversal32DX11   () {CompileVS(EnginePath+VSEngineProject, "Debug Universal DX11", "2) 32 bit", CopyEngineDebugWindowsUniversal32DX11);}
void CompileEngineDebugWindowsUniversalArm32DX11() {CompileVS(EnginePath+VSEngineProject, "Debug Universal DX11", "3) ARM"   , CopyEngineDebugWindowsUniversalArm32DX11);}
void CompileEngineDebugNintendoSwitch           () {if(CheckNintendoSwitch())CompileVS(EnginePath+VSEngineProject, "Debug DX11"          , "5) Nintendo Switch", CopyEngineDebugNintendoSwitch);}

void  DelEditorExe          () {FDelFile(EditorSourcePath+ENGINE_NAME " Editor.exe");} // VS has a bug that it won't rebuild the EXE if no changes were made in source (so if EXE was built with DX9 lib, and then we're compiling for DX11, then it may not be relinked)
void CopyEditorWindows64DX11() {    Copy(EditorSourcePath+ENGINE_NAME " Editor.exe", EditorPath+ENGINE_NAME " Editor.exe");}
void CopyEditorWindows32DX9 () {    Copy(EditorSourcePath+ENGINE_NAME " Editor.exe", EditorPath+ENGINE_NAME " Editor 32 DX9.exe");}

void CompileEditorWindows64DX11() {CompileVS(EditorSourcePath+VSEditorProject, "Release DX11", "1) 64 bit", CopyEditorWindows64DX11, DelEditorExe);}
void CompileEditorWindows32DX9 () {CompileVS(EditorSourcePath+VSEditorProject, "Release DX9" , "2) 32 bit", CopyEditorWindows32DX9 , DelEditorExe);}

void CleanEngineWeb    () {build_threads.queue(build_requests.New().set(CleanEngineWebDo), BuildRun);}
void            WebLibs() {build_threads.queue(build_requests.New().set(MakeWebLibs     ), BuildRun);}

void   CleanEngineApple() {CleanXcode(EnginePath      +XcodeEngineProject, "Release", "Mac");
                           CleanXcode(EnginePath      +XcodeEngineProject, "Release", "iOS");
                           CleanXcode(EnginePath      +XcodeEngineProject, "Release", "iOS Simulator", "iphonesimulator");
                           CleanXcode(EnginePath      +XcodeEngineProject, "Debug"  , "Mac");
                           CleanXcode(EnginePath      +XcodeEngineProject, "Debug"  , "iOS");
                           CleanXcode(EnginePath      +XcodeEngineProject, "Debug"  , "iOS Simulator", "iphonesimulator");}
void   CleanEditorMac  () {CleanXcode(EditorSourcePath+XcodeEditorProject, "Release", "Mac");}

void CopyEngineMac         () {Copy(EnginePath+"Build/Release/Engine.a"                , EditorPath+"Bin/Engine Mac.a");}
void CopyEngineiOS         () {Copy(EnginePath+"Build/Release-iphoneos/Engine.a"       , EditorPath+"Bin/Engine iOS.a");}
void CopyEngineiOSSimulator() {Copy(EnginePath+"Build/Release-iphonesimulator/Engine.a", EditorPath+"Bin/Engine iOS Simulator.a");}

void CopyEngineDebugMac         () {Copy(EnginePath+"Build/Debug/Engine.a"                , EditorPath+"Bin/Engine Mac.a");}
void CopyEngineDebugiOS         () {Copy(EnginePath+"Build/Debug-iphoneos/Engine.a"       , EditorPath+"Bin/Engine iOS.a");}
void CopyEngineDebugiOSSimulator() {Copy(EnginePath+"Build/Debug-iphonesimulator/Engine.a", EditorPath+"Bin/Engine iOS Simulator.a");}

void CopyEditorMac() {ReplaceDir(EditorSourcePath+ENGINE_NAME " Editor.app", EditorPath+ENGINE_NAME " Editor.app");}

void CompileEngineMac              () {CompileXcode(EnginePath      +XcodeEngineProject, "Release", "Mac"          , ""               , CopyEngineMac);}
void CompileEngineiOS              () {CompileXcode(EnginePath      +XcodeEngineProject, "Release", "iOS"          , ""               , CopyEngineiOS);}
void CompileEngineiOSSimulator     () {CompileXcode(EnginePath      +XcodeEngineProject, "Release", "iOS Simulator", "iphonesimulator", CopyEngineiOSSimulator);}
void CompileEngineDebugMac         () {CompileXcode(EnginePath      +XcodeEngineProject, "Debug"  , "Mac"          , ""               , CopyEngineDebugMac);}
void CompileEngineDebugiOS         () {CompileXcode(EnginePath      +XcodeEngineProject, "Debug"  , "iOS"          , ""               , CopyEngineDebugiOS);}
void CompileEngineDebugiOSSimulator() {CompileXcode(EnginePath      +XcodeEngineProject, "Debug"  , "iOS Simulator", "iphonesimulator", CopyEngineDebugiOSSimulator);}
void CompileEditorMac              () {CompileXcode(EditorSourcePath+XcodeEditorProject, "Release", "Mac"          , ""               , CopyEditorMac);}

void    CopyEditorLinux    () {Copy(EditorSourcePath+ENGINE_NAME " Editor", EditorPath+ENGINE_NAME " Editor");}
void         CleanLinux    () {build_threads.queue(build_requests.New().set(CleanLinuxDo), BuildRun);}
void CompileEngineLinux    () {CompileLinux(EnginePath      +LinuxEngineProject, "Release");}
void CompileEditorLinux    () {CompileLinux(EditorSourcePath+LinuxEditorProject, "Release", CopyEditorLinux);}
void              LinuxLibs() {build_threads.queue(build_requests.New().set(MakeLinuxLibs), BuildRun);}

void  EnginePak() {build_threads.queue(build_requests.New().set(CreateEnginePak), BuildRun);}
void  EditorPak() {build_threads.queue(build_requests.New().set(CreateEditorPak), BuildRun);}
void    Headers() {build_threads.queue(build_requests.New().set(UpdateHeaders  ), BuildRun);}
void CodeEditorData()
{ // must be done on the main thread
   CE.create    (null, true); // don't use GUI because we don't need it, also it would require "Editor.pak" which may not be available yet
   CE.genSymbols(EditorPath+"Bin");
   CE.del       ();
}

TaskBase CompileEngineWindowsTB[]=
{
 //{null, null, CompileEngineWindows32DX9},
 //{null, null, CompileEngineWindows64DX9},
 //{null, null, CompileEngineWindows32DX11},
   {null, null, CompileEngineWindows64DX11},
   {null, null, CompileEngineWindowsUniversal64DX11},
 //{null, null, CompileEngineWindowsUniversal32DX11},
 //{null, null, CompileEngineWindowsUniversalArm32DX11},
};
TaskBase CompileEngineDebugWindowsTB[]=
{
 //{null, null, CompileEngineDebugWindows32DX9},
 //{null, null, CompileEngineDebugWindows64DX9},
 //{null, null, CompileEngineDebugWindows32DX11},
   {null, null, CompileEngineDebugWindows64DX11},
   {null, null, CompileEngineDebugWindowsUniversal64DX11},
 //{null, null, CompileEngineDebugWindowsUniversal32DX11},
 //{null, null, CompileEngineDebugWindowsUniversalArm32DX11},
};
TaskBase CompileEngineAppleTB[]=
{
   {null, null, CompileEngineMac},
   {null, null, CompileEngineiOS},
   {null, null, CompileEngineiOSSimulator},
};
TaskBase CompileEngineDebugAppleTB[]=
{
   {null, null, CompileEngineDebugMac},
   {null, null, CompileEngineDebugiOS},
   {null, null, CompileEngineDebugiOSSimulator},
};
void CompileEngineWindows     () {FREPAO(CompileEngineWindowsTB     ).queue();}
void CompileEngineDebugWindows() {FREPAO(CompileEngineDebugWindowsTB).queue();}
void CompileEngineApple       () {FREPAO(CompileEngineAppleTB       ).queue();}
void CompileEngineDebugApple  () {FREPAO(CompileEngineDebugAppleTB  ).queue();}

TaskBase CompileEditorWindowsTB[]=
{
   {null, null, CompileEditorWindows64DX11},
 //{null, null, CompileEditorWindows32DX9 },
};
void CompileEditorWindows() {FREPAO(CompileEditorWindowsTB).queue();}
/******************************************************************************/
#define ANDROID_DEFAULT false//WINDOWS
#define     WEB_DEFAULT false

TaskBase TaskBases[]=
{
#if WINDOWS
   {"Compile Windows 64 DX11    "      , "Compile the Engine in Release mode for Windows 64-bit DX10+ WinAPI only"       , CompileEngineWindows64DX11              , false},
   {"Compile Windows 64 DX11 UWP"      , "Compile the Engine in Release mode for Windows 64-bit DX10+ UWP only"          , CompileEngineWindowsUniversal64DX11     , false},
   {"Compile Windows All        "      , "Compile the Engine in Release mode for all Windows targets"                    , CompileEngineWindows                    , true , true},
   {"Compile Windows 64 DX11     Debug", "Compile the Engine in Debug mode for Windows 64-bit DX10+ WinAPI only"         , CompileEngineDebugWindows64DX11         , false},
   {"Compile Windows 64 DX11 UWP Debug", "Compile the Engine in Debug mode for Windows 64-bit DX10+ UWP only"            , CompileEngineDebugWindowsUniversal64DX11, false},
   {"Compile Windows All         Debug", "Compile the Engine in Debug mode for all Windows targets"                      , CompileEngineDebugWindows               , false, true},
#elif APPLE
   {"Clean Apple"               , "Clean temporary files generated during Engine compilation for Apple platforms"       ,   CleanEngineApple        , false},
   {"Compile Mac"               , "Compile the Engine in Release mode for Mac only"                                     , CompileEngineMac          , false},
   {"Compile iOS"               , "Compile the Engine in Release mode for iOS only"                                     , CompileEngineiOS          , false},
   {"Compile iOS Simulator"     , "Compile the Engine in Release mode for iOS Simulator only"                           , CompileEngineiOSSimulator , false},
   {"Compile Apple"             , "Compile the Engine in Release mode for all Apple platforms (Mac, iOS, iOS Simulator)", CompileEngineApple        , true , true},
   {"Compile Mac (Debug)"       , "Compile the Engine in Debug mode for Mac only"                                       , CompileEngineDebugMac     , false},
   {"Compile iOS (Debug)"       , "Compile the Engine in Debug mode for iOS only"                                       , CompileEngineDebugiOS     , false},
   {"Compile Apple (Debug)"     , "Compile the Engine in Debug mode for all Apple platforms (Mac, iOS, iOS Simulator)"  , CompileEngineDebugApple   , false, true},
#elif LINUX
   {"Clean Linux"               , "Clean temporary files generated during Engine/Editor compilation for Linux platform",   CleanLinux               , false},
   {"Compile Linux"             , "Compile the Engine in Release mode for Linux"                                       , CompileEngineLinux         , true },
   {"Make Linux Libs"           , "Make the Engine Linux Lib from the compilation result to the Editor Bin folder"     , LinuxLibs                  , true },
#endif
 /*{"Clean Android"             , "Clean temporary files generated during Engine compilation for Android"              ,   CleanEngineAndroid       , false},
   {"Compile Android"           , "Compile the Engine in Release mode for Android"                                     , CompileEngineAndroid       , ANDROID_DEFAULT},
   {"Make Android Libs"         , "Make the Engine Android Libs from the compilation result to the Editor Bin folder"  , AndroidLibs                , ANDROID_DEFAULT},*/
#if WINDOWS
   {"Compile Android"              , "Compile the Engine in Release mode for Android"                                  , CompileEngineAndroid            , false},
   {"Compile Nintendo Switch"      , "Compile the Engine in Release mode for Nintendo Switch"                          , CompileEngineNintendoSwitch     , false},
   {"Compile Nintendo Switch Debug", "Compile the Engine in Debug mode for Nintendo Switch"                            , CompileEngineDebugNintendoSwitch, false},
   {"Clean Web"                 , "Clean temporary files generated during Engine compilation for the Web"              ,   CleanEngineWeb                , false},
   {"Compile Web"               , "Compile the Engine for Web"                                                         , CompileEngineWeb                , WEB_DEFAULT},
   {"Make Web Libs"             , "Make the Engine Web Lib from the compilation result to the Editor Bin folder"       , WebLibs                         , WEB_DEFAULT},
#endif
   {"Copy Headers"              , "Copy cleaned Engine Headers from the Engine folder to the Editor folder.\nCleaning removes all 'EE_PRIVATE' sections from the headers."                                                                    , Headers       , true },
   {"Create \"Code Editor.dat\"", "Create \"Code Editor.dat\" file needed for Code Editor in the Engine's Editor.\nThis data is generated from the Engine headers in the Editor Bin folder, which are generated in the \"Copy Headers\" step.", CodeEditorData, true },
   {"Create \"Engine.pak\""     , "Create \"Engine.pak\" file from the \"Data\" folder into the Editor Bin folder"       , EnginePak                , true },
   {"Create \"Editor.pak\""     , "Create \"Editor.pak\" file from the \"Editor Data\" folder into the Editor Bin folder", EditorPak                , true },
#if WINDOWS
   {"Compile Editor"            , "Compile the Editor"                                                                 , CompileEditorWindows64DX11 , true },
#elif APPLE
   {"Compile Editor"            , "Compile the Editor"                                                                 , CompileEditorMac           , true },
#elif LINUX
   {"Compile Editor"            , "Compile the Editor"                                                                 , CompileEditorLinux         , true },
#endif
};
Memc<TaskBase> CustomTasks;
/******************************************************************************/
void Resize(Flt=0, Flt=0)
{
   D.scale(D.screenH()/Flt(D.resH())*(1000./1080));
   region.rect(Rect(-D.w()+0.53, -D.h(), D.w(), D.h()));
   clear.rect(Rect_RU(region.rect().ru()-Vec2(region.slidebarSize(), 0), 0.2, 0.05));
   copy .rect(Rect_RU(clear.rect().lu(), 0.2, 0.05));

   Flt x=-D.w()+0.02, y=D.h()-0.10, h=0.067;
   OptionsButton.rect(Rect_L(x+0.06, y, 0.45, 0.05)); y-=h;
   y-=0.02;
   FREPA(Tasks){Tasks[i].pos(x, y); y-=h;}
   y-=0.02;

     All.rect(Rect_L(x     , y, 0.05, 0.05));
   DoSel.rect(Rect_L(x+0.06, y, 0.45, 0.05));
}
/******************************************************************************/
Bool ExtractTry(File &f, C ZipFile &zip_file, C Str &dest, DateTime *utc)
{
   if(f.pos(zip_file.offset))
   {
      FCreateDirs(GetPath(dest));
      File temp;
      return DecompressRaw(f, temp, COMPRESS_ZLIB, zip_file.compressed_size, zip_file.uncompressed_size, true)
          && temp.pos(0)
          && SafeOverwrite(temp, dest, utc);
   }
   return false;
}
Bool ExtractTry(C Str &name)
{
   File f; if(f.readStdTry(name))
   {
      Memt<ZipFile> files; if(ParseZip(f, files))
      {
         Str path=GetPath(name).tailSlash(true);
         FREPA(files)
         {
          C ZipFile &zip_file=files[i]; if(zip_file.uncompressed_size) // process only files with data
            {
               Str dest=path+zip_file.name;
               FileInfoSystem fi(dest);
               DateTime utc=zip_file.modify_time_local; utc.toUTC();
               if(fi.size!=zip_file.uncompressed_size || fi.modify_time_utc!=utc) // need to extract
                  if(!ExtractTry(f, zip_file, dest, &utc))return false;
            }
         }
         return true;
      }
   }
   return false;
}
void Extract(C Str &path) {if(!ExtractTry(path))Exit(S+"Can't extract: "+path);}
/******************************************************************************/
void InitPre()
{
   App.flag=APP_RESIZABLE|APP_MINIMIZABLE|APP_MAXIMIZABLE|APP_NO_PAUSE_ON_WINDOW_MOVE_SIZE|APP_WORK_IN_BACKGROUND|APP_FULL_TOGGLE;
   App.x=1;
   App.y=1;
#if DEBUG
   App.flag|=APP_BREAKPOINT_ON_ERROR|APP_MEM_LEAKS|APP_CALLSTACK_ON_ERROR;
#endif
   D.screen_changed=Resize;
   Flt scale=D.screenH()/1080.0f;
   D.mode(800*scale, 725*scale);
   App.name("Engine Builder");

   for(Str path=GetPath(App.exe()); ; path=GetPath(path))
   {
      if(!path.is())Exit("Can't find Engine Root Path");
      if(FExistSystem(path+"/Data")
      && FExistSystem(path+"/Editor Data")
      && FExistSystem(path+"/Editor")
      && FExistSystem(path+"/Engine"))
      {
         RootPath=path.tailSlash(true);
         if(!FullPath(    EngineDataPath))    EngineDataPath=RootPath+    EngineDataPath;
         if(!FullPath(    EditorDataPath))    EditorDataPath=RootPath+    EditorDataPath;
         if(!FullPath(        EnginePath))        EnginePath=RootPath+        EnginePath;
         if(!FullPath(        EditorPath))        EditorPath=RootPath+        EditorPath;
         if(!FullPath(  EditorSourcePath))  EditorSourcePath=RootPath+  EditorSourcePath;
         if(!FullPath(ThirdPartyLibsPath))ThirdPartyLibsPath=RootPath+ThirdPartyLibsPath;
         break;
      }
   }

   DataPath(EngineDataPath);

   // extract LIB files that are too big for GitHub to handle
#if WINDOWS
   Extract(RootPath+"ThirdPartyLibs/Xbox Live/2018.6.20181010.2/x64.zip");
#endif
}
void SetPaths()
{
   // setup VS path if not specified
   if(!VSPath.is() || !VSPath2010.is())
   {
      Memc<VisualStudioInstallation> installs; GetVisualStudioInstallations(installs);
      if(!VSPath.is())
      {
         REPA(installs) // go from the end to try the latest version first
         {
          C VisualStudioInstallation &install=installs[i]; if(CheckVisualStudio(install.ver) && DevEnvPath(install.path).is())
            {
               VSPath=install.path; break;
            }
         }
      }
      if(!VSPath2010.is())
      {
         REPA(installs) // go from the end to try the latest version first
         {
          C VisualStudioInstallation &install=installs[i]; if(install.ver.x==10 && DevEnvPath(install.path).is())
            {
               VSPath2010=install.path; break;
            }
         }
      }
   }

   // setup Android NDK path if not specified
   /*if(!AndroidNDKPath.is())
   {
      Str AndroidSDKPath=GetPath(GetRegStr(RKG_LOCAL_MACHINE, "Software/Android SDK Tools/Path"));
      if( AndroidSDKPath.is())for(FileFind ff(AndroidSDKPath); ff(); )
      {
         Str ndk_path=ff.pathName(); if(NDKBuildPath(ndk_path).is())
         {
            AndroidNDKPath=ndk_path; break;
         }
      }
   }*/
}
Bool Init()
{
   SetPaths();
   Options.create();
   Gui+=region.create();
   ListColumn lc[]=
   {
      ListColumn(DATA_STR, 0, SIZE(Str), LCW_MAX_DATA_PARENT, u"Output"),
   };
   region+=list.create(lc, Elms(lc)); list.elmHeight(0.037).textSize(0.037); FlagDisable(list.flag, LIST_SORTABLE); region.slidebar[0].scrollOptions(0.75f); region.slidebar[1].scrollOptions(0.75f);
   Gui+=copy .create("Copy" ).func(CopyOutput);
   Gui+=clear.create("Clear").func(Clear);

   Gui+=OptionsButton.create("Options").func(OptionsToggle);
#ifdef CUSTOM_TASKS
   CUSTOM_TASKS
#endif
   FREPA(TaskBases  )Tasks.New().create(TaskBases  [i]);
   FREPA(CustomTasks)Tasks.New().create(CustomTasks[i]);
   Gui+=All  .create().func(ToggleAll).set(true, QUIET);
   Gui+=DoSel.create("Do Selected").func(DoSelected).desc("Execute all selected tasks in order");

   Resize();
   build_threads.create(true, Cpu.threads(), 0, "BuildThreads");
   return true;
}
/******************************************************************************/
void Shut()
{
   Options.save();
   shut=true;
   build_threads.del();
}
/******************************************************************************/
Bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   if(!App.active())Time.wait(1);

   if(!build_threads.queued() && Todo.elms())
   {
      MemcThreadSafeLock lock(Todo); if(Todo.elms())
      {
         App.stayAwake(AWAKE_SYSTEM); // don't sleep when building
         done++; // start with small progress
         App.stateProgress(Flt(done)/(Todo.elms()+done));
         if(TaskBase *t=Todo.lockedPopFirst())t->call();
      }
   }
   if(done && !build_threads.queued() && !Todo.elms()) // finished all tasks
   {
      done=0;
      App.stateNormal().flash();
      App.stayAwake(AWAKE_OFF); // allow sleeping
   }

   if(data_new.elms())
   {
      Bool at_end=region.slidebar[1].wantedAtEnd(0.02f);
      SyncLocker locker(lock);
      FREPA(data_new)Swap(data.New(), data_new[i]);
      data_new.clear();
      list.setData(data);

      if(at_end)region.scrollEndY();
   }

   Gui.update();

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(AZURE);
   TextStyleParams ts; ts.size=0.055; ts.align.set(1, -1); D.text(ts, -D.w()+0.01, D.h()-0.01, S+"Busy:"+build_threads.queued()+", Queued:"+Todo.elms());
   Gui.draw();
}
/******************************************************************************/
