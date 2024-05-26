/******************************************************************************/
#include "!Header.h"
#include "Light Apply.h"

#ifndef REFLECT_MODE
#define REFLECT_MODE 1
#endif

#ifndef REFLECT
#define REFLECT (REFLECT_MODE > 0)
#endif

#ifndef CLEAR_COAT
#define CLEAR_COAT (REFLECT_MODE > 1)
#endif

#define AO_ALL 1 // !! must be the same as 'D.aoAll()' !! if apply Ambient Occlusion to all lights (not just Ambient), this was disabled in the past, however in LINEAR_GAMMA the darkening was too strong in low light, enabling this option solves that problem

// MULTI_SAMPLE, AO, CEL_SHADE, NIGHT_SHADE, GLOW, REFLECT
// Img=Nrm, ImgMS=Nrm, Img1=Col, ImgMS1=Col, Img2=Lum, ImgMS2=Lum, Img3=Spec, ImgMS3=Spec, ImgXY=Ext, ImgXYMS=Ext, ImgX=AO, Img4=CelShade
/******************************************************************************/
Half CelShade(Half lum) { return TexLod(Img4, VecH2(lum, 0.5)).x; } // have to use linear filtering
/******************************************************************************/
VecH LitCol(VecH base_col, Half glow, Vec nrm, Half rough, Half reflect, VecH lum, VecH spec, Half ao, VecH night_shade_col, Bool night_shade_ao, Vec eye_dir, Flt mode) {
    Half max_lum = Max(lum);
    if (CEL_SHADE) {
        max_lum = CelShade(max_lum);
        lum = max_lum;
    }
    VecH lit_col = base_col * lum;
    if (NIGHT_SHADE) {
#if LINEAR_GAMMA
        Half night_shade_intensity = Sat(1 - max_lum)                    // only for low light
                                     * LinearLumOfLinearColor(base_col); // set based on unlit color luminance
#else
        Half night_shade_intensity = Sat(1 - max_lum)                // only for low light
                                     * SRGBLumOfSRGBColor(base_col); // set based on unlit color luminance
#endif
        if (night_shade_ao)
            night_shade_intensity *= ao;
        lit_col += night_shade_intensity * night_shade_col;
    }
    Half inv_metal = ReflectToInvMetal(reflect), diffuse = Diffuse(inv_metal);
    if (GLOW)
        ProcessGlow(glow, diffuse);
#if REFLECT
    Half NdotV = -Dot(nrm, eye_dir);
    Vec reflect_dir = ReflectDir(eye_dir, nrm);
    VecH reflect_col = ReflectCol(reflect, base_col, inv_metal);
    VecH env_col = ReflectTex(reflect_dir, rough) * ReflectEnv(rough, reflect, reflect_col, NdotV, true);

#if CLEAR_COAT
    BRANCH if (IsClearCoat(mode)) {
        Half clear_coat_rough = 0;
        Half clear_coat_reflect = 0.04;

#if 1 // faster but still good quality
        Half clear_coat = F_Schlick(clear_coat_reflect, 1, Sat(NdotV));
#else
        Half clear_coat = ReflectEnv(clear_coat_rough, clear_coat_reflect, ReflectCol(clear_coat_reflect, 1), NdotV, true).x;
#endif

        // diffuse*=1-clear_coat; // TODO: FIXME: this should be on?

        env_col *= 1 - clear_coat;
        env_col += ReflectTex(reflect_dir, clear_coat_rough) * clear_coat;
    }
#endif

    spec += EnvColor * env_col;
#endif
    lit_col = lit_col * diffuse + spec;
    if (AO && AO_ALL)
        lit_col *= ao;
    if (GLOW)
        ProcessGlow(glow, base_col, lit_col);
    return lit_col;
}
/******************************************************************************/
VecH4 ApplyLight_PS(NOPERSP Vec2 uv
                    : UV,
                      NOPERSP Vec2 posXY
                    : POS_XY,
                      NOPERSP PIXEL) : TARGET {
    Half ao;
    VecH ambient;
    if (AO) {
        ao = TexLod(ImgX, uv).x;
        if (!AO_ALL)
            ambient = AmbientColor * ao;
    } // use 'TexLod' because AO can be of different size and we need to use tex filtering. #AmbientInLum
    VecI2 pix = pixel.xy;
    Vec eye_dir = Normalize(Vec(posXY, 1));
    if (MULTI_SAMPLE == 0) {
        VecH4 color = Img1[pix]; // #RTOutput
        VecH lum = Img2[pix].rgb;
        VecH spec = Img3[pix].rgb;
        Vec4 nrm = GetNormal(pix);
        VecH2 ext = GetExt(pix); // #RTOutput
        if (AO && !AO_ALL)
            lum += ambient;
        color.rgb = LitCol(color.rgb, color.a, nrm.xyz, ext.x, ext.y, lum, spec, ao, NightShadeColor, AO && !AO_ALL, eye_dir, nrm.w);
        return color;
    } else if (MULTI_SAMPLE == 1) // 1 sample
    {
        VecH4 color = TexSample(ImgMS1, pix, 0); // #RTOutput
        VecH lum = Img2[pix].rgb;                //  Lum1S
        VecH spec = Img3[pix].rgb;               // Spec1S
        Vec4 nrm = GetNormalMS(pix, 0);
        VecH2 ext = GetExtMS(pix, 0); // #RTOutput
        if (AO && !AO_ALL)
            lum += ambient;
        color.rgb = LitCol(color.rgb, color.a, nrm.xyz, ext.x, ext.y, lum, spec, ao, NightShadeColor, AO && !AO_ALL, eye_dir, nrm.w);
        return color;
    } else // n samples
    {
        VecH4 color_sum = 0;
        Half valid_samples = HALF_MIN;
        VecH night_shade_col;
        if (NIGHT_SHADE && AO && !AO_ALL)
            night_shade_col = NightShadeColor * ao;                                              // compute it once, and not inside 'LitCol'
        UNROLL for (Int i = 0; i < MS_SAMPLES; i++) if (DEPTH_FOREGROUND(TexDepthRawMS(pix, i))) // valid sample
        {
            VecH4 color = TexSample(ImgMS1, pix, i);   // #RTOutput
            VecH lum = TexSample(ImgMS2, pix, i).rgb;  //  LumMS
            VecH spec = TexSample(ImgMS3, pix, i).rgb; // SpecMS
            Vec4 nrm = GetNormalMS(pix, i);
            VecH2 ext = GetExtMS(pix, i); // #RTOutput
            if (AO && !AO_ALL)
                lum += ambient;
            color.rgb = LitCol(color.rgb, color.a, nrm.xyz, ext.x, ext.y, lum, spec, ao, (NIGHT_SHADE && AO && !AO_ALL) ? night_shade_col : NightShadeColor, false, eye_dir, nrm.w); // we've already adjusted 'night_shade_col' by 'ao', so set 'night_shade_ao' as false
            color_sum += color;
            valid_samples++;
        }
        return color_sum / ((MULTI_SAMPLE == 2) ? MS_SAMPLES : valid_samples); // MULTI_SAMPLE=3 is used for Sky which uses Alpha Blend and needs all sub-pixels to look the same hard (no AA), but when Sky is not used, then we need to soften (have AA)
    }
}
/******************************************************************************/
