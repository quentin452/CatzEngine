/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#include "Import/BC.h"
#include "Import/ETC.h"
Bool DecompressPVRTC(C Image &src, Image &dest, Int max_mip_maps=INT_MAX);
Bool DecompressASTC (C Image &src, Image &dest, Int max_mip_maps=INT_MAX);

#if DX11
   // make sure that there's direct mapping for cube face
   ASSERT(D3D11_TEXTURECUBE_FACE_POSITIVE_X==DIR_RIGHT
       && D3D11_TEXTURECUBE_FACE_NEGATIVE_X==DIR_LEFT
       && D3D11_TEXTURECUBE_FACE_POSITIVE_Y==DIR_UP
       && D3D11_TEXTURECUBE_FACE_NEGATIVE_Y==DIR_DOWN
       && D3D11_TEXTURECUBE_FACE_POSITIVE_Z==DIR_FORWARD
       && D3D11_TEXTURECUBE_FACE_NEGATIVE_Z==DIR_BACK);
#elif GL
   // make sure that there's direct mapping for cube face
   ASSERT(GL_TEXTURE_CUBE_MAP_POSITIVE_X-GL_TEXTURE_CUBE_MAP_POSITIVE_X==DIR_RIGHT
       && GL_TEXTURE_CUBE_MAP_NEGATIVE_X-GL_TEXTURE_CUBE_MAP_POSITIVE_X==DIR_LEFT
       && GL_TEXTURE_CUBE_MAP_POSITIVE_Y-GL_TEXTURE_CUBE_MAP_POSITIVE_X==DIR_UP
       && GL_TEXTURE_CUBE_MAP_NEGATIVE_Y-GL_TEXTURE_CUBE_MAP_POSITIVE_X==DIR_DOWN
       && GL_TEXTURE_CUBE_MAP_POSITIVE_Z-GL_TEXTURE_CUBE_MAP_POSITIVE_X==DIR_FORWARD
       && GL_TEXTURE_CUBE_MAP_NEGATIVE_Z-GL_TEXTURE_CUBE_MAP_POSITIVE_X==DIR_BACK);
#endif

#define GL_SWIZZLE (GL && !GL_ES) // Modern Desktop OpenGL (3.2) does not support GL_ALPHA8, GL_LUMINANCE8, GL_LUMINANCE8_ALPHA8, use swizzle instead
/******************************************************************************/
DEFINE_CACHE(Image, Images, ImagePtr, "Image");
 const ImagePtr ImageNull;
