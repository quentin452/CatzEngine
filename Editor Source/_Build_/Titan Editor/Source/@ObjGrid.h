﻿#pragma once
/******************************************************************************/
/******************************************************************************/
class ObjGrid : Region {
    Button bxz, by;
    TextLine xz, y;

    ObjGrid &create();
    virtual void update(C GuiPC &gpc) override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
