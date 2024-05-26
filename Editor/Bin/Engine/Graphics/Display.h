/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Display' to handle all display related functions.
   Including:
      -setting drawing/rendering options
      -handling basic 2D drawing

/******************************************************************************/
enum ASPECT_MODE : Byte // Aspect Ratio Mode, controls setting the sizes of screen coordinates - D.w and D.h
{
    ASPECT_Y,       // sets D.h to 1.0, D.w will be proportional to D.h depending on the display aspect and selected resolution
    ASPECT_X,       // sets D.w to 1.0, D.h will be proportional to D.w depending on the display aspect and selected resolution
    ASPECT_SMALLER, // this mode is useful for Mobile devices which can be rotated and changed aspect will not affect the display scale, 1.0 will be set to D.w and D.h will be set proportionally (if width is smaller than height) and 1.0 will be set to D.h and D.w will be set proportionally (if height is smaller than width)
    ASPECT_NUM,     // number of Aspect Ratio Modes
};
enum DIFFUSE_MODE : Byte // Diffuse Shading Mode
{
    DIFFUSE_LAMBERT,    // fastest
    DIFFUSE_OREN_NAYAR, // based on roughness
    DIFFUSE_BURLEY,     // based on roughness (aka Disney)
    DIFFUSE_NUM,        // number of Diffuse Modes
};
enum BUMP_MODE : Byte // Bump Mapping Mode
{
    BUMP_FLAT,     // flat
    BUMP_NORMAL,   // normal
    BUMP_PARALLAX, // parallax
    BUMP_RELIEF,   // relief
    BUMP_NUM,      // number of Bump Mapping Modes
};
enum AMBIENT_MODE : Byte // Ambient Occlusion Mode
{
    AMBIENT_FLAT,  // flat
    AMBIENT_LOW,   // low    quality
    AMBIENT_MED,   // medium quality
    AMBIENT_HIGH,  // high   quality
    AMBIENT_ULTRA, // ultra  quality
    AMBIENT_NUM,   // number of Ambient Occlusion Modes
};
enum AMBIENT_SOFT_MODE : Byte {
    AMBIENT_SOFT_NUM = 6,
};
enum SHADOW_MODE : Byte // Shadowing Mode
{
    SHADOW_NONE, // none
    SHADOW_MAP,  // shadow mapping
    SHADOW_NUM,  // number of Shadowing Modes
};
enum SHADOW_SOFT_MODE : Byte {
    SHADOW_SOFT_NUM = 6,
};
enum MOTION_MODE : Byte // Motion Blur Mode
{
    MOTION_NONE,          // none
    MOTION_CAMERA,        // screen is blurred according to camera velocities only, objects velocities are not taken into account, objects are treated as if they were all stationary (their velocity is zero)
    MOTION_CAMERA_OBJECT, // screen is blurred according to camera and object velocities, available only in RT_DEFERRED renderer
    MOTION_NUM,           // number of Motion Blur modes
};
enum DOF_MODE : Byte // Depth of Field Mode
{
    DOF_NONE,     // none
    DOF_GAUSSIAN, // based on Gaussian Blur, fast but not realistic
    DOF_NUM,      // number of Depth of Field modes
};
enum EDGE_DETECT_MODE : Byte // Edge Detect Mode
{
    EDGE_DETECT_NONE, // disabled
    EDGE_DETECT_SOFT, // soft edges (slower)
    EDGE_DETECT_THIN, // thin edges
    EDGE_DETECT_NUM,  // number of Edge Detect Modes
};
enum EDGE_SOFTEN_MODE : Byte // Edge Softening Mode
{
    EDGE_SOFTEN_NONE, // disabled
    EDGE_SOFTEN_FXAA, //     Fast Approximate   Anti Aliasing
#if SUPPORT_MLAA
    EDGE_SOFTEN_MLAA, //          Morphological Anti Aliasing
#endif
    EDGE_SOFTEN_SMAA, // Subpixel Morphological Anti Aliasing
    EDGE_SOFTEN_NUM,  // number of Edge Softening Modes
};
enum TEXTURE_USAGE : Byte // Information about using additional texture maps in material
{
    TEX_USE_DISABLE, // map is always disabled
    TEX_USE_SINGLE,  // map is enabled for single materials only
    TEX_USE_MULTI,   // map is enabled for single materials and multiple materials which are blended together
    TEX_USE_NUM,     // number of texture maps usage modes
};
enum FOV_MODE : Byte // Field of View mode, determines calculating actual fov values
{
    FOV_Y,  // vertical   fov will be taken from given value, horizontal fov will be calculated proportionally
    FOV_X,  // horizontal fov will be taken from given value, vertical   fov will be calculated proportionally
    FOV_XY, // both horizontal and vertical fov's will be taken from given value
#if EE_PRIVATE
    FOV_ORTHO_Y, // orthogonal, vertical   fov will be taken from 'fov', horizontal will be calculated proportionally
    FOV_ORTHO_X, // orthogonal, horizontal fov will be taken from 'fov', vertical   will be calculated proportionally
    FOV_ORTHO,   // orthogonal, custom     fov will be taken from 'fov'
#endif
};
enum SHADER_MODEL : Byte {
    SM_UNKNOWN,   // unknown
    SM_GL_ES_3,   //           (OpenGL ES      3.0)
    SM_GL_ES_3_1, //           (OpenGL ES      3.1)
    SM_GL_ES_3_2, //           (OpenGL ES      3.2)
    SM_GL_3,      //           (OpenGL Desktop 3.2)
    SM_GL_4,      //           (OpenGL Desktop 4.0)
    SM_GL_4_2,    //           (OpenGL Desktop 4.2)
    SM_GL_4_3,    //           (OpenGL Desktop 4.3)
    SM_4,         // Model 4.0 (DirectX 10        )
    SM_4_1,       // Model 4.1 (DirectX 10.1      )
    SM_5,         // Model 5.0 (DirectX 11        )
    SM_6,         // Model 6.0 (DirectX 12        )
    SM_6_2,       // Model 6.2 (DirectX 12        )
};
#if EE_PRIVATE
inline Bool FovPerspective(FOV_MODE mode) { return mode < FOV_ORTHO_Y; }                  // if fov mode is perspective
inline Bool FovOrthogonal(FOV_MODE mode) { return mode >= FOV_ORTHO_Y; }                  // if fov mode is orthogonal
inline Bool FovHorizontal(FOV_MODE mode) { return mode == FOV_X || mode == FOV_ORTHO_X; } // if fov mode is horizontal
#endif
struct ViewSettings {
    FOV_MODE fov_mode;
    Flt from, range, fov;
};
/******************************************************************************/
struct DisplayClass : DisplayState, DisplayDraw // Display Control
{
    void (*screen_changed)(Flt old_width, Flt old_height); // pointer to custom function (may be null) called when screen proportions have changed

    Flt (*shadow_step)(Int i, Int num); // pointer to custom function (may be null) which allows to manually specify the fraction range of the i-th out of 'num' shadow map splits

    Int (*image_load_shrink)(ImageHeader &image_header, C Str &name); // pointer to custom function (may be null) called when 'Image' is being loaded from a File allowing to load the Image as smaller than how it is stored in the File. 'name'=file name of the image. You can use this for example when targeting devices with small amount of RAM and wishing to load the Images as half their size. Inside the function you should return by how many steps the final size should be divided by 2. For example, if an Image is being loaded with its original size = 1024x1024, based on returned value the final size will be adjusted accordingly: return value 0 -> 1024x1024 (original size), return value 1 -> 512x512 (half size), return value 2 -> 256x256 (quarter size), and so on.

    void (*set_shader)(); // pointer to custom function (may be null) called when Mesh shaders need to be updated, when changing some display options during application runtime, meshes require to have their shaders updated, Meshes created by using Cache are processed automatically, however manually created meshes need to be processed manually, to do this - create a global function which calls 'Mesh.setShader()' on all manually created meshes and then in 'InitPre' function set 'D.set_shader' to point to that function

