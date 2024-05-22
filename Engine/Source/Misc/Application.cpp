/******************************************************************************/
#include "stdafx.h"
#if MAC
#include "../Platforms/Mac/MyApplication.h"
#elif IOS
#include "../Platforms/iOS/iOS.h"
#endif
namespace EE {
/******************************************************************************/
Bool LogInit = false;

ASSERT(MEMBER_SIZE(ImageTypeInfo, format) == SIZE(UInt));
ASSERT(MEMBER_SIZE(VtxIndBuf, _prim_type) == SIZE(UInt));
ASSERT(MEMBER_SIZE(DisplayState, _prim_type) == SIZE(UInt));
#if WINDOWS_OLD
#define SM_DIGITIZER 94
#define SM_MAXIMUMTOUCHES 95
ASSERT(SIZE(D3DVECTOR) == SIZE(Vec)); // shaders
                                      // ASSERT(SIZE(D3DXVECTOR4)==SIZE(Vec4   )); // shaders
// ASSERT(SIZE(D3DXMATRIX )==SIZE(Matrix4)); // shaders

static Bool ShutCOM = false;
static ITaskbarList3 *TaskbarList;
#elif LINUX
Atom _NET_WM_STATE, _NET_WM_NAME, UTF8_STRING;
static Atom _NET_WM_STATE_FULLSCREEN, _NET_WM_STATE_DEMANDS_ATTENTION, _NET_WM_ICON;
static int (*OldErrorHandler)(::Display *d, XErrorEvent *e);
static int ErrorHandler(::Display *d, XErrorEvent *e) {
#if 1 // just always ignore errors in case 'glXQueryExtension' fails, or 'GLXBadFBConfig' is not 9 anymore
    return 0;
#else
    if (e->error_code == BadWindow)
        return 0;
    int errorBase, eventBase;
    if (glXQueryExtension(d, &errorBase, &eventBase)) {
        const int GLXBadFBConfig = 9;
        if (e->error_code == errorBase + GLXBadFBConfig)
            return 0; // can happen when trying to create OpenGL context using 'glXCreateContextAttribsARB' with unsupported version, with message "Error of failed request:  GLXBadFBConfig"
    }
    return OldErrorHandler ? OldErrorHandler(d, e) : 0;
#endif
}
#endif
/******************************************************************************/
Application App;
/******************************************************************************/
Application::Application() {
#if 0 // there's only one 'Application' global 'App' and it doesn't need clearing members to zero
   flag=0;
       active_wait=0;
   background_wait=0;
   cipher=null;
   receive_data=null;
   save_state=null;
   paused=null;
   resumed=null;
   drop=null;
   quit=null;
   exit=null;
   low_memory=null;
   notification=null;
  _active=_initialized=_minimized=_maximized=_close=_closed=_del_self_at_exit=_elevated=_back_full=false;
  _thread_priority=0;
#if WINDOWS_NEW
  _waiting=false;
#endif
  _stay_awake=AWAKE_OFF;
  _lang=LANG_NONE;
//_mem_leaks=0; don't set this as it could have been already modified
  _process_id=_parent_process_id=0;
  _hwnd=null;
  _window_pos=_window_size=_window_resized=_desktop_size.zero();
  _desktop_area=_bound=_bound_maximized.zero();
#if WINDOWS_OLD
  _style_window=_style_window_maximized=_style_full=0;
  _hinstance=null;
#endif
#if WINDOWS
  _icon=null;
#endif
#if MAC
  _style_window=0;
#endif
#endif
    _thread_mask = ~0;
    _status_color = _nav_color = true;
    _status = SYSTEM_BAR_HIDDEN;                          // must match #SystemBar
    _nav = IOS ? SYSTEM_BAR_OVERLAY : SYSTEM_BAR_VISIBLE; // must match #SystemBar
    _thread_id = GetThreadID();
    _back_text = "Running in background";
}
Application &Application::name(C Str &name) {
    T._name = name;
#if WINDOWS_OLD
    SetWindowText(window(), name);
#elif WINDOWS_NEW
    Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->Title = ref new Platform::String(name);
#elif MAC
    if (window())
        [window() setTitle:NSStringAuto(name)];
#elif LINUX
    if (XDisplay && window()) {
        Str8 utf8 = UTF8(name), name8 = name;
        XClassHint class_hint;
        Zero(class_hint); // this is needed to display name on the taskbar, without this, name can be displayed as "Unknown"
        class_hint.res_name = class_hint.res_class = (char *)name8();
        XSetClassHint(XDisplay, window(), &class_hint);
        XStoreName(XDisplay, window(), name8); // this is needed in case _NET_WM_NAME/UTF8_STRING are not available
        if (_NET_WM_NAME && UTF8_STRING)
            XChangeProperty(XDisplay, window(), _NET_WM_NAME, UTF8_STRING, 8, PropModeReplace, (unsigned char *)utf8(), utf8.length());
    }
#endif
    return T;
}
/******************************************************************************/
Memx<Notification> Notifications;
#if ANDROID
static void RemoveNotification(Int id) {
    JNI jni;
    if (jni && ActivityClass)
        if (JMethodID removeNotification = jni.staticFunc(ActivityClass, "removeNotification", "(I)V"))
            jni->CallStaticVoidMethod(ActivityClass, removeNotification, jint(id));
}
static void SetNotification(Int id, C Str &title, C Str &text, Bool dismissable) {
    JNI jni;
    if (jni && ActivityClass)
        if (JMethodID setNotification = jni.staticFunc(ActivityClass, "setNotification", "(ILjava/lang/String;Ljava/lang/String;Z)V"))
            if (JString j_title = JString(jni, title))
                if (JString j_text = JString(jni, text))
                    jni->CallStaticVoidMethod(ActivityClass, setNotification, jint(id), j_title(), j_text(), jboolean(dismissable));
}
extern "C" JNIEXPORT void JNICALL Java_com_esenthel_Native_notification(JNIEnv *env, jclass clazz, jint id, jboolean selected) {
    if (Notifications.absToValidIndex(id) >= 0) // if present
    {
        Notification &notification = Notifications.absElm(id);
        if (!selected)
            notification._visible = false; // if was dismissed then set as hidden (not visible), so calling 'hide' won't do anything and calling 'set' could restore it
        if (auto call = App.notification)
            call(notification, selected);
        else
            Notifications.removeAbs(id, true); // if there's no callback then remove the notification
    } else if (selected)
        RemoveNotification(id); // if not found and selected (not dismissed), then remove manually
}
#endif
Notification::~Notification() {
    hide();
}
void Notification::hide() {
    if (_visible) {
#if ANDROID
        Int abs = Notifications.absIndex(this);
        if (abs >= 0)
            RemoveNotification(abs);
#endif
        _visible = false;
    }
}
void Notification::remove() {
    Notifications.removeData(this, true);
    // !! do not do anything here because this notification is no longer valid !!
}
void Notification::set(C Str &title, C Str &text, Bool dismissable) {
    if (!Equal(_title, title, true) || !Equal(_text, text, true) || _dismissable != dismissable || !_visible) {
        T._title = title;
        T._text = text;
        T._dismissable = dismissable;
#if ANDROID
        Int valid = Notifications.validIndex(this);
        if (valid >= 0) // if present
        {
            Int abs = Notifications.validToAbsIndex(valid);
            if (abs >= 0) {
                SetNotification(abs, title, text, dismissable);
                T._visible = true;
            }
        }
#endif
    }
}
void HideNotifications() {
#if ANDROID // on Android, force closing app does not hide notifications, so if there's a non-dismissable notification, it will get stuck
    // REPA(Notifications)if(!Notifications[i].dismissable())Notifications.removeValid(i, true);
    Notifications.clear(); // actually hide all notifications, so if a notification is shown, and app force closed, then the notification has ID belonging to a Notification that no longer exists (because force close removed it), so it can't be found in the 'Notifications' list anymore
#endif
}
Application &Application::backgroundText(C Str &text) {
    if (!Equal(T._back_text, text, true)) {
        T._back_text = text;
#if ANDROID
        if (!App.active())
            SetNotification(-1, App.name(), text, false); // adjust existing, -1=BACKGROUND_NOTIFICATION_ID from Java
#endif
    }
    return T;
}
/******************************************************************************/
#if ANDROID
void SetSystemBars() {
    JNI jni;
    if (jni && ActivityClass)
        if (JMethodID systemBars = jni.staticFunc(ActivityClass, "systemBars", "(BBZZ)V")) {
            jni->CallStaticVoidMethod(ActivityClass, systemBars, jbyte(App.statusBar()), jbyte(App.navBar()), jboolean(App.statusBarColor()), jboolean(App.navBarColor()));
            extern void AndroidResized();
            AndroidResized(); // immediatelly request size check to call 'D.screenChanged' ASAP. This is so we can get latest 'D.rectUI' before waiting for Android/Java to process requests on other threads. For this we need #ImmediateInsets
        }
}
#endif
Application &Application::systemBars(SYSTEM_BAR status, SYSTEM_BAR navigation) {
#if ANDROID
    if (_status != status || _nav != navigation) {
        _status = status;
        _nav = navigation;
        SetSystemBars();
    }
#else
    statusBar(status);
    navBar(navigation);
#endif
    return T;
}
Application &Application::statusBar(SYSTEM_BAR bar) {
    if (_status != bar) {
        _status = bar;
#if ANDROID
        SetSystemBars();
#elif IOS
        if (ViewController)
            [ViewController setNeedsStatusBarAppearanceUpdate];
#endif
    }
    return T;
}
Application &Application::navBar(SYSTEM_BAR bar) {
    if (_nav != bar) {
        _nav = bar;
#if ANDROID
        SetSystemBars();
#elif IOS
        if (ViewController) {
            [ViewController setNeedsUpdateOfHomeIndicatorAutoHidden];
            [ViewController setNeedsUpdateOfScreenEdgesDeferringSystemGestures];
        }
#endif
    }
    return T;
}
Application &Application::statusBarColor(Bool light) {
    if (_status_color != light) {
        _status_color = light;
#if ANDROID
        SetSystemBars();
#elif IOS
        if (ViewController)
            [ViewController setNeedsStatusBarAppearanceUpdate];
#endif
    }
    return T;
}
Application &Application::navBarColor(Bool light) {
    if (_nav_color != light) {
        _nav_color = light;
#if ANDROID
        SetSystemBars();
#elif IOS
        // unsupported
#endif
    }
    return T;
}
/******************************************************************************/
#if WINDOWS || ANDROID
Bool Application::minimized() C { return _minimized; }
Bool Application::maximized() C { return _maximized; }
#elif LINUX
Bool Application::minimized() C { return window().minimized(); }
Bool Application::maximized() C { return _maximized; } // '_maximized' is obtained in 'ConfigureNotify' system message
#elif MAC
Bool Application::minimized() C { return _minimized; } // '_minimized' is obtained in 'windowDidMiniaturize, windowDidDeminiaturize'
Bool Application::maximized() C { return window().maximized(); }
#elif IOS
Bool Application::minimized() C { return false; }
Bool Application::maximized() C { return true; }
#elif SWITCH
Bool Application::minimized() C { return false; }
Bool Application::maximized() C { return true; }
#elif WEB
Bool Application::minimized() C { return false; }
Bool Application::maximized() C { return _maximized; }
#endif

UInt Application::parentProcessID() C {
#if WINDOWS_OLD
    if (!_parent_process_id) {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 proc;
            proc.dwSize = SIZE(PROCESSENTRY32);
            if (Process32First(snap, &proc))
                do {
                    if (proc.th32ProcessID == processID()) {
                        _parent_process_id = proc.th32ParentProcessID;
                        break;
                    }
                } while (Process32Next(snap, &proc));
            CloseHandle(snap);
        }
    }
#endif
    return _parent_process_id;
}

