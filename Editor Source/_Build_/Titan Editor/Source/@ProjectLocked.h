﻿#pragma once
/******************************************************************************/
/******************************************************************************/
class ProjectLocked : ClosableWindow {
    static void OK(ProjectLocked &pl);

    TextNoTest text;
    Button ok, cancel;
    UID proj_id;

    void create(C UID &proj_id);

  public:
    ProjectLocked();
};
/******************************************************************************/
/******************************************************************************/
extern ProjectLocked ProjectLock;
/******************************************************************************/