    // get
    C Vec2 &size() C { return _rect.max; }                     // get Application           Screen     Size     (in screen coordinates, depends on selected resolution, aspect mode and scale
    Flt w() C { return _rect.max.x; }                          // get Application           Screen     Width    (in screen coordinates, depends on selected resolution, aspect mode and scale, usually something near 1.3)
    Flt h() C { return _rect.max.y; }                          // get Application           Screen     Height   (in screen coordinates, depends on selected resolution, aspect mode and scale, usually something near 1.0)
    C Vec2 &size2() C { return _size2; }                       // get Application           Screen     Size  *2 (in screen coordinates, depends on selected resolution, aspect mode and scale
    Flt w2() C { return _size2.x; }                            // get Application           Screen     Width *2 (in screen coordinates, depends on selected resolution, aspect mode and scale, usually something near 2.6)
    Flt h2() C { return _size2.y; }                            // get Application           Screen     Height*2 (in screen coordinates, depends on selected resolution, aspect mode and scale, usually something near 2.0)
    C VecI2 &res() C { return _res; }                          // get Application           Resolution          (in pixels)
    Int resW() C { return _res.x; }                            // get Application           Resolution Width    (in pixels)
    Int resH() C { return _res.y; }                            // get Application           Resolution Height   (in pixels)
    C VecI2 &render() C { return _render_res; }                // get Application Rendering Resolution          (in pixels), which is Resolution affected by 'density'
    Int renderW() C { return _render_res.x; }                  // get Application Rendering Resolution Width    (in pixels), which is Resolution affected by 'density'
    Int renderH() C { return _render_res.y; }                  // get Application Rendering Resolution Height   (in pixels), which is Resolution affected by 'density'
    VecI2 screen() C;                                          // get          Main Display Screen              (in pixels) at the current moment
    Int screenW() C { return screen().x; }                     // get          Main Display Screen     Width    (in pixels) at the current moment
    Int screenH() C { return screen().y; }                     // get          Main Display Screen     Height   (in pixels) at the current moment
    Int freq() C { return _freq_got; }                         // get               Display Screen     Frequency (refresh rate)
    Bool hdr() C { return _hdr; }                              // get            if Display Screen is in HDR mode, to enable HDR rendering - see 'outputPrecision' and 'litColRTPrecision'
    IMAGE_PRECISION colorPrecision() C { return _color_prec; } // get               Display Screen Color   Precision (bits per color, default=IMAGE_PRECISION_8 which is 8-bits per color)
    Flt whiteLum() C { return _white_lum; }                    // get               Display Screen White   Luminance in Color values (default=1), this will always be 1 when not in HDR mode, in HDR this can be higher
    Flt screenMaxLum() C { return _screen_max_lum; }           // get               Display Screen Maximum Luminance in Color values (default=1), this will always be 1 when not in HDR mode, in HDR this can be higher
    Flt screenNits() C { return _screen_nits; }                // get               Display Screen Maximum Luminance in Nits         (default=0=unknown)
    SHADER_MODEL shaderModel() C { return _shader_model; }     // get available Shader Model
    Str8 shaderModelName() C;                                  // get available Shader Model in text format
    Str8 apiName() C;                                          // get Rendering API          in text format
    Int maxTexFilter() C { return _max_tex_filter; }           // get maximum available Anisotropic Filtering
    Int maxTexSize() C { return _max_tex_size; }               // get maximum available Texture     Size
    C Mems<VecI2> &modes() C { return _modes; }                // get fullscreen display modes, where x=width, y=height (in pixels)
    C Str8 &deviceName() C { return _device_name; }            // get GPU device name
    Long deviceMemory() C { return _device_mem; }              // get GPU memory (-1 if unknown)
    Bool smallSize() C;                                        // if  display device is of a small size (phone size)
    Flt browserZoom() C;                                       // get current browser zoom level (1.0=100%, 2.0=200%, etc), this is valid only for the WEB platform (other platforms always return 1.0)

