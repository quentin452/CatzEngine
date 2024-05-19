/******************************************************************************/
#include "!Header.h"
#include "Fog.h"
#include "Sky.h"
/******************************************************************************/
// SKIN, COLORS, LAYOUT, BUMP_MODE, ALPHA_TEST, ALPHA, REFLECT, EMISSIVE_MAP, UV_SCALE, FX, PER_PIXEL, SHADOW_MAPS, TESSELATE
#define NO_AMBIENT 0 // this could be set to 1 for Secondary Passes, if we would use this then we could remove 'FirstPass'
#define HAS_AMBIENT (!NO_AMBIENT)

#define HEIGHTMAP 0
#define LIGHT 1
#define SHADOW (SHADOW_MAPS > 0)
#define VTX_LIGHT (LIGHT && !PER_PIXEL)
#define AMBIENT_IN_VTX (VTX_LIGHT && !SHADOW)                                      // if stored per-vertex (in either 'vtx.col' or 'vtx.lum')
#define LIGHT_IN_COL (VTX_LIGHT && !DETAIL && (NO_AMBIENT || !SHADOW) && !REFLECT) // can't mix light with vtx.col when REFLECT because for reflections we need unlit color
#define FOG_IN_COL (!REFLECT)                                                      // can't mix fog with vtx.col when REFLECT because for reflections we need unlit color
#define USE_VEL ALPHA_TEST
#define SET_POS ((LIGHT && PER_PIXEL) || SHADOW || REFLECT || TESSELATE)
#define SET_UV (LAYOUT || DETAIL || EMISSIVE_MAP || BUMP_MODE > SBUMP_FLAT)
#define SET_LUM (VTX_LIGHT && !LIGHT_IN_COL)
#define SET_FOG (!FOG_IN_COL)
#define VTX_REFLECT (REFLECT && !PER_PIXEL && BUMP_MODE <= SBUMP_FLAT) // require !PER_PIXEL because even without normal maps (SBUMP_FLAT) the quality suffers
#define PIXEL_NORMAL ((PER_PIXEL && LIGHT) || REFLECT)                 // if calculate normal in the pixel shader
#define GRASS_FADE (FX == FX_GRASS_2D || FX == FX_GRASS_3D)
#define ALPHA_CLIP 0.5
/******************************************************************************/
struct Data {
#if SET_POS
    Vec pos : POS;
#endif

#if SET_UV
    Vec2 uv : UV;
#endif

    VecH4 col : COLOR;
    VecH col_add : COLOR_ADD;
#if SET_FOG
    Half fog_rev : FOG;
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

#if USE_VEL
    Vec projected_prev_pos_xyw : PREV_POS;
#endif

#if VTX_REFLECT
    Vec reflect_dir : REFLECTION;
#endif

#if SET_LUM
    VecH lum : LUM;
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
    Vec local_pos = vtx.pos(), view_pos, view_vel;
    VecH nrm, tan;
    if (BUMP_MODE >= SBUMP_FLAT)
        nrm = vtx.nrm();
    if (BUMP_MODE > SBUMP_FLAT)
        tan = vtx.tan(nrm, HEIGHTMAP);

    // VEL
    Vec local_pos_prev, view_pos_prev;
    if (USE_VEL)
        local_pos_prev = local_pos;

#if SET_UV
    O.uv = vtx.uv(HEIGHTMAP);
    if (UV_SCALE)
        O.uv *= Material.uv_scale;
#endif

    O.col = Material.color;
    if (COLORS)
        O.col *= vtx.colorFast();

    if (FX == FX_LEAF_2D || FX == FX_LEAF_3D) {
        if (BUMP_MODE > SBUMP_FLAT)
            BendLeaf(vtx.hlp(), local_pos, nrm, tan);
        else if (BUMP_MODE == SBUMP_FLAT)
            BendLeaf(vtx.hlp(), local_pos, nrm);
        else
            BendLeaf(vtx.hlp(), local_pos);
        if (USE_VEL)
            BendLeaf(vtx.hlp(), local_pos_prev, true);
    }
    if (FX == FX_LEAFS_2D || FX == FX_LEAFS_3D) {
        if (BUMP_MODE > SBUMP_FLAT)
            BendLeafs(vtx.hlp(), vtx.size(), local_pos, nrm, tan);
        else if (BUMP_MODE == SBUMP_FLAT)
            BendLeafs(vtx.hlp(), vtx.size(), local_pos, nrm);
        else
            BendLeafs(vtx.hlp(), vtx.size(), local_pos);
        if (USE_VEL)
            BendLeafs(vtx.hlp(), vtx.size(), local_pos_prev, true);
    }

