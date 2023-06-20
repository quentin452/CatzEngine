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
/******************************************************************************/
#define MUL (1)
static const MatrixH3 ACESInputMat= // sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
{
   {0.59719*MUL, 0.35458*MUL, 0.04823*MUL},
   {0.07600*MUL, 0.90834*MUL, 0.01566*MUL},
   {0.02840*MUL, 0.13383*MUL, 0.83777*MUL},
};
#undef MUL
static const MatrixH3 ACESOutputMat= // ODT_SAT => XYZ => D60_2_D65 => sRGB
{
   { 1.60475, -0.53108, -0.07367},
   {-0.10208,  1.10813, -0.00605},
   {-0.00327, -0.07276,  1.07602},
};
/******************************************************************************/
void DarkenDarks(inout VecH x)
{
   VecH step=Sat(x/ToneMapDarkenRange);
   x=Lerp(Pow(step, ToneMapDarkenExp)*ToneMapDarkenRange, x, Sqr(step)); // alternative: SmoothCube(step), but it's more expensive and only a small difference, not necessarily better
}
/******************************************************************************/
Half TonemapLum(VecH x) {return LinearLumOfLinearColor(x);} // could also be "Avg(x)" to darken bright blue skies
/******************************************************************************/
Half  TonemapRcp(Half  x) {return x/(1+x);} // x=0..Inf
VecH  TonemapRcp(VecH  x) {return x/(1+x);} // x=0..Inf
VecH4 TonemapRcp(VecH4 x) {return x/(1+x);} // x=0..Inf

Half  TonemapRcpSqr(Half  x) {return SigmoidSqrt(x);} // x=-Inf..Inf
VecH  TonemapRcpSqr(VecH  x) {return SigmoidSqrt(x);} // x=-Inf..Inf
VecH4 TonemapRcpSqr(VecH4 x) {return SigmoidSqrt(x);} // x=-Inf..Inf

/* constants were calculated to have linear growth at the start, similar to y=x, using:
Half TonemapLog(Half x) {return Log2(1+x);}

Flt scale=1, min=0, max=16;
Flt x=1.0/65536;
REP(65536)
{
   scale=Avg(min, max);
   Flt h=TonemapLog(x*scale);
   if(h<x)min=scale;
   if(h>x)max=scale;
}
Flt z=TonemapLog(x*scale)/x;

Half TonemapLog(Half x) {return Log2(1+x*0.69140625);} // x=0..Inf

Half TonemapLog(Half x, Half max_lum, Half mul) {return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}

Flt scale=1, min=0, max=16, max_lum=4;
Flt x=1.0/65536;
REP(65536)
{
   scale=Avg(min, max);
   Flt h=TonemapLog(x, max_lum, scale);
   if(h<x)min=scale;
   if(h>x)max=scale;
}
Flt z=TonemapLog(x, max_lum, scale)/x; */

Half  TonemapLog(Half  x) {return Log2(1+x*0.69140625);} // x=0..Inf
VecH  TonemapLog(VecH  x) {return Log2(1+x*0.69140625);} // x=0..Inf
VecH4 TonemapLog(VecH4 x) {return Log2(1+x*0.69140625);} // x=0..Inf

Half  TonemapExp(Half  x) {return 1-Exp(-x);} // x=-Inf..Inf
VecH  TonemapExp(VecH  x) {return 1-Exp(-x);} // x=-Inf..Inf
VecH4 TonemapExp(VecH4 x) {return 1-Exp(-x);} // x=-Inf..Inf

// Max Lum versions
Half  _TonemapRcp(Half  x, Half max_lum) {return (1+x/Sqr(max_lum))/(1+x);} // Max Lum version x=0..max_lum - internal without "x*"
VecH  _TonemapRcp(VecH  x, Half max_lum) {return (1+x/Sqr(max_lum))/(1+x);} // Max Lum version x=0..max_lum - internal without "x*"
VecH4 _TonemapRcp(VecH4 x, Half max_lum) {return (1+x/Sqr(max_lum))/(1+x);} // Max Lum version x=0..max_lum - internal without "x*"

