/******************************************************************************/
#include "!Header.h"
#include "Bloom.h"

#ifndef MODE
   #define MODE 0 // 0=no glow, 1=glow, 2=bloom/glow already precomputed
#endif
#define GLOW        (MODE==1)
#define PRECOMPUTED (MODE==2)

#ifndef VIEW_FULL
#define VIEW_FULL 1
#endif

#ifndef HALF_RES
   #define HALF_RES 0
#endif

#ifndef DITHER
   #define DITHER 0
#endif

#define BLOOM_GLOW_GAMMA_PER_PIXEL 0 // #BloomGlowGammaPerPixel can be disabled because it will be faster and visual difference is minimal
/******************************************************************************/
void BloomDS_VS(VtxInput vtx,
    NOPERSP out Vec2 uv   :UV,
    NOPERSP out Vec4 pixel:POSITION)
{
   uv   =vtx.uv (); if(GLOW)uv-=ImgSize.xy*Vec2(HALF_RES ? 0.5 : 1.5, HALF_RES ? 0.5 : 1.5);
   pixel=vtx.pos4();
}
VecH BloomDS_PS(NOPERSP Vec2 uv:UV):TARGET // "Max(0, " of the result is not needed because we're rendering to 1 byte per channel RT
{
   if(GLOW)
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
      color =BloomColor(color, PRECOMPUTED);
      color+=glow.rgb;
      return color;
   }else
   {
      if(HALF_RES)
      {
         return BloomColor(TexLod(Img, UVInView(uv, VIEW_FULL)).rgb, PRECOMPUTED);
      }else
      {
         Vec2 uv_min=UVInView(uv-ImgSize.xy, VIEW_FULL),
              uv_max=UVInView(uv+ImgSize.xy, VIEW_FULL);
         return BloomColor(TexLod(Img, Vec2(uv_min.x, uv_min.y)).rgb
                          +TexLod(Img, Vec2(uv_max.x, uv_min.y)).rgb
                          +TexLod(Img, Vec2(uv_min.x, uv_max.y)).rgb
                          +TexLod(Img, Vec2(uv_max.x, uv_max.y)).rgb, PRECOMPUTED);
      }
   }
}
/******************************************************************************/
VecH4 Bloom_PS(NOPERSP Vec2 uv:UV,
               NOPERSP PIXEL     ):TARGET
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

   col.rgb=col.rgb*BloomParams.x + TexLod(Img1, uv).rgb; // bloom, can't use 'TexPoint' because 'Img1' can be smaller
   if(DITHER)ApplyDither(col.rgb, pixel.xy);
   return col;
}
/******************************************************************************/
