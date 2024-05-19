/******************************************************************************/
#include "Fog.h"
#include "!Header.h"
/******************************************************************************/
// GLOBAL
/******************************************************************************/
VecH4 Fog_PS(NOPERSP Vec2 posXY
             : POS_XY,
               NOPERSP PIXEL
#if MULTI_SAMPLE == 2
               ,
               UInt index
             : SV_SampleIndex
#endif
             ) : TARGET {
    VecI2 pix = pixel.xy;
#if MULTI_SAMPLE != 1
#if MULTI_SAMPLE == 0 // 0: 1s->1s
    Vec pos = GetPosPix(pix, posXY);
#else // 2: ms->ms
    Vec pos = GetPosMS(pix, index, posXY);
#endif
    Half dns = AccumulatedDensity(FogDensity, Length(pos));
#else // 1: ms->1s
    Half dns = 0, valid = HALF_MIN;
    UNROLL for (Int i = 0; i < MS_SAMPLES; i++) {
        Flt depth = TexDepthRawMS(pix, i);
        if (DEPTH_FOREGROUND(depth)) {
            Vec pos = GetPos(LinearizeDepth(depth), posXY);
            dns += AccumulatedDensity(FogDensity, Length(pos));
            valid++;
        }
    }
    dns /= valid;
#endif
    return VecH4(FogColor, dns);
}
/******************************************************************************/
// LOCAL
/******************************************************************************/
// TODO: optimize fog shaders
void FogBox_VS(VtxInput vtx,
               out Vec outPos
               : TEXCOORD0,
                 out Vec uvw
               : TEXCOORD1,
                 NOINTERP out Vec4 size
               : SIZE,
                 NOINTERP out Matrix3 mtrx
               : MATRIX,
                 out Vec4 vpos
               : POSITION) {
    mtrx[0] = ViewMatrixX();
    size.x = Length(mtrx[0]);
    mtrx[0] /= size.x;
    mtrx[1] = ViewMatrixY();
    size.y = Length(mtrx[1]);
    mtrx[1] /= size.y;
    mtrx[2] = ViewMatrixZ();
    size.z = Length(mtrx[2]);
    mtrx[2] /= size.z;
    size.w = Max(size.xyz);

    // convert to texture space (0..1)
    uvw = vtx.pos() * 0.5 + 0.5;
    vpos = Project(outPos = TransformPos(vtx.pos()));
}
void FogBox_PS(
    Vec inPos
    : TEXCOORD0,
      Vec uvw
    : TEXCOORD1,
      NOINTERP Vec4 size
    : SIZE,
      NOINTERP Matrix3 mtrx
    : MATRIX,
      PIXEL,

      out VecH4 color
    : TARGET0,
      out Half alpha
    : TARGET1 // #RTOutput.Blend
) {
    Flt z = TexDepthPix(pixel.xy);
    Vec pos = uvw,
        dir = Normalize(inPos);
    dir *= Min((SQRT3 * 2) * size.w, (z - inPos.z) / dir.z);
    dir = TransformTP(dir, mtrx); // convert to box space

    // convert to texture space (0..1)
    dir = dir / (2 * size.xyz);

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
    dir *= size.xyz;
    Flt len = Length(dir) / size.w;

    Flt dns = LocalFogDensity;
#if HEIGHT
    dns *= 1 - Avg(pos.y, end.y);
    len *= 3;
#endif

    color.rgb = LocalFogColor;
    color.a = AccumulatedDensity(dns, len);
    alpha = color.a;
}
/******************************************************************************/
void FogBoxI_VS(VtxInput vtx,
                NOPERSP out Vec2 posXY
                : POS_XY,
                  NOINTERP out Vec4 size
                : SIZE,
                  NOINTERP out Matrix3 mtrx
                : MATRIX,
                  NOPERSP out Vec4 vpos
                : POSITION) {
    mtrx[0] = ViewMatrixX();
    size.x = Length(mtrx[0]);
    mtrx[0] /= size.x;
    mtrx[1] = ViewMatrixY();
    size.y = Length(mtrx[1]);
    mtrx[1] /= size.y;
    mtrx[2] = ViewMatrixZ();
    size.z = Length(mtrx[2]);
    mtrx[2] /= size.z;
    size.w = Max(size.xyz);

    vpos = vtx.pos4();
    posXY = UVToPosXY(vtx.uv());
}
void FogBoxI_PS(
    NOPERSP Vec2 posXY
    : POS_XY,
      NOINTERP Vec4 size
    : SIZE,
      NOINTERP Matrix3 mtrx
    : MATRIX,
      NOPERSP PIXEL,

      out VecH4 color
    : TARGET0,
      out Half alpha
    : TARGET1 // #RTOutput.Blend
) {
    Vec pos = GetPosPix(pixel.xy, posXY),
        dir = Normalize(pos);
    dir *= Min((SQRT3 * 2) * size.w, (pos.z - Viewport.from) / dir.z);
    dir = TransformTP(dir, mtrx); // convert to box space

    // convert to texture space (0..1)
    pos = LocalFogInside / (2 * size.xyz) + 0.5;
    dir = dir / (2 * size.xyz);

#if INSIDE == 0
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
#endif

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
    dir *= size.xyz;
    Flt len = Length(dir) / size.w;

    Flt dns = LocalFogDensity;
#if HEIGHT
    dns *= 1 - Avg(pos.y, end.y);
    len *= 3;
#endif

    color.rgb = LocalFogColor;
    color.a = AccumulatedDensity(dns, len);
    alpha = color.a;
}
/******************************************************************************/
void FogBall_VS(VtxInput vtx,
                out Vec outPos
                : TEXCOORD0,
                  out Vec uvw
                : TEXCOORD1,
                  NOINTERP out Flt size
                : SIZE,
                  out Vec4 vpos
                : POSITION) {
    uvw = vtx.pos();
    size = Length(ViewMatrixX());
    vpos = Project(outPos = TransformPos(vtx.pos()));
}
void FogBall_PS(
    Vec inPos
    : TEXCOORD0,
      Vec uvw
    : TEXCOORD1,
      NOINTERP Flt size
    : SIZE,
      PIXEL,

      out VecH4 color
    : TARGET0,
      out Half alpha
    : TARGET1 // #RTOutput.Blend
) {
    Flt z = TexDepthPix(pixel.xy);
    Vec pos = Normalize(uvw),
        dir = Normalize(inPos);
    Flt max_length = (z - inPos.z) / (dir.z * size);
    dir = Transform3(dir, CamMatrix); // convert to ball space

    // collision detection
    Vec p = PointOnPlane(pos, dir);
    Flt s = Length(p);
    s = Sat(1 - s * s);
    Vec end = p + Sqrt(s) * dir;

    Flt len = Min(Dist(pos, end), max_length);

    Flt dns = LocalFogDensity * s;

    color.rgb = LocalFogColor;
    color.a = AccumulatedDensity(dns, len);
    alpha = color.a;
}
/******************************************************************************/
void FogBallI_VS(VtxInput vtx,
                 NOPERSP out Vec2 posXY
                 : POS_XY,
                   NOINTERP out Flt size
                 : SIZE,
                   NOPERSP out Vec4 vpos
                 : POSITION) {
    vpos = vtx.pos4();
    posXY = UVToPosXY(vtx.uv());
    size = Length(ViewMatrixX());
}
void FogBallI_PS(
    NOPERSP Vec2 posXY
    : POS_XY,
      NOINTERP Flt size
    : SIZE,
      NOPERSP PIXEL,

      out VecH4 color
    : TARGET0,
      out Half alpha
    : TARGET1 // #RTOutput.Blend
) {
    Vec pos = GetPosPix(pixel.xy, posXY),
        dir = Normalize(pos);
    Flt max_length = (pos.z - Viewport.from) / (dir.z * size);
    dir = Transform3(dir, CamMatrix); // convert to ball space

    pos = LocalFogInside / size;

    // collision detection
    Vec p = PointOnPlane(pos, dir);
    Flt s = Length(p);
    s = Sat(1 - s * s);
    Vec end = p + dir * Sqrt(s);

    Flt len = Min(Dist(pos, end), max_length);

    Flt dns = LocalFogDensity * s;

    color.rgb = LocalFogColor;
    color.a = AccumulatedDensity(dns, len);
    alpha = color.a;
}
/******************************************************************************/
