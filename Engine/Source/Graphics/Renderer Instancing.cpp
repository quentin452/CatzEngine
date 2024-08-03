/******************************************************************************/
#include "stdafx.h"
/******************************************************************************

   Rendering is sorted by:
      -Shaders
      -Materials
      -Meshes
      -Instances

   Most Materials will use only 1 Shader (only few cases will use more)
   Because of that, Materials have one link ('MaterialShader') to the Shader already included in Material object (to reduce overhead).
   In cases where Material needs another shader, then that link can point to next link in 'MaterialShaders' container (this is a linked list, where links can continuously point to next links in that container)
   With this mechanism we have unique 'MaterialShader' objects (Material -> Shader), however during rendering we need to have links the other way around (Shader -> Material).
   Because first we set Shaders, then iterate all Materials belonging to this Shader.
   All draw calls will store 'ShaderDraw' first, and then that object will have a linked list of all Materials belonging to this Shader ('ShaderMaterial' objects).
   Then 'ShaderMaterial' has a linked list of all meshes 'ShaderMaterialMesh', which then have instances '*ShaderMaterialMeshInstance'.

   'ShaderMaterial' needs to be separate from 'MaterialShader', because:
      'MaterialShader' is a linked list of   shaders for 1 material
      'ShaderMaterial' is a linked list of materials for 1 shader

   However all instances are managed per 'MeshPart.Variation', which is not perfect, because for example, 'MeshPart' can have multiple variations, and some of them may have the same 'Material'.
   Even though material is the same, they will not be instanced together. We would have to manage some sort of "unique variation" list in the 'MeshPart', which would complicate things.

TODO: most of (shader_param_changes, stencil) instance parameters are never used, can we do something about it? to remove overhead and memory usage
TODO: on stereo rendering some pixels are wasted, use a stencil mask, see - http://media.steampowered.com/apps/valve/2015/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
/******************************************************************************/
namespace EE {
/******************************************************************************/
void SetDrawMask(UInt draw_mask) { Renderer._mesh_draw_mask = draw_mask; }
void SetVariation(Int variation) { Renderer._mesh_variation_1 = variation - 1; } // offset it over here, instead of every time in mesh draw
void SetStencilValue(Bool terrain) { Renderer._mesh_stencil_value = (terrain ? STENCIL_REF_TERRAIN : STENCIL_REF_ZERO); }
void SetStencilMode(Bool terrain_only) { Renderer._mesh_stencil_mode = (terrain_only ? STENCIL_TERRAIN_TEST : STENCIL_NONE); }
void SetBlendAlpha(ALPHA_MODE alpha) { Renderer._mesh_blend_alpha = alpha; }
void SetEarlyZ(Bool on) { Renderer._mesh_early_z = on; }
void SetBehindBias(Flt distance) { Sh.BehindBias->setConditional(distance); }
/******************************************************************************/
static Bool CreateHighlight(Memc<ShaderParamChange> &data, C Color &color, Ptr user) {
    PROFILE_START("CatzEngine::CreateHighlight(Memc<ShaderParamChange> &data, C Color &color, Ptr user)")
    data.New().set(Renderer.highlight).set(color);
    PROFILE_STOP("CatzEngine::CreateHighlight(Memc<ShaderParamChange> &data, C Color &color, Ptr user)")
    return true;
}

static Map<Color, Memc<ShaderParamChange>> Highlights(Compare, CreateHighlight);
static Memc<ShaderParamChange> *Highlight;

void SetHighlight() {
    PROFILE_START("CatzEngine::SetHighlight()")
    if (Highlight) {
        UnlinkShaderParamChanges(*Highlight);
        Highlight = null;
    }
    PROFILE_STOP("CatzEngine::SetHighlight()")
}
void SetHighlight(C Color &color) {
    PROFILE_START("CatzEngine::SetHighlight(C Color &color)")
    auto highlight = (color.any() ? Highlights(color) : null);
    if (Highlight != highlight) {
        if (Highlight)
            UnlinkShaderParamChanges(*Highlight);
        if (Highlight = highlight)
            LinkShaderParamChanges(*Highlight);
    }
    PROFILE_STOP("CatzEngine::SetHighlight(C Color &color)")
}
/******************************************************************************
// SHADER PARAM CHANGES
/******************************************************************************/
void LinkShaderParamChanges(const_mem_addr C Memc<ShaderParamChange> &changes) {
    PROFILE_START("CatzEngine::LinkShaderParamChanges(const_mem_addr C Memc<ShaderParamChange> &changes)")
    if (!Renderer._shader_param_changes)
        Renderer._shader_param_changes = &changes;
    else
        Exit("Trying to 'LinkShaderParamChanges' while there are other changes that weren't unlinked.");
    PROFILE_STOP("CatzEngine::LinkShaderParamChanges(const_mem_addr C Memc<ShaderParamChange> &changes)")
}
void UnlinkShaderParamChanges(const_mem_addr C Memc<ShaderParamChange> &changes) {
    PROFILE_START("CatzEngine::UnlinkShaderParamChanges(const_mem_addr C Memc<ShaderParamChange> &changes)")
    if (Renderer._shader_param_changes == &changes)
        Renderer._shader_param_changes = null;
    else
        Exit("Trying to 'UnlinkShaderParamChanges' that weren't linked.");
    PROFILE_STOP("CatzEngine::UnlinkShaderParamChanges(const_mem_addr C Memc<ShaderParamChange> &changes)")
}
struct ShaderParamRestore {
    ShaderParam *param;
    Vec4 value;

    void restore() C { param->setSafe(value); } // have to use 'setSafe' because the 'ShaderParam' can be smaller than what we're setting

