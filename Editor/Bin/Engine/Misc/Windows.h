#pragma once
/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'WindowCapture' to capture visual contents of a system window.

   Use 'Window' functions to handle OS window management.

/******************************************************************************/
struct SysWindow // Operating System Window
{
#if EE_PRIVATE
#if WINDOWS_OLD
    typedef HWND Type;
    /*#elif WINDOWS_NEW fails to compile
         typedef Windows::UI::Core::CoreWindow ^Type;*/
#elif MAC
    typedef NSWindow *Type;
#elif LINUX
    typedef XWindow Type;
    ASSERT(SIZE(Type) == SIZE(unsigned long));
#elif ANDROID
    typedef ANativeWindow *Type;
#else
    typedef Ptr Type;
#endif
#else
#if LINUX
    typedef unsigned long Type;
#else
    typedef Ptr Type;
#endif
#endif

    Str text() C;                           // get      window title text
    void minimize(Bool force = false) C;    // minimize window
    void maximize(Bool force = false) C;    // maximize window
    void reset(Bool force = false) C;       // reset    window from    maximized/minimized to normal state (maximized/minimized -> normal)
    void toggle(Bool force = false) C;      // toggle   window between maximized          and normal state (maximized          <-> normal)
    void activate() C;                      // activate window
    void close() C;                         // close    window
    void move(Int dx, Int dy) C;            // move     window by delta
    VecI2 pos() C;                          // get      window position
    void pos(Int x, Int y) C;               // set      window position
    void size(Int w, Int h, Bool client) C; // set      window size
    VecI2 size(Bool client) C;              // get      window size     , 'client'=if take only the client size      (not including the borders)
    RectI rect(Bool client) C;              // get      window rectangle, 'client'=if take only the client rectangle (not including the borders)
    Bool maximized() C;                     // if       window is maximized
    Bool minimized() C;                     // if       window is minimized
    UIntPtr threadID() C;                   // get      window thread  ID
    UInt processID() C;                     // get      window process ID
    SysWindow parent() C;                   // get      window          parent
    SysWindow parentTop() C;                // get      window most top parent
    void sendData(CPtr data, Int size) C;   // send binary data to application of the specified window, that application can receive the data using "App.receive_data" callback function

    operator Bool() C { return window != NULL; } // if window is valid

    Bool operator==(C SysWindow &w) C { return T.window == w.window; } // if pointers are equal
    Bool operator!=(C SysWindow &w) C { return T.window != w.window; } // if pointers are different

#if EE_PRIVATE
    Bool operator==(C Type &w) C { return T.window == w; } // if pointers are equal
    Bool operator!=(C Type &w) C { return T.window != w; } // if pointers are different

    operator Type() C { return window; } // auto cast

#if WINDOWS_NEW
    typedef Windows::UI::Core::CoreWindow ^ Type1;
    C Type1 &operator()() C { return (Type1 &)window; }
    C Type1 &operator->() C { return (Type1 &)window; }
    void release() C { (Type1 &)T.window = null; }
    void set(C Type1 &window) C { (Type1 &)T.window = window; }
#else
    Type operator()() C { return window; }
#endif
#endif

    SysWindow(null_t = null) { T.window = NULL; }
    SysWindow(Type window) { T.window = window; }

#if !EE_PRIVATE
  private:
#endif
    Type window;
};
/******************************************************************************/
SysWindow WindowActive();                   // get     window of active application
SysWindow WindowMouse();                    // get     window under mouse cursor
void WindowList(MemPtr<SysWindow> windows); // get all window handles in the System
/******************************************************************************/
struct WindowCapture // System Window Image Capture
{
    void del();                                             // delete manually
    Bool capture(Image &image, C SysWindow &window = null); // capture 'window' client screen to 'image' (use null for 'window' to capture full desktop), false on fail

    ~WindowCapture() { del(); }
    WindowCapture() { data = null; }

  private:
    Ptr data;
    NO_COPY_CONSTRUCTOR(WindowCapture);
};
/******************************************************************************/
