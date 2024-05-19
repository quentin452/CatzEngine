/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'App' to setup initial application parameters inside 'InitPre' function.

/******************************************************************************/
enum APP_FLAG // Application Flags
{
    APP_NO_TITLE_BAR = 1 << 0,                     // application window will have no title bar [Supported Platforms: Windows, Linux]
    APP_NO_CLOSE = 1 << 1,                         // application will have close    box on the window disabled, and won't be closed by Alt+F4
    APP_MINIMIZABLE = 1 << 2,                      // apllication will have minimize box on the window
    APP_MAXIMIZABLE = 1 << 3,                      // apllication will have maximize box on the window
    APP_RESIZABLE = 1 << 4,                        // application window can be resized by dragging the window edges/corners
    APP_HIDDEN = 1 << 5,                           // application window will be initially hidden
    APP_FULL_TOGGLE = 1 << 6,                      // display can be altered with Alt+Enter keys (windowed/fullscreen toggle), Alt+Shift+Enter can also be used to switch into fullscreen with resolution taken from the window size
    APP_WORK_IN_BACKGROUND = 1 << 7,               // application will work also in background (when not focused)
    APP_ON_RUN_EXIT = 1 << 8,                      // if the application is already running in another instance, then new instance will not be created, but instead the existing instance window will be activated
    APP_ON_RUN_WAIT = 1 << 9,                      // if the application is already running in another instance, then wait until it exits and then continue
    APP_MEM_LEAKS = 1 << 10,                       // error will be shown at program exit if memory leaks were found, use this only for debugging as it may slow down the application [Supported Platforms: Windows]
    APP_NO_PAUSE_ON_WINDOW_MOVE_SIZE = 1 << 11,    // application will not pause when the main window is being moved or resized [Supported Platforms: Windows, other platforms don't need this]
    APP_AUTO_FREE_IMAGE_OPEN_GL_ES_DATA = 1 << 12, // this flag is used only for OpenGL ES, it specifies that all Images will automatically call 'freeOpenGLESData' after loading their data, the method frees the software copy of the GPU data which increases available memory, however after calling this method the data can no longer be accessed on the CPU (can no longer be locked or saved to file), you can enable this option for release version of your game to reduce its memory usage
    APP_AUTO_FREE_MESH_OPEN_GL_ES_DATA = 1 << 13,  // this flag is used only for OpenGL ES, it specifies that all Meshes will automatically call 'freeOpenGLESData' after loading their data, the method frees the software copy of the GPU data which increases available memory, however after calling this method the data can no longer be accessed on the CPU (can no longer be locked or saved to file), you can enable this option for release version of your game to reduce its memory usage
    APP_AUTO_FREE_PHYS_BODY_HELPER_DATA = 1 << 14, // 'PhysBody' objects will automatically call their 'freeHelperData' method after loading from file, this will free up some memory, however it will disable saving the 'PhysBody' to file, or converting it to 'MeshBase', you can enable this option for release version of your game to reduce its memory usage
    APP_ALLOW_NO_GPU = 1 << 15,                    // allow application to run if the graphics driver does not meet the minimum requirements, this mode is recommended for server applications where GPU can be limited or not available. This flag works differently for different API's : if in DX10+ the graphics failed to initialize then application will run and will display graphics but using "WARP - Windows Advanced Rasterization Platform" which uses CPU for graphics, in order to ensure maximum CPU performance on servers you may want to create the application window small - less pixels to draw = smaller CPU usage, or minimize the application window at startup = completely disables any drawing/rendering, in OpenGL graphics will always be disabled when using this flag
    APP_ALLOW_NO_XDISPLAY = 1 << 16,               // this flag is only for Linux: allow application to run if the X Window System failed to initialize or is not available
    APP_EXIT_IMMEDIATELY = 1 << 17,                // if this flag is specified then the application will exit immediately after 'InitPre' function
    APP_BREAKPOINT_ON_ERROR = 1 << 18,             // if program encounters an error and 'Exit' function is called, then breakpoint will be forced by calling the 'Break' during the 'Exit' function, allowing you to enter debug mode, use this option only for debugging
    APP_CALLSTACK_ON_ERROR = 1 << 19,              // if program encounters an error and 'Exit' function is called, then current call stack will be included in the error message [Supported Platforms: Windows]
    APP_WEB_DISABLE_AUTO_RESIZE = 1 << 20,         // Web applications by default will auto resize the canvas to cover the entire browser window client area, to disable this behavior you can enable this option [Supported Platforms: Web]
    APP_FADE_OUT = 1 << 21,                        // if enabled then application window and all sounds will smoothly fade out at application close, available only on Desktop platforms
    APP_IGNORE_PRECOMPILED_SHADER_CACHE = 1 << 22, // if ignore loading shaders from Precompiled Shader Cache
    APP_SHADER_CACHE_MAX_COMPRESS = 1 << 23,       // if enable maximum compression for the Shader Cache, this slows down generation of the Shader Cache, but reduces its size
    APP_JOYPAD_THREAD = 1 << 24,                   // if create a dedicated thread for polling joypads state, enables higher precision joypad events at the cost of extra processing
    APP_JOYPAD_DISABLE_MINI = 1 << 25,             // if disable support for a single Nintendo Switch Joy-Con Left or Right held horizontally [Supported Platforms: NintendoSwitch]

