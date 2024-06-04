﻿#pragma once
/******************************************************************************/
/******************************************************************************/
class ReplaceNameClass : ClosableWindow {
    Memc<UID> elm_ids;
    Text t_from, t_to;
    TextLine from, to;

    void create();
    void activate(C MemPtr<UID> &elm_ids);
    virtual void update(C GuiPC &gpc) override;
};
/******************************************************************************/
/******************************************************************************/
extern ReplaceNameClass ReplaceName;
/******************************************************************************/