    // settings
    DisplayClass &mode(Int w = -1, Int h = -1, Int full = -1, Bool auto_full = true); // set    Display Resolution  Mode, 'w'=width in pixels, 'h'=height in pixels, 'full'=if fullscreen (true/false), 'auto_full'=if allow forcing fullscreen when mode covers entire screen. Specifying -1 for any parameter means keep current value, if the method fails then previous mode is restored, if previous mode can't be restored then 'Exit' is called
    DisplayClass &toggle(Bool window_size = false);                                   // toggle Fullscreen/Windowed Mode, 'window_size'=if when switching to fullscreen want to set resolution from current window size instead of desktop size
    DisplayClass &full(Bool full, Bool window_size = false);                          // set    Fullscreen Mode         , 'window_size'=if when switching to fullscreen want to set resolution from current window size instead of desktop size
    Bool full() C { return _full; }                                                   // if in  Fullscreen Mode (true/false, default=false)

#if EE_PRIVATE
    void setSync();
    void gammaSet();
    Bool densityFast(Byte density);
    void densityFast1() { densityFast(127); }
    void densityUpdate();
    Bool ambientResFast(Byte scale);
    void ambientResFast1() { ambientResFast(255); }
    void setColorLUT();
    void getScreenInfo();
    Bool densityUsed() C { return _density != 127; }    // get if Density is != 1.0
    Bool densityUpsample() C { return _density > 127; } // get if Density is >  1.0
    Byte densityFast() C { return _density; }           // get    Density
    Byte ambientResFast() C { return _amb_res; }        // get    Ambient Resolution
    Bool multiSample() C { return _samples > 1; }       // get if Multi Sampling is used
    void aspectRatioEx(Bool force = true, Bool quiet = false);
    constexpr Bool signedNrmRT() C { return false; } // if Normal   Render Target is signed #SIGNED_NRM_RT
    constexpr Bool signedVelRT() C { return true; }  // if Velocity Render Target is signed
    Flt eyeDistance_2() C { return _eye_dist_2; }
    Bool exclusiveFull() C;                           // if actually in exclusive full-screen mode
    Bool colorManaged() C { return _color_lut.is(); } // if need to perform any color transformations
#endif
    C Rect &rect() C { return _rect; }      // get     full screen rectangle Rect(-w(), -h(), w(), h())
    C Rect &rectUI() C { return _rect_ui; } // get     full screen rectangle that's safe for UI placement, normally it's equal to 'rect' however it may be smaller due to system controls, rounded corners, notch, etc.
    Rect rectUIKB() C;                      // get     full screen rectangle that's safe for UI placement, excluding Screen Keyboard
    DisplayClass &exclusive(Bool exclusive);
    Bool exclusive() C { return _exclusive; } // get/set if fullscreen mode should be exclusive (true/false                         , default=            false                             ), this affects only Windows DirectX fullscreen mode, exclusive offers better performance, non-exclusive offers faster Alt+Tab switching
    DisplayClass &colorSpace(COLOR_SPACE color_space);
    COLOR_SPACE colorSpace() C { return _color_space; } // get/set if App should be color managed         (COLOR_SPACE                        , default= COLOR_SPACE_NONE                             ), if enabled then Application will convert colors from specified color space into monitor color space, based on selected monitor color profile in the Operating System. If there's no monitor color profile selected in the Operating System, then this option will have no effect. Using COLOR_SPACE_NONE disables color management, while other modes enable it. Warning: enabling color management reduces performance.
    DisplayClass &density(Flt density);
    Flt density() C; // get/set Rendering Pixel Density                (0..2                               , default=                1                             ), density affects the number of pixels used during rendering, density=2 makes the rendering use 2x bigger render targets (super sampling) but slower performance, density=0.5 makes the rendering use 2x smaller render targets making the result more blurry but with faster performance, the change is NOT instant, avoid calling real-time
    DisplayClass &densityFilter(FILTER_TYPE filter);
    FILTER_TYPE densityFilter() C { return _density_filter; } // get/set Density Filtering                      (FILTER_TYPE                        , default=      FILTER_EASU                             ), density filter affects filtering used when up-scaling the image, this method supports only FILTER_NONE, FILTER_LINEAR, FILTER_CUBIC_FAST and FILTER_CUBIC_PLUS, the change is instant, you can call it real-time
    DisplayClass &sharpen(Bool sharpen);
    Bool sharpen() C { return _sharpen; } // get/set Rendering Sharpening                   (true/false                         , default=            false                             ), the change is instant, you can call it real-time
    DisplayClass &sharpenIntensity(Flt intensity);
    Flt sharpenIntensity() C { return _sharpen_intensity; } // get/set Rendering Sharpening Intensity         (0..1                               , default=                0                             ), the change is instant, you can call it real-time
    DisplayClass &samples(Byte samples);
    Byte samples() C { return _samples; } // set/get Sample Count for Multi Sampling AA     (1..16                              , default=                1                             ), multi-sampling for RT_DEFERRED renderer can be enabled only for ShaderModel>=4.1, enabling multi-sampling for ShaderModel<=4.0 disables all depth-based effects, the change is NOT instant, avoid calling real-time
    DisplayClass &scale(Flt scale);
    Flt scale() C { return _scale; } // get/set Draw Scale                             (0..Inf                             , default=                1                             ), changing display scale affects display screen sizes D.w and D.h, the change is NOT instant, avoid calling real-time
    DisplayClass &aspectMode(ASPECT_MODE mode);
    ASPECT_MODE aspectMode() C { return _aspect_mode; } // set/get Aspect Mode                            (ASPECT_MODE                        , default=         ASPECT_Y (ASPECT_SMALLER  for Mobile))
    DisplayClass &aspectRatio(Flt aspect_ratio);
    Flt aspectRatio() C { return _disp_aspect_ratio; } // set/get Display Aspect Ratio                   (0=autodetect, 4/3, 5/4, 16/9, 16/10, default=                0                             )
    DisplayClass &diffuseMode(DIFFUSE_MODE mode);
    DIFFUSE_MODE diffuseMode() C { return _diffuse_mode; } // set/get Diffuse Shading Mode                   (DIFFUSE_MODE                       , default=   DIFFUSE_BURLEY (DIFFUSE_LAMBERT for Mobile)), this affects only RT_DEFERRED renderer, RT_FORWARD always uses DIFFUSE_LAMBERT, the change is instant, you can call it real-time
    DisplayClass &bumpMode(BUMP_MODE mode);
    BUMP_MODE bumpMode() C { return _bump_mode; } // set/get Bump Mapping Mode                      (BUMP_MODE                          , default=      BUMP_RELIEF (BUMP_NORMAL     for Mobile)), using this method during application runtime requires setting 'D.set_shader' (please check comments on 'D.set_shader' for more info), the change is NOT instant, avoid calling real-time
    DisplayClass &gamma(Flt gamma);
    Flt gamma() C { return _gamma; } // set/get Gamma value                            (-Inf..Inf                          , default=                0                             ), using gamma modifies hardware color processing and does not reduce rendering performance
    DisplayClass &highPrecColRT(Bool on);
    Bool highPrecColRT() C { return _hp_col_rt; } // set/get Color     Render Target High Precision (true/false                         , default=            false                             ), enabling high precision render targets gives higher quality graphics at the cost of slower performance and more memory usage, high precision means that Render Targets are created using a different format, that is more precise but also uses more memory and bandwidth, Color     Render Target is used for storing colors                   in Deferred Renderer , this affects precision of color textures adjusted by material colors, the change is NOT instant, avoid calling real-time
    DisplayClass &highPrecNrmRT(Bool on);
    Bool highPrecNrmRT() C { return _hp_nrm_rt; } // set/get Normal    Render Target High Precision (true/false                         , default=            false                             ), enabling high precision render targets gives higher quality graphics at the cost of slower performance and more memory usage, high precision means that Render Targets are created using a different format, that is more precise but also uses more memory and bandwidth, Normal    Render Target is used for storing normal vectors           in Deferred Renderer , this affects lighting calculations (especially specular) to be more precise, the change is NOT instant, avoid calling real-time
    DisplayClass &highPrecLumRT(Bool on);
    Bool highPrecLumRT() C { return _hp_lum_rt; } // set/get Light     Render Target High Precision (true/false                         , default=            false                             ), enabling high precision render targets gives higher quality graphics at the cost of slower performance and more memory usage, high precision means that Render Targets are created using a different format, that is more precise but also uses more memory and bandwidth, Light     Render Target is used for storing light  intensity         in Deferred Renderer , most importantly it allows to store light intensities with values higher than 1.0 (which is the limit for low precision render targets), the change is NOT instant, avoid calling real-time
    DisplayClass &highPrecPalRT(Bool on);
    Bool highPrecPalRT() C { return _hp_pal_rt; } // set/get Palette   Render Target High Precision (true/false                         , default=            false                             ), enabling high precision render targets gives higher quality graphics at the cost of slower performance and more memory usage, high precision means that Render Targets are created using a different format, that is more precise but also uses more memory and bandwidth, Palette   Render Target is used for storing particles in RM_PALETTE                       , the change is NOT instant, avoid calling real-time
    DisplayClass &litColRTPrecision(IMAGE_PRECISION precision);
    IMAGE_PRECISION litColRTPrecision() C { return _lit_col_rt_prec; } // set/get Lit Color Render Target      Precision (IMAGE_PRECISION                    , default=IMAGE_PRECISION_8                             ), enabling high precision render targets gives higher quality graphics at the cost of slower performance and more memory usage, high precision means that Render Targets are created using a different format, that is more precise but also uses more memory and bandwidth, Lit Color Render Target is used for storing colors adjusted by light in all      Renderers, the change is NOT instant, avoid calling real-time
    DisplayClass &outputPrecision(IMAGE_PRECISION precision);
    IMAGE_PRECISION outputPrecision() C { return _output_prec; } // set/get                       Output Precision (IMAGE_PRECISION                    , default=IMAGE_PRECISION_8                             ), use this function to specify Output Render Target precision, use IMAGE_PRECISION_8 for 8-bit per-channel (24-bit total) screens, IMAGE_PRECISION_10 for 10-bit per channel (30-bit total) screens, and IMAGE_PRECISION_16 for 16-bit per channel (64-bit total) HDR screens, avoid setting higher precision than what your screen can actually support because instead of getting higher quality results you will get lower quality
    DisplayClass &dither(Bool on);
    Bool dither() C { return _dither; } // set/get Color Dithering                        (true/false                         , default=             true                             ), the change is instant, you can call it real-time
    DisplayClass &sync(Bool sync);
    Bool sync() C { return _sync; } // set/get Screen Synchronization                 (true/false
    DisplayClass &autosavescript(Bool autosavescript);
    Bool autosavescript() C { return _autosavescript; } // set/get Auto Save Script                 (true/false                         , default=             true                             ), the change is NOT instant, avoid calling real-time
    DisplayClass &maxLights(Byte max_lights);
    Byte maxLights() C { return _max_lights; } // set/get Maximum Number of Lights               (0=unlimited, 1..255                , default=                0                             ), this will automatically limit the number of lights on the scene, the change is instant, you can call it real-time
    DisplayClass &maxLightsSoft(Bool on);
    Bool maxLightsSoft() C { return _max_lights_soft; } // set/get Maximum Number of Lights Soft Mode     (true/false                         , default=             true                             ), this allows the light limit number to be soft/relaxed, allowing number of lights to temporarily exceed the limit in order to smoothly fade lights out
    DisplayClass &materialBlend(Bool per_pixel);
    Bool materialBlend() C { return _mtrl_blend; } // set/get Multi Material Blending Quality        (true/false                         , default=             true                             ), false=per vertex, true=per pixel, in order to get correct results, your materials should have a bump map, the change is NOT instant, avoid calling real-time
    DisplayClass &texFilter(Byte filter);
    Byte texFilter() C { return _tex_filter; } // set/get Texture Filtering Quality              (0..maxTexFilter()                  , default=               16 (4               for Mobile)), 0=no filtering, 1=linear filtering, 2 and more=anisotropic filtering, the change is NOT instant, avoid calling real-time
    DisplayClass &texMipFilter(Bool on);
    Bool texMipFilter() C { return _tex_mip_filter; } // set/get Texture MipMap Filtering               (true/false                         , default=             true                             ), enables or disables filtering between different mip map levels, the change is NOT instant, avoid calling real-time
    DisplayClass &texMipMin(Byte min);
    Byte texMipMin() C { return _tex_mip_min; } // set/get Texture Minimum MipMap                 (0..16                              , default=                0                             ), values bigger than zero indicate lower texture quality (smaller mip maps will be used), but faster rendering, the change is NOT instant, avoid calling real-time
#if EE_PRIVATE
    DisplayClass &texMipBias(Flt bias);
    Flt texMipBias() C { return _tex_mip_bias; } // set/get Texture MipMap Bias                    (-Inf..Inf                          , default=                0                             ), value >0 decreases sharpness (smaller mip maps will be used), value==0 doesn't change sharpness, value<0 increases sharpness (bigger mip maps will be used), the change is NOT instant, avoid calling real-time
#endif
    DisplayClass &texMacro(Bool use);
    Bool texMacro() C { return _tex_macro; } // set/get Macro  Textures usage                  (true/false                         , default=             true                             ), determines usage of additional Macro  Textures in Materials, using this method during application runtime requires setting 'D.set_shader' (please check comments on 'D.set_shader' for more info), the change is NOT instant, avoid calling real-time
    DisplayClass &texDetail(TEXTURE_USAGE usage);
    TEXTURE_USAGE texDetail() C { return _tex_detail; } // set/get Detail Textures usage                  (TEXTURE_USAGE                      , default=    TEX_USE_MULTI (TEX_USE_DISABLE for Mobile)), determines usage of additional Detail Textures in Materials, using this method during application runtime requires setting 'D.set_shader' (please check comments on 'D.set_shader' for more info), the change is NOT instant, avoid calling real-time
    DisplayClass &texDetailLOD(Bool on);
    Bool texDetailLOD() C { return _tex_detail_lod; } // set/get Detail Textures on all Mesh LODs       (true/false                         , default=            false                             ), determines usage of additional Detail Textures in Materials, using this method during application runtime requires setting 'D.set_shader' (please check comments on 'D.set_shader' for more info), the change is NOT instant, avoid calling real-time, if this option is enabled then all LODs in a Mesh will use detail (if available), if disabled then only first LOD will use it
    DisplayClass &imageMipBias(Flt bias);
    Flt imageMipBias() C { return _image_mip_bias; } // set/get Image MipMap Bias                      (-Inf..Inf                          , default=            -0.50                             ), specifies Mip Map Texture Bias for regular Images, value >0 decreases sharpness (smaller mip maps will be used), value==0 doesn't change sharpness, value<0 increases sharpness (bigger mip maps will be used), the change is NOT instant, avoid calling real-time
    DisplayClass &fontMipBias(Flt bias);
    Flt fontMipBias() C { return _font_mip_bias; } // set/get Font  MipMap Bias                      (-Inf..Inf                          , default=            -0.75                             ), specifies Mip Map Texture Bias for Font    Images, value >0 decreases sharpness (smaller mip maps will be used), value==0 doesn't change sharpness, value<0 increases sharpness (bigger mip maps will be used), the change is NOT instant, avoid calling real-time
    DisplayClass &bendLeafs(Bool on);
    Bool bendLeafs() C { return _bend_leafs; } // set/get Leafs Bending animation                (true/false                         , default=             true                             ), using this method during application runtime requires setting 'D.set_shader' (please check comments on 'D.set_shader' for more info), the change is NOT instant, avoid calling real-time
    DisplayClass &outlineMode(EDGE_DETECT_MODE mode);
    EDGE_DETECT_MODE outlineMode() C { return _outline_mode; } // set/get Mesh Outline   Mode                    (EDGE_DETECT_MODE                   , default= EDGE_DETECT_THIN                             ), the change is instant, you can call it real-time
    DisplayClass &edgeDetect(EDGE_DETECT_MODE mode);
    EDGE_DETECT_MODE edgeDetect() C { return _edge_detect; } // set/get Edge Detect    Mode                    (EDGE_DETECT_MODE                   , default= EDGE_DETECT_NONE                             ), the change is instant, you can call it real-time
    DisplayClass &edgeSoften(EDGE_SOFTEN_MODE mode);
    EDGE_SOFTEN_MODE edgeSoften() C { return _edge_soften; } // set/get Edge Softening Mode                    (EDGE_SOFTEN_MODE                   , default= EDGE_SOFTEN_NONE                             ), this is fake Anti-Aliasing, it is not used when Multi Sampling is enabled, the change is instant, you can call it real-time
    DisplayClass &smaaThreshold(Flt threshold);
    Flt smaaThreshold() C { return _smaa_threshold; } // set/get Edge Softening SMAA Threshold          (   0..1                            , default=            0.1                               ), this affects SMAA edge softening quality, lower values give better quality, while higher values give better performance, recommended values: 0.05=high quality, 0.1=medium, 0.15=high performance
    DisplayClass &eyeDistance(Flt dist);
    Flt eyeDistance() C { return _eye_dist; } // set/get distance between the eyes              (-Inf..Inf                          , default=            0.064                             ), interpupillary distance (distance between the eye pupils) used for stereoscopic rendering, the change is instant, you can call it real-time
    DisplayClass &drawNullMaterials(Bool on);
    Bool drawNullMaterials() C { return _draw_null_mtrl; } // set/get if draw mesh parts with no material    (true/false                         , default=            false                             ), the change is NOT instant, avoid calling real-time
    DisplayClass &secondaryOpenGLContexts(Byte contexts);
    Int secondaryOpenGLContexts() C;         // set/get number of secondary OpenGL contexts to create during application initialization (this does not include the main context for the main thread which is always created), each secondary context allows 1 secondary thread to perform operations on the GPU data, for more information please check the 'ThreadMayUseGPUData' global function, this should be called only in 'InitPre', after that, calls to this method are ignored, this value is used only for OpenGL renderer, for DirectX it is ignored, default=1
    DisplayClass &shaderCache(C Str &path);  // set     path for OpenGL Shader Cache
    Bool canUseGPUDataOnSecondaryThread() C; //     get if display supports operating on GPU data on a secondary thread, this is always true for DirectX, for OpenGL it will be true only if secondary OpenGL contexts were successfully created during the display initialization
    static Bool created();                   //     get if display is created, this can be false when the Engine still hasn't finished initializing (before 'Init'), or on Linux if XDisplay is not available and APP_ALLOW_NO_XDISPLAY was specified

