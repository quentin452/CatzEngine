#pragma once
/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Renderer(func)'  to start rendering using custom rendering function.
   Use 'Renderer()'      to access current rendering phase/mode.
   Use 'Renderer.type()' to get/set rendering type.

/******************************************************************************/
enum RENDER_TYPE : Byte // Rendering Type, please check documentation for more info
{
    RT_DEFERRED, // full             bump mapping, multiple lights, full shadows, full    special effects, good for complex scenes with          many lights
    RT_FORWARD,  // only flat/normal bump mapping, multiple lights, full shadows, limited special effects, good for simple  scenes with low number of lights (ideally 1 directional light only)
    RT_NUM,      // number of Rendering Types
};
#if EE_PRIVATE
ASSERT(RT_DEFERRED == 0); // keep Deferred as the first value as it's the most important renderer, when being first, it will improve 'switch' performance for this renderer
#endif
enum RENDER_MODE : Byte // Rendering Mode, rendering phase of the rendering process
{
    RM_OPAQUE,   // opaque
    RM_OPAQUE_M, // opaque in mirrors/reflections
    RM_OVERLAY,  // overlay mode for rendering semi transparent surfaces onto opaque meshes (like bullet holes)
    RM_EMISSIVE, // emissive
    RM_OUTLINE,  // here you can optionally draw outlines of meshes using 'Mesh.drawOutline'
    RM_BEHIND,   // here you can optionally draw meshes which are behind the visible meshes using 'Mesh.drawBehind'
    RM_FUR,      // fur
    RM_BLEND,    // alpha blending
    RM_SHADOW,   // shadow map, render all shadow casting objects here using 'Mesh.drawShadow', objects not rendered in this phase will not cast shadows
    RM_EARLY_Z,  // early z
    RM_WATER,    // water surfaces
    RM_PALETTE,  // color palette #0 (rendering is performed using 'D.colorPalette'  texture)
    RM_PALETTE1, // color palette #1 (rendering is performed using 'D.colorPalette1' texture)
    RM_PREPARE,  // render all objects here using 'Mesh.draw', and add all lights to the scene using 'Light*.add'

