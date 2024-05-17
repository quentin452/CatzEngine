/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Sky' to set custom sky.

/******************************************************************************/
struct SkyClass
{
   // manage
   SkyClass& clear     (             );                                             // disable sky rendering
   SkyClass& frac      (Flt frac     );   Flt  frac      ()C {return _frac       ;} // set/get sky fraction (fraction of the Viewport range where the sky starts), 0..1, default=0.8, (1 is the fastest)
   SkyClass& nightLight(Flt intensity);   Flt  nightLight()C {return _night_light;} // set/get sky night light (blue filter) intensity, 0..1, default=0
                                          Bool is        ()C {return _is         ;} // if      sky rendering is enabled

   // atmospheric sky
   SkyClass& atmospheric                (                     );                                                                 // enable  drawing sky as atmospheric sky
   SkyClass& atmosphericDensityExponent (  Flt       exp      );   Flt       atmosphericDensityExponent ()C {return _dns_exp  ;} // set/get density exponent          ,            0..1                   , default=1.0, (1 is the fastest)
   SkyClass& atmosphericHorizonExponent (  Flt       exp      );   Flt       atmosphericHorizonExponent ()C {return _hor_exp  ;} // set/get horizon exponent          ,            0..Inf                 , default=3.5, (this affects at what height the horizon color will be selected instead of the sky color)
   SkyClass& atmosphericHorizonColorS   (C Vec4     &color_s  );   Vec4      atmosphericHorizonColorS   ()C;                     // set/get horizon color sRGB   gamma,    (0,0,0,0)..(1,1,1,1)           , here alpha specifies opacity to combine with star map when used
   SkyClass& atmosphericHorizonColorL   (C Vec4     &color_l  ); C Vec4&     atmosphericHorizonColorL   ()C {return _hor_col_l;} // set/get horizon color linear gamma,    (0,0,0,0)..(1,1,1,1)           , here alpha specifies opacity to combine with star map when used
   SkyClass& atmosphericSkyColorS       (C Vec4     &color_s  );   Vec4      atmosphericSkyColorS       ()C;                     // set/get sky     color sRGB   gamma,    (0,0,0,0)..(1,1,1,1)           , here alpha specifies opacity to combine with star map when used
   SkyClass& atmosphericSkyColorL       (C Vec4     &color_l  ); C Vec4&     atmosphericSkyColorL       ()C {return _sky_col_l;} // set/get sky     color linear gamma,    (0,0,0,0)..(1,1,1,1)           , here alpha specifies opacity to combine with star map when used
   SkyClass& atmosphericStars           (C ImagePtr &cube     ); C ImagePtr& atmosphericStars           ()C {return _stars    ;} // set/get sky     star map          , image must be in IMAGE_CUBE format, default=null
   SkyClass& atmosphericStarsOrientation(C Matrix3  &orn      );   Matrix3   atmosphericStarsOrientation()C {return _stars_m  ;} // set/get sky     star orientation  , 'orn' must be normalized          , default=MatrixIdentity
   SkyClass& atmosphericPrecision       (  Bool      per_pixel);   Bool      atmosphericPrecision       ()C {return _precision;} // set/get sky     precision         ,          true/false               , default=true (false for Mobile), if false is set then sky calculations are done per-vertex with lower quality

   SkyClass& atmosphericColorS(C Vec4 &horizon_color_s, C Vec4 &sky_color_s) {return atmosphericHorizonColorS(horizon_color_s).atmosphericSkyColorS(sky_color_s);} // set atmospheric horizon and sky color sRGB   gamma
   SkyClass& atmosphericColorL(C Vec4 &horizon_color_l, C Vec4 &sky_color_l) {return atmosphericHorizonColorL(horizon_color_l).atmosphericSkyColorL(sky_color_l);} // set atmospheric horizon and sky color linear gamma

