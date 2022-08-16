/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
static Color ColorArray[]=
{
   RED   , GREEN, BLUE  ,
   YELLOW, CYAN , PURPLE,
   ORANGE, AZURE, BROWN ,
};
       Flt  ByteToFltArray[256], ByteSRGBToLinearArray[256], LinearByteToSRGBArray[256], *SRGBToDisplayArray=ByteToFltArray;
static Byte LinearToByteSRGBArray[3139], // 3139=smallest array size which preserves "LinearToByteSRGB(ByteSRGBToLinear(s))==s"
            SRGBToLinearByteArray[ 554]; //  554=smallest array size which preserves "SRGBToLinearByte(LinearByteToSRGB(l))==l"
/******************************************************************************/
Color::Color(C Vec &color)
{
   r=FltToByte(color.x);
   g=FltToByte(color.y);
   b=FltToByte(color.z);
   a=255;
}
Color::Color(C Vec4 &color)
{
   r=FltToByte(color.x);
   g=FltToByte(color.y);
   b=FltToByte(color.z);
   a=FltToByte(color.w);
}
Color::operator Vec()C
{
   return Vec(r/255.0f,
              g/255.0f,
              b/255.0f);
}
Color::operator Vec4()C
{
   return Vec4(r/255.0f,
               g/255.0f,
               b/255.0f,
               a/255.0f);
}
/******************************************************************************/
Color::Color(C VecB &color)
{
   r=color.x;
   g=color.y;
   b=color.z;
   a=255;
}
Color::Color(C VecB4 &color)
{
#if 0
   r=color.x;
   g=color.y;
   b=color.z;
   a=color.w;
#else
   u=color.u;
#endif
}
StrO Color::asText()C {StrO s; s.reserve(18); s+=r; s+=", "; s+=g; s+=", "; s+=b; s+=", "; s+=a; return s;} // 18 because of 4x"255" + 3x", " = 4*3 + 3*2 = 12 + 6 = 18
StrO Color::asHex ()C {ASSERT(SIZE(T)==4); return TextHexMem(this, SIZE(T), false);}
StrO Color::asHex3()C {ASSERT(SIZE(T)==4); return TextHexMem(this,       3, false);}