    RM_SHADER_NUM = RM_EARLY_Z + SUPPORT_EARLY_Z, // all modes from start RM_OPAQUE to RM_EARLY_Z are included in the 'MeshPart' shader lookup list
};
enum RENDER_STAGE : Byte // Rendering Stage, allows displaying only desired rendering stages
{
    RS_DEFAULT,      // default, rendering is performed normally
    RS_DEPTH,        // display Depth
    RS_COLOR,        // display Unlit Color        (available only in RT_DEFERRED)
    RS_NORMAL,       // display Normals            (available only in RT_DEFERRED)
    RS_SMOOTH,       // display Smoothness         (available only in RT_DEFERRED)
    RS_REFLECT,      // display Reflectivity       (available only in RT_DEFERRED)
    RS_GLOW,         // display Glow
    RS_EMISSIVE,     // display Emissive           (available only in RT_DEFERRED)
    RS_MOTION,       // display Motion             (available only in RT_DEFERRED)
    RS_LIGHT,        // display Light              (available only in RT_DEFERRED)
    RS_LIGHT_AO,     // display combined Light+AO  (available only in RT_DEFERRED)
    RS_AO,           // display Ambient Occlusion
    RS_LIT_COLOR,    // display Lit Color
    RS_ALPHA,        // display Alpha
    RS_REFLECTION,   // display Reflection         (available if Water reflection is visible)
    RS_WATER_COLOR,  // display Water Unlit Color  (available only in RT_DEFERRED and if 'Water.max1Light' is disabled)
    RS_WATER_NORMAL, // display Water       Normal (available only in RT_DEFERRED and if 'Water.max1Light' is disabled)
    RS_WATER_LIGHT,  // display Water       Light  (available only in RT_DEFERRED and if 'Water.max1Light' is disabled)
#if EE_PRIVATE
    RS_MOTION_CONVERTED,    // display Converted    Motion (available only in RT_DEFERRED)
    RS_MOTION_DILATED_FULL, // display Dilated Full Motion (available only in RT_DEFERRED)
    RS_MOTION_DILATED_BLUR, // display Dilated Blur Motion (available only in RT_DEFERRED)
#endif
};
#if EE_PRIVATE
enum DEPTH_READ_MODE {
    NO_DEPTH_READ,   // default
    WANT_DEPTH_READ, // want if possible, but not required
    NEED_DEPTH_READ, // need and can't run without it
    DEPTH_READ_NUM,  // number of depth read modes
};
#endif
/******************************************************************************/
struct RendererClass // handles rendering
{
    RENDER_STAGE stage;         // display desired rendering stage, default=RS_DEFAULT
    Bool combine,               // if enabled this will blend the final rendered image onto the previous background instead of fully overwriting it, default=false
        alpha,                  // if enabled then when using 'Renderer.target' or 'Renderer.get', result will have alpha channel set based on opacity instead of 1, default=false
        wire,                   // if use wireframe during rendering (not available under OpenGL ES), default=false
        indoor,                 // if current rendering is in-door (has a lot of overlapping occluders), this affects how non-directional lights are rendered, this is only a performance hint (it does not affect rendering result, but only affects performance), default=false
        allow_temporal;         // if allow Temporal effects (Anti-Aliasing and Super-Resolution), default=true
    Color clear_color,          // screen clearing color, default=BLACK, used only for RT_FORWARD renderer
        ms_samples_color;       // visualize multi-sampled pixels, default=TRANSPARENT (off)
    Dbl lowest_visible_point;   // Y coordinates of lowest visible point on the scene, by default=-DBL_MAX (which means full visibility), you can optionally set this to a custom value before the shadow rendering stage, the value should be world-space Y coordinate of the lowest visible point on the scene below which you don't expect any objects to be visible, for example if your scene has heightmaps and there won't be anything visible under the heightmaps, then you can set this value to the minimum of all heightmap mesh box Y coordinates, setting the value will improve shadow quality and rendering performance
    ImageRTC *target;           // render target destination, this can be set to a custom 'ImageRT' of IMAGE_RT mode, in that case the rendering will be performed onto the selected image, this image should have its aspect (proportions) the same as the screen (D.resW, D.resH), if set to null then rendering is performed to the screen, default=null
    ImagePtr cel_shade_palette; // cel shading light palette, you can set this to custom texture in "Init()", if used then this should point to a palette of custom width and 1 pixel height, if set to null then usage of cel shading is disabled, used only in RT_DEFERRED renderer, default=null
    CPtr temporal_id;           // Temporal unique ID, you can use this to allow usage of separate render targets for Temporal effects

    ShaderParam *C material_color_l, // Vec4 Linear Gamma
        *C highlight,                // Vec
        *C vtx_uv_scale;             // Flt

#if EE_PRIVATE
    C VecI2 &res() C { return _res; } // get Current Render Target Size   (in pixels)
    Int resW() C { return _res.x; }   // get Current Render Target Width  (in pixels)
    Int resH() C { return _res.y; }   // get Current Render Target Height (in pixels)

    Int fxW() C { return Min(_col->w(), _final->w()); } // get width  for post process render target (remember that final can be set to VR texture, so we can't use 'D.renderW')
    Int fxH() C { return Min(_col->h(), _final->h()); } // get height for post process render target (remember that final can be set to VR texture, so we can't use 'D.renderH')
    VecI2 fx() C { return VecI2(fxW(), fxH()); }

    Bool inside() C { return _render != null; } // if currently inside rendering process

    void requestMirror(C PlaneM &plane, Int priority, Bool shadows, Int resolution); // request engine to render mirror reflection for 'plane'

