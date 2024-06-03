/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Image' for handling images (textures).

/******************************************************************************/
enum FILTER_TYPE : Byte // Filtering Type
{
    FILTER_NONE,              //  1.0000 speed, worst  quality, uses 1x1 samples no 2D filtering. Also known as Point/Nearest filter
    FILTER_LINEAR,            // ~0.7500 speed, low    quality, uses 2x2 samples in 2D filtering
    FILTER_CUBIC_FAST,        // ~0.1700 speed, high   quality, uses 4x4 samples in 2D filtering, low    sharpening is applied
    FILTER_CUBIC_FAST_SMOOTH, // ~0.1700 speed, blurry quality, uses 4x4 samples in 2D filtering, no     sharpening is applied, result will appear blurry however without aliasing
    FILTER_CUBIC_FAST_MED,    // ~0.1700 speed, high   quality, uses 4x4 samples in 2D filtering, medium sharpening is applied (best for down-scaling)
    FILTER_CUBIC_FAST_SHARP,  // ~0.1700 speed, high   quality, uses 4x4 samples in 2D filtering, high   sharpening is applied (best for  mip-maps   )
    FILTER_CUBIC_PLUS,        // ~0.1000 speed, high   quality, uses 6x6 samples in 2D filtering, medium sharpening is applied (best for   up-scaling)
    FILTER_CUBIC_PLUS_SHARP,  // ~0.1000 speed, high   quality, uses 6x6 samples in 2D filtering, high   sharpening is applied
    FILTER_WAIFU,             // ~0.0005 speed, super  quality however super slow, this filter is available only if 'SupportFilterWaifu' was called inside 'InitPre', it's used only for up-scaling
    FILTER_EASU,              //                high   quality, AMD Edge Adaptive Spatial Upsampling, this filter is available only for up-scaling

    FILTER_BEST,       // automatically choose the best filter (currently FILTER_WAIFU for up-scaling, FILTER_CUBIC_FAST_MED for down-scaling, FILTER_CUBIC_FAST_SHARP for mip-maps)
    FILTER_NO_STRETCH, // does not perform any stretching, pixels out of range are either wrapped or clamped
    FILTER_NUM,        // number of filters

#if EE_PRIVATE
    FILTER_DOWN = FILTER_CUBIC_FAST_MED, // best filter for down-scaling
#endif
    FILTER_MIP = FILTER_CUBIC_FAST_SHARP, // best filter for mip-maps
};
/******************************************************************************/
enum LOCK_MODE : Byte {
    LOCK_NONE = 0,       // no lock
    LOCK_READ = 1,       // lock for reading only
    LOCK_WRITE = 2,      // lock for writing only (if the image is stored in the GPU then any previous image data may get lost when using this mode, use if you wish to replace the whole image data)
    LOCK_READ_WRITE = 3, // lock for reading and writing
#if EE_PRIVATE
    LOCK_APPEND = 4, // lock for writing only without overwriting existing data
#endif
};
/******************************************************************************/
enum IMAGE_TYPE : Byte // Image Type, comments specify in which mode the type is available (Soft: Software, DX10: DirectX 10, DX11: DirectX 11, GL: Desktop OpenGL, partial: may be supported on some devices but not all of them)
{
    IMAGE_NONE, // none

    IMAGE_R8G8B8A8,      // 32-bit (R,G,B,A), linear gamma,  0..1 unsigned, Soft, DX10+, GL, GL ES
    IMAGE_R8G8B8A8_SRGB, // 32-bit (R,G,B,A), sRGB   gamma,  0..1 unsigned, Soft, DX10+, GL, GL ES
    IMAGE_R8G8B8A8_SIGN, // 32-bit (R,G,B,A), linear gamma, -1..1   signed, Soft, DX10+, GL, GL ES
    IMAGE_R8G8B8,        // 24-bit (R,G,B,1), linear gamma,  0..1 unsigned, Soft
    IMAGE_R8G8B8_SRGB,   // 24-bit (R,G,B,1), sRGB   gamma,  0..1 unsigned, Soft
    IMAGE_R8G8,          // 16-bit (R,G,0,1), linear gamma,  0..1 unsigned, Soft, DX10+, GL, GL ES
    IMAGE_R8G8_SIGN,     // 16-bit (R,G,0,1), linear gamma, -1..1   signed, Soft, DX10+, GL, GL ES
    IMAGE_R8,            //  8-bit (R,0,0,1), linear gamma,  0..1 unsigned, Soft, DX10+, GL, GL ES
    IMAGE_R8_SIGN,       //  8-bit (R,0,0,1), linear gamma, -1..1   signed, Soft, DX10+, GL, GL ES

    IMAGE_R10G10B10A2, // 32-bit (R,G,B,A), linear gamma, 0..1 unsigned, Soft, DX10+, GL, GL ES

    IMAGE_A8,        //  8-bit           alpha (0,0,0,A),               Soft, DX10+, GL, GL ES
    IMAGE_L8,        //  8-bit luminance       (L,L,L,1), linear gamma, Soft,        GL, GL ES
    IMAGE_L8_SRGB,   //  8-bit luminance       (L,L,L,1), sRGB   gamma, Soft
    IMAGE_L8A8,      // 16-bit luminance alpha (L,L,L,A), linear gamma, Soft,        GL, GL ES
    IMAGE_L8A8_SRGB, // 16-bit luminance alpha (L,L,L,A), sRGB   gamma, Soft

    IMAGE_I8,         //      8-bit integer              , linear gamma, Soft
    IMAGE_I16,        //     16-bit integer              , linear gamma, Soft
    IMAGE_I24,        //     24-bit integer              , linear gamma, Soft
    IMAGE_I32,        //     32-bit integer              , linear gamma, Soft
    IMAGE_F16,        //     16-bit float                , linear gamma, Soft, DX10+, GL, GL ES
    IMAGE_F32,        //     32-bit float                , linear gamma, Soft, DX10+, GL, GL ES
    IMAGE_F16_2,      // 2 x 16-bit float ( 32-bit total), linear gamma, Soft, DX10+, GL, GL ES
    IMAGE_F32_2,      // 2 x 32-bit float ( 64-bit total), linear gamma, Soft, DX10+, GL, GL ES
    IMAGE_F16_3,      // 3 x 16-bit float ( 48-bit total), linear gamma, Soft         GL, GL ES
    IMAGE_F32_3,      // 3 x 32-bit float ( 96-bit total), linear gamma, Soft, DX10+, GL, GL ES
    IMAGE_F16_4,      // 4 x 16-bit float ( 64-bit total), linear gamma, Soft, DX10+, GL, GL ES
    IMAGE_F32_4,      // 4 x 32-bit float (128-bit total), linear gamma, Soft, DX10+, GL, GL ES
    IMAGE_F32_3_SRGB, // 3 x 32-bit float ( 96-bit total), sRGB   gamma, Soft
    IMAGE_F32_4_SRGB, // 4 x 32-bit float (128-bit total), sRGB   gamma, Soft

    // compressed format for Desktop
    IMAGE_BC1,      // BC1/DXT1 4-bit lossy RGBA compression with 1-bit  alpha            , linear gamma,  0..1        unsigned, Soft, DX10+, GL, partial Android
    IMAGE_BC1_SRGB, // BC1/DXT1 4-bit lossy RGBA compression with 1-bit  alpha            , sRGB   gamma,  0..1        unsigned, Soft, DX10+, GL, partial Android
    IMAGE_BC2,      // BC2/DXT3 8-bit lossy RGBA compression with sharp  alpha transitions, linear gamma,  0..1        unsigned, Soft, DX10+, GL, partial Android
    IMAGE_BC2_SRGB, // BC2/DXT3 8-bit lossy RGBA compression with sharp  alpha transitions, sRGB   gamma,  0..1        unsigned, Soft, DX10+, GL, partial Android
    IMAGE_BC3,      // BC3/DXT5 8-bit lossy RGBA compression with smooth alpha transitions, linear gamma,  0..1        unsigned, Soft, DX10+, GL, partial Android
    IMAGE_BC3_SRGB, // BC3/DXT5 8-bit lossy RGBA compression with smooth alpha transitions, sRGB   gamma,  0..1        unsigned, Soft, DX10+, GL, partial Android
    IMAGE_BC4,      // BC4      4-bit lossy R    compression                              , linear gamma,  0..1        unsigned, Soft, DX10+, GL, partial Android
    IMAGE_BC4_SIGN, // BC4      4-bit lossy R    compression                              , linear gamma, -1..1          signed, Soft, DX10+, GL, partial Android
    IMAGE_BC5,      // BC5      8-bit lossy RG   compression                              , linear gamma,  0..1        unsigned, Soft, DX10+, GL, partial Android
    IMAGE_BC5_SIGN, // BC5      8-bit lossy RG   compression                              , linear gamma, -1..1          signed, Soft, DX10+, GL, partial Android
    IMAGE_BC6,      // BC6      8-bit lossy RGB 16-bit floating point compression         , linear gamma,  0..HALF_MAX unsigned, Soft, DX11+, partial GL (compressing images to this format is available only when 'SupportCompressBC' was called in 'InitPre')
    IMAGE_BC7,      // BC7      8-bit lossy RGBA         high quality compression         , linear gamma,  0..1        unsigned, Soft, DX11+, partial GL (compressing images to this format is available only when 'SupportCompressBC' was called in 'InitPre')
    IMAGE_BC7_SRGB, // BC7      8-bit lossy RGBA         high quality compression         , sRGB   gamma,  0..1        unsigned, Soft, DX11+, partial GL (compressing images to this format is available only when 'SupportCompressBC' was called in 'InitPre')

    // compressed format for Android/iOS (compressing images to these formats is available only when 'SupportCompressETC' was called in 'InitPre')
    IMAGE_ETC2_R,          // Ericsson 4-bit lossy R    compression with no    alpha (R,0,0,1     ), linear gamma,  0..1 unsigned, Soft, partial GL, GL ES
    IMAGE_ETC2_R_SIGN,     // Ericsson 4-bit lossy R    compression with no    alpha (R,0,0,1     ), linear gamma, -1..1   signed, Soft, partial GL, GL ES
    IMAGE_ETC2_RG,         // Ericsson 8-bit lossy RG   compression with no    alpha (R,G,0,1     ), linear gamma,  0..1 unsigned, Soft, partial GL, GL ES
    IMAGE_ETC2_RG_SIGN,    // Ericsson 8-bit lossy RG   compression with no    alpha (R,G,0,1     ), linear gamma, -1..1   signed, Soft, partial GL, GL ES
    IMAGE_ETC2_RGB,        // Ericsson 4-bit lossy RGB  compression with no    alpha (R,G,B,1     ), linear gamma,  0..1 unsigned, Soft, partial GL, GL ES
    IMAGE_ETC2_RGB_SRGB,   // Ericsson 4-bit lossy RGB  compression with no    alpha (R,G,B,1     ), sRGB   gamma,  0..1 unsigned, Soft, partial GL, GL ES
    IMAGE_ETC2_RGBA1,      // Ericsson 4-bit lossy RGBA compression with 1-bit alpha (R,G,B,0 or 1), linear gamma,  0..1 unsigned, Soft, partial GL, GL ES
    IMAGE_ETC2_RGBA1_SRGB, // Ericsson 4-bit lossy RGBA compression with 1-bit alpha (R,G,B,0 or 1), sRGB   gamma,  0..1 unsigned, Soft, partial GL, GL ES
    IMAGE_ETC2_RGBA,       // Ericsson 8-bit lossy RGBA compression with 8-bit alpha (R,G,B,A     ), linear gamma,  0..1 unsigned, Soft, partial GL, GL ES
    IMAGE_ETC2_RGBA_SRGB,  // Ericsson 8-bit lossy RGBA compression with 8-bit alpha (R,G,B,A     ), sRGB   gamma,  0..1 unsigned, Soft, partial GL, GL ES

    // compressed formats for iOS (compressing images to these formats is available only on Desktop platforms when 'SupportCompressPVRTC' was called in 'InitPre', decompression and especially compression may be slow, formats are recommended to be used only on iOS)
    IMAGE_PVRTC1_2,      // PVRTC1 2-bit lossy RGBA compression, linear gamma, 0..1 unsigned, Soft, iOS, partial Android
    IMAGE_PVRTC1_2_SRGB, // PVRTC1 2-bit lossy RGBA compression, sRGB   gamma, 0..1 unsigned, Soft, iOS, partial Android
    IMAGE_PVRTC1_4,      // PVRTC1 4-bit lossy RGBA compression, linear gamma, 0..1 unsigned, Soft, iOS, partial Android
    IMAGE_PVRTC1_4_SRGB, // PVRTC1 4-bit lossy RGBA compression, sRGB   gamma, 0..1 unsigned, Soft, iOS, partial Android

    IMAGE_TYPES, // number of types
#if EE_PRIVATE
    IMAGE_B8G8R8A8, // 32-bit (R,G,B,A), Soft, DX10+
    IMAGE_B8G8R8A8_SRGB,
    IMAGE_B8G8R8,
    IMAGE_B8G8R8_SRGB,

    IMAGE_B5G6R5,
    IMAGE_B5G5R5A1,
    IMAGE_B4G4R4A4,

    IMAGE_D16,
    IMAGE_D24X8,
    IMAGE_D24S8,
    IMAGE_D32,
    IMAGE_D32S8X24,

    IMAGE_ETC1, // Ericsson 4-bit lossy RGB compression with no alpha (R,G,B,1), linear gamma, Soft, Android

