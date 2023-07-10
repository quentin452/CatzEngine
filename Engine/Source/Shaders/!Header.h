/******************************************************************************

   Optimization guidelines:
      -use MAD wherever possible A*B+C
      -Sign(x)*y can be replaced with (x>=0) ? y : -y (if Sign(x)==0 is not important)
      -use Sat(x) instead of Max(0, x) or Min(1, x) where 'x' is expected to be in 0..1 range
      -use Half, VecH2, VecH, VecH4 precision where possible
      -use NOPERSP for flat 2D shaders
      -offload some calculations on the CPU or Vertex Shader
      -use 'TexGather' or 'TexMin' or 'TexMax' wherever possible
      -use 'TexPoint' or 'TexLod' wherever possible
      -use constants without any suffix (0.0 instead of 0.0f or 0.0h) - https://gpuopen.com/first-steps-implementing-fp16/ - "Using either the h or f suffix will result in a conversion. It is better to use the unadorned literal, such as 0.0, 1.5 and so on."
      -if want to use "branching, continue, break" then have to check how it affects performance
      -compute shaders might be slow if "numthreads" is low (1,1,1)

/******************************************************************************/
#include "!Header CPU.h"
#define LINEAR_GAMMA 1
#define CONCAT(a, b) a##b
#if DX_SHADER_COMPILER
   #define LOC(i) [[vk::location(i)]]
#else
   #define LOC(i)
#endif
/******************************************************************************/
// DATA TYPES
/******************************************************************************/
#pragma pack_matrix(column_major) // always use "Column Major" matrix packing for most efficient 'Matrix' packing (orn+scale+pos) to use only 3x Vec4

#define Bool    bool
#define Int     int
#define UInt    uint
#define Flt     float
#define VecI2   int2
#define VecI    int3
#define VecI4   int4
#define VecU2   uint2
#define VecU    uint3
#define VecU4   uint4
#define Vec2    float2
#define Vec     float3
#define Vec4    float4
#define Matrix3 float3x3
#define Matrix  float4x3
#define Matrix4 float4x4

#define ImageF      Texture2D  <Flt  >
#define ImageF2     Texture2D  <Vec2 >
#define ImageH      Texture2D  <Half >
#define ImageH2     Texture2D  <VecH2>
#define Image       Texture2D  <VecH4>
#define Image3D     Texture3D  <VecH4>
#define Image3DH2   Texture3D  <VecH2>
#define ImageCube   TextureCube<VecH4>
#define ImageShadow Texture2D  <Half >

#define RWImageH  RWTexture2D<Half >
#define RWImageH2 RWTexture2D<VecH2>
#define RWImage   RWTexture2D<VecH4>

#define        SAMPLER(name, index) sampler                name : register(s##index) //        sampler
#define SHADOW_SAMPLER(name, index) SamplerComparisonState name : register(s##index) // shadow sampler
/******************************************************************************/
// HELPERS
/******************************************************************************/
#define PIXEL               Vec4 pixel :SV_Position               // pixel coordinates, integer based in format Vec4(x, y, 0, 0) ranges from (0, 0) to (RenderTarget.w(), RenderTarget.h())
#define FACE                UInt face  :SV_PrimitiveID            // face ID
#define IS_FRONT            Bool front :SV_IsFrontFace            // face front side
#define CLIP_DIST       out Flt  O_clip:SV_ClipDistance           // clip plane distance
#define CLIP_PLANE(pos) O_clip=Dot(Vec4((pos).xyz, 1), ClipPlane) // perform user plane clipping

#define BUFFER(name)          cbuffer name {                      // declare a constant buffer
#define BUFFER_I(name, index) cbuffer name : register(b##index) { // declare a constant buffer with custom buffer index
#define BUFFER_END            }                                   // end constant buffer declaration

#define POSITION SV_Position
#define DEPTH    SV_Depth
#define TARGET   SV_Target
#define TARGET0  SV_Target0
#define TARGET1  SV_Target1
#define TARGET2  SV_Target2
#define TARGET3  SV_Target3
#define TARGET4  SV_Target4
#define TARGET5  SV_Target5
#define TARGET6  SV_Target6
#define TARGET7  SV_Target7

#define NOINTERP nointerpolation // disable             interpolation and take value from the main vertex
#define NOPERSP  noperspective   // disable perspective interpolation

#define FLATTEN [flatten] // will make a conditional statement flattened, use before 'if'        statement
#define BRANCH  [branch ] // will make a conditional statement branched , use before 'if'        statement
#define LOOP    [loop   ] // will make a loop looped                    , use before 'for while' statements
#define UNROLL  [unroll ] // will make a loop unrolled                  , use before 'for while' statements
/******************************************************************************/
// FUNCTIONS
/******************************************************************************/
#define Dot       dot
#define Cross     cross
#define Sign      sign
#define Abs       abs
#define Sat       saturate
#define Mid       clamp
#define Frac      frac
#define Round     round
#define Trunc     trunc
#define Floor     floor
#define Ceil      ceil
#define Sqrt      sqrt
#define Rsqrt     rsqrt
#define Normalize normalize
#define Pow       pow // Exp2(Log2(x)*y) !! FASTER IF 'x' IS KNOWN AT COMPILE TIME !!
#define Sin       sin
#define Cos       cos
#define Tan       tan
#define Acos      acos
#define Asin      asin
#define Atan      atan
#define Lerp      lerp
#define Exp       exp  // identical performance as Exp2 = Exp2(x/Log(2))
#define Exp2      exp2 // identical performance as Exp
#define Log       log  // identical performance as Log2
#define Log2      log2 // identical performance as Log
/******************************************************************************/
// CONSTANTS
/******************************************************************************/
#define MAX_MATRIX 256 // maximum number of matrixes
#define FLT_MIN    1.175494351e-38f                   // Minimum positive value of 32-bit real (Flt )
#define HALF_MIN   0.00006103515625                   // Minimum positive value of 16-bit real (Half)
#define HALF_MAX   65504                              // Maximum possible value of 16-bit real (Half)
#define EPS        0.0001                             // float epsilon
#define EPS_COL    (1.0/256)                          // color epsilon
#define EPS_LUM    (LINEAR_GAMMA ? 1.0/512 : EPS_COL) // light epsilon (need a little extra precision for linear gamma)
#define EPS_RSQRT  HALF_MIN                           // rsqrt epsilon, makes sure that vec*rsqrt(Length2(vec)+EPS_RSQRT) gives range -1..1

#define PI_6    0.5235987755982988 // PI/6 ( 30 deg) Flt
#define PI_4    0.7853981633974483 // PI/4 ( 45 deg) Flt
#define PI_3    1.0471975511965977 // PI/3 ( 60 deg) Flt
#define PI_2    1.5707963267948966 // PI/2 ( 90 deg) Flt
#define PI      3.1415926535897932 // PI   (180 deg) Flt
#define PI2     6.2831853071795864 // PI*2 (360 deg) Flt
#define SQRT2   1.4142135623730950 // Sqrt(2)
#define SQRT3   1.7320508075688773 // Sqrt(3)
#define SQRT2_2 0.7071067811865475 // Sqrt(2)/2
#define SQRT3_3 0.5773502691896257 // Sqrt(3)/3
#define TAN     0.5                // tangent calculation constant

#define LOG_2  0.69314718055994529    // Log (2)
#define LOG2_E 1.44269504088896340736 // Log2(e)

#define ColorLumWeight  VecH(0.2126, 0.7152, 0.0722)
#define ColorLumWeight2 VecH(0.2990, 0.5870, 0.1140)

// Pixel Shade Mode #PIXEL_SHADE_MODE This can be [0..3] only, because we have R10G10B10A2 2 bits - 4 values
#define PSM_NONE        0
#define PSM_TRANSLUCENT 1 // used for double sided lighting for plants
#define PSM_CLEAR_COAT  2 // used for car surfaces

Bool IsPSM        (Flt value, Int mode) {return Abs(value-mode/3.0)<=0.5/3.0;}
Bool IsTranslucent(Flt value          ) {return IsPSM(value, PSM_TRANSLUCENT);}
Bool IsClearCoat  (Flt value          ) {return IsPSM(value, PSM_CLEAR_COAT );}

#define TRANSLUCENT_VAL 0.5

#define REFLECT_OCCL 0 // if apply occlusion for reflectivity below 0.02 #SpecularReflectionFromZeroSmoothReflectivity

#define JITTER_RANGE 4 // 3 or 4, don't use 3 because it's not good enough
/******************************************************************************/
// RENDER TARGETS
/******************************************************************************/
#define MS_SAMPLES 4 // number of samples in multi-sampled render targets

// signed formats in GL depend on "GL_EXT_render_snorm"
#define SIGNED_NRM_RT 0 // if Normal Render Target is signed, never because we use IMAGE_R10G10B10A2 which is unsigned

#define REVERSE_DEPTH (!GL) // if Depth Buffer is reversed. Can't enable on GL because for some reason (might be related to #glClipControl) it disables far-plane depth clipping, which can be observed when using func=FUNC_ALWAYS inside 'D.depthFunc'. Even though we clear the depth buffer, there may still be performance hit, because normally geometry would already get clipped due to far plane, but without it, per-pixel depth tests need to be performed.
#if     REVERSE_DEPTH
   #define Z_FRONT             1.0
   #define Z_BACK              0.0
   #define DEPTH_MIN           Max
   #define DEPTH_MAX           Min
   #define DEPTH_FOREGROUND(x) ((x)> Z_BACK)
   #define DEPTH_BACKGROUND(x) ((x)<=Z_BACK)
   #define DEPTH_SMALLER(x, y) ((x)> (y))
   #define DEPTH_INC(x, y)     ((x)-=(y))
   #define DEPTH_DEC(x, y)     ((x)+=(y))
   #define DEPTH_GE            SV_DepthLessEqual
   #define DEPTH_LE            SV_DepthGreaterEqual
   #define TexDepthRawMin(uv)  TexMax(Depth, uv).x
   #define TexDepthRawMax(uv)  TexMin(Depth, uv).x
#else
   #define Z_FRONT             0.0
   #define Z_BACK              1.0
   #define DEPTH_MIN           Min
   #define DEPTH_MAX           Max
   #define DEPTH_FOREGROUND(x) ((x)< Z_BACK)
   #define DEPTH_BACKGROUND(x) ((x)>=Z_BACK)
   #define DEPTH_SMALLER(x, y) ((x)< (y))
   #define DEPTH_INC(x, y)     ((x)+=(y))
   #define DEPTH_DEC(x, y)     ((x)-=(y))
   #define DEPTH_GE            SV_DepthGreaterEqual
   #define DEPTH_LE            SV_DepthLessEqual
   #define TexDepthRawMin(uv)  TexMin(Depth, uv).x
   #define TexDepthRawMax(uv)  TexMax(Depth, uv).x
#endif
/******************************************************************************/
#define RTex(    image, uv     )   image.Sample     (SamplerRender     , uv     ) // access 2D texture                               using Render Sampler
#if GL // for GL use 'SamplerRender' to avoid mixing different samplers, because on GL it would require to use a separate glsl 'sampler2D' which would increase CPU overhead for setting images/samplers and possibly exceed the MAX_SHADER_IMAGES
#define RTexLod( image, uv     )   image.SampleLevel(SamplerRender     , uv,   0) // access 2D texture   0-th MipMap (LOD level=  0) using Render Sampler (however since 'SampleLevel' makes Anisotropy disabled then for possible better performance use 'SamplerLinearWrap')
#define RTexLodI(image, uv, lod)   image.SampleLevel(SamplerRender     , uv, lod) // access 2D texture lod-th MipMap (LOD level=lod) using Render Sampler (however since 'SampleLevel' makes Anisotropy disabled then for possible better performance use 'SamplerLinearWrap')
#else
#define RTexLod( image, uv     )   image.SampleLevel(SamplerLinearWrap , uv,   0) // access 2D texture   0-th MipMap (LOD level=  0) using Render Sampler (however since 'SampleLevel' makes Anisotropy disabled then for possible better performance use 'SamplerLinearWrap')
#define RTexLodI(image, uv, lod)   image.SampleLevel(SamplerLinearWrap , uv, lod) // access 2D texture lod-th MipMap (LOD level=lod) using Render Sampler (however since 'SampleLevel' makes Anisotropy disabled then for possible better performance use 'SamplerLinearWrap')
#endif
#define  Tex(    image, uv     )   image.Sample     (SamplerLinearClamp, uv     ) // access 2D texture
#define  TexLod( image, uv     )   image.SampleLevel(SamplerLinearClamp, uv,   0) // access 2D texture   0-th MipMap (LOD level=  0)
#define  TexLodI(image, uv, lod)   image.SampleLevel(SamplerLinearClamp, uv, lod) // access 2D texture lod-th MipMap (LOD level=lod)

#define Tex3D(   image, uvw)   image.Sample     (SamplerLinearClamp, uvw   ) // access 3D texture
#define Tex3DLod(image, uvw)   image.SampleLevel(SamplerLinearClamp, uvw, 0) // access 3D texture 0-th MipMap (LOD level=0)

#define TexCube(     image, uvw     )   image.Sample     (SamplerRender     , uvw     ) // access Cube texture
#if GL // for GL use 'SamplerRender' to avoid mixing different samplers, because on GL it would require to use a separate glsl 'sampler2D' which would increase CPU overhead for setting images/samplers and possibly exceed the MAX_SHADER_IMAGES
#define TexCubeLod(  image, uvw     )   image.SampleLevel(SamplerRender     , uvw,   0) // access Cube texture   0-th MipMap (LOD level=  0), since 'SampleLevel' makes Anisotropy disabled then for possible better performance instead of 'SamplerRender' use 'SamplerLinearWrap'
#define TexCubeLodI( image, uvw, lod)   image.SampleLevel(SamplerRender     , uvw, lod) // access Cube texture lod-th MipMap (LOD level=lod), since 'SampleLevel' makes Anisotropy disabled then for possible better performance instead of 'SamplerRender' use 'SamplerLinearWrap'
#else
#define TexCubeLod(  image, uvw     )   image.SampleLevel(SamplerLinearWrap , uvw,   0) // access Cube texture   0-th MipMap (LOD level=  0), since 'SampleLevel' makes Anisotropy disabled then for possible better performance instead of 'SamplerRender' use 'SamplerLinearWrap'
#define TexCubeLodI( image, uvw, lod)   image.SampleLevel(SamplerLinearWrap , uvw, lod) // access Cube texture lod-th MipMap (LOD level=lod), since 'SampleLevel' makes Anisotropy disabled then for possible better performance instead of 'SamplerRender' use 'SamplerLinearWrap'
#endif
#define TexCubeClamp(image, uvw     )   image.Sample     (SamplerLinearClamp, uvw     ) // access Cube texture

#define TexPoint(   image, uv     )   image.SampleLevel(SamplerPoint, uv, 0)
#define TexPointOfs(image, uv, ofs)   image.SampleLevel(SamplerPoint, uv, 0, ofs)

#define TexMin(   image, uv     )   image.SampleLevel(SamplerMinimum, uv, 0)      // returns minimum out of all samples
#define TexMinOfs(image, uv, ofs)   image.SampleLevel(SamplerMinimum, uv, 0, ofs) // returns minimum out of all samples

#define TexMax(   image, uv     )   image.SampleLevel(SamplerMaximum, uv, 0)      // returns maximum out of all samples
#define TexMaxOfs(image, uv, ofs)   image.SampleLevel(SamplerMaximum, uv, 0, ofs) // returns maximum out of all samples

/* TEXTURE ACCESSING                 (Y^)
   GATHER returns in following order: V1 X  Y
                                      V0 W  Z
                                       + U0 U1 (X>) */
#define TexGather(   image, uv     )   image.Gather(SamplerPoint, uv     ) // gather available since SM_4_1, GL 4.0, GL ES 3.1
#define TexGatherOfs(image, uv, ofs)   image.Gather(SamplerPoint, uv, ofs) // gather available since SM_4_1, GL 4.0, GL ES 3.1

#define TexGatherR(   image, uv     )   image.GatherRed  (SamplerPoint, uv     ) // gather channel available since SM_5, GL 4.0, GL ES 3.1
#define TexGatherG(   image, uv     )   image.GatherGreen(SamplerPoint, uv     ) // gather channel available since SM_5, GL 4.0, GL ES 3.1
#define TexGatherB(   image, uv     )   image.GatherBlue (SamplerPoint, uv     ) // gather channel available since SM_5, GL 4.0, GL ES 3.1
#define TexGatherA(   image, uv     )   image.GatherAlpha(SamplerPoint, uv     ) // gather channel available since SM_5, GL 4.0, GL ES 3.1
#define TexGatherROfs(image, uv, ofs)   image.GatherRed  (SamplerPoint, uv, ofs) // gather channel available since SM_5, GL 4.0, GL ES 3.1
#define TexGatherGOfs(image, uv, ofs)   image.GatherGreen(SamplerPoint, uv, ofs) // gather channel available since SM_5, GL 4.0, GL ES 3.1
#define TexGatherBOfs(image, uv, ofs)   image.GatherBlue (SamplerPoint, uv, ofs) // gather channel available since SM_5, GL 4.0, GL ES 3.1
#define TexGatherAOfs(image, uv, ofs)   image.GatherAlpha(SamplerPoint, uv, ofs) // gather channel available since SM_5, GL 4.0, GL ES 3.1

#define TexSample(image, pixel, i)   image.Load(pixel, i) // access i-th sample of a multi-sampled texture

#define TexDepthRawPoint(      uv)                       TexPoint    (Depth  , uv     ).x
#define TexDepthRawPointOfs(   uv, ofs)                  TexPointOfs (Depth  , uv, ofs).x
#define TexDepthRawLinear(     uv)                       TexLod      (Depth  , uv     ).x
#define TexDepthPoint(         uv)        LinearizeDepth(TexPoint    (Depth  , uv     ).x)
#define TexDepthPointOfs(      uv, ofs)   LinearizeDepth(TexPointOfs (Depth  , uv, ofs).x)
#define TexDepthLinear(        uv)        LinearizeDepth(TexLod      (Depth  , uv     ).x)
#define TexDepthRawGather(     uv)                       TexGather   (Depth  , uv     )
#define TexDepthRawGatherOfs(  uv, ofs)                  TexGatherOfs(Depth  , uv, ofs)
#define TexDepthRawMS(   pixel, sample)                  TexSample   (DepthMS, pixel, sample).x
#define TexDepthMS(      pixel, sample)   LinearizeDepth(TexSample   (DepthMS, pixel, sample).x)
#define TexDepthPix(     pixel        )   LinearizeDepth(             Depth  [ pixel])

#if !GL
#define TexShadow(image, uvw)   image.SampleCmpLevelZero(SamplerShadowMap, uvw.xy, uvw.z)
#else
#define TexShadow(image, uvw)   image.SampleCmpLevelZero(SamplerShadowMap, uvw.xy, uvw.z*0.5+0.5) // adjust OpenGL depth scale (z' = z*0.5 + 0.5) #glClipControl
#endif
/******************************************************************************/
// CONSTANTS
/******************************************************************************/
#include "!Set Prec Struct.h"

struct RectI
{
   VecI2 min, max;
};

struct ViewportClass
{
   Flt  from, range;
   Vec2 center, size, size_fov_tan;
   Vec4 FracToPosXY, UVToPosXY, ProjectedPosToUV;
};

BUFFER_I(Viewport, SBI_VIEWPORT)
   Vec4          Coords  ;
   Vec4          RTSize  ; // xy=1/Image.hwSize(), zw=Image.hwSize(), this format is also required for SMAA
   ViewportClass Viewport;
BUFFER_END

