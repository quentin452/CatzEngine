/******************************************************************************

   EASU must always be performed in gamma space (because when done in linear gamma, there are some artifacts / more blocky)

   RCAS can be done in either but in gamma space it produces sharper results, so use that.

/******************************************************************************
ALPHA, DITHER, IN_GAMMA, OUT_GAMMA, GATHER, HALF
/******************************************************************************/
#define FAST_GAMMA (IN_GAMMA && OUT_GAMMA) // can use fast gamma only if we do both conversions in the shader
/******************************************************************************/
#include "../!Header.h"
#define Quart _Quart // "ffx_a.h" has its own 'Quart'
#define A_GPU  1
#define A_HLSL 1
#if HALF // workaround for Nvidia GeForce Half FIXME: TODO: check again in the future if this is still needed, if Nvidia bug is fixed then ALWAYS do "#define A_HALF 1" #NvidiaGeForceFidelityFXHalf
   #define A_HALF 1
#endif
#include "ffx_a.h"
/******************************************************************************/
VecH4 GetChannel(VecH4 c)
{
#if IN_GAMMA
   #if FAST_GAMMA
      c.x=LinearToSRGBFast(c.x);
      c.y=LinearToSRGBFast(c.y);
      c.z=LinearToSRGBFast(c.z);
      c.w=LinearToSRGBFast(c.w);
   #else
      c.x=LinearToSRGB(c.x);
      c.y=LinearToSRGB(c.y);
      c.z=LinearToSRGB(c.z);
      c.w=LinearToSRGB(c.w);
   #endif
#endif
   return c;
}
VecH4 GetColor(VecH4 col)
{
#if IN_GAMMA
   #if FAST_GAMMA
      col.rgb=LinearToSRGBFast(col.rgb);
   #else
      col.rgb=LinearToSRGB(col.rgb);
   #endif
#endif
   return col;
}

VecH4 FsrEasuRH(Vec2 p) {return GetChannel(TexGatherR(Img, p));}
VecH4 FsrEasuGH(Vec2 p) {return GetChannel(TexGatherG(Img, p));}
VecH4 FsrEasuBH(Vec2 p) {return GetChannel(TexGatherB(Img, p));}
Vec4  FsrEasuRF(Vec2 p) {return GetChannel(TexGatherR(Img, p));}
Vec4  FsrEasuGF(Vec2 p) {return GetChannel(TexGatherG(Img, p));}
Vec4  FsrEasuBF(Vec2 p) {return GetChannel(TexGatherB(Img, p));}

VecH4 FsrEasuH(Vec2 p, VecI2 ofs) {return GetColor(TexPointOfs(Img, p, ofs));}
Vec4  FsrEasuF(Vec2 p, VecI2 ofs) {return GetColor(TexPointOfs(Img, p, ofs));}

#if A_HALF
VecH4 FsrRcasLoadH(ASW2 p) {return GetColor(Img[p]);}
void FsrRcasInputH(inout AH1 r, inout AH1 g, inout AH1 b) {}
#endif
Vec4  FsrRcasLoadF(ASU2 p) {return GetColor(Img[p]);}
void FsrRcasInputF(inout AF1 r, inout AF1 g, inout AF1 b) {}
/******************************************************************************/
#define FSR_EASU_H 1
#define FSR_EASU_F 1
#define FSR_RCAS_H 1
#define FSR_RCAS_F 1
#if ALPHA
   #define FSR_RCAS_PASSTHROUGH_ALPHA 1
#endif
#include "ffx_fsr1.h"
/******************************************************************************/
struct EASU
{
   AU4 c0, c1, c2, c3;
};
BUFFER(EASU)
   EASU Easu;
BUFFER_END

VecH4 EASU_PS(NOPERSP PIXEL):TARGET
{
   VecI2 pix=pixel.xy;
   VecH4 col;
#if A_HALF && GATHER // use Half only for Gather version, because Half non-gather fails to compile
   FsrEasuH(col.rgb, pix, Easu.c0, Easu.c1, Easu.c2, Easu.c3);
#else
   FsrEasuF(col.rgb, pix, Easu.c0, Easu.c1, Easu.c2, Easu.c3);
#endif

#if ALPHA
   // FIXME OPTIMIZE
   Vec2 uv=(Vec2(pix) * AF2_AU2(Easu.c0.xy) + AF2_AU2(Easu.c0.zw)) * AF2_AU2(Easu.c1.xy) + Vec2(0.5, -0.5) * AF2_AU2(Easu.c1.zw);
   col.a=Tex(Img, uv).a;
#else
   col.a=1;
#endif

#if DITHER
   ApplyDither(col.rgb, pixel.xy, false); // here 'col' is already in gamma space
#endif

#if OUT_GAMMA
   #if FAST_GAMMA
      col.rgb=SRGBToLinearFast(col.rgb);
   #else
      col.rgb=SRGBToLinear(col.rgb);
   #endif
#endif

#if COLORS
   col=col*Color[0]+Color[1]; // this needs to be done in Linear Gamma
#endif
   return col;
}
/******************************************************************************/
struct RCAS
{
   AU4 c0;
};
BUFFER(RCAS)
   RCAS Rcas;