DIR_ENUM Application::orientation() C {
#if IOS
    // switch([[UIDevice currentDevice] orientation]) this is faulty (if the app starts rotated, then this has wrong value that doesn't get updated until device is rotated)
    switch ([UIApplication sharedApplication].statusBarOrientation) {
    default:
        return DIR_UP;
    case UIInterfaceOrientationPortraitUpsideDown:
        return DIR_DOWN;
    case UIInterfaceOrientationLandscapeLeft:
        return DIR_RIGHT;
    case UIInterfaceOrientationLandscapeRight:
        return DIR_LEFT;
    }
#else
    return _orientation;
#endif
}
/******************************************************************************/
Flt Application::opacity() C {
#if WINDOWS_OLD
    BYTE alpha;
    if (GetLayeredWindowAttributes(window(), null, &alpha, null))
        return alpha / 255.0f;
#elif MAC
    if (window())
        return window()().alphaValue;
#endif
    return 1;
}
Application &Application::opacity(Flt opacity) {
#if WINDOWS_OLD
    Byte alpha = FltToByte(opacity);
    if (alpha == 255) {
        SetWindowLong(window(), GWL_EXSTYLE, GetWindowLong(window(), GWL_EXSTYLE) & ~WS_EX_LAYERED);
    } else {
        SetWindowLong(window(), GWL_EXSTYLE, GetWindowLong(window(), GWL_EXSTYLE) | WS_EX_LAYERED);
        SetLayeredWindowAttributes(window(), 0, alpha, LWA_ALPHA);
    }
#elif MAC
    if (window())
        [window() setAlphaValue:opacity];
#endif
    return T;
}
Application &Application::flash() {
#if WINDOWS_OLD
    FlashWindow(window(), true);
#elif MAC
    [NSApp requestUserAttention:NSInformationalRequest];
#elif LINUX
    if (XDisplay && window() && _NET_WM_STATE && _NET_WM_STATE_DEMANDS_ATTENTION) {
#if 0   // this doesn't work at all
      XClientMessageEvent event; Zero(event);
      event.type        =ClientMessage;
      event.message_type=_NET_WM_STATE;
      event.display   =XDisplay;
      event.serial    =0;
      event.window    =window();
      event.send_event=1;
      event.format   =32;
      event.data.l[0]=_NET_WM_STATE_ADD;
      event.data.l[1]=_NET_WM_STATE_DEMANDS_ATTENTION;
      XSendEvent(XDisplay, DefaultRootWindow(XDisplay), false, SubstructureRedirectMask|SubstructureNotifyMask, (XEvent*)&event);
#elif 0 // this doesn't work at all
        XEvent e;
        Zero(e);
        e.xclient.type = ClientMessage;
        e.xclient.window = window();
        e.xclient.message_type = _NET_WM_STATE;
        e.xclient.format = 32;
        e.xclient.data.l[0] = _NET_WM_STATE_ADD;
        e.xclient.data.l[1] = _NET_WM_STATE_DEMANDS_ATTENTION;
        e.xclient.data.l[2] = 0;
        e.xclient.data.l[3] = 1;
        XSendEvent(XDisplay, DefaultRootWindow(XDisplay), false, SubstructureRedirectMask | SubstructureNotifyMask, &e);
#else   // more complex code but works
        Atom type = NULL;
        int format = 0;
        unsigned long items = 0, bytes_after = 0;
        unsigned char *data = null;
        if (!XGetWindowProperty(XDisplay, window(), _NET_WM_STATE, 0, 1024, false, XA_ATOM, &type, &format, &items, &bytes_after, &data)) {
            Atom *atoms = (Atom *)data, temp[1024];
            if (items < Elms(temp) - 1) // room for '_NET_WM_STATE_DEMANDS_ATTENTION'
            {
                bool has = false;
                for (unsigned long i = 0; i < items; i++) {
                    temp[i] = atoms[i];
                    if (temp[i] == _NET_WM_STATE_DEMANDS_ATTENTION)
                        has = true;
                }
                if (!has) {
                    temp[items++] = _NET_WM_STATE_DEMANDS_ATTENTION;
                    XChangeProperty(XDisplay, window(), _NET_WM_STATE, XA_ATOM, 32, PropModeReplace, (unsigned char *)temp, items);
                }
            }
        }
        if (data)
            XFree(data);
#endif
    }
#endif
    return T;
}
/******************************************************************************/
#if WINDOWS_OLD
Application &Application::stateNormal() {
    if (TaskbarList)
        TaskbarList->SetProgressState(window(), TBPF_NOPROGRESS);
    return T;
}
Application &Application::stateWorking() {
    if (TaskbarList)
        TaskbarList->SetProgressState(window(), TBPF_INDETERMINATE);
    return T;
}
Application &Application::stateProgress(Flt progress) {
    if (TaskbarList) {
        TaskbarList->SetProgressState(window(), TBPF_NORMAL);
        TaskbarList->SetProgressValue(window(), RoundU(Sat(progress) * 65536), 65536);
    }
    return T;
}
Application &Application::statePaused(Flt progress) {
    if (TaskbarList) {
        TaskbarList->SetProgressState(window(), TBPF_PAUSED);
        TaskbarList->SetProgressValue(window(), RoundU(Sat(progress) * 65536), 65536);
    }
    return T;
}
Application &Application::stateError(Flt progress) {
    if (TaskbarList) {
        TaskbarList->SetProgressState(window(), TBPF_ERROR);
        TaskbarList->SetProgressValue(window(), RoundU(Sat(progress) * 65536), 65536);
    }
    return T;
}
#else
// TODO: WINDOWS_NEW TaskBar Progress - check this in the future as right now this is not available in UWP
Application &Application::stateNormal() { return T; }
Application &Application::stateWorking() { return T; }
Application &Application::stateProgress(Flt progress) { return T; }
Application &Application::statePaused(Flt progress) { return T; }
Application &Application::stateError(Flt progress) { return T; }
#endif
/******************************************************************************/
Bool Application::hidden() C {
#if WINDOWS_OLD
    return !IsWindowVisible(window());
#elif MAC
    return NSApp.hidden;
#else
    return false;
#endif
}
Application &Application::hide() {
#if WINDOWS_OLD
    ShowWindow(window(), SW_HIDE);
#elif MAC
    [NSApp hide:NSApp];
#elif LINUX
    if (XDisplay && window())
        XUnmapWindow(XDisplay, window());
#elif ANDROID
    if (AndroidApp && AndroidApp->activity)
        ANativeActivity_finish(AndroidApp->activity);
#endif
    return T;
}
Application &Application::show(Bool activate) {
#if WINDOWS_OLD
    ShowWindow(window(), activate ? SW_SHOW : SW_SHOWNA);
#elif MAC
    if (activate)
        [NSApp unhide:NSApp];
    else
        [NSApp unhideWithoutActivation];
#elif LINUX
    if (XDisplay && window()) {
        XMapWindow(XDisplay, window());
        if (activate)
            window().activate();
    }
#endif
    return T;
}
/******************************************************************************/
Application &Application::icon(C Image &icon) {
#if WINDOWS_OLD
    HICON hicon = CreateIcon(icon);
    if (window()) {
        SendMessage(window(), WM_SETICON, ICON_BIG, (LPARAM)hicon);
        SendMessage(window(), WM_SETICON, ICON_SMALL, (LPARAM)hicon);
    }
    if (_icon)
        DestroyIcon(_icon);
    _icon = hicon;
#elif LINUX
    if (XDisplay && window() && _NET_WM_ICON) {
        Image temp;
        C Image *src = (icon.is() ? &icon : null);
        if (src && src->compressed())
            if (src->copy(temp, -1, -1, 1, IMAGE_B8G8R8A8_SRGB, IMAGE_SOFT, 1))
                src = &temp;
            else
                src = null;
        if (src && src->is() && src->lockRead()) {
            Memt<long> data;
            data.setNum(2 + src->w() * src->h());
            data[0] = src->w();
            data[1] = src->h();
            FREPD(y, src->h())
            FREPD(x, src->w()) {
                Color col = src->color(x, y);
                VecB4 c(col.b, col.g, col.r, col.a);
                data[2 + x + y * src->w()] = c.u;
            }
            XChangeProperty(XDisplay, window(), _NET_WM_ICON, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)data.data(), data.elms());
            src->unlock();
        } else {
            XDeleteProperty(XDisplay, window(), _NET_WM_ICON);
        }
        XFlush(XDisplay);
        _icon.del(); // delete at end in case it's 'icon'
    } else {
        // remember it so it will be set later
        icon.copy(_icon, -1, -1, 1, IMAGE_B8G8R8A8_SRGB, IMAGE_SOFT, 1);
    }
