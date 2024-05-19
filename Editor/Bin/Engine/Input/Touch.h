/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Touches' container to access all active touches.

/******************************************************************************/
struct Touch // Single Touch on a Touch-Screen
{
    UInt user_type; // user data, default=0   , this can be freely modified, for example you can set this to a custom type    when the touch is pushed ('pd' method)
    Ptr user_ptr;   // user data, default=null, this can be freely modified, for example you can set this to a custom pointer when the touch is pushed ('pd' method)

    C Vec2 &startPos() C { return _start_pos; } // starting position, this is equal to the touch position at the moment of its start
    C Vec2 &pos() C { return _pos; }            // current  position
    C Vec2 &smoothPos() C { return _sm_pos; }   // current  position with smoothing applied
    C Vec2 &d() C { return _delta; }            //          position delta since last frame
    C Vec2 &ad() C { return _abs_delta; }       // absolute position delta since last frame (unaffected by current display scale)
    C Vec2 &vel() C { return _vel; }            // absolute velocity                        (unaffected by current display scale), it's calculated based on few last positions

    C VecI2 &pixelDelta() C { return _delta_pixeli_clp; } // cursor position delta in Pixel Coordinates

    BS_FLAG state() C { return _state; }                // get state
    Bool on() C { return ButtonOn(_state); }            // if is on
    Bool pd() C { return ButtonPd(_state); }            // if pushed   in this frame
    Bool rs() C { return ButtonRs(_state); }            // if released in this frame
    Bool db() C { return ButtonDb(_state); }            // if double clicked
    Bool tapped() C { return ButtonTp(_state); }        // if tapped, tapping is a single quick push and release without any movement, this can be true at the moment of the release with the condition that there was no movement and the touch life was very short
    Bool tappedFirst() C { return tapped() && _first; } // if tapped which was caused by first click of a double-click, double-clicks generate two taps, you can use this method to detect only the first one
    Bool longPress() C { return ButtonLp(_state); }     // if this touch is pressed for a long time without movement

    Dbl startTime() C { return _start_time; } // time when the touch has started, obtained using "Time.appTime()"
    Flt life() C { return _life; }            // how  long the touch is  active
#if EE_PRIVATE
    Flt Life() C { return Flt(Time.appTime() - _start_time); } // how long the touch is active
#endif

    Bool selecting() C { return _selecting; } // if enough                     movement occurred since the touch start to consider it selecting
    Bool dragging() C { return _dragging; }   // if enough time has passed and movement occurred since the touch start to consider it dragging
    Bool scrolling() C { return _scrolling; } // if this touch is currently being used to scroll a Region

    GuiObj *guiObj() C { return _gui_obj; }      // get gui object at touch position when the touch was started
    void guiObj(GuiObj *obj) { _gui_obj = obj; } // manually change the gui object for this touch

    UInt id() C { return _id; } // get unique ID which was initialized at the start of the touch

    Bool stylus() C { return _stylus; } // if this touch is generated with a stylus

    // operations
    void eat();           // eat input state of this touch from this frame so it will not be processed by the remaining codes in frame, this disables all BS_FLAG states (BS_PUSHED, BS_RELEASED, etc.) except BS_ON
    void disableScroll(); // disable scrolling by this touch

    Touch();

#if !EE_PRIVATE
  private:
#endif
    Bool _selecting, _dragging, _allow_scrolling, _scrolling, _stylus, _first, _remove;
    Byte _axis_moved;
    BS_FLAG _state;
    UInt _id;
    Flt _life;
    Dbl _start_time;
    VecI2 _pixeli, _delta_pixeli_clp;
    Vec2 _start_pos, _prev_pos, _pos, _sm_pos, _delta, _abs_delta, _vel;
    CPtr _handle;
    GuiObj *_gui_obj;
    SmoothValue2 _sv_pos;
    SmoothValueTime2 _sv_vel;

#if EE_PRIVATE
    Touch &init(C VecI2 &pixeli, C Vec2 &pos, CPtr handle, Bool stylus);
    Touch &reinit(C VecI2 &pixeli, C Vec2 &pos);
#endif
};

extern MemtN<Touch, 10> Touches; // container of active Touches

Touch *FindTouch(UInt id);     // find touch in 'Touches' container according to its 'id', null on fail
void SimulateTouches(Bool on); // if enabled then pressing mouse button on the screen will simulate the 'Touches' behavior, enabling you to develop touch based applications (for example iPhone) without the need of a touch screen but just a mouse (on a PC/Mac). In the iPhone SDK simulator the touches are always simulated using the mouse (there's no need to call this function). default=false
Bool SupportedTouches();       // if touch input is supported

#if EE_PRIVATE
extern Bool TouchesSupported;

Touch *FindTouchByHandle(CPtr handle); // find touch in 'Touches' container according to its '_handle', null on fail