    APP_AUTO_FREE_OPEN_GL_ES_DATA = APP_AUTO_FREE_IMAGE_OPEN_GL_ES_DATA | APP_AUTO_FREE_MESH_OPEN_GL_ES_DATA,
};
/******************************************************************************/
enum AWAKE_MODE : Byte {
#if EE_PRIVATE
// !! THESE ENUMS MUST BE EQUAL TO "EsenthelActivity.java" !!
#endif
    AWAKE_OFF,    // the system and screen can go to sleep
    AWAKE_SYSTEM, // prevent the system from going to sleep, however the screen can go to sleep
    AWAKE_SCREEN, // prevent the system and screen from going to sleep, as long as the App is active or has APP_WORK_IN_BACKGROUND enabled
};
/******************************************************************************/
enum SYSTEM_BAR : Byte {
#if EE_PRIVATE
// !! THESE ENUMS MUST BE EQUAL TO "EsenthelActivity.java" !!
#endif
    SYSTEM_BAR_HIDDEN,  // completely hidden
    SYSTEM_BAR_OVERLAY, // semi-transparent drawn on top of the application
    SYSTEM_BAR_VISIBLE, // completely visible
};
/******************************************************************************/
struct Application // Application Settings
{
    Int x = -1, y = 1;   // initial position (-1..1) of the window on the desktop at the creation of application
    UInt flag;           // APP_FLAG
    Int active_wait,     // amount of milliseconds the application should wait before making 'Update' calls when in active     mode                                        , -1=unlimited (app will wait until event occurs  ), 0=instant (app will keep calling 'Update' continuously), >0=wait (app will wait specified time until event occurs before making 'Update' calls), default=0. It's recommended to use this instead of manually waiting with 'Time.wait', because this method allows app to resume instantly when event occurs     , unlike 'Time.wait' which waits without stopping.
        background_wait; // amount of milliseconds the application should wait before making 'Update' calls when in background mode and with APP_WORK_IN_BACKGROUND enabled, -1=unlimited (app will wait until it's activated), 0=instant (app will keep calling 'Update' continuously), >0=wait (app will wait specified time until activated    before making 'Update' calls), default=0. It's recommended to use this instead of manually waiting with 'Time.wait', because this method allows app to resume instantly when it gets activated, unlike 'Time.wait' which waits without stopping.
    Mems<Str> cmd_line;  // command line arguments

    void (*save_state)();                                                  // pointer to custom function called when application is being put into background or will be terminated, in this function you should save current state of data which you may want to restore at next application startup, this function is used only on mobile platforms where the Operating System may close the application for any reason, default=null
    void (*paused)();                                                      // pointer to custom function called when application is being  paused (lost   focus), default=null
    void (*resumed)();                                                     // pointer to custom function called when application is being resumed (gained focus), default=null
    void (*drop)(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos); // pointer to custom function called when a file is Drag and Dropped on the application window, 'names'=list of file names being drag and dropped, 'focus_obj'=gui object at which elements are being dropped, 'screen_pos'=screen position at which dropping occurs, default=null
    void (*receive_file)(File &f);                                         // pointer to custom function called when a file is received [Supported Platforms: Android, iOS]
    void (*receive_data)(CPtr data, Int size, C SysWindow &sender_window); // pointer to custom function called when application has received binary data sent using 'SysWindow.sendData' function, application may not access 'data' memory after the callback function returns, 'sender_window'=system window of the sender, default=null
    void (*quit)();                                                        // pointer to custom function called when the application was requested to quit (for example by pressing Alt+F4 or clicking on the "X" close window button), if this member is different than null, then the application will not exit automatically, manual exiting can be called inside custom 'quit' function (for example by activating 'StateExit' State), this member is ignored when the application was created with APP_NO_CLOSE flag, default=null, this is not supported on Windows Universal Apps
    void (*exit)(CChar *error);                                            // pointer to custom function called when 'Exit' function is called, typically this happens when application has encountered an error and is about to be terminated, default=null
    void (*low_memory)();                                                  // pointer to custom function called when the application has received low memory notification from the system, inside it you can release any unnecessary memory, default=null
    void (*notification)(Notification &notification, Bool selected);       // pointer to custom function called when the application notification was selected or dismissed. If the callback is specified, then this notification is not automatically removed, you should call 'notification.remove' to remove it. If no callback is specified, then 'notification.remove' is called automatically.
    void (*sleep)(Bool sleep);                                             // pointer to custom function called when the device is going to sleep.
    void (*joypad_changed)();                                              // pointer to custom function called when a Joypad has been added, removed or changed order, default=null
    void (*joypad_user_changed)(UInt joypad_id);                           // pointer to custom function called when user associated with a Joypad has changed. This is called on Windows UWP only.

