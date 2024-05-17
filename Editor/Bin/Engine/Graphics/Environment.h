/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   'Environment' holds information about graphics options like:
      ambient, bloom, clouds, fog, sky, sun.

/******************************************************************************/
struct Environment
{
   struct Ambient
   {
      Bool on                 ; // if enabled                  ,     true/false    , default=true
      Vec  color_s            , // ambient     color sRGB gamma, (0,0,0) .. (1,1,1), default=(0.366, 0.366, 0.366)
           night_shade_color_s; // night shade color sRGB gamma, (0,0,0) .. (1,1,1), default=(0.000, 0.000, 0.000)

      // set / get
      void set  ()C; // apply these settings to graphics
      void get  () ; // get current graphics settings and store them in self
      void reset() ; // reset to default values

      // io
      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

      Ambient() {reset();}
   };

   struct Bloom
   {
      Bool on      ; // if enabled     , true/false, default=true
      Flt  original, // original color ,    0..Inf , default=1.0
           scale   , // bloom scale    ,    0..Inf , default=0.8
           cut     , // bloom cutoff   ,    0..Inf , default=0.3
           glow    ; // bloom glow     ,    0..Inf , default=1.0

      // set / get
      void set  ()C; // apply these settings to graphics
      void get  () ; // get current graphics settings and store them in self
      void reset() ; // reset to default values

      // io
      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

      Bloom() {reset();}
   };

   struct Clouds
   {
      struct Layer // Cloud Layer
      {
         Flt      scale   ; // texture scale           ,         0..Inf      , default={0.35, 0.41, 0.50, 0.62}
         Vec2     velocity; // texture velocity        ,      -Inf..Inf      , default={0.010, 0.008, 0.006, 0.004}
         Vec4     color_s ; // texture color sRGB gamma, (0,0,0,0)..(1,1,1,1), default=(1,1,1,1)
         ImagePtr image   ; // texture                 ,                       default=null
      };

      Bool  on            ; // if enabled              , true/false, default=true
      Flt   vertical_scale; // vertical texture scaling,    1..2   , default=1.05, setting this value higher than 1 helps covering the empty gap between flat ground and the clouds
      Layer layers[4]     ; // layer array

      // set / get
      void set  ()C; // apply these settings to graphics
      void get  () ; // get current graphics settings and store them in self
      void reset() ; // reset to default values

      // io
      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

      Clouds() {reset();}
   };

   struct Fog
   {
      Bool on        , // if enabled          ,    true/false   , default=false
           affect_sky; // if fog affects sky  ,    true/false   , default=false
      Flt  density   ; // fog density         ,       0..1      , default=0.02
      Vec  color_s   ; // fog color sRGB gamma, (0,0,0)..(1,1,1), default=(0.5, 0.5, 0.5)

      // set / get
      void set  ()C; // apply these settings to graphics
      void get  () ; // get current graphics settings and store them in self
      void reset() ; // reset to default values

      // io
      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

      Fog() {reset();}
   };

   struct Sky
   {
      Bool on  ; // if enabled  , true/false, default=true
      Flt  frac; // sky fraction,    0..1   , default=0.8, (1 is the fastest), fraction of the Viewport range where the sky starts

      Flt      atmospheric_density_exponent , // atmospheric density exponent        ,            0..1                   , default=1.0, (1 is the fastest)
               atmospheric_horizon_exponent ; // atmospheric horizon exponent        ,            0..Inf                 , default=3.5, (this affects at what height the horizon color will be selected instead of the sky color)
      Vec4     atmospheric_horizon_color_s  , // atmospheric horizon color sRGB gamma,    (0,0,0,0)..(1,1,1,1)           , here alpha specifies opacity to combine with star map when used
               atmospheric_sky_color_s      ; // atmospheric sky     color sRGB gamma,    (0,0,0,0)..(1,1,1,1)           , here alpha specifies opacity to combine with star map when used
      ImagePtr atmospheric_stars            ; // atmospheric star map                , image must be in IMAGE_CUBE format, default=null
      Matrix3  atmospheric_stars_orientation; // atmospheric star orientation        ,       must be normalized          , default=MatrixIdentity3

      ImagePtr skybox; // skybox image, default=null, when specified then it will be used instead of atmospheric sky

      // set / get
      void set  ()C; // apply these settings to graphics
      void get  () ; // get current graphics settings and store them in self
      void reset() ; // reset to default values

      // io
      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

      Sky() {reset();}
   };

   struct Sun
   {
      Bool     on             ; // if enabled                  ,        true/false         , default=true
      Byte     glow           ; // glow amount                 ,           0..255          , default=128 , total glow amount is equal to ('image' alpha channel * 'image_color' alpha component * 'glow')
      Flt      size           , // image size                  ,           0..1            , default=0.15
               highlight_front, // highlight on atmospheric sky,           0..Inf          , default=0.20
               highlight_back ; // highlight on atmospheric sky,           0..Inf          , default=0.15
      Vec      pos            , // position on sky sphere      , its length must be equal 1, default=!Vec(-1, 1, -1)
               light_color_s  , // light color sRGB gamma      ,     (0,0,0)..(1,1,1)      , default=(0.950, 0.950, 0.950), value of (0, 0, 0) disables light casting
                rays_color    ; // rays  color                 ,     (0,0,0)..(1,1,1)      , default=(0.120, 0.120, 0.120)
      Vec4     image_color    ; // image color                 ,   (0,0,0,0)..(1,1,1,1)    , default=(1, 1, 1, 1)
      ImagePtr image          ; // image                                                   , default=null

      // set / get
      void set  ()C; // apply these settings to graphics
      void get  () ; // get current graphics settings and store them in self
      void reset() ; // reset to default values

      // io
      Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
      Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

      Sun() {reset();}
   };

   Ambient ambient;
   Bloom   bloom  ;
   Clouds  clouds ;
   Fog     fog    ;
   Sky     sky    ;
   Sun     sun    ;

   // set / get
   void set  ()C; // apply these settings to graphics                     (this will call 'set'   on all members of this class)
   void get  () ; // get current graphics settings and store them in self (this will call 'get'   on all members of this class)
   void reset() ; // reset to default values                              (this will call 'reset' on all members of this class)

   // io
   Bool save(File &f, CChar *path=null)C; // save to   file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f, CChar *path=null) ; // load from file, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

   Bool save(C Str &name)C; // save to   file, false on fail
   Bool load(C Str &name) ; // load from file, false on fail
};
/******************************************************************************/
DECLARE_CACHE(Environment, Environments, EnvironmentPtr); // 'Environments' cache storing 'Environment' objects which can be accessed by 'EnvironmentPtr' pointer
/******************************************************************************/
