/******************************************************************************/
class EEItem
{
   bool         opened=false;
   byte         flag=0; // ELM_FLAG
   ELM_TYPE     type=ELM_NONE;
   ImagePtr     icon;
   Str          base_name, full_name;
   EEItem      *parent=null;
   Memx<EEItem> children;
}
class CodeView : Region, Edit.CodeEditorInterface
{
   Memx<EEItem > items;
   Memc<EEItem*> items_sorted;

   void clearAuto()
   {
      android_asset_packs=-1;
   }

   virtual void configChangedDebug()override
   {
      Misc.build.menu("Debug"  ,  configDebug(), QUIET);
      Misc.build.menu("Release", !configDebug(), QUIET);
   }
   virtual void configChanged32Bit()override
   {
      Misc.build.menu("32-bit",  config32Bit(), QUIET);
      Misc.build.menu("64-bit", !config32Bit(), QUIET);
   }
   virtual void configChangedAPI()override
   {
      //Misc.build.menu("DirectX 11", configAPI(), QUIET);
   }
   virtual void configChangedEXE()override
   {
      Misc.build.menu("Windows EXE"      , configEXE()==Edit.EXE_EXE  , QUIET);
      Misc.build.menu("Windows DLL"      , configEXE()==Edit.EXE_DLL  , QUIET);
      Misc.build.menu("Windows LIB"      , configEXE()==Edit.EXE_LIB  , QUIET);
      Misc.build.menu("Windows Universal", configEXE()==Edit.EXE_UWP  , QUIET);
      Misc.build.menu("Android APK"      , configEXE()==Edit.EXE_APK  , QUIET);
      Misc.build.menu("Android AAB"      , configEXE()==Edit.EXE_AAB  , QUIET);
      Misc.build.menu("Mac APP"          , configEXE()==Edit.EXE_MAC  , QUIET);
      Misc.build.menu("iOS APP"          , configEXE()==Edit.EXE_IOS  , QUIET);
      Misc.build.menu("Linux"            , configEXE()==Edit.EXE_LINUX, QUIET);
      Misc.build.menu("Web"              , configEXE()==Edit.EXE_WEB  , QUIET);
      Misc.build.menu("Nintendo Switch"  , configEXE()==Edit.EXE_NS   , QUIET);
      Misc.build.text=Edit.ShortName(configEXE());
      Proj.refresh(false, true); // 'refresh' because 'finalPublish' depends on Platform 'configEXE', have to reset publish, set invalid refs (missing dependencies), warnings, etc.
      Proj.elmChangedParentRemovePublish();
   }
   virtual void visibleChangedOptions      ()override {Misc.build.menu("View Options"           , visibleOptions      (), QUIET);}
   virtual void visibleChangedOpenedFiles  ()override {}
   virtual void visibleChangedOutput       ()override {Misc.build.menu("View Output"            , visibleOutput       (), QUIET);}
   virtual void visibleChangedAndroidDevLog()override {Misc.build.menu("View Android Device Log", visibleAndroidDevLog(), QUIET);}

   virtual UID projectID()override {return Proj.id;}

