/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
UpdaterClass Updater;
/******************************************************************************/
UpdateWindowClass UpdateWindow;
/******************************************************************************/
bool UpdateDo()
{
   // <- unload DLL's here

   bool inside=StartsPath(App.exe(), Updater.path);
   Str  old=App.exe(); // remember old app name

#if WINDOWS
   // move exe to temporary file so 'FMoveDir' will succeed for the exe as well
   if(inside)
   {
      Str rel=SkipStartPath(App.exe(), Updater.path);
      if(FExistSystem(Updater.update_path+rel)) // if there exists a replacement for that file
      {
         if(!App.renameSelf(Updater.path+APP_NAME+" Old."+GetExt(old))){Gui.msgBox(S, "Can't update self"); if(InstallerMode)StateInstall.set(StateFadeTime);else StateProjectList.set(StateFadeTime); return true;}
         App.deleteSelfAtExit(); // delete the "Editor Old" file
      }
   }
#endif

   // remove those that are not wanted (do this first, before replacing files)
   FREPA(Updater.local_remove) // go from start to recycle folders first
   {
      Str full=Updater.path+Updater.local_files[Updater.local_remove[i]].full_name;
   #if WINDOWS
      if(EqualPath(full, App.exe())){App.deleteSelfAtExit(); continue;} // on Windows we can't recycle self
   #endif
      FRecycle(full);
   }

   // replace all files
   FMoveDir(Updater.update_path, Updater.path);

   if(inside && FExistSystem(old))RunAtExit=old;

   if(InstallerMode)
   {
      Str ext;
   #if WINDOWS
      ext=".exe";
   #elif MAC
      ext=".app";
   #elif LINUX
      ext="";
   #else 
      #error unknown platform
   #endif
      CreateShortcut(Updater.path+APP_NAME+ext, SystemPath(SP_DESKTOP  ).tailSlash(true)+APP_NAME);
      CreateShortcut(Updater.path+APP_NAME+ext, SystemPath(SP_MENU_PROG).tailSlash(true)+APP_NAME);
   }

   // finish
   if(RunAtExit.is())
   {
      // if we run app on exit, then no messages need to be displayed
   }else // we won't run any app
   {
      Explore(Updater.path); // open folder so user can see what's happened there
      App.hide(); // hide window because 'OSMsgBox' causes a pause
      /*if(inside)*/OSMsgBox("Success", InstallerMode ? ENGINE_NAME " has installed successfully." : ENGINE_NAME " has updated successfully."); // show confirmation using OS msg box
      //else        Gui.msgBox("Success", ENGINE_NAME " has updated properly.");
   }
   /*if(inside)*/return false; // close the application
   //else StateInstall.set(StateFadeTime); // if we're not inside then it means we're an installer
}
/******************************************************************************/
State StateUpdate(UpdateUpdate, DrawUpdate, InitUpdate, ShutUpdate);
Str   UpdateMessage;
/******************************************************************************/
bool InitUpdate()
{
   App.stateWorking();
   SetKbExclusive();
   Proj.close();

   // auto close all processes in path folder except this one - App.exe
   Memc<uint> proc; ProcList(proc);
   REPA(proc)if(proc[i]!=App.processID())
   {
      Str proc_name=ProcName(proc[i]);
      if(StartsPath(proc_name, Updater.path))ProcClose(proc_name);
   }
   UpdateMessage.clear();

   return true;
}
void ShutUpdate()
{
   
}
bool UpdateUpdate()
{
   if(Kb.bp(KB_ESC) || Kb.bp(KB_NAV_BACK))
   {
      if(!InstallerMode)StateProjectList.set(StateFadeTime);
   }
   Time.wait(1000/30);
   Gui.update();
   Server.update(null, true);
   if(Ms.bp(MS_MAXIMIZE))App.window().toggle();

   UpdateMessage.clear();
   Memc<uint> proc; ProcList(proc);
   REPA(proc)if(proc[i]!=App.processID())
   {
      Str proc_name=ProcName(proc[i]);
      if(StartsPath(proc_name, Updater.path))
      {
         if(!UpdateMessage.is())UpdateMessage+="Waiting for applications to exit:";
         UpdateMessage.line()+=proc_name;
      }
   }
   if(!UpdateMessage.is()) // if there are no active apps then perform update
      return UpdateDo();

   return true;
}
void DrawUpdate()
{
   D.clear(BackgroundColor());
   Gui.draw();
   D.text(D.rect(), UpdateMessage);
   Draw();
}
/******************************************************************************/
State    StateInstall(UpdateInstall, DrawInstall, InitInstall, ShutInstall);
WindowIO InstallIO;
bool     InstallerMode=false;
/******************************************************************************/
Str InstallPath()
{
   Str path=InstallIO.path(); path.tailSlash(true)+=InstallIO.subPath();
   if(InstallIO.textline().is())
   {
      if(FullPath(InstallIO.textline()))path=InstallIO.textline();else path.tailSlash(true)+=InstallIO.textline();
      path=Replace(NormalizePath(path), '/', '\\');
   }
   if(path.tailSlash(false).is())
      if(!ContainsAny(path, "Esenthel Titan " ENGINE_NAME))path.tailSlash(true)+=ENGINE_NAME;
   return path;
}
void SelectInstall(C Str &path, ptr)
{
   if(!InstallPath().is())InstallIO.activate();else Updater.create(); // reactivate if user selected empty/invalid path
}
void ResizeInstall()
{
   flt w=Min(0.6f, D.w());
   InstallIO.rect(Rect(-w, -D.h(), w, D.h()-0.17f));
}
/******************************************************************************/
bool InitInstall()
{
   SetKbExclusive();
   if(!InstallIO.is())
   {
      InstallIO.create(S, S, S, SelectInstall, SelectInstall).modeDirSelect(); FlagDisable(InstallIO.flag, WIN_MOVABLE|WIN_RESIZABLE);
      InstallIO.button[2].func(MiscRegion::Quit, Misc).hide();
      InstallIO.cancel   .func(MiscRegion::Quit, Misc);
      InstallIO.activate();
      ResizeInstall();
   }
   Gui+=UpdateProgress.create(Rect_C(0, -0.05f, 1, 0.045f));
   Gui+=InstallIO;
   return true;
}
void ShutInstall()
{
   UpdateProgress.del();
   Gui-=InstallIO;
}
bool UpdateInstall()
{
   if(!App.active())Time.wait(1000/30);else if(!D.sync())Time.wait(1000/100);
   Gui.update();
   if(!InstallIO.visible() && !Updater.updating())
   {
      if(Updater.ready) // success
      {
         StateUpdate.set(StateFadeTime);
      }else // failed
      {
         InstallIO.activate();
         App.stateError(Updater.progress());
      }
   }
   if(!InstallIO.visible())App.stateProgress(Updater.progress());
   UpdateProgress.set(Updater.progress());
   if(Ms.bp(MS_MAXIMIZE))App.window().toggle();
   if(!App.maximized())REPA(MT)if(MT.b(i))
   {
      GuiObj *go=MT.guiObj(i); if(!go || go==Gui.desktop() || go->type()==GO_WINDOW && !(go->asWindow().flag&WIN_MOVABLE))App.window().move(MT.pixelDelta(i).x, MT.pixelDelta(i).y);
   }
   return true;
}
void DrawInstall()
{
   D.clear(BackgroundColor());
   TextStyleParams ts;
   ts.size=0.055f; D.text(ts, 0, D.h()-0.05f, InstallIO.visible() ? "Please select path for " ENGINE_NAME " installation" : "Installing to");
   ts.size=0.045f; D.text(ts, 0, D.h()-0.11f, S+"\""+InstallPath()+'"');
   Gui.draw();
   Draw();
}
/******************************************************************************/

