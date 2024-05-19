/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Mouse' to access Mouse input.

/******************************************************************************/
struct MouseCursorHW // Hardware Mouse Cursor
{
    void del();                                                   // delete manually
    Bool create(C Image &image, C VecI2 &hot_spot = VecI2(0, 0)); // create from image, 'hot_spot'=focus position in image coordinates, 'hardware'=if use hardware cursor (this allows to draw the mouse cursor with full display speed, regardless of the game speed), false on fail

    ~MouseCursorHW() { del(); }
    MouseCursorHW() { _cursor = null; }
    Bool is() C { return _cursor != null; }

#if EE_PRIVATE
#if WINDOWS_OLD
    HCURSOR _cursor;
#elif WINDOWS_NEW
    Windows::UI::Core::CoreCursor ^ _cursor;
#elif MAC
    NSCursor *_cursor;
#elif LINUX
    Ptr _cursor; // cast to XCursor which is unsigned long
#else
    Ptr _cursor;
#endif
#else
  private:
    Ptr _cursor;
#endif
    Image _image;

    NO_COPY_CONSTRUCTOR(MouseCursorHW);
};
/******************************************************************************/
struct MouseCursor // Mouse Cursor
{
    void del();                                                                            // delete manually
    void create(C ImagePtr &image, C VecI2 &hot_spot = VecI2(0, 0), Bool hardware = true); // create from image, 'hot_spot'=focus position in image coordinates, 'hardware'=if use hardware cursor (this allows to draw the mouse cursor with full display speed, regardless of the game speed)

#if !EE_PRIVATE
  private:
#endif
    MouseCursorHW _hw;
    ImagePtr _image;
    VecI2 _hot_spot;
};
/******************************************************************************/
struct MouseClass // Mouse Input
{
    C Vec2 &pos() C { return _pos; }
    void pos(C Vec2 &pos);                      // get/set cursor position                      (in Screen Coordinates), setting position ignores 'freeze' (new position is always set regardless if mouse is frozen)
    C Vec2 &startPos() C { return _start_pos; } // get     cursor position of first button push (in Screen Coordinates), this     is equal to the most recent cursor position at the moment of first button push - "bp(0)"
    C Vec2 &d() C { return _delta_rel_sm; }     // get     cursor position delta                (in Screen Coordinates), delta    is not affected by mouse clipping or  display scale, use this for smooth unlimited mouse movement deltas (for example rotate the player)
    C Vec2 &dc() C { return _delta_clp; }       // get     cursor position delta clipped        (in Screen Coordinates), delta    is     affected by mouse clipping and display scale, use this for 2D gui object movement, limited by mouse cursor position
    C Vec2 &vel() C { return _vel; }            // get     cursor velocity                      (in Screen Coordinates), velocity is not affected by mouse clipping or  display scale, it's calculated based on few last positions

    BS_FLAG state(Int b) C { return InRange(b, _button) ? _button[b] : BS_NONE; }       // get button 'b' state
    Bool b(Int b) C { return InRange(b, _button) ? ButtonOn(_button[b]) : false; }      // if  button 'b' is on
    Bool bp(Int b) C { return InRange(b, _button) ? ButtonPd(_button[b]) : false; }     // if  button 'b' pushed   in this frame
    Bool br(Int b) C { return InRange(b, _button) ? ButtonRs(_button[b]) : false; }     // if  button 'b' released in this frame
    Bool bd(Int b) C { return InRange(b, _button) ? ButtonDb(_button[b]) : false; }     // if  button 'b' double clicked
    Bool tapped(Int b) C { return InRange(b, _button) ? ButtonTp(_button[b]) : false; } // if  button 'b' tapped, tapping is a single quick push and release of the button without any movement, this can be true at the moment of the release with the condition that there was no movement and the push life was very short
    Bool tappedFirst(Int b) C { return tapped(b) && _first; }                           // if  tapped which was caused by first click of a double-click, double-clicks generate two taps, you can use this method to detect only the first one

    Flt wheel() C { return _wheel.y; }  // get vertical   mouse wheel delta
    Flt wheelX() C { return _wheel.x; } // get horizontal mouse wheel delta

    Int wheelI() C { return _wheel_i.y; }  // get vertical   mouse wheel delta in integer steps
    Int wheelIX() C { return _wheel_i.x; } // get horizontal mouse wheel delta in integer steps

    Dbl startTime() C { return _start_time; }                  // get time when the latest button was pushed, obtained using "Time.appTime()"
    Flt life() C { return Flt(Time.appTime() - _start_time); } // get how  long the latest button is  pushed

    Bool selecting() C { return _selecting; } // if enough                     movement occurred since the latest button was pushed to consider it selecting
    Bool dragging() C { return _dragging; }   // if enough time has passed and movement occurred since the latest button was pushed to consider it dragging

    C VecI2 &windowPos() C { return _window_pixeli; }     // cursor position in Application Window  in Pixel Coordinates
    C VecI2 &desktopPos() C { return _desktop_pixeli; }   // cursor position in System      Desktop in Pixel Coordinates
    C VecI2 &pixelDelta() C { return _delta_pixeli_clp; } // cursor position delta                  in Pixel Coordinates

    Bool hardware() C { return _hardware; }  // if hardware mouse is available
    Bool detected() C { return _detected; }  // if mouse was detected in the system
    Bool onClient() C { return _on_client; } // if mouse is currently on top of the application window client area (and not occluded by other windows)