#if GL
   Vec2 RcasMulAdd;
#endif
BUFFER_END

VecH4 RCAS_PS(NOPERSP PIXEL):TARGET
{
#if GL
   VecI2 pix=VecI2(pixel.x, pixel.y*RcasMulAdd.x+RcasMulAdd.y);
#else
   VecI2 pix=pixel.xy;
#endif
   VecH4 col;
#if A_HALF && GATHER // use Half only for Gather version, because Half non-gather fails to compile
   #if ALPHA
      FsrRcasH(col.r, col.g, col.b, col.a, pix, Rcas.c0);
   #else
      FsrRcasH(col.r, col.g, col.b, pix, Rcas.c0); col.a=1;
   #endif
#else
   #if ALPHA
      FsrRcasF(col.r, col.g, col.b, col.a, pix, Rcas.c0);
   #else
      FsrRcasF(col.r, col.g, col.b, pix, Rcas.c0); col.a=1;
   #endif
#endif

#if DITHER
   ApplyDither(col.rgb, pixel.xy, false); // here 'col' is already in gamma space
#endif

#if OUT_GAMMA
   #if FAST_GAMMA
      col.rgb=SRGBToLinearFast(col.rgb);
   #else
      col.rgb=SRGBToLinear(col.rgb);
   #endif
#endif
   return col;
}
/******************************************************************************
void Filter(int2 pos)
{
#if UPSCALE
   if(0) // linear filtering
   {
      Vec2 pp = (Vec2(pos) * AF2_AU2(Easu.c0.xy) + AF2_AU2(Easu.c0.zw)) * AF2_AU2(Easu.c1.xy) + Vec2(0.5, -0.5) * AF2_AU2(Easu.c1.zw);
      RWImg[pos]=TexLod(Img, pp);
   }else
   {
      VecH4 col; FsrEasuH(col.rgb, pos, Easu.c0, Easu.c1, Easu.c2, Easu.c3); col.a=1; RWImg[pos]=col;
   }
#else
   VecH4 col; FsrRcasH(col.r, col.g, col.b, pos, Rcas.c0); col.a=1; RWImg[pos]=col;
#endif
}

[numthreads(64, 1, 1)]
void CS(uint3 LocalThreadId : SV_GroupThreadID, uint3 WorkGroupId : SV_GroupID, uint3 Dtid : SV_DispatchThreadID)
{
   AU2 gxy = ARmp8x8(LocalThreadId.x) + AU2(WorkGroupId.x << 4u, WorkGroupId.y << 4u);
   Filter(gxy); gxy.x += 8u;
   Filter(gxy); gxy.y += 8u;
   Filter(gxy); gxy.x -= 8u;
   Filter(gxy);
}
/******************************************************************************
Don't use, RCAS is better (CAS has too strong sharpening on big contrast, when black touching white, and not strong enough on medium contrast)
AH3 CasLoadH(ASW2 p) {return GetColor(Img[p]);}
AF3 CasLoad (ASU2 p) {return GetColor(Img[p]);}
void CasInputH(inout AH2 r, inout AH2 g, inout AH2 b) {}
void CasInput (inout AF1 r, inout AF1 g, inout AF1 b) {}
//#define CAS_BETTER_DIAGONALS 1
#include "ffx_cas.h"

struct CAS
{
   uint4 const0;
   uint4 const1;
};
BUFFER(CAS)
   CAS Cas;
BUFFER_END

VecH4 CAS_PS(NOPERSP PIXEL):TARGET
{
   VecH4 col; CasFilter(col.r, col.g, col.b, pixel.xy, Cas.const0, Cas.const1, true);
#if ALPHA
   col.a=Img[pixel.xy].a;
#else
   col.a=1;
#endif

#if DITHER
   ApplyDither(col.rgb, pixel.xy, false); // here 'col' is already in gamma space
#endif

#if OUT_GAMMA
   #if FAST_GAMMA
      col.rgb=SRGBToLinearFast(col.rgb);
   #else
      col.rgb=SRGBToLinear(col.rgb);
   #endif
#endif
   return col;
}
/******************************************************************************/
