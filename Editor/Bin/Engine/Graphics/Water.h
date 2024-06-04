#pragma once
/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************/
struct WaterMtrlParams // Water Material Parameters
{
    Vec color;        // color Linear Gamma          , (0,0,0)..(1,1,1), default=(1,1,1)
    Flt rough,        // roughness                   ,       0..1      , default=0
        reflect,      // reflectivity                ,       0..1      , default=0.02
        normal,       // normal map sharpness        ,       0..1      , default=1
        wave_scale,   // vertical wave scale         ,       0..1      , default=0 (0 is the fastest)
        scale_color,  // scale color  texture        ,       0..Inf    , default=1/200
        scale_normal, // scale normal texture        ,       0..Inf    , default=1/10
        scale_bump,   // scale bump   texture        ,       0..Inf    , default=1/100

        density,     //                 density     ,       0..1      , default=0.3
        density_add, // factor added to density     ,       0..1      , default=0

        refract,            // refraction power            ,       0..1      , default=0.10 (0 is the fastest)
        refract_reflection; // refraction of the reflection,       0..1      , default=0.06

    C Vec &colorL() C { return color; }
    void colorL(C Vec &color_l) { T.color = color_l; } // get/set Linear Gamma color
    Vec colorS() C;
    void colorS(C Vec &color_s); // get/set sRGB   Gamma color
    Flt smooth() C { return 1 - rough; }
    void smooth(Flt smooth) { T.rough = 1 - smooth; } // get/set smoothness
};
struct WaterMtrl : WaterMtrlParams // Water Material
{
    Vec color_underwater0, // color when viewed underwater on surface Linear Gamma, (0,0,0)..(1,1,1)
        color_underwater1; // color when viewed underwater deep       Linear Gamma, (0,0,0)..(1,1,1)

    C Vec &colorUnderwater0L() C { return color_underwater0; }
    void colorUnderwater0L(C Vec &color_l) { T.color_underwater0 = color_l; } // get/set Linear Gamma color
    Vec colorUnderwater0S() C;
    void colorUnderwater0S(C Vec &color_s); // get/set sRGB   Gamma color, default=(0.26, 0.35, 0.42)

    C Vec &colorUnderwater1L() C { return color_underwater1; }
    void colorUnderwater1L(C Vec &color_l) { T.color_underwater1 = color_l; } // get/set Linear Gamma color
    Vec colorUnderwater1S() C;
    void colorUnderwater1S(C Vec &color_s); // get/set sRGB   Gamma color, default=(0.10, 0.20, 0.30)

    // set / get
    WaterMtrl &colorMap(C ImagePtr &image);
    C ImagePtr &colorMap() C { return _color_map; } // set/get color  map
    WaterMtrl &normalMap(C ImagePtr &image);
    C ImagePtr &normalMap() C { return _normal_map; } // set/get normal map
    WaterMtrl &bumpMap(C ImagePtr &image);
    C ImagePtr &bumpMap() C { return _bump_map; } // set/get bump   map

    // operations
    WaterMtrl &reset();    // reset to default values
    WaterMtrl &validate(); // this needs to be called after manually changing the parameters/textures
#if EE_PRIVATE
    void set() C;
#endif

    // io
    Bool save(C Str &name) C; // save, false on fail
    Bool load(C Str &name);   // load, false on fail

    Bool save(File &f, CChar *path = null) C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
    Bool load(File &f, CChar *path = null);   // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

    WaterMtrl();

#if !EE_PRIVATE
  private:
#endif
    ImagePtr _color_map, _normal_map, _bump_map;
};
/******************************************************************************/
DECLARE_CACHE(WaterMtrl, WaterMtrls, WaterMtrlPtr); // Water Material Cache
#if EE_PRIVATE
extern const WaterMtrl *WaterMtrlLast; // Last set Water Material
extern WaterMtrlPtr WaterMtrlNull;

#define WATER_TRANSITION 0.1f // transition between above and under surface. this is in meters, this value is added on top of water surfaces. So underwater effect starts fading from water_surface+water_up*WATER_TRANSITION (intensity=0) to water_surface (intensity=1)
#endif
/******************************************************************************/
struct WaterClass : WaterMtrl // Main water control
{
    Bool draw,                  // if draw the water plane      , true/false, default=false                  , it can be drawn only after setting 'draw' to true, and setting valid water images in parameters
        reflection_allow,       // if allow rendering reflection, true/false, default=true (false for Mobile)
        reflection_shadows;     // if use shadows in reflection , true/false, default=false                  , to have shadows in reflection you must also enable global shadowing
    Byte reflection_resolution; // reflection resolution        ,    0..4   , default=1                      , the bigger value the worse quality but faster rendering
    PlaneM plane;               // water plane                  ,           , default=(pos(0,0,0), normal(0,1,0))

    WaterClass &reflectionRenderer(RENDER_TYPE type);
    RENDER_TYPE reflectionRenderer() C { return _reflect_renderer; } // set/get Renderer used for rendering the reflaction , default=RT_DEFERRED
    WaterClass &max1Light(Bool on);
    Bool max1Light() C { return _max_1_light; } // set/get if use only up to 1 light for water surface, default=true (this greatly increases water rendering performance, however allows only 1 directional light affecting water surface), this affects only RT_DEFERRED renderer, all other renderers are always limited to only 1 directional light. If this is disabled then some water material parameters are taken from global 'Water' instead of per water mesh material.

