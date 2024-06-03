/******************************************************************************/
#include "../../stdafx.h"
/******************************************************************************/

/******************************************************************************/
void LeafRegion::Texture::update(C GuiPC &gpc) {
    super::update(gpc);
    if (gpc.visible) {
        image = (MtrlEdit.game ? MtrlEdit.game->base_0 : ImagePtr());
        if (Gui.ms() == this) {
            bool set = false;
            Memc<LeafAttachment> &attachments = ObjEdit.leaf.attachments;
            if (Ms.bp(1)) {
                if (!Kb.ctrlCmd())
                    attachments.removeLast();
                set = true;
            } // remove only if no Ctrl pressed, so Ctrl+RMB can be used as SET without changing anything (to be used for different objects with existing attachment setup)
            if (Ms.bp(0) || Ms.b(0)) {
                Vec2 screen_pos = pos() + gpc.offset;
                Vec2 tex = (Ms.pos() - screen_pos) / size();
                CHS(tex.y);
                tex.sat();
                if (Ms.bp(0)) {
                    if (!Kb.ctrlCmd())
                        attachments.clear();
                    LeafAttachment &at = attachments.New();
                    at.center = at.attachment = tex;
                    set = true;
                }
                if (attachments.elms()) {
                    LeafAttachment &at = attachments.last();
                    if (at.attachment != tex) {
                        at.attachment = tex;
                        set = true;
                    }
                }
            }
            if (set) {
                bool changed = false;
                REPD(i, ObjEdit.mesh.lods())
                if (ObjEdit.selLod() == i || !ObjEdit.visibleLodSelection()) {
                    MeshLod &lod = ObjEdit.mesh.lod(i);
                    REPA(lod.parts)
                    if (ObjEdit.partOp(i) || !ObjEdit.mesh_parts.visibleOnActiveDesktop()) {
                        MeshPart &part = lod.parts[i];
                        if (HasMaterial(part, MtrlEdit.game)) {
                            ObjEdit.mesh_undos.set("leaf");
                            part.setLeafAttachment(attachments);
                            changed = true;
                        }
                    }
                }
                if (changed)
                    ObjEdit.setChangedMesh(true, false);
            }
        }
    }
}
void LeafRegion::Texture::draw(C GuiPC &gpc) {
    super::draw(gpc);
    Memc<LeafAttachment> &attachments = ObjEdit.leaf.attachments;
    if (attachments.elms()) {
        Rect r = T.rect() + gpc.offset;
        r.swapY();
        FREPA(attachments) {
            LeafAttachment &at = attachments[i];
            Edge2 e;
            e.p[0] = r.lerp(at.center);
            e.p[1] = r.lerp(at.attachment);
            e.p[0].draw(PURPLE);
            e.draw(PURPLE);
        }
    }
}
void LeafRegion::RemoveAttachment(LeafRegion &leaf) { ObjEdit.remVtx(VTX_HLP, true, MtrlEdit.game); }
void LeafRegion::RemoveBending(LeafRegion &leaf) { ObjEdit.remVtx(VTX_SIZE, true, MtrlEdit.game); }
void LeafRegion::RemoveColor(LeafRegion &leaf) { ObjEdit.remVtx(VTX_COLOR, true, MtrlEdit.game); }
void LeafRegion::SetAttachmentCam(LeafRegion &leaf) {
    ObjEdit.mesh_undos.set("leaf");
    bool changed = false;
    Vec pos = ObjEdit.v4.last()->camera.at / ObjEdit.mesh_matrix;
    if (ObjEdit.mesh_parts.edit_selected()) {
        MeshLod &lod = ObjEdit.getLod();
        Memt<int> changed_parts;
        Memt<VecI2> vtxs;
        ObjEdit.getSelectedVtxs(vtxs);
        REPA(vtxs) {
            C VecI2 &v = vtxs[i];
            if (MeshPart *part = lod.parts.addr(v.x))
                if (HasMaterial(*part, MtrlEdit.game)) {
                    MeshBase &base = part->base;
                    if (InRange(v.y, base.vtx)) {
                        if (base.vtx.hlp()) {
                            base.vtx.hlp(v.y) = pos;
                            changed_parts.binaryInclude(v.x);
                        }
                    }
                }
        }
        REPA(changed_parts) {
            MeshPart &part = lod.parts[changed_parts[i]];
            part.setRender();
        }
        if (changed_parts.elms())
            changed = true;
    } else
        REPD(i, ObjEdit.mesh.lods())
    if (ObjEdit.selLod() == i || !ObjEdit.visibleLodSelection()) {
        MeshLod &lod = ObjEdit.mesh.lod(i);
        REPA(lod.parts)
        if (ObjEdit.partOp(i) || !ObjEdit.mesh_parts.visibleOnActiveDesktop()) {
            MeshPart &part = lod.parts[i];
            if (HasMaterial(part, MtrlEdit.game)) {
                part.setLeafAttachment(pos);
                changed = true;
            }
        }
    }
    if (changed)
        ObjEdit.setChangedMesh(true, false);
}
void LeafRegion::RandomBending(LeafRegion &leaf) {
    ObjEdit.mesh_undos.set("bending");
    bool changed = false;
    REPD(i, ObjEdit.mesh.lods())
    if (ObjEdit.selLod() == i || !ObjEdit.visibleLodSelection()) {
        MeshLod &lod = ObjEdit.mesh.lod(i);
        REPA(lod.parts)
        if (ObjEdit.partOp(i) || !ObjEdit.mesh_parts.visibleOnActiveDesktop()) {
            MeshPart &part = lod.parts[i];
            if (HasMaterial(part, MtrlEdit.game) && (part.flag() & VTX_HLP)) // set only for parts that already have leaf attachment
            {
                part.setRandomLeafBending();
                changed = true;
            }
        }
    }
    if (changed)
        ObjEdit.setChangedMesh(true, false);
}
void LeafRegion::SameRandomBending(LeafRegion &leaf) {
    ObjEdit.mesh_undos.set("bending");
    bool changed = false;
    flt random = Random.f(1024);
    if (ObjEdit.mesh_parts.edit_selected()) {
        MeshLod &lod = ObjEdit.getLod();
        Memt<int> changed_parts;
        Memt<VecI2> vtxs;
        ObjEdit.getSelectedVtxs(vtxs);
        REPA(vtxs) {
            C VecI2 &v = vtxs[i];
            if (MeshPart *part = lod.parts.addr(v.x))
                if (HasMaterial(*part, MtrlEdit.game) && (part->flag() & VTX_HLP)) // set only for parts that already have leaf attachment
                {
                    MeshBase &base = part->base;
                    if (InRange(v.y, base.vtx)) {
                        if (!base.vtx.size()) {
                            base.include(VTX_SIZE);
                            ZeroN(base.vtx.size(), base.vtxs());
                        }
                        if (base.vtx.size())
                            base.vtx.size(v.y) = random;
                        changed_parts.binaryInclude(v.x);
                    }
                }
        }
        REPA(changed_parts) {
            MeshPart &part = lod.parts[changed_parts[i]];
            part.setRender();
        }
        if (changed_parts.elms())
            changed = true;
    } else
        REPD(i, ObjEdit.mesh.lods())
    if (ObjEdit.selLod() == i || !ObjEdit.visibleLodSelection()) {
        MeshLod &lod = ObjEdit.mesh.lod(i);
        REPA(lod.parts)
        if (ObjEdit.partOp(i) || !ObjEdit.mesh_parts.visibleOnActiveDesktop()) {
            MeshPart &part = lod.parts[i];
            if (HasMaterial(part, MtrlEdit.game) && (part.flag() & VTX_HLP)) // set only for parts that already have leaf attachment
            {
                part.setRandomLeafBending(random);
                changed = true;
            }
        }
    }
    if (changed)
        ObjEdit.setChangedMesh(true, false);
}
void LeafRegion::RandomColor(LeafRegion &leaf) {
    ObjEdit.mesh_undos.set("color");
    flt variation = TextFlt(leaf.color_value());
    bool changed = false;
    REPD(i, ObjEdit.mesh.lods())
    if (ObjEdit.selLod() == i || !ObjEdit.visibleLodSelection()) {
        MeshLod &lod = ObjEdit.mesh.lod(i);
        REPA(lod.parts)
        if (ObjEdit.partOp(i) || !ObjEdit.mesh_parts.visibleOnActiveDesktop()) {
            MeshPart &part = lod.parts[i];
            if (HasMaterial(part, MtrlEdit.game)) {
                part.setRandomLeafColor(variation);
                changed = true;
            }
        }
    }
    if (changed)
        ObjEdit.setChangedMesh(true, false);
}
bool LeafRegion::meshHasMtrl(C MaterialPtr &mtrl) {
    REPD(i, ObjEdit.mesh.lods()) {
        MeshLod &lod = ObjEdit.mesh.lod(i);
        REPA(lod.parts)
        if (HasMaterial(lod.parts[i], mtrl))
            return true;
    }
    return false;
}
LeafRegion &LeafRegion::create() {
    super::create(Rect_LU(0, 0, 0.44f + 0.12f, 0.793f + 0.12f)).skin(&TransparentSkin, false);
    kb_lit = false;
    ts.reset();
    ts.size = 0.038f;
    ts.align.set(1, 0);
    flt h = 0.044f, p = 0.005f, vh = h * 0.97f, y = -0.025f, w = rect().w() - 0.02f;
    T += leaf_attachment.create(Vec2(0.01f, y), "Set Leaf Attachment", &ts);
    y -= h / 2;
    T += texture.create(Rect(0.01f, y - w, rect().w() - 0.01f, y)).desc("Click on the image to set leaf attachment according to selected texture position.\nClick on approximate leaf/branch center then drag into attachment point.\nHold Ctrl to add new attachments.\nClick RMB to remove last attachment.\nPress Ctrl+RMB to apply attachments to current object.");
    y -= w + h / 2;
    texture.alpha_mode = ALPHA_NONE;
    T += set_attachment_cam.create(Rect_L(0.01f, y, w, vh), "Set At Cam Target").func(SetAttachmentCam, T).desc("This function will set Leaf Attachment at current Camera Target");
    y -= h;
    T += remove_attachment.create(Rect_L(0.01f, y, w, vh), "Del Leaf Attachment").func(RemoveAttachment, T);
    y -= h + p;
    T += random_bending.create(Rect_L(0.01f, y, w, vh), "Set Random Bending").func(RandomBending, T).desc("This function will force each leaf to bend differently");
    y -= h;
    T += same_random_bending.create(Rect_L(0.01f, y, w, vh), "Set Same Random Bending").func(SameRandomBending, T).desc("This function will set the same random bending value for selected mesh parts");
    y -= h;
    T += remove_bending.create(Rect_L(0.01f, y, w, vh), "Del Random Bending").func(RemoveBending, T);
    y -= h + p;
    T += random_color.create(Rect_L(0.01f, y, w - 0.09f, vh), "Set Random Color").func(RandomColor, T).desc("This function will randomize color of each leaf");
    y -= h;
    T += remove_color.create(Rect_L(0.01f, y, w - 0.09f, vh), "Del Random Color").func(RemoveColor, T);
    y -= h;
    T += color_value.create(Rect_L(random_color.rect().right() + Vec2(0.01f, 0), 0.08f, vh), "0.3").desc("Random color variation (0..1)");
    return T;
}
void LeafRegion::clear() { attachments.clear(); }
void LeafRegion::update(C GuiPC &gpc) {
    visible(MtrlEdit.visible() && HasLeaf(MtrlEdit.edit.tech) && meshHasMtrl(MtrlEdit.game));
    super::update(gpc);
    if (Ms.bp(2) && contains(Gui.ms()))
        MtrlEdit.set(null);
}
/******************************************************************************/