#endif
    return T;
}
/******************************************************************************/
#if WINDOWS
HMONITOR Application::hmonitor() C {
#if WINDOWS_OLD
    return MonitorFromWindow(window(), MONITOR_DEFAULTTONULL);
#else
    HMONITOR monitor = null;
    if (SwapChain) {
        IDXGIOutput *output = null;
        SwapChain->GetContainingOutput(&output);
        if (output) {
            DXGI_OUTPUT_DESC desc;
            if (OK(output->GetDesc(&desc)))
                monitor = desc.Monitor;
            output->Release();
        }
    }
    return monitor;
#endif
}
#endif
/******************************************************************************/
Application &Application::lang(LANG_TYPE lang) {
    if (T._lang != lang) {
        T._lang = lang;
        Gui.setText();
    }
    return T;
}
/******************************************************************************/
#if WINDOWS_NEW
static Bool StayAwake;
static Windows::System::Display::DisplayRequest ^ DisplayRequest; // need to be as pointer, because crash happens if constructor is called on app startup. can't be created inside 'stayAwake' because crash will happen if that was called on secondary thread, must be created on main thread
#elif MAC
static Bool AssertionIDValid;
static IOPMAssertionID AssertionID;
#endif
#if !SWITCH
static void SetStayAwake() { App.setStayAwake(); }
void Application::setStayAwake() {
#if IOS // can be called only on the main thread
    if (!App.mainThread()) {
        App.includeFuncCall(SetStayAwake);
        return;
    }
#endif

    auto mode = _stay_awake;

#if DESKTOP
    if (mode == AWAKE_SCREEN)                                        // if we want to keep the screen on
        if (!(active() || FlagOn(App.flag, APP_WORK_IN_BACKGROUND))) // however the app is not focused
            mode = AWAKE_OFF;                                        // then disable staying awake
#endif

#if WINDOWS_OLD
    SetThreadExecutionState(ES_CONTINUOUS | ((mode == AWAKE_OFF) ? 0 : (mode == AWAKE_SCREEN) ? ES_DISPLAY_REQUIRED
                                                                                              : ES_SYSTEM_REQUIRED));
#elif WINDOWS_NEW
    if (StayAwake != (mode != AWAKE_OFF)) // !! CALLS TO 'DisplayRequest' ARE CUMULATIVE, SO CALL ONLY ON CHANGE !! - https://docs.microsoft.com/en-us/uwp/api/windows.system.display.displayrequest
        if (DisplayRequest) {
            try // need try/catch because can throw
            {   // can be called on secondary threads
                if (StayAwake)
                    DisplayRequest->RequestRelease(); // here 'StayAwake' means current state (before change)
                else
                    DisplayRequest->RequestActive();
                StayAwake ^= 1; // !! CHANGE AFTER CALL !! because if failed then exception is thrown and this is not called
            } catch (...) {
            }
        }
#elif MAC
    if (AssertionIDValid) {
        IOPMAssertionRelease(AssertionID);
        AssertionIDValid = false;
    } // release current
    if (mode && IOPMAssertionCreateWithName((mode == AWAKE_SCREEN) ? kIOPMAssertionTypeNoDisplaySleep : kIOPMAssertionTypeNoIdleSleep, kIOPMAssertionLevelOn, CFSTR("Busy"), &AssertionID) == kIOReturnSuccess)
        AssertionIDValid = true;
#elif ANDROID
    if (mode == AWAKE_SYSTEM)                                        // if we want to keep the system on
        if (!(active() || FlagOn(App.flag, APP_WORK_IN_BACKGROUND))) // however the app is not focused
            mode = AWAKE_OFF;                                        // then disable staying awake
    JNI jni;
    if (jni && ActivityClass)
        if (JMethodID stayAwake = jni.staticFunc(ActivityClass, "stayAwake", "(I)V"))
            jni->CallStaticVoidMethod(ActivityClass, stayAwake, jint(mode));
#elif IOS
    [UIApplication sharedApplication].idleTimerDisabled = (mode != AWAKE_OFF);
#elif LINUX
    // TODO: add 'stayAwake' support for Linux
#endif
}
#endif
Application &Application::stayAwake(AWAKE_MODE mode) {
    if (_stay_awake != mode) {
        _stay_awake = mode;
        setStayAwake();
    }
    return T;
}
/******************************************************************************/
void Application::activeOrBackFullChanged() {
    if (D.full()) // full screen
    {
#if WINDOWS_OLD
        if (D.exclusiveFull()) {
#if DX11
            if (SwapChain) {
                SyncLocker locker(D._lock);
                if (activeOrBackFull()) {
                    SwapChain->SetFullscreenState(true, null);
                    SwapChain->ResizeTarget(&SwapChainDesc.BufferDesc);
                } else {
                    window().minimize(true);
                    SwapChain->SetFullscreenState(false, null);
                }
            }
#endif
        } else // non-exclusive
        {
            if (activeOrBackFull()) {
                SetDisplayMode();
                window().reset(true);
            } else {
                window().minimize(true);
                SetDisplayMode();
            }
        }
#elif MAC
        if (!activeOrBackFull())
            hide();
        SetDisplayMode();
#elif LINUX
        if (!activeOrBackFull())
            window().minimize();
        SetDisplayMode();
#endif
    }
}
void Application::setActive(Bool active) {
    if (T.active() != active) {
        Bool active_or_back_full = activeOrBackFull();
        T._active = active;
        if (active_or_back_full != activeOrBackFull())
            activeOrBackFullChanged();

        Time.skipUpdate();
        InputDevices.acquire(active);

        if (_initialized) {
            if (active) {
                if (T.resumed)
                    T.resumed();
                D.getScreenInfo();
                D.setColorLUT();
            } // reset color LUT on activate in case it was changed in the system
            else {
                if (T.paused)
                    T.paused();
            }
        }
#if DESKTOP || ANDROID // also on Android in case a new Activity/Window was created, call this after potential 'paused/resumed' in case user modifies APP_WORK_IN_BACKGROUND which affect 'stayAwake'
        if (_stay_awake)
            setStayAwake(); // reset sleeping when app gets de/activated
#endif
        _initialized = true;
#if IOS
        if (EAGLView *view = GetUIView())
            [view setUpdate];
#endif
    }
}
Application &Application::backgroundFull(Bool on) {
    if (T._back_full != on) {
        Bool active_or_back_full = activeOrBackFull();
        T._back_full = on;
        if (D.full() && _initialized) {
            if (active_or_back_full != activeOrBackFull())
                activeOrBackFullChanged();
            windowAdjust();
        }
    }
    return T;
}
void Application::close() {
    if (quit)
        quit();
    else
        _close = true;
}
/******************************************************************************/
void Application::pickPhoto(Int max_files) {
    if (max_files > 0) {
#if ANDROID
        if (Jni && ActivityClass && Activity)
            if (JMethodID pickPhoto = Jni.func(ActivityClass, "pickPhoto", "(I)V"))
                Jni->CallVoidMethod(Activity, pickPhoto, jint(max_files));
#elif IOS
        if (ViewController) {
            if ([PHPickerConfiguration class]) // if class was found, this is needed because PHPickerConfiguration is available on iOS 14.0+
            {
                PHPickerConfiguration *config = [[PHPickerConfiguration alloc] init];
                config.selectionLimit = max_files;
                config.filter = [PHPickerFilter imagesFilter];

                PHPickerViewController *picker = [[PHPickerViewController alloc] initWithConfiguration:config];
                [config release];
                picker.delegate = GetAppDelegate();
                [ViewController presentViewController:picker animated:YES completion:nil];
            } else {
                UIImagePickerController *picker = [[UIImagePickerController alloc] init];
                picker.delegate = GetAppDelegate();
                picker.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;

                [ViewController presentViewController:picker animated:YES completion:nil];
            }
        }
#endif
    }
}
/******************************************************************************/
Bool Application::testInstance() {
    if (flag & APP_EXIT_IMMEDIATELY)
        return false;

    if (flag & (APP_ON_RUN_EXIT | APP_ON_RUN_WAIT)) {
        Memt<UInt> id;
        ProcList(id);
        REPA(id)
        if (processID() != id[i] && StartsPath(ProcName(id[i]), exe())) // use 'StartsPath' because on Mac 'ProcName' returns the file inside APP folder
        {
            if (flag & APP_ON_RUN_WAIT)
                ProcWait(id[i]);
            else {
                if (auto window = ProcWindow(id[i]))
                    window.activate();
                return false;
            }
        }
    }
    return true;
}
void Application::deleteSelf() {
    if (_del_self_at_exit) {
#if WINDOWS_OLD
        Str base = _GetBase(exe());
#if 0 // this won't work for various reasons, possibly 'createMem' does not support labels and goto (because each command is executed separately), and possibly because 'ConsoleProcess' is a child process which will get closed if the parent (the executable) gets closed, however the child can only delete the file once the parent process is no longer running
         ConsoleProcess cp;
         cp.createMem(S+":Repeat\n"
                       +"del \""     +base+"\" >> NUL\n"       // delete EXE
                       +"if exist \""+base+"\" goto Repeat", // if exists then try again
                      GetPath(exe()));
#else
        if (HasUnicode(base)) {
            if (!App.renameSelf(GetPath(exe()) + '\\' + FFirst("temp ", "tmp")))
                return;
            base = _GetBase(exe());
        }
        Str bat = S + exe() + ".bat";
        FileText f;
        if (f.write(bat, ANSI)) // BAT doesn't support wide chars
        {
            f.putText(S + ":Repeat\n" + "del \"" + base + "\" >> NUL\n"                                     // delete EXE
                      + "if exist \"" + base + "\" goto Repeat\n" + "del \"" + GetBase(bat) + "\" >> NUL"); // delete BAT
            f.del();
            Run(bat, S, true);
        }
#endif
#elif MAC
        FDelDirs(exe()); // under Mac OS application is not an "exe" file, but a folder with "app" extension
#elif LINUX
        FDelFile(exe());
#endif
    }
}
Bool Application::renameSelf(C Str &dest) {
    if (FRename(_exe, dest)) {
        _exe = dest;
        return true;
    }
    return false;
}
void Application::deleteSelfAtExit() {
    _del_self_at_exit = true;
}
void Application::detectMemLeaks() {
#if DEBUG
    if ((flag & APP_MEM_LEAKS) && _mem_leaks) {
#if WINDOWS
        //_cexit();
        if (Int m = Abs(_mem_leaks)) {
            ListMemLeaks();

            // Log the number of memory leaks detected
            GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
                LogLevel::ERRORING, __FILE__, __LINE__,
                std::string("Memory leaks detected: ") + std::to_string(m));

            showError(S + m + " Memory Leak(s)");
        }
        //_exit(-1); // manual exit after cleaning with '_cexit'
#elif 0 && MAC
        LogN("Application Memory Leaks Remaining:");
        // GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
        //     LogLevel::ERRORING, __FILE__, __LINE__,
        //    "Application Memory Leaks Remaining:");
#endif
    }
