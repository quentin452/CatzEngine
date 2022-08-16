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
