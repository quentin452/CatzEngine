/******************************************************************************/
#include "../../stdafx.h"
namespace EE {
/******************************************************************************/
#define OPAQUE_FRAC 0.8f
// Flt alpha=1-Sat((Abs(pos.z)-opaque_frac)/(1-opaque_frac));
// Flt alpha=Sat(Abs(pos.z)/-(1-opaque_frac) + (opaque_frac/(1-opaque_frac)+1));
// Flt alpha=Sat(Abs(pos.z)*(1/(opaque_frac-1)) + (opaque_frac/(1-opaque_frac)+1));
inline Flt OpaqueFracMul(Flt frac = OPAQUE_FRAC) { return 1.0f / (frac - 1); }
inline Flt OpaqueFracAdd(Flt frac = OPAQUE_FRAC) { return frac / (1 - frac) + 1; }
/******************************************************************************/
static const ALPHA_MODE Alpha[] =
    {
        ALPHA_OVERLAY,             // 0=overlay
        ALPHA_RENDER_BLEND_FACTOR, // 1=blend
        ALPHA_ADD,                 // 2=palette
};
static Int GetMode() {
    if (Renderer() == RM_OVERLAY)
        return 0; // overlay
    if (!Renderer._palette_mode)
        return 1; // blend
    return 2;     // palette
}
/******************************************************************************/
void Decal::zero() {
    terrain_only = false;
    color = 1;
    matrix.identity();
    Zero(_shader);
}
void Decal::del() {
    _material.clear();
    zero();
}
Decal::Decal() { zero(); }
void Decal::setShader() {
    REPD(mode, 3)
    REPD(fullscreen, 2) { // #MaterialTextureLayout
        if (_material && _material->base_0) {
            Int layout = (mode == 0 && _material->base_2);
            Bool normal = (mode == 0 && _material->base_1);
            Shader *&src = Sh.Decal[mode][fullscreen][layout][normal];
            if (!src) {
                src = ShaderFiles("Effects 3D")->get(S8 + "Decal" + mode + fullscreen + (layout + 1) + normal);
                Sh.DecalParams = GetShaderParam("DecalParams");
            }
            _shader[mode][fullscreen] = src;
        } else
            _shader[mode][fullscreen] = null;
    }
}
Decal &Decal::material(C MaterialPtr &material) {
    if (T._material != material) {
        T._material = material;
        setShader();
    }
    return T;
}
Decal &Decal::setMatrix(C Matrix &object_world_matrix, C Matrix &decal_world_matrix) {
    object_world_matrix.inverse(matrix);
    decal_world_matrix.mul(matrix, matrix); // 'matrix'=local matrix
    return T;
}
/******************************************************************************/
void Decal::drawStatic() C {
    if (_shader[0][0] && Renderer.canReadDepth()) {
        BallM ball((matrix.x + matrix.y + matrix.z).length(), matrix.pos);
        if (Frustum(ball)) {
            Bool inside = Cuts(CamMatrix.pos, ball.extend(Frustum.view_quad_max_dist));
            Int mode = GetMode();
            if (Shader *shader = T._shader[mode][inside]) {
                SetOneMatrix(matrix);

                _material->setBlend();
                Sh.DecalParams->set(Vec2(OpaqueFracMul(), OpaqueFracAdd()));
                Sh.Color[0]->set(color);

                if (terrain_only)
                    D.stencil(STENCIL_TERRAIN_TEST);

                Renderer.needDepthRead();
                D.alpha(Alpha[mode]);
                if (inside) {
                    shader->draw();
                } else {
                    D.cull(true);
                    D.depthOnWrite(true, false);
                    shader->begin();
                    MshrBox.set().draw();
                }

                if (terrain_only)
                    D.stencil(STENCIL_NONE);
            }
        }
    }
}
void Decal::drawAnimated(C Matrix &object_world_matrix) C {
    if (_shader[0][0] && Renderer.canReadDepth()) {
        Matrix m;
        matrix.mul(object_world_matrix, m);
        BallM ball((m.x + m.y + m.z).length(), m.pos);
        if (Frustum(ball)) {
            Bool inside = Cuts(CamMatrix.pos, ball.extend(Frustum.view_quad_max_dist));
            Int mode = GetMode();
            if (Shader *shader = T._shader[mode][inside]) {
                SetOneMatrix(m);

                _material->setBlend();
                Sh.DecalParams->set(Vec2(OpaqueFracMul(), OpaqueFracAdd()));
                Sh.Color[0]->set(color);

                if (terrain_only)
                    D.stencil(STENCIL_TERRAIN_TEST);

                Renderer.needDepthRead();
                D.alpha(Alpha[mode]);
                if (inside) {
                    shader->draw();
                } else {
                    D.cull(true);
                    D.depthOnWrite(true, false);
                    shader->begin();
                    MshrBox.set().draw();
                }

                if (terrain_only)
                    D.stencil(STENCIL_NONE);
            }
        }
    }
}
/******************************************************************************/
Bool Decal::save(File &f) C {
    f.cmpUIntV(0); // version
    f << terrain_only << color << matrix;
    f.putAsset(_material.id());
    return f.ok();
}
Bool Decal::load(File &f) {
    switch (f.decUIntV()) // version
    {
    case 0: {
        f >> terrain_only >> color >> matrix;
        _material = f.getAssetID();
        setShader();
        if (f.ok())
            return true;
    } break;
    }
    del();
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
