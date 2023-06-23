/******************************************************************************
// AMD LPM
#define Quart _Quart // "ffx_a.h" has its own 'Quart'
#define A_GPU  1
#define A_HLSL 1
#define A_HALF 1
#include "FidelityFX/ffx_a.h"

BUFFER(AMD_LPT)
   uint4 AMD_LPT_constant[24];
BUFFER_END
AU4 LpmFilterCtl(AU1 i) {return AMD_LPT_constant[i];}

#define LPM_NO_SETUP 1
#define AD1_(a) ((AD1)(a))
#define AF1_(a) ((AF1)(a))
#define AL1_(a) ((AL1)(a))
#define AU1_(a) ((AU1)(a))
#include "FidelityFX/ffx_lpm.h"
#undef Quart
/******************************************************************************/
#include "!Set Prec Struct.h"
BUFFER(Hdr)
   Flt  HdrBrightness,
        HdrExp       ,
        HdrMaxDark   ,
        HdrMaxBright ;
   VecH HdrWeight    ;
BUFFER_END

BUFFER(ToneMap)
   Half ToneMapMonitorMaxLum,
        ToneMapTopRange,
        ToneMapDarkenRange,
        ToneMapDarkenExp;
BUFFER_END
#include "!Set Prec Default.h"
/******************************************************************************
// sRGB -> ACEScg - https://www.colour-science.org/apps/
static const MatrixH3 ACESInputMat=
{
   {0.613132422390542, 0.339538015799666, 0.047416696048269},
   {0.070124380833917, 0.916394011313573, 0.013451523958235},
   {0.020587657528185, 0.109574571610682, 0.869785404035327},
};
static const MatrixH3 ACESOutputMat=
{
   {1.7048733135277740, -0.62171816990580908, -0.083326763721652430},
   {-0.13010871309710900, 1.1407022301733414, -0.010548415774357027},
   {-0.023963084814622154, -0.12898841130813682, 1.1530100831323533},
};
/******************************************************************************/
Half TonemapLum(VecH x) {return LinearLumOfLinearColor(x);} // could also be "Avg(x)" to darken bright blue skies
/******************************************************************************/
// All functions below need to start initially with derivative=1, at the start looking like y=x (scale=1), only after that they can bend down

Half  TonemapDiv(Half  x) {return x/(1+x);} // x=0..Inf, returns 0..1
VecH  TonemapDiv(VecH  x) {return x/(1+x);} // x=0..Inf, returns 0..1
VecH4 TonemapDiv(VecH4 x) {return x/(1+x);} // x=0..Inf, returns 0..1

Half  TonemapSqr(Half  x) {return SigmoidSqr(x);} // x=0..Inf, returns 0..1
VecH  TonemapSqr(VecH  x) {return SigmoidSqr(x);} // x=0..Inf, returns 0..1
VecH4 TonemapSqr(VecH4 x) {return SigmoidSqr(x);} // x=0..Inf, returns 0..1

Half  TonemapPow(Half  x, Half exp) {return x/Pow(1+Pow(x, exp), 1/exp);}
VecH  TonemapPow(VecH  x, Half exp) {return x/Pow(1+Pow(x, exp), 1/exp);}
VecH4 TonemapPow(VecH4 x, Half exp) {return x/Pow(1+Pow(x, exp), 1/exp);}

Half  TonemapExp(Half  x) {return SigmoidExp(x);} // x=0..Inf, returns 0..1
VecH  TonemapExp(VecH  x) {return SigmoidExp(x);} // x=0..Inf, returns 0..1
VecH4 TonemapExp(VecH4 x) {return SigmoidExp(x);} // x=0..Inf, returns 0..1

Half  TonemapExpA(Half  x) {return 1-Exp(-x);} // x=0..Inf, returns 0..1
VecH  TonemapExpA(VecH  x) {return 1-Exp(-x);} // x=0..Inf, returns 0..1
VecH4 TonemapExpA(VecH4 x) {return 1-Exp(-x);} // x=0..Inf, returns 0..1