    Bool wantDepth() C;
    Bool canReadDepth() C;
    Bool safeCanReadDepth() C;
    Bool canReadDepth1S() C;
    Bool hasDepthAttached() C;
    Bool hasStencilAttached() C;
    Bool wantEdgeSoften() C;
    Bool hasEdgeSoften() C;
    Bool wantEyeAdapt() C;
    Bool hasEyeAdapt() C;
    Bool wantBloom() C;
    Bool hasBloom() C;
    Bool wantMotion() C;
    Bool hasMotion() C;
    Bool wantDof() C;
    Bool hasDof() C;
    Bool wantTemporal() C;
    Bool hasTemporal() C;
    Bool hasAO() C;

    Bool fastCombine() C;
    Bool slowCombine() C;
    Bool processAlpha() C;
    Bool processAlphaFinal() C;
    Bool hasVolLight() C;
    Bool anyDeferred() C;
    Bool anyForward() C;
    Bool ambientInLum() C;
#endif

    // get / set
    RendererClass &type(RENDER_TYPE type);
    RENDER_TYPE type() C { return _type; }          // set/get Rendering Type, default=RT_DEFERRED, the change is NOT instant, avoid calling real-time
    RENDER_MODE operator()() C { return _mode; }    //     get active RENDER_MODE, call this inside custom 'Render' function to obtain current phase of rendering process
    Bool mirror() C { return _mirror; }             // if      currently rendering mirrored/reflection side
    Bool firstPass() C { return _first_pass; }      // if      currently rendering the first   pass (    the main one) of the Forward Renderer
    Bool secondaryPass() C { return !_first_pass; } // if      currently rendering a secondary pass (not the main one) of the Forward Renderer
    RendererClass &forwardPrecision(Bool per_pixel);
    Bool forwardPrecision() C { return _forward_prec; } // set/get precision of the RT_FORWARD renderer, default=true (false for Mobile), the change is NOT instant, avoid calling real-time
    Bool lowDepthPrecision() C;                         //     get if current depth buffer has low precision (16-bits) for which you may want to increase viewport near plane "D.viewFrom" to avoid depth artifacts, such low precision can be encountered on old graphic cards or some mobile devices

    // operations
    RendererClass &operator()(void (&render)()); // perform 3D rendering using custom 'render' callback function
    ImageRTPtr get(void (&render)());            // perform 3D rendering using custom 'render' callback function and return the result as a render target instead of drawing it to the screen

    void setDepthForDebugDrawing(); // call this after rendering (calling the 'Renderer') if you wish to perform 3D debug drawing
    void hasGlow();                 // call this inside 'render' function if you've manually drawn elements on the screen that have glow to notify the engine that glow is visible on the scene (if engine is not notified about glow being present then it may skip this effect), normally you don't need to call this as for example meshes handle this automatically, however you may want to call this for manually drawn graphics
#if EE_PRIVATE
    void finalizeGlow();
#endif

    Bool capture(Image &image, Int w = -1, Int h = -1, Int type = IMAGE_R8G8B8A8_SRGB, Int mode = IMAGE_2D, Int mip_maps = 1, Bool alpha = false); // capture screen to 'image', 'w h'=desired size (in pixels, -1=autodetect), 'type'=IMAGE_TYPE, 'mode'=IMAGE_MODE, 'mip_maps'=number of mip-maps (0=autodetect), 'alpha'=if include alpha transparency
    Bool screenShot(C Str &name, Bool alpha = false);                                                                                              // capture screen and save as 'name'                                                                , sample usage: screenShot ("shot.bmp"   )                 , 'alpha'=if include alpha transparency
    Bool screenShots(C Str &name, C Str &ext, Bool alpha = false);                                                                                 // capture screen and save as 'name' followed by automatically detected number, with 'ext' extension, sample usage: screenShots("shot ","bmp")                 , 'alpha'=if include alpha transparency, this method allows to store screenshot in a folder where screenshots can already exist, in that situation the method will automatically set the new screenshot filename so it will not overwrite any previous files

