/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
GuiPC::GuiPC(Desktop &desktop)
{
   visible=desktop.visible();
   enabled=desktop.enabled();
   offset.zero();
   client_rect=clip=desktop.rect();
}
/******************************************************************************/
void Desktop::zero()
{
}
Desktop::Desktop() {zero(); create();}
Desktop& Desktop::del()
{
   if(this==Gui.desktop())Gui._desktop=null;
  _children.del();
   super::del(); zero(); return T;
}
Desktop& Desktop::create()
{
   del();
 
  _type   =GO_DESKTOP;
  _visible=true;
  _rect   =D.rect();
   return T;
}
Desktop& Desktop::create(C Desktop &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
        _children.del();
         copyParams(src);
        _type=GO_DESKTOP;
      }
   }
   return T;
}
/******************************************************************************/
void Desktop::addChild(GuiObj &child)
{
  _children.add(child, T);
}
void Desktop::removeChild(GuiObj &child)
{
  _children.remove(child);
}
/******************************************************************************/
GuiObj* Desktop::test(C Vec2 &pos, GuiObj* &mouse_wheel)
{
   if(visible())
   {
      GuiPC gpc(T);

      mouse_wheel=this;

      // test overlay textline
      Vec2 offset; if(TextLine *tl=Gui.overlayTextLine(offset))
      {
         GuiPC gpc_tl=gpc; gpc_tl.offset=offset; if(GuiObj *go=tl->test(gpc_tl, pos, mouse_wheel))return go;
         if(Cuts(pos, Rect(D.rectUI().min.x, tl->rect().min.y+offset.y, D.rectUI().max.x, tl->rect().max.y+offset.y)))return tl;
      }

      // test children
      if(GuiObj *go=_children.test(gpc, pos, mouse_wheel))return go;

      // test self
      if(CutsEps(pos, rect()))return this; // use EPS version to make sure that for example Mouse Position will cut the Desktop despite numerical precision issues
   }
   return null;
}
void Desktop::nearest(GuiObjNearest &gon)
{
   if(visible())
   {
      GuiPC gpc(T); _children.nearest(gpc, gon);
   }
}
/******************************************************************************/
void Desktop::update()
{
   GuiPC gpc(T); DEBUG_BYTE_LOCK(_used); _children.update(gpc);
}
void Desktop::draw()
{
   if(visible())
   {
      // draw children
      GuiPC gpc(T); _children.draw(gpc);

      // draw overlay textline
      Vec2 offset; if(TextLine *tl=Gui.overlayTextLine(offset)) // don't use "Gui._overlay_textline" because offset can change during textline's window resize
      {
         D.clip(gpc.clip); Rect(D.rectUI().min.x, tl->rect().min.y+offset.y, D.rectUI().max.x, tl->rect().max.y+offset.y).extend(0.01f).drawShaded(Color(0, 0, 0, 208), TRANSPARENT, 0.01f);
         GuiPC gpc_tl=gpc; gpc_tl.offset=offset; tl->draw(gpc_tl);
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
