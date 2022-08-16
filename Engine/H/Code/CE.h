/******************************************************************************/
namespace Edit{
/******************************************************************************/
enum EXE_TYPE : Byte // Executable Type
{
   EXE_EXE  , // Windows Executable
   EXE_DLL  , // Windows Dynamically Linked Library
   EXE_LIB  , // Windows Statically  Linked Library (currently not supported, reserved for future use)
   EXE_UWP  , // Windows Universal App
   EXE_APK  , // Android Application Package
   EXE_AAB  , // Android Application Bundle
   EXE_MAC  , // Mac     Application
   EXE_IOS  , // iOS     Application
   EXE_LINUX, // Linux
   EXE_NS   , // Nintendo Switch
   EXE_WEB  , // Web     Html/JavaScript
   EXE_NUM  ,
};
enum EXPORT_MODE : Byte // Export Mode
{
   EXPORT_EXE           , // Export to project according to current Executable Type
   EXPORT_TXT           , // Export to text files
   EXPORT_CPP           , // Export to C++  files
   EXPORT_VS            , // Export to Visual Studio version that is currently selected in the Editor
   EXPORT_VS2015        , // Export to Visual Studio 2015 project
   EXPORT_VS2017        , // Export to Visual Studio 2017 project
   EXPORT_VS2019        , // Export to Visual Studio 2019 project
   EXPORT_VS2022        , // Export to Visual Studio 2022 project
   EXPORT_ANDROID       , // Export to Android            project
   EXPORT_XCODE         , // Export to Xcode              project
   EXPORT_LINUX_MAKE    , // Export to Linux              project to be compiled using 'make'
   EXPORT_LINUX_NETBEANS, // Export to Linux              project to be compiled using NetBeans
};
enum BUILD_MODE : Byte
{
   BUILD_EXPORT , // activated through "Build/Export"
   BUILD_IDE    , // activated through "Build/OpenInIDE"
   BUILD_PLAY   , // activated through "Play"                (Debug/Release)
   BUILD_DEBUG  , // activated through "Build/Debug"         (Debug/Release)
   BUILD_BUILD  , // activated through "Build/Build,Rebuild" (Debug/Release)
   BUILD_PUBLISH, // activated through "Build/Publish"       (      Release)
};
enum STORAGE_MODE : Byte
{
   STORAGE_INTERNAL,
   STORAGE_EXTERNAL,
   STORAGE_AUTO    ,
};
enum XBOX_LIVE : Byte // https://docs.microsoft.com/en-us/gaming/xbox-live/get-started/join-dev-program/live-dev-program-overview
{
   XBOX_LIVE_CREATORS,
   XBOX_LIVE_ID_XBOX , // ID@Xbox, Managed Partners
   XBOX_LIVE_NUM     ,
};
struct CodeEditorInterface
{
   Int android_asset_packs=-1; // -1=not using asset packs

   void del        ();
   void clear      ();
   void create     (GuiObj &parent, Bool menu_on_top);
   void update     (Bool active);
   void draw       ();
   void resize     ();
   void skinChanged();
   Bool initialized();
   Str  title      ();
   Str  appPath    (C Str &app_name);
   Str   androidPath();
   Str   androidProjectPakPath();
   Str       iOSProjectPakPath();
   Str       UWPProjectPakPath();
   Str  nintendoProjectPakPath();
   void saveChanges();
   void saveChanges(Memc<Edit::SaveChanges::Elm> &elms);
   void saveChanges(Memc<Edit::SaveChanges::Elm> &elms, void (*after_save_close)(Bool all_saved, Ptr user), Ptr user=null);
   void sourceRename     (C UID &id  ); // call this when source code file was renamed in the project
   Bool sourceCur        (C Str &name); // set current source
   Bool sourceCur        (C UID &id  ); // set current source
   Bool sourceCurIs      (           ); // if there is a current source
   Str  sourceCurName    (           ); // get current source
   UID  sourceCurId      (           ); // get current source
   Bool sourceCurModified(           ); // get current source modified state
   Bool sourceCurConst   (           ); // get current source const    state
   void sourceDrawPreview(C Str &name); // draw preview of source
   void sourceDrawPreview(C UID &id  ); // draw preview of source
   void sourceAuto       (C Str &data); // set data of the automatically generated source
   void sourceRemove     (C UID &id  ); // remove selected source
   void sourceOverwrite  (C UID &id  ); // overwrite any changes of selected source
   Bool sourceDataGet    (C UID &id,   Str &data); // get source data, false if source is not loaded
   Bool sourceDataSet    (C UID &id, C Str &data); // set source data, false if source is not loaded
   void projectsBuildPath(C Str &path); // set build path of all projects
   void menuEnabled         (Bool on);
   void kbSet               ();
   void close               ();
   void closeAll            ();
   void  hideAll            ();
   Bool visibleOptions      ();
   void visibleOptions      (Bool on);
   Bool visibleOpenedFiles  ();
   void visibleOpenedFiles  (Bool on);
   Bool visibleOutput       ();
   void visibleOutput       (Bool on);
   Bool visibleAndroidDevLog();
   void visibleAndroidDevLog(Bool on);

