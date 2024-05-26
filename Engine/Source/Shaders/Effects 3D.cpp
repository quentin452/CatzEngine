/******************************************************************************/
#include "!Header.h"
#ifndef INSIDE
#define INSIDE 0
#endif
#ifndef LA
#define LA 0
#endif
#ifndef NORMALS
#define NORMALS 0
#endif
/******************************************************************************

   Keep here effects that are rarely used.

/******************************************************************************/
// VOLUME
/******************************************************************************/
#include "!Set Prec Struct.h"
struct VolumeClass {
    Flt min_steps,
        max_steps,
        density_factor,
        precision;
    Vec size,
        pixels,
        inside;
};

BUFFER(Volume)
VolumeClass Volume;
BUFFER_END
#include "!Set Prec Default.h"

void Volume_VS(VtxInput vtx,
               out Vec4 vpos
               : POSITION,
                 out Vec outPos
               : TEXCOORD0,
                 out Vec outTex
               : TEXCOORD1,
                 NOINTERP out Matrix3 mtrx
               : MATRIX) {
    mtrx[0] = Normalize(ViewMatrixX());
    mtrx[1] = Normalize(ViewMatrixY());
    mtrx[2] = Normalize(ViewMatrixZ());

    // convert to texture space (0..1)
    if (INSIDE)
        outTex = Volume.inside / (2 * Volume.size) + 0.5;
    else
        outTex = vtx.pos() * 0.5 + 0.5;

    vpos = Project(outPos = TransformPos(vtx.pos()));
}
void Volume_PS(
    PIXEL,

    Vec inPos
    : TEXCOORD0,
      Vec inTex
    : TEXCOORD1,
      NOINTERP Matrix3 mtrx
    : MATRIX,

      out VecH4 color
    : TARGET0,
      out Half alpha
    : TARGET1 // #RTOutput.Blend
) {
    Flt z = TexDepthPix(pixel.xy);
    Vec pos = inTex;
    Vec dir = Normalize(inPos);
    dir *= Min((SQRT3 * 2) * Max(Volume.size), (z - (INSIDE ? Viewport.from : inPos.z)) / dir.z);
    dir = TransformTP(dir, mtrx); // convert to box space

    // convert to texture space (0..1)
    dir = dir / (2 * Volume.size);

    if (INSIDE == 1) {
        if (pos.x < 0)
            pos += (0 - pos.x) / dir.x * dir;
        if (pos.x > 1)
            pos += (1 - pos.x) / dir.x * dir;
        if (pos.y < 0)
            pos += (0 - pos.y) / dir.y * dir;
        if (pos.y > 1)
            pos += (1 - pos.y) / dir.y * dir;
        if (pos.z < 0)
            pos += (0 - pos.z) / dir.z * dir;
        if (pos.z > 1)
            pos += (1 - pos.z) / dir.z * dir;
    }

    Vec end = pos + dir;

    if (end.x < 0)
        end += (0 - end.x) / dir.x * dir;
    if (end.x > 1)
        end += (1 - end.x) / dir.x * dir;
    if (end.y < 0)
        end += (0 - end.y) / dir.y * dir;
    if (end.y > 1)
        end += (1 - end.y) / dir.y * dir;
    if (end.z < 0)
        end += (0 - end.z) / dir.z * dir;
    if (end.z > 1)
        end += (1 - end.z) / dir.z * dir;

    dir = end - pos;
    Flt pixels = Length(dir * Volume.pixels);
    Int steps = Mid(pixels * Volume.precision, Volume.min_steps, Volume.max_steps);
    Flt steps_speed = pixels / steps;

    Flt density_factor = Volume.density_factor;
    density_factor = 1 - Pow(1 - density_factor, steps_speed); // modify by steps speed

    dir /= steps;

    // !! Use high precision here because iterating many samples !!

    if (LA) {
        Vec2 col = 0;

        LOOP for (Int i = 0; i < steps; i++) {
            Vec2 sample = Tex3DLod(VolXY, pos).rg; // HP, UV clamp needed
            Flt alpha = sample.g * density_factor * (1 - col.g);

            col.r += alpha * sample.r;
            col.g += alpha;

            pos += dir;
        }

        col.r /= col.g + HALF_MIN; // NaN

        if (LINEAR_GAMMA)
            col.r = SRGBToLinearFast(col.r); // for best precision this could be 'SRGBToLinear', however this is used for clouds, and slightly different gamma is acceptable

        color = col.rrrg * Color[0] + Color[1];
    } else {
        Vec4 col = 0;

        LOOP for (Int i = 0; i < steps; i++) {
            Vec4 sample = Tex3DLod(Vol, pos); // HP, UV clamp needed
            Flt alpha = sample.a * density_factor * (1 - col.a);

            col.rgb += alpha * sample.rgb;
            col.a += alpha;

            pos += dir;
        }

        col.rgb /= col.a + HALF_MIN; // NaN

        color = col * Color[0] + Color[1];
    }
    alpha = color.a;
}
/******************************************************************************/
// LASER
/******************************************************************************/
void Laser_VS(VtxInput vtx,
              out Vec outPos
              : TEXCOORD0,
#if NORMALS
                out VecH outNrm
              : TEXCOORD1,
#endif
                out Vec4 vpos
              : POSITION) {
#if NORMALS
    outNrm = TransformDir(vtx.nrm());
#endif
    vpos = Project(outPos = TransformPos(vtx.pos()));
}
void Laser_PS(Vec inPos
              : TEXCOORD0,
#if NORMALS
                VecH inNrm
              : TEXCOORD1,
#endif
                out DeferredOutput output) {
#if NORMALS
    inNrm = Normalize(inNrm);
    Half stp = Max(-Dot(inNrm, Normalize(inPos)), -inNrm.z);
    stp = Sat(stp);
    stp = Pow(stp, Step);
    VecH4 col = Lerp(Color[0], Color[1], stp);
    output.color(col.rgb);
    output.glow(col.a);
#else
    output.color(Color[0].rgb);
    output.glow(Color[0].a);
#endif
    output.normal(0);
    output.mode(PSM_NONE);
    output.rough(1);
    output.reflect(0);
    output.motionZero();
}
/******************************************************************************/
// DECAL
/******************************************************************************/
// MODE 0-overlay, 1-blend, 2-palette, FULLSCREEN, LAYOUT, NORMALS
#include "!Set Prec Struct.h"
BUFFER(Decal)
VecH2 DecalParams; // x=OpaqueFracMul, y=OpaqueFracAdd
BUFFER_END
#include "!Set Prec Default.h"

