/******************************************************************************/
#include "stdafx.h"
namespace EE {
namespace Game {
/******************************************************************************/
static SkelAnim *GetAnim(AnimatedSkeleton &anim_skel, C ObjectPtr &obj, CChar8 *anim) {
    PROFILE_START("CatzEngine::SkelAnim *GetAnim(AnimatedSkeleton &anim_skel, C ObjectPtr &obj, CChar8 *anim)")
    if (obj)
        if (C Param *param = obj->findParam(anim)) {
            PROFILE_STOP("CatzEngine::SkelAnim *GetAnim(AnimatedSkeleton &anim_skel, C ObjectPtr &obj, CChar8 *anim)")
            return anim_skel.findSkelAnim(param->asID());
        }
    PROFILE_STOP("CatzEngine::SkelAnim *GetAnim(AnimatedSkeleton &anim_skel, C ObjectPtr &obj, CChar8 *anim)")
    return null;
}
/******************************************************************************/
Animatable::~Animatable() {
}
Animatable::Animatable() {
    scale = 0;
    mesh_variation = 0;
    skel_anim = null;
    _matrix.zero();
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void Animatable::create(Object &obj) {
    PROFILE_START("CatzEngine::Animatable::create(Object &obj)")
    arbitrary_name = "Animatable Object";
    base = obj.firstStored();
    mesh_variation = obj.meshVariationIndex();
    scale = obj.scale();
    _matrix = obj.matrixFinal().normalize();

    setUnsavedParams();

    if (skel.skeleton())
        skel_anim = GetAnim(skel, &obj, "animation");
    PROFILE_STOP("CatzEngine::Animatable::create(Object &obj)")
}
void Animatable::setUnsavedParams() {
    PROFILE_START("CatzEngine::Animatable::setUnsavedParams()")
    mesh = (base ? base->mesh() : MeshPtr());
    skel.create(mesh ? mesh->skeleton() : null, _matrix);

    if (base && base->phys())
        actor.create(*base->phys(), 0, scale)
            .matrix(_matrix)
            .obj(this)
            .group(AG_TERRAIN);
    PROFILE_STOP("CatzEngine::Animatable::setUnsavedParams()")
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Vec Animatable::pos() { return _matrix.pos; }
Matrix Animatable::matrix() { return _matrix; }
void Animatable::pos(C Vec &pos) {
    actor.pos(pos);
    T._matrix.pos = pos;
}
void Animatable::matrix(C Matrix &matrix) {
    actor.matrix(matrix);
    T._matrix = matrix;
}
/******************************************************************************/
// CALLBACKS
/******************************************************************************/
void Animatable::memoryAddressChanged() {
    PROFILE_START("CatzEngine::Animatable::memoryAddressChanged()")
    actor.obj(this);
    PROFILE_STOP("CatzEngine::Animatable::memoryAddressChanged()")
}
/******************************************************************************/
// UPDATE
/******************************************************************************/
Bool Animatable::update() {
    PROFILE_START("CatzEngine::Animatable::update()")
    skel.updateBegin()
        .clear()
        .animate(skel_anim, Time.time())
        .animateRoot(skel_anim ? skel_anim->animation() : null, Time.time())
        .updateMatrix(Matrix(_matrix).scaleOrn(scale))
        .updateEnd();
    PROFILE_STOP("CatzEngine::Animatable::update()")
    return true;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
UInt Animatable::drawPrepare() {
    PROFILE_START("CatzEngine::Animatable::drawPrepare()")
    if (mesh)
        if (Frustum(*mesh, skel.matrix())) {
            SetVariation(mesh_variation);
            mesh->draw(skel);
            SetVariation();
        }
    PROFILE_STOP("CatzEngine::Animatable::drawPrepare()")
    return 0; // no additional render modes required
}
/******************************************************************************/
void Animatable::drawShadow() {
    PROFILE_START("CatzEngine::Animatable::drawShadow()")
    if (mesh)
        if (Frustum(*mesh, skel.matrix())) {
            SetVariation(mesh_variation);
            mesh->drawShadow(skel);
            SetVariation();
        }
    PROFILE_STOP("CatzEngine::Animatable::drawShadow()")
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Animatable::save(File &f) {
    PROFILE_START("CatzEngine::Animatable::save(File &f)")
    if (super::save(f)) {
        f.cmpUIntV(0); // version

        f.putAsset(base.id()) << scale << _matrix;
        f.putAsset(skel_anim ? skel_anim->id() : UIDZero);
        f.putUInt(mesh ? mesh->variationID(mesh_variation) : 0);
        PROFILE_STOP("CatzEngine::Animatable::save(File &f)")
        return f.ok();
    }
    PROFILE_STOP("CatzEngine::Animatable::save(File &f)")
    return false;
}
/******************************************************************************/
Bool Animatable::load(File &f) {
    PROFILE_START("CatzEngine::Animatable::load(File &f)")
    if (super::load(f))
        switch (f.decUIntV()) // version
        {
        case 0: {
            base = f.getAssetID();
            f >> scale >> _matrix;
            setUnsavedParams();
            UID anim_id = f.getAssetID();
            if (skel.skeleton())
                skel_anim = skel.getSkelAnim(anim_id);
            UInt mesh_variation_id = f.getUInt();
            mesh_variation = (mesh ? mesh->variationFind(mesh_variation_id) : 0);
            if (f.ok()) {
                PROFILE_STOP("CatzEngine::Animatable::load(File &f)")
            }
            return true;
        } break;
        }
    PROFILE_STOP("CatzEngine::Animatable::load(File &f)")
    return false;
}
/******************************************************************************/
} // namespace Game
} // namespace EE
/******************************************************************************/
