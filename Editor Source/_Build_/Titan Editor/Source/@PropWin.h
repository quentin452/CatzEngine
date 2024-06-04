#pragma once
/******************************************************************************/
class PropWin : ClosableWindow, PropExs {
    TextBlack ts;

    Rect create(C Str &name, C Vec2 &lu = Vec2(0.02f, -0.02f), C Vec2 &text_scale = 0.036f, flt property_height = 0.043f, flt value_width = 0.3f);
    PropWin &autoData(ptr data);
    ptr autoData() C;

    virtual PropWin &hide() override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