    Flt speed() C;
    void speed(Flt speed); // get/set mouse movement speed, this affects only 'Ms.d', default=1

    SMOOTH_VALUE_MODE smooth() C { return _sv_delta.mode(); }     // get position delta smoothing, default=SV_WEIGHT4
    void smooth(SMOOTH_VALUE_MODE mode) { _sv_delta.mode(mode); } // set position delta smoothing

    CChar8 *buttonName(Int b) C; // get button name, buttonName(0) -> "Mouse1", buttonName(1) -> "Mouse2", ..

    // test if mouse cursor is in rectangle
    Bool test(C Rect &rect) C { return Cuts(pos(), rect); }

    // cursor movement clipping
    MouseClass &clip(C Rect *rect = null, Int window = -1); // clip mouse cursor to given rectangle, 'window'=if additionally clip to the application window client rectangle (-1=don't change, 0=off, 1=on)
    MouseClass &freeze();                                   // call this in each frame when you want to freeze the mouse cursor position
    Bool frozen() C { return _frozen; }                     // if currently frozen

    // cursor visuals
#if EE_PRIVATE
    void resetCursor();
#endif
    Bool visible() C { return _visible; }                // if     cursor is visible
    Bool hidden() C { return !_visible; }                // if     cursor is hidden
    MouseClass &visible(Bool visible);                   // set    cursor visibility
    MouseClass &toggle() { return visible(!visible()); } // toggle cursor visibility
    MouseClass &show() { return visible(true); }         // show   cursor
    MouseClass &hide() { return visible(false); }        // hide   cursor

    MouseClass &cursor(C ImagePtr &image, C VecI2 &hot_spot = VecI2(0, 0), Bool hardware = true, Bool reset = false); // set cursor image, 'hot_spot'=focus position in image coordinates, 'hardware'=if use hardware cursor (this allows to draw the mouse cursor with full display speed, regardless of the game speed), 'reset'=if reset cursor even if the parameters are same as before (for example if image data has changed). If you change cursors frequently, it's recommended to instead use 'MouseCursor' to avoid overhead. This method has extra overhead however doesn't require to manually create 'MouseCursor' objects.
    MouseClass &cursor(const_mem_addr C MouseCursor *cursor);                                                         // set cursor from an already created cursor, which will avoid some overhead each time a cursor is changed, 'cursor' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)

    // operations
    void eat();           // eat all buttons input from this frame so it will not be processed by the remaining codes in frame, this disables all BS_FLAG states (BS_PUSHED, BS_RELEASED, etc.) except BS_ON
    void eat(Int button); // eat    'button' input from this frame so it will not be processed by the remaining codes in frame, this disables all BS_FLAG states (BS_PUSHED, BS_RELEASED, etc.) except BS_ON
    void eatWheel();      // eat    'wheel'  input from this frame so it will not be processed by the remaining codes in frame

    void simulate() { _detected = true; } // specify that mouse will be manually simulated via 'pos', 'push', 'release' methods, this method will force mouse status as "detected" in the device, even if the mouse is not present

    void push(Byte b);              // manually push    'b' button
    void release(Byte b);           // manually release 'b' button
    void move(C Vec2 &screen_d);    // manually move position by 'screen_d' screen delta
    void moveAbs(C Vec2 &screen_d); // manually move position by 'screen_d' screen delta (unaffected by current display scale)
    void scroll(C Vec2 &d);         // manually apply wheel delta

#if EE_PRIVATE
    // manage
    void del();
    void create();
    void acquire(Bool on);

    // operations
    void clear();
    void _push(Byte b);
    void _release(Byte b);
    void updatePos();
    void update();
    void clipUpdate();
    void clipUpdateConditional();
    void draw();
#endif

#if !EE_PRIVATE
  private:
#endif
    BS_FLAG _button[8];
    Bool _selecting, _dragging, _first, _hardware, _detected, _on_client, _visible, _clip_rect_on, _clip_window, _freeze, _frozen, _action, _want_cur_hw, _locked, _swapped;
    Int _cur;
    Flt _speed;
    Dbl _start_time, _wheel_time;
    Vec2 _pos, _delta_rel_sm, _delta_clp, _delta_rel, _vel, _start_pos, _move_offset, _wheel, _wheel_f;
    VecI2 _window_pixeli, _desktop_pixeli, _delta_pixeli_clp, _wheel_i;
    Rect _clip_rect;
    SmoothValue2 _sv_delta;
    SmoothValueTime2 _sv_vel;
    CChar8 *_button_name[8];
    MouseCursor _cursor_temp;
    C MouseCursor *_cursor;
#if WINDOWS_OLD
#if EE_PRIVATE && MS_DIRECT_INPUT
    IDirectInputDevice8 *_device;
#else
    Ptr _device;
#endif
#endif

    MouseClass();
    NO_COPY_CONSTRUCTOR(MouseClass);
} extern Ms;
inline MouseClass &Mouse = Ms; // 'Mouse' alias ('Mouse' can be used the same way as 'Ms')
/******************************************************************************/
#define MOUSE_IMAGE_SIZE 0.05f

#define MS_BACK 3
#define MS_MAXIMIZE 4
/******************************************************************************/