Half  TonemapRcp(Half  x, Half max_lum) {return x*_TonemapRcp(x, max_lum);} // Max Lum version x=0..max_lum
VecH  TonemapRcp(VecH  x, Half max_lum) {return x*_TonemapRcp(x, max_lum);} // Max Lum version x=0..max_lum
VecH4 TonemapRcp(VecH4 x, Half max_lum) {return x*_TonemapRcp(x, max_lum);} // Max Lum version x=0..max_lum

VecH TonemapRcpLum(VecH x              ) {Half lum=TonemapLum(x); return x/(1+lum)                  ;} // optimized "x*(TonemapRcp(lum         )/lum)"
VecH TonemapRcpLum(VecH x, Half max_lum) {Half lum=TonemapLum(x); return x*_TonemapRcp(lum, max_lum);} // optimized "x*(TonemapRcp(lum, max_lum)/lum)"

VecH TonemapRcpSat(VecH x) // preserves saturation
{
   VecH d=TonemapRcp   (x); // desaturated, per channel
   VecH s=TonemapRcpLum(x); //   saturated, luminance based
   return Lerp(s, d, d);
}
VecH TonemapRcpSat(VecH x, Half max_lum) // preserves saturation
{
   VecH d=TonemapRcp   (x, max_lum); // desaturated, per channel
   VecH s=TonemapRcpLum(x, max_lum); //   saturated, luminance based
   return Lerp(s, d, d);
}