Half  TonemapLog(Half  x) {return Log(1+x);} // x=0..Inf, returns 0..Inf
VecH  TonemapLog(VecH  x) {return Log(1+x);} // x=0..Inf, returns 0..Inf
VecH4 TonemapLog(VecH4 x) {return Log(1+x);} // x=0..Inf, returns 0..Inf

Half  TonemapLog2(Half  x) {return Log2(1+x/LOG2_E);} // x=0..Inf, returns 0..Inf
VecH  TonemapLog2(VecH  x) {return Log2(1+x/LOG2_E);} // x=0..Inf, returns 0..Inf
VecH4 TonemapLog2(VecH4 x) {return Log2(1+x/LOG2_E);} // x=0..Inf, returns 0..Inf

Half  TonemapAtan(Half  x) {return SigmoidAtan(x);} // x=0..Inf, returns 0..1
VecH  TonemapAtan(VecH  x) {return SigmoidAtan(x);} // x=0..Inf, returns 0..1
VecH4 TonemapAtan(VecH4 x) {return SigmoidAtan(x);} // x=0..Inf, returns 0..1
/******************************************************************************/
// Max Lum versions
Half  _TonemapDiv(Half  x, Half max_lum) {return (1+x/Sqr(max_lum))/(1+x);} // Max Lum version internal without "x*", x=0..max_lum
VecH  _TonemapDiv(VecH  x, Half max_lum) {return (1+x/Sqr(max_lum))/(1+x);} // Max Lum version internal without "x*", x=0..max_lum
VecH4 _TonemapDiv(VecH4 x, Half max_lum) {return (1+x/Sqr(max_lum))/(1+x);} // Max Lum version internal without "x*", x=0..max_lum

Half  TonemapDiv(Half  x, Half max_lum) {return x*_TonemapDiv(x, max_lum);} // Max Lum version, x=0..max_lum, returns 0..1
VecH  TonemapDiv(VecH  x, Half max_lum) {return x*_TonemapDiv(x, max_lum);} // Max Lum version, x=0..max_lum, returns 0..1
VecH4 TonemapDiv(VecH4 x, Half max_lum) {return x*_TonemapDiv(x, max_lum);} // Max Lum version, x=0..max_lum, returns 0..1

Half  TonemapDiv1(Half  x, Half max_lum) {return x/(1+x*((max_lum-1)/max_lum));} // Max Lum version, x=0..max_lum, returns 0..1
VecH  TonemapDiv1(VecH  x, Half max_lum) {return x/(1+x*((max_lum-1)/max_lum));} // Max Lum version, x=0..max_lum, returns 0..1
VecH4 TonemapDiv1(VecH4 x, Half max_lum) {return x/(1+x*((max_lum-1)/max_lum));} // Max Lum version, x=0..max_lum, returns 0..1

/*
SigmoidSqr(max_lum*mul)/mul=1
max_lum*mul/Sqrt(1+max_lum*max_lum*mul*mul)/mul=1
max_lum/Sqrt(1+max_lum*max_lum*mul*mul)=1
max_lum/Sqrt(1+max_lum*max_lum*mul*mul)=1
max_lum=Sqrt(1+max_lum*max_lum*mul*mul)
max_lum*max_lum=1+max_lum*max_lum*mul*mul
(max_lum*max_lum-1)/(max_lum*max_lum)=mul*mul
mul=Sqrt((max_lum*max_lum-1)/(max_lum*max_lum))
mul=Sqrt(max_lum*max_lum-1)/max_lum
*/
Half  SigmoidSqrMul(Half max_lum) {return Sqrt(max_lum*max_lum-1)/max_lum;}
Half  TonemapSqr(Half  x, Half max_lum) {Half mul=SigmoidSqrMul(max_lum); return SigmoidSqr(x*mul)/mul;} // x=0..max_lum, returns 0..1
VecH  TonemapSqr(VecH  x, Half max_lum) {Half mul=SigmoidSqrMul(max_lum); return SigmoidSqr(x*mul)/mul;} // x=0..max_lum, returns 0..1
VecH4 TonemapSqr(VecH4 x, Half max_lum) {Half mul=SigmoidSqrMul(max_lum); return SigmoidSqr(x*mul)/mul;} // x=0..max_lum, returns 0..1