   Bool importPaths       ()C;
   Bool importImageMipMaps()C;

   void saveSettings(  TextNode &code);
   void loadSettings(C TextNode &code);

   void clearActiveSources(); // set all sources to be inactive (this must be called when changing app)
   void activateSource(C UID &id   ); // make project based source active
   void activateSource(C Str &name ); // make file    based source active
   void activateApp   (Bool rebuild); // finalize changing app

   void paste(C CMemPtr<UID> &elms, GuiObj *obj, C Vec2 &screen_pos); // paste project elements at specified position
   void paste(C Str &text         , GuiObj *obj, C Vec2 &screen_pos); // paste text             at specified position
   void paste(C Str &text                                          ); // paste text             at current   position

 C Memx<Item>& items(); // get Engine items

   void        play();
   void       debug();
   void runToCursor();
   void     publish();
   void       build();
   void     rebuild();
   void     rebuildSymbols();
   void       clean();
   void    cleanAll();
   void        stop();
   void     openIDE();
   void exportPaths(Bool relative);
   Bool      Export(EXPORT_MODE mode);

   void     configDebug(Bool     debug);
   Bool     configDebug(              );
   void     config32Bit(Bool     bit32);
   Bool     config32Bit(              );
   void     configAPI  (Byte     api  );
   Byte     configAPI  (              );
   void     configEXE  (EXE_TYPE exe  );
   EXE_TYPE configEXE  (              );

   virtual void visibleChangedOptions      () {}
   virtual void visibleChangedOpenedFiles  () {}
   virtual void visibleChangedOutput       () {}
   virtual void visibleChangedAndroidDevLog() {}

   virtual void configChangedDebug() {}
   virtual void configChanged32Bit() {}
   virtual void configChangedAPI  () {}
   virtual void configChangedEXE  () {}

   virtual UID projectID() {return UIDZero;} // get active Project ID

