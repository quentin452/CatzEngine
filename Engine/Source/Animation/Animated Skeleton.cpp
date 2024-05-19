/******************************************************************************/
#include "stdafx.h"
namespace EE {
#define WORLD_POS 0
#define FUR 0
/******************************************************************************/
// ANIM SKEL BONE
/******************************************************************************/
void AnimatedSkeletonBone::clear() {
    orn.zero();
    pos.zero();
    scale.zero();
#if HAS_ANIM_SKEL_ROT
    rot.zero();
#endif
#if HAS_ANIM_COLOR
    color.set(1);
#endif
    _force_matrix = _world_space_transform = false;
}
void AnimatedSkeletonBone::keep01(Flt blend) {
    orn *= blend;
    pos *= blend;
    scale *= blend;
#if HAS_ANIM_SKEL_ROT
    rot *= blend;
#endif
#if HAS_ANIM_COLOR
    color *= blend;
    color += 1 - blend; // color=Lerp(Vec4(1), color, blend)
#endif
    if (_world_space_transform)
        _world_space_transform_matrix.keep01(blend);
}
void AnimatedSkeletonBone::clear(Flt blend) {
    if (blend > 0) {
        if (blend >= 1)
            clear();
        else
            keep01(1 - blend);
    }
}
/*
   Flt                     fur_stiffness, // determines the speed of                fur velocities changes,    0..1  , default= 0.0001
                           fur_gravity  , // gravity which affects                  fur velocities        , -Inf..Inf, default=-1
                           fur_vel_scale; // how much does skeleton movement affect fur velocities        , -Inf..Inf, default=-0.75

 C Vec    &         vel  (     )C {return root.    _vel                      ;} // get root         velocity
 C Vec    &      angVel  (     )C {return root._ang_vel                      ;} // get root angular velocity

// the following parameters are valid only after calling 'updateEnd'
   Vec pointVelL(C Vec &local_pos)C; // get point velocity, 'local_pos' is in object local space, returned velocity is in world space, it's valid after animation, matrix and velocity updates (using 'updateEnd' method)
Vec AnimatedSkeletonBone::pointVelL(C Vec &local_pos)C
{
   return _vel + Cross(_ang_vel, local_pos*matrix().orn());
}*/
void AnimatedSkeletonBone::forceMatrix(C MatrixM &matrix) {
    _force_matrix = true;
    _matrix = matrix;
}
/******************************************************************************/
// ANIM SKEL
/******************************************************************************/
void AnimatedSkeleton::zero() {
#if FUR
    fur_stiffness = 0;
    fur_gravity = 0;
    fur_vel_scale = 0;
#endif
    root.zero();

    _skeleton = null;
}
AnimatedSkeleton::AnimatedSkeleton() { zero(); }
AnimatedSkeleton &AnimatedSkeleton::del() {
    bones.del();
    slots.del();
    zero();
    return T;
}
AnimatedSkeleton &AnimatedSkeleton::create(C Skeleton *skeleton, C MatrixM &initial_matrix) // !! 'initial_matrix' can be 'root._matrix' !!
{
    auto temp = initial_matrix; // copy in case 'initial_matrix' belongs to this (for example 'root._matrix' and may get destroyed), use 'auto' depending on matrix type

    if (T._skeleton = skeleton) {
        slots.setNum(skeleton->slots.elms());
        REPAO(slots).zero();
        bones.setNum(skeleton->bones.elms());
        REPA(bones) {
            AnimSkelBone &bone = bones[i];
            bone.zero();
            bone._matrix_prev = bone._matrix = temp;
#if WORLD_POS
            C SkelBone &sbon = skeleton->bones[i];
            bone._world_pos = sbon.pos * bone.matrix();
#endif
        }
    } else {
        bones.clear();
        slots.clear();
    }

#if FUR
    fur_stiffness = 0.0001f;
    fur_gravity = -1;
    fur_vel_scale = -0.75f;
#endif

    root.zero();
    root._matrix_prev = root._matrix = temp;
#if WORLD_POS
    root._world_pos = temp.pos;
#endif

    return T;
}
AnimatedSkeleton &AnimatedSkeleton::create(AnimatedSkeleton &src) {
    if (&src != this) {
        _skeleton = src._skeleton;
        bones = src.bones;
        slots = src.slots;

        root = src.root;

#if FUR
        fur_gravity = src.fur_gravity;
        fur_stiffness = src.fur_stiffness;
        fur_vel_scale = src.fur_vel_scale;
#endif
    }
    return T;
}
/******************************************************************************/
// GET
/******************************************************************************/
AnimSkelBone *AnimatedSkeleton::findBone(BONE_TYPE type, Int type_index, Int type_sub) { return skeleton() ? bones.addr(skeleton()->findBoneI(type, type_index, type_sub)) : null; }
Int AnimatedSkeleton::findBoneI(BONE_TYPE type, Int type_index, Int type_sub) C { return skeleton() ? skeleton()->findBoneI(type, type_index, type_sub) : -1; }
Byte AnimatedSkeleton::findBoneB(BONE_TYPE type, Int type_index, Int type_sub) C { return skeleton() ? skeleton()->findBoneB(type, type_index, type_sub) : 255; }
SkelAnim *AnimatedSkeleton::findSkelAnim(C Str &name) C { return skeleton() ? skeleton()->findSkelAnim(name) : null; }
SkelAnim *AnimatedSkeleton::findSkelAnim(C UID &id) C { return skeleton() ? skeleton()->findSkelAnim(id) : null; }
Int AnimatedSkeleton::findBoneI(CChar8 *name) C { return skeleton() ? skeleton()->findBoneI(name) : -1; }
Int AnimatedSkeleton::findSlotI(CChar8 *name) C { return skeleton() ? skeleton()->findSlotI(name) : -1; }
Byte AnimatedSkeleton::findSlotB(CChar8 *name) C { return skeleton() ? skeleton()->findSlotB(name) : 255; }
AnimSkelBone *AnimatedSkeleton::findBone(CChar8 *name) { return bones.addr(findBoneI(name)); }
OrientM *AnimatedSkeleton::findSlot(CChar8 *name) { return slots.addr(findSlotI(name)); }
AnimSkelBone *AnimatedSkeleton::getBone(CChar8 *name) { return bones.addr(getBoneI(name)); }
OrientM *AnimatedSkeleton::getSlot(CChar8 *name) { return slots.addr(getSlotI(name)); }
Int AnimatedSkeleton::getBoneI(CChar8 *name) C {
    Int i = findBoneI(name);
    if (i < 0)
        Exit(S + "Bone \"" + name + "\" not found in skeleton \"" + Skeletons.name(skeleton()) + "\".");
    return i;
}
Int AnimatedSkeleton::getSlotI(CChar8 *name) C {
    Int i = findSlotI(name);
    if (i < 0)
        Exit(S + "Slot \"" + name + "\" not found in skeleton \"" + Skeletons.name(skeleton()) + "\".");
    return i;
}
SkelAnim *AnimatedSkeleton::getSkelAnim(C Str &name) C {
    SkelAnim *skel_anim = findSkelAnim(name);
    if (!skel_anim && name.is())
        Exit(S + "SkelAnim \"" + name + "\" not found in skeleton \"" + Skeletons.name(skeleton()) + "\".");
    return skel_anim;
}
SkelAnim *AnimatedSkeleton::getSkelAnim(C UID &id) C {
    SkelAnim *skel_anim = findSkelAnim(id);
    if (!skel_anim && id.valid())
        Exit(S + "SkelAnim \"" + id.asCString() + "\" not found in skeleton \"" + Skeletons.name(skeleton()) + "\".");
    return skel_anim;
}
/******************************************************************************/
// SET
/******************************************************************************/
AnimatedSkeleton &AnimatedSkeleton::disable(Int i, Bool disable) {
    if (InRange(i, bones))
        bones[i]._disabled = disable;
    return T;
}
static void DisableChildren(AnimatedSkeleton &anim_skel, Int i, Bool disable) {
    if (C SkelBone *bone = anim_skel.skeleton()->bones.addr(i))
        REP(bone->children_num) {
            Int child_i = bone->children_offset + i;
            if (InRange(child_i, anim_skel.bones)) {
                anim_skel.bones[child_i]._disabled = disable;
                DisableChildren(anim_skel, child_i, disable);
            }
        }
}
AnimatedSkeleton &AnimatedSkeleton::disableChildren(Int i, Bool disable) {
    if (InRange(i, bones)) {
        Bool &disabled = bones[i]._disabled_children;
        if (disabled != disable) {
            disabled = disable;
            if (skeleton())
                DisableChildren(T, i, disable); // test for 'skeleton' only once here and not everytime in 'DisableChildren'
        }
    }
    return T;
}
/******************************************************************************/
// ANIMATE
/******************************************************************************/
AnimatedSkeleton &AnimatedSkeleton::clear() {
    root.clear();
    REPAO(bones).clear();
    return T;
}
AnimatedSkeleton &AnimatedSkeleton::clear(Flt blend) {
    if (blend > 0) {
        if (blend >= 1)
            clear();
        else {
            Flt blend1 = 1 - blend;
            root.keep01(blend1);
            REPAO(bones).keep01(blend1);
        }
    }
    return T;
}
/******************************************************************************/
struct AnimParamsBlend : AnimParams {
    Flt blend;

