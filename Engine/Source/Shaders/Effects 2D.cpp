/******************************************************************************/
#include "!Header.h"
/******************************************************************************

   Keep here effects that are rarely used.

/******************************************************************************/
#include "!Set Prec Struct.h"
BUFFER(ColTrans)
Vec ColTransHsb;
MatrixH ColTransMatrix;
BUFFER_END
#include "!Set Prec Default.h"

VecH4 ColTrans_PS(NOPERSP Vec2 uv
                  : UV) : TARGET {
    return VecH4(Transform(Tex(Img, uv).rgb, ColTransMatrix), Step);
}
VecH4 ColTransHB_PS(NOPERSP Vec2 uv
                    : UV) : TARGET {
    VecH color = Tex(Img, uv).rgb;
    Half brightness = Max(color);
    color = Transform(color, ColTransMatrix);
    Half max = Max(color);
    if (max)
        color *= ColTransHsb.z * brightness / max;
    return VecH4(color, Step);
}
VecH4 ColTransHSB_PS(NOPERSP Vec2 uv
                     : UV) : TARGET {
    Vec hsb = RgbToHsb(Tex(Img, uv).rgb);
    return VecH4(HsbToRgb(Vec(hsb.x + ColTransHsb.x, hsb.y * ColTransHsb.y, hsb.z * ColTransHsb.z)), Step);
}
/******************************************************************************/
#include "!Set Prec Struct.h"
struct RippleClass {
    Flt xx, xy,
        yx, yy,
        stp,
        power,
        alpha_scale,
        alpha_add;
    Vec2 center;
};

BUFFER(Ripple)
RippleClass Rppl;
BUFFER_END
#include "!Set Prec Default.h"

VecH4 Ripple_PS(NOPERSP Vec2 uv
                : UV) : TARGET {
    uv += Vec2(Sin(uv.x * Rppl.xx + uv.y * Rppl.xy + Rppl.stp),
               Sin(uv.x * Rppl.yx + uv.y * Rppl.yy + Rppl.stp)) *
          Rppl.power;

    VecH4 c = Tex(Img, uv);
    c.a *= Sat((Rppl.alpha_scale * 2) * Length(uv - Rppl.center) + Rppl.alpha_add);
    return c;
}
/******************************************************************************/
#include "!Set Prec Struct.h"
struct TitlesClass {
    Flt stp;
    Flt center;
    Flt range;
    Flt smooth;
    Flt swirl;
};

BUFFER(Titles)
TitlesClass Titles;
BUFFER_END
#include "!Set Prec Default.h"

VecH4 Titles_PS(NOPERSP Vec2 uv
                : UV) : TARGET {
    Flt s = Sat((Abs(uv.y - Titles.center) - Titles.range) / Titles.smooth);
    uv.x += Sin(s * s * s * (PI * 4) + Titles.stp) * s * s * s * Titles.swirl;
    Int range = Round(s * 12);
    VecH4 color = 0;
    LOOP for (Int i = -range; i <= range; i++) color += Tex(Img, uv + s * ImgSize.xy * Vec2(i, 0));
    color /= range * 2 + 1;
    color.a *= Pow(1 - s, 2.0);
    return color;
}
/******************************************************************************/
VecH4 Fade_PS(NOPERSP Vec2 uv
              : UV) : TARGET {
    VecH4 c = Tex(Img, uv);
    c.a *= Sat(Step * 3 + (uv.x + uv.y) * 0.5 - 1 - Tex(Img1, uv).a);
    return c;
}
/******************************************************************************/
void Wave_VS(VtxInput vtx,
             NOPERSP out Vec2 uv
             : UV,
               NOPERSP out Vec4 vpos
             : POSITION) {
    uv.x = vtx.uv().x * Color[0].x + vtx.uv().y * Color[0].y + Color[0].z;
    uv.y = vtx.uv().x * Color[1].x + vtx.uv().y * Color[1].y + Color[1].z;
    vpos = vtx.pos4();
}
VecH4 Wave_PS(NOPERSP Vec2 uv
              : UV) : TARGET {
    return Tex(Img, uv);
}
/******************************************************************************/
VecH4 RadialBlur_PS(NOPERSP Vec2 uv
                    : UV) : TARGET {
    Vec2 uv1 = uv / Color[0].z + (Color[0].xy - Color[0].xy / Color[0].z); // (uv-RadialBlurPos)/step+RadialBlurPos
    Vec color = 0;                                                         // use HP
    const Int steps = 32;
    for (Int i = 0; i < steps; i++)
        color += Tex(Img, Lerp(uv, uv1, i / Flt(steps - 1))).rgb;
    return VecH4(color / steps, Color[0].w);
}
/******************************************************************************/
