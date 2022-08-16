/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#if WINDOWS
   #define COMPILE_DX       0
   #define COMPILE_GL       0
   #define COMPILE_GL_SPIRV 0
#endif

/**
#define MAIN

#define DEFERRED
#define BLEND_LIGHT
#define FORWARD

#define AMBIENT_OCCLUSION
#define BEHIND
#define BLEND
#define BONE_HIGHLIGHT
#define DEPTH_OF_FIELD
#define EARLY_Z
#define EFFECTS_2D
#define EFFECTS_3D
#define EMISSIVE
#define FOG_LOCAL
#define FUR
#define FXAA
#define HDR
#define LAYERED_CLOUDS
#define MOTION_BLUR
#define OVERLAY
#define POSITION
#define SET_COLOR
#define TEMPORAL
#define VOLUMETRIC_CLOUDS
#define VOLUMETRIC_LIGHTS
#define WATER
#define WORLD_EDITOR
/******************************************************************************
#define DX10_INPUT_LAYOUT // #VTX_INPUT_LAYOUT
/******************************************************************************/
// SHADER NAMES
/******************************************************************************/
Str8 ShaderDeferred   (Int skin, Int materials, Int layout, Int bump_mode, Int alpha_test, Int detail, Int macro, Int color, Int mtrl_blend, Int heightmap, Int fx, Int tesselate) {return S8+skin+materials+layout+bump_mode+alpha_test+detail+macro+color+mtrl_blend+heightmap+fx+tesselate;}
Str8 ShaderBlendLight (Int skin, Int color    , Int layout, Int bump_mode, Int alpha_test, Int alpha, Int reflect, Int emissive_map, Int fx, Int per_pixel, Int shadow_maps, Int tesselate) {return S8+skin+color+layout+bump_mode+alpha_test+alpha+reflect+emissive_map+fx+per_pixel+shadow_maps+tesselate;}
Str8 ShaderForward    (Int skin, Int materials, Int layout, Int bump_mode, Int alpha_test, Int reflect, Int emissive_map, Int detail, Int color, Int mtrl_blend, Int heightmap, Int fx, Int per_pixel,   Int light_dir, Int light_dir_shd, Int light_dir_shd_num,   Int light_point, Int light_point_shd,   Int light_linear, Int light_linear_shd,   Int light_cone, Int light_cone_shd,   Int tesselate) {return S8+skin+materials+layout+bump_mode+alpha_test+reflect+emissive_map+detail+color+mtrl_blend+heightmap+fx+per_pixel+light_dir+light_dir_shd+light_dir_shd_num+light_point+light_point_shd+light_linear+light_linear_shd+light_cone+light_cone_shd+tesselate;}

