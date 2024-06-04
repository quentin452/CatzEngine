#pragma once
/******************************************************************************/
#include "!Set Prec Struct.h"
struct CloudLayer {
    VecH4 color;
    Vec2 scale, position;
};

BUFFER(CloudLayer)
CloudLayer CL[4];

Vec2 LCRange;
Flt LCScaleY;
BUFFER_END
#include "!Set Prec Default.h"
/******************************************************************************/
Flt CloudAlpha(Flt y) { return y * 8 - 0.15; }
/******************************************************************************/
