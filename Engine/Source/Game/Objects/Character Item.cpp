/******************************************************************************/
#include "stdafx.h"
namespace EE {
namespace Game {
/******************************************************************************/
void Chr::grabStart(Item &item, C Vec &local_pos, Flt power) {
    PROFILE_START("CatzEngine::Chr::grabStart(Item &item, C Vec &local_pos, Flt power)")
    grabStop(); // stop any active grabbing

    if (!item.grabber()) // if not grabbed already
    {
        grab.create(item.actor, local_pos, power); // grab item
        item.grabber(this);                        // set item's grabber to character
    }
    PROFILE_STOP("CatzEngine::Chr::grabStart(Item &item, C Vec &local_pos, Flt power)")
}
/******************************************************************************/
void Chr::grabStop() {
    PROFILE_START("CatzEngine::Chr::grabStop()")
    if (grab.is()) // if grabbing something
    {
        if (Item *item = CAST(Item, (Game::Obj *)grab.grabbedActor()->obj())) // if it's an item
            item->grabber(null);                                              // clear grabber in the item

        grab.del(); // delete grab
    }
    PROFILE_STOP("CatzEngine::Chr::grabStop()")
}
/******************************************************************************/
} // namespace Game
} // namespace EE
/******************************************************************************/