    // screen fading
    Bool fading() C;                                                          // if    fading is currently enabled
    void setFade(Flt seconds, Bool immediate = false, Bool auto_draw = true); // start fading screen for the following 'seconds', 'immediate'=if make an immediate copy of the screen when calling this function with current application state (this is slower because it re-draws entire screen, but uses current application state), false=use the next frame result (faster but uses next frame which may not have current application state), 'auto_draw'=if automatically perform fade drawing (if false then 'drawFade' has to be called manually)
    void clearFade();                                                         // clear any active fading, the change is instant, you can call it real-time
    Bool drawFade() C;                                                        // manually draw fading, this has to be called only if 'auto_draw' was set to false in 'setFade', returns true if fade was drawn on the screen

    // Color Palette
    DisplayClass &colorPaletteAllow(Bool on);
    Bool colorPaletteAllow() C { return _color_palette_allow; } // set/get if RM_PALETTE/RM_PALETTE1 rendering modes are allowed, disabling them increases rendering performance, default=true (false for Mobile)
    DisplayClass &colorPalette(C ImagePtr &palette);
    C ImagePtr &colorPalette() C { return _color_palette[0]; } // set/get color palette image that is used during RM_PALETTE  rendering mode, palette textures need to have a height of 4 pixels (width can be set freely), where each row represents an intensity color palette for respectively (red, green, blue, alpha) components when rendering in RM_PALETTE  mode, default=ImagePtr().get("Img/Color Palette.img")
    DisplayClass &colorPalette1(C ImagePtr &palette);
    C ImagePtr &colorPalette1() C { return _color_palette[1]; } // set/get color palette image that is used during RM_PALETTE1 rendering mode, palette textures need to have a height of 4 pixels (width can be set freely), where each row represents an intensity color palette for respectively (red, green, blue, alpha) components when rendering in RM_PALETTE1 mode, default=null

    // Particles
    DisplayClass &particlesSoft(Bool on);
    Bool particlesSoft() C { return _particles_soft; } // set/get Particles Softing          (true/false, default=true                   ), the change is instant, you can call it real-time
    DisplayClass &particlesSmoothAnim(Bool on);
    Bool particlesSmoothAnim() C { return _particles_smooth; } // set/get Particles Smooth Animation (true/false, default=true (false for Mobile)), if enabled then particles with animated images will be displayed with better quality by smooth blending between animation frames, the change is instant, you can call it real-time

    // Temporal
    DisplayClass &temporalAntiAlias(Bool on);
    Bool temporalAntiAlias() C { return _temp_anti_alias; } // set/get Temporal Anti-Aliasing    (true/false, default=false), this is  Anti-Aliasing that jitters projection matrix per-frame, moving it slightly with every frame and accumulating rendering results over time
    DisplayClass &temporalSuperRes(Bool on);
    Bool temporalSuperRes() C { return _temp_super_res; } // set/get Temporal Super Resolution (true/false, default=false), this is Super-Sampling that jitters projection matrix per-frame, moving it slightly with every frame and accumulating rendering results over time. This option Lowers Quality but Increases Performance. It works by rendering to a lower resolution and then upscaling back to original resolution.
    DisplayClass &temporalReset();                        // reset   Temporal history                                     , call this method if you want to clear the history of previous rendering results
#if EE_PRIVATE
    Bool temporal() C { return temporalAntiAlias() || temporalSuperRes(); }
#endif

    // Bloom, setting 'original' value to 1 and 'scale' to 0 disables bloom and increases rendering performance, optionally you can disable it with "bloomAllow(false)"
#if EE_PRIVATE
    void bloomScaleCut(Flt scale, Flt cut);
    Flt bloomAdd() C { return _bloom_add; }
#endif
    DisplayClass &glowAllow(Bool allow);
    Bool glowAllow() C { return _glow_allow; } // set/get Allow Glow Effect     (true/false, default=true (false for Mobile)), this can work only if 'bloomAllow' is enabled, the change is instant, you can call it real-time
    DisplayClass &bloomAllow(Bool allow);
    Bool bloomAllow() C { return _bloom_allow; } // set/get Allow Bloom           (true/false, default=true (false for Mobile)), the change is instant, you can call it real-time
    DisplayClass &bloomOriginal(Flt original);
    Flt bloomOriginal() C { return _bloom_original; } // set/get Bloom Original Color  (   0..Inf , default=1.0                    ), the change is instant, you can call it real-time
    DisplayClass &bloomScale(Flt scale);
    Flt bloomScale() C { return _bloom_mul; } // set/get Bloom Scale           (   0..Inf , default=0.8                    ), the change is instant, you can call it real-time
    DisplayClass &bloomCut(Flt cut);
    Flt bloomCut() C { return _bloom_cut; } // set/get Bloom Cutoff          (   0..Inf , default=0.3                    ), the change is instant, you can call it real-time
    DisplayClass &bloomGlow(Flt glow);
    Flt bloomGlow() C { return _bloom_glow; } // set/get Bloom from Glow       (   0..Inf , default=1.0                    ), the change is instant, you can call it real-time
    Bool bloomUsed() C;                       //     if  Bloom post process is going to be used

    // Contrast
    DisplayClass &contrast(Flt contrast);
    Flt contrast() C { return _contrast; } // set/get contrast (0..Inf, 0=disabled, default=0.8), the change is instant, you can call it real-time
#if EE_PRIVATE
    Bool useContrast() C { return _contrast >= HALF_MIN; }
    void setContrast() C;
#endif

    // Ambient Light
#if EE_PRIVATE
    Bool aoWant() C;
    Bool aoAll() C { return _ao_all; }
    void ambientSet() C;
    void ambientSetRange() C;
#if LINEAR_GAMMA
    INLINE C Vec &ambientColorD() C { return ambientColorL(); }
    INLINE C Vec &nightShadeColorD() C { return nightShadeColorL(); }
#else
    INLINE Vec ambientColorD() C { return ambientColorS(); }
    INLINE Vec nightShadeColorD() C { return nightShadeColorS(); }
#endif
#endif
    DisplayClass &ambientMode(AMBIENT_MODE mode);
    AMBIENT_MODE ambientMode() C { return _amb_mode; } // set/get Ambient Mode               (AMBIENT_MODE         , default=AMBIENT_LOW), the change is instant, you can call it real-time
    DisplayClass &ambientSoft(Byte soft);
    Byte ambientSoft() C { return _amb_soft; } // set/get Ambient Softing            (0..AMBIENT_SOFT_NUM-1, default=          1), if soften the AO result, the change is instant, you can call it real-time
    DisplayClass &ambientJitter(Bool jitter);
    Bool ambientJitter() C { return _amb_jitter; } // set/get Ambient Jittering          (true/false           , default=       true), jittering enables per pixel randomization of the AO samples, the change is instant, you can call it real-time
    DisplayClass &ambientNormal(Bool normal);
    Bool ambientNormal() C { return _amb_normal; } // set/get Ambient Normal Map Use     (true/false           , default=       true), if include per pixel normal vectors from the render target, the change is instant, you can call it real-time
    DisplayClass &ambientRes(Flt scale);
    Flt ambientRes() C; // set/get Ambient Resolution         (0..1                 , default=        0.5), this determines the size of the buffers used for calculating the Ambient Occlusion effect, 1=full size, 0.5=half size, 0.25=quarter size, .., smaller sizes offer faster performance but worse quality, the change is NOT instant, avoid calling real-time
    DisplayClass &ambientPowerL(Flt lin_power);
    Flt ambientPowerL() C { return _amb_color_l.max(); } // set/get Ambient Power Linear Gamma (0..1                                      ), this is equivalent to using 'ambientColorL' with RGB components set to the same value, the change is instant, you can call it real-time
    DisplayClass &ambientPowerS(Flt srgb_power);
    Flt ambientPowerS() C; // set/get Ambient Power sRGB   Gamma (0..1                 , default=      0.366), this is equivalent to using 'ambientColorS' with RGB components set to the same value, the change is instant, you can call it real-time
    DisplayClass &ambientColorL(C Vec &lin_color);
    C Vec &ambientColorL() C { return _amb_color_l; } // set/get Ambient Color Linear Gamma (0..1                                      ), the change is instant, you can call it real-time
    DisplayClass &ambientColorS(C Vec &srgb_color);
    Vec ambientColorS() C; // set/get Ambient Color sRGB   Gamma (0..1                 , default=      0.366), the change is instant, you can call it real-time
    DisplayClass &ambientContrast(Flt contrast);
    Flt ambientContrast() C { return _amb_contrast; } // set/get Ambient Contrast           (0..Inf               , default=        2.0), the change is instant, you can call it real-time
    DisplayClass &ambientMin(Flt min);
    Flt ambientMin() C { return _amb_min; } // set/get Ambient Minimum            (0..1                 , default=        0.2), controls the limit that AO can darken, the change is instant, you can call it real-time
    DisplayClass &ambientRange(Flt range);
    Flt ambientRange() C { return _amb_range; } // set/get Ambient Range              (0..Inf               , default=        0.4), the change is instant, you can call it real-time

