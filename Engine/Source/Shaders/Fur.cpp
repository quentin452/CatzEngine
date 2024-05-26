/******************************************************************************/
#include "!Header.h"
#include "Fur.h"
/******************************************************************************/
#define USE_VEL 1
#define FACTOR (-0.7) // prevents complete darkness at the bottom layers, gives ambient=0.3, it will match the 'size' version
/******************************************************************************/
VecH GetBoneFurVel(VecU4 bone, VecH4 weight) { return weight.x * FurVel[bone.x] + weight.y * FurVel[bone.y] + weight.z * FurVel[bone.z] + weight.w * FurVel[bone.w]; }
/******************************************************************************/
// SKIN, SIZE, DIFFUSE
/******************************************************************************/
struct BaseData {
    Vec2 uv : UV;

    centroid VecH nrm : NORMAL; // !! not Normalized !! have to use 'centroid' to prevent values from getting outside of range, without centroid values can get MUCH different which might cause normals to be very big (very big vectors can't be normalized well, making them (0,0,0), which later causes NaN on normalization in other shaders)

#if USE_VEL
    Vec projected_prev_pos_xyw : PREV_POS;
#endif

#if SIZE
    Half len : LENGTH;
#endif
};
void Base_VS(
    VtxInput vtx,

    out BaseData O,
    out Vec4 vpos
    : POSITION,

      CLIP_DIST) {
    Vec view_pos, view_pos_prev;
    O.uv = vtx.uv();

    if (!SKIN) {
        if (true) // instance
        {
            view_pos = TransformPos(vtx.pos(), vtx.instance());
            if (USE_VEL)
                view_pos_prev = TransformPosPrev(vtx.pos(), vtx.instance());
            O.nrm = TransformDir(vtx.nrm(), vtx.instance());
        } else {
            view_pos = TransformPos(vtx.pos());
            if (USE_VEL)
                view_pos_prev = TransformPosPrev(vtx.pos());
            O.nrm = TransformDir(vtx.nrm());
        }
    } else {
        VecU4 bone = vtx.bone();
        view_pos = TransformPos(vtx.pos(), bone, vtx.weight());
        if (USE_VEL)
            view_pos_prev = TransformPosPrev(vtx.pos(), bone, vtx.weight());
        O.nrm = TransformDir(vtx.nrm(), bone, vtx.weight());
    }
#if SIZE
    O.len = vtx.size();
#endif
    vpos = Project(view_pos);
    CLIP_PLANE(view_pos);
#if USE_VEL
    O.projected_prev_pos_xyw = ProjectPrevXYW(view_pos_prev);
#endif
}
/******************************************************************************/
void Base_PS(
    BaseData I,
#if USE_VEL
    PIXEL,
#endif
    out DeferredOutput output) {
    Half fur = RTex(FurCol, I.uv * Material.det_uv_scale).r;
#if SIZE
    VecH col = Sat(I.len * -fur + 1); // I.len*-fur+step+1 : fur*FACTOR+step+1, here step=0
#else
    VecH col = Sat(fur * FACTOR + 1); // I.len*-fur+step+1 : fur*FACTOR+step+1, here step=0
#endif
    if (DIFFUSE)
        col *= RTex(Col, I.uv).rgb;
    col = col * Material.color.rgb + Highlight.rgb;

    I.nrm = Normalize(I.nrm);

    output.color(col);
    output.glow(0);
    output.normal(I.nrm);
    output.mode(PSM_NONE);
    output.rough(Material.rough_add);
    output.reflect(Material.reflect_add);
#if USE_VEL
    output.motion(I.projected_prev_pos_xyw, pixel);
#else
    output.motionZero();
#endif
}
/******************************************************************************/
void Soft_VS(
    VtxInput vtx,

    out Vec2 uv
    : UV,
      out Vec orig_pos
    : ORIG_POS,
#if SIZE
      out Half length
    : LENGTH,
#endif
      out Vec4 pixel
    : POSITION) {
    Vec pos = vtx.pos();
    VecH nrm = vtx.nrm();

    uv = vtx.uv();

    if (!SKIN) {
        pos = TransformPos(pos);
        nrm += FurVel[0];
        nrm = Normalize(nrm);
        nrm = TransformDir(nrm);
    } else {
        VecU4 bone = vtx.bone();
        pos = TransformPos(pos, bone, vtx.weight());
        nrm += GetBoneFurVel(bone, vtx.weight());
        nrm = Normalize(nrm);
        nrm = TransformDir(nrm, bone, vtx.weight());
    }
    orig_pos = ProjectXYW(pos); // set in 'orig_pos' the original position without expansion
#if SIZE
    length = vtx.size();
#endif
    pos += nrm * (SIZE ? vtx.size() * Material.det_power * FurStep.x : Material.det_power * FurStep.x);
    pixel = Project(pos);
}
/******************************************************************************/
VecH4 Soft_PS(
    Vec2 uv
    : UV,
      Vec orig_pos
    : ORIG_POS
#if SIZE
      ,
      Half length
    : LENGTH
#endif
      ,
      out Half outAlpha
    : TARGET1 // #RTOutput.Blend
    ) : TARGET {
    Half fur = RTex(FurCol, uv * Material.det_uv_scale).r;

    VecH4 color;
#if SIZE
    color.rgb = Sat(length * -fur + FurStep.y);    // length*-fur+step+1 : fur*FACTOR+step+1
    color.a = Sat(length * (1 - FurStep.x / fur)); // alternative: Sat(1-FurStep.x/(fur*length))
#else
    color.rgb = Sat(fur * FACTOR + FurStep.y); // length*-fur+step+1 : fur*FACTOR+step+1
    color.a = Sat(1 - FurStep.x / fur);        // alternative: Sat(1-FurStep.x/(fur*length))
#endif

    outAlpha = color.a;

    if (DIFFUSE)
        color.rgb *= RTex(Col, uv).rgb;
    color.rgb = (color.rgb * Material.color.rgb + Highlight.rgb) * TexPoint(FurLight, ProjectedPosXYWToUV(orig_pos)).rgb; // we need to access the un-expanded pixel and not current pixel, 'TexPoint' is used in case it can offer faster performance
    return color;
}
/******************************************************************************/