static SyncLock ImageSoftLock; // it's important to use a separate lock from 'D._lock' so we don't need to wait for GPU to finish drawing
/******************************************************************************/
ImageTypeInfo ImageTI[IMAGE_ALL_TYPES]= // !! in case multiple types have the same format, preferred version must be specified in 'ImageFormatToType' !!
{ // Name            , cmprs,HiPrec,byte,bit,   R, G, B, A,Dept,S,Chn, Bx,By,Bs,       Precision   ,Usage,           Format
   {"None"           , false, false,   0,  0,   0, 0, 0, 0,   0,0, 0,   0, 0, 0, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, 0)},

   {"R8G8B8A8"       , false, false,   4, 32,   8, 8, 8, 8,   0,0, 4,   1, 1, 4, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_R8G8B8A8_UNORM     , GL_RGBA8       )},
   {"R8G8B8A8_SRGB"  , false, false,   4, 32,   8, 8, 8, 8,   0,0, 4,   1, 1, 4, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, GL_SRGB8_ALPHA8)},
   {"R8G8B8A8_SIGN"  , false, true ,   4, 32,   8, 8, 8, 8,   0,0, 4,   1, 1, 4, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_R8G8B8A8_SNORM     , GL_RGBA8_SNORM )},
   {"R8G8B8"         , false, false,   3, 24,   8, 8, 8, 0,   0,0, 3,   1, 1, 3, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN            , GL_RGB8        )},
   {"R8G8B8_SRGB"    , false, false,   3, 24,   8, 8, 8, 0,   0,0, 3,   1, 1, 3, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN            , GL_SRGB8       )},
   {"R8G8"           , false, false,   2, 16,   8, 8, 0, 0,   0,0, 2,   1, 1, 2, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_R8G8_UNORM         , GL_RG8         )},
   {"R8G8_SIGN"      , false, true ,   2, 16,   8, 8, 0, 0,   0,0, 2,   1, 1, 2, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_R8G8_SNORM         , GL_RG8_SNORM   )},
   {"R8"             , false, false,   1,  8,   8, 0, 0, 0,   0,0, 1,   1, 1, 1, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_R8_UNORM           , GL_R8          )},
   {"R8_SIGN"        , false, true ,   1,  8,   8, 0, 0, 0,   0,0, 1,   1, 1, 1, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_R8_SNORM           , GL_R8_SNORM    )},

   {"R10G10B10A2"    , false, true ,   4, 32,  10,10,10, 2,   0,0, 4,   1, 1, 4, IMAGE_PRECISION_10, 0, GPU_API(DXGI_FORMAT_R10G10B10A2_UNORM, GL_RGB10_A2)},

   {"A8"             , false, false,   1,  8,   0, 0, 0, 8,   0,0, 1,   1, 1, 1, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_A8_UNORM, GL_SWIZZLE ? GL_R8  : GL_ALPHA8           )},
   {"L8"             , false, false,   1,  8,   8, 8, 8, 0,   0,0, 1,   1, 1, 1, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN , GL_SWIZZLE ? GL_R8  : GL_LUMINANCE8       )},
   {"L8_SRGB"        , false, false,   1,  8,   8, 8, 8, 0,   0,0, 1,   1, 1, 1, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN , 0)},
   {"L8A8"           , false, false,   2, 16,   8, 8, 8, 8,   0,0, 2,   1, 1, 2, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN , GL_SWIZZLE ? GL_RG8 : GL_LUMINANCE8_ALPHA8)},
   {"L8A8_SRGB"      , false, false,   2, 16,   8, 8, 8, 8,   0,0, 2,   1, 1, 2, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN , 0)},

   {"I8"             , false, false,   1,  8,   8, 0, 0, 0,   0,0, 1,   1, 1, 1, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN           , 0)},
   {"I16"            , false, true ,   2, 16,  16, 0, 0, 0,   0,0, 1,   1, 1, 2, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN           , 0)},
   {"I24"            , false, true ,   3, 24,  24, 0, 0, 0,   0,0, 1,   1, 1, 3, IMAGE_PRECISION_24, 0, GPU_API(DXGI_FORMAT_UNKNOWN           , 0)},
   {"I32"            , false, true ,   4, 32,  32, 0, 0, 0,   0,0, 1,   1, 1, 4, IMAGE_PRECISION_32, 0, GPU_API(DXGI_FORMAT_UNKNOWN           , 0)},
   {"F16"            , false, true ,   2, 16,  16, 0, 0, 0,   0,0, 1,   1, 1, 2, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_R16_FLOAT         , GL_R16F   )},
   {"F32"            , false, true ,   4, 32,  32, 0, 0, 0,   0,0, 1,   1, 1, 4, IMAGE_PRECISION_32, 0, GPU_API(DXGI_FORMAT_R32_FLOAT         , GL_R32F   )},
   {"F16_2"          , false, true ,   4, 32,  16,16, 0, 0,   0,0, 2,   1, 1, 4, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_R16G16_FLOAT      , GL_RG16F  )},
   {"F32_2"          , false, true ,   8, 64,  32,32, 0, 0,   0,0, 2,   1, 1, 8, IMAGE_PRECISION_32, 0, GPU_API(DXGI_FORMAT_R32G32_FLOAT      , GL_RG32F  )},
   {"F16_3"          , false, true ,   6, 48,  16,16,16, 0,   0,0, 3,   1, 1, 6, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN           , GL_RGB16F )},
   {"F32_3"          , false, true ,  12, 96,  32,32,32, 0,   0,0, 3,   1, 1,12, IMAGE_PRECISION_32, 0, GPU_API(DXGI_FORMAT_R32G32B32_FLOAT   , GL_RGB32F )},
   {"F16_4"          , false, true ,   8, 64,  16,16,16,16,   0,0, 4,   1, 1, 8, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_R16G16B16A16_FLOAT, GL_RGBA16F)},
   {"F32_4"          , false, true ,  16,128,  32,32,32,32,   0,0, 4,   1, 1,16, IMAGE_PRECISION_32, 0, GPU_API(DXGI_FORMAT_R32G32B32A32_FLOAT, GL_RGBA32F)},
   {"F32_3_SRGB"     , false, true ,  12, 96,  32,32,32, 0,   0,0, 3,   1, 1,12, IMAGE_PRECISION_32, 0, GPU_API(DXGI_FORMAT_UNKNOWN           , 0)},
   {"F32_4_SRGB"     , false, true ,  16,128,  32,32,32,32,   0,0, 4,   1, 1,16, IMAGE_PRECISION_32, 0, GPU_API(DXGI_FORMAT_UNKNOWN           , 0)},

   {"BC1"            , true , false, 1/2,  4,   5, 6, 5, 0,   0,0, 4,   4, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_BC1_UNORM     , GL_COMPRESSED_RGBA_S3TC_DXT1_EXT      )}, // set 0 alpha bits, even though BC1 can support 1-bit alpha, it's never used in the engine because it's limited (transparent pixels need to be black), so for simplicity BC1 is assumed to be only RGB, other formats are used for alpha #BC1RGB
   {"BC1_SRGB"       , true , false, 1/2,  4,   5, 6, 5, 0,   0,0, 4,   4, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_BC1_UNORM_SRGB, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT)}, // set 0 alpha bits, even though BC1 can support 1-bit alpha, it's never used in the engine because it's limited (transparent pixels need to be black), so for simplicity BC1 is assumed to be only RGB, other formats are used for alpha #BC1RGB
   {"BC2"            , true , false,   1,  8,   5, 6, 5, 4,   0,0, 4,   4, 4,16, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_BC2_UNORM     , GL_COMPRESSED_RGBA_S3TC_DXT3_EXT      )},
   {"BC2_SRGB"       , true , false,   1,  8,   5, 6, 5, 4,   0,0, 4,   4, 4,16, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_BC2_UNORM_SRGB, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT)},
   {"BC3"            , true , false,   1,  8,   5, 6, 5, 8,   0,0, 4,   4, 4,16, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_BC3_UNORM     , GL_COMPRESSED_RGBA_S3TC_DXT5_EXT      )},
   {"BC3_SRGB"       , true , false,   1,  8,   5, 6, 5, 8,   0,0, 4,   4, 4,16, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_BC3_UNORM_SRGB, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT)},
   {"BC4"            , true , false, 1/2,  4,   8, 0, 0, 0,   0,0, 1,   4, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_BC4_UNORM     , GL_COMPRESSED_RED_RGTC1               )},
   {"BC4_SIGN"       , true , true , 1/2,  4,   8, 0, 0, 0,   0,0, 1,   4, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_BC4_SNORM     , GL_COMPRESSED_SIGNED_RED_RGTC1        )},
   {"BC5"            , true , false,   1,  8,   8, 8, 0, 0,   0,0, 2,   4, 4,16, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_BC5_UNORM     , GL_COMPRESSED_RG_RGTC2                )},
   {"BC5_SIGN"       , true , true ,   1,  8,   8, 8, 0, 0,   0,0, 2,   4, 4,16, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_BC5_SNORM     , GL_COMPRESSED_SIGNED_RG_RGTC2         )},
   {"BC6"            , true , true ,   1,  8,  16,16,16, 0,   0,0, 3,   4, 4,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_BC6H_UF16     , GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT )},
   {"BC7"            , true , false,   1,  8,   7, 7, 7, 8,   0,0, 4,   4, 4,16, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_BC7_UNORM     , GL_COMPRESSED_RGBA_BPTC_UNORM         )},
   {"BC7_SRGB"       , true , false,   1,  8,   7, 7, 7, 8,   0,0, 4,   4, 4,16, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_BC7_UNORM_SRGB, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM   )},

   {"ETC2_R"         , true , false, 1/2,  4,   8, 0, 0, 0,   0,0, 1,   4, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_R11_EAC)},
   {"ETC2_R_SIGN"    , true , true , 1/2,  4,   8, 0, 0, 0,   0,0, 1,   4, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SIGNED_R11_EAC)},
   {"ETC2_RG"        , true , false,   1,  8,   8, 8, 0, 0,   0,0, 2,   4, 4,16, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RG11_EAC)},
   {"ETC2_RG_SIGN"   , true , true ,   1,  8,   8, 8, 0, 0,   0,0, 2,   4, 4,16, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SIGNED_RG11_EAC)},
   {"ETC2_RGB"       , true , false, 1/2,  4,   8, 8, 8, 0,   0,0, 3,   4, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGB8_ETC2)},
   {"ETC2_RGB_SRGB"  , true , false, 1/2,  4,   8, 8, 8, 0,   0,0, 3,   4, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SRGB8_ETC2)},
   {"ETC2_RGBA1"     , true , false, 1/2,  4,   8, 8, 8, 1,   0,0, 4,   4, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2)},
   {"ETC2_RGBA1_SRGB", true , false, 1/2,  4,   8, 8, 8, 1,   0,0, 4,   4, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2)},
   {"ETC2_RGBA"      , true , false,   1,  8,   8, 8, 8, 8,   0,0, 4,   4, 4,16, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA8_ETC2_EAC)},
   {"ETC2_RGBA_SRGB" , true , false,   1,  8,   8, 8, 8, 8,   0,0, 4,   4, 4,16, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC)},

   {"PVRTC1_2"       , true , false, 1/4,  2,   8, 8, 8, 8,   0,0, 4,   8, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)},
   {"PVRTC1_2_SRGB"  , true , false, 1/4,  2,   8, 8, 8, 8,   0,0, 4,   8, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT)},
   {"PVRTC1_4"       , true , false, 1/2,  4,   8, 8, 8, 8,   0,0, 4,   4, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)},
   {"PVRTC1_4_SRGB"  , true , false, 1/2,  4,   8, 8, 8, 8,   0,0, 4,   4, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT)},

   {null             , false, false,   0,  0,   0, 0, 0, 0,   0,0, 0,   0, 0, 0, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, 0)},

   {"B8G8R8A8"       , false, false,   4, 32,   8, 8, 8, 8,   0,0, 4,   1, 1, 4, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_B8G8R8A8_UNORM     , 0)},
   {"B8G8R8A8_SRGB"  , false, false,   4, 32,   8, 8, 8, 8,   0,0, 4,   1, 1, 4, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, 0)},
   {"B8G8R8"         , false, false,   3, 24,   8, 8, 8, 0,   0,0, 3,   1, 1, 3, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN            , 0)},
   {"B8G8R8_SRGB"    , false, false,   3, 24,   8, 8, 8, 0,   0,0, 3,   1, 1, 3, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN            , 0)},

   {"B5G6R5"         , false, false,   2, 16,   5, 6, 5, 0,   0,0, 3,   1, 1, 2, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_B5G6R5_UNORM  , 0)},
   {"B5G5R5A1"       , false, false,   2, 16,   5, 5, 5, 1,   0,0, 4,   1, 1, 2, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_B5G5R5A1_UNORM, 0)},
   {"B4G4R4A4"       , false, false,   2, 16,   4, 4, 4, 4,   0,0, 4,   1, 1, 2, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN       , 0)},

   {"D16"            , false, true ,   2, 16,   0, 0, 0, 0,  16,0, 1,   1, 1, 2, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_D16_UNORM           , GL_DEPTH_COMPONENT16 )},
   {"D24X8"          , false, true ,   4, 32,   0, 0, 0, 0,  24,0, 1,   1, 1, 4, IMAGE_PRECISION_24, 0, GPU_API(DXGI_FORMAT_UNKNOWN             , GL_DEPTH_COMPONENT24 )},
   {"D24S8"          , false, true ,   4, 32,   0, 0, 0, 0,  24,8, 2,   1, 1, 4, IMAGE_PRECISION_24, 0, GPU_API(DXGI_FORMAT_D24_UNORM_S8_UINT   , GL_DEPTH24_STENCIL8  )},
   {"D32"            , false, true ,   4, 32,   0, 0, 0, 0,  32,0, 1,   1, 1, 4, IMAGE_PRECISION_32, 0, GPU_API(DXGI_FORMAT_D32_FLOAT           , GL_DEPTH_COMPONENT32F)},
   {"D32S8X24"       , false, true ,   8, 64,   0, 0, 0, 0,  32,8, 2,   1, 1, 8, IMAGE_PRECISION_32, 0, GPU_API(DXGI_FORMAT_D32_FLOAT_S8X24_UINT, GL_DEPTH32F_STENCIL8 )},

   {"ETC1"           , true , false, 1/2,  4,   8, 8, 8, 0,   0,0, 3,   4, 4, 8, IMAGE_PRECISION_8 , 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_ETC1_RGB8_OES)},

   {"ASTC_4x4"       , true , true ,   1,  8,  16,16,16,16,   0,0, 4,   4, 4,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA_ASTC_4x4_KHR)},
   {"ASTC_4x4_SRGB"  , true , true ,   1,  8,  16,16,16,16,   0,0, 4,   4, 4,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR)},
   {"ASTC_5x4"       , true , true ,0.80,6.40, 16,16,16,16,   0,0, 4,   5, 4,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA_ASTC_5x4_KHR)},
   {"ASTC_5x4_SRGB"  , true , true ,0.80,6.40, 16,16,16,16,   0,0, 4,   5, 4,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR)},
   {"ASTC_5x5"       , true , true ,0.64,5.12, 16,16,16,16,   0,0, 4,   5, 5,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA_ASTC_5x5_KHR)},
   {"ASTC_5x5_SRGB"  , true , true ,0.64,5.12, 16,16,16,16,   0,0, 4,   5, 5,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR)},
   {"ASTC_6x5"       , true , true ,0.53,4.27, 16,16,16,16,   0,0, 4,   6, 5,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA_ASTC_6x5_KHR)},
   {"ASTC_6x5_SRGB"  , true , true ,0.53,4.27, 16,16,16,16,   0,0, 4,   6, 5,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR)},
   {"ASTC_6x6"       , true , true ,0.44,3.56, 16,16,16,16,   0,0, 4,   6, 6,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA_ASTC_6x6_KHR)},
   {"ASTC_6x6_SRGB"  , true , true ,0.44,3.56, 16,16,16,16,   0,0, 4,   6, 6,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR)},
   {"ASTC_8x5"       , true , true ,0.40,3.20, 16,16,16,16,   0,0, 4,   8, 5,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA_ASTC_8x5_KHR)},
   {"ASTC_8x5_SRGB"  , true , true ,0.40,3.20, 16,16,16,16,   0,0, 4,   8, 5,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR)},
   {"ASTC_8x6"       , true , true ,0.33,2.67, 16,16,16,16,   0,0, 4,   8, 6,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA_ASTC_8x6_KHR)},
   {"ASTC_8x6_SRGB"  , true , true ,0.33,2.67, 16,16,16,16,   0,0, 4,   8, 6,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR)},
   {"ASTC_8x8"       , true , true , 1/4,  2,  16,16,16,16,   0,0, 4,   8, 8,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_RGBA_ASTC_8x8_KHR)},
   {"ASTC_8x8_SRGB"  , true , true , 1/4,  2,  16,16,16,16,   0,0, 4,   8, 8,16, IMAGE_PRECISION_16, 0, GPU_API(DXGI_FORMAT_UNKNOWN, GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR)},

   {"R11G11B10F"     , false, true ,   4, 32,  11,11,10, 0,   0,0, 3,   1, 1, 4, IMAGE_PRECISION_10, 0, GPU_API(DXGI_FORMAT_R11G11B10_FLOAT   , GL_R11F_G11F_B10F)},
   {"R9G9B9E5F"      , false, true ,   4, 32,  14,14,14, 0,   0,0, 3,   1, 1, 4, IMAGE_PRECISION_10, 0, GPU_API(DXGI_FORMAT_R9G9B9E5_SHAREDEXP, GL_RGB9_E5)},
}; ASSERT(IMAGE_ALL_TYPES==89);
Bool ImageTypeInfo::_usage_known=false;
/******************************************************************************/
Bool IsSRGB(IMAGE_TYPE type)
{
   switch(type)
   {
      default: return false;

      case IMAGE_B8G8R8A8_SRGB  :
      case IMAGE_B8G8R8_SRGB    :
      case IMAGE_R8G8B8A8_SRGB  :
      case IMAGE_R8G8B8_SRGB    :
      case IMAGE_L8_SRGB        :
      case IMAGE_L8A8_SRGB      :
      case IMAGE_F32_3_SRGB     :
      case IMAGE_F32_4_SRGB     :
      case IMAGE_BC1_SRGB       :
      case IMAGE_BC2_SRGB       :
      case IMAGE_BC3_SRGB       :
      case IMAGE_BC7_SRGB       :
      case IMAGE_ETC2_RGB_SRGB  :
      case IMAGE_ETC2_RGBA1_SRGB:
      case IMAGE_ETC2_RGBA_SRGB :
      case IMAGE_PVRTC1_2_SRGB  :
      case IMAGE_PVRTC1_4_SRGB  :
      case IMAGE_ASTC_4x4_SRGB  :
      case IMAGE_ASTC_5x4_SRGB  :
      case IMAGE_ASTC_5x5_SRGB  :
      case IMAGE_ASTC_6x5_SRGB  :
      case IMAGE_ASTC_6x6_SRGB  :
      case IMAGE_ASTC_8x5_SRGB  :
      case IMAGE_ASTC_8x6_SRGB  :
      case IMAGE_ASTC_8x8_SRGB  :
         return true;
   }
}
Bool IsSByte(IMAGE_TYPE type)
{
   switch(type)
   {
      default: return false;

      case IMAGE_R8G8B8A8_SIGN:
      case IMAGE_R8G8_SIGN:
      case IMAGE_R8_SIGN:
      case IMAGE_BC4_SIGN:
      case IMAGE_BC5_SIGN:
      case IMAGE_ETC2_R_SIGN:
      case IMAGE_ETC2_RG_SIGN:
         return true;
   }
}
IMAGE_TYPE ImageTypeIncludeRGB(IMAGE_TYPE type)
{
   switch(type)
   {
      default: return type;

      case IMAGE_R8  :
      case IMAGE_R8G8:
         return IMAGE_R8G8B8;

      case IMAGE_BC4:
      case IMAGE_BC5:
         return IMAGE_BC1;

      case IMAGE_ETC2_R :
      case IMAGE_ETC2_RG:
         return IMAGE_ETC2_RGB;

      case IMAGE_R8_SIGN     :
      case IMAGE_R8G8_SIGN   :
      case IMAGE_BC4_SIGN    :
      case IMAGE_BC5_SIGN    :
      case IMAGE_ETC2_R_SIGN :
      case IMAGE_ETC2_RG_SIGN:
         return IMAGE_R8G8B8A8_SIGN; // since there's no IMAGE_R8G8B8_SIGN

      case IMAGE_A8       : return IMAGE_R8G8B8A8;
      case IMAGE_L8       : return IMAGE_R8G8B8;
      case IMAGE_L8_SRGB  : return IMAGE_R8G8B8_SRGB;
      case IMAGE_L8A8     : return IMAGE_R8G8B8A8;
      case IMAGE_L8A8_SRGB: return IMAGE_R8G8B8A8_SRGB;

      case IMAGE_I8 : return IMAGE_R8G8B8;
      case IMAGE_I16:
      case IMAGE_I24:
      case IMAGE_I32:
         return IMAGE_F32_3;

      case IMAGE_F16  :
      case IMAGE_F16_2:
         return IMAGE_F16_3;

      case IMAGE_F32  :
      case IMAGE_F32_2:
         return IMAGE_F32_3;
   }
}
IMAGE_TYPE ImageTypeIncludeAlpha(IMAGE_TYPE type)
{
   switch(type)
   {
      default: return type;

      case IMAGE_R8G8B8:
      case IMAGE_R8G8  :
      case IMAGE_R8    :
         return IMAGE_R8G8B8A8;

      case IMAGE_B8G8R8_SRGB: return IMAGE_B8G8R8A8_SRGB;

      case IMAGE_R8G8B8_SRGB: return IMAGE_R8G8B8A8_SRGB;

      case IMAGE_L8 :
      case IMAGE_I8 :
      case IMAGE_I16:
      case IMAGE_I24:
      case IMAGE_I32:
         return IMAGE_L8A8;

      case IMAGE_L8_SRGB: return IMAGE_L8A8_SRGB;

      case IMAGE_BC1     : case IMAGE_BC4: case IMAGE_BC5: return IMAGE_BC7     ; // BC1 has only 1-bit alpha which is not enough
      case IMAGE_BC1_SRGB:                                 return IMAGE_BC7_SRGB; // BC1 has only 1-bit alpha which is not enough

      case IMAGE_BC4_SIGN    :
      case IMAGE_BC5_SIGN    :
      case IMAGE_ETC2_R_SIGN :
      case IMAGE_ETC2_RG_SIGN:
         return IMAGE_R8G8B8A8_SIGN;

      case IMAGE_BC6  :
      case IMAGE_F16  :
      case IMAGE_F16_2:
      case IMAGE_F16_3:
      case IMAGE_R11G11B10F:
         return IMAGE_F16_4;

      case IMAGE_F32  :
      case IMAGE_F32_2:
      case IMAGE_F32_3:
         return IMAGE_F32_4;

      case IMAGE_F32_3_SRGB: return IMAGE_F32_4_SRGB;

      case IMAGE_ETC1      :
      case IMAGE_ETC2_R    :
      case IMAGE_ETC2_RG   :
      case IMAGE_ETC2_RGB  :
      case IMAGE_ETC2_RGBA1: // ETC2_RGBA1 has only 1-bit alpha which is not enough
         return IMAGE_ETC2_RGBA;

      case IMAGE_ETC2_RGB_SRGB  :
      case IMAGE_ETC2_RGBA1_SRGB:
         return IMAGE_ETC2_RGBA_SRGB; // ETC2_RGBA1_SRGB has only 1-bit alpha which is not enough
   }
}
IMAGE_TYPE ImageTypeExcludeAlpha(IMAGE_TYPE type)
{
   switch(type)
   {
      default: return type;

      case IMAGE_R8G8B8A8: return IMAGE_R8G8B8;
      case IMAGE_B8G8R8A8: return IMAGE_B8G8R8;

      case IMAGE_R8G8B8A8_SRGB: return IMAGE_R8G8B8_SRGB;
      case IMAGE_B8G8R8A8_SRGB: return IMAGE_B8G8R8_SRGB;

      case IMAGE_L8A8: return IMAGE_L8;

      case IMAGE_L8A8_SRGB: return IMAGE_L8_SRGB;

      case IMAGE_BC2:
      case IMAGE_BC3:
      case IMAGE_BC7:
         return IMAGE_BC1;

      case IMAGE_BC2_SRGB:
      case IMAGE_BC3_SRGB:
      case IMAGE_BC7_SRGB:
         return IMAGE_BC1_SRGB;

      case IMAGE_F16_4: return IMAGE_F16_3;
      case IMAGE_F32_4: return IMAGE_F32_3;

      case IMAGE_F32_4_SRGB: return IMAGE_F32_3_SRGB;

      case IMAGE_ETC2_RGBA1:
      case IMAGE_ETC2_RGBA :
         return IMAGE_ETC2_RGB;

      case IMAGE_ETC2_RGBA1_SRGB:
      case IMAGE_ETC2_RGBA_SRGB :
         return IMAGE_ETC2_RGB_SRGB;
   }
}
IMAGE_TYPE ImageTypeUncompressed(IMAGE_TYPE type)
{
   switch(type)
   {
      default: return type;

      case IMAGE_BC4   :
      case IMAGE_ETC2_R:
         return IMAGE_R8;

      case IMAGE_BC5    :
      case IMAGE_ETC2_RG:
         return IMAGE_R8G8;

      case IMAGE_BC4_SIGN   :
      case IMAGE_ETC2_R_SIGN:
         return IMAGE_R8_SIGN;

      case IMAGE_BC5_SIGN    :
      case IMAGE_ETC2_RG_SIGN:
         return IMAGE_R8G8_SIGN;

      case IMAGE_BC1 : // use since there's no other desktop compressed format without alpha
      case IMAGE_ETC1:
      case IMAGE_ETC2_RGB:
         return IMAGE_R8G8B8;

      case IMAGE_BC2:
      case IMAGE_BC3:
      case IMAGE_BC7:
      case IMAGE_ETC2_RGBA1:
      case IMAGE_ETC2_RGBA :
      case IMAGE_PVRTC1_2:
      case IMAGE_PVRTC1_4:
         return IMAGE_R8G8B8A8;

      case IMAGE_BC1_SRGB:
      case IMAGE_ETC2_RGB_SRGB:
         return IMAGE_R8G8B8_SRGB;

      case IMAGE_BC2_SRGB:
      case IMAGE_BC3_SRGB:
      case IMAGE_BC7_SRGB:
      case IMAGE_ETC2_RGBA1_SRGB:
      case IMAGE_ETC2_RGBA_SRGB :
      case IMAGE_PVRTC1_2_SRGB:
      case IMAGE_PVRTC1_4_SRGB:
         return IMAGE_R8G8B8A8_SRGB;

      case IMAGE_BC6:
         return IMAGE_F16_3;

      case IMAGE_ASTC_4x4:
      case IMAGE_ASTC_5x4:
      case IMAGE_ASTC_5x5:
      case IMAGE_ASTC_6x5:
      case IMAGE_ASTC_6x6:
      case IMAGE_ASTC_8x5:
      case IMAGE_ASTC_8x6:
      case IMAGE_ASTC_8x8:
         return IMAGE_F16_4;

      case IMAGE_ASTC_4x4_SRGB:
      case IMAGE_ASTC_5x4_SRGB:
      case IMAGE_ASTC_5x5_SRGB:
      case IMAGE_ASTC_6x5_SRGB:
      case IMAGE_ASTC_6x6_SRGB:
      case IMAGE_ASTC_8x5_SRGB:
      case IMAGE_ASTC_8x6_SRGB:
      case IMAGE_ASTC_8x8_SRGB:
         return IMAGE_F16_4; // Warning: TODO: should be IMAGE_F16_4_SRGB, without it these require IC_CONVERT_GAMMA
   }
}
IMAGE_TYPE ImageTypeOnFail(IMAGE_TYPE type) // this is for HW images, don't return IMAGE_R8G8B8 / IMAGE_R8G8B8_SRGB
{
   switch(type)
   {
      default: return IMAGE_R8G8B8A8;

      case IMAGE_NONE         : // don't try if original is empty
      case IMAGE_R8G8B8A8     : // don't try the same type again
      case IMAGE_R8G8B8A8_SRGB:
         return IMAGE_NONE;

      case IMAGE_B8G8R8A8_SRGB  :
      case IMAGE_B8G8R8_SRGB    :
      case IMAGE_R8G8B8_SRGB    :
      case IMAGE_L8_SRGB        :
      case IMAGE_L8A8_SRGB      :
      case IMAGE_BC1_SRGB       :
      case IMAGE_BC2_SRGB       :
      case IMAGE_BC3_SRGB       :
      case IMAGE_BC7_SRGB       :
      case IMAGE_ETC2_RGB_SRGB  :
      case IMAGE_ETC2_RGBA1_SRGB:
      case IMAGE_ETC2_RGBA_SRGB :
      case IMAGE_PVRTC1_2_SRGB  :
      case IMAGE_PVRTC1_4_SRGB  :
         return IMAGE_R8G8B8A8_SRGB;

      case IMAGE_F16_3:
         return IMAGE_F16_4;

      // Warning: these require IC_CONVERT_GAMMA
      case IMAGE_F32_3_SRGB: return IMAGE_F32_3;
      case IMAGE_F32_4_SRGB: return IMAGE_F32_4;

      case IMAGE_BC4   :
      case IMAGE_ETC2_R:
         return IMAGE_R8;

      case IMAGE_BC5    :
      case IMAGE_ETC2_RG:
         return IMAGE_R8G8;

      case IMAGE_BC4_SIGN   :
      case IMAGE_ETC2_R_SIGN:
         return IMAGE_R8_SIGN;

      case IMAGE_BC5_SIGN    :
      case IMAGE_ETC2_RG_SIGN:
         return IMAGE_R8G8_SIGN;

      case IMAGE_BC6:
      case IMAGE_R11G11B10F:
         return IMAGE_F16_3;

      case IMAGE_ASTC_4x4:
      case IMAGE_ASTC_5x4:
      case IMAGE_ASTC_5x5:
      case IMAGE_ASTC_6x5:
      case IMAGE_ASTC_6x6:
      case IMAGE_ASTC_8x5:
      case IMAGE_ASTC_8x6:
      case IMAGE_ASTC_8x8:
         return IMAGE_F16_4;

      case IMAGE_ASTC_4x4_SRGB:
      case IMAGE_ASTC_5x4_SRGB:
      case IMAGE_ASTC_5x5_SRGB:
      case IMAGE_ASTC_6x5_SRGB:
      case IMAGE_ASTC_6x6_SRGB:
      case IMAGE_ASTC_8x5_SRGB:
      case IMAGE_ASTC_8x6_SRGB:
      case IMAGE_ASTC_8x8_SRGB:
         return IMAGE_F16_4; // Warning: TODO: should be IMAGE_F16_4_SRGB, without it these require IC_CONVERT_GAMMA
   }
}
IMAGE_TYPE ImageTypeIncludeSRGB(IMAGE_TYPE type)
{
   switch(type)
   {
      default              : return type;
      case IMAGE_B8G8R8A8  : return IMAGE_B8G8R8A8_SRGB;
      case IMAGE_B8G8R8    : return IMAGE_B8G8R8_SRGB;
      case IMAGE_R8G8B8A8  : return IMAGE_R8G8B8A8_SRGB;
      case IMAGE_R8G8B8    : return IMAGE_R8G8B8_SRGB;
      case IMAGE_L8        : return IMAGE_L8_SRGB;
      case IMAGE_L8A8      : return IMAGE_L8A8_SRGB;
      case IMAGE_F32_3     : return IMAGE_F32_3_SRGB;
      case IMAGE_F32_4     : return IMAGE_F32_4_SRGB;
      case IMAGE_BC1       : return IMAGE_BC1_SRGB;
      case IMAGE_BC2       : return IMAGE_BC2_SRGB;
      case IMAGE_BC3       : return IMAGE_BC3_SRGB;
      case IMAGE_BC7       : return IMAGE_BC7_SRGB;
      case IMAGE_ETC2_RGB  : return IMAGE_ETC2_RGB_SRGB;
      case IMAGE_ETC2_RGBA1: return IMAGE_ETC2_RGBA1_SRGB;
      case IMAGE_ETC2_RGBA : return IMAGE_ETC2_RGBA_SRGB;
      case IMAGE_PVRTC1_2  : return IMAGE_PVRTC1_2_SRGB;
      case IMAGE_PVRTC1_4  : return IMAGE_PVRTC1_4_SRGB;
      case IMAGE_ASTC_4x4  : return IMAGE_ASTC_4x4_SRGB;
      case IMAGE_ASTC_5x4  : return IMAGE_ASTC_5x4_SRGB;
      case IMAGE_ASTC_5x5  : return IMAGE_ASTC_5x5_SRGB;
      case IMAGE_ASTC_6x5  : return IMAGE_ASTC_6x5_SRGB;
      case IMAGE_ASTC_6x6  : return IMAGE_ASTC_6x6_SRGB;
      case IMAGE_ASTC_8x5  : return IMAGE_ASTC_8x5_SRGB;
      case IMAGE_ASTC_8x6  : return IMAGE_ASTC_8x6_SRGB;
      case IMAGE_ASTC_8x8  : return IMAGE_ASTC_8x8_SRGB;
   }
}
IMAGE_TYPE ImageTypeExcludeSRGB(IMAGE_TYPE type)
{
   switch(type)
   {
      default                   : return type;
      case IMAGE_B8G8R8A8_SRGB  : return IMAGE_B8G8R8A8;
      case IMAGE_B8G8R8_SRGB    : return IMAGE_B8G8R8;
      case IMAGE_R8G8B8A8_SRGB  : return IMAGE_R8G8B8A8;
      case IMAGE_R8G8B8_SRGB    : return IMAGE_R8G8B8;
      case IMAGE_L8_SRGB        : return IMAGE_L8;
      case IMAGE_L8A8_SRGB      : return IMAGE_L8A8;
      case IMAGE_F32_3_SRGB     : return IMAGE_F32_3;
      case IMAGE_F32_4_SRGB     : return IMAGE_F32_4;
      case IMAGE_BC1_SRGB       : return IMAGE_BC1;
      case IMAGE_BC2_SRGB       : return IMAGE_BC2;
      case IMAGE_BC3_SRGB       : return IMAGE_BC3;
      case IMAGE_BC7_SRGB       : return IMAGE_BC7;
      case IMAGE_ETC2_RGB_SRGB  : return IMAGE_ETC2_RGB;
      case IMAGE_ETC2_RGBA1_SRGB: return IMAGE_ETC2_RGBA1;
      case IMAGE_ETC2_RGBA_SRGB : return IMAGE_ETC2_RGBA;
      case IMAGE_PVRTC1_2_SRGB  : return IMAGE_PVRTC1_2;
      case IMAGE_PVRTC1_4_SRGB  : return IMAGE_PVRTC1_4;
      case IMAGE_ASTC_4x4_SRGB  : return IMAGE_ASTC_4x4;
      case IMAGE_ASTC_5x4_SRGB  : return IMAGE_ASTC_5x4;
      case IMAGE_ASTC_5x5_SRGB  : return IMAGE_ASTC_5x5;
      case IMAGE_ASTC_6x5_SRGB  : return IMAGE_ASTC_6x5;
      case IMAGE_ASTC_6x6_SRGB  : return IMAGE_ASTC_6x6;
      case IMAGE_ASTC_8x5_SRGB  : return IMAGE_ASTC_8x5;
      case IMAGE_ASTC_8x6_SRGB  : return IMAGE_ASTC_8x6;
      case IMAGE_ASTC_8x8_SRGB  : return IMAGE_ASTC_8x8;
   }
}
IMAGE_TYPE ImageTypeToggleSRGB(IMAGE_TYPE type)
{
   switch(type)
   {
      default                   : return type;
      case IMAGE_B8G8R8A8_SRGB  : return IMAGE_B8G8R8A8  ;   case IMAGE_B8G8R8A8  : return IMAGE_B8G8R8A8_SRGB;
      case IMAGE_B8G8R8_SRGB    : return IMAGE_B8G8R8    ;   case IMAGE_B8G8R8    : return IMAGE_B8G8R8_SRGB;
      case IMAGE_R8G8B8A8_SRGB  : return IMAGE_R8G8B8A8  ;   case IMAGE_R8G8B8A8  : return IMAGE_R8G8B8A8_SRGB;
      case IMAGE_R8G8B8_SRGB    : return IMAGE_R8G8B8    ;   case IMAGE_R8G8B8    : return IMAGE_R8G8B8_SRGB;
      case IMAGE_L8_SRGB        : return IMAGE_L8        ;   case IMAGE_L8        : return IMAGE_L8_SRGB;
      case IMAGE_L8A8_SRGB      : return IMAGE_L8A8      ;   case IMAGE_L8A8      : return IMAGE_L8A8_SRGB;
      case IMAGE_F32_3_SRGB     : return IMAGE_F32_3     ;   case IMAGE_F32_3     : return IMAGE_F32_3_SRGB;
      case IMAGE_F32_4_SRGB     : return IMAGE_F32_4     ;   case IMAGE_F32_4     : return IMAGE_F32_4_SRGB;
      case IMAGE_BC1_SRGB       : return IMAGE_BC1       ;   case IMAGE_BC1       : return IMAGE_BC1_SRGB;
      case IMAGE_BC2_SRGB       : return IMAGE_BC2       ;   case IMAGE_BC2       : return IMAGE_BC2_SRGB;
      case IMAGE_BC3_SRGB       : return IMAGE_BC3       ;   case IMAGE_BC3       : return IMAGE_BC3_SRGB;
      case IMAGE_BC7_SRGB       : return IMAGE_BC7       ;   case IMAGE_BC7       : return IMAGE_BC7_SRGB;
      case IMAGE_ETC2_RGB_SRGB  : return IMAGE_ETC2_RGB  ;   case IMAGE_ETC2_RGB  : return IMAGE_ETC2_RGB_SRGB;
      case IMAGE_ETC2_RGBA1_SRGB: return IMAGE_ETC2_RGBA1;   case IMAGE_ETC2_RGBA1: return IMAGE_ETC2_RGBA1_SRGB;
      case IMAGE_ETC2_RGBA_SRGB : return IMAGE_ETC2_RGBA ;   case IMAGE_ETC2_RGBA : return IMAGE_ETC2_RGBA_SRGB;
      case IMAGE_PVRTC1_2_SRGB  : return IMAGE_PVRTC1_2  ;   case IMAGE_PVRTC1_2  : return IMAGE_PVRTC1_2_SRGB;
      case IMAGE_PVRTC1_4_SRGB  : return IMAGE_PVRTC1_4  ;   case IMAGE_PVRTC1_4  : return IMAGE_PVRTC1_4_SRGB;
      case IMAGE_ASTC_4x4_SRGB  : return IMAGE_ASTC_4x4  ;   case IMAGE_ASTC_4x4  : return IMAGE_ASTC_4x4_SRGB;
      case IMAGE_ASTC_5x4_SRGB  : return IMAGE_ASTC_5x4  ;   case IMAGE_ASTC_5x4  : return IMAGE_ASTC_5x4_SRGB;
      case IMAGE_ASTC_5x5_SRGB  : return IMAGE_ASTC_5x5  ;   case IMAGE_ASTC_5x5  : return IMAGE_ASTC_5x5_SRGB;
      case IMAGE_ASTC_6x5_SRGB  : return IMAGE_ASTC_6x5  ;   case IMAGE_ASTC_6x5  : return IMAGE_ASTC_6x5_SRGB;
      case IMAGE_ASTC_6x6_SRGB  : return IMAGE_ASTC_6x6  ;   case IMAGE_ASTC_6x6  : return IMAGE_ASTC_6x6_SRGB;
      case IMAGE_ASTC_8x5_SRGB  : return IMAGE_ASTC_8x5  ;   case IMAGE_ASTC_8x5  : return IMAGE_ASTC_8x5_SRGB;
      case IMAGE_ASTC_8x6_SRGB  : return IMAGE_ASTC_8x6  ;   case IMAGE_ASTC_8x6  : return IMAGE_ASTC_8x6_SRGB;
      case IMAGE_ASTC_8x8_SRGB  : return IMAGE_ASTC_8x8  ;   case IMAGE_ASTC_8x8  : return IMAGE_ASTC_8x8_SRGB;
   }
}
IMAGE_TYPE ImageTypeHighPrec(IMAGE_TYPE type)
{
 C ImageTypeInfo &type_info=ImageTI[type];
   Bool srgb=IsSRGB(type);
   if(type_info.a)return  srgb ? IMAGE_F32_4_SRGB :  IMAGE_F32_4;
   if(type_info.b)return  srgb ? IMAGE_F32_3_SRGB :  IMAGE_F32_3;
   if(type_info.g)return/*srgb ? IMAGE_F32_2_SRGB :*/IMAGE_F32_2;
   if(type       )return/*srgb ? IMAGE_F32_SRGB   :*/IMAGE_F32  ;
                  return IMAGE_NONE;
}
Bool ImageSupported(IMAGE_TYPE type, IMAGE_MODE mode, Byte samples)
{
   if(!InRange(type, IMAGE_ALL_TYPES))return false; // invalid type
   if( IsSoft (mode)                 )return true ; // software supports all modes
   if(!type                          )return true ; // empty type is OK
   if(!ImageTypeInfo::usageKnown()   )return true ; // if usage unknown then assume it's supported so we can try
   UInt need=0, got=ImageTI[type].usage();
   switch(mode)
   {
      case IMAGE_2D        : need=ImageTypeInfo::USAGE_IMAGE_2D  ; break;
      case IMAGE_3D        : need=ImageTypeInfo::USAGE_IMAGE_3D  ; break;
      case IMAGE_CUBE      : need=ImageTypeInfo::USAGE_IMAGE_CUBE; break;
    //case IMAGE_SOFT     
    //case IMAGE_SOFT_CUBE
      case IMAGE_RT        : need=ImageTypeInfo::USAGE_IMAGE_2D  |ImageTypeInfo::USAGE_IMAGE_RT; break;
      case IMAGE_RT_CUBE   : need=ImageTypeInfo::USAGE_IMAGE_CUBE|ImageTypeInfo::USAGE_IMAGE_RT; break;
      case IMAGE_DS        : need=ImageTypeInfo::USAGE_IMAGE_2D  |ImageTypeInfo::USAGE_IMAGE_DS; break;
      case IMAGE_SHADOW_MAP: need=ImageTypeInfo::USAGE_IMAGE_2D  |ImageTypeInfo::USAGE_IMAGE_DS; break;
   #if DX11
      case IMAGE_STAGING   : return true;
   #elif GL
      case IMAGE_GL_RB     : return true;
   #endif
   }
   if(samples>1 && !(got&ImageTypeInfo::USAGE_IMAGE_MS))return false; // if need multi-sampling then require USAGE_IMAGE_MS
   return FlagAll(got, need); // require all flags from 'need'
}
#if DX11
static DXGI_FORMAT Typeless(IMAGE_TYPE type)
{
   switch(type)
   {
      default: return ImageTI[type].format;

      // these are the only sRGB formats that are used for Render Targets
      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: return DXGI_FORMAT_B8G8R8A8_TYPELESS;

      // these are needed in case we want to dynamically switch between sRGB/non-sRGB drawing
      case IMAGE_BC1: case IMAGE_BC1_SRGB: return DXGI_FORMAT_BC1_TYPELESS;
      case IMAGE_BC2: case IMAGE_BC2_SRGB: return DXGI_FORMAT_BC2_TYPELESS;
      case IMAGE_BC3: case IMAGE_BC3_SRGB: return DXGI_FORMAT_BC3_TYPELESS;
      case IMAGE_BC7: case IMAGE_BC7_SRGB: return DXGI_FORMAT_BC7_TYPELESS;

      // depth stencil
      case IMAGE_D16     : return DXGI_FORMAT_R16_TYPELESS;
      case IMAGE_D24S8   : return DXGI_FORMAT_R24G8_TYPELESS;
      case IMAGE_D32     : return DXGI_FORMAT_R32_TYPELESS;
      case IMAGE_D32S8X24: return DXGI_FORMAT_R32G8X24_TYPELESS;
   }
}
#endif
/******************************************************************************/
GPU_API(DXGI_FORMAT, UInt) ImageTypeToFormat(Int                        type  ) {return InRange(type, ImageTI) ? ImageTI[type].format : GPU_API(DXGI_FORMAT_UNKNOWN, 0);}
IMAGE_TYPE                 ImageFormatToType(GPU_API(DXGI_FORMAT, UInt) format
#if GL
, IMAGE_TYPE type
#endif
)
{
#if GL && GL_SWIZZLE
   switch(format)
   {
      case GL_R8 : if(type==IMAGE_A8 || type==IMAGE_L8)return type; return IMAGE_R8;
      case GL_RG8: if(type==IMAGE_L8A8                )return type; return IMAGE_R8G8;
   }
#endif
   FREPA(ImageTI) // !! it's important to go from the start in case some formats are listed more than once, return the first one (also this improves performance because most common formats are at the start) !!
      if(ImageTI[i].format==format)return IMAGE_TYPE(i);
   return IMAGE_NONE;
}
/******************************************************************************/
IMAGE_PRECISION BitsToPrecision(Int bits)
{
   ASSERT(IMAGE_PRECISION_64+1==IMAGE_PRECISION_NUM);
   if(bits>=64)return IMAGE_PRECISION_64;
   if(bits>=32)return IMAGE_PRECISION_32;
   if(bits>=16)return IMAGE_PRECISION_16;
   if(bits>=10)return IMAGE_PRECISION_10;
 /*if(bits>= 8)*/return IMAGE_PRECISION_8;
}
IMAGE_TYPE BytesToImageType(Int byte_pp)
{
   switch(byte_pp)
   {
      default: return IMAGE_NONE;
      case  1: return IMAGE_I8;
      case  2: return IMAGE_I16;
      case  3: return IMAGE_I24;
      case  4: return IMAGE_I32;
   }
}
/******************************************************************************/
Bool IsSoft(IMAGE_MODE mode)
{
   switch(mode)
   {
      default: return false;

      case IMAGE_SOFT:
      case IMAGE_SOFT_CUBE: return true;
   }
}
Bool IsHW(IMAGE_MODE mode)
{
   switch(mode)
   {
      default: return true;

      case IMAGE_SOFT:
      case IMAGE_SOFT_CUBE: return false;
   }
}
Bool IsCube(IMAGE_MODE mode)
{
   switch(mode)
   {
      default: return false;

      case IMAGE_CUBE     :
      case IMAGE_SOFT_CUBE:
      case IMAGE_RT_CUBE  : return true;
   }
}
IMAGE_MODE AsSoft(IMAGE_MODE mode)
{
   switch(mode)
   {
      default: return IMAGE_SOFT;

      case IMAGE_CUBE     :
      case IMAGE_SOFT_CUBE:
      case IMAGE_RT_CUBE  : return IMAGE_SOFT_CUBE;
   }
}
/******************************************************************************/
Int ImageFaces(IMAGE_MODE mode) {return IsCube(mode) ? 6 : 1;}
Int PaddedWidth(Int w, Int h, Int mip, IMAGE_TYPE type)
{
 C auto &ti=ImageTI[type];

   switch(type) // special cases
   {
      case IMAGE_PVRTC1_2: case IMAGE_PVRTC1_2_SRGB:
      case IMAGE_PVRTC1_4: case IMAGE_PVRTC1_4_SRGB:
      {
         w=h=CeilPow2(Max(w, h)); // PVRTC1 must be square and power of 2
         UInt mip_w=Max(1, w>>mip); // 'UInt' for faster 'DivCeil'
         return Max(DivCeil(mip_w, (UInt)ti.block_w), 2)*ti.block_w; // PVRTC1 has min texture size (2bit=16x8, 4bit=8x8) which is equal to block*2
      }
   }

   UInt   mip_w=Max(1, w>>mip); if(ti.block_w>1)mip_w=AlignCeil(mip_w, (UInt)ti.block_w); // 'UInt' for faster 'AlignCeil'
   return mip_w;
}
Int PaddedHeight(Int w, Int h, Int mip, IMAGE_TYPE type)
{
 C auto &ti=ImageTI[type];

   switch(type) // special cases
   {
      case IMAGE_PVRTC1_2: case IMAGE_PVRTC1_2_SRGB:
      case IMAGE_PVRTC1_4: case IMAGE_PVRTC1_4_SRGB:
      {
         w=h=CeilPow2(Max(w, h)); // PVRTC1 must be square and power of 2
         UInt mip_h=Max(1, h>>mip); // 'UInt' for faster 'DivCeil'
         return Max(DivCeil(mip_h, (UInt)ti.block_h), 2)*ti.block_h; // PVRTC1 has min texture size (2bit=16x8, 4bit=8x8) which is equal to block*2
      }
   }

   UInt   mip_h=Max(1, h>>mip); if(ti.block_h>1)mip_h=AlignCeil(mip_h, (UInt)ti.block_h); // 'UInt' for faster 'AlignCeil'
   return mip_h;
}
Int ImagePitch(Int w, Int h, Int mip, IMAGE_TYPE type)
{
 C auto &ti=ImageTI[type];

   switch(type) // special cases
   {
      case IMAGE_PVRTC1_2: case IMAGE_PVRTC1_2_SRGB:
      case IMAGE_PVRTC1_4: case IMAGE_PVRTC1_4_SRGB:
      {
         w=h=CeilPow2(Max(w, h)); // PVRTC1 must be square and power of 2
         UInt mip_w=Max(1, w>>mip); // 'UInt' for faster 'DivCeil'
         return Max(DivCeil(mip_w, (UInt)ti.block_w), 2)*ti.block_bytes; // PVRTC1 has min texture size (2bit=16x8, 4bit=8x8) which is equal to block*2
      }
   }
   UInt   mip_w=Max(1, w>>mip); if(ti.block_w>1)mip_w=DivCeil(mip_w, (UInt)ti.block_w); // 'UInt' for faster 'DivCeil'
   return mip_w*ti.block_bytes;
}
Int ImageBlocksY(Int w, Int h, Int mip, IMAGE_TYPE type)
{
 C auto &ti=ImageTI[type];

   switch(type) // special cases
   {
      case IMAGE_PVRTC1_2: case IMAGE_PVRTC1_2_SRGB:
      case IMAGE_PVRTC1_4: case IMAGE_PVRTC1_4_SRGB:
      {
         w=h=CeilPow2(Max(w, h)); // PVRTC1 must be square and power of 2
         UInt mip_h=Max(1, h>>mip); // 'UInt' for faster 'DivCeil'
         return Max(DivCeil(mip_h, (UInt)ti.block_h), 2); // PVRTC1 has min texture size (2bit=16x8, 4bit=8x8) which is equal to block*2
      }
   }

   UInt   mip_h=Max(1, h>>mip); if(ti.block_h>1)mip_h=DivCeil(mip_h, (UInt)ti.block_h); // 'UInt' for faster 'DivCeil'
   return mip_h;
}
Int ImagePitch2(Int w, Int h, Int mip, IMAGE_TYPE type)
{
   return ImagePitch  (w, h, mip, type)
         *ImageBlocksY(w, h, mip, type);
}
Int ImageFaceSize(Int w, Int h, Int d, Int mip, IMAGE_TYPE type)
{
   return ImagePitch2(w, h, mip, type)*Max(1, d>>mip);
}
UInt ImageMipOffset(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps, Int mip_map)
{
   UInt   size=0; mip_map++; REP(mip_maps-mip_map)size+=ImageFaceSize(w, h, d, mip_map+i, type); if(IsCube(mode))size*=6; // #MipOrder
   return size;
}
UInt ImageSize(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps)
{
   UInt   size=0; REP(mip_maps)size+=ImageFaceSize(w, h, d, i, type); if(IsCube(mode))size*=6;
   return size;
}
/******************************************************************************/
Int TotalMipMaps(Int w, Int h, Int d)
{
 //if(type==IMAGE_PVRTC1_2 || type==IMAGE_PVRTC1_4 || type==IMAGE_PVRTC1_2_SRGB || type==IMAGE_PVRTC1_4_SRGB)w=h=CeilPow2(Max(w, h)); // PVRTC1 must be square and power of 2, for simplicity ignore this. In worst case we won't have the last 1x1 mip for non-square PVRTC
   Int    total=0; for(Int i=Max(w, h, d); i>=1; i>>=1)total++;
   return total;
}
/******************************************************************************/
Bool CompatibleLock(LOCK_MODE cur, LOCK_MODE lock)
{
   switch(cur)
   {
      default             :              // no lock yet
      case LOCK_READ_WRITE: return true; // full access

      case LOCK_WRITE     :
      case LOCK_APPEND    : return lock==LOCK_WRITE || lock==LOCK_APPEND;

      case LOCK_READ      : return lock==LOCK_READ;
   }
}
Bool IgnoreGamma(UInt flags, IMAGE_TYPE src, IMAGE_TYPE dest)
{
   if(IsSRGB(src)==IsSRGB(dest))return true; // if gamma is the same, then we can ignore it
   if(Int ignore=FlagOn(flags, IC_IGNORE_GAMMA)-FlagOn(flags, IC_CONVERT_GAMMA))return ignore>0; // if none or both flag specified then continue below (auto-detect)
   return (ImageTI[src].precision<=IMAGE_PRECISION_8) && (ImageTI[dest].precision<=IMAGE_PRECISION_8); // auto-detect, ignore only if both types are low-precision
}
Bool CanDoRawCopy(IMAGE_TYPE src, IMAGE_TYPE dest, Bool ignore_gamma)
{
   if(ignore_gamma)
   {
      src =ImageTypeExcludeSRGB(src );
      dest=ImageTypeExcludeSRGB(dest);
   }
   return src==dest;
}
Bool CanDoRawCopy(C Image &src, C Image &dest, Bool ignore_gamma)
{
   IMAGE_TYPE src_hwType= src.hwType(),  src_type= src.type(),
             dest_hwType=dest.hwType(), dest_type=dest.type();
   if(ignore_gamma)
   {
       src_hwType=ImageTypeExcludeSRGB( src_hwType);
      dest_hwType=ImageTypeExcludeSRGB(dest_hwType);
       src_type  =ImageTypeExcludeSRGB( src_type  );
      dest_type  =ImageTypeExcludeSRGB(dest_type  );
   }
   return src_hwType==dest_hwType
     && (dest_type  ==dest_hwType || src_type==dest_type); // check 'type' too in case we have to perform color adjustment
}
Bool CanCompress(IMAGE_TYPE dest)
{
   switch(dest)
   {
      default: return true;

      case IMAGE_BC6: case IMAGE_BC7: case IMAGE_BC7_SRGB:
         return CompressBC67!=null;

      case IMAGE_ETC1      :
      case IMAGE_ETC2_R    : case IMAGE_ETC2_R_SIGN    :
      case IMAGE_ETC2_RG   : case IMAGE_ETC2_RG_SIGN   :
      case IMAGE_ETC2_RGB  : case IMAGE_ETC2_RGB_SRGB  :
      case IMAGE_ETC2_RGBA1: case IMAGE_ETC2_RGBA1_SRGB:
      case IMAGE_ETC2_RGBA : case IMAGE_ETC2_RGBA_SRGB :
         return CompressETC!=null;

      case IMAGE_PVRTC1_2: case IMAGE_PVRTC1_2_SRGB:
      case IMAGE_PVRTC1_4: case IMAGE_PVRTC1_4_SRGB:
         return CompressPVRTC!=null;

      case IMAGE_ASTC_4x4: case IMAGE_ASTC_4x4_SRGB:
      case IMAGE_ASTC_5x4: case IMAGE_ASTC_5x4_SRGB:
      case IMAGE_ASTC_5x5: case IMAGE_ASTC_5x5_SRGB:
      case IMAGE_ASTC_6x5: case IMAGE_ASTC_6x5_SRGB:
      case IMAGE_ASTC_6x6: case IMAGE_ASTC_6x6_SRGB:
      case IMAGE_ASTC_8x5: case IMAGE_ASTC_8x5_SRGB:
      case IMAGE_ASTC_8x6: case IMAGE_ASTC_8x6_SRGB:
      case IMAGE_ASTC_8x8: case IMAGE_ASTC_8x8_SRGB:
         return CompressASTC!=null;
   }
}
/******************************************************************************/
#if GL
UInt SourceGLFormat(IMAGE_TYPE type)
{
   switch(type)
   {
      case IMAGE_I8 :
      case IMAGE_I16:
      case IMAGE_I24:
      case IMAGE_I32: return GL_LUMINANCE;

   #if GL_SWIZZLE
      case IMAGE_A8  : return GL_RED;
      case IMAGE_L8  : return GL_RED;
      case IMAGE_L8A8: return GL_RG;
   #else
      case IMAGE_A8  : return GL_ALPHA;
      case IMAGE_L8  : return GL_LUMINANCE;
      case IMAGE_L8A8: return GL_LUMINANCE_ALPHA;
   #endif

      case IMAGE_F16    :
      case IMAGE_F32    :
      case IMAGE_R8     :
      case IMAGE_R8_SIGN: return GL_RED;

      case IMAGE_F16_2    :
      case IMAGE_F32_2    :
      case IMAGE_R8G8     :
      case IMAGE_R8G8_SIGN: return GL_RG;

      case IMAGE_R11G11B10F :
      case IMAGE_R9G9B9E5F  :
      case IMAGE_F16_3      :
      case IMAGE_F32_3      :
      case IMAGE_R8G8B8_SRGB: // must be GL_RGB and NOT GL_SRGB
      case IMAGE_R8G8B8     : return GL_RGB;

      case IMAGE_F16_4        :
      case IMAGE_F32_4        :
      case IMAGE_R8G8B8A8     :
      case IMAGE_R8G8B8A8_SIGN:
      case IMAGE_R8G8B8A8_SRGB: // must be GL_RGBA and NOT GL_SRGB_ALPHA
      case IMAGE_R10G10B10A2  : return GL_RGBA;

      case IMAGE_B5G6R5:
      case IMAGE_B8G8R8_SRGB: // must be GL_BGR and NOT GL_SBGR
      case IMAGE_B8G8R8: return GL_BGR;

      case IMAGE_B4G4R4A4:
      case IMAGE_B5G5R5A1:
      case IMAGE_B8G8R8A8_SRGB: // must be GL_BGRA and NOT GL_SBGR_ALPHA
      case IMAGE_B8G8R8A8: return GL_BGRA;

      case IMAGE_D24S8:
      case IMAGE_D32S8X24: return GL_DEPTH_STENCIL;

      case IMAGE_D16  :
      case IMAGE_D24X8:
      case IMAGE_D32  : return GL_DEPTH_COMPONENT;

      default: return 0;
   }
}
/******************************************************************************/
UInt SourceGLType(IMAGE_TYPE type)
{
   switch(type)
   {
      case IMAGE_F16  :
      case IMAGE_F16_2:
      case IMAGE_F16_3:
      case IMAGE_F16_4: return GL_HALF_FLOAT;

      case IMAGE_F32  :
      case IMAGE_F32_2:
      case IMAGE_F32_3:
      case IMAGE_F32_4: return GL_FLOAT;

      case IMAGE_D16     : return GL_UNSIGNED_SHORT;
      case IMAGE_D24S8   : return GL_UNSIGNED_INT_24_8;
      case IMAGE_D24X8   : return GL_UNSIGNED_INT;
      case IMAGE_D32     : return GL_FLOAT;
      case IMAGE_D32S8X24: return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;

      case IMAGE_I8 : return GL_UNSIGNED_BYTE ;
      case IMAGE_I16: return GL_UNSIGNED_SHORT;
      case IMAGE_I32: return GL_UNSIGNED_INT  ;

      case IMAGE_R10G10B10A2: return GL_UNSIGNED_INT_2_10_10_10_REV;

      case IMAGE_R11G11B10F: return GL_UNSIGNED_INT_10F_11F_11F_REV;
      case IMAGE_R9G9B9E5F : return GL_UNSIGNED_INT_5_9_9_9_REV;

      case IMAGE_B4G4R4A4: return GL_UNSIGNED_SHORT_4_4_4_4;

      case IMAGE_B5G5R5A1: return GL_UNSIGNED_SHORT_5_5_5_1;

      case IMAGE_B5G6R5: return GL_UNSIGNED_SHORT_5_6_5;

      case IMAGE_R8G8B8A8_SIGN:
      case IMAGE_R8G8_SIGN    :
      case IMAGE_R8_SIGN      : return GL_BYTE;

      case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB:
      case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB:
      case IMAGE_R8G8B8  : case IMAGE_R8G8B8_SRGB  :
      case IMAGE_R8G8    :
      case IMAGE_R8      :
      case IMAGE_A8      :
      case IMAGE_L8      : case IMAGE_L8_SRGB      :
      case IMAGE_L8A8    : case IMAGE_L8A8_SRGB    :
      case IMAGE_B8G8R8  : case IMAGE_B8G8R8_SRGB  :
         return GL_UNSIGNED_BYTE;

      default: return 0;
   }
}
#endif
/******************************************************************************/
// MANAGE
/******************************************************************************/
Image::~Image()
{
   del();

   // remove image from 'ShaderImages', 'ShaderRWImages' and 'VI.image'
#if !SYNC_LOCK_SAFE
   if(ShaderImages.elms())
#endif
   {
      ShaderImages.lock  (); REPA(ShaderImages){ShaderImage &image=ShaderImages.lockedData(i); if(image.get()==this)image.set(null);}
      ShaderImages.unlock();
   }
#if !SYNC_LOCK_SAFE
   if(ShaderRWImages.elms())
#endif
   {
      ShaderRWImages.lock  (); REPA(ShaderRWImages){ShaderRWImage &image=ShaderRWImages.lockedData(i); if(image.get()==this)image.set(null);}
      ShaderRWImages.unlock();
   }
   if(VI._image==this)VI._image=null;
}
       Image::Image    (                                                                   )           {zero();}
       Image::Image    (C Image &src                                                       ) : Image() {src.mustCopy(T);}
       Image::Image    (Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps) : Image() {mustCreate(w, h, d, type, mode, mip_maps);}
