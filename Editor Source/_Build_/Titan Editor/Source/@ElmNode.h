﻿#pragma once
/******************************************************************************/
class ElmNode {
    bool added;         // if this element was added
    byte flag;          // ELM_FLAG
    int parent;         // index of parent in hierarchy
    Memc<int> children; // valid indexes of elms

    void clear();

  public:
    ElmNode();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
