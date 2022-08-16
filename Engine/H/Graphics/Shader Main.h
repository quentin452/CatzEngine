/******************************************************************************/
#if EE_PRIVATE
struct MainShaderClass
{
   Str         path;
   ShaderFile *shader;

   // get
   Shader* find(C Str8 &name) {return shader->find(name);} // find shader, null on fail
   Shader*  get(C Str8 &name) {return shader-> get(name);} //  get shader, Exit on fail

   // effects
   static void clear(                C Vec4  &color,                                 C Rect *rect=null);
   static void draw (C Image &image,                                                 C Rect *rect=null);
   static void draw (C Image &image, C Color &color, C Color &color_add=TRANSPARENT, C Rect *rect=null);
   static void draw (C Image &image, C Vec4  &color, C Vec4  &color_add=Vec4Zero   , C Rect *rect=null);

   static INLINE Vec4 GetImgSize(C VecI2 &size ) {return Vec4(1.0f/size, size);} // xy=1/size, zw=size, this format is also required by SMAA
          INLINE void    imgSize(C VecI2 &size ) {ImgSize->set(GetImgSize(size));}
          INLINE void    imgSize(C Image &image) {imgSize(image.hwSize());}
          INLINE void     rtSize(C Image &image) {RTSize ->setConditional(GetImgSize(image.hwSize()));}

   // private
   void del           ();
   void createSamplers();
   void create        ();
   void compile       ();
   void getTechniques ();
   void connectRT     ();

   MainShaderClass();

   ShaderImage
      DummyImage,
      *Img  [6], *ImgMS[4],
      *ImgX [4], *ImgXMS,
      *ImgXF[2],
      *ImgXY[3], *ImgXYMS,
      *Env=&DummyImage,
      *Cub[2],
      *Vol,
      *VolXY[2],
      *Depth, *DepthMS,
      *ShdMap[2],

      *Col[4],
      *Nrm[4],
      *Ext[4],
      *Det[4],
      *Mac[4],
      *Lum;

   ShaderRWImage
      *RWImg,
      *RWImgX[2],
      *RWImgXY;

   ShaderParam
       Dummy,

      *ImgSize ,
      *ImgClamp,
      *RTSize  ,
      *Coords  =&Dummy,
      *Viewport=&Dummy,
      *AspectRatio,
      *TemporalOffset,
      *TemporalOffsetStart,
      *DepthWeightScale=&Dummy,

      *ViewMatrix    =&Dummy,
      *ViewMatrixPrev=&Dummy,
      *CamMatrix     =&Dummy,
      *CamMatrixPrev =&Dummy,
      *ProjMatrix    =&Dummy,
      *ProjMatrixPrev=&Dummy,
      *ViewToViewPrev=&Dummy,
      *FurVel   ,
      *ClipPlane,

      *Material        ,
      *MultiMaterial[4],

      *LightDir   ,
      *LightPoint ,
      *LightLinear,
      *LightCone  ,

      *Step,
      *Color[2]  ={&Dummy, &Dummy},
      *BehindBias= &Dummy,

      *FontShadow  ,
      *FontLum     ,
      *FontContrast,
      *FontShade   ,
      *FontDepth   =&Dummy,

      *LightMapScale,

      *GrassRangeMulAdd=&Dummy,
      *BendFactor      =&Dummy,
      *BendFactorPrev  =&Dummy,

      *Volume,

      *RippleParams,

      *AmbientRange_2     =&Dummy,
      *AmbientRangeInvSqr2=&Dummy,
      *AmbientContrast2   =&Dummy,
      *AmbientMin         =&Dummy,
      *AmbientColor_l     =&Dummy, // Vec Linear Gamma
      *AmbientColorNS_l   =&Dummy, // Vec Linear Gamma + NightShade
      *NightShadeColor    =&Dummy,
      *EnvColor           =&Dummy,
      *EnvMipMaps         =&Dummy,

      *HdrBrightness=&Dummy,
      *HdrExp       =&Dummy,
      *HdrMaxDark   =&Dummy,
      *HdrMaxBright =&Dummy,
      *HdrWeight    =&Dummy,

      *TesselationDensity=&Dummy,

      *Sun,
      *SkyFracMulAdd  =&Dummy,
      *SkyDnsMulAdd   =&Dummy,
      *SkyDnsExp      =&Dummy,
      *SkyHorExp      =&Dummy,
      *SkyBoxBlend    =&Dummy,
      *SkyHorCol      =&Dummy,
      *SkySkyCol      =&Dummy,
      *SkyStarOrn     =&Dummy,
      *SkySunHighlight=&Dummy,
      *SkySunPos      =&Dummy,

