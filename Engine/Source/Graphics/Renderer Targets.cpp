/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   #RTOutput
   RT           : Width    , Height   ,                                                                                           Type           , Samples  , Comments
  _main         : D.   resW, D.   resH,                                                                                           IMAGERT_SRGBA  , 1        , COLOR RGB, Opacity
  _main_ds      : D.   resW, D.   resH,                                                                                           IMAGERT_DS     , 1        , this is the Main DepthStencil buffer to be used together with '_main' RT, on OpenGL (except iOS) it is provided by the system
  _ds           : D.renderW, D.renderW,                                                                                           IMAGERT_DS     , D.samples
  _ds_1s        : D.renderW, D.renderW,                                                                                           IMAGERT_DS     , 1        , if '_ds' is Multi-Sampled then this is created as a standalone 1-sampled depth buffer, otherwise it's a duplicate of '_ds'
  _col          : D.renderW, D.renderH,                                                      D.highPrecColRT ? IMAGERT_SRGBA_P  : IMAGERT_SRGBA  , D.samples, COLOR RGB, GLOW
  _nrm          : D.renderW, D.renderH,                D.highPrecNrmRT ? IMAGERT_RGB_A2_H : (D.signedNrmRT   ? IMAGERT_RGB_A2_S : IMAGERT_RGB_A2), D.samples, NRM   XYZ, #PIXEL_SHADE_MODE
  _ext          : D.renderW, D.renderH,                                                                                           IMAGERT_TWO    , D.samples, ROUGH    , REFLECT
  _vel          : D.renderW, D.renderH,                                                                                           IMAGERT_TWO_H  , D.samples, 2D MOTION (UV delta from CUR_POS to PREV_FRAME_POS)
  _alpha        : D.renderW, D.renderH,                                                                                           IMAGERT_ONE    , D.samples, OPACITY
  _lum          : D.renderW, D.renderH,                                                        D.highPrecLumRT ? IMAGERT_SRGB_H : IMAGERT_SRGB   , D.samples, LIGHT RGB
  _lum_1s       : D.renderW, D.renderH,                                                        D.highPrecLumRT ? IMAGERT_SRGB_H : IMAGERT_SRGB   , 1        , LIGHT RGB. if '_lum' is Multi-Sampled then this is created as a standalone 1-sampled depth buffer, otherwise it's a duplicate of '_lum'
  _spec         : D.renderW, D.renderH,                                                        D.highPrecLumRT ? IMAGERT_SRGB_H : IMAGERT_SRGB   , D.samples, LIGHT SPEC RGB
  _spec_1s      : D.renderW, D.renderH,                                                        D.highPrecLumRT ? IMAGERT_SRGB_H : IMAGERT_SRGB   , 1        , LIGHT SPEC RGB. if '_spec' is Multi-Sampled then this is created as a standalone 1-sampled depth buffer, otherwise it's a duplicate of '_spec'

  _water_col    : D.renderW, D.renderH,                                                                                           IMAGERT_SRGB   , 1        , COLOR   RGB
  _water_nrm    : D.renderW, D.renderH,                                                           D.signedNrmRT ? IMAGERT_RGB_S : IMAGERT_RGB    , 1        , NRM     XYZ
  _water_refract: D.renderW, D.renderH,                                                                                           IMAGERT_TWO_S  , 1        , REFRACT XY
   there's no _water_ext #WaterExt
  _water_ds     : D.renderW, D.renderH,                                                                                           IMAGERT_DS     , 1        , Water   Depth
  _water_lum    : D.renderW, D.renderH,                                                                                           IMAGERT_SRGB   , 1        , LIGHT   RGB
  _water_spec   : D.renderW, D.renderH,                                                                                           IMAGERT_SRGB   , 1        , LIGHT   SPEC RGB

   '_gui' is set to '_main', unless stereoscopic rendering is enabled then it's set to VR RT

   If using Renderer.combine or Renderer.alpha (Renderer.processAlpha && D.independentBlendAvailable) #RTOutput.Blend then after all opaque meshes are drawn:
     -'_alpha' RT is created and set as RT1
     -ALPHA_MODE states such as ALPHA_RENDER_BLEND ALPHA_RENDER_BLEND_FACTOR are set to Increase RT1
     -all alpha-blended effects output alpha into RT2:
         -Mesh 3D Shaders (Blend, BlendLight)
         -Particles
         -Palette apply
         -post process effects operate on alpha as well (EyeAdapt, Bloom, MotionBlur, DoF)

   If '_ds' is multi-sampled on DX10+ then:
      In Deferred Renderer:
         -'_ds_1s' is set to down-sampled copy of '_ds'
         -both '_ds' and '_ds_1s' have STENCIL_REF_MSAA set
      In Non-Deferred Renderer:
         -if "processAlpha || ms_samples_color.a"                      then '_ds_1s' has STENCIL_REF_MSAA set
         -if "Fog.draw || Sky.isActual || Atmospheres || processAlpha" then '_ds'    has STENCIL_REF_MSAA set

   In OpenGL (except iOS):
      '_main' and '_main_ds' don't have '_rb' and '_txtr' set, because they're provided by the system and not created by the engine.
      This means that when setting '_main' it's always paired with '_main_ds' depth buffer, and '_main_ds' can't be read as a depth texture.

   In OpenGL:
      '_main' and '_main_ds' are flipped vertically when compared to other render targets.

