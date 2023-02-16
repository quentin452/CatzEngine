/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
#define MacroFrom   192.0
#define MacroTo     320.0
#define MacroMax    0.70
#define MacroScale (1.0/32)

#define PARALLAX_MODE 1 // 1=best

#define RELIEF_STEPS_MAX    32
#define RELIEF_STEPS_BINARY 3 // 3 works good in most cases, 4 could be used for absolute best quality
#define RELIEF_STEPS_MUL    0.75 // 0.75 gets slightly worse quality but better performance, 1.0 gets full quality but slower performance, default=0.75
#define RELIEF_LOD_OFFSET   0.33 // values >0 increase performance (by using fewer steps and smaller LODs) which also makes results more soft and flat helping to reduce jitter for distant pixels, default=0.33
#define RELIEF_TAN_POS      1 // 0=gets worse quality but better performance (not good for triangles with vertexes with very different normals or for surfaces very close to camera), 1=gets full quality but slower performance, default=1
#define RELIEF_DEC_NRM      1 // if reduce relief bump intensity where there are big differences between vtx normals, tangents and binormals, default=1
#define RELIEF_MODE         1 // 1=best
#define RELIEF_Z_LIMIT      0.4 // smaller values may cause leaking (UV swimming), and higher reduce bump intensity at angles, default=0.4

#define FAST_TPOS  ((BUMP_MODE>=SBUMP_PARALLAX_MIN && BUMP_MODE<=SBUMP_PARALLAX_MAX) || (BUMP_MODE==SBUMP_RELIEF && !RELIEF_TAN_POS))
#define GRASS_FADE (FX==FX_GRASS_2D || FX==FX_GRASS_3D)

#define USE_VEL 1
#define TESSELATE_VEL (TESSELATE && USE_VEL && 0) // FIXME