      *FogColor       ,
      *FogDensity     ,
      *LocalFogColor  ,
      *LocalFogDensity,
      *LocalFogInside ,

      *ShdJitter     =&Dummy,
      *ShdRange      =&Dummy,
      *ShdRangeMulAdd=&Dummy,
      *ShdOpacity    ,
      *ShdStep[6]    ,
      *ShdMatrix     ,
      *ShdMatrix4[6] ,

      *ParticleFrames,

      *DecalParams,
      *OverlayParams,

      *Easu,
      *Rcas=&Dummy,
      
      *SMAAThreshold=&Dummy;

   ShaderParamBool
      *FirstPass,
      *VtxSkinning;

   ShaderParamInt
      *NoiseOffset,
      *TemporalOffsetGatherIndex,
      *TemporalCurPixel;

   // SHADERS
   Shader
      *Draw2DFlat             ,
      *Draw3DFlat             ,
      *Draw2DCol              ,
      *Draw3DCol              ,
      *Draw2DTex              ,
      *Draw2DTexC             ,
      *Draw2DTexCol           ,
      *Draw3DTex     [2][2][2], // [AlphaTest] [Color] [Fog]
      *Draw2DDepthTex[2][2]   , // [AlphaTest] [Color]
      *DrawX                  ,
      *DrawXG                 ,
      *DrawXC[2][2]           , // [Dither] [Gamma]
      *Simple                 ,

      *DrawMask[2][2], // [Alpha][Point]
      *DrawCubeFace,

      *FontCur,
      *FontCurSP,
      *Font  [2][2], // [Depth][Gamma]
      *FontSP[2][2], // [Depth][Gamma]

      *Laser[2],

      *PaletteDraw,

      // BASIC 2D
      *SetCol,
      *Draw[2][2], // [Alpha][Dither]

      *DrawC  ,
      *DrawG  ,
      *DrawCG ,
      *DrawMs1,
      *DrawMsN,
      *DrawMsM,
      *ClearDeferred,
      *ClearLight   ,
      *ColorLUT[2][2][2][2], // [HDR][Dither][InGamma][OutGamma]

      // BLUR
      #define SHADER_BLUR_RANGE 5 // 5 pixel range in both directions
      *BlurX[2], // [High]
      *BlurY[2], // [High]
    /**BlurX_X,
      *BlurY_X,*/

      // MAX
      *MaxX,
      *MaxY,

      // VIDEO
      *YUV[2][2], // [Gamma][Alpha]

      // 2D FX
      *ColTrans   ,
      *ColTransHB ,
      *ColTransHSB,
      *Ripple     ,
      *Titles     ,
      *Fade       ,
      *Wave       ,
      *RadialBlur ,

      // 3D FX
      *Decal[3][2][2][2], // [Mode][FullScreen][Layout][Normal]

      // RENDERING
      *Outline        ,
      *OutlineDS      ,
      *OutlineClip    ,
      *OutlineApply   ,
      *EdgeDetect     ,
      *EdgeDetectApply,
      *DetectMSCol    ,
    //*DetectMSNrm    ,

      *LinearizeDepth[3][2], // [MultiSample] [Perspective]
      *ResolveDepth,
      *SetDepth,
      *SetAlphaFromDepth        [2], // [Sky]
      *SetAlphaFromDepthMS      [2], // [Sky]
      *SetAlphaFromDepthAndCol  [2], // [Sky]
      *SetAlphaFromDepthAndColMS[2], // [Sky]
      *CombineAlpha,
      *ReplaceAlpha,

      // FOG
      *Fog[3]    , // [MultiSample]
      *FogBox    ,
      *FogBox0   ,
      *FogBox1   ,
      *FogHeight ,
      *FogHeight0,
      *FogHeight1,
      *FogBall   ,
      *FogBall0  ,
      *FogBall1  ;
   void loadFogBoxShaders   ();
   void loadFogHeightShaders();
   void loadFogBallShaders  ();

   Shader
      // VOLUME
      *DrawVolume[3][2], // [Inside] [RedGreen as LumAlpha]

      // EDGE SOFTEN
      *FXAA[2], // [Gamma]
   #if SUPPORT_MLAA
      *MLAAEdge ,
      *MLAABlend,
      *MLAA     ,
   #endif
      *SMAAEdge[2], // [Gamma]
      *SMAABlend  ,
      *SMAA       ,
      *Temporal[3][2][2], // [Mode][ViewFull][Alpha]
   #if TEMPORAL_SEPARATE_SUPER_RES_OLD_WEIGHT
      *TemporalOldWeight[2], // [ViewFull]
   #endif