/******************************************************************************/
void RendererClass::createShadowMap()
{
   SyncLocker locker(D._lock);

   // shadow maps
   D._shd_map_size_actual=Max(0, Min(D.shadowMapSize()*3, D.maxTexSize())/3);
   VecI2 shd_map_size(D.shadowMapSizeActual()*2,
                      D.shadowMapSizeActual()*3);
   if(!_shd_map.create(shd_map_size, IMAGE_D32  , IMAGE_SHADOW_MAP)) // D32 shadow maps have no performance penalty (tested on GeForce 650m) so use them if possible
   if(!_shd_map.create(shd_map_size, IMAGE_D24X8, IMAGE_SHADOW_MAP)) // we don't need stencil so avoid it in case it causes performance penalty
   if(!_shd_map.create(shd_map_size, IMAGE_D24S8, IMAGE_SHADOW_MAP))
       _shd_map.create(shd_map_size, IMAGE_D16  , IMAGE_SHADOW_MAP);
   if(!_shd_map.is())D._shd_map_size_actual=0;

   // cloud shadow maps
   VecI2 cld_map_size(D.cloudsMapSize()*2,
                      D.cloudsMapSize()*3);
   if(!_cld_map.create(cld_map_size, IMAGE_R8      , IMAGE_RT))
   if(!_cld_map.create(cld_map_size, IMAGE_R8G8    , IMAGE_RT))
       _cld_map.create(cld_map_size, IMAGE_R8G8B8A8, IMAGE_RT);

   Sh.connectRT();
   D.shadowJitterSet();
}
void RendererClass::rtClear()
{
  _h0           .clear();
  _h1           .clear();
  _q0           .clear();
  _q1           .clear();
  _col          .clear();
  _nrm          .clear();
  _ext          .clear();
  _vel          .clear();
  _alpha        .clear();
  _lum          .clear();
  _lum_1s       .clear();
  _spec         .clear();
  _spec_1s      .clear();
  _shd_1s       .clear();
  _shd_ms       .clear();
  _ds           .clear();
  _ds_1s        .clear();
  _water_col    .clear();
  _water_nrm    .clear();
  _water_refract.clear();
  _water_ds     .clear();
  _water_lum    .clear();
  _water_spec   .clear();
  _vol          .clear();
  _ao           .clear();
  _mirror_rt    .clear();
  _outline_rt   .clear();
  _final        =null;
   D.temporalReset();
   // don't clear '_back' and '_back_ds' here in case they are used
}
void RendererClass::rtClean()
{
   SyncLocker locker(D._lock);
   rtClear();
   REPA(_rts)if(_rts[i].available())_rts.removeValid(i);
}
void RendererClass::rtDel()
{
   SyncLocker locker(D._lock);

#if GL
   if(FBO) // detach all render targets
   {
      D.fbo(FBO); // set custom frame buffer
      glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D  , 0, 0);
      glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 , GL_TEXTURE_2D  , 0, 0);
      glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2 , GL_TEXTURE_2D  , 0, 0);
      glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3 , GL_TEXTURE_2D  , 0, 0);
      glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_TEXTURE_2D  , 0, 0);
      glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D  , 0, 0);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_RENDERBUFFER, 0   );
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0   );
   #if !IOS
    //D.fbo(0); // set default frame buffer (iOS doesn't have it) !! Don't do this, because we clear all attachments and '_cur' to null, we assume that no RT's are set, however that's possible only in a custom FBO, so we need to keep it, so that the next 'set' RT change will be able to properly detect the change !!
   #endif
   }
#endif

   rtClear();
   D.clearFade(); // _fade.clear(); this is already cleared in 'clearFade'
  _back   .clear();
  _back_ds.clear();

  _ui   =_cur_main   =_ptr_main   =&_main;
  _ui_ds=_cur_main_ds=_ptr_main_ds=&_main_ds;

   unmapMain();
#if DX11 || IOS // only on these platforms we're creating custom '_main_ds', on other platforms the system creates it, so we're not deleting (to keep the info about IMAGE_TYPE and samples)
  _main_ds.del();