    // Night Shade
    DisplayClass &nightShadeColorL(C Vec &lin_color);
    C Vec &nightShadeColorL() C { return _ns_color_l; } // set/get Night Shade color Linear Gamma (0..1, default=0), the change is instant, you can call it real-time, setting color to 0 disables Night Shade effect
    DisplayClass &nightShadeColorS(C Vec &srgb_color);
    Vec nightShadeColorS() C; // set/get Night Shade color sRGB   Gamma (0..1, default=0), the change is instant, you can call it real-time, setting color to 0 disables Night Shade effect

    // Environment
    DisplayClass &envColor(C Vec &color);
    C Vec &envColor() C { return _env_color; } // set/get Environment color  (0..1, default=1), the change is         instant, you can call it real-time
    DisplayClass &envMap(C ImagePtr &cube);
    C ImagePtr &envMap() C { return _env_map; } // set/get Environment map                     , the change may not be instant, avoid   calling real-time, default=ImagePtr().get("Img/Environment.img"), images passed to this method must be created with IC_ENV_CUBE flag enabled in the 'Image.copy*' functions or have "Environment" mode selected in the "Engine Editor \ Image Editor"
    DisplayClass &envMatrix(C Matrix3 &matrix);
    C Matrix3 &envMatrix() C { return _env_matrix; } // set/get Environment matrix                  , the change is         instant, you can call it real-time
#if EE_PRIVATE
    void envMatrixSet() C;
#endif

    // Shadowing
    DisplayClass &shadowMode(SHADOW_MODE mode);
    SHADOW_MODE shadowMode() C { return _shd_mode; } // set/get Shadow Mode                                  (SHADOW_MODE         , default=SHADOW_MAP (SHADOW_NONE for Mobile)), the change is instant, you can call it real-time
    DisplayClass &shadowSoft(Byte soft);
    Byte shadowSoft() C { return _shd_soft; } // set/get Shadow Softing                               (0..SHADOW_SOFT_NUM-1, default=         1                         ), available only in RT_DEFERRED renderer, the change is instant, you can call it real-time
    DisplayClass &shadowJitter(Bool jitter);
    Bool shadowJitter() C { return _shd_jitter; } // set/get Shadow Jittering                             (true/false          , default=      true                         ), works best when combined with shadow softing, the change is instant, you can call it real-time
    DisplayClass &shadowReduceFlicker(Bool reduce);
    Bool shadowReduceFlicker() C { return _shd_reduce; } // set/get Shadow Flickering Decreasing                 (true/false          , default=     false                         ), this option reduces directional light shadow map flickering when rotating the camera, however at the expense of slightly increasing the shadow map blockiness, enable only when the flickering is really disturbing, the change is instant, you can call it real-time
    DisplayClass &shadowFrac(Flt frac);
    Flt shadowFrac() C { return _shd_frac; } // set/get Shadow Range Fraction for Directional Lights (  0..1              , default=         1                         ), this option can limit shadowing range to a fraction of the viewport range, the change is instant, you can call it real-time
    DisplayClass &shadowFade(Flt fade);
    Flt shadowFade() C { return _shd_fade; } // set/get Shadow Fade  Fraction for Directional Lights (  0..1              , default=         1                         ), this option specifies at which part of the shadowing range, shadow fading occurs, the change is instant, you can call it real-time
    DisplayClass &shadowMapSize(Int map_size);
    Int shadowMapSize() C { return _shd_map_size; } // set/get Shadow Map Size                              (  1..Inf            , default=      1024                         ), this option specifies the size of a single shadow map (in pixels), bigger size results in more precise shadows but smaller performance, the change is NOT instant, avoid calling real-time
    DisplayClass &shadowMapSizeLocal(Flt frac);
    Flt shadowMapSizeLocal() C { return _shd_map_size_l; } // set/get Shadow Map Size Fraction for    Local Lights (  0..1              , default=         1                         ), this option specifies global size factor for computing the size of shadow maps for local lights (point/cone), final size of shadow maps for local lights is calculated using following formula: "shadowMapSize()*shadowMapSizeLocal()"                    , this means that for shadowMapSizeLocal()==1 full shadowMapSize() is used, for shadowMapSizeLocal()==0.5 half of shadowMapSize() is used, and so on, the change is instant, you can call it real-time
    DisplayClass &shadowMapSizeCone(Flt factor);
    Flt shadowMapSizeCone() C { return _shd_map_size_c; } // set/get Shadow Map Size Factor   for    Cone  Lights (  0..2              , default=         1                         ), this option specifies global size factor for computing the size of shadow maps for cone  lights             , final size of shadow maps for cone  lights is calculated using following formula: "shadowMapSize()*shadowMapSizeLocal()*shadowMapSizeCone()", this means that for shadowMapSizeCone ()==1 full shadowMapSize() is used, for shadowMapSizeCone ()==0.5 half of shadowMapSize() is used, for shadowMapSizeCone()==2 double of shadowMapSize() is used, the range for shadowMapSizeCone is 0..2, which means that if set to 2, it can double the default shadow quality for all cone lights, the change is instant, you can call it real-time
    DisplayClass &shadowMapNum(Byte map_num);
    Byte shadowMapNum() C { return _shd_map_num; } // set/get Number of Shadow Maps for Directional Lights (  1..6              , default=         6                         ), this option specifies the number of shadow maps used when rendering directional lights, the more shadow maps are used the better quality but smaller performance, RT_FORWARD renderer supports only even numbers of maps, the change is instant, you can call it real-time
    DisplayClass &shadowMapSplit(C Vec2 &factor);
    C Vec2 &shadowMapSplit() C { return _shd_map_split; } // set/get Split Factor used for calculating Map Ranges (                    , default=    Vec2(2, 1)                     ), this option affects calculation of shadow map split ranges from the formula "Pow(x, factor.x + factor.y*x)". Ideal value is Vec2(2, 0) - "Pow(x, 2 + 0*x)" which gives equal distribution among near/far splits. However games may prefer better quality closer to the camera, while sacrificing the far quality, in that case it is beneficial to pass bigger values than Vec2(2, 0).
    DisplayClass &cloudsMapSize(Int map_size);
    Int cloudsMapSize() C { return _cld_map_size; } // set/get Clouds Shadow Map Size                       (  1..Inf            , default=       128                         ), the change is NOT instant, avoid calling real-time
#if EE_PRIVATE
    void shadowJitterSet();
    void shadowRangeSet();
    Int shadowMapNumActual() C;
    Int shadowMapSizeActual() C { return _shd_map_size_actual; }
    Bool shadowSupported() C;
#endif

    // Motion Blur
    DisplayClass &motionMode(MOTION_MODE mode);
    MOTION_MODE motionMode() C { return _mtn_mode; } // set/get Motion Blur Mode           (MOTION_MODE, default=MOTION_NONE), the change is instant, you can call it real-time
    DisplayClass &motionScale(Flt scale);
    Flt motionScale() C { return _mtn_scale; } // set/get Motion Blur Velocity Scale (  0..1     , default=        1.0), the change is instant, you can call it real-time
    DisplayClass &motionRes(Flt scale);
    Flt motionRes() C; // set/get Motion Blur Resolution     (  0..1     , default=       1/16), this determines the size of the buffers used for calculating the Motion Blur effect, 1=full size, 0.5=half size, 0.25=quarter size, .., smaller sizes offer faster performance but worse quality, the change is NOT instant, avoid calling real-time
    DisplayClass &motionJitter(Bool on);
    Bool motionJitter() C { return _mtn_jitter; } // set/get Motion Blur Jitter         (true/false , default=       true), the change is instant, you can call it real-time

    // Depth of Field
#if EE_PRIVATE
#define EPS_DOF (1.0f / 1024)
    Bool dofWant() C { return dofMode() != DOF_NONE && dofIntensity() > EPS_DOF; }
#endif
    DisplayClass &dofMode(DOF_MODE mode);
    DOF_MODE dofMode() C { return _dof_mode; } // set/get Depth of Field Mode                 ( DOF_MODE , default=DOF_NONE), the change is instant, you can call it real-time
    DisplayClass &dofFocusMode(Bool realistic);
    Bool dofFocusMode() C { return _dof_foc_mode; } // set/get Depth of Field Focus Mode           (true/false, default=   false), this affects how the focus is calculated based on depth, simple mode (realistic=false) operates on 'dofFocus' and 'dofRange' only, while realistic mode (realistic=true) operates on 'dofFocus' only
    DisplayClass &dofFocus(Flt z);
    Flt dofFocus() C { return _dof_focus; } // set/get Depth of Field Focus Depth Position (   0..Inf , default=       0), this is the distance from camera which should be completely focused and not blurred, the change is instant, you can call it real-time
    DisplayClass &dofRange(Flt range);
    Flt dofRange() C { return _dof_range; } // set/get Depth of Field Focus Depth Range    (   0..Inf , default=      30), this is the distance from 'dofFocus' (Focus Depth Position) where blurring should be set to maximum, used only in simple focus mode "dofFocusMode==false" (the change is instant, you can call it real-time
    DisplayClass &dofIntensity(Flt intensity);
    Flt dofIntensity() C { return _dof_intensity; } // set/get Depth of Field Intensity            (   0..Inf , default=       1), intensity of the depth of field effect, the change is instant, you can call it real-time

