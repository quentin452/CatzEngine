#pragma once
/******************************************************************************/
/******************************************************************************/
class LeafRegion : Region {
    class Texture : ImageSkin {
        virtual void update(C GuiPC &gpc) override;
        virtual void draw(C GuiPC &gpc) override;
    };
    TextWhite ts;
    Text leaf_attachment;
    Texture texture;
    Button remove_attachment, set_attachment_cam, random_bending, same_random_bending, remove_bending, random_color, remove_color;
    TextLine color_value;
    Memc<LeafAttachment> attachments;

    static void RemoveAttachment(LeafRegion &leaf);
    static void RemoveBending(LeafRegion &leaf);
    static void RemoveColor(LeafRegion &leaf);
    static void SetAttachmentCam(LeafRegion &leaf);
    static void RandomBending(LeafRegion &leaf);
    static void SameRandomBending(LeafRegion &leaf);
    static void RandomColor(LeafRegion &leaf);
    bool meshHasMtrl(C MaterialPtr &mtrl);

    LeafRegion &create();
    void clear();
    virtual void update(C GuiPC &gpc) override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