    void operator=(ShaderParam *param) {
        T.param = param;
        T.value = *(Vec4 *)param->_data;
    } // set param and remember its current value
};
static Memc<ShaderParamRestore> RestoreChanges;
static C Memc<ShaderParamChange> *LastChanges;

Bool SetShaderParamChanges(C Memc<ShaderParamChange> *changes) {
    PROFILE_START("CatzEngine::SetShaderParamChanges(C Memc<ShaderParamChange> *changes)")
    if (LastChanges != changes) {
        // restore previous changes if any
        REPAO(RestoreChanges).restore(); // restore in backward order
        RestoreChanges.clear();

        // apply new changes
        if (LastChanges = changes)
            REPA(*LastChanges) {
                C ShaderParamChange &change = (*LastChanges)[i];
                ShaderParam *param = change.param;
                // if(param) skip for performance reasons, expect to be valid
                { // apply change, have to use '#setSafeConditional' because the 'ShaderParam' can be smaller than what we're setting
                    C auto &value = change.value;
                    auto size = Min(param->_gpu_data_size, SIZEU(value));
                    if (!EqualMemFast(param->_data, &value, size)) {
                        if (change.restore)
                            RestoreChanges.add(param); // add for restoration !! BEFORE COPY !!
                        CopyFast(param->_data, &value, size);
                        param->setChanged();
                    }
                }
            }
        PROFILE_STOP("CatzEngine::SetShaderParamChanges(C Memc<ShaderParamChange> *changes)")
        return true;
    }
    PROFILE_STOP("CatzEngine::SetShaderParamChanges(C Memc<ShaderParamChange> *changes)")
    return false;
}
/******************************************************************************/
void ShaderMaterial::unlink() {
    PROFILE_START("CatzEngine::ShaderMaterial::unlink()")
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
    if (..)
        material->unlink(); // need to detect if this is 'material' or 'umm', and we can't check smm->mesh->_umm because this could have already been changed with MeshPart.material call, or this could be ShaderMaterial for shadow which ignores umm
    else
        umm->unlink();
#else
    if (ShaderMaterialMesh *smm = ShaderMaterialMeshes.addr(first_shader_material_mesh)) {
        if (C Material *material = smm->Variation().material())
            material->unlink();
        if (UniqueMultiMaterialData *umm = smm->mesh->_umm)
            umm->unlink();
    }
#endif
    PROFILE_STOP("CatzEngine::ShaderMaterial::unlink()")
}
/******************************************************************************/
Memc<Material::MaterialShader> MaterialShaders;
Memc<ShaderDraw> ShaderDraws, MultiMaterialShaderDraws;
Memc<ShaderMaterial> ShaderMaterials;
Memc<ShaderMaterialMesh> ShaderMaterialMeshes;
Memc<OpaqueShaderMaterialMeshInstance> OpaqueShaderMaterialMeshInstances;
Memc<ShadowShaderMaterialMeshInstance> ShadowShaderMaterialMeshInstances;
Memc<EmissiveInstance> EmissiveInstances;

Memc<SkeletonShader> SkeletonShaders;
Memc<SkeletonBlendShader> SkeletonBlendShaders;
Memc<SkeletonShaderMaterial> SkeletonShaderMaterials, SkeletonBlendShaderMaterials;
Memc<SkeletonShaderMaterialMeshInstance> SkeletonShadowShaderMaterialMeshInstances;
Memc<SkeletonOpaqueShaderMaterialMeshInstance> SkeletonOpaqueShaderMaterialMeshInstances;
Memc<SkeletonBlendShaderMaterialMeshInstance> SkeletonBlendShaderMaterialMeshInstances;
SkeletonInstances SkeletonOpaqueInstances, SkeletonShadowInstances;
Memc<SkeletonEmissiveInstance> SkeletonEmissiveInstances;

BlendInstancesClass BlendInstances;
// ClothInstances      OpaqueClothInstances, ShadowClothInstances;

GameObjects PaletteObjects, Palette1Objects, OverlayObjects, OpaqueObjects, EmissiveObjects, OutlineObjects, BehindObjects;
GameAreas PaletteAreas, Palette1Areas;
/******************************************************************************/
#if 0 // this doesn't work, because we need to adjust ViewMatrix and not ProjMatrix (for example Shadows in Forward Renderer will not work, because 'Data.pos' in shaders depends on ViewMatrix only, it affects shadows, and reflections, etc.)
// precomputed view matrixes were generated for the first eye, so for next eye we need to adjust the projection matrix (instead of adjusting camera and recalculating view matrixes which would be slower)
static void BeginPrecomputedViewMatrix() {if(/*Renderer._stereo && */Renderer._eye!=0)SetProjMatrix(ProjMatrixEyeOffset[Renderer._eye], -D.eyeDistance());}
static void   EndPrecomputedViewMatrix() {if(/*Renderer._stereo && */Renderer._eye!=0)SetProjMatrix(ProjMatrixEyeOffset[Renderer._eye]);}
struct EyeCache
{
   Int cur_eye, normal_eye, precomputed_eye;

   EyeCache() {cur_eye=normal_eye=Renderer._eye; precomputed_eye=0;}

   void setEye(Int eye) {if(cur_eye!=eye){cur_eye=eye; SetProjMatrix(ProjMatrixEyeOffset[Renderer._eye], eye ? 0 : -D.eyeDistance());}}