Str8 ShaderBehind     (Int skin, Int color, Int alpha_test) {return S8+skin+color+alpha_test;}
Str8 ShaderBlend      (Int skin, Int color, Int layout, Int bump_mode, Int reflect, Int emissive_map) {return S8+skin+color+layout+bump_mode+reflect+emissive_map;}
Str8 ShaderEarlyZ     (Int skin) {return S8+skin;}
Str8 ShaderEmissive   (Int skin, Int color, Int alpha_test, Int emissive_map, Int fx, Int tesselate) {return S8+skin+color+alpha_test+emissive_map+fx+tesselate;}
Str8 ShaderFurBase    (Int skin, Int size, Int diffuse) {return S8+"Base"+skin+size+diffuse;}
Str8 ShaderFurSoft    (Int skin, Int size, Int diffuse) {return S8+"Soft"+skin+size+diffuse;}
Str8 ShaderPosition   (Int skin, Int color, Int alpha_test, Int test_blend, Int fx, Int tesselate) {return S8+skin+color+alpha_test+test_blend+fx+tesselate;}
Str8 ShaderSetColor   (Int skin, Int color, Int alpha_test, Int tesselate) {return S8+skin+color+alpha_test+tesselate;}
Str8 ShaderOverlay    (Int skin, Int tesselate) {return S8+skin+tesselate;}
Str8 ShaderMeshOverlay(Int skin, Int normal, Int layout) {return S8+skin+normal+layout;}
/******************************************************************************/
#if COMPILE_DX || COMPILE_GL || COMPILE_GL_SPIRV
/******************************************************************************/
static Memx<ShaderCompiler> ShaderCompilers; // use 'Memx' because we store pointers to 'ShaderCompiler'
/******************************************************************************/
// LISTING ALL SHADER TECHNIQUES
/******************************************************************************/
static void Compile(API api, SC_FLAG flag=SC_NONE)
{
   if(!DataPath().is())Exit("Can't compile default shaders - 'DataPath' not specified");

   Str src_path=GetPath(GetPath(__FILE__))+"\\Shaders\\", // for example "C:/Esenthel/Engine/Src/Shaders/"
      dest_path=DataPath();                               // for example "C:/Esenthel/Data/Shader/4/"
   switch(api)
   {
      default: return;

      case API_GL: dest_path+=((flag&SC_SPIRV) ? "Shader\\GL SPIR-V\\" : "Shader\\GL\\"); break;
      case API_DX: dest_path+=                                           "Shader\\4\\"  ; break;
   }
   FCreateDirs(dest_path);
   SHADER_MODEL model=SM_4;

   Bool ms  =(api==API_DX), // if support multi-sampling in shaders
        tess=(api==API_DX); // if support tesselation    in shaders
   Int  fxs[]={FX_GRASS_2D, FX_GRASS_3D, FX_LEAF_2D, FX_LEAF_3D, FX_LEAFS_2D, FX_LEAFS_3D}; // don't list FX_CLEAR_COAT here, because only Deferred shader uses it, so it's processed manually

#ifdef MAIN
{
   ShaderCompiler &compiler=ShaderCompilers.New().set(dest_path+"Main", model, api, flag);
   {
      ShaderCompiler::Source &src=compiler.New(src_path+"Main.cpp");
   #if DEBUG && 0
      #pragma message("!! Warning: Use this only for debugging !!")
      src.New("Test", "DrawUV_VS", "Test_PS")("MODE", 0);
      src.New("Test", "DrawUV_VS", "Test_PS")("MODE", 1);
   #endif
      src.New("Draw2DFlat", "Draw2DFlat_VS", "DrawFlat_PS");
      src.New("Draw3DFlat", "Draw3DFlat_VS", "DrawFlat_PS");
      src.New("SetCol"    , "Draw_VS"      , "DrawFlat_PS");

      src.New("ClearDeferred", "ClearDeferred_VS", "ClearDeferred_PS");
      src.New("ClearLight"   , "Draw_VS"         , "ClearLight_PS");

      REPD(hdr     , 2)
      REPD(dither  , 2)
      REPD(in_gamma, 2)REPD(out_gamma, 2) // we need 2 gamma for in/out because here we need precise gamma conversions
         src.New("ColorLUT", "DrawUV_VS", "ColorLUT_PS")("HDR", hdr, "DITHER", dither, "IN_GAMMA", in_gamma, "OUT_GAMMA", out_gamma);

      src.New("Draw2DCol", "Draw2DCol_VS", "Draw2DCol_PS");
      src.New("Draw3DCol", "Draw3DCol_VS", "Draw3DCol_PS");

      src.New("Draw2DTex" , "DrawScreenUV_VS",  "Draw2DTex_PS");
      src.New("Draw2DTexC", "DrawScreenUV_VS", "Draw2DTexC_PS");

      src.New("DrawTexX", "DrawScreenUV_VS", "DrawTexX_PS");
      src.New("DrawTexY", "DrawScreenUV_VS", "DrawTexY_PS");
      src.New("DrawTexZ", "DrawScreenUV_VS", "DrawTexZ_PS");
      src.New("DrawTexW", "DrawScreenUV_VS", "DrawTexW_PS");

      src.New("DrawTexXG", "DrawScreenUV_VS", "DrawTexXG_PS");
      src.New("DrawTexYG", "DrawScreenUV_VS", "DrawTexYG_PS");
      src.New("DrawTexZG", "DrawScreenUV_VS", "DrawTexZG_PS");
      src.New("DrawTexWG", "DrawScreenUV_VS", "DrawTexWG_PS");

      src.New("DrawTexXIG", "DrawScreenUV_VS", "DrawTexXIG_PS");
      src.New("DrawTexYIG", "DrawScreenUV_VS", "DrawTexYIG_PS");
      src.New("DrawTexZIG", "DrawScreenUV_VS", "DrawTexZIG_PS");
      src.New("DrawTexWIG", "DrawScreenUV_VS", "DrawTexWIG_PS");

      src.New("DrawTexXSG" , "DrawScreenUV_VS", "DrawTexXSG_PS");
      src.New("DrawTexXYSG", "DrawScreenUV_VS", "DrawTexXYSG_PS");
      src.New("DrawTexSG"  , "DrawScreenUV_VS", "DrawTexSG_PS");

      src.New("DrawTexNrm"   , "DrawScreenUV_VS", "DrawTexNrm_PS");
      src.New("DrawTexDetNrm", "DrawScreenUV_VS", "DrawTexDetNrm_PS");

      REPD(alpha , 2)
      REPD(dither, 2)src.New("Draw", "DrawUV_VS", "Draw_PS")("ALPHA", alpha, "DITHER", dither);

      src.New("DrawC" , "DrawUV_VS", "Draw2DTexC_PS");
      src.New("DrawCG", "DrawUV_VS", "DrawTexCG_PS");
      src.New("DrawG" , "DrawUV_VS", "DrawTexG_PS");
      src.New("DrawA" , "DrawUV_VS", "Draw2DTexA_PS");

      REPD(in0, 2)
      REPD(in1, 2)
      REPD(out, 2)src.New("Fade", "DrawUV_VS", "DrawFade_PS")("IN0_GAMMA", in0, "IN1_GAMMA", in1, "OUT_GAMMA", out);

      src.New("DrawX" , "DrawUV_VS", "DrawX_PS" );
      src.New("DrawXG", "DrawUV_VS", "DrawXG_PS");
      REPD(dither, 2)
      REPD(gamma , 2)src.New("DrawXC", "DrawUV_VS", "DrawXC_PS")("DITHER", dither, "GAMMA", gamma);

      src.New("DrawTexPoint" , "DrawScreenUV_VS", "DrawTexPoint_PS");
      src.New("DrawTexPointC", "DrawScreenUV_VS", "DrawTexPointC_PS");

      src.New("Draw2DTexCol", "Draw2DTexCol_VS", "Draw2DTexCol_PS");

      REPD(alpha_test, 2)
      REPD(color     , 2)
      {
                     src.New("Draw2DDepthTex", "Draw2DDepthTex_VS", "Draw2DDepthTex_PS")("ALPHA_TEST", alpha_test, "COLORS", color);
         REPD(fog, 2)src.New("Draw3DTex"     , "Draw3DTex_VS"     , "Draw3DTex_PS"     )("ALPHA_TEST", alpha_test, "COLORS", color, "USE_FOG", fog);
      }

      if(ms) // Multi-Sampling
      {
         src.New("DrawMs1", "Draw_VS", "DrawMs1_PS");
         src.New("DrawMsN", "Draw_VS", "DrawMsN_PS");
         src.New("DrawMsM", "Draw_VS", "DrawMsM_PS").multiSample();

         src.New("DetectMSCol", "Draw_VS", "DetectMSCol_PS");
       //src.New("DetectMSNrm", "Draw_VS", "DetectMSNrm_PS");
      }

      REPD(alpha, 2)
      REPD(point, 2)src.New("DrawMask", "DrawMask_VS", "DrawMask_PS")("ALPHA", alpha, "POINT", point);
      src.New("DrawCubeFace", "DrawCubeFace_VS", "DrawCubeFace_PS");
      src.New("Simple", "Simple_VS", "Simple_PS");

      REPD(sky, 2)
      {
               src.New("SetAlphaFromDepth"        , "DrawPosXY_VS"  , "SetAlphaFromDepth_PS"        )("SKY", sky);
         if(ms)src.New("SetAlphaFromDepthMS"      , "DrawPosXY_VS"  , "SetAlphaFromDepthMS_PS"      )("SKY", sky).multiSample();
               src.New("SetAlphaFromDepthAndCol"  , "DrawUVPosXY_VS", "SetAlphaFromDepthAndCol_PS"  )("SKY", sky);
         if(ms)src.New("SetAlphaFromDepthAndColMS", "DrawUVPosXY_VS", "SetAlphaFromDepthAndColMS_PS")("SKY", sky).multiSample();
      }

      src.New("CombineAlpha", "DrawUV_VS", "CombineAlpha_PS");
      src.New("ReplaceAlpha", "DrawUV_VS", "ReplaceAlpha_PS");

      if(ms)src.New("ResolveDepth", "Draw_VS", "ResolveDepth_PS");
      src.New("SetDepth", "DrawUV_VS", "SetDepth_PS");
      src.New("DrawDepth", "DrawUV_VS", "DrawDepth_PS");

      REPD(perspective, 2)
      {
                src.New("LinearizeDepth0", "DrawUV_VS", "LinearizeDepth0_PS")("PERSPECTIVE", perspective);
         if(ms){src.New("LinearizeDepth1", "Draw_VS"  , "LinearizeDepth1_PS")("PERSPECTIVE", perspective);
                src.New("LinearizeDepth2", "Draw_VS"  , "LinearizeDepth2_PS")("PERSPECTIVE", perspective).multiSample();
         }
      }

      src.New("EdgeDetect"     , "DrawUVPosXY_VS",      "EdgeDetect_PS");
      src.New("EdgeDetectApply", "DrawUV_VS"     , "EdgeDetectApply_PS");

      src.New("PaletteDraw", "DrawUV_VS", "PaletteDraw_PS");

      if(api==API_GL)src.New("WebLToS", "DrawUV_VS", "WebLToS_PS"); // #WebSRGB

      src.New("Params0", S, "Params0_PS").dummy=true;
      src.New("Params1", S, "Params1_PS").dummy=true;
      src.New("Params2", S, "Params2_PS").dummy=true;
   }
   { // BLOOM
      ShaderCompiler::Source &src=compiler.New(src_path+"Bloom.cpp");
      REPD(view_full, 2)
      REPD(half_res , 2)
         src.New("PrecomputedBloomDS", "BloomDS_VS", "BloomDS_PS")("VIEW_FULL", view_full, "HALF_RES", half_res).extra("PRECOMPUTED", 1);
      REPD(exposure, 2)
      {
         REPD(glow     , 2)
         REPD(view_full, 2)
         REPD(half_res , 2)
            src.New("BloomDS", "BloomDS_VS", "BloomDS_PS")("GLOW", glow, "VIEW_FULL", view_full, "HALF_RES", half_res, "EXPOSURE", exposure);

         ASSERT(TONE_MAP_OFF     ==STONE_MAP_OFF
             && TONE_MAP_DEFAULT ==STONE_MAP_DEFAULT
             && TONE_MAP_ACES_LDR==STONE_MAP_ACES_LDR
             && TONE_MAP_ACES_HDR==STONE_MAP_ACES_HDR
             && TONE_MAP_NUM     ==STONE_MAP_NUM);
         REPD(tone_map, TONE_MAP_NUM)
         REPD(alpha   , 3)
         REPD(dither  , 2)
            src.New("Bloom", "Bloom_VS", "Bloom_PS")("TONE_MAP", tone_map, "ALPHA", alpha, "DITHER", dither, "EXPOSURE", exposure);
      }
   }
   { // BLUR
      ShaderCompiler::Source &src=compiler.New(src_path+"Blur.cpp");
      src.New("BlurX", "DrawUV_VS", "BlurX_PS")("SAMPLES", 4);
      src.New("BlurX", "DrawUV_VS", "BlurX_PS")("SAMPLES", 6);
      src.New("BlurY", "DrawUV_VS", "BlurY_PS")("SAMPLES", 4);
      src.New("BlurY", "DrawUV_VS", "BlurY_PS")("SAMPLES", 6);

      src.New("BlurX_X", "DrawUV_VS", "BlurX_X_PS");
      src.New("BlurY_X", "DrawUV_VS", "BlurY_X_PS");

    //src.New("MaxX", "DrawUV_VS", "MaxX_PS");
    //src.New("MaxY", "DrawUV_VS", "MaxY_PS");
   }
   { // CUBIC
      ShaderCompiler::Source &src=compiler.New(src_path+"Cubic.cpp");
      REPD(color, 2)
      {
         src.New("DrawTexCubicFast", "DrawScreenUV_VS", "DrawTexCubicFast_PS")("COLORS", color).extra("ALPHA", true, "DITHER", false);
         src.New("DrawTexCubicPlus", "DrawScreenUV_VS", "DrawTexCubicPlus_PS")("COLORS", color).extra("ALPHA", true, "DITHER", false, "IN_GAMMA", true, "OUT_GAMMA", true);
      }
      REPD(alpha , 2)
      REPD(dither, 2)
      {
       //REPD(in_gamma, 2)REPD(out_gamma, 2)src.New("DrawTexCubicPlusF", "DrawUV_VS", "DrawTexCubicPlus_PS")("ALPHA", alpha, "DITHER", dither, "IN_GAMMA", in_gamma, "OUT_GAMMA", out_gamma); do just one gamma instead of in/out, to avoid having to do expensive gamma conversion in the shader
         REPD(   gamma, 2)                  src.New("DrawTexCubicPlusF", "DrawUV_VS", "DrawTexCubicPlus_PS")("ALPHA", alpha, "DITHER", dither, "IN_GAMMA",    gamma, "OUT_GAMMA",     gamma);
                                            src.New("DrawTexCubicFastF", "DrawUV_VS", "DrawTexCubicFast_PS")("ALPHA", alpha, "DITHER", dither);
      }
   }
   { // FIDELITY_FX
      ShaderCompiler::Source &src=compiler.New(src_path+"FidelityFX/Fidelity FX.cpp");
      REPD(half, 2) // workaround for Nvidia GeForce Half FIXME: TODO: check again in the future if this is still needed #NvidiaGeForceFidelityFXHalf
      {
      REPD(color , 2)
      REPD(gather, 2)
      {
         src.New("EASUScreen", "DrawScreen_VS", "EASU_PS")("COLORS", color, "GATHER", gather, "HALF", half).extra("ALPHA", true, "DITHER", false, "IN_GAMMA", true, "OUT_GAMMA", true).gatherChannel(gather);
      }
      REPD(alpha   , 2)
      REPD(dither  , 2)
    //REPD(in_gamma, 2)REPD(out_gamma, 2) do just one gamma instead of in/out, to avoid having to do expensive gamma conversion in the shader
      REPD(   gamma, 2)
      REPD(gather  , 2)
      {
         src.New("EASU", "Draw_VS", "EASU_PS")("ALPHA", alpha, "DITHER", dither, "IN_GAMMA", gamma, "OUT_GAMMA", gamma, "GATHER", gather)("HALF", half).gatherChannel(gather);
         src.New("RCAS", "Draw_VS", "RCAS_PS")("ALPHA", alpha, "DITHER", dither, "IN_GAMMA", gamma, "OUT_GAMMA", gamma, "GATHER", gather)("HALF", half).gatherChannel(gather); // even though this doesn't use gather, it's used to force Shader Model 5 because lower than that fails to compile halfs (DX Shader Compiler error)
       //src.New( "CAS", "Draw_VS",  "CAS_PS")("ALPHA", alpha, "DITHER", dither, "IN_GAMMA", gamma, "OUT_GAMMA", gamma, "HALF", half);
                      //src.computeNew("EASU")("ALPHA", alpha, "DITHER", dither, "IN_GAMMA", gamma, "OUT_GAMMA", gamma, "HALF", half);
                      //src.computeNew("RCAS")("ALPHA", alpha, "DITHER", dither, "IN_GAMMA", gamma, "OUT_GAMMA", gamma, "HALF", half);
      }
      }
   }
   { // FOG
      ShaderCompiler::Source &src=compiler.New(src_path+"Fog.cpp");
      REPD(multi_sample, ms ? 3 : 1)src.New("Fog", "DrawPosXY_VS", "Fog_PS")("MULTI_SAMPLE", multi_sample).multiSample(multi_sample>=2);
   }
   { // FONT
      ShaderCompiler::Source &src=compiler.New(src_path+"Font.cpp");
      REPD(depth, 2)
      REPD(gamma, 2)
      {
         src.New("Font"  , "Font_VS"  , "Font_PS"  )("SET_DEPTH", depth, "GAMMA", gamma);
         src.New("FontSP", "FontSP_VS", "FontSP_PS")("SET_DEPTH", depth, "GAMMA", gamma);
      }
   }
   { // LIGHT
      ShaderCompiler::Source &src=compiler.New(src_path+"Light.cpp");
      REPD(diffuse     , DIFFUSE_NUM)
      REPD(multi_sample, ms ? 3 : 1)
      REPD(light_mode  , LIGHT_MODE_NUM)
      REPD(shadow      , 2)
      {
                           src.New("DrawLightDir"     , "DrawPosXY_VS", "LightDir_PS"   ).multiSample(multi_sample)("DIFFUSE_MODE", diffuse, "MULTI_SAMPLE", multi_sample, "LIGHT_MODE", light_mode, "SHADOW", shadow); // Directional light is always fullscreen, so can use 2D shader
         REPD(gl_es, (api==API_GL) ? 2 : 1) // GL ES doesn't support NOPERSP and 'D.depthClip'
         {
                           src.New("DrawLightPoint"   , "Geom_VS"     , "LightPoint_PS" ).multiSample(multi_sample)("DIFFUSE_MODE", diffuse, "MULTI_SAMPLE", multi_sample, "LIGHT_MODE", light_mode, "SHADOW", shadow)(                "GL_ES", gl_es).extra("CLAMP_DEPTH", gl_es);  // 3D Geom Mesh (only ball based are depth-clamped, because Dir is fullscreen and Cone has too many artifacts when depth clamping)
                           src.New("DrawLightLinear"  , "Geom_VS"     , "LightLinear_PS").multiSample(multi_sample)("DIFFUSE_MODE", diffuse, "MULTI_SAMPLE", multi_sample, "LIGHT_MODE", light_mode, "SHADOW", shadow)(                "GL_ES", gl_es).extra("CLAMP_DEPTH", gl_es);  // 3D Geom Mesh (only ball based are depth-clamped, because Dir is fullscreen and Cone has too many artifacts when depth clamping)
            REPD(image, 2){src.New("DrawLightCone"    , "Geom_VS"     , "LightCone_PS"  ).multiSample(multi_sample)("DIFFUSE_MODE", diffuse, "MULTI_SAMPLE", multi_sample, "LIGHT_MODE", light_mode, "SHADOW", shadow)("IMAGE", image, "GL_ES", gl_es)                            ;  // 3D Geom Mesh
                  if(gl_es)src.New("DrawLightConeFlat", "DrawPosXY_VS", "LightCone_PS"  ).multiSample(multi_sample)("DIFFUSE_MODE", diffuse, "MULTI_SAMPLE", multi_sample, "LIGHT_MODE", light_mode, "SHADOW", shadow)("IMAGE", image                )                            ;} // 2D Flat
         }
      }
   }
   { // LIGHT APPLY
      ShaderCompiler::Source &src=compiler.New(src_path+"Light Apply.cpp");
      REPD(multi_sample, ms ? 3 : 1)
      REPD(reflect_mode, 3) // None, Reflect, Reflect+ClearCoat
      REPD(ao          , 2)
      REPD(  cel_shade , 2)
      REPD(night_shade , 2)
      REPD(glow        , 2)
         src.New("ApplyLight", "DrawUVPosXY_VS", "ApplyLight_PS")("MULTI_SAMPLE", multi_sample, "REFLECT_MODE", reflect_mode, "AO", ao, "CEL_SHADE", cel_shade, "NIGHT_SHADE", night_shade)("GLOW", glow);
   }
   { // SHADOW
      ShaderCompiler::Source &src=compiler.New(src_path+"Shadow.cpp");
      REPD(multi_sample, ms ? 2 : 1)
      {
         REPD(map_num, 6)
         REPD(cloud  , 2)src.New("ShdDir"  , "DrawPosXY_VS", "ShdDir_PS"  ).multiSample(multi_sample)("MULTI_SAMPLE", multi_sample, "MAP_NUM", map_num+1, "CLOUD", cloud);
                         src.New("ShdPoint", "DrawPosXY_VS", "ShdPoint_PS").multiSample(multi_sample)("MULTI_SAMPLE", multi_sample);
                         src.New("ShdCone" , "DrawPosXY_VS", "ShdCone_PS" ).multiSample(multi_sample)("MULTI_SAMPLE", multi_sample);
      }
      REPD(gl_es       , (api==API_GL) ? 2 : 1) // GL ES doesn't support NOPERSP, 'D.depthClip' and TexDepthLinear (0=no GL ES, 1=GL ES)
      REPD(geom        , 2)
      REPD(linear_depth, 2)
      REPD(gather      , 2)
      {
         CChar8 *vs=(geom ? "Geom_VS" : "DrawUV_VS");
         src.New("ShdBlur"      , vs, "ShdBlur_PS"      )("GEOM", geom, "LINEAR_DEPTH", linear_depth, "GATHER", gather)("GL_ES", gl_es)("SAMPLES",  4).gather(gather);
       //src.New("ShdBlur"      , vs, "ShdBlur_PS"      )("GEOM", geom, "LINEAR_DEPTH", linear_depth, "GATHER", gather)("GL_ES", gl_es)("SAMPLES",  5).gather(gather);
         src.New("ShdBlur"      , vs, "ShdBlur_PS"      )("GEOM", geom, "LINEAR_DEPTH", linear_depth, "GATHER", gather)("GL_ES", gl_es)("SAMPLES",  6).gather(gather);
         src.New("ShdBlur"      , vs, "ShdBlur_PS"      )("GEOM", geom, "LINEAR_DEPTH", linear_depth, "GATHER", gather)("GL_ES", gl_es)("SAMPLES",  8).gather(gather);
       //src.New("ShdBlur"      , vs, "ShdBlur_PS"      )("GEOM", geom, "LINEAR_DEPTH", linear_depth, "GATHER", gather)("GL_ES", gl_es)("SAMPLES",  9).gather(gather);
         src.New("ShdBlur"      , vs, "ShdBlur_PS"      )("GEOM", geom, "LINEAR_DEPTH", linear_depth, "GATHER", gather)("GL_ES", gl_es)("SAMPLES", 12).gather(gather);
       //src.New("ShdBlur"      , vs, "ShdBlur_PS"      )("GEOM", geom, "LINEAR_DEPTH", linear_depth, "GATHER", gather)("GL_ES", gl_es)("SAMPLES", 13).gather(gather);
       //src.New("ShdBlurX"     , vs, "ShdBlurX_PS"     )("GEOM", geom, "LINEAR_DEPTH", linear_depth, "GATHER", gather)("GL_ES", gl_es)("RANGE"  ,  1).gather(gather);
       //src.New("ShdBlurY"     , vs, "ShdBlurY_PS"     )("GEOM", geom, "LINEAR_DEPTH", linear_depth, "GATHER", gather)("GL_ES", gl_es)("RANGE"  ,  1).gather(gather);
         src.New("ShdBlurX"     , vs, "ShdBlurX_PS"     )("GEOM", geom, "LINEAR_DEPTH", linear_depth, "GATHER", gather)("GL_ES", gl_es)("RANGE"  ,  2).gather(gather);
         src.New("ShdBlurY"     , vs, "ShdBlurY_PS"     )("GEOM", geom, "LINEAR_DEPTH", linear_depth, "GATHER", gather)("GL_ES", gl_es)("RANGE"  ,  2).gather(gather);
         src.New("ShdBlurJitter", vs, "ShdBlurJitter_PS")("GEOM", geom, "LINEAR_DEPTH", linear_depth, "GATHER", gather)("GL_ES", gl_es)               .gather(gather);
      }
   }
   { // OUTLINE
      ShaderCompiler::Source &src=compiler.New(src_path+"Outline.cpp");
      src.New("Outline", "DrawUV_VS", "Outline_PS")("DOWN_SAMPLE", 0, "DISCARD", 0);
      src.New("Outline", "DrawUV_VS", "Outline_PS")("DOWN_SAMPLE", 1, "DISCARD", 0);
      src.New("Outline", "DrawUV_VS", "Outline_PS")("DOWN_SAMPLE", 0, "DISCARD", 1);
      src.New("OutlineApply", "DrawUV_VS", "OutlineApply_PS");
   }
   { // PARTICLES
      ShaderCompiler::Source &src=compiler.New(src_path+"Particles.cpp");
      REPD(palette             , 2)
      REPD(soft                , 2)
      REPD(anim                , 3)
      REPD(motion_affects_alpha, 2)
         src.New("Particle", "Particle_VS", "Particle_PS")("PALETTE", palette, "SOFT", soft, "ANIM", anim)("MOTION_STRETCH", 1, "MOTION_AFFECTS_ALPHA", motion_affects_alpha);
         src.New("Particle", "Particle_VS", "Particle_PS")("PALETTE",       0, "SOFT",    0, "ANIM",    0)("MOTION_STRETCH", 0, "MOTION_AFFECTS_ALPHA",                    0);
   }
   { // SKY
      ShaderCompiler::Source &src=compiler.New(src_path+"Sky.cpp");
      REPD(dither, 2)
      REPD(cloud , 2)
      {
         // Textures Flat
         REPD(textures, 2)src.New("Sky", "Sky_VS", "Sky_PS")("MULTI_SAMPLE", 0, "FLAT", 1, "DENSITY", 0, "TEXTURES", textures+1)("STARS", 0, "DITHER", dither, "PER_VERTEX", 0, "CLOUD", cloud);

         // Atmospheric Flat
         REPD(per_vertex, 2)
         REPD(stars     , 2)src.New("Sky", "Sky_VS", "Sky_PS")("MULTI_SAMPLE", 0, "FLAT", 1, "DENSITY", 0, "TEXTURES", 0)("STARS", stars, "DITHER", dither, "PER_VERTEX", per_vertex, "CLOUD", cloud);

         REPD(multi_sample, ms ? 3 : 1)
         {
            // Textures
            REPD(textures, 2)src.New("Sky", "Sky_VS", "Sky_PS")("MULTI_SAMPLE", multi_sample, "FLAT", 0, "DENSITY", 0, "TEXTURES", textures+1)("STARS", 0, "DITHER", dither, "PER_VERTEX", 0, "CLOUD", cloud).multiSample(multi_sample>=2);

            // Atmospheric
            REPD(per_vertex, 2)
            REPD(density   , 2)
            REPD(stars     , 2)src.New("Sky", "Sky_VS", "Sky_PS")("MULTI_SAMPLE", multi_sample, "FLAT", 0, "DENSITY", density, "TEXTURES", 0)("STARS", stars, "DITHER", dither, "PER_VERTEX", per_vertex, "CLOUD", cloud).multiSample(multi_sample>=2);
         }
      }
   }
   { // SMAA
      ShaderCompiler::Source &src=compiler.New(src_path+"SMAA.cpp");
      REPD(gamma, 2)src.New("SMAAEdge" , "SMAAEdge_VS" , "SMAAEdge_PS" )("GAMMA", gamma);
                    src.New("SMAABlend", "SMAABlend_VS", "SMAABlend_PS");
                    src.New("SMAA"     , "SMAA_VS"     , "SMAA_PS"     );
                 #if SUPPORT_MLAA
                    src.New("MLAAEdge" , "MLAA_VS", "MLAAEdge_PS" );
                    src.New("MLAABlend", "MLAA_VS", "MLAABlend_PS");
                    src.New("MLAA"     , "MLAA_VS", "MLAA_PS"     );
                 #endif
   }
   { // SUN
      ShaderCompiler::Source &src=compiler.New(src_path+"Sun.cpp");

      REPD(alpha , 2)
      REPD(dither, 2)
      REPD(jitter, 2)
      REPD(gamma , 2)
         src.New("SunRays", "DrawUVPosXY_VS", "SunRays_PS")("ALPHA", alpha, "DITHER", dither, "JITTER", jitter, "GAMMA", gamma);
   }
   { // VIDEO
      ShaderCompiler::Source &src=compiler.New(src_path+"Video.cpp");
      REPD(gamma    , 2)
      REPD(alpha    , 2)
      REPD(uv_merged, 2)
         src.New("YUV", "DrawScreenUV_VS", "YUV_PS")("GAMMA", gamma, "ALPHA", alpha, "UV_MERGED", uv_merged);
   }
}
#endif

#ifdef AMBIENT_OCCLUSION
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Ambient Occlusion", model, api, flag).New(src_path+"Ambient Occlusion.cpp");
   REPD(quality, 4)
   REPD(jitter , 2)
   REPD(normal , 2)
      src.New("AO", "AO_VS", "AO_PS")("QUALITY", quality, "JITTER", jitter, "NORMALS", normal);
}
#endif