    // time measurement
    void timeMeasure(Bool on); // enable/disable rendering time measurement, default=false (warning: time measurement may decrease rendering performance)
    Bool timeMeasure() C { return _t_measure; }
    Flt timeReflection() C { return _t_reflection[0]; }
    Flt timePrepare() C { return _t_prepare[0]; }
    Flt timeOpaque() C { return _t_opaque[0]; }
    Flt timeOverlay() C { return _t_overlay[0]; }
    Flt timeWater() C { return _t_water[0]; }
    Flt timeLight() C { return _t_light[0]; }
    Flt timeEmissive() C { return _t_emissive[0]; }
    Flt timeSky() C { return _t_sky[0]; }
    Flt timeWaterUnder() C { return _t_water_under[0]; }
    Flt timeEdgeDetect() C { return _t_edge_detect[0]; }
    Flt timeBlend() C { return _t_blend[0]; }
    Flt timePalette() C { return _t_palette[0]; }
    Flt timeBehind() C { return _t_behind[0]; }
    Flt timeRays() C { return _t_rays[0]; }
    Flt timeVolumetric() C { return _t_volumetric[0]; }
    Flt timePostProcess() C { return _t_post_process[0]; }
    Flt timeGpuWait() C { return _t_gpu_wait[0]; }

#if EE_PRIVATE
    void del();
    void create();

    ImageRTPtr getBackBuffer(); // this method will copy the current render target data to an internal image, and return reference to the image so it can be used as a texture in custom shaders rendering, if this method is used then it should be called only inside RM_BLEND rendering mode or outside the Render function, the returned image should be used only for setting it as a shader texture, do not use the image afterwards

    void getShdRT();
    void getLumRT();
    void getWaterLumRT();

    Bool show(C ImageRTPtr &image, Bool srgb, Bool sign = false, Int channel = -1, Bool inverse = false);
    void setOutline(C Color &color);

    void mode(RENDER_MODE mode);

    void setDS();
    void linearizeDepth(ImageRT &dest, ImageRT &depth); // this reads from depth buffer and stores it to custom render target in linearized mode
    void resolveDepth();
    void resolveDepth1();
    Bool dilateMotion(ImageRTPtr *dilated_full_motion, ImageRTPtr *dilated_blur_motion);
    ImageRT *adaptEye(ImageRTC &src, ImageRT *dest);
    void motionBlur(ImageRT &src, ImageRT &dest, ImageRTPtr &bloom_glow, ImageRTPtr &dilated_motion, Bool alpha, Bool combine, ImageRT *exposure);
    void bloom(ImageRT &src, ImageRT &dest, ImageRTPtr &bloom_glow, Bool alpha, Bool combine, ImageRT *exposure);
    void dof(ImageRT &src, ImageRT &dest, Bool alpha, Bool combine);

    void setAlphaFromDepth();
    void setAlphaFromDepthAndCol();

    Bool reflection();
    void prepare();
    void setForwardCol();
    void opaque();
    void overlay();
    void waterPreLight();
    void light();
    Bool waterPostLight();
    void emissive();
    void sky();
    void waterUnder();
    void edgeDetect();
    void temporalCheck();
    void temporal(ImageRTPtr &dilated_motion);
    void temporalFinish();
    void blend();
    void palette(Int index);
    void behind();
    void edgeSoften();
    void outline();
    void ao();
    void aoApply();
    void resolveMultiSample();
    void downSample();
    void applyOutline();
    void volumetric();
    void postProcess();
    void cleanup();
    void cleanup1();