   virtual UID           appID                              () {return            UIDZero;} // get ID                                  of current app
   virtual Str           appName                            () {return                  S;} // get name                                of current app
   virtual Str           appDirsWindows                     () {return                  S;} // get all directories                     of current app
   virtual Str           appDirsMac                         () {return                  S;} // get all directories                     of current app
   virtual Str           appDirsLinux                       () {return                  S;} // get all directories                     of current app
   virtual Str           appDirsAndroid                     () {return                  S;} // get all directories                     of current app
   virtual Str           appDirsiOS                         () {return                  S;} // get all directories                     of current app
   virtual Str           appDirsNintendo                    () {return                  S;} // get all directories                     of current app
   virtual Str           appDirsWeb                         () {return                  S;} // get all directories                     of current app
   virtual Str           appHeadersWindows                  () {return                  S;} // get all headers                         of current app
   virtual Str           appHeadersMac                      () {return                  S;} // get all headers                         of current app
   virtual Str           appHeadersLinux                    () {return                  S;} // get all headers                         of current app
   virtual Str           appHeadersAndroid                  () {return                  S;} // get all headers                         of current app
   virtual Str           appHeadersiOS                      () {return                  S;} // get all headers                         of current app
   virtual Str           appHeadersNintendo                 () {return                  S;} // get all headers                         of current app
   virtual Str           appHeadersWeb                      () {return                  S;} // get all headers                         of current app
   virtual Str           appLibsWindows                     () {return                  S;} // get all libraries                       of current app
   virtual Str           appLibsMac                         () {return                  S;} // get all libraries                       of current app
   virtual Str           appLibsLinux                       () {return                  S;} // get all libraries                       of current app
   virtual Str           appLibsAndroid                     () {return                  S;} // get all libraries                       of current app
   virtual Str           appLibsiOS                         () {return                  S;} // get all libraries                       of current app
   virtual Str           appLibsNintendo                    () {return                  S;} // get all libraries                       of current app
   virtual Str           appLibsWeb                         () {return                  S;} // get all libraries                       of current app
   virtual Str           appPackage                         () {return                  S;} // get package name                        of current app
   virtual UID           appMicrosoftPublisherID            () {return            UIDZero;} // get Microsoft Publisher ID              of current app
   virtual Str           appMicrosoftPublisherName          () {return                  S;} // get Microsoft Publisher Name            of current app
   virtual XBOX_LIVE     appXboxLiveProgram                 () {return XBOX_LIVE_CREATORS;} // get Xbox Live program                   of current app
   virtual ULong         appXboxLiveTitleID                 () {return                  0;} // get Xbox Live Title ID                  of current app
   virtual UID           appXboxLiveSCID                    () {return            UIDZero;} // get Xbox Live SCID                      of current app
   virtual Bool          appWindowsCodeSign                 () {return              false;} // get if code sign exe                    of current app
   virtual Str           appGooglePlayLicenseKey            () {return                  S;} // get Google Play license key             of current app
   virtual Bool          appGooglePlayAssetDelivery         () {return              false;} // get Google Play Asset Delivery          of current app
   virtual Str           appLocationUsageReason             () {return                  S;} // get location usage reason               of current app
   virtual Str           appNintendoInitialCode             () {return                  S;} // get Nintendo Initial Code               of current app
   virtual ULong         appNintendoAppID                   () {return                  0;} // get Nintendo App ID                     of current app
   virtual Str           appNintendoPublisherName           () {return                  S;} // get Nintendo Publisher Name             of current app
   virtual Str           appNintendoLegalInformation        () {return                  S;} // get Nintendo Legal Information          of current app
   virtual Int           appBuild                           () {return                  1;} // get build number                        of current app
   virtual ULong         appFacebookAppID                   () {return                  0;} // get Facebook   App ID                   of current app
   virtual Str           appAdMobAppIDiOS                   () {return                  S;} // get AdMob      App ID                   of current app for iOS         platform
   virtual Str           appAdMobAppIDGooglePlay            () {return                  S;} // get AdMob      App ID                   of current app for Google Play platform
   virtual Str           appChartboostAppIDiOS              () {return                  S;} // get Chartboost App ID                   of current app for iOS         platform
   virtual Str           appChartboostAppSignatureiOS       () {return                  S;} // get Chartboost App Signature            of current app for iOS         platform
   virtual Str           appChartboostAppIDGooglePlay       () {return                  S;} // get Chartboost App ID                   of current app for Google Play platform
   virtual Str           appChartboostAppSignatureGooglePlay() {return                  S;} // get Chartboost App Signature            of current app for Google Play platform
   virtual STORAGE_MODE  appPreferredStorage                () {return   STORAGE_INTERNAL;} // get preferred storage                   of current app
   virtual UInt          appSupportedOrientations           () {return                  0;} // get supported orientations (DIR_FLAG)   of current app
   virtual UID           appGuiSkin                         () {return            UIDZero;} // get default Gui Skin                    of current app
   virtual ImagePtr      appIcon                            () {return               null;} // get icon                                of current app
   virtual ImagePtr      appImagePortrait                   () {return               null;} // get portrait  splash screen             of current app
   virtual ImagePtr      appImageLandscape                  () {return               null;} // get landscape splash screen             of current app
   virtual ImagePtr      appNotificationIcon                () {return               null;} // get notification icon                   of current app
   virtual Int           appEmbedEngineData                 () {return                  0;} // get if embed   engine  data             of current app
   virtual Cipher*       appEmbedCipher                     () {return               null;} // get cipher      used for embedding data of current app
   virtual Bool          appPublishProjData                 () {return               true;} // get if publish project data             of current app
   virtual Bool          appPublishPhysxDll                 () {return               true;} // get if copy PhysX  dll's                of current app
   virtual Bool          appPublishSteamDll                 () {return              false;} // get if copy Steam  dll                  of current app
   virtual Bool          appPublishOpenVRDll                () {return              false;} // get if copy OpenVR dll                  of current app
   virtual Bool          appPublishDataAsPak                () {return               true;} // get if publish data as paks             of current app
   virtual Long          appSaveSize                        () {return                 -1;} // get max save disk usage                 of current app
   virtual void          appInvalidProperty                 (C Str &msg                                  ) {                         } // called when application property was detected as invalid
   virtual COMPRESS_TYPE appEmbedCompress                   (                           EXE_TYPE exe_type) {return     COMPRESS_NONE;} // get compression used for embedding data of current app
   virtual Int           appEmbedCompressLevel              (                           EXE_TYPE exe_type) {return                 0;} // get compression used for embedding data of current app
   virtual DateTime      appEmbedSettingsTime               (                           EXE_TYPE exe_type) {return DateTime().zero();} // get settings time    for embedding data of current app
   virtual void          appSpecificFiles                   (MemPtr<PakFileData> files, EXE_TYPE exe_type) {                         } // get specific files      of current app
   virtual void          appLanguages                       (MemPtr<LANG_TYPE  > langs                   ) {langs.clear();           } // get supported languages of current app