BUFFER(Constants)
   const Vec2 BlendOfs4[4]=
   {
      Vec2( 0.5, -1.5),
      Vec2(-1.5, -0.5),
      Vec2( 1.5,  0.5),
      Vec2(-0.5,  1.5),
   };
 /*const Vec2 BlendOfs5[5]=
   {
      Vec2(-0.5, -1.5),

      Vec2(-1.5, -0.5),
      Vec2( 1.5, -0.5),

      Vec2(-0.5,  1.5),
      Vec2( 1.5,  1.5),
   };*/
   const Vec2 BlendOfs6[6]=
   {
      Vec2( 0.5, -2.5),

      Vec2(-0.5, -0.5),
      Vec2( 1.5, -0.5),

      Vec2(-2.5,  0.5),

      Vec2(-0.5,  1.5),
      Vec2( 1.5,  1.5),
   };
   const Vec2 BlendOfs8[8]=
   {
      Vec2(-1.5, -2.5),

      Vec2( 0.5, -1.5),
      Vec2( 2.5, -1.5),

      Vec2(-1.5, -0.5),

      Vec2( 1.5,  0.5),

      Vec2(-2.5,  1.5),
      Vec2(-0.5,  1.5),

      Vec2( 1.5,  2.5),
   };
 /*const Vec2 BlendOfs9[9]=
   {
      Vec2(-2.5, -2.5),
      Vec2(-0.5, -2.5),
      Vec2( 1.5, -2.5),

      Vec2(-2.5, -0.5),
      Vec2(-0.5, -0.5),
      Vec2( 1.5, -0.5),

      Vec2(-2.5,  1.5),
      Vec2(-0.5,  1.5),
      Vec2( 1.5,  1.5),
   };*/
   const Vec2 BlendOfs12[12]=
   {
      Vec2( 0.5, -3.5),

      Vec2(-1.5, -2.5),

      Vec2( 0.5, -1.5),
      Vec2( 2.5, -1.5),

      Vec2(-3.5, -0.5),
      Vec2(-1.5, -0.5),

      Vec2( 1.5,  0.5),
      Vec2( 3.5,  0.5),

      Vec2(-2.5,  1.5),
      Vec2(-0.5,  1.5),

      Vec2( 1.5,  2.5),

      Vec2(-0.5,  3.5),
   };
 /*const Vec2 BlendOfs13[13]=
   {
      Vec2(-0.5, -3.5),

      Vec2( 1.5, -2.5),

      Vec2(-0.5, -1.5),

      Vec2(-3.5, -0.5),
      Vec2(-1.5, -0.5),
      Vec2( 1.5, -0.5),
      Vec2( 3.5, -0.5),

      Vec2(-2.5,  1.5),
      Vec2(-0.5,  1.5),
      Vec2( 1.5,  1.5),
      Vec2( 3.5,  1.5),

      Vec2(-0.5,  3.5),
      Vec2( 1.5,  3.5),
   };*/
   #define V(x) ((x+0.5)/4-0.5) // gives -0.375 .. 0.375 range
   const Flt Noise4[4]=
   {
      V(0), V(2),
      V(3), V(1),
   };
   #define V(x) ((x+0.5)/16-0.5) // gives -0.46875 .. 0.46875 range
   const Flt Noise16[16]=
   {
      V( 0), V( 8), V( 2), V(10),
      V(12), V( 4), V(14), V( 6),
      V( 3), V(11), V( 1), V( 9),
      V(15), V( 7), V(13), V( 5),
   };
   #define V(x) ((x+0.5)/64-0.5) // gives -0.4921875 .. 0.4921875 range
   const Flt Noise64[64]=
   {
      V( 0), V(32), V( 8), V(40), V( 2), V(34), V(10), V(42),
      V(48), V(16), V(56), V(24), V(50), V(18), V(58), V(26),
      V(12), V(44), V( 4), V(36), V(14), V(46), V( 6), V(38),
      V(60), V(28), V(52), V(20), V(62), V(30), V(54), V(22),
      V( 3), V(35), V(11), V(43), V( 1), V(33), V( 9), V(41),
      V(51), V(19), V(59), V(27), V(49), V(17), V(57), V(25),
      V(15), V(47), V( 7), V(39), V(13), V(45), V( 5), V(37),
      V(63), V(31), V(55), V(23), V(61), V(29), V(53), V(21),
   };
   #undef V
BUFFER_END

BUFFER(Step)
   Flt Step;
BUFFER_END

BUFFER(ImgSize)
   Vec4 ImgSize; // xy=1/Image.hwSize(), zw=Image.hwSize(), this format is also required for SMAA
BUFFER_END

BUFFER(ImgClamp)
   Vec4 ImgClamp; // xy=min.xy, zw=max.xy
BUFFER_END

BUFFER_I(Color, SBI_COLOR)
   VecH4 Color[2];
BUFFER_END
/******************************************************************************/
BUFFER_I(Frame, SBI_FRAME) // once per-frame
   Vec4    ClipPlane=Vec4(0, 0, 0, 1); // clipping plane
   Vec4    BendFactor                ; // factors used for grass/leaf bending calculation
   Vec4    BendFactorPrev            ; // factors used for grass/leaf bending calculation (for previous frame)
   VecI2   NoiseOffset               ; // per-frame texture noise offset
   Vec2    GrassRangeMulAdd          ; // factors used for grass opacity calculation
   Flt     TesselationDensity        ; // tesselation density
   Bool    FirstPass=true            ; // if first pass (apply Material Emissive and Light from Glow)
   VecH    AmbientNSColor            ; // ambient combined with night shade
   Half    AspectRatio               ; // converts UV to Screen aspect ratio
   VecH    EnvColor                  ; // environment map color
   Half    EnvMipMaps                ; // environment map mip-maps
   Matrix3 EnvMatrix                 ; // environment map matrix
BUFFER_END

BUFFER_I(Camera, SBI_CAMERA) // this gets changed when drawing shadow maps
   Matrix4 ProjMatrix    ; // projection matrix
   Matrix4 ProjMatrixPrev; // projection matrix (for previous frame)
   Matrix  CamMatrix     ; // camera     matrix
   Matrix  CamMatrixPrev ; // camera     matrix (for previous frame)
BUFFER_END

BUFFER(ViewToViewPrev)
   Matrix ViewToViewPrev; // converts from current view space to view space from previous frame
BUFFER_END

BUFFER_I(Mesh, SBI_MESH)
   Bool  VtxSkinning;
   Flt   VtxUVScale;
   VecH4 Highlight; // this can be modified by engine's 'SetHighlight' function
BUFFER_END

BUFFER(Behind)
   Half BehindBias; // this can be modified by engine's 'SetBehindBias' function
BUFFER_END
/******************************************************************************/
// MATERIALS
/******************************************************************************/
struct MaterialClass // this is used when a MeshPart has only one material
{
   VecH4 color; // !! color must be listed first because ShaderParam handle for setting 'Material.color' is set from the entire Material object pointer !!
   VecH  emissive;
   Half  emissive_glow,
           rough_mul,   rough_add,
         reflect_mul, reflect_add,
         glow,
         normal,
         bump,
         det_power;
   Flt   det_uv_scale,
             uv_scale;
};
BUFFER_I(Material, SBI_MATERIAL)
   MaterialClass Material;
BUFFER_END
/******************************************************************************/
struct MultiMaterialClass // this is used when a MeshPart has multiple materials
{
   VecH4 color;
   VecH  refl_rogh_glow_mul, refl_rogh_glow_add;
   Half  normal, bump, det_mul, det_add, det_inv, macro;
   Flt   uv_scale, det_uv_scale;
};
BUFFER(MultiMaterial0) MultiMaterialClass MultiMaterial0; BUFFER_END
BUFFER(MultiMaterial1) MultiMaterialClass MultiMaterial1; BUFFER_END
BUFFER(MultiMaterial2) MultiMaterialClass MultiMaterial2; BUFFER_END
BUFFER(MultiMaterial3) MultiMaterialClass MultiMaterial3; BUFFER_END
/******************************************************************************/
#include "!Set Prec Default.h"
/******************************************************************************/
// IMAGES
/******************************************************************************/
#include "!Set Prec Image.h"
// #MaterialTextureLayout
Image   Col, Col1, Col2, Col3;
ImageH2 Nrm, Nrm1, Nrm2, Nrm3;
Image   Ext, Ext1, Ext2, Ext3,
        Det, Det1, Det2, Det3,
        Mac, Mac1, Mac2, Mac3,
        Lum;

// #MaterialTextureLayout
#define BUMP_IMAGE Ext

// #MaterialTextureLayoutDetail
#define APPLY_DETAIL_ROUGH(rough, tx_delta) rough+=(TEX_IS_ROUGH ? tx_delta : -tx_delta) // apply texture relative delta onto roughness (this is -tx and not 1-tx because this is relative delta and not absolute value)

Image     Img, Img1, Img2, Img3, Img4, Img5;
ImageH    ImgX, ImgX1, ImgX2, ImgX3, ImgNoise;
ImageF    ImgXF, ImgXF1, Depth;
ImageH2   ImgXY, ImgXY1, ImgXY2, EnvDFG;
ImageCube Env, Cub, Cub1;
Image3D   Vol;
Image3DH2 VolXY, VolXY1;
//Image     SkyA, SkyB;

Texture2DMS<VecH4, MS_SAMPLES> ImgMS, ImgMS1, ImgMS2, ImgMS3;
Texture2DMS<Half , MS_SAMPLES> ImgXMS;
Texture2DMS<VecH2, MS_SAMPLES> ImgXYMS;
Texture2DMS<Flt  , MS_SAMPLES> DepthMS;

RWImage   RWImg;
RWImageH  RWImgX, RWImgX1;
RWImageH2 RWImgXY;
#include "!Set Prec Default.h"

       SAMPLER(SamplerRender     , SSI_RENDER      );
       SAMPLER(SamplerPoint      , SSI_POINT       );
       SAMPLER(SamplerLinearClamp, SSI_LINEAR_CLAMP);
       SAMPLER(SamplerLinearWrap , SSI_LINEAR_WRAP );
       SAMPLER(SamplerLinearCWW  , SSI_LINEAR_CWW  );
SHADOW_SAMPLER(SamplerShadowMap  , SSI_SHADOW      );
       SAMPLER(SamplerFont       , SSI_FONT        );
       SAMPLER(SamplerMinimum    , SSI_MINIMUM     );
       SAMPLER(SamplerMaximum    , SSI_MAXIMUM     );
/******************************************************************************/
Bool CanDiv(Half x) {return x>=HALF_MIN;} // instead of >0 to skip subnormals in case they might cause bad values
Bool CanDiv(Flt  x) {return x>= FLT_MIN;} // instead of >0 to skip subnormals in case they might cause bad values

// force convert to Half (can be used for testing Precisions)
Half  AsHalf(Flt  x) {return f16tof32(f32tof16(x));}
VecH2 AsHalf(Vec2 x) {return f16tof32(f32tof16(x));}
VecH  AsHalf(Vec  x) {return f16tof32(f32tof16(x));}
VecH4 AsHalf(Vec4 x) {return f16tof32(f32tof16(x));}

// have to use custom functions because DX compiler might optimize-away default 'isnan' and 'isinf'
Bool  NaN(Flt  x) {return (asuint(x)&0x7FFFFFFF)> 0x7F800000;}
Bool  Inf(Flt  x) {return (asuint(x)&0x7FFFFFFF)==0x7F800000;}
bool2 NaN(Vec2 x) {return bool2(NaN(x.x), NaN(x.y));}
bool2 Inf(Vec2 x) {return bool2(Inf(x.x), Inf(x.y));}
bool3 NaN(Vec  x) {return bool3(NaN(x.x), NaN(x.y), NaN(x.z));}
bool3 Inf(Vec  x) {return bool3(Inf(x.x), Inf(x.y), Inf(x.z));}
bool4 NaN(Vec4 x) {return bool4(NaN(x.x), NaN(x.y), NaN(x.z), NaN(x.w));}
bool4 Inf(Vec4 x) {return bool4(Inf(x.x), Inf(x.y), Inf(x.z), Inf(x.w));}

Int   Min(Int   x, Int   y                  ) {return min(x, y);}
Half  Min(Half  x, Half  y                  ) {return min(x, y);}
Half  Min(Bool  x, Half  y                  ) {return min(x, y);}
Half  Min(Half  x, Bool  y                  ) {return min(x, y);}
Half  Min(Int   x, Half  y                  ) {return min(x, y);}
Half  Min(Half  x, Int   y                  ) {return min(x, y);}
Flt   Min(Flt   x, Flt   y                  ) {return min(x, y);}
Flt   Min(Bool  x, Flt   y                  ) {return min(x, y);}
Flt   Min(Flt   x, Bool  y                  ) {return min(x, y);}
Flt   Min(Int   x, Flt   y                  ) {return min(x, y);}
Flt   Min(Flt   x, Int   y                  ) {return min(x, y);}
VecH2 Min(VecH2 x, VecH2 y                  ) {return min(x, y);}
Vec2  Min(Vec2  x, Vec2  y                  ) {return min(x, y);}
VecH  Min(VecH  x, VecH  y                  ) {return min(x, y);}
Vec   Min(Vec   x, Vec   y                  ) {return min(x, y);}
VecH4 Min(VecH4 x, VecH4 y                  ) {return min(x, y);}
Vec4  Min(Vec4  x, Vec4  y                  ) {return min(x, y);}
Int   Min(Int   x, Int   y, Int   z         ) {return min(x, min(y, z));}
Half  Min(Half  x, Half  y, Half  z         ) {return min(x, min(y, z));}
Flt   Min(Flt   x, Flt   y, Flt   z         ) {return min(x, min(y, z));}
VecH2 Min(VecH2 x, VecH2 y, VecH2 z         ) {return min(x, min(y, z));}
Vec2  Min(Vec2  x, Vec2  y, Vec2  z         ) {return min(x, min(y, z));}
VecH  Min(VecH  x, VecH  y, VecH  z         ) {return min(x, min(y, z));}
Vec   Min(Vec   x, Vec   y, Vec   z         ) {return min(x, min(y, z));}
VecH4 Min(VecH4 x, VecH4 y, VecH4 z         ) {return min(x, min(y, z));}
Vec4  Min(Vec4  x, Vec4  y, Vec4  z         ) {return min(x, min(y, z));}
Int   Min(Int   x, Int   y, Int   z, Int   w) {return min(x, min(y, min(z, w)));}
Half  Min(Half  x, Half  y, Half  z, Half  w) {return min(x, min(y, min(z, w)));}
Flt   Min(Flt   x, Flt   y, Flt   z, Flt   w) {return min(x, min(y, min(z, w)));}
VecH2 Min(VecH2 x, VecH2 y, VecH2 z, VecH2 w) {return min(x, min(y, min(z, w)));}
Vec2  Min(Vec2  x, Vec2  y, Vec2  z, Vec2  w) {return min(x, min(y, min(z, w)));}
VecH  Min(VecH  x, VecH  y, VecH  z, VecH  w) {return min(x, min(y, min(z, w)));}
Vec   Min(Vec   x, Vec   y, Vec   z, Vec   w) {return min(x, min(y, min(z, w)));}
VecH4 Min(VecH4 x, VecH4 y, VecH4 z, VecH4 w) {return min(x, min(y, min(z, w)));}
Vec4  Min(Vec4  x, Vec4  y, Vec4  z, Vec4  w) {return min(x, min(y, min(z, w)));}

Int   Max(Int   x, Int   y                  ) {return max(x, y);}
Half  Max(Half  x, Half  y                  ) {return max(x, y);}
Half  Max(Bool  x, Half  y                  ) {return max(x, y);}
Half  Max(Half  x, Bool  y                  ) {return max(x, y);}
Half  Max(Int   x, Half  y                  ) {return max(x, y);}
Half  Max(Half  x, Int   y                  ) {return max(x, y);}
Flt   Max(Flt   x, Flt   y                  ) {return max(x, y);}
Flt   Max(Bool  x, Flt   y                  ) {return max(x, y);}
Flt   Max(Flt   x, Bool  y                  ) {return max(x, y);}
Flt   Max(Int   x, Flt   y                  ) {return max(x, y);}
Flt   Max(Flt   x, Int   y                  ) {return max(x, y);}
VecH2 Max(VecH2 x, VecH2 y                  ) {return max(x, y);}
Vec2  Max(Vec2  x, Vec2  y                  ) {return max(x, y);}
VecH  Max(VecH  x, VecH  y                  ) {return max(x, y);}
Vec   Max(Vec   x, Vec   y                  ) {return max(x, y);}
VecH4 Max(VecH4 x, VecH4 y                  ) {return max(x, y);}
Vec4  Max(Vec4  x, Vec4  y                  ) {return max(x, y);}
Int   Max(Int   x, Int   y, Int   z         ) {return max(x, max(y, z));}
Half  Max(Half  x, Half  y, Half  z         ) {return max(x, max(y, z));}
Flt   Max(Flt   x, Flt   y, Flt   z         ) {return max(x, max(y, z));}
VecH2 Max(VecH2 x, VecH2 y, VecH2 z         ) {return max(x, max(y, z));}
Vec2  Max(Vec2  x, Vec2  y, Vec2  z         ) {return max(x, max(y, z));}
VecH  Max(VecH  x, VecH  y, VecH  z         ) {return max(x, max(y, z));}
Vec   Max(Vec   x, Vec   y, Vec   z         ) {return max(x, max(y, z));}
VecH4 Max(VecH4 x, VecH4 y, VecH4 z         ) {return max(x, max(y, z));}
Vec4  Max(Vec4  x, Vec4  y, Vec4  z         ) {return max(x, max(y, z));}
Int   Max(Int   x, Int   y, Int   z, Int   w) {return max(x, max(y, max(z, w)));}
Half  Max(Half  x, Half  y, Half  z, Half  w) {return max(x, max(y, max(z, w)));}
Flt   Max(Flt   x, Flt   y, Flt   z, Flt   w) {return max(x, max(y, max(z, w)));}
VecH2 Max(VecH2 x, VecH2 y, VecH2 z, VecH2 w) {return max(x, max(y, max(z, w)));}
Vec2  Max(Vec2  x, Vec2  y, Vec2  z, Vec2  w) {return max(x, max(y, max(z, w)));}
VecH  Max(VecH  x, VecH  y, VecH  z, VecH  w) {return max(x, max(y, max(z, w)));}
Vec   Max(Vec   x, Vec   y, Vec   z, Vec   w) {return max(x, max(y, max(z, w)));}
VecH4 Max(VecH4 x, VecH4 y, VecH4 z, VecH4 w) {return max(x, max(y, max(z, w)));}
Vec4  Max(Vec4  x, Vec4  y, Vec4  z, Vec4  w) {return max(x, max(y, max(z, w)));}

Half  Avg(Half  x, Half  y                  ) {return (x+y    )/2   ;}
Flt   Avg(Flt   x, Flt   y                  ) {return (x+y    )*0.50;}
VecH2 Avg(VecH2 x, VecH2 y                  ) {return (x+y    )/2   ;}
Vec2  Avg(Vec2  x, Vec2  y                  ) {return (x+y    )*0.50;}
VecH  Avg(VecH  x, VecH  y                  ) {return (x+y    )/2   ;}
Vec   Avg(Vec   x, Vec   y                  ) {return (x+y    )*0.50;}
VecH4 Avg(VecH4 x, VecH4 y                  ) {return (x+y    )/2   ;}
Vec4  Avg(Vec4  x, Vec4  y                  ) {return (x+y    )*0.50;}
Half  Avg(Half  x, Half  y, Half  z         ) {return (x+y+z  )/3   ;}
Flt   Avg(Flt   x, Flt   y, Flt   z         ) {return (x+y+z  )/3.00;}
VecH2 Avg(VecH2 x, VecH2 y, VecH2 z         ) {return (x+y+z  )/3   ;}
Vec2  Avg(Vec2  x, Vec2  y, Vec2  z         ) {return (x+y+z  )/3.00;}
VecH  Avg(VecH  x, VecH  y, VecH  z         ) {return (x+y+z  )/3   ;}
Vec   Avg(Vec   x, Vec   y, Vec   z         ) {return (x+y+z  )/3.00;}
VecH4 Avg(VecH4 x, VecH4 y, VecH4 z         ) {return (x+y+z  )/3   ;}
Vec4  Avg(Vec4  x, Vec4  y, Vec4  z         ) {return (x+y+z  )/3.00;}
Half  Avg(Half  x, Half  y, Half  z, Half  w) {return (x+y+z+w)/4   ;}
Flt   Avg(Flt   x, Flt   y, Flt   z, Flt   w) {return (x+y+z+w)*0.25;}
VecH2 Avg(VecH2 x, VecH2 y, VecH2 z, VecH2 w) {return (x+y+z+w)/4   ;}
Vec2  Avg(Vec2  x, Vec2  y, Vec2  z, Vec2  w) {return (x+y+z+w)*0.25;}
VecH  Avg(VecH  x, VecH  y, VecH  z, VecH  w) {return (x+y+z+w)/4   ;}
Vec   Avg(Vec   x, Vec   y, Vec   z, Vec   w) {return (x+y+z+w)*0.25;}
VecH4 Avg(VecH4 x, VecH4 y, VecH4 z, VecH4 w) {return (x+y+z+w)/4   ;}
Vec4  Avg(Vec4  x, Vec4  y, Vec4  z, Vec4  w) {return (x+y+z+w)*0.25;}