    // render target methods
#if DX11 // needed on DX11 because it doesn't allow reading and writing to RT's and Depth Buffer at the same time
    static void setDSLookup(); // !! needs to be called after 'set' !! this needs to be called if we plan to call methods below
    static void setDS(ID3D11DepthStencilView *dsv);
    static void needDepthTest(); // !! needs to be called after 'D.depthWrite' !! set only if we need depth writing, or there's no depth buffer attached, otherwise keep current to avoid unnecessary state changes
    static void wantDepthRead();
    static void needDepthRead();
#elif WEB // needed on WEB because it doesn't allow reading and writing to RT's and Depth Buffer at the same time
    static void setDSLookup();
    static void setDS(UInt ds_txtr_id);
    static void needDepthTest();
    INLINE void wantDepthRead() { needDepthTest(); } // works always the same as 'needDepthTest' on WEB
    INLINE void needDepthRead() { setDS(0); }        // never supported on WEB
#elif DEPTH_FLUSH
    INLINE void setDSLookup() {}
    INLINE void needDepthTest() {}
    void needDepthRead();
    INLINE void wantDepthRead() { needDepthRead(); }
#else
    INLINE void setDSLookup() {}
    INLINE void needDepthTest() {}
    INLINE void wantDepthRead() {}
    INLINE void needDepthRead() {}
#endif

    void set(ImageRT *t0, ImageRT *t1, ImageRT *t2, ImageRT *t3, ImageRT *ds, Bool custom_viewport, DEPTH_READ_MODE depth_read_mode = NO_DEPTH_READ);                              // set render targets, 'custom_viewport'=if true then custom viewport based on 'D.viewRect' is used and if false then full viewport is used
    void set(ImageRT *t0, ImageRT *ds, Bool custom_viewport, DEPTH_READ_MODE depth_read_mode = NO_DEPTH_READ) { set(t0, null, null, null, ds, custom_viewport, depth_read_mode); } // set render targets, 'custom_viewport'=if true then custom viewport based on 'D.viewRect' is used and if false then full viewport is used

    Bool swapDS1S(ImageRTPtr &ds_1s);

    void setMainViewportCam(); // set main        viewport and camera
    void setEyeViewportCam();  // set current eye viewport and camera
    Rect *setEyeParams();      // set current eye params without setting the viewport

    void createShadowMap();
    void rtClear();
    void rtClean();
    void rtDel();
    Bool rtCreateMain();
    Bool rtCreate();
    void setMain();
    void update();

    Bool mapMain();
    void unmapMain();

    void setPixelSize(); // must be called if 'Renderer.res' or 'D.size' changes

    inline Vec2 screenToPixel(C Vec2 &screen) C { return ScreenToPixel(screen, _res); }
    inline Rect screenToPixel(C Rect &screen) C { return ScreenToPixel(screen, _res); }
    inline RectI screenToPixelI(C Rect &screen) C { return ScreenToPixelI(screen, _res); }
    static Vec2 screenToPixelSize(C Vec2 &screen);
    static Rect pixelToScreen(C RectI &pixel);
    static Vec2 pixelToScreenSize(Flt pixel);

#if GL
    inline Bool mainFBO(C ImageRT *dest) C { return dest == &_main; } // !! WARNING: this checks only _main, but not _main_ds !!
#endif
#endif

#if !EE_PRIVATE
  private:
#endif
    struct Context {
        struct Sub {
            Bool used;
            Matrix4 proj_matrix_prev;