      // PARTICLE
      *Bilb                ,
      *Particle[2][2][3][2], // [Palette] [Soft] [Anim] [MotionAffectsAlpha]

      // POINT
      *DrawTexPoint ,
      *DrawTexPointC,

      // CUBIC
      *DrawTexCubicFast       [2], // [Color]
      *DrawTexCubicFastF   [2][2], // [Alpha][Dither]
      *DrawTexCubicPlus       [2], // [Color]
      *DrawTexCubicPlusF[2][2][2], // [Alpha][Dither][Gamma]

      // FIDELITY FX
      *EASUScreen[2], // [Color]
      *EASU[2][2][2], // [Alpha][Dither][Gamma]
      *RCAS[2][2][2]; // [Alpha][Dither][Gamma]
   void initCubicShaders();   INLINE void loadCubicShaders() {if(SLOW_SHADER_LOAD)initCubicShaders();}

   // SHADOWS
   Shader
      *ShdDir       [6][2][2], // [NumberOfMaps] [Clouds] [MultiSample]
      *ShdPoint           [2], //                         [MultiSample]
      *ShdCone            [2], //                         [MultiSample]
      *ShdBlur      [2][2][4], // [Geom] [LinearDepth] [Quality]
      *ShdBlurX     [2][2]   , // [Geom] [LinearDepth]
      *ShdBlurY     [2][2]   , // [Geom] [LinearDepth]
      *ShdBlurJitter[2][2]   ; // [Geom] [LinearDepth]
   Shader* getShdDir  (Int map_num, Bool clouds, Bool multi_sample);
   Shader* getShdPoint(                          Bool multi_sample);
   Shader* getShdCone (                          Bool multi_sample);

   // LIGHT
   Shader
      *DrawLightDir   [DIFFUSE_NUM][3][LIGHT_MODE_NUM][2]   , // [Diffuse] [MultiSample] [LightMode] [Shadow]
      *DrawLightPoint [DIFFUSE_NUM][3][LIGHT_MODE_NUM][2]   , // [Diffuse] [MultiSample] [LightMode] [Shadow]
      *DrawLightLinear[DIFFUSE_NUM][3][LIGHT_MODE_NUM][2]   , // [Diffuse] [MultiSample] [LightMode] [Shadow]
      *DrawLightCone  [DIFFUSE_NUM][3][LIGHT_MODE_NUM][2][2]; // [Diffuse] [MultiSample] [LightMode] [Shadow] [Image]
   Shader* getDrawLightDir   (Int diffuse, Int multi_sample, Int light_mode, Bool shadow);
   Shader* getDrawLightPoint (Int diffuse, Int multi_sample, Int light_mode, Bool shadow);
   Shader* getDrawLightLinear(Int diffuse, Int multi_sample, Int light_mode, Bool shadow);
   Shader* getDrawLightCone  (Int diffuse, Int multi_sample, Int light_mode, Bool shadow, Bool image);
#if !DEPTH_CLIP_SUPPORTED
   Shader *   DrawLightConeFlat[DIFFUSE_NUM][3][LIGHT_MODE_NUM][2][2]; // [Diffuse] [MultiSample] [LightMode] [Shadow] [Image]
   Shader* getDrawLightConeFlat(Int diffuse, Int multi_sample, Int light_mode, Bool shadow, Bool image);
#endif

   // APPLY LIGHT
   Shader
      *ApplyLight[3][3][2][2][2][2]; // [MultiSample] [ReflectMode] [AmbientOcclusion] [CelShade] [NightShade] [Glow]
   Shader* getApplyLight(Int multi_sample, Int reflect_mode, Bool ao, Bool cel_shade, Bool night_shade, Bool glow);

   // BLOOM
   ShaderParam
      *BloomParams;
   Shader
      *PrecomputedBloomDS    [2][2], // [ViewFull] [HalfRes]
      *BloomDS         [2][2][2][2], // [Glow] [ViewFull] [HalfRes] [Exposure]
      *Bloom[TONE_MAP_NUM][3][2][2]; // [ToneMap] [Alpha] [Dither] [Exposure]
   Shader* getPrecomputedBloomDS(Bool view_full, Bool half_res);
   Shader* getBloomDS(Bool glow, Bool view_full, Bool half_res, Bool exposure);
   Shader* getBloom  (Int tone_map, Int alpha, Bool dither, Bool exposure);

   // SUN
   Shader *SunRays[2][2][2][2]; // [Alpha] [Dither] [Jitter] [Gamma]
   Shader* getSunRays(Bool alpha, Bool dither, Bool jitter, Bool gamma);