   // sky from skybox
   SkyClass& skybox     (C ImagePtr &image           ); C ImagePtr& skybox     ()C {return _image[0] ;} // enable drawing sky as skybox
   SkyClass& skybox     (C ImagePtr &a, C ImagePtr &b); C ImagePtr& skybox2    ()C {return _image[1] ;} // enable drawing sky as blend between 2 skyboxes
   SkyClass& skyboxBlend(  Flt       blend           );   Flt       skyboxBlend()C {return _box_blend;} // set/get blend factor between 2 skyboxes, 0..1, default=0.5

#if EE_PRIVATE
   Bool      isActual     ()C {return _is && FovPerspective(D.viewFovMode());}
   SkyClass& del          ();
   SkyClass& create       ();
   Bool      wantDepth    ()C;
   void      draw         ();
   void      setFracMulAdd();
   #if LINEAR_GAMMA
      INLINE C Vec4& atmosphericHorizonColorD()C {return atmosphericHorizonColorL();}
      INLINE C Vec4& atmosphericSkyColorD    ()C {return atmosphericSkyColorL    ();}
   #else
      INLINE   Vec4  atmosphericHorizonColorD()C {return atmosphericHorizonColorS();}
      INLINE   Vec4  atmosphericSkyColorD    ()C {return atmosphericSkyColorS    ();}
   #endif
#endif

   SkyClass();

#if !EE_PRIVATE
private:
#endif
   Bool       _is, _precision;
   Flt        _frac, _night_light, _dns_exp, _hor_exp, _box_blend;
   Vec4       _hor_col_l, _sky_col_l;
   Matrix3    _stars_m;
   MeshRender _mshr;
   ImagePtr   _image[2], _stars;
}extern
   Sky; // Main Sky
/******************************************************************************/
struct Atmosphere : BallM // BallM.r = total radius of the atmosphere = planet radius + atmosphere height, BallM.pos = world-space position
{
   static Flt  ViewRange; // View Range for rendering Atmosphere, 0..Inf, this value is used inside the Atmosphere shader, and can be larger than 'D.viewRange', to allow drawing full atmosphere in Planetary View, while limiting terrain and objects to 'D.viewRange'
   static VecD SunPos   ; // world-space sun position that will give light for the Atmosphere

   Flt planet_radius  , // planet radius, excluding atmosphere, 0..Inf
       height         , // height of the atmosphere itself, excluding 'planet_radius', for the total atmosphere radius please see 'BallM.r' = planet_radius+height, 0..Inf
       light_scale    , // light scale, 0..Inf
       fog_reduce     , // fog reduction = 1-MaxFog, 0..1
       fog_reduce_dist, // fog reduction distance  , 0..Inf
       darken         , // how much to darken space background based on light intensity, 0..Inf
       mie_extinction ; // Mie extinction=scattering+absorption, 0..Inf

   Atmosphere() {}
   Atmosphere(C VecD &pos, Flt planet_radius, Flt height, Flt light_scale) {T.pos=pos; T.planet_radius=planet_radius; T.height=height; T.r=planet_radius+height; T.light_scale=light_scale; T.fog_reduce=1.0f; T.fog_reduce_dist=height; T.darken=14; T.mie_extinction=0.0016f;}

   Vec calcCol(Flt look_angle                        )C; // calculate sky color when standing on ground surface                       , when looking at direction based on 'look_angle' (0..PI/2, 0=look up, PI/2=look forward), at noon                                                                  , this ignores 'light_scale', 'fog_reduce' and Mie scattering
   Vec calcCol(C Vec &pos, C Vec &ray, C Vec &sun_pos)C; // calculate sky color when standing on 'pos' position relative to atmosphere, when looking at 'ray' direction (must be normalized)                                   , with sun located at 'sun_pos' relative to atmosphere (must be normalized), this ignores 'light_scale', 'fog_reduce' and Mie scattering

   void draw()C; // draw this Atmosphere object, this should be called only in RM_PREPARE mode
#if EE_PRIVATE
   void scattering(Flt height, Vec &rayleigh_scattering, Flt &mie_scattering, Vec &extinction)C;
   void drawDo(Int multi_sample, Bool dither)C;
   Bool toScreenRect(Rect &rect)C {return ToFullScreenRect(T, rect);}
#endif
};
#if EE_PRIVATE
#define SKY_MESH_MIN_DIST 0.98f // it's good to make it a bit smaller than 'dist' to have some epsilon for precision issues, this is the closest point on the mesh to the Vec(0,0,0), it's not equal to radius=1, because the mesh is composed out of triangles, and the triangle surfaces are closer
extern Memc<Atmosphere> Atmospheres;
#endif
/******************************************************************************/