#define SET_POS (TESSELATE || MACRO || (!FAST_TPOS && BUMP_MODE>SBUMP_FLAT))
#define SET_UV  (LAYOUT || DETAIL || MACRO || BUMP_MODE>SBUMP_FLAT)
/******************************************************************************
SKIN, MATERIALS, LAYOUT, BUMP_MODE, ALPHA_TEST, DETAIL, MACRO, COLORS, MTRL_BLEND, HEIGHTMAP, FX, TESSELATE
/******************************************************************************/
struct Data
{
#if SET_UV
   Vec2 uv:UV;
#endif

#if SET_POS
   Vec pos:POS;
#endif

#if USE_VEL
   Vec projected_prev_pos_xyw:PREV_POS;
#endif

#if FAST_TPOS
   Vec _tpos:TPOS;
   Vec  tpos() {return Normalize(_tpos);}
#elif BUMP_MODE>SBUMP_FLAT
   Vec  tpos() {return Normalize(TransformTP(-pos, mtrx));} // need high precision here for 'TransformTP'
#else
   Vec  tpos() {return 0;}
#endif

#if MATERIALS>1
   centroid VecH4 material:MATERIAL; // have to use 'centroid' to prevent values from getting outside of 0..1 range, without centroid values can get MUCH different which might cause infinite loop in Relief=crash, and cause normals to be very big (very big vectors can't be normalized well, making them (0,0,0), which later causes NaN on normalization in other shaders)
#endif

#if COLORS
   #if ALPHA_TEST==ALPHA_TEST_DITHER
      VecH4 col:COLOR;
   #else
      VecH col:COLOR;
   #endif
#endif

#if GRASS_FADE
   Half fade_out:FADE_OUT;
#endif

#if   BUMP_MODE> SBUMP_FLAT
   centroid MatrixH3 mtrx:MATRIX; // !! may not be Normalized !! have to use 'centroid' to prevent values from getting outside of range, without centroid values can get MUCH different which might cause normals to be very big (very big vectors can't be normalized well, making them (0,0,0), which later causes NaN on normalization in other shaders)
   VecH Nrm() {return mtrx[2];}
#elif BUMP_MODE==SBUMP_FLAT
   centroid VecH nrm:NORMAL; // !! may not be Normalized !! have to use 'centroid' to prevent values from getting outside of range, without centroid values can get MUCH different which might cause normals to be very big (very big vectors can't be normalized well, making them (0,0,0), which later causes NaN on normalization in other shaders)
   VecH Nrm() {return nrm;}
#else
   VecH Nrm() {return 0;}
#endif

#if TESSELATE_VEL
   centroid VecH nrm_prev:PREV_NORMAL;
#endif

#if ALPHA_TEST==ALPHA_TEST_DITHER
   NOINTERP VecU2 face_id:FACE_ID;
#endif
};
/******************************************************************************/
// VS
/******************************************************************************/
void VS
(
   VtxInput vtx,

   out Data O,
   out Vec4 vpos:POSITION,

   CLIP_DIST
)
{
   Vec  local_pos=vtx.pos(), view_pos;
   VecH nrm, tan; if(BUMP_MODE>=SBUMP_FLAT)nrm=vtx.nrm(); if(BUMP_MODE>SBUMP_FLAT)tan=vtx.tan(nrm, HEIGHTMAP);

   // VEL
   Vec local_pos_prev, view_pos_prev; VecH nrm_prev; if(USE_VEL){local_pos_prev=local_pos; if(TESSELATE_VEL)nrm_prev=nrm;}

#if SET_UV
   O.uv=vtx.uv(HEIGHTMAP);
   if(HEIGHTMAP && MATERIALS==1)O.uv*=Material.uv_scale;
#endif

#if ALPHA_TEST==ALPHA_TEST_DITHER
   O.face_id=vtx.faceID();
#endif

#if MATERIALS>1
   O.material=vtx.material();
#endif

#if COLORS
   #if ALPHA_TEST==ALPHA_TEST_DITHER
      if(MATERIALS<=1)O.col=vtx.colorFast()*Material.color;
      else            O.col=vtx.colorFast();
   #else
      if(MATERIALS<=1)O.col=vtx.colorFast3()*Material.color.rgb;
      else            O.col=vtx.colorFast3();
   #endif
#endif

   if(FX==FX_LEAF_2D || FX==FX_LEAF_3D)
   {
      if(BUMP_MODE> SBUMP_FLAT)BendLeaf(vtx.hlp(), local_pos, nrm, tan);else
      if(BUMP_MODE==SBUMP_FLAT)BendLeaf(vtx.hlp(), local_pos, nrm     );else
                               BendLeaf(vtx.hlp(), local_pos          );
      if(USE_VEL)
      {
         if(TESSELATE_VEL)BendLeaf(vtx.hlp(), local_pos_prev, nrm_prev, true);else
                          BendLeaf(vtx.hlp(), local_pos_prev          , true);
      }
   }
   if(FX==FX_LEAFS_2D || FX==FX_LEAFS_3D)
   {
      if(BUMP_MODE> SBUMP_FLAT)BendLeafs(vtx.hlp(), vtx.size(), local_pos, nrm, tan);else
      if(BUMP_MODE==SBUMP_FLAT)BendLeafs(vtx.hlp(), vtx.size(), local_pos, nrm     );else
                               BendLeafs(vtx.hlp(), vtx.size(), local_pos          );
      if(USE_VEL)
      {
         if(TESSELATE_VEL)BendLeafs(vtx.hlp(), vtx.size(), local_pos_prev, nrm_prev, true);else
                          BendLeafs(vtx.hlp(), vtx.size(), local_pos_prev          , true);
      }
   }

   if(!SKIN)
   {
      if(true) // instance
      {
                    view_pos     =TransformPos    (local_pos     , vtx.instance());
         if(USE_VEL)view_pos_prev=TransformPosPrev(local_pos_prev, vtx.instance());

      #if   BUMP_MODE> SBUMP_FLAT
         O.mtrx[2]=TransformDir(nrm, vtx.instance());
         O.mtrx[0]=TransformDir(tan, vtx.instance());
      #elif BUMP_MODE==SBUMP_FLAT
         O.nrm    =TransformDir(nrm, vtx.instance());
      #endif
      #if TESSELATE_VEL
         O.nrm_prev=TransformDirPrev(nrm_prev, vtx.instance());
      #endif

         if(FX==FX_GRASS_2D || FX==FX_GRASS_3D)
         {
                       BendGrass(local_pos     , view_pos     , vtx.instance());
            if(USE_VEL)BendGrass(local_pos_prev, view_pos_prev, vtx.instance(), true);
         }
      #if GRASS_FADE
         O.fade_out=GrassFadeOut(vtx.instance());
      #endif
      }else
      {
                    view_pos     =TransformPos    (local_pos);
         if(USE_VEL)view_pos_prev=TransformPosPrev(local_pos_prev);

      #if   BUMP_MODE> SBUMP_FLAT
         O.mtrx[2]=TransformDir(nrm);
         O.mtrx[0]=TransformDir(tan);
      #elif BUMP_MODE==SBUMP_FLAT
         O.nrm    =TransformDir(nrm);
      #endif
      #if TESSELATE_VEL
         O.nrm_prev=TransformDirPrev(nrm_prev);
      #endif

         if(FX==FX_GRASS_2D || FX==FX_GRASS_3D)
         {
                       BendGrass(local_pos     , view_pos);
            if(USE_VEL)BendGrass(local_pos_prev, view_pos_prev, 0, true);
         }
      #if GRASS_FADE
         O.fade_out=GrassFadeOut();
      #endif
      }
   }else
   {
      VecU4 bone    =vtx.bone  ();
      VecH4 weight_h=vtx.weight();
                 view_pos     =TransformPos    (local_pos     , bone, vtx.weight());
      if(USE_VEL)view_pos_prev=TransformPosPrev(local_pos_prev, bone, vtx.weight());

   #if   BUMP_MODE> SBUMP_FLAT
      O.mtrx[2]=TransformDir(nrm, bone, weight_h);
      O.mtrx[0]=TransformDir(tan, bone, weight_h);
   #elif BUMP_MODE==SBUMP_FLAT
      O.nrm    =TransformDir(nrm, bone, weight_h);
   #endif
   #if TESSELATE_VEL
      O.nrm_prev=TransformDirPrev(nrm_prev, bone, weight_h);
   #endif
   }

   // normalize (have to do all at the same time, so all have the same lengths)
   if(BUMP_MODE> SBUMP_FLAT // calculating binormal (this also covers the case when we have tangent from heightmap which is not Normalized)
   || BUMP_MODE==SBUMP_RELIEF && RELIEF_DEC_NRM // needed for RELIEF_DEC_NRM effect
   || TESSELATE) // needed for tesselation
   {
   #if   BUMP_MODE> SBUMP_FLAT
      O.mtrx[2]=Normalize(O.mtrx[2]);
      O.mtrx[0]=Normalize(O.mtrx[0]);
   #elif BUMP_MODE==SBUMP_FLAT
      O.nrm    =Normalize(O.nrm);
   #endif
   #if TESSELATE_VEL
      O.nrm_prev=Normalize(O.nrm_prev);
   #endif
   }

#if BUMP_MODE>SBUMP_FLAT
   O.mtrx[1]=vtx.bin(O.mtrx[2], O.mtrx[0], HEIGHTMAP);
#endif

#if FAST_TPOS
   O._tpos=TransformTP(-view_pos, O.mtrx); // need high precision here, we can't Normalize because it's important to keep distances
#endif

#if SET_POS
   O.pos=view_pos;
#endif
   vpos=Project(view_pos); CLIP_PLANE(view_pos);
#if USE_VEL
   O.projected_prev_pos_xyw=ProjectPrevXYW(view_pos_prev);
#endif
}
/******************************************************************************/
// PS
/******************************************************************************/
//Half Pixels(VecH2 uv, Vec2 ImgSize, Flt lod) {return  Length(uv*ImgSize) /exp2(lod);} // how many pixels, exp2(lod)=Pow(2, lod)
  Half Pixels(VecH2 uv, Vec2 ImgSize, Flt lod) {return Max(Abs(uv*ImgSize))/exp2(lod);} // how many pixels, exp2(lod)=Pow(2, lod)

