﻿#pragma once
/******************************************************************************/
/******************************************************************************/
class ElmIconSetts : ElmData {
    ElmImage::TYPE type;

    virtual bool mayContain(C UID &id) C override;

    // operations
    void from(C IconSettings &src);
    uint undo(C ElmIconSetts &src); // don't adjust 'ver' here because it also relies on 'IconSettings', because of that this is included in 'ElmFileInShort'
    uint sync(C ElmIconSetts &src); // don't adjust 'ver' here because it also relies on 'IconSettings', because of that this is included in 'ElmFileInShort'

    // io
    virtual bool save(File &f) C override;
    virtual bool load(File &f) override;
    virtual void save(MemPtr<TextNode> nodes) C override;
    virtual void load(C MemPtr<TextNode> &nodes) override;

  public:
    ElmIconSetts();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