Bool Color::fromHex(C Str &t)
{
   ASSERT(SIZE(T)==4); return TextHexMem(t, this, SIZE(T));
}
/******************************************************************************/
Vec4 operator*(Flt f, C Color &c)
{
   f/=255;
   return Vec4(c.r*f,
               c.g*f,
               c.b*f,
               c.a*f);
}
Vec4 operator*(C Color &c, Flt f)
{
   f/=255;
   return Vec4(c.r*f,
               c.g*f,
               c.b*f,
               c.a*f);
}
/******************************************************************************/
Color ColorI(Int i) {return ColorArray[Mod(i, Elms(ColorArray))];}
/******************************************************************************/
Color ColorInverse(C Color &color)
{
   return Color(255-color.r, 255-color.g, 255-color.b, color.a);
}
Color ColorBrightness(Flt brightness)
{
   Byte b=FltToByte(brightness);
   return Color(b, b, b);
}
Color ColorBrightness(C Color &color, Flt brightness)
{
   return Color(Mid(RoundPos(color.r*brightness), 0, 255),
                Mid(RoundPos(color.g*brightness), 0, 255),
                Mid(RoundPos(color.b*brightness), 0, 255), color.a);
}
Color ColorBrightnessB(C Color &color, Byte brightness)
{
   return Color((color.r*brightness+128)/255,
                (color.g*brightness+128)/255,
                (color.b*brightness+128)/255, color.a);
}
Color ColorAlpha(Flt alpha)
{
   return Color(255, 255, 255, FltToByte(alpha));
}
Color ColorAlpha(C Color &color, Flt alpha)
{
   return Color(color.r, color.g, color.b, Mid(RoundPos(color.a*alpha), 0, 255));
}
Color ColorBA(Flt brightness, Flt alpha)
{
                  Byte b=FltToByte(brightness);
   return Color(b, b, b, FltToByte(alpha     ));
}
Color ColorBA(C Color &color, Flt brightness, Flt alpha)
{
   return Color(Mid(RoundPos(color.r*brightness), 0, 255),
                Mid(RoundPos(color.g*brightness), 0, 255),
                Mid(RoundPos(color.b*brightness), 0, 255),
                Mid(RoundPos(color.a*alpha     ), 0, 255));
}
Color ColorMul(Flt mul)
{
   Byte x=FltToByte(mul);
   return Color(x, x, x, x);
}
Color ColorMul(C Color &color, Flt mul)
{
   return Color(Mid(RoundPos(color.r*mul), 0, 255),
                Mid(RoundPos(color.g*mul), 0, 255),
                Mid(RoundPos(color.b*mul), 0, 255),
                Mid(RoundPos(color.a*mul), 0, 255));
}
Color ColorMul(C Color &c0, C Color &c1)
{
   return Color((c0.r*c1.r+128)/255,
                (c0.g*c1.g+128)/255,
                (c0.b*c1.b+128)/255,
                (c0.a*c1.a+128)/255);
}
Color ColorMul(C Color &c0, C Color &c1, C Color &c2)
{
   const UInt div=255*255, div_2=div/2;
   return Color((c0.r*c1.r*c2.r+div_2)/div,
                (c0.g*c1.g*c2.g+div_2)/div,
                (c0.b*c1.b*c2.b+div_2)/div,
                (c0.a*c1.a*c2.a+div_2)/div);
}
Color ColorAdd(C Color &c0, C Color &c1)
{
   return Color(Mid(c0.r+c1.r, 0, 255),
                Mid(c0.g+c1.g, 0, 255),
                Mid(c0.b+c1.b, 0, 255),
                Mid(c0.a+c1.a, 0, 255));
}
Color ColorAdd(C Color &c0, C Color &c1, C Color &c2)
{
   return Color(Mid(c0.r+c1.r+c2.r, 0, 255),
                Mid(c0.g+c1.g+c2.g, 0, 255),
                Mid(c0.b+c1.b+c2.b, 0, 255),
                Mid(c0.a+c1.a+c2.a, 0, 255));
}
Color ColorMulZeroAlpha(C Color &color, Flt mul)
{
   return Color(Mid(RoundPos(color.r*mul), 0, 255),
                Mid(RoundPos(color.g*mul), 0, 255),
                Mid(RoundPos(color.b*mul), 0, 255), 0);
}
Color ColorMulZeroAlpha(C Color &c0, C Color &c1)
{
   return Color((c0.r*c1.r+128)/255,
                (c0.g*c1.g+128)/255,
                (c0.b*c1.b+128)/255, 0);
}
/******************************************************************************/
Color Avg(C Color &c0, C Color &c1)
{
   return Color((c0.r+c1.r+1)>>1,
                (c0.g+c1.g+1)>>1,
                (c0.b+c1.b+1)>>1,
                (c0.a+c1.a+1)>>1);
}
Color Avg(C Color &c0, C Color &c1, C Color &c2)
{
   return Color((c0.r+c1.r+c2.r+1)/3,
                (c0.g+c1.g+c2.g+1)/3,
                (c0.b+c1.b+c2.b+1)/3,
                (c0.a+c1.a+c2.a+1)/3);
}
Color Avg(C Color &c0, C Color &c1, C Color &c2, C Color &c3)
{
   return Color((c0.r+c1.r+c2.r+c3.r+2)>>2,
                (c0.g+c1.g+c2.g+c3.g+2)>>2,
                (c0.b+c1.b+c2.b+c3.b+2)>>2,
                (c0.a+c1.a+c2.a+c3.a+2)>>2);
}
/******************************************************************************/
Color Lerp(C Color &c0, C Color &c1, Flt step)
{
   Flt step1=1-step;
   return Color(Mid(RoundPos(c0.r*step1 + c1.r*step), 0, 255),
                Mid(RoundPos(c0.g*step1 + c1.g*step), 0, 255),
                Mid(RoundPos(c0.b*step1 + c1.b*step), 0, 255),
                Mid(RoundPos(c0.a*step1 + c1.a*step), 0, 255));
}
Color Lerp(C Color &c0, C Color &c1, C Color &c2, C Vec &blend)
{
   return Color(Mid(RoundPos(c0.r*blend.x + c1.r*blend.y + c2.r*blend.z), 0, 255),
                Mid(RoundPos(c0.g*blend.x + c1.g*blend.y + c2.g*blend.z), 0, 255),
                Mid(RoundPos(c0.b*blend.x + c1.b*blend.y + c2.b*blend.z), 0, 255),
                Mid(RoundPos(c0.a*blend.x + c1.a*blend.y + c2.a*blend.z), 0, 255));
}
/******************************************************************************/
Color ColorHue(Flt hue)
{
   hue=6*Frac(hue);
   Byte x=RoundU(Frac(hue)*255);
   if(hue<1)return Color(255  ,     x,     0);
   if(hue<2)return Color(255-x, 255  ,     0);
   if(hue<3)return Color(    0, 255  ,     x);
   if(hue<4)return Color(    0, 255-x, 255  );
   if(hue<5)return Color(    x,     0, 255  );
            return Color(  255,     0, 255-x);
}
Color ColorHue(C Color &color, Flt hue)
{
   Vec  hsb=RgbToHsb(color);
   Color  c=ColorHSB(hsb.x+hue, hsb.y, hsb.z); c.a=color.a;
   return c;
}
Vec ColorHue(C Vec &color, Flt hue)
{
   Vec hsb=RgbToHsb(color); hsb.x+=hue; return HsbToRgb(hsb);
}
Color ColorHSB(Flt h, Flt s, Flt b)
{
   SAT(s);
   SAT(b)*=255;
        h=6*Frac(h);
   Flt  f=  Frac(h);
   Byte v=RoundU(b),
        p=RoundU(b*(1-s)),
        q=RoundU(b*(1-s*f)),
        t=RoundU(b*(1-s*(1-f)));
   if(h<1)return Color(v, t, p);
   if(h<2)return Color(q, v, p);
   if(h<3)return Color(p, v, t);
   if(h<4)return Color(p, q, v);
   if(h<5)return Color(t, p, v);
          return Color(v, p, q);
}
Vec RgbToHsb(C Vec &rgb)
{
   Flt max=rgb.max(),
       min=rgb.min(),
       d  =max-min;

   Vec O;
   if(d    <=  0)O.x=0;else
   if(rgb.x>=max)
   {
      if(rgb.y>=rgb.z)O.x=(rgb.y-rgb.z)/d;
      else            O.x=(rgb.y-rgb.z)/d+6;
   }else
   if(rgb.y>=max)O.x=(rgb.z-rgb.x)/d+2;
   else          O.x=(rgb.x-rgb.y)/d+4;

   O.x/=6;
   O.y =(max ? 1-min/max : 1);
   O.z = max;

   return O;
}
Vec HsbToRgb(C Vec &hsb)
{
   Flt h=Frac(hsb.x)*6,
       s=Sat (hsb.y),
       f=Frac(h),
       v=hsb.z,
       p=hsb.z*(1-s      ),
       q=hsb.z*(1-s*(  f)),
       t=hsb.z*(1-s*(1-f));
   if(h<1)return Vec(v, t, p);
   if(h<2)return Vec(q, v, p);
   if(h<3)return Vec(p, v, t);
   if(h<4)return Vec(p, q, v);
   if(h<5)return Vec(t, p, v);
          return Vec(v, p, q);
}
/******************************************************************************/
Vec RgbToYuv(C Vec &rgb)
{
   return Vec( 66/256.0f*rgb.x + 129/256.0f*rgb.y +  25/256.0f*rgb.z +  16/256.0f,
              -38/256.0f*rgb.x -  74/256.0f*rgb.y + 112/256.0f*rgb.z + 128/256.0f,
              112/256.0f*rgb.x -  94/256.0f*rgb.y -  18/256.0f*rgb.z + 128/256.0f);
}
Vec YuvToRgb(C Vec &yuv)
{
   Vec YUV(yuv.x -  16/256.0f,
           yuv.y - 128/256.0f,
           yuv.z - 128/256.0f);
   return Vec(298/256.0f * YUV.x                      + 409/256.0f * YUV.z,
              298/256.0f * YUV.x - 100/256.0f * YUV.y - 208/256.0f * YUV.z,
              298/256.0f * YUV.x + 516/256.0f * YUV.y                     );
}
/******************************************************************************/
Int ColorDiffSum(C Color &x, C Color &y)
{
   return Abs(x.r - y.r)
         +Abs(x.g - y.g)
         +Abs(x.b - y.b)
         +Abs(x.a - y.a);
}
Int ColorDiffMax(C Color &x, C Color &y)
{
   return Max(Abs(x.r - y.r),
              Abs(x.g - y.g),
              Abs(x.b - y.b),
              Abs(x.a - y.a));
}
Flt ColorDiffMax(C Vec &x, C Vec &y)
{
   return Max(Abs(x.x - y.x),
              Abs(x.y - y.y),
              Abs(x.z - y.z));
}
/******************************************************************************/
Color Blend(C Color &base, C Color &color)
{
   Color out;
#if 1 // faster Int version
   UInt  base_w=(255-color.a)*base.a, // "1-color.w" because of standard 'Lerp' (to make base insignificant if new color is fully opaque and covers the base), mul by 'base.w' because if the 'base' is mostly transparent, then we want to make it even more insignificant - for example transparent red 'base' (1, 0, 0, 0) blended with half transparent black 'color' (0, 0, 0, 0.5) would normally blend into (0.5, 0, 0, 0.5)
        color_w=     color.a *255   ; //   'color.w' because of standard 'Lerp', this shouldn't be multiplied by additional 'color.w' because if base.w is 1, and color.w is 0.5 then we would blend it by 0.25 which is not what we want, mul by 255 to match the scale of 'base_w'
   if(UInt sum=base_w+color_w)
   {
       base_w=(base_w*0xFFFF + sum/2)/sum;
      color_w=0xFFFF-base_w;
      out.r=(base.r*base_w + color.r*color_w + 0x8000)>>16;
      out.g=(base.g*base_w + color.g*color_w + 0x8000)>>16;
      out.b=(base.b*base_w + color.b*color_w + 0x8000)>>16;
   }else out.r=out.g=out.b=0;
#else // slower Flt version
   Flt  base_w=((255-color.a)*base.a)/255.0f, // "1-color.w" because of standard 'Lerp' (to make base insignificant if new color is fully opaque and covers the base), mul by 'base.w' because if the 'base' is mostly transparent, then we want to make it even more insignificant - for example transparent red 'base' (1, 0, 0, 0) blended with half transparent black 'color' (0, 0, 0, 0.5) would normally blend into (0.5, 0, 0, 0.5), divide by 255.0f to match the scale of 'color_w' which is 0..255, it will be normalized with 'sum' below
       color_w=      color.a                ; //   'color.w' because of standard 'Lerp', this shouldn't be multiplied by additional 'color.w' because if base.w is 1, and color.w is 0.5 then we would blend it by 0.25 which is not what we want
   if(Flt sum=base_w+color_w)
   {
      base_w/=sum; color_w=1-base_w; // faster than "color_w/=sum;"
      out.r=RoundU(base.r*base_w + color.r*color_w);
      out.g=RoundU(base.g*base_w + color.g*color_w);
      out.b=RoundU(base.b*base_w + color.b*color_w);
   }else out.r=out.g=out.b=0;
#endif
   out.a=base.a+(color.a*(255-base.a)+128)/255;
   return out;
}
Vec4 FastBlend(C Vec4 &base, C Vec4 &color)
{
   return Vec4(base.xyz*(1-color.w) + color.xyz*  color.w  ,
               base.w               + color.w  *(1-base.w));
}
Vec4 Blend(C Vec4 &base, C Vec4 &color)
{
   Vec4 out;
   Flt  base_w=(1-color.w)*base.w, // "1-color.w" because of standard 'Lerp' (to make base insignificant if new color is fully opaque and covers the base), mul by 'base.w' because if the 'base' is mostly transparent, then we want to make it even more insignificant - for example transparent red 'base' (1, 0, 0, 0) blended with half transparent black 'color' (0, 0, 0, 0.5) would normally blend into (0.5, 0, 0, 0.5)
       color_w=   color.w        ; //   'color.w' because of standard 'Lerp', this shouldn't be multiplied by additional 'color.w' because if base.w is 1, and color.w is 0.5 then we would blend it by 0.25 which is not what we want
   if(Flt sum=base_w+color_w)
   {
      base_w/=sum; color_w=1-base_w; // faster than "color_w/=sum;"
      out.xyz=base.xyz*base_w + color.xyz*color_w;
   }else out.xyz.zero();
   out.w=base.w+color.w*(1-base.w);
   return out;
}
Vec4 FastMergeBlend(C Vec4 &base, C Vec4 &color)
{
   return Vec4(base.xyz*(1-color.w) + color.xyz          ,
               base.w               + color.w*(1-base.w));
}
Vec4 MergeBlend(C Vec4 &base, C Vec4 &color)
{
   Vec4 out;
   Flt  base_w=(1-color.w)*base.w, // "1-color.w" because of standard 'Lerp' (to make base insignificant if new color is fully opaque and covers the base), mul by 'base.w' because if the 'base' is mostly transparent, then we want to make it even more insignificant - for example transparent red 'base' (1, 0, 0, 0) blended with half transparent black 'color' (0, 0, 0, 0.5) would normally blend into (0.5, 0, 0, 0.5)
       color_w=   color.w        ; //   'color.w' because of standard 'Lerp', this shouldn't be multiplied by additional 'color.w' because if base.w is 1, and color.w is 0.5 then we would blend it by 0.25 which is not what we want
   if(Flt sum=base_w+color_w)
   {
      base_w/=sum; color_w=1-base_w; // faster than "color_w/=sum;"
      out.xyz=base.xyz*base_w + color.xyz;
   }else out.xyz.zero();
   out.w=base.w+color.w*(1-base.w);
   return out;
}
Vec4 AdditiveBlend(C Vec4 &base, C Vec4 &color)
{
   Vec4 out;
   out.xyz=base.xyz*base .w + color.xyz*color.w;
   out.w  =base.w  +color.w*(1-base.w);
   if(out.w)out.xyz/=out.w;
   return out;
}
/******************************************************************************/
// SRGB
/******************************************************************************/
Flt SRGBToLinear(Flt s) {return (s<=0.04045f  ) ? s/12.92f : Pow((s+0.055f)/1.055f, 2.4f);} // convert 0..1 srgb   to 0..1 linear
Flt LinearToSRGB(Flt l) {return (l<=0.0031308f) ? l*12.92f : Pow(l, 1/2.4f)*1.055f-0.055f;} // convert 0..1 linear to 0..1 srgb

