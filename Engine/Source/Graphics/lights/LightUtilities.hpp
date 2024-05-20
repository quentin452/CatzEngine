#ifndef LIGHT_UTILITIES_HPP
#define LIGHT_UTILITIES_HPP

#include "stdafx.h"

// TODO IMPROVE AGAIN MAINTAINABILITY
// TODO MADE SAME FOR Light::drawForward(ALPHA_MODE alpha)
// STARTED UTILITIES METHODS FOR Light::draw()

inline void SetDepthAndShadow_CASE_LIGHT_POINT(Light &CurrentLight, Flt range, Vec &pos, Bool front_face) {
    if (CurrentLight.shadow) {
        D.depthClip(true);
        ShadowMap(range, CurrentLight.point.pos);
    }
    D.depthClip(front_face);
}

inline void SetLightShadow_CASE_LIGHT_POINT(Light &CurrentLight, Flt range, Vec &pos, Bool front_face) {
    if (CurrentLight.shadow) {
        Renderer.getShdRT();
        D.depth2DOn();
        Flt mp_z_z;
        ApplyViewSpaceBias(mp_z_z);
        if (!Renderer._ds->multiSample()) {
            Renderer.set(Renderer._shd_1s, Renderer._ds_1s, true, NEED_DEPTH_READ);
            REPS(Renderer._eye, Renderer._eye_num)
            if (SetLightEye(true))
                GetShdPoint(false)->draw(&CurrentLight.rect);
        } else {
            Renderer.set(Renderer._shd_ms, Renderer._ds, true, NEED_DEPTH_READ);
            D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
            REPS(Renderer._eye, Renderer._eye_num)
            if (SetLightEye(true))
                GetShdPoint(true)->draw(&CurrentLight.rect);
            Renderer.set(Renderer._shd_1s, Renderer._ds_1s, true, NEED_DEPTH_READ);
            D.stencil(STENCIL_NONE);
            REPS(Renderer._eye, Renderer._eye_num)
            if (SetLightEye(true))
                GetShdPoint(false)->draw(&CurrentLight.rect);
        }
        RestoreViewSpaceBias(mp_z_z);
        MapSoft((front_face ? FUNC_LESS : FUNC_GREATER), &MatrixM(front_face ? range : -range, pos));
        ApplyVolumetric(CurrentLight.point);
    }
}

inline void DrawWaterLum_CASE_LIGHT_POINT(Light &CurrentLight, Flt range, Vec &pos, Bool front_face, UInt depth_func) {
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

inline void DrawLum_CASE_LIGHT_POINT(Light &CurrentLight, Flt range, Vec &pos, Bool front_face, UInt depth_func) {
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

inline void ProcessShadowMapForCone_CASE_LIGHT_CONE() {
    D.depthClip(true);
    ShadowMap(CurrentLight.cone);
}

inline void RenderWaterLum_CASE_LIGHT_CONE(MatrixM &light_matrix) {
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

inline void RenderLum_CASE_LIGHT_CONE(Bool front_face, MatrixM &light_matrix) {
    if (CurrentLight.shadow) {
        Renderer.getShdRT();
        D.depth2DOn();
        Flt mp_z_z;
        ApplyViewSpaceBias(mp_z_z);
        if (!Renderer._ds->multiSample()) {
            Renderer.set(Renderer._shd_1s, Renderer._ds_1s, true, NEED_DEPTH_READ);
            REPS(Renderer._eye, Renderer._eye_num)
            if (SetLightEye(true))
                GetShdCone(false)->draw(&CurrentLight.rect);
        } else {
            Renderer.set(Renderer._shd_ms, Renderer._ds, true, NEED_DEPTH_READ);
            D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
            REPS(Renderer._eye, Renderer._eye_num)
            if (SetLightEye(true))
                GetShdCone(true)->draw(&CurrentLight.rect);
            Renderer.set(Renderer._shd_1s, Renderer._ds_1s, true, NEED_DEPTH_READ);
            D.stencil(STENCIL_NONE);
            REPS(Renderer._eye, Renderer._eye_num)
            if (SetLightEye(true))
                GetShdCone(false)->draw(&CurrentLight.rect);
        }
        RestoreViewSpaceBias(mp_z_z);
        MapSoft(front_face ? FUNC_LESS : FUNC_GREATER, &light_matrix);
        ApplyVolumetric(CurrentLight.cone);
    }
}

inline void SetLightImage_CASE_LIGHT_CONE() {
    Sh.LightMapScale->set(CurrentLight.image_scale);
    Sh.Img[2]->set(CurrentLight.image);
}

inline Bool SetLum2_CASE_LIGHT_CONE(MatrixM &light_matrix) {
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

inline void RenderMultiSample_CASE_LIGHT_CONE(MatrixM &light_matrix, UInt depth_func) {
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

inline void setupDepthAndShadows_CASE_LIGHT_DIR(Light &CurrentLight, int &shd_map_num, bool &cloud, UInt &depth_func) {
    D.depthClip(true);

    if (CurrentLight.shadow) {
        shd_map_num = D.shadowMapNumActual();
        cloud = ShadowMap(CurrentLight.dir);
    }

    depth_func = FUNC_FOREGROUND;
}

inline void setupWaterLum_CASE_LIGHT_DIR(int &shd_map_num, bool &cloud) {
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

inline void setupLum_CASE_LIGHT_DIR(Light &CurrentLight, int shd_map_num, bool cloud) {
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

inline void setupVolumetric_CASE_LIGHT_DIR(Light &CurrentLight, int shd_map_num,UInt depth_func) {
    MapSoft(depth_func);
    ApplyVolumetric(CurrentLight.dir, shd_map_num, false);
}

inline void processLighting_CASE_LIGHT_DIR(Light &CurrentLight, int shd_map_num, bool cloud, UInt depth_func) {
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

inline void processLightDir_CASE_LIGHT_DIR(Light &CurrentLight) {
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
        setupVolumetric_CASE_LIGHT_DIR(CurrentLight, shd_map_num,depth_func);
    }

    processLighting_CASE_LIGHT_DIR(CurrentLight, shd_map_num, cloud, depth_func);
}

// FINISHED UTILITIES METHODS FOR Light::draw()

#endif // LIGHT_UTILITIES_HPP
