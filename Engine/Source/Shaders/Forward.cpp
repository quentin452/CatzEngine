/******************************************************************************/
#include "!Header.h"
/******************************************************************************
SKIN, MATERIALS, LAYOUT, BUMP_MODE, ALPHA_TEST, REFLECT, EMISSIVE_MAP, DETAIL, COLORS, MTRL_BLEND, UV_SCALE, HEIGHTMAP, FX, PER_PIXEL
LIGHT_DIR, LIGHT_DIR_SHD, LIGHT_DIR_SHD_NUM
LIGHT_POINT, LIGHT_POINT_SHD
LIGHT_LINEAR, LIGHT_LINEAR_SHD
LIGHT_CONE, LIGHT_CONE_SHD
TESSELATE

FinalLight = Ambient + Light*Shadow
Final = (TexCol*MtrlCol*VtxCol+Detail)*FinalLight
/******************************************************************************/
#define NO_AMBIENT 0 // this could be set to 1 for Secondary Passes, if we would use this then we could remove 'FirstPass'
#define HAS_AMBIENT (!NO_AMBIENT)

#define LIGHT (LIGHT_DIR || LIGHT_POINT || LIGHT_LINEAR || LIGHT_CONE)
#define SHADOW (LIGHT_DIR_SHD || LIGHT_POINT_SHD || LIGHT_LINEAR_SHD || LIGHT_CONE_SHD)
#define VTX_LIGHT (LIGHT && !PER_PIXEL)
#define AMBIENT_IN_VTX (VTX_LIGHT && !SHADOW)                                      // if stored per-vertex (in either 'vtx.col' or 'vtx.lum')
#define LIGHT_IN_COL (VTX_LIGHT && !DETAIL && (NO_AMBIENT || !SHADOW) && !REFLECT) // can't mix light with vtx.col when REFLECT because for reflections we need unlit color
#define SET_POS ((LIGHT && PER_PIXEL) || SHADOW || REFLECT || TESSELATE)
#define SET_UV (LAYOUT || DETAIL || EMISSIVE_MAP || BUMP_MODE > SBUMP_FLAT)
#define SET_COL (COLORS || LIGHT_IN_COL)
#define SET_LUM (VTX_LIGHT && !LIGHT_IN_COL)
#define VTX_REFLECT (REFLECT && !PER_PIXEL && BUMP_MODE <= SBUMP_FLAT) // require !PER_PIXEL because even without normal maps (SBUMP_FLAT) the quality suffers
#define PIXEL_NORMAL ((PER_PIXEL && LIGHT) || REFLECT)                 // if calculate normal in the pixel shader
#define GRASS_FADE (FX == FX_GRASS_2D || FX == FX_GRASS_3D)
/******************************************************************************/
struct Data {
#if SET_POS
    Vec pos : POS;
#endif

#if SET_UV
    Vec2 uv : UV;
#endif

#if VTX_REFLECT
    Vec reflect_dir : REFLECTION;
#endif

#if BUMP_MODE > SBUMP_FLAT && PIXEL_NORMAL
    centroid MatrixH3 mtrx : MATRIX; // !! may not be Normalized !! have to use 'centroid' to prevent values from getting outside of range, without centroid values can get MUCH different which might cause normals to be very big (very big vectors can't be normalized well, making them (0,0,0), which later causes NaN on normalization in other shaders)
    VecH Nrm() { return mtrx[2]; }
#elif BUMP_MODE >= SBUMP_FLAT && (PIXEL_NORMAL || TESSELATE)
    centroid VecH nrm : NORMAL; // !! may not be Normalized !! have to use 'centroid' to prevent values from getting outside of range, without centroid values can get MUCH different which might cause normals to be very big (very big vectors can't be normalized well, making them (0,0,0), which later causes NaN on normalization in other shaders)
    VecH Nrm() { return nrm; }
#else
    VecH Nrm() { return 0; }
#endif

#if MATERIALS > 1
    centroid VecH4 material : MATERIAL; // have to use 'centroid' to prevent values from getting outside of 0..1 range, without centroid values can get MUCH different which might cause infinite loop in Relief=crash, and cause normals to be very big (very big vectors can't be normalized well, making them (0,0,0), which later causes NaN on normalization in other shaders)
#endif

#if SET_COL
#if ALPHA_TEST == ALPHA_TEST_DITHER
    VecH4 col : COLOR;
#else
    VecH col : COLOR;
#endif
#endif

#if GRASS_FADE
    Half fade_out : FADE_OUT;
#endif

#if SET_LUM
    VecH lum : LUM;
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
    : POSITION,