#ifdef BEHIND
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Behind", model, api, flag).New(src_path+"Behind.cpp");
   REPD(skin      , 2)
   REPD(color     , 2)
   REPD(alpha_test, ALPHA_TEST_NUM)
      src.New()("SKIN", skin, "COLORS", color, "ALPHA_TEST", alpha_test);
}
#endif

#ifdef BLEND
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Blend", model, api, flag).New(src_path+"Blend.cpp");

   REPD(skin        , 2)
   REPD(color       , 2)
   REPD(layout      , 3)
   for(Int bump_mode=SBUMP_ZERO; bump_mode<=SBUMP_NORMAL; bump_mode++)
   REPD(reflect     , (bump_mode==SBUMP_ZERO) ? 1 : 2)
   REPD(emissive_map, 2)
      src.New()("SKIN", skin, "COLORS", color, "LAYOUT", layout, "BUMP_MODE", bump_mode)("REFLECT", reflect, "EMISSIVE_MAP", emissive_map);
}
#endif

#ifdef BONE_HIGHLIGHT
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Bone Highlight", model, api, flag).New(src_path+"Bone Highlight.cpp");
   REPD(skin     , 2)
   REPD(bump_mode, 2)src.New()("SKIN", skin, "BUMP_MODE", bump_mode ? SBUMP_FLAT : SBUMP_ZERO);
}
#endif