   INLINE void BeginPrecomputedViewMatrix() {setEye(precomputed_eye);}
   INLINE void   EndPrecomputedViewMatrix() {setEye(     normal_eye);}
};
static INLINE void SetViewOffset() {}
static INLINE void SetViewMatrix(C Matrix &view_matrix, Int i=0) {ViewMatrix[i]=view_matrix;} // !! Warning: this doesn't call 'setChanged' !!
#else
static INLINE void BeginPrecomputedViewMatrix() {}
static INLINE void EndPrecomputedViewMatrix() {}
struct EyeCache {
    INLINE void BeginPrecomputedViewMatrix() {}
    INLINE void EndPrecomputedViewMatrix() {}
};
static Flt ViewOffset;
static INLINE void SetViewOffset() { ViewOffset = (Renderer._eye ? -D.eyeDistance() : 0); }
static INLINE void SetViewMatrix(C Matrix &view_matrix, Int i = 0) {
    ViewMatrix[i] = view_matrix;
    ViewMatrix[i]._x += ViewOffset;
} // !! Warning: this doesn't call 'setChanged' !!
static INLINE void SetViewMatrixPrev(C Matrix &view_matrix, Int i = 0) {
    ViewMatrixPrev[i] = view_matrix;
    ViewMatrixPrev[i]._x += ViewOffset;
} // !! Warning: this doesn't call 'setChanged' !!
#endif
/******************************************************************************/
// EARLY Z
/******************************************************************************/
#if SUPPORT_EARLY_Z
Memc<EarlyZInstance> EarlyZInstances[2];
Bool HasEarlyZInstances() {
    return (EarlyZInstances[0].elms() + EarlyZInstances[1].elms()) > 0;
}
void DrawEarlyZInstances() {
    PROFILE_START("CatzEngine::DrawEarlyZInstances()")
    SetViewOffset();
    BeginPrecomputedViewMatrix();
    SetMatrixCount();
    DisableSkinning();
    Renderer._shader_early_z->start(); // this shader doesn't require any textures
    // TODO: this could be optimized to group in state changes (mesh->instance) and allow instancing #EarlyZInstancing
    FREPA(EarlyZInstances) {
        C Memc<EarlyZInstance> &instances = EarlyZInstances[i];
        if (instances.elms()) {
            D.cull(i);
            FREPA(instances) {
                C EarlyZInstance &instance = instances[i];
                SetViewMatrix(instance.view_matrix);
                Sh.ViewMatrix->setChanged();
                Renderer._shader_early_z->commit();
                instance.mesh->set().draw();
            }
        }
    }
    EndPrecomputedViewMatrix();
    PROFILE_STOP("CatzEngine::DrawEarlyZInstances()")
}
void ClearEarlyZInstances() {
    EarlyZInstances[0].clear();
    EarlyZInstances[1].clear();
}
#endif
/******************************************************************************/
// OPAQUE
/******************************************************************************/
static INLINE void DrawOpaqueInstances(Bool forward) // !! this function should be safe to call 2 times in a row for both eyes, so can't do any clearing/unlinking that would break things !!
{
    PROFILE_START("CatzEngine::DrawOpaqueInstances(Bool forward)")
    SetViewOffset();
    BeginPrecomputedViewMatrix();

    // opaque
    // SetMatrixCount(); not needed since we always call this before drawing
    DisableSkinning();
    FREPA(ShaderDraws) {
        ShaderDraw &shader_draw = ShaderDraws[i];
        shader_draw.unlink(); // have to unlink 'shader_draw' instead of 'shader' (because it can be FRST)
        Shader &shader = shader_draw.shader->getShader(forward);
        shader.start();
        for (ShaderMaterial *shader_material = &ShaderMaterials[shader_draw.first_shader_material];;) {
            ShaderMaterialMesh *shader_material_mesh = &ShaderMaterialMeshes[shader_material->first_shader_material_mesh];
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
            C Material &material = *shader_material->material;
#else
            C MeshPart::Variation *variation = &shader_material_mesh->Variation(); // have to access 'variation' here, because we need it to access 'material' since we're not storing it in the 'ShaderMaterial'
            C Material &material = variation->getMaterial();
#endif
            material.unlinkOpaque();
            material.setOpaque();
            D.cull(material.cull);
            shader.commitTex();

            for (;;) {
                C MeshPart &mesh = *shader_material_mesh->mesh;
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
                mesh.unlinkOpaque();
#else
                variation->unlinkOpaque();
#endif
                C MeshRender &render = mesh.render.set();
                C Bool instancing_mesh = !(render.flag() & VTX_SKIN); // can do instancing only if mesh doesn't have skinning (otherwise a skinned shader is set which does not use instancing)

                for (OpaqueShaderMaterialMeshInstance *instance = &OpaqueShaderMaterialMeshInstances[shader_material_mesh->first_instance];;) {
                    SetViewMatrix(instance->view_matrix.cur);
                    Sh.ViewMatrix->setChanged();
                    if (!forward) {
                        SetViewMatrixPrev(instance->view_matrix.prev);
                        Sh.ViewMatrixPrev->setChanged();
                    }
                    SetShaderParamChanges(instance->shader_param_changes);
                    D.stencilRef(instance->stencil_value);
                    Int instances = 1;
                    if (instancing_mesh)
                        for (; instance->next_instance >= 0;) {
                            OpaqueShaderMaterialMeshInstance &next = OpaqueShaderMaterialMeshInstances[instance->next_instance];
                            if (next.shader_param_changes == instance->shader_param_changes && next.stencil_value == instance->stencil_value) {
                                instance = &next;
                                SetViewMatrix(next.view_matrix.cur, instances);
                                if (!forward)
                                    SetViewMatrixPrev(next.view_matrix.prev, instances);
                                if (++instances >= MAX_MATRIX)
                                    break;
                            } else
                                break;
                        }
                    SetMatrixCount(instances);
                    shader.commit();
                    if (instances > 1)
                        render.drawInstanced(instances);
                    else
                        render.draw();

                    if (instance->next_instance < 0)
                        break;
                    instance = &OpaqueShaderMaterialMeshInstances[instance->next_instance];
                }
                if (shader_material_mesh->next_shader_material_mesh < 0)
                    break;
                shader_material_mesh = &ShaderMaterialMeshes[shader_material_mesh->next_shader_material_mesh];
#if !SUPPORT_MATERIAL_CHANGE_IN_RENDERING
                variation = &shader_material_mesh->Variation();
#endif
            }

            SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
            if (shader_material->next_shader_material < 0)
                break;
            shader_material = &ShaderMaterials[shader_material->next_shader_material];
        }
    }
    D.stencilRef(0);
    // can't clear 'ShaderDraws' because we might need it for second eye rendering

    EndPrecomputedViewMatrix();

    // skeleton
    EnableSkinning();
    FREPA(SkeletonOpaqueInstances) {
        SkeletonInstance &skel = SkeletonOpaqueInstances[i];
        skel.unlinkOpaque();
        skel.anim_skel->setMatrix();
        for (SkeletonShader *skel_shader = &skel.skel_shader;;) {
            Shader &shader = skel_shader->shader->getShader(forward);
            shader.start();
            for (SkeletonShaderMaterial *skel_shader_material = &skel_shader->material;;) {
                C Material &material = *skel_shader_material->material;
                material.setOpaque();
                D.cull(material.cull);
                shader.commitTex();
                Bool shader_params_changed = true;
                for (SkeletonOpaqueShaderMaterialMeshInstance *instance = &SkeletonOpaqueShaderMaterialMeshInstances[skel_shader_material->first_mesh_instance];;) {
                    shader_params_changed |= SetShaderParamChanges(instance->shader_param_changes);
                    if (shader_params_changed) {
                        shader_params_changed = false;
                        shader.commit();
                    }
                    instance->mesh->set().draw();
                    if (instance->next_instance < 0)
                        break;
                    instance = &SkeletonOpaqueShaderMaterialMeshInstances[instance->next_instance];
                }
                SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
                if (skel_shader_material->next_skeleton_shader_material < 0)
                    break;
                skel_shader_material = &SkeletonShaderMaterials[skel_shader_material->next_skeleton_shader_material];
            }
            if (skel_shader->next_skeleton_shader < 0)
                break;
            skel_shader = &SkeletonShaders[skel_shader->next_skeleton_shader];
        }
    }
    // can't clear 'SkeletonOpaqueInstances' because we might need it for second eye rendering

    /*// cloth
    if(OpaqueClothInstances.elms())
    {
       SetOneMatrixAndPrev(); // current cloth shaders might use velocity
       {
          SetDefaultVAO(); D.vf(VI._vf3D_cloth.vf); // OpenGL requires setting 1)VAO 2)VB+IB 3)VF
          FREPA(OpaqueClothInstances)
          {
             ClothInstance &ci=OpaqueClothInstances[i];
             Shader   &shader  = ci.shader  ->getShader(forward);
           C Material &material=*ci.material; material.setOpaque(); D.cull(material.cull);
             shader.begin(); ci.cloth->_drawPhysical();
          }
       }
    }*/

    BeginPrecomputedViewMatrix();

    // multi material (at last stage, because these are usually heightmaps on the bottom)
#define MULTI_MATERIAL_INSTANCING 0 // these are heightmaps, and we never need instancing for them because they're drawn only 1 time each
    SetMatrixCount();
    DisableSkinning();
    FREPA(MultiMaterialShaderDraws) {
        ShaderDraw &shader_draw = MultiMaterialShaderDraws[i];
        shader_draw.unlink(); // have to unlink 'shader_draw' instead of 'shader' (because it can be FRST)
        Shader &shader = shader_draw.shader->getShader(forward);
        shader.start();
        for (ShaderMaterial *shader_material = &ShaderMaterials[shader_draw.first_shader_material];;) {
            ShaderMaterialMesh *shader_material_mesh = &ShaderMaterialMeshes[shader_material->first_shader_material_mesh];
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
            UniqueMultiMaterialData &umm = *shader_material->umm;
            umm.unlink();
#else
            UniqueMultiMaterialData &umm = *shader_material_mesh->mesh->_umm;
            umm.unlink();
#endif
            C UniqueMultiMaterialKey &umm_key = UniqueMultiMaterialMap.dataInMapToKey(umm);
            Bool cull = true;
            if (C Material *m = umm_key.m[0]) {
                m->setMulti(0);
                cull &= m->cull;
            } else {
                MaterialDefault.setMulti(0);
                cull &= MaterialDefault.cull;
            }
            if (C Material *m = umm_key.m[1]) {
                m->setMulti(1);
                cull &= m->cull;
            }
            if (C Material *m = umm_key.m[2]) {
                m->setMulti(2);
                cull &= m->cull;
            }
            if (C Material *m = umm_key.m[3]) {
                m->setMulti(3);
                cull &= m->cull;
            }
            D.cull(cull);
            shader.commitTex();

            for (;;) {
                C MeshPart &mesh = *shader_material_mesh->mesh;
                C MeshRender &render = mesh.render.set();
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
                mesh.unlinkOpaque();
#else
                C MeshPart::Variation &variation = shader_material_mesh->Variation();
                variation.unlinkOpaque();
#endif
#if MULTI_MATERIAL_INSTANCING
                C Bool instancing_mesh = !(render.flag() & VTX_SKIN); // can do instancing only if mesh doesn't have skinning (otherwise a skinned shader is set which does not use instancing)
#endif

                for (OpaqueShaderMaterialMeshInstance *instance = &OpaqueShaderMaterialMeshInstances[shader_material_mesh->first_instance];;) {
                    SetViewMatrix(instance->view_matrix.cur);
                    Sh.ViewMatrix->setChanged();
                    if (!forward) {
                        SetViewMatrixPrev(instance->view_matrix.prev);
                        Sh.ViewMatrixPrev->setChanged();
                    }
                    SetShaderParamChanges(instance->shader_param_changes);
                    D.stencilRef(instance->stencil_value);
                    Int instances = 1;
#if MULTI_MATERIAL_INSTANCING
                    if (instancing_mesh)
                        for (; instance->next_instance >= 0;) {
                            OpaqueShaderMaterialMeshInstance &next = OpaqueShaderMaterialMeshInstances[instance->next_instance];
                            if (next.shader_param_changes == instance->shader_param_changes && next.stencil_value == instance->stencil_value) {
                                instance = &next;
                                SetViewMatrix(next.view_matrix.cur, instances);
                                if (!forward)
                                    SetViewMatrixPrev(next.view_matrix.prev, instances);
                                if (++instances >= MAX_MATRIX)
                                    break;
                            } else
                                break;
                        }
                    SetMatrixCount(instances);
#endif
                    shader.commit();
                    if (instances > 1)
                        render.drawInstanced(instances);
                    else
                        render.draw();

                    if (instance->next_instance < 0)
                        break;
                    instance = &OpaqueShaderMaterialMeshInstances[instance->next_instance];
                }

                if (shader_material_mesh->next_shader_material_mesh < 0)
                    break;
                shader_material_mesh = &ShaderMaterialMeshes[shader_material_mesh->next_shader_material_mesh];
            }

            SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
            if (shader_material->next_shader_material < 0)
                break;
            shader_material = &ShaderMaterials[shader_material->next_shader_material];
        }
    }
    D.stencilRef(0);
    // can't clear 'MultiMaterialShaderDraws' because we might need it for second eye rendering

    // finish
    EndPrecomputedViewMatrix();

    FREPAO(OpaqueObjects)->drawOpaque();
    PROFILE_STOP("CatzEngine::DrawOpaqueInstances(Bool forward)")
}
void ClearOpaqueInstances() {
    MaterialShaders.clear();
    ShaderDraws.clear();
    MultiMaterialShaderDraws.clear();
    ShaderMaterials.clear();
    ShaderMaterialMeshes.clear();
    OpaqueShaderMaterialMeshInstances.clear();

    SkeletonShaders.clear();
    SkeletonShaderMaterials.clear();
    SkeletonOpaqueShaderMaterialMeshInstances.clear();
    SkeletonOpaqueInstances.clear();

    // OpaqueClothInstances.clear();
    OpaqueObjects.clear();
}
void DrawOpaqueInstances() {
    PROFILE_START("CatzEngine::DrawOpaqueInstances()")
    if (Renderer._cur_type != RT_FORWARD)
        DrawOpaqueInstances(false);
    else
        DrawOpaqueInstances(true);
    PROFILE_STOP("CatzEngine::DrawOpaqueInstances()")
}
/******************************************************************************/
// AMBIENT
/******************************************************************************/
static Int Compare(C EmissiveInstance &a, C EmissiveInstance &b) {
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
    if (Int c = ComparePtr(a.shader, b.shader))
        return c;
    if (Int c = ComparePtr(a.material, b.material))
        return c;
#else
    if (Int c = ComparePtr(a.variation->shader[RM_EMISSIVE], b.variation->shader[RM_EMISSIVE]))
        return c;
    if (Int c = ComparePtr(a.variation->material(), b.variation->material()))
        return c;
#endif
    if (Int c = ComparePtr(a.mesh, b.mesh))
        return c;
    return 0;
}
static Int Compare(C SkeletonEmissiveInstance &a, C SkeletonEmissiveInstance &b) {
    if (Int c = ComparePtr(a.anim_skel, b.anim_skel))
        return c;
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
    if (Int c = ComparePtr(a.shader, b.shader))
        return c;
    if (Int c = ComparePtr(a.material, b.material))
        return c;
#else
    if (Int c = ComparePtr(a.variation->shader[RM_EMISSIVE], b.variation->shader[RM_EMISSIVE]))
        return c;
    if (Int c = ComparePtr(a.variation->material(), b.variation->material()))
        return c;
#endif
    if (Int c = ComparePtr(a.mesh, b.mesh))
        return c;
    return 0;
}
void SortEmissiveInstances() {
#if SUPPORT_EMISSIVE
    EmissiveInstances.sort(Compare);
    SkeletonEmissiveInstances.sort(Compare);
#endif
}
void DrawEmissiveInstances() // !! this function should be safe to call 2 times in a row for both eyes, so can't do any clearing/unlinking that would break things !!
{                            // TODO: add support for #EmissiveInstancing
    PROFILE_START("CatzEngine::DrawEmissiveInstances()")
#if SUPPORT_EMISSIVE
    SetViewOffset();
    BeginPrecomputedViewMatrix();
    SetMatrixCount();
    DisableSkinning();
    FREPA(EmissiveInstances) {
        // TODO: this could be optimized to group in state changes (shader->material->mesh->instance) and allow instancing, this is already sorted, we would just need to detect changes between this and next instance
        C EmissiveInstance &instance = EmissiveInstances[i];
        C MeshPart &mesh = *instance.mesh;
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
        Shader &shader = *instance.shader;
        C Material &material = *instance.material;
#else
        C MeshPart::Variation &variation = instance.Variation();
        Shader &shader = *variation.shader[RM_EMISSIVE];
        C Material &material = variation.getMaterial();
#endif
        material.setEmissive();
        D.cull(material.cull);
        SetViewMatrix(instance.view_matrix);
        Sh.ViewMatrix->setChanged();
        SetShaderParamChanges(instance.shader_param_changes);
        shader.begin();
        mesh.render.set().draw();
        SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
    }

    EndPrecomputedViewMatrix();
    EnableSkinning();
    FREPA(SkeletonEmissiveInstances) {
        // TODO: this could be optimized to group in state changes (shader->material->mesh->instance), this is already sorted, we would just need to detect changes between this and next instance
        C SkeletonEmissiveInstance &instance = SkeletonEmissiveInstances[i];
        C MeshPart &mesh = *instance.mesh;
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
        Shader &shader = *instance.shader;
        C Material &material = *instance.material;
#else
        C MeshPart::Variation &variation = instance.Variation();
        Shader &shader = *variation.shader[RM_EMISSIVE];
        C Material &material = variation.getMaterial();
#endif
        material.setEmissive();
        D.cull(material.cull);
        instance.anim_skel->setMatrix();
        SetShaderParamChanges(instance.shader_param_changes);
        shader.begin();
        mesh.render.set().draw();
        SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
    }
#endif

    FREPAO(EmissiveObjects)->drawEmissive();
    PROFILE_STOP("CatzEngine::DrawEmissiveInstances()")
}
void ClearEmissiveInstances() {
    EmissiveInstances.clear();
    SkeletonEmissiveInstances.clear();
    EmissiveObjects.clear();
}
/******************************************************************************/
// SHADOW
/******************************************************************************/
static Int MaterialShadersNum, ShaderDrawsNum, ShaderMaterialsNum, ShaderMaterialMeshesNum, SkeletonShadersNum, SkeletonShaderMaterialsNum;
void PrepareShadowInstances() {
    PROFILE_START("CatzEngine::PrepareShadowInstances()")
    // remember current number of elements in these containers (they may already have data in Forward renderer)
    MaterialShadersNum = MaterialShaders.elms();
    ShaderDrawsNum = ShaderDraws.elms();
    ShaderMaterialsNum = ShaderMaterials.elms();
    ShaderMaterialMeshesNum = ShaderMaterialMeshes.elms();

    SkeletonShadersNum = SkeletonShaders.elms();
    SkeletonShaderMaterialsNum = SkeletonShaderMaterials.elms();
    PROFILE_STOP("CatzEngine::PrepareShadowInstances()")
}
void DrawShadowInstances() // this is called only 1 time and not for each eye
{
    PROFILE_START("CatzEngine::DrawShadowInstances()")
    // this doesn't require 'ViewOffset' and 'BeginPrecomputedViewMatrix' because shadows are drawn only 1 time, and not for each eye

    // opaque
    PROFILE_START("CatzEngine::DrawShadowInstances()Opaque Section")
    SetMatrixCount();
    DisableSkinning();
    for (Int i = ShaderDrawsNum; i < ShaderDraws.elms(); i++) {
        PROFILE_START("CatzEngine::DrawShadowInstances()Shader Draw Loop")
        ShaderDraw &shader_draw = ShaderDraws[i];
        shader_draw.unlink();
        Shader &shader = shader_draw.shader->getShader(false);
        shader.start();
#if !SUPPORT_MATERIAL_CHANGE_IN_RENDERING
        Bool reuse_default_material = ReuseDefaultMaterialForNonSkinnedShadowShader(&shader);
#endif
        for (ShaderMaterial *shader_material = &ShaderMaterials[shader_draw.first_shader_material];;) {
            PROFILE_START("CatzEngine::DrawShadowInstances()Shader Material Loop")
            ShaderMaterialMesh *shader_material_mesh = &ShaderMaterialMeshes[shader_material->first_shader_material_mesh];
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
            C Material &material = *shader_material->material;
#else
            C MeshPart::Variation *variation = &shader_material_mesh->Variation(); // have to access 'variation' here, because we need it to access 'material' since we're not storing it in the 'ShaderMaterial'
            C Material &material = variation->getShadowMaterial(reuse_default_material);
#endif
            material.unlinkShadow();
            material.setShadow();
            D.cull(material.cull);
            shader.commitTex();

            for (;;) {
                PROFILE_START("CatzEngine::DrawShadowInstances()Mesh Loop")
                C MeshPart &mesh = *shader_material_mesh->mesh;
                C MeshRender &render = mesh.render.set();
                C Bool instancing_mesh = !(render.flag() & VTX_SKIN); // can do instancing only if mesh doesn't have skinning (otherwise a skinned shader is set which does not use instancing)
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
                mesh.unlinkShadow();
#else
                variation->unlinkShadow();
#endif

                for (ShadowShaderMaterialMeshInstance *instance = &ShadowShaderMaterialMeshInstances[shader_material_mesh->first_instance];;) {
                    PROFILE_START("CatzEngine::DrawShadowInstances()Instance Loop")
                    ViewMatrix[0] = instance->view_matrix;
                    Sh.ViewMatrix->setChanged(); // this doesn't need 'ViewOffset'
                    SetShaderParamChanges(instance->shader_param_changes);
                    Int instances = 1;
                    if (instancing_mesh)
                        for (; instance->next_instance >= 0;) {
                            PROFILE_START("CatzEngine::DrawShadowInstances()Instance Loop - Next Instance")
                            ShadowShaderMaterialMeshInstance &next = ShadowShaderMaterialMeshInstances[instance->next_instance];
                            if (next.shader_param_changes == instance->shader_param_changes) {
                                instance = &next;
                                ViewMatrix[instances] = next.view_matrix; // this doesn't need 'ViewOffset'
                                if (++instances >= MAX_MATRIX)
                                    break;
                            } else
                                break;
                            PROFILE_STOP("CatzEngine::DrawShadowInstances()Instance Loop - Next Instance")
                        }
                    SetMatrixCount(instances);
                    shader.commit();
                    if (instances > 1)
                        render.drawInstanced(instances);
                    else
                        render.draw();

                    if (instance->next_instance < 0)
                        break;
                    instance = &ShadowShaderMaterialMeshInstances[instance->next_instance];
                    PROFILE_STOP("CatzEngine::DrawShadowInstances()Instance Loop")
                }

                if (shader_material_mesh->next_shader_material_mesh < 0)
                    break;
                shader_material_mesh = &ShaderMaterialMeshes[shader_material_mesh->next_shader_material_mesh];
#if !SUPPORT_MATERIAL_CHANGE_IN_RENDERING
                variation = &shader_material_mesh->Variation();
#endif
                PROFILE_STOP("CatzEngine::DrawShadowInstances()Mesh Loop")
            }

            SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
            if (shader_material->next_shader_material < 0)
                break;
            shader_material = &ShaderMaterials[shader_material->next_shader_material];
            PROFILE_STOP("CatzEngine::DrawShadowInstances()Shader Material Loop")
        }
        PROFILE_STOP("CatzEngine::DrawShadowInstances()Shader Draw Loop")
    }
    PROFILE_STOP("CatzEngine::DrawShadowInstances()Opaque Section")

    // skeleton
    PROFILE_START("CatzEngine::DrawShadowInstances()Skeleton Section")
    EnableSkinning();
    FREPA(SkeletonShadowInstances) {
        PROFILE_START("CatzEngine::DrawShadowInstances()Skeleton Instance Loop")
        SkeletonInstance &skel = SkeletonShadowInstances[i];
        skel.unlinkShadow();
        skel.anim_skel->setMatrix();
        for (SkeletonShader *skel_shader = &skel.skel_shader;;) {
            PROFILE_START("CatzEngine::DrawShadowInstances()Skeleton Shader Loop")
            Shader &shader = skel_shader->shader->getShader(false);
            shader.start();
            for (SkeletonShaderMaterial *skel_shader_material = &skel_shader->material;;) {
                PROFILE_START("CatzEngine::DrawShadowInstances()Skeleton Shader Material Loop")
                C Material &material = *skel_shader_material->material;
                material.setShadow();
                D.cull(material.cull);
                shader.commitTex();
                Bool shader_params_changed = true;
                for (SkeletonShaderMaterialMeshInstance *instance = &SkeletonShadowShaderMaterialMeshInstances[skel_shader_material->first_mesh_instance];;) {
                    PROFILE_START("CatzEngine::DrawShadowInstances()Skeleton Instance Loop")
                    shader_params_changed |= SetShaderParamChanges(instance->shader_param_changes);
                    if (shader_params_changed) {
                        shader_params_changed = false;
                        shader.commit();
                    }
                    instance->mesh->set().draw();
                    if (instance->next_instance < 0)
                        break;
                    instance = &SkeletonShadowShaderMaterialMeshInstances[instance->next_instance];
                    PROFILE_STOP("CatzEngine::DrawShadowInstances()Skeleton Instance Loop")
                }
                SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
                if (skel_shader_material->next_skeleton_shader_material < 0)
                    break;
                skel_shader_material = &SkeletonShaderMaterials[skel_shader_material->next_skeleton_shader_material];
                PROFILE_STOP("CatzEngine::DrawShadowInstances()Skeleton Shader Material Loop")
            }
            if (skel_shader->next_skeleton_shader < 0)
                break;
            skel_shader = &SkeletonShaders[skel_shader->next_skeleton_shader];
            PROFILE_STOP("CatzEngine::DrawShadowInstances()Skeleton Shader Loop")
        }
        PROFILE_STOP("CatzEngine::DrawShadowInstances()Skeleton Instance Loop")
    }
    PROFILE_STOP("CatzEngine::DrawShadowInstances()Skeleton Section")

    // we can't just clear all containers, because in Forward renderer there's still data needed for opaque shaders which are processed after shadows, so restore to what we had before drawing shadows
    MaterialShaders.setNum(MaterialShadersNum);
    ShaderDraws.setNum(ShaderDrawsNum);
    ShaderMaterials.setNum(ShaderMaterialsNum);
    ShaderMaterialMeshes.setNum(ShaderMaterialMeshesNum);
    ShadowShaderMaterialMeshInstances.clear();

    SkeletonShaders.setNum(SkeletonShadersNum);
    SkeletonShaderMaterials.setNum(SkeletonShaderMaterialsNum);
    SkeletonShadowShaderMaterialMeshInstances.clear();
    SkeletonShadowInstances.clear();

    /*// physical
    if(ShadowClothInstances.elms())
    {
       SetOneMatrixAndPrev(); // current cloth shaders might use velocity

       // cloth
       {
          SetDefaultVAO(); D.vf(VI._vf3D_cloth.vf); // OpenGL requires setting 1)VAO 2)VB+IB 3)VF
          FREPA(ShadowClothInstances)
          {
             ClothInstance &ci=ShadowClothInstances[i];
             Shader   &shader  = ci.shader  ->asShader();
           C Material &material=*ci.material; material.setShadow(); D.cull(material.cull);
             shader.begin(); ci.cloth->_drawPhysical();
          }
          ShadowClothInstances.clear();
       }
    }*/
    PROFILE_STOP("CatzEngine::DrawShadowInstances()")
}

/******************************************************************************/
// BLEND
/******************************************************************************/
Int Compare(C BlendInstance &a, C BlendInstance &b) {
    if (Int z = Compare(a.z, b.z))
        return z;
    // compare other values in case 'z' is the same (which means the same object draws many parts with the same matrix, which may cause flickering)
    // compare by shader, material, mesh, because that's the order of rendering
    if (Int c = ComparePtr(a.stat.shader, b.stat.shader))
        return c;
    if (Int c = ComparePtr(a.stat.material, b.stat.material))
        return c;
    if (Int c = ComparePtr(a.stat.mesh, b.stat.mesh))
        return c;
    return 0;
}
void ClearBlendInstances() {
    BlendInstances.clear();
    SkeletonBlendShaders.clear();
    SkeletonBlendShaderMaterials.clear();
    SkeletonBlendShaderMaterialMeshInstances.clear();
}
void DrawBlendInstances() // !! this function should be safe to call 2 times in a row for both eyes, so can't do any clearing/unlinking that would break things !!
{
    PROFILE_START("CatzEngine::DrawBlendInstances()")
    SetViewOffset();
    EyeCache ec;
    ALPHA_MODE alpha = (Renderer.fastCombine() ? ALPHA_BLEND : ALPHA_RENDER_BLEND_FACTOR);
    D.alpha(alpha);
    D.depth(true);
    REPA(BlendInstances) // go from back to start
    {
        BlendInstance *object = &BlendInstances[i];
        switch (object->type) {
        case BlendInstance::STATIC: {
            PROFILE_START("CatzEngine::DrawBlendInstances()1")
            ec.BeginPrecomputedViewMatrix();
            DisableSkinning();
            // this doesn't use velocities
            Shader &shader = *object->stat.shader;
            shader.start();
        opaque_shader:
            C Material &material = *object->stat.material;
            material.setBlend();
            D.cull(material.cull);
            D.depthWrite(material.hasDepthWrite());
            Renderer.needDepthTest();
            shader.commitTex(); // !! 'needDepthTest' after 'depthWrite' !!
        opaque_shader_material:
            C MeshRender &render = object->stat.mesh->render.set();
            D.stencil(object->stat.stencil_mode);
            SetShaderParamChanges(object->stat.shader_param_changes);
            SetViewMatrix(object->stat.view_matrix.cur);
            Sh.ViewMatrix->setChanged();
            SetViewMatrixPrev(object->stat.view_matrix.prev);
            Sh.ViewMatrixPrev->setChanged();
            C Bool instancing_mesh = !(render.flag() & VTX_SKIN); // can do instancing only if mesh doesn't have skinning (otherwise a skinned shader is set which does not use instancing)
            Int instances = 1;
            for (; i;) // if there's next one
            {
                BlendInstance &next = BlendInstances[i - 1]; // grab next
                if (next.stat.shader == &shader              // same shader
                                                             //&& next.type==BlendInstance::STATIC not needed since if the shader is the same, then type should be the same too, make debug assertion below just in case
                ) {
                    DEBUG_ASSERT(next.type == BlendInstance::STATIC, "Blend Instance type");
                    i--;                                 // we will process this instance
                    if (next.stat.material == &material) // same material
                    {
                        if (&next.stat.mesh->render == &render && next.stat.shader_param_changes == LastChanges && next.stat.stencil_mode == D._stencil) {
                            if (instancing_mesh) {
                                if (!InRange(instances, MAX_MATRIX)) // there's no room for this instance
                                {
                                    SetMatrixCount(instances);
                                    shader.commit();
                                    render.drawInstanced(instances); // draw what we have
                                    instances = 0;                   // reset counter
                                    Sh.ViewMatrix->setChanged();     // mark as modified
                                    Sh.ViewMatrixPrev->setChanged(); // mark as modified
                                }
                                SetViewMatrix(next.stat.view_matrix.cur, instances);
                                SetViewMatrixPrev(next.stat.view_matrix.prev, instances);
                                instances++;
                            } else {
                                SetMatrixCount();
                                shader.commit();
                                render.draw(); // draw what we have
                                SetViewMatrix(next.stat.view_matrix.cur);
                                Sh.ViewMatrix->setChanged();
                                SetViewMatrixPrev(next.stat.view_matrix.prev);
                                Sh.ViewMatrixPrev->setChanged();
                            }
                        } else // we have the same shader/material, but different mesh/params
                        {
                            SetMatrixCount(instances);
                            shader.commit();
                            if (instances > 1)
                                render.drawInstanced(instances);
                            else
                                render.draw(); // draw what we have
                            object = &next;
                            goto opaque_shader_material;
                        }
                    } else // we have the same shader, but different material/mesh/params
                    {
                        SetMatrixCount(instances);
                        shader.commit();
                        if (instances > 1)
                            render.drawInstanced(instances);
                        else
                            render.draw();       // draw what we have
                        SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
                        object = &next;
                        goto opaque_shader;
                    }
                } else
                    break;
            }
            SetMatrixCount(instances);
            shader.commit();
            if (instances > 1)
                render.drawInstanced(instances);
            else
                render.draw();
            SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
            PROFILE_STOP("CatzEngine::DrawBlendInstances()1")
        } break;
        case BlendInstance::STATIC_BLST: {
            PROFILE_START("CatzEngine::DrawBlendInstances()2")
            ec.BeginPrecomputedViewMatrix();
            DisableSkinning();
            Shader &shader = object->stat.shader->asBlendShader();
            shader.start();
        opaque_blst_shader:
            C Material &material = *object->stat.material;
            material.setBlend();
            D.cull(material.cull);
            D.depthWrite(material.hasDepthWrite());
            Renderer.needDepthTest();
            shader.commitTex(); // !! 'needDepthTest' after 'depthWrite' !!
        opaque_blst_shader_material:
            C MeshRender &render = object->stat.mesh->render.set();
            D.stencil(object->stat.stencil_mode);
            SetShaderParamChanges(object->stat.shader_param_changes);
            SetViewMatrix(object->stat.view_matrix.cur);
            Sh.ViewMatrix->setChanged();
            SetViewMatrixPrev(object->stat.view_matrix.prev);
            Sh.ViewMatrixPrev->setChanged();
            const Bool instancing_mesh = !(render.flag() & VTX_SKIN); // can do instancing only if mesh doesn't have skinning (otherwise a skinned shader is set which does not use instancing)
            Int instances = 1;
            for (; i;) // if there's next one
            {
                BlendInstance &next = BlendInstances[i - 1]; // grab next
                if (next.stat.blst == object->stat.blst      // same shader
                                                             //&& next.type==BlendInstance::STATIC_BLST not needed since if the shader is the same, then type should be the same too, make debug assertion below just in case
                ) {
                    DEBUG_ASSERT(next.type == BlendInstance::STATIC_BLST, "Blend Instance type");
                    i--;                                 // we will process this instance
                    if (next.stat.material == &material) // same material
                    {
                        if (&next.stat.mesh->render == &render && next.stat.shader_param_changes == LastChanges && next.stat.stencil_mode == D._stencil) {
                            if (instancing_mesh) {
                                if (!InRange(instances, MAX_MATRIX)) // there's no room for this instance
                                {
                                    SetMatrixCount(instances);
                                    shader.commit();
                                    render.drawInstanced(instances); // draw what we have
                                    instances = 0;                   // reset counter
                                    Sh.ViewMatrix->setChanged();     // mark as modified
                                    Sh.ViewMatrixPrev->setChanged(); // mark as modified
                                }
                                SetViewMatrix(next.stat.view_matrix.cur, instances);
                                SetViewMatrixPrev(next.stat.view_matrix.prev, instances);
                                instances++;
                            } else {
                                SetMatrixCount();
                                shader.commit();
                                render.draw(); // draw what we have
                                SetViewMatrix(next.stat.view_matrix.cur);
                                Sh.ViewMatrix->setChanged();
                                SetViewMatrixPrev(next.stat.view_matrix.prev);
                                Sh.ViewMatrixPrev->setChanged();
                            }
                        } else // we have the same shader/material, but different mesh/params
                        {
                            SetMatrixCount(instances);
                            shader.commit();
                            if (instances > 1)
                                render.drawInstanced(instances);
                            else
                                render.draw(); // draw what we have
                            object = &next;
                            goto opaque_blst_shader_material;
                        }
                    } else // we have the same shader, but different material/mesh/params
                    {
                        SetMatrixCount(instances);
                        shader.commit();
                        if (instances > 1)
                            render.drawInstanced(instances);
                        else
                            render.draw();       // draw what we have
                        SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
                        object = &next;
                        goto opaque_blst_shader;
                    }
                } else
                    break;
            }
            SetMatrixCount(instances);
            shader.commit();
            if (instances > 1)
                render.drawInstanced(instances);
            else
                render.draw();
            SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
            PROFILE_STOP("CatzEngine::DrawBlendInstances()2")
        } break;

        case BlendInstance::STATIC_FUR: {
            PROFILE_START("CatzEngine::DrawBlendInstances()3")
            ec.BeginPrecomputedViewMatrix();
            DisableSkinning();
            Flt scale = object->stat.view_matrix.cur.x.length() / D._view_active.fov_tan.y;
            if (FovPerspective(D.viewFovMode()))
                scale /= object->stat.view_matrix.cur.pos.z;
            D.stencil(STENCIL_NONE);
            SetMatrixCount();
            SetFurVelCount();
            SetViewMatrix(object->stat.view_matrix.cur);
            Sh.ViewMatrix->setChanged();
            SetViewMatrixPrev(object->stat.view_matrix.prev);
            Sh.ViewMatrixPrev->setChanged();
            SetShaderParamChanges(object->stat.shader_param_changes);

            Shader &shader = *object->stat.shader;
            C Material &material = *object->stat.material;
            material.setBlend();
            D.cull(material.cull);
            D.depthWrite(material.hasDepthWrite());
            Renderer.needDepthTest(); // !! 'needDepthTest' after 'depthWrite' !!
            C MeshRender &render = object->stat.mesh->render.set();
            shader.begin();
            DrawFur(render, shader, scale);
            SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
            PROFILE_STOP("CatzEngine::DrawBlendInstances()3")
        } break;

        case BlendInstance::SKELETON: {
            PROFILE_START("CatzEngine::DrawBlendInstances()4")
            ec.EndPrecomputedViewMatrix();
            EnableSkinning();
            SkeletonBlendInstance &skel = object->skeleton;
            skel.unlinkBlend();
            skel.anim_skel->setMatrix();
            Bool fur_set = false;
            Flt fur_scale;
            for (SkeletonBlendShader *skel_shader = &skel.skel_shader;;) {
                if (skel_shader->type == BlendInstance::STATIC_FUR && !fur_set) {
                    fur_set = true;
                    skel.anim_skel->setFurVel(); // !! needs to be called before 'shader.start' !!
                    fur_scale = skel.anim_skel->matrix().x.length() / D._view_active.fov_tan.y;
                    if (FovPerspective(D.viewFovMode()))
                        fur_scale /= DistPointActiveCamPlaneZ(skel.anim_skel->pos());
                }
                Shader &shader = skel_shader->shader->getBlendShader(skel_shader->type == BlendInstance::STATIC_BLST);
                shader.start();
                for (SkeletonShaderMaterial *skel_shader_material = &skel_shader->material;;) {
                    C Material &material = *skel_shader_material->material;
                    material.setBlend();
                    D.cull(material.cull);
                    D.depthWrite(material.hasDepthWrite());
                    Renderer.needDepthTest();
                    shader.commitTex(); // !! 'needDepthTest' after 'depthWrite' !!
                    Bool shader_params_changed = true;
                    for (SkeletonBlendShaderMaterialMeshInstance *instance = &SkeletonBlendShaderMaterialMeshInstances[skel_shader_material->first_mesh_instance];;) {
                        D.stencil(instance->stencil_mode);
                        shader_params_changed |= SetShaderParamChanges(instance->shader_param_changes);
                        if (shader_params_changed) {
                            shader_params_changed = false;
                            shader.commit();
                        }
                        instance->mesh->set();
                        if (skel_shader->type != BlendInstance::STATIC_FUR)
                            instance->mesh->draw();
                        else
                            DrawFur(*instance->mesh, shader, fur_scale);
                        if (instance->next_instance < 0)
                            break;
                        instance = &SkeletonBlendShaderMaterialMeshInstances[instance->next_instance];
                    }
                    SetShaderParamChanges(); // this must be called here before setting new shader params, because we may have some 'ShaderParamRestore' that we need to apply before any new shader params, for example if we don't call it here, and a new material is set, and we process 'SetShaderParamChanges' later, then it could restore the material values that are now old because new material was already set
                    if (skel_shader_material->next_skeleton_shader_material < 0)
                        break;
                    skel_shader_material = &SkeletonBlendShaderMaterials[skel_shader_material->next_skeleton_shader_material];
                }
                if (skel_shader->next_skeleton_shader < 0)
                    break;
                skel_shader = &SkeletonBlendShaders[skel_shader->next_skeleton_shader];
            }
            PROFILE_STOP("CatzEngine::DrawBlendInstances()4")
        } break;

        case BlendInstance::BLEND_OBJ:
            PROFILE_START("CatzEngine::DrawBlendInstances()5")
            ec.EndPrecomputedViewMatrix();
            D.stencil(STENCIL_NONE);
            object->blend_obj->drawBlend();
            D.alpha(alpha);
            D.depth(true);
            PROFILE_STOP("CatzEngine::DrawBlendInstances()5")
            break;
        case BlendInstance::GAME_OBJ:
            PROFILE_START("CatzEngine::DrawBlendInstances()6")
            ec.EndPrecomputedViewMatrix();
            D.stencil(STENCIL_NONE);
            object->game_obj->drawBlend();
            D.alpha(alpha);
            D.depth(true);
            PROFILE_STOP("CatzEngine::DrawBlendInstances()6")
            break;
        case BlendInstance::GAME_AREA:
            PROFILE_START("CatzEngine::DrawBlendInstances()7")
            ec.EndPrecomputedViewMatrix();
            D.stencil(STENCIL_NONE);
            object->game_area->customDrawBlend();
            D.alpha(alpha);
            D.depth(true);
            PROFILE_STOP("CatzEngine::DrawBlendInstances()7")
            break;
        }
    }

    // finish
    ec.EndPrecomputedViewMatrix();
    D.depthWrite(true);
    D.stencil(STENCIL_NONE);
    PROFILE_STOP("CatzEngine::DrawBlendInstances()")
}
/******************************************************************************/
// PALETTE
/******************************************************************************/
void DrawPaletteObjects() {
    PROFILE_START("CatzEngine::DrawPaletteObjects()")
    FREPAO(PaletteObjects)->drawPalette();
    FREPAO(PaletteAreas)->customDrawPalette();
    PROFILE_STOP("CatzEngine::DrawPaletteObjects()")
}
void DrawPalette1Objects() {
    PROFILE_START("CatzEngine::DrawPalette1Objects()")
    FREPAO(Palette1Objects)->drawPalette1();
    FREPAO(Palette1Areas)->customDrawPalette1();
    PROFILE_STOP("CatzEngine::DrawPalette1Objects()")
}
/******************************************************************************/
// OVERLAY
/******************************************************************************/
void DrawOverlayObjects() {
    PROFILE_START("CatzEngine::DrawOverlayObjects()")
    FREPAO(OverlayObjects)->drawOverlay();
    PROFILE_STOP("CatzEngine::DrawOverlayObjects()")
}
/******************************************************************************/
// OUTLINE
/******************************************************************************/
void DrawOutlineObjects() {
    PROFILE_START("CatzEngine::DrawOutlineObjects()")
    FREPAO(OutlineObjects)->drawOutline();
    PROFILE_STOP("CatzEngine::DrawOutlineObjects()")
}
/******************************************************************************/
// BEHIND
/******************************************************************************/
void DrawBehindObjects() {
    PROFILE_START("CatzEngine::DrawBehindObjects()")
    FREPAO(BehindObjects)->drawBehind();
    PROFILE_STOP("CatzEngine::DrawBehindObjects()")
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void ShutInstances() {
    MaterialShaders.del();
    ShaderDraws.del();
    MultiMaterialShaderDraws.del();
    ShaderMaterials.del();
    ShaderMaterialMeshes.del();
    OpaqueShaderMaterialMeshInstances.del();
    ShadowShaderMaterialMeshInstances.del();
    EmissiveInstances.del();
    SkeletonShaders.del();
    SkeletonBlendShaders.del();
    SkeletonShaderMaterials.del();
    SkeletonBlendShaderMaterials.del();
    SkeletonShadowShaderMaterialMeshInstances.del();
    SkeletonOpaqueShaderMaterialMeshInstances.del();
    SkeletonBlendShaderMaterialMeshInstances.del();
    SkeletonOpaqueInstances.del();
    SkeletonShadowInstances.del();
    SkeletonEmissiveInstances.del();

    BlendInstances.del();

    // OpaqueClothInstances.del(); ShadowClothInstances.del();

    PaletteObjects.del();
    Palette1Objects.del();
    OverlayObjects.del();
    OpaqueObjects.del();
    EmissiveObjects.del();
    OutlineObjects.del();
    BehindObjects.del();
    PaletteAreas.del();
    Palette1Areas.del();

    SetHighlight();
    Highlights.del();
}
void InitInstances() {
    PROFILE_START("CatzEngine::InitInstances()")
    // MaterialShaders                    .reserve(16); no need because this is used only in special cases where materials use more than 1 shader
    ShaderDraws.reserve(16);
    MultiMaterialShaderDraws.reserve(4);
    ShaderMaterials.reserve(16);
    ShaderMaterialMeshes.reserve(16);
    OpaqueShaderMaterialMeshInstances.reserve(16);
    ShadowShaderMaterialMeshInstances.reserve(16);
    SkeletonShaders.reserve(16);
    SkeletonBlendShaders.reserve(16);
    SkeletonShaderMaterials.reserve(16);
    SkeletonBlendShaderMaterials.reserve(16);
    SkeletonShadowShaderMaterialMeshInstances.reserve(16);
    SkeletonOpaqueShaderMaterialMeshInstances.reserve(16);
    SkeletonBlendShaderMaterialMeshInstances.reserve(16);
    SkeletonOpaqueInstances.reserve(16);
    SkeletonShadowInstances.reserve(16);
    BlendInstances.reserve(16);
    PROFILE_STOP("CatzEngine::InitInstances()")
}
void ClearInstances() {
    // unlink default materials because shadow draws may reuse them
    MaterialDefault.unlink();
    MaterialDefaultNoCull.unlink();

#if SUPPORT_EARLY_Z
    ClearEarlyZInstances();
#endif
    MaterialShaders.clear();
    REPAO(ShaderDraws).unlink();
    ShaderDraws.clear();
    REPAO(MultiMaterialShaderDraws).unlink();
    MultiMaterialShaderDraws.clear();
    REPAO(ShaderMaterials).unlink();
    ShaderMaterials.clear(); // !! unlink before 'ShaderMaterialMeshes' !! because 'ShaderMaterial.unlink' makes use of 'ShaderMaterialMeshes'
    REPAO(ShaderMaterialMeshes).unlink();
    ShaderMaterialMeshes.clear(); // !! unlink after  'ShaderMaterials'      !!
    OpaqueShaderMaterialMeshInstances.clear();
    ShadowShaderMaterialMeshInstances.clear();
    EmissiveInstances.clear();

    SkeletonShaders.clear();
    SkeletonBlendShaders.clear();
    SkeletonShaderMaterials.clear();
    SkeletonBlendShaderMaterials.clear();
    SkeletonShadowShaderMaterialMeshInstances.clear();
    SkeletonOpaqueShaderMaterialMeshInstances.clear();
    SkeletonBlendShaderMaterialMeshInstances.clear();
    REPAO(SkeletonOpaqueInstances).unlinkOpaque();
    SkeletonOpaqueInstances.clear();
    REPAO(SkeletonShadowInstances).unlinkShadow();
    SkeletonShadowInstances.clear();
    SkeletonEmissiveInstances.clear();

    REPAO(BlendInstances).unlink();
    BlendInstances.clear();

    // OpaqueClothInstances.clear(); ShadowClothInstances.clear();

    PaletteObjects.clear();
    Palette1Objects.clear();
    OverlayObjects.clear();
    OpaqueObjects.clear();
    EmissiveObjects.clear();
    OutlineObjects.clear();
    BehindObjects.clear();
    PaletteAreas.clear();
    Palette1Areas.clear();

#if DEBUG && 0
#pragma message("!! Warning: Use this only for debugging !!")
    Materials.lock();
    REPA(Materials)
    DYNAMIC_ASSERT(Materials.lockedData(i).canBeRemoved(), "Material still has some instance references after rendering finished");
    Materials.unlock();
#endif
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