Half Min(VecH2 v) {return Min(v.x, v.y);}
Half Max(VecH2 v) {return Max(v.x, v.y);}
Half Avg(VecH2 v) {return Avg(v.x, v.y);}
Flt  Min(Vec2  v) {return Min(v.x, v.y);}
Flt  Max(Vec2  v) {return Max(v.x, v.y);}
Flt  Avg(Vec2  v) {return Avg(v.x, v.y);}
Half Min(VecH  v) {return Min(v.x, v.y, v.z);}
Half Max(VecH  v) {return Max(v.x, v.y, v.z);}
Half Avg(VecH  v) {return Avg(v.x, v.y, v.z);}
Flt  Min(Vec   v) {return Min(v.x, v.y, v.z);}
Flt  Max(Vec   v) {return Max(v.x, v.y, v.z);}
Flt  Avg(Vec   v) {return Avg(v.x, v.y, v.z);}
Half Min(VecH4 v) {return Min(v.x, v.y, v.z, v.w);}
Half Max(VecH4 v) {return Max(v.x, v.y, v.z, v.w);}
Half Avg(VecH4 v) {return Avg(v.x, v.y, v.z, v.w);}
Flt  Min(Vec4  v) {return Min(v.x, v.y, v.z, v.w);}
Flt  Max(Vec4  v) {return Max(v.x, v.y, v.z, v.w);}
Flt  Avg(Vec4  v) {return Avg(v.x, v.y, v.z, v.w);}

Half Sum(VecH2 v) {return v.x+v.y        ;}
Flt  Sum(Vec2  v) {return v.x+v.y        ;}
Half Sum(VecH  v) {return v.x+v.y+v.z    ;}
Flt  Sum(Vec   v) {return v.x+v.y+v.z    ;}
Half Sum(VecH4 v) {return v.x+v.y+v.z+v.w;}
Flt  Sum(Vec4  v) {return v.x+v.y+v.z+v.w;}

Int   Sqr(Int   x) {return x*x;}
Half  Sqr(Half  x) {return x*x;}
Flt   Sqr(Flt   x) {return x*x;}
VecH2 Sqr(VecH2 x) {return x*x;}
Vec2  Sqr(Vec2  x) {return x*x;}
VecH  Sqr(VecH  x) {return x*x;}
Vec   Sqr(Vec   x) {return x*x;}
VecH4 Sqr(VecH4 x) {return x*x;}
Vec4  Sqr(Vec4  x) {return x*x;}

Int   SqrS(Int   x) {return (x>=0) ? x*x : -x*x;}
Half  SqrS(Half  x) {return (x>=0) ? x*x : -x*x;}
Flt   SqrS(Flt   x) {return (x>=0) ? x*x : -x*x;}
VecH2 SqrS(VecH2 x) {return (x>=0) ? x*x : -x*x;}
Vec2  SqrS(Vec2  x) {return (x>=0) ? x*x : -x*x;}
VecH  SqrS(VecH  x) {return (x>=0) ? x*x : -x*x;}
Vec   SqrS(Vec   x) {return (x>=0) ? x*x : -x*x;}
VecH4 SqrS(VecH4 x) {return (x>=0) ? x*x : -x*x;}
Vec4  SqrS(Vec4  x) {return (x>=0) ? x*x : -x*x;}

Int   Cube(Int   x) {return x*x*x;}
Half  Cube(Half  x) {return x*x*x;}
Flt   Cube(Flt   x) {return x*x*x;}
VecH2 Cube(VecH2 x) {return x*x*x;}
Vec2  Cube(Vec2  x) {return x*x*x;}
VecH  Cube(VecH  x) {return x*x*x;}
Vec   Cube(Vec   x) {return x*x*x;}
VecH4 Cube(VecH4 x) {return x*x*x;}
Vec4  Cube(Vec4  x) {return x*x*x;}

Int   Quart(Int   x) {return Sqr(x*x);}
Half  Quart(Half  x) {return Sqr(x*x);}
Flt   Quart(Flt   x) {return Sqr(x*x);}
VecH2 Quart(VecH2 x) {return Sqr(x*x);}
Vec2  Quart(Vec2  x) {return Sqr(x*x);}
VecH  Quart(VecH  x) {return Sqr(x*x);}
Vec   Quart(Vec   x) {return Sqr(x*x);}
VecH4 Quart(VecH4 x) {return Sqr(x*x);}
Vec4  Quart(Vec4  x) {return Sqr(x*x);}

Int   Quint(Int   x) {return Quart(x)*x;}
Half  Quint(Half  x) {return Quart(x)*x;}
Flt   Quint(Flt   x) {return Quart(x)*x;}
VecH2 Quint(VecH2 x) {return Quart(x)*x;}
Vec2  Quint(Vec2  x) {return Quart(x)*x;}
VecH  Quint(VecH  x) {return Quart(x)*x;}
Vec   Quint(Vec   x) {return Quart(x)*x;}
VecH4 Quint(VecH4 x) {return Quart(x)*x;}
Vec4  Quint(Vec4  x) {return Quart(x)*x;}

Half Length(VecH2 v) {return length(v);}
Flt  Length(Vec2  v) {return length(v);}
Half Length(VecH  v) {return length(v);}
Flt  Length(Vec   v) {return length(v);}
Half Length(VecH4 v) {return length(v);}
Flt  Length(Vec4  v) {return length(v);}

Half Length2(VecH2 v) {return Dot(v, v);}
Flt  Length2(Vec2  v) {return Dot(v, v);}
Half Length2(VecH  v) {return Dot(v, v);}
Flt  Length2(Vec   v) {return Dot(v, v);}
Half Length2(VecH4 v) {return Dot(v, v);}
Flt  Length2(Vec4  v) {return Dot(v, v);}

Flt  Dist(Int   a, Int   b) {return Sqrt(Flt(a*a + b*b));}
Half Dist(Half  a, Half  b) {return Sqrt(    a*a + b*b );}
Flt  Dist(Flt   a, Flt   b) {return Sqrt(    a*a + b*b );}
Half Dist(VecH2 a, VecH2 b) {return distance(a, b );}
Flt  Dist(Vec2  a, Vec2  b) {return distance(a, b );}
Half Dist(VecH  a, VecH  b) {return distance(a, b );}
Flt  Dist(Vec   a, Vec   b) {return distance(a, b );}

Half DistH(Int a, Int b) {return Sqrt(Half(a*a + b*b));}

Int  Dist2(Int   a, Int   b) {return a*a + b*b;}
Half Dist2(Half  a, Half  b) {return a*a + b*b;}
Flt  Dist2(Flt   a, Flt   b) {return a*a + b*b;}
Half Dist2(VecH2 a, VecH2 b) {return Length2(a-b);}
Flt  Dist2(Vec2  a, Vec2  b) {return Length2(a-b);}
Half Dist2(VecH  a, VecH  b) {return Length2(a-b);}
Flt  Dist2(Vec   a, Vec   b) {return Length2(a-b);}
Half Dist2(VecH4 a, VecH4 b) {return Length2(a-b);}
Flt  Dist2(Vec4  a, Vec4  b) {return Length2(a-b);}

Half DistPointPlane(VecH2 pos,                  VecH2 plane_normal) {return Dot(pos          , plane_normal);}
Flt  DistPointPlane(Vec2  pos,                  Vec2  plane_normal) {return Dot(pos          , plane_normal);}
Half DistPointPlane(VecH  pos,                  VecH  plane_normal) {return Dot(pos          , plane_normal);}
Flt  DistPointPlane(Vec   pos,                  Vec   plane_normal) {return Dot(pos          , plane_normal);}
Half DistPointPlane(VecH2 pos, VecH2 plane_pos, VecH2 plane_normal) {return Dot(pos-plane_pos, plane_normal);}
Flt  DistPointPlane(Vec2  pos, Vec2  plane_pos, Vec2  plane_normal) {return Dot(pos-plane_pos, plane_normal);}
Half DistPointPlane(VecH  pos, VecH  plane_pos, VecH  plane_normal) {return Dot(pos-plane_pos, plane_normal);}
Flt  DistPointPlane(Vec   pos, Vec   plane_pos, Vec   plane_normal) {return Dot(pos-plane_pos, plane_normal);}

VecH2 PointOnPlane(VecH2 pos,                  VecH2 plane_normal) {return pos-plane_normal*DistPointPlane(pos,            plane_normal);}
Vec2  PointOnPlane(Vec2  pos,                  Vec2  plane_normal) {return pos-plane_normal*DistPointPlane(pos,            plane_normal);}
VecH  PointOnPlane(VecH  pos,                  VecH  plane_normal) {return pos-plane_normal*DistPointPlane(pos,            plane_normal);}
Vec   PointOnPlane(Vec   pos,                  Vec   plane_normal) {return pos-plane_normal*DistPointPlane(pos,            plane_normal);}
VecH2 PointOnPlane(VecH2 pos, VecH2 plane_pos, VecH2 plane_normal) {return pos-plane_normal*DistPointPlane(pos, plane_pos, plane_normal);}
Vec2  PointOnPlane(Vec2  pos, Vec2  plane_pos, Vec2  plane_normal) {return pos-plane_normal*DistPointPlane(pos, plane_pos, plane_normal);}
VecH  PointOnPlane(VecH  pos, VecH  plane_pos, VecH  plane_normal) {return pos-plane_normal*DistPointPlane(pos, plane_pos, plane_normal);}
Vec   PointOnPlane(Vec   pos, Vec   plane_pos, Vec   plane_normal) {return pos-plane_normal*DistPointPlane(pos, plane_pos, plane_normal);}

Half Angle (Half  x, Half y) {return atan2(  y,   x);}
Flt  Angle (Flt   x, Flt  y) {return atan2(  y,   x);}
Half Angle (VecH2 v        ) {return atan2(v.y, v.x);}
Flt  Angle (Vec2  v        ) {return atan2(v.y, v.x);}
Half CosSin(Half  cs       ) {return Sqrt (1-cs*cs );} // NaN
Flt  CosSin(Flt   cs       ) {return Sqrt (1-cs*cs );} // NaN
void CosSin(out Half cos, out Half sin, Half angle) {sincos(angle, sin, cos);}
void CosSin(out Flt  cos, out Flt  sin, Flt  angle) {sincos(angle, sin, cos);}

Half Cross2D(VecH2 a, VecH2 b) {return a.x*b.y - a.y*b.x;}
Flt  Cross2D(Vec2  a, Vec2  b) {return a.x*b.y - a.y*b.x;}

Half CalcZ(VecH2 v) {return Sqrt(Sat(1-Dot(v, v)));} // 1 - v.x*v.x - v.y*v.y, NaN
Flt  CalcZ(Vec2  v) {return Sqrt(Sat(1-Dot(v, v)));} // 1 - v.x*v.x - v.y*v.y, NaN

Half SignFast(Half x) {return (x>=0) ? 1 : -1;} // ignores 0
Flt  SignFast(Flt  x) {return (x>=0) ? 1 : -1;} // ignores 0

Half AngleFull     (Half  angle         ) {return Frac(angle/PI2         )*PI2   ;} // normalize angle to   0..PI2
Flt  AngleFull     (Flt   angle         ) {return Frac(angle/PI2         )*PI2   ;} // normalize angle to   0..PI2
Half AngleNormalize(Half  angle         ) {return Frac(angle/PI2 + PI/PI2)*PI2-PI;} // normalize angle to -PI..PI
Flt  AngleNormalize(Flt   angle         ) {return Frac(angle/PI2 + PI/PI2)*PI2-PI;} // normalize angle to -PI..PI
Half AngleDelta    (Half  from, Half  to) {return AngleNormalize(to-from)        ;} // get angle delta    -PI..PI
Flt  AngleDelta    (Flt   from, Flt   to) {return AngleNormalize(to-from)        ;} // get angle delta    -PI..PI
Half AngleBetween  (VecH2 a   , VecH2 b ) {return AngleDelta(Angle(a), Angle(b)) ;}
Flt  AngleBetween  (Vec2  a   , Vec2  b ) {return AngleDelta(Angle(a), Angle(b)) ;}

VecH2 Perp(VecH2 vec) {return VecH2(vec.y, -vec.x);} // get perpendicular vector
Vec2  Perp(Vec2  vec) {return Vec2 (vec.y, -vec.x);} // get perpendicular vector

VecH Perp(VecH v)
{
   if(Abs(v.x)<Abs(v.z))return VecH(0, v.z, -v.y); // Cross(v, VecH(1, 0,  0));
   else                 return VecH(-v.y, v.x, 0); // Cross(v, VecH(0, 0, -1));
}
Vec Perp(Vec v)
{
   if(Abs(v.x)<Abs(v.z))return Vec(0, v.z, -v.y); // Cross(v, Vec(1, 0,  0));
   else                 return Vec(-v.y, v.x, 0); // Cross(v, Vec(0, 0, -1));
}

VecH2 Rotate(VecH2 vec, VecH2 cos_sin) // rotate vector by cos and sin values obtained from a custom angle
{
   return VecH2(vec.x*cos_sin.x - vec.y*cos_sin.y,
                vec.x*cos_sin.y + vec.y*cos_sin.x);
}
Vec2 Rotate(Vec2 vec, Vec2 cos_sin) // rotate vector by cos and sin values obtained from a custom angle
{
   return Vec2(vec.x*cos_sin.x - vec.y*cos_sin.y,
               vec.x*cos_sin.y + vec.y*cos_sin.x);
}

Half LerpR (Half from, Half to, Half v) {return     (v-from)/(to-from) ;}
Flt  LerpR (Flt  from, Flt  to, Flt  v) {return     (v-from)/(to-from) ;}
VecH LerpR (VecH from, VecH to, VecH v) {return     (v-from)/(to-from) ;}
Half LerpRS(Half from, Half to, Half v) {return Sat((v-from)/(to-from));}
Flt  LerpRS(Flt  from, Flt  to, Flt  v) {return Sat((v-from)/(to-from));}

Half Angle1Fast(Half x, Half y)
{
   Half r=Abs(x)+Abs(y); return r ? ((y>=0) ? 1-x/r : x/r-1) : 0;
}
VecH2 Angle1FastToPos(Half angle_fast)
{
   return (angle_fast>=0) ? (angle_fast<= 1) ? VecH2(1-angle_fast,    angle_fast)
                                             : VecH2(1-angle_fast,  2-angle_fast)
                          : (angle_fast>=-1) ? VecH2(1+angle_fast,    angle_fast)
                                             : VecH2(1+angle_fast, -2-angle_fast);
}
VecH2 ToLen2Angle1Fast(VecH2 v) // 'v'=pos XY, returns (X=length2, Y=Angle1Fast)
{
   Half r=Abs(v.x)+Abs(v.y); return r ? VecH2(Length2(v), (v.y>=0) ? 1-v.x/r : v.x/r-1) : 0;
}
VecH2 FromLen2Angle1Fast(VecH2 v) // 'v'=(X=length2, Y=Angle1Fast), returns pos XY
{
   VecH2  p=Angle1FastToPos(v.y); // never zero
   return p*Sqrt(v.x/Length2(p));
}
/******************************************************************************/
#include "Fast Math.h"
/******************************************************************************/
Half  DistPointLine(VecH2 pos, VecH2 line_pos, VecH2 line_dir) {return Abs(DistPointPlane(pos, line_pos, Perp(line_dir)));}
Half Dist2PointLine(VecH2 pos, VecH2 line_pos, VecH2 line_dir) {return Sqr(DistPointPlane(pos, line_pos, Perp(line_dir)));}

Half DistPointEdge(VecH2 pos, VecH2 edge_a, VecH2 edge_b) // safe in case 'edge' is zero length because 'd' would be zero, and 'DistPointPlane' would be zero
{
   VecH2 d=edge_b-edge_a;
   if(DistPointPlane(pos, edge_a, d)<=0)return Dist         (pos, edge_a);
   if(DistPointPlane(pos, edge_b, d)>=0)return Dist         (pos, edge_b);
                                        return DistPointLine(pos, edge_a, Normalize(d));
}
Half Dist2PointEdge(VecH2 pos, VecH2 edge_a, VecH2 edge_b) // safe in case 'edge' is zero length because 'd' would be zero, and 'DistPointPlane' would be zero
{
   VecH2 d=edge_b-edge_a;
   if(DistPointPlane(pos, edge_a, d)<=0)return Dist2         (pos, edge_a);
   if(DistPointPlane(pos, edge_b, d)>=0)return Dist2         (pos, edge_b);
                                        return Dist2PointLine(pos, edge_a, Normalize(d));
}
/******************************************************************************/
#if 1 // faster (1.6 fps) tested on GeForce 1050 Ti
Vec  Transform(Vec  v, Matrix3  m) {return v.x*m[0] + (v.y*m[1] + (v.z*m[2]));} // transform 'v' vector by 'm' orientation-scale matrix
VecH Transform(VecH v, MatrixH3 m) {return v.x*m[0] + (v.y*m[1] + (v.z*m[2]));} // transform 'v' vector by 'm' orientation-scale matrix
#else // slower (1.0 fps)
Vec  Transform(Vec  v, Matrix3  m) {return mul(v, m);} // transform 'v' vector by 'm' orientation-scale matrix
VecH Transform(VecH v, MatrixH3 m) {return mul(v, m);} // transform 'v' vector by 'm' orientation-scale matrix
#endif

#if 1 // was faster on GeForce 650m, but on GeForce 1050 Ti performance is the same, however keep this version as in other cases 'mul' is slower
Vec  Transform(Vec  v, Matrix  m) {return v.x*m[0] + (v.y*m[1] + (v.z*m[2] + m[3]));} // transform 'v' vector by 'm' orientation-scale-translation matrix, faster version of "mul(Vec4 (v, 1), m)"
VecH Transform(VecH v, Matrix  m) {return v.x*m[0] + (v.y*m[1] + (v.z*m[2] + m[3]));} // transform 'v' vector by 'm' orientation-scale-translation matrix, faster version of "mul(Vec4 (v, 1), m)"
VecH Transform(VecH v, MatrixH m) {return v.x*m[0] + (v.y*m[1] + (v.z*m[2] + m[3]));} // transform 'v' vector by 'm' orientation-scale-translation matrix, faster version of "mul(VecH4(v, 1), m)"
Vec4 Transform(Vec  v, Matrix4 m) {return v.x*m[0] + (v.y*m[1] + (v.z*m[2] + m[3]));} // transform 'v' vector by 'm' 4x4                           matrix, faster version of "mul(Vec4 (v, 1), m)"
#else
Vec  Transform(Vec  v, Matrix  m) {return mul(Vec4 (v, 1), m);} // transform 'v' vector by 'm' orientation-scale-translation matrix
VecH Transform(VecH v, Matrix  m) {return mul(VecH4(v, 1), m);} // transform 'v' vector by 'm' orientation-scale-translation matrix
VecH Transform(VecH v, MatrixH m) {return mul(VecH4(v, 1), m);} // transform 'v' vector by 'm' orientation-scale-translation matrix
Vec4 Transform(Vec  v, Matrix4 m) {return mul(Vec4 (v, 1), m);} // transform 'v' vector by 'm' 4x4                           matrix
#endif

#if 1 // faster (1.6 fps) tested on GeForce 1050 Ti
Vec  Transform3(Vec  v, Matrix  m) {return v.x*m[0] + (v.y*m[1] + (v.z*m[2]));} // transform 'v' vector by 'm' orientation-scale matrix
VecH Transform3(VecH v, MatrixH m) {return v.x*m[0] + (v.y*m[1] + (v.z*m[2]));} // transform 'v' vector by 'm' orientation-scale matrix
VecH Transform3(VecH v, Matrix  m) {return v.x*m[0] + (v.y*m[1] + (v.z*m[2]));} // transform 'v' vector by 'm' orientation-scale matrix, TODO: #ShaderHalf would it be faster to cast 'v' to 'Vec' first? Mixing precisions is not perfect however alternative would require to store matrixes in additional half precision but that would slow down (calculating on CPU side and uploading to GPU)
#else // slower (1.0 fps)
Vec  Transform3(Vec  v, Matrix  m) {return mul(v, (Matrix3 )m);} // transform 'v' vector by 'm' orientation-scale matrix
VecH Transform3(VecH v, MatrixH m) {return mul(v, (MatrixH3)m);} // transform 'v' vector by 'm' orientation-scale matrix
VecH Transform3(VecH v, Matrix  m) {return mul(v, (MatrixH3)m);} // transform 'v' vector by 'm' orientation-scale matrix
#endif

#if 1 // faster 4.3 fps
Vec  TransformTP(Vec  v, Matrix3  m) {return mul(m, v);} // transform 'v' vector by transposed 'm' orientation-scale matrix
VecH TransformTP(VecH v, MatrixH3 m) {return mul(m, v);} // transform 'v' vector by transposed 'm' orientation-scale matrix
Vec  TransformTP(Vec  v, MatrixH3 m) {return mul(m, v);} // transform 'v' vector by transposed 'm' orientation-scale matrix
#else // slower 3.2 fps
Vec  TransformTP(Vec  v, Matrix3  m) {return Vec(Dot(v, m[0]), Dot(v, m[1]), Dot(v, m[2]));} // transform 'v' vector by transposed 'm' orientation-scale matrix
VecH TransformTP(VecH v, MatrixH3 m) {return Vec(Dot(v, m[0]), Dot(v, m[1]), Dot(v, m[2]));} // transform 'v' vector by transposed 'm' orientation-scale matrix
Vec  TransformTP(Vec  v, MatrixH3 m) {return Vec(Dot(v, m[0]), Dot(v, m[1]), Dot(v, m[2]));} // transform 'v' vector by transposed 'm' orientation-scale matrix
#endif
/******************************************************************************/
#if !GL // FIXME broken for Mac - https://feedbackassistant.apple.com/feedback/7116525
#include "!Set Prec Struct.h"
BUFFER_I(ObjMatrix, SBI_OBJ_MATRIX) // !! WARNING: this CB is dynamically resized, do not add other members !!
   Matrix ViewMatrix[MAX_MATRIX]; // object transformation matrixes relative to view space (this is object matrix * inversed camera matrix = object matrix / camera matrix)
