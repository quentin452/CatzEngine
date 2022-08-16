/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   TODO: on DX11 states are activated ('set') for each single state change, while they should be activated only after all single changes

/******************************************************************************/
#define DEPTH_VALUE(x) (REVERSE_DEPTH ? -(x) : (x))

// #ShadowBias

#define DEPTH_BIAS_SHADOW  DEPTH_VALUE( 0)
#define DEPTH_BIAS_OVERLAY DEPTH_VALUE(-1)
#define DEPTH_BIAS_EARLY_Z DEPTH_VALUE( 1)

#define SLOPE_SCALED_DEPTH_BIAS_SHADOW  DEPTH_VALUE( 2.0f)
#define SLOPE_SCALED_DEPTH_BIAS_OVERLAY DEPTH_VALUE(-1.0f)
#define SLOPE_SCALED_DEPTH_BIAS_EARLY_Z DEPTH_VALUE( 0.0f)
/******************************************************************************/
#if DX11
struct BlendState
{
   ID3D11BlendState *state;
   
   void del()
   {
      if(state)
      {
      #if GPU_LOCK // lock not needed for 'Release'
         SyncLocker locker(D._lock); if(state)
      #endif
            {if(D.created())state->Release(); state=null;}
      }
   }
   void create(D3D11_BLEND_DESC &desc)
   {
   #if GPU_LOCK // lock not needed for 'D3D'
      SyncLocker locker(D._lock);
   #endif
      del();
      if(D3D && OK(D3D->CreateBlendState(&desc, &state)))return;
      Exit("Can't create a Blend State");
   }
   void set()
   {
      D3DC->OMSetBlendState(state, D._alpha_factor_v4.c, D._sample_mask);
   }
   BlendState() {state=null;}
  ~BlendState() {del();}
};
/******************************************************************************/
struct DepthState
{
   ID3D11DepthStencilState *state;

   void del()
   {
      if(state)
      {
      #if GPU_LOCK // lock not needed for 'Release'
         SyncLocker locker(D._lock); if(state)
      #endif
            {if(D.created())state->Release(); state=null;}
      }
   }
   void create(D3D11_DEPTH_STENCIL_DESC &desc)
   {
   #if GPU_LOCK // lock not needed for 'D3D'
      SyncLocker locker(D._lock);
   #endif
      del();
      if(D3D && OK(D3D->CreateDepthStencilState(&desc, &state)))return;
      Exit("Can't create a DepthStencil State");
   }
   void set()
   {
      D3DC->OMSetDepthStencilState(state, D._stencil_ref);
   }
   DepthState() {state=null;}
  ~DepthState() {del();}
};
/******************************************************************************/
struct RasterizerState
{
   ID3D11RasterizerState *state;

   void del()
   {
      if(state)
      {
      #if GPU_LOCK // lock not needed for 'Release'
         SyncLocker locker(D._lock); if(state)
      #endif
            {if(D.created())state->Release(); state=null;}
      }
   }
   void create(D3D11_RASTERIZER_DESC &desc)
   {
   #if GPU_LOCK // lock not needed for 'D3D'
      SyncLocker locker(D._lock);
   #endif
      del();
      if(D3D && OK(D3D->CreateRasterizerState(&desc, &state)))return;
      Exit("Can't create a Rasterizer State");
   }
   void set()
   {
      D3DC->RSSetState(state);
   }
   RasterizerState() {state=null;}
  ~RasterizerState() {del();}
};
/******************************************************************************/
// set order that first array sizes are those not pow2, and followed by pow2 (faster accessing)
static BlendState       BlendStates[  ALPHA_NUM]                  ; // [AlphaMode]
static DepthState       DepthStates[STENCIL_NUM][2][2][8]         ; // [StencilMode][DepthUse][DepthWrite][DepthFunc]
static RasterizerState RasterStates[   BIAS_NUM][2][2][2][2][2][2]; // [Bias][Cull][LineSmooth][Wire][Clip][DepthClip][FrontFace]
#elif GL
static Bool DepthAllow=true, DepthReal;
static Byte Col0WriteAllow=COL_WRITE_RGBA, Col0WriteReal=COL_WRITE_RGBA;
static UInt StencilFunc=GL_ALWAYS, StencilMask=~0;
#if !WINDOWS && !SWITCH
static void (*glBlendFunci)(GLuint buf, GLenum src, GLenum dst); // see 'D.independentBlendAvailable'
#endif
#endif

#if !DX11
static STENCIL_MODE LastStencilMode;
#endif
/******************************************************************************/
DisplayState::DisplayState()
{
#if 0 // there's only one 'DisplayState' global 'D' and it doesn't need clearing members to zero
  _cull            =false;
  _line_smooth     =false;
  _wire            =false;
  _clip            =false;
  _clip_real       =false;
  _clip_rect       .zero();
  _clip_recti      .zero();
  _clip_plane_allow=false;
  _clip_plane      .zero();
  _depth_lock      =false;
  _depth           =false;
  _front_face      =false;
  _stencil_ref     =0;
  _viewport        .zero();
  _alpha_factor    .zero();
  _alpha_factor_v4 .zero();
  _vf              =null;
  _prim_type       =0;
#endif

  _linear_gamma    =LINEAR_GAMMA;
  _clip_allow      =true;
  _depth_write     =true;
  _depth_clip      =true;
  _depth_bias      =BIAS_ZERO;
  _depth_func      =FUNC_DEFAULT;
  _alpha           =ALPHA_BLEND;
  _stencil         =STENCIL_NONE;
  _col_write[0]    =COL_WRITE_RGBA;
  _col_write[1]    =COL_WRITE_RGBA;
  _col_write[2]    =COL_WRITE_RGBA;
  _col_write[3]    =COL_WRITE_RGBA;
  _sample_mask     =~0;
}
/******************************************************************************/
// DEPTH / STENCIL
/******************************************************************************/
void DisplayState::depthUnlock(       ) {           D._depth_lock=false;}
void DisplayState::depthLock  (Bool on) {depth(on); D._depth_lock=true ;}
/******************************************************************************/
#if DX11
   #define D3D11_COMPARISON_FIRST D3D11_COMPARISON_NEVER
