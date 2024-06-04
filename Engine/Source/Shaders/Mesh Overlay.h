#pragma once
/******************************************************************************/
#include "!Set Prec Struct.h"
struct OverlayClass {
    VecH4 param;
    Matrix mtrx;
};

BUFFER(Overlay)
OverlayClass OverlayParams;
BUFFER_END
#include "!Set Prec Default.h"

Half OverlayOpaqueFrac() { return OverlayParams.param.x; }
Half OverlayAlpha() { return OverlayParams.param.y; }
/******************************************************************************/
