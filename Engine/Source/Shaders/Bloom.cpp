/******************************************************************************/
// GLOW, VIEW_FULL, HALF_RES, DITHER, PRECOMPUTED, EXPOSURE, TONE_MAP
#include "!Header.h"
#include "Bloom.h"
#include "Hdr.h"

#ifndef GLOW
#define GLOW 0
#endif

#ifndef VIEW_FULL
#define VIEW_FULL 1
#endif

#ifndef HALF_RES
#define HALF_RES 0
#endif

#ifndef DITHER
#define DITHER 0
#endif

#ifndef PRECOMPUTED
#define PRECOMPUTED 0
#endif

#define BLOOM_GLOW_GAMMA_PER_PIXEL 0 // #BloomGlowGammaPerPixel can be disabled because it will be faster and visual difference is minimal
/******************************************************************************/
void BloomDS_VS(VtxInput vtx,
#if EXPOSURE
   NOINTERP out Half bloom_scale:BLOOM_SCALE,
#endif
   NOPERSP out Vec2 uv  :UV,
   NOPERSP out Vec4 vpos:POSITION)
{
#if EXPOSURE
   bloom_scale=BloomScale()*ImgX1[VecI2(0, 0)];
#endif
   uv  =vtx.uv  (); if(GLOW)uv-=ImgSize.xy*Vec2(HALF_RES ? 0.5 : 1.5, HALF_RES ? 0.5 : 1.5);
   vpos=vtx.pos4();
}
/******************************************************************************/
VecH BloomDS_PS
(
#if EXPOSURE
   NOINTERP Half bloom_scale:BLOOM_SCALE,
#endif
   NOPERSP Vec2 uv:UV
):TARGET // "Max(0, " of the result is not needed because we're rendering to 1 byte per channel RT
{
#if !EXPOSURE
   Half bloom_scale=BloomScale();
#endif
   if(GLOW) // this always has PRECOMPUTED=0
   {
      const Int res=(HALF_RES ? 2 : 4);

      VecH  color=0;
      VecH4 glow =0;
      UNROLL for(Int y=0; y<res; y++)
      UNROLL for(Int x=0; x<res; x++)
      {
         VecH4 c=TexLod(Img, UVInView(uv+ImgSize.xy*Vec2(x, y), VIEW_FULL)); // can't use 'TexPoint' because 'Img' can be supersampled
         if(BLOOM_GLOW_GAMMA_PER_PIXEL)c.a=SRGBToLinearFast(c.a); // have to convert to linear because small glow of 1/255 would give 12.7/255 sRGB (Glow was sampled from non-sRGB texture and stored in RT alpha channel without any gamma conversions)
         color   +=c.rgb;
         glow.rgb+=c.rgb*c.a;
         glow.a  +=c.a;
      }
      if(!BLOOM_GLOW_GAMMA_PER_PIXEL)glow.a=SRGBToLinearFast(glow.a); // have to convert to linear because small glow of 1/255 would give 12.7/255 sRGB (Glow was sampled from non-sRGB texture and stored in RT alpha channel without any gamma conversions)
      glow.rgb*=(glow.a*BloomGlow())/Max(Max(glow.rgb), HALF_MIN); // #Glow
      color =BloomColor(color, bloom_scale);
      color+=glow.rgb;
      return color;
   }else
   {
      if(HALF_RES)
      {
         VecH col=TexLod(Img, UVInView(uv, VIEW_FULL)).rgb;
         return PRECOMPUTED ? col : BloomColor(col, bloom_scale);
      }else
      {
         Vec2 uv_min=UVInView(uv-ImgSize.xy, VIEW_FULL),
              uv_max=UVInView(uv+ImgSize.xy, VIEW_FULL);
         VecH col=TexLod(Img, Vec2(uv_min.x, uv_min.y)).rgb
                 +TexLod(Img, Vec2(uv_max.x, uv_min.y)).rgb
                 +TexLod(Img, Vec2(uv_min.x, uv_max.y)).rgb
                 +TexLod(Img, Vec2(uv_max.x, uv_max.y)).rgb;
         return PRECOMPUTED ? col/4 : BloomColor(col, bloom_scale);
      }
   }
}
/******************************************************************************/
void Bloom_VS(VtxInput vtx,
#if EXPOSURE
   NOINTERP out Half bloom_orig:BLOOM_ORIG,
#endif
   NOPERSP out Vec2 uv  :UV,
   NOPERSP out Vec4 vpos:POSITION)
{
#if EXPOSURE
   bloom_orig=BloomOriginal()*ImgX1[VecI2(0, 0)];
#endif
   uv  =vtx.uv();
   vpos=vtx.pos4();
}
/******************************************************************************/
VecH4 Bloom_PS
(
#if EXPOSURE
   NOINTERP Half bloom_orig:BLOOM_ORIG,
#endif
   NOPERSP Vec2 uv:UV
#if DITHER
 , NOPERSP PIXEL
#endif
):TARGET
{
   // final=src*original + Sat((src-cut)*scale)
   VecH4 col;

#if ALPHA==2 // separate
   col.rgb=TexLod(Img , uv).rgb; // original, can't use 'TexPoint' because 'Img'  can be supersampled
   col.a  =TexLod(ImgX, uv)    ; //           can't use 'TexPoint' because 'ImgX' can be supersampled
#elif ALPHA==1 // use alpha
   col=TexLod(Img, uv); // original, can't use 'TexPoint' because 'Img' can be supersampled
#else // no alpha
   col.rgb=TexLod(Img, uv).rgb; // original, can't use 'TexPoint' because 'Img' can be supersampled
   col.a=1; // force full alpha so back buffer effects can work ok
#endif

#if !EXPOSURE
   Half bloom_orig=BloomOriginal();
#endif
   col.rgb=col.rgb*bloom_orig + TexLod(Img1, uv).rgb; // bloom, can't use 'TexPoint' because 'Img1' can be smaller

#if TONE_MAP // needs to be before CONTRAST
   if(TONE_MAP==STONE_MAP_DEFAULT )col.rgb=TonemapEsenthel          (col.rgb);
   if(TONE_MAP==STONE_MAP_ACES_LDR)col.rgb=TonemapACES_LDR_Narkowicz(col.rgb);
   if(TONE_MAP==STONE_MAP_ACES_HDR)col.rgb=TonemapACES_HDR_Narkowicz(col.rgb);

 /*if(TONE_MAP==1                                 )col.rgb=TonemapAMD_Cauldron(col.rgb);
   if(TONE_MAP==2                                 )col.rgb=TonemapLog     (col.rgb);
   if(TONE_MAP==3                                 )col.rgb=TonemapRcpSqr  (col.rgb);
   if(TONE_MAP==4                                 )col.rgb=TonemapExp     (col.rgb);
   if(TONE_MAP==5                                 )col.rgb=TonemapRcp     (col.rgb);
   if(TONE_MAP==6                                 )col.rgb=TonemapEsenthel(col.rgb);
   if(TONE_MAP==7                                 )col.rgb=TonemapLogML4  (col.rgb);
   if(TONE_MAP==8                                 )col.rgb=TonemapLogML5  (col.rgb);
   if(TONE_MAP==9                                 )col.rgb=TonemapLogML6  (col.rgb);

   if(TONE_MAP==STONE_MAP_ACES_HILL               )col.rgb=TonemapACESHill         (col.rgb);
   if(TONE_MAP==STONE_MAP_ACES_LOTTES             )col.rgb=TonemapACESLottes       (col.rgb);
   if(TONE_MAP==STONE_MAP_HEJL_BURGESS_DAWSON     )col.rgb=ToneMapHejlBurgessDawson(col.rgb);*/

   #if 0 // Debug Drawing
      Vec2 pos=Vec2(uv.x*AspectRatio, 1-uv.y);
   #if 1
      Flt eps=1/(SRGBToLinear(pos.y+1.0/512)-SRGBToLinear(pos.y));
      pos=SRGBToLinear(pos);
   #else
      Flt eps=256;
   #endif
      //Flt eps=1/pos.y*128;//pos.x;
      pos*=16; eps/=16;
      DrawLine(col.rgb, VecH(1,1,1), pos, eps, pos.x);
    /*DrawLine(col.rgb, VecH(0.5,0,0), pos, eps, TonemapLog(pos.x));
      DrawLine(col.rgb, VecH(0.5,0,0), pos, eps, TonemapLogML4(pos.x));
      DrawLine(col.rgb, VecH(0.5,0,0), pos, eps, TonemapLogML5(pos.x));
      DrawLine(col.rgb, VecH(0.5,0,0), pos, eps, TonemapLogML6(pos.x));
      DrawLine(col.rgb, VecH(0,0.5,0), pos, eps, TonemapRcpSqr(pos.x));
      DrawLine(col.rgb, VecH(0,0,0.5), pos, eps, TonemapExp(pos.x));
      DrawLine(col.rgb, VecH(0.5,0.5,0), pos, eps, TonemapRcp(pos.x));*/
      DrawLine(col.rgb, VecH(1,0,0), pos, eps, TonemapEsenthel(pos.x));
      DrawLine(col.rgb, VecH(0,1,0), pos, eps, TonemapACES_LDR_Narkowicz(pos.x));
      DrawLine(col.rgb, VecH(0,0,1), pos, eps, TonemapACES_HDR_Narkowicz(pos.x));
   #endif
#endif

#if CONTRAST // needs to be after TONE_MAP
   const Bool ACES=false;
   const int  gamma=0;

   if(ACES)col.rgb=mul(ACESInputMat, col.rgb);

   if(gamma==0)col.rgb=LinearToSRGB (col.rgb);else // preserves sRGB 0.5 and overall brightness
   if(gamma==1)col.rgb=LinearToSRGB1(col.rgb);else // preserves sRGB 0.5 and overall brightness, dark colors darkened too much
               col.rgb=Sqrt         (col.rgb);     // darkens everything

 //col.rgb=col.rgb*2-1; col.rgb=SigmoidSqr(col.rgb*contrast)/SigmoidSqr(contrast); col.rgb=col.rgb*0.5+0.5;
   col.rgb=SigmoidSqr(col.rgb*Contrast.x+Contrast.y); col.rgb=col.rgb*Contrast.z+0.5;

   if(gamma==0)col.rgb=SRGBToLinear (col.rgb);else
   if(gamma==1)col.rgb=SRGBToLinear1(col.rgb);else
               col.rgb=Sqr          (col.rgb);

   if(ACES){col.rgb=mul(ACESOutputMat, col.rgb); col.rgb=Max(0, col.rgb);}
#endif

#if DITHER
   ApplyDither(col.rgb, pixel.xy);
#endif
   return col;
}
/******************************************************************************/