BUFFER_END

BUFFER_I(ObjMatrixPrev, SBI_OBJ_MATRIX_PREV) // !! WARNING: this CB is dynamically resized, do not add other members !!
   Matrix ViewMatrixPrev[MAX_MATRIX]; // object transformation matrixes relative to view space (this is object matrix * inversed camera matrix = object matrix / camera matrix) for previous frame
BUFFER_END
#include "!Set Prec Default.h"

Vec  TransformPos(Vec  pos, UInt mtrx=0) {return Transform (pos, ViewMatrix[mtrx]);}
VecH TransformDir(VecH dir, UInt mtrx=0) {return Transform3(dir, ViewMatrix[mtrx]);}

Vec  TransformPosPrev(Vec  pos, UInt mtrx=0) {return Transform (pos, ViewMatrixPrev[mtrx]);}
VecH TransformDirPrev(VecH dir, UInt mtrx=0) {return Transform3(dir, ViewMatrixPrev[mtrx]);}

Vec  TransformPos(Vec  pos, VecU4 bone, Vec4  weight) {return weight.x*Transform (pos, ViewMatrix[bone.x]) + weight.y*Transform (pos, ViewMatrix[bone.y]) + weight.z*Transform (pos, ViewMatrix[bone.z]) + weight.w*Transform (pos, ViewMatrix[bone.w]);}
VecH TransformDir(VecH dir, VecU4 bone, VecH4 weight) {return weight.x*Transform3(dir, ViewMatrix[bone.x]) + weight.y*Transform3(dir, ViewMatrix[bone.y]) + weight.z*Transform3(dir, ViewMatrix[bone.z]) + weight.w*Transform3(dir, ViewMatrix[bone.w]);} // no need HP for dirs

Vec  TransformPosPrev(Vec  pos, VecU4 bone, Vec4  weight) {return weight.x*Transform (pos, ViewMatrixPrev[bone.x]) + weight.y*Transform (pos, ViewMatrixPrev[bone.y]) + weight.z*Transform (pos, ViewMatrixPrev[bone.z]) + weight.w*Transform (pos, ViewMatrixPrev[bone.w]);}
VecH TransformDirPrev(VecH dir, VecU4 bone, VecH4 weight) {return weight.x*Transform3(dir, ViewMatrixPrev[bone.x]) + weight.y*Transform3(dir, ViewMatrixPrev[bone.y]) + weight.z*Transform3(dir, ViewMatrixPrev[bone.z]) + weight.w*Transform3(dir, ViewMatrixPrev[bone.w]);} // no need HP for dirs

Vec ViewMatrixX  (UInt mtrx=0) {return ViewMatrix[mtrx][0];}
Vec ViewMatrixY  (UInt mtrx=0) {return ViewMatrix[mtrx][1];}
Vec ViewMatrixZ  (UInt mtrx=0) {return ViewMatrix[mtrx][2];}
Vec ViewMatrixPos(UInt mtrx=0) {return ViewMatrix[mtrx][3];}

Vec ViewMatrixPrevX  (UInt mtrx=0) {return ViewMatrixPrev[mtrx][0];}
Vec ViewMatrixPrevY  (UInt mtrx=0) {return ViewMatrixPrev[mtrx][1];}
Vec ViewMatrixPrevZ  (UInt mtrx=0) {return ViewMatrixPrev[mtrx][2];}
Vec ViewMatrixPrevPos(UInt mtrx=0) {return ViewMatrixPrev[mtrx][3];}

Matrix GetViewMatrix() {return ViewMatrix[0];}
#else // Mac currently has no known workaround and must use this
#include "!Set Prec Struct.h"
BUFFER_I(ObjMatrix, SBI_OBJ_MATRIX) // !! WARNING: this CB is dynamically resized, do not add other members !!
   Vec4 ViewMatrix[MAX_MATRIX*3]; // object transformation matrixes relative to view space (this is object matrix * inversed camera matrix = object matrix / camera matrix)
BUFFER_END

BUFFER_I(ObjMatrixPrev, SBI_OBJ_MATRIX_PREV) // !! WARNING: this CB is dynamically resized, do not add other members !!
   Vec4 ViewMatrixPrev[MAX_MATRIX*3]; // object transformation matrixes relative to view space (this is object matrix * inversed camera matrix = object matrix / camera matrix) for previous frame
BUFFER_END
#include "!Set Prec Default.h"

Vec TransformPos(Vec pos)
{
   return Vec(Dot(pos, ViewMatrix[0].xyz) + ViewMatrix[0].w,
              Dot(pos, ViewMatrix[1].xyz) + ViewMatrix[1].w,
              Dot(pos, ViewMatrix[2].xyz) + ViewMatrix[2].w);
}
VecH TransformDir(VecH dir)
{
   return VecH(Dot(dir, ViewMatrix[0].xyz),
               Dot(dir, ViewMatrix[1].xyz),
               Dot(dir, ViewMatrix[2].xyz));
}

Vec TransformPos(Vec pos, UInt mtrx)
{
   mtrx*=3;
   return Vec(Dot(pos, ViewMatrix[mtrx  ].xyz) + ViewMatrix[mtrx  ].w,
              Dot(pos, ViewMatrix[mtrx+1].xyz) + ViewMatrix[mtrx+1].w,
              Dot(pos, ViewMatrix[mtrx+2].xyz) + ViewMatrix[mtrx+2].w);
}
VecH TransformDir(VecH dir, UInt mtrx)
{
   mtrx*=3;
   return VecH(Dot(dir, ViewMatrix[mtrx  ].xyz),
               Dot(dir, ViewMatrix[mtrx+1].xyz),
               Dot(dir, ViewMatrix[mtrx+2].xyz));
}

Vec  TransformPos(Vec  pos, VecU4 bone, Vec4  weight) {return weight.x*TransformPos(pos, bone.x) + weight.y*TransformPos(pos, bone.y) + weight.z*TransformPos(pos, bone.z) + weight.w*TransformPos(pos, bone.w);}
VecH TransformDir(VecH dir, VecU4 bone, VecH4 weight) {return weight.x*TransformDir(dir, bone.x) + weight.y*TransformDir(dir, bone.y) + weight.z*TransformDir(dir, bone.z) + weight.w*TransformDir(dir, bone.w);} // no need HP for dirs

// -

Vec TransformPosPrev(Vec pos)
{
   return Vec(Dot(pos, ViewMatrixPrev[0].xyz) + ViewMatrixPrev[0].w,
              Dot(pos, ViewMatrixPrev[1].xyz) + ViewMatrixPrev[1].w,
              Dot(pos, ViewMatrixPrev[2].xyz) + ViewMatrixPrev[2].w);
}
VecH TransformDirPrev(VecH dir)
{
   return VecH(Dot(dir, ViewMatrixPrev[0].xyz),
               Dot(dir, ViewMatrixPrev[1].xyz),
               Dot(dir, ViewMatrixPrev[2].xyz));
}

Vec TransformPosPrev(Vec pos, UInt mtrx)
{
   mtrx*=3;
   return Vec(Dot(pos, ViewMatrixPrev[mtrx  ].xyz) + ViewMatrixPrev[mtrx  ].w,
              Dot(pos, ViewMatrixPrev[mtrx+1].xyz) + ViewMatrixPrev[mtrx+1].w,
              Dot(pos, ViewMatrixPrev[mtrx+2].xyz) + ViewMatrixPrev[mtrx+2].w);
}
VecH TransformDirPrev(VecH dir, UInt mtrx)
{
   mtrx*=3;
   return VecH(Dot(dir, ViewMatrixPrev[mtrx  ].xyz),
               Dot(dir, ViewMatrixPrev[mtrx+1].xyz),
               Dot(dir, ViewMatrixPrev[mtrx+2].xyz));
}

Vec  TransformPosPrev(Vec  pos, VecU4 bone, Vec4  weight) {return weight.x*TransformPosPrev(pos, bone.x) + weight.y*TransformPosPrev(pos, bone.y) + weight.z*TransformPosPrev(pos, bone.z) + weight.w*TransformPosPrev(pos, bone.w);}
VecH TransformDirPrev(VecH dir, VecU4 bone, VecH4 weight) {return weight.x*TransformDirPrev(dir, bone.x) + weight.y*TransformDirPrev(dir, bone.y) + weight.z*TransformDirPrev(dir, bone.z) + weight.w*TransformDirPrev(dir, bone.w);} // no need HP for dirs

// -

Vec ViewMatrixX  () {return Vec(ViewMatrix[0].x, ViewMatrix[1].x, ViewMatrix[2].x);}
Vec ViewMatrixY  () {return Vec(ViewMatrix[0].y, ViewMatrix[1].y, ViewMatrix[2].y);}
Vec ViewMatrixZ  () {return Vec(ViewMatrix[0].z, ViewMatrix[1].z, ViewMatrix[2].z);}
Vec ViewMatrixPos() {return Vec(ViewMatrix[0].w, ViewMatrix[1].w, ViewMatrix[2].w);}

Vec ViewMatrixPrevPos() {return Vec(ViewMatrixPrev[0].w, ViewMatrixPrev[1].w, ViewMatrixPrev[2].w);}

Vec ViewMatrixY  (UInt mtrx) {mtrx*=3; return Vec(ViewMatrix[mtrx].y, ViewMatrix[mtrx+1].y, ViewMatrix[mtrx+2].y);}
Vec ViewMatrixPos(UInt mtrx) {mtrx*=3; return Vec(ViewMatrix[mtrx].w, ViewMatrix[mtrx+1].w, ViewMatrix[mtrx+2].w);}

Vec ViewMatrixPrevY  (UInt mtrx) {mtrx*=3; return Vec(ViewMatrixPrev[mtrx].y, ViewMatrixPrev[mtrx+1].y, ViewMatrixPrev[mtrx+2].y);}
Vec ViewMatrixPrevPos(UInt mtrx) {mtrx*=3; return Vec(ViewMatrixPrev[mtrx].w, ViewMatrixPrev[mtrx+1].w, ViewMatrixPrev[mtrx+2].w);}

Matrix GetViewMatrix() {Matrix m; m[0]=ViewMatrixX(); m[1]=ViewMatrixY(); m[2]=ViewMatrixZ(); m[3]=ViewMatrixPos(); return m;}
#endif
/******************************************************************************/
Vec  MatrixX(Matrix3  m) {return m[0];}
VecH MatrixX(MatrixH3 m) {return m[0];}
Vec  MatrixY(Matrix3  m) {return m[1];}
VecH MatrixY(MatrixH3 m) {return m[1];}
Vec  MatrixZ(Matrix3  m) {return m[2];}
VecH MatrixZ(MatrixH3 m) {return m[2];}

Vec  MatrixX  (Matrix  m) {return m[0];}
VecH MatrixX  (MatrixH m) {return m[0];}
Vec  MatrixY  (Matrix  m) {return m[1];}
VecH MatrixY  (MatrixH m) {return m[1];}
Vec  MatrixZ  (Matrix  m) {return m[2];}
VecH MatrixZ  (MatrixH m) {return m[2];}
Vec  MatrixPos(Matrix  m) {return m[3];}
VecH MatrixPos(MatrixH m) {return m[3];}

Vec ObjWorldPos    (UInt mtrx=0) {return Transform(ViewMatrixPos    (mtrx), CamMatrix    );} // get the world position of the object matrix
Vec ObjWorldPosPrev(UInt mtrx=0) {return Transform(ViewMatrixPrevPos(mtrx), CamMatrixPrev);} // get the world position of the object matrix in previous frame
/******************************************************************************/
Vec4 Project(Vec pos)
{
#if 1 // 2x faster on Intel (made no difference for GeForce)
   return Vec4(pos.x*ProjMatrix[0].x + pos.z*ProjMatrix[2].x, // "pos.z*ProjMatrix[2].x" only needed for Stereo or Temporal
               pos.y*ProjMatrix[1].y + pos.z*ProjMatrix[2].y, // "pos.z*ProjMatrix[2].y" only needed for           Temporal
                                       pos.z*ProjMatrix[2].z + ProjMatrix[3].z,
                                       pos.z*ProjMatrix[2].w + ProjMatrix[3].w);
#else // slower
   return Transform(pos, ProjMatrix);
#endif
}
Vec ProjectXYW(Vec pos)
{
#if 1 // 2x faster on Intel (made no difference for GeForce)
   return Vec(pos.x*ProjMatrix[0].x + pos.z*ProjMatrix[2].x, // "pos.z*ProjMatrix[2].x" only needed for Stereo or Temporal
              pos.y*ProjMatrix[1].y + pos.z*ProjMatrix[2].y, // "pos.z*ProjMatrix[2].y" only needed for           Temporal
                                    //pos.z*ProjMatrix[2].z + ProjMatrix[3].z,
                                      pos.z*ProjMatrix[2].w + ProjMatrix[3].w);
#else // slower
   return Transform(pos, ProjMatrix).xyw;
#endif
}
Vec ProjectPrevXYW(Vec pos)
{
#if 1 // 2x faster on Intel (made no difference for GeForce)
   return Vec(pos.x*ProjMatrixPrev[0].x + pos.z*ProjMatrixPrev[2].x, // "pos.z*ProjMatrixPrev[2].x" only needed for Stereo or Temporal
              pos.y*ProjMatrixPrev[1].y + pos.z*ProjMatrixPrev[2].y, // "pos.z*ProjMatrixPrev[2].y" only needed for           Temporal
                                        //pos.z*ProjMatrixPrev[2].z + ProjMatrixPrev[3].z,
                                          pos.z*ProjMatrixPrev[2].w + ProjMatrixPrev[3].w);
#else // slower
   return Transform(pos, ProjMatrixPrev).xyw;
#endif
}
/******************************************************************************/
Vec2 UVClamp      (Vec2 uv, Bool do_clamp=true) {return do_clamp ? Mid(uv, ImgClamp.xy, ImgClamp.zw) : uv;} // clamp  UV
Vec2 UVInView     (Vec2 uv, Bool view_full    ) {return         UVClamp(uv, !view_full);                  } // return UV inside viewport, 'view_full'=if viewport is full
Bool UVInsideView (Vec2 uv                    ) {return all(uv==UVClamp(uv));                             } // if UV is  inside viewport
Bool UVOutsideView(Vec2 uv                    ) {return any(uv!=UVClamp(uv));                             } // if UV is outside viewport "any(uv<ImgClamp.xy || uv>ImgClamp.zw)"

Flt ViewportClamp(Vec2 pos, Vec2 dir) // this can be used to clamp 'pos' to Viewport rectangle along 'dir', works OK if 'pos' is both inside and outside viewport, returns fraction of 'dir' to move from 'pos' to be inside viewport, assuming 'dir' points towards viewport (dir=start-pos where start=some point inside viewport)
{
#if 0
   if(pos.x>1)
   {
      Flt frac=(1-start.x)/(pos.x-start.x);
      pos.x =1;
      pos.y-=(1-frac)*(pos.y-start.y);
   }else
   if(pos.x<0)
   {
      Flt frac=(start.x)/(start.x-pos.x);
      pos.x =0;
      pos.y-=(1-frac)*(pos.y-start.y);
   }

   if(pos.y>1)
   {
      Flt frac=(1-start.y)/(pos.y-start.y);
      pos.y =1;
      pos.x-=(1-frac)*(pos.x-start.x);
   }else
   if(pos.y<0)
   {
      Flt frac=(start.y)/(start.y-pos.y);
      pos.y =0;
      pos.x-=(1-frac)*(pos.x-start.x);
   }
#else
   return Max(Max(Vec2(0, 0), Abs(pos-Viewport.center)-Viewport.size/2)/Abs(dir)); // Max(Max(0, Abs(pos.x-0.5)-0.5)/Abs(pos.x-start.x), Max(0, Abs(pos.y-0.5)-0.5)/Abs(pos.y-start.y));
#endif
}
/******************************************************************************/
Vec2 FracToPosXY(Vec2 frac) // return view space xy position at z=1
{
   return frac * Viewport.FracToPosXY.xy + Viewport.FracToPosXY.zw;
}
Vec2 UVToPosXY(Vec2 uv) // return view space xy position at z=1
{
   return uv * Viewport.UVToPosXY.xy + Viewport.UVToPosXY.zw;
}
Vec2 UVToPosXY(Vec2 uv, Flt z) // return view space xy position at z='z'
{
   return UVToPosXY(uv)*z;
}
/******************************************************************************/
Vec2 ProjectedPosToUV(Vec4 projected_pos) // prefer using 'PixelToUV' if possible, returns (0,0)..(1,1) range
{
   return (projected_pos.xy/projected_pos.w) * Viewport.ProjectedPosToUV.xy + Viewport.ProjectedPosToUV.zw;
}
Vec2 ProjectedPosXYWToUV(Vec projected_pos_xyw) // prefer using 'PixelToUV' if possible, returns (0,0)..(1,1) range, same as 'ProjectedPosToUV' except Z channel is skipped because it's not needed
{
   return (projected_pos_xyw.xy/projected_pos_xyw.z) * Viewport.ProjectedPosToUV.xy + Viewport.ProjectedPosToUV.zw;
}
Vec2 PosToUV(Vec view_pos)
{
   return ProjectedPosXYWToUV(ProjectXYW(view_pos)); // TODO: can this be done faster?
}
Vec2 PixelToUV(Vec4 pixel) // faster and more accurate than 'ProjectedPosToUV', returns (0,0)..(1,1) range
{
   return pixel.xy*RTSize.xy;
}
Vec2 UVToPixel(Vec2 uv)
{
   return uv*RTSize.zw;
}
/******************************************************************************/
Vec2 DownSamplePointUV(Vec2 uv) // !! needs 'ImgSize' !! this function will return tex coordinates for downsampling with point filtering
{ /* if downsampling 1x (no downsample, just copy), then 'uv' will be located exactly at the center of 1x1 image texel  (  correct), no offset needed
     if downsampling 2x                             then 'uv' will be located exactly at the center of 2x2 image texels (incorrect), due to precision issues point filter might sometimes return any of them         , to fix this problem, offset tex coordinates
     if downsampling 3x                             then 'uv' will be located exactly at the center of 3x3 image texels (  correct), no offset needed
     if downsampling 4x                             then 'uv' will be located exactly at the center of 4x4 image texels (incorrect), due to precision issues point filter might sometimes return any of the inner 2x2, to fix this problem, offset tex coordinates
   */
   return ImgSize.xy*-0.25+uv; // uv-ImgSize.xy/4; instead of doing /2, do /4 because if there's no downsampling (scale=1) then UV is exactly at the center, and moving by /2 we would actually put it at biggest risk (at border between texels), so just move by /4
}
/******************************************************************************/
// VELOCITIES
/******************************************************************************
struct Velocity
{
   VecH lin, //  linear, use VecH to allow velocity functions work faster
        ang; // angular, use VecH to allow velocity functions work faster
};

BUFFER_I(ObjVel, SBI_OBJ_VEL) // !! WARNING: this CB is dynamically resized, do not add other members !!
   Velocity ObjVel[MAX_MATRIX]; // object velocities (actually deltas not velocities)
BUFFER_END

VecH GetVel(VecH local_pos, UInt mtrx=0) // does not include velocity from Camera Angle Vel
{
   return             ObjVel[mtrx].lin
  +TransformDir(Cross(ObjVel[mtrx].ang, local_pos), mtrx);
}
Vec GetCamAngVel(Vec view_pos)
{
   return -Cross(CamAngVel, view_pos);
}
Vec GetObjVel(VecH local_pos, Vec view_pos, UInt mtrx=0)
{
   return GetVel      (local_pos, mtrx)
         +GetCamAngVel(view_pos);
}
Vec GetBoneVel(VecH local_pos, Vec view_pos, VecU bone, VecH weight) // no need HP for vels
{
   return (GetVel      (local_pos, bone.x)*weight.x
          +GetVel      (local_pos, bone.y)*weight.y
          +GetVel      (local_pos, bone.z)*weight.z)
          +GetCamAngVel( view_pos);
}
/******************************************************************************/
VecH2 GetMotionUV(Vec projected_prev_pos_xyw, Vec2 uv, Bool geom=true) // 'geom'=if generated using 3D geometry, which could result in position being behind the camera
{
   if(geom)projected_prev_pos_xyw.z=Max(projected_prev_pos_xyw.z, Viewport.from); // prevent division by <=0 (needed for previous positions that are behind the camera)
   VecH2  vel=uv-ProjectedPosXYWToUV(projected_prev_pos_xyw); // cur-prev #MotionDir
   return vel;
}
VecH2 GetMotion(Vec projected_prev_pos_xyw, Vec4 pixel, Bool geom=true) {return GetMotionUV(projected_prev_pos_xyw, PixelToUV(pixel), geom);}
/******************************************************************************/
// DEPTH
/******************************************************************************/
Flt DelinearizeDepth(Flt z, Bool perspective=true)
{
   Flt a=ProjMatrix[2][2], // ProjMatrix.z  .z
       b=ProjMatrix[3][2], // ProjMatrix.pos.z
       w=(perspective ? (z*a+b)/z : (z*a+REVERSE_DEPTH));

#if GL
   w=w*0.5+0.5; // #glClipControl
#endif

   return w;
}
Flt LinearizeDepth(Flt w, Bool perspective=true)
{
   Flt a=ProjMatrix[2][2], // ProjMatrix.z  .z
       b=ProjMatrix[3][2]; // ProjMatrix.pos.z

#if GL
   w=w*2-1; // #glClipControl
#endif

   // w   = (z*a+b)/z
   // w*z = a*z + b
   // a*z - w*z + b = 0
   // (a-w)*z + b = 0
   // z = -b/(a-w)
   // z =  b/(w-a)
   // precise: if(perspective && w>=1)return Viewport.range;
   return perspective ? b/(w-a) : (w-REVERSE_DEPTH)/a;
}
/******************************************************************************/
// Perspective: pos.xy=pos_xy*z
// Orthogonal : pos.xy=pos_xy
Vec GetPos(Flt z, Vec2 pos_xy) // Get Viewspace Position at 'z' depth, 'pos_xy'=known xy position at depth=1
{
   Vec pos;         pos.z =z;
 //if(ORTHO_SUPPORT)pos.xy=pos_xy*(Viewport.ortho ? 1 : pos.z);else
                    pos.xy=pos_xy*pos.z;
   return pos;
}
Vec GetPosPix   (VecI2 pixel,              Vec2 pos_xy) {return GetPos(TexDepthPix   (pixel        ),     pos_xy   );}
Vec GetPosMS    (VecI2 pixel, UInt sample, Vec2 pos_xy) {return GetPos(TexDepthMS    (pixel, sample),     pos_xy   );}
Vec GetPosPoint (Vec2  uv                             ) {return GetPos(TexDepthPoint (uv           ), UVToPosXY(uv));} // Get Viewspace Position at 'uv' coordinates
Vec GetPosPoint (Vec2  uv   ,              Vec2 pos_xy) {return GetPos(TexDepthPoint (uv           ),     pos_xy   );} // Get Viewspace Position at 'uv' coordinates, 'pos_xy'=known xy position at depth=1
Vec GetPosLinear(Vec2  uv                             ) {return GetPos(TexDepthLinear(uv           ), UVToPosXY(uv));} // Get Viewspace Position at 'uv' coordinates
Vec GetPosLinear(Vec2  uv   ,              Vec2 pos_xy) {return GetPos(TexDepthLinear(uv           ),     pos_xy   );} // Get Viewspace Position at 'uv' coordinates, 'pos_xy'=known xy position at depth=1
/******************************************************************************/
// sRGB
/******************************************************************************/
#define SRGBToLinearFast Sqr  // simple and fast sRGB -> Linear conversion
#define LinearToSRGBFast Sqrt // simple and fast Linear -> sRGB conversion