Dbl SRGBToLinear(Dbl s) {return (s<=0.04045  ) ? s/12.92 : Pow((s+0.055)/1.055, 2.4);} // convert 0..1 srgb   to 0..1 linear
Dbl LinearToSRGB(Dbl l) {return (l<=0.0031308) ? l*12.92 : Pow(l, 1/2.4)*1.055-0.055;} // convert 0..1 linear to 0..1 srgb

Vec SRGBToLinear(C Vec &s) {return Vec(SRGBToLinear(s.x), SRGBToLinear(s.y), SRGBToLinear(s.z));}
Vec LinearToSRGB(C Vec &l) {return Vec(LinearToSRGB(l.x), LinearToSRGB(l.y), LinearToSRGB(l.z));}

Vec4 SRGBToLinear(C Vec4 &s) {return Vec4(SRGBToLinear(s.x), SRGBToLinear(s.y), SRGBToLinear(s.z), s.w);}
Vec4 LinearToSRGB(C Vec4 &l) {return Vec4(LinearToSRGB(l.x), LinearToSRGB(l.y), LinearToSRGB(l.z), l.w);}

Vec4 SRGBToLinear(C Color &s) {return Vec4(ByteSRGBToLinear(s.r), ByteSRGBToLinear(s.g), ByteSRGBToLinear(s.b), ByteToFlt(s.a));}
Vec4 LinearToSRGB(C Color &l) {return Vec4(LinearByteToSRGB(l.r), LinearByteToSRGB(l.g), LinearByteToSRGB(l.b), ByteToFlt(l.a));}

