/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
MainShaderClass    Sh;
AmbientOcclusion   AO;
   LayeredCloudsFx LC;
VolumetricCloudsFx VolCloud;
VolumetricLights   VL;
HDR                Hdr;
MotionBlur         Mtn;
DepthOfField       Dof;
WaterShader        WS;

ShaderSampler SamplerPoint, SamplerLinearWrap, SamplerLinearWCC, SamplerLinearCWC, SamplerLinearCWW, SamplerLinearClamp, SamplerMinimum, SamplerMaximum, SamplerFont, SamplerAnisotropic, SamplerAnisotropicClamp, SamplerShadowMap;
/******************************************************************************/
// MAIN SHADER
/******************************************************************************/
void Create2DSampler()
{
#if DX11
   D3D11_SAMPLER_DESC sd; Zero(sd);
   sd.MipLODBias    =D.imageMipBias();
   sd.MaxAnisotropy =1;
   sd.MinLOD        =0;
   sd.MaxLOD        =FLT_MAX;
   sd.ComparisonFunc=D3D11_COMPARISON_NEVER;
   sd.Filter        =D3D11_FILTER_MIN_MAG_MIP_LINEAR;

   sd.AddressU=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;
   SamplerLinearWrap.create(sd);

   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   SamplerLinearClamp.create(sd);
#elif GL
   if(SamplerLinearWrap .sampler)glSamplerParameterf(SamplerLinearWrap .sampler, GL_TEXTURE_LOD_BIAS, D.imageMipBias());
   if(SamplerLinearClamp.sampler)glSamplerParameterf(SamplerLinearClamp.sampler, GL_TEXTURE_LOD_BIAS, D.imageMipBias());
#if GL_ES
   if(SamplerLinearWrap .sampler_no_filter)glSamplerParameterf(SamplerLinearWrap .sampler_no_filter, GL_TEXTURE_LOD_BIAS, D.imageMipBias());
   if(SamplerLinearClamp.sampler_no_filter)glSamplerParameterf(SamplerLinearClamp.sampler_no_filter, GL_TEXTURE_LOD_BIAS, D.imageMipBias());
#endif
#endif
}
void CreateFontSampler()
{
#if DX11
   D3D11_SAMPLER_DESC sd; Zero(sd);
   sd.Filter  =D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.MipLODBias    =D.fontMipBias();
   sd.MaxAnisotropy =1;
   sd.MinLOD        =0;
   sd.MaxLOD        =FLT_MAX;
   sd.ComparisonFunc=D3D11_COMPARISON_NEVER;
   SamplerFont.create(sd);
#elif GL
   if(SamplerFont.sampler)glSamplerParameterf(SamplerFont.sampler, GL_TEXTURE_LOD_BIAS, D.fontMipBias());
#if GL_ES
   if(SamplerFont.sampler_no_filter)glSamplerParameterf(SamplerFont.sampler_no_filter, GL_TEXTURE_LOD_BIAS, D.fontMipBias());
#endif
#endif
}
void CreateAnisotropicSampler()
{
#if DX11
   D3D11_SAMPLER_DESC  sd; Zero(sd);
   if(D.texFilter()> 1)sd.Filter=D3D11_FILTER_ANISOTROPIC;else
   if(D.texFilter()==1)sd.Filter=(D.texMipFilter() ? D3D11_FILTER_MIN_MAG_MIP_LINEAR              : D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT);else
                       sd.Filter=(D.texMipFilter() ? D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR : D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT);
   sd.AddressU=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.MipLODBias    =    D.texMipBias();
   sd.MaxAnisotropy =Mid(D.texFilter (), 1, 16);
   sd.MinLOD        =    D.texMipMin ();
   sd.MaxLOD        =FLT_MAX;
   sd.ComparisonFunc=D3D11_COMPARISON_NEVER;
   SamplerAnisotropic.create(sd);

   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   SamplerAnisotropicClamp.create(sd);
#elif GL
   if(SamplerAnisotropic.sampler)
   {
      glSamplerParameteri(SamplerAnisotropic.sampler, GL_TEXTURE_MAX_ANISOTROPY, Max(D.texFilter   (), 1));
      glSamplerParameteri(SamplerAnisotropic.sampler, GL_TEXTURE_MIN_FILTER    ,     D.texMipFilter() ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
      glSamplerParameteri(SamplerAnisotropic.sampler, GL_TEXTURE_MAG_FILTER    ,     D.texFilter   () ? GL_LINEAR : GL_NEAREST);
      glSamplerParameteri(SamplerAnisotropic.sampler, GL_TEXTURE_BASE_LEVEL    ,     D.texMipMin   ());
      glSamplerParameterf(SamplerAnisotropic.sampler, GL_TEXTURE_LOD_BIAS      ,     D.texMipBias  ());
   }
   if(SamplerAnisotropicClamp.sampler)
   {
      glSamplerParameteri(SamplerAnisotropicClamp.sampler, GL_TEXTURE_MAX_ANISOTROPY, Max(D.texFilter   (), 1));
      glSamplerParameteri(SamplerAnisotropicClamp.sampler, GL_TEXTURE_MIN_FILTER    ,     D.texMipFilter() ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
      glSamplerParameteri(SamplerAnisotropicClamp.sampler, GL_TEXTURE_MAG_FILTER    ,     D.texFilter   () ? GL_LINEAR : GL_NEAREST);
      glSamplerParameteri(SamplerAnisotropicClamp.sampler, GL_TEXTURE_BASE_LEVEL    ,     D.texMipMin   ());
      glSamplerParameterf(SamplerAnisotropicClamp.sampler, GL_TEXTURE_LOD_BIAS      ,     D.texMipBias  ());
   }
#if GL_ES
   if(SamplerAnisotropic.sampler_no_filter)
   {
      glSamplerParameteri(SamplerAnisotropic.sampler_no_filter, GL_TEXTURE_MAX_ANISOTROPY,        Max(D.texFilter   (), 1));
      glSamplerParameteri(SamplerAnisotropic.sampler_no_filter, GL_TEXTURE_MIN_FILTER    , GLNoFilter(D.texMipFilter() ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST));
      glSamplerParameteri(SamplerAnisotropic.sampler_no_filter, GL_TEXTURE_MAG_FILTER    , GLNoFilter(D.texFilter   () ? GL_LINEAR : GL_NEAREST));
      glSamplerParameteri(SamplerAnisotropic.sampler_no_filter, GL_TEXTURE_BASE_LEVEL    ,            D.texMipMin   ());
      glSamplerParameterf(SamplerAnisotropic.sampler_no_filter, GL_TEXTURE_LOD_BIAS      ,            D.texMipBias  ());
   }
   if(SamplerAnisotropicClamp.sampler_no_filter)
   {
      glSamplerParameteri(SamplerAnisotropicClamp.sampler_no_filter, GL_TEXTURE_MAX_ANISOTROPY,        Max(D.texFilter   (), 1));
      glSamplerParameteri(SamplerAnisotropicClamp.sampler_no_filter, GL_TEXTURE_MIN_FILTER    , GLNoFilter(D.texMipFilter() ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST));
      glSamplerParameteri(SamplerAnisotropicClamp.sampler_no_filter, GL_TEXTURE_MAG_FILTER    , GLNoFilter(D.texFilter   () ? GL_LINEAR : GL_NEAREST));
      glSamplerParameteri(SamplerAnisotropicClamp.sampler_no_filter, GL_TEXTURE_BASE_LEVEL    ,            D.texMipMin   ());
      glSamplerParameterf(SamplerAnisotropicClamp.sampler_no_filter, GL_TEXTURE_LOD_BIAS      ,            D.texMipBias  ());
   }
#endif
#endif
}
/******************************************************************************/
static Vec4 DummyData[1]; ASSERT(SIZE(DummyData)==MIN_SHADER_PARAM_DATA_SIZE);
MainShaderClass::MainShaderClass()
{
   // 'Dummy' is used so we can already pre-assign some shader handles to this 'Dummy' at engine startup, so we don't have to check if they're not null "if(Sh.param)Sh.param->set(..)" but just operate on them straight away "Sh.param->set(..)"
   Dummy._data   =(Byte*)DummyData;
   Dummy._changed=(Bool*)DummyData;
}
/******************************************************************************/
void MainShaderClass::del()
{
   // delete all to detect GPU memory leaks when using D3D_DEBUG
   shader        =null;
   ShaderFiles  .del();
   ShaderParams .del(); // params before buffers, because they point to them
   ShaderBuffers.del();
   ShaderImages .del();

   SamplerPoint           .del();
   SamplerLinearWrap      .del();
   SamplerLinearWCC       .del();
   SamplerLinearCWC       .del();
   SamplerLinearCWW       .del();
   SamplerLinearClamp     .del();
   SamplerMinimum         .del();
   SamplerMaximum         .del();
   SamplerFont            .del();
   SamplerAnisotropic     .del();
   SamplerAnisotropicClamp.del();
   SamplerShadowMap       .del();
}
void MainShaderClass::createSamplers()
{
#if DX11
   D3D11_SAMPLER_DESC sd; Zero(sd);
   sd.MipLODBias    =0;
   sd.MaxAnisotropy =1;
   sd.MinLOD        =0;
   sd.MaxLOD        =FLT_MAX;
   sd.ComparisonFunc=D3D11_COMPARISON_NEVER;

   sd.Filter  =D3D11_FILTER_MIN_MAG_MIP_POINT;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   SamplerPoint.create(sd);

   sd.Filter  =D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   SamplerLinearWCC.create(sd);

   sd.Filter  =D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   SamplerLinearCWC.create(sd);

   sd.Filter  =D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_WRAP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;
   SamplerLinearCWW.create(sd);

   sd.Filter  =D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR; // this must be LINEAR to take Min from multiple texels, POINT would work as 'SamplerPoint'
   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   if(SamplerMinimum.createTry(sd)) // optional because not supported everywhere - https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_filter
   { // create 'SamplerMaximum' only if 'SamplerMinimum' succeeded
      sd.Filter  =D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR; // this must be LINEAR to take Max from multiple texels, POINT would work as 'SamplerPoint'
      sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
      sd.AddressV=D3D11_TEXTURE_ADDRESS_CLAMP;
      sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
      SamplerMaximum.createTry(sd); // optional because not supported everywhere - https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_filter
   }

   // !! THIS AT THE END BECAUSE IT MODIFIES 'ComparisonFunc' !!
   sd.Filter  =D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
   sd.AddressU=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressV=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
   sd.ComparisonFunc=(REVERSE_DEPTH ? D3D11_COMPARISON_GREATER_EQUAL : D3D11_COMPARISON_LESS_EQUAL);
   SamplerShadowMap.create(sd);
#elif GL
         SamplerPoint.filter_min=GL_NEAREST;
         SamplerPoint.filter_mag=GL_NEAREST;
   REPAO(SamplerPoint.address)=GL_CLAMP_TO_EDGE;
         SamplerPoint.create();

         SamplerLinearWrap.filter_min=GL_LINEAR_MIPMAP_LINEAR;
         SamplerLinearWrap.filter_mag=GL_LINEAR;
   REPAO(SamplerLinearWrap.address)=GL_REPEAT;
         SamplerLinearWrap.create();

         SamplerLinearWCC.filter_min=GL_LINEAR_MIPMAP_LINEAR;
         SamplerLinearWCC.filter_mag=GL_LINEAR;
   REPAO(SamplerLinearWCC.address)=GL_CLAMP_TO_EDGE; SamplerLinearWCC.address[0]=GL_REPEAT;
         SamplerLinearWCC.create();

         SamplerLinearCWC.filter_min=GL_LINEAR_MIPMAP_LINEAR;
         SamplerLinearCWC.filter_mag=GL_LINEAR;
   REPAO(SamplerLinearCWC.address)=GL_CLAMP_TO_EDGE; SamplerLinearCWC.address[1]=GL_REPEAT;
         SamplerLinearCWC.create();

         SamplerLinearCWW.filter_min=GL_LINEAR_MIPMAP_LINEAR;
         SamplerLinearCWW.filter_mag=GL_LINEAR;
   REPAO(SamplerLinearCWW.address)=GL_REPEAT; SamplerLinearCWW.address[0]=GL_CLAMP_TO_EDGE;
         SamplerLinearCWW.create();

         SamplerLinearClamp.filter_min=GL_LINEAR_MIPMAP_LINEAR;
         SamplerLinearClamp.filter_mag=GL_LINEAR;
   REPAO(SamplerLinearClamp.address)=GL_CLAMP_TO_EDGE;
         SamplerLinearClamp.create();

// these are optional because not supported everywhere - https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_filter_minmax.txt
#ifdef GL_TEXTURE_REDUCTION_MODE_EXT
         SamplerMinimum.filter_min=GL_LINEAR; // this must be GL_LINEAR to take Min from multiple texels, GL_NEAREST would work as 'SamplerPoint'
         SamplerMinimum.filter_mag=GL_LINEAR; // this must be GL_LINEAR to take Min from multiple texels, GL_NEAREST would work as 'SamplerPoint'
   REPAO(SamplerMinimum.address)=GL_CLAMP_TO_EDGE;
         SamplerMinimum.create();

         SamplerMaximum.filter_min=GL_LINEAR; // this must be GL_LINEAR to take Max from multiple texels, GL_NEAREST would work as 'SamplerPoint'
         SamplerMaximum.filter_mag=GL_LINEAR; // this must be GL_LINEAR to take Max from multiple texels, GL_NEAREST would work as 'SamplerPoint'
   REPAO(SamplerMaximum.address)=GL_CLAMP_TO_EDGE;
         SamplerMaximum.create();

   if(SamplerMinimum.is())
   {
      glGetError(); // clear any previous errors
      glSamplerParameteri(SamplerMinimum.sampler, GL_TEXTURE_REDUCTION_MODE_EXT, GL_MIN);
      if(glGetError()!=GL_NO_ERROR)SamplerMinimum.del(); // delete on error
   }
   if(SamplerMaximum.is())
   {
      glGetError(); // clear any previous errors
      glSamplerParameteri(SamplerMaximum.sampler, GL_TEXTURE_REDUCTION_MODE_EXT, GL_MAX);
      if(glGetError()!=GL_NO_ERROR)SamplerMaximum.del(); // delete on error
   }
   #if GL_ES
      if(SamplerMinimum.sampler_no_filter)glSamplerParameteri(SamplerMinimum.sampler_no_filter, GL_TEXTURE_REDUCTION_MODE_EXT, GL_MIN);
      if(SamplerMaximum.sampler_no_filter)glSamplerParameteri(SamplerMaximum.sampler_no_filter, GL_TEXTURE_REDUCTION_MODE_EXT, GL_MAX);
   #endif
#endif

         SamplerFont.filter_min=GL_LINEAR_MIPMAP_LINEAR;
         SamplerFont.filter_mag=GL_LINEAR;
   REPAO(SamplerFont.address)=GL_CLAMP_TO_EDGE;
         SamplerFont.create();

         SamplerAnisotropic.filter_min=GL_LINEAR_MIPMAP_LINEAR;
         SamplerAnisotropic.filter_mag=GL_LINEAR;
   REPAO(SamplerAnisotropic.address)=GL_REPEAT;
         SamplerAnisotropic.create();

         SamplerAnisotropicClamp.filter_min=GL_LINEAR_MIPMAP_LINEAR;
         SamplerAnisotropicClamp.filter_mag=GL_LINEAR;
   REPAO(SamplerAnisotropicClamp.address)=GL_CLAMP_TO_EDGE;
         SamplerAnisotropicClamp.create();

         SamplerShadowMap.filter_min=GL_LINEAR;
         SamplerShadowMap.filter_mag=GL_LINEAR;
   REPAO(SamplerShadowMap.address)=GL_CLAMP_TO_EDGE;
         SamplerShadowMap.create();
      if(SamplerShadowMap.sampler)
      {
         glSamplerParameteri(SamplerShadowMap.sampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
         glSamplerParameteri(SamplerShadowMap.sampler, GL_TEXTURE_COMPARE_FUNC, REVERSE_DEPTH ? GL_GEQUAL : GL_LEQUAL);
      }
   #if GL_ES
      if(SamplerShadowMap.sampler_no_filter)
      {
         glSamplerParameteri(SamplerShadowMap.sampler_no_filter, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
         glSamplerParameteri(SamplerShadowMap.sampler_no_filter, GL_TEXTURE_COMPARE_FUNC, REVERSE_DEPTH ? GL_GEQUAL : GL_LEQUAL);
      }
   #endif
#endif
   Create2DSampler         ();
   CreateFontSampler       ();
   CreateAnisotropicSampler();
}
void MainShaderClass::create()
{
   if(LogInit)LogN("MainShaderClass.create");
   compile();

#if DX11
   path="Shader\\4\\";
#elif GL
   path=(D.SpirVAvailable() ? "Shader\\GL SPIR-V\\" : "Shader\\GL\\");
#else
   #error Unsupported
#endif

   shader=ShaderFiles("Main"); if(!shader)Exit("Can't load the Main Shader");
#define MEASURE_SHADER_LOAD_TIME 0
#if     MEASURE_SHADER_LOAD_TIME
 //#define SLOW_SHADER_LOAD 0
   #pragma message("!! Warning: Use this only for debugging !!")
   Flt t=Time.curTime();
#endif
   getTechniques();
#if MEASURE_SHADER_LOAD_TIME
   Exit(S+"Shaders Loaded in: "+(Time.curTime()-t)+'s');
#endif
   connectRT    ();
}
/******************************************************************************/
void MainShaderClass::clear(                C   Vec4  &color,                       C Rect *rect) {Sh.Color[0]->set(color);                              Sh.SetCol->draw(       rect);}
void MainShaderClass::draw (C Image &image                  ,                       C Rect *rect) {                                                      Sh.Draw  ->draw(image, rect);}
void MainShaderClass::draw (C Image &image, C   Vec4  &color, C   Vec4  &color_add, C Rect *rect) {Sh.Color[0]->set(color); Sh.Color[1]->set(color_add); Sh.DrawC ->draw(image, rect);}
void MainShaderClass::draw (C Image &image, C ::Color &color, C ::Color &color_add, C Rect *rect) {Sh.Color[0]->set(color); Sh.Color[1]->set(color_add); Sh.DrawC ->draw(image, rect);}
/******************************************************************************/
Shader* MainShaderClass::getBloomDS(Bool glow, Bool uv_clamp, Bool half_res) {return get(S8+"BloomDS"+glow+uv_clamp+half_res);}
Shader* MainShaderClass::getBloom  (Bool dither, Bool alpha                ) {return get(S8+"Bloom"  +dither+alpha);}

Shader* MainShaderClass::getShdDir  (Int map_num, Bool clouds, Bool multi_sample) {return get(S8+"ShdDir"  +multi_sample+map_num+clouds);}
Shader* MainShaderClass::getShdPoint(                          Bool multi_sample) {return get(S8+"ShdPoint"+multi_sample);}
Shader* MainShaderClass::getShdCone (                          Bool multi_sample) {return get(S8+"ShdCone" +multi_sample);}

Shader* MainShaderClass::getDrawLightDir   (Int diffuse, Bool shadow, Bool multi_sample, Bool water            ) {return get(S8+"DrawLightDir"   +diffuse+shadow+multi_sample+water            );}
Shader* MainShaderClass::getDrawLightPoint (Int diffuse, Bool shadow, Bool multi_sample, Bool water            ) {return get(S8+"DrawLightPoint" +diffuse+shadow+multi_sample+water      +GL_ES);}
Shader* MainShaderClass::getDrawLightLinear(Int diffuse, Bool shadow, Bool multi_sample, Bool water            ) {return get(S8+"DrawLightLinear"+diffuse+shadow+multi_sample+water      +GL_ES);}
Shader* MainShaderClass::getDrawLightCone  (Int diffuse, Bool shadow, Bool multi_sample, Bool water, Bool image) {return get(S8+"DrawLightCone"  +diffuse+shadow+multi_sample+water+image+GL_ES);}
#if !DEPTH_CLIP_SUPPORTED
Shader* MainShaderClass::getDrawLightConeFlat(Int diffuse, Bool shadow, Bool multi_sample, Bool water, Bool image) {return get(S8+"DrawLightConeFlat"+diffuse+shadow+multi_sample+water+image);}
#endif

Shader* MainShaderClass::getApplyLight(Int multi_sample, Bool ao, Bool cel_shade, Bool night_shade, Bool glow, Bool reflect) {return get(S8+"ApplyLight"+multi_sample+ao+cel_shade+night_shade+glow+reflect);}

Shader* MainShaderClass::getSunRaysMask(Bool mask                                      ) {return get(S8+"SunRaysMask"+mask);}
Shader* MainShaderClass::getSunRays    (Bool mask, Bool dither, Bool jitter, Bool gamma) {return get(S8+"SunRays"    +mask+dither+jitter+gamma);}

Shader* MainShaderClass::getSky(Int multi_sample, Bool flat, Bool density, Int textures, Bool stars, Bool dither, Bool per_vertex, Bool cloud) {return get(S8+"Sky"+multi_sample+flat+density+textures+stars+dither+per_vertex+cloud);}

Shader* MainShaderClass::getSkyTF(                  Int  textures  ,                           Bool dither, Bool cloud) {Int multi_sample=0;                 Bool flat=true , density=false, stars=false, per_vertex=false; return getSky(multi_sample, flat, density, textures, stars, dither, per_vertex, cloud);}
Shader* MainShaderClass::getSkyT (Int multi_sample, Int  textures  ,                           Bool dither, Bool cloud) {                                    Bool flat=false, density=false, stars=false, per_vertex=false; return getSky(multi_sample, flat, density, textures, stars, dither, per_vertex, cloud);}
Shader* MainShaderClass::getSkyAF(                  Bool per_vertex,               Bool stars, Bool dither, Bool cloud) {Int multi_sample=0; Int textures=0; Bool flat=true , density=false                               ; return getSky(multi_sample, flat, density, textures, stars, dither, per_vertex, cloud);}
Shader* MainShaderClass::getSkyA (Int multi_sample, Bool per_vertex, Bool density, Bool stars, Bool dither, Bool cloud) {                    Int textures=0; Bool flat=false                                              ; return getSky(multi_sample, flat, density, textures, stars, dither, per_vertex, cloud);}

void MainShaderClass::initCubicShaders()
{
   if(!DrawTexCubic[0]) // check if not yet initialized because this is called multiple times for SLOW_SHADER_LOAD
   {
      REPD(color , 2)DrawTexCubic[color]=get(S8+"DrawTexCubic"+color);
      REPD(dither, 2)
      {
         DrawTexCubicF   [dither]=get(S8+"DrawTexCubicF"   +dither);
         DrawTexCubicFRGB[dither]=get(S8+"DrawTexCubicFRGB"+dither);
      }
   }
}
void MainShaderClass::loadFogBoxShaders()
{
   if(!FogBox) // check if not yet initialized because this is called multiple times
   {
      ShaderFile &sf=*ShaderFiles("Fog Local");
      FogBox =sf.get("FogBox"  );
      FogBox0=sf.get("FogBoxI0");
      FogBox1=sf.get("FogBoxI1");
   }
}
void MainShaderClass::loadFogHeightShaders()
{
   if(!FogHeight) // check if not yet initialized because this is called multiple times
   {
      ShaderFile &sf=*ShaderFiles("Fog Local");
      FogHeight =sf.get("FogHeight"  );
      FogHeight0=sf.get("FogHeightI0");
      FogHeight1=sf.get("FogHeightI1");
   }
}
void MainShaderClass::loadFogBallShaders()
{
   if(!FogBall) // check if not yet initialized because this is called multiple times
   {
      ShaderFile &sf=*ShaderFiles("Fog Local");
      FogBall =sf.get("FogBall"  );
      FogBall0=sf.get("FogBallI0");
      FogBall1=sf.get("FogBallI1");
   }
}

void MainShaderClass::getTechniques()
{
   // images
   Img[0]=ShaderImages("Img" );
   Img[1]=ShaderImages("Img1");
   Img[2]=ShaderImages("Img2");
   Img[3]=ShaderImages("Img3");
   Img[4]=ShaderImages("Img4");
   Img[5]=ShaderImages("Img5");

   ImgMS[0]=ShaderImages("ImgMS" );
   ImgMS[1]=ShaderImages("ImgMS1");
   ImgMS[2]=ShaderImages("ImgMS2");
   ImgMS[3]=ShaderImages("ImgMS3");

   ImgX[0] =ShaderImages("ImgX"   );
   ImgX[1] =ShaderImages("ImgX1"  );
   ImgX[2] =ShaderImages("ImgX2"  );
   ImgX[3] =ShaderImages("ImgX3"  );
   ImgXMS  =ShaderImages("ImgXMS" );
   ImgXF[0]=ShaderImages("ImgXF"  );
   ImgXF[1]=ShaderImages("ImgXF1" );
   ImgXY[0]=ShaderImages("ImgXY"  );
   ImgXY[1]=ShaderImages("ImgXY1" );
   ImgXY[2]=ShaderImages("ImgXY2" );
   ImgXYMS =ShaderImages("ImgXYMS");

   Env   =ShaderImages("Env"); Env->set(D.envMap()());
   Cub[0]=ShaderImages("Cub");
   Cub[1]=ShaderImages("Cub1");

   Vol     =ShaderImages("Vol"    );
   VolXY[0]=ShaderImages("VolXY"  );
   VolXY[1]=ShaderImages("VolXY1" );

   Depth    =ShaderImages("Depth"  );
   DepthMS  =ShaderImages("DepthMS");
   ShdMap[0]=ShaderImages("ShdMap" );
   ShdMap[1]=ShaderImages("ShdMap1");

   // material textures
   Col[0]=ShaderImages("Col" );
   Col[1]=ShaderImages("Col1");
   Col[2]=ShaderImages("Col2");
   Col[3]=ShaderImages("Col3");
   Nrm[0]=ShaderImages("Nrm" );
   Nrm[1]=ShaderImages("Nrm1");
   Nrm[2]=ShaderImages("Nrm2");
   Nrm[3]=ShaderImages("Nrm3");
   Ext[0]=ShaderImages("Ext" );
   Ext[1]=ShaderImages("Ext1");
   Ext[2]=ShaderImages("Ext2");
   Ext[3]=ShaderImages("Ext3");
   Det[0]=ShaderImages("Det" );
   Det[1]=ShaderImages("Det1");
   Det[2]=ShaderImages("Det2");
   Det[3]=ShaderImages("Det3");
   Mac[0]=ShaderImages("Mac" );
   Mac[1]=ShaderImages("Mac1");
   Mac[2]=ShaderImages("Mac2");
   Mac[3]=ShaderImages("Mac3");
   Lum   =ShaderImages("Lum" );

   ImgSize           =GetShaderParam("ImgSize" );
   ImgClamp          =GetShaderParam("ImgClamp");
   RTSize            =GetShaderParam("RTSize"  );
   Coords            =GetShaderParam("Coords"  );
   Viewport          =GetShaderParam("Viewport");
   AspectRatio       =GetShaderParam("AspectRatio");
   TAAOffset         =GetShaderParam("TAAOffset");
   TAAOffsetCurToPrev=GetShaderParam("TAAOffsetCurToPrev");
   DepthWeightScale  =GetShaderParam("DepthWeightScale");

   ViewMatrix    =GetShaderParam("ViewMatrix"    );
   ViewMatrixPrev=GetShaderParam("ViewMatrixPrev");
   CamMatrix     =GetShaderParam("CamMatrix"     );
   CamMatrixPrev =GetShaderParam("CamMatrixPrev" );
   ProjMatrix    =GetShaderParam("ProjMatrix"    );
   ProjMatrixPrev=GetShaderParam("ProjMatrixPrev");
   ViewToViewPrev=GetShaderParam("ViewToViewPrev");
   FurVel        =GetShaderParam("FurVel"        );
   ClipPlane     =GetShaderParam("ClipPlane"     );

            ConstCast(Renderer.highlight       )=GetShaderParam("Highlight");
   Material=ConstCast(Renderer.material_color_l)=GetShaderParam("Material");
   MultiMaterial[0]=GetShaderParam("MultiMaterial0");
   MultiMaterial[1]=GetShaderParam("MultiMaterial1");
   MultiMaterial[2]=GetShaderParam("MultiMaterial2");
   MultiMaterial[3]=GetShaderParam("MultiMaterial3");

   LightDir   =GetShaderParam("LightDir"   );
   LightPoint =GetShaderParam("LightPoint" );
   LightLinear=GetShaderParam("LightLinear");
   LightCone  =GetShaderParam("LightCone"  );

   Step      =GetShaderParam("Step"      );
   Color[0]  =GetShaderParam("Color[0]"  );
   Color[1]  =GetShaderParam("Color[1]"  );
   BehindBias=GetShaderParam("BehindBias");

   VtxSkinning=GetShaderParamBool("VtxSkinning");

   LightMapScale=GetShaderParam("LightMapScale");

   GrassRangeMulAdd=GetShaderParam("GrassRangeMulAdd");
   BendFactor      =GetShaderParam("BendFactor");
   BendFactorPrev  =GetShaderParam("BendFactorPrev");

   EnvColor           =GetShaderParam    ("EnvColor"           ); EnvColor->set(D.envColor());
   EnvMipMaps         =GetShaderParam    ("EnvMipMaps"         ); if(D.envMap())EnvMipMaps->set(D.envMap()->mipMaps()-1);
   FirstPass          =GetShaderParamBool("FirstPass"          );
   NightShadeColor    =GetShaderParam    ("NightShadeColor"    ); // set in 'D.ambientSet()'
   AmbientColor_l     =GetShaderParam    ("AmbientColor"       ); // set in 'D.ambientSet()'
   AmbientColorNS_l   =GetShaderParam    ("AmbientNSColor"     ); // set in 'D.ambientSet()'
   AmbientContrast2   =GetShaderParam    ("AmbientContrast2"   ); AmbientContrast2->set(D.ambientContrast()*2);
   AmbientMin         =GetShaderParam    ("AmbientMin"         ); AmbientMin      ->set(D.ambientMin     ()  );
   AmbientRange_2     =GetShaderParam    ("AmbientRange_2"     );
   AmbientRangeInvSqr2=GetShaderParam    ("AmbientRangeInvSqr2");
   D.ambientSet(); D.ambientSetRange();

   HdrBrightness=GetShaderParam("HdrBrightness"); HdrBrightness->set(D.eyeAdaptationBrightness());
   HdrExp       =GetShaderParam("HdrExp"       ); HdrExp       ->set(D.eyeAdaptationExp       ());
   HdrMaxDark   =GetShaderParam("HdrMaxDark"   ); HdrMaxDark   ->set(D.eyeAdaptationMaxDark   ());
   HdrMaxBright =GetShaderParam("HdrMaxBright" ); HdrMaxBright ->set(D.eyeAdaptationMaxBright ());
   HdrWeight    =GetShaderParam("HdrWeight"    ); HdrWeight    ->set(D.eyeAdaptationWeight()/4  );

   TesselationDensity=GetShaderParam("TesselationDensity"); TesselationDensity->set(D.tesselationDensity());

   // TECHNIQUES
   Draw2DFlat  =get("Draw2DFlat");
   Draw3DFlat  =get("Draw3DFlat");
   Draw2DCol   =get("Draw2DCol");
   Draw3DCol   =get("Draw3DCol");
   Draw2DTex   =get("Draw2DTex");
   Draw2DTexC  =get("Draw2DTexC");
   Draw2DTexCol=get("Draw2DTexCol");
   REPD(alpha_test, 2)
   REPD(color     , 2)
   {
                  Draw2DDepthTex[alpha_test][color]     =get(S8+"Draw2DDepthTex"+alpha_test+color);
      REPD(fog, 2)Draw3DTex     [alpha_test][color][fog]=get(S8+"Draw3DTex"     +alpha_test+color+fog);
   }
   PaletteDraw=get("PaletteDraw");
   Simple     =get("Simple");

   DrawX =get("DrawX");
   DrawXG=get("DrawXG");
   REPD(dither, 2)
   REPD(gamma , 2)DrawXC[dither][gamma]=get(S8+"DrawXC"+dither+gamma);

 //DrawTexX     =get("DrawTexX"); used by Editor
 //DrawTexY     =get("DrawTexY"); used by Editor
 //DrawTexZ     =get("DrawTexZ"); used by Editor
 //DrawTexW     =get("DrawTexW"); used by Editor
 //DrawTexXG    =get("DrawTexXG"); used by Editor
 //DrawTexYG    =get("DrawTexYG"); used by Editor
 //DrawTexZG    =get("DrawTexZG"); used by Editor
 //DrawTexWG    =get("DrawTexWG"); used by Editor
 //DrawTexXIG   =get("DrawTexXIG"); used by Editor
 //DrawTexYIG   =get("DrawTexYIG"); used by Editor
 //DrawTexZIG   =get("DrawTexZIG"); used by Editor
 //DrawTexWIG   =get("DrawTexWIG"); used by Editor
 //DrawTexXSG   =get("DrawTexXSG"); used by Editor
 //DrawTexXYSG  =get("DrawTexXYSG"); used by Editor
 //DrawTexSG    =get("DrawTexSG"); used by Editor
 //DrawTexNrm   =get("DrawTexNrm"); used by Editor
 //DrawTexDetNrm=get("DrawTexDetNrm"); used by Editor

   // MASK
   REPD(alpha, 2)
   REPD(point, 2)DrawMask[alpha][point]=get(S8+"DrawMask"+alpha+point);

   // POINT (these can be null if failed to load)
   DrawTexPoint =find("DrawTexPoint");
   DrawTexPointC=find("DrawTexPointC");

   // CUBIC (these can be null if failed to load)
   REPD(color , 2)DrawTexCubicFast[color]=get(S8+"DrawTexCubicFast"+color);
   REPD(dither, 2)
   {
      DrawTexCubicFastF   [dither]=find(S8+"DrawTexCubicFastF"   +dither);
      DrawTexCubicFastFRGB[dither]=find(S8+"DrawTexCubicFastFRGB"+dither);
   }
#if !SLOW_SHADER_LOAD
   initCubicShaders();
#endif

   // FONT
   FontShadow  =GetShaderParam("FontShadow"  );
   FontLum     =GetShaderParam("FontLum"     );
   FontContrast=GetShaderParam("FontContrast");
   FontShade   =GetShaderParam("FontShade"   );
   FontDepth   =GetShaderParam("FontDepth"   );

   REPD(depth, 2)
   REPD(gamma, 2)
   {
      Font  [depth][gamma]=get(S8+"Font"  +depth+gamma);
      FontSP[depth][gamma]=get(S8+"FontSP"+depth+gamma);
   }

   // BASIC 2D
   Dither=get("Dither");
   SetCol=get("SetCol");
   Draw  =get("Draw"  );
   DrawC =get("DrawC" );
   DrawG =get("DrawG" );
   DrawCG=get("DrawCG");
   DrawA =get("DrawA" );
   if(D.shaderModel()>=SM_4)
   {
                                 DrawMs1=get("DrawMs1");
                                 DrawMsN=get("DrawMsN");
      if(D.shaderModel()>=SM_4_1)DrawMsM=get("DrawMsM");
   }

   // BLOOM
   BloomParams=GetShaderParam("BloomParams");
#if !SLOW_SHADER_LOAD
   REPD(glow , 2)
   REPD(clamp, 2)
   REPD(half , 2)
      BloomDS[glow][clamp][half]=getBloomDS(glow, clamp, half);

   REPD(dither, 2)
   REPD(alpha , 2)
      Bloom[dither][alpha]=getBloom(dither, alpha);
#endif

   // BLUR
   REPD(samples, 2)
   {
      BlurX[samples]=get(S8+"BlurX"+(samples?6:4));
      BlurY[samples]=get(S8+"BlurY"+(samples?6:4));
   }

   // DEPTH
   REPD(m, (D.shaderModel()>=SM_4_1) ? 3 : (D.shaderModel()>=SM_4) ? 2 : 1)
   REPD(p, 2)LinearizeDepth[m][p]=get(S8+"LinearizeDepth"+m+p);

   SetDepth=get("SetDepth");

   if(D.shaderModel()>=SM_4)
   {
      ResolveDepth=get("ResolveDepth");
      DetectMSCol =get("DetectMSCol");
    //DetectMSNrm =get("DetectMSNrm");
   }

#if !SLOW_SHADER_LOAD
                              SetAlphaFromDepth        =get("SetAlphaFromDepth");
   if(D.shaderModel()>=SM_4_1)SetAlphaFromDepthMS      =get("SetAlphaFromDepthMS");
                              SetAlphaFromDepthAndCol  =get("SetAlphaFromDepthAndCol");
   if(D.shaderModel()>=SM_4_1)SetAlphaFromDepthAndColMS=get("SetAlphaFromDepthAndColMS");
                              CombineAlpha             =get("CombineAlpha");
                              ReplaceAlpha             =get("ReplaceAlpha");
#endif

   // SKY
   Sun            =GetShaderParam("Sun"            );
   SkyDnsExp      =GetShaderParam("SkyDnsExp"      );
   SkyHorExp      =GetShaderParam("SkyHorExp"      );
   SkyBoxBlend    =GetShaderParam("SkyBoxBlend"    );
   SkyHorCol      =GetShaderParam("SkyHorCol"      );
   SkySkyCol      =GetShaderParam("SkySkyCol"      );
   SkyStarOrn     =GetShaderParam("SkyStarOrn"     );
   SkyFracMulAdd  =GetShaderParam("SkyFracMulAdd"  );
   SkyDnsMulAdd   =GetShaderParam("SkyDnsMulAdd"   );
   SkySunHighlight=GetShaderParam("SkySunHighlight");
   SkySunPos      =GetShaderParam("SkySunPos"      );
#if !SLOW_SHADER_LOAD
   REPD(dither, 2)
   REPD(cloud , 2)
   {
      // Textures Flat
      REPD(textures, 2)SkyTF[textures][dither][cloud]=getSkyTF(textures+1, dither, cloud);

      // Atmospheric Flat
      REPD(per_vertex, 2)
      REPD(stars     , 2)SkyAF[per_vertex][stars][dither][cloud]=getSkyAF(per_vertex, stars, dither, cloud);

      REPD(multi_sample, (D.shaderModel()>=SM_4_1) ? 3 : (D.shaderModel()>=SM_4) ? 2 : 1)
      {
         // Textures
         REPD(textures, 2)SkyT[multi_sample][textures][dither][cloud]=getSkyT(multi_sample, textures+1, dither, cloud);

         // Atmospheric
         REPD(per_vertex, 2)
         REPD(density   , 2)
         REPD(stars     , 2)SkyA[multi_sample][per_vertex][density][stars][dither][cloud]=getSkyA(multi_sample, per_vertex, density, stars, dither, cloud);
      }
   }

   REPD(m, 2)SunRaysMask[m]=getSunRaysMask(m);

   REPD(m, 2)
   REPD(d, 2)
   REPD(j, 2)
   REPD(g, 2)
      SunRays[m][d][j][g]=getSunRays(m, d, j, g);
 //SunRaysSoft=get("SunRaysSoft");
#endif

   // SHADOWS
   REPAO(ShdStep      )=GetShaderParam(S8+"ShdStep["+i+']');
         ShdJitter     =GetShaderParam("ShdJitter");
         ShdRange      =GetShaderParam("ShdRange");
         ShdRangeMulAdd=GetShaderParam("ShdRangeMulAdd");
         ShdOpacity    =GetShaderParam("ShdOpacity");
         ShdMatrix     =GetShaderParam("ShdMatrix");
   REPAO(ShdMatrix4   )=GetShaderParam(S8+"ShdMatrix4["+i+']');

   // can be used for shadows in deferred and AO
   {
      Bool gather=D.gatherAvailable(), gl_es=GL_ES;
      REPD(geom        , 2)
      REPD(linear_depth, 2)
      {
         ShdBlur      [geom][linear_depth][0]=get(S8+"ShdBlur"      +geom+linear_depth+gather+gl_es+4);
         ShdBlur      [geom][linear_depth][1]=get(S8+"ShdBlur"      +geom+linear_depth+gather+gl_es+6);
         ShdBlur      [geom][linear_depth][2]=get(S8+"ShdBlur"      +geom+linear_depth+gather+gl_es+8);
         ShdBlur      [geom][linear_depth][3]=get(S8+"ShdBlur"      +geom+linear_depth+gather+gl_es+12);
         ShdBlurX     [geom][linear_depth]   =get(S8+"ShdBlurX"     +geom+linear_depth+gather+gl_es+2);
         ShdBlurY     [geom][linear_depth]   =get(S8+"ShdBlurY"     +geom+linear_depth+gather+gl_es+2);
         ShdBlurJitter[geom][linear_depth]   =get(S8+"ShdBlurJitter"+geom+linear_depth+gather+gl_es);
      }
   }

   if(!D.deferredUnavailable())
   {
      // CLEAR
      ClearDeferred=get("ClearDeferred");
      ClearLight   =get("ClearLight"   );

   #if !SLOW_SHADER_LOAD
      // SHADOW
      REPD(multi_sample, (D.shaderModel()>=SM_4_1) ? 2 : 1)
      {
         REPD(n, 6)
         REPD(c, 2)ShdDir[n][c][multi_sample]=getShdDir  (n+1, c, multi_sample);
                   ShdPoint    [multi_sample]=getShdPoint(        multi_sample);
                   ShdCone     [multi_sample]=getShdCone (        multi_sample);
      }

      // LIGHT
      REPD(diffuse     , DIFFUSE_NUM)
      REPD(multi_sample, (D.shaderModel()>=SM_4_1) ? 2 : 1)
      REPD(shadow      , 2)
      REPD(water       , 2)
      {
                        DrawLightDir   [diffuse][shadow][multi_sample][water]       =getDrawLightDir   (diffuse, shadow, multi_sample, water);
                        DrawLightPoint [diffuse][shadow][multi_sample][water]       =getDrawLightPoint (diffuse, shadow, multi_sample, water);
                        DrawLightLinear[diffuse][shadow][multi_sample][water]       =getDrawLightLinear(diffuse, shadow, multi_sample, water);
         REPD(image, 2){DrawLightCone  [diffuse][shadow][multi_sample][water][image]=getDrawLightCone  (diffuse, shadow, multi_sample, water, image);
                     #if !DEPTH_CLIP_SUPPORTED
                        DrawLightConeFlat[diffuse][shadow][multi_sample][water][image]=getDrawLightConeFlat(diffuse, shadow, multi_sample, water, image);
                     #endif
                       }
      }

      // COL LIGHT
      REPD(multi_sample, (D.shaderModel()>=SM_4_1) ? 3 : 1)
      REPD(ao          , 2)
      REPD(  cel_shade , 2)
      REPD(night_shade , 2)
      REPD(glow        , 2)
      REPD(reflect     , 2)
         ApplyLight[multi_sample][ao][cel_shade][night_shade][glow][reflect]=getApplyLight(multi_sample, ao, cel_shade, night_shade, glow, reflect);
   #endif
   }

   // PARTICLE
   ParticleFrames=GetShaderParam("ParticleFrames");
   REPD(palette             , 2)
   REPD(soft                , 2)
   REPD(anim                , 3)
   REPD(motion_affects_alpha, 2)
      Particle[palette][soft][anim][motion_affects_alpha]=get(S8+"Particle"+palette+soft+anim+ 1 +motion_affects_alpha);
      Bilb                                               =get(S8+"Particle"+      0+   0+   0+ 0 +                   0);

   // FOG
   FogColor       =GetShaderParam(     "FogColor"  );
   FogDensity     =GetShaderParam(     "FogDensity");
   LocalFogColor  =GetShaderParam("LocalFogColor"  );
   LocalFogDensity=GetShaderParam("LocalFogDensity");
   LocalFogInside =GetShaderParam("LocalFogInside" );
                                 Fog[0]=get("Fog0");
   if(D.shaderModel()>=SM_4)
   {
                                 Fog[1]=get("Fog1");
      if(D.shaderModel()>=SM_4_1)Fog[2]=get("Fog2");
   }

   // OVERLAY
   OverlayParams=GetShaderParam("OverlayParams");
}
void MainShaderClass::connectRT()
{
   if(ShdMap[0])ShdMap[0]->set(Renderer._shd_map);
   if(ShdMap[1])ShdMap[1]->set(Renderer._cld_map);
}
/******************************************************************************/
// EFFECTS
/******************************************************************************/
void VolumetricCloudsFx::load()
{
   if(!shader)if(shader=ShaderFiles("Volumetric Clouds"))
   {
      Cloud     =GetShaderParam("Cloud");
      CloudMap  =GetShaderParam("CloudMap");
      Clouds    =shader->get("Clouds");
      CloudsMap =shader->get("CloudsMap");
      REPD(g, 2)CloudsDraw[g]=shader->get(S8+"CloudsDraw"+g);
   }
}
/******************************************************************************/
void VolumetricLights::load()
{
   if(!shader)if(shader=ShaderFiles("Volumetric Lights"))
   {
      REPD(n, 6)
      REPD(c, 2)VolDir[n][c]=shader->get(S8+"VolDir"+(n+1)+c);
                VolPoint    =shader->get(   "VolPoint" );
                VolLinear   =shader->get(   "VolLinear");
                VolCone     =shader->get(   "VolCone"  );
      Volumetric =shader->get("Volumetric0");
      VolumetricA=shader->get("Volumetric1");
   }
}
/******************************************************************************/
void HDR::load()
{
   if(!shader)if(shader=ShaderFiles("Hdr"))
   {
      HdrDS[0] =shader->get("HdrDS0"   );
      HdrDS[1] =shader->get("HdrDS1"   );
      HdrUpdate=shader->get("HdrUpdate");
      Hdr[0]   =shader->get("Hdr0"     );
      Hdr[1]   =shader->get("Hdr1"     );
   }   
}
/******************************************************************************/
void MotionBlur::load()
{
   if(!shader)if(shader=ShaderFiles("Motion Blur"))
   {
      MotionScale_2=GetShaderParam("MotionScale_2"); MotionScale_2->set(D.motionScale()*0.5f);

      //Explosion=shader->get("Explosion");

      SetVel=shader->get("SetVel");

      REPD(range, 6)
      REPD(clamp, 2)Convert[range][clamp]=shader->get(S8+"Convert"+clamp+(1<<range));

      // #MotionBlurDilateRanges
      Dilates[0].range=1;
      Dilates[1].range=2;
      Dilates[2].range=3;
      Dilates[3].range=4;
      Dilates[4].range=6;
      Dilates[5].range=8;
      Dilates[6].range=10;
      Dilates[7].range=12;
      Dilates[8].range=16;
      Dilates[9].range=20;
      Dilates[10].range=24;
      Dilates[11].range=32;
      Dilates[12].range=40;
      Dilates[13].range=48;
      ASSERT(ELMS(Dilates)==14);

      // #MotionBlurSamples
      Blurs[0].samples=5;
      Blurs[1].samples=7;
      Blurs[2].samples=9;
      Blurs[3].samples=14;
      ASSERT(ELMS(Blurs)==4);
   }
}
C MotionBlur::DilateRange& MotionBlur::getDilate(Int range)
{
   DilateRange *dr; FREPA(Dilates) // start from the smallest to find exact match or bigger, order is important
   {
      dr=&Dilates[i]; if(dr->range>=range)break; // if this covers desired samples
   }
   Shader* &shader=dr->Dilate;
   if(!shader)shader=T.shader->get(S8+"Dilate"+dr->range);
   return *dr;
}
Shader* MotionBlur::getBlur(Int samples, Bool dither, Bool alpha)
{
   BlurRange *b; FREPA(Blurs) // start from the smallest to find exact match or bigger, order is important
   {
      b=&Blurs[i]; if(b->samples>=samples)break; // if this covers desired samples
   }
   Shader* &shader=b->Blur[dither][alpha];
   if(!shader)shader=T.shader->get(S8+"Blur"+dither+alpha+b->samples);
   return shader;
}
/******************************************************************************/
Shader* DepthOfField::getDS(Bool clamp , Bool realistic, Bool alpha, Bool half_res) {return shader->get(S8+"DofDS"+clamp+realistic+alpha+half_res+(D.filterMinMaxAvailable() ? 2 : D.gatherAvailable() ? 1 : 0));}
Shader* DepthOfField::get  (Bool dither, Bool realistic, Bool alpha               ) {return shader->get(S8+"Dof"+dither+realistic+alpha);}

void DepthOfField::load()
{
   if(!shader)if(shader=ShaderFiles("Depth of Field"))
   {
      DofParams=GetShaderParam("DofParams");

   #if !SLOW_SHADER_LOAD
      REPD(clamp    , 2)
      REPD(realistic, 2)
      REPD(alpha    , 2)
      REPD(half_res , 2)
         DofDS[clamp][realistic][alpha][half_res]=getDS(clamp, realistic, alpha, half_res);

      REPD(dither   , 2)
      REPD(realistic, 2)
      REPD(alpha    , 2)
         Dof[dither][realistic][alpha]=get(dither, realistic, alpha);
   #endif

      pixels[ 0].pixels=2;
      pixels[ 1].pixels=3;
      pixels[ 2].pixels=4;
      pixels[ 3].pixels=5;
      pixels[ 4].pixels=6;
      pixels[ 5].pixels=7;
      pixels[ 6].pixels=8;
      pixels[ 7].pixels=9;
      pixels[ 8].pixels=10;
      pixels[ 9].pixels=11;
      pixels[10].pixels=12;
      ASSERT(ELMS(pixels)==11);
   }
}
C DepthOfField::Pixel& DepthOfField::pixel(Bool alpha, Int pixel)
{
   Pixel *p;
   FREPA(pixels) // start from the smallest to find exact match or bigger, order is important
   {
      p=&pixels[i]; if(p->pixels>=pixel)break; // if this covers desired range of pixels to blur
   }
   if(!p->BlurX[alpha])
   {
      p->BlurX[alpha]=shader->get(S8+"DofBlurX"+alpha+p->pixels);
      p->BlurY[alpha]=shader->get(S8+"DofBlurY"+alpha+p->pixels);
   }
   return *p;
}
/******************************************************************************/
void WaterShader::load()
{
   if(!shader)if(shader=ShaderFiles("Water"))
   {
      WaterMaterial           =GetShaderParam("WaterMaterial");
      Water_color_underwater0 =GetShaderParam("Water_color_underwater0");
      Water_color_underwater1 =GetShaderParam("Water_color_underwater1");
      Water_refract_underwater=GetShaderParam("Water_refract_underwater");
      WaterUnderStep          =GetShaderParam("WaterUnderStep");
      WaterOfsCol             =GetShaderParam("WaterOfsCol");
      WaterOfsNrm             =GetShaderParam("WaterOfsNrm");
      WaterOfsBump            =GetShaderParam("WaterOfsBump");
      WaterYMulAdd            =GetShaderParam("WaterYMulAdd");
      WaterPlanePos           =GetShaderParam("WaterPlanePos");
      WaterPlaneNrm           =GetShaderParam("WaterPlaneNrm");
      WaterFlow               =GetShaderParam("WaterFlow");
      WaterReflectMulAdd      =GetShaderParam("WaterReflectMulAdd");
      WaterClamp              =GetShaderParam("WaterClamp");

      Bool gather=D.gatherAvailable();
      Lake =shader->get(S8+"Lake" +0+0+0+0+0+0);
      River=shader->get(S8+"River"+0+0+0+0+0+0);
      Ocean=shader->get(S8+"Ocean"+0+0+0+0+0+0);
      REPD(refract, 2)
      {
         REPD(reflect_env   , 2)
         REPD(reflect_mirror, 2)
         {
            REPD(shadow, 7)
            REPD(soft  , 2)
            {
               LakeL [shadow][soft][reflect_env][reflect_mirror][refract]=shader->get(S8+"Lake" +1+shadow+soft+reflect_env+reflect_mirror+refract+gather);
               RiverL[shadow][soft][reflect_env][reflect_mirror][refract]=shader->get(S8+"River"+1+shadow+soft+reflect_env+reflect_mirror+refract+gather);
               OceanL[shadow][soft][reflect_env][reflect_mirror][refract]=shader->get(S8+"Ocean"+1+shadow+soft+reflect_env+reflect_mirror+refract+gather);
            }
            REPD(depth, 2)Apply[depth][reflect_env][reflect_mirror][refract]=shader->get(S8+"Apply"+depth+reflect_env+reflect_mirror+refract+gather);
         }
         Under[refract]=shader->get(S8+"Under"+refract);
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
