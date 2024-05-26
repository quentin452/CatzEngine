/******************************************************************************/
#include "!Header.h"
#ifndef MAP_NUM
#define MAP_NUM 0
#endif
#ifndef CLOUD
#define CLOUD 0
#endif
#ifndef SAMPLES
#define SAMPLES 0
#endif
#ifndef RANGE
#define RANGE 0
#endif
#ifndef GATHER
#define GATHER 0
#endif
#ifndef LINEAR_DEPTH
#define LINEAR_DEPTH 0 // if depth is already linearized
#endif
/******************************************************************************/
void Geom_VS // for 3D Geom
    (
        VtxInput vtx,
#if !GL_ES
        out NOPERSP Vec2 uv
        : UV,
// out NOPERSP Vec2 posXY:POS_XY,
#endif
          out Vec4 vpos
        : POSITION) {
    vpos = Project(TransformPos(vtx.pos()));

#if GL_ES // simulate D.depthClip(false), needed for GL ES which doesn't support it, Warning: this introduces a bit too much clipping at the viewport end, because the neighboring vertexes remain the same, and only the vertex behind the viewport gets repositioned, the line between them won't cover entire original area (however it's small)
#if REVERSE_DEPTH
    vpos.z = Max(vpos.z, 0);
#else
    vpos.z = Min(vpos.z, vpos.w);
#endif
#endif

#if !GL_ES
    uv = ProjectedPosToUV(vpos);
    // posXY=          UVToPosXY(uv);
#endif
}
/******************************************************************************/
// SHADOW SET
/******************************************************************************/
Half ShdDir_PS(NOPERSP Vec2 posXY
               : POS_XY,
                 NOPERSP PIXEL
#if MULTI_SAMPLE
                 ,
                 UInt index
               : SV_SampleIndex
#endif
               ) : TARGET {
#if MULTI_SAMPLE
    return ShadowDirValue(GetPosMS(pixel.xy, index, posXY), ShadowJitter(pixel.xy), true, MAP_NUM, CLOUD);
#else
    return ShadowDirValue(GetPosPix(pixel.xy, posXY), ShadowJitter(pixel.xy), true, MAP_NUM, CLOUD);
#endif
}
/******************************************************************************/
Half ShdPoint_PS(NOPERSP Vec2 posXY
                 : POS_XY,
                   NOPERSP PIXEL
#if MULTI_SAMPLE
                   ,
                   UInt index
                 : SV_SampleIndex
#endif
                 ) : TARGET {
#if MULTI_SAMPLE
    return ShadowPointValue(GetPosMS(pixel.xy, index, posXY), ShadowJitter(pixel.xy), true);
#else
    return ShadowPointValue(GetPosPix(pixel.xy, posXY), ShadowJitter(pixel.xy), true);
#endif
}
/******************************************************************************/
Half ShdCone_PS(NOPERSP Vec2 posXY
                : POS_XY,
                  NOPERSP PIXEL
#if MULTI_SAMPLE
                  ,
                  UInt index
                : SV_SampleIndex
#endif
                ) : TARGET {
#if MULTI_SAMPLE
    return ShadowConeValue(GetPosMS(pixel.xy, index, posXY), ShadowJitter(pixel.xy), true);
#else
    return ShadowConeValue(GetPosPix(pixel.xy, posXY), ShadowJitter(pixel.xy), true);
#endif
}
/******************************************************************************/
// SHADOW BLUR
/******************************************************************************/
Flt LinearDepth(Flt z) {
    return LINEAR_DEPTH ? z : LinearizeDepth(z);
}
// can use 'RTSize' instead of 'ImgSize' since there's no scale
#if GL_ES && GATHER // GL ES with GATHER support
#undef TexDepthRawLinear
Flt TexDepthRawLinear(Vec2 uv) // because GL ES 3 can't do 'TexDepthRawLinear'
{
    Vec2 pixel = uv * RTSize.zw + 0.5,
         pixeli = Floor(pixel),
         f = pixel - pixeli;
    Flt fx1 = 1 - f.x;
    uv = pixeli * RTSize.xy; // adjust 'uv' to make sure pixels will be selected based on 'pixeli' (because of precision issues)
    Vec4 t = TexDepthRawGather(uv);

    return Lerp(t.w * fx1 + t.z * f.x,
                t.x * fx1 + t.y * f.x, f.y);
}
#undef TexDepthLinear
Flt TexDepthLinear(Vec2 uv) { return LinearDepth(TexDepthRawLinear(uv)); } // because GL ES 3 can't do 'TexDepthLinear'
#endif
void Process(inout Half color, inout Half weight, Half c, Flt raw_z, Flt base_z, Vec2 dw_mad) {
    if (1) {
        Half w = DepthWeight(base_z - LinearDepth(raw_z), dw_mad);
        color += w * c;
        weight += w;
    } else {
        color += c;
        weight += 1;
    }
}
/******************************************************************************/
Half ShdBlurJitter_PS(
#if GL_ES && GEOM // doesn't support NOPERSP
    PIXEL // 3D
#else
    NOPERSP Vec2 uv
    : UV
#endif
    ) : TARGET {
#if GL_ES && GEOM // doesn't support NOPERSP
    Vec2 uv = PixelToUV(pixel);
#endif

    Half weight, color;
    Flt z;
    Vec2 dw_mad;
    if (GATHER) {
        uv -= RTSize.xy / 2;   // move at the center of 2x2
        if (JITTER_RANGE == 3) // 3x3
        {
            UNROLL for (Int y = 0; y < 2; y++)
                UNROLL for (Int x = 0; x < 2; x++) {
                VecH4 c = TexGatherOfs(ImgX, uv, VecI2(x * 2, y * 2));
                if (1) // use depth
                {
                    Vec4 d = TexDepthRawGatherOfs(uv, VecI2(x * 2, y * 2));
                    if (x == 0 && y == 0) {
                        z = LinearDepth(d.y);
                        dw_mad = DepthWeightMADPoint(z);
                        color = c.y;
                        weight = 1;
                    } // Y component is the x=0,y=1 pixel which is the center pixel because we've offseted 'uv'
                    if (x == 0 && y == 0) {
                        Process(color, weight, c.x, d.x, z, dw_mad); /*Process(color, weight, c.y, d.y, z, dw_mad); already processed above*/
                        Process(color, weight, c.z, d.z, z, dw_mad);
                        Process(color, weight, c.w, d.w, z, dw_mad);
                    } else if (x == 1 && y == 0) {
                        Process(color, weight, c.x, d.x, z, dw_mad);
                        Process(color, weight, c.w, d.w, z, dw_mad);
                    } else if (x == 0 && y == 1) {
                        Process(color, weight, c.w, d.w, z, dw_mad);
                        Process(color, weight, c.z, d.z, z, dw_mad);
                    } else if (x == 1 && y == 1) {
                        Process(color, weight, c.w, d.w, z, dw_mad);
                    }
                } else {
                    if (x == 0 && y == 0) {
                        color += Sum(c);
                        weight += 4;
                    } else if (x == 1 && y == 0) {
                        color += c.x + c.w;
                        weight += 2;
                    } else if (x == 0 && y == 1) {
                        color += c.w + c.z;
                        weight += 2;
                    } else if (x == 1 && y == 1) {
                        color += c.w;
                        weight += 1;
                    }
                }
            }
        } else // 4x4 Warning: 4x4 blur causes the contents to be offseted by 1 pixel XY
        {
            UNROLL for (Int y = 0; y < 2; y++)
                UNROLL for (Int x = 0; x < 2; x++) {
                VecH4 c = TexGatherOfs(ImgX, uv, VecI2(x * 2, y * 2));
                if (1) // use depth
                {
                    Vec4 d = TexDepthRawGatherOfs(uv, VecI2(x * 2, y * 2));
                    if (x == 0 && y == 0) {
                        z = LinearDepth(d.y);
                        dw_mad = DepthWeightMADPoint(z);
                        color = c.y;
                        weight = 1;
                    } else
                        Process(color, weight, c.y, d.y, z, dw_mad); // Y component is the x=0,y=1 pixel which is the center pixel because we've offseted 'uv'
                    Process(color, weight, c.x, d.x, z, dw_mad);
                    Process(color, weight, c.z, d.z, z, dw_mad);
                    Process(color, weight, c.w, d.w, z, dw_mad);
                } else {
                    color += Sum(c);
                    weight += 4;
                }
            }
        }
    } else {
        weight = 1;
        color = TexPoint(ImgX, uv).x;
        z = LinearDepth(TexDepthRawPoint(uv));
        dw_mad = DepthWeightMADPoint(z);
        Int min, max; // inclusive
        if (JITTER_RANGE == 3) {
            min = -1;
            max = 1;
        } // 3x3
        else {
            min = -1;
            max = 2;
        } // 4x4
        UNROLL for (Int y = min; y <= max; y++)
            UNROLL for (Int x = min; x <= max; x++) if (x || y) // skip 0,0
            Process(color, weight, TexPointOfs(ImgX, uv, VecI2(x, y)).x, TexDepthRawPointOfs(uv, VecI2(x, y)), z, dw_mad);
    }
    return color / weight;
}
/******************************************************************************/
Half ShdBlur_PS(
#if !(GL_ES && GEOM) // doesn't support NOPERSP
    NOPERSP Vec2 uv
    : UV,
#endif
      PIXEL // 3D
    ) : TARGET {
#if GL_ES && GEOM // doesn't support NOPERSP
    Vec2 uv = PixelToUV(pixel);
#endif

    Half weight = 0.25,
         color = ImgX[pixel.xy].x * weight;
    Flt z = LinearDepth(Depth[pixel.xy]);
    Vec2 dw_mad = DepthWeightMADLinear(z);
    UNROLL for (Int i = 0; i < SAMPLES; i++) {
        Vec2 t;
        if (SAMPLES == 4)
            t = RTSize.xy * BlendOfs4[i] + uv;
        // if(SAMPLES== 5)t=RTSize.xy*BlendOfs5 [i]+uv;
        if (SAMPLES == 6)
            t = RTSize.xy * BlendOfs6[i] + uv;
        if (SAMPLES == 8)
            t = RTSize.xy * BlendOfs8[i] + uv;
        // if(SAMPLES== 9)t=RTSize.xy*BlendOfs9 [i]+uv;
        if (SAMPLES == 12)
            t = RTSize.xy * BlendOfs12[i] + uv;
        // if(SAMPLES==13)t=RTSize.xy*BlendOfs13[i]+uv;
        //  use linear filtering because texcoords are not rounded
        Process(color, weight, TexLod(ImgX, t).x, TexDepthRawLinear(t), z, dw_mad); // use linear filtering because texcoords aren't rounded
    }
    return color / weight;
}
/******************************************************************************/
Half ShdBlurX_PS(
#if !(GL_ES && GEOM) // doesn't support NOPERSP
    NOPERSP Vec2 uv
    : UV,
#endif
      PIXEL // 3D
    ) : TARGET {
#if GL_ES && GEOM // doesn't support NOPERSP
    Vec2 uv = PixelToUV(pixel);
#endif

    Half weight = 0.5,
         color = ImgX[pixel.xy].x * weight;
    Flt z = LinearDepth(Depth[pixel.xy]);
    Vec2 dw_mad = DepthWeightMADLinear(z), t;
    t.y = uv.y;
    UNROLL for (Int i = -RANGE; i <= RANGE; i++) if (i) {
        // use linear filtering because texcoords are not rounded
        t.x = RTSize.x * (2 * i + ((i > 0) ? -0.5 : 0.5)) + uv.x;
        Process(color, weight, TexLod(ImgX, t).x, TexDepthRawLinear(t), z, dw_mad); // use linear filtering because texcoords aren't rounded
    }
    return color / weight;
}
/******************************************************************************/
Half ShdBlurY_PS(
#if !(GL_ES && GEOM) // doesn't support NOPERSP
    NOPERSP Vec2 uv
    : UV,
#endif
      PIXEL // 3D
    ) : TARGET {
#if GL_ES && GEOM // doesn't support NOPERSP
    Vec2 uv = PixelToUV(pixel);
#endif

    Half weight = 0.5,
         color = ImgX[pixel.xy].x * weight;
    Flt z = LinearDepth(Depth[pixel.xy]);
    Vec2 dw_mad = DepthWeightMADLinear(z), t;
    t.x = uv.x;
    UNROLL for (Int i = -RANGE; i <= RANGE; i++) if (i) {
        // use linear filtering because texcoords are not rounded
        t.y = RTSize.y * (2 * i + ((i > 0) ? -0.5 : 0.5)) + uv.y;
        Process(color, weight, TexLod(ImgX, t).x, TexDepthRawLinear(t), z, dw_mad); // use linear filtering because texcoords aren't rounded
    }
    return color / weight;
}
/******************************************************************************/