   virtual UID               appID                              ()override {if(Elm *app=Proj.findElm(Proj.curApp()))                                  return app.id                          ; return super.appID();}
   virtual Str               appName                            ()override {if(Elm *app=Proj.findElm(Proj.curApp()))                                  return app.name                        ; return super.appName();}
   virtual Str               appDirsWindows                     ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.dirs_windows           ; return super.appDirsWindows();}
   virtual Str               appDirsMac                         ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.dirs_mac               ; return super.appDirsMac();}
   virtual Str               appDirsLinux                       ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.dirs_linux             ; return super.appDirsLinux();}
   virtual Str               appDirsAndroid                     ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.dirs_android           ; return super.appDirsAndroid();}
   virtual Str               appDirsiOS                         ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.dirs_ios               ; return super.appDirsiOS();}
   virtual Str               appDirsNintendo                    ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.dirs_nintendo          ; return super.appDirsNintendo();}
   virtual Str               appHeadersWindows                  ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.headers_windows        ; return super.appHeadersWindows();}
   virtual Str               appHeadersMac                      ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.headers_mac            ; return super.appHeadersMac();}
   virtual Str               appHeadersLinux                    ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.headers_linux          ; return super.appHeadersLinux();}
   virtual Str               appHeadersAndroid                  ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.headers_android        ; return super.appHeadersAndroid();}
   virtual Str               appHeadersiOS                      ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.headers_ios            ; return super.appHeadersiOS();}
   virtual Str               appHeadersNintendo                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.headers_nintendo       ; return super.appHeadersNintendo();}
   virtual Str               appLibsWindows                     ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.libs_windows           ; return super.appLibsWindows();}
   virtual Str               appLibsMac                         ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.libs_mac               ; return super.appLibsMac();}
   virtual Str               appLibsLinux                       ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.libs_linux             ; return super.appLibsLinux();}
   virtual Str               appLibsAndroid                     ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.libs_android           ; return super.appLibsAndroid();}
   virtual Str               appLibsiOS                         ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.libs_ios               ; return super.appLibsiOS();}
   virtual Str               appLibsNintendo                    ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.libs_nintendo          ; return super.appLibsNintendo();}
   virtual Str               appPackage                         ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.package                ; return super.appPackage();}
   virtual UID               appMicrosoftPublisherID            ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.ms_publisher_id        ; return super.appMicrosoftPublisherID();}
   virtual Str               appMicrosoftPublisherName          ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.ms_publisher_name      ; return super.appMicrosoftPublisherName();}
   virtual Edit.XBOX_LIVE    appXboxLiveProgram                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.xbl_program            ; return super.appXboxLiveProgram();}
   virtual ULong             appXboxLiveTitleID                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.xbl_title_id           ; return super.appXboxLiveTitleID();}
   virtual UID               appXboxLiveSCID                    ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.xbl_scid               ; return super.appXboxLiveSCID();}
   virtual Str               appGooglePlayLicenseKey            ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.android_license_key    ; return super.appGooglePlayLicenseKey();}
   virtual bool              appGooglePlayAssetDelivery         ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.playAssetDelivery()    ; return super.appGooglePlayAssetDelivery();}
   virtual Str               appLocationUsageReason             ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.location_usage_reason  ; return super.appLocationUsageReason();}
   virtual Str               appNintendoInitialCode             ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.nintendo_initial_code  ; return super.appNintendoInitialCode();}
   virtual ULong             appNintendoAppID                   ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.nintendo_app_id        ; return super.appNintendoAppID();}
   virtual Str               appNintendoPublisherName           ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.nintendo_publisher_name; return super.appNintendoPublisherName();}
   virtual Str               appNintendoLegalInformation        ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.nintendo_legal_info    ; return super.appNintendoLegalInformation();}
   virtual Int               appBuild                           ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.build                  ; return super.appBuild();}
   virtual Long              appSaveSize                        ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.save_size              ; return super.appSaveSize();}
   virtual ulong             appFacebookAppID                   ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.fb_app_id              ; return super.appFacebookAppID();}
   virtual Str               appAdMobAppIDiOS                   ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.am_app_id_ios          ; return super.appAdMobAppIDiOS();}
   virtual Str               appAdMobAppIDGooglePlay            ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.am_app_id_google       ; return super.appAdMobAppIDGooglePlay();}
   virtual Str               appChartboostAppIDiOS              ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.cb_app_id_ios          ; return super.appChartboostAppIDiOS();}
   virtual Str               appChartboostAppSignatureiOS       ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.cb_app_signature_ios   ; return super.appChartboostAppSignatureiOS();}
   virtual Str               appChartboostAppIDGooglePlay       ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.cb_app_id_google       ; return super.appChartboostAppIDGooglePlay();}
   virtual Str               appChartboostAppSignatureGooglePlay()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.cb_app_signature_google; return super.appChartboostAppSignatureGooglePlay();}
   virtual Edit.STORAGE_MODE appPreferredStorage                ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.storage                ; return super.appPreferredStorage();}
   virtual UInt              appSupportedOrientations           ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.supported_orientations ; return super.appSupportedOrientations();}
   virtual UID               appGuiSkin                         ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.gui_skin               ; return super.appGuiSkin();}
   virtual int               appEmbedEngineData                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.embedEngineData ()     ; return super.appEmbedEngineData();}
   virtual Cipher*           appEmbedCipher                     ()override {static ProjectCipher cipher; /*if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())*/return cipher.set(Proj)(); return super.appEmbedCipher();}
   virtual Bool              appPublishProjData                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.publishProjData ()     ; return super.appPublishProjData();}
   virtual Bool              appPublishPhysxDll                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.publishPhysxDll ()     ; return super.appPublishPhysxDll();}
   virtual Bool              appPublishSteamDll                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.publishSteamDll ()     ; return super.appPublishSteamDll();}
   virtual Bool              appPublishOpenVRDll                ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.publishOpenVRDll()     ; return super.appPublishOpenVRDll();}
   virtual Bool              appPublishDataAsPak                ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.publishDataAsPak()     ; return super.appPublishDataAsPak();}
 //virtual Bool              appWindowsCodeSign                 ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())return app_data.windowsCodeSign ()     ; return super.appWindowsCodeSign();}
   virtual ImagePtr          appIcon                            ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())if(app_data.icon             .valid())return ImagePtr().get(Proj.gamePath(app_data.icon             )); return super.appIcon();}
   virtual ImagePtr          appImagePortrait                   ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())if(app_data.image_portrait   .valid())return ImagePtr().get(Proj.gamePath(app_data.image_portrait   )); return super.appImagePortrait();}
   virtual ImagePtr          appImageLandscape                  ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())if(app_data.image_landscape  .valid())return ImagePtr().get(Proj.gamePath(app_data.image_landscape  )); return super.appImageLandscape();}
   virtual ImagePtr          appNotificationIcon                ()override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())if(app_data.notification_icon.valid())return ImagePtr().get(Proj.gamePath(app_data.notification_icon)); return super.appNotificationIcon();}
   virtual void              appLanguages                       (MemPtr<LANG_TYPE> langs)override {if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData()){langs=app_data.supported_languages; return;} return super.appLanguages(langs);}

   virtual int  editorAddrPort(              )override {return EditServer.port        ();}
   virtual void editorAddr    (SockAddr &addr)override {  addr=EditServer.localAddress();}

   virtual void focus()override {if(Mode.tabAvailable(MODE_CODE))Mode.set(MODE_CODE);}

   static void ImageGenerateProcess(ImageGenerate &generate, ptr user, int thread_index) {ThreadMayUseGPUData(); generate.process();}

   virtual COMPRESS_TYPE appEmbedCompress     (                           Edit.EXE_TYPE exe_type)override {/*if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())*/return     Proj.compress_type [ProjCompres(exe_type)]; return super.appEmbedCompress     (exe_type);}
   virtual int           appEmbedCompressLevel(                           Edit.EXE_TYPE exe_type)override {/*if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())*/return     Proj.compress_level[ProjCompres(exe_type)]; return super.appEmbedCompressLevel(exe_type);}
   virtual DateTime      appEmbedSettingsTime (                           Edit.EXE_TYPE exe_type)override {/*if(Elm *app=Proj.findElm(Proj.curApp()))if(ElmApp *app_data=app.appData())*/return Max(Proj.compress_time [ProjCompres(exe_type)], Max(Proj.cipher_time, Proj.cipher_key_time)).asDateTime(); return super.appEmbedSettingsTime(exe_type);} // return Max of all params affecting PAKs
   virtual void          appSpecificFiles     (MemPtr<PakFileData> files, Edit.EXE_TYPE exe_type)override
   {
      Memc<ImageGenerate> generate;
      Memc<ImageConvert>  convert;
      Memt<Elm*>          app_elms; Proj.getActiveAppElms(app_elms, exe_type);
      AddPublishFiles(app_elms, files, generate, convert, exe_type);
      // all generations/conversions need to be processed here so 'files' point correctly
      WorkerThreads.process1(generate, ImageGenerateProcess);
      if(convert.elms()) // image compression is already multi-threaded, so allow only one at a time
      {
         ThreadMayUseGPUData       (); FREPAO(convert).process();
         ThreadFinishedUsingGPUData();
      }
   }
   virtual void appInvalidProperty(C Str &msg)override
   {
      if(Elm *app=Proj.findElm(Proj.curApp()))AppPropsEdit.set(app); Gui.msgBox(S, msg);
   }

   virtual void validateActiveSources()override {Proj.activateSources(1);}

   virtual Rect menuRect()override
   {
      Rect r=D.rect();
      if(         Mode.visibleTabs())MIN(r.max.y,          Mode.rect().min.y);
      if(     MtrlEdit.visible    ())MIN(r.max.x,      MtrlEdit.rect().min.x);
      if(WaterMtrlEdit.visible    ())MIN(r.max.x, WaterMtrlEdit.rect().min.x);

      if(Misc.visible() && Misc.pos.y==(int)CodeMenuOnTop && !(Mode.visibleTabs() && Misc.pos.y))
      {
         if(Misc.move_misc.visible())
         {
            if(Misc.pos.x==0)MAX(r.min.x, Misc.move_misc.rect().max.x);
            else             MIN(r.max.x, Misc.move_misc.rect().min.x);
         }else
         {
            if(Misc.pos.x==0)MAX(r.min.x, Misc.rect().max.x);
            else             MIN(r.max.x, Misc.rect().min.x);
         }
      }else if(Proj.visible())MAX(r.min.x, Proj.rect().max.x);

      return r;
   }
   virtual Rect sourceRect()override {return EditRect();}

   virtual Str sourceProjPath(C UID &id)override
   {
      Str path;
      if(Elm *elm=Proj.findElm(id))
         for(path=CleanFileName(elm.name); elm=Proj.findElm(elm.parent_id); ) // always add name of first element
      {
         if(elm.type!=ELM_APP && elm.type!=ELM_LIB && elm.type!=ELM_FOLDER)continue; // don't add name of a parent if it's not a folder
         if(elm.type==ELM_APP)return path; // return before adding app name
         path=CleanFileName(elm.name).tailSlash(true)+path;
         if(elm.type==ELM_LIB)return path; // return after  adding lib name
      }
      return S; // parent is not app and not lib, then return empty string
   }
   virtual Edit.ERROR_TYPE sourceDataLoad(C UID &id, Str &data)override
   {
      if(Elm *elm=Proj.findElm(id))
      {
         if(ElmCode *code_data=elm.codeData())
         {
            Edit.ERROR_TYPE error=LoadCode(data, Proj.codePath(id));
            if(error==Edit.EE_ERR_FILE_NOT_FOUND && !code_data.ver)return Edit.EE_ERR_ELM_NOT_DOWNLOADED; // if file not found and ver not set yet, then report as not yet downloaded
            return error;
         }
         return Edit.EE_ERR_ELM_NOT_CODE;
      }
      return Edit.EE_ERR_ELM_NOT_FOUND;
   }
   virtual Bool sourceDataSave(C UID &id, C Str &data)override
   {
      if(Elm *elm=Proj.findElm(id))if(ElmCode *code_data=elm.codeData())if(SaveCode(data, Proj.codePath(id)))
      {
         code_data.newVer();
         code_data.from(data);
         return true;
      }
      return false;
   }
   virtual void sourceChanged(bool activate=false)override
   {
      Mode.tabAvailable(MODE_CODE, sourceCurIs());
      if(activate && sourceCurIs()){Mode.set(MODE_CODE); HideBig();} // set mode in case Code Editor triggers source opening
      SetTitle();
      if(activate)Proj.refresh();
   }
   virtual Bool elmValid    (C UID &id              )override {return Proj.findElm(id)!=null;}
   virtual Str  elmBaseName (C UID &id              )override {if(Elm *elm=Proj.findElm(id))return CleanFileName(elm.name); return S;}
   virtual Str  elmFullName (C UID &id              )override {return Proj.elmFullName(id);}
   virtual void elmHighlight(C UID &id, C Str  &name)override {Proj.elmHighlight(id, name);}
   virtual void elmOpen     (C UID &id              )override {if(Elm *elm=Proj.findElm(id))if(!(selected() && id==sourceCurId()))Proj.elmToggle(elm);} // don't toggle currently opened source because it will close it
   virtual void elmLocate   (C UID &id              )override {Proj.elmLocate(id);}
   virtual void elmPreview  (C UID &id, C Vec2 &pos, bool mouse, C Rect &clip)override
   {
      if(Elm *elm=Proj.findElm(id))
      {
         Rect_LU r(pos, D.h()*0.65); if(mouse)r-=D.pixelToScreenSize(VecI2(0, 32)); // avg mouse cursor height
         if(r.max.x>clip.max.x){r.min.x-=r.max.x-clip.max.x; r.max.x=clip.max.x;} if(r.min.x<clip.min.x){r.max.x+=clip.min.x-r   .min.x; r.min.x=clip.min.x;}
         if(r.min.y<clip.min.y){r+=Vec2(0, pos.y-r.min.y+0.01)                 ;} if(r.max.y>clip.max.y){r.min.y-=   r.max.y-clip.max.y; r.max.y=clip.max.y;}
         Preview.draw(*elm, r);
      }
   }
   virtual Str idToText(C UID &id, Bool *valid)override {return Proj.idToText(id, valid, ProjectEx.ID_SKIP_UNKNOWN);} // if we're in code then it already displays an ID, no need to write the same ID on top of that

   virtual void getProjPublishElms(Memc<ElmLink> &elms)override // get all publishable elements in the project, this is used for auto-complete
   {
      elms.clear(); FREPA(Proj.elms) // process in order
      {
         Elm &elm=Proj.elms[i];
         if(elm.finalPublish() && ElmPublish(elm.type) && ElmVisible(elm.type))elms.New().set(elm.id, Proj.elmFullName(&elm), Proj.elmIcon(elm.type));
      }
   }

   Str importPaths(C Str &path)C {return super.importPaths() ? GetRelativePath(GetPath(App.exe()), path) : path;}

   void drag(C MemPtr<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(selected())
      {
         Memc<UID> temp;
         FREPA(elms)if(Elm *elm=Proj.findElm(elms[i]))if(ElmPublish(elm.type))temp.add(elm.id); // filter out non-publishable elements
         super.paste(temp, obj, screen_pos);
      }
   }
   void drop(C MemPtr<Str> &names, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(selected())
      {
         Str text;
         FREPA(names)
         {
            if(i)text+=", ";
            text+='"';
            text+=Replace(names[i], '\\', '/');
            text+='"';
         }
         super.paste(text, obj, screen_pos);
      }
   }

   bool selected()C {return Mode()==MODE_CODE;}
   void selectedChanged() {menuEnabled(selected());}
   void flush() {} // do nothing because sources need to be saved manually

   void overwriteChanges()
   {
      REPA(Proj.elms)if(Proj.elms[i].type==ELM_CODE)sourceOverwrite(Proj.elms[i].id);
   }
   void sourceTitleChanged(C UID &id) // call if name or "modified state" changed
   {
      if(selected() && id==sourceCurId())SetTitle();
   }
   void sourceRename(C UID &id)
   {
      super.sourceRename(id);
      sourceTitleChanged(id);
   }
   bool sourceDataSet(C UID &id, C Str &data)
   {
      if(super.sourceDataSet(id, data)){sourceTitleChanged(id); return true;}
      return false;
   }

   void cleanAll()
   {
      super.cleanAll(); // first call super to stop any build in progress
      FDelDirs(ProjectsPath.tailSlash(true)+ProjectsPublishPath);
   }

   static int Compare(C Enum.Elm &a, C Enum.Elm &b) {return .Compare(a.name, b.name, true);}
   void makeAuto(bool publish=false)
   {
      if(Proj.valid())
      {
         Memt<Enum.Elm> obj_types; REPA(Proj.existing_obj_classes)if(Elm *elm=Proj.findElm(Proj.existing_obj_classes[i]))obj_types.New().set(NameToEnum(elm.name), elm.id); obj_types.sort(Compare);
         int max_length=0; REPA(obj_types)MAX(max_length, Length(obj_types[i].name));

         Str data;
         data ="/******************************************************************************/\n";
         // generate header
         data+=  "// THIS IS AN AUTOMATICALLY GENERATED FILE\n";
         data+=S+"// THIS FILE WAS GENERATED BY "+CaseUp(ENGINE_NAME)+" EDITOR, ACCORDING TO PROJECT SETTINGS\n";
         data+="/******************************************************************************/\n";
         data+="// CONSTANTS\n";
         data+="/******************************************************************************/\n";
         // generate constants
         data+=S+"#define    STEAM   "+appPublishSteamDll ()+" // if Application properties have Steam enabled\n" ; // display it here even if it's just Auto.cpp and doesn't affect other codes, so that the user sees the macro and can be aware of it
         data+=S+"#define    OPEN_VR "+appPublishOpenVRDll()+" // if Application properties have OpenVR enabled\n"; // display it here even if it's just Auto.cpp and doesn't affect other codes, so that the user sees the macro and can be aware of it
         data+=S+"const bool PUBLISH          =" +TextBool(publish)+"; // this is set to true when compiling for publishing\n";
         data+=S+"const bool EMBED_ENGINE_DATA=("+TextBool(appEmbedEngineData()!=0)+" && !WINDOWS_NEW && !MOBILE && !WEB); // this is set to true when \"Embed Engine Data\" was enabled in application settings, this is always disabled for WindowsNew, Mobile and Web builds\n";
       //data+=S+"cchar *C   EE_SDK_PATH      =                                     \""+Replace(SDKPath()                             , '\\', '/').tailSlash(false)+"\";\n";
       //data+=S+"cchar *C   EE_PHYSX_DLL_PATH=((WINDOWS_NEW || MOBILE || WEB) ? null             : PUBLISH ? u\"Bin\"             : u\""+Replace(BinPath()             , '\\', '/').tailSlash(false)+"\");\n";
         data+=S+"cchar *C    ENGINE_DATA_PATH=((WINDOWS_NEW || MOBILE || WEB) ? u\"Engine.pak\"  : PUBLISH ? u\"Bin/Engine.pak\"  : u\""+Replace(BinPath()+"Engine.pak", '\\', '/').tailSlash(false)+"\");\n";
         data+=S+"cchar *C   PROJECT_DATA_PATH=((WINDOWS_NEW || MOBILE || WEB) ? u\"Project.pak\" : PUBLISH ? u\"Bin/Project.pak\" : u\""+Replace(Proj.game_path        , '\\', '/').tailSlash(false)+"\");\n";
         data+=S+"cchar *C   PROJECT_NAME     =u\""+CString(Proj.name)+"\";\n";
         data+=S+"cchar *C   APP_NAME         =u\""+CString(appName())+"\";\n";
         data+=S+"const int  APP_BUILD        ="+appBuild()+";\n";
         data+=S+"const UID  APP_GUI_SKIN     ="+appGuiSkin().asCString()+";\n";
       //data+=S+"const bool GOOGLE_PLAY_ASSET_DELIVERY="+TextBool(appGooglePlayAssetDelivery())+"; // this is set to true when project data is managed by Google Play Asset Delivery\n";
         if(cchar8 *cipher_class=(InRange(Proj.cipher, CIPHER_NUM) ? CipherText[Proj.cipher].clazz : null))
         {
            data+=S+cipher_class+"   _PROJECT_CIPHER   ("; FREPA(Proj.cipher_key){if(i)data+=", "; data+=Proj.cipher_key[i];} data+=");\n";
            data+=S+      "Cipher *C  PROJECT_CIPHER   =&_PROJECT_CIPHER;\n";
         }else
         {
            data+=S+"Cipher *C  PROJECT_CIPHER   =null;\n";
         }
         data+="/******************************************************************************/\n";
         data+="// FUNCTIONS\n";
         data+="/******************************************************************************/\n";
         // functions
         data+="void INIT(bool load_engine_data=true, bool load_project_data=true)\n";
         data+="{\n";
         data+="   App.name(APP_NAME); // set application name\n";
if(appGuiSkin().valid())data+="   Gui.default_skin=APP_GUI_SKIN; // set default Gui Skin\n"; // override only if specified
         data+="   INIT_OBJ_TYPE(); // initialize object type enum\n";
         data+="   LoadEmbeddedPaks(PROJECT_CIPHER); // load data embedded in Application executable file\n";
         data+="   if(load_engine_data )if(!EMBED_ENGINE_DATA)Paks.add(ENGINE_DATA_PATH); // load engine data\n";
         data+="   if(load_project_data) // load project data\n";
         data+="   {\n";
      if(android_asset_packs>=0) // generate this code only when using asset packs
       data+=S+"      if(ANDROID)LoadAndroidAssetPacks("+android_asset_packs+", PROJECT_CIPHER);else\n";
         data+="      if(WINDOWS_NEW || MOBILE || WEB || PUBLISH)Paks.add(PROJECT_DATA_PATH, PROJECT_CIPHER);else DataPath(PROJECT_DATA_PATH);\n";
         data+="   }\n";
         data+="}\n";
         data+="void INIT_OBJ_TYPE() // this function will setup 'ObjType' enum used for object types\n";
         data+="{\n";
         if(obj_types.elms())
         {
            data+="   Enum.Elm elms[]= // list of Editor created Object Classes\n";
            data+="   {\n";
            FREPA(obj_types)
            {
               int length=Length(obj_types[i].name);
               data+=S+"      {\""+CString(obj_types[i].name)+'"'; REP(max_length-length)data+=' '; data+=", "; data+=obj_types[i].id.asCString(); data+="},\n";
            }
            data+="   };\n";
            data+="   ObjType.create(\"OBJ_TYPE\", elms); // create 'ObjType' enum\n";
         }
         data+="}\n";
         data+="/******************************************************************************/\n";
         data+="// ENUMS\n";
         data+="/******************************************************************************/\n";
         // generate enums
         // OBJ_TYPE
         data+="enum OBJ_TYPE // Object Class Types\n";
         data+="{\n";
         FREPA(obj_types)
         {
            int length=Length(obj_types[i].name);
            data+=S+"   "+obj_types[i].name; REP(max_length-length)data+=' '; data+=",\n";
         }
         data+="}\n";
         // custom enums
         FREPA(Proj.existing_enums)
            if(Enum *e=Enums.get(Proj.gamePath(Proj.existing_enums[i])))
            if(Elm *elm=Proj.findElm(Proj.existing_enums[i]))
            if(ElmEnum *enum_data=elm.enumData())
         {
            data+=S+"enum "+e.name;
            switch(enum_data.type)
            {
               case EditEnums.TYPE_1: data+=" : byte"  ; break;
               case EditEnums.TYPE_2: data+=" : ushort"; break;
               case EditEnums.TYPE_4: data+=" : uint"  ; break;
            }
            data+="\n{\n";
            int max_length=0; REPA(*e)MAX(max_length, Length((*e)[i].name));
            FREPA(*e)
            {
               int length=Length((*e)[i].name);
               data+=S+"   "+(*e)[i].name; REP(max_length-length)data+=' '; data+=",\n";
            }
            data+="}\n";
         }
         if(1 || Proj.existing_enums.elms())data+="/******************************************************************************/\n";
         sourceAuto(data);
      }
   }

   void codeDo(Edit.BUILD_MODE mode)
   {
      switch(mode)
      {
         case Edit.BUILD_BUILD  :                 super.build  ();                               break;
         case Edit.BUILD_PUBLISH: makeAuto(true); super.publish(); clearAuto(); makeAuto(false); break; // 'clearAuto' so we already set correct codes with this 'makeAuto'
         case Edit.BUILD_PLAY   :                 super.play   ();                               break;
         case Edit.BUILD_DEBUG  :                 super.debug  ();                               break;
      }
   }
   void buildDo(Edit.BUILD_MODE mode)
   {
      if(PublishDataNeeded(configEXE()))StartPublish(S, configEXE(), mode);else 
      {
         Proj.flush(); // flush in case we want to play with latest data
         codeDo(mode);
      }
   }
   void build  () {buildDo(Edit.BUILD_BUILD  );}
   void publish() {buildDo(Edit.BUILD_PUBLISH);}
   void play   () {buildDo(Edit.BUILD_PLAY   );}
   void debug  () {buildDo(Edit.BUILD_DEBUG  );}
   void rebuild() {clean(); rebuildSymbols(); build();} // override to call custom 'build'
   void openIDE()
   {
      if(PublishDataNeeded(configEXE()))StartPublish(S, configEXE(), Edit.BUILD_IDE, true, S, true);else // we need to create project data pak first
      {
         Proj.flush(); // flush in case we want to play with latest data
         super.openIDE();
      }
   }
   bool Export(Edit.EXPORT_MODE mode, bool data=false)
   {
      bool ok=false;
      makeAuto(true); // before exporting, reset auto header to force PUBLISH as true, important because exported projects are meant to be distributed to other computers, and compiled for publishing (such as EE Editor), in such case they can't be using paths from this computer, therefore publishing will make them use target paths
      if(ok=super.Export(mode))if(data)
      {
         Edit.EXE_TYPE exe=Edit.EXE_NUM;
         switch(mode)
         {
            case Edit.EXPORT_EXE    : exe=configEXE() ; break;
            case Edit.EXPORT_ANDROID: exe=Edit.EXE_APK; break;
            case Edit.EXPORT_XCODE  : exe=Edit.EXE_IOS; break;

            // #VisualStudio
            case Edit.EXPORT_VS    :
            case Edit.EXPORT_VS2015:
            case Edit.EXPORT_VS2017:
            case Edit.EXPORT_VS2019:
            case Edit.EXPORT_VS2022:
               exe=Edit.EXE_UWP; break;
         }
         if(exe!=Edit.EXE_NUM && PublishDataNeeded(exe))StartPublish(S, exe, Edit.BUILD_PUBLISH, true);
      }
      makeAuto(false); // restore after exporting
      return ok;
   }

   virtual void publishSuccess(C Str &exe_name, Edit.EXE_TYPE exe_type, Edit.BUILD_MODE build_mode, C UID &project_id)override // code compiled successfully
   {
      if(Proj.id==project_id && StateActive==&StateProject)
      {
         if(PublishDataNeeded(exe_type)) // if data was already published then package is ready
         {
            PublishSuccess(exe_name, S+"File name: "+GetBase(exe_name)+"\nFile size: "+SizeBytes(FSize(exe_name)));
         }else // proceed to data publishing
         {
            StartPublish(exe_name, exe_type, build_mode);
         }
      }
   }

   virtual CodeView& del()override
   {
      CodeEditorInterface.del();
                   Region.del();
      return T;
   }
   static int CompareItem(EEItem *C &a, EEItem *C &b) {return ComparePathNumber(a.full_name, b.full_name);}
   static int CompareItem(EEItem *C &a,     C Str &b) {return ComparePathNumber(a.full_name, b          );}
   void createItems(Memx<EEItem> &dest, C Memx<Edit.Item> &src, EEItem *parent)
   {
      FREPA(src)
      {
       C Edit.Item &s=src[i];
            EEItem &d=dest.New();
         d.base_name=s.base_name;
         d.full_name=s.full_name;
         d.type     =(s.children.elms() ? ELM_FOLDER : ELM_CODE);
         d.icon     =((d.type==ELM_FOLDER) ? Proj.icon_folder : Proj.icon_code);
         d.parent   =parent;
         if(d.full_name.is())items_sorted.binaryInclude(&d, CompareItem);
         createItems(d.children, s.children, &d);
      }
   }
   void create(GuiObj &parent)
   {
      parent+=Region.create();
      CodeEditorInterface.create(parent, CodeMenuOnTop);
      EEItem &ee=items.New();
      ee.base_name=ENGINE_NAME " Engine";
      ee.type=ELM_LIB;
      ee.icon=Proj.icon_lib;
      createItems(ee.children, CodeEditorInterface.items(), &ee);
      if(C TextNode *code=Settings.findNode("Code"))loadSettings(*code);
   }
   void resize(bool full=true)
   {
      rect(EditRect());
      if(full)CodeEditorInterface.resize();
   }
   void activate(Elm *elm)
   {
      if(elm && elm.type==ELM_CODE)
      {
         sourceCur(elm.id); Mode.set(MODE_CODE);
      }
   }
   void toggle(Elm *elm)
   {
      if(elm && elm.type==ELM_CODE)
      {
         if(sourceCurId()!=elm.id)sourceCur(elm.id);else if(selected())close();else Mode.set(MODE_CODE);
      }
   }
   void toggle(EEItem *item)
   {
      if(item && item.type==ELM_CODE && item.full_name.is())
      {
         if(!EqualPath(sourceCurName(), item.full_name))sourceCur(item.full_name);else if(selected())close();else Mode.set(MODE_CODE);
      }
   }
   void erasing(C UID &elm_id) {sourceRemove(elm_id);}
   void kbSet() {CodeEditorInterface.kbSet();}

   virtual GuiObj* test  (C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override {return null;}
   virtual void    update(C GuiPC &gpc)override
   {
    //Region             .update(gpc);
      CodeEditorInterface.update(StateActive==&StateProject);
   }
   virtual void draw(C GuiPC &gpc)override
   {
   }
   void drawPreview(ListElm *list_elm)
   {
      if(EEItem *item=list_elm.item)
         if(item.type==ELM_CODE)
            if(item.full_name.is())
               if(selected() ? !EqualPath(sourceCurName(), item.full_name) : true) // don't preview source which is currently being edited
                  sourceDrawPreview(item.full_name);

      if(Elm *elm=list_elm.elm)
         if(ElmCode *code_data=elm.codeData())
            if(selected() ? sourceCurId()!=elm.id : true) // don't preview source which is currently being edited
               sourceDrawPreview(elm.id);
   }
}
CodeView CodeEdit;
/******************************************************************************/
class AppPropsEditor : PropWin
{
   static const LANG_TYPE Languages[]={EN, DE, LANG_DUTCH, FR, IT, SP, PO, PL, RU, JP, KO, CN, TH};
   enum ORIENT
   {
      ORIENT_PORTRAIT        ,
      ORIENT_LANDSCAPE       ,
      ORIENT_ALL             ,
      ORIENT_ALL_NO_DOWN     ,
      ORIENT_PORTRAIT_LOCKED ,
      ORIENT_LANDSCAPE_LOCKED,
   }
   static cchar8 *OrientName[]=
   { 
      "Portrait",
      "Landscape",
      "Portrait and Landscape",
      "Portrait and Landscape (No Down)",
      "Locked Portrait",
      "Locked Landscape",
   };
   static NameDesc EmbedEngine[]=
   {
      {u"No"     , u"Engine data will not be embedded in the Application executable, instead, a standalone \"Engine.pak\" file will get created, which the Application has to load at startup."},
      {u"2D Only", u"Engine data will be embedded in the Application executable, and standalone \"Engine.pak\" file will not get created.\nHowever this option embeds only simple data used for 2D graphics only, shaders used for 3D graphics will not be included, if your application will attempt to render 3D graphics an error will occur."},
      {u"Full"   , u"Engine data will be embedded in the Application executable, and standalone \"Engine.pak\" file will not get created.\nAll engine data will get embedded, including support for 2D and 3D graphics."},
   };
   static NameDesc StorageName[]=
   {
      {u"Internal", u"The application must be installed on the internal device storage only. If this is set, the application will never be installed on the external storage. If the internal storage is full, then the system will not install the application."},
      {u"External", u"The application prefers to be installed on the external storage (SD card). There is no guarantee that the system will honor this request. The application might be installed on internal storage if the external media is unavailable or full, or if the application uses the forward-locking mechanism (not supported on external storage). Once installed, the user can move the application to either internal or external storage through the system settings."},
      {u"Auto"    , u"The application may be installed on the external storage, but the system will install the application on the internal storage by default. If the internal storage is full, then the system will install it on the external storage. Once installed, the user can move the application to either internal or external storage through the system settings."},
   }; ASSERT(Edit.STORAGE_INTERNAL==0 && Edit.STORAGE_EXTERNAL==1 && Edit.STORAGE_AUTO==2);
   static ORIENT FlagToOrient(uint flag)
   {
      bool portrait =FlagOn(flag, DIRF_Y),
           landscape=FlagOn(flag, DIRF_X);
      if( flag==(DIRF_X|DIRF_UP)  )return ORIENT_ALL_NO_DOWN     ;
      if((flag&DIRF_X)==DIRF_RIGHT)return ORIENT_LANDSCAPE_LOCKED;
      if((flag&DIRF_Y)==DIRF_UP   )return ORIENT_PORTRAIT_LOCKED ;
      if( landscape && !portrait  )return ORIENT_LANDSCAPE       ;
      if(!landscape &&  portrait  )return ORIENT_PORTRAIT        ;
                                   return ORIENT_ALL             ;
   }
   static uint OrientToFlag(ORIENT orient)
   {
      switch(orient)
      {
         default                     : return DIRF_X|DIRF_Y ; // ORIENT_ALL
         case ORIENT_ALL_NO_DOWN     : return DIRF_X|DIRF_UP;
         case ORIENT_PORTRAIT        : return        DIRF_Y ;
         case ORIENT_LANDSCAPE       : return DIRF_X        ;
         case ORIENT_PORTRAIT_LOCKED : return DIRF_UP       ;
         case ORIENT_LANDSCAPE_LOCKED: return DIRF_RIGHT    ;
      }
   }