    // Eye Adaptation
    DisplayClass &eyeAdaptation(Bool on);
    Bool eyeAdaptation() C { return _eye_adapt; } // set/get Eye Adaptation usage                (                true/false                  , default=false            ), enables automatic screen brightness adjustment, the change is instant, you can call it real-time
    DisplayClass &eyeAdaptationBrightness(Flt brightness);
    Flt eyeAdaptationBrightness() C { return _eye_adapt_brightness; } // set/get Eye Adaptation brightness           (                   0..Inf                   , default= 0.1             ), total scale of lighting, the change is instant, you can call it real-time
    DisplayClass &eyeAdaptationExp(Flt exp);
    Flt eyeAdaptationExp() C { return _eye_adapt_exp; } // set/get Eye Adaptation exponent             (                   0..1                     , default= 0.333           ), affects how much brightness can change towards desired 'eyeAdaptationBrightness', value of 1 means that all targets can be reached, value of 0.5 means that the actual brightness scale will be a square root of full brightness scale, brightness scale is calculated based on following formula: "actual_scale = Pow(desired_scale, exp)", where 'desired_scale' is the scale needed to exactly reach target brightness, making exponent smaller results in making the 'actual_scale' closer to 1.0, which means smaller brightness scales are performed, the change is instant, you can call it real-time
    DisplayClass &eyeAdaptationIntensity(Flt intensity);
    Flt eyeAdaptationIntensity() C { return _eye_adapt_intensity; } // set/get Eye Adaptation intensity            (                   0..1                     , default= 1.0             ), the change is instant, you can call it real-time
    DisplayClass &eyeAdaptationMaxDark(Flt max_dark);
    Flt eyeAdaptationMaxDark() C { return _eye_adapt_max_dark; } // set/get Eye Adaptation maximum   darkening  (                   0..eyeAdaptationMaxBright, default= 0.5             ), this is a hard limit for maximum   darkening, the change is instant, you can call it real-time
    DisplayClass &eyeAdaptationMaxBright(Flt max_bright);
    Flt eyeAdaptationMaxBright() C { return _eye_adapt_max_bright; } // set/get Eye Adaptation maximum brightening  (eyeAdaptationMaxDark..Inf                   , default= 2.0             ), this is a hard limit for maximum brightening, the change is instant, you can call it real-time
    DisplayClass &eyeAdaptationSpeed(Flt speed);
    Flt eyeAdaptationSpeed() C { return _eye_adapt_speed; } // set/get Eye Adaptation speed                (                   1..Inf                   , default= 6.5             ), the change is instant, you can call it real-time
    DisplayClass &eyeAdaptationWeight(C Vec &weight);
    C Vec &eyeAdaptationWeight() C { return _eye_adapt_weight; } // set/get Eye Adaptation color weight         (           (0, 0, 0)..(1, 1, 1)             , default=(0.509, 1, 0.194)), the change is instant, you can call it real-time
    DisplayClass &resetEyeAdaptation(Flt brightness = 1);        // reset   Eye Adaptation value, eye adaptation changes over time according to screen colors, this method resets the adaptation to its original state, 'brightness'=initial brightness (0..Inf), the change is NOT instant, avoid calling real-time

    // Tone Mapping
    DisplayClass &toneMap(Bool on);
    Bool toneMap() C { return _tone_map; } // set/get Tone Mapping (true/false, default=false), the change is instant, you can call it real-time
#if EE_PRIVATE
    void setToneMap();
    void toneMapMonitorMaxLumAuto(); // automatically set
    DisplayClass &toneMapMonitorMaxLum(Flt max_lum);
    Flt toneMapMonitorMaxLum() C { return _tone_map_max_lum; } // set/get Tone Mapping Monitor Max Luminance (0..Inf, default=1), the change is instant, you can call it real-time, this is the maximum luminance supported by your monitor, LDR monitors have 1, while HDR monitors can be higher
#endif

    // Level of Detail
#if EE_PRIVATE
    void lodSetCurrentFactor();
    void lodUpdateFactors();
#endif
    DisplayClass &lod(Flt general, Flt mirror); // set     Level of Detail factors, the change is instant, you can call it real-time
    DisplayClass &lodFactor(Flt factor);
    Flt lodFactor() C { return _lod_factor; } // set/get Level of Detail general factor which determines                                    Lod selection, 0..Inf, default=1, it's a scaling factor: values from 0..1 increase details and values from 1..Inf decrease details, the change is instant, you can call it real-time
    DisplayClass &lodFactorMirror(Flt factor);
    Flt lodFactorMirror() C { return _lod_factor_mirror; } // set/get Level of Detail mirror  factor which is also applied to meshes rendered in mirrors Lod selection, 0..Inf, defailt=2, it's a scaling factor: values from 0..1 increase details and values from 1..Inf decrease details, the change is instant, you can call it real-time

    // Tesselation
    DisplayClass &tesselation(Bool on);
    Bool tesselation() C { return _tesselation; } // set/get Tesselation               (true/false, default=false), Tesselation smoothes the mesh polygons, it is available only on Shader Model>=5.0, the change is NOT instant, avoid calling real-time
    DisplayClass &tesselationHeightmap(Bool on);
    Bool tesselationHeightmap() C { return _tesselation_heightmap; } // set/get Tesselation of Heightmaps (true/false, default=false), the change is NOT instant, avoid calling real-time
    DisplayClass &tesselationDensity(Flt density);
    Flt tesselationDensity() C { return _tesselation_density; } // set/get Tesselation Density       (  0..Inf  , default=60   ), the change is instant, you can call it real-time
#if EE_PRIVATE
    DisplayClass &tesselationAllow(Bool on);
    Bool tesselationAllow() C { return _tesselation_allow; } // set/get Tesselation Allow         (true/false, default=true ), the change is instant, you can call it real-time
#endif

    // Grass
    DisplayClass &grassRange(Flt range);
    Flt grassRange() C { return _grass_range; } // set/get Grass Range             (  0..Inf  , default=50                ), grass objects above specified range will not be rendered, the change is instant, you can call it real-time
    DisplayClass &grassDensity(Flt density);
    Flt grassDensity() C { return _grass_density; } // set/get Grass Density           (  0..1    , default=1 (0.5 for Mobile)), controls the amount of grass objects rendered, the change is instant, you can call it real-time
    DisplayClass &grassShadow(Bool on);
    Bool grassShadow() C { return _grass_shadow; } // set/get Grass Shadow Casting    (true/false, default=false             ), if grass objects should cast shadows, the change is instant, you can call it real-time
    DisplayClass &grassMirror(Bool on);
    Bool grassMirror() C { return _grass_mirror; } // set/get Grass Drawing in Mirror (true/false, default=false             ), if grass objects should be drawn in mirrors, the change is instant, you can call it real-time
    DisplayClass &grassUpdate();                   // update  Grass and Leafs bending animation, call this each frame to update the animation, the change is instant, you can call it real-time

    // Fur
    DisplayClass &furStaticGravity(Flt gravity);
    Flt furStaticGravity() C { return _fur_gravity; } //                     gravity affecting fur on non-animated meshes which aren't controlled by AnimatedSkeleton (AnimatedSkeleton.fur_* parameters), -Inf..Inf, default=-1   , the change is instant, you can call it real-time
    DisplayClass &furStaticVelScale(Flt vel_scale);
    Flt furStaticVelScale() C { return _fur_vel_scale; } // how much does mesh movement affect    fur on non-animated meshes which aren't controlled by AnimatedSkeleton (AnimatedSkeleton.fur_* parameters), -Inf..Inf, default=-0.75, the change is instant, you can call it real-time

    // Volumetric Lighting
    DisplayClass &volLight(Bool on);
    Bool volLight() C { return _vol_light; } // set/get Volumetric Lighting       (true/false, default=false), the change is instant, you can call it real-time
    DisplayClass &volAdd(Bool on);
    Bool volAdd() C { return _vol_add; } // set/get Volumetric Apply Mode     (true/false, default=false), when true is set, Volumetric Light will be added to the scene, if false is set, Lerp to 1 will be performed, the change is instant, you can call it real-time
    DisplayClass &volMax(Flt max);
    Flt volMax() C { return _vol_max; } // set/get Volumetric Global Maximum (   0..Inf , default=1.0  ), the change is instant, you can call it real-time

    // Viewport Settings
#if EE_PRIVATE
    void setViewFovTan();
    void viewUpdate();
    void viewReset();
    DisplayClass &view(C RectI &rect, Flt from, Flt range, C Vec2 &fov, FOV_MODE fov_mode);
    DisplayClass &view(C Rect &rect, Flt from, Flt range, C Vec2 &fov, FOV_MODE fov_mode);
    Flt viewFromActual() C { return _view_from; }             // get actual viewport near clip plane (this depends on FovMode)
    C Vec2 &viewCenter() C { return _view_center; }           // get viewport center
    C Vec2 &viewFovTanGui() C { return _view_fov_tan_gui; }   // get viewport fov tan, used for point transformation between 3D and VR 'GuiTexture'
    C Vec2 &viewFovTanFull() C { return _view_fov_tan_full; } // get viewport fov tan, used for point transformation between 3D and Full Screen
#endif
    DisplayClass &viewRect(C RectI *rect);
    C RectI &viewRectI() C { return _view_main.recti; } // set/get viewport rectangle (null for fullscreen), custom viewport rectangle is reset at start of each frame to fullscreen, that's why when used it must be set manually in each frame
    DisplayClass &viewRect(C Rect &rect);
    C Rect &viewRect() C { return _view_rect; } // set/get viewport rectangle                      , custom viewport rectangle is reset at start of each frame to fullscreen, that's why when used it must be set manually in each frame
    DisplayClass &viewFrom(Flt from);
    Flt viewFrom() C { return _view_main.from; } // set/get viewport near clip plane       , default=0.05
    DisplayClass &viewRange(Flt range);
    Flt viewRange() C { return _view_main.range; } // set/get viewport far  clip plane       , default=100
    DisplayClass &viewFov(Flt fov, FOV_MODE mode = FOV_Y);
    Flt viewFov() C { return _view_fov; } // set/get viewport          field of view, default={DegToRad(70), FOV_Y}
    DisplayClass &viewFov(C Vec2 &fov);
    C Vec2 &viewFovXY() C { return _view_main.fov; }         // set/get viewport          field of view from both horizontal and vertical values
    Flt viewFovX() C { return _view_main.fov.x; }            //     get actual horizontal field of view
    Flt viewFovY() C { return _view_main.fov.y; }            //     get actual vertical   field of view
    FOV_MODE viewFovMode() C { return _view_main.fov_mode; } //     get viewport          field of view mode, default=FOV_Y
    Flt viewQuadDist() C;                                    //     get viewport quad max distance from camera
    DisplayClass &viewForceSquarePixel(Bool square = false); //   force pixel aspect ratio to 1, this may be needed for rendering to textures, default rendering should have this disabled, default=false