#endif
}
#if WINDOWS_NEW
ref struct Exiter sealed {
    Exiter(Platform::String ^ title, Platform::String ^ error) {
        if (auto dialog = ref new Windows::UI::Popups::MessageDialog(error, title)) {
            App._closed = true; // disable callback processing
            dialog->Commands->Append(ref new Windows::UI::Popups::UICommand("OK", ref new Windows::UI::Popups::UICommandInvokedHandler(this, &Exiter::onOK)));
            dialog->ShowAsync();
            // try // this can crash if app didn't finish initializing, however using try/catch didn't help
            {
                Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessUntilQuit);
            }
            // catch(...){ExitNow();}
        }
    }
    void onOK(Windows::UI::Popups::IUICommand ^ command) {
        Windows::ApplicationModel::Core::CoreApplication::Exit();
    }
};
#elif ANDROID
extern "C" JNIEXPORT void JNICALL Java_com_esenthel_Native_closedError(JNIEnv *env, jclass clazz) { ExitNow(); }
#endif
#if !SWITCH
void Application::showError(CChar *error) {
    if (Is(error)) {
        if (D.full() && App.window()) {
            if (!ANDROID)
                hide();
#if DX11 // hiding window on DX10+ is not enough, try disabling Fullscreen
         // ChangeDisplaySettings(null, 0); this didn't help
            if (SwapChain
#if WINDOWS_OLD
                && D.exclusiveFull()
#endif
            ) {
                if (App.mainThread()) // we can make call to 'SetFullscreenState' only on the main thread, calling it on others made no difference
                {
                    SyncLocker locker(D._lock); // we should always be able to lock on the main thread
                    if (SwapChain)
                        SwapChain->SetFullscreenState(false, null);
                } else
                    App._close = true; // request the main thread to close the app (but don't call 'App.close' because that would call quit)
            }
#endif
        }
        CChar *title = TError();

#if WINDOWS
        OutputDebugString(WChar(error));
        OutputDebugStringA("\n"); // first write to console
#endif
#if WINDOWS_OLD
        // SetCursor(LoadCursor(null, IDC_ARROW)); // reset cursor first, in case app had it disabled, actually this didn't help
        ClipCursor(null);                                                                // disable cursor clipping first
        MessageBox(null, WChar(error), WChar(title), MB_OK | MB_TOPMOST | MB_ICONERROR); // use OS 'MessageBox' instead of EE 'OSMsgBox' to avoid memory allocation when creating Str objects, because this may be called when out of memory
#elif WINDOWS_NEW
        Exiter(ref new Platform::String(WChar(title)), ref new Platform::String(WChar(error)));
#elif LINUX // on Linux additionally display the error in the console in case the OS doesn't support displaying messages boxes
        fputs(UTF8(error), stdout);
        fputs("\n", stdout);
        fflush(stdout); // without the flush, messages won't be displayed immediately
        OSMsgBox(title, error, true);
#elif ANDROID
        // first write to console, use '__android_log_write' with 'ANDROID_LOG_ERROR' instead of 'Log' which uses 'ANDROID_LOG_INFO'
        Memc<Str> lines;
        Split(lines, error, '\n'); // android has limit for too long messages
        FREPA(lines) {
            Str8 line = UTF8(lines[i]);
            if (line.is())
                __android_log_write(ANDROID_LOG_ERROR, ENGINE_NAME, line.is() ? line : " ");
        } // '__android_log_write' will crash if text is null or ""

#if 0 // output the error into root of SD Card so users can send the text file
      Str path=SystemPath(SP_SD_CARD); if(!path.is())path=SystemPath(SP_APP_DATA_PUBLIC); if(!path.is())path=SystemPath(SP_APP_DATA);
      FileText f; if(f.append(path.tailSlash(true)+ENGINE_NAME " Error.txt"))f.putText(error);
#endif

        if (ActivityClass) {
            if (!minimized()) // if app is not minimized then we can show message box, this works OK even when app is starting (InitPre)
            {
                JNI jni;
                if (jni && ActivityClass)
                    if (JMethodID messageBox = jni.staticFunc(ActivityClass, "messageBox", "(Ljava/lang/String;Ljava/lang/String;Z)V"))
                        if (JString ti = JString(jni, title))
                            if (JString te = JString(jni, error))
                                jni->CallStaticVoidMethod(ActivityClass, messageBox, ti(), te(), jboolean(true));

                if (App.mainThread()) {
                    // remove callbacks
                    AndroidApp->onAppCmd = null;
                    AndroidApp->onInputEvent = null;

                    android_poll_source *source;
                    for (; !AndroidApp->destroyRequested && ALooper_pollAll(-1, null, null, (void **)&source) >= 0;) // since the message box is only queued, we need to wait until it's actually displayed, need to check for 'destroyRequested' as well, in case the system decided to close the app before 'closedError' got called
                    {
                        if (source)
                            source->process(AndroidApp, source); // process this event
                        Time.wait(1);
                    }
                } else
                    for (; !AndroidApp->destroyRequested;)
                        Time.wait(1);
            } else // can't display a message box if app is minimized, so display a toast instead
            {
                Str message = S + App.name() + " exited";
                if (Is(error))
                    message.line() += error;
                OSToast(message);
                Time.wait(4000); // wait 4 seconds because toast will disappear as soon as we crash
            }
        }
#elif IOS
        if (NSStringAuto str = error)
            NSLog(@"%@", str()); // first write to console

        // display message box
        if (NSString *ns_title = AppleString(title)) // have to use 'AppleString' because it will get copied in the local function below
        {
            if (NSString *ns_text = AppleString(error)) // have to use 'AppleString' because it will get copied in the local function below
            {
                dispatch_async(dispatch_get_main_queue(), ^{ // this is needed in case we're calling from a secondary thread
                  if (UIAlertController *alert_controller = [UIAlertController alertControllerWithTitle:ns_title message:ns_text preferredStyle:UIAlertControllerStyleAlert]) {
                      [alert_controller addAction:[UIAlertAction actionWithTitle:@"OK"
                                                                           style:UIAlertActionStyleDefault
                                                                         handler:^(UIAlertAction *action) {
                                                                           ExitNow();
                                                                         }]];
                      [[[[UIApplication sharedApplication] keyWindow] rootViewController] presentViewController:alert_controller animated:YES completion:nil];
                      //[alert_controller release]; release will crash
                  }
                });
                [ns_text release];
            }
            [ns_title release];
        }

        _closed = true;
        if (EAGLView *view = GetUIView())
            [view setUpdate]; // disable callback processing and stop updating
        [[NSRunLoop mainRunLoop] run];
#elif WEB // on Web display the error as both console output and message box
        fputs(UTF8(error), stdout); // first write to console
        OSMsgBox(title, error, true);
#else
        OSMsgBox(title, error, true);
#endif
    }
}
#endif
void Application::lowMemory() {
    // call this first before releasing caches
    if (low_memory)
        low_memory();

    // release memory from caches (process in order from parents to base elements)
    DelayRemoveNow();
    Environments.delayRemoveNow();
    Objects.delayRemoveNow();
    Meshes.delayRemoveNow();
    PhysBodies.delayRemoveNow();
    WaterMtrls.delayRemoveNow();
    Materials.delayRemoveNow();
    GuiSkins.delayRemoveNow();
    Panels.delayRemoveNow();
    PanelImages.delayRemoveNow();
    TextStyles.delayRemoveNow();
    Fonts.delayRemoveNow();
    ImageAtlases.delayRemoveNow();
    Images.delayRemoveNow();
}
/******************************************************************************/
void Application::windowAdjust(Bool set) {
    RectI full, work;
    VecI2 max_normal_win_client_size, maximized_win_client_size;
    D.getMonitor(full, work, max_normal_win_client_size, maximized_win_client_size);

#if DEBUG && 0
    LogN(S + "full:" + full.asText() + ", work:" + work.asText() + ", App._window_pos:" + _window_pos + ", D.res:" + D.res());
#endif

#if WINDOWS_OLD
    if (D.full()) // fullscreen
    {
        SetWindowLong(window(), GWL_STYLE, _style_full);
        SetWindowPos(window(), (backgroundFull() && !D.exclusiveFull()) ? HWND_NOTOPMOST : HWND_TOPMOST, full.min.x, full.min.y, D.resW(), D.resH(), 0);
    } else if (D.resW() >= maximized_win_client_size.x && D.resH() >= maximized_win_client_size.y) // maximized
    {
        SetWindowLong(window(), GWL_STYLE, _style_window_maximized);
#if 0 // this doesn't work as expected
      SetWindowPos (window(), HWND_TOP , work.min.x+_bound_maximized.min.x, work.min.y-_bound_maximized.max.y, D.resW()+_bound_maximized.w(), D.resH()+_bound_maximized.h(), SWP_NOACTIVATE);
#else
        SetWindowPos(window(), HWND_TOP, work.min.x + _bound_maximized.min.x, work.min.y - _bound_maximized.max.y, D.resW() + _bound_maximized.max.x, D.resH() - _bound_maximized.min.y, SWP_NOACTIVATE);
#endif
    } else // normal window
    {
        Int w = D.resW() + _bound.w(),
            h = D.resH() + _bound.h();

        if (_window_pos.x == INT_MAX) {
            if (x <= -1)
                _window_pos.x = work.min.x;
            else if (!x)
                _window_pos.x = work.centerXI() - w / 2;
            else
                _window_pos.x = work.max.x - w + _bound.max.x - 1;
        }
        if (_window_pos.y == INT_MAX) {
            if (y >= 1)
                _window_pos.y = work.min.y;
            else if (!y)
                _window_pos.y = work.centerYI() - h / 2;
            else
                _window_pos.y = work.max.y - h + _bound.max.y - 1;
        }

        // make sure the window is not completely outside of working area
        const Int b = 32;
        Int r = b;
        if (!(flag & APP_NO_TITLE_BAR)) // has bar
        {
            Int size = GetSystemMetrics(SM_CXSIZE); // TODO: this should be OK because we're DPI-Aware, however it doesn't work OK
                                                    /*if(HDC hdc=GetDC(window()))
                                                      {
                                                         size=DivCeil(size*GetDeviceCaps(hdc, LOGPIXELSX), 96);
                                                         ReleaseDC(window(), hdc);
                                                      }*/
            if (!(flag & APP_NO_CLOSE))
                r += size; // has close                button
            if (flag & (APP_MINIMIZABLE | APP_MAXIMIZABLE))
                r += size * 2; // has minimize or maximize button (if any is enabled, then both will appear)
        }

        if (_window_pos.x + b > work.max.x)
            _window_pos.x = Max(work.min.x, work.max.x - b);
        else {
            Int p = _window_pos.x + w;
            if (p - r < work.min.x)
                _window_pos.x = Min(work.min.x + r, work.max.x) - w;
        }
        if (_window_pos.y + b > work.max.y)
            _window_pos.y = Max(work.min.y, work.max.y - b);
        else {
            Int p = _window_pos.y + h;
            if (p - b < work.min.y)
                _window_pos.y = Min(work.min.y + b, work.max.y) - h;
        }

        SetWindowLong(window(), GWL_STYLE, _style_window);
        SetWindowPos(window(), HWND_NOTOPMOST, _window_pos.x, _window_pos.y, w, h, SWP_NOACTIVATE);
    }
#elif MAC
    if (D.full()) // fullscreen
    {
        if (!(flag & APP_NO_TITLE_BAR))
            [window() setStyleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskFullSizeContentView]; // need to toggle this only if window wants to have title bar, if it doesn't then we don't need to change anything. But if window wants title bar, then we can't just disable it here and use NSBorderlessWindowMask, because that will make content view (OpenGLView) disappear (some bug in Mac OS?), so have to use NSWindowStyleMaskTitled|NSWindowStyleMaskFullSizeContentView which will make content overlap title bar, and in fullscreen title bar will be hidden
        window().size(D.resW(), D.resH(), true);
        window().pos(0, 0);
    } else {
        if (_window_pos.x == INT_MAX) {
            Int w = D.resW();
            if (x <= -1)
                _window_pos.x = work.min.x;
            else if (!x)
                _window_pos.x = work.centerXI() - w / 2;
            else
                _window_pos.x = work.max.x - w + _bound.max.x - 1;
        }
        if (_window_pos.y == INT_MAX) {
            Int h = D.resH();
            if (y >= 1)
                _window_pos.y = work.min.y;
            else if (!y)
                _window_pos.y = work.centerYI() - h / 2;
            else
                _window_pos.y = work.max.y - h + _bound.max.y - 1;
        }

        if (!(flag & APP_NO_TITLE_BAR))
            [window() setStyleMask:_style_window];
        window().size(D.resW(), D.resH(), true);
        window().pos(_window_pos.x, _window_pos.y);
    }
#elif LINUX
    if (window()) {
        // set window fullscreen state
        {
            // setting fullscreen mode will fail if window is not resizable, so force it to be just for this operation
            Bool set_resizable = (D.full() && !(flag & APP_RESIZABLE));
            if (set_resizable)
                setWindowFlags(true);

#define _NET_WM_STATE_REMOVE 0
#define _NET_WM_STATE_ADD 1
#define _NET_WM_STATE_TOGGLE 2
            XEvent e;
            Zero(e);
            e.xclient.type = ClientMessage;
            e.xclient.window = window();
            e.xclient.message_type = _NET_WM_STATE;
            e.xclient.format = 32;
            e.xclient.data.l[0] = (D.full() ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE);
            e.xclient.data.l[1] = _NET_WM_STATE_FULLSCREEN;
            e.xclient.data.l[2] = 0;
            e.xclient.data.l[3] = 1;
            XSendEvent(XDisplay, DefaultRootWindow(XDisplay), false, SubstructureRedirectMask | SubstructureNotifyMask, &e);
            XSync(XDisplay, false);

            if (set_resizable)
                setWindowFlags();
        }
        // set window size
        if (!D.full() && !set)
            window().size(D.resW(), D.resH(), true); // don't resize Window on Linux when changing mode due to 'set' (when window got resized due to OS/User input instead of calling 'D.mode', because there the window is already resized and calling this would cause window jumping)
    }
#endif
}
/******************************************************************************/
static RectI GetDesktopArea() {
    RectI recti(0, 0, App.desktopW(), App.desktopH());
#if WINDOWS_OLD
    RECT rect;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
    recti.set(rect.left, rect.top, rect.right, rect.bottom);
#elif LINUX
    if (XDisplay)
        if (Atom FIND_ATOM(_NET_WORKAREA)) {
            Atom type = NULL;
            int format = 0;
            unsigned long items = 0, bytes_after = 0;
            unsigned char *data = null;
            if (!XGetWindowProperty(XDisplay, DefaultRootWindow(XDisplay), _NET_WORKAREA, 0, 16, 0, XA_CARDINAL, &type, &format, &items, &bytes_after, &data))
                if (long *l = (long *)data)
                    if (items >= 4) {
                        long left = l[0],
                             top = l[1],
                             width = l[2],
                             height = l[3];
                        recti.set(left, top, left + width, top + height);
                    }
            if (data)
                XFree(data);
        }
#elif MAC
    NSRect rect = [[NSScreen mainScreen] visibleFrame];
    recti.min.x = Round(rect.origin.x);
    recti.max.x = recti.min.x + Round(rect.size.width);
    recti.max.y = App.desktopH() - Round(rect.origin.y);
    recti.min.y = recti.max.y - Round(rect.size.height);
#elif WEB
    // it's not possible to get correct results, because on Chrome: this value is adjusted by "System DPI/Scaling", but not 'D.browserZoom', and does not change when zooming. Because "System DPI/Scaling" is unknown, it can't be calculated.
    recti.min.set(JavaScriptRunI("screen.availLeft"), JavaScriptRunI("screen.availTop"));
    recti.max.set(JavaScriptRunI("screen.availWidth"), JavaScriptRunI("screen.availHeight")) += recti.min;
#endif
    return recti;
}
static Str GetAppPathName() {
#if WINDOWS
    wchar_t module[MAX_LONG_PATH];
    GetModuleFileName(null, module, Elms(module));
    return module;
#elif APPLE
    Str app;
    if (CFBundleRef bundle = CFBundleGetMainBundle()) {
        if (CFURLRef url = CFBundleCopyBundleURL(bundle)) {
            Char8 url_path[MAX_UTF_PATH];
            CFURLGetFileSystemRepresentation(url, true, (UInt8 *)url_path, Elms(url_path));
            app = FromUTF8(url_path);
            CFRelease(url);
        }
        CFRelease(bundle);
    }
    return app;
#elif LINUX
    char path[MAX_UTF_PATH];
    path[0] = '\0';
    ssize_t r = readlink("/proc/self/exe", path, SIZE(path));
    if (InRange(r, path))
        path[r] = '\0';
    return FromUTF8(path);
#elif ANDROID
    return AndroidAppPath; // obtained externally
#else
    return S;
#endif
}
static Bool GetProcessElevation() {
#if WINDOWS_OLD
    Bool elevated = false;
    HANDLE token = null;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elevation;
        DWORD size;
        if (GetTokenInformation(token, TokenElevation, &elevation, SIZE(elevation), &size)) // elevation supported (>= Windows Vista)
        {
            elevated = (elevation.TokenIsElevated != 0);
        } else // elevation not supported (< Windows Vista)
        {
            elevated = true;
        }
    }

    if (token) {
        CloseHandle(token);
        token = null;
    }

    return elevated;