   virtual Rect         menuRect    (                      ) {return D.rect();}
   virtual Rect       sourceRect    (                      ) {return D.rect();}
   virtual Str        sourceProjPath(C UID &id             ) {return S;}
   virtual ERROR_TYPE sourceDataLoad(C UID &id,   Str &data) {return EE_ERR_ELM_NOT_FOUND;}
   virtual Bool       sourceDataSave(C UID &id, C Str &data) {return false;}
   virtual void       sourceChanged (Bool activate=false) {} // called on change of current source

   virtual void publishSuccess(C Str &exe_name, EXE_TYPE exe_type, BUILD_MODE build_mode, C UID &project_id) {}

   virtual void validateActiveSources() {} // called when sources need to have their activation reset

   virtual Int  editorAddrPort(              ) {return     0;} // get Editor Network Interface Address Port
   virtual void editorAddr    (SockAddr &addr) {addr.clear();} // get Editor Network Interface Address

   virtual Bool elmValid    (C UID &id              ) {return false;} // if element of this ID exists in the project
   virtual Str  elmBaseName (C UID &id              ) {return     S;} // get base name of element
   virtual Str  elmFullName (C UID &id              ) {return     S;} // get full name of element
   virtual void elmHighlight(C UID &id, C Str  &name) {             } // highlight element
   virtual void elmOpen     (C UID &id              ) {             } // open      element
   virtual void elmLocate   (C UID &id              ) {             } // locate    element
   virtual void elmPreview  (C UID &id, C Vec2 &pos, Bool mouse, C Rect &clip) {} // draw element preview at 'pos' position, 'mouse'=if triggered by mouse

   virtual void focus() {}

   virtual Str idToText(C UID &id, Bool *valid=null) {if(valid)*valid=false; return S;} // return ID in text format (this can be element/object/waypoint/.. ID)

   struct ElmLink
   {
      UID      id;
      Str      full_name;
      ImagePtr icon;

      void set(C UID &id, C Str &full_name, C ImagePtr &icon) {T.id=id; T.full_name=full_name; T.icon=icon;}
   };
   virtual void getProjPublishElms(Memc<ElmLink> &elms) {} // get list of project elements for publishing
};
#if EE_PRIVATE
/******************************************************************************/
struct VisualStudioInstallation
{
   Str   name, path;
   VecI4 ver; // 9="VS 2008", 10="VS 2010", 11="VS 2012", 12="VS 2013", 14="VS 2015", 15="VS 2017", 16="VS 2019", ..
};
Bool CheckVisualStudio(C VecI4 &vs_ver, Str *message=null, Bool check_minor=true); // check if specified VS version is supported, 'message'=optional message, false on fail
Bool GetVisualStudioInstallations(MemPtr<VisualStudioInstallation> installs); // false on fail
Str  MSBuildPath(C Str &vs_path=S, C VecI4 &vs_ver=0); // 'vs_path'=preferred Visual Studio installation path, use empty for auto-detect

Str MSBuildParams(C Str &project, C Str &config, C Str &platform=S);
Str VSBuildParams(C Str &project, C Str &config, C Str &platform=S, C Str &log=S);

Str XcodeBuildParams     (C Str &project, C Str &config, C Str &platform, C Str &sdk=S);
Str XcodeBuildCleanParams(C Str &project, C Str &config, C Str &platform, C Str &sdk=S);

Str LinuxBuildParams(C Str &project, C Str &config, Int build_threads=Cpu.threads());
/******************************************************************************/
struct ColorTheme
{
   Color colors[TOKEN_TYPES];