Half SRGBToLinear(Half s) {return (s<=0.04045  ) ? s/12.92 : Pow(s/1.055+0.055/1.055, 2.4);} // convert 0..1 srgb   to 0..1 linear, (s+0.055)/1.055
Half LinearToSRGB(Half l) {return (l<=0.0031308) ? l*12.92 : Pow(l, 1/2.4)*1.055-0.055    ;} // convert 0..1 linear to 0..1 srgb

// a little bit faster approximation, exp calculated to preserve sRGB 0.5 (grey) using: Dbl l=SRGBToLinear(0.5), exp=Ln(l)/Ln(0.5), s=Pow(l, 1/exp);
Half SRGBToLinear1(Half s) {return Pow(s,   2.2240399129654920);}
Half LinearToSRGB1(Half l) {return Pow(l, 1/2.2240399129654920);}

VecH2 SRGBToLinear(VecH2 s) {return VecH2(SRGBToLinear(s.x), SRGBToLinear(s.y));}
VecH2 LinearToSRGB(VecH2 l) {return VecH2(LinearToSRGB(l.x), LinearToSRGB(l.y));}

VecH SRGBToLinear(VecH s) {return VecH(SRGBToLinear(s.x), SRGBToLinear(s.y), SRGBToLinear(s.z));}
VecH LinearToSRGB(VecH l) {return VecH(LinearToSRGB(l.x), LinearToSRGB(l.y), LinearToSRGB(l.z));}

VecH SRGBToLinear1(VecH s) {return VecH(SRGBToLinear1(s.x), SRGBToLinear1(s.y), SRGBToLinear1(s.z));}
VecH LinearToSRGB1(VecH l) {return VecH(LinearToSRGB1(l.x), LinearToSRGB1(l.y), LinearToSRGB1(l.z));}

VecH4 SRGBToLinear(VecH4 s) {return VecH4(SRGBToLinear(s.x), SRGBToLinear(s.y), SRGBToLinear(s.z), s.w);}
VecH4 LinearToSRGB(VecH4 l) {return VecH4(LinearToSRGB(l.x), LinearToSRGB(l.y), LinearToSRGB(l.z), l.w);}

Half LinearLumOfLinearColor(VecH l) {return                  Dot(                 l , ColorLumWeight2) ;}
Half LinearLumOfSRGBColor  (VecH s) {return                  Dot(SRGBToLinearFast(s), ColorLumWeight2) ;}
Half   SRGBLumOfSRGBColor  (VecH s) {return LinearToSRGBFast(Dot(SRGBToLinearFast(s), ColorLumWeight2));}
/******************************************************************************/
// DITHER
/******************************************************************************/
Half Noise1D_4(VecI2 pixel) // 4 steps, -0.375 .. 0.375
{
 //return Noise4[(pixel.x&1) + (pixel.y&1)*2];
   return ((pixel.x*2 + (pixel.y&1)*3)&3)*(1.0/4) - 0.375;
}
Half Noise1D_16(VecI2 pixel) // 16 steps, -0.46875 .. 0.46875
{
   return Noise16[(pixel.x&3) + (pixel.y&3)*4];
}
Half Noise1D_64(VecI2 pixel) // 64 steps, -0.4921875 .. 0.4921875
{
   return Noise64[(pixel.x&7) + (pixel.y&7)*8];
}
Half Noise1D_Blue(VecI2 pixel) // many steps, -1.0 .. 1.0
{
   return ImgNoise[pixel&(NOISE_IMAGE_RES-1)];
}
Half Noise1D(Vec2 pixel) // many steps, -0.5 .. 0.5
{
#if 0 // low
   return Frac(Dot(pixel, Vec2(1.0, 0.5)/3))-0.5;
#elif 0 // medium
   return Frac(Dot(pixel, Vec2(3, 1)/8))-0.5;
#elif 1 // good
   return Frac(Dot(pixel, Vec2(1.6, 1)*0.25))-0.5;
#endif
}
VecH Noise3D(Vec2 pixel) // -0.5 .. 0.5
{
   Flt noise=Dot(Vec2(171, 231), pixel);
   return Frac(noise/Vec(103, 71, 96.9999))-0.5; // 96.9999 instead of 97 fixes blue artifacts (there are some inconsistent white lines when using 97)
}
Vec2 Noise2D(VecI2 pixel) // (-0.5, -0.5) .. (0.5, 0.5)
{
   pixel&=1;
#if 0
   Vec2 p=pixel-0.5; Vec2 cos_sin; CosSin(cos_sin.x, cos_sin.y, PI_4); // PI_4 is the best angle because it looks like doubling resolution
   return Rotate(p, cos_sin)*(0.5/SQRT2_2); // 0.5 is the best scale, it puts texels exactly in the middle
#else // optimized
   Vec2 p=pixel*0.5-0.25; return Vec2(p.x-p.y, p.x+p.y);
#endif
}
void ApplyDither(inout VecH col, Vec2 pixel, Bool linear_gamma=LINEAR_GAMMA)
{
   if(linear_gamma)col=LinearToSRGBFast(col);
   col+=Noise3D(pixel)*(1.0/255); // use 1.0 because any bigger generates too much noise on repeated dither calls, 4.0/3 was the biggest value that didn't introduce artifacts (1.35 already had them), 1.5 is good however has some artifacts
   if(linear_gamma)col=SRGBToLinearFast(col);
}
/******************************************************************************/
struct VtxInput // Vertex Input, use this class to access vertex data in vertex shaders
{
#include "!Set Prec Struct.h"
#if GL || VULKAN
   // !! LOC, ATTR numbers AND list order, must be in sync with GL_VTX_SEMANTIC !!
   LOC( 0) Vec4  _pos     :ATTR0 ;
   LOC( 1) Vec   _hlp     :ATTR1 ;
   LOC( 2) VecH  _nrm     :ATTR2 ;
   LOC( 3) VecH4 _tan     :ATTR3 ;
   LOC( 4) Vec2  _uv      :ATTR4 ;
   LOC( 5) Vec2  _uv1     :ATTR5 ;
   LOC( 6) Vec2  _uv2     :ATTR6 ;
   LOC( 7) Vec2  _uv3     :ATTR7 ;
   LOC( 8) Half  _size    :ATTR8 ;
   LOC( 9) Vec4  _bone    :ATTR9 ; // this has to be Vec4 because VecI4 and VecU4 don't work for some reason
   LOC(10) Vec4  _weight  :ATTR10; // this has to be Vec4 instead of VecH4 because of 2 reasons, we need sum of weights to be equal to 1.0 (half's can't do that), also when converting to GLSL the explicit casts to "Vec4 weight" precision are optimized away and perhaps some GLSL compilers may want to perform optimizations where Half*Vec4 is converted to VecH4 which would destroy precision for skinned characters
   LOC(11) VecH4 _material:ATTR11;
   LOC(12) VecH4 _color   :ATTR12;
   LOC(13) VecU2 _face_id :ATTR13;
#else
   // !! IF MAKING ANY CHANGE (EVEN PRECISION) THEN DON'T FORGET TO RE-CREATE 'VS_Code' FOR 'CreateInputLayout', see #VTX_INPUT_LAYOUT !!
   Vec4  _pos     :POSITION0   ;
   Vec   _hlp     :POSITION1   ;
   VecH  _nrm     :NORMAL      ;
   VecH4 _tan     :TANGENT     ;
   Vec2  _uv      :UV0         ;
   Vec2  _uv1     :UV1         ;
   Vec2  _uv2     :UV2         ;
   Vec2  _uv3     :UV3         ;
   Half  _size    :PSIZE       ;
   VecU4 _bone    :BLENDINDICES;
   Vec4  _weight  :BLENDWEIGHT ; // this has to be Vec4 instead of VecH4 because of 2 reasons, we need sum of weights to be equal to 1.0 (half's can't do that), also when converting to GLSL the explicit casts to "Vec4 weight" precision are optimized away and perhaps some GLSL compilers may want to perform optimizations where Half*Vec4 is converted to VecH4 which would destroy precision for skinned characters
   VecH4 _material:COLOR0      ;
   VecH4 _color   :COLOR1      ;
   VecU2 _face_id :FACE_ID     ;
#endif
   UInt  _id      :SV_VertexID;
   UInt  _instance:SV_InstanceID;
#include "!Set Prec Default.h"

   VecH  nrm      (                                              ) {return _nrm                                                                  ;} // vertex normal
   VecH  tan      (VecH nrm          , Int heightmap=HEIGHTMAP_NO) {return heightmap ? VecH(1-nrm.x*nrm.x, -nrm.y*nrm.x, -nrm.z*nrm.x) : _tan.xyz;} // vertex tangent, for heightmap: PointOnPlane(Vec(1,0,0), nrm()), Vec(1,0,0)-nrm*nrm.x, which gives a perpendicular however not Normalized !!
   VecH  bin      (VecH nrm, VecH tan, Int heightmap=HEIGHTMAP_NO) {return heightmap ? Cross(nrm, tan) : Cross(nrm, tan)*_tan.w                  ;} // binormal from transformed normal and tangent
   Vec2  pos2     (                                              ) {return _pos.xy                                                               ;} // vertex position
   Vec   pos      (                                              ) {return _pos.xyz                                                              ;} // vertex position
   Vec4  pos4     (                                              ) {return _pos                                                                  ;} // vertex position in Vec4(pos.xyz, 1) format
   Flt   posZ     (                                              ) {return _pos.z                                                                ;} // vertex position Z
   Vec   hlp      (                                              ) {return _hlp                                                                  ;} // helper position
   VecH  tan      (                                              ) {return _tan.xyz                                                              ;} // helper position
   Vec2  uv       (                    Int heightmap=HEIGHTMAP_NO) {return (heightmap==HEIGHTMAP_FLAT) ? Vec2(_pos.x, -_pos.z) : (heightmap==HEIGHTMAP_SPHERE) ? VtxUVScale*AtanFast(Vec2(_pos.x, -_pos.z)/_pos.y) : _uv;} // tex coords 0
   Vec2  uv1      (                                              ) {return _uv1                                                                  ;} // tex coords 1
   Vec2  uv2      (                                              ) {return _uv2                                                                  ;} // tex coords 2
   Vec2  uv3      (                                              ) {return _uv3                                                                  ;} // tex coords 3
#if GL
   VecU4 bone     (                                              ) {return VtxSkinning ? VecU4(_bone) : VecU4(0, 0, 0, 0)                        ;} // bone matrix indexes
#else
   VecU4 bone     (                                              ) {return VtxSkinning ?       _bone  : VecU4(0, 0, 0, 0)                        ;} // bone matrix indexes
#endif
   Vec4  weight   (                                              ) {return _weight                                                               ;} // bone matrix weights
   VecH4 material (                                              ) {return _material                                                             ;} // material    weights
   VecH  material3(                                              ) {return _material.xyz                                                         ;} // material    weights
   Half  size     (                                              ) {return _size                                                                 ;} // point size

   // need to apply gamma correction in the shader because R8G8B8A8_SRGB can't be specified in vertex buffer
#if LINEAR_GAMMA
   VecH4 color     () {return VecH4(SRGBToLinear    (_color.rgb), _color.a);} // sRGB vertex color (precise)
   VecH  color3    () {return       SRGBToLinear    (_color.rgb)           ;} // sRGB vertex color (precise)
   VecH4 colorFast () {return VecH4(SRGBToLinearFast(_color.rgb), _color.a);} // sRGB vertex color (fast)
   VecH  colorFast3() {return       SRGBToLinearFast(_color.rgb)           ;} // sRGB vertex color (fast)
   Half  colorFastA() {return                                     _color.a ;} // sRGB vertex color (fast)
#else
   VecH4 color     () {return _color                                       ;} // sRGB vertex color (precise)
   VecH  color3    () {return _color.rgb                                   ;} // sRGB vertex color (precise)
   VecH4 colorFast () {return _color                                       ;} // sRGB vertex color (fast)
   VecH  colorFast3() {return _color.rgb                                   ;} // sRGB vertex color (fast)
   Half  colorFastA() {return _color.a                                     ;} // sRGB vertex color (fast)
#endif
   VecH4 colorF    () {return _color                                       ;} // linear vertex color
   VecH  colorF3   () {return _color.rgb                                   ;} // linear vertex color

   VecU2 faceID  () {return _face_id;}
   UInt  id      () {return _id;}
   UInt  instance() {return _instance;}
};
/******************************************************************************/
void Draw_VS(VtxInput vtx,
          NOPERSP out Vec4 pixel:POSITION)
{
   pixel=Vec4(vtx.pos2(), Z_BACK, 1); // set Z to be at the end of the viewport, this enables optimizations by processing only foreground pixels (no sky/background)
}
void DrawUV_VS(VtxInput vtx,
   NOPERSP out Vec2 uv   :UV,
   NOPERSP out Vec4 pixel:POSITION)
{
   uv   =vtx.uv();
   pixel=Vec4(vtx.pos2(), Z_BACK, 1); // set Z to be at the end of the viewport, this enables optimizations by processing only foreground pixels (no sky/background)
}
void DrawUVPosXY_VS(VtxInput vtx,
        NOPERSP out Vec2 uv   :UV    ,
        NOPERSP out Vec2 posXY:POS_XY,
        NOPERSP out Vec4 pixel:POSITION)
{
   uv   =vtx.uv();
   posXY=UVToPosXY(uv);
   pixel=Vec4(vtx.pos2(), Z_BACK, 1); // set Z to be at the end of the viewport, this enables optimizations by processing only foreground pixels (no sky/background)
}
void DrawPosXY_VS(VtxInput vtx,
      NOPERSP out Vec2 posXY:POS_XY,
      NOPERSP out Vec4 pixel:POSITION)
{
   posXY=UVToPosXY(vtx.uv());
   pixel=Vec4(vtx.pos2(), Z_BACK, 1); // set Z to be at the end of the viewport, this enables optimizations by processing only foreground pixels (no sky/background)
}
void DrawScreen_VS(VtxInput vtx,
      NOPERSP out Vec4 pixel:POSITION)
{
   pixel=Vec4(vtx.pos2()*Coords.xy+Coords.zw, Z_FRONT, 1);
}
void DrawScreenUV_VS(VtxInput vtx,
      NOPERSP out Vec2 uv   :UV,
      NOPERSP out Vec4 pixel:POSITION)
{
   uv   =vtx.uv();
   pixel=Vec4(vtx.pos2()*Coords.xy+Coords.zw, Z_FRONT, 1);
}
/******************************************************************************/
Half DistPointPlaneRay(VecH2 p,                  VecH2 plane_normal, VecH2 ray) {Half rd=Dot(ray, plane_normal); return rd ? Dot           (p,            plane_normal)/rd : 0;}
Flt  DistPointPlaneRay(Vec2  p,                  Vec2  plane_normal, Vec2  ray) {Flt  rd=Dot(ray, plane_normal); return rd ? Dot           (p,            plane_normal)/rd : 0;}
Half DistPointPlaneRay(VecH  p,                  VecH  plane_normal, VecH  ray) {Half rd=Dot(ray, plane_normal); return rd ? Dot           (p,            plane_normal)/rd : 0;}
Flt  DistPointPlaneRay(Vec   p,                  Vec   plane_normal, Vec   ray) {Flt  rd=Dot(ray, plane_normal); return rd ? Dot           (p,            plane_normal)/rd : 0;}
Half DistPointPlaneRay(VecH2 p, VecH2 plane_pos, VecH2 plane_normal, VecH2 ray) {Half rd=Dot(ray, plane_normal); return rd ? DistPointPlane(p, plane_pos, plane_normal)/rd : 0;}
Flt  DistPointPlaneRay(Vec2  p, Vec2  plane_pos, Vec2  plane_normal, Vec2  ray) {Flt  rd=Dot(ray, plane_normal); return rd ? DistPointPlane(p, plane_pos, plane_normal)/rd : 0;}
Half DistPointPlaneRay(VecH  p, VecH  plane_pos, VecH  plane_normal, VecH  ray) {Half rd=Dot(ray, plane_normal); return rd ? DistPointPlane(p, plane_pos, plane_normal)/rd : 0;}
Flt  DistPointPlaneRay(Vec   p, Vec   plane_pos, Vec   plane_normal, Vec   ray) {Flt  rd=Dot(ray, plane_normal); return rd ? DistPointPlane(p, plane_pos, plane_normal)/rd : 0;}