   class AppImage : ImageSkin
   {
      Button      remove;
      Image       image_2d;
      ImagePtr    game_image;
      MemberDesc  md, md_time;

      static void Remove(AppImage &ai) {ai.setImage(UIDZero);}

      void setImage()
      {
         ElmApp *app=(AppPropsEdit.elm ? AppPropsEdit.elm.appData() : null);
         UID     id=md.asUID(app);
         if(id.valid())game_image=Proj.gamePath(id);else game_image=null;
         image=game_image; image_2d.del();
         if(image && image->is() && image->mode()!=IMAGE_2D)
         {
            VecI2 size=image->size();
            if(size.x>256)size=size*256/size.x;
            if(size.y>256)size=size*256/size.y;
            image->copyTry(image_2d, Max(size.x, 1), Max(size.y, 1), 1, ImageTypeUncompressed(image->type()), IMAGE_2D, 1, FILTER_BEST, IC_CLAMP|IC_ALPHA_WEIGHT);
            image=&image_2d;
         }
         remove.visible(id.valid());
         desc(S+"\""+Proj.elmFullName(id)+"\"\nDrag and drop an image here");
      }
      void setImage(C UID &image_id)
      {
         if(ElmApp *app=(AppPropsEdit.elm ? AppPropsEdit.elm.appData() : null))
         {
            md.fromUID(app, image_id); md_time.as<TimeStamp>(app).getUTC(); AppPropsEdit.setChanged();
            setImage();
         }
      }
      AppImage& create(C MemberDesc &md, C MemberDesc &md_time, GuiObj &parent, C Rect &rect)
      {
         T.md=md;
         T.md_time=md_time;
         parent+=super.create(rect); fit=true;
         parent+=remove.create(Rect_RU(rect.ru(), 0.045, 0.045)).func(Remove, T); remove.image="Gui/close.img";
         return T;
      }
   }