            Sub();
        };
        ImageRTPtr old_data, old_alpha, old_col,
            new_data, new_alpha, new_col;
        Map<RectI, Sub> subs;
    };
    RENDER_TYPE _type, _cur_type;
    RENDER_MODE _mode;
    ALPHA_MODE _mesh_blend_alpha;
    Bool _has_glow, _forward_prec, _mirror, _mirror_want, _mirror_shadows, _first_pass, _palette_mode, _eye_adapt_scale_cur, _t_measure, _set_depth_needed, _get_target, _stereo, _mesh_early_z, _mesh_shader_vel, _temporal_use, _temporal_reset, _modified_depth;
    Byte _has, _opaque_mode_index, _mesh_stencil_value, _mesh_stencil_mode, _outline, _clear;
    Int _eye, _eye_num, _mirror_priority, _mirror_resolution, _mesh_variation_1;
    UInt _frst_light_offset, _blst_light_offset, _mesh_draw_mask;
    VecI2 _res;
    Vec2 _pixel_size, _pixel_size_inv, _temporal_offset;
    Rect _clip;
    PlaneM _mirror_plane;
    Shader *_shader_early_z, *_shader_shd_map, *_shader_shd_map_skin;
#if SUPPORT_MLAA
    Image _mlaa_area;
#endif
    Image _smaa_area, _smaa_search, _env_dfg, _noise;
    // Image                 _sky_a, _sky_b;
    void (*_render)();
    C Memc<ShaderParamChange> *_shader_param_changes;
    ImageRT _shd_map, _cld_map,
        _eye_adapt_scale[2],
        *_cur[4], *_cur_ds;
    ImageRTC _main, _main_ds,
        _main_temp, _main_temp_ds,
        *_ptr_main, *_ptr_main_ds, // need if have to perform color management
        *_cur_main, *_cur_main_ds,
        *_ui, *_ui_ds,
        *_final;
    ImageRTPtr _h0, _h1, _q0, _q1, // <- these members are to be used only temporarily
        _col, _ds, _ds_1s, _nrm, _ext, _vel, _alpha,
        _lum, _lum_1s, _spec, _spec_1s, _shd_1s, _shd_ms,
        _water_col, _water_nrm, _water_refract, _water_ds, _water_lum, _water_spec,
        _vol, _ao, _fade, _back, _back_ds, _mirror_rt, _outline_rt;
    Memx<ImageRTC> _rts;
    Map<CPtr, Context> _ctxs;
    Context *_ctx;
    Context::Sub _ctx_sub_dummy, *_ctx_sub;
#if EE_PRIVATE
    GPU_API(
        ID3D11RenderTargetView *_cur_id[4], union {UInt _cur_id[4]    ; Ptr _cur_id_ptr[4]    ; });
    GPU_API(
        ID3D11DepthStencilView *_cur_ds_id, union {UInt _cur_ds_id    ; Ptr _cur_ds_id_ptr    ; });
    GPU_API(
        ID3D11DepthStencilView *_cur_ds_ids[3], union {UInt _cur_ds_ids[3]; Ptr _cur_ds_ids_ptr[3]; });
    ASSERT(DEPTH_READ_NUM == 3); // [DEPTH_READ_NUM]
#else
    Ptr _cur_id[4], _cur_ds_id, _cur_ds_ids[3];
#endif

    Int _t_measures[2];
    Dbl _t_last_measure;
    Flt _t_reflection[2], _t_prepare[2], _t_opaque[2], _t_overlay[2], _t_water[2], _t_light[2], _t_emissive[2], _t_sky[2], _t_water_under[2], _t_edge_detect[2], _t_blend[2], _t_palette[2], _t_behind[2], _t_rays[2], _t_volumetric[2], _t_post_process[2], _t_gpu_wait[2];

    RendererClass();
} extern Renderer; // Main Renderer
/******************************************************************************/
#if EE_PRIVATE
extern MeshRender MshrBox, MshrBoxR, MshrBall;

Flt DepthError(Dbl from, Dbl range, Dbl z, Bool perspective, Int bits);

void SetEASU(C Image &src, C Vec2 &dest_size, C Vec2 &screen_lu);

INLINE Bool ReuseDefaultMaterialForNonSkinnedShadowShader(Shader *shader) { return shader == Renderer._shader_shd_map; }   // this is the most common shadow shader, for which we don't use any material properties (except culling) so we can put all instances to default materials to reduce overhead
INLINE Bool ReuseDefaultMaterialForSkinnedShadowShader(Shader *shader) { return shader == Renderer._shader_shd_map_skin; } // this is the most common shadow shader, for which we don't use any material properties (except culling) so we can put all instances to default materials to reduce overhead