Flt SignSRGBToLinear(Flt s) {return (s>=0) ? SRGBToLinear(s) : -SRGBToLinear(-s);}
Flt SignLinearToSRGB(Flt l) {return (l>=0) ? LinearToSRGB(l) : -LinearToSRGB(-l);}

Flt LinearLumOfLinearColor(C Vec &l) {return              Dot(      l        , ColorLumWeight2) ;}
Flt LinearLumOfSRGBColor  (C Vec &s) {return              Dot(SRGBToLinear(s), ColorLumWeight2) ;}
Flt   SRGBLumOfLinearColor(C Vec &l) {return LinearToSRGB(Dot(      l        , ColorLumWeight2));}
Flt   SRGBLumOfSRGBColor  (C Vec &s) {return LinearToSRGB(Dot(SRGBToLinear(s), ColorLumWeight2));}

Flt AlphaToDisplay(Flt alpha)
{
#if !LINEAR_GAMMA
   return alpha;
#elif 0 // targets full luminance
   return 1-SRGBToDisplay(1-alpha);
#else // targets custom luminance
/* col_lin*opacity_lin=SRGBToLinear(LinearToSRGB(col_lin)*opacity_srgb)
           opacity_lin=SRGBToLinear(LinearToSRGB(col_lin)*opacity_srgb)/col_lin

   opacity = 1-alpha */
 //const Flt col_srgb=0.5f, col_lin=SRGBToLinear(col_srgb); // target mid tones
   const Flt col_srgb=0.5f, col_lin=0.214041144f          ; // target mid tones, precalculated SRGBToLinear(0.5)
   return 1-SRGBToLinear(col_srgb*(1-alpha))/col_lin;
#endif
}
/******************************************************************************/
Vec NightLightFactor(Flt intensity)
{
   return Vec(1,                                                    // red
              Lerp(1-Sqr(intensity), Sqrt(1-intensity), intensity), // green, "1-Sqr(intensity)" is better at the start and "Sqrt(1-intensity)" is better at the end, so do interpolation between them
              1-intensity);                                         // blue
}
/******************************************************************************/
Byte  LinearToByteSRGB(  Flt   l) {return LinearToByteSRGBArray[Mid(RoundPos(l*(Elms(LinearToByteSRGBArray)-1)), 0, Elms(LinearToByteSRGBArray)-1)];}
Byte  SRGBToLinearByte(  Flt   s) {return SRGBToLinearByteArray[Mid(RoundPos(s*(Elms(SRGBToLinearByteArray)-1)), 0, Elms(SRGBToLinearByteArray)-1)];}
VecB  LinearToSVecB   (C Vec  &l) {return VecB (LinearToByteSRGB(l.x), LinearToByteSRGB(l.y), LinearToByteSRGB(l.z));}
Color LinearToSColor  (C Vec  &l) {return Color(LinearToByteSRGB(l.x), LinearToByteSRGB(l.y), LinearToByteSRGB(l.z));}
Color LinearToSColor  (C Vec4 &l) {return Color(LinearToByteSRGB(l.x), LinearToByteSRGB(l.y), LinearToByteSRGB(l.z), FltToByte(l.w));}