    // get / set
    Application &name(C Str &name);                    // set application name
    C Str &name() C { return _name; }                  // get application name
    C Str &exe() C { return _exe; }                    // get executable path and name
    Application &lang(LANG_TYPE lang);                 // set application language, some engine messages rely on this value, changing language triggers 'GuiObj.setText' for all gui objects, default=OSLanguage()
    LANG_TYPE lang() C { return _lang; }               // get application language, some engine messages rely on this value, changing language triggers 'GuiObj.setText' for all gui objects, default=OSLanguage()
    Bool elevated() C { return _elevated; }            // get application        process elevation (true if has administrator rights)
    UInt parentProcessID() C;                          // get application parent process ID
    UInt processID() C { return _process_id; }         // get application        process ID
    UIntPtr threadID() C { return _thread_id; }        // get application main   thread  ID
    Application &threadPriority(Int priority);         // set application main   thread  priority, 'priority'=-3..3
    Application &threadMask(ULong mask);               // set application main   thread  CPU HW Threads on which the thread is allowed to run (every bit specifies different HW Thread)
    C SysWindow &window() C { return _window; }        // get application system window
    C RectI &desktopArea() C { return _desktop_area; } // get available desktop area (not covered by windows taskbar or other desktop toolbars)
    C VecI2 &desktop() C { return _desktop_size; }     // get screen size   at the moment of application start (desktop size  )
    Int desktopW() C { return _desktop_size.x; }       // get screen width  at the moment of application start (desktop width )
    Int desktopH() C { return _desktop_size.y; }       // get screen height at the moment of application start (desktop height)
    Bool active() C { return _active; }                // if  application is active (its window is focused)
    Bool minimized() C;                                // if  application is minimized
    Bool maximized() C;                                // if  application is maximized
    Bool closed() C { return _closed; }                // if  application has finished closing, this is enabled at the very end of application life cycle, right before all global C++ destructors being called
    Flt opacity() C;                                   // get application window opacity, 0..1
    Application &opacity(Flt opacity);                 // set application window opacity, 0..1
    Application &flash();                              // set application window to flash
    Application &stateNormal();                        // set application window to be displayed as normal                             (this will work only on Window 7 or newer)
    Application &stateWorking();                       // set application window to be displayed as working with unknown progress      (this will work only on Window 7 or newer)
    Application &stateProgress(Flt progress);          // set application window to be displayed as working with 'progress' 0..1 value (this will work only on Window 7 or newer)
    Application &statePaused(Flt progress);            // set application window to be displayed as paused  with 'progress' 0..1 value (this will work only on Window 7 or newer)
    Application &stateError(Flt progress);             // set application window to be displayed as error   with 'progress' 0..1 value (this will work only on Window 7 or newer)
    Bool hidden() C;                                   // if  application window is hidden
    Application &hide();                               // set application window to be hidden
    Application &show(Bool activate);                  // set application window to be visible, 'activate'=if also activate
    DIR_ENUM orientation() C;                          // get device orientation, this is valid for mobile devices which support accelerometers, for those devices the method will return one of the following orientation: DIR_UP (default), DIR_DOWN (rotated down), DIR_LEFT (rotated left), DIR_RIGHT (rotated right), if the device doesn't support accelerometers then DIR_UP is returned
    Bool mainThread() C;                               // if  current thread is the main thread
    Application &icon(C Image &icon);                  // set custom application icon
    Application &stayAwake(AWAKE_MODE mode);           // set preventing the Operating System from going to sleep
    C Str &backgroundText() C { return _back_text; }   // get text displayed on the Status Bar Notification when App is running in Background mode on Android, default="Running in background"
    Application &backgroundText(C Str &text);          // set text displayed on the Status Bar Notification when App is running in Background mode on Android, default="Running in background"
    Bool backgroundFull() C { return _back_full; }     // get if Application is allowed to remain visible on the screen when in fullscreen mode but inactive (if false then application is minimized), default=false
    Application &backgroundFull(Bool on);              // set if Application is allowed to remain visible on the screen when in fullscreen mode but inactive (if false then application is minimized), default=false