Bool SetShaderParamChanges(C Memc<ShaderParamChange> *changes = null);
#endif
void SetDrawMask(UInt draw_mask = 0xFFFFFFFF);                    // set   MeshPart "draw group" bit combination      , this can be called before rendering       meshes                               , draw mask is used to control which MeshParts should be rendered, it is a bit combination of all MeshPart draw groups (which are set using 'MeshPart.drawGroup') that should be included in rendering, each bit is responsible for a different group, for example SetDrawMask(0xFFFFFFFF) enables drawing of all groups, SetDrawMask(IndexToFlag(1) | IndexToFlag(3)) enables drawing of only 1 and 3 group, SetDrawMask(0) completely disables rendering
void SetHighlight();                                              // clear highlight of rendered meshes               , this can be called before rendering       meshes in RM_PREPARE             mode
void SetHighlight(C Color &color);                                // set   highlight of rendered meshes               , this can be called before rendering       meshes in RM_PREPARE             mode
void SetVariation(Int variation = 0);                             // set   material variation of rendered meshes      , this can be called before rendering       meshes in RM_PREPARE             mode, if variation index is out of range, then default #0 variation will be used
void SetStencilValue(Bool terrain = false);                       // set   stencil value applied when rendering meshes, this can be called before rendering       meshes in RM_PREPARE             mode
void SetStencilMode(Bool terrain_only = false);                   // set   stencil mode  applied when rendering meshes, this can be called before rendering blend meshes in RM_PREPARE or RM_BLEND mode
void SetBehindBias(Flt distance);                                 // set   bias tolerance for behind effect in meters , this can be called before rendering       meshes in RM_BEHIND              mode
void SetBlendAlpha(ALPHA_MODE alpha = ALPHA_RENDER_BLEND_FACTOR); // set   custom alpha blending of rendered meshes   , this can be called before rendering       meshes in RM_BLEND               mode
void SetEarlyZ(Bool on = false);                                  // set   early Z of rendered meshes                 , this can be called before rendering       meshes in RM_PREPARE             mode, default=false

void RenderIcon(void (&render)(), C ViewSettings *view, ImageRTC &image, C VecI2 &image_size, Int super_sample = 1); // create icon by performing rendering into 'image', 'render'=rendering function, 'view'=view settings (optional, if null then current 'D.view' settings will be used), 'image'=image render target, 'image_size'=image size in pixels, 'super_sample'=super sampling to be used for rendering (1=disabled, 2=2x, 4=4x, ..), this function ignores 'D.density', 'D.maxLights', 'D.eyeAdaptation', 'D.lodFactor', 'D.motionMode', 'D.temporalAntiAlias', 'D.temporalSuperRes', 'D.ambientRes'. This function can be called at any time (in State Update, Draw, Init, Shut) however only on the main thread.
void RenderIcon(void (&render)(), C ViewSettings *view, Image &image, C VecI2 &image_size, Int super_sample = 1);    // create icon by performing rendering into 'image', 'render'=rendering function, 'view'=view settings (optional, if null then current 'D.view' settings will be used), 'image'=image render target, 'image_size'=image size in pixels, 'super_sample'=super sampling to be used for rendering (1=disabled, 2=2x, 4=4x, ..), this function ignores 'D.density', 'D.maxLights', 'D.eyeAdaptation', 'D.lodFactor', 'D.motionMode', 'D.temporalAntiAlias', 'D.temporalSuperRes', 'D.ambientRes'. This function can be called at any time (in State Update, Draw, Init, Shut) however only on the main thread.
/******************************************************************************
   Use following functions for setting Shader Parameter Changes for specific instances of meshes
   Call in following order: 1. link, 2. draw, 3. unlink, like this:
        LinkShaderParamChanges(changes); mesh.draw(..);
      UnlinkShaderParamChanges(changes);
/******************************************************************************/
void LinkShaderParamChanges(const_mem_addr C Memc<ShaderParamChange> &changes);   // this can  be called before drawing mesh, 'changes' must point to object in constant memory address (only pointer is stored through which the object is later accessed), 'changes' must exist until the rendering is finished
void UnlinkShaderParamChanges(const_mem_addr C Memc<ShaderParamChange> &changes); // this must be called after  drawing mesh
/******************************************************************************/