#ifdef DEPTH_OF_FIELD
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Depth of Field", model, api, flag).New(src_path+"Depth of Field.cpp");
   REPD(view_full, 2)
   REPD(realistic, 2)
   REPD(alpha    , 2)
   REPD(half_res , 2)
   REPD(mode     , 3)
      src.New("DofDS", "DrawUV_VS", "DofDS_PS")("VIEW_FULL", view_full, "REALISTIC", realistic, "ALPHA", alpha, "HALF_RES", half_res)("MODE", mode).gather(mode==1);

   REPD(alpha, 2)
   for(Int range=2; range<=12; range++)
   {
      src.New("DofBlurX", "DrawUV_VS", "DofBlurX_PS")("ALPHA", alpha, "RANGE", range);
      src.New("DofBlurY", "DrawUV_VS", "DofBlurY_PS")("ALPHA", alpha, "RANGE", range);
   }

   REPD(dither   , 2)
   REPD(realistic, 2)
   REPD(alpha    , 2)
      src.New("Dof", "DrawUV_VS", "Dof_PS")("DITHER", dither, "REALISTIC", realistic, "ALPHA", alpha);
}
#endif

#ifdef EARLY_Z
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Early Z", model, api, flag).New(src_path+"Early Z.cpp");
   REPD(skin, 2)src.New()("SKIN", skin);
}
#endif

