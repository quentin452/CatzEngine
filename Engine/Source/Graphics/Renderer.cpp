/******************************************************************************

   Graphics API differences:
      Reading and Writing to the same Render Target - Yes: GL         No: DX10+
      Deferred Multi-Sampling                       - Yes: DX>=10.1   No: DX10.0, GL

/******************************************************************************/
#include "stdafx.h"

#define A_CPU 1
#include "../Shaders/FidelityFX/ffx_a.h"
#include "../Shaders/FidelityFX/ffx_fsr1.h"
//#include "../Shaders/FidelityFX/ffx_cas.h"

namespace EE{
// #RTOutput
#define SVEL_CLEAR Vec4Zero
#define  VEL_CLEAR Vec4(0.5f, 0.5f, 0.5f, 0)
#define SNRM_CLEAR Vec4(0   , 0   ,   -1, 0) // normally Z should be set to 0   to set 'VecZero' normals, however Z is set to -1 makes ambient occlusion more precise when it uses normals (because ambient occlusion will not work good on the VecZero normals) #NRM_CLEAR
#define  NRM_CLEAR Vec4(0.5f, 0.5f,    0, 0) // normally Z should be set to 0.5 to set 'VecZero' normals, however Z is set to  0 makes ambient occlusion more precise when it uses normals (because ambient occlusion will not work good on the VecZero normals) #NRM_CLEAR
#define  EXT_CLEAR Vec4(1   , 0   ,    0, 0) // rough, reflect
#define  NRM_CLEAR_START 1 // 1 works faster on GeForce 650m GT, TODO: check on newer hardware
#define  EXT_CLEAR_START 0 // TODO: which is better?
#define  VEL_CLEAR_START 0 // this is not needed because "ClearDeferred" is used later, performance tests suggested it's better don't clear unless necessary, instead 'Image.discard' is used and improves performance (at least on Mobile), TODO: check on newer hardware
// if there's no sky but we use an environment map, currently PBR formulas can generate specular/reflection even if smooth and reflectivity are zero, so for that case we also need to clear normals as it depends on eye_dir, view_nrm, smooth and reflectivity #SpecularReflectionFromZeroSmoothReflectivity
static inline Bool NeedBackgroundNrm() {return (!Sky.isActual() && D.envMap()) || D.aoWant() && D.ambientNormal() || Renderer.stage==RS_NORMAL;}
static inline Bool NeedBackgroundExt() {return (!Sky.isActual() && D.envMap()) || Renderer.stage==RS_SMOOTH || Renderer.stage==RS_REFLECT || Renderer.stage==RS_LIT_COLOR;}
/******************************************************************************/
static const  Byte  TemporalSuperResIndex[]={0, 3, 1, 2};
static inline VecI2 TemporalSuperResIndexToOfsI(Byte i) {return VecI2(i&1, i>>1);}
static inline Vec2  TemporalSuperResIndexToOfs (Byte i) {return Vec2 (-(i&1)+0.5f, (i>>1)-0.5f);}
static const  Vec2  TemporalSuperResOffsets[]=
{
   TemporalSuperResIndexToOfs(TemporalSuperResIndex[0]),
   TemporalSuperResIndexToOfs(TemporalSuperResIndex[1]),
   TemporalSuperResIndexToOfs(TemporalSuperResIndex[2]),
   TemporalSuperResIndexToOfs(TemporalSuperResIndex[3])
};
/******************************************************************************/
static const Vec2 TemporalAntiAliasOffsets[]=
{
#if 1 // 8 samples (good enough quality and smaller flickering)
   #if 1 // Halton (smaller flickering), REPAO(TemporalAntiAliasOffsets).set(Halton(i+1, 2)*2-1, Halton(i+1, 3)*2-1)
      #define TEMPORAL_MUL 1.1f // this makes AA more smooth
      { 0.000f*TEMPORAL_MUL, -0.333333313f*TEMPORAL_MUL},
      {-0.500f*TEMPORAL_MUL,  0.333333373f*TEMPORAL_MUL},
      { 0.500f*TEMPORAL_MUL, -0.777777791f*TEMPORAL_MUL},
      {-0.750f*TEMPORAL_MUL, -0.111111045f*TEMPORAL_MUL},
      { 0.250f*TEMPORAL_MUL,  0.555555582f*TEMPORAL_MUL},
      {-0.250f*TEMPORAL_MUL, -0.555555522f*TEMPORAL_MUL},
      { 0.750f*TEMPORAL_MUL,  0.111111164f*TEMPORAL_MUL},
      {-0.875f*TEMPORAL_MUL,  0.777777910f*TEMPORAL_MUL},
   #else // ARM sorted
      #define TEMPORAL_MUL (1.0f/8.0f)
      {-1*TEMPORAL_MUL, -3*TEMPORAL_MUL},
      { 1*TEMPORAL_MUL,  3*TEMPORAL_MUL},
      { 5*TEMPORAL_MUL, -1*TEMPORAL_MUL},
      {-3*TEMPORAL_MUL,  5*TEMPORAL_MUL},
      {-7*TEMPORAL_MUL,  1*TEMPORAL_MUL},
      {-5*TEMPORAL_MUL, -5*TEMPORAL_MUL},
      { 3*TEMPORAL_MUL, -7*TEMPORAL_MUL},
      { 7*TEMPORAL_MUL,  7*TEMPORAL_MUL},
   #endif
#else // 16 samples ARM (better quality however much bigger flickering)
   #define TEMPORAL_MUL (1.0f/8.0f)
	{-8*TEMPORAL_MUL,  0*TEMPORAL_MUL},
	{-6*TEMPORAL_MUL, -4*TEMPORAL_MUL},
	{-3*TEMPORAL_MUL, -2*TEMPORAL_MUL},
	{-2*TEMPORAL_MUL, -6*TEMPORAL_MUL},
	{ 1*TEMPORAL_MUL, -1*TEMPORAL_MUL},
	{ 2*TEMPORAL_MUL, -5*TEMPORAL_MUL},
	{ 6*TEMPORAL_MUL, -7*TEMPORAL_MUL},
	{ 5*TEMPORAL_MUL, -3*TEMPORAL_MUL},
	{ 4*TEMPORAL_MUL,  1*TEMPORAL_MUL},
	{ 7*TEMPORAL_MUL,  4*TEMPORAL_MUL},
	{ 3*TEMPORAL_MUL,  5*TEMPORAL_MUL},
	{ 0*TEMPORAL_MUL,  7*TEMPORAL_MUL},
	{-1*TEMPORAL_MUL,  3*TEMPORAL_MUL},
	{-4*TEMPORAL_MUL,  6*TEMPORAL_MUL},
	{-7*TEMPORAL_MUL,  8*TEMPORAL_MUL},
   {-5*TEMPORAL_MUL,  2*TEMPORAL_MUL},
#endif
};
#undef TEMPORAL_MUL
static const VecI2 NoiseOffsets[]= // these should be spread out across entire noise image and not close together
{
   {-1*NOISE_IMAGE_RES/16, -3*NOISE_IMAGE_RES/16},
   { 1*NOISE_IMAGE_RES/16,  3*NOISE_IMAGE_RES/16},
   { 5*NOISE_IMAGE_RES/16, -1*NOISE_IMAGE_RES/16},
   {-3*NOISE_IMAGE_RES/16,  5*NOISE_IMAGE_RES/16},
   {-7*NOISE_IMAGE_RES/16,  1*NOISE_IMAGE_RES/16},
   {-5*NOISE_IMAGE_RES/16, -5*NOISE_IMAGE_RES/16},
   { 3*NOISE_IMAGE_RES/16, -7*NOISE_IMAGE_RES/16},
   { 7*NOISE_IMAGE_RES/16,  7*NOISE_IMAGE_RES/16},
};
/******************************************************************************/
RendererClass Renderer;
MeshRender    MshrBox ,
              MshrBoxR,
              MshrBall;
/******************************************************************************/
static void InitMshr()
{
   MeshBase mshb;

   mshb.createFast(Box(1));
                   MshrBox .create(mshb);
   mshb.reverse(); MshrBoxR.create(mshb);

   mshb.create(Ball(1), MESH_NONE, 12);
   MshrBall.create(mshb);
}
static void ShutMshr()
{
   MshrBox .del();
   MshrBoxR.del();
   MshrBall.del();
}
/******************************************************************************/
// FIDELITY FX
/******************************************************************************/
struct EASU
{
   AU1 c0[4], c1[4], c2[4], c3[4];
};
static inline void SetEASU(C Image &src, C ImageRT &dest)
{
   EASU easu;
   FsrEasuCon(easu.c0, easu.c1, easu.c2, easu.c3,
   src .  w(), src .  h(),  // Viewport size (top left aligned) in the input image which is to be scaled.
   src .hwW(), src .hwH(),  // The size of the input image.
   dest.  w(), dest.  h()); // The output resolution.
            
#if GL
   if(Renderer.mainFBO(&dest)) // flip Y
   {
      Vec2 &mul=(Vec2&)easu.c0[0];
      Vec2 &add=(Vec2&)easu.c0[2];
    //AF2 pp=AF2(ip)*AF2_AU2(con0.xy)+AF2_AU2(con0.zw);
    //pixel*mul+add
    //(dest.h()-1-pixel.y)*mul1+add1=pixel*mul+add
      add.y+=(dest.h()-1)*mul.y; mul.chsY();
   }
#endif

   Sh.Easu->set(easu);
}
void SetEASU(C Image &src, C Vec2 &dest_size, C Vec2 &screen_lu)
{
   EASU easu;
   FsrEasuCon(easu.c0, easu.c1, easu.c2, easu.c3,
   src.  w(),   src.  h(),  // Viewport size (top left aligned) in the input image which is to be scaled.
   src.hwW(),   src.hwH(),  // The size of the input image.
 dest_size.x, dest_size.y); // The output resolution.

   // apply offset
   Vec2 pixel_pos=Renderer.screenToPixel(screen_lu);
   Vec2 &mul=(Vec2&)easu.c0[0];
   Vec2 &add=(Vec2&)easu.c0[2];
   add-=pixel_pos*mul;

#if GL
   if(D.mainFBO()) // flip Y
   {
      add.y+=(Renderer.resH()-1)*mul.y; mul.chsY();
   }
#endif

   Sh.Easu->set(easu);
}
DisplayClass& DisplayClass::sharpenIntensity(Flt intensity)
{
   SAT(intensity);
   if(_sharpen_intensity!=intensity)
   {
     _sharpen_intensity=intensity;
      struct RCAS
      {
         AU1 c0[4];
      }rcas;
      FsrRcasCon(rcas.c0, 1-intensity);
      Sh.Rcas->set(rcas);
   }
   return T;
}
#if GL
static ShaderParam *RcasMulAdd;
#endif
/******************************************************************************/
RendererClass::Context::Sub::Sub()
{
 //used=true; not needed since we always set 'used=true' when accessing/creating new subs
   proj_matrix_prev=ProjMatrix; // copy from current (needed for MotionBlur and Temporal)
}
/******************************************************************************/
RendererClass::RendererClass() : material_color_l(null), highlight(null)
{
#if 0 // there's only one 'RendererClass' global 'Renderer' and it doesn't need clearing members to zero
   stage=RS_DEFAULT;
   combine=false;
   alpha=false;
   wire=false;
   indoor=false;
   clear_color.zero();
   ms_samples_color.zero();
   target=null;
   temporal_id=null;

  _mode=RM_OPAQUE;
  _mesh_blend_alpha=ALPHA_NONE;
  _has_glow=_mirror=_mirror_want=_mirror_shadows=_palette_mode=_eye_adapt_scale_cur=_t_measure=_set_depth_needed=_get_target=_stereo=_mesh_early_z=_mesh_shader_vel=_temporal_use=_temporal_reset=false;
  _has=0;
  _outline=_clear=0;
  _mirror_priority=_mirror_resolution=0;
  _frst_light_offset=_blst_light_offset=0;
  _res.zero();
  _pixel_size.zero(); _pixel_size_inv.zero();
  _temporal_offset.zero();
  _clip.zero();
  _mirror_plane.zero();
  _shader_early_z=_shader_shd_map=_shader_shd_map_skin=null;
  _render=null;
  _shader_param_changes=null;
   REPAO(_t_measures)=0;
  _t_last_measure=0;
   REPAO(_t_reflection)=0; REPAO(_t_prepare)=0; REPAO(_t_opaque)=0; REPAO(_t_overlay)=0; REPAO(_t_water)=0; REPAO(_t_light)=0; REPAO(_t_emissive)=0; REPAO(_t_sky)=0; REPAO(_t_water_under)=0; REPAO(_t_edge_detect)=0; REPAO(_t_blend)=0; REPAO(_t_palette)=0; REPAO(_t_behind)=0; REPAO(_t_rays)=0; REPAO(_t_volumetric)=0; REPAO(_t_post_process)=0; REPAO(_t_gpu_wait)=0;
  _ctx=null;
#endif

  _opaque_mode_index=RM_OPAQUE;

   allow_temporal=true;
  _ctx_sub=&_ctx_sub_dummy;

   lowest_visible_point=-DBL_MAX;

  _first_pass=true;
  _eye=0; _eye_num=1;

  _type=_cur_type=RT_DEFERRED;
  _forward_prec=!MOBILE;

  _mesh_blend_alpha  =ALPHA_RENDER_BLEND_FACTOR;
  _mesh_stencil_value=STENCIL_REF_ZERO;
  _mesh_stencil_mode =STENCIL_NONE;
  _mesh_highlight    .zero();
  _mesh_draw_mask    =0xFFFFFFFF;
   SetVariation(0);

  _ui   =_cur_main   =_ptr_main   =&_main;
  _ui_ds=_cur_main_ds=_ptr_main_ds=&_main_ds;
}
void RendererClass::del()
{
  _mesh_shader_vel=false; // prevent setting velocities (which may ignore range checks) after deleting, because shader handles may be already released

   Sky   .del();
   Astros.del();
   Clouds.del();
   Water .del();

   ShutInstances();
   ShutMshr     ();
   ShutParticles();
   ShutMesh     ();
   ShutMaterial ();
   ShutLight    ();

   rtDel();
}
void RendererClass::create()
{
   if(LogInit)LogN("RendererClass.create");

#if 0 // convert DDS from SMAA source to Engine Image format
   Str in="D:/!/SMAA/", out="C:/Esenthel/Data/Img/";
   Image img;
   img.mustImport(in+"SearchTex.dds", -1, IMAGE_2D); img._type=IMAGE_R8;
   img.save(out+"SMAA Search.img");
   img.mustImport(in+"AreaTexDX10.dds", IMAGE_R8G8, IMAGE_2D);
   img.save(out+"SMAA Area.img");
#endif

#if SUPPORT_EARLY_Z
  _shader_early_z     =DefaultShaders(&MaterialDefault, VTX_POS         , 0, false).EarlyZ();
#endif
  _shader_shd_map     =DefaultShaders(&MaterialDefault, VTX_POS         , 0, false).Shadow();
  _shader_shd_map_skin=DefaultShaders(&MaterialDefault, VTX_POS|VTX_SKIN, 0, false).Shadow();

   InitCamera   ();
   InitLight    ();
   InitMaterial ();
   InitFur      ();
   InitMshr     ();
   InitInstances();

   Sky   .create();
   Clouds.create();
   Water .create();

   if(_env_dfg.load("Img/Environment DFG.img"))GetShaderImage("EnvDFG"  )->set(_env_dfg);
   if(_noise  .load("Img/Blue Noise 128.img" ))GetShaderImage("ImgNoise")->set(_noise  ); ASSERT(NOISE_IMAGE_RES==128);
#if GL
   RcasMulAdd=GetShaderParam("RcasMulAdd");
#endif
}
RendererClass& RendererClass::type(RENDER_TYPE type)
{
	Clamp(type, RENDER_TYPE(0), RENDER_TYPE(RT_NUM-1));
   if(type==RT_DEFERRED && D.deferredUnavailable())return T;
   if(T._type!=type)
   {
      T._type=T._cur_type=type;
      if(type==RT_DEFERRED && D.deferredMSUnavailable())D.samples(1); // disable multi-sampling if we can't support it
      D.densityUpdate(); // might affect 'temporalSuperRes' and thus rendering resolution
      D.setShader(); // needed because shaders are set only for current renderer type
   }
   return T;
}
void RendererClass::mode(RENDER_MODE mode)
{
   T._mode             =mode;
   T._palette_mode     =(mode==RM_PALETTE || mode==RM_PALETTE1);
   T._mesh_shader_vel  =(_vel && (mode==RM_OPAQUE || mode==RM_OPAQUE_M || mode==RM_BLEND)); // only these modes use velocity
   T._opaque_mode_index=(mirror() ? RM_OPAQUE_M : RM_OPAQUE);
   D.setFrontFace();
   MaterialClear(); // must be called when changing rendering modes, because when setting materials, we may set only some of their shader values depending on mode
}
RendererClass& RendererClass::forwardPrecision(Bool per_pixel)
{
   if(T._forward_prec!=per_pixel)
   {
      T._forward_prec=per_pixel;
      D.setShader();
   }
   return T;
}
/******************************************************************************/
void RendererClass::requestMirror(C PlaneM &plane, Int priority, Bool shadows, Int resolution)
{
   if(!_mirror_want || priority>T._mirror_priority)
   {
     _mirror_want      =true;
     _mirror_priority  =priority;
     _mirror_plane     =plane;
     _mirror_shadows   =shadows;
     _mirror_resolution=resolution;
   }
}
/******************************************************************************/
void RendererClass::linearizeDepth(ImageRT &dest, ImageRT &depth)
{
   D.alpha(ALPHA_NONE);
   set(&dest, null, true);
   if(!depth.multiSample() || depth.size()!=dest.size()){Sh.imgSize(depth); Sh.Depth  ->set(depth); Sh.LinearizeDepth[0][FovPerspective(D.viewFovMode())]->draw(); Sh.Depth  ->set(_ds_1s);}else // 1s->1s, set and restore depth, if we're resizing then we also need to use the simple version, 'imgSize' needed for 'DownSamplePointUV'
   if(!dest .multiSample()                             ){                   Sh.DepthMS->set(depth); Sh.LinearizeDepth[1][FovPerspective(D.viewFovMode())]->draw(); Sh.DepthMS->set(_ds   );}else // ms->1s, set and restore depth
                                                        {                   Sh.DepthMS->set(depth); Sh.LinearizeDepth[2][FovPerspective(D.viewFovMode())]->draw(); Sh.DepthMS->set(_ds   );}     // ms->ms, set and restore depth
}
void RendererClass::setDepthForDebugDrawing()
{
   if(_set_depth_needed)
   {
     _set_depth_needed=false;
      if(_ds_1s)if(Shader *shader=Sh.SetDepth)
      {
         ImageRT *rt=_cur[0]; set(null, _cur_ds, true);
         ALPHA_MODE alpha=D.alpha(ALPHA_NONE); D.depthLock  (true); D.depthFunc(FUNC_ALWAYS ); shader->draw();
                          D.alpha(alpha     ); D.depthUnlock(    ); D.depthFunc(FUNC_DEFAULT);
         set(rt, _cur_ds, true);
      }
   }
}
ImageRTPtr RendererClass::getBackBuffer() // this may get called during rendering and outside of it
{
   if(Image *src=_cur[0])
   {
      ImageRTPtr hlp(ImageRTDesc(src->w(), src->h(), GetImageRTType(src->type())));
      src->copyHw(*hlp, true);
      return hlp;
   }
   return null;
}
/******************************************************************************/
ImageRT* RendererClass::adaptEye(ImageRTC &src, ImageRT *dest)
{
   Hdr.load();
   D.alpha(ALPHA_NONE);
   ImageRTPtr cur=&src;
   VecI2    size=RoundPos(fx()*(D.viewRect().size()/D.size2())); // calculate viewport size in pixels
   Int  max_size=size.min()/4;
   Int  s=1, num=1; for(;;){Int next_size=s*4; if(next_size>max_size)break; s=next_size; num++;} // go from 1 up to 'max_size', increase *4 in each step
   FREP(num) // now go backward, from up to 'max_size' to 1 inclusive
   {
      ImageRTPtr next=cur; next.get(ImageRTDesc(s, s, IMAGERT_F32)); s/=4; // we could use 16-bit as according to calculations, the max error for 1920x1080, starting with 256x256 as first step and going down to 1x1, with average luminance of 1.0 (255 byte) is 0.00244140625 at the final stage, which gives 410 possible colors, however we may use some special tricks in the shader that requires higher precision (for example BRIGHT with Sqr and Sqrt later, or use Linear/sRGB)
      set(next, null, false);
      Sh.imgSize(*cur);
      if(i){Sh.ImgXF[0]->set(cur); Hdr.HdrDS[1]->draw();}
      else {Sh.Img  [0]->set(cur); Hdr.HdrDS[0]->draw(null, D.screenToUV(D.viewRect()));}
      cur=next;
   }
   Sh.Step->set(Pow(Mid(1/D.eyeAdaptationSpeed(), EPS, 1.0f), Time.d())); // can use EPS and not EPS_GPU because we're using Pow here and not on GPU
   Sh.ImgXF[0]->set(cur); Sh.ImgXF[1]->set(_eye_adapt_scale[_eye_adapt_scale_cur]); _eye_adapt_scale_cur^=1; _eye_adapt_scale[_eye_adapt_scale_cur].discard(); set(&_eye_adapt_scale[_eye_adapt_scale_cur], null, false); Hdr.HdrUpdate->draw();
   ImageRT &mul=_eye_adapt_scale[_eye_adapt_scale_cur];
   if(dest)
   {  // even though in the shader we just multiply by luminance, in tests it was faster to always write to new RT rather than do ALPHA_MUL and write to existing RT
      Sh.ImgX[0]->set(mul); set(dest, null, true); Hdr.AdaptEye[D.dither() /*&& src.highPrecision()*/ && !dest->highPrecision()]->draw(src);
      return null;
   }
   return &mul;
}
/******************************************************************************/
Bool RendererClass::dilateMotion(ImageRTPtr *dilated_full_motion, ImageRTPtr *dilated_blur_motion)
{
   if(dilated_full_motion || dilated_blur_motion)
   {
      Mtn.load();
      D.alpha(ALPHA_NONE);

      VecI2 fx=T.fx();
      if(!_vel) // MOTION_CAMERA, however can already be set for Temporal
      {
        _vel.get(ImageRTDesc(fx.x, fx.y, IMAGERT_TWO_H));
         set(_vel, null, true);
         SetViewToViewPrev();
         Mtn.SetVel->draw();
      }

    C VecI2      &convert_src=(1 ? fx : _vel->size()); // 1 is faster but slightly lower quality
      VecI2       res=ByteScaleRes(fx, D._mtn_res);
      Flt         scale=Flt(convert_src.y)/res.y;
      Int         shift=Log2Round(RoundU(scale)); // Int shift=Round(Log2(scale)); 0=full-res, 1=half, 2=quarter, 3=1/8, 4=1/16, 5=1/32 (this value is unclamped)
      ImageRTDesc rt_desc(res.x, res.y, IMAGERT_RGBA_H); // XY=biggest motion, ZW=smallest motion
      Vec2        pixel_size;
      Int         dilate_pixels=DivCeil((UInt)rt_desc.size.y, (UInt)5*2); // dilate up to 20% of the screen vertically (this is the max amount of blur length, *2 because we blur both ways) #MaxMotionBlurLength
    C auto       *dilate=&Mtn.getDilate(dilate_pixels);
      Shader     *shader= Mtn.getConvert(shift); // 'shift' is clamped in this function
      Rect        rect, *rect_ptr=null;

      ImageRTPtr dilated_motion(rt_desc);
      set(dilated_motion, null, false);
      Sh.ImgXY[0]->set(_vel);
      Sh.imgSize(convert_src); // need to have full-res size

      /*ComputeShader *cs=Mtn.shader->computeFind(S8+"Convert"+(1<<shift)); 'shift' is unclamped
      if(cs && dilated_motion->hasUAV() && Kb.ctrl())
      {
         need to set viewport min+size
         Sh.RWImg->set(dilated_motion);
         cs->compute(rt_desc.size);
      }else*/
      if(D._view_main.full)REPS(_eye, _eye_num)
      {
         Rect *eye_rect=setEyeParams();
         shader->draw(eye_rect);
      }else // when not rendering entire viewport
      {
         pixel_size=pixelToScreenSize(1); // call this after setting new RT to get 'dilated_motion' size, have to use 'Renderer.pixelToScreenSize' and not 'D.pixelToScreenSize'
          rect_ptr =&(rect=D.viewRect()).extend(pixel_size); // extend by 1 pixel because this will be used in full-res blur with texture filtering

         Sh.ImgClamp->setConditional(ImgClamp(D.viewRect(), _vel->size()));
                      rect_ptr->drawBorder(Vec4Zero, pixel_size*dilate->range); // draw black border around the viewport to clear and prevent from potential artifacts on viewport edges
         shader->draw(rect_ptr); // convert after drawing border
      }

      if(stage==RS_MOTION_CONVERTED && show(dilated_motion, false, true))return true;

      // Dilate
      if(DUAL_DILATE_MOTION || dilate_pixels>0) // always have to dilate for DUAL_DILATE_MOTION in order to scale by 'MotionScale_2' and clamp max length
      {  // we need to apply Dilation, for example, if a ball object has movement, then it should be blurred, and also pixels around the ball should be blurred too
         // however velocity for pixels around the ball (the background) may be zero, so we need to apply dilation and apply the velocity onto neighboring pixels

       //Sh.imgSize(*dilated_motion); we can just use 'RTSize' instead of 'ImgSize' since there's no scale
      #if DUAL_DILATE_MOTION
         if(dilated_full_motion)dilated_full_motion->get(rt_desc);
         if(dilated_blur_motion)dilated_blur_motion->get(rt_desc);
         set(dilated_full_motion ? *dilated_full_motion : null,
             dilated_blur_motion ? *dilated_blur_motion : null, null, null, null, false);
         // DUAL_DILATE_MOTION doesn't support multiple dilation because motions are scaled by 'MotionScale_2' inside dilation and input is taken only from 1 motion RT TODO: this could be worked around if 'MotionScale_2' was set to 1 after 1st dilate and then restored at the end
      #else
         ImageRTPtr next(rt_desc);
      again:
         dilate_pixels-=dilate->range;
       C auto *next_dilate=(!DUAL_DILATE_MOTION && dilate_pixels>0) ? &Mtn.getDilate(dilate_pixels) : null;
         set(next, null, false);
         if(rect_ptr && next_dilate) // if not full viewport, and there's a next dilate
            rect_ptr->drawBorder(Vec4Zero, pixel_size*next_dilate->range); // draw black border around the viewport to clear and prevent from potential artifacts on viewport edges
      #endif
         dilate->Dilate->draw(dilated_motion, rect_ptr); // dilate after drawing border
      #if !DUAL_DILATE_MOTION
         Swap(dilated_motion, next);
         if(next_dilate){next->discard(); dilate=next_dilate; goto again;}
      #endif
      }
   #if !DUAL_DILATE_MOTION
      if(dilated_full_motion)*dilated_full_motion=dilated_motion;
      if(dilated_blur_motion)*dilated_blur_motion=dilated_motion;
   #endif
      if(stage)switch(stage)
      {
         case RS_MOTION_DILATED_FULL: if(dilated_full_motion && show(*dilated_full_motion, false, true))return true; break;
         case RS_MOTION_DILATED_BLUR: if(dilated_blur_motion && show(*dilated_blur_motion, false, true))return true; break;
      }
   }
   return false;
}
/******************************************************************************/
// !! Assumes that 'ImgClamp' was already set !!
void RendererClass::motionBlur(ImageRT &src, ImageRT &dest, ImageRTPtr &bloom_glow, ImageRTPtr &dilated_motion, Bool alpha, Bool combine, ImageRT *exposure)
{
   #if DEBUG && 0 // test UV CLAMP
   {
      set(&src, null, false);
      D.viewRect().drawBorder(Vec4(1,0,0,0), Vec2(1));
   }
   #endif

   if(_has_glow)
   {
      Sh.BloomParams->setConditional(Vec4(D.bloomOriginal(), D.bloomScale(), D.bloomAdd(), D.bloomGlow()));
      bloom_glow.get(ImageRTDesc(dest.w(), dest.h(), IMAGERT_SRGB));
   }
   Sh.Img [1]->set(dilated_motion);
   Sh.ImgX[0]->set(_alpha);
   Sh.ImgX[1]->set(exposure);
   Sh.imgSize(*_ds);
   set(&dest, bloom_glow, null, null, null, true); D.alpha((combine && &dest==_final) ? ALPHA_MERGE : ALPHA_NONE);
   Mtn.getBlur(Round(dest.h()*(7.0f/1080)), _has_glow ? exposure ? 2 : 1 : 0, (D.dither() && !dest.highPrecision()) ? src.highPrecision() ? 1/*always: should be 2 but disabled because rarely used*/ : 1/*only in blur*/ : 0, alpha)->draw(src); // here blurring may generate high precision values, use 7 samples on a 1080 resolution #MotionBlurSamples
}
/******************************************************************************/
INLINE Shader* GetPrecomputedBloomDS(Bool view_full, Bool half_res               ) {Shader* &s=Sh.PrecomputedBloomDS[view_full][half_res]     ; if(SLOW_SHADER_LOAD && !s)s=Sh.getPrecomputedBloomDS(view_full, half_res     ); return s;}
INLINE Shader* GetBloomDS(Bool glow, Bool view_full, Bool half_res, Bool exposure) {Shader* &s=Sh.BloomDS[glow][view_full][half_res][exposure]; if(SLOW_SHADER_LOAD && !s)s=Sh.getBloomDS(glow, view_full, half_res, exposure); return s;}
INLINE Shader* GetBloom  (Int tone_map, Int alpha, Bool dither    , Bool exposure) {Shader* &s=Sh.Bloom[tone_map][alpha][dither]    [exposure]; if(SLOW_SHADER_LOAD && !s)s=Sh.getBloom  (tone_map, alpha, dither  , exposure); return s;}
// !! Assumes that 'ImgClamp' was already set !!
void RendererClass::bloom(ImageRT &src, ImageRT &dest, ImageRTPtr &bloom_glow, Bool alpha, Bool combine, ImageRT *exposure)
{
   // '_alpha' RT from 'processAlpha' can't be used/modified because Bloom works by ADDING blurred results on top of existing background, NOT BLENDING (which is used for applying renderer results onto existing background when combining), we could potentially use a secondary RT to store bloom and add it on top of render, however that uses more memory, slower, and problematic with Motion Blur and DoF
   const Bool    half =true;
   const Int     shift=(half ? 1 : 2);
   ImageRTPtrRef rt0(half ? _h0 : _q0);

   D.alpha(ALPHA_NONE);
   Sh.ImgX[1]->set(exposure); // this is used by both BloomDS and Bloom
   if(D.bloomAllow() && // this has to be checked because bloom is merged with tonemapping, and if bloom is disabled but tone mapping enabled, then we will still run this code
     (_has_glow || D.bloomScale())) // if we have something there
   {
      ImageRTDesc rt_desc(fxW()>>shift, fxH()>>shift, IMAGERT_SRGB); // using IMAGERT_SRGB will clip to 0..1 range !! USING HIGH PRECISION WOULD REQUIRE CLAMPING IN THE SHADER TO MAKE SURE VALUES DON'T GO BELOW 0 !!
                                           rt0.get(rt_desc);
      ImageRTPtrRef rt1(half ? _h1 : _q1); rt1.get(rt_desc); Bool discard=false; // we've already discarded in 'get' so no need to do it again

      set(rt0, null, false);

      const Int blurs=1;
      Rect ext_rect, *rect=null; // set rect, after setting render target
      if(!D._view_main.full){ext_rect=D.viewRect(); rect=&ext_rect.extend(pixelToScreenSize(blurs*SHADER_BLUR_RANGE+1));} // when not rendering entire viewport, then extend the rectangle, add +1 because of texture filtering, have to use 'Renderer.pixelToScreenSize' and not 'D.pixelToScreenSize'

      const Bool half_res=(Flt(src.h())/rt0->h() <= 2.5f); // half_res=scale 2, ..3.., quarter=scale 4, 2.5 was the biggest scale that didn't cause jittering when using half down-sampling
      if(bloom_glow)
      {
         Sh.BloomParams->setConditional(D.bloomOriginal());
         GetPrecomputedBloomDS(D._view_main.full, half_res)->draw(bloom_glow, rect);
      }else
      {
         const Int res=(half_res ? 2 : 4), res2=Sqr(res);
         #define BLOOM_GLOW_GAMMA_PER_PIXEL 0 // #BloomGlowGammaPerPixel
         Sh.BloomParams->setConditional(Vec4(D.bloomOriginal(), _has_glow ? D.bloomScale()/res2
                                                               : half_res ? D.bloomScale()
                                                                          : D.bloomScale()/4,
                                                                            D.bloomAdd  (),
                                                                            D.bloomGlow ()/(BLOOM_GLOW_GAMMA_PER_PIXEL ? res2 : Sqr(res2)))); // for !BLOOM_GLOW_GAMMA_PER_PIXEL we need to square res2 because of "glow.a=SRGBToLinearFast(glow.a)" in the shader which is before "glow.a/=res2;", so "SRGBToLinearFast(glow.a/res2)=SRGBToLinearFast(glow.a)/Sqr(res2)"
         Sh.imgSize(src); GetBloomDS(_has_glow, D._view_main.full, half_res, exposure!=null)->draw(src, rect);
      }

    //Sh.imgSize(*rt0); we can just use 'RTSize' instead of 'ImgSize' since there's no scale
      REP(blurs)
      { // 'discard' before 'set' because it already may have requested discard, and if we 'discard' manually after 'set' then we might discard 2 times
         if(discard)rt1->discard(); set(rt1, null, false); Sh.BlurX[1]->draw(rt0, rect); discard=true; // discard next time
                    rt0->discard(); set(rt0, null, false); Sh.BlurY[1]->draw(rt1, rect);
      }
   }else
   {
      rt0.get(ImageRTDesc(1, 1, IMAGERT_SRGB));
      rt0->clearFull();
      Sh.BloomParams->setConditional(D.bloomOriginal());
   }
   set(&dest, null, true); if(combine && &dest==_final)D.alpha(ALPHA_MERGE);
   Sh.Img [1]->set( rt0  );
   Sh.ImgX[0]->set(_alpha);
   GetBloom(D.toneMap(), _alpha ? 2 : alpha ? 1 : 0, D.dither() /*&& (src.highPrecision() || rt0->highPrecision())*/ && !dest.highPrecision(), exposure!=null)->draw(src); // merging 2 RT's ('src' and 'rt0') with some scaling factors will give high precision
   bloom_glow.clear(); // not needed anymore
}
/******************************************************************************/
INLINE Shader* GetDofDS(Bool view_full, Bool realistic, Bool alpha, Bool half_res) {Shader* &s=Dof.DofDS[view_full][realistic][alpha][half_res]; if(SLOW_SHADER_LOAD && !s)s=Dof.getDS(view_full, realistic, alpha, half_res); return s;}
INLINE Shader* GetDof  (Bool dither   , Bool realistic, Bool alpha               ) {Shader* &s=Dof.Dof  [dither   ][realistic][alpha]          ; if(SLOW_SHADER_LOAD && !s)s=Dof.get  (dither   , realistic, alpha          ); return s;}
// !! Assumes that 'ImgClamp' was already set !!
void RendererClass::dof(ImageRT &src, ImageRT &dest, Bool alpha, Bool combine)
{ // Depth of Field shader does not require stereoscopic processing because it just reads the depth buffer
   const Int   shift=1; // half_res
   const VecI2 fx=T.fx();
   ImageRTDesc rt_desc(fx.x>>shift, fx.y>>shift, src.highPrecision() ? IMAGERT_SRGBA_H : IMAGERT_SRGBA); // here Alpha is used to store amount of Blur, use high precision if source is to don't lose smooth gradients when having full blur (especially visible on sky), IMAGERT_SRGBA_H vs IMAGERT_SRGBA has no significant difference on GeForce 1050Ti
   ImageRTPtr  rt0(rt_desc),
               rt1(rt_desc), blur_smooth[2];
   if(alpha)
   {
      rt_desc.type(IMAGERT_ONE_S);
      blur_smooth[0].get(rt_desc);
      blur_smooth[1].get(rt_desc);
   }

   Bool half_res=(Flt(src.h())/rt0->h() <= 2.5f); // half_res=scale 2, ..3.., quarter=scale 4, 2.5 was the biggest scale that didn't cause jittering when using half down-sampling
   Dof.load();
   D.alpha(ALPHA_NONE);
 C DepthOfField::Pixel &pixel=Dof.pixel(alpha, Round(fx.y*(5.0f/1080))); // use 5 pixel range blur on a 1080 resolution

   Flt range_inv=1.0f/Max(D.dofRange(), EPS);
   Dof.DofParams->setConditional(Vec4(D.dofIntensity(), D.dofFocus(), range_inv, -D.dofFocus()*range_inv));

   set(rt0, blur_smooth[0], null, null, null, false); Rect ext_rect, *rect=null; if(!D._view_main.full){ext_rect=D.viewRect(); rect=&ext_rect.extend(pixelToScreenSize(pixel.pixels+1));} // when not rendering entire viewport, then extend the rectangle because of blurs checking neighbors, add +1 because of texture filtering, we can ignore stereoscopic there because that's always disabled for not full viewports, have to use 'Renderer.pixelToScreenSize' and not 'D.pixelToScreenSize' and call after setting RT
   Sh.imgSize( src); GetDofDS(D._view_main.full, D.dofFocusMode(), alpha, half_res)->draw(src, rect);
 //Sh.imgSize(*rt0); we can just use 'RTSize' instead of 'ImgSize' since there's no scale
                   set(rt1, blur_smooth[1], null, null, null, false); Sh.ImgX[0]->set(blur_smooth[0]); pixel.BlurX[alpha]->draw(rt0, rect);
   rt0->discard(); set(rt0, blur_smooth[0], null, null, null, false); Sh.ImgX[0]->set(blur_smooth[1]); pixel.BlurY[alpha]->draw(rt1, rect);

   set(&dest, null, true); if(combine && &dest==_final)D.alpha(ALPHA_MERGE);
   Sh.Img [1]->set(          rt0 );
   Sh.ImgX[0]->set(blur_smooth[0]);
   GetDof(D.dither() /*&& (src.highPrecision() || rt0->highPrecision())*/ && !dest.highPrecision(), D.dofFocusMode(), alpha)->draw(src);
}
/******************************************************************************
void RendererClass::toneMap(ImageRT &src, ImageRT &dest, Bool alpha, Bool combine)
{
   Hdr.load();
   set(&dest, null, true); D.alpha((combine && &dest==_final) ? ALPHA_MERGE : ALPHA_NONE);
   Hdr.ToneMap[D.toneMap()-1][alpha][D.dither() && !dest.highPrecision()]->draw(src);
}
/******************************************************************************/
INLINE Shader* GetSetAlphaFromDepth        (Bool sky) {Shader* &s=Sh.SetAlphaFromDepth        [sky]; if(SLOW_SHADER_LOAD && !s)s=Sh.get(S8+"SetAlphaFromDepth"        +sky); return s;}
INLINE Shader* GetSetAlphaFromDepthMS      (Bool sky) {Shader* &s=Sh.SetAlphaFromDepthMS      [sky]; if(SLOW_SHADER_LOAD && !s)s=Sh.get(S8+"SetAlphaFromDepthMS"      +sky); return s;}
INLINE Shader* GetSetAlphaFromDepthAndCol  (Bool sky) {Shader* &s=Sh.SetAlphaFromDepthAndCol  [sky]; if(SLOW_SHADER_LOAD && !s)s=Sh.get(S8+"SetAlphaFromDepthAndCol"  +sky); return s;}
INLINE Shader* GetSetAlphaFromDepthAndColMS(Bool sky) {Shader* &s=Sh.SetAlphaFromDepthAndColMS[sky]; if(SLOW_SHADER_LOAD && !s)s=Sh.get(S8+"SetAlphaFromDepthAndColMS"+sky); return s;}

INLINE Shader* GetCombineAlpha() {Shader* &s=Sh.CombineAlpha; if(SLOW_SHADER_LOAD && !s)s=Sh.get("CombineAlpha"); return s;}
INLINE Shader* GetReplaceAlpha() {Shader* &s=Sh.ReplaceAlpha; if(SLOW_SHADER_LOAD && !s)s=Sh.get("ReplaceAlpha"); return s;}
/******************************************************************************/
void RendererClass::cleanup()
{
   FlagDisable(_has, HAS_CLEAR_COAT|HAS_FUR); // !! DO NOT CLEAR 'HAS_GLOW' and '_has_glow' because this is called also for reflections, but if reflections have glow, then it means final result should have glow too !!
//_final     =null   ; do not clear '_final' because this is called also for reflections, after which we still need '_final'
  _ds        .clear();
//_ds_1s     .clear(); do not clear '_ds_1s' because 'setDepthForDebugDrawing' may be called after rendering finishes, also 'capture' makes use of it
   if(!_get_target)_col.clear();
  _nrm       .clear();
  _ext       .clear();
  _vel       .clear();
  _alpha     .clear();
  _lum       .clear();
  _lum_1s    .clear();
  _spec      .clear();
  _spec_1s   .clear();
  _shd_1s    .clear();
  _shd_ms    .clear();
  _water_col .clear();
  _water_nrm .clear();
  _water_ds  .clear();
  _water_lum .clear();
  _water_spec.clear();
  _vol       .clear();
  _ao        .clear();
  _mirror_rt .clear();
  _outline_rt.clear();
   Lights.clear();
   ClearInstances();
}
void RendererClass::cleanup1()
{
  _ds_1s.clear(); // '_ds_1s' isn't cleared in 'cleanup' in case it's used for drawing, so clear it here to make sure we can call discard
   REPA(_ctxs)
   {
      Context &ctx=_ctxs[i];
      ctx.old_data =ctx.new_data ; ctx.new_data .clear();
   #if TEMPORAL_SEPARATE_ALPHA
      ctx.old_alpha=ctx.new_alpha; ctx.new_alpha.clear();
   #endif
      ctx.old_col  =ctx.new_col  ; ctx.new_col  .clear();
      REPA(ctx.subs)
      {
         Context::Sub &sub=ctx.subs[i];
         if(!sub.used)ctx.subs.remove(i);else sub.used=false; // remove unused sub-contexts
      }
      if(!ctx.subs.elms())_ctxs.remove(i); // if was unused then remove it
      // !! here can't access 'ctx' anymore !!
   }
}
RendererClass& RendererClass::operator()(void (&render)())
{
   DEBUG_ASSERT(App.mainThread(), "Rendering can't be performed on a secondary thread"); // on GL VAOs are limited only to the main thread, so they can't be used at all on other threads. And on all GPU_APIs Cache/Map use SyncUnlocker which may unlock D._lock while still in Rendering, and at that time State Update/Draw may lock D._lock and perform operations on GPU Device and Renderer itself, or even just change some display settings, especially D.view*, or others. To solve this, SyncUnlocker would have to be not used (not sure if possible), or another SyncLock D._draw_lock introduced, and used in most places where D._lock is used (have to search for "D._lock" and "_lock" in "Display.cpp")
#if DEBUG
   if(Kb.b(KB_NP0))stage=RS_DEPTH;else
   if(Kb.b(KB_NP1))stage=RS_COLOR;else
   if(Kb.b(KB_NP2))stage=RS_NORMAL;else
   if(Kb.b(KB_NP3))stage=RS_MOTION;else
   if(Kb.b(KB_NP4))stage=RS_SMOOTH;else
   if(Kb.b(KB_NP5))stage=RS_REFLECT;else
   if(Kb.b(KB_NP6))stage=RS_GLOW;else
   if(Kb.b(KB_NP7))stage=(Kb.b(KB_NP8) ? RS_LIGHT_AO : RS_LIGHT);else
   if(Kb.b(KB_NP8))stage=RS_AO;else
   if(Kb.b(KB_NPDEL))stage=RS_LIT_COLOR;else
   if(Kb.b(KB_NPDIV))stage=RS_WATER_COLOR;else
   if(Kb.b(KB_NPMUL))stage=RS_WATER_NORMAL;else
   if(Kb.b(KB_NPSUB))stage=RS_WATER_LIGHT;else
   if(Kb.b(KB_NPADD))stage=RS_REFLECTION;else
   if(Kb.br(KB_NP0) || Kb.br(KB_NP1) || Kb.br(KB_NP2) || Kb.br(KB_NP3) || Kb.br(KB_NP4) || Kb.br(KB_NP5) || Kb.br(KB_NP6) || Kb.br(KB_NP7) || Kb.br(KB_NP8) || Kb.br(KB_NP9)
   || Kb.br(KB_NPDIV) || Kb.br(KB_NPMUL) || Kb.br(KB_NPSUB) || Kb.br(KB_NPADD) || Kb.br(KB_NPDEL))stage=RS_DEFAULT;
#endif

   // prepare
  _render     =render;
  _stereo     =(VR.active() && D._view_main.full && !combine && !target && !_get_target && D._allow_stereo); // use stereo only for full viewport, if we're not combining (games may use combining to display 3D items/characters in Gui)
  _eye_num    =_stereo+1; // _stereo ? 2 : 1
  _mirror_want=false;
  _has        =0;
  _outline    =0;
   if(target)
   {
      target->_ptr_num++; // increase ptr count to make sure this won't be discarded in case user does not hold a pointer to the target. This will be decreased at the end of this function when rendering finished
     _final=target;
   }else
  _final=(_stereo ? VR.getRender() : _cur_main);

  _ctx=_ctxs(temporal_id);
  _ctx_sub=_ctx->subs(D._view_main.recti, _temporal_reset); _ctx_sub->used=true; // find a unique sub-context based on main viewport rectangle, set '_temporal_reset' as 'just_created' to force reset Temporal when creating new sub, mark that it was used in this frame

   if(VR.active())D.setViewFovTan(); // !! call after setting _stereo and _render !!

   // !! it is important to call this as the first thing during rendering, because 'Game.WorldManager.draw' assumes so, if this needs to be changed then rework 'Game.WorldManager.draw' !!
   // set water
   Water.prepare();

#define MEASURE(x) if(_t_measure){D.finish(); Dbl c=Time.curTime(); x+=c-t; t=c;}

   // render
   {
      Dbl t; Flt temp, water;
      if(_t_measure){D.finish(); t=Time.curTime(); temp=water=0; _t_measures[0]++;}

      if(reflection())goto finished; MEASURE(_t_reflection[1])

      prepare(); MEASURE(_t_prepare[1])
      opaque (); MEASURE(_t_opaque [1])
   #if GL && TILE_BASED_GPU && !WEB // we need to make sure that depth RT is flushed to depth texture on tile-based deferred renderers, this is because on those devices the RT's (including depth buffer) are stored in fast on-chip memory and to be able to read from them, we need to flush them to the texture memory. This is done after reading opaque's and before we may read from the depth buffer. No need to do on WEB because there we can never read from depth while writing to it.
      if(canReadDepth())
         if(D.edgeDetect() || D.particlesSoft() || Sky.wantDepth() || Clouds.wantDepth() || Fog.draw || Water._under_mtrl/* || Sun.wantDepth()*/) // here we need to check only effects that may read from depth without changing any RT's, because on any RT change the depth is flushed. Sun doesn't bind DS to FBO when drawing rays. TODO: we wouldn't need to do this if all shaders reading from the depth would use gl_LastFragDepth - https://www.khronos.org/registry/OpenGL/extensions/ARM/ARM_shader_framebuffer_fetch_depth_stencil.txt
      { // unbinding will force the flush (calling just 'glFlush' was not enough)
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_RENDERBUFFER, 0);
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        _cur_ds=null; _cur_ds_id=0;
      }
   #endif
      overlay(); MEASURE(_t_overlay[1])

      // set background sky pixels not drawn by foreground object meshes (everything at the end of depth buffer), this needs to be done before 'blend' because fur may set depth buffer without setting velocities, and before water surface
      {
         const Bool clear_nrm=(!NRM_CLEAR_START && _nrm && NeedBackgroundNrm()),
                    clear_ext=(!EXT_CLEAR_START && _ext && NeedBackgroundExt()),
                    clear_vel=(!VEL_CLEAR_START && _vel);
         if(clear_nrm || clear_ext || clear_vel)
         {
            if(clear_vel)SetViewToViewPrev();
            D.alpha    (ALPHA_NONE);
            D.depth2DOn(FUNC_BACKGROUND); // this enables depth but disables 'D.depthWrite'
            set(null, clear_nrm ? _nrm() : null, clear_ext ? _ext() : null, clear_vel ? _vel() : null, _ds, true); // use DS because we use it for 'D.depth2D' optimization #RTOutput
            Sh.ClearDeferred->draw();
            D.depth2DOff();
         }
      }

      if(stage)switch(stage)
      {
         case RS_COLOR  : if(_cur_type==RT_DEFERRED && show(_col  , true                  ))goto finished; break; // only on deferred renderer the color is unlit
         case RS_NORMAL : if(                          show(_nrm  , false, D.signedNrmRT()))goto finished; break;
         case RS_SMOOTH : if(                          show(_ext  , false, false, 0, true ))goto finished; break; // #RTOutput inverse because it's roughness
         case RS_REFLECT: if(                          show(_ext  , false, false, 1       ))goto finished; break; // #RTOutput
         case RS_DEPTH  : if(                          show(_ds_1s, false                 ))goto finished; break; // this may be affected by test blend materials later
      }
      waterPreLight(); MEASURE(water)
      light        (); MEASURE(_t_light[1])
      if(stage)switch(stage)
      {
         case RS_LIT_COLOR: if(show(_col, true))goto finished; break;

         case RS_LIGHT: if(_lum_1s)
         {
            if(!ambientInLum()) // if ambient hasn't been applied to '_lum_1s', apply it here
            {
               set(_lum_1s, null, true);
               D.alpha(ALPHA_ADD);
               Sh.clear(Vec4(D.ambientColorD(), 0));
            }
            if(show(_lum_1s, true))goto finished;
         }break;

         case RS_AO: if(_ao)
         {
            set(_final, null, true); D.alpha(ALPHA_NONE); Sh.ImgX[0]->set(_ao); (LINEAR_GAMMA ? Sh.DrawXG : Sh.DrawX)->draw();
            goto finished;
         }break;

         case RS_LIGHT_AO: if(_lum_1s)
         {
            if(_ao)
            {
               set(_lum_1s, null, true);
               Sh.ImgX[0]->set(_ao);
               if(D.aoAll())
               {
                  D.alpha(ALPHA_MUL);
                  Sh.DrawX->draw();
               }else
               {
                  D.alpha(ALPHA_ADD);
                  Sh.Color[0]->set(Vec4(D.ambientColorD(), 0));
                  Sh.Color[1]->set(Vec4Zero                  );
                  Sh.DrawXC[0][0]->draw();
               }
            }
            if(show(_lum_1s, true))goto finished;
         }break;
      }
      if(waterPostLight())goto finished; MEASURE(_t_water[1])if(_t_measure)_t_water[1]+=water;

      emissive  (); MEASURE(_t_emissive[1])
      if(stage)switch(stage)
      {
         case RS_GLOW    : if(                          show(_col, false, false, 3))goto finished; break; // #RTOutput
         case RS_EMISSIVE: if(_cur_type==RT_DEFERRED && show(_col, true           ))goto finished; break; // only on deferred renderer
      }
      sky       (); MEASURE(_t_sky[1])
      waterUnder(); MEASURE(_t_water_under[1])
      edgeDetect(); MEASURE(_t_edge_detect[1])
      blend     (); MEASURE(_t_blend[1])
      if(hasDof()){resolveDepth1(); MEASURE(temp)}
    /*if(stage)switch(stage) check this earlier together with other stages, to avoid doing a single extra check here
      {
         case RS_DEPTH: if(show(_ds_1s, false))goto finished; break;
      }*/

      palette(0);
      palette(1); MEASURE(_t_palette[1])
      behind ( ); MEASURE(_t_behind [1])
      outline( ); MEASURE(temp)

      // 2D
      finalizeGlow(); // !! assume that nothing below can trigger glow on the scene !!
      applyOutline();

      if(stage)switch(stage)
      {
         case RS_ALPHA: if(show(_alpha, false))goto finished; break;
      }

      if(!D.temporalSuperRes())edgeSoften(); MEASURE(temp) // if have temporal super-res then 'edgeSoften' must be run after upscale
      AstroDrawRays(); MEASURE(_t_rays[1])
      volumetric   (); MEASURE(_t_volumetric[1])
      postProcess  (); MEASURE(_t_post_process[1])
   finished:;
   }

