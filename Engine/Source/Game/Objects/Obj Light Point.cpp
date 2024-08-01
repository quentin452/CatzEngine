/******************************************************************************/
#include "stdafx.h"
namespace EE {
namespace Game {
/******************************************************************************/
ObjLightPoint::~ObjLightPoint() {
}
ObjLightPoint::ObjLightPoint() {
    cast_shadows = true;
    range = 0;
    volumetric = 0;
    position.zero();
    color.set(1, 1, 1);
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void ObjLightPoint::create(Object &obj) {
    PROFILE_START("CatzEngine::ObjLightPoint::create(Object &obj)")
    arbitrary_name = "ObjLightPoint Object";
    range = obj.scale();
    position = obj.matrixFinal().pos;
    if (Param *param = obj.findParam("cast shadows"))
        cast_shadows = param->asBool();
    if (Param *param = obj.findParam("volumetric"))
        volumetric = Max(param->asFlt(), 0.0f);
    if (Param *param = obj.findParam("color"))
        color = SRGBToLinear(param->asVec());
    PROFILE_STOP("CatzEngine::ObjLightPoint::create(Object &obj)")
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Vec ObjLightPoint::pos() { return position; }
void ObjLightPoint::pos(C Vec &pos) { position = pos; }
Matrix ObjLightPoint::matrix() { return position; }
void ObjLightPoint::matrix(C Matrix &matrix) { position = matrix.pos; }
/******************************************************************************/
// UPDATE
/******************************************************************************/
Bool ObjLightPoint::update() {
    return true;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
void ObjLightPoint::drawPrepare(C Matrix &matrix) {
    PROFILE_START("CatzEngine::ObjLightPoint::drawPrepare(C Matrix &matrix)")
    LightLinear(range, position * matrix, color, volumetric).add(cast_shadows, this);
    PROFILE_STOP("CatzEngine::ObjLightPoint::drawPrepare(C Matrix &matrix)")
}
UInt ObjLightPoint::drawPrepare() {
    PROFILE_START("CatzEngine::ObjLightPoint::drawPrepare()")
    LightLinear(range, position, color, volumetric).add(cast_shadows, this);
    PROFILE_STOP("CatzEngine::ObjLightPoint::drawPrepare()")
    return 0; // no additional render modes required
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool ObjLightPoint::save(File &f) {
    if (super::save(f)) {
        f.cmpUIntV(0); // version

        f << cast_shadows << range << position << color;
        return f.ok();
    }
    return false;
}
Bool ObjLightPoint::load(File &f) {
    if (super::load(f))
        switch (f.decUIntV()) // version
        {
        case 0: {
            f >> cast_shadows >> range >> position >> color;
            if (f.ok())
                return true;
        } break;
        }
    return false;
}
/******************************************************************************/
} // namespace Game
} // namespace EE
/******************************************************************************/