VecH2 PointOnPlaneRay(VecH2 p,                  VecH2 plane_normal, VecH2 ray) {return p-ray*DistPointPlaneRay(p,            plane_normal, ray);}
Vec2  PointOnPlaneRay(Vec2  p,                  Vec2  plane_normal, Vec2  ray) {return p-ray*DistPointPlaneRay(p,            plane_normal, ray);}
VecH  PointOnPlaneRay(VecH  p,                  VecH  plane_normal, VecH  ray) {return p-ray*DistPointPlaneRay(p,            plane_normal, ray);}
Vec   PointOnPlaneRay(Vec   p,                  Vec   plane_normal, Vec   ray) {return p-ray*DistPointPlaneRay(p,            plane_normal, ray);}
VecH2 PointOnPlaneRay(VecH2 p, VecH2 plane_pos, VecH2 plane_normal, VecH2 ray) {return p-ray*DistPointPlaneRay(p, plane_pos, plane_normal, ray);}
Vec2  PointOnPlaneRay(Vec2  p, Vec2  plane_pos, Vec2  plane_normal, Vec2  ray) {return p-ray*DistPointPlaneRay(p, plane_pos, plane_normal, ray);}
VecH  PointOnPlaneRay(VecH  p, VecH  plane_pos, VecH  plane_normal, VecH  ray) {return p-ray*DistPointPlaneRay(p, plane_pos, plane_normal, ray);}
Vec   PointOnPlaneRay(Vec   p, Vec   plane_pos, Vec   plane_normal, Vec   ray) {return p-ray*DistPointPlaneRay(p, plane_pos, plane_normal, ray);}
/******************************************************************************/
Matrix3 Inverse(Matrix3 m, Bool normalized)
{
   m=transpose(m);
   if(!normalized)
   {
      m[0]/=Length2(m[0]);
      m[1]/=Length2(m[1]);
      m[2]/=Length2(m[2]);
   }
   return m;
}
/******************************************************************************
Flt Lerp4(Flt v0, Flt v1, Flt v2, Flt v3, Flt s)
{
   return s*s*s * ((2-TAN)*(v1-v2) + TAN*(v3-v0)                            )
        + s*s   * ((TAN-3)*(v1   ) - TAN*(v3   ) - (2*TAN-3)*v2 + (2*TAN)*v0)
        + s     * ( TAN   *(v2-v0)                                          )
        + v1;
}
Half Lerp4(Half v0, Half v1, Half v2, Half v3, Half s)
{
   return s*s*s * ((2-TAN)*(v1-v2) + TAN*(v3-v0)                            )
        + s*s   * ((TAN-3)*(v1   ) - TAN*(v3   ) - (2*TAN-3)*v2 + (2*TAN)*v0)
        + s     * ( TAN   *(v2-v0)                                          )
        + v1;
}

Vec Lerp4(Vec v0, Vec v1, Vec v2, Vec v3, Flt s)
{
   Flt s2=s*s,
       s3=s*s*s;

   return v1 * ((2-TAN)*s3     + (  TAN-3)*s2         + 1)
        - v2 * ((2-TAN)*s3     + (2*TAN-3)*s2 - TAN*s    )
        - v0 * (   TAN*(s3+s ) - (2*TAN  )*s2            )
        + v3 * (   TAN*(s3-s2)                           );
}
VecH Lerp4(VecH v0, VecH v1, VecH v2, VecH v3, Half s)
{
   Half s2=s*s,
        s3=s*s*s;

   return v1 * ((2-TAN)*s3     + (  TAN-3)*s2         + 1)
        - v2 * ((2-TAN)*s3     + (2*TAN-3)*s2 - TAN*s    )
        - v0 * (   TAN*(s3+s ) - (2*TAN  )*s2            )
        + v3 * (   TAN*(s3-s2)                           );
}

Vec4 Lerp4(Vec4 v0, Vec4 v1, Vec4 v2, Vec4 v3, Flt s)
{
   Flt s2=s*s,
       s3=s*s*s;

   return v1 * ((2-TAN)*s3     + (  TAN-3)*s2         + 1)
        - v2 * ((2-TAN)*s3     + (2*TAN-3)*s2 - TAN*s    )
        - v0 * (   TAN*(s3+s ) - (2*TAN  )*s2            )
        + v3 * (   TAN*(s3-s2)                           );
}
VecH4 Lerp4(VecH4 v0, VecH4 v1, VecH4 v2, VecH4 v3, Half s)
{
   Half s2=s*s,
        s3=s*s*s;

   return v1 * ((2-TAN)*s3     + (  TAN-3)*s2         + 1)
        - v2 * ((2-TAN)*s3     + (2*TAN-3)*s2 - TAN*s    )
        - v0 * (   TAN*(s3+s ) - (2*TAN  )*s2            )
        + v3 * (   TAN*(s3-s2)                           );
}
/******************************************************************************/
Half SmoothCube(Half s) {return (3-2*s)*s*s;}
Flt  SmoothCube(Flt  s) {return (3-2*s)*s*s;}
VecH SmoothCube(VecH s) {return (3-2*s)*s*s;}

Half LerpSmoothCube(Half from, Half to, Half s) {return Lerp(from, to, SmoothCube(s));}
Flt  LerpSmoothCube(Flt  from, Flt  to, Flt  s) {return Lerp(from, to, SmoothCube(s));}
VecH LerpSmoothCube(VecH from, VecH to, Half s) {return Lerp(from, to, SmoothCube(s));}

Half BlendSqr(Half x) {return Sat(1-x*x);}
Flt  BlendSqr(Flt  x) {return Sat(1-x*x);}

Half BlendSmoothCube(Half x) {x=Sat(Abs(x)); return 1-SmoothCube(x);}
Flt  BlendSmoothCube(Flt  x) {x=Sat(Abs(x)); return 1-SmoothCube(x);}

Half BlendSmoothSin(Half x) {x=Sat(Abs(x)); return Cos(x*PI)*0.5+0.5;}
Flt  BlendSmoothSin(Flt  x) {x=Sat(Abs(x)); return Cos(x*PI)*0.5+0.5;}

Half Gaussian(Half x) {return Exp(-x*x);}
Flt  Gaussian(Flt  x) {return Exp(-x*x);}

Half ScaleFactor (Half x) {return (x>=0) ? (1+x) : (1/(1-x));}
Flt  ScaleFactor (Flt  x) {return (x>=0) ? (1+x) : (1/(1-x));}
Half ScaleFactorR(Half s) {return (s>=1) ? (s-1) : (1-(1/s));}
Flt  ScaleFactorR(Flt  s) {return (s>=1) ? (s-1) : (1-(1/s));}

Half Pinch      (Half x, Half pinch) {return x*pinch/(1+x*(pinch-1));}
Flt  Pinch      (Flt  x, Flt  pinch) {return x*pinch/(1+x*(pinch-1));}
Half PinchFactor(Half x, Half pinch) {return Pinch(x, ScaleFactor(pinch));}
Flt  PinchFactor(Flt  x, Flt  pinch) {return Pinch(x, ScaleFactor(pinch));}

Flt   SigmoidSqr(Flt   x) {return x/Sqrt(1+x*x);}
Half  SigmoidSqr(Half  x) {return x/Sqrt(1+x*x);}
Vec   SigmoidSqr(Vec   x) {return x/Sqrt(1+x*x);}
VecH  SigmoidSqr(VecH  x) {return x/Sqrt(1+x*x);}
VecH4 SigmoidSqr(VecH4 x) {return x/Sqrt(1+x*x);}

Flt   SigmoidSqrInv(Flt   y) {return y/Sqrt(1-y*y);}
Half  SigmoidSqrInv(Half  y) {return y/Sqrt(1-y*y);}
Vec   SigmoidSqrInv(Vec   y) {return y/Sqrt(1-y*y);}
VecH  SigmoidSqrInv(VecH  y) {return y/Sqrt(1-y*y);}
VecH4 SigmoidSqrInv(VecH4 y) {return y/Sqrt(1-y*y);}

Flt   SigmoidExp(Flt   x) {return 2/(1+Exp(x*-2))-1;}
Half  SigmoidExp(Half  x) {return 2/(1+Exp(x*-2))-1;}
Vec   SigmoidExp(Vec   x) {return 2/(1+Exp(x*-2))-1;}
VecH  SigmoidExp(VecH  x) {return 2/(1+Exp(x*-2))-1;}
VecH4 SigmoidExp(VecH4 x) {return 2/(1+Exp(x*-2))-1;}

Flt   SigmoidAtan(Flt   x) {return Atan(x*PI_2)*(2/PI);}
Half  SigmoidAtan(Half  x) {return Atan(x*PI_2)*(2/PI);}
Vec   SigmoidAtan(Vec   x) {return Atan(x*PI_2)*(2/PI);}
VecH  SigmoidAtan(VecH  x) {return Atan(x*PI_2)*(2/PI);}
VecH4 SigmoidAtan(VecH4 x) {return Atan(x*PI_2)*(2/PI);}
/******************************************************************************/
Half     VisibleOpacity(Flt density, Flt range) {return   Pow(1-density, range);} // calculate visible     opacity (0..1) having 'density' environment density (0..1), and 'range' (0..Inf)
Half AccumulatedDensity(Flt density, Flt range) {return 1-Pow(1-density, range);} // calculate accumulated density (0..1) having 'density' environment density (0..1), and 'range' (0..Inf)
/******************************************************************************/
// RGB <-> HSB
/******************************************************************************/
Vec RgbToHsb(Vec rgb)
{
   Flt max=Max(rgb),
       min=Min(rgb),
       d  =max-min;
   Vec hsb;

   if(d    <=  0)hsb.x=0;else
   if(rgb.x>=max)
   {
      if(rgb.y>=rgb.z)hsb.x=(rgb.y-rgb.z)/d;
      else            hsb.x=(rgb.y-rgb.z)/d+6;
   }else
   if(rgb.y>=max)hsb.x=(rgb.z-rgb.x)/d+2;
   else          hsb.x=(rgb.x-rgb.y)/d+4;

   hsb.x/=6;
   hsb.y =(max ? 1-min/max : 1);
   hsb.z = max;

   return hsb;
}
Vec HsbToRgb(Vec hsb)
{
   Flt h=Frac(hsb.x)*6,
       s=Sat (hsb.y),
       f=Frac(h),
       v=hsb.z,
       p=hsb.z*(1-s      ),
       q=hsb.z*(1-s*(  f)),
       t=hsb.z*(1-s*(1-f));
   if(h<1)return Vec(v, t, p);
   if(h<2)return Vec(q, v, p);
   if(h<3)return Vec(p, v, t);
   if(h<4)return Vec(p, q, v);
   if(h<5)return Vec(t, p, v);
          return Vec(v, p, q);
}
/******************************************************************************/
// BLEND
/******************************************************************************/
Flt     BlendAlpha(          Flt base_a,            Flt color_a) {return base_a + color_a*(1-base_a);}
Vec     BlendAlpha(          Vec base_a,            Vec color_a) {return base_a + color_a*(1-base_a);}
Vec FastBlendColor(Vec base, Vec base_a, Vec color, Vec color_a) {return base*(1-color_a) + color*color_a;}
Vec     BlendColor(Vec base, Vec base_a, Vec color, Vec color_a)
{
   base_a*=1-color_a;
   Vec ret, sum=base_a+color_a;
   if(sum.r){base_a.r/=sum.r; color_a.r=1-base_a.r; ret.r=base.r*base_a.r + color.r*color_a.r;}else ret.r=0;
   if(sum.g){base_a.g/=sum.g; color_a.g=1-base_a.g; ret.g=base.g*base_a.g + color.g*color_a.g;}else ret.g=0;
   if(sum.b){base_a.b/=sum.b; color_a.b=1-base_a.b; ret.b=base.b*base_a.b + color.b*color_a.b;}else ret.b=0;
   return ret;
}
Vec MergeBlendColor(Vec base, Vec base_a, Vec color, Vec color_a)
{
   base_a*=1-color_a;
   Vec sum=base_a+color_a;
#if 1
   return sum ? (base*base_a+color)/sum : 0;
#else
   Vec ret;
   if(sum.r)ret.r=(base.r*base_a.r + color.r)/sum.r;else ret.r=0;
   if(sum.g)ret.g=(base.g*base_a.g + color.g)/sum.g;else ret.g=0;
   if(sum.b)ret.b=(base.b*base_a.b + color.b)/sum.b;else ret.b=0;
   return ret;
#endif
}
/******************************************************************************/
// ALPHA TEST
/******************************************************************************/
void MaterialAlphaTest(Half alpha)
{
   clip(alpha+Material.color.a-1);
}
void MaterialAlphaTestDither(Half alpha, VecI2 pixel, VecU2 face, bool noise_offset=true)
{
   pixel=pixel+(noise_offset ? NoiseOffset : 0)+face; // can't use xor because it would destroy noise image continuity, adjust by face to make sure that multiple faces on top of each other would use different weights (example #0 face with alpha=0.5 and then #1 face with alpha=0.5 drawn on top of #0 would use the same pixels, but with face index variation they will use different)
   Half scale=1-1.0/1024; // this is needed to preserve fully opaque =1 alphas 
#if 0 // 64-step cbuffer
   alpha=alpha+(Noise1D_64  (pixel)*     scale -0.5);
#else // blue noise image
   alpha=alpha+(Noise1D_Blue(pixel)*(0.5*scale)-0.5);
#endif
   if(alpha<=0)discard;
}
/******************************************************************************/
// NORMAL + EXT
/******************************************************************************/
Vec4 UnpackNormal(VecH4 nrm)
{
#if !SIGNED_NRM_RT
   nrm.xyz=nrm.xyz*2-1;
#endif
   Vec4 nrm_hp=nrm; // convert to HP before normalization
   nrm_hp.xyz=Normalize(nrm_hp.xyz); // normalize needed even if source was F16 format because it improves quality for specular
   return nrm_hp;
}
Vec4  GetNormal  (VecI2 pixel             ) {return UnpackNormal(          Img     [pixel]);}
Vec4  GetNormalMS(VecI2 pixel, UInt sample) {return UnpackNormal(TexSample(ImgMS  , pixel, sample));}
VecH2 GetExt     (VecI2 pixel             ) {return                        ImgXY   [pixel];}
VecH2 GetExtMS   (VecI2 pixel, UInt sample) {return              TexSample(ImgXYMS, pixel, sample) ;}
/******************************************************************************/
// LOD INDEX
/******************************************************************************/
// TODO: 'CalculateLevelOfDetail' could be used however it's only DX 10.1 SM_4_1 / GL 4.0+ / GL ES ?
Flt GetLod(Vec2 uv, Flt  tex_size) {Vec2 pix=uv*tex_size; return 0.5*Log2(Max(Length2(ddx(pix)), Length2(ddy(pix))));}
Flt GetLod(Vec2 uv, Vec2 tex_size) {Vec2 pix=uv*tex_size; return 0.5*Log2(Max(Length2(ddx(pix)), Length2(ddy(pix))));}
/******************************************************************************/
// GRASS AND LEAF
/******************************************************************************/
#define GrassBendFreq  1.0
#define GrassBendScale 0.033

#define LeafBendFreq   2.0
#define LeafBendScale  0.033
#define LeafsBendScale (LeafBendScale/2)
/******************************************************************************/
Vec2 GetGrassBend(Vec world_pos, Bool prev=false)
{
   Vec4 bend_factor=(prev ? BendFactorPrev : BendFactor);
   Flt  offset=Dot(world_pos.xz, Vec2(0.7, 0.9)*GrassBendFreq);
   return Vec2((1.0*GrassBendScale)*Sin(offset+bend_factor.x) + (1.0*GrassBendScale)*Sin(offset+bend_factor.y),
               (1.0*GrassBendScale)*Sin(offset+bend_factor.z) + (1.0*GrassBendScale)*Sin(offset+bend_factor.w));
}
Vec2 GetLeafBend(Vec center, Bool prev=false)
{
   Vec4 bend_factor=(prev ? BendFactorPrev : BendFactor);
   Flt  offset=Dot(center.xy, Vec2(0.7, 0.8)*LeafBendFreq);
   return Vec2((1.0*LeafBendScale)*Sin(offset+bend_factor.x) + (1.0*LeafBendScale)*Sin(offset+bend_factor.y),
               (1.0*LeafBendScale)*Sin(offset+bend_factor.z) + (1.0*LeafBendScale)*Sin(offset+bend_factor.w));
}
Vec2 GetLeafsBend(Vec center, Flt offset, Bool prev=false)
{
   Vec4 bend_factor=(prev ? BendFactorPrev : BendFactor);
   offset+=Dot(center.xy, Vec2(0.7, 0.8)*LeafBendFreq);
   return Vec2((1.0*LeafsBendScale)*Sin(offset+bend_factor.x) + (1.0*LeafsBendScale)*Sin(offset+bend_factor.y),
               (1.0*LeafsBendScale)*Sin(offset+bend_factor.z) + (1.0*LeafsBendScale)*Sin(offset+bend_factor.w));
}
/******************************************************************************/
Half GrassFadeOut(UInt mtrx=0)
{
   return Sat(Length2(ViewMatrixPos(mtrx))*GrassRangeMulAdd.x+GrassRangeMulAdd.y);
}
void BendGrass(Vec local_pos, inout Vec view_pos, UInt mtrx=0, Bool prev=false)
{
#if 1 // slower but higher quality
   if(local_pos.y>0)
   {
      Vec center=(prev ? ViewMatrixPrevPos(mtrx) : ViewMatrixPos(mtrx)),
          delta =view_pos-center;

      Flt len2 =Length2(delta),
          blend=Min(len2, Sqr(local_pos.y*2)); // minimize with vertical position
        //blend=Min(Sqrt(len2), local_pos.y*2); alternative formula: looks more stiff at the bottom, but tops don't bend so much, maybe could use a mix of both formulas, but that would be slow
      Vec2 bend=GetGrassBend(prev ? ObjWorldPosPrev(mtrx) : ObjWorldPos(mtrx), prev)*blend;

      if(!prev)
      {
         delta+=Vec(CamMatrix[0].x, CamMatrix[1].x, CamMatrix[2].x)*bend.x; // world X right
         delta+=Vec(CamMatrix[0].z, CamMatrix[1].z, CamMatrix[2].z)*bend.y; // world Z forward
      }else
      {
         delta+=Vec(CamMatrixPrev[0].x, CamMatrixPrev[1].x, CamMatrixPrev[2].x)*bend.x; // world X right
         delta+=Vec(CamMatrixPrev[0].z, CamMatrixPrev[1].z, CamMatrixPrev[2].z)*bend.y; // world Z forward
      }

      Flt l2=Length2(delta); delta*=Sqrt(len2/l2); // delta.setLength2(len2);
      view_pos=center+delta;
   }
#else // fast & simple
   if(local_pos.y>0)
   {
      Flt blend=Sqr(local_pos.y);
      if(!prev)
      {
         Vec2 bend=GetGrassBend(ObjWorldPos(mtrx), prev)*(blend*Length2(ViewMatrixY(mtrx)));
         view_pos+=Vec(CamMatrix[0].x, CamMatrix[1].x, CamMatrix[2].x)*bend.x; // world X right
         view_pos+=Vec(CamMatrix[0].z, CamMatrix[1].z, CamMatrix[2].z)*bend.y; // world Z forward
      }else
      {
         Vec2 bend=GetGrassBend(ObjWorldPosPrev(mtrx), prev)*(blend*Length2(ViewMatrixPrevY(mtrx)));
         view_pos+=Vec(CamMatrixPrev[0].x, CamMatrixPrev[1].x, CamMatrixPrev[2].x)*bend.x; // world X right
         view_pos+=Vec(CamMatrixPrev[0].z, CamMatrixPrev[1].z, CamMatrixPrev[2].z)*bend.y; // world Z forward
      }
   }
#endif
}
/******************************************************************************/
// Here can't use Half's because precision was not good enough
void BendLeaf(Vec center, inout Vec pos, Bool prev=false)
{
   VecH   delta=pos-center;
   VecH2  cos_sin, bend=GetLeafBend(center, prev);
   CosSin(cos_sin.x, cos_sin.y, bend.x); delta.xy=Rotate(delta.xy, cos_sin);
   CosSin(cos_sin.x, cos_sin.y, bend.y); delta.zy=Rotate(delta.zy, cos_sin);
   pos=center+delta;
}
void BendLeaf(Vec center, inout Vec pos, inout VecH nrm, Bool prev=false)
{
   VecH   delta=pos-center;
   VecH2  cos_sin, bend=GetLeafBend(center, prev);
   CosSin(cos_sin.x, cos_sin.y, bend.x); delta.xy=Rotate(delta.xy, cos_sin); nrm.xy=Rotate(nrm.xy, cos_sin);
   CosSin(cos_sin.x, cos_sin.y, bend.y); delta.zy=Rotate(delta.zy, cos_sin); nrm.zy=Rotate(nrm.zy, cos_sin);
   pos=center+delta;
}
void BendLeaf(Vec center, inout Vec pos, inout VecH nrm, inout VecH tan, Bool prev=false)
{
   VecH   delta=pos-center;
   VecH2  cos_sin, bend=GetLeafBend(center, prev);
   CosSin(cos_sin.x, cos_sin.y, bend.x); delta.xy=Rotate(delta.xy, cos_sin); nrm.xy=Rotate(nrm.xy, cos_sin); tan.xy=Rotate(tan.xy, cos_sin);
   CosSin(cos_sin.x, cos_sin.y, bend.y); delta.zy=Rotate(delta.zy, cos_sin); nrm.zy=Rotate(nrm.zy, cos_sin); tan.zy=Rotate(tan.zy, cos_sin);
   pos=center+delta;
}
/******************************************************************************/
// Here can't use Half's because precision was not good enough
void BendLeafs(Vec center, Flt offset, inout Vec pos, Bool prev=false)
{
   VecH   delta=pos-center;
   VecH2  cos_sin, bend=GetLeafsBend(center, offset, prev);
   CosSin(cos_sin.x, cos_sin.y, bend.x); delta.xy=Rotate(delta.xy, cos_sin);
   CosSin(cos_sin.x, cos_sin.y, bend.y); delta.zy=Rotate(delta.zy, cos_sin);
   pos=center+delta;
}
void BendLeafs(Vec center, Flt offset, inout Vec pos, inout VecH nrm, Bool prev=false)
{
   VecH   delta=pos-center;
   VecH2  cos_sin, bend=GetLeafsBend(center, offset, prev);
   CosSin(cos_sin.x, cos_sin.y, bend.x); delta.xy=Rotate(delta.xy, cos_sin); nrm.xy=Rotate(nrm.xy, cos_sin);
   CosSin(cos_sin.x, cos_sin.y, bend.y); delta.zy=Rotate(delta.zy, cos_sin); nrm.zy=Rotate(nrm.zy, cos_sin);
   pos=center+delta;
}
void BendLeafs(Vec center, Flt offset, inout Vec pos, inout VecH nrm, inout VecH tan, Bool prev=false)
{
   VecH   delta=pos-center;
   VecH2  cos_sin, bend=GetLeafsBend(center, offset, prev);
   CosSin(cos_sin.x, cos_sin.y, bend.x); delta.xy=Rotate(delta.xy, cos_sin); nrm.xy=Rotate(nrm.xy, cos_sin); tan.xy=Rotate(tan.xy, cos_sin);
   CosSin(cos_sin.x, cos_sin.y, bend.y); delta.zy=Rotate(delta.zy, cos_sin); nrm.zy=Rotate(nrm.zy, cos_sin); tan.zy=Rotate(tan.zy, cos_sin);
   pos=center+delta;
}
/******************************************************************************/
// DEPTH WEIGHT
/******************************************************************************/
#include "!Set Prec Struct.h"
BUFFER(DepthWeight)
   Vec2 DepthWeightScale; // X=for linear filtering, Y=for point filtering
