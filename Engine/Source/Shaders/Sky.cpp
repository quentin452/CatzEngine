/******************************************************************************/
#include "!Header.h"
#include "Sky.h"
#include "Layered Clouds.h"
/******************************************************************************/
VecH4 SkyColor(Vec uvw)
{
   Half hor=Pow(1-Sat(uvw.y), SkyHorExp);
   return Lerp(SkySkyCol, SkyHorCol, hor);
}
/******************************************************************************/
// MULTI_SAMPLE, FLAT, DENSITY, TEXTURES, STARS, DITHER, PER_VERTEX, CLOUD
void Sky_VS
(
   VtxInput vtx,
   out Vec4 vpos:POSITION

#if !FLAT
 , out Vec pos:POS
#endif

#if TEXTURES || !PER_VERTEX
 , out Vec uvw:UVW
#endif

#if STARS && TEXTURES==0
 , out Vec uvw_star:UVW_STAR
#endif

#if PER_VERTEX && TEXTURES==0
 , out VecH4 col:COLOR
#endif

#if CLOUD
 , out Vec   uvw_cloud:UVW_CLOUD
 , out VecH4 col_cloud:COL_CLOUD
#endif
)
{
#if FLAT
   Vec pos;
#endif
   pos=TransformPos(vtx.pos()); vpos=Project(pos);

#if TEXTURES || !PER_VERTEX
   uvw=vtx.pos();
#endif

#if STARS && TEXTURES==0
   uvw_star=Transform(vtx.pos(), SkyStarOrn);
#endif

#if PER_VERTEX && TEXTURES==0
   col=SkyColor(vtx.pos());
#endif

#if CLOUD
   uvw_cloud=vtx.pos()*Vec(LCScale, 1, LCScale);
   col_cloud=CL[0].color; col_cloud.a*=Sat(CloudAlpha(vtx.pos().y));
#endif
}
/******************************************************************************/
VecH4 Sky_PS
(
   PIXEL

#if !FLAT
 , Vec pos:POS
#endif

#if TEXTURES || !PER_VERTEX
 , Vec uvw:UVW
#endif

#if STARS && TEXTURES==0
 , Vec uvw_star:UVW_STAR
#endif

#if PER_VERTEX && TEXTURES==0
 , VecH4 vtx_col:COLOR
#endif

#if CLOUD
 , Vec   uvw_cloud:UVW_CLOUD
 , VecH4 col_cloud:COL_CLOUD
#endif

#if MULTI_SAMPLE==2 && !FLAT
 , UInt index:SV_SampleIndex
#endif
):TARGET
{
   Half alpha;
#if FLAT
   alpha=0; // flat uses ALPHA_NONE
#else
      Flt pos_scale=Normalize(pos).z;
   #if   MULTI_SAMPLE==0
      Flt dist=TexDepthPix(pixel.xy)/pos_scale;
      alpha=Sat(dist*SkyFracMulAdd.x + SkyFracMulAdd.y);
   #elif MULTI_SAMPLE==1
      alpha=0; UNROLL for(Int i=0; i<MS_SAMPLES; i++){Flt dist=TexDepthMS(pixel.xy, i)/pos_scale; alpha+=Sat(dist*SkyFracMulAdd.x + SkyFracMulAdd.y);}
      alpha/=MS_SAMPLES;
   #elif MULTI_SAMPLE==2
      Flt dist=TexDepthMS(pixel.xy, index)/pos_scale;
      alpha=Sat(dist*SkyFracMulAdd.x + SkyFracMulAdd.y);
   #endif

   if(DENSITY)alpha=Pow(SkyDnsExp, alpha)*SkyDnsMulAdd.x+SkyDnsMulAdd.y; // here 'alpha' means opacity of the sky which is used as the distance from start to end point, this function matches 'AccumulatedDensity'
#endif

   VecH4 col;

#if   TEXTURES==2
   col.rgb=Lerp(TexCube(Cub, uvw).rgb, TexCube(Cub1, uvw).rgb, SkyBoxBlend);
#elif TEXTURES==1
   col.rgb=TexCube(Cub, uvw).rgb;
#else
   #if PER_VERTEX
      col=vtx_col;
   #else
      uvw=Normalize(uvw);
      col=SkyColor (uvw);

      Half cos      =Dot(SkySunPos, uvw),
           highlight=1+Sqr(cos)*((cos>0) ? SkySunHighlight.x : SkySunHighlight.y); // rayleigh, here 'Sqr' works better than 'Abs'
      col.rgb*=highlight;
   #endif

   #if STARS
      col.rgb=Lerp(TexCube(Cub, uvw_star).rgb, col.rgb, col.a);
   #endif
#endif

   col.a=alpha;

#if CLOUD
   Vec2  cloud_tex=Normalize(uvw_cloud).xz;
   VecH4 cloud_col=RTex(Img, cloud_tex*CL[0].scale + CL[0].position)*col_cloud;
   col.rgb=Lerp(col.rgb, cloud_col.rgb, cloud_col.a);
#endif

   if(DITHER && MULTI_SAMPLE!=2)ApplyDither(col.rgb, pixel.xy); // skip dither for MS because it won't be noticeable

   return col;
}
/******************************************************************************/
