/******************************************************************************

   This header is visible on both CPU and GPU

   !! If any change is made here then all shaders need to be recompiled !!

/******************************************************************************/
// Alpha Test Modes
#define ALPHA_TEST_NO     0
#define ALPHA_TEST_YES    1
#define ALPHA_TEST_DITHER 2
#define ALPHA_TEST_NUM    3

// Shader Bump Mapping Modes
#define SBUMP_ZERO      0 // no vertex normal
#define SBUMP_FLAT      1
#define SBUMP_NORMAL    2
#define SBUMP_PARALLAX0 2 // !! this is on purpose the same as SBUMP_NORMAL because it's never used anyway, it's used just for calculations
#define SBUMP_PARALLAX1 3
#define SBUMP_PARALLAX2 4
#define SBUMP_PARALLAX3 5
#define SBUMP_PARALLAX4 6
#define SBUMP_RELIEF    7

#define SBUMP_PARALLAX_MIN SBUMP_PARALLAX2 // ignore SBUMP_PARALLAX1 because just 1 sample is not worth to keep as separate shader
#define SBUMP_PARALLAX_MAX SBUMP_PARALLAX4

// Shader Diffuse Modes
#define SDIFFUSE_LAMBERT    0 // set Lambert as 0 to be the default mode for Forward/Blend shaders which don't have DIFFUSE_MODE specified
#define SDIFFUSE_OREN_NAYAR 1
#define SDIFFUSE_BURLEY     2
#define SDIFFUSE_NUM        3

// Effects
#define FX_NONE       0
#define FX_GRASS_2D   1
#define FX_GRASS_3D   2
#define FX_LEAF_2D    3
#define FX_LEAF_3D    4
#define FX_LEAFS_2D   5
#define FX_LEAFS_3D   6
#define FX_CLEAR_COAT 7

// Light Mode
#define LIGHT_MODE_DEFAULT    0
#define LIGHT_MODE_CLEAR_COAT 1
#define LIGHT_MODE_WATER      2
#define LIGHT_MODE_NUM        3

// Heightmap
#define HEIGHTMAP_NO     0
#define HEIGHTMAP_FLAT   1
#define HEIGHTMAP_SPHERE 2
#define HEIGHTMAP_NUM    3

// Motion
#define DUAL_DILATE_MOTION 1 // 1=generate separate dilated motion for Temporal and Motion

// Temporal
#define TEMPORAL_SEPARATE_ALPHA 1 // if store alpha in a separate Temporal RT, if 0 then it's stored in Temporal Data RT
#define TEMPORAL_SEPARATE_SUPER_RES_OLD_WEIGHT 1 // if calculate Old Weight in a separate pass for Super Res, this greatly improves performance when there's a lot of movement on the screen

// Buffer Indexes
#define SBI_FRAME           0
#define SBI_CAMERA          1
#define SBI_OBJ_MATRIX      2
#define SBI_OBJ_MATRIX_PREV 3
#define SBI_MESH            4
#define SBI_MATERIAL        5
#define SBI_VIEWPORT        6
#define SBI_COLOR           7
#define SBI_NUM             8

// Sampler Indexes
#define SSI_RENDER       0
#define SSI_POINT        1
#define SSI_LINEAR_CLAMP 2
#define SSI_LINEAR_WRAP  3
#define SSI_SHADOW       4
#define SSI_FONT         5
#define SSI_LINEAR_CWW   6
#define SSI_MINIMUM      7
#define SSI_MAXIMUM      8
#define SSI_NUM          9
#define SSI_DEFAULT_2D   SSI_LINEAR_CLAMP

// Material Textures
#define TEX_IS_ROUGH 1 // if texture has roughness (if 0 then it has smoothness)

// #MaterialTextureLayout
// base_0
#define BASE_CHANNEL_ALPHA w
// base_1
#define BASE_CHANNEL_NORMAL   xy
#define BASE_CHANNEL_NORMAL_X x
#define BASE_CHANNEL_NORMAL_Y y
// base_2
#define BASE_CHANNEL_ROUGH y
#define BASE_CHANNEL_METAL x
#define BASE_CHANNEL_BUMP  z
#define BASE_CHANNEL_GLOW  w

// #MaterialTextureLayoutDetail
#define DETAIL_CHANNEL_NORMAL   xy
#define DETAIL_CHANNEL_NORMAL_X x
#define DETAIL_CHANNEL_NORMAL_Y y
#define DETAIL_CHANNEL_ROUGH    z
#define DETAIL_CHANNEL_COLOR    w

// Other
#define MAX_MTRLS 4 // 3 or 4 (3 to make shaders smaller, 4 to support more materials per-triangle)

#define LCScale 0.2

#define NOISE_IMAGE_RES 128
/******************************************************************************/
