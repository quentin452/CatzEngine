/******************************************************************************
   
   'CubicFastSharp' is equal to 'Lerp4Weights' in following way:

      const int n=100; REP(n)
      {
         Flt  step=i/Flt(n-1);
         Vec4 w; Lerp4Weights(w, step);
         Flt  c=CubicFastSharp(step*2);
         Vec2(step*0.5f+0.5f, w.x).draw(RED);
         Vec2(step*0.5f     , w.y).draw(GREEN);
         Vec2(step          , c  ).draw(BLUE);
      }

   Upscaling should be done in sRGB Gamma space to allow for smooth gradients - looks much better !!

/******************************************************************************/
#include "stdafx.h"
namespace EE{
#include "Import/BC.h"
#include "Import/ETC.h"

#define SUPPORT_DEPTH_TO_COLOR 0

 // alpha limit at which colors start to blend to RGB
#define ALPHA_LIMIT_LINEAR            (1.0f/16)
#define ALPHA_LIMIT_CUBIC_FAST        (1.0f/ 8)
#define ALPHA_LIMIT_CUBIC_FAST_SMOOTH (1.0f/16)
#define ALPHA_LIMIT_CUBIC_FAST_SHARP  (1.0f/ 4) // Warning: if increasing this value then it might cause overflow for integer processing (for 'CWA8AlphaLimit')
#define ALPHA_LIMIT_CUBIC_PLUS        (1.0f/ 6)
#define ALPHA_LIMIT_CUBIC_PLUS_SHARP  (1.0f/ 4)
#define ALPHA_LIMIT_NONE              FLT_MIN // use FLT_MIN so we still process 0<alpha_limit

/*
   Flt CW [8][8];
   Int CWI[8][8];
   Int smallest=-1; Flt error=0;
   for(Int i=1; i<=INT_MAX/255/8/8; i++)
   {
      Flt  e=0;
      Long sum=0;
      REPD(y, 8)
      {
         Flt fy2=Sqr(-1.75f+0.5f*y);
         REPD(x, 8)
         {
            Flt fx2=Sqr(-1.75f+0.5f*x), w=CubicFastSharp(Sqrt(fx2+fy2));
            if(Min(x, 7-x)+Min(y, 7-y)<2)w=0; // we will disable corner samples
         #if 0
            Flt mul=i;
         #else
            Flt mul=i/0.00417041779f; // 0.00417041779f is 'w' weight for smallest sample (y=0, x=2)
         #endif
            CW [y][x]=w;
            CWI[y][x]=Round(CW[y][x]*mul);
            e+=Abs(CWI[y][x]/mul-CW[y][x]);
            sum+=255*CWI[y][x];
         }
      }
      if(sum + sum/2>INT_MAX)break; // we will add sum/2 for rounding
      if(smallest<0 || e<error){error=e; smallest=i;}
      if(smallest==6806) // this is the value that will have smallest error for "mul=i/0.00417041779f"
      {
         Str s;
         s.line()+=S+"static const Int CW8Sum="+sum+"/255;";
         s.line()+=S+"static const Int CW8[8][8]={";
         FREPD(y, 8)
         {
            s.line()+="   {";
            FREPD(x, 8)
            {
               if(x)s+=", ";
               s+=CWI[y][x];
            }
            s+="},";
         }
         s.line()+="};";
         ClipSet(s.line());
      }
   }

   Flt CWA[8][8];
   Int sum=0;
   REPD(y, 8)
   {
      Flt fy2=Sqr(-1.75f+0.5f*y);
      REPD(x, 8)
      {
         Flt fx2=Sqr(-1.75f+0.5f*x), w=CubicFastSharp(Sqrt(fx2+fy2));
         CWA[y][x]=w*0xFF08/2.9391276836395264;
         sum+=Round(CWA[y][x]);
      }
   }
   Int s=sum;


   Dbl CFSMW8[8][8], W=0;
   REPD(y, 8)
   {
      Dbl fy2=Sqr(-1.75+0.5*y);
      REPD(x, 8)
      {
         Dbl fx2=Sqr(-1.75+0.5*x), w=fx2+fy2; w=((w<Sqr(CUBIC_FAST_RANGE)) ? CubicFastSmooth(Sqrt(w)) : 0);
         W+=w;
         CFSMW8[y][x]=w;
      }
   }
   REPD(y, 8)
   REPD(x, 8)CFSMW8[y][x]/=W;

   Str s;
   s.line()+=S+"static const Flt CFSMW8[8][8]= // [y][x]";
   s.line()+='{';
   FREPD(y, 8)
   {
      s.line()+="   {";
      FREPD(x, 8)
      {
         if(x)s+=", ";
         s+=CFSMW8[y][x]; s+='f';
      }
      s+="},";
   }
   s.line()+="};";
   ClipSet(s.line());
*/
static const Int CW8Sum=1431592440/255;
static const Int CW8[8][8]={
   {0, 0, -7966, -39547, -39547, -7966, 0, 0},
   {0, -39547, -128548, -138021, -138021, -128548, -39547, 0},
   {-7966, -128548, -51119, 341260, 341260, -51119, -128548, -7966},
   {-39547, -138021, 341260, 1439832, 1439832, 341260, -138021, -39547},
   {-39547, -138021, 341260, 1439832, 1439832, 341260, -138021, -39547},
   {-7966, -128548, -51119, 341260, 341260, -51119, -128548, -7966},
   {0, -39547, -128548, -138021, -138021, -128548, -39547, 0},
   {0, 0, -7966, -39547, -39547, -7966, 0, 0},
};
static const Int CWA8Sum=1420666200/255/255;
static const Int CWA8AlphaLimit=CWA8Sum*255*ALPHA_LIMIT_CUBIC_FAST_SHARP;
static const Int CWA8[8][8]={
   {0, 0, -31, -154, -154, -31, 0, 0},
   {0, -154, -500, -537, -537, -500, -154, 0},
   {-31, -500, -199, 1328, 1328, -199, -500, -31},
   {-154, -537, 1328, 5603, 5603, 1328, -537, -154},
   {-154, -537, 1328, 5603, 5603, 1328, -537, -154},
   {-31, -500, -199, 1328, 1328, -199, -500, -31},
   {0, -154, -500, -537, -537, -500, -154, 0},
   {0, 0, -31, -154, -154, -31, 0, 0},
};

static const Flt CFSMW8[8][8]= // [y][x]
{
   {0.000000000f, 0.000000000f, 0.000025184f, 0.000355931f, 0.000355931f, 0.000025184f, 0.000000000f, 0.000000000f},
   {0.000000000f, 0.000355931f, 0.004531289f, 0.010840487f, 0.010840487f, 0.004531289f, 0.000355931f, 0.000000000f},
   {0.000025184f, 0.004531289f, 0.023553867f, 0.049229048f, 0.049229048f, 0.023553867f, 0.004531289f, 0.000025184f},
   {0.000355931f, 0.010840487f, 0.049229048f, 0.096126321f, 0.096126321f, 0.049229048f, 0.010840487f, 0.000355931f},
   {0.000355931f, 0.010840487f, 0.049229048f, 0.096126321f, 0.096126321f, 0.049229048f, 0.010840487f, 0.000355931f},
   {0.000025184f, 0.004531289f, 0.023553867f, 0.049229048f, 0.049229048f, 0.023553867f, 0.004531289f, 0.000025184f},
   {0.000000000f, 0.000355931f, 0.004531289f, 0.010840487f, 0.010840487f, 0.004531289f, 0.000355931f, 0.000000000f},
   {0.000000000f, 0.000000000f, 0.000025184f, 0.000355931f, 0.000355931f, 0.000025184f, 0.000000000f, 0.000000000f},
};
/******************************************************************************/
// FILTERING
/******************************************************************************/
Bool (*ResizeWaifu)(C Image &src, Image &dest, UInt flags);
/******************************************************************************/
static Flt Linear(Flt x) {ABS(x); if(x>=1)return 0; return 1-x;}
/******************************************************************************/
#define CEIL(x) int(x+0.99f)
ASSERT(CEIL(0.0f)==0);
ASSERT(CEIL(0.4f)==1);
ASSERT(CEIL(0.5f)==1);
ASSERT(CEIL(0.6f)==1);
ASSERT(CEIL(0.9f)==1);
ASSERT(CEIL(1.0f)==1);
ASSERT(CEIL(1.1f)==2);

#define CUBIC_FAST_RANGE            2
#define CUBIC_FAST_SAMPLES         CEIL(CUBIC_FAST_RANGE)
#define CUBIC_PLUS_SHARPNESS       (2/2.5f) // (2/2.65f) is smooth and correctly works with gradients, but (2/2.5f) is sharper and looks like JincJinc for regular images
#define CUBIC_PLUS_SHARP_SHARPNESS (2/2.5f) // (2/2.65f) is smooth and correctly works with gradients, but (2/2.5f) is sharper and looks like JincJinc for regular images
#define CUBIC_PLUS_RANGE           (2/CUBIC_PLUS_SHARPNESS      )
#define CUBIC_PLUS_SHARP_RANGE     (2/CUBIC_PLUS_SHARP_SHARPNESS)
#define CUBIC_PLUS_SAMPLES         CEIL(CUBIC_PLUS_RANGE        )
#define CUBIC_PLUS_SHARP_SAMPLES   CEIL(CUBIC_PLUS_SHARP_RANGE  )

static INLINE Flt Cubic(Flt x, Flt blur, Flt sharpen) // x=0..2, https://en.wikipedia.org/wiki/Mitchell%E2%80%93Netravali_filters
{
   Flt x2=x*x,
       x3=x*x*x;
   return (x<=1) ? ((12-9*blur-6*sharpen)/6*x3 + (-18+12*blur+6*sharpen)/6*x2 +                             (6-2*blur         )/6)
                 : ((-blur-6*sharpen    )/6*x3 + (6*blur+30*sharpen    )/6*x2 + (-12*blur-48*sharpen)/6*x + (8*blur+24*sharpen)/6);
}
static INLINE Flt CatmullRom       (Flt x) {return Cubic(x, 0.0f  , 0.5f  );}
static INLINE Flt MitchellNetravali(Flt x) {return Cubic(x, 1.0f/3, 1.0f/3);}
static INLINE Flt Robidoux         (Flt x) {return Cubic(x, 12/(19+9*SQRT2), 113/(58+216*SQRT2));}
static INLINE Flt RobidouxSharp    (Flt x) {return Cubic(x,  6/(13+7*SQRT2),   7/( 2+ 12*SQRT2));}

static INLINE Flt CubicFast      (Flt x) {return Cubic(x, 1.0f/3, 1.0f/3);}
static INLINE Flt CubicFastSmooth(Flt x) {return Cubic(x, 1.0f  , 0.000f);}
static INLINE Flt CubicFastSharp (Flt x) {return Cubic(x, 0.0f  , 0.500f);}
static INLINE Flt CubicPlus      (Flt x) {return Cubic(x, 0.0f  , 0.400f);}
static INLINE Flt CubicPlusSharp (Flt x) {return Cubic(x, 0.0f  , 0.500f);}

static Flt CubicFast2      (Flt xx) {return CubicFast      (SqrtFast(xx));}
static Flt CubicFastSmooth2(Flt xx) {return CubicFastSmooth(SqrtFast(xx));}
static Flt CubicFastSharp2 (Flt xx) {return CubicFastSharp (SqrtFast(xx));}
static Flt CubicPlus2      (Flt xx) {return CubicPlus      (SqrtFast(xx));}
static Flt CubicPlusSharp2 (Flt xx) {return CubicPlusSharp (SqrtFast(xx));}
/******************************************************************************/
#define SINC_RANGE      2
#define JINC_HALF_RANGE 1.2196698912665045f
#define JINC_RANGE      2.233130455f
#define JINC_SMOOTH     0.42f // 0.0 (sharp) .. 0.5 (smooth)
#define JINC_SAMPLES    CEIL(JINC_RANGE)

//static inline Flt sinc(Flt x) {return x ? sin(x)/x : 1.0f;}
//static inline Flt jinc(Flt x) {return x ?  j1(x)/x : 0.5f;}

static INLINE Flt SincSinc(Flt x)
{
 //return Sinc(x)*Sinc(x*JINC_SMOOTH);
   x*=PI; Flt xx=Sqr(x); return xx ? Sin(x)*Sin(x*JINC_SMOOTH)/xx : JINC_SMOOTH;
}
#define J1 PLATFORM(_j1, j1)
static INLINE Flt JincJinc(Flt x)
{
 //return Jinc(x)*Jinc(x*JINC_SMOOTH);
   x*=PI; Flt xx=Sqr(x); return xx ? J1(x)*J1(x*JINC_SMOOTH)/xx : 0.25f*JINC_SMOOTH;
}
static Flt SincSinc2(Flt xx) {return SincSinc(SqrtFast(xx));}
static Flt JincJinc2(Flt xx) {return JincJinc(SqrtFast(xx));}
/******************************************************************************/
static inline Flt AlphaLimitBlend(Flt blend) {return Sqr(blend);} // using 'Sqr' reduces artifacts
static void Add(Vec4 &color, Vec &rgb, C Vec4 &sample, Bool alpha_weight) // here 'weight'=1
{
   if(alpha_weight)
   {
         rgb      +=sample.xyz         ; // RGB
         color.xyz+=sample.xyz*sample.w; // RGB*Alpha
         color.w  +=           sample.w; //     Alpha
   }else color    +=sample             ;
}
static void Add(Vec4 &color, Vec &rgb, C Vec4 &sample, Flt weight, Bool alpha_weight)
{
   if(alpha_weight)
   {
         rgb      +=sample.xyz*weight; // RGB*      weight
         weight   *=sample.w         ; // adjust 'weight' by Alpha
         color.xyz+=sample.xyz*weight; // RGB*Alpha*weight
         color.w  +=           weight; //     Alpha*weight
   }else color    +=sample    *weight;
}
static void Normalize(Vec4 &color, C Vec &rgb, Bool alpha_weight, Flt alpha_limit) // here 'weight'=1
{
   if(alpha_weight)
   {
      // normalize
      if(color.w>0)color.xyz/=color.w;

      // for low opacities we need to preserve the original RGB, because, at opacity=0 RGB information is totally lost, and because the image may be used for drawing, in which case the GPU will use bilinear filtering, and we need to make sure that mostly transparent pixel RGB values aren't artificially upscaled too much, also because at low opacities the alpha precision is low (because normally images are in 8-bit format, and we get alpha values like 1,2,3,..) and because we need to divide by alpha, inaccuracies may occur
      if(color.w<alpha_limit)
      {
         if(color.w<=0)
         {
            color.w=0; // 'color.w' can be negative due to sharpening
            color.xyz=rgb;
         }else
         {
            Flt blend=AlphaLimitBlend(color.w/alpha_limit);
            // color.xyz = Lerp(rgb, color.xyz, blend);
            color.xyz*=blend;
            color.xyz+=rgb*(1-blend);
         }
      }
   }
}
static void Normalize(Vec4 &color, C Vec &rgb, Flt weight, Bool alpha_weight, Flt alpha_limit) // Warning: 'weight' must be non-zero
{
   if(alpha_weight)
   {
      // normalize
      if(color.w>0)
      {
         color.xyz/=color.w;
         color.w  /=weight;
      }

      // for low opacities we need to preserve the original RGB, because, at opacity=0 RGB information is totally lost, and because the image may be used for drawing, in which case the GPU will use bilinear filtering, and we need to make sure that mostly transparent pixel RGB values aren't artificially upscaled too much, also because at low opacities the alpha precision is low (because normally images are in 8-bit format, and we get alpha values like 1,2,3,..) and because we need to divide by alpha, inaccuracies may occur
      if(color.w<alpha_limit)
      {
         if(color.w<=0)
         {
            color.w=0; // 'color.w' can be negative due to sharpening
            color.xyz=rgb/weight;
         }else
         {
            Flt blend=AlphaLimitBlend(color.w/alpha_limit);
            // color.xyz = Lerp(rgb, color.xyz, blend);
            color.xyz*=blend;
            color.xyz+=rgb*((1-blend)/weight);
         }
      }
   }else color/=weight;
}
/******************************************************************************/
PtrImagePixel GetImagePixelF(FILTER_TYPE filter)
{
   switch(filter)
   {
      case FILTER_NONE             : return &Image::pixelFNearest        ;
      case FILTER_LINEAR           : return &Image::pixelFLinear         ;
      case FILTER_CUBIC_FAST       : return &Image::pixelFCubicFast      ;
      case FILTER_CUBIC_FAST_SMOOTH: return &Image::pixelFCubicFastSmooth;
      case FILTER_CUBIC_FAST_SHARP : return &Image::pixelFCubicFastSharp ;
      default                      : // FILTER_BEST, FILTER_WAIFU
      case FILTER_CUBIC_PLUS       : return &Image::pixelFCubicPlus      ;
      case FILTER_CUBIC_PLUS_SHARP : return &Image::pixelFCubicPlusSharp ;
   }
}
PtrImagePixel3D GetImagePixel3DF(FILTER_TYPE filter)
{
   switch(filter)
   {
      case FILTER_NONE             : return &Image::pixel3DFNearest        ;
      case FILTER_LINEAR           : return &Image::pixel3DFLinear         ;
      case FILTER_CUBIC_FAST       : return &Image::pixel3DFCubicFast      ;
      case FILTER_CUBIC_FAST_SMOOTH: return &Image::pixel3DFCubicFastSmooth;
      case FILTER_CUBIC_FAST_SHARP : return &Image::pixel3DFCubicFastSharp ;
      default                      : // FILTER_BEST, FILTER_WAIFU
      case FILTER_CUBIC_PLUS       : return &Image::pixel3DFCubicPlus      ;
      case FILTER_CUBIC_PLUS_SHARP : return &Image::pixel3DFCubicPlusSharp ;
   }
}
PtrImageColor GetImageColorF(FILTER_TYPE filter)
{
   switch(filter)
   {
      case FILTER_NONE             : return &Image::colorFNearest        ;
      case FILTER_LINEAR           : return &Image::colorFLinear         ;
      case FILTER_CUBIC_FAST       : return &Image::colorFCubicFast      ;
      case FILTER_CUBIC_FAST_SMOOTH: return &Image::colorFCubicFastSmooth;
      case FILTER_CUBIC_FAST_SHARP : return &Image::colorFCubicFastSharp ;
      default                      : // FILTER_BEST, FILTER_WAIFU
      case FILTER_CUBIC_PLUS       : return &Image::colorFCubicPlus      ;
      case FILTER_CUBIC_PLUS_SHARP : return &Image::colorFCubicPlusSharp ;
   }
}
PtrImageColor3D GetImageColor3DF(FILTER_TYPE filter)
{
   switch(filter)
   {
      case FILTER_NONE             : return &Image::color3DFNearest        ;
      case FILTER_LINEAR           : return &Image::color3DFLinear         ;
      case FILTER_CUBIC_FAST       : return &Image::color3DFCubicFast      ;
      case FILTER_CUBIC_FAST_SMOOTH: return &Image::color3DFCubicFastSmooth;
      case FILTER_CUBIC_FAST_SHARP : return &Image::color3DFCubicFastSharp ;
      default                      : // FILTER_BEST, FILTER_WAIFU
      case FILTER_CUBIC_PLUS       : return &Image::color3DFCubicPlus      ;
      case FILTER_CUBIC_PLUS_SHARP : return &Image::color3DFCubicPlusSharp ;
   }
}
PtrImageAreaColor GetImageAreaColor(FILTER_TYPE filter, Bool &linear_gamma)
{
   switch(filter)
   {
    //case FILTER_AVERAGE          : linear_gamma=false; return &Image::areaColorFAverage        ;
      case FILTER_LINEAR           : linear_gamma=true ; return &Image::areaColorLLinear         ;
      case FILTER_CUBIC_FAST       : linear_gamma=true ; return &Image::areaColorLCubicFast      ;
      case FILTER_CUBIC_FAST_SMOOTH: linear_gamma=true ; return &Image::areaColorLCubicFastSmooth;
      default                      : ASSERT(FILTER_DOWN==FILTER_CUBIC_FAST_SHARP); // FILTER_BEST, FILTER_WAIFU
      case FILTER_CUBIC_FAST_SHARP : linear_gamma=false; return &Image::areaColorFCubicFastSharp ; // FILTER_CUBIC_FAST_SHARP is not suitable for linear gamma
      case FILTER_CUBIC_PLUS       : linear_gamma=false; return &Image::areaColorFCubicPlus      ; // FILTER_CUBIC_PLUS       is not suitable for linear gamma
      case FILTER_CUBIC_PLUS_SHARP : linear_gamma=false; return &Image::areaColorFCubicPlusSharp ; // FILTER_CUBIC_PLUS_SHARP is not suitable for linear gamma
   }
}
/******************************************************************************/
// PIXEL / COLOR
/******************************************************************************/
static inline void SetPixel(Byte *data, Int byte_pp, UInt pixel)
{
   switch(byte_pp)
   {
      case 1: (*(U8 *)data)=pixel&0xFF; break;
      case 2: (*(U16*)data)=pixel&0xFFFF; break;
      case 3: (*(U16*)data)=pixel&0xFFFF; data[2]=(pixel>>16)&0xFF; break;
      case 4: (*(U32*)data)=pixel; break;
   }
}
void Image::pixel(Int x, Int y, UInt pixel)
{
   if(InRange(x, lw()) && InRange(y, lh()) && !compressed()) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetPixel(data() + x*bytePP() + y*pitch(), bytePP(), pixel);
}
void Image::pixel3D(Int x, Int y, Int z, UInt pixel)
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld()) && !compressed()) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetPixel(data() + x*bytePP() + y*pitch() + z*pitch2(), bytePP(), pixel);
}
/******************************************************************************/
static inline UInt GetPixel(C Byte *data, Int byte_pp)
{
   switch(byte_pp)
   {
      case 1: return *(U8 *)data;
      case 2: return *(U16*)data;
      case 3: return *(U16*)data | (data[2]<<16);
      case 4: return *(U32*)data;
   }
   return 0;
}
UInt Image::pixel(Int x, Int y)C
{
   if(InRange(x, lw()) && InRange(y, lh()) && !compressed()) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      return GetPixel(data() + x*bytePP() + y*pitch(), bytePP());
   return 0;
}
UInt Image::pixel3D(Int x, Int y, Int z)C
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld()) && !compressed()) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      return GetPixel(data() + x*bytePP() + y*pitch() + z*pitch2(), bytePP());
   return 0;
}
/******************************************************************************/
// 11 BIT FLOAT - EEEEE FFFFFF (E-5 bit Exponent, F-6 bit Fraction), there's no sign, uses 0x7FF0=0xFFFF-0x8000-1-2-4-8    mask from 16-bit float (16bits -sign -4smallest bits)
// 10 BIT FLOAT - EEEEE FFFFF  (E-5 bit Exponent, F-5 bit Fraction), there's no sign, uses 0x7FE0=0xFFFF-0x8000-1-2-4-8-16 mask from 16-bit float (16bits -sign -5smallest bits)
static Vec GetR11G11B10F(CPtr data)
{
   VecH h;
   UInt u=*(UInt*)data;
   h.x.data=(u& 2047     )<<    4 ;
   h.y.data=(u&(2047<<11))>>(11-4);
   h.z.data=(u&(1023<<22))>>(22-5);
   return h;
}
static void SetR11G11B10F(Ptr data, C Vec &rgb)
{
   VecH h=rgb; // if negative   ? 0
  *(UInt*)data=(h.x.data&0x8000 ? 0 : ((h.x.data&0x7FF0)>>    4 ))
              |(h.y.data&0x8000 ? 0 : ((h.y.data&0x7FF0)<<(11-4)))
              |(h.z.data&0x8000 ? 0 : ((h.z.data&0x7FE0)<<(22-5)));
}
/******************************************************************************/
static void SetPixelF(Byte *data, IMAGE_TYPE type, Flt pixel)
{
   switch(type)
   {
      case IMAGE_F32  :                        (*(Flt *)data)=pixel; break;
      case IMAGE_F32_2:                        (*(Vec2*)data)=pixel; break;
      case IMAGE_F32_3: case IMAGE_F32_3_SRGB: (*(Vec *)data)=pixel; break;
      case IMAGE_F32_4: case IMAGE_F32_4_SRGB: (*(Vec4*)data)=pixel; break;

      case IMAGE_F16  : {U16 *d=(U16*)data; d[0]=               Half(pixel).data;} break;
      case IMAGE_F16_2: {U16 *d=(U16*)data; d[0]=d[1]=          Half(pixel).data;} break;
      case IMAGE_F16_3: {U16 *d=(U16*)data; d[0]=d[1]=d[2]=     Half(pixel).data;} break;
      case IMAGE_F16_4: {U16 *d=(U16*)data; d[0]=d[1]=d[2]=d[3]=Half(pixel).data;} break;

      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB:
      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB:
                                {VecB4 &v=*(VecB4*)data; v.x=v.y=v.z=FltToByte(pixel); v.w=255;} break;

      case IMAGE_B8G8R8: case IMAGE_B8G8R8_SRGB:
      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB:
                                {VecB  &v=*(VecB *)data; v.x=v.y=v.z=FltToByte(pixel);} break;

      case IMAGE_R8G8:          {VecB2 &v=*(VecB2*)data; v.x=v.y=FltToByte(pixel);} break;

      case IMAGE_R8_SIGN      : {SByte  &v=*(SByte *)data; v  =        SFltToSByte(pixel);         } break;
      case IMAGE_R8G8_SIGN    : {VecSB2 &v=*(VecSB2*)data; v.x=v.y=    SFltToSByte(pixel);         } break;
      case IMAGE_R8G8B8A8_SIGN: {VecSB4 &v=*(VecSB4*)data; v.x=v.y=v.z=SFltToSByte(pixel); v.w=127;} break;

      case IMAGE_R10G10B10A2: {UInt v=FltToU10(pixel); (*(UInt*)data)=v|(v<<10)|(v<<20)|(3<<30);} break;

      case IMAGE_R8 :
      case IMAGE_A8 :
      case IMAGE_L8 : case IMAGE_L8_SRGB:
      case IMAGE_I8 : (*(U8 *)data)=FltToByte  (    pixel             ); break; // it's okay   to clamp int for small  values
      case IMAGE_I16: (*(U16*)data)=RoundU     (Sat(pixel)*0x0000FFFFu); break; // it's better to clamp flt for bigger values
      case IMAGE_I32: (*(U32*)data)=RoundUClamp(Sat(pixel)*0xFFFFFFFFu); break; // it's better to clamp flt for bigger values
      case IMAGE_I24: {  U32  c    =RoundU     (Sat(pixel)*0x00FFFFFFu); (*(U16*)data)=c; data[2]=(c>>16);} break; // it's better to clamp flt for bigger values
 
      case IMAGE_R11G11B10F: SetR11G11B10F(data, pixel); break;
   }
}
void Image::pixelF(Int x, Int y, Flt pixel)
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetPixelF(data() + x*bytePP() + y*pitch(), hwType(), pixel);
}
void Image::pixel3DF(Int x, Int y, Int z, Flt pixel)
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetPixelF(data() + x*bytePP() + y*pitch() + z*pitch2(), hwType(), pixel);
}
/******************************************************************************/
static Color DecompressPixel(C Image &image, Int x, Int y)
{
   Int   x3=x&3, y3=y&3;
 C Byte *data=image.data() + (y>>2)*image.pitch();
   switch(image.hwType())
   {
      // IMAGE_BC4_SIGN, IMAGE_BC5_SIGN, IMAGE_BC6, IMAGE_ETC2_R_SIGN, IMAGE_ETC2_RG_SIGN, handled elsewhere
      case IMAGE_BC1       : case IMAGE_BC1_SRGB       : return DecompressPixelBC1      (data + (x>>2)* 8, x3, y3);
      case IMAGE_BC2       : case IMAGE_BC2_SRGB       : return DecompressPixelBC2      (data + (x>>2)*16, x3, y3);
      case IMAGE_BC3       : case IMAGE_BC3_SRGB       : return DecompressPixelBC3      (data + (x>>2)*16, x3, y3);
      case IMAGE_BC4       :                             return DecompressPixelBC4      (data + (x>>2)* 8, x3, y3);
      case IMAGE_BC5       :                             return DecompressPixelBC5      (data + (x>>2)*16, x3, y3);
      case IMAGE_BC7       : case IMAGE_BC7_SRGB       : return DecompressPixelBC7      (data + (x>>2)*16, x3, y3);
      case IMAGE_ETC1      :                             return DecompressPixelETC1     (data + (x>>2)* 8, x3, y3);
      case IMAGE_ETC2_R    :                             return DecompressPixelETC2R    (data + (x>>2)* 8, x3, y3);
      case IMAGE_ETC2_RG   :                             return DecompressPixelETC2RG   (data + (x>>2)*16, x3, y3);
      case IMAGE_ETC2_RGB  : case IMAGE_ETC2_RGB_SRGB  : return DecompressPixelETC2RGB  (data + (x>>2)* 8, x3, y3);
      case IMAGE_ETC2_RGBA1: case IMAGE_ETC2_RGBA1_SRGB: return DecompressPixelETC2RGBA1(data + (x>>2)* 8, x3, y3);
      case IMAGE_ETC2_RGBA : case IMAGE_ETC2_RGBA_SRGB : return DecompressPixelETC2RGBA (data + (x>>2)*16, x3, y3);
   }
   return TRANSPARENT;
}
static Color DecompressPixel(C Image &image, Int x, Int y, Int z)
{
   Int   x3=x&3, y3=y&3;
 C Byte *data=image.data() + (y>>2)*image.pitch() + z*image.pitch2();
   switch(image.hwType())
   {
      // IMAGE_BC4_SIGN, IMAGE_BC5_SIGN, IMAGE_BC6, IMAGE_ETC2_R_SIGN, IMAGE_ETC2_RG_SIGN, handled elsewhere
      case IMAGE_BC1       : case IMAGE_BC1_SRGB       : return DecompressPixelBC1      (data + (x>>2)* 8, x3, y3);
      case IMAGE_BC2       : case IMAGE_BC2_SRGB       : return DecompressPixelBC2      (data + (x>>2)*16, x3, y3);
      case IMAGE_BC3       : case IMAGE_BC3_SRGB       : return DecompressPixelBC3      (data + (x>>2)*16, x3, y3);
      case IMAGE_BC4       :                             return DecompressPixelBC4      (data + (x>>2)* 8, x3, y3);
      case IMAGE_BC5       :                             return DecompressPixelBC5      (data + (x>>2)*16, x3, y3);
      case IMAGE_BC7       : case IMAGE_BC7_SRGB       : return DecompressPixelBC7      (data + (x>>2)*16, x3, y3);
      case IMAGE_ETC1      :                             return DecompressPixelETC1     (data + (x>>2)* 8, x3, y3);
      case IMAGE_ETC2_R    :                             return DecompressPixelETC2R    (data + (x>>2)* 8, x3, y3);
      case IMAGE_ETC2_RG   :                             return DecompressPixelETC2RG   (data + (x>>2)*16, x3, y3);
      case IMAGE_ETC2_RGB  : case IMAGE_ETC2_RGB_SRGB  : return DecompressPixelETC2RGB  (data + (x>>2)* 8, x3, y3);
      case IMAGE_ETC2_RGBA1: case IMAGE_ETC2_RGBA1_SRGB: return DecompressPixelETC2RGBA1(data + (x>>2)* 8, x3, y3);
      case IMAGE_ETC2_RGBA : case IMAGE_ETC2_RGBA_SRGB : return DecompressPixelETC2RGBA (data + (x>>2)*16, x3, y3);
   }
   return TRANSPARENT;
}
/******************************************************************************/
static inline Flt GetPixelF(C Byte *data, C Image &image, Bool _2d, Int x, Int y, Int z=0)
{
   switch(image.hwType())
   {
      case IMAGE_F32  :                        return *(Flt*)data;
      case IMAGE_F32_2:                        return *(Flt*)data;
      case IMAGE_F32_3: case IMAGE_F32_3_SRGB: return *(Flt*)data;
      case IMAGE_F32_4: case IMAGE_F32_4_SRGB: return *(Flt*)data;

      case IMAGE_F16  : return *(Half*)data;
      case IMAGE_F16_2: return *(Half*)data;
      case IMAGE_F16_3: return *(Half*)data;
      case IMAGE_F16_4: return *(Half*)data;

      case IMAGE_B8G8R8  : case IMAGE_B8G8R8_SRGB  :
      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB:
         return ((VecB4*)data)->z/Flt(0xFF);

      case IMAGE_R8      :
      case IMAGE_R8G8    :
      case IMAGE_R8G8B8  : case IMAGE_R8G8B8_SRGB:
      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB:
      case IMAGE_A8      :
      case IMAGE_L8      : case IMAGE_L8_SRGB:
      case IMAGE_L8A8    : case IMAGE_L8A8_SRGB:
      case IMAGE_I8      :
         return (*(U8*)data)/Flt(0x000000FFu);

      // 16
      case IMAGE_D16: if(GL)return (*(U16*)data)/Flt(0x0000FFFFu)*2-1; // !! else fall through no break on purpose !!
      case IMAGE_I16:
         return (*(U16*)data)/Flt(0x0000FFFFu);

      // 32
      case IMAGE_D32     :
      case IMAGE_D32S8X24:       return GL ? (*(Flt*)data)*2-1 : *(Flt*)data;
    //case IMAGE_D32I    : if(GL)return (*(U32*)data)/Dbl(0xFFFFFFFFu)*2-1; // !! else fall through no break on purpose !!
      case IMAGE_I32     :
         return (*(U32*)data)/Dbl(0xFFFFFFFFu); // Dbl required to get best precision

      // 24
      case IMAGE_D24S8:
      case IMAGE_D24X8: if(GL)return (*(U16*)(data+1) | (data[3]<<16))/Flt(0x00FFFFFFu)*2-1; // !! else fall through no break on purpose !!
      case IMAGE_I24  :
         return (*(U16*)data | (data[2]<<16))/Flt(0x00FFFFFFu); // here Dbl is not required, this was tested

      case IMAGE_R10G10B10A2: return U10ToFlt((*(UInt*)data)&0x3FF);

      case IMAGE_R11G11B10F: return GetR11G11B10F(data).x;

      case IMAGE_R8_SIGN      :
      case IMAGE_R8G8_SIGN    :
      case IMAGE_R8G8B8A8_SIGN:
         return SByteToSFlt(*(SByte*)data);

      case IMAGE_B4G4R4A4: return (((*(U16*)data)>> 8)&0x0F)/15.0f;
      case IMAGE_B5G5R5A1: return (((*(U16*)data)>>10)&0x1F)/31.0f;
      case IMAGE_B5G6R5  : return (((*(U16*)data)>>11)&0x1F)/31.0f;

      case IMAGE_BC6: return DecompressPixelBC6(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3).x;

      case IMAGE_BC4_SIGN: return SByteToSFlt(DecompressPixelBC4S(image.data() + (x>>2)* 8 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3));
      case IMAGE_BC5_SIGN: return SByteToSFlt(DecompressPixelBC4S(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3)); // can use 'DecompressPixelBC4S' because BC5 is made of 2xBC4

      case IMAGE_ETC2_R_SIGN : return SByteToSFlt(DecompressPixelETC2RS(image.data() + (x>>2)* 8 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3));
      case IMAGE_ETC2_RG_SIGN: return SByteToSFlt(DecompressPixelETC2RS(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3)); // can use 'DecompressPixelETC2RS' because ETC2_RG is made of 2xETC2_R

      case IMAGE_BC1       : case IMAGE_BC1_SRGB       :
      case IMAGE_BC2       : case IMAGE_BC2_SRGB       :
      case IMAGE_BC3       : case IMAGE_BC3_SRGB       :
      case IMAGE_BC4       :
      case IMAGE_BC5       :
      case IMAGE_BC7       : case IMAGE_BC7_SRGB       :
      case IMAGE_ETC1      :
      case IMAGE_ETC2_R    :
      case IMAGE_ETC2_RG   :
      case IMAGE_ETC2_RGB  : case IMAGE_ETC2_RGB_SRGB  :
      case IMAGE_ETC2_RGBA1: case IMAGE_ETC2_RGBA1_SRGB:
      case IMAGE_ETC2_RGBA : case IMAGE_ETC2_RGBA_SRGB :
         return ByteToFlt((_2d ? DecompressPixel(image, x, y) : DecompressPixel(image, x, y, z)).r);
   }
   return 0;
}
Flt Image::pixelF(Int x, Int y)C
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      return GetPixelF(data() + x*bytePP() + y*pitch(), T, true, x, y);
   return 0;
}
Flt Image::pixel3DF(Int x, Int y, Int z)C
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      return GetPixelF(data() + x*bytePP() + y*pitch() + z*pitch2(), T, false, x, y, z);
   return 0;
}
/******************************************************************************/
static inline Color GetColor(C Byte *data, C Image &image, Bool _2d, Int x, Int y, Int z=0)
{
   switch(image.hwType())
   {
      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: {C Color &c=*(Color*)data; return Color(  c.b, c.g, c.r,   c.a);}
      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB:                            return*(Color*)data;
      case IMAGE_R8G8B8  : case IMAGE_R8G8B8_SRGB  : {C VecB  &v=*(VecB *)data; return Color(  v.x, v.y, v.z,   255);}
      case IMAGE_R8G8    :                           {C VecB2 &v=*(VecB2*)data; return Color(  v.x, v.y,   0,   255);}
      case IMAGE_R8      :                                                      return Color(*data,   0,   0,   255);
      case IMAGE_A8      :                                                      return Color(    0,   0,   0, *data);
      case IMAGE_L8      : case IMAGE_L8_SRGB  :     {  Byte   b=*data;         return Color(    b,   b,   b,   255);}
      case IMAGE_L8A8    : case IMAGE_L8A8_SRGB:     {C VecB2 &v=*(VecB2*)data; return Color(  v.x, v.x, v.x,   v.y);}
      case IMAGE_I8      :                           {  Byte   b=data[0];       return Color(    b,   b,   b,   255);}
      case IMAGE_I16     :                           {  Byte   b=data[1];       return Color(    b,   b,   b,   255);}
      case IMAGE_I24     :                           {  Byte   b=data[2];       return Color(    b,   b,   b,   255);}
      case IMAGE_I32     :                           {  Byte   b=data[3];       return Color(    b,   b,   b,   255);}
      case IMAGE_B8G8R8  : case IMAGE_B8G8R8_SRGB:   {C VecB  &v=*(VecB *)data; return Color(  v.z, v.y, v.x,   255);}

      case IMAGE_B4G4R4A4   : {U16  d=*(U16 *)data; return Color(U4ToByte((d>> 8)&0x0F), U4ToByte((d>> 4)&0x0F), U4ToByte((d    )&0x0F), U4ToByte(d>>12));}
      case IMAGE_B5G5R5A1   : {U16  d=*(U16 *)data; return Color(U5ToByte((d>>10)&0x1F), U5ToByte((d>> 5)&0x1F), U5ToByte((d    )&0x1F), U1ToByte(d>>15));}
      case IMAGE_B5G6R5     : {U16  d=*(U16 *)data; return Color(U5ToByte((d>>11)&0x1F), U6ToByte((d>> 5)&0x3F), U5ToByte((d    )&0x1F),             255);}
      case IMAGE_R10G10B10A2: {UInt d=*(UInt*)data; return Color(         (d>> 2)&0xFF ,          (d>>12)&0xFF ,          (d>>22)&0xFF , U2ToByte(d>>30));}

      case IMAGE_R8_SIGN      : {  SByte   b=*(SByte *)data; return Color(SByteToByte(b  ),                0,                0,              255);}
      case IMAGE_R8G8_SIGN    : {C VecSB2 &v=*(VecSB2*)data; return Color(SByteToByte(v.x), SByteToByte(v.y),                0,              255);}
      case IMAGE_R8G8B8A8_SIGN: {C VecSB4 &v=*(VecSB4*)data; return Color(SByteToByte(v.x), SByteToByte(v.y), SByteToByte(v.z), SByteToByte(v.w));}

      case IMAGE_F32  :                        {C Flt  &v=*(Flt *)data; return Color(FltToByte(v  ),              0,              0,            255);}
      case IMAGE_F32_2:                        {C Vec2 &v=*(Vec2*)data; return Color(FltToByte(v.x), FltToByte(v.y),              0,            255);}
      case IMAGE_F32_3: case IMAGE_F32_3_SRGB: {C Vec  &v=*(Vec *)data; return Color(FltToByte(v.x), FltToByte(v.y), FltToByte(v.z),            255);}
      case IMAGE_F32_4: case IMAGE_F32_4_SRGB: {C Vec4 &v=*(Vec4*)data; return Color(FltToByte(v.x), FltToByte(v.y), FltToByte(v.z), FltToByte(v.w));}

      case IMAGE_F16  : {C Half  &v=*(Half *)data; return Color(FltToByte(v  ),              0,              0,            255);}
      case IMAGE_F16_2: {C VecH2 &v=*(VecH2*)data; return Color(FltToByte(v.x), FltToByte(v.y),              0,            255);}
      case IMAGE_F16_3: {C VecH  &v=*(VecH *)data; return Color(FltToByte(v.x), FltToByte(v.y), FltToByte(v.z),            255);}
      case IMAGE_F16_4: {C VecH4 &v=*(VecH4*)data; return Color(FltToByte(v.x), FltToByte(v.y), FltToByte(v.z), FltToByte(v.w));}

      case IMAGE_R11G11B10F: return GetR11G11B10F(data);

      case IMAGE_BC6: return LinearToSColor(DecompressPixelBC6(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3));

      case IMAGE_BC4_SIGN: {SByte  p=DecompressPixelBC4S(image.data() + (x>>2)* 8 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Color(SByteToByte(p  ),                0, 0, 255);}
      case IMAGE_BC5_SIGN: {VecSB2 p=DecompressPixelBC5S(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Color(SByteToByte(p.x), SByteToByte(p.y), 0, 255);}

      case IMAGE_ETC2_R_SIGN : {SByte  p=DecompressPixelETC2RS (image.data() + (x>>2)* 8 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Color(SByteToByte(p  ),                0, 0, 255);}
      case IMAGE_ETC2_RG_SIGN: {VecSB2 p=DecompressPixelETC2RGS(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Color(SByteToByte(p.x), SByteToByte(p.y), 0, 255);}

      case IMAGE_BC1       : case IMAGE_BC1_SRGB       :
      case IMAGE_BC2       : case IMAGE_BC2_SRGB       :
      case IMAGE_BC3       : case IMAGE_BC3_SRGB       :
      case IMAGE_BC4       :
      case IMAGE_BC5       :
      case IMAGE_BC7       : case IMAGE_BC7_SRGB       :
      case IMAGE_ETC1      :
      case IMAGE_ETC2_R    :
      case IMAGE_ETC2_RG   :
      case IMAGE_ETC2_RGB  : case IMAGE_ETC2_RGB_SRGB  :
      case IMAGE_ETC2_RGBA1: case IMAGE_ETC2_RGBA1_SRGB:
      case IMAGE_ETC2_RGBA : case IMAGE_ETC2_RGBA_SRGB :
         return _2d ? DecompressPixel(image, x, y) : DecompressPixel(image, x, y, z);
   }
   return TRANSPARENT;
}
Color Image::color(Int x, Int y)C
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      return GetColor(data() + x*bytePP() + y*pitch(), T, true, x, y);
   return TRANSPARENT;
}
Color Image::color3D(Int x, Int y, Int z)C
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      return GetColor(data() + x*bytePP() + y*pitch() + z*pitch2(), T, false, x, y, z);
   return TRANSPARENT;
}
/******************************************************************************/
static void SetColor(Byte *data, IMAGE_TYPE type, C Color &color)
{
   switch(type)
   {
      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: ((VecB4*)data)->set(color.b, color.g, color.r, color.a); break;
      case IMAGE_B8G8R8  : case IMAGE_B8G8R8_SRGB  : ((VecB *)data)->set(color.b, color.g, color.r); break;
      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: *(Color*)data=color; break;
      case IMAGE_R8G8B8  : case IMAGE_R8G8B8_SRGB  : ((VecB *)data)->set(color.r, color.g, color.b); break;
      case IMAGE_R8G8    :                           ((VecB2*)data)->set(color.r, color.g); break;
      case IMAGE_L8A8    : case IMAGE_L8A8_SRGB    : ((VecB2*)data)->set(color.lum(), color.a); break;
      case IMAGE_L8      : case IMAGE_L8_SRGB      : *        data=color.lum(); break;

      case IMAGE_R8      : *      data=color.r; break;
      case IMAGE_A8      : *      data=color.a; break;
      case IMAGE_I8      : *      data=color.r; break;
      case IMAGE_I16     : *(U16*)data=(color.r<<8); break;
      case IMAGE_I24     : *(U16*)data=0; data[2]=color.r; break;
      case IMAGE_I32     : *(U32*)data=(color.r<<24); break;

      case IMAGE_B4G4R4A4: *(U16*)data=((color.r>>4)<< 8) | ((color.g>>4)<<4) | (color.b>> 4) | ((color.a>>4)<<12); break;
      case IMAGE_B5G5R5A1: *(U16*)data=((color.r>>3)<<10) | ((color.g>>3)<<5) | (color.b>> 3) | ((color.a>>7)<<15); break;
      case IMAGE_B5G6R5  : *(U16*)data=((color.r>>3)<<11) | ((color.g>>2)<<5) | (color.b>> 3)                     ; break;

      case IMAGE_R8_SIGN      : *(SByte *)data =    (color.r>>1); break;
      case IMAGE_R8G8_SIGN    : ((VecSB2*)data)->set(color.r>>1, color.g>>1); break;
      case IMAGE_R8G8B8A8_SIGN: ((VecSB4*)data)->set(color.r>>1, color.g>>1, color.b>>1, color.a>>1); break;

      case IMAGE_R10G10B10A2: *(U32*)data=ByteToU10(color.r) | (ByteToU10(color.g)<<10) | (ByteToU10(color.b)<<20) | (ByteToU2(color.a)<<30); break;

      case IMAGE_R11G11B10F: SetR11G11B10F(data, color); break;

      case IMAGE_F32  :                        *(Flt *)data =     ByteToFlt(color.r); break;
      case IMAGE_F32_2:                        ((Vec2*)data)->set(ByteToFlt(color.r), ByteToFlt(color.g)); break;
      case IMAGE_F32_3: case IMAGE_F32_3_SRGB: ((Vec *)data)->set(ByteToFlt(color.r), ByteToFlt(color.g), ByteToFlt(color.b)); break;
      case IMAGE_F32_4: case IMAGE_F32_4_SRGB: ((Vec4*)data)->set(ByteToFlt(color.r), ByteToFlt(color.g), ByteToFlt(color.b), ByteToFlt(color.a)); break;

      case IMAGE_F16  : *(Half *)data =     ByteToFlt(color.r); break;
      case IMAGE_F16_2: ((VecH2*)data)->set(ByteToFlt(color.r), ByteToFlt(color.g)); break;
      case IMAGE_F16_3: ((VecH *)data)->set(ByteToFlt(color.r), ByteToFlt(color.g), ByteToFlt(color.b)); break;
      case IMAGE_F16_4: ((VecH4*)data)->set(ByteToFlt(color.r), ByteToFlt(color.g), ByteToFlt(color.b), ByteToFlt(color.a)); break;
   }
}
static void SetColor(Byte *data, IMAGE_TYPE type, IMAGE_TYPE hw_type, C Color &color)
{
   if(type==hw_type)normal: return SetColor(data, hw_type, color); // first check if types are the same, the most common case
   Color c; switch(type) // however if we want 'type' but we've got 'hw_type' then we have to adjust the color we're going to set. This will prevent setting different R G B values for type=IMAGE_L8 when hw_type=IMAGE_R8G8B8A8
   {
      case IMAGE_B8G8R8: case IMAGE_B8G8R8_SRGB:
      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB:
      case IMAGE_F16_3 :
      case IMAGE_F32_3 : case IMAGE_F32_3_SRGB:
         c.set(color.r, color.g, color.b, 255); break;

      case IMAGE_R8G8 :
      case IMAGE_F16_2:
      case IMAGE_F32_2:
         c.set(color.r, color.g, 0, 255); break;

      case IMAGE_R8 :
      case IMAGE_I8 :
      case IMAGE_I16:
      case IMAGE_I24:
      case IMAGE_I32:
      case IMAGE_F16:
      case IMAGE_F32:
         c.set(color.r, 0, 0, 255); break;

      case IMAGE_A8  :                       c.set(0, 0, 0    , color.a); break;
      case IMAGE_L8  : case IMAGE_L8_SRGB  : c.set(color.lum(),     255); break;
      case IMAGE_L8A8: case IMAGE_L8A8_SRGB: c.set(color.lum(), color.a); break;

      default: goto normal;
   }
   SetColor(data, hw_type, c);
}
void Image::color(Int x, Int y, C Color &color)
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetColor(data() + x*bytePP() + y*pitch(), type(), hwType(), color);
}
void Image::color3D(Int x, Int y, Int z, C Color &color)
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetColor(data() + x*bytePP() + y*pitch() + z*pitch2(), type(), hwType(), color);
}
/******************************************************************************/
static void _SetColorF(Byte *data, IMAGE_TYPE type, C Vec4 &color)
{
   switch(type)
   {
      case IMAGE_F32  :                        (*(Flt *)data)=color.x  ; break;
      case IMAGE_F32_2:                        (*(Vec2*)data)=color.xy ; break;
      case IMAGE_F32_3: case IMAGE_F32_3_SRGB: (*(Vec *)data)=color.xyz; break;
      case IMAGE_F32_4: case IMAGE_F32_4_SRGB: (*(Vec4*)data)=color    ; break;

      case IMAGE_F16  : (*(Half *)data)=color.x  ; break;
      case IMAGE_F16_2: (*(VecH2*)data)=color.xy ; break;
      case IMAGE_F16_3: (*(VecH *)data)=color.xyz; break;
      case IMAGE_F16_4: (*(VecH4*)data)=color    ; break;

      case IMAGE_A8: (*(U8*)data)=FltToByte(color.w); break; // it's okay to clamp int for small values
      case IMAGE_I8: (*(U8*)data)=FltToByte(color.x); break; // it's okay to clamp int for small values

      case IMAGE_I16: (*(U16*)data)=RoundU     (Sat(color.x)*0x0000FFFFu); break; // it's better to clamp flt for bigger values
      case IMAGE_I32: (*(U32*)data)=RoundUClamp(Sat(color.x)*0xFFFFFFFFu); break; // it's better to clamp flt for bigger values
      case IMAGE_I24: {  U32  c    =RoundU     (Sat(color.x)*0x00FFFFFFu); (*(U16*)data)=c; data[2]=(c>>16);} break; // it's better to clamp flt for bigger values

      case IMAGE_L8  : case IMAGE_L8_SRGB  :   (*(U8*)data)=     FltToByte(color.xyz.max()); break; // it's okay to clamp int for small values
      case IMAGE_L8A8: case IMAGE_L8A8_SRGB: ((VecB2*)data)->set(FltToByte(color.xyz.max()), FltToByte(color.w)); break;

      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: ((VecB4*)data)->set(FltToByte(color.z), FltToByte(color.y), FltToByte(color.x), FltToByte(color.w)); break;
      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: ((VecB4*)data)->set(FltToByte(color.x), FltToByte(color.y), FltToByte(color.z), FltToByte(color.w)); break;
      case IMAGE_R8G8B8  : case IMAGE_R8G8B8_SRGB  : ((VecB *)data)->set(FltToByte(color.x), FltToByte(color.y), FltToByte(color.z)                    ); break;
      case IMAGE_R8G8    :                           ((VecB2*)data)->set(FltToByte(color.x), FltToByte(color.y)                                        ); break;
      case IMAGE_R8      :                           *(Byte *)data  =    FltToByte(color.x)                                                             ; break;
      case IMAGE_B8G8R8  : case IMAGE_B8G8R8_SRGB  : ((VecB *)data)->set(FltToByte(color.z), FltToByte(color.y), FltToByte(color.x)                    ); break;

      case IMAGE_R8_SIGN      : *(SByte *)data =     SFltToSByte(color.x)                                                                   ; break;
      case IMAGE_R8G8_SIGN    : ((VecSB2*)data)->set(SFltToSByte(color.x), SFltToSByte(color.y)                                            ); break;
      case IMAGE_R8G8B8A8_SIGN: ((VecSB4*)data)->set(SFltToSByte(color.x), SFltToSByte(color.y), SFltToSByte(color.z), SFltToSByte(color.w)); break;

      case IMAGE_R10G10B10A2: {(*(UInt*)data)=FltToU10(color.x)|(FltToU10(color.y)<<10)|(FltToU10(color.z)<<20)|(FltToU2(color.w)<<30);} break;

      case IMAGE_R11G11B10F: SetR11G11B10F(data, color.xyz); break;
   }
}
static void SetColorF(Byte *data, IMAGE_TYPE type, IMAGE_TYPE hw_type, C Vec4 &color)
{
   if(type==hw_type)normal: return _SetColorF(data, hw_type, color); // first check if types are the same, the most common case
   Vec4 c; switch(type) // however if we want 'type' but we've got 'hw_type' then we have to adjust the color we're going to set. This will prevent setting different R G B values for type=IMAGE_L8 when hw_type=IMAGE_R8G8B8A8
   {
      case IMAGE_B8G8R8: case IMAGE_B8G8R8_SRGB:
      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB:
      case IMAGE_F16_3 :
      case IMAGE_F32_3 : case IMAGE_F32_3_SRGB:
         c.set(color.x, color.y, color.z, 1); break;

      case IMAGE_R8G8 :
      case IMAGE_F16_2:
      case IMAGE_F32_2:
         c.set(color.x, color.y, 0, 1); break;

      case IMAGE_R8 :
      case IMAGE_I8 :
      case IMAGE_I16:
      case IMAGE_I24:
      case IMAGE_I32:
      case IMAGE_F16:
      case IMAGE_F32:
         c.set(color.x, 0, 0, 1); break;

      case IMAGE_A8  :                                               c.set(0, 0, 0, color.w);  break;
      case IMAGE_L8  : case IMAGE_L8_SRGB  : {Flt l=color.xyz.max(); c.set(l, l, l,       1);} break;
      case IMAGE_L8A8: case IMAGE_L8A8_SRGB: {Flt l=color.xyz.max(); c.set(l, l, l, color.w);} break;

      default: goto normal;
   }
  _SetColorF(data, hw_type, c);
}
void Image::colorF(Int x, Int y, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetColorF(data() + x*bytePP() + y*pitch(), type(), hwType(), color);
}
void Image::color3DF(Int x, Int y, Int z, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetColorF(data() + x*bytePP() + y*pitch() + z*pitch2(), type(), hwType(), color);
}
/******************************************************************************/
static void _SetColorL(Byte *data, IMAGE_TYPE type, C Vec4 &color)
{
   switch(type)
   {
      case IMAGE_F32  : (*(Flt *)data)=color.x  ; break;
      case IMAGE_F32_2: (*(Vec2*)data)=color.xy ; break;
      case IMAGE_F32_3: (*(Vec *)data)=color.xyz; break;
      case IMAGE_F32_4: (*(Vec4*)data)=color    ; break;

      case IMAGE_F32_3_SRGB: (*(Vec *)data)=LinearToSRGB(color.xyz); break;
      case IMAGE_F32_4_SRGB: (*(Vec4*)data)=LinearToSRGB(color    ); break;

      case IMAGE_F16  : (*(Half *)data)=color.x  ; break;
      case IMAGE_F16_2: (*(VecH2*)data)=color.xy ; break;
      case IMAGE_F16_3: (*(VecH *)data)=color.xyz; break;
      case IMAGE_F16_4: (*(VecH4*)data)=color    ; break;

      case IMAGE_A8: (*(U8*)data)=FltToByte(color.w); break; // it's okay to clamp int for small values
      case IMAGE_I8: (*(U8*)data)=FltToByte(color.x); break; // it's okay to clamp int for small values

      case IMAGE_I16: (*(U16*)data)=RoundU     (Sat(color.x)*0x0000FFFFu); break; // it's better to clamp flt for bigger values
      case IMAGE_I32: (*(U32*)data)=RoundUClamp(Sat(color.x)*0xFFFFFFFFu); break; // it's better to clamp flt for bigger values
      case IMAGE_I24: {  U32  c    =RoundU     (Sat(color.x)*0x00FFFFFFu); (*(U16*)data)=c; data[2]=(c>>16);} break; // it's better to clamp flt for bigger values

      case IMAGE_L8     : (*(U8*)data)=   FltToByte    (color.xyz.max()); break; // it's okay to clamp int for small values
      case IMAGE_L8_SRGB: (*(U8*)data)=LinearToByteSRGB(color.xyz.max()); break; // it's okay to clamp int for small values

      case IMAGE_L8A8     : ((VecB2*)data)->set(   FltToByte    (color.xyz.max()), FltToByte(color.w)); break;
      case IMAGE_L8A8_SRGB: ((VecB2*)data)->set(LinearToByteSRGB(color.xyz.max()), FltToByte(color.w)); break;

      case IMAGE_B8G8R8A8: ((VecB4*)data)->set(FltToByte(color.z), FltToByte(color.y), FltToByte(color.x), FltToByte(color.w)); break;
      case IMAGE_R8G8B8A8: ((VecB4*)data)->set(FltToByte(color.x), FltToByte(color.y), FltToByte(color.z), FltToByte(color.w)); break;
      case IMAGE_R8G8B8  : ((VecB *)data)->set(FltToByte(color.x), FltToByte(color.y), FltToByte(color.z)                    ); break;
      case IMAGE_R8G8    : ((VecB2*)data)->set(FltToByte(color.x), FltToByte(color.y)                                        ); break;
      case IMAGE_R8      : *(Byte *)data  =    FltToByte(color.x)                                                             ; break;
      case IMAGE_B8G8R8  : ((VecB *)data)->set(FltToByte(color.z), FltToByte(color.y), FltToByte(color.x)                    ); break;

      case IMAGE_B8G8R8A8_SRGB: ((VecB4*)data)->set(LinearToByteSRGB(color.z), LinearToByteSRGB(color.y), LinearToByteSRGB(color.x), FltToByte(color.w)); break;
      case IMAGE_R8G8B8A8_SRGB: ((VecB4*)data)->set(LinearToByteSRGB(color.x), LinearToByteSRGB(color.y), LinearToByteSRGB(color.z), FltToByte(color.w)); break;
      case IMAGE_B8G8R8_SRGB  : ((VecB *)data)->set(LinearToByteSRGB(color.z), LinearToByteSRGB(color.y), LinearToByteSRGB(color.x)                    ); break;
      case IMAGE_R8G8B8_SRGB  : ((VecB *)data)->set(LinearToByteSRGB(color.x), LinearToByteSRGB(color.y), LinearToByteSRGB(color.z)                    ); break;

      case IMAGE_R8_SIGN      : *(SByte *)data =     SFltToSByte(color.x)                                                                   ; break;
      case IMAGE_R8G8_SIGN    : ((VecSB2*)data)->set(SFltToSByte(color.x), SFltToSByte(color.y)                                            ); break;
      case IMAGE_R8G8B8A8_SIGN: ((VecSB4*)data)->set(SFltToSByte(color.x), SFltToSByte(color.y), SFltToSByte(color.z), SFltToSByte(color.w)); break;

      case IMAGE_R10G10B10A2: {(*(UInt*)data)=FltToU10(color.x)|(FltToU10(color.y)<<10)|(FltToU10(color.z)<<20)|(FltToU2(color.w)<<30);} break;

      case IMAGE_R11G11B10F: SetR11G11B10F(data, color.xyz); break;
   }
}
static void SetColorL(Byte *data, IMAGE_TYPE type, IMAGE_TYPE hw_type, C Vec4 &color)
{
   if(type==hw_type)normal: return _SetColorL(data, hw_type, color); // first check if types are the same, the most common case
   Vec4 c; switch(type) // however if we want 'type' but we've got 'hw_type' then we have to adjust the color we're going to set. This will prevent setting different R G B values for type=IMAGE_L8 when hw_type=IMAGE_R8G8B8A8
   {
      case IMAGE_B8G8R8: case IMAGE_B8G8R8_SRGB:
      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB:
      case IMAGE_F16_3 :
      case IMAGE_F32_3 : case IMAGE_F32_3_SRGB:
         c.set(color.x, color.y, color.z, 1); break;

      case IMAGE_R8G8 :
      case IMAGE_F16_2:
      case IMAGE_F32_2:
         c.set(color.x, color.y, 0, 1); break;

      case IMAGE_R8 :
      case IMAGE_I8 :
      case IMAGE_I16:
      case IMAGE_I24:
      case IMAGE_I32:
      case IMAGE_F16:
      case IMAGE_F32:
         c.set(color.x, 0, 0, 1); break;

      case IMAGE_A8  :                                               c.set(0, 0, 0, color.w);  break;
      case IMAGE_L8  : case IMAGE_L8_SRGB  : {Flt l=color.xyz.max(); c.set(l, l, l,       1);} break;
      case IMAGE_L8A8: case IMAGE_L8A8_SRGB: {Flt l=color.xyz.max(); c.set(l, l, l, color.w);} break;

      default: goto normal;
   }
  _SetColorL(data, hw_type, c);
}
void Image::colorL(Int x, Int y, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetColorL(data() + x*bytePP() + y*pitch(), type(), hwType(), color);
}
void Image::color3DL(Int x, Int y, Int z, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetColorL(data() + x*bytePP() + y*pitch() + z*pitch2(), type(), hwType(), color);
}
/******************************************************************************/
static void _SetColorS(Byte *data, IMAGE_TYPE type, C Vec4 &color)
{
   switch(type)
   {
      case IMAGE_F32  : (*(Flt *)data)=    SRGBToLinear(color.x  ); break;
      case IMAGE_F32_2: (*(Vec2*)data).set(SRGBToLinear(color.x  ), SRGBToLinear(color.y)); break;
      case IMAGE_F32_3: (*(Vec *)data)=    SRGBToLinear(color.xyz); break;
      case IMAGE_F32_4: (*(Vec4*)data)=    SRGBToLinear(color    ); break;

      case IMAGE_F32_3_SRGB: (*(Vec *)data)=color.xyz; break;
      case IMAGE_F32_4_SRGB: (*(Vec4*)data)=color    ; break;

      case IMAGE_F16  : (*(Half *)data)=    SRGBToLinear(color.x  ); break;
      case IMAGE_F16_2: (*(VecH2*)data).set(SRGBToLinear(color.x  ), SRGBToLinear(color.y)); break;
      case IMAGE_F16_3: (*(VecH *)data)=    SRGBToLinear(color.xyz); break;
      case IMAGE_F16_4: (*(VecH4*)data)=    SRGBToLinear(color    ); break;

      case IMAGE_A8: (*(U8*)data)= FltToByte      (color.w); break; // it's okay to clamp int for small values
      case IMAGE_I8: (*(U8*)data)=SRGBToLinearByte(color.x); break; // it's okay to clamp int for small values

      case IMAGE_I16: (*(U16*)data)=RoundU     (Sat(SRGBToLinear(color.x))*0x0000FFFFu); break; // it's better to clamp flt for bigger values
      case IMAGE_I32: (*(U32*)data)=RoundUClamp(Sat(SRGBToLinear(color.x))*0xFFFFFFFFu); break; // it's better to clamp flt for bigger values
      case IMAGE_I24: {  U32  c    =RoundU     (Sat(SRGBToLinear(color.x))*0x00FFFFFFu); (*(U16*)data)=c; data[2]=(c>>16);} break; // it's better to clamp flt for bigger values

      case IMAGE_L8     : (*(U8*)data)=SRGBToLinearByte(color.xyz.max()); break; // it's okay to clamp int for small values
      case IMAGE_L8_SRGB: (*(U8*)data)= FltToByte      (color.xyz.max()); break; // it's okay to clamp int for small values

      case IMAGE_L8A8     : ((VecB2*)data)->set(SRGBToLinearByte(color.xyz.max()), FltToByte(color.w)); break;
      case IMAGE_L8A8_SRGB: ((VecB2*)data)->set( FltToByte      (color.xyz.max()), FltToByte(color.w)); break;

      case IMAGE_B8G8R8A8: ((VecB4*)data)->set(SRGBToLinearByte(color.z), SRGBToLinearByte(color.y), SRGBToLinearByte(color.x), FltToByte(color.w)); break;
      case IMAGE_R8G8B8A8: ((VecB4*)data)->set(SRGBToLinearByte(color.x), SRGBToLinearByte(color.y), SRGBToLinearByte(color.z), FltToByte(color.w)); break;
      case IMAGE_R8G8B8  : ((VecB *)data)->set(SRGBToLinearByte(color.x), SRGBToLinearByte(color.y), SRGBToLinearByte(color.z)                    ); break;
      case IMAGE_R8G8    : ((VecB2*)data)->set(SRGBToLinearByte(color.x), SRGBToLinearByte(color.y)                                               ); break;
      case IMAGE_R8      : *(Byte *)data  =    SRGBToLinearByte(color.x)                                                                           ; break;
      case IMAGE_B8G8R8  : ((VecB *)data)->set(SRGBToLinearByte(color.z), SRGBToLinearByte(color.y), SRGBToLinearByte(color.x)                    ); break;

      case IMAGE_B8G8R8A8_SRGB: ((VecB4*)data)->set(FltToByte(color.z), FltToByte(color.y), FltToByte(color.x), FltToByte(color.w)); break;
      case IMAGE_R8G8B8A8_SRGB: ((VecB4*)data)->set(FltToByte(color.x), FltToByte(color.y), FltToByte(color.z), FltToByte(color.w)); break;
      case IMAGE_B8G8R8_SRGB  : ((VecB *)data)->set(FltToByte(color.z), FltToByte(color.y), FltToByte(color.x)                    ); break;
      case IMAGE_R8G8B8_SRGB  : ((VecB *)data)->set(FltToByte(color.x), FltToByte(color.y), FltToByte(color.z)                    ); break;

      case IMAGE_R8_SIGN      : *(SByte *)data =     SFltToSByte(SignSRGBToLinear(color.x))                                                                                                       ; break;
      case IMAGE_R8G8_SIGN    : ((VecSB2*)data)->set(SFltToSByte(SignSRGBToLinear(color.x)), SFltToSByte(SignSRGBToLinear(color.y))                                                              ); break;
      case IMAGE_R8G8B8A8_SIGN: ((VecSB4*)data)->set(SFltToSByte(SignSRGBToLinear(color.x)), SFltToSByte(SignSRGBToLinear(color.y)), SFltToSByte(SignSRGBToLinear(color.z)), SFltToSByte(color.w)); break;

      case IMAGE_R10G10B10A2: {(*(UInt*)data)=FltToU10(SRGBToLinear(color.x))|(FltToU10(SRGBToLinear(color.y))<<10)|(FltToU10(SRGBToLinear(color.z))<<20)|(FltToU2(color.w)<<30);} break;

      case IMAGE_R11G11B10F: SetR11G11B10F(data, SRGBToLinear(color.xyz)); break;
   }
}
static void SetColorS(Byte *data, IMAGE_TYPE type, IMAGE_TYPE hw_type, C Vec4 &color)
{
   if(type==hw_type)normal: return _SetColorS(data, hw_type, color); // first check if types are the same, the most common case
   Vec4 c; switch(type) // however if we want 'type' but we've got 'hw_type' then we have to adjust the color we're going to set. This will prevent setting different R G B values for type=IMAGE_L8 when hw_type=IMAGE_R8G8B8A8
   {
      case IMAGE_B8G8R8: case IMAGE_B8G8R8_SRGB:
      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB:
      case IMAGE_F16_3 :
      case IMAGE_F32_3 : case IMAGE_F32_3_SRGB:
         c.set(color.x, color.y, color.z, 1); break;

      case IMAGE_R8G8 :
      case IMAGE_F16_2:
      case IMAGE_F32_2:
         c.set(color.x, color.y, 0, 1); break;

      case IMAGE_R8 :
      case IMAGE_I8 :
      case IMAGE_I16:
      case IMAGE_I24:
      case IMAGE_I32:
      case IMAGE_F16:
      case IMAGE_F32:
         c.set(color.x, 0, 0, 1); break;

      case IMAGE_A8  :                                               c.set(0, 0, 0, color.w);  break;
      case IMAGE_L8  : case IMAGE_L8_SRGB  : {Flt l=color.xyz.max(); c.set(l, l, l,       1);} break;
      case IMAGE_L8A8: case IMAGE_L8A8_SRGB: {Flt l=color.xyz.max(); c.set(l, l, l, color.w);} break;

      default: goto normal;
   }
  _SetColorS(data, hw_type, c);
}
void Image::colorS(Int x, Int y, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetColorS(data() + x*bytePP() + y*pitch(), type(), hwType(), color);
}
void Image::color3DS(Int x, Int y, Int z, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      SetColorS(data() + x*bytePP() + y*pitch() + z*pitch2(), type(), hwType(), color);
}
/******************************************************************************/
static inline void ApplyBlend(Vec4 &src, C Vec4 &color) {src=Blend(src, color);}
void Image::blendF(Int x, Int y, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
      Vec4 src; Byte *data=T.data() + x*bytePP() + y*pitch();
      switch(hwType())
      {
         case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: {Color &c=*(Color*)data; src=c; ApplyBlend(src, color); c=src;} break;
         case IMAGE_F32_4   : case IMAGE_F32_4_SRGB   : {Vec4  &c=*(Vec4 *)data;        ApplyBlend(  c, color);       } break;
         default            :                           {             src=colorF(x, y); ApplyBlend(src, color); SetColorF(data, type(), hwType(), src);} break;
      }
   }
}
/*void Image::blendL(Int x, Int y, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
      Vec4 src; Byte *data=T.data() + x*bytePP() + y*pitch();
      switch(hwType())
      {
         case IMAGE_R8G8B8A8     : {Color &c=*(Color*)data; src=             c ; ApplyBlend(src, color); c=               src ;} break;
         case IMAGE_R8G8B8A8_SRGB: {Color &c=*(Color*)data; src=SRGBToLinear(c); ApplyBlend(src, color); c=LinearToSColor(src);} break;
         case IMAGE_F32_4        : {Vec4  &c=*(Vec4 *)data;                      ApplyBlend(  c, color);                       } break;
         case IMAGE_F32_4_SRGB   : {Vec4  &c=*(Vec4 *)data; src=SRGBToLinear(c); ApplyBlend(  c, color); c=LinearToSRGB  (src);} break;
         default                 : {                        src=   colorL(x, y); ApplyBlend(src, color); SetColorL(data, type(), hwType(), src);} break;
      }
   }
}*/
static inline void ApplyMerge(Vec4 &src, C Vec4 &color) {src=MergeBlend(src, color);}
void Image::mergeF(Int x, Int y, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
      Vec4 src; Byte *data=T.data() + x*bytePP() + y*pitch();
      switch(hwType())
      {
         case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: {Color &c=*(Color*)data; src=c; ApplyMerge(src, color); c=src;} break;
         case IMAGE_F32_4   : case IMAGE_F32_4_SRGB   : {Vec4  &c=*(Vec4 *)data;        ApplyMerge(  c, color);       } break;
         default            :                           {             src=colorF(x, y); ApplyMerge(src, color); SetColorF(data, type(), hwType(), src);} break;
      }
   }
}
/*void Image::mergeL(Int x, Int y, C Vec4 &color)
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
   {
      Vec4 src; Byte *data=T.data() + x*bytePP() + y*pitch();
      switch(hwType())
      {
         case IMAGE_R8G8B8A8     : {Color &c=*(Color*)data; src=             c ; ApplyMerge(src, color); c=               src ;} break;
         case IMAGE_R8G8B8A8_SRGB: {Color &c=*(Color*)data; src=SRGBToLinear(c); ApplyMerge(src, color); c=LinearToSColor(src);} break;
         case IMAGE_F32_4        : {Vec4  &c=*(Vec4 *)data;                      ApplyMerge(  c, color);                       } break;
         case IMAGE_F32_4_SRGB   : {Vec4  &c=*(Vec4 *)data; src=SRGBToLinear(c); ApplyMerge(  c, color); c=LinearToSRGB  (src);} break;
         default                 : {                        src=   colorL(x, y); ApplyMerge(src, color); SetColorL(data, type(), hwType(), src);} break;
      }
   }
}
/******************************************************************************/
Vec4 ImageColorF(CPtr data, IMAGE_TYPE hw_type)
{
   switch(hw_type)
   {
      case IMAGE_F32  :                        return Vec4(*(Flt *)data, 0, 0, 1);
      case IMAGE_F32_2:                        return Vec4(*(Vec2*)data,    0, 1);
      case IMAGE_F32_3: case IMAGE_F32_3_SRGB: return Vec4(*(Vec *)data,       1);
      case IMAGE_F32_4: case IMAGE_F32_4_SRGB: return      *(Vec4*)data          ;

      case IMAGE_F16  : return Vec4(((Half*)data)[0],                0,                0,                1);
      case IMAGE_F16_2: return Vec4(((Half*)data)[0], ((Half*)data)[1],                0,                1);
      case IMAGE_F16_3: return Vec4(((Half*)data)[0], ((Half*)data)[1], ((Half*)data)[2],                1);
      case IMAGE_F16_4: return Vec4(((Half*)data)[0], ((Half*)data)[1], ((Half*)data)[2], ((Half*)data)[3]);

      case IMAGE_A8: return Vec4(0, 0, 0, ByteToFlt(*(U8*)data));
      case IMAGE_I8: return Vec4(Vec(     ByteToFlt(*(U8*)data)), 1);

      // 16
   #if SUPPORT_DEPTH_TO_COLOR
      case IMAGE_D16: if(GL)return Vec4(Vec((*(U16*)data)/Flt(0x0000FFFFu)*2-1), 1); // !! else fall through no break on purpose !!
   #endif
      case IMAGE_I16:
         return Vec4(Vec((*(U16*)data)/Flt(0x0000FFFFu)), 1);

      // 32
   #if SUPPORT_DEPTH_TO_COLOR
      case IMAGE_D32     :
      case IMAGE_D32S8X24: return Vec4(Vec(GL ? (*(Flt*)data)*2-1 : *(Flt*)data), 1);
    //case IMAGE_D32I    : if(GL)return Vec4(Vec((*(U32*)data)/Dbl(0xFFFFFFFFu)*2-1), 1); // !! else fall through no break on purpose !!
   #endif
      case IMAGE_I32:
         return Vec4(Vec((*(U32*)data)/Dbl(0xFFFFFFFFu)), 1); // Dbl required to get best precision

      // 24
   #if SUPPORT_DEPTH_TO_COLOR
      case IMAGE_D24S8:
      case IMAGE_D24X8: if(GL)return Vec4(Vec((*(U16*)(((Byte*)data)+1) | (((Byte*)data)[3]<<16))/Flt(0x00FFFFFFu)*2-1), 1); // !! else fall through no break on purpose !!
   #endif
      case IMAGE_I24:
         return Vec4(Vec((*(U16*)data | (((Byte*)data)[2]<<16))/Flt(0x00FFFFFFu)), 1); // here Dbl is not required, this was tested

      case IMAGE_L8  : case IMAGE_L8_SRGB  : {Byte  &b=*(Byte *)data; Flt l=ByteToFlt(b  ); return Vec4(l, l, l,              1);}
      case IMAGE_L8A8: case IMAGE_L8A8_SRGB: {VecB2 &c=*(VecB2*)data; Flt l=ByteToFlt(c.x); return Vec4(l, l, l, ByteToFlt(c.y));}

      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: {VecB4 &c=*(VecB4*)data; return Vec4(ByteToFlt(c.z), ByteToFlt(c.y), ByteToFlt(c.x), ByteToFlt(c.w));}
      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: {VecB4 &c=*(VecB4*)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y), ByteToFlt(c.z), ByteToFlt(c.w));}
      case IMAGE_B8G8R8  : case IMAGE_B8G8R8_SRGB  : {VecB  &c=*(VecB *)data; return Vec4(ByteToFlt(c.z), ByteToFlt(c.y), ByteToFlt(c.x),              1);}
      case IMAGE_R8G8B8  : case IMAGE_R8G8B8_SRGB  : {VecB  &c=*(VecB *)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y), ByteToFlt(c.z),              1);}
      case IMAGE_R8G8    :                           {VecB2 &c=*(VecB2*)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y),              0,              1);}
      case IMAGE_R8      :                           {Byte   c=*(Byte *)data; return Vec4(ByteToFlt(c  ),              0,              0,              1);}

      case IMAGE_R8_SIGN      : {SByte  &c=*(SByte *)data; return Vec4(SByteToSFlt(c  ),                0,                0,                1);}
      case IMAGE_R8G8_SIGN    : {VecSB2 &c=*(VecSB2*)data; return Vec4(SByteToSFlt(c.x), SByteToSFlt(c.y),                0,                1);}
      case IMAGE_R8G8B8A8_SIGN: {VecSB4 &c=*(VecSB4*)data; return Vec4(SByteToSFlt(c.x), SByteToSFlt(c.y), SByteToSFlt(c.z), SByteToSFlt(c.w));}

      case IMAGE_R10G10B10A2: {UInt u=*(UInt*)data; return Vec4(U10ToFlt(u&0x3FF), U10ToFlt((u>>10)&0x3FF), U10ToFlt((u>>20)&0x3FF), U2ToFlt(u>>30));}

      case IMAGE_R11G11B10F: return Vec4(GetR11G11B10F(data), 1);
   }
   return 0;
}
static inline Vec4 GetColorF(CPtr data, C Image &image, Bool _2d, Int x, Int y, Int z=0)
{
   switch(image.hwType())
   {
      case IMAGE_F32  :                        return Vec4(*(Flt *)data, 0, 0, 1);
      case IMAGE_F32_2:                        return Vec4(*(Vec2*)data,    0, 1);
      case IMAGE_F32_3: case IMAGE_F32_3_SRGB: return Vec4(*(Vec *)data,       1);
      case IMAGE_F32_4: case IMAGE_F32_4_SRGB: return      *(Vec4*)data          ;

      case IMAGE_F16  : return Vec4(((Half*)data)[0],                0,                0,                1);
      case IMAGE_F16_2: return Vec4(((Half*)data)[0], ((Half*)data)[1],                0,                1);
      case IMAGE_F16_3: return Vec4(((Half*)data)[0], ((Half*)data)[1], ((Half*)data)[2],                1);
      case IMAGE_F16_4: return Vec4(((Half*)data)[0], ((Half*)data)[1], ((Half*)data)[2], ((Half*)data)[3]);

      case IMAGE_A8: return Vec4(0, 0, 0, ByteToFlt(*(U8*)data));
      case IMAGE_I8: return Vec4(Vec(     ByteToFlt(*(U8*)data)), 1);

      // 16
   #if SUPPORT_DEPTH_TO_COLOR
      case IMAGE_D16: if(GL)return Vec4(Vec((*(U16*)data)/Flt(0x0000FFFFu)*2-1), 1); // !! else fall through no break on purpose !!
   #endif
      case IMAGE_I16:
         return Vec4(Vec((*(U16*)data)/Flt(0x0000FFFFu)), 1);

      // 32
   #if SUPPORT_DEPTH_TO_COLOR
      case IMAGE_D32     :
      case IMAGE_D32S8X24: return Vec4(Vec(GL ? (*(Flt*)data)*2-1 : *(Flt*)data), 1);
    //case IMAGE_D32I    : if(GL)return Vec4(Vec((*(U32*)data)/Dbl(0xFFFFFFFFu)*2-1), 1); // !! else fall through no break on purpose !!
   #endif
      case IMAGE_I32:
         return Vec4(Vec((*(U32*)data)/Dbl(0xFFFFFFFFu)), 1); // Dbl required to get best precision

      // 24
   #if SUPPORT_DEPTH_TO_COLOR
      case IMAGE_D24S8:
      case IMAGE_D24X8: if(GL)return Vec4(Vec((*(U16*)(((Byte*)data)+1) | (((Byte*)data)[3]<<16))/Flt(0x00FFFFFFu)*2-1), 1); // !! else fall through no break on purpose !!
   #endif
      case IMAGE_I24:
         return Vec4(Vec((*(U16*)data | (((Byte*)data)[2]<<16))/Flt(0x00FFFFFFu)), 1); // here Dbl is not required, this was tested

      case IMAGE_L8  : case IMAGE_L8_SRGB  : {Byte  &b=*(Byte *)data; Flt l=ByteToFlt(b  ); return Vec4(l, l, l,              1);}
      case IMAGE_L8A8: case IMAGE_L8A8_SRGB: {VecB2 &c=*(VecB2*)data; Flt l=ByteToFlt(c.x); return Vec4(l, l, l, ByteToFlt(c.y));}

      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: {VecB4 &c=*(VecB4*)data; return Vec4(ByteToFlt(c.z), ByteToFlt(c.y), ByteToFlt(c.x), ByteToFlt(c.w));}
      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: {VecB4 &c=*(VecB4*)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y), ByteToFlt(c.z), ByteToFlt(c.w));}
      case IMAGE_B8G8R8  : case IMAGE_B8G8R8_SRGB  : {VecB  &c=*(VecB *)data; return Vec4(ByteToFlt(c.z), ByteToFlt(c.y), ByteToFlt(c.x),              1);}
      case IMAGE_R8G8B8  : case IMAGE_R8G8B8_SRGB  : {VecB  &c=*(VecB *)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y), ByteToFlt(c.z),              1);}
      case IMAGE_R8G8    :                           {VecB2 &c=*(VecB2*)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y),              0,              1);}
      case IMAGE_R8      :                           {Byte   c=*(Byte *)data; return Vec4(ByteToFlt(c  ),              0,              0,              1);}

      case IMAGE_R8_SIGN      : {SByte  &c=*(SByte *)data; return Vec4(SByteToSFlt(c  ),                0,                0,                1);}
      case IMAGE_R8G8_SIGN    : {VecSB2 &c=*(VecSB2*)data; return Vec4(SByteToSFlt(c.x), SByteToSFlt(c.y),                0,                1);}
      case IMAGE_R8G8B8A8_SIGN: {VecSB4 &c=*(VecSB4*)data; return Vec4(SByteToSFlt(c.x), SByteToSFlt(c.y), SByteToSFlt(c.z), SByteToSFlt(c.w));}

      case IMAGE_R10G10B10A2: {UInt u=*(UInt*)data; return Vec4(U10ToFlt(u&0x3FF), U10ToFlt((u>>10)&0x3FF), U10ToFlt((u>>20)&0x3FF), U2ToFlt(u>>30));}

      case IMAGE_R11G11B10F: return Vec4(GetR11G11B10F(data), 1);

      case IMAGE_BC6: return Vec4(DecompressPixelBC6(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3), 1);

      case IMAGE_BC4_SIGN: {SByte  p=DecompressPixelBC4S(image.data() + (x>>2)* 8 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Vec4(SByteToSFlt(p  ),                0, 0, 1);}
      case IMAGE_BC5_SIGN: {VecSB2 p=DecompressPixelBC5S(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Vec4(SByteToSFlt(p.x), SByteToSFlt(p.y), 0, 1);}

      case IMAGE_ETC2_R_SIGN : {SByte  p=DecompressPixelETC2RS (image.data() + (x>>2)* 8 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Vec4(SByteToSFlt(p  ),                0, 0, 1);}
      case IMAGE_ETC2_RG_SIGN: {VecSB2 p=DecompressPixelETC2RGS(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Vec4(SByteToSFlt(p.x), SByteToSFlt(p.y), 0, 1);}

      case IMAGE_BC1       : case IMAGE_BC1_SRGB       :
      case IMAGE_BC2       : case IMAGE_BC2_SRGB       :
      case IMAGE_BC3       : case IMAGE_BC3_SRGB       :
      case IMAGE_BC4       :
      case IMAGE_BC5       :
      case IMAGE_BC7       : case IMAGE_BC7_SRGB       :
      case IMAGE_ETC1      :
      case IMAGE_ETC2_R    :
      case IMAGE_ETC2_RG   :
      case IMAGE_ETC2_RGB  : case IMAGE_ETC2_RGB_SRGB  :
      case IMAGE_ETC2_RGBA1: case IMAGE_ETC2_RGBA1_SRGB:
      case IMAGE_ETC2_RGBA : case IMAGE_ETC2_RGBA_SRGB :
         return _2d ? DecompressPixel(image, x, y) : DecompressPixel(image, x, y, z);
   }
   return 0;
}
Vec4 Image::colorF(Int x, Int y)C
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      return GetColorF(data() + x*bytePP() + y*pitch(), T, true, x, y);
   return 0;
}
Vec4 Image::color3DF(Int x, Int y, Int z)C
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      return GetColorF(data() + x*bytePP() + y*pitch() + z*pitch2(), T, false, x, y, z);
   return 0;
}
/******************************************************************************/
Vec4 ImageColorL(CPtr data, IMAGE_TYPE hw_type)
{
   switch(hw_type)
   {
      case IMAGE_F32  : return Vec4(*(Flt *)data, 0, 0, 1);
      case IMAGE_F32_2: return Vec4(*(Vec2*)data,    0, 1);
      case IMAGE_F32_3: return Vec4(*(Vec *)data,       1);
      case IMAGE_F32_4: return      *(Vec4*)data          ;

      case IMAGE_F32_3_SRGB: return Vec4(SRGBToLinear(*(Vec *)data), 1);
      case IMAGE_F32_4_SRGB: return      SRGBToLinear(*(Vec4*)data)    ;

      case IMAGE_F16  : return Vec4(((Half*)data)[0],                0,                0,                1);
      case IMAGE_F16_2: return Vec4(((Half*)data)[0], ((Half*)data)[1],                0,                1);
      case IMAGE_F16_3: return Vec4(((Half*)data)[0], ((Half*)data)[1], ((Half*)data)[2],                1);
      case IMAGE_F16_4: return Vec4(((Half*)data)[0], ((Half*)data)[1], ((Half*)data)[2], ((Half*)data)[3]);

      case IMAGE_A8: return Vec4(0, 0, 0, ByteToFlt(*(U8*)data));
      case IMAGE_I8: return Vec4(Vec(     ByteToFlt(*(U8*)data)), 1);

      // 16
   #if SUPPORT_DEPTH_TO_COLOR
      case IMAGE_D16: if(GL)return Vec4(Vec((*(U16*)data)/Flt(0x0000FFFFu)*2-1), 1); // !! else fall through no break on purpose !!
   #endif
      case IMAGE_I16:
         return Vec4(Vec((*(U16*)data)/Flt(0x0000FFFFu)), 1);

      // 32
   #if SUPPORT_DEPTH_TO_COLOR
      case IMAGE_D32     :
      case IMAGE_D32S8X24: return Vec4(Vec(GL ? (*(Flt*)data)*2-1 : *(Flt*)data), 1);
    //case IMAGE_D32I    : if(GL)return Vec4(Vec((*(U32*)data)/Dbl(0xFFFFFFFFu)*2-1), 1); // !! else fall through no break on purpose !!
   #endif
      case IMAGE_I32:
         return Vec4(Vec((*(U32*)data)/Dbl(0xFFFFFFFFu)), 1); // Dbl required to get best precision

      // 24
   #if SUPPORT_DEPTH_TO_COLOR
      case IMAGE_D24S8:
      case IMAGE_D24X8: if(GL)return Vec4(Vec((*(U16*)(((Byte*)data)+1) | (((Byte*)data)[3]<<16))/Flt(0x00FFFFFFu)*2-1), 1); // !! else fall through no break on purpose !!
   #endif
      case IMAGE_I24:
         return Vec4(Vec((*(U16*)data | (((Byte*)data)[2]<<16))/Flt(0x00FFFFFFu)), 1); // here Dbl is not required, this was tested

      case IMAGE_L8     : return Vec4(Vec(    ByteToFlt   (*(U8*)data)), 1);
      case IMAGE_L8_SRGB: return Vec4(Vec(ByteSRGBToLinear(*(U8*)data)), 1);

      case IMAGE_L8A8     : {VecB2 &c=*(VecB2*)data; Flt l=    ByteToFlt   (c.x); return Vec4(l, l, l, ByteToFlt(c.y));}
      case IMAGE_L8A8_SRGB: {VecB2 &c=*(VecB2*)data; Flt l=ByteSRGBToLinear(c.x); return Vec4(l, l, l, ByteToFlt(c.y));}

      case IMAGE_B8G8R8A8: {VecB4 &c=*(VecB4*)data; return Vec4(ByteToFlt(c.z), ByteToFlt(c.y), ByteToFlt(c.x), ByteToFlt(c.w));}
      case IMAGE_R8G8B8A8: {VecB4 &c=*(VecB4*)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y), ByteToFlt(c.z), ByteToFlt(c.w));}
      case IMAGE_B8G8R8  : {VecB  &c=*(VecB *)data; return Vec4(ByteToFlt(c.z), ByteToFlt(c.y), ByteToFlt(c.x),              1);}
      case IMAGE_R8G8B8  : {VecB  &c=*(VecB *)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y), ByteToFlt(c.z),              1);}
      case IMAGE_R8G8    : {VecB2 &c=*(VecB2*)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y),              0,              1);}
      case IMAGE_R8      : {Byte   c=*(Byte *)data; return Vec4(ByteToFlt(c  ),              0,              0,              1);}

      case IMAGE_B8G8R8A8_SRGB: {VecB4 &c=*(VecB4*)data; return Vec4(ByteSRGBToLinear(c.z), ByteSRGBToLinear(c.y), ByteSRGBToLinear(c.x), ByteToFlt(c.w));}
      case IMAGE_R8G8B8A8_SRGB: {VecB4 &c=*(VecB4*)data; return Vec4(ByteSRGBToLinear(c.x), ByteSRGBToLinear(c.y), ByteSRGBToLinear(c.z), ByteToFlt(c.w));}
      case IMAGE_B8G8R8_SRGB  : {VecB  &c=*(VecB *)data; return Vec4(ByteSRGBToLinear(c.z), ByteSRGBToLinear(c.y), ByteSRGBToLinear(c.x),              1);}
      case IMAGE_R8G8B8_SRGB  : {VecB  &c=*(VecB *)data; return Vec4(ByteSRGBToLinear(c.x), ByteSRGBToLinear(c.y), ByteSRGBToLinear(c.z),              1);}
      
      case IMAGE_R8_SIGN      : {SByte  &c=*(SByte *)data; return Vec4(SByteToSFlt(c  ),                0,                0,                1);}
      case IMAGE_R8G8_SIGN    : {VecSB2 &c=*(VecSB2*)data; return Vec4(SByteToSFlt(c.x), SByteToSFlt(c.y),                0,                1);}
      case IMAGE_R8G8B8A8_SIGN: {VecSB4 &c=*(VecSB4*)data; return Vec4(SByteToSFlt(c.x), SByteToSFlt(c.y), SByteToSFlt(c.z), SByteToSFlt(c.w));}

      case IMAGE_R10G10B10A2: {UInt u=*(UInt*)data; return Vec4(U10ToFlt(u&0x3FF), U10ToFlt((u>>10)&0x3FF), U10ToFlt((u>>20)&0x3FF), U2ToFlt(u>>30));}

      case IMAGE_R11G11B10F: return Vec4(GetR11G11B10F(data), 1);
   }
   return 0;
}
static inline Vec4 GetColorL(CPtr data, C Image &image, Bool _2d, Int x, Int y, Int z=0)
{
   switch(image.hwType())
   {
      case IMAGE_F32  : return Vec4(*(Flt *)data, 0, 0, 1);
      case IMAGE_F32_2: return Vec4(*(Vec2*)data,    0, 1);
      case IMAGE_F32_3: return Vec4(*(Vec *)data,       1);
      case IMAGE_F32_4: return      *(Vec4*)data          ;

      case IMAGE_F32_3_SRGB: return Vec4(SRGBToLinear(*(Vec *)data), 1);
      case IMAGE_F32_4_SRGB: return      SRGBToLinear(*(Vec4*)data)    ;

      case IMAGE_F16  : return Vec4(((Half*)data)[0],                0,                0,                1);
      case IMAGE_F16_2: return Vec4(((Half*)data)[0], ((Half*)data)[1],                0,                1);
      case IMAGE_F16_3: return Vec4(((Half*)data)[0], ((Half*)data)[1], ((Half*)data)[2],                1);
      case IMAGE_F16_4: return Vec4(((Half*)data)[0], ((Half*)data)[1], ((Half*)data)[2], ((Half*)data)[3]);

      case IMAGE_A8: return Vec4(0, 0, 0, ByteToFlt(*(U8*)data));
      case IMAGE_I8: return Vec4(Vec(     ByteToFlt(*(U8*)data)), 1);

      // 16
   #if SUPPORT_DEPTH_TO_COLOR
      case IMAGE_D16: if(GL)return Vec4(Vec((*(U16*)data)/Flt(0x0000FFFFu)*2-1), 1); // !! else fall through no break on purpose !!
   #endif
      case IMAGE_I16:
         return Vec4(Vec((*(U16*)data)/Flt(0x0000FFFFu)), 1);

      // 32
   #if SUPPORT_DEPTH_TO_COLOR
      case IMAGE_D32     :
      case IMAGE_D32S8X24: return Vec4(Vec(GL ? (*(Flt*)data)*2-1 : *(Flt*)data), 1);
    //case IMAGE_D32I    : if(GL)return Vec4(Vec((*(U32*)data)/Dbl(0xFFFFFFFFu)*2-1), 1); // !! else fall through no break on purpose !!
   #endif
      case IMAGE_I32:
         return Vec4(Vec((*(U32*)data)/Dbl(0xFFFFFFFFu)), 1); // Dbl required to get best precision

      // 24
   #if SUPPORT_DEPTH_TO_COLOR
      case IMAGE_D24S8:
      case IMAGE_D24X8: if(GL)return Vec4(Vec((*(U16*)(((Byte*)data)+1) | (((Byte*)data)[3]<<16))/Flt(0x00FFFFFFu)*2-1), 1); // !! else fall through no break on purpose !!
   #endif
      case IMAGE_I24:
         return Vec4(Vec((*(U16*)data | (((Byte*)data)[2]<<16))/Flt(0x00FFFFFFu)), 1); // here Dbl is not required, this was tested

      case IMAGE_L8     : return Vec4(Vec(    ByteToFlt   (*(U8*)data)), 1);
      case IMAGE_L8_SRGB: return Vec4(Vec(ByteSRGBToLinear(*(U8*)data)), 1);

      case IMAGE_L8A8     : {VecB2 &c=*(VecB2*)data; Flt l=    ByteToFlt   (c.x); return Vec4(l, l, l, ByteToFlt(c.y));}
      case IMAGE_L8A8_SRGB: {VecB2 &c=*(VecB2*)data; Flt l=ByteSRGBToLinear(c.x); return Vec4(l, l, l, ByteToFlt(c.y));}

      case IMAGE_B8G8R8A8: {VecB4 &c=*(VecB4*)data; return Vec4(ByteToFlt(c.z), ByteToFlt(c.y), ByteToFlt(c.x), ByteToFlt(c.w));}
      case IMAGE_R8G8B8A8: {VecB4 &c=*(VecB4*)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y), ByteToFlt(c.z), ByteToFlt(c.w));}
      case IMAGE_B8G8R8  : {VecB  &c=*(VecB *)data; return Vec4(ByteToFlt(c.z), ByteToFlt(c.y), ByteToFlt(c.x),              1);}
      case IMAGE_R8G8B8  : {VecB  &c=*(VecB *)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y), ByteToFlt(c.z),              1);}
      case IMAGE_R8G8    : {VecB2 &c=*(VecB2*)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y),              0,              1);}
      case IMAGE_R8      : {Byte   c=*(Byte *)data; return Vec4(ByteToFlt(c  ),              0,              0,              1);}

      case IMAGE_B8G8R8A8_SRGB: {VecB4 &c=*(VecB4*)data; return Vec4(ByteSRGBToLinear(c.z), ByteSRGBToLinear(c.y), ByteSRGBToLinear(c.x), ByteToFlt(c.w));}
      case IMAGE_R8G8B8A8_SRGB: {VecB4 &c=*(VecB4*)data; return Vec4(ByteSRGBToLinear(c.x), ByteSRGBToLinear(c.y), ByteSRGBToLinear(c.z), ByteToFlt(c.w));}
      case IMAGE_B8G8R8_SRGB  : {VecB  &c=*(VecB *)data; return Vec4(ByteSRGBToLinear(c.z), ByteSRGBToLinear(c.y), ByteSRGBToLinear(c.x),              1);}
      case IMAGE_R8G8B8_SRGB  : {VecB  &c=*(VecB *)data; return Vec4(ByteSRGBToLinear(c.x), ByteSRGBToLinear(c.y), ByteSRGBToLinear(c.z),              1);}

      case IMAGE_R8_SIGN      : {SByte  &c=*(SByte *)data; return Vec4(SByteToSFlt(c  ),                0,                0,                1);}
      case IMAGE_R8G8_SIGN    : {VecSB2 &c=*(VecSB2*)data; return Vec4(SByteToSFlt(c.x), SByteToSFlt(c.y),                0,                1);}
      case IMAGE_R8G8B8A8_SIGN: {VecSB4 &c=*(VecSB4*)data; return Vec4(SByteToSFlt(c.x), SByteToSFlt(c.y), SByteToSFlt(c.z), SByteToSFlt(c.w));}

      case IMAGE_R10G10B10A2: {UInt u=*(UInt*)data; return Vec4(U10ToFlt(u&0x3FF), U10ToFlt((u>>10)&0x3FF), U10ToFlt((u>>20)&0x3FF), U2ToFlt(u>>30));}

      case IMAGE_R11G11B10F: return Vec4(GetR11G11B10F(data), 1);

      case IMAGE_BC6: return Vec4(DecompressPixelBC6(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3), 1);

      case IMAGE_BC4_SIGN: {SByte  p=DecompressPixelBC4S(image.data() + (x>>2)* 8 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Vec4(SByteToSFlt(p  ),                0, 0, 1);}
      case IMAGE_BC5_SIGN: {VecSB2 p=DecompressPixelBC5S(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Vec4(SByteToSFlt(p.x), SByteToSFlt(p.y), 0, 1);}

      case IMAGE_ETC2_R_SIGN : {SByte  p=DecompressPixelETC2RS (image.data() + (x>>2)* 8 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Vec4(SByteToSFlt(p  ),                0, 0, 1);}
      case IMAGE_ETC2_RG_SIGN: {VecSB2 p=DecompressPixelETC2RGS(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Vec4(SByteToSFlt(p.x), SByteToSFlt(p.y), 0, 1);}

      case IMAGE_BC1       :
      case IMAGE_BC2       :
      case IMAGE_BC3       :
      case IMAGE_BC4       :
      case IMAGE_BC5       :
      case IMAGE_BC7       :
      case IMAGE_ETC1      :
      case IMAGE_ETC2_R    :
      case IMAGE_ETC2_RG   :
      case IMAGE_ETC2_RGB  :
      case IMAGE_ETC2_RGBA1:
      case IMAGE_ETC2_RGBA :
         return _2d ? DecompressPixel(image, x, y) : DecompressPixel(image, x, y, z);

      case IMAGE_BC1_SRGB       :
      case IMAGE_BC2_SRGB       :
      case IMAGE_BC3_SRGB       :
      case IMAGE_BC7_SRGB       :
      case IMAGE_ETC2_RGB_SRGB  :
      case IMAGE_ETC2_RGBA1_SRGB:
      case IMAGE_ETC2_RGBA_SRGB :
         return SRGBToLinear(_2d ? DecompressPixel(image, x, y) : DecompressPixel(image, x, y, z));
   }
   return 0;
}
Vec4 Image::colorL(Int x, Int y)C
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      return GetColorL(data() + x*bytePP() + y*pitch(), T, true, x, y);
   return 0;
}
Vec4 Image::color3DL(Int x, Int y, Int z)C
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      return GetColorL(data() + x*bytePP() + y*pitch() + z*pitch2(), T, false, x, y, z);
   return 0;
}
/******************************************************************************/
static inline Vec4 GetColorS(CPtr data, C Image &image, Bool _2d, Int x, Int y, Int z=0)
{
   switch(image.hwType())
   {
      case IMAGE_F32  : return Vec4(LinearToSRGB(*(Flt *)data)    ,                             0, 0, 1);
      case IMAGE_F32_2: return Vec4(LinearToSRGB(((Flt *)data)[0]), LinearToSRGB(((Flt*)data)[1]), 0, 1);
      case IMAGE_F32_3: return Vec4(LinearToSRGB(*(Vec *)data)                                      , 1);
      case IMAGE_F32_4: return      LinearToSRGB(*(Vec4*)data);

      case IMAGE_F32_3_SRGB: return Vec4(*(Vec *)data, 1);
      case IMAGE_F32_4_SRGB: return      *(Vec4*)data;

      case IMAGE_F16  : return Vec4(LinearToSRGB(((Half *)data)[0]),                              0, 0, 1);
      case IMAGE_F16_2: return Vec4(LinearToSRGB(((Half *)data)[0]), LinearToSRGB(((Half*)data)[1]), 0, 1);
      case IMAGE_F16_3: return Vec4(LinearToSRGB(*(VecH *)data)                                       , 1);
      case IMAGE_F16_4: return      LinearToSRGB(*(VecH4*)data);

      case IMAGE_A8: return Vec4(0, 0, 0,   ByteToFlt(*(U8*)data));
      case IMAGE_I8: return Vec4(Vec(LinearByteToSRGB(*(U8*)data)), 1);

      case IMAGE_I16: return Vec4(Vec(LinearToSRGB((*(U16*)data)/Flt(0x0000FFFFu))), 1);
      case IMAGE_I32: return Vec4(Vec(LinearToSRGB((*(U32*)data)/Dbl(0xFFFFFFFFu))), 1); // Dbl required to get best precision
      case IMAGE_I24: return Vec4(Vec(LinearToSRGB((*(U16*)data | (((Byte*)data)[2]<<16))/Flt(0x00FFFFFFu))), 1); // here Dbl is not required, this was tested

      case IMAGE_L8     : return Vec4(Vec(LinearByteToSRGB(*(U8*)data)), 1);
      case IMAGE_L8_SRGB: return Vec4(Vec(      ByteToFlt (*(U8*)data)), 1);

      case IMAGE_L8A8     : {VecB2 &c=*(VecB2*)data; Flt l=LinearByteToSRGB(c.x); return Vec4(l, l, l, ByteToFlt(c.y));}
      case IMAGE_L8A8_SRGB: {VecB2 &c=*(VecB2*)data; Flt l=      ByteToFlt (c.x); return Vec4(l, l, l, ByteToFlt(c.y));}

      case IMAGE_B8G8R8A8: {VecB4 &c=*(VecB4*)data; return Vec4(LinearByteToSRGB(c.z), LinearByteToSRGB(c.y), LinearByteToSRGB(c.x), ByteToFlt(c.w));}
      case IMAGE_R8G8B8A8: {VecB4 &c=*(VecB4*)data; return Vec4(LinearByteToSRGB(c.x), LinearByteToSRGB(c.y), LinearByteToSRGB(c.z), ByteToFlt(c.w));}
      case IMAGE_B8G8R8  : {VecB  &c=*(VecB *)data; return Vec4(LinearByteToSRGB(c.z), LinearByteToSRGB(c.y), LinearByteToSRGB(c.x),              1);}
      case IMAGE_R8G8B8  : {VecB  &c=*(VecB *)data; return Vec4(LinearByteToSRGB(c.x), LinearByteToSRGB(c.y), LinearByteToSRGB(c.z),              1);}
      case IMAGE_R8G8    : {VecB2 &c=*(VecB2*)data; return Vec4(LinearByteToSRGB(c.x), LinearByteToSRGB(c.y),                     0,              1);}
      case IMAGE_R8      : {Byte   c=*(Byte *)data; return Vec4(LinearByteToSRGB(c  ),                     0,                     0,              1);}

      case IMAGE_B8G8R8A8_SRGB: {VecB4 &c=*(VecB4*)data; return Vec4(ByteToFlt(c.z), ByteToFlt(c.y), ByteToFlt(c.x), ByteToFlt(c.w));}
      case IMAGE_R8G8B8A8_SRGB: {VecB4 &c=*(VecB4*)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y), ByteToFlt(c.z), ByteToFlt(c.w));}
      case IMAGE_B8G8R8_SRGB  : {VecB  &c=*(VecB *)data; return Vec4(ByteToFlt(c.z), ByteToFlt(c.y), ByteToFlt(c.x),              1);}
      case IMAGE_R8G8B8_SRGB  : {VecB  &c=*(VecB *)data; return Vec4(ByteToFlt(c.x), ByteToFlt(c.y), ByteToFlt(c.z),              1);}

      case IMAGE_R8_SIGN      : {SByte  &c=*(SByte *)data; return Vec4(SignLinearToSRGB(SByteToSFlt(c  )),                                  0,                                  0,                1);}
      case IMAGE_R8G8_SIGN    : {VecSB2 &c=*(VecSB2*)data; return Vec4(SignLinearToSRGB(SByteToSFlt(c.x)), SignLinearToSRGB(SByteToSFlt(c.y)),                                  0,                1);}
      case IMAGE_R8G8B8A8_SIGN: {VecSB4 &c=*(VecSB4*)data; return Vec4(SignLinearToSRGB(SByteToSFlt(c.x)), SignLinearToSRGB(SByteToSFlt(c.y)), SignLinearToSRGB(SByteToSFlt(c.z)), SByteToSFlt(c.w));}

      case IMAGE_R10G10B10A2: {UInt u=*(UInt*)data; return Vec4(LinearToSRGB(U10ToFlt(u&0x3FF)), LinearToSRGB(U10ToFlt((u>>10)&0x3FF)), LinearToSRGB(U10ToFlt((u>>20)&0x3FF)), U2ToFlt(u>>30));}

      case IMAGE_R11G11B10F: return Vec4(LinearToSRGB(GetR11G11B10F(data)), 1);

      case IMAGE_BC6: return Vec4(LinearToSRGB(DecompressPixelBC6(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3)), 1);

      case IMAGE_BC4_SIGN: {SByte  p=DecompressPixelBC4S(image.data() + (x>>2)* 8 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Vec4(SignLinearToSRGB(SByteToSFlt(p  )),                                  0, 0, 1);}
      case IMAGE_BC5_SIGN: {VecSB2 p=DecompressPixelBC5S(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Vec4(SignLinearToSRGB(SByteToSFlt(p.x)), SignLinearToSRGB(SByteToSFlt(p.y)), 0, 1);}

      case IMAGE_ETC2_R_SIGN : {SByte  p=DecompressPixelETC2RS (image.data() + (x>>2)* 8 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Vec4(SignLinearToSRGB(SByteToSFlt(p  )),                                  0, 0, 1);}
      case IMAGE_ETC2_RG_SIGN: {VecSB2 p=DecompressPixelETC2RGS(image.data() + (x>>2)*16 + (y>>2)*image.pitch() + (_2d ? 0 : z*image.pitch2()), x&3, y&3); return Vec4(SignLinearToSRGB(SByteToSFlt(p.x)), SignLinearToSRGB(SByteToSFlt(p.y)), 0, 1);}

      case IMAGE_BC1       :
      case IMAGE_BC2       :
      case IMAGE_BC3       :
      case IMAGE_BC4       :
      case IMAGE_BC5       :
      case IMAGE_BC7       :
      case IMAGE_ETC1      :
      case IMAGE_ETC2_R    :
      case IMAGE_ETC2_RG   :
      case IMAGE_ETC2_RGB  :
      case IMAGE_ETC2_RGBA1:
      case IMAGE_ETC2_RGBA :
         return LinearToSRGB(_2d ? DecompressPixel(image, x, y) : DecompressPixel(image, x, y, z));

      case IMAGE_BC1_SRGB       :
      case IMAGE_BC2_SRGB       :
      case IMAGE_BC3_SRGB       :
      case IMAGE_BC7_SRGB       :
      case IMAGE_ETC2_RGB_SRGB  :
      case IMAGE_ETC2_RGBA1_SRGB:
      case IMAGE_ETC2_RGBA_SRGB :
         return _2d ? DecompressPixel(image, x, y) : DecompressPixel(image, x, y, z);
   }
   return 0;
}
Vec4 Image::colorS(Int x, Int y)C
{
   if(InRange(x, lw()) && InRange(y, lh())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      return GetColorS(data() + x*bytePP() + y*pitch(), T, true, x, y);
   return 0;
}
Vec4 Image::color3DS(Int x, Int y, Int z)C
{
   if(InRange(x, lw()) && InRange(y, lh()) && InRange(z, ld())) // no need to check for "&& data()" because being "InRange(lockSize())" already guarantees 'data' being available
      return GetColorS(data() + x*bytePP() + y*pitch() + z*pitch2(), T, false, x, y, z);
   return 0;
}
/******************************************************************************/
// LINEAR
/******************************************************************************/
Flt Image::pixelFLinear(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[2]; xo[0]=Floor(x); x-=xo[0];
      Int yo[2]; yo[0]=Floor(y); y-=yo[0];
      if(clamp)
      {
         xo[1]=xo[0]+1; if(xo[1]<0)xo[0]=xo[1]=0;else if(xo[0]>=lw())xo[0]=xo[1]=lw()-1;else if(xo[0]<0)xo[0]=0;else if(xo[1]>=lw())xo[1]=lw()-1;
         yo[1]=yo[0]+1; if(yo[1]<0)yo[0]=yo[1]=0;else if(yo[0]>=lh())yo[0]=yo[1]=lh()-1;else if(yo[0]<0)yo[0]=0;else if(yo[1]>=lh())yo[1]=lh()-1;
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh();
      }
      Flt p[2][2]; gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]

      return p[0][0]*(1-x)*(1-y)
            +p[0][1]*(  x)*(1-y)
            +p[1][0]*(1-x)*(  y)
            +p[1][1]*(  x)*(  y);
   }
   return 0;
}
/******************************************************************************/
Flt Image::pixel3DFLinear(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[2]; xo[0]=Floor(x); x-=xo[0];
      Int yo[2]; yo[0]=Floor(y); y-=yo[0];
      Int zo[2]; zo[0]=Floor(z); z-=zo[0];
      if(clamp)
      {
         xo[1]=xo[0]+1; if(xo[1]<0)xo[0]=xo[1]=0;else if(xo[0]>=lw())xo[0]=xo[1]=lw()-1;else if(xo[0]<0)xo[0]=0;else if(xo[1]>=lw())xo[1]=lw()-1;
         yo[1]=yo[0]+1; if(yo[1]<0)yo[0]=yo[1]=0;else if(yo[0]>=lh())yo[0]=yo[1]=lh()-1;else if(yo[0]<0)yo[0]=0;else if(yo[1]>=lh())yo[1]=lh()-1;
         zo[1]=zo[0]+1; if(zo[1]<0)zo[0]=zo[1]=0;else if(zo[0]>=ld())zo[0]=zo[1]=ld()-1;else if(zo[0]<0)zo[0]=0;else if(zo[1]>=ld())zo[1]=ld()-1;
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh();
         zo[0]=Mod(zo[0], ld()); zo[1]=(zo[0]+1)%ld();
      }

      Flt p[2][2][2]; gather(&p[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]

      return p[0][0][0]*(1-x)*(1-y)*(1-z)
            +p[0][0][1]*(  x)*(1-y)*(1-z)
            +p[0][1][0]*(1-x)*(  y)*(1-z)
            +p[0][1][1]*(  x)*(  y)*(1-z)
            +p[1][0][0]*(1-x)*(1-y)*(  z)
            +p[1][0][1]*(  x)*(1-y)*(  z)
            +p[1][1][0]*(1-x)*(  y)*(  z)
            +p[1][1][1]*(  x)*(  y)*(  z);
   }
   return 0;
}
/******************************************************************************/
Flt Image::pixelFNearest(Flt x, Flt y, Bool clamp)C
{
   Int ix=Round(x), iy=Round(y);
   if(clamp)
   {
      Clamp(ix, 0, lw()-1);
      Clamp(iy, 0, lh()-1);
   }else
   {
      ix=Mod(ix, lw());
      iy=Mod(iy, lh());
   }
   return pixelF(ix, iy);
}
Flt Image::pixel3DFNearest(Flt x, Flt y, Flt z, Bool clamp)C
{
   Int ix=Round(x), iy=Round(y), iz=Round(z);
   if(clamp)
   {
      Clamp(ix, 0, lw()-1);
      Clamp(iy, 0, lh()-1);
      Clamp(iz, 0, ld()-1);
   }else
   {
      ix=Mod(ix, lw());
      iy=Mod(iy, lh());
      iz=Mod(iz, ld());
   }
   return pixel3DF(ix, iy, iz);
}
Vec4 Image::colorFNearest(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   Int ix=Round(x), iy=Round(y);
   if(clamp)
   {
      Clamp(ix, 0, lw()-1);
      Clamp(iy, 0, lh()-1);
   }else
   {
      ix=Mod(ix, lw());
      iy=Mod(iy, lh());
   }
   return colorF(ix, iy);
}
Vec4 Image::color3DFNearest(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight)C
{
   Int ix=Round(x), iy=Round(y), iz=Round(z);
   if(clamp)
   {
      Clamp(ix, 0, lw()-1);
      Clamp(iy, 0, lh()-1);
      Clamp(iz, 0, ld()-1);
   }else
   {
      ix=Mod(ix, lw());
      iy=Mod(iy, lh());
      iz=Mod(iz, ld());
   }
   return color3DF(ix, iy, iz);
}
/******************************************************************************/
Vec4 Image::colorFLinear(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int xo[2]; xo[0]=Floor(x); x-=xo[0];
      Int yo[2]; yo[0]=Floor(y); y-=yo[0];
      if(clamp)
      {
         xo[1]=xo[0]+1; if(xo[1]<0)xo[0]=xo[1]=0;else if(xo[0]>=lw())xo[0]=xo[1]=lw()-1;else if(xo[0]<0)xo[0]=0;else if(xo[1]>=lw())xo[1]=lw()-1;
         yo[1]=yo[0]+1; if(yo[1]<0)yo[0]=yo[1]=0;else if(yo[0]>=lh())yo[0]=yo[1]=lh()-1;else if(yo[0]<0)yo[0]=0;else if(yo[1]>=lh())yo[1]=lh()-1;
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh();
      }

      Vec4 c[2][2]; gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      if(alpha_weight)
      {
         Vec  rgb  =0;
         Vec4 color=0;
         Add(color, rgb, c[0][0], (1-x)*(1-y), alpha_weight);
         Add(color, rgb, c[0][1], (  x)*(1-y), alpha_weight);
         Add(color, rgb, c[1][0], (1-x)*(  y), alpha_weight);
         Add(color, rgb, c[1][1], (  x)*(  y), alpha_weight);
         Normalize(color, rgb, alpha_weight, ALPHA_LIMIT_LINEAR);
         return color;
      }else
         return c[0][0]*(1-x)*(1-y)
               +c[0][1]*(  x)*(1-y)
               +c[1][0]*(1-x)*(  y)
               +c[1][1]*(  x)*(  y);
   }
   return 0;
}
Vec4 Image::colorFLinearTTNF32_4(Flt x, Flt y, Bool clamp)C // !! this assumes that image is already locked, exists and is of F32_4 type
{
   Int xo[2]; xo[0]=Floor(x); x-=xo[0];
   Int yo[2]; yo[0]=Floor(y); y-=yo[0];
   if(clamp)
   {
      xo[1]=xo[0]+1; if(xo[1]<0)xo[0]=xo[1]=0;else if(xo[0]>=lw())xo[0]=xo[1]=lw()-1;else if(xo[0]<0)xo[0]=0;else if(xo[1]>=lw())xo[1]=lw()-1;
      yo[1]=yo[0]+1; if(yo[1]<0)yo[0]=yo[1]=0;else if(yo[0]>=lh())yo[0]=yo[1]=lh()-1;else if(yo[0]<0)yo[0]=0;else if(yo[1]>=lh())yo[1]=lh()-1;
   }else
   {
      xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw();
      yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh();
   }

 C Vec4 &c00=pixF4(xo[0], yo[0]),
        &c01=pixF4(xo[1], yo[0]),
        &c10=pixF4(xo[0], yo[1]),
        &c11=pixF4(xo[1], yo[1]);
   Vec4 color=0;
   if(c00.w>0 || c01.w>0 || c10.w>0 || c11.w>0) // we're only interested in interpolation if there will be any alpha
   {
      Vec rgb=0;
      Add(color, rgb, c00, (1-x)*(1-y), true);
      Add(color, rgb, c01, (  x)*(1-y), true);
      Add(color, rgb, c10, (1-x)*(  y), true);
      Add(color, rgb, c11, (  x)*(  y), true);
      Normalize(color, rgb, true, ALPHA_LIMIT_NONE); // this is used only for 'transparentToNeighbor' and there we need no limit
   }
   return color;
}
/******************************************************************************/
Vec4 Image::color3DFLinear(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh() && ld())
   {
      Int xo[2]; xo[0]=Floor(x); x-=xo[0];
      Int yo[2]; yo[0]=Floor(y); y-=yo[0];
      Int zo[2]; zo[0]=Floor(z); z-=zo[0];
      if(clamp)
      {
         xo[1]=xo[0]+1; if(xo[1]<0)xo[0]=xo[1]=0;else if(xo[0]>=lw())xo[0]=xo[1]=lw()-1;else if(xo[0]<0)xo[0]=0;else if(xo[1]>=lw())xo[1]=lw()-1;
         yo[1]=yo[0]+1; if(yo[1]<0)yo[0]=yo[1]=0;else if(yo[0]>=lh())yo[0]=yo[1]=lh()-1;else if(yo[0]<0)yo[0]=0;else if(yo[1]>=lh())yo[1]=lh()-1;
         zo[1]=zo[0]+1; if(zo[1]<0)zo[0]=zo[1]=0;else if(zo[0]>=ld())zo[0]=zo[1]=ld()-1;else if(zo[0]<0)zo[0]=0;else if(zo[1]>=ld())zo[1]=ld()-1;
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh();
         zo[0]=Mod(zo[0], ld()); zo[1]=(zo[0]+1)%ld();
      }

      Vec4 c[2][2][2]; gather(&c[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      if(alpha_weight)
      {
         Vec  rgb  =0;
         Vec4 color=0;
         Add(color, rgb, c[0][0][0], (1-x)*(1-y)*(1-z), alpha_weight);
         Add(color, rgb, c[0][0][1], (  x)*(1-y)*(1-z), alpha_weight);
         Add(color, rgb, c[0][1][0], (1-x)*(  y)*(1-z), alpha_weight);
         Add(color, rgb, c[0][1][1], (  x)*(  y)*(1-z), alpha_weight);
         Add(color, rgb, c[1][0][0], (1-x)*(1-y)*(  z), alpha_weight);
         Add(color, rgb, c[1][0][1], (  x)*(1-y)*(  z), alpha_weight);
         Add(color, rgb, c[1][1][0], (1-x)*(  y)*(  z), alpha_weight);
         Add(color, rgb, c[1][1][1], (  x)*(  y)*(  z), alpha_weight);
         Normalize(color, rgb, alpha_weight, ALPHA_LIMIT_LINEAR);
         return color;
      }else
         return c[0][0][0]*(1-x)*(1-y)*(1-z)
               +c[0][0][1]*(  x)*(1-y)*(1-z)
               +c[0][1][0]*(1-x)*(  y)*(1-z)
               +c[0][1][1]*(  x)*(  y)*(1-z)
               +c[1][0][0]*(1-x)*(1-y)*(  z)
               +c[1][0][1]*(  x)*(1-y)*(  z)
               +c[1][1][0]*(1-x)*(  y)*(  z)
               +c[1][1][1]*(  x)*(  y)*(  z);
   }
   return 0;
}
/******************************************************************************/
// CUBIC FAST
/******************************************************************************/
Flt Image::pixelFCubicFast(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }
      Flt p[4][4]; gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
          y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2),
          v=0, weight=0, w;
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[0][0]*w; weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[0][1]*w; weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[0][2]*w; weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[0][3]*w; weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[1][0]*w; weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[1][1]*w; weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[1][2]*w; weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[1][3]*w; weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[2][0]*w; weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[2][1]*w; weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[2][2]*w; weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[2][3]*w; weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[3][0]*w; weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[3][1]*w; weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[3][2]*w; weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); v+=p[3][3]*w; weight+=w;}
      return v/weight;
   }
   return 0;
}
Flt Image::pixelFCubicFastSmooth(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }
      Flt p[4][4]; gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
          y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2),
          v=0, weight=0, w;
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[0][0]*w; weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[0][1]*w; weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[0][2]*w; weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[0][3]*w; weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[1][0]*w; weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[1][1]*w; weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[1][2]*w; weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[1][3]*w; weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[2][0]*w; weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[2][1]*w; weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[2][2]*w; weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[2][3]*w; weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[3][0]*w; weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[3][1]*w; weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[3][2]*w; weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); v+=p[3][3]*w; weight+=w;}
      return v/weight;
   }
   return 0;
}
Flt Image::pixelFCubicFastSharp(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }
      Flt p[4][4]; gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
          y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2),
          v=0, weight=0, w;
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[0][0]*w; weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[0][1]*w; weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[0][2]*w; weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[0][3]*w; weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[1][0]*w; weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[1][1]*w; weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[1][2]*w; weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[1][3]*w; weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[2][0]*w; weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[2][1]*w; weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[2][2]*w; weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[2][3]*w; weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[3][0]*w; weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[3][1]*w; weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[3][2]*w; weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); v+=p[3][3]*w; weight+=w;}
      return v/weight;
   }
   return 0;
}
/******************************************************************************/
Vec4 Image::colorFCubicFast(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }

      Vec4 c[4][4]; gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Vec  rgb   =0;
      Vec4 color =0;
      Flt  weight=0, w,
           x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
           y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2);
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][0], w, alpha_weight); weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][1], w, alpha_weight); weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][2], w, alpha_weight); weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][3], w, alpha_weight); weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][0], w, alpha_weight); weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][1], w, alpha_weight); weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][2], w, alpha_weight); weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][3], w, alpha_weight); weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][0], w, alpha_weight); weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][1], w, alpha_weight); weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][2], w, alpha_weight); weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][3], w, alpha_weight); weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][0], w, alpha_weight); weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][1], w, alpha_weight); weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][2], w, alpha_weight); weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][3], w, alpha_weight); weight+=w;}
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_FAST);
      return color;
   }
   return 0;
}
Vec4 Image::colorLCubicFast(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }

      Vec4 c[4][4]; gatherL(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Vec  rgb   =0;
      Vec4 color =0;
      Flt  weight=0, w,
           x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
           y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2);
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][0], w, alpha_weight); weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][1], w, alpha_weight); weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][2], w, alpha_weight); weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][3], w, alpha_weight); weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][0], w, alpha_weight); weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][1], w, alpha_weight); weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][2], w, alpha_weight); weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][3], w, alpha_weight); weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][0], w, alpha_weight); weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][1], w, alpha_weight); weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][2], w, alpha_weight); weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][3], w, alpha_weight); weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][0], w, alpha_weight); weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][1], w, alpha_weight); weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][2], w, alpha_weight); weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][3], w, alpha_weight); weight+=w;}
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_FAST);
      return color;
   }
   return 0;
}
Vec4 Image::colorSCubicFast(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }

      Vec4 c[4][4]; gatherS(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Vec  rgb   =0;
      Vec4 color =0;
      Flt  weight=0, w,
           x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
           y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2);
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][0], w, alpha_weight); weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][1], w, alpha_weight); weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][2], w, alpha_weight); weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[0][3], w, alpha_weight); weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][0], w, alpha_weight); weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][1], w, alpha_weight); weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][2], w, alpha_weight); weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[1][3], w, alpha_weight); weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][0], w, alpha_weight); weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][1], w, alpha_weight); weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][2], w, alpha_weight); weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[2][3], w, alpha_weight); weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][0], w, alpha_weight); weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][1], w, alpha_weight); weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][2], w, alpha_weight); weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFast2(w); Add(color, rgb, c[3][3], w, alpha_weight); weight+=w;}
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_FAST);
      return color;
   }
   return 0;
}
/******************************************************************************/
Vec4 Image::colorFCubicFastSmooth(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }

      Vec4 c[4][4]; gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Vec  rgb   =0;
      Vec4 color =0;
      Flt  weight=0, w,
           x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
           y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2);
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[0][0], w, alpha_weight); weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[0][1], w, alpha_weight); weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[0][2], w, alpha_weight); weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[0][3], w, alpha_weight); weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[1][0], w, alpha_weight); weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[1][1], w, alpha_weight); weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[1][2], w, alpha_weight); weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[1][3], w, alpha_weight); weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[2][0], w, alpha_weight); weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[2][1], w, alpha_weight); weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[2][2], w, alpha_weight); weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[2][3], w, alpha_weight); weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[3][0], w, alpha_weight); weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[3][1], w, alpha_weight); weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[3][2], w, alpha_weight); weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSmooth2(w); Add(color, rgb, c[3][3], w, alpha_weight); weight+=w;}
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_FAST_SMOOTH);
      return color;
   }
   return 0;
}
Vec4 Image::colorFCubicFastSharp(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }

      Vec4 c[4][4]; gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Vec  rgb   =0;
      Vec4 color =0;
      Flt  weight=0, w,
           x0w=Sqr(x+1), x1w=Sqr(x), x2w=Sqr(x-1), x3w=Sqr(x-2),
           y0w=Sqr(y+1), y1w=Sqr(y), y2w=Sqr(y-1), y3w=Sqr(y-2);
      w=x0w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[0][0], w, alpha_weight); weight+=w;}
      w=x1w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[0][1], w, alpha_weight); weight+=w;}
      w=x2w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[0][2], w, alpha_weight); weight+=w;}
      w=x3w+y0w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[0][3], w, alpha_weight); weight+=w;}

      w=x0w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[1][0], w, alpha_weight); weight+=w;}
      w=x1w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[1][1], w, alpha_weight); weight+=w;}
      w=x2w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[1][2], w, alpha_weight); weight+=w;}
      w=x3w+y1w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[1][3], w, alpha_weight); weight+=w;}

      w=x0w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[2][0], w, alpha_weight); weight+=w;}
      w=x1w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[2][1], w, alpha_weight); weight+=w;}
      w=x2w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[2][2], w, alpha_weight); weight+=w;}
      w=x3w+y2w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[2][3], w, alpha_weight); weight+=w;}

      w=x0w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[3][0], w, alpha_weight); weight+=w;}
      w=x1w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[3][1], w, alpha_weight); weight+=w;}
      w=x2w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[3][2], w, alpha_weight); weight+=w;}
      w=x3w+y3w; if(w<Sqr(CUBIC_FAST_RANGE)){w=CubicFastSharp2(w); Add(color, rgb, c[3][3], w, alpha_weight); weight+=w;}
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_FAST_SHARP);
      return color;
   }
   return 0;
}
/******************************************************************************/
Flt Image::pixel3DFCubicFast(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], zo[CUBIC_FAST_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[CUBIC_FAST_SAMPLES*2], yw[CUBIC_FAST_SAMPLES*2], zw[CUBIC_FAST_SAMPLES*2];
      Flt p [CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_FAST_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_FAST_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&p[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt weight=0, v=0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_FAST_RANGE))
         {
            w=CubicFast2(w); v+=p[z][y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
Flt Image::pixel3DFCubicFastSmooth(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], zo[CUBIC_FAST_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[CUBIC_FAST_SAMPLES*2], yw[CUBIC_FAST_SAMPLES*2], zw[CUBIC_FAST_SAMPLES*2];
      Flt p [CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_FAST_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_FAST_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&p[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt weight=0, v=0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_FAST_RANGE))
         {
            w=CubicFastSmooth2(w); v+=p[z][y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
Flt Image::pixel3DFCubicFastSharp(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], zo[CUBIC_FAST_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[CUBIC_FAST_SAMPLES*2], yw[CUBIC_FAST_SAMPLES*2], zw[CUBIC_FAST_SAMPLES*2];
      Flt p [CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_FAST_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_FAST_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&p[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt weight=0, v=0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_FAST_RANGE))
         {
            w=CubicFastSharp2(w); v+=p[z][y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
/******************************************************************************/
Flt Image::pixelFCubicPlus(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[CUBIC_PLUS_SAMPLES*2], yo[CUBIC_PLUS_SAMPLES*2], xi=Floor(x), yi=Floor(y);
      Flt xw[CUBIC_PLUS_SAMPLES*2], yw[CUBIC_PLUS_SAMPLES*2];
      Flt p [CUBIC_PLUS_SAMPLES*2][CUBIC_PLUS_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_PLUS_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_PLUS_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
         }
      }
      gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt weight=0, v=0;
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]; if(w<Sqr(CUBIC_PLUS_RANGE))
         {
            w=CubicPlus2(w*Sqr(CUBIC_PLUS_SHARPNESS)); v+=p[y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
Flt Image::pixelFCubicPlusSharp(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[CUBIC_PLUS_SHARP_SAMPLES*2], yo[CUBIC_PLUS_SHARP_SAMPLES*2], xi=Floor(x), yi=Floor(y);
      Flt xw[CUBIC_PLUS_SHARP_SAMPLES*2], yw[CUBIC_PLUS_SHARP_SAMPLES*2];
      Flt p [CUBIC_PLUS_SHARP_SAMPLES*2][CUBIC_PLUS_SHARP_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_PLUS_SHARP_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_PLUS_SHARP_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
         }
      }
      gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt weight=0, v=0;
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]; if(w<Sqr(CUBIC_PLUS_SHARP_RANGE))
         {
            w=CubicPlusSharp2(w*Sqr(CUBIC_PLUS_SHARP_SHARPNESS)); v+=p[y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
Flt Image::pixel3DFCubicPlus(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[CUBIC_PLUS_SAMPLES*2], yo[CUBIC_PLUS_SAMPLES*2], zo[CUBIC_PLUS_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[CUBIC_PLUS_SAMPLES*2], yw[CUBIC_PLUS_SAMPLES*2], zw[CUBIC_PLUS_SAMPLES*2];
      Flt p [CUBIC_PLUS_SAMPLES*2][CUBIC_PLUS_SAMPLES*2][CUBIC_PLUS_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_PLUS_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_PLUS_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_PLUS_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&p[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt weight=0, v=0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_PLUS_RANGE))
         {
            w=CubicPlus2(w*Sqr(CUBIC_PLUS_SHARPNESS)); v+=p[z][y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
Flt Image::pixel3DFCubicPlusSharp(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[CUBIC_PLUS_SHARP_SAMPLES*2], yo[CUBIC_PLUS_SHARP_SAMPLES*2], zo[CUBIC_PLUS_SHARP_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[CUBIC_PLUS_SHARP_SAMPLES*2], yw[CUBIC_PLUS_SHARP_SAMPLES*2], zw[CUBIC_PLUS_SHARP_SAMPLES*2];
      Flt p [CUBIC_PLUS_SHARP_SAMPLES*2][CUBIC_PLUS_SHARP_SAMPLES*2][CUBIC_PLUS_SHARP_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_PLUS_SHARP_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_PLUS_SHARP_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_PLUS_SHARP_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&p[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt weight=0, v=0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_PLUS_SHARP_RANGE))
         {
            w=CubicPlusSharp2(w*Sqr(CUBIC_PLUS_SHARP_SHARPNESS)); v+=p[z][y][x]*w; weight+=w;
         }
      }
      return v/weight;
   }
   return 0;
}
/******************************************************************************/
Vec4 Image::colorFCubicPlus(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int  xo[CUBIC_PLUS_SAMPLES*2], yo[CUBIC_PLUS_SAMPLES*2], xi=Floor(x), yi=Floor(y);
      Flt  xw[CUBIC_PLUS_SAMPLES*2], yw[CUBIC_PLUS_SAMPLES*2];
      Vec4 c [CUBIC_PLUS_SAMPLES*2][CUBIC_PLUS_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_PLUS_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_PLUS_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
         }
      }
      gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]; if(w<Sqr(CUBIC_PLUS_RANGE))
         {
            w=CubicPlus2(w*Sqr(CUBIC_PLUS_SHARPNESS)); Add(color, rgb, c[y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_PLUS);
      return color;
   }
   return 0;
}
Vec4 Image::colorFCubicPlusSharp(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int  xo[CUBIC_PLUS_SHARP_SAMPLES*2], yo[CUBIC_PLUS_SHARP_SAMPLES*2], xi=Floor(x), yi=Floor(y);
      Flt  xw[CUBIC_PLUS_SHARP_SAMPLES*2], yw[CUBIC_PLUS_SHARP_SAMPLES*2];
      Vec4 c [CUBIC_PLUS_SHARP_SAMPLES*2][CUBIC_PLUS_SHARP_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_PLUS_SHARP_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_PLUS_SHARP_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
         }
      }
      gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]; if(w<Sqr(CUBIC_PLUS_SHARP_RANGE))
         {
            w=CubicPlusSharp2(w*Sqr(CUBIC_PLUS_SHARP_SHARPNESS)); Add(color, rgb, c[y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_PLUS_SHARP);
      return color;
   }
   return 0;
}
Vec4 Image::color3DFCubicPlus(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh() && ld())
   {
      Int  xo[CUBIC_PLUS_SAMPLES*2], yo[CUBIC_PLUS_SAMPLES*2], zo[CUBIC_PLUS_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt  xw[CUBIC_PLUS_SAMPLES*2], yw[CUBIC_PLUS_SAMPLES*2], zw[CUBIC_PLUS_SAMPLES*2];
      Vec4 c [CUBIC_PLUS_SAMPLES*2][CUBIC_PLUS_SAMPLES*2][CUBIC_PLUS_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_PLUS_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_PLUS_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_PLUS_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&c[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_PLUS_RANGE))
         {
            w=CubicPlus2(w*Sqr(CUBIC_PLUS_SHARPNESS)); Add(color, rgb, c[z][y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_PLUS);
      return color;
   }
   return 0;
}
Vec4 Image::color3DFCubicPlusSharp(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh() && ld())
   {
      Int  xo[CUBIC_PLUS_SHARP_SAMPLES*2], yo[CUBIC_PLUS_SHARP_SAMPLES*2], zo[CUBIC_PLUS_SHARP_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt  xw[CUBIC_PLUS_SHARP_SAMPLES*2], yw[CUBIC_PLUS_SHARP_SAMPLES*2], zw[CUBIC_PLUS_SHARP_SAMPLES*2];
      Vec4 c [CUBIC_PLUS_SHARP_SAMPLES*2][CUBIC_PLUS_SHARP_SAMPLES*2][CUBIC_PLUS_SHARP_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_PLUS_SHARP_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_PLUS_SHARP_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_PLUS_SHARP_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&c[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_PLUS_SHARP_RANGE))
         {
            w=CubicPlusSharp2(w*Sqr(CUBIC_PLUS_SHARP_SHARPNESS)); Add(color, rgb, c[z][y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_PLUS_SHARP);
      return color;
   }
   return 0;
}
Vec4 Image::color3DFCubicFast(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh() && ld())
   {
      Int  xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], zo[CUBIC_FAST_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt  xw[CUBIC_FAST_SAMPLES*2], yw[CUBIC_FAST_SAMPLES*2], zw[CUBIC_FAST_SAMPLES*2];
      Vec4 c [CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_FAST_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_FAST_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&c[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_FAST_RANGE))
         {
            w=CubicFast2(w); Add(color, rgb, c[z][y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_FAST);
      return color;
   }
   return 0;
}
Vec4 Image::color3DFCubicFastSmooth(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh() && ld())
   {
      Int  xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], zo[CUBIC_FAST_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt  xw[CUBIC_FAST_SAMPLES*2], yw[CUBIC_FAST_SAMPLES*2], zw[CUBIC_FAST_SAMPLES*2];
      Vec4 c [CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_FAST_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_FAST_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&c[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_FAST_RANGE))
         {
            w=CubicFastSmooth2(w); Add(color, rgb, c[z][y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_FAST_SMOOTH);
      return color;
   }
   return 0;
}
Vec4 Image::color3DFCubicFastSharp(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh() && ld())
   {
      Int  xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], zo[CUBIC_FAST_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt  xw[CUBIC_FAST_SAMPLES*2], yw[CUBIC_FAST_SAMPLES*2], zw[CUBIC_FAST_SAMPLES*2];
      Vec4 c [CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-CUBIC_FAST_SAMPLES+1+i; xw[i]=Sqr(x-xo[i]);
         yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(y-yo[i]);
         zo[i]=zi-CUBIC_FAST_SAMPLES+1+i; zw[i]=Sqr(z-zo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zo[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zo[i]=Mod(zo[i], ld());
         }
      }
      gather(&c[0][0][0], xo, Elms(xo), yo, Elms(yo), zo, Elms(zo)); // [z][y][x]
      Flt  weight=0;
      Vec  rgb   =0;
      Vec4 color =0;
      REPAD(z, zo)
      REPAD(y, yo)
      REPAD(x, xo)
      {
         Flt w=xw[x]+yw[y]+zw[z]; if(w<Sqr(CUBIC_FAST_RANGE))
         {
            w=CubicFastSharp2(w); Add(color, rgb, c[z][y][x], w, alpha_weight); weight+=w;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_FAST_SHARP);
      return color;
   }
   return 0;
}
/******************************************************************************/
// CUBIC ORTHO
/******************************************************************************
Flt Image::pixelFCubicOrtho(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }
      Flt p[4][4]; gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
   #if 0
      return Lerp4(
               Lerp4(p[0][0], p[0][1], p[0][2], p[0][3], x),
               Lerp4(p[1][0], p[1][1], p[1][2], p[1][3], x),
               Lerp4(p[2][0], p[2][1], p[2][2], p[2][3], x),
               Lerp4(p[3][0], p[3][1], p[3][2], p[3][3], x), y);
   #else // optimized
      Vec4 xb; Lerp4Weights(xb, x);
      return Lerp4(
               p[0][0]*xb.x + p[0][1]*xb.y + p[0][2]*xb.z + p[0][3]*xb.w,
               p[1][0]*xb.x + p[1][1]*xb.y + p[1][2]*xb.z + p[1][3]*xb.w,
               p[2][0]*xb.x + p[2][1]*xb.y + p[2][2]*xb.z + p[2][3]*xb.w,
               p[3][0]*xb.x + p[3][1]*xb.y + p[3][2]*xb.z + p[3][3]*xb.w, y);
   #endif
   }
   return 0;
}
/******************************************************************************
Flt Image::pixel3DFCubicOrtho(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      Int zo[4]; zo[0]=Floor(z); z-=zo[0]; zo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
         zo[1]=Mid(zo[0]+1, 0, ld()-1); zo[2]=Mid(zo[0]+2, 0, ld()-1); zo[3]=Mid(zo[0]+3, 0, ld()-1); Clamp(zo[0], 0, ld()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
         zo[0]=Mod(zo[0], ld()); zo[1]=(zo[0]+1)%ld(); zo[2]=(zo[0]+2)%ld(); zo[3]=(zo[0]+3)%ld();
      }

      Flt p000=pixel3DF(xo[0], yo[0], zo[0]), p100=pixel3DF(xo[1], yo[0], zo[0]), p200=pixel3DF(xo[2], yo[0], zo[0]), p300=pixel3DF(xo[3], yo[0], zo[0]),
          p010=pixel3DF(xo[0], yo[1], zo[0]), p110=pixel3DF(xo[1], yo[1], zo[0]), p210=pixel3DF(xo[2], yo[1], zo[0]), p310=pixel3DF(xo[3], yo[1], zo[0]),
          p020=pixel3DF(xo[0], yo[2], zo[0]), p120=pixel3DF(xo[1], yo[2], zo[0]), p220=pixel3DF(xo[2], yo[2], zo[0]), p320=pixel3DF(xo[3], yo[2], zo[0]),
          p030=pixel3DF(xo[0], yo[3], zo[0]), p130=pixel3DF(xo[1], yo[3], zo[0]), p230=pixel3DF(xo[2], yo[3], zo[0]), p330=pixel3DF(xo[3], yo[3], zo[0]),

          p001=pixel3DF(xo[0], yo[0], zo[1]), p101=pixel3DF(xo[1], yo[0], zo[1]), p201=pixel3DF(xo[2], yo[0], zo[1]), p301=pixel3DF(xo[3], yo[0], zo[1]),
          p011=pixel3DF(xo[0], yo[1], zo[1]), p111=pixel3DF(xo[1], yo[1], zo[1]), p211=pixel3DF(xo[2], yo[1], zo[1]), p311=pixel3DF(xo[3], yo[1], zo[1]),
          p021=pixel3DF(xo[0], yo[2], zo[1]), p121=pixel3DF(xo[1], yo[2], zo[1]), p221=pixel3DF(xo[2], yo[2], zo[1]), p321=pixel3DF(xo[3], yo[2], zo[1]),
          p031=pixel3DF(xo[0], yo[3], zo[1]), p131=pixel3DF(xo[1], yo[3], zo[1]), p231=pixel3DF(xo[2], yo[3], zo[1]), p331=pixel3DF(xo[3], yo[3], zo[1]),

          p002=pixel3DF(xo[0], yo[0], zo[2]), p102=pixel3DF(xo[1], yo[0], zo[2]), p202=pixel3DF(xo[2], yo[0], zo[2]), p302=pixel3DF(xo[3], yo[0], zo[2]),
          p012=pixel3DF(xo[0], yo[1], zo[2]), p112=pixel3DF(xo[1], yo[1], zo[2]), p212=pixel3DF(xo[2], yo[1], zo[2]), p312=pixel3DF(xo[3], yo[1], zo[2]),
          p022=pixel3DF(xo[0], yo[2], zo[2]), p122=pixel3DF(xo[1], yo[2], zo[2]), p222=pixel3DF(xo[2], yo[2], zo[2]), p322=pixel3DF(xo[3], yo[2], zo[2]),
          p032=pixel3DF(xo[0], yo[3], zo[2]), p132=pixel3DF(xo[1], yo[3], zo[2]), p232=pixel3DF(xo[2], yo[3], zo[2]), p332=pixel3DF(xo[3], yo[3], zo[2]),

          p003=pixel3DF(xo[0], yo[0], zo[3]), p103=pixel3DF(xo[1], yo[0], zo[3]), p203=pixel3DF(xo[2], yo[0], zo[3]), p303=pixel3DF(xo[3], yo[0], zo[3]),
          p013=pixel3DF(xo[0], yo[1], zo[3]), p113=pixel3DF(xo[1], yo[1], zo[3]), p213=pixel3DF(xo[2], yo[1], zo[3]), p313=pixel3DF(xo[3], yo[1], zo[3]),
          p023=pixel3DF(xo[0], yo[2], zo[3]), p123=pixel3DF(xo[1], yo[2], zo[3]), p223=pixel3DF(xo[2], yo[2], zo[3]), p323=pixel3DF(xo[3], yo[2], zo[3]),
          p033=pixel3DF(xo[0], yo[3], zo[3]), p133=pixel3DF(xo[1], yo[3], zo[3]), p233=pixel3DF(xo[2], yo[3], zo[3]), p333=pixel3DF(xo[3], yo[3], zo[3]);

   #if 0
      return Lerp4(
                Lerp4(
                  Lerp4(p000, p100, p200, p300, x),
                  Lerp4(p010, p110, p210, p310, x),
                  Lerp4(p020, p120, p220, p320, x),
                  Lerp4(p030, p130, p230, p330, x), y),

                Lerp4(
                  Lerp4(p001, p101, p201, p301, x),
                  Lerp4(p011, p111, p211, p311, x),
                  Lerp4(p021, p121, p221, p321, x),
                  Lerp4(p031, p131, p231, p331, x), y),

                Lerp4(
                  Lerp4(p002, p102, p202, p302, x),
                  Lerp4(p012, p112, p212, p312, x),
                  Lerp4(p022, p122, p222, p322, x),
                  Lerp4(p032, p132, p232, p332, x), y),

                Lerp4(
                  Lerp4(p003, p103, p203, p303, x),
                  Lerp4(p013, p113, p213, p313, x),
                  Lerp4(p023, p123, p223, p323, x),
                  Lerp4(p033, p133, p233, p333, x), y), z);
   #else // optimized
      Vec4 xb, yb; Lerp4Weights(xb, x);
                   Lerp4Weights(yb, y);
      return Lerp4(
                (p000*xb.x + p100*xb.y + p200*xb.z + p300*xb.w)*yb.x
               +(p010*xb.x + p110*xb.y + p210*xb.z + p310*xb.w)*yb.y
               +(p020*xb.x + p120*xb.y + p220*xb.z + p320*xb.w)*yb.z
               +(p030*xb.x + p130*xb.y + p230*xb.z + p330*xb.w)*yb.w,

                (p001*xb.x + p101*xb.y + p201*xb.z + p301*xb.w)*yb.x
               +(p011*xb.x + p111*xb.y + p211*xb.z + p311*xb.w)*yb.y
               +(p021*xb.x + p121*xb.y + p221*xb.z + p321*xb.w)*yb.z
               +(p031*xb.x + p131*xb.y + p231*xb.z + p331*xb.w)*yb.w,

                (p002*xb.x + p102*xb.y + p202*xb.z + p302*xb.w)*yb.x
               +(p012*xb.x + p112*xb.y + p212*xb.z + p312*xb.w)*yb.y
               +(p022*xb.x + p122*xb.y + p222*xb.z + p322*xb.w)*yb.z
               +(p032*xb.x + p132*xb.y + p232*xb.z + p332*xb.w)*yb.w,

                (p003*xb.x + p103*xb.y + p203*xb.z + p303*xb.w)*yb.x
               +(p013*xb.x + p113*xb.y + p213*xb.z + p313*xb.w)*yb.y
               +(p023*xb.x + p123*xb.y + p223*xb.z + p323*xb.w)*yb.z
               +(p033*xb.x + p133*xb.y + p233*xb.z + p333*xb.w)*yb.w, z);
   #endif
   }
   return 0;
}
/******************************************************************************
Flt Image::pixelFLanczosOrtho(Flt x, Flt y, Bool clamp)C
{
   if(lw() && lh())
   {
      Int xo[LANCZOS_SAMPLES*2], yo[LANCZOS_SAMPLES*2], xi=Floor(x), yi=Floor(y);
      Flt xw[LANCZOS_SAMPLES*2], yw[LANCZOS_SAMPLES*2], xs=0, ys=0;
      Flt p [LANCZOS_SAMPLES*2][LANCZOS_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-LANCZOS_SAMPLES+1+i; xw[i]=LanczosSharp(x-xo[i]); xs+=xw[i];
         yo[i]=yi-LANCZOS_SAMPLES+1+i; yw[i]=LanczosSharp(y-yo[i]); ys+=yw[i];
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
         }
      }
      gather(&p[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Flt fy=0; REPAD(y, yo)
      {
         Flt fx=0; REPAD(x, xo)fx+=p[y][x]*xw[x];
         fy+=fx*yw[y];
      }
      return fy/(xs*ys);
   }
   return 0;
}
/******************************************************************************
Vec4 Image::colorFCubicOrtho(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
      }

      Vec4 c[4][4]; gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Vec  rgb  =0;
      Vec4 color=0;
      Vec4 xb; Lerp4Weights(xb, x);
      Vec4 yb; Lerp4Weights(yb, y);
      Add(color, rgb, c[0][0], xb.x*yb.x, alpha_weight);
      Add(color, rgb, c[0][1], xb.y*yb.x, alpha_weight);
      Add(color, rgb, c[0][2], xb.z*yb.x, alpha_weight);
      Add(color, rgb, c[0][3], xb.w*yb.x, alpha_weight);

      Add(color, rgb, c[1][0], xb.x*yb.y, alpha_weight);
      Add(color, rgb, c[1][1], xb.y*yb.y, alpha_weight);
      Add(color, rgb, c[1][2], xb.z*yb.y, alpha_weight);
      Add(color, rgb, c[1][3], xb.w*yb.y, alpha_weight);

      Add(color, rgb, c[2][0], xb.x*yb.z, alpha_weight);
      Add(color, rgb, c[2][1], xb.y*yb.z, alpha_weight);
      Add(color, rgb, c[2][2], xb.z*yb.z, alpha_weight);
      Add(color, rgb, c[2][3], xb.w*yb.z, alpha_weight);

      Add(color, rgb, c[3][0], xb.x*yb.w, alpha_weight);
      Add(color, rgb, c[3][1], xb.y*yb.w, alpha_weight);
      Add(color, rgb, c[3][2], xb.z*yb.w, alpha_weight);
      Add(color, rgb, c[3][3], xb.w*yb.w, alpha_weight);
      Normalize(color, rgb, alpha_weight, ALPHA_LIMIT);
      return color;
   }
   return 0;
}
/******************************************************************************
Vec4 Image::color3DFCubicOrtho(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[4]; xo[0]=Floor(x); x-=xo[0]; xo[0]--;
      Int yo[4]; yo[0]=Floor(y); y-=yo[0]; yo[0]--;
      Int zo[4]; zo[0]=Floor(z); z-=zo[0]; zo[0]--;
      if(clamp)
      {
         xo[1]=Mid(xo[0]+1, 0, lw()-1); xo[2]=Mid(xo[0]+2, 0, lw()-1); xo[3]=Mid(xo[0]+3, 0, lw()-1); Clamp(xo[0], 0, lw()-1);
         yo[1]=Mid(yo[0]+1, 0, lh()-1); yo[2]=Mid(yo[0]+2, 0, lh()-1); yo[3]=Mid(yo[0]+3, 0, lh()-1); Clamp(yo[0], 0, lh()-1);
         zo[1]=Mid(zo[0]+1, 0, ld()-1); zo[2]=Mid(zo[0]+2, 0, ld()-1); zo[3]=Mid(zo[0]+3, 0, ld()-1); Clamp(zo[0], 0, ld()-1);
      }else
      {
         xo[0]=Mod(xo[0], lw()); xo[1]=(xo[0]+1)%lw(); xo[2]=(xo[0]+2)%lw(); xo[3]=(xo[0]+3)%lw();
         yo[0]=Mod(yo[0], lh()); yo[1]=(yo[0]+1)%lh(); yo[2]=(yo[0]+2)%lh(); yo[3]=(yo[0]+3)%lh();
         zo[0]=Mod(zo[0], ld()); zo[1]=(zo[0]+1)%ld(); zo[2]=(zo[0]+2)%ld(); zo[3]=(zo[0]+3)%ld();
      }

      Vec4 c000=color3DF(xo[0], yo[0], zo[0]), c100=color3DF(xo[1], yo[0], zo[0]), c200=color3DF(xo[2], yo[0], zo[0]), c300=color3DF(xo[3], yo[0], zo[0]),
           c010=color3DF(xo[0], yo[1], zo[0]), c110=color3DF(xo[1], yo[1], zo[0]), c210=color3DF(xo[2], yo[1], zo[0]), c310=color3DF(xo[3], yo[1], zo[0]),
           c020=color3DF(xo[0], yo[2], zo[0]), c120=color3DF(xo[1], yo[2], zo[0]), c220=color3DF(xo[2], yo[2], zo[0]), c320=color3DF(xo[3], yo[2], zo[0]),
           c030=color3DF(xo[0], yo[3], zo[0]), c130=color3DF(xo[1], yo[3], zo[0]), c230=color3DF(xo[2], yo[3], zo[0]), c330=color3DF(xo[3], yo[3], zo[0]),

           c001=color3DF(xo[0], yo[0], zo[1]), c101=color3DF(xo[1], yo[0], zo[1]), c201=color3DF(xo[2], yo[0], zo[1]), c301=color3DF(xo[3], yo[0], zo[1]),
           c011=color3DF(xo[0], yo[1], zo[1]), c111=color3DF(xo[1], yo[1], zo[1]), c211=color3DF(xo[2], yo[1], zo[1]), c311=color3DF(xo[3], yo[1], zo[1]),
           c021=color3DF(xo[0], yo[2], zo[1]), c121=color3DF(xo[1], yo[2], zo[1]), c221=color3DF(xo[2], yo[2], zo[1]), c321=color3DF(xo[3], yo[2], zo[1]),
           c031=color3DF(xo[0], yo[3], zo[1]), c131=color3DF(xo[1], yo[3], zo[1]), c231=color3DF(xo[2], yo[3], zo[1]), c331=color3DF(xo[3], yo[3], zo[1]),

           c002=color3DF(xo[0], yo[0], zo[2]), c102=color3DF(xo[1], yo[0], zo[2]), c202=color3DF(xo[2], yo[0], zo[2]), c302=color3DF(xo[3], yo[0], zo[2]),
           c012=color3DF(xo[0], yo[1], zo[2]), c112=color3DF(xo[1], yo[1], zo[2]), c212=color3DF(xo[2], yo[1], zo[2]), c312=color3DF(xo[3], yo[1], zo[2]),
           c022=color3DF(xo[0], yo[2], zo[2]), c122=color3DF(xo[1], yo[2], zo[2]), c222=color3DF(xo[2], yo[2], zo[2]), c322=color3DF(xo[3], yo[2], zo[2]),
           c032=color3DF(xo[0], yo[3], zo[2]), c132=color3DF(xo[1], yo[3], zo[2]), c232=color3DF(xo[2], yo[3], zo[2]), c332=color3DF(xo[3], yo[3], zo[2]),

           c003=color3DF(xo[0], yo[0], zo[3]), c103=color3DF(xo[1], yo[0], zo[3]), c203=color3DF(xo[2], yo[0], zo[3]), c303=color3DF(xo[3], yo[0], zo[3]),
           c013=color3DF(xo[0], yo[1], zo[3]), c113=color3DF(xo[1], yo[1], zo[3]), c213=color3DF(xo[2], yo[1], zo[3]), c313=color3DF(xo[3], yo[1], zo[3]),
           c023=color3DF(xo[0], yo[2], zo[3]), c123=color3DF(xo[1], yo[2], zo[3]), c223=color3DF(xo[2], yo[2], zo[3]), c323=color3DF(xo[3], yo[2], zo[3]),
           c033=color3DF(xo[0], yo[3], zo[3]), c133=color3DF(xo[1], yo[3], zo[3]), c233=color3DF(xo[2], yo[3], zo[3]), c333=color3DF(xo[3], yo[3], zo[3]);

   #if 0
      return Lerp4(
                Lerp4(
                  Lerp4(c000, c100, c200, c300, x),
                  Lerp4(c010, c110, c210, c310, x),
                  Lerp4(c020, c120, c220, c320, x),
                  Lerp4(c030, c130, c230, c330, x), y),

                Lerp4(
                  Lerp4(c001, c101, c201, c301, x),
                  Lerp4(c011, c111, c211, c311, x),
                  Lerp4(c021, c121, c221, c321, x),
                  Lerp4(c031, c131, c231, c331, x), y),

                Lerp4(
                  Lerp4(c002, c102, c202, c302, x),
                  Lerp4(c012, c112, c212, c312, x),
                  Lerp4(c022, c122, c222, c322, x),
                  Lerp4(c032, c132, c232, c332, x), y),

                Lerp4(
                  Lerp4(c003, c103, c203, c303, x),
                  Lerp4(c013, c113, c213, c313, x),
                  Lerp4(c023, c123, c223, c323, x),
                  Lerp4(c033, c133, c233, c333, x), y), z);
   #else // optimized
      Vec4 xb, yb; Lerp4Weights(xb, x);
                   Lerp4Weights(yb, y);
      return Lerp4(
                (c000*xb.x + c100*xb.y + c200*xb.z + c300*xb.w)*yb.x
               +(c010*xb.x + c110*xb.y + c210*xb.z + c310*xb.w)*yb.y
               +(c020*xb.x + c120*xb.y + c220*xb.z + c320*xb.w)*yb.z
               +(c030*xb.x + c130*xb.y + c230*xb.z + c330*xb.w)*yb.w,

                (c001*xb.x + c101*xb.y + c201*xb.z + c301*xb.w)*yb.x
               +(c011*xb.x + c111*xb.y + c211*xb.z + c311*xb.w)*yb.y
               +(c021*xb.x + c121*xb.y + c221*xb.z + c321*xb.w)*yb.z
               +(c031*xb.x + c131*xb.y + c231*xb.z + c331*xb.w)*yb.w,

                (c002*xb.x + c102*xb.y + c202*xb.z + c302*xb.w)*yb.x
               +(c012*xb.x + c112*xb.y + c212*xb.z + c312*xb.w)*yb.y
               +(c022*xb.x + c122*xb.y + c222*xb.z + c322*xb.w)*yb.z
               +(c032*xb.x + c132*xb.y + c232*xb.z + c332*xb.w)*yb.w,

                (c003*xb.x + c103*xb.y + c203*xb.z + c303*xb.w)*yb.x
               +(c013*xb.x + c113*xb.y + c213*xb.z + c313*xb.w)*yb.y
               +(c023*xb.x + c123*xb.y + c223*xb.z + c323*xb.w)*yb.z
               +(c033*xb.x + c133*xb.y + c233*xb.z + c333*xb.w)*yb.w, z);
   #endif
   }
   return 0;
}
/******************************************************************************
Vec4 Image::colorFLanczosOrtho(Flt x, Flt y, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Int  xo[LANCZOS_SAMPLES*2], yo[LANCZOS_SAMPLES*2], xi=Floor(x), yi=Floor(y);
      Flt  xw[LANCZOS_SAMPLES*2], yw[LANCZOS_SAMPLES*2], xs=0, ys=0;
      Vec4 c [LANCZOS_SAMPLES*2][LANCZOS_SAMPLES*2];
      REPA(xo)
      {
         xo[i]=xi-LANCZOS_SAMPLES+1+i; xw[i]=LanczosSharp(x-xo[i]); xs+=xw[i];
         yo[i]=yi-LANCZOS_SAMPLES+1+i; yw[i]=LanczosSharp(y-yo[i]); ys+=yw[i];
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
         }
      }
      gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
      Vec  rgb  =0;
      Vec4 color=0;
      REPAD(y, yo)
      REPAD(x, xo)Add(color, rgb, c[y][x], xw[x]*yw[y], alpha_weight);
      Normalize(color, rgb, alpha_weight, ALPHA_LIMIT);
      return color/(xs*ys);
   }
   return 0;
}
/******************************************************************************
Flt Image::pixel3DFLanczosOrtho(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[LANCZOS_SAMPLES*2], yo[LANCZOS_SAMPLES*2], zc[LANCZOS_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[LANCZOS_SAMPLES*2], yw[LANCZOS_SAMPLES*2], zw[LANCZOS_SAMPLES*2], xs=0, ys=0, zs=0;
      REPA(xo)
      {
         xo[i]=xi-LANCZOS_SAMPLES+1+i; xw[i]=LanczosSharp(x-xo[i]); xs+=xw[i];
         yo[i]=yi-LANCZOS_SAMPLES+1+i; yw[i]=LanczosSharp(y-yo[i]); ys+=yw[i];
         zc[i]=zi-LANCZOS_SAMPLES+1+i; zw[i]=LanczosSharp(z-zc[i]); zs+=zw[i];
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zc[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zc[i]=Mod(zc[i], ld());
         }
      }
      Flt fz=0; REPAD(z, zc)
      {
         Flt fy=0; REPAD(y, yo)
         {
            Flt fx=0; REPAD(x, xo)fx+=pixel3DF(xo[x], yo[y], zc[z])*xw[x];
            fy+=fx*yw[y];
         }
         fz+=fy*zw[z];
      }
      return fz/(xs*ys*zs);
   }
   return 0;
}
/******************************************************************************
Vec4 Image::color3DFLanczosOrtho(Flt x, Flt y, Flt z, Bool clamp)C
{
   if(lw() && lh() && ld())
   {
      Int xo[LANCZOS_SAMPLES*2], yo[LANCZOS_SAMPLES*2], zc[LANCZOS_SAMPLES*2], xi=Floor(x), yi=Floor(y), zi=Floor(z);
      Flt xw[LANCZOS_SAMPLES*2], yw[LANCZOS_SAMPLES*2], zw[LANCZOS_SAMPLES*2], xs=0, ys=0, zs=0;
      REPA(xo)
      {
         xo[i]=xi-LANCZOS_SAMPLES+1+i; xw[i]=LanczosSharp(x-xo[i]); xs+=xw[i];
         yo[i]=yi-LANCZOS_SAMPLES+1+i; yw[i]=LanczosSharp(y-yo[i]); ys+=yw[i];
         zc[i]=zi-LANCZOS_SAMPLES+1+i; zw[i]=LanczosSharp(z-zc[i]); zs+=zw[i];
         if(clamp)
         {
            Clamp(xo[i], 0, lw()-1);
            Clamp(yo[i], 0, lh()-1);
            Clamp(zc[i], 0, ld()-1);
         }else
         {
            xo[i]=Mod(xo[i], lw());
            yo[i]=Mod(yo[i], lh());
            zc[i]=Mod(zc[i], ld());
         }
      }
      Vec4 cz=0; REPAD(z, zc)
      {
         Vec4 cy=0; REPAD(y, yo)
         {
            Vec4 cx=0; REPAD(x, xo)cx+=color3DF(xo[x], yo[y], zc[z])*xw[x];
            cy+=cx*yw[y];
         }
         cz+=cy*zw[z];
      }
      return cz/(xs*ys*zs);
   }
   return 0;
}
/******************************************************************************/
// AREA
/******************************************************************************/
Vec4 Image::areaColorFAverage(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      Rect_C rect (pos, Max(size.x-1, 0), Max(size.y-1, 0)); // 'size' here means scale, subtract 1 to convert to inclusive coordinates
      RectI  recti(Floor(rect.min.x), Floor(rect.min.y), Ceil(rect.max.x), Ceil(rect.max.y)); // inclusive coordinates. Have to use Ceil for max, because we need to process the neighbor pixel too (for example if rect.min.x=0.5 and rect.max.x=0.5, then we need to process both x=0 and x=1 pixels)
      Int    xo[2], yo[2];

      if(clamp)
      {
         xo[0]=Mid(recti.min.x, 0, lw()-1); yo[0]=Mid(recti.min.y, 0, lh()-1);
         xo[1]=Mid(recti.max.x, 0, lw()-1); yo[1]=Mid(recti.max.y, 0, lh()-1);
      }else
      {
         xo[0]=Mod(recti.min.x, lw()); yo[0]=Mod(recti.min.y, lh());
         xo[1]=Mod(recti.max.x, lw()); yo[1]=Mod(recti.max.y, lh());
      }

      if(recti.min==recti.max)return colorF(xo[0], yo[0]); // if coordinates are the same, then return this pixel

      // calculate blending factors
      Flt l=1+recti.min.x-rect.min.x, r=1+rect.max.x-recti.max.x, // l=1-(rect.min.x-recti.min.x), r=1-(recti.max.x-rect.max.x)
          t=1+recti.min.y-rect.min.y, b=1+rect.max.y-recti.max.y; // t=1-(rect.min.y-recti.min.y), b=1-(recti.max.y-rect.max.y)

      Vec  rgb  =0;
      Vec4 color=0;

      // add inside
      for(Int                                                y=recti.min.y+1; y<recti.max.y; y++)
      for(Int yo=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh())), x=recti.min.x+1; x<recti.max.x; x++)Add(color, rgb, colorF(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()), yo), alpha_weight);

      // add sides
      if(recti.min.y==recti.max.y)
      for(Int x=recti.min.x+1; x<recti.max.x; x++)
      {
         Int xo=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()));
         Add(color, rgb, colorF(xo, yo[0]), alpha_weight);
      }else
      for(Int x=recti.min.x+1; x<recti.max.x; x++)
      {
         Int  xo=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()));
         Vec4 c[2]; gatherL(c, &xo, 1, yo, Elms(yo));
         Add(color, rgb, c[0], t, alpha_weight); // top
         Add(color, rgb, c[1], b, alpha_weight); // bottom
      }

      if(recti.min.x==recti.max.x)
      for(Int y=recti.min.y+1; y<recti.max.y; y++)
      {
         Int yo=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         Add(color, rgb, colorF(xo[0], yo), alpha_weight);
      }else
      for(Int y=recti.min.y+1; y<recti.max.y; y++)
      {
         Int  yo=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         Vec4 c[2]; gatherL(c, xo, Elms(xo), &yo, 1);
         Add(color, rgb, c[0], l, alpha_weight); // left
         Add(color, rgb, c[1], r, alpha_weight); // right
      }

      // add corners
      if(recti.min.y==recti.max.y)
      {
         Vec4 c[2]; gatherL(&c[0], xo, Elms(xo), yo, 1);
         Add(color, rgb, c[0], l, alpha_weight);
         Add(color, rgb, c[1], r, alpha_weight);
      }else
      if(recti.min.x==recti.max.x)
      {
         Vec4 c[2]; gatherL(&c[0], xo, 1, yo, Elms(yo));
         Add(color, rgb, c[0], t, alpha_weight);
         Add(color, rgb, c[1], b, alpha_weight);
      }else
      {
         Vec4 c[2][2]; gatherL(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
         Add(color, rgb, c[0][0], l*t, alpha_weight);
         Add(color, rgb, c[0][1], r*t, alpha_weight);
         Add(color, rgb, c[1][0], l*b, alpha_weight);
         Add(color, rgb, c[1][1], r*b, alpha_weight);
      }

      Normalize(color, rgb, (rect.w()+1)*(rect.h()+1), alpha_weight, ALPHA_LIMIT_LINEAR); // weight is always non-zero here
      return color;
   }
   return 0;
}
/******************************************************************************/
Vec4 Image::areaColorFLinear(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C // this is orthogonal
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 s(Max(1, size.x*0.75f), Max(1, size.y*0.75f)); // 0.5 is too sharp, 1.0 is too blurry, 0.75 is best and gives same results as Avg(a,b)
      Vec2 x_mul_add; x_mul_add.x=1/s.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=1/s.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-s.x), x1=FloorSpecial(pos.x+s.x),
          y0=CeilSpecial(pos.y-s.y), y1=FloorSpecial(pos.y+s.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy=y*y_mul_add.x + y_mul_add.y; fy=Linear(fy); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx=x*x_mul_add.x + x_mul_add.y; fx=Linear(fx); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()));
            fx*=fy;
            Add(color, rgb, colorF(xi, yi), fx, alpha_weight); weight+=fx;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_LINEAR);
      return color;
   }
   return 0;
}
Vec4 Image::areaColorLLinear(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C // this is orthogonal
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 s(Max(1, size.x*0.75f), Max(1, size.y*0.75f)); // 0.5 is too sharp, 1.0 is too blurry, 0.75 is best and gives same results as Avg(a,b)
      Vec2 x_mul_add; x_mul_add.x=1/s.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=1/s.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-s.x), x1=FloorSpecial(pos.x+s.x),
          y0=CeilSpecial(pos.y-s.y), y1=FloorSpecial(pos.y+s.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy=y*y_mul_add.x + y_mul_add.y; fy=Linear(fy); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx=x*x_mul_add.x + x_mul_add.y; fx=Linear(fx); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()));
            fx*=fy;
            Add(color, rgb, colorL(xi, yi), fx, alpha_weight); weight+=fx;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_LINEAR);
      return color;
   }
   return 0;
}
/******************************************************************************/
Vec4 Image::areaColorFCubicFast(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(CUBIC_FAST_RANGE, size.x*CUBIC_FAST_RANGE), Max(CUBIC_FAST_RANGE, size.y*CUBIC_FAST_RANGE));
      Vec2 x_mul_add; x_mul_add.x=CUBIC_FAST_RANGE/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=CUBIC_FAST_RANGE/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<Sqr(CUBIC_FAST_RANGE))
            {
               w=CubicFast2(w); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorF(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_FAST);
      return color;
   }
   return 0;
}
Vec4 Image::areaColorLCubicFast(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(CUBIC_FAST_RANGE, size.x*CUBIC_FAST_RANGE), Max(CUBIC_FAST_RANGE, size.y*CUBIC_FAST_RANGE));
      Vec2 x_mul_add; x_mul_add.x=CUBIC_FAST_RANGE/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=CUBIC_FAST_RANGE/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<Sqr(CUBIC_FAST_RANGE))
            {
               w=CubicFast2(w); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorL(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_FAST);
      return color;
   }
   return 0;
}
/******************************************************************************/
Vec4 Image::areaColorFCubicFastSmooth(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(CUBIC_FAST_RANGE, size.x*CUBIC_FAST_RANGE), Max(CUBIC_FAST_RANGE, size.y*CUBIC_FAST_RANGE));
      Vec2 x_mul_add; x_mul_add.x=CUBIC_FAST_RANGE/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=CUBIC_FAST_RANGE/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<Sqr(CUBIC_FAST_RANGE))
            {
               w=CubicFastSmooth2(w); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorF(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_FAST_SMOOTH);
      return color;
   }
   return 0;
}
Vec4 Image::areaColorLCubicFastSmooth(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(CUBIC_FAST_RANGE, size.x*CUBIC_FAST_RANGE), Max(CUBIC_FAST_RANGE, size.y*CUBIC_FAST_RANGE));
      Vec2 x_mul_add; x_mul_add.x=CUBIC_FAST_RANGE/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=CUBIC_FAST_RANGE/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<Sqr(CUBIC_FAST_RANGE))
            {
               w=CubicFastSmooth2(w); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorL(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_FAST_SMOOTH);
      return color;
   }
   return 0;
}
/******************************************************************************/
Vec4 Image::areaColorFCubicFastSharp(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(CUBIC_FAST_RANGE, size.x*CUBIC_FAST_RANGE), Max(CUBIC_FAST_RANGE, size.y*CUBIC_FAST_RANGE));
      Vec2 x_mul_add; x_mul_add.x=CUBIC_FAST_RANGE/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=CUBIC_FAST_RANGE/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<Sqr(CUBIC_FAST_RANGE))
            {
               w=CubicFastSharp2(w); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorF(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_FAST_SHARP);
      return color;
   }
   return 0;
}
Vec4 Image::areaColorFCubicPlus(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(CUBIC_PLUS_RANGE, size.x*CUBIC_PLUS_RANGE), Max(CUBIC_PLUS_RANGE, size.y*CUBIC_PLUS_RANGE));
      Vec2 x_mul_add; x_mul_add.x=CUBIC_PLUS_RANGE/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=CUBIC_PLUS_RANGE/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<Sqr(CUBIC_PLUS_RANGE))
            {
               w=CubicPlus2(w*Sqr(CUBIC_PLUS_SHARPNESS)); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorF(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_PLUS);
      return color;
   }
   return 0;
}
Vec4 Image::areaColorFCubicPlusSharp(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(CUBIC_PLUS_SHARP_RANGE, size.x*CUBIC_PLUS_SHARP_RANGE), Max(CUBIC_PLUS_SHARP_RANGE, size.y*CUBIC_PLUS_SHARP_RANGE));
      Vec2 x_mul_add; x_mul_add.x=CUBIC_PLUS_SHARP_RANGE/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=CUBIC_PLUS_SHARP_RANGE/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<Sqr(CUBIC_PLUS_SHARP_RANGE))
            {
               w=CubicPlusSharp2(w*Sqr(CUBIC_PLUS_SHARP_SHARPNESS)); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorF(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT_CUBIC_PLUS_SHARP);
      return color;
   }
   return 0;
}
/******************************************************************************
Vec4 Image::areaColorCubicOrtho(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_2(Max(2, size.x*2), Max(2, size.y*2));
      Vec2 x_mul_add; x_mul_add.x=2/size_2.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=2/size_2.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_2.x), x1=FloorSpecial(pos.x+size_2.x),
          y0=CeilSpecial(pos.y-size_2.y), y1=FloorSpecial(pos.y+size_2.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy2=Sqr(y*y_mul_add.x + y_mul_add.y); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx2=Sqr(x*x_mul_add.x + x_mul_add.y), w=fx2+fy2;
            if(w<4) // Cubic returns 0 for values >=2, since we use Sqr, check for 4
            {
               w=CubicSmoothFast2(w); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw())); Add(color, rgb, colorF(xi, yi), w, alpha_weight); weight+=w;
            }
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT);
      return color;
   }
   return 0;
}
Vec4 Image::areaColorCubicOrtho(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_2(Max(2, size.x*2), Max(2, size.y*2));
      Vec2 x_mul_add; x_mul_add.x=2/size_2.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=2/size_2.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_2.x), x1=FloorSpecial(pos.x+size_2.x),
          y0=CeilSpecial(pos.y-size_2.y), y1=FloorSpecial(pos.y+size_2.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy=y*y_mul_add.x + y_mul_add.y; fy=CubicFastSharp(fy); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx=x*x_mul_add.x + x_mul_add.y; fx=CubicFastSharp(fx); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()));
            fx*=fy;
            Add(color, rgb, colorF(xi, yi), fx, alpha_weight); weight+=fx;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT);
      return color;
   }
   return 0;
}
/******************************************************************************
Vec4 Image::areaColorLanczosOrtho(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight)C
{
   if(lw() && lh())
   {
      // f=(p-center)/size
      const Vec2 size_a(Max(LANCZOS_SAMPLES, size.x*LANCZOS_SAMPLES), Max(LANCZOS_SAMPLES, size.y*LANCZOS_SAMPLES));
      Vec2 x_mul_add; x_mul_add.x=LANCZOS_SAMPLES/size_a.x; x_mul_add.y=-pos.x*x_mul_add.x;
      Vec2 y_mul_add; y_mul_add.x=LANCZOS_SAMPLES/size_a.y; y_mul_add.y=-pos.y*y_mul_add.x;

      // ceil is used for min, and floor used for max, because these are coordinates at which the weight function is zero, so we need to process next/previous pixels because they will be the first ones with some weight
      Int x0=CeilSpecial(pos.x-size_a.x), x1=FloorSpecial(pos.x+size_a.x),
          y0=CeilSpecial(pos.y-size_a.y), y1=FloorSpecial(pos.y+size_a.y);

      Flt  weight=0; // this is always non-zero
      Vec  rgb   =0;
      Vec4 color =0;
      for(Int y=y0; y<=y1; y++)
      {
         Flt fy=y*y_mul_add.x + y_mul_add.y; fy=LanczosSharp(fy); Int yi=(clamp ? Mid(y, 0, lh()-1) : Mod(y, lh()));
         for(Int x=x0; x<=x1; x++)
         {
            Flt fx=x*x_mul_add.x + x_mul_add.y; fx=LanczosSharp(fx); Int xi=(clamp ? Mid(x, 0, lw()-1) : Mod(x, lw()));
            fx*=fy;
            Add(color, rgb, colorF(xi, yi), fx, alpha_weight); weight+=fx;
         }
      }
      Normalize(color, rgb, weight, alpha_weight, ALPHA_LIMIT);
      return color;
   }
   return 0;
}
/******************************************************************************/
// GATHER
/******************************************************************************/
static        void RangeAssert (Int *  offset, Int   offsets,   Int   size) {REP(offsets)RANGE_ASSERT(offset[i], size);}
static INLINE void RangeAssertX(Int *x_offset, Int x_offsets, C Image &img) {if(DEBUG)RangeAssert(x_offset, x_offsets, img.lw());}
static INLINE void RangeAssertY(Int *y_offset, Int y_offsets, C Image &img) {if(DEBUG)RangeAssert(y_offset, y_offsets, img.lh());}
static INLINE void RangeAssertZ(Int *z_offset, Int z_offsets, C Image &img) {if(DEBUG)RangeAssert(z_offset, z_offsets, img.ld());}
/******************************************************************************/
void Image::gather(Flt *pixels, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   switch(hwType())
   {
      case IMAGE_F32: FREPD(y, y_offsets)
      {
       C Flt *pixel=(Flt*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*pixels++=pixel[x_offset[x]];
      }break;

      case IMAGE_R8:
      case IMAGE_A8:
      case IMAGE_L8: case IMAGE_L8_SRGB:
      case IMAGE_I8: FREPD(y, y_offsets)
      {
       C Byte *pixel=data()+y_offset[y]*pitch();
         FREPD(x, x_offsets)*pixels++=pixel[x_offset[x]]/Flt(0xFFu);
      }break;

      case IMAGE_I16: FREPD(y, y_offsets)
      {
       C U16 *pixel=(U16*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*pixels++=pixel[x_offset[x]]/Flt(0xFFFFu);
      }break;

      case IMAGE_I32: FREPD(y, y_offsets)
      {
       C U32 *pixel=(U32*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*pixels++=pixel[x_offset[x]]/Dbl(0xFFFFFFFFu); // Dbl required to get best precision
      }break;

      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; *pixels++=ByteToFlt(src.b);}
      }break;

      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; *pixels++=ByteToFlt(src.r);}
      }break;

      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB: FREPD(y, y_offsets)
      {
       C VecB *color=(VecB*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB &src=color[x_offset[x]]; *pixels++=ByteToFlt(src.x);}
      }break;

      default:
      {
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*pixels++=pixelF(x_offset[x], y_offset[y]);
      }break;
   }
}
/******************************************************************************/
void Image::gather(VecB *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(src.b, src.g, src.r);}
      }break;

      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=color[x_offset[x]].v3;
      }break;

      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB: FREPD(y, y_offsets)
      {
       C VecB *color=(VecB*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=color[x_offset[x]];
      }break;

      default:
      {
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=color(x_offset[x], y_offset[y]).v3;
      }break;
   }
}
/******************************************************************************/
void Image::gather(Color *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(src.b, src.g, src.r, src.a);}
      }break;

      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=color[x_offset[x]];
      }break;

      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB: FREPD(y, y_offsets)
      {
       C VecB *color=(VecB*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB &src=color[x_offset[x]]; (colors++)->set(src.x, src.y, src.z);}
      }break;

      default:
      {
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=color(x_offset[x], y_offset[y]);
      }break;
   }
}
/******************************************************************************/
void Image::gather(Vec2 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src.b), ByteToFlt(src.g));}
      }break;

      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src.r), ByteToFlt(src.g));}
      }break;

      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB: FREPD(y, y_offsets)
      {
       C VecB *color=(VecB*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src.x), ByteToFlt(src.y));}
      }break;

      case IMAGE_R8G8: FREPD(y, y_offsets)
      {
       C VecB2 *color=(VecB2*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB2 &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src.x), ByteToFlt(src.y));}
      }break;

      case IMAGE_R8: case IMAGE_I8: FREPD(y, y_offsets)
      {
       C Byte *color=(Byte*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Byte &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src), 0);}
      }break;

      // signed
      case IMAGE_R8G8B8A8_SIGN: FREPD(y, y_offsets)
      {
       C VecSB4 *color=(VecSB4*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecSB4 &src=color[x_offset[x]]; (colors++)->set(SByteToSFlt(src.x), SByteToSFlt(src.y));}
      }break;

      case IMAGE_R8G8_SIGN: FREPD(y, y_offsets)
      {
       C VecSB2 *color=(VecSB2*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecSB2 &src=color[x_offset[x]]; (colors++)->set(SByteToSFlt(src.x), SByteToSFlt(src.y));}
      }break;

      case IMAGE_R8_SIGN: FREPD(y, y_offsets)
      {
       C SByte *color=(SByte*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C SByte &src=color[x_offset[x]]; (colors++)->set(SByteToSFlt(src), 0);}
      }break;

      // Flt
      case IMAGE_F32_4: case IMAGE_F32_4_SRGB: FREPD(y, y_offsets)
      {
       C Vec4 *color=(Vec4*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=color[x_offset[x]].xy;
      }break;

      case IMAGE_F32_3: case IMAGE_F32_3_SRGB: FREPD(y, y_offsets)
      {
       C Vec *color=(Vec*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=color[x_offset[x]].xy;
      }break;

      case IMAGE_F32_2: FREPD(y, y_offsets)
      {
       C Vec2 *color=(Vec2*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=color[x_offset[x]];
      }break;

      case IMAGE_F32: FREPD(y, y_offsets)
      {
       C Flt *color=(Flt*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)(colors++)->set(color[x_offset[x]], 0);
      }break;

      default:
      {
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=colorF(x_offset[x], y_offset[y]).xy;
      }break;
   }
}
void Image::gather(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src.b), ByteToFlt(src.g), ByteToFlt(src.r), ByteToFlt(src.a));}
      }break;

      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src.r), ByteToFlt(src.g), ByteToFlt(src.b), ByteToFlt(src.a));}
      }break;

      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB: FREPD(y, y_offsets)
      {
       C VecB *color=(VecB*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src.x), ByteToFlt(src.y), ByteToFlt(src.z), 1);}
      }break;

      case IMAGE_L8A8: case IMAGE_L8A8_SRGB: FREPD(y, y_offsets) // used for soft text drawing
      {
       C VecB2 *color=(VecB2*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB2 &src=color[x_offset[x]]; Flt l=ByteToFlt(src.x); (colors++)->set(l, l, l, ByteToFlt(src.y));}
      }break;

      // Flt
      case IMAGE_F32_4: case IMAGE_F32_4_SRGB: FREPD(y, y_offsets)
      {
       C Vec4 *color=(Vec4*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=color[x_offset[x]];
      }break;

      case IMAGE_F32_3: case IMAGE_F32_3_SRGB: FREPD(y, y_offsets)
      {
       C Vec *color=(Vec*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)(colors++)->set(color[x_offset[x]], 1);
      }break;

      default:
      {
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=colorF(x_offset[x], y_offset[y]);
      }break;
   }
}
/******************************************************************************/
void Image::gatherL(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src.b), ByteToFlt(src.g), ByteToFlt(src.r), ByteToFlt(src.a));}
      }break;

      case IMAGE_R8G8B8A8: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src.r), ByteToFlt(src.g), ByteToFlt(src.b), ByteToFlt(src.a));}
      }break;

      case IMAGE_R8G8B8: FREPD(y, y_offsets)
      {
       C VecB *color=(VecB*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src.x), ByteToFlt(src.y), ByteToFlt(src.z), 1);}
      }break;

      case IMAGE_L8A8: FREPD(y, y_offsets) // used for soft text drawing
      {
       C VecB2 *color=(VecB2*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB2 &src=color[x_offset[x]]; Flt l=ByteToFlt(src.x); (colors++)->set(l, l, l, ByteToFlt(src.y));}
      }break;

      // sRGB
      case IMAGE_B8G8R8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(ByteSRGBToLinear(src.b), ByteSRGBToLinear(src.g), ByteSRGBToLinear(src.r), ByteToFlt(src.a));}
      }break;

      case IMAGE_R8G8B8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(ByteSRGBToLinear(src.r), ByteSRGBToLinear(src.g), ByteSRGBToLinear(src.b), ByteToFlt(src.a));}
      }break;

      case IMAGE_R8G8B8_SRGB: FREPD(y, y_offsets)
      {
       C VecB *color=(VecB*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB &src=color[x_offset[x]]; (colors++)->set(ByteSRGBToLinear(src.x), ByteSRGBToLinear(src.y), ByteSRGBToLinear(src.z), 1);}
      }break;

      case IMAGE_L8A8_SRGB: FREPD(y, y_offsets) // used for soft text drawing
      {
       C VecB2 *color=(VecB2*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB2 &src=color[x_offset[x]]; Flt l=ByteSRGBToLinear(src.x); (colors++)->set(l, l, l, ByteToFlt(src.y));}
      }break;

      // Flt
      case IMAGE_F32_4: FREPD(y, y_offsets)
      {
       C Vec4 *color=(Vec4*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=color[x_offset[x]];
      }break;

      case IMAGE_F32_3: FREPD(y, y_offsets)
      {
       C Vec *color=(Vec*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)(colors++)->set(color[x_offset[x]], 1);
      }break;

      case IMAGE_F32_4_SRGB: FREPD(y, y_offsets)
      {
       C Vec4 *color=(Vec4*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=SRGBToLinear(color[x_offset[x]]);
      }break;

      case IMAGE_F32_3_SRGB: FREPD(y, y_offsets)
      {
       C Vec *color=(Vec*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)(colors++)->set(SRGBToLinear(color[x_offset[x]]), 1);
      }break;

      default:
      {
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=colorL(x_offset[x], y_offset[y]);
      }break;
   }
}
/******************************************************************************/
void Image::gatherS(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(LinearByteToSRGB(src.b), LinearByteToSRGB(src.g), LinearByteToSRGB(src.r), ByteToFlt(src.a));}
      }break;

      case IMAGE_R8G8B8A8: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(LinearByteToSRGB(src.r), LinearByteToSRGB(src.g), LinearByteToSRGB(src.b), ByteToFlt(src.a));}
      }break;

      case IMAGE_R8G8B8: FREPD(y, y_offsets)
      {
       C VecB *color=(VecB*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB &src=color[x_offset[x]]; (colors++)->set(LinearByteToSRGB(src.x), LinearByteToSRGB(src.y), LinearByteToSRGB(src.z), 1);}
      }break;

      // sRGB
      case IMAGE_B8G8R8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src.b), ByteToFlt(src.g), ByteToFlt(src.r), ByteToFlt(src.a));}
      }break;

      case IMAGE_R8G8B8A8_SRGB: FREPD(y, y_offsets)
      {
       C Color *color=(Color*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C Color &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src.r), ByteToFlt(src.g), ByteToFlt(src.b), ByteToFlt(src.a));}
      }break;

      case IMAGE_R8G8B8_SRGB: FREPD(y, y_offsets)
      {
       C VecB *color=(VecB*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets){C VecB &src=color[x_offset[x]]; (colors++)->set(ByteToFlt(src.x), ByteToFlt(src.y), ByteToFlt(src.z), 1);}
      }break;

      // Flt
      case IMAGE_F32_4: FREPD(y, y_offsets)
      {
       C Vec4 *color=(Vec4*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=LinearToSRGB(color[x_offset[x]]);
      }break;

      case IMAGE_F32_3: FREPD(y, y_offsets)
      {
       C Vec *color=(Vec*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)(colors++)->set(LinearToSRGB(color[x_offset[x]]), 1);
      }break;

      case IMAGE_F32_4_SRGB: FREPD(y, y_offsets)
      {
       C Vec4 *color=(Vec4*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)*colors++=color[x_offset[x]];
      }break;

      case IMAGE_F32_3_SRGB: FREPD(y, y_offsets)
      {
       C Vec *color=(Vec*)(data()+y_offset[y]*pitch());
         FREPD(x, x_offsets)(colors++)->set(color[x_offset[x]], 1);
      }break;

      default:
      {
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=colorS(x_offset[x], y_offset[y]);
      }break;
   }
}
/******************************************************************************/
void Image::gather(Flt *pixels, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   RangeAssertZ(z_offset, z_offsets, T);
   switch(hwType())
   {
      case IMAGE_F32: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Flt *pixel_y=(Flt*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*pixels++=pixel_y[x_offset[x]];
         }
      }break;

      case IMAGE_R8:
      case IMAGE_A8:
      case IMAGE_L8: case IMAGE_L8_SRGB:
      case IMAGE_I8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Byte *pixel_y=data_z+y_offset[y]*pitch();
            FREPD(x, x_offsets)*pixels++=pixel_y[x_offset[x]]/Flt(0xFFu);
         }
      }break;

      case IMAGE_I16: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C U16 *pixel_y=(U16*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*pixels++=pixel_y[x_offset[x]]/Flt(0xFFFFu);
         }
      }break;

      case IMAGE_I32: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C U32 *pixel_y=(U32*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*pixels++=pixel_y[x_offset[x]]/Dbl(0xFFFFFFFFu); // Dbl required to get best precision
         }
      }break;

      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; *pixels++=ByteToFlt(src.b);}
         }
      }break;

      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; *pixels++=ByteToFlt(src.r);}
         }
      }break;

      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB *color_y=(VecB*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecB &src=color_y[x_offset[x]]; *pixels++=ByteToFlt(src.x);}
         }
      }break;

      default:
      {
         FREPD(z, z_offsets)
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*pixels++=pixel3DF(x_offset[x], y_offset[y], z_offset[z]);
      }break;
   }
}
/******************************************************************************/
void Image::gather(VecB *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   RangeAssertZ(z_offset, z_offsets, T);
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(src.b, src.g, src.r);}
         }
      }break;

      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=color_y[x_offset[x]].v3;
         }
      }break;

      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB *color_y=(VecB*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=color_y[x_offset[x]];
         }
      }break;

      default:
      {
         FREPD(z, z_offsets)
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=color3D(x_offset[x], y_offset[y], z_offset[z]).v3;
      }break;
   }
}
/******************************************************************************/
void Image::gather(Color *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   RangeAssertZ(z_offset, z_offsets, T);
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(src.b, src.g, src.r, src.a);}
         }
      }break;

      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=color_y[x_offset[x]];
         }
      }break;

      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB *color_y=(VecB*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecB &src=color_y[x_offset[x]]; (colors++)->set(src.x, src.y, src.z);}
         }
      }break;

      default:
      {
         FREPD(z, z_offsets)
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=color3D(x_offset[x], y_offset[y], z_offset[z]);
      }break;
   }
}
/******************************************************************************/
void Image::gather(Vec2 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   RangeAssertZ(z_offset, z_offsets, T);
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src.b), ByteToFlt(src.g));}
         }
      }break;

      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src.r), ByteToFlt(src.g));}
         }
      }break;

      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB *color_y=(VecB*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecB &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src.x), ByteToFlt(src.y));}
         }
      }break;

      case IMAGE_R8G8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB2 *color_y=(VecB2*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecB2 &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src.x), ByteToFlt(src.y));}
         }
      }break;

      case IMAGE_R8: case IMAGE_I8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Byte *color_y=(Byte*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Byte &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src), 0);}
         }
      }break;

      // signed
      case IMAGE_R8G8B8A8_SIGN: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecSB4 *color_y=(VecSB4*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecSB4 &src=color_y[x_offset[x]]; (colors++)->set(SByteToSFlt(src.x), SByteToSFlt(src.y));}
         }
      }break;

      case IMAGE_R8G8_SIGN: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecSB2 *color_y=(VecSB2*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecSB2 &src=color_y[x_offset[x]]; (colors++)->set(SByteToSFlt(src.x), SByteToSFlt(src.y));}
         }
      }break;

      case IMAGE_R8_SIGN: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C SByte *color_y=(SByte*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C SByte &src=color_y[x_offset[x]]; (colors++)->set(SByteToSFlt(src), 0);}
         }
      }break;

      // Flt
      case IMAGE_F32_4: case IMAGE_F32_4_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec4 *color_y=(Vec4*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=color_y[x_offset[x]].xy;
         }
      }break;

      case IMAGE_F32_3: case IMAGE_F32_3_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec *color_y=(Vec*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=color_y[x_offset[x]].xy;
         }
      }break;

      case IMAGE_F32_2: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec2 *color_y=(Vec2*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=color_y[x_offset[x]];
         }
      }break;

      case IMAGE_F32: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Flt *color_y=(Flt*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)(colors++)->set(color_y[x_offset[x]], 0);
         }
      }break;

      default:
      {
         FREPD(z, z_offsets)
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=color3DF(x_offset[x], y_offset[y], z_offset[z]).xy;
      }break;
   }
}
/******************************************************************************/
void Image::gather(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   RangeAssertZ(z_offset, z_offsets, T);
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src.b), ByteToFlt(src.g), ByteToFlt(src.r), ByteToFlt(src.a));}
         }
      }break;

      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src.r), ByteToFlt(src.g), ByteToFlt(src.b), ByteToFlt(src.a));}
         }
      }break;

      case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB *color_y=(VecB*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecB &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src.x), ByteToFlt(src.y), ByteToFlt(src.z), 1);}
         }
      }break;

      // Flt
      case IMAGE_F32_4: case IMAGE_F32_4_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec4 *color_y=(Vec4*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=color_y[x_offset[x]];
         }
      }break;

      case IMAGE_F32_3: case IMAGE_F32_3_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec *color_y=(Vec*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)(colors++)->set(color_y[x_offset[x]], 1);
         }
      }break;

      default:
      {
         FREPD(z, z_offsets)
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=color3DF(x_offset[x], y_offset[y], z_offset[z]);
      }break;
   }
}
/******************************************************************************/
void Image::gatherL(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   RangeAssertZ(z_offset, z_offsets, T);
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src.b), ByteToFlt(src.g), ByteToFlt(src.r), ByteToFlt(src.a));}
         }
      }break;

      case IMAGE_R8G8B8A8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src.r), ByteToFlt(src.g), ByteToFlt(src.b), ByteToFlt(src.a));}
         }
      }break;

      case IMAGE_R8G8B8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB *color_y=(VecB*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecB &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src.x), ByteToFlt(src.y), ByteToFlt(src.z), 1);}
         }
      }break;

      // sRGB
      case IMAGE_B8G8R8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(ByteSRGBToLinear(src.b), ByteSRGBToLinear(src.g), ByteSRGBToLinear(src.r), ByteToFlt(src.a));}
         }
      }break;

      case IMAGE_R8G8B8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(ByteSRGBToLinear(src.r), ByteSRGBToLinear(src.g), ByteSRGBToLinear(src.b), ByteToFlt(src.a));}
         }
      }break;

      case IMAGE_R8G8B8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB *color_y=(VecB*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecB &src=color_y[x_offset[x]]; (colors++)->set(ByteSRGBToLinear(src.x), ByteSRGBToLinear(src.y), ByteSRGBToLinear(src.z), 1);}
         }
      }break;

      // Flt
      case IMAGE_F32_4: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec4 *color_y=(Vec4*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=color_y[x_offset[x]];
         }
      }break;

      case IMAGE_F32_3: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec *color_y=(Vec*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)(colors++)->set(color_y[x_offset[x]], 1);
         }
      }break;

      case IMAGE_F32_4_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec4 *color_y=(Vec4*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=SRGBToLinear(color_y[x_offset[x]]);
         }
      }break;

      case IMAGE_F32_3_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec *color_y=(Vec*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)(colors++)->set(SRGBToLinear(color_y[x_offset[x]]), 1);
         }
      }break;

      default:
      {
         FREPD(z, z_offsets)
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=color3DL(x_offset[x], y_offset[y], z_offset[z]);
      }break;
   }
}
/******************************************************************************/
void Image::gatherS(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets)C
{
   RangeAssertX(x_offset, x_offsets, T);
   RangeAssertY(y_offset, y_offsets, T);
   RangeAssertZ(z_offset, z_offsets, T);
   switch(hwType())
   {
      case IMAGE_B8G8R8A8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(LinearByteToSRGB(src.b), LinearByteToSRGB(src.g), LinearByteToSRGB(src.r), ByteToFlt(src.a));}
         }
      }break;

      case IMAGE_R8G8B8A8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(LinearByteToSRGB(src.r), LinearByteToSRGB(src.g), LinearByteToSRGB(src.b), ByteToFlt(src.a));}
         }
      }break;

      case IMAGE_R8G8B8: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB *color_y=(VecB*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecB &src=color_y[x_offset[x]]; (colors++)->set(LinearByteToSRGB(src.x), LinearByteToSRGB(src.y), LinearByteToSRGB(src.z), 1);}
         }
      }break;

      // sRGB
      case IMAGE_B8G8R8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src.b), ByteToFlt(src.g), ByteToFlt(src.r), ByteToFlt(src.a));}
         }
      }break;

      case IMAGE_R8G8B8A8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Color *color_y=(Color*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C Color &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src.r), ByteToFlt(src.g), ByteToFlt(src.b), ByteToFlt(src.a));}
         }
      }break;

      case IMAGE_R8G8B8_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C VecB *color_y=(VecB*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets){C VecB &src=color_y[x_offset[x]]; (colors++)->set(ByteToFlt(src.x), ByteToFlt(src.y), ByteToFlt(src.z), 1);}
         }
      }break;

      // Flt
      case IMAGE_F32_4: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec4 *color_y=(Vec4*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=LinearToSRGB(color_y[x_offset[x]]);
         }
      }break;

      case IMAGE_F32_3: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec *color_y=(Vec*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)(colors++)->set(LinearToSRGB(color_y[x_offset[x]]), 1);
         }
      }break;

      case IMAGE_F32_4_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec4 *color_y=(Vec4*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)*colors++=color_y[x_offset[x]];
         }
      }break;

      case IMAGE_F32_3_SRGB: FREPD(z, z_offsets)
      {
       C Byte *data_z=data()+z_offset[z]*pitch2(); FREPD(y, y_offsets)
         {
          C Vec *color_y=(Vec*)(data_z+y_offset[y]*pitch());
            FREPD(x, x_offsets)(colors++)->set(color_y[x_offset[x]], 1);
         }
      }break;

      default:
      {
         FREPD(z, z_offsets)
         FREPD(y, y_offsets)
         FREPD(x, x_offsets)*colors++=color3DS(x_offset[x], y_offset[y], z_offset[z]);
      }break;
   }
}
/******************************************************************************/
static Bool NeedMultiChannel(IMAGE_TYPE src, IMAGE_TYPE dest)
{
   return ImageTI[src].channels>1 || src!=dest;
}
void CopyNoStretch(C Image &src, Image &dest, Bool clamp, Bool ignore_gamma) // assumes 'src', 'dest' are locked and non-compressed
{
   Bool high_precision=(src.highPrecision() && dest.highPrecision()); // high precision requires FP
   if(CanDoRawCopy(src, dest, ignore_gamma)) // no retype
   {
      Int w=Min(src.lw(), dest.lw()),
          h=Min(src.lh(), dest.lh()),
          d=Min(src.ld(), dest.ld()),
          copy_pitch=w*src.bytePP();
      FREPD(z, d)
      {
         Byte *dest_z=dest.data() + z*dest.pitch2();
       C Byte * src_z=src .data() + z*src .pitch2();
         FREPD(y, h)
         {
            Byte *dest_y=dest_z + y*dest.pitch();
          C Byte * src_y= src_z + y*src .pitch();
            CopyFast(dest_y, src_y, copy_pitch);

            if(high_precision)                                                      for(Int x=w; x<dest.lw(); x++)dest.color3DF(x, y, z, clamp ? src.color3DF(Min(x, src.lw()-1), Min(y, src.lh()-1), Min(z, src.ld()-1)) : src.color3DF(x%src.lw(), y%src.lh(), z%src.ld()));
            else                                                                    for(Int x=w; x<dest.lw(); x++)dest.color3D (x, y, z, clamp ? src.color3D (Min(x, src.lw()-1), Min(y, src.lh()-1), Min(z, src.ld()-1)) : src.color3D (x%src.lw(), y%src.lh(), z%src.ld()));
         }
         if(high_precision)                           for(Int y=h; y<dest.lh(); y++)for(Int x=0; x<dest.lw(); x++)dest.color3DF(x, y, z, clamp ? src.color3DF(Min(x, src.lw()-1), Min(y, src.lh()-1), Min(z, src.ld()-1)) : src.color3DF(x%src.lw(), y%src.lh(), z%src.ld()));
         else                                         for(Int y=h; y<dest.lh(); y++)for(Int x=0; x<dest.lw(); x++)dest.color3D (x, y, z, clamp ? src.color3D (Min(x, src.lw()-1), Min(y, src.lh()-1), Min(z, src.ld()-1)) : src.color3D (x%src.lw(), y%src.lh(), z%src.ld()));
      }
      if(high_precision)for(Int z=d; z<dest.ld(); z++)for(Int y=0; y<dest.lh(); y++)for(Int x=0; x<dest.lw(); x++)dest.color3DF(x, y, z, clamp ? src.color3DF(Min(x, src.lw()-1), Min(y, src.lh()-1), Min(z, src.ld()-1)) : src.color3DF(x%src.lw(), y%src.lh(), z%src.ld()));
      else              for(Int z=d; z<dest.ld(); z++)for(Int y=0; y<dest.lh(); y++)for(Int x=0; x<dest.lw(); x++)dest.color3D (x, y, z, clamp ? src.color3D (Min(x, src.lw()-1), Min(y, src.lh()-1), Min(z, src.ld()-1)) : src.color3D (x%src.lw(), y%src.lh(), z%src.ld()));
   }else
 /*if(dest.hwType()==IMAGE_R8G8B8A8
   && dest.  type()==IMAGE_R8G8B8A8) // check 'type' too in case we have to perform color adjustment
   {
      Int x_blocks=dest.lw()/4,
          y_blocks=dest.lh()/4;
      REPD(z, dest.ld())
      {
         Byte *dest_z=dest.data() + z*dest.pitch2();
         Int   zo    =(clamp ? Min(z, src.ld()-1) : z%src.ld());
         REPD(by, y_blocks)
         {
            Int py=by*4, yo[4]; // pixel and offset
            REPAO(yo)=(clamp ? Min(py+i, src.lh()-1) : (py+i)%src.lh());
            Byte *dest_y=dest_z + py*dest.pitch();
            REPD(bx, x_blocks)
            {
               Int px=bx*4, xo[4]; // pixel and offset
               REPAO(xo)=(clamp ? Min(px+i, src.lw()-1) : (px+i)%src.lw());
               Color col[4][4];
               src.gather(col[0], xo, Elms(xo), yo, Elms(yo), &zo, 1);
               Byte *dest_x=dest_y + px*SIZE(Color);
               REP(4)CopyFast(dest_x + i*dest.pitch(), col[i], SIZE(Color)*4);
            }
         }

         // process partial blocks
         x_blocks*=4;
         y_blocks*=4;

         // process right side (excluding shared corner)
         if(x_blocks!=dest.lw())
            for(Int y=       0; y<y_blocks; y++)
            for(Int x=x_blocks; x<dest.lw(); x++)dest.color3D(x, y, z, clamp ? src.color3D(Min(x, src.lw()-1), Min(y, src.lh()-1), zo) : src.color3D(x%src.lw(), y%src.lh(), zo));

         // process bottom side (including shared corner)
         //if(y_blocks!=dest.lh()) not needed since we're starting with Y's and this will be checked on its own
            for(Int y=y_blocks; y<dest.lh(); y++)
            for(Int x=       0; x<dest.lw(); x++)dest.color3D(x, y, z, clamp ? src.color3D(Min(x, src.lw()-1), Min(y, src.lh()-1), zo) : src.color3D(x%src.lw(), y%src.lh(), zo));
      }
   }else*/
   if(!ignore_gamma)
   {
      REPD(z, dest.ld())
      REPD(y, dest.lh())
      REPD(x, dest.lw())dest.color3DL(x, y, z, clamp ? src.color3DL(Min(x, src.lw()-1), Min(y, src.lh()-1), Min(z, src.ld()-1)) : src.color3DL(x%src.lw(), y%src.lh(), z%src.ld()));
   }else
   if(high_precision)
   {
      REPD(z, dest.ld())
      REPD(y, dest.lh())
      REPD(x, dest.lw())dest.color3DF(x, y, z, clamp ? src.color3DF(Min(x, src.lw()-1), Min(y, src.lh()-1), Min(z, src.ld()-1)) : src.color3DF(x%src.lw(), y%src.lh(), z%src.ld()));
   }else
   {
      REPD(z, dest.ld())
      REPD(y, dest.lh())
      REPD(x, dest.lw())dest.color3D(x, y, z, clamp ? src.color3D(Min(x, src.lw()-1), Min(y, src.lh()-1), Min(z, src.ld()-1)) : src.color3D(x%src.lw(), y%src.lh(), z%src.ld()));
   }
}
struct CopyContext
{
 C Image &src ;
   Image &dest;
   const Bool        clamp, keep_edges, alpha_weight, no_alpha_limit, src_srgb, dest_srgb, ignore_gamma, ignore_gamma_ds, src_high_prec, high_prec;
   const FILTER_TYPE filter;
   const Int         src_faces1;
   const UInt        flags;
   void (*const SetColor)(Byte *data, IMAGE_TYPE type, IMAGE_TYPE hw_type, C Vec4 &color);

