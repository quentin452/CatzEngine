/******************************************************************************/
class ElmApp : ElmData
{
   enum FLAG
   {
      EMBED_ENGINE_DATA     =1<<0,
      PUBLISH_PROJ_DATA     =1<<1,
      PUBLISH_PHYSX_DLL     =1<<2,
      PUBLISH_DATA_AS_PAK   =1<<3,
      PLAY_ASSET_DELIVERY   =1<<4, // Android https://developer.android.com/guide/playcore/asset-delivery
      PUBLISH_STEAM_DLL     =1<<5,
      PUBLISH_OPEN_VR_DLL   =1<<6,
      EMBED_ENGINE_DATA_FULL=1<<7,
    //WINDOWS_CODE_SIGN     =1<< ,
   };
   Str               dirs_windows, dirs_mac, dirs_linux, dirs_android, dirs_ios, dirs_nintendo,
                     headers_windows, headers_mac, headers_linux, headers_android, headers_ios, headers_nintendo,
                     libs_windows, libs_mac, libs_linux, libs_android, libs_ios, libs_nintendo,
                     package, android_license_key, location_usage_reason,
                     am_app_id_ios, am_app_id_google,
                     cb_app_id_ios, cb_app_signature_ios, cb_app_id_google, cb_app_signature_google, 
                     ms_publisher_name,
                     nintendo_initial_code,
                     nintendo_publisher_name,
                     nintendo_legal_info;
   int               build, save_size;
   ulong             fb_app_id, 
                     xbl_title_id,
                     nintendo_app_id;
   Edit::STORAGE_MODE storage;
   Edit::XBOX_LIVE    xbl_program;
   byte              supported_orientations, // DIR_FLAG
                     flag;
   UID               icon, notification_icon,
                     image_portrait, image_landscape, // splash screen
                     gui_skin, 
                     ms_publisher_id,
                     xbl_scid;
   Mems<LANG_TYPE>   supported_languages;
   TimeStamp         dirs_windows_time, dirs_mac_time, dirs_linux_time, dirs_android_time, dirs_ios_time, dirs_nintendo_time,
                     headers_windows_time, headers_mac_time, headers_linux_time, headers_android_time, headers_ios_time, headers_nintendo_time,
                     fb_app_id_time, am_app_id_ios_time, am_app_id_google_time, cb_app_id_ios_time, cb_app_signature_ios_time, cb_app_id_google_time, cb_app_signature_google_time,
                     libs_windows_time, libs_mac_time, libs_linux_time, libs_android_time, libs_ios_time, libs_nintendo_time,
                     package_time, android_license_key_time, location_usage_reason_time, build_time, save_size_time, storage_time, supported_orientations_time, supported_languages_time,
                     embed_engine_data_time, publish_proj_data_time, publish_physx_dll_time, publish_steam_dll_time, publish_open_vr_dll_time, publish_data_as_pak_time, play_asset_delivery_time,
                     icon_time, notification_icon_time, image_portrait_time, image_landscape_time, gui_skin_time,
                     ms_publisher_id_time, ms_publisher_name_time,
                     xbl_program_time, xbl_title_id_time, xbl_scid_time, 
                     nintendo_initial_code_time, nintendo_app_id_time, nintendo_publisher_name_time, nintendo_legal_info_time;

   // get
   bool equal(C ElmApp &src)C;
   bool newer(C ElmApp &src)C;

   virtual bool mayContain(C UID &id)C override;

   int     embedEngineData(     )C;
   ElmApp& embedEngineData(int e); 

   bool publishProjData  ()C;   ElmApp& publishProjData  (bool on);
   bool publishPhysxDll  ()C;   ElmApp& publishPhysxDll  (bool on);
   bool publishSteamDll  ()C;   ElmApp& publishSteamDll  (bool on);
   bool publishOpenVRDll ()C;   ElmApp& publishOpenVRDll (bool on);
   bool publishDataAsPak ()C;   ElmApp& publishDataAsPak (bool on);
   bool playAssetDelivery()C;   ElmApp& playAssetDelivery(bool on);
 //bool windowsCodeSign  ()C {return FlagOn(flag, WINDOWS_CODE_SIGN  );}   ElmApp& windowsCodeSign  (bool on) {FlagSet(flag, WINDOWS_CODE_SIGN  , on); return T;}

   // operations
   virtual void newData()override;
   uint undo(C ElmApp &src);
   uint sync(C ElmApp &src, bool manual);

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   class StorageMode
   {
      Edit::STORAGE_MODE mode;
      cchar8           *name;
   };
   static StorageMode StorageModes[]
;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmApp();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