/* Constants were calculated to have derivative=1:
here if internally 'TonemapLog' or 'TonemapLog2' is used, it doesn't matter, results are the same, however 'mul' has to be calculated differently, so choose the func that's faster on GPU
Dbl TonemapLog (Dbl x) {return Log (1+x       );} // x=0..Inf
Dbl TonemapLog2(Dbl x) {return Log2(1+x/LOG2_E);} // x=0..Inf
void InitPre()
{
   Dbl mul=1, min=0, max=16, max_lum=8;
   REP(65536)
   {
      mul=Avg(min, max);
      Dbl t=TonemapLog(max_lum*mul)/mul;
      if(t>1)min=mul;
      if(t<1)max=mul;
   }
   Dbl t=TonemapLog(max_lum*mul)/mul; // 't' should be 1
*/
Half  TonemapLogML2(Half  x) {Half mul=1.2564312086261697; return TonemapLog(x*mul)/mul;} // x=0..2, returns 0..1
VecH  TonemapLogML2(VecH  x) {Half mul=1.2564312086261697; return TonemapLog(x*mul)/mul;} // x=0..2, returns 0..1
VecH4 TonemapLogML2(VecH4 x) {Half mul=1.2564312086261697; return TonemapLog(x*mul)/mul;} // x=0..2, returns 0..1

Half  TonemapLogML3(Half  x) {Half mul=1.9038136944403834; return TonemapLog(x*mul)/mul;} // x=0..3, returns 0..1
VecH  TonemapLogML3(VecH  x) {Half mul=1.9038136944403834; return TonemapLog(x*mul)/mul;} // x=0..3, returns 0..1
VecH4 TonemapLogML3(VecH4 x) {Half mul=1.9038136944403834; return TonemapLog(x*mul)/mul;} // x=0..3, returns 0..1

Half  TonemapLogML4(Half  x) {Half mul=2.3366629822630536; return TonemapLog(x*mul)/mul;} // x=0..4, returns 0..1
VecH  TonemapLogML4(VecH  x) {Half mul=2.3366629822630536; return TonemapLog(x*mul)/mul;} // x=0..4, returns 0..1
VecH4 TonemapLogML4(VecH4 x) {Half mul=2.3366629822630536; return TonemapLog(x*mul)/mul;} // x=0..4, returns 0..1

Half  TonemapLogML5(Half  x) {Half mul=2.6603990584636850; return TonemapLog(x*mul)/mul;} // x=0..5, returns 0..1
VecH  TonemapLogML5(VecH  x) {Half mul=2.6603990584636850; return TonemapLog(x*mul)/mul;} // x=0..5, returns 0..1
VecH4 TonemapLogML5(VecH4 x) {Half mul=2.6603990584636850; return TonemapLog(x*mul)/mul;} // x=0..5, returns 0..1

Half  TonemapLogML6(Half  x) {Half mul=2.9183004757830524; return TonemapLog(x*mul)/mul;} // x=0..6, returns 0..1
VecH  TonemapLogML6(VecH  x) {Half mul=2.9183004757830524; return TonemapLog(x*mul)/mul;} // x=0..6, returns 0..1
VecH4 TonemapLogML6(VecH4 x) {Half mul=2.9183004757830524; return TonemapLog(x*mul)/mul;} // x=0..6, returns 0..1

Half  TonemapLogML8(Half  x) {Half mul=3.3148773617860550; return TonemapLog(x*mul)/mul;} // x=0..8, returns 0..1
VecH  TonemapLogML8(VecH  x) {Half mul=3.3148773617860550; return TonemapLog(x*mul)/mul;} // x=0..8, returns 0..1
VecH4 TonemapLogML8(VecH4 x) {Half mul=3.3148773617860550; return TonemapLog(x*mul)/mul;} // x=0..8, returns 0..1

