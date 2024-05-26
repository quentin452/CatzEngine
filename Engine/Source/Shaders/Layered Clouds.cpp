/******************************************************************************/
#include "!Header.h"
#include "Layered Clouds.h"
// NUM, BLEND
/******************************************************************************/
void LayeredClouds_VS(
    VtxInput vtx,

    out Vec pos
    : POS,
      out Vec uvw
    : UVW, // 3d tex coord
      out Half alpha
    : ALPHA,
      out Vec4 vpos
    : POSITION) {
    pos = vtx.pos();
    uvw = pos.xyz * Vec(LCScale, 1, LCScale);
    alpha = CloudAlpha(uvw.y);
    pos.y = pos.y * LCScaleY + (-LCScaleY + 1); // (pos.y-1)*LCScaleY+1
    pos = TransformPos(pos);
    vpos = Project(pos);
}
/******************************************************************************/
VecH4 LayeredClouds_PS(
    Vec pos
    : POS,
      Vec uvw
    : UVW,
      Half alpha
    : ALPHA,
      PIXEL,
      out Half outAlpha
    : TARGET1 // #RTOutput.Blend
    ) : TARGET {
    alpha = Sat(alpha);
    if (BLEND) {
        Flt range = TexDepthPix(pixel.xy) / Normalize(pos).z; // 0..Viewport.range
        alpha *= Sat(range * LCRange.x + LCRange.y);
    }
    Vec2 uv = Normalize(uvw).xz;
    VecH4 color;
    if (NUM >= 4) {
        VecH4 tex = RTex(Img3, uv * CL[3].scale + CL[3].position) * CL[3].color;
        if (NUM == 4)
            color = tex;
        else
            color = Lerp(color, tex, tex.a);
    }
    if (NUM >= 3) {
        VecH4 tex = RTex(Img2, uv * CL[2].scale + CL[2].position) * CL[2].color;
        if (NUM == 3)
            color = tex;
        else
            color = Lerp(color, tex, tex.a);
    }
    if (NUM >= 2) {
        VecH4 tex = RTex(Img1, uv * CL[1].scale + CL[1].position) * CL[1].color;
        if (NUM == 2)
            color = tex;
        else
            color = Lerp(color, tex, tex.a);
    }
    if (NUM >= 1) {
        VecH4 tex = RTex(Img, uv * CL[0].scale + CL[0].position) * CL[0].color;
        if (NUM == 1)
            color = tex;
        else
            color = Lerp(color, tex, tex.a);
    }

    color.a *= alpha;
    outAlpha = color.a;
    return color;
}
/******************************************************************************/