Half  TonemapLogML2(Half  x) {Half mul=1.81725168, max_lum=2; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH  TonemapLogML2(VecH  x) {Half mul=1.81725168, max_lum=2; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH4 TonemapLogML2(VecH4 x) {Half mul=1.81725168, max_lum=2; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}

Half  TonemapLogML3(Half  x) {Half mul=2.75361061, max_lum=3; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH  TonemapLogML3(VecH  x) {Half mul=2.75361061, max_lum=3; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH4 TonemapLogML3(VecH4 x) {Half mul=2.75361061, max_lum=3; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}

Half  TonemapLogML4(Half  x) {Half mul=3.37967825, max_lum=4; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH  TonemapLogML4(VecH  x) {Half mul=3.37967825, max_lum=4; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH4 TonemapLogML4(VecH4 x) {Half mul=3.37967825, max_lum=4; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}

Half  TonemapLogML5(Half  x) {Half mul=3.84792900, max_lum=5; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH  TonemapLogML5(VecH  x) {Half mul=3.84792900, max_lum=5; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH4 TonemapLogML5(VecH4 x) {Half mul=3.84792900, max_lum=5; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}

Half  TonemapLogML6(Half  x) {Half mul=4.22095823, max_lum=6; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH  TonemapLogML6(VecH  x) {Half mul=4.22095823, max_lum=6; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH4 TonemapLogML6(VecH4 x) {Half mul=4.22095823, max_lum=6; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}

Half  TonemapLogML8(Half  x) {Half mul=4.79456997, max_lum=8; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH  TonemapLogML8(VecH  x) {Half mul=4.79456997, max_lum=8; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH4 TonemapLogML8(VecH4 x) {Half mul=4.79456997, max_lum=8; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}

Half  TonemapLogML16(Half  x) {Half mul=6.11720181, max_lum=16; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH  TonemapLogML16(VecH  x) {Half mul=6.11720181, max_lum=16; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}
VecH4 TonemapLogML16(VecH4 x) {Half mul=6.11720181, max_lum=16; return TonemapLog(mul*x)/TonemapLog(mul*max_lum);}

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

// here 'mul' can be ignored because in tests it was 0.983722866 for max_lum=4, and 1.00362146 for max_lum=16, for max_lum>=4 they look almost identical, so maybe no need to use them
Half  TonemapExp(Half  x, Half max_lum) {return TonemapExp(x)/TonemapExp(max_lum);}
VecH  TonemapExp(VecH  x, Half max_lum) {return TonemapExp(x)/TonemapExp(max_lum);}
VecH4 TonemapExp(VecH4 x, Half max_lum) {return TonemapExp(x)/TonemapExp(max_lum);}
/******************************************************************************/
VecH ToneMapSigmoid(VecH x)
{ // here 'SigmoidExp' works better than 'SigmoidSqrt' because sqrt introduces too much highlights on already bright colors
   x=mul(ACESInputMat, x); // convert to ACES and prevent saturation
   Half contrast=2, brightness=2;
   x=SigmoidExp(Vec(x*brightness))/SigmoidExp(brightness); // need to use high precision
   x=x*2-1;
   x=SigmoidExp(Vec(x*contrast  ))/SigmoidExp(contrast  ); // need to use high precision
   x=x*0.5+0.5;
   x=mul(ACESOutputMat, x); x=Max(0, x);
   return x;
}
/******************************************************************************/
VecH TonemapEsenthel(VecH x)
{
   Half end=ToneMapMonitorMaxLum, start=end-ToneMapTopRange;
 //VecH f=Max(0, LerpR(start, end, x)); // max 0 needed because negative colors are not allowed and may cause artifacts
   VecH f=Max(0, (x-start)/ToneMapTopRange); // max 0 needed because negative colors are not allowed and may cause artifacts

   VecH l=TonemapLogML8Sat(f); // have to use 'f' instead of "x-start" because that would break continuity
#if 0 // testing
   if(Mode==1)l=TonemapRcpSat   (f, 3);
   if(Mode==2)l=TonemapRcpSat   (f, 4);
   if(Mode==3)l=TonemapRcpSat   (f, 5);
   if(Mode==4)l=TonemapRcpSat   (f, 8);
   if(Mode==5)l=TonemapLogML3Sat(f);
   if(Mode==6)l=TonemapLogML4Sat(f);
   if(Mode==7)l=TonemapLogML5Sat(f);
   if(Mode==8)l=TonemapLogML8Sat(f);
   if(Mode==9)l=TonemapExp      (f);
#endif

 //x=(x>start ? Lerp(start, end, l) : x);
   x=(x>start ? l*ToneMapTopRange+start : x);
   DarkenDarks(x);
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
   //DarkenDarks(col);
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
   x*=0.8; // everything is too bright, so darken, also this matches UE4
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
   x*=0.7; // 0.7 matches preserving mid tone with TonemapACES_LDR_Narkowicz, 0.6 matches original ACES
   x=Min(x, 64); // for Half, values bigger than 64 will result in Infinity

   Half a=15.8;
   Half b=2.12;
   Half c=1.2;
   Half d=5.92;
   Half e=1.9;
   return (x*(a*x+b))/(x*(c*x+d)+e); // ((x*0.7)*(15.8*(x*0.7)+2.12))/((x*0.7)*(1.2*(x*0.7)+5.92)+1.9)
}
/******************************************************************************
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
after tweaking 'mid' parameters it's almost the same as Narkowicz
VecH TonemapACESLottes(VecH x) // Timothy Lottes "Advanced Techniques and Optimization of HDR Color Pipelines" - https://gpuopen.com/wp-content/uploads/2016/03/GdcVdrLottes.pdf
{
   const Half a     =1.6;
   const Half d     =0.977;
   const Half hdrMax=MAX_LUM;
   const Half midIn =0.18;
   const Half midOut=0.267;

   // can be precomputed
   const Half b = (-Pow(midIn, a) + Pow(hdrMax, a) * midOut) / ((Pow(hdrMax, a * d) - Pow(midIn, a * d)) * midOut);
   const Half c = (Pow(hdrMax, a * d) * Pow(midIn, a) - Pow(hdrMax, a) * Pow(midIn, a * d) * midOut) / ((Pow(hdrMax, a * d) - Pow(midIn, a * d)) * midOut);

   return Pow(x, a)/(Pow(x, a*d)*b+c);
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
