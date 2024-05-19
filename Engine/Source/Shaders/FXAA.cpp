/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
// can use 'RTSize' instead of 'ImgSize' since there's no scale

#define FxaaTex Image

#define FxaaInt2 VecI2
#define FxaaTexTop(t, p) t.SampleLevel(SamplerLinearClamp, p, 0)
#define FxaaTexOff(t, p, o, r) t.SampleLevel(SamplerLinearClamp, p, 0, o)

#define FXAA_PC 1
#define FXAA_GREEN_AS_LUMA 1
#define FXAA_QUALITY__PRESET 12

#include "!Set Prec Default.h"
#include "!Set Prec Image.h"
#include "FXAA.h"

VecH4 FXAA_PS(NOPERSP Vec2 uv
              : UV) : TARGET {
    return FxaaPixelShader(uv, 0, Img, Img, Img, RTSize.xy, 0, 0, 0, 0.475, 0.15, 0.0833, 8.0, 0.125, 0.05, Vec4(1.0, -1.0, 0.25, -0.25));
}
/******************************************************************************/