   // SKY
   Shader
      *SkyTF   [2]      [2][2], //               [Textures(0->1, 1->2)]                   [Dither] [Cloud]   Textures    Flat
      *SkyT [3][2]      [2][2], // [MultiSample] [Textures(0->1, 1->2)]                   [Dither] [Cloud]   Textures
      *SkyAF   [2]   [2][2][2], //               [PerVertex           ]           [Stars] [Dither] [Cloud]   Atmospheric Flat
      *SkyA [3][2][2][2][2][2]; // [MultiSample] [PerVertex           ] [Density] [Stars] [Dither] [Cloud]   Atmospheric
   Shader* getSkyTF(                  Int  textures  ,                           Bool dither, Bool cloud);
   Shader* getSkyT (Int multi_sample, Int  textures  ,                           Bool dither, Bool cloud);
   Shader* getSkyAF(                  Bool per_vertex,               Bool stars, Bool dither, Bool cloud);
   Shader* getSkyA (Int multi_sample, Bool per_vertex, Bool density, Bool stars, Bool dither, Bool cloud);

   Shader* getSky(Int multi_sample, Bool flat, Bool density, Int textures, Bool stars, Bool dither, Bool per_vertex, Bool cloud);
}extern
   Sh;

struct AmbientOcclusion
{
   Shader *AO[4][2][2]; // [Quality] [Jitter] [Normal]

   Shader* get(Int quality, Bool jitter, Bool normal);
}extern
   AO;

struct LayeredCloudsFx
{
   ShaderFile  *shader;
   ShaderParam *CL[4], *range;
   Shader      *Clouds[4][2]; // [#Layers] [Blend]

   void    load();
   Shader* get(Int layers, Bool blend);
}extern
   LC;

struct VolumetricCloudsFx
{
   ShaderFile  *shader;
   ShaderParam *Cloud, *CloudMap;
   Shader      *Clouds, *CloudsMap,
               *CloudsDraw[2]; // [Gamma]

   void load();
}extern
   VolCloud;

struct VolumetricLights
{
   ShaderFile *shader;
   Shader     *VolDir[6][2], // [ShdMapNum] [Clouds]
              *VolPoint    ,
              *VolLinear   ,
              *VolCone     ,
              *Volumetric  ,
              *VolumetricA ;

   void load();
}extern
   VL;

struct HDR
{
   ShaderFile *shader;
   Shader     *HdrDS[2], // [Step]
              *HdrUpdate,
              *AdaptEye[2]; // [Dither]

   void load();
}extern
   Hdr;

struct MotionBlur
{
   ShaderFile  *shader;
   ShaderParam *MotionScale_2=&Sh.Dummy;
   Shader      *Explosion,
               *SetVel,
               *Convert[6][2]; // [Range][ViewFull]

   struct DilateRange
   {
      Int     range;
      Shader *Dilate;
   }Dilates[10]; // #MotionBlurDilateRanges

   struct BlurRange
   {
      Int     samples;
      Shader *Blur[3][2][2][2][2][2]; // [Glow][Dither][Jitter][Alpha][Temporal][ViewFull]
   }Blurs[4];

   void load();
   Shader     * getConvert(Int range);
 C DilateRange& getDilate (Int range);
   Shader     * getBlur   (Int samples, Int glow, Int dither, Bool alpha);
}extern
   Mtn;

struct DepthOfField
{
   ShaderFile  *shader;
   ShaderParam *DofParams;
   Shader      *DofDS[2][2][2][2], // [ViewFull][Realistic][Alpha][Half]
               *Dof  [2][2][2]   ; // [Dither][Realistic][Alpha]

   struct Pixel
   {
      Int     pixels;
      Shader *BlurX[2], // [Alpha]
             *BlurY[2]; // [Alpha]
   }pixels[11];

   void load();
   Shader* getDS(Bool view_full, Bool realistic, Bool alpha, Bool half_res);
   Shader* get  (Bool dither   , Bool realistic, Bool alpha);
 C Pixel&  pixel(Bool alpha    , Int  pixel);
}extern
   Dof;

struct WaterShader
{
   ShaderFile *shader;
   Shader     *Ocean                , //
              *Lake                 , //
              *River                , //
              *OceanL[7][2][2][2][2], // [Shadows] [Soft ] [ReflectEnv] [ReflectMirror] [Refract]
              *LakeL [7][2][2][2][2], // [Shadows] [Soft ] [ReflectEnv] [ReflectMirror] [Refract]
              *RiverL[7][2][2][2][2], // [Shadows] [Soft ] [ReflectEnv] [ReflectMirror] [Refract]
              *Apply    [2][2][2][2], //           [Depth] [ReflectEnv] [ReflectMirror] [Refract]
              *Under                ;