   void save(  TextNode &node)C;
   void load(C TextNode &node);
}extern
   Theme, ThemeCustom;
/******************************************************************************/
const_mem_addr struct CodeEditor
{
   struct CodeTab : Tab
   {
      Source *source=null;
   };
   struct CodeTabs : Tabs
   {
      CodeTab& tab(  Int  i   ) {return (CodeTab&)super::tab(i   );}
      CodeTab& New(C Str &text) {return (CodeTab&)super::New(text);}

      CodeTabs() {replaceClass<CodeTab>();}
   };

   GuiObj     * parent;
   Source     *_cur;
   Memx<Item>   items;
   Memx<Source> sources;
   Button       b_close;
   WindowIO     load_source;
   CodeTabs     code_tabs;
   Bool         config_debug, config_32_bit, symbols_loaded, menu_on_top;
   Byte         config_api;
   EXE_TYPE     config_exe;
   Str          engine_h_path;
   TextStyle    ts, ts_small, ts_cjk;
   GuiSkin      source_skin, suggestions_skin, find_result_skin, cjk_skin;

   struct BuildResult
   {
      Byte      mode; // 0=default, 1=warning, 2=error
      Str       text;
      UID       line;
      Color     color;
      SourceLoc source_loc;

      void clear() {mode=0; text.clear(); source_loc.clear(); line.zero(); setColor();}

      BuildResult& set(C Str &text                ) {clear(); T.text=text; return T;}
      BuildResult& set(C Str &text, Source *source) {clear(); T.text=text; if(source){T.source_loc=      source->loc;                 } return T;}
      BuildResult& set(C Str &text, Line   *line  ) {clear(); T.text=text; if(line  ){T.source_loc=line->source->loc; T.line=line->id;} return T;}
      BuildResult& set(C Str &text, Token  *token ) {return set(text, token  ? token ->line       : null);}
      BuildResult& set(C Str &text, Symbol *symbol) {return set(text, symbol ? symbol->getToken() : null);}

      void setWarning();
      void setError  ();
      void setColor  ();

      Bool jumpTo(); // jump to source and line, false on fail

      BuildResult() {mode=0; line.zero(); setColor();}
   };

   struct BuildFile
   {
      enum MODE : Byte
      {
         SOURCE, // .cpp .h
         TEXT  , // .txt
         LIB   , // .lib
         PAK   , // .pak
         COPY  , // .*
         IGNORE, //
      };
      MODE      mode;
      Str       src_proj_path, dest_file_path, dest_proj_path, ext_not, filter;
      SourceLoc src_loc;
      ULong     xcode_file_id, xcode_mac_id, xcode_ios_id;

      Bool includeInProj() {return mode==SOURCE || mode==LIB || mode==TEXT || mode==COPY;}

      BuildFile& set(MODE mode, C Str &src_proj_path, C SourceLoc &src_loc)
      {
         T.mode         =mode;
         T.src_proj_path=src_proj_path;
         T.src_loc      =src_loc;
         return T;
      }
      void adjustPaths(C Str &path)
      {
         switch(mode)
         {
            case SOURCE:
            {
               dest_proj_path=S+"Source\\"+ext_not+".cpp";
               dest_file_path=path+dest_proj_path;
            }break;
         }
      }

      BuildFile() {mode=IGNORE; xcode_file_id=xcode_mac_id=xcode_ios_id=0;}
   };

   struct BuildList : List<BuildResult>
   {
      Int highlight_line;
      Dbl highlight_time;

      virtual void update(C GuiPC &gpc);
      virtual void draw  (C GuiPC &gpc);

      BuildList() {highlight_line=0; highlight_time=0;}
   };
   struct DeviceLog
   {
      enum MODE : Byte
      {
         DEFAULT,
         WARNING,
         ERROR  ,
         APP    ,
      }mode;
      Color color;
      Str   time, app, message;

      Str asText();

      void setWarning();
      void setError  ();
      void setApp    ();
      void setColor  ();

      DeviceLog() {mode=DEFAULT; setColor();}
   };
   struct BuildEmbed
   {
      UInt type;
      Str  path;

      BuildEmbed& set(UInt type, C Str &path) {T.type=type; T.path=path; return T;}
   };

   BUILD_MODE        build_mode;
   EXE_TYPE          build_exe_type;
   Memc<BuildFile  > build_files;
   Memc<BuildResult> build_data;
   Memc<BuildEmbed > build_embed;
   BuildList         build_list;
   Button            build_close, build_export, build_copy;
   Region            build_region;
   Progress          build_progress;
   ConsoleProcess    build_process;
   Bool              build_debug, build_windows_code_sign;
   Int               build_phase, build_phases, build_step, build_steps;
   Flt               build_refresh;
   UID               build_project_id;
   Str               build_path, build_project_name, build_project_file, build_source, build_exe, build_package, build_output, build_log, adb_path;
   Cache<LineMap>    build_line_maps;
   WindowIO          build_io;
   Str               devlog_text;
   Memc<DeviceLog>   devlog_data;
   List<DeviceLog>   devlog_list;
   Region            devlog_region;
   ConsoleProcess    devlog_process;
   Button            devlog_close, devlog_export, devlog_filter, devlog_clear;
   WindowIO          devlog_io;
   MemberDesc        devlog_time_sort;

   Str   projects_build_path, vs_path, devenv_path, android_sdk, android_ndk, jdk_path, netbeans_path, android_cert_file, android_cert_pass, apple_team_id;
   VecI4 devenv_version;
   Bool  devenv_express, devenv_com, build_msbuild;

   struct GotoLineWindow : ClosableWindow
   {
      TextLine    textline;
      CodeEditor *ce;

              void    create  (CodeEditor &ce);
      virtual GuiObj& activate();
      virtual void    update  (C GuiPC &gpc);
      GotoLineWindow() {ce=null;}
   };
   GotoLineWindow goto_line_window;

   struct Options : ClosableWindow
   {
      struct VSVersions : ClosableWindow
      {
         CodeEditor *ce;

         VSVersions() {ce=null;}

         struct Version : Button
         {
            static void OK(Version &ver)
            {
               Str msg; if(!CheckVisualStudio(ver.install.ver, &msg))Error(msg);else
               if(ver.ce){ver.ce->setVSPath(ver.install.path); ver.ce->options.vs_versions.hide();}
            }

            VisualStudioInstallation install;
            CodeEditor              *ce;

            Version() {ce=null;}

            Button& create(C Vec2 &pos, C VisualStudioInstallation &install, CodeEditor *ce)
            {
               T.install=install;
               T.ce     =ce;
               super::create(Rect_C(pos, 0.7f, 0.06f), install.name).func(OK, T).desc(S+"Path: "+install.path+"\nVersion: "+TextVer(install.ver));
               return T;
            }
         };
         Memx<Version> versions; // must be Memx

                 void    create(CodeEditor &ce);
         virtual Window& show  ();
      };

      struct FontSize
      {
         Int size;
         Str text;

         void set(Int size, C Str &text) {T.size=size; T.text=text;}
      };

      struct FontData
      {
         Int  size;
         Vec2 spacing;
         Str  font, custom_chars;
         Memc<LANG_TYPE> langs;

         FontData();

         Str  chars()C;
         Bool make (Font &font, C Str *chars=null)C;

         void save(  TextNode &node)C;
         void load(C TextNode &node);
      };
      struct FontParams : FontData
      {
         Str  sample_text;
         Bool chinese, japanese, korean;

         FontParams();

         void load(C TextNode &node);
      };

      struct ColorThemeEditor : ClosableWindow
      {
         TextStyle      ts;
         Memx<Property> props;

         ColorThemeEditor& create();
         void skinChanged();
      };

      struct FontEditor : ClosableWindow
      {
         TextStyle      ts;
         FontParams     params;
         Memx<Property> props;
         Button         apply;
         Font           font;
         Viewport       viewport;
         Thread         thread;
         SyncLock       lock;

        ~FontEditor() {del();} // delete thread before other members

         Bool make   ();
         void preview();
         void skinChanged();

         virtual FontEditor& del   ();
         virtual FontEditor& create();

         virtual FontEditor& hide();
         virtual FontEditor& show();
      };

      TextStyle        ts;
      Tabs             tabs;
      Text           t_vs_path, t_netbeans_path, t_android_sdk, t_android_ndk, t_jdk_path, t_android_cert_file, t_android_cert_pass  , t_apple_team_id, t_font_size, t_color_theme, t_export_path_mode, t_import_path_mode;
      TextLine         vs_path,   netbeans_path,   android_sdk,   android_ndk,   jdk_path,   android_cert_file,   android_cert_pass  ,   apple_team_id;
      Button         b_vs_path, b_netbeans_path, b_android_sdk, b_android_ndk, b_jdk_path, b_android_cert_file,   android_cert_create, b_apple_team_id, facebook_android_key_hash, authenticode, vs_path_auto, netbeans_path_auto, android_sdk_auto, android_ndk_auto, jdk_path_auto, d_vs, d_netbeans, d_android_sdk, d_android_ndk, d_jdk, color_theme_edit, font_edit;
      ComboBox         font_size, color_theme, export_path_mode, import_path_mode;
      Button           ac_on_enter, simple, imm_scroll, eol_clip, line_numbers, hide_horizontal_slidebar, auto_hide_menu, show_file_tabs, import_image_mip_maps;
      WindowIO       w_vs_path, w_netbeans_path, w_android_sdk, w_android_ndk, w_jdk_path, w_android_cert_file;
      VSVersions       vs_versions;
      ColorThemeEditor color_theme_editor;
      FontEditor       font_editor;
      CodeEditor      *ce;
      Memc<FontSize>   font_sizes;

              Bool    guided       ()C {return !simple();}
              void    activatePaths();
              void    activateCert ();
              void    create       (CodeEditor &ce);
              void    skinChanged  ();
      virtual Window& show         ();
      virtual Window& hide         ();

      Options() {ce=null;}
   };
   Options options;

   struct AndroidCertificate : ClosableWindow
   {
      Text    torg_name, tpass;
      TextLine org_name,  pass;
      Button   save;
      WindowIO win_io;

              void    create(CodeEditor &ce);
      virtual Window& hide  ();
   };
   AndroidCertificate android_certificate;

   struct CurPos
   {
      Int     col;
      UID     line;
      Source *source;

      Bool operator==(C CurPos &cp) {return col==cp.col && line==cp.line && source==cp.source;}
      Bool operator!=(C CurPos &cp) {return !(T==cp);}

      CurPos() {}
      CurPos(Int col, C UID &line, Source *source) {T.col=col; T.line=line; T.source=source;}
   };
   Memc<CurPos> curpos;
   Int          curposi;

   Find        find;
   ReplaceText replace;

   Bool     view_elm_names, view_comments, view_funcs, view_func_bodies, view_private_members;
   Button   view_mode, b_find;
   ComboBox view_what;

   SaveChanges save_changes;
   
   struct MenuBarEx : MenuBar
   {
              C Rect&    rect                   ()C {return GuiObj::rect();}
      virtual MenuBarEx& rect                   (C Rect &rect);
      virtual void       parentClientRectChanged(C Rect *old_client, C Rect *new_client);
   };
   MenuBarEx menu;

   CodeEditorInterface *cei_ptr, cei_temp; // 'cei_ptr' must always point to something
   CodeEditorInterface& cei() {return *cei_ptr;}   void cei(CodeEditorInterface &cei) {cei_ptr=&cei;} // get/set CEI

  ~CodeEditor() {_cur=null;}
   CodeEditor();

   // get
   Rect      sourceRect();
   Flt   lineNumberSize()C;
   Flt  fontSpaceOffset()C;

   Source* findSource(C SourceLoc &loc);
   Source*  getSource(C SourceLoc &loc, ERROR_TYPE *error=null);

   Int     curI();
   Source* cur ();

   Str title();

   Str zipalignPath()C;
   Str      adbPath()C;
   Str ndkBuildPath()C;

   // main
   void del        ();
   void replacePath(C Str &src, C Str &dest);
   void setMenu    (Node<MenuElm> &menu);
   void create     (GuiObj *parent, Bool menu_on_top);
   void init       ();
   void update     (Bool active);
   void draw       ();

   void rebuild3rdPartyHeaders();
   void rebuildSymbols        (Bool rebuild_3rd_party_headers);
   void validateActiveSources (Bool rebuild_3rd_party_headers=false);

   // parser
   void parseHeaderEx(Str h, C Str &parent_path, Bool try_system, Memc<Token> &dest, Memc<Macro> &macros, Memc<SymbolPtr> &usings, Bool ee, Memc<PrepCond> &prep_if);
   void parseHeader  (C Str &h, Memc<Macro> &macros, Memc<SymbolPtr> &usings, Bool ee);

   // gui
   void           hideAll   ();
   void           resize    ();
   void          skinChanged();
   void           zoom      (Int zoom);
   void          fontChanged();
   void         themeChanged();
   void        scrollChanged();
   void  hideSlideBarChanged();
   void setMenuBarVisibility();
   void         buildClear  ();
   BuildResult& buildNew    ();
   void         buildNew    (Memc<Message> &msgs);
   void         buildUpdate (Bool show=true);
   Bool visibleOpenedFiles  (       );
   void visibleOpenedFiles  (Bool on);
   Bool visibleOutput       (       );
   void visibleOutput       (Bool on);
   Bool visibleAndroidDevLog(       );
   void visibleAndroidDevLog(Bool on);
   void focus() {cei().focus();}

   static void HideAndFocusCE(GuiObj &go);

   // edit
   void undo      (                                    ) {if(cur())cur()->undo     ();}
   void redo      (                                    ) {if(cur())cur()->redo     ();}
   void cut       (                                    ) {if(cur())cur()->cut      ();}
   void copy      (                                    ) {if(cur())cur()->copy     ();}
   void paste     (C Str *text=null, Bool move_cur=true) {if(cur())cur()->paste    (text, move_cur);}
   void separator (                                    ) {if(cur())cur()->separator();}
   void selectAll (                                    ) {if(cur())cur()->selAll   ();}
   void selectWord(                                    ) {if(cur())cur()->selWord  ();}
   void makeCase  (Bool upper                          ) {if(cur())cur()->makeCase (upper);}

   // C++
   void VS     (C Str &command,   Bool console,   Bool hidden              );
   void VSClean(C Str &project                                             );
   void VSBuild(C Str &project, C Str &config , C Str &platform, C Str &out);
   void VSRun  (C Str &project                                 , C Str &out);
   void VSOpen (C Str &project, C Str &file=S                              );
   void VSDebug(C Str &exe                                                 );

   void validateDevEnv     ();
   void setVSPath          (C Str &path);
   void setNetBeansPath    (C Str &path);
   void setAndroidSDKPath  (C Str &path);
   void setAndroidNDKPath  (C Str &path);
   void setJDKPath         (C Str &path);
   void setAndroidCertPath (C Str &path);
   void setAndroidCertPass (C Str &pass);
   void setAppleTeamID     (C Str &id  );
   Bool verifyVS           ();
   Bool verifyXcode        ();
   Bool verifyAndroid      ();
   Bool verifyLinuxMake    ();
   Bool verifyLinuxNetBeans();

   Bool         getBuildPath (Str &build_path, Str &build_project_name, C Str *app_name=null); // 'app_name'=if use custom app or current one
   Bool      verifyBuildPath ();
   Bool      verifyBuildFiles(Memc<Message> &msgs);
   Bool      adjustBuildFiles();
   Bool           generateTXT(Memc<Message> &msgs);
   void      generateHeadersH(Memc<Symbol*> &sorted_classes, EXPORT_MODE export_mode);
   Bool          generateCPPH(Memc<Symbol*> &sorted_classes, EXPORT_MODE export_mode);
   Bool        generateVSProj(Int version);
   Bool     generateXcodeProj();
   Bool   generateAndroidProj();
   Bool generateLinuxMakeProj();
   Bool generateLinuxNBProj  ();
   Bool                Export(EXPORT_MODE mode, BUILD_MODE build_mode);
   void             stopBuild();
   void             killBuild();
   void                 build(BUILD_MODE mode=BUILD_BUILD);
   void                  play();
   void                 debug();
   void               openIDE();
   void           runToCursor();
   void                 clean();
   void              cleanAll();
   void               rebuild();

   void configDebug(Bool     debug);
   void config32Bit(Bool     bit32);
   void configAPI  (Byte     api  );
   void configEXE  (EXE_TYPE exe  );

   void createFuncList    ();
   void playEngineCompiler();

   // sources
   void    cur                 (Source *cur);
   Source& New                 ();
   void    sourceRemoved       (Source &src);
   void    closeDo             (Source &src);
   void    closeAll            (); // close all sources ignoring any unsaved changes
   void    close               (Source *src);
   void    close               () {close(cur());}
   void    saveChanges         ();
   void    saveChanges         (Memc<Edit::SaveChanges::Elm> &elms);
   void    removeUselessSources();

   CurPos   curPos();
   void markCurPos();
   void prevCurPos();
   void nextCurPos();

   void nextFile(); // activate next opened source
   void prevFile(); // activate prev opened source

   void prevIssue();
   void nextIssue();

   void jumpTo(CurPos &cp);
   void jumpTo(Source *source, Int line);
   void jumpTo(Macro  *macro );
   void jumpTo(Symbol *symbol);

   void findAllReferences(C Str  &text);
   void findAllReferences(Symbol *symbol);
   void findAllReferences(C UID  &id);

   // io
   void  genSymbols(C Str &editor_bin);
   Bool saveSymbols(C Str &file);
   Bool loadSymbols(C Str &file, Bool all=true);

   void saveSettings(  TextNode &code);
   void loadSettings(C TextNode &code);

   Bool load     (                C SourceLoc &loc, Bool quiet=false, Bool Const=false);
   void save     (Source *source, C SourceLoc &loc);
   void overwrite();
   void save     ();
   void load     ();

   NO_COPY_CONSTRUCTOR(CodeEditor);
}extern
   CE;
/******************************************************************************/
extern Memc<Macro    > EEMacros, ProjectMacros;
extern Memc<SymbolPtr> EEUsings, ProjectUsings;
extern Memc<Str      > Suggestions;
/******************************************************************************/
void SuggestionsUsed    (C Str &text);
Int  SuggestionsPriority(C Str &suggestion, C Str &text, Bool all_up_case);
#endif
CChar8* ShortName(EXE_TYPE type);
} // namespace
/******************************************************************************/