   // cleanup
   {
      temporalFinish();
     _ctx_sub->proj_matrix_prev=ProjMatrix; // set always because needed for MotionBlur and Temporal
     _ctx_sub=&_ctx_sub_dummy;
     _ctx    =null;
     _render =null; // this specifies that we're outside of Rendering
     _final  =null;
      D.alpha(ALPHA_BLEND); mode(RM_OPAQUE);
      set(_cur_main, _cur_main_ds, true);

      Sky.setFracMulAdd(); // in case user draws billboards/particles outside of Renderer, call before 'cleanup' because this relies on depth buffer being available
      cleanup();

      if(VR.active())
      {
         D.setViewFovTan(); // !! call after clearing _render !!
         if(_stereo)
         {
            D.clearCol(); // normally after rendering has finished we expect to draw on top of the result, however for stereoscopic, we've rendered to a separate render target, that won't be used for 2D drawing now, instead we're now back to '_cur_main' which is not yet initialized, so we need to clear it here
            // restore settings to centered (not one of the eyes)
            D._view_main.setShader().setProjMatrix(); // viewport
            SetCam(ActiveCam.matrix, ActiveCam._matrix_prev); // camera, 'Frustum' remains the same
         }
      }
   }
   if(_shader_param_changes)Exit("'LinkShaderParamChanges' was called without 'UnlinkShaderParamChanges'.");
   if(target)target->_ptr_num--; // decrease ptr count without discarding
   return T;
}
ImageRTPtr RendererClass::get(void (&render)())
{
  _get_target=true ; T(render);
  _get_target=false; ImageRTPtr temp=_col; _col.clear(); return temp;
}
Bool RendererClass::reflection()
{
   // render reflection
   if(_mirror_want)
   {
      // remember current settings and disable fancy effects
      auto cam            =ActiveCam            ;
      auto combine        =T. combine           ;                     T. combine        =false             ;
      auto alpha          =T. alpha             ;                     T. alpha          =false             ;
      auto get_target     =T._get_target        ;                     T._get_target     =true              ;
      auto stereo         =T._stereo            ;                     T._stereo         =false             ; // don't reset FOV because we want to render the reflection with the same exact FOV settings as only one eye, because this reflection will be reused for both eyes
      auto eye_num        =T._eye_num           ;                     T._eye_num        =1                 ;
      auto render_type    =T._cur_type          ;                     T._cur_type       =Water.reflectionRenderer();
      auto hp_col_rt      =D.highPrecColRT    ();                     D._hp_col_rt      =false             ;
      auto hp_nrm_rt      =D.highPrecNrmRT    ();                     D._hp_nrm_rt      =false             ;
      auto hp_lum_rt      =D.highPrecLumRT    ();                     D._hp_lum_rt      =false             ;
      auto hp_pal_rt      =D.highPrecPalRT    ();                     D._hp_pal_rt      =false             ;
      auto lit_col_rt_prec=D.litColRTPrecision();                     D._lit_col_rt_prec=IMAGE_PRECISION_8 ;
      auto eye_adapt      =D.eyeAdaptation    ();                     D.eyeAdaptation   (false            );
      auto tone_map       =D.toneMap          ();                     D.toneMap         (TONE_MAP_OFF     );
      auto vol_light      =D.volLight         ();                     D.volLight        (false            ); // if it will be enabled, then calling 'volumetric' is required and clearing '_vol_is'
      auto amb_mode       =D.ambientMode      ();                     D.ambientMode     (AMBIENT_FLAT     );
      auto mtn_mode       =D.motionMode       ();                     D.motionMode      (MOTION_NONE      );
      auto shd_mode       =D.shadowMode       (); if(!_mirror_shadows)D.shadowMode      (SHADOW_NONE      );
      auto shd_soft       =D.shadowSoft       ();                     D.shadowSoft      (0                );
      auto shd_jitter     =D.shadowJitter     ();                     D.shadowJitter    (false            );
      auto edge_soft      =D.edgeSoften       ();                     D._edge_soften    =EDGE_SOFTEN_NONE  ;
      auto tesselation    =D.tesselationAllow ();                     D.tesselationAllow(false            );
      auto density        =D.densityFast      ();                     D.densityFast     (Mid(((D.densityFast()+1)>>_mirror_resolution)-1, 0, 255));

      // set new settings
     _mirror=true;                                                           // set before viewport and camera
      // <- change viewport here if needed
      ConstCast(ActiveCam).matrix.mirror(_mirror_plane); ActiveCamChanged(); // set mirrored camera and frustum
      D.clipPlane(_mirror_plane);                                            // set clip plane after viewport and camera
      D.lodSetCurrentFactor();

      // render !! adding new modes here will require setting there D.clipPlane !!
      prepare();
      opaque ();
      light  ();
      sky    ();
      blend  ();
      AstroDrawRays();

      // cleanup
      cleanup(); Swap(_mirror_rt, _col); // 'cleanup' clears '_mirror_rt' so we need to set it after calling the function

      // restore effects (before viewport and camera, because stereoscopic affects viewport fov)
      T. combine        =combine        ;
      T. alpha          =alpha          ;
      T._get_target     =get_target     ;
      T._stereo         =stereo         ;
      T._eye_num        =eye_num        ;
      T._cur_type       =render_type    ;
      D._hp_col_rt      =hp_col_rt      ;
      D._hp_nrm_rt      =hp_nrm_rt      ;
      D._hp_lum_rt      =hp_lum_rt      ;
      D._hp_pal_rt      =hp_pal_rt      ;
      D._lit_col_rt_prec=lit_col_rt_prec;
      D.eyeAdaptation   (eye_adapt     );
      D.toneMap         (tone_map      );
      D.volLight        (vol_light     );
      D.ambientMode     (amb_mode      );
      D.motionMode      (mtn_mode      );
      D.shadowMode      (shd_mode      );
      D.shadowSoft      (shd_soft      );
      D.shadowJitter    (shd_jitter    );
      D._edge_soften    =edge_soft      ;
      D.tesselationAllow(tesselation   );
      D.densityFast     (density       );

      // restore previous settings (mirror, viewport and camera)
     _mirror=false;            // !! set before viewport and camera, because it affects the Frustum, and after 'cleanup' !!
      // <- reset viewport here if needed
      cam.set();               // camera, this will also reset 'Frustum'
      D.lodSetCurrentFactor();

      if(stage==RS_REFLECTION && show(_mirror_rt, true))return true;
   }
   return false;
}
/******************************************************************************/
Bool RendererClass::wantTemporal()C {return allow_temporal && D.temporal() && _cur_type==RT_DEFERRED && D._max_rt>=4;} // requires Vel RT which is only in Deferred and has slot #3 #RTOutput
Bool RendererClass:: hasTemporal()C {return wantTemporal() && !fastCombine();}