void PS
(
   Data I
#if USE_VEL || ALPHA_TEST==ALPHA_TEST_DITHER
 , PIXEL
#endif
#if FX!=FX_GRASS_2D && FX!=FX_LEAF_2D && FX!=FX_LEAFS_2D
 , IS_FRONT
#endif
 , out DeferredOutput output
)
{
#if ALPHA_TEST==ALPHA_TEST_DITHER
   VecH4 col;
#else
   VecH  col;
#endif
   VecH nrm;
   Half rough, reflect, glow;

#if COLORS
   col=I.col;
#elif MATERIALS<=1
   #if ALPHA_TEST==ALPHA_TEST_DITHER
      col=Material.color;
   #else
      col=Material.color.rgb;
   #endif
#endif

#if MATERIALS==1
   // apply uv coord bump offset
   #if BUMP_MODE>=SBUMP_PARALLAX_MIN && BUMP_MODE<=SBUMP_PARALLAX_MAX // Parallax
   {
      const Int steps=BUMP_MODE-SBUMP_PARALLAX0;

      VecH tpos=I.tpos();
   #if   PARALLAX_MODE==0 // too flat
      Half scale=Material.bump/steps;
   #elif PARALLAX_MODE==1 // best results (not as flat, and not much aliasing)
      Half scale=Material.bump/(steps*Lerp(1, tpos.z, tpos.z)); // Material.bump/steps/Lerp(1, tpos.z, tpos.z);
   #elif PARALLAX_MODE==2 // generates too steep walls (not good for parallax)
      Half scale=Material.bump/(steps*Lerp(1, tpos.z, Sat(tpos.z/0.5))); // Material.bump/steps/Lerp(1, tpos.z, tpos.z);
   #elif PARALLAX_MODE==3 // introduces a bit too much aliasing/artifacts on surfaces perpendicular to view direction
      Half scale=Material.bump/steps*(2-tpos.z); // Material.bump/steps*Lerp(1, 1/tpos.z, tpos.z)
   #else // correct however introduces way too much aliasing/artifacts on surfaces perpendicular to view direction
      Half scale=Material.bump/(steps*tpos.z);
   #endif
      tpos.xy*=scale; VecH2 add=-0.5*tpos.xy;
      UNROLL for(Int i=0; i<steps; i++)I.uv+=RTex(BUMP_IMAGE, I.uv).BASE_CHANNEL_BUMP*tpos.xy+add; // (tex-0.5)*tpos.xy = tex*tpos.xy + -0.5*tpos.xy
   }
   #elif BUMP_MODE==SBUMP_RELIEF // Relief
   {
      VecH tpos=I.tpos();
   #if   RELIEF_MODE==0
      Half scale=Material.bump;
   #elif RELIEF_MODE==1 // best
      Half scale=Material.bump/Lerp(1, tpos.z, Sat(tpos.z/RELIEF_Z_LIMIT));
   #elif RELIEF_MODE==2 // produces slight aliasing/artifacts on surfaces perpendicular to view direction
      Half scale=Material.bump/Max(tpos.z, RELIEF_Z_LIMIT);
   #else // correct however introduces way too much aliasing/artifacts on surfaces perpendicular to view direction
      Half scale=Material.bump/tpos.z;
   #endif

   #if RELIEF_DEC_NRM
      scale*=Length2(I.mtrx[0])*Length2(I.mtrx[1])*Length2(I.mtrx[2]); // vtx matrix vectors are interpolated linearly, which means that if there are big differences between vtx vectors, then their length will be smaller and smaller, for example if #0 vtx normal is (1,0), and #1 vtx normal is (0,1), then interpolated value between them will be (0.5, 0.5)
   #endif
      tpos.xy*=-scale;

      I.uv-=tpos.xy*0.5;

      Vec2 TexSize; BUMP_IMAGE.GetDimensions(TexSize.x, TexSize.y);
      Flt  lod=Max(0, GetLod(I.uv, TexSize)+RELIEF_LOD_OFFSET); // yes, can be negative, so use Max(0) to avoid increasing number of steps when surface is close to camera
    //lod=Trunc(lod); don't do this as it would reduce performance and generate more artifacts, with this disabled, we generate fewer steps gradually, and blend with the next MIP level softening results

      Half length=Pixels(tpos.xy, TexSize, lod);
      if(RELIEF_STEPS_MUL!=1)if(lod>0)length*=RELIEF_STEPS_MUL; // don't use this for first LOD

      Half steps  =Mid(length, 1, RELIEF_STEPS_MAX),
           stp    =1.0/steps,
           ray    =1;
      Vec2 uv_step=tpos.xy*stp; // keep as HP to avoid conversions several times in the loop below

   #if 1 // linear + interval search (faster)
      // linear search
      Half height_next, height_prev=0.5; // use 0.5 as approximate average value, we could do "RTexLodI(BUMP_IMAGE, I.uv, lod).BASE_CHANNEL_BUMP", however in tests that wasn't needed but only reduced performance
      LOOP for(;;)
      {
         ray -=stp;
         I.uv+=uv_step;
         height_next=RTexLodI(BUMP_IMAGE, I.uv, lod).BASE_CHANNEL_BUMP;
         if(height_next>=ray)break;
            height_prev=height_next;
      }

      // interval search
      if(1)
      {
         Half ray_prev=ray+stp;
         // prev pos: I.uv-uv_step, height_prev-ray_prev
         // next pos: I.uv        , height_next-ray
         Half hn=height_next-ray,
              hp=height_prev-ray_prev,
              frac=Sat(hn/(hn-hp));
         I.uv-=uv_step*frac;

         BRANCH if(lod<=0) // extra step (needed only for closeup)
         {
            Half ray_cur=ray+stp*frac,
                 height_cur=RTexLodI(BUMP_IMAGE, I.uv, lod).BASE_CHANNEL_BUMP;
            if(  height_cur>=ray_cur) // if still below, then have to go back more, lerp between this position and prev pos
            {
               // prev pos: I.uv-uv_step (BUT I.uv before adjustment), height_prev-ray_prev
               // next pos: I.uv                                     , height_cur -ray_cur
               uv_step*=1-frac; // we've just travelled "uv_step*frac", so to go to the prev point, we need what's left, "uv_step*(1-frac)"
            }else // we went back too far, go forward, lerp between next pos and this position
            {
               // prev pos: I.uv                             , height_cur -ray_cur
               // next pos: I.uv (BUT I.uv before adjustment), height_next-ray
               hp=hn;
               uv_step*=-frac; // we've just travelled "uv_step*frac", so to go to the next point, we need the same way but other direction, "uv_step*-frac"
            }
            hn=height_cur-ray_cur;
            frac=Sat(hn/(hn-hp));
            I.uv-=uv_step*frac;
         }
      }
   #else // linear + binary search (slower because requires 3 tex reads in binary to get the same results as with only 0-1 tex reads in interval)
      // linear search
      LOOP for(Int i=0; ; i++)
      {
         ray -=stp;
         I.uv+=uv_step;
         if(i>=steps || RTexLodI(BUMP_IMAGE, I.uv, lod).BASE_CHANNEL_BUMP>=ray)break;
      }

      // binary search
      {
         Half ray_prev=ray+stp,
              l=0, r=1, m=0.5;
         UNROLL for(Int i=0; i<RELIEF_STEPS_BINARY; i++)
         {
            Half height=RTexLodI(BUMP_IMAGE, I.uv-uv_step*m, lod).BASE_CHANNEL_BUMP;
            if(  height>Lerp(ray, ray_prev, m))l=m;else r=m;
            m=Avg(l, r);
         }
         I.uv-=uv_step*m;
      }
   #endif
   }
   #endif

   VecH4 det;
#if DETAIL
   det=GetDetail(I.uv);
#endif

   // #MaterialTextureLayout
   #if LAYOUT==0
   {
      rough  =Material.  rough_add;
      reflect=Material.reflect_add;
      glow   =Material.glow;
      if(DETAIL){col.rgb*=det.DETAIL_CHANNEL_COLOR; APPLY_DETAIL_ROUGH(rough, det.DETAIL_CHANNEL_ROUGH);} // #MaterialTextureLayoutDetail
   }
   #elif LAYOUT==1
   {
      VecH4 tex_col=RTex(Col, I.uv);
      if(ALPHA_TEST)
      {
      #if GRASS_FADE
         tex_col.a-=I.fade_out;
      #endif
      #if ALPHA_TEST==ALPHA_TEST_YES
         MaterialAlphaTest(tex_col.a);
      #elif ALPHA_TEST==ALPHA_TEST_DITHER
         col.a*=tex_col.a; MaterialAlphaTestDither(col.a, pixel.xy, I.face_id);
      #endif
      }
      col.rgb*=tex_col.rgb;
      rough   =Material.  rough_add;
      reflect =Material.reflect_add;
      glow    =Material.glow;
      if(DETAIL){col.rgb*=det.DETAIL_CHANNEL_COLOR; APPLY_DETAIL_ROUGH(rough, det.DETAIL_CHANNEL_ROUGH);} // #MaterialTextureLayoutDetail
   }
   #elif LAYOUT==2
   {
      VecH4 tex_col=RTex(Col, I.uv);
      if(ALPHA_TEST)
      {
      #if GRASS_FADE
         tex_col.a-=I.fade_out;
      #endif
      #if ALPHA_TEST==ALPHA_TEST_YES
         MaterialAlphaTest(tex_col.a);
      #elif ALPHA_TEST==ALPHA_TEST_DITHER
         col.a*=tex_col.a; MaterialAlphaTestDither(col.a, pixel.xy, I.face_id);
      #endif
      }
      VecH4 tex_ext=RTex(Ext, I.uv);
      col.rgb*=tex_col.rgb;
      rough   =tex_ext.BASE_CHANNEL_ROUGH*Material.  rough_mul+Material.  rough_add; // no need to saturate because we store this value in 0..1 RT
      reflect =tex_ext.BASE_CHANNEL_METAL*Material.reflect_mul+Material.reflect_add;
      glow    =tex_ext.BASE_CHANNEL_GLOW *Material.glow;
      if(DETAIL){col.rgb*=det.DETAIL_CHANNEL_COLOR; APPLY_DETAIL_ROUGH(rough, det.DETAIL_CHANNEL_ROUGH);} // #MaterialTextureLayoutDetail
   }
   #endif

#if MACRO
   col.rgb=Lerp(col.rgb, RTex(Mac, I.uv*MacroScale).rgb, LerpRS(MacroFrom, MacroTo, Length(I.pos))*MacroMax);
#endif

   // normal
#if   BUMP_MODE==SBUMP_ZERO
   nrm=VecH(0, 0, -1);
#elif BUMP_MODE==SBUMP_FLAT
   nrm=I.Nrm(); // can't add DETAIL normal because it would need 'I.mtrx'
#else
   #if 0 // lower quality, but compatible with multi-materials
                nrm.xy =RTex(Nrm, I.uv).BASE_CHANNEL_NORMAL*Material.normal;
      if(DETAIL)nrm.xy+=det.DETAIL_CHANNEL_NORMAL; // #MaterialTextureLayoutDetail
                nrm.z  =CalcZ(nrm.xy);
   #else // better quality
                nrm.xy =RTex(Nrm, I.uv).BASE_CHANNEL_NORMAL;
                nrm.z  =CalcZ(nrm.xy);
                nrm.xy*=Material.normal; // alternatively this could be "nrm.z*=Material.normal_inv", with "normal_inv=1/Max(normal, HALF_EPS)" to avoid div by 0 and also big numbers which would be problematic for Halfs, however this would make detail nrm unproportional (too big/small compared to base nrm)
      if(DETAIL)nrm.xy+=det.DETAIL_CHANNEL_NORMAL; // #MaterialTextureLayoutDetail
   #endif
      nrm=Transform(nrm, I.mtrx);
#endif

#else // MATERIALS>1
   // on GeForce with Half support (GeForce 1150+) it was verified that these values can get outside of 0..1 range, which might cause infinite loop in Relief=crash and cause normals to be very big and after Normalization make them equal to 0,0,0 which later causes NaN on normalization in other shaders
   if(MATERIALS==1)I.material.x   =Sat(I.material.x   );
   if(MATERIALS==2)I.material.xy  =Sat(I.material.xy  );
   if(MATERIALS==3)I.material.xyz =Sat(I.material.xyz );
   if(MATERIALS==4)I.material.xyzw=Sat(I.material.xyzw);

   // assuming that in multi materials LAYOUT!=0
   Vec2 uv0, uv1, uv2, uv3;
                   uv0=I.uv*MultiMaterial0.uv_scale;
                   uv1=I.uv*MultiMaterial1.uv_scale;
   if(MATERIALS>=3)uv2=I.uv*MultiMaterial2.uv_scale;
   if(MATERIALS>=4)uv3=I.uv*MultiMaterial3.uv_scale;

   // apply uv coord bump offset
   #if BUMP_MODE>=SBUMP_PARALLAX_MIN && BUMP_MODE<=SBUMP_PARALLAX_MAX // Parallax
   {
      const Int steps=BUMP_MODE-SBUMP_PARALLAX0;

      VecH tpos=I.tpos();
   #if   PARALLAX_MODE==0 // too flat
      Half scale=(1.0/steps);
   #elif PARALLAX_MODE==1 // best results (not as flat, and not much aliasing)
      Half scale=(1.0/steps)/Lerp(1, tpos.z, tpos.z);
   #elif PARALLAX_MODE==2 // generates too steep walls (not good for parallax)
      Half scale=(1.0/steps)/Lerp(1, tpos.z, Sat(tpos.z/0.5));
   #elif PARALLAX_MODE==3 // introduces a bit too much aliasing/artifacts on surfaces perpendicular to view direction
      Half scale=(1.0/steps)*(2-tpos.z); // 1/steps*Lerp(1, 1/tpos.z, tpos.z)
   #else // correct however introduces way too much aliasing/artifacts on surfaces perpendicular to view direction
      Half scale=(1.0/steps)/tpos.z;
   #endif
      tpos.xy*=scale;

      // h=(tex-0.5)*bump_mul = x*bump_mul - 0.5*bump_mul
      VecH4 bump_mul; bump_mul.x=MultiMaterial0.bump;
      VecH4 bump_add; bump_mul.y=MultiMaterial1.bump; if(MATERIALS==2){bump_mul.xy  *=I.material.xy  ; bump_add.xy  =bump_mul.xy  *-0.5;}
      if(MATERIALS>=3)bump_mul.z=MultiMaterial2.bump; if(MATERIALS==3){bump_mul.xyz *=I.material.xyz ; bump_add.xyz =bump_mul.xyz *-0.5;}
      if(MATERIALS>=4)bump_mul.w=MultiMaterial3.bump; if(MATERIALS==4){bump_mul.xyzw*=I.material.xyzw; bump_add.xyzw=bump_mul.xyzw*-0.5;}

      UNROLL for(Int i=0; i<steps; i++) // I.uv+=h*tpos.xy;
      {
                    Half h =RTex(       BUMP_IMAGE    , uv0).BASE_CHANNEL_BUMP*bump_mul.x+bump_add.x;
                         h+=RTex(CONCAT(BUMP_IMAGE, 1), uv1).BASE_CHANNEL_BUMP*bump_mul.y+bump_add.y;
         if(MATERIALS>=3)h+=RTex(CONCAT(BUMP_IMAGE, 2), uv2).BASE_CHANNEL_BUMP*bump_mul.z+bump_add.z;
         if(MATERIALS>=4)h+=RTex(CONCAT(BUMP_IMAGE, 3), uv3).BASE_CHANNEL_BUMP*bump_mul.w+bump_add.w;

         Vec2 offset=h*tpos.xy; // keep as HP to avoid multiple conversions below

                         uv0+=offset;
                         uv1+=offset;
         if(MATERIALS>=3)uv2+=offset;
         if(MATERIALS>=4)uv3+=offset;
      }
   }
   #elif BUMP_MODE==SBUMP_RELIEF // Relief
   {
      VecH4 bump_mul; bump_mul.x=MultiMaterial0.bump; Half avg_bump;
                      bump_mul.y=MultiMaterial1.bump; if(MATERIALS==2){bump_mul.xy  *=I.material.xy  ; avg_bump=Sum(bump_mul.xy  );} // use 'Sum' because they're premultipled by 'I.material'
      if(MATERIALS>=3)bump_mul.z=MultiMaterial2.bump; if(MATERIALS==3){bump_mul.xyz *=I.material.xyz ; avg_bump=Sum(bump_mul.xyz );}
      if(MATERIALS>=4)bump_mul.w=MultiMaterial3.bump; if(MATERIALS==4){bump_mul.xyzw*=I.material.xyzw; avg_bump=Sum(bump_mul.xyzw);}

      VecH tpos=I.tpos();
   #if   RELIEF_MODE==0
      Half scale=avg_bump;
   #elif RELIEF_MODE==1 // best
      Half scale=avg_bump/Lerp(1, tpos.z, Sat(tpos.z/RELIEF_Z_LIMIT));
   #elif RELIEF_MODE==2 // produces slight aliasing/artifacts on surfaces perpendicular to view direction
      Half scale=avg_bump/Max(tpos.z, RELIEF_Z_LIMIT);
   #else // correct however introduces way too much aliasing/artifacts on surfaces perpendicular to view direction
      Half scale=avg_bump/tpos.z;
   #endif

   #if RELIEF_DEC_NRM
      scale*=Length2(I.mtrx[0])*Length2(I.mtrx[1])*Length2(I.mtrx[2]); // vtx matrix vectors are interpolated linearly, which means that if there are big differences between vtx vectors, then their length will be smaller and smaller, for example if #0 vtx normal is (1,0), and #1 vtx normal is (0,1), then interpolated value between them will be (0.5, 0.5)
   #endif
      tpos.xy*=-scale;

      //I.uv-=tpos.xy*0.5;
      Vec2 offset=tpos.xy*0.5; // keep as HP to avoid multiple conversions below
                      uv0-=offset;
                      uv1-=offset;
      if(MATERIALS>=3)uv2-=offset;
      if(MATERIALS>=4)uv3-=offset;

      Vec2 TexSize0; Flt lod0;                 {       BUMP_IMAGE    .GetDimensions(TexSize0.x, TexSize0.y); lod0=Max(0, GetLod(uv0, TexSize0)+RELIEF_LOD_OFFSET);} // yes, can be negative, so use Max(0) to avoid increasing number of steps when surface is close to camera
      Vec2 TexSize1; Flt lod1;                 {CONCAT(BUMP_IMAGE, 1).GetDimensions(TexSize1.x, TexSize1.y); lod1=Max(0, GetLod(uv1, TexSize1)+RELIEF_LOD_OFFSET);}
      Vec2 TexSize2; Flt lod2; if(MATERIALS>=3){CONCAT(BUMP_IMAGE, 2).GetDimensions(TexSize2.x, TexSize2.y); lod2=Max(0, GetLod(uv2, TexSize2)+RELIEF_LOD_OFFSET);}
      Vec2 TexSize3; Flt lod3; if(MATERIALS>=4){CONCAT(BUMP_IMAGE, 3).GetDimensions(TexSize3.x, TexSize3.y); lod3=Max(0, GetLod(uv3, TexSize3)+RELIEF_LOD_OFFSET);}

      Half length;
                      {Half length0=Pixels(tpos.xy, TexSize0, lod0); if(RELIEF_STEPS_MUL!=1)if(lod0>0)length0*=RELIEF_STEPS_MUL; length =length0*I.material.x;} // don't use RELIEF_STEPS_MUL for first LOD
                      {Half length1=Pixels(tpos.xy, TexSize1, lod1); if(RELIEF_STEPS_MUL!=1)if(lod1>0)length1*=RELIEF_STEPS_MUL; length+=length1*I.material.y;} // don't use RELIEF_STEPS_MUL for first LOD
      if(MATERIALS>=3){Half length2=Pixels(tpos.xy, TexSize2, lod2); if(RELIEF_STEPS_MUL!=1)if(lod2>0)length2*=RELIEF_STEPS_MUL; length+=length2*I.material.z;} // don't use RELIEF_STEPS_MUL for first LOD
      if(MATERIALS>=4){Half length3=Pixels(tpos.xy, TexSize3, lod3); if(RELIEF_STEPS_MUL!=1)if(lod3>0)length3*=RELIEF_STEPS_MUL; length+=length3*I.material.w;} // don't use RELIEF_STEPS_MUL for first LOD

      Half steps  =Mid(length, 1, RELIEF_STEPS_MAX),
           stp    =1.0/steps,
           ray    =1;
      Vec2 uv_step=tpos.xy*stp; // keep as HP to avoid conversions several times in the loop below

   #if 1 // linear + interval search (faster)
      // linear search
      Half height_next, height_prev=0.5; // use 0.5 as approximate average value, we could do "RTexLodI(BUMP_IMAGE, I.uv, lod).BASE_CHANNEL_BUMP", however in tests that wasn't needed but only reduced performance
      LOOP for(;;)
      {
         ray-=stp;

         //I.uv+=uv_step;
                         uv0+=uv_step;
                         uv1+=uv_step;
         if(MATERIALS>=3)uv2+=uv_step;
         if(MATERIALS>=4)uv3+=uv_step;

                       //height_next =RTexLodI(       BUMP_IMAGE  , I.uv , lod ).BASE_CHANNEL_BUMP;
                         height_next =RTexLodI(       BUMP_IMAGE    , uv0, lod0).BASE_CHANNEL_BUMP*I.material.x;
                         height_next+=RTexLodI(CONCAT(BUMP_IMAGE, 1), uv1, lod1).BASE_CHANNEL_BUMP*I.material.y;
         if(MATERIALS>=3)height_next+=RTexLodI(CONCAT(BUMP_IMAGE, 2), uv2, lod2).BASE_CHANNEL_BUMP*I.material.z;
         if(MATERIALS>=4)height_next+=RTexLodI(CONCAT(BUMP_IMAGE, 3), uv3, lod3).BASE_CHANNEL_BUMP*I.material.w;

         if(height_next>=ray)break;
            height_prev=height_next;
      }

      // interval search
      if(1)
      {
         Half ray_prev=ray+stp;
         // prev pos: I.uv-uv_step, height_prev-ray_prev
         // next pos: I.uv        , height_next-ray
         Half hn=height_next-ray, hp=height_prev-ray_prev,
            frac=Sat(hn/(hn-hp));

        //I.uv-=uv_step*frac;
         offset=uv_step*frac;
                         uv0-=offset;
                         uv1-=offset;
         if(MATERIALS>=3)uv2-=offset;
         if(MATERIALS>=4)uv3-=offset;

         Bool      extra=(lod0<=0 || lod1<=0); if(MATERIALS>=3 && lod2<=0)extra=true; if(MATERIALS>=4 && lod3<=0)extra=true; // extra step (needed only for closeup)
         BRANCH if(extra)
         {
            Half ray_cur=ray+stp*frac,
                          //height_cur =RTexLodI(       BUMP_IMAGE  , I.uv , lod ).BASE_CHANNEL_BUMP;
                            height_cur =RTexLodI(       BUMP_IMAGE    , uv0, lod0).BASE_CHANNEL_BUMP*I.material.x;
                            height_cur+=RTexLodI(CONCAT(BUMP_IMAGE, 1), uv1, lod1).BASE_CHANNEL_BUMP*I.material.y;
            if(MATERIALS>=3)height_cur+=RTexLodI(CONCAT(BUMP_IMAGE, 2), uv2, lod2).BASE_CHANNEL_BUMP*I.material.z;
            if(MATERIALS>=4)height_cur+=RTexLodI(CONCAT(BUMP_IMAGE, 3), uv3, lod3).BASE_CHANNEL_BUMP*I.material.w;

            if(height_cur>=ray_cur) // if still below, then have to go back more, lerp between this position and prev pos
            {
               // prev pos: I.uv-uv_step (BUT I.uv before adjustment), height_prev-ray_prev
               // next pos: I.uv                                     , height_cur -ray_cur
               uv_step*=1-frac; // we've just travelled "uv_step*frac", so to go to the prev point, we need what's left, "uv_step*(1-frac)"
            }else // we went back too far, go forward, lerp between next pos and this position
            {
               // prev pos: I.uv                             , height_cur -ray_cur
               // next pos: I.uv (BUT I.uv before adjustment), height_next-ray
               hp=hn;
               uv_step*=-frac; // we've just travelled "uv_step*frac", so to go to the next point, we need the same way but other direction, "uv_step*-frac"
            }
            hn=height_cur-ray_cur;
            frac=Sat(hn/(hn-hp));

           //I.uv-=uv_step*frac;
            offset=uv_step*frac;
                            uv0-=offset;
                            uv1-=offset;
            if(MATERIALS>=3)uv2-=offset;
            if(MATERIALS>=4)uv3-=offset;
         }
      }
   #else // linear + binary search (slower because requires 3 tex reads in binary to get the same results as with only 0-1 tex reads in interval)
      this needs to be updated for 4 materials

      // linear search
      LOOP for(Int i=0; ; i++)
      {
         ray -=stp;
         I.uv+=uv_step;
         if(i>=steps || RTexLodI(BUMP_IMAGE, I.uv, lod).BASE_CHANNEL_BUMP>=ray)break;
      }

      // binary search
      {
         Half ray_prev=ray+stp,
              l=0, r=1, m=0.5;
         UNROLL for(Int i=0; i<RELIEF_STEPS_BINARY; i++)
         {
            Half height=RTexLodI(BUMP_IMAGE, I.uv-uv_step*m, lod).BASE_CHANNEL_BUMP;
            if(  height>Lerp(ray, ray_prev, m))l=m;else r=m;
            m=Avg(l, r);
         }
         I.uv-=uv_step*m;
      }
   #endif
   }
   #endif // Relief

   // #MaterialTextureLayout #MaterialTextureLayoutDetail

   // detail texture
   VecH4 det0, det1, det2, det3;
   if(DETAIL)
   {
                      det0=GetDetail0(uv0);
                      det1=GetDetail1(uv1);
      if(MATERIALS>=3)det2=GetDetail2(uv2);
      if(MATERIALS>=4)det3=GetDetail3(uv3);
   }

   // macro texture
   Half mac_blend;
#if MACRO
   mac_blend=LerpRS(MacroFrom, MacroTo, Length(I.pos))*MacroMax;
#endif

   // Reflect, Rough, Bump, Glow !! DO THIS FIRST because it may modify 'I.material' which affects everything !!
   VecH refl_rogh_glow;
   if(LAYOUT==2)
   {
      VecH4 ext0, ext1, ext2, ext3;
                      ext0=RTex(Ext , uv0);
                      ext1=RTex(Ext1, uv1);
      if(MATERIALS>=3)ext2=RTex(Ext2, uv2);
      if(MATERIALS>=4)ext3=RTex(Ext3, uv3);
      if(MTRL_BLEND)
      {
         VecH4 mtrl;      mtrl.x=MultiMaterialWeight(I.material.x, ext0.BASE_CHANNEL_BUMP);
                          mtrl.y=MultiMaterialWeight(I.material.y, ext1.BASE_CHANNEL_BUMP); if(MATERIALS==2){Half sum=Sum(mtrl.xy  ); if(CanDiv(sum))I.material.xy  =mtrl.xy  /sum;}
         if(MATERIALS>=3){mtrl.z=MultiMaterialWeight(I.material.z, ext2.BASE_CHANNEL_BUMP); if(MATERIALS==3){Half sum=Sum(mtrl.xyz ); if(CanDiv(sum))I.material.xyz =mtrl.xyz /sum;}}
         if(MATERIALS>=4){mtrl.w=MultiMaterialWeight(I.material.w, ext3.BASE_CHANNEL_BUMP); if(MATERIALS==4){Half sum=Sum(mtrl.xyzw); if(CanDiv(sum))I.material.xyzw=mtrl.xyzw/sum;}}
      }
                      {VecH refl_rogh_glow0=ext0.xyw*MultiMaterial0.refl_rogh_glow_mul+MultiMaterial0.refl_rogh_glow_add; if(DETAIL)APPLY_DETAIL_ROUGH(refl_rogh_glow0.y, det0.DETAIL_CHANNEL_ROUGH); refl_rogh_glow =refl_rogh_glow0*I.material.x;} // #MaterialTextureLayoutDetail
                      {VecH refl_rogh_glow1=ext1.xyw*MultiMaterial1.refl_rogh_glow_mul+MultiMaterial1.refl_rogh_glow_add; if(DETAIL)APPLY_DETAIL_ROUGH(refl_rogh_glow1.y, det1.DETAIL_CHANNEL_ROUGH); refl_rogh_glow+=refl_rogh_glow1*I.material.y;}
      if(MATERIALS>=3){VecH refl_rogh_glow2=ext2.xyw*MultiMaterial2.refl_rogh_glow_mul+MultiMaterial2.refl_rogh_glow_add; if(DETAIL)APPLY_DETAIL_ROUGH(refl_rogh_glow2.y, det2.DETAIL_CHANNEL_ROUGH); refl_rogh_glow+=refl_rogh_glow2*I.material.z;}
      if(MATERIALS>=4){VecH refl_rogh_glow3=ext3.xyw*MultiMaterial3.refl_rogh_glow_mul+MultiMaterial3.refl_rogh_glow_add; if(DETAIL)APPLY_DETAIL_ROUGH(refl_rogh_glow3.y, det3.DETAIL_CHANNEL_ROUGH); refl_rogh_glow+=refl_rogh_glow3*I.material.w;}
   }else
   {
                      {VecH refl_rogh_glow0=MultiMaterial0.refl_rogh_glow_add; if(DETAIL)APPLY_DETAIL_ROUGH(refl_rogh_glow0.y, det0.DETAIL_CHANNEL_ROUGH); refl_rogh_glow =refl_rogh_glow0*I.material.x;} // #MaterialTextureLayoutDetail
                      {VecH refl_rogh_glow1=MultiMaterial1.refl_rogh_glow_add; if(DETAIL)APPLY_DETAIL_ROUGH(refl_rogh_glow1.y, det1.DETAIL_CHANNEL_ROUGH); refl_rogh_glow+=refl_rogh_glow1*I.material.y;}
      if(MATERIALS>=3){VecH refl_rogh_glow2=MultiMaterial2.refl_rogh_glow_add; if(DETAIL)APPLY_DETAIL_ROUGH(refl_rogh_glow2.y, det2.DETAIL_CHANNEL_ROUGH); refl_rogh_glow+=refl_rogh_glow2*I.material.z;}
      if(MATERIALS>=4){VecH refl_rogh_glow3=MultiMaterial3.refl_rogh_glow_add; if(DETAIL)APPLY_DETAIL_ROUGH(refl_rogh_glow3.y, det3.DETAIL_CHANNEL_ROUGH); refl_rogh_glow+=refl_rogh_glow3*I.material.w;}
   }
   rough  =refl_rogh_glow.y;
   reflect=refl_rogh_glow.x;
   glow   =refl_rogh_glow.z;

   // Color + Detail + Macro !! do this second after modifying 'I.material' !! here Alpha is ignored for multi-materials
   VecH rgb;
                   {VecH col0=RTex(Col , uv0).rgb; col0.rgb*=MultiMaterial0.color.rgb; if(DETAIL)col0.rgb*=det0.DETAIL_CHANNEL_COLOR; if(MACRO)col0.rgb=Lerp(col0.rgb, RTex(Mac , uv0*MacroScale).rgb, MultiMaterial0.macro*mac_blend); rgb =I.material.x*col0;} // #MaterialTextureLayoutDetail
                   {VecH col1=RTex(Col1, uv1).rgb; col1.rgb*=MultiMaterial1.color.rgb; if(DETAIL)col1.rgb*=det1.DETAIL_CHANNEL_COLOR; if(MACRO)col1.rgb=Lerp(col1.rgb, RTex(Mac1, uv1*MacroScale).rgb, MultiMaterial1.macro*mac_blend); rgb+=I.material.y*col1;}
   if(MATERIALS>=3){VecH col2=RTex(Col2, uv2).rgb; col2.rgb*=MultiMaterial2.color.rgb; if(DETAIL)col2.rgb*=det2.DETAIL_CHANNEL_COLOR; if(MACRO)col2.rgb=Lerp(col2.rgb, RTex(Mac2, uv2*MacroScale).rgb, MultiMaterial2.macro*mac_blend); rgb+=I.material.z*col2;}
   if(MATERIALS>=4){VecH col3=RTex(Col3, uv3).rgb; col3.rgb*=MultiMaterial3.color.rgb; if(DETAIL)col3.rgb*=det3.DETAIL_CHANNEL_COLOR; if(MACRO)col3.rgb=Lerp(col3.rgb, RTex(Mac3, uv3*MacroScale).rgb, MultiMaterial3.macro*mac_blend); rgb+=I.material.w*col3;}
#if COLORS
   col.rgb*=rgb.rgb;
#else
   col.rgb =rgb.rgb;
#endif

   // normal
#if   BUMP_MODE==SBUMP_ZERO
   nrm=VecH(0, 0, -1);
#elif BUMP_MODE==SBUMP_FLAT
   nrm=I.Nrm(); // can't add DETAIL normal because it would need 'I.mtrx'
#else
   if(DETAIL)
   { // #MaterialTextureLayoutDetail
                      nrm.xy =(RTex(Nrm , uv0).BASE_CHANNEL_NORMAL*MultiMaterial0.normal + det0.DETAIL_CHANNEL_NORMAL)*I.material.x;
                      nrm.xy+=(RTex(Nrm1, uv1).BASE_CHANNEL_NORMAL*MultiMaterial1.normal + det1.DETAIL_CHANNEL_NORMAL)*I.material.y;
      if(MATERIALS>=3)nrm.xy+=(RTex(Nrm2, uv2).BASE_CHANNEL_NORMAL*MultiMaterial2.normal + det2.DETAIL_CHANNEL_NORMAL)*I.material.z;
      if(MATERIALS>=4)nrm.xy+=(RTex(Nrm3, uv3).BASE_CHANNEL_NORMAL*MultiMaterial3.normal + det3.DETAIL_CHANNEL_NORMAL)*I.material.w;
   }else
   {
                      nrm.xy =RTex(Nrm , uv0).BASE_CHANNEL_NORMAL*(MultiMaterial0.normal*I.material.x);
                      nrm.xy+=RTex(Nrm1, uv1).BASE_CHANNEL_NORMAL*(MultiMaterial1.normal*I.material.y);
      if(MATERIALS>=3)nrm.xy+=RTex(Nrm2, uv2).BASE_CHANNEL_NORMAL*(MultiMaterial2.normal*I.material.z);
      if(MATERIALS>=4)nrm.xy+=RTex(Nrm3, uv3).BASE_CHANNEL_NORMAL*(MultiMaterial3.normal*I.material.w);
   }
   nrm.z=CalcZ(nrm.xy);
   nrm  =Transform(nrm, I.mtrx);
#endif

#endif // MATERIALS

   col.rgb+=Highlight.rgb;

#if BUMP_MODE!=SBUMP_ZERO
   nrm=Normalize(nrm); // transforming by matrix might scale normal in >SBUMP_FLAT, and in SBUMP_FLAT normal is interpolated linearly and normalization will push to full range, however we're storing to 0..1 range, so have to normalize
#endif

#if FX!=FX_GRASS_2D && FX!=FX_LEAF_2D && FX!=FX_LEAFS_2D
   BackFlip(nrm, front);
#endif

   output.color     (col.rgb);
   output.glow      (glow   );
   output.normal    (nrm    );
   output.mode      ((FX==FX_GRASS_3D || FX==FX_LEAF_3D || FX==FX_LEAFS_3D) ? PSM_TRANSLUCENT : (FX==FX_CLEAR_COAT) ? PSM_CLEAR_COAT : PSM_NONE);
   output.rough     (rough  );
   output.reflect   (reflect);
#if USE_VEL
   output.motion    (I.projected_prev_pos_xyw, pixel);
#else
   output.motionZero();
#endif
}
/******************************************************************************/
// HULL / DOMAIN
/******************************************************************************/
#if TESSELATE
HSData HSConstant(InputPatch<Data,3> I) {return GetHSData(I[0].pos, I[1].pos, I[2].pos, I[0].Nrm(), I[1].Nrm(), I[2].Nrm());}
[maxtessfactor(5.0)]
[domain("tri")]
[partitioning("fractional_odd")] // use 'odd' because it supports range from 1.0 ('even' supports range from 2.0)
[outputtopology("triangle_cw")]
[patchconstantfunc("HSConstant")]
[outputcontrolpoints(3)]
Data HS
(
   InputPatch<Data,3> I,
   UInt cp_id:SV_OutputControlPointID
)
{
   Data O;

   O.pos=I[cp_id].pos;

#if USE_VEL
   O.projected_prev_pos_xyw=I[cp_id].projected_prev_pos_xyw;
#endif

#if TESSELATE_VEL
   O.nrm_prev=I[cp_id].nrm_prev;
#endif

#if SET_UV
   O.uv=I[cp_id].uv;
#endif

#if   BUMP_MODE> SBUMP_FLAT
   O.mtrx=I[cp_id].mtrx;
#elif BUMP_MODE==SBUMP_FLAT
   O.nrm =I[cp_id].nrm ;
#endif

#if MATERIALS>1
   O.material=I[cp_id].material;
#endif

#if COLORS
   O.col=I[cp_id].col;
#endif

#if FAST_TPOS
   O._tpos=I[cp_id]._tpos;
#endif

#if GRASS_FADE
   O.fade_out=I[cp_id].fade_out;
#endif

#if ALPHA_TEST==ALPHA_TEST_DITHER
   O.face_id=I[cp_id].face_id;
#endif

   return O;
}
/******************************************************************************/
[domain("tri")]
void DS
(
   HSData hs_data, const OutputPatch<Data,3> I, Vec B:SV_DomainLocation,

   out Data O,
   out Vec4 pixel:POSITION
)
{
#if   BUMP_MODE> SBUMP_FLAT
   O.mtrx[0]=I[0].mtrx[0]*B.z + I[1].mtrx[0]*B.x + I[2].mtrx[0]*B.y;
   O.mtrx[1]=I[0].mtrx[1]*B.z + I[1].mtrx[1]*B.x + I[2].mtrx[1]*B.y;
   SetDSPosNrm(O.pos, O.mtrx[2], I[0].pos, I[1].pos, I[2].pos, I[0].Nrm(), I[1].Nrm(), I[2].Nrm(), B, hs_data, false, 0);
#elif BUMP_MODE==SBUMP_FLAT
   SetDSPosNrm(O.pos, O.nrm    , I[0].pos, I[1].pos, I[2].pos, I[0].Nrm(), I[1].Nrm(), I[2].Nrm(), B, hs_data, false, 0);
#endif

#if SET_UV
   O.uv=I[0].uv*B.z + I[1].uv*B.x + I[2].uv*B.y;
#endif

#if MATERIALS>1
   O.material=I[0].material*B.z + I[1].material*B.x + I[2].material*B.y;
#endif

#if COLORS
   O.col=I[0].col*B.z + I[1].col*B.x + I[2].col*B.y;
#endif

#if FAST_TPOS
   O._tpos=I[0]._tpos*B.z + I[1]._tpos*B.x + I[2]._tpos*B.y;
#endif

#if GRASS_FADE
   O.fade_out=I[0].fade_out*B.z + I[1].fade_out*B.x + I[2].fade_out*B.y;
#endif

#if ALPHA_TEST==ALPHA_TEST_DITHER
   O.face_id=I[0].face_id;
#endif

   pixel=Project(O.pos);

#if USE_VEL
   #if TESSELATE_VEL
      FIXME
   #else // this is just an approximation
      Vec interpolated_pos    =I[0].pos                   *B.z + I[1].pos                   *B.x + I[2].pos                   *B.y;
      O.projected_prev_pos_xyw=I[0].projected_prev_pos_xyw*B.z + I[1].projected_prev_pos_xyw*B.x + I[2].projected_prev_pos_xyw*B.y
                              +pixel.xyw-ProjectXYW(interpolated_pos); // + delta (tesselated pos - interpolated pos)
   #endif
#endif
}
#endif
/******************************************************************************/