    if (!SKIN) {
        if (true) // instance
        {
            view_pos = TransformPos(local_pos, vtx.instance());
            if (USE_VEL)
                view_pos_prev = TransformPosPrev(local_pos_prev, vtx.instance());

#if BUMP_MODE > SBUMP_FLAT
            nrm = TransformDir(nrm, vtx.instance());
            tan = TransformDir(tan, vtx.instance());
#elif BUMP_MODE == SBUMP_FLAT
            nrm = TransformDir(nrm, vtx.instance());
#endif

            if (FX == FX_GRASS_2D || FX == FX_GRASS_3D) {
                BendGrass(local_pos, view_pos, vtx.instance());
                if (USE_VEL)
                    BendGrass(local_pos_prev, view_pos_prev, vtx.instance(), true);
            }
            if (GRASS_FADE)
                O.col.a *= 1 - GrassFadeOut(vtx.instance());
        } else {
            view_pos = TransformPos(local_pos);
            if (USE_VEL)
                view_pos_prev = TransformPosPrev(local_pos_prev);

#if BUMP_MODE > SBUMP_FLAT
            nrm = TransformDir(nrm);
            tan = TransformDir(tan);
#elif BUMP_MODE == SBUMP_FLAT
            nrm = TransformDir(nrm);
#endif

            if (FX == FX_GRASS_2D || FX == FX_GRASS_3D) {
                BendGrass(local_pos, view_pos);
                if (USE_VEL)
                    BendGrass(local_pos_prev, view_pos_prev, 0, true);
            }
            if (GRASS_FADE)
                O.col.a *= 1 - GrassFadeOut();
        }
    } else {
        VecU4 bone = vtx.bone();
        VecH4 weight_h = vtx.weight();
        view_pos = TransformPos(local_pos, bone, vtx.weight());
        if (USE_VEL)
            view_pos_prev = TransformPosPrev(local_pos_prev, bone, vtx.weight());

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

    Flt dist = Length(view_pos);

#if VTX_REFLECT
    O.reflect_dir = ReflectDir(view_pos / dist, nrm);
#endif

    // sky
    O.col.a *= Sat(dist * SkyFracMulAdd.x + SkyFracMulAdd.y);

    // fog
    Half fog_rev = VisibleOpacity(FogDensity, dist);    // fog_rev=1-fog
    O.col_add = Lerp(FogColor, Highlight.rgb, fog_rev); //         1-fog
#if FOG_IN_COL
    O.col.rgb *= fog_rev; //       *=1-fog
#else
    O.fog_rev = fog_rev;
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
        total_lum += LightDir.color.rgb * Sat(Dot(nrm, LightDir.dir));

// STORE
#if LIGHT_IN_COL
        O.col.rgb *= total_lum;
#else
        O.lum = total_lum;
#endif
    }
#endif