#elif WINDOWS_NEW
    return false;
#else
    return true;
#endif
}
/******************************************************************************/
#if WINDOWS_OLD
static BOOL CALLBACK EnumResources(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR user) {
    if (HRSRC resource = FindResource(hModule, lpName, lpType)) {
        Paks.addMem(LockResource(LoadResource(null, resource)), SizeofResource(null, resource), (Cipher *)user, false);
        return true;
    }
    Exit(MLTC(u"Can't load resource data from exe", PL, u"Nie można wczytać danych z pliku exe"));
    return false;
}
#endif
void LoadEmbeddedPaks(Cipher *cipher) {
#if WINDOWS_OLD
    EnumResourceNames(App._hinstance, L"PAK", EnumResources, IntPtr(cipher)); // iterate through all "PAK" resources embedded in .exe file
    SetLastError(0);                                                          // clear error 1813 of not found resource type
#elif MAC
    for (FileFind ff(App.exe() + "/Contents/Resources", "pak"); ff();)
        Paks.add(ff.pathName(), cipher, false); // iterate all PAK files inside APP resources folder
#elif LINUX
    File f;
    if (f.readStd(App.exe()))
        for (Long next = f.size(); f.pos(next - 2 * 4);) {
            Long skip = f.getUInt(); // !! use Long and not UInt, because of "-skip" below, which would cause incorrect behavior
            UInt end = f.getUInt();
#define CC4_CHNK CC4('C', 'H', 'N', 'K')
            if (end == CC4_CHNK && f.skip(-skip)) {
                next = f.pos();
                if (f.getUInt() == CC4_CHNK) {
                    UInt size = f.getUInt();
                    if (size + 4 * 4 == skip) // 2xCHNK + 2xSIZE = 4 TOTAL
                    {
                        ULong total_size, applied_offset;
                        f.limit(total_size, applied_offset, size);
                        switch (f.getUInt()) {
                        case CC4('P', 'A', 'K', 0): {
                            Paks.addTry(App.exe(), cipher, false, f.posAbs()); // use 'addTry' unlike on other platforms, because this could be not EE data (very unlikely)
                        } break;

                        case CC4('I', 'C', 'O', 'N'): {
                            Image icon;
                            if (icon.Import(f, -1, IMAGE_SOFT, 1))
                                if (icon.is())
                                    App.icon(icon);
                        } break;
                        }
                        f.unlimit(total_size, applied_offset);
                        continue; // try next chunk
                    }
                }
            }
            break;
        }
#endif
    Paks.rebuild();
}
#if !ANDROID
void LoadAndroidAssetPacks(Int asset_packs, Cipher *cipher) {}
#endif
/******************************************************************************/
Bool Application::create0() {
#if WINDOWS_OLD
    ShutCOM = OK(CoInitialize(null)); // required by: creating shortctuts - IShellLink, Unicode IME support, ITaskbarList3, Visual Studio Installation detection - SetupConfiguration, SHOpenFolderAndSelectItems, anything that calls 'CoCreateInstance'
    CoCreateInstance(CLSID_TaskbarList, null, CLSCTX_ALL, IID_ITaskbarList3, (Ptr *)&TaskbarList);
    SetLastError(0); // clear error 2
    TouchesSupported = (GetSystemMetrics(SM_MAXIMUMTOUCHES) > 0);
#elif WINDOWS_NEW
    TouchesSupported = (Windows::Devices::Input::TouchCapabilities().TouchPresent > 0);
    try {
        DisplayRequest = ref new Windows::System::Display::DisplayRequest;
    } catch (...) {
    }                          // need try/catch because can throw
#elif MAC
    [MyApplication sharedApplication]; // this allocates 'NSApp' application as our custom class
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp finishLaunching];
    [NSApp setDelegate:[[MyAppDelegate alloc] init]]; // don't release delegate, instead it's released in 'MyApplication' dealloc