#ifdef EFFECTS_2D
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Effects 2D", model, api, flag).New(src_path+"Effects 2D.cpp");
   src.New("ColTrans"   , "DrawUV_VS", "ColTrans_PS");
   src.New("ColTransHB" , "DrawUV_VS", "ColTransHB_PS");
   src.New("ColTransHSB", "DrawUV_VS", "ColTransHSB_PS");

   src.New("Fade", "DrawUV_VS", "Fade_PS");
   src.New("RadialBlur", "DrawUV_VS", "RadialBlur_PS");
   src.New("Ripple", "DrawScreenUV_VS", "Ripple_PS");
   src.New("Titles", "DrawUV_VS", "Titles_PS");
   src.New("Wave", "Wave_VS", "Wave_PS");
}
#endif

#ifdef EFFECTS_3D
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Effects 3D", model, api, flag).New(src_path+"Effects 3D.cpp");
   REPD(inside, 3)
   REPD(la    , 2)src.New("Volume", "Volume_VS", "Volume_PS")("INSIDE", inside, "LA", la);

   REPD(normals, 2)src.New("Laser", "Laser_VS", "Laser_PS")("NORMALS", normals);

   REPD(mode      , 3) // 0-overlay, 1-blend, 2-palette
   REPD(fullscreen, 2)
   for(Int layout=1; layout<=((mode==0) ? 2 : 1); layout++)
   REPD(normal    ,          ((mode==0) ? 2 : 1))
      src.New("Decal", "Decal_VS", "Decal_PS")("MODE", mode, "FULLSCREEN", fullscreen, "LAYOUT", layout, "NORMALS", normal);
}
#endif