Bool RendererClass:: hasEdgeSoften()C {return wantEdgeSoften() && !fastCombine();}
Bool RendererClass::wantEdgeSoften()C
{
   switch(D.edgeSoften())
   {
      case EDGE_SOFTEN_FXAA: return /*Sh.FXAA[0]!=null &&*/ Sh.FXAA[1]!=null; // check 'FXAA[1]' only, because it's set only if all loaded OK
   #if SUPPORT_MLAA
      case EDGE_SOFTEN_MLAA: return Sh.MLAAEdge && Sh.MLAABlend && Sh.MLAA && _mlaa_area.is();
   #endif
      case EDGE_SOFTEN_SMAA: return /*Sh.SMAAEdge[0] && Sh.SMAAEdge[1] && Sh.SMAABlend && Sh.SMAA && _smaa_area.is() &&*/ _smaa_search.is(); // check '_smaa_search' only, because it's set only if all loaded OK
   }
   return false;
}
Bool RendererClass::wantDepth         ()C {return wantTemporal() || wantMotion() || wantDof() || D.aoWant() || D.edgeDetect() || D.particlesSoft() || D.volLight() || Sky.wantDepth() || Clouds.wantDepth() || Fog.draw || Sun.wantDepth() || !Water.max1Light();} // TODO: even though we check all known things here, there are still some things about we don't know up-front (like local fog, decals, Image.drawVolume, ..)
Bool RendererClass::canReadDepth      ()C {return        _ds->depthTexture();} // have to check '_ds' because this is the original source depth, it can be multi-sampled (in that case it's possible depth reading won't be available), but '_ds_1s' is 1-sampled (and could have depth-reads even if '_ds' doesn't)
Bool RendererClass::safeCanReadDepth  ()C {return _ds && _ds->depthTexture();}
Bool RendererClass::hasStencilAttached()C {return hasDepthAttached() && _cur_ds->hwTypeInfo().s;}
Bool RendererClass::hasDepthAttached  ()C
{
#if GL
   return _cur_ds && _cur_ds->_txtr==_cur_ds_id; // check both '_cur_ds' and '_cur_ds_id' because '_cur_ds_id' will be 0 when Image is a RenderBuffer or temporarily unbound Texture (only Textures can be temporarily unbound), this will work OK for RenderBuffers because both '_cur_ds_id' and 'cur_ds->_txtr' will be zero
#else
   return _cur_ds_id!=null; // we have to check '_cur_ds_id' because on DX10 it can be null if read-only is not supported
#endif
}
Bool RendererClass::canReadDepth1S()C
{
   return canReadDepth()
   #if DX11
      && _ds_1s->_dsv !=_cur_ds_id // we always read from '_ds_1s', we can do that only if it's not bound as Depth RT, on purpose we check '_dsv' and not '_rdsv' because '_rdsv' IS allowed
   #elif WEB
      && _ds_1s->_txtr!=_cur_ds_id // we always read from '_ds_1s', we can do that only if it's not bound as Depth RT
   #endif
   ;
}