    // compressing images to these formats is available only when 'SupportCompressASTC' was called in 'InitPre')
    IMAGE_ASTC_4x4,      // ASTC 8.00-bit lossy RGBA compression (R,G,B,A), linear gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_4x4_SRGB, // ASTC 8.00-bit lossy RGBA compression (R,G,B,A), sRGB   gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_5x4,      // ASTC 6.40-bit lossy RGBA compression (R,G,B,A), linear gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_5x4_SRGB, // ASTC 6.40-bit lossy RGBA compression (R,G,B,A), sRGB   gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_5x5,      // ASTC 5.12-bit lossy RGBA compression (R,G,B,A), linear gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_5x5_SRGB, // ASTC 5.12-bit lossy RGBA compression (R,G,B,A), sRGB   gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_6x5,      // ASTC 4.27-bit lossy RGBA compression (R,G,B,A), linear gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_6x5_SRGB, // ASTC 4.27-bit lossy RGBA compression (R,G,B,A), sRGB   gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_6x6,      // ASTC 3.56-bit lossy RGBA compression (R,G,B,A), linear gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_6x6_SRGB, // ASTC 3.56-bit lossy RGBA compression (R,G,B,A), sRGB   gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_8x5,      // ASTC 3.20-bit lossy RGBA compression (R,G,B,A), linear gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_8x5_SRGB, // ASTC 3.20-bit lossy RGBA compression (R,G,B,A), sRGB   gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_8x6,      // ASTC 2.67-bit lossy RGBA compression (R,G,B,A), linear gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_8x6_SRGB, // ASTC 2.67-bit lossy RGBA compression (R,G,B,A), sRGB   gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_8x8,      // ASTC 2.00-bit lossy RGBA compression (R,G,B,A), linear gamma, Soft, partial GL, GL ES
    IMAGE_ASTC_8x8_SRGB, // ASTC 2.00-bit lossy RGBA compression (R,G,B,A), sRGB   gamma, Soft, partial GL, GL ES

    IMAGE_R11G11B10F,
    IMAGE_R9G9B9E5F,

    IMAGE_ALL_TYPES, // number of all types
#endif
};
Bool IsSRGB(IMAGE_TYPE type);  // if this is a sRGB image
Bool IsSByte(IMAGE_TYPE type); // if image 'type' channels have signed byte/8-bit precision
enum IMAGE_MODE : Byte         // Image Mode
{
    IMAGE_2D,        // Hardware 2D   Texture
    IMAGE_3D,        // Hardware 3D   Texture
    IMAGE_CUBE,      // Hardware Cube Texture
    IMAGE_SOFT,      // Software      Image   (this type is used for software processing only - it can't be drawn on the screen)
    IMAGE_SOFT_CUBE, // Software Cube Image   (this type is used for software processing only - it can't be drawn on the screen)
    IMAGE_RT,        // Hardware RenderTarget (only this mode can be used as custom rendering destination for 'Renderer.target', after you have rendered to this image you can treat it as typical IMAGE_2D texture)
#if EE_PRIVATE
    IMAGE_RT_CUBE,    // Hardware RenderTarget Cube
    IMAGE_DS,         // Hardware Depth Stencil
    IMAGE_SHADOW_MAP, // Hardware Shadow Map (Depth Texture)
#if DX11
    IMAGE_STAGING, // DirectX Image used for copying data between CPU<->GPU
#elif GL
    IMAGE_GL_RB, // OpenGL Render Buffer (can be color or depth stencil)
#endif
#endif
};
Bool IsSoft(IMAGE_MODE mode); // if this is a software image     (IMAGE_SOFT, IMAGE_SOFT_CUBE)
Bool IsHW(IMAGE_MODE mode);   // if this is a hardware image NOT (IMAGE_SOFT, IMAGE_SOFT_CUBE)
Bool IsCube(IMAGE_MODE mode); // if this is a cube     image     (IMAGE_CUBE, IMAGE_SOFT_CUBE or IMAGE_RT_CUBE)
enum IMAGE_PRECISION : Byte   // Image Precision
{
    IMAGE_PRECISION_8,   // up to 8-bits
    IMAGE_PRECISION_10,  //      10-bits
    IMAGE_PRECISION_16,  //      16-bits
    IMAGE_PRECISION_24,  //      24-bits
    IMAGE_PRECISION_32,  //      32-bits
    IMAGE_PRECISION_64,  //      64-bits
    IMAGE_PRECISION_NUM, // number of precisions
};
#if EE_PRIVATE
inline IMAGE_PRECISION Min(IMAGE_PRECISION a, IMAGE_PRECISION b) { return (a < b) ? a : b; }
inline IMAGE_PRECISION Max(IMAGE_PRECISION a, IMAGE_PRECISION b) { return (a > b) ? a : b; }

IMAGE_PRECISION BitsToPrecision(Int bits);
#endif
enum CUBE_LAYOUT : Byte {
    CUBE_LAYOUT_ONE,
    CUBE_LAYOUT_CROSS, // 4x3 cross
    CUBE_LAYOUT_6x1,   // Left, Front, Right, Back, Down, Up
};
/******************************************************************************/
struct ImageTypeInfo // Image Type Information
{
    enum USAGE_FLAG {
        USAGE_VTX = 1 << 0,        // type can be used in a Vertex Buffer
        USAGE_IMAGE_2D = 1 << 1,   // type can be used in a 2D   Image
        USAGE_IMAGE_3D = 1 << 2,   // type can be used in a 3D   Image
        USAGE_IMAGE_CUBE = 1 << 3, // type can be used in a Cube Image
        USAGE_IMAGE_RT = 1 << 4,   // type can be used in a Render Target
        USAGE_IMAGE_DS = 1 << 5,   // type can be used in a Depth Stencil Buffer
        USAGE_IMAGE_MS = 1 << 6,   // type can be used in a Multi-Sampled Render Target or Depth Stencil (depending on USAGE_IMAGE_RT, USAGE_IMAGE_DS)
        USAGE_IMAGE_UAV = 1 << 7,  // type can be used in a Unordered Access View
    };

    const CChar8 *name;    // type name
    const Bool compressed, // if type is compressed
        high_precision;    // if type requires high precision storage (for example Flt/Vec4 instead of Byte/Color)
    const Byte byte_pp,    // bytes per pixel
        bit_pp,            // bits  per pixel
        r,                 // number of red     bits
        g,                 // number of green   bits
        b,                 // number of blue    bits
        a,                 // number of alpha   bits
        d,                 // number of depth   bits
        s,                 // number of stencil bits
        channels,          // number of channels
        block_w,           // number of pixels in block X
        block_h,           // number of pixels in block Y
        block_bytes;       // number of bytes  in block (size)
    const IMAGE_PRECISION precision;

    Byte usage() C { return _usage; } // get a combination of USAGE_FLAG, valid only if 'usageKnown' (on DX11/12, OpenGL 4.2+)
    static Bool usageKnown() { return _usage_known; }
#if EE_PRIVATE
    constexpr Bool filterable() C { return (!GL_ES) || (precision < IMAGE_PRECISION_32 && !d); } // GLES3 doesn't support filtering F32/Depth textures - https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glTexImage2D.xhtml , "depth textures are not filterable" - https://arm-software.github.io/opengl-es-sdk-for-android/occlusion_culling.html
#endif

#if !EE_PRIVATE
  private:
#endif
    Byte _usage;
#if EE_PRIVATE
    const GPU_API(DXGI_FORMAT, UInt) format;
#else
    const UInt format;
#endif
#if EE_PRIVATE
  private:
    friend struct DisplayClass;
#endif
    static Bool _usage_known;
};
extern ImageTypeInfo
    ImageTI[]; // Image Type Info Array, allows obtaining information about specified IMAGE_TYPE, sample usage: ImageTI[IMAGE_R8G8B8A8].name -> "R8G8B8A8"
/******************************************************************************/
enum IMAGE_COPY_FLAG {
    IC_CLAMP = 0,              // perform UVW coordinate clamping when filtering pixels
    IC_WRAP = 1 << 0,          // perform UVW coordinate wrapping when filtering pixels
    IC_ALPHA_WEIGHT = 1 << 1,  // if use pixel's alpha for weight of pixel's color
    IC_KEEP_EDGES = 1 << 2,    // if preserve the edges of the image when resizing
    IC_NO_ALT_TYPE = 1 << 3,   // don't try using alternative IMAGE_TYPE if a specified is not supported
    IC_CONVERT_GAMMA = 1 << 4, // make sure gamma conversion is performed (if no IC_CONVERT_GAMMA/IC_IGNORE_GAMMA are specified then choice is made automatically)
    IC_IGNORE_GAMMA = 1 << 5,  // make sure gamma conversion is ignored   (if no IC_CONVERT_GAMMA/IC_IGNORE_GAMMA are specified then choice is made automatically)
    IC_ENV_CUBE = 1 << 6,      // cube image is meant to be used as Environment Map and will have its mip maps blurred in a special way
#if EE_PRIVATE
    IC_NO_ALPHA_LIMIT = 1 << 7, // don't apply alpha limit when processing pixel colors
#endif
};
#if EE_PRIVATE
inline Bool IcWrap(UInt flag) { return FlagOn(flag, IC_WRAP); }
inline Bool IcClamp(UInt flag) { return FlagOff(flag, IC_WRAP); }
#endif
/******************************************************************************/
struct Image // Image (Texture)
{
    // get
    Int w() C { return _size.x; }       // get width                              (in pixels)
    Int h() C { return _size.y; }       // get height                             (in pixels)
    Int d() C { return _size.z; }       // get depth                              (in pixels)
    Int lw() C { return _lock_size.x; } // get width  of currently locked mip map (in pixels)
    Int lh() C { return _lock_size.y; } // get height of currently locked mip map (in pixels)
    Int ld() C { return _lock_size.z; } // get depth  of currently locked mip map (in pixels)
    Int hwW() C { return _hw_size.x; }  // get width  as it is stored on the GPU  (in pixels), this can be different than 'w' if the size is a non-power of 2, but the GPU does not support non-power of 2 textures, in which case the hardware size will be rounded up to the nearest power of 2
    Int hwH() C { return _hw_size.y; }  // get height as it is stored on the GPU  (in pixels), this can be different than 'h' if the size is a non-power of 2, but the GPU does not support non-power of 2 textures, in which case the hardware size will be rounded up to the nearest power of 2
    Int hwD() C { return _hw_size.z; }  // get depth  as it is stored on the GPU  (in pixels), this can be different than 'd' if the size is a non-power of 2, but the GPU does not support non-power of 2 textures, in which case the hardware size will be rounded up to the nearest power of 2

    C VecI2 &size() C { return _size.xy; }          // get image size (in pixels)
    C VecI &size3() C { return _size; }             // get image size (in pixels)
    C VecI2 &hwSize() C { return _hw_size.xy; }     // get image size as it is stored on the GPU (in pixels)
    C VecI &hwSize3() C { return _hw_size; }        // get image size as it is stored on the GPU (in pixels)
    C VecI2 &lockSize() C { return _lock_size.xy; } // get lock  size (in pixels)

    Bool is() C { return _type != IMAGE_NONE; }   // if  valid
    IMAGE_TYPE type() C { return _type; }         // get image  type
    IMAGE_TYPE hwType() C { return _hw_type; }    // get image  type in which it is stored on the GPU (this can be different than 'type' if it is not supported directly on the hardware, for example image was created as compressed format which the GPU does not support, 'type' will be set to the compressed format but 'hwType' may be set to R8G8B8A8 format as stored on the GPU)
    IMAGE_MODE mode() C { return _mode; }         // get image  mode
    Int mipMaps() C { return _mips; }             // get number of mip maps
    Byte samples() C { return _samples; }         // get number of samples per pixel
    Bool multiSample() C { return _samples > 1; } // if  this   is a multi sampled image
    Bool partial() C { return _partial; }         // if  'hwSize' is different than 'size'
    Int faces() C;                                // get how many faces this image has (0=empty, 1=default, 6=cube)
    Bool soft() C { return IsSoft(mode()); }      // if this is a software image     (IMAGE_SOFT, IMAGE_SOFT_CUBE)
    Bool hw() C { return IsHW(mode()); }          // if this is a hardware image NOT (IMAGE_SOFT, IMAGE_SOFT_CUBE)
    Bool cube() C { return IsCube(mode()); }      // if this is a cube     image     (IMAGE_CUBE, IMAGE_SOFT_CUBE or IMAGE_RT_CUBE)

    Int lMipMap() C { return _lock_mip; }         // get index              of locked mip map
    DIR_ENUM lCubeFace() C { return _lock_face; } // get                       locked cube face
    UInt pitch() C { return _pitch; }             // get width        pitch of locked mip map
    UInt pitch2() C { return _pitch2; }           // get width*height pitch of locked mip map
    Byte *data() { return _data; }                // get address            of locked data, memory accessed using this method should be interpreted according to 'hwType' (and not 'type')
    C Byte *data() C { return _data; }            // get address            of locked data, memory accessed using this method should be interpreted according to 'hwType' (and not 'type')
    LOCK_MODE lockMode() C { return _lock_mode; } // get current lock mode
    Int lockCount() C { return _lock_count; }     // get current lock count

    Flt aspect() C { return Flt(w()) / h(); }                        // get          aspect ratio of image "width/height"
    Flt invAspect() C { return Flt(h()) / w(); }                     // get inversed aspect ratio of image "height/width"
    UInt memUsage() C;                                               // get actual  memory usage of the image, this method operates on 'hwType' (what the image is using right now)
    UInt typeMemUsage() C;                                           // get desired memory usage of the image, this method operates on   'type' (what the image would use if it was stored in its desired type)
    C ImageTypeInfo &typeInfo() C { return ImageTI[_type]; }         // get image          type info
    C ImageTypeInfo &hwTypeInfo() C { return ImageTI[_hw_type]; }    // get image hardware type info
    Byte bytePP() C { return _byte_pp; }                             // get number of bytes per pixel
    Bool compressed() C { return hwTypeInfo().compressed; }          // if  hardware type is compressed
    IMAGE_PRECISION precision() C { return hwTypeInfo().precision; } // get image precision
    Bool highPrecision() C { return hwTypeInfo().high_precision; }   // if  image requires high precision storage (for example Flt/Vec4 instead of Byte/Color)
    Byte typeChannels() C { return typeInfo().channels; }            // get number of chanels for image type
    Bool sRGB() C { return IsSRGB(_hw_type); }                       // if  this is a sRGB image
    Bool isSByte() C { return IsSByte(_hw_type); }                   // if  this is a signed byte/8-bit precision
#if EE_PRIVATE
    constexpr Bool filterable() C { return hwTypeInfo().filterable() || _mode == IMAGE_SHADOW_MAP; } // even though IMAGE_SHADOW_MAP is a depth map, it can be filtered
    Byte baseMip() C { return _base_mip; }
#endif

    CUBE_LAYOUT cubeLayout() C; // auto-detect cube layout based on image size