static void SetDS() {DepthStates[D._stencil][D._depth][D._depth_write][D._depth_func-D3D11_COMPARISON_FIRST].set();}
void DisplayState::depth           (Bool on                       ) {                          if(D._depth!=on && !D._depth_lock                              ){D._depth=on;                                           SetDS();}}
Bool DisplayState::depthWrite      (         Bool write           ) {Bool last=D._depth_write; if(                D._depth_write!=write                       ){             D._depth_write=write;                     SetDS();} return last;}
void DisplayState::depthFunc       (                     UInt func) {                          if(                                         D._depth_func!=func){                                   D._depth_func=func; SetDS();}}
void DisplayState::depthOnWrite    (Bool on, Bool write           ) {                          if(D._depth!=on || D._depth_write!=write                       ){D._depth=on; D._depth_write=write;                     SetDS();}} // this ignores '_depth_lock'
void DisplayState::depthOnWriteFunc(Bool on, Bool write, UInt func) {                          if(D._depth!=on || D._depth_write!=write || D._depth_func!=func){D._depth=on; D._depth_write=write; D._depth_func=func; SetDS();}} // this ignores '_depth_lock'
void DisplayState::stencilRef      (                   Byte ref   ) {                          if(D._stencil_ref!=ref                                         ){D._stencil_ref=ref ;                                   SetDS();}}
void DisplayState::stencil         (STENCIL_MODE mode, Byte ref   ) {                          if(D._stencil_ref!=ref || D._stencil!=mode                     ){D._stencil_ref=ref ; D._stencil=mode;                  SetDS();}}
#elif GL
Bool DisplayState::depthWrite(Bool on  ) {Bool last=D._depth_write; if(D._depth_write!=on  )glDepthMask  (             D._depth_write=on              ); return last;}
void DisplayState::depthFunc (UInt func) {                          if(D._depth_func !=func)glDepthFunc  (             D._depth_func =func            );}
void DisplayState::stencilRef(Byte ref ) {                          if(D._stencil_ref!=ref )glStencilFunc(StencilFunc, D._stencil_ref=ref, StencilMask);}
void DisplayState::stencil   (STENCIL_MODE mode, Byte ref)
{
#if 1
   STENCIL_MODE old_mode=LastStencilMode; // remember old values
   Byte         old_ref =D._stencil_ref; D._stencil_ref=ref; // already change 'D._stencil_ref' because calling 'stencil' might use it
   stencil(mode);
   if(old_mode==LastStencilMode && old_ref!=ref)glStencilFunc(StencilFunc, ref, StencilMask); // if 'stencil' didn't change 'LastStencilMode' and we wanted to change 'D._stencil_ref' then manually change the 'ref'
#else
   stencil(mode); stencilRef(ref);
#endif
}
void DisplayState::depth(Bool on)
{
   if(D._depth!=on && !D._depth_lock)
   {
      D._depth=on;
   #if !IOS // on desktop OpenGL and OpenGL ES (except iOS) '_main' is always linked with '_main_ds', when setting "_main null" RT DS, '_main_ds' is set either way but with depth disabled
      on&=DepthAllow; if(on==DepthReal)return; DepthReal=on;
   #endif
      if(on)glEnable(GL_DEPTH_TEST);else glDisable(GL_DEPTH_TEST);
   }
}
void DisplayState::depthAllow(Bool on)
{
   if(DepthAllow!=on)
   {
      DepthAllow=on;
      on&=D._depth;
      if(DepthReal!=on)if(DepthReal=on)glEnable(GL_DEPTH_TEST);else glDisable(GL_DEPTH_TEST);
   }
}
void DisplayState::depthOnWrite    (Bool on, Bool write           ) {D.depth(on); D.depthWrite(write);}
void DisplayState::depthOnWriteFunc(Bool on, Bool write, UInt func) {D.depth(on); D.depthWrite(write); D.depthFunc(func);}
#endif
/******************************************************************************/
void DisplayState::depth2DOn(UInt func)
{
#if DX11
   if(D._depth!=true || D._depth_write!=false || D._depth_func!=func)
   {
      D._depth      =true ;
      D._depth_write=false;
      D._depth_func =func ;
      SetDS();
   }
   D._depth_lock=true;
#else
   depthLock (true );
   depthWrite(false);
   depthFunc (func );
#endif
}
void DisplayState::depth2DOff()
{
   UInt func=(Renderer.firstPass() ? FUNC_DEFAULT : FUNC_LESS_EQUAL); // this can be called while rendering secondary lights for forward renderer, in that case we're using FUNC_LESS_EQUAL because we need to apply secondary lights for same depths
#if DX11
   if(D._depth_write!=true || D._depth_func!=func)
   {
      D._depth_write=true;
      D._depth_func =func;
      SetDS();
   }
   D._depth_lock=false;
#else
   depthWrite (true);
   depthUnlock(    );
   depthFunc  (func);
#endif
}
/******************************************************************************/
void DisplayState::stencil(STENCIL_MODE stencil)
{
#if DX11
   if(D._stencil!=stencil){D._stencil=stencil; SetDS();}
#elif GL
   if(D._stencil!=stencil)
   {
      D._stencil=stencil;
      if(!stencil)
      {
         glDisable(GL_STENCIL_TEST);
      }else
      {
         glEnable(GL_STENCIL_TEST);

         if(LastStencilMode!=stencil)switch(LastStencilMode=stencil)
         {
            case STENCIL_ALWAYS_SET:
               glStencilFunc(StencilFunc=GL_ALWAYS, D._stencil_ref, StencilMask=~0); // set full mask
               glStencilOp  (GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

            case STENCIL_TERRAIN_TEST:
               glStencilFunc(StencilFunc=GL_EQUAL, D._stencil_ref, StencilMask=STENCIL_REF_TERRAIN);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_KEEP);
            break;

            case STENCIL_MSAA_SET:
               glStencilFunc(StencilFunc=GL_ALWAYS, D._stencil_ref, StencilMask=STENCIL_REF_MSAA);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

            case STENCIL_MSAA_TEST:
               glStencilFunc(StencilFunc=GL_EQUAL, D._stencil_ref, StencilMask=STENCIL_REF_MSAA);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_KEEP);
            break;

            case STENCIL_EDGE_SOFT_SET:
               glStencilFunc(StencilFunc=GL_ALWAYS, D._stencil_ref, StencilMask=STENCIL_REF_EDGE_SOFT);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

            case STENCIL_EDGE_SOFT_TEST:
               glStencilFunc(StencilFunc=GL_EQUAL, D._stencil_ref, StencilMask=STENCIL_REF_EDGE_SOFT);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_KEEP);
            break;

            case STENCIL_WATER_SET:
               glStencilFunc(StencilFunc=GL_ALWAYS, D._stencil_ref, StencilMask=STENCIL_REF_WATER);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

            case STENCIL_WATER_TEST:
               glStencilFunc(StencilFunc=GL_EQUAL, D._stencil_ref, StencilMask=STENCIL_REF_WATER);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_KEEP);
            break;

            case STENCIL_OUTLINE_SET:
               glStencilFunc(StencilFunc=GL_ALWAYS, D._stencil_ref, StencilMask=STENCIL_REF_OUTLINE);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

            case STENCIL_OUTLINE_TEST:
               glStencilFunc(StencilFunc=GL_EQUAL, D._stencil_ref, StencilMask=STENCIL_REF_OUTLINE);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_KEEP);
            break;
         }
      }
   }
