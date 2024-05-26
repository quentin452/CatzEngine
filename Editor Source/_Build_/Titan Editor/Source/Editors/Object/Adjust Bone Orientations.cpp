﻿/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
/******************************************************************************

   Usage of 'rotateToDir' instead of setting 'dir' and calling 'fix' gives better precision for cases when we're rotating the bone by 90 deg.

/******************************************************************************/

/******************************************************************************/
cchar8 *AdjustBoneOrns::HandMode[] =
    {
        "Skip",
        "Middle Finger",
        "Average Finger",
        "Parent Bone",
};
cchar8 *AdjustBoneOrns::FootMode[] =
    {
        "Skip",
        "Default",
        "Down",
        "Average Toe",
        "Parent Bone",
};
cchar8 *AdjustBoneOrns::RotShoulder[] =
    {
        "No",
        "Start",
        "Target",
};
/******************************************************************************/
void AdjustBoneOrns::Changed(C Property &prop) { ObjEdit.adjust_bone_orns.refresh(); }
void AdjustBoneOrns::Adjust(AdjustBoneOrns &abo) {
    if (ObjEdit.mesh_skel) {
        ObjEdit.mesh_undos.set("adjustBoneOrns");
        if (abo.adjustDo(*ObjEdit.mesh_skel, &ObjEdit.mesh))
            ObjEdit.setChangedMesh(true, false);
        ObjEdit.setChangedSkel(true);
    }
    abo.hide();
}
bool AdjustBoneOrns::SetTarget(SkelBone &bone, C Vec &pos) {
    Vec dir = pos - bone.pos;
    flt length = dir.normalize();
    if (length > 0.002f) // in case the target is different (it may be the same if bones are located at the same position)
    {
        bone.length = length;
        bone.rotateToDir(dir);
        return true;
    }
    return false;
}
flt AdjustBoneOrns::BoneMeshLength(C SkelBone &bone) {
    flt dist = 0;
    int bone_i = ObjEdit.mesh.boneFind(bone.name);
    if (bone_i >= 0) {
        bone_i += VIRTUAL_ROOT_BONE;
        // convert from world space to original mesh matrix
        Vec pos = bone.pos / ObjEdit.mesh_matrix,
            dir = bone.dir / ObjEdit.mesh_matrix.orn();
        dir /= dir.length2();        // warning: this will be scaled (not normalized), we have to inverse scale, to convert back from mesh space to world space
        MeshLod &lod = ObjEdit.mesh; // use main lod
        REPA(lod) {
            MeshBase &base = lod.parts[i].base;
            if (base.vtx.pos() && base.vtx.matrix() && base.vtx.blend())
                REPA(base.vtx) {
                    C VtxBone &matrix = base.vtx.matrix(i);
                    C VecB4 &blend = base.vtx.blend(i);
                    if (matrix.x == bone_i && blend.x > 0 || matrix.y == bone_i && blend.y > 0 || matrix.z == bone_i && blend.z > 0 || matrix.w == bone_i && blend.w > 0)
                        MAX(dist, DistPointPlane(base.vtx.pos(i), pos, dir));
                }
        }
    }
    return dist;
}
Str8 AdjustBoneOrns::UniqueName(C Skeleton &skel, C Str8 &name) {
    if (skel.findBoneI(name) < 0)
        return name;
    for (int i = 1;; i++) {
        Str8 test = name + '@' + i;
        if (skel.findBoneI(test) < 0)
            return test;
    }
}
void AdjustBoneOrns::SetBoneLength(Skeleton &skel, SkelBone &bone, flt min_length) // set length as Max of all children
{
    bone.length = min_length;

    if (!bone.children_num)
        MAX(bone.length, BoneMeshLength(bone));
    else
        REP(bone.children_num)
    if (SkelBone *child = skel.bones.addr(bone.children_offset + i))
        MAX(bone.length, DistPointPlane(child->pos, bone.pos, bone.dir));
}
bool AdjustBoneOrns::adjustDo(Skeleton &skel, Mesh *mesh) // return if mesh was changed
{
    Box box = skel;
    const flt min_length = Max(0.002f, box.size().max() * 0.0125f), shoulder_frac = 0.25f;

    bool mesh_changed = false, added_bone = false;

    if (add_shoulder) {
        REPA(skel.bones) {
            SkelBone &bone = skel.bones[i];
            C SkelBone *parent = skel.bones.addr(bone.parent);
            if (bone.type == BONE_UPPER_ARM && bone.type_sub == 0 && (!parent || parent->type != BONE_SHOULDER) && skel.bones.elms() + 1 <= BONE_NULL) {
                added_bone = true;
                int bi = skel.bones.elms();
                SkelBone &new_bone = skel.bones.New(), &bone = skel.bones[i]; // !! have to access 'bone' again because it memory address changed
                new_bone.parent = bone.parent;
                bone.parent = bi;
                Set(new_bone.name, UniqueName(skel, S + "Shoulder" + ((bone.type_index >= 0) ? 'R' : 'L') + ((bone.type_index > 0) ? TextInt(bone.type_index) : bone.type_index < -1 ? TextInt(-bone.type_index - 1)
                                                                                                                                                                                     : S)));
                new_bone.type = BONE_SHOULDER;
                Vec from = bone.pos;
                from.x *= shoulder_frac;
                new_bone.setFromTo(from, bone.pos);
            }
        }
    }

    if (added_bone)
        skel.sortBones();

    FREPA(skel.bones) // go from the start to set parents first
    {
        SkelBone &bone = skel.bones[i];
        C SkelBone *parent = skel.bones.addr(bone.parent);

        if (force_head_up && bone.type == BONE_HEAD && bone.type_sub == 0) {
            bone.rotateToDir(Vec(0, 1, 0));
            bone.length = Max(min_length, BoneMeshLength(bone));
        } else if (force_neck_up && bone.type == BONE_NECK) {
            bone.rotateToDir(Vec(0, 1, 0));
            SetBoneLength(skel, bone, min_length);
        } else if (force_spine_up && bone.type == BONE_SPINE) {
            bone.rotateToDir(Vec(0, 1, 0));
            SetBoneLength(skel, bone, min_length);
        } else if (force_eye_forward && (bone.type == BONE_EYE || bone.type == BONE_EYELID || bone.type == BONE_EYEBROW) && bone.type_sub == 0) {
            bone.rotateToDir(Vec(0, 0, 1));
            bone.length = Max(min_length, 2 * BoneMeshLength(bone));
        } else if (force_nose_forward && bone.type == BONE_NOSE && bone.type_sub == 0) {
            bone.rotateToDir(Vec(0, 0, 1));
            bone.length = Max(min_length, BoneMeshLength(bone));
        } else if (force_jaw_forward && bone.type == BONE_JAW && bone.type_sub == 0) {
            bone.rotateToDir(Vec(0, 0, 1));
            bone.length = Max(min_length, BoneMeshLength(bone));
        } else if (force_breast_forward && bone.type == BONE_BREAST && bone.type_sub == 0) {
            bone.rotateToDir(Vec(0, 0, 1));
            SetBoneLength(skel, bone, min_length);
        } else if (force_toe_forward && bone.type == BONE_TOE && bone.type_sub == 0) {
            if (1 && parent) // rotate to parent
            {
                Vec dir = bone.pos - parent->pos;
                dir.y = 0;
                if (dir.normalize())
                    bone.rotateToDir(dir);
            } else // forward
            {
                bone.rotateToDir(Vec(0, 0, 1));
            }
            SetBoneLength(skel, bone, min_length);
        } else if (rotate_shoulder && bone.type == BONE_SHOULDER) {
            FREP(bone.children_num)
            if (C SkelBone *child = skel.bones.addr(bone.children_offset + i))
                if (child->type == BONE_UPPER_ARM) {
                    if (rotate_shoulder == ROT_SHOULDER_SRC) {
                        Vec from = child->pos;
                        from.x *= shoulder_frac;
                        bone.setFromTo(from, child->pos);
                    } else {
                        bone.rotateToDir(Vec(Sign(bone.dir.x), 0, 0));
                        bone.length = Max(min_length, DistPointPlane(child->pos, bone.pos, bone.dir));
                    }
                    break;
                }
        } else if (bone.type == BONE_HAND) {
            switch (hand_mode) {
            case HAND_MIDDLE: {
                MemtN<int, 256> fingers;
                FREP(bone.children_num)
                if (SkelBone *child = skel.bones.addr(bone.children_offset + i))
                    if (child->type == BONE_FINGER)
                        fingers.binaryInclude(int(child->type_index));
                if (fingers.elms()) {
                    int mid = DivRound(fingers.elms() - 1, 2), // use -1 and DivRound to get: 0 for 1 fingers, 1 for 2/3 fingers (to avoid thumbs), and 2 for 4/5 fingers
                        finger_type_index = fingers[mid];
                    FREP(bone.children_num)
                    if (SkelBone *child = skel.bones.addr(bone.children_offset + i))
                        if (child->type == BONE_FINGER && child->type_index == finger_type_index) {
                            SetTarget(bone, child->pos);
                            break;
                        }
                }
            } break;

            case HAND_AVG: {
                Vec pos = 0;
                int num = 0;
                FREP(bone.children_num)
                if (SkelBone *child = skel.bones.addr(bone.children_offset + i))
                    if (child->type == BONE_FINGER) {
                        pos += child->pos;
                        num++;
                    }
                if (num)
                    SetTarget(bone, pos / num);
            } break;

            case HAND_PARENT:
                goto from_parent;
            }
        } else if (bone.type == BONE_FOOT && bone.type_sub == 0 && foot_mode != FOOT_DEFAULT) {
            switch (foot_mode) {
            case FOOT_PARENT:
                if (parent) {
                    bone.rotateToDir(parent->dir);
                    bone.length = Max(min_length, BoneMeshLength(bone));
                }
                break;

            case FOOT_DOWN: {
                bone.rotateToDir(Vec(0, -1, 0));
                if (bone.pos.y > 0)
                    bone.length = Max(0.02f, bone.pos.y); // above ground, for feet use a small min_length in case the bone.length is actually used for something
                else
                    bone.length = Max(0.02f, BoneMeshLength(bone));
            } break;

            case FOOT_AVG: {
                Vec pos = 0;
                int num = 0;
                FREP(bone.children_num)
                if (SkelBone *child = skel.bones.addr(bone.children_offset + i))
                    if (child->type == BONE_TOE) {
                        pos += child->pos;
                        num++;
                    }
                if (num)
                    SetTarget(bone, pos / num);
            } break;
            }
        } else if (zero_child && bone.children_num == 0) {
            if (bone.type == BONE_BREAST) {
                bone.rotateToDir(Vec(0, 0, 1));
                bone.length = Max(min_length, BoneMeshLength(bone));
            } else if (bone.type == BONE_BUTT) {
                bone.rotateToDir(Vec(0, 0, -1));
                bone.length = Max(min_length, BoneMeshLength(bone));
            } else if (parent) {
                bone.rotateToDir(parent->dir);
                bone.length = Max(min_length, BoneMeshLength(bone));
            }
        } else if (one_child && bone.children_num == 1) {
            if (SkelBone *child = skel.bones.addr(bone.children_offset))
                SetTarget(bone, child->pos);
        } else if (multi_child && bone.children_num > 1) {
            // check if all children share the same position
            {
                Vec child_pos = 0;
                FREP(bone.children_num)
                if (SkelBone *child = skel.bones.addr(bone.children_offset + i)) {
                    if (!i)
                        child_pos = child->pos;
                    else if (!Equal(child_pos, child->pos))
                        goto different;
                }
                if (SetTarget(bone, child_pos))
                    goto next;
            different:;
            }

            // get main child type
            BONE_TYPE child_type;
            switch (bone.type) {
            default:
                child_type = BONE_UNKNOWN;
                break;
            case BONE_SPINE:
                child_type = BONE_NECK;
                break;
            case BONE_SHOULDER:
                child_type = BONE_UPPER_ARM;
                break;
            case BONE_UPPER_ARM:
                child_type = BONE_LOWER_ARM;
                break;
            case BONE_LOWER_ARM:
                child_type = BONE_HAND;
                break;
            case BONE_NECK:
                child_type = BONE_HEAD;
                break;
            case BONE_UPPER_LEG:
                child_type = BONE_LOWER_LEG;
                break;
            case BONE_LOWER_LEG:
                child_type = BONE_FOOT;
                break;
            }

            // first check if there's any child of main child type directly connected to this bone (this is for cases when processing ARM that has children ARM TWIST and FOREARM, there we want to set target as FOREARM and not ARM TWIST)
            FREP(bone.children_num)
            if (SkelBone *child = skel.bones.addr(bone.children_offset + i))
                if (child->type == child_type)
                    if (SetTarget(bone, child->pos))
                        goto next;

            // try setting from main sub child first
            if (bone.type && bone.type != BONE_SHOULDER) // avoid things like shoulder pads
                if (SkelBone *child = skel.findBone(bone.type, bone.type_index, bone.type_sub + 1))
                    if (SetTarget(bone, child->pos))
                        goto next;

            // try setting from main child type
            if (child_type)
                if (SkelBone *child = skel.findBone(child_type, bone.type_index))
                    if (SetTarget(bone, child->pos))
                        goto next;

            // try setting from secondary child type
            switch (bone.type) {
            default:
                child_type = BONE_UNKNOWN;
                break;
            case BONE_SPINE:
                child_type = BONE_HEAD;
                break;
            }
            if (child_type)
                if (SkelBone *child = skel.findBone(child_type, bone.type_index))
                    if (SetTarget(bone, child->pos))
                        goto next;

            // try setting from parent
        from_parent:
            if (parent) {
                bone.rotateToDir(parent->dir);
                SetBoneLength(skel, bone, min_length);
                goto next;
            }
        }
    next:;
    }
    if (reset_perp)
        FREPA(skel.bones) // process parents first
        {
            SkelBone &bone = skel.bones[i];
            if (bone.type) {
                C SkelBone *parent = skel.bones.addr(bone.parent);
                switch (bone.type) {
                case BONE_SPINE:
                case BONE_NECK:
                case BONE_HEAD:
                    bone.perp.set(0, -1, 1);
                    break; // use diagonal because of animals standing on 4 feet

                case BONE_UPPER_LEG:
                case BONE_LOWER_LEG:
                    bone.perp.set(0, 0, 1);
                    break;

                case BONE_FOOT: {
                    bone.perp.set(0, 0, 1);
                    if (1 && bone.children_num) // optional, rotate towards children
                    {
                        Vec child_pos = 0;
                        int children_num = 0;
                        REP(bone.children_num)
                        if (SkelBone *child = skel.bones.addr(bone.children_offset + i))
                            if (child->type == BONE_TOE) {
                                child_pos += child->pos;
                                children_num++;
                            }
                        if (children_num) {
                            child_pos /= children_num;
                            bone.perp = child_pos - bone.pos;
                        }
                    }
                } break;

                case BONE_CAPE:
                    bone.perp.set(0, 0, -1);
                    break;
                default:
                    bone.perp.set(0, 1, 0);
                    break;

                case BONE_UPPER_ARM:
                case BONE_LOWER_ARM:
                case BONE_HAND:
                case BONE_FINGER: {
                    if (parent) {
                        Orient temp = *parent;
                        temp.rotateToDir(bone.dir);
                        bone.perp = temp.perp;
                    } else {
                        // set as vertical perpendicular because some characters have arms downwards
                        // if bone.dir.y=0 then (0,1,0), if bone.dir.y=-1 then (1,0,0) for right and (-1,0,0) for left
                        flt x = bone.dir.y * SignBool(bone.type_index < 0);
                        bone.perp.set(x, CosSin(x), 0);
                    }
                } break;
                }
                bone.fixPerp();
            }
        }
    skel.setBoneTypes(); // bone orientation may affect bone type indexes
    if (mesh)
        if (1 || added_bone) // bone orientation could have changed bone type indexes or we could've added a new bone
        {
            mesh->skeleton(&skel, true).skeleton(null);
            mesh_changed = true;
        }
    return mesh_changed;
}
Skeleton &AdjustBoneOrns::getSkel() {
    if (refresh_needed) {
        refresh_needed = false;
        if (ObjEdit.mesh_skel)
            skel = *ObjEdit.mesh_skel;
        else
            skel.del();
        adjustDo(skel);
    }
    return skel;
}
void AdjustBoneOrns::refresh() { refresh_needed = true; }
AdjustBoneOrns &AdjustBoneOrns::create() {
    add("Bones with 0 children", MEMBER(AdjustBoneOrns, zero_child));
    add("Bones with 1 child", MEMBER(AdjustBoneOrns, one_child));
    add("Bones with >1 children", MEMBER(AdjustBoneOrns, multi_child));
    add("Force Head Up", MEMBER(AdjustBoneOrns, force_head_up));
    add("Force Neck Up", MEMBER(AdjustBoneOrns, force_neck_up));
    add("Force Spine Up", MEMBER(AdjustBoneOrns, force_spine_up));
    add("Force Eye Forward", MEMBER(AdjustBoneOrns, force_eye_forward));
    add("Force Nose Forward", MEMBER(AdjustBoneOrns, force_nose_forward));
    add("Force Jaw Forward", MEMBER(AdjustBoneOrns, force_jaw_forward));
    add("Force Breast Forward", MEMBER(AdjustBoneOrns, force_breast_forward));
    add("Force Toe Forward", MEMBER(AdjustBoneOrns, force_toe_forward));
    add("Hand", MEMBER(AdjustBoneOrns, hand_mode)).setEnum(HandMode, Elms(HandMode));
    add("Foot", MEMBER(AdjustBoneOrns, foot_mode)).setEnum(FootMode, Elms(FootMode));
    add("Add Shoulder", MEMBER(AdjustBoneOrns, add_shoulder));
    add("Rotate Shoulder", MEMBER(AdjustBoneOrns, rotate_shoulder)).setEnum(RotShoulder, Elms(RotShoulder));
    add("Reset Bone Perpendicular", MEMBER(AdjustBoneOrns, reset_perp));
    Rect r = super::create("Adjust Bone Orientations", Vec2(0.02f, -0.02f), 0.040f, 0.046f);
    button[2].show();
    autoData(this).changed(Changed);
    resize(Vec2(0, 0.1f));
    T += adjust.create(Rect_D(clientWidth() / 2, -clientHeight() + 0.03f, 0.2f, 0.06f), "Adjust").func(Adjust, T);
    hide();
    return T;
}
AdjustBoneOrns::AdjustBoneOrns() : zero_child(true), one_child(true), multi_child(true), force_eye_forward(true), force_nose_forward(true), force_jaw_forward(true), force_breast_forward(true), force_spine_up(true), force_neck_up(true), force_head_up(true), force_toe_forward(true), add_shoulder(false), reset_perp(true), refresh_needed(true), hand_mode(HAND_AVG), foot_mode(FOOT_DOWN), rotate_shoulder(ROT_SHOULDER_NO) {}

/******************************************************************************/