    AnimParamsBlend(C Animation &animation, Flt time, Flt blend) : AnimParams(animation, time), blend(blend) {}
};
struct AnimParamsReplace : AnimParams {
    Flt blend, blend1;

    AnimParamsReplace(C Animation &animation, Flt time, Flt blend) : AnimParams(animation, time), blend(blend), blend1(Sat(1 - blend)) {} // 'blend1' needs to be clamped to 0..1 range because old keyframes are multiplied by this
};
static void Animate(AnimSkelBone &asbon, C AnimKeys &keys, C AnimParams &params) {
    if (!asbon._disabled) {
        // orientation
        if (keys.orns.elms()) {
            Orient &bone_orn = asbon.orn, orn;
            keys.orn(orn, params);
            bone_orn += orn;
        }

        // position
        if (keys.poss.elms()) {
            Vec &bone_pos = asbon.pos, pos;
            keys.pos(pos, params);
            bone_pos += pos;
        }

        // scale
        if (keys.scales.elms()) {
            Vec &bone_scale = asbon.scale, scale;
            keys.scale(scale, params);
            bone_scale += scale;
        }

#if HAS_ANIM_ROT
        // rotation
        if (keys.rots.elms()) {
            AxisRoll &bone_rot = asbon.rot, rot;
            keys.rot(rot, params);
            bone_rot.v4() += rot.v4();
        }
#endif

#if HAS_ANIM_COLOR
        // color
        if (keys.colors.elms()) {
            Vec4 &bone_color = asbon.color, color;
            keys.color(color, params);
            bone_color = color; // colors are always replaced
        }
#endif
    }
}
static void Animate(AnimSkelBone &asbon, C AnimKeys &keys, C AnimParamsBlend &params) {
    if (!asbon._disabled) {
        // orientation
        if (keys.orns.elms()) {
            Orient &bone_orn = asbon.orn, orn;
            keys.orn(orn, params);
            bone_orn += params.blend * orn;
        }

        // position
        if (keys.poss.elms()) {
            Vec &bone_pos = asbon.pos, pos;
            keys.pos(pos, params);
            bone_pos += params.blend * pos;
        }

        // scale
        if (keys.scales.elms()) {
            Vec &bone_scale = asbon.scale, scale;
            keys.scale(scale, params);
            bone_scale += params.blend * scale;
        }

#if HAS_ANIM_ROT
        // rotation
        if (keys.rots.elms()) {
            AxisRoll &bone_rot = asbon.rot, rot;
            keys.rot(rot, params);
            bone_rot.v4() += params.blend * rot.v4();
        }
#endif

#if HAS_ANIM_COLOR
        // color
        if (keys.colors.elms()) {
            Vec4 &bone_color = asbon.color, color;
            keys.color(color, params);
            bone_color *= 1 - params.blend;     // colors are always replaced
            bone_color += params.blend * color; // colors are always replaced
        }
#endif
    }
}
static void Animate(AnimSkelBone &asbon, C AnimKeys &keys, C AnimParamsReplace &params) {
    if (!asbon._disabled) {
        // orientation
        if (keys.orns.elms()) {
            asbon.keep01(params.blend1);

            Orient &bone_orn = asbon.orn, orn;
            keys.orn(orn, params);
            // bone_orn*=params.blend1; done in 'keep01' above
            bone_orn += params.blend * orn;
        }

        // position
        if (keys.poss.elms()) {
            Vec &bone_pos = asbon.pos, pos;
            keys.pos(pos, params);
            // bone_pos*=params.blend1; done in 'keep01' above
            bone_pos += params.blend * pos;
        }

        // scale
        if (keys.scales.elms()) {
            Vec &bone_scale = asbon.scale, scale;
            keys.scale(scale, params);
            // bone_scale*=params.blend1; done in 'keep01' above
            bone_scale += params.blend * scale;
        }

#if HAS_ANIM_ROT
        // rotation
        if (keys.rots.elms()) {
            AxisRoll &bone_rot = asbon.rot, rot;
            keys.rot(rot, params);
            // bone_rot.v4()*=params.blend1; done in 'keep01' above
            bone_rot.v4() += params.blend * rot.v4();
        }
#endif

#if HAS_ANIM_COLOR
        // color
        if (keys.colors.elms()) {
            Vec4 &bone_color = asbon.color, color;
            keys.color(color, params);
            this is wrong because 'keep01' already replaced towards Vec4(1)
                bone_color *= params.blend1;
            bone_color += params.blend * color;
        }
#endif
    }
}
static void AnimRoot(AnimatedSkeleton &anim_skel, C Animation *animation, Flt time, Flt blend) {
    // if(blend>EPS_ANIM_BLEND) // this is already checked in methods calling this function
    if (animation) {
        AnimParamsBlend params(*animation, time, blend);
        Animate(anim_skel.root, animation->keys, params); // animate root
    }
}
/******************************************************************************/
AnimatedSkeleton &AnimatedSkeleton::animate(C SkelAnim &skel_anim, Flt time, Flt blend) {
    if (blend > EPS_ANIM_BLEND)
        if (C Animation *animation = skel_anim.animation()) {
            if (blend == 1) {
                AnimParams params(*animation, time);

                // Animate(root, animation->keys, params); // animate root - this is no longer done here, instead, root animations need to be processed manually
                REPA(animation->bones) // animate bones
                {
                    auto sbon = skel_anim.abonToSbon(i);
                    if (InRange(sbon, bones))
                        Animate(bones[sbon], animation->bones[i], params);
                }
            } else {
                AnimParamsBlend params(*animation, time, blend);

                // Animate(root, animation->keys, params); // animate root - this is no longer done here, instead, root animations need to be processed manually
                REPA(animation->bones) // animate bones
                {
                    auto sbon = skel_anim.abonToSbon(i);
                    if (InRange(sbon, bones))
                        Animate(bones[sbon], animation->bones[i], params);
                }
            }
        }
    return T;
}

AnimatedSkeleton &AnimatedSkeleton::animateRoot(C Animation *anim, Flt time) {
    if (anim)
        animateRoot(*anim, time);
    return T;
}
AnimatedSkeleton &AnimatedSkeleton::animateRoot(C Animation &anim, Flt time) {
    AnimParams params(anim, time);
    Animate(root, anim.keys, params); // animate root
    return T;
}
AnimatedSkeleton &AnimatedSkeleton::animateEx(C SkelAnim &skel_anim, Flt time, Bool exact_time, Bool animate_root, Bool animate_bones) {
    if (C Animation *animation = skel_anim.animation()) {
        AnimParams params(*animation, time);
        if (exact_time)
            params.time = time; // re-apply time to remove possible fraction

        if (animate_root)
            Animate(root, animation->keys, params); // animate root
        if (animate_bones)
            REPA(animation->bones) // animate bones
            {
                auto sbon = skel_anim.abonToSbon(i);
                if (InRange(sbon, bones))
                    Animate(bones[sbon], animation->bones[i], params);
            }
    }
    return T;
}
AnimatedSkeleton &AnimatedSkeleton::animate(C SkelAnim *skel_anim, Flt time, Flt blend) {
    if (skel_anim)
        animate(*skel_anim, time, blend);
    return T;
}
AnimatedSkeleton &AnimatedSkeleton::animate(C Motion &motion) {
    if (motion.is())
        animate(*motion.skel_anim, motion.time, motion.animBlend());
    return T;
}
AnimatedSkeleton &AnimatedSkeleton::animate(C Str &anim_name, Flt time, Flt blend) {
    if (anim_name.is() && blend > EPS_ANIM_BLEND)
        if (skeleton())
            animate(*getSkelAnim(anim_name), time, blend);
        else
            AnimRoot(T, Animations(anim_name), time, blend);
    return T;
} // in these methods check 'blend' first to avoid unnecessary animation loads
AnimatedSkeleton &AnimatedSkeleton::animate(C UID &anim_id, Flt time, Flt blend) {
    if (anim_id.valid() && blend > EPS_ANIM_BLEND)
        if (skeleton())
            animate(*getSkelAnim(anim_id), time, blend);
        else
            AnimRoot(T, Animations(anim_id), time, blend);
    return T;
} // in these methods check 'blend' first to avoid unnecessary animation loads
/******************************************************************************/
AnimatedSkeleton &AnimatedSkeleton::animateReplace(C SkelAnim &skel_anim, Flt time, Flt blend) {
    if (blend > EPS_ANIM_BLEND)
        if (C Animation *animation = skel_anim.animation()) {
            AnimParamsReplace params(*animation, time, blend);

            // Animate(root, animation->keys, params); // animate root - this is no longer done here, instead, root animations need to be processed manually
            REPA(animation->bones) // animate bones
            {
                auto sbon = skel_anim.abonToSbon(i);
                if (InRange(sbon, bones))
                    Animate(bones[sbon], animation->bones[i], params);
            }
        }
    return T;
}
AnimatedSkeleton &AnimatedSkeleton::animateReplace(C SkelAnim *skel_anim, Flt time, Flt blend) {
    if (skel_anim)
        animateReplace(*skel_anim, time, blend);
    return T;
}
AnimatedSkeleton &AnimatedSkeleton::animateReplace(C Motion &motion) {
    if (motion.is())
        animateReplace(*motion.skel_anim, motion.time, motion.animBlend());
    return T;
}
/******************************************************************************/
static void UpdateRootBoneMatrix(AnimatedSkeleton &anim_skel, C MatrixM &body_matrix) {
    AnimSkelBone &bone = anim_skel.root;

    if (bone._disabled) {
        bone._matrix = body_matrix;
    } else {
        Orient &bone_orn = bone.orn; // we can modify it directly, because we're just calling 'fix' on it

        // rotation
#if HAS_ANIM_SKEL_ROT
        if (bone.rot.any()) {
            Vec axis = bone.rot.axis;
            Flt angle = axis.normalize();

            if (bone.rot.roll) {
                bone._matrix.orn().setRotateZ(bone.rot.roll).rotate(axis, angle);
            } else {
                bone._matrix.orn().setRotate(axis, angle);
            }

            if (bone_orn.fix()) // orientation
            {
                bone._matrix.orn() *= Matrix3(bone_orn);
            }
        } else
#endif
            if (bone_orn.fix()) // orientation
        {
            bone._matrix.orn() = bone_orn;
        } else {
            // only position/scale
            bone._matrix = body_matrix;

            // apply animation position
            if (bone.pos.any())
                bone._matrix.pos += bone.pos * bone._matrix.orn();

            // apply animation scale
            if (bone.scale.any()) {
                bone._matrix.x *= ScaleFactor(bone.scale.x);
                bone._matrix.y *= ScaleFactor(bone.scale.y);
                bone._matrix.z *= ScaleFactor(bone.scale.z);
            }
            return;
        }

        // set scale
        if (bone.scale.any()) {
            bone._matrix.x *= ScaleFactor(bone.scale.x);
            bone._matrix.y *= ScaleFactor(bone.scale.y);
            bone._matrix.z *= ScaleFactor(bone.scale.z);
        }

        // set position
        bone._matrix.pos = bone.pos;
        bone._matrix.mul(body_matrix);
    }
}
static void UpdateBoneMatrix(AnimatedSkeleton &anim_skel, Int i) {
    AnimSkelBone &bone = anim_skel.bones[i];
    if (bone._force_matrix)
        return; // it's important to don't do any adjustments (for example '_world_space_transform') for '_matrix' if '_force_matrix' is enabled, because this function can be called several times before skeleton finishes animating, which would adjust several times
    C SkelBone &sbon = anim_skel.skeleton()->bones[i];
    C SkelBone *parent = anim_skel.skeleton()->bones.addr(sbon.parent);
    C auto &parent_transform_matrix = anim_skel.boneRoot(sbon.parent)._matrix; // use 'auto' depending on matrix type

    if (bone._disabled)
        bone._matrix = parent_transform_matrix;
    else {
        Matrix3 parent_matrix;
        if (parent)
            parent_matrix = *parent;
        Orient bone_orn = bone.orn;

        /* Animation Formula:

           #1 Rotation

              Vec axis =bone.rot.axis   ; // rotation in parent space
              Flt angle=axis.normalize();
              if(parent)axis*=parent_matrix; // rotation in world space

              bone.matrix.setPos(-sbon.pos               )
                         .rot   ( sbon.dir               , bone.rot.roll)
                         .rot   ( axis                   , angle)
                         .move  ( sbon.pos               )
                         .mul   ( parent_transform_matrix);

           #2 Orientation

              Orient src =sbon    ;             // current orientation in world  space
              Orient dest=bone_orn; dest.fix(); // target  orientation in parent space
              if(parent)dest*=parent_matrix;    // target  orientation in world  space
              Matrix3 transform=GetTransform(src, dest);

              bone.matrix.setPos(-sbon.pos)
                         .mul   (transform)
                         .move  ( sbon.pos)
                         .mul   (parent_transform_matrix);
        */

        // rotation
#if HAS_ANIM_SKEL_ROT
        if (bone.rot.any()) {
            Vec axis = bone.rot.axis; // rotation in parent space
            Flt angle = axis.normalize();
            if (parent)
                axis *= parent_matrix; // rotation in world  space

            if (bone.rot.roll) {
                bone._matrix.orn().setRotate(sbon.dir, bone.rot.roll).rotate(axis, angle);
            } else {
                bone._matrix.orn().setRotate(axis, angle);
            }

            if (bone_orn.fix()) // orientation
            {
                if (parent)
                    bone_orn.mul(parent_matrix, true); // transform target orientation from parent space to world space
                Matrix3 transform;
                GetTransform(transform, sbon, bone_orn);
                bone._matrix.orn() *= transform;
            }
        } else
#endif
            if (bone_orn.fix()) // orientation
        {
            if (parent)
                bone_orn.mul(parent_matrix, true); // transform target orientation from parent space to world space
            GetTransform(bone._matrix.orn(), sbon, bone_orn);
        } else {
            // set scale
            if (bone.scale.any()) {
                bone._matrix.orn().identity();
                bone._matrix.orn().scale(sbon.cross(), ScaleFactor(bone.scale.x));
                bone._matrix.orn().scale(sbon.perp, ScaleFactor(bone.scale.y));
                bone._matrix.orn().scale(sbon.dir, ScaleFactor(bone.scale.z));
                goto scale_set;
            }

            // only position
            bone._matrix = parent_transform_matrix;
            if (bone.pos.any()) {
#if 1 // pos relative to parent
                bone._matrix.pos += (parent ? bone.pos * parent_matrix : bone.pos) * parent_transform_matrix.orn();
#else
                bone._matrix.pos += bone.pos * parent_transform_matrix.orn();
#endif
            }
            goto matrix_set;
        }

        // set scale
        if (bone.scale.any()) {
            Orient sbon_transformed = sbon;
            sbon_transformed.mul(bone._matrix.orn(), true);
            bone._matrix.orn().scale(sbon_transformed.cross(), ScaleFactor(bone.scale.x));
            bone._matrix.orn().scale(sbon_transformed.perp, ScaleFactor(bone.scale.y));
            bone._matrix.orn().scale(sbon_transformed.dir, ScaleFactor(bone.scale.z));
        }
    scale_set:

        // set position
        bone._matrix.anchor(sbon.pos);
#if 1 // pos relative to parent
        if (bone.pos.any())
            bone._matrix.pos += (parent ? bone.pos * parent_matrix : bone.pos);
#else
        bone._matrix.pos += bone.pos;
#endif

        bone._matrix *= parent_transform_matrix;
    }
matrix_set:

    // world space transformation
    if (bone._world_space_transform)
        bone._matrix.transformAtPos(sbon.pos * bone._matrix, bone._world_space_transform_matrix);
}
static void UpdateSlot(AnimatedSkeleton &anim_skel, Int i) {
    C SkelSlot &skel_slot = anim_skel.skeleton()->slots[i];
    OrientM &slot = anim_skel.slots[i];
    slot = skel_slot;
    slot.mul(anim_skel.boneRoot(skel_slot.bone).matrix(), true);
    if (skel_slot.bone != skel_slot.bone1) {
        OrientM secondary = skel_slot;
        secondary.mul(anim_skel.boneRoot(skel_slot.bone1).matrix(), true);
        slot += secondary;
        slot.fix();
        slot.pos *= 0.5;
    }
}
/******************************************************************************/
AnimatedSkeleton &AnimatedSkeleton::updateMatrix(C MatrixM &body_matrix) {
    UpdateRootBoneMatrix(T, body_matrix);
    if (skeleton()) // test 'skeleton' once here, and not everytime in 'UpdateBoneMatrix' and 'UpdateSlot'
    {
        Int min_bones = minBones();
        FREP(min_bones)
        UpdateBoneMatrix(T, i); // process bones in order to update parents first
        REP(minSlots())
        UpdateSlot(T, i); // order is not important, because slots are attached to bones (not slots)
    }
    return T;
}
/******************************************************************************/
static void UpdateBoneMatrixRecursiveUp(AnimatedSkeleton &anim_skel, Int i) {
    BoneType parent = anim_skel.skeleton()->bones[i].parent;
    if (parent < i)
        UpdateBoneMatrixRecursiveUp(anim_skel, parent); // first update parents, "parent<i" means that parent is !=BONE_NULL, parent fits in minBones range and this prevents infinite loops (looped parent cycle)
    UpdateBoneMatrix(anim_skel, i);                     // now   update self
}
AnimatedSkeleton &AnimatedSkeleton::updateMatrixParents(C MatrixM &body_matrix, Int bone) {
    UpdateRootBoneMatrix(T, body_matrix); // first update root
    if (skeleton() && InRange(bone, minBones()))
        UpdateBoneMatrixRecursiveUp(T, bone); // now   update parents and self, test 'skeleton' once here, and not everytime in 'UpdateBoneMatrixRecursiveUp'
    return T;
}
/******************************************************************************/
static void UpdateBoneMatrixRecursiveDown(AnimatedSkeleton &anim_skel, Int i, Int min_bones) {
    C SkelBone &bone = anim_skel.skeleton()->bones[i];
    UpdateBoneMatrix(anim_skel, i);                                                                      // first update self
    for (Int i = Min(bone.children_offset + bone.children_num, min_bones); --i >= bone.children_offset;) // now update children
        UpdateBoneMatrixRecursiveDown(anim_skel, i, min_bones);
}
AnimatedSkeleton &AnimatedSkeleton::updateMatrixChildren(Int bone) // this updates 'bone' too
{
    if (skeleton()) // test 'skeleton' once here, and not everytime in 'UpdateBoneMatrixRecursiveDown' and 'UpdateSlot'
    {
        Int min_bones = minBones();
        if (InRange(bone, min_bones)) {
            UpdateBoneMatrixRecursiveDown(T, bone, min_bones);
            REP(minSlots())
            UpdateSlot(T, i); // update slots once bones are ready (because slots are attached to bones)
        }
    }
    return T;
}
/******************************************************************************/
AnimatedSkeleton &AnimatedSkeleton::forceMatrix(Int bone, C MatrixM &matrix, Bool auto_update_matrixes) {
    if (InRange(bone, bones)) {
        bones[bone].forceMatrix(matrix);
        if (auto_update_matrixes)
            updateMatrixChildren(bone); // this will update 'bone' too
    }
    return T;
}
AnimatedSkeleton &AnimatedSkeleton::transformInWorldSpace(Int bone, C Matrix3 &matrix, Bool auto_update_matrixes) {
    if (InRange(bone, bones)) {
        AnimSkelBone &b = bones[bone];

        if (b._world_space_transform)
            b._world_space_transform_matrix *= matrix;
        else // if there was already a world transform, then adjust it
        {
            b._world_space_transform = true;
            b._world_space_transform_matrix = matrix;
        }

        if (auto_update_matrixes)
            updateMatrixChildren(bone); // this will update 'bone' too
    }
    return T;
}
AnimatedSkeleton &AnimatedSkeleton::transformInWorldSpace(Int bone, C Matrix &matrix, Bool auto_update_matrixes) {
    if (InRange(bone, bones)) {
        AnimSkelBone &b = bones[bone];

        if (b._world_space_transform)
            b._world_space_transform_matrix *= matrix;
        else // if there was already a world transform, then adjust it
        {
            b._world_space_transform = true;
            b._world_space_transform_matrix = matrix;
        }

        if (auto_update_matrixes)
            updateMatrixChildren(bone); // this will update 'bone' too
    }
    return T;
}
AnimatedSkeleton &AnimatedSkeleton::transformInWorldSpace(Int bone, C MatrixM &matrix, Bool auto_update_matrixes) {
    if (InRange(bone, bones)) {
        AnimSkelBone &b = bones[bone];

        if (b._world_space_transform)
            b._world_space_transform_matrix *= matrix;
        else // if there was already a world transform, then adjust it
        {
            b._world_space_transform = true;
            b._world_space_transform_matrix = matrix;
        }

        if (auto_update_matrixes)
            updateMatrixChildren(bone); // this will update 'bone' too
    }
    return T;
}
/******************************************************************************
static Vec FurVel(C Vec &vel, Flt fur_vel_scale, Flt fur_gravity)
{
   Vec    fur_vel=vel*fur_vel_scale; fur_vel.y+=fur_gravity; fur_vel.clipLength(0.92f);
   return fur_vel;
}
   AnimatedSkeleton& vel(C Vec &vel, C Vec &ang_vel=VecZero); // force custom velocity to root and all bones
AnimatedSkeleton& AnimatedSkeleton::vel(C Vec &vel, C Vec &ang_vel)
{
   Vec fur_vel=FurVel(vel, fur_vel_scale, fur_gravity);
   root.    _vel=    vel;
   root._ang_vel=ang_vel;
   root._fur_vel=fur_vel;
   REPA(bones)
   {
      AnimSkelBone &bone=bones[i];
      bone.    _vel=    vel;
      bone._ang_vel=ang_vel;
      bone._fur_vel=fur_vel;
   }
   return T;
}*/
AnimatedSkeleton &AnimatedSkeleton::updateBegin() {
    Bool physics_relative = false;
    if (physics_relative && !Physics.updated())
        return T;

    root._matrix_prev = root._matrix;
    REPA(bones) {
        AnimSkelBone &bone = bones[i];
        bone._matrix_prev = bone._matrix;
    }
    return T;
}
void AnimatedSkeleton::updateEnd() {
    /*Bool physics_relative=false, ragdoll_bones_only=false;

    Flt time_mul;

    if(physics_relative)
    {
       if(!       Physics.updated    ())return;
       time_mul=1/Physics.updatedTime();
    }else
    {
       time_mul=((Time.d()>EPS) ? 1/Time.d() : 1);
    }

    Vec pos_delta, ang_delta;

    // root
    GetDelta(pos_delta, ang_delta, root._matrix_prev, root._matrix);
    root.    _vel=pos_delta*time_mul;
    root._ang_vel=ang_delta*time_mul;
 #if WORLD_POS
    root._world_pos=root._matrix.pos;
 #endif

 #if FUR
    AdjustValTime(root._fur_vel, FurVel(vel(), fur_vel_scale, fur_gravity), fur_stiffness);
 #endif

    // bones
    if(skeleton())
    {
       Int min_bones=minBones(); FREP(min_bones) // order is important (parents first)
       {
            C SkelBone &sbon=skeleton()->bones[i];
          AnimSkelBone &bone=            bones[i];
          if(!ragdoll_bones_only || (sbon.flag&BONE_RAGDOLL))
          {
             GetDelta(ang_delta, bone._matrix_prev, bone._matrix);

             Vec     rot_pos=sbon.pos; rot_pos*=bone.matrix().orn(); // no need for VecD
             auto  trans_pos=rot_pos+bone.matrix().pos; // trans_pos=sbon.pos*bone.matrix(), use 'auto' depending on vector type
          #if WORLD_POS
           C auto &prev_world_pos=bone._world_pos; // use 'auto' depending on vector type
          #else
             auto  prev_world_pos=sbon.pos*bone._matrix_prev; // use 'auto' depending on vector type
          #endif
             Vec world_delta=trans_pos-prev_world_pos; // world pos movement, no need for VecD
             pos_delta=world_delta
                      -Cross(ang_delta, rot_pos); // subtract angular velocity based on 'sbon.pos' to make sure that it does not affect points on that line ("pointVelL(sbon.pos)" will be zero if only angular velocities are present)

          #if WORLD_POS
             bone._world_pos=trans_pos;
          #endif
             bone.    _vel=pos_delta*time_mul;
             bone._ang_vel=ang_delta*time_mul;
             AdjustValTime(bone._fur_vel, FurVel(world_delta*time_mul, fur_vel_scale, fur_gravity), fur_stiffness); // set based only on linear movement
          }else // inherit values from the parent
          {
             AnimSkelBone &parent=boneRoot(sbon.parent);
             bone.      _vel=parent.      _vel;
             bone.  _ang_vel=parent.  _ang_vel;
             bone.  _fur_vel=parent.  _fur_vel;
          #if WORLD_POS
             bone._world_pos=parent._world_pos; or calculate manually
          #endif
          }
       }
    }*/
}
/******************************************************************************/
void AnimatedSkeleton::move(C VecD &d) {
    root += d;
    REPAO(bones) += d;
    REPAO(slots) += d;
}
void AnimatedSkeleton::offset(C VecD &d) {
    root._matrix += d;
    REPAO(bones)._matrix += d;
    REPAO(slots) += d;
}
/******************************************************************************/
void AnimatedSkeleton::getMatrixes(MemPtrN<MatrixM, 256> matrixes) C {
    matrixes.setNum(bones.elms() + 1);
    matrixes[0] = matrix();
    REPA(bones)
    matrixes[i + 1] = bones[i].matrix();
}
/******************************************************************************/
// DRAW
/******************************************************************************/
void AnimatedSkeleton::draw(C Color &bone_color, C Color &slot_color) C {
    if (bone_color.a && skeleton())
        REP(minBones())
    (skeleton()->bones[i] * bones[i].matrix()).draw(bone_color);
    if (slot_color.a)
        REPAO(slots).draw(slot_color);
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool AnimatedSkeleton::save(File &f) C // !! if changing file format, then keep backward compatibility with old save games !!
{
    f.putMulti(Byte(0), matrix()); // version
    f.putAsset(Skeletons.id(skeleton()));
#if FUR
    f.putMulti(fur_stiffness, fur_gravity, fur_vel_scale);
#endif
    return f.ok();
}
Bool AnimatedSkeleton::load(File &f) {
    switch (f.decUIntV()) // version
    {
    case 0: {
        f.getMulti(root._matrix);
        create(Skeletons(f.getAssetID()), root._matrix);
#if FUR
        f.getMulti(fur_stiffness, fur_gravity, fur_vel_scale);
#endif
        if (f.ok())
            return true;
    } break;
    }
    del();
    return false;
}
/******************************************************************************/
} // namespace EE
  /******************************************************************************/