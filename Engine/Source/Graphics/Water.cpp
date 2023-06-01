/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
      WaterClass         Water;
const WaterMtrl         *WaterMtrlLast;
      WaterMtrlPtr       WaterMtrlNull;
      Memc<C WaterBall*> WaterBalls;
DEFINE_CACHE(WaterMtrl, WaterMtrls, WaterMtrlPtr, "Water Material");
/******************************************************************************/
// WATER PARAMETERS
/******************************************************************************/
Vec  WaterMtrlParams::colorS(              )C {return        LinearToSRGB(color  ) ;}
void WaterMtrlParams::colorS(C Vec &color_s)  {return colorL(SRGBToLinear(color_s));}

Vec  WaterMtrl::colorUnderwater0S(              )C {return                   LinearToSRGB(color_underwater0) ;}
void WaterMtrl::colorUnderwater0S(C Vec &color_s)  {return colorUnderwater0L(SRGBToLinear(color_s          ));}

Vec  WaterMtrl::colorUnderwater1S(              )C {return                   LinearToSRGB(color_underwater1) ;}
void WaterMtrl::colorUnderwater1S(C Vec &color_s)  {return colorUnderwater1L(SRGBToLinear(color_s          ));}
/******************************************************************************/
WaterMtrl::WaterMtrl()
{
   color=1;
   colorUnderwater0S(Vec(0.26f, 0.35f, 0.42f));
   colorUnderwater1S(Vec(0.10f, 0.20f, 0.30f));

   rough             =0;
   reflect           =0.02f;
   normal            =1;
   wave_scale        =0;

   scale_color       =1.0f/200;
   scale_normal      =1.0f/10;
   scale_bump        =1.0f/100;

   density           =0.3f;
   density_add       =0;

   refract           =0.10f;
   refract_reflection=0.06f;
}
/******************************************************************************/
WaterMtrl& WaterMtrl:: colorMap(C ImagePtr &image) {T. _color_map=image; return T;}
WaterMtrl& WaterMtrl::normalMap(C ImagePtr &image) {T._normal_map=image; return T;}
WaterMtrl& WaterMtrl::  bumpMap(C ImagePtr &image) {T.  _bump_map=image; return T;}
WaterMtrl& WaterMtrl::reset()
{
   T=WaterMtrl();
   return validate();
}
WaterMtrl& WaterMtrl::validate()
{
   if(WaterMtrlLast==this)WaterMtrlLast=null;
   return T;
}
void WaterMtrl::set()C
{
   if(WaterMtrlLast!=this)
   {
      WaterMtrlLast=this;
      WS.WaterMaterial->set<WaterMtrlParams>(T);
      Sh.Col[0]->set( _color_map()); // #MaterialTextureLayoutWater
      Sh.Nrm[0]->set(_normal_map());
      Sh.Ext[0]->set(  _bump_map());
   }
}
/******************************************************************************/
Bool WaterMtrl::save(File &f, CChar *path)C
{
   f.cmpUIntV(2); // version

   Flt refract_underwater=0.01f;
   f<<SCAST(C WaterMtrlParams, T)
    <<color_underwater0
    <<color_underwater1
    <<refract_underwater;

   // textures
   f.putStr( _color_map.name(path)); // !! can't use 'id' because textures are stored in "Tex/" folder, so there's no point in using 'putAsset' !!
   f.putStr(_normal_map.name(path));
   f.putStr(  _bump_map.name(path));

   return f.ok();
}
Bool WaterMtrl::load(File &f, CChar *path)
{
   Char temp[MAX_LONG_PATH];
   Flt  refract_underwater;
   switch(f.decUIntV()) // version
   {
      case 2:
      {
         f>>SCAST(WaterMtrlParams, T)
          >>color_underwater0
          >>color_underwater1
          >>refract_underwater;
         f.getStr(temp);  _color_map.require(temp, path);
         f.getStr(temp); _normal_map.require(temp, path);
         f.getStr(temp);   _bump_map.require(temp, path);
         if(f.ok())return true;
      }break;

      case 1:
      {
         Flt smooth;
         f>>color>>smooth>>reflect>>normal>>wave_scale>>scale_color>>scale_normal>>scale_bump>>density>>density_add>>refract>>refract_reflection; T.smooth(smooth);
         f>>color_underwater0
          >>color_underwater1
          >>refract_underwater;
         f.getStr(temp);  _color_map.require(temp, path);
         f.getStr(temp); _normal_map.require(temp, path);
         f.getStr(temp);   _bump_map.require(temp, path);
         if(f.ok())return true;
      }break;

      case 0:
      {
         Flt temp_flt;
         Vec temp_vec;
         reset();
         f>>density>>density_add>>temp_flt>>temp_flt>>scale_color>>scale_normal>>scale_bump>>normal>>temp_flt>>temp_flt>>refract>>refract_reflection>>refract_underwater>>temp_flt>>wave_scale>>temp_flt>>temp_flt;
         f>>temp_vec>>color>>color_underwater0>>color_underwater1;
         colorS           (color);
         colorUnderwater0S(color_underwater0);
         colorUnderwater1S(color_underwater1);
         scale_color =1/scale_color ;
         scale_normal=1/scale_normal;
         scale_bump  =1/scale_bump  ;
          _color_map.require(f._getStr(), path);
         _normal_map.require(f._getStr(), path);
                             f._getStr();
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
Bool WaterMtrl::save(C Str &name)C
{
   File f; if(f.write(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool WaterMtrl::load(C Str &name)
{
   File f; if(f.read(name))return load(f, _GetPath(name));
   reset(); return false;
}
/******************************************************************************/
// WATER PLANE
/******************************************************************************/
WaterClass::WaterClass()
{
   draw                 =false;
   reflection_allow     =!MOBILE;
   reflection_shadows   =false;
   reflection_resolution=1;
   plane.normal.y       =1;
  _max_1_light          =true;
  _draw_plane_surface   =false;
  _use_secondary_rt     =false;
  _began                =false;
  _swapped_ds           =false;
  _reflect_renderer     =RT_DEFERRED;
}
void WaterClass::del()
{
  _mshr.del();
}
void WaterClass::create()
{
   del();

   // create mesh
   MeshBase mshb; mshb.createPlane(128, 192); // keep vtx number under 64K to use 16-bit index buffers
   REPA(mshb.vtx)
   {
      Vec &pos=mshb.vtx.pos(i);
      pos.y=1-pos.y;
      if(  pos.x<=EPS || pos.x>=1-EPS
      ||   pos.y<=EPS || pos.y>=1-EPS)pos.xy=(pos.xy-0.5f)*6+0.5f; // stretch border vertexes more far away from the center
   }
  _mshr.create(mshb);
}
/******************************************************************************/
WaterClass& WaterClass::reflectionRenderer(RENDER_TYPE type)
{
   Clamp(type, RENDER_TYPE(0), RENDER_TYPE(RT_NUM-1));
   if(type==RT_DEFERRED && D.deferredUnavailable())return T;
   if(type!=_reflect_renderer)
   {
     _reflect_renderer=type;
      if(type==RT_DEFERRED && D.deferredMSUnavailable())D.samples(1); // disable multi-sampling if we can't support it
      D.setShader(); // needed because shaders are set only for current renderer type
   }
   return T;
}
WaterClass& WaterClass::max1Light(Bool on) {/*if(_max_1_light!=on)*/_max_1_light=on; return T;}
/******************************************************************************/
void WaterClass::prepare() // this is called at the start
{
   WaterMtrlLast=null; // reset 'WaterMtrlLast' because we may have 'set' regular Materials which share the same texture shader images as WaterMaterials, so make sure we will reset them when setting water materials

  _under_mtrl        =null;
  _draw_plane_surface=false;

   // check visibility for water plane
   if(draw)
   {
      Matrix3 matrix; matrix.setUp(plane.normal);
      Flt     size=D.viewRange();
      VecD    p   =PointOnPlane(CamMatrix.pos, plane.pos, plane.normal);

      under(plane, T);
      if(Frustum(Extent(Vec(size, wave_scale, size)), MatrixM(matrix, p)))
      {
        _draw_plane_surface=true;
        _quad.set(p+size*(matrix.z-matrix.x), p+size*(matrix.x+matrix.z), p+size*(matrix.x-matrix.z), p+size*(-matrix.x-matrix.z));
      }
   }

   // !! it is important to call this as the first thing during rendering, because 'Game.WorldManager.draw' assumes so, if this needs to be changed then rework 'Game.WorldManager.draw' !!
   // test for under-water on all other water elements
   {
     _mode=MODE_UNDERWATER; Renderer.mode(RM_WATER); Renderer._render();
     _mode=MODE_DRAW      ;
   }

   //  set draw plane surface
   if(_draw_plane_surface)
   {
      if(_under_mtrl && _under_step>=1) // totally under water
      {
        _draw_plane_surface=false;
      }else
      {
         VecD left =FullScreenToPos(Vec2(D.viewRect().min.x, 0), D.viewRange()),
              right=FullScreenToPos(Vec2(D.viewRect().max.x, 0), D.viewRange());

         left =PointOnPlaneRay(left , plane.pos, plane.normal, CamMatrix.y);
         right=PointOnPlaneRay(right, plane.pos, plane.normal, CamMatrix.y);

         Vec2 l2=PosToFullScreen(left ),
              r2=PosToFullScreen(right);

         Rect rect=D.viewRect();
         Flt  dot =Dot(CamMatrix.y, plane.normal);

         if(dot>EPS)
         {
               rect.max.y=Max(l2.y, r2.y);
            if(rect.max.y<rect.min.y)_draw_plane_surface=false;else MIN(rect.max.y, D.viewRect().max.y);
         }else
         if(dot<-EPS)
         {
               rect.min.y=Min(l2.y, r2.y);
            if(rect.min.y>rect.max.y)_draw_plane_surface=false;else MAX(rect.min.y, D.viewRect().min.y);
         }

       /*l2.draw(RED);
         r2.draw(RED);
         rect.draw(0x30FFFFFF);*/

         if(_draw_plane_surface)
         {
            // full    range is D.viewRect().min.y .. D.viewRect().max.y
            // drawing range is       rect  .min.y ..       rect  .max.y
            Flt y_min_frac=LerpR(D.viewRect().min.y, D.viewRect().max.y, rect.min.y),
                y_max_frac=LerpR(D.viewRect().min.y, D.viewRect().max.y, rect.max.y);
         #if 1
           _y_mul_add.x=y_max_frac-y_min_frac;
           _y_mul_add.y=1-y_max_frac;
         #else
           _y_mul_add.x=1;
           _y_mul_add.y=0;
         #endif

            WS.load();
            if(reflection_allow && reflect>EPS_COL8_NATIVE)Renderer.requestMirror(plane, 1, reflection_shadows, reflection_resolution);
         }
      }
   }

   // test for lake reflections
   if(reflection_allow && reflect>EPS_COL8_NATIVE && !Renderer._mirror_want)
      if(!(_under_mtrl && _under_step>=1))
   {
     _mode=MODE_REFLECTION; Renderer.mode(RM_WATER); Renderer._render();
     _mode=MODE_DRAW      ;
   }
}
/******************************************************************************/
void WaterClass::setEyeViewportCam()
{
   Renderer.setEyeViewportCam();
   if(Renderer._mirror_rt)
   {
      Vec2 scale=1,
           offs =0;
      if(Renderer._stereo)
      {
         scale.x=2;
         if(Renderer._eye)offs.x-=1;
         offs.x-=ProjMatrixEyeOffset[Renderer._eye]*0.5f;
      }
      WS.WaterReflectMulAdd->set(Vec4(scale, offs));
   }else
   {
      WS.WaterReflectMulAdd->set(Vec4(1, 1, 0, 0));
   }
}
/******************************************************************************/
void WaterClass::begin()
{
   if(!_began)
   {
     _began=true;

      WS.load();
      D.alpha(ALPHA_NONE);

      ImageRTDesc rt_desc(Renderer._col->w(), Renderer._col->h(), IMAGERT_SRGB);
      if(_use_secondary_rt)
      {
        _swapped_ds=false;
         Renderer._water_col    .get  (rt_desc.type(                                  IMAGERT_SRGB )); // here Alpha is unused
         Renderer._water_nrm    .get  (rt_desc.type(D.signedNrmRT() ? IMAGERT_RGB_S : IMAGERT_RGB  )); // here Alpha is unused
         Renderer._water_refract.get  (rt_desc.type(                                  IMAGERT_TWO_S)); // here Alpha is unused
         Renderer._water_ds     .getDS(Renderer._col->w(), Renderer._col->h(), 1, false);

         if(Renderer.stage)switch(Renderer.stage)
         {
            case RS_WATER_COLOR : Renderer._water_col->clearFull(); break;
            case RS_WATER_NORMAL: Renderer._water_nrm->clearFull(); break;
         }

         // set RT's and depth buffer
         if(Shader *shader=Sh.SetDepth) // if we can copy depth buffer from existing opaque's, then do it, to prevent drawing water pixels if they're occluded
         {
            Renderer.set(null, Renderer._water_ds, true);
            D.depthLock  (true); D.depthFunc(FUNC_ALWAYS ); D.stencil(STENCIL_ALWAYS_SET, 0); shader->draw();
            D.depthUnlock(    ); D.depthFunc(FUNC_DEFAULT); D.stencil(STENCIL_NONE         );
          //Renderer.set(Renderer._water_col, Renderer._water_nrm, Renderer._water_refract, null, Renderer._water_ds, true); don't set, instead swap first and set later
           _swapped_ds=Renderer.swapDS1S(Renderer._water_ds); // try to swap DS to put existing stencil values into '_water_ds' because we will write water depths onto '_water_ds' and we want to use it later instead of '_ds_1s' so we want all stencil values to be kept
            Renderer.set(Renderer._water_col, Renderer._water_nrm, Renderer._water_refract, null, Renderer._water_ds, true);
         }else // if we can't copy then just clear it
         {
            Renderer.set(Renderer._water_col, Renderer._water_nrm, Renderer._water_refract, null, Renderer._water_ds, true);
            D.clearDS();
         }
      }else
      {
         if(Lights.elms())
         {
            Light &light=Lights.first(); if(light.type==LIGHT_DIR && light.allow_main){light.dir.set(); goto light_set;}
         }
         LightDir(Vec(0,-1,0), VecZero).set();
      light_set:

         if(_shader_soft) // we're going to draw water on top of existing RT, including refraction, so we need to have a color copy of what's underwater (background) for the refraction, also we want to do softing so we need to backup depth because we can't read and write to depth in the same time
         {
            Renderer._water_col.get(rt_desc.type(GetImageRTType(Renderer._col->type()))); // create RT for the copy
            Renderer._col->copyHw(*Renderer._water_col, false, D.viewRect()); // copy

         #if GL_ES // iOS (and possibly some Android devices) doesn't have IMAGERT_F32
            Renderer._water_ds.getDS(rt_desc.size.x, rt_desc.size.y, 1, false);
            Renderer._ds_1s->copyDepth(*Renderer._water_ds, false, D.viewRect());
         #else
            Renderer._water_ds.get(rt_desc.type(IMAGERT_F32));
            Renderer._ds_1s->copyHw(*Renderer._water_ds, false, D.viewRect());
         #endif
         }
         setImages(Renderer._water_col, Renderer._water_ds);
         Renderer.set(Renderer._col, Renderer._ds, true);
      }

      SetOneMatrix();
      D.wire      (Renderer.wire);
      D.depth     (true);
      D.cull      (true);
      D.stencil   (STENCIL_WATER_SET, STENCIL_REF_WATER);
      WS.WaterFlow   ->set(Time.time()*3);
      WS.WaterOfsCol ->set(_offset_col  );
      WS.WaterOfsNrm ->set(_offset_nrm  );
      WS.WaterOfsBump->set(_offset_bump );
      Rect uv=D.screenToUV(D.viewRect()); // UV
      if(Renderer._mirror_rt)
      {
         uv=Round(uv*Renderer._mirror_rt->size()); // convert to RectI of RT size
         uv.extend(-0.5f)/=Renderer._mirror_rt->size(); // decrease half pixel to avoid tex filtering issues and convert back to UV
         WS.WaterPlanePos->set(Renderer._mirror_plane.pos   *CamMatrixInv      );
         WS.WaterPlaneNrm->set(Renderer._mirror_plane.normal*CamMatrixInv.orn());
      }
      WS.WaterClamp->set(uv);
      setEyeViewportCam();
   }
}
void WaterClass::end()
{
   if(_began)
   {
     _began=false;

      D.wire   (false);
      D.stencil(STENCIL_NONE);

      MaterialClear(); // clear Materials because we've potentially set WaterMaterials which share the same textures

      if(!_use_secondary_rt)
      {
         endImages();
         Renderer._water_col.clear();
         Renderer._water_ds .clear();
         if(Renderer._ds!=Renderer._ds_1s && Renderer._ds_1s) // if we've drawn to MSAA ds, then we need to setup 1S DS
         {
            Renderer.set(null, Renderer._ds_1s, true);
            D.alpha(ALPHA_NONE);
            if(FUNC_DEFAULT!=FUNC_LESS)D.depthFunc(FUNC_LESS   ); D.depthLock  (true); Sh.ResolveDepth->draw(); // FUNC_LESS because 1S DS was already set before and we just need to apply water on top
            if(FUNC_DEFAULT!=FUNC_LESS)D.depthFunc(FUNC_DEFAULT); D.depthUnlock(    );
         }
      }
   #if DEPTH_FLUSH
      Renderer._modified_depth=true;
   #endif
   }
}
/******************************************************************************/
void WaterClass::under(C PlaneM &plane, WaterMtrl &mtrl)
{
   Flt step=(Dist(CamMatrix.pos, plane)-D.viewFromActual())/-WATER_TRANSITION+1;
   if( step>EPS_COL8)
   {
      if(!_under_mtrl || step>_under_step){_under_mtrl=&mtrl; _under_step=step; _under_plane=plane; _under_plane.pos+=_under_plane.normal*(D.viewFromActual()+WATER_TRANSITION);}
   }
}
/******************************************************************************/
void WaterClass::setImages(Image *src, Image *depth)
{
   // these are used by both draw surface and apply water shaders
   Sh.Img  [1]->set(Renderer._mirror_rt); // reflection
   Sh.Img  [2]->set(          src      ); // background underwater
   Sh.ImgXF[0]->set(          depth    ); // background depth
}
void WaterClass::endImages()
{
}
/******************************************************************************/
Bool WaterClass::bump()
{
   #define EPS_WAVE_SCALE 0.001f // 1 mm
   return _bump_map && wave_scale>EPS_WAVE_SCALE;
}
Shader* WaterClass::shader()
{
   return _use_secondary_rt ? (bump() ? WS.Ocean  : WS.Lake )
                            : (bump() ? WS.OceanL : WS.LakeL)[_shader_shadow_maps][_shader_soft][_shader_reflect_env][_shader_reflect_mirror][refract>EPS_MATERIAL_BUMP];
}
/******************************************************************************/
void WaterClass::drawSurfaces()
{
   // these are used only when '_use_secondary_rt' is disabled
   if(Lights.elms())
   {
      Light &light=Lights.first(); if(light.type==LIGHT_DIR && light.allow_main && light.shadow){_shader_shadow_maps=D.shadowMapNumActual(); goto light_set;}
   }
  _shader_shadow_maps   =0;
light_set:
  _shader_soft          =Renderer.canReadDepth();
  _shader_reflect_env   =(         D.envMap()!=null);
  _shader_reflect_mirror=(Renderer._mirror_rt!=null);

   REPS(Renderer._eye, Renderer._eye_num)
   {
      setEyeViewportCam();

      if(_draw_plane_surface)
         if(Shader *shader=T.shader())
      {
         begin();
         set  ();

         if(bump())
         {
            WS.WaterPlanePos->set(plane.pos   *CamMatrixInv      );
            WS.WaterPlaneNrm->set(plane.normal*CamMatrixInv.orn());
            WS.WaterYMulAdd ->set(_y_mul_add                     );
            shader->begin(); _mshr.set().draw();
         }else
         {
            VI.shader(shader);
            VI.cull  (true  );
            VI.quad  (_quad );
            VI.end   (      );
         }
      }

      if(!(_under_mtrl && _under_step>=1)) // don't draw any surfaces when totally under water
      {
         Renderer.mode(RM_WATER); Renderer._render();
      }

      // !! DRAW THIS LAST BECAUSE IT MIGHT USE CUSTOM DRAW MATRIX !! but those above need identity
      if(WaterBalls.elms())
      {
         Water.begin();
         REPAO(WaterBalls)->drawDo();
      }
   }

   end();
}
/******************************************************************************/
WaterClass& WaterClass::update(C Vec2 &vel)
{
  _offset_col +=vel *Time.d();
  _offset_nrm +=0.5f*Time.d();
  _offset_bump+=1.2f*Time.d();
   return T;
}
/******************************************************************************/
// WATER MESH
/******************************************************************************/
void WaterMesh::zero()
{
   depth=0;
  _lake =false;
  _box.zero();
}
WaterMesh::WaterMesh() {zero();}
void WaterMesh::del()
{
  _mshb.del();
  _mshr.del();
  _material.clear();
   zero();
}
void WaterMesh::create(C MeshBase &src, Bool lake, Flt depth, C WaterMtrlPtr &material)
{
   T._lake    =lake;
   T. depth   =depth;
   T._material=material;
  _mshb.create(src, VTX_POS|VTX_TEX0|TRI_IND|QUAD_IND).getBox(_box);
  _mshr.create(_mshb); // 'mshb' is kept for testing 'under'
   WS.load();
}
/******************************************************************************/
Bool WaterMesh::under(C Vec &pos, Flt *depth)C
{
   if(pos.x>=_box.min.x && pos.x<=_box.max.x
   && pos.z>=_box.min.z && pos.z<=_box.max.z
   && pos.y<=_box.max.y && pos.y>=_box.min.y-T.depth)
   {
      Bool flat=(_box.h()<=0.01f);

      // per face precision
      Vec2 pos2=pos.xz();
    C Vec *p   =_mshb.vtx.pos();
      REPA(_mshb.tri)
      {
       C VecI &ind=_mshb.tri.ind(i);
         Tri2  tri(p[ind.x].xz(), p[ind.y].xz(), p[ind.z].xz());
         if(Cuts(pos2, tri))
         {
            if(flat)
            {
               if(depth)*depth=_box.max.y-pos.y;
               return true;
            }else
            {
               Vec blend=TriBlend(pos2, tri);
               Flt water=p[ind.x].y*blend.x + p[ind.y].y*blend.y + p[ind.z].y*blend.z,
                   d    =water-pos.y;
               if( d>=0 && d<T.depth)
               {
                  if(depth)*depth=d;
                  return true;
               }
               return false;
            }
         }
      }
      REPA(_mshb.quad)
      {
       C VecI4 &ind=_mshb.quad.ind(i);
         Quad2  quad(p[ind.x].xz(), p[ind.y].xz(), p[ind.z].xz(), p[ind.w].xz());
         if(Cuts(pos2, quad))
         {
            if(flat)
            {
               if(depth)*depth=_box.max.y-pos.y;
               return true;
            }else
            {
               Vec blend=TriBlend(pos2, quad.tri013());
               Flt water=p[ind.x].y*blend.x + p[ind.y].y*blend.y + p[ind.w].y*blend.z,
                   d    =water-pos.y;
               if( d>=0 && d<T.depth)
               {
                  if(depth)*depth=d;
                  return true;
               }
               return false;
            }
         }
      }
   }
   return false;
}
/******************************************************************************/
Shader* WaterMesh::shader()C
{
   if(WaterMtrl *mtrl=getMaterial())
   {
      return Water._use_secondary_rt ? (_lake ? WS.Lake  : WS.River )
                                     : (_lake ? WS.LakeL : WS.RiverL)[Water._shader_shadow_maps][Water._shader_soft][Water._shader_reflect_env][Water._shader_reflect_mirror][mtrl->refract>EPS_MATERIAL_BUMP];
   }
   return null;
}
/******************************************************************************/
void WaterMesh::draw()C
{
   if(WaterMtrl *mtrl=getMaterial())switch(Water._mode)
   {
      case WaterClass::MODE_DRAW:
      {
         if(Frustum(_box) && _mshr.is())
            if(Shader *shader=T.shader())
         {
            Water  .begin();
            mtrl  ->set  ();
            shader->begin(); _mshr.set().draw();
         }
      }break;

      case WaterClass::MODE_REFLECTION:
      {
         if(_box.h()<=0.01f /*&& mtrl->reflect>EPS_COL8_NATIVE*/) // for reflections accept only flat waters, 'reflect' is taken globally
            if(CamMatrix.pos.y>=_box.min.y) // if camera is above surface
               if(Frustum(_box) && _mshr.is())
                  Renderer.requestMirror(PlaneM(VecD(0, _box.max.y, 0), Vec(0, 1, 0)), 0, Water.reflection_shadows, Water.reflection_resolution);
      }break;

      case WaterClass::MODE_UNDERWATER:
      {
         if(_mshb.vtxs())
         {
            Flt  depth, eps=D.viewFromActual()+WATER_TRANSITION;
            VecD test=CamMatrix.pos; test.y-=eps;
            if(under(test, &depth))
            {
               test.y+=depth;
               Water.under(PlaneM(test, Vec(0, 1, 0)), *mtrl);
            }
         }
      }break;
   }
}
/******************************************************************************/
Bool WaterMesh::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<_lake<<depth<<_box;
   if(_mshb.save(f))
   if(_mshr.save(f))
   {
      f._putStr(_material.name(path));
      return f.ok();
   }
   return false;
}
Bool WaterMesh::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>_lake>>depth>>_box;
         if(_mshb.load(f))
         if(_mshr.load(f))
         {
           _material.require(f._getStr(), path);
            if(f.ok())
            {
               WS.load();
               return true;
            }
         }
      }break;
   }
   del(); return false;
}
/******************************************************************************/
// WATER BALL
/******************************************************************************/
void WaterBall::draw()C
{
   DEBUG_ASSERT(Renderer()==RM_PREPARE, "'WaterBall.draw' called outside of RM_PREPARE");
   if(Frustum(T) && Renderer.firstPass())
   {
      Flt eps=D.viewFromActual()+WATER_TRANSITION;
      Vec delta=CamMatrix.pos-pos;
      Flt delta_len=delta.normalize();
      Flt dist=delta_len-r;
      if( dist>0) // above surface
      {
         if(Dot(delta, _uv_plane.z)<0.9f) // if new direction (from ball pos to camera pos) is much different than last saved, then recalc uv plane. this value is smallest that doesn't introduce too much stretching
            ConstCast(_uv_plane).rotateToZKeepY(delta); // try to preserve up
         WaterBalls.add(&T);
      }
      if(dist<eps) // under surface
      {
         Water.under(PlaneM(pos+delta*r, delta), *material);
      }
   }
}
void WaterBall::drawDo()C
{
   Flt  draw_r=r/SKY_MESH_MIN_DIST;
   Flt  dist2 =Dist2(ActiveCam.matrix.pos, pos); // use 'ActiveCam' instead of 'CamMatrix' because it's not affected by eyes
   Bool flat  =(dist2<=Sqr(draw_r+FrustumMain.view_quad_max_dist+D.eyeDistance_2())); // use flat if camera intersects with mesh

   if(Shader *shader=Water._use_secondary_rt ? WS.Ball                                                                                                                                              [flat]
                                             : WS.BallL[Water._shader_shadow_maps][Water._shader_soft][Water._shader_reflect_env][Water._shader_reflect_mirror][material->refract>EPS_MATERIAL_BUMP][flat])
   {
      MatrixM matrix; if(!flat)
      {
         matrix.setScalePos(-draw_r, pos); // reverse faces because 'Sky._mshr' is reversed
         Sky._mshr.set();
         D.depth(true);
         D.cull (true);
      }else
      {
         D.depthLock(true);
      }

      material->set();

    //REPS(Renderer._eye, Renderer._eye_num) this is already under loop
      {
       //Renderer.setEyeViewportCam();
         WS.WaterBallPosRadius->set(Vec4(Vec(CamMatrix.pos-pos)*CamMatrixInv.orn(), r)); if(!flat)SetFastMatrix(matrix); // set these after 'setEyeViewportCam'. position that we set is for camera relative to ball. because shader assumes ball is at Vec(0,0,0) and the position that we specify is camera position relative to ball in view space
         WS.WaterBallX->set(_uv_plane.x*CamMatrixInv.orn());
         WS.WaterBallY->set(_uv_plane.y*CamMatrixInv.orn());
         if(flat)
         {
            Rect rect; if(toScreenRect(rect))if(!Renderer._stereo || ToEyeRect(rect))shader->draw(&rect);
         }else{shader->begin(); Sky._mshr.draw();} // call this next
      }
      if(flat)D.depthUnlock();
   }
}
/******************************************************************************/
// WATER DROPS
/******************************************************************************
struct WaterDrops
{
   Int    num   ; // number of water drops
   Vec   *pos   , // position
         *nrm   , // normal  (1st random direction)
         *tan   ; // tangent (2nd random direction)
   Flt   *time  , // time offset
         *blend ; // bone blend value
   VecB4 *matrix; // bone matrix index
   Image *image ; // drop texture

   // manage
   WaterDrops& del   (                                              ); // delete
   WaterDrops& create(Image &image,Int num                          ); // create with 'num' drops
   WaterDrops& create(Image &image,Int num,Shape   *shape,Int shapes); // create from shapes
   WaterDrops& create(Image &image,Int num,Ragdoll &ragdoll         ); // create from ragdoll

   // operations
   void setSkin(Skeleton &skeleton); // recalculate bone skinning values according to 'skeleton'

   // draw
   void draw(                           ); // render with  active     matrix
   void draw(Matrix           &matrix   ); // render with 'matrix'    matrix
   void draw(AnimatedSkeleton &anim_skel); // render with 'anim_skel' matrixes

  ~WaterDrops() {del ( );}
   WaterDrops() {Zero(T);}

   NO_COPY_CONSTRUCTOR(WaterDrops);
};
/******************************************************************************
WaterDrops& WaterDrops::del()
{
   Free(pos);
   Free(nrm);
   Free(tan);
   Free(time);
   Free(blend);
   Free(matrix);
   Zero(T); return T;
}
WaterDrops& WaterDrops::create(Image &image,Int num)
{
   del();

   T.image=&image;
   T.num  = num  ;
   Alloc(pos ,num);
   Alloc(nrm ,num);
   Alloc(tan ,num);
   Alloc(time,num);
   return T;
}
WaterDrops& WaterDrops::create(Image &image,Int num,Shape *shape,Int shapes)
{
   create(image,num);

   Box  box;
   Bool box_set=false;
   REP(shapes)
   {
      Box    b;
      Shape &s=shape[i];
      if(s.type==SHAPE_BALL)
      {
         Ball &ball=s.ball;
         b.set(ball.pos-ball.r,ball.pos+ball.r);
      }else
      if(s.type==SHAPE_CAPSULE)
      {
         Capsule &capsule=s.capsule;
         Vec      up     =capsule.up*capsule.innerHeightHalf();
         b.from(capsule.pos-up,capsule.pos+up).extend(capsule.r);
      }else continue;
      if(!box_set){box_set=true; box=b;}else box|=b;
   }
   REP(num)
   {
      pos [i].zero();
      nrm [i]=Random  (Ball(1),false)*Random.f(1/1.2f,1.2f)*0.008f;
      tan [i]=Random  (Ball(1),false)*Random.f(1/1.2f,1.2f)*0.008f;
      time[i]=Random.f(10);
      Vec &p=pos[i];
      REPD(a,256)
      {
         p=Random(box);
         Bool cut=false;
         REP(shapes)
         {
            Shape &s=shape[i];
            if(s.type==SHAPE_BALL    && Cuts(p,s.ball   )){cut=true; break;}else
            if(s.type==SHAPE_CAPSULE && Cuts(p,s.capsule)){cut=true; break;}
         }
         if(cut)break;
      }
   }
   return T;
}
WaterDrops& WaterDrops::create(Image &image,Int num,Ragdoll &ragdoll)
{
   return create(image,num,ragdoll.shape,ragdoll.bones);
}
/******************************************************************************
void WaterDrops::setSkin(Skeleton &skeleton)
{
   if(!blend )Alloc(blend ,num);
   if(!matrix)Alloc(matrix,num);
   REP(num)skeleton.getSkin(pos[i],blend[i],matrix[i]);
}
/******************************************************************************
void WaterDrops::draw()
{
   VI.image    (image);
   VI.technique(blend ? Sh.RndrWaterdS : Sh.RndrWaterd);
   REP(num)
   {
      Flt t1=Sin(Time.time()*1.5f+time[i]),
          t2=Sin(Time.time()*1.7f+time[i]);
      Vec p=pos[i]+t1*nrm[i]+t2*tan[i];
      if(blend)VI.image(WHITE,0.020f,p,blend[i],matrix[i]);
      else     VI.image(WHITE,0.020f,p);
   }
   VI.end();
}
void WaterDrops::draw(Matrix &matrix)
{
   SetOneMatrix(matrix);
   draw();
}
void WaterDrops::draw(AnimatedSkeleton &anim_skel)
{
   anim_skel.setMatrix();
   draw();
}
/******************************************************************************/
}
/******************************************************************************/
