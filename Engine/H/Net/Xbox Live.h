/******************************************************************************

   Xbox Live is available only on Windows UWP

   In order to use Xbox Live functionality, you need to:
      1. Be a Microsoft Partner - https://partner.microsoft.com/en-us/dashboard

      2. Create an Application - https://partner.microsoft.com/en-us/dashboard/apps-and-games/overview
      3. Enable Xbox Live for it
         1. Open Application Page on https://partner.microsoft.com/en-us/dashboard/apps-and-games/overview
         2. Select "Services \ Xbox Live" in the menu
         3. Enable Xbox Live for Application
      4. Authorize Xbox Live accounts for testing
      5. Publish configuration using the "Test" button

      Full Microsoft documentation here - https://docs.microsoft.com/en-us/gaming/gdk/_content/gc/live/get-started/live-getstarted-nav

      6. Set Xbox Live Sandbox ID in the Windows Device Portal - https://docs.microsoft.com/en-us/windows/uwp/debug-test-perf/device-portal-desktop#set-up-windows-device-portal-on-a-desktop-device

      7. Configure your Application in Engine Editor
         1. Double-click your Application Element in the Project to open its properties
         2. Fill out the "Microsoft" and "XboxLive" fields (you can use "Get" buttons to navigate to Microsoft website where you can obtain these values)

   Please note that some Xbox Live functionality is limited depending on the type of your Microsoft Partner account:
      Creators                   - everything except Friends and Achievements
      Managed Partners (ID@Xbox) - full support

   Xbox Live has a 16 MB limit for individual cloud file sizes:
      -individual calls to 'cloudSave' may not exceed that limit

/******************************************************************************/
struct XBOXLive
{
   enum EVENT
   {
      STATUS_CHANGED   , // user log in status has changed
      USER_PROFILE     , // received full profile information for a user
      USER_FRIENDS     , // received list of friends, however their info is not complete (only 'id' and 'favorite' members will be valid, remaining members should be received soon through 'USER_PROFILE' events for each friend separately)
      USER_ACHIEVEMENTS, // received list of achievements due to a 'getAchievements' call, or list was updated due to a 'setAchievement' call
   };
   enum STATUS : Byte
   {
      LOGGED_OUT, // user currently logged out
      LOGGING_IN, // user currently logging in, but not finished yet
      LOGGED_IN , // user currently logged  in
   };

   struct User
   {
      ULong id=0;
      Str   name, image_url;
      Long  score=-1; // -1=unknown
   #if EE_PRIVATE
      void clear();
   #endif
   };
   struct Friend : User
   {
      Bool favorite=false;
   #if EE_PRIVATE
      void clear();
   #endif
   };

   struct Achievement
   {
      Str  id, name, unlocked_desc, locked_desc;
      Flt  progress; // achievement progress, 0=not started, 1=fully completed
      Bool secret; // if this is a secret achievement

      Bool notStarted()C {return progress<=0              ;} // if not yet started
      Bool inProgress()C {return progress> 0 && progress<1;} // if in progress
      Bool   unlocked()C {return progress>=1              ;} // if unlocked/achieved/completed
   };

   SyncLock lock;
   void   (*callback)(EVENT event, ULong user_id)=null; // pointer to a custom function that will be called with processed events, 'event'=event occurring at the moment, 'user_id'=user ID affected by this event

   STATUS  status()C {return _status;} // get current log in status
   Bool  loggedIn()C {return _status==LOGGED_IN;} // if currently logged in
   void     logIn(); // initiate log in process, result will be reported through the 'callback' function with a STATUS_CHANGED event
   
 C User& me          ()C {return _me          ;} // get user                           . This is valid only after being logged in
   ULong userID      ()C {return _me.id       ;} // get user ID           ,  0  on fail. This is valid only after being logged in
   Long  userScore   ()C {return _me.score    ;} // get user score        , -1  on fail. This is valid only after being logged in and after USER_PROFILE event
 C Str&  userName    ()C {return _me.name     ;} // get user name/gamertag,  "" on fail. This is valid only after being logged in
 C Str&  userImageURL()C {return _me.image_url;} // get user image url from which you can download his/her photo, for example by using the 'Download' class. This is valid only after being logged in and after USER_PROFILE event