    // manage
#if EE_PRIVATE
    Bool createEx(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps, Byte samples = 1, CPtr *data = null, Int base_mip = 0);
    Bool createHWfromSoft(C Image &soft, IMAGE_TYPE type, IMAGE_MODE mode, UInt flags = IC_CLAMP); // 'soft' must be software, its sizes have not been manipulated - 'adjustInfo' was not called, because we need correct 'size' 'hwSize'
#endif
    Image &del(); // delete

    Bool create(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps = 0, Bool alt_type_on_fail = true);                                                         // create                 image, 'mip_maps'=number of mip-maps (0=autodetect), 'alt_type_on_fail'=if try using an alternative type if 'type' is not supported, false on fail
    Bool create2D(Int w, Int h, IMAGE_TYPE type, Int mip_maps = 0, Bool alt_type_on_fail = true) { return create(w, h, 1, type, IMAGE_2D, mip_maps, alt_type_on_fail); }        // create hardware 2D   texture, 'mip_maps'=number of mip-maps (0=autodetect), 'alt_type_on_fail'=if try using an alternative type if 'type' is not supported, false on fail
    Bool create3D(Int w, Int h, Int d, IMAGE_TYPE type, Int mip_maps = 1, Bool alt_type_on_fail = true) { return create(w, h, d, type, IMAGE_3D, mip_maps, alt_type_on_fail); } // create hardware 3D   texture, 'mip_maps'=number of mip-maps (0=autodetect), 'alt_type_on_fail'=if try using an alternative type if 'type' is not supported, false on fail
    Bool createCube(Int w, IMAGE_TYPE type, Int mip_maps = 1, Bool alt_type_on_fail = true) { return create(w, w, 1, type, IMAGE_CUBE, mip_maps, alt_type_on_fail); }           // create hardware cube texture, 'mip_maps'=number of mip-maps (0=autodetect), 'alt_type_on_fail'=if try using an alternative type if 'type' is not supported, false on fail
    Bool createSoft(Int w, Int h, Int d, IMAGE_TYPE type, Int mip_maps = 1) { return create(w, h, d, type, IMAGE_SOFT, mip_maps, false); }                                      // create software        image, 'mip_maps'=number of mip-maps (0=autodetect),                                                                                 false on fail
    Bool createSoftCube(Int w, IMAGE_TYPE type, Int mip_maps = 1) { return create(w, w, 1, type, IMAGE_SOFT_CUBE, mip_maps, false); }                                           // create software cube   image, 'mip_maps'=number of mip-maps (0=autodetect),                                                                                 false on fail

    Image &mustCreate(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps = 0, Bool alt_type_on_fail = true);                                                             // create                 image, 'mip_maps'=number of mip-maps (0=autodetect), 'alt_type_on_fail'=if try using an alternative type if 'type' is not supported, Exit  on fail
    Image &mustCreate2D(Int w, Int h, IMAGE_TYPE type, Int mip_maps = 0, Bool alt_type_on_fail = true) { return mustCreate(w, h, 1, type, IMAGE_2D, mip_maps, alt_type_on_fail); }        // create hardware 2D   texture, 'mip_maps'=number of mip-maps (0=autodetect), 'alt_type_on_fail'=if try using an alternative type if 'type' is not supported, Exit  on fail
    Image &mustCreate3D(Int w, Int h, Int d, IMAGE_TYPE type, Int mip_maps = 1, Bool alt_type_on_fail = true) { return mustCreate(w, h, d, type, IMAGE_3D, mip_maps, alt_type_on_fail); } // create hardware 3D   texture, 'mip_maps'=number of mip-maps (0=autodetect), 'alt_type_on_fail'=if try using an alternative type if 'type' is not supported, Exit  on fail
    Image &mustCreateCube(Int w, IMAGE_TYPE type, Int mip_maps = 1, Bool alt_type_on_fail = true) { return mustCreate(w, w, 1, type, IMAGE_CUBE, mip_maps, alt_type_on_fail); }           // create hardware cube texture, 'mip_maps'=number of mip-maps (0=autodetect), 'alt_type_on_fail'=if try using an alternative type if 'type' is not supported, Exit  on fail
    Image &mustCreateSoft(Int w, Int h, Int d, IMAGE_TYPE type, Int mip_maps = 1) { return mustCreate(w, h, d, type, IMAGE_SOFT, mip_maps, false); }                                      // create software        image, 'mip_maps'=number of mip-maps (0=autodetect),                                                                                 Exit  on fail

    Bool copy(Image &dest, Int w = -1, Int h = -1, Int d = -1, Int type = -1, Int mode = -1, Int mip_maps = -1, FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP) C;     // copy to 'dest', -1=keep original value, 'type'=IMAGE_TYPE, 'mode'=IMAGE_MODE (this method does not support IMAGE_3D), 'mip_maps'=number of mip-maps (0=autodetect), 'flags'=IMAGE_COPY_FLAG, false on fail
    void mustCopy(Image &dest, Int w = -1, Int h = -1, Int d = -1, Int type = -1, Int mode = -1, Int mip_maps = -1, FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP) C; // copy to 'dest', -1=keep original value, 'type'=IMAGE_TYPE, 'mode'=IMAGE_MODE (this method does not support IMAGE_3D), 'mip_maps'=number of mip-maps (0=autodetect), 'flags'=IMAGE_COPY_FLAG, Exit  on fail

    // lock
#if EE_PRIVATE
    void lockSoft();
    void lockedSetMipData(CPtr data, Int mip_map); // set data for 'mip_map' for all faces, assumes that 'data' is in HW alignment !! NEEDS 'D._lock' !!
    Bool setFaceData(CPtr data, Int data_pitch, Int mip_map = 0, DIR_ENUM cube_face = DIR_RIGHT);
#endif
    Bool lock(LOCK_MODE lock = LOCK_READ_WRITE, Int mip_map = 0, DIR_ENUM cube_face = DIR_RIGHT); //   lock image for editing specified 'mip_map', this needs to be called before manual setting/getting pixels/colors on hardware images (IMAGE_SOFT doesn't need locking), 'cube_face'=desired cube face (this is used only for IMAGE_CUBE modes)
    Bool lockRead(Int mip_map = 0, DIR_ENUM cube_face = DIR_RIGHT) C;                             //   lock image for reading specified 'mip_map', this needs to be called before manual setting/getting pixels/colors on hardware images (IMAGE_SOFT doesn't need locking), 'cube_face'=desired cube face (this is used only for IMAGE_CUBE modes), this method has the same effect as calling "lock(LOCK_READ, mip_map, cube_face)", however unlike 'lock' method it has 'const' modifier and can be called on "const Image" objects
    Image &unlock();                                                                              // unlock image                                , this needs to be called after  manual setting/getting pixels/colors on hardware images (IMAGE_SOFT doesn't need locking), if you want the mip maps to be updated according to any change applied during the lock then you must call 'updateMipMaps' after 'unlock'
    C Image &unlock() C;                                                                          // unlock image                                , this needs to be called after  manual setting/getting pixels/colors on hardware images (IMAGE_SOFT doesn't need locking), if you want the mip maps to be updated according to any change applied during the lock then you must call 'updateMipMaps' after 'unlock'

    Byte *softData() { return _data_all; }                                                                                // get software image data without locking the image
    C Byte *softData() C { return _data_all; }                                                                            // get software image data without locking the image
    Byte *softData(Int mip_map, DIR_ENUM cube_face = DIR_RIGHT);                                                          // get software image data for 'mip_map' and 'cube_face' without locking the image
    C Byte *softData(Int mip_map, DIR_ENUM cube_face = DIR_RIGHT) C { return ConstCast(T).softData(mip_map, cube_face); } // get software image data for 'mip_map' and 'cube_face' without locking the image
    UInt softPitch(Int mip_map) C;                                                                                        // get pitch     of specified 'mip_map'
    UInt softBlocksY(Int mip_map) C;                                                                                      // get blocksY   of specified 'mip_map'
    UInt softPitch2(Int mip_map) C;                                                                                       // get pitch2    of specified 'mip_map' = pitch  * blocksY
    Int softFaceSize(Int mip_map) C;                                                                                      // get face size of specified 'mip_map' = pitch2 * depth
    Int softMipSize(Int mip_map) C;                                                                                       // get mip  size of specified 'mip_map' = face   * faces

#if EE_PRIVATE
    void lockedBaseMip(Int base_mip); // !! NEEDS 'D._lock' !!
    void baseMip(Int base_mip);
    void cancelStream();
    Bool updateMipMaps(C Image &src, Int src_mip, FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP, Int mip_start = 0);
#endif
    Bool waitForStream(Int mip = 0) C;                                                                // wait until streaming has finished, false on fail
    Image &updateMipMaps(FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP, Int mip_start = 0); // update mip maps of the image, 'flags'=IMAGE_COPY_FLAG, 'mip_start'=index of the mip map to start with (this mip map will be taken, and downsampled to following mip maps)

    Bool blurCubeAngle(Flt angle, Bool linear);       // blur cube image,                                             'linear'=if image pixels are in linear space (or false in spherical space), false on fail
    Bool blurCubePixel(Flt pixel_range, Bool linear); // blur cube image, angle is calculated based on 'pixel_range', 'linear'=if image pixels are in linear space (or false in spherical space), false on fail
    Bool blurCubeMipMaps();                           // blur mip maps based on increasing angles per mip-map, this method is only for Cube Images, false on fail

    Image &freeOpenGLESData(); // this method is used only under OpenGL ES (on other platforms it is ignored), the method frees the software copy of the GPU data which increases available memory, however after calling this method the data can no longer be accessed on the CPU (can no longer be locked or saved to file)

    // pixel
    UInt pixel(Int x, Int y) C;
    void pixel(Int x, Int y, UInt pixel); // get/set pixel UInt value, image  gamma (no gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixelF(Int x, Int y) C;
    void pixelF(Int x, Int y, Flt pixel); // get/set pixel Flt  value, image  gamma (no gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixelL(Int x, Int y) C;
    void pixelL(Int x, Int y, Flt pixel); // get/set pixel Flt  value, linear gamma (   gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)
    Color color(Int x, Int y) C;
    void color(Int x, Int y, C Color &color); // get/set color Byte color, image  gamma (no gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 colorF(Int x, Int y) C;
    void colorF(Int x, Int y, C Vec4 &color); // get/set color Flt  color, image  gamma (no gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 colorL(Int x, Int y) C;
    void colorL(Int x, Int y, C Vec4 &color); // get/set color Flt  color, linear gamma (   gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 colorS(Int x, Int y) C;
    void colorS(Int x, Int y, C Vec4 &color); // get/set color Flt  color, sRGB   gamma (   gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)

    void blendF(Int x, Int y, C Vec4 &color); // apply 'color' pixel using ALPHA_BLEND formula
    void mergeF(Int x, Int y, C Vec4 &color); // apply 'color' pixel using ALPHA_MERGE formula

    Flt pixelFNearest(Flt x, Flt y, Bool clamp = true) C;         // get pixel Flt with no                interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixelFLinear(Flt x, Flt y, Bool clamp = true) C;          // get pixel Flt with Linear            interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixelLLinear(Flt x, Flt y, Bool clamp = true) C;          // get pixel Flt with Linear            interpolation, linear gamma (   gamma conversion), 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixelFCubicFast(Flt x, Flt y, Bool clamp = true) C;       // get pixel Flt with Cubic Fast        interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixelFCubicFastSmooth(Flt x, Flt y, Bool clamp = true) C; // get pixel Flt with Cubic Fast Smooth interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixelFCubicFastMed(Flt x, Flt y, Bool clamp = true) C;    // get pixel Flt with Cubic Fast Med    interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixelFCubicFastSharp(Flt x, Flt y, Bool clamp = true) C;  // get pixel Flt with Cubic Fast Sharp  interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixelFCubicPlus(Flt x, Flt y, Bool clamp = true) C;       // get pixel Flt with Cubic Plus        interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixelFCubicPlusSharp(Flt x, Flt y, Bool clamp = true) C;  // get pixel Flt with Cubic Plus Sharp  interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)

    Vec4 colorFNearest(Flt x, Flt y, Bool clamp = true, Bool alpha_weight = false) C;         // get color Vec4 with no                interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 colorFLinear(Flt x, Flt y, Bool clamp = true, Bool alpha_weight = false) C;          // get color Vec4 with Linear            interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 colorFCubicFast(Flt x, Flt y, Bool clamp = true, Bool alpha_weight = false) C;       // get color Vec4 with Cubic Fast        interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 colorLCubicFast(Flt x, Flt y, Bool clamp = true, Bool alpha_weight = false) C;       // get color Vec4 with Cubic Fast        interpolation, linear gamma (   gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 colorSCubicFast(Flt x, Flt y, Bool clamp = true, Bool alpha_weight = false) C;       // get color Vec4 with Cubic Fast        interpolation, sRGB   gamma (   gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 colorFCubicFastSmooth(Flt x, Flt y, Bool clamp = true, Bool alpha_weight = false) C; // get color Vec4 with Cubic Fast Smooth interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 colorFCubicFastMed(Flt x, Flt y, Bool clamp = true, Bool alpha_weight = false) C;    // get color Vec4 with Cubic Fast Med    interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 colorFCubicFastSharp(Flt x, Flt y, Bool clamp = true, Bool alpha_weight = false) C;  // get color Vec4 with Cubic Fast Sharp  interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 colorFCubicPlus(Flt x, Flt y, Bool clamp = true, Bool alpha_weight = false) C;       // get color Vec4 with Cubic Plus        interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 colorFCubicPlusSharp(Flt x, Flt y, Bool clamp = true, Bool alpha_weight = false) C;  // get color Vec4 with Cubic Plus Sharp  interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
#if EE_PRIVATE
    Vec4 colorFLinearTTNF32_4(Flt x, Flt y, Bool clamp) C; // optimized version specifically for 'transparentToNeighbor', image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels
#endif