/******************************************************************************/
   cchar8   *UpdaterClass::TutorialsProjID="541oob6a_h_5el6-6!zan_50";
   const int UpdaterClass::MaxDownloadAttempts=3;
/******************************************************************************/
   bool      UpdaterClass::CreateFailedDownload(int &failed, C Str &file, ptr user) {failed=0; return true;}
   FILE_LIST_MODE UpdaterClass::Filter(C FileFind &ff, UpdaterClass &updater)
   {
      if(updater.thread.wantStop())return FILE_LIST_BREAK;
      if(ff.name==".DS_Store")return FILE_LIST_SKIP;

      Str rel=SkipStartPath(ff.pathName(), updater.path);
      if( rel=="Settings.txt"
      ||  rel=="Server Settings"
      ||  rel=="Projects (Server)"
      ||  rel==S+APP_NAME+" Old.exe"
      ||  rel==S+APP_NAME+" Old.exe.bat" // used by 'App.deleteSelfAtExit'
      ||  rel==S+APP_NAME+" Old.app"
      ||  rel==S+APP_NAME+" Old"
      ||  EqualPath(rel, "Bin/Store.dat")
      ||  EqualPath(rel, "Bin/Code Editor.font")
      ||  EqualPath(rel, "Bin/Update")
      )return FILE_LIST_SKIP; // skip these elements

      if(StartsPath(rel, "Projects"))
      {
         Str project=SkipStartPath(rel, "Projects");
         if( project.is())
         {
            if(!StartsPath(project, TutorialsProjID))return FILE_LIST_SKIP; // list only default "Tutorials" project
         }
      }

      updater.local_files.New().set(rel, ff);
      return FILE_LIST_CONTINUE;
   }
   FILE_LIST_MODE UpdaterClass::FilterUpdate(C FileFind &ff, UpdaterClass &updater)
   {
      if(updater.thread.wantStop())return FILE_LIST_BREAK;
      if(ff.name==".DS_Store")return FILE_LIST_SKIP;
      updater.local_files.New().set(SkipStartPath(ff.pathName(), updater.update_path), ff);
      return FILE_LIST_CONTINUE;
   }
   FILE_LIST_MODE UpdaterClass::HasUpdate(C FileFind &ff, UpdaterClass &updater)
   {
      if(updater.thread.wantStop())return FILE_LIST_BREAK;
      if(ff.name==".DS_Store")return FILE_LIST_SKIP;
      Str rel=SkipStartPath(ff.pathName(), updater.update_path);
      if(StartsPath(rel, "Projects"))
      {
         Str project=SkipStartPath(rel, "Projects");
         if(!project.is()            )return FILE_LIST_CONTINUE; // keep checking inside but don't report "Projects"  folder  as an update
         if( project==TutorialsProjID)return FILE_LIST_SKIP    ; // don't check inside   and don't report "Tutorials" project as an update
      }
      updater.has_update=true; // this is a file other than "Projects" folder and "Tutorials" project, which means that it's a new update, report that we have an update and stop processing
      return FILE_LIST_BREAK;
   }
   bool UpdaterClass::hasUpdate()
   {
      if(0) // don't notify about updates if they're just about file removal
         if(C Pak *pak=patcher.index())
            REPA(local_remove) // iterate all files for removal
      {
       C Str &name=local_files[local_remove[i]].full_name;
         if(!pak->find(name))if(FExistSystem(path+name))return true; // if a file is not present on the server but exists locally
      }
      has_update=false; FList(update_path, HasUpdate, T); // check if there are any files to update (except "Tutorials" project)
      return has_update;
   }
   bool UpdaterClass::updating() {return thread.active();}
   flt  UpdaterClass::progress() {if(long total=patcher.filesSize())return dbl(patcher.progress())/total; return 0;}
   bool UpdaterClass::Update(Thread &thread) {((UpdaterClass*)thread.user)->update(); return false;}
          void UpdaterClass::update() // !! this is called on secondary thread !!
   {
      if(InstallerMode)App.stayAwake(AWAKE_SYSTEM);
      updateEx();
      patcher.del();
      if(InstallerMode)App.stayAwake(AWAKE_OFF);
   }
   void UpdaterClass::updateEx() // !! this is called on secondary thread !!
   {
      // download latest ver
      Str upload_name;
   #if WINDOWS
      upload_name="Esenthel";
   #elif MAC
      upload_name="Esenthel Mac";
   #elif LINUX
      upload_name="Esenthel Linux";
   #else 
      #error unknown platform
   #endif
      patcher.create("http://esenthel.com/download/Patcher", upload_name);
      patcher.downloadIndex();

      // check local ver (do this before update ver)
      path=(InstallerMode ? InstallPath() : GetPath(App.exe())).tailSlash(true);
      FList(path, Filter, T); if(thread.wantStop())return;

      // check local update ver (do this after original ver so these files will overwrite previous info)
      update_path=path+"Bin\\Update\\";
      FList(update_path, FilterUpdate, T); if(thread.wantStop())return;

      // wait for patcher index
      for(; ; Time.wait(1))
      {
         if(thread.wantStop())return;
         if(patcher.indexState()==DWNL_ERROR){if(InstallerMode)Gui.msgBox(S, "Can't access Esenthel Server"); return;}
         if(patcher.indexState()==DWNL_DONE )break;
      }

      // process what to remove/download
      Memc<int> server_download;
      if(!patcher.compare(local_files, local_remove, server_download))return;
      bool deleted=false; REPA(local_remove)deleted|=FDel(update_path+local_files[local_remove[i]].full_name); // from leafs to root to delete files first (we're operating on temporary files here so there's no need for 'FRecycle')
      if(  deleted) // if deleted any file then list local files again, in case now we have them up to date
      {
         local_files.clear();
         FList(       path, Filter      , T); if(thread.wantStop())return;
         FList(update_path, FilterUpdate, T); if(thread.wantStop())return;
         if(!patcher.compare(local_files, local_remove, server_download))return;
      }
      FREPA(server_download)       patcher.downloadFile(server_download[i]); // from root to download folders first
       REPA( local_remove  )FDel(update_path+local_files[local_remove  [i]].full_name); // from leafs to root to delete files first (we're operating on temporary files here so there's no need for 'FRecycle')

      // download files
      bool ok=true;
      Map<Str, int> failed_download(ComparePathCI, CreateFailedDownload); // doesn't need to be thread-safe, this is a list of files that failed to download (how many times)
      for(;;)
      {
         if(thread.wantStop())return;
         Patcher::Downloaded download;
         if(patcher.getNextDownload(download))
         {
            if(download.success)
            {
               Str full=update_path+download.full_name;
               switch(download.type)
               {
                  case FSTD_DIR: FCreateDirs(full); break;

                  case FSTD_LINK:
                  {
                     FCreateDirs(GetPath(full));
                     FDelFile(full);
                     if(CreateSymLink(full, DecodeSymLink(download.data)))FTimeUTC(full, download.modify_time_utc);else{Gui.msgBox("Error", S+"Can't write to:\n\""+full+'"'); return;}
                  }break;

                  case FSTD_FILE:
                  {
                     FCreateDirs(GetPath(full));
                     if(!SafeOverwrite(download.data, full, &download.modify_time_utc)){Gui.msgBox("Error", S+"Can't write to:\n\""+full+"\"\nTry running as administrator."); return;}
                  }break;
               }
            }else
            {
               int &failed =*failed_download(download.full_name);
               if(++failed>=MaxDownloadAttempts)
               {
                  ok=false;
                  if(InstallerMode){Gui.msgBox(S, S+"Error downloading file:\n\""+download.full_name+"\"\nPlease try again.\nUsing the same path will resume the download."); return;}
               }else // try again
               {
                  patcher.downloadFile(download.index);
               }
            }
         }else
         if(!patcher.filesLeft())break;else Time.wait(1);
      }

      // update
      if(ok) // if all succeeded
         if(InstallerMode || hasUpdate()) // there is an actual update
      {
         ready=true;
         if(!InstallerMode)App.addFuncCall(AskUpdate); // call on the main thread
      }
   }
   void UpdaterClass::AskUpdate() {UpdateWindow.create();}
   void UpdaterClass::del()
   {
      thread.del(); // delete the thread first
      ready=false;
      patcher     .del();
      local_remove.del();
      local_files .del();
   }
   void UpdaterClass::create()
   {
      del();
   #if DESKTOP && !STEAM
      #if DEBUG
         if(ForceInstaller>=-1)
      #endif
            thread.create(Update, this, 0, false, "Update");
   #endif
   }
   void UpdaterClass::check()
   {
      if(Time.realTime()>=time && UpdateWindow.hidden() && !InstallerMode)
      {
         time=Time.realTime()+12*60*60; // next check in 12 hours
         create();
      }
   }
  UpdaterClass::~UpdaterClass() {del();}
   void UpdateWindowClass::ApplyDo(bool all_saved, ptr) {if(all_saved){UpdateWindow.hide(); if(Updater.ready)StateUpdate.set(StateFadeTime);}}
   void UpdateWindowClass::ShowChanges(UpdateWindowClass &uw) {Explore("https://esenthel.com/forum/forumdisplay.php?fid=8");}
   void UpdateWindowClass::Apply(UpdateWindowClass &uw) {if(StateActive==&StateProject)SaveChanges(ApplyDo);else ApplyDo();}
   void UpdateWindowClass::create()
   {
      Gui+=super       ::create(Rect_C(0, 0, 1, 0.48f)).barVisible(false).hide().fadeIn(); button[2].func(HideEditAct, SCAST(GuiObj, T)).show();
      T  +=text        .create(Vec2  (clientWidth()/2, -0.19f), "An update to " ENGINE_NAME " is available.\nWould you like to apply it now?\n\nWarning: Applying update will restore the default\n\"Tutorials\" project to its original state.\nAny changes you've made to it will be lost.");
      T  +=apply       .create(Rect_D(clientWidth()*1/6, -clientHeight()+0.04f, 0.26f, 0.06f), "Apply"       ).focusable(false).func(Apply, T);
      T  +=show_changes.create(Rect_D(clientWidth()/2  , -clientHeight()+0.04f, 0.32f, 0.06f), "Show Changes").focusable(false).func(ShowChanges, T);
      T  +=not_now     .create(Rect_D(clientWidth()*5/6, -clientHeight()+0.04f, 0.26f, 0.06f), "Not Now"     ).focusable(false).func(HideEditAct, SCAST(GuiObj, T));
   }
   GuiObj* UpdateWindowClass::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
      return (alpha()>=1) ? super::test(gpc, pos, mouse_wheel) : null; // check for alpha to avoid accidental clicks when window suddenly appears
   }
UpdaterClass::UpdaterClass() : ready(false), has_update(false), time(0) {}

/******************************************************************************/
