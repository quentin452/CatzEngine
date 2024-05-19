/******************************************************************************/
#include "!Header.h"

#include "!Set Prec Struct.h"
BUFFER(BoneHighlight)
Flt BoneHighlight;
BUFFER_END
#include "!Set Prec Default.h"
/******************************************************************************
SKIN, BUMP_MODE
/******************************************************************************/
struct Data {
#if BUMP_MODE >= SBUMP_FLAT
    centroid VecH nrm : NORMAL; // !! may not be Normalized !! have to use 'centroid' to prevent values from getting outside of range, without centroid values can get MUCH different which might cause normals to be very big (very big vectors can't be normalized well, making them (0,0,0), which later causes NaN on normalization in other shaders)
#endif
    VecH col : COLOR;
};
/******************************************************************************/
void VS(
    VtxInput vtx,

    out Data O,
    out Vec4 vpos
    : POSITION) {
#if BUMP_MODE >= SBUMP_FLAT
    O.nrm = TransformDir(vtx.nrm());
#endif

    Flt b;
    if (!SKIN)
        b = 0;
    else {
        VecI bone = vtx._bone; // use real bone regardless if skinning is enabled
        b = vtx.weight().x * Sat(1 - Abs(bone.x - BoneHighlight)) + vtx.weight().y * Sat(1 - Abs(bone.y - BoneHighlight)) + vtx.weight().z * Sat(1 - Abs(bone.z - BoneHighlight));
    }
    O.col = ((b > EPS) ? VecH(b, 0, 1 - b) : VecH(1, 1, 1));

    vpos = Project(TransformPos(vtx.pos()));
}
/******************************************************************************/
VecH4 PS(Data I, IS_FRONT) : TARGET {
    if (LINEAR_GAMMA)
        I.col.rgb = SRGBToLinear(I.col.rgb);
    I.col.rgb += Highlight.rgb;

    // perform lighting
#if BUMP_MODE >= SBUMP_FLAT
    VecH nrm = Normalize(I.nrm);
    BackFlip(nrm, front);
    Half d = Sat(Dot(nrm, LightDir.dir));
    if (LINEAR_GAMMA)
        d = Sqr(d);
    VecH lum = LightDir.color.rgb * d + AmbientNSColor;
    I.col.rgb *= lum;
#endif

    return VecH4(I.col.rgb, 0);
}
/******************************************************************************/
