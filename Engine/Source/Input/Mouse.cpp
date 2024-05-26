/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
#define SELECT_DIST_2 Sqr(0.01f)
#define SPEED 0.005f
#define BUF_BUTTONS 256

#if WINDOWS_OLD
enum COOP_MODE {
    BACKGROUND,
    FOREGROUND,
    MOUSE_MODE = BACKGROUND, // use background mode so we can get correct information about relative movement
};
#elif WINDOWS_NEW
using namespace Windows::System;
using namespace Windows::UI::Core;
#endif
/******************************************************************************
#if WINDOWS_OLD
#define MOUSEEVENTF_MASK      0xFFFFFF00
#define MOUSEEVENTF_FROMTOUCH 0xFF515700
#define SET_HOOK              1
static HHOOK MsHook;
static LRESULT CALLBACK MsLLProc(int nCode, WPARAM wParam, LPARAM lParam)
{
   if(nCode>=0)
   {
    //MSLLHOOKSTRUCT &ms=*(MSLLHOOKSTRUCT*)lParam;
      MOUSEHOOKSTRUCT  &ms=*(MOUSEHOOKSTRUCT*)lParam;
      if((ms.dwExtraInfo&MOUSEEVENTF_MASK)==MOUSEEVENTF_FROMTOUCH)
         return 1;
   }
   return CallNextHookEx(MsHook, nCode, wParam, lParam);
}
static void SetHook() {if(!MsHook)MsHook=SetWindowsHookEx(WH_MOUSE_LL, MsLLProc, App._hinstance, 0);}
//static void SetHook() {if(!MsHook)MsHook=SetWindowsHookEx(WH_MOUSE, MsLLProc, App._hinstance, GetCurrentThreadId());}
static void  UnHook() {if( MsHook){UnhookWindowsHookEx(MsHook); MsHook=null;}}
#endif
/******************************************************************************/
void MouseCursorHW::del() {
#if WINDOWS_OLD
    if (_cursor) {
        DestroyIcon(_cursor);
        _cursor = null;
    }
#elif WINDOWS_NEW
    _cursor = null;
#elif MAC
    [_cursor release];
    _cursor = null;
#elif LINUX
    if (_cursor) {
        if (XDisplay)
            XFreeCursor(XDisplay, (XCursor)_cursor);
        _cursor = null;
    }
#endif
    _image.del();
}
Bool MouseCursorHW::create(C Image &image, C VecI2 &hot_spot) {
    del();
#if WINDOWS_OLD
    _cursor = CreateIcon(image, &hot_spot);
#elif WINDOWS_NEW
    // TODO: WINDOWS_NEW currently there's no way to dynamically create a 'CoreCursor'
#elif MAC // Mac must keep the cursor image data, it is stored in 'T._image', if released then cursor image data gets corrupted, that's why it must be kept in memory (yes that was tested)
    image.mustCopy(_image, -1, -1, 1, IMAGE_R8G8B8A8_SRGB, IMAGE_SOFT, 1);
    REPD(y, _image.h())
    REPD(x, _image.w()) {
        // premultiply alpha (required by Mac OS)
        Color c = _image.color(x, y);
        c.r = c.r * c.a / 255;
        c.g = c.g * c.a / 255;
        c.b = c.b * c.a / 255;
        _image.color(x, y, c);
    }

    unsigned char *image_data = _image.data();
    if (NSBitmapImageRep *bitmap = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&image_data
                                                                           pixelsWide:_image.w()
                                                                           pixelsHigh:_image.h()
                                                                        bitsPerSample:8
                                                                      samplesPerPixel:4
                                                                             hasAlpha:YES
                                                                             isPlanar:NO
                                                                       colorSpaceName:NSCalibratedRGBColorSpace
                                                                          bytesPerRow:_image.pitch()
                                                                         bitsPerPixel:32]) {
        if (NSImage *ns_image = [[NSImage alloc] initWithSize:NSMakeSize(_image.w(), _image.h())]) {
            [ns_image addRepresentation:bitmap];
            NSPoint point;
            point.x = hot_spot.x;
            point.y = hot_spot.y;
            _cursor = [[NSCursor alloc] initWithImage:ns_image hotSpot:point];
            [ns_image release];
        }
        [bitmap release];
    }
#elif LINUX
    if (XDisplay) {
        Image temp;
        C Image *src = &image;
        if (src->compressed())
            if (src->copy(temp, -1, -1, 1, IMAGE_R8G8B8A8_SRGB, IMAGE_SOFT, 1))
                src = &temp;
            else
                src = null;
        if (src && src->lockRead()) {
            if (XcursorImage *image = XcursorImageCreate(src->w(), src->h())) {
                image->xhot = hot_spot.x;
                image->yhot = hot_spot.y;
                image->delay = 0;
                VecB4 *bgra = (VecB4 *)image->pixels;
                FREPD(y, src->h())
                FREPD(x, src->w()) {
                    Color c = src->color(x, y);
                    bgra++->set(c.b, c.g, c.r, c.a);
                }
                _cursor = (Ptr)XcursorImageLoadCursor(XDisplay, image);
                XcursorImageDestroy(image);
            }
            src->unlock();
        }
    }
