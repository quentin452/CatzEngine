/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
SunClass Sun;
Bool AstrosDraw = true;
Memc<Astro> Astros;
/******************************************************************************/
Astro::Astro() {
    draw = true;

    glow = 0;
    size = 0.15f;
    pos.set(-SQRT3_3, SQRT3_3, -SQRT3_3);
    image_color = WHITE;

    light_color_l = 0;
    light_vol = 0; // 0.2f
    light_vol_exp = 1.0f;
    light_vol_steam = 0.5f;
}
/******************************************************************************/
Vec Astro::lightColorS() C { return LinearToSRGB(light_color_l); }
void Astro::lightColorS(C Vec &color_s) { return lightColorL(SRGBToLinear(color_s)); }
/******************************************************************************/
void Astro::light() {
    if (is()) {
        Flt radius_frac = Atan(Sqr(size * 0.5f)) / PI_2; // this formula matches visual results of Sun image size vs specular reflection, use "size*0.5" because usually the image on the texture fades out smoothly from the center, instead of having full opacity until the image borders with sharp alpha cutout
        LightDir(-pos, light_color_l, radius_frac, light_vol, light_vol_exp, light_vol_steam).add(true, this);
    }
}
void Astro::Draw() {
    if (is() && Frustum(BallM(D.viewRange() * size * SQRT2, CamMatrix.pos + pos * D.viewRange()))) {
        // TODO: apply per-pixel softing based on depth buffer, exactly like particle softing (draw closer to camera, but scale XY size, along CamMatrix.xy) and modify pixel shader
        if (glow)
            Renderer._has |= HAS_GLOW;
        D.alphaFactor(VecB4(0, 0, 0, glow));
        MaterialClear(); // 'MaterialClear' must be called when changing 'D.alphaFactor'
        D.alpha(Renderer.fastCombine() ? ALPHA_BLEND : ALPHA_RENDER_BLEND_FACTOR);
        VI.image(image());
        VI.setType(VI_3D_TEX_COL, VI_STRIP);
        if (Vtx3DTexCol *v = (Vtx3DTexCol *)VI.addVtx(4)) {
            Matrix3 m;
            m.setRotation(Vec(0, 1, 0), pos); // use top as original position, because this makes the most realistic UV rotation when the object rotates around the sky
            C Vec &right = (m.x *= size),
                  &up = (m.z *= size);

            v[0].pos = pos - right + up;
            v[1].pos = pos + right + up;
            v[2].pos = pos - right - up;
            v[3].pos = pos + right - up;
            Flt depth = 0;
            REP(4)
            MAX(depth, Dot(v[i].pos, CamMatrix.z)); // project onto viewport far plane, so it's always at the back, need to find max depth of all vertexes, because if vertexes would be projected individually, then UV would be stretched unrealistically
            Flt range = D.viewRange() - 0.01f,      // make it smaller to avoid being clipped by viewport far plane
                scale = range / depth;
            REP(4)
            v[i].pos *= scale;
            v[0].color = v[1].color = v[2].color = v[3].color = image_color;
            v[0].tex.set(0, 0);
            v[1].tex.set(image->_part.x, 0);
            v[2].tex.set(0, image->_part.y);
            v[3].tex.set(image->_part.x, image->_part.y);
        }
        VI.end();
    }
}
/******************************************************************************/
SunClass::SunClass() {
    glow = 128;
    lightColorS(0.7f); // !! if changing then also change 'Environment.Sun'

    highlight_front = 0.20f; // !! if changing then also change 'Environment.Sun'
    highlight_back = 0.15f;

    rays_color = 0.12f; // !! if changing then also change 'Environment.Sun' !! don't set more than (LINEAR_FILTER ? 0.15 : 0.1) because it will require jittering
    rays_jitter = -1;   // auto
    rays_mode = SUN_RAYS_OPAQUE_BLEND;
    _rays_res = FltToByteScale(0.25f);
    _rays_mask_res = FltToByteScale(1.0f); // we can use full res because it makes a small performance difference
}
Bool SunClass::wantRays() C { return is() && rays_mode && rays_color.max() > EPS_COL8_NATIVE; }

