﻿#pragma once
/******************************************************************************/
/******************************************************************************/
class EE1EditWaypoint : Game::Waypoint, EE1ObjGlobal {
    UID id;
    Str name;

    bool loadData(File &f);
    bool load(File &f, C Str &name);

  public:
    EE1EditWaypoint();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