#elif WEB
    // TODO: Web mouse cursor
#endif

    return _cursor != null;
}
/******************************************************************************/
void MouseCursor::del() {
    _hw.del();
    _image.clear();
    _hot_spot.zero();
    if (this == Ms._cursor)
        Ms.resetCursor();
}
void MouseCursor::create(C ImagePtr &image, C VecI2 &hot_spot, Bool hardware) {
    if (hardware && image)
        _hw.create(*image, hot_spot);
    else
        _hw.del();
    _image = image; // always remember, even if hardware succeeded, because if VR gets active, then we can't use hardware
    _hot_spot = hot_spot;
    if (this == Ms._cursor)
        Ms.resetCursor();
}
/******************************************************************************/
#if MAC
VecI2 MouseIgnore;
static Bool MouseClipOn;
static RectI MouseClipRect;
#elif LINUX
static MouseCursorHW MsCurEmpty;
static XWindow Grab;
#endif

struct MouseEvent {
    Bool push;
    Byte button;

    void set(Bool push, Byte button) {
        T.push = push;
        T.button = button;
    }
};
static Memc<MouseEvent> MouseEvents;

MouseClass Ms;
/******************************************************************************/
MouseClass::MouseClass() {
#if 0 // there's only one 'MouseClass' global 'Ms' and it doesn't need clearing members to zero
   REPAO(_button)=0;
  _selecting=_dragging=_first=_hardware=_detected=_on_client=_clip_rect_on=_clip_window=_freeze=_frozen=_action=_locked=_swapped=false;
  _start_time=_wheel_time=0;
  _pos=_delta_rel_sm=_delta_clp=_delta_rel=_start_pos=_move_offset=_wheel=_wheel_f=0;
  _window_pixeli=_desktop_pixeli=_delta_pixeli_clp=_wheel_i=0;
  _clip_rect.zero();
  _cursor=null;
#if WINDOWS_OLD
  _device=null;
#endif
#endif
    _visible = _want_cur_hw = true;
    _cur = -1;
    _speed = SPEED;
    _button_name[0] = "Mouse1";
    _button_name[1] = "Mouse2";
    _button_name[2] = "Mouse3";
    _button_name[3] = "Mouse4";
    _button_name[4] = "Mouse5";
    _button_name[5] = "Mouse6";
    _button_name[6] = "Mouse7";
    _button_name[7] = "Mouse8";
#if SWITCH
    _on_client = true; // #NintendoSwitchRes
#endif
}
void MouseClass::del() {
    _cursor = null;
    _cursor_temp.del(); // clear the pointer and images because display and images are already deleted, and attempting to use it afterwards will result in a crash
#if WINDOWS_OLD
#if MS_RAW_INPUT
    RAWINPUTDEVICE rid[1];

    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x02; // mouse
    rid[0].dwFlags = RIDEV_REMOVE;
    rid[0].hwndTarget = App.window();

    RegisterRawInputDevices(rid, Elms(rid), SIZE(RAWINPUTDEVICE));
#elif MS_DIRECT_INPUT
    RELEASE(_device);
#endif
#elif LINUX
    if (Grab) {
        XDestroyWindow(XDisplay, Grab);
        Grab = NULL;
    }
#endif
}
void MouseClass::create() {
    if (LogInit)
        LogN("Mouse.create");
#if WINDOWS_OLD
    _swapped = (GetSystemMetrics(SM_SWAPBUTTON) != 0);
#if MS_RAW_INPUT
    RAWINPUTDEVICE rid[1];

    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x02; // mouse
    rid[0].dwFlags = ((MOUSE_MODE == BACKGROUND) ? RIDEV_INPUTSINK : 0);
    rid[0].hwndTarget = App.window();

    RegisterRawInputDevices(rid, Elms(rid), SIZE(RAWINPUTDEVICE));
    // detection is done in 'checkMouseKeyboard'
#elif MS_DIRECT_INPUT
    if (InputDevices.DI) // need to use DirectInput to be able to obtain '_delta_rel'
        if (OK(InputDevices.DI->CreateDevice(GUID_SysMouse, &_device, null))) {
            if (OK(_device->SetDataFormat(&c_dfDIMouse2)))
                if (OK(_device->SetCooperativeLevel(App.window(), DISCL_NONEXCLUSIVE | ((MOUSE_MODE == FOREGROUND) ? DISCL_FOREGROUND : DISCL_BACKGROUND)))) {
                    DIPROPDWORD dipdw;
                    dipdw.diph.dwSize = SIZE(DIPROPDWORD);
                    dipdw.diph.dwHeaderSize = SIZE(DIPROPHEADER);
                    dipdw.diph.dwObj = 0;
                    dipdw.diph.dwHow = DIPH_DEVICE;
                    dipdw.dwData = BUF_BUTTONS;
                    _device->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

                    if (MOUSE_MODE == BACKGROUND)
                        _device->Acquire();       // in background mode we always want the mouse to be acquired
                    _hardware = _detected = true; // don't set false in case user called 'Ms.simulate'
                    goto ok;
                }
            RELEASE(_device);
        }
ok:;
#endif
#elif WINDOWS_NEW
        // detection is done in 'checkMouseKeyboard'
#elif MAC || LINUX || WEB // FIXME: TODO:
    _hardware = _detected = true; // don't set false in case user called 'Ms.simulate'
#endif
#if LINUX
    // create empty cursor
    Image temp(1, 1, 1, IMAGE_L8A8_SRGB, IMAGE_SOFT, 1);
    temp.color(0, 0, TRANSPARENT);
    MsCurEmpty.create(temp);

    // create window for grabbing
    if (XDisplay) {
        XSetWindowAttributes win_attr;
        Zero(win_attr);
        win_attr.event_mask = ButtonPressMask | ButtonReleaseMask | StructureNotifyMask;
        win_attr.override_redirect = true;
        if (Grab = XCreateWindow(XDisplay, DefaultRootWindow(XDisplay), -1, -1, 1, 1, 0, 0, InputOnly, CopyFromParent, CWEventMask | CWOverrideRedirect, &win_attr)) // set at -1,-1 pos 1,1 size (outside of desktop, with valid size, invalid size would fail to create window)
            if (XMapRaised(XDisplay, Grab) == 1) {
                XSync(XDisplay, false);
                REP(1024) // attempts to check if window was mapped, otherwise grabbing will fail (last time this was tested, the loop was not needed, but since other 'XCheckTypedWindowEvent' in the engine required it, then keep this one just in case)
                {
                    XEvent event;
                    if (XCheckTypedWindowEvent(XDisplay, Grab, MapNotify, &event))
                        break;
                    usleep(1);
                }
            }
    }
#endif

#if SWITCH
    _desktop_pixeli = _window_pixeli.set(1280 / 2, 720 / 2); // NintendoSwitch APIs are always for 1280,720 res #NintendoSwitchRes
#else
    _desktop_pixeli = _window_pixeli = D.res() / 2; // initially set at screen center, in case mouse is unavailable
#endif
    updatePos();
    _delta_pixeli_clp.zero(); // always get position at the start, and clear any pixel delta. This is needed so that further readings of mouse position will properly detect if there was any change, so we can trigger '_detected'. Also initial position is needed in codes below

#if WINDOWS_NEW
    _on_client = Cuts(_window_pixeli, RectI(0, 0, D.resW(), D.resH())); // set initial value of '_on_client', because 'OnPointerEntered' is not called at start
#endif
}
/******************************************************************************/
CChar8 *MouseClass::buttonName(Int b) C {
    return InRange(b, _button_name) ? _button_name[b] : null;
}
/******************************************************************************/
void MouseClass::speed(Flt speed) { T._speed = speed * SPEED; }
Flt MouseClass::speed() C { return T._speed / SPEED; }
/******************************************************************************/
void MouseClass::pos(C Vec2 &pos) {
    Vec2 old = T._pos;
    T._pos = pos;
#if MOBILE // for mobile prevent cursor from going outside the window
    T._pos &= D.rect();
#endif

    if (_frozen && App.active())
        clipUpdate(); // update clip before setting new position !! after adjusting T._pos !!
    VecI2 window_pixel_i = D.screenToWindowPixelI(T._pos);

#if WINDOWS_OLD
    POINT point = {window_pixel_i.x, window_pixel_i.y};
    ClientToScreen(App.window(), &point); // convert from window to desktop
    SetCursorPos(point.x, point.y);
#elif WINDOWS_NEW
    if (App.window()) {
        Windows::Foundation::Rect bounds = App.window()->Bounds;
        App.window()->PointerPosition = Windows::Foundation::Point(bounds.X + PixelsToDips(window_pixel_i.x), bounds.Y + PixelsToDips(window_pixel_i.y));
    }
#elif MAC
    RectI client = App.window().rect(true);
    CGPoint point;
    point.x = window_pixel_i.x + client.min.x;
    point.y = window_pixel_i.y + client.min.y;
    CGWarpMouseCursorPosition(point);
#elif LINUX
    if (XDisplay)
        XWarpPointer(XDisplay, NULL, App.window(), 0, 0, 0, 0, window_pixel_i.x, window_pixel_i.y);
#else
    _window_pixeli = _desktop_pixeli = window_pixel_i;
#endif

    updatePos();
    _pos = D.windowPixelToScreen(_window_pixeli);
    _delta_clp += T._pos - old;
}
/******************************************************************************/
static void Clip(RectI *rect) // 'rect' is in window client space, full rect is (0, 0), (D.resW, D.resH)
{
    // !! we can't disable clipping (set rect to null) when rect covers entire screen and in full screen, because computer can have multiple monitors connected, and we may want to prevent going to other monitors !!
#if WINDOWS_OLD
    if (!rect)
        ClipCursor(null);
    else {
        RECT r;
        r.left = rect->min.x;
        r.right = rect->max.x;
        r.top = rect->min.y;
        r.bottom = rect->max.y;

        POINT p = {0, 0};
        ClientToScreen(App.window(), &p);
        r.left += p.x;
        r.right += p.x;
        r.top += p.y;
        r.bottom += p.y;

        ClipCursor(&r);
    }
#elif WINDOWS_NEW
    /*if(App.window()) // this only prevents any action taken on the title bar, like move window, minimize/maximize/close, but the cursor can still move outside the window and activate other apps on click
      {
         if(rect)App.window()->    SetPointerCapture();
         else    App.window()->ReleasePointerCapture();
      }*/
    if (rect && !Cuts(Ms._window_pixeli, *rect) && App.window()) {
        VecI2 pixeli = Ms._window_pixeli;
        Ms._window_pixeli &= *rect;
        pixeli -= Ms._window_pixeli;
        Ms._desktop_pixeli -= pixeli;
        Ms._delta_pixeli_clp += pixeli;
        Windows::Foundation::Rect bounds = App.window()->Bounds;
        App.window()->PointerPosition = Windows::Foundation::Point(bounds.X + PixelsToDips(Ms._window_pixeli.x), bounds.Y + PixelsToDips(Ms._window_pixeli.y));
    }
#elif MAC
    if (rect)
        MouseClipRect = *rect;
    MouseClipOn = (rect != null);
    CGAssociateMouseAndMouseCursorPosition(!MouseClipOn); // freeze mouse cursor when needed
    // !! warning: clipping using CGAssociateMouseAndMouseCursorPosition + CGWarpMouseCursorPosition will introduce delay in mouse movement, because 'CGAssociateMouseAndMouseCursorPosition' always freezes mouse, and 'CGWarpMouseCursorPosition' moves it manually based on detected input, there used to be "CGSetLocalEventsSuppressionInterval(0)" removing this delay, but it's now deprecated and its replacement doesn't affect clipping anymore
#elif LINUX
    if (XDisplay) {
        if (!rect)
            XUngrabPointer(XDisplay, CurrentTime);
        else {
            Bool custom = (*rect != RectI(0, 0, D.resW(), D.resH()));
            if (custom) {
                VecI2 pos = 0;
                XWindow child = NULL;
                XTranslateCoordinates(XDisplay, App.window(), DefaultRootWindow(XDisplay), 0, 0, &pos.x, &pos.y, &child); // convert to desktop space
                XMoveResizeWindow(XDisplay, Grab, rect->min.x + pos.x, rect->min.y + pos.y, rect->w(), rect->h());
            } else
                XMoveResizeWindow(XDisplay, Grab, -1, -1, 1, 1); // move outside of desktop area
            XFlush(XDisplay);
            XGrabPointer(XDisplay, App.window(), false, ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask, GrabModeAsync, GrabModeAsync, custom ? Grab : App.window()(), NULL, CurrentTime);
        }
    }
#elif WEB
    if (rect)
        emscripten_request_pointerlock(null, true);
    else
        emscripten_exit_pointerlock();
#endif
}
void MouseClass::clipUpdate() // !! Don't call always, to avoid changing clip for other apps when inactive !!
{
    if (App.active() && (_frozen || _clip_rect_on || _clip_window)) {
        RectI recti, window_rect;
        if (_clip_window) {
            window_rect = D.screenToWindowPixelI(D.rect()); // usually this is (0, 0)..(D.resW, D.resH) however for VR it's calculated based on its GuiTexture
#if MAC                                                     // on Mac having mouse on the window border captures cursor for resizing, and mouse clicks do nothing
            if (!D.full()) {
                const Int border = 3;
                window_rect.min.x += border;
                window_rect.max.x -= border;
                window_rect.max.y -= border;
            }
#endif
        }
        if (_frozen) {
            Vec2 p = pos();
            if (_clip_rect_on)
                p &= _clip_rect;
            VecI2 pi = D.screenToWindowPixelI(p);
            if (_clip_window)
                pi &= window_rect;
            recti = pi;
        } else if (_clip_rect_on) {
            recti = D.screenToWindowPixelI(_clip_rect);
            if (_clip_window) { // intersect min/max separately to make sure 'recti' isn't invalid, doing just "recti&=window_rect" could result in invalid rectangle and system call could be ignored
                recti.min &= window_rect;
                recti.max &= window_rect;
            }
        } else // window
        {
            recti = window_rect;
        }
        // at this stage 'recti' is inclusive (0..resW()-1, 0..resH()-1)
#if !WINDOWS_NEW && !MAC // can't do this for WINDOWS_NEW and MAC because there we need 'recti' to be inclusive
        recti.max++;
#endif
        Clip(&recti);
    } else
        Clip(null);
}
void MouseClass::clipUpdateConditional() {
    if (App.active() && (/*_frozen || */ _clip_rect_on || _clip_window))
        clipUpdate(); // update only if clipping to rect/window (this ignores freeze because cases calling this method don't need it)
}
MouseClass &MouseClass::clip(C Rect *rect, Int window) {
    Bool rect_on = (rect != null),
         win = ((window < 0) ? _clip_window : (window != 0));                                // <0 - keep old, >=0 - set new
    if (_clip_rect_on != rect_on || (rect_on && _clip_rect != *rect) || _clip_window != win) // if something changes
    {
        if (_clip_rect_on = rect_on)
            _clip_rect = *rect;
        _clip_window = win;
        if (App.active())
            clipUpdate();
    }
    return T;
}
MouseClass &MouseClass::freeze() {
    _freeze = true;
    return T;
}
/******************************************************************************/
#if WINDOWS_NEW
static void MouseResetCursor() { Ms.resetCursor(); }
#endif
static Bool CanUseHWCursor() { return Ms._cursor && Ms._cursor->_hw.is() && !VR.active(); } // can't use hardware cursor in VR mode (it can be enabled there, however that would also require drawing it manually on the gui surface, and that would result in cursor being drawn twice on the window - 1-on gui surface 2-using hardware cursor)
void MouseClass::resetCursor() {
#if WINDOWS_NEW
    if (!App.mainThread()) {
        App.includeFuncCall(MouseResetCursor);
        return;
    } // for Windows New this can be called only on the main thread
#endif

    Int cur; // -1=system default, 0=hidden, 1=custom hardware
    if (!App.active()
#if !WINDOWS_NEW
        || !_on_client
#endif
    )
        cur = -1;
    else // for example: not on the window
        if (hidden())
        cur = 0;
    else // for example: want to be hidden
        if (CanUseHWCursor())
        cur = 1;
    else // for example: our own custom hardware cursor
        if (_cursor && _cursor->_image)
        cur = 0;
    else          // for example: our own custom non-hardware cursor
        cur = -1; // use default

#if WINDOWS_OLD
    if (!_on_client)
        return; // don't set cursor if not on client, to let OS decide which cursor to use (sometimes it can be resizing window cursor)
    SetCursor((cur < 0) ? LoadCursor(null, IDC_ARROW) : (cur == 0) ? null
                                                                   : _cursor->_hw._cursor);
#elif WINDOWS_NEW
    if (App.window()) {
        App.window()->PointerCursor = ((cur < 0) ? ref new CoreCursor(CoreCursorType::Arrow, 0) : (cur == 0) ? null
                                                                                                             : _cursor->_hw._cursor);
        _locked = (App.window()->PointerCursor == null);
    }
#elif MAC
    if (cur)
        [((cur > 0) ? _cursor->_hw._cursor : [NSCursor arrowCursor]) set];
    Bool visible = (cur != 0);
    if (visible != CGCursorIsVisible())
        if (visible)
            [NSCursor unhide];
        else
            [NSCursor hide];
#elif LINUX
    if (XDisplay && App.window())
        XDefineCursor(XDisplay, App.window(), XCursor((cur < 0) ? null : (cur == 0) ? MsCurEmpty._cursor
                                                                                    : _cursor->_hw._cursor));
#endif
}
MouseClass &MouseClass::cursor(C MouseCursor *cursor) {
    if (T._cursor != cursor) {
        T._cursor = cursor;
        resetCursor();
    }
    return T;
}
MouseClass &MouseClass::cursor(C ImagePtr &image, C VecI2 &hot_spot, Bool hardware, Bool reset) {
    if (_cursor_temp._image != image || _cursor_temp._hot_spot != hot_spot || _want_cur_hw != hardware || reset) {
        _cursor_temp.create(image, hot_spot, _want_cur_hw = hardware);
        cursor(&_cursor_temp);
    }
    return T;
}
/******************************************************************************/
MouseClass &MouseClass::visible(Bool show) {
    if (_visible != show) {
        _visible = show;
        resetCursor();
    }
    return T;
}
/******************************************************************************/
void MouseClass::eat(Int b) {
    if (InRange(b, _button))
        FlagDisable(_button[b], BS_NOT_ON);
}
void MouseClass::eatWheel() {
    _wheel.zero();
    _wheel_i.zero();
} // don't clear '_wheel_f' because it should continue to be accumulated
void MouseClass::eat() {
    REPA(_button)
    eat(i);
}
/******************************************************************************/
void MouseClass::acquire(Bool on) {
#if WINDOWS_OLD
#if MS_DIRECT_INPUT
    if (MOUSE_MODE == FOREGROUND && _device) {
        if (on)
            _device->Acquire();
        else
            _device->Unacquire();
    } // we need to change acquire only if we're operating in Foreground mode
#endif
#if SET_HOOK
    if (on)
        SetHook();
    else
        UnHook();
#endif
    if (on) // when restoring access to mouse
    {
        _delta_rel.zero(); // clear any deltas (to disable processing mouse movement when app was inactive)
    }
#endif
    if (_frozen || _clip_rect_on || _clip_window)
        clipUpdate(); // this gets called when app gets de/activated, so we have to update clip only if we want any clip (to enable it when activating and disable when deactivating)
    resetCursor();
}
/******************************************************************************/
void MouseClass::clear() {
    eatWheel();
    _delta_rel.zero();
    _delta_pixeli_clp.zero();
    REPAO(_button) &= ~BS_NOT_ON;
    REP(Min(2, MouseEvents.elms())) // process only up to 2 queued mouse events (so we can do push+release in one time, but not push+release+push)
    {
        C MouseEvent &event = MouseEvents.first();
        if (event.push)
            _push(event.button);
        else
            _release(event.button);
        MouseEvents.remove(0, true);
    }
}
/******************************************************************************/
void MouseClass::_push(Byte b) // !! assumes 'b' is in range !!
{
    DEBUG_RANGE_ASSERT(b, _button);
    if (!(_button[b] & BS_ON)) {
        _button[b] |= BS_PUSHED | BS_ON;
        if (_cur == b && _first && life() <= DoubleClickTime + Time.ad()) {
            _button[b] |= BS_DOUBLE;
            _first = false;
        } else {
            _first = true;
            _detected = true;
        }
        _cur = b;
        _start_pos = pos();
        _start_time = Time.appTime();
        Inputs.New().set(true, INPUT_MOUSE, b);
    }
}
void MouseClass::_release(Byte b) // !! assumes 'b' is in range !!
{
    DEBUG_RANGE_ASSERT(b, _button);
    if (_button[b] & BS_ON) {
        FlagDisable(_button[b], BS_ON);
        FlagEnable(_button[b], BS_RELEASED);
        if (!selecting() && life() <= TapTime + Time.ad())
            _button[b] |= BS_TAPPED;
        Inputs.New().set(false, INPUT_MOUSE, b);
    }
}
void MouseClass::push(Byte b) {
    if (InRange(b, _button)) {
        if (MouseEvents.elms() || br(b))
            MouseEvents.New().set(true, b); // if have any events queued or button was released in the same frame, then can't push it now, but delay for the next frame, also needed for 'tapped' 'tappedFirst' can be properly detected
        else
            _push(b);
    }
}
void MouseClass::release(Byte b) {
    if (InRange(b, _button)) {
        if (MouseEvents.elms())
            MouseEvents.New().set(false, b); // if have any events queued
        else
            _release(b);
    }
}
void MouseClass::moveAbs(C Vec2 &screen_d) { move(screen_d / D.scale()); }
void MouseClass::move(C Vec2 &screen_d) {
    if (screen_d.any()) {
        Vec2 pixel_d = D.screenToWindowPixelSize(screen_d); // convert to pixel delta
        _delta_rel_sm += pixel_d * _speed;                  // adjust by speed
        if (!frozen()) {
#if 1 // pixel align (needed because all Operating Systems internally store mouse position as 'int')
            pixel_d += _move_offset;                             // add what we've accumulated before
            VecI2 pixel_di = Round(pixel_d);                     // round to nearest pixel
            _move_offset = pixel_d - pixel_di;                   // calculate what we want - what we've got
            Vec2 screen_d = D.windowPixelToScreenSize(pixel_di); // move by aligned pixel delta
#endif
            pos(pos() + screen_d);
        }
    }
}
void MouseClass::scroll(C Vec2 &d) { _wheel += d; }
/******************************************************************************/
void MouseClass::updatePos() {
    // !! here '_delta_pixeli_clp' is += because this can be called several times per frame (once in 'update' and many in 'pos') !!
#if WINDOWS_OLD
    // position and delta
    POINT p;
    if (GetCursorPos(&p)) {
        _delta_pixeli_clp.x += p.x - _desktop_pixeli.x;
        _desktop_pixeli.x = p.x; // calc based on desktop position and not window position
        _delta_pixeli_clp.y += p.y - _desktop_pixeli.y;
        _desktop_pixeli.y = p.y;

        ScreenToClient(App.window(), &p);
        _window_pixeli.x = p.x;
        _window_pixeli.y = p.y;
    }
    _on_client = (InRange(_window_pixeli.x, D.resW()) && InRange(_window_pixeli.y, D.resH()) && WindowMouse() == App.window());
    // 'resetCursor' is always called in 'WM_SETCURSOR'
#elif WINDOWS_NEW
    if (App.window()) {
        VecI2 desktop_pixeli(DipsToPixelsI(App.window()->PointerPosition.X), DipsToPixelsI(App.window()->PointerPosition.Y));
        _delta_pixeli_clp += desktop_pixeli - _desktop_pixeli; // calc based on desktop position and not window position
        _desktop_pixeli = desktop_pixeli;
        Windows::Foundation::Rect bounds = App.window()->Bounds;
        _window_pixeli.set(desktop_pixeli.x - DipsToPixelsI(bounds.X),
                           desktop_pixeli.y - DipsToPixelsI(bounds.Y));
    }
    // '_on_client' is managed through 'OnPointerEntered' and 'OnPointerExited' callbacks
#elif MAC
    VecI2 screen = D.screen();
    RectI client = App.window().rect(true);
    VecI2 desktop_pixeli;

    if (MouseClipOn) // clipping
    {
        RectI clip = MouseClipRect;
        clip.min += client.min;
        MAX(clip.min.x, 2);
        MAX(clip.min.y, 2); // padd to don't touch edges in order to avoid popping dock
        clip.max += client.min;
        MIN(clip.max.x, screen.x - 3);
        MIN(clip.max.y, screen.y - 3);

        desktop_pixeli.set(_desktop_pixeli.x + Round(_delta_rel.x),
                           _desktop_pixeli.y - Round(_delta_rel.y)) &= clip;

        MouseIgnore += desktop_pixeli - _desktop_pixeli;
        CGPoint p;
        p.x = desktop_pixeli.x;
        p.y = desktop_pixeli.y;
        CGWarpMouseCursorPosition(p);
    } else {
        NSPoint p = [NSEvent mouseLocation];
        desktop_pixeli.x = Round(p.x);
        desktop_pixeli.y = screen.y - Round(p.y);
    }

    _delta_pixeli_clp += desktop_pixeli - _desktop_pixeli; // calc based on desktop position and not window position
    _desktop_pixeli = desktop_pixeli;
    _window_pixeli = desktop_pixeli - client.min;
    // '_on_client' is managed through 'mouseEntered' and 'mouseExited' callbacks
#elif LINUX
    if (XDisplay) {
        VecI2 desktop_pixeli;
        unsigned int mask;
        XWindow root, child;
        XQueryPointer(XDisplay, App.window(), &root, &child, &desktop_pixeli.x, &desktop_pixeli.y, &_window_pixeli.x, &_window_pixeli.y, &mask);

        _delta_pixeli_clp += desktop_pixeli - _desktop_pixeli; // calc based on desktop position and not window position
        _desktop_pixeli = desktop_pixeli;
        // '_on_client' is managed through 'EnterNotify' and 'LeaveNotify' events
    }
#elif SWITCH // NintendoSwitch APIs are always for 1280,720 res #NintendoSwitchRes, so can't compare with 'D.res', instead always set to true in constructor
#else
    // desktop and window pos obtained externally in main loop
    _on_client = (InRange(_window_pixeli.x, D.resW()) && InRange(_window_pixeli.y, D.resH()));
#if WEB
    _on_client |= _locked;
#endif
#endif
}
void MouseClass::update() {
    // clip
    if (_freeze) {
        if (!_frozen) {
            _frozen = true;
            clipUpdate();
        }
        _freeze = false;
    } else {
        if (_frozen) {
            _frozen = false;
            clipUpdate();
        }
    }

#if WINDOWS_OLD && MS_DIRECT_INPUT
    if (_device) {
        DIMOUSESTATE2 dims;
        if (OK(_device->GetDeviceState(SIZE(dims), &dims))) {
            _delta_rel.x = dims.lX;
            _delta_rel.y = -dims.lY;
        } else
            _device->Acquire(); // try to re-acquire if lost access for some reason

        DIDEVICEOBJECTDATA didods[BUF_BUTTONS];
        DWORD elms = BUF_BUTTONS;
        if (OK(_device->GetDeviceData(SIZE(DIDEVICEOBJECTDATA), didods, &elms, 0)))
            FREP(elms) // process in order
            {
                ASSERT(DIMOFS_BUTTON0 + 1 == DIMOFS_BUTTON1 && DIMOFS_BUTTON1 + 1 == DIMOFS_BUTTON2 && DIMOFS_BUTTON2 + 1 == DIMOFS_BUTTON3 && DIMOFS_BUTTON3 + 1 == DIMOFS_BUTTON4 && DIMOFS_BUTTON4 + 1 == DIMOFS_BUTTON5 && DIMOFS_BUTTON5 + 1 == DIMOFS_BUTTON6 && DIMOFS_BUTTON6 + 1 == DIMOFS_BUTTON7); // check that values are continuous
                C DIDEVICEOBJECTDATA &didod = didods[i];
                Int b = didod.dwOfs - DIMOFS_BUTTON0;
                if (InRange(b, 8)) // check range because this is also called for other events (like movements, not only buttons)
                {
                    if (didod.dwData & 0x80) {
                        if ((MOUSE_MODE == FOREGROUND || App.active()) && _on_client)
                            push(b);
                    } else
                        release(b);
                }
            }
    }
#endif

    updatePos();
#if WINDOWS_NEW
    if (App.active() && (_frozen || _clip_rect_on || _clip_window))
        clipUpdate();
#endif

    Vec2 old = _pos; // remember old

    // calculate new
#if WINDOWS_NEW || WEB
    if (_locked) // for WINDOWS_NEW and WEB when '_locked', the '_window_pixeli' never changes so we need to manually adjust the '_pos' based on '_delta_rel'
    {
        if (!_frozen)
            _pos += D.windowPixelToScreenSize(_delta_rel);

        // clip
        if (_clip_rect_on)
            _pos &= _clip_rect;
        else {
            Clamp(_pos.x, -D.w(), D.w());
            Clamp(_pos.y, -D.h(), D.h());
        }
    } else
#endif
    {
        _pos = D.windowPixelToScreen(_window_pixeli);
    }

    _delta_clp = _pos - old; // get delta = new-old
    _delta_rel *= _speed;
    _delta_rel_sm = _sv_delta.update(_delta_rel); // yes, mouse delta smoothing is needed, especially for low fps (for example ~40), without this, player camera rotation was not smooth
    if (Time.ad())
        _vel = _sv_vel.update(_delta_clp / Time.ad(), Time.ad()); // use '_delta_clp' to match exact cursor position

    // dragging
    if (b(_cur)) {
        if (!dragging()) // since dragging can be enabled only if selecting, then check this first to allow skipping check for selecting
        {
#if 1 // screen units
            if (!selecting() && Dist2(pos(), startPos()) * Sqr(D.scale() * (D.smallSize() ? 0.5f : 1.0f)) >= SELECT_DIST_2)
                _selecting = true;
#else // pixel  units
            if (!selecting() && D.screenToWindowPixelSize(pos() - startPos()).length2() >= Sqr(3.5))
                _selecting = true;
#endif
            if (/*!dragging () && already checked above*/ selecting() && life() >= DragTime + Time.ad())
                _dragging = true;
        }
    } else if (!br(_cur))
        _dragging = _selecting = false; // disable dragging only if button not on and not released, to allow detection inside the game for "release after dragging"

    // wheel
    if (_wheel.any()) {
        if (Time.appTime() > _wheel_time + 1)
            _wheel_f = _wheel;
        else
            _wheel_f += _wheel;       // if enough time passed without any wheel movement then start from scratch, otherwise keep accumulating
        _wheel_time = Time.appTime(); // remember current app time
        if (Int i = RoundEps(_wheel_f.x, 0.1f)) {
            _wheel_i.x += i;
            _wheel_f.x -= i;
        } // increase integer counters if enough movement occurred, use custom epsilon instead of 0.5f, we need it >0 to make sure that for example 2.999f will get converted to 3
        if (Int i = RoundEps(_wheel_f.y, 0.1f)) {
            _wheel_i.y += i;
            _wheel_f.y -= i;
        } // increase integer counters if enough movement occurred, use custom epsilon instead of 0.5f, we need it >0 to make sure that for example 2.999f will get converted to 3
    }

    // action
    _action = (pixelDelta().any() || _wheel.any() || bp(0));
    _detected |= _action;
}
/******************************************************************************/
void MouseClass::draw() {
    if (_cursor && _cursor->_image && _detected && visible() && !CanUseHWCursor() && _on_client
#if !WINDOWS_OLD
        && App.active()
#endif
    ) {
        Vec2 pos = D.screenAlignedToPixel(T.pos());
        if (_cursor->_hot_spot.any()) {
            Vec2 size = D.pixelToScreenSize(_cursor->_hot_spot);
            pos.x -= size.x;
            pos.y += size.y;
        }
        _cursor->_image->draw(Rect_LU(pos, D.pixelToScreenSize(_cursor->_image->size())));
    }
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