Bool RendererClass::wantBloom   ()C {return D.bloomUsed();}
Bool RendererClass:: hasBloom   ()C {return wantBloom() && !fastCombine();}
Bool RendererClass::wantEyeAdapt()C {return D.eyeAdaptation() && _eye_adapt_scale[0].is();}
Bool RendererClass:: hasEyeAdapt()C {return wantEyeAdapt() && !fastCombine();}
Bool RendererClass::wantMotion  ()C {return D.motionMode() && D.motionScale()>EPS_COL8 && (D.motionMode()==MOTION_CAMERA || (_cur_type==RT_DEFERRED && D._max_rt>=4));} // MOTION_CAMERA_OBJECT requires Vel RT which is only in Deferred and has slot #3 #RTOutput
Bool RendererClass:: hasMotion  ()C {return wantMotion() && canReadDepth() && !fastCombine();}
Bool RendererClass::wantDof     ()C {return D.dofWant();}
Bool RendererClass:: hasDof     ()C {return wantDof() && canReadDepth() && !fastCombine();}
Bool RendererClass:: hasAO      ()C {return D.aoWant() && canReadDepth() && !fastCombine();}

Bool RendererClass::fastCombine      ()C {return combine && _col==_final;}
Bool RendererClass::slowCombine      ()C {return combine && !fastCombine() && canReadDepth();}
Bool RendererClass::processAlpha     ()C {return ((alpha && (_get_target || target) || Sun._actual_rays_mode==SUN_RAYS_OPAQUE_BLEND || stage==RS_ALPHA) && canReadDepth()) || slowCombine();}
Bool RendererClass::processAlphaFinal()C {return ((alpha && (_get_target || target)                                                 || stage==RS_ALPHA) && canReadDepth()) || slowCombine();}
Bool RendererClass::hasVolLight      ()C {return D.volLight() && canReadDepth();}
Bool RendererClass::anyDeferred      ()C {return type()==RT_DEFERRED || Water.reflectionRenderer()==RT_DEFERRED;}
Bool RendererClass::anyForward       ()C {return type()==RT_FORWARD  || Water.reflectionRenderer()==RT_FORWARD ;}
Bool RendererClass::lowDepthPrecision()C {return _main_ds.type()==IMAGE_D16;} // this can happen only on Android, and there we do have information about the depth buffer
Bool RendererClass::ambientInLum     ()C {return D.aoAll() || !Renderer._ao;} // only these cases allow putting ambient inside lum RT's #AmbientInLum
/******************************************************************************/
Bool RendererClass::show(C ImageRTPtr &image, Bool srgb, Bool sign, Int channel, Bool inverse)
{
   if(image)
   {
      if(image->hwTypeInfo().d) // depth
      {
         if(!image->depthTexture())return false; // can't read
         set(_final, null, true); D.alpha(ALPHA_NONE); Sh.Depth->set(image); Sh.get("DrawDepth")->draw(); Sh.Depth->set(_ds_1s);
      }else
      if(sign)
      {
         // we need to draw image*0.5+0.5
         Sh.Color[0]->set(Vec4(0.5f, 0.5f, 0.5f, 0));
         Sh.Color[1]->set(Vec4(0.5f, 0.5f, 0.5f, 1));
         set(_final, null, true); D.alpha(ALPHA_NONE); ((LINEAR_GAMMA && !srgb) ? Sh.DrawCG : Sh.DrawC)->draw(image);
      }else
      if(InRange(channel, 4))
      {
         set(_final, null, true); D.alpha(ALPHA_NONE);
         Shader *shader;
         if(LINEAR_GAMMA && !srgb)switch(channel) // gamma convert
         {
            case 0: shader=Sh.get(inverse ? "DrawTexXIG" : "DrawTexXG"); break;
            case 1: shader=Sh.get(inverse ? "DrawTexYIG" : "DrawTexYG"); break;
            case 2: shader=Sh.get(inverse ? "DrawTexZIG" : "DrawTexZG"); break;
            case 3: shader=Sh.get(inverse ? "DrawTexWIG" : "DrawTexWG"); break;
         }else switch(channel)
         {
            case 0: shader=Sh.get(inverse ? "DrawTexXI" : "DrawTexX"); break;
            case 1: shader=Sh.get(inverse ? "DrawTexYI" : "DrawTexY"); break;
            case 2: shader=Sh.get(inverse ? "DrawTexZI" : "DrawTexZ"); break;
            case 3: shader=Sh.get(inverse ? "DrawTexWI" : "DrawTexW"); break;
         }
         VI.shader(shader); image->draw(D.rect());
      }else
      if(LINEAR_GAMMA && !srgb)
      {
         set(_final, null, true); D.alpha(ALPHA_NONE); Sh.DrawG->draw(image);
      }else
         image->copyHw(*_final, false, D.viewRect());
      return true;
   }
   return false;
}
Bool RendererClass::swapDS1S(ImageRTPtr &ds_1s)
{
   if(!T._ds_1s->accessible())return false; // can't swap if current DS is not accessible
   if( T._ds==T._ds_1s)
   {
       if(T._ds==T._cur_main_ds)T._cur_main_ds=ds_1s;
          T._ds   = ds_1s ; Sh.DepthMS->set(T._ds   );
   } Swap(T._ds_1s, ds_1s); Sh.Depth  ->set(T._ds_1s);
   return true;
}
void RendererClass::setDS()
{
   if(_col==&_main    )_ds=&_main_ds    ;else // we should always pair '_main' with '_main_ds', even if it's not 'accessible'
   if(_col== _cur_main)_ds= _cur_main_ds;else // reuse '_cur_main_ds' if we're rendering to '_cur_main'
                       _ds.getDS(_col->w(), _col->h(), _col->samples()); // create a new one
}
void RendererClass::temporalCheck() // needs to be called after RT and viewport were set
{
   if(hasTemporal())
   {
     _temporal_use=true;
      Vec2 offset;
      if(_temporal_reset) // if doesn't have previous RT
      {
         offset.zero(); // render at center
      }else
      {
         auto frame=Unsigned(Time.frame());
         if(D.temporalSuperRes())
         {
            Byte i=TemporalSuperResIndex[frame%Elms(TemporalSuperResIndex)];
            Sh.TemporalCurPixel->set(TemporalSuperResIndexToOfsI(i));
            offset=TemporalSuperResOffsets[frame%Elms(TemporalSuperResOffsets)];
            if(D.temporalAntiAlias())offset+=TemporalAntiAliasOffsets[(frame/Elms(TemporalSuperResIndex))%Elms(TemporalAntiAliasOffsets)]*0.5f;
         }else
         {
            offset=TemporalAntiAliasOffsets[frame%Elms(TemporalAntiAliasOffsets)];
         }
      }

    C VecI2 &size       =res();
      RectI  viewport   =(_stereo ? screenToPixelI(D._view_eye_rect[0]) : D._view_active.recti);
      Vec2   mul        =Vec2(0.5f, -0.5f)/size;
        _temporal_offset=offset/viewport.size();
      Vec2 shader_offset=offset*mul;
       Sh.TemporalOffset->set(shader_offset); // this always changes so don't use 'setConditional'

      /*Select index to point to the component as if we were accessing without TemporalOffset - TexPoint(uv)
        TEXTURE ACCESSING                 (Y^)
        GATHER returns in following order: V1 X0 Y1
                                           V0 W3 Z2
                                            + U0 U1 (X>)*/
      Sh.TemporalOffsetGatherIndex->set((shader_offset.y>=0) ? (shader_offset.x>=0) ? 3 : 2 // this is 100% correct (tested and verified), always changes so don't use 'setConditional'
                                                             : (shader_offset.x>=0) ? 0 : 1);
      // this is offset to apply for UV located at the exact center of a pixel, in order to get the (0,0) coordinates of the 2x2 pixels that are returned with shader gather, add +(1,1) to get last
      Sh.TemporalOffsetStart->set(Vec2((shader_offset.x>=0) ? 0 : -1.0f/size.x, // if TemporalOffset is >=0 then we're starting from the same pixel 0 .. 1, but if it's negative <0, then we have to start from -1 .. 0
                                       (shader_offset.y>=0) ? 0 : -1.0f/size.y));
      D._view_active.setShader();
   }
   SetProjMatrix(); // call after setting '_temporal_offset', always call because needed for MotionBlur and Temporal
}
void RendererClass::prepare()
{
   Byte  samples=(mirror() ? 1 : D.samples()); // disable multi-sampling for reflection
   VecI2 rt_size;
start:
   if(VR.active() && D._allow_stereo // following conditions affect this: _stereo, _allow_stereo, mirror()
   || target)
   { /* We want this case when:
         -rendering to VR    'GuiTexture'
         -rendering to VR 'RenderTexture'
         -rendering to mirror reflection to be used for VR 'RenderTexture'
        Remember that '_stereo' gets temporarily disabled when rendering mirror reflection */
      rt_size=_final->size();
      if(D.densityUsed())
      {
         Int mul=D.densityFast()+1;
         rt_size.set(Mid((rt_size.x*mul+64)/128, 1, D.maxTexSize()),
                     Mid((rt_size.y*mul+64)/128, 1, D.maxTexSize()));
      }
      if(Renderer.wantTemporal() && D.temporalSuperRes())rt_size.set(Max(1, rt_size.x>>1), Max(1, rt_size.y>>1));
   }else rt_size=D.render();

   IMAGE_PRECISION prec=((_cur_type==RT_DEFERRED) ? D.highPrecColRT() ? IMAGE_PRECISION_10 : IMAGE_PRECISION_8 : D.litColRTPrecision()); // for deferred renderer we first render to col and only after that we mix it with light, other modes render color already mixed with light, for high precision we need only 10-bit, no need for 16-bit
   if(_cur_type==RT_DEFERRED /*|| mirror() _get_target already enabled for mirror*/ || _get_target // <- these always require
   || _final->size()!=rt_size || _final->samples()!=samples || _final->precision()<prec // if current RT does not match the requested rendering settings
   || wantEdgeSoften() || wantTemporal() || wantEyeAdapt() || D.toneMap() || wantMotion() || wantBloom() || wantDof() || D.sharpen() // if we want to perform post process effects then we will be rendering to a secondary RT anyway, so let's start with secondary with a chance that during the effect we can render directly to '_final'
   || (D.glowAllow() && _final->hwTypeInfo().a<8) // we need alpha for glow, this check is needed for example if we have IMAGE_R10G10B10A2
   || (_final==&_main && !_main_ds.depthTexture() && wantDepth()) // if we're setting '_main' which is always paired with '_main_ds', and that is not a depth texture but we need to access depth, then try getting custom RT with depth texture (don't check for '_cur_main' and '_cur_main_ds' because depth buffers other than '_main_ds' are always tried to be created as depth texture first, so if that failed, then there's no point in trying to create one again)
   )    _col.get(ImageRTDesc(rt_size.x, rt_size.y, GetImageRTType(D.glowAllow(), prec), samples)); // here Alpha is used for glow
   else _col=_final;

   // depth stencil buffer
   setDS();
   if(combine && !canReadDepth() && _col!=_final) // if we need to combine (treat combine with priority), but after getting the depth buffer it turns out we can't read it (can't do combine), then we must render to final render target directly
   {
     _col=_final; setDS();
   }

   if(!canReadDepth() && _cur_type==RT_DEFERRED) // if depth access is not available and we want deferred renderer then fall back to forward renderer
   {
      if(type                    ()==RT_DEFERRED)type                    (RT_FORWARD);
      if(Water.reflectionRenderer()==RT_DEFERRED)Water.reflectionRenderer(RT_FORWARD);
                                            _cur_type                    =RT_FORWARD ;
      goto start;
   }

   // if we're going to completely replace '_final' then we can discard it
   if(D._view_main.full && !_get_target && !combine)_final->discard();

   if(!_ds->multiSample ())_ds_1s=_ds;else // if the source is not multi-sampled then reuse it
   if( _ds->depthTexture())_ds_1s.getDS(_ds->w(), _ds->h());else // create new only if we can resolve multi-sample onto 1-sample
                           _ds_1s.clear(); // there's no point in creating a 1-sampled depth buffer if we can't resolve from the multi-sampled
   Sh.Depth  ->set(_ds_1s);
   Sh.DepthMS->set(_ds   );

  _set_depth_needed=(_ds!=_cur_main_ds && _ds_1s!=_cur_main_ds && canReadDepth()); // if we're not rendering to currently main depth buffer and we have depth access

   // now 'col' and 'ds' are known, including their sizes

   Sh.NoiseOffset->set(hasTemporal() ? NoiseOffsets[ // use a set of predefined repeating offsets, because using always random generates too much flickering
      Time.frame()%(D.temporalAntiAlias() ?     Elms(NoiseOffsets) // with Anti-Alias we can use all samples
                                          : Min(Elms(NoiseOffsets), Elms(TemporalSuperResIndex)))] : VecI2Zero); // without Anti-Alias have to repeat
   
   D.alpha(ALPHA_NONE);

   mode(RM_PREPARE); AstroPrepare(); // !! call after obtaining '_col', '_ds' and '_ds_1s' because we rely on having them, and after RM_PREPARE because we may add lights !!
  _eye=0; if(_stereo)SetCam(EyeMatrix[_eye], EyeMatrixPrev[_eye]); // start with the first eye and set camera so we can calculate view_matrix for instances, this is important, because precalculated view_matrix is assumed to be for the first eye, so when rendering instances we need to adjust the projection matrix for next eye, this affects 'BeginPrecomputedViewMatrix', 'Frustum' remains the same
  _render(); // we can call '_render' only once for RM_PREPARE
   UpdateLights(); // update lights after RM_PREPARE to have knowledge of active lights ASAP
   Bool clear_ds=true; // if need to clear depth

#if SUPPORT_EARLY_Z
   if(HasEarlyZInstances())
   {
      set(null, _ds, true);
      temporalCheck(); D.clearDS(); clear_ds=false; // already cleared so no need anymore, 'temporalCheck' and 'D.clearDS' are paired to make sure 'temporalCheck' is called only once
      D.set3D(); D.depth(true); if(FUNC_DEFAULT!=FUNC_LESS_EQUAL)D.depthBias(BIAS_EARLY_Z); // one option is to write Z+1 values for EarlyZ using depth bias, so we can use FUNC_LESS for depth tests (better), another option is to don't use depth bias, but use FUNC_LESS_EQUAL for depth tests (potentially slower due to potential overdraw)

   early_z:
      setEyeViewportCam();
      DrawEarlyZInstances();
      if(++_eye<_eye_num)goto early_z;

      ClearEarlyZInstances();
      D.set2D(); if(FUNC_DEFAULT!=FUNC_LESS_EQUAL)D.depthBias(BIAS_ZERO);
   }
#endif

   const Bool clear_col=((!Sky.isActual() || stage==RS_COLOR || stage==RS_GLOW || stage==RS_EMISSIVE || stage==RS_LIT_COLOR || _col->multiSample()) && !fastCombine()); // performance tests suggested it's better don't clear unless necessary, instead 'Image.discard' is used and improves performance (at least on Mobile), always have to clear for multi-sampled to allow for proper detection of MSAA pixels using 'Sh.DetectMSCol' (this is needed for all renderers, not only Deferred, without this edges of sky/meshes may not get multi-sampled, especially when there's small variation in material color texture or no texture at all having just a single color) #RTOutput
   switch(_cur_type)
   {
      case RT_DEFERRED:
      {
         ImageRTDesc rt_desc(_col->w(), _col->h(), IMAGERT_SRGBA, _col->samples());

         if(D.motionMode()==MOTION_CAMERA_OBJECT && hasMotion() // motion blur
         || hasTemporal())                                      // Temporal
            if(!mirror()) // not for reflections (there motion is disabled, and Temporal may be enabled so we can apply offsets, however we don't want velocity RT, also it would be set to 'new_vel' which we don't want)
              _vel.get(rt_desc.type(IMAGERT_TWO_H));

         const Bool merged_clear=true, // D.mergedClear() FIXME: TODO: workaround for Nvidia GeForce flickering bug - https://devtalk.nvidia.com/default/topic/1068770/directx-and-direct-compute/dx11-driver-bug-significant-flickering-on-geforce/
                     clear_nrm  =(NRM_CLEAR_START && NeedBackgroundNrm()),
                     clear_ext  =(EXT_CLEAR_START && NeedBackgroundExt()),
                     clear_vel  =(VEL_CLEAR_START && _vel);
                     ASSERT(!VEL_CLEAR_START); // some of codes below clear velocity to constant value, however we always need to use "ClearDeferred" because of camera angular velocities

        _ext.get(rt_desc.type(                    IMAGERT_TWO                                                             ));
        _nrm.get(rt_desc.type(D.highPrecNrmRT() ? IMAGERT_RGB_A2_H : (D.signedNrmRT() ? IMAGERT_RGB_A2_S : IMAGERT_RGB_A2))); // here Alpha is used for #PIXEL_SHADE_MODE

         if(!merged_clear)
         { // clear from last to first to minimize RT changes
            if(clear_vel)_vel->clearViewport(D.signedVelRT() ? SVEL_CLEAR : VEL_CLEAR);
            if(clear_ext)_ext->clearViewport(                               EXT_CLEAR);
            if(clear_nrm)_nrm->clearViewport(D.signedNrmRT() ? SNRM_CLEAR : NRM_CLEAR);
            if(clear_col)_col->clearViewport();
         }

         set(_col, _nrm, _ext, _vel, _ds, true);

         if(merged_clear)
         {
            if(D._view_main.full)
            {
               if(clear_col)D.clearCol(0, Vec4Zero);
               if(clear_nrm)D.clearCol(1, D.signedNrmRT() ? SNRM_CLEAR : NRM_CLEAR);
               if(clear_ext)D.clearCol(2,                                EXT_CLEAR);
               if(clear_vel)D.clearCol(3, D.signedVelRT() ? SVEL_CLEAR : VEL_CLEAR);
            }else
            if(clear_col || clear_nrm || clear_ext || clear_vel)Sh.ClearDeferred->draw();
         }
         if(clear_ds){temporalCheck(); D.clearDS();} // 'temporalCheck' and 'D.clearDS' are paired to make sure 'temporalCheck' is called only once
      }break;

      case RT_FORWARD:
      {
        _clear=((clear_col?1:0) | (clear_ds?2:0)); // set '_clear' to be checked later
         // don't set RT or clear here, because it's very likely we will draw shadows first, before drawing to '_col', this will avoid setting useless '_col' which is very important for Mobile platforms with tile-based deferred renderers, instead we will check this in 'setForwardCol'
      }break;
   }
}
void RendererClass::setForwardCol()
{
   set(_col, _ds, true);
   if(_clear) // need to clear something
   {
      if(_clear&1) D.clearCol(combine ? TRANSPARENT : Color(clear_color.r, clear_color.g, clear_color.b, 0));
      if(_clear&2){D.clearDS (); temporalCheck();} // 'temporalCheck' and 'D.clearDS' are paired to make sure 'temporalCheck' is called only once
     _clear=0; // cleared
   }
}
void RendererClass::aoApply()
{
   if(hasAO())
   {
      ao(); if(_ao)
      {
         set(_col, _ds, true); // restore rendering RT's after calculating AO
         D .alpha(ALPHA_MUL);
         D .depth2DOn(); // this enables depth but disables 'D.depthWrite'
         Sh.ImgX[0]->set(_ao);
         Sh.DrawX->draw();
         D .depth2DOff();
      }
   }
}
void RendererClass::opaque()
{
   switch(_cur_type)
   {
      case RT_DEFERRED:
      {
         D.stencil(STENCIL_ALWAYS_SET, 0); D.set3D(); D.depth(true); mode(RM_OPAQUE);
         REPS(_eye, _eye_num)
         {
            setEyeViewportCam();
            DrawOpaqueInstances(); _render();
         }
         ClearOpaqueInstances();
         D.stencil(STENCIL_NONE); D.set2D();

         resolveDepth();
      }break;

      case RT_FORWARD:
      {
         // find initial light
         Int first_light=-1;
         if(Lights.elms() && (D.aoAll() || !hasAO())) // if we do "AO && !aoAll" then first we need to draw without lights (ambient only)
         {
            // lights are already sorted, and 0-th light is the most significant, its shadow map must be rendered last to be used by BLEND_LIGHT which happens at a later stage
            Int skip_light=-1;
            if(Lights[0].type==LIGHT_DIR && Lights[0].shadow) // 0-th is LightDir (which BlendLight only supports) and needs shadows
               for(Int i=Lights.elms(); --i>=1; )if(Lights[i].shadow) // if at least one different light needs shadows
                  {skip_light=0; break;} // it means we can't start with 0-th, because drawing another light will overwrite shadow map, so skip #0 when choosing the 'first_light'

            // find the best light
            Flt cost=FLT_MAX, view_rect_area=D.viewRect().area(); Dbl frustum_volume=Frustum.volume();
            REPA(Lights)if(i!=skip_light)
            {
               Flt c=Lights[i].firstLightCost(view_rect_area, frustum_volume); if(c<cost){cost=c; first_light=i;} // find light with smallest cost
            }
         }

         // draw main light
         ALPHA_MODE alpha=ALPHA_NONE; // #RTOutput
         if(first_light>=0)
         {
            Lights[first_light].drawForward(alpha);
         }else // no light
         {
            setForwardCol();
           _frst_light_offset=OFFSET(FRST, none);
            D.alpha(alpha);
            D.stencil(STENCIL_ALWAYS_SET, 0); D.set3D(); D.depth(true); mode(RM_OPAQUE);
            REPS(_eye, _eye_num)
            {
               setEyeViewportCam();
               DrawOpaqueInstances(); _render();
            }
            ClearOpaqueInstances();
            D.stencil(STENCIL_NONE); D.set2D();

            resolveDepth();
         }

         // apply ambient occlusion
         if(!D.aoAll())aoApply();

         // draw rest of the lights
         if(Lights.elms()-(first_light>=0)>0)
         {
           _first_pass=false;
            Bool clip=D._clip, clip_allow=D._clip_allow; T._clip=(clip ? D._clip_rect : D.rect()); // remember clipping because 'drawForward' may change it
            Sh.AmbientColorNS_l->set(VecZero); Sh.FirstPass->set(false); // disable ambient lighting
            if(FUNC_DEFAULT!=FUNC_LESS_EQUAL)D.depthFunc(FUNC_LESS_EQUAL); // need to make sure we can apply lights on existing depth
            REPA(Lights)if(i!=first_light)Lights[i].drawForward(ALPHA_ADD_KEEP); // draw 0-th at the end to setup shadow maps (needed for BLEND_LIGHT), keep alpha which is glow #RTOutput
            D.clip(clip ? &T._clip : null); D.clipAllow(clip_allow);
            if(FUNC_DEFAULT!=FUNC_LESS_EQUAL)D.depthFunc(FUNC_DEFAULT);
           _first_pass=true;

            // restore settings
            D.ambientSet(); Sh.FirstPass->set(true); // restore ambient lighting
         }

         // apply ambient occlusion
         if(D.aoAll())aoApply();

         Frustum=FrustumMain; // restore frustum after it being potentially changed when drawing shadow maps or setting frustum for visible objects for lights
       //resolveDepth(); was already called for the main light
      }break;
   }
}
void RendererClass::resolveDepth()
{
   // this resolves the entire '_ds' into '_ds_1s' (by choosing Min of depth samples), and sets 'STENCIL_REF_MSAA' if needed
   if(_ds->multiSample() && _ds->depthTexture())
   {
      Bool swap_srgb=(LINEAR_GAMMA && _col->canSwapSRV()); if(swap_srgb)_col->swapSRV(); // use sRGB if possible because we're interested in visual differences
      D.alpha(ALPHA_NONE);

      // set multi-sampled '_ds' MSAA
      if(_cur_type==RT_DEFERRED                             // for     deferred set it always (needed for lighting)
      || Fog.draw || Sky.isActual() || processAlphaFinal()) // for non-deferred it will be used only for fog, sky and alpha
      {
         D.stencil(STENCIL_MSAA_SET, STENCIL_REF_MSAA);
         set(null, _ds, true);
       //if(_nrm){Sh.ImgMS[0]->set(_nrm); Sh.DetectMSNrm->draw();}else 'DetectMSNrm' generates too many MS pixels, making rendering slower, so don't use
                 {Sh.ImgMS[0]->set(_col); Sh.DetectMSCol->draw();}
      }

      // always resolve '_ds' into '_ds_1s'
      set(null, _ds_1s, true);
      D.stencil(STENCIL_ALWAYS_SET, 0); // use 'STENCIL_ALWAYS_SET' here so when down-sampling depth, we clear the entire stencil mask for '_ds_1s'
      D.depthFunc(FUNC_ALWAYS ); D.depthLock  (true); Sh.ResolveDepth->draw();
      D.depthFunc(FUNC_DEFAULT); D.depthUnlock(    );

      // set 1-sampled '_ds_1s' MSAA
      if(_cur_type==RT_DEFERRED                     // for     deferred set it always (needed for lighting)
      || processAlphaFinal() || ms_samples_color.a) // for non-deferred it will be used only for alpha and visualizing multi-samples
      {
         D.stencilRef(STENCIL_REF_MSAA);
       //if(_nrm){Sh.ImgMS[0]->set(_nrm); Sh.DetectMSNrm->draw();}else 'DetectMSNrm' generates too many MS pixels, making rendering slower, so don't use
                 {Sh.ImgMS[0]->set(_col); Sh.DetectMSCol->draw();}
      }

      D.stencil(STENCIL_NONE);
      if(swap_srgb)_col->swapSRV(); // restore
   }
}
void RendererClass::resolveDepth1()
{
   if(_ds->multiSample() && _ds->depthTexture())
   {
      // always resolve '_ds' into '_ds_1s'
      set(null, _ds_1s, true);
      if(FUNC_DEFAULT!=FUNC_LESS)D.depthFunc(FUNC_LESS   ); D.depthLock  (true); Sh.ResolveDepth->draw(); // this resolves secondary time so can modify only pixels that are closer than before
      if(FUNC_DEFAULT!=FUNC_LESS)D.depthFunc(FUNC_DEFAULT); D.depthUnlock(    );
   }
}
void RendererClass::overlay()
{
   D.stencilRef(STENCIL_REF_TERRAIN); // set in case draw codes will use stencil

   set(_col, D.bumpMode()>BUMP_FLAT ? _nrm() : null, _ext, null, _ds, true, WANT_DEPTH_READ); // #RTOutput
   setDSLookup(); // 'setDSLookup' after 'set'
   D.alpha(ALPHA_OVERLAY);
   D.set3D(); D.depthOnWriteFunc(true, false, FUNC_LESS_EQUAL); D.depthBias(BIAS_OVERLAY); mode(RM_OVERLAY); // overlay requires BIAS because we may use 'MeshOverlay' which generates triangles by clipping existing ones
   REPS(_eye, _eye_num)
   {
      setEyeViewportCam();
      DrawOverlayObjects(); _render();
   }
   D.set2D(); D.depthOnWriteFunc(false, true, FUNC_DEFAULT); D.depthBias(BIAS_ZERO);

   D.stencil(STENCIL_NONE); // disable any stencil that might have been enabled
   OverlayObjects.clear();
}
void RendererClass::waterPreLight()
{
   Water._use_secondary_rt=(!Water.max1Light()
                          && canReadDepth()
                          && D._max_rt>=2 // col+nrm #RTOutput
                          && _cur_type!=RT_FORWARD); // for forward for the moment we can't do it, because all lights have already been applied, but in current mode we expect opaques to be drawn (so we have depth set because we copy it, and stencil set because we swap DS to preserve it and restore later)
   if(Water._use_secondary_rt)Water.drawSurfaces(); // if we use secondary RT's then we need to draw water surfaces before we calculate lights (otherwise setup lights first and then draw surfaces having shadow-maps known)
}
inline Shader* AmbientOcclusion::get(Int quality, Bool jitter, Bool normal)
{
   Shader* &s=AO[quality][jitter][normal]; if(!s)s=ShaderFiles("Ambient Occlusion")->get(S8+"AO"+quality+jitter+normal);
   return   s;
}
void RendererClass::ao()
{
   D.alpha(ALPHA_NONE);
   Bool use_nrm=(D.ambientNormal() && _nrm);
   Shader *ao=AO.get(D.ambientMode()-1, D.ambientJitter(), use_nrm);
   VecI2 res=ByteScaleRes(fx(), D._amb_res);
   ImageRTDesc rt_desc(res.x, res.y, IMAGERT_ONE); _ao.get(rt_desc);

   // always downsample and linearize at the same time
   ImageRTPtr ao_depth;
   ao_depth.get(rt_desc.type(IMAGERT_F32)); // don't try to reduce to IMAGERT_F16 because it can create artifacts on big view ranges under certain angles (especially when we don't use normal maps, like in forward renderer)
   linearizeDepth(*ao_depth, *_ds_1s);

   if(use_nrm)
   {
      Sh.imgSize    (*_nrm); // for AO ImgSize has full original size (used for normal) and RTSize of AO size
      Sh.Img[0]->set( _nrm);
   }
   Sh.Depth->set(ao_depth);
   Bool foreground=_ao->compatible(*_ds_1s);
   if(_col->multiSample())foreground&=Sky.isActual(); // when having multi-sampling, then allow this optimization only if we're rendering Sky, this is related to smooth edges between opaque and sky pixels
   if(stage)if(stage==RS_AO || stage==RS_LIGHT_AO)foreground=false; // if we will display AO then set fully

   if(foreground)D.depth2DOn(); // this enables depth but disables 'D.depthWrite'
   ImageRT *depth=(foreground ? _ds_1s() : null);
   set(_ao, depth, true, NEED_DEPTH_READ); // use DS for 'D.depth2D'
   REPS(_eye, _eye_num)
   {
      Sh.ImgClamp->setConditional(ImgClamp(_stereo ? D._view_eye_rect[_eye] : D.viewRect(), ao_depth->size()));
      ao->draw(setEyeParams()); // calculate occlusion
   }
   
   const Bool linear_depth=true; // perform blur using downsampled linear depth
   if(!linear_depth)Sh.Depth->set(_ds_1s); // restore full resolution depth
   if(D.ambientSoft()) // this needs to be in sync with 'D.shadowSoft'
   {
      rt_desc.type(IMAGERT_ONE);
    //Sh.imgSize(*_ao); we can just use 'RTSize' instead of 'ImgSize' since there's no scale
      if(D.ambientJitter())
      {
         ImageRTPtr src=_ao; _ao.get(rt_desc);
         set(_ao, depth, true, NEED_DEPTH_READ); Sh.ImgX[0]->set(src); Sh.ShdBlurJitter[0][linear_depth]->draw(); // use DS for 'D.depth2D'
      }else
      if(D.ambientSoft()<5)
      {
         ImageRTPtr src=_ao; _ao.get(rt_desc);
         set(_ao, depth, true, NEED_DEPTH_READ); Sh.ImgX[0]->set(src); Sh.ShdBlur[0][linear_depth][D.ambientSoft()-1]->draw(); // use DS for 'D.depth2D'
      }else
      {
         ImageRTPtr temp; temp.get(rt_desc);
                        set(temp, depth, true, NEED_DEPTH_READ); Sh.ImgX[0]->set( _ao); Sh.ShdBlurX[0][linear_depth]->draw(); // use DS for 'D.depth2D'
        _ao->discard(); set( _ao, depth, true, NEED_DEPTH_READ); Sh.ImgX[0]->set(temp); Sh.ShdBlurY[0][linear_depth]->draw(); // use DS for 'D.depth2D'
      }
   }
   if(linear_depth)Sh.Depth->set(_ds_1s); // restore full resolution depth
   if(foreground)D.depth2DOff();
}
void RendererClass::setAlphaFromDepth()
{
   D.alpha(ALPHA_NONE);
  _alpha.get(ImageRTDesc(_ds->w(), _ds->h(), IMAGERT_ONE, _ds->samples()));
   Bool sky=Sky.isActual(); if(sky)Sky.setFracMulAdd();
   if(_alpha->multiSample())
   {
      set(_alpha, _ds, true, NEED_DEPTH_READ);
      if(hasStencilAttached())
      {
         D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA); GetSetAlphaFromDepthMS(sky)->draw();
                         D.stencilRef(0               ); GetSetAlphaFromDepth  (sky)->draw();
         D.stencil(STENCIL_NONE     );
      }else
      {
         GetSetAlphaFromDepthMS(sky)->draw(); // we have to run all at multi-sampled frequency
      }
   }else
   {
      set(_alpha, null, true);
      GetSetAlphaFromDepth(sky)->draw();
   }
}
void RendererClass::setAlphaFromDepthAndCol()
{
   D.alpha(ALPHA_NONE);
  _alpha.get(ImageRTDesc(_ds->w(), _ds->h(), IMAGERT_ONE, _ds->samples()));
   Sh.Img[0]->set(_col);
   Bool sky=Sky.isActual(); if(sky)Sky.setFracMulAdd();
   if(_alpha->multiSample())
   {
      set(_alpha, _ds, true, NEED_DEPTH_READ);
      if(hasStencilAttached())
      {
         D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA); GetSetAlphaFromDepthAndColMS(sky)->draw();
                         D.stencilRef(0               ); GetSetAlphaFromDepthAndCol  (sky)->draw();
         D.stencil(STENCIL_NONE     );
      }else
      {
         GetSetAlphaFromDepthAndColMS(sky)->draw(); // we have to run all at multi-sampled frequency
      }
   }else
   {
      set(_alpha, null, true);
      GetSetAlphaFromDepthAndCol(sky)->draw();
   }
}
INLINE Shader* GetApplyLight(Int multi_sample, Int reflect_mode, Bool ao, Bool cel_shade, Bool night_shade, Bool glow) {Shader* &s=Sh.ApplyLight[multi_sample][reflect_mode][ao][cel_shade][night_shade][glow]; if(SLOW_SHADER_LOAD && !s)s=Sh.getApplyLight(multi_sample, reflect_mode, ao, cel_shade, night_shade, glow); return s;}
void RendererClass::light()
{
   if(processAlpha() && D.independentBlendAvailable())setAlphaFromDepth(); // setup alpha before applying lights instead of after, because after we end up with '_col' already bound, so doing this before will reduce RT changes
   if(_cur_type==RT_DEFERRED) // on other renderers light is applied when rendering opaque
   {
      /*
      -set '_ao' as Ambient Occlusion (one channel, without D.ambientColor)
      -clear '_lum', '_lum_1s', '_spec', '_spec_1s'
      -calculate screen space light (on MSAA and non-MSAA)
      -final light = sum of all buffers together
     _ao    =AO;
     _lum   = 0;     MSAA of _lum   +=light;
     _lum_1s= 0; non-MSAA of _lum_1s+=light;
      if( _ao &&  D.aoAll)LIGHT=((_lum OR _lum_1s) + D.ambientColor)*_ao;else
      if( _ao && !D.aoAll)LIGHT= (_lum OR _lum_1s) + D.ambientColor *_ao;else
      if(!_ao            )LIGHT= (_lum OR _lum_1s) + D.ambientColor     ;

      However for optimization _lum and _lum_1s can be cleared to D.ambientColor instead of 0 in these cases: #AmbientInLum
      if( _ao &&  D.aoAll)LIGHT=((_lum OR _lum_1s) + D.ambientColor)*_ao;else
      if(!_ao            )LIGHT= (_lum OR _lum_1s) + D.ambientColor     ;

      Which then becomes:
     _lum   =D.ambientColor;     MSAA of _lum   +=light;
     _lum_1s=D.ambientColor; non-MSAA of _lum_1s+=light;
      if( _ao &&  D.aoAll)LIGHT=((_lum OR _lum_1s))*_ao;else
      if(!_ao            )LIGHT= (_lum OR _lum_1s)     ;
      */

      // Ambient Occlusion
      if(hasAO())ao();

      // add dynamic lights
      DrawLights();
      getLumRT(); // get in case we still haven't initialized it

      // light buffer is ready so we can combine it with color
      Bool ao=(_ao!=null), env=(D.envMap()!=null), cel_shade=(cel_shade_palette!=null), night_shade=(D.nightShadeColorD().max()>EPS_COL8_NATIVE),
         glow=((_has&HAS_GLOW) && _col->hwTypeInfo().a); // process glow only if some object reported it and we actually have alpha channel in RT (otherwise glow could be always 1.0)
                                                         // !! AT THIS STAGE WE HAVE TO CHECK "_has&HAS_GLOW" AND NOT '_has_glow' BECAUSE THAT'S NOT SET YET !!
      ImageRTPtr src=_col; // can't read and write to the same RT
      Bool has_last_frag_color=false, // TODO: there would be no need to write to a new RT if we would use gl_LastFragColor/gl_LastFragData[0] using extensions - https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_shader_framebuffer_fetch.txt and https://www.khronos.org/registry/OpenGL/extensions/ARM/ARM_shader_framebuffer_fetch.txt
           use_last_frag_color=(has_last_frag_color && (D.highPrecColRT() ? IMAGE_PRECISION_10 : IMAGE_PRECISION_8)==D.litColRTPrecision());
      if( !use_last_frag_color)_col.get(ImageRTDesc(_col->w(), _col->h(), GetImageRTType(D.glowAllow(), D.litColRTPrecision()), _col->samples())); // glow requires alpha

      if(stage==RS_EMISSIVE)
      {
         _lum_1s->clearViewport(); if(_lum != _lum_1s)_lum ->clearViewport();
        _spec_1s->clearViewport(); if(_spec!=_spec_1s)_spec->clearViewport();
         ao=env=cel_shade=night_shade=false;
      }
      Int reflect_mode=(env ? (_has&HAS_CLEAR_COAT) ? 2 : 1 : 0);

      Sh.Img  [0]->set(_nrm                );
      Sh.Img  [1]->set( src                );
      Sh.Img  [2]->set(_lum_1s             );
      Sh.Img  [3]->set(_spec_1s            );
      Sh.Img  [4]->set( cel_shade_palette());
      Sh.ImgX [0]->set(_ao                 );
      Sh.ImgXY[0]->set(_ext                );

      D.alpha(ALPHA_NONE);
      set(_col, _ds, true, NEED_DEPTH_READ); // use DS because it may be used for 'D.depth2D' optimization and stencil tests
      if(_col==src || Sky.isActual()) // we can skip background only if we're applying to the same RT or if the background will be later overwritten by Sky
         switch(stage)
      {
         case RS_LIT_COLOR:
         case RS_GLOW     :
         case RS_EMISSIVE : break; // these stages need full background

         default: D.depth2DOn(); break;
      }
      if(!_col->multiSample())GetApplyLight(0, reflect_mode, ao, cel_shade, night_shade, glow)->draw();else
      {
         Sh.ImgMS[0]->set(_nrm );
         Sh.ImgMS[1]->set( src );
         Sh.ImgMS[2]->set(_lum );
         Sh.ImgMS[3]->set(_spec);
         Sh.ImgXYMS ->set(_ext );
         if(hasStencilAttached())
         {
            D.stencil   (STENCIL_MSAA_TEST, 0); GetApplyLight(1, reflect_mode, ao, cel_shade, night_shade, glow)->draw(); // 1 sample
            if(Sky.isActual())D.depth2DOff();                                                                             // multi-sampled always fill fully when sky will be rendered
            D.stencilRef(STENCIL_REF_MSAA    ); GetApplyLight(2, reflect_mode, ao, cel_shade, night_shade, glow)->draw(); // n samples
            D.stencil   (STENCIL_NONE        );
         }else
         {
            if(Sky.isActual())D.depth2DOff();                                         // multi-sampled always fill fully when sky will be rendered
            GetApplyLight(2, reflect_mode, ao, cel_shade, night_shade, glow)->draw(); // n samples
         }
      }
      D.depth2DOff();
      if(_lum !=_lum_1s  && (  (_has&HAS_FUR) ||   stage==RS_LIGHT || stage==RS_LIGHT_AO)){set(_lum_1s , null, true); D.alpha(ALPHA_ADD); if(ambientInLum())Sh.draw(*_lum, Vec4(1), Vec4(-D.ambientColorD(), 0));else Sh.draw(*_lum );} // need to apply multi-sampled lum  to 1-sample for fur and show light stage, if ambient is in _lum and _lum_1s RT's then it means we have to subtract ambient from '_lum' before adding to '_lum_1s', so we don't end up with 2x ambient
      if(_spec!=_spec_1s && (/*(_has&HAS_FUR) ||*/ stage==RS_LIGHT || stage==RS_LIGHT_AO)){set(_spec_1s, null, true); D.alpha(ALPHA_ADD);                                                                             Sh.draw(*_spec);} // need to apply multi-sampled spec to 1-sample for         show light stage
            src    .clear();
           _nrm    .clear();
   //_water_nrm    .clear(); we may still need it for refraction
           _ext    .clear();
           _lum    .clear(); // '_lum' will not be used after this point, however '_lum_1s' may be for rendering fur or showing light stage
           _spec   .clear();
           _spec_1s.clear();
   }
}
Bool RendererClass::waterPostLight()
{
   if(!Water._use_secondary_rt)Water.drawSurfaces();else // if we don't want to use secondary RT's
   if(_water_col) // only if we've got some water
   {
   /* -we can always do soft, because '_use_secondary_rt' is enabled only if we can read from depth buffer
      -we need to read from both depth buffers to perform softing, so in order to modify depth, we need to do this after in another operation
      -when doing refraction, we need to have a copy of '_col' and apply to '_col' (this is better for multi-sampling because copy can be 1-sampled)
          or alternatively apply to a new RT using existing '_col' (however the new RT would have to be multi-sampled for MS case)
      -when not doing refraction, we don't need any copy or separate RT, we can just apply to existing '_col' using alpha blending without reading from it, however doing this would prevent from applying glow, so don't do it
      -we can't use stencil optimizations, because:
         -when applying to MS RT the DS is MS and does not have any information about water
         -otherwise we apply to another RT and we have to write all pixels
   */
      getWaterLumRT(); // get in case we still haven't initialized it
      Bool refract=(Water.refract>EPS_MATERIAL_BUMP);

      ImageRTPtr src=_col;
      Bool depth_test;
      if(  depth_test=src->multiSample()) // multi-sampled textures can't be sampled smoothly as there will be artifacts, so resolve them, also in this case we render back to '_col' (which is not set to a new RT because we have a copy of it in 'src') but only to pixels with depth FUNC_LESS, this solves the problem of AA with water
      {  // convert to 1 sample
         ImageRTPtr temp(ImageRTDesc(src->w(), src->h(), GetImageRTType(src->type())));
         src->copyMs(*temp, false, false, D.viewRect()); Swap(src, temp);
         D.depthLock (true); // we need depth testing
         D.depthWrite(false); // disable depth writing because we can't read and write to same DS
         if(FUNC_DEFAULT!=FUNC_LESS)D.depthFunc(FUNC_LESS); // process only pixels that are closer (which means water on top of existing opaque)
      }
      if(_col==src)_col.get(ImageRTDesc(_col->w(), _col->h(), GetImageRTType(_col->type()), _col->samples())); // can't read and write to same RT, in multi-sampling we're writing back to '_col' as it's not changed

      SetOneMatrix(); // needed for refraction
      set(_col, _ds, true, NEED_DEPTH_READ); // we need depth read because we always need to read depth buffer for softing, but still use '_ds' in case we apply to existing '_col'
      D.alpha(ALPHA_NONE);

      Water.set();
      Water.setImages(src, _water_ds);
      Sh.Img[0]->set(_water_nrm ); // 'Img0' required by shader 'GetNormal', 'GetNormalMS' functions
      Sh.Img[3]->set(_water_col );
      Sh.Img[4]->set(_water_lum );
      Sh.Img[5]->set(_water_spec);
      Shader *shader=WS.Apply[depth_test][Water._shader_reflect_env][Water._shader_reflect_mirror][refract]; // we need to output depth only if we need it for depth testing
      REPS(_eye, _eye_num)
      {
         Water.setEyeViewportCam();
         shader->draw();
      }
      if(depth_test)
      {
         D.depthUnlock();
         D.depthWrite (true);
         if(FUNC_DEFAULT!=FUNC_LESS)D.depthFunc(FUNC_DEFAULT);
      }
      Water.endImages();

      // now we have to modify the depth buffer
      if((!Water._swapped_ds || !swapDS1S(_water_ds)) && Sh.SetDepth) // if we haven't swapped before, or swap back failed, then we have to apply '_water_ds' on top of existing '_ds_1s', otherwise we just swap back '_water_ds' because it had the stencil values
      {
         set(null, _ds_1s, true);
         if(FUNC_DEFAULT!=FUNC_LESS)D.depthFunc(FUNC_LESS   ); D.depthLock(true); Sh.Depth->set(_water_ds); Sh.SetDepth->draw(); Sh.Depth->set(_ds_1s); // FUNC_LESS to modify only those that are closer
         if(FUNC_DEFAULT!=FUNC_LESS)D.depthFunc(FUNC_DEFAULT); D.depthUnlock();
      }
      if(_ds!=_ds_1s && Sh.SetDepth) // multi-sample
      {
         set(null, _ds, true);
         if(FUNC_DEFAULT!=FUNC_LESS)D.depthFunc(FUNC_LESS   ); D.depthLock(true); Sh.Depth->set(_water_ds); Sh.SetDepth->draw(); Sh.Depth->set(_ds_1s); // FUNC_LESS to modify only those that are closer
         if(FUNC_DEFAULT!=FUNC_LESS)D.depthFunc(FUNC_DEFAULT); D.depthUnlock();
      }

      if(stage)switch(stage)
      {
         case RS_WATER_COLOR : if(show(_water_col, true                  ))return true; break;
         case RS_WATER_NORMAL: if(show(_water_nrm, false, D.signedNrmRT()))return true; break;
         case RS_WATER_LIGHT : if(show(_water_lum, true                  ))return true; break;
      }
   }
  _water_col .clear();
  _water_nrm .clear();
  _water_ds  .clear();
  _water_lum .clear();
  _water_spec.clear();
  _mirror_rt .clear();
   return false;
}
void RendererClass::emissive()
{
#if SUPPORT_EMISSIVE
   set(_col, _ds, true);
   D.alpha(ALPHA_ADD);
   D.set3D(); D.depthOnWriteFunc(true, false, FUNC_LESS_EQUAL); mode(RM_EMISSIVE); // need to make sure we can apply meshes on existing depth
   SortEmissiveInstances();
   REPS(_eye, _eye_num)
   {
      setEyeViewportCam();
      DrawEmissiveInstances(); _render();
   }
   ClearEmissiveInstances();
   D.set2D(); D.depthOnWriteFunc(false, true, FUNC_DEFAULT);
#endif
}
void RendererClass::sky()
{
   Fog.Draw(false);
   Sky.draw();
   if(!mirror())AstroDraw();
   Clouds.drawAll();
      Fog.Draw(true);
}
void RendererClass::waterUnder()
{
   if(Water._under_mtrl && canReadDepth() && !fastCombine())
   {
      WS.load();
    C WaterMtrl &under=*Water._under_mtrl;

      Flt under_step=Sat(Water._under_step);

      set(_col, null, true);
      D .alpha(ALPHA_RENDER_BLEND);
      Sh.Step->set(Time.time());
      under.set();
      WS.WaterPlanePos          ->set(Water._under_plane.pos   *CamMatrixInv      );
      WS.WaterPlaneNrm          ->set(Water._under_plane.normal*CamMatrixInv.orn());
      WS.WaterUnderStep         ->set(under_step);
      WS.Water_color_underwater0->set(under.color_underwater0);
      WS.Water_color_underwater1->set(under.color_underwater1);
      REPS(_eye, _eye_num)WS.Under->draw(setEyeParams());
   }
}
void RendererClass::temporal(ImageRTPtr &dilated_motion) // !! assumes 'resolveMultiSample' was already called !!
{
/* Don't 'downSample' (Super-Sampling) because quality will suffer

   by default 1 previous frame of color RT is used that continously gets updated with new data, simplified:
      old=Lerp(old, cur, 1.0/8) which is make 'old' look more like 'cur'
   So even after 100 frames, it still contains some tiny amount of data from the first frame

   'D.temporalDualHistory' works by using 2 color RT history, both get updated like this:
   old =Lerp(old , cur, 1.0/8)
   old1=Lerp(old1, cur, 1.0/8)
   and weight is increased (weight+=1/8) to know how much data we have for those RT's
   once both RT's get full, then we can completely discard 'old', treat it as empty, and use data from 'old1' (this actually works by moving 'old1' into 'old': "old=old1" and treating 'old1' as empty, by settings its weight=0)
   This way we're sure that the RT's contain only 8 last frames of data.

   FIXME: TODO: Warning: there might be an issue if using the same context, and rendering to multiple viewports, and then suddenly changing 'processAlphaFinal' in some of them
   */
   ImageRTDesc rt_desc(_col->w(), _col->h(), IMAGERT_ONE); if(D.temporalSuperRes())rt_desc.size*=2;
   Bool        alpha=processAlphaFinal(),
        merged_alpha=(!TEMPORAL_SEPARATE_ALPHA && alpha),
      separate_alpha=( TEMPORAL_SEPARATE_ALPHA && alpha);
   if(!_ctx->new_data) // doesn't have new RT's yet
   {
     _ctx->new_data.get(rt_desc.type(merged_alpha ? IMAGERT_TWO : IMAGERT_ONE)); // merged_alpha ? (X=alpha, Y=flicker) : (X=flicker); !! STORE ALPHA IN X SO IT CAN BE USED FOR '_alpha' RT !!
     _ctx->new_col .get(rt_desc.type(GetImageRTType(D.glowAllow(), D.litColRTPrecision()))); // #RTOutput
   }
   if(separate_alpha)
   {
      if(!_ctx->new_alpha) // doesn't have new RT's yet
      {
        _ctx->new_alpha.get(rt_desc.type(IMAGERT_ONE));
      }
   }
   if(_temporal_reset // RT's just got created or rendering to new sub-context (viewport)
#if TEMPORAL_SEPARATE_ALPHA
   || alpha && !_ctx->old_alpha // want alpha but doesn't have old
#else
   || (_ctx->old_data->typeChannels()>=2)!=merged_alpha // format doesn't match
#endif
   )
   {
      REPS(_eye, _eye_num)
      {
       C Rect &rect=(_stereo ? D._view_eye_rect[_eye] : D.viewRect());
        _col->copyHw(*_ctx->new_col, false, rect);
         if(merged_alpha)
         {
            if(_alpha)_alpha->copyHw(*_ctx->new_data, false, rect);else _ctx->new_data->clearViewport();
         }else
         {
                                                                                             _ctx->new_data ->clearViewport();
            if(_ctx->new_alpha){if(_alpha)_alpha->copyHw(*_ctx->new_alpha, false, rect);else _ctx->new_alpha->clearViewport();}
         }
      }
   }else
   {
      if(merged_alpha)
      {
         Sh.ImgXY[1]->set(_ctx->old_data ); // old data with alpha
      }else
      {
         Sh.ImgX [1]->set(_ctx->old_data ); // old data
         Sh.ImgX [2]->set(_ctx->old_alpha); // old alpha
      }
         Sh.Img  [0]->set(_col           ); // cur
         Sh.ImgX [0]->set(_alpha         ); // cur alpha
         Sh.Img  [1]->set(_ctx->old_col  ); // old
         Sh.ImgXY[0]->set(_vel           ); //         motion
         Sh.Img  [2]->set( dilated_motion); // dilated motion

      Sh.imgSize(*_col); // this is needed for Cubic Sampler and SUPER_RES

      /* CS is slightly slower than PS version, so don't use
      ShaderFile &sf=*ShaderFiles("Temporal"); fixme
      Bool gamma=true; fixme
      ComputeShader *cs=sf.computeFind(S8+"Temporal"+D.temporalAntiAlias()+D._view_main.full+alpha+gamma); fixme
      if(D.temporalSuperRes() && cs && _ctx->new_col->hasUAV() && (!_ctx->new_data || _ctx->new_data->hasUAV()) && (!_ctx->new_alpha || _ctx->new_alpha->hasUAV()))
      {
         Sh.rtSize       (*_ctx->new_col);
         Sh.RWImg    ->set(_ctx->new_col);
         Sh.RWImgXY  ->set(_ctx->new_data);
         Sh.RWImgX[0]->set(_ctx->new_data);
         Sh.RWImgX[1]->set(_ctx->new_alpha);
         REPS(_eye, _eye_num)
         {
            RectI viewport=ScreenToPixelI(_stereo ? D._view_eye_rect[_eye] : D.viewRect(), _col->size());
            Sh.ImgClamp->setConditional(ImgClamp(viewport, _col->size()));
            GetShaderParamInt("ViewportRectI")->setConditional(viewport); fixme
            cs->compute(DivCeil16(viewport.size()));
         }
      }else*/
      {
         D.alpha(ALPHA_NONE);
      #if TEMPORAL_SEPARATE_SUPER_RES_OLD_WEIGHT
         ImageRTPtr old_weight;
         if(D.temporalSuperRes()) // this is used only for SuperRes because these calculations are the same for full-pixel and sub-pixel, so for super-res it's more efficient to calculate this only once per pixel instead of doing the same calculations for each 4 sub-pixels per pixel. But for no super-res don't do this, because that would slow things down due to overhead of writing to RT and reading from it.
         {
            rt_desc.type(IMAGERT_ONE).size=_col->size(); old_weight.get(rt_desc);
            set(old_weight, null, true);
            Shader *shader=Sh.TemporalOldWeight[D._view_main.full];
            REPS(_eye, _eye_num)
            {
               Sh.ImgClamp->setConditional(ImgClamp(_stereo ? D._view_eye_rect[_eye] : D.viewRect(), _col->size()));
               shader->draw(_stereo ? &D._view_eye_rect[_eye] : null);
            }
            Sh.ImgX[3]->set(old_weight);
         }
      #endif
         set(_ctx->new_col, _ctx->new_data, _ctx->new_alpha, null, null, true);
         Shader *shader=Sh.Temporal[D.temporalAntiAlias()+2*D.temporalSuperRes()-1][D._view_main.full][alpha];
         REPS(_eye, _eye_num)
         {
            Sh.ImgClamp->setConditional(ImgClamp(_stereo ? D._view_eye_rect[_eye] : D.viewRect(), _col->size()));
            shader->draw(_stereo ? &D._view_eye_rect[_eye] : null);
         }
      }
   }

  _col=_ctx->new_col;
   if(alpha)_alpha=(TEMPORAL_SEPARATE_ALPHA ? _ctx->new_alpha : _ctx->new_data); // !! Warning: for TEMPORAL_SEPARATE_ALPHA=0 '_alpha' may point to multi-channel "Alpha, Flicker", this assumes that '_alpha' will not be further modified, we can't do the same for '_col' because we may modify it !!

   temporalFinish();
}
void RendererClass::temporalFinish()
{
   if(_temporal_use) // hasTemporal()
   {
     _temporal_use=false; D._view_active.setShader(); SetProjMatrix();
   }
}
void RendererClass::edgeDetect()
{
   if(D.edgeDetect() && !mirror() && canReadDepth())
   {
      Sky.setFracMulAdd();
      switch(D.edgeDetect()) // here can't use 'D.depth2DOn' because we want to affect sky
      {
         case EDGE_DETECT_THIN:
         {
            D.alpha(ALPHA_MUL); set(_col, _ds, true, NEED_DEPTH_READ); Sh.imgSize(*_ds_1s); Sh.EdgeDetect->draw();
         }break;

         case EDGE_DETECT_SOFT:
         {
            ImageRTPtr edge(ImageRTDesc(fxW(), fxH(), IMAGERT_ONE));
            D.alpha(ALPHA_NONE); set(edge, null, true, NEED_DEPTH_READ); Sh.imgSize(*_ds_1s); Sh.EdgeDetect->draw(); // we need to fill the entire buffer because below we're using blurring (which takes nearby texels)
            D.alpha(ALPHA_MUL ); set(_col, _ds , true,   NO_DEPTH_READ); Sh.imgSize(* edge ); Sh.ImgX[0]->set(edge); Sh.EdgeDetectApply->draw();
         }break;
      }
   }
}
void RendererClass::blend()
{
   Sky.setFracMulAdd();

   // set main light parameters for *BLEND_LIGHT* and 'Mesh.drawBlend'
   if(Lights.elms() && Lights[0].type==LIGHT_DIR) // use 0 index as it has already been set in 'UpdateLights'
   {
      Lights[0].dir.set();
     _blst_light_offset=OFFSET(BLST, dir[Lights[0].shadow ? D.shadowMapNumActual() : 0]);
   }else
   {
      LightDir(Vec(0, -1, 0), VecZero).set(); // set dummy light
     _blst_light_offset=OFFSET(BLST, dir[0]);
   }

   // apply light in case of drawing fur, which samples the light buffer
   if(_has&HAS_FUR)
   {
      if(_ao)
      {
         set(_lum_1s, null, true);
         Sh.ImgX[0]->set(_ao);
         if(D.aoAll())
         {
            D.alpha(ALPHA_MUL);
            Sh.DrawX->draw();
         }else
         {
            D.alpha(ALPHA_ADD);
            Sh.Color[0]->set(Vec4(D.ambientColorD(), 0));
            Sh.Color[1]->set(Vec4Zero                  );
            Sh.DrawXC[0][0]->draw();
         }
      }
      PrepareFur();
   }
  _ao.clear(); // '_ao' will not be used after this point

   D.stencilRef(STENCIL_REF_TERRAIN); // set in case draw codes will use stencil

   const Bool blend_affect_vel=true;
   ALPHA_MODE alpha=(Renderer.fastCombine() ? ALPHA_BLEND : ALPHA_RENDER_BLEND);
   set(_col, _alpha, blend_affect_vel ? _vel() : null, null, _ds, true); setDSLookup(); // 'setDSLookup' after 'set' #RTOutput.Blend, needed for 'DrawBlendInstances'
   D.set3D(); D.depthOnWriteFunc(true, false, FUNC_LESS_EQUAL); mode(RM_BLEND); // use LESS_EQUAL for blend because we may want to draw blend graphics on top of existing pixels (for example world editor terrain highlight)
   SortBlendInstances();
   REPS(_eye, _eye_num)
   {
      setEyeViewportCam();
   #if 1
      D.alpha(alpha); _render(); DrawBlendInstances(); // first call '_render' to for example get 'getBackBuffer' and then draw objects in 'DrawBlendInstances'
   #else
      DrawBlendInstances(); D.alpha(alpha); _render();
   #endif
   }
   ClearBlendInstances();
  _SetHighlight(TRANSPARENT);
   D.set2D(); D.depthOnWriteFunc(false, true, FUNC_DEFAULT);

   D.stencil(STENCIL_NONE); // disable any stencil that might have been enabled

  _lum_1s.clear(); // '_lum_1s' will not be used after this point
}
void RendererClass::palette(Int index)
{
   if(D.colorPaletteAllow())
      if(C ImagePtr &palette=D._color_palette[index])
   {
      ImageRT &ds=(_ds_1s ? *_ds_1s : *_ds); // Warning: this will disable applying palette only on terrain using STENCIL_REF_TERRAIN for multisampling
      Sky.setFracMulAdd();

      ImageRTPtr intensity(ImageRTDesc(_col->w(), _col->h(), D.highPrecPalRT() ? IMAGERT_RGBA_H : IMAGERT_RGBA, ds.samples())); // we need to match depth multi-sampling, here Alpha is used for 4th palette channel
      D.stencilRef(STENCIL_REF_TERRAIN); // set in case draw codes will use stencil

      set(intensity, &ds, true, WANT_DEPTH_READ); setDSLookup(); // we need depth-testing, but want depth-read for particle softing, 'setDSLookup' after 'set'
      D.clearCol();
      D.alpha(ALPHA_ADD);
      D.set3D(); D.depthOnWriteFunc(true, false, FUNC_LESS_EQUAL); mode(index ? RM_PALETTE1 : RM_PALETTE);
      REPS(_eye, _eye_num)
      {
         setEyeViewportCam();
        _render(); if(index)DrawPalette1Objects();else DrawPaletteObjects();
      }
      D.set2D(); D.depthOnWriteFunc(false, true, FUNC_DEFAULT);

      D.stencil(STENCIL_NONE); // disable any stencil that might have been enabled

      set(_col, _alpha, null, null, null, true); // #RTOutput.Blend
      D .alpha(ALPHA_RENDER_BLEND);
      Sh.Img[1]->set(palette());
      Sh.PaletteDraw->draw(intensity);
   }
   if(index)
   {
      Palette1Objects.clear();
      Palette1Areas  .clear();
   }else
   {
      PaletteObjects.clear();
      PaletteAreas  .clear();
   }
}
void RendererClass::behind()
{
   if(canReadDepth())
   {
      Sky.setFracMulAdd();

      set(_col, _ds, true, NEED_DEPTH_READ); // we will read from the depth buffer
      D.alpha(ALPHA_RENDER_BLEND);
      D.set3D(); D.depthOnWriteFunc(true, false, FUNC_GREATER); mode(RM_BEHIND);
      REPS(_eye, _eye_num)
      {
         setEyeViewportCam();
        _render(); DrawBehindObjects();
      }
      D.set2D(); D.depthOnWriteFunc(false, true, FUNC_DEFAULT);
   }
   BehindObjects.clear();
}
static const IMAGERT_TYPE IMAGERT_OUTLINE=IMAGERT_SRGBA; // here Alpha is used for outline opacity
void RendererClass::setOutline(C Color &color)
{
  _SetHighlight(color);
   if(!_outline) // not initialized at all
   {
     _outline_rt.get(ImageRTDesc(_col->w(), _col->h(), IMAGERT_OUTLINE, _col->samples())); // here Alpha is used for outline opacity
      set(_outline_rt, _ds, true);
      D.clearCol        ();
      D.alpha           (ALPHA_NONE);
      D.depthOnWriteFunc(true, false, FUNC_LESS_EQUAL);
      if(D.outlineMode()==EDGE_DETECT_THIN)D.stencil(STENCIL_OUTLINE_SET, STENCIL_REF_OUTLINE);
   }
   Int           outline_eye=(1<<_eye);
   if(!(_outline&outline_eye)) // not yet enabled for this eye
   {
     _outline|=outline_eye; // enable
      setEyeViewportCam(); // set viewport if needed
   }
}
void RendererClass::applyOutline()
{
   if(_outline_rt)
   {
     _SetHighlight(TRANSPARENT); // disable 'SetHighlight' which was called during mesh drawing
      D.depthOnWriteFunc(false, true, FUNC_DEFAULT); // restore default

      resolveMultiSample(); // don't do 'downSample' here because 'edgeSoften' and 'temporal' will be called later and they require to operate on full-sampled data
      ImageRT *ds=_ds_1s; // we've resolved multi-sample so have to use 1-sample

      if(!Sh.Outline)
      {
         Sh.Outline     =Sh.get("Outline00");
         Sh.OutlineDS   =Sh.get("Outline10");
         Sh.OutlineClip =Sh.get("Outline01");
         Sh.OutlineApply=Sh.get("OutlineApply");
      }

      Sh.imgSize(*_outline_rt);
      switch(D.outlineMode())
      {
         case EDGE_DETECT_THIN: if(Sh.OutlineClip)
         {
            set(_col, (ds && ds->compatible(*_col)) ? ds : null, true);
            D.depth2DOn (); // this enables depth but disables 'D.depthWrite'
            D.stencil   ((_cur_ds==_ds) ? STENCIL_OUTLINE_TEST : STENCIL_NONE); // we can use the stencil optimization only if we will use the DS to which we've written stencil to
            D.alpha     (ALPHA_RENDER_BLEND);
            Sh.OutlineClip->draw(_outline_rt);
            D.stencil   (STENCIL_NONE);
            D.depth2DOff();
         }break;

         case EDGE_DETECT_SOFT: if(Sh.Outline && Sh.OutlineDS && Sh.OutlineApply)
         {
            ImageRTPtr temp(ImageRTDesc(fxW(), fxH(), IMAGERT_OUTLINE)); // here Alpha is used for outline opacity
            set(temp, null, true);
            D.alpha(ALPHA_NONE);
            ((temp->w()<_outline_rt->w()) ? Sh.OutlineDS : Sh.Outline)->draw(_outline_rt);
            set(_col, (ds && ds->compatible(*_col)) ? ds : null, true);
            D .alpha    (ALPHA_RENDER_BLEND);
          //D .depth2DOn(); can't enable because we need to affect sky
            Sh.imgSize(*temp); Sh.OutlineApply->draw(temp);
          //D.depth2DOff();
         }break;
      }
     _outline_rt.clear();
   }
}
void RendererClass::outline()
{
   // start outline
   if(D.outlineMode())
   {
      mode(RM_OUTLINE);
      REPS(_eye, _eye_num)
      {
       //setEyeViewportCam(); viewport is set in 'setOutline' method
         DrawOutlineObjects(); _render();
      }
   }
   OutlineObjects.clear();
}
void RendererClass::resolveMultiSample() // !! assumes that 'finalizeGlow' was called !! this should be called before 'downSample'
{
   if(_col->multiSample())
   {
      ImageRTPtr src=_col; _col.get(ImageRTDesc(_col->w(), _col->h(), GetImageRTType(_has_glow, D.litColRTPrecision())));
      src->copyMs(*_col, false, true, D.viewRect());
   }
}
void RendererClass::downSample() // !! assumes that 'finalizeGlow' was called !!
{
   resolveMultiSample();
   if(_col->w()>_final->w()) // if down-sample is needed
   {
      ImageRTPtr src=_col; _col.get(ImageRTDesc(_final->w(), _final->h(), GetImageRTType(_has_glow, D.litColRTPrecision())));
      src->copyHw(*_col, false, D.viewRect());
   }
}
void RendererClass::edgeSoften() // !! assumes that 'finalizeGlow' was called !!
{
   if(hasEdgeSoften())
   {
      resolveMultiSample();
      D.alpha(ALPHA_NONE);
      if(!D._view_main.full)
      {
         const Int pixel_range=6; // currently FXAA/SMAA shaders use this range
         set(_col, null, false); // need full viewport
         D.viewRect().drawBorder(Vec4Zero, pixelToScreenSize(pixel_range)); // draw black border around the viewport to clear and prevent from potential artifacts on viewport edges
      }
      ImageRTDesc rt_desc(_col->w(), _col->h(), GetImageRTType(_has_glow, D.litColRTPrecision()));
      ImageRTPtr  dest(rt_desc);
      // D.depth2DOn/depth2DOff can't be applied here, this was tested and resulted in loss of softening at object/sky edges
    //Sh.imgSize(*_col); we can just use 'RTSize' instead of 'ImgSize' since there's no scale
      switch(D.edgeSoften())
      {
         case EDGE_SOFTEN_FXAA:
         {
            // there's no need for separate in_gamma/out_gamma because they will be the same since src/dest types are the same
            Bool gamma=LINEAR_GAMMA, swap_srgb=(gamma && _col->canSwapSRV() && dest->canSwapRTV()); if(swap_srgb){gamma=false; dest->swapRTV(); _col->swapSRV();} // if we have a non-sRGB access, then just use it instead of doing the more expensive shader, later we have to restore it
            set(dest, null, true); Sh.FXAA[gamma]->draw(_col);
            if(swap_srgb){dest->swapRTV(); _col->swapSRV();} // restore
         }break;

      #if SUPPORT_MLAA
         case EDGE_SOFTEN_MLAA:
         {
           _col->copyHw(*dest, false, D.viewRect());
            D.stencil(STENCIL_EDGE_SOFT_SET, STENCIL_REF_EDGE_SOFT); // have to use '_ds_1s' in write mode to be able to use stencil
            ImageRTPtr edge (rt_desc.type(IMAGERT_TWO )); set(edge (), _ds_1s(), true); D.clearCol(); Sh.MLAAEdge ->draw(_col ()); Sh.Img[1]->set(_mlaa_area); D.stencil(STENCIL_EDGE_SOFT_TEST);
            ImageRTPtr blend(rt_desc.type(IMAGERT_RGBA)); set(blend(), _ds_1s(), true); D.clearCol(); Sh.MLAABlend->draw( edge()); Sh.Img[1]->set( blend  ()); edge.clear();
                                                          set(dest (), _ds_1s(), true);               Sh.MLAA     ->draw(_col ());                             D.stencil(STENCIL_NONE          );
         }break;
      #endif

         case EDGE_SOFTEN_SMAA:
         {
          C auto &src_ds=_ds_1s;
            ImageRTPtr ds; if(_col->compatible(*src_ds) && src_ds->hwTypeInfo().s)ds=src_ds;else ds.find(ImageRTDesc(_col->w(), _col->h(), IMAGERT_DS_S, _col->samples()));
            // there's no need for separate in_gamma/out_gamma because they will be the same since src/dest types are the same
            Bool gamma=LINEAR_GAMMA, swap_srgb=(gamma && _col->canSwapSRV()); if(swap_srgb){gamma=false; _col->swapSRV();} // if we have a non-sRGB access, then just use it instead of doing the more expensive shader, later we have to restore it
            D.stencil(STENCIL_EDGE_SOFT_SET, STENCIL_REF_EDGE_SOFT); // have to use 'ds' in write mode to be able to use stencil
            ImageRTPtr edge(rt_desc.type(IMAGERT_TWO)); set(edge, ds, true); D.clearCol(); if(ds!=src_ds)D.clearDepth(); Sh.SMAAEdge[gamma]->draw(_col); Sh.Img[1]->set(_smaa_area); Sh.Img[2]->set(_smaa_search); D.stencil(STENCIL_EDGE_SOFT_TEST);
            if(swap_srgb)_col->swapSRV(); // restore

            ImageRTPtr blend(rt_desc.type(IMAGERT_RGBA)); // this does not store color, but intensities how much to blend in each axis
            set(blend, ds, true); D.clearCol(); Sh.SMAABlend->draw(edge); Sh.Img[1]->set(blend); edge.clear(); D.stencil(STENCIL_NONE);

            swap_srgb=(!LINEAR_GAMMA && dest->canSwapRTV() && _col->canSwapSRV()); if(swap_srgb){dest->swapRTV(); _col->swapSRV();} // this we have to perform if we're NOT using Linear Gamma, because if possible, we WANT to use it, as it will improve quality, making AA softer
            set(dest, null, true); Sh.SMAA->draw(_col);
            if(swap_srgb){dest->swapRTV(); _col->swapSRV();} // restore
         }break;
      }
      Swap(dest, _col);
   }
}
void RendererClass::volumetric()
{
   if(_vol)
   {
      set(_col, null, true);
      SPSet("VolMax", Vec(D.volMax()));
      Sh.imgSize(*_vol);
      D.alpha(D.volAdd() ? ALPHA_ADD      : ALPHA_RENDER_BLEND);
             (D.volAdd() ? VL.VolumetricA : VL.Volumetric)->draw(_vol);
     _vol.clear();
   }
}
static Bool ColConst() {return Renderer._col==Renderer._ctx->new_col;} // if '_col' points to 'new_col' then we can't modify it
void RendererClass::postProcess()
{
   resolveMultiSample(); // we need to resolve the MS Image so it's 1-sample for effects
   if(ms_samples_color.a && D.multiSample())
   {
      D.alpha(ALPHA_BLEND);
      D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
      set(_col, _ds_1s, true);
      D.viewRect().draw(ms_samples_color, true);
      D.stencil(STENCIL_NONE);
   }

   // !! WARNING: EFFECTS SHOULD NEVER MODIFY '_col' IF 'ColConst' !!
   // !! WARNING: 'temporal' might output '_col' that points to 'new_col', in that case it can't be modified because it's needed for the next frame !!
   // !! so if any effect wants to modify '_col' instead of drawing to another RT, it first must check it for 'ColConst' !!
   Bool temporal =_temporal_use       , // hasTemporal()
        adapt_eye= hasEyeAdapt      (),
        motion   = hasMotion        (),
        bloom    =(hasBloom         () || _has_glow || D.toneMap()),
        alpha    = processAlphaFinal(), // this is always enabled for 'slowCombine'
        dof      = hasDof           (),
        sharpen  = D.sharpen        (),
        combine  = slowCombine      (),
        alpha_set= fastCombine      (); // if alpha channel is set properly in the RT, skip this if we're doing 'fastCombine' because we're rendering to existing RT which has its Alpha already set

   if(alpha){if(!_alpha)setAlphaFromDepthAndCol();} // create '_alpha' if not yet available
   else          _alpha.clear(); // make sure to clear if we don't use it

   ImageRTPtr dest, bloom_glow, dilated_full_motion, dilated_blur_motion;
   if(dilateMotion(temporal ? &dilated_full_motion : null, motion ? &dilated_blur_motion : null))return;
   if(stage==RS_MOTION && show(_vel, false, D.signedVelRT()))return; // velocity could've been used for MotionBlur or Temporal, check it after 'dilateMotion' because it might generate it for MOTION_CAMERA

   if(temporal)
   {
      T.temporal(dilated_full_motion); // !! AFTER 'temporal' we must be checking for 'ColConst' !!
                 dilated_full_motion.clear(); // no longer needed
   }

   if(D.temporalSuperRes())edgeSoften(); // if have temporal super-res then 'edgeSoften' must be run after temporal upscale

   ImageRTDesc rt_desc(fxW(), fxH(), IMAGERT_SRGBA/*this is changed later*/);
   Bool upscale=(_final->w()>_col->w() || _final->h()>_col->h()); // we're going to upscale at the end, this needs to be set after 'temporal' which might upscale '_col'
   Int  fxs=(_get_target ? -1 : adapt_eye+motion+bloom+alpha+dof+upscale+sharpen); // this counter specifies how many effects are still left in the queue, and if we can render directly to '_final'
   Sh.ImgClamp->setConditional(ImgClamp(rt_desc.size)); // set 'ImgClamp' that may be needed for Bloom, DoF, MotionBlur, this is the viewport rect within texture, so reading will be clamped to what was rendered inside the viewport

   ImageRT *exposure=null;
   if(adapt_eye)
   {
      --fxs;
      if(bloom)exposure=adaptEye(*_col, null);else // if there will be bloom then don't apply now, but this will be done later in bloom
      {
         if(!fxs)dest=_final;else dest.get(rt_desc.type(GetImageRTType(_has_glow, D.litColRTPrecision()))); // can't read and write to the same RT, glow requires Alpha channel
         adaptEye(*_col, dest); Swap(_col, dest); // Eye Adaptation keeps Alpha
      }
   }

   rt_desc.type(GetImageRTType(alpha, D.litColRTPrecision())); // we need alpha channel after this stage only if we use alpha

   if(motion) // tests have shown that it's better to do Motion Blur before Depth of Field
   {
      downSample(); // motion blur shader works by taking a lot of samples from src, so it's better to downsample first to make cache work faster. This will be a little slower if there's no movement on the screen (branch path using just 1 sample), but will be a lot faster when there's movement on the screen (branch path using many samples)
      if(_alpha && !_has_glow) // if we have separate alpha image, and not using glow, then it's better to put alpha image to alpha channel to avoid doing multiple texture reads in the motion blur shader #AlphaGlow
      {
         Sh.ImgX[0]->set(_alpha);
         if(_col->hwTypeInfo().a>=8 && !ColConst()) // if '_col' already has alpha-channel, then just reuse it and replace that channel
         {
            set(_col, null, true); D.alpha(ALPHA_KEEP_SET);
            GetReplaceAlpha()->draw();
         }else
         {
            Sh.Img[0]->set(_col);
            dest.get(rt_desc);
            set(dest, null, true); D.alpha(ALPHA_NONE);
            GetCombineAlpha()->draw();
           _col=dest;
         }
      }
      if(_alpha)fxs--; // motion blur always merges '_alpha' so remove it from list
      if(!--fxs)dest=_final;else dest.get(rt_desc); // can't read and write to the same RT
      motionBlur(*_col, *dest, bloom_glow, dilated_blur_motion, alpha, combine, exposure); Swap(_col, dest); alpha_set=true; // Motion Blur sets Alpha
     _alpha.clear(); // got merged so clear it
      dilated_blur_motion.clear(); // no longer needed
   }
  _vel.clear(); // velocity could've been used for MotionBlur or Temporal, but after this stage we no longer need it

   if(bloom) // bloom needs to be done before DoF because of per-pixel glow
   {
      if(_alpha)fxs--; // bloom always merges '_alpha' so remove it from list
      if(!--fxs)dest=_final;else dest.get(rt_desc); // can't read and write to the same RT
      T.bloom(*_col, *dest, bloom_glow, alpha, combine, exposure); Swap(_col, dest); alpha_set=true; // Bloom sets Alpha
     _alpha.clear(); // got merged so clear it
   }

   if(_alpha) // merge '_col' with '_alpha'
   {
      // TODO: when using multi-sampling and 'D.particlesSoft' then there can be outlines around objects touching particles, this is because Particle soft shader uses TexDepthPoint
      Sh.Img [0]->set(_col  );
      Sh.ImgX[0]->set(_alpha);
      if(!--fxs)
      { // directly into '_final'
         set(_final, null, true); D.alpha(combine ? ALPHA_MERGE : ALPHA_NONE);
         GetCombineAlpha()->draw();
        _col=_final;
      }else
      if(_col->hwTypeInfo().a>=8 && !ColConst()) // if '_col' already has alpha-channel, then just reuse it and replace that channel
      {
         set(_col, null, true); D.alpha(ALPHA_KEEP_SET);
         GetReplaceAlpha()->draw();
      }else
      {
         dest.get(rt_desc);
         set(dest, null, true); D.alpha(ALPHA_NONE);
         GetCombineAlpha()->draw();
        _col=dest;
      }
      alpha_set=true; // Combine sets Alpha
     _alpha.clear(); // got merged so clear it
   }

   if(dof) // after Motion Blur
   {
      if(!--fxs)dest=_final;else dest.get(rt_desc); // can't read and write to the same RT
      T.dof(*_col, *dest, alpha, combine); Swap(_col, dest); alpha_set=true; // DoF sets Alpha
   }
   /*if(tone_map)
   {
      if(!--fxs)dest=_final;else dest.get(rt_desc); // can't read and write to the same RT
      toneMap(*_col, *dest, alpha, combine); Swap(_col, dest); alpha_set=true; // ToneMap sets Alpha
   }*/
   if(upscale)
   {
      if(!--fxs)dest=_final;else
      {
         rt_desc.size=_final->size();
         dest.get(rt_desc);
      }
      Bool    dither=(D.dither() && !dest->highPrecision());
      Bool     gamma=LINEAR_GAMMA, swap_srgb=false;
      Int     pixels=1+1; // 1 for filtering + 1 for borders (because source is smaller and may not cover the entire range for dest, for example in dest we want 100 pixels, but 1 source pixel covers 30 dest pixels, so we may get only 3 source pixels covering 90 dest pixels)
      Shader *shader=null;

      switch(D.densityFilter())
      {
         case FILTER_NONE:
         {
            pixels=1; // 1 for borders
            SamplerPoint.setPS(SSI_DEFAULT_2D);
         }break;

         case FILTER_CUBIC_FAST       :
         case FILTER_CUBIC_FAST_SMOOTH:
         case FILTER_CUBIC_FAST_SHARP :
            pixels=2+1; // 2 for filtering + 1 for borders
            Sh.imgSize(*_col); shader=Sh.DrawTexCubicFastF[alpha][dither]; // this doesn't need to check for "_col->highPrecision" because resizing and cubic filtering generates smooth values
         break;

         case FILTER_CUBIC_PLUS      :
         case FILTER_CUBIC_PLUS_SHARP:
            swap_srgb=(gamma && _col->canSwapSRV() && dest->canSwapRTV()); if(swap_srgb){gamma=false; _col->swapSRV(); dest->swapRTV();} // do just one gamma instead of in/out, to avoid having to do expensive gamma conversion in the shader
            pixels=3+1; // 3 for filtering + 1 for borders
            Sh.imgSize(*_col); Sh.loadCubicShaders(); shader=Sh.DrawTexCubicPlusF[alpha][dither][gamma]; // this doesn't need to check for "_col->highPrecision" because resizing and cubic filtering generates smooth values
         break;

         case FILTER_BEST :
         case FILTER_WAIFU: // fall back to best available shader (EASU)
         case FILTER_EASU :
         {
            swap_srgb=(gamma && _col->canSwapSRV() && dest->canSwapRTV()); if(swap_srgb){gamma=false; _col->swapSRV(); dest->swapRTV();} // do just one gamma instead of in/out, to avoid having to do expensive gamma conversion in the shader
            shader=Sh.EASU[alpha][dither][gamma];
            pixels=2+1; // 2 for filtering + 1 for borders
            SetEASU(*_col, *dest);

      /* rt_desc.size=dest->size();
         if(!D.canSwapSRGB())rt_desc.type(GetImageRTTypeLinear(alpha, D.litColRTPrecision())); // if can't swap sRGB then have to use linear type because UAV's don't support sRGB writes
         dest.get(rt_desc);
         ComputeShader *cs=ShaderFiles("FSR")->computeFind("Upscale");
         if(cs && dest->hasUAV() && Kb.ctrl())
         {
            support !full viewports
            set(null, null, false); is this still needed? // make sure '_col' is not bound as RT as it would prevent it from being used as src image, so just set 'dest' which is going to be used later to minimize state changes
            _col->swapSRGB();
            dest->swapSRGB();
            Sh.RWImg->set(dest);
            Sh.Img[0]->set(_col);
            cs->compute(VecI2(DivCeil16(rt_desc.size.x), DivCeil16(rt_desc.size.y)));
            dest->swapSRGB();
            _col->swapSRGB();
            Swap(_col, dest);*/
         }break;
      }
      if(!shader)shader=Sh.Draw[alpha][dither];
      if(!D._view_main.full)
      {
         set(_col, null, false); D.alpha(ALPHA_NONE); // need full viewport
         D.viewRect().drawBorder(Vec4Zero, pixelToScreenSize(pixels)); // draw black border around the viewport to clear and prevent from potential artifacts on viewport edges
      }
      set(dest, null, true); D.alpha((combine && dest()==_final) ? ALPHA_MERGE : ALPHA_NONE);
      shader->draw(_col);
   #if 0
      if( in_swap_srgb)_col->swapSRV();
      if(out_swap_srgb)dest->swapRTV();
   #else
      if(swap_srgb){_col->swapSRV(); dest->swapRTV();}
   #endif
      Swap(_col, dest); alpha_set=true;
      if(D.densityFilter()==FILTER_NONE)SamplerLinearClamp.setPS(SSI_DEFAULT_2D);
   }
   if(sharpen)
   {
      rt_desc.size=_col->size(); // RCAS operates on same size only
      if(!--fxs && _final->size()==rt_desc.size)dest=_final;else dest.get(rt_desc);
   #if 0
      Bool  in_gamma=LINEAR_GAMMA,  in_swap_srgb=( in_gamma && _col->canSwapSRV()); if( in_swap_srgb){ in_gamma=false; _col->swapSRV();}
      Bool out_gamma=LINEAR_GAMMA, out_swap_srgb=(out_gamma && dest->canSwapRTV()); if(out_swap_srgb){out_gamma=false; dest->swapRTV();}
   #else
      Bool gamma=LINEAR_GAMMA, swap_srgb=(gamma && _col->canSwapSRV() && dest->canSwapRTV()); if(swap_srgb){gamma=false; _col->swapSRV(); dest->swapRTV();} // do just one gamma instead of in/out, to avoid having to do expensive gamma conversion in the shader
   #endif
      Bool dither=(D.dither() && !dest->highPrecision());
      set(dest, null, true); D.alpha((combine && dest()==_final) ? ALPHA_MERGE : ALPHA_NONE);

   #if GL
      Vec2 mul_add; if(D.mainFBO())mul_add.set(-1, dest->h());else mul_add.set(1, 0); // use "dest->h()" and not "dest->h()-1" because this operates on Flt pixels with 0.5 offset
      RcasMulAdd->set(mul_add);
   #endif

    /*if(Kb.shift())
      {
         struct CAS
         {
            varAU4(const0);
            varAU4(const1);
         }cas;
         CasSetup(cas.const0, cas.const1,
                         0.5f,  // Sharpness tuning knob (0.0 to 1.0)
         _col->w(), _col->h(),  // input size
         dest->w(), dest->h()); // output size

         GetShaderParam("Cas")->set(cas);
         Sh.get(S8+"CAS"+alpha+dither+gamma+gamma)->draw(_col);
         D.text(0,0,S+Ms.pos().x);
      }else*/
      Sh.RCAS[alpha][dither][gamma]->draw(_col);

   #if 0
      if( in_swap_srgb)_col->swapSRV();
      if(out_swap_srgb)dest->swapRTV();
   #else
      if(swap_srgb){_col->swapSRV(); dest->swapRTV();}
   #endif
      Swap(_col, dest); alpha_set=true;
   }
   if(!_get_target) // for '_get_target' leave the '_col' result for further processing
   {
      if(_col!=_final)
      {
      #if DEBUG
         static Bool shown=false; if(!shown){shown=true; OSMsgBox("Warning", S+"Rendering Performance penalty.\n'_col' in most cases should already be applied onto '_final'.\nThis is possible to happen and it's just a DEBUG warning which can be ignored, however it could also be a sign of problem with 'fxs' calculation.\n" __FILE__ "\"\nLine: "+__LINE__);}
      #endif
         if(!D._view_main.full)
         {
            Int pixels=1+1; // 1 for filtering + 1 for borders (because source is smaller and may not cover the entire range for dest, for example in dest we want 100 pixels, but 1 source pixel covers 30 dest pixels, so we may get only 3 source pixels covering 90 dest pixels)
            set(_col, null, false); D.alpha(ALPHA_NONE); // need full viewport
            D.viewRect().drawBorder(Vec4Zero, pixelToScreenSize(pixels)); // draw black border around the viewport to clear and prevent from potential artifacts on viewport edges
         }
         set(_final, null, true); D.alpha(combine ? ALPHA_MERGE : ALPHA_NONE);
         Sh.Draw[alpha][D.dither() && !_final->highPrecision() && (_col->highPrecision() || _col->size()!=_final->size())]->draw(_col); // allow dithering only if the source has high precision, or if we're resizing (because that generates high precision too)
         alpha_set=true;
      }
     _col.clear(); // release as it's no longer needed
      if(!alpha_set && _back==_final) // if we need to have alpha channel set for back buffer effect
      {
         set(_final, null, true);
         D.alpha(ALPHA_KEEP_SET);
         Sh.clear(Vec4(0, 0, 0, 1)); // force full alpha so back buffer effects can work ok
      }
   }
}
/******************************************************************************/
void SetFov(Vec2 &fov, FOV_MODE fov_mode, Flt aspect)
{
   if(FovPerspective(fov_mode))
   {
      switch(fov_mode)
      {
         case FOV_X:
            Clamp(fov.x, EPS, PI-EPS);
            fov.y=2*Atan(Tan(fov.x*0.5f)/aspect);
         break;

         case FOV_Y:
            Clamp(fov.y, EPS, PI-EPS);
            fov.x=2*Atan(Tan(fov.y*0.5f)*aspect);
         break;

         case FOV_XY:
            Clamp(fov.x, EPS, PI-EPS);
            Clamp(fov.y, EPS, PI-EPS);
         break;
      }
   }else
   {
      switch(fov_mode)
      {
       /*case FOV_ORTHO:
            fov.x=fov.x;
            fov.y=fov.y;
         break;*/

         case FOV_ORTHO_X:
          //fov.x=fov.x;
            fov.y=fov.x/aspect;
         break;

         case FOV_ORTHO_Y:
          //fov.y=fov.y;
            fov.x=fov.y*aspect;
         break;
      }
   }
}
void RenderIcon(void (&render)(), C ViewSettings *view, ImageRTC &image, C VecI2 &image_size, Int super_sample)
{
   if(image_size.x>0 && image_size.y>0)
   {
      Int shift=((super_sample<=1) ? 0 : Log2Ceil(Unsigned(super_sample)));
      if(image.create(image_size, IMAGE_R8G8B8A8_SRGB))
      {
         ImageRTC temp;
      again:
         if(shift)
            if(!temp.create(VecI2(Min(image_size.x<<shift, D.maxTexSize()),
                                  Min(image_size.y<<shift, D.maxTexSize())), IMAGE_R8G8B8A8_SRGB)){shift--; goto again;}

         SyncLocker lock(D._lock);

         auto        density=D.densityFast        (); D.densityFast1();
         auto        amb_res=D.ambientResFast     (); D.ambientResFast1();
         auto     max_lights=D.maxLights          (); D.maxLights(0);
         auto      eye_adapt=D.eyeAdaptation      (); D.eyeAdaptation(false);
         auto     lod_factor=D.lodFactor          (); D.lodFactor(0);
         auto         motion=D.motionMode         (); D.motionMode(MOTION_NONE);
         auto allow_temporal=Renderer.allow_temporal; Renderer.allow_temporal=false;

         DisplayClass::ViewportSettings cur_view; cur_view.get();
       //Bool                           view_force_square=D._view_square_pixel; D.viewForceSquarePixel(true); not needed because FOV_XY is used
         Vec2 fov_xy=(view ? view->fov : D.viewFov()); SetFov(fov_xy, view ? view->fov_mode : D.viewFovMode(), image.aspect());
         D.view(D.rect(), view ? view->from : D.viewFrom(), view ? view->range : D.viewRange(), fov_xy, FOV_XY);

         Renderer.target=(shift ? &temp : &image);
         Renderer.alpha =true;
         Renderer(render);
         Renderer.alpha =false;
         Renderer.target=null;

         D.motionMode(motion).maxLights(max_lights).eyeAdaptation(eye_adapt).lodFactor(lod_factor); D.densityFast(density); D.ambientResFast(amb_res);
         Renderer.allow_temporal=allow_temporal;

         if(shift)
         {
            ImageRT *rt[Elms(Renderer._cur)], *ds;
            REPAO(rt)=Renderer._cur[i];
                  ds =Renderer._cur_ds;

            ImageRT *cur=&temp, temp1;
            if(shift>1)
            {
               ImageRT *next=&temp1;
               for(;;)
               {
                  if(!next->create(cur->size()/2, cur->type()))break;
                  cur->copyHw(*next, false);
                  Swap(cur, next);
                  if(--shift<=1)break;
               }
            }
            cur->copyHw(image, false);

            Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, false); // don't restore viewport, because we reset viewport later below
         }

       //D.viewForceSquarePixel(view_force_square);
         cur_view.set();
         return;
      }
   }
   image.del();
}
void RenderIcon(void (&render)(), C ViewSettings *view, Image &image, C VecI2 &image_size, Int super_sample)
{
   ImageRTC temp; Image &temp_image=temp;
   Swap(image, temp_image); // Swap to try to reuse existing 'image' if it was already created #SwapImageRT
   RenderIcon(render, view, temp, image_size, super_sample);
   Swap(image, temp_image);
}
/******************************************************************************/
}
/******************************************************************************/