    // pixel 3D
    UInt pixel3D(Int x, Int y, Int z) C;
    void pixel3D(Int x, Int y, Int z, UInt pixel); // get/set pixel 3D UInt value, image  gamma (no gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixel3DF(Int x, Int y, Int z) C;
    void pixel3DF(Int x, Int y, Int z, Flt pixel); // get/set pixel 3D Flt  value, image  gamma (no gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixel3DL(Int x, Int y, Int z) C;
    void pixel3DL(Int x, Int y, Int z, Flt pixel); // get/set pixel 3D Flt  value, linear gamma (   gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)
    Color color3D(Int x, Int y, Int z) C;
    void color3D(Int x, Int y, Int z, C Color &color); // get/set color 3D Byte color, image  gamma (no gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 color3DF(Int x, Int y, Int z) C;
    void color3DF(Int x, Int y, Int z, C Vec4 &color); // get/set color 3D Flt  color, image  gamma (no gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 color3DL(Int x, Int y, Int z) C;
    void color3DL(Int x, Int y, Int z, C Vec4 &color); // get/set color 3D Flt  color, linear gamma (   gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 color3DS(Int x, Int y, Int z) C;
    void color3DS(Int x, Int y, Int z, C Vec4 &color); // get/set color 3D Flt  color, sRGB   gamma (   gamma conversion), (these methods may not support all compressed types, instead try using 'copy' method first)

    Flt pixel3DFNearest(Flt x, Flt y, Flt z, Bool clamp = true) C;         // get 3D pixel Flt with no                interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixel3DFLinear(Flt x, Flt y, Flt z, Bool clamp = true) C;          // get 3D pixel Flt with Linear            interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixel3DFCubicFast(Flt x, Flt y, Flt z, Bool clamp = true) C;       // get 3D pixel Flt with Cubic Fast        interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixel3DFCubicFastSmooth(Flt x, Flt y, Flt z, Bool clamp = true) C; // get 3D pixel Flt with Cubic Fast Smooth interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixel3DFCubicFastMed(Flt x, Flt y, Flt z, Bool clamp = true) C;    // get 3D pixel Flt with Cubic Fast Med    interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixel3DFCubicFastSharp(Flt x, Flt y, Flt z, Bool clamp = true) C;  // get 3D pixel Flt with Cubic Fast Sharp  interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixel3DFCubicPlus(Flt x, Flt y, Flt z, Bool clamp = true) C;       // get 3D pixel Flt with Cubic Plus        interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)
    Flt pixel3DFCubicPlusSharp(Flt x, Flt y, Flt z, Bool clamp = true) C;  // get 3D pixel Flt with Cubic Plus Sharp  interpolation, 'clamp'=if use clamping when filtering pixels, (these methods may not support all compressed types, instead try using 'copy' method first)

    Vec4 color3DFNearest(Flt x, Flt y, Flt z, Bool clamp = true, Bool alpha_weight = false) C;         // get 3D color Vec4 with no                interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 color3DFLinear(Flt x, Flt y, Flt z, Bool clamp = true, Bool alpha_weight = false) C;          // get 3D color Vec4 with Linear            interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 color3DFCubicFast(Flt x, Flt y, Flt z, Bool clamp = true, Bool alpha_weight = false) C;       // get 3D color Vec4 with Cubic Fast        interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 color3DFCubicFastSmooth(Flt x, Flt y, Flt z, Bool clamp = true, Bool alpha_weight = false) C; // get 3D color Vec4 with Cubic Fast Smooth interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 color3DFCubicFastMed(Flt x, Flt y, Flt z, Bool clamp = true, Bool alpha_weight = false) C;    // get 3D color Vec4 with Cubic Fast Med    interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 color3DFCubicFastSharp(Flt x, Flt y, Flt z, Bool clamp = true, Bool alpha_weight = false) C;  // get 3D color Vec4 with Cubic Fast Sharp  interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 color3DFCubicPlus(Flt x, Flt y, Flt z, Bool clamp = true, Bool alpha_weight = false) C;       // get 3D color Vec4 with Cubic Plus        interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 color3DFCubicPlusSharp(Flt x, Flt y, Flt z, Bool clamp = true, Bool alpha_weight = false) C;  // get 3D color Vec4 with Cubic Plus Sharp  interpolation, 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)

    // pixel area
    Vec4 areaColorFAverage(C Vec2 &pos, C Vec2 &size, Bool clamp = true, Bool alpha_weight = false) C;         // get average color Vec4 of specified 'pos' position and 'size' coverage with Average           interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 areaColorFLinear(C Vec2 &pos, C Vec2 &size, Bool clamp = true, Bool alpha_weight = false) C;          // get average color Vec4 of specified 'pos' position and 'size' coverage with Linear            interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 areaColorLLinear(C Vec2 &pos, C Vec2 &size, Bool clamp = true, Bool alpha_weight = false) C;          // get average color Vec4 of specified 'pos' position and 'size' coverage with Linear            interpolation, linear gamma (   gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 areaColorFCubicFast(C Vec2 &pos, C Vec2 &size, Bool clamp = true, Bool alpha_weight = false) C;       // get average color Vec4 of specified 'pos' position and 'size' coverage with Cubic Fast        interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 areaColorLCubicFast(C Vec2 &pos, C Vec2 &size, Bool clamp = true, Bool alpha_weight = false) C;       // get average color Vec4 of specified 'pos' position and 'size' coverage with Cubic Fast        interpolation, linear gamma (   gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 areaColorFCubicFastSmooth(C Vec2 &pos, C Vec2 &size, Bool clamp = true, Bool alpha_weight = false) C; // get average color Vec4 of specified 'pos' position and 'size' coverage with Cubic Fast Smooth interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 areaColorLCubicFastSmooth(C Vec2 &pos, C Vec2 &size, Bool clamp = true, Bool alpha_weight = false) C; // get average color Vec4 of specified 'pos' position and 'size' coverage with Cubic Fast Smooth interpolation, linear gamma (   gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 areaColorFCubicFastMed(C Vec2 &pos, C Vec2 &size, Bool clamp = true, Bool alpha_weight = false) C;    // get average color Vec4 of specified 'pos' position and 'size' coverage with Cubic Fast Med    interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 areaColorFCubicFastSharp(C Vec2 &pos, C Vec2 &size, Bool clamp = true, Bool alpha_weight = false) C;  // get average color Vec4 of specified 'pos' position and 'size' coverage with Cubic Fast Sharp  interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 areaColorFCubicPlus(C Vec2 &pos, C Vec2 &size, Bool clamp = true, Bool alpha_weight = false) C;       // get average color Vec4 of specified 'pos' position and 'size' coverage with Cubic Plus        interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)
    Vec4 areaColorFCubicPlusSharp(C Vec2 &pos, C Vec2 &size, Bool clamp = true, Bool alpha_weight = false) C;  // get average color Vec4 of specified 'pos' position and 'size' coverage with Cubic Plus Sharp  interpolation, image  gamma (no gamma conversion), 'clamp'=if use clamping when filtering pixels, 'alpha_weight'=if use pixel's alpha for weight of pixel's color (these methods may not support all compressed types, instead try using 'copy' method first)

    // pixel cube
    Flt cubePixelFNearest(C Vec &dir, Bool linear) C;   // get pixel Flt of specified 'dir' direction (doesn't need to be normalized), with no         interpolation, image gamma (no gamma conversion), this function supports only non-compressed IMAGE_SOFT_CUBE, 'linear'=if image pixels are in linear space (or false in spherical space)
    Flt cubePixelFLinear(C Vec &dir, Bool linear) C;    // get pixel Flt of specified 'dir' direction (doesn't need to be normalized), with Linear     interpolation, image gamma (no gamma conversion), this function supports only non-compressed IMAGE_SOFT_CUBE, 'linear'=if image pixels are in linear space (or false in spherical space)
    Flt cubePixelFCubicFast(C Vec &dir, Bool linear) C; // get pixel Flt of specified 'dir' direction (doesn't need to be normalized), with Cubic Fast interpolation, image gamma (no gamma conversion), this function supports only non-compressed IMAGE_SOFT_CUBE, 'linear'=if image pixels are in linear space (or false in spherical space)

    Vec4 cubeColorFNearest(C Vec &dir, Bool linear) C;   // get color Vec4 of specified 'dir' direction (doesn't need to be normalized), with no         interpolation, image gamma (no gamma conversion), this function supports only non-compressed IMAGE_SOFT_CUBE, 'linear'=if image pixels are in linear space (or false in spherical space)
    Vec4 cubeColorFLinear(C Vec &dir, Bool linear) C;    // get color Vec4 of specified 'dir' direction (doesn't need to be normalized), with Linear     interpolation, image gamma (no gamma conversion), this function supports only non-compressed IMAGE_SOFT_CUBE, 'linear'=if image pixels are in linear space (or false in spherical space)
    Vec4 cubeColorFCubicFast(C Vec &dir, Bool linear) C; // get color Vec4 of specified 'dir' direction (doesn't need to be normalized), with Cubic Fast interpolation, image gamma (no gamma conversion), this function supports only non-compressed IMAGE_SOFT_CUBE, 'linear'=if image pixels are in linear space (or false in spherical space)

    // operations
    Image &clear();                                                                                                                                                                             // clear to 0 (transparent black)
    Image &normalize(Bool red = true, Bool green = true, Bool blue = true, Bool alpha = true, C BoxI *box = null);                                                                              // normalize selected components to 0..1 range, 'box'=optional box in which perform the operation (use null for entire image)
    Image &mulAdd(C Vec4 &mul, C Vec4 &add, C BoxI *box = null);                                                                                                                                // transform color to "color*mul+add"         , 'box'=optional box in which perform the operation (use null for entire image)
    void bumpToNormal(Image &dest, Flt scale, Bool high_precision = false) C;                                                                                                                   // convert bump map to normal map, 'scale'=bump scaling factor 0..Inf, 'high_precision'=if create signed float image type, or unsigned byte image type
    void crop(Image &dest, Int x, Int y, Int w, Int h, C Vec4 *clear_color = &Vec4Zero) C;                                                                                                      // crop      image, 'clear_color'=color to be used for pixels that didn't exist in the source (if null then they will remain uninitialized)
    void crop3D(Image &dest, Int x, Int y, Int z, Int w, Int h, Int d, C Vec4 *clear_color = &Vec4Zero) C;                                                                                      // crop   3D image, 'clear_color'=color to be used for pixels that didn't exist in the source (if null then they will remain uninitialized)
    Image &cropTransparent(Bool border = false);                                                                                                                                                // crop transparent pixels, 'border'=if keep 1 pixel border
    Image &resize(Int w, Int h, FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP);                                                                                                       // resize    image, 'flags'=IMAGE_COPY_FLAG
    Image &resize3D(Int w, Int h, Int d, FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP);                                                                                              // resize 3D image, 'flags'=IMAGE_COPY_FLAG
    Image &mirrorX();                                                                                                                                                                           // mirror    image horizontally
    Image &mirrorY();                                                                                                                                                                           // mirror    image   vertically
    Image &mirrorXY();                                                                                                                                                                          // mirror    image horizontally and vertically
    Image &rotateL();                                                                                                                                                                           // rotate    image left
    Image &rotateR();                                                                                                                                                                           // rotate    image right
    void rotate(Image &dest, Flt angle, FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP) C;                                                                                             // rotate image by 'angle'                    and store in 'dest'
    void rotateScale(Image &dest, Flt angle, Flt scale, FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP) C;                                                                             // rotate image by 'angle', scale by 'scale', and store in 'dest'
    Image &alphaFromKey(C Color &key = PURPLE);                                                                                                                                                 // transform to ((pixel color==key) ? (0, 0, 0, 0) : (r, g, b, 255))
    Image &alphaFromBrightness();                                                                                                                                                               // transform to (r  , g  , b  , brightness)
    Image &divRgbByAlpha();                                                                                                                                                                     // transform to (r/a, g/a, b/a, a)
    Bool stats(Vec4 *min = null, Vec4 *max = null, Vec4 *avg = null, Vec4 *median = null, Vec4 *mode = null, Vec *avg_alpha_weight = null, Vec *med_alpha_weight = null, C BoxI *box = null) C; // get image statistics               , such as: minimum, maximum, average, median and mode color values, 'box'=optional box in which perform the operation (use null for entire image), false on fail
    Bool statsSat(Flt *min = null, Flt *max = null, Flt *avg = null, Flt *median = null, Flt *mode = null, Flt *avg_alpha_weight = null, Flt *med_alpha_weight = null, C BoxI *box = null) C;   // get image statistics for saturation, such as: minimum, maximum, average, median and mode       values, 'box'=optional box in which perform the operation (use null for entire image), false on fail
    Bool needsAlpha() C;                                                                                                                                                                        // check if image is needs alpha channel (any A   value  is != 1)
    Bool monochromatic() C;                                                                                                                                                                     // check if image is monochromatic       (all RGB values are the same)
    Bool monochromaticRG() C;                                                                                                                                                                   // check if image is monochromatic       (all RG  values are the same, Blue values are ignored)
#if EE_PRIVATE
    void transform(Image &dest, C Matrix2 &matrix, FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP) C; // transform image by 'matrix' and store in 'dest'
    Bool extractNonCompressedMipMapNoStretch(Image &dest, Int w, Int h, Int d, Int mip_map, DIR_ENUM cube_face = DIR_RIGHT, Bool clamp = true) C;
#endif
    Bool extractMipMap(Image &dest, Int type, Int mip_map, DIR_ENUM cube_face = DIR_RIGHT, FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP) C; // extract specified mipmap to   'dest' 0-th mipmap, false on fail, 'type'=IMAGE_TYPE (-1=keep), 'dest' will always be IMAGE_SOFT, 'filter'=what kind of filtering to use when source is of different size than the target, 'flags'=IMAGE_COPY_FLAG
    Bool injectMipMap(C Image &src, Int mip_map, DIR_ENUM cube_face = DIR_RIGHT, FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP);             //  inject specified mipmap from 'src'  0-th mipmap, false on fail                                                               , 'filter'=what kind of filtering to use when source is of different size than the target, 'flags'=IMAGE_COPY_FLAG
    Image &downSample(FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP);                                                                        // downsample to half resolution size, 'flags'=IMAGE_COPY_FLAG
    Bool averageX(Image &dest, Int range, Bool clamp) C;                                                                                               // horizontal average, 'range'=range of blurring (in pixels), false on fail
    Bool averageY(Image &dest, Int range, Bool clamp) C;                                                                                               // vertical   average, 'range'=range of blurring (in pixels), false on fail
    Bool averageZ(Image &dest, Int range, Bool clamp) C;                                                                                               // depth      average, 'range'=range of blurring (in pixels), false on fail
    Bool average(Image &dest, C VecI &range, Bool clamp) C;                                                                                            //            average, 'range'=range of blurring (in pixels), false on fail
    Image &average(C VecI &range, Bool clamp);                                                                                                         //            average, 'range'=range of blurring (in pixels)
    Bool blurX(Image &dest, Flt range, Bool clamp) C;                                                                                                  // horizontal blur   , 'range'=range of blurring (in pixels), false on fail
    Bool blurY(Image &dest, Flt range, Bool clamp) C;                                                                                                  // vertical   blur   , 'range'=range of blurring (in pixels), false on fail
    Bool blurZ(Image &dest, Flt range, Bool clamp) C;                                                                                                  // depth      blur   , 'range'=range of blurring (in pixels), false on fail
    Bool blur(Image &dest, C Vec &range, Bool clamp) C;                                                                                                //            blur   , 'range'=range of blurring (in pixels), false on fail
    Image &blur(C Vec &range, Bool clamp);                                                                                                             //            blur   , 'range'=range of blurring (in pixels)
    Image &sharpen(Flt power, C Vec &range, Bool clamp, Bool blur = true);                                                                             // sharpen image, 'power'=0..Inf (default=1.0), 'range'=blurring range, 'clamp'=blurring clamp, 'blur'=if use blur or averaging
    Image &noise(Byte red, Byte green, Byte blue, Byte alpha);                                                                                         // add noise of selected scale per component to image
    Image &RGBToHSB();                                                                                                                                 // convert Red Green Blue image to Hue Saturation Brightness (Alpha will be kept)
    Image &HSBToRGB();                                                                                                                                 // convert Hue Saturation Brightness image to Red Green Blue (Alpha will be kept)
    Image &tile(C VecI2 &range);                                                                                                                       // make tileable, 'range'=number of pixels to blend
    Image &minimum(Flt distance);                                                                                                                      // apply minimum filter, 'distance'=pixel range (0..1)
    Image &maximum(Flt distance);                                                                                                                      // apply maximum filter, 'distance'=pixel range (0..1)
    Bool getSameColorNeighbors(Int x, Int y, MemPtr<VecI2> pixels, Bool diagonal = true) C;                                                            // get a list of all neighbor pixels   with the same color, 'diagonal'=if allow diagonal movements, false on fail
    Image &fill(Int x, Int y, C Color &color, Bool diagonal = true);                                                                                   // fill image at specified coordinates with given    color, 'diagonal'=if allow diagonal movements

