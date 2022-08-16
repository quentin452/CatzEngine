/******************************************************************************/
#include "stdafx.h"
namespace EE{
#if 0
   #define TEX_ZERO HalfZero
   #define TEX_ONE  HalfOne
   #define SET_TEX(t, x, y) t.set(x, y)
#else
   #define TEX_ZERO 0
   #define TEX_ONE  255
   #define SET_TEX(t, x, y) t.set(x, y, 0, 0)
#endif
/******************************************************************************/
DisplayDraw::DisplayDraw()
{
  _text_depth=false;
}
/******************************************************************************
   static Vec2 posToScreen(Vec2 &pos   ); // convert from world  position to screen position, according to 'scale offset'
   static Vec2 screenToPos(Vec2 &screen); // convert from screen position to world  position, according to 'scale offset'
Vec2 DisplayDraw::posToScreen(Vec2 &pos   ){return D._scale * (pos+D._offset);}
Vec2 DisplayDraw::screenToPos(Vec2 &screen){return screen/D._scale-D._offset ;}
/******************************************************************************/
void DisplayDraw::lineX(C Color &color, Flt y, Flt x0, Flt x1)
{
   VI.color  (color);
   VI.setType(VI_2D_FLAT, VI_LINE);
   if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(2))
   {
      v[0].pos.set(x0, y);
      v[1].pos.set(x1, y);
   }
   VI.end();
}
void DisplayDraw::lineY(C Color &color, Flt x, Flt y0, Flt y1)
{
   VI.color  (color);
   VI.setType(VI_2D_FLAT, VI_LINE);
   if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(2))
   {
      v[0].pos.set(x, y0);
      v[1].pos.set(x, y1);
   }
   VI.end();
}
void DisplayDraw::lines(C Color &color, C Vec2 *point, Int points)
{
   if(point && points>1)
   {
      VI.color(color);
      if(points*SIZE(*point)>VI._mem_max)
      {
         VI.setType(VI_2D_FLAT, VI_LINE);
         Int  lines=points-1;
         FREP(lines)if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(2))
         {
            v[0].pos=point[i  ];
            v[1].pos=point[i+1];
         }
      }else
      {
         VI.setType(VI_2D_FLAT, VI_LINE|VI_STRIP);
         if(Vtx2DFlat *v=(Vtx2DFlat*)VI.addVtx(points))CopyFast(v, point, points*SIZE(*v));
      }
      VI.end();
   }
}
/******************************************************************************/
void Image::draw(C Rect &rect)C
{
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      v[0].tex.set(      0,       0);
      v[1].tex.set(_part.x,       0);
      v[2].tex.set(      0, _part.y);
      v[3].tex.set(_part.x, _part.y);
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::draw(C Color &color, C Color &color_add, C Rect &rect)C
{
   VI.color  (color    );
   VI.color1 (color_add);
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      v[0].tex.set(      0,       0);
      v[1].tex.set(_part.x,       0);
      v[2].tex.set(      0, _part.y);
      v[3].tex.set(_part.x, _part.y);
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::draw(C Vec4 &color, C Vec4 &color_add, C Rect &rect)C
{
   VI.color  (color    );
   VI.color1 (color_add);
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      v[0].tex.set(      0,       0);
      v[1].tex.set(_part.x,       0);
      v[2].tex.set(      0, _part.y);
      v[3].tex.set(_part.x, _part.y);
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::drawVertical(C Rect &rect)C
{
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      v[0].tex.set(_part.y,       0);
      v[1].tex.set(_part.y, _part.x);
      v[2].tex.set(      0,       0);
      v[3].tex.set(      0, _part.x);
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::drawVertical(C Color &color, C Color &color_add, C Rect &rect)C
{
   VI.color  (color    );
   VI.color1 (color_add);
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      v[0].tex.set(_part.y,       0);
      v[1].tex.set(_part.y, _part.x);
      v[2].tex.set(      0,       0);
      v[3].tex.set(      0, _part.x);
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::drawFilter(C Rect &rect, FILTER_TYPE filter)C
{
   Vec2  pixelf=Renderer.screenToPixelSize(rect.size());
   VecI2 pixel =Round(pixelf).abs(); // get target pixel size
   if(pixel.x>w() || pixel.y>h())switch(filter) // if that size is bigger than the image resolution
   {
    //case FILTER_LINEAR: VI.shader(null); break;

      case FILTER_NONE: VI.shader(Sh.DrawTexPoint); break; // or SamplerPoint.setPS(SSI_DEFAULT_2D);

      case FILTER_CUBIC_FAST       :
      case FILTER_CUBIC_FAST_SMOOTH:
      case FILTER_CUBIC_FAST_SHARP : Sh.imgSize(T); VI.shader(Sh.DrawTexCubicFast[false]); break;

      case FILTER_CUBIC_PLUS      :
      case FILTER_CUBIC_PLUS_SHARP: Sh.imgSize(T); Sh.loadCubicShaders(); VI.shader(Sh.DrawTexCubicPlus[false]); break;

      case FILTER_BEST :
      case FILTER_WAIFU: // fall back to best available shaders
      case FILTER_EASU :
      {
         VI.shader(Sh.EASUScreen[false]);
         SetEASU(T, pixelf, rect.lu());
      }break;
   }
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      v[0].tex.set(      0,       0);
      v[1].tex.set(_part.x,       0);
      v[2].tex.set(      0, _part.y);
      v[3].tex.set(_part.x, _part.y);
   }
   VI.end();
 //if(filter==FILTER_NONE)SamplerLinearClamp.setPS(SSI_DEFAULT_2D);
}
void Image::drawFilter(C Color &color, C Color &color_add, C Rect &rect, FILTER_TYPE filter)C
{
   Vec2  pixelf=Renderer.screenToPixelSize(rect.size());
   VecI2 pixel =Round(pixelf).abs(); // get target pixel size
   if(pixel.x>w() || pixel.y>h())switch(filter) // if that size is bigger than the image resolution
   {
    //case FILTER_LINEAR: VI.shader(null); break;

      case FILTER_NONE: VI.shader(Sh.DrawTexPointC); break; // or SamplerPoint.setPS(SSI_DEFAULT_2D);

      case FILTER_CUBIC_FAST       :
      case FILTER_CUBIC_FAST_SMOOTH:
      case FILTER_CUBIC_FAST_SHARP : Sh.imgSize(T); VI.shader(Sh.DrawTexCubicFast[true]); break;

      case FILTER_CUBIC_PLUS      :
      case FILTER_CUBIC_PLUS_SHARP: Sh.imgSize(T); Sh.loadCubicShaders(); VI.shader(Sh.DrawTexCubicPlus[true]); break;

      case FILTER_BEST :
      case FILTER_WAIFU: // fall back to best available shaders
      case FILTER_EASU :
      {
         VI.shader(Sh.EASUScreen[true]);
         SetEASU(T, pixelf, rect.lu());
      }break;
   }
   VI.color  (color    );
   VI.color1 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      v[0].tex.set(      0,       0);
      v[1].tex.set(_part.x,       0);
      v[2].tex.set(      0, _part.y);
      v[3].tex.set(_part.x, _part.y);
   }
   VI.end();
 //if(filter==FILTER_NONE)SamplerLinearClamp.setPS(SSI_DEFAULT_2D);
}
/******************************************************************************
void Image::drawOutline(C Color &color, C Rect &rect, Flt tex_range)
{
   if(color.a)
   {
      VI.shader (Sh.OutlineI);
      VI.color  (color      );
      VI.image  (this       );
      VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
      if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
      {
         Vec2 tr=tex_range;
         v[0].pos.set(rect.min.x, rect.max.y);
         v[1].pos.set(rect.max.x, rect.max.y);
         v[2].pos.set(rect.min.x, rect.min.y);
         v[3].pos.set(rect.max.x, rect.min.y);
         v[0].tex.set(      0,       0);
         v[1].tex.set(_part.x,       0);
         v[2].tex.set(      0, _part.y);
         v[3].tex.set(_part.x, _part.y);
         Sh.imgSize(T);
      }
      VI.end();
   }
}
/******************************************************************************/
#define DEFAULT_FILTER (MOBILE ? FILTER_LINEAR : FILTER_BEST)
void Image::drawFs(                                    FIT_MODE fit, Int filter)C {drawFilter(                  T.fit(D.rect(), fit), (filter>=0) ? FILTER_TYPE(filter) : DEFAULT_FILTER);}
void Image::drawFs(C Color &color, C Color &color_add, FIT_MODE fit, Int filter)C {drawFilter(color, color_add, T.fit(D.rect(), fit), (filter>=0) ? FILTER_TYPE(filter) : DEFAULT_FILTER);}
/******************************************************************************/
void Image::drawPart(C Rect &screen_rect, C Rect &tex_rect)C
{
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(screen_rect.min.x, screen_rect.max.y);
      v[1].pos.set(screen_rect.max.x, screen_rect.max.y);
      v[2].pos.set(screen_rect.min.x, screen_rect.min.y);
      v[3].pos.set(screen_rect.max.x, screen_rect.min.y);
      if(partial())
      {
         v[0].tex.x=v[2].tex.x=tex_rect.min.x*_part.x;
         v[1].tex.x=v[3].tex.x=tex_rect.max.x*_part.x;
         v[0].tex.y=v[1].tex.y=tex_rect.min.y*_part.y;
         v[2].tex.y=v[3].tex.y=tex_rect.max.y*_part.y;
      }else
      {
         v[0].tex.set(tex_rect.min.x, tex_rect.min.y);
         v[1].tex.set(tex_rect.max.x, tex_rect.min.y);
         v[2].tex.set(tex_rect.min.x, tex_rect.max.y);
         v[3].tex.set(tex_rect.max.x, tex_rect.max.y);
      }
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::drawPart(C Color &color, C Color &color_add, C Rect &screen_rect, C Rect &tex_rect)C
{
   VI.color  (color    );
   VI.color1 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(screen_rect.min.x, screen_rect.max.y);
      v[1].pos.set(screen_rect.max.x, screen_rect.max.y);
      v[2].pos.set(screen_rect.min.x, screen_rect.min.y);
      v[3].pos.set(screen_rect.max.x, screen_rect.min.y);
      if(partial())
      {
         v[0].tex.x=v[2].tex.x=tex_rect.min.x*_part.x;
         v[1].tex.x=v[3].tex.x=tex_rect.max.x*_part.x;
         v[0].tex.y=v[1].tex.y=tex_rect.min.y*_part.y;
         v[2].tex.y=v[3].tex.y=tex_rect.max.y*_part.y;
      }else
      {
         v[0].tex.set(tex_rect.min.x, tex_rect.min.y);
         v[1].tex.set(tex_rect.max.x, tex_rect.min.y);
         v[2].tex.set(tex_rect.min.x, tex_rect.max.y);
         v[3].tex.set(tex_rect.max.x, tex_rect.max.y);
      }
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::drawPart(C Vec4 &color, C Vec4 &color_add, C Rect &screen_rect, C Rect &tex_rect)C
{
   VI.color  (color    );
   VI.color1 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(screen_rect.min.x, screen_rect.max.y);
      v[1].pos.set(screen_rect.max.x, screen_rect.max.y);
      v[2].pos.set(screen_rect.min.x, screen_rect.min.y);
      v[3].pos.set(screen_rect.max.x, screen_rect.min.y);
      if(partial())
      {
         v[0].tex.x=v[2].tex.x=tex_rect.min.x*_part.x;
         v[1].tex.x=v[3].tex.x=tex_rect.max.x*_part.x;
         v[0].tex.y=v[1].tex.y=tex_rect.min.y*_part.y;
         v[2].tex.y=v[3].tex.y=tex_rect.max.y*_part.y;
      }else
      {
         v[0].tex.set(tex_rect.min.x, tex_rect.min.y);
         v[1].tex.set(tex_rect.max.x, tex_rect.min.y);
         v[2].tex.set(tex_rect.min.x, tex_rect.max.y);
         v[3].tex.set(tex_rect.max.x, tex_rect.max.y);
      }
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::drawPartVertical(C Rect &screen_rect, C Rect &tex_rect)C
{
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(screen_rect.min.x, screen_rect.min.y);
      v[1].pos.set(screen_rect.min.x, screen_rect.max.y);
      v[2].pos.set(screen_rect.max.x, screen_rect.min.y);
      v[3].pos.set(screen_rect.max.x, screen_rect.max.y);
      if(partial())
      {
         v[0].tex.x=v[2].tex.x=tex_rect.min.x*_part.x;
         v[1].tex.x=v[3].tex.x=tex_rect.max.x*_part.x;
         v[0].tex.y=v[1].tex.y=tex_rect.min.y*_part.y;
         v[2].tex.y=v[3].tex.y=tex_rect.max.y*_part.y;
      }else
      {
         v[0].tex.set(tex_rect.min.x, tex_rect.min.y);
         v[1].tex.set(tex_rect.max.x, tex_rect.min.y);
         v[2].tex.set(tex_rect.min.x, tex_rect.max.y);
         v[3].tex.set(tex_rect.max.x, tex_rect.max.y);
      }
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
void Image::drawPartVertical(C Color &color, C Color &color_add, C Rect &screen_rect, C Rect &tex_rect)C
{
   VI.color  (color    );
   VI.color1 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(screen_rect.min.x, screen_rect.min.y);
      v[1].pos.set(screen_rect.min.x, screen_rect.max.y);
      v[2].pos.set(screen_rect.max.x, screen_rect.min.y);
      v[3].pos.set(screen_rect.max.x, screen_rect.max.y);
      if(partial())
      {
         v[0].tex.x=v[2].tex.x=tex_rect.min.x*_part.x;
         v[1].tex.x=v[3].tex.x=tex_rect.max.x*_part.x;
         v[0].tex.y=v[1].tex.y=tex_rect.min.y*_part.y;
         v[2].tex.y=v[3].tex.y=tex_rect.max.y*_part.y;
      }else
      {
         v[0].tex.set(tex_rect.min.x, tex_rect.min.y);
         v[1].tex.set(tex_rect.max.x, tex_rect.min.y);
         v[2].tex.set(tex_rect.min.x, tex_rect.max.y);
         v[3].tex.set(tex_rect.max.x, tex_rect.max.y);
      }
   }
   VI.end(); // always call 'VI.end' in case 'VI.shader' was overriden before calling current method
}
/******************************************************************************/
void Image::drawRotate(C Vec2 &center, C Vec2 &size, Flt angle, C Vec2 *rotation_center_uv)C
{
   VI.image  (this);
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      Vec2 c=(rotation_center_uv ? *rotation_center_uv : 0.5f);
           c.x*= size.x;
           c.y*=-size.y;
      Matrix2P m; m.orn().setRotate(angle); m.pos=center-c*m.orn();
      v[0].pos.set(     0,       0)*=m;
      v[1].pos.set(size.x,       0)*=m;
      v[2].pos.set(     0, -size.y)*=m;
      v[3].pos.set(size.x, -size.y)*=m;
      v[0].tex.set(      0,       0);
      v[1].tex.set(_part.x,       0);
      v[2].tex.set(      0, _part.y);
      v[3].tex.set(_part.x, _part.y);
   }
   VI.end();
}
void Image::drawRotate(C Color &color, C Color &color_add, C Vec2 &center, C Vec2 &size, Flt angle, C Vec2 *rotation_center_uv)C
{
   VI.color  (color    );
   VI.color1 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      Vec2 c=(rotation_center_uv ? *rotation_center_uv : 0.5f);
           c.x*= size.x;
           c.y*=-size.y;
      Matrix2P m; m.orn().setRotate(angle); m.pos=center-c*m.orn();
      v[0].pos.set(     0,       0)*=m;
      v[1].pos.set(size.x,       0)*=m;
      v[2].pos.set(     0, -size.y)*=m;
      v[3].pos.set(size.x, -size.y)*=m;
      v[0].tex.set(      0,       0);
      v[1].tex.set(_part.x,       0);
      v[2].tex.set(      0, _part.y);
      v[3].tex.set(_part.x, _part.y);
   }
   VI.end();
}
/******************************************************************************/
static void DrawMask(C Image &image, C Color &color, C Color &color_add, C Rect &rect, C Image &mask, C Rect &mask_rect, C Shader *shader)
{
   Rect r; r.from(     rect.min,      rect.max); // needed in case      'rect' is flipped
   Rect m; m.from(mask_rect.min, mask_rect.max); // needed in case 'mask_rect' is flipped
   Rect rm=r&m;
   if(  rm.valid())
   {
      VI.color (color    );
      VI.color1(color_add);
      Sh.Img[0]->set(image);
      Sh.Img[1]->set(mask );
      VI.setType(VI_2D_TEX2, VI_STRIP);
      VI.shader (shader);
      if(Vtx2DTex2 *v=(Vtx2DTex2*)VI.addVtx(4))
      {
         v[0].pos.set(rm.min.x, rm.max.y);
         v[1].pos.set(rm.max.x, rm.max.y);
         v[2].pos.set(rm.min.x, rm.min.y);
         v[3].pos.set(rm.max.x, rm.min.y);

         v[0].tex[0].x=v[2].tex[0].x=LerpR(rect.min.x, rect.max.x, rm.min.x); // min x
         v[1].tex[0].x=v[3].tex[0].x=LerpR(rect.min.x, rect.max.x, rm.max.x); // max x
         v[0].tex[0].y=v[1].tex[0].y=LerpR(rect.max.y, rect.min.y, rm.max.y); // min y
         v[2].tex[0].y=v[3].tex[0].y=LerpR(rect.max.y, rect.min.y, rm.min.y); // max y

         v[0].tex[1].x=v[2].tex[1].x=LerpR(mask_rect.min.x, mask_rect.max.x, rm.min.x); // min x
         v[1].tex[1].x=v[3].tex[1].x=LerpR(mask_rect.min.x, mask_rect.max.x, rm.max.x); // max x
         v[0].tex[1].y=v[1].tex[1].y=LerpR(mask_rect.max.y, mask_rect.min.y, rm.max.y); // min y
         v[2].tex[1].y=v[3].tex[1].y=LerpR(mask_rect.max.y, mask_rect.min.y, rm.min.y); // max y

         if(image.partial())
         {
            v[0].tex[0]*=image._part.xy;
            v[1].tex[0]*=image._part.xy;
            v[2].tex[0]*=image._part.xy;
            v[3].tex[0]*=image._part.xy;
         }
         if(mask.partial())
         {
            v[0].tex[1]*=mask._part.xy;
            v[1].tex[1]*=mask._part.xy;
            v[2].tex[1]*=mask._part.xy;
            v[3].tex[1]*=mask._part.xy;
         }
      }
      VI.end();
   }
}
void Image::drawMask         (C Color &color, C Color &color_add, C Rect &rect, C Image &mask, C Rect &mask_rect)C {DrawMask(T, color, color_add, rect, mask, mask_rect, Sh.DrawMask[0][0]);}
void Image::drawMaskNoFilter (C Color &color, C Color &color_add, C Rect &rect, C Image &mask, C Rect &mask_rect)C {DrawMask(T, color, color_add, rect, mask, mask_rect, Sh.DrawMask[0][1]);}
void Image::drawMaskA        (C Color &color, C Color &color_add, C Rect &rect, C Image &mask, C Rect &mask_rect)C {DrawMask(T, color, color_add, rect, mask, mask_rect, Sh.DrawMask[1][0]);}
void Image::drawMaskANoFilter(C Color &color, C Color &color_add, C Rect &rect, C Image &mask, C Rect &mask_rect)C {DrawMask(T, color, color_add, rect, mask, mask_rect, Sh.DrawMask[1][1]);}
/******************************************************************************/
void Image::drawTile(C Rect &rect, Flt tex_scale)C
{
   VI.image  (this);
   VI.wrap   (    );
   VI.setType(VI_2D_TEX, VI_STRIP);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      Flt w=rect.w()*tex_scale,
          h=rect.h()*tex_scale*aspect();
      v[0].tex.set(0, 0);
      v[1].tex.set(w, 0);
      v[2].tex.set(0, h);
      v[3].tex.set(w, h);
   }
   VI.end();
}
void Image::drawTile(C Color &color, C Color &color_add, C Rect &rect, Flt tex_scale)C
{
   VI.color  (color    );
   VI.color1 (color_add);
   VI.image  (this     );
   VI.wrap   (         );
   VI.setType(VI_2D_TEX, VI_STRIP|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(4))
   {
      v[0].pos.set(rect.min.x, rect.max.y);
      v[1].pos.set(rect.max.x, rect.max.y);
      v[2].pos.set(rect.min.x, rect.min.y);
      v[3].pos.set(rect.max.x, rect.min.y);
      Flt w=rect.w()*tex_scale,
          h=rect.h()*tex_scale*aspect();
      v[0].tex.set(0, 0);
      v[1].tex.set(w, 0);
      v[2].tex.set(0, h);
      v[3].tex.set(w, h);
   }
   VI.end();
}
/******************************************************************************/
void Image::drawBorder(C Rect &rect, Flt border, Flt tex_scale, Flt tex_offset, Bool wrap_mode)C
{
   VI.image  (this);
   VI.wrapX  (    );
   VI.setType(VI_2D_TEX, wrap_mode ? VI_STRIP : 0);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(10))
   {
      Flt x1=rect.min.x, x2=rect.max.x, x0=x1-border, x3=x2+border,
          y1=rect.min.y, y2=rect.max.y, y0=y1-border, y3=y2+border;

      Flt scale=border*aspect(),
          w    =rect.w()/scale,
          h    =rect.h()/scale,
          b    =border  /scale;

      if(wrap_mode)
      {
         v[0].pos.set(x1, y2);
         v[1].pos.set(x0, y3);

         v[2].pos.set(x2, y2);
         v[3].pos.set(x3, y3);

         v[4].pos.set(x2, y1);
         v[5].pos.set(x3, y0);

         v[6].pos.set(x1, y1);
         v[7].pos.set(x0, y0);

         v[8].pos.set(x1, y2);
         v[9].pos.set(x0, y3);

         v[0].tex.set(      0, _part.y);
         v[1].tex.set(      0,       0);
         v[2].tex.set(w      , _part.y);
         v[3].tex.set(w      ,       0);
         v[4].tex.set(w+h    , _part.y);
         v[5].tex.set(w+h    ,       0);
         v[6].tex.set(w+h+w  , _part.y);
         v[7].tex.set(w+h+w  ,       0);
         v[8].tex.set(w+h+w+h, _part.y);
         v[9].tex.set(w+h+w+h,       0);
      }else
      {
         v[0].pos.set(x1, y2);
         v[1].pos.set(x2, y2);
         v[2].pos.set(x2, y1);
         v[3].pos.set(x1, y1);
         v[4].pos.set(x0, y3);
         v[5].pos.set(x3, y3);
         v[6].pos.set(x3, y0);
         v[7].pos.set(x0, y0);
         v[8].pos.set(x2, y1);
         v[9].pos.set(x3, y0);

         v[0].tex.set(    0, _part.y);
         v[1].tex.set(w    , _part.y);
         v[2].tex.set(w-h  , _part.y);
         v[3].tex.set(  h  , _part.y);
         v[4].tex.set(   -b,       0);
         v[5].tex.set(w  +b,       0);
         v[6].tex.set(w-h-b,       0);
         v[7].tex.set(  h+b,       0);
         v[8].tex.set(h-w  , _part.y);
         v[9].tex.set(h-w-b,       0);
      }

      if(tex_scale!=1 || tex_offset!=0)REP(10)v[i].tex.x=v[i].tex.x*tex_scale+tex_offset;

      if(wrap_mode)VI.end();else VI.flushIndexed(IndBufBorder, 4*2*3);
   }
   VI.clear();
}
void Image::drawBorder(C Color &color, C Color &color_add, C Rect &rect, Flt border, Flt tex_scale, Flt tex_offset, Bool wrap_mode)C
{
   VI.color  (color    );
   VI.color1 (color_add);
   VI.image  (this     );
   VI.wrapX  (         );
   VI.setType(VI_2D_TEX, (wrap_mode ? VI_STRIP : 0)|VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(10))
   {
      Flt x1=rect.min.x, x2=rect.max.x, x0=x1-border, x3=x2+border,
          y1=rect.min.y, y2=rect.max.y, y0=y1-border, y3=y2+border;

      Flt scale=border*aspect(),
          w    =rect.w()/scale,
          h    =rect.h()/scale,
          b    =border  /scale;

      if(wrap_mode)
      {
         v[0].pos.set(x1, y2);
         v[1].pos.set(x0, y3);

         v[2].pos.set(x2, y2);
         v[3].pos.set(x3, y3);

         v[4].pos.set(x2, y1);
         v[5].pos.set(x3, y0);

         v[6].pos.set(x1, y1);
         v[7].pos.set(x0, y0);

         v[8].pos.set(x1, y2);
         v[9].pos.set(x0, y3);

         v[0].tex.set(      0, _part.y);
         v[1].tex.set(      0,       0);
         v[2].tex.set(w      , _part.y);
         v[3].tex.set(w      ,       0);
         v[4].tex.set(w+h    , _part.y);
         v[5].tex.set(w+h    ,       0);
         v[6].tex.set(w+h+w  , _part.y);
         v[7].tex.set(w+h+w  ,       0);
         v[8].tex.set(w+h+w+h, _part.y);
         v[9].tex.set(w+h+w+h,       0);
      }else
      {
         v[0].pos.set(x1, y2);
         v[1].pos.set(x2, y2);
         v[2].pos.set(x2, y1);
         v[3].pos.set(x1, y1);
         v[4].pos.set(x0, y3);
         v[5].pos.set(x3, y3);
         v[6].pos.set(x3, y0);
         v[7].pos.set(x0, y0);
         v[8].pos.set(x2, y1);
         v[9].pos.set(x3, y0);

         v[0].tex.set(    0, _part.y);
         v[1].tex.set(w    , _part.y);
         v[2].tex.set(w-h  , _part.y);
         v[3].tex.set(  h  , _part.y);
         v[4].tex.set(   -b,       0);
         v[5].tex.set(w  +b,       0);
         v[6].tex.set(w-h-b,       0);
         v[7].tex.set(  h+b,       0);
         v[8].tex.set(h-w  , _part.y);
         v[9].tex.set(h-w-b,       0);
      }

      if(tex_scale!=1 || tex_offset!=0)REP(10)v[i].tex.x=v[i].tex.x*tex_scale+tex_offset;

      if(wrap_mode)VI.end();else VI.flushIndexed(IndBufBorder, 4*2*3);
   }
   VI.clear();
}
/******************************************************************************/
void Image::drawFadeLR(C Color &color, C Rect &rect, Flt trans_l, Flt opaque_l, Flt opaque_r, Flt trans_r)C
{
   VI.image  (this);
   VI.setType(VI_2D_TEX_COL);
   if(Vtx2DTexCol *v=(Vtx2DTexCol*)VI.addVtx(16))
   {
      Flt min, max; MinMax(rect.min.x, rect.max.x, min, max); // this is needed in case 'rect' is flipped
      Flt x0=Mid(trans_l, min, max), x3=Mid(trans_r, min, max), x1=Mid(opaque_l, x0, x3), x2=Mid(opaque_r, x0, x3),
          y0=rect.min.y, y1=rect.min.y, y2=rect.max.y, y3=rect.max.y;

      Flt u0=LerpR(rect.min.x, rect.max.x, x0),
          u1=LerpR(rect.min.x, rect.max.x, x1),
          u2=LerpR(rect.min.x, rect.max.x, x2),
          u3=LerpR(rect.min.x, rect.max.x, x3),
          v0=0,
          v1=0,
          v2=1,
          v3=1;

      Color trans=color; trans.a=0;

      v[ 0].pos.set(x0, y3);
      v[ 1].pos.set(x1, y3);
      v[ 2].pos.set(x2, y3);
      v[ 3].pos.set(x3, y3);
      v[ 4].pos.set(x0, y2);
      v[ 5].pos.set(x1, y2);
      v[ 6].pos.set(x2, y2);
      v[ 7].pos.set(x3, y2);
      v[ 8].pos.set(x0, y1);
      v[ 9].pos.set(x1, y1);
      v[10].pos.set(x2, y1);
      v[11].pos.set(x3, y1);
      v[12].pos.set(x0, y0);
      v[13].pos.set(x1, y0);
      v[14].pos.set(x2, y0);
      v[15].pos.set(x3, y0);

      v[ 0].tex.set(u0, v0);
      v[ 1].tex.set(u1, v0);
      v[ 2].tex.set(u2, v0);
      v[ 3].tex.set(u3, v0);
      v[ 4].tex.set(u0, v1);
      v[ 5].tex.set(u1, v1);
      v[ 6].tex.set(u2, v1);
      v[ 7].tex.set(u3, v1);
      v[ 8].tex.set(u0, v2);
      v[ 9].tex.set(u1, v2);
      v[10].tex.set(u2, v2);
      v[11].tex.set(u3, v2);
      v[12].tex.set(u0, v3);
      v[13].tex.set(u1, v3);
      v[14].tex.set(u2, v3);
      v[15].tex.set(u3, v3);

      v[ 0].color=trans;
      v[ 1].color=trans;
      v[ 2].color=trans;
      v[ 3].color=trans;
      v[ 4].color=trans;
      v[ 5].color=color;
      v[ 6].color=color;
      v[ 7].color=trans;
      v[ 8].color=trans;
      v[ 9].color=color;
      v[10].color=color;
      v[11].color=trans;
      v[12].color=trans;
      v[13].color=trans;
      v[14].color=trans;
      v[15].color=trans;

      if(partial())REP(16)v[i].tex*=_part.xy;

      VI.flushIndexed(IndBufPanel, 3*3*2*3);
   }
   VI.clear();
}
/******************************************************************************/
void Image::draw3x3(C Color &color, C Color &color_add, C Rect &rect, Flt border_size, Flt tex_frac)C
{
   VI.color  (color    );
   VI.color1 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(16))
   {
      Flt w=rect.w(), W=((w>=0) ? Min(w/2, border_size) : Max(w/2, -border_size)),
          h=rect.h(), H=((h>=0) ? Min(h/2, border_size) : Max(h/2, -border_size)),
          x0=rect.min.x, x3=rect.max.x, x1=x0+W, x2=x3-W,
          y0=rect.max.y, y3=rect.min.y, y1=y0-H, y2=y3+H,
          tex_frac1=1-tex_frac;

      v[ 0].pos.set(x0, y0);
      v[ 1].pos.set(x1, y0);
      v[ 2].pos.set(x2, y0);
      v[ 3].pos.set(x3, y0);
      v[ 4].pos.set(x0, y1);
      v[ 5].pos.set(x1, y1);
      v[ 6].pos.set(x2, y1);
      v[ 7].pos.set(x3, y1);
      v[ 8].pos.set(x0, y2);
      v[ 9].pos.set(x1, y2);
      v[10].pos.set(x2, y2);
      v[11].pos.set(x3, y2);
      v[12].pos.set(x0, y3);
      v[13].pos.set(x1, y3);
      v[14].pos.set(x2, y3);
      v[15].pos.set(x3, y3);

      v[ 0].tex.set(        0,         0);
      v[ 1].tex.set(tex_frac ,         0);
      v[ 2].tex.set(tex_frac1,         0);
      v[ 3].tex.set(        1,         0);
      v[ 4].tex.set(        0, tex_frac );
      v[ 5].tex.set(tex_frac , tex_frac );
      v[ 6].tex.set(tex_frac1, tex_frac );
      v[ 7].tex.set(        1, tex_frac );
      v[ 8].tex.set(        0, tex_frac1);
      v[ 9].tex.set(tex_frac , tex_frac1);
      v[10].tex.set(tex_frac1, tex_frac1);
      v[11].tex.set(        1, tex_frac1);
      v[12].tex.set(        0,         1);
      v[13].tex.set(tex_frac ,         1);
      v[14].tex.set(tex_frac1,         1);
      v[15].tex.set(        1,         1);

      if(partial())REP(16)v[i].tex*=_part.xy;

      VI.flushIndexed(IndBufPanel, 3*3*2*3);
   }
   VI.clear();
}
void Image::draw3x3Vertical(C Color &color, C Color &color_add, C Rect &rect, Flt border_size, Flt tex_frac)C
{
   VI.color  (color    );
   VI.color1 (color_add);
   VI.image  (this     );
   VI.setType(VI_2D_TEX, VI_SP_COL);
   if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(16))
   {
      Flt w=rect.w(), W=((w>=0) ? Min(w/2, border_size) : Max(w/2, -border_size)),
          h=rect.h(), H=((h>=0) ? Min(h/2, border_size) : Max(h/2, -border_size)),
          x0=rect.min.x, x3=rect.max.x, x1=x0+W, x2=x3-W,
          y0=rect.max.y, y3=rect.min.y, y1=y0-H, y2=y3+H,
          tex_frac1=1-tex_frac;

      v[ 0].pos.set(x0, y0);
      v[ 1].pos.set(x1, y0);
      v[ 2].pos.set(x2, y0);
      v[ 3].pos.set(x3, y0);
      v[ 4].pos.set(x0, y1);
      v[ 5].pos.set(x1, y1);
      v[ 6].pos.set(x2, y1);
      v[ 7].pos.set(x3, y1);
      v[ 8].pos.set(x0, y2);
      v[ 9].pos.set(x1, y2);
      v[10].pos.set(x2, y2);
      v[11].pos.set(x3, y2);
      v[12].pos.set(x0, y3);
      v[13].pos.set(x1, y3);
      v[14].pos.set(x2, y3);
      v[15].pos.set(x3, y3);

      v[ 0].tex.set(        1,         0);
      v[ 1].tex.set(        1, tex_frac );
      v[ 2].tex.set(        1, tex_frac1);
      v[ 3].tex.set(        1,         1);
      v[ 4].tex.set(tex_frac1,         0);
      v[ 5].tex.set(tex_frac1, tex_frac );
      v[ 6].tex.set(tex_frac1, tex_frac1);
      v[ 7].tex.set(tex_frac1,         1);
      v[ 8].tex.set(tex_frac ,         0);
      v[ 9].tex.set(tex_frac , tex_frac );
      v[10].tex.set(tex_frac , tex_frac1);
      v[11].tex.set(tex_frac ,         1);
      v[12].tex.set(        0,         0);
      v[13].tex.set(        0, tex_frac );
      v[14].tex.set(        0, tex_frac1);
      v[15].tex.set(        0,         1);

      if(partial())REP(16)v[i].tex*=_part.xy;

      VI.flushIndexed(IndBufPanel, 3*3*2*3);
   }
   VI.clear();
}
/******************************************************************************/
void DisplayDraw::drawShadow(Byte alpha, C Rect &rect, Flt shadow_radius)
{
   if(Gui.image_shadow)
   {
      Flt tex_frac=1.0f/3;
      Sh.Color[0]->set(Vec4(0, 0, 0, AlphaToDisplay(ByteToFlt(alpha))));
      Sh.Color[1]->set(Vec4Zero);
      VI.image  (Gui.image_shadow());
      VI.setType(VI_2D_TEX, VI_SP_COL);
      if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(16))
      {
         Flt x1=rect.min.x, x0=x1-shadow_radius, x2=rect.max.x, x3=x2+shadow_radius,
             y1=rect.max.y, y0=y1+shadow_radius, y2=rect.min.y, y3=y2-shadow_radius,
             tex_frac1=1-tex_frac;

         v[ 0].pos.set(x0, y0);
         v[ 1].pos.set(x1, y0);
         v[ 2].pos.set(x2, y0);
         v[ 3].pos.set(x3, y0);
         v[ 4].pos.set(x0, y1);
         v[ 5].pos.set(x1, y1);
         v[ 6].pos.set(x2, y1);
         v[ 7].pos.set(x3, y1);
         v[ 8].pos.set(x0, y2);
         v[ 9].pos.set(x1, y2);
         v[10].pos.set(x2, y2);
         v[11].pos.set(x3, y2);
         v[12].pos.set(x0, y3);
         v[13].pos.set(x1, y3);
         v[14].pos.set(x2, y3);
         v[15].pos.set(x3, y3);

         v[ 0].tex.set(        0,         0);
         v[ 1].tex.set(tex_frac ,         0);
         v[ 2].tex.set(tex_frac1,         0);
         v[ 3].tex.set(        1,         0);
         v[ 4].tex.set(        0, tex_frac );
         v[ 5].tex.set(tex_frac , tex_frac );
         v[ 6].tex.set(tex_frac1, tex_frac );
         v[ 7].tex.set(        1, tex_frac );
         v[ 8].tex.set(        0, tex_frac1);
         v[ 9].tex.set(tex_frac , tex_frac1);
         v[10].tex.set(tex_frac1, tex_frac1);
         v[11].tex.set(        1, tex_frac1);
         v[12].tex.set(        0,         1);
         v[13].tex.set(tex_frac ,         1);
         v[14].tex.set(tex_frac1,         1);
         v[15].tex.set(        1,         1);

         if(Gui.image_shadow->partial())REP(16)v[i].tex*=Gui.image_shadow->_part.xy;

         VI.flushIndexed(IndBufPanel, 3*3*2*3);
      }
      VI.clear();
   }
}
void DisplayDraw::drawShadowBorders(Byte alpha, C Rect &rect, Flt shadow_radius)
{
   if(Gui.image_shadow)
   {
      Flt tex_frac=1.0f/3;
      Sh.Color[0]->set(Vec4(0, 0, 0, AlphaToDisplay(ByteToFlt(alpha))));
      Sh.Color[1]->set(Vec4Zero);
      VI.image  (Gui.image_shadow());
      VI.setType(VI_2D_TEX, VI_SP_COL);
      if(Vtx2DTex *v=(Vtx2DTex*)VI.addVtx(16))
      {
         Flt x1=rect.min.x, x0=x1-shadow_radius, x2=rect.max.x, x3=x2+shadow_radius,
             y1=rect.max.y, y0=y1+shadow_radius, y2=rect.min.y, y3=y2-shadow_radius,
             tex_frac1=1-tex_frac;

         v[ 0].pos.set(x0, y0);
         v[ 1].pos.set(x1, y0);
         v[ 2].pos.set(x2, y0);
         v[ 3].pos.set(x3, y0);
         v[ 4].pos.set(x0, y1);
         v[ 5].pos.set(x1, y1);
         v[ 6].pos.set(x2, y1);
         v[ 7].pos.set(x3, y1);
         v[ 8].pos.set(x0, y2);
         v[ 9].pos.set(x1, y2);
         v[10].pos.set(x2, y2);
         v[11].pos.set(x3, y2);
         v[12].pos.set(x0, y3);
         v[13].pos.set(x1, y3);
         v[14].pos.set(x2, y3);
         v[15].pos.set(x3, y3);

         v[ 0].tex.set(        0,         0);
         v[ 1].tex.set(tex_frac ,         0);
         v[ 2].tex.set(tex_frac1,         0);
         v[ 3].tex.set(        1,         0);
         v[ 4].tex.set(        0, tex_frac );
         v[ 5].tex.set(tex_frac , tex_frac );
         v[ 6].tex.set(tex_frac1, tex_frac );
         v[ 7].tex.set(        1, tex_frac );
         v[ 8].tex.set(        0, tex_frac1);
         v[ 9].tex.set(tex_frac , tex_frac1);
         v[10].tex.set(tex_frac1, tex_frac1);
         v[11].tex.set(        1, tex_frac1);
         v[12].tex.set(        0,         1);
         v[13].tex.set(tex_frac ,         1);
         v[14].tex.set(tex_frac1,         1);
         v[15].tex.set(        1,         1);

         if(Gui.image_shadow->partial())REP(16)v[i].tex*=Gui.image_shadow->_part.xy;

         VI.flushIndexed(IndBufPanel, (3*3-1)*2*3);
      }
      VI.clear();
   }
}
/******************************************************************************/
void Image::drawCubeFace(C Color &color, C Color &color_add, C Rect &rect, DIR_ENUM face)C
{
   if(InRange(face, 6))
   {
      if(!Sh.DrawCubeFace)Sh.DrawCubeFace=Sh.get("DrawCubeFace");
      VI.shader (Sh.DrawCubeFace);
      VI.color  (color    );
      VI.color1 (color_add);
      VI.setType(VI_2D_FONT, VI_STRIP);
      Sh.Cub[0]->set(this);
      if(Vtx2DFont *v=(Vtx2DFont*)VI.addVtx(4))
      {
         v[0].pos.set(rect.min.x, rect.max.y);
         v[1].pos.set(rect.max.x, rect.max.y);
         v[2].pos.set(rect.min.x, rect.min.y);
         v[3].pos.set(rect.max.x, rect.min.y);
         switch(face)
         { // here 'shade' is used as the 3rd tex Z coordinate
            case DIR_RIGHT  : v[0].tex.set( 1,  1); v[0].shade= 1;
                              v[1].tex.set( 1,  1); v[1].shade=-1;
                              v[2].tex.set( 1, -1); v[2].shade= 1;
                              v[3].tex.set( 1, -1); v[3].shade=-1; break;
            case DIR_LEFT   : v[0].tex.set(-1,  1); v[0].shade=-1;
                              v[1].tex.set(-1,  1); v[1].shade= 1;
                              v[2].tex.set(-1, -1); v[2].shade=-1;
                              v[3].tex.set(-1, -1); v[3].shade= 1; break;
            case DIR_UP     : v[0].tex.set(-1,  1); v[0].shade=-1;
                              v[1].tex.set( 1,  1); v[1].shade=-1;
                              v[2].tex.set(-1,  1); v[2].shade= 1;
                              v[3].tex.set( 1,  1); v[3].shade= 1; break;
            case DIR_DOWN   : v[0].tex.set(-1, -1); v[0].shade= 1;
                              v[1].tex.set( 1, -1); v[1].shade= 1;
                              v[2].tex.set(-1, -1); v[2].shade=-1;
                              v[3].tex.set( 1, -1); v[3].shade=-1; break;
            case DIR_FORWARD: v[0].tex.set(-1,  1); v[0].shade= 1;
                              v[1].tex.set( 1,  1); v[1].shade= 1;
                              v[2].tex.set(-1, -1); v[2].shade= 1;
                              v[3].tex.set( 1, -1); v[3].shade= 1; break;
            case DIR_BACK   : v[0].tex.set( 1,  1); v[0].shade=-1;
                              v[1].tex.set(-1,  1); v[1].shade=-1;
                              v[2].tex.set( 1, -1); v[2].shade=-1;
                              v[3].tex.set(-1, -1); v[3].shade=-1; break;
         }
      }
      VI.end();
   }
}
/******************************************************************************/
void Image::draw3D(C Color &color, Flt size, Flt angle, C Vec &pos)C
{
   D .alpha  (ALPHA_RENDER_BLEND);
   VI.image  (this);
   VI.setType(VI_3D_BILB, VI_STRIP);
   if(Vtx3DBilb *v=(Vtx3DBilb*)VI.addVtx(4))
   {
      v[0].pos      =v[1].pos      =v[2].pos      =v[3].pos      =pos;
   #if GPU_HALF_SUPPORTED
      v[0].vel_angle=v[1].vel_angle=v[2].vel_angle=v[3].vel_angle.set(HalfZero, HalfZero, HalfZero, angle);
   #else
      v[0].vel_angle=v[1].vel_angle=v[2].vel_angle=v[3].vel_angle.set(       0,        0,        0, angle);
   #endif
      v[0].color    =v[1].color    =v[2].color    =v[3].color    =color;
      v[0].size     =v[1].size     =v[2].size     =v[3].size     =size;
   #if GL // needed for iOS PVRTC Pow2 #ParticleImgPart
      Sh.ImgSize->setConditional(_part.xy);
   #endif
      SET_TEX(v[0].tex, TEX_ZERO, TEX_ZERO);
      SET_TEX(v[1].tex, TEX_ONE , TEX_ZERO);
      SET_TEX(v[2].tex, TEX_ZERO, TEX_ONE );
      SET_TEX(v[3].tex, TEX_ONE , TEX_ONE );
   }
   VI.end();
}
/******************************************************************************/
#pragma pack(push, 4)
struct GpuVolume
{
   Flt min_steps,
       max_steps,
       density_factor,
       precision     ;
   Vec size  ,
       pixels,
       inside;
};
#pragma pack(pop)
void Image::drawVolume(C Color &color, C Color &color_add, C OBox &obox, Flt voxel_density_factor, Flt precision, Int min_steps, Int max_steps)C
{
   if(Frustum(obox) && Renderer.canReadDepth())
   {
      if(!Sh.Volume)
      {
         ShaderFile &sf=*ShaderFiles("Effects 3D");
         REPD(inside, 3)
         REPD(la    , 2)Sh.DrawVolume[inside][la]=sf.get(S8+"Volume"+inside+la);
         Sh.Volume=GetShaderParam("Volume");
      }
      GpuVolume v;
      Bool LA=(type()==IMAGE_R8G8); // check 'type' instead of 'hwType' because volumetric clouds may want to be set as RG, but got RGBA, however only RG was set

      v.min_steps     =Mid(Flt(min_steps),        2.0f, 1024.0f);
      v.max_steps     =Mid(Flt(max_steps), v.min_steps, 1024.0f);
      v.density_factor=Mid(voxel_density_factor, EPS_GPU, 1-EPS_GPU);
      v.precision     =precision;
      v.size          =obox.box.size()*0.5f;
      v.pixels        =size3();

      Matrix temp;
      temp.pos=obox.center();
      temp.x  =obox.matrix.x*v.size.x;
      temp.y  =obox.matrix.y*v.size.y;
      temp.z  =obox.matrix.z*v.size.z;
      SetOneMatrix(temp);

      Vec delta=CamMatrix.pos-temp.pos;
      v.inside.x=Dot(delta, obox.matrix.x);
      v.inside.y=Dot(delta, obox.matrix.y);
      v.inside.z=Dot(delta, obox.matrix.z);

      D .alphaFactor(TRANSPARENT); MaterialClear(); // 'MaterialClear' must be called when changing 'D.alphaFactor'
      ShaderImage &si=(LA ? *Sh.VolXY[0] : *Sh.Vol);
      si          .set(T        );
      Sh.Color[0]->set(color    );
      Sh.Color[1]->set(color_add);
      Sh.Volume  ->set(v);

      D.cull (true);
      D.alpha(ALPHA_RENDER_BLEND);
      Renderer.needDepthRead();

      Flt e=Frustum.view_quad_max_dist;
      if(v.inside.x>=-v.size.x-e && v.inside.x<=v.size.x+e
      && v.inside.y>=-v.size.y-e && v.inside.y<=v.size.y+e
      && v.inside.z>=-v.size.z-e && v.inside.z<=v.size.z+e)
      {
         D.depth(false);

         if(v.inside.x>=-v.size.x+e && v.inside.x<=v.size.x-e
         && v.inside.y>=-v.size.y+e && v.inside.y<=v.size.y-e
         && v.inside.z>=-v.size.z+e && v.inside.z<=v.size.z-e)Sh.DrawVolume[2][LA]->begin();
         else                                                 Sh.DrawVolume[1][LA]->begin();
         MshrBoxR.set().draw();
      }else
      {
         D.depthOnWrite(true, false); Sh.DrawVolume[0][LA]->begin(); MshrBox.set().draw();
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
