/******************************************************************************

   This is used by 'MeshOverlay'

/******************************************************************************/
#include "!Header.h"
#include "Mesh Overlay.h"
/******************************************************************************
SKIN, NORMALS, LAYOUT
/******************************************************************************/
struct Data {
    Vec2 uv : UV;
    Half alpha : ALPHA;
#if NORMALS
    centroid MatrixH3 mtrx : MATRIX; // have to use 'centroid' to prevent values from getting outside of range, without centroid values can get MUCH different which might cause normals to be very big (very big vectors can't be normalized well, making them (0,0,0), which later causes NaN on normalization in other shaders)
#else
    centroid VecH nrm : NORMAL; // have to use 'centroid' to prevent values from getting outside of range, without centroid values can get MUCH different which might cause normals to be very big (very big vectors can't be normalized well, making them (0,0,0), which later causes NaN on normalization in other shaders)
#endif
};
/******************************************************************************/
void VS(
    VtxInput vtx,

    out Data D,
    out Vec4 vpos
    : POSITION) {
    Matrix3 m;
    m[0] = OverlayParams.mtrx[0] / Length2(OverlayParams.mtrx[0]);
    m[1] = OverlayParams.mtrx[1] / Length2(OverlayParams.mtrx[1]);
    m[2] = OverlayParams.mtrx[2] / Length2(OverlayParams.mtrx[2]);
    Vec pos = TransformTP(vtx.pos() - OverlayParams.mtrx[3], m);
    D.uv = pos.xy * 0.5 + 0.5;
    D.alpha = 1 - (Abs(pos.z) - OverlayOpaqueFrac()) / (1 - OverlayOpaqueFrac()); // Abs(pos.z)/-(1-OverlayOpaqueFrac()) + (OverlayOpaqueFrac()/(1-OverlayOpaqueFrac())+1)

    if (!SKIN) {
#if NORMALS
        D.mtrx[1] = Normalize(TransformDir(OverlayParams.mtrx[1]));
        D.mtrx[2] = Normalize(TransformDir(OverlayParams.mtrx[2]));
#else
        D.nrm = Normalize(TransformDir(OverlayParams.mtrx[2]));
#endif

        vpos = Project(TransformPos(vtx.pos()));
    } else {
        VecU4 bone = vtx.bone();

#if NORMALS
        D.mtrx[1] = Normalize(TransformDir(OverlayParams.mtrx[1], bone, vtx.weight()));
        D.mtrx[2] = Normalize(TransformDir(OverlayParams.mtrx[2], bone, vtx.weight()));
#else
        D.nrm = Normalize(TransformDir(OverlayParams.mtrx[2], bone, vtx.weight()));
#endif

        vpos = Project(TransformPos(vtx.pos(), bone, vtx.weight()));
    }
#if NORMALS
    D.mtrx[0] = Cross(D.mtrx[1], D.mtrx[2]);
#endif
}
/******************************************************************************/
VecH4 PS(
    Data D,
    out VecH4 outNrm
    : TARGET1,
      out VecH4 outExt
    : TARGET2) : TARGET // #RTOutput
{
    VecH4 col = RTex(Col, D.uv);
    col *= Material.color;
    col.a *= Sat(D.alpha) * OverlayAlpha();

    VecH nrm;
#if NORMALS
#if 0
                  nrm.xy =RTex(Nrm, D.uv).BASE_CHANNEL_NORMAL*Material.normal; // #MaterialTextureLayout
      //if(DETAIL)nrm.xy+=det.DETAIL_CHANNEL_NORMAL; // #MaterialTextureLayoutDetail
                  nrm.z  =CalcZ(nrm.xy);
#else
    nrm.xy = RTex(Nrm, D.uv).BASE_CHANNEL_NORMAL; // #MaterialTextureLayout
    nrm.z = CalcZ(nrm.xy);
    nrm.xy *= Material.normal;
    // if(DETAIL)nrm.xy+=det.DETAIL_CHANNEL_NORMAL; // #MaterialTextureLayoutDetail
#endif
    nrm = Normalize(Transform(nrm, D.mtrx));
#else
    nrm = Normalize(D.nrm);
#endif

    // Nrm
#if SIGNED_NRM_RT
    outNrm.xyz = nrm;
#else
    outNrm.xyz = nrm * 0.5 + 0.5;
#endif
    outNrm.w = col.a; // alpha needed because of blending

    // Ext
    Half rough, reflect;
#if LAYOUT == 2 // #MaterialTextureLayout
    VecH2 ext = RTex(Ext, D.uv).xy;
    rough = Sat(ext.BASE_CHANNEL_ROUGH * Material.rough_mul + Material.rough_add); // need to saturate to avoid invalid values. Even though we store values in 0..1 RT, we use alpha blending which may produce different results if values are outside 0..1
    reflect = ext.BASE_CHANNEL_METAL * Material.reflect_mul + Material.reflect_add;
#else
    rough = Material.rough_add;
    reflect = Material.reflect_add;
#endif
    outExt.x = rough;
    outExt.y = reflect;
    outExt.z = 0;
    outExt.w = col.a; // alpha needed because of blending

    return col;
}
/******************************************************************************/
