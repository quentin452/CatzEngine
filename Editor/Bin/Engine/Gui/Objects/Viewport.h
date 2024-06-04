#pragma once
/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************/
const_mem_addr struct Viewport : GuiObj // Viewport !! must be stored in constant memory address !!
{
    // saved
    Color rect_color; // rectangle color, default=Gui.borderColor

    // following members are not saved in the 'save' method
    FOV_MODE fov_mode;             // field of view mode      , default=D.viewFovMode()
    Flt fov,                       // field of view           , default=D.viewFov    ()
        from,                      // viewport near clip plane, default=D.viewFrom   ()
        range;                     // viewport far  clip plane, default=D.viewRange  ()
    void (*draw_func)(Viewport &); // pointer to drawing function

    // manage
    virtual Viewport &del() override;                                   // delete
    Viewport &create(void (*draw)(Viewport &) = null, Ptr user = null); // create
    Viewport &create(C Rect &rect, void (*draw)(Viewport &) = null, Ptr user = null) {
        create(draw, user).rect(rect);
        return T;
    }                                  // create
    Viewport &create(C Viewport &src); // create from 'src'

    // operations
    void setDisplayView() C; // activate display viewport settings from current viewport parameters

    // main
    virtual GuiObj *test(C GuiPC &gpc, C Vec2 &pos, GuiObj *&mouse_wheel) override; // test if 'pos' screen position intersects with the object, by returning pointer to object or its children upon intersection and null in case no intersection, 'mouse_wheel' may be modified upon intersection either to the object or its children or null
    virtual void draw(C GuiPC &gpc) override;                                       // draw object

#if EE_PRIVATE
    void zero();
    void reset();
#endif

    ~Viewport() { del(); }
    Viewport();

  protected:
    virtual Bool save(File &f, CChar *path = null) C override;
    virtual Bool load(File &f, CChar *path = null) override;
};
/******************************************************************************/