#elif LINUX
    XInitThreads();
    XDisplay = XOpenDisplay(null);
    OldErrorHandler = XSetErrorHandler(ErrorHandler); // set custom error handler, since I've noticed that occasionally BadWindow errors get generated, so just ignore them
    if (XDisplay) {
        FIND_ATOM(_NET_WM_STATE);
        FIND_ATOM(_NET_WM_STATE_FULLSCREEN);
        FIND_ATOM(_NET_WM_STATE_DEMANDS_ATTENTION);
        FIND_ATOM(_NET_WM_ICON);
        FIND_ATOM(_NET_WM_NAME);
        FIND_ATOM(UTF8_STRING);
    }
#endif

    T._thread_id = GetThreadID(); // !! adjust the thread ID here, because on WINDOWS_NEW it will be a different value !! no need to reset 'threadPriority' 'threadMask' because 'InitPre' was not yet called
    T._thread_priority ^= 1;
    threadPriority(T._thread_priority ^ 1); // force-set
    T._elevated = GetProcessElevation();
    T._process_id = PLATFORM(GetCurrentProcessId(), getpid());
    T._desktop_size = D.screen();
    T._desktop_area = GetDesktopArea(); // !! call after getting '_desktop_size' !!
    T._exe = GetAppPathName();
    T._lang = OSLanguage();

    Time.create(); // set first, to start measuring init time
    InitHash();
    InitMisc();
    InitIO(); // init IO early in case we want to output logs to files
    InitMesh();
    InitSRGB();
    InitSocket();
    InitStream();
    InitState();
    Kb.init();
    D.init();
