/******************************************************************************/
#include "stdafx.h"
#define TEST_LIGHT_RECT (DEBUG && 0)
#define FLAT_SHADOW_MAP 1                 // TODO try implementing Cube Shadow Maps?
#define LOCAL_SHADOW_MAP_FROM (1.0f / 64) // 0.015625m, 1.5 cm

#define LIGHT_MESH_BALL_RES 2 // res 2 gives 'dist'=0.971646905, make radius bigger to make sure ball covers all pixels with radius=1, use only res 2, to avoid having too many triangles/edges because GPU's have to process pixels always in 2x2 blocks, so due to edges, some pixels are wasted
#define LIGHT_MESH_BALL_RADIUS (1 / 0.971646905f)

#define LIGHT_MESH_CONE_RES 16 // res 16 gives 'dist'=0.980785191, make radius bigger to make sure cone covers all pixels with radius=1, use only res 16, to avoid having too many triangles/edges because GPU's have to process pixels always in 2x2 blocks, so due to edges, some pixels are wasted
#define LIGHT_MESH_CONE_RADIUS (1 / 0.980785191f)

#define ALWAYS_RESTORE_FRUSTUM 0 // 0=skip (faster)
/* !! WARNING: For performance reasons 'Frustum' is not restored after (drawing shadows AND custom frustum for forward local lights), but only after all lights finished
   !! instead we set 'Frustum' only when needed
   !! however when processing lights we have to use 'toScreenRect' which needs a frustum, since we don't restore it, those functions were made to always use 'FrustumMain'
   !! because of that we can't use 'toScreenRect' for shadows to get the rect in shadow RT
   Camera however is always reset after drawing shadows */

#if DX12
use ID3D12GraphicsCommandList1::OMSetDepthBounds to process only shadow / light pixels within depth ranges(including drawForward secondary pass)
                                                                              https : // docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist1-omsetdepthbounds
