﻿#pragma once
/******************************************************************************/
/******************************************************************************/
class RenameBoneClass : ClosableWindow {
    Str bone_name;
    TextLine textline;

    void create();
    void activate(int bone);
    virtual void update(C GuiPC &gpc) override;
};
/******************************************************************************/
/******************************************************************************/
extern RenameBoneClass RenameBone;
/******************************************************************************/