#ifdef EMISSIVE
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Emissive", model, api, flag).New(src_path+"Emissive.cpp");
   REPD(color, 2)
   {
      REPD(skin        , 2)
      REPD(alpha_test  , ALPHA_TEST_NUM)
      REPD(emissive_map, 2)
      REPD(tesselate   , tess ? 2 : 1)
         src.New()("SKIN", skin, "COLORS", color, "ALPHA_TEST", alpha_test, "EMISSIVE_MAP", emissive_map, "FX", FX_NONE).tesselate(tesselate);

      // grass + leaf
      REPD (emissive_map, 2  )
      REPAD(fx          , fxs)
      REPD (tesselate   , tess ? 2 : 1)
         src.New()("SKIN", false, "COLORS", color, "ALPHA_TEST", true, "EMISSIVE_MAP", emissive_map, "FX", fxs[fx]).tesselate(tesselate);
   }
}
#endif

#ifdef FOG_LOCAL
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Fog Local", model, api, flag).New(src_path+"Fog.cpp");
                  src.New("FogBox" , "FogBox_VS" , "FogBox_PS" )                  .extra("HEIGHT", 0);
   REPD(inside, 2)src.New("FogBoxI", "FogBoxI_VS", "FogBoxI_PS")("INSIDE", inside).extra("HEIGHT", 0);

                  src.New("FogHeight" , "FogBox_VS" , "FogBox_PS" )                  .extra("HEIGHT", 1);
   REPD(inside, 2)src.New("FogHeightI", "FogBoxI_VS", "FogBoxI_PS")("INSIDE", inside).extra("HEIGHT", 1);

                  src.New("FogBall" , "FogBall_VS" , "FogBall_PS" );
   REPD(inside, 2)src.New("FogBallI", "FogBallI_VS", "FogBallI_PS")("INSIDE", inside);
}
#endif

#ifdef FUR
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Fur", model, api, flag).New(src_path+"Fur.cpp");
   REPD(skin   , 2)
   REPD(size   , 2)
   REPD(diffuse, 2)
   {
      src.New("Base", "Base_VS", "Base_PS")("SKIN", skin, "SIZE", size, "DIFFUSE", diffuse); // base
      src.New("Soft", "Soft_VS", "Soft_PS")("SKIN", skin, "SIZE", size, "DIFFUSE", diffuse); // soft
   }
}
#endif

#ifdef FXAA // FXAA unlike SMAA is kept outside of Main shader, because it's rarely used.
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"FXAA", model, api, flag).New(src_path+"FXAA.cpp");
   REPD(gamma, 2)src.New("FXAA", "DrawUV_VS", "FXAA_PS")("GAMMA", gamma);
}
#endif

#ifdef HDR
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Hdr", model, api, flag).New(src_path+"Hdr.cpp");
   REPD(step, 2)src.New("HdrDS", "DrawUV_VS", "HdrDS_PS")("STEP", step);

   src.New("HdrUpdate", "Draw_VS", "HdrUpdate_PS");

   REPD(dither, 2)src.New("AdaptEye", "AdaptEye_VS", "AdaptEye_PS")("DITHER", dither);
}
#endif

#ifdef LAYERED_CLOUDS
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Layered Clouds", model, api, flag).New(src_path+"Layered Clouds.cpp");
   REPD(num  , 4)
   REPD(blend, 2)src.New("Clouds", "LayeredClouds_VS", "LayeredClouds_PS")("NUM", num+1, "BLEND", blend);
#endif

#ifdef MOTION_BLUR
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Motion Blur", model, api, flag).New(src_path+"Motion Blur.cpp");
   //src.New("Explosion", "Explosion_VS", "Explosion_PS");

   src.New("SetVel", "SetVel_VS", "SetVel_PS");

   ASSERT(6==Elms(Mtn.Convert));
   REPD(range, 6)
   {
      REPD(view_full, 2)src.       New("Convert", "DrawUV_VS", "Convert_PS")("VIEW_FULL", view_full, "RANGE", 1<<range);
                      //src.computeNew("Convert",              "Convert_CS")(                        "RANGE", 1<<range);
   }

   const Int dilate_ranges[]={1, 2, 3, 4, 6, 8, 10, 12, 16, 20}; ASSERT(Elms(dilate_ranges)==Elms(Mtn.Dilates)); // #MotionBlurDilateRanges
   REPAD(range, dilate_ranges)
      src.New("Dilate", "DrawUV_VS", "Dilate_PS")("RANGE", dilate_ranges[range]);

   const Int samples[]={5, 7, 9, 14}; ASSERT(Elms(samples)==Elms(Mtn.Blurs)); // 5-720, 7-1080, 9-1440, 14-2160 #MotionBlurSamples
   REPD (glow     , 3)
   REPD (dither   , 2)
   REPD (jitter   , 2)
   REPD (alpha    , 2)
   REPD (temporal , 2)
   REPD (view_full, 2)
   REPD (gather   , temporal ? 2 : 1) // gather only needed for Temporal
   REPAD(sample   , samples)
      src.New("Blur", "Blur_VS", "Blur_PS")("GLOW", glow, "DITHER", dither, "JITTER", jitter, "ALPHA", alpha)("TEMPORAL", temporal, "VIEW_FULL", view_full, "GATHER", gather)("SAMPLES", samples[sample]).gatherChannel(gather);
}
#endif

#ifdef OVERLAY
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Mesh Overlay", model, api, flag).New(src_path+"Mesh Overlay.cpp");
   REPD(skin  , 2)
   REPD(normal, 2)
   for(Int layout=1; layout<=2; layout++)
      src.New()("SKIN", skin, "NORMALS", normal, "LAYOUT", layout);
}
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Overlay", model, api, flag).New(src_path+"Overlay.cpp");
   REPD(tesselate, tess ? 2 : 1)
   REPD(skin     , 2           )src.New()("SKIN", skin).tesselate(tesselate);
}
#endif

#ifdef POSITION
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Position", model, api, flag).New(src_path+"Position.cpp");
   REPD(color, 2)
   {
      REPD(tesselate , tess ? 2 : 1)
      REPD(skin      , 2)
      REPD(alpha_test, ALPHA_TEST_NUM)
      REPD(test_blend, alpha_test ? 2 : 1)
         src.New().position(skin, color, alpha_test, test_blend, FX_NONE, tesselate);

      // grass + leafs
      REPD (test_blend, 2)
      REPAD(fx        , fxs)
         src.New().position(0, color, true, test_blend, fxs[fx], 0);
   }
}
#endif

#ifdef SET_COLOR
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Set Color", model, api, flag).New(src_path+"Set Color.cpp");
   REPD(tesselate , tess ? 2 : 1)
   REPD(skin      , 2)
   REPD(color     , 2)
   REPD(alpha_test, ALPHA_TEST_NUM)
      src.New()("SKIN", skin, "COLORS", color, "ALPHA_TEST", alpha_test).tesselate(tesselate);
}
#endif