#endif
  _cld_map.del();
  _shd_map.del();
   REPAO(_eye_adapt_scale).del();
  _rts.clear();
   REPAO(_cur       )=null; _cur_ds   =null;
   REPAO(_cur_id    )=NULL; _cur_ds_id=NULL;
   REPAO(_cur_ds_ids)=NULL;
}
Bool RendererClass::rtCreateMain() // !! call only under lock !!
{
   ImageRT *old=_ptr_main, *old_ds=_ptr_main_ds,
           *cur_ds=Renderer._cur_ds, *cur[ELMS(Renderer._cur)]; REPAO(cur)=Renderer._cur[i]; // remember these before creating/deleting RT's because when doing that, 'Renderer._cur', 'Renderer._cur_ds' might get cleared to null

   Bool secondary=(D.colorManaged() || WEB), ok=true; // need to create secondary main if we perform color management, or for WEB sRGB conversion #WebSRGB
   if(  secondary)
   {
      Bool high_prec=D.colorManaged(); // if we're going to convert colors then we need high precision
      if(!(high_prec && _main_temp   .create(_main.size(), IMAGE_F16_4        , IMAGE_RT, _main.samples())))
      if(!(             _main_temp   .create(_main.size(), IMAGE_R8G8B8A8_SRGB, IMAGE_RT, _main.samples())))goto error;

      if(!(             _main_temp_ds.create(_main.size(), IMAGE_D24S8        , IMAGE_DS, _main.samples())))goto error;
     _ptr_main   =&_main_temp;
     _ptr_main_ds=&_main_temp_ds;
   }else
   {
   clear:
     _main_temp   .del();
     _main_temp_ds.del();
     _ptr_main   =&_main;
     _ptr_main_ds=&_main_ds;
      D._color_lut.del(); // can't color manage without temp's
   }
   if(secondary // if created new RT's
   || old!=_ptr_main || old_ds!=_ptr_main_ds) // or changed something
   {
      // remap from old to new
               if(_ui         ==old   )_ui         =_ptr_main;
               if(_ui_ds      ==old_ds)_ui_ds      =_ptr_main_ds;
               if(_cur_main   ==old   )_cur_main   =_ptr_main;
               if(_cur_main_ds==old_ds)_cur_main_ds=_ptr_main_ds;
      REPA(cur)if( cur[i]     ==old   ) cur[i]     =_ptr_main;
               if( cur_ds     ==old_ds) cur_ds     =_ptr_main_ds;
      Renderer.set(cur[0], cur[1], cur[2], cur[3], cur_ds, true);
   }
   return ok;
error:
   ok=false; goto clear;
}
Bool RendererClass::rtCreate()
{
   if(LogInit)LogN("RendererClass.rtCreate");
   SyncLocker locker(D._lock);

   rtDel();
   ResetImageTypeCreateResult();
   if(!D.created())return true; // don't bother with render targets for APP_ALLOW_NO_GPU/APP_ALLOW_NO_XDISPLAY

   if(!mapMain())return false;

   // main depth
#if DX11
   if(!_main_ds.create(_main.size(), IMAGE_D24S8, IMAGE_DS, _main.samples()))return false;
#elif IOS // on iOS we have access to '_main' so let's keep '_main_ds' the same
   if(!_main_ds.create(_main.size(), IMAGE_D24S8, IMAGE_DS, _main.samples()))
   if(!_main_ds.create(_main.size(), IMAGE_D32  , IMAGE_DS, _main.samples()))
   if(!_main_ds.create(_main.size(), IMAGE_D24X8, IMAGE_DS, _main.samples()))
   if(!_main_ds.create(_main.size(), IMAGE_D16  , IMAGE_DS, _main.samples()))
   if(!_main_ds.create(_main.size(), IMAGE_D24S8, IMAGE_GL_RB, _main.samples()))
   if(!_main_ds.create(_main.size(), IMAGE_D32  , IMAGE_GL_RB, _main.samples()))
   if(!_main_ds.create(_main.size(), IMAGE_D24X8, IMAGE_GL_RB, _main.samples()))
   if(!_main_ds.create(_main.size(), IMAGE_D16  , IMAGE_GL_RB, _main.samples()))return false;
#else // other platforms have '_main_ds' linked with '_main' provided by the system
  _main_ds.forceInfo(_main.w(), _main.h(), 1, _main_ds.type() ? _main_ds.type() : IMAGE_D24S8, IMAGE_GL_RB, _main.samples()); // if we know the type then use it, otherwise assume the default IMAGE_D24S8
#endif

   // secondary main
   if(!rtCreateMain())return false;

   createShadowMap();

   // eye adaptation
  _eye_adapt_scale_cur=0;
   if(!(_eye_adapt_scale[0].create(1, IMAGE_F32) || _eye_adapt_scale[0].create(1, IMAGE_F16))
   || !(_eye_adapt_scale[1].create(1, IMAGE_F32) || _eye_adapt_scale[1].create(1, IMAGE_F16)))
      REPAO(_eye_adapt_scale).del(); // if any failed then delete both

   setMain();

   Sh.connectRT();
   return true;
}
/******************************************************************************/
void RendererClass::update()
{
   if(_t_measure)
   {
      Dbl t=Time.curTime();
      if( t>_t_last_measure+1)
      {
         Flt mul=1.0f/_t_measures[0];

        _t_reflection  [0]=_t_reflection  [1]*mul;            _t_reflection  [1]=0;
        _t_prepare     [0]=_t_prepare     [1]*mul;            _t_prepare     [1]=0;
        _t_opaque      [0]=_t_opaque      [1]*mul;            _t_opaque      [1]=0;
        _t_overlay     [0]=_t_overlay     [1]*mul;            _t_overlay     [1]=0;
        _t_water       [0]=_t_water       [1]*mul;            _t_water       [1]=0;
        _t_light       [0]=_t_light       [1]*mul;            _t_light       [1]=0;
        _t_emissive    [0]=_t_emissive    [1]*mul;            _t_emissive    [1]=0;
        _t_sky         [0]=_t_sky         [1]*mul;            _t_sky         [1]=0;
        _t_water_under [0]=_t_water_under [1]*mul;            _t_water_under [1]=0;
        _t_edge_detect [0]=_t_edge_detect [1]*mul;            _t_edge_detect [1]=0;
        _t_blend       [0]=_t_blend       [1]*mul;            _t_blend       [1]=0;
        _t_palette     [0]=_t_palette     [1]*mul;            _t_palette     [1]=0;
        _t_behind      [0]=_t_behind      [1]*mul;            _t_behind      [1]=0;
        _t_rays        [0]=_t_rays        [1]*mul;            _t_rays        [1]=0;
        _t_volumetric  [0]=_t_volumetric  [1]*mul;            _t_volumetric  [1]=0;
        _t_post_process[0]=_t_post_process[1]*mul;            _t_post_process[1]=0;
        _t_gpu_wait    [0]=_t_gpu_wait    [1]/_t_measures[1]; _t_gpu_wait    [1]=0; // '_t_gpu_wait' has it's own counter (_t_measures[1]) because it's called once per frame, while others can be called multiple times per frame

        _t_last_measure=t;
        _t_measures[0]=0;
        _t_measures[1]=0;
      }
   }
}
/******************************************************************************/
void RendererClass::setMain() // !! requires 'D._lock' !! this is called after RT creation, and when VR GuiTexture is created/deleted/changed, and at the end of frame drawing for stereo mode (to advance to the next VR frame)
{
#if DX12
   map needs to be called for all images, cache the values, and adjust '_main' in every frame, possibly setRT too
#endif
   if(VR.active() && (_ui=VR.getUI()))
   {
     _ui_ds=&VR._ui_ds;
   }else
   {
     _ui   =_ptr_main;
     _ui_ds=_ptr_main_ds;
   }
  _cur_main   =_ui;
  _cur_main_ds=_ui_ds;

   set(_cur_main, _cur_main_ds, false);
}
Bool RendererClass::mapMain()
{
   return _main.map();
}
void RendererClass::unmapMain()
{
  _main.unmap();
  _cur   [0]=null;
  _cur_id[0]=NULL;
}
/******************************************************************************/
void RendererClass::setPixelSize()
{
 C VecI2 &res=_res;
  _pixel_size    .set(D.w2()/res.x , D.h2()/res.y );
  _pixel_size_inv.set(res.x /D.w2(), res.y /D.h2());
}
Rect RendererClass::pixelToScreen(C RectI &pixel)
{
   return Rect(pixel.min.x*Renderer._pixel_size.x-D.w(), D.h()-pixel.max.y*Renderer._pixel_size.y,
               pixel.max.x*Renderer._pixel_size.x-D.w(), D.h()-pixel.min.y*Renderer._pixel_size.y);
}
Vec2 RendererClass::screenToPixelSize(C Vec2 &screen) {return screen*Renderer._pixel_size_inv;}
Vec2 RendererClass::pixelToScreenSize(  Flt   pixel ) {return pixel *Renderer._pixel_size    ;}
/******************************************************************************/
#if GL
static void SwitchedFBO()
{
   // update settings that depend on main FBO being active 
   D.setFrontFace(); // adjust culling according to Y axis
   SetProjMatrix (); // flip Y 3D coords when Rendering To Texture
}
static inline Bool EqualRT(C Image *a, C Image *b)
{
   UInt a_txtr, a_rb; if(a){a_txtr=a->_txtr; a_rb=a->_rb;}else a_txtr=a_rb=0;
   UInt b_txtr, b_rb; if(b){b_txtr=b->_txtr; b_rb=b->_rb;}else b_txtr=b_rb=0;
   return a_txtr==b_txtr && a_rb==b_rb;
}
static inline Bool EqualDS(C Image *a, C Image *b, UInt a_txtr)
{
   UInt         a_rb; if(a){                 a_rb=a->_rb;}else        a_rb=0;
   UInt b_txtr, b_rb; if(b){b_txtr=b->_txtr; b_rb=b->_rb;}else b_txtr=b_rb=0;
   return a_txtr==b_txtr && a_rb==b_rb;
}
static Bool EqualTxtr(C Image *a, C Image *b) {return (a ? a->_txtr : 0)==(b ? b->_txtr : 0);} // simpler version that checks texture ID's only, this can be used for #1+ RT's which never use RenderBuffers but only textures
#endif
#define R Renderer
#if DX11
void RendererClass::setDSLookup()
{
   if(R._cur_ds)
   {
      R._cur_ds_ids[  NO_DEPTH_READ]=R._cur_ds-> _dsv;
      R._cur_ds_ids[NEED_DEPTH_READ]=R._cur_ds->_rdsv;
      R._cur_ds_ids[WANT_DEPTH_READ]=R._cur_ds_ids[R._cur_ds_ids[NEED_DEPTH_READ] ? NEED_DEPTH_READ : NO_DEPTH_READ]; // use RDSV if available, if not then DSV
   }else
   {
      R._cur_ds_ids[  NO_DEPTH_READ]=null;
      R._cur_ds_ids[WANT_DEPTH_READ]=null;
      R._cur_ds_ids[NEED_DEPTH_READ]=null;
   }
}
void RendererClass::setDS(ID3D11DepthStencilView *dsv)
{
   if(R._cur_ds_id!=dsv)
   {
      if(dsv==R._cur_ds_ids[NO_DEPTH_READ])D.texClear(R._cur_ds->_srv); // if we're writing to depth then we need to unbind it from reading (because DirectX will do it)
      D3DC->OMSetRenderTargets(Elms(R._cur_id), R._cur_id, R._cur_ds_id=dsv);
   }
}
void RendererClass::needDepthTest() {if(D._depth_write || !R._cur_ds_id)setDS(R._cur_ds_ids[  NO_DEPTH_READ]);}
void RendererClass::wantDepthRead() {                                   setDS(R._cur_ds_ids[WANT_DEPTH_READ]);}
void RendererClass::needDepthRead() {                                   setDS(R._cur_ds_ids[NEED_DEPTH_READ]);}
#elif WEB
void RendererClass::setDSLookup()
{
 /*R._cur_ds_ids[WANT_DEPTH_READ]=*/R._cur_ds_ids[NO_DEPTH_READ]=(R._cur_ds ? R._cur_ds->_txtr : 0); // WANT_DEPTH_READ will always be the same as NO_DEPTH_READ so never use it
 //R._cur_ds_ids[NEED_DEPTH_READ]=NULL; this will always be null so no need to change it, this is already cleared at startup, besides it's never accessed anyway
}
void RendererClass::setDS(UInt ds_txtr_id)
{
   if(R._cur_ds_id!=ds_txtr_id)
   {
      R._cur_ds_id=ds_txtr_id;
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_TEXTURE_2D, ds_txtr_id, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, ds_txtr_id, 0); // this could fail if Image doesn't have stencil component
   }
}
void RendererClass::needDepthTest() {setDS(R._cur_ds_ids[NO_DEPTH_READ]);}
#elif DEPTH_FLUSH
void RendererClass::needDepthRead()
{
   if(_modified_depth)
   {
     _modified_depth=false;
      if(_cur_ds_id) // this means there's current "_cur_ds && _cur_ds->_txtr" DS is readable (we don't need this for other cases)
      {  // re-apply will force the flush (calling just 'glFlush' was not enough)
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_TEXTURE_2D, _cur_ds_id, 0);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, _cur_ds_id, 0);
      }
   }
}
#endif
#undef R
#define DEBUG_DISCARD 0
#if     DEBUG_DISCARD
   #pragma message("!! Warning: Use this only for debugging !!")