BUFFER_END
#include "!Set Prec Default.h"

#if 0
Flt P1=0.004, P2=2;
Vec2 DepthWeightMAD      (Flt depth) {return Vec2(-1.0/(depth*DepthWeightScale+P1), P2);}
#else
Vec2 DepthWeightMADLinear(Flt depth) {return Vec2(-1.0/(depth*DepthWeightScale.x+0.004), 2);}
Vec2 DepthWeightMADPoint (Flt depth) {return Vec2(-1.0/(depth*DepthWeightScale.y+0.008), 2);} // use 2x bigger tolerance because point filtering doesn't smoothen depth values
#endif
Half DepthWeight(Flt delta, Vec2 dw_mad)
{
   return Sat(Abs(delta)*dw_mad.x + dw_mad.y);
}
/******************************************************************************/
// DETAIL
/******************************************************************************/
VecH4 GetDetail(Vec2 tex) // XY=nrm.xy -1..1 delta, Z=rough -1..1 delta, W=color 0..2 scale
{
   VecH4 det=RTex(Det, tex*Material.det_uv_scale); // XY=nrm.xy 0..1, Z=rough 0..1, W=color 0..1 #MaterialTextureLayoutDetail

   /* unoptimized
   det.xyz                 =(det.xyz            -0.5)*2*Material.det_power;
   det.DETAIL_CHANNEL_COLOR= det.DETAIL_CHANNEL_COLOR*2*Material.det_power+(1-Material.det_power); // Lerp(1, det*2, Material.det_power) = 1*(1-Material.det_power) + det*2*Material.det_power */

   // optimized TODO: these constants could be precalculated as det_power2=det_power*2; det_power_neg=-det_power; det_power_inv=1-det_power; however that would increase the Material buffer size
   det.xyz                 =det.xyz                 *(Material.det_power*2)+( -Material.det_power);
   det.DETAIL_CHANNEL_COLOR=det.DETAIL_CHANNEL_COLOR*(Material.det_power*2)+(1-Material.det_power);

   return det;
}
VecH4 GetDetail0(Vec2 tex) {VecH4 det=RTex(Det , tex*MultiMaterial0.det_uv_scale); det.xyz=det.xyz*MultiMaterial0.det_mul+MultiMaterial0.det_add; det.DETAIL_CHANNEL_COLOR=det.DETAIL_CHANNEL_COLOR*MultiMaterial0.det_mul+MultiMaterial0.det_inv; return det;}
VecH4 GetDetail1(Vec2 tex) {VecH4 det=RTex(Det1, tex*MultiMaterial1.det_uv_scale); det.xyz=det.xyz*MultiMaterial1.det_mul+MultiMaterial1.det_add; det.DETAIL_CHANNEL_COLOR=det.DETAIL_CHANNEL_COLOR*MultiMaterial1.det_mul+MultiMaterial1.det_inv; return det;}
VecH4 GetDetail2(Vec2 tex) {VecH4 det=RTex(Det2, tex*MultiMaterial2.det_uv_scale); det.xyz=det.xyz*MultiMaterial2.det_mul+MultiMaterial2.det_add; det.DETAIL_CHANNEL_COLOR=det.DETAIL_CHANNEL_COLOR*MultiMaterial2.det_mul+MultiMaterial2.det_inv; return det;}
VecH4 GetDetail3(Vec2 tex) {VecH4 det=RTex(Det3, tex*MultiMaterial3.det_uv_scale); det.xyz=det.xyz*MultiMaterial3.det_mul+MultiMaterial3.det_add; det.DETAIL_CHANNEL_COLOR=det.DETAIL_CHANNEL_COLOR*MultiMaterial3.det_mul+MultiMaterial3.det_inv; return det;}
/******************************************************************************/
// FACE NORMAL HANDLING
/******************************************************************************/
void BackFlip(inout VecH dir, Bool front) {if(!front)dir=-dir;}
/******************************************************************************/
Half MultiMaterialWeight(Half weight, Half alpha) // 'weight'=weight of this material, 'alpha'=color texture alpha (opacity or bump)
{
#if 0 // linear
   return weight;
#elif 0 // Pinch (not sharp enough)
   const Half sharpen=32;
   return Pinch(weight, ScaleFactor(alpha*sharpen - 0.5*sharpen));
#elif 0 // PinchFactor (too sharp transitions)
   const Half sharpen=32;
   return PinchFactor(weight, alpha*sharpen - 0.5*sharpen);
#elif 0 // Sigmoid (not sharp enough, but when increasing sharpen then too bright on low weights, causing visible jumps due to material reduction for low weights
   const Half sharpen=21;
   alpha=alpha*sharpen - 0.5*sharpen; // (alpha-0.5)*sharpen
   if(alpha<-HALF_MIN)weight=SigmoidSqrInv(weight*SigmoidSqr(alpha))/           alpha ;else
   if(alpha> HALF_MIN)weight=SigmoidSqr   (weight*           alpha )/SigmoidSqr(alpha);
   return weight;
#elif 0 // Pow (too bright on low weights, causing visible jumps due to material reduction for low weights, also blurry (small contrast) compared to Pinch
   const Half sharpen=-5;
   return Pow(weight, ScaleFactor(alpha*sharpen - 0.5*sharpen));
#elif 0 // Flippable Pow approximation, however causes artifacts
   const Half sharpen=8;
   return Max(0, weight // base
                +weight*(1-weight)*(alpha*sharpen - 0.5*sharpen)); // "weight"=ignore alpha at start "1-weight" ignore alpha at end, "(alpha-0.5)*sharpen" alpha
#else // Flippable Pow - best
   const Half sharpen=12;
   #if 0
      return (alpha<=0.5) ?   Pow(  weight, 1+(0.5-alpha)*sharpen)
                          : 1-Pow(1-weight, 1+(alpha-0.5)*sharpen);
   #else // optimized using only 1 Pow
      VecH2 mad;
      if(alpha<=0.5)
      {
         mad=VecH2(1, 0);
      }else
      {
         mad=VecH2(-1, 1);
       //alpha =1-alpha ; // or alternatively do alpha  MAD below
       //weight=1-weight; // or alternatively do weight MAD below
      }
      alpha =alpha *mad.x+mad.y; // MAD generates smaller SPIR-V code
      weight=weight*mad.x+mad.y; // MAD generates smaller SPIR-V code
      return Pow(weight, 1+0.5*sharpen-alpha*sharpen)*mad.x+mad.y;
   #endif
#endif
}
/******************************************************************************/
// LIGHTS
/******************************************************************************/
#include "!Set Prec Struct.h"
struct GpuLightDir
{
   Vec  dir; // high precision needed for HQ specular
   VecH color;
   VecH vol_exponent_steam;
   Half radius_frac; // 0..1
};
struct GpuLightPoint
{
   Flt  power , // 0..Inf
        radius; // 0..Inf
   Vec  pos;
   Half lum_max,
        vol, vol_max;
   VecH color;
};
struct GpuLightLinear
{
   Flt  neg_inv_range, // -1/range
        radius       ; // 0..Inf
   Vec  pos;
   VecH color;
   Half vol, vol_max;
};
struct GpuLightCone
{
   Flt      neg_inv_range, // -1/range
            radius       ; // 0..Inf
   Vec2     falloff;
   Vec      pos;
   VecH     color;
   MatrixH3 mtrx;
   Half     scale,
            vol, vol_max;
};

BUFFER(LightDir   ) GpuLightDir    LightDir   ; BUFFER_END
BUFFER(LightPoint ) GpuLightPoint  LightPoint ; BUFFER_END
BUFFER(LightLinear) GpuLightLinear LightLinear; BUFFER_END
BUFFER(LightCone  ) GpuLightCone   LightCone  ; BUFFER_END
#include "!Set Prec Default.h"
/******************************************************************************/
Half LightPointDist (Flt  inv_dist2) {return Min(Half(inv_dist2*LightPoint .power    ), LightPoint.lum_max);} // LightPoint.power/Length2(pos), NaN
Half LightLinearDist(Flt  dist     ) {return Sat(         dist *LightLinear.neg_inv_range + 1             );} // 1-Length(pos)/LightLinear.range
Half LightConeDist  (Flt  dist     ) {return Sat(         dist *LightCone  .neg_inv_range + 1             );} // 1-Length(pos)/LightCone  .range
Half LightConeAngle (Vec2 pos      ) {Half v=Sat(  Length(pos) *LightCone  .falloff.x+LightCone.falloff.y ); return v;} // alternative is Sqr(v)
/******************************************************************************/
Half F_Schlick(Half f0, Half f90, Half cos) // High Precision not needed
{
   Half q=Quint(1-cos); // Quint(1-x) = ~Exp2(-9.28*x)
   return (f90-f0)*q + f0;
}
VecH F_Schlick(VecH f0, Half f90, Half cos) // High Precision not needed
{
   Half q=Quint(1-cos); // Quint(1-x) = ~Exp2(-9.28*x)
   return f90*q + f0*(1-q); // re-ordered because of Vec
}
Half Vis_SmithR2Inv(Half rough2, Half NdotL, Half NdotV) // High Precision not needed, "rough2=Sqr(rough)", result is inversed 1/x
{
#if 1
   Half view =NdotV+Sqrt((-NdotV*rough2+NdotV)*NdotV+rough2);
	Half light=NdotL+Sqrt((-NdotL*rough2+NdotL)*NdotL+rough2);
	return view*light;
#else // gives same results but has 2 MUL and 1 ADD, instead of 2 ADD 1 MUL, don't use in case MUL is slower than ADD
   // Warning: "NdotL*" and "NdotV*" are exchanged on purpose
   Half view =NdotL*Sqrt((-NdotV*rough2+NdotV)*NdotV+rough2);
   Half light=NdotV*Sqrt((-NdotL*rough2+NdotL)*NdotL+rough2);
   return (view+light)*2;
#endif
}
Half Vis_SmithFastInv(Half rough, Half NdotL, Half NdotV) // fast approximation of 'Vis_Smith', High Precision not needed, result is inversed 1/x
{
	Half view =NdotL*(NdotV*(1-rough)+rough);
	Half light=NdotV*(NdotL*(1-rough)+rough);
	return (view+light)*2;
}
/*Half Vis_Kelemen(Half VdotH)
{
   return (0.25/PI)/Sqr(VdotH);
}
Half D_GGX(Half rough, Flt NdotH) // Trowbridge-Reitz, High Precision required
{
#if 1
   Flt rough2=Sqr(rough);
   Flt f=(NdotH*rough2-NdotH)*NdotH+1;
   return rough2/(f*f); // NaN
#else // alternative formula
   Flt a=NdotH*rough;
   Flt k=rough/(1 - NdotH*NdotH + a*a); // NaN
   return k*k;
#endif
}*/
Half D_GGX_Vis_Smith(Half rough, Flt NdotH, Half NdotL, Half NdotV, Bool quality) // 'D_GGX' and 'Vis_Smith' combined together, High Precision required
{
   Half rough2=Sqr(rough);
   Flt  f=(NdotH*rough2-NdotH)*NdotH+1;
   Flt  div=f*f*(quality ? Vis_SmithR2Inv(rough2, NdotL, NdotV) : Vis_SmithFastInv(rough, NdotL, NdotV));
   return div ? rough2/div : HALF_MAX;
}
// !! HERE USE 'Flt' FOR 'rough' AND 'rough2' TO WORKAROUND https://github.com/Esenthel/EsenthelEngine/issues/21 !!
Half D_GGX_Vis_Kelemen(Flt rough, Flt NdotH, Half VdotH) // 'D_GGX' and 'Vis_Kelemen' combined together, but without (0.25/PI), High Precision required
{
   Flt rough2=Sqr(rough);
   Flt f=(NdotH*rough2-NdotH)*NdotH+1;
   Flt div=f*f*Sqr(VdotH);
   return div ? Min(rough2/div, HALF_MAX) : HALF_MAX; // this version can get very big, so have to clamp it to avoid black pixels, probably due to infinity
}
/******************************************************************************
Popular game engines use only metalness texture, and calculate "Diffuse{return (1-metal)*base_color}" and "ReflectCol{return Lerp(0.04, base_color, metal)}"
Unreal:
   Unreal Specular map (reflectivity for dielectrics) in Unreal is mostly unused, and the recommendation is to use default Specular=0.5
   float DielectricSpecularToF0(float Specular) {return 0.08f * Specular;} with default Specular=0.5 this returns 0.04
   float3 ComputeF0(float Specular, float3 BaseColor, float Metallic) {return lerp(DielectricSpecularToF0(Specular).xxx, BaseColor, Metallic.xxx);} which is "lerp(0.04, BaseColor, Metallic)"
Unity:
   half OneMinusReflectivityMetallic(half metallic) {lerp(dielectricSpec, 1, metallic);} with 'dielectricSpec' defined as 0.04
To achieve compatibility with a lot of assets for those engines, similar formula is used, however tweaked to preserve original diffuse color and minimize reflectivity at low reflectivity values:
   for reflectivities<=0.04 to make 'Diffuse' return 1 and 'ReflectCol' return 'reflectivity'
   for reflectivities> 0.04 there's a minor difference due to the fact that 'ReflectToInvMetal' is slightly modified however the difference is negligible (full compatibility would require a secondary 'Lerp')
*/
Half ReflectToInvMetal(Half reflectivity) // this returns "1-metal" to make 'Diffuse' calculation faster, returned range is 0..1+ (slightly bigger than 1 for reflectivities <0.04) so result might need to be "Min(1, )"
{
   return LerpR(1.00, 0.04, reflectivity); // treat 0 .. 0.04 reflectivity as dielectrics (metal=0), after that go linearly to metal=1, because for dielectrics we want to preserve original texture fully (make 'Diffuse' return 1), and then go to 1.0 so we can get smooth transition to metal and slowly decrease 'Diffuse' and affect 'ReflectCol'
}
Half Diffuse(Half inv_metal) {return Min(1, inv_metal);}
VecH ReflectCol(Half reflectivity, VecH base_col, Half inv_metal) // non-metals (with low reflectivity) have white reflection and metals (with high reflectivity) have colored reflection
{
   return (reflectivity<=0.04) ? reflectivity : Lerp(base_col, 0.04, inv_metal);
}
VecH ReflectCol(Half reflectivity, VecH base_col)
{
   return ReflectCol(reflectivity, base_col, ReflectToInvMetal(reflectivity));
}
/******************************************************************************/
struct LightParams
{
   Flt                                       NdotH_HP, // High Precision needed for 'D_GGX'
        NdotL_HP, NdotV_HP, VdotL_HP                 ; // High Precision needed for 'NdotH, VdotH' calculation
   Half NdotL   , NdotV   , VdotL   , VdotH          ; // VdotH=LdotH, because H is in the middle between L and V

   void set(Vec N, Vec L) // operate on High Precision since it's needed below anyway
   {
      NdotL=NdotL_HP=Dot(N, L); // 'Sat' not needed !! instead do "NdotL>0" checks !!
   }
   void set(Vec N, Vec L, Vec nV) // nV=-V, High Precision required for all 3 vectors (this was tested)
   {
    /*if(Q)N=AsHalf(N);
      if(W)L=AsHalf(L);
      if(E)nV=AsHalf(nV);*/
	   NdotV=NdotV_HP=-Dot(N, nV);
	   VdotL=VdotL_HP=-Dot(nV, L);
   #if 0
      Vec H=Normalize(L-nV); // L+V
      NdotH_HP=Dot(N, H);
      VdotH   =Dot(L, H);
   #else // faster
    //VdotL=2*VdotH*VdotH-1
	   Flt VL=Rsqrt(VdotL_HP*2+2);
	   NdotH_HP=(NdotL_HP+NdotV_HP)*VL;
	   VdotH   =       VL*VdotL_HP +VL;
   #endif
    /*if(Q)NdotV=AsHalf(NdotV);
      if(W)VdotL=AsHalf(VdotL);
      if(E)NdotV_HP=AsHalf(NdotV_HP);
      if(R)NdotH_HP=AsHalf(NdotH_HP);*/
   }

   // High Precision not needed for Diffuse
   Half diffuseOrenNayar(Half rough) // highlights edges starting from smooth = 1 -> 0
   {
	   Half a=Sqr(rough), a2=Sqr(a); // it's better to square roughness for 'a' too, because it provides smoother transitions between 0..1
	   Half s=VdotL - NdotV*NdotL;
	   Half A=1-0.5*a2/(a2+0.33);
	   Half B= 0.45*a2/(a2+0.09)*s; if(s>=0)B/=Max(NdotL, NdotV);
	   return (A+B)*(a*0.5+1);
   }
   Half diffuseBurley(Half rough) // aka Disney, highlights edges starting from smooth = 0.5 -> 0 and darkens starting from smooth = 0.5 -> 1.0
   {
    //Half f90=0.5+(2*VdotH*VdotH)*rough; 2*VdotH*VdotH=1+VdotL;
      Half f90=0.5+rough+rough*VdotL;
      Half light_scatter=F_Schlick(1, f90,     NdotL );
      Half  view_scatter=F_Schlick(1, f90, Abs(NdotV));
      return 0.965521237*light_scatter*view_scatter;
   }
   Half diffuseWater() // simulate light intensity based on angle between view and water surface normal as described in "Water.cpp"
   {
      const Half min=0.25;
   #if 0 // original
      Half d=Sat(1-NdotV);
      d=1-Sqr(1-d);
      return Lerp(min, 1.0, d);
   #else // optimized
      return Lerp(1.0, min, Sqr(NdotV));
   #endif
   }
   Half diffuse(Half rough)
   {
   #if WATER
      return diffuseWater();
   #elif DIFFUSE_MODE==SDIFFUSE_OREN_NAYAR
      return diffuseOrenNayar(rough);
   #elif DIFFUSE_MODE==SDIFFUSE_BURLEY
      return diffuseBurley(rough);
   #else
      return 1;
   #endif
   }

