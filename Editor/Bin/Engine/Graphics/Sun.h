/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Sun' to set the main sun object.

   Use 'Astros' container to create custom astronomical objects.

/******************************************************************************/
enum SUN_RAYS_MODE : Byte
{
   SUN_RAYS_OFF         , // disabled
   SUN_RAYS_OPAQUE      , // enabled with opaque       occlusion detection
   SUN_RAYS_OPAQUE_BLEND, // enabled with opaque+blend occlusion detection
   SUN_RAYS_NUM         , // number of sun ray modes
};
/******************************************************************************/
struct Astro // Astronomical Object (Star/Planet/Moon)
{
   Bool draw; // if use this object in drawing, true/false, default=true

   Byte     glow       ; // glow amount        ,           0..255               , default=0   , total glow amount is equal to ('image' alpha channel * 'image_color' alpha component * 'glow')
   Flt      size       ; // image      size    ,           0..1                 , default=0.15
   Vec      pos        ; // normalized position, position on sky sphere radius=1, default=!Vec(-1,1,-1)
   Color    image_color; // image      color   ,                                , default=WHITE
   ImagePtr image      ; // image                                               , default=null

   Vec light_color_l  ; // light      color linear gamma, (0,0,0)..(1,1,1), default=(0,0,0), value of (0,0,0) disables light casting
   Flt light_vol      , // volumetric amount            ,       0..Inf    , default=0.0
       light_vol_exp  , // volumetric exponent          ,       0..Inf    , default=1.0
       light_vol_steam; // volumetric steam             ,       0..1      , default=0.5

 C Vec& lightColorL()C {return light_color_l;}   void lightColorL(C Vec &color_l) {T.light_color_l=color_l;} // get/set Linear Gamma color
   Vec  lightColorS()C;                          void lightColorS(C Vec &color_s);                           // get/set sRGB   Gamma color

   Astro();

#if EE_PRIVATE
   Bool is   ()C {return draw && image!=null;}
   void light();
   void Draw ();
#endif
};
/******************************************************************************/
struct SunClass : Astro // Sun objects have default member values: 'glow'=128, 'light_color'=(0.7, 0.7, 0.7)
{
   Flt highlight_front, // amount of highlight applied on atmospheric sky, 0..Inf, default=0.20
       highlight_back ; // amount of highlight applied on atmospheric sky, 0..Inf, default=0.15

   Vec           rays_color ; // rays color , (0,0,0)..(1,1,1), default=(0.12, 0.12, 0.12)
   SByte         rays_jitter; // rays jitter,   -1/false/true , default=-1, false=always disabled, true=always enabled, -1=auto
   SUN_RAYS_MODE rays_mode  ; // rays mode  ,   SUN_RAYS_MODE , default=SUN_RAYS_OPAQUE_BLEND

   SunClass& raysRes    (Flt scale);   Flt raysRes    ()C; // set/get rays      resolution (0..1, default=1/4), this determines the size of the buffers used for calculating the Sun Rays      effect, 1=full size, 0.5=half size, 0.25=quarter size, .., smaller sizes offer faster performance but worse quality, the change is NOT instant, avoid calling real-time
   SunClass& raysMaskRes(Flt scale);   Flt raysMaskRes()C; // set/get rays mask resolution (0..1, default=1  ), this determines the size of the buffers used for calculating the Sun Rays Mask effect, 1=full size, 0.5=half size, 0.25=quarter size, .., smaller sizes offer faster performance but worse quality, the change is NOT instant, avoid calling real-time

   SunClass();

#if EE_PRIVATE
   Bool wantRays ()C;
   Bool wantDepth()C {return wantRays();}
   void drawRays(Vec &color);
#endif

#if !EE_PRIVATE
private:
#endif
   SUN_RAYS_MODE _actual_rays_mode;
   Byte          _rays_res, _rays_mask_res;
   Vec2          _pos2;
}extern
   Sun; // Main Sun
/******************************************************************************/
extern Bool        AstrosDraw; // if draw      astronomical objects, default=true
extern Memc<Astro> Astros    ; // container of astronomical objects
/******************************************************************************/
#if EE_PRIVATE
void AstroPrepare ();
void AstroDraw    ();
void AstroDrawRays();
#endif
/******************************************************************************/