Image& Image::operator=(C Image &src                                                       )           {if(this!=&src)src.mustCopy(T); return T;}
/******************************************************************************/
Image& Image::del()
{
   cancelStream();
   unlock();
   if(D.created())
   {
   #if DX11
      if(_txtr || _srv)
      {
         D.texClearAll(_srv);
      #if GPU_LOCK // lock not needed for 'Release'
         SyncLocker locker(D._lock);
      #endif
         if(D.created())
         {
            // release children first
            RELEASE(_srv);
            // now main resources
            RELEASE(_txtr);
         }
      }
   #elif GL
      if(_txtr || _rb)
      {
         D.texClearAll(_txtr);
      #if GPU_LOCK
         SyncLocker locker(D._lock);
      #endif
         if(D.created())
         {
            glDeleteTextures     (1, &_txtr);
            glDeleteRenderbuffers(1, &_rb  );
         }
      }
   #endif
   }
   Free(_data_all);
   zero(); return T;
}
/******************************************************************************/
void Image::setPartial()
{
   if(_partial=(w()!=hwW() || h()!=hwH() || d()!=hwD()))_part.set(Flt(w())/hwW(), Flt(h())/hwH(), Flt(d())/hwD());
   else                                                 _part=1;
}
#if DX11
Bool Image::setSRV()
{
   switch(mode())
   {
      case IMAGE_2D:
      case IMAGE_3D:
      case IMAGE_CUBE:
      case IMAGE_RT:
      case IMAGE_RT_CUBE:
      case IMAGE_DS:
      case IMAGE_SHADOW_MAP:
      {
         D3D11_SHADER_RESOURCE_VIEW_DESC srvd; Zero(srvd);
         switch(hwType())
         {
            default            : srvd.Format=hwTypeInfo().format; break;
            case IMAGE_D16     : srvd.Format=DXGI_FORMAT_R16_UNORM; break;
            case IMAGE_D24S8   : srvd.Format=DXGI_FORMAT_R24_UNORM_X8_TYPELESS; break;
            case IMAGE_D32     : srvd.Format=DXGI_FORMAT_R32_FLOAT; break;
            case IMAGE_D32S8X24: srvd.Format=DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS; break;
         }
         if(cube()          ){srvd.ViewDimension=D3D11_SRV_DIMENSION_TEXTURECUBE; srvd.TextureCube.MostDetailedMip=_base_mip; srvd.TextureCube.MipLevels=mipMaps()-_base_mip;}else
         if(mode()==IMAGE_3D){srvd.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE3D  ; srvd.Texture3D  .MostDetailedMip=_base_mip; srvd.Texture3D  .MipLevels=mipMaps()-_base_mip;}else
         if(!multiSample()  ){srvd.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D  ; srvd.Texture2D  .MostDetailedMip=_base_mip; srvd.Texture2D  .MipLevels=mipMaps()-_base_mip;}else
                             {srvd.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2DMS;}
      #if GPU_LOCK // lock not needed for 'D3D'
         SyncLocker locker(D._lock);
      #endif

      #if 0 // atomic
            ID3D11ShaderResourceView *new_srv=null; D3D->CreateShaderResourceView(_txtr, &srvd, &new_srv); // create new
         if(ID3D11ShaderResourceView *old_srv=AtomicGetSet(_srv, new_srv)){D.texClearAll(old_srv); old_srv->Release();}  // swap and delete old
         if(!new_srv && mode()!=IMAGE_DS)return false; // allow SRV optional in IMAGE_DS (for example it can fail for multi-sampled DS on FeatureLevel 10.0)
      #else
         if(_srv){D.texClearAll(_srv); _srv->Release(); _srv=null;} // delete old
         D3D->CreateShaderResourceView(_txtr, &srvd, &_srv); // create new
         if(!_srv && mode()!=IMAGE_DS)return false; // allow SRV optional in IMAGE_DS (for example it can fail for multi-sampled DS on FeatureLevel 10.0)
      #endif
      }break;
   }
   return true;
}
#endif
Bool Image::finalize()
{
#if DX11
   if(!setSRV())return false;
#endif
  _byte_pp=hwTypeInfo().byte_pp; // keep a copy for faster access
   setPartial();
   return true;
}
Bool Image::setInfo()
{
#if DX11
   if(_txtr)
   {
      if(mode()==IMAGE_3D)
      {
         D3D11_TEXTURE3D_DESC desc; static_cast<ID3D11Texture3D*>(_txtr)->GetDesc(&desc);
        _mips     =desc.MipLevels;
        _samples  =1;
        _hw_size.x=desc.Width;
        _hw_size.y=desc.Height;
        _hw_size.z=desc.Depth;
         if(IMAGE_TYPE hw_type=ImageFormatToType(desc.Format))T._hw_type=hw_type; // override only if detected, because Image could have been created with TYPELESS format which can't be directly decoded and IMAGE_NONE could be returned
      }else
      {
         D3D11_TEXTURE2D_DESC desc; static_cast<ID3D11Texture2D*>(_txtr)->GetDesc(&desc);
        _mips     =desc.MipLevels;
        _samples  =desc.SampleDesc.Count;
        _hw_size.x=desc.Width;
        _hw_size.y=desc.Height;
        _hw_size.z=1;
         if(IMAGE_TYPE hw_type=ImageFormatToType(desc.Format))T._hw_type=hw_type; // override only if detected, because Image could have been created with TYPELESS format which can't be directly decoded and IMAGE_NONE could be returned
      }
   }
#elif GL
   if(_txtr)switch(mode())
   {
      case IMAGE_2D        :
      case IMAGE_RT        :
      case IMAGE_DS        :
      case IMAGE_SHADOW_MAP:
      {
      #if !GL_ES // unavailable on OpenGL ES
         Int format;
         D.texBind               (GL_TEXTURE_2D, _txtr);
         glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH          , &_hw_size.x);
         glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT         , &_hw_size.y);
         glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, & format   );
        _hw_type=ImageFormatToType(format, hwType());
      #endif
        _hw_size.z=1;
      }break;

      case IMAGE_3D:
      {
      #if !GL_ES // unavailable on OpenGL ES
         Int format;
         D.texBind               (GL_TEXTURE_3D, _txtr);
         glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH          , &_hw_size.x);
         glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT         , &_hw_size.y);
         glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH          , &_hw_size.z);
         glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_INTERNAL_FORMAT, & format   );
        _hw_type=ImageFormatToType(format, hwType());
      #endif
      }break;

      case IMAGE_CUBE:
      case IMAGE_RT_CUBE:
      {
      #if !GL_ES // unavailable on OpenGL ES
         Int format;
         D.texBind               (GL_TEXTURE_CUBE_MAP, _txtr);
         glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH          , &_hw_size.x);
         glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT         , &_hw_size.y);
         glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_INTERNAL_FORMAT, & format   );
        _hw_type=ImageFormatToType(format, hwType());
      #endif
        _hw_size.z=1;
      }break;
   }else
   if(_rb)
   {
      Int format;
      glBindRenderbuffer          (GL_RENDERBUFFER, _rb);
      glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH          , &_hw_size.x);
      glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT         , &_hw_size.y);
      glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, & format   );
     _hw_type  =ImageFormatToType(format, hwType());
     _hw_size.z=1;
      MAX(_mips   , 1);
      MAX(_samples, 1);
   }