    Image &transparentToNeighbor(Bool clamp = true, Flt step = 1);        // replace transparent pixels with neighbors
    Image &transparentForFiltering(Bool clamp = true, Flt intensity = 1); // adjust  transparent pixels to optimize for filtering

    Image &createShadow(C Image &src, Int blur, Flt shadow_opacity = 1.0f, Flt shadow_spread = 0.0f, Bool border_padd = true);                                              // create shadow image IMAGE_A8 from 'src' alpha channel, 'blur'=blur range (in pixels), 'shadow_opacity'=shadow opacity (0..1), 'shadow_spread'=shadow spread (0..1)
    Image &applyShadow(C Image &shadow, C Color &shadow_color = BLACK, C VecI2 &offset = 0, Int image_type = 0, Bool combine = true);                                       // apply  shadow image to self, 'offset'=offset in pixels where to apply the shadow, 'image_type'=IMAGE_TYPE (-1=keep, 0=autodetect), 'combine'=if combine on to the color map (if false then alpha will be set to shadow intensity)
    Image &setShadow(Int blur, Flt shadow_opacity = 1.0f, Flt shadow_spread = 0.0f, Bool border_padd = true, C VecI2 &offset = 0, Int image_type = 0, Bool combine = true); // create shadow and apply to self

#if EE_PRIVATE
    Bool accessible() C;
    Bool compatible(C Image &image) C;

    Bool toCube(C Image &src, Int layout = -1, Int size = -1, Int type = -1, Int mode = -1, Int mip_maps = -1, FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP); // convert from 'src' image      to cube image, 'size'=desired resolution of the image (-1=keep), 'type'=IMAGE_TYPE (-1=keep), 'mode'=IMAGE_MODE (-1=auto), 'mip_maps'=number of mip-maps (0=autodetect), 'filter'=what kind of filtering to use when source is of different size than the target, 'flags'=IMAGE_COPY_FLAG
    Bool fromCube(C Image &src, Int uncompressed_type = -1);                                                                                                             // convert from 'src' cube image to 6x1  image,                                      'uncompressed_type'=IMAGE_TYPE to use if source is compressed
#endif

    Bool raycast(C Vec &start, C Vec &move, C Matrix *image_matrix = null, Flt *hit_frac = null, Vec *hit_pos = null, Flt precision = 1.0f) C; // perform ray casting from 'start' position along 'move' vector, return true if collision encountered with the image, by default the Image is on XY plane with its pixel values extending it towards the Z axis, 'image_matrix'=image transformation matrix, 'hit_frac'=fraction of the movement where collision occurs, 'hit_pos'=position where collision occurs, 'precision'=affects number of pixels to advance in a single step (lower value makes calculations faster but less precise)

    // !! WARNING: following methods do not check for coordinates being out of range, image type matching, and image being locked, use only if you know what you're doing !!
    Byte &pixB(Int x, Int y) { return *(Byte *)(_data + x * SIZE(Byte) + y * _pitch); }
    Byte &pixB(C VecI2 &v) { return pixB(v.x, v.y); }
    UInt &pix(Int x, Int y) { return *(UInt *)(_data + x * SIZE(UInt) + y * _pitch); }
    UInt &pix(C VecI2 &v) { return pix(v.x, v.y); }
    Color &pixC(Int x, Int y) { return *(Color *)(_data + x * SIZE(Color) + y * _pitch); }
    Color &pixC(C VecI2 &v) { return pixC(v.x, v.y); }
    VecB &pixB3(Int x, Int y) { return *(VecB *)(_data + x * SIZE(VecB) + y * _pitch); }
    VecB &pixB3(C VecI2 &v) { return pixB3(v.x, v.y); }
    VecB4 &pixB4(Int x, Int y) { return *(VecB4 *)(_data + x * SIZE(VecB4) + y * _pitch); }
    VecB4 &pixB4(C VecI2 &v) { return pixB4(v.x, v.y); }
    Flt &pixF(Int x, Int y) { return *(Flt *)(_data + x * SIZE(Flt) + y * _pitch); }
    Flt &pixF(C VecI2 &v) { return pixF(v.x, v.y); }
    Vec2 &pixF2(Int x, Int y) { return *(Vec2 *)(_data + x * SIZE(Vec2) + y * _pitch); }
    Vec2 &pixF2(C VecI2 &v) { return pixF2(v.x, v.y); }
    Vec &pixF3(Int x, Int y) { return *(Vec *)(_data + x * SIZE(Vec) + y * _pitch); }
    Vec &pixF3(C VecI2 &v) { return pixF3(v.x, v.y); }
    Vec4 &pixF4(Int x, Int y) { return *(Vec4 *)(_data + x * SIZE(Vec4) + y * _pitch); }
    Vec4 &pixF4(C VecI2 &v) { return pixF4(v.x, v.y); }

    // 3D
    Byte &pixB(Int x, Int y, Int z) { return *(Byte *)(_data + x * SIZE(Byte) + y * _pitch + z * _pitch2); }
    Byte &pixB(C VecI &v) { return pixB(v.x, v.y, v.z); }
    UInt &pix(Int x, Int y, Int z) { return *(UInt *)(_data + x * SIZE(UInt) + y * _pitch + z * _pitch2); }
    UInt &pix(C VecI &v) { return pix(v.x, v.y, v.z); }
    Color &pixC(Int x, Int y, Int z) { return *(Color *)(_data + x * SIZE(Color) + y * _pitch + z * _pitch2); }
    Color &pixC(C VecI &v) { return pixC(v.x, v.y, v.z); }
    VecB &pixB3(Int x, Int y, Int z) { return *(VecB *)(_data + x * SIZE(VecB) + y * _pitch + z * _pitch2); }
    VecB &pixB3(C VecI &v) { return pixB3(v.x, v.y, v.z); }
    VecB4 &pixB4(Int x, Int y, Int z) { return *(VecB4 *)(_data + x * SIZE(VecB4) + y * _pitch + z * _pitch2); }
    VecB4 &pixB4(C VecI &v) { return pixB4(v.x, v.y, v.z); }
    Flt &pixF(Int x, Int y, Int z) { return *(Flt *)(_data + x * SIZE(Flt) + y * _pitch + z * _pitch2); }
    Flt &pixF(C VecI &v) { return pixF(v.x, v.y, v.z); }
    Vec2 &pixF2(Int x, Int y, Int z) { return *(Vec2 *)(_data + x * SIZE(Vec2) + y * _pitch + z * _pitch2); }
    Vec2 &pixF2(C VecI &v) { return pixF2(v.x, v.y, v.z); }
    Vec &pixF3(Int x, Int y, Int z) { return *(Vec *)(_data + x * SIZE(Vec) + y * _pitch + z * _pitch2); }
    Vec &pixF3(C VecI &v) { return pixF3(v.x, v.y, v.z); }
    Vec4 &pixF4(Int x, Int y, Int z) { return *(Vec4 *)(_data + x * SIZE(Vec4) + y * _pitch + z * _pitch2); }
    Vec4 &pixF4(C VecI &v) { return pixF4(v.x, v.y, v.z); }

    // IMAGE_SOFT_CUBE only (no IMAGE_CUBE)
    Byte &pixB(Int x, Int y, DIR_ENUM cube_face) { return *(Byte *)(_data + x * SIZE(Byte) + y * _pitch + cube_face * _pitch2); }
    Byte &pixB(C VecI2 &v, DIR_ENUM cube_face) { return pixB(v.x, v.y, cube_face); }
    UInt &pix(Int x, Int y, DIR_ENUM cube_face) { return *(UInt *)(_data + x * SIZE(UInt) + y * _pitch + cube_face * _pitch2); }
    UInt &pix(C VecI2 &v, DIR_ENUM cube_face) { return pix(v.x, v.y, cube_face); }
    Color &pixC(Int x, Int y, DIR_ENUM cube_face) { return *(Color *)(_data + x * SIZE(Color) + y * _pitch + cube_face * _pitch2); }
    Color &pixC(C VecI2 &v, DIR_ENUM cube_face) { return pixC(v.x, v.y, cube_face); }
    VecB &pixB3(Int x, Int y, DIR_ENUM cube_face) { return *(VecB *)(_data + x * SIZE(VecB) + y * _pitch + cube_face * _pitch2); }
    VecB &pixB3(C VecI2 &v, DIR_ENUM cube_face) { return pixB3(v.x, v.y, cube_face); }
    VecB4 &pixB4(Int x, Int y, DIR_ENUM cube_face) { return *(VecB4 *)(_data + x * SIZE(VecB4) + y * _pitch + cube_face * _pitch2); }
    VecB4 &pixB4(C VecI2 &v, DIR_ENUM cube_face) { return pixB4(v.x, v.y, cube_face); }
    Flt &pixF(Int x, Int y, DIR_ENUM cube_face) { return *(Flt *)(_data + x * SIZE(Flt) + y * _pitch + cube_face * _pitch2); }
    Flt &pixF(C VecI2 &v, DIR_ENUM cube_face) { return pixF(v.x, v.y, cube_face); }
    Vec2 &pixF2(Int x, Int y, DIR_ENUM cube_face) { return *(Vec2 *)(_data + x * SIZE(Vec2) + y * _pitch + cube_face * _pitch2); }
    Vec2 &pixF2(C VecI2 &v, DIR_ENUM cube_face) { return pixF2(v.x, v.y, cube_face); }
    Vec &pixF3(Int x, Int y, DIR_ENUM cube_face) { return *(Vec *)(_data + x * SIZE(Vec) + y * _pitch + cube_face * _pitch2); }
    Vec &pixF3(C VecI2 &v, DIR_ENUM cube_face) { return pixF3(v.x, v.y, cube_face); }
    Vec4 &pixF4(Int x, Int y, DIR_ENUM cube_face) { return *(Vec4 *)(_data + x * SIZE(Vec4) + y * _pitch + cube_face * _pitch2); }
    Vec4 &pixF4(C VecI2 &v, DIR_ENUM cube_face) { return pixF4(v.x, v.y, cube_face); }

    // CONST
    C Byte &pixB(Int x, Int y) C { return *(Byte *)(_data + x * SIZE(Byte) + y * _pitch); }
    C Byte &pixB(C VecI2 &v) C { return pixB(v.x, v.y); }
    C UInt &pix(Int x, Int y) C { return *(UInt *)(_data + x * SIZE(UInt) + y * _pitch); }
    C UInt &pix(C VecI2 &v) C { return pix(v.x, v.y); }
    C Color &pixC(Int x, Int y) C { return *(Color *)(_data + x * SIZE(Color) + y * _pitch); }
    C Color &pixC(C VecI2 &v) C { return pixC(v.x, v.y); }
    C VecB &pixB3(Int x, Int y) C { return *(VecB *)(_data + x * SIZE(VecB) + y * _pitch); }
    C VecB &pixB3(C VecI2 &v) C { return pixB3(v.x, v.y); }
    C VecB4 &pixB4(Int x, Int y) C { return *(VecB4 *)(_data + x * SIZE(VecB4) + y * _pitch); }
    C VecB4 &pixB4(C VecI2 &v) C { return pixB4(v.x, v.y); }
    C Flt &pixF(Int x, Int y) C { return *(Flt *)(_data + x * SIZE(Flt) + y * _pitch); }
    C Flt &pixF(C VecI2 &v) C { return pixF(v.x, v.y); }
    C Vec2 &pixF2(Int x, Int y) C { return *(Vec2 *)(_data + x * SIZE(Vec2) + y * _pitch); }
    C Vec2 &pixF2(C VecI2 &v) C { return pixF2(v.x, v.y); }
    C Vec &pixF3(Int x, Int y) C { return *(Vec *)(_data + x * SIZE(Vec) + y * _pitch); }
    C Vec &pixF3(C VecI2 &v) C { return pixF3(v.x, v.y); }
    C Vec4 &pixF4(Int x, Int y) C { return *(Vec4 *)(_data + x * SIZE(Vec4) + y * _pitch); }
    C Vec4 &pixF4(C VecI2 &v) C { return pixF4(v.x, v.y); }

