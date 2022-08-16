/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   windows aero colors
   on black background - R  21, G  46, B  71
   on white background - R 199, G 224, B 249
   panel_window.back_color.set(178, 178, 178);
   panel_window.blur_color.set( 21,  46,  71);

/******************************************************************************/
#define CC4_GSTL CC4('G','S','T','L')
#define DEFAULT_SIZE 0.1f

// TODO: these could be set as Panel members
#define side_fade_ext -0.03f // when side image fading starts relative to rect border, example: fade_right_start=panel_rect.max.x+side_fade_ext (<0 means that fading starts already inside the rectangle)
#define side_max_ext   0.01f // when side image fading ends   relative to rect border, example: fade_right_end  =panel_rect.max.x+side_max_ext
/******************************************************************************/
DEFINE_CACHE(Panel, Panels, PanelPtr, "Panel");
/******************************************************************************/
void Panel::getRectTop(C Rect &rect, Rect &top, Bool mirror)C
{
   Flt w;
   if(side_mode)w=rect.w();else
   {
      w=top_size*top_image->aspect();
      if(side_min_scale<1)
      {
         Flt aw=Abs(w), rect_w=rect.w(); if(aw>rect_w)
         {
            Flt scale=Max(rect_w/aw, side_min_scale);
            top.setD(rect.centerX()/*+top_offset.x*/, rect.max.y+top_offset*scale, w*scale, top_size*scale);
            return;
         }
      }
   }
   top.setD(rect.centerX()/*+top_offset.x*/, rect.max.y+top_offset, w, top_size);
}
void Panel::getRectBottom(C Rect &rect, Rect &bottom, Bool mirror)C
{
   Flt w;
   if(side_mode)w=rect.w();else
   {
      w=bottom_size*bottom_image->aspect();
      if(side_min_scale<1)
      {
         Flt aw=Abs(w), rect_w=rect.w(); if(aw>rect_w)
         {
            Flt scale=Max(rect_w/aw, side_min_scale);
            bottom.setU(rect.centerX()/*+bottom_offset.x*/, rect.min.y+bottom_offset*scale, w*scale, bottom_size*scale);
            goto end;
         }
      }
   }
   bottom.setU(rect.centerX()/*+bottom_offset.x*/, rect.min.y+bottom_offset, w, bottom_size);
end:
   if(mirror && bottom_image==top_image)bottom.swapY(); // mirror vertically if it's the same as top
}
void Panel::getRectLeftRight(C Rect &rect, Rect &left, Rect &right, Bool mirror)C
{
   Flt w =left_right_size,
       l =rect.min.x-left_right_offset.x,
       r =rect.max.x+left_right_offset.x,
       yb,
       yt;
   if(side_mode)
   {
      yb=rect.min.y;
      yt=rect.max.y;
   }else
   {
      Flt h=left_right_size*left_right_image->invAspect()*0.5f,
          y=rect.centerY()+left_right_offset.y;
      yb=y-h,
      yt=y+h;
   }
             right.set(r, yb, r+w, yt);
   if(mirror)left .set(l, yb, l-w, yt);
   else      left .set(l-w, yb, l, yt);
}
void Panel::getRectTopCorner(C Rect &rect, Rect &left, Rect &right, Bool mirror)C
{
   Flt w =top_corner_size*top_corner_image->aspect(),
       h =top_corner_size,
       l =rect.min.x-top_corner_offset.x,
       r =rect.max.x+top_corner_offset.x,
       yb=rect.max.y+top_corner_offset.y,
       yt=yb+h;

             right.set(r, yb, r+w, yt);
   if(mirror)left .set(l, yb, l-w, yt);
   else      left .set(l-w, yb, l, yt);
}
void Panel::getRectBottomCorner(C Rect &rect, Rect &left, Rect &right, Bool mirror)C
{
   Flt w =bottom_corner_size*bottom_corner_image->aspect(),
       h =bottom_corner_size,
       l =rect.min.x-bottom_corner_offset.x,
       r =rect.max.x+bottom_corner_offset.x,
       yt=rect.min.y+bottom_corner_offset.y,
       yb=yt-h;

             right.set(r, yb, r+w, yt);
   if(mirror)left .set(l, yb, l-w, yt);
   else      left .set(l-w, yb, l, yt);
   if(mirror && bottom_corner_image==top_corner_image){left.swapY(); right.swapY();} // mirror vertically if it's the same as top
}
/******************************************************************************/
static inline void ClipX(C Panel &panel, Rect &elm_rect, C Rect &panel_rect)
{
   if(elm_rect.max.x>panel_rect.max.x) // 'elm_rect' is always at the center X of 'panel_rect', so we can check if only one side (right) is outside 'panel_rect', because other side (left) will be the same
   {
      MAX(elm_rect.min.x, panel_rect.min.x-side_max_ext);
      MIN(elm_rect.max.x, panel_rect.max.x+side_max_ext);
   }
}
void Panel::extendedRect(C Rect &rect, Rect &extended)C
{
   Rect r; if(panel_image)panel_image->extendedRect(rect, r);else r=rect;
   if(shadow_opacity)r|=(rect+Vec2(shadow_offset, -shadow_offset)).extend(shadow_radius);
   if(border_color.a && border_size>0)r.extend(border_size);
   if(  side_color.a)
   {
      if(          top_image){Rect top        ; getRectTop         (rect, top        , false); ClipX(T, top   , rect); r|=top   ;}
      if(       bottom_image){Rect bottom     ; getRectBottom      (rect, bottom     , false); ClipX(T, bottom, rect); r|=bottom;}
      if(   left_right_image){Rect left, right; getRectLeftRight   (rect, left, right, false); r|=left; r|=right;}
      if(   top_corner_image){Rect left, right; getRectTopCorner   (rect, left, right, false); r|=left; r|=right;}
      if(bottom_corner_image){Rect left, right; getRectBottomCorner(rect, left, right, false); r|=left; r|=right;}
   }
   extended=r; // modify at the end in case 'extended' is 'rect'
}
void Panel::defaultInnerPadding(Rect &padding)C
{
   if(panel_image)padding=panel_image->defaultInnerPadding();else padding.zero();
   MAX(padding.max.y, bar_size);
   // border is not included
}
void Panel::innerPadding(C Rect &rect, Rect &padding)C
{
   if(panel_image)panel_image->innerPadding(rect, padding);else padding.zero();
   MAX(padding.max.y, bar_size);
   // border is not included
}
Vec2 Panel::defaultInnerPaddingSize()C {Rect padding; defaultInnerPadding(padding); return padding.min+padding.max;}
/******************************************************************************/
void Panel::reset()
{
   center_stretch=false;
   center_color=WHITE;
      bar_color=TRANSPARENT;
   border_color=WHITE;
     side_color=WHITE;
     blur_color=TRANSPARENT;
   center_shadow =false;
   shadow_opacity=170;
   shadow_radius =0.035f;
   shadow_offset =0;
   center_scale  =1;
   bar_size=0;
   border_size=0;
   side_min_scale=1;
   top_size=bottom_size=left_right_size=top_corner_size=bottom_corner_size=DEFAULT_SIZE;
   top_offset=bottom_offset=0;
   left_right_offset=top_corner_offset=bottom_corner_offset.zero();
   center_image=bar_image=border_image=top_image=bottom_image=left_right_image=top_corner_image=bottom_corner_image.clear();
   panel_image.clear();
}
/******************************************************************************/
Bool Panel::pixelBorder()C
{
   return border_color.a && !border_image && !border_size;
}
Bool Panel::getSideScale(C Rect &rect, Flt &scale)C
{
   if(panel_image)return panel_image->getSideScale(rect, scale);
   return false;
}
void Panel::scaleBorder(Flt scale)
{
          border_size*=scale;
             top_size*=scale;           top_offset*=scale;
          bottom_size*=scale;        bottom_offset*=scale;
      left_right_size*=scale;    left_right_offset*=scale;
      top_corner_size*=scale;    top_corner_offset*=scale;
   bottom_corner_size*=scale; bottom_corner_offset*=scale;
}
/******************************************************************************/
void Panel::drawShadow(Byte shadow, C Rect &rect)C
{
   if(center_shadow)D.drawShadow       (shadow, rect+Vec2(shadow_offset, -shadow_offset), shadow_radius);
   else             D.drawShadowBorders(shadow, rect                                    , shadow_radius);
}
void Panel::drawCenter(C Color &color, C Rect &rect)C
{
   if(  panel_image  )panel_image ->draw    (color, TRANSPARENT, rect);else
   if(!center_image  )rect        . draw    (color                   );else
   if(!center_stretch)center_image->drawTile(color, TRANSPARENT, rect, center_scale);else
                      center_image->draw    (color, TRANSPARENT, rect);
}
void Panel::drawBar(C Color &color, C Rect &rect)C
{
   if(bar_size>0)
   {
      Rect r(rect.min.x, rect.max.y-bar_size, rect.max.x, rect.max.y);
      if(bar_image)bar_image->draw(color, TRANSPARENT, r);
      else         r        . draw(color, true);
   }
}
void Panel::drawBorder(C Color &color, C Rect &rect)C
{
   Rect r;
   if(bar_size>0 && border_size<=0)r.set(rect.min.x, rect.min.y, rect.max.x, rect.max.y-bar_size); // if have bar and border is inner, move it under the bar
   else                            r=rect;
   if(border_image)border_image->drawBorder(color, TRANSPARENT, r, border_size);else
   if(border_size )r           . drawBorder(color, border_size);else
                   r           . draw      (color, false);
}
static void DrawTopBottom(C Panel &panel, C ImagePtr &image, C Color &color, C Rect &elm_rect, C Rect &panel_rect)
{
   switch(panel.side_mode)
   {
      default:
      {
       // 'elm_rect' is always at the center X of 'panel_rect', so we can check if only one side (right) is outside 'panel_rect', because other side (left) will be the same
       //if(elm_rect.max.x>panel_rect.max.x) can't do this if 'side_fade_ext' can be <0
         {
            Flt opaque_r=panel_rect.max.x+side_fade_ext; if(elm_rect.max.x>opaque_r
                                                         || elm_rect.min.x>opaque_r) // this is needed when using negative sizes which cause 'elm_rect' to be flipped
            {
               Flt trans_r=panel_rect.max.x+side_max_ext,
                   trans_l=panel_rect.min.x-side_max_ext,
                  opaque_l=panel_rect.min.x-side_fade_ext;
               image->drawFadeLR(color, elm_rect, trans_l, opaque_l, opaque_r, trans_r);
               return;
            }
         }
         image->draw(color, TRANSPARENT, elm_rect);
      }break;

      case Panel::SM_STRETCH: VI.clampAniso(); image->draw    (color, TRANSPARENT, elm_rect); break;
      case Panel::SM_WRAP   : VI.wrapX     (); image->drawPart(color, TRANSPARENT, elm_rect, Rect(0, 0, image->invAspect()*elm_rect.aspect(), 1)); break;
   }
}
static void DrawLeftRight(C Panel &panel, C ImagePtr &image, C Color &color, C Rect &elm_rect/*, C Rect &panel_rect*/)
{
   switch(panel.side_mode)
   {
      default               :                  image->draw    (color, TRANSPARENT, elm_rect); break;
      case Panel::SM_STRETCH: VI.clampAniso(); image->draw    (color, TRANSPARENT, elm_rect); break;
      case Panel::SM_WRAP   : VI.wrapY     (); image->drawPart(color, TRANSPARENT, elm_rect, Rect(0, 0, 1, image->aspect()*elm_rect.invAspect())); break;
   }
}
void Panel::drawSide(C Color &color, C Rect &rect)C
{
   if(          top_image){Rect top        ; getRectTop         (rect, top        ); DrawTopBottom(T,        top_image, color, top   , rect);}
   if(       bottom_image){Rect bottom     ; getRectBottom      (rect, bottom     ); DrawTopBottom(T,     bottom_image, color, bottom, rect);}
   if(   left_right_image){Rect left, right; getRectLeftRight   (rect, left, right); DrawLeftRight(T, left_right_image, color, left        );
                                                                                     DrawLeftRight(T, left_right_image, color, right       );}
   if(   top_corner_image){Rect left, right; getRectTopCorner   (rect, left, right);    top_corner_image->draw(color, TRANSPARENT, left  );
                                                                                        top_corner_image->draw(color, TRANSPARENT, right );}
   if(bottom_corner_image){Rect left, right; getRectBottomCorner(rect, left, right); bottom_corner_image->draw(color, TRANSPARENT, left  );
                                                                                     bottom_corner_image->draw(color, TRANSPARENT, right );}
}
void Panel::draw(C Rect &rect)C
{
#if !MOBILE // too slow
   if( blur_color.any() // fast check
 //&& (blur_color.r || blur_color.g || blur_color.b) // precise check, because 'blur_color.a' is ignored !! can ignore this check because Editor always clears 'blur_color.a' !!
   )
   {
      const Bool    hi   =true;
      const Int     shift=(hi ? 1 : 2);
      ImageRTDesc   rt_desc(Renderer._ui->w()>>shift, Renderer._ui->h()>>shift, IMAGERT_SRGB);
      ImageRTPtrRef rt0(hi ? Renderer._h0 : Renderer._q0); rt0.get(rt_desc);
      ImageRTPtrRef rt1(hi ? Renderer._h1 : Renderer._q1); rt1.get(rt_desc);
      ImageRT      *cur      =Renderer._cur[0], *ds=Renderer._cur_ds;
      Rect          r        =rect; r.extend(D.pixelToScreenSize(SHADER_BLUR_RANGE<<shift));
      Bool          secondary=(Renderer._ui!=cur); // required when "window.fade && blur" is used

                   Renderer._ui->copyHw(*rt0, false, r   ); // use 'Renderer.gui' instead of 'Renderer.cur[0]' in case we're drawing transparent Window and we're inside 'D.fxBegin' but need to access default gui RT
      if(secondary)Renderer._ui->copyHw(*cur, false, rect); // set background to be a copy

    //Sh.imgSize(*rt0); we can just use 'RTSize' instead of 'ImgSize' since there's no scale
      ALPHA_MODE alpha=D.alpha(ALPHA_NONE); Renderer.set(rt1, null, false); Sh.BlurX[true]->draw(rt0, &r);
                                            Renderer.set(rt0, null, false); Sh.BlurY[true]->draw(rt1, &r);
                                            Renderer.set(cur, ds  , true );
      if(shadow_opacity)
      {
         D.alpha(ALPHA_BLEND);
         if(shadow_opacity)
         {
            if(center_shadow)D.drawShadow       (shadow_opacity, rect+Vec2(shadow_offset, -shadow_offset), shadow_radius);
            else             D.drawShadowBorders(shadow_opacity, rect                                    , shadow_radius);
         }
      }
      if(secondary) // for secondary we need to force rt.alpha to 1.0
      {
         D.alphaFactor(Color(center_color.a, center_color.a, center_color.a, 255)); MaterialClear(); // 'MaterialClear' must be called when changing 'D.alphaFactor'
         D.alpha(ALPHA_FACTOR); Sh.Color[0]->set(Color(blur_color.r, blur_color.g, blur_color.b, 0)); Sh.Color[1]->set(Color(center_color.r, center_color.g, center_color.b, 255)); Sh.DrawC->draw(rt0, &rect);
      }else
      {
         D.alpha(ALPHA_BLEND ); Sh.Color[0]->set(Color(blur_color.r, blur_color.g, blur_color.b, 0)); Sh.Color[1]->set(center_color); Sh.DrawC->draw(rt0, &rect);
      }
         D.alpha(alpha       );
   }else
#endif
   {
      if(shadow_opacity)drawShadow(shadow_opacity, rect);
      if(center_color.a)drawCenter(center_color  , rect);
   }

   if(   bar_color.a)drawBar   (   bar_color, rect);
   if(border_color.a)drawBorder(border_color, rect);
   if(  side_color.a)drawSide  (  side_color, rect);
}
void Panel::draw(C Color &color, C Rect &rect)C
{
   Byte  shadow_opacity=        (T.shadow_opacity* color.a+128)/255;
   Color center_color  =ColorMul(T.center_color  , color),
            bar_color  =ColorMul(T.   bar_color  , color),
         border_color  =ColorMul(T.border_color  , color),
           side_color  =ColorMul(T.  side_color  , color);

#if !MOBILE // too slow
   Color blur_color=ColorMulZeroAlpha(T.blur_color, color); // !! because of 'ColorMulZeroAlpha' we can ignore precise check below !!
   if(   blur_color.any() // fast check
 //&&   (blur_color.r || blur_color.g || blur_color.b) // precise check, because 'blur_color.a' is ignored !! can ignore this check because of 'ColorMulZeroAlpha' above !!
   )
   {
      const Bool    hi   =true;
      const Int     shift=(hi ? 1 : 2);
      ImageRTDesc   rt_desc(Renderer._ui->w()>>shift, Renderer._ui->h()>>shift, IMAGERT_SRGB);
      ImageRTPtrRef rt0(hi ? Renderer._h0 : Renderer._q0); rt0.get(rt_desc);
      ImageRTPtrRef rt1(hi ? Renderer._h1 : Renderer._q1); rt1.get(rt_desc);
      ImageRT      *cur      =Renderer._cur[0], *ds=Renderer._cur_ds;
      Rect          r        =rect; r.extend(D.pixelToScreenSize(SHADER_BLUR_RANGE<<shift));
      Bool          secondary=(Renderer._ui!=cur); // required when "window.fade && blur" is used

                   Renderer._ui->copyHw(*rt0, false, r   ); // use 'Renderer.gui' instead of 'Renderer.cur[0]' in case we're drawing transparent Window and we're inside 'D.fxBegin' but need to access default gui RT
      if(secondary)Renderer._ui->copyHw(*cur, false, rect); // set background to be a copy

    //Sh.imgSize(*rt0); we can just use 'RTSize' instead of 'ImgSize' since there's no scale
      ALPHA_MODE alpha=D.alpha(ALPHA_NONE); Renderer.set(rt1, null, false); Sh.BlurX[true]->draw(rt0, &r);
                                            Renderer.set(rt0, null, false); Sh.BlurY[true]->draw(rt1, &r);
                                            Renderer.set(cur, ds  , true );
      if(shadow_opacity)
      {
         D.alpha(ALPHA_BLEND);
         if(shadow_opacity)
         {
            if(center_shadow)D.drawShadow       (shadow_opacity, rect+Vec2(shadow_offset, -shadow_offset), shadow_radius);
            else             D.drawShadowBorders(shadow_opacity, rect                                    , shadow_radius);
         }
      }
      if(secondary) // for secondary we need to force rt.alpha to 1.0
      {
         D.alphaFactor(Color(center_color.a, center_color.a, center_color.a, 255)); MaterialClear(); // 'MaterialClear' must be called when changing 'D.alphaFactor'
         D.alpha(ALPHA_FACTOR); Sh.Color[0]->set(Color(blur_color.r, blur_color.g, blur_color.b, 0)); Sh.Color[1]->set(Color(center_color.r, center_color.g, center_color.b, 255)); Sh.DrawC->draw(rt0, &rect);
      }else
      {
         D.alpha(ALPHA_BLEND ); Sh.Color[0]->set(Color(blur_color.r, blur_color.g, blur_color.b, 0)); Sh.Color[1]->set(center_color); Sh.DrawC->draw(rt0, &rect);
      }
         D.alpha(alpha       );
   }else
#endif
   {
      if(shadow_opacity)drawShadow(shadow_opacity, rect);
      if(center_color.a)drawCenter(center_color  , rect);
   }

   if(   bar_color.a)drawBar   (   bar_color, rect);
   if(border_color.a)drawBorder(border_color, rect);
   if(  side_color.a)drawSide  (  side_color, rect);
}
/******************************************************************************/
void Panel::drawLines(C Color &line_color, C Rect &rect)C
{
   rect.draw(line_color, false);
   if(          top_image){Rect top        ; getRectTop         (rect, top        , false);    top.draw(line_color, false);}
   if(       bottom_image){Rect bottom     ; getRectBottom      (rect, bottom     , false); bottom.draw(line_color, false);}
   if(   left_right_image){Rect left, right; getRectLeftRight   (rect, left, right, false);   left.draw(line_color, false); right.draw(line_color, false);}
   if(   top_corner_image){Rect left, right; getRectTopCorner   (rect, left, right, false);   left.draw(line_color, false); right.draw(line_color, false);}
   if(bottom_corner_image){Rect left, right; getRectBottomCorner(rect, left, right, false);   left.draw(line_color, false); right.draw(line_color, false);}
}
/******************************************************************************/
#pragma pack(push, 1)
struct PanelDesc
{
   Bool  center_stretch, center_shadow;
   Byte  shadow_opacity;
   Panel::SIDE_MODE side_mode;
   Color center_color, bar_color, border_color, side_color, blur_color;
   Flt   shadow_radius, shadow_offset,
         center_scale, bar_size, border_size, side_min_scale, top_size, bottom_size, left_right_size, top_corner_size, bottom_corner_size,
         top_offset, bottom_offset;
   Vec2  left_right_offset, top_corner_offset, bottom_corner_offset;
};
struct PanelDesc7
{
   Bool  center_stretch, center_shadow;
   Byte  shadow_opacity;
   Color center_color, bar_color, border_color, side_color, blur_color;
   Flt   shadow_radius, shadow_offset,
         center_scale, bar_size, border_size, side_min_scale, top_size, bottom_size, left_right_size, top_corner_size, bottom_corner_size,
         top_offset, bottom_offset;
   Vec2  left_right_offset, top_corner_offset, bottom_corner_offset;
};
struct PanelDesc5
{
   Bool  center_stretch, side_stretch, center_shadow;
   Byte  shadow_opacity;
   Color center_color, border_color, side_color, blur_color;
   Flt   shadow_radius, shadow_offset,
         border_size, center_scale, top_size, bottom_size, left_right_size, top_corner_size, bottom_corner_size,
         top_offset, bottom_offset;
   Vec2  left_right_offset, top_corner_offset, bottom_corner_offset;
};
struct PanelDesc2
{
   Byte  center_stretch, shadow_opacity;
   Color center_color, border_color, blur_color;
   Flt   shadow_offset, shadow_radius, border_size, center_scale, corner_size, top_size;
   Vec2  corner_offset, top_offset;
};
#pragma pack(pop)

