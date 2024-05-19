/******************************************************************************/
#include "!Header.h"
/******************************************************************************
SKIN, COLORS, ALPHA_TEST, TEST_BLEND, FX, TESSELATE
/******************************************************************************/
#define SET_ALPHA (COLORS && ALPHA_TEST == ALPHA_TEST_DITHER)
/******************************************************************************/
struct Data {
#if TESSELATE
    Vec pos : POS;
    VecH nrm : NORMAL;
#endif

#if SET_ALPHA
    Half alpha : ALPHA;
#endif

#if ALPHA_TEST
    Vec2 uv : UV;
#endif

#if ALPHA_TEST == ALPHA_TEST_DITHER
    NOINTERP VecU2 face_id : FACE_ID;
#endif
};
/******************************************************************************/
// VS
/******************************************************************************/
void VS(
    VtxInput vtx,

    out Data O,
    out Vec4 vpos
    : POSITION) {
    Vec pos = vtx.pos();
    VecH nrm;
    if (TESSELATE)
        nrm = vtx.nrm();

    if (FX == FX_LEAF_2D || FX == FX_LEAF_3D) {
        if (TESSELATE)
            BendLeaf(vtx.hlp(), pos, nrm);
        else
            BendLeaf(vtx.hlp(), pos);
    }
    if (FX == FX_LEAFS_2D || FX == FX_LEAFS_3D) {
        if (TESSELATE)
            BendLeafs(vtx.hlp(), vtx.size(), pos, nrm);
        else
            BendLeafs(vtx.hlp(), vtx.size(), pos);
    }

    if (!SKIN) {
        Vec local_pos;
        if (FX == FX_GRASS_2D || FX == FX_GRASS_3D)
            local_pos = pos;
        if (true) // instance
        {
            pos = TransformPos(pos, vtx.instance());
            if (TESSELATE)
                nrm = Normalize(TransformDir(nrm, vtx.instance()));
            if (FX == FX_GRASS_2D || FX == FX_GRASS_3D)
                BendGrass(local_pos, pos, vtx.instance());
        } else {
            pos = TransformPos(pos);
            if (TESSELATE)
                nrm = Normalize(TransformDir(nrm));
            if (FX == FX_GRASS_2D || FX == FX_GRASS_3D)
                BendGrass(local_pos, pos);
        }
    } else {
        VecU4 bone = vtx.bone();
        pos = TransformPos(pos, bone, vtx.weight());
        if (TESSELATE)
            nrm = Normalize(TransformDir(nrm, bone, vtx.weight()));
    }

#if SET_ALPHA
    O.alpha = vtx.colorFastA() * Material.color.a;
#endif

#if ALPHA_TEST
    O.uv = vtx.uv();
#endif

#if ALPHA_TEST == ALPHA_TEST_DITHER
    O.face_id = vtx.faceID();
#endif

#if TESSELATE
    O.pos = pos;
    O.nrm = nrm;
#endif

    vpos = Project(pos);
}
/******************************************************************************/
// PS
/******************************************************************************/
void PS(
    Data I
#if ALPHA_TEST == ALPHA_TEST_DITHER
    ,
    PIXEL
#endif
) {
#if ALPHA_TEST && TEST_BLEND
    clip(RTex(Col, I.uv).a + Material.color.a * 0.5 - 1);
#elif ALPHA_TEST == ALPHA_TEST_YES
    MaterialAlphaTest(RTex(Col, I.uv).a);
#elif ALPHA_TEST == ALPHA_TEST_DITHER
#if SET_ALPHA
    Half alpha = I.alpha;
#else
    Half alpha = Material.color.a;
#endif
    MaterialAlphaTestDither(RTex(Col, I.uv).a * alpha, pixel.xy, I.face_id, false); // don't use noise offset for shadows because the shadow texels can be big on the screen and flickering disturbing
#endif
}
/******************************************************************************/
// HULL / DOMAIN
/******************************************************************************/
#if TESSELATE
HSData HSConstant(InputPatch<Data, 3> I) { return GetHSData(I[0].pos, I[1].pos, I[2].pos, I[0].nrm, I[1].nrm, I[2].nrm, true); }
[maxtessfactor(5.0)]
    [domain("tri")]
    [partitioning("fractional_odd")] // use 'odd' because it supports range from 1.0 ('even' supports range from 2.0)
    [outputtopology("triangle_cw")]
    [patchconstantfunc("HSConstant")]
    [outputcontrolpoints(3)] Data
    HS(InputPatch<Data, 3> I, UInt cp_id
       : SV_OutputControlPointID) {
        Data O;
        O.pos = I[cp_id].pos;
        O.nrm = I[cp_id].nrm;
#if SET_ALPHA
        O.alpha = I[cp_id].alpha;
#endif
#if ALPHA_TEST
        O.uv = I[cp_id].uv;
#endif
#if ALPHA_TEST == ALPHA_TEST_DITHER
        O.face_id = I[cp_id].face_id;
#endif
        return O;
    }
        /******************************************************************************/
        [domain("tri")] void DS(
            HSData hs_data, const OutputPatch<Data, 3> I, Vec B
            : SV_DomainLocation,

              out Data O,
              out Vec4 pixel
            : POSITION) {
#if SET_ALPHA
    O.alpha = I[0].alpha * B.z + I[1].alpha * B.x + I[2].alpha * B.y;
#endif

#if ALPHA_TEST
    O.uv = I[0].uv * B.z + I[1].uv * B.x + I[2].uv * B.y;
#endif

#if ALPHA_TEST == ALPHA_TEST_DITHER
    O.face_id = I[0].face_id;
#endif

    SetDSPosNrm(O.pos, O.nrm, I[0].pos, I[1].pos, I[2].pos, I[0].nrm, I[1].nrm, I[2].nrm, B, hs_data, false, 0);
    pixel = Project(O.pos);
}
#endif
/******************************************************************************/