    // CONST 3D
    C Byte &pixB(Int x, Int y, Int z) C { return *(Byte *)(_data + x * SIZE(Byte) + y * _pitch + z * _pitch2); }
    C Byte &pixB(C VecI &v) C { return pixB(v.x, v.y, v.z); }
    C UInt &pix(Int x, Int y, Int z) C { return *(UInt *)(_data + x * SIZE(UInt) + y * _pitch + z * _pitch2); }
    C UInt &pix(C VecI &v) C { return pix(v.x, v.y, v.z); }
    C Color &pixC(Int x, Int y, Int z) C { return *(Color *)(_data + x * SIZE(Color) + y * _pitch + z * _pitch2); }
    C Color &pixC(C VecI &v) C { return pixC(v.x, v.y, v.z); }
    C VecB &pixB3(Int x, Int y, Int z) C { return *(VecB *)(_data + x * SIZE(VecB) + y * _pitch + z * _pitch2); }
    C VecB &pixB3(C VecI &v) C { return pixB3(v.x, v.y, v.z); }
    C VecB4 &pixB4(Int x, Int y, Int z) C { return *(VecB4 *)(_data + x * SIZE(VecB4) + y * _pitch + z * _pitch2); }
    C VecB4 &pixB4(C VecI &v) C { return pixB4(v.x, v.y, v.z); }
    C Flt &pixF(Int x, Int y, Int z) C { return *(Flt *)(_data + x * SIZE(Flt) + y * _pitch + z * _pitch2); }
    C Flt &pixF(C VecI &v) C { return pixF(v.x, v.y, v.z); }
    C Vec2 &pixF2(Int x, Int y, Int z) C { return *(Vec2 *)(_data + x * SIZE(Vec2) + y * _pitch + z * _pitch2); }
    C Vec2 &pixF2(C VecI &v) C { return pixF2(v.x, v.y, v.z); }
    C Vec &pixF3(Int x, Int y, Int z) C { return *(Vec *)(_data + x * SIZE(Vec) + y * _pitch + z * _pitch2); }
    C Vec &pixF3(C VecI &v) C { return pixF3(v.x, v.y, v.z); }
    C Vec4 &pixF4(Int x, Int y, Int z) C { return *(Vec4 *)(_data + x * SIZE(Vec4) + y * _pitch + z * _pitch2); }
    C Vec4 &pixF4(C VecI &v) C { return pixF4(v.x, v.y, v.z); }

    // CONST IMAGE_SOFT_CUBE only (no IMAGE_CUBE)
    C Byte &pixB(Int x, Int y, DIR_ENUM cube_face) C { return *(Byte *)(_data + x * SIZE(Byte) + y * _pitch + cube_face * _pitch2); }
    C Byte &pixB(C VecI2 &v, DIR_ENUM cube_face) C { return pixB(v.x, v.y, cube_face); }
    C UInt &pix(Int x, Int y, DIR_ENUM cube_face) C { return *(UInt *)(_data + x * SIZE(UInt) + y * _pitch + cube_face * _pitch2); }
    C UInt &pix(C VecI2 &v, DIR_ENUM cube_face) C { return pix(v.x, v.y, cube_face); }
    C Color &pixC(Int x, Int y, DIR_ENUM cube_face) C { return *(Color *)(_data + x * SIZE(Color) + y * _pitch + cube_face * _pitch2); }
    C Color &pixC(C VecI2 &v, DIR_ENUM cube_face) C { return pixC(v.x, v.y, cube_face); }
    C VecB &pixB3(Int x, Int y, DIR_ENUM cube_face) C { return *(VecB *)(_data + x * SIZE(VecB) + y * _pitch + cube_face * _pitch2); }
    C VecB &pixB3(C VecI2 &v, DIR_ENUM cube_face) C { return pixB3(v.x, v.y, cube_face); }
    C VecB4 &pixB4(Int x, Int y, DIR_ENUM cube_face) C { return *(VecB4 *)(_data + x * SIZE(VecB4) + y * _pitch + cube_face * _pitch2); }
    C VecB4 &pixB4(C VecI2 &v, DIR_ENUM cube_face) C { return pixB4(v.x, v.y, cube_face); }
    C Flt &pixF(Int x, Int y, DIR_ENUM cube_face) C { return *(Flt *)(_data + x * SIZE(Flt) + y * _pitch + cube_face * _pitch2); }
    C Flt &pixF(C VecI2 &v, DIR_ENUM cube_face) C { return pixF(v.x, v.y, cube_face); }
    C Vec2 &pixF2(Int x, Int y, DIR_ENUM cube_face) C { return *(Vec2 *)(_data + x * SIZE(Vec2) + y * _pitch + cube_face * _pitch2); }
    C Vec2 &pixF2(C VecI2 &v, DIR_ENUM cube_face) C { return pixF2(v.x, v.y, cube_face); }
    C Vec &pixF3(Int x, Int y, DIR_ENUM cube_face) C { return *(Vec *)(_data + x * SIZE(Vec) + y * _pitch + cube_face * _pitch2); }
    C Vec &pixF3(C VecI2 &v, DIR_ENUM cube_face) C { return pixF3(v.x, v.y, cube_face); }
    C Vec4 &pixF4(Int x, Int y, DIR_ENUM cube_face) C { return *(Vec4 *)(_data + x * SIZE(Vec4) + y * _pitch + cube_face * _pitch2); }
    C Vec4 &pixF4(C VecI2 &v, DIR_ENUM cube_face) C { return pixF4(v.x, v.y, cube_face); }

    // !! WARNING: 'gather' methods are written for speed and not safety, they assume that image is locked and that offsets are in range, these methods set 'pixels/colors' array from image values, coordinates are specified in the 'offset' parameters !!
    void gather(Flt *pixels, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets) C;
    void gatherL(Flt *pixels, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets) C;
    void gather(VecB *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets) C;
    void gather(Color *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets) C;
    void gather(Vec2 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets) C;
    void gather(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets) C;
    void gatherL(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets) C;
    void gatherS(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets) C;
    void gather(Flt *pixels, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets) C;
    void gather(VecB *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets) C;
    void gather(Color *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets) C;
    void gather(Vec2 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets) C;
    void gather(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets) C;
    void gatherL(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets) C;
    void gatherS(Vec4 *colors, Int *x_offset, Int x_offsets, Int *y_offset, Int y_offsets, Int *z_offset, Int z_offsets) C;

    // fit
    Rect fit(C Rect &rect, FIT_MODE fit = FIT_FULL) C { return Fit(aspect(), rect, fit); }            // get rectangle that can be used for drawing of the image to the 'rect' destination while preserving image proportions according to specified 'fit' mode
    Rect fitVertical(C Rect &rect, FIT_MODE fit = FIT_FULL) C { return Fit(invAspect(), rect, fit); } // get rectangle that can be used for drawing of the image to the 'rect' destination while preserving image proportions according to specified 'fit' mode

    // draw
    void draw(C Rect &rect) C;
    void draw(C Color &color, C Color &color_add, C Rect &rect) C;
    void draw(C Vec4 &color, C Vec4 &color_add, C Rect &rect) C;
    void drawVertical(C Rect &rect) C;                                     // draw with texture coordinates in vertical mode
    void drawVertical(C Color &color, C Color &color_add, C Rect &rect) C; // draw with texture coordinates in vertical mode

    // draw to fit best in given space, while preserving image proportions
    void drawFit(C Rect &rect, FIT_MODE fit = FIT_FULL) C { return draw(T.fit(rect, fit)); }
    void drawFit(C Color &color, C Color &color_add, C Rect &rect, FIT_MODE fit = FIT_FULL) C { return draw(color, color_add, T.fit(rect, fit)); }
    void drawFitVertical(C Color &color, C Color &color_add, C Rect &rect, FIT_MODE fit = FIT_FULL) C { return drawVertical(color, color_add, T.fitVertical(rect, fit)); }

    // draw to fullscreen
    void drawFs(FIT_MODE fit = FIT_FULL, Int filter = -1) C;                                                   // draw to fullscreen, 'filter'=custom filtering (FILTER_TYPE) for this parameter you can set any of the FILTER_TYPE enums or use -1 to let the engine decide on filtering (for example linear filtering will be used on Mobile platforms and better filtering on other platforms)
    void drawFs(C Color &color, C Color &color_add = TRANSPARENT, FIT_MODE fit = FIT_FULL, Int filter = -1) C; // draw to fullscreen, 'filter'=custom filtering (FILTER_TYPE) for this parameter you can set any of the FILTER_TYPE enums or use -1 to let the engine decide on filtering (for example linear filtering will be used on Mobile platforms and better filtering on other platforms)

    // draw only part of the image
    void drawPart(C Rect &screen_rect, C Rect &tex_rect) C;
    void drawPart(C Color &color, C Color &color_add, C Rect &screen_rect, C Rect &tex_rect) C;
    void drawPart(C Vec4 &color, C Vec4 &color_add, C Rect &screen_rect, C Rect &tex_rect) C;
    void drawPartVertical(C Rect &screen_rect, C Rect &tex_rect) C;                                     // draw with texture coordinates in vertical mode
    void drawPartVertical(C Color &color, C Color &color_add, C Rect &screen_rect, C Rect &tex_rect) C; // draw with texture coordinates in vertical mode

    // draw rotated
    void drawRotate(C Vec2 &center, C Vec2 &size, Flt angle, C Vec2 *rotation_center_uv = null) C;                                     // 'rotation_center_uv'=optional rotation center in range 0..1
    void drawRotate(C Color &color, C Color &color_add, C Vec2 &center, C Vec2 &size, Flt angle, C Vec2 *rotation_center_uv = null) C; // 'rotation_center_uv'=optional rotation center in range 0..1

    // draw masked
    void drawMask(C Color &color, C Color &color_add, C Rect &rect, C Image &mask, C Rect &mask_rect) C;          // draw image using 'mask', final result is calculated as: this*           mask   *color+color_add
    void drawMaskNoFilter(C Color &color, C Color &color_add, C Rect &rect, C Image &mask, C Rect &mask_rect) C;  // draw image using 'mask', final result is calculated as: this*           mask   *color+color_add, uses FILTER_NONE for this, but FILTER_LINEAR for 'mask'
    void drawMaskA(C Color &color, C Color &color_add, C Rect &rect, C Image &mask, C Rect &mask_rect) C;         // draw image using 'mask', final result is calculated as: this*Vec4(1,1,1,mask.a)*color+color_add
    void drawMaskANoFilter(C Color &color, C Color &color_add, C Rect &rect, C Image &mask, C Rect &mask_rect) C; // draw image using 'mask', final result is calculated as: this*Vec4(1,1,1,mask.a)*color+color_add, uses FILTER_NONE for this, but FILTER_LINEAR for 'mask'

    // draw faded
    void drawFadeLR(C Color &color, C Rect &rect, Flt trans_l, Flt opaque_l, Flt opaque_r, Flt trans_r) C; // draw and fade out left/right sides, 'trans_l'=X screen coordinate where left part is fully transparent, 'opaque_l'=X screen coordinate where left part is fully opaque, 'opaque_r'=X screen coordinate where right part is fully opaque, 'trans_r'=X screen coordinate where right part is fully transparent

    // draw image as tiled background, 'tex_scale'=texture coordinates scaling
    void drawTile(C Rect &rect, Flt tex_scale = 1) C;
    void drawTile(C Color &color, C Color &color_add, C Rect &rect, Flt tex_scale = 1) C;

    void drawTileRotate(C Color &color, C Color &color_add, C Vec2 &center, C Vec2 &size, Flt tex_scale, Flt angle, C Vec2 *rotation_center_uv) C;

    // draw image as rectangle's border
    void drawBorder(C Rect &rect, Flt border = 0.02f, Flt tex_scale = 1, Flt tex_offset = 0, Bool wrap_mode = false) C;
    void drawBorder(C Color &color, C Color &color_add, C Rect &rect, Flt border = 0.02f, Flt tex_scale = 1, Flt tex_offset = 0, Bool wrap_mode = false) C;

    // draw stretched image from 3x3 parts
    void draw3x3(C Color &color, C Color &color_add, C Rect &rect, Flt border_size, Flt tex_frac = 0.25f) C;         // 'color'=color that will be multiplied by the texture, 'color_add'=color that will be added to the texture using following formula "final_color = texture_color * color + color_add", 'rect'=screen rectangle at which the image will be drawn, 'border_size'=size of the border inside the screen 'rect' rectangle that will be drawn as image borders (this value should be less than rectangle dimensions), 'tex_frac'=fraction of the image texture that will be used for drawing the borders (this value should be in range of 0 .. 0.5)
    void draw3x3Vertical(C Color &color, C Color &color_add, C Rect &rect, Flt border_size, Flt tex_frac = 0.25f) C; // 'color'=color that will be multiplied by the texture, 'color_add'=color that will be added to the texture using following formula "final_color = texture_color * color + color_add", 'rect'=screen rectangle at which the image will be drawn, 'border_size'=size of the border inside the screen 'rect' rectangle that will be drawn as image borders (this value should be less than rectangle dimensions), 'tex_frac'=fraction of the image texture that will be used for drawing the borders (this value should be in range of 0 .. 0.5), this function will draw with texture coordinates in vertical mode