#ifdef TEMPORAL
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Temporal", model, api, flag).New(src_path+"Temporal.cpp");
   REPD(view_full, 2)
   {
      REPD(alpha, 2)
      {
         REPD(mode  , 3)
         REPD(gather, 2)
            src.New("Temporal", "DrawUV_VS", "Temporal_PS")("MODE", mode, "VIEW_FULL", view_full, "ALPHA", alpha, "GATHER", gather).gatherChannel(gather);

       /*REPD(anti_alias, 2)
         REPD(gamma     , 2)
            src.computeNew("Temporal", "Temporal_CS")("ANTI_ALIAS", anti_alias, "VIEW_FULL", view_full, "ALPHA", alpha, "GAMMA", gamma).extra("COMPUTE", 1, "SUPER_RES", 1, "GATHER", 1); // compute only used for super_res, gather is always supported when using compute*/
      }
      if(TEMPORAL_SEPARATE_SUPER_RES_OLD_WEIGHT)
      REPD(gather, 2)
         src.New("TemporalOldWeight", "DrawUV_VS", "OldWeight_PS")("VIEW_FULL", view_full, "GATHER", gather).extra("ANTI_ALIAS", 0, "SUPER_RES", 0).gatherChannel(gather); // ANTI_ALIAS=0 is ignored, need to set SUPER_RES=0 because in this shader we're drawing to small sized RT and it affects usage of RTSize/ImgSize
   }
}
#endif

#ifdef VOLUMETRIC_CLOUDS
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Volumetric Clouds", model, api, flag).New(src_path+"Volumetric Clouds.cpp");
                 src.New("Clouds"    , "Clouds_VS"    , "Clouds_PS"    );
                 src.New("CloudsMap" , "CloudsMap_VS" , "CloudsMap_PS" );
   REPD(gamma, 2)src.New("CloudsDraw", "CloudsDraw_VS", "CloudsDraw_PS")("GAMMA", gamma);
}
#endif

#ifdef VOLUMETRIC_LIGHTS
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Volumetric Lights", model, api, flag).New(src_path+"Volumetric Lights.cpp");
   REPD(num  , 6)
   REPD(cloud, 2)
      src.New("VolDir", "DrawUVPosXY_VS", "VolDir_PS")("NUM", num+1, "CLOUD", cloud);

   src.New("VolPoint" , "DrawUVPosXY_VS", "VolPoint_PS" );
   src.New("VolLinear", "DrawUVPosXY_VS", "VolLinear_PS");
   src.New("VolCone"  , "DrawUVPosXY_VS", "VolCone_PS"  );

   REPD(add, 2)src.New("Volumetric", "DrawUV_VS", "Volumetric_PS")("ADD", add);
}
#endif

#ifdef WATER
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Water", model, api, flag).New(src_path+"Water.cpp");
   src.New("Lake" , "Surface_VS", "Surface_PS")("LIGHT", 0, "SHADOW", 0, "SOFT", 0)("REFLECT_ENV", 0, "REFLECT_MIRROR", 0, "REFRACT", 0).extra("WAVES", 0, "RIVER", 0);
   src.New("River", "Surface_VS", "Surface_PS")("LIGHT", 0, "SHADOW", 0, "SOFT", 0)("REFLECT_ENV", 0, "REFLECT_MIRROR", 0, "REFRACT", 0).extra("WAVES", 0, "RIVER", 1);
   src.New("Ocean", "Surface_VS", "Surface_PS")("LIGHT", 0, "SHADOW", 0, "SOFT", 0)("REFLECT_ENV", 0, "REFLECT_MIRROR", 0, "REFRACT", 0).extra("WAVES", 1, "RIVER", 0);
   REPD(refract, 2)
   {
      REPD(reflect_env   , 2)
      REPD(reflect_mirror, 2)
      REPD(gather        , 2)
      {
         REPD(shadow, 7)
         REPD(soft  , 2)
         {
            src.New("Lake" , "Surface_VS", "Surface_PS")("LIGHT", 1, "SHADOW", shadow, "SOFT", soft)("REFLECT_ENV", reflect_env, "REFLECT_MIRROR", reflect_mirror, "REFRACT", refract, "GATHER", gather).extra("WAVES", 0, "RIVER", 0).gather(gather);
            src.New("River", "Surface_VS", "Surface_PS")("LIGHT", 1, "SHADOW", shadow, "SOFT", soft)("REFLECT_ENV", reflect_env, "REFLECT_MIRROR", reflect_mirror, "REFRACT", refract, "GATHER", gather).extra("WAVES", 0, "RIVER", 1).gather(gather);
            src.New("Ocean", "Surface_VS", "Surface_PS")("LIGHT", 1, "SHADOW", shadow, "SOFT", soft)("REFLECT_ENV", reflect_env, "REFLECT_MIRROR", reflect_mirror, "REFRACT", refract, "GATHER", gather).extra("WAVES", 1, "RIVER", 0).gather(gather);
         }
         REPD(set_depth, 2)
            src.New("Apply", "DrawUVPosXY_VS", "Apply_PS")("SET_DEPTH", set_depth)("REFLECT_ENV", reflect_env, "REFLECT_MIRROR", reflect_mirror, "REFRACT", refract, "GATHER", gather).gather(gather);
      }
   }
   src.New("Under", "DrawPosXY_VS", "Under_PS");
}
#endif

#ifdef WORLD_EDITOR
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"World Editor", model, api, flag).New(src_path+"World Editor.cpp");
   src.New("WhiteVtx", "Color_VS", "Color_PS").extra("COL_VALUE", "1, 1, 1", "VTX_COL", 1);
   src.New("White"   , "Color_VS", "Color_PS").extra("COL_VALUE", "1, 1, 1");
   src.New("Green"   , "Color_VS", "Color_PS").extra("COL_VALUE", "0, 1, 0");
   src.New("Yellow"  , "Color_VS", "Color_PS").extra("COL_VALUE", "1, 1, 0");
   src.New("Red"     , "Color_VS", "Color_PS").extra("COL_VALUE", "1, 0, 0");

   src.New("Circle" , "FX_VS", "Circle_PS");
   src.New("Square" , "FX_VS", "Square_PS");
   src.New("Grid"   , "FX_VS",   "Grid_PS");
}
#endif

#ifdef DX10_INPUT_LAYOUT
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(S, model, api, flag).New(src_path+"DX10+ Input Layout.cpp");
   src.New();
}
#endif