#endif
}
/******************************************************************************/
// RASTERIZER
/******************************************************************************/
#if DX11
static void SetRS() {RasterStates[D._depth_bias][D._cull][D._line_smooth][D._wire][D._clip_real][D._depth_clip][D._front_face].set();}
Bool DisplayState::lineSmooth(Bool      on  ) {Bool old=D._line_smooth; if(D._line_smooth!=on){D._line_smooth=on; if(D3DC)SetRS();} return old;}
void DisplayState::wire      (Bool      on  ) {if(D._wire      !=on  ){D._wire      =on  ; SetRS();}}
void DisplayState::cull      (Bool      on  ) {if(D._cull      !=on  ){D._cull      =on  ; SetRS();}}
void DisplayState::depthBias (BIAS_MODE bias) {if(D._depth_bias!=bias){D._depth_bias=bias; SetRS();}}
void DisplayState::depthClip (Bool      on  ) {if(D._depth_clip!=on  ){D._depth_clip=on  ; SetRS();}}
void DisplayState::frontFace (Bool      ccw ) {if(D._front_face!=ccw ){D._front_face=ccw ; SetRS();}}
#elif GL
Bool DisplayState::lineSmooth(Bool on)
{
   Bool old=D._line_smooth;
   if(D._line_smooth!=on)
   {
      D._line_smooth=on;
   #ifdef GL_LINE_SMOOTH
      if(on)glEnable(GL_LINE_SMOOTH);else glDisable(GL_LINE_SMOOTH);
   #endif
   }
   return old;
}
void DisplayState::wire(Bool on)
{
#if !GL_ES
	if(D._wire!=on)glPolygonMode(GL_FRONT_AND_BACK, (D._wire=on) ? GL_LINE : GL_FILL);
#endif
}
void DisplayState::cull(Bool on) {if(D._cull!=on)if(D._cull=on)glEnable(GL_CULL_FACE);else glDisable(GL_CULL_FACE);}
void DisplayState::depthBias(BIAS_MODE bias)
{
   if(D._depth_bias!=bias)switch(D._depth_bias=bias)
   {
      case BIAS_ZERO:
      {
         glDisable      (GL_POLYGON_OFFSET_FILL);
         glPolygonOffset(0, 0);
      }break;

      case BIAS_SHADOW:
      {
         glEnable       (GL_POLYGON_OFFSET_FILL);
         glPolygonOffset(SLOPE_SCALED_DEPTH_BIAS_SHADOW, DEPTH_BIAS_SHADOW);
      }break;

      case BIAS_OVERLAY:
      {
         glEnable       (GL_POLYGON_OFFSET_FILL);
         glPolygonOffset(SLOPE_SCALED_DEPTH_BIAS_OVERLAY, DEPTH_BIAS_OVERLAY);
      }break;

      case BIAS_EARLY_Z:
      {
         glEnable       (GL_POLYGON_OFFSET_FILL);
         glPolygonOffset(SLOPE_SCALED_DEPTH_BIAS_EARLY_Z, DEPTH_BIAS_EARLY_Z);
      }break;
   }
}
void DisplayState::depthClip(Bool on)
{
   if(D._depth_clip!=on)
   {
      D._depth_clip=on;
   #ifdef GL_DEPTH_CLAMP
      if(on)glDisable(GL_DEPTH_CLAMP);else glEnable(GL_DEPTH_CLAMP); // !! on GL enabling GL_DEPTH_CLAMP actually disables clipping !!
   #endif
   }
}
void DisplayState::frontFace(Bool ccw)
{
   if(D._front_face!=ccw)
   {
      D._front_face=ccw;
      glFrontFace(ccw ? GL_CCW : GL_CW);
   }
}
#endif
void DisplayState::setFrontFace()
{
   Bool ccw=(Renderer.mirror() && Renderer()!=RM_SHADOW);
#if GL
   if(!D.mainFBO())ccw^=1;
#endif
   frontFace(ccw);
}
/******************************************************************************/
static void SetClip()
{
   if(D._clip_real!=(D._clip && D._clip_allow))
   {
      D._clip_real^=1;
   #if DX11
      SetRS();
   #elif GL
      if(D._clip_real)glEnable(GL_SCISSOR_TEST);else glDisable(GL_SCISSOR_TEST);
   #endif
   }
}
static void SetClipRect(C RectI &rect)
{
#if GL
   RectI r(rect.min.x, D.mainFBO() ? Renderer.resH()-rect.max.y : rect.min.y, rect.w(), rect.h()); // RectI(pos, size)
   #define rect r
#endif
   if(D._clip_recti!=rect)
   {
      D._clip_recti=rect; // !! Warning: for GL this is actually RectI(pos, size)
   #if DX11
      D3D11_RECT rectangle; rectangle.left=rect.min.x; rectangle.right=rect.max.x; rectangle.top=rect.min.y; rectangle.bottom=rect.max.y;
      D3DC->RSSetScissorRects(1, &rectangle);
   #elif GL
      glScissor(r.min.x, r.min.y, r.max.x, r.max.y); // glScissor(pos, size)
      #undef rect
   #endif
   }
}
static void SetClipRect()
{
   RectI recti=Renderer.screenToPixelI(D._clip_rect);
   if(recti.max.x<=recti.min.x || recti.max.y<=recti.min.y)recti.zero(); // if the rectangle is invalid then zero it, to prevent negative sizes in case drivers can't handle them properly
   SetClipRect(recti);
}
void DisplayState::clip(C Rect *rect)
{
   if(rect)
   {
      if(!D._clip || D._clip_rect!=*rect)
      {
         D._clip     = true;
         D._clip_rect=*rect;
         SetClip    ();
         SetClipRect();
      }
   }else
   if(D._clip)
   {
      D._clip=false; SetClip();
   }
}
void DisplayState::clipAllow(C RectI &rect) // this gets called during shadow rendering
{
   SetClipRect(rect);
   D._clip_rect=Renderer.pixelToScreen(rect); // needed in case we call 'clip' later which checks if '_clip_rect' changes
   D._clip=D._clip_allow=true; SetClip(); // call after setting '_clip' and '_clip_allow'
}
void DisplayState::clipAllow(Bool on) // this gets called when a render target has changed
{
 //if(D._clip_allow!=on) we must always do below, because: we may have a different sized RT now, and need to recalculate clip rect. for GL we may have changed between D.mainFBO for which we need different orientation of the clip rect
   {
      D._clip_allow=on; SetClip();
      if(D._clip_real)SetClipRect(); // call after 'SetClip' because we need '_clip_real'
   }
}
/******************************************************************************/
void DisplayState::clipPlane(Bool on)
{
#if DX11
   D._clip_plane_allow=on;
   Sh.ClipPlane->set(on ? D._clip_plane : Vec4(0, 0, 0, 1));
#elif GL && defined GL_CLIP_DISTANCE0
   if(on)glEnable(GL_CLIP_DISTANCE0);else glDisable(GL_CLIP_DISTANCE0);
#endif
}
void DisplayState::clipPlane(C PlaneM &plane)
{
   VecD pos=plane.pos   *CamMatrixInv;
   Vec  nrm=plane.normal*CamMatrixInv.orn();
#if DX11
   D._clip_plane.set(nrm, -Dot(pos, nrm));
   clipPlane(D._clip_plane_allow);
#elif GL
   Vec4 clip_plane(nrm, -Dot(pos, nrm));
   Sh.ClipPlane->set(clip_plane);
#endif
}
/******************************************************************************/
// OUTPUT MERGER
/******************************************************************************/
#if DX11
static void SetBS() {BlendStates[D._alpha].set();}
#endif
ALPHA_MODE DisplayState::alpha(ALPHA_MODE alpha)
{
   ALPHA_MODE prev=D.alpha();
#if DX11
   if(D._alpha!=alpha)BlendStates[D._alpha=alpha].set();
#elif GL
   if(D._alpha!=alpha)switch(D._alpha=alpha)
   {
      case ALPHA_NONE:
         glDisable(GL_BLEND);
      break;

      case ALPHA_BLEND:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
      break;
      case ALPHA_RENDER_BLEND:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
         if(glBlendFunci)glBlendFunci(1, GL_ONE_MINUS_DST_COLOR, GL_ONE); // #RTOutput.Blend set RT1 as Increase
      break;

      case ALPHA_ADD:
         glEnable       (GL_BLEND);
         glBlendEquation(GL_FUNC_ADD);
         glBlendFunc    (GL_ONE, GL_ONE);
      break;
      case ALPHA_MUL:
         glEnable       (GL_BLEND);
         glBlendEquation(GL_FUNC_ADD);
         glBlendFunc    (GL_DST_COLOR, GL_ZERO);
      break;

      case ALPHA_MERGE:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
      break;
      case ALPHA_ADD_KEEP:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
      break;
      case ALPHA_ADDBLEND_KEEP:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
      break;

      case ALPHA_RENDER_BLEND_FACTOR:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_CONSTANT_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         if(glBlendFunci)glBlendFunci(1, GL_ONE_MINUS_DST_COLOR, GL_ONE); // #RTOutput.Blend set RT1 as Increase
      break;

      case ALPHA_OVERLAY:
      {
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_CONSTANT_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }break;

      case ALPHA_FACTOR:
         glEnable       (GL_BLEND);
         glBlendEquation(GL_FUNC_ADD);
         glBlendFunc    (GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR);
      break;

      case ALPHA_INVERT:
         glEnable       (GL_BLEND);
         glBlendEquation(GL_FUNC_ADD);
         glBlendFunc    (GL_ONE_MINUS_DST_COLOR, GL_ZERO);
      break;

      case ALPHA_FONT:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      break;
      case ALPHA_FONT_DEC:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
      break;

      case ALPHA_KEEP_SET:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
      break;
   }
#endif
   return prev;
}
void DisplayState::alphaFactor(C Color &factor) // 'MaterialClear' must be called if changing this
{
   if(D._alpha_factor!=factor)
   {
      D._alpha_factor=factor;
	#if DX11
      D._alpha_factor_v4=SRGBToDisplay(factor); SetBS();
   #elif GL
      glBlendColor(ByteSRGBToDisplay(factor.r), ByteSRGBToDisplay(factor.g), ByteSRGBToDisplay(factor.b), ByteToFlt(factor.a));
   #endif
   }
}
/******************************************************************************/
void DisplayState::colWrite(Byte color_mask, Byte index)
{
   DEBUG_RANGE_ASSERT(index, D._col_write);
#if DX11
   // color writes are not supported on DX10+ implementation
#elif GL
   if(D._col_write[index]!=color_mask)
   {
      D._col_write[index]=color_mask;
   #if !IOS // on desktop OpenGL and OpenGL ES (except iOS) '_main' is always linked with '_main_ds', when setting "null _main_ds" RT DS, '_main' is set either way but with color writes disabled
      if(!index){color_mask&=Col0WriteAllow; if(color_mask==Col0WriteReal)return; Col0WriteReal=color_mask;}
   #endif
   #if GL_ES
      if(!index)glColorMask(FlagOn(color_mask, COL_WRITE_R), FlagOn(color_mask, COL_WRITE_G), FlagOn(color_mask, COL_WRITE_B), FlagOn(color_mask, COL_WRITE_A)); // TODO: 'glColorMaski' requires GLES 3.2 - https://www.khronos.org/registry/OpenGL-Refpages/es3/html/glColorMask.xhtml
   #else
      glColorMaski(index, FlagOn(color_mask, COL_WRITE_R), FlagOn(color_mask, COL_WRITE_G), FlagOn(color_mask, COL_WRITE_B), FlagOn(color_mask, COL_WRITE_A));
   #endif
   }
#endif
}
#if GL
void DisplayState::colWriteAllow(Byte color_mask) // this operates only on "index==0"
{
   if(Col0WriteAllow!=color_mask)
   {
      Col0WriteAllow=color_mask;
      color_mask&=D._col_write[0];
      if(Col0WriteReal!=color_mask)
      {
         Col0WriteReal=color_mask;
         glColorMask(FlagOn(color_mask, COL_WRITE_R), FlagOn(color_mask, COL_WRITE_G), FlagOn(color_mask, COL_WRITE_B), FlagOn(color_mask, COL_WRITE_A));
      }
   }
}
#endif
/******************************************************************************/
#if DX11
void DisplayState::sampleMask(UInt mask)
{
   if(mask!=D._sample_mask){D._sample_mask=mask; SetBS();}
}
#endif
/******************************************************************************/
void DisplayState::viewport(C RectI &rect)
{
   if(D._viewport!=rect)
   {
      D._viewport=rect;
   #if DX11
      D3D11_VIEWPORT vp;
      vp.TopLeftX=D._viewport.min.x; vp.Width =D._viewport.w(); vp.MinDepth=0;
      vp.TopLeftY=D._viewport.min.y; vp.Height=D._viewport.h(); vp.MaxDepth=1;
      D3DC->RSSetViewports(1, &vp);
   #elif GL
      glViewport(D._viewport.min.x, D._viewport.min.y, D._viewport.w(), D._viewport.h());
   #endif
   }
}
/******************************************************************************/
void DisplayState::vf(GPU_API(ID3D11InputLayout, VtxFormatGL) *vf)
{
#if DX11
   if(D._vf!=vf)D3DC->IASetInputLayout(D._vf=vf);
#elif GL
 // !! when using VAO's, this can be called only after setting the default 'VAO', because we use 'disable' expecting previous 'vf' to be in the same VAO !!
 //if(D._vf!=vf) OpenGL requires resetting this for each new VBO (for example Layered Clouds didn't work after displaying Sky, even though they have the same VF but different VBO)
   {
      if(D._vf   )D._vf->disable   ();
      if(D._vf=vf)D._vf-> enableSet();
   }
#endif
}
/******************************************************************************/
void DisplayState::set2D() {                     D.clipPlane(false); D.wire(false        );}
void DisplayState::set3D() {if(Renderer.mirror())D.clipPlane(true ); D.wire(Renderer.wire);}
/******************************************************************************/
void DisplayState::linearGamma(Bool on)
{
   if(D._linear_gamma!=on)
   {
      D._linear_gamma=on;
      SRGBToDisplayArray=(on ? ByteSRGBToLinearArray : ByteToFltArray);
      Sh.FontCur  =Sh.Font  [false][on];
      Sh.FontCurSP=Sh.FontSP[false][on];
      // alpha factor depends on gamma, have to reset it
   #if 1 // keep old value
      if(D._alpha_factor.any()) // we can do it only if 'any' because all zeroes have the same value for all gammas and don't need reset
         {Color old=D._alpha_factor; D._alpha_factor.r^=1; D.alphaFactor(old);}
   #else // clear to zero
      D.alphaFactor(TRANSPARENT); MaterialClear(); // TRANSPARENT gives the same Vec4 for sRGB and non-sRGB, 'MaterialClear' must be called when changing 'D.alphaFactor'
   #endif
   }
}
/******************************************************************************/
#if GL
void DisplayState::fbo(UInt fbo)
{
   if(D._fbo!=fbo)glBindFramebuffer(GL_FRAMEBUFFER, D._fbo=fbo);
}
#endif
#if IOS
Bool DisplayState::mainFBO()C
{
   return Renderer._cur[0]==&Renderer._main || Renderer._cur_ds==&Renderer._main_ds; // check both, because it's possible only one of them is attached
}
#endif
/******************************************************************************/
void DisplayState::setDeviceSettings()
{
   DisplayState old=T;

   SamplerRender     .set(SSI_RENDER);
   SamplerPoint      .set(SSI_POINT);
   SamplerLinearWrap .set(SSI_LINEAR_WRAP);
   SamplerLinearClamp.set(SSI_LINEAR_CLAMP);
   SamplerLinearCWW  .set(SSI_LINEAR_CWW);
   SamplerShadowMap  .set(SSI_SHADOW);
   SamplerFont       .set(SSI_FONT);
   SamplerMinimum    .set(SSI_MINIMUM);
   SamplerMaximum    .set(SSI_MAXIMUM);
 //SPSet("AllowAlphaToCoverage", D.multiSample() || D.densityByte()==255);

#if GL
      glEnable     (GL_DITHER);
   #if GL_ES
      glClearDepthf(REVERSE_DEPTH ? 0.5f : 1.0f); // #glClipControl
   #else
      glClearDepth (REVERSE_DEPTH ? 0.5  : 1.0 ); // #glClipControl
    //glAlphaFunc  (GL_GREATER, 0);
      glHint       (GL_LINE_SMOOTH_HINT, GL_NICEST); 
   #endif
   #if !IOS
      if(DepthReal)glEnable   (GL_DEPTH_TEST);else glDisable(GL_DEPTH_TEST);
                   glColorMask(FlagOn(Col0WriteReal, COL_WRITE_R), FlagOn(Col0WriteReal, COL_WRITE_G), FlagOn(Col0WriteReal, COL_WRITE_B), FlagOn(Col0WriteReal, COL_WRITE_A));
   #endif
#endif

  _vf=null;

  _linear_gamma^=1; linearGamma(!_linear_gamma);

#if DX11
   SetDS();
   SetRS();
   SetBS();
#else
  _depth      ^=1; depth     (old._depth      );
  _depth_write^=1; depthWrite(old._depth_write);
  _depth_clip ^=1; depthClip (old._depth_clip );
  _depth_func ^=1; depthFunc (old._depth_func );

                   stencil   ( STENCIL_ALWAYS_SET           ); // required for GL because of 'LastStencilMode'
                   stencil   ( STENCIL_TERRAIN_TEST         );
                   stencil   ((STENCIL_MODE)old._stencil    );
  _stencil_ref^=1; stencilRef(              old._stencil_ref);

  _line_smooth^=1; lineSmooth(!_line_smooth             );
  _wire       ^=1; wire      (!_wire                    );
  _cull       ^=1; cull      (!_cull                    );
  _front_face ^=1; frontFace (!_front_face              );
  _depth_bias ^=1; depthBias ((BIAS_MODE)old._depth_bias);

  _alpha          =ALPHA_MODE(_alpha^1); alpha      (old._alpha       );
  _alpha_factor.r^=                  1 ; alphaFactor(old._alpha_factor);

  _col_write[0]^=1; colWrite(old._col_write[0], 0);
  _col_write[1]^=1; colWrite(old._col_write[1], 1);
  _col_write[2]^=1; colWrite(old._col_write[2], 2);
  _col_write[3]^=1; colWrite(old._col_write[3], 3);
#endif

  _clip_recti.set(0, -1);
  _clip      ^=1; clip     (old._clip ? &old._clip_rect : null);
  _clip_allow^=1; clipAllow(old._clip_allow);

  _viewport.set(0, -1); viewport(old._viewport);

  _prim_type^=1; primType(old._prim_type);

   clearShader();
}
/******************************************************************************/
void DisplayState::del()
{
#if DX11
   REPAO(BlendStates).del();
   REPAD(i, DepthStates)REPAD(j, DepthStates[i])REPAD(k, DepthStates[i][j])REPAOD(l, DepthStates[i][j][k]).del();
   REPAD(i, RasterStates)REPAD(j, RasterStates[i])REPAD(k, RasterStates[i][j])REPAD (l, RasterStates[i][j][k])REPAD(m, RasterStates[i][j][k][l])REPAD(n, RasterStates[i][j][k][l][m])REPAOD(o, RasterStates[i][j][k][l][m][n]).del();
#endif
}
void DisplayState::create()
{
   if(LogInit)LogN("DisplayState.create");

#if DX11
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=false;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BlendStates[ALPHA_NONE].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable   =true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_SRC_ALPHA;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_INV_DEST_ALPHA;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BlendStates[ALPHA_BLEND].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable   =true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_SRC_ALPHA;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ZERO;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      if(D.independentBlendAvailable()) // #RTOutput.Blend set RT1 as Increase
      {
         desc.IndependentBlendEnable=true;
         for(Int i=1; i<Elms(desc.RenderTarget); i++)desc.RenderTarget[i]=desc.RenderTarget[0];

         desc.RenderTarget[1]. SrcBlend     =D3D11_BLEND_INV_DEST_COLOR;
         desc.RenderTarget[1].DestBlend     =D3D11_BLEND_ONE           ;
         desc.RenderTarget[1]. SrcBlendAlpha=D3D11_BLEND_ZERO          ; // these are ignored because alpha RT is R8
         desc.RenderTarget[1].DestBlendAlpha=D3D11_BLEND_ONE           ; // these are ignored because alpha RT is R8
      }
      BlendStates[ALPHA_RENDER_BLEND].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].  BlendOp=desc.RenderTarget[0].  BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend=desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ONE;
      desc.RenderTarget[0].DestBlend=desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BlendStates[ALPHA_ADD].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].  BlendOp=desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_DEST_COLOR;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_DEST_ALPHA;
      desc.RenderTarget[0].DestBlend     =desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ZERO;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BlendStates[ALPHA_MUL].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable   =true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_ONE;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_INV_DEST_ALPHA;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BlendStates[ALPHA_MERGE].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_ONE ;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_ONE ;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ZERO;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE ;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BlendStates[ALPHA_ADD_KEEP].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_SRC_ALPHA;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_ONE ;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ZERO;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE ;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BlendStates[ALPHA_ADDBLEND_KEEP].create(desc); // this is used as graphical effect for Dungeon Hero
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_SRC_ALPHA    ;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_BLEND_FACTOR ;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      if(D.independentBlendAvailable()) // #RTOutput.Blend set RT1 as Increase
      {
         desc.IndependentBlendEnable=true;
         for(Int i=1; i<Elms(desc.RenderTarget); i++)desc.RenderTarget[i]=desc.RenderTarget[0];

         desc.RenderTarget[1]. SrcBlend     =D3D11_BLEND_INV_DEST_COLOR;
         desc.RenderTarget[1].DestBlend     =D3D11_BLEND_ONE           ;
         desc.RenderTarget[1]. SrcBlendAlpha=D3D11_BLEND_ZERO          ; // these are ignored because alpha RT is R8
         desc.RenderTarget[1].DestBlendAlpha=D3D11_BLEND_ONE           ; // these are ignored because alpha RT is R8
      }
      BlendStates[ALPHA_RENDER_BLEND_FACTOR].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_SRC_ALPHA    ;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_BLEND_FACTOR ;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BlendStates[ALPHA_OVERLAY].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_BLEND_FACTOR;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_BLEND_FACTOR;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_BLEND_FACTOR;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_BLEND_FACTOR;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BlendStates[ALPHA_FACTOR].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].  BlendOp     =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_INV_DEST_COLOR;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_INV_DEST_ALPHA;
      desc.RenderTarget[0].DestBlend     =desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ZERO;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BlendStates[ALPHA_INVERT].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].  BlendOp     =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_BLEND_FACTOR;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_SRC_COLOR;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ONE;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BlendStates[ALPHA_FONT].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].  BlendOp     =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_BLEND_FACTOR;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_SRC_COLOR;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ZERO;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BlendStates[ALPHA_FONT_DEC].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=false;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALPHA;
      BlendStates[ALPHA_KEEP_SET].create(desc);
   }

   // depth stencil state
   REPD(stencil    , STENCIL_NUM)
   REPD(depth_use  , 2)
   REPD(depth_write, 2)
   REPD(depth_func , 8)
   {
      D3D11_DEPTH_STENCIL_DESC desc; Zero(desc);
      desc.DepthEnable     =depth_use;
      desc.DepthWriteMask  =(depth_write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO);
      desc.DepthFunc       =D3D11_COMPARISON_FUNC(D3D11_COMPARISON_FIRST+depth_func);
      desc.StencilEnable   =(stencil!=STENCIL_NONE);
      desc.StencilReadMask =D3D11_DEFAULT_STENCIL_READ_MASK;
      desc.StencilWriteMask=D3D11_DEFAULT_STENCIL_WRITE_MASK;
      desc.FrontFace.StencilFailOp=desc.BackFace.StencilFailOp=D3D11_STENCIL_OP_KEEP;
      switch(stencil)
      {
         case STENCIL_ALWAYS_SET:
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_ALWAYS;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_TERRAIN_TEST:
            desc.StencilReadMask      =STENCIL_REF_TERRAIN;
            desc.StencilWriteMask     =0;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_EQUAL;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_MSAA_SET:
            desc.StencilReadMask =STENCIL_REF_MSAA;
            desc.StencilWriteMask=STENCIL_REF_MSAA;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_ALWAYS;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_MSAA_TEST:
            desc.StencilReadMask =STENCIL_REF_MSAA;
            desc.StencilWriteMask=0;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_EQUAL;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_EDGE_SOFT_SET:
            desc.StencilReadMask =STENCIL_REF_EDGE_SOFT;
            desc.StencilWriteMask=STENCIL_REF_EDGE_SOFT;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_ALWAYS;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_EDGE_SOFT_TEST:
            desc.StencilReadMask =STENCIL_REF_EDGE_SOFT;
            desc.StencilWriteMask=0;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_EQUAL;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_WATER_SET:
            desc.StencilReadMask =STENCIL_REF_WATER;
            desc.StencilWriteMask=STENCIL_REF_WATER;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_ALWAYS;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_WATER_TEST:
            desc.StencilReadMask =STENCIL_REF_WATER;
            desc.StencilWriteMask=0;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_EQUAL;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_OUTLINE_SET:
            desc.StencilReadMask =STENCIL_REF_OUTLINE;
            desc.StencilWriteMask=STENCIL_REF_OUTLINE;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_ALWAYS;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_OUTLINE_TEST:
            desc.StencilReadMask =STENCIL_REF_OUTLINE;
            desc.StencilWriteMask=0;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_EQUAL;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;
      }

      DepthStates[stencil][depth_use][depth_write][depth_func].create(desc);
   }

   REPD(bias, BIAS_NUM)
   REPD(cull, 2)
   REPD(line, 2)
   REPD(wire, 2)
   REPD(clip, 2)
   REPD(depth_clip, 2)
   REPD(front_face, 2)
   {
      D3D11_RASTERIZER_DESC desc; Zero(desc);
      desc.FillMode             =(wire ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID);
      desc.CullMode             =((cull==0) ? D3D11_CULL_NONE : (cull==1) ? D3D11_CULL_BACK : D3D11_CULL_FRONT); // D3D11_CULL_FRONT is not used (instead 'FrontCounterClockwise' is used, so we don't have to make "BackFlip" in shader conditional on a constant)
      desc.ScissorEnable        =clip;
      desc.DepthClipEnable      =depth_clip;
      desc.FrontCounterClockwise=front_face;
      if(D.shaderModel()>=SM_4_1) // in 4.1 following members affect only lines
      {
         desc.MultisampleEnable    =false;
         desc.AntialiasedLineEnable=line ;
      }else // let on 4.0 smooth lines be always disabled
      {
         desc.MultisampleEnable    =true ; // for smooth lines this should be false (but let's not bother with additional state combination only for smooth lines on 4.0)
         desc.AntialiasedLineEnable=false;
      }
      switch(bias)
      {
         case BIAS_ZERO   : desc.DepthBias=                 0; desc.SlopeScaledDepthBias=                              0; break;
         case BIAS_SHADOW : desc.DepthBias=DEPTH_BIAS_SHADOW ; desc.SlopeScaledDepthBias=SLOPE_SCALED_DEPTH_BIAS_SHADOW ; break;
         case BIAS_OVERLAY: desc.DepthBias=DEPTH_BIAS_OVERLAY; desc.SlopeScaledDepthBias=SLOPE_SCALED_DEPTH_BIAS_OVERLAY; break;
         case BIAS_EARLY_Z: desc.DepthBias=DEPTH_BIAS_EARLY_Z; desc.SlopeScaledDepthBias=SLOPE_SCALED_DEPTH_BIAS_EARLY_Z; break;
      }
      RasterStates[bias][cull][line][wire][clip][depth_clip][front_face].create(desc);
   }
#elif GL
   #if !WINDOWS && !SWITCH
      glBlendFunci=(decltype(glBlendFunci))D.glGetProcAddress("glBlendFunci");
   #endif
#endif
   setDeviceSettings();
}
/******************************************************************************/
}
/******************************************************************************/
