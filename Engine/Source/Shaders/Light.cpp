/******************************************************************************

   These shaders use 'discard' a few times to skip processing if light is known to be zero.

   However using 'discard' is not free:
      if no  pixels are discarded, then performance will be slower than without 'discard'
      if all pixels are discarded, then performance will be higher than without 'discard'
      benefit depends on amount of discarded pixels and the GPU

/******************************************************************************/
#pragma ruledisable "abs instruction to abs modifier match" // workaround for https://github.com/Esenthel/EsenthelEngine/issues/21 FIXME: TODO: check again in the future if this is still needed

#define WATER (LIGHT_MODE == LIGHT_MODE_WATER)
#define CLEAR_COAT (LIGHT_MODE == LIGHT_MODE_CLEAR_COAT)

#include "!Header.h"
#include "Water.h"

#include "!Set Prec Struct.h"
BUFFER(LightMap)
Flt LightMapScale = 1;
BUFFER_END
#include "!Set Prec Default.h"

#define SHADOW_PERCEPTUAL (SHADOW && 0) // disable because in tests it looked a little worse, and also it requires 1 extra MUL instruction
/******************************************************************************/
void Geom_VS // for 3D Geom
    (
        VtxInput vtx,
#if !GL_ES
        out NOPERSP Vec2 posXY
        : POS_XY,
#endif
          out Vec4 vpos
        : POSITION) {
    vpos = Project(TransformPos(vtx.pos()));

#if CLAMP_DEPTH // simulate "D.depthClip(false)", needed for GL ES which doesn't support it, Warning: this introduces a bit too much clipping at the viewport end, because the neighboring vertexes remain the same, and only the vertex behind the viewport gets repositioned, the line between them won't cover entire original area (however it's small)
#if REVERSE_DEPTH
    vpos.z = Max(vpos.z, 0);
#else
    vpos.z = Min(vpos.z, vpos.w);
#endif
#endif

#if !GL_ES
    Vec2 uv = ProjectedPosToUV(vpos);
    posXY = UVToPosXY(uv);
#endif
}
/******************************************************************************/
// Img=Nrm (this also used for Water Apply shader), ImgMS=NrmMS, Img1=Col, ImgMS1=ColMS, ImgXY=Ext, ImgXYMS=ExtMS, Img2=ConeLight.Lightmap, ImgX=shadow
/******************************************************************************/
VecH LightDir_PS(
    NOPERSP Vec2 posXY
    : POS_XY,
      NOPERSP PIXEL // 2D
#if MULTI_SAMPLE == 2
      ,
      UInt index
    : SV_SampleIndex
#endif
      ,
      out VecH outSpec
    : TARGET1) : TARGET {
    VecI2 pix = pixel.xy;
    // shadow (start with shadows because they're IMAGE_R8 and have small bandwidth)
#if MULTI_SAMPLE <= 1
    Half shadow;
    if (SHADOW)
        shadow = ImgX[pix];
#else
    Half shadow;
    if (SHADOW)
        shadow = TexSample(ImgXMS, pix, index).x;
#endif
    if (SHADOW && shadow <= EPS_LUM)
        discard;

        // normal
#if MULTI_SAMPLE == 0
    Vec4 nrm = GetNormal(pix);
#elif MULTI_SAMPLE == 1
    Vec4 nrm = GetNormalMS(pix, 0);
#else
    Vec4 nrm = GetNormalMS(pix, index);
#endif

    // light
    Vec light_dir = LightDir.dir;
    LightParams lp;
    lp.set(nrm.xyz, light_dir);
    Half lum = lp.NdotL;
    if (SHADOW)
        lum *= shadow;
    if (lum <= EPS_LUM) {
        if (!WATER && IsTranslucent(nrm.w) && -lum > EPS_LUM) {
            outSpec = 0;
            return LightDir.color.rgb * (lum * -TRANSLUCENT_VAL);
        }        // #RTOutput translucent
        discard; // !! have to skip when "NdotL<=0" to don't apply negative values to RT !!
    }
    if (SHADOW_PERCEPTUAL)
        lum *= shadow;

        // ext+col
#if WATER
    VecH2 ext = {WaterMaterial.rough, WaterMaterial.reflect}; // #RTOutput Water doesn't have Ext Texture #WaterExt
    Half base_col = WaterMaterial.reflect;                    // this is used for calculation of final reflectivity, just copy from water reflectivity #WaterExt
#elif MULTI_SAMPLE == 0
    VecH2 ext = GetExt(pix);
    VecH base_col = Img1[pix].rgb;
#elif MULTI_SAMPLE == 1
    VecH2 ext = GetExtMS(pix, 0);
    VecH base_col = TexSample(ImgMS1, pix, 0).rgb;
#else
    VecH2 ext = GetExtMS(pix, index);
    VecH base_col = TexSample(ImgMS1, pix, index).rgb;
#endif

    // light #1
    Vec eye_dir = Normalize(Vec(posXY, 1));
    lp.set(nrm.xyz, light_dir, eye_dir);

    VecH lum_rgb = LightDir.color.rgb * lum;
    outSpec = lum_rgb * lp.specularEx(ext.x, ext.y, ReflectCol(ext.y, base_col), CLEAR_COAT ? IsClearCoat(nrm.w) : false, LightDir.radius_frac); // specular #RTOutput
    return lum_rgb * lp.diffuse(ext.x);                                                                                                          // diffuse  #RTOutput
}
/******************************************************************************/
VecH LightPoint_PS(
#if !GL_ES // doesn't support NOPERSP
    NOPERSP Vec2 posXY
    : POS_XY,
#endif
      PIXEL // 3D
#if MULTI_SAMPLE == 2
      ,
      UInt index
    : SV_SampleIndex
#endif
      ,
      out VecH outSpec
    : TARGET1) : TARGET {
    VecI2 pix = pixel.xy;
#if GL_ES // doesn't support NOPERSP
    Vec2 uv = PixelToUV(pixel);
    Vec2 posXY = UVToPosXY(uv);
#endif

    // shadow (start with shadows because they're IMAGE_R8 and have small bandwidth)
#if MULTI_SAMPLE <= 1
    Half shadow;
    if (SHADOW)
        shadow = ShadowFinal(ImgX[pix]);
#else
    Half shadow;
    if (SHADOW)
        shadow = ShadowFinal(TexSample(ImgXMS, pix, index).x);
#endif
    if (SHADOW && shadow <= EPS_LUM)
        discard;

        // distance
#if MULTI_SAMPLE <= 1
    Vec pos = GetPosPix(pix, posXY);
#else
    Vec pos = GetPosMS(pix, index, posXY);
#endif
    Vec delta = LightPoint.pos - pos;
    Flt inv_dist2 = 1 / Length2(delta);
    Half lum = LightPointDist(inv_dist2);
    if (SHADOW)
        lum *= shadow;
    if (lum <= EPS_LUM)
        discard;

        // normal
#if MULTI_SAMPLE == 0
    Vec4 nrm = GetNormal(pix);
#elif MULTI_SAMPLE == 1
    Vec4 nrm = GetNormalMS(pix, 0);
#else
    Vec4 nrm = GetNormalMS(pix, index);
#endif

    // light
    Vec light_dir = delta * Sqrt(inv_dist2); // Normalize(delta);
    LightParams lp;
    lp.set(nrm.xyz, light_dir);
    lum *= lp.NdotL;
    if (lum <= EPS_LUM) {
        if (!WATER && IsTranslucent(nrm.w) && -lum > EPS_LUM) {
            outSpec = 0;
            return LightPoint.color.rgb * (lum * -TRANSLUCENT_VAL);
        }        // #RTOutput translucent
        discard; // !! have to skip when "NdotL<=0" to don't apply negative values to RT !!
    }
    if (SHADOW_PERCEPTUAL)
        lum *= shadow;

        // ext+col
#if WATER
    VecH2 ext = {WaterMaterial.rough, WaterMaterial.reflect}; // #RTOutput Water doesn't have Ext Texture #WaterExt
    Half base_col = WaterMaterial.reflect;                    // this is used for calculation of final reflectivity, just copy from water reflectivity #WaterExt
#elif MULTI_SAMPLE == 0
    VecH2 ext = GetExt(pix);
    VecH base_col = Img1[pix].rgb;
#elif MULTI_SAMPLE == 1
    VecH2 ext = GetExtMS(pix, 0);
    VecH base_col = TexSample(ImgMS1, pix, 0).rgb;
#else
    VecH2 ext = GetExtMS(pix, index);
    VecH base_col = TexSample(ImgMS1, pix, index).rgb;
#endif

    // light #1
    Vec eye_dir = Normalize(pos);
    lp.set(nrm.xyz, light_dir, eye_dir);

    VecH lum_rgb = LightPoint.color.rgb * lum;
    outSpec = lum_rgb * lp.specularEx(ext.x, ext.y, ReflectCol(ext.y, base_col), CLEAR_COAT ? IsClearCoat(nrm.w) : false); // specular #RTOutput
    return lum_rgb * lp.diffuse(ext.x);                                                                                    // diffuse  #RTOutput
}
/******************************************************************************/
VecH LightLinear_PS(
#if !GL_ES // doesn't support NOPERSP
    NOPERSP Vec2 posXY
    : POS_XY,
#endif
      PIXEL // 3D
#if MULTI_SAMPLE == 2
      ,
      UInt index
    : SV_SampleIndex
#endif
      ,
      out VecH outSpec
    : TARGET1) : TARGET {
    VecI2 pix = pixel.xy;
#if GL_ES // doesn't support NOPERSP
    Vec2 uv = PixelToUV(pixel);
    Vec2 posXY = UVToPosXY(uv);
#endif

    // shadow (start with shadows because they're IMAGE_R8 and have small bandwidth)
#if MULTI_SAMPLE <= 1
    Half shadow;
    if (SHADOW)
        shadow = ShadowFinal(ImgX[pix]);
#else
    Half shadow;
    if (SHADOW)
        shadow = ShadowFinal(TexSample(ImgXMS, pix, index).x);
#endif
    if (SHADOW && shadow <= EPS_LUM)
        discard;

        // distance
#if MULTI_SAMPLE <= 1
    Vec pos = GetPosPix(pix, posXY);
#else
    Vec pos = GetPosMS(pix, index, posXY);
#endif
    Vec delta = LightLinear.pos - pos;
    Flt dist = Length(delta);
    Half lum = LightLinearDist(dist);
    if (SHADOW)
        lum *= shadow;
    if (lum <= EPS_LUM)
        discard;

        // normal
#if MULTI_SAMPLE == 0
    Vec4 nrm = GetNormal(pix);
#elif MULTI_SAMPLE == 1
    Vec4 nrm = GetNormalMS(pix, 0);
#else
    Vec4 nrm = GetNormalMS(pix, index);
#endif

    // light
    Vec light_dir = delta / dist; // Normalize(delta);
    LightParams lp;
    lp.set(nrm.xyz, light_dir);
    lum *= lp.NdotL;
    if (lum <= EPS_LUM) {
        if (!WATER && IsTranslucent(nrm.w) && -lum > EPS_LUM) {
            outSpec = 0;
            return LightLinear.color.rgb * (lum * -TRANSLUCENT_VAL);
        }        // #RTOutput translucent
        discard; // !! have to skip when "NdotL<=0" to don't apply negative values to RT !!
    }
    if (SHADOW_PERCEPTUAL)
        lum *= shadow;

        // ext+col
#if WATER
    VecH2 ext = {WaterMaterial.rough, WaterMaterial.reflect}; // #RTOutput Water doesn't have Ext Texture #WaterExt
    Half base_col = WaterMaterial.reflect;                    // this is used for calculation of final reflectivity, just copy from water reflectivity #WaterExt
#elif MULTI_SAMPLE == 0
    VecH2 ext = GetExt(pix);
    VecH base_col = Img1[pix].rgb;
#elif MULTI_SAMPLE == 1
    VecH2 ext = GetExtMS(pix, 0);
    VecH base_col = TexSample(ImgMS1, pix, 0).rgb;
#else
    VecH2 ext = GetExtMS(pix, index);
    VecH base_col = TexSample(ImgMS1, pix, index).rgb;
#endif

    // light #1
    Vec eye_dir = Normalize(pos);
    lp.set(nrm.xyz, light_dir, eye_dir);

    VecH lum_rgb = LightLinear.color.rgb * lum;
    outSpec = lum_rgb * lp.specularEx(ext.x, ext.y, ReflectCol(ext.y, base_col), CLEAR_COAT ? IsClearCoat(nrm.w) : false); // specular #RTOutput
    return lum_rgb * lp.diffuse(ext.x);                                                                                    // diffuse  #RTOutput
}
/******************************************************************************/
VecH LightCone_PS(
#if !GL_ES // doesn't support NOPERSP
    NOPERSP Vec2 posXY
    : POS_XY,
#endif
      PIXEL // 3D
#if MULTI_SAMPLE == 2
      ,
      UInt index
    : SV_SampleIndex
#endif
      ,
      out VecH outSpec
    : TARGET1) : TARGET {
    VecI2 pix = pixel.xy;
#if GL_ES // doesn't support NOPERSP
    Vec2 uv = PixelToUV(pixel);
    Vec2 posXY = UVToPosXY(uv);
#endif

    // shadow (start with shadows because they're IMAGE_R8 and have small bandwidth)
#if MULTI_SAMPLE <= 1
    Half shadow;
    if (SHADOW)
        shadow = ShadowFinal(ImgX[pix]);
#else
    Half shadow;
    if (SHADOW)
        shadow = ShadowFinal(TexSample(ImgXMS, pix, index).x);
#endif
    if (SHADOW && shadow <= EPS_LUM)
        discard;

        // distance & angle
#if MULTI_SAMPLE <= 1
    Vec pos = GetPosPix(pix, posXY);
#else
    Vec pos = GetPosMS(pix, index, posXY);
#endif
    Vec delta = LightCone.pos - pos,
        dir = TransformTP(delta, LightCone.mtrx);
    dir.xy /= dir.z;
    clip(Vec(1 - Abs(dir.xy), dir.z));
    Flt dist = Length(delta);
    Half lum = LightConeAngle(dir.xy) * LightConeDist(dist);
    if (SHADOW)
        lum *= shadow;
    if (lum <= EPS_LUM)
        discard;

        // normal
#if MULTI_SAMPLE == 0
    Vec4 nrm = GetNormal(pix);
#elif MULTI_SAMPLE == 1
    Vec4 nrm = GetNormalMS(pix, 0);
#else
    Vec4 nrm = GetNormalMS(pix, index);
#endif

    // light
    Vec light_dir = delta / dist; // Normalize(delta);
    LightParams lp;
    lp.set(nrm.xyz, light_dir);
    lum *= lp.NdotL;
    if (lum <= EPS_LUM) {
        if (!WATER && IsTranslucent(nrm.w) && -lum > EPS_LUM) // #RTOutput translucent
        {
            outSpec = 0;
            lum *= -TRANSLUCENT_VAL;
            VecH lum_rgb = LightCone.color.rgb * lum;
#if IMAGE
            lum_rgb *= RTex(Img2, dir.xy * (LightMapScale * 0.5) + 0.5).rgb;
#endif
            return lum_rgb;
        }
        discard; // !! have to skip when "NdotL<=0" to don't apply negative values to RT !!
    }
    if (SHADOW_PERCEPTUAL)
        lum *= shadow;

        // ext+col
#if WATER
    VecH2 ext = {WaterMaterial.rough, WaterMaterial.reflect}; // #RTOutput Water doesn't have Ext Texture #WaterExt
    Half base_col = WaterMaterial.reflect;                    // this is used for calculation of final reflectivity, just copy from water reflectivity #WaterExt
#elif MULTI_SAMPLE == 0
    VecH2 ext = GetExt(pix);
    VecH base_col = Img1[pix].rgb;
#elif MULTI_SAMPLE == 1
    VecH2 ext = GetExtMS(pix, 0);
    VecH base_col = TexSample(ImgMS1, pix, 0).rgb;
#else
    VecH2 ext = GetExtMS(pix, index);
    VecH base_col = TexSample(ImgMS1, pix, index).rgb;
#endif

    // light #1
    Vec eye_dir = Normalize(pos);
    lp.set(nrm.xyz, light_dir, eye_dir);

    VecH lum_rgb = LightCone.color.rgb * lum;
#if IMAGE
    lum_rgb *= RTex(Img2, dir.xy * (LightMapScale * 0.5) + 0.5).rgb;
#endif

    outSpec = lum_rgb * lp.specularEx(ext.x, ext.y, ReflectCol(ext.y, base_col), CLEAR_COAT ? IsClearCoat(nrm.w) : false); // specular #RTOutput
    return lum_rgb * lp.diffuse(ext.x);                                                                                    // diffuse  #RTOutput
}
/******************************************************************************/