    // draw with custom filtering
    void drawFilter(C Rect &rect, FILTER_TYPE filter = FILTER_BEST) C;                                     // this method will draw the image with custom filtering which allows to achieve better quality than linear filtering, the pixel shader for better filters is very expensive, therefore use it only if performance is not critical and just for few images (for example displaying one image on the screen), this method supports only FILTER_NONE, FILTER_LINEAR, FILTER_CUBIC_FAST, FILTER_CUBIC_PLUS, FILTER_EASU
    void drawFilter(C Color &color, C Color &color_add, C Rect &rect, FILTER_TYPE filter = FILTER_BEST) C; // this method will draw the image with custom filtering which allows to achieve better quality than linear filtering, the pixel shader for better filters is very expensive, therefore use it only if performance is not critical and just for few images (for example displaying one image on the screen), this method supports only FILTER_NONE, FILTER_LINEAR, FILTER_CUBIC_FAST, FILTER_CUBIC_PLUS, FILTER_EASU

    // draw cube face
    void drawCubeFace(C Color &color, C Color &color_add, C Rect &rect, DIR_ENUM face) C;

    // draw in 3D space
    void draw3D(C Color &color, Flt size, Flt angle, C Vec &pos) C; // draw as 3D billboard, this can be called in RM_BLEND mode or outside rendering function (in drawing function), this method supports only IMAGE_2D images, this relies on active object matrix which can be set using 'SetMatrix' function

    void drawVolume(C Color &color, C Color &color_add, C OBox &obox, Flt voxel_density_factor = 0.01f, Flt precision = 1.0f, Int min_steps = 2, Int max_steps = 64) C; // draw as 3D volumetric image, 'voxel_density_factor'=density factor of a single voxel (0..1), 'precision'=number of steps per voxel (0..Inf), 'min_steps max_steps'=minimum and maximum number of steps (2..1024), this method can be called in RM_BLEND rendering mode, this method supports only IMAGE_3D images

    // io
    void operator=(C Str &name); // load, Exit  on fail
    void operator=(C UID &id);   // load, Exit  on fail
#if EE_PRIVATE
    Bool saveData(File &f) C;                                                                    // save, false on fail
    Bool loadData(File &f, ImageHeader *header = null, C Str &name = S, Bool can_del_f = false); // load, false on fail, 'can_del_f'=if allow deleting 'f' (always enable if possible, to allow image streaming)
    Bool _loadData(File &f, ImageHeader *header = null, C Str &name = S);                        // load, false on fail - Deprecated do not use !!
#endif
    Bool save(C Str &name) C; // save, false on fail
    Bool load(C Str &name);   // load, false on fail
    Bool save(File &f) C;     // save, false on fail
    Bool load(File &f);       // load, false on fail

#if EE_PRIVATE
    Bool ImportTGA(C Str &name, Int type, Int mode = -1, Int mip_maps = -1);
    Bool ImportTGA(File &f, Int type, Int mode = -1, Int mip_maps = -1);
    Bool ImportDDS(C Str &name, Int type, Int mode = -1, Int mip_maps = -1);
    Bool ImportDDS(File &f, Int type, Int mode = -1, Int mip_maps = -1);
    Bool ImportBMPRaw(File &f, Bool ico = false);
    Bool ExportBMPRaw(File &f, Byte byte_pp, Bool ico = false) C;
#endif

    Bool Import(C Str &name, Int type = -1, Int mode = -1, Int mip_maps = -1);       // import BMP PNG JPG JXL WEBP AVIF HEIF TGA TIF DDS PSD ICO HDR, 'type'=IMAGE_TYPE, 'mode'=IMAGE_MODE, 'mip_maps'=number of mip-maps (0=autodetect), -1=keep original value, false on fail
    Image &mustImport(C Str &name, Int type = -1, Int mode = -1, Int mip_maps = -1); // import BMP PNG JPG JXL WEBP AVIF HEIF TGA TIF DDS PSD ICO HDR, 'type'=IMAGE_TYPE, 'mode'=IMAGE_MODE, 'mip_maps'=number of mip-maps (0=autodetect), -1=keep original value, Exit  on fail
    Bool Import(File &f, Int type = -1, Int mode = -1, Int mip_maps = -1);           // import BMP PNG JPG JXL WEBP AVIF HEIF TGA TIF DDS PSD ICO HDR, 'type'=IMAGE_TYPE, 'mode'=IMAGE_MODE, 'mip_maps'=number of mip-maps (0=autodetect), -1=keep original value, false on fail
    Image &mustImport(File &f, Int type = -1, Int mode = -1, Int mip_maps = -1);     // import BMP PNG JPG JXL WEBP AVIF HEIF TGA TIF DDS PSD ICO HDR, 'type'=IMAGE_TYPE, 'mode'=IMAGE_MODE, 'mip_maps'=number of mip-maps (0=autodetect), -1=keep original value, Exit  on fail

    Bool Export(C Str &name, Flt rgb_quality = -1, Flt alpha_quality = -1, Flt compression_level = -1, Int sub_sample = -1) C; // export according to extension, false on fail, 'rgb_quality'=color quality 0..1 (-1=default, 0=smallest size, 1=best quality), 'alpha_quality'=alpha quality 0..1 (-1=use 'rgb_quality', 0=smallest size, 1=best quality), 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size), 'sub_sample'=0..2 (chroma sub-sampling for RGB images, 0=none, 1=half, 2=quarter, -1=default)

    Bool ImportCube(C Image &right, C Image &left, C Image &up, C Image &down, C Image &forward, C Image &back, Int type = -1, Bool soft = false, Int mip_maps = 1, Bool resize_to_pow2 = true, FILTER_TYPE filter = FILTER_BEST); // import                                                        as cube texture, 'type'=IMAGE_TYPE (-1=keep original value), 'soft'=if use IMAGE_SOFT_CUBE or IMAGE_CUBE, false on fail
    Bool ImportCube(C Str &right, C Str &left, C Str &up, C Str &down, C Str &forward, C Str &back, Int type = -1, Bool soft = false, Int mip_maps = 1, Bool resize_to_pow2 = true, FILTER_TYPE filter = FILTER_BEST);             // import BMP PNG JPG JXL WEBP AVIF HEIF TGA TIF DDS PSD ICO HDR as cube texture, 'type'=IMAGE_TYPE (-1=keep original value), 'soft'=if use IMAGE_SOFT_CUBE or IMAGE_CUBE, false on fail
    Image &mustImportCube(C Str &right, C Str &left, C Str &up, C Str &down, C Str &forward, C Str &back, Int type = -1, Bool soft = false, Int mip_maps = 1, Bool resize_to_pow2 = true, FILTER_TYPE filter = FILTER_BEST);       // import BMP PNG JPG JXL WEBP AVIF HEIF TGA TIF DDS PSD ICO HDR as cube texture, 'type'=IMAGE_TYPE (-1=keep original value), 'soft'=if use IMAGE_SOFT_CUBE or IMAGE_CUBE, Exit  on fail

    Bool ImportBMP(C Str &name);                                          // import    BMP  from file, false on fail
    Bool ImportBMP(File &f);                                              // import    BMP  from file, false on fail
    Bool ExportBMP(C Str &name) C;                                        // export as BMP  to   file, false on fail
    Bool ExportBMP(File &f) C;                                            // export as BMP  to   file, false on fail
    Bool ImportPNG(C Str &name);                                          // import    PNG  from file, false on fail
    Bool ImportPNG(File &f);                                              // import    PNG  from file, false on fail
    Bool ExportPNG(C Str &name, Flt compression_level = -1) C;            // export as PNG  to   file, false on fail, 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size)
    Bool ExportPNG(File &f, Flt compression_level = -1) C;                // export as PNG  to   file, false on fail, 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size)
    Bool ImportJPG(C Str &name);                                          // import    JPG  from file, false on fail
    Bool ImportJPG(File &f);                                              // import    JPG  from file, false on fail
    Bool ExportJPG(C Str &name, Flt quality = -1, Int sub_sample = -1) C; // export as JPG  to   file, false on fail, 'quality'=0..1 (-1=default, 0=smallest size, 1=best quality), 'sub_sample'=0..2 (chroma sub-sampling for RGB images, 0=none, 1=half, 2=quarter, -1=default)
    Bool ExportJPG(File &f, Flt quality = -1, Int sub_sample = -1) C;     // export as JPG  to   file, false on fail, 'quality'=0..1 (-1=default, 0=smallest size, 1=best quality), 'sub_sample'=0..2 (chroma sub-sampling for RGB images, 0=none, 1=half, 2=quarter, -1=default)

    // supported only if 'SupportImportJXL' was called in 'InitPre'
    Bool ImportJXL(C Str &name); // import    JXL  from file, false on fail
    Bool ImportJXL(File &f);     // import    JXL  from file, false on fail

    // supported only if 'SupportExportJXL' was called in 'InitPre'
    Bool ExportJXL(C Str &name, Flt quality = -1, Flt compression_level = -1) C; // export as JXL  to   file, false on fail, 'quality'=0..1 (-1=default, 0=smallest size, 1=best quality), 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size)
    Bool ExportJXL(File &f, Flt quality = -1, Flt compression_level = -1) C;     // export as JXL  to   file, false on fail, 'quality'=0..1 (-1=default, 0=smallest size, 1=best quality), 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size)

    Bool ImportWEBP(C Str &name);                                                 // import    WEBP from file, false on fail
    Bool ImportWEBP(File &f);                                                     // import    WEBP from file, false on fail
    Bool ExportWEBP(C Str &name, Flt rgb_quality = -1, Flt alpha_quality = -1) C; // export as WEBP to   file, false on fail, 'rgb_quality'=color quality 0..1 (-1=default, 0=smallest size, 1=lossless), 'alpha_quality'=alpha quality 0..1 (-1=use 'rgb_quality', 0=smallest size, 1=lossless)
    Bool ExportWEBP(File &f, Flt rgb_quality = -1, Flt alpha_quality = -1) C;     // export as WEBP to   file, false on fail, 'rgb_quality'=color quality 0..1 (-1=default, 0=smallest size, 1=lossless), 'alpha_quality'=alpha quality 0..1 (-1=use 'rgb_quality', 0=smallest size, 1=lossless)

    // supported only if 'SupportImportAVIF' was called in 'InitPre'
    Bool ImportAVIF(C Str &name); // import    AVIF from file, false on fail
    Bool ImportAVIF(File &f);     // import    AVIF from file, false on fail

    // supported only if 'SupportExportAVIF' was called in 'InitPre'
    Bool ExportAVIF(C Str &name, Flt rgb_quality = -1, Flt alpha_quality = -1, Flt compression_level = -1) C; // export as AVIF to   file, false on fail, 'rgb_quality'=color quality 0..1 (-1=default, 0=smallest size, 1=lossless), 'alpha_quality'=alpha quality 0..1 (-1=use 'rgb_quality', 0=smallest size, 1=lossless), 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size)
    Bool ExportAVIF(File &f, Flt rgb_quality = -1, Flt alpha_quality = -1, Flt compression_level = -1) C;     // export as AVIF to   file, false on fail, 'rgb_quality'=color quality 0..1 (-1=default, 0=smallest size, 1=lossless), 'alpha_quality'=alpha quality 0..1 (-1=use 'rgb_quality', 0=smallest size, 1=lossless), 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size)

    Bool ImportHEIF(C Str &name);                              // import    HEIF from file, false on fail
    Bool ImportHEIF(File &f);                                  // import    HEIF from file, false on fail
    Bool ExportHEIF(C Str &name, Flt quality = -1) C;          // export as HEIF to   file, false on fail, 'quality'=0..1 (-1=default, 0=smallest size, 1=lossless)
    Bool ExportHEIF(File &f, Flt quality = -1) C;              // export as HEIF to   file, false on fail, 'quality'=0..1 (-1=default, 0=smallest size, 1=lossless)
    Bool ImportTGA(C Str &name);                               // import    TGA  from file, false on fail
    Bool ImportTGA(File &f);                                   // import    TGA  from file, false on fail
    Bool ExportTGA(C Str &name) C;                             // export as TGA  to   file, false on fail
    Bool ExportTGA(File &f) C;                                 // export as TGA  to   file, false on fail
    Bool ImportTIF(C Str &name);                               // import    TIF  from file, false on fail
    Bool ImportTIF(File &f);                                   // import    TIF  from file, false on fail
    Bool ExportTIF(C Str &name, Flt compression_level = -1) C; // export as TIF  to   file, false on fail, 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size)
    Bool ExportTIF(File &f, Flt compression_level = -1) C;     // export as TIF  to   file, false on fail, 'compression_level'=0..1 (-1=default, 0=fast/biggest size, 1=slow/smallest size)
    Bool ImportDDS(C Str &name);                               // import    DDS  from file, false on fail
    Bool ImportDDS(File &f);                                   // import    DDS  from file, false on fail
    Bool ExportDDS(C Str &name) C;                             // export as DDS  to   file, false on fail
    Bool ImportPSD(C Str &name);                               // import    PSD  from file, false on fail
    Bool ImportPSD(File &f);                                   // import    PSD  from file, false on fail
    Bool ImportICO(C Str &name);                               // import    ICO  from file, false on fail
    Bool ImportICO(File &f);                                   // import    ICO  from file, false on fail
    Bool ExportICO(C Str &name) C;                             // export as ICO  to   file, false on fail
    Bool ExportICO(File &f) C;                                 // export as ICO  to   file, false on fail
    Bool ExportICNS(C Str &name) C;                            // export as ICNS to   file, false on fail
    Bool ExportICNS(File &f) C;                                // export as ICNS to   file, false on fail
    Bool ImportHDR(C Str &name);                               // import    HDR  from file, false on fail
    Bool ImportHDR(File &f);                                   // import    HDR  from file, false on fail

    Image &operator=(C Image &src); // create from 'src' image using 'copy' method, Exit on fail
    ~Image();
    Image();
    Image(C Image &src);                                                                     // create from 'src' image using 'copy' method, Exit on fail
    explicit Image(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps = 0); // create with specified parameters using 'create' method, Exit on fail, 'mip_maps'=number of mip-maps (0=autodetect)

#if EE_PRIVATE
    void zero() { Zero(T); }
    Bool setSRV();
    Bool finalize();
    Bool setInfo();
    void forceInfo(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int samples);
    void adjustInfo(Int w, Int h, Int d, IMAGE_TYPE type);
    void adjustType(IMAGE_TYPE type);
    void setPartial();
    void setGLParams();