#if WEB
    InitSound(); // on WEB init sound before the 'Preload' and 'InitPre' so we can play some music while waiting
#endif

    return true;
}
Bool Application::create1() {
    if (LogInit)
        LogN("InitPre");
    InitPre();
#if LINUX
    if (!XDisplay && !(flag & APP_ALLOW_NO_XDISPLAY))
        Exit("Can't open XDisplay");
#endif
    if (!testInstance())
        return false;
    windowCreate();
    InitSound();
    if (!D.create())
        return false;
    InputDevices.create(); // create input devices after display, because mouse might use 'D.res' for initial position, and some sensors are set based on screen refresh rate
#if WINDOWS_OLD
    if (!(flag & APP_HIDDEN) && hidden())
        show(true);      // if we don't want window hidden, but it is (for example due to WS_EX_NOREDIRECTIONBITMAP) then show it
    window().activate(); // manually activate because on Windows if application is loading for a long time, then it might lose focus
#endif
#if ANDROID
    if (_stay_awake)
        setStayAwake(); // on Android we need to apply this after window was created
#endif
    if (LogInit)
        LogN("Init");
    if (!Init())
        return false;
    return true;
}
Bool Application::create() {
    return create0() && create1();
}
static void FadeOut() {
#if DESKTOP
    if (App.flag & APP_FADE_OUT) {
        Bool fade_sound = PlayingAnySound(), fade_window = !D.full();
        Flt alpha = App.opacity();
#if WINDOWS_OLD
        ANIMATIONINFO ai;
        ai.cbSize = SIZE(ai);
        SystemParametersInfo(SPI_GETANIMATION, SIZE(ai), &ai, 0);
        if (ai.iMinAnimate)
            fade_window = false; // if Windows has animations enabled, then don't fade manually
#elif WINDOWS_NEW || LINUX       // WindowsNew and Linux don't support 'App.opacity'
        fade_window = false;
#endif
        if (!fade_window)
            App.hide();
        if (fade_sound || fade_window) {
            const Int step = 1;
            const Flt vol = SoundVolume.global(), length = 0.2f;
            const Dbl end_time = Time.curTime() + length - step / 1000.0f;
            for (;;) {
                Flt remaining = end_time - Time.curTime(), frac = Max(0, remaining / length);
                if (fade_sound) {
                    SoundVolume.global(vol * frac);
                    UpdateSound();
                }
                if (fade_window)
                    App.opacity(alpha * frac);
#if MAC                                                                                                                                                       // on Mac we have to update events, otherwise 'App.opacity' won't do anything. We have to do it even when not fading window (when exiting from fullscreen mode) because without it, the window will be drawn as a restored window
                for (; NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];) // 'distantPast' will not wait for any new events but return those that happened already
                    [NSApp sendEvent:event];
#endif
                if (remaining <= 0)
                    break; // check this after setting values, to make sure we will set 0
                UpdateThreads();
                Time.wait(Min(Round(remaining * 1000), step));
            }
        }
    }
#endif
}
void Application::del() {
    GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__,
        "Start del call from Application");
    { // do brackets to make sure that any temp objects created here are destroyed before 'detectMemLeaks' is called
        if (LogInit)
            LogN("ShutState");
        ShutState();
        if (LogInit)
            LogN("Shut");
        Shut();

        _initialized = false;
        setActive(false); // set '_initialized' to false to prevent from calling custom 'paused' callback in 'setActive', call 'setActive' because app will no longer be active, this is needed in case 'InputDevices.acquire' is called at a later stage, this is also needed because currently we need to disable disable magnetometer callback for WINDOWS_NEW (otherwise it will crash on Windows Phone when closing app)
        FlushIO();
        ShutStreamLoads(); // stop stream loads ASAP because we won't need them anymore
        ShutObj();
        Physics.del();
        D.del();
        FadeOut();
        ShutSound();
        ShutEnum();
        ShutAnimation();
        ShutStream();
        GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
            LogLevel::INFO, __FILE__, __LINE__,
            "Middle del call from Application");
        ShutSocket();
        windowDel();
        Paks.del();         // !! delete after deleting sound  !! because sound streaming can still use file data
        InputDevices.del(); // !! delete after deleting window !! because releasing some joypads may take some time and window would be left visible
        HideNotifications();
        GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
            LogLevel::INFO, __FILE__, __LINE__,
            "4 del call from Application");
#if WINDOWS_OLD
        RELEASE(TaskbarList);
        if (ShutCOM) {
            ShutCOM = false;
            CoUninitialize();
        } // required by 'CoInitialize'
#elif WINDOWS_NEW
        DisplayRequest = null; // release
#elif LINUX
        if (XDisplay) {
            XCloseDisplay(XDisplay);
            XDisplay = null;
        }
#elif MAC
        stayAwake(AWAKE_OFF); // on Mac disable staying awake, because we've created 'AssertionID' for it
#endif
        deleteSelf();
#if 1 // !! after 'deleteSelf' !! reduce mem leaks logging on Mac
        cmd_line.del();
        _exe.del();
        _name.del();
        _back_text.del();