    WaterClass &update(C Vec2 &vel); // update wave movement, 'vel'=velocity

#if EE_PRIVATE
    enum MODE : Byte {
        MODE_DRAW,
        MODE_REFLECTION,
        MODE_UNDERWATER,
    };

    Bool bump();
    Shader *shader();

    void del();
    void create();
    void prepare();
    void begin();                                 // this is called just before       WaterPlane and WaterMesh drawing
    void end();                                   // this is called at the end of all WaterPlane and WaterMesh drawing
    void under(C PlaneM &plane, WaterMtrl &mtrl); // set if camera is under custom water plane
    void setImages(Image *src, Image *depth);
    void endImages();
    void drawSurfaces();
    void setEyeViewportCam();
#endif

#if !EE_PRIVATE
  private:
#endif
    Bool _max_1_light, _draw_plane_surface, _use_secondary_rt, _began, _swapped_ds;
    Byte _mode, _shader_shadow_maps, _shader_soft, _shader_reflect_env, _shader_reflect_mirror;
    RENDER_TYPE _reflect_renderer;
    Flt _under_step, _offset_nrm, _offset_bump;
    Vec2 _offset_col, _y_mul_add;
    Quad _quad;
    PlaneM _under_plane;
    MeshRender _mshr;
    WaterMtrlPtr _under_mtrl;

    WaterClass();
} extern Water; // water
/******************************************************************************/
struct WaterMesh // manually specified water mesh, water meshes don't support wave scales and custom density on surface (density is taken from 'Water')
{
    Flt depth; // water depth, default=3

    // get
    Bool is() C { return _mshb.is(); }                 // if created
    Bool lake() C { return _lake; }                    // if it's a lake
    Bool under(C Vec &pos, Flt *depth = null) C;       // if 'pos' position is under water, 'depth'=optional parameter to receive the depth of position when it is under water
    C MeshBase &mesh() C { return _mshb; }             // get water mesh
    C Box &box() C { return _box; }                    // get water box (not including depth)
    C WaterMtrlPtr &material() C { return _material; } // get water material
#if EE_PRIVATE
    Shader *shader() C;
    WaterMtrl *getMaterial() C { return _material ? _material() : &Water; }
    void zero();
#endif

    // manage
    void del();
    void create(C MeshBase &src, Bool lake, Flt depth = 3, C WaterMtrlPtr &material = null); // create from 'src' mesh

    // draw
    void draw() C; // draw water area, this should be called only in RM_WATER mode, automatically uses frustum culling

    // io
    Bool save(File &f, CChar *path = null) C; // save, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
    Bool load(File &f, CChar *path = null);   // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

    WaterMesh();

#if !EE_PRIVATE
  private:
#endif
    Bool _lake;
    Box _box;

    MeshBase _mshb;
    MeshRender _mshr;
    WaterMtrlPtr _material;
};
/******************************************************************************/
struct WaterBall : BallM // BallM.r = water ball radius, BallM.pos = world-space position
{
    WaterMtrlPtr material;

    void draw() C; // draw this water ball object, this should be called only in RM_PREPARE mode !! OBJECT MUST REMAIN IN CONSTANT MEMORY ADDRESS UNTIL RENDERING HAS FINISHED !!
#if EE_PRIVATE
    void drawDo() C;
    Bool toScreenRect(Rect &rect) C { return ToFullScreenRect(T, rect); }
#endif

    WaterBall() { _uv_plane.zero(); }

  private:
    Matrix3 _uv_plane;
};
#if EE_PRIVATE
extern Memc<C WaterBall *> WaterBalls;
#endif
/******************************************************************************/
TEX_FLAG CreateWaterBaseTextures(Image &base_0, Image &base_1, Image &base_2, C ImageSource &color, C ImageSource &alpha, C ImageSource &bump, C ImageSource &normal, C ImageSource &smooth, C ImageSource &reflect, C ImageSource &glow, Bool resize_to_pow2 = true, Bool flip_normal_y = false, Bool smooth_is_rough = false); // create 'base_0', 'base_1' and 'base_2' base material textures from given images, textures will be created as IMAGE_R8G8B8A8_SRGB, IMAGE_R8G8_SIGN IMAGE_SOFT, 'flip_normal_y'=if flip normal map Y channel, 'smooth_is_rough'=if smoothness map is actually roughness map, returns bit combination of used textures
TEX_FLAG ExtractWaterBase0Texture(C Image &base_0, Image *color);                                                                                                                                                                                                                                                                // returns bit combination of used textures
TEX_FLAG ExtractWaterBase1Texture(C Image &base_1, Image *normal);                                                                                                                                                                                                                                                               // returns bit combination of used textures
TEX_FLAG ExtractWaterBase2Texture(C Image &base_2, Image *bump);                                                                                                                                                                                                                                                                 // returns bit combination of used textures
/******************************************************************************/