#endif
void RendererClass::set(ImageRT *t0, ImageRT *t1, ImageRT *t2, ImageRT *t3, ImageRT *ds, Bool custom_viewport, DEPTH_READ_MODE depth_read_mode)
{
   Bool changed=false;
#if DX11
   ID3D11RenderTargetView *id0=(t0 ? t0->_rtv : null),
                          *id1=(t1 ? t1->_rtv : null),
                          *id2=(t2 ? t2->_rtv : null),
                          *id3=(t3 ? t3->_rtv : null);
   ID3D11DepthStencilView *ids=(ds ? (depth_read_mode==NEED_DEPTH_READ
                                   || depth_read_mode==WANT_DEPTH_READ && ds->_rdsv) ? ds->_rdsv : ds->_dsv : null);

   if(_cur_id[0]!=id0 || _cur_id[1]!=id1 || _cur_id[2]!=id2 || _cur_id[3]!=id3 || _cur_ds_id!=ids)
   { // on DX11 when setting RT, it automatically unbinds it from SRVs, so have to clear it manually (UAVs are unbound too, however engine always resets them)
      if(id0 &&                  _cur_id[0]!=id0)D.texClear(t0->_srv);
      if(id1 &&                  _cur_id[1]!=id1)D.texClear(t1->_srv);
      if(id2 &&                  _cur_id[2]!=id2)D.texClear(t2->_srv);
      if(id3 &&                  _cur_id[3]!=id3)D.texClear(t3->_srv);
      if(ids && ids==ds->_dsv && _cur_ds_id!=ids)D.texClear(ds->_srv); // if we're writing to depth then we need to unbind it from reading (because DirectX will do it)

      changed=true;
     _cur[0]=t0; _cur_id[0]=id0;
     _cur[1]=t1; _cur_id[1]=id1;
     _cur[2]=t2; _cur_id[2]=id2;
     _cur[3]=t3; _cur_id[3]=id3;
     _cur_ds=ds; _cur_ds_id=ids;
      D3DC->OMSetRenderTargets(Elms(_cur_id), _cur_id, _cur_ds_id); ASSERT(ELMS(_cur_id)<=D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
   }else
   if(_cur_ds!=ds) // even if we're not changing RenderTargetView it's still possible we're changing RenderTarget Image, this can happen on DX10 when using NEED_DEPTH_READ, '_rdsv' null and last '_cur_ds_id' also null
   {
      changed=true;
     _cur_ds =ds;
   }
#elif GL
   // !! '_cur_id' is not set for GL, only '_cur_ds_id' is !!
   Image    *set_ds =((WEB && depth_read_mode==NEED_DEPTH_READ) ? null : ds); // if we require reading from the depth buffer, then we can't set it
   Bool was_main_fbo=D.mainFBO(),
            main_fbo=(t0==&_main || ds==&_main_ds), // check 'ds' and not 'set_ds' !!
          change_0, change_ds;
   if(main_fbo)
   {
      change_0 =((_cur[0]==&_main   )!=(t0==&_main   ));
      change_ds=((_cur_ds==&_main_ds)!=(ds==&_main_ds)); // check 'ds' and not 'set_ds' !!
   }else
   {
      change_0 =!EqualRT(_cur[0], t0);
      change_ds=!EqualDS(_cur_ds, set_ds, _cur_ds_id);
   }
#if DEPTH_FLUSH
   if(_modified_depth && depth_read_mode!=NO_DEPTH_READ) // if depth was modified and want to read it
   {
      _modified_depth=false;
      if(_cur_ds_id) // if actually have some depth texture
      {
         change_ds=true; // force re-apply will force the flush (calling just 'glFlush' was not enough)
      }
   }
#endif

   if(main_fbo!=was_main_fbo || change_0 || change_ds || !EqualTxtr(_cur[1], t1) || !EqualTxtr(_cur[2], t2) || !EqualTxtr(_cur[3], t3))
   {
   #if !IOS // there is no default frame buffer on iOS
      D.colWriteAllow((main_fbo && t0!=&_main) ? 0 : COL_WRITE_RGBA); // on desktop OpenGL and OpenGL ES (except iOS) '_main' is always linked with '_main_ds', when setting null RT and '_main_ds' DS, '_main' is set either way but with color writes disabled
      D.   depthAllow(!main_fbo || ds==&_main_ds); // check 'ds' and not 'set_ds' !!
      if(main_fbo)
      {
         D.fbo(0); // set default frame buffer
        _cur_ds_id=0; // main FBO always has 0 depth txtr ID
      #if !MAC
      #if WINDOWS
         if(glInvalidateFramebuffer) // requires GL 4.3, GL ES 3.0
      #endif
         {
         #if DEBUG_DISCARD
            if(_main   ._discard){_main   ._discard=false; D.clearCol(0, PURPLE);} // use indexed version to ignore the viewport
            if(_main_ds._discard){_main_ds._discard=false; D.clearDS (         );}
         #else
            // discard, for main FBO we need to setup different values - https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glInvalidateFramebuffer.xhtml
            GLenum attachment[3]; GLsizei attachments=0; // RT0+Depth+Stencil
            if(_main   ._discard){_main   ._discard=false; attachment[attachments++]=GL_COLOR;}
            if(_main_ds._discard){_main_ds._discard=false; attachment[attachments++]=GL_DEPTH; if(_main_ds.hwTypeInfo().s)attachment[attachments++]=GL_STENCIL;}
            if(attachments)glInvalidateFramebuffer(GL_FRAMEBUFFER, attachments, attachment);
         #endif
         }
      #endif
      }else
   #endif
      {
      #if !IOS // on iOS there's only one FBO and nothing else, so it is set during creation and not here
         D.fbo(FBO); // set custom frame buffer
      #endif

      #if IOS // on iOS there's only one FBO, so we can safely apply only changed RT's, otherwise we need to always set all of them
         if(change_ds)
      #endif
         {
            if(!set_ds)
            {
              _cur_ds_id=0;
               glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_RENDERBUFFER, 0);
               glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
            }else
            if(_cur_ds_id=set_ds->_txtr)
            {
               glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_TEXTURE_2D,                          _cur_ds_id    , 0);
               glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, set_ds->hwTypeInfo().s ? _cur_ds_id : 0, 0);
            }else
            {
            //_cur_ds_id=0; already set above
               glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_RENDERBUFFER,                          set_ds->_rb    );
               glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, set_ds->hwTypeInfo().s ? set_ds->_rb : 0);
            }
         #if DEPTH_FLUSH
           _modified_depth=false; // changing RTs flushed depth
         #endif
         }

      #if IOS // on iOS there's only one FBO, so we can safely apply only changed RT's, otherwise we need to always set all of them
         if(change_0)
      #endif
         {
            if(!t0       )glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,            0);else
            if( t0->_txtr)glFramebufferTexture2D   (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D  , t0->_txtr, 0);else
	                       glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, t0->_rb     );
         }

         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, t1 ? t1->_txtr : 0, 0);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, t2 ? t2->_txtr : 0, 0);
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, t3 ? t3->_txtr : 0, 0);

         GLenum buffers[]={GLenum(t0 ? GL_COLOR_ATTACHMENT0 : GL_NONE),
                           GLenum(t1 ? GL_COLOR_ATTACHMENT1 : GL_NONE),
                           GLenum(t2 ? GL_COLOR_ATTACHMENT2 : GL_NONE),
                           GLenum(t3 ? GL_COLOR_ATTACHMENT3 : GL_NONE)};
         glDrawBuffers(Elms(buffers), buffers);
         glReadBuffer (buffers[0]);

         // discard
      #if !MAC
      #if WINDOWS
         if(glInvalidateFramebuffer) // requires GL 4.3, GL ES 3.0
      #endif
         {
         #if DEBUG_DISCARD
            if(t0     &&     t0->_discard){    t0->_discard=false; _cur[0]=t0; D.clearCol(0, PURPLE);}
            if(t1     &&     t1->_discard){    t1->_discard=false; _cur[1]=t1; D.clearCol(1, PURPLE);}
            if(t2     &&     t2->_discard){    t2->_discard=false; _cur[2]=t2; D.clearCol(2, PURPLE);}
            if(t3     &&     t3->_discard){    t3->_discard=false; _cur[3]=t3; D.clearCol(3, PURPLE);}
            if(set_ds && set_ds->_discard){set_ds->_discard=false; _cur_ds=ds; D.clearDS (         );}
         #else
            GLenum attachment[ELMS(_cur)+1]; GLsizei attachments=0; // RT's+DS
            if(t0     &&     t0->_discard){    t0->_discard=false; attachment[attachments++]=GL_COLOR_ATTACHMENT0;}
            if(t1     &&     t1->_discard){    t1->_discard=false; attachment[attachments++]=GL_COLOR_ATTACHMENT1;}
            if(t2     &&     t2->_discard){    t2->_discard=false; attachment[attachments++]=GL_COLOR_ATTACHMENT2;}
            if(t3     &&     t3->_discard){    t3->_discard=false; attachment[attachments++]=GL_COLOR_ATTACHMENT3;}
            if(set_ds && set_ds->_discard){set_ds->_discard=false; attachment[attachments++]=(set_ds->hwTypeInfo().s ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT);}
            if(attachments)glInvalidateFramebuffer(GL_FRAMEBUFFER, attachments, attachment);
         #endif
         }
      #endif

      #if DEBUG
         GLenum error =glCheckFramebufferStatus(GL_FRAMEBUFFER);
         if(    error!=GL_FRAMEBUFFER_COMPLETE)
         {
            LogN(S+"Invalid OpenGL FramebufferStatus: "+error+", frame:"+Time.frame()+", t0:"+(t0 ? t0->w() : 0)+", ds:"+(ds ? ds->w() : 0));
            Break();
            switch(error)
            {
               case GL_FRAMEBUFFER_UNSUPPORTED                  : error=0; break;
               case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT        : error=0; break;
               case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: error=0; break;
            #if GL_ES && defined GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
               case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS        : error=0; break;
            #else
               case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER       : error=0; break;
               case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER       : error=0; break;
            #endif
            }
         }
      #endif
      }

     _cur[0] =t0;
     _cur[1] =t1;
     _cur[2] =t2;
     _cur[3] =t3;
     _cur_ds =ds;
      changed=true;
   }else
   if(_cur_ds!=ds) // check this in case 'set_ds' was cleared to null
   {
     _cur_ds =ds;
      changed=true;
   #if !IOS // there is no default frame buffer on iOS
      D.depthAllow(!main_fbo || ds==&_main_ds); // check 'ds' and not 'set_ds' !!
   #endif
   }
