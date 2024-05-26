/******************************************************************************/
#include "!Header.h"

#include "!Set Prec Struct.h"
BUFFER(Font)
Half FontShadow,
    FontContrast = 1,
    FontShade;
Flt FontDepth;
VecH FontLum;
BUFFER_END
#include "!Set Prec Default.h"
/******************************************************************************/
void Font_VS(VtxInput vtx,
             NOPERSP out Vec2 uv
             : UV,
               NOPERSP out Half shade
             : SHADE,
               NOPERSP out Vec4 vpos
             : POSITION) {
    uv = vtx.uv();
    shade = vtx.size();
    vpos = Vec4(vtx.pos2() * Coords.xy + Coords.zw, SET_DEPTH ? DelinearizeDepth(FontDepth) : Z_FRONT, 1);
}
VecH4 Font_PS(
    NOPERSP Vec2 uv
    : UV,
      NOPERSP Half shade
    : SHADE) : TARGET {
    // c=color, s=shadow, a=alpha

    // final=dest *(1-s) + 0*s; -> background after applying shadow
    // final=final*(1-a) + c*a; -> background after applying shadow after applying color

    // final=(dest*(1-s) + 0*s)*(1-a) + c*a;

    // final=dest*(1-s)*(1-a) + c*a;

    VecH2 as = ImgXY.Sample(SamplerFont, uv).rg; // #FontImageLayout
    Half a = Sat(as.x * FontContrast),           // font opacity, "Min(as.x*FontContrast, 1)", scale up by 'FontContrast' to improve quality when font is very small
        s = as.y * FontShadow;                   // font shadow

    if (GAMMA) {
        // a=  Sqr(  a); // good for bright text
        // a=1-Sqr(1-a); // good for dark   text
        // Half lum=Min(Max(Color[0].rgb)*4.672, 1); // calculate text brightness, multiply by "1/SRGBToLinear(0.5)" will give better results for grey text color, 'FontShade' is ignored for performance reasons
        a = Lerp(1 - Sqr(1 - a), Sqr(a), FontLum.x); // TODO: could this be done somehow per RGB channel? similar to Sub-Pixel
        s = 1 - Sqr(1 - s);
    }

    // Half final_alpha=1-(1-s)*(1-a);
    // 1-(1-s)*(1-a)
    // 1-(1-a-s+sa)
    // 1-1+a+s-sa
    // a + s - s*a
    Half final_alpha = a + s - s * a;

#if 1 // use for ALPHA_BLEND (this option is better because we don't need to set blend state specifically for drawing fonts)
    return VecH4(Color[0].rgb * (Lerp(FontShade, 1, Sat(shade)) * a / (final_alpha + HALF_MIN)), Color[0].a * final_alpha); // NaN, division by 'final_alpha' is required because of the hardware ALPHA_BLEND formula, without it we would get dark borders around the font
#else // use for ALPHA_MERGE
    return VecH4(Color[0].rgb * (Lerp(FontShade, 1, Sat(shade)) * a * Color[0].a), Color[0].a * final_alpha);
#endif
}
/******************************************************************************/
// SUB-PIXEL
/******************************************************************************/
void FontSP_VS(VtxInput vtx,
               NOPERSP out Vec2 uv
               : UV,
                 NOPERSP out Vec4 vpos
               : POSITION) {
    uv = vtx.uv();
    vpos = Vec4(vtx.pos2() * Coords.xy + Coords.zw, SET_DEPTH ? DelinearizeDepth(FontDepth) : Z_FRONT, 1);
}
VecH4 FontSP_PS(
    NOPERSP Vec2 uv
    : UV) : TARGET {
    VecH4 c = TexPoint(Img, uv);
    if (GAMMA) {
        // c.rgb=  Sqr(  c.rgb); // good for bright text
        // c.rgb=1-Sqr(1-c.rgb); // good for dark   text
        c.rgb = Lerp(1 - Sqr(1 - c.rgb), Sqr(c.rgb), FontLum); // auto
    }
    return c * Color[0].a;
}
/******************************************************************************/