    // system bars
    Application &systemBars(SYSTEM_BAR status, SYSTEM_BAR navigation); //     set system     bars      [Supported Platforms: Android, iOS]
    SYSTEM_BAR statusBar() C { return _status; }
    Application &statusBar(SYSTEM_BAR bar); // get/set status     bar       [Supported Platforms: Android, iOS]
    SYSTEM_BAR navBar() C { return _nav; }
    Application &navBar(SYSTEM_BAR bar); // get/set navigation bar       [Supported Platforms: Android, iOS]
    Bool statusBarColor() C { return _status_color; }
    Application &statusBarColor(Bool light); // get/set status     bar color [Supported Platforms: Android, iOS]
    Bool navBarColor() C { return _nav_color; }
    Application &navBarColor(Bool light); // get/set navigation bar color [Supported Platforms: Android     ]

    // operations
    Bool renameSelf(C Str &dest); // rename application executable file to 'dest' location, false on fail
    void deleteSelfAtExit();      // notify that the exe should delete itself at application exit
    void close();                 // request application to be closed, if 'App.quit' was specified then it will be called instead

    void pickPhoto(Int max_files); // open Photo Picker allowing to select 'max_files' number of images, they will be received through 'App.receive_file' callback [Supported Platforms: Android, iOS]

    // function callbacks
    void addFuncCall(void func()) { _callbacks.add(func); }                         // add custom function to the Application callback list to be automatically called during Application Update on the main thread
    void addFuncCall(void func(Ptr user), Ptr user) { _callbacks.add(func, user); } // add custom function to the Application callback list to be automatically called during Application Update on the main thread
    T1(TYPE)
    void addFuncCall(void func(TYPE *user), TYPE *user) { addFuncCall((void (*)(Ptr))func, user); } // add custom function to the Application callback list to be automatically called during Application Update on the main thread
    T1(TYPE)
    void addFuncCall(void func(TYPE &user), TYPE &user) { addFuncCall((void (*)(Ptr))func, &user); } // add custom function to the Application callback list to be automatically called during Application Update on the main thread

    void includeFuncCall(void func()) { _callbacks.include(func); }                         // include custom function to the gui function callback list to be automatically called at the end of the 'Gui.update'
    void includeFuncCall(void func(Ptr user), Ptr user) { _callbacks.include(func, user); } // include custom function to the gui function callback list to be automatically called at the end of the 'Gui.update'
    T1(TYPE)
    void includeFuncCall(void func(TYPE *user), TYPE *user) { includeFuncCall((void (*)(Ptr))func, user); } // include custom function to the gui function callback list to be automatically called at the end of the 'Gui.update'
    T1(TYPE)
    void includeFuncCall(void func(TYPE &user), TYPE &user) { includeFuncCall((void (*)(Ptr))func, &user); } // include custom function to the gui function callback list to be automatically called at the end of the 'Gui.update'

#if EE_PRIVATE
    static Bool Fullscreen();

    void del();
    Bool create0();
    Bool create1();
    Bool create();
    void update();
    void loop();
#if WINDOWS
    HMONITOR hmonitor() C;
#endif
#if WINDOWS_NEW
    void wait(SyncEvent &event); // wait for async operation to complete
    static void ExecuteRecordedEvents();
#elif LINUX
    void setWindowFlags(Bool force_resizable = false);
#endif
    Bool activeOrBackFull() C { return _active || _back_full; }
    void activeOrBackFullChanged();
    void setActive(Bool active);
    void setStayAwake();