Half  TonemapLogML16(Half  x) {Half mul=4.2292934127543553; return TonemapLog(x*mul)/mul;} // x=0..16, returns 0..1
VecH  TonemapLogML16(VecH  x) {Half mul=4.2292934127543553; return TonemapLog(x*mul)/mul;} // x=0..16, returns 0..1
VecH4 TonemapLogML16(VecH4 x) {Half mul=4.2292934127543553; return TonemapLog(x*mul)/mul;} // x=0..16, returns 0..1

Half  TonemapAtanML4(Half  x) {Half mul=1.3932490753255884; return Atan(x*mul)/mul;} // x=0..4, returns 0..1
VecH  TonemapAtanML4(VecH  x) {Half mul=1.3932490753255884; return Atan(x*mul)/mul;} // x=0..4, returns 0..1
VecH4 TonemapAtanML4(VecH4 x) {Half mul=1.3932490753255884; return Atan(x*mul)/mul;} // x=0..4, returns 0..1

Half  TonemapAtanML8(Half  x) {Half mul=1.4869275602717664; return Atan(x*mul)/mul;} // x=0..8, returns 0..1
VecH  TonemapAtanML8(VecH  x) {Half mul=1.4869275602717664; return Atan(x*mul)/mul;} // x=0..8, returns 0..1
VecH4 TonemapAtanML8(VecH4 x) {Half mul=1.4869275602717664; return Atan(x*mul)/mul;} // x=0..8, returns 0..1

// here 'mul' can be ignored because in tests it was 0.983722866 for max_lum=4, and 1.00362146 for max_lum=16, for max_lum>=4 they look almost identical, so maybe no need to use them
Half  TonemapExpA(Half  x, Half max_lum) {return TonemapExpA(x)/TonemapExpA(max_lum);}
VecH  TonemapExpA(VecH  x, Half max_lum) {return TonemapExpA(x)/TonemapExpA(max_lum);}
VecH4 TonemapExpA(VecH4 x, Half max_lum) {return TonemapExpA(x)/TonemapExpA(max_lum);}
/******************************************************************************/
VecH TonemapDivLum(VecH x              ) {Half lum=TonemapLum(x); return x/(1+lum)                  ;} // optimized "x*(TonemapDiv(lum         )/lum)", x=0..Inf    , returns 0..1
VecH TonemapDivLum(VecH x, Half max_lum) {Half lum=TonemapLum(x); return x*_TonemapDiv(lum, max_lum);} // optimized "x*(TonemapDiv(lum, max_lum)/lum)", x=0..max_lum, returns 0..1

VecH TonemapDivSat(VecH x) // preserves saturation
{
   VecH d=TonemapDiv   (x); // desaturated, per channel
   VecH s=TonemapDivLum(x); //   saturated, luminance based
   return Lerp(s, d, d);
}
VecH TonemapDivSat(VecH x, Half max_lum) // preserves saturation
{
   VecH d=TonemapDiv   (x, max_lum); // desaturated, per channel
   VecH s=TonemapDivLum(x, max_lum); //   saturated, luminance based
   return Lerp(s, d, d);
}

