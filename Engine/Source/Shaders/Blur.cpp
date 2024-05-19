/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
// !!
// !! If changing number of samples then change also SHADER_BLUR_RANGE !!
// !!
/******************************************************************************/
#if 1 // Gaussian(2*x) is used because from all functions when using 2 blurs (1X + 1Y) it gives results closest to a circle blur
#define WEIGHT4_0 0.285375185
#define WEIGHT4_1 0.222250419
#define WEIGHT4_2 0.104983666
#define WEIGHT4_3 0.030078322
// WEIGHT4_0 + WEIGHT4_1*2 + WEIGHT4_2*2 + WEIGHT4_3*2 = 1

#define WEIGHT5_0 0.227928684
#define WEIGHT5_1 0.194228007
#define WEIGHT5_2 0.120185056
#define WEIGHT5_3 0.054002630
#define WEIGHT5_4 0.017619965
// WEIGHT5_0 + WEIGHT5_1*2 + WEIGHT5_2*2 + WEIGHT5_3*2 + WEIGHT5_4*2 = 1

#define WEIGHT6_0 0.189746915
#define WEIGHT6_1 0.169792994
#define WEIGHT6_2 0.121662002
#define WEIGHT6_3 0.069803991
#define WEIGHT6_4 0.032069751
#define WEIGHT6_5 0.011797805
// WEIGHT6_0 + WEIGHT6_1*2 + WEIGHT6_2*2 + WEIGHT6_3*2 + WEIGHT6_4*2 + WEIGHT6_5*2 = 1
#else // BlendSmoothSin
#define WEIGHT4_0 0.250000000
#define WEIGHT4_1 0.213388354
#define WEIGHT4_2 0.124999993
#define WEIGHT4_3 0.036611654
// WEIGHT4_0 + WEIGHT4_1*2 + WEIGHT4_2*2 + WEIGHT4_3*2 = 1

#define WEIGHT5_0 0.200000014
#define WEIGHT5_1 0.180901707
#define WEIGHT5_2 0.130901704
#define WEIGHT5_3 0.069098287
#define WEIGHT5_4 0.019098295
// WEIGHT5_0 + WEIGHT5_1*2 + WEIGHT5_2*2 + WEIGHT5_3*2 + WEIGHT5_4*2 = 1

#define WEIGHT6_0 0.166666668
#define WEIGHT6_1 0.155502122
#define WEIGHT6_2 0.125000001
#define WEIGHT6_3 0.083333329
#define WEIGHT6_4 0.041666662
#define WEIGHT6_5 0.011164551
// WEIGHT6_0 + WEIGHT6_1*2 + WEIGHT6_2*2 + WEIGHT6_3*2 + WEIGHT6_4*2 + WEIGHT6_5*2 = 1
#endif

#define TEST_BLUR 0
#if TEST_BLUR
Flt Mode = 4, Range = 5, Test = 1;
Flt Weight(Int i, Int r) {
    Flt frac = i / Flt(r + 1);
    Int mode = Round(Mode);
    if (mode == 1)
        return Cube(1 - frac);
    if (mode == 2)
        return Sqr(1 - frac);
    if (mode == 3)
        return Gaussian(1.5 * frac);
    if (mode == 4)
        return Gaussian(2 * frac); // BEST - results closest to a circle blur
    if (mode == 5)
        return BlendSmoothSin(frac); // nearly identical to BlendSmoothCube
    if (mode == 6)
        return (1 - frac);
    return 1;
}
#endif