void InitSRGB()
{
   REPAO(ByteToFltArray       )=i/255.0f; // don't use 'ByteToFlt' in case it's based on 'ByteToFltArray'
   REPAO(ByteSRGBToLinearArray)=SRGBToLinear(i/255.0); // use Dbl version for more precision
   REPAO(LinearByteToSRGBArray)=LinearToSRGB(i/255.0); // use Dbl version for more precision
   REPAO(LinearToByteSRGBArray)=FltToByte(LinearToSRGB(i/Flt(Elms(LinearToByteSRGBArray)-1)));
   REPAO(SRGBToLinearByteArray)=FltToByte(SRGBToLinear(i/Flt(Elms(SRGBToLinearByteArray)-1)));

#if 0 // code used for finding minimum array size
   Elms_SRGBToLinearByteArray_=0;
again:
   FREP(Elms_SRGBToLinearByteArray_)SRGBToLinearByteArray[i]=FltToByte(SRGBToLinear(i/Flt(Elms_SRGBToLinearByteArray_-1)));
   FREP(256)if(SRGBToLinearByte(LinearByteToSRGB(i))!=i){Elms_SRGBToLinearByteArray_++; goto again;}
#endif

#if DEBUG && 0 // do some debug checks
   FREP(Elms(ByteSRGBToLinearArray)-1)DYNAMIC_ASSERT(ByteSRGBToLinearArray[i]<=ByteSRGBToLinearArray[i+1], "ByteSRGBToLinearArray[i] > ByteSRGBToLinearArray[i+1]");
   FREP(Elms(LinearToByteSRGBArray)-1)DYNAMIC_ASSERT(LinearToByteSRGBArray[i]<=LinearToByteSRGBArray[i+1], "LinearToByteSRGBArray[i] > LinearToByteSRGBArray[i+1]");
   FREP(                          256)DYNAMIC_ASSERT(LinearToByteSRGB(ByteSRGBToLinear(i))==i            , "LinearToByteSRGB(ByteSRGBToLinear(s))!=s");
   FREP(                          256)DYNAMIC_ASSERT(SRGBToLinearByte(LinearByteToSRGB(i))==i            , "SRGBToLinearByte(LinearByteToSRGB(s))!=s");
#endif
}
#if 0 // approximate functions
/*
   Error was calculated using:
   Flt d0=0; Int d1=0;
   REP(256)
   {
      Flt  f=i/255.0f,
           l1=ByteSRGBToLinear  (i),
           l2=SRGBToLinearApprox(i);
      Byte s1=LinearToByteSRGB  (f),
           s2=LinearToSRGBApprox(f);
      d0+=Abs(l1-l2);
      d1+=Abs(s1-s2);
   }
   Exit(S+d0/256+' '+d1/255.0f/256);
*/
#define SRGB_MODE 0
static Flt SRGBToLinearApprox(Byte s)
{
   Flt f=s/255.0f;
#if   SRGB_MODE==0 // average error = 0.023
   return Sqr(f);
#elif SRGB_MODE==1 // average error = 0.004
   return Pow(f, 2.2f);
#else              // average error = 0.001
   return f*(f*(f*0.305306011f+0.682171111f)+0.012522878f);
#endif
}
static Byte LinearToSRGBApprox(Flt l)
{
   if(l<=0)return 0;
#if   SRGB_MODE==0 // average error = 0.023
   Flt s=SqrtFast(l);
#elif SRGB_MODE==1 // average error = 0.004
   Flt s=Pow(l, 1/2.2f);
#else              // average error = 0.001
   Flt s1=SqrtFast(l),
       s2=SqrtFast(s1),
       s3=SqrtFast(s2),
       s =0.585122381f*s1 + 0.783140355f * s2 - 0.368262736f*s3;
#endif
   return Min(RoundPos(s*255.0f), 255);
}
#endif
/******************************************************************************/
Vec4 SRGBToDisplay(C Color &s)
{
   return Vec4(ByteSRGBToDisplay(s.r), ByteSRGBToDisplay(s.g), ByteSRGBToDisplay(s.b), ByteToFlt(s.a));
}
/******************************************************************************/
#define COLOR_TRANSFOR_MAX_DELTA (DEBUG && 0)
#if     COLOR_TRANSFOR_MAX_DELTA
#pragma message("!! Warning: Use only for testing !!")
static Int MaxDelta=0;
#endif