#endif

                                                                                      namespace EE {
    /******************************************************************************/
    static inline Bool NeedBackgroundLum() { return !Sky.isActual() || Renderer.stage == RS_LIGHT || Renderer.stage == RS_LIGHT_AO || Renderer.stage == RS_LIT_COLOR; }
    /******************************************************************************

       Hardware Shadow Map RT is created with resolution:
          x=D.shadowMapSizeActual()*2
          y=D.shadowMapSizeActual()*3

       Like this (2x3):
          SS
          SS
          SS

       1. Shadows are set for CurrentLight.rect (2D)
       2. Shadows are blurred using 3D Geom Mesh shaders (3D) (except first pass for 2-pass blurs)
       3. Light is calculated using 3D Geom Mesh shaders

       if DEPTH_CLIP_SUPPORTED not supported and it wanted to be used, then Cone Lights are processed as 2D shaders,
          Point/Linear/Ball Lights are ignored because clipped artifacts (using simulated clipping in the shader) are minimal.

       TODO: calculating shadows could use 3D Geom Mesh shaders if shadow map softing is disabled

       TODO: currently Cone Light for Forward renderer, set Frustum based on light volume only,
          however this should be intersected with FrustumMain, to don't draw objects that are within light, but outside of view.
          Try to don't slow down Deferred renderer when implementing this.

    /******************************************************************************/
    static Matrix ShdMatrix[2];      //           [Eye]
    static Matrix4 ShdMatrix4[6][2]; // [MapIndex][Eye]
    static Matrix4 HsmMatrix, HsmMatrixCone;
    Memc<Light> Lights;
    Light CurrentLight;
    static Bool CurrentLightOn[2];
    static Rect CurrentLightRect[2];
    static Vec2 CurrentLightZRange; // Current Light Z range, relative to Camera position, to be used for 'OMSetDepthBounds'. Dbl precision not required since it's relative to Camera
    static MeshRender LightMeshBall, LightMeshCone;
    static Int LightMode;
    /******************************************************************************/
    static inline Int HsmX(DIR_ENUM dir) { return dir & 1; }
    static inline Int HsmY(DIR_ENUM dir) { return dir >> 1; }
    static inline Matrix4 &HsmMatrixSet(DIR_ENUM dir) {
        Int x = HsmX(dir),
            y = HsmY(dir);
        HsmMatrix.pos.x = (x ? 0.75f : 0.25f);
        HsmMatrix.pos.y = Avg(0, 1.0f / 3) + (1.0f / 3) * y;
        return HsmMatrix;
    }
    static inline void MatrixFovScaleX(Matrix & matrix, Flt scale) {
        matrix.x.x /= scale;
        matrix.y.x /= scale;
        matrix.z.x /= scale;
        matrix.pos.x /= scale;
    }
    static inline void MatrixFovScaleY(Matrix & matrix, Flt scale) {
        matrix.x.y /= scale;
        matrix.y.y /= scale;
        matrix.z.y /= scale;
        matrix.pos.y /= scale;
    }
    static inline void MatrixFovScale(Matrix & matrix, Flt scale) {
        MatrixFovScaleX(matrix, scale);
        MatrixFovScaleY(matrix, scale);
    }
    /******************************************************************************/
    static inline void SetShadowOpacity(Flt opacity) { Sh.ShdOpacity->set(Vec2(opacity, 1 - opacity)); } // shd=Lerp(1, shd, shadow_opacity) -> shd=1*(1-shadow_opacity) + shd*(shadow_opacity) -> shd=shd*shadow_opacity + 1-shadow_opacity

    static inline void SetShdMatrix() {
        Sh.ShdMatrix->set(ShdMatrix[Renderer._eye]);
        REP(6)
        Sh.ShdMatrix4[i]->set(ShdMatrix4[i][Renderer._eye]);
    }
    /******************************************************************************/
    static Flt ShadowStep(Int i, Int num) // 0..1
    {
        if (i == 0)
            return 0;
        if (i == num)
            return 1;
        if (D.shadow_step)
            return D.shadow_step(i, num);
        Flt s = Flt(i) / num;
        return Pow(s, D.shadowMapSplit().x + D.shadowMapSplit().y * s); // Pow(s, 2) == s*s produces results nearly identical to "Lerp(1/((1-s)*D.viewRange()), s, s);"
    }
    /******************************************************************************/
    static void ApplyViewSpaceBias(Flt & mp_z_z) {
        if (FovPerspective(D.viewFovMode())) // needed only for perspective because it can produce big errors
        {                                    // 'ProjMatrix' here is from main view
            mp_z_z = ProjMatrix.z.z;
            // #ShadowBias
            ProjMatrix.z.z += (REVERSE_DEPTH ? -1.0 : 1.0) / (1ull << Renderer._ds->hwTypeInfo().d); // this adjusts the value that is responsible for 'LinearizeDepth' by moving everything back by 1 value (in depth buffer bit precision)
            if (ProjMatrix.z.z == mp_z_z)                                                            // if adding hasn't changed anything, then change by 1 bit (this can happen for small values)
                if (REVERSE_DEPTH)
                    DecRealByBit(ProjMatrix.z.z);
                else
                    IncRealByBit(ProjMatrix.z.z);
            SetProjMatrix();
        }
    }
    static void RestoreViewSpaceBias(Flt mp_z_z) {
        if (FovPerspective(D.viewFovMode())) {
            ProjMatrix.z.z = mp_z_z;
            SetProjMatrix();
        }
    }
    /******************************************************************************/
    void RendererClass::getShdRT() { // always do 'get' to call 'discard', do "ImgX[0]->set" it will be used by drawing lights 'Light.draw, drawForward' (GetDrawLight*->draw) and 'MapSoft'
        {
            Renderer._shd_1s.get(ImageRTDesc(Renderer._ds_1s->w(), Renderer._ds_1s->h(), IMAGERT_ONE));
            Sh.ImgX[0]->set(Renderer._shd_1s);
        }
        if (Renderer._ds->multiSample()) {
            Renderer._shd_ms.get(ImageRTDesc(Renderer._ds->w(), Renderer._ds->h(), IMAGERT_ONE, Renderer._ds->samples()));
            Sh.ImgXMS->set(Renderer._shd_ms);
        }
        D.alpha(ALPHA_NONE);
    }
    /******************************************************************************/
    static void ClearLumSeparate(C Vec4 & lum_color, C ImageRTPtr & lum, C ImageRTPtr & spec) { // clear from last to first to minimize RT changes, 'clearViewport' ignores depth usage for 'D.depth2D'
        spec->clearViewport(Vec4Zero);
        lum->clearViewport(lum_color);
    }
    static void ClearLumMerged(C Vec4 & lum_color) {
        if (D._view_main.full) { // 'D.clearCol(Int i' ignores depth usage for 'D.depth2D'
            D.clearCol(0, lum_color);
            D.clearCol(1, Vec4Zero);
        } else {
            if (!NeedBackgroundLum())
                D.depth2DOn();
            else
                D.depth2DOff(); // if don't need background lum, then enable depth usage so we can clear light only for valid pixels
            D.alpha(ALPHA_NONE);
            Sh.Color[0]->set(lum_color);
            Sh.ClearLight->draw();
            // D.depth2DOff(); ignore this because 'D.depth' is set always when needed
        }
    }
    /******************************************************************************/
    static inline Bool MergedClearLum() { return true; } // here always use "true" instead of "D.mergedClear()" to enable merged clear, because performance is the same and this workarounds Nvidia GeForce flickering bug - https://devtalk.nvidia.com/default/topic/1068770/directx-and-direct-compute/dx11-driver-bug-significant-flickering-on-geforce/
    static void GetLum() {
        ImageRTDesc rt_desc(Renderer._col->w(), Renderer._col->h(), D.highPrecLumRT() ? IMAGERT_SRGB_H : IMAGERT_SRGB, Renderer._col->samples());
        Renderer._lum.get(rt_desc);
        Renderer._spec.get(rt_desc);
        rt_desc.samples = 1;
        if (Renderer._lum->multiSample())
            Renderer._lum_1s.get(rt_desc);
        else
            Renderer._lum_1s = Renderer._lum;
        if (Renderer._spec->multiSample())
            Renderer._spec_1s.get(rt_desc);
        else
            Renderer._spec_1s = Renderer._spec;
    }
    static Bool SetLum() {
        Vec4 lum_color;
        Bool merged_clear;
        Bool clear = !Renderer._lum;
        if (clear) {
            GetLum();
            lum_color = (Renderer.ambientInLum() ? Vec4(D.ambientColorD(), 0) : Vec4Zero);
            merged_clear = MergedClearLum();
            if (!merged_clear)
                ClearLumSeparate(lum_color, Renderer._lum_1s, Renderer._spec_1s);
        }
        Renderer.set(Renderer._lum_1s, Renderer._spec_1s, null, null, Renderer._ds_1s, true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth' optimization, 3D geometric shaders and stencil tests, start with '_lum_1s' so '_lum' will be processed later, because at the end we still have to render ambient from 3d meshes to '_lum' this way we avoid changing RT's
        if (clear && merged_clear)
            ClearLumMerged(lum_color);
        D.alpha(ALPHA_ADD);
        Sh.Img[0]->set(Renderer._nrm);
        Sh.ImgMS[0]->set(Renderer._nrm);
        Sh.Img[1]->set(Renderer._col);
        Sh.ImgMS[1]->set(Renderer._col);
        Sh.ImgXY[0]->set(Renderer._ext);
        Sh.ImgXYMS->set(Renderer._ext);
        return clear;
    }
    static void SetLumMS(Bool clear) {
        Vec4 lum_color;
        Bool merged_clear;
        if (clear) {
            D.stencil(STENCIL_NONE);
            lum_color = (Renderer.ambientInLum() ? Vec4(D.ambientColorD(), 0) : Vec4Zero);
            merged_clear = MergedClearLum();
            if (!merged_clear)
                ClearLumSeparate(lum_color, Renderer._lum, Renderer._spec);
        }
        Renderer.set(Renderer._lum, Renderer._spec, null, null, Renderer._ds, true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth' optimization, 3D geometric shaders and stencil tests
        if (clear && merged_clear)
            ClearLumMerged(lum_color);
        D.alpha(ALPHA_ADD);
    }
    void RendererClass::getLumRT() // this is called after drawing all lights, in order to make sure we have some RT's (in case there are no lights), after this ambient meshes will be drawn
    {
        if (!_lum) {
            GetLum();
            Vec4 lum_color = (Renderer.ambientInLum() ? Vec4(D.ambientColorD(), 0) : Vec4Zero);
            Bool merged_clear = MergedClearLum();
            if (!merged_clear) {
                if (_lum_1s != _lum)
                    ClearLumSeparate(Vec4Zero, _lum_1s, _spec_1s);
                ClearLumSeparate(lum_color, _lum, _spec);
            } else {
                if (_lum_1s != _lum) {
                    set(_lum_1s, _spec_1s, null, null, _ds_1s, true, NEED_DEPTH_READ);
                    ClearLumMerged(Vec4Zero);
                }
                set(_lum, _spec, null, null, _ds, true, NEED_DEPTH_READ);
                ClearLumMerged(lum_color);
            }
        }
    }
    /******************************************************************************/
    static void GetWaterLum() {
        ImageRTDesc rt_desc(Renderer._water_ds->w(), Renderer._water_ds->h(), IMAGERT_SRGB);
        Renderer._water_lum.get(rt_desc);
        Renderer._water_spec.get(rt_desc);
        Water.set(); // set main water material to set default "smoothness, reflectivity" in case we don't use '_water_ext' #WaterExt
    }
    static void SetWaterLum() {
        Vec4 lum_color;
        Bool merged_clear;
        Bool clear = !Renderer._water_lum;
        if (clear) {
            GetWaterLum();
            lum_color = Vec4(D.ambientColorD(), 0);
            merged_clear = MergedClearLum();
            if (!merged_clear)
                ClearLumSeparate(lum_color, Renderer._water_lum, Renderer._water_spec);
        }
        Renderer.set(Renderer._water_lum, Renderer._water_spec, null, null, Renderer._water_ds, true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth' optimization, 3D geometric shaders and stencil tests
        if (clear && merged_clear)
            ClearLumMerged(lum_color);
        D.alpha(ALPHA_ADD);
        Sh.Img[0]->set(Renderer._water_nrm);
        Sh.ImgMS[0]->set(Renderer._water_nrm);
        // Sh.Img  [1]->set(Renderer._water_col); Sh.ImgMS[1]->set(Renderer._water_col); ignored for water and copied from water reflectivity #WaterExt
        // Sh.ImgXY[0]->set(Renderer._water_ext); Sh.ImgXYMS ->set(Renderer._water_ext); Water doesn't have EXT #WaterExt
    }
    void RendererClass::getWaterLumRT() {
        if (!_water_lum) {
            GetWaterLum();
            Vec4 lum_color = Vec4(D.ambientColorD(), 0);
            Bool merged_clear = MergedClearLum();
            if (!merged_clear) {
                ClearLumSeparate(lum_color, _water_lum, _water_spec);
            } else {
                set(_water_lum, _water_spec, null, null, _ds, true, NEED_DEPTH_READ);
                ClearLumMerged(lum_color);
            }
        }
    }
    /******************************************************************************/
    static void MapSoft(UInt depth_func = FUNC_FOREGROUND, C MatrixM *light_matrix = null) {
        if (D.shadowSoft()) // !! 'D.shadowSoft' values need to be in sync with 'D.ambientSoft' !!
        {
            ImageRTDesc rt_desc(Renderer._shd_1s->w(), Renderer._shd_1s->h(), IMAGERT_ONE);
            // Sh.imgSize(*Renderer._shd_1s); we can just use 'RTSize' instead of 'ImgSize' since there's no scale
            Bool geom = (CurrentLight.type != LIGHT_DIR);
#if !DEPTH_CLIP_SUPPORTED                                                                                  // Flat
            if (CurrentLight.type == LIGHT_CONE && !D._depth_clip && CurrentLightZRange.y > D.viewRange()) // if light is behind far plane
                geom = false;                                                                              // draw as Flat
#endif

            MeshRender *mesh;
            if (geom)
                mesh = ((CurrentLight.type == LIGHT_CONE) ? &LightMeshCone : &LightMeshBall);
            if (D.shadowSoft() >= 5) // use 2 pass blur (BlurX, BlurY)
            {
                // first pass has to be flat (!geom) and cover full screen, because second pass gathers nearby pixels
                ImageRTPtr temp(rt_desc);
                Renderer.set(temp, Renderer._ds_1s, true, NEED_DEPTH_READ);
                REPS(Renderer._eye, Renderer._eye_num)
                if (CurrentLightOn[Renderer._eye])
                    Sh.ShdBlurX[0][0]->draw(&CurrentLightRect[Renderer._eye]); // use DS because it may be used for 'D.depth' optimization and 3D geometric shaders
                Renderer._shd_1s->discard();
                Renderer.set(Renderer._shd_1s, Renderer._ds_1s, true, NEED_DEPTH_READ);
                Sh.ImgX[0]->set(temp); // use DS because it may be used for 'D.depth' optimization and 3D geometric shaders
                Shader *shader = Sh.ShdBlurY[geom][0];
                if (geom) {
                    D.depth2DOn(depth_func);
                    shader->startTex();
                    mesh->set();
                }
                REPS(Renderer._eye, Renderer._eye_num)
                if (CurrentLightOn[Renderer._eye]) {
                    if (geom) {
                        Renderer.setEyeViewportCam();
                        SetFastMatrix(*light_matrix);
                        shader->commit();
                        mesh->draw();
                    } else
                        shader->draw(&CurrentLightRect[Renderer._eye]);
                }
            } else // use single pass
            {
                ImageRTPtr src = Renderer._shd_1s;
                Renderer._shd_1s.get(rt_desc);
                Renderer.set(Renderer._shd_1s, Renderer._ds_1s, true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth' optimization and 3D geometric shaders
                Shader *shader = Sh.ShdBlur[geom][0][D.shadowSoft() - 1];
                if (geom) {
                    D.depth2DOn(depth_func);
                    shader->startTex();
                    mesh->set();
                }
                REPS(Renderer._eye, Renderer._eye_num)
                if (CurrentLightOn[Renderer._eye]) {
                    if (geom) {
                        Renderer.setEyeViewportCam();
                        SetFastMatrix(*light_matrix);
                        shader->commit();
                        mesh->draw();
                    } else
                        shader->draw(&CurrentLightRect[Renderer._eye]);
                }
            }
            Sh.ImgX[0]->set(Renderer._shd_1s);
        }
    }
    static void RestoreShadowMapSettings() {
        D._view_active.set3DFrom(D._view_main).setProjMatrix();
        SetCam(ActiveCam.matrix);
        if (ALWAYS_RESTORE_FRUSTUM)
            Frustum = FrustumMain;
    }
    /******************************************************************************/
    LightCone::LightCone(Flt length, C VecD & pos, C Vec & dir, C Vec & color_l, Flt vol, Flt vol_max) {
        T.pyramid.scale = 1;
        T.pyramid.h = length;
        T.pyramid.setPosDir(pos, dir);
        T.color_l = color_l;
        T.falloff = 0.5f;
        T.vol = vol;
        T.vol_max = vol_max;
    }
    /******************************************************************************/
    Flt LightPoint::range() C {
        return Sqrt(power / (LINEAR_GAMMA ? 1.0f / 1024 : 1.0f / 256));
        /* Use 1024 for LINEAR_GAMMA, because with that value, sRGB at light range is ~3.2 on 0..255 scale
           LightPoint lp(1, 0);
           Flt  light_range=lp.range();
           Flt  light_intensity=lp.power/Sqr(light_range), expected_light_intensity=1.0f/1024;
           Flt  albedo=1, srgb_col=LinearToSRGB(light_intensity*albedo)*255; */
    }
    /******************************************************************************/
    void LightDir ::add(Bool shadow, CPtr light_src, Bool allow_main) {
        DEBUG_ASSERT(Renderer() == RM_PREPARE, "'LightDir.add' called outside of RM_PREPARE");
        if (color_l.max() > EPS_COL8_LINEAR && Renderer.firstPass()) {
            Lights.New().set(T, shadow, light_src, allow_main);
        }
    }
    void LightPoint ::add(Flt shadow_opacity, CPtr light_src) {
        Rect rect;
        DEBUG_ASSERT(Renderer() == RM_PREPARE, "'LightPoint.add' called outside of RM_PREPARE");
        if (color_l.max() > EPS_COL8_LINEAR && power > EPS && toScreenRect(rect) && Renderer.firstPass()) {
            Lights.New().set(T, rect, shadow_opacity, light_src);
        }
    }
    void LightLinear::add(Flt shadow_opacity, CPtr light_src) {
        Rect rect;
        DEBUG_ASSERT(Renderer() == RM_PREPARE, "'LightLinear.add' called outside of RM_PREPARE");
        if (color_l.max() > EPS_COL8_LINEAR && range > EPS && toScreenRect(rect) && Renderer.firstPass()) {
            Lights.New().set(T, rect, shadow_opacity, light_src);
        }
    }
    void LightCone ::add(Flt shadow_opacity, CPtr light_src, Image * image, Flt image_scale) {
        Rect rect;
        DEBUG_ASSERT(Renderer() == RM_PREPARE, "'LightCone.add' called outside of RM_PREPARE");
        if (color_l.max() > EPS_COL8_LINEAR && pyramid.h > EPS && toScreenRect(rect) && Renderer.firstPass()) {
            Light &l = Lights.New();
            l.set(T, rect, shadow_opacity, light_src);
            l.image = image;
            l.image_scale = image_scale;
        }
    }
/******************************************************************************/
#pragma pack(push, 4)
    struct GpuLightDir {
        Vec dir;
        Vec color;
        Flt vol, vol_exponent, vol_steam;
        Flt radius_frac;
    };
    struct GpuLightPoint {
        Flt power,
            radius;
        Vec pos;
        Flt lum_max,
            vol, vol_max;
        Vec color;
    };
    struct GpuLightLinear {
        Flt neg_inv_range,
            radius;
        Vec pos;
        Vec color;
        Flt vol, vol_max;
    };
    struct GpuLightCone {
        Flt neg_inv_range,
            radius;
        Vec2 falloff;
        Vec pos;
        Vec color;
        Matrix3 mtrx;
        Flt scale,
            vol, vol_max;
    };
#pragma pack(pop)
    void LightDir::set() {
        GpuLightDir l;
        l.dir.fromDivNormalized(dir, CamMatrix.orn()).chs();
        l.color = LinearToDisplay(color_l);
        l.radius_frac = radius_frac;
        l.vol = vol;
        l.vol_exponent = vol_exponent;
        l.vol_steam = vol_steam;
        Sh.LightDir->set(l);
    }
    void LightPoint::set(Flt shadow_opacity) {
        GpuLightPoint l;
        l.power = power;
        l.lum_max = lum_max;
        l.vol = vol * shadow_opacity;
        l.vol_max = vol_max;
        l.pos.fromDivNormalized(pos, CamMatrix);
        l.color = LinearToDisplay(color_l);
        Sh.LightPoint->set(l);
    }
    void LightLinear::set(Flt shadow_opacity) {
        GpuLightLinear l;
        l.neg_inv_range = -1 / range;
        l.vol = vol * shadow_opacity;
        l.vol_max = vol_max;
        l.pos.fromDivNormalized(pos, CamMatrix);
        l.color = LinearToDisplay(color_l);
        Sh.LightLinear->set(l);
    }
    void LightCone::set(Flt shadow_opacity) {
        GpuLightCone l;
        // angular intensity = Length(pos)*l.falloff.x+l.falloff.y             falloff=0 Y|\         falloff=1 Y|  |
        // falloff=0         ->            l.falloff.x=-1    l.falloff.y=1                | \                   |  |
        // falloff=1         ->            l.falloff.x=-Inf  l.falloff.y=Inf              +--\X                 +--|X
        l.falloff.x = -1.0f / Mid(falloff, EPS, 1.0f);
        l.falloff.y = -l.falloff.x;
        l.vol = vol * shadow_opacity;
        l.vol_max = vol_max;
        l.color = LinearToDisplay(color_l);
        l.neg_inv_range = -1 / pyramid.h;
        l.scale = pyramid.scale;
        l.mtrx.x = -pyramid.cross() / pyramid.scale;
        l.mtrx.y = -pyramid.perp / pyramid.scale;
        l.mtrx.z = -pyramid.dir;
        l.mtrx.divNormalized(CamMatrix.orn());
        l.pos.fromDivNormalized(pyramid.pos, CamMatrix);
        Sh.LightCone->set(l);
    }
    /******************************************************************************/
    enum SHADOW_MAP_FLAG {
        SM_FRUSTUM = 0x1, // perform frustum tests
        SM_CLOUDS = 0x2,
    };
    struct MatrixFovFrac {
        Vec2 fov; // no need for 'VecD2'
        MatrixM matrix;
        Rect frac; // 0..1 (fraction of the viewport that we actually need)
    };
    static void DrawShadowMap(DIR_ENUM dir, C MatrixM & cam_matrix, UInt flag, Flt view_from, Flt view_range, C Vec2 & fov, FOV_MODE fov_mode, Int border, Flt bias, C MatrixFovFrac *frustum = null) {
#if FLAT_SHADOW_MAP
        {
            // apply cam matrix bias
            MatrixM cam_matrix_biased = cam_matrix;
            if (CurrentLight.type == LIGHT_DIR)
                cam_matrix_biased -= cam_matrix.z * bias;

            // set viewport
            RectI viewport_rect;
            if (CurrentLight.type == LIGHT_CONE) {
                viewport_rect.set(0, 0, D.shadowMapSizeActual() * 2, D.shadowMapSizeActual() * 2); // cone lights use 2x shadow map size
            } else {
                Int x = HsmX(dir),
                    y = HsmY(dir);
                viewport_rect.set(x * D.shadowMapSizeActual(), y * D.shadowMapSizeActual(), (x + 1) * D.shadowMapSizeActual(), (y + 1) * D.shadowMapSizeActual());
            }
            if (frustum) // in 'D.shadowReduceFlicker', the needed area may not be the entire viewport rect, so use scissor
                D.clipAllow(RectI(Floor(viewport_rect.lerpX(frustum->frac.min.x)),
                                  Floor(viewport_rect.lerpY(frustum->frac.min.y)),
                                  Ceil(viewport_rect.lerpX(frustum->frac.max.x)),
                                  Ceil(viewport_rect.lerpY(frustum->frac.max.y))) &
                            viewport_rect); // AND with original viewport in case frac is slightly <0 || >1 or after floor/ceil we get +-1 values

            viewport_rect.extend(-border);
            D._view_active.set(viewport_rect, view_from, view_range, fov, fov_mode);
            SetCam(cam_matrix_biased);
            if (frustum)
                Frustum.set(D._view_active.range, frustum->fov, frustum->matrix);
            else
                Frustum.set(D._view_active.range, D._view_active.fov, cam_matrix);
            if (flag & SM_FRUSTUM)
                if (!FrustumMain(Frustum))
                    return;                               // check if shadow frustum is visible (lies in main camera view frustum)
            D._view_active.setViewport().setProjMatrix(); // we set 'Frustum' above

            // set matrix converting from shadow map to main camera space (required for tesselation - adaptive tesselation factors)
            Matrix temp;
            cam_matrix.divNormalized(ActiveCam.matrix, temp); // temp = cam_matrix/ActiveCam.matrix
            Sh.ShdMatrix->set(temp);

            D.depthBias(BIAS_SHADOW);
            D.depth(true);
            PrepareShadowInstances();
            Renderer._render();
            DrawShadowInstances();
            D.depthBias(BIAS_ZERO);

            if ((flag & SM_CLOUDS) && Renderer._cld_map.is()) // clouds are only for directional lights
            {
                Int x = HsmX(dir),
                    y = HsmY(dir);
                ImageRT *rt = Renderer._cur[0], *rtz = Renderer._cur_ds;
                Renderer.set(&Renderer._cld_map, null, false);
                D._view_active.set(RectI(x * D.cloudsMapSize(), y * D.cloudsMapSize(), (x + 1) * D.cloudsMapSize(), (y + 1) * D.cloudsMapSize()), view_from, view_range, fov, fov_mode).setViewport().setProjMatrix(); // viewport
                SetCam(cam_matrix);                                                                                                                                                                                    // camera only, frustum not needed for cloud shadows
                Clouds.shadowMap();
                Renderer.set(rt, rtz, false);
            }
        }
#else
    {
        Renderer.setCube(null, &Renderer._shd_map, dir);
        D._view_active.set(RectI(0, 0, Renderer.resW(), Renderer.resH()), view_from, view_range, fov, fov_mode).setViewport().setProjMatrix(); // viewport
        SetCam(cam_matrix);
        if (frustum)
            Frustum.set(D._view_active.range, frustum->fov, frustum->matrix);
        else
            Frustum.set(D._view_active.range, D._view_active.fov, cam_matrix);
        if (flag & SM_FRUSTUM)
            if (!FrustumMain(Frustum))
                return;
        Sh.clear(Vec4(D._view_active.range * 2));
        D.clearDepth();

        D.depthBias(BIAS_SHADOW);
        D.depth(true);
        PrepareShadowInstances();
        Renderer._render();
        DrawShadowInstances();
        D.depthBias(BIAS_ZERO);

        if ((flag & SM_CLOUDS) && Renderer._cld_map.is()) {
            Renderer.setCube(Renderer._cld_map, null, dir);
            D._view_active.set(RectI(0, 0, Renderer.resW(), Renderer.resH()), view_from, view_range, fov, fov_mode).setViewport().setProjMatrix(); // viewport
            SetCam(cam_matrix);                                                                                                                    // camera only, frustum not needed for cloud shadows
            Clouds.shadowMap();
        }
    }
#endif
    }
    /******************************************************************************/
    static void StartVol() {
        Bool clear = false;
        if (!Renderer._vol) {
            Renderer._vol.get(ImageRTDesc(Renderer.fxW() >> 2, Renderer.fxH() >> 2, IMAGERT_SRGB)); // doesn't use Alpha
            clear = true;
            VL.load();
        }
        D.alpha(ALPHA_ADD);
        Renderer.set(Renderer._vol, null, false);
        if (clear)
            D.clearCol(); // no need to disable 'D.depth' optimization because we're not setting Depth buffer

        if (Renderer._stereo) // for stereoscopic we need to set correct eye, otherwise it was already set
        {
            // SetCam(EyeMatrix[Renderer._eye]); this is not needed because all positions in shaders are based on ShdMatrix, 'Frustum' not needed too
            SetShdMatrix();
            Renderer.setEyeParams();
        }
        CurrentLight.set(); // call after setting the camera
    }
    static void ApplyVolumetric(LightDir & light, Int shd_map_num, Bool cloud_vol) {
        if (Renderer.hasVolLight() && light.vol > EPS_COL8_NATIVE && shd_map_num > 0)
            REPS(Renderer._eye, Renderer._eye_num)
        if (CurrentLightOn[Renderer._eye]) {
            StartVol();
            VL.VolDir[shd_map_num - 1][cloud_vol]->draw(Renderer._vol, &CurrentLightRect[Renderer._eye]);
        }
    }
    static void ApplyVolumetric(LightPoint & light) {
        if (Renderer.hasVolLight() && light.vol > EPS_COL8_NATIVE)
            REPS(Renderer._eye, Renderer._eye_num)
        if (CurrentLightOn[Renderer._eye]) {
            StartVol();
            VL.VolPoint->draw(Renderer._vol, &CurrentLightRect[Renderer._eye]);
        }
    }
    static void ApplyVolumetric(LightLinear & light) {
        if (Renderer.hasVolLight() && light.vol > EPS_COL8_NATIVE)
            REPS(Renderer._eye, Renderer._eye_num)
        if (CurrentLightOn[Renderer._eye]) {
            StartVol();
            VL.VolLinear->draw(Renderer._vol, &CurrentLightRect[Renderer._eye]);
        }
    }
    static void ApplyVolumetric(LightCone & light) {
        if (Renderer.hasVolLight() && light.vol > EPS_COL8_NATIVE)
            REPS(Renderer._eye, Renderer._eye_num)
        if (CurrentLightOn[Renderer._eye]) {
            StartVol();
            VL.VolCone->draw(Renderer._vol, &CurrentLightRect[Renderer._eye]);
        }
    }
    /******************************************************************************/
    static Bool StereoCurrentLightRect() // this relies on current Viewport, Camera Matrix and 'Frustum' (!! Actually right now 'toScreenRect' are based on 'FrustumMain' so we don't have to restore 'Frustum' !!)
    {
        if (!CurrentLight.toScreenRect(CurrentLight.rect))
            return false;
        return ToEyeRect(CurrentLight.rect);
    }
    static Bool GetCurrentLightRect() {
        return Renderer._stereo ? StereoCurrentLightRect() : true; // for non-stereo the rect is already valid (if light was not visible then it wouldn't be added to the light list)
    }
    static Bool SetLightEye(Bool shadow = false) {
        CurrentLightOn[Renderer._eye] = false;
        if (Renderer._stereo) {
            Renderer.setEyeViewportCam();
            if (!StereoCurrentLightRect())
                return false;
        }
        if (shadow) {
            SetShdMatrix();
            CurrentLightOn[Renderer._eye] = true;
            CurrentLightRect[Renderer._eye] = CurrentLight.rect;
        } else {
            CurrentLight.set();
        }
        return true;
    }
    /******************************************************************************/
    static DIR_ENUM ShdDirStepDir[6] =
        {
            DIR_FORWARD,
            DIR_BACK,
            DIR_RIGHT,
            DIR_LEFT,
            DIR_UP,
            DIR_DOWN,
        };
    static Byte ConnectedPoints[8][3] = // contains the indexes of 3 other points that this point is connected to
        {
            // points connected to point #: (the first value is from the other side, the 2 next values are +1 and -1 modulo)
            {4, 1, 3}, // 0, example: point 0 is connected to point 4 from the other side and mod(0+1)=1 and mod(0-1)=3
            {5, 2, 0}, // 1
            {6, 3, 1}, // 2
            {7, 0, 2}, // 3
            {0, 5, 7}, // 4
            {1, 6, 4}, // 5
            {2, 7, 5}, // 6
            {3, 4, 6}, // 7
        };
    static Flt ShadowFrac(C VecD & start, C VecD & end) {
        return (start.y <= Renderer.lowest_visible_point) ? 0 // if start is below lowest visible point, then we don't need any shadows
               : (start.y <= end.y)                       ? 1 // if start is below end (we're looking up, and not down, then we need full shadows)
                                                          : PointOnPlaneStep(start.y - Renderer.lowest_visible_point, end.y - Renderer.lowest_visible_point);
    }
    static Bool ShadowMap(LightDir & light) {
        // init
        RENDER_MODE mode = Renderer();
        Renderer.mode(RM_SHADOW);
        D.alpha(ALPHA_NONE); // disable alpha

        Renderer._shd_map.discard();
        Renderer.set(null, &Renderer._shd_map, false);
        D.clearDepth(); // clear all shadow map parts at once, because for directional lights we'll use them all anyway

        Bool cloud_shd = false,
             cloud_vol = false;
        UInt flag = 0;
        if (Clouds.draw && Clouds.volumetric.drawable() && Renderer._cld_map.is()) {
            if (Clouds.volumetric.shadow > EPS_COL8)
                cloud_shd = true;
            if (Renderer.hasVolLight() && CurrentLight.vol() > EPS_COL8_NATIVE)
                cloud_vol = true;
            if (cloud_shd || cloud_vol) {
                flag |= SM_CLOUDS;
                Renderer._cld_map.discard();
            }
        }

        // set light direction orientation
        Matrix3 orn;
        orn.setDir(light.dir); // set any orientation initially

        // set steps
        Bool point_clip = (Renderer.lowest_visible_point > -DBL_MAX),
             clip = D._clip;
        Rect clip_rect = D._clip_rect;
        Int steps = D.shadowMapNumActual();
        Int points = 8, points_1 = points - 1;
        VecD point[8], point_temp[8 * 3], *point_ptr = (point_clip ? point_temp : point);
        Flt bias = D._shd_bias,
            range = D._shd_range * SHADOW_MAP_DIR_RANGE_MUL, // view range of the shadow map render target (this needs to be increased in case there are shadow occluders located distant from the camera), TODO: make SHADOW_MAP_DIR_RANGE_MUL configurable?
            shd_from = D.viewFromActual(),                   // where does the shadow start in the main render target
            shd_to = Max(D._shd_range, shd_from + 0.01f),    // where does the shadow end   in the main render target (must be slightly larger than starting point)
            shd_from_frac = shd_from / D.viewRange(),
            shd_to_frac = shd_to / D.viewRange();

        if (point_clip && !D.shadowReduceFlicker()) {
            Flt frac = shd_from_frac + 0.01f / D.viewRange(); // must be slightly larger than starting point
            if (FrustumMain.points == 5)                      // perspective (pyramid frustum)
            {
                MAX(frac, ShadowFrac(FrustumMain.point[0], FrustumMain.point[1]));
                MAX(frac, ShadowFrac(FrustumMain.point[0], FrustumMain.point[2]));
                MAX(frac, ShadowFrac(FrustumMain.point[0], FrustumMain.point[3]));
                MAX(frac, ShadowFrac(FrustumMain.point[0], FrustumMain.point[4]));
            } else // orthogonal (box frustum)
            {
                MAX(frac, ShadowFrac(FrustumMain.point[0], FrustumMain.point[4]));
                MAX(frac, ShadowFrac(FrustumMain.point[1], FrustumMain.point[5]));
                MAX(frac, ShadowFrac(FrustumMain.point[2], FrustumMain.point[6]));
                MAX(frac, ShadowFrac(FrustumMain.point[3], FrustumMain.point[7]));
            }
            MIN(shd_to_frac, frac);
        }

        Flt s0 = Lerp(shd_from_frac, shd_to_frac, ShadowStep(0, steps));

        if (FrustumMain.points == 5) // perspective (pyramid frustum)
        {
            point[0] = Lerp(FrustumMain.point[0], FrustumMain.point[1], s0);
            point[1] = Lerp(FrustumMain.point[0], FrustumMain.point[2], s0);
            point[2] = Lerp(FrustumMain.point[0], FrustumMain.point[3], s0);
            point[3] = Lerp(FrustumMain.point[0], FrustumMain.point[4], s0);
        } else // orthogonal (box frustum)
        {
            point[0] = Lerp(FrustumMain.point[0], FrustumMain.point[4], s0);
            point[1] = Lerp(FrustumMain.point[1], FrustumMain.point[5], s0);
            point[2] = Lerp(FrustumMain.point[2], FrustumMain.point[6], s0);
            point[3] = Lerp(FrustumMain.point[3], FrustumMain.point[7], s0);
        }
        FREP(steps) // go from start because 's0' and 'point' are already set for first step, and we calculate 's1' and 'point' for next steps in the loop
        {
            Flt                                                                  // s0=Lerp(shd_from_frac, shd_to_frac, ShadowStep(i  , steps)); // convert to shd_from_frac .. shd_to_frac, already calculated
                s1 = Lerp(shd_from_frac, shd_to_frac, ShadowStep(i + 1, steps)), // convert to shd_from_frac .. shd_to_frac
                view_main_max = s1 * D.viewRange();
            Sh.ShdStep[i]->set(view_main_max); // this value specifies at what depth what shadow map index should be used

            if (FrustumMain.points == 5) // perspective (pyramid frustum)
            {
                point[4] = Lerp(FrustumMain.point[0], FrustumMain.point[1], s1);
                point[5] = Lerp(FrustumMain.point[0], FrustumMain.point[2], s1);
                point[6] = Lerp(FrustumMain.point[0], FrustumMain.point[3], s1);
                point[7] = Lerp(FrustumMain.point[0], FrustumMain.point[4], s1);
            } else // orthogonal (box frustum)
            {
                point[4] = Lerp(FrustumMain.point[0], FrustumMain.point[4], s1);
                point[5] = Lerp(FrustumMain.point[1], FrustumMain.point[5], s1);
                point[6] = Lerp(FrustumMain.point[2], FrustumMain.point[6], s1);
                point[7] = Lerp(FrustumMain.point[3], FrustumMain.point[7], s1);
            }

            MatrixM light_matrix;
            light_matrix.orn() = orn;

            if (point_clip) // if we want to clip the point edges
            {
                points = 0;
                REPA(point) // iterate all original points
                {
                    C VecD &p = point[i];
                    if (p.y >= Renderer.lowest_visible_point)
                        point_temp[points++] = p;
                    else                                             // if the point is above visible area, then include it as it is
                    {                                                // if the point is below
                        Dbl d = p.y - Renderer.lowest_visible_point; // calculate Y distance to the plane
                        REPD(j, 3)                                   // iterate all 3 connected points to this one
                        {
                            C VecD &p1 = point[ConnectedPoints[i][j]]; // get j-th connected point to point 'i'
                            if (p1.y > Renderer.lowest_visible_point)  // other point must be above visible area (if both are below then we can just ignore this edge), here use > instead of >=, because if 'p1' is == then it's going to be included anyway in the check above, and the calculated point on plane below would be same as 'p1', so we would have the same point listed twice
                            {
                                Dbl d1 = p1.y - Renderer.lowest_visible_point;     // calculate Y distance to the plane
                                point_temp[points++] = PointOnPlane(p, p1, d, d1); // add the point that is intersection of the edge and the plane
                            }
                        }
                    }
                }
                if (points < 3) // min 3 points required to construct a volume, otherwise we would have zero dimension frustum or even "points_1==-1"
                {
                    light_matrix.pos.zero();
                    REPA(point)
                    light_matrix.pos += point[i];
                    light_matrix.pos /= Elms(point);                                                // set position in the center of all points
                    D._view_active.setFrom(0).setRange(range).setFov(1, FOV_ORTHO).setProjMatrix(); // set 'ProjMatrix' projection matrix needed below to calculate correct transformation matrix, needed in case there will be some pixels outside of the 'Renderer.lowest_visible_point', changing 'Frustum' not needed because it's not needed here and will be set/reset later
                    goto skip;
                }
                points_1 = points - 1;
            }

            {
                // convert frustum points onto 2D light orientation space
                VecD2 frustum_points_2D[Elms(point_temp)];
                REP(points) {
                    C VecD &p = point_ptr[i];
                    frustum_points_2D[i].set(Dot(p, light_matrix.x), Dot(p, light_matrix.y));
                }
                MatrixFovFrac frustum;
                VecD2 axis;
                if (BestFit(frustum_points_2D, points, axis)) // find best orientation alignment
                {
                    frustum.matrix.y = axis.x * light_matrix.x + axis.y * light_matrix.y;
                    frustum.matrix.x = Cross(frustum.matrix.y, light_matrix.z);
                } else {
                    frustum.matrix.x = light_matrix.x;
                    frustum.matrix.y = light_matrix.y;
                }
                if (!D.shadowReduceFlicker()) // we can use best fit matrix for light matrix only if 'shadowReduceFlicker' is disabled, because 'shadowReduceFlicker' works in a way that it always uses biggest worst possible alignment, so doing this would actually work against 'shadowReduceFlicker'
                {
                    light_matrix.x = frustum.matrix.x;
                    light_matrix.y = frustum.matrix.y;
                }

                // set sizes
                Dbl x[2], y[2], z[2];
                {
                    // setup values from last point
                    x[0] = x[1] = DistPointPlane(point_ptr[points_1], light_matrix.x);
                    y[0] = y[1] = DistPointPlane(point_ptr[points_1], light_matrix.y);
                    z[0] = z[1] = DistPointPlane(point_ptr[points_1], light_matrix.z);
                    REP(points_1) // now iterate all remaining points and adjust the values
                    {
                        Dbl d = DistPointPlane(point_ptr[i], light_matrix.x);
                        if (d < x[0])
                            x[0] = d;
                        else if (d > x[1])
                            x[1] = d;
                        d = DistPointPlane(point_ptr[i], light_matrix.y);
                        if (d < y[0])
                            y[0] = d;
                        else if (d > y[1])
                            y[1] = d;
                        d = DistPointPlane(point_ptr[i], light_matrix.z);
                        if (d < z[0])
                            z[0] = d;
                        else if (d > z[1])
                            z[1] = d;
                    }
                }

                // align sizes
                if (D.shadowReduceFlicker()) {
                    Dbl max_dist = Sqrt(Max(Dist2(point[0], point[6]), Dist2(point[4], point[6]))); // Max(diagonal back<->front, diagonal front<->front), here use 'point' instead of 'point_ptr' (we need original points)
                    VecD2 center(Avg(x[0], x[1]), Avg(y[0], y[1]));
                    {
                        Dbl a = max_dist / D.shadowMapSizeActual();
                        Dbl o = AlignTrunc(center.x, 1024.0);
                        center.x -= o;
                        center.x = AlignFloor(center.x, a);
                        center.x += o; // offsetting by 1024 helps with precision issues at very big distances (512 km), we use Trunc in this case, because we want to move towards zero
                        o = AlignTrunc(center.y, 1024.0);
                        center.y -= o;
                        center.y = AlignFloor(center.y, a);
                        center.y += o;
                    }
                    Dbl max_dist_2 = max_dist / 2;
                    Dbl min = center.x - max_dist_2;
                    frustum.frac.setX((x[0] - min) / max_dist, (x[1] - min) / max_dist);
                    x[0] = min;
                    x[1] = center.x + max_dist_2;
                    min = center.y - max_dist_2;
                    frustum.frac.setY((y[0] - min) / max_dist, (y[1] - min) / max_dist);
                    y[0] = min;
                    y[1] = center.y + max_dist_2;
                } else {
                    Dbl a = (x[1] - x[0]) / D.shadowMapSizeActual();
                    x[0] = AlignFloor(x[0], a);
                    x[1] = AlignFloor(x[1], a);
                    a = (y[1] - y[0]) / D.shadowMapSizeActual();
                    y[0] = AlignFloor(y[0], a);
                    y[1] = AlignFloor(y[1], a);
                }

                // calculate fov and position
                Vec2 fov((x[1] - x[0]) * 0.5f, (y[1] - y[0]) * 0.5f); // no need for 'VecD2'
                light_matrix.pos = Avg(x[0], x[1]) * light_matrix.x + Avg(y[0], y[1]) * light_matrix.y + (z[1] - range) * light_matrix.z;

                // !! Warning: this modifies 'x' and 'y' to frustum fov !!
                if (D.shadowReduceFlicker()) // we need to set it only if we're going to use it
                {
                    x[0] = x[1] = DistPointPlane(point_ptr[points_1], frustum.matrix.x);
                    y[0] = y[1] = DistPointPlane(point_ptr[points_1], frustum.matrix.y);
                    REP(points_1) // now iterate all remaining points and adjust the values
                    {
                        Dbl d = DistPointPlane(point_ptr[i], frustum.matrix.x);
                        if (d < x[0])
                            x[0] = d;
                        else if (d > x[1])
                            x[1] = d;
                        d = DistPointPlane(point_ptr[i], frustum.matrix.y);
                        if (d < y[0])
                            y[0] = d;
                        else if (d > y[1])
                            y[1] = d;
                    }
                    frustum.fov.set((x[1] - x[0]) * 0.5f, (y[1] - y[0]) * 0.5f);
                    frustum.matrix.z = light_matrix.z;
                    frustum.matrix.pos = Avg(x[0], x[1]) * frustum.matrix.x + Avg(y[0], y[1]) * frustum.matrix.y + (z[1] - range) * frustum.matrix.z;
                }

                // draw shadow map
                Flt step_bias = fov.max() * bias + DepthError(0, range, range, false, Renderer._shd_map.hwTypeInfo().d);
                DrawShadowMap(ShdDirStepDir[i], light_matrix, flag, 0, range, fov, FOV_ORTHO, 0, step_bias, D.shadowReduceFlicker() ? &frustum : null); // set 'frustum' only if we need it, because if 'shadowReduceFlicker' is disabled, then we've already aligned the matrix and we don't need to do anything more
            }
        skip:

            // matrix, transform from camera space to light space, ShdMatrix=CamMatrix/LightMatrix
            // this is used for positions in camera view space, so multiply first by CamMatrix to convert from view space to world space, and then div by LightMatrix to convert to light space
            // potentially we could do 'light_matrix.inverse(true);" and then CamMatrix*light_matrix, however 'divNormalized' is just as fast
            if (Renderer._stereo) {
                EyeMatrix[0].divNormalized(light_matrix, ShdMatrix[0]);
                EyeMatrix[1].divNormalized(light_matrix, ShdMatrix[1]);
            } else
                ActiveCam.matrix.divNormalized(light_matrix, ShdMatrix[0]);

#if FLAT_SHADOW_MAP
            {
                Matrix4 mp;
                ProjMatrix.mul(HsmMatrixSet(ShdDirStepDir[i]), mp); // 'ProjMatrix' here is from shadow view
                REPS(Renderer._eye, Renderer._eye_num)
                ShdMatrix[Renderer._eye].mul(mp, ShdMatrix4[i][Renderer._eye]);
            }
#else // cube shadow maps
        REPS(Renderer._eye, Renderer._eye_num) {
            // scale down from -fov..fov to -1..1 range
            Matrix m = ShdMatrix[Renderer._eye];
            MatrixFovScaleX(m, D._view_active.fov.x);
            MatrixFovScaleY(m, D._view_active.fov.y);

            Matrix4 &d = ShdMatrix4[i][Renderer._eye];
            d = m;

            // copy 'z' to 'w' which will be used as final 'z' comparison value
            d.x.w = d.x.z;
            d.y.w = d.y.z;
            d.z.w = d.z.z;
            d.pos.w = d.pos.z;

            // apply bias
            d.pos.w -= step_bias;

            // adjust orientation for cube texture
            switch (ShdDirStepDir[i]) {
            case DIR_FORWARD:
                d.x.z = 0;
                d.y.z = 0;
                d.z.z = 0;
                d.pos.z = 1;
                break;
            case DIR_BACK:
                CHS(d.x.x);
                CHS(d.y.x);
                CHS(d.z.x);
                CHS(d.pos.x);
                d.x.z = 0;
                d.y.z = 0;
                d.z.z = 0;
                d.pos.z = -1;
                break;
            case DIR_RIGHT:
                d.x.z = -d.x.x;
                d.y.z = -d.y.x;
                d.z.z = -d.z.x;
                d.pos.z = -d.pos.x;
                d.x.x = 0;
                d.y.x = 0;
                d.z.x = 0;
                d.pos.x = 1;
                break;
            case DIR_LEFT:
                d.x.z = d.x.x;
                d.y.z = d.y.x;
                d.z.z = d.z.x;
                d.pos.z = d.pos.x;
                d.x.x = 0;
                d.y.x = 0;
                d.z.x = 0;
                d.pos.x = -1;
                break;
            case DIR_UP:
                d.x.z = -d.x.y;
                d.y.z = -d.y.y;
                d.z.z = -d.z.y;
                d.pos.z = -d.pos.y;
                d.x.y = 0;
                d.y.y = 0;
                d.z.y = 0;
                d.pos.y = 1;
                break;
            case DIR_DOWN:
                d.x.z = d.x.y;
                d.y.z = d.y.y;
                d.z.z = d.z.y;
                d.pos.z = d.pos.y;
                d.x.y = 0;
                d.y.y = 0;
                d.z.y = 0;
                d.pos.y = -1;
                break;
            }
        }
#endif

            s0 = s1;
            CopyFastN(&point[0], &point[4], 4);
        }

        // restore settings
        Renderer.mode(mode);
        RestoreShadowMapSettings();
        D.clip(clip ? &clip_rect : null); // no need to reset 'D.clipAllow' because it will be reset in the nearest RT change

        return cloud_shd;
    }
    /******************************************************************************/
    static void ShadowMap(Flt range, VecD & pos) {
        RENDER_MODE mode = Renderer();
        Renderer.mode(RM_SHADOW);
        D.alpha(ALPHA_NONE); // disable alpha
        Renderer._shd_map.discard();
        Renderer.set(null, &Renderer._shd_map, false);
        D.clearDepth(); // clear all at once, must be done here because DX10+ and GL depth clear always clears full depth buffer

        Int border = 0;
#if FLAT_SHADOW_MAP
        {
            Flt detail = Sat(CurrentLight.rect.size().max() / Min(D.w2(), D.h2())) * D.shadowMapSizeLocal();
            Int half = D.shadowMapSizeActual() / 2;
            border = Mid(half - RoundPos(detail * half), 0, half - 1);
        }
#endif
        Int map_size = D.shadowMapSizeActual() - border * 2;

        // render to shadow maps
        {
            UInt flag = (FrustumMain(pos) ? 0 : SM_FRUSTUM); // if the light source is inside the main frustum, then we don't need to test all light frustums because they all will be required
#if FLAT_SHADOW_MAP
            Vec2 fov = 2 * Atan(1 + 2.0f / map_size); // padd 2 pixels of shadow map, needed for correct filtering at borders of each of 6 shadow maps, this doesn't require adjusting any other parameters, because it affects 'ProjMatrix' which is used later for matrix calculation
#else
        Vec2 fov = PI_2;
#endif
            // Flt view_main_max=DistPointActiveCamPlaneZ(pos)+range;
            DrawShadowMap(DIR_FORWARD, MatrixM().setPos(pos), flag, LOCAL_SHADOW_MAP_FROM, range, fov, FOV_XY, border, 0);
            DrawShadowMap(DIR_BACK, MatrixM().setPosDir(pos, VecDir[DIR_BACK], VecDir[DIR_UP]), flag, LOCAL_SHADOW_MAP_FROM, range, fov, FOV_XY, border, 0);
            DrawShadowMap(DIR_LEFT, MatrixM().setPosDir(pos, VecDir[DIR_LEFT], VecDir[DIR_UP]), flag, LOCAL_SHADOW_MAP_FROM, range, fov, FOV_XY, border, 0);
            DrawShadowMap(DIR_RIGHT, MatrixM().setPosDir(pos, VecDir[DIR_RIGHT], VecDir[DIR_UP]), flag, LOCAL_SHADOW_MAP_FROM, range, fov, FOV_XY, border, 0);
            DrawShadowMap(DIR_DOWN, MatrixM().setPosDir(pos, VecDir[DIR_DOWN], VecDir[DIR_FORWARD]), flag, LOCAL_SHADOW_MAP_FROM, range, fov, FOV_XY, border, 0);
            DrawShadowMap(DIR_UP, MatrixM().setPosDir(pos, VecDir[DIR_UP], VecDir[DIR_BACK]), flag, LOCAL_SHADOW_MAP_FROM, range, fov, FOV_XY, border, 0);
        }

        // matrix, transform from camera space to light space, ShdMatrix=CamMatrix/LightMatrix, ShdMatrix=CamMatrix-pos
        if (Renderer._stereo) {
            ShdMatrix[0].orn() = EyeMatrix[0].orn();
            ShdMatrix[0].pos = EyeMatrix[0].pos - pos;
            ShdMatrix[1].orn() = EyeMatrix[1].orn();
            ShdMatrix[1].pos = EyeMatrix[1].pos - pos;
        } else {
            ShdMatrix[0].orn() = ActiveCam.matrix.orn();
            ShdMatrix[0].pos = ActiveCam.matrix.pos - pos;
        }

#if FLAT_SHADOW_MAP
        {
            Flt f = D.shadowMapSizeActual() / Flt(map_size), bias = 1 - D._shd_bias; // for local lights instead of offsetting camera by bias, we scale it, because the shadow map is perspective so it needs to depend on distance to light
            Matrix m;
            Matrix4 mp;
            // 'ProjMatrix' here is from shadow view
            ProjMatrix.mul(HsmMatrixSet(DIR_RIGHT), mp);
            REPS(Renderer._eye, Renderer._eye_num) {
                Matrix &s = ShdMatrix[Renderer._eye];
                m = s;
                m.x.x = -s.x.z;
                m.y.x = -s.y.z;
                m.z.x = -s.z.z;
                m.pos.x = -s.pos.z;
                m.x.z = s.x.x;
                m.y.z = s.y.x;
                m.z.z = s.z.x;
                m.pos.z = s.pos.x;
                MatrixFovScale(m, f);
                m.scale(bias);
                m.mul(mp, ShdMatrix4[0][Renderer._eye]);
            } // right
            ProjMatrix.mul(HsmMatrixSet(DIR_LEFT), mp);
            REPS(Renderer._eye, Renderer._eye_num) {
                Matrix &s = ShdMatrix[Renderer._eye];
                m = s;
                m.x.x = s.x.z;
                m.y.x = s.y.z;
                m.z.x = s.z.z;
                m.pos.x = s.pos.z;
                m.x.z = -s.x.x;
                m.y.z = -s.y.x;
                m.z.z = -s.z.x;
                m.pos.z = -s.pos.x;
                MatrixFovScale(m, f);
                m.scale(bias);
                m.mul(mp, ShdMatrix4[1][Renderer._eye]);
            } // left
            ProjMatrix.mul(HsmMatrixSet(DIR_UP), mp);
            REPS(Renderer._eye, Renderer._eye_num) {
                Matrix &s = ShdMatrix[Renderer._eye];
                m = s;
                m.x.z = s.x.y;
                m.y.z = s.y.y;
                m.z.z = s.z.y;
                m.pos.z = s.pos.y;
                m.x.y = -s.x.z;
                m.y.y = -s.y.z;
                m.z.y = -s.z.z;
                m.pos.y = -s.pos.z;
                MatrixFovScale(m, f);
                m.scale(bias);
                m.mul(mp, ShdMatrix4[2][Renderer._eye]);
            } // up
            ProjMatrix.mul(HsmMatrixSet(DIR_DOWN), mp);
            REPS(Renderer._eye, Renderer._eye_num) {
                Matrix &s = ShdMatrix[Renderer._eye];
                m = s;
                m.x.z = -s.x.y;
                m.y.z = -s.y.y;
                m.z.z = -s.z.y;
                m.pos.z = -s.pos.y;
                m.x.y = s.x.z;
                m.y.y = s.y.z;
                m.z.y = s.z.z;
                m.pos.y = s.pos.z;
                MatrixFovScale(m, f);
                m.scale(bias);
                m.mul(mp, ShdMatrix4[3][Renderer._eye]);
            } // down
            ProjMatrix.mul(HsmMatrixSet(DIR_BACK), mp);
            REPS(Renderer._eye, Renderer._eye_num) {
                Matrix &s = ShdMatrix[Renderer._eye];
                m = s;
                CHS(m.x.x);
                CHS(m.y.x);
                CHS(m.z.x);
                CHS(m.pos.x);
                CHS(m.x.z);
                CHS(m.y.z);
                CHS(m.z.z);
                CHS(m.pos.z);
                MatrixFovScale(m, f);
                m.scale(bias);
                m.mul(mp, ShdMatrix4[5][Renderer._eye]);
            } // back
            ProjMatrix.mul(HsmMatrixSet(DIR_FORWARD), mp);
            REPS(Renderer._eye, Renderer._eye_num) {
                Matrix &s = ShdMatrix[Renderer._eye];
                m = s;
                MatrixFovScale(m, f);
                m.scale(bias);
                m.mul(mp, ShdMatrix4[4][Renderer._eye]);
            } // forward
        }
#endif

        // restore settings
        Renderer.mode(mode); // restore rendering mode to correctly restore viewport
        RestoreShadowMapSettings();
    }
    /******************************************************************************/
    static void ShadowMap(LightCone & light) {
        RENDER_MODE mode = Renderer();
        Renderer.mode(RM_SHADOW);
        D.alpha(ALPHA_NONE); // disable alpha
        Renderer._shd_map.discard();
        Renderer.set(null, &Renderer._shd_map, false);
        D.clearDepth(); // clear all at once, must be done here because DX10+ and GL depth clear always clears full depth buffer

        Int map_size = D.shadowMapSizeActual(), border = 0;
#if FLAT_SHADOW_MAP
        {
            map_size *= 2; // for FLAT_SHADOW_MAP Cone we can use 2x map size because we have 2x3 sized full shadow map
            Flt detail = Sat(CurrentLight.rect.size().max() / Min(D.w2(), D.h2())) * D.shadowMapSizeLocal() * (D.shadowMapSizeCone() * 0.5f);
            Int half = map_size / 2;
            border = Mid(half - RoundPos(detail * half), 0, half - 1);
            map_size -= border * 2;
        }
#endif

        // shadow map
        MatrixM light_matrix = light.pyramid;
        // Flt view_main_max=DistPointActiveCamPlaneZ(light_matrix.pos)+light.pyramid.h; // this is not precise but worst case scenario
        DrawShadowMap(DIR_FORWARD, light_matrix, 0, LOCAL_SHADOW_MAP_FROM, light.pyramid.h, Vec2(2 * Atan(light.pyramid.scale)), FOV_XY, border, 0); // use DIR_FORWARD because cube map shadows require that

        // matrix, transform from camera space to light space, ShdMatrix=CamMatrix/LightMatrix
        // potentially we could do "light_matrix.inverse(true);" and then CamMatrix*LightMatrix, however 'divNormalized' is just as fast
        if (Renderer._stereo) {
            EyeMatrix[0].divNormalized(light_matrix, ShdMatrix[0]);
            EyeMatrix[1].divNormalized(light_matrix, ShdMatrix[1]);
        } else
            ActiveCam.matrix.divNormalized(light_matrix, ShdMatrix[0]);

#if FLAT_SHADOW_MAP
        {
            Flt f = D.shadowMapSizeActual() / Flt(map_size), bias = 1 - D._shd_bias; // for local lights instead of offsetting camera by bias, we scale it, because the shadow map is perspective so it needs to depend on distance to light
            Matrix4 mp;
            ProjMatrix.mul(HsmMatrixCone, mp); // 'ProjMatrix' here is from shadow view
            REPS(Renderer._eye, Renderer._eye_num) {
                Matrix m = ShdMatrix[Renderer._eye];
                MatrixFovScale(m, f);
                m.scale(bias);
                m.mul(mp, ShdMatrix4[0][Renderer._eye]);
            }
        }
#endif
        REPS(Renderer._eye, Renderer._eye_num)
        MatrixFovScale(ShdMatrix[Renderer._eye], light.pyramid.scale);

        // restore settings
        Renderer.mode(mode); // restore rendering mode to correctly restore viewport
        RestoreShadowMapSettings();
    }
    /******************************************************************************/
    Flt Light::range() C {
        switch (type) {
        case LIGHT_POINT:
            return point.range();
        case LIGHT_LINEAR:
            return linear.range;
        case LIGHT_CONE:
            return cone.pyramid.h;
        default:
            return 0;
        }
    }
    Flt Light::vol() C {
        switch (type) {
        case LIGHT_DIR:
            return dir.vol;
        case LIGHT_POINT:
            return point.vol;
        case LIGHT_LINEAR:
            return linear.vol;
        case LIGHT_CONE:
            return cone.vol;
        default:
            return 0;
        }
    }
    VecD Light::pos() C {
        switch (type) {
        case LIGHT_POINT:
            return point.pos;
        case LIGHT_LINEAR:
            return linear.pos;
        case LIGHT_CONE:
            return cone.pyramid.pos;
        default:
            return 0;
        }
    }
    Flt Light::firstLightCost(Flt view_rect_area, Dbl frustum_volume) C {
        /* Normally we draw lights clipped to their screen rect and its frustum volume
           however for the first light we have to draw entire view rect and entire frustum
           which means that some pixels     (based on 2D area  ) are processed unnecessarily (they're wasted)
                        and some draw calls (based on 3D volume) are processed unnecessarily (they're wasted)

           Shader cost was measured by drawing 1000 fullscreen images "ALPHA_MODE alpha=D.alpha(ALPHA_NONE); REP(1000)box.parts[0].material()->base_0->draw(D.rect());"
              got FPS=8.7
           Then drawing 1000 fullscreen 3D box with the same material texture, normal mapping, and lights (tested for all light types with shadows on/off and "D.depthFunc(FUNC_LESS_EQUAL);" added before drawing lights in RM_PREPARE
              light shader cost = 2D-FPS / 3D-FPS = 8.7-FPS / 3D-FPS
           The results are divided using 8.7 fps to simulate cost relative to tex read + RT write (approximate cost for flushing/restoring RT on TILE_BASED_GPU)
           Measured on GeForce 1050 Ti
        */
        Flt shader_cost, waste_3d, light_vol;
        switch (type) {
        case LIGHT_DIR:
            shader_cost = (shadow ? 8.7f / 5.10f : 8.7f / 7.70f);
            waste_3d = 0;
            break;
        case LIGHT_POINT:
            shader_cost = (shadow ? 8.7f / 4.25f : 8.7f / 6.30f);
            light_vol = (PI * 4 / 3) * Cube(point.range());
            waste_3d = Max(0, frustum_volume - light_vol) / frustum_volume;
            break;
        case LIGHT_LINEAR:
            shader_cost = (shadow ? 8.7f / 4.30f : 8.7f / 6.40f);
            light_vol = (PI * 4 / 3) * Cube(linear.range);
            waste_3d = Max(0, frustum_volume - light_vol) / frustum_volume;
            break;
        case LIGHT_CONE:
            shader_cost = (shadow ? 8.7f / 4.35f : 8.7f / 5.60f);
            light_vol = cone.pyramid.volume();
            waste_3d = Max(0, frustum_volume - light_vol) / frustum_volume;
            break;
        default:
            return FLT_MAX;
        }
        Flt wasted_area = view_rect_area - (rect & D.viewRect()).area(), // wasted area = total area - needed area
            waste_2d = shader_cost * wasted_area;
#if TILE_BASED_GPU
        if (shadow)
            waste_2d -= view_rect_area * 2; // (*2 because there are 2 transfers) for tile-based GPU's prefer choosing lights with shadows first, to avoid the cost of transferring RT to AND from chip fast memory (example: drawing #1 non-shadow light, #2 shadow light = draw light #1, flush RT, draw shadows for #2, restore RT, draw light #2), (example: drawing #1 shadow light, #2 non-shadow light = draw shadows for #1, draw light #1, draw light #2)
#endif
        return waste_2d / view_rect_area * 16 + waste_3d; // since it's difficult to estimate 2D pixel vs 3D volume cost, they're just normalized to approximate 0..1 ranges and added together, with 2D having bigger weight
    }
    Bool Light::toScreenRect(Rect & rect) C {
        switch (type) {
        case LIGHT_DIR:
            return dir.toScreenRect(rect);
        case LIGHT_POINT:
            return point.toScreenRect(rect);
        case LIGHT_LINEAR:
            return linear.toScreenRect(rect);
        case LIGHT_CONE:
            return cone.toScreenRect(rect);
        default:
            return false;
        }
    }
    void Light::fade(Flt fade) {
        switch (type) { // color_l=SRGBToLinear(LinearToSRGB(color_l)*scale)
        case LIGHT_DIR:
            dir.color_l *= Sqr(fade);
            break;
        case LIGHT_POINT:
            point.color_l *= Sqr(fade);
            break;
        case LIGHT_LINEAR:
            linear.color_l *= Sqr(fade);
            break;
        case LIGHT_CONE:
            cone.color_l *= Sqr(fade);
            break;
        }
    }
    /******************************************************************************
    void Light::scalePower(Flt scale)
    {
       switch(type)
       {
          case LIGHT_DIR   : dir   .color_l  *=Sqr(scale); break;
          case LIGHT_POINT : point .power    *=Sqr(scale); break;
          case LIGHT_LINEAR: linear.range    *=    scale ; break;
          case LIGHT_CONE  : cone  .pyramid.h*=    scale ; break;
       }
    }
    /******************************************************************************/
    void Light::set() {
        switch (type) {
        case LIGHT_DIR:
            dir.set();
            break;
        case LIGHT_POINT:
            point.set(shadow_opacity);
            break;
        case LIGHT_LINEAR:
            linear.set(shadow_opacity);
            break;
        case LIGHT_CONE:
            cone.set(shadow_opacity);
            break;
        }
    }
    /******************************************************************************/
    static Bool CanDoShadow() {
        return D.shadowMode() && D.shadowSupported() && FovPerspective(D.viewFovMode());
    }
    void Light::set(LightDir & light, Bool shadow, CPtr light_src, Bool allow_main) {
        Zero(T);
        type = LIGHT_DIR;
        dir = light;
        T.rect = D.viewRect();
        T.shadow = (shadow && CanDoShadow());
        T.shadow_opacity = (T.shadow);
        T.src = light_src;
        T.allow_main = allow_main;
    }
    void Light::set(LightPoint & light, C Rect & rect, Flt shadow_opacity, CPtr light_src) {
        Zero(T);
        type = LIGHT_POINT;
        point = light;
        T.rect = rect;
        T.shadow = (shadow_opacity > EPS_COL8 && CanDoShadow());
        T.shadow_opacity = (T.shadow ? Sat(shadow_opacity) : 0);
        T.src = light_src;
    }
    void Light::set(LightLinear & light, C Rect & rect, Flt shadow_opacity, CPtr light_src) {
        Zero(T);
        type = LIGHT_LINEAR;
        linear = light;
        T.rect = rect;
        T.shadow = (shadow_opacity > EPS_COL8 && CanDoShadow());
        T.shadow_opacity = (T.shadow ? Sat(shadow_opacity) : 0);
        T.src = light_src;
    }
    void Light::set(LightCone & light, C Rect & rect, Flt shadow_opacity, CPtr light_src) {
        Zero(T);
        type = LIGHT_CONE;
        cone = light;
        T.rect = rect;
        T.shadow = (shadow_opacity > EPS_COL8 && CanDoShadow());
        T.shadow_opacity = (T.shadow ? Sat(shadow_opacity) : 0);
        T.src = light_src;
    }
    /******************************************************************************/
    INLINE Shader *GetShdDir(Int map_num, Bool clouds, Bool multi_sample) {
        Shader *&s = Sh.ShdDir[map_num - 1][clouds][multi_sample];
        if (SLOW_SHADER_LOAD && !s)
            s = Sh.getShdDir(map_num, clouds, multi_sample);
        return s;
    }
    INLINE Shader *GetShdPoint(Bool multi_sample) {
        Shader *&s = Sh.ShdPoint[multi_sample];
        if (SLOW_SHADER_LOAD && !s)
            s = Sh.getShdPoint(multi_sample);
        return s;
    }
    INLINE Shader *GetShdCone(Bool multi_sample) {
        Shader *&s = Sh.ShdCone[multi_sample];
        if (SLOW_SHADER_LOAD && !s)
            s = Sh.getShdCone(multi_sample);
        return s;
    }
    /******************************************************************************/
    INLINE Shader *GetDrawLightDir(Int multi_sample, Int light_mode, Bool shadow) {
        DIFFUSE_MODE diffuse = D.diffuseMode();
        Shader *&s = Sh.DrawLightDir[diffuse][multi_sample][light_mode][shadow];
        if (SLOW_SHADER_LOAD && !s)
            s = Sh.getDrawLightDir(diffuse, multi_sample, light_mode, shadow);
        return s;
    }
    INLINE Shader *GetDrawLightPoint(Int multi_sample, Int light_mode, Bool shadow) {
        DIFFUSE_MODE diffuse = D.diffuseMode();
        Shader *&s = Sh.DrawLightPoint[diffuse][multi_sample][light_mode][shadow];
        if (SLOW_SHADER_LOAD && !s)
            s = Sh.getDrawLightPoint(diffuse, multi_sample, light_mode, shadow);
        return s;
    }
    INLINE Shader *GetDrawLightLinear(Int multi_sample, Int light_mode, Bool shadow) {
        DIFFUSE_MODE diffuse = D.diffuseMode();
        Shader *&s = Sh.DrawLightLinear[diffuse][multi_sample][light_mode][shadow];
        if (SLOW_SHADER_LOAD && !s)
            s = Sh.getDrawLightLinear(diffuse, multi_sample, light_mode, shadow);
        return s;
    }
    INLINE Shader *GetDrawLightCone(Int multi_sample, Int light_mode, Bool shadow, Bool image) {
        DIFFUSE_MODE diffuse = D.diffuseMode();
        Shader *&s = Sh.DrawLightCone[diffuse][multi_sample][light_mode][shadow][image];
        if (SLOW_SHADER_LOAD && !s)
            s = Sh.getDrawLightCone(diffuse, multi_sample, light_mode, shadow, image);
        return s;
    }
#if !DEPTH_CLIP_SUPPORTED
    INLINE Shader *GetDrawLightConeFlat(Int multi_sample, Int light_mode, Bool shadow, Bool image) {
        DIFFUSE_MODE diffuse = D.diffuseMode();
        Shader *&s = Sh.DrawLightConeFlat[diffuse][multi_sample][light_mode][shadow][image];
        if (SLOW_SHADER_LOAD && !s)
            s = Sh.getDrawLightConeFlat(diffuse, multi_sample, light_mode, shadow, image);
        return s;
    }
#endif
    /******************************************************************************/
    static void DrawLightDir(Int multi_sample, Int light_mode) {
        if (TEST_LIGHT_RECT) {
            D.depthUnlock();
            REPS(Renderer._eye, Renderer._eye_num)
            if (SetLightEye())
                Sh.clear(Vec4(0.3f, 0.3f, 0.3f, 0), &CurrentLight.rect);
            D.depthLock(true);
        }
        // Flat
        Shader *shader = GetDrawLightDir(multi_sample, light_mode, CurrentLight.shadow);
        REPS(Renderer._eye, Renderer._eye_num)
        if (SetLightEye())
            shader->draw(&CurrentLight.rect);
    }
    static void DrawLightPoint(C MatrixM & light_matrix, Int multi_sample, Int light_mode) {
        if (TEST_LIGHT_RECT) {
            D.depthUnlock();
            REPS(Renderer._eye, Renderer._eye_num)
            if (SetLightEye())
                Sh.clear(Vec4(0.3f, 0.3f, 0.3f, 0), &CurrentLight.rect);
            D.depthLock(true);
        }
#if 1 // Geom
        Shader *shader = GetDrawLightPoint(multi_sample, light_mode, CurrentLight.shadow);
        shader->startTex();
        LightMeshBall.set();
        REPS(Renderer._eye, Renderer._eye_num)
        if (SetLightEye()) {
            SetFastMatrix(light_matrix);
            shader->commit();
            LightMeshBall.draw();
        }
#else // Flat
    Shader *shader = GetDrawLightPointFlat(multi_sample, light_mode, CurrentLight.shadow);
    D.depth2DOn();
    REPS(Renderer._eye, Renderer._eye_num)
    if (SetLightEye())
        shader->draw(&CurrentLight.rect);
#endif
    }
    static void DrawLightLinear(C MatrixM & light_matrix, Int multi_sample, Int light_mode) {
        if (TEST_LIGHT_RECT) {
            D.depthUnlock();
            REPS(Renderer._eye, Renderer._eye_num)
            if (SetLightEye())
                Sh.clear(Vec4(0.3f, 0.3f, 0.3f, 0), &CurrentLight.rect);
            D.depthLock(true);
        }
#if 1 // Geom
        Shader *shader = GetDrawLightLinear(multi_sample, light_mode, CurrentLight.shadow);
        shader->startTex();
        LightMeshBall.set();
        REPS(Renderer._eye, Renderer._eye_num)
        if (SetLightEye()) {
            SetFastMatrix(light_matrix);
            shader->commit();
            LightMeshBall.draw();
        }
#else // Flat
    Shader *shader = GetDrawLightLinearFlat(multi_sample, light_mode, CurrentLight.shadow);
    D.depth2DOn();
    REPS(Renderer._eye, Renderer._eye_num)
    if (SetLightEye())
        shader->draw(&CurrentLight.rect);
#endif
    }
    static void DrawLightCone(C MatrixM & light_matrix, Int multi_sample, Int light_mode) {
        if (TEST_LIGHT_RECT) {
            D.depthUnlock();
            REPS(Renderer._eye, Renderer._eye_num)
            if (SetLightEye())
                Sh.clear(Vec4(0.3f, 0.3f, 0.3f, 0), &CurrentLight.rect);
            D.depthLock(true);
        }
#if !DEPTH_CLIP_SUPPORTED                                           // Flat
        if (!D._depth_clip && CurrentLightZRange.y > D.viewRange()) // if light is behind far plane
        {                                                           // draw as Flat
            Shader *shader = GetDrawLightConeFlat(multi_sample, light_mode, CurrentLight.shadow, CurrentLight.image != null);
            REPS(Renderer._eye, Renderer._eye_num)
            if (SetLightEye())
                shader->draw(&CurrentLight.rect);
            return;
        }
#endif
        // Geom
        Shader *shader = GetDrawLightCone(multi_sample, light_mode, CurrentLight.shadow, CurrentLight.image != null);
        shader->startTex();
        LightMeshCone.set();
        REPS(Renderer._eye, Renderer._eye_num)
        if (SetLightEye()) {
            SetFastMatrix(light_matrix);
            shader->commit();
            LightMeshCone.draw();
        }
    }
    /******************************************************************************/
    static Bool LightFrontFaceBall(Flt range, C VecD & light_pos) {
        Bool front_face = Renderer.indoor; // draw as front only for in-door scenes, which will allow to hide the light by occluders, in other cases, we will most likely have to process fewer pixels if not using front
        if (front_face) {
            Flt dist2 = Dist2(ActiveCam.matrix.pos, light_pos); // use 'ActiveCam' instead of 'CamMatrix' because it's not affected by eyes
            if (dist2 <= Sqr(range * LIGHT_MESH_BALL_RADIUS + FrustumMain.view_quad_max_dist + D.eyeDistance_2()))
                front_face = false; // if camera intersects with light mesh, then we can't use front
        }
        return front_face;
    }
    static Bool LightFrontFace(C PyramidM & pyramid) {
        Bool front_face = Renderer.indoor; // draw as front only for in-door scenes, which will allow to hide the light by occluders, in other cases, we will most likely have to process fewer pixels if not using front
        if (front_face) {
            ConeM cone(0, LIGHT_MESH_CONE_RADIUS * pyramid.scale * pyramid.h, pyramid.h, pyramid.pos, pyramid.dir);
            Flt dist = Dist(ActiveCam.matrix.pos, cone); // use 'ActiveCam' instead of 'CamMatrix' because it's not affected by eyes
            if (dist <= FrustumMain.view_quad_max_dist + D.eyeDistance_2())
                front_face = false; // if camera intersects with light mesh, then we can't use front
        }
        return front_face;
    }
    static void SetLightMatrixCone(MatrixM & light_matrix, Bool front_face) {
        Flt s = CurrentLight.cone.pyramid.h * CurrentLight.cone.pyramid.scale;
        light_matrix.orn().setDir(CurrentLight.cone.pyramid.dir);
        light_matrix.x *= (front_face ? s : -s); // reverse faces
        light_matrix.y *= s;
        light_matrix.z *= CurrentLight.cone.pyramid.h;
        light_matrix.pos = CurrentLight.cone.pyramid.pos;
    }
    static void SetLightZRangeCone() {
        VecD p[5];
        CurrentLight.cone.pyramid.toCorners(p);
        Dbl max = Dot(p[4], ActiveCam.matrix.z), min = max;
        REP(Elms(p) - 1) {
            Dbl d = Dot(p[i], ActiveCam.matrix.z);
            if (d < min)
                min = d;
            else if (d > max)
                max = d;
        }
        CurrentLightZRange.set(min - ActiveCamZ, max - ActiveCamZ); // Z relative to camera position
    }
#include "LightUtilities.hpp"

    void Light::draw() {
        // !! Here process water lum first, because after drawing all lights, we still have to apply ambient from meshes, so if we process water lum first, and then lum, then we don't need to change RT's to lum again when drawing ambient from meshes !!
        SetShadowOpacity(shadow_opacity);

        CurrentLight = T;
        switch (CurrentLight.type) {
        case LIGHT_DIR: {
            processLightDir_CASE_LIGHT_DIR(CurrentLight);
        } break;

        case LIGHT_POINT: {
            Flt range = CurrentLight.point.range();
            Vec pos = CurrentLight.point.pos;
            Flt z_center = DistPointActiveCamPlaneZ(pos);
            CurrentLightZRange.set(z_center - range, z_center + range);

            Bool front_face = LightFrontFaceBall(range, pos);
            UInt depth_func = (front_face ? FUNC_LESS : FUNC_GREATER);

            SetDepthAndShadow_CASE_LIGHT_POINT(CurrentLight, range, pos, front_face);
            SetMatrixCount();

            DrawWaterLum_CASE_LIGHT_POINT(CurrentLight, range, pos, front_face, depth_func);
            SetLightShadow_CASE_LIGHT_POINT(CurrentLight, range, pos, front_face);
            DrawLum_CASE_LIGHT_POINT(CurrentLight, range, pos, front_face, depth_func);
        } break;

        case LIGHT_LINEAR: {
            Flt range = CurrentLight.linear.range,
                z_center = DistPointActiveCamPlaneZ(CurrentLight.linear.pos); // Z relative to camera position
            CurrentLightZRange.set(z_center - range, z_center + range);       // use for DX12 OMSetDepthBounds
            Bool front_face = LightFrontFaceBall(range, CurrentLight.linear.pos);
            UInt depth_func = (front_face ? FUNC_LESS : FUNC_GREATER);
            MatrixM light_matrix(front_face ? range : -range, CurrentLight.linear.pos); // reverse faces
            D.depthClip(front_face);                                                    // Warning: not available on GL ES
            SetMatrixCount();                                                           // needed for drawing light mesh

            process_CASE_LIGHT_LINEAR(CurrentLight, depth_func, light_matrix);
        } break;

        case LIGHT_CONE: {
            SetLightZRangeCone();

            if (CurrentLight.shadow) {
                ProcessShadowMapForCone_CASE_LIGHT_CONE();
            }

            Bool front_face = LightFrontFace(CurrentLight.cone.pyramid);
            UInt depth_func = (front_face ? FUNC_LESS : FUNC_GREATER);
            MatrixM light_matrix;
            SetLightMatrixCone(light_matrix, front_face);
            SetMatrixCount();

            if (Renderer._water_nrm) {
                RenderWaterLum_CASE_LIGHT_CONE(light_matrix);
            }

            RenderLum_CASE_LIGHT_CONE(front_face, light_matrix);

            if (CurrentLight.image) {
                SetLightImage_CASE_LIGHT_CONE();
            }

            Bool clear = SetLum2_CASE_LIGHT_CONE(light_matrix);
            RenderMultiSample_CASE_LIGHT_CONE(light_matrix, depth_func);

        } break;
        }
        D.depth2DOff();
        Renderer._shd_1s.clear();
        Renderer._shd_ms.clear();
    }
    void Light::drawForward(ALPHA_MODE alpha) {
        SetShadowOpacity(shadow_opacity);

        CurrentLight = T;
        switch (CurrentLight.type) {
        case LIGHT_DIR: {
            DrawLightDirForward_CASE_LIGHT_DIR(alpha);
            if (Renderer._water_nrm) {
                DrawWaterLumDir_CASE_LIGHT_DIR();
            }
        } break;
        case LIGHT_POINT: {
            Flt range = CurrentLight.point.range(),
                z_center = DistPointActiveCamPlaneZ(CurrentLight.point.pos); // Z relative to camera position
            CurrentLightZRange.set(z_center - range, z_center + range);      // use for DX12 OMSetDepthBounds

            if (CurrentLight.shadow) {
                ShadowMap(range, CurrentLight.point.pos);
                Renderer._frst_light_offset = OFFSET(FRST, point_shd);
            } else {
                Renderer._frst_light_offset = OFFSET(FRST, point);
            }

            DrawLightPointForward_CASE_LIGHT_POINT(alpha, range);
            DrawWaterLumPoint_CASE_LIGHT_POINT(range);
            D.set2D();
        } break;

        case LIGHT_LINEAR: {
            process_FORWARD_CASE_LIGHT_LINEAR(alpha);
            if (Renderer._water_nrm) {
                drawWaterLumLinear_CASE_LIGHT_LINEAR();
            }
        } break;

        case LIGHT_CONE: {
            process_FORWARD_CASE_LIGHT_CONE(alpha); // Z relative to camera position
        } break;
        }
        Renderer._shd_1s.clear();
        Renderer._shd_ms.clear();
    }
    static Bool CreateLightFade(Flt & fade, C CPtr & key, Ptr user) {
        fade = 0;
        return true;
    }
    static Map<CPtr, Flt> LightFades(Compare, CreateLightFade);
    static Memc<FloatIndex> LightImportance;
    static Memc<Light> LightsTemp;

    void UpdateLights() {
        // limit
        if (D.maxLights()) {
            // limit number of lights
            if (Lights.elms() > D.maxLights()) {
                LightImportance.setNum(Lights.elms());
                REPA(Lights) {
                    Light &l = Lights[i];
                    FloatIndex &li = LightImportance[i];
                    li.i = i;
                    // since we sort from smallest to biggest value, and then keep only the first few lights, then make most important lights to have small 'f' "importance value" instead of big
                    switch (l.type) {
                    case LIGHT_DIR:
                        li.f = -l.dir.color_l.max();
                        break; // set highest color intensity directional lights to have smallest negative value from -Inf..0, and keep 0..Inf range for non-directional lights
                               /* Through simple visual test results, linear light ranges have to be set from point power as "linear.range=Sqrt(point.power)*3" to achieve similar overall brightness as point lights, and in order to achieve same importance as point lights using formula below, "/3" has to be applied for linear lights to counter the *3 factor from before, "Dist2(ActiveCam.at, l.pos())" was replaced as "1" since it's the same everywhere
                                  Flt importance_power1_point=1/1.0f, importance_power1_linear=1/Sqr(Sqrt(1.0f)*3 /3);
                                  Flt importance_power2_point=1/2.0f, importance_power2_linear=1/Sqr(Sqrt(2.0f)*3 /3);
                                  remember that here, smaller value = more important, higher value = less important, so high distance means less important, etc. */
                    case LIGHT_POINT:
                        li.f = Dist2(ActiveCam.at, l.pos()) / l.point.power;
                        break; // this is the most realistic/common type of light so this calculation has to be fast, 'Dist2' is used instead of 'Dist' to avoid having to do 'Sqrt', so 'point.power' needs to be linear
                    case LIGHT_LINEAR:
                        li.f = Dist2(ActiveCam.at, l.pos()) / Sqr(l.linear.range / 3);
                        break; // use 'Sqr' to keep proportions with point lights, div by 3 to achieve approximate similar importance as point lights
                    case LIGHT_CONE:
                        li.f = Dist2(ActiveCam.at, l.pos()) / Sqr(l.cone.pyramid.h / 3);
                        break; // use 'Sqr' to keep proportions with point lights, div by 3 to achieve approximate similar importance as point lights
                    }
                }
                LightImportance.sort(FloatIndex::Compare);
                LightsTemp.setNum(D.maxLights());
                FREPAO(LightsTemp) = Lights[LightImportance[i].i]; // always copy first maxLights
                if (D.maxLightsSoft())
                    for (Int i = D.maxLights(); i < Lights.elms(); i++) // for soft/relaxed copy remaining lights conditionally
                    {
                        C Light &light = Lights[LightImportance[i].i];
                        if (light.src)
                            LightsTemp.add(light); // only if has 'src' which means we can fade it
                    }
                Swap(Lights, LightsTemp);
            }

            // fade (fade in active lights, and all remaining fade out)
            Flt fade_dec = Time.rd() * 4, fade_inc = fade_dec * 2; // for simplicity we fade out all lights, so fade in has to be 2x bigger (to cancel out its fade out)
            Int fade_in_lights = Min(Lights.elms(), D.maxLights());
            REP(fade_in_lights) // fade in active lights
            {
                Light &l = Lights[i];
                if (l.src)
                    *LightFades(l.src) += fade_inc;
            }
            REPA(LightFades) // fade out all lights
            {
                Flt &fade = LightFades[i];
                fade -= fade_dec;
                if (fade <= 0)
                    LightFades.remove(i);
            }
            REPA(Lights) {
                Light &l = Lights[i];
                if (l.src) {
                    if (Flt *fade = LightFades.find(l.src)) {
                        if (*fade >= 1)
                            *fade = 1;
                        else
                            l.fade(*fade);
                    } else
                        Lights.remove(i); // if didn't found fade then delete this light
                }
            }
        }

        // sort
        // put main directional light at start (this is needed for RM_BLEND_LIGHT*, RT_FORWARD, Water.max1Light)
        if (Lights.elms() > 1) {
            Int main = -1;
            Flt power = 0;
            REPA(Lights) {
                Light &light = Lights[i];
                if (light.type == LIGHT_DIR && light.allow_main) {
                    Flt p = light.dir.color_l.max();
                    if (p > power) {
                        main = i;
                        power = p;
                    }
                }
            }
            if (main > 0)
                Swap(Lights.first(), Lights[main]); // >0 already handles !=-1 (not found) and !=0 (no need to move)
        }
    }
    void DrawLights() {
        if (Lights.elms()) {
            LightMode = ((Renderer._has & HAS_CLEAR_COAT) ? LIGHT_MODE_CLEAR_COAT : LIGHT_MODE_DEFAULT);
            REPAO(Lights).draw(); // apply in backward order to leave main shadow map in the end

            if (!ALWAYS_RESTORE_FRUSTUM) // here use !ALWAYS_RESTORE_FRUSTUM because we have to set frustum only if it wasn't restored before, if it was then it means we already have 'FrustumMain'
                Frustum = FrustumMain;   // restore 'Frustum' because it could've changed when drawing shadows

            D.depthClip(true); // restore default
        }
    }
    /******************************************************************************/
    Flt GetLightFade(CPtr src) {
        if (D.maxLights() && src) {
            if (Flt *fade = LightFades.find(src))
                return *fade;
            return 0;
        }
        return 1;
    }
    /******************************************************************************/
    void ShutLight() {
        Lights.del();
        LightImportance.del();
        LightMeshBall.del();
        LightMeshCone.del();
    }
    void InitLight() {
        HsmMatrix.x.x = 0.5f / 2;
        HsmMatrix.y.y = -0.5f / 3;
        HsmMatrix.z.z = 1;
        HsmMatrix.pos.w = 1;

        HsmMatrixCone = HsmMatrix;
        HsmMatrixCone.pos.x = 0.5f;
        HsmMatrixCone.pos.y = Avg(0, 2.0f / 3);

        MeshBase mshb;

        mshb.createIco(Ball(LIGHT_MESH_BALL_RADIUS), MESH_NONE, LIGHT_MESH_BALL_RES);
        LightMeshBall.create(mshb);
#if DEBUG && 0 // calculate actual distance
        mshb.createIco(Ball(1), 0, LIGHT_MESH_BALL_RES);
        Flt dist = 1;
        C Vec *pos = mshb.vtx.pos();
        REPA(mshb.tri) {
            C VecI &t = mshb.tri.ind(i);
            MIN(dist, Dist(VecZero, Tri(pos[t.x], pos[t.y], pos[t.z])));
        }
        REPA(mshb.quad) {
            C VecI4 &q = mshb.quad.ind(i);
            MIN(dist, Dist(VecZero, Quad(pos[q.x], pos[q.y], pos[q.z], pos[q.w])));
        }
        int z = 0;
#endif

        mshb.create(Cone(0, LIGHT_MESH_CONE_RADIUS, 1, VecZero, Vec(0, 0, 1)), MESH_NONE, LIGHT_MESH_CONE_RES);
        LightMeshCone.create(mshb);
#if DEBUG && 0                                                  // calculate actual distance
        mshb.createEdge(Circle(1), false, LIGHT_MESH_CONE_RES); // use a circle
        Flt dist = 1;
        C Vec *pos = mshb.vtx.pos();
        REPA(mshb.edge) {
            C VecI2 &e = mshb.edge.ind(i);
            C Vec2 &a = pos[e.x].xy, &b = pos[e.y].xy;
            MIN(dist, DistPointEdge(Vec2Zero, a, b));
        }
        int z = 0;
#endif
    }
    /******************************************************************************/
} // namespace EE
/******************************************************************************/