    vpos = Project(view_pos);
#if SET_POS
    O.pos = view_pos;
#endif
#if USE_VEL
    O.projected_prev_pos_xyw = ProjectPrevXYW(view_pos_prev);
#endif
}
/******************************************************************************/
// PS
/******************************************************************************/
void PS(
    Data I,
#if USE_VEL
    PIXEL,
#endif
#if PIXEL_NORMAL && FX != FX_GRASS_2D && FX != FX_LEAF_2D && FX != FX_LEAFS_2D
    IS_FRONT,
#endif

    out VecH4 outCol
    : TARGET0, out Half outAlpha
    : TARGET1 // #RTOutput.Blend
#if USE_VEL
      ,
      out VecH4 outVel
    : TARGET2 // #RTOutput.Blend
#endif
    ) // #RTOutput
{
    Half rough, reflect, glow;

    // #MaterialTextureLayout
#if LAYOUT == 0
    rough = Material.rough_add;
    reflect = Material.reflect_add;
    glow = Material.glow;
#elif LAYOUT == 1
    VecH4 tex_col = RTex(Col, I.uv);
    if (ALPHA_TEST)
        clip(tex_col.a - ALPHA_CLIP);
    if (ALPHA)
        I.col *= tex_col;
    else
        I.col.rgb *= tex_col.rgb;
    rough = Material.rough_add;
    reflect = Material.reflect_add;
    glow = Material.glow;
#elif LAYOUT == 2
    VecH4 tex_col = RTex(Col, I.uv);
    if (ALPHA_TEST)
        clip(tex_col.a - ALPHA_CLIP);
    VecH4 tex_ext = RTex(Ext, I.uv);
    if (ALPHA)
        I.col *= tex_col;
    else
        I.col.rgb *= tex_col.rgb;
    rough = Sat(tex_ext.BASE_CHANNEL_ROUGH * Material.rough_mul + Material.rough_add); // need to saturate to avoid invalid values
    reflect = tex_ext.BASE_CHANNEL_METAL * Material.reflect_mul + Material.reflect_add;
    glow = tex_ext.BASE_CHANNEL_GLOW * Material.glow;
#endif

    // normal
#if PIXEL_NORMAL
    VecH nrmh;
#if BUMP_MODE == SBUMP_ZERO
    nrmh = 0;
#elif BUMP_MODE == SBUMP_FLAT
    nrmh = I.Nrm();
#else
#if 0
         nrmh.xy=RTex(Nrm, I.uv).BASE_CHANNEL_NORMAL*Material.normal;
         nrmh.z =CalcZ(nrmh.xy);
#else
    nrmh.xy = RTex(Nrm, I.uv).BASE_CHANNEL_NORMAL;
    nrmh.z = CalcZ(nrmh.xy);
    nrmh.xy *= Material.normal;
#endif
    nrmh = Transform(nrmh, I.mtrx);
#endif

#if FX != FX_GRASS_2D && FX != FX_LEAF_2D && FX != FX_LEAFS_2D
    BackFlip(nrmh, front);
#endif

    Vec nrm = Normalize(Vec(nrmh)); // normalize after converting to HP, needed for HQ specular
#endif

#if (LIGHT && PER_PIXEL) || REFLECT
    Vec eye_dir = Normalize(I.pos);
#endif

    Bool translucent = (FX == FX_GRASS_3D || FX == FX_LEAF_3D || FX == FX_LEAFS_3D);

    Half inv_metal = ReflectToInvMetal(reflect);
    VecH reflect_col = ReflectCol(reflect, I.col.rgb, inv_metal); // calc 'reflect_col' from unlit color

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
        shadow = Sat(ShadowDirValue(I.pos, 0, false, SHADOW_MAPS, false)); // for improved performance don't use shadow jittering because it's not much needed for blended objects
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

        // directional light
        {
            // shadow
            Half shadow;
            if (SHADOW)
                shadow = Sat(ShadowDirValue(I.pos, 0, false, SHADOW_MAPS, false));

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
    }
#endif

    Half diffuse = Diffuse(inv_metal);
    if (/*FirstPass; Blend Light is always 1 pass only */ 1) // add all below only to the first pass
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
        ApplyGlow(glow, I.col.rgb, diffuse, total_specular);

        // if(MATERIALS<=1) // emissive, this should be done after 'ApplyGlow' because emissive glow should not be applied to base color
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

        /*//if(MATERIALS<=1) glow from emissive, not needed because final glow is ignored in secondary passes
            {
            #if EMISSIVE_MAP
               VecH emissive=RTex(Lum, I.uv).rgb;
               glow+=Material.emissive_glow*Max(emissive);
            #else
               glow+=Material.emissive_glow;
            #endif
            }*/
    }
    I.col.rgb = I.col.rgb * total_lum * diffuse + total_specular;

#if SET_FOG
    I.col.rgb *= I.fog_rev;
#endif
    I.col.rgb += I.col_add; // add after lighting and reflection because this could have fog

    // can't output glow because we use alpha-blending here
    outCol = I.col;
    outAlpha = I.col.a;

#if USE_VEL
    outVel.xy = GetMotion(I.projected_prev_pos_xyw, pixel);
    outVel.z = 0;
    outVel.w = I.col.a; // alpha needed because of blending, Z needed because have to write all channels
#endif
}
/******************************************************************************/