   UID       elm_id=UIDZero;
   Elm      *elm=null;
   bool      changed=false, changed_headers=false;
   Property *p_icon=null, *p_image_portrait=null, *p_image_landscape=null, *p_notification_icon=null, *p_languages=null;
   AppImage    icon     ,    image_portrait     ,    image_landscape     ,    notification_icon;
   Tabs      platforms;
   PropExs   win_props, mac_props, linux_props, android_props, ios_props, nintendo_props;

   static void Changed              (C Property &prop) {AppPropsEdit.setChanged();}
   static void GetAndroidLicenseKey (  ptr           ) {Explore("https://play.google.com/console/");}
   static void GetFacebookAppID     (  ptr           ) {Explore("https://developers.facebook.com/apps");}
   static void GetAdMobApp          (  ptr           ) {Explore("https://apps.admob.com");}
   static void GetChartboostApp     (  ptr           ) {Explore("https://dashboard.chartboost.com/tools/sdk");}
   static void GetMicrosoftPublisher(  ptr           ) {Explore("https://partner.microsoft.com/en-us/dashboard/account/v3/organization/legalinfo");}
   static void GetXboxLive          (  ptr           ) {Explore("https://partner.microsoft.com/en-us/dashboard/windows/overview");}
   static void GetNintendo          (  ptr           ) {Explore("https://developer.nintendo.com/group/development/products");}
   static void GetNintendoLegal     (  ptr           ) {Explore("https://slim.mng.nintendo.net/slim/home");}

