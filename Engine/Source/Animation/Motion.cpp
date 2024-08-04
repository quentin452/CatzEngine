/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
Motion &Motion::clear() {
    skel_anim = null;
    time = 0;
    blend = 0;

    time_prev = time_delta = 0;
    return T;
}
Motion &Motion::set(AnimatedSkeleton &anim_skel, C Str &animation) {
    clear();
    skel_anim = anim_skel.findSkelAnim(animation);
    return T;
}
Motion &Motion::set(AnimatedSkeleton &anim_skel, C UID &anim_id) {
    clear();
    skel_anim = anim_skel.findSkelAnim(anim_id);
    return T;
}
/******************************************************************************/
Bool Motion::eventAfter(CChar8 *name) C { return skel_anim ? skel_anim->eventAfter(name, time_prev) : false; }
Bool Motion::eventOccurred(CChar8 *name) C { return skel_anim ? skel_anim->eventOccurred(name, time_prev, time_delta) : false; }
Bool Motion::eventBetween(CChar8 *from, CChar8 *to) C { return skel_anim ? skel_anim->eventBetween(from, to, time_prev, time_delta) : false; }
Flt Motion::eventProgress(CChar8 *from, CChar8 *to) C { return skel_anim ? skel_anim->eventProgress(from, to, time) : 0; }

Bool Motion::eventAfter(Flt event_time) C { return skel_anim ? time >= event_time : false; }
Bool Motion::eventOccurred(Flt event_time) C { return skel_anim ? EventOccurred(event_time, time_prev, time_delta) : false; }
Bool Motion::eventBetween(Flt from, Flt to) C { return skel_anim ? EventBetween(from, to, time_prev, time_delta) : false; }
Flt Motion::eventProgress(Flt from, Flt to) C { return skel_anim ? LerpR(from, to, time) : 0; }
/******************************************************************************/
void Motion::getRootTransform(RevMatrix &transform) C {
    PROFILE_START("CatzEngine::Motion::getRootTransform(RevMatrix &transform)")
    if (skel_anim)
        if (C Animation *anim = skel_anim->animation()) {
            anim->getRootTransform(transform, time_prev, time);
            PROFILE_STOP("CatzEngine::Motion::getRootTransform(RevMatrix &transform)")
            return;
        }
    transform.identity();
    PROFILE_STOP("CatzEngine::Motion::getRootTransform(RevMatrix &transform)")
}
/******************************************************************************/
Bool Motion::updateAuto(Flt blend_in_speed, Flt blend_out_speed, Flt time_speed) {
    PROFILE_START("CatzEngine::Motion::updateAuto(Flt blend_in_speed, Flt blend_out_speed, Flt time_speed)")
    if (!is()) {
        PROFILE_STOP("CatzEngine::Motion::updateAuto(Flt blend_in_speed, Flt blend_out_speed, Flt time_speed)")
        return false; // return false if the motion isn't valid and can be deleted
    }

    if (time < skel_anim->length()) // if not finished
    {
        blend += blend_in_speed * Time.d(); // blend in
        if (blend >= 1)                     // if fully blended
        {
            blend = 1;
            time_prev = time;
            time_delta = time_speed * Time.d();
            time += time_delta;
        }
    } else // if finished
    {
        // prevent events at end of anim firing multiple times
        time_prev = time;
        time_delta = 0;

        blend -= blend_out_speed * Time.d(); // blend out
        if (blend <= 0)                      // if blended out
        {
            blend = 0;
            skel_anim = null;
            PROFILE_STOP("CatzEngine::Motion::updateAuto(Flt blend_in_speed, Flt blend_out_speed, Flt time_speed)")
            return false;
        }
    }
    PROFILE_STOP("CatzEngine::Motion::updateAuto(Flt blend_in_speed, Flt blend_out_speed, Flt time_speed)")
    return true;
}
Bool Motion::updateIn(Flt blend_in_speed, Flt time_speed) {
    PROFILE_START("CatzEngine::Motion::updateIn(Flt blend_in_speed, Flt time_speed)")
    if (!is()) {
        PROFILE_STOP("CatzEngine::Motion::updateIn(Flt blend_in_speed, Flt time_speed)")
        return false; // return false if the motion isn't valid and can be deleted
    }
    blend += blend_in_speed * Time.d(); // blend in
    if (blend >= 1)                     // if fully blended
    {
        blend = 1;
        time_prev = time;
        time_delta = time_speed * Time.d();
        time += time_delta;

        if (time >= skel_anim->length()) {
            PROFILE_STOP("CatzEngine::Motion::updateIn(Flt blend_in_speed, Flt time_speed)")
            return false; // if finished
        }
    }
    PROFILE_STOP("CatzEngine::Motion::updateIn(Flt blend_in_speed, Flt time_speed)")
    return true;
}
Bool Motion::updateOut(Flt blend_out_speed) {
    PROFILE_START("CatzEngine::Motion::updateOut(Flt blend_out_speed)")
    if (!is()) {
        PROFILE_STOP("CatzEngine::Motion::updateOut(Flt blend_out_speed)")
        return false; // return false if the motion isn't valid and can be deleted
    }
    // prevent events at end of anim firing multiple times
    time_prev = time;
    time_delta = 0;

    blend -= blend_out_speed * Time.d(); // blend out
    if (blend <= 0)                      // if blended out
    {
        blend = 0;
        skel_anim = null;
        PROFILE_STOP("CatzEngine::Motion::updateOut(Flt blend_out_speed)")
        return false;
    }
    PROFILE_STOP("CatzEngine::Motion::updateOut(Flt blend_out_speed)")
    return true;
}
/******************************************************************************/
Bool Motion::save(File &f) C {
    PROFILE_START("CatzEngine::Motion::save(File &f)")
    if (!skel_anim)
        f.cmpUIntV(0);
    else // version #0 specifies totally empty motion (which doesn't require additional data, and uses only 1 byte of storage)
    {
        f.cmpUIntV(1); // version
        f.putAsset(skel_anim->id());
        f.putMulti(blend, time, time_prev, time_delta);
    }
    PROFILE_STOP("CatzEngine::Motion::save(File &f)")
    return f.ok();
}
Bool Motion::load(File &f, C AnimatedSkeleton &anim_skel) {
    PROFILE_START("CatzEngine::Motion::load(File &f, C AnimatedSkeleton &anim_skel)")
    switch (f.decUIntV()) // version
    {
    case 0:
        clear();
        PROFILE_STOP("CatzEngine::Motion::load(File &f, C AnimatedSkeleton &anim_skel)")
        return f.ok();

    case 1: {
        skel_anim = anim_skel.findSkelAnim(f.getAssetID());
        f.getMulti(blend, time, time_prev, time_delta);
        if (f.ok()) {
            PROFILE_STOP("CatzEngine::Motion::load(File &f, C AnimatedSkeleton &anim_skel)")
            return true;
        }
    } break;
    }
    clear();
    PROFILE_STOP("CatzEngine::Motion::load(File &f, C AnimatedSkeleton &anim_skel)")
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