    // Convert Coordinates                                                          src.X           src.Y                          dest.X          dest.Y
    static Vec2 screenToUV(C Vec2 &screen);               // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to UV          (   0 .. 1     ,    0 .. 1     )
    static Rect screenToUV(C Rect &screen);               // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to UV          (   0 .. 1     ,    0 .. 1     )
    static Vec2 screenToPixel(C Vec2 &screen);            // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to pixel       (   0 .. D.resW,    0 .. D.resH)
    static VecI2 screenToPixelI(C Vec2 &screen);          // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to pixel       (   0 .. D.resW,    0 .. D.resH)
    static Rect screenToPixel(C Rect &screen);            // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to pixel       (   0 .. D.resW,    0 .. D.resH)
    static RectI screenToPixelI(C Rect &screen);          // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to pixel       (   0 .. D.resW,    0 .. D.resH)
    static Vec2 screenToPixelSize(C Vec2 &screen);        // from screen size (   0 .. D.w*2 ,    0 .. D.h*2 ) to pixel  size (   0 .. D.resW,    0 .. D.resH), use this function for widths and heights
    static Vec2 UVToScreen(C Vec2 &uv);                   // from UV          (   0 .. 1     ,    0 .. 1     ) to screen      (-D.w .. D.w   , -D.h .. D.h   )
    static Vec2 pixelToScreen(C Vec2 &pixel);             // from pixel       (   0 .. D.resW,    0 .. D.resH) to screen      (-D.w .. D.w   , -D.h .. D.h   )
    static Vec2 pixelToScreen(C VecI2 &pixel);            // from pixel       (   0 .. D.resW,    0 .. D.resH) to screen      (-D.w .. D.w   , -D.h .. D.h   )
    static Rect pixelToScreen(C Rect &pixel);             // from pixel       (   0 .. D.resW,    0 .. D.resH) to screen      (-D.w .. D.w   , -D.h .. D.h   )
    static Rect pixelToScreen(C RectI &pixel);            // from pixel       (   0 .. D.resW,    0 .. D.resH) to screen      (-D.w .. D.w   , -D.h .. D.h   )
    static Vec2 pixelToScreenSize(Flt pixel);             // from pixel  size (   0 .. D.resW,    0 .. D.resH) to screen size (   0 .. D.w*2 ,    0 .. D.h*2 ), use this function for widths and heights
    static Vec2 pixelToScreenSize(C Vec2 &pixel);         // from pixel  size (   0 .. D.resW,    0 .. D.resH) to screen size (   0 .. D.w*2 ,    0 .. D.h*2 ), use this function for widths and heights
    static Vec2 pixelToScreenSize(C VecI2 &pixel);        // from pixel  size (   0 .. D.resW,    0 .. D.resH) to screen size (   0 .. D.w*2 ,    0 .. D.h*2 ), use this function for widths and heights
    static Vec2 screenAlignedToPixel(C Vec2 &screen);     // get   screen space position  aligned to nearest pixel
    static Vec2 alignScreenToPixelOffset(C Vec2 &screen); // get   offset needed for aligning screen space position to nearest pixel
    static void alignScreenToPixel(Vec2 &screen);         // align screen space position  to nearest pixel
    static void alignScreenToPixel(Rect &screen);         // align screen space rectangle so its top-left corner will be aligned to nearest pixel while preserving the rectangle size
    C Vec2 &pixelToScreenSize() C { return _pixel_size; } // get screen size of a single pixel
#if EE_PRIVATE
    static void alignScreenXToPixel(Flt &screen_x);      // align screen space x position to nearest pixel
    static void alignScreenYToPixel(Flt &screen_y);      // align screen space y position to nearest pixel
    static Vec2 windowPixelToScreen(C Vec2 &pixel);      // from pixel       (   0 .. D.resW,    0 .. D.resH) to screen      (-D.w .. D.w   , -D.h .. D.h   ) taking into account System Window -> VR Gui
    static Vec2 windowPixelToScreen(C VecI2 &pixel);     // from pixel       (   0 .. D.resW,    0 .. D.resH) to screen      (-D.w .. D.w   , -D.h .. D.h   ) taking into account System Window -> VR Gui
    static Vec2 windowPixelToScreenSize(C Vec2 &pixel);  // from pixel  size (   0 .. D.resW,    0 .. D.resH) to screen size (   0 .. D.w*2 ,    0 .. D.h*2 ) taking into account System Window -> VR Gui, use this function for widths and heights
    static Vec2 windowPixelToScreenSize(C VecI2 &pixel); // from pixel  size (   0 .. D.resW,    0 .. D.resH) to screen size (   0 .. D.w*2 ,    0 .. D.h*2 ) taking into account System Window -> VR Gui, use this function for widths and heights
    static Vec2 screenToWindowPixel(C Vec2 &screen);     // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to pixel       (   0 .. D.resW,    0 .. D.resH) taking into account System Window -> VR Gui
    static Vec2 screenToWindowPixelSize(C Vec2 &screen); // from screen size (   0 .. D.w*2 ,    0 .. D.h*2 ) to pixel  size (   0 .. D.resW,    0 .. D.resH) taking into account System Window -> VR Gui, use this function for widths and heights
    static VecI2 screenToWindowPixelI(C Vec2 &screen);   // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to pixel       (   0 .. D.resW,    0 .. D.resH) taking into account System Window -> VR Gui
    static RectI screenToWindowPixelI(C Rect &screen);   // from screen      (-D.w .. D.w   , -D.h .. D.h   ) to pixel       (   0 .. D.resW,    0 .. D.resH) taking into account System Window -> VR Gui
#endif

    // Clear Screen
    static void clear(C Color &srgb_color = TRANSPARENT); // clear screen viewport to 'srgb_color' and clear depth buffer
    static void clearCol(C Color &srgb_color);            // clear screen viewport to 'srgb_color'
    static void clearCol(C Vec4 &color = Vec4Zero);       // clear screen viewport to      'color'
    static void clearDepth();                             // clear                                           depth buffer
#if EE_PRIVATE
    static void clearDS(Byte s = 0);            // clear depth buffer and stencil (if available)
    static void clearStencil(Byte s = 0);       // clear                  stencil
    static void clearCol(Int i, C Vec4 &color); // clear i-th full (not viewport) RT to 'color'

    Bool mergedClear() C { return _view_main.full || TILE_BASED_GPU; } // use only when having full viewport ("clearCol(Int i" ignores viewport), or when having a tile-based GPU to avoid overhead of RT transfers. Don't enable in other cases, because on Intel GPU Windows it made things much slower, on GeForce 1050 Ti it made no difference.
#endif

    // operations
    void setShader(C Material *material = null); // manually trigger resetting shaders for all meshes, 'material'=if reset shaders only for meshes having specified material (use null for all meshes)

    static void flush(); // flush queued commands to the GPU

#if !EE_PRIVATE
  private:
#endif
    struct ViewportSettings : ViewSettings {
        Rect rect;

        void get();   // get viewport settings from current display settings "T=D"
        void set() C; // set viewport settings as   current display settings "D=T"
    };
    struct Viewport {
        // 2D
        RectI recti;
        Bool full;

        // 3D
        FOV_MODE fov_mode;
        Flt from, range;
        Vec2 fov, fov_sin, fov_cos, fov_tan;

#if EE_PRIVATE
        Viewport &set3DFrom(C Viewport &src);
        Viewport &setRect(C RectI &recti);
        Viewport &setFrom(Flt from);
        Viewport &setRange(Flt range);
        Viewport &setFov(); // this needs to be called after 'setRect'
        Viewport &setFov(C Vec2 &fov, FOV_MODE fov_mode);
        Viewport &set(C RectI &recti, Flt from, Flt range, C Vec2 &fov, FOV_MODE fov_mode);
        Viewport &setViewport();
        Viewport &setShader(Flt *offset = null);
        Viewport &setProjMatrix(); // 'Frustum' may have to be reset after calling this method
#endif
    };
    struct Monitor {
        RectI full, work;
        Bool primary;
        Mems<VecI2> modes;
        Str name;
#if WINDOWS_OLD
        Char device_name[32], device_key[128];
#endif

#if EE_PRIVATE
        Bool is() C { return full.w() > 0; } // if initialized
        VecI2 mode() C;                      // get current mode
        Str colorProfilePath() C;
        Str standardColorProfilePath() C;
#if WINDOWS_OLD
        Bool set(HMONITOR monitor);
#endif
        Monitor();
#endif
    };