   static void DirsWin                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.dirs_windows=text; app_data.dirs_windows_time.getUTC(); ap.changed_headers=true;}}
   static Str  DirsWin                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.dirs_windows; return S;}
   static void DirsMac                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.dirs_mac=text; app_data.dirs_mac_time.getUTC(); ap.changed_headers=true;}}
   static Str  DirsMac                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.dirs_mac; return S;}
   static void DirsLinux                   (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.dirs_linux=text; app_data.dirs_linux_time.getUTC(); ap.changed_headers=true;}}
   static Str  DirsLinux                   (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.dirs_linux; return S;}
   static void DirsAndroid                 (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.dirs_android=text; app_data.dirs_android_time.getUTC(); ap.changed_headers=true;}}
   static Str  DirsAndroid                 (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.dirs_android; return S;}
   static void DirsiOS                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.dirs_ios=text; app_data.dirs_ios_time.getUTC(); ap.changed_headers=true;}}
   static Str  DirsiOS                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.dirs_ios; return S;}
   static void DirsNintendo                (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.dirs_nintendo=text; app_data.dirs_nintendo_time.getUTC(); ap.changed_headers=true;}}
   static Str  DirsNintendo                (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.dirs_nintendo; return S;}
   static void HeadersWin                  (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.headers_windows=text; app_data.headers_windows_time.getUTC(); ap.changed_headers=true;}}
   static Str  HeadersWin                  (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.headers_windows; return S;}
   static void HeadersMac                  (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.headers_mac=text; app_data.headers_mac_time.getUTC(); ap.changed_headers=true;}}
   static Str  HeadersMac                  (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.headers_mac; return S;}
   static void HeadersLinux                (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.headers_linux=text; app_data.headers_linux_time.getUTC(); ap.changed_headers=true;}}
   static Str  HeadersLinux                (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.headers_linux; return S;}
   static void HeadersAndroid              (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.headers_android=text; app_data.headers_android_time.getUTC(); ap.changed_headers=true;}}
   static Str  HeadersAndroid              (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.headers_android; return S;}
   static void HeadersiOS                  (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.headers_ios=text; app_data.headers_ios_time.getUTC(); ap.changed_headers=true;}}
   static Str  HeadersiOS                  (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.headers_ios; return S;}
   static void HeadersNintendo             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.headers_nintendo=text; app_data.headers_nintendo_time.getUTC(); ap.changed_headers=true;}}
   static Str  HeadersNintendo             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.headers_nintendo; return S;}
   static void LibsWin                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.libs_windows=text; app_data.libs_windows_time.getUTC();}}
   static Str  LibsWin                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.libs_windows; return S;}
   static void LibsMac                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.libs_mac=text; app_data.libs_mac_time.getUTC();}}
   static Str  LibsMac                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.libs_mac; return S;}
   static void LibsLinux                   (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.libs_linux=text; app_data.libs_linux_time.getUTC();}}
   static Str  LibsLinux                   (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.libs_linux; return S;}
   static void LibsAndroid                 (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.libs_android=text; app_data.libs_android_time.getUTC();}}
   static Str  LibsAndroid                 (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.libs_android; return S;}
   static void LibsiOS                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.libs_ios=text; app_data.libs_ios_time.getUTC();}}
   static Str  LibsiOS                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.libs_ios; return S;}
   static void LibsNintendo                (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.libs_nintendo=text; app_data.libs_nintendo_time.getUTC();}}
   static Str  LibsNintendo                (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.libs_nintendo; return S;}
   static void Package                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.package=text; app_data.package_time.getUTC();}}
   static Str  Package                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.package; return S;}
   static void MicrosoftPublisherID        (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.ms_publisher_id.fromCanonical(SkipStart(text, "CN=")); app_data.ms_publisher_id_time.getUTC();}}
   static Str  MicrosoftPublisherID        (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())if(app_data.ms_publisher_id.valid())return S+"CN="+CaseUp(app_data.ms_publisher_id.asCanonical()); return S;}
   static void MicrosoftPublisherName      (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.ms_publisher_name=text; app_data.ms_publisher_name_time.getUTC();}}
   static Str  MicrosoftPublisherName      (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.ms_publisher_name; return S;}
   static void XboxLiveProgram             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.xbl_program=(Edit.XBOX_LIVE)TextInt(text); app_data.xbl_program_time.getUTC();}}
   static Str  XboxLiveProgram             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.xbl_program; return S;}
   static void XboxLiveTitleID             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.xbl_title_id=TextULong(text); app_data.xbl_title_id_time.getUTC();}}
   static Str  XboxLiveTitleID             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())if(app_data.xbl_title_id)return app_data.xbl_title_id; return S;}
   static void XboxLiveSCID                (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.xbl_scid.fromCanonical(text); app_data.xbl_scid_time.getUTC();}}
   static Str  XboxLiveSCID                (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())if(app_data.xbl_scid.valid())return CaseDown(app_data.xbl_scid.asCanonical()); return S;}
   static void NintendoInitialCode         (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.nintendo_initial_code=text; app_data.nintendo_initial_code_time.getUTC();}}
   static Str  NintendoInitialCode         (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.nintendo_initial_code; return S;}
   static void NintendoAppID               (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.nintendo_app_id=TextULong(text); app_data.nintendo_app_id_time.getUTC();}}
   static Str  NintendoAppID               (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())if(app_data.nintendo_app_id)return TextHex(app_data.nintendo_app_id, 16, 0, true); return S;}
   static void NintendoPublisherName       (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.nintendo_publisher_name=text; app_data.nintendo_publisher_name_time.getUTC();}}
   static Str  NintendoPublisherName       (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.nintendo_publisher_name; return S;}
   static void NintendoLegalInfo           (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.nintendo_legal_info=text; app_data.nintendo_legal_info_time.getUTC();}}
   static Str  NintendoLegalInfo           (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.nintendo_legal_info; return S;}
   static void AndroidLicenseKey           (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.android_license_key=text; app_data.android_license_key_time.getUTC();}}
   static Str  AndroidLicenseKey           (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.android_license_key; return S;}
   static void PlayAssetDelivery           (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.playAssetDelivery(TextBool(text)); app_data.play_asset_delivery_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  PlayAssetDelivery           (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.playAssetDelivery(); return S;}
   static void Build                       (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.build=TextInt(text); app_data.build_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  Build                       (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.build; return S;}
   static void SaveSize                    (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){CalcValue cv; TextValue(text, cv, false); int v=(cv.type ? cv.asInt() : -1); app_data.save_size=((v>=0) ? v*1024*1024 : -1); app_data.save_size_time.getUTC();}}
   static Str  SaveSize                    (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())if(app_data.save_size>=0)return DivRound(app_data.save_size, 1024*1024); return S;}
   static void LocationUsageReason         (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.location_usage_reason=text; app_data.location_usage_reason_time.getUTC();}}
   static Str  LocationUsageReason         (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.location_usage_reason; return S;}
   static void FacebookAppID               (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.fb_app_id=TextULong(text); app_data.fb_app_id_time.getUTC();}}
   static Str  FacebookAppID               (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())if(app_data.fb_app_id)return app_data.fb_app_id; return S;}
   static void AdMobAppIDiOS               (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.am_app_id_ios=text; app_data.am_app_id_ios_time.getUTC();}}
   static Str  AdMobAppIDiOS               (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.am_app_id_ios; return S;}
   static void AdMobAppIDGoogle            (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.am_app_id_google=text; app_data.am_app_id_google_time.getUTC();}}
   static Str  AdMobAppIDGoogle            (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.am_app_id_google; return S;}
   static void ChartboostAppIDiOS          (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.cb_app_id_ios=text; app_data.cb_app_id_ios_time.getUTC();}}
   static Str  ChartboostAppIDiOS          (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.cb_app_id_ios; return S;}
   static void ChartboostAppSignatureiOS   (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.cb_app_signature_ios=text; app_data.cb_app_signature_ios_time.getUTC();}}
   static Str  ChartboostAppSignatureiOS   (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.cb_app_signature_ios; return S;}
   static void ChartboostAppIDGoogle       (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.cb_app_id_google=text; app_data.cb_app_id_google_time.getUTC();}}
   static Str  ChartboostAppIDGoogle       (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.cb_app_id_google; return S;}
   static void ChartboostAppSignatureGoogle(  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.cb_app_signature_google=text; app_data.cb_app_signature_google_time.getUTC();}}
   static Str  ChartboostAppSignatureGoogle(C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.cb_app_signature_google; return S;}
   static void Storage                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.storage=Edit.STORAGE_MODE(TextInt(text)); app_data.storage_time.getUTC();}}
   static Str  Storage                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.storage; return S;}
   static void GuiSkin                     (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.gui_skin=Proj.findElmID(text, ELM_GUI_SKIN); app_data.gui_skin_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  GuiSkin                     (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return Proj.elmFullName(app_data.gui_skin); return S;}
   static void EmbedEngineData             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.embedEngineData(TextInt(text)).embed_engine_data_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  EmbedEngineData             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.embedEngineData(); return S;}
   static void PublishProjData             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.publishProjData(TextBool(text)).publish_proj_data_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  PublishProjData             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.publishProjData(); return S;}
   static void PublishDataAsPak            (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.publishDataAsPak(TextBool(text)).publish_data_as_pak_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  PublishDataAsPak            (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.publishDataAsPak(); return S;}
   static void PublishPhysxDll             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.publishPhysxDll(TextBool(text)).publish_physx_dll_time.getUTC(); if(ap.elm_id==Proj.curApp())CodeEdit.makeAuto();}}
   static Str  PublishPhysxDll             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.publishPhysxDll(); return S;}
   static void PublishSteamDll             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.publishSteamDll(TextBool(text)).publish_steam_dll_time.getUTC(); if(ap.elm_id==Proj.curApp()){CodeEdit.makeAuto(); CodeEdit.rebuildSymbols();}}}
   static Str  PublishSteamDll             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.publishSteamDll(); return S;}
   static void PublishOpenVRDll            (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.publishOpenVRDll(TextBool(text)).publish_open_vr_dll_time.getUTC(); if(ap.elm_id==Proj.curApp()){CodeEdit.makeAuto(); CodeEdit.rebuildSymbols();}}}
   static Str  PublishOpenVRDll            (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.publishOpenVRDll(); return S;}
 //static void WindowsCodeSign             (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.windowsCodeSign(TextBool(text)).windows_code_sign_time.getUTC();}}
 //static Str  WindowsCodeSign             (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return app_data.windowsCodeSign(); return S;}
   static void Orientation                 (  AppPropsEditor &ap, C Str &text) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData()){app_data.supported_orientations=OrientToFlag(ORIENT(TextInt(text))); app_data.supported_orientations_time.getUTC();}}
   static Str  Orientation                 (C AppPropsEditor &ap             ) {if(ap.elm)if(ElmApp *app_data=ap.elm.appData())return FlagToOrient(app_data.supported_orientations); return S;}
   static void Language                    (ptr lang_ptr                     )
   {
      if(AppPropsEdit.p_languages && AppPropsEdit.elm)if(ElmApp *app_data=AppPropsEdit.elm.appData())
      {
         LANG_TYPE lang_type=LANG_TYPE((intptr)lang_ptr);
         if(AppPropsEdit.p_languages.combobox.menu(LanguageName(lang_type)))app_data.supported_languages.include(lang_type);
         else                                                               app_data.supported_languages.exclude(lang_type);
         app_data.supported_languages_time.getUTC();
         AppPropsEdit.setLanguages();
      }
   }

   enum PLATFORM
   {
      PWIN,
      PMAC,
      PLIN,
      PAND,
      PIOS,
      PNIN,
   }
   static cchar8 *platforms_t[]=
   {
      "Windows",
      "Mac",
      "Linux",
      "Android",
      "iOS",
      "Nintendo",
   };
   static cchar8 *xbox_live_program_t[]=
   {
      "Creators"                 , // 0
      "ID@Xbox, Managed Partners", // 1
   };
   ASSERT(Edit.XBOX_LIVE_CREATORS==0 && Edit.XBOX_LIVE_ID_XBOX==1 && Edit.XBOX_LIVE_NUM==2);
   void create()
   {
      flt h=0.05;

                        add("Package Name"               , MemberDesc(DATA_STR                               ).setFunc(Package                     , Package                     )).desc("Application package name.\nMust be in following format: \"com.company_name.app_name\"\nWhere 'company_name' is the name of developer/company,\nand 'app_name' is the name of the application.\n\nThe package name should be unique.\nThe name parts may contain uppercase or lowercase letters 'A' through 'Z', numbers, hyphens '-' and underscores '_'.\n\nOnce you publish your application, you cannot change the package name.\nThe package name defines your application's identity,\nso if you change it, then it is considered to be a different application\nand users of the previous version cannot update to the new version.");
                        add("Build Number"               , MemberDesc(DATA_INT                               ).setFunc(Build                       , Build                       )).desc("Application build number.\nUsed to identify the version of the application.\nThis must be specified in order for the application to update correctly through online stores.\nTypically you should increase this value by 1 when making each new release.").min(0).mouseEditSpeed(2); // was min(1) but Nintendo requires submitting Version 0 first, 1 fails
    Property &fb_app_id=add("Facebook App ID"            , MemberDesc(MEMBER(ElmApp, fb_app_id              )).setFunc(FacebookAppID               , FacebookAppID               )).desc("Facebook Application ID").mouseEditDel();
                        add("Supported Orientations"     , MemberDesc(DATA_INT                               ).setFunc(Orientation                 , Orientation                 )).setEnum(OrientName, Elms(OrientName)).desc("Supported orientations for mobile platforms");
           p_languages=&add("Supported Languages"        , MemberDesc(DATA_INT                               )).setEnum();
   {
      Node<MenuElm> n;
      FREPA(Languages)n.New().create(LanguageName(Languages[i]), Language, ptr(Languages[i])).flag(MENU_TOGGLABLE);
      p_languages.combobox.setData(n);
   }
                        add("Default Gui Skin"           , MemberDesc(MEMBER(ElmApp, gui_skin               )).setFunc(GuiSkin                     , GuiSkin                     )).elmType(ELM_GUI_SKIN).desc("Set default Gui Skin used by this Application.\nGui Skin will be loaded during Application Engine initialization stage.");
      PropEx &embed    =add("Embed Engine Data"          , MemberDesc(DATA_INT                               ).setFunc(EmbedEngineData             , EmbedEngineData             )).setEnum().desc("If embed engine data into the application executable file, so it doesn't require separate \"Engine.pak\" file.\nThis option is recommended for applications that want to be distributed as standalone executables without any additional files.\nThis option is ignored for Mobile and Web builds.\nDefault value for this option is \"No\"."); embed.combobox.setColumns(NameDescListColumn, Elms(NameDescListColumn)).setData(EmbedEngine, Elms(EmbedEngine)).menu.list.setElmDesc(MEMBER(NameDesc, desc));
                        add("Publish Project Data"       , MemberDesc(DATA_BOOL                              ).setFunc(PublishProjData             , PublishProjData             )).desc("If include project data when publishing the application.\nDisable this if your application will not initially include the data, but will download it manually later.\nDefault value for this option is true.");
                        add("Publish Data as PAK"        , MemberDesc(DATA_BOOL                              ).setFunc(PublishDataAsPak            , PublishDataAsPak            )).desc("If archive data files into a singe PAK file.\nDisable this option if you plan to upload the application using Uploader tool, in which case it's better that the files are stored separately (instead of archived).\nThen you can make your Game/Installer to download files from the server, and optionally archive them as PAK.\nDefault value for this option is true.");
                      //add("Publish PhysX DLLs"         , MemberDesc(DATA_BOOL                              ).setFunc(PublishPhysxDll             , PublishPhysxDll             )).desc("If include PhysX DLL files when publishing the application.\nDisable this if your application doesn't use physics, or it will download the files manually later.\nThis option is used only for EXE and DLL targets.\nDefault value for this option is true.");
                        add("Publish Steam DLL"          , MemberDesc(DATA_BOOL                              ).setFunc(PublishSteamDll             , PublishSteamDll             )).desc("If include Steam DLL file when publishing the application.\nEnable this only if your application uses Steam API.\nDefault value for this option is false.\nBased on this option, \"STEAM\" C++ macro will be set to 0 or 1.");
                        add("Publish OpenVR DLL"         , MemberDesc(DATA_BOOL                              ).setFunc(PublishOpenVRDll            , PublishOpenVRDll            )).desc("If include OpenVR DLL file when publishing the application.\nEnable this only if your application uses OpenVR API.\nDefault value for this option is false.\nBased on this option, \"OPEN_VR\" C++ macro will be set to 0 or 1.");
                      //add("Windows Code Sign"          , MemberDesc(DATA_BOOL                              ).setFunc(WindowsCodeSign             , WindowsCodeSign             )).desc("If automatically sign the application when publishing for Windows EXE platform.\nWindows signtool.exe must be installed together with your Microsoft Windows Authenticode Digital Signature in the Certificate Store.\nSign tool will be used with the /a option making it to choose the best certificate out of all available.");
                p_icon=&add("Icon"             );
      p_image_portrait=&add("Portrait Image"   );
     p_image_landscape=&add("Landscape Image"  );
   p_notification_icon=&add("Notification Icon");

      win_props.add("Include Headers"    , MemberDesc(DATA_STR).setFunc(HeadersWin, HeadersWin)).desc("Type full paths to header file names.\nSeparate each with | for example:\nC:\\Lib1\\Main.h | C:\\Lib2\\Main.h");
      win_props.add("Include Libraries"  , MemberDesc(DATA_STR).setFunc(LibsWin   , LibsWin   )).desc("Type full paths to lib file names.\nSeparate each with | for example:\nC:\\Lib1\\Main.lib | C:\\Lib2\\Main.lib");
      win_props.add("Include Directories", MemberDesc(DATA_STR).setFunc(DirsWin   , DirsWin   )).desc("Type full paths to additional include directories.\nSeparate each with | for example:\nC:\\Lib1 | C:\\Lib2");
      PropEx &ms_pub_id=win_props.add("Microsoft Publisher ID"     , MemberDesc(DATA_STR                               ).setFunc(MicrosoftPublisherID        , MicrosoftPublisherID        ));
      PropEx &ms_pub_nm=win_props.add("Microsoft Publisher Name"   , MemberDesc(DATA_STR                               ).setFunc(MicrosoftPublisherName      , MicrosoftPublisherName      ));
      PropEx &xb_prog  =win_props.add("XboxLive Program"           , MemberDesc(DATA_INT                               ).setFunc(XboxLiveProgram             , XboxLiveProgram             )).setEnum(xbox_live_program_t, Elms(xbox_live_program_t));
      PropEx &xb_tit_id=win_props.add("XboxLive Title ID (decimal)", MemberDesc(DATA_STR                               ).setFunc(XboxLiveTitleID             , XboxLiveTitleID             ));
      PropEx &xb_scid  =win_props.add("XboxLive SCID"              , MemberDesc(DATA_STR                               ).setFunc(XboxLiveSCID                , XboxLiveSCID                ));

      mac_props.add("Include Headers"    , MemberDesc(DATA_STR).setFunc(HeadersMac, HeadersMac)).desc("Type full paths to header file names.\nSeparate each with | for example:\n/Lib1/Main.h | /Lib2/Main.h");
      mac_props.add("Include Libraries"  , MemberDesc(DATA_STR).setFunc(LibsMac   , LibsMac   )).desc("Type full paths to lib file names.\nSeparate each with | for example:\n/Lib1/Main.a | /Lib2/Main.a");
      mac_props.add("Include Directories", MemberDesc(DATA_STR).setFunc(DirsMac   , DirsMac   )).desc("Type full paths to additional include directories.\nSeparate each with | for example:\n/Lib2 | /Lib2");

      linux_props.add("Include Headers"    , MemberDesc(DATA_STR).setFunc(HeadersLinux, HeadersLinux)).desc("Type full paths to header file names.\nSeparate each with | for example:\n/Lib1/Main.h | /Lib2/Main.h");
      linux_props.add("Include Libraries"  , MemberDesc(DATA_STR).setFunc(LibsLinux   , LibsLinux   )).desc("Type full paths to lib file names.\nSeparate each with | for example:\n/Lib1/Main.a | /Lib2/Main.a");
      linux_props.add("Include Directories", MemberDesc(DATA_STR).setFunc(DirsLinux   , DirsLinux   )).desc("Type full paths to additional include directories.\nSeparate each with | for example:\n/Lib2 | /Lib2");

      android_props.add("Include Headers"    , MemberDesc(DATA_STR).setFunc(HeadersAndroid, HeadersAndroid)).desc("Type full paths to header file names.\nSeparate each with | for example:\nC:\\Lib1\\Main.h | C:\\Lib2\\Main.h");
      android_props.add("Include Libraries"  , MemberDesc(DATA_STR).setFunc(LibsAndroid   , LibsAndroid   )).desc("Type full paths to lib file names.\nSeparate each with | for example:\nC:\\Lib1\\XXX.a | C:\\Lib2\\libXXX.so\n\n$(TARGET_ARCH_ABI) can be used in the path, which will be replaced with target architecture (such as armeabi-v7a, arm64-v8a, x86, x86_64), for example:\nC:\\Path\\XXX-$(TARGET_ARCH_ABI).a\nC:\\Path\\$(TARGET_ARCH_ABI)\\libXXX.so");
      android_props.add("Include Directories", MemberDesc(DATA_STR).setFunc(DirsAndroid   , DirsAndroid   )).desc("Type full paths to additional include directories.\nSeparate each with | for example:\nC:\\Lib1 | C:\\Lib2");
      PropEx &am_ai_g   =android_props.add("AdMob App ID"               , MemberDesc(MEMBER(ElmApp, am_app_id_google       )).setFunc(AdMobAppIDGoogle            , AdMobAppIDGoogle            )).desc("AdMob Application ID");
      PropEx &cb_ai_g   =android_props.add("Chartboost App ID"          , MemberDesc(MEMBER(ElmApp, cb_app_id_google       )).setFunc(ChartboostAppIDGoogle       , ChartboostAppIDGoogle       )).desc("Chartboost Application ID");
      PropEx &cb_as_g   =android_props.add("Chartboost App Signature"   , MemberDesc(MEMBER(ElmApp, cb_app_signature_google)).setFunc(ChartboostAppSignatureGoogle, ChartboostAppSignatureGoogle)).desc("Chartboost Application Signature");
      PropEx &google_lk =android_props.add("License Key"                , MemberDesc(DATA_STR                               ).setFunc(AndroidLicenseKey           , AndroidLicenseKey           )).desc("Google Play app license key.\nThis key is used for verification of purchases in Google Play Store.\nYou can obtain this key from \"Google Play Developer Console website \\ Your App \\ Monetize \\ Monetization setup \\ Licensing\".\nUpon providing your license key, all purchases will be automatically verified and only those that pass the verification test will be returned.\nIf you don't specify your key then all purchases will be listed without any verification.");
      PropEx &google_pad=android_props.add("Play Asset Delivery"        , MemberDesc(DATA_BOOL                              ).setFunc(PlayAssetDelivery           , PlayAssetDelivery           ));
      PropEx &storage   =android_props.add("Preferred Storage"          , MemberDesc(DATA_INT                               ).setFunc(Storage                     , Storage                     )).setEnum().desc("Preferred installation location for the application\n\nInternal - The application must be installed on the internal device storage only. If this is set, the application will never be installed on the external storage. If the internal storage is full, then the system will not install the application.\n\nExternal - The application prefers to be installed on the external storage (SD card). There is no guarantee that the system will honor this request. The application might be installed on internal storage if the external media is unavailable or full, or if the application uses the forward-locking mechanism (not supported on external storage). Once installed, the user can move the application to either internal or external storage through the system settings.\n\nAuto - The application may be installed on the external storage, but the system will install the application on the internal storage by default. If the internal storage is full, then the system will install it on the external storage. Once installed, the user can move the application to either internal or external storage through the system settings."); storage.combobox.setColumns(NameDescListColumn, Elms(NameDescListColumn)).setData(StorageName, Elms(StorageName)).menu.list.setElmDesc(MEMBER(NameDesc, desc));

      ios_props.add("Include Headers"    , MemberDesc(DATA_STR).setFunc(HeadersiOS, HeadersiOS)).desc("Type full paths to header file names.\nSeparate each with | for example:\n/Lib1/Main.h | /Lib2/Main.h");
      ios_props.add("Include Libraries"  , MemberDesc(DATA_STR).setFunc(LibsiOS   , LibsiOS   )).desc("Type full paths to lib file names.\nSeparate each with | for example:\n/Lib1/Main.a | /Lib2/Main.a");
      ios_props.add("Include Directories", MemberDesc(DATA_STR).setFunc(DirsiOS   , DirsiOS   )).desc("Type full paths to additional include directories.\nSeparate each with | for example:\n/Lib2 | /Lib2");
      PropEx &am_ai_i  =ios_props.add("AdMob App ID"               , MemberDesc(MEMBER(ElmApp, am_app_id_ios          )).setFunc(AdMobAppIDiOS               , AdMobAppIDiOS               )).desc("AdMob Application ID");
      PropEx &cb_ai_i  =ios_props.add("Chartboost App ID"          , MemberDesc(MEMBER(ElmApp, cb_app_id_ios          )).setFunc(ChartboostAppIDiOS          , ChartboostAppIDiOS          )).desc("Chartboost Application ID");
      PropEx &cb_as_i  =ios_props.add("Chartboost App Signature"   , MemberDesc(MEMBER(ElmApp, cb_app_signature_ios   )).setFunc(ChartboostAppSignatureiOS   , ChartboostAppSignatureiOS   )).desc("Chartboost Application Signature");
      PropEx &loc_usage=ios_props.add("Location Usage Reason"      , MemberDesc(DATA_STR                               ).setFunc(LocationUsageReason         , LocationUsageReason         )).desc("Reason for accessing the user's location information.\nThis is needed for iOS (on other platforms this is ignored).\nThis will be displayed on the user screen when trying to access the Location.");

                        nintendo_props.add("Include Headers"           , MemberDesc(DATA_STR).setFunc(HeadersNintendo      , HeadersNintendo      )).desc("Type full paths to header file names.\nSeparate each with | for example:\nC:\\Lib1\\Main.h | C:\\Lib2\\Main.h");
                        nintendo_props.add("Include Libraries"         , MemberDesc(DATA_STR).setFunc(LibsNintendo         , LibsNintendo         )).desc("Type full paths to lib file names.\nSeparate each with | for example:\nC:\\Lib1\\Main.a | C:\\Lib2\\Main.a");
                        nintendo_props.add("Include Directories"       , MemberDesc(DATA_STR).setFunc(DirsNintendo         , DirsNintendo         )).desc("Type full paths to additional include directories.\nSeparate each with | for example:\nC:\\Lib1 | C:\\Lib2");
      PropEx &nn_ini_cd=nintendo_props.add("Nintendo Initial Code"     , MemberDesc(DATA_STR).setFunc(NintendoInitialCode  , NintendoInitialCode  ));
      PropEx &nn_app_id=nintendo_props.add("Nintendo App ID"           , MemberDesc(DATA_STR).setFunc(NintendoAppID        , NintendoAppID        ));
      PropEx &nn_pub_nm=nintendo_props.add("Nintendo Publisher Name"   , MemberDesc(DATA_STR).setFunc(NintendoPublisherName, NintendoPublisherName));
      PropEx &nn_legal =nintendo_props.add("Nintendo Legal Information", MemberDesc(DATA_STR).setFunc(NintendoLegalInfo    , NintendoLegalInfo    ));
                        nintendo_props.add("Max Save Disk Usage (MB)"  , MemberDesc(DATA_STR).setFunc(SaveSize             , SaveSize             )).desc("Maximum disk usage for all save files in MegaBytes");

      autoData(this); win_props.autoData(this); mac_props.autoData(this); linux_props.autoData(this); android_props.autoData(this); ios_props.autoData(this); nintendo_props.autoData(this);
      flt  vw=0.85;
      Rect rect=super.create(S, Vec2(0.02, -0.01), 0.04, h, vw); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      Vec2 pos=rect.min; pos.y-=h*3.5;
      flt  th=fb_app_id.textline.rect().h(), cw=clientWidth(), pw=cw-0.02; vw=cw;
      T+=platforms.create(platforms_t, Elms(platforms_t)).valid(true).set(PWIN).rect(Rect_LU(pos, cw-0.04, 0.05)); pos.y-=h;
      AddProperties(     win_props.props, platforms.tab(PWIN), pos, h, vw, &ts, &pw);
      AddProperties(     mac_props.props, platforms.tab(PMAC), pos, h, vw, &ts, &pw);
      AddProperties(   linux_props.props, platforms.tab(PLIN), pos, h, vw, &ts, &pw);
      AddProperties( android_props.props, platforms.tab(PAND), pos, h, vw, &ts, &pw);
      AddProperties(     ios_props.props, platforms.tab(PIOS), pos, h, vw, &ts, &pw);
      AddProperties(nintendo_props.props, platforms.tab(PNIN), pos, h, vw, &ts, &pw);
      super.changed(Changed); win_props.changed(Changed); mac_props.changed(Changed); linux_props.changed(Changed); android_props.changed(Changed); ios_props.changed(Changed); nintendo_props.changed(Changed);

      T+=fb_app_id.button.create(Rect_RU(fb_app_id.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetFacebookAppID); fb_app_id.textline.rect(Rect(fb_app_id.textline.rect().ld(), fb_app_id.button.rect().lu()));

      platforms.tab(PWIN)+=ms_pub_id.button.create(Rect_RU(ms_pub_id.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetMicrosoftPublisher); ms_pub_id.textline.rect(Rect(ms_pub_id.textline.rect().ld(), ms_pub_id.button.rect().lu()));
      platforms.tab(PWIN)+=ms_pub_nm.button.create(Rect_RU(ms_pub_nm.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetMicrosoftPublisher); ms_pub_nm.textline.rect(Rect(ms_pub_nm.textline.rect().ld(), ms_pub_nm.button.rect().lu()));
      platforms.tab(PWIN)+=xb_prog.button.create(Rect_RU(xb_prog.combobox.rect().ru()+Vec2(0, 0), th*2, th), "Get").func(GetXboxLive); xb_prog.combobox.rect(Rect(xb_prog.combobox.rect().ld(), xb_prog.button.rect().lu()));
      platforms.tab(PWIN)+=xb_tit_id.button.create(Rect_RU(xb_tit_id.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetXboxLive); xb_tit_id.textline.rect(Rect(xb_tit_id.textline.rect().ld(), xb_tit_id.button.rect().lu()));
      platforms.tab(PWIN)+=xb_scid.button.create(Rect_RU(xb_scid.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetXboxLive); xb_scid.textline.rect(Rect(xb_scid.textline.rect().ld(), xb_scid.button.rect().lu()));

      platforms.tab(PAND)+=am_ai_g.button.create(Rect_RU(am_ai_g.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetAdMobApp); am_ai_g.textline.rect(Rect(am_ai_g.textline.rect().ld(), am_ai_g.button.rect().lu()));
      platforms.tab(PAND)+=cb_ai_g.button.create(Rect_RU(cb_ai_g.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetChartboostApp); cb_ai_g.textline.rect(Rect(cb_ai_g.textline.rect().ld(), cb_ai_g.button.rect().lu()));
      platforms.tab(PAND)+=cb_as_g.button.create(Rect_RU(cb_as_g.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetChartboostApp); cb_as_g.textline.rect(Rect(cb_as_g.textline.rect().ld(), cb_as_g.button.rect().lu()));
      platforms.tab(PAND)+=google_lk.button.create(Rect_RU(google_lk.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetAndroidLicenseKey); google_lk.textline.rect(Rect(google_lk.textline.rect().ld(), google_lk.button.rect().lu()));

      platforms.tab(PIOS)+=am_ai_i.button.create(Rect_RU(am_ai_i.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetAdMobApp); am_ai_i.textline.rect(Rect(am_ai_i.textline.rect().ld(), am_ai_i.button.rect().lu()));
      platforms.tab(PIOS)+=cb_ai_i.button.create(Rect_RU(cb_ai_i.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetChartboostApp); cb_ai_i.textline.rect(Rect(cb_ai_i.textline.rect().ld(), cb_ai_i.button.rect().lu()));
      platforms.tab(PIOS)+=cb_as_i.button.create(Rect_RU(cb_as_i.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetChartboostApp); cb_as_i.textline.rect(Rect(cb_as_i.textline.rect().ld(), cb_as_i.button.rect().lu()));

      platforms.tab(PNIN)+=nn_ini_cd.button.create(Rect_RU(nn_ini_cd.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetNintendo); nn_ini_cd.textline.rect(Rect(nn_ini_cd.textline.rect().ld(), nn_ini_cd.button.rect().lu()));
      platforms.tab(PNIN)+=nn_app_id.button.create(Rect_RU(nn_app_id.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetNintendo); nn_app_id.textline.rect(Rect(nn_app_id.textline.rect().ld(), nn_app_id.button.rect().lu()));
      platforms.tab(PNIN)+=nn_pub_nm.button.create(Rect_RU(nn_pub_nm.textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetNintendo); nn_pub_nm.textline.rect(Rect(nn_pub_nm.textline.rect().ld(), nn_pub_nm.button.rect().lu()));
      platforms.tab(PNIN)+=nn_legal .button.create(Rect_RU(nn_legal .textline.rect().ru()+Vec2(th, 0), th*2, th), "Get").func(GetNintendoLegal); nn_legal .textline.rect(Rect(nn_legal .textline.rect().ld(), nn_legal .button.rect().lu()));

      p_image_portrait   .move(Vec2(rect.w()  /3, h  ));
      p_image_landscape  .move(Vec2(rect.w()*2/3, h*2));
      p_notification_icon.move(Vec2(rect.w()*2/3, h*7));
      icon             .create(MEMBER(ElmApp, icon             ), MEMBER(ElmApp,              icon_time), T, Rect_LU(p_icon             .text.rect().left()-Vec2(0, h/2), 0.3));
      image_portrait   .create(MEMBER(ElmApp, image_portrait   ), MEMBER(ElmApp,    image_portrait_time), T, Rect_LU(p_image_portrait   .text.rect().left()-Vec2(0, h/2), 0.3));
      image_landscape  .create(MEMBER(ElmApp, image_landscape  ), MEMBER(ElmApp,   image_landscape_time), T, Rect_LU(p_image_landscape  .text.rect().left()-Vec2(0, h/2), 0.3));
      notification_icon.create(MEMBER(ElmApp, notification_icon), MEMBER(ElmApp, notification_icon_time), T, Rect_LU(p_notification_icon.text.rect().left()-Vec2(0, h/2), 0.13));

      clientRect(Rect_C(0, 0, clientWidth(), -pos.y+h*9+0.02));
   }
   void setLanguages()
   {
      if(p_languages)
      {
         p_languages.combobox.text.clear();
         if(elm)if(ElmApp *app_data=elm.appData())FREPA(app_data.supported_languages)p_languages.combobox.text.space()+=CaseUp(LanguageCode(app_data.supported_languages[i]));
      }
   }
   void toGui()
   {
      super.toGui(); win_props.toGui(); mac_props.toGui(); linux_props.toGui(); android_props.toGui(); ios_props.toGui(); nintendo_props.toGui();
      icon             .setImage();
      image_portrait   .setImage();
      image_landscape  .setImage();
      notification_icon.setImage();

      setLanguages();
      if(p_languages)
      {
         ElmApp *app_data=(elm ? elm.appData() : null);
         FREPA(Languages)p_languages.combobox.menu(LanguageName(Languages[i]), app_data && app_data.supported_languages.has(Languages[i]), QUIET);
      }
   }

   virtual AppPropsEditor& hide()override {set(null); super.hide(); return T;}

   void flush()
   {
      if(elm && changed)
      {
       //if(ElmApp *data=elm.appData())data.newVer(); Server.setElmLong(elm.id); // modify just before saving/sending in case we've received data from server after edit, don't send to server as apps/codes are synchronized on demand
         if(changed_headers && elm_id==Proj.curApp())Proj.activateSources(1); // if changed headers on active app then rebuild symbols
      }
      changed=false;
      changed_headers=false;
   }
   void setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmApp *data=elm.appData())data.newVer();
      }
   }
   void set(Elm *elm)
   {
      if(elm && elm.type!=ELM_APP)elm=null;
      if(T.elm!=elm)
      {
         flush();
         T.elm   =elm;
         T.elm_id=(elm ? elm.id : UIDZero);
         toGui();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
         if(elm)setTitle(S+'"'+elm.name+"\" Settings");
      }
   }
   void activate(Elm *elm) {set(elm); if(T.elm)super.activate();}
   void toggle  (Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void elmChanged(C UID &elm_id)
   {
      if(elm && elm.id==elm_id)toGui();
      if(elm_id==Proj.curApp())CodeEdit.makeAuto();
   }
   void erasing(C UID &elm_id) {if(elm && elm.id==elm_id)set(null);}
   void drag(Memc<UID> &elms, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(obj==&icon || obj==&image_portrait || obj==&image_landscape || obj==&notification_icon)REPA(elms)if(Elm *elm=Proj.findElm(elms[i]))if(ElmImageLike(elm.type))
      {
         if(obj==&icon             )icon             .setImage(elm.id);
         if(obj==&image_portrait   )image_portrait   .setImage(elm.id);
         if(obj==&image_landscape  )image_landscape  .setImage(elm.id);
         if(obj==&notification_icon)notification_icon.setImage(elm.id);
         break;
      }
   }
   void drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(obj==&icon || obj==&image_portrait || obj==&image_landscape || obj==&notification_icon)
         Gui.msgBox(S, "Application images must be set from project elements and not from files.\nPlease drag and drop your file to the project, then select \"Disable Publishing\" so that it will not additionally increase the game data size.\nThen drag and drop the project element onto the application image slot.");
   }
}
AppPropsEditor AppPropsEdit;
/******************************************************************************/
