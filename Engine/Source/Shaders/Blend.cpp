/******************************************************************************/
#include "!Header.h"
#include "Sky.h"
/******************************************************************************
SKIN, COLORS, LAYOUT, BUMP_MODE, REFLECT, EMISSIVE_MAP
/******************************************************************************/
#ifndef PER_PIXEL
#define PER_PIXEL 1
#endif
#define HEIGHTMAP    0
#define TESSELATE    0
#define SET_UV       (LAYOUT || BUMP_MODE>SBUMP_FLAT || EMISSIVE_MAP)
#define VTX_REFLECT  (REFLECT && !PER_PIXEL && BUMP_MODE<=SBUMP_FLAT) // require !PER_PIXEL because even without normal maps (SBUMP_FLAT) the quality suffers
#define SET_POS      (REFLECT || TESSELATE)
#define PIXEL_NORMAL (REFLECT) // if calculate normal in the pixel shader
/******************************************************************************/
struct Data
{
#if SET_POS
   Vec pos:POS;
#endif

#if SET_UV
   Vec2 uv:UV;
#endif

   VecH4 color:COLOR;

#if   BUMP_MODE> SBUMP_FLAT && PIXEL_NORMAL
   centroid MatrixH3 mtrx:MATRIX; // !! may not be Normalized !! have to use 'centroid' to prevent values from getting outside of range, without centroid values can get MUCH different which might cause normals to be very big (very big vectors can't be normalized well, making them (0,0,0), which later causes NaN on normalization in other shaders)
   VecH Nrm() {return mtrx[2];}
#elif BUMP_MODE>=SBUMP_FLAT && (PIXEL_NORMAL || TESSELATE)
   centroid VecH nrm:NORMAL; // !! may not be Normalized !! have to use 'centroid' to prevent values from getting outside of range, without centroid values can get MUCH different which might cause normals to be very big (very big vectors can't be normalized well, making them (0,0,0), which later causes NaN on normalization in other shaders)
   VecH Nrm() {return nrm;}
#else
   VecH Nrm() {return 0;}
#endif

#if VTX_REFLECT
   Vec reflect_dir:REFLECTION;
#endif
};
void VS
(
   VtxInput vtx,

   out Data O,
   out Vec4 vpos:POSITION
)
{
#if SET_UV
   O.uv=vtx.uv();
#endif
             O.color =Material.color;
   if(COLORS)O.color*=vtx.colorFast();

   Vec  pos=vtx.pos();
   VecH nrm, tan; if(BUMP_MODE>=SBUMP_FLAT)nrm=vtx.nrm(); if(BUMP_MODE>SBUMP_FLAT)tan=vtx.tan(nrm, HEIGHTMAP);
   if(!SKIN)
   {
      if(true) // instance
      {
         pos=TransformPos(pos, vtx.instance());
      #if   BUMP_MODE> SBUMP_FLAT
         nrm=TransformDir(nrm, vtx.instance());
         tan=TransformDir(tan, vtx.instance());
      #elif BUMP_MODE==SBUMP_FLAT
         nrm=TransformDir(nrm, vtx.instance());
      #endif
      }else
      {
         pos=TransformPos(pos);
      #if   BUMP_MODE> SBUMP_FLAT
         nrm=TransformDir(nrm);
         tan=TransformDir(tan);
      #elif BUMP_MODE==SBUMP_FLAT
         nrm=TransformDir(nrm);
      #endif
      }
   }else
   {
      VecU4 bone    =vtx.bone  ();
      VecH4 weight_h=vtx.weight();
      pos=TransformPos(pos, bone, vtx.weight());
   #if   BUMP_MODE> SBUMP_FLAT
      nrm=TransformDir(nrm, bone, weight_h);
      tan=TransformDir(tan, bone, weight_h);
   #elif BUMP_MODE==SBUMP_FLAT
      nrm=TransformDir(nrm, bone, weight_h);
   #endif
   }

   // normalize (have to do all at the same time, so all have the same lengths)
   if(BUMP_MODE>SBUMP_FLAT // calculating binormal (this also covers the case when we have tangent from heightmap which is not Normalized)
   || VTX_REFLECT // per-vertex reflection
   || TESSELATE) // needed for tesselation
   {
                              nrm=Normalize(nrm);
      if(BUMP_MODE>SBUMP_FLAT)tan=Normalize(tan);
   }

#if   BUMP_MODE> SBUMP_FLAT && PIXEL_NORMAL
   O.mtrx[0]=tan;
   O.mtrx[2]=nrm;
   O.mtrx[1]=vtx.bin(nrm, tan, HEIGHTMAP);
#elif BUMP_MODE>=SBUMP_FLAT && (PIXEL_NORMAL || TESSELATE)
   O.nrm=nrm;
#endif

#if VTX_REFLECT
   O.reflect_dir=ReflectDir(Normalize(pos), nrm);
#endif

#if SET_POS
   O.pos=pos;
#endif
   vpos=Project(pos);

   O.color.a*=Sat(Length(pos)*SkyFracMulAdd.x + SkyFracMulAdd.y);
}
/******************************************************************************/
VecH4 PS
(
   Data I,
/*#if PIXEL_NORMAL && FX!=FX_GRASS_2D && FX!=FX_LEAF_2D && FX!=FX_LEAFS_2D
   IS_FRONT,
#endif*/
   out Half outAlpha:TARGET1 // #RTOutput.Blend
):TARGET
{
   Half rough, reflect;

   // #MaterialTextureLayout
#if LAYOUT==0
   rough  =Material.  rough_add;
   reflect=Material.reflect_add;
#elif LAYOUT==1
   I.color*=RTex(Col, I.uv);
   rough  =Material.  rough_add;
   reflect=Material.reflect_add;
#elif LAYOUT==2
    I.color*=RTex(Col, I.uv);
   VecH2 ext=RTex(Ext, I.uv).xy;
   rough  =Sat(ext.BASE_CHANNEL_ROUGH*Material.  rough_mul+Material.  rough_add); // need to saturate to avoid invalid values
   reflect=    ext.BASE_CHANNEL_METAL*Material.reflect_mul+Material.reflect_add ;
#endif

   // normal
#if PIXEL_NORMAL
   VecH nrmh;
   #if   BUMP_MODE==SBUMP_ZERO
      nrmh=0;
   #elif BUMP_MODE==SBUMP_FLAT
      nrmh=I.Nrm();
   #else
      #if 0
         nrmh.xy=RTex(Nrm, I.uv).BASE_CHANNEL_NORMAL*Material.normal;
         nrmh.z =CalcZ(nrmh.xy);
      #else
         nrmh.xy =RTex(Nrm, I.uv).BASE_CHANNEL_NORMAL;
         nrmh.z  =CalcZ(nrmh.xy);
         nrmh.xy*=Material.normal;
      #endif
         nrmh=Transform(nrmh, I.mtrx);
   #endif

 /*#if FX!=FX_GRASS_2D && FX!=FX_LEAF_2D && FX!=FX_LEAFS_2D
      BackFlip(nrmh, front);
   #endif*/

   Vec nrm=Normalize(Vec(nrmh)); // normalize after converting to HP, needed for HQ specular
#endif

   I.color.rgb+=Highlight.rgb;

   Half inv_metal  =ReflectToInvMetal(reflect);
   VecH reflect_col=ReflectCol       (reflect, I.color.rgb, inv_metal); // calc 'reflect_col' from unlit color

   I.color.rgb=I.color.rgb*Diffuse(inv_metal);

#if REFLECT // reflection
   Vec eye_dir=Normalize(I.pos);
   #if VTX_REFLECT
      Vec reflect_dir=I.reflect_dir;
   #else
      Vec reflect_dir=ReflectDir(eye_dir, nrm);
   #endif
   I.color.rgb+=ReflectTex(reflect_dir, rough)*EnvColor*ReflectEnv(rough, reflect, reflect_col, -Dot(nrm, eye_dir), false);
#endif

#if EMISSIVE_MAP
   VecH emissive=RTex(Lum, I.uv).rgb;
   I.color.rgb+=Material.emissive*emissive;
#else
   I.color.rgb+=Material.emissive;
#endif

   outAlpha=I.color.a;
   return   I.color;
}
/******************************************************************************/
