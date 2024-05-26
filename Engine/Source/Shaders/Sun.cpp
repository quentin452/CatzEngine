/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
#include "!Set Prec Struct.h"
struct SunClass {
    Vec2 pos2;
    VecH pos, color;
};

BUFFER(Sun)
SunClass Sun;
BUFFER_END
#include "!Set Prec Default.h"
/******************************************************************************/
/*Vec4 SunRaysSoft_PS(NOPERSP Vec2 uv:UV):TARGET
{
   const Int samples=6;
         Flt color  =TexLod(ImgX, uv).x; // use linear filtering because 'ImgX' can be of different size
   for(Int i=0; i<samples; i++)
   {
      Vec2 t=uv+BlendOfs6[i]*ImgSize.xy;
      color+=TexLod(ImgX, t).x; // use linear filtering because texcoords aren't rounded
   }
   return Vec4(color/(samples+1)*Color[0].rgb, 0);
}
TECHNIQUE(SunRaysSoft, DrawUV_VS(), SunRaysSoft_PS());*/
/******************************************************************************/
// ALPHA, DITHER, JITTER, GAMMA
VecH4 SunRays_PS(NOPERSP Vec2 uv
                 : UV,
                   NOPERSP Vec2 posXY
                 : POS_XY,
                   NOPERSP PIXEL) : TARGET {
    VecH pos = Normalize(Vec(posXY, 1));
    Half cos = Dot(Sun.pos, pos),
         power = (LINEAR_GAMMA ? cos : (cos > 0) ? Sqr(cos)
                                                 : 0);
    VecH4 col = 0;

#if 0 // can't use 'clip' because we always have to set output (to 0 if no rays)
   clip(power-EPS_COL);
#else
    BRANCH if (power > EPS_COL)
#endif
    {
        const Int steps = 40;

        // limit sun position
        Vec2 sun_pos = Sun.pos2;
        Vec2 delta = uv - sun_pos;                // towards viewport
        Flt frac = ViewportClamp(sun_pos, delta); // returns 0 if already inside viewport
        sun_pos += frac * delta;                  // move towards viewport
                                                  // power  *=(1-frac);

#if JITTER
        uv += (sun_pos - uv) * (Noise1D(pixel.xy) * (3.0 / steps)); // a good value is 2.5 or 3.0 (3.0 was slightly better)
#endif

#if ALPHA
        Flt occl = 0; // use HP because we're iterating lot of samples
#else
        Flt back = 0; // use HP because we're iterating lot of samples
#endif

        UNROLL for (Int i = 0; i < steps; i++) {
            Vec2 t = Lerp(uv, sun_pos, i / Flt(steps)); // /(steps) worked better than /(steps-1)
#if ALPHA
            occl += TexLod(ImgX, t).x; // depth and clouds combined together, use linear filtering because texcoords aren't rounded
#else
            back += DEPTH_BACKGROUND(TexDepthRawPoint(t)); // use simpler version here unlike in 'SunRaysMask_PS' because this one is called for each step for each pixel
#endif
        }

#if ALPHA
        Half light = 1 - occl / steps;
#else
        Half light = back / steps;
#endif
        col.rgb = (light * power) * Sun.color;

#if DITHER
        ApplyDither(col.rgb, pixel.xy, false); // here we're always in sRGB gamma
#endif

#if GAMMA
        col.rgb = SRGBToLinearFast(col.rgb); // 'SRGBToLinearFast' works better here than 'SRGBToLinear' (gives high contrast, dark colors remain darker, while 'SRGBToLinear' highlights them more)
#endif
    }
    return col;
}
/******************************************************************************/