Flt SunClass::raysRes() C { return ByteScaleToFlt(_rays_res); }
SunClass &SunClass::raysRes(Flt scale) {
    Byte res = FltToByteScale(scale);
    if (res != _rays_res) {
        _rays_res = res;
        Renderer.rtClean();
    }
    return T;
}

Flt SunClass::raysMaskRes() C { return ByteScaleToFlt(_rays_mask_res); }
SunClass &SunClass::raysMaskRes(Flt scale) {
    Byte res = FltToByteScale(scale);
    if (res != _rays_mask_res) {
        _rays_mask_res = res;
        Renderer.rtClean();
    }
    return T;
}

#pragma pack(push, 4)
struct GpuSun {
    Vec2 pos2;
    Vec pos, color;
};
#pragma pack(pop)

static INLINE Shader *GetSunRays(Bool alpha, Bool dither, Bool jitter, Bool gamma) {
    Shader *&s = Sh.SunRays[alpha][dither][jitter][gamma];
    if (SLOW_SHADER_LOAD && !s)
        s = Sh.getSunRays(alpha, dither, jitter, gamma);
    return s;
}

void SunClass::drawRays(Vec &color) {
    GpuSun sun;
    sun.pos = pos * CamMatrixInv.orn();
    sun.pos2 = D.screenToUV(_pos2);

    Flt alpha = Sat(sun.pos.z / 0.18f); // if sun is close to camera (or behind it) then decrease opacity
    if (Fog.draw && Fog.affect_sky)
        alpha *= VisibleOpacity(Fog.density, D.viewRange()); // rays are in ALPHA_ADD mode, so we need to downscale the color
    color = rays_color * alpha;
    Bool dither, gamma;

    if (Renderer._cur[0] != Renderer._col) // if rendering to a separate RT, then render with full color, so later when upscaling, we can use dithering with higher precision, during dithering the color will be set to the right value
    {
        sun.color = 1;
        dither = false; // we will do dithering later
        gamma = false;
    } else // if rendering to the main target, then use final color
    {
        sun.color = color;
        dither = (D.dither() && !Renderer._cur[0]->highPrecision());
        gamma = LINEAR_GAMMA; // can't do 'swapRTV' here, because when operating in linear color space, the effect looks totally different
    }

    REPS(Renderer._eye, Renderer._eye_num) {
        Rect *rect = Renderer.setEyeParams(), ext_rect;
        if (!D._view_main.full) {
            ext_rect = *rect;
            rect = &ext_rect.extend(Renderer.pixelToScreenSize(1 /*+(rays_soft?SHADER_BLUR_RANGE:0)*/));
        } // always extend by 1 because we may apply this RT to the final RT usin linear filtering
        if (Renderer._stereo) {
            sun.pos = pos;
            sun.pos.divNormalized(ActiveCam.matrix.orn());
            sun.pos2 = D.screenToUV(_pos2);
            sun.pos2.x += ProjMatrixEyeOffset[Renderer._eye] * 0.5f;
            sun.pos2.x *= 0.5f;
            if (Renderer._eye)
                sun.pos2.x += 0.5f;
        }
        Sh.Sun->set(sun);
        Bool jitter = ((rays_jitter < 0) ? rays_color.max() > (LINEAR_GAMMA ? 0.15f : 0.1f) + EPS_COL8_NATIVE : rays_jitter != 0); // for auto, enable jittering only if rays have a high brightness
        if (Renderer._alpha) {
            Sh.ImgX[0]->set(Renderer._alpha);
            GetSunRays(true, dither, jitter, gamma)->draw(rect);
        } else {
            GetSunRays(false, dither, jitter, gamma)->draw(rect);
        }
    }
}
/******************************************************************************/
void AstroPrepare() {
    Sun._actual_rays_mode = SUN_RAYS_OFF;

    if (AstrosDraw) {
        if (Sun.wantRays() && Renderer.canReadDepth() && !Renderer.mirror() && PosToFullScreen(CamMatrix.pos + Sun.pos * D.viewRange(), Sun._pos2) && FovPerspective(D.viewFovMode()) && !(Fog.draw && Fog.affect_sky && VisibleOpacity(Fog.density, D.viewRange()) <= EPS_COL8_NATIVE) // if fog is too dense then don't draw sun rays
        )
            Sun._actual_rays_mode = ((Sun.rays_mode == SUN_RAYS_OPAQUE_BLEND && D._max_rt >= 2) ? SUN_RAYS_OPAQUE_BLEND : SUN_RAYS_OPAQUE); // Alpha RT is on #1 #RTOutput.Blend
        Sun.light();
        FREPAO(Astros).light();
    }
}
void AstroDraw() {
    if (AstrosDraw) {
        // !! THIS MUST NOT MODIFY 'Renderer._alpha' BECAUSE THAT WOULD DISABLE SUN RAYS !!
        Renderer.set(Renderer._col, Renderer._ds, true); // use DS for depth tests
        SetOneMatrix(MatrixM(CamMatrix.pos));            // normally we have to set matrixes after 'setEyeViewportCam', however since matrixes are always relative to the camera, and here we set exactly at the camera position, so the matrix will be the same for both eyes
        Sh.SkyFracMulAdd->set(Vec2(0, 1));               // astronomical objects are drawn as billboards which make use of sky fraction, so be sure to disable it before drawing
        D.depthOnWriteFunc(true, false, FUNC_LESS_EQUAL);
        REPS(Renderer._eye, Renderer._eye_num) {
            Renderer.setEyeViewportCam();
            Sun.Draw();
            FREPAO(Astros).Draw();
        }
        D.depthOnWriteFunc(false, true, FUNC_DEFAULT);
    }
}
void AstroDrawRays() {
    if (Sun._actual_rays_mode) {
        const VecI2 res = ByteScaleRes(Renderer.fx(), Sun._rays_res);
        ImageRTPtr rt0;
        if (res != Renderer._col->size())
            rt0.get(ImageRTDesc(res.x, res.y, IMAGERT_ONE));

        Renderer.set(rt0 ? rt0 : Renderer._col, null, rt0 == null); // set viewport only when drawing to 'Renderer._col'
        SetOneMatrix();
        D.alpha(rt0 ? ALPHA_NONE : ALPHA_ADD); // if drawing to rt0 then SET, if drawing to Renderer._col then ADD

        Vec color;
        Sun.drawRays(color);
        if (rt0) {
            /*if(Sun.rays_soft && shift>=2)
              {
                 if(!Sh.BlurX_X)
                 {
                    Sh.BlurX_X=Sh.get("BlurX_X");
                    Sh.BlurY_X=Sh.get("BlurY_X");
                 }
                 D.alpha(ALPHA_NONE);
                 rt1.get(ImageRTDesc(Renderer.fxW()>>shift, Renderer.fxH()>>shift, IMAGERT_ONE));
               //Sh.imgSize(*rt0); we can just use 'RTSize' instead of 'ImgSize' since there's no scale
                 Renderer.set(rt1, null, false); Sh.ImgX->set(rt0); Sh.BlurX_X->draw(&D.viewRect());
                 Renderer.set(rt0, null, false); Sh.ImgX->set(rt1); Sh.BlurY_X->draw(&D.viewRect());
              }*/
            Renderer.set(Renderer._col, null, true);
            D.alpha(ALPHA_ADD);
            Sh.Color[0]->set(Vec4(color, 0)); // we've rendered the sun with full intensity, so now we need to make it darker
            Sh.Color[1]->set(Vec4Zero);
            Bool dither = (D.dither() && !Renderer._col->highPrecision()); // don't do dithering for high precision RT
            Shader *shader;
            Sh.ImgX[0]->set(rt0);
            // if(Sun.rays_soft && shift==1){shader=Sh.SunRaysSoft; Sh.imgSize(*rt0);}else
            shader = Sh.DrawXC[dither][LINEAR_GAMMA];
            REPS(Renderer._eye, Renderer._eye_num)
            shader->draw(Renderer._stereo ? &D._view_eye_rect[Renderer._eye] : &D.viewRect());
        }
        if (!Renderer.processAlphaFinal())
            Renderer._alpha.clear(); // if we don't need alpha anymore then clear it
    }
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