#endif
   return finalize();
}
void Image::forceInfo(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int samples)
{
  _hw_size=_size.set(w, h, d);
  _hw_type=_type=type;
           _mode=mode;
        _samples=samples;
   setInfo();
}
void Image::adjustInfo(Int w, Int h, Int d, IMAGE_TYPE type)
{
  _type=type;
  _size.x=Min(Max(1, w), hwW());
  _size.y=Min(Max(1, h), hwH());
  _size.z=Min(Max(1, d), hwD());
   if(soft())lockSoft();
   setPartial();
}
/******************************************************************************/
void Image::setGLParams()
{
#if GL
#if GPU_LOCK
   SyncLocker locker(D._lock);
#endif
   if(D.created() && _txtr)
   {
      UInt target;
      switch(mode())
      {
         case IMAGE_2D:
         case IMAGE_RT:
         case IMAGE_DS:
         case IMAGE_SHADOW_MAP:
            target=GL_TEXTURE_2D; break;

         case IMAGE_3D:
            target=GL_TEXTURE_3D; break;

         case IMAGE_CUBE:
         case IMAGE_RT_CUBE:
            target=GL_TEXTURE_CUBE_MAP; break;

         default: return;
      }
      D.texBind(target, _txtr);

      glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, _base_mip  );
      glTexParameteri(target, GL_TEXTURE_MAX_LEVEL , mipMaps()-1); // this is needed, without it images with mips=1 will fail to draw

   #if GL_SWIZZLE
      switch(hwType())
      {
         case IMAGE_A8:
         {
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_ZERO);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_ZERO);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_ZERO);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_RED );
         }break;

         case IMAGE_L8:
         {
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_RED);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_RED);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_RED);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_ONE);
         }break;

         case IMAGE_L8A8:
         {
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_RED  );
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_RED  );
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_RED  );
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_GREEN);
         }break;
      }
   #endif
   }
#endif
}
/******************************************************************************/
Bool Image::createEx(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps, Byte samples, CPtr *data, Int base_mip)
{
   // verify parameters
   if(w<=0 || h<=0 || d<=0 || type==IMAGE_NONE){del(); return !w && !h && !d;}

   if((d!=1 && mode!=IMAGE_SOFT && mode!=IMAGE_3D) // "d!=1" can be specified only for SOFT or 3D
   || !InRange(type, IMAGE_ALL_TYPES)
#if DX11
   || (samples!=1 && (mode==IMAGE_SOFT || mode==IMAGE_SOFT_CUBE || mode==IMAGE_3D)) // SOFT SOFT_CUBE 3D don't support multi-sampling
#else
   ||  samples!=1 // not yet implemented
#endif
   )goto error; // type out of range

   {
      // mip maps
      Int      total_mip_maps=TotalMipMaps(w, h, d); // don't use hardware texture size hwW(), hwH(), hwD(), so that number of mip-maps will always be the same (and not dependant on hardware capabilities like TexPow2 sizes), also because 1x1 image has just 1 mip map, but if we use padding then 4x4 block would generate 3 mip maps
      if(mip_maps<=0)mip_maps=total_mip_maps ; // if mip maps not specified (or we want multiple mip maps with type that requires full chain) then use full chain
      else       MIN(mip_maps,total_mip_maps); // don't use more than maximum allowed
   #if GL
      if(mode==IMAGE_GL_RB)mip_maps=1;
   #endif

      // check if already matches what we want
      if(T.w()==w && T.h()==h && T.d()==d && T.type()==type && T.mode()==mode && T.mipMaps()==mip_maps && T.samples()==samples)
         if(!data || soft()) // only soft can keep existing members if we want to set data (HW images have to be created as new)
         #if IMAGE_STREAM_FULL
            if(!_stream) // in IMAGE_STREAM_FULL this image can be smaller when streaming, so we can't reuse it in that case
         #endif
      {
         cancelStream();
         baseMip(base_mip);
         unlock(); // unlock if was locked, because we expect 'createEx' to fully reset the state
         if(data)
         {
            // here have to calculate manually because image could have HW size different, due to 'adjustInfo'
            VecI data_hw_size(PaddedWidth(w, h, 0, type), PaddedHeight(w, h, 0, type), d);
            Int  faces=T.faces();
            REPD(m, mipMaps())
               if(Byte *mip_data=(Byte*)data[m])
            {
               Int data_pitch   =ImagePitch  (data_hw_size.x, data_hw_size.y, m, type),
                   data_blocks_y=ImageBlocksY(data_hw_size.x, data_hw_size.y, m, type),
                   data_pitch2  =data_pitch*data_blocks_y,
                   data_d       =Max(1, data_hw_size.z>>m),
                    img_d       =Max(1,          hwD()>>m);
               CopyImgData(mip_data, softData(m), data_pitch, softPitch(m), data_blocks_y, softBlocksY(m), data_pitch2, softPitch2(m), data_d, img_d, faces);
            }
         }
         return true;
      }

      // create as new
   #if WEB && GL
      Memt<Byte> temp;
   #endif

      const Bool hw=IsHW(mode);
   #if GPU_LOCK // lock not needed for 'D3D'
      SyncLockerEx locker(D._lock, hw);
   #endif
      if(hw && !D.created())goto error; // device not yet created, check this after lock

      // do a quick check, check this after 'D.created()'
      if(!ImageSupported(type, mode, samples))goto error;

      del();

      // create
     _size.set(w, h, d); _hw_size.set(PaddedWidth(w, h, 0, type), PaddedHeight(w, h, 0, type), d);
     _type   =type     ; _hw_type=type;
     _mode   =mode     ;
     _mips   =mip_maps ; _base_mip=base_mip;
     _samples=samples  ;

   #if DX11
      D3D11_SUBRESOURCE_DATA *initial_data; MemtN<D3D11_SUBRESOURCE_DATA, MAX_MIP_MAPS*6> res_data; // mip maps * faces
      if(hw)
      {
         initial_data=null;
         if(data) // always assumed to exactly match
         {
            Int faces=T.faces();
            initial_data=res_data.setNum(mipMaps()*faces).data();
            REPD(m, mipMaps())
            {
               Int   mip_pitch    =  softPitch  (m)           , // X
                     mip_pitch2   =  softBlocksY(m)*mip_pitch , // X*Y
                     mip_face_size=Max(1, hwD()>>m)*mip_pitch2; // X*Y*Z
             C Byte *mip_data     =(Byte*)data[m];
               FREPD(f, faces)
               {
                  D3D11_SUBRESOURCE_DATA &srd=initial_data[D3D11CalcSubresource(m, f, mipMaps())];
                  srd.pSysMem         =mip_data;
                  srd.SysMemPitch     =mip_pitch;
                  srd.SysMemSlicePitch=mip_pitch2;
                 if(mip_data)mip_data+=mip_face_size;
               }
            }
         }
      }
   #endif

      switch(mode)
      {
         case IMAGE_SOFT:
         case IMAGE_SOFT_CUBE:
            if(finalize())
         {
            Alloc(_data_all, memUsage());
            lockSoft(); // set default lock members to main mip map
            if(data)
            {
               Int faces=T.faces();
               REPD(m, mipMaps())
                  if(CPtr mip_data=data[m])
                     CopyFast(softData(m), mip_data, softFaceSize(m)*faces);
            }
            return true;
         }break;

      #if DX11
         case IMAGE_2D:
         {
            D3D11_TEXTURE2D_DESC desc; desc.Format=Typeless(type); if(desc.Format!=DXGI_FORMAT_UNKNOWN)
            {
               desc.Width             =hwW();
               desc.Height            =hwH();
               desc.MipLevels         =mipMaps();
               desc.Usage             =D3D11_USAGE_DEFAULT;
               desc.BindFlags         =D3D11_BIND_SHADER_RESOURCE;
               desc.MiscFlags         =0;
               desc.CPUAccessFlags    =0;
               desc.SampleDesc.Count  =samples;
               desc.SampleDesc.Quality=0;
               desc.ArraySize         =1;
               if(OK(D3D->CreateTexture2D(&desc, initial_data, (ID3D11Texture2D**)&_txtr)) && finalize())return true;
            }
         }break;

         case IMAGE_RT:
         {
            D3D11_TEXTURE2D_DESC desc; desc.Format=Typeless(type); if(desc.Format!=DXGI_FORMAT_UNKNOWN)
            {
               desc.Width             =hwW();
               desc.Height            =hwH();
               desc.MipLevels         =mipMaps();
               desc.Usage             =D3D11_USAGE_DEFAULT;
               desc.BindFlags         =D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE; if(samples<=1 && D.computeAvailable())desc.BindFlags|=D3D11_BIND_UNORDERED_ACCESS;
               desc.MiscFlags         =0;
               desc.CPUAccessFlags    =0;
               desc.SampleDesc.Count  =samples;
               desc.SampleDesc.Quality=0;
               desc.ArraySize         =1;
               if(OK(D3D->CreateTexture2D(&desc, initial_data, (ID3D11Texture2D**)&_txtr)) && finalize())return true;
            }
         }break;

         case IMAGE_3D:
         {
            D3D11_TEXTURE3D_DESC desc; desc.Format=Typeless(type); if(desc.Format!=DXGI_FORMAT_UNKNOWN)
            {
               desc.Width         =hwW();
               desc.Height        =hwH();
               desc.Depth         =hwD();
               desc.MipLevels     =mipMaps();
               desc.Usage         =D3D11_USAGE_DEFAULT;
               desc.BindFlags     =D3D11_BIND_SHADER_RESOURCE;
               desc.MiscFlags     =0;
               desc.CPUAccessFlags=0;
               if(OK(D3D->CreateTexture3D(&desc, initial_data, (ID3D11Texture3D**)&_txtr)) && finalize())return true;
            }
         }break;

         case IMAGE_CUBE:
         {
            D3D11_TEXTURE2D_DESC desc; desc.Format=Typeless(type); if(desc.Format!=DXGI_FORMAT_UNKNOWN)
            {
               desc.Width             =hwW();
               desc.Height            =hwH();
               desc.MipLevels         =mipMaps();
               desc.Usage             =D3D11_USAGE_DEFAULT;
               desc.BindFlags         =D3D11_BIND_SHADER_RESOURCE;
               desc.MiscFlags         =D3D11_RESOURCE_MISC_TEXTURECUBE;
               desc.CPUAccessFlags    =0;
               desc.SampleDesc.Count  =samples;
               desc.SampleDesc.Quality=0;
               desc.ArraySize         =6;
               if(OK(D3D->CreateTexture2D(&desc, initial_data, (ID3D11Texture2D**)&_txtr)) && finalize())return true;
            }
         }break;

         case IMAGE_RT_CUBE:
         {
            D3D11_TEXTURE2D_DESC desc; desc.Format=Typeless(type); if(desc.Format!=DXGI_FORMAT_UNKNOWN)
            {
               desc.Width             =hwW();
               desc.Height            =hwH();
               desc.MipLevels         =mipMaps();
               desc.Usage             =D3D11_USAGE_DEFAULT;
               desc.BindFlags         =D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
               desc.MiscFlags         =D3D11_RESOURCE_MISC_TEXTURECUBE;
               desc.CPUAccessFlags    =0;
               desc.SampleDesc.Count  =samples;
               desc.SampleDesc.Quality=0;
               desc.ArraySize         =6;
               if(OK(D3D->CreateTexture2D(&desc, initial_data, (ID3D11Texture2D**)&_txtr)) && finalize())return true;
            }
         }break;

         case IMAGE_DS:
         case IMAGE_SHADOW_MAP:
         {
            D3D11_TEXTURE2D_DESC desc; desc.Format=Typeless(type); if(desc.Format!=DXGI_FORMAT_UNKNOWN)
            {
               desc.Width             =hwW();
               desc.Height            =hwH();
               desc.MipLevels         =mipMaps();
               desc.Usage             =D3D11_USAGE_DEFAULT;
               desc.BindFlags         =D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE;
               desc.MiscFlags         =0;
               desc.CPUAccessFlags    =0;
               desc.SampleDesc.Count  =samples;
               desc.SampleDesc.Quality=0;
               desc.ArraySize         =1;
               if(OK(D3D->CreateTexture2D(&desc, initial_data, (ID3D11Texture2D**)&_txtr)) && finalize())return true;
               FlagDisable(desc.BindFlags, D3D11_BIND_SHADER_RESOURCE); // disable shader reading
               if(OK(D3D->CreateTexture2D(&desc, initial_data, (ID3D11Texture2D**)&_txtr)) && finalize())return true;
            }
         }break;

         case IMAGE_STAGING:
         {
            D3D11_TEXTURE2D_DESC desc; desc.Format=Typeless(type); if(desc.Format!=DXGI_FORMAT_UNKNOWN)
            {
               desc.Width             =hwW();
               desc.Height            =hwH();
               desc.MipLevels         =mipMaps();
               desc.Usage             =D3D11_USAGE_STAGING;
               desc.BindFlags         =0;
               desc.MiscFlags         =0;
               desc.CPUAccessFlags    =D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE;
               desc.SampleDesc.Count  =samples;
               desc.SampleDesc.Quality=0;
               desc.ArraySize         =1;
               if(OK(D3D->CreateTexture2D(&desc, initial_data, (ID3D11Texture2D**)&_txtr)) && finalize())return true;
            }
         }break;
      #elif GL
         case IMAGE_2D:
         {
            glGenTextures(1, &_txtr); if(_txtr)
            {
               glGetError (); // clear any previous errors
               setGLParams(); // call this first to bind the texture

               // !! WARNING: IF THIS IS REPLACED TO USE 'glTexStorage2D' THEN CAN'T USE 'glTexImage2D/glCompressedTexImage2D' TO UPDATE MIP MAPS IN UNLOCK AND 'lockedSetMipData', 'setFaceData' and .., check 'glTexSubImage' and compressed? !!

               UInt format=hwTypeInfo().format, gl_format=SourceGLFormat(hwType()), gl_type=SourceGLType(hwType());
               if(data)
               {
               #if GL_ES
                  if(!(App.flag&APP_AUTO_FREE_IMAGE_OPEN_GL_ES_DATA))Alloc(_data_all, CeilGL(memUsage())); Byte *dest=_data_all; // create a software copy only if we want it
               #endif
                  REPD(m, mipMaps()) // order important #MipOrder
                  {
                   //if(m==mipMaps()-2 && glGetError()!=GL_NO_ERROR)goto error; // check at the start 2nd mip-map to skip this when there's only one mip-map, if first mip failed, then fail #MipOrder
                     VecI2 mip_size(Max(1, hwW()>>m), Max(1, hwH()>>m));
                     Int       size=ImagePitch2(mip_size.x, mip_size.y, 0, hwType());
                     CPtr  mip_data=data[m];
                     if(!compressed())glTexImage2D(GL_TEXTURE_2D, m, format, mip_size.x, mip_size.y, 0, gl_format, gl_type, mip_data);
                     else   glCompressedTexImage2D(GL_TEXTURE_2D, m, format, mip_size.x, mip_size.y, 0,               size, mip_data);
                  #if GL_ES
                     if(dest){if(mip_data)CopyFast(dest, mip_data, size); dest+=size;}
                  #endif
                  }
               }else
               if(!compressed())glTexImage2D(GL_TEXTURE_2D, 0, format, hwW(), hwH(), 0, gl_format, gl_type, null);else
               {
                  Int size=ImagePitch2(hwW(), hwH(), 0, hwType());
               #if WEB // for WEB, null can't be specified in 'glCompressedTexImage*'
                  glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, hwW(), hwH(), 0, size, temp.setNumZero(CeilGL(size)).dataNull());
               #else
                  glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, hwW(), hwH(), 0, size, null);
               #endif
               }

               if(glGetError()==GL_NO_ERROR && finalize()) // ok
               {
                  glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
               #if GL_ES
                  if(!data)Alloc(_data_all, CeilGL(memUsage()));
               #endif
                  return true;
               }
            }
         }break;

         case IMAGE_RT: if(!compressed())
         {
            glGenTextures(1, &_txtr); if(_txtr)
            {
               glGetError (); // clear any previous errors
               setGLParams(); // call this first to bind the texture

               UInt format=hwTypeInfo().format;
               if(D.canSwapSRGB())
               {
                  glTexStorage2D(GL_TEXTURE_2D, mipMaps(), format, hwW(), hwH());
               }else
               {
                  UInt gl_format=SourceGLFormat(hwType()), gl_type=SourceGLType(hwType());
                  glTexImage2D(GL_TEXTURE_2D, 0, format, hwW(), hwH(), 0, gl_format, gl_type, null);
               }
               if(glGetError()==GL_NO_ERROR && finalize()) // ok
               {
                  glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
                  return true;
               }
            }
         }break;

         case IMAGE_3D:
         {
            glGenTextures(1, &_txtr); if(_txtr)
            {
               glGetError (); // clear any previous errors
               setGLParams(); // call this first to bind the texture

               UInt format=hwTypeInfo().format, gl_format=SourceGLFormat(hwType()), gl_type=SourceGLType(hwType());
               if(data)
               {
               #if GL_ES
                  if(!(App.flag&APP_AUTO_FREE_IMAGE_OPEN_GL_ES_DATA))Alloc(_data_all, CeilGL(memUsage())); Byte *dest=_data_all; // create a software copy only if we want it
               #endif
                  REPD(m, mipMaps()) // order important #MipOrder
                  {
                   //if(m==mipMaps()-2 && glGetError()!=GL_NO_ERROR)goto error; // check at the start 2nd mip-map to skip this when there's only one mip-map, if first mip failed, then fail #MipOrder
                     VecI  mip_size(Max(1, hwW()>>m), Max(1, hwH()>>m), Max(1, hwD()>>m));
                     Int       size=ImageFaceSize(mip_size.x, mip_size.y, mip_size.z, 0, hwType());
                     CPtr  mip_data=data[m];
                     if(!compressed())glTexImage3D(GL_TEXTURE_3D, m, format, mip_size.x, mip_size.y, mip_size.z, 0, gl_format, gl_type, mip_data);
                     else   glCompressedTexImage3D(GL_TEXTURE_3D, m, format, mip_size.x, mip_size.y, mip_size.z, 0,               size, mip_data);
                  #if GL_ES
                     if(dest){if(mip_data)CopyFast(dest, mip_data, size); dest+=size;}
                  #endif
                  }
               }else
               if(!compressed())glTexImage3D(GL_TEXTURE_3D, 0, format, hwW(), hwH(), hwD(), 0, gl_format, gl_type, null);
               else   glCompressedTexImage3D(GL_TEXTURE_3D, 0, format, hwW(), hwH(), hwD(), 0, ImageFaceSize(hwW(), hwH(), hwD(), 0, hwType()), null);

               if(glGetError()==GL_NO_ERROR && finalize()) // ok
               {
                  glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
               #if GL_ES
                  if(!data)Alloc(_data_all, CeilGL(memUsage()));
               #endif
            	   return true;
               }
            }
         }break;

         case IMAGE_CUBE:
         case IMAGE_RT_CUBE:
         {
            glGenTextures(1, &_txtr); if(_txtr)
            {
               glGetError (); // clear any previous errors
               setGLParams(); // call this first to bind the texture

               // set params which are set only at creation time, so they don't need to be set again later
               glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
               glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            #if GL_ES
               if(data && mode!=IMAGE_RT_CUBE && !(App.flag&APP_AUTO_FREE_IMAGE_OPEN_GL_ES_DATA))Alloc(_data_all, CeilGL(memUsage())); Byte *dest=_data_all; // create a software copy only if we want it
            #endif

               UInt format=hwTypeInfo().format, gl_format=SourceGLFormat(hwType()), gl_type=SourceGLType(hwType());
               Int  mip_maps=(data ? mipMaps() : 1); // when have src we need to initialize all mip-maps, without src we can just try 1 mip-map to see if it succeeds
            #if WEB // for WEB, null can't be specified in 'glCompressedTexImage*'
               Ptr  dummy=null; if(compressed())FREPD(m, mip_maps)if(!data || !data[m]){dummy=temp.setNumZero(CeilGL(ImagePitch2(hwW(), hwH(), m, hwType()))).dataNull(); break;} // order important, start from biggest mip, find first that doesn't have data specified
            #endif
               REPD(m, mip_maps) // order important #MipOrder
               {
                  VecI2 mip_size(Max(1, hwW()>>m), Max(1, hwH()>>m));
                  Int       size=ImagePitch2(mip_size.x, mip_size.y, 0, hwType());
                C Byte *mip_data=(data ? (Byte*)data[m] : null);
                  FREPD(f, 6) // faces, order important
                  {
                     if(!compressed())glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+f, m, format, mip_size.x, mip_size.y, 0, gl_format, gl_type, mip_data);
                  #if WEB
                     else   glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+f, m, format, mip_size.x, mip_size.y, 0,               size, mip_data ? mip_data : dummy);
                  #else
                     else   glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+f, m, format, mip_size.x, mip_size.y, 0,               size, mip_data);
                  #endif

                   //if(m==mip_maps-1 && !f && glGetError()!=GL_NO_ERROR)goto error; // check for error only on the first mip and face #MipOrder
                     
                  #if GL_ES
                     if(dest){if(mip_data)CopyFast(dest, mip_data, size); dest+=size;}
                  #endif
                     if(mip_data)mip_data+=size;
                  }
               }

               if(glGetError()==GL_NO_ERROR && finalize()) // ok
               {
                  glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
               #if GL_ES
                  if(mode!=IMAGE_RT_CUBE)
                  {
                     if(!data)Alloc(_data_all, CeilGL(memUsage()));
                  }
               #endif
            	   return true;
               }
            }
         }break;

         case IMAGE_DS:
         {
            glGenTextures(1, &_txtr); if(_txtr)
            {
               glGetError (); // clear any previous errors
               setGLParams(); // call this first to bind the texture

               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

               glTexImage2D(GL_TEXTURE_2D, 0, hwTypeInfo().format, hwW(), hwH(), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), null);

               if(glGetError()==GL_NO_ERROR && finalize())return true; // ok
            }
         }break;

         case IMAGE_SHADOW_MAP:
         {
            glGenTextures(1, &_txtr); if(_txtr)
            {
               glGetError (); // clear any previous errors
               setGLParams(); // call this first to bind the texture

               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S      , GL_CLAMP_TO_EDGE);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T      , GL_CLAMP_TO_EDGE);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER  , GL_LINEAR);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER  , GL_LINEAR);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, REVERSE_DEPTH ? GL_GEQUAL : GL_LEQUAL);

               glTexImage2D(GL_TEXTURE_2D, 0, hwTypeInfo().format, hwW(), hwH(), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), null);

               if(glGetError()==GL_NO_ERROR && finalize())return true; // ok
            }
         }break;

         case IMAGE_GL_RB:
         {
            glGenRenderbuffers(1, &_rb); if(_rb)
            {
               glGetError(); // clear any previous errors

               glBindRenderbuffer   (GL_RENDERBUFFER, _rb);
               glRenderbufferStorage(GL_RENDERBUFFER, hwTypeInfo().format, hwW(), hwH());

               if(glGetError()==GL_NO_ERROR && finalize())return true; // ok
            }
         }break;
      #endif
      }
   }