   // Friends - This functionality is available only for Microsoft Managed Partners for ID@Xbox program
   void             getFriends(                         ) ;                   // initiate process of obtaining friend list, result will be reported through the 'callback' function with USER_FRIENDS event, only after that event methods below will return valid results
   Bool             getFriends(MemPtr<ULong > friend_ids)C;                   // get friend ID list, false on fail (this will always fail if 'getFriends' was not yet called or has not yet completed with a USER_FRIENDS event)
   Bool             getFriends(MemPtr<Friend> friends   )C;                   // get friend    list, false on fail (this will always fail if 'getFriends' was not yet called or has not yet completed with a USER_FRIENDS event)
 C Mems<Friend>& lockedFriends(                         )C {return _friends;} // get friend    list, empty on fail (this will always fail if 'getFriends' was not yet called or has not yet completed with a USER_FRIENDS event) this method can be called only under lock for 'lock' member
   Str             userName   (       ULong   user_id   )C;                   // get user name     , ""    on fail (this will always fail if 'getFriends' was not yet called or has not yet completed with a USER_FRIENDS event)

   // cloud saves
   Bool cloudSupported    ()C; // if cloud saves are supported, this will always be false if currently not logged in
   Long cloudAvailableSize()C; // get number of available bytes for cloud storage, -1 on fail

   Bool cloudDel(C Str &file_name); // delete 'file_name', false on fail

   Bool cloudSave(C Str &file_name, File &file,              Cipher *cipher=null); // save data from 'file' to 'file_name' cloud file, false on fail, only data from current 'file' file position to the end of the file is saved. !! There is a 16 MB limit for file size in Xbox Live !!
   Bool cloudLoad(C Str &file_name, File &file, Bool memory, Cipher *cipher=null); // load file from 'file_name' cloud file to 'file', false on fail, 'file' should be already opened for writing if 'memory' is set to false, if 'memory' is set to true then 'file' will be first reinitialized with 'writeMemFixed' before loading, which means that load result will not be stored into original 'file' target, but instead into a dynamically allocated memory

   struct CloudFile
   {
      Str  name;
      Long size;
   };
   Bool cloudFiles(MemPtr<CloudFile> files)C; // get list of files that are currently stored in the cloud, false on fail

   // Achievements - This functionality is available only for Microsoft Managed Partners for ID@Xbox program
   void                  getAchievements();                                     // initiate process of obtaining achievement list, result will be reported through the 'callback' function with USER_ACHIEVEMENTS event, only after that event methods below will return valid results
   Bool                  getAchievements(MemPtr<Achievement> achievements   )C; // get achievement list, false on fail (this will always fail if 'getAchievements' was not yet called or has not yet completed with a USER_ACHIEVEMENTS event)
   void                  setAchievement (C Str &achievement_id, Flt progress);  // set achievement progress, 'achievement_id'=ID of the achievement, 'progress'=0..1 (0=not started, 1=fully completed)
 C Mems<Achievement>& lockedAchievements()C {return _achievements;}             // get achievement list, empty on fail (this will always fail if 'getAchievements' was not yet called or has not yet completed with a USER_ACHIEVEMENTS event) this method can be called only under lock for 'lock' member

private:
   STATUS            _status=LOGGED_OUT;
   Bool              _friends_known=false, _friends_getting=false, _achievements_known=false, _achievements_getting=false;
   User              _me;
   Mems<Friend>      _friends;
   Mems<Achievement> _achievements;
#if EE_PRIVATE
   void setStatus(STATUS status);
   void logInOK();
   void logInOK1();
   void getUserProfile(); // request extra profile information for current user (such as score and image url), result will be reported through the 'callback' function
   void getUserProfile(C CMemPtr<ULong> &user_ids);
#endif
}extern
   XboxLive;
/******************************************************************************/