VecH TonemapLogSat(VecH x)
{
   VecH4 rgbl=VecH4(x, TonemapLum(x));
   VecH4 d=TonemapLog(rgbl);            // desaturated, per channel
   VecH  s=rgbl.w ? x*(d.w/rgbl.w) : 0; //   saturated, luminance based
   return Lerp(s, d.rgb, d.rgb);
}
VecH TonemapLogML2Sat(VecH x)
{
   VecH4 rgbl=VecH4(x, TonemapLum(x));
   VecH4 d=TonemapLogML2(rgbl);         // desaturated, per channel
   VecH  s=rgbl.w ? x*(d.w/rgbl.w) : 0; //   saturated, luminance based
   return Lerp(s, d.rgb, d.rgb);
}
VecH TonemapLogML3Sat(VecH x)
{
   VecH4 rgbl=VecH4(x, TonemapLum(x));
   VecH4 d=TonemapLogML3(rgbl);         // desaturated, per channel
   VecH  s=rgbl.w ? x*(d.w/rgbl.w) : 0; //   saturated, luminance based
   return Lerp(s, d.rgb, d.rgb);
}
VecH TonemapLogML4Sat(VecH x)
{
   VecH4 rgbl=VecH4(x, TonemapLum(x));
   VecH4 d=TonemapLogML4(rgbl);         // desaturated, per channel
   VecH  s=rgbl.w ? x*(d.w/rgbl.w) : 0; //   saturated, luminance based
   return Lerp(s, d.rgb, d.rgb);
}
VecH TonemapLogML5Sat(VecH x)
{
   VecH4 rgbl=VecH4(x, TonemapLum(x));
   VecH4 d=TonemapLogML5(rgbl);         // desaturated, per channel
   VecH  s=rgbl.w ? x*(d.w/rgbl.w) : 0; //   saturated, luminance based
   return Lerp(s, d.rgb, d.rgb);
}
VecH TonemapLogML6Sat(VecH x)
{
   VecH4 rgbl=VecH4(x, TonemapLum(x));
   VecH4 d=TonemapLogML6(rgbl);         // desaturated, per channel
   VecH  s=rgbl.w ? x*(d.w/rgbl.w) : 0; //   saturated, luminance based
   return Lerp(s, d.rgb, d.rgb);
}
VecH TonemapLogML8Sat(VecH x)
{
   VecH4 rgbl=VecH4(x, TonemapLum(x));
   VecH4 d=TonemapLogML8(rgbl);         // desaturated, per channel
   VecH  s=rgbl.w ? x*(d.w/rgbl.w) : 0; //   saturated, luminance based
   return Lerp(s, d.rgb, d.rgb);
}
VecH TonemapLogML16Sat(VecH x)
{
   VecH4 rgbl=VecH4(x, TonemapLum(x));
   VecH4 d=TonemapLogML16(rgbl);        // desaturated, per channel
   VecH  s=rgbl.w ? x*(d.w/rgbl.w) : 0; //   saturated, luminance based
   return Lerp(s, d.rgb, d.rgb);
}
/******************************************************************************/
VecH TonemapEsenthel(VecH x)
{
   Half end=ToneMapMonitorMaxLum, start=end-ToneMapTopRange;
 //VecH f=Max(0, LerpR(start, end, x)); // max 0 needed because negative colors are not allowed and may cause artifacts
   VecH f=Max(0, (x-start)/ToneMapTopRange); // max 0 needed because negative colors are not allowed and may cause artifacts

   VecH l=TonemapLogML8Sat(f); // have to use 'f' instead of "x-start" because that would break continuity
#if 0 // testing
   if(Mode==1)l=TonemapDivSat   (f, 3);
   if(Mode==2)l=TonemapDivSat   (f, 4);
   if(Mode==3)l=TonemapDivSat   (f, 5);
   if(Mode==4)l=TonemapDivSat   (f, 8);
   if(Mode==5)l=TonemapLogML3Sat(f);
   if(Mode==6)l=TonemapLogML4Sat(f);
   if(Mode==7)l=TonemapLogML5Sat(f);
   if(Mode==8)l=TonemapLogML8Sat(f);
   if(Mode==9)l=TonemapExpA     (f);
#endif

 //x=(x>start ? Lerp(start, end, l) : x);
   x=(x>start ? l*ToneMapTopRange+start : x);
   return x;
}
/******************************************************************************
AMD Tonemapper
AMD Cauldron code
Copyright(c) 2020 Advanced Micro Devices, Inc.All rights reserved.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. *
Half ColToneB(Half hdrMax, Half contrast, Half shoulder, Half midIn, Half midOut) // General tonemapping operator, build 'b' term.
{
   return -((-Pow(midIn, contrast) + (midOut*(Pow(hdrMax, contrast*shoulder)*Pow(midIn, contrast) -
              Pow(hdrMax, contrast)*Pow(midIn, contrast*shoulder)*midOut)) /
             (Pow(hdrMax, contrast*shoulder)*midOut - Pow(midIn, contrast*shoulder)*midOut)) /
             (Pow(midIn, contrast*shoulder)*midOut));
}
Half ColToneC(Half hdrMax, Half contrast, Half shoulder, Half midIn, Half midOut) // General tonemapping operator, build 'c' term.
{
    return (Pow(hdrMax, contrast*shoulder)*Pow(midIn, contrast) - Pow(hdrMax, contrast)*Pow(midIn, contrast*shoulder)*midOut) /
           (Pow(hdrMax, contrast*shoulder)*midOut - Pow(midIn, contrast*shoulder)*midOut);
}
Half ColTone(Half x, VecH4 p) // General tonemapping operator, p := {contrast,shoulder,b,c}.
{ 
   Half   z= Pow(x, p.r); 
   return z/(Pow(z, p.g)*p.b + p.a); 
}
VecH TonemapAMD_Cauldron(VecH col, Half Contrast=0) // Contrast=0..1, 1=desaturates too much, so don't use, can use 0 and 0.5
{
   //Contrast=SH/2;
   const Half hdrMax  =MAX_LUM; // How much HDR range before clipping. HDR modes likely need this pushed up to say 25.0.
   const Half shoulder=1; // Likely don't need to mess with this factor, unless matching existing tonemapper is not working well..
   const Half contrast=Lerp(1+1.0/16, 1+2.0/3, Contrast); // good values are 1+1.0/16=darks closest to original, 1+2.0/3=matches ACES
   const Half midIn   =0.18; // most games will have a {0.0 to 1.0} range for LDR so midIn should be 0.18.
   const Half midOut  =0.18; // Use for LDR. For HDR10 10:10:10:2 use maybe 0.18/25.0 to start. For scRGB, I forget what a good starting point is, need to re-calculate.

   Half b=ColToneB(hdrMax, contrast, shoulder, midIn, midOut);
   Half c=ColToneC(hdrMax, contrast, shoulder, midIn, midOut);

   Half peak =Max(Max(col), HALF_MIN);
   VecH ratio=col/peak; // always 0..1
   peak=ColTone(peak, VecH4(contrast, shoulder, b, c)); // should be 0..1

   // ratio
   if(1) // better quality
   {
    //Half crossTalk      = 4; // controls amount of channel crossTalk
      Half      saturation= 1; // full tonal range saturation control, "1" works better (saturation looks closer to original) than "contrast" from original source code
      Half crossSaturation=16; // crossTalk saturation, using "16" or "contrast*16" as in original source code made no visual difference, so keep 16 as it might be faster

      // wrap crossTalk in transform
      ratio=Pow (ratio, saturation/crossSaturation); // ratio 0..1
      ratio=Lerp(ratio, 1, Quart(peak)); // Pow(peak, crossTalk), ratio 0..1
      ratio=Pow (ratio, crossSaturation); // ratio 0..1
   }else // faster but lower saturation for high values
      ratio=Lerp(ratio, 1, Quart(peak)); // ratio 0..1

   col=peak*ratio;
   return col;
}
/******************************************************************************
VecH TonemapAMD_LPM(VecH col) currently disabled on the CPU side
{
   LpmFilter(col.r, col.g, col.b, false, LPM_CONFIG_709_709);
   return col;
}
/******************************************************************************
Desaturates
VecH _TonemapHable(VecH x) // http://filmicworlds.com/blog/filmic-tonemapping-operators/
{
   Half A=0.15;
   Half B=0.50;
   Half C=0.10;
   Half D=0.20;
   Half E=0.02;
   Half F=0.30;
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}
VecH TonemapHable(VecH col)
{
#if MAX_LUM // col=0..MAX_LUM
   #if   MAX_LUM==2
      Half scale=1.74936676;
   #elif MAX_LUM==4
      Half scale=2.55470848;
   #elif MAX_LUM==6
      Half scale=2.82278252;
   #elif MAX_LUM==8
      Half scale=2.95678377;
   #elif MAX_LUM==12
      Half scale=3.09075975;
   #elif MAX_LUM==16
      Half scale=3.15773392;
   #elif MAX_LUM==24
      Half scale=3.22470760;
   #elif MAX_LUM==32
      Half scale=3.25820065;
   #endif
   return _TonemapHable(scale*col)/_TonemapHable(scale*MAX_LUM);
#else // no limit = 0..Inf, this version is very similar to 'TonemapReinhard'
   return _TonemapHable(3.35864878*col)/0.93333333333; // max value of _TonemapHable is 0.93333333333 calculated based on _TonemapHable(65536*256).x
#endif
}
/* scale calculated using:
Vec TonemapHableNoLimit(Vec col, Flt scale)
{
   return _TonemapHable(scale*col)/0.93333333333;
}
Vec TonemapHableMaxLum(Vec col, Flt scale)
{
   return _TonemapHable(scale*col)/_TonemapHable(scale*MAX_LUM);
}
Flt scale=1, min=0, max=16;
REP(65536)
{
   scale=Avg(min, max);
   Flt x=1.0/256, h=TonemapHable(x, scale).x;
   if(h<x)min=scale;
   if(h>x)max=scale;
}
/******************************************************************************
VecH _TonemapUchimura(VecH x, Half P, Half a, Half m, Half l, Half c, Half b) // Uchimura 2017, "HDR theory and practice"
{  // Math: https://www.desmos.com/calculator/gslcdxvipg
   // Source: https://www.slideshare.net/nikuque/hdr-theory-and-practicce-jp
   Half l0 = ((P - m) * l) / a;
   Half L0 = m - m / a;
   Half L1 = m + (1 - m) / a;
   Half S0 = m + l0;
   Half S1 = m + a * l0;
   Half C2 = (a * P) / (P - S1);
   Half CP = -C2 / P;

   VecH w0 = 1 - smoothstep(0, m, x);
   VecH w2 = step(m + l0, x);
   VecH w1 = 1 - w0 - w2;

   VecH T = m * Pow(x / m, c) + b;
   VecH S = P - (P - S1) * Exp(CP * (x - S0));
   VecH L = m + a * (x - m);

   return T * w0 + L * w1 + S * w2;
}
VecH TonemapUchimura(VecH x, Half black=1) // 'black' can also be 1.33
{
   const Half P=1;     // max display brightness
   const Half a=1;     // contrast
   const Half m=0.22;  // linear section start
   const Half l=0.4;   // linear section length
   const Half c=black; // black
   const Half b=0;     // pedestal
   return _TonemapUchimura(x, P, a, m, l, c, b);
}
/******************************************************************************/
VecH TonemapACES_LDR_Narkowicz(VecH x) // returns 0..1 (0..80 nits), Krzysztof Narkowicz - https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
{
   x*=0.72; // 0.72 matches SigmoidSqr 0.8 contrast and UE5, 0.8 matches UE4
   x=Min(x, 160); // for Half, values bigger than 160 will result in Infinity

   Half a=2.51;
   Half b=0.03;
   Half c=2.43;
   Half d=0.59;
   Half e=0.14;

   return (x*(a*x+b))/(x*(c*x+d)+e);
}
VecH TonemapACES_HDR_Narkowicz(VecH x) // returns 0 .. 12.5 (0..1000 nits), Krzysztof Narkowicz - https://knarkowicz.wordpress.com/2016/08/31/hdr-display-first-steps/
{
   x*=0.62; // 0.62 matches SigmoidSqr 0.6 contrast, 0.6 matches original ACES
   x=Min(x, 64); // for Half, values bigger than 64 will result in Infinity

   Half a=15.8;
   Half b=2.12;
   Half c=1.2;
   Half d=5.92;
   Half e=1.9;
   x=(x*(a*x+b))/(x*(c*x+d)+e);
   return x;
}
/******************************************************************************
static const MatrixH3 ACESInputMat= // sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
{
   {0.59719, 0.35458, 0.04823},
   {0.07600, 0.90834, 0.01566},
   {0.02840, 0.13383, 0.83777},
};
static const MatrixH3 ACESOutputMat= // ODT_SAT => XYZ => D60_2_D65 => sRGB
{
   { 1.60475, -0.53108, -0.07367},
   {-0.10208,  1.10813, -0.00605},
   {-0.00327, -0.07276,  1.07602},
};
VecH RRTAndODTFit(VecH v)
{
   VecH a=v*(v+0.0245786)-0.000090537;
   VecH b=v*(0.983729*v+0.4329510)+0.238081;
   return a/b;
}
VecH TonemapACESHill(VecH color) // Stephen Hill "self_shadow", desaturates too much
{
   color=mul(ACESInputMat, color);
   color=RRTAndODTFit(color); // Apply RRT and ODT
   color=mul(ACESOutputMat, color);
   color=Max(0, color);
   return color;
}
/******************************************************************************
VecH TonemapACESLottes(VecH x) // Timothy Lottes "Advanced Techniques and Optimization of HDR Color Pipelines" - https://gpuopen.com/wp-content/uploads/2016/03/GdcVdrLottes.pdf
{
   const Half a     =1.28;
   const Half d     =1.24;
   const Half hdrMax=1;
   const Half midIn =0.18;
   const Half midOut=0.18;

   // can be precomputed
   const Half b = (-Pow(midIn, a) + Pow(hdrMax, a) * midOut) / ((Pow(hdrMax, a * d) - Pow(midIn, a * d)) * midOut);
   const Half c = (Pow(hdrMax, a * d) * Pow(midIn, a) - Pow(hdrMax, a) * Pow(midIn, a * d) * midOut) / ((Pow(hdrMax, a * d) - Pow(midIn, a * d)) * midOut);

   const Bool lum=false;
   if(lum)
   {
      Half l=Max(x); // don't use l=TonemapLum(x); because it darkens compared to non-lum version
      if(CanDiv(l))
      {
         l=Min(l, hdrMax); // if value is outside hdrMax range, then artifacts can occur and colors can get actually darker and black
         Half o=l;
         l=Pow(l, a)/(Pow(l, a*d)*b+c);
         x*=l/o;
      }
   }else
   {
      x=Min(x, hdrMax); // if value is outside hdrMax range, then artifacts can occur and colors can get actually darker and black
      x=Pow(x, a)/(Pow(x, a*d)*b+c);
   }
   return x;
}
/******************************************************************************
VecH TonemapUnreal(VecH x) // Unreal 3, Documentation: "Color Grading", adapted to be close to TonemapACES with similar range
{
   x=x/(x+0.155)*1.019;
   return SRGBToLinear(x);
}
/******************************************************************************
VecH ToneMapHejl(VecH col) // too dark
{
   VecH4  vh=VecH4(col, MAX_LUM);
   VecH4  va=(1.435*vh)+0.05;
   VecH4  vf=((vh*va+0.004)/((vh*(va+0.55)+0.0491)))-0.0821;
   return vf.xyz/vf.w;
}
/******************************************************************************
OK but Esenthel is better
VecH ToneMapHejlBurgessDawson(VecH col) // Jim Hejl + Richard Burgess-Dawson "Filmic" - http://filmicworlds.com/blog/filmic-tonemapping-operators/
{
   col=Max(0, col-0.004);
   col=(col*(6.2*col+0.5))/(col*(6.2*col+1.7)+0.06);
   return SRGBToLinear(col);
}
/******************************************************************************/