#ifndef SAMPLES
#define SAMPLES 0
#endif
/******************************************************************************/
// can use 'RTSize' instead of 'ImgSize' since there's no scale
VecH BlurX_PS(NOPERSP Vec2 uv
              : UV) : TARGET {
#if TEST_BLUR
    if (Test) {
        Int r = Round(Range);
        Vec4 color = 0;
        Flt weight = 0;
        for (Int i = -r; i <= r; i++) {
            Flt w = Weight(Abs(i), r);
            weight += w;
            color.rgb += w * TexPoint(Img, uv + RTSize.xy * Vec2(i, 0)).rgb;
        }
        color.rgb /= weight;
        return color;
    }
#endif
    // use linear filtering because texcoords aren't rounded
    if (SAMPLES == 4)                                                                                                               // -3 .. 3
        return TexLod(Img, uv + RTSize.xy * Vec2(0 + WEIGHT4_1 / (WEIGHT4_0 / 2 + WEIGHT4_1), 0)).rgb * (WEIGHT4_0 / 2 + WEIGHT4_1) // 0th sample is divided by 2 because it's obtained here and line below to preserve symmetry
               + TexLod(Img, uv + RTSize.xy * Vec2(-0 - WEIGHT4_1 / (WEIGHT4_0 / 2 + WEIGHT4_1), 0)).rgb * (WEIGHT4_0 / 2 + WEIGHT4_1) + TexLod(Img, uv + RTSize.xy * Vec2(2 + WEIGHT4_3 / (WEIGHT4_2 + WEIGHT4_3), 0)).rgb * (WEIGHT4_2 + WEIGHT4_3) + TexLod(Img, uv + RTSize.xy * Vec2(-2 - WEIGHT4_3 / (WEIGHT4_2 + WEIGHT4_3), 0)).rgb * (WEIGHT4_2 + WEIGHT4_3);
    if (SAMPLES == 5) // -4 .. 4
        return TexLod(Img, uv).rgb * WEIGHT5_0 + TexLod(Img, uv + RTSize.xy * Vec2(1 + WEIGHT5_2 / (WEIGHT5_1 + WEIGHT5_2), 0)).rgb * (WEIGHT5_1 + WEIGHT5_2) + TexLod(Img, uv + RTSize.xy * Vec2(-1 - WEIGHT5_2 / (WEIGHT5_1 + WEIGHT5_2), 0)).rgb * (WEIGHT5_1 + WEIGHT5_2) + TexLod(Img, uv + RTSize.xy * Vec2(3 + WEIGHT5_4 / (WEIGHT5_3 + WEIGHT5_4), 0)).rgb * (WEIGHT5_3 + WEIGHT5_4) + TexLod(Img, uv + RTSize.xy * Vec2(-3 - WEIGHT5_4 / (WEIGHT5_3 + WEIGHT5_4), 0)).rgb * (WEIGHT5_3 + WEIGHT5_4);
    if (SAMPLES == 6)                                                                                                               // -5 .. 5
        return TexLod(Img, uv + RTSize.xy * Vec2(0 + WEIGHT6_1 / (WEIGHT6_0 / 2 + WEIGHT6_1), 0)).rgb * (WEIGHT6_0 / 2 + WEIGHT6_1) // 0th sample is divided by 2 because it's obtained here and line below to preserve symmetry
               + TexLod(Img, uv + RTSize.xy * Vec2(-0 - WEIGHT6_1 / (WEIGHT6_0 / 2 + WEIGHT6_1), 0)).rgb * (WEIGHT6_0 / 2 + WEIGHT6_1) + TexLod(Img, uv + RTSize.xy * Vec2(2 + WEIGHT6_3 / (WEIGHT6_2 + WEIGHT6_3), 0)).rgb * (WEIGHT6_2 + WEIGHT6_3) + TexLod(Img, uv + RTSize.xy * Vec2(-2 - WEIGHT6_3 / (WEIGHT6_2 + WEIGHT6_3), 0)).rgb * (WEIGHT6_2 + WEIGHT6_3) + TexLod(Img, uv + RTSize.xy * Vec2(4 + WEIGHT6_5 / (WEIGHT6_4 + WEIGHT6_5), 0)).rgb * (WEIGHT6_4 + WEIGHT6_5) + TexLod(Img, uv + RTSize.xy * Vec2(-4 - WEIGHT6_5 / (WEIGHT6_4 + WEIGHT6_5), 0)).rgb * (WEIGHT6_4 + WEIGHT6_5);
    return 0;
}
/******************************************************************************/
VecH BlurY_PS(NOPERSP Vec2 uv
              : UV) : TARGET {
#if TEST_BLUR
    if (Test) {
        Int r = Round(Range);
        Vec4 color = 0;
        Flt weight = 0;
        for (Int i = -r; i <= r; i++) {
            Flt w = Weight(Abs(i), r);
            weight += w;
            color.rgb += w * TexPoint(Img, uv + RTSize.xy * Vec2(0, i)).rgb;
        }
        color.rgb /= weight;
        return color;
    }
#endif
    // use linear filtering because texcoords aren't rounded
    if (SAMPLES == 4)                                                                                                               // -3 .. 3
        return TexLod(Img, uv + RTSize.xy * Vec2(0, 0 + WEIGHT4_1 / (WEIGHT4_0 / 2 + WEIGHT4_1))).rgb * (WEIGHT4_0 / 2 + WEIGHT4_1) // 0th sample is divided by 2 because it's obtained here and line below to preserve symmetry
               + TexLod(Img, uv + RTSize.xy * Vec2(0, -0 - WEIGHT4_1 / (WEIGHT4_0 / 2 + WEIGHT4_1))).rgb * (WEIGHT4_0 / 2 + WEIGHT4_1) + TexLod(Img, uv + RTSize.xy * Vec2(0, 2 + WEIGHT4_3 / (WEIGHT4_2 + WEIGHT4_3))).rgb * (WEIGHT4_2 + WEIGHT4_3) + TexLod(Img, uv + RTSize.xy * Vec2(0, -2 - WEIGHT4_3 / (WEIGHT4_2 + WEIGHT4_3))).rgb * (WEIGHT4_2 + WEIGHT4_3);
    if (SAMPLES == 5) // -4 .. 4
        return TexLod(Img, uv).rgb * WEIGHT5_0 + TexLod(Img, uv + RTSize.xy * Vec2(0, 1 + WEIGHT5_2 / (WEIGHT5_1 + WEIGHT5_2))).rgb * (WEIGHT5_1 + WEIGHT5_2) + TexLod(Img, uv + RTSize.xy * Vec2(0, -1 - WEIGHT5_2 / (WEIGHT5_1 + WEIGHT5_2))).rgb * (WEIGHT5_1 + WEIGHT5_2) + TexLod(Img, uv + RTSize.xy * Vec2(0, 3 + WEIGHT5_4 / (WEIGHT5_3 + WEIGHT5_4))).rgb * (WEIGHT5_3 + WEIGHT5_4) + TexLod(Img, uv + RTSize.xy * Vec2(0, -3 - WEIGHT5_4 / (WEIGHT5_3 + WEIGHT5_4))).rgb * (WEIGHT5_3 + WEIGHT5_4);
    if (SAMPLES == 6)                                                                                                               // -5 .. 5
        return TexLod(Img, uv + RTSize.xy * Vec2(0, 0 + WEIGHT6_1 / (WEIGHT6_0 / 2 + WEIGHT6_1))).rgb * (WEIGHT6_0 / 2 + WEIGHT6_1) // 0th sample is divided by 2 because it's obtained here and line below to preserve symmetry
               + TexLod(Img, uv + RTSize.xy * Vec2(0, -0 - WEIGHT6_1 / (WEIGHT6_0 / 2 + WEIGHT6_1))).rgb * (WEIGHT6_0 / 2 + WEIGHT6_1) + TexLod(Img, uv + RTSize.xy * Vec2(0, 2 + WEIGHT6_3 / (WEIGHT6_2 + WEIGHT6_3))).rgb * (WEIGHT6_2 + WEIGHT6_3) + TexLod(Img, uv + RTSize.xy * Vec2(0, -2 - WEIGHT6_3 / (WEIGHT6_2 + WEIGHT6_3))).rgb * (WEIGHT6_2 + WEIGHT6_3) + TexLod(Img, uv + RTSize.xy * Vec2(0, 4 + WEIGHT6_5 / (WEIGHT6_4 + WEIGHT6_5))).rgb * (WEIGHT6_4 + WEIGHT6_5) + TexLod(Img, uv + RTSize.xy * Vec2(0, -4 - WEIGHT6_5 / (WEIGHT6_4 + WEIGHT6_5))).rgb * (WEIGHT6_4 + WEIGHT6_5);
    return 0;
}
/******************************************************************************/
Half BlurX_X_PS(NOPERSP Vec2 uv
                : UV) : TARGET {
    // use linear filtering because texcoords aren't rounded
    return TexLod(ImgX, uv + RTSize.xy * Vec2(0 + WEIGHT6_1 / (WEIGHT6_0 / 2 + WEIGHT6_1), 0)).x * (WEIGHT6_0 / 2 + WEIGHT6_1) + TexLod(ImgX, uv + RTSize.xy * Vec2(-0 - WEIGHT6_1 / (WEIGHT6_0 / 2 + WEIGHT6_1), 0)).x * (WEIGHT6_0 / 2 + WEIGHT6_1) + TexLod(ImgX, uv + RTSize.xy * Vec2(2 + WEIGHT6_3 / (WEIGHT6_2 + WEIGHT6_3), 0)).x * (WEIGHT6_2 + WEIGHT6_3) + TexLod(ImgX, uv + RTSize.xy * Vec2(-2 - WEIGHT6_3 / (WEIGHT6_2 + WEIGHT6_3), 0)).x * (WEIGHT6_2 + WEIGHT6_3) + TexLod(ImgX, uv + RTSize.xy * Vec2(4 + WEIGHT6_5 / (WEIGHT6_4 + WEIGHT6_5), 0)).x * (WEIGHT6_4 + WEIGHT6_5) + TexLod(ImgX, uv + RTSize.xy * Vec2(-4 - WEIGHT6_5 / (WEIGHT6_4 + WEIGHT6_5), 0)).x * (WEIGHT6_4 + WEIGHT6_5);
}
Half BlurY_X_PS(NOPERSP Vec2 uv
                : UV) : TARGET {
    // use linear filtering because texcoords aren't rounded
    return TexLod(ImgX, uv + RTSize.xy * Vec2(0, 0 + WEIGHT6_1 / (WEIGHT6_0 / 2 + WEIGHT6_1))).x * (WEIGHT6_0 / 2 + WEIGHT6_1) + TexLod(ImgX, uv + RTSize.xy * Vec2(0, -0 - WEIGHT6_1 / (WEIGHT6_0 / 2 + WEIGHT6_1))).x * (WEIGHT6_0 / 2 + WEIGHT6_1) + TexLod(ImgX, uv + RTSize.xy * Vec2(0, 2 + WEIGHT6_3 / (WEIGHT6_2 + WEIGHT6_3))).x * (WEIGHT6_2 + WEIGHT6_3) + TexLod(ImgX, uv + RTSize.xy * Vec2(0, -2 - WEIGHT6_3 / (WEIGHT6_2 + WEIGHT6_3))).x * (WEIGHT6_2 + WEIGHT6_3) + TexLod(ImgX, uv + RTSize.xy * Vec2(0, 4 + WEIGHT6_5 / (WEIGHT6_4 + WEIGHT6_5))).x * (WEIGHT6_4 + WEIGHT6_5) + TexLod(ImgX, uv + RTSize.xy * Vec2(0, -4 - WEIGHT6_5 / (WEIGHT6_4 + WEIGHT6_5))).x * (WEIGHT6_4 + WEIGHT6_5);
}
/******************************************************************************/
// MAX
/******************************************************************************
// can use 'RTSize' instead of 'ImgSize' since there's no scale
VecH4 MaxX_PS(NOPERSP Vec2 uv:UV):TARGET
{
   VecH color; // color=0; for(Int i=0; i<11; i++)color=Max(color, TexPoint(Img, uv+RTSize.xy*Vec2(i-5, 0)).rgb*(BlendWeight[i]/BlendWeight[5])); original slower version
   // use linear filtering because texcoords aren't rounded
   color=           TexLod(Img, uv+RTSize.xy*Vec2( 0+WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1), 0)).rgb ;
   color=Max(color, TexLod(Img, uv+RTSize.xy*Vec2(-0-WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1), 0)).rgb);
   color=Max(color, TexLod(Img, uv+RTSize.xy*Vec2( 2+WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3), 0)).rgb*((WEIGHT6_2+WEIGHT6_3)/(WEIGHT6_0+WEIGHT6_1)));
   color=Max(color, TexLod(Img, uv+RTSize.xy*Vec2(-2-WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3), 0)).rgb*((WEIGHT6_2+WEIGHT6_3)/(WEIGHT6_0+WEIGHT6_1)));
   color=Max(color, TexLod(Img, uv+RTSize.xy*Vec2( 4+WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5), 0)).rgb*((WEIGHT6_4+WEIGHT6_5)/(WEIGHT6_0+WEIGHT6_1)));
   color=Max(color, TexLod(Img, uv+RTSize.xy*Vec2(-4-WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5), 0)).rgb*((WEIGHT6_4+WEIGHT6_5)/(WEIGHT6_0+WEIGHT6_1)));
   return VecH4(color, 0);
}
VecH4 MaxY_PS(NOPERSP Vec2 uv:UV):TARGET
{
   VecH color; // color=0; for(Int i=0; i<11; i++)color=Max(color, TexPoint(Img, uv+RTSize.xy*Vec2(0, i-5)).rgb*(BlendWeight[i]/BlendWeight[5])); original slower version
   // use linear filtering because texcoords aren't rounded
   color=           TexLod(Img, uv+RTSize.xy*Vec2(0,  0+WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1))).rgb ;
   color=Max(color, TexLod(Img, uv+RTSize.xy*Vec2(0, -0-WEIGHT6_1/(WEIGHT6_0/2+WEIGHT6_1))).rgb);
   color=Max(color, TexLod(Img, uv+RTSize.xy*Vec2(0,  2+WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3))).rgb*((WEIGHT6_2+WEIGHT6_3)/(WEIGHT6_0+WEIGHT6_1)));
   color=Max(color, TexLod(Img, uv+RTSize.xy*Vec2(0, -2-WEIGHT6_3/(WEIGHT6_2  +WEIGHT6_3))).rgb*((WEIGHT6_2+WEIGHT6_3)/(WEIGHT6_0+WEIGHT6_1)));
   color=Max(color, TexLod(Img, uv+RTSize.xy*Vec2(0,  4+WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5))).rgb*((WEIGHT6_4+WEIGHT6_5)/(WEIGHT6_0+WEIGHT6_1)));
   color=Max(color, TexLod(Img, uv+RTSize.xy*Vec2(0, -4-WEIGHT6_5/(WEIGHT6_4  +WEIGHT6_5))).rgb*((WEIGHT6_4+WEIGHT6_5)/(WEIGHT6_0+WEIGHT6_1)));
   return VecH4(color, 0);
}
/******************************************************************************/