#endif
    }
    GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__,
        "5 del call from Application");
    _closed = true; // !! this needs to be set before 'detectMemLeaks' because that may trigger calling destructors !!
    GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__,
        "6 del call from Application");
    detectMemLeaks();
    GlobalsLoggerInstance::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__,
        "Finish del call from Application");
}
/******************************************************************************/
void Application::update() {
    Time.update();
    InputDevices.update();
    Renderer.update();
    D.fadeUpdate();
    _callbacks.update();
    if (!(UpdateState() && DrawState()))
        _close = true;
    InputDevices.clear();
}
/******************************************************************************/
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0) {
    if (App.lang() == l0)
        return t0;
    return english;
}
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    return english;
}
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    return english;
}
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    return english;
}
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    return english;
}
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    return english;
}
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5, LANG_TYPE l6, C Str &t6) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    if (App.lang() == l6)
        return t6;
    return english;
}
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5, LANG_TYPE l6, C Str &t6, LANG_TYPE l7, C Str &t7) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    if (App.lang() == l6)
        return t6;
    if (App.lang() == l7)
        return t7;
    return english;
}
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5, LANG_TYPE l6, C Str &t6, LANG_TYPE l7, C Str &t7, LANG_TYPE l8, C Str &t8) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    if (App.lang() == l6)
        return t6;
    if (App.lang() == l7)
        return t7;
    if (App.lang() == l8)
        return t8;
    return english;
}
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5, LANG_TYPE l6, C Str &t6, LANG_TYPE l7, C Str &t7, LANG_TYPE l8, C Str &t8, LANG_TYPE l9, C Str &t9) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    if (App.lang() == l6)
        return t6;
    if (App.lang() == l7)
        return t7;
    if (App.lang() == l8)
        return t8;
    if (App.lang() == l9)
        return t9;
    return english;
}
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5, LANG_TYPE l6, C Str &t6, LANG_TYPE l7, C Str &t7, LANG_TYPE l8, C Str &t8, LANG_TYPE l9, C Str &t9, LANG_TYPE l10, C Str &t10) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    if (App.lang() == l6)
        return t6;
    if (App.lang() == l7)
        return t7;
    if (App.lang() == l8)
        return t8;
    if (App.lang() == l9)
        return t9;
    if (App.lang() == l10)
        return t10;
    return english;
}
Str MLT(C Str &english, LANG_TYPE l0, C Str &t0, LANG_TYPE l1, C Str &t1, LANG_TYPE l2, C Str &t2, LANG_TYPE l3, C Str &t3, LANG_TYPE l4, C Str &t4, LANG_TYPE l5, C Str &t5, LANG_TYPE l6, C Str &t6, LANG_TYPE l7, C Str &t7, LANG_TYPE l8, C Str &t8, LANG_TYPE l9, C Str &t9, LANG_TYPE l10, C Str &t10, LANG_TYPE l11, C Str &t11) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    if (App.lang() == l6)
        return t6;
    if (App.lang() == l7)
        return t7;
    if (App.lang() == l8)
        return t8;
    if (App.lang() == l9)
        return t9;
    if (App.lang() == l10)
        return t10;
    if (App.lang() == l11)
        return t11;
    return english;
}

CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0) {
    if (App.lang() == l0)
        return t0;
    return english;
}
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    return english;
}
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    return english;
}
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    return english;
}
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    return english;
}
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    return english;
}
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5, LANG_TYPE l6, CChar *t6) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    if (App.lang() == l6)
        return t6;
    return english;
}
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5, LANG_TYPE l6, CChar *t6, LANG_TYPE l7, CChar *t7) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    if (App.lang() == l6)
        return t6;
    if (App.lang() == l7)
        return t7;
    return english;
}
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5, LANG_TYPE l6, CChar *t6, LANG_TYPE l7, CChar *t7, LANG_TYPE l8, CChar *t8) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    if (App.lang() == l6)
        return t6;
    if (App.lang() == l7)
        return t7;
    if (App.lang() == l8)
        return t8;
    return english;
}
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5, LANG_TYPE l6, CChar *t6, LANG_TYPE l7, CChar *t7, LANG_TYPE l8, CChar *t8, LANG_TYPE l9, CChar *t9) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    if (App.lang() == l6)
        return t6;
    if (App.lang() == l7)
        return t7;
    if (App.lang() == l8)
        return t8;
    if (App.lang() == l9)
        return t9;
    return english;
}
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5, LANG_TYPE l6, CChar *t6, LANG_TYPE l7, CChar *t7, LANG_TYPE l8, CChar *t8, LANG_TYPE l9, CChar *t9, LANG_TYPE l10, CChar *t10) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    if (App.lang() == l6)
        return t6;
    if (App.lang() == l7)
        return t7;
    if (App.lang() == l8)
        return t8;
    if (App.lang() == l9)
        return t9;
    if (App.lang() == l10)
        return t10;
    return english;
}
CChar *MLTC(CChar *english, LANG_TYPE l0, CChar *t0, LANG_TYPE l1, CChar *t1, LANG_TYPE l2, CChar *t2, LANG_TYPE l3, CChar *t3, LANG_TYPE l4, CChar *t4, LANG_TYPE l5, CChar *t5, LANG_TYPE l6, CChar *t6, LANG_TYPE l7, CChar *t7, LANG_TYPE l8, CChar *t8, LANG_TYPE l9, CChar *t9, LANG_TYPE l10, CChar *t10, LANG_TYPE l11, CChar *t11) {
    if (App.lang() == l0)
        return t0;
    if (App.lang() == l1)
        return t1;
    if (App.lang() == l2)
        return t2;
    if (App.lang() == l3)
        return t3;
    if (App.lang() == l4)
        return t4;
    if (App.lang() == l5)
        return t5;
    if (App.lang() == l6)
        return t6;
    if (App.lang() == l7)
        return t7;
    if (App.lang() == l8)
        return t8;
    if (App.lang() == l9)
        return t9;
    if (App.lang() == l10)
        return t10;
    if (App.lang() == l11)
        return t11;
    return english;
}
/******************************************************************************/
void Break() {
#if WINDOWS
    __debugbreak();
#elif LINUX
    asm("int3");
#elif !MOBILE && !WEB // everything except Mobile and Web
    __asm {int 3}
    ;
#endif
}
void ExitNow() {
    if (App.flag & APP_BREAKPOINT_ON_ERROR)
        Break();
#if WEB
    emscripten_exit_with_live_runtime(); // '_exit' would allow calling global destructors, 'emscripten_exit_with_live_runtime' does not allow it because it gets not caught and the browser stops, alternative is to use 'abort'
#elif SWITCH
    exit(-1);
#else
    _exit(-1);
#endif
}
void ExitEx(CChar *error) {
    if (App.exit)
        App.exit(error);
    FlushIO();
    App.showError(error);
    ExitNow();
}
void Exit(C Str &error) {
    if (App.flag & APP_CALLSTACK_ON_ERROR) {
        Str stack;
        if (GetCallStack(stack))
            ExitEx(Str(error).line() + "Current Call Stack:\n" + stack);
    }
    ExitEx(error);
}
void StartEEManually(Ptr dll_module_instance) {
#if WINDOWS_OLD
    App._hinstance = (dll_module_instance ? (HINSTANCE)dll_module_instance : GetModuleHandle(null));
    if (App.create())
        App.loop();
    App.del();
#else
    Exit("'StartEEManually' is unsupported on this platform");
#endif
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
#if MAC || LINUX
int main(int argc, char *argv[]) {
    const Int start = 1; // start from #1, because #0 is always the executable name (if file name has spaces, then they're included in the #0 argv)
    App.cmd_line.setNum(argc - start);
    FREPAO(App.cmd_line) = argv[start + i];
    if (App.create())
        App.loop();
    App.del();
    return 0;
}
#elif IOS
int main(int argc, char *argv[]) {
    extern Bool DontRemoveThisOriOSAppDelegateClassWontBeLinked;
    DontRemoveThisOriOSAppDelegateClassWontBeLinked = false;
    extern Bool DontRemoveThisOrMyViewControllerClassWontBeLinked;
    DontRemoveThisOrMyViewControllerClassWontBeLinked = false;
    extern Bool DontRemoveThisOrEAGLViewClassWontBeLinked;
    DontRemoveThisOrEAGLViewClassWontBeLinked = false;
    return UIApplicationMain(argc, argv, nil, nil);
}
#endif
/******************************************************************************/