error:
   del(); return false;
}
Bool Image::createTry(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps, Bool alt_type_on_fail)
{
   if(createEx(w, h, d, type, mode, mip_maps, 1))return true;

   if(alt_type_on_fail && w>0 && h>0 && d>0)
      for(IMAGE_TYPE alt_type=type; alt_type=ImageTypeOnFail(alt_type); )
         if(createEx(w, h, d, alt_type, mode, mip_maps, 1))
   {
      adjustInfo(w, h, d, type);
      return true;
   }

   return false;
}
Image& Image::mustCreate(Int w, Int h, Int d, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps, Bool alt_type_on_fail)
{
   if(!createTry(w, h, d, type, mode, mip_maps, alt_type_on_fail))Exit(MLT(S+"Can't create Image "        +w+'x'+h+'x'+d+", type "+ImageTI[type].name+", mode "+mode+".",
                                                                       PL,S+u"Nie można utworzyć obrazka "+w+'x'+h+'x'+d+", typ " +ImageTI[type].name+", tryb "+mode+"."));
   return T;
}
/******************************************************************************/
// COPY
/******************************************************************************/
static Bool Decompress(C Image &src, Image &dest, Int max_mip_maps=INT_MAX) // assumes that 'src' and 'dest' are 2 different objects, 'src' is compressed, and 'dest' not compressed or not yet created, this always ignores gamma
{
   void (*decompress_block       )(C Byte *b, Color  (&block)[4][4])=null, (*decompress_block_pitch       )(C Byte *b, Color  *dest, Int pitch)=null;
   void (*decompress_block_SByte )(C Byte *b, SByte  (&block)[4][4])=null, (*decompress_block_pitch_SByte )(C Byte *b, SByte  *dest, Int pitch)=null;
   void (*decompress_block_VecSB2)(C Byte *b, VecSB2 (&block)[4][4])=null, (*decompress_block_pitch_VecSB2)(C Byte *b, VecSB2 *dest, Int pitch)=null;
   void (*decompress_block_VecH  )(C Byte *b, VecH   (&block)[4][4])=null, (*decompress_block_pitch_VecH  )(C Byte *b, VecH   *dest, Int pitch)=null;
   switch(src.hwType())
   {
      default: return false;

      case IMAGE_BC1         : case IMAGE_BC1_SRGB       : decompress_block       =DecompressBlockBC1      ; decompress_block_pitch       =DecompressBlockBC1      ; break;
      case IMAGE_BC2         : case IMAGE_BC2_SRGB       : decompress_block       =DecompressBlockBC2      ; decompress_block_pitch       =DecompressBlockBC2      ; break;
      case IMAGE_BC3         : case IMAGE_BC3_SRGB       : decompress_block       =DecompressBlockBC3      ; decompress_block_pitch       =DecompressBlockBC3      ; break;
      case IMAGE_BC4         :                             decompress_block       =DecompressBlockBC4      ; decompress_block_pitch       =DecompressBlockBC4      ; break;
      case IMAGE_BC4_SIGN    :                             decompress_block_SByte =DecompressBlockBC4S     ; decompress_block_pitch_SByte =DecompressBlockBC4S     ; break;
      case IMAGE_BC5         :                             decompress_block       =DecompressBlockBC5      ; decompress_block_pitch       =DecompressBlockBC5      ; break;
      case IMAGE_BC5_SIGN    :                             decompress_block_VecSB2=DecompressBlockBC5S     ; decompress_block_pitch_VecSB2=DecompressBlockBC5S     ; break;
      case IMAGE_BC6         :                             decompress_block_VecH  =DecompressBlockBC6      ; decompress_block_pitch_VecH  =DecompressBlockBC6      ; break;
      case IMAGE_BC7         : case IMAGE_BC7_SRGB       : decompress_block       =DecompressBlockBC7      ; decompress_block_pitch       =DecompressBlockBC7      ; break;
      case IMAGE_ETC1        :                             decompress_block       =DecompressBlockETC1     ; decompress_block_pitch       =DecompressBlockETC1     ; break;
      case IMAGE_ETC2_R      :                             decompress_block       =DecompressBlockETC2R    ; decompress_block_pitch       =DecompressBlockETC2R    ; break;
      case IMAGE_ETC2_R_SIGN :                             decompress_block_SByte =DecompressBlockETC2RS   ; decompress_block_pitch_SByte =DecompressBlockETC2RS   ; break;
      case IMAGE_ETC2_RG     :                             decompress_block       =DecompressBlockETC2RG   ; decompress_block_pitch       =DecompressBlockETC2RG   ; break;
      case IMAGE_ETC2_RG_SIGN:                             decompress_block_VecSB2=DecompressBlockETC2RGS  ; decompress_block_pitch_VecSB2=DecompressBlockETC2RGS  ; break;
      case IMAGE_ETC2_RGB    : case IMAGE_ETC2_RGB_SRGB  : decompress_block       =DecompressBlockETC2RGB  ; decompress_block_pitch       =DecompressBlockETC2RGB  ; break;
      case IMAGE_ETC2_RGBA1  : case IMAGE_ETC2_RGBA1_SRGB: decompress_block       =DecompressBlockETC2RGBA1; decompress_block_pitch       =DecompressBlockETC2RGBA1; break;
      case IMAGE_ETC2_RGBA   : case IMAGE_ETC2_RGBA_SRGB : decompress_block       =DecompressBlockETC2RGBA ; decompress_block_pitch       =DecompressBlockETC2RGBA ; break;

      case IMAGE_PVRTC1_2: case IMAGE_PVRTC1_2_SRGB:
      case IMAGE_PVRTC1_4: case IMAGE_PVRTC1_4_SRGB:
         return DecompressPVRTC(src, dest, max_mip_maps);

      case IMAGE_ASTC_4x4: case IMAGE_ASTC_4x4_SRGB:
      case IMAGE_ASTC_5x4: case IMAGE_ASTC_5x4_SRGB:
      case IMAGE_ASTC_5x5: case IMAGE_ASTC_5x5_SRGB:
      case IMAGE_ASTC_6x5: case IMAGE_ASTC_6x5_SRGB:
      case IMAGE_ASTC_6x6: case IMAGE_ASTC_6x6_SRGB:
      case IMAGE_ASTC_8x5: case IMAGE_ASTC_8x5_SRGB:
      case IMAGE_ASTC_8x6: case IMAGE_ASTC_8x6_SRGB:
      case IMAGE_ASTC_8x8: case IMAGE_ASTC_8x8_SRGB:
         return DecompressASTC(src, dest, max_mip_maps);
   }
   if(dest.is() || dest.createTry(src.w(), src.h(), src.d(), decompress_block        ? (src.sRGB() ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8A8) // use 'IMAGE_R8G8B8A8'  because Decompress Block functions operate on 'Color'
                                                           : decompress_block_SByte  ?               IMAGE_R8_SIGN                         // use 'IMAGE_R8_SIGN'   because Decompress Block functions operate on 'SByte'
                                                           : decompress_block_VecSB2 ?               IMAGE_R8G8_SIGN                       // use 'IMAGE_R8G8_SIGN' because Decompress Block functions operate on 'VecSB2'
                                                           : decompress_block_VecH   ?               IMAGE_F16_3                           // use 'IMAGE_F16_3'     because Decompress Block functions operate on 'VecH'
                                                           :                                         IMAGE_NONE, AsSoft(src.mode()), src.mipMaps()))
      if(dest.size3()==src.size3())
   {
      Int src_faces1=src.faces()-1,
          x_mul     =src.hwTypeInfo().block_bytes;
      REPD(mip , Min(src.mipMaps(), dest.mipMaps(), max_mip_maps))
      REPD(face, dest.faces())
      {
         if(! src.lockRead(            mip, (DIR_ENUM)Min(face, src_faces1)))return false;
         if(!dest.lock    (LOCK_WRITE, mip, (DIR_ENUM)    face             )){src.unlock(); return false;}

         Int full_blocks_x=         dest.lw()/4,
             full_blocks_y=         dest.lh()/4,
              all_blocks_x=DivCeil4(dest.lw()),
              all_blocks_y=DivCeil4(dest.lh());

         REPD(z, dest.ld())
         {
            Int done_x=0, done_y=0;
            if(decompress_block)
            {
               Color color[4][4];
               if((dest.hwType()==IMAGE_R8G8B8A8 || dest.hwType()==IMAGE_R8G8B8A8_SRGB)  // decompress directly into 'dest'
               && (dest.  type()==IMAGE_R8G8B8A8 || dest.  type()==IMAGE_R8G8B8A8_SRGB)) // check 'type' too in case we have to perform color adjustment
               {
                  // process full blocks only
                C Byte * src_data_z= src.data() + z* src.pitch2();
                  Byte *dest_data_z=dest.data() + z*dest.pitch2();
                  REPD(by, full_blocks_y)
                  {
                     const Int py=by*4; // pixel
                   C Byte * src_data_y= src_data_z + by* src.pitch();
                     Byte *dest_data_y=dest_data_z + py*dest.pitch();
                     REPD(bx, full_blocks_x)
                     {
                        const Int px=bx*4; // pixel
                        decompress_block_pitch(src_data_y + bx*x_mul, (Color*)(dest_data_y + px*SIZE(Color)), dest.pitch());
                     }
                  }
                  done_x=full_blocks_x;
                  done_y=full_blocks_y;
               }

               // process right blocks (excluding the shared corner)
               for(Int by=     0; by<done_y      ; by++)
               for(Int bx=done_x; bx<all_blocks_x; bx++)
               {
                  Int px=bx*4, py=by*4; // pixel
                  decompress_block(src.data() + bx*x_mul + by*src.pitch() + z*src.pitch2(), color);
                  REPD(y, 4)
                  REPD(x, 4)dest.color3D(px+x, py+y, z, color[y][x]);
               }

               // process bottom blocks (including the shared corner)
               for(Int by=done_y; by<all_blocks_y; by++)
               for(Int bx=     0; bx<all_blocks_x; bx++)
               {
                  Int px=bx*4, py=by*4; // pixel
                  decompress_block(src.data() + bx*x_mul + by*src.pitch() + z*src.pitch2(), color);
                  REPD(y, 4)
                  REPD(x, 4)dest.color3D(px+x, py+y, z, color[y][x]);
               }
            }else
            if(decompress_block_SByte)
            {
               SByte color[4][4];
               if(dest.hwType()==IMAGE_R8_SIGN  // decompress directly into 'dest'
               && dest.  type()==IMAGE_R8_SIGN) // check 'type' too in case we have to perform color adjustment
               {
                  // process full blocks only
                C Byte * src_data_z= src.data() + z* src.pitch2();
                  Byte *dest_data_z=dest.data() + z*dest.pitch2();
                  REPD(by, full_blocks_y)
                  {
                     const Int py=by*4; // pixel
                   C Byte * src_data_y= src_data_z + by* src.pitch();
                     Byte *dest_data_y=dest_data_z + py*dest.pitch();
                     REPD(bx, full_blocks_x)
                     {
                        const Int px=bx*4; // pixel
                        decompress_block_pitch_SByte(src_data_y + bx*x_mul, (SByte*)(dest_data_y + px*SIZE(SByte)), dest.pitch());
                     }
                  }
                  done_x=full_blocks_x;
                  done_y=full_blocks_y;
               }

               Vec4 color4; color4.y=0; color4.z=0; color4.w=1;

               // process right blocks (excluding the shared corner)
               for(Int by=     0; by<done_y      ; by++)
               for(Int bx=done_x; bx<all_blocks_x; bx++)
               {
                  Int px=bx*4, py=by*4; // pixel
                  decompress_block_SByte(src.data() + bx*x_mul + by*src.pitch() + z*src.pitch2(), color);
                  REPD(y, 4)
                  REPD(x, 4){color4.x=SByteToSFlt(color[y][x]); dest.color3DF(px+x, py+y, z, color4);}
               }

               // process bottom blocks (including the shared corner)
               for(Int by=done_y; by<all_blocks_y; by++)
               for(Int bx=     0; bx<all_blocks_x; bx++)
               {
                  Int px=bx*4, py=by*4; // pixel
                  decompress_block_SByte(src.data() + bx*x_mul + by*src.pitch() + z*src.pitch2(), color);
                  REPD(y, 4)
                  REPD(x, 4){color4.x=SByteToSFlt(color[y][x]); dest.color3DF(px+x, py+y, z, color4);}
               }
            }else
            if(decompress_block_VecSB2)
            {
               VecSB2 color[4][4];
               if(dest.hwType()==IMAGE_R8G8_SIGN  // decompress directly into 'dest'
               && dest.  type()==IMAGE_R8G8_SIGN) // check 'type' too in case we have to perform color adjustment
               {
                  // process full blocks only
                C Byte * src_data_z= src.data() + z* src.pitch2();
                  Byte *dest_data_z=dest.data() + z*dest.pitch2();
                  REPD(by, full_blocks_y)
                  {
                     const Int py=by*4; // pixel
                   C Byte * src_data_y= src_data_z + by* src.pitch();
                     Byte *dest_data_y=dest_data_z + py*dest.pitch();
                     REPD(bx, full_blocks_x)
                     {
                        const Int px=bx*4; // pixel
                        decompress_block_pitch_VecSB2(src_data_y + bx*x_mul, (VecSB2*)(dest_data_y + px*SIZE(VecSB2)), dest.pitch());
                     }
                  }
                  done_x=full_blocks_x;
                  done_y=full_blocks_y;
               }

               Vec4 color4; color4.z=0; color4.w=1;

               // process right blocks (excluding the shared corner)
               for(Int by=     0; by<done_y      ; by++)
               for(Int bx=done_x; bx<all_blocks_x; bx++)
               {
                  Int px=bx*4, py=by*4; // pixel
                  decompress_block_VecSB2(src.data() + bx*x_mul + by*src.pitch() + z*src.pitch2(), color);
                  REPD(y, 4)
                  REPD(x, 4){color4.xy.set(SByteToSFlt(color[y][x].x), SByteToSFlt(color[y][x].y)); dest.color3DF(px+x, py+y, z, color4);}
               }

               // process bottom blocks (including the shared corner)
               for(Int by=done_y; by<all_blocks_y; by++)
               for(Int bx=     0; bx<all_blocks_x; bx++)
               {
                  Int px=bx*4, py=by*4; // pixel
                  decompress_block_VecSB2(src.data() + bx*x_mul + by*src.pitch() + z*src.pitch2(), color);
                  REPD(y, 4)
                  REPD(x, 4){color4.xy.set(SByteToSFlt(color[y][x].x), SByteToSFlt(color[y][x].y)); dest.color3DF(px+x, py+y, z, color4);}
               }
            }else
            if(decompress_block_VecH)
            {
               VecH color[4][4];
               if(dest.hwType()==IMAGE_F16_3  // decompress directly into 'dest'
               && dest.  type()==IMAGE_F16_3) // check 'type' too in case we have to perform color adjustment
               {
                  // process full blocks only
                C Byte * src_data_z= src.data() + z* src.pitch2();
                  Byte *dest_data_z=dest.data() + z*dest.pitch2();
                  REPD(by, full_blocks_y)
                  {
                     const Int py=by*4; // pixel
                   C Byte * src_data_y= src_data_z + by* src.pitch();
                     Byte *dest_data_y=dest_data_z + py*dest.pitch();
                     REPD(bx, full_blocks_x)
                     {
                        const Int px=bx*4; // pixel
                        decompress_block_pitch_VecH(src_data_y + bx*x_mul, (VecH*)(dest_data_y + px*SIZE(VecH)), dest.pitch());
                     }
                  }
                  done_x=full_blocks_x;
                  done_y=full_blocks_y;
               }

               Vec4 color4; color4.w=1;

               // process right blocks (excluding the shared corner)
               for(Int by=     0; by<done_y      ; by++)
               for(Int bx=done_x; bx<all_blocks_x; bx++)
               {
                  Int px=bx*4, py=by*4; // pixel
                  decompress_block_VecH(src.data() + bx*x_mul + by*src.pitch() + z*src.pitch2(), color);
                  REPD(y, 4)
                  REPD(x, 4){color4.xyz=color[y][x]; dest.color3DF(px+x, py+y, z, color4);}
               }

               // process bottom blocks (including the shared corner)
               for(Int by=done_y; by<all_blocks_y; by++)
               for(Int bx=     0; bx<all_blocks_x; bx++)
               {
                  Int px=bx*4, py=by*4; // pixel
                  decompress_block_VecH(src.data() + bx*x_mul + by*src.pitch() + z*src.pitch2(), color);
                  REPD(y, 4)
                  REPD(x, 4){color4.xyz=color[y][x]; dest.color3DF(px+x, py+y, z, color4);}
               }
            }
         }
         dest.unlock();
          src.unlock();
      }
      return true;
   }
   return false;
}
/******************************************************************************/
static Bool Compress(C Image &src, Image &dest) // assumes that 'src' and 'dest' are 2 different objects, 'src' is created as non-compressed, and 'dest' is created as compressed, they have the same 'size3'
{
   switch(dest.hwType())
   {
      case IMAGE_BC1: case IMAGE_BC1_SRGB:
      case IMAGE_BC2: case IMAGE_BC2_SRGB:
      case IMAGE_BC3: case IMAGE_BC3_SRGB:
      case IMAGE_BC4: case IMAGE_BC4_SIGN:
      case IMAGE_BC5: case IMAGE_BC5_SIGN:
         return CompressBC(src, dest);

      case IMAGE_BC6:
      case IMAGE_BC7: case IMAGE_BC7_SRGB:
         DEBUG_ASSERT(CompressBC67, "'SupportCompressBC/SupportCompressAll' was not called"); return CompressBC67 ? CompressBC67(src, dest) : false;

      case IMAGE_ETC1      :
      case IMAGE_ETC2_R    : case IMAGE_ETC2_R_SIGN    :
      case IMAGE_ETC2_RG   : case IMAGE_ETC2_RG_SIGN   :
      case IMAGE_ETC2_RGB  : case IMAGE_ETC2_RGB_SRGB  :
      case IMAGE_ETC2_RGBA1: case IMAGE_ETC2_RGBA1_SRGB:
      case IMAGE_ETC2_RGBA : case IMAGE_ETC2_RGBA_SRGB :
         DEBUG_ASSERT(CompressETC, "'SupportCompressETC/SupportCompressAll' was not called"); return CompressETC ? CompressETC(src, dest, -1) : false;

      case IMAGE_PVRTC1_2: case IMAGE_PVRTC1_2_SRGB:
      case IMAGE_PVRTC1_4: case IMAGE_PVRTC1_4_SRGB:
         DEBUG_ASSERT(CompressPVRTC, "'SupportCompressPVRTC/SupportCompressAll' was not called"); return CompressPVRTC ? CompressPVRTC(src, dest, -1) : false;

      case IMAGE_ASTC_4x4: case IMAGE_ASTC_4x4_SRGB:
      case IMAGE_ASTC_5x4: case IMAGE_ASTC_5x4_SRGB:
      case IMAGE_ASTC_5x5: case IMAGE_ASTC_5x5_SRGB:
      case IMAGE_ASTC_6x5: case IMAGE_ASTC_6x5_SRGB:
      case IMAGE_ASTC_6x6: case IMAGE_ASTC_6x6_SRGB:
      case IMAGE_ASTC_8x5: case IMAGE_ASTC_8x5_SRGB:
      case IMAGE_ASTC_8x6: case IMAGE_ASTC_8x6_SRGB:
      case IMAGE_ASTC_8x8: case IMAGE_ASTC_8x8_SRGB:
         DEBUG_ASSERT(CompressASTC, "'SupportCompressASTC/SupportCompressAll' was not called"); return CompressASTC ? CompressASTC(src, dest) : false;
   }
   return false;
}
/******************************************************************************/
static Int CopyMipMaps(C Image &src, Image &dest, Bool ignore_gamma, Int max_mip_maps=INT_MAX) // this assumes that "&src != &dest", returns how many mip-maps were copied
{
   if(CanDoRawCopy(src, dest, ignore_gamma)
   && dest.w()<=src.w()
   && dest.h()<=src.h()
   && dest.d()<=src.d())
      FREP(src.mipMaps()) // find location of first 'dest' mip-map in 'src'
   {
      Int src_mip_w=Max(1, src.w()>>i),
          src_mip_h=Max(1, src.h()>>i),
          src_mip_d=Max(1, src.d()>>i);
      if( src_mip_w==dest.w() && src_mip_h==dest.h() && src_mip_d==dest.d()) // i-th 'src' mip-map is the same as first 'dest' mip-map
      {
         Int mip_maps  =Min(src.mipMaps()-i, dest.mipMaps(), max_mip_maps), // how many to copy
            dest_faces =dest.faces(),
             src_faces1= src.faces()-1;
         FREPD(mip ,  mip_maps )
         FREPD(face, dest_faces)
         {
            if(!src .lockRead(          i+mip, (DIR_ENUM)Min(face, src_faces1)))return 0;
            if(!dest.lock    (LOCK_WRITE, mip, (DIR_ENUM)    face             )){src.unlock(); return 0;}

            CopyImgData(src.data(), dest.data(), src.pitch(), dest.pitch(), src.softBlocksY(i+mip), dest.softBlocksY(mip), src.pitch2(), dest.pitch2(), src.ld(), dest.ld());

            dest.unlock();
            src .unlock();
         }
         return mip_maps;
      }
   }
   return 0;
}
/******************************************************************************/
static Bool BlurCubeMipMaps(Image &src, Image &dest, Int type, Int mode, FILTER_TYPE filter, UInt flags)
{
   return src.blurCubeMipMaps()
       && src.copyTry(dest, -1, -1, -1, IMAGE_TYPE(type), IMAGE_MODE(mode), -1, filter, flags&~IC_ENV_CUBE); // disable IC_ENV_CUBE because we've already blurred mip-maps
}
/******************************************************************************/
Bool Image::copyTry(Image &dest, Int w, Int h, Int d, Int type, Int mode, Int mip_maps, FILTER_TYPE filter, UInt flags)C
{
   if(!is()){dest.del(); return true;}

   // adjust settings
   if(type<=0)type=T.type(); // get type before 'fromCube' because it may change it
   if(mode< 0)mode=T.mode();

 C Image *src=this; Image temp_src;
   if(src->cube() && !IsCube(IMAGE_MODE(mode))) // if converting from cube to non-cube
   {
      if(temp_src.fromCube(*src))src=&temp_src;else return false; // automatically convert to 6x1 images
   }else
   if(!src->cube() && IsCube((IMAGE_MODE)mode)) // if converting from non-cube to cube
   {
      return dest.toCube(*src, -1, (w>0) ? w : h, type, mode, mip_maps, filter, flags);
   }

   // calculate dimensions after cube conversion
   if(w<=0)w=src->w();
   if(h<=0)h=src->h();
   if(d<=0)d=src->d();

   // mip maps
   if(mip_maps<0)
   {
      if(w==src->w() && h==src->h() && d==src->d()                )mip_maps=src->mipMaps();else // same size
      if(src->mipMaps()<TotalMipMaps(src->w(), src->h(), src->d()))mip_maps=src->mipMaps();else // less than total
      if(src->mipMaps()==1                                        )mip_maps=             1;else // use  only one
                                                                   mip_maps=             0;     // auto-detect mip maps
   }
   Int dest_total_mip_maps=TotalMipMaps(w, h, d);
   if(mip_maps<=0)mip_maps=dest_total_mip_maps ; // if mip maps not specified then use full chain
   else       MIN(mip_maps,dest_total_mip_maps); // don't use more than maximum allowed

   Bool alt_type_on_fail=FlagOff(flags, IC_NO_ALT_TYPE),
        env=(FlagOn(flags, IC_ENV_CUBE) && IsCube((IMAGE_MODE)mode) && mip_maps>1);

   // check if doesn't require conversion
   if(this==&dest && w==T.w() && h==T.h() && d==T.d() && mode==T.mode() && mip_maps==T.mipMaps() && !env) // here check 'T' instead of 'src' (which could've already encountered some cube conversion, however here we want to check if we can just return without doing any conversions at all)
   {
      if(type==T.type())return true;
      if(soft() && CanDoRawCopy(T.hwType(), (IMAGE_TYPE)type, IgnoreGamma(flags, T.hwType(), IMAGE_TYPE(type))))
         {dest._hw_type=dest._type=(IMAGE_TYPE)type; return true;} // if software and can do a raw copy, then just adjust types
   }

   // convert
   {
      // create destination
      Image temp_dest, &target=((src==&dest) ? temp_dest : dest);
      if(!target.createTry(w, h, d, env ? ImageTypeUncompressed(IMAGE_TYPE(type)) : IMAGE_TYPE(type), env ? IMAGE_SOFT_CUBE : IMAGE_MODE(mode), mip_maps, alt_type_on_fail))return false; // 'env'/'blurCubeMipMaps' requires uncompressed/soft image
      Bool ignore_gamma=IgnoreGamma(flags, src->hwType(), target.hwType()); // calculate after knowing 'target.hwType'

      // copy
      Int  copied_mip_maps=0,
              max_mip_maps=(env ? 1 : INT_MAX); // for 'env' copy only the first mip map, the rest will be blurred manually
      Bool same_size=(src->size3()==target.size3());
      if(same_size // if we use the same size (for which case 'filter' and IC_KEEP_EDGES are ignored)
      || (
            (filter==FILTER_BEST || filter==FILTER_DOWN || filter==FILTER_WAIFU) // we're going to use default filter for downsampling (which is typically used for mip-map generation), FILTER_WAIFU falls back to FILTER_BEST
         && FlagOff(flags, IC_KEEP_EDGES)                                        // we're not keeping the edges                        (which is typically used for mip-map generation)
         && !env                                                                 // for 'env' allow copying only if we have same size, so we can copy only first mip-map (in case others are blurred specially)
         )
      )copied_mip_maps=CopyMipMaps(*src, target, ignore_gamma, max_mip_maps);
      if(!copied_mip_maps)
      {
      /* This case is already handled in 'CopyMipMaps'
         if(same_size && CanDoRawCopy(*src, target, ignore_gamma)) // if match in size and hardware type
         {
            if(!src->copySoft(target, FILTER_NONE, clamp, alpha_weight, keep_edges))return false; // do raw memory copy
            copied_mip_maps=src->mipMaps();
         }else*/
         {
            // decompress
            Image decompressed_src; if(src->compressed())
            {
               if(same_size && ignore_gamma && !target.compressed()) // if match in size, can ignore gamma and just want to be decompressed
               {
                  copied_mip_maps=(env ? 1 : src->mipMaps());
                  if(!Decompress(*src, target, copied_mip_maps))return false; // decompress directly to 'target'
                  goto finish;
               }
               if(!Decompress(*src, decompressed_src, max_mip_maps))return false; src=&decompressed_src; // take decompressed source
            }
            if(target.compressed()) // target wants to be compressed (always false for 'env')
            {
               Image resized_src;
               if(!same_size) // resize needed
               {
                  if(!resized_src.createTry(target.w(), target.h(), target.d(), src->hwType(), (src->cube() && target.cube()) ? IMAGE_SOFT_CUBE : IMAGE_SOFT, 1))return false; // for resize use only 1 mip map, and remaining set with 'updateMipMaps' below
                  if(!src->copySoft(resized_src, filter, flags))return false; src=&resized_src; decompressed_src.del(); // we don't need 'decompressed_src' anymore so delete it to release memory
               }
               if(!Compress(*src, target))return false;
               // in this case we have to use last 'src' mip map as the base mip map to set remaining 'target' mip maps, because now 'target' is compressed and has lower quality, while 'src' has better, this is also faster because we don't have to decompress initial mip map
               Int mip_start=src->mipMaps()-1;
               target.updateMipMaps(*src, mip_start, FILTER_BEST, flags, mip_start);
               goto skip_mip_maps; // we've already set mip maps, so skip them
            }else
            {
               copied_mip_maps=((same_size && !env) ? src->mipMaps() : 1); // if resize is needed, copy/resize only 1 mip map, and remaining set with 'updateMipMaps' below
               if(!src->copySoft(target, filter, flags, copied_mip_maps))return false;
            }
            // !! can't access 'src' here because it may point to 'resized_src' which is now invalid !!
         }
         // !! can't access 'src' here because it may point to 'decompressed_src, resized_src' which are now invalid !!
      }
   finish:
      if(env)return BlurCubeMipMaps(target, dest, type, mode, filter, flags);
      target.updateMipMaps(FILTER_BEST, flags, copied_mip_maps-1);
   skip_mip_maps:
      if(&target!=&dest)Swap(dest, target);
      return true;
   }
}
void Image::mustCopy(Image &dest, Int w, Int h, Int d, Int type, Int mode, Int mip_maps, FILTER_TYPE filter, UInt flags)C
{
   if(!copyTry(dest, w, h, d, type, mode, mip_maps, filter, flags))
   {
      Str s="Can't copy Image";
      if(!CanCompress((IMAGE_TYPE)type))s+="\n'SupportCompress*' function was not called";
      Exit(s);
   }
}
/******************************************************************************/
CUBE_LAYOUT Image::cubeLayout()C
{
   const Flt one_aspect=1.0f, cross_aspect=4.0f/3, _6x_aspect=6.0f;
   Flt aspect=T.aspect();
   if( aspect>Avg(cross_aspect,   _6x_aspect))return CUBE_LAYOUT_6x1;
 //if( aspect>Avg(  one_aspect, cross_aspect))return CUBE_LAYOUT_CROSS; TODO: not yet supported
                                              return CUBE_LAYOUT_ONE;
}
Bool Image::toCube(C Image &src, Int layout, Int size, Int type, Int mode, Int mip_maps, FILTER_TYPE filter, UInt flags)
{
   if(!src.cube() && src.is())
   {
      if(type  <=0                )type    =src.type      ();
      if(layout< 0                )layout  =src.cubeLayout();
      if(size  < 0                )size    =((layout==CUBE_LAYOUT_CROSS) ? src.w()/4 : src.h());
      if(!IsCube(IMAGE_MODE(mode)))mode    =(IsSoft(src.mode()) ? IMAGE_SOFT_CUBE : IMAGE_CUBE);
      if(mip_maps<0               )mip_maps=((src.mipMaps()==1) ? 1 : 0); // if source has 1 mip map, then create only 1, else create full

      Int dest_total_mip_maps=TotalMipMaps(size, size, 1);
      if(mip_maps<=0)mip_maps=dest_total_mip_maps ; // if mip maps not specified then use full chain
      else       MIN(mip_maps,dest_total_mip_maps); // don't use more than maximum allowed

      Bool  alt_type_on_fail=FlagOff(flags, IC_NO_ALT_TYPE),
            env=(FlagOn(flags, IC_ENV_CUBE) && mip_maps>1);
      Image temp; if(temp.createTry(size, size, 1, env ? ImageTypeUncompressed(IMAGE_TYPE(type)) : IMAGE_TYPE(type), env ? IMAGE_SOFT_CUBE : IMAGE_MODE(mode), mip_maps, alt_type_on_fail)) // 'env'/'blurCubeMipMaps' requires uncompressed/soft image
      {
         if(layout==CUBE_LAYOUT_ONE)
         {
            REP(6)if(!temp.injectMipMap(src, 0, DIR_ENUM(i), filter, flags))return false;
         }else
         {
          C Image *s=&src;
            Image  decompressed; if(src.compressed()){if(!src.copyTry(decompressed, -1, -1, -1, ImageTypeUncompressed(src.type()), IMAGE_SOFT, 1))return false; s=&decompressed;}
            Image  face; // keep outside the loop in case we can reuse it
            REP(6)
            {
               s->crop(face, i*s->w()/6, 0, s->w()/6, s->h());

               DIR_ENUM cube_face;
               switch(i)
               {
                  case 0: cube_face=DIR_LEFT   ; break;
                  case 1: cube_face=DIR_FORWARD; break;
                  case 2: cube_face=DIR_RIGHT  ; break;
                  case 3: cube_face=DIR_BACK   ; break;
                  case 4: cube_face=DIR_DOWN   ; break;
                  case 5: cube_face=DIR_UP     ; break;
               }
               if(!temp.injectMipMap(face, 0, cube_face, filter, flags))return false; // inject face
            }
         }
         if(env)return BlurCubeMipMaps(temp, T, type, mode, filter, flags);
         Swap(T, temp.updateMipMaps());
         return true;
      }
   }
   return false;
}
/******************************************************************************/
Bool Image::fromCube(C Image &src, Int uncompressed_type)
{
   if(src.cube())
   {
      IMAGE_TYPE type=src.hwType(); // choose 'hwType' (instead of 'type') because this method is written for speed, to avoid unnecesary conversions
      if(ImageTI[type].compressed) // a non-compressed type is needed, so we can easily copy each cube face into part of the 'temp' image (compressed types cannot be copied this way easily, example: PVRTC1 requires pow2 sizes and is very complex)
         type=((InRange(uncompressed_type, ImageTI) && !ImageTI[uncompressed_type].compressed) ? IMAGE_TYPE(uncompressed_type) : ImageTypeUncompressed(type));

      // extract 6 faces
      Int   size=src.h();
      Image temp; if(!temp.createTry(size*6, size, 1, type, IMAGE_SOFT, 1))return false;
      if(temp.lock(LOCK_WRITE))
      {
         Image face; // keep outside the loop in case we can reuse it
         REPD(f, 6)
         {
            DIR_ENUM cube_face;
            switch(f)
            {
               case 0: cube_face=DIR_LEFT   ; break;
               case 1: cube_face=DIR_FORWARD; break;
               case 2: cube_face=DIR_RIGHT  ; break;
               case 3: cube_face=DIR_BACK   ; break;
               case 4: cube_face=DIR_DOWN   ; break;
               case 5: cube_face=DIR_UP     ; break;
            }
            if(!src.extractMipMap(face, temp.hwType(), 0, cube_face))return false; // extract face, we need 'temp.hwType' so we can do fast copy to 'temp' below
            // copy non-compressed 2D face to non-compressed 6*2D
            if(!face.lockRead())return false;
            REPD(y, size)
            {
               CopyFast(temp.data() + y*temp.pitch() + f*temp.bytePP()*size,
                        face.data() + y*face.pitch()                       , face.bytePP()*size);
            }
            face.unlock();
         }

         temp.unlock();
       //if(!temp.copyTry(temp, -1, -1, -1, type, IMAGE_SOFT))return false; skip this step to avoid unnecessary operations, this method uses 'uncompressed_type' instead of 'type'
         Swap(T, temp); return true;
      }
   }
   return false;
}
/******************************************************************************/
// LOCK
/******************************************************************************/
UInt  Image::softPitch   (Int mip_map                    )C {return ImagePitch   (hwW(), hwH(),        mip_map, hwType());}
UInt  Image::softBlocksY (Int mip_map                    )C {return ImageBlocksY (hwW(), hwH(),        mip_map, hwType());}
UInt  Image::softPitch2  (Int mip_map                    )C {return ImagePitch2  (hwW(), hwH(),        mip_map, hwType());}
Int   Image::softFaceSize(Int mip_map                    )C {return ImageFaceSize(hwW(), hwH(), hwD(), mip_map, hwType());}
Int   Image::softMipSize (Int mip_map                    )C {return ImageFaceSize(hwW(), hwH(), hwD(), mip_map, hwType())*faces();}
Byte* Image::softData    (Int mip_map, DIR_ENUM cube_face)
{
   return _data_all+ImageMipOffset(hwW(), hwH(), hwD(), hwType(), mode(), mipMaps(), mip_map)+(cube_face ? softFaceSize(mip_map)*cube_face : 0); // call 'softFaceSize' only when needed because most likely 'cube_face' is zero
}
void Image::lockSoft()
{
  _pitch      =softPitch  (lMipMap());
  _pitch2     =softBlocksY(lMipMap())*_pitch;
  _lock_size.x=Max(1, w()>>lMipMap());
  _lock_size.y=Max(1, h()>>lMipMap());
  _lock_size.z=Max(1, d()>>lMipMap());
  _data       =softData(lMipMap(), lCubeFace());
}
Bool Image::lock(LOCK_MODE lock, Int mip_map, DIR_ENUM cube_face)
{
   if(InRange(mip_map, mipMaps()) && InRange(cube_face, 6)) // this already handles the case of "is()"
   {
   #if IMAGE_STREAM_FULL
      if(_stream&IMAGE_STREAM_NEED_MORE)
   #else
      if(mip_map<baseMip())
   #endif
         if(!waitForStream(mip_map))return false; // if want to access mip-map that's still streaming, then wait for it

      if(mode()==IMAGE_SOFT)
      {
         if(mipMaps()==1)return true; // if there's only one mip-map then we don't need to do anything
         SyncLocker locker(ImageSoftLock);
         if(!_lock_count) // if wasn't locked yet
         {
           _lock_count=1;
           _lock_mip  =mip_map; lockSoft(); // set '_lock_mip' before calling 'lockSoft'
            return true;
         }
         if(lMipMap()==mip_map){_lock_count++; return true;} // we want the same mip-map that's already locked
      }else
      if(mode()==IMAGE_SOFT_CUBE)
      {
         SyncLocker locker(ImageSoftLock);
         if(!_lock_count) // if wasn't locked yet
         {
           _lock_count=1;
           _lock_mip  =mip_map; _lock_face=cube_face; lockSoft(); // set '_lock_mip', '_lock_face' before calling 'lockSoft'
            return true;
         }
         if(lMipMap()==mip_map && lCubeFace()==cube_face){_lock_count++; return true;} // we want the same mip-map and cube-face that's already locked
      }else
      if(lock) // we want to set a proper lock
      {
         SyncLocker locker(D._lock);
         if(D.created())
         {
            if(!_lock_mode)switch(mode()) // first lock
            {
            #if DX11
               case IMAGE_RT:
               case IMAGE_DS:
               case IMAGE_2D: if(_txtr)
               {
                  Int blocks_y=softBlocksY(mip_map),
                      pitch   =softPitch  (mip_map),
                      pitch2  =pitch*blocks_y;

                  if(lock==LOCK_WRITE)Alloc(_data, pitch2);else
                  {
                     // get from GPU
                     Image temp; if(temp.createEx(Max(1, hwW()>>mip_map), Max(1, hwH()>>mip_map), 1, hwType(), IMAGE_STAGING, 1))
                     {
                        D3DC->CopySubresourceRegion(temp._txtr, D3D11CalcSubresource(0, 0, temp.mipMaps()), 0, 0, 0, _txtr, D3D11CalcSubresource(mip_map, 0, mipMaps()), null);
                        if(temp.lockRead())
                        {
                           Alloc(_data, pitch2);
                           CopyImgData(temp.data(), data(), temp.pitch(), pitch, temp.softBlocksY(0), blocks_y);
                        }
                     }
                  }
                  if(data())
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=1;
                    _lock_mip   =mip_map;
                  //_lock_face  =0;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =pitch;
                    _pitch2     =pitch2;
                     return true;
                  }
               }break;

               case IMAGE_3D: if(_txtr)
               {
                  Int ld      =Max(1, d()>>mip_map),
                      blocks_y=softBlocksY(mip_map),
                      pitch   =softPitch  (mip_map),
                      pitch2  =pitch *blocks_y,
                      pitch3  =pitch2*ld;
                  if(lock==LOCK_WRITE)Alloc(_data, pitch3);else
                  {
                     // get from GPU
                     D3D11_TEXTURE3D_DESC desc;
                     desc.Width         =PaddedWidth (hwW(), hwH(), mip_map, hwType());
                     desc.Height        =PaddedHeight(hwW(), hwH(), mip_map, hwType());
                     desc.Depth         =ld;
                     desc.MipLevels     =1;
                     desc.Format        =hwTypeInfo().format;
                     desc.Usage         =D3D11_USAGE_STAGING;
                     desc.BindFlags     =0;
                     desc.MiscFlags     =0;
                     desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE;

                     ID3D11Texture3D *temp; if(OK(D3D->CreateTexture3D(&desc, null, &temp)))
                     {
                        D3DC->CopySubresourceRegion(temp, D3D11CalcSubresource(0,0,1), 0, 0, 0, _txtr, D3D11CalcSubresource(mip_map, 0, mipMaps()), null);
                        D3D11_MAPPED_SUBRESOURCE map; if(OK(D3DC->Map(temp, D3D11CalcSubresource(0,0,1), D3D11_MAP_READ, 0, &map)))
                        {
                           Alloc(_data, pitch3);
                           CopyImgData((Byte*)map.pData, data(), map.RowPitch, pitch, blocks_y/*TODO: temp.softBlocksY(0)*/, blocks_y, map.DepthPitch, pitch2, ld, ld);
                           D3DC->Unmap(temp, D3D11CalcSubresource(0,0,1));
                        }
                        RELEASE(temp);
                     }
                  }
                  if(data())
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=ld;
                    _lock_mip   =mip_map;
                  //_lock_face  =0;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =pitch;
                    _pitch2     =pitch2;
                     return true;
                  }
               }break;

               case IMAGE_CUBE: if(_txtr)
               {
                  Int blocks_y=softBlocksY(mip_map),
                      pitch   =softPitch  (mip_map),
                      pitch2  =pitch*blocks_y;
                  if(lock==LOCK_WRITE)Alloc(_data, pitch2);else
                  {
                     // get from GPU
                     Image temp; if(temp.createTry(PaddedWidth(hwW(), hwH(), mip_map, hwType()), PaddedHeight(hwW(), hwH(), mip_map, hwType()), 1, hwType(), IMAGE_STAGING, 1, false))
                     {
                        D3DC->CopySubresourceRegion(temp._txtr, D3D11CalcSubresource(0, 0, temp.mipMaps()), 0, 0, 0, _txtr, D3D11CalcSubresource(mip_map, cube_face, mipMaps()), null);
                        if(temp.lockRead())
                        {
                           Alloc(_data, pitch2);
                           CopyImgData(temp.data(), data(), temp.pitch(), pitch, temp.softBlocksY(0), blocks_y);
                        }
                     }
                  }
                  if(data())
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=1;
                    _lock_mip   =mip_map;
                    _lock_face  =cube_face;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =pitch;
                    _pitch2     =pitch2;
                     return true;
                  }
               }break;

               case IMAGE_STAGING: if(_txtr)
               {
                  D3D11_MAPPED_SUBRESOURCE map; if(OK(D3DC->Map(_txtr, D3D11CalcSubresource(mip_map, 0, mipMaps()), (lock==LOCK_READ) ? D3D11_MAP_READ : (lock==LOCK_WRITE) ? D3D11_MAP_WRITE : D3D11_MAP_READ_WRITE, 0, &map))) // staging does not support D3D11_MAP_WRITE_DISCARD
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=1;
                    _lock_mip   =mip_map;
                  //_lock_face  =0;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _data       =(Byte*)map.pData;
                    _pitch      =       map.  RowPitch;
                    _pitch2     =       map.DepthPitch;
                     return true;
                  }
               }break;
            #elif GL
               case IMAGE_2D:
               case IMAGE_RT:
               case IMAGE_DS:
               case IMAGE_GL_RB:
               {
                  Int pitch =softPitch  (mip_map),
                      pitch2=softBlocksY(mip_map)*pitch;
               #if !GL_ES // 'glGetTexImage' not available on GL ES
                  if(_txtr)
                  {
                     Alloc(_data, CeilGL(pitch2));
                     if(lock!=LOCK_WRITE) // get from GPU
                     {
                        glGetError(); // clear any previous errors
                                             D.texBind(GL_TEXTURE_2D, _txtr);
                        if(!compressed())glGetTexImage(GL_TEXTURE_2D, mip_map, SourceGLFormat(hwType()), SourceGLType(hwType()), data());
                        else   glGetCompressedTexImage(GL_TEXTURE_2D, mip_map, data());
                        if(glGetError()!=GL_NO_ERROR)Free(_data);
                     }
                  }else
               #endif
                  if(mode()==IMAGE_RT
                  || mode()==IMAGE_GL_RB) // must use 'glReadPixels', also we don't want '_data_all' for these modes, because RT's are assumed to always change due to rendering
                  {
                     if(!compressed())
                     {
                        Alloc(_data, CeilGL(pitch2));
                        if(lock!=LOCK_WRITE && mip_map==0) // get from GPU
                        {
                           ImageRT *rt[Elms(Renderer._cur)], *ds;
                           Bool     restore_viewport=!D._view_active.full;
                           REPAO(rt)=Renderer._cur[i];
                                 ds =Renderer._cur_ds;

                           { // !! here operate on 'src' instead of 'T' !! (because 'T' can be temporarily swapped into 'temp')
                              ImageRT temp, *src;
                              if(this== Renderer._cur[0])src= Renderer._cur[0];else
                              if(this==&Renderer._main  )src=&Renderer._main  ;else
                                                        {src=&temp; Swap(T, SCAST(Image, temp));} // we can do a swap because on OpenGL 'ImageRT' doesn't have anything extra, this swap is only to allow this method to belong to 'Image' instead of having to use 'ImageRT'
                              Renderer.set(src, null, false); // put 'this' to FBO
                              glGetError(); // clear any previous errors
                              glReadPixels(0, 0, PaddedWidth (src->hwW(), src->hwH(), mip_map, src->hwType()),
                                                 PaddedHeight(src->hwW(), src->hwH(), mip_map, src->hwType()), SourceGLFormat(src->hwType()), SourceGLType(src->hwType()), src->_data);
                              if(glGetError()!=GL_NO_ERROR)Free(src->_data); // check for error right after 'glReadPixels' without doing any other operations which could overwrite 'glGetError'

                              // restore settings
                              Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, restore_viewport);

                              if(src==&temp)Swap(T, SCAST(Image, temp)); // swap back after restore
                           }
                           if(_data && this==&Renderer._main) // on OpenGL main has to be flipped
                              REPD(y, h()/2)Swap(_data + y*pitch, _data + (h()-1-y)*pitch, pitch);
                        }
                     }
                  }
               #if GL_ES
                  else
                  {
                     if(!_data_all && lock==LOCK_WRITE && mipMaps()==1)Alloc(_data_all, CeilGL(memUsage())); // if GL ES data is not available, but we want to write to it and we have only 1 mip map then re-create it
                     if( _data_all)_data=softData(mip_map);
                  }
               #endif
                  if(data())
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=1;
                    _lock_mip   =mip_map;
                  //_lock_face  =0;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =pitch;
                    _pitch2     =pitch2;
                     return true;
                  }
               }break;

               case IMAGE_3D: if(_txtr)
               {
                  Int ld    =Max(1, d()>>mip_map),
                      pitch =softPitch  (mip_map),
                      pitch2=softBlocksY(mip_map)*pitch;

               #if GL_ES // 'glGetTexImage' not available on GL ES
                  if(!_data_all && lock==LOCK_WRITE && mipMaps()==1)Alloc(_data_all, CeilGL(memUsage())); // if GL ES data is not available, but we want to write to it and we have only 1 mip map then re-create it
                  if( _data_all)_data=softData(mip_map);
               #else
                  Alloc(_data, CeilGL(pitch2*ld));
                  if(lock!=LOCK_WRITE) // get from GPU
                  {
                     glGetError(); // clear any previous errors
                                          D.texBind(GL_TEXTURE_3D, _txtr);
                     if(!compressed())glGetTexImage(GL_TEXTURE_3D, mip_map, SourceGLFormat(hwType()), SourceGLType(hwType()), data());
                     else   glGetCompressedTexImage(GL_TEXTURE_3D, mip_map, data());
                     if(glGetError()!=GL_NO_ERROR)Free(_data);
                  }
               #endif
                  if(data())
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=ld;
                    _lock_mip   =mip_map;
                  //_lock_face  =0;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =pitch;
                    _pitch2     =pitch2;
                     return true;
                  }
               }break;

               case IMAGE_CUBE: if(_txtr)
               {
                  Int pitch =softPitch  (mip_map),
                      pitch2=softBlocksY(mip_map)*pitch;

               #if GL_ES
                //if(!_data_all && lock==LOCK_WRITE && mipMaps()==1)Alloc(_data_all, CeilGL(memUsage())); // if GL ES data is not available, but we want to write to it and we have only 1 mip map then re-create it, can't do it here, because we have 6 faces, but we can lock only 1
                  if( _data_all)_data=softData(mip_map, cube_face);
               #else
                  Alloc(_data, CeilGL(pitch2));
                  if(lock!=LOCK_WRITE) // get from GPU
                  {
                     glGetError(); // clear any previous errors
                                          D.texBind(GL_TEXTURE_CUBE_MAP, _txtr);
                     if(!compressed())glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X+cube_face, mip_map, SourceGLFormat(hwType()), SourceGLType(hwType()), data());
                     else   glGetCompressedTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X+cube_face, mip_map, data());
                     if(glGetError()!=GL_NO_ERROR)Free(_data);
                  }
               #endif
                  if(data())
                  {
                    _lock_size.x=Max(1, w()>>mip_map);
                    _lock_size.y=Max(1, h()>>mip_map);
                    _lock_size.z=1;
                    _lock_mip   =mip_map;
                    _lock_face  =cube_face;
                    _lock_mode  =lock;
                    _lock_count =1;
                    _pitch      =pitch;
                    _pitch2     =pitch2;
                     return true;
                  }
               }break;
            #endif
            }else
            if(CompatibleLock(_lock_mode, lock) && lMipMap()==mip_map && lCubeFace()==cube_face){_lock_count++; return true;} // there was already a lock, just increase the counter and return success
         }
      }
   }
   return false;
}
/******************************************************************************/
Image& Image::unlock()
{
   if(_lock_count>0) // if image was locked
   {
      if(soft())
      {
       //for IMAGE_SOFT we don't need "if(mipMaps()>1)" check, because for IMAGE_SOFT, '_lock_count' will be set only if image has mip-maps, and since we've already checked "_lock_count>0" before, then we know we have mip-maps
         SafeSyncLocker locker(ImageSoftLock);
         if(_lock_count>0) // if locked
            if(!--_lock_count) // if unlocked now
               if(lMipMap()!=0 || lCubeFace()!=0) // if last locked mip-map or cube-face was not main
         {
           _lock_mip=0; _lock_face=DIR_ENUM(0); lockSoft(); // set default lock members to main mip map and cube face, set '_lock_mip', '_lock_face' before calling 'lockSoft'
         }
      }else
      {
         SafeSyncLockerEx locker(D._lock);
         if(_lock_count>0)if(!--_lock_count)switch(mode())
         {
         #if DX11
            case IMAGE_RT:
            case IMAGE_2D:
            case IMAGE_3D:
            case IMAGE_DS:
            {
               if(_lock_mode!=LOCK_READ && D3DC)D3DC->UpdateSubresource(_txtr, D3D11CalcSubresource(lMipMap(), 0, mipMaps()), null, data(), pitch(), pitch2());
              _lock_size.zero();
              _lock_mip =0;
            //_lock_face=0;
              _lock_mode=LOCK_NONE;
              _pitch    =0;
              _pitch2   =0;
               Free(_data);
            }break;

            case IMAGE_CUBE:
            {
               if(_lock_mode!=LOCK_READ && D3DC)D3DC->UpdateSubresource(_txtr, D3D11CalcSubresource(lMipMap(), lCubeFace(), mipMaps()), null, data(), pitch(), pitch2());
              _lock_size.zero();
              _lock_mip =0;
              _lock_face=DIR_ENUM(0);
              _lock_mode=LOCK_NONE;
              _pitch    =0;
              _pitch2   =0;
               Free(_data);
            }break;

            case IMAGE_STAGING:
            {
               if(D3DC)D3DC->Unmap(_txtr, D3D11CalcSubresource(lMipMap(), 0, mipMaps()));
              _lock_size.zero();
              _lock_mip =0;
            //_lock_face=0;
              _lock_mode=LOCK_NONE;
              _pitch    =0;
              _pitch2   =0;
              _data     =null;
            }break;
         #elif GL
            case IMAGE_2D:
            case IMAGE_RT:
            case IMAGE_DS:
            case IMAGE_GL_RB:
            {
               if(_lock_mode!=LOCK_READ && D.created())
               {
               #if GL_ES
                  if(mode()==IMAGE_RT)
                  {
                     // glDrawPixels
                  }else
               #endif
                  {
                    _lock_count++; locker.off(); // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                                         D.texBind(GL_TEXTURE_2D, _txtr);
                     if(!compressed())glTexImage2D(GL_TEXTURE_2D, lMipMap(), hwTypeInfo().format, Max(1, hwW()>>lMipMap()), Max(1, hwH()>>lMipMap()), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), data());
                     else   glCompressedTexImage2D(GL_TEXTURE_2D, lMipMap(), hwTypeInfo().format, Max(1, hwW()>>lMipMap()), Max(1, hwH()>>lMipMap()), 0, pitch2(), data());
                                           glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
                     locker.on(); _lock_count--;
                  }
               }
               if(!_lock_count)
               {
                 _lock_size.zero();
                 _lock_mip =0;
               //_lock_face=0;
                 _lock_mode=LOCK_NONE;
                 _pitch    =0;
                 _pitch2   =0;
                 _discard  =false;
               #if GL_ES
                  if(_data_all)_data=null;else Free(_data); // if we have '_data_all' then '_data' was set to part of it, so just clear it, otherwise it was allocated 
               #else
                  Free(_data);
               #endif
               }
            }break;

            case IMAGE_3D:
            {
               if(_lock_mode!=LOCK_READ && D.created())
               {
                 _lock_count++; locker.off(); // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                                      D.texBind(GL_TEXTURE_3D, _txtr);
                  if(!compressed())glTexImage3D(GL_TEXTURE_3D, lMipMap(), hwTypeInfo().format, Max(1, hwW()>>lMipMap()), Max(1, hwH()>>lMipMap()), Max(1, hwD()>>lMipMap()), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), data());
                  else   glCompressedTexImage3D(GL_TEXTURE_3D, lMipMap(), hwTypeInfo().format, Max(1, hwW()>>lMipMap()), Max(1, hwH()>>lMipMap()), Max(1, hwD()>>lMipMap()), 0, pitch2()*ld(), data());
                                        glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
                  locker.on(); _lock_count--;
               }
               if(!_lock_count)
               {
                 _lock_size.zero();
                 _lock_mip =0;
               //_lock_face=0;
                 _lock_mode=LOCK_NONE;
                 _pitch    =0;
                 _pitch2   =0;
               #if GL_ES
                 _data     =null;
               #else
                  Free(_data);
               #endif
               }
            }break;

            case IMAGE_CUBE:
            {
               if(_lock_mode!=LOCK_READ && D.created())
               {
                 _lock_count++; locker.off(); // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                                      D.texBind(GL_TEXTURE_CUBE_MAP, _txtr);
                  if(!compressed())glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+lCubeFace(), lMipMap(), hwTypeInfo().format, Max(1, hwW()>>lMipMap()), Max(1, hwH()>>lMipMap()), 0, SourceGLFormat(hwType()), SourceGLType(hwType()), data());
                  else   glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+lCubeFace(), lMipMap(), hwTypeInfo().format, Max(1, hwW()>>lMipMap()), Max(1, hwH()>>lMipMap()), 0, pitch2(), data());
                                        glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
                  locker.on(); _lock_count--;
               }
               if(!_lock_count)
               {
                 _lock_size.zero();
                 _lock_mip =0;
                 _lock_face=DIR_ENUM(0);
                 _lock_mode=LOCK_NONE;
                 _pitch    =0;
                 _pitch2   =0;
               #if GL_ES
                 _data     =null;
               #else
                  Free(_data);
               #endif
               }
            }break;
         #endif
         }
      }
   }
   return T;
}
  Bool   Image::  lockRead(Int mip_map, DIR_ENUM cube_face)C {return ConstCast(T).  lock(LOCK_READ, mip_map, cube_face);}