Bool Panel::save(File &f, CChar *path)C
{
   f.putUInt (CC4_GSTL);
   f.cmpUIntV(9       ); // version

   PanelDesc desc;

   Unaligned(desc.       center_stretch  ,        center_stretch  );
   Unaligned(desc.       center_shadow   ,        center_shadow   );
   Unaligned(desc.       shadow_opacity  ,        shadow_opacity  );
   Unaligned(desc.         side_mode     ,          side_mode     );
   Unaligned(desc.       center_color    ,        center_color    );
   Unaligned(desc.          bar_color    ,           bar_color    );
   Unaligned(desc.       border_color    ,        border_color    );
   Unaligned(desc.         side_color    ,          side_color    );
   Unaligned(desc.         blur_color    ,          blur_color    );
   Unaligned(desc.       shadow_radius   ,        shadow_radius   );
   Unaligned(desc.       shadow_offset   ,        shadow_offset   );
   Unaligned(desc.       center_scale    ,        center_scale    );
   Unaligned(desc.          bar_size     ,           bar_size     );
   Unaligned(desc.       border_size     ,        border_size     );
   Unaligned(desc.         side_min_scale,          side_min_scale);
   Unaligned(desc.          top_size     ,           top_size     );
   Unaligned(desc.       bottom_size     ,        bottom_size     );
   Unaligned(desc.   left_right_size     ,    left_right_size     );
   Unaligned(desc.   top_corner_size     ,    top_corner_size     );
   Unaligned(desc.bottom_corner_size     , bottom_corner_size     );
   Unaligned(desc.          top_offset   ,           top_offset   );
   Unaligned(desc.       bottom_offset   ,        bottom_offset   );
   Unaligned(desc.   left_right_offset   ,    left_right_offset   );
   Unaligned(desc.   top_corner_offset   ,    top_corner_offset   );
   Unaligned(desc.bottom_corner_offset   , bottom_corner_offset   );
   f<<desc;
   f.putAsset(       center_image.id());
   f.putAsset(          bar_image.id());
   f.putAsset(       border_image.id());
   f.putAsset(          top_image.id());
   f.putAsset(       bottom_image.id());
   f.putAsset(   left_right_image.id());
   f.putAsset(   top_corner_image.id());
   f.putAsset(bottom_corner_image.id());
   f.putAsset(        panel_image.id());
   return f.ok();
}
Bool Panel::load(File &f, CChar *path)
{
   if(f.getUInt()==CC4_GSTL)switch(f.decUIntV()) // version
   {
      case 9:
      {
         PanelDesc desc; if(f.getFast(desc))
         {
            Unaligned(       center_stretch  , desc.       center_stretch  );
            Unaligned(       center_shadow   , desc.       center_shadow   );
            Unaligned(       shadow_opacity  , desc.       shadow_opacity  );
            Unaligned(         side_mode     , desc.         side_mode     );
            Unaligned(       center_color    , desc.       center_color    );
            Unaligned(          bar_color    , desc.          bar_color    );
            Unaligned(       border_color    , desc.       border_color    );
            Unaligned(         side_color    , desc.         side_color    );
            Unaligned(         blur_color    , desc.         blur_color    );
            Unaligned(       shadow_radius   , desc.       shadow_radius   );
            Unaligned(       shadow_offset   , desc.       shadow_offset   );
            Unaligned(       center_scale    , desc.       center_scale    );
            Unaligned(          bar_size     , desc.          bar_size     );
            Unaligned(       border_size     , desc.       border_size     );
            Unaligned(         side_min_scale, desc.         side_min_scale);
            Unaligned(          top_size     , desc.          top_size     );
            Unaligned(       bottom_size     , desc.       bottom_size     );
            Unaligned(   left_right_size     , desc.   left_right_size     );
            Unaligned(   top_corner_size     , desc.   top_corner_size     );
            Unaligned(bottom_corner_size     , desc.bottom_corner_size     );
            Unaligned(          top_offset   , desc.          top_offset   );
            Unaligned(       bottom_offset   , desc.       bottom_offset   );
            Unaligned(   left_right_offset   , desc.   left_right_offset   );
            Unaligned(   top_corner_offset   , desc.   top_corner_offset   );
            Unaligned(bottom_corner_offset   , desc.bottom_corner_offset   );

            center_image.require(f.getAssetID(), path);
               bar_image.require(f.getAssetID(), path);
            border_image.require(f.getAssetID(), path);
               top_image.require(f.getAssetID(), path);
            bottom_image.require(f.getAssetID(), path);
        left_right_image.require(f.getAssetID(), path);
        top_corner_image.require(f.getAssetID(), path);
     bottom_corner_image.require(f.getAssetID(), path);
             panel_image.require(f.getAssetID(), path);

            if(f.ok())return true;
         }
      }break;

      case 8:
      {
         PanelDesc desc; if(f.getFast(desc))
         {
            Unaligned(       center_stretch  , desc.       center_stretch  );
            Unaligned(       center_shadow   , desc.       center_shadow   );
            Unaligned(       shadow_opacity  , desc.       shadow_opacity  );
            Unaligned(         side_mode     , desc.         side_mode     );
            Unaligned(       center_color    , desc.       center_color    );
            Unaligned(          bar_color    , desc.          bar_color    );
            Unaligned(       border_color    , desc.       border_color    );
            Unaligned(         side_color    , desc.         side_color    );
            Unaligned(         blur_color    , desc.         blur_color    );
            Unaligned(       shadow_radius   , desc.       shadow_radius   );
            Unaligned(       shadow_offset   , desc.       shadow_offset   );
            Unaligned(       center_scale    , desc.       center_scale    );
            Unaligned(          bar_size     , desc.          bar_size     );
            Unaligned(       border_size     , desc.       border_size     );
            Unaligned(         side_min_scale, desc.         side_min_scale);
            Unaligned(          top_size     , desc.          top_size     );
            Unaligned(       bottom_size     , desc.       bottom_size     );
            Unaligned(   left_right_size     , desc.   left_right_size     );
            Unaligned(   top_corner_size     , desc.   top_corner_size     );
            Unaligned(bottom_corner_size     , desc.bottom_corner_size     );
            Unaligned(          top_offset   , desc.          top_offset   );
            Unaligned(       bottom_offset   , desc.       bottom_offset   );
            Unaligned(   left_right_offset   , desc.   left_right_offset   );
            Unaligned(   top_corner_offset   , desc.   top_corner_offset   );
            Unaligned(bottom_corner_offset   , desc.bottom_corner_offset   );

            center_image.require(f.getAssetID(), path);
               bar_image.require(f.getAssetID(), path);
            border_image.require(f.getAssetID(), path);
               top_image.require(f.getAssetID(), path);
            bottom_image.require(f.getAssetID(), path);
        left_right_image.require(f.getAssetID(), path);
        top_corner_image.require(f.getAssetID(), path);
     bottom_corner_image.require(f.getAssetID(), path);
             panel_image.require(f.getAssetID(), path);

            if(blur_color.a){Swap(blur_color, center_color); blur_color.a=0;}
            if(f.ok())return true;
         }
      }break;

      case 7:
      {
         PanelDesc7 desc; if(f.getFast(desc))
         {
            Unaligned(       center_stretch  , desc.       center_stretch  );
            Unaligned(       center_shadow   , desc.       center_shadow   );
            Unaligned(       shadow_opacity  , desc.       shadow_opacity  );
            Unaligned(       center_color    , desc.       center_color    );
            Unaligned(          bar_color    , desc.          bar_color    );
            Unaligned(       border_color    , desc.       border_color    );
            Unaligned(         side_color    , desc.         side_color    );
            Unaligned(         blur_color    , desc.         blur_color    );
            Unaligned(       shadow_radius   , desc.       shadow_radius   );
            Unaligned(       shadow_offset   , desc.       shadow_offset   );
            Unaligned(       center_scale    , desc.       center_scale    );
            Unaligned(          bar_size     , desc.          bar_size     );
            Unaligned(       border_size     , desc.       border_size     );
            Unaligned(         side_min_scale, desc.         side_min_scale);
            Unaligned(          top_size     , desc.          top_size     );
            Unaligned(       bottom_size     , desc.       bottom_size     );
            Unaligned(   left_right_size     , desc.   left_right_size     );
            Unaligned(   top_corner_size     , desc.   top_corner_size     );
            Unaligned(bottom_corner_size     , desc.bottom_corner_size     );
            Unaligned(          top_offset   , desc.          top_offset   );
            Unaligned(       bottom_offset   , desc.       bottom_offset   );
            Unaligned(   left_right_offset   , desc.   left_right_offset   );
            Unaligned(   top_corner_offset   , desc.   top_corner_offset   );
            Unaligned(bottom_corner_offset   , desc.bottom_corner_offset   );

            center_image.require(f.getAssetID(), path);
               bar_image.require(f.getAssetID(), path);
            border_image.require(f.getAssetID(), path);
               top_image.require(f.getAssetID(), path);
            bottom_image.require(f.getAssetID(), path);
        left_right_image.require(f.getAssetID(), path);
        top_corner_image.require(f.getAssetID(), path);
     bottom_corner_image.require(f.getAssetID(), path);
             panel_image.require(f.getAssetID(), path);
              side_mode=SM_DEFAULT;

            if(blur_color.a){Swap(blur_color, center_color); blur_color.a=0;}
            if(f.ok())return true;
         }
      }break;

      case 6:
      {
         PanelDesc5 desc; if(f.getFast(desc))
         {
            Unaligned(       center_stretch, desc.       center_stretch);
            Unaligned(       center_shadow , desc.       center_shadow );
            Unaligned(       shadow_opacity, desc.       shadow_opacity);
            Unaligned(       center_color  , desc.       center_color  );
            Unaligned(       border_color  , desc.       border_color  );
            Unaligned(         side_color  , desc.         side_color  );
            Unaligned(         blur_color  , desc.         blur_color  );
            Unaligned(       shadow_radius , desc.       shadow_radius );
            Unaligned(       shadow_offset , desc.       shadow_offset );
            Unaligned(       border_size   , desc.       border_size   ); CHS(border_size);
            Unaligned(       center_scale  , desc.       center_scale  );
            Unaligned(          top_size   , desc.          top_size   );
            Unaligned(       bottom_size   , desc.       bottom_size   );
            Unaligned(   left_right_size   , desc.   left_right_size   );
            Unaligned(   top_corner_size   , desc.   top_corner_size   );
            Unaligned(bottom_corner_size   , desc.bottom_corner_size   );
            Unaligned(          top_offset , desc.          top_offset ); top_offset-=top_size/2;
            Unaligned(       bottom_offset , desc.       bottom_offset ); bottom_offset+=bottom_size/2;
            Unaligned(   left_right_offset , desc.   left_right_offset ); left_right_offset.x-=left_right_size/2;
            Unaligned(   top_corner_offset , desc.   top_corner_offset ); top_corner_offset-=top_corner_size/2;
            Unaligned(bottom_corner_offset , desc.bottom_corner_offset ); bottom_corner_offset.x-=bottom_corner_size/2; bottom_corner_offset.y+=bottom_corner_size/2;

            center_image.require(f.getAssetID(), path);
            border_image.require(f.getAssetID(), path);
               top_image.require(f.getAssetID(), path);
            bottom_image.require(f.getAssetID(), path);
        left_right_image.require(f.getAssetID(), path);
        top_corner_image.require(f.getAssetID(), path);
     bottom_corner_image.require(f.getAssetID(), path);
             panel_image.require(f.getAssetID(), path);
              side_mode=SM_DEFAULT; bar_color=TRANSPARENT; bar_size=0; bar_image.clear(); side_min_scale=1; if(desc.side_stretch && top_image){bar_color=side_color; bar_size=top_size; Swap(bar_image, top_image);}

            if(blur_color.a){Swap(blur_color, center_color); blur_color.a=0;}
            if(f.ok())return true;
         }
      }break;

      case 5:
      {
         PanelDesc5 desc; if(f.getFast(desc))
         {
            Unaligned(       center_stretch, desc.       center_stretch);
            Unaligned(       center_shadow , desc.       center_shadow );
            Unaligned(       shadow_opacity, desc.       shadow_opacity);
            Unaligned(       center_color  , desc.       center_color  );
            Unaligned(       border_color  , desc.       border_color  );
            Unaligned(         side_color  , desc.         side_color  );
            Unaligned(         blur_color  , desc.         blur_color  );
            Unaligned(       shadow_radius , desc.       shadow_radius );
            Unaligned(       shadow_offset , desc.       shadow_offset );
            Unaligned(       border_size   , desc.       border_size   ); CHS(border_size);
            Unaligned(       center_scale  , desc.       center_scale  );
            Unaligned(          top_size   , desc.          top_size   );
            Unaligned(       bottom_size   , desc.       bottom_size   );
            Unaligned(   left_right_size   , desc.   left_right_size   );
            Unaligned(   top_corner_size   , desc.   top_corner_size   );
            Unaligned(bottom_corner_size   , desc.bottom_corner_size   );
            Unaligned(          top_offset , desc.          top_offset ); top_offset-=top_size/2;
            Unaligned(       bottom_offset , desc.       bottom_offset ); bottom_offset+=bottom_size/2;
            Unaligned(   left_right_offset , desc.   left_right_offset ); left_right_offset.x-=left_right_size/2;
            Unaligned(   top_corner_offset , desc.   top_corner_offset ); top_corner_offset-=top_corner_size/2;
            Unaligned(bottom_corner_offset , desc.bottom_corner_offset ); bottom_corner_offset.x-=bottom_corner_size/2; bottom_corner_offset.y+=bottom_corner_size/2;

            center_image.require(f._getAsset(), path);
            border_image.require(f._getAsset(), path);
               top_image.require(f._getAsset(), path);
            bottom_image.require(f._getAsset(), path);
        left_right_image.require(f._getAsset(), path);
        top_corner_image.require(f._getAsset(), path);
     bottom_corner_image.require(f._getAsset(), path);
             panel_image.require(f._getAsset(), path);
              side_mode=SM_DEFAULT; bar_color=TRANSPARENT; bar_size=0; bar_image.clear(); side_min_scale=1; if(desc.side_stretch && top_image){bar_color=side_color; bar_size=top_size; Swap(bar_image, top_image);}

            if(blur_color.a){Swap(blur_color, center_color); blur_color.a=0;}
            if(f.ok())return true;
         }
      }break;

      case 4:
      {
         #pragma pack(push, 1)
         struct PanelDesc4
         {
            Byte  center_stretch, shadow_opacity;
            Color center_color, border_color, blur_color;
            Flt   shadow_offset, shadow_radius, border_size, center_scale, corner_size, top_size, bottom_size, left_right_size;
            Vec2  corner_offset, top_offset, bottom_offset, left_right_offset;
         }desc;
         #pragma pack(pop)
         if(f.getFast(desc))
         {
                          center_stretch=(Unaligned(desc.center_stretch)!=0);
            Unaligned(    shadow_opacity, desc.    shadow_opacity );
            Unaligned(    center_color  , desc.    center_color   );
            Unaligned(    border_color  , desc.    border_color   );
            Unaligned(      blur_color  , desc.      blur_color   );
            Unaligned(    shadow_offset , desc.    shadow_offset  );
            Unaligned(    shadow_radius , desc.    shadow_radius  );
            Unaligned(    border_size   , desc.    border_size    ); CHS(border_size);
            Unaligned(    center_scale  , desc.    center_scale   );
            Unaligned(top_corner_size   , desc.    corner_size    );
            Unaligned(       top_size   , desc.       top_size    );
            Unaligned(    bottom_size   , desc.    bottom_size    );
            Unaligned(left_right_size   , desc.left_right_size    );
            Unaligned(top_corner_offset , desc.    corner_offset  ); top_corner_offset-=top_corner_size/2;
            Unaligned(       top_offset , desc.       top_offset.y); top_offset-=top_size/2;
            Unaligned(    bottom_offset , desc.    bottom_offset.y); bottom_offset+=bottom_size/2;
            Unaligned(left_right_offset , desc.left_right_offset  ); left_right_offset.x-=left_right_size/2;

                   top_image.require(f._getStr1(), path);
                center_image.require(f._getStr1(), path);
                border_image.require(f._getStr1(), path);
            top_corner_image.require(f._getStr1(), path);
                bottom_image.require(f._getStr1(), path);
            left_right_image.require(f._getStr1(), path);

              side_color        =border_color;
            shadow_offset      *=shadow_radius;
            bottom_corner_size  =top_corner_size;
            bottom_corner_image =top_corner_image;
            bottom_corner_offset.set(top_corner_offset.x, -top_corner_offset.y);
             panel_image        .clear();
            center_shadow=!Equal(shadow_offset, 0);
              side_mode=SM_DEFAULT; bar_color=TRANSPARENT; bar_size=0; bar_image.clear(); side_min_scale=1;

            if(blur_color.a){Swap(blur_color, center_color); blur_color.a=0;}
            if(f.ok())return true;
         }
      }break;

      case 3:
      {
         PanelDesc2 desc; if(f.getFast(desc))
         {
                          center_stretch=(Unaligned(desc.center_stretch)!=0);
            Unaligned(    shadow_opacity, desc.shadow_opacity );
            Unaligned(    center_color  , desc.center_color   );
            Unaligned(    border_color  , desc.border_color   );
            Unaligned(      blur_color  , desc.  blur_color   );
            Unaligned(    shadow_offset , desc.shadow_offset  );
            Unaligned(    shadow_radius , desc.shadow_radius  );
            Unaligned(    border_size   , desc.border_size    ); CHS(border_size);
            Unaligned(    center_scale  , desc.center_scale   );
            Unaligned(top_corner_size   , desc.corner_size    );
            Unaligned(       top_size   , desc.   top_size    );
            Unaligned(top_corner_offset , desc.corner_offset  ); top_corner_offset-=top_corner_size/2;
            Unaligned(       top_offset , desc.   top_offset.y); top_offset-=top_size/2;

                   top_image.require(f._getStr(), path);
                center_image.require(f._getStr(), path);
                border_image.require(f._getStr(), path);
            top_corner_image.require(f._getStr(), path);

              side_color        =border_color;
            shadow_offset      *=shadow_radius;
            bottom_corner_size  =top_corner_size;
            bottom_corner_image =top_corner_image;
            bottom_corner_offset.set(top_corner_offset.x, -top_corner_offset.y);
             panel_image        .clear();
            bottom_image.clear(); left_right_image.clear(); bottom_size=left_right_size=DEFAULT_SIZE; left_right_offset=bottom_offset=0;
            center_shadow=!Equal(shadow_offset, 0);
              side_mode=SM_DEFAULT; bar_color=TRANSPARENT; bar_size=0; bar_image.clear(); side_min_scale=1;

            if(blur_color.a){Swap(blur_color, center_color); blur_color.a=0;}
            if(f.ok())return true;
         }
      }break;

      case 2:
      {
         PanelDesc2 desc; if(f.getFast(desc))
         {
                          center_stretch=(Unaligned(desc.center_stretch)!=0);
            Unaligned(    shadow_opacity, desc.shadow_opacity );
            Unaligned(    center_color  , desc.center_color   ); Swap(center_color.r, center_color.b);
            Unaligned(    border_color  , desc.border_color   ); Swap(border_color.r, border_color.b);
            Unaligned(      blur_color  , desc.  blur_color   ); Swap(  blur_color.r,   blur_color.b);
            Unaligned(    shadow_offset , desc.shadow_offset  );
            Unaligned(    shadow_radius , desc.shadow_radius  );
            Unaligned(    border_size   , desc.border_size    ); CHS(border_size);
            Unaligned(    center_scale  , desc.center_scale   );
            Unaligned(top_corner_size   , desc.corner_size    );
            Unaligned(       top_size   , desc.   top_size    );
            Unaligned(top_corner_offset , desc.corner_offset  ); top_corner_offset-=top_corner_size/2;
            Unaligned(       top_offset , desc.   top_offset.y); top_offset-=top_size/2;

                   top_image.require(f._getStr(), path);
                center_image.require(f._getStr(), path);
                border_image.require(f._getStr(), path);
            top_corner_image.require(f._getStr(), path);

              side_color        =border_color;
            shadow_offset      *=shadow_radius;
            bottom_corner_size  =top_corner_size;
            bottom_corner_image =top_corner_image;
            bottom_corner_offset.set(top_corner_offset.x, -top_corner_offset.y);
             panel_image        .clear();
            bottom_image.clear(); left_right_image.clear(); bottom_size=left_right_size=DEFAULT_SIZE; left_right_offset=bottom_offset=0;
            center_shadow=!Equal(shadow_offset, 0);
              side_mode=SM_DEFAULT; bar_color=TRANSPARENT; bar_size=0; bar_image.clear(); side_min_scale=1;

            if(blur_color.a){Swap(blur_color, center_color); blur_color.a=0;}
            if(f.ok())return true;
         }
      }break;

      case 1:
      {
         #pragma pack(push, 4)
         struct PanelDesc1
         {
            Byte  shadow_opacity, center_stretch;
            VecB4 center_color, border_color, blur_color;
            Flt   center_scale, border_size, shadow_radius, shadow_offset;
         }desc;
         #pragma pack(pop)
         if(f.getFast(desc))
         {
                          center_stretch=(Unaligned(desc.center_stretch)!=0);
            Unaligned(    shadow_opacity, desc.shadow_opacity);
                          center_color  .set(Unaligned(desc.center_color.z), Unaligned(desc.center_color.y), Unaligned(desc.center_color.x), Unaligned(desc.center_color.w));
                          border_color  .set(Unaligned(desc.border_color.z), Unaligned(desc.border_color.y), Unaligned(desc.border_color.x), Unaligned(desc.border_color.w));
                            blur_color  .set(Unaligned(desc.  blur_color.z), Unaligned(desc.  blur_color.y), Unaligned(desc.  blur_color.x), Unaligned(desc.  blur_color.w));
            Unaligned(    shadow_offset , desc.shadow_offset);
            Unaligned(    shadow_radius , desc.shadow_radius);
            Unaligned(    border_size   , desc.border_size  ); CHS(border_size);
            Unaligned(    center_scale  , desc.center_scale );
            Unaligned(top_corner_size   , desc.border_size  );
            Unaligned(       top_size   , desc.border_size  );
                      top_corner_offset =-top_corner_size/2;
                             top_offset =-top_size/2;

                   top_image.require(f._getStr8(), path);
                center_image.require(f._getStr8(), path);
                border_image.require(f._getStr8(), path);
            top_corner_image.require(f._getStr8(), path);

              side_color        =border_color;
            shadow_offset      *=shadow_radius;
            bottom_corner_size  =top_corner_size;
            bottom_corner_image =top_corner_image;
            bottom_corner_offset.set(top_corner_offset.x, -top_corner_offset.y);
             panel_image        .clear();
            bottom_image.clear(); left_right_image.clear(); bottom_size=left_right_size=DEFAULT_SIZE; left_right_offset=bottom_offset=0;
            center_shadow=!Equal(shadow_offset, 0);
              side_mode=SM_DEFAULT; bar_color=TRANSPARENT; bar_size=0; bar_image.clear(); side_min_scale=1;

            if(blur_color.a){Swap(blur_color, center_color); blur_color.a=0;}
            if(f.ok())return true;
         }
      }break;

      case 0:
      {
         #pragma pack(push, 4)
         struct PanelDesc0
         {
            Byte  shadow_opacity, center_stretch;
            VecB4 center_color, border_color;
            Flt   center_scale, border_size, shadow_radius, shadow_offset;
         }desc;
         #pragma pack(pop)
         if(f.getFast(desc))
         {
                          center_stretch=(Unaligned(desc.center_stretch)!=0);
            Unaligned(    shadow_opacity, desc.shadow_opacity);
                          center_color  .set(Unaligned(desc.center_color.z), Unaligned(desc.center_color.y), Unaligned(desc.center_color.x), Unaligned(desc.center_color.w));
                          border_color  .set(Unaligned(desc.border_color.z), Unaligned(desc.border_color.y), Unaligned(desc.border_color.x), Unaligned(desc.border_color.w));
                            blur_color  .zero();
            Unaligned(    shadow_offset , desc.shadow_offset);
            Unaligned(    shadow_radius , desc.shadow_radius);
            Unaligned(    border_size   , desc.border_size  ); CHS(border_size);
            Unaligned(    center_scale  , desc.center_scale );
            Unaligned(top_corner_size   , desc.border_size  );
            Unaligned(       top_size   , desc.border_size  );
                      top_corner_offset =-top_corner_size/2;
                             top_offset =-top_size/2;

                   top_image.require(f._getStr8(), path);
                center_image.require(f._getStr8(), path);
                border_image.require(f._getStr8(), path);
            top_corner_image.require(f._getStr8(), path);

              side_color        =border_color;
            shadow_offset      *=shadow_radius;
            bottom_corner_size  =top_corner_size;
            bottom_corner_image =top_corner_image;
            bottom_corner_offset.set(top_corner_offset.x, -top_corner_offset.y);
             panel_image        .clear();
            bottom_image.clear(); left_right_image.clear(); bottom_size=left_right_size=DEFAULT_SIZE; left_right_offset=bottom_offset=0;
            center_shadow=!Equal(shadow_offset, 0);
              side_mode=SM_DEFAULT; bar_color=TRANSPARENT; bar_size=0; bar_image.clear(); side_min_scale=1;

            if(f.ok())return true;
         }
      }break;
   }
   reset(); return false;
}
Bool Panel::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool Panel::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   reset(); return false;
}
void Panel::operator=(C Str &name)
{
   if(!load(name))Exit(MLT(S+"Can't load Panel \""        +name+"\"",
                       PL,S+u"Nie można wczytać Panelu \""+name+"\""));
}
/******************************************************************************/
}
/******************************************************************************/