static CChar8* ColorSpaceFileName(COLOR_SPACE color_space)
{
   switch(color_space)
   {
      case COLOR_SPACE_SRGB      : return "Color/sRGB.icc";
      case COLOR_SPACE_DISPLAY_P3: return "Color/DisplayP3.icc";
    //case COLOR_SPACE_DCI_P3    : return "Color/DCI-P3-D65.icc";
    //case COLOR_SPACE_BT_2020   : return "Color/ITU-R_BT2020(beta).icc";
      default                    : return null;
   }
}

#if WINDOWS_OLD
struct WinColorTransform
{
   HPROFILE   dest_profile=null;
   HPROFILE    src_profile=null;
   HTRANSFORM transform   =null;

  ~WinColorTransform() {del();}
   void del()
   {
      if(transform   ){DeleteColorTransform(transform   ); transform   =null;}
      if( src_profile){CloseColorProfile   ( src_profile);  src_profile=null;}
      if(dest_profile){CloseColorProfile   (dest_profile); dest_profile=null;}
   }
   Bool create(COLOR_SPACE src_color_space, C Str &dest_color_space)
   {
      del();

      PROFILE profile;
      profile.dwType      =PROFILE_FILENAME;
      profile.pProfileData=ConstCast(dest_color_space());
      profile.cbDataSize  =dest_color_space.length()*SIZE(Char);
      if(dest_profile=OpenColorProfileW(&profile, PROFILE_READ, FILE_SHARE_READ, OPEN_EXISTING))
      {
         File f; if(f.readTry(ColorSpaceFileName(src_color_space)))
         {
            Memt<Byte> data; data.setNum(f.size()); if(f.getFast(data.data(), data.elms()))
            {
               profile.dwType      =PROFILE_MEMBUFFER;
               profile.pProfileData=data.data();
               profile.cbDataSize  =data.elms();
               if(src_profile=OpenColorProfileW(&profile, PROFILE_READ, FILE_SHARE_READ, OPEN_EXISTING))
               {
                  HPROFILE profiles[]={src_profile, dest_profile};
                  DWORD     intents[]={INTENT_PERCEPTUAL};
                  if(transform=CreateMultiProfileTransform(profiles, Elms(profiles), intents, Elms(intents), BEST_MODE, INDEX_DONT_CARE))
                     return true;
               }
            }
         }
      }
      del(); return false;
   }
   Bool convert(C Vec4 &src, Vec4 &dest, Bool &different)C
   {
      if(transform)
      {
         COLOR s, d;
         s.rgb.red  =Mid(RoundPos(src.x*65535), 0, 65535);
         s.rgb.green=Mid(RoundPos(src.y*65535), 0, 65535);
         s.rgb.blue =Mid(RoundPos(src.z*65535), 0, 65535);
         TranslateColors(transform, &s, 1, COLOR_RGB, &d, COLOR_RGB);
         dest.x=d.rgb.red  /65535.0f;
         dest.y=d.rgb.green/65535.0f;
         dest.z=d.rgb.blue /65535.0f;

         Int delta=Max(Abs(s.rgb.red  -d.rgb.red  ),
                       Abs(s.rgb.green-d.rgb.green),
                       Abs(s.rgb.blue -d.rgb.blue ));
         different=(delta>384); // 1.5 on 255 scale (use this value because when using sRGB profile -> sRGB target, 'WinColorTransform' still gives 334 error)
      #if COLOR_TRANSFOR_MAX_DELTA
         MAX(MaxDelta, delta);
      #endif
         return true;
      }
      dest=src; different=false;
      return false;
   }
};
#endif
#if 0
static Str MakeFullColorProfilePath(C Str &name) // some libraries need full path
{
#if WINDOWS_OLD
   if(name.is() && !FullPath(name))return SystemPath(SP_SYSTEM).tailSlash(true)+"spool/drivers/color/"+name;
#endif
   return name;
}
#include "../../../ThirdPartyLibs/QCMS/lib/qcms.h" // QCMS does not support 16-bit precision
struct QCMSColorTransform
{
   qcms_profile   *qcms_dest=null;
   qcms_profile   *qcms_src =null;
   qcms_transform *transform=null;