void TouchesUpdate();
void TouchesClear();
#endif
/******************************************************************************/
struct MouseTouch // Mouse and Touch input combined into one class
{
    static C Vec2 &startPos(Int i) { return InRange(i, Touches) ? Touches[i].startPos() : Ms.startPos(); }        // get starting position                            of i-th touch or mouse
    static C Vec2 &pos(Int i) { return InRange(i, Touches) ? Touches[i].pos() : Ms.pos(); }                       // get current  position                            of i-th touch or mouse
    static C Vec2 &smoothPos(Int i) { return InRange(i, Touches) ? Touches[i].smoothPos() : Ms.pos(); }           // get current  position with smoothing applied     of i-th touch or mouse
    static C Vec2 &d(Int i) { return InRange(i, Touches) ? Touches[i].d() : Ms.d(); }                             // get          position delta                      of i-th touch or mouse
    static C Vec2 &ad(Int i) { return InRange(i, Touches) ? Touches[i].ad() : Ms.d(); }                           // get absolute position delta                      of i-th touch or mouse
    static C Vec2 &dc(Int i) { return InRange(i, Touches) ? Touches[i].d() : Ms.dc(); }                           // get          position delta clipped              of i-th touch or mouse
    static C Vec2 &vel(Int i) { return InRange(i, Touches) ? Touches[i].vel() : Ms.vel(); }                       // get          velocity                            of i-th touch or mouse
    static C VecI2 &pixelDelta(Int i) { return InRange(i, Touches) ? Touches[i].pixelDelta() : Ms.pixelDelta(); } // get          position delta in Pixel Coordinates of i-th touch or mouse

    static BS_FLAG state(Int i, Int b = 0) { return InRange(i, Touches) ? (b == 0 ? Touches[i].state() : BS_NONE) : Ms.state(b); }       // get    button 'b' state          of i-th touch or mouse
    static Bool b(Int i, Int b = 0) { return InRange(i, Touches) ? (b == 0 && Touches[i].on()) : Ms.b(b); }                              // get if button 'b' on             of i-th touch or mouse
    static Bool bp(Int i, Int b = 0) { return InRange(i, Touches) ? (b == 0 && Touches[i].pd()) : Ms.bp(b); }                            // get if button 'b' pushed         of i-th touch or mouse
    static Bool br(Int i, Int b = 0) { return InRange(i, Touches) ? (b == 0 && Touches[i].rs()) : Ms.br(b); }                            // get if button 'b' released       of i-th touch or mouse
    static Bool bd(Int i, Int b = 0) { return InRange(i, Touches) ? (b == 0 && Touches[i].db()) : Ms.bd(b); }                            // get if button 'b' double clicked of i-th touch or mouse
    static Bool tapped(Int i, Int b = 0) { return InRange(i, Touches) ? (b == 0 && Touches[i].tapped()) : Ms.tapped(b); }                // if tapped, tapping is a single quick push and release without any movement, this can be true at the moment of the release with the condition that there was no movement and the push life was very short
    static Bool tappedFirst(Int i, Int b = 0) { return InRange(i, Touches) ? (b == 0 && Touches[i].tappedFirst()) : Ms.tappedFirst(b); } // if tapped which was caused by first click of a double-click, double-clicks generate two taps, you can use this method to detect only the first one

    static Dbl startTime(Int i) { return InRange(i, Touches) ? Touches[i].startTime() : Ms.startTime(); } // time of when the main button was pressed, obtained using "Time.appTime()"
    static Flt life(Int i) { return InRange(i, Touches) ? Touches[i].life() : Ms.life(); }                // how long     the main button is  active

    static Bool selecting(Int i) { return InRange(i, Touches) ? Touches[i].selecting() : Ms.selecting(); } // if enough                     movement occurred since the button was pressed to consider it selecting
    static Bool dragging(Int i) { return InRange(i, Touches) ? Touches[i].dragging() : Ms.dragging(); }    // if enough time has passed and movement occurred since the button was pressed to consider it dragging

    static Bool mouse(Int i) { return InRange(i, Touches) ? false : true; }                   // if  input is from a mouse
    static Touch *touch(Int i) { return InRange(i, Touches) ? &Touches[i] : null; }           // get touch linked with i-th input
    static UInt id(Int i) { return InRange(i, Touches) ? Touches[i].id() : 0; }               // get unique ID which was initialized at the start of the touch
    static Bool hoverable(Int i) { return InRange(i, Touches) ? Touches[i].stylus() : true; } // if  input is hoverable (it's a stylus or a mouse)

    static GuiObj *guiObj(Int i) { return InRange(i, Touches) ? Touches[i].guiObj() : Gui.ms(); } // get gui object focus of i-th touch or mouse
    static void guiObj(Int i, GuiObj *obj);                                                       // manually change the gui object for i-th touch or mouse

    static void freeze(Int i) {
        if (!InRange(i, Touches))
            Ms.freeze();
    }

    static void disableScroll(Int i) {
        if (InRange(i, Touches))
            Touches[i].disableScroll();
    }

    static void eat(Int i) { return InRange(i, Touches) ? Touches[i].eat() : Ms.eat(); }         // eat any button  input of i-th touch or mouse from this frame so it will not be processed by the remaining codes in frame, this disables all BS_FLAG states (BS_PUSHED, BS_RELEASED, etc.) except BS_ON
    static void eat(Int i, Int b) { return InRange(i, Touches) ? Touches[i].eat() : Ms.eat(b); } // eat    'button' input of i-th touch or mouse from this frame so it will not be processed by the remaining codes in frame, this disables all BS_FLAG states (BS_PUSHED, BS_RELEASED, etc.) except BS_ON

    static Int elms() { return Touches.elms() + Ms.detected(); } // all touches + mouse
} extern MT;
inline MouseTouch &MouseTouches = MT; // 'MouseTouches' alias ('MouseTouches' can be used the same way as 'MT')

inline Int Elms(C MouseTouch &mt) { return mt.elms(); }
/******************************************************************************/
