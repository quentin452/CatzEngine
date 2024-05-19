/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************/
enum SLIDEBAR_BACK_CLICK_MODE : Byte // action when clicking on the background of the SlideBar
{
    SBC_OFF,     // nothing happens
    SBC_STEP,    // one time scroll   will be performed by a single step
    SBC_SMOOTH,  // smooth scrolling  will be performed as long as the click is hold
    SBC_SET_POS, // slidebar position will be immediately set to the click position
};
enum SLIDEBAR_BUTTON : Byte {
    SB_MIDDLE,
    SB_LEFT_UP,
    SB_RIGHT_DOWN,
};
enum SCROLL : Byte {
    SCROLL_STOP, // stop scroll
    SCROLL_REL,  // relative - keep scrolling by the same delta
    SCROLL_ABS,  // absolute - keep scrolling to the same target
};
const_mem_addr struct SlideBar : GuiObj // Gui SlideBar !! must be stored in constant memory address !!
{
    Button button[3];             // 3 SLIDEBAR_BUTTON buttons
    SLIDEBAR_BACK_CLICK_MODE sbc; // default=SBC_STEP

    // manage
    virtual SlideBar &del() override;                              // delete
    SlideBar &create();                                            // create
    SlideBar &create(C Rect &rect) { return create().rect(rect); } // create and set rectangle
    SlideBar &create(C SlideBar &src);                             // create from 'src'

    // set / get
    SlideBar &setLengths(Flt length, Flt length_total);                                     // set, 'length'=covered length, 'length_total'=total length
    SlideBar &set(Flt step, SET_MODE mode = SET_DEFAULT, SCROLL scroll = SCROLL_STOP);      // set slidebar step                 (0..1)
    Flt operator()() C;                                                                     // get slidebar step                 (0..1)
    Flt wantedOffset() C;                                                                   // get slidebar offset target        (0..length_total-length), if slidebar is currently scrolling then this returns target offset
    Flt offset() C { return _offset; }                                                      // get slidebar offset at the moment (0..length_total-length)
    SlideBar &offset(Flt offset, SET_MODE mode = SET_DEFAULT, SCROLL scroll = SCROLL_STOP); // set slidebar offset               (0..length_total-length)
    Flt maxOffset() C { return _length_total - _length; }                                   // get slidebar max possible offset
    Flt length() C { return _length; }                                                      // get slidebar length
    Flt lengthTotal() C { return _length_total; }                                           // get slidebar length total
    Bool vertical() C { return _vertical; }                                                 // if  slidebar is   vertical
    Bool horizontal() C { return !_vertical; }                                              // if  slidebar is horizontal
    SlideBar &skin(C GuiSkinPtr &skin);                                                     // set skin override, default=null (if set to null then current value of 'Gui.skin' is used), changing this value will automatically change the skin of the SlideBar buttons
    C GuiSkinPtr &skin() C { return _skin; }                                                // get skin override, default=null (if set to null then current value of 'Gui.skin' is used)
    GuiSkin *getSkin() C { return _skin ? _skin() : Gui.skin(); }                           // get actual skin

    Bool atEnd(Flt eps = EPS) C { return offset() + eps >= maxOffset(); }             // if slidebar is              at the end
    Bool wantedAtEnd(Flt eps = EPS) C { return wantedOffset() + eps >= maxOffset(); } // if slidebar is wanted to be at the end

    SlideBar &func(void (*func)(Ptr user), Ptr user = null, Bool immediate = true); // set function called when value has changed, with 'user' as its parameter
    T1(TYPE)
    SlideBar &func(void (*func)(TYPE *user), TYPE *user, Bool immediate = true) { return T.func((void (*)(Ptr))func, user, immediate); } // set function called when value has changed, with 'user' as its parameter
    T1(TYPE)
    SlideBar &func(void (*func)(TYPE &user), TYPE &user, Bool immediate = true) { return T.func((void (*)(Ptr))func, &user, immediate); } // set function called when value has changed, with 'user' as its parameter

    virtual SlideBar &desc(C Str &desc) override;
    C Str &desc() C { return super::desc(); } // set/get description

    SlideBar &focusable(Bool on);
    Bool focusable() C { return _focusable; } // set/get if can catch keyboard focus, default=true

    // scroll
    Bool scrolling() C { return _scroll; } // if  currently scrolling
    Flt scrollDelta() C;                   // get amount of scroll that's still left to be done

    SlideBar &scroll(Flt delta, Bool immediate = false);           // scroll by delta
    SlideBar &scrollTo(Flt pos, Bool immediate = false);           // scroll to pos
    SlideBar &scrollFit(Flt min, Flt max, Bool immediate = false); // scroll to fit min..max range
    SlideBar &scrollEnd(Bool immediate = false);                   // scroll to end

    SlideBar &scrollLeftUp();    // scroll left/up
    SlideBar &scrollRightDown(); // scroll right/down

    SlideBar &scrollOptions(Flt relative = 0.5f, Flt base = 0, Bool immediate = false, Flt button_speed = 1.5f); // set scrolling options, 'relative'=amount of scrolling using the mouse wheel relative to slidebar 'length' (0..Inf, default=1), 'base'=constant amount of scrolling using mouse wheel (0..Inf, default=0), 'immediate'=if mouse wheel scrolling is immediate or smooth, 'button_speed'=speed of scrolling upon pressing the left/up/right/down buttons

    // operations
    virtual SlideBar &rect(C Rect &rect) override;
    C Rect &rect() C { return super::rect(); }      // set/get rectangle
    virtual SlideBar &move(C Vec2 &delta) override; // move by delta

    SlideBar &removeSideButtons(); // remove side buttons (left/up and right/down) leaving only the middle button

    // main
    virtual GuiObj *test(C GuiPC &gpc, C Vec2 &pos, GuiObj *&mouse_wheel) override; // test if 'pos' screen position intersects with the object, by returning pointer to object or its children upon intersection and null in case no intersection, 'mouse_wheel' may be modified upon intersection either to the object or its children or null
    virtual void update(C GuiPC &gpc) override;                                     // update object
    virtual void draw(C GuiPC &gpc) override;                                       // draw   object

#if EE_PRIVATE
    void zero();
    void setButtonSubType();
    void setParams();
    void setButtonRect();
    void call();
#endif

    ~SlideBar() { del(); }
    SlideBar();

#if !EE_PRIVATE
  private:
#endif
    Bool _scroll_immediate, _scroll, _vertical, _usable, _focusable, _func_immediate;
    Flt _scroll_to, _offset, _length, _length_total, _scroll_mul, _scroll_add, _scroll_button, _button_size;
    Ptr _func_user;
    void (*_func)(Ptr user);
    GuiSkinPtr _skin;

  protected:
    virtual Bool save(File &f, CChar *path = null) C override;
    virtual Bool load(File &f, CChar *path = null) override;

    NO_COPY_CONSTRUCTOR(SlideBar);
#if EE_PRIVATE
    friend struct Region;
    friend struct TextBox;
#endif
};
/******************************************************************************/