C Image& Image::unlock    (                               )C {return ConstCast(T).unlock();}
/******************************************************************************/
void Image::lockedSetMipData(CPtr data, Int mip_map)
{
   if(data && InRange(mip_map, mipMaps()))
   {
    //if(!waitForStream(mip_map))return false; this is called during streaming
      if(soft())CopyFast(softData(mip_map), data, softMipSize(mip_map));else
      {
      #if DX11
         if(D3DC)
         {
            Int pitch    =softPitch   (mip_map);
            Int pitch2   =softPitch2  (mip_map);
            Int face_size=softFaceSize(mip_map);
            Int faces    =T.faces();
            FREPD(face, faces)
            {
               D3DC->UpdateSubresource(_txtr, D3D11CalcSubresource(mip_map, face, mipMaps()), null, data, pitch, pitch2);
               data=(Byte*)data+face_size;
            }
         }
      #elif GL
         #if GL_ES
            if(softData())CopyFast(softData(mip_map), data, softMipSize(mip_map));
         #endif
         if(D.created())
         {
            VecI size(Max(1, hwW()>>mip_map), Max(1, hwH()>>mip_map), Max(1, hwD()>>mip_map));
            UInt format=hwTypeInfo().format, gl_format=SourceGLFormat(hwType()), gl_type=SourceGLType(hwType());
            switch(mode())
            {
               case IMAGE_2D:
               case IMAGE_RT:
               case IMAGE_DS:
               { // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                                      D.texBind(GL_TEXTURE_2D, _txtr);
                  if(!compressed())glTexImage2D(GL_TEXTURE_2D, mip_map, format, size.x, size.y, 0, gl_format, gl_type, data);
                  else   glCompressedTexImage2D(GL_TEXTURE_2D, mip_map, format, size.x, size.y, 0, softPitch2(mip_map), data);
                                        glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
                 _discard=false;
               }break;

               case IMAGE_3D:
               { // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                                      D.texBind(GL_TEXTURE_3D, _txtr);
                  if(!compressed())glTexImage3D(GL_TEXTURE_3D, mip_map, format, size.x, size.y, size.z, 0, gl_format, gl_type, data);
                  else   glCompressedTexImage3D(GL_TEXTURE_3D, mip_map, format, size.x, size.y, size.z, 0, softFaceSize(mip_map), data);
                                        glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
               }break;

               case IMAGE_CUBE:
               case IMAGE_RT_CUBE:
               { // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                  Int pitch2=softPitch2(mip_map);
                  D.texBind(GL_TEXTURE_CUBE_MAP, _txtr);
                  FREPD(face, 6)
                  {
                     if(!compressed())glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, mip_map, format, size.x, size.y, 0, gl_format, gl_type, data);
                     else   glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, mip_map, format, size.x, size.y, 0, pitch2, data);
                     data=(Byte*)data+pitch2;
                  }
                  glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
               }break;
            }
         }
      #endif
      }
   }
}
Bool Image::setFaceData(CPtr data, Int data_pitch, Int mip_map, DIR_ENUM cube_face)
{
   if(data && InRange(mip_map, mipMaps()) && InRange(cube_face, faces()) && waitForStream(mip_map))
   {
      Int data_blocks_y=ImageBlocksY(w(), h(), mip_map, hwType()), // !! USE VALID PIXELS FOR NOW, but this could be changed !!
          data_pitch2  =data_pitch*data_blocks_y,
          data_d       =Max(1, d()>>mip_map); // !! USE VALID PIXELS FOR NOW, but this could be changed !!
      if(soft())
      {
      #if GL
      soft:
      #endif
         Int img_d=Max(1, hwD()>>mip_map);
         CopyImgData((Byte*)data, softData(mip_map, cube_face), data_pitch, softPitch(mip_map), data_blocks_y, softBlocksY(mip_map), data_pitch2, softPitch2(mip_map), data_d, img_d);
         return true;
      }else
      {
      #if DX11
         SyncLocker locker(D._lock); if(D3DC)switch(mode())
         {
            case IMAGE_RT:
            case IMAGE_2D:
            case IMAGE_DS:
            case IMAGE_3D:
            case IMAGE_CUBE:
            {
               D3DC->UpdateSubresource(_txtr, D3D11CalcSubresource(mip_map, cube_face, mipMaps()), null, data, data_pitch, data_pitch2);
            }return true;
         }
      #elif GL // GL can accept only HW sizes
         Int img_pitch   =softPitch  (mip_map),
             img_blocks_y=softBlocksY(mip_map),
             img_pitch2  =img_pitch*img_blocks_y;
         if( img_pitch==data_pitch /*&& img_blocks_y==data_blocks_y ignore this because it's not specified by user*/ && D.created())
         {
            VecI size(Max(1, hwW()>>mip_map), Max(1, hwH()>>mip_map), Max(1, hwD()>>mip_map));
            UInt format=hwTypeInfo().format, gl_format=SourceGLFormat(hwType()), gl_type=SourceGLType(hwType());
            switch(mode())
            {
               case IMAGE_2D:
               case IMAGE_RT:
               case IMAGE_DS:
               { // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                                      D.texBind(GL_TEXTURE_2D, _txtr);
                  if(!compressed())glTexImage2D(GL_TEXTURE_2D, mip_map, format, size.x, size.y, 0, gl_format, gl_type, data);
                  else   glCompressedTexImage2D(GL_TEXTURE_2D, mip_map, format, size.x, size.y, 0, img_pitch2, data);
                                        glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
                 _discard=false;
                  if(GL_ES && softData())goto soft;
               }return true;

               case IMAGE_3D:
               { // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                                      D.texBind(GL_TEXTURE_3D, _txtr);
                  if(!compressed())glTexImage3D(GL_TEXTURE_3D, mip_map, format, size.x, size.y, size.z, 0, gl_format, gl_type, data);
                  else   glCompressedTexImage3D(GL_TEXTURE_3D, mip_map, format, size.x, size.y, size.z, 0, img_pitch2*size.z, data);
                                        glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
                  if(GL_ES && softData())goto soft;
               }return true;

               case IMAGE_CUBE:
               case IMAGE_RT_CUBE:
               { // OpenGL has per-thread context states, which means we don't need to be locked during following calls, this is important as following calls can be slow
                                      D.texBind(GL_TEXTURE_CUBE_MAP, _txtr);
                  if(!compressed())glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+cube_face, mip_map, format, size.x, size.y, 0, gl_format, gl_type, data);
                  else   glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+cube_face, mip_map, format, size.x, size.y, 0, img_pitch2, data);
                                        glFlush(); // to make sure that the data was initialized, in case it'll be accessed on a secondary thread
                  if(GL_ES && softData())goto soft;
               }return true;
            }
         }
      #endif
      }
      if(lock(LOCK_WRITE, mip_map, cube_face))
      {
         CopyImgData((Byte*)data, T.data(), data_pitch, T.pitch(), data_blocks_y, T.softBlocksY(mip_map), data_pitch2, T.pitch2(), data_d, T.ld());
         unlock();
         return true;
      }
   }
   return false;
}
/******************************************************************************/
Image& Image::freeOpenGLESData()
{
#if GL_ES
   if(hw()){unlock(); Free(_data_all);}
#endif
   return T;
}
/******************************************************************************/
Bool Image::updateMipMaps(C Image &src, Int src_mip, FILTER_TYPE filter, UInt flags, Int mip_start)
{
   if(mip_start<0)
   {
      src_mip -=mip_start; // use -= because if mip_start=-1 and src_mip=0 then it means that for mip #0 we're using src_mip=1
      mip_start=0;
   }
   if(!InRange(mip_start+1, mipMaps()))return true ; // if we can set the next one
   if(!InRange(src_mip, src.mipMaps()))return false; // if we can access the source
   Bool  ok=true;
   Int   src_faces1=src.faces()-1;
   Image temp; // keep outside the loop in case we can reuse the image
   REPD(face, faces())
   {
      ok&=src.extractMipMap(temp, typeInfo().compressed ? ImageTypeUncompressed(type()) : type(), src_mip, (DIR_ENUM)Min(face, src_faces1)); // use 'type' instead of 'hwType' (this is correct), use destination type instead of 'src.type' because we extract only one time, but inject several times
      for(Int mip=mip_start; ++mip<mipMaps(); )
      {
         temp.downSample(filter, flags);
         ok&=injectMipMap(temp, mip, DIR_ENUM(face), FILTER_BEST, flags);
      }
   }
   return ok;
}
Image& Image::updateMipMaps(FILTER_TYPE filter, UInt flags, Int mip_start)
{
   updateMipMaps(T, mip_start, filter, flags, mip_start); return T;
}
/******************************************************************************/
// GET
/******************************************************************************/
Int Image::faces()C {return is() ? ImageFaces(mode()) : 0;}
/******************************************************************************/
UInt Image::    memUsage()C {return ImageSize(hwW(), hwH(), hwD(), hwType(), mode(), mipMaps());}
UInt Image::typeMemUsage()C {return ImageSize(hwW(), hwH(), hwD(),   type(), mode(), mipMaps());}
/******************************************************************************/
void Image::lockedBaseMip(Int base_mip)
{
   if(_base_mip!=base_mip)
   {
     _base_mip=base_mip;
   #if DX11
      DYNAMIC_ASSERT(setSRV(), "Image.baseMip.setSRV");
   #elif GL
      UInt target;
      switch(mode())
      {
         case IMAGE_2D:
         case IMAGE_RT: target=GL_TEXTURE_2D; break;

         case IMAGE_3D: target=GL_TEXTURE_3D; break;

         case IMAGE_CUBE:
         case IMAGE_RT_CUBE: target=GL_TEXTURE_CUBE_MAP; break;

         default: goto skip;
      }
      D.texBind(target, _txtr);
      glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, _base_mip);
   skip:;
   #endif
   }
}
void Image::baseMip(Int base_mip)
{
   if(_base_mip!=base_mip)
   {
      if(soft())_base_mip=base_mip;else
      {
         SyncLocker locker(D._lock);
         lockedBaseMip(base_mip);
      }
   }
}
/******************************************************************************/
// HARDWARE
/******************************************************************************/
void Image::copyMs(ImageRT &dest, Bool restore_rt, Bool multi_sample, C RectI *rect)C
{
   if(this!=&dest)
   {
      ImageRT *rt[Elms(Renderer._cur)], *ds;
      Bool     restore_viewport;
      if(restore_rt)
      {
         REPAO(rt)=Renderer._cur[i];
               ds =Renderer._cur_ds;
         restore_viewport=!D._view_active.full;
      }

      Renderer.set(&dest, null, false);
      ALPHA_MODE alpha=D.alpha(ALPHA_NONE);

      Sh.Img  [0]->set(this);
      Sh.ImgMS[0]->set(this);
      VI.shader(!multiSample() ? Sh.Draw[true][false] : // 1s->1s, 1s->ms
                !multi_sample  ? Sh.DrawMs1           : // #0->1s, #0->ms
            dest.multiSample() ? Sh.DrawMsM           : // ms->ms
                                 Sh.DrawMsN          ); // ms->1s
                                VI.setType(VI_2D_TEX, VI_STRIP);
      if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx (4))
      {
         if(!rect)
         {
            v[0].pos.set(-1,  1);
            v[1].pos.set( 1,  1);
            v[2].pos.set(-1, -1);
            v[3].pos.set( 1, -1);

            v[0].tex.set(0, 0);
            v[1].tex.set(1, 0);
            v[2].tex.set(0, 1);
            v[3].tex.set(1, 1);
         }else
         {
            Rect frac(rect->min.x/Flt(dest.hwW())*2-1, rect->max.y/Flt(dest.hwH())*-2+1,
                      rect->max.x/Flt(dest.hwW())*2-1, rect->min.y/Flt(dest.hwH())*-2+1);
            v[0].pos.set(frac.min.x, frac.max.y);
            v[1].pos.set(frac.max.x, frac.max.y);
            v[2].pos.set(frac.min.x, frac.min.y);
            v[3].pos.set(frac.max.x, frac.min.y);

            Rect tex(Flt(rect->min.x)/hwW(), Flt(rect->min.y)/hwH(),
                     Flt(rect->max.x)/hwW(), Flt(rect->max.y)/hwH());
            v[0].tex.set(tex.min.x, tex.min.y);
            v[1].tex.set(tex.max.x, tex.min.y);
            v[2].tex.set(tex.min.x, tex.max.y);
            v[3].tex.set(tex.max.x, tex.max.y);
         }
      }
      VI.end();

      D.alpha(alpha);
      if(restore_rt)Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, restore_viewport);
   }
}
void Image::copyMs(ImageRT &dest, Bool restore_rt, Bool multi_sample, C Rect &rect)C
{
   copyMs(dest, restore_rt, multi_sample, &NoTemp(Round(D.screenToUV(rect)*size())));
}
/******************************************************************************/
void Image::copyHw(ImageRT &dest, Bool restore_rt, C RectI *rect_src, C RectI *rect_dest)C
{
   if(this!=&dest)
   {
      if(!rect_dest)dest.discard(); // if we set entire 'dest' then we can discard it
   #if GL
      if(this==&Renderer._main) // in OpenGL cannot directly copy from main
      {
         if(dest._txtr)
         {
            RectI rs(0, 0,    T.w(),    T.h()); if(rect_src )rs&=*rect_src ; if(rs.min.x>=rs.max.x || rs.min.y>=rs.max.y)return;
            RectI rd(0, 0, dest.w(), dest.h()); if(rect_dest)rd&=*rect_dest; if(rd.min.x>=rd.max.x || rd.min.y>=rd.max.y)return;

            // remember settings
            ImageRT *rt[Elms(Renderer._cur)], *ds;
            Bool     restore_viewport;
            if(restore_rt)
            {
               REPAO(rt)=Renderer._cur[i];
                     ds =Renderer._cur_ds;
               restore_viewport=!D._view_active.full;
            }

            Renderer.set(&dest, null, false); // put 'dest' to FBO
         #if IOS // there is no default frame buffer on iOS
            glBindFramebuffer        (GL_READ_FRAMEBUFFER, FBO1); // for unknown reason we can't use 'FBO' here, needs to be different
            glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _rb);
         #else
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); // read from default FBO
         #endif
         #if LINEAR_GAMMA && defined GL_FRAMEBUFFER_SRGB // on GL (not GL ES) reading from 'Renderer._main' gives results as if it's RGB and not sRGB, so we have to set as if we're writing RGB
            glDisable(GL_FRAMEBUFFER_SRGB);
         #endif
            glBlitFramebuffer(rs.min.x, h()-rs.min.y, rs.max.x, h()-rs.max.y,
                              rd.min.x,     rd.min.y, rd.max.x,     rd.max.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            // restore settings
         #if LINEAR_GAMMA && defined GL_FRAMEBUFFER_SRGB
            glEnable(GL_FRAMEBUFFER_SRGB);
         #endif
            glBindFramebuffer(GL_READ_FRAMEBUFFER, D._fbo);
            if(restore_rt)Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, restore_viewport);
         }
         return;
      }
   #endif
      // remember settings
      ImageRT *rt[Elms(Renderer._cur)], *ds;
      Bool     restore_viewport;
      if(restore_rt)
      {
         REPAO(rt)=Renderer._cur[i];
               ds =Renderer._cur_ds;
         restore_viewport=!D._view_active.full;
      }

   #if WINDOWS && LINEAR_GAMMA
      Bool swap=(this==&Renderer._main && type()==IMAGE_R8G8B8A8 && hwType()==IMAGE_R8G8B8A8_SRGB && dest.canSwapRTV()); if(swap)dest.swapRTV(); // #SwapFlipSRGB may fail to create '_main' as sRGB in that case create as linear and 'swapRTV' in 'ImageRT.map'
   #endif
      Renderer.set(&dest, null, false);
      ALPHA_MODE alpha=D.alpha(ALPHA_NONE);

      VI.image  (this);
      VI.shader (Sh.Draw[true][false]);
      VI.setType(VI_2D_TEX, VI_STRIP);
      if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
      {
         if(!rect_dest)
         {
            v[0].pos.set(-1,  1);
            v[1].pos.set( 1,  1);
            v[2].pos.set(-1, -1);
            v[3].pos.set( 1, -1);
         }else
         {
            Flt  xm=2.0f/dest.hwW(),
                 ym=2.0f/dest.hwH();
            Rect frac(rect_dest->min.x*xm-1, -rect_dest->max.y*ym+1,
                      rect_dest->max.x*xm-1, -rect_dest->min.y*ym+1);
            v[0].pos.set(frac.min.x, frac.max.y);
            v[1].pos.set(frac.max.x, frac.max.y);
            v[2].pos.set(frac.min.x, frac.min.y);
            v[3].pos.set(frac.max.x, frac.min.y);
         }

         if(!rect_src)
         {
            v[0].tex.set(0, 0);
            v[1].tex.set(1, 0);
            v[2].tex.set(0, 1);
            v[3].tex.set(1, 1);
         }else
         {
            Rect tex(Flt(rect_src->min.x)/hwW(), Flt(rect_src->min.y)/hwH(),
                     Flt(rect_src->max.x)/hwW(), Flt(rect_src->max.y)/hwH());
            v[0].tex.set(tex.min.x, tex.min.y);
            v[1].tex.set(tex.max.x, tex.min.y);
            v[2].tex.set(tex.min.x, tex.max.y);
            v[3].tex.set(tex.max.x, tex.max.y);
         }
      #if GL
         if(!D.mainFBO()) // in OpenGL when drawing to RenderTarget the 'dest.pos.y' must be flipped
         {
            CHS(v[0].pos.y);
            CHS(v[1].pos.y);
            CHS(v[2].pos.y);
            CHS(v[3].pos.y);
         }
      #endif
      }
      VI.end();

      // restore settings
   #if WINDOWS && LINEAR_GAMMA
      if(swap)dest.swapRTV();
   #endif
      D.alpha(alpha);
      if(restore_rt)Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, restore_viewport);
   }
}
static void SetRects(C Image &src, C Image &dest, RectI &rect_src, RectI &rect_dest, C Rect &rect)
{
   Rect uv=D.screenToUV(rect);

   // first the smaller Image must be set, and then the bigger Image must be set proportionally

   if(dest.hwW()<src.hwW()){rect_dest.setX(Round(uv.min.x*dest.hwW()), Round(uv.max.x*dest.hwW())); rect_src .setX(rect_dest.min.x* src.hwW()/dest.hwW(), rect_dest.max.x* src.hwW()/dest.hwW());}
   else                    {rect_src .setX(Round(uv.min.x* src.hwW()), Round(uv.max.x* src.hwW())); rect_dest.setX(rect_src .min.x*dest.hwW()/ src.hwW(), rect_src .max.x*dest.hwW()/ src.hwW());}

   if(dest.hwH()<src.hwH()){rect_dest.setY(Round(uv.min.y*dest.hwH()), Round(uv.max.y*dest.hwH())); rect_src .setY(rect_dest.min.y* src.hwH()/dest.hwH(), rect_dest.max.y* src.hwH()/dest.hwH());}
   else                    {rect_src .setY(Round(uv.min.y* src.hwH()), Round(uv.max.y* src.hwH())); rect_dest.setY(rect_src .min.y*dest.hwH()/ src.hwH(), rect_src .max.y*dest.hwH()/ src.hwH());}
}
void Image::copyHw(ImageRT &dest, Bool restore_rt, C Rect &rect)C
{
   RectI rect_src, rect_dest;
   SetRects(T, dest, rect_src, rect_dest, rect);
   copyHw(dest, restore_rt, &rect_src, &rect_dest);
}
/******************************************************************************/
Bool Image::capture(C ImageRT &src)
{
#if DX11
   if(src._txtr)
   {
      SyncLocker locker(D._lock);
      if(src.multiSample())
      {
         if(createTry(src.w(), src.h(), 1, src.hwType(), IMAGE_2D, 1, false))
         {
            D3DC->ResolveSubresource(_txtr, 0, src._txtr, 0, src.hwTypeInfo().format);
            return true;
         }
      }else
      if(createTry(PaddedWidth(src.hwW(), src.hwH(), 0, src.hwType()), PaddedHeight(src.hwW(), src.hwH(), 0, src.hwType()), 1, src.hwType(), IMAGE_STAGING, 1, false))
      {
         D3DC->CopySubresourceRegion(_txtr, D3D11CalcSubresource(0, 0, mipMaps()), 0, 0, 0, src._txtr, D3D11CalcSubresource(0, 0, src.mipMaps()), null);
         return true;
      }
   }
#elif GL
   if(src.lockRead())
   {
      Bool ok=false;
      if(createTry(src.w(), src.h(), 1, src.hwType(), IMAGE_SOFT, 1, false))ok=src.copySoft(T, FILTER_NO_STRETCH);
      src.unlock();
      return ok;
   }else
   {
      Bool depth=(src.hwTypeInfo().d>0);
      if( !depth || src.depthTexture())
         if(createTry(src.w(), src.h(), 1, depth ? IMAGE_F32 : src.hwType(), IMAGE_RT, 1, false))
      {
         ImageRT temp; Swap(T, SCAST(Image, temp)); // we can do a swap because on OpenGL 'ImageRT' doesn't have anything extra, this swap is only to allow 'capture' to be a method of 'Image' instead of having to use 'ImageRT'
         {SyncLocker locker(D._lock); src.copyHw(temp, true);}
         Swap(T, SCAST(Image, temp));
         return true;
      }
   }
#endif
   return false;
}
/******************************************************************************/
Bool Image::accessible()C
{
#if GL
   return _rb || _txtr; // on some platforms with OpenGL, 'Renderer._main' and 'Renderer._main_ds' are provided by the system, so their values may be zero, and we can't directly access it
#else
   return true;
#endif
}
Bool Image::compatible(C Image &image)C
{
   return size3()==image.size3() && samples()==image.samples();
}
/******************************************************************************/
DIR_ENUM DirToCubeFace(C Vec &dir)
{
   if(Flt f=Abs(dir).max())
   {
      Vec n=dir/f;
      const Flt one=1-FLT_EPS; // have to use epsilon, because just "1" failed in some cases
    //if(n.x>= one)return DIR_RIGHT; already listed at the bottom
      if(n.x<=-one)return DIR_LEFT;
      if(n.y>= one)return DIR_UP;
      if(n.y<=-one)return DIR_DOWN;
      if(n.z>= one)return DIR_FORWARD;
      if(n.z<=-one)return DIR_BACK;
   }
   return DIR_RIGHT;
}
DIR_ENUM DirToCubeFace(C Vec &dir, Int res, Vec2 &tex)
{
   if(Flt f=Abs(dir).max())
   {
      Vec n=dir/f;
      const Flt one=1-FLT_EPS; // have to use epsilon, because just "1" failed in some cases
      // tex.x=(n.x+1)/2*res-0.5
      // tex.x=(n.x+1)*res/2-0.5
      // tex.x=n.x*res/2 + res/2-0.5
      Flt mul=res*0.5f, add=mul-0.5f;
      if(n.x>= one){tex.set(-n.z*mul+add, -n.y*mul+add); return DIR_RIGHT  ;}
      if(n.x<=-one){tex.set( n.z*mul+add, -n.y*mul+add); return DIR_LEFT   ;}
      if(n.y>= one){tex.set( n.x*mul+add,  n.z*mul+add); return DIR_UP     ;}
      if(n.y<=-one){tex.set( n.x*mul+add, -n.z*mul+add); return DIR_DOWN   ;}
      if(n.z>= one){tex.set( n.x*mul+add, -n.y*mul+add); return DIR_FORWARD;}
      if(n.z<=-one){tex.set(-n.x*mul+add, -n.y*mul+add); return DIR_BACK   ;}
   }
   tex.zero(); return DIR_RIGHT;
}
Vec CubeFaceToDir(Flt x, Flt y, Int res, DIR_ENUM cube_face)
{
   // tex.x=(dir.x+1)/2*res-0.5
   // (tex.x+0.5)*2/res-1=dir.x
   // dir.x=tex.x*2/res + 0.5*2/res - 1
   // dir.x=tex.x*2/res + 1/res-1
   if(res>0)
   {
      Flt inv_res=1.0f/res, mul=2*inv_res, add=inv_res-1;
      switch(cube_face)
      {
         case DIR_RIGHT  : return Vec( 1, -y*mul-add, -x*mul-add);
         case DIR_LEFT   : return Vec(-1, -y*mul-add,  x*mul+add);
         case DIR_UP     : return Vec( x*mul+add,  1,  y*mul+add);
         case DIR_DOWN   : return Vec( x*mul+add, -1, -y*mul-add);
         case DIR_FORWARD: return Vec( x*mul+add, -y*mul-add,  1);
         case DIR_BACK   : return Vec(-x*mul-add, -y*mul-add, -1);
      }
   }
   return VecZero;
}
/******************************************************************************/
}
/******************************************************************************/
