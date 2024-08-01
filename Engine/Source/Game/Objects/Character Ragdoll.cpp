/******************************************************************************/
#include "stdafx.h"
namespace EE {
namespace Game {
/******************************************************************************/
void Chr::ragdollValidate() {
    PROFILE_START("CatzEngine::Chr::ragdollValidate()")
    if (!ragdoll.is()) // if ragdoll hasn't yet been created
    {
        ragdoll.create(skel, scale) // create from 'AnimatedSkeleton'
            .obj(this)              // set Game.Chr object
            .ignore(ctrl.actor);    // ignore collisions with the character controller
    }
    PROFILE_STOP("CatzEngine::Chr::ragdollValidate()")
}
/******************************************************************************/
void Chr::ragdollEnable() {
    PROFILE_START("CatzEngine::Chr::ragdollEnable()")
    if (ragdoll_mode != RAGDOLL_FULL) {
        ragdollValidate(); // make sure the ragdoll is created

        ragdoll.active(true)                   // enable ragdoll actors
            .gravity(true)                     // gravity should be enabled for full ragdoll mode
            .fromSkel(skel, ctrl.actor.vel()); // set ragdoll initial pose
        ctrl.actor.active(false);              // disable character controller completely, do this after setting ragdoll in case deactivating clears velocity which is used above

        ragdoll_mode = RAGDOLL_FULL;
    }
    PROFILE_STOP("CatzEngine::Chr::ragdollEnable()")
}
/******************************************************************************/
void Chr::ragdollDisable() {
    PROFILE_START("CatzEngine::Chr::ragdollDisable()")
    if (ragdoll_mode == RAGDOLL_FULL) {
        ragdoll.active(false);
        ctrl.actor.active(true);

        ragdoll_mode = RAGDOLL_NONE;
    }
    PROFILE_STOP("CatzEngine::Chr::ragdollDisable()")
}
/******************************************************************************/
Bool Chr::ragdollBlend() {
    PROFILE_START("CatzEngine::Chr::ragdollBlend()")
    if (ragdoll_mode != RAGDOLL_FULL) {
        ragdollValidate(); // make sure the ragdoll is created

        ragdoll.active(true)                   // enable ragdoll collisions
            .gravity(false)                    // disable gravity for hit-simulation effects because they look better this way
            .fromSkel(skel, ctrl.actor.vel()); // set ragdoll initial pose

        ragdoll_time = 0;
        ragdoll_mode = RAGDOLL_PART;
        PROFILE_STOP("CatzEngine::Chr::ragdollBlend()")
        return true; // ragdoll set successfully
    }
    PROFILE_STOP("CatzEngine::Chr::ragdollBlend()")
    return false; // can't set ragdoll mode
}
/******************************************************************************/
} // namespace Game
} // namespace EE
/******************************************************************************/