   #define DEFAULT_LIGHT_RADIUS_FRAC 0.0036
   VecH specular(Half rough, Half reflectivity, VecH reflect_col, Bool quality, Half light_radius_frac=DEFAULT_LIGHT_RADIUS_FRAC)
   { // currently specular can be generated even for smooth=0 and reflectivity=0 #SpecularReflectionFromZeroSmoothReflectivity
   #if 0
      if( Q)rough=Lerp(Pow(light_radius_frac, E ? 1.0/4 : 1.0/2), Pow(1, E ? 1.0/4 : 1.0/2), rough);
      rough=(E ? Quart(rough) : Sqr(rough));
      if(!Q)rough=Lerp(light_radius_frac, 1, rough);
   #else
      rough =Sqr(rough);
      rough+=light_radius_frac*(1-rough); // rough=Lerp(light_radius_frac, 1, rough);
   #endif

      VecH F=F_Schlick(reflect_col, REFLECT_OCCL ? Sat(reflectivity*50) : 1, VdotH);
   #if 0
      Half D=D_GGX(rough, NdotH_HP);
      Half Vis=(quality ? Vis_Smith(rough, NdotL, Abs(NdotV)) : Vis_SmithFast(rough, NdotL, Abs(NdotV))); // use "Abs(NdotV)" as it helps greatly with faces away from the camera
      return F*(D*Vis/PI);
   #else
      Half D_Vis=D_GGX_Vis_Smith(rough, NdotH_HP, NdotL, Abs(NdotV), quality); // use "Abs(NdotV)" as it helps greatly with faces away from the camera
      return F*(D_Vis/PI);
   #endif
   }
   VecH specularEx(Half rough, Half reflectivity, VecH reflect_col, Bool clear_coat, Half light_radius_frac=DEFAULT_LIGHT_RADIUS_FRAC)
   {
      VecH spec=specular(rough, reflectivity, reflect_col, true, light_radius_frac);
      BRANCH if(clear_coat)
      {
         Half clear_coat_rough=0;
         Half clear_coat_reflect=0.04;

         clear_coat_rough =Sqr(clear_coat_rough);
         clear_coat_rough+=light_radius_frac*(1-clear_coat_rough); // rough=Lerp(light_radius_frac, 1, rough);

         Half clear_coat=F_Schlick(clear_coat_reflect, 1, VdotH);
         spec*=1-clear_coat;
         // TODO: should this also be applied to diffuse?

      #if 1 // faster but still good quality
         spec+=clear_coat*D_GGX_Vis_Kelemen(clear_coat_rough, NdotH_HP, VdotH)*(0.25/PI);
      #else // slower
         spec+=clear_coat*D_GGX_Vis_Smith(clear_coat_rough, NdotH_HP, NdotL, Abs(NdotV), true)/PI; // use "Abs(NdotV)" as it helps greatly with faces away from the camera
      #endif
      }
      return spec;
   }
};
/******************************************************************************/
// PBR REFLECTION
/******************************************************************************/
VecH2 EnvDFGTex(Half rough, Half NdotV) // uses precomputed texture
{
   return TexLod(EnvDFG, VecH2(rough, NdotV)).xy; // need UV clamp
}
/* https://blog.selfshadow.com/publications/s2013-shading-course/lazarov/s2013_pbs_black_ops_2_notes.pdf
originally developed by Lazarov, modified by Karis */
VecH2 EnvDFGLazarovKaris(Half rough, Half NdotV) 
{
   const VecH4 m={-1, -0.0275, -0.572,  0.022},
               a={ 1,  0.0425,  1.04 , -0.04 };
#if 1
   VecH4 r=rough*m+a;
#else
   // rough=1-smooth
   VecH4 r=smooth*(-m)+(a+m);
#endif
   Half mul=Min(r.x*r.x, Quint(1-NdotV))*r.x + r.y;
   return VecH2(-1.04, 1.04)*mul + r.zw;
}
/*
http://miciwan.com/SIGGRAPH2015/course_notes_wip.pdf
NO because has overshots in low reflectivity
VecH2 EnvDFGIwanicki(Half rough, Half NdotV)
{
   Half bias=Exp2(-(7*NdotV+4*rough));
   Half scale=1-bias-rough*Max(bias, Min(Sqrt(rough), 0.739 + 0.323*NdotV)-0.434);
   return VecH2(scale, bias);
}

https://knarkowicz.wordpress.com/2014/12/27/analytical-dfg-term-for-ibl/
NO because for low reflect and high smooth, 'EnvDFGLazarovKaris' looks better
VecH2 EnvDFGLazarovNarkowiczSmooth(Half smooth, Half NdotV)
{
   smooth=Sqr(smooth);
   VecH4 p0 = VecH4( 0.5745, 1.548, -0.02397, 1.301 );
   VecH4 p1 = VecH4( 0.5753, -0.2511, -0.02066, 0.4755 );

   VecH4 t = smooth * p0 + p1;

   Half bias  = Sat( t.x * Min( t.y, Exp2( -7.672 * NdotV ) ) + t.z );
   Half delta = Sat( t.w );
   Half scale = delta - bias;

   return VecH2(scale, bias);
}

https://knarkowicz.wordpress.com/2014/12/27/analytical-dfg-term-for-ibl/
NO because for low reflect, and high smooth it looks the same for long smooth range
VecH2 EnvDFGNarkowiczSmooth(Half smooth, Half NdotV)
{
   smooth=Sqr(smooth);

   Half x = smooth;
   Half y = NdotV;

   Half b1 = -0.1688;
   Half b2 = 1.895;
   Half b3 = 0.9903;
   Half b4 = -4.853;
   Half b5 = 8.404;
   Half b6 = -5.069;
   Half bias = Sat( Min( b1 * x + b2 * x * x, b3 + b4 * y + b5 * y * y + b6 * y * y * y ) );
 
   Half d0 = 0.6045;
   Half d1 = 1.699;
   Half d2 = -0.5228;
   Half d3 = -3.603;
   Half d4 = 1.404;
   Half d5 = 0.1939;
   Half d6 = 2.661;
   Half delta = Sat( d0 + d1 * x + d2 * y + d3 * x * x + d4 * x * y + d5 * y * y + d6 * x * x * x );
   Half scale = delta - bias;
 
   return VecH2(scale, bias);
}*/
VecH ReflectEnv(Half rough, Half reflectivity, VecH reflect_col, Half NdotV, Bool quality)
{
   // currently reflection can be generated even for smooth=0 and reflectivity=0 #SpecularReflectionFromZeroSmoothReflectivity
   VecH2 mad;
 //rough=  Sqr(  rough); // don't do because it decreases highlights too much
 //rough=1-Sqr(1-rough); // don't do because it increases highlights too much
   if(quality)mad=EnvDFGTex         (rough, NdotV);
   else       mad=EnvDFGLazarovKaris(rough, NdotV);

   // energy compensation, increase reflectivity if it's close to 1 to account for multi-bounce https://google.github.io/filament/Filament.html#materialsystem/improvingthebrdfs/energylossinspecularreflectance
#if 1 // Esenthel version
   VecH mul=mad.x+(1-mad.y-mad.x)*reflect_col; // mad.x=Lerp(mad.x, 1-mad.y, reflect_col);
   return reflect_col*mul + mad.y*(REFLECT_OCCL ? Sat(reflectivity*50) : 1);
#else // same results but slower
   return (reflect_col*mad.x + mad.y*(REFLECT_OCCL ? Sat(reflectivity*50) : 1))*(1+reflect_col*(1/(mad.x+mad.y)-1));
#endif
}
Vec ReflectDir(Vec eye_dir, Vec nrm) // High Precision needed for high resolution texture coordinates
{
   return Transform(reflect(eye_dir, nrm), EnvMatrix);
}
VecH ReflectTex(Vec reflect_dir, Half rough)
{
   return TexCubeLodI(Env, reflect_dir, rough*EnvMipMaps).rgb;
}
/******************************************************************************/
#define GLOW_OCCLUSION 0 // glow should not be occluded, so remove occlusion from color and use maximized color
void ProcessGlow(inout Half glow, inout Half diffuse)
{
   glow=SRGBToLinearFast(glow); // have to convert to linear because small glow of 1/255 would give 12.7/255 sRGB (Glow was sampled from non-sRGB texture and stored in RT alpha channel without any gamma conversions), this has to be done first before using glow and before adjusting 'diffuse' (if it's done after 'diffuse' then 'glow' could actually darken colors)
   diffuse*=Max(0, 1-glow); // focus on glow by reducing 'lit_col' where glow is present, glowable pixels should not be affected by lighting (for example if glow light is half covered by shadow, and half not, then half will be darker and half brighter which will look bad)
}
void ProcessGlow(Half glow, VecH base_col, inout VecH col)
{
   if(GLOW_OCCLUSION)col+=base_col*(glow*2); // boost glow by 2 because here we don't maximize 'base_col', so average 'base_col' 0..1 is 0.5, so *2 makes it 1.0
   else {Half max=Max(Max(base_col), 1.0/32); col+=base_col*(glow/max);} // use a decent max value (1/32 was the smallest value that prevented artifacts) to have 2 effects: prevent division by zero, and another very important is for colors close to black (0,0,0) like (1/255, 2/255, 0/255) where color can't be extracted precisely due to low precision and texture compression artifacts, this will make those colors remain very dark and don't affect glow much, thus preventing from showing artifacts
}
void ApplyGlow(Half glow, inout Half diffuse) {ProcessGlow(glow, diffuse);} // !! THIS CAN'T MODIFY 'glow' AS "inout" BECAUSE ORIGINAL IS STILL NEEDED !!
void ApplyGlow(Half glow, VecH base_col, inout Half diffuse, inout VecH col) // !! THIS CAN'T MODIFY 'glow' AS "inout" BECAUSE ORIGINAL IS STILL NEEDED !!
{
   if(glow>0)
   {
      ProcessGlow(glow, diffuse);
      ProcessGlow(glow, base_col, col);
   }
}
/******************************************************************************/
// SHADOWS
/******************************************************************************/
#include "!Set Prec Struct.h"
BUFFER(Shadow)
   Flt     ShdRange      ,
           ShdStep[6]    ;
   Vec2    ShdRangeMulAdd;
   Vec2    ShdJitter     ;
   VecH2   ShdOpacity    ;
   Matrix  ShdMatrix     ;
   Matrix4 ShdMatrix4[6] ;
BUFFER_END

#include "!Set Prec Image.h"

ImageShadow ShdMap;
ImageH      ShdMap1;

#include "!Set Prec Default.h"

Half ShadowFinal(Half shadow) {return shadow*ShdOpacity.x+ShdOpacity.y;}
Vec2 ShadowJitter(Vec2 pixel)
{
   return Noise2D(pixel)*ShdJitter;
}
Vec ShadowDirTransform(Vec pos, Int num)
{  // using "Int/UInt matrix_index" and "matrix_index=.."  and "ShdMatrix4[matrix_index]" was slower 
   // using "Matrix4  m"            and "m=ShdMatrix4[..]" and "p=Transform(pos, m)"      had the same performance as version below
   // imitate binary search
   Vec4 p;
   if(num<=4)
   {
      BRANCH if(num<=2 || pos.z<=ShdStep[1])
      {
         BRANCH if(num<=1 || pos.z<=ShdStep[0])p=Transform(pos, ShdMatrix4[0]);else
                                               p=Transform(pos, ShdMatrix4[1]);
      }else
      {
         BRANCH if(num<=3 || pos.z<=ShdStep[2])p=Transform(pos, ShdMatrix4[2]);else
                                               p=Transform(pos, ShdMatrix4[3]);
      }
   }else
   {
      BRANCH if(num<=3 || pos.z<=ShdStep[2])
      {
         BRANCH if(num<=1 || pos.z<=ShdStep[0])p=Transform(pos, ShdMatrix4[0]);else
         BRANCH if(num<=2 || pos.z<=ShdStep[1])p=Transform(pos, ShdMatrix4[1]);else
                                               p=Transform(pos, ShdMatrix4[2]);
      }else
      {
         BRANCH if(num<=4 || pos.z<=ShdStep[3])p=Transform(pos, ShdMatrix4[3]);else
         BRANCH if(num<=5 || pos.z<=ShdStep[4])p=Transform(pos, ShdMatrix4[4]);else
                                               p=Transform(pos, ShdMatrix4[5]);
      }
   }
   return p.xyz/p.w;
}
Vec ShadowPointTransform(Vec pos)
{
   Vec  local=Transform(pos, ShdMatrix);
   Flt  a=Max(Abs(local));
   Vec4 p;
   // order of matrixes is based on probability of their usage (most frequently light is hanged upwards, making lower texture most used, then sides -x +x -z +z, and last is top y+)
   BRANCH if(-local.y>=a)p=Transform(pos, ShdMatrix4[3]);else
   BRANCH if( local.x>=a)p=Transform(pos, ShdMatrix4[0]);else
   BRANCH if(-local.x>=a)p=Transform(pos, ShdMatrix4[1]);else
   BRANCH if( local.z>=a)p=Transform(pos, ShdMatrix4[4]);else
   BRANCH if(-local.z>=a)p=Transform(pos, ShdMatrix4[5]);else
                         p=Transform(pos, ShdMatrix4[2]);
   return p.xyz/p.w;
}
/******************************************************************************/
Half CompareDepth(Vec pos, Vec2 jitter_value, Bool jitter)
{
   if(jitter)pos.xy+=jitter_value;
   return TexShadow(ShdMap, pos);
}
Half CompareDepth2(Vec pos) // 'ShdMap1' is not a Shadow Map Depth Buffer but a Shadow Intensity Color RT
{
   return TexLod(ShdMap1, pos.xy).x;
}
/******************************************************************************/
Half ShadowDirValue(Vec pos, Vec2 jitter_value, Bool jitter, Int num, Bool cloud)
{
   Half fade=Sat(Length2(pos)*ShdRangeMulAdd.x+ShdRangeMulAdd.y);
   Vec  p=ShadowDirTransform(pos, num);
   if(cloud)return fade+CompareDepth(p, jitter_value, jitter)*CompareDepth2(p);
   else     return fade+CompareDepth(p, jitter_value, jitter);
}
Half ShadowPointValue(Vec pos, Vec2 jitter_value, Bool jitter)
{
   Vec p=ShadowPointTransform(pos);
   return CompareDepth(p, jitter_value, jitter);
}
Half ShadowConeValue(Vec pos, Vec2 jitter_value, Bool jitter)
{
   Vec4 p=Transform(pos, ShdMatrix4[0]); p.xyz/=p.w;
   return CompareDepth(p.xyz, jitter_value, jitter);
}
/******************************************************************************/
struct DeferredOutput // use this structure in Pixel Shader for setting the output of RT_DEFERRED modes
{
   // #RTOutput
   VecH4 out0:TARGET0; // Col, Glow
   VecH4 out1:TARGET1; // Nrm XYZ, #PIXEL_SHADE_MODE
   VecH2 out2:TARGET2; // Rough, Reflect
   VecH2 out3:TARGET3; // Motion (UV delta)

   // set components
   void color (VecH color ) {out0.rgb=color;}
   void glow  (Half glow  ) {out0.w  =glow ;}
   void normal(VecH normal)
   {
   #if SIGNED_NRM_RT
      out1.xyz=normal;
   #else
      out1.xyz=normal*0.5+0.5; // -1..1 -> 0..1
   #endif
   }
   void mode(Int mode) {out1.w=mode/3.0;}

   void rough  (Half rough  ) {out2.x=rough  ;}
   void reflect(Half reflect) {out2.y=reflect;}

   void motion    (Vec projected_prev_pos_xyw, Vec4 pixel) {out3.xy=GetMotion(projected_prev_pos_xyw, pixel);}
   void motion2D  (Vec projected_prev_pos_xyw, Vec4 pixel) {out3.xy=GetMotion(projected_prev_pos_xyw, pixel, false);} // use this only for 2D flat calculations that are known to be in front of camera
   void motionZero(                                      ) {out3.xy=0;}
};
/******************************************************************************/
// TESSELATION
/******************************************************************************/
struct HSData
{
   Flt TessFactor[3]   :SV_TessFactor,
       InsideTessFactor:SV_InsideTessFactor;

   Vec B210:TEXCOORD0,
       B120:TEXCOORD1,
       B021:TEXCOORD2,
       B012:TEXCOORD3,
       B102:TEXCOORD4,
       B201:TEXCOORD5,
       B111:TEXCOORD6;

#define NORMAL_CUBIC_INTERPOLATE 0 // if use cubic interpolation for vertex normals
#if     NORMAL_CUBIC_INTERPOLATE
   VecH N110:NORMAL0,
        N011:NORMAL1,
        N101:NORMAL2;
#endif
};
Vec2 ToScreen(Vec pos)
{
   return pos.xy/Max(0.1, pos.z);
}
HSData GetHSData(Vec pos0, Vec pos1, Vec pos2, VecH nrm0, VecH nrm1, VecH nrm2, Bool shadow_map=false)
{
   HSData O;

 //BRANCH if(AdaptiveTesselation)
   {
      Vec2 p0=ToScreen(shadow_map ? Transform(pos0, ShdMatrix) : pos0),
           p1=ToScreen(shadow_map ? Transform(pos1, ShdMatrix) : pos1),
           p2=ToScreen(shadow_map ? Transform(pos2, ShdMatrix) : pos2);
      O.TessFactor[0]=Dist(p2, p0)*TesselationDensity;
      O.TessFactor[1]=Dist(p0, p1)*TesselationDensity;
      O.TessFactor[2]=Dist(p1, p2)*TesselationDensity;
      O.InsideTessFactor=Avg(O.TessFactor[0], O.TessFactor[1], O.TessFactor[2]);
   }/*else
   {
      O.TessFactor[0]=O.TessFactor[1]=O.TessFactor[2]=Tess;
      O.InsideTessFactor=Tess;
   }*/

   BRANCH if(O.InsideTessFactor>1)
   {
      Vec B003=pos0,
          B030=pos1,
          B300=pos2;

      VecH N002=nrm0,
           N020=nrm1,
           N200=nrm2;

      O.B210=2*B003+B030-Dot(B030-B003,N002)*N002;
      O.B120=2*B030+B003-Dot(B003-B030,N020)*N020;
      O.B021=2*B030+B300-Dot(B300-B030,N020)*N020;
      O.B012=2*B300+B030-Dot(B030-B300,N200)*N200;
      O.B102=2*B300+B003-Dot(B003-B300,N200)*N200;
      O.B201=2*B003+B300-Dot(B300-B003,N002)*N002;

      Vec E=O.B210+O.B120+O.B021+O.B012+O.B102+O.B201,
          V=B003+B030+B300;
      O.B111=E*0.5-V;
   }

#if NORMAL_CUBIC_INTERPOLATE
   Flt V12=2*Dot(B030-B003, N002+N020)/Dot(B030-B003, B030-B003); O.N110=Normalize(N002+N020-V12*(B030-B003));
   Flt V23=2*Dot(B300-B030, N020+N200)/Dot(B300-B030, B300-B030); O.N011=Normalize(N020+N200-V23*(B300-B030));
   Flt V31=2*Dot(B003-B300, N200+N002)/Dot(B003-B300, B003-B300); O.N101=Normalize(N200+N002-V31*(B003-B300));
#endif

   return O;
}
/******************************************************************************/
void SetDSPosNrm(out Vec pos, out VecH nrm, Vec pos0, Vec pos1, Vec pos2, VecH nrm0, VecH nrm1, VecH nrm2, Vec B, HSData hs_data, Bool clamp_tess, Flt clamp_tess_factor)
{
   // TODO: we could encode 'clamp_tess_factor' in vtx.nrm.w

   Flt U=B.x, UU=U*U,
       V=B.y, VV=V*V,
       W=B.z, WW=W*W;

#if NORMAL_CUBIC_INTERPOLATE
   nrm=Normalize(nrm0*WW
                +nrm1*UU
                +nrm2*VV
                +hs_data.N110*(W*U)
                +hs_data.N011*(U*V)
                +hs_data.N101*(W*V));
#else // linear interpolation
   nrm=Normalize(nrm0*W + nrm1*U + nrm2*V);
#endif

   BRANCH if(hs_data.InsideTessFactor>1)
   {
      pos=pos0*(WW*W)
         +pos1*(UU*U)
         +pos2*(VV*V)
         +hs_data.B210*(WW*U)
         +hs_data.B120*(W*UU)
         +hs_data.B201*(WW*V)
         +hs_data.B021*(UU*V)
         +hs_data.B102*(W*VV)
         +hs_data.B012*(U*VV)
         +hs_data.B111*(W*U*V);
      if(clamp_tess)pos=Lerp(pos0*W + pos1*U + pos2*V, pos, clamp_tess_factor);
   }else
   {
      pos=pos0*W + pos1*U + pos2*V;
   }
}
/******************************************************************************/
void TestDepth(inout Flt depth, Flt d, inout VecI2 ofs, VecI2 o)
{
   if(DEPTH_SMALLER(d, depth)){depth=d; ofs=o;}
}
/******************************************************************************/
void DrawLine(inout VecH col, VecH line_col, Vec2 screen, Flt eps, Flt y)
{
   col=Lerp(col, line_col, Sat(Half(1-Abs(screen.y-y)*eps)));
}
/******************************************************************************
Flt CT, SH, AL, WI, SP, TB, MX, MY; // variables that can be used for testing, Ctrl, Shift, ..
/******************************************************************************/