      CLIP_DIST) {
    Vec pos = vtx.pos();
    VecH nrm, tan;
    if (BUMP_MODE >= SBUMP_FLAT)
        nrm = vtx.nrm();
    if (BUMP_MODE > SBUMP_FLAT)
        tan = vtx.tan(nrm, HEIGHTMAP);

#if SET_UV
    O.uv = vtx.uv(HEIGHTMAP);
    if (UV_SCALE && MATERIALS == 1)
        O.uv *= Material.uv_scale;
#endif

#if ALPHA_TEST == ALPHA_TEST_DITHER
    O.face_id = vtx.faceID();
#endif

#if MATERIALS > 1
    O.material = vtx.material();
#endif

#if COLORS
#if ALPHA_TEST == ALPHA_TEST_DITHER
    if (MATERIALS <= 1)
        O.col = vtx.colorFast() * Material.color;
    else
        O.col = vtx.colorFast();
#else
    if (MATERIALS <= 1)
        O.col = vtx.colorFast3() * Material.color.rgb;
    else
        O.col = vtx.colorFast3();
#endif
#endif

    if (FX == FX_LEAF_2D || FX == FX_LEAF_3D) {
        if (BUMP_MODE > SBUMP_FLAT)
            BendLeaf(vtx.hlp(), pos, nrm, tan);
        else if (BUMP_MODE == SBUMP_FLAT)
            BendLeaf(vtx.hlp(), pos, nrm);
        else
            BendLeaf(vtx.hlp(), pos);
    }
    if (FX == FX_LEAFS_2D || FX == FX_LEAFS_3D) {
        if (BUMP_MODE > SBUMP_FLAT)
            BendLeafs(vtx.hlp(), vtx.size(), pos, nrm, tan);
        else if (BUMP_MODE == SBUMP_FLAT)
            BendLeafs(vtx.hlp(), vtx.size(), pos, nrm);
        else
            BendLeafs(vtx.hlp(), vtx.size(), pos);
    }

    Vec local_pos;
    if (FX == FX_GRASS_2D || FX == FX_GRASS_3D)
        local_pos = pos;
    if (!SKIN) {
        if (true) // instance
        {
            pos = TransformPos(pos, vtx.instance());

#if BUMP_MODE > SBUMP_FLAT
            nrm = TransformDir(nrm, vtx.instance());
            tan = TransformDir(tan, vtx.instance());
#elif BUMP_MODE == SBUMP_FLAT
            nrm = TransformDir(nrm, vtx.instance());
#endif

            if (FX == FX_GRASS_2D || FX == FX_GRASS_3D)
                BendGrass(local_pos, pos, vtx.instance());
#if GRASS_FADE
            O.fade_out = GrassFadeOut(vtx.instance());
#endif
        } else {
            pos = TransformPos(pos);

#if BUMP_MODE > SBUMP_FLAT
            nrm = TransformDir(nrm);
            tan = TransformDir(tan);
#elif BUMP_MODE == SBUMP_FLAT
            nrm = TransformDir(nrm);
#endif

            if (FX == FX_GRASS_2D || FX == FX_GRASS_3D)
                BendGrass(local_pos, pos);
#if GRASS_FADE
            O.fade_out = GrassFadeOut();
#endif
        }
    } else {
        VecU4 bone = vtx.bone();
        VecH4 weight_h = vtx.weight();
        pos = TransformPos(pos, bone, vtx.weight());

#if BUMP_MODE > SBUMP_FLAT
        nrm = TransformDir(nrm, bone, weight_h);
        tan = TransformDir(tan, bone, weight_h);
#elif BUMP_MODE == SBUMP_FLAT
        nrm = TransformDir(nrm, bone, weight_h);
#endif
    }

    // normalize (have to do all at the same time, so all have the same lengths)
    if (BUMP_MODE > SBUMP_FLAT // calculating binormal (this also covers the case when we have tangent from heightmap which is not Normalized)
        || VTX_REFLECT         // per-vertex reflection
        || VTX_LIGHT           // per-vertex lighting
        || TESSELATE)          // needed for tesselation
    {
        nrm = Normalize(nrm);
        if (BUMP_MODE > SBUMP_FLAT)
            tan = Normalize(tan);
    }

#if BUMP_MODE > SBUMP_FLAT && PIXEL_NORMAL
    O.mtrx[0] = tan;
    O.mtrx[2] = nrm;
    O.mtrx[1] = vtx.bin(nrm, tan, HEIGHTMAP);
#elif BUMP_MODE >= SBUMP_FLAT && (PIXEL_NORMAL || TESSELATE)
    O.nrm = nrm;
#endif

#if VTX_REFLECT
    O.reflect_dir = ReflectDir(Normalize(pos), nrm);
#endif

//  per-vertex light
#if VTX_LIGHT
    {
        VecH total_lum;

        // AMBIENT
        if (HAS_AMBIENT && AMBIENT_IN_VTX)
            total_lum = AmbientNSColor;
        else
            total_lum = 0;

// LIGHTS
#if LIGHT_DIR
        {
            // diffuse
            Vec light_dir = LightDir.dir;
            Half lum = Sat(Dot(nrm, light_dir));
            total_lum += LightDir.color.rgb * lum;
        }
#endif

#if LIGHT_POINT
        {
            // distance
            Vec delta = LightPoint.pos - pos;
            Flt inv_dist2 = 1 / Length2(delta);
            Half lum = LightPointDist(inv_dist2);

            // diffuse
            Vec light_dir = delta * Sqrt(inv_dist2); // Normalize(delta);
            lum *= Sat(Dot(nrm, light_dir));
            total_lum += LightPoint.color.rgb * lum;
        }
#endif

#if LIGHT_LINEAR
        {
            // distance
            Vec delta = LightLinear.pos - pos;
            Flt dist = Length(delta);
            Half lum = LightLinearDist(dist);

            // diffuse
            Vec light_dir = delta / dist; // Normalize(delta);
            lum *= Sat(Dot(nrm, light_dir));
            total_lum += LightLinear.color.rgb * lum;
        }
#endif

#if LIGHT_CONE
        {
            // distance & angle
            Vec delta = LightCone.pos - pos;
            Vec dir = TransformTP(delta, LightCone.mtrx);
            if (dir.z > 0) {
                Flt dist = Length(delta);
                Half lum = LightConeAngle(dir.xy / dir.z) * LightConeDist(dist);

                // diffuse
                Vec light_dir = delta / dist; // Normalize(delta);
                lum *= Sat(Dot(nrm, light_dir));
                total_lum += LightCone.color.rgb * lum;
            }
        }
#endif

// STORE
#if LIGHT_IN_COL
        if (COLORS)
            O.col.rgb *= total_lum;
        else if (MATERIALS <= 1)
            O.col.rgb = total_lum * Material.color.rgb;
        else
            O.col.rgb = total_lum;
#else
        O.lum = total_lum;
#endif
    }
#endif

    vpos = Project(pos);
    CLIP_PLANE(pos);
#if SET_POS
    O.pos = pos;
#endif
}
/******************************************************************************/
// PS
/******************************************************************************/
VecH4 PS(
    Data I,
    PIXEL

#if PIXEL_NORMAL && FX != FX_GRASS_2D && FX != FX_LEAF_2D && FX != FX_LEAFS_2D
    ,
    IS_FRONT
#endif
    ) : TARGET {
#if ALPHA_TEST == ALPHA_TEST_DITHER
    VecH4 col;
#else
    VecH col;
#endif
#if PIXEL_NORMAL
    VecH nrmh;
#endif
    Half rough, reflect, glow;

#if SET_COL
    col = I.col;
#elif MATERIALS <= 1
#if ALPHA_TEST == ALPHA_TEST_DITHER
    col = Material.color;
#else
    col = Material.color.rgb;
#endif
#endif

#if MATERIALS == 1
    VecH4 det;
#if DETAIL
    det = GetDetail(I.uv);
#endif

// #MaterialTextureLayout
#if LAYOUT == 0
    {
        rough = Material.rough_add;
        reflect = Material.reflect_add;
        glow = Material.glow;
        if (DETAIL) {
            col.rgb *= det.DETAIL_CHANNEL_COLOR;
            APPLY_DETAIL_ROUGH(rough, det.DETAIL_CHANNEL_ROUGH);
        } // #MaterialTextureLayoutDetail
    }
#elif LAYOUT == 1
    {
        VecH4 tex_col = RTex(Col, I.uv);
        if (ALPHA_TEST) {
#if GRASS_FADE
            tex_col.a -= I.fade_out;
#endif
#if ALPHA_TEST == ALPHA_TEST_YES
            MaterialAlphaTest(tex_col.a);
#elif ALPHA_TEST == ALPHA_TEST_DITHER
            col.a *= tex_col.a;
            MaterialAlphaTestDither(col.a, pixel.xy, I.face_id);
#endif
        }
        col.rgb *= tex_col.rgb;
        rough = Material.rough_add;
        reflect = Material.reflect_add;
        glow = Material.glow;
        if (DETAIL) {
            col.rgb *= det.DETAIL_CHANNEL_COLOR;
            APPLY_DETAIL_ROUGH(rough, det.DETAIL_CHANNEL_ROUGH);
        } // #MaterialTextureLayoutDetail
    }
#elif LAYOUT == 2
    {
        VecH4 tex_col = RTex(Col, I.uv);
        if (ALPHA_TEST) {
#if GRASS_FADE
            tex_col.a -= I.fade_out;
#endif
#if ALPHA_TEST == ALPHA_TEST_YES
            MaterialAlphaTest(tex_col.a);
#elif ALPHA_TEST == ALPHA_TEST_DITHER
            col.a *= tex_col.a;
            MaterialAlphaTestDither(col.a, pixel.xy, I.face_id);
#endif
        }
        VecH4 tex_ext = RTex(Ext, I.uv);
        col.rgb *= tex_col.rgb;
        rough = tex_ext.BASE_CHANNEL_ROUGH * Material.rough_mul + Material.rough_add; // saturated later below
        reflect = tex_ext.BASE_CHANNEL_METAL * Material.reflect_mul + Material.reflect_add;
        glow = tex_ext.BASE_CHANNEL_GLOW * Material.glow;
        if (DETAIL) {
            col.rgb *= det.DETAIL_CHANNEL_COLOR;
            APPLY_DETAIL_ROUGH(rough, det.DETAIL_CHANNEL_ROUGH);
        } // #MaterialTextureLayoutDetail
    }
#endif

// normal
#if PIXEL_NORMAL
#if BUMP_MODE == SBUMP_ZERO
    nrmh = 0;
#elif BUMP_MODE == SBUMP_FLAT
    nrmh = I.Nrm(); // can't add DETAIL normal because it would need 'I.mtrx'
#else
#if 0
                      nrmh.xy =RTex(Nrm, I.uv).BASE_CHANNEL_NORMAL*Material.normal;
            if(DETAIL)nrmh.xy+=det.DETAIL_CHANNEL_NORMAL; // #MaterialTextureLayoutDetail
                      nrmh.z  =CalcZ(nrmh.xy);
#else
    nrmh.xy = RTex(Nrm, I.uv).BASE_CHANNEL_NORMAL;
    nrmh.z = CalcZ(nrmh.xy);
    nrmh.xy *= Material.normal;
    if (DETAIL)
        nrmh.xy += det.DETAIL_CHANNEL_NORMAL; // #MaterialTextureLayoutDetail
#endif
    nrmh = Transform(nrmh, I.mtrx);
#endif
#endif

#else // MATERIALS>1
    // on GeForce with Half support (GeForce 1150+) it was verified that these values can get outside of 0..1 range, which might cause infinite loop in Relief=crash and cause normals to be very big and after Normalization make them equal to 0,0,0 which later causes NaN on normalization in other shaders
    if (MATERIALS == 1)
        I.material.x = Sat(I.material.x);
    if (MATERIALS == 2)
        I.material.xy = Sat(I.material.xy);
    if (MATERIALS == 3)
        I.material.xyz = Sat(I.material.xyz);
    if (MATERIALS == 4)
        I.material.xyzw = Sat(I.material.xyzw);

    // assuming that in multi materials LAYOUT!=0
    Vec2 uv0, uv1, uv2, uv3;
    if (UV_SCALE) {
        uv0 = I.uv * MultiMaterial0.uv_scale;
        uv1 = I.uv * MultiMaterial1.uv_scale;
        if (MATERIALS >= 3)
            uv2 = I.uv * MultiMaterial2.uv_scale;
        if (MATERIALS >= 4)
            uv3 = I.uv * MultiMaterial3.uv_scale;
    } else {
        uv0 = I.uv;
        uv1 = I.uv;
        if (MATERIALS >= 3)
            uv2 = I.uv;
        if (MATERIALS >= 4)
            uv3 = I.uv;
    }

    // #MaterialTextureLayout #MaterialTextureLayoutDetail

    // detail texture
    VecH4 det0, det1, det2, det3;
    if (DETAIL) {
        det0 = GetDetail0(uv0);
        det1 = GetDetail1(uv1);
        if (MATERIALS >= 3)
            det2 = GetDetail2(uv2);
        if (MATERIALS >= 4)
            det3 = GetDetail3(uv3);
    }

    // macro texture
    // Half mac_blend; if(MACRO)mac_blend=LerpRS(MacroFrom, MacroTo, Length(I.pos))*MacroMax;

    // Reflect, Rough, Bump, Glow !! DO THIS FIRST because it may modify 'I.material' which affects everything !!
    VecH refl_rogh_glow;
    if (LAYOUT == 2) {
        VecH4 ext0, ext1, ext2, ext3;
        ext0 = RTex(Ext, uv0);
        ext1 = RTex(Ext1, uv1);
        if (MATERIALS >= 3)
            ext2 = RTex(Ext2, uv2);
        if (MATERIALS >= 4)
            ext3 = RTex(Ext3, uv3);
        if (MTRL_BLEND) {
            VecH4 mtrl;
            mtrl.x = MultiMaterialWeight(I.material.x, ext0.BASE_CHANNEL_BUMP);
            mtrl.y = MultiMaterialWeight(I.material.y, ext1.BASE_CHANNEL_BUMP);
            if (MATERIALS == 2) {
                Half sum = Sum(mtrl.xy);
                if (CanDiv(sum))
                    I.material.xy = mtrl.xy / sum;
            }
            if (MATERIALS >= 3) {
                mtrl.z = MultiMaterialWeight(I.material.z, ext2.BASE_CHANNEL_BUMP);
                if (MATERIALS == 3) {
                    Half sum = Sum(mtrl.xyz);
                    if (CanDiv(sum))
                        I.material.xyz = mtrl.xyz / sum;
                }
            }
            if (MATERIALS >= 4) {
                mtrl.w = MultiMaterialWeight(I.material.w, ext3.BASE_CHANNEL_BUMP);
                if (MATERIALS == 4) {
                    Half sum = Sum(mtrl.xyzw);
                    if (CanDiv(sum))
                        I.material.xyzw = mtrl.xyzw / sum;
                }
            }
        }
        {
            VecH refl_rogh_glow0 = ext0.xyw * MultiMaterial0.refl_rogh_glow_mul + MultiMaterial0.refl_rogh_glow_add;
            if (DETAIL)
                APPLY_DETAIL_ROUGH(refl_rogh_glow0.y, det0.DETAIL_CHANNEL_ROUGH);
            refl_rogh_glow = refl_rogh_glow0 * I.material.x;
        } // #MaterialTextureLayoutDetail
        {
            VecH refl_rogh_glow1 = ext1.xyw * MultiMaterial1.refl_rogh_glow_mul + MultiMaterial1.refl_rogh_glow_add;
            if (DETAIL)
                APPLY_DETAIL_ROUGH(refl_rogh_glow1.y, det1.DETAIL_CHANNEL_ROUGH);
            refl_rogh_glow += refl_rogh_glow1 * I.material.y;
        }
        if (MATERIALS >= 3) {
            VecH refl_rogh_glow2 = ext2.xyw * MultiMaterial2.refl_rogh_glow_mul + MultiMaterial2.refl_rogh_glow_add;
            if (DETAIL)
                APPLY_DETAIL_ROUGH(refl_rogh_glow2.y, det2.DETAIL_CHANNEL_ROUGH);
            refl_rogh_glow += refl_rogh_glow2 * I.material.z;
        }
        if (MATERIALS >= 4) {
            VecH refl_rogh_glow3 = ext3.xyw * MultiMaterial3.refl_rogh_glow_mul + MultiMaterial3.refl_rogh_glow_add;
            if (DETAIL)
                APPLY_DETAIL_ROUGH(refl_rogh_glow3.y, det3.DETAIL_CHANNEL_ROUGH);
            refl_rogh_glow += refl_rogh_glow3 * I.material.w;
        }
    } else {
        {
            VecH refl_rogh_glow0 = MultiMaterial0.refl_rogh_glow_add;
            if (DETAIL)
                APPLY_DETAIL_ROUGH(refl_rogh_glow0.y, det0.DETAIL_CHANNEL_ROUGH);
            refl_rogh_glow = refl_rogh_glow0 * I.material.x;
        } // #MaterialTextureLayoutDetail
        {
            VecH refl_rogh_glow1 = MultiMaterial1.refl_rogh_glow_add;
            if (DETAIL)
                APPLY_DETAIL_ROUGH(refl_rogh_glow1.y, det1.DETAIL_CHANNEL_ROUGH);
            refl_rogh_glow += refl_rogh_glow1 * I.material.y;
        }
        if (MATERIALS >= 3) {
            VecH refl_rogh_glow2 = MultiMaterial2.refl_rogh_glow_add;
            if (DETAIL)
                APPLY_DETAIL_ROUGH(refl_rogh_glow2.y, det2.DETAIL_CHANNEL_ROUGH);
            refl_rogh_glow += refl_rogh_glow2 * I.material.z;
        }
        if (MATERIALS >= 4) {
            VecH refl_rogh_glow3 = MultiMaterial3.refl_rogh_glow_add;
            if (DETAIL)
                APPLY_DETAIL_ROUGH(refl_rogh_glow3.y, det3.DETAIL_CHANNEL_ROUGH);
            refl_rogh_glow += refl_rogh_glow3 * I.material.w;
        }
    }
    rough = refl_rogh_glow.y;
    reflect = refl_rogh_glow.x;
    glow = refl_rogh_glow.z;

    // Color + Detail + Macro !! do this second after modifying 'I.material' !! here Alpha is ignored for multi-materials
    VecH rgb;
    {
        VecH col0 = RTex(Col, uv0).rgb;
        col0.rgb *= MultiMaterial0.color.rgb;
        if (DETAIL)
            col0.rgb *= det0.DETAIL_CHANNEL_COLOR; /*if(MACRO)col0.rgb=Lerp(col0.rgb, RTex(Mac , uv0*MacroScale).rgb, MultiMaterial0.macro*mac_blend);*/
        rgb = I.material.x * col0;
    } // #MaterialTextureLayoutDetail
    {
        VecH col1 = RTex(Col1, uv1).rgb;
        col1.rgb *= MultiMaterial1.color.rgb;
        if (DETAIL)
            col1.rgb *= det1.DETAIL_CHANNEL_COLOR; /*if(MACRO)col1.rgb=Lerp(col1.rgb, RTex(Mac1, uv1*MacroScale).rgb, MultiMaterial1.macro*mac_blend);*/
        rgb += I.material.y * col1;
    }
    if (MATERIALS >= 3) {
        VecH col2 = RTex(Col2, uv2).rgb;
        col2.rgb *= MultiMaterial2.color.rgb;
        if (DETAIL)
            col2.rgb *= det2.DETAIL_CHANNEL_COLOR; /*if(MACRO)col2.rgb=Lerp(col2.rgb, RTex(Mac2, uv2*MacroScale).rgb, MultiMaterial2.macro*mac_blend);*/
        rgb += I.material.z * col2;
    }
    if (MATERIALS >= 4) {
        VecH col3 = RTex(Col3, uv3).rgb;
        col3.rgb *= MultiMaterial3.color.rgb;
        if (DETAIL)
            col3.rgb *= det3.DETAIL_CHANNEL_COLOR; /*if(MACRO)col3.rgb=Lerp(col3.rgb, RTex(Mac3, uv3*MacroScale).rgb, MultiMaterial3.macro*mac_blend);*/
        rgb += I.material.w * col3;
    }
#if SET_COL
    col.rgb *= rgb.rgb;
#else
    col.rgb = rgb.rgb;
#endif

// normal
#if PIXEL_NORMAL
#if BUMP_MODE == SBUMP_ZERO
    nrmh = 0;
#elif BUMP_MODE == SBUMP_FLAT
    nrmh = I.Nrm(); // can't add DETAIL normal because it would need 'I.mtrx'
#else
    if (DETAIL) { // #MaterialTextureLayoutDetail
        nrmh.xy = (RTex(Nrm, uv0).BASE_CHANNEL_NORMAL * MultiMaterial0.normal + det0.DETAIL_CHANNEL_NORMAL) * I.material.x;
        nrmh.xy += (RTex(Nrm1, uv1).BASE_CHANNEL_NORMAL * MultiMaterial1.normal + det1.DETAIL_CHANNEL_NORMAL) * I.material.y;
        if (MATERIALS >= 3)
            nrmh.xy += (RTex(Nrm2, uv2).BASE_CHANNEL_NORMAL * MultiMaterial2.normal + det2.DETAIL_CHANNEL_NORMAL) * I.material.z;
        if (MATERIALS >= 4)
            nrmh.xy += (RTex(Nrm3, uv3).BASE_CHANNEL_NORMAL * MultiMaterial3.normal + det3.DETAIL_CHANNEL_NORMAL) * I.material.w;
    } else {
        nrmh.xy = RTex(Nrm, uv0).BASE_CHANNEL_NORMAL * (MultiMaterial0.normal * I.material.x);
        nrmh.xy += RTex(Nrm1, uv1).BASE_CHANNEL_NORMAL * (MultiMaterial1.normal * I.material.y);
        if (MATERIALS >= 3)
            nrmh.xy += RTex(Nrm2, uv2).BASE_CHANNEL_NORMAL * (MultiMaterial2.normal * I.material.z);
        if (MATERIALS >= 4)
            nrmh.xy += RTex(Nrm3, uv3).BASE_CHANNEL_NORMAL * (MultiMaterial3.normal * I.material.w);
    }
    nrmh.z = CalcZ(nrmh.xy);
    nrmh = Transform(nrmh, I.mtrx);
#endif
#endif

#endif // MATERIALS

    col.rgb += Highlight.rgb;
    if (LAYOUT == 2 || DETAIL)
        rough = Sat(rough); // need to saturate to avoid invalid values

#if PIXEL_NORMAL
#if FX != FX_GRASS_2D && FX != FX_LEAF_2D && FX != FX_LEAFS_2D
    BackFlip(nrmh, front);
#endif
    Vec nrm = Normalize(Vec(nrmh)); // normalize after converting to HP, needed for HQ specular
#endif

#if (LIGHT && PER_PIXEL) || REFLECT
    Vec eye_dir = Normalize(I.pos);
#endif

    Bool translucent = (FX == FX_GRASS_3D || FX == FX_LEAF_3D || FX == FX_LEAFS_3D);

    Vec2 jitter_value;
    if (SHADOW)
        jitter_value = ShadowJitter(pixel.xy);

    Half inv_metal = ReflectToInvMetal(reflect);
    VecH reflect_col = ReflectCol(reflect, col.rgb, inv_metal); // calc 'reflect_col' from unlit color

    // lighting
    VecH ambient;
    if (HAS_AMBIENT && !AMBIENT_IN_VTX)
        ambient = AmbientNSColor;
    else
        ambient = 0;

    VecH total_lum,
        total_specular = 0;

#if VTX_LIGHT // per-vertex
    {
        Half shadow;
#if SHADOW
        if (LIGHT_DIR_SHD)
            shadow = Sat(ShadowDirValue(I.pos, jitter_value, true, LIGHT_DIR_SHD_NUM, false));
        if (LIGHT_POINT_SHD)
            shadow = ShadowFinal(ShadowPointValue(I.pos, jitter_value, true));
        if (LIGHT_LINEAR_SHD)
            shadow = ShadowFinal(ShadowPointValue(I.pos, jitter_value, true));
        if (LIGHT_CONE_SHD)
            shadow = ShadowFinal(ShadowConeValue(I.pos, jitter_value, true));
#else
        shadow = 1;
#endif

        VecH light;
#if SET_LUM
        light = I.lum;
#else
        light = 1;
#endif

        total_lum = light * shadow + ambient;
    }
#else // per-pixel
    {
        total_lum = ambient;

#if LIGHT_DIR
        {
            // shadow
            Half shadow;
            if (LIGHT_DIR_SHD)
                shadow = Sat(ShadowDirValue(I.pos, jitter_value, true, LIGHT_DIR_SHD_NUM, false));

            // light
            Vec light_dir = LightDir.dir;
            LightParams lp;
            lp.set(nrm, light_dir);
            Half lum = lp.NdotL;
            if (SHADOW)
                lum *= shadow;
            if (translucent && -lum > EPS_LUM) {
                total_lum += LightDir.color.rgb * (lum * -TRANSLUCENT_VAL);
            }
            BRANCH if (lum > EPS_LUM) {
                // light #1
                lp.set(nrm, light_dir, eye_dir);

                VecH lum_rgb = LightDir.color.rgb * lum;
                total_lum += lum_rgb * lp.diffuse(rough);                                                          // diffuse
                total_specular += lum_rgb * lp.specular(rough, reflect, reflect_col, false, LightDir.radius_frac); // specular
            }
        }
#endif

#if LIGHT_POINT
        {
            // shadow
            Half shadow;
            if (LIGHT_POINT_SHD)
                shadow = ShadowFinal(ShadowPointValue(I.pos, jitter_value, true));

            // distance
            Vec delta = LightPoint.pos - I.pos;
            Flt inv_dist2 = 1 / Length2(delta);
            Half lum = LightPointDist(inv_dist2);
            if (LIGHT_POINT_SHD)
                lum *= shadow;

            // light
            Vec light_dir = delta * Sqrt(inv_dist2); // Normalize(delta);
            LightParams lp;
            lp.set(nrm, light_dir);
            lum *= lp.NdotL;
            if (translucent && -lum > EPS_LUM) {
                total_lum += LightPoint.color.rgb * (lum * -TRANSLUCENT_VAL);
            }
            BRANCH if (lum > EPS_LUM) {
                // light #1
                lp.set(nrm, light_dir, eye_dir);

                VecH lum_rgb = LightPoint.color.rgb * lum;
                total_lum += lum_rgb * lp.diffuse(rough);                                    // diffuse
                total_specular += lum_rgb * lp.specular(rough, reflect, reflect_col, false); // specular
            }
        }
#endif

#if LIGHT_LINEAR
        {
            // shadow
            Half shadow;
            if (LIGHT_LINEAR_SHD)
                shadow = ShadowFinal(ShadowPointValue(I.pos, jitter_value, true));

            // distance
            Vec delta = LightLinear.pos - I.pos;
            Flt dist = Length(delta);
            Half lum = LightLinearDist(dist);
            if (LIGHT_LINEAR_SHD)
                lum *= shadow;

            // light
            Vec light_dir = delta / dist; // Normalize(delta);
            LightParams lp;
            lp.set(nrm, light_dir);
            lum *= lp.NdotL;
            if (translucent && -lum > EPS_LUM) {
                total_lum += LightLinear.color.rgb * (lum * -TRANSLUCENT_VAL);
            }
            BRANCH if (lum > EPS_LUM) {
                // light #1
                lp.set(nrm, light_dir, eye_dir);

                VecH lum_rgb = LightLinear.color.rgb * lum;
                total_lum += lum_rgb * lp.diffuse(rough);                                    // diffuse
                total_specular += lum_rgb * lp.specular(rough, reflect, reflect_col, false); // specular
            }
        }
#endif

#if LIGHT_CONE
        {
            // shadow
            Half shadow;
            if (LIGHT_CONE_SHD)
                shadow = ShadowFinal(ShadowConeValue(I.pos, jitter_value, true));

            // distance & angle
            Vec delta = LightCone.pos - I.pos;
            Vec dir = TransformTP(delta, LightCone.mtrx);
            if (dir.z > 0) {
                Flt dist = Length(delta);
                Half lum = LightConeAngle(dir.xy / dir.z) * LightConeDist(dist);
                if (LIGHT_CONE_SHD)
                    lum *= shadow;

                // light
                Vec light_dir = delta / dist; // Normalize(delta);
                LightParams lp;
                lp.set(nrm, light_dir);
                lum *= lp.NdotL;
                if (translucent && -lum > EPS_LUM) {
                    total_lum += LightCone.color.rgb * (lum * -TRANSLUCENT_VAL);
                }
                BRANCH if (lum > EPS_LUM) {
                    // light #1
                    lp.set(nrm, light_dir, eye_dir);

                    VecH lum_rgb = LightCone.color.rgb * lum;
                    total_lum += lum_rgb * lp.diffuse(rough);                                    // diffuse
                    total_specular += lum_rgb * lp.specular(rough, reflect, reflect_col, false); // specular
                }
            }
        }
#endif
    }
#endif

    Half diffuse = Diffuse(inv_metal);
    if (FirstPass) // add all below only to the first pass
    {
#if REFLECT // reflection
        {
#if VTX_REFLECT
            Vec reflect_dir = I.reflect_dir;
#else
            Vec reflect_dir = ReflectDir(eye_dir, nrm);
#endif
            total_specular += ReflectTex(reflect_dir, rough) * EnvColor * ReflectEnv(rough, reflect, reflect_col, -Dot(nrm, eye_dir), false);
        }
#endif

        // glow
        ApplyGlow(glow, col.rgb, diffuse, total_specular);

        if (MATERIALS <= 1) // emissive, this should be done after 'ApplyGlow' because emissive glow should not be applied to base color
        {
#if EMISSIVE_MAP
            VecH emissive = RTex(Lum, I.uv).rgb;
            total_specular += Material.emissive * emissive;
            glow += Material.emissive_glow * Max(emissive);
#else
            total_specular += Material.emissive;
            glow += Material.emissive_glow;
#endif
        }
    } else {
        // glow
        ApplyGlow(glow, diffuse);

        /*if(MATERIALS<=1) // glow from emissive, not needed because final glow is ignored in secondary passes
          {
          #if EMISSIVE_MAP
             VecH emissive=RTex(Lum, I.uv).rgb;
             glow+=Material.emissive_glow*Max(emissive);
          #else
             glow+=Material.emissive_glow;
          #endif
          }*/
    }
    col.rgb = col.rgb * total_lum * diffuse + total_specular;

    return VecH4(col.rgb, glow);
}
/******************************************************************************/
// HULL / DOMAIN
/******************************************************************************/
#if TESSELATE
HSData HSConstant(InputPatch<Data, 3> I) { return GetHSData(I[0].pos, I[1].pos, I[2].pos, I[0].Nrm(), I[1].Nrm(), I[2].Nrm()); }
[maxtessfactor(5.0)]
    [domain("tri")]
    [partitioning("fractional_odd")] // use 'odd' because it supports range from 1.0 ('even' supports range from 2.0)
    [outputtopology("triangle_cw")]
    [patchconstantfunc("HSConstant")]
    [outputcontrolpoints(3)] Data
    HS(
        InputPatch<Data, 3> I,
        UInt cp_id
        : SV_OutputControlPointID) {
    Data O;
    O.pos = I[cp_id].pos;

#if BUMP_MODE > SBUMP_FLAT && PIXEL_NORMAL
    O.mtrx = I[cp_id].mtrx;
#else
    O.nrm = I[cp_id].nrm;
#endif

#if SET_UV
    O.uv = I[cp_id].uv;
#endif

#if MATERIALS > 1
    O.material = I[cp_id].material;
#endif

#if SET_COL
    O.col = I[cp_id].col;
#endif

#if GRASS_FADE
    O.fade_out = I[cp_id].fade_out;
#endif

#if VTX_REFLECT
    O.reflect_dir = I[cp_id].reflect_dir;
#endif

#if SET_LUM
    O.lum = I[cp_id].lum;
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
#if BUMP_MODE > SBUMP_FLAT && PIXEL_NORMAL
    O.mtrx[0] = I[0].mtrx[0] * B.z + I[1].mtrx[0] * B.x + I[2].mtrx[0] * B.y;
    O.mtrx[1] = I[0].mtrx[1] * B.z + I[1].mtrx[1] * B.x + I[2].mtrx[1] * B.y;
    SetDSPosNrm(O.pos, O.mtrx[2], I[0].pos, I[1].pos, I[2].pos, I[0].Nrm(), I[1].Nrm(), I[2].Nrm(), B, hs_data, false, 0);
#else
    SetDSPosNrm(O.pos, O.nrm, I[0].pos, I[1].pos, I[2].pos, I[0].Nrm(), I[1].Nrm(), I[2].Nrm(), B, hs_data, false, 0);
#endif

#if SET_UV
    O.uv = I[0].uv * B.z + I[1].uv * B.x + I[2].uv * B.y;
#endif

#if MATERIALS > 1
    O.material = I[0].material * B.z + I[1].material * B.x + I[2].material * B.y;
#endif

#if SET_COL
    O.col = I[0].col * B.z + I[1].col * B.x + I[2].col * B.y;
#endif

#if GRASS_FADE
    O.fade_out = I[0].fade_out * B.z + I[1].fade_out * B.x + I[2].fade_out * B.y;
#endif

#if VTX_REFLECT
    O.reflect_dir = I[0].reflect_dir * B.z + I[1].reflect_dir * B.x + I[2].reflect_dir * B.y;
#endif

#if SET_LUM
    O.lum = I[0].lum * B.z + I[1].lum * B.x + I[2].lum * B.y;
#endif

#if ALPHA_TEST == ALPHA_TEST_DITHER
    O.face_id = I[0].face_id;
#endif

    pixel = Project(O.pos);
}
#endif
/******************************************************************************/