   ShaderParam
      *WaterMaterial,
      *Water_color_underwater0,
      *Water_color_underwater1,
      *WaterUnderStep,
      *WaterOfsCol,
      *WaterOfsNrm,
      *WaterOfsBump,
      *WaterYMulAdd,
      *WaterPlanePos,
      *WaterPlaneNrm,
      *WaterFlow,
      *WaterReflectMulAdd=&Sh.Dummy,
      *WaterClamp;

   void load();
}extern
   WS;

extern ShaderSampler SamplerPoint, SamplerLinearWrap, SamplerLinearWCC, SamplerLinearCWC, SamplerLinearCWW, SamplerLinearClamp, SamplerMinimum, SamplerMaximum, SamplerFont, SamplerRender, SamplerAnisotropicClamp, SamplerShadowMap;

void Create2DSampler    ();
void CreateFontSampler  ();
void CreateRenderSampler();

Str8 ShaderDeferred   (Int skin, Int materials, Int layout, Int bump_mode, Int alpha_test, Int detail, Int macro, Int color, Int mtrl_blend, Int heightmap, Int fx, Int tesselate);
Str8 ShaderForward    (Int skin, Int materials, Int layout, Int bump_mode, Int alpha_test, Int reflect, Int emissive_map, Int detail, Int color, Int mtrl_blend, Int heightmap, Int fx, Int per_pixel,   Int light_dir, Int light_dir_shd, Int light_dir_shd_num,   Int light_point, Int light_point_shd,   Int light_linear, Int light_linear_shd,   Int light_cone, Int light_cone_shd,   Int tesselate);
Str8 ShaderBlendLight (Int skin, Int color    , Int layout, Int bump_mode, Int alpha_test, Int alpha, Int reflect, Int emissive_map, Int fx, Int per_pixel, Int shadow_maps, Int tesselate);
Str8 ShaderPosition   (Int skin, Int color, Int alpha_test, Int test_blend, Int fx, Int tesselate);
Str8 ShaderBlend      (Int skin, Int color, Int layout, Int bump_mode, Int reflect, Int emissive_map);
Str8 ShaderSetColor   (Int skin, Int color, Int alpha_test, Int tesselate);
Str8 ShaderBehind     (Int skin, Int color, Int alpha_test);
Str8 ShaderEarlyZ     (Int skin);
Str8 ShaderEmissive   (Int skin, Int color, Int alpha_test, Int emissive_map, Int fx, Int tesselate);
Str8 ShaderMeshOverlay(Int skin, Int normal, Int layout);
Str8 ShaderFurBase    (Int skin, Int size, Int diffuse);
Str8 ShaderFurSoft    (Int skin, Int size, Int diffuse);
Str8 ShaderOverlay    (Int skin, Int tesselate);

struct DefaultShaders
{
   Bool valid,
        detail, macro, reflect,
        mtrl_blend,
        heightmap,
        tex, normal, color, size,
        fur, grass, leaf,
        alpha, alpha_blend, alpha_blend_no_light, alpha_blend_light,
        skin,
        tesselate,
        clear_coat;
   Byte materials, alpha_test, layout, bump, emissive, fx;

   void      init(C Material *material[4], MESH_FLAG mesh_flag, Int lod_index, Bool heightmap);
   DefaultShaders(C Material *material[4], MESH_FLAG mesh_flag, Int lod_index, Bool heightmap) {init(material, mesh_flag, lod_index, heightmap);}
   DefaultShaders(C Material *material   , MESH_FLAG mesh_flag, Int lod_index, Bool heightmap);

   Shader* EarlyZ  ()C;
   Shader* Opaque  (Bool mirror=false)C;
   Shader* Overlay ()C;
   Shader* Emissive()C;
   Shader* Outline ()C;
   Shader* Behind  ()C;
   Shader* Fur     ()C;
   Shader* Shadow  ()C;
   Shader* Blend   ()C;
   Shader* get     (RENDER_MODE mode)C;
   FRST  * Frst    ()C;
   BLST  * Blst    ()C;

   void set(Shader *shader[RM_SHADER_NUM], FRST **frst, BLST **blst);
};
INLINE void DisableSkinning() {Sh.VtxSkinning->setConditional(false);}
INLINE void  EnableSkinning() {Sh.VtxSkinning->setConditional(true );}
#endif
/******************************************************************************/
