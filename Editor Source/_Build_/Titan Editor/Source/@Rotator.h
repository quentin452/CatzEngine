﻿#pragma once
/******************************************************************************/
/******************************************************************************/
class Rotator : GuiCustom {
    Vec2 delta;

    virtual GuiObj *test(C GuiPC &gpc, C Vec2 &pos, GuiObj *&mouse_wheel) override;
    virtual void update(C GuiPC &gpc) override;
    virtual void draw(C GuiPC &gpc) override;

  public:
    Rotator();
};
/******************************************************************************/
/******************************************************************************/
extern Rotator Rot;
/******************************************************************************/
