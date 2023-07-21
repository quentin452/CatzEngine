/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
GuiPC::GuiPC(C GuiPC &old, Region &region)
{
   visible    =old.visible&region.visible();
   enabled    =old.enabled&region.enabled();
   client_rect=region.clientRect()+old.offset;
   offset     =client_rect.lu();
   offset.x  -=region.slidebar[0].offset();
   offset.y  +=region.slidebar[1].offset();
   GuiSkin *skin=region.getSkin(); clip=old.clip&((skin && skin->region.normal && skin->region.normal->pixelBorder()) ? Rect(client_rect).extend(-D._pixel_size) : client_rect); // if the panel draws 1-pixel-border then leave it, this is so that Region children will not draw on top of it, because border should look like it's on top

   if(1)D.alignScreenToPixel(offset);
}
/******************************************************************************/
void Region::zero()
{
   kb_lit=true; _flag=0;
  _slidebar_size=0.05f;
  _crect.zero();
}
Region::Region() {zero();}
Region& Region::del()
{
  _children   .del  ();
   slidebar[0].del  ();
   slidebar[1].del  ();
   view       .del  ();
  _skin       .clear();
   super::del(); zero(); return T;
}
void Region::setParent(Bool on)
{
   slidebar[0]._parent=slidebar[1]._parent=view._parent=(on ? this : null);
}
void Region::setParams()
{
  _type=GO_REGION;
   view._sub_type=BUTTON_TYPE_REGION_VIEW;
   setParent();
}
Region& Region::create()
{
   del();

  _visible=true;

   view       .create().mode=BUTTON_CONTINUOUS;
   slidebar[0].create().setLengths(0, 0).hide();
   slidebar[1].create().setLengths(0, 0).hide();
   view._focusable=slidebar[0]._focusable=slidebar[1]._focusable=false;
   setParams();

   return T;
}
Region& Region::create(C Region &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
        _children.del();
         copyParams(src);
        _type         =GO_REGION;
         kb_lit       =src. kb_lit;
        _slidebar_size=src._slidebar_size;
        _flag         =src._flag;
        _skin         =src._skin;
        _crect        =src._crect;
         view       .create(src.view       );
         slidebar[0].create(src.slidebar[0]);
         slidebar[1].create(src.slidebar[1]);
         setParent();
      }   
   }
   return T;
}
/******************************************************************************/
void Region::childRectChanged(C Rect *old_rect, C Rect *new_rect, GuiObj &child)
{
   // null rectangle means object is hidden
   if(old_rect || new_rect)
   {
      // decreasing size if old size was greater than virtual size and new size isn't
      Flt     x=(new_rect ? GuiMaxX(*new_rect) : 0),
              y=(new_rect ? GuiMaxY(*new_rect) : 0),
          old_x=(old_rect ? GuiMaxX(*old_rect) : 0),
          old_y=(old_rect ? GuiMaxY(*old_rect) : 0);
      if((old_x>=virtualWidth ()-EPS && x<virtualWidth ()-EPS) // if decreasing in at least one dimension then we need to recalculate fully
      || (old_y>=virtualHeight()-EPS && y<virtualHeight()-EPS))virtualSize(null);else
      {
         // increasing size if new size is greater than virtual size
         Bool inc_w=(x>virtualWidth ()+EPS),
              inc_h=(y>virtualHeight()+EPS);
         if(inc_w || inc_h)
         {
            if(inc_w)slidebar[0]._length_total=x;
            if(inc_h)slidebar[1]._length_total=y;
            setRectAndButtons();
         }
      }
   }//else having null rectangles means object is hidden and we don't need to do anything
}
void Region::addChild(GuiObj &child)
{
   if(_children.add(child, T))childRectChanged(null, child.visible() ? &child.rect() : null, child);
}
void Region::removeChild(GuiObj &child)
{
   if(_children.remove(child))childRectChanged(child.visible() ? &child.rect() : null, null, child);
}
/******************************************************************************/
Region& Region::removeSlideBars()
{
   setParent(false); // detach before setting rectangle so virtual size won't be changed
   slidebar[0].del();
   slidebar[1].del();
   view       .del();
   setRectAndButtons(); return T;
}
Bool    Region::alwaysHideHorizontalSlideBar(         )C {return FlagOn(_flag, ALWAYS_HIDE_HORIZONTAL_SLIDEBAR);}
Region& Region::alwaysHideHorizontalSlideBar(Bool hide)
{
   if(hide!=alwaysHideHorizontalSlideBar())
   {
     _flag^=ALWAYS_HIDE_HORIZONTAL_SLIDEBAR;
      setRectAndButtons();
   }
   return T;
}
/******************************************************************************/
void Region::setRectAndButtons()
{
   Rect old_client=localClientRect();

   // first calculate client rectangle
   T._crect=rect();
   if(GuiSkin *skin=getSkin())
      if(Panel *panel=skin->region.normal())
   {
      Rect padd; panel->innerPadding(_crect, padd);
     _crect.min+=padd.min;
     _crect.max-=padd.max;
   }

   Bool vertical, horizontal;
   Flt  width =virtualWidth (),
        height=virtualHeight();

   Rect srect=_crect, mrect=_crect;
   if( vertical  =(slidebar[1].is() && clientHeight()+EPS<height)){srect.max.x-=slidebarSize();                                    _crect.max.x-=slidebarSize();}
   if( horizontal=(slidebar[0].is() && clientWidth ()+EPS<width )){srect.min.y+=slidebarSize(); if(!alwaysHideHorizontalSlideBar())_crect.min.y+=slidebarSize();
   if(!vertical && slidebar[1].is() && clientHeight()+EPS<height ){srect.max.x-=slidebarSize();                                    _crect.max.x-=slidebarSize(); vertical=true;}}

   setParent(false); // detach before setting rectangle so virtual size won't be changed
   slidebar[0].setLengths(clientWidth (), width ).rect(Rect(mrect.min.x               , mrect.min.y, srect.max.x, mrect.min.y+slidebarSize())); slidebar[0].visible(slidebar[0]._usable && !alwaysHideHorizontalSlideBar());
   slidebar[1].setLengths(clientHeight(), height).rect(Rect(mrect.max.x-slidebarSize(), srect.min.y, mrect.max.x, mrect.max.y               )); slidebar[1].visible(slidebar[1]._usable                                   );
   view                                          .rect(Rect(mrect.max.x-slidebarSize(), mrect.min.y, mrect.max.x, mrect.min.y+slidebarSize())).visible(slidebar[0]._usable && slidebar[1]._usable);
   setParent();

   Rect new_client=localClientRect();
   if(  old_client!=new_client)notifyChildrenOfClientRectChange(&old_client, &new_client);
}
static void ChildrenSize(Vec2 &size, C GuiObjChildren &children)
{
   REPA(children)if(C GuiObj *c=children[i])if(c->visible())
   {
      MAX(size.x, GuiMaxX(c->rect()));
      MAX(size.y, GuiMaxY(c->rect()));
      if(c->type()==GO_TABS)
      {
       C Tabs &tabs=c->asTabs(); if(InRange(tabs(), tabs))ChildrenSize(size, tabs.tab(tabs())._children);
      }
   }
}
Vec2 Region::childrenSize()C
{
   Vec2 size=0;
   ChildrenSize(size, _children);
   return size;
}
Region& Region::slidebarSize(Flt size)
{
   MAX(size, 0);
   if(_slidebar_size!=size)
   {
     _slidebar_size=size;
      setRectAndButtons();
   }
   return T;
}
Region& Region::virtualSize(C Vec2 *size)
{
   Vec2 temp; if(!size)size=&(temp=childrenSize());
   slidebar[0]._length_total=size->x;
   slidebar[1]._length_total=size->y;
   setRectAndButtons(); return T;
}
Region& Region::rect(C Rect &rect)
{
   if(T.rect()!=rect)
   {
      Rect /*old_client=localClientRect(), */old_rect=T.rect(); T._rect=rect; setRectAndButtons(); // set manually instead of calling 'super::rect' because that will call 'notifyChildrenOfClientRectChange' before we finish setting client rectangle
    //Rect   new_client=localClientRect();
    //notifyChildrenOfClientRectChange(&old_client, &new_client); already called in 'setRectAndButtons'
      notifyParentOfRectChange        ( old_rect  ,  visible() );
   }
   return T;
}
Region& Region::move(C Vec2 &delta)
{
   if(delta.any())
   {
      super::move(delta);
     _crect  +=   delta ;
      setParent(false); // detach before setting rectangle so virtual size won't be changed
      view       .move(delta);
      slidebar[0].move(delta);
      slidebar[1].move(delta);
      setParent();
   }
   return T;
}
/******************************************************************************/
Region& Region::scrollTo(C GuiObj &child, Bool immediate)
{
   if(contains(&child))
   {
      Rect child_rect=child.screenRect();
      Vec2  this_pos =      screenPos ();
      this_pos.x-=slidebar[0].offset();
      this_pos.y+=slidebar[1].offset();
      slidebar[0].scrollFit(child_rect.min.x-this_pos.x, child_rect.max.x-this_pos.x, immediate);
      slidebar[1].scrollFit(this_pos.y-child_rect.max.y, this_pos.y-child_rect.min.y, immediate);
   }
   return T;
}
/******************************************************************************/
Region& Region::skin(C GuiSkinPtr &skin, Bool sub_objects)
{
   if(sub_objects)
   {
            view     .skin=skin ;
      REPAO(slidebar).skin(skin);
   }
   if(_skin!=skin)
   {
     _skin=skin;
      setRectAndButtons();
   }
   return T;
}
/******************************************************************************/
GuiObj* Region::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   if(GuiObj *go=super::test(gpc, pos, mouse_wheel))
   {
      if(!Kb.ctrlCmd()) // when Ctrl is hold then don't set mouse wheel focus to slidebars when we focus on Region, to allow child 'ComboBox' and 'Slider' to catch it, and if nothing is catched, then still leave empty so we don't accidentally scroll when using mouse wheel near ComboBox or Slider but accidentally not on it
      {
         Bool priority=!Kb.shift(); // when 'Kb.shift' disabled (default) then priority=1 (vertical), when enabled then priority=0 (horizontal)
         if(slidebar[ priority]._usable)mouse_wheel=&slidebar[ priority];else // check  priority slidebar first
         if(slidebar[!priority]._usable)mouse_wheel=&slidebar[!priority];     // check !priority slidebar next
      }

      GuiPC gpc_children(gpc, T                   ); if(GuiObj *go=_children.test(gpc_children, pos, mouse_wheel))return go;
      GuiPC gpc_this    (gpc, visible(), enabled());
      if(GuiObj *go=slidebar[0].test(gpc_this, pos, mouse_wheel))return go;
      if(GuiObj *go=slidebar[1].test(gpc_this, pos, mouse_wheel))return go;
      if(GuiObj *go=view       .test(gpc_this, pos, mouse_wheel))return go;

      return go;
   }
   return null;
}
void Region::nearest(C GuiPC &gpc, GuiObjNearest &gon)
{
   if(/*gpc.visible &&*/ visible() && gon.test((rect()+gpc.offset)&gpc.clip))
   {
      GuiPC gpc_children(gpc, T); _children.nearest(gpc_children, gon);
   }
}
GuiObj* Region::nearest(C Vec2 &screen_pos, C Vec2 &dir)
{
   if(_children.children.elms())
   {
      GuiObjNearest gon; gon.plane.normal=dir; if(gon.plane.normal.normalize())
      {
         GuiPC gpc;
         gpc.visible  =gpc.enabled=true;
         gpc.offset   =screenPos();
         gpc.offset.x-=slidebar[0].offset();
         gpc.offset.y+=slidebar[1].offset();
         gpc.client_rect=gpc.clip.set(-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX);
         GuiObj *mouse_wheel=null;

            gon.state    =0;
            gon.rect     =screen_pos;
            gon.plane.pos=screen_pos;
            gon.min_dist =D.pixelToScreenSize().max(); // use pixel size because this function may operate on mouse position which may be aligned to pixels
         if(gon.obj      =_children.test(gpc, screen_pos, mouse_wheel))switch(gon.obj->type())
         {
            case GO_NONE   : // ignore for GO_NONE too, which is used for 'ModalWindow._background'
            case GO_DESKTOP:
            case GO_WINDOW :
            case GO_REGION :
               break;
            default: gon.rect=gon.obj->screenRect().extend(-EPS); break;
         }

        _children.nearest(gpc, gon);

         if(auto *nearest=gon.findNearest())
         {
          //out_pos=nearest->rect.center();
            return  nearest->obj;
         }
      }
   }
   return null;
}
/******************************************************************************/
void Region::update(C GuiPC &gpc)
{
   GuiPC gpc_this(gpc, visible(), enabled());
   if(   gpc_this.enabled)
   {
      DEBUG_BYTE_LOCK(_used);

      view.update(gpc_this);
      if(view())
      {
         if(Gui.ms()==&view)
         {
            Ms.freeze();
            slidebar[0].offset(slidebar[0]._offset + Ms.d().x*2);
            slidebar[1].offset(slidebar[1]._offset - Ms.d().y*2);
         }
         REPA(Touches)
         {
            Touch &t=Touches[i]; if(t.guiObj()==&view && t.on())
            {
               slidebar[0].offset(slidebar[0]._offset + t.d().x*2);
               slidebar[1].offset(slidebar[1]._offset - t.d().y*2);
            }
         }
      }

      // scroll horizontally
      if(Ms.wheelX()
      && (Gui.wheel()==&slidebar[0] || Gui.wheel()==&slidebar[1]) // if has focus on any of slidebars
      && slidebar[0]._usable) // we will scroll only horizontally, so check if that's possible
         slidebar[0].scroll(Ms.wheelX()*(slidebar[0]._scroll_mul*slidebar[0].length()+slidebar[0]._scroll_add), slidebar[0]._scroll_immediate);

      slidebar[0].update(gpc_this);
      slidebar[1].update(gpc_this);

      GuiPC gpc_children(gpc, T); _children.update(gpc_children);
   }
}
void Region::draw(C GuiPC &gpc)
{
   if(/*gpc.visible &&*/ visible())
   {
      GuiSkin *skin=getSkin();
      Rect     rect=T.rect()+gpc.offset, ext_rect;
      if(skin && skin->region.normal)skin->region.normal->extendedRect(rect, ext_rect);else ext_rect=rect;
      if(Cuts(ext_rect, gpc.clip))
      {
         if(skin)
         {
            D.clip(gpc.clip);
            if(skin->region.normal        )skin->region.normal->draw(skin->region.normal_color, rect);else
            if(skin->region.normal_color.a)                rect.draw(skin->region.normal_color);
         }
         view       .draw(gpc);
         slidebar[0].draw(gpc);
         slidebar[1].draw(gpc);

         GuiPC gpc_children(gpc, T); _children.draw(gpc_children);

         if(kb_lit && contains(Gui.kb())){D.clip(gpc.clip); Gui.kbLit(this, rect, skin);}
      }
   }
}
/******************************************************************************/
Bool Region::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      // !! in the future save '_flag' !!
      f.putMulti(Byte(3), kb_lit, _slidebar_size, _crect); // version
      f.putAsset(_skin.id());
      if(view       .save(f, path))
      if(slidebar[0].save(f, path))
      if(slidebar[1].save(f, path))
         return f.ok();
   }
   return false;
}
Bool Region::load(File &f, CChar *path)
{
   del(); if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 3:
      {
         f.getMulti(kb_lit, _slidebar_size, _crect);
        _skin.require(f.getAssetID(), path);
         if(view       .load(f, path))
         if(slidebar[0].load(f, path))
         if(slidebar[1].load(f, path))
            if(f.ok()){setParams(); return true;}
      }break;

      case 2:
      {
         f>>kb_lit>>_slidebar_size>>_crect;
        _skin.require(f._getAsset(), path);
         if(view       .load(f, path))
         if(slidebar[0].load(f, path))
         if(slidebar[1].load(f, path))
            if(f.ok()){setParams(); return true;}
      }break;

      case 1:
      {
         f>>kb_lit>>_slidebar_size>>_crect;
         if(view       .load(f, path))
         if(slidebar[0].load(f, path))
         if(slidebar[1].load(f, path))
            if(f.ok()){f._getStr(); setParams(); return true;}
      }break;

      case 0:
      {
         f>>kb_lit>>_slidebar_size>>_crect;
         if(view       .load(f, path))
         if(slidebar[0].load(f, path))
         if(slidebar[1].load(f, path))
            if(f.ok()){f._getStr8(); setParams(); return true;}
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
