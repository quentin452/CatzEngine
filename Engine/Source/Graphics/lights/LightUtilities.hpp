#pragma once

#include "stdafx.h"

// STARTED UTILITIES METHODS FOR Light::draw()

INLINE void SetDepthAndShadow_CASE_LIGHT_POINT(Light &CurrentLight, Flt range, Vec &pos, Bool front_face) {
    if (CurrentLight.shadow) {
        D.depthClip(true);
        ShadowMap(range, CurrentLight.point.pos);
    }
    D.depthClip(front_face);
}

INLINE void SetLightShadow(Light &CurrentLight, std::function<Shader *(bool)> GetShdPointFunc) {
    if (!Renderer._ds->multiSample()) {
        Renderer.set(Renderer._shd_1s, Renderer._ds_1s, true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth' optimization and 3D geometric shaders
        REPS(Renderer._eye, Renderer._eye_num)
        if (SetLightEye(true))
            GetShdPointFunc(false)->draw(&CurrentLight.rect);
    } else { // we can ignore 'Renderer.hasStencilAttached' because we would have to apply for all samples of '_shd_ms' and '_shd_1s' which will happen anyway below
        Renderer.set(Renderer._shd_ms, Renderer._ds, true, NEED_DEPTH_READ);
        D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
        REPS(Renderer._eye, Renderer._eye_num)
        if (SetLightEye(true))
            GetShdPointFunc(true)->draw(&CurrentLight.rect); // use DS because it may be used for 'D.depth' optimization and 3D geometric shaders
        Renderer.set(Renderer._shd_1s, Renderer._ds_1s, true, NEED_DEPTH_READ);
        D.stencil(STENCIL_NONE);
        REPS(Renderer._eye, Renderer._eye_num)
        if (SetLightEye(true))
            GetShdPointFunc(false)->draw(&CurrentLight.rect); // use DS because it may be used for 'D.depth' optimization and 3D geometric shaders, for all stencil samples because they are needed for smoothing
    }
}
INLINE void SetLightShadow_CASE_LIGHT_POINT(Light &CurrentLight, Flt range, Vec &pos, Bool front_face) {
    if (CurrentLight.shadow) {
        Renderer.getShdRT();
        D.depth2DOn();
        Flt mp_z_z;
        ApplyViewSpaceBias(mp_z_z);
        SetLightShadow(CurrentLight, &GetShdPoint);
        RestoreViewSpaceBias(mp_z_z);
        MapSoft((front_face ? FUNC_LESS : FUNC_GREATER), &MatrixM(front_face ? range : -range, pos));
        ApplyVolumetric(CurrentLight.point);
    }
}

INLINE void DrawWaterLum_CASE_LIGHT_POINT(Light &CurrentLight, Flt range, Vec &pos, Bool front_face, UInt depth_func) {
    if (Renderer._water_nrm) {
        D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
        Sh.Depth->set(Renderer._water_ds);
        if (CurrentLight.shadow) {
            Renderer.getShdRT();
            Renderer.set(Renderer._shd_1s, Renderer._water_ds, true, NEED_DEPTH_READ);
            D.depth2DOn();
            REPS(Renderer._eye, Renderer._eye_num)
            if (SetLightEye(true))
                GetShdPoint(false)->draw(&CurrentLight.rect);
        }
        SetWaterLum();
        D.depth2DOn(depth_func);
        DrawLightPoint(MatrixM(front_face ? range : -range, pos), 0, LIGHT_MODE_WATER);
        Sh.Depth->set(Renderer._ds_1s);
        D.stencil(STENCIL_NONE);
    }
}

INLINE void DrawLum_CASE_LIGHT_POINT(Light &CurrentLight, Flt range, Vec &pos, Bool front_face, UInt depth_func) {
    Bool clear = SetLum();
    D.depth2DOn(depth_func);
    if (!Renderer._ds->multiSample()) {
        DrawLightPoint(MatrixM(front_face ? range : -range, pos), 0, LightMode);
    } else {
        if (Renderer.hasStencilAttached()) {
            D.stencil(STENCIL_MSAA_TEST, 0);
            DrawLightPoint(MatrixM(front_face ? range : -range, pos), 1, LightMode);
        }
        SetLumMS(clear);
        D.depth2DOn(depth_func);
        D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
        DrawLightPoint(MatrixM(front_face ? range : -range, pos), 2, LightMode);
        D.stencil(STENCIL_NONE);
    }
}

INLINE void ProcessShadowMapForCone_CASE_LIGHT_CONE() {
    D.depthClip(true);
    ShadowMap(CurrentLight.cone);
}

INLINE void RenderWaterLum_CASE_LIGHT_CONE(MatrixM &light_matrix) {
    D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
    Sh.Depth->set(Renderer._water_ds);
    if (CurrentLight.shadow) {
        Renderer.getShdRT();
        Renderer.set(Renderer._shd_1s, Renderer._water_ds, true, NEED_DEPTH_READ);
        D.depth2DOn();
        REPS(Renderer._eye, Renderer._eye_num)
        if (SetLightEye(true))
            GetShdCone(false)->draw(&CurrentLight.rect);
    }
    SetWaterLum();
    D.depth2DOn(FUNC_LESS);
    DrawLightCone(light_matrix, 0, LIGHT_MODE_WATER);
    Sh.Depth->set(Renderer._ds_1s);
    D.stencil(STENCIL_NONE);
}

INLINE void RenderLum_CASE_LIGHT_CONE(Bool front_face, MatrixM &light_matrix) {
    if (CurrentLight.shadow) {
        Renderer.getShdRT();
        D.depth2DOn();
        Flt mp_z_z;
        ApplyViewSpaceBias(mp_z_z);
        SetLightShadow(CurrentLight, &GetShdCone);
        RestoreViewSpaceBias(mp_z_z);
        MapSoft(front_face ? FUNC_LESS : FUNC_GREATER, &light_matrix);
        ApplyVolumetric(CurrentLight.cone);
    }
}

INLINE void SetLightImage_CASE_LIGHT_CONE() {
    Sh.LightMapScale->set(CurrentLight.image_scale);
    Sh.Img[2]->set(CurrentLight.image);
}

INLINE Bool SetLum2_CASE_LIGHT_CONE(MatrixM &light_matrix) {
    Bool clear = true;
    D.depth2DOn(FUNC_LESS);
    if (!Renderer._ds->multiSample()) {
        DrawLightCone(light_matrix, 0, LightMode);
    } else {
        if (Renderer.hasStencilAttached()) {
            D.stencil(STENCIL_MSAA_TEST, 0);
            DrawLightCone(light_matrix, 1, LightMode);
        }
        SetLumMS(clear);
        D.depth2DOn(FUNC_LESS);
        D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
        DrawLightCone(light_matrix, 2, LightMode);
        D.stencil(STENCIL_NONE);
    }
    return clear;
}

INLINE void RenderMultiSample_CASE_LIGHT_CONE(MatrixM &light_matrix, UInt depth_func) {
    D.depth2DOn();
    if (!Renderer._ds->multiSample()) {
        DrawLightCone(light_matrix, 0, LightMode);
    } else {
        if (Renderer.hasStencilAttached()) {
            D.stencil(STENCIL_MSAA_TEST, 0);
            DrawLightCone(light_matrix, 1, LightMode);
        }
        SetLumMS(true);
        D.depth2DOn();
        D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
        DrawLightCone(light_matrix, 2, LightMode);
        D.stencil(STENCIL_NONE);
    }
}

INLINE void setupDepthAndShadows_CASE_LIGHT_DIR(Light &CurrentLight, int &shd_map_num, bool &cloud, UInt &depth_func) {
    D.depthClip(true);
    if (CurrentLight.shadow) {
        shd_map_num = D.shadowMapNumActual();
        cloud = ShadowMap(CurrentLight.dir);
    }
    depth_func = FUNC_FOREGROUND;
}

INLINE void setupWaterLum_CASE_LIGHT_DIR(int &shd_map_num, bool &cloud) {
    D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
    Sh.Depth->set(Renderer._water_ds);
    if (CurrentLight.shadow) {
        Renderer.getShdRT();
        Renderer.set(Renderer._shd_1s, Renderer._water_ds, true, NEED_DEPTH_READ);
        D.depth2DOn();
        REPS(Renderer._eye, Renderer._eye_num)
        if (SetLightEye(true))
            GetShdDir(Mid(shd_map_num, 1, 6), cloud, false)->draw(&CurrentLight.rect);
    }
    SetWaterLum();
    D.depth2DOn();
    DrawLightDir(0, LIGHT_MODE_WATER);
    Sh.Depth->set(Renderer._ds_1s);
    D.stencil(STENCIL_NONE);
}

INLINE void setupLum_CASE_LIGHT_DIR(Light &CurrentLight, int shd_map_num, bool cloud) {
    Renderer.getShdRT();
    Bool multi_sample = Renderer._ds->multiSample();
    D.depth2DOn();
    Flt mp_z_z;
    ApplyViewSpaceBias(mp_z_z);
    Renderer.set(Renderer._shd_1s, Renderer._ds_1s, true, NEED_DEPTH_READ);
    REPS(Renderer._eye, Renderer._eye_num)
    if (SetLightEye(true))
        GetShdDir(shd_map_num, cloud, false)->draw(&CurrentLight.rect);
    if (multi_sample) {
        D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
        Renderer.set(Renderer._shd_ms, Renderer._ds, true, NEED_DEPTH_READ);
        GetShdDir(shd_map_num, cloud, true)->draw(&CurrentLight.rect);
        D.stencil(STENCIL_NONE);
        Renderer.set(Renderer._shd_1s, Renderer._ds_1s, true, NEED_DEPTH_READ);
        GetShdDir(shd_map_num, cloud, false)->draw(&CurrentLight.rect);
    }
    RestoreViewSpaceBias(mp_z_z);
}

INLINE void setupVolumetric_CASE_LIGHT_DIR(Light &CurrentLight, int shd_map_num, UInt depth_func) {
    MapSoft(depth_func);
    ApplyVolumetric(CurrentLight.dir, shd_map_num, false);
}

INLINE void processLighting_CASE_LIGHT_DIR(Light &CurrentLight, int shd_map_num, bool cloud, UInt depth_func) {
    Bool clear = SetLum();
    D.depth2DOn(depth_func);
    if (!Renderer._ds->multiSample()) {
        DrawLightDir(0, LightMode);
    } else {
        if (Renderer.hasStencilAttached()) {
            D.stencil(STENCIL_MSAA_TEST, 0);
            DrawLightDir(1, LightMode);
        }
        SetLumMS(clear);
        D.depth2DOn(depth_func);
        D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
        DrawLightDir(2, LightMode);
        D.stencil(STENCIL_NONE);
    }
}

INLINE void processDirectionalLight(Light &CurrentLight) {
    int shd_map_num = 0;
    bool cloud = false;
    UInt depth_func = 0;
    setupDepthAndShadows_CASE_LIGHT_DIR(CurrentLight, shd_map_num, cloud, depth_func);
    // Water lum first
    if (Renderer._water_nrm) {
        setupWaterLum_CASE_LIGHT_DIR(shd_map_num, cloud);
    }
    // Lum
    if (CurrentLight.shadow) {
        setupLum_CASE_LIGHT_DIR(CurrentLight, shd_map_num, cloud);
        setupVolumetric_CASE_LIGHT_DIR(CurrentLight, shd_map_num, depth_func);
    }
    processLighting_CASE_LIGHT_DIR(CurrentLight, shd_map_num, cloud, depth_func);
}

INLINE void DrawWaterLum_CASE_LIGHT_LINEAR(Light &CurrentLight) {
    Flt range = CurrentLight.linear.range,
        z_center = DistPointActiveCamPlaneZ(CurrentLight.linear.pos); // Z relative to camera position
    CurrentLightZRange.set(z_center - range, z_center + range);       // use for DX12 OMSetDepthBounds
    if (CurrentLight.shadow) {
        D.depthClip(true);
        ShadowMap(range, CurrentLight.linear.pos);
    }
    Bool front_face = LightFrontFaceBall(range, CurrentLight.linear.pos);
    UInt depth_func = (front_face ? FUNC_LESS : FUNC_GREATER);
    MatrixM light_matrix(front_face ? range : -range, CurrentLight.linear.pos); // reverse faces
    D.depthClip(front_face);                                                    // Warning: not available on GL ES
    SetMatrixCount();                                                           // needed for drawing light mesh
    if (Renderer._water_nrm) {
        D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
        Sh.Depth->set(Renderer._water_ds); // set water depth
        if (CurrentLight.shadow) {
            // no need for view space bias, because we're calculating shadow for water surfaces, which by themself don't cast shadows and are usually above shadow surfaces
            Renderer.getShdRT();
            Renderer.set(Renderer._shd_1s, Renderer._water_ds, true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth' optimization, 3D geometric shaders and stencil tests
            D.depth2DOn();
            REPS(Renderer._eye, Renderer._eye_num)
            if (SetLightEye(true))
                GetShdPoint(false)->draw(&CurrentLight.rect);
        }
        SetWaterLum();
        D.depth2DOn(depth_func);
        DrawLightLinear(light_matrix, 0, LIGHT_MODE_WATER);
        Sh.Depth->set(Renderer._ds_1s); // restore default depth
        D.stencil(STENCIL_NONE);
    }
}

INLINE void DrawLum_CASE_LIGHT_LINEAR(Light &CurrentLight, UInt depth_func, MatrixM &light_matrix) {
    if (CurrentLight.shadow) {
        Renderer.getShdRT();
        D.depth2DOn();
        Flt mp_z_z;
        ApplyViewSpaceBias(mp_z_z);
        SetLightShadow(CurrentLight, &GetShdPoint);
        RestoreViewSpaceBias(mp_z_z);
        MapSoft(depth_func, &light_matrix);
        ApplyVolumetric(CurrentLight.linear);
    }
    Bool clear = SetLum();
    D.depth2DOn(depth_func);
    if (!Renderer._ds->multiSample()) // 1-sample
    {
        DrawLightLinear(light_matrix, 0, LightMode);
    } else // multi-sample
    {
        if (Renderer.hasStencilAttached()) // if we can use stencil tests, then process 1-sample pixels using 1-sample shader, if we can't use stencil then all pixels will be processed using multi-sample shader later below
        {
            D.stencil(STENCIL_MSAA_TEST, 0);
            DrawLightLinear(light_matrix, 1, LightMode);
        }
        SetLumMS(clear);
        D.depth2DOn(depth_func);
        /*if(Renderer.hasStencilAttached()) not needed because stencil tests are disabled without stencil RT */ D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
        DrawLightLinear(light_matrix, 2, LightMode);
        D.stencil(STENCIL_NONE);
    }
}

INLINE void process_CASE_LIGHT_LINEAR(Light &CurrentLight, UInt depth_func, MatrixM &light_matrix) {
    // water lum first, as noted above in the comments
    DrawWaterLum_CASE_LIGHT_LINEAR(CurrentLight);
    // lum
    DrawLum_CASE_LIGHT_LINEAR(CurrentLight, depth_func, light_matrix);
}

INLINE void processPointLight(Light &CurrentLight) {
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
}

INLINE void processLinearLight(Light &CurrentLight) {
    Flt range = CurrentLight.linear.range;
    Flt z_center = DistPointActiveCamPlaneZ(CurrentLight.linear.pos);
    CurrentLightZRange.set(z_center - range, z_center + range);
    Bool front_face = LightFrontFaceBall(range, CurrentLight.linear.pos);
    UInt depth_func = (front_face ? FUNC_LESS : FUNC_GREATER);
    MatrixM light_matrix(front_face ? range : -range, CurrentLight.linear.pos);
    D.depthClip(front_face);
    SetMatrixCount();
    process_CASE_LIGHT_LINEAR(CurrentLight, depth_func, light_matrix);
}

INLINE void processConeLight(Light &CurrentLight) {
    SetLightZRangeCone();
    if (CurrentLight.shadow)
        ProcessShadowMapForCone_CASE_LIGHT_CONE();
    Bool front_face = LightFrontFace(CurrentLight.cone.pyramid);
    UInt depth_func = (front_face ? FUNC_LESS : FUNC_GREATER);
    MatrixM light_matrix;
    SetLightMatrixCone(light_matrix, front_face);
    SetMatrixCount();
    if (Renderer._water_nrm)
        RenderWaterLum_CASE_LIGHT_CONE(light_matrix);
    RenderLum_CASE_LIGHT_CONE(front_face, light_matrix);
    if (CurrentLight.image)
        SetLightImage_CASE_LIGHT_CONE();
    Bool clear = SetLum2_CASE_LIGHT_CONE(light_matrix);
    RenderMultiSample_CASE_LIGHT_CONE(light_matrix, depth_func);
}

// FINISHED UTILITIES METHODS FOR Light::draw()

// STARTED UTILITIES METHODS FOR Light::drawForward(ALPHA_MODE alpha)

INLINE void DrawLightDirForward_CASE_LIGHT_DIR(ALPHA_MODE alpha) {
    Int shd_map_num;
    if (CurrentLight.shadow) {
        shd_map_num = D.shadowMapNumActual();
        ShadowMap(CurrentLight.dir);
        Renderer._frst_light_offset = OFFSET(FRST, dir_shd[Mid(shd_map_num, 1, 6) - 1]);
    } else {
        Renderer._frst_light_offset = OFFSET(FRST, dir);
    }
    Renderer.setForwardCol();
    D.alpha(alpha);
    D.set3D();
    D.depth(true);
    if (!ALWAYS_RESTORE_FRUSTUM) // here use !ALWAYS_RESTORE_FRUSTUM because we have to set frustum only if it wasn't restored before, if it was then it means we already have 'FrustumMain'
        Frustum = FrustumMain;   // directional lights always use original frustum
    if (Renderer.firstPass()) {
        D.stencil(STENCIL_ALWAYS_SET, 0);
    } else { // we need to generate list of objects
        Renderer.mode(RM_PREPARE);
        Renderer._render();
        D.clipAllow(true);
    }
    Renderer.mode(RM_OPAQUE);
    REPS(Renderer._eye, Renderer._eye_num) {
        Renderer.setEyeViewportCam();
        if (CurrentLight.shadow)
            SetShdMatrix();
        CurrentLight.dir.set();
        if (Renderer.secondaryPass())
            D.clip(Renderer._clip); // clip rendering to area affected by the light
        DrawOpaqueInstances();
        Renderer._render();
    }
    ClearOpaqueInstances();
    D.set2D();
    if (Renderer.firstPass()) {
        D.stencil(STENCIL_NONE);
        Renderer.resolveDepth();
    }
}

INLINE void DrawWaterLumDir_CASE_LIGHT_DIR() {
    D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
    Sh.Depth->set(Renderer._water_ds); // set water depth
    UInt depth_func = FUNC_FOREGROUND;
    Int shd_map_num;
    if (CurrentLight.shadow) {
        shd_map_num = D.shadowMapNumActual();
        // no need for view space bias, because we're calculating shadow for water surfaces, which by themself don't cast shadows and are usually above shadow surfaces
        Renderer.getShdRT();
        Renderer.set(Renderer._shd_1s, Renderer._water_ds, true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth' optimization, 3D geometric shaders and stencil tests
        D.depth2DOn();
        REPS(Renderer._eye, Renderer._eye_num)
        if (SetLightEye(true))
            GetShdDir(Mid(shd_map_num, 1, 6), false, false)->draw(&CurrentLight.rect);
    }
    SetWaterLum();
    D.depth2DOn(depth_func);
    DrawLightDir(0, LIGHT_MODE_WATER);
    Sh.Depth->set(Renderer._ds_1s); // restore default depth
    D.depth2DOff();
    D.stencil(STENCIL_NONE);
}

INLINE void DrawLightPointForward_CASE_LIGHT_POINT(ALPHA_MODE alpha, Flt range) {
    Renderer.setForwardCol();
    D.alpha(alpha);
    D.set3D();
    D.depth(true);
    if (!ALWAYS_RESTORE_FRUSTUM)
        Frustum = FrustumMain;
    if (Renderer.firstPass()) {
        D.stencil(STENCIL_ALWAYS_SET, 0);
        // need to use entire Frustum for first pass
    } else { // we need to generate list of objects
        Frustum.setExtraBall(BallM(range, CurrentLight.point.pos));
        Renderer.mode(RM_PREPARE);
        Renderer._render();
        if (ALWAYS_RESTORE_FRUSTUM)
            Frustum.clearExtraBall();
        D.clipAllow(true);
    }
    Renderer.mode(RM_OPAQUE);
    REPS(Renderer._eye, Renderer._eye_num) {
        Renderer.setEyeViewportCam();
        if (GetCurrentLightRect()) {
            if (CurrentLight.shadow)
                SetShdMatrix();
            CurrentLight.point.set(CurrentLight.shadow_opacity);
            if (Renderer.secondaryPass())
                D.clip(CurrentLight.rect & Renderer._clip); // clip rendering to area affected by the light
            DrawOpaqueInstances();
            Renderer._render();
        }
    }
    ClearOpaqueInstances();
    D.set2D();
    if (Renderer.firstPass()) {
        D.stencil(STENCIL_NONE);
        Renderer.resolveDepth();
    }
}

INLINE void DrawWaterLumPoint_CASE_LIGHT_POINT(Flt range) {
    D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
    Sh.Depth->set(Renderer._water_ds); // set water depth
    Bool front_face = LightFrontFaceBall(range, CurrentLight.point.pos);
    UInt depth_func = (front_face ? FUNC_LESS : FUNC_GREATER);
    MatrixM light_matrix(front_face ? range : -range, CurrentLight.point.pos); // reverse faces
    D.depthClip(front_face);                                                   // Warning: not available on GL ES
    SetMatrixCount();                                                          // needed for drawing light mesh
    if (CurrentLight.shadow) {
        // no need for view space bias, because we're calculating shadow for water surfaces, which by themself don't cast shadows and are usually above shadow surfaces
        Renderer.getShdRT();
        Renderer.set(Renderer._shd_1s, Renderer._water_ds, true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth' optimization, 3D geometric shaders and stencil tests
        D.depth2DOn();
        REPS(Renderer._eye, Renderer._eye_num)
        if (SetLightEye(true))
            GetShdPoint(false)->draw(&CurrentLight.rect);
    }
    SetWaterLum();
    D.depth2DOn(depth_func);
    DrawLightPoint(light_matrix, 0, LIGHT_MODE_WATER);
    Sh.Depth->set(Renderer._ds_1s); // restore default depth
    D.depth2DOff();
    D.depthClip(true);
    D.stencil(STENCIL_NONE);
}

INLINE void process_FORWARD_CASE_LIGHT_LINEAR(ALPHA_MODE alpha) {
    Flt range = CurrentLight.linear.range,
        z_center = DistPointActiveCamPlaneZ(CurrentLight.linear.pos); // Z relative to camera position
    CurrentLightZRange.set(z_center - range, z_center + range);       // use for DX12 OMSetDepthBounds
    if (CurrentLight.shadow) {
        ShadowMap(range, CurrentLight.linear.pos);
        Renderer._frst_light_offset = OFFSET(FRST, linear_shd);
    } else {
        Renderer._frst_light_offset = OFFSET(FRST, linear);
    }
    Renderer.setForwardCol();
    D.alpha(alpha);
    D.set3D();
    D.depth(true);
    if (!ALWAYS_RESTORE_FRUSTUM) // here use !ALWAYS_RESTORE_FRUSTUM because we have to set frustum only if it wasn't restored before, if it was then it means we already have 'FrustumMain'
        Frustum = FrustumMain;
    if (Renderer.firstPass()) {
        D.stencil(STENCIL_ALWAYS_SET, 0);
        // need to use entire Frustum for first pass
    } else { // we need to generate list of objects
        Frustum.setExtraBall(BallM(range, CurrentLight.linear.pos));
        Renderer.mode(RM_PREPARE);
        Renderer._render();
        if (ALWAYS_RESTORE_FRUSTUM)
            Frustum.clearExtraBall();
        D.clipAllow(true);
    }
    Renderer.mode(RM_OPAQUE);
    REPS(Renderer._eye, Renderer._eye_num) {
        Renderer.setEyeViewportCam();
        if (GetCurrentLightRect()) // check this after setting viewport and camera
        {
            if (CurrentLight.shadow)
                SetShdMatrix();
            CurrentLight.linear.set(CurrentLight.shadow_opacity);
            if (Renderer.secondaryPass())
                D.clip(CurrentLight.rect & Renderer._clip); // clip rendering to area affected by the light
            DrawOpaqueInstances();
            Renderer._render();
        }
    }
    ClearOpaqueInstances();
    D.set2D();
    if (Renderer.firstPass()) {
        D.stencil(STENCIL_NONE);
        Renderer.resolveDepth();
    }
}

INLINE void CommonWaterLumSetup(Bool front_face, UInt depth_func, const MatrixM &light_matrix) {
    D.stencil(STENCIL_WATER_TEST, STENCIL_REF_WATER);
    Sh.Depth->set(Renderer._water_ds);
    D.depthClip(front_face);
    SetMatrixCount();
    if (CurrentLight.shadow) {
        Renderer.getShdRT();
        Renderer.set(Renderer._shd_1s, Renderer._water_ds, true, NEED_DEPTH_READ);
        D.depth2DOn();
        REPS(Renderer._eye, Renderer._eye_num) {
            if (SetLightEye(true)) {
                if (CurrentLight.type == LIGHT_LINEAR)
                    GetShdPoint(false)->draw(&CurrentLight.rect);
                else
                    GetShdCone(false)->draw(&CurrentLight.rect);
            }
        }
    }
    SetWaterLum();
    D.depth2DOn(depth_func);
    Sh.Depth->set(Renderer._ds_1s);
    D.depth2DOff();
    D.depthClip(true);
    D.stencil(STENCIL_NONE);
}

INLINE void drawWaterLumLinear_CASE_LIGHT_LINEAR() {
    Bool front_face = LightFrontFaceBall(CurrentLight.linear.range, CurrentLight.linear.pos);
    UInt depth_func = (front_face ? FUNC_LESS : FUNC_GREATER);
    MatrixM light_matrix(front_face ? CurrentLight.linear.range : -CurrentLight.linear.range, CurrentLight.linear.pos);
    CommonWaterLumSetup(front_face, depth_func, light_matrix);
    DrawLightLinear(light_matrix, 0, LIGHT_MODE_WATER);
}

INLINE void RenderWaterLuminosity_CASE_LIGHT_CONE() {
    Bool front_face = LightFrontFace(CurrentLight.cone.pyramid);
    UInt depth_func = (front_face ? FUNC_LESS : FUNC_GREATER);
    MatrixM light_matrix;
    SetLightMatrixCone(light_matrix, front_face);
    CommonWaterLumSetup(front_face, depth_func, light_matrix);
    DrawLightCone(light_matrix, 0, LIGHT_MODE_WATER);
}

INLINE void process_FORWARD_CASE_LIGHT_CONE(ALPHA_MODE alpha) {
    SetLightZRangeCone(); // Z relative to camera position
    if (CurrentLight.shadow) {
        ShadowMap(CurrentLight.cone);
        Renderer._frst_light_offset = OFFSET(FRST, cone_shd);
    } else {
        Renderer._frst_light_offset = OFFSET(FRST, cone);
    }
    Renderer.setForwardCol();
    D.alpha(alpha);
    D.set3D();
    D.depth(true);
    if (Renderer.firstPass()) {
        D.stencil(STENCIL_ALWAYS_SET, 0);
        if (!ALWAYS_RESTORE_FRUSTUM)
            Frustum = FrustumMain;
    } else {
        Frustum.from(CurrentLight.cone.pyramid);
        Renderer.mode(RM_PREPARE);
        Renderer._render();
        if (ALWAYS_RESTORE_FRUSTUM) 
            Frustum = FrustumMain;
        D.clipAllow(true);
    }
    Renderer.mode(RM_OPAQUE);
    REPS(Renderer._eye, Renderer._eye_num) {
        Renderer.setEyeViewportCam();
        if (GetCurrentLightRect()) {
            if (CurrentLight.shadow) 
                SetShdMatrix();
            CurrentLight.cone.set(CurrentLight.shadow_opacity);
            if (Renderer.secondaryPass()) 
                D.clip(CurrentLight.rect & Renderer._clip);
            DrawOpaqueInstances();
            Renderer._render();
        }
    }
    ClearOpaqueInstances();
    D.set2D();
    if (Renderer.firstPass()) {
        D.stencil(STENCIL_NONE);
        Renderer.resolveDepth();
    }
    // water lum
    if (Renderer._water_nrm)
        RenderWaterLuminosity_CASE_LIGHT_CONE();
}

INLINE void processDirectionalLightForward(Light &CurrentLight, ALPHA_MODE alpha) {
    DrawLightDirForward_CASE_LIGHT_DIR(alpha);
    if (Renderer._water_nrm)
        DrawWaterLumDir_CASE_LIGHT_DIR();
}

INLINE void processPointLightForward(Light &CurrentLight, ALPHA_MODE alpha) {
    Flt range = CurrentLight.point.range();
    Flt z_center = DistPointActiveCamPlaneZ(CurrentLight.point.pos);
    CurrentLightZRange.set(z_center - range, z_center + range);
    if (CurrentLight.shadow) {
        ShadowMap(range, CurrentLight.point.pos);
        Renderer._frst_light_offset = OFFSET(FRST, point_shd);
    } else {
        Renderer._frst_light_offset = OFFSET(FRST, point);
    }
    DrawLightPointForward_CASE_LIGHT_POINT(alpha, range);
    if (Renderer._water_nrm)
        DrawWaterLumPoint_CASE_LIGHT_POINT(range);
    D.set2D();
}

INLINE void processLinearLightForward(Light &CurrentLight, ALPHA_MODE alpha) {
    process_FORWARD_CASE_LIGHT_LINEAR(alpha);
    if (Renderer._water_nrm)
        drawWaterLumLinear_CASE_LIGHT_LINEAR();
}

INLINE void processConeLightForward(Light &CurrentLight, ALPHA_MODE alpha) {
    process_FORWARD_CASE_LIGHT_CONE(alpha);
}
// FINISHED UTILITIES METHODS FOR Light::drawForward(ALPHA_MODE alpha)

// STARTED COMMON CODE
INLINE void finalizeDrawing() {
    D.depth2DOff();
    Renderer._shd_1s.clear();
    Renderer._shd_ms.clear();
}
// FINISHED COMMON CODE