  ~QCMSColorTransform() {del();}
   void del()
   {
      if(transform){qcms_transform_release(transform); transform=null;}
      if(qcms_src ){qcms_profile_release  (qcms_src ); qcms_src =null;}
      if(qcms_dest){qcms_profile_release  (qcms_dest); qcms_dest=null;}
   }
   Bool create(COLOR_SPACE src_color_space, C Str &dest_color_space)
   {
      del();

      if(qcms_dest=qcms_profile_from_unicode_path(MakeFullColorProfilePath(dest_color_space)))
      {
         switch(src_color_space)
         {
            case COLOR_SPACE_SRGB: qcms_src=qcms_profile_sRGB(); break;
            default              :
            {
               File f; if(f.readTry(ColorSpaceFileName(src_color_space)))
               {
                  Memt<Byte> data; data.setNum(f.size()); if(f.getFast(data.data(), data.elms()))qcms_src=qcms_profile_from_memory(data.data(), data.elms());
               }
            }break;
         }

         if(qcms_src)
         if(transform=qcms_transform_create(qcms_src, QCMS_DATA_RGB_8, qcms_dest, QCMS_DATA_RGB_8, QCMS_INTENT_PERCEPTUAL))
            return true;
      }
      del(); return false;
   }
   Bool convert(C Vec4 &src, Vec4 &dest, Bool &different)C
   {
      if(transform)
      {
         Color s, d;
         s=src;
         qcms_transform_data(transform, &s, &d, 1); d.a=s.a;
         dest=d;

         Int delta=ColorDiffMax(s, d);
         different=(delta>1);
      #if COLOR_TRANSFOR_MAX_DELTA
         MAX(MaxDelta, delta);
      #endif
         return true;
      }
      dest=src; different=false;
      return false;
   }
};
#endif
Bool SetColorLUT(COLOR_SPACE src_color_space, C Str &dest_color_space, Image &lut)
{
   if(src_color_space!=COLOR_SPACE_NONE && dest_color_space.is())
   {
   #if WINDOWS_OLD
      WinColorTransform ct; if(ct.create(src_color_space, dest_color_space))
      {
         // here we can't use any sRGB format (or store using 'color3DS' to image) to get a free conversion to dest, because if for example 'res' is 2 (from black to white) then results still have to be converted to linear space to get perceptual smoothness
         const Int res=64;
         Bool prec16=(Renderer._main.precision()>=IMAGE_PRECISION_16); // can ignore dither because we always set at least 10-bits
         if(!(prec16 && lut.create3DTry(res, res, res, IMAGE_F16_3      , 1, false)))
         if(!(prec16 && lut.create3DTry(res, res, res, IMAGE_F16_4      , 1, false)))
         if(!(          lut.create3DTry(res, res, res, IMAGE_R10G10B10A2, 1       )))
            return false;

         if(lut.lock(LOCK_WRITE))
         {
            Bool different=false;
            Vec4 src; src.w=1;
            REPD(z, lut.d())
            {
               src.z=z/Flt(lut.d()-1);
               REPD(y, lut.h())
               {
                  src.y=y/Flt(lut.h()-1);
                  REPD(x, lut.w())
                  {
                     src.x=x/Flt(lut.w()-1);
                     Vec4 dest; Bool diff; ct.convert(src, dest, diff);
                     lut.color3DF(x, y, z, dest); different|=diff;
                  }
               }
            }
            lut.unlock();
            if(different)return true; // no point in using LUT if it's not different
         }
      }
   #endif
   }
   lut.del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