   // case-dependent
   Bool hp, manual_linear_to_srgb;
   void (*set_color)(Byte *data, IMAGE_TYPE type, IMAGE_TYPE hw_type, C Vec4 &color);

   Vec2 x_mul_add, y_mul_add, z_mul_add;
   Vec  area_size;
   union // pointer to class method
   {
      PtrImagePixel     ptr_pixel;
      PtrImagePixel3D   ptr_pixel_3D;
      PtrImageColor     ptr_color;
      PtrImageColor3D   ptr_color_3D;
      PtrImageAreaColor ptr_area_color;
   };

   Flt   alpha_limit;
   Flt (*Weight)(Flt x);

   static void Downsize2xLinear(IntPtr elm_index, CopyContext &ctx, Int thread_index) {ctx.downsize2xLinear(elm_index);}
          void downsize2xLinear(Int y)
   {
      Int yc[2]; yc[0]=y*2; yc[1]=(clamp ? Min(yc[0]+1, src.lh()-1) : (yc[0]+1)%src.lh()); // yc[0] is always OK
      Byte *dest_data_y=dest.data()+y*dest.pitch();
      Byte *dest_data_x=dest_data_y; FREPD(x, dest.lw()) // iterate forward so we can increase pointers
      {
         Int xc[2]; xc[0]=x*2; xc[1]=(clamp ? Min(xc[0]+1, src.lw()-1) : (xc[0]+1)%src.lw()); // xc[0] is always OK
         if(hp)
         {
            Vec4 col, c[2][2]; src.gatherL(&c[0][0], xc, Elms(xc), yc, Elms(yc)); // [y][x]
            if(!alpha_weight)
            {
               col.w=Avg(c[0][0].w, c[0][1].w, c[1][0].w, c[1][1].w);
            rgb_f:
               col.x=Avg(c[0][0].x, c[0][1].x, c[1][0].x, c[1][1].x);
               col.y=Avg(c[0][0].y, c[0][1].y, c[1][0].y, c[1][1].y);
               col.z=Avg(c[0][0].z, c[0][1].z, c[1][0].z, c[1][1].z);
            }else
            {
               Flt a=c[0][0].w+c[0][1].w+c[1][0].w+c[1][1].w;
               if(!a){col.w=0; goto rgb_f;}
               col.w=a/4;
               col.x=(c[0][0].x*c[0][0].w + c[0][1].x*c[0][1].w + c[1][0].x*c[1][0].w + c[1][1].x*c[1][1].w)/a;
               col.y=(c[0][0].y*c[0][0].w + c[0][1].y*c[0][1].w + c[1][0].y*c[1][0].w + c[1][1].y*c[1][1].w)/a;
               col.z=(c[0][0].z*c[0][0].w + c[0][1].z*c[0][1].w + c[1][0].z*c[1][0].w + c[1][1].z*c[1][1].w)/a;
            }
            if(manual_linear_to_srgb)col.xyz=LinearToSRGB(col.xyz);
            set_color(dest_data_x, dest.type(), dest.hwType(), col);
         }else
         {
            Color col, c[2][2]; src.gather(&c[0][0], xc, Elms(xc), yc, Elms(yc)); // [y][x]
            if(!alpha_weight)
            {
               col.a=((c[0][0].a+c[0][1].a+c[1][0].a+c[1][1].a+2)>>2);
            rgb:
               col.r=((c[0][0].r+c[0][1].r+c[1][0].r+c[1][1].r+2)>>2);
               col.g=((c[0][0].g+c[0][1].g+c[1][0].g+c[1][1].g+2)>>2);
               col.b=((c[0][0].b+c[0][1].b+c[1][0].b+c[1][1].b+2)>>2);
            }else
            {
               UInt a=c[0][0].a+c[0][1].a+c[1][0].a+c[1][1].a;
               if( !a){col.a=0; goto rgb;}
               col.a=((a+2)>>2); UInt a_2=a>>1;
               col.r=(c[0][0].r*c[0][0].a + c[0][1].r*c[0][1].a + c[1][0].r*c[1][0].a + c[1][1].r*c[1][1].a + a_2)/a;
               col.g=(c[0][0].g*c[0][0].a + c[0][1].g*c[0][1].a + c[1][0].g*c[1][0].a + c[1][1].g*c[1][1].a + a_2)/a;
               col.b=(c[0][0].b*c[0][0].a + c[0][1].b*c[0][1].a + c[1][0].b*c[1][0].a + c[1][1].b*c[1][1].a + a_2)/a;
            }
            dest.color(x, y, col);
         }
         dest_data_x+=dest.bytePP();
      }
   }
   static void Downsize2xLinear3D(IntPtr elm_index, CopyContext &ctx, Int thread_index) {ctx.downsize2xLinear3D(elm_index);}
          void downsize2xLinear3D(IntPtr elm_index)
   {
      Int z=elm_index/dest.lh(),
          y=elm_index%dest.lh();
      Int zc[2]; zc[0]=z*2; zc[1]=(clamp ? Min(zc[0]+1, src.ld()-1) : (zc[0]+1)%src.ld()); // zc[0] is always OK
      Int yc[2]; yc[0]=y*2; yc[1]=(clamp ? Min(yc[0]+1, src.lh()-1) : (yc[0]+1)%src.lh()); // yc[0] is always OK
      REPD(x, dest.lw())
      {
         Int xc[2]; xc[0]=x*2; xc[1]=(clamp ? Min(xc[0]+1, src.lw()-1) : (xc[0]+1)%src.lw()); // xc[0] is always OK
         Color col, c[2][2][2]; src.gather(&c[0][0][0], xc, Elms(xc), yc, Elms(yc), zc, Elms(zc)); // [z][y][x]
         if(!alpha_weight)
         {
            col.a=((c[0][0][0].a+c[0][0][1].a+c[0][1][0].a+c[0][1][1].a+c[1][0][0].a+c[1][0][1].a+c[1][1][0].a+c[1][1][1].a+4)>>3);
         rgb_3D:
            col.r=((c[0][0][0].r+c[0][0][1].r+c[0][1][0].r+c[0][1][1].r+c[1][0][0].r+c[1][0][1].r+c[1][1][0].r+c[1][1][1].r+4)>>3);
            col.g=((c[0][0][0].g+c[0][0][1].g+c[0][1][0].g+c[0][1][1].g+c[1][0][0].g+c[1][0][1].g+c[1][1][0].g+c[1][1][1].g+4)>>3);
            col.b=((c[0][0][0].b+c[0][0][1].b+c[0][1][0].b+c[0][1][1].b+c[1][0][0].b+c[1][0][1].b+c[1][1][0].b+c[1][1][1].b+4)>>3);
         }else
         {
            UInt a=c[0][0][0].a+c[0][0][1].a+c[0][1][0].a+c[0][1][1].a+c[1][0][0].a+c[1][0][1].a+c[1][1][0].a+c[1][1][1].a;
            if( !a){col.a=0; goto rgb_3D;}
            col.a=((a+4)>>3); UInt a_2=a>>1;
            col.r=(c[0][0][0].r*c[0][0][0].a + c[0][0][1].r*c[0][0][1].a + c[0][1][0].r*c[0][1][0].a + c[0][1][1].r*c[0][1][1].a + c[1][0][0].r*c[1][0][0].a + c[1][0][1].r*c[1][0][1].a + c[1][1][0].r*c[1][1][0].a + c[1][1][1].r*c[1][1][1].a + a_2)/a;
            col.g=(c[0][0][0].g*c[0][0][0].a + c[0][0][1].g*c[0][0][1].a + c[0][1][0].g*c[0][1][0].a + c[0][1][1].g*c[0][1][1].a + c[1][0][0].g*c[1][0][0].a + c[1][0][1].g*c[1][0][1].a + c[1][1][0].g*c[1][1][0].a + c[1][1][1].g*c[1][1][1].a + a_2)/a;
            col.b=(c[0][0][0].b*c[0][0][0].a + c[0][0][1].b*c[0][0][1].a + c[0][1][0].b*c[0][1][0].a + c[0][1][1].b*c[0][1][1].a + c[1][0][0].b*c[1][0][0].a + c[1][0][1].b*c[1][0][1].a + c[1][1][0].b*c[1][1][0].a + c[1][1][1].b*c[1][1][1].a + a_2)/a;
         }
         dest.color3D(x, y, z, col);
      }
   }
   static void Downsize2xCubicFastSharp(IntPtr elm_index, CopyContext &ctx, Int thread_index) {ctx.downsize2xCubicFastSharp(elm_index);}
          void downsize2xCubicFastSharp(Int y)
   {
      Int yc[8]; yc[3]=y*2; // 'y[3]' is always OK
      if(clamp){yc[0]=Max(yc[3]-3, 0       ); yc[1]=Max(yc[3]-2, 0       ); yc[2]=Max(yc[3]-1, 0       ); yc[4]=Min(yc[3]+1, src.lh()-1); yc[5]=Min(yc[3]+2, src.lh()-1); yc[6]=Min(yc[3]+3, src.lh()-1); yc[7]=Min(yc[3]+4, src.lh()-1);}
      else     {yc[0]=Mod(yc[3]-3, src.lh()); yc[1]=Mod(yc[3]-2, src.lh()); yc[2]=Mod(yc[3]-1, src.lh()); yc[4]=   (yc[3]+1)%src.lh()   ; yc[5]=   (yc[3]+2)%src.lh()   ; yc[6]=   (yc[3]+3)%src.lh()   ; yc[7]=   (yc[3]+4)%src.lh()   ;}
      REPD(x, dest.lw())
      {
         Int xc[8]; xc[3]=x*2; // 'x[3]' is always OK
         if(clamp){xc[0]=Max(xc[3]-3, 0       ); xc[1]=Max(xc[3]-2, 0       ); xc[2]=Max(xc[3]-1, 0       ); xc[4]=Min(xc[3]+1, src.lw()-1); xc[5]=Min(xc[3]+2, src.lw()-1); xc[6]=Min(xc[3]+3, src.lw()-1); xc[7]=Min(xc[3]+4, src.lw()-1);}
         else     {xc[0]=Mod(xc[3]-3, src.lw()); xc[1]=Mod(xc[3]-2, src.lw()); xc[2]=Mod(xc[3]-1, src.lw()); xc[4]=   (xc[3]+1)%src.lw()   ; xc[5]=   (xc[3]+2)%src.lw()   ; xc[6]=   (xc[3]+3)%src.lw()   ; xc[7]=   (xc[3]+4)%src.lw()   ;}
         Color col, c[8][8]; // [y][x]
      #if 1 // read 8x8
         src.gather(&c[0][0], xc, Elms(xc), yc, Elms(yc));
      #else // read 4x1, 8x6, 4x1, performance is the same
         src.gather(&c[0][2], xc+2, Elms(xc)-4, yc  , 1         ); // top
         src.gather(&c[1][0], xc  , Elms(xc)  , yc+1, Elms(yc)-2); // center
         src.gather(&c[7][2], xc+2, Elms(xc)-4, yc+7, 1         ); // bottom
      #endif
         if(!alpha_weight)
         {
            col.a=Mid((/*c[0][0].a*CW8[0][0]  + c[0][1].a*CW8[0][1]*/+ c[0][2].a*CW8[0][2] + c[0][3].a*CW8[0][3] + c[0][4].a*CW8[0][4] + c[0][5].a*CW8[0][5] +/*c[0][6].a*CW8[0][6] +  c[0][7].a*CW8[0][7]*/
                     /*+ c[1][0].a*CW8[1][0]*/+ c[1][1].a*CW8[1][1]  + c[1][2].a*CW8[1][2] + c[1][3].a*CW8[1][3] + c[1][4].a*CW8[1][4] + c[1][5].a*CW8[1][5] +  c[1][6].a*CW8[1][6] +/*c[1][7].a*CW8[1][7]*/
                       + c[2][0].a*CW8[2][0]  + c[2][1].a*CW8[2][1]  + c[2][2].a*CW8[2][2] + c[2][3].a*CW8[2][3] + c[2][4].a*CW8[2][4] + c[2][5].a*CW8[2][5] +  c[2][6].a*CW8[2][6] +  c[2][7].a*CW8[2][7]
                       + c[3][0].a*CW8[3][0]  + c[3][1].a*CW8[3][1]  + c[3][2].a*CW8[3][2] + c[3][3].a*CW8[3][3] + c[3][4].a*CW8[3][4] + c[3][5].a*CW8[3][5] +  c[3][6].a*CW8[3][6] +  c[3][7].a*CW8[3][7]
                       + c[4][0].a*CW8[4][0]  + c[4][1].a*CW8[4][1]  + c[4][2].a*CW8[4][2] + c[4][3].a*CW8[4][3] + c[4][4].a*CW8[4][4] + c[4][5].a*CW8[4][5] +  c[4][6].a*CW8[4][6] +  c[4][7].a*CW8[4][7]
                       + c[5][0].a*CW8[5][0]  + c[5][1].a*CW8[5][1]  + c[5][2].a*CW8[5][2] + c[5][3].a*CW8[5][3] + c[5][4].a*CW8[5][4] + c[5][5].a*CW8[5][5] +  c[5][6].a*CW8[5][6] +  c[5][7].a*CW8[5][7]
                     /*+ c[6][0].a*CW8[6][0]*/+ c[6][1].a*CW8[6][1]  + c[6][2].a*CW8[6][2] + c[6][3].a*CW8[6][3] + c[6][4].a*CW8[6][4] + c[6][5].a*CW8[6][5] +  c[6][6].a*CW8[6][6] +/*c[6][7].a*CW8[6][7]*/
                     /*+ c[7][0].a*CW8[7][0]  + c[7][1].a*CW8[7][1]*/+ c[7][2].a*CW8[7][2] + c[7][3].a*CW8[7][3] + c[7][4].a*CW8[7][4] + c[7][5].a*CW8[7][5] +/*c[7][6].a*CW8[7][6] +  c[7][7].a*CW8[7][7]*/ + CW8Sum/2)/CW8Sum, 0, 255);
         rgb:
            col.r=Mid((/*c[0][0].r*CW8[0][0]  + c[0][1].r*CW8[0][1]*/+ c[0][2].r*CW8[0][2] + c[0][3].r*CW8[0][3] + c[0][4].r*CW8[0][4] + c[0][5].r*CW8[0][5] +/*c[0][6].r*CW8[0][6] +  c[0][7].r*CW8[0][7]*/
                     /*+ c[1][0].r*CW8[1][0]*/+ c[1][1].r*CW8[1][1]  + c[1][2].r*CW8[1][2] + c[1][3].r*CW8[1][3] + c[1][4].r*CW8[1][4] + c[1][5].r*CW8[1][5] +  c[1][6].r*CW8[1][6] +/*c[1][7].r*CW8[1][7]*/
                       + c[2][0].r*CW8[2][0]  + c[2][1].r*CW8[2][1]  + c[2][2].r*CW8[2][2] + c[2][3].r*CW8[2][3] + c[2][4].r*CW8[2][4] + c[2][5].r*CW8[2][5] +  c[2][6].r*CW8[2][6] +  c[2][7].r*CW8[2][7]
                       + c[3][0].r*CW8[3][0]  + c[3][1].r*CW8[3][1]  + c[3][2].r*CW8[3][2] + c[3][3].r*CW8[3][3] + c[3][4].r*CW8[3][4] + c[3][5].r*CW8[3][5] +  c[3][6].r*CW8[3][6] +  c[3][7].r*CW8[3][7]
                       + c[4][0].r*CW8[4][0]  + c[4][1].r*CW8[4][1]  + c[4][2].r*CW8[4][2] + c[4][3].r*CW8[4][3] + c[4][4].r*CW8[4][4] + c[4][5].r*CW8[4][5] +  c[4][6].r*CW8[4][6] +  c[4][7].r*CW8[4][7]
                       + c[5][0].r*CW8[5][0]  + c[5][1].r*CW8[5][1]  + c[5][2].r*CW8[5][2] + c[5][3].r*CW8[5][3] + c[5][4].r*CW8[5][4] + c[5][5].r*CW8[5][5] +  c[5][6].r*CW8[5][6] +  c[5][7].r*CW8[5][7]
                     /*+ c[6][0].r*CW8[6][0]*/+ c[6][1].r*CW8[6][1]  + c[6][2].r*CW8[6][2] + c[6][3].r*CW8[6][3] + c[6][4].r*CW8[6][4] + c[6][5].r*CW8[6][5] +  c[6][6].r*CW8[6][6] +/*c[6][7].r*CW8[6][7]*/
                     /*+ c[7][0].r*CW8[7][0]  + c[7][1].r*CW8[7][1]*/+ c[7][2].r*CW8[7][2] + c[7][3].r*CW8[7][3] + c[7][4].r*CW8[7][4] + c[7][5].r*CW8[7][5] +/*c[7][6].r*CW8[7][6] +  c[7][7].r*CW8[7][7]*/ + CW8Sum/2)/CW8Sum, 0, 255);
            col.g=Mid((/*c[0][0].g*CW8[0][0]  + c[0][1].g*CW8[0][1]*/+ c[0][2].g*CW8[0][2] + c[0][3].g*CW8[0][3] + c[0][4].g*CW8[0][4] + c[0][5].g*CW8[0][5] +/*c[0][6].g*CW8[0][6] +  c[0][7].g*CW8[0][7]*/
                     /*+ c[1][0].g*CW8[1][0]*/+ c[1][1].g*CW8[1][1]  + c[1][2].g*CW8[1][2] + c[1][3].g*CW8[1][3] + c[1][4].g*CW8[1][4] + c[1][5].g*CW8[1][5] +  c[1][6].g*CW8[1][6] +/*c[1][7].g*CW8[1][7]*/
                       + c[2][0].g*CW8[2][0]  + c[2][1].g*CW8[2][1]  + c[2][2].g*CW8[2][2] + c[2][3].g*CW8[2][3] + c[2][4].g*CW8[2][4] + c[2][5].g*CW8[2][5] +  c[2][6].g*CW8[2][6] +  c[2][7].g*CW8[2][7]
                       + c[3][0].g*CW8[3][0]  + c[3][1].g*CW8[3][1]  + c[3][2].g*CW8[3][2] + c[3][3].g*CW8[3][3] + c[3][4].g*CW8[3][4] + c[3][5].g*CW8[3][5] +  c[3][6].g*CW8[3][6] +  c[3][7].g*CW8[3][7]
                       + c[4][0].g*CW8[4][0]  + c[4][1].g*CW8[4][1]  + c[4][2].g*CW8[4][2] + c[4][3].g*CW8[4][3] + c[4][4].g*CW8[4][4] + c[4][5].g*CW8[4][5] +  c[4][6].g*CW8[4][6] +  c[4][7].g*CW8[4][7]
                       + c[5][0].g*CW8[5][0]  + c[5][1].g*CW8[5][1]  + c[5][2].g*CW8[5][2] + c[5][3].g*CW8[5][3] + c[5][4].g*CW8[5][4] + c[5][5].g*CW8[5][5] +  c[5][6].g*CW8[5][6] +  c[5][7].g*CW8[5][7]
                     /*+ c[6][0].g*CW8[6][0]*/+ c[6][1].g*CW8[6][1]  + c[6][2].g*CW8[6][2] + c[6][3].g*CW8[6][3] + c[6][4].g*CW8[6][4] + c[6][5].g*CW8[6][5] +  c[6][6].g*CW8[6][6] +/*c[6][7].g*CW8[6][7]*/
                     /*+ c[7][0].g*CW8[7][0]  + c[7][1].g*CW8[7][1]*/+ c[7][2].g*CW8[7][2] + c[7][3].g*CW8[7][3] + c[7][4].g*CW8[7][4] + c[7][5].g*CW8[7][5] +/*c[7][6].g*CW8[7][6] +  c[7][7].g*CW8[7][7]*/ + CW8Sum/2)/CW8Sum, 0, 255);
            col.b=Mid((/*c[0][0].b*CW8[0][0]  + c[0][1].b*CW8[0][1]*/+ c[0][2].b*CW8[0][2] + c[0][3].b*CW8[0][3] + c[0][4].b*CW8[0][4] + c[0][5].b*CW8[0][5] +/*c[0][6].b*CW8[0][6] +  c[0][7].b*CW8[0][7]*/
                     /*+ c[1][0].b*CW8[1][0]*/+ c[1][1].b*CW8[1][1]  + c[1][2].b*CW8[1][2] + c[1][3].b*CW8[1][3] + c[1][4].b*CW8[1][4] + c[1][5].b*CW8[1][5] +  c[1][6].b*CW8[1][6] +/*c[1][7].b*CW8[1][7]*/
                       + c[2][0].b*CW8[2][0]  + c[2][1].b*CW8[2][1]  + c[2][2].b*CW8[2][2] + c[2][3].b*CW8[2][3] + c[2][4].b*CW8[2][4] + c[2][5].b*CW8[2][5] +  c[2][6].b*CW8[2][6] +  c[2][7].b*CW8[2][7]
                       + c[3][0].b*CW8[3][0]  + c[3][1].b*CW8[3][1]  + c[3][2].b*CW8[3][2] + c[3][3].b*CW8[3][3] + c[3][4].b*CW8[3][4] + c[3][5].b*CW8[3][5] +  c[3][6].b*CW8[3][6] +  c[3][7].b*CW8[3][7]
                       + c[4][0].b*CW8[4][0]  + c[4][1].b*CW8[4][1]  + c[4][2].b*CW8[4][2] + c[4][3].b*CW8[4][3] + c[4][4].b*CW8[4][4] + c[4][5].b*CW8[4][5] +  c[4][6].b*CW8[4][6] +  c[4][7].b*CW8[4][7]
                       + c[5][0].b*CW8[5][0]  + c[5][1].b*CW8[5][1]  + c[5][2].b*CW8[5][2] + c[5][3].b*CW8[5][3] + c[5][4].b*CW8[5][4] + c[5][5].b*CW8[5][5] +  c[5][6].b*CW8[5][6] +  c[5][7].b*CW8[5][7]
                     /*+ c[6][0].b*CW8[6][0]*/+ c[6][1].b*CW8[6][1]  + c[6][2].b*CW8[6][2] + c[6][3].b*CW8[6][3] + c[6][4].b*CW8[6][4] + c[6][5].b*CW8[6][5] +  c[6][6].b*CW8[6][6] +/*c[6][7].b*CW8[6][7]*/
                     /*+ c[7][0].b*CW8[7][0]  + c[7][1].b*CW8[7][1]*/+ c[7][2].b*CW8[7][2] + c[7][3].b*CW8[7][3] + c[7][4].b*CW8[7][4] + c[7][5].b*CW8[7][5] +/*c[7][6].b*CW8[7][6] +  c[7][7].b*CW8[7][7]*/ + CW8Sum/2)/CW8Sum, 0, 255);
         }else
         {
            Int w[8][8]={{/*CWA8[0][0]*c[0][0].a*/0,/*CWA8[0][1]*c[0][1].a*/0, CWA8[0][2]*c[0][2].a, CWA8[0][3]*c[0][3].a, CWA8[0][4]*c[0][4].a, CWA8[0][5]*c[0][5].a,/*CWA8[0][6]*c[0][6].a*/0,/*CWA8[0][7]*c[0][7].a*/0},
                         {/*CWA8[1][0]*c[1][0].a*/0,  CWA8[1][1]*c[1][1].a   , CWA8[1][2]*c[1][2].a, CWA8[1][3]*c[1][3].a, CWA8[1][4]*c[1][4].a, CWA8[1][5]*c[1][5].a,  CWA8[1][6]*c[1][6].a   ,/*CWA8[1][7]*c[1][7].a*/0},
                         {  CWA8[2][0]*c[2][0].a   ,  CWA8[2][1]*c[2][1].a   , CWA8[2][2]*c[2][2].a, CWA8[2][3]*c[2][3].a, CWA8[2][4]*c[2][4].a, CWA8[2][5]*c[2][5].a,  CWA8[2][6]*c[2][6].a   ,  CWA8[2][7]*c[2][7].a   },
                         {  CWA8[3][0]*c[3][0].a   ,  CWA8[3][1]*c[3][1].a   , CWA8[3][2]*c[3][2].a, CWA8[3][3]*c[3][3].a, CWA8[3][4]*c[3][4].a, CWA8[3][5]*c[3][5].a,  CWA8[3][6]*c[3][6].a   ,  CWA8[3][7]*c[3][7].a   },
                         {  CWA8[4][0]*c[4][0].a   ,  CWA8[4][1]*c[4][1].a   , CWA8[4][2]*c[4][2].a, CWA8[4][3]*c[4][3].a, CWA8[4][4]*c[4][4].a, CWA8[4][5]*c[4][5].a,  CWA8[4][6]*c[4][6].a   ,  CWA8[4][7]*c[4][7].a   },
                         {  CWA8[5][0]*c[5][0].a   ,  CWA8[5][1]*c[5][1].a   , CWA8[5][2]*c[5][2].a, CWA8[5][3]*c[5][3].a, CWA8[5][4]*c[5][4].a, CWA8[5][5]*c[5][5].a,  CWA8[5][6]*c[5][6].a   ,  CWA8[5][7]*c[5][7].a   },
                         {/*CWA8[6][0]*c[6][0].a*/0,  CWA8[6][1]*c[6][1].a   , CWA8[6][2]*c[6][2].a, CWA8[6][3]*c[6][3].a, CWA8[6][4]*c[6][4].a, CWA8[6][5]*c[6][5].a,  CWA8[6][6]*c[6][6].a   ,/*CWA8[6][7]*c[6][7].a*/0},
                         {/*CWA8[7][0]*c[7][0].a*/0,/*CWA8[7][1]*c[7][1].a*/0, CWA8[7][2]*c[7][2].a, CWA8[7][3]*c[7][3].a, CWA8[7][4]*c[7][4].a, CWA8[7][5]*c[7][5].a,/*CWA8[7][6]*c[7][6].a*/0,/*CWA8[7][7]*c[7][7].a*/0}};
            Int total_alpha_weight=/*w[0][0]  + w[0][1]*/+ w[0][2] + w[0][3] + w[0][4] + w[0][5]/*+ w[0][6]  + w[0][7]*/
                                 /*+ w[1][0]*/+ w[1][1]  + w[1][2] + w[1][3] + w[1][4] + w[1][5]  + w[1][6]/*+ w[1][7]*/
                                   + w[2][0]  + w[2][1]  + w[2][2] + w[2][3] + w[2][4] + w[2][5]  + w[2][6]  + w[2][7]
                                   + w[3][0]  + w[3][1]  + w[3][2] + w[3][3] + w[3][4] + w[3][5]  + w[3][6]  + w[3][7]
                                   + w[4][0]  + w[4][1]  + w[4][2] + w[4][3] + w[4][4] + w[4][5]  + w[4][6]  + w[4][7]
                                   + w[5][0]  + w[5][1]  + w[5][2] + w[5][3] + w[5][4] + w[5][5]  + w[5][6]  + w[5][7]
                                 /*+ w[6][0]*/+ w[6][1]  + w[6][2] + w[6][3] + w[6][4] + w[6][5]  + w[6][6]/*+ w[6][7]*/
                                 /*+ w[7][0]  + w[7][1]*/+ w[7][2] + w[7][3] + w[7][4] + w[7][5]/*+ w[7][6]  + w[7][7]*/;
            if(total_alpha_weight<=0){col.a=0; goto rgb;}
            col.a=Min(DivRound(total_alpha_weight, CWA8Sum), 255); // here "total_alpha_weight>0" so no need to do "Max(0, "
            if(total_alpha_weight<CWA8AlphaLimit) // below this limit, lerp to RGB
            {
               // instead of lerping colors, we lerp just the weights
               Flt blend=AlphaLimitBlend(Flt(total_alpha_weight)/CWA8AlphaLimit), blend1=1-blend;
               // sample.rgb*sample.a*sample_weight/total_alpha_weight*blend + sample.rgb*sample_weight*((1-blend)/total_weight)
               // sample.rgb*sample_weight*(sample.a/total_alpha_weight*blend + (1-blend)/total_weight)
               Int new_scale=INT_MAX/256/2; // "INT_MAX/256" was not enough, because this test failed: Long sum=0; REPD(y, 8)REPD(x, 8)sum+=255*Max(0, w[y][x]); if(sum+new_scale/2>INT_MAX)Exit("fail");
               blend *=Flt(new_scale)/total_alpha_weight;
               blend1*=Flt(new_scale)/CWA8Sum;
               REPD(y, 8)
               REPD(x, 8)w[y][x]=Round(CWA8[y][x]*(c[y][x].a*blend + blend1));
               total_alpha_weight=new_scale;
            }
            Int round=total_alpha_weight>>1;
            col.r=Mid((/*c[0][0].r*w[0][0]  + c[0][1].r*w[0][1]*/+ c[0][2].r*w[0][2] + c[0][3].r*w[0][3] + c[0][4].r*w[0][4] + c[0][5].r*w[0][5] +/*c[0][6].r*w[0][6] +  c[0][7].r*w[0][7]*/
                     /*+ c[1][0].r*w[1][0]*/+ c[1][1].r*w[1][1]  + c[1][2].r*w[1][2] + c[1][3].r*w[1][3] + c[1][4].r*w[1][4] + c[1][5].r*w[1][5] +  c[1][6].r*w[1][6] +/*c[1][7].r*w[1][7]*/
                       + c[2][0].r*w[2][0]  + c[2][1].r*w[2][1]  + c[2][2].r*w[2][2] + c[2][3].r*w[2][3] + c[2][4].r*w[2][4] + c[2][5].r*w[2][5] +  c[2][6].r*w[2][6] +  c[2][7].r*w[2][7]
                       + c[3][0].r*w[3][0]  + c[3][1].r*w[3][1]  + c[3][2].r*w[3][2] + c[3][3].r*w[3][3] + c[3][4].r*w[3][4] + c[3][5].r*w[3][5] +  c[3][6].r*w[3][6] +  c[3][7].r*w[3][7]
                       + c[4][0].r*w[4][0]  + c[4][1].r*w[4][1]  + c[4][2].r*w[4][2] + c[4][3].r*w[4][3] + c[4][4].r*w[4][4] + c[4][5].r*w[4][5] +  c[4][6].r*w[4][6] +  c[4][7].r*w[4][7]
                       + c[5][0].r*w[5][0]  + c[5][1].r*w[5][1]  + c[5][2].r*w[5][2] + c[5][3].r*w[5][3] + c[5][4].r*w[5][4] + c[5][5].r*w[5][5] +  c[5][6].r*w[5][6] +  c[5][7].r*w[5][7]
                     /*+ c[6][0].r*w[6][0]*/+ c[6][1].r*w[6][1]  + c[6][2].r*w[6][2] + c[6][3].r*w[6][3] + c[6][4].r*w[6][4] + c[6][5].r*w[6][5] +  c[6][6].r*w[6][6] +/*c[6][7].r*w[6][7]*/
                     /*+ c[7][0].r*w[7][0]  + c[7][1].r*w[7][1]*/+ c[7][2].r*w[7][2] + c[7][3].r*w[7][3] + c[7][4].r*w[7][4] + c[7][5].r*w[7][5] +/*c[7][6].r*w[7][6] +  c[7][7].r*w[7][7]*/ + round)/total_alpha_weight, 0, 255);
            col.g=Mid((/*c[0][0].g*w[0][0]  + c[0][1].g*w[0][1]*/+ c[0][2].g*w[0][2] + c[0][3].g*w[0][3] + c[0][4].g*w[0][4] + c[0][5].g*w[0][5] +/*c[0][6].g*w[0][6] +  c[0][7].g*w[0][7]*/
                     /*+ c[1][0].g*w[1][0]*/+ c[1][1].g*w[1][1]  + c[1][2].g*w[1][2] + c[1][3].g*w[1][3] + c[1][4].g*w[1][4] + c[1][5].g*w[1][5] +  c[1][6].g*w[1][6] +/*c[1][7].g*w[1][7]*/
                       + c[2][0].g*w[2][0]  + c[2][1].g*w[2][1]  + c[2][2].g*w[2][2] + c[2][3].g*w[2][3] + c[2][4].g*w[2][4] + c[2][5].g*w[2][5] +  c[2][6].g*w[2][6] +  c[2][7].g*w[2][7]
                       + c[3][0].g*w[3][0]  + c[3][1].g*w[3][1]  + c[3][2].g*w[3][2] + c[3][3].g*w[3][3] + c[3][4].g*w[3][4] + c[3][5].g*w[3][5] +  c[3][6].g*w[3][6] +  c[3][7].g*w[3][7]
                       + c[4][0].g*w[4][0]  + c[4][1].g*w[4][1]  + c[4][2].g*w[4][2] + c[4][3].g*w[4][3] + c[4][4].g*w[4][4] + c[4][5].g*w[4][5] +  c[4][6].g*w[4][6] +  c[4][7].g*w[4][7]
                       + c[5][0].g*w[5][0]  + c[5][1].g*w[5][1]  + c[5][2].g*w[5][2] + c[5][3].g*w[5][3] + c[5][4].g*w[5][4] + c[5][5].g*w[5][5] +  c[5][6].g*w[5][6] +  c[5][7].g*w[5][7]
                     /*+ c[6][0].g*w[6][0]*/+ c[6][1].g*w[6][1]  + c[6][2].g*w[6][2] + c[6][3].g*w[6][3] + c[6][4].g*w[6][4] + c[6][5].g*w[6][5] +  c[6][6].g*w[6][6] +/*c[6][7].g*w[6][7]*/
                     /*+ c[7][0].g*w[7][0]  + c[7][1].g*w[7][1]*/+ c[7][2].g*w[7][2] + c[7][3].g*w[7][3] + c[7][4].g*w[7][4] + c[7][5].g*w[7][5] +/*c[7][6].g*w[7][6] +  c[7][7].g*w[7][7]*/ + round)/total_alpha_weight, 0, 255);
            col.b=Mid((/*c[0][0].b*w[0][0]  + c[0][1].b*w[0][1]*/+ c[0][2].b*w[0][2] + c[0][3].b*w[0][3] + c[0][4].b*w[0][4] + c[0][5].b*w[0][5] +/*c[0][6].b*w[0][6] +  c[0][7].b*w[0][7]*/
                     /*+ c[1][0].b*w[1][0]*/+ c[1][1].b*w[1][1]  + c[1][2].b*w[1][2] + c[1][3].b*w[1][3] + c[1][4].b*w[1][4] + c[1][5].b*w[1][5] +  c[1][6].b*w[1][6] +/*c[1][7].b*w[1][7]*/
                       + c[2][0].b*w[2][0]  + c[2][1].b*w[2][1]  + c[2][2].b*w[2][2] + c[2][3].b*w[2][3] + c[2][4].b*w[2][4] + c[2][5].b*w[2][5] +  c[2][6].b*w[2][6] +  c[2][7].b*w[2][7]
                       + c[3][0].b*w[3][0]  + c[3][1].b*w[3][1]  + c[3][2].b*w[3][2] + c[3][3].b*w[3][3] + c[3][4].b*w[3][4] + c[3][5].b*w[3][5] +  c[3][6].b*w[3][6] +  c[3][7].b*w[3][7]
                       + c[4][0].b*w[4][0]  + c[4][1].b*w[4][1]  + c[4][2].b*w[4][2] + c[4][3].b*w[4][3] + c[4][4].b*w[4][4] + c[4][5].b*w[4][5] +  c[4][6].b*w[4][6] +  c[4][7].b*w[4][7]
                       + c[5][0].b*w[5][0]  + c[5][1].b*w[5][1]  + c[5][2].b*w[5][2] + c[5][3].b*w[5][3] + c[5][4].b*w[5][4] + c[5][5].b*w[5][5] +  c[5][6].b*w[5][6] +  c[5][7].b*w[5][7]
                     /*+ c[6][0].b*w[6][0]*/+ c[6][1].b*w[6][1]  + c[6][2].b*w[6][2] + c[6][3].b*w[6][3] + c[6][4].b*w[6][4] + c[6][5].b*w[6][5] +  c[6][6].b*w[6][6] +/*c[6][7].b*w[6][7]*/
                     /*+ c[7][0].b*w[7][0]  + c[7][1].b*w[7][1]*/+ c[7][2].b*w[7][2] + c[7][3].b*w[7][3] + c[7][4].b*w[7][4] + c[7][5].b*w[7][5] +/*c[7][6].b*w[7][6] +  c[7][7].b*w[7][7]*/ + round)/total_alpha_weight, 0, 255);
         }
         dest.color(x, y, col);
      }
   }
   static void Downsize2xCubicFastSmooth(IntPtr elm_index, CopyContext &ctx, Int thread_index) {ctx.downsize2xCubicFastSmooth(elm_index);}
          void downsize2xCubicFastSmooth(Int y)
   {
      Int yc[8]; yc[3]=y*2; // 'y[3]' is always OK
      if(clamp){yc[0]=Max(yc[3]-3, 0       ); yc[1]=Max(yc[3]-2, 0       ); yc[2]=Max(yc[3]-1, 0       ); yc[4]=Min(yc[3]+1, src.lh()-1); yc[5]=Min(yc[3]+2, src.lh()-1); yc[6]=Min(yc[3]+3, src.lh()-1); yc[7]=Min(yc[3]+4, src.lh()-1);}
      else     {yc[0]=Mod(yc[3]-3, src.lh()); yc[1]=Mod(yc[3]-2, src.lh()); yc[2]=Mod(yc[3]-1, src.lh()); yc[4]=   (yc[3]+1)%src.lh()   ; yc[5]=   (yc[3]+2)%src.lh()   ; yc[6]=   (yc[3]+3)%src.lh()   ; yc[7]=   (yc[3]+4)%src.lh()   ;}
      Byte *dest_data_y=dest.data()+y*dest.pitch();
      Byte *dest_data_x=dest_data_y; FREPD(x, dest.lw()) // iterate forward so we can increase pointers
      {
         Int xc[8]; xc[3]=x*2; // 'x[3]' is always OK
         if(clamp){xc[0]=Max(xc[3]-3, 0       ); xc[1]=Max(xc[3]-2, 0       ); xc[2]=Max(xc[3]-1, 0       ); xc[4]=Min(xc[3]+1, src.lw()-1); xc[5]=Min(xc[3]+2, src.lw()-1); xc[6]=Min(xc[3]+3, src.lw()-1); xc[7]=Min(xc[3]+4, src.lw()-1);}
         else     {xc[0]=Mod(xc[3]-3, src.lw()); xc[1]=Mod(xc[3]-2, src.lw()); xc[2]=Mod(xc[3]-1, src.lw()); xc[4]=   (xc[3]+1)%src.lw()   ; xc[5]=   (xc[3]+2)%src.lw()   ; xc[6]=   (xc[3]+3)%src.lw()   ; xc[7]=   (xc[3]+4)%src.lw()   ;}
         Vec rgb=0; Vec4 color=0, c[8][8]; // [y][x]
         src.gatherL(&c[0][0], xc, Elms(xc), yc, Elms(yc));
         REPD(x, 8)
         REPD(y, 8)if(Flt w=CFSMW8[y][x])Add(color, rgb, c[y][x], w, alpha_weight);
         Normalize(color, rgb, alpha_weight, alpha_limit);
         if(manual_linear_to_srgb)color.xyz=LinearToSRGB(color.xyz);
         set_color(dest_data_x, dest.type(), dest.hwType(), color);
         dest_data_x+=dest.bytePP();
      }
   }
   static void DownsizeArea(IntPtr elm_index, CopyContext &ctx, Int thread_index) {ctx.downsizeArea(elm_index);}
          void downsizeArea(Int y)
   {
      Vec2 pos;
      pos.y=y*y_mul_add.x+y_mul_add.y;
      Byte *dest_data_y=dest.data()+y*dest.pitch();
      Byte *dest_data_x=dest_data_y; FREPD(x, dest.lw()) // iterate forward so we can increase pointers
      {
         pos.x=x*x_mul_add.x+x_mul_add.y;
         Vec4 color=(src.*ptr_area_color)(pos, area_size.xy, clamp, alpha_weight);
         if(manual_linear_to_srgb)color.xyz=LinearToSRGB(color.xyz);
         set_color(dest_data_x, dest.type(), dest.hwType(), color);
         dest_data_x+=dest.bytePP();
      }
   }
   static void UpsizeCubicPlus(IntPtr elm_index, CopyContext &ctx, Int thread_index) {ctx.upsizeCubicPlus(elm_index);}
          void upsizeCubicPlus(IntPtr elm_index)
   {
      Int z=elm_index/dest.lh(),
          y=elm_index%dest.lh();
      Byte *dest_data_z=dest.data()+z*dest.pitch2();
      Byte *dest_data_y=dest_data_z+y*dest.pitch ();
      Byte *dest_data_x=dest_data_y;
      Flt   sy=y*y_mul_add.x+y_mul_add.y,
            sx=/*x*x_mul_add.x+*/x_mul_add.y; // 'x' is zero at this step so ignore it
      Int   xo[CUBIC_PLUS_SAMPLES*2], yo[CUBIC_PLUS_SAMPLES*2], xi=Floor(sx), yi=Floor(sy);
      Flt   yw[CUBIC_PLUS_SAMPLES*2];
      REPA( xo)
      {
         xo[i]=xi-CUBIC_PLUS_SAMPLES+1+i;
         yo[i]=yi-CUBIC_PLUS_SAMPLES+1+i; yw[i]=Sqr(sy-yo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, src.lw()-1);
            Clamp(yo[i], 0, src.lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], src.lw());
            yo[i]=Mod(yo[i], src.lh());
         }
      }
      if(NeedMultiChannel(src.type(), dest.type()))
      {
         Vec4 c[CUBIC_PLUS_SAMPLES*2][CUBIC_PLUS_SAMPLES*2];
         src.gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
         REPD(x, CUBIC_PLUS_SAMPLES*2)REPD(y, x)Swap(c[y][x], c[x][y]); // convert [y][x] -> [x][y] so we can use later 'gather' to read a single column with new x

         Int x_offset=0;
         FREPD(x, dest.lw()) // iterate forward so we can increase pointers
         {
            Flt sx=x*x_mul_add.x+x_mul_add.y;
            Int xi2=Floor(sx); if(xi!=xi2)
            {
               xi=xi2;
               Int xo_last=xi+CUBIC_PLUS_SAMPLES; if(clamp)Clamp(xo_last, 0, src.lw()-1);else xo_last=Mod(xo_last, src.lw());
               src.gather(&c[x_offset][0], &xo_last, 1, yo, Elms(yo)); // read new column
               x_offset=(x_offset+1)%(CUBIC_PLUS_SAMPLES*2);
            }

            Flt  weight=0;
            Vec  rgb   =0;
            Vec4 color =0;
            REPAD(x, xo)
            {
               Int xc=(x+x_offset)%(CUBIC_PLUS_SAMPLES*2);
               Flt xw=Sqr(sx-(xi-CUBIC_PLUS_SAMPLES+1+x));
               REPAD(y, yo)
               {
                  Flt w=xw+yw[y]; if(w<Sqr(CUBIC_PLUS_RANGE))
                  {
                     w=Weight(w*Sqr(CUBIC_PLUS_SHARPNESS)); Add(color, rgb, c[xc][y], w, alpha_weight); weight+=w;
                  }
               }
            }
            Normalize(color, rgb, weight, alpha_weight, alpha_limit);
            SetColor(dest_data_x, dest.type(), dest.hwType(), color);
            dest_data_x+=dest.bytePP();
         }
      }else
      {
         Flt v[CUBIC_PLUS_SAMPLES*2][CUBIC_PLUS_SAMPLES*2];
         src.gather(&v[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
         REPD(x, CUBIC_PLUS_SAMPLES*2)REPD(y, x)Swap(v[y][x], v[x][y]); // convert [y][x] -> [x][y] so we can use later 'gather' to read a single column with new x

         Int x_offset=0;
         FREPD(x, dest.lw()) // iterate forward so we can increase pointers
         {
            Flt sx=x*x_mul_add.x+x_mul_add.y;
            Int xi2=Floor(sx); if(xi!=xi2)
            {
               xi=xi2;
               Int xo_last=xi+CUBIC_PLUS_SAMPLES; if(clamp)Clamp(xo_last, 0, src.lw()-1);else xo_last=Mod(xo_last, src.lw());
               src.gather(&v[x_offset][0], &xo_last, 1, yo, Elms(yo)); // read new column
               x_offset=(x_offset+1)%(CUBIC_PLUS_SAMPLES*2);
            }

            Flt weight=0, value=0;
            REPAD(x, xo)
            {
               Int xc=(x+x_offset)%(CUBIC_PLUS_SAMPLES*2);
               Flt xw=Sqr(sx-(xi-CUBIC_PLUS_SAMPLES+1+x));
               REPAD(y, yo)
               {
                  Flt w=xw+yw[y]; if(w<Sqr(CUBIC_PLUS_RANGE))
                  {
                     w=Weight(w*Sqr(CUBIC_PLUS_SHARPNESS)); value+=v[xc][y]*w; weight+=w;
                  }
               }
            }
            SetPixelF(dest_data_x, dest.hwType(), value/weight);
            dest_data_x+=dest.bytePP();
         }
      }
   }
   static void UpsizeCubicFast(IntPtr elm_index, CopyContext &ctx, Int thread_index) {ctx.upsizeCubicFast(elm_index);}
          void upsizeCubicFast(IntPtr elm_index)
   {
      Int z=elm_index/dest.lh(),
          y=elm_index%dest.lh();
      Byte *dest_data_z=dest.data()+z*dest.pitch2();
      Byte *dest_data_y=dest_data_z+y*dest.pitch ();
      Byte *dest_data_x=dest_data_y;
      Flt   sy=y*y_mul_add.x+y_mul_add.y,
            sx=/*x*x_mul_add.x+*/x_mul_add.y; // 'x' is zero at this step so ignore it
      Int   xo[CUBIC_FAST_SAMPLES*2], yo[CUBIC_FAST_SAMPLES*2], xi=Floor(sx), yi=Floor(sy);
      Flt   yw[CUBIC_FAST_SAMPLES*2];
      REPA( xo)
      {
         xo[i]=xi-CUBIC_FAST_SAMPLES+1+i;
         yo[i]=yi-CUBIC_FAST_SAMPLES+1+i; yw[i]=Sqr(sy-yo[i]);
         if(clamp)
         {
            Clamp(xo[i], 0, src.lw()-1);
            Clamp(yo[i], 0, src.lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], src.lw());
            yo[i]=Mod(yo[i], src.lh());
         }
      }
      if(NeedMultiChannel(src.type(), dest.type()))
      {
         Vec4 c[CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
         src.gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
         REPD(x, CUBIC_FAST_SAMPLES*2)REPD(y, x)Swap(c[y][x], c[x][y]); // convert [y][x] -> [x][y] so we can use later 'gather' to read a single column with new x

         Int x_offset=0;
         FREPD(x, dest.lw()) // iterate forward so we can increase pointers
         {
            Flt sx=x*x_mul_add.x+x_mul_add.y;
            Int xi2=Floor(sx); if(xi!=xi2)
            {
               xi=xi2;
               Int xo_last=xi+CUBIC_FAST_SAMPLES; if(clamp)Clamp(xo_last, 0, src.lw()-1);else xo_last=Mod(xo_last, src.lw());
               src.gather(&c[x_offset][0], &xo_last, 1, yo, Elms(yo)); // read new column
               x_offset=(x_offset+1)%(CUBIC_FAST_SAMPLES*2);
            }

            Flt  weight=0;
            Vec  rgb   =0;
            Vec4 color =0;
            REPAD(x, xo)
            {
               Int xc=(x+x_offset)%(CUBIC_FAST_SAMPLES*2);
               Flt xw=Sqr(sx-(xi-CUBIC_FAST_SAMPLES+1+x));
               REPAD(y, yo)
               {
                  Flt w=xw+yw[y]; if(w<Sqr(CUBIC_FAST_RANGE))
                  {
                     w=Weight(w); Add(color, rgb, c[xc][y], w, alpha_weight); weight+=w;
                  }
               }
            }
            Normalize(color, rgb, weight, alpha_weight, alpha_limit);
            SetColor(dest_data_x, dest.type(), dest.hwType(), color);
            dest_data_x+=dest.bytePP();
         }
      }else
      {
         Flt v[CUBIC_FAST_SAMPLES*2][CUBIC_FAST_SAMPLES*2];
         src.gather(&v[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
         REPD(x, CUBIC_FAST_SAMPLES*2)REPD(y, x)Swap(v[y][x], v[x][y]); // convert [y][x] -> [x][y] so we can use later 'gather' to read a single column with new x

         Int x_offset=0;
         FREPD(x, dest.lw()) // iterate forward so we can increase pointers
         {
            Flt sx=x*x_mul_add.x+x_mul_add.y;
            Int xi2=Floor(sx); if(xi!=xi2)
            {
               xi=xi2;
               Int xo_last=xi+CUBIC_FAST_SAMPLES; if(clamp)Clamp(xo_last, 0, src.lw()-1);else xo_last=Mod(xo_last, src.lw());
               src.gather(&v[x_offset][0], &xo_last, 1, yo, Elms(yo)); // read new column
               x_offset=(x_offset+1)%(CUBIC_FAST_SAMPLES*2);
            }

            Flt weight=0, value=0;
            REPAD(x, xo)
            {
               Int xc=(x+x_offset)%(CUBIC_FAST_SAMPLES*2);
               Flt xw=Sqr(sx-(xi-CUBIC_FAST_SAMPLES+1+x));
               REPAD(y, yo)
               {
                  Flt w=xw+yw[y]; if(w<Sqr(CUBIC_FAST_RANGE))
                  {
                     w=Weight(w); value+=v[xc][y]*w; weight+=w;
                  }
               }
            }
            SetPixelF(dest_data_x, dest.hwType(), value/weight);
            dest_data_x+=dest.bytePP();
         }
      }
   }
   static void UpsizeLinear(IntPtr elm_index, CopyContext &ctx, Int thread_index) {ctx.upsizeLinear(elm_index);}
          void upsizeLinear(IntPtr elm_index)
   {
      Int z=elm_index/dest.lh(),
          y=elm_index%dest.lh();
      Byte *dest_data_z=dest.data()+z*dest.pitch2();
      Byte *dest_data_y=dest_data_z+y*dest.pitch ();
      Byte *dest_data_x=dest_data_y;
      Flt   sy=y*y_mul_add.x+y_mul_add.y,
            sx=/*x*x_mul_add.x+*/x_mul_add.y; // 'x' is zero at this step so ignore it
      Int   xo[2], yo[2], xi=Floor(sx), yi=Floor(sy);
      Flt   yw[2]; yw[1]=sy-yi; yw[0]=1-yw[1];
      REPA( xo)
      {
         xo[i]=xi+i;
         yo[i]=yi+i;
         if(clamp)
         {
            Clamp(xo[i], 0, src.lw()-1);
            Clamp(yo[i], 0, src.lh()-1);
         }else
         {
            xo[i]=Mod(xo[i], src.lw());
            yo[i]=Mod(yo[i], src.lh());
         }
      }
      if(NeedMultiChannel(src.type(), dest.type()))
      {
         Vec4 c[2][2];
         src.gather(&c[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
         REPD(x, 2)REPD(y, x)Swap(c[y][x], c[x][y]); // convert [y][x] -> [x][y] so we can use later 'gather' to read a single column with new x

         Int x_offset=0;
         FREPD(x, dest.lw()) // iterate forward so we can increase pointers
         {
            Flt sx=x*x_mul_add.x+x_mul_add.y;
            Int xi2=Floor(sx); if(xi!=xi2)
            {
               xi=xi2;
               Int xo_last=xi+1; if(clamp)Clamp(xo_last, 0, src.lw()-1);else xo_last=Mod(xo_last, src.lw());
               src.gather(&c[x_offset][0], &xo_last, 1, yo, Elms(yo)); // read new column
               x_offset^=1;
            }

            Vec  rgb  =0;
            Vec4 color=0;
            Flt  xw[2]; xw[1]=sx-xi; xw[0]=1-xw[1];
            REPAD(x, xo)
            {
               Int xc=(x+x_offset)&1;
               REPAD(y, yo)Add(color, rgb, c[xc][y], xw[x]*yw[y], alpha_weight);
            }
            Normalize(color, rgb, alpha_weight, alpha_limit);
            SetColor(dest_data_x, dest.type(), dest.hwType(), color);
            dest_data_x+=dest.bytePP();
         }
      }else
      {
         Flt v[2][2];
         src.gather(&v[0][0], xo, Elms(xo), yo, Elms(yo)); // [y][x]
         REPD(x, 2)REPD(y, x)Swap(v[y][x], v[x][y]); // convert [y][x] -> [x][y] so we can use later 'gather' to read a single column with new x

         Int x_offset=0;
         FREPD(x, dest.lw()) // iterate forward so we can increase pointers
         {
            Flt sx=x*x_mul_add.x+x_mul_add.y;
            Int xi2=Floor(sx); if(xi!=xi2)
            {
               xi=xi2;
               Int xo_last=xi+1; if(clamp)Clamp(xo_last, 0, src.lw()-1);else xo_last=Mod(xo_last, src.lw());
               src.gather(&v[x_offset][0], &xo_last, 1, yo, Elms(yo)); // read new column
               x_offset^=1;
            }

            Flt value=0, xw[2]; xw[1]=sx-xi; xw[0]=1-xw[1];
            REPAD(x, xo)
            {
               Int xc=(x+x_offset)&1;
               REPAD(y, yo)value+=v[xc][y]*xw[x]*yw[y];
            }
            SetPixelF(dest_data_x, dest.hwType(), value);
            dest_data_x+=dest.bytePP();
         }
      }
   }

   static void Upsize(IntPtr elm_index, CopyContext &ctx, Int thread_index) {ctx.upsize(elm_index);}
          void upsize(IntPtr elm_index)
   {
      Int z=elm_index/dest.lh(),
          y=elm_index%dest.lh();
      Flt sz=z*z_mul_add.x+z_mul_add.y;
      Flt sy=y*y_mul_add.x+y_mul_add.y;
      Byte *dest_data_z=dest.data()+z*dest.pitch2();
      Byte *dest_data_y=dest_data_z+y*dest.pitch ();
      Byte *dest_data_x=dest_data_y;
      if(NeedMultiChannel(src.type(), dest.type()))
      {
         FREPD(x, dest.lw()) // iterate forward so we can increase pointers
         {
            Flt  sx=x*x_mul_add.x+x_mul_add.y;
            Vec4 color=((src.ld()<=1) ? (src.*ptr_color   )(sx, sy,     clamp, alpha_weight)
                                      : (src.*ptr_color_3D)(sx, sy, sz, clamp, alpha_weight));
            SetColor(dest_data_x, dest.type(), dest.hwType(), color);
            dest_data_x+=dest.bytePP();
         }
      }else // single channel
      {
         FREPD(x, dest.lw()) // iterate forward so we can increase pointers
         {
            Flt sx=x*x_mul_add.x+x_mul_add.y;
            Flt pix=((src.ld()<=1) ? (src.*ptr_pixel   )(sx, sy,     clamp)
                                   : (src.*ptr_pixel_3D)(sx, sy, sz, clamp));
            SetPixelF(dest_data_x, dest.hwType(), pix);
            dest_data_x+=dest.bytePP();
         }
      }
   }

   CopyContext(C Image &src, Image &dest, FILTER_TYPE filter, UInt flags) : src(src), dest(dest),
      clamp(IcClamp(flags)),
      keep_edges    (FlagOn(flags, IC_KEEP_EDGES)),
      alpha_weight  (FlagOn(flags, IC_ALPHA_WEIGHT) && src.typeInfo().a), // only if source has alpha
      no_alpha_limit(FlagOn(flags, IC_NO_ALPHA_LIMIT)),
      src_srgb(src.sRGB()), dest_srgb(dest.sRGB()),
      ignore_gamma(IgnoreGamma(flags, src.hwType(), dest.hwType())),

   /* When downsampling, some filters operate in linear gamma (to preserve brightness) and end up with linear gamma result
         (However some sharpening filters don't do this, because serious artifacts occur)

      Vec4 linear_color;
      if(linear_gamma)
      {
         if(ignore_gamma) // we don't want to convert gamma
         {
            if(src_srgb)linear_color.xyz=LinearToSRGB(linear_color.xyz); // source is sRGB however we have 'linear_color', so convert it back to sRGB
               dest.colorF(x, y, linear_color);
         }else dest.colorL(x, y, linear_color); // write 'linear_color', 'colorL' will perform gamma conversion
      }

      However if dest is sRGB then 'dest.colorL' will already call 'LinearToSRGB' inside (potentially faster for Byte types).
      So if 'src_srgb' and we have to do 'LinearToSRGB', and dest is sRGB then we can just skip 'ignore_gamma' and call 'dest.colorL' */
      ignore_gamma_ds(ignore_gamma && !(src_srgb && dest_srgb)), // if both are sRGB then disable 'ignore_gamma_ds'

      src_high_prec(src.highPrecision()), high_prec((src_high_prec && dest.highPrecision()) || !ignore_gamma),
      filter(filter),
      src_faces1(src.faces()-1),
      flags(flags),
      SetColor(ignore_gamma ? SetColorF : src_srgb ? SetColorS : SetColorL) // pointer to function, when resizing we operate on source native gamma, so if source is sRGB then we're setting sRGB color, and if linear then linear
   {}
   Bool process(Int max_mip_maps, Flt sharp_smooth)
   {
      REPD(mip , Min(src.mipMaps(), dest.mipMaps(), max_mip_maps))
      REPD(face, dest.faces())
      {
         if(! src.lockRead(            mip, (DIR_ENUM)Min(face, src_faces1)))               return false;
         if(!dest.lock    (LOCK_WRITE, mip, (DIR_ENUM)    face             )){src.unlock(); return false;}

         if(src.size3()==dest.size3()) // no resize
         {
            if(CanDoRawCopy(src, dest, ignore_gamma)) // no retype
            {
               CopyImgData(src.data(), dest.data(), src.pitch(), dest.pitch(), src.softBlocksY(mip), dest.softBlocksY(mip), src.pitch2(), dest.pitch2(), src.ld(), dest.ld());
            }else // retype
            {
               IMAGE_TYPE src_hwType= src.hwType(),//src_type= src.type(),
                         dest_hwType=dest.hwType(), dest_type=dest.type();
               if(ignore_gamma)
               {
                   src_hwType=ImageTypeExcludeSRGB( src_hwType);
                  dest_hwType=ImageTypeExcludeSRGB(dest_hwType);
                 //src_type  =ImageTypeExcludeSRGB( src_type  );
                  dest_type  =ImageTypeExcludeSRGB(dest_type  );
               }

               // RGB -> RGBA, very common case for importing images
               if(src_hwType==IMAGE_R8G8B8      && dest_hwType==IMAGE_R8G8B8A8      && dest_type==IMAGE_R8G8B8A8       // check 'type' too in case we have to perform color adjustment
               || src_hwType==IMAGE_R8G8B8_SRGB && dest_hwType==IMAGE_R8G8B8A8_SRGB && dest_type==IMAGE_R8G8B8A8_SRGB) // check 'type' too in case we have to perform color adjustment
               {
                  FREPD(z, src.ld())
                  {
                   C Byte *sz=src .data() + z*src .pitch2();
                     Byte *dz=dest.data() + z*dest.pitch2();
                     FREPD(y, src.lh())
                     {
                      C VecB  *s=(VecB *)(sz + y*src .pitch());
                        VecB4 *d=(VecB4*)(dz + y*dest.pitch());
                        REPD(x, src.lw()){(d++)->set(s->x, s->y, s->z, 255); s++;}
                     }
                  }
               }else
               // RGB -> BGRA, very common case for exporting to WEBP from RGB
               if(src_hwType==IMAGE_R8G8B8      && dest_hwType==IMAGE_B8G8R8A8      && dest_type==IMAGE_B8G8R8A8       // check 'type' too in case we have to perform color adjustment
               || src_hwType==IMAGE_R8G8B8_SRGB && dest_hwType==IMAGE_B8G8R8A8_SRGB && dest_type==IMAGE_B8G8R8A8_SRGB) // check 'type' too in case we have to perform color adjustment
               {
                  FREPD(z, src.ld())
                  {
                   C Byte *sz=src .data() + z*src .pitch2();
                     Byte *dz=dest.data() + z*dest.pitch2();
                     FREPD(y, src.lh())
                     {
                      C VecB  *s=(VecB *)(sz + y*src .pitch());
                        VecB4 *d=(VecB4*)(dz + y*dest.pitch());
                        REPD(x, src.lw()){(d++)->set(s->z, s->y, s->x, 255); s++;}
                     }
                  }
               }else
               // RGBA -> BGRA, very common case for exporting to WEBP from RGBA
               if(src_hwType==IMAGE_R8G8B8A8      && dest_hwType==IMAGE_B8G8R8A8      && dest_type==IMAGE_B8G8R8A8       // check 'type' too in case we have to perform color adjustment
               || src_hwType==IMAGE_R8G8B8A8_SRGB && dest_hwType==IMAGE_B8G8R8A8_SRGB && dest_type==IMAGE_B8G8R8A8_SRGB) // check 'type' too in case we have to perform color adjustment
               {
                  FREPD(z, src.ld())
                  {
                   C Byte *sz=src .data() + z*src .pitch2();
                     Byte *dz=dest.data() + z*dest.pitch2();
                     FREPD(y, src.lh())
                     {
                      C VecB4 *s=(VecB4*)(sz + y*src .pitch());
                        VecB4 *d=(VecB4*)(dz + y*dest.pitch());
                        REPD(x, src.lw()){(d++)->set(s->z, s->y, s->x, s->w); s++;}
                     }
                  }
               }else
               if(!ignore_gamma)
               {
                  REPD(z, src.ld())
                  REPD(y, src.lh())
                  REPD(x, src.lw())dest.color3DL(x, y, z, src.color3DL(x, y, z));
               }else
               if(high_prec) // high precision requires FP
               {
                  REPD(z, src.ld())
                  REPD(y, src.lh())
                  REPD(x, src.lw())dest.color3DF(x, y, z, src.color3DF(x, y, z));
               }else
               {
                  REPD(z, src.ld())
                  REPD(y, src.lh())
                  REPD(x, src.lw())dest.color3D(x, y, z, src.color3D(x, y, z));
               }
            }
         }else
         if(filter==FILTER_NO_STRETCH)
         {
            CopyNoStretch(src, dest, clamp, ignore_gamma);
         }else // resize
         {
            // check for optimized downscale
            if(dest.lw()==Max(1, src.lw()>>1)
            && dest.lh()==Max(1, src.lh()>>1)
            && dest.ld()==Max(1, src.ld()>>1) // 2x downsample
            && !keep_edges
            && (Equal(sharp_smooth, 1) || filter==FILTER_NONE)
            )
            {
               if(src.ld()<=1) // 2D
               {
                  switch(filter)
                  {
                     case FILTER_NONE: REPD(y, dest.lh())
                     {
                        Int yc=y*2;
                        if(!ignore_gamma)REPD(x, dest.lw())dest.colorL(x, y, src.colorL(x*2, yc));else
                        if( high_prec   )REPD(x, dest.lw())dest.colorF(x, y, src.colorF(x*2, yc));else
                                         REPD(x, dest.lw())dest.color (x, y, src.color (x*2, yc));
                     }goto finish;

                     case FILTER_LINEAR: // this operates on linear gamma
                     {
                        hp=high_prec|src_srgb; // for FILTER_LINEAR down-sampling always operate on linear gamma to preserve brightness, so if source is not linear (sRGB) then we need high precision
                        manual_linear_to_srgb=(ignore_gamma_ds && src_srgb); // source is sRGB however we have linear color, so convert it back to sRGB
                        set_color=(ignore_gamma_ds ? SetColorF : SetColorL); // pointer to function
                        ImageThreads.init().process(dest.lh(), Downsize2xLinear, T);
                     }goto finish;

                     case FILTER_WAIFU: // there's no downscale for Waifu, so fall back to best available
                     case FILTER_BEST:
                     case FILTER_CUBIC_FAST_SHARP: ASSERT(FILTER_DOWN==FILTER_CUBIC_FAST_SHARP); // this operates on source native gamma
                     {
                      //high_prec|=src_srgb; FILTER_CUBIC_FAST_SHARP is not suitable for linear gamma (artifacts happen due to sharpening)
                        if(!high_prec)
                        {
                           ImageThreads.init().process(dest.lh(), Downsize2xCubicFastSharp, T);
                           goto finish;
                        }
                     }break;

                     case FILTER_CUBIC_FAST_SMOOTH: // used by 'transparentToNeighbor', this operates on linear gamma
                     {
                      //high_prec|=src_srgb; not needed since we always do high prec here
                        manual_linear_to_srgb=(ignore_gamma_ds && src_srgb); // source is sRGB however we have linear color, so convert it back to sRGB
                        set_color=(ignore_gamma_ds ? SetColorF : SetColorL); // pointer to function
                        alpha_limit=(no_alpha_limit ? ALPHA_LIMIT_NONE : ALPHA_LIMIT_CUBIC_FAST_SMOOTH);
                        ImageThreads.init().process(dest.lh(), Downsize2xCubicFastSmooth, T);
                     }goto finish;
                  } // switch(filter)
               }else // 3D
               {
                  switch(filter)
                  {
                     case FILTER_NONE: REPD(z, dest.ld())
                     {
                        Int zc=z*2; REPD(y, dest.lh())
                        {
                           Int yc=y*2;
                           if(!ignore_gamma)REPD(x, dest.lw())dest.color3DL(x, y, z, src.color3DL(x*2, yc, zc));else
                           if( high_prec   )REPD(x, dest.lw())dest.color3DF(x, y, z, src.color3DF(x*2, yc, zc));else
                                            REPD(x, dest.lw())dest.color3D (x, y, z, src.color3D (x*2, yc, zc));
                        }
                     }goto finish;

                     case FILTER_LINEAR: // this operates on linear gamma
                     {
                        hp=high_prec|src_srgb; // for FILTER_LINEAR down-sampling always operate on linear gamma to preserve brightness, so if source is not linear (sRGB) then we need high precision
                        if(!hp)
                        {
                           ImageThreads.init().process(dest.lh()*dest.ld(), Downsize2xLinear3D, T);
                           goto finish;
                        }
                     }break;
                  }
               }
            }
         
            // any scale
            {
               // for scale 1->1 offset=0.0
               //           2->1 offset=0.5
               //           3->1 offset=1.0
               //           4->1 offset=1.5
               if(keep_edges)
               {
                  x_mul_add.set(Flt(src.lw()-1)/(dest.lw()-1), 0);
                  y_mul_add.set(Flt(src.lh()-1)/(dest.lh()-1), 0);
                  z_mul_add.set(Flt(src.ld()-1)/(dest.ld()-1), 0);
               }else
               {
                  x_mul_add.x=Flt(src.lw())/dest.lw(); x_mul_add.y=x_mul_add.x*0.5f-0.5f;
                  y_mul_add.x=Flt(src.lh())/dest.lh(); y_mul_add.y=y_mul_add.x*0.5f-0.5f;
                  z_mul_add.x=Flt(src.ld())/dest.ld(); z_mul_add.y=z_mul_add.x*0.5f-0.5f;
               }
               area_size.set(x_mul_add.x, y_mul_add.x, z_mul_add.x); area_size*=sharp_smooth;
               if(filter!=FILTER_NONE && (area_size.x>1+EPS || area_size.y>1+EPS) && src.ld()==1 && dest.ld()==1) // if we're downsampling (any scale is higher than 1) then we must use more complex 'areaColor*' methods
               {
                  Bool linear_gamma; // some down-sampling filters operate on linear gamma here
                  ptr_area_color=GetImageAreaColor(filter, linear_gamma);
                  manual_linear_to_srgb=false;
                  set_color=SetColor; // pointer to function
                  if(linear_gamma)
                  {
                     if(ignore_gamma_ds) // we don't want to convert gamma
                     {
                        if(src_srgb)manual_linear_to_srgb=true; // source is sRGB however we have linear color, so convert it back to sRGB
                           set_color=SetColorF;
                     }else set_color=SetColorL; // write linear color, 'SetColorL' will perform gamma conversion
                  }
                  ImageThreads.init().process(dest.lh(), DownsizeArea, T);
               }else
               if((filter==FILTER_BEST || filter==FILTER_WAIFU) && ResizeWaifu && (dest.lw()>src.lw() || dest.lh()>src.lh()) && src.ld()==1 && dest.ld()==1 && ResizeWaifu(src, dest, flags)){}else
               // !! Codes below operate on Source Image Native Gamma !! because upscaling sRGB images looks better if they're not sRGB, and linear images (such as normal maps) need linear anyway
               if((filter==FILTER_BEST || filter==FILTER_WAIFU || filter==FILTER_CUBIC_PLUS || filter==FILTER_CUBIC_PLUS_SHARP) // optimized Cubic+ upscale, check FILTER_BEST/FILTER_WAIFU again in case Waifu was not available
               && src.ld()==1)
               {
                  alpha_limit=(no_alpha_limit ? ALPHA_LIMIT_NONE : (filter==FILTER_CUBIC_PLUS_SHARP) ? ALPHA_LIMIT_CUBIC_PLUS_SHARP : ALPHA_LIMIT_CUBIC_PLUS);
                  Weight     =(                                    (filter==FILTER_CUBIC_PLUS_SHARP) ? CubicPlusSharp2              : CubicPlus2            ); ASSERT(CUBIC_PLUS_SAMPLES==CUBIC_PLUS_SHARP_SAMPLES && CUBIC_PLUS_RANGE==CUBIC_PLUS_SHARP_RANGE && CUBIC_PLUS_SHARPNESS==CUBIC_PLUS_SHARP_SHARPNESS);
                  ImageThreads.init().process(dest.lh()*dest.ld(), UpsizeCubicPlus, T);
               }else
               if((filter==FILTER_CUBIC_FAST || filter==FILTER_CUBIC_FAST_SMOOTH || filter==FILTER_CUBIC_FAST_SHARP) // optimized CubicFast upscale
               && src.ld()==1)
               {
                  alpha_limit=(no_alpha_limit ? ALPHA_LIMIT_NONE : (filter==FILTER_CUBIC_FAST) ? ALPHA_LIMIT_CUBIC_FAST : (filter==FILTER_CUBIC_FAST_SMOOTH) ? ALPHA_LIMIT_CUBIC_FAST_SMOOTH : ALPHA_LIMIT_CUBIC_FAST_SHARP);
                  Weight     =(                                    (filter==FILTER_CUBIC_FAST) ? CubicFast2             : (filter==FILTER_CUBIC_FAST_SMOOTH) ? CubicFastSmooth2              : CubicFastSharp2             );
                  ImageThreads.init().process(dest.lh()*dest.ld(), UpsizeCubicFast, T);
               }else
               if(filter==FILTER_LINEAR // optimized Linear upscale, this is used for Texture Sharpness calculation
               && src.ld()==1)
               {
                  alpha_limit=(no_alpha_limit ? ALPHA_LIMIT_NONE : ALPHA_LIMIT_LINEAR);
                  ImageThreads.init().process(dest.lh()*dest.ld(), UpsizeLinear, T);
               }else
               {
                  if(NeedMultiChannel(src.type(), dest.type()))
                  {
                     if(src.ld()<=1)ptr_color   =GetImageColorF  (filter); // 2D
                     else           ptr_color_3D=GetImageColor3DF(filter); // 3D
                  }else
                  {
                     if(src.ld()<=1)ptr_pixel   =GetImagePixelF  (filter); // 2D
                     else           ptr_pixel_3D=GetImagePixel3DF(filter); // 3D
                  }
                  ImageThreads.init().process(dest.lh()*dest.ld(), Upsize, T);
               }
            }
         }

      finish:
         dest.unlock();
          src.unlock();
      }
      return true;
   }
};
Bool Image::copySoft(Image &dest, FILTER_TYPE filter, UInt flags, Int max_mip_maps, Flt sharp_smooth)C // this does not support compressed images
{
   if(this==&dest)return true;
   return CopyContext(T, dest, filter, flags).process(max_mip_maps, sharp_smooth);
}
/******************************************************************************/
}
/******************************************************************************/
