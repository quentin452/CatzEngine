/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************

   About reusing same animations for different skeletons in a better way (the following has not been tested, it's only an initial idea) :

   To be able to achieve "absolute target" animations,
      So for example an animation would work the same for 2 different skeletons which:
         -one has UpperArm connected properly to ShoulderBone (this parent points to the side)
         -2nd has UpperArm connected          to Spine, because ShoulderBone is not present (this parent points up)

   Most likely we would need to have:
      -some sort of Matrix3/Orientation in AnimKeys/AnimBone (just 1 per bone, and not 1 per keyframe), which describes the child<->parent orientation relation for which the animation was created
         -bone orn, parent orn, or just bone orn relative to parent orn
      -some sort of transform, stored per bone in SkelAnim (which is based on above AnimBone orn and adjusted by SkelBone orn of Skeleton that will actually use this animation and its relation to its parent, basically this transforms from what the animation was created for, to what is it going to be used for)
      -use that transform in AnimSkelBone when animating bones by animations:
         void Animate(AnimSkelBone &asbon, C AnimKeys &keys, C AnimParamsEx &params)
         {
            ..
            bone_orn+=params.blend*orn; // <- this 'orn' would need to be transformed somehow by the above transform

      -if that would be done, then it could be done in such a way, that:
         void UpdateBoneMatrix(AnimatedSkeleton &anim_skel, Int i)
         {
            ..
            if(parent)bone_orn.transform(parent_matrix, true); // <- this would not be needed because 'bone_orn' is already transformed correctly above

   The downsides of this approach:
      -more memory needed:
         -storing orn in AnimKeys
         -storing orn in SkelAnim
      -extra processing needed in "void Animate(AnimSkelBone &asbon, C AnimKeys &keys, C AnimParamsEx &params)"

      The benefit of simplified "void UpdateBoneMatrix" is smaller compared to more expensive "void Animate", because:
         -Animate          is called many times for 1 bone (for each blended animation)
         -UpdateBoneMatrix is called 1    time  for 1 bone

      The extra processing in "Animate" could be removed completely, if entire animation (all keyframes) would be transformed to the target Skeleton,
         however in that case we would need to have entire copies of every animation for every skeleton that uses them, so I'm not considering this at all, as that would use too much memory.

   Unless a better approach is figured out, I think it's just better to make sure to keep Skeletons similar
      (all have shoulder bones, and the same bone orientations, setup using "Adjust Bone Orientations" in the Editor).

   ----

   Scales Keyframes Limitation,
      Currently there is a limitation that if imported a Skeleton with Bones with X Scale,
      and then importing Animations having the same Skeleton but some Bones with Y scale,
         then scales won't be set correctly, because 'Animation.*adjust*' methods take into account only bone positions and orientations,
         but don't do anything about bone scales, since there's no functionality for storing original FBX node matrix scales in Skeleton Bones.
      This is noticeable when importing "Weretoad.fbx" (which has scaled tongue sticking out) and then trying to import animations to it (which animations initially have tongue at smaller size).
      The solution is to Create a Static / T-Pose / Bind Pose FBX from animation files that have normal scales (for example, import "Weretoad.fbx" into 3ds max, then import "idle" animation on top of that to adjust the mesh, and then export the FBX as new base model).
      And finally import that base model, and animations to it.

   ----

   Animation Bone Names vs Types:
      Currently the Editor will always adjust AnimBone.name,type,.. when:
         -making any change to the Skeleton linked with the Animation
         -changing Skeleton linked with the Animation (this may result in loss of animation bones if target skeleton doesn't have some bones as the source, however it results in better consistency so we don't have any leftover animations not visible with the new skeleton, but could show up in the future when changing to new skeletons)
      See also: FIND_ANIM_BY_NAME_ONLY and 'RenameAnimBonesOnSkelChange' in the Editor

/******************************************************************************
struct Rot // Relative Rotation
{
   Flt      time; // time position (seconds)
   AxisRoll rot , // rotation
            tan ; // tangent

#if EE_PRIVATE
   void save(MemPtr<TextNode> nodes)C; // save text
#endif
};
struct Color // Color
{
   Flt  time ; // time  position (seconds)
   Vec4 color, // color value
        tan  ; // color tangent

#if EE_PRIVATE
   void save(MemPtr<TextNode> nodes)C; // save text
#endif
};
Mems<Rot  > rots;
Mems<Color> colors;
/******************************************************************************/
#define CC4_ANIM CC4('A', 'N', 'I', 'M')
#define SET_ON_FAIL 1
#define FIND_ANIM_BY_NAME_ONLY 1 // if search for animation bones by bone name only
/******************************************************************************/
Cache<Animation> Animations("Animation");
/******************************************************************************/
void AnimParams::set(C Animation &animation, Flt time) { set(animation.loop(), animation.linear(), animation.length(), time); }
/******************************************************************************/
// ANIMATION KEYS
/******************************************************************************/
Bool AnimKeys::orn(Orient &orn, C AnimParams &params) C {
    PROFILE_START("CatzEngine::AnimKeys::orn(Orient &orn, C AnimParams &params)")
    switch (orns.elms()) {
    case 0:
        if (SET_ON_FAIL) {
            orn.identity();
            goto stop_and_return_false;
        }
    case 1:
    first:
        orn = orns[0].orn;
        goto stop_and_return_true;
    default: {
        Int l, r;
        for (l = 0, r = orns.elms(); l < r;) {
            Int mid = UInt(l + r) / 2;
            if (params.time < orns[mid].time)
                r = mid;
            else
                l = mid + 1;
        }
        Int prev = l - 1, next = l;
        C Orn *p, *n;
        Flt step;
        if (prev < 0) // before first key
        {
            if (params.loop) {
                prev = orns.elms() - 1; // last
                p = &orns[prev];
                n = &orns[next];
                if (Flt length = params.length - p->time + n->time)
                    step = (params.time - p->time + params.length) / length;
                else
                    goto first;
            } else
                goto first;
        } else if (next == orns.elms()) // after last key
        {
            if (params.loop) {
                next = 0; // first
                p = &orns[prev];
                n = &orns[next];
                if (Flt length = params.length - p->time + n->time)
                    step = (params.time - p->time) / length;
                else
                    goto last;
            } else {
            last:
                orn = orns.last().orn;
                goto stop_and_return_true;
            }
        } else {
            p = &orns[prev];
            n = &orns[next];
            if (Flt length = n->time - p->time)
                step = (params.time - p->time) / length;
            else {
                orn = p->orn;
                goto stop_and_return_true;
            }
        }
        if (params.linear) {
            orn.dir = Lerp(p->orn.dir, n->orn.dir, step);
            orn.perp = Lerp(p->orn.perp, n->orn.perp, step);
        } else {
#if HAS_ANIM_TANGENT
            orn.dir = LerpTan(prev->orn.dir, next->orn.dir, step, prev->tan.dir, next->tan.dir);
            orn.perp = LerpTan(prev->orn.perp, next->orn.perp, step, prev->tan.perp, next->tan.perp);
#else
            C Orn *p2, *n2;
            if (params.loop) {
                p2 = &orns[(prev - 1 + orns.elms()) % orns.elms()];
                if (prev == 0 && Equal(p2->time, p->time + params.length))
                    p2 = &orns[orns.elms() - 2];
                n2 = &orns[(next + 1) % orns.elms()];
                if (next == orns.elms() - 1 && Equal(n->time, n2->time + params.length))
                    n2 = &orns[1];
            } else {
                p2 = &orns[Max(prev - 1, 0)];
                n2 = &orns[Min(next + 1, orns.elms() - 1)];
            }
            orn.dir = Lerp4(p2->orn.dir, p->orn.dir, n->orn.dir, n2->orn.dir, step);
            orn.perp = Lerp4(p2->orn.perp, p->orn.perp, n->orn.perp, n2->orn.perp, step);
#endif
        }
        orn.fix();
    }
        goto stop_and_return_true;
    }

stop_and_return_true:
    PROFILE_STOP("CatzEngine::AnimKeys::orn(Orient &orn, C AnimParams &params)")
    return true;

stop_and_return_false:
    PROFILE_STOP("CatzEngine::AnimKeys::orn(Orient &orn, C AnimParams &params)")
    return false;
}

Bool AnimKeys::pos(Vec &pos, C AnimParams &params) C {
    PROFILE_START("CatzEngine::AnimKeys::pos(Vec &pos, C AnimParams &params)")
    switch (poss.elms()) {
    case 0:
        if (SET_ON_FAIL)
            pos.zero();
        PROFILE_STOP("CatzEngine::AnimKeys::pos(Vec &pos, C AnimParams &params)")
        return false;
    case 1:
    first:
        pos = poss[0].pos;
        PROFILE_STOP("CatzEngine::AnimKeys::pos(Vec &pos, C AnimParams &params)")
        return true;
    default: {
        Int l, r;
        for (l = 0, r = poss.elms(); l < r;) {
            Int mid = UInt(l + r) / 2;
            if (params.time < poss[mid].time)
                r = mid;
            else
                l = mid + 1;
        }
        Int prev = l - 1, next = l;
        C Pos *p, *n;
        Flt step;
        if (prev < 0) // before first key
        {
            if (params.loop) {
                prev = poss.elms() - 1; // last
                p = &poss[prev];
                n = &poss[next];
                if (Flt length = params.length - p->time + n->time)
                    step = (params.time - p->time + params.length) / length;
                else
                    goto first;
            } else
                goto first;
        } else if (next == poss.elms()) // after last key
        {
            if (params.loop) {
                next = 0; // first
                p = &poss[prev];
                n = &poss[next];
                if (Flt length = params.length - p->time + n->time)
                    step = (params.time - p->time) / length;
                else
                    goto last;
            } else {
            last:
                pos = poss.last().pos;
                PROFILE_STOP("CatzEngine::AnimKeys::pos(Vec &pos, C AnimParams &params)")
                return true;
            }
        } else {
            p = &poss[prev];
            n = &poss[next];
            if (Flt length = n->time - p->time)
                step = (params.time - p->time) / length;
            else {
                pos = p->pos;
                PROFILE_STOP("CatzEngine::AnimKeys::pos(Vec &pos, C AnimParams &params)")
                return true;
            }
        }
        if (params.linear)
            pos = Lerp(p->pos, n->pos, step);
        else {
#if HAS_ANIM_TANGENT
            pos = LerpTan(prev->pos, next->pos, step, prev->tan, next->tan);
#else
            C Pos *p2, *n2;
            if (params.loop) {
                p2 = &poss[(prev - 1 + poss.elms()) % poss.elms()];
                if (prev == 0 && Equal(p2->time, p->time + params.length))
                    p2 = &poss[poss.elms() - 2];
                n2 = &poss[(next + 1) % poss.elms()];
                if (next == poss.elms() - 1 && Equal(n->time, n2->time + params.length))
                    n2 = &poss[1];
            } else {
                p2 = &poss[Max(prev - 1, 0)];
                n2 = &poss[Min(next + 1, poss.elms() - 1)];
            }
            pos = Lerp4(p2->pos, p->pos, n->pos, n2->pos, step);
#endif
        }
    }
        PROFILE_STOP("CatzEngine::AnimKeys::pos(Vec &pos, C AnimParams &params)")
        return true;
    }
    PROFILE_STOP("CatzEngine::AnimKeys::pos(Vec &pos, C AnimParams &params)")
}
Bool AnimKeys::scale(Vec &scale, C AnimParams &params) C {
    PROFILE_START("CatzEngine::AnimKeys::scale(Vec &scale, C AnimParams &params)")
    switch (scales.elms()) {
    case 0:
        if (SET_ON_FAIL)
            scale.zero();
        PROFILE_STOP("CatzEngine::AnimKeys::scale(Vec &scale, C AnimParams &params)")
        return false;
    case 1:
    first:
        scale = scales[0].scale;
        PROFILE_STOP("CatzEngine::AnimKeys::scale(Vec &scale, C AnimParams &params)")
        return true;
    default: {
        Int l, r;
        for (l = 0, r = scales.elms(); l < r;) {
            Int mid = UInt(l + r) / 2;
            if (params.time < scales[mid].time)
                r = mid;
            else
                l = mid + 1;
        }
        Int prev = l - 1, next = l;
        C Scale *p, *n;
        Flt step;
        if (prev < 0) // before first key
        {
            if (params.loop) {
                prev = scales.elms() - 1; // last
                p = &scales[prev];
                n = &scales[next];
                if (Flt length = params.length - p->time + n->time)
                    step = (params.time - p->time + params.length) / length;
                else
                    goto first;
            } else
                goto first;
        } else if (next == scales.elms()) // after last key
        {
            if (params.loop) {
                next = 0; // first
                p = &scales[prev];
                n = &scales[next];
                if (Flt length = params.length - p->time + n->time)
                    step = (params.time - p->time) / length;
                else
                    goto last;
            } else {
            last:
                scale = scales.last().scale;
                PROFILE_STOP("CatzEngine::AnimKeys::scale(Vec &scale, C AnimParams &params)")
                return true;
            }
        } else {
            p = &scales[prev];
            n = &scales[next];
            if (Flt length = n->time - p->time)
                step = (params.time - p->time) / length;
            else {
                scale = p->scale;
                PROFILE_STOP("CatzEngine::AnimKeys::scale(Vec &scale, C AnimParams &params)")
                return true;
            }
        }
        if (params.linear)
            scale = Lerp(p->scale, n->scale, step);
        else {
#if HAS_ANIM_TANGENT
            scale = LerpTan(prev->scale, next->scale, step, prev->tan, next->tan);
#else
            C Scale *p2, *n2;
            if (params.loop) {
                p2 = &scales[(prev - 1 + scales.elms()) % scales.elms()];
                if (prev == 0 && Equal(p2->time, p->time + params.length))
                    p2 = &scales[scales.elms() - 2];
                n2 = &scales[(next + 1) % scales.elms()];
                if (next == scales.elms() - 1 && Equal(n->time, n2->time + params.length))
                    n2 = &scales[1];
            } else {
                p2 = &scales[Max(prev - 1, 0)];
                n2 = &scales[Min(next + 1, scales.elms() - 1)];
            }
            scale = Lerp4(p2->scale, p->scale, n->scale, n2->scale, step);
#endif
        }
    }
        PROFILE_STOP("CatzEngine::AnimKeys::scale(Vec &scale, C AnimParams &params)")
        return true;
    }
    PROFILE_STOP("CatzEngine::AnimKeys::scale(Vec &scale, C AnimParams &params)")
}
#if HAS_ANIM_ROT
Bool AnimKeys::rot(AxisRoll &rot, C AnimParams &params) C {
    PROFILE_START("CatzEngine::AnimKeys::rot(AxisRoll &rot, C AnimParams &params)")
    switch (rots.elms()) {
    case 0:
        if (SET_ON_FAIL)
            rot.zero();
        PROFILE_STOP("CatzEngine::AnimKeys::rot(AxisRoll &rot, C AnimParams &params)")
        return false;
    case 1:
    first:
        rot = rots[0].rot;
        PROFILE_STOP("CatzEngine::AnimKeys::rot(AxisRoll &rot, C AnimParams &params)")
        return true;
    default: {
        Int l, r;
        for (l = 0, r = rots.elms(); l < r;) {
            Int mid = UInt(l + r) / 2;
            if (params.time < rots[mid].time)
                r = mid;
            else
                l = mid + 1;
        }
        Int prev = l - 1, next = l;
        C Rot *p, *n;
        Flt step;
        if (prev < 0) // before first key
        {
            if (params.loop) {
                prev = rots.elms() - 1; // last
                p = &rots[prev];
                n = &rots[next];
                if (Flt length = params.length - p->time + n->time)
                    step = (params.time - p->time + params.length) / length;
                else
                    goto first;
            } else
                goto first;
        } else if (next == rots.elms()) // after last key
        {
            if (params.loop) {
                next = 0; // first
                p = &rots[prev];
                n = &rots[next];
                if (Flt length = params.length - p->time + n->time)
                    step = (params.time - p->time) / length;
                else
                    goto last;
            } else {
            last:
                rot = rots.last().rot;
                PROFILE_STOP("CatzEngine::AnimKeys::rot(AxisRoll &rot, C AnimParams &params)")
                return true;
            }
        } else {
            p = &rots[prev];
            n = &rots[next];
            if (Flt length = n->time - p->time)
                step = (params.time - p->time) / length;
            else {
                rot = p->rot;
                PROFILE_STOP("CatzEngine::AnimKeys::rot(AxisRoll &rot, C AnimParams &params)")
                return true;
            }
        }
        if (params.linear)
            rot.v4() = Lerp(p->rot.v4(), n->rot.v4(), step);
        else {
#if HAS_ANIM_TANGENT
            rot.v4() = LerpTan(prev->rot.v4(), next->rot.v4(), step, prev->tan.v4(), next->tan.v4());
#else
            C Rot *p2, *n2;
            if (params.loop) {
                p2 = &rots[(prev - 1 + rots.elms()) % rots.elms()];
                if (prev == 0 && Equal(p2->time, p->time + params.length))
                    p2 = &rots[rots.elms() - 2];
                n2 = &rots[(next + 1) % rots.elms()];
                if (next == rots.elms() - 1 && Equal(n->time, n2->time + params.length))
                    n2 = &rots[1];
            } else {
                p2 = &rots[Max(prev - 1, 0)];
                n2 = &rots[Min(next + 1, rots.elms() - 1)];
            }
            rot.v4() = Lerp4(p2->rot.v4(), p->rot.v4(), n->rot.v4(), n2->rot.v4(), step);
#endif
        }
    }
        PROFILE_STOP("CatzEngine::AnimKeys::rot(AxisRoll &rot, C AnimParams &params)")
        return true;
    }
    PROFILE_STOP("CatzEngine::AnimKeys::rot(AxisRoll &rot, C AnimParams &params)")
}
#endif
#if HAS_ANIM_COLOR
Bool AnimKeys::color(Vec4 &color, C AnimParams &params) C {
    PROFILE_START("CatzEngine::AnimKeys::color(Vec4 &color, C AnimParams &params)")
    switch (colors.elms()) {
    case 0:
        if (SET_ON_FAIL)
            color = 1;
        PROFILE_STOP("CatzEngine::AnimKeys::color(Vec4 &color, C AnimParams &params)")
        return false;
    case 1:
    first:
        color = colors[0].color;
        PROFILE_STOP("CatzEngine::AnimKeys::color(Vec4 &color, C AnimParams &params)")
        return true;
    default: {
        Int l, r;
        for (l = 0, r = colors.elms(); l < r;) {
            Int mid = UInt(l + r) / 2;
            if (params.time < colors[mid].time)
                r = mid;
            else
                l = mid + 1;
        }
        Int prev = l - 1, next = l;
        C Color *p, *n;
        Flt step;
        if (prev < 0) // before first key
        {
            if (params.loop) {
                prev = colors.elms() - 1; // last
                p = &colors[prev];
                n = &colors[next];
                if (Flt length = params.length - p->time + n->time)
                    step = (params.time - p->time + params.length) / length;
                else
                    goto first;
            } else
                goto first;
        } else if (next == colors.elms()) // after last key
        {
            if (params.loop) {
                next = 0; // first
                p = &colors[prev];
                n = &colors[next];
                if (Flt length = params.length - p->time + n->time)
                    step = (params.time - p->time) / length;
                else
                    goto last;
            } else {
            last:
                color = colors.last().color;
                PROFILE_STOP("CatzEngine::AnimKeys::color(Vec4 &color, C AnimParams &params)")
                return true;
            }
        } else {
            p = &colors[prev];
            n = &colors[next];
            if (Flt length = n->time - p->time)
                step = (params.time - p->time) / length;
            else {
                color = p->color;
                PROFILE_STOP("CatzEngine::AnimKeys::color(Vec4 &color, C AnimParams &params)")
                return true;
            }
        }
        if (params.linear)
            color = Lerp(p->color, n->color, step);
        else {
#if HAS_ANIM_TANGENT
            color = LerpTan(prev->color, next->color, step, prev->tan, next->tan);
#else
            C Color *p2, *n2;
            if (params.loop) {
                p2 = &colors[(prev - 1 + colors.elms()) % colors.elms()];
                if (prev == 0 && Equal(p2->time, p->time + params.length))
                    p2 = &colors[colors.elms() - 2];
                n2 = &colors[(next + 1) % colors.elms()];
                if (next == colors.elms() - 1 && Equal(n->time, n2->time + params.length))
                    n2 = &colors[1];
            } else {
                p2 = &colors[Max(prev - 1, 0)];
                n2 = &colors[Min(next + 1, colors.elms() - 1)];
            }
            color = Lerp4(p2->color, p->color, n->color, n2->color, step);
#endif
        }
    }
        PROFILE_STOP("CatzEngine::AnimKeys::color(Vec4 &color, C AnimParams &params)")
        return true;
    }
    PROFILE_STOP("CatzEngine::AnimKeys::color(Vec4 &color, C AnimParams &params)")
}
#endif
/******************************************************************************/
void AnimKeys::matrixNoScale(Matrix &matrix, C AnimParams &params) C {
    PROFILE_START("CatzEngine::AnimKeys::matrixNoScale(Matrix &matrix, C AnimParams &params)")
    Orient orn;
    if (T.orn(orn, params))
        matrix.orn() = orn;
    else
        matrix.orn().identity(); // if empty then call 'identity' because it's faster than setting from 'Orient'
    if (!T.pos(matrix.pos, params) && !SET_ON_FAIL)
        matrix.pos.zero();
    PROFILE_STOP("CatzEngine::AnimKeys::matrixNoScale(Matrix &matrix, C AnimParams &params)")
}
void AnimKeys::matrix(Matrix &matrix, C AnimParams &params) C {
    PROFILE_START("CatzEngine::AnimKeys::matrix(Matrix &matrix, C AnimParams &params)")
    matrixNoScale(matrix, params);
    Vec scale;
    if (T.scales.elms() && T.scale(scale, params))
        matrix.scaleOrnL(ScaleFactor(scale)); // most likely there won't be any scale, so do a fast check without the function call
    PROFILE_STOP("CatzEngine::AnimKeys::matrix(Matrix &matrix, C AnimParams &params)")
}
void AnimKeys::matrix(Matrix3 &matrix, C AnimParams &params, C Matrix3 &default_orn) C {
    PROFILE_START("CatzEngine::AnimKeys::matrix(Matrix3 &matrix, C AnimParams &params, C Matrix3 &default_orn)")
    Orient orn;
    if (T.orn(orn, params))
        matrix = orn;
    else
        matrix = default_orn;
    Vec scale;
    if (T.scales.elms() && T.scale(scale, params))
        matrix.scaleL(ScaleFactor(scale)); // most likely there won't be any scale, so do a fast check without the function call
    PROFILE_STOP("CatzEngine::AnimKeys::matrix(Matrix3 &matrix, C AnimParams &params, C Matrix3 &default_orn)")
}
void AnimKeys::matrix(Matrix &matrix, C AnimParams &params, C Matrix3 &default_orn) C {
    PROFILE_START("CatzEngine::AnimKeys::matrix(Matrix &matrix, C AnimParams &params, C Matrix3 &default_orn)")
    Orient orn;
    if (T.orn(orn, params))
        matrix.orn() = orn;
    else
        matrix.orn() = default_orn;
    if (!T.pos(matrix.pos, params) && !SET_ON_FAIL)
        matrix.pos.zero();
    Vec scale;
    if (T.scales.elms() && T.scale(scale, params))
        matrix.scaleOrnL(ScaleFactor(scale)); // most likely there won't be any scale, so do a fast check without the function call
    PROFILE_STOP("CatzEngine::AnimKeys::matrix(Matrix &matrix, C AnimParams &params, C Matrix3 &default_orn)")
}
/******************************************************************************/
void AnimKeys::Orn::save(MemPtr<TextNode> nodes) C {
    nodes.New().set("time", time);
    nodes.New().set("orn.dir", orn.dir);
    nodes.New().set("orn.perp", orn.perp);
#if HAS_ANIM_TANGENT
    nodes.New().set("tan.dir", tan.dir);
    nodes.New().set("tan.perp", tan.perp);
#endif
}
void AnimKeys::Pos::save(MemPtr<TextNode> nodes) C {
    nodes.New().set("time", time);
    nodes.New().set("pos", pos);
#if HAS_ANIM_TANGENT
    nodes.New().set("tan", tan);
#endif
}
void AnimKeys::Scale::save(MemPtr<TextNode> nodes) C {
    nodes.New().set("time", time);
    nodes.New().set("scale", scale);
#if HAS_ANIM_TANGENT
    nodes.New().set("tan", tan);
#endif
}
#if HAS_ANIM_ROT
void AnimKeys::Rot::save(MemPtr<TextNode> nodes) C {
    nodes.New().set("time", time);
    nodes.New().set("rot.axis", rot.axis);
    nodes.New().set("rot.roll", rot.roll);
#if HAS_ANIM_TANGENT
    nodes.New().set("tan.axis", tan.axis);
    nodes.New().set("tan.roll", tan.roll);
#endif
}
#endif
#if HAS_ANIM_COLOR
void AnimKeys::Color::save(MemPtr<TextNode> nodes) C {
    nodes.New().set("time", time);
    nodes.New().set("color", color);
#if HAS_ANIM_TANGENT
    nodes.New().set("tan", tan);
#endif
}
#endif
/******************************************************************************/
AnimKeys &AnimKeys::del() {
    orns.del();
    poss.del();
    scales.del();
#if HAS_ANIM_ROT
    rots.del();
#endif
#if HAS_ANIM_COLOR
    colors.del();
#endif
    return T;
}
/******************************************************************************/
Bool AnimKeys::timeRange(Flt &min, Flt &max) C {
    PROFILE_START("CatzEngine::AnimKeys::timeRange(Flt &min, Flt &max)")
    if (!is()) {
        min = max = 0;
        PROFILE_STOP("CatzEngine::AnimKeys::timeRange(Flt &min, Flt &max)")
        return false;
    }
    min = FLT_MAX;
    max = -FLT_MAX;
    REPA(orns) {
        Flt t = orns[i].time;
        MIN(min, t);
        MAX(max, t);
    }
    REPA(poss) {
        Flt t = poss[i].time;
        MIN(min, t);
        MAX(max, t);
    }
    REPA(scales) {
        Flt t = scales[i].time;
        MIN(min, t);
        MAX(max, t);
    }
#if HAS_ANIM_ROT
    REPA(rots) {
        Flt t = rots[i].time;
        MIN(min, t);
        MAX(max, t);
    }
#endif
#if HAS_ANIM_COLOR
    REPA(colors) {
        Flt t = colors[i].time;
        MIN(min, t);
        MAX(max, t);
    }
#endif
    PROFILE_STOP("CatzEngine::AnimKeys::timeRange(Flt &min, Flt &max)")
    return true;
}
AnimKeys &AnimKeys::scale(Flt scale) {
    PROFILE_STOP("CatzEngine::AnimKeys::scale(Flt scale)")
    REPA(poss) {
        Pos &pos = T.poss[i];
        pos.pos *= scale;
#if HAS_ANIM_TANGENT
        pos.tan *= scale;
#endif
    }
    PROFILE_STOP("CatzEngine::AnimKeys::scale(Flt scale)")
    return T;
}
AnimKeys &AnimKeys::mirrorX() {
    PROFILE_START("CatzEngine::AnimKeys::mirrorX()")
    REPA(orns) {
        Orn &orn = T.orns[i];
        orn.orn.mirrorX();
#if HAS_ANIM_TANGENT
        orn.tan.mirrorX();
#endif
    }
    REPA(poss) {
        Pos &pos = T.poss[i];
        CHS(pos.pos.x);
#if HAS_ANIM_TANGENT
        CHS(pos.tan.x);
#endif
    }
#if HAS_ANIM_ROT
    REPA(rots) {
        Rot &rot = T.rots[i];
        CHS(rot.rot.axis.y);
        CHS(rot.rot.axis.z);
        CHS(rot.rot.roll);
#if HAS_ANIM_TANGENT
        CHS(rot.tan.axis.y);
        CHS(rot.tan.axis.z);
        CHS(rot.tan.roll);
#endif
    }
#endif
    PROFILE_STOP("CatzEngine::AnimKeys::mirrorX()")
    return T;
}
AnimKeys &AnimKeys::transform(C Matrix3 &matrix) // assumes that 'matrix' is normalized
{
    PROFILE_START("CatzEngine::AnimKeys::transform(C Matrix3 &matrix)")
    REPA(orns) {
        Orn &orn = T.orns[i];
        orn.orn.mul(matrix, true);
#if HAS_ANIM_TANGENT
        orn.tan.mul(matrix, true);
#endif
    }
    REPA(poss) {
        Pos &pos = T.poss[i];
        pos.pos *= matrix;
#if HAS_ANIM_TANGENT
        pos.tan *= matrix;
#endif
    }
#if HAS_ANIM_ROT
    REPA(rots) {
        Rot &rot = T.rots[i];
        rot.rot *= matrix;
#if HAS_ANIM_TANGENT
        rot.tan *= matrix;
#endif
    }
#endif
    PROFILE_STOP("CatzEngine::AnimKeys::transform(C Matrix3 &matrix)")
    return T;
}
/******************************************************************************/
static Int Compare(C AnimKeys::Orn &k0, C AnimKeys::Orn &k1) { return Compare(k0.time, k1.time); }
static Int Compare(C AnimKeys::Pos &k0, C AnimKeys::Pos &k1) { return Compare(k0.time, k1.time); }
static Int Compare(C AnimKeys::Scale &k0, C AnimKeys::Scale &k1) { return Compare(k0.time, k1.time); }
#if HAS_ANIM_ROT
static Int Compare(C AnimKeys::Rot &k0, C AnimKeys::Rot &k1) { return Compare(k0.time, k1.time); }
#endif
#if HAS_ANIM_COLOR
static Int Compare(C AnimKeys::Color &k0, C AnimKeys::Color &k1) { return Compare(k0.time, k1.time); }
#endif

static Int Compare(C AnimKeys::Orn &k, C Flt &time) { return Compare(k.time, time); }
static Int Compare(C AnimKeys::Pos &k, C Flt &time) { return Compare(k.time, time); }
static Int Compare(C AnimKeys::Scale &k, C Flt &time) { return Compare(k.time, time); }

static Int CompareEps(C AnimKeys::Orn &k, C Flt &time) { return CompareEps(k.time, time); }
static Int CompareEps(C AnimKeys::Pos &k, C Flt &time) { return CompareEps(k.time, time); }

AnimKeys &AnimKeys::sortFrames() {
    PROFILE_START("CatzEngine::AnimKeys::sortFrames()")
    orns.sort(Compare);
    poss.sort(Compare);
    scales.sort(Compare);
#if HAS_ANIM_ROT
    rots.sort(Compare);
#endif
#if HAS_ANIM_COLOR
    colors.sort(Compare);
#endif
    PROFILE_STOP("CatzEngine::AnimKeys::sortFrames()")
    return T;
}
struct TimeScaleClip {
    Flt scale, length, length_eps;

    TimeScaleClip(Flt scale, Flt length) {
        T.scale = scale;
        T.length = length;
        T.length_eps = length - EPS;
    }
    void adjust(Flt &time) C {
        Flt t = time * scale;
        if (t > length_eps)
            t = length; // align to end, because looped root motion is calculated at time=0 and time=length, so if times are slightly offsetted, we could get root motion start that is actually end
        time = t;
    }
};
AnimKeys &AnimKeys::scaleTime(Flt scale, Flt anim_length) {
    PROFILE_START("CatzEngine::AnimKeys::scaleTime(Flt scale, Flt anim_length)")
    TimeScaleClip tsc(scale, anim_length);
    REPA(orns)
    tsc.adjust(orns[i].time);
    REPA(poss)
    tsc.adjust(poss[i].time);
    REPA(scales)
    tsc.adjust(scales[i].time);
#if HAS_ANIM_ROT
    REPA(rots)
    tsc.adjust(rots[i].time);
#endif
#if HAS_ANIM_COLOR
    REPA(colors)
    tsc.adjust(colors[i].time);
#endif
    PROFILE_STOP("CatzEngine::AnimKeys::scaleTime(Flt scale, Flt anim_length)")
    return T;
}
AnimKeys &AnimKeys::offsetTime(Flt dt, Flt anim_length) {
    PROFILE_START("CatzEngine::AnimKeys::offsetTime(Flt dt, Flt anim_length)")
    REPAO(orns).time = Frac(orns[i].time + dt, anim_length);
    REPAO(poss).time = Frac(poss[i].time + dt, anim_length);
    REPAO(scales).time = Frac(scales[i].time + dt, anim_length);
#if HAS_ANIM_ROT
    REPAO(rots).time = Frac(rots[i].time + dt, anim_length);
#endif
#if HAS_ANIM_COLOR
    REPAO(colors).time = Frac(colors[i].time + dt, anim_length);
#endif
    PROFILE_STOP("CatzEngine::AnimKeys::offsetTime(Flt dt, Flt anim_length)")
    return sortFrames();
}
AnimKeys &AnimKeys::reverse(Flt anim_length) {
    PROFILE_START("CatzEngine::AnimKeys::reverse(Flt anim_length)")
    REPA(orns) {
        Orn &orn = orns[i];
        orn.time = anim_length - orn.time;
#if HAS_ANIM_TANGENT
        orn.tan.chs();
#endif
    }
    orns.reverseOrder();

    REPA(poss) {
        Pos &pos = poss[i];
        pos.time = anim_length - pos.time;
#if HAS_ANIM_TANGENT
        pos.tan.chs();
#endif
    }
    poss.reverseOrder();

    REPA(scales) {
        Scale &scale = scales[i];
        scale.time = anim_length - scale.time;
#if HAS_ANIM_TANGENT
        scale.tan.chs();
#endif
    }
    scales.reverseOrder();

#if HAS_ANIM_ROT
    REPA(rots) {
        Rot &rot = rots[i];
        rot.time = anim_length - rot.time;
#if HAS_ANIM_TANGENT
        rot.tan.v4().chs();
#endif
    }
    rots.reverseOrder();
#endif

#if HAS_ANIM_COLOR
    REPA(colors) {
        Color &color = colors[i];
        color.time = anim_length - color.time;
#if HAS_ANIM_TANGENT
        color.tan.chs();
#endif
    }
    colors.reverseOrder();
#endif
    PROFILE_STOP("CatzEngine::AnimKeys::reverse(Flt anim_length)")
    return T;
}

AnimKeys &AnimKeys::setTangents(Bool anim_loop, Flt anim_length) {
    PROFILE_START("CatzEngine::AnimKeys::setTangents(Bool anim_loop, Flt anim_length)")
#if HAS_ANIM_TANGENT
    REPA(orns) {
        Int prev = i - 1,
            next = i + 1;
        if (prev == -1) {
            if (anim_loop) {
                prev += orns.elms();
                if (Equal(orns[i].time, 0) && Equal(orns[prev].time, anim_length))
                    prev--;
            }
        }
        if (next == orns.elms()) {
            if (anim_loop) {
                next -= orns.elms();
                if (Equal(orns[next].time, 0) && Equal(orns[i].time, anim_length))
                    next++;
            } else
                next = -1;
        }
        if (InRange(prev, orns) && InRange(next, orns)) {
            orns[i].tan.dir = GetTangentDir(orns[prev].orn.dir, orns[next].orn.dir);
            orns[i].tan.perp = GetTangentDir(orns[prev].orn.perp, orns[next].orn.perp);
        } else
            orns[i].tan.zero();
    }
    REPA(poss) {
        Int prev = i - 1,
            next = i + 1;
        if (prev == -1) {
            if (anim_loop) {
                prev += poss.elms();
                if (Equal(poss[i].time, 0) && Equal(poss[prev].time, anim_length))
                    prev--;
            }
        }
        if (next == poss.elms()) {
            if (anim_loop) {
                next -= poss.elms();
                if (Equal(poss[next].time, 0) && Equal(poss[i].time, anim_length))
                    next++;
            } else
                next = -1;
        }
#if 1
        if (InRange(prev, poss) && InRange(next, poss))
            poss[i].tan = GetTangent(poss[prev].pos, poss[i].pos, poss[next].pos); // this gives better results for circular movement
#else
        if (InRange(prev, poss) && InRange(next, poss))
            poss[i].tan = GetTangent(poss[prev].pos, poss[next].pos);
#endif
        else
            poss[i].tan.zero();
    }
    REPA(scales) {
        Int prev = i - 1,
            next = i + 1;
        if (prev == -1) {
            if (anim_loop) {
                prev += scales.elms();
                if (Equal(scales[i].time, 0) && Equal(scales[prev].time, anim_length))
                    prev--;
            }
        }
        if (next == scales.elms()) {
            if (anim_loop) {
                next -= scales.elms();
                if (Equal(scales[next].time, 0) && Equal(scales[i].time, anim_length))
                    next++;
            } else
                next = -1;
        }
        if (InRange(prev, scales) && InRange(next, scales))
            scales[i].tan = GetTangent(scales[prev].scale, scales[next].scale);
        else
            scales[i].tan.zero();
    }
#if HAS_ANIM_ROT
    REPA(rots) {
        Int prev = i - 1,
            next = i + 1;
        if (prev == -1) {
            if (anim_loop) {
                prev += rots.elms();
                if (Equal(rots[i].time, 0) && Equal(rots[prev].time, anim_length))
                    prev--;
            }
        }
        if (next == rots.elms()) {
            if (anim_loop) {
                next -= rots.elms();
                if (Equal(rots[next].time, 0) && Equal(rots[i].time, anim_length))
                    next++;
            } else
                next = -1;
        }
        if (InRange(prev, rots) && InRange(next, rots))
            rots[i].tan.v4() = GetTangent(rots[prev].rot.v4(), rots[next].rot.v4());
        else
            rots[i].tan.zero();
    }
#endif
#if HAS_ANIM_COLOR
    REPA(colors) {
        Int prev = i - 1,
            next = i + 1;
        if (prev == -1) {
            if (anim_loop) {
                prev += colors.elms();
                if (Equal(colors[i].time, 0) && Equal(colors[prev].time, anim_length))
                    prev--;
            }
        }
        if (next == colors.elms()) {
            if (anim_loop) {
                next -= colors.elms();
                if (Equal(colors[next].time, 0) && Equal(colors[i].time, anim_length))
                    next++;
            } else
                next = -1;
        }
        if (InRange(prev, colors) && InRange(next, colors))
            colors[i].tan = GetTangent(colors[prev].color, colors[next].color);
        else
            colors[i].tan.zero();
    }
#endif
#endif
    PROFILE_STOP("CatzEngine::AnimKeys::setTangents(Bool anim_loop, Flt anim_length)")
    return T;
}
/******************************************************************************/
// Here operate on Angles instead of Dot, because precision for Flt is too low (example: dot_eps=Cos(angle_eps); for angle_eps=0.0002f gives dot_eps=1.0f)
static Flt MaxAbsAngleBetween(C Orient &a, C Orient &b) {
    return Max(AbsAngleBetweenN(a.dir, b.dir), AbsAngleBetweenN(a.perp, b.perp));
}
static Flt MaxAbsAngleBetween(C AxisRoll &a, C AxisRoll &b) {
    return Dist(a.axis, b.axis) + Abs(a.roll - b.roll);
}
static Flt MaxAbsDelta(C Vec &a, C Vec &b) {
    return Abs(a - b).max();
}
static Flt MaxAbsDelta(C Vec4 &a, C Vec4 &b) {
    return Abs(a - b).max();
}
static Int Index(Int i, Bool loop, Int elms) {
    return loop ? Mod(i, elms) : Mid(i, 0, elms - 1);
}
static Int Index(Int i, Bool loop, Int elms, Int ignore) // this assumes that "elms>=2" and "InRange(ignore, elms)"
{
    PROFILE_START("CatzEngine::Int Index(Int i, Bool loop, Int elms, Int ignore)")
    if (i > ignore)
        i--;
    i = (loop ? Mod(i, elms - 1) : Mid(i, 0, elms - 2)); // add extra -1 to 'elms' because we need it for 'ignore'
    if (i >= ignore)
        i++;
    PROFILE_STOP("CatzEngine::Int Index(Int i, Bool loop, Int elms, Int ignore)")
    return i;
}
static Flt AfterTime(Flt time, Flt base, Flt length) {
    return (time >= base) ? time : time + length;
}
AnimKeys &AnimKeys::optimize(Bool anim_loop, Bool anim_linear, Flt anim_length, Flt angle_eps, Flt pos_eps, Flt scale_eps, C Orient *bone, C Orient *bone_parent) {
    PROFILE_START("CatzEngine::AnimKeys::optimize(Bool anim_loop, Bool anim_linear, Flt anim_length, Flt angle_eps, Flt pos_eps, Flt scale_eps, C Orient *bone, C Orient *bone_parent)")
    AnimParams anim_params(anim_loop, anim_linear, anim_length, 0);
    Memt<Int> optimized; // indexes to original elements

    // orientation
    if (angle_eps >= 0) {
        optimized.setNumDiscard(orns.elms());
        REPAO(optimized) = i;
        if (optimized.elms() >= 2) {
            Bool same_first_last = (MaxAbsAngleBetween(orns.first().orn, orns.last().orn) <= angle_eps); // if first and last keys are the same
            REPA(optimized)
            if (optimized.elms() >= 2)
                if (same_first_last || (i > 0 && i < optimized.elms() - 1)) // don't remove first-last keys, in case animation has later looping toggled which could cause issues (for example root motion turning animation, imported as non-looping, finished 90 deg turn at 50% animation time, and the last half keys are removed, then switched to looping, and turn gets reverted back to start due to loop and last keys missing), however allow removing if keys are the same
                {                                                           // here ignore this 'i' key, because we want to test if keys with 'i' removed will provide similar results as with 'i' present
                    Int prev2 = optimized[Index(i - 2, anim_loop, optimized.elms(), i)],
                        prev = optimized[Index(i - 1, anim_loop, optimized.elms(), i)],
                        next = optimized[Index(i + 1, anim_loop, optimized.elms(), i)],
                        next2 = optimized[Index(i + 2, anim_loop, optimized.elms(), i)],
                        stop = (InRange(i + 1, optimized) ? optimized[Min(i + (anim_linear ? 1 : 2), optimized.elms() - 1)] : orns.elms()); // stop on the next key that wasn't removed (for non-linear use 2nd next key)
                    C Orn *p = &orns[prev], *n = &orns[next], *p2 = &orns[prev2], *n2 = &orns[next2];
                    Flt n_time = AfterTime(n->time, p->time, anim_length);
                    Orient test;

                    // check between this and previous key !! this is extra important for orientations, both Linear and Cubic !!
                    {
                        C Orn &orn = orns[i];
                        Flt prev_time = (InRange(i - 1, orns) ? orns[i - 1].time : anim_loop ? orns.last().time - anim_length
                                                                                             : 0);
                        if (!Equal(prev_time, orn.time)) // if times are the same then we don't need to do the following (this can happen when first key is at the start of anim and (it's not looped or the last key is at the end))
                        {
                            anim_params.time = Avg(prev_time, orn.time); // set time between previous and this key from the source

                            // calculate source orn (with 'i' key present)
                            Int src_prev = Index(i - 1, anim_loop, orns.elms()),
                                src_next = i; // here 'i' is included
                            Orient orn;
                            C Orn &src_p = orns[src_prev], &src_n = orns[src_next];
                            Flt step = 0.5f; // here for source, step is always 0.5, because we want to test between keys
                            if (anim_linear) {
                                orn.dir = Lerp(src_p.orn.dir, src_n.orn.dir, step);
                                orn.perp = Lerp(src_p.orn.perp, src_n.orn.perp, step);
                            } else {
#if HAS_ANIM_TANGENT
                                orn.dir = LerpTan(src_p.orn.dir, src_n.orn.dir, step, src_p.tan.dir, src_n.tan.dir);
                                orn.perp = LerpTan(src_p.orn.perp, src_n.orn.perp, step, src_p.tan.perp, src_n.tan.perp);
#else
                                Int src_prev2 = Index(i - 2, anim_loop, orns.elms()),
                                    src_next2 = Index(i + 1, anim_loop, orns.elms());
                                C Orn &src_p2 = orns[src_prev2], &src_n2 = orns[src_next2];
                                orn.dir = Lerp4(src_p2.orn.dir, src_p.orn.dir, src_n.orn.dir, src_n2.orn.dir, step);
                                orn.perp = Lerp4(src_p2.orn.perp, src_p.orn.perp, src_n.orn.perp, src_n2.orn.perp, step);
#endif
                            }
                            orn.fix();

                            // calculate optimized orn (with 'i' key removed)
                            step = LerpRS(p->time, n_time, AfterTime(anim_params.time, p->time, anim_length)); // use Sat in case orn.time is outside of range, or 'p' has the same time as 'n'
                            if (anim_linear) {
                                test.dir = Lerp(p->orn.dir, n->orn.dir, step);
                                test.perp = Lerp(p->orn.perp, n->orn.perp, step);
                            } else {
#if HAS_ANIM_TANGENT
                                test.dir = LerpTan(p->orn.dir, n->orn.dir, step, GetTangentDir(p2->orn.dir, n->orn.dir), GetTangentDir(p->orn.dir, n2->orn.dir));
                                test.perp = LerpTan(p->orn.perp, n->orn.perp, step, GetTangentDir(p2->orn.perp, n->orn.perp), GetTangentDir(p->orn.perp, n2->orn.perp));
#else
                                test.dir = Lerp4(p2->orn.dir, p->orn.dir, n->orn.dir, n2->orn.dir, step);
                                test.perp = Lerp4(p2->orn.perp, p->orn.perp, n->orn.perp, n2->orn.perp, step);
#endif
                            }
                            test.fix();

                            if (MaxAbsAngleBetween(test, orn) > angle_eps)
                                goto orn_needed;
                        }
                    }

                    for (Int src = i, i_next = i + 3;;) // check that all removed keyframes (including this) do not deviate, start from the left because below we're checking if we've reached the next index, we need to go only to the right, because to the left no keys have been removed yet
                    {
                        C Orn &orn = orns[src];
#if HAS_ANIM_TANGENT
                        Orient tan[2];
                        if (!anim_linear) {
                            tan[0].dir = GetTangentDir(p2->orn.dir, n->orn.dir);
                            tan[0].perp = GetTangentDir(p2->orn.perp, n->orn.perp);
                            tan[1].dir = GetTangentDir(p->orn.dir, n2->orn.dir);
                            tan[1].perp = GetTangentDir(p->orn.perp, n2->orn.perp);
                        }
#endif
                        // check at key location
                        if (src != prev) // no need to check time where both source and optimized have a keyframe
                        {
                            Flt step = LerpRS(p->time, n_time, AfterTime(orn.time, p->time, anim_length)); // use Sat in case orn.time is outside of range, or 'p' has the same time as 'n'
                            if (anim_linear) {
                                test.dir = Lerp(p->orn.dir, n->orn.dir, step);
                                test.perp = Lerp(p->orn.perp, n->orn.perp, step);
                            } else {
#if HAS_ANIM_TANGENT
                                test.dir = LerpTan(p->orn.dir, n->orn.dir, step, tan[0].dir, tan[1].dir);
                                test.perp = LerpTan(p->orn.perp, n->orn.perp, step, tan[0].perp, tan[1].perp);
#else
                                test.dir = Lerp4(p2->orn.dir, p->orn.dir, n->orn.dir, n2->orn.dir, step);
                                test.perp = Lerp4(p2->orn.perp, p->orn.perp, n->orn.perp, n2->orn.perp, step);
#endif
                            }
                            test.fix();
                            if (MaxAbsAngleBetween(test, orn.orn) > angle_eps)
                                goto orn_needed;
                        }

                        // check between keys
                        if (!anim_linear) // for cubic interpolation we need to check between keys, because due to cubic interpolation the values can get way off
                        {
                            Flt next_time = (InRange(src + 1, orns) ? orns[src + 1].time : anim_loop ? orns[0].time + anim_length
                                                                                                     : anim_length);
                            if (!Equal(orn.time, next_time)) // if times are the same then we don't need to do the following (this can happen when last key is at the end of anim and (it's not looped or the first key is at the start))
                            {
                                anim_params.time = Avg(orn.time, next_time); // set time between this and next key from the source
                                Orient orn;
                                T.orn(orn, anim_params);
                                Flt step = LerpRS(p->time, n_time, AfterTime(anim_params.time, p->time, anim_length)); // use Sat in case orn.time is outside of range, or 'p' has the same time as 'n'
#if HAS_ANIM_TANGENT
                                test.dir = LerpTan(p->orn.dir, n->orn.dir, step, tan[0].dir, tan[1].dir);
                                test.perp = LerpTan(p->orn.perp, n->orn.perp, step, tan[0].perp, tan[1].perp);
#else
                                test.dir = Lerp4(p2->orn.dir, p->orn.dir, n->orn.dir, n2->orn.dir, step);
                                test.perp = Lerp4(p2->orn.perp, p->orn.perp, n->orn.perp, n2->orn.perp, step);
#endif
                                test.fix();
                                if (MaxAbsAngleBetween(test, orn) > angle_eps)
                                    goto orn_needed;
                            }
                        }

                        if (++src >= stop)
                            break;
                        if (src == next) // remember that 'next' can be wrapped due to looping, so use == for safety
                        {
                            prev2 = prev;
                            prev = next;
                            next = next2;
                            next2 = optimized[Index(i_next++, anim_loop, optimized.elms(), i)];
                            p = &orns[prev];
                            n = &orns[next];
                            p2 = &orns[prev2];
                            n2 = &orns[next2];
                            n_time = AfterTime(n->time, p->time, anim_length);
                        }
                    }

                    optimized.remove(i, true);
                orn_needed:;
                }
        }
        if (optimized.elms() == 1 && bone) // be careful when removing the only 'orn' keyframe even if it's an identity, because it may be needed for correct multi-animation blending
        {
            Orient identity = GetAnimOrient(*bone, bone_parent);
            if (MaxAbsAngleBetween(identity, orns[optimized[0]].orn) <= angle_eps)
                optimized.clear();
        }
        if (optimized.elms() != orns.elms()) {
            Mems<Orn> temp;
            temp.setNumDiscard(optimized.elms());
            REPAO(temp) = orns[optimized[i]];
            Swap(temp, orns);
        }
    }

    // position
    if (pos_eps >= 0) {
        pos_eps *= pos_eps; // make square
        optimized.setNumDiscard(poss.elms());
        REPAO(optimized) = i;
        if (optimized.elms() >= 2) {
            Bool same_first_last = (Dist2(poss.first().pos, poss.last().pos) <= pos_eps); // if first and last keys are the same
            REPA(optimized)
            if (optimized.elms() >= 2)
                if (same_first_last || (i > 0 && i < optimized.elms() - 1)) // don't remove first-last keys, in case animation has later looping toggled which could cause issues (for example root motion turning animation, imported as non-looping, finished 90 deg turn at 50% animation time, and the last half keys are removed, then switched to looping, and turn gets reverted back to start due to loop and last keys missing), however allow removing if keys are the same
                {
                    Int prev2 = optimized[Index(i - 2, anim_loop, optimized.elms(), i)],
                        prev = optimized[Index(i - 1, anim_loop, optimized.elms(), i)],
                        next = optimized[Index(i + 1, anim_loop, optimized.elms(), i)],
                        next2 = optimized[Index(i + 2, anim_loop, optimized.elms(), i)],
                        stop = (InRange(i + 1, optimized) ? optimized[Min(i + (anim_linear ? 1 : 2), optimized.elms() - 1)] : poss.elms()); // stop on the next key that wasn't removed (for non-linear use 2nd next key)
                    C Pos *p = &poss[prev], *n = &poss[next], *p2 = &poss[prev2], *n2 = &poss[next2];
                    Flt n_time = AfterTime(n->time, p->time, anim_length);
                    for (Int src = i, i_next = i + 3;;) // check that all removed keyframes (including this) do not deviate, start from the left because below we're checking if we've reached the next index, we need to go only to the right, because to the left no keys have been removed yet
                    {
                        C Pos &pos = poss[src];
                        Vec test;
#if HAS_ANIM_TANGENT
                        Vec tan[2];
                        if (!anim_linear) {
                            tan[0] = GetTangent(p2->pos, p->pos, n->pos);
                            tan[1] = GetTangent(p->pos, n->pos, n2->pos);
                        }
#endif
                        // check at key location
                        if (src != prev) // no need to check time where both source and optimized have a keyframe
                        {
                            Flt step = LerpRS(p->time, n_time, AfterTime(pos.time, p->time, anim_length)); // use Sat in case pos.time is outside of range, or 'p' has the same time as 'n'
                            if (anim_linear)
                                test = Lerp(p->pos, n->pos, step);
                            else {
#if HAS_ANIM_TANGENT
                                test = LerpTan(p->pos, n->pos, step, tan[0], tan[1]);
#else
                                test = Lerp4(p2->pos, p->pos, n->pos, n2->pos, step);
#endif
                            }
                            if (Dist2(test, pos.pos) > pos_eps)
                                goto pos_needed;
                        }

                        // check between keys
                        if (!anim_linear) // for cubic interpolation we need to check between keys, because due to cubic interpolation the values can get way off
                        {
                            Flt next_time = (InRange(src + 1, poss) ? poss[src + 1].time : anim_loop ? poss[0].time + anim_length
                                                                                                     : anim_length);
                            if (!Equal(pos.time, next_time)) // if times are the same then we don't need to do the following (this can happen when last key is at the end of anim and (it's not looped or the first key is at the start))
                            {
                                anim_params.time = Avg(pos.time, next_time); // set time between this and next key from the source
                                Vec pos;
                                T.pos(pos, anim_params);
                                Flt step = LerpRS(p->time, n_time, AfterTime(anim_params.time, p->time, anim_length)); // use Sat in case pos.time is outside of range, or 'p' has the same time as 'n'
                                if (anim_linear)
                                    test = Lerp(p->pos, n->pos, step);
                                else {
#if HAS_ANIM_TANGENT
                                    test = LerpTan(p->pos, n->pos, step, tan[0], tan[1]);
#else
                                    test = Lerp4(p2->pos, p->pos, n->pos, n2->pos, step);
#endif
                                }
                                if (Dist2(test, pos) > pos_eps)
                                    goto pos_needed;
                            }
                        }

                        if (++src >= stop)
                            break;
                        if (src == next) // remember that 'next' can be wrapped due to looping, so use == for safety
                        {
                            prev2 = prev;
                            prev = next;
                            next = next2;
                            next2 = optimized[Index(i_next++, anim_loop, optimized.elms(), i)];
                            p = &poss[prev];
                            n = &poss[next];
                            p2 = &poss[prev2];
                            n2 = &poss[next2];
                            n_time = AfterTime(n->time, p->time, anim_length);
                        }
                    }
                    optimized.remove(i, true);
                pos_needed:;
                }
        }
        if (optimized.elms() == 1 && poss[optimized[0]].pos.length2() <= pos_eps)
            optimized.clear();
        if (optimized.elms() != poss.elms()) {
            Mems<Pos> temp;
            temp.setNumDiscard(optimized.elms());
            REPAO(temp) = poss[optimized[i]];
            Swap(temp, poss);
        }
    }

    // scale
    if (scale_eps >= 0) {
        optimized.setNumDiscard(scales.elms());
        REPAO(optimized) = i;
        if (optimized.elms() >= 2) {
            Bool same_first_last = (MaxAbsDelta(scales.first().scale, scales.last().scale) <= scale_eps); // if first and last keys are the same
            REPA(optimized)
            if (optimized.elms() >= 2)
                if (same_first_last || (i > 0 && i < optimized.elms() - 1)) // don't remove first-last keys, in case animation has later looping toggled which could cause issues (for example root motion turning animation, imported as non-looping, finished 90 deg turn at 50% animation time, and the last half keys are removed, then switched to looping, and turn gets reverted back to start due to loop and last keys missing), however allow removing if keys are the same
                {
                    Int prev2 = optimized[Index(i - 2, anim_loop, optimized.elms(), i)],
                        prev = optimized[Index(i - 1, anim_loop, optimized.elms(), i)],
                        next = optimized[Index(i + 1, anim_loop, optimized.elms(), i)],
                        next2 = optimized[Index(i + 2, anim_loop, optimized.elms(), i)],
                        stop = (InRange(i + 1, optimized) ? optimized[Min(i + (anim_linear ? 1 : 2), optimized.elms() - 1)] : scales.elms()); // stop on the next key that wasn't removed (for non-linear use 2nd next key)
                    C Scale *p = &scales[prev], *n = &scales[next], *p2 = &scales[prev2], *n2 = &scales[next2];
                    Flt n_time = AfterTime(n->time, p->time, anim_length);
                    for (Int src = i, i_next = i + 3;;) // check that all removed keyframes (including this) do not deviate, start from the left because below we're checking if we've reached the next index, we need to go only to the right, because to the left no keys have been removed yet
                    {
                        C Scale &scale = scales[src];
                        Vec test;
#if HAS_ANIM_TANGENT
                        Vec tan[2];
                        if (!anim_linear) {
                            tan[0] = GetTangent(p2->scale, p->scale, n->scale);
                            tan[1] = GetTangent(p->scale, n->scale, n2->scale);
                        }
#endif
                        // check at key location
                        if (src != prev) // no need to check time where both source and optimized have a keyframe
                        {
                            Flt step = LerpRS(p->time, n_time, AfterTime(scale.time, p->time, anim_length)); // use Sat in case scale.time is outside of range, or 'p' has the same time as 'n'
                            if (anim_linear)
                                test = Lerp(p->scale, n->scale, step);
                            else {
#if HAS_ANIM_TANGENT
                                test = LerpTan(p->scale, n->scale, step, tan[0], tan[1]);
#else
                                test = Lerp4(p2->scale, p->scale, n->scale, n2->scale, step);
#endif
                            }
                            if (MaxAbsDelta(test, scale.scale) > scale_eps)
                                goto scale_needed;
                        }

                        // check between keys
                        if (!anim_linear) // for cubic interpolation we need to check between keys, because due to cubic interpolation the values can get way off
                        {
                            Flt next_time = (InRange(src + 1, scales) ? scales[src + 1].time : anim_loop ? scales[0].time + anim_length
                                                                                                         : anim_length);
                            if (!Equal(scale.time, next_time)) // if times are the same then we don't need to do the following (this can happen when last key is at the end of anim and (it's not looped or the first key is at the start))
                            {
                                anim_params.time = Avg(scale.time, next_time); // set time between this and next key from the source
                                Vec scale;
                                T.scale(scale, anim_params);
                                Flt step = LerpRS(p->time, n_time, AfterTime(anim_params.time, p->time, anim_length)); // use Sat in case scale.time is outside of range, or 'p' has the same time as 'n'
                                if (anim_linear)
                                    test = Lerp(p->scale, n->scale, step);
                                else {
#if HAS_ANIM_TANGENT
                                    test = LerpTan(p->scale, n->scale, step, tan[0], tan[1]);
#else
                                    test = Lerp4(p2->scale, p->scale, n->scale, n2->scale, step);
#endif
                                }
                                if (MaxAbsDelta(test, scale) > scale_eps)
                                    goto scale_needed;
                            }
                        }

                        if (++src >= stop)
                            break;
                        if (src == next) // remember that 'next' can be wrapped due to looping, so use == for safety
                        {
                            prev2 = prev;
                            prev = next;
                            next = next2;
                            next2 = optimized[Index(i_next++, anim_loop, optimized.elms(), i)];
                            p = &scales[prev];
                            n = &scales[next];
                            p2 = &scales[prev2];
                            n2 = &scales[next2];
                            n_time = AfterTime(n->time, p->time, anim_length);
                        }
                    }
                    optimized.remove(i, true);
                scale_needed:;
                }
        }
        if (optimized.elms() == 1 && MaxAbsDelta(scales[optimized[0]].scale, VecZero) <= scale_eps)
            optimized.clear();
        if (optimized.elms() != scales.elms()) {
            Mems<Scale> temp;
            temp.setNumDiscard(optimized.elms());
            REPAO(temp) = scales[optimized[i]];
            Swap(temp, scales);
        }
    }

#if HAS_ANIM_ROT
    // rotation
    if (angle_eps >= 0) {
        optimized.setNumDiscard(rots.elms());
        REPAO(optimized) = i;
        REPA(optimized)
        if (optimized.elms() >= 2) {
            Int prev2 = optimized[Index(i - 2, anim_loop, optimized.elms(), i)],
                prev = optimized[Index(i - 1, anim_loop, optimized.elms(), i)],
                next = optimized[Index(i + 1, anim_loop, optimized.elms(), i)],
                next2 = optimized[Index(i + 2, anim_loop, optimized.elms(), i)],
                stop = (InRange(i + 1, optimized) ? optimized[Min(i + (anim_linear ? 1 : 2), optimized.elms() - 1)] : rots.elms()); // stop on the next key that wasn't removed (for non-linear use 2nd next key)
            C Rot *p = &rots[prev], *n = &rots[next], *p2 = &rots[prev2], *n2 = &rots[next2];
            Flt n_time = AfterTime(n->time, p->time, anim_length);
            for (Int src = i, i_next = i + 3;;) // check that all removed keyframes (including this) do not deviate, start from the left because below we're checking if we've reached the next index, we need to go only to the right, because to the left no keys have been removed yet
            {
                C Rot &rot = rots[src];
                AxisRoll test;
#if HAS_ANIM_TANGENT
                AxisRoll tan[2];
                if (!anim_linear) {
                    tan[0].v4() = GetTangent(p2->rot.v4(), p->rot.v4(), n->rot.v4());
                    tan[1].v4() = GetTangent(p->rot.v4(), n->rot.v4(), n2->rot.v4());
                }
#endif
                // check at key location
                if (src != prev) // no need to check time where both source and optimized have a keyframe
                {
                    Flt step = LerpRS(p->time, n_time, AfterTime(rot.time, p->time, anim_length)); // use Sat in case rot.time is outside of range, or 'p' has the same time as 'n'
                    if (anim_linear)
                        test.v4() = Lerp(p->rot.v4(), n->rot.v4(), step);
                    else {
#if HAS_ANIM_TANGENT
                        test.v4() = LerpTan(p->rot.v4(), n->rot.v4(), step, tan[0].v4(), tan[1].v4());
#else
                        test.v4() = Lerp4(p2->rot.v4(), p->rot.v4(), n->rot.v4(), n2->rot.v4(), step);
#endif
                    }
                    if (MaxAbsAngleBetween(test, rot.rot) > angle_eps)
                        goto rot_needed;
                }

                // check between keys
                if (!anim_linear) // for cubic interpolation we need to check between keys, because due to cubic interpolation the values can get way off
                {
                    Flt next_time = (InRange(src + 1, rots) ? rots[src + 1].time : anim_loop ? rots[0].time + anim_length
                                                                                             : anim_length);
                    if (!Equal(rot.time, next_time)) // if times are the same then we don't need to do the following (this can happen when last key is at the end of anim and (it's not looped or the first key is at the start))
                    {
                        anim_params.time = Avg(rot.time, next_time); // set time between this and next key from the source
                        AxisRoll rot;
                        T.rot(rot, anim_params);
                        Flt step = LerpRS(p->time, n_time, AfterTime(anim_params.time, p->time, anim_length)); // use Sat in case rot.time is outside of range, or 'p' has the same time as 'n'
                        if (anim_linear)
                            test.v4() = Lerp(p->rot.v4(), n->rot.v4(), step);
                        else {
#if HAS_ANIM_TANGENT
                            test.v4() = LerpTan(p->rot.v4(), n->rot.v4(), step, tan[0].v4(), tan[1].v4());
#else
                            test.v4() = Lerp4(p2->rot.v4(), p->rot.v4(), n->rot.v4(), n2->rot.v4(), step);
#endif
                        }
                        if (MaxAbsAngleBetween(test, rot) > angle_eps)
                            goto rot_needed;
                    }
                }

                if (++src >= stop)
                    break;
                if (src == next) // remember that 'next' can be wrapped due to looping, so use == for safety
                {
                    prev2 = prev;
                    prev = next;
                    next = next2;
                    next2 = optimized[Index(i_next++, anim_loop, optimized.elms(), i)];
                    p = &rots[prev];
                    n = &rots[next];
                    p2 = &rots[prev2];
                    n2 = &rots[next2];
                    n_time = AfterTime(n->time, p->time, anim_length);
                }
            }
            optimized.remove(i, true);
        rot_needed:;
        }
        if (optimized.elms() == 1 && MaxAbsAngleBetween(rots[optimized[0]].rot, AxisRoll().zero()) <= angle_eps)
            optimized.clear();
        if (optimized.elms() != rots.elms()) {
            Mems<Rot> temp;
            temp.setNumDiscard(optimized.elms());
            REPAO(temp) = rots[optimized[i]];
            Swap(temp, rots);
        }
    }
#endif

#if HAS_ANIM_COLOR
    // color
    if (color_eps >= 0) {
        optimized.setNumDiscard(colors.elms());
        REPAO(optimized) = i;
        REPA(optimized)
        if (optimized.elms() >= 2) {
            Int prev2 = optimized[Index(i - 2, anim_loop, optimized.elms(), i)],
                prev = optimized[Index(i - 1, anim_loop, optimized.elms(), i)],
                next = optimized[Index(i + 1, anim_loop, optimized.elms(), i)],
                next2 = optimized[Index(i + 2, anim_loop, optimized.elms(), i)],
                stop = (InRange(i + 1, optimized) ? optimized[Min(i + (anim_linear ? 1 : 2), optimized.elms() - 1)] : colors.elms()); // stop on the next key that wasn't removed (for non-linear use 2nd next key)
            C Color *p = &colors[prev], *n = &colors[next], *p2 = &colors[prev2], *n2 = &colors[next2];
            Flt n_time = AfterTime(n->time, p->time, anim_length);
            for (Int src = i, i_next = i + 3;;) // check that all removed keyframes (including this) do not deviate, start from the left because below we're checking if we've reached the next index, we need to go only to the right, because to the left no keys have been removed yet
            {
                C Color &color = colors[src];
                Vec4 test;
#if HAS_ANIM_TANGENT
                Vec4 tan[2];
                if (!anim_linear) {
                    tan[0] = GetTangent(p2->color, p->color, n->color);
                    tan[1] = GetTangent(p->color, n->color, n2->color);
                }
#endif
                // check at key location
                if (src != prev) // no need to check time where both source and optimized have a keyframe
                {
                    Flt step = LerpRS(p->time, n_time, AfterTime(color.time, p->time, anim_length)); // use Sat in case color.time is outside of range, or 'p' has the same time as 'n'
                    if (anim_linear)
                        test = Lerp(p->color, n->color, step);
                    else {
#if HAS_ANIM_TANGENT
                        test = LerpTan(p->color, n->color, step, tan[0], tan[1]);
#else
                        test = Lerp4(p2->color, p->color, n->color, n2->color, step);
#endif
                    }
                    if (MaxAbsDelta(test, color.color) > color_eps)
                        goto color_needed;
                }

                // check between keys
                if (!anim_linear) // for cubic interpolation we need to check between keys, because due to cubic interpolation the values can get way off
                {
                    Flt next_time = (InRange(src + 1, colors) ? colors[src + 1].time : anim_loop ? colors[0].time + anim_length
                                                                                                 : anim_length);
                    if (!Equal(color.time, next_time)) // if times are the same then we don't need to do the following (this can happen when last key is at the end of anim and (it's not looped or the first key is at the start))
                    {
                        anim_params.time = Avg(color.time, next_time); // set time between this and next key from the source
                        Vec4 color;
                        T.color(color, anim_params);
                        Flt step = LerpRS(p->time, n_time, AfterTime(anim_params.time, p->time, anim_length)); // use Sat in case color.time is outside of range, or 'p' has the same time as 'n'
                        if (anim_linear)
                            test = Lerp(p->color, n->color, step);
                        else {
#if HAS_ANIM_TANGENT
                            test = LerpTan(p->color, n->color, step, tan[0], tan[1]);
#else
                            test = Lerp4(p2->color, p->color, n->color, n2->color, step);
#endif
                        }
                        if (MaxAbsDelta(test, color) > color_eps)
                            goto color_needed;
                    }
                }

                if (++src >= stop)
                    break;
                if (src == next) // remember that 'next' can be wrapped due to looping, so use == for safety
                {
                    prev2 = prev;
                    prev = next;
                    next = next2;
                    next2 = optimized[Index(i_next++, anim_loop, optimized.elms(), i)];
                    p = &colors[prev];
                    n = &colors[next];
                    p2 = &colors[prev2];
                    n2 = &colors[next2];
                    n_time = AfterTime(n->time, p->time, anim_length);
                }
            }
            optimized.remove(i, true);
        color_needed:;
        }
        if (optimized.elms() == 1 && MaxAbsDelta(colors[optimized[0]].color, Vec4(1, 1, 1, 1)) <= color_eps)
            optimized.clear();
        if (optimized.elms() != colors.elms()) {
            Mems<Color> temp;
            temp.setNumDiscard(optimized.elms());
            REPAO(temp) = colors[optimized[i]];
            Swap(temp, colors);
        }
    }
#endif
    PROFILE_STOP("CatzEngine::AnimKeys::optimize(Bool anim_loop, Bool anim_linear, Flt anim_length, Flt angle_eps, Flt pos_eps, Flt scale_eps, C Orient *bone, C Orient *bone_parent)")
    return T;
}
/******************************************************************************/
#define TIME_EPS (EPS)
struct TimeClip {
    Flt start_time, end_time, length, length_eps;

    TimeClip(Flt start_time, Flt end_time) {
        T.start_time = start_time;
        T.end_time = end_time;
        T.length = end_time - start_time;
        T.length_eps = length - TIME_EPS;
    }
    void adjust(Flt &time) C {
        Flt t = time - start_time;
        if (t < TIME_EPS)
            t = 0;
        else // it's important to align to start
            if (t > length_eps)
                t = length; //                        and end, because looped root motion is calculated at time=0 and time=length, so if times are slightly offsetted, we could get root motion start that is actually end
        time = t;
    }
    T1(TYPE)
    void find(Mems<TYPE> &keys, Int &before_start, Int &after_end) C {
        before_start = -1;
        FREPA(keys)
        if (keys[i].time < start_time)
            before_start = i;
        else
            break; // go from start
        after_end = -1;
        REPA(keys)
        if (keys[i].time > end_time)
            after_end = i;
        else
            break; // go from end
    }
    T1(TYPE)
    NOINLINE void clip(Mems<TYPE> &keys) C // use 'NOINLINE' to prevent from inlining because we're using Memt which uses lot of stack memory
    {
        // first add all good keys to a temporary container, because if we would be removing from 'Mems' one by one, then it would be very slow
        Memt<TYPE> clipped;
        FREPA(keys) // process in order to preserve order
        {
            TYPE &key = keys[i];
            if (key.time >= start_time && key.time <= end_time) // add only if inside the time range
            {
                adjust(key.time); // adjust time before adding
                clipped.add(key);
            }
        }
        keys = clipped; // always assign, not only if number of keys is different, because we're not only removing, but also adjusting 'time' values
    }
};
AnimKeys &AnimKeys::clip(Bool anim_loop, Bool anim_linear, Flt anim_length, Flt start_time, Flt end_time) {
    PROFILE_START("CatzEngine::AnimKeys::clip(Bool anim_loop, Bool anim_linear, Flt anim_length, Flt start_time, Flt end_time)")
    Bool reverse;
    if (reverse = (end_time < start_time))
        Swap(start_time, end_time);
    TimeClip tc(start_time, end_time);
    AnimParams params(anim_loop, anim_linear, anim_length, 0);
    Int before_start, after_end;

    // orientations
    if (orns.elms() == 1)
        tc.adjust(orns[0].time);
    else                     // if there is only 1 frame, then keep it
        if (orns.elms() > 1) // otherwise check for before start and after end
        {
            tc.find(orns, before_start, after_end);

            // check for keyframes after end
            if (after_end >= 0)                                                                      // there is a keyframe after end
                if (!InRange(after_end - 1, orns) || orns[after_end - 1].time < end_time - TIME_EPS) // previous keyframe doesn't exist or isn't at the end
                {                                                                                    // adjust 'after_end' keyframe so it's positioned at the end
                    Orn &next = orns[after_end];
                    params.time = end_time;
                    orn(next.orn, params);
                    next.time = end_time; // adjust keyframe 'time' after evaluation so the change doesn't affect it
                }

            // check for keyframes before start
            if (before_start >= 0)                                                                           // there is a keyframe before start
                if (!InRange(before_start + 1, orns) || orns[before_start + 1].time > start_time + TIME_EPS) // next keyframe doesn't exist or isn't at the start
                {                                                                                            // adjust 'before_start' keyframe so it's positioned at the start
                    Orn &prev = orns[before_start];
                    params.time = start_time;
                    orn(prev.orn, params);
                    prev.time = start_time; // adjust keyframe 'time' after evaluation so the change doesn't affect it
                }

            tc.clip(orns); // remove keyframes out of range and adjust time
        }

    // positions
    if (poss.elms() == 1)
        tc.adjust(poss[0].time);
    else                     // if there is only 1 frame, then keep it
        if (poss.elms() > 1) // otherwise check for before start and after end
        {
            tc.find(poss, before_start, after_end);

            // check for keyframes after end
            if (after_end >= 0)                                                                      // there is a keyframe after end
                if (!InRange(after_end - 1, poss) || poss[after_end - 1].time < end_time - TIME_EPS) // previous keyframe doesn't exist or isn't at the end
                {                                                                                    // adjust 'after_end' keyframe so it's positioned at the end
                    Pos &next = poss[after_end];
                    params.time = end_time;
                    pos(next.pos, params);
                    next.time = end_time; // adjust keyframe 'time' after evaluation so the change doesn't affect it
                }

            // check for keyframes before start
            if (before_start >= 0)                                                                           // there is a keyframe before start
                if (!InRange(before_start + 1, poss) || poss[before_start + 1].time > start_time + TIME_EPS) // next keyframe doesn't exist or isn't at the start
                {                                                                                            // adjust 'before_start' keyframe so it's positioned at the start
                    Pos &prev = poss[before_start];
                    params.time = start_time;
                    pos(prev.pos, params);
                    prev.time = start_time; // adjust keyframe 'time' after evaluation so the change doesn't affect it
                }

            tc.clip(poss); // remove keyframes out of range and adjust time
        }

    // scales
    if (scales.elms() == 1)
        tc.adjust(scales[0].time);
    else                       // if there is only 1 frame, then keep it
        if (scales.elms() > 1) // otherwise check for before start and after end
        {
            tc.find(scales, before_start, after_end);

            // check for keyframes after end
            if (after_end >= 0)                                                                          // there is a keyframe after end
                if (!InRange(after_end - 1, scales) || scales[after_end - 1].time < end_time - TIME_EPS) // previous keyframe doesn't exist or isn't at the end
                {                                                                                        // adjust 'after_end' keyframe so it's positioned at the end
                    Scale &next = scales[after_end];
                    params.time = end_time;
                    scale(next.scale, params);
                    next.time = end_time; // adjust keyframe 'time' after evaluation so the change doesn't affect it
                }

            // check for keyframes before start
            if (before_start >= 0)                                                                               // there is a keyframe before start
                if (!InRange(before_start + 1, scales) || scales[before_start + 1].time > start_time + TIME_EPS) // next keyframe doesn't exist or isn't at the start
                {                                                                                                // adjust 'before_start' keyframe so it's positioned at the start
                    Scale &prev = scales[before_start];
                    params.time = start_time;
                    scale(prev.scale, params);
                    prev.time = start_time; // adjust keyframe 'time' after evaluation so the change doesn't affect it
                }

            tc.clip(scales); // remove keyframes out of range and adjust time
        }

#if HAS_ANIM_ROT
    // rotations
    if (rots.elms() == 1)
        tc.adjust(rots[0].time);
    else                     // if there is only 1 frame, then keep it
        if (rots.elms() > 1) // otherwise check for before start and after end
        {
            tc.find(rots, before_start, after_end);

            // check for keyframes after end
            if (after_end >= 0)                                                                      // there is a keyframe after end
                if (!InRange(after_end - 1, rots) || rots[after_end - 1].time < end_time - TIME_EPS) // previous keyframe doesn't exist or isn't at the end
                {                                                                                    // adjust 'after_end' keyframe so it's positioned at the end
                    Rot &next = rots[after_end];
                    params.time = end_time;
                    rot(next.rot, params);
                    next.time = end_time; // adjust keyframe 'time' after evaluation so the change doesn't affect it
                }

            // check for keyframes before start
            if (before_start >= 0)                                                                           // there is a keyframe before start
                if (!InRange(before_start + 1, rots) || rots[before_start + 1].time > start_time + TIME_EPS) // next keyframe doesn't exist or isn't at the start
                {                                                                                            // adjust 'before_start' keyframe so it's positioned at the start
                    Rot &prev = rots[before_start];
                    params.time = start_time;
                    rot(prev.rot, params);
                    prev.time = start_time; // adjust keyframe 'time' after evaluation so the change doesn't affect it
                }

            tc.clip(rots); // remove keyframes out of range and adjust time
        }
#endif

#if HAS_ANIM_COLOR
    // colors
    if (colors.elms() == 1)
        tc.adjust(colors[0].time);
    else                       // if there is only 1 frame, then keep it
        if (colors.elms() > 1) // otherwise check for before start and after end
        {
            tc.find(colors, before_start, after_end);

            // check for keyframes after end
            if (after_end >= 0)                                                                          // there is a keyframe after end
                if (!InRange(after_end - 1, colors) || colors[after_end - 1].time < end_time - TIME_EPS) // previous keyframe doesn't exist or isn't at the end
                {                                                                                        // adjust 'after_end' keyframe so it's positioned at the end
                    Color &next = colors[after_end];
                    params.time = end_time;
                    color(next.color, params);
                    next.time = end_time; // adjust keyframe 'time' after evaluation so the change doesn't affect it
                }

            // check for keyframes before start
            if (before_start >= 0)                                                                               // there is a keyframe before start
                if (!InRange(before_start + 1, colors) || colors[before_start + 1].time > start_time + TIME_EPS) // next keyframe doesn't exist or isn't at the start
                {                                                                                                // adjust 'before_start' keyframe so it's positioned at the start
                    Color &prev = colors[before_start];
                    params.time = start_time;
                    color(prev.color, params);
                    prev.time = start_time; // adjust keyframe 'time' after evaluation so the change doesn't affect it
                }

            tc.clip(colors); // remove keyframes out of range and adjust time
        }
#endif

    if (reverse)
        T.reverse(tc.length);
    PROFILE_STOP("CatzEngine::AnimKeys::clip(Bool anim_loop, Bool anim_linear, Flt anim_length, Flt start_time, Flt end_time)")
    return setTangents(anim_loop, tc.length);
}
/******************************************************************************/
void AnimKeys::includeTimes(MemPtr<Flt, 16384> orn_times, MemPtr<Flt, 16384> pos_times, MemPtr<Flt, 16384> scale_times) C { // process forward because keys are sorted
    PROFILE_START("CatzEngine::AnimKeys::includeTimes(MemPtr<Flt, 16384> orn_times, MemPtr<Flt, 16384> pos_times, MemPtr<Flt, 16384> scale_times)")
    if (orn_times) {
        if (orn_times.elms())
            FREPA(orns)
        orn_times.binaryInclude(orns[i].time, CompareEps);
        else {
            orn_times.setNum(orns.elms());
            REPAO(orn_times) = orns[i].time;
        }
    }
    if (pos_times) {
        if (pos_times.elms())
            FREPA(poss)
        pos_times.binaryInclude(poss[i].time, CompareEps);
        else {
            pos_times.setNum(poss.elms());
            REPAO(pos_times) = poss[i].time;
        }
    }
    if (scale_times) {
        if (scale_times.elms())
            FREPA(scales)
        scale_times.binaryInclude(scales[i].time, CompareEps);
        else {
            scale_times.setNum(scales.elms());
            REPAO(scale_times) = scales[i].time;
        }
    }
    PROFILE_STOP("CatzEngine::AnimKeys::includeTimes(MemPtr<Flt, 16384> orn_times, MemPtr<Flt, 16384> pos_times, MemPtr<Flt, 16384> scale_times)")
}
void AnimKeys::includeTimes(MemPtr<Flt, 16384> orn_times, MemPtr<Flt, 16384> pos_times, MemPtr<Flt, 16384> scale_times, Flt start, Flt end) C {
    PROFILE_START("CatzEngine::AnimKeys::includeTimes(MemPtr<Flt, 16384> orn_times, MemPtr<Flt, 16384> pos_times, MemPtr<Flt, 16384> scale_times, Flt start, Flt end)")
    Int i; // process forward because keys are sorted
    if (orn_times) {
        orns.binarySearch(start, i, Compare);
        for (; i < orns.elms(); i++) {
            Flt time = orns[i].time;
            if (time >= end)
                break;
            orn_times.binaryInclude(time, CompareEps);
        }
    }
    if (pos_times) {
        poss.binarySearch(start, i, Compare);
        for (; i < poss.elms(); i++) {
            Flt time = poss[i].time;
            if (time >= end)
                break;
            pos_times.binaryInclude(time, CompareEps);
        }
    }
    if (scale_times) {
        scales.binarySearch(start, i, Compare);
        for (; i < scales.elms(); i++) {
            Flt time = scales[i].time;
            if (time >= end)
                break;
            scale_times.binaryInclude(time, CompareEps);
        }
    }
    PROFILE_STOP("CatzEngine::AnimKeys::includeTimes(MemPtr<Flt, 16384> orn_times, MemPtr<Flt, 16384> pos_times, MemPtr<Flt, 16384> scale_times, Flt start, Flt end)")
}
static void IncludeTimes(C CMemPtr<Flt, 16384> &src, MemPtr<Flt, 16384> dest) {
    PROFILE_START("CatzEngine::IncludeTimes(C CMemPtr<Flt, 16384> &src, MemPtr<Flt, 16384> dest)")
    FREPA(src)
    dest.binaryInclude(src[i], CompareEps); // process forward because 'src' is most likely sorted
    PROFILE_STOP("CatzEngine::IncludeTimes(C CMemPtr<Flt, 16384> &src, MemPtr<Flt, 16384> dest)")
}
/******************************************************************************/
#if HAS_ANIM_ROT
AnimKeys &AnimKeys::convertRotToOrn(C Skeleton &skeleton, Int skel_bone_index, Bool anim_loop, Flt anim_length) // this method can ignore name differences because it's used only during importing while the names are the same
{
    PROFILE_START("CatzEngine::AnimKeys::convertRotToOrn(C Skeleton &skeleton, Int skel_bone_index, Bool anim_loop, Flt anim_length)")
    if (rots.elms() && (skel_bone_index < 0 || InRange(skel_bone_index, skeleton.bones))) // <0 means keys of 'root' bone
    {
        if (skel_bone_index < 0) {
            orns.setNumDiscard(rots.elms());
            REPA(orns) {
                C Rot &rot = T.rots[i];
                Orn &orn = T.orns[i];
                orn.time = rot.time;

                Vec axis = rot.rot.axis; // rotation in parent space
                Flt angle = axis.normalize();

                orn.orn = Matrix3().setRotateZ(rot.rot.roll).rotate(axis, angle); // temp is now set to target orientation in global space
            }
        } else {
            C SkelBone &sbon = skeleton.bones[skel_bone_index],
                       *parent = skeleton.bones.addr(sbon.parent);
            Matrix3 parent_matrix, parent_matrix_inv;
            if (parent) {
                parent_matrix = *parent;
                parent_matrix.inverse(parent_matrix_inv, true);
            }

            orns.setNumDiscard(rots.elms());
            REPA(orns) {
                C Rot &rot = T.rots[i];
                Orn &orn = T.orns[i];
                orn.time = rot.time;

                Vec axis = rot.rot.axis; // rotation in parent space
                Flt angle = axis.normalize();
                if (parent)
                    axis *= parent_matrix; // rotation in global space

                Orient temp = sbon;

                temp *= Matrix3().setRotate(sbon.dir, rot.rot.roll).rotate(axis, angle); // temp is now set to target orientation in global space

                if (parent)
                    temp *= parent_matrix_inv; // convert to be in parent space

                orn.orn = temp;
            }
        }
        rots.clear();
        setTangents(anim_loop, anim_length);
    }
    PROFILE_STOP("CatzEngine::AnimKeys::convertRotToOrn(C Skeleton &skeleton, Int skel_bone_index, Bool anim_loop, Flt anim_length)")
    return T;
}
AnimKeys &AnimKeys::convertOrnToRot(C Skeleton &skeleton, Int skel_bone_index, Bool anim_loop, Flt anim_length) // this method can ignore name differences because it's used only during importing while the names are the same
{
    PROFILE_START("CatzEngine::AnimKeys::convertOrnToRot(C Skeleton &skeleton, Int skel_bone_index, Bool anim_loop, Flt anim_length)")
    if (orns.elms() && (skel_bone_index < 0 || InRange(skel_bone_index, skeleton.bones))) // <0 means keys of 'root' bone
    {
        if (skel_bone_index < 0) {
            rots.setNumDiscard(orns.elms());
            REPA(rots) {
                C Orn &orn = T.orns[i];
                Rot &rot = T.rots[i];
                rot.time = orn.time;

                Orient dest = orn.orn, // target  orientation in parent space
                    src;
                src.identity(); // current orientation in global space

                rot.rot.from(src, dest);
            }
        } else {
            C SkelBone &sbon = skeleton.bones[skel_bone_index],
                       *parent = skeleton.bones.addr(sbon.parent);
            Matrix3 parent_matrix_inv;
            if (parent)
                parent->inverse(parent_matrix_inv);

            rots.setNumDiscard(orns.elms());
            REPA(rots) {
                C Orn &orn = T.orns[i];
                Rot &rot = T.rots[i];
                rot.time = orn.time;

                Orient dest = orn.orn, // target  orientation in parent space
                    src = sbon;        // current orientation in global space
                if (parent)
                    src *= parent_matrix_inv; // current orientation in parent space

                rot.rot.from(src, dest);
            }
        }
        orns.clear();
        setTangents(anim_loop, anim_length);
    }
    PROFILE_STOP("CatzEngine::AnimKeys::convertOrnToRot(C Skeleton &skeleton, Int skel_bone_index, Bool anim_loop, Flt anim_length)")
    return T;
}
#endif
/******************************************************************************/
static void LoadOrnTan(File &f, Mems<AnimKeys::Orn> &orns) {
    PROFILE_START("CatzEngine::LoadOrnTan(File &f, Mems<AnimKeys::Orn> &orns)")
#if HAS_ANIM_TANGENT
    f.getN(orns.data(), orns.elms());
#else
    FREPA(orns) {
        AnimKeys::Orn &orn = orns[i];
        f >> orn.time >> orn.orn;
        f.skip(SIZE(Orient));
    }
#endif
    PROFILE_STOP("CatzEngine::LoadOrnTan(File &f, Mems<AnimKeys::Orn> &orns)")
}
static void LoadPosTan(File &f, Mems<AnimKeys::Pos> &poss) {
    PROFILE_START("CatzEngine::LoadPosTan(File &f, Mems<AnimKeys::Pos> &poss)")
#if HAS_ANIM_TANGENT
    f.getN(poss.data(), poss.elms());
#else
    FREPA(poss) {
        AnimKeys::Pos &pos = poss[i];
        f >> pos.time >> pos.pos;
        f.skip(SIZE(Vec));
    }
#endif
    PROFILE_STOP("CatzEngine::LoadPosTan(File &f, Mems<AnimKeys::Pos> &poss)")
}
static void LoadScaleTan(File &f, Mems<AnimKeys::Scale> &scales) {
    PROFILE_START("CatzEngine::LoadScaleTan(File &f, Mems<AnimKeys::Scale> &scales)")
#if HAS_ANIM_TANGENT
    f.getN(scales.data(), scales.elms());
#else
    FREPA(scales) {
        AnimKeys::Scale &scale = scales[i];
        f >> scale.time >> scale.scale;
        f.skip(SIZE(Vec));
    }
#endif
    PROFILE_STOP("CatzEngine::LoadScaleTan(File &f, Mems<AnimKeys::Scale> &scales)")
}

#if HAS_ANIM_ROT
static void LoadRotTan(File &f, Mems<AnimKeys::Rot> &rots) {
    PROFILE_START("CatzEngine::LoadRotTan(File &f, Mems<AnimKeys::Rot> &rots)")
#if HAS_ANIM_TANGENT
    f.getN(rots.data(), rots.elms());
#else
    FREPA(rots) {
        AnimKeys::Rot &rot = rots[i];
        f >> rot.time >> rot.rot;
        f.skip(SIZE(AxisRoll));
    }
#endif
    PROFILE_STOP("CatzEngine::LoadRotTan(File &f, Mems<AnimKeys::Rot> &rots)")
}
#else
static void LoadRotTan(File &f, Int rots) { f.skip((SIZE(Flt) + SIZE(AxisRoll) + SIZE(AxisRoll)) * rots); } // (time+rot+tan)*rots
#endif

#if HAS_ANIM_COLOR
static void LoadColorTan(File &f, Mems<AnimKeys::Color> &colors) {
    PROFILE_START("CatzEngine::LoadColorTan(File &f, Mems<AnimKeys::Color> &colors)")
#if HAS_ANIM_TANGENT
    f.getN(colors.data(), colors.elms());
#else
    FREPA(colors) {
        AnimKeys::Color &color = colors[i];
        f >> color.time >> color.color;
        f.skip(SIZE(Vec4));
    }
#endif
    PROFILE_STOP("CatzEngine::LoadColorTan(File &f, Mems<AnimKeys::Color> &colors)")
}
#else
static void LoadColorTan(File &f, Int colors) { f.skip((SIZE(Flt) + SIZE(Vec4) + SIZE(Vec4)) * colors); } // (time+color+tan)*color
#endif
/******************************************************************************/
Bool AnimKeys::saveData(File &f) C {
    PROFILE_START("CatzEngine::AnimKeys::saveData(File &f)")
    ASSERT(HAS_ANIM_TANGENT == 0);
    if (orns.saveRaw(f))
        if (poss.saveRaw(f))
            if (scales.saveRaw(f)) {
                PROFILE_STOP("CatzEngine::AnimKeys::saveData(File &f)")
                return f.ok();
            }
    PROFILE_STOP("CatzEngine::AnimKeys::saveData(File &f)")
    return false;
}
Bool AnimKeys::loadData(File &f) {
    PROFILE_START("CatzEngine::AnimKeys::loadData(File &f)")
    ASSERT(HAS_ANIM_TANGENT == 0);
    if (orns.loadRaw(f))
        if (poss.loadRaw(f))
            if (scales.loadRaw(f))
                if (f.ok()) {
                    PROFILE_STOP("CatzEngine::AnimKeys::loadData(File &f)")
                    return true;
                }
    del();
    PROFILE_STOP("CatzEngine::AnimKeys::loadData(File &f)")
    return false;
}
void AnimKeys::loadData3(File &f) {
    PROFILE_START("CatzEngine::AnimKeys::loadData3(File &f)")
    orns.setNumDiscard(f.decUIntV());
    LoadOrnTan(f, orns);
#if HAS_ANIM_ROT
    rots.setNumDiscard(f.decUIntV());
    LoadRotTan(f, rots);
#else
    LoadRotTan(f, f.decUIntV());
#endif
    poss.setNumDiscard(f.decUIntV());
    LoadPosTan(f, poss);
    scales.setNumDiscard(f.decUIntV());
    LoadScaleTan(f, scales);
#if HAS_ANIM_COLOR
    colors.setNumDiscard(f.decUIntV());
    LoadColorTan(f, colors);
#else
    LoadColorTan(f, f.decUIntV());
#endif
    PROFILE_STOP("CatzEngine::AnimKeys::loadData3(File &f)")
}
void AnimKeys::loadData2(File &f) {
    PROFILE_START("CatzEngine::AnimKeys::loadData2(File &f)")
    orns.setNumDiscard(f.getUShort());
#if HAS_ANIM_ROT
    rots.setNumDiscard(f.getUShort());
#else
    Int rots = f.getUShort();
#endif
    poss.setNumDiscard(f.getUShort());
    scales.setNumDiscard(f.getUShort());
#if HAS_ANIM_COLOR
    colors.setNumDiscard(f.getUShort());
#else
    Int colors = f.getUShort();
#endif

    LoadOrnTan(f, orns);
    LoadRotTan(f, rots);
    LoadPosTan(f, poss);
    LoadScaleTan(f, scales);
    LoadColorTan(f, colors);
    PROFILE_STOP("CatzEngine::AnimKeys::loadData2(File &f)")
}
void AnimKeys::loadData1(File &f) {
    PROFILE_START("CatzEngine::AnimKeys::loadData1(File &f)")
    orns.setNumDiscard(f.getUShort());
#if HAS_ANIM_ROT
    rots.setNumDiscard(f.getUShort());
#else
    Int rots = f.getUShort();
#endif
    poss.setNumDiscard(f.getUShort());
    scales.setNumDiscard(f.getUShort());

    LoadOrnTan(f, orns);
    LoadRotTan(f, rots);
    LoadPosTan(f, poss);
    FREPA(scales) {
        scales[i].time = f.getFlt();
        scales[i].scale = f.getFlt();
#if HAS_ANIM_TANGENT
        scales[i].tan = f.getFlt();
#else
        f.skip(SIZE(Flt));
#endif
    }
    PROFILE_STOP("CatzEngine::AnimKeys::loadData1(File &f)")
}
void AnimKeys::loadData0(File &f) {
    PROFILE_START("CatzEngine::AnimKeys::loadData0(File &f)")
    orns.setNumDiscard(f.getUShort());
#if HAS_ANIM_ROT
    rots.setNumDiscard(f.getUShort());
#else
    Int rots = f.getUShort();
#endif
    poss.setNumDiscard(f.getUShort());

    LoadOrnTan(f, orns);
    LoadRotTan(f, rots);
    LoadPosTan(f, poss);
    PROFILE_STOP("CatzEngine::AnimKeys::loadData0(File &f)")
}
void AnimKeys::save(MemPtr<TextNode> nodes) C {
    PROFILE_START("CatzEngine::AnimKeys::save(MemPtr<TextNode> nodes)")
    if (orns.elms()) {
        TextNode &node = nodes.New();
        node.name = "Orientations";
        FREPAO(orns).save(node.nodes.New().nodes);
    }
    if (poss.elms()) {
        TextNode &node = nodes.New();
        node.name = "Positions";
        FREPAO(poss).save(node.nodes.New().nodes);
    }
    if (scales.elms()) {
        TextNode &node = nodes.New();
        node.name = "Scales";
        FREPAO(scales).save(node.nodes.New().nodes);
    }
#if HAS_ANIM_ROT
    if (rots.elms()) {
        TextNode &node = nodes.New();
        node.name = "Rotations";
        FREPAO(rots).save(node.nodes.New().nodes);
    }
#endif
#if HAS_ANIM_COLOR
    if (colors.elms()) {
        TextNode &node = nodes.New();
        node.name = "Colors";
        FREPAO(colors).save(node.nodes.New().nodes);
    }
#endif
    PROFILE_STOP("CatzEngine::AnimKeys::save(MemPtr<TextNode> nodes)")
}
/******************************************************************************/
// ANIMATION BONE
/******************************************************************************/
void AnimBone::save(TextNode &node) C {
    PROFILE_START("CatzEngine::AnimBone::save(TextNode &node)")
    node.set(name);
    super::save(node.nodes);
    PROFILE_STOP("CatzEngine::AnimBone::save(TextNode &node)")
}
/******************************************************************************/
// ANIMATION EVENT
/******************************************************************************/
static Int Compare(C AnimEvent &a, C AnimEvent &b) { return Compare(a.time, b.time); }
static Int Compare(C AnimEvent &a, C Flt &b_time) { return Compare(a.time, b_time); }

Bool AnimEvent::operator==(C AnimEvent &event) C {
    return time == event.time && Equal(name, event.name);
}
void AnimEvent::save(TextNode &node) C {
    node.set(name);
    node.nodes.New().set("Time", time);
}
/******************************************************************************/
// ANIMATION
/******************************************************************************/
void Animation::zero() {
    _flag = 0;
    _length = 0;
    _root_start.identity();
    _root_start_inv.identity();
    _root_end.identity();
    _root_transform.identity();
}
Animation::Animation() {
    zero();
}
Animation &Animation::del() {
    bones.del();
    events.del();
    keys.del();
    zero();
    return T;
}
/******************************************************************************/
AnimEvent *Animation::findEvent(CChar8 *name) {
    PROFILE_START("CatzEngine::Animation::findEvent(CChar8 *name)")
    REPA(events) {
        AnimEvent &event = events[i];
        if (Equal(event.name, name)) {
            PROFILE_STOP("CatzEngine::Animation::findEvent(CChar8 *name)")
            return &event;
        }
    }
    PROFILE_STOP("CatzEngine::Animation::findEvent(CChar8 *name)")
    return null;
}
Int Animation::eventCount(CChar8 *name) C {
    PROFILE_START("CatzEngine::Animation::eventCount(CChar8 *name)")
    Int num = 0;
    REPA(events)
    if (Equal(events[i].name, name))
        num++;
    PROFILE_STOP("CatzEngine::Animation::eventCount(CChar8 *name)")
    return num;
}
Bool Animation::eventAfter(CChar8 *name, Flt time) C // event.name==name && event.time<=time (time>=event.time)
{
    PROFILE_START("CatzEngine::Animation::eventAfter(CChar8 *name, Flt time)")
    FREPA(events) // check all events from the start until 'time'
    {
        C AnimEvent &event = events[i];
        if (event.time > time)
            break;
        if (Equal(event.name, name)) {
            PROFILE_STOP("CatzEngine::Animation::eventAfter(CChar8 *name, Flt time)")
            return true;
        }
    }
    PROFILE_STOP("CatzEngine::Animation::eventAfter(CChar8 *name, Flt time)")
    return false;
}
Bool Animation::eventBefore(CChar8 *name, Flt time) C // event.name==name && event.time>time (time<event.time)
{
    PROFILE_START("CatzEngine::Animation::eventBefore(CChar8 *name, Flt time)")
    REPA(events) // check all events from the end until 'time'
    {
        C AnimEvent &event = events[i];
        if (event.time <= time)
            break;
        if (Equal(event.name, name)) {
            PROFILE_STOP("CatzEngine::Animation::eventBefore(CChar8 *name, Flt time)")
            return true;
        }
    }
    PROFILE_STOP("CatzEngine::Animation::eventBefore(CChar8 *name, Flt time)")
    return false;
}
Bool Animation::eventOccurred(CChar8 *name, Flt start_time, Flt dt) C {
    PROFILE_START("CatzEngine::Animation::eventOccurred(CChar8 *name, Flt start_time, Flt dt)")
    if (loop()) {
        start_time = Frac(start_time, _length);
        Flt end_time = start_time + dt;
        Int start_event;
        events.binarySearchFirst(start_time, start_event, Compare);
        for (Int i = start_event; i < events.elms(); i++) {
            C AnimEvent &event = events[i];
            if (event.time >= end_time)
                goto end; // EventOccurred(event_time, start_time, dt) = (start_time<=event_time && end_time>event_time)
            if (Equal(event.name, name)) {
                PROFILE_STOP("CatzEngine::Animation::eventOccurred(CChar8 *name, Flt start_time, Flt dt)")
                return true;
            }
        }
        for (Int i = 0; i < start_event; i++) // check events before 'start_time' but treating them as if they're after animation
        {
            C AnimEvent &event = events[i];
            Flt event_time = event.time + _length;
            if (event_time >= end_time)
                goto end; // EventOccurred(event_time, start_time, dt) = (start_time<=event_time && end_time>event_time)
            if (Equal(event.name, name)) {
                PROFILE_STOP("CatzEngine::Animation::eventOccurred(CChar8 *name, Flt start_time, Flt dt)")
                return true;
            }
        }
    } else {
        Flt end_time = start_time + dt;
        Int start_event;
        events.binarySearchFirst(start_time, start_event, Compare);
        for (; start_event < events.elms(); start_event++) {
            C AnimEvent &event = events[start_event];
            if (event.time >= end_time)
                goto end; // EventOccurred(event_time, start_time, dt) = (start_time<=event_time && end_time>event_time)
            if (Equal(event.name, name)) {
                PROFILE_STOP("CatzEngine::Animation::eventOccurred(CChar8 *name, Flt start_time, Flt dt)")
                return true;
            }
        }
    }
end:
    PROFILE_STOP("CatzEngine::Animation::eventOccurred(CChar8 *name, Flt start_time, Flt dt)")
    return false;
}
Int Animation::eventsOccurred(Int &first_event, Flt start_time, Flt dt) C {
    PROFILE_START("CatzEngine::Animation::eventsOccurred(Int &first_event, Flt start_time, Flt dt)")
    Int num = 0;
    if (loop()) {
        start_time = Frac(start_time, _length);
        Flt end_time = start_time + dt;
        Int start_event;
        events.binarySearchFirst(start_time, start_event, Compare);
        first_event = start_event;
        for (Int i = start_event; i < events.elms(); i++) {
            C AnimEvent &event = events[i];
            if (event.time >= end_time)
                goto end; // EventOccurred(event_time, start_time, dt) = (start_time<=event_time && end_time>event_time)
            num++;
        }
        for (Int i = 0; i < start_event; i++) // check events before 'start_time' but treating them as if they're after animation
        {
            C AnimEvent &event = events[i];
            Flt event_time = event.time + _length;
            if (event_time >= end_time)
                goto end; // EventOccurred(event_time, start_time, dt) = (start_time<=event_time && end_time>event_time)
            num++;
        }
    } else {
        Flt end_time = start_time + dt;
        Int start_event;
        events.binarySearchFirst(start_time, start_event, Compare);
        first_event = start_event;
        for (; start_event < events.elms(); start_event++) {
            C AnimEvent &event = events[start_event];
            if (event.time >= end_time)
                goto end; // EventOccurred(event_time, start_time, dt) = (start_time<=event_time && end_time>event_time)
            num++;
        }
    }
end:
    PROFILE_STOP("CatzEngine::Animation::eventsOccurred(Int &first_event, Flt start_time, Flt dt)")
    return num;
}
// FIXME: TODO: events are now sorted by 'time', functions below can be optimized to make use of that
Bool Animation::eventBetween(CChar8 *from, CChar8 *to, Flt start_time, Flt dt) C {
    PROFILE_START("CatzEngine::Animation::eventBetween(CChar8 *from, CChar8 *to, Flt start_time, Flt dt)")
    C AnimEvent *event_from = null,
                *event_to = null;
    REPA(events) {
        C AnimEvent &event = events[i];
        if (!event_from && Equal(from, event.name)) {
            event_from = &event;
            if (event_to)
                goto found;
        }
        if (!event_to && Equal(to, event.name)) {
            event_to = &event;
            if (event_from)
                goto found;
        }
    }
    PROFILE_STOP("CatzEngine::Animation::eventBetween(CChar8 *from, CChar8 *to, Flt start_time, Flt dt)")
    return false;
found:
    Flt time_to = event_to->time;
    if (loop()) {
        start_time = Frac(start_time, _length);
        if (start_time < event_from->time)
            start_time += _length; // make sure that 'start_time' is after 'event_from->time'
        if (time_to < event_from->time)
            time_to += _length; // make sure that 'time_to'    is after 'event_from->time'
    }
    PROFILE_STOP("CatzEngine::Animation::eventBetween(CChar8 *from, CChar8 *to, Flt start_time, Flt dt)")
    return EventBetween(event_from->time, time_to, start_time, dt);
}
Flt Animation::eventProgress(CChar8 *from, CChar8 *to, Flt time) C {
    PROFILE_START("CatzEngine::Animation::eventProgress(CChar8 *from, CChar8 *to, Flt time)")
    C AnimEvent *event_from = null,
                *event_to = null;
    REPA(events) {
        C AnimEvent &event = events[i];
        if (!event_from && Equal(from, event.name)) {
            event_from = &event;
            if (event_to)
                goto found;
        }
        if (!event_to && Equal(to, event.name)) {
            event_to = &event;
            if (event_from)
                goto found;
        }
    }
    PROFILE_STOP("CatzEngine::Animation::eventProgress(CChar8 *from, CChar8 *to, Flt time)")
    return 0;
found:
    Flt time_to = event_to->time;
    if (loop()) {
        time = Frac(time, _length);
        if (time_to < event_from->time)
            time_to += _length; // make sure that 'time_to' is after 'event_from->time'
    }
    PROFILE_STOP("CatzEngine::Animation::eventProgress(CChar8 *from, CChar8 *to, Flt time)")
    return LerpR(event_from->time, time_to, time);
}
/******************************************************************************/
AnimBone *Animation::findBone(CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub) { return bones.addr(findBoneI(name, type, type_index, type_sub)); }
AnimBone &Animation::getBone(CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub) {
    PROFILE_START("CatzEngine::Animation::getBone(CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub)")
    if (AnimBone *bone = findBone(name, type, type_index, type_sub)) {
        PROFILE_STOP("CatzEngine::Animation::getBone(CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub)")
        return *bone;
    }
    AnimBone &bone = bones.New();
    bone.set(name, type, type_index, type_sub);
    PROFILE_STOP("CatzEngine::Animation::getBone(CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub)")
    return bone;
}
/******************************************************************************/
Animation &Animation::loop(Bool loop) {
    PROFILE_START("CatzEngine::Animation::loop(Bool loop)")
    if (T.loop() != loop) {
        _flag ^= ANIM_LOOP;
        setTangents().setRootMatrix();
    }
    PROFILE_STOP("CatzEngine::Animation::loop(Bool loop)")
    return T;
}
Animation &Animation::linear(Bool linear) {
    PROFILE_START("CatzEngine::Animation::linear(Bool linear)")
    if (T.linear() != linear) {
        FlagToggle(_flag, ANIM_LINEAR);
        setRootMatrix();
    }
    PROFILE_STOP("CatzEngine::Animation::linear(Bool linear)")
    return T;
}
/******************************************************************************/
Animation &Animation::length(Flt length, Bool rescale_keyframes) {
    PROFILE_START("CatzEngine::Animation::length(Flt length, Bool rescale_keyframes)")
    if (length < 0) {
        reverse();
        CHS(length);
    }
    if (T._length != length) {
        if (rescale_keyframes) {
            Flt scale = ((T._length > EPS) ? length / T._length : 0);
            TimeScaleClip tsc(scale, length);
            keys.scaleTime(scale, length);
            REPAO(bones).scaleTime(scale, length);
            REPA(events)
            tsc.adjust(events[i].time);
        } else if (length < T.length())
            clip(0, length); // if making the length shorter, then clip and remove keyframes out of range
        T._length = length;
    }
    PROFILE_STOP("CatzEngine::Animation::length(Flt length, Bool rescale_keyframes)")
    return T;
}
/******************************************************************************/
void Animation::setRootMatrix2() {
    PROFILE_START("CatzEngine::Animation::setRootMatrix2()")
    _root_start.inverse(_root_start_inv);
    _root_end.mul(_root_start_inv, _root_transform); // this is "GetTransform(_root_transform, _root_start, _root_end)"
    PROFILE_STOP("CatzEngine::Animation::setRootMatrix2()")
}
Animation &Animation::setRootMatrix() {
    PROFILE_START("CatzEngine::Animation::setRootMatrix()")
    getRootMatrixExactTime(_root_end, length());
    getRootMatrix(_root_start, 0);
    setRootMatrix2();
    PROFILE_STOP("CatzEngine::Animation::setRootMatrix()")
    return T;
}
void Animation::getRootMatrix(Matrix &matrix, Flt time) C {
    PROFILE_START("CatzEngine::Animation::getRootMatrix(Matrix &matrix, Flt time)")
    AnimParams params(T, time);
    keys.matrix(matrix, params);
    PROFILE_STOP("CatzEngine::Animation::getRootMatrix(Matrix &matrix, Flt time)")
}
void Animation::getRootMatrixCumulative(Matrix &matrix, Flt time) C {
    PROFILE_START("CatzEngine::Animation::getRootMatrixCumulative(Matrix &matrix, Flt time)")
    AnimParams params(T, time);
    keys.matrix(matrix, params);
    if (params.loop)
        if (Int rounds = Round((time - params.time) / params.length)) {
            Matrix root_round; // this is root matrix at the start of the round
#if 0
      rootStart().mulTimes(rounds  , rootTransform(), root_round);
#else                                                                    // prefer this version, because most likely we're playing animations forward, and this way we save 1 matrix multiplication
            rootEnd().mulTimes(rounds - 1, rootTransform(), root_round); // use "rounds-1" because we start from 'rootEnd' which is end of the loop (1 round already)
#endif
            RevMatrix rm;
            matrix.mul(_root_start_inv, rm); // this is "GetTransform(rm, rootStart(), matrix)", this is OK for negative times too, because even though we're transforming from 'rootStart' to 'matrix', we start with 'root_round' which already was moved before 'time' - at the start of that round.
            root_round.mul(rm, matrix);      // result = matrix at the start of a round, transformed by (transform from 'rootStart' to matrix at 'params.time')
        }
    PROFILE_STOP("CatzEngine::Animation::getRootMatrixCumulative(Matrix &matrix, Flt time)")
}
void Animation::getRootMatrixExactTime(Matrix &matrix, Flt time) C {
    PROFILE_START("CatzEngine::Animation::getRootMatrixExactTime(Matrix &matrix, Flt time)")
    AnimParams params(T, time);
    params.time = time; // re-apply time to remove possible fraction
    keys.matrix(matrix, params);
    PROFILE_STOP("CatzEngine::Animation::getRootMatrixExactTime(Matrix &matrix, Flt time)")
}
void Animation::getRootTransform(RevMatrix &transform, Flt start_time, Flt end_time) C {
    PROFILE_START("CatzEngine::Animation::getRootTransform(RevMatrix &transform, Flt start_time, Flt end_time)")
    AnimParams params;
    RevMatrix rm;
    Matrix m;
    if (!keys.scales.elms()) // normalized
    {
        if (!keys.orns.elms()) // no orientation
        {
            if (!keys.poss.elms())
                transform.identity();
            else // nothing
            {    // position only
                params.set(T, start_time);
                keys.pos(transform.pos, params);
                transform.pos.chs(); // pos=-start
                params.time += end_time - start_time;
                if (params.loop) {
                    if (params.time >= params.length) {
                        transform.pos += rootTransform().pos;
                        params.time -= params.length;
                        if (params.time >= params.length) // if still greater than length, then it means we made more than 1 round (this is unlikely to happen, only when we play animations extremely fast)
                        {
                            Int rounds = Trunc(params.time / params.length);
                            params.time -= rounds * params.length;
                            transform.pos += rounds * rootTransform().pos;
                        }
                    } else if (params.time < 0) // going back
                    {
                        transform.pos -= rootTransform().pos;
                        params.time += params.length;
                        if (params.time < 0) // if still smaller than 0, then it means we made more than 1 round (this is unlikely to happen, only when we play animations extremely fast)
                        {
                            Int rounds = Floor(params.time / params.length);
                            params.time -= rounds * params.length;
                            transform.pos += rounds * rootTransform().pos;
                        }
                    }
                }
                Vec end;
                keys.pos(end, params);
                transform.pos += end; // pos+=end
                transform.orn().identity();
            }
        } else // orientation with optional position
        {
            params.set(T, start_time);
            keys.matrixNoScale(m, params);
            params.time += end_time - start_time;
            if (params.loop) {
                if (params.time >= params.length) {
                    GetTransformNormalized(transform, m, rootEnd());
                    params.time -= params.length;
                    if (params.time >= params.length) // if still greater than length, then it means we made more than 1 round (this is unlikely to happen, only when we play animations extremely fast)
                    {
                        Int rounds = Trunc(params.time / params.length);
                        params.time -= rounds * params.length;
                        transform.mulTimes(rounds, rootTransform());
                    }
                    keys.matrixNoScale(m, params);
                    m.mul(_root_start_inv, rm); // this is "GetTransformNormalized(rm, rootStart(), m)"
                    transform *= rm;
                    PROFILE_STOP("CatzEngine::Animation::getRootTransform(RevMatrix &transform, Flt start_time, Flt end_time)")
                    return;
                } else if (params.time < 0) // going back
                {
                    GetTransformNormalized(transform, m, rootStart());
                    params.time += params.length;
                    if (params.time < 0) // if still smaller than 0, then it means we made more than 1 round (this is unlikely to happen, only when we play animations extremely fast)
                    {
                        Int rounds = Floor(params.time / params.length);
                        params.time -= rounds * params.length;
                        transform.mulTimes(rounds, rootTransform());
                    }
                    keys.matrixNoScale(m, params);
                    GetTransformNormalized(rm, rootEnd(), m);
                    transform *= rm;
                    PROFILE_STOP("CatzEngine::Animation::getRootTransform(RevMatrix &transform, Flt start_time, Flt end_time)")
                    return;
                }
            }
            Matrix end;
            keys.matrixNoScale(end, params);
            GetTransformNormalized(transform, m, end);
        }
    } else // scaled with optional orientation+position
    {
        params.set(T, start_time);
        keys.matrix(m, params);
        params.time += end_time - start_time;
        if (params.loop) {
            if (params.time >= params.length) {
                GetTransform(transform, m, rootEnd());
                params.time -= params.length;
                if (params.time >= params.length) // if still greater than length, then it means we made more than 1 round (this is unlikely to happen, only when we play animations extremely fast)
                {
                    Int rounds = Trunc(params.time / params.length);
                    params.time -= rounds * params.length;
                    transform.mulTimes(rounds, rootTransform());
                }
                keys.matrix(m, params);
                m.mul(_root_start_inv, rm); // this is "GetTransform(rm, rootStart(), m)"
                transform *= rm;
                PROFILE_STOP("CatzEngine::Animation::getRootTransform(RevMatrix &transform, Flt start_time, Flt end_time)")
                return;
            } else if (params.time < 0) // going back
            {
                GetTransform(transform, m, rootStart());
                params.time += params.length;
                if (params.time < 0) // if still smaller than 0, then it means we made more than 1 round (this is unlikely to happen, only when we play animations extremely fast)
                {
                    Int rounds = Floor(params.time / params.length);
                    params.time -= rounds * params.length;
                    transform.mulTimes(rounds, rootTransform());
                }
                keys.matrix(m, params);
                GetTransform(rm, rootEnd(), m);
                transform *= rm;
                PROFILE_STOP("CatzEngine::Animation::getRootTransform(RevMatrix &transform, Flt start_time, Flt end_time)")
                return;
            }
        }
        Matrix end;
        keys.matrix(end, params);
        GetTransform(transform, m, end);
    }
    PROFILE_STOP("CatzEngine::Animation::getRootTransform(RevMatrix &transform, Flt start_time, Flt end_time)")
}
/******************************************************************************/
Animation &Animation::removeUnused() {
    PROFILE_START("CatzEngine::Animation::removeUnused()")
    REPA(bones)
    if (!bones[i].is())
        bones.remove(i);
    PROFILE_STOP("CatzEngine::Animation::removeUnused()")
    return T;
}
/******************************************************************************
Animation& removeClones(); // remove bone animations which share the same name, keeping only one of them
Animation& Animation::removeClones()
{
   REPA(bones)
   REPD(j,  i)if(Equal(bones[i].name, bones[j].name))
   {
      bones.remove(i);
      break;
   }
   return T;
}
/******************************************************************************/
static Bool SameBone(C SkelBone *a, C SkelBone *b, C CMemPtr<Mems<IndexWeight>> &new_to_old_weights = null, Int old_i = -1, Int new_i = -1) {
    PROFILE_START("CatzEngine::SameBone(C SkelBone *a, C SkelBone *b, C CMemPtr<Mems<IndexWeight>> &new_to_old_weights = null, Int old_i = -1, Int new_i = -1)")
    if (!a && !b) {
        PROFILE_STOP("CatzEngine::SameBone(C SkelBone *a, C SkelBone *b, C CMemPtr<Mems<IndexWeight>> &new_to_old_weights = null, Int old_i = -1, Int new_i = -1)")
        return true; // both are null
    }
    if (a && b) // both are valid
    {
        if (new_to_old_weights) // if have weights
        {
            if (InRange(new_i, new_to_old_weights)) {
                C Mems<IndexWeight> &weights = new_to_old_weights[new_i];
                PROFILE_STOP("CatzEngine::SameBone(C SkelBone *a, C SkelBone *b, C CMemPtr<Mems<IndexWeight>> &new_to_old_weights = null, Int old_i = -1, Int new_i = -1)")
                return weights.elms() == 1 && weights[0].index == old_i;
            }
        } else {
            PROFILE_STOP("CatzEngine::SameBone(C SkelBone *a, C SkelBone *b, C CMemPtr<Mems<IndexWeight>> &new_to_old_weights = null, Int old_i = -1, Int new_i = -1)")
            return Equal(a->name, b->name); // check by name only
        }
    }
    PROFILE_STOP("CatzEngine::SameBone(C SkelBone *a, C SkelBone *b, C CMemPtr<Mems<IndexWeight>> &new_to_old_weights = null, Int old_i = -1, Int new_i = -1)")
    return false;
}
static Bool SameSet(C SkelBone &a_child, C SkelBone &b_child, C SkelBone *a_parent, C SkelBone *b_parent, C CMemPtr<Mems<IndexWeight>> &new_to_old_weights, Int old_parent_i, Int new_parent_i) // this assumes that 'a_child' and 'b_child' are the same "SameBone(a_child, b_child)", and checks if their parents are the same, and children positions relative to their parents are also the same
{
    PROFILE_START("CatzEngine::SameSet(C SkelBone &a_child, C SkelBone &b_child, C SkelBone *a_parent, C SkelBone *b_parent, C CMemPtr<Mems<IndexWeight>> &new_to_old_weights, Int old_parent_i, Int new_parent_i)")
    if (SameBone(a_parent, b_parent, new_to_old_weights, old_parent_i, new_parent_i)) {
        PROFILE_STOP("CatzEngine::SameSet(C SkelBone &a_child, C SkelBone &b_child, C SkelBone *a_parent, C SkelBone *b_parent, C CMemPtr<Mems<IndexWeight>> &new_to_old_weights, Int old_parent_i, Int new_parent_i)")
        return a_parent ? Equal(a_child.pos - a_parent->pos, b_child.pos - b_parent->pos)
                        : Equal(a_child.pos, b_child.pos);
    }
    PROFILE_STOP("CatzEngine::SameSet(C SkelBone &a_child, C SkelBone &b_child, C SkelBone *a_parent, C SkelBone *b_parent, C CMemPtr<Mems<IndexWeight>> &new_to_old_weights, Int old_parent_i, Int new_parent_i)")
    return false;
}
Animation &Animation::adjustForSameSkeletonWithDifferentPose(C Skeleton &source, C Skeleton &target) // 'source' and 'target' are assumed to have the same names, this animation bones are also assumed to have the same names because this is typically called during importing
{
    PROFILE_START("CatzEngine::Animation::adjustForSameSkeletonWithDifferentPose(C Skeleton &source, C Skeleton &target)")
    if (&source != &target) {
        // adjust per-bone orientations (needed because some skeletons may have bones rotated already in the skeleton, not only in keyframes)
        // this will only insert 1 orientation keyframe if skeleton bone orientations are different, it doesn't modify existing keyframes
        REPA(target.bones) // for each bone in target skeleton
        {
            C SkelBone &bone_target = target.bones[i];
            Int bone_source_i = source.findBoneI(bone_target.name); // find the same bone in source skeleton
            if (bone_source_i >= 0) {
                C SkelBone &bone_source = source.bones[bone_source_i];
                AnimBone *abon = findBone(bone_target.name);
                if (!abon || !abon->orns.elms()) // if doesn't have any target orientation keyframes
                {
                    C SkelBone *old_parent = source.bones.addr(bone_source.parent),
                               *new_parent = target.bones.addr(bone_target.parent);
                    if (SameBone(old_parent, new_parent)) {
                        OrientD orient_source = bone_source;
                        if (old_parent)
                            orient_source.div(*old_parent, true); // convert to parent space
                        OrientD orient_target = bone_target;
                        if (new_parent)
                            orient_target.div(*new_parent, true); // convert to parent space
                        if (Dot(orient_source.dir, orient_target.dir) < EPS_COS || Dot(orient_source.perp, orient_target.perp) < EPS_COS) {
                            AnimKeys::Orn &orn = getBone(bone_target.name, bone_target.type, bone_target.type_index, bone_target.type_sub).orns.New();
                            orn.time = 0;
                            orn.orn = orient_source; // set the main target orientation keyframe
#if HAS_ANIM_TANGENT
                            orn.tan.zero();
#endif
                        }
                    }
                }
            }
        }

        // adjust per-bone position offsets (needed because some skeletons may have bones repositioned already in the skeleton, not only in keyframes)
        // this will offset existing position keyframes if skeleton bone positions are different (if there are no keyfames then 1 is added)
        REPA(target.bones) // for each bone in target skeleton
        {
            C SkelBone &bone_target = target.bones[i];
            Int bone_source_i = source.findBoneI(bone_target.name); // find the same bone in source skeleton
            if (bone_source_i >= 0) {
                C SkelBone &bone_source = source.bones[bone_source_i],
                           *old_parent = source.bones.addr(bone_source.parent),
                           *new_parent = target.bones.addr(bone_target.parent);
                if (SameBone(old_parent, new_parent)) {
                    VecD pos_target = bone_target.pos;
                    if (new_parent)
                        pos_target.divNormalized(MatrixD(*new_parent));
                    VecD pos_source = bone_source.pos;
                    if (old_parent)
                        pos_source.divNormalized(MatrixD(*old_parent));
                    Vec delta = pos_source - pos_target;
                    if (delta.length2() > Sqr(EPS_ANIM_POS)) // if calculated delta is significant, then apply it to the animation bone
                    {
                        AnimBone &abon = getBone(bone_target.name, bone_target.type, bone_target.type_index, bone_target.type_sub);
                        REPA(abon.poss)
                        abon.poss[i].pos += delta; // apply delta
                        if (!abon.poss.elms())     // if no position keyframes exist, then create one
                        {
                            AnimKeys::Pos &pos = abon.poss.New();
                            pos.time = 0;
                            pos.pos = delta;
#if HAS_ANIM_TANGENT
                            pos.tan.zero();
#endif
                        }
                    }
                }
            }
        }

        // setTangents  (); currently not needed since tangents   don't change  in this method
        // setRootMatrix(); currently not needed since root keys aren't changed in this method
    }
    PROFILE_STOP("CatzEngine::Animation::adjustForSameSkeletonWithDifferentPose(C Skeleton &source, C Skeleton &target)")
    return T;
}
struct BoneWeight : IndexWeight {
    C SkelBone *bone;
    C AnimSkelBone *asbone;
};
static Vec BoneWeightPos(C MemtN<BoneWeight, 4> &weights) {
    PROFILE_START("CatzEngine::BoneWeightPos(C MemtN<BoneWeight, 4> &weights)")
    if (weights.elms() == 1) {
        PROFILE_STOP("CatzEngine::BoneWeightPos(C MemtN<BoneWeight, 4> &weights)")
        return weights[0].asbone->matrix().pos;
    }
    Vec pos = 0;
    REPA(weights) {
        C BoneWeight &weight = weights[i];
        pos += weight.asbone->matrix().pos * weight.weight;
    }
    PROFILE_STOP("CatzEngine::BoneWeightPos(C MemtN<BoneWeight, 4> &weights)")
    return pos;
}
static Matrix3 BoneWeightOrn(C MemtN<BoneWeight, 4> &weights) {
    PROFILE_START("CatzEngine::BoneWeightOrn(C MemtN<BoneWeight, 4> &weights)")
    if (weights.elms() == 1) {
        PROFILE_STOP("CatzEngine::BoneWeightOrn(C MemtN<BoneWeight, 4> &weights)")
        return weights[0].asbone->matrix().orn();
    }
    Vec scale = 0;
    Matrix3 matrix;
    matrix.zero();
    REPA(weights) {
        C BoneWeight &weight = weights[i];
        Flt w = weight.weight;
        Matrix3 m = weight.asbone->matrix().orn();
        Flt s = m.x.normalize();
        scale.x += s * w;
        s = m.y.normalize();
        scale.y += s * w;
        s = m.z.normalize();
        scale.z += s * w;
        m *= w;
        matrix += m;
    }
    matrix.x.setLength(scale.x);
    matrix.y.setLength(scale.y);
    matrix.z.setLength(scale.z);
    PROFILE_STOP("CatzEngine::BoneWeightOrn(C MemtN<BoneWeight, 4> &weights)")
    return matrix;
}
static void DelRot(Orient &orn, UInt flag) {
    PROFILE_START("CatzEngine::DelRot(Orient &orn, UInt flag)")
    if (flag & ROOT_DEL_ROTATION) {
        if (FlagAll(flag, ROOT_DEL_ROTATION))
            orn.identity();
        else {
            MatrixD3 m = orn;
            VecD axis = m.axisAngle(true);
            if (flag & ROOT_DEL_ROTATION_X)
                axis.x = 0;
            if (flag & ROOT_DEL_ROTATION_Y)
                axis.y = 0;
            if (flag & ROOT_DEL_ROTATION_Z)
                axis.z = 0;
            if (Dbl angle = axis.normalize())
                orn = m.setRotate(axis, angle);
            else
                orn.identity();
        }
    }
    PROFILE_STOP("CatzEngine::DelRot(Orient &orn, UInt flag)")
}
static void DelRot(Animation &anim, UInt flag) {
    PROFILE_START("CatzEngine::DelRot(Animation &anim, UInt flag)")
    if (FlagAll(flag, ROOT_DEL_ROTATION))
        anim.keys.orns.del();
    else {
        Bool x = FlagOn(flag, ROOT_DEL_ROTATION_X),
             y = FlagOn(flag, ROOT_DEL_ROTATION_Y),
             z = FlagOn(flag, ROOT_DEL_ROTATION_Z);
        REPA(anim.keys.orns) {
            Orient &orn = anim.keys.orns[i].orn;
            MatrixD3 m = orn;
            VecD axis = m.axisAngle(true);
            if (x)
                axis.x = 0;
            if (y)
                axis.y = 0;
            if (z)
                axis.z = 0;
            if (Dbl angle = axis.normalize())
                orn = m.setRotate(axis, angle);
            else
                orn.identity();
        }
        anim.keys.setTangents(anim.loop(), anim.length());
    }
    PROFILE_STOP("CatzEngine::DelRot(Animation &anim, UInt flag)")
}
struct SphericalInterpolator {
    Bool linear;
    Flt rot_angle, circle_r;
    Vec rot_axis, circle_pos, circle_x, circle_y, linear_delta;

    Flt circleDist() C { return circle_r * rot_angle; }             // distance travelled along the circle edge
    Flt linearDist() C { return linear_delta.length(); }            // distance travelled along the 'linear_delta' edge
    Flt travelDist() C { return Dist(circleDist(), linearDist()); } // linear distance is always perpendicular to the rotation plane (on separate axis), this is a helix length

    Bool init(C Animation &anim, Flt eps = EPS) // 'eps' must be >=0 because codes below will fail if angle is <=0
    {
        PROFILE_START("CatzEngine::init(C Animation &anim, Flt eps = EPS)")
        C Matrix &start = anim.rootStart(), &end = anim.rootEnd();
        DEBUG_ASSERT(eps >= 0, "SphericalInterpolator eps");
        GetDelta(rot_axis, start, end);
        rot_angle = rot_axis.normalize();
        if (rot_angle <= eps) {
            PROFILE_STOP("CatzEngine::init(C Animation &anim, Flt eps = EPS)")
            return false; // if 'rot_angle' is close to zero, then there is no rotation and we can fall back to linear interpolation
        }

        if (rot_angle >= PI - eps) // rotation by 180 deg, special case where 'rot_axis' direction may be imprecise
        {
            // have to calculate transform from start to middle (instead of end), and then make it a full 180 deg rotation
            Orient orn;
            AnimParams params(anim, anim.length() / 2);
            if (anim.keys.orn(orn, params)) // calculate orientation at middle of animation
            {
                Vec temp;
                GetDelta(temp, start, orn); // get rotation from start to middle
                if (temp.normalize() > eps) // if have any rotation (and normalize it)
                {                           // use that rotation
                    rot_axis = temp;
                    rot_angle = PI; // make it a full 180 deg
                }
            }
        }

        Vec full_delta = end.pos - start.pos;                     // full delta from 'start' to 'end'
        Vec rot_plane_delta = PointOnPlane(full_delta, rot_axis); // delta on rotation plane
        linear_delta = full_delta - rot_plane_delta;              // remaining delta that has to be processed linearly

        // isosceles triangle inside the interpolation circle, triangle top is located at the center of circle, bottom vertexes are 'start.pos' and 'end.pos', bottom edge is 'delta' (before normalization)
        Flt delta_len = rot_plane_delta.normalize();
        if (delta_len <= eps) // linear
        {
            linear = true;
            circle_r = 0;
            circle_pos = start.pos;
            circle_x.zero();
            circle_y.zero();
        } else {
            linear = false;
            Vec cross = Cross(rot_plane_delta, rot_axis); // no need for 'CrossN' because 'rot_plane_delta' and 'rot_axis' are already normalized and perpendicular
            Flt delta_len_2 = delta_len * 0.5f,
                tri_height = delta_len_2 * Ctg(rot_angle / 2); // ctg=1/tan
            circle_r = Dist(tri_height, delta_len_2);
            circle_pos = start.pos + rot_plane_delta * delta_len_2 // center between start and end (bottom if triangle)
                         - cross * tri_height;
            circle_x = cross * circle_r;
            circle_y = rot_plane_delta * circle_r;
        }
        PROFILE_STOP("CatzEngine::init(C Animation &anim, Flt eps = EPS)")
        return true;
    }
    void set(Vec &pos, Flt frac) {
        if (linear)
            pos = circle_pos + linear_delta * frac;
        else {
            Flt angle = (frac - 0.5f) * rot_angle; // subtract 0.5 because triangle has bottom half-way through 'delta'
            Vec2 cs;
            CosSin(cs.x, cs.y, angle);
            pos = circle_pos + circle_x * cs.x + circle_y * cs.y + linear_delta * frac;
        }
    }
};
Animation &Animation::adjustForSameTransformWithDifferentSkeleton(C Skeleton &old_skel, C Skeleton &new_skel, Int old_skel_bone_as_root, C CMemPtr<Mems<IndexWeight>> &weights, UInt root_flags) {
    PROFILE_START("CatzEngine::Animation::adjustForSameTransformWithDifferentSkeleton(C Skeleton &old_skel, C Skeleton &new_skel, Int old_skel_bone_as_root, C CMemPtr<Mems<IndexWeight>> &weights, UInt root_flags)")
    /*
       'old_skel' and 'new_skel'     are assumed to have the same names when 'weights' are null (if 'weights' are not null, then their indexes are used as 'new_skel' -> 'old_skel' mapping, and names/types are ignored)
       'old_skel' and this Animation are assumed to have the same names when FIND_ANIM_BY_NAME_ONLY


       Warning: this method should handle cases when bones are removed, for example:

       'old_skel':
       "Spine0" BONE_SPINE 0
       "Spine1" BONE_SPINE 1
       "Spine2" BONE_SPINE 2

       'new_skel' is 'old_skel' with "Spine1" removed:
       "Spine0" BONE_SPINE 0
       "Spine2" BONE_SPINE 1

       "Spine2" now has 'type_index' 1 the same as "Spine1" in 'old_skel'


       The goal here is to preserve transforms (transformation matrixes applied to bones),
          In order to do that, we calculate transforms applied to old skeleton bones,
          and then we set the animations to make sure that they will trigger the same transforms on new skeleton bones.

    */
    Animation anim_out;
    anim_out.copyParams(T).events = events;
    AnimSkel old_askel;
    old_askel.create(&old_skel);
    SkelAnim old_skela(old_skel, T);
    AnimSkel new_askel;
    new_askel.create(&new_skel);
    Memt<Flt, 16384> orn_times, pos_times, scale_times;

    Bool root_not_changed = !InRange(old_skel_bone_as_root, old_skel.bones),
         root_not_changed_post = true;

    // process root
    if (!root_not_changed) {
        C SkelBone &old_bone = old_skel.bones[old_skel_bone_as_root];
        C AnimSkelBone &old_abon = old_askel.bones[old_skel_bone_as_root];
#if FIND_ANIM_BY_NAME_ONLY
        C AnimBone *old_anim = findBone(old_bone.name);
#else
        C AnimBone *old_anim = findBone(old_bone.name, old_bone.type, old_bone.type_index, old_bone.type_sub);
#endif
        Bool no_parent = (old_skel.boneParent(old_skel_bone_as_root) < 0); // bone has no parent

        includeTimesForBoneAndItsParents(old_skel, old_skel_bone_as_root, orn_times, pos_times, scale_times);

        // orientation
        if (no_parent && !keys.orns.elms() && !keys.scales.elms()) // simple version
        {
            MatrixD3 old_bone_inv;
            old_bone.inverse(old_bone_inv);
            anim_out.keys.orns.setNumDiscard(old_anim ? old_anim->orns.elms() : 0);
            REPA(anim_out.keys.orns) {
                C AnimKeys::Orn &src = old_anim->orns[i];
                AnimKeys::Orn &orn = anim_out.keys.orns[i];
                orn.time = src.time;
#if 0
            orn.orn=GetTransform(old_bone, src.orn);
#else
                MatrixD3 transform = old_bone_inv;
                transform *= MatrixD3(src.orn);
                orn.orn = transform;
#endif
            }
        } else // complex version
        {
            anim_out.keys.orns.setNumDiscard(orn_times.elms());
            REPA(anim_out.keys.orns) {
                AnimKeys::Orn &orn = anim_out.keys.orns[i];
                old_askel.clear().animateEx(old_skela, orn.time = orn_times[i]).updateMatrixParents(MatrixMIdentity, old_skel_bone_as_root);
                OrientD o = old_abon.matrix();
                o.fix(); // root bone world orientation (identity transformed by bone transform) at the current time
                orn.orn = o;
            }
        }

        // scale
        anim_out.keys.scales.setNumDiscard(scale_times.elms());
        REPA(anim_out.keys.scales) { // Warning: Scale conversion is not perfect, because scales are possible only on each axis separately, so if new bone is not rotated by 90 deg, then artifacts may occur
            AnimKeys::Scale &scale = anim_out.keys.scales[i];
            old_askel.clear().animateEx(old_skela, scale.time = scale_times[i]).updateMatrixParents(MatrixMIdentity, old_skel_bone_as_root);
            Vec old_scale(old_abon.matrix().x.length(),
                          old_abon.matrix().y.length(),
                          old_abon.matrix().z.length());
            scale.scale = ScaleFactorR(old_scale);
        }

        // position
        Bool root_zero; // if root position is zero
        Vec root_pos;
        if (root_flags & ROOT_BONE_POSITION) {
            root_pos = old_bone.pos;
            root_zero = (root_pos.length2() <= Sqr(EPS));
        } else
            root_zero = true;

        if (no_parent && !keys.is() && root_zero) // no parent, no root keys, and root position is zero
        {
            if (old_anim)
                anim_out.keys.poss = old_anim->poss;
        } else {
            IncludeTimes(orn_times, pos_times);   // this bone and parent orientations affect this position
            IncludeTimes(scale_times, pos_times); // this bone and parent scales       affect this position
            if (pos_times.elms()) {
                anim_out.keys.poss.setNumDiscard(pos_times.elms());
                REPA(anim_out.keys.poss) {
                    AnimKeys::Pos &pos = anim_out.keys.poss[i];
                    old_askel.clear().animateEx(old_skela, pos.time = pos_times[i]).updateMatrixParents(MatrixMIdentity, old_skel_bone_as_root);
                    pos.pos = (root_zero ? old_abon.matrix().pos          // desired mesh transform offset
                                         : root_pos * old_abon.matrix()); // desired bone position
                }
                if (!root_zero)
                    root_not_changed_post = false;
            }
        }
        orn_times.clear();
        scale_times.clear();
        pos_times.clear();

        anim_out.keys.setTangents(loop(), length());
        anim_out.setRootMatrix();
    } else { // preserve root keys
        anim_out.keys = keys;
        anim_out._root_start = _root_start;
        anim_out._root_start_inv = _root_start_inv;
        anim_out._root_end = _root_end;
        anim_out._root_transform = _root_transform;
    }

    MemtN<BoneWeight, 4> old_bones;

    if ((root_flags & ROOT_START_IDENTITY) && anim_out.keys.is() && !Equal(anim_out.rootStart(), MatrixIdentity)) // !! adjust before ROOT_DEL and ROOT_SMOOTH !!
    {
        AnimParams params(anim_out, 0);
        if (!anim_out.keys.scales.elms()) {
#if 0 // using Matrix
         REPAO(anim_out.keys.poss).pos*=anim_out._root_start_inv;
         REPAO(anim_out.keys.orns).orn*=anim_out._root_start_inv;
         anim_out.keys.setTangents(anim_out.loop(), anim_out.length());
         anim_out._root_start    .identity();
         anim_out._root_start_inv.identity();
         anim_out._root_end      =rootTransform();
       //anim_out._root_transform=rootTransform(); remains the same
#else // using RevMatrix
            REPA(anim_out.keys.orns) {
                Orient &orn = anim_out.keys.orns[i].orn;
                orn = anim_out._root_start_inv.orn() * orn;
            }
#endif
        } else {
            Matrix root_start_inv = anim_out.rootStart();
            Vec root_start_scale = 1 / root_start_inv.scale();
            root_start_inv.scaleOrnL(root_start_scale);
            root_start_inv.inverse(true);
            REPA(anim_out.keys.orns) {
                Orient &orn = anim_out.keys.orns[i].orn;
                orn = root_start_inv.orn() * orn;
            }
            REPA(anim_out.keys.scales) {
                Vec &scale = anim_out.keys.scales[i].scale;
                scale = ScaleFactorR(ScaleFactor(scale) * root_start_scale);
            }
        }
        REPA(anim_out.keys.poss) {
            AnimKeys::Pos &pos = anim_out.keys.poss[i];
            params.time = pos.time;
            Orient orn;
            if (!anim_out.keys.orn(orn, params) && !SET_ON_FAIL)
                orn.identity();
            else
                DelRot(orn, root_flags);
            pos.pos -= anim_out.rootStart().pos * orn;
        } // this needs to be done after adjusting orientations, it's better to delete rotations before adjusting positions, "this is needed to cast position on plane" for example if character body is rotating and moving, then this will try to cancel out movement through rotation, and just focus on the movement only
        anim_out.keys.setTangents(anim_out.loop(), anim_out.length());
        anim_out.setRootMatrix();
        root_not_changed = root_not_changed_post = false;
    }
    if ((root_flags & ROOT_DEL) && anim_out.keys.is()) // !! delete after ROOT_START_IDENTITY because it needs rotations and positions to perform adjustment !!
    {
        Bool changed = false;
        if (FlagAll(root_flags, ROOT_DEL)) {
            anim_out.keys.del();
            changed = true;
        } else {
            if ((root_flags & ROOT_DEL_POSITION) && anim_out.keys.poss.elms()) {
                if (FlagAll(root_flags, ROOT_DEL_POSITION))
                    anim_out.keys.poss.del();
                else {
                    REPA(anim_out.keys.poss) {
                        Vec &pos = anim_out.keys.poss[i].pos;
                        if (root_flags & ROOT_DEL_POSITION_X)
                            pos.x = 0;
                        if (root_flags & ROOT_DEL_POSITION_Y)
                            pos.y = 0;
                        if (root_flags & ROOT_DEL_POSITION_Z)
                            pos.z = 0;
                    }
                    anim_out.keys.setTangents(anim_out.loop(), anim_out.length());
                }
                changed = true;
            }
            if ((root_flags & ROOT_DEL_ROTATION) && anim_out.keys.orns.elms()) {
                DelRot(anim_out, root_flags);
                changed = true;
            }
            if ((root_flags & ROOT_DEL_SCALE) && anim_out.keys.scales.elms()) {
                anim_out.keys.scales.del();
                changed = true;
            }
        }
        if (changed) {
            anim_out.setRootMatrix();
            root_not_changed = root_not_changed_post = false;
        }
    }
    if (root_flags & (ROOT_SMOOTH | ROOT_LINEAR_POS)) // !! do this after ROOT_START_IDENTITY, because otherwise using only ROOT_SMOOTH may change positions too !!
    {
        if (root_flags & ROOT_LINEAR_POS)
            FlagDisable(root_flags, ROOT_SMOOTH_POS); // if want only simple linear pos, then disable smooth pos

        Bool changed = false;
        SphericalInterpolator si;
        if ((root_flags & ROOT_SMOOTH_ROT_POS) && anim_out.keys.orns.elms() >= 2 && si.init(anim_out, EPS_ANIM_ANGLE)) // use spherical interpolation only if we actually have some rotations
        {
            if (root_flags & ROOT_SMOOTH_ROT) // orientations
            {
                const Int precision = 4; // number of keyframes per 90 deg, can be modified, this is needed because rotation interpolation is done by interpolating axis vectors, and few samples are needed to get smooth results
                Int num = 1 + Max(1, Round(si.rot_angle * (precision / PI_2)));
                anim_out.keys.orns.setNum(num);
                if (num) {
                    OrientD orn = anim_out.rootStart();
                    orn.fix();
                    AnimKeys::Orn &key = anim_out.keys.orns.first();
                    key.time = 0;
                    key.orn = orn;
                    if (num >= 2) {
                        AnimKeys::Orn &key = anim_out.keys.orns.last();
                        key.time = anim_out.length();
                        (key.orn = anim_out.rootEnd()).fix();
                        if (num >= 3) {
                            num--;
                            MatrixD3 rot;
                            rot.setRotate(si.rot_axis, si.rot_angle / num);
                            for (int i = 1; i < num; i++) {
                                orn.mul(rot, true);
                                AnimKeys::Orn &key = anim_out.keys.orns[i];
                                key.time = Flt(i) / num * anim_out.length();
                                key.orn = orn;
                            }
                        }
                    }
                }
                FlagDisable(root_flags, ROOT_SMOOTH_ROT); // mark as processed
                changed = true;
            }

            if ((root_flags & ROOT_SMOOTH_POS) && !si.linear) // positions (if linear then skip here, and we will process them later below)
            {
                Flt travel_distance = si.travelDist();
                const Int precision = 10; // number of keyframes per meter, can be modified
                Int num = ((travel_distance <= EPS) ? Equal(anim_out.rootStart().pos, VecZero) ? 0 : 1 : 1 + Max(1, Round(travel_distance * precision)));
                anim_out.keys.poss.setNum(num);
                if (num) {
                    AnimKeys::Pos &key = anim_out.keys.poss.first();
                    key.time = 0;
                    key.pos = anim_out.rootStart().pos;
                    if (num >= 2) {
                        AnimKeys::Pos &key = anim_out.keys.poss.last();
                        key.time = anim_out.length();
                        key.pos = anim_out.rootEnd().pos;
                        num--;
                        for (Int i = 1; i < num; i++) {
                            Flt frac = Flt(i) / num;
                            AnimKeys::Pos &key = anim_out.keys.poss[i];
                            key.time = frac * anim_out.length();
                            si.set(key.pos, frac);
                        }
                    }
                }
                FlagDisable(root_flags, ROOT_SMOOTH_POS); // mark as processed
                changed = true;
            }
        }
        if ((root_flags & ROOT_SMOOTH_ROT) && anim_out.keys.orns.elms() > 2) // if still have orientations to process
        {
            anim_out.keys.orns.setNumDiscard(2);
            anim_out.keys.orns[0].time = 0;
            anim_out.keys.orns[0].orn = anim_out.rootStart();
            anim_out.keys.orns[0].orn.fix();
            anim_out.keys.orns[1].time = anim_out.length();
            anim_out.keys.orns[1].orn = anim_out.rootEnd();
            anim_out.keys.orns[1].orn.fix();
            changed = true;
        }
        if ((root_flags & (ROOT_SMOOTH_POS | ROOT_LINEAR_POS)) && anim_out.keys.poss.elms() > 2) // if still have positions to process
        {
            anim_out.keys.poss.setNumDiscard(2);
            anim_out.keys.poss[0].time = 0;
            anim_out.keys.poss[0].pos = anim_out.rootStart().pos;
            anim_out.keys.poss[1].time = anim_out.length();
            anim_out.keys.poss[1].pos = anim_out.rootEnd().pos;
            changed = true;
        }
        if ((root_flags & ROOT_SMOOTH_SCALE) && anim_out.keys.scales.elms() > 2) // if still have scales to process
        {
            anim_out.keys.scales.setNumDiscard(2);
            anim_out.keys.scales[0].time = 0;
            anim_out.keys.scales[0].scale = ScaleFactorR(anim_out.rootStart().scale());
            anim_out.keys.scales[1].time = anim_out.length();
            anim_out.keys.scales[1].scale = ScaleFactorR(anim_out.rootEnd().scale());
            changed = true;
        }
        if (changed) {
            anim_out.keys.setTangents(anim_out.loop(), anim_out.length());
            // anim_out.setRootMatrix(); no need to call because it doesn't change
            root_not_changed = root_not_changed_post = false;
        }
    }

    // process bones
    FREPAD(new_bone_i, new_skel.bones) // process in order because when calculating this for children, we need to have parents already setup
    {
        C SkelBone &new_bone = new_skel.bones[new_bone_i];
        // get all links from 'new_bone' to 'old_bones'
        if (!weights) {
            Int old_bone_i = old_skel.findBoneI(new_bone.name);
            if (old_bone_i >= 0)
                old_bones.New().set(old_bone_i, 1);
        } else if (InRange(new_bone_i, weights)) {
            C Mems<IndexWeight> &weight = weights[new_bone_i];
            FREPA(weight) {
                C IndexWeight &iw = weight[i];
                if (InRange(iw.index, old_skel.bones))
                    SCAST(IndexWeight, old_bones.New()) = iw;
            }
        }
        if (old_bones.elms()) // if there are any links
        {
            Flt weight_sum = 0;
            REPA(old_bones) {
                BoneWeight &bw = old_bones[i];
                bw.bone = &old_skel.bones[bw.index];
                bw.asbone = &old_askel.bones[bw.index];
                weight_sum += bw.weight; // set bone pointers and calculate total weight
            }
            REPAO(old_bones).weight /= weight_sum; // normalize weights

            C SkelBone &old_bone = *old_bones[0].bone,
                       *new_parent = new_skel.bones.addr(new_bone.parent),
                       *old_parent = old_skel.bones.addr(old_bone.parent);

            if (old_bones.elms() == 1 && old_bones[0].index == old_skel_bone_as_root && !new_parent && root_not_changed_post) // if this new bone is made only from one old bone that is converted to root, and the new bone doesn't have any parents, and root wasn't post-changed
            {
                // we can just set identity orientation, because everything else will be handled by root
                OrientD orient = new_bone;
                if (new_parent)
                    orient.div(*new_parent, true); // convert to parent space
                AnimBone &new_anim = anim_out.bones.New();
                new_anim.id() = new_bone;
                AnimKeys::Orn &orn = new_anim.orns.New();
                orn.time = 0;
                orn.orn = orient;
#if HAS_ANIM_TANGENT
                orn.tan.zero();
#endif
            } else                                                                                                    // we can do the simple version only if
                if (old_bones.elms() == 1                                                                             // bone maps to only 1 old bone
                    && SameSet(old_bone, new_bone, old_parent, new_parent, weights, old_bone.parent, new_bone.parent) // it has the same set of bones/parents
                    && (old_parent || root_not_changed))                                                              // and there are parents or (when there are no parents, which means that parent is root) we preserve root animations
                {
#if FIND_ANIM_BY_NAME_ONLY
                    if (C AnimBone *old_anim = findBone(old_bone.name))
#else
                    if (C AnimBone *old_anim = findBone(old_bone.name, old_bone.type, old_bone.type_index, old_bone.type_sub))
#endif
                    {
                        OrientD new_bone_d = new_bone;
                        MatrixD3 old_bone_m = old_bone, old_bone_m_inv, old_parent_m, new_parent_m_inv;
                        old_bone.inverse(old_bone_m_inv); // in the simple version we HAVE TO ignore positions of bones and process only orientations, yes this was tested
                        if (old_parent)
                            old_parent_m = *old_parent;
                        if (new_parent)
                            new_parent->inverse(new_parent_m_inv);
                        AnimBone &new_anim = anim_out.bones.New();
                        new_anim = *old_anim;
                        new_anim.id() = new_bone;

                        REPA(new_anim.orns) // orientation
                        {
                            OrientD orn = new_anim.orns[i].orn;
                            if (old_parent)
                                orn.mul(old_parent_m, true); // convert to world space from old parent space
                            MatrixD3 anim_transform;
#if 0
                  GetTransform(anim_transform, old_bone_m, MatrixD3(orn)); // transform which animates skel bone into animated bone
#else
                            anim_transform = old_bone_m_inv;
                            anim_transform *= MatrixD3(orn); // transform which animates skel bone into animated bone
#endif
                            orn = new_bone_d;              // get new bone
                            orn.mul(anim_transform, true); // apply animation transform
                            if (new_parent)
                                orn.mul(new_parent_m_inv, true); // convert to new parent space
                            new_anim.orns[i].orn = orn;
                        }
                        // if(!new_anim.orns.elms()).. any code like that is not needed, because here we preserve transformations, so if source keyframe is not found, then it means transformation was identity

                        // position
                        // if(!new_anim.poss.elms()){AnimKeys::Pos &pos=new_anim.poss.New(); pos.pos.zero(); pos.time=0;} any code like that is not needed, because we operate only on rotating existing positions here in the simple version
                        if (new_anim.poss.elms()) {
                            Bool pos_transform = (old_parent || new_parent); // if transform pos
                            MatrixD3 pos_matrix;
                            if (old_parent && new_parent)
                                old_parent_m.mul(new_parent_m_inv, pos_matrix);
                            else // both present, pos_matrix=old_parent_m*new_parent_m_inv
                                if (old_parent)
                                    pos_matrix = old_parent_m;
                                else // only old_parent
                                    if (new_parent)
                                        pos_matrix = new_parent_m_inv; // only new_parent
                            REPA(new_anim.poss)                        // position
                            {
                                VecD pos = new_anim.poss[i].pos;
#if 0
                     if(old_parent)pos*=old_parent_m    ; // convert to world space from old parent space
                     if(new_parent)pos*=new_parent_m_inv; // convert to new parent space
#else // optimized
                                if (pos_transform)
                                    pos *= pos_matrix;
#endif
                                new_anim.poss[i].pos = pos;
                            }
                        }

                        // scale
                        if (new_anim.scales.elms()) {                             // Warning: Scale conversion is not perfect, because scales are possible only on each axis separately, so if new bone is not rotated by 90 deg, then artifacts may occur
                            MatrixD3 new_bone_in_old_bone_space = new_bone_d;     // get the new bone
                            new_bone_in_old_bone_space.divNormalized(old_bone_m); // put it into old bone space
                            REPA(new_anim.scales) {
                                VecD scale = new_anim.scales[i].scale;
                                MatrixD3 scaled_new_bone = new_bone_in_old_bone_space;
                                scaled_new_bone.scale(ScaleFactor(scale));                        // apply old bone scale (we're in old bone space, so scaling in XYZ scales along old bone cross perp and dir)
                                new_anim.scales[i].scale = ScaleFactorR(scaled_new_bone.scale()); // calculate new bone scale which is simply each axis length
                            }
                        }
                        new_anim.setTangents(loop(), length());
                    }
                } else // complex/advanced
                {
                    REPA(old_bones)
                    for (Int old_parent_i = old_bones[i].index;;) // iterate all old parents until one of them is an ancestor of new bone
                    {
                        old_parent_i = old_skel.boneParent(old_parent_i);
                        if (old_parent_i < 0) {
                            keys.includeTimes(orn_times, pos_times, scale_times);
                            break;
                        }
                        C SkelBone &old_parent = old_skel.bones[old_parent_i];

#if FIND_ANIM_BY_NAME_ONLY
                        if (AnimBone *abon = findBone(old_parent.name))
#else
                        if (AnimBone *abon = findBone(old_parent.name, old_parent.type, old_parent.type_index, old_parent.type_sub))
#endif
                            abon->includeTimes(orn_times, pos_times, scale_times);

                        if (!weights) {
                            Int new_parent_i = new_skel.findBoneI(old_parent.name);
                            if (new_parent_i >= 0 && new_skel.contains(new_parent_i, new_bone_i))
                                break; // if the parent contains the bone in the new skeleton
                        } else {
                            Bool found = false;
                            REPAD(new_parent_i, weights) // iterate all new bones to check which are connected to 'old_parent_i'
                            {
                                C Mems<IndexWeight> &weight = weights[new_parent_i];
                                REPA(weight)
                                if (weight[i].index == old_parent_i) // if this bone points to 'old_parent_i'
                                {
                                    if (!new_skel.contains(new_parent_i, new_bone_i))
                                        goto new_parent_not_contains_bone; // if at least one parent doesn't contain the bone, then stop this loop and keep checking next parents
                                    found = true;                          // we've found bones connected with 'old_parent_i'
                                    break;
                                }
                            }
                            if (found)
                                break; // found a parent and all parents contain 'new_bone_i', so we can stop now
                        new_parent_not_contains_bone:;
                        }
                    }
                    for (Int new_parent_i = new_bone_i;;) // iterate all new parents until one of them is an ancestor of old bone
                    {
                        new_parent_i = new_skel.boneParent(new_parent_i);
                        if (new_parent_i < 0) {
                            anim_out.keys.includeTimes(orn_times, pos_times, scale_times);
                            break;
                        }
                        C SkelBone &new_parent = new_skel.bones[new_parent_i];

                        if (AnimBone *abon = anim_out.findBone(new_parent.name)) // here we're looking in 'anim_out' which already has names set according to 'new_skel'
                            abon->includeTimes(orn_times, pos_times, scale_times);

                        if (!weights) {
                            Int old_parent_i = old_skel.findBoneI(new_parent.name);
                            if (old_parent_i >= 0 && old_skel.contains(old_parent_i, old_bones[0].index))
                                break; // if the parent contains the bone in the old skeleton
                        } else if (InRange(new_parent_i, weights)) {
                            C Mems<IndexWeight> &parent_weights = weights[new_parent_i];
                            if (parent_weights.elms()) // if this bone has any links
                            {
                                REPA(parent_weights) {
                                    Int old_parent_i = parent_weights[i].index;
                                    REPA(old_bones)
                                    if (!old_skel.contains(old_parent_i, old_bones[i].index))
                                        goto old_parent_not_contains_bone; // if at least one isn't contained, then stop this loop and keep checking next parents
                                }
                                break; // all 'old_bones' are contained in this parent, so we can stop now
                            old_parent_not_contains_bone:;
                            }
                        }
                    }

                    // step 1, normally this could be done after step 2, because this orientations/scales normally don't affect this positions, unless this bone was moved
                    REPA(old_bones) {
                        C SkelBone &old_bone = *old_bones[i].bone;
#if FIND_ANIM_BY_NAME_ONLY
                        if (AnimBone *old_anim = findBone(old_bone.name))
#else
                        if (AnimBone *old_anim = findBone(old_bone.name, old_bone.type, old_bone.type_index, old_bone.type_sub))
#endif
                            old_anim->includeTimes(orn_times, pos_times, scale_times);
                    }
                    // step 2
                    IncludeTimes(orn_times, pos_times);   // parent orientations affect this position
                    IncludeTimes(scale_times, pos_times); // parent scales       affect this position

                    // step 3, make sure we have at least 1 keyframe to process (after including orn times in pos times, in case orn is empty, but pos has something, in which case we would always add a useless key at time 0 for pos since it already has another)
                    if (!orn_times.elms())
                        orn_times.add(0);
                    if (!pos_times.elms())
                        pos_times.add(0);

                    if (orn_times.elms() || pos_times.elms() || scale_times.elms()) {
                        C AnimSkelBone &new_abon = new_askel.bones[new_bone_i];
                        SkelAnim new_skela(new_skel, anim_out); // this needs to be created here and not once at the start, because we're dynamically adding new AnimBones to 'anim_out'
                        AnimBone *added = null, anim;
                        anim.id() = new_bone;
                        Bool root_has_orn_scale = (anim_out.keys.orns.elms() || anim_out.keys.scales.elms());

                        if (orn_times.elms()) // orientation
                        {
                            MatrixD3 new_bone_m = new_bone, new_parent_m;
                            if (new_parent)
                                new_parent_m = *new_parent;
                            anim.orns.setNumDiscard(orn_times.elms());
                            REPA(anim.orns) {
                                AnimKeys::Orn &orn = anim.orns[i];
                                orn.time = orn_times[i];
                                old_askel.clear().animateEx(old_skela, orn.time);
                                if (old_bones.elms() == 1)
                                    old_askel.updateMatrixParents(MatrixMIdentity, old_bones[0].index);
                                else
                                    old_askel.updateMatrix();
                                new_askel.clear().animateEx(new_skela, orn.time);
                                new_askel.updateMatrixParents(MatrixMIdentity, new_bone_i);

                                MatrixD3 new_bone_transformed = new_bone_m;
                                new_bone_transformed *= BoneWeightOrn(old_bones); // bone world orientation at the current time
                                if (new_parent) {
                                    MatrixD3 new_parent_transformed;
                                    new_parent_m.mul(new_askel.bones[new_bone.parent].matrix(), new_parent_transformed); // parent bone world orientation at the current time, new_parent_transformed=new_parent_m*new_askel.bones[new_bone.parent].matrix()
                                    new_bone_transformed *= new_parent_transformed.inverseNonOrthogonal();               // new_bone_transformed/=new_parent_transformed with non-orthogonal support (needed because of possible scale keyframes)
                                } else if (root_has_orn_scale)
                                    new_bone_transformed /= new_askel.root.matrix().orn();
                                OrientD o = new_bone_transformed;
                                o.fix();
                                orn.orn = o;
                            }
                            orn_times.clear();
                            // add orientations to the animation only after everything was setup
                            anim.setTangents(loop(), length());
                            if (added)
                                Swap(anim.orns, added->orns);
                            else {
                                added = &anim_out.bones.New();
                                Swap(anim, *added);
                            }
                            new_skela.create(new_skel, anim_out); // need to re-create because we have added orientations
                        }

                        if (scale_times.elms()) // scale
                        {
                            anim.scales.setNumDiscard(scale_times.elms());
                            REPA(anim.scales) { // Warning: Scale conversion is not perfect, because scales are possible only on each axis separately, so if new bone is not rotated by 90 deg, then artifacts may occur
                                AnimKeys::Scale &scale = anim.scales[i];
                                scale.time = scale_times[i];
                                old_askel.clear().animateEx(old_skela, scale.time);
                                if (old_bones.elms() == 1)
                                    old_askel.updateMatrixParents(MatrixMIdentity, old_bones[0].index);
                                else
                                    old_askel.updateMatrix();
                                new_askel.clear().animateEx(new_skela, scale.time);
                                new_askel.updateMatrixParents(MatrixMIdentity, new_bone_i);

                                Matrix3 old_abon_matrix___orn = BoneWeightOrn(old_bones);
                                Vec new_bone_cross = new_bone.cross(),
                                    old_scale((new_bone_cross * old_abon_matrix___orn).length(), // desired scale (yes here should be 'new_bone' too, because we want to calculate the scale for new bone using old transformation matrix)
                                              (new_bone.perp * old_abon_matrix___orn).length(),
                                              (new_bone.dir * old_abon_matrix___orn).length()),
                                    new_scale((new_bone_cross * new_abon.matrix().orn()).length(), // current scale
                                              (new_bone.perp * new_abon.matrix().orn()).length(),
                                              (new_bone.dir * new_abon.matrix().orn()).length()),
                                    rel_scale = old_scale / new_scale; // relative scale that should be applied
                                scale.scale = ScaleFactorR(rel_scale);
                            }
                            scale_times.clear();
                            // add scales to the animation only after everything was setup
                            anim.setTangents(loop(), length());
                            if (added)
                                Swap(anim.scales, added->scales);
                            else {
                                added = &anim_out.bones.New();
                                Swap(anim, *added);
                            }
                            new_skela.create(new_skel, anim_out); // need to re-create because we have added scales
                        }

                        if (pos_times.elms()) // position
                        {
                            MatrixD3 new_parent_m;
                            if (new_parent)
                                new_parent_m = *new_parent;
                            anim.poss.setNumDiscard(pos_times.elms());
                            REPA(anim.poss) {
                                AnimKeys::Pos &pos = anim.poss[i];
                                pos.time = pos_times[i];
                                old_askel.clear().animateEx(old_skela, pos.time);
                                if (old_bones.elms() == 1)
                                    old_askel.updateMatrixParents(MatrixMIdentity, old_bones[0].index);
                                else
                                    old_askel.updateMatrix();
                                new_askel.clear().animateEx(new_skela, pos.time);
                                new_askel.updateMatrixParents(MatrixMIdentity, new_bone_i);

                                pos.pos = BoneWeightPos(old_bones) // desired mesh transform offset
                                          - new_abon.matrix().pos; // current mesh transform offset
                                if (new_parent) {
                                    MatrixD3 new_parent_transformed;
                                    new_parent_m.mul(new_askel.bones[new_bone.parent].matrix(), new_parent_transformed); // parent bone world orientation at the current time, new_parent_transformed=new_parent_m*new_askel.bones[new_bone.parent].matrix()
                                    pos.pos *= new_parent_transformed.inverseNonOrthogonal();                            // pos.pos/=new_parent_transformed with non-orthogonal support (needed because of possible scale keyframes)
                                } else if (root_has_orn_scale)
                                    pos.pos /= new_askel.root.matrix().orn();
                            }
                            pos_times.clear();
                            // add positions to the animation only after everything was setup
                            anim.setTangents(loop(), length());
                            if (added)
                                Swap(anim.poss, added->poss);
                            else {
                                added = &anim_out.bones.New();
                                Swap(anim, *added);
                            }
                            // new_skela.create(new_skel, anim_out); // need to re-create because we have added positions, won't be used for anything
                        }
                    }
                }
            old_bones.clear();
        }
    }
    PROFILE_STOP("CatzEngine::Animation::adjustForSameTransformWithDifferentSkeleton(C Skeleton &old_skel, C Skeleton &new_skel, Int old_skel_bone_as_root, C CMemPtr<Mems<IndexWeight>> &weights, UInt root_flags)")
    Swap(anim_out, T);
    return T;
}
Animation &Animation::offsetRootBones(C Skeleton &skeleton, C Vec &move) {
    PROFILE_START("CatzEngine::Animation::offsetRootBones(C Skeleton &skeleton, C Vec &move)")
    if (move.any()) {
        AnimParams params(T, 0);
        Memt<Flt, 16384> times;
        Bool adjust_root = (true && keys.orns.elms() > 1); // if adjust root positions too, because normally root is based on body movements relative to body start, when root has variable orientations, then 'move' depends on those orientations, so in that case body movement is variable based on current root orientation, so the body moves in different ways, and this allows to adjust the root movement to follow the body movement, this is needed only if root orientations change during animation, so check for more than 1 orientation keys
        Orient orn;
        Vec root_start, pos;
        if (adjust_root) {
            keys.orn(orn, params); // this always succeeds because 'adjust_root' requires orientation keys to be available
            root_start = move * orn;
        }
        REPA(skeleton.bones) {
            C SkelBone &sbon = skeleton.bones[i];
            if (sbon.parent == BONE_NULL) {
                AnimBone &abon = getBone(sbon.name, sbon.type, sbon.type_index, sbon.type_sub);
                if (keys.orns.elms()) {
                    times.clear();
                    keys.includeTimes(times, null, null); // orientations from root
                    abon.includeTimes(null, times, null); //    positions from bone
                    if (!times.elms())
                        times.add(0);
                    Mems<AnimKeys::Pos> poss;
                    poss.setNumDiscard(times.elms());
                    FREPA(poss) {
                        AnimKeys::Pos &key = poss[i];
                        key.time = params.time = times[i];
                        if (keys.orn(orn, params)) {
                            if (adjust_root) {
                                /*Vec root_offset=root_start - move*orn;
                                  key.pos=move/orn - root_offset/orn; base formula adjusted by 'root_offset'
                                  key.pos=move/orn - root_start/orn + move*orn/orn;
                                  key.pos=(move-root_start)/orn + move;*/
                                key.pos.fromDivNormalized(move - root_start, orn) += move;
                            } else {
                                key.pos.fromDivNormalized(move, orn); // key.pos=move/orn; this is the base formula without 'adjust_root'
                            }
                        } else
                            key.pos = move; // if there are no orientations then no need to check for 'adjust_root'
                        if (abon.pos(pos, params))
                            key.pos += pos;
                    }
                    Swap(abon.poss, poss);
                    abon.setTangents(loop(), length());
                } else {
                    if (!abon.poss.elms()) {
                        AnimKeys::Pos &key = abon.poss.New();
                        key.time = 0;
                        key.pos = move;
                    } else
                        REPAO(abon.poss).pos += move;
                    // tangents don't change
                }
            }
        }
        if (adjust_root) {
            times.clear();
            keys.includeTimes(times, times, null); // orientations+positions from root
            Mems<AnimKeys::Pos> poss;
            poss.setNumDiscard(times.elms());
            FREPA(poss) {
                AnimKeys::Pos &key = poss[i];
                key.time = params.time = times[i];
                keys.orn(orn, params);             // this always succeeds because 'adjust_root' requires orientation keys to be available
                key.pos = root_start - move * orn; // this is 'root_offset'
                if (keys.pos(pos, params))
                    key.pos += pos;
            }
            Swap(keys.poss, poss);
            keys.setTangents(loop(), length());
            setRootMatrix();
        }
    }
    PROFILE_STOP("CatzEngine::Animation::offsetRootBones(C Skeleton &skeleton, C Vec &move)")
    return T;
}
/******************************************************************************/
static void FreezePos(Animation &anim, C Skeleton &skel, Int skel_bone, Int key_index, C Vec *delta) {
    PROFILE_START("CatzEngine::FreezePos(Animation &anim, C Skeleton &skel, Int skel_bone, Int key_index, C Vec *delta)")
    Bool root = (skel_bone < 0);
    if (root || InRange(skel_bone, skel.bones)) {
        AnimBone *abon;
        AnimKeys *keys;
        Matrix3 default_orn;
        if (root) {
            abon = null;
            keys = &anim.keys;
            default_orn.identity();
        } else {
            C SkelBone &sbon = skel.bones[skel_bone];
            if (abon = anim.findBone(sbon.name, sbon.type, sbon.type_index, sbon.type_sub)) {
                keys = abon;
                default_orn = GetAnimOrient(sbon, skel.bones.addr(sbon.parent));
            } else {
                PROFILE_STOP("CatzEngine::FreezePos(Animation &anim, C Skeleton &skel, Int skel_bone, Int key_index, C Vec *delta)")
                return;
            }
        }
        Bool all_keys = (key_index < 0);
        if (all_keys || InRange(key_index, keys->poss)) {
            BoneType bone_index = (root ? BONE_NULL : skel_bone);

            Memt<Flt, 16384> times;
            Flt prev_time, next_time;
            if (all_keys)
                keys->includeTimes(times, times, times);
            else {
                prev_time = (InRange(key_index - 1, keys->poss) ? keys->poss[key_index - 1].time : 0);
                Flt key_time = keys->poss[key_index].time;
                next_time = (InRange(key_index + 1, keys->poss) ? keys->poss[key_index + 1].time : anim.length());
                times.binaryInclude(prev_time, CompareEps);
                times.binaryInclude(key_time, CompareEps);
                times.binaryInclude(next_time, CompareEps);
                keys->includeTimes(times, null, times, prev_time, next_time); // include orientation and scale keys between 'prev_time..next_time'
            }

            AnimKeys src = *keys; // before removal TODO: it's not optimal to copy like this

            if (delta) // move
            {
                if (all_keys)
                    REPAO(keys->poss).pos += *delta;
                else
                    keys->poss[key_index].pos += *delta;
            } else // delete
            {
                if (all_keys)
                    keys->poss.del();
                else
                    keys->poss.remove(key_index, true);
            }
            keys->setTangents(anim.loop(), anim.length());
            if (root)
                anim.setRootMatrix();

            AnimKeys cur = *keys; // after removal, keep as copy because 'removeData' and 'getBone' below, might change mem address TODO: it's not optimal to copy like this

            if (abon && !abon->is())
                anim.bones.removeData(abon);
            // !! CAN'T ACCESS 'keys', 'abon' ANYMORE AFTER THIS POINT !!

            AnimParams anim_params(anim, 0);
            REPA(skel.bones) {
                C SkelBone &child_sbon = skel.bones[i];
                if (child_sbon.parent == bone_index) // 'skel_bone' child
                {
                    AnimBone &abon = anim.getBone(child_sbon.name, child_sbon.type, child_sbon.type_index, child_sbon.type_sub); // 'skel_bone' child

                    Memt<Flt, 16384> child_times = times; // include child orientation keys for higher precision
                    if (all_keys)
                        abon.includeTimes(child_times, null, null);
                    else
                        abon.includeTimes(child_times, null, null, prev_time, next_time);

                    // add keys (Warning: TODO: this is precise only for linear interpolation, for other case we would have to copy 'abon' to temporary and calculate from it)
                    FREPA(child_times) // process forward because keys are sorted
                    {
                        Flt time = child_times[i];
                        Int index;
                        if (!abon.poss.binarySearch(time, index, CompareEps)) {
                            Vec pos;
                            anim_params.time = time;
                            if (!abon.pos(pos, anim_params) && !SET_ON_FAIL)
                                pos.zero(); // calculate before adding
                            AnimKeys::Pos &key = abon.poss.NewAt(index);
                            key.time = time;
                            key.pos = pos;
                            ASSERT(!HAS_ANIM_TANGENT);
                        }
                    }

                    Int start, end;
                    if (all_keys) {
                        start = 0;
                        end = abon.poss.elms();
                    } else {
                        abon.poss.binarySearch(prev_time, start, Compare);
                        if (abon.poss.binarySearch(next_time, end, Compare))
                            end++; // end is exclusive, so if found match, process it too, in case key is at the end
                    }
                    for (Int i = start; i < end; i++) // process keys from start to end
                    {
                        AnimKeys::Pos &pos = abon.poss[i];
                        anim_params.time = pos.time;
                        Matrix src_matrix;
                        src.matrix(src_matrix, anim_params, default_orn);
                        Vec cur_matrix_pos;
                        if (!cur.pos(cur_matrix_pos, anim_params) && !SET_ON_FAIL)
                            cur_matrix_pos.zero(); // only position has changed
                        pos.pos += (src_matrix.pos - cur_matrix_pos) / src_matrix.orn();
                    }

                    abon.setTangents(anim.loop(), anim.length());
                }
            }
        }
    }
    PROFILE_STOP("CatzEngine::FreezePos(Animation &anim, C Skeleton &skel, Int skel_bone, Int key_index, C Vec *delta)")
}
static void FreezeRot(Animation &anim, C Skeleton &skel, Int skel_bone, Int key_index, C Matrix3 *matrix, Bool pos) {
    PROFILE_START("CatzEngine::FreezeRot(Animation &anim, C Skeleton &skel, Int skel_bone, Int key_index, C Matrix3 *matrix, Bool pos)")
    Bool root = (skel_bone < 0);
    if (root || InRange(skel_bone, skel.bones)) {
        C SkelBone *sbon;
        AnimBone *abon;
        AnimKeys *keys;
        Orient default_orn;
        if (root) {
            sbon = null;
            abon = null;
            keys = &anim.keys;
            default_orn.identity();
        } else {
            sbon = &skel.bones[skel_bone];
            if (abon = anim.findBone(sbon->name, sbon->type, sbon->type_index, sbon->type_sub)) {
                keys = abon;
                default_orn = GetAnimOrient(*sbon, skel.bones.addr(sbon->parent));
            } else {
                PROFILE_STOP("CatzEngine::FreezeRot(Animation &anim, C Skeleton &skel, Int skel_bone, Int key_index, C Matrix3 *matrix, Bool pos)")
                return;
            }
        }
        Bool all_keys = (key_index < 0);
        if (all_keys || InRange(key_index, keys->orns)) {
            BoneType bone_index = (root ? BONE_NULL : skel_bone);

            Memt<Flt, 16384> times;
            Flt prev_time, next_time;
            if (all_keys)
                keys->includeTimes(times, null, null);
            else {
                prev_time = (InRange(key_index - 1, keys->orns) ? keys->orns[key_index - 1].time : 0);
                Flt key_time = keys->orns[key_index].time;
                next_time = (InRange(key_index + 1, keys->orns) ? keys->orns[key_index + 1].time : anim.length());
                times.binaryInclude(prev_time, CompareEps);
                times.binaryInclude(key_time, CompareEps);
                times.binaryInclude(next_time, CompareEps);
            }

            AnimKeys src = *keys; // before removal TODO: it's not optimal to copy like this

            if (matrix) // transform
            {
                if (all_keys)
                    REPAO(keys->orns).orn.mul(*matrix, true).fix();
                else
                    keys->orns[key_index].orn.mul(*matrix, true).fix();
            } else // delete
            {
                if (all_keys)
                    keys->orns.del();
                else
                    keys->orns.remove(key_index, true);
            }
            keys->setTangents(anim.loop(), anim.length());
            if (root)
                anim.setRootMatrix();

            AnimKeys cur = *keys; // after removal, keep as copy because 'removeData' and 'getBone' below, might change mem address TODO: it's not optimal to copy like this

            if (abon && !abon->is())
                anim.bones.removeData(abon);
            // !! CAN'T ACCESS 'keys', 'abon' ANYMORE AFTER THIS POINT !!

            AnimParams anim_params(anim, 0);
            REPA(skel.bones) {
                C SkelBone &child_sbon = skel.bones[i];
                if (child_sbon.parent == bone_index) // 'skel_bone' child
                {
                    AnimBone &abon = anim.getBone(child_sbon.name, child_sbon.type, child_sbon.type_index, child_sbon.type_sub); // 'skel_bone' child

                    // add keys (Warning: TODO: this is precise only for linear interpolation, for other case we would have to copy 'abon' to temporary and calculate from it)
                    FREPA(times) // process forward because keys are sorted
                    {
                        Flt time = times[i];
                        Int index;
                        if (!abon.orns.binarySearch(time, index, CompareEps)) {
                            Orient orn;
                            anim_params.time = time;
                            if (!abon.orn(orn, anim_params))
                                orn = GetAnimOrient(child_sbon, sbon); // calculate before adding
                            AnimKeys::Orn &key = abon.orns.NewAt(index);
                            key.time = time;
                            key.orn = orn;
                            ASSERT(!HAS_ANIM_TANGENT);
                        }
                    }

                    Int start, end;
                    if (all_keys) {
                        start = 0;
                        end = abon.orns.elms();
                    } else {
                        abon.orns.binarySearch(prev_time, start, Compare);
                        if (abon.orns.binarySearch(next_time, end, Compare))
                            end++; // end is exclusive, so if found match, process it too, in case key is at the end
                    }
                    for (Int i = start; i < end; i++) // process keys from start to end
                    {
                        AnimKeys::Orn &orn = abon.orns[i];
                        anim_params.time = orn.time;
                        Orient src_orn;
                        if (!src.orn(src_orn, anim_params))
                            src_orn = default_orn;
                        Orient cur_orn;
                        if (!cur.orn(cur_orn, anim_params))
                            cur_orn = default_orn;

                        // orn * src_orn           = orn1 * cur_orn
                        // orn * src_orn / cur_orn = orn1

                        orn.orn.mul(src_orn, true);
                        orn.orn.div(cur_orn, true);
                    }

                    abon.setTangents(anim.loop(), anim.length());
                }
            }

            if (pos) {
                Matrix3 sbon_matrix;
                if (!root) {
                    sbon_matrix = *sbon;
                    abon = anim.findBone(sbon->name, sbon->type, sbon->type_index, sbon->type_sub);
                    keys = abon;
                }
                if (keys) { // orientation is already included in 'times'
                    if (all_keys)
                        keys->includeTimes(null, times, times);
                    else
                        keys->includeTimes(null, times, times, prev_time, next_time);
                }
                Matrix3 default_orn_m = default_orn;
                REPA(skel.bones) {
                    C SkelBone &child_sbon = skel.bones[i];
                    if (child_sbon.parent == bone_index) // 'skel_bone' child
                    {
                        AnimBone &abon = anim.getBone(child_sbon.name, child_sbon.type, child_sbon.type_index, child_sbon.type_sub); // 'skel_bone' child

                        Memt<Flt, 16384> child_times = times; // include child orientation keys for higher precision
                        if (all_keys)
                            abon.includeTimes(child_times, null, null);
                        else
                            abon.includeTimes(child_times, null, null, prev_time, next_time);

                        // add keys (Warning: TODO: this is precise only for linear interpolation, for other case we would have to copy 'abon' to temporary and calculate from it)
                        FREPA(child_times) // process forward because keys are sorted
                        {
                            Flt time = child_times[i];
                            Int index;
                            if (!abon.poss.binarySearch(time, index, CompareEps)) {
                                Vec pos;
                                anim_params.time = time;
                                if (!abon.pos(pos, anim_params) && !SET_ON_FAIL)
                                    pos.zero(); // calculate before adding
                                AnimKeys::Pos &key = abon.poss.NewAt(index);
                                key.time = time;
                                key.pos = pos;
                                ASSERT(!HAS_ANIM_TANGENT);
                            }
                        }

                        Int start, end;
                        if (all_keys) {
                            start = 0;
                            end = abon.poss.elms();
                        } else {
                            abon.poss.binarySearch(prev_time, start, Compare);
                            if (abon.poss.binarySearch(next_time, end, Compare))
                                end++; // end is exclusive, so if found match, process it too, in case key is at the end
                        }

                        Vec rel = child_sbon.pos;
                        if (sbon) // child position relative to parent
                        {
                            rel -= sbon->pos;
                            rel.divNormalized(sbon_matrix); // convert to parent space
                        }

                        for (Int i = start; i < end; i++) // process keys from start to end
                        {
                            AnimKeys::Pos &pos = abon.poss[i];
                            anim_params.time = pos.time;
                            Matrix3 src_matrix;
                            src.matrix(src_matrix, anim_params, default_orn_m);
                            Matrix3 cur_matrix;
                            cur.matrix(cur_matrix, anim_params, default_orn_m);

                            // (rel+pos)*src_matrix = (rel+pos1)*cur_matrix
                            // (rel+pos)*src_matrix/cur_matrix = rel+pos1
                            // (rel+pos)*src_matrix/cur_matrix - rel = pos1

                            pos.pos += rel;
                            pos.pos.mul(src_matrix);
                            pos.pos.div(cur_matrix);
                            pos.pos -= rel;
                        }

                        abon.setTangents(anim.loop(), anim.length());
                    }
                }
            }
        }
    }
    PROFILE_STOP("CatzEngine::FreezeRot(Animation &anim, C Skeleton &skel, Int skel_bone, Int key_index, C Matrix3 *matrix, Bool pos)")
}
void Animation::freezeDelPos(C Skeleton &skel, Int skel_bone, Int key_index) { FreezePos(T, skel, skel_bone, key_index, null); }
void Animation::freezeMove(C Skeleton &skel, Int skel_bone, Int key_index, C Vec &delta) { FreezePos(T, skel, skel_bone, key_index, &delta); }
void Animation::freezeDelRot(C Skeleton &skel, Int skel_bone, Int key_index, Bool pos) { FreezeRot(T, skel, skel_bone, key_index, null, pos); }
void Animation::freezeRotate(C Skeleton &skel, Int skel_bone, Int key_index, C Matrix3 &matrix, Bool pos) { FreezeRot(T, skel, skel_bone, key_index, &matrix, pos); }
/******************************************************************************/
Animation &Animation::setBoneTypeIndexesFromSkeleton(C Skeleton &skeleton) {
    PROFILE_START("CatzEngine::Animation::setBoneTypeIndexesFromSkeleton(C Skeleton &skeleton)")
    enum MODE {
        KEEP,
        CLEAR,
        REMOVE,
    } mode = REMOVE;

    REPA(bones) {
        AnimBone &bone = bones[i];
        if (C SkelBone *sbon = skeleton.findBone(bone.name))
            bone.id() = *sbon;
        else
            switch (mode) // search only by name because we assume that types/indexes are invalid and need to be set
            {
            case REMOVE:
                bones.remove(i, true);
                break;
            case CLEAR:
                bone.set(bone.name);
                break; // clear to BONE_UNKNOWN
            }
    }
    PROFILE_STOP("CatzEngine::Animation::setBoneTypeIndexesFromSkeleton(C Skeleton &skeleton)")
    return T;
}
Bool Animation::setBoneNameTypeIndexesFromSkeleton(C Skeleton &skeleton) {
    PROFILE_START("CatzEngine::Animation::setBoneNameTypeIndexesFromSkeleton(C Skeleton &skeleton)")
    Bool changed = false;
    REPA(bones) {
        AnimBone &bone = bones[i];
        if (C SkelBone *sbon = skeleton.findBone(bone.name, bone.type, bone.type_index, bone.type_sub)) // this should use names/types/indexes, because this method is called when we're changing animation from one skeleton to another
        {
            if (bone.id() != sbon->id()) {
                bone.id() = *sbon;
                changed = true;
            }
        } else {
            bones.remove(i, true);
            changed = true;
        }
    }
    PROFILE_STOP("CatzEngine::Animation::setBoneNameTypeIndexesFromSkeleton(C Skeleton &skeleton)")
    return changed;
}
/******************************************************************************/
Animation &Animation::reverse() {
    PROFILE_START("CatzEngine::Animation::reverse()")
    keys.reverse(length());
    REPAO(bones).reverse(length());

    REPA(events) {
        AnimEvent &event = events[i];
        event.time = length() - event.time;
    }
    events.reverseOrder();

    PROFILE_STOP("CatzEngine::Animation::reverse()")
    return setRootMatrix();
}
Animation &Animation::sortEvents() {
    PROFILE_START("CatzEngine::Animation::sortEvents()")
    events.sort(Compare);
    PROFILE_STOP("CatzEngine::Animation::sortEvents()")
    return T;
}
Animation &Animation::sortFrames() {
    PROFILE_START("CatzEngine::Animation::sortFrames()")
    keys.sortFrames();
    REPAO(bones).sortFrames();
    PROFILE_STOP("CatzEngine::Animation::sortFrames()")
    return setRootMatrix();
}
Animation &Animation::setTangents() {
    PROFILE_START("CatzEngine::Animation::setTangents()")
    keys.setTangents(loop(), length());
    REPAO(bones).setTangents(loop(), length());
    PROFILE_STOP("CatzEngine::Animation::setTangents()")
    return T;
}
Animation &Animation::optimize(Flt angle_eps, Flt pos_eps, Flt scale_eps, Bool remove_unused_bones) {
    PROFILE_START("CatzEngine::Animation::optimize(Flt angle_eps, Flt pos_eps, Flt scale_eps, Bool remove_unused_bones)")
    keys.optimize(loop(), linear(), length(), angle_eps, pos_eps, scale_eps, &NoTemp(Orient(Vec(0, 0, 1), Vec(0, 1, 0)))); // we can remove default orientation for root
    REPAO(bones).optimize(loop(), linear(), length(), angle_eps, pos_eps, scale_eps);
    if (remove_unused_bones)
        removeUnused();
    setRootMatrix();
    PROFILE_STOP("CatzEngine::Animation::optimize(Flt angle_eps, Flt pos_eps, Flt scale_eps, Bool remove_unused_bones)")
    return T;
}
Animation &Animation::clip(Flt start_time, Flt end_time, Bool remove_unused_bones) {
    PROFILE_START("CatzEngine::Animation::clip(Flt start_time, Flt end_time, Bool remove_unused_bones)")
    // if(start_time!=0 || end_time!=length()) don't check this, because we may have keyframe data outside of this range that needs to be removed
    {
        keys.clip(loop(), linear(), length(), start_time, end_time);
        REPAO(bones).clip(loop(), linear(), length(), start_time, end_time);
        if (remove_unused_bones)
            removeUnused();

        _length = Abs(end_time - start_time);

        Flt start_eps_m = start_time - EPS, end_eps_m = end_time - EPS,
            start_eps_p = start_time + EPS, end_eps_p = end_time + EPS;
        REPA(events) {
            Flt &time = events[i].time;
            if (time > end_eps_p || time < start_eps_m)
                events.remove(i, true);
            else if (time >= end_eps_m)
                time = _length;
            else if (time <= start_eps_p)
                time = 0;
            else
                time -= start_time;
        }
        setRootMatrix();
    }
    PROFILE_STOP("CatzEngine::Animation::clip(Flt start_time, Flt end_time, Bool remove_unused_bones)")
    return T;
}
Bool Animation::timeRange(Flt &min, Flt &max) C {
    PROFILE_START("CatzEngine::Animation::timeRange(Flt &min, Flt &max)")
    min = FLT_MAX;
    max = -FLT_MAX;
    Flt s, e;
    if (keys.timeRange(s, e)) {
        MIN(min, s);
        MAX(max, e);
    }
    REPA(bones)
    if (bones[i].timeRange(s, e)) {
        MIN(min, s);
        MAX(max, e);
    }
    REPA(events) {
        Flt t = events[i].time;
        MIN(min, t);
        MAX(max, t);
    }
    if (min <= max) {
        PROFILE_STOP("CatzEngine::Animation::timeRange(Flt &min, Flt &max)")
        return true;
    }
    min = max = 0;
    PROFILE_STOP("CatzEngine::Animation::timeRange(Flt &min, Flt &max)")
    return false;
}
Animation &Animation::clipAuto() {
    PROFILE_START("CatzEngine::Animation::clipAuto()")
    Flt min, max;
    timeRange(min, max);
    PROFILE_STOP("CatzEngine::Animation::clipAuto()")
    return clip(min, max);
}
Animation &Animation::maximizeLength() {
    PROFILE_START("CatzEngine::Animation::maximizeLength()")
    Flt min, max;
    timeRange(min, max);
    PROFILE_STOP("CatzEngine::Animation::maximizeLength()")
    return length(Max(length(), max), false);
}
Animation &Animation::offsetTime(Flt dt) {
    PROFILE_START("CatzEngine::Animation::offsetTime(Flt dt)")
    if (Flt time = Frac(-dt, length())) {
        if (1) {
            AnimParams params(T, time);
            Matrix cur, transform, transform2, start = (1 ? MatrixIdentity : rootStart());
            keys.matrix(cur, params);
            GetTransform(transform, cur, start);
            _root_start_inv.mul(rootEnd(), transform2); // this is "GetTransform(transform2, rootStart(), rootEnd())" can't just copy from 'rootTransform' because that's in 'RevMatrix' format, this converts from root start space to root end space
            transform2 *= transform;                    // this will additionally convert from 'cur' to 'start' space
#if DEBUG && 0
#pragma message("!! Warning: Use only for testing !!")
            AnimKeys keys;
            keys.poss.setNum(100);
            keys.orns.setNum(100);
            FREPA(keys.poss) {
                AnimKeys::Pos &pos = keys.poss[i];
                AnimKeys::Orn &orn = keys.orns[i];
                pos.time = orn.time = Lerp(0.0f, length(), i / 99.0f);
                Matrix m;
                getRootMatrixCumulative(m, pos.time + time);
                m *= transform;
                pos.pos = m.pos;
                orn.orn = m.orn();
            }
            Swap(T.keys, keys);
#else
            if (keys.orns.elms()) {
                Memc<AnimKeys::Orn> orns;
                orns.reserve(keys.orns.elms());
                Flt time_start = time + EPS, time_end = time + length() - EPS;
                {
                    AnimKeys::Orn &orn = orns.New();
                    orn.time = 0;
                    orn.orn = start.orn();
                    orn.orn.fix();
                } // add first key
                FREPA(keys.orns) // add keys after 'time'
                {
                    C AnimKeys::Orn &src = keys.orns[i];
                    if (src.time > time_start && src.time < time_end) {
                        time_start = src.time + EPS;
                        AnimKeys::Orn &orn = orns.New();
                        orn.time = src.time - time;
                        orn.orn = src.orn * transform.orn();
                        orn.orn.fix();
                    }
                }
                time_start -= length();
                time_end -= length();
                FREPA(keys.orns) // add keys before 'time'
                {
                    C AnimKeys::Orn &src = keys.orns[i];
                    if (src.time > time_start && src.time < time_end) {
                        time_start = src.time + EPS;
                        AnimKeys::Orn &orn = orns.New();
                        orn.time = src.time - time + length();
                        orn.orn = src.orn * transform2.orn();
                        orn.orn.fix();
                    }
                }
                {
                    AnimKeys::Orn &orn = orns.New();
                    orn.time = length();
                    orn.orn = cur.orn() * transform2.orn();
                    orn.orn.fix();
                } // add last key
                keys.orns = orns;
            }
            if (keys.poss.elms()) {
                Memc<AnimKeys::Pos> poss;
                poss.reserve(keys.poss.elms());
                Flt time_start = time + EPS, time_end = time + length() - EPS;
                {
                    AnimKeys::Pos &pos = poss.New();
                    pos.time = 0;
                    pos.pos = start.pos;
                } // add first key
                FREPA(keys.poss) // add keys after 'time'
                {
                    C AnimKeys::Pos &src = keys.poss[i];
                    if (src.time > time_start && src.time < time_end) {
                        time_start = src.time + EPS;
                        AnimKeys::Pos &pos = poss.New();
                        pos.time = src.time - time;
                        pos.pos = src.pos * transform;
                    }
                }
                time_start -= length();
                time_end -= length();
                FREPA(keys.poss) // add keys before 'time'
                {
                    C AnimKeys::Pos &src = keys.poss[i];
                    if (src.time > time_start && src.time < time_end) {
                        time_start = src.time + EPS;
                        AnimKeys::Pos &pos = poss.New();
                        pos.time = src.time - time + length();
                        pos.pos = src.pos * transform2;
                    }
                }
                {
                    AnimKeys::Pos &pos = poss.New();
                    pos.time = length();
                    pos.pos = cur.pos * transform2;
                } // add last key
                keys.poss = poss;
            }
#endif
            // TODO: Warning: here scale is ignored
            keys.setTangents(loop(), length());
        } else
            keys.offsetTime(dt, length());
        REPAO(bones).offsetTime(dt, length());
        REPAO(events).time = Frac(events[i].time + dt, length());
        sortEvents();
        setRootMatrix();
    }
    PROFILE_STOP("CatzEngine::Animation::offsetTime(Flt dt)")
    return T;
}
/******************************************************************************/
struct TimeScaleRange {
    Flt start_time, end_time, scale, delta, new_length, new_length_eps;

    TimeScaleRange(Flt start_time, Flt end_time, Flt scale, Flt old_length) {
        T.start_time = start_time;
        T.end_time = end_time;
        T.scale = scale;

        Flt old_duration = end_time - start_time,
            new_duration = old_duration * scale;

        T.delta = new_duration - old_duration;
        T.new_length = old_length + delta;
        T.new_length_eps = new_length - EPS;
    }
    void adjust(Flt &time) C {
        Flt t = time;

        if (t >= end_time)
            t += delta;
        else if (t > start_time)
            t = (t - start_time) * scale + start_time; // this could be optimized into a single MulAdd however most likely it would lose precision

        if (t > new_length_eps)
            t = new_length; // align to end, because looped root motion is calculated at time=0 and time=length, so if times are slightly offsetted, we could get root motion start that is actually end

        time = t;
    }
    void adjust(AnimEvent &event) C { adjust(event.time); }
    void adjust(AnimKeys &keys) C {
        REPA(keys.orns)
        adjust(keys.orns[i].time);
        REPA(keys.poss)
        adjust(keys.poss[i].time);
        REPA(keys.scales)
        adjust(keys.scales[i].time);
#if HAS_ANIM_ROT
        REPA(keys.rots)
        adjust(keys.rots[i].time);
#endif
#if HAS_ANIM_COLOR
        REPA(keys.colors)
        adjust(keys.colors[i].time);
#endif
    }
};
Animation &Animation::scaleTime(Flt start_time, Flt end_time, Flt scale) {
    PROFILE_START("CatzEngine::Animation::scaleTime(Flt start_time, Flt end_time, Flt scale)")
    if (scale != 1 && scale >= 0) {
        Clamp(start_time, 0, length());
        Clamp(end_time, 0, length());
        if (end_time > start_time) {
            TimeScaleRange ts(start_time, end_time, scale, length());

            ts.adjust(keys);
            REPA(bones)
            ts.adjust(bones[i]);
            REPA(events)
            ts.adjust(events[i]);

            length(ts.new_length, false);
        }
    }
    PROFILE_STOP("CatzEngine::Animation::scaleTime(Flt start_time, Flt end_time, Flt scale)")
    return T;
}
/******************************************************************************/
Animation &Animation::scale(Flt scale) {
    PROFILE_START("CatzEngine::Animation::scale(Flt scale)")
    keys.scale(scale);
    REPAO(bones).scale(scale);
    PROFILE_STOP("CatzEngine::Animation::scale(Flt scale)")
    return setRootMatrix();
}
Animation &Animation::mirrorX() {
    PROFILE_START("CatzEngine::Animation::mirrorX()")
    keys.mirrorX();
    REPAO(bones).mirrorX();
    PROFILE_STOP("CatzEngine::Animation::mirrorX()")
    return setRootMatrix();
}
Animation &Animation::transform(C Matrix &matrix, C Skeleton &source, Bool skel_const) {
    PROFILE_START("CatzEngine::Animation::transform(C Matrix &matrix, C Skeleton &source, Bool skel_const)")
    // root
    AnimParams anim_params(T, 0);
    // AnimKeys   old=keys;
    if (keys.orns.elms()) {
        MatrixD3 matrix_dn = matrix, matrix_dn_inv;
        matrix_dn.normalize();
        matrix_dn.inverse(matrix_dn_inv, true);
        REPA(keys.orns) {
            Orient &orn = keys.orns[i].orn;
            orn = matrix_dn_inv * MatrixD3(orn) * matrix_dn;
        }
    }
    if (keys.orns.elms() && matrix.pos.any()) // if there are any orientations and matrix position offset
    {
        Memt<Flt, 16384> times;
        keys.includeTimes(times, times, null); // we need to setup position keys from all orientation/position times
        Mems<AnimKeys::Pos> poss;
        poss.setNumDiscard(times.elms()); // work on temporary container
        REPA(poss) {
            AnimKeys::Pos &pos = poss[i];
            anim_params.time = pos.time = times[i];
            Orient orn;
            keys.orn(orn, anim_params);
            if (!keys.pos(pos.pos, anim_params) && !SET_ON_FAIL)
                pos.pos.zero();
            pos.pos *= matrix.orn();
            pos.pos += matrix.pos - matrix.pos * orn;
        }
        Swap(keys.poss, poss); // when finished, swap temp with actual
    } else {
        REPAO(keys.poss).pos *= matrix.orn();
    }
#if HAS_ANIM_ROT
    ? ?
#endif

      // bones
        Flt scale = matrix.avgScale();
    Matrix3 matrix_n = matrix;
    matrix_n.normalize();
    if (skel_const) // if we're not changing the skeleton (if skeleton is changing, then setting key is not needed, as they're updated dynamically during the animation process based on current skeleton bone orientation)
    {
        Bool set_orn = !Equal(MatrixIdentity.orn(), matrix_n), set_pos = !Equal(MatrixIdentity, matrix);
        if (set_orn || set_pos) // if we have to set oriention/pos keys
            FREPA(source.bones) // iterate all skeleton bones
            {
                C SkelBone &bone = source.bones[i];
                if (bone.parent == BONE_NULL) // process only main bones
                {
                    AnimBone &abon = getBone(bone.name, bone.type, bone.type_index, bone.type_sub);
                    if (set_orn && !abon.orns.elms()) // if there are no orn keys
                    {
                        AnimKeys::Orn &orn = abon.orns.New();
                        orn.time = 0;
                        orn.orn = bone;
                    }
                    if (set_pos && !abon.poss.elms()) // if there are no pos keys
                    {
                        AnimKeys::Pos &pos = abon.poss.New();
                        pos.time = 0;
                        pos.pos.zero();
                    }
                }
            }
    }
    REPA(bones) {
        AnimBone &abon = bones[i];
#if FIND_ANIM_BY_NAME_ONLY
        Int sbon_index = source.findBoneI(abon.name);
#else
        Int sbon_index = source.findBoneI(abon.name, abon.type, abon.type_index, abon.type_sub);
#endif
        Bool main = (sbon_index >= 0 && source.bones[sbon_index].parent == BONE_NULL);
        if (main) {
#if 1 // optimized
            REPAO(abon.orns).orn.mul(matrix_n, true);
#else // full formula
            REPA(abon.orns) {
                AnimKeys::Orn &orn = abon.orns[i];
                anim_params.time = orn.time;
                Orient old_orn, new_orn;
                // orn.orn=orn.orn*old_orn*matrix_n/new_orn;
                if (old.orn(old_orn, anim_params))
                    orn.orn.mul(old_orn, true);
                orn.orn.mul(matrix_n, true);
                if (keys.orn(new_orn, anim_params))
                    orn.orn.div(new_orn, true);
            }
#endif
#if HAS_ANIM_ROT
            REPAO(abon.rots).rot *= matrix_n;
#endif

            // position
#if 1                       // optimized
            if (skel_const) // as explained below, the more complex formula has to be done only for ANIM_TRANSFORM_SKEL_CONST, because for other cases, 'temp.pos' is zero
            {
                Vec bone = source.bones[sbon_index].pos;
                Matrix temp(matrix.orn(), bone * matrix - bone);
                REPAO(abon.poss).pos *= temp;
            } else
                REPAO(abon.poss).pos *= matrix.orn();
#else // full formula
            Vec bone = source.bones[sbon_index].pos, old_bone, new_bone;
            switch (mode) {
            case ANIM_TRANSFORM_SKEL_ALREADY_TRANSFORMED: {
                new_bone = bone;
                old_bone = bone;
                old_bone -= matrix.pos;
                old_bone.divNormalized(matrix_n); // old_bone=bone/matrix
            } break;

            case ANIM_TRANSFORM_SKEL_WILL_BE_TRANSFORMED: {
                old_bone = bone;
                new_bone = bone;
                new_bone.mul(matrix_n) += matrix.pos; // new_bone=bone*matrix
            } break;

            default:
                new_bone = old_bone = bone;
                break; // ANIM_TRANSFORM_SKEL_CONST
            }
            Memt<Flt, 16384> times, times2;
            keys.includeTimes(times, times, null);
            Mems<AnimKeys::Pos> poss; // reuse these from the top
            times2 = times;
            abon.includeTimes(null, times2, null);
            poss.setNumDiscard(times2.elms());
            REPA(poss) {
                AnimKeys::Pos &pos = poss[i];
                pos.time = anim_params.time = times2[i];
                Vec bone_offset;
                if (!abon.pos(bone_offset, anim_params) && !SET_ON_FAIL)
                    bone_offset.zero();
                Matrix old_root;
                old.matrix(old_root, anim_params);
                Matrix new_root;
                keys.matrix(new_root, anim_params);
                DEBUG_ASSERT(Equal(old_root * matrix / new_root, matrix), "matrixes should be equal");
                DEBUG_ASSERT(Equal((old_bone + bone_offset) * old_root * matrix / new_root - new_bone, (old_bone + bone_offset) * matrix - new_bone), "vectors should be equal");
                pos.pos = (old_bone + bone_offset) * old_root * matrix / new_root - new_bone; // Vec global=(old_bone+bone_offset)*old_root; "old_root*matrix/new_root"=matrix
                                                                                              // pos=(old_bone+bone_offset)*matrix-new_bone;
                                                                                              // pos=old_bone*matrix.orn() + bone_offset*matrix.orn() + matrix.pos - new_bone; multiplied by matrix
                                                                                              // pos=bone_offset*matrix.orn() + old_bone*matrix.orn() + matrix.pos - new_bone;
                                                                                              // pos=bone_offset*matrix.orn() + old_bone*matrix                    - new_bone;
                // ANIM_TRANSFORM_SKEL_ALREADY_TRANSFORMED: (bone/matrix)*matrix - bone        == 0; old_bone=bone/matrix; new_bone=bone
                // ANIM_TRANSFORM_SKEL_WILL_BE_TRANSFORMED:       bone   *matrix - bone*matrix == 0; old_bone=bone       ; new_bone=bone*matrix
                //  in both cases above, the formula can get simplified
            }
            Swap(abon.poss, poss);
#endif
        } else
            REPAO(abon.poss).pos *= scale; // for sub bones we can use only uniform scale
    }
    PROFILE_STOP("CatzEngine::Animation::transform(C Matrix &matrix, C Skeleton &source, Bool skel_const)")
    return setTangents().setRootMatrix();
}
Animation &Animation::rightToLeft(C Skeleton &source) // this method can ignore name differences because it's used only during importing while the names are the same
{
    return transform(Matrix3().setRotateX(-PI_2), source, false).mirrorX(); // set 'skel_const'=false because this method is called only during import, in which cases the skeleton is always going to get transformed too
}
static Str BoneNeutralName(C Str &name) {
    PROFILE_START("CatzEngine::BoneNeutralName(C Str &name)")
    Str n = Replace(name, "right", CharAlpha);
    n = Replace(n, "left", CharAlpha);
    n.replace('r', CharBeta).replace('l', CharBeta).replace('R', CharBeta).replace('L', CharBeta);
    PROFILE_STOP("CatzEngine::BoneNeutralName(C Str &name)")
    return n;
}
Animation &Animation::mirror(C Skeleton &source) {
    PROFILE_START("CatzEngine::Animation::mirror(C Skeleton &source)")
    MemtN<Bool, 256> bone_remapped;
    bone_remapped.setNumZero(bones.elms());
    REPA(bones)
    if (!bone_remapped[i]) {
        bone_remapped[i] = true;
        AnimBone &abon = bones[i];
        Str name = BoneNeutralName(abon.name);
        REPAD(s, source.bones) {
            C SkelBone &sbon = source.bones[s];
            if (!Equal(abon.name, sbon.name) && name == BoneNeutralName(sbon.name)) {
                REPD(j, i)
                if (!bone_remapped[j] && Equal(bones[j].name, sbon.name)) {
                    bone_remapped[j] = true;
                    Swap(abon.id(), bones[j].id());
                    goto finished;
                }
                abon.id() = sbon;
                break;
            }
        }
    finished:;
    }
    PROFILE_STOP("CatzEngine::Animation::mirror(C Skeleton &source)")
    return mirrorX();
}
/******************************************************************************/
#if HAS_ANIM_ROT
Animation &Animation::convertRotToOrn(C Skeleton &skeleton) // this method can ignore name differences because it's used only during importing while the names are the same
{
    PROFILE_START("CatzEngine::Animation::convertRotToOrn(C Skeleton &skeleton)")
    SkelAnim skel_anim(skeleton, T);
    keys.convertRotToOrn(skeleton, -1, loop(), length());
    REPAO(bones).convertRotToOrn(skeleton, skel_anim.abonToSbon(i), loop(), length());
    PROFILE_STOP("CatzEngine::Animation::convertRotToOrn(C Skeleton &skeleton)")
    return T;
}
Animation &Animation::convertOrnToRot(C Skeleton &skeleton) // this method can ignore name differences because it's used only during importing while the names are the same
{
    PROFILE_START("CatzEngine::Animation::convertOrnToRot(C Skeleton &skeleton)")
    SkelAnim skel_anim(skeleton, T);
    keys.convertOrnToRot(skeleton, -1, loop(), length());
    REPAO(bones).convertOrnToRot(skeleton, skel_anim.abonToSbon(i), loop(), length());
    PROFILE_STOP("CatzEngine::Animation::convertOrnToRot(C Skeleton &skeleton)")
    return T;
}
#endif
/******************************************************************************/
void Animation::includeTimesForBoneAndItsParents(C Skeleton &skel, Int skel_bone, MemPtr<Flt, 16384> orn_times, MemPtr<Flt, 16384> pos_times, MemPtr<Flt, 16384> scale_times) C {
    PROFILE_START("CatzEngine::Animation::includeTimesForBoneAndItsParents(C Skeleton &skel, Int skel_bone, MemPtr<Flt, 16384> orn_times, MemPtr<Flt, 16384> pos_times, MemPtr<Flt, 16384> scale_times)")
    if (InRange(skel_bone, skel.bones))
        for (;;) {
            C SkelBone &sbon = skel.bones[skel_bone];
#if FIND_ANIM_BY_NAME_ONLY
            if (C AnimBone *abon = findBone(sbon.name))
#else
            if (C AnimBone *abon = findBone(sbon.name, sbon.type, sbon.type_index, sbon.type_sub))
#endif
                abon->includeTimes(orn_times, pos_times, scale_times);
            if (sbon.parent >= skel_bone)
                break; // proceed only if the parent has a smaller index (this solves the issue of never ending loops with incorrect data)
            skel_bone = sbon.parent;
        }
    keys.includeTimes(orn_times, pos_times, scale_times);
    PROFILE_STOP("CatzEngine::Animation::includeTimesForBoneAndItsParents(C Skeleton &skel, Int skel_bone, MemPtr<Flt, 16384> orn_times, MemPtr<Flt, 16384> pos_times, MemPtr<Flt, 16384> scale_times)")
}
/******************************************************************************/
Animation &Animation::copyParams(C Animation &src) {
    T._length = src._length;
    T._flag = src._flag;
    return T;
}
/******************************************************************************/
void Animation::freezeBone(C Skeleton &skel, Int skel_bone) {
    PROFILE_START("CatzEngine::Animation::freezeBone(C Skeleton &skel, Int skel_bone)")
    if (InRange(skel_bone, skel.bones)) {
        SkelAnim skel_anim(skel, T);
        AnimSkel anim_skel;
        anim_skel.create(&skel);
        C SkelBone &bone_sel = skel.bones[skel_bone];
        C AnimSkelBone &abone_sel = anim_skel.bones[skel_bone];
        Memt<Flt, 16384> bone_times;
        includeTimesForBoneAndItsParents(skel, skel_bone, bone_times, bone_times, bone_times);
        anim_skel.clear().animateEx(skel_anim, 0).updateMatrix(); // animate first to get the 'start_pos'
        Vec start_pos = bone_sel.pos * abone_sel.matrix();

        // get root bones
        MemtN<BoneType, 32> root_bones;
        REPA(skel.bones)
        if (skel.bones[i].parent == BONE_NULL)
            root_bones.add(i);
        MemtN<AnimKeys, 32> root_bone_keys;
        root_bone_keys.setNumDiscard(root_bones.elms());
        Memt<Flt, 16384> root_times; // keep outside the loop to reduce overhead
        REPAD(rb, root_bones)        // iterate all root bones
        {
            BoneType iroot = root_bones[rb];
            C SkelBone &root = skel.bones[iroot];
            C AnimSkelBone &aroot = anim_skel.bones[iroot];
            AnimKeys &keys = root_bone_keys[rb];
            root_times = bone_times;
#if FIND_ANIM_BY_NAME_ONLY
            if (AnimBone *abon = findBone(root.name))
#else
            if (AnimBone *abon = findBone(root.name, root.type, root.type_index, root.type_sub))
#endif
                abon->includeTimes(null, root_times, null);
            keys.poss.setNumDiscard(root_times.elms());
            FREPAD(t, root_times) // irerate all frames
            {
                Flt time = root_times[t];
                anim_skel.clear().animateEx(skel_anim, time).updateMatrix();
                Vec cur_pos = bone_sel.pos * abone_sel.matrix(), delta = cur_pos - start_pos;
                keys.poss[t].time = time;
                keys.poss[t].pos = aroot.pos - delta;
            }
            keys.setTangents(loop(), length()).optimize(loop(), linear(), length());
        }

        // once all is ready, we need to store results in this animation, this can't be done before, because that would affect results of animating the skeleton
        REPAD(rb, root_bones) {
            BoneType iroot = root_bones[rb];
            C SkelBone &root = skel.bones[iroot];
            AnimKeys &keys = root_bone_keys[rb];
            Swap(getBone(root.name, root.type, root.type_index, root.type_sub).poss, keys.poss); // swap only positions
        }
    }
    PROFILE_STOP("CatzEngine::Animation::freezeBone(C Skeleton &skel, Int skel_bone)")
}
/******************************************************************************/
Bool Animation::save(File &f) C {
    f.putMulti(UInt(CC4_ANIM), Byte(11), _flag, _length, _root_start, _root_end); // version
    if (keys.saveData(f)) {
        f.cmpUIntV(bones.elms());
        FREPA(bones) {
            C AnimBone &abon = bones[i];
            f.putStr(abon.name).putMulti(abon.type, abon.type_index, abon.type_sub);
            if (!abon.saveData(f))
                return false;
        }
        if (events.saveRaw(f))
            return f.ok();
    }
    return false;
}
Bool Animation::load(File &f) {
    if (f.getUInt() == CC4_ANIM)
        switch (f.decUIntV()) // version
        {
        case 11: {
            f.getMulti(_flag, _length, _root_start, _root_end);
            setRootMatrix2();
            if (!keys.loadData(f))
                goto error;
            bones.setNum(f.decUIntV());
            FREPA(bones) {
                AnimBone &abon = bones[i];
                f.getStr(abon.name).getMulti(abon.type, abon.type_index, abon.type_sub);
                if (!abon.loadData(f))
                    goto error;
            }
            if (!events.loadRaw(f))
                goto error;
            if (f.ok())
                return true;
        } break;

        case 10: {
            f.getMulti(_flag, _length, _root_start, _root_end);
            setRootMatrix2();
            if (!keys.loadData(f))
                goto error;
            bones.setNum(f.decUIntV());
            FREPA(bones) {
                AnimBone &abon = bones[i];
                f.getStr(abon.name).getMulti(abon.type, abon.type_index, abon.type_sub);
                if (!abon.loadData(f))
                    goto error;
            }
            if (!events.loadRaw(f))
                goto error;
            sortEvents();
            if (f.ok())
                return true;
        } break;

        case 9: {
            f.getMulti(_flag, _length);
            if (!keys.loadData(f))
                goto error;
            bones.setNum(f.decUIntV());
            FREPA(bones) {
                AnimBone &abon = bones[i];
                f.getStr(abon.name).getMulti(abon.type, abon.type_index, abon.type_sub);
                if (!abon.loadData(f))
                    goto error;
            }
            if (!events.loadRaw(f))
                goto error;
            sortEvents();
            if (f.ok()) {
                setRootMatrix();
                return true;
            }
        } break;

        case 8: {
            f.getMulti(_flag, _length);
            if (!keys.loadData(f))
                goto error;
            bones.setNum(f.decUIntV());
            FREPA(bones) {
                AnimBone &abon = bones[i];
                f._getStr1(abon.name).getMulti(abon.type, abon.type_index, abon.type_sub);
                if (!abon.loadData(f))
                    goto error;
            }
            if (!events.loadRaw(f))
                goto error;
            sortEvents();
            if (f.ok()) {
                setRootMatrix();
                return true;
            }
        } break;

        case 7: {
            f >> _flag >> _length;
            keys.loadData3(f);
            bones.setNum(f.decUIntV());
            FREPA(bones) {
                AnimBone &abon = bones[i];
                f._getStr1(abon.name).getMulti(abon.type, abon.type_index, abon.type_sub);
                abon.loadData3(f);
            }
            if (!events.loadRaw(f))
                goto error;
            sortEvents();
            if (f.ok()) {
                setRootMatrix();
                return true;
            }
        } break;

        case 6: {
            f >> _flag >> _length;
            keys.loadData3(f);
            bones.setNum(f.decUIntV());
            FREPA(bones) {
                AnimBone &abon = bones[i];
                abon.set(null);
                f._getStr1(abon.name);
                abon.loadData3(f);
            }
            if (!events.loadRaw(f))
                goto error;
            sortEvents();
            if (f.ok()) {
                setRootMatrix();
                return true;
            }
        } break;

        case 5: {
            bones.setNum(f.getUShort());
            events.setNum(f.getUShort());
            _flag = f.getUShort();
            f >> _length;
            keys.loadData2(f);
            FREPA(bones) {
                AnimBone &abon = bones[i];
                abon.set(null);
                f >> abon.name;
                abon.loadData2(f);
            }
            f.getN(events.data(), events.elms());
            sortEvents();
            if (f.ok()) {
                setRootMatrix();
                return true;
            }
        } break;

        case 4: {
            bones.setNum(f.getUShort());
            events.setNum(f.getUShort());
            _flag = f.getUShort();
            f >> _length;
            keys.loadData1(f);
            FREPA(bones) {
                AnimBone &abon = bones[i];
                abon.set(null);
                f >> abon.name;
                abon.loadData1(f);
            }
            f.getN(events.data(), events.elms());
            sortEvents();
            if (f.ok()) {
                setRootMatrix();
                return true;
            }
        } break;

        case 3: {
            bones.setNum(f.getUShort());
            events.setNum(f.getUShort());
            _flag = f.getUShort();
            f >> _length;
            keys.loadData0(f);
            FREPA(bones) {
                AnimBone &abon = bones[i];
                abon.set(null);
                f >> abon.name;
                abon.loadData0(f);
            }
            f.getN(events.data(), events.elms());
            sortEvents();
            if (f.ok()) {
                setRootMatrix();
                return true;
            }
        } break;

        case 2: {
            bones.setNum(f.getUShort());
            events.setNum(f.getUShort());
            _flag = f.getUShort();
            f >> _length;
            FREPA(bones) {
                AnimBone &abon = bones[i];
                abon.set(null);
                abon.orns.setNumDiscard(f.getUShort());
#if HAS_ANIM_ROT
                Mems<AnimKeys::Rot> &rots = abon.rots.setNumDiscard(f.getUShort());
#else
                Int rots = f.getUShort();
#endif
                abon.poss.setNumDiscard(f.getUShort());
                f.get(abon.name, 16);
                ASSERT(ELMS(abon.name) >= 16);
                LoadOrnTan(f, abon.orns);
                LoadRotTan(f, rots);
                LoadPosTan(f, abon.poss);
            }
            FREPA(events) {
                AnimEvent &e = events[i];
                f.get(e.name, 16);
                ASSERT(ELMS(e.name) >= 16);
                f >> e.time;
            }
            sortEvents();
            if (f.ok()) {
                setRootMatrix();
                return true;
            }
        } break;

        case 1: {
            f.skip(1); // old version byte
            bones.setNum(f.getUShort());
            events.setNum(f.getUShort());
            _flag = f.getUShort();
            f >> _length;
            FREPA(bones) {
                AnimBone &abon = bones[i];
                abon.set(null);
                abon.orns.setNumDiscard(f.getUShort());
                abon.poss.setNumDiscard(f.getUShort());
                f.get(abon.name, 16);
                ASSERT(ELMS(abon.name) >= 16);
                LoadOrnTan(f, abon.orns);
                LoadPosTan(f, abon.poss);
            }
            FREPA(events) {
                AnimEvent &e = events[i];
                f.get(e.name, 16);
                ASSERT(ELMS(e.name) >= 16);
                f >> e.time;
            }
            sortEvents();
            if (f.ok()) {
                setRootMatrix();
                return true;
            }
        } break;

        case 0: {
            f.skip(1); // old version byte
            bones.setNum(f.getUShort());
            events.setNum(f.getUShort());
            _flag = f.getUShort();
            f >> _length;
            FREPA(bones) {
                AnimBone &abon = bones[i];
                abon.set(null);
                abon.orns.setNumDiscard(f.getUShort());
                f.get(abon.name, 16);
                ASSERT(ELMS(abon.name) >= 16);
                LoadOrnTan(f, abon.orns);
            }
            FREPA(events) {
                AnimEvent &e = events[i];
                f.get(e.name, 16);
                ASSERT(ELMS(e.name) >= 16);
                f >> e.time;
            }
            sortEvents();
            if (f.ok()) {
                setRootMatrix();
                return true;
            }
        } break;
        }
error:
    del();
    return false;
}
/******************************************************************************/
Bool Animation::save(C Str &name) C {
    File f;
    if (f.write(name)) {
        if (save(f) && f.flush())
            return true;
        f.del();
        FDelFile(name);
    }
    return false;
}
Bool Animation::load(C Str &name) {
    File f;
    if (f.read(name))
        return load(f);
    del();
    return false;
}
void Animation::operator=(C UID &id) { T = _EncodeFileName(id); }
void Animation::operator=(C Str &name) {
    if (!load(name))
        Exit(MLT(S + "Can't load Animation \"" + name + "\"",
                 PL, S + u"Nie można wczytać Animacji \"" + name + "\""));
}
/******************************************************************************/
void Animation::save(MemPtr<TextNode> nodes) C {
    nodes.New().set("Linear", linear());
    nodes.New().set("Loop", loop());
    nodes.New().set("Length", length());
    if (bones.elms()) {
        TextNode &node = nodes.New();
        node.name = "Bones";
        FREPAO(bones).save(node.nodes.New());
    }
    if (events.elms()) {
        TextNode &node = nodes.New();
        node.name = "Events";
        FREPAO(events).save(node.nodes.New());
    }
    if (keys.is()) {
        TextNode &node = nodes.New();
        node.name = "Keys";
        keys.save(node);
    }
}
/******************************************************************************/
void XAnimation::save(MemPtr<TextNode> nodes) C {
    nodes.New().set("Name", name);
    nodes.New().set("FPS", fps);
    nodes.New().set("Start", start);
    anim.save(nodes);
}
Bool XAnimation::save(File &f) C {
    f.cmpUIntV(0); // version
    f << fps << start << name;
    if (anim.save(f))
        return f.ok();
    return false;
}
Bool XAnimation::load(File &f) {
    switch (f.decUIntV()) // version
    {
    case 0: {
        f >> fps >> start >> name;
        if (anim.load(f))
            if (f.ok())
                return true;
    } break;
    }
    del();
    return false;
}
Bool XAnimation::save(C Str &name) C {
    File f;
    if (f.write(name)) {
        if (save(f) && f.flush())
            return true;
        f.del();
        FDelFile(name);
    }
    return false;
}
Bool XAnimation::load(C Str &name) {
    File f;
    if (f.read(name))
        return load(f);
    del();
    return false;
}
/******************************************************************************/
// SKELETON ANIMATION
/******************************************************************************/
void SkelAnim::zero() {
    _bone = null;
    _animation = null;
}
SkelAnim::SkelAnim() { zero(); }
SkelAnim::SkelAnim(C Skeleton &skeleton, C Animation &animation) : SkelAnim() { create(skeleton, animation); }
SkelAnim::SkelAnim(C SkelAnim &src) : SkelAnim() { T = src; }

void SkelAnim::operator=(C SkelAnim &src) {
    if (this != &src) {
        del();
        if (_animation = src._animation) {
            Int bones = _animation->bones.elms();
            Alloc(_bone, bones);
            CopyFastN(_bone, src._bone, bones);
        }
    }
}
SkelAnim &SkelAnim::del() {
    Free(_bone);
    zero();
    return T;
}
SkelAnim &SkelAnim::create(C Skeleton &skeleton, C Animation &animation) {
    T._animation = &animation;
    Alloc(Free(_bone), animation.bones.elms());
    REPA(animation.bones) {
        C AnimBone &abon = animation.bones[i];
        Int sbon = skeleton.findBoneI(abon.name, abon.type, abon.type_index, abon.type_sub); // here we should always use names/types/indexes because we may share animation between multiple skeletons
        _bone[i] = ((sbon >= 0) ? sbon : BONE_NULL);
    }
    return T;
}

Bool SkelAnim::load(C Str &name, Ptr user) {
    if (Animation *anim = Animations.get(name)) {
        create(*(Skeleton *)user, *anim);
        return true;
    }
    del();
    return false;
}
Int SkelAnim::sbonToAbon(Int sbon) C {
    if (_animation && sbon >= 0)
        REPA(_animation->bones)
    if (_bone[i] == sbon)
        return i;
    return -1;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
void ShutAnimation() {
    Skeletons.del();
    Animations.del();
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