    Bool copySoft(Image &dest, FILTER_TYPE filter = FILTER_BEST, UInt flags = IC_CLAMP, Int max_mip_maps = INT_MAX, Flt sharp_smooth = 1.0f) C; // software             copy, 'flags'=IMAGE_COPY_FLAG, 'sharp_smooth'=factor affecting sharpness/smoothness (0..Inf, the closer to 0.0 then the sharper result, the bigger than 1.0 then the more blurry result, default=1.0)
    void copyMs(ImageRT &dest, Bool restore_rt, Bool multi_sample, C RectI *rect = null) C;                                                     // multi sample texture copy, 'multi_sample'=average samples when writing to single sample, or copy texture on a per sample basis, this is needed because multi-sampled textures can't be sampled smoothly in the shader, this assumes that both source and dest are of the same size
    void copyMs(ImageRT &dest, Bool restore_rt, Bool multi_sample, C Rect &rect) C;                                                             // multi sample texture copy, 'multi_sample'=average samples when writing to single sample, or copy texture on a per sample basis, this is needed because multi-sampled textures can't be sampled smoothly in the shader, this assumes that both source and dest are of the same size
    void copyHw(ImageRT &dest, Bool restore_rt, C RectI *rect_src = null, C RectI *rect_dest = null) C;                                         // hardware     texture copy
    void copyHw(ImageRT &dest, Bool restore_rt, C Rect &rect) C;                                                                                // hardware     texture copy
    void copyDepth(ImageRT &dest, Bool restore_rt, C RectI *rect_src = null, C RectI *rect_dest = null) C;                                      //        depth texture copy
    void copyDepth(ImageRT &dest, Bool restore_rt, C Rect &rect) C;                                                                             //        depth texture copy

    Bool capture(C ImageRT &src);
#endif

#if !EE_PRIVATE
  private:
#endif
    IMAGE_TYPE _type, _hw_type;
    IMAGE_MODE _mode;
    LOCK_MODE _lock_mode;
    DIR_ENUM _lock_face;
    Byte _mips, _samples, _lock_mip, _byte_pp, _base_mip, _stream;
    Bool _partial, _discard;
    Int _lock_count;
    UInt _pitch, _pitch2;
    VecI _size, _hw_size, _lock_size;
    Vec _part;
    Byte *_data, *_data_all;
    union {
        struct
        {
#if EE_PRIVATE && DX11
            ID3D11Resource *_txtr;
            ID3D11ShaderResourceView *_srv;
#else
            Ptr _ptr[2];
#endif
        };
        struct
        {
#if EE_PRIVATE && GL
            UInt _txtr, _rb;
#else
            UInt _uint[2];
#endif
        };
    };
};
/******************************************************************************/
DECLARE_CACHE(Image, Images, ImagePtr); // 'Images' cache storing 'Image' objects which can be accessed by 'ImagePtr' pointer
extern const ImagePtr ImageNull;
/******************************************************************************/
struct ImageHeader {
    VecI size;
    Int mip_maps;
    IMAGE_TYPE type;
    IMAGE_MODE mode;

    Bool is() C;
    void zero() {
        size.zero();
        mip_maps = 0;
        type = IMAGE_NONE;
        mode = IMAGE_2D;
    }
    ImageHeader() { zero(); }
};
Bool ImageLoadHeader(File &f, ImageHeader &header);     // load image header from file, this method is faster than 'Image.load' if you're not interested in the Image data, but only in information about it. After reading the header, 'f' file position is reset to the same place before making this call, false on fail
Bool ImageLoadHeader(C Str &name, ImageHeader &header); // load image header from file, this method is faster than 'Image.load' if you're not interested in the Image data, but only in information about it.                                                                                                 false on fail
/******************************************************************************/
struct ImageCompare {
    Bool skipped; // if comparison was interrupted due to 'skip_dif'
    Flt max_dif,  // max     difference between colors, 0..1
        avg_dif,  // average difference between colors, 0..1
        avg_dif2, // average difference between colors, 0..1 (using square method)
        similar,  // fraction of the image that is similar (difference is below 'similar_dif'), 0..1
        psnr;     // peak signal to noise ratio, 0..Inf

    Bool compare(C Image &a, C Image &b, Flt similar_dif = 0.01f, Bool alpha_weight = false, Int a_mip = 0, Flt skip_dif = 1.0f); // compare 2 images, 'similar_dif'=limit used to determine that colors are similar, 'alpha_weight'=if use alpha channel as the weight, 'a_mip'=mip map of 'a' image used for comparison (based on this value, mip map of 'b' image will be selected to match the size of selected 'a' mip map), 'skip_dif'=if a single color difference exceeds this limit then skip remaining processing and return as soon as possible (value >=1 disables skipping), this function returns true on success and false on fail (if couldn't find matching mip map size between images, or if failed to lock the images)
};
/******************************************************************************/
struct ImageThreadsClass : Threads {
    ImageThreadsClass &init(); // initialize, call this at least once before any processing

    void process(Int elms, void func(IntPtr elm_index, Ptr user, Int thread_index), Ptr user) { process1(elms, func, user, INT_MAX); } // use all available threads, including this one
    T1(USER_DATA)
    void process(Int elms, void func(IntPtr elm_index, USER_DATA &user, Int thread_index), USER_DATA &user) { process1(elms, func, user, INT_MAX); } // use all available threads, including this one
} extern ImageThreads;
/******************************************************************************/
IMAGE_TYPE BytesToImageType(Int byte_pp); // get IMAGE_TYPE needed to store 'byte_pp' amount of bytes per pixel, which is 1->IMAGE_I8, 2->IMAGE_I16, 3->IMAGE_I24, 4->IMAGE_I32, other->IMAGE_NONE

IMAGE_TYPE ImageTypeIncludeRGB(IMAGE_TYPE type); // convert 'type' to the most similar IMAGE_TYPE that has RGB channels

IMAGE_TYPE ImageTypeIncludeAlpha(IMAGE_TYPE type); // convert 'type' to the most similar IMAGE_TYPE that has    alpha channel
IMAGE_TYPE ImageTypeExcludeAlpha(IMAGE_TYPE type); // convert 'type' to the most similar IMAGE_TYPE that has no alpha channel

IMAGE_TYPE ImageTypeToggleSRGB(IMAGE_TYPE type);  // convert 'type' to the most similar IMAGE_TYPE that has different sRGB
IMAGE_TYPE ImageTypeIncludeSRGB(IMAGE_TYPE type); // convert 'type' to the most similar IMAGE_TYPE that has           sRGB
IMAGE_TYPE ImageTypeExcludeSRGB(IMAGE_TYPE type); // convert 'type' to the most similar IMAGE_TYPE that has no        sRGB

IMAGE_TYPE ImageTypeHighPrec(IMAGE_TYPE type); // convert 'type' to the most similar IMAGE_TYPE that has F32 precision per channel

IMAGE_TYPE ImageTypeUncompressed(IMAGE_TYPE type); // convert 'type' to the most similar IMAGE_TYPE that is not compressed

IMAGE_TYPE ImageTypeForMode(IMAGE_TYPE type, IMAGE_MODE mode); // convert 'type' to the most similar IMAGE_TYPE that can be used for 'mode'
/******************************************************************************/
#if EE_PRIVATE
#define MAX_MIP_MAPS 32
INLINE Bool CheckMipNum(Int mips) { return InRange(mips, MAX_MIP_MAPS + 1); } // +1 because this checks number of elements

// pointers to class method
typedef Flt (Image::*PtrImagePixel)(Flt x, Flt y, Bool clamp) C;
typedef Flt (Image::*PtrImagePixel3D)(Flt x, Flt y, Flt z, Bool clamp) C;
typedef Vec4 (Image::*PtrImageColor)(Flt x, Flt y, Bool clamp, Bool alpha_weight) C;
typedef Vec4 (Image::*PtrImageColor3D)(Flt x, Flt y, Flt z, Bool clamp, Bool alpha_weight) C;
typedef Vec4 (Image::*PtrImageAreaColor)(C Vec2 &pos, C Vec2 &size, Bool clamp, Bool alpha_weight) C;

PtrImagePixel GetImagePixelF(FILTER_TYPE filter);
PtrImagePixel3D GetImagePixel3DF(FILTER_TYPE filter);
PtrImageColor GetImageColorF(FILTER_TYPE filter);
PtrImageColor3D GetImageColor3DF(FILTER_TYPE filter);
PtrImageAreaColor GetImageAreaColor(FILTER_TYPE filter, Bool &linear_gamma);

IMAGE_MODE AsSoft(IMAGE_MODE mode);
Int ImageFaces(IMAGE_MODE mode);
Int PaddedWidth(Int w, Int h, Int mip, IMAGE_TYPE type);
Int PaddedHeight(Int w, Int h, Int mip, IMAGE_TYPE type);
UInt ImagePitch(Int w, Int h, Int mip, IMAGE_TYPE type);
UInt ImageBlocksY(Int w, Int h, Int mip, IMAGE_TYPE type);
UInt ImagePitch2(Int w, Int h, Int mip, IMAGE_TYPE type);
ULong ImagePitch2L(Int w, Int h, Int mip, IMAGE_TYPE type);
UInt ImageFaceSize(Int w, Int h, Int d, Int mip, IMAGE_TYPE type);
ULong ImageFaceSizeL(Int w, Int h, Int d, Int mip, IMAGE_TYPE type);
UInt ImageMipOffset(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps, Int mip_map);
UInt ImageSize(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps);
ULong ImageSizeL(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps);
GPU_API(DXGI_FORMAT, UInt)
ImageTypeToFormat(Int type);                                     // convert from IMAGE_TYPE to API_FORMAT
IMAGE_TYPE ImageFormatToType(GPU_API(DXGI_FORMAT, UInt) format); // convert from API_FORMAT to IMAGE_TYPE
Int TotalMipMaps(Int w, Int h);
Int TotalMipMaps(Int w, Int h, Int d);

IMAGE_TYPE ImageTypeOnFail(IMAGE_TYPE type);
Bool ImageSupported(IMAGE_TYPE type, IMAGE_MODE mode, Byte samples = 1);
Bool IgnoreGamma(UInt flags, IMAGE_TYPE src, IMAGE_TYPE dest);
Bool CanDoRawCopy(IMAGE_TYPE src, IMAGE_TYPE dest, Bool ignore_gamma = false);
Bool CanDoRawCopy(C Image &src, C Image &dest, Bool ignore_gamma = false);
Bool CanCompress(IMAGE_TYPE dest);
Bool NeedMultiChannel(IMAGE_TYPE src, IMAGE_TYPE dest);
Bool CompatibleLock(LOCK_MODE cur, LOCK_MODE lock); // if 'lock' is okay to be applied when 'cur' is already applied
Flt ImagePixelF(CPtr data, IMAGE_TYPE hw_type);
Flt ImagePixelL(CPtr data, IMAGE_TYPE hw_type);
Vec4 ImageColorF(CPtr data, IMAGE_TYPE hw_type);
Vec4 ImageColorL(CPtr data, IMAGE_TYPE hw_type);
void CopyNoStretch(C Image &src, Image &dest, Bool clamp, Bool ignore_gamma = false); // assumes 'src,dest' are locked and non-compressed

void _CopyImgData(C Byte *&src_data, Byte *&dest_data, Int src_pitch, Int dest_pitch, Int src_blocks_y, Int dest_blocks_y);
void _CopyImgData(C Byte *&src_data, Byte *&dest_data, Int src_pitch, Int dest_pitch, Int src_blocks_y, Int dest_blocks_y, Int src_pitch2, Int dest_pitch2, Int src_d, Int dest_d);
void _CopyImgData(C Byte *&src_data, Byte *&dest_data, Int src_pitch, Int dest_pitch, Int src_blocks_y, Int dest_blocks_y, Int src_pitch2, Int dest_pitch2, Int src_d, Int dest_d, Int faces);

inline void CopyImgData(C Byte *src_data, Byte *dest_data, Int src_pitch, Int dest_pitch, Int src_blocks_y, Int dest_blocks_y) { _CopyImgData(src_data, dest_data, src_pitch, dest_pitch, src_blocks_y, dest_blocks_y); }
inline void CopyImgData(C Byte *src_data, Byte *dest_data, Int src_pitch, Int dest_pitch, Int src_blocks_y, Int dest_blocks_y, Int src_pitch2, Int dest_pitch2, Int src_d, Int dest_d) { _CopyImgData(src_data, dest_data, src_pitch, dest_pitch, src_blocks_y, dest_blocks_y, src_pitch2, dest_pitch2, src_d, dest_d); }
inline void CopyImgData(C Byte *src_data, Byte *dest_data, Int src_pitch, Int dest_pitch, Int src_blocks_y, Int dest_blocks_y, Int src_pitch2, Int dest_pitch2, Int src_d, Int dest_d, Int faces) { _CopyImgData(src_data, dest_data, src_pitch, dest_pitch, src_blocks_y, dest_blocks_y, src_pitch2, dest_pitch2, src_d, dest_d, faces); }

#if WINDOWS
HICON CreateIcon(C Image &image, C VecI2 *cursor_hot_spot = null); // 'cursor_hot_spot'=if this is specified then the icon is treated as a mouse cursor with given hot spot, otherwise it's a normal icon
#endif
#if GL
UInt SourceGLFormat(IMAGE_TYPE type);
UInt SourceGLType(IMAGE_TYPE type);
#endif

#define IMAGE_STREAM_FULL true // load only small image, but stream the entire full image and replace the small with the full one, much faster on both DX and GL !! WARNING: IN THIS MODE THE IMAGES ARE CREATED SMALLER, BUT THEIR MEMBERS: SIZE/MIPS ARE SET AS IF FULL, WHICH DOES NOT MATCH THE ACTUAL IMAGE DATA !!
enum {
    IMAGE_STREAM_LOADING = 1 << 0,   // if still loading - in streaming queue
    IMAGE_STREAM_NEED_MORE = 1 << 1, // if still need more data to load (this can be enabled even if Image is not IMAGE_STREAM_LOADING anymore, for example due to an error, but there's still data left unstreamed)
};
void LockedUpdateStreamLoads(); // assumes 'D._lock'
void UpdateStreamLoads();
void ShutStreamLoads();
#endif
/******************************************************************************/