#endif

   if(changed)
   {
      if(Image *main=(t0 ? t0 : ds))
      {
        _res=main->size();
         setPixelSize();
         Sh.rtSize(*main);
      }

   #if GL
      if(was_main_fbo!=main_fbo)SwitchedFBO();
   #endif
      D._view_active.setRect(custom_viewport ? screenToPixelI(D.viewRect()) : RectI(0, 0, resW(), resH())).setViewport();
      D.clipAllow(_cur[0]==_cur_main);
      D.validateCoords(); // viewport was changed, also for OpenGL (flip Y 2D coords when Rendering To Texture)
   }else // render targets weren't changed, so set viewport only
   {
      RectI recti(custom_viewport ? screenToPixelI(D.viewRect()) : RectI(0, 0, resW(), resH()));
      if(   recti!=D._view_active.recti) // over here we can do a quick != check first, because the Render Targets haven't changed (Renderer.resW(), resH() is the same, and that affects 'setRect')
      {
         D._view_active.setRect(recti).setViewport();
         D.validateCoords(); // viewport was changed, also for OpenGL (flip Y 2D coords when Rendering To Texture)
      }
   }
}
/******************************************************************************/
void RendererClass::setMainViewportCam()
{
   if(_stereo)
   {
      D._view_active.setRect(Renderer.screenToPixelI(D._view_rect)).setViewport().setShader();
      SetProjMatrix();
      SetCam(ActiveCam.matrix, ActiveCam._matrix_prev); // 'Frustum' remains the same
      D.validateCoords();
      D.setViewFovTan();
   }
}
void RendererClass::setEyeViewportCam()
{
   if(_stereo)
   {
      D._view_active.setRect(Renderer.screenToPixelI(D._view_eye_rect[_eye])).setViewport().setShader(&ProjMatrixEyeOffset[_eye]); // 'setShader' needed for 'PosToScreen' and 'fur'
      SetProjMatrix(ProjMatrixEyeOffset[_eye]);
      SetCam(EyeMatrix[_eye], EyeMatrixPrev[_eye]); // 'Frustum' remains the same
      D.validateCoords(_eye);
      D.setViewFovTan();
   }
}
Rect* RendererClass::setEyeParams()
{
   if(_stereo)
   {
      RectI recti=D._view_active.recti;
      D._view_active.setRect(Renderer.screenToPixelI(D._view_eye_rect[_eye])).setShader(&ProjMatrixEyeOffset[_eye]).setRect(recti); // set rect temporarily to set shader params and restore it afterwards
      return &D._view_eye_rect[_eye];
   }
   return &D._view_rect;
}
/******************************************************************************/
void RendererClass::     hasGlow() {_has|=HAS_GLOW;}
void RendererClass::finalizeGlow()
{
  _has_glow=((_has&HAS_GLOW) && _col->typeInfo().a && D.glowAllow() && D.bloomAllow() && D.bloomGlow()>EPS_COL8_NATIVE && !fastCombine()); // glow can be done only if we have Alpha Channel in the RT, if we're allowing bloom processing (because it's done together in the same shader), if we're allowing glow, and if 'fastCombine' is not active
}
/******************************************************************************/
Bool RendererClass::capture(Image &image, Int w, Int h, Int type, Int mode, Int mip_maps, Bool alpha)
{
   if(image.capture(*_ptr_main))
   {
      if(type<=0)type=image.type();else MIN(type, IMAGE_TYPES);
      if(!_ds_1s)alpha=false;

      if(ImageTI[type].a && image.typeInfo().a && !alpha && image.lock()) // dest has alpha and src has alpha, and don't want to manually set alpha
      {
         REPD(y, image.h())
         REPD(x, image.w())
         {
            Color color=image.color(x, y); color.a=255; // force full alpha
                        image.color(x, y,  color);
         }
         image.unlock();
      }

      if(image.copy(image, w, h, 1, type, mode, mip_maps))
      {
         if(alpha && image.typeInfo().a && image.lock()) // set alpha from depth
         {
            Image depth; if(depth.capture(*_ds_1s) && depth.lockRead())
            {
               Image alpha(depth.w(), depth.h(), 1, IMAGE_A8, IMAGE_SOFT, 1);
               REPD(y, depth.h())
               REPD(x, depth.w())
               {
                  Flt w=depth.pixelF(x, y);
               #if REVERSE_DEPTH
                  alpha.pixB(x, y)=((w>0.0f) ? 0xFF : 0);
               #else
                  alpha.pixB(x, y)=((w<1.0f) ? 0xFF : 0);
               #endif
               }
               depth.unlock();

               alpha.resize(image.w(), image.h(), FILTER_LINEAR);
               REPD(y, image.h())
               REPD(x, image.w())
               {
                  Color color=image.color(x, y); color.a=alpha.pixB(x, y);
                              image.color(x, y,  color);
               }
            }else
            {
               REPD(y, image.h())
               REPD(x, image.w())
               {
                  Color color=image.color(x, y); color.a=255; // force full alpha
                              image.color(x, y,  color);
               }
            }
            image.unlock().updateMipMaps();
         }
         return true;
      }
   }
   image.del(); return false;
}
Bool RendererClass::screenShot(C Str &name, Bool alpha)
{
   FCreateDirs(GetPath(name));
   Image temp;
   if(alpha) // with alpha
   {
      if(capture(temp, -1, -1, IMAGE_R8G8B8A8_SRGB, IMAGE_SOFT, 1, true))return temp.Export(name);
   }else
   if(temp.capture(*_ptr_main)) // no alpha
   {
      if(temp.typeInfo().a)temp.copy(temp, -1, -1, -1, IMAGE_R8G8B8_SRGB, IMAGE_SOFT, 1); // if captured image has alpha channel then let's remove it
      return temp.Export(name);
   }
   return false;
}
Bool RendererClass::screenShots(C Str &name, C Str &ext, Bool alpha)
{
   Str n=FFirst(name, ext);
   return n.is() ? screenShot(n, alpha) : false;
}
/******************************************************************************/
void RendererClass::timeMeasure(Bool on)
{
   if(_t_measure!=on)
   {
     _t_measure=on;
     _t_last_measure=Time.curTime();
   }
}
/******************************************************************************/
}
/******************************************************************************/