    ASPECT_MODE _aspect_mode;
    DIFFUSE_MODE _diffuse_mode;
    BUMP_MODE _bump_mode;
    AMBIENT_MODE _amb_mode;
    SHADOW_MODE _shd_mode;
    MOTION_MODE _mtn_mode;
    DOF_MODE _dof_mode;
    EDGE_DETECT_MODE _edge_detect, _outline_mode;
    EDGE_SOFTEN_MODE _edge_soften;
    TEXTURE_USAGE _tex_detail;
    SHADER_MODEL _shader_model;
    IMAGE_PRECISION _output_prec, _lit_col_rt_prec, _color_prec;
    FILTER_TYPE _density_filter;
    COLOR_SPACE _color_space;
    Bool _full, _sync, _autosavescript, _exclusive, _hdr,
        _hp_col_rt, _hp_nrm_rt, _hp_lum_rt, _hp_pal_rt,
        _particles_soft, _particles_smooth,
        _tex_mip_filter, _tex_macro, _tex_detail_lod,
        _bloom_allow,
        _tesselation, _tesselation_heightmap, _tesselation_allow,
        _ao_all, _amb_jitter, _amb_normal,
        _mtn_jitter,
        _shd_jitter, _shd_reduce,
        _grass_shadow, _grass_mirror,
        _vol_light, _vol_add,
        _temp_anti_alias, _temp_super_res,
        _glow_allow, _dither, _bend_leafs, _eye_adapt, _dof_foc_mode, _color_palette_allow, _gamma_all, _fade_get, _fade_auto_draw, _mtrl_blend, _draw_null_mtrl, _view_square_pixel, _allow_stereo, _max_lights_soft, _sharpen,
        _tone_map, _tone_map_allow, _tone_map_use,
        _initialized, _resetting, _no_gpu;
    SByte _half_supported;
    Byte _density, _samples, _max_tex_filter, _max_rt,
        _amb_soft, _amb_res,
        _shd_soft, _shd_map_num,
        _mtn_res,
        _max_lights,
        _tex_filter, _tex_mip_min;
    VecB2 _gl_ver;
    UShort _gamma_array[3][256];
    Int _shd_map_size, _shd_map_size_actual, _cld_map_size, _freq_want, _freq_got, _max_tex_size;
    Long _device_mem;
    VecI2 _res, _render_res;
    Flt _app_aspect_ratio, _disp_aspect_ratio, _disp_aspect_ratio_want, _pixel_aspect, _gamma, _tex_mip_bias, _image_mip_bias, _font_mip_bias, _scale,
        _amb_range, _amb_contrast, _amb_min,
        _eye_adapt_brightness, _eye_adapt_exp, _eye_adapt_intensity, _eye_adapt_max_dark, _eye_adapt_max_bright, _eye_adapt_speed,
        _eye_dist, _eye_dist_2,
        _shd_frac, _shd_fade, _shd_range, _shd_map_size_l, _shd_map_size_c, _shd_bias,
        _bloom_original, _bloom_mul, _bloom_add, _bloom_glow, _bloom_cut,
        _contrast,
        _mtn_scale,
        _dof_focus, _dof_range, _dof_intensity,
        _vol_max,
        _grass_range, _grass_range_sqr, _grass_density,
        _lod_factor, _lod_factor_mirror, _lod_factors[2], _lod_factors_fov[2], _lod_fov2, _lod_current_factor,
        _tesselation_density,
        _fur_gravity, _fur_vel_scale,
        _view_fov, _view_from,
        _fade_alpha, _fade_speed,
        _sharpen_intensity,
        _smaa_threshold,
        _white_lum, _screen_max_lum, _screen_nits,
        _tone_map_max_lum;
    Vec2 _unscaled_size, _size2, _pixel_size, _pixel_size_2, _pixel_size_inv,
        _window_pixel_to_screen_mul, _window_pixel_to_screen_add, _window_pixel_to_screen_scale,
        _shd_map_split;
    Vec _amb_color_l, _ns_color_l, _env_color, _eye_adapt_weight;
    Vec2 _view_center, _view_fov_tan_gui, _view_fov_tan_full;
    Rect _rect, _rect_ui, _view_rect, _view_eye_rect[2];
    Matrix3 _env_matrix;
    Viewport _view_main, _view_active;
    Str8 _device_name;
    ImagePtr _color_palette[2], _env_map;
    Image _color_palette_soft[2], _color_lut;
    Mems<VecI2> _modes;
    SyncLock _lock;
    C Material *_set_shader_material;
    Map<Ptr, Monitor> _monitors;
    ImageRTPtr _flip;

#if EE_PRIVATE
    // manage
    void init();
    void del();
    Bool create();
    void createDevice();
    void androidClose();
    void androidOpen();

    // operations
    enum RESET_RESULT {
        RESET_OK,
        RESET_ERROR_NOT_CREATED,
        RESET_ERROR_SET_DISPLAY_MODE,
        RESET_ERROR_SET_FULLSCREEN_STATE,
        RESET_ERROR_RESIZE_TARGET,
        RESET_ERROR_RESIZE_BUFFERS,
        RESET_ERROR_RENDER_TARGET_CREATE,
    };
    static CChar8 *AsText(RESET_RESULT result);
    static void ResetFailed(RESET_RESULT New, RESET_RESULT old);
    RESET_RESULT ResetTry(Bool set = false);
    void Reset();
    RESET_RESULT modeTry(Int w = -1, Int h = -1, Int full = -1, Bool auto_full = true, Bool set = false); // try setting Display Mode, specifying -1 for any parameter means keep current value, 'auto_full'=if allow forcing fullscreen when mode covers entire screen
    void modeSet(Int w = -1, Int h = -1, Int full = -1);                                                  //     set     Display Mode, specifying -1 for any parameter means keep current value

    Bool findMode(Bool auto_full = true); // 'auto_full'=if allow forcing fullscreen when mode covers entire screen
    void getGamma();
    void getCaps();
    void after(Bool resize_callback);
    Bool initialized() C { return _initialized; }
    static void flip();
    static void finish();
    void screenChanged(Flt old_width, Flt old_height);
    void sizeChanged();
    void setRectUI();
    void validateCoords(Int eye = -1);
    void fadeUpdate();
    void fadeDraw();

#if GL
    static VecI2 glVer();
    static Ptr glGetProcAddress(CChar8 *name);
#endif
    Bool gatherAvailable() C;
    Bool gatherChannelAvailable() C;
    Bool conservativeDepthAvailable() C;
    Bool packHalf2x16Available() C;
    Bool computeAvailable() C;
    Bool filterMinMaxAvailable() C;
    Bool independentBlendAvailable() C;
    Bool deferredUnavailable() C;
    Bool deferredMSUnavailable() C;
    Bool SpirVAvailable() C;
    Bool canSwapSRGB() C;

#if WINDOWS_OLD
    Monitor *getMonitor(HMONITOR hmonitor);
#endif
#if DX11
    Monitor *getMonitor(IDXGIOutput &output);
#endif
    C Monitor *mainMonitor() C;
    C Monitor *curMonitor(); // current monitor (null if none)
    C Monitor *getMonitor(); // current monitor (main if none)

    void getMonitor(RectI &full, RectI &work, VecI2 &max_normal_win_client_size, VecI2 &maximized_win_client_size);
#endif

    DisplayClass();
} extern D;                       // Main Display Control
inline DisplayClass &Display = D; // 'Display' alias ('Display' can be used the same way as 'D')
/******************************************************************************/
#if EE_PRIVATE

#if DX11
extern ID3D11Device *D3D;
extern ID3D11DeviceContext *D3DC;
extern ID3D11DeviceContext1 *D3DC1;
#if WINDOWS_OLD
extern IDXGISwapChain *SwapChain;
extern DXGI_SWAP_CHAIN_DESC SwapChainDesc;
#else
extern IDXGISwapChain1 *SwapChain;
extern DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
#endif
extern D3D_FEATURE_LEVEL FeatureLevel;
#elif GL
struct GLContext {
    Bool locked;
#if WINDOWS
    HGLRC context;
#elif MAC
    CGLContextObj context;
#elif LINUX
    GLXContext context;
#elif ANDROID || SWITCH
    EGLSurface surface;
    EGLContext context;
#elif IOS
    EAGLContext *context;
#elif WEB
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;
#endif

    Bool is() C;
    GLContext();
    ~GLContext() { del(); }
    void del();
    Bool createSecondary();
    void lock();
    void unlock();
} extern MainContext;

Ptr GetCurrentContext();

extern UInt FBO, VAO;
#if IOS
extern UInt FBO1;
#elif LINUX
extern GLXFBConfig GLConfig;
#elif ANDROID || SWITCH
extern EGLConfig GLConfig;
extern EGLDisplay GLDisplay;
#endif

#if !WINDOWS && !SWITCH
extern void (*glTexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);                                                           // available on GL 4.2+, GL ES 3.0+
extern void (*glTextureView)(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers); // available on GL 4.3+, GL ES NO
#endif
#endif

Bool SetDisplayMode(Int mode = 1); // 0=always off (use at shutdown), 1=if want full and app is active, 2=if want full (use at init)
void RequestDisplayMode(Int w, Int h, Int full);

Vec2 ScreenToPixel(C Vec2 &screen, C VecI2 &res);
Rect ScreenToPixel(C Rect &screen, C VecI2 &res);
RectI ScreenToPixelI(C Rect &screen, C VecI2 &res);

Rect ImgClamp(C Rect &screen, C VecI2 &res);
Rect ImgClamp(C RectI &pixel, C VecI2 &res);
inline Rect ImgClamp(C VecI2 &res) { return ImgClamp(D.viewRect(), res); }

#if WINDOWS_NEW
extern Flt ScreenScale;

Flt DipsToPixels(Flt dips);
Int DipsToPixelsI(Flt dips);
Flt PixelsToDips(Int pix);
Flt PixelsToDips(Flt pix);
#elif MAC
extern NSOpenGLContext *OpenGLContext;
#elif LINUX
extern ::Display *XDisplay;
#elif IOS
extern Flt ScreenScale;
#endif

#endif
void SetFov(Vec2 &fov, FOV_MODE fov_mode, Flt aspect);
/******************************************************************************/