#ifdef DEFERRED
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Deferred", model, api, flag).New(src_path+"Deferred.cpp");
   REPD(color, 2)
   {
      // zero (no vtx normals)
      REPD(skin, 2)
         src.New().deferred(skin, 1, 0, SBUMP_ZERO, false, false, false, color, false, false, FX_NONE, false);

      // default
      REPD(skin, 2)
      for(Int materials=1, materials_max=(skin ? 1 : MAX_MTRLS); materials<=materials_max; materials++)
      for(Int layout=((materials==1) ? 0 : 1); layout<=2; layout++) // multi-materials don't support 0 textures
      for(Int bump_mode=SBUMP_FLAT, bump_max=((layout>=2) ? SBUMP_RELIEF : SBUMP_NORMAL); bump_mode<=bump_max; bump_mode++)if(bump_mode<=SBUMP_NORMAL || bump_mode>=SBUMP_PARALLAX_MIN)
      REPD(mtrl_blend, (materials> 1 && layout>=2           ) ? 2 : 1) // can do per-pixel mtrl-blend only if we have bump map
      REPD(heightmap , (!skin                               ) ? 2 : 1)
      REPD(alpha_test, (materials==1 && layout && !heightmap) ? ALPHA_TEST_NUM : 1)
      REPD(macro     , (!skin  &&                  heightmap) ? 2 : 1)
      REPD(tesselate , tess ? 2 : 1)
      REPD(detail    , 2)
         src.New().deferred(skin, materials, layout, bump_mode, alpha_test, detail, macro, color, mtrl_blend, heightmap, FX_NONE, tesselate);

      // grass + leaf, 1 material, 1-2 tex
      for(Int layout=1; layout<=2; layout++)
      REPD (bump_mode, 2)
      REPAD(fx       , fxs)
         src.New().deferred(false, 1, layout, bump_mode ? SBUMP_NORMAL : SBUMP_FLAT, true, false, false, color, false, false, fxs[fx], false);

      // clear coat
      REPD(skin, 2)
      for(Int layout=0; layout<=2; layout++)
      for(Int bump_mode=SBUMP_FLAT; bump_mode<=SBUMP_NORMAL; bump_mode++)
      REPD(tesselate, tess ? 2 : 1)
      REPD(detail   , 1)
         src.New().deferred(skin, 1, layout, bump_mode, false, detail, false, color, false, false, FX_CLEAR_COAT, tesselate);
   }
}
#endif

#ifdef BLEND_LIGHT
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Blend Light", model, api, flag).New(src_path+"Blend Light.cpp");
   REPD(per_pixel  , 2)
   REPD(shadow_maps, 7) // 7=(6+off), 0=off
   REPD(color      , 2)
   REPD(reflect    , 2)
   {
      // base
      REPD(skin        , 2)
      REPD(layout      , 3)
      REPD(bump_mode   , per_pixel ? 2 : 1)
      REPD(alpha_test  , layout    ? 2 : 1)
      REPD(alpha       , layout    ? 2 : 1) // BlendLight doesn't support ALPHA_TEST_DITHER
      REPD(emissive_map, 2)
         src.New().blendLight(skin, color, layout, bump_mode ? SBUMP_NORMAL : SBUMP_FLAT, alpha_test, alpha, reflect, emissive_map, FX_NONE, per_pixel, shadow_maps);

      // grass+leaf, 1 material, 1-2 tex
      for(Int layout=1; layout<=2; layout++)
      REPD (bump_mode   , per_pixel ? 2 : 1)
      REPD (alpha_test  , layout    ? 2 : 1) // BlendLight doesn't support ALPHA_TEST_DITHER, here 'alpha_test' options are needed because of MTECH_BLEND_LIGHT_GRASS/MTECH_TEST_BLEND_LIGHT_GRASS etc.
      REPD (emissive_map, 2)
      REPAD(fx          , fxs)
         src.New().blendLight(false, color, layout, bump_mode ? SBUMP_NORMAL : SBUMP_FLAT, alpha_test, true, reflect, emissive_map, fxs[fx], per_pixel, shadow_maps);
   }
}
#endif

#ifdef FORWARD
{
   ShaderCompiler::Source &src=ShaderCompilers.New().set(dest_path+"Forward", model, api, flag).New(src_path+"Forward.cpp");
   REPD(color, 2)
   {
      // zero (no vtx normals)
      REPD(skin, 2)
         src.forward(skin, 1, 0, SBUMP_ZERO, false, false, false, false, color, false, false, FX_NONE, false,   false,false,0,   false,false,   false,false,   false,false,   false);

      // default
      REPD(skin        , 2)
      REPD(per_pixel   , 2)
      for(Int materials=1, materials_max=(skin ? 1 : MAX_MTRLS); materials<=materials_max; materials++)
      for(Int layout=((materials==1) ? 0 : 1); layout<=2; layout++) // multi-materials don't support 0 textures
      REPD(bump_mode   , per_pixel                              ? 2 : 1)
      REPD(mtrl_blend  , (materials> 1 && layout>=2           ) ? 2 : 1) // can do per-pixel mtrl-blend only if we have bump map
      REPD(heightmap   , (!skin                               ) ? 2 : 1)
      REPD(alpha_test  , (materials==1 && layout && !heightmap) ? ALPHA_TEST_NUM : 1)
      REPD(emissive_map, (materials==1 &&           !heightmap) ? 2 : 1)
      REPD(tesselate   , (tess && SUPPORT_FORWARD_TESSELATE   ) ? 2 : 1)
      REPD(detail      , (        SUPPORT_FORWARD_DETAIL      ) ? 2 : 1)
      REPD(reflect     , 2)
         src.forwardLight(skin, materials, layout, bump_mode ? SBUMP_NORMAL : SBUMP_FLAT, alpha_test, reflect, emissive_map, detail, color, mtrl_blend, heightmap, FX_NONE, per_pixel, tesselate);

      // grass + leaf, 1 material, 1-2 tex
      for(Int layout=1; layout<=2; layout++)
      REPD (per_pixel   , 2)
      REPD (bump_mode   , per_pixel ? 2 : 1)
      REPD (reflect     , 2)
      REPD (emissive_map, 2)
      REPAD(fx          , fxs)
         src.forwardLight(false, 1, layout, bump_mode ? SBUMP_NORMAL : SBUMP_FLAT, true, reflect, emissive_map, false, color, false, false, fxs[fx], per_pixel, false);
   }
}
#endif
}
/******************************************************************************/
#endif // COMPILE
/******************************************************************************/
static void SetShaderHash()
{
   Memc<Str> files; for(FileFind ff(DataPath()+"Shader/GL"); ff(); )files.add(ff.pathName());
   DYNAMIC_ASSERT(files.elms(), "No Shader Files");
   files.sort(ComparePathNumberCI);
   xxHash64 hash;
   Memt<Byte> temp;
   File f;
   FREPA(files)
   {
    C Str &file=files[i];
      hash.update(GetBase(file));
      f.mustReadStd(file);
      temp.setNumDiscard(f.size());
      if(!f.getFast(temp.data(), temp.elms()))Exit("Can't read data");
      hash.update(temp.data(), temp.elms());
   }
   Str header=GetPath(__FILE__)+"/Shader Hash.h";
   FileText ft;
   ft.write(header, ANSI);
   ft.putLine(S+"#define SHADER_HASH "+hash()+"ull");
}
void MainShaderClass::compile()
{
#if COMPILE_DX || COMPILE_GL || COMPILE_GL_SPIRV
   App.stayAwake(AWAKE_SYSTEM);

#if COMPILE_DX
   Compile(API_DX);
#endif
#if COMPILE_GL
   Compile(API_GL);
#endif
#if COMPILE_GL_SPIRV
   Compile(API_GL, SC_SPIRV);
#endif

   if(ShaderCompilers.elms())
   {
      App.show(false); // show in case it's hidden
      ProcPriority(-2); // compiling shaders may slow down entire CPU, so make this process have smaller priority
      Threads threads; threads.create(false, Cpu.threads()-1, 0, "EE.ShaderCompiler #");
      FREPAO(ShaderCompilers).compile(threads);
      ProcPriority(0);
   }

#if COMPILE_GL
   SetShaderHash();
#endif

   App.stayAwake(AWAKE_OFF);
#endif
}
/******************************************************************************/
}
/******************************************************************************/