Half DecalOpaqueFracMul() { return DecalParams.x; }
Half DecalOpaqueFracAdd() { return DecalParams.y; }

struct Data {
    Matrix pos_mtrx : POS_MATRIX;

#if MODE == 0
#if NORMALS
    MatrixH3 nrm_mtrx : NRM_MATRIX;
#else
    VecH nrm : NORMAL;
#endif
#endif
};

void Decal_VS(
    VtxInput vtx,
    out Data D,
    out Vec4 vpos
    : POSITION) {
    D.pos_mtrx = GetViewMatrix();
    D.pos_mtrx[0] /= Length2(D.pos_mtrx[0]);
    D.pos_mtrx[1] /= Length2(D.pos_mtrx[1]);
    D.pos_mtrx[2] /= Length2(D.pos_mtrx[2]);

#if MODE == 0
#if NORMALS
    D.nrm_mtrx[0] = Normalize(D.pos_mtrx[0]);
    D.nrm_mtrx[1] = Normalize(D.pos_mtrx[1]);
    D.nrm_mtrx[2] = Normalize(D.pos_mtrx[2]);
#else
    D.nrm = Normalize(D.pos_mtrx[2]);
#endif
#endif

#if FULLSCREEN
    vpos = vtx.pos4();
#else
    vpos = Project(TransformPos(vtx.pos()));
#endif
}
VecH4 Decal_PS(
    Data D,
    PIXEL
#if MODE == 0 // overlay
    ,
    out VecH4 outNrm
    : TARGET1, out VecH4 outExt
    : TARGET2
#elif MODE == 1 // blend
    ,
    out Half alpha
    : TARGET1 // #RTOutput.Blend
#endif
    ) : TARGET // #RTOutput
{
    Vec pos = GetPosPoint(PixelToUV(pixel));
    pos = TransformTP(pos - D.pos_mtrx[3], (Matrix3)D.pos_mtrx);
    Half fade = Sat(Abs(pos.z) * DecalOpaqueFracMul() + DecalOpaqueFracAdd());

    clip(Vec(1 - Abs(pos.xy), fade - EPS_COL));

    Vec2 uv = pos.xy * 0.5 + 0.5;

    VecH4 col = RTex(Col, uv);
    col.a *= fade;

#if MODE == 0 // overlay
    col *= Material.color * Color[0];

    // Nrm
    VecH nrm;
#if NORMALS
#if 0
         nrm.xy =RTex(Nrm, uv).BASE_CHANNEL_NORMAL*Material.normal; // #MaterialTextureLayout
//if(DETAIL)nrm.xy+=det.DETAIL_CHANNEL_NORMAL; // #MaterialTextureLayoutDetail
         nrm.z  =CalcZ(nrm.xy);
#else
    nrm.xy = RTex(Nrm, uv).BASE_CHANNEL_NORMAL; // #MaterialTextureLayout
    nrm.z = CalcZ(nrm.xy);
    nrm.xy *= Material.normal;
    // if(DETAIL)nrm.xy+=det.DETAIL_CHANNEL_NORMAL; // #MaterialTextureLayoutDetail
#endif
    nrm = Normalize(Transform(nrm, D.nrm_mtrx));
#else
    nrm = Normalize(D.nrm);
#endif

#if SIGNED_NRM_RT
    outNrm.xyz = nrm;
#else
    outNrm.xyz = nrm * 0.5 + 0.5;
#endif
    outNrm.w = col.a; // alpha needed because of blending

    // Ext
    Half rough, reflect;
#if LAYOUT == 2 // #MaterialTextureLayout
    VecH2 ext = RTex(Ext, uv).xy;
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
#elif MODE == 1 // blend
    col *= Material.color * Color[0];
    alpha = col.a;
    return col;
#else // palette
    return (col.a * Material.color.a) * Color[0];
#endif
}
/******************************************************************************/