    Bool testInstance();
    void windowCreate();
    void windowDel();
    void windowMsg();
    void windowAdjust(Bool set = false);
    void deleteSelf();
    void detectMemLeaks();
    void showError(CChar *error);
    void lowMemory();
#endif

#if !EE_PRIVATE
  private:
#endif
    Bool _active, _initialized, _minimized, _maximized, _close, _closed, _del_self_at_exit, _elevated, _back_full, _status_color, _nav_color;
#if WINDOWS_NEW
    Bool _waiting;
#endif
    AWAKE_MODE _stay_awake;
    DIR_ENUM _orientation = DIR_UP;
    LANG_TYPE _lang;
    SYSTEM_BAR _status, _nav;
    SByte _thread_priority;
    Int _mem_leaks;
    mutable UInt _parent_process_id;
    UInt _process_id;
    UIntPtr _thread_id;
    ULong _thread_mask;
    SysWindow _window;
    VecI2 _window_pos, _window_size, _window_resized, _desktop_size;
    RectI _desktop_area, _bound, _bound_maximized;
    Str _exe, _name, _back_text;
#if WINDOWS_OLD
    UInt _style_window, _style_window_maximized, _style_full;
#if EE_PRIVATE
    HINSTANCE _hinstance;
#else
    Ptr _hinstance;
#endif
#endif
#if WINDOWS
#if EE_PRIVATE
    HICON _icon;
#else
    Ptr _icon;
#endif
#else
    Image _icon;
#endif
#if MAC
    UInt _style_window;
#endif
    ThreadSafeCallbacks _callbacks;

    Application();
} extern App; // Application Settings
/******************************************************************************/
void StartEEManually(Ptr dll_module_instance); // this function imitates "WinMain" function, and should be called only in a DLL based application to start the engine manually

void LoadEmbeddedPaks(Cipher *cipher);                       // load PAK files embedded in Application executable file, you can call this inside 'InitPre' function, 'cipher'=Cipher used for encrypting PAK files
void LoadAndroidAssetPacks(Int asset_packs, Cipher *cipher); // load Project Data from Android Asset Packs

void SupportCompressBC();    // call this inside 'InitPre' function to add support for compressing IMAGE_BC6/7 formats                     , using this will however make your executable file bigger
void SupportCompressETC();   // call this inside 'InitPre' function to add support for compressing IMAGE_ETC   formats                     , using this will however make your executable file bigger
void SupportCompressPVRTC(); // call this inside 'InitPre' function to add support for compressing IMAGE_PVRTC formats on Desktop platforms, using this will however make your executable file bigger
void SupportCompressAll();   // call this inside 'InitPre' function to add support for compressing all         formats                     , using this will however make your executable file bigger
/******************************************************************************/
// Multi Language Text, returns one of the few translations provided depending on current application language
inline Str MLT(C Str &english) { return english; }
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0);
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1);
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2);
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3);
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4);
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5);
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5, LANG_TYPE l6, C Str &t6);
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5, LANG_TYPE l6, C Str &t6, LANG_TYPE l7, C Str &t7);
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5, LANG_TYPE l6, C Str &t6, LANG_TYPE l7, C Str &t7, LANG_TYPE l8, C Str &t8);
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5, LANG_TYPE l6, C Str &t6, LANG_TYPE l7, C Str &t7, LANG_TYPE l8, C Str &t8, LANG_TYPE l9, C Str &t9);
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5, LANG_TYPE l6, C Str &t6, LANG_TYPE l7, C Str &t7, LANG_TYPE l8, C Str &t8, LANG_TYPE l9, C Str &t9, LANG_TYPE l10, C Str &t10);
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5, LANG_TYPE l6, C Str &t6, LANG_TYPE l7, C Str &t7, LANG_TYPE l8, C Str &t8, LANG_TYPE l9, C Str &t9, LANG_TYPE l10, C Str &t10, LANG_TYPE l11, C Str &t11);

// Multi Language Text Constant, returns one of the few translations provided depending on current application language
inline CChar *MLTC(CChar *english) { return english; }
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0);
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1);
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2);
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3);
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4);
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5);
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5, LANG_TYPE l6, CChar *t6);
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5, LANG_TYPE l6, CChar *t6, LANG_TYPE l7, CChar *t7);
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5, LANG_TYPE l6, CChar *t6, LANG_TYPE l7, CChar *t7, LANG_TYPE l8, CChar *t8);
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5, LANG_TYPE l6, CChar *t6, LANG_TYPE l7, CChar *t7, LANG_TYPE l8, CChar *t8, LANG_TYPE l9, CChar *t9);
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5, LANG_TYPE l6, CChar *t6, LANG_TYPE l7, CChar *t7, LANG_TYPE l8, CChar *t8, LANG_TYPE l9, CChar *t9, LANG_TYPE l10, CChar *t10);
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5, LANG_TYPE l6, CChar *t6, LANG_TYPE l7, CChar *t7, LANG_TYPE l8, CChar *t8, LANG_TYPE l9, CChar *t9, LANG_TYPE l10, CChar *t10, LANG_TYPE l11, CChar *t11);
/******************************************************************************/
#if EE_PRIVATE
extern Bool LogInit;
#if LINUX
extern Atom _NET_WM_STATE, _NET_WM_NAME, UTF8_STRING;
#endif
#endif
/******************************************************************************/
