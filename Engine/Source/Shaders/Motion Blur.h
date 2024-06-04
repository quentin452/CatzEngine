#pragma once
/******************************************************************************/
#include "!Set Prec Struct.h"
BUFFER(MotionBlur)
Half MotionScale_2; // MotionScale/2 is used because we blur in both ways (read above why), so we have to make scale 2x smaller
BUFFER_END
#include "!Set Prec Default.h"
/******************************************************************************/
#define DEPTH_TOLERANCE 0.2 // 20 cm
/******************************************************************************/
