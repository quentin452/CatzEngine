/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
static void MenuPushed(C CMemPtr<MenuPush> &path, ComboBox &cb)
{
   if(path.elms())cb.setAbsText(path.first().abs, path.last().name);
}
static void ChangedButton(ComboBox &cb)
{
   if(cb.Button::operator()())
   {
      Bool touch=false; REPA(Touches)if(Touches[i].pd() && Touches[i].guiObj()==&cb){touch=true; break;} cb.menu.setSize(touch);
      cb.menu.list.cur=-1;
      if(0 && D.smallSize())cb.menu.posC(0);else cb.menu.posAround(cb.screenRect(), cb.menu_align);
      cb.menu.activate();
   }else
   {
      cb.menu.hide();
   }
}
/******************************************************************************/
void ComboBox::zero()
{
   flag      =COMBOBOX_MOUSE_WHEEL;
   menu_align= 1;
  _abs_sel   =-1;

  _func_immediate=false;
  _func_user     =null;
  _func          =null;
}
ComboBox::ComboBox() {zero();}
ComboBox& ComboBox::del()
{
   menu.del();
   super::del(); zero(); return T;
}
void ComboBox::setParams()
{
  _type    =GO_COMBOBOX;
  _sub_type=BUTTON_TYPE_COMBOBOX;

   super::func(ChangedButton, T); // this must be 'Button.func'

   menu.create().func(MenuPushed, T).skin(skin())._owner=this;
   Gui+=menu; // add after setting '_owner' because it may affect level in children array
}
ComboBox& ComboBox::create()
{
   del();

   super::create();
   setParams(); // call after 'create'
   mode      =BUTTON_TOGGLE; // call after 'create'
   text_align=1; // call after 'create'

   return T;
}
ComboBox& ComboBox::create(C ComboBox &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         super::create(src);
        _type          =GO_COMBOBOX;
         flag          =src. flag;
         menu_align    =src. menu_align;
        _abs_sel       =src._abs_sel;
        _func_immediate=src._func_immediate;
        _func_user     =src._func_user;
        _func          =src._func;

         super::func(ChangedButton, T); // this must be 'Button.func'

              T.menu.create(src.menu).func(MenuPushed, T)._owner=this;
         Gui+=T.menu; // add after setting '_owner' because it may affect level in children array
      }   
   }
   return T;
}
/******************************************************************************/
ComboBox& ComboBox::clear()
{
   menu.list.clear();
   return T;
}
/******************************************************************************/
ComboBox& ComboBox::resetText()
{
   if(!(flag&COMBOBOX_CONST_TEXT))
   {
      if(absSel()>=0)if(ListColumn *lc=menu.listColumn()){text=lc->md.asText(menu.list.absToData(absSel())); extra.clear(); goto end;}
      text.clear(); extra.clear();
   end:;
   }
   return T;
}
ComboBox& ComboBox::set(Int abs_sel, SET_MODE mode)
{
   if(flag&COMBOBOX_SET_CLAMP)MIN(abs_sel, menu.list.totalElms()-1); // clamp to last value
   if(menu.list.absToVis(abs_sel)<0)abs_sel=-1; // disable selecting hidden and out of range elements
   if(T._abs_sel!=abs_sel)
   {
      T._abs_sel=abs_sel;
      resetText();
      if(mode!=QUIET)call();
   }else resetText(); // if we're not changing selection, the element's text can be now different, so set it in case it got changed
   return T;
}
ComboBox& ComboBox::setText(C Str &text, Bool force, SET_MODE mode)
{
   StrEx extra;

   // find element that matches desired text
   if(ListColumn *lc=menu.listColumn())
   {
      REPA(menu.list)if(Equal(lc->md.asText(menu.list.visToData(i), lc->precision), text) && !extra.is())
      {
         return set(menu.list.visToAbs(i), mode);
      }
   }

   // element was not found
   if(!Equal(T.text, text, true) || T.extra!=extra) // new text is different
   {
     _abs_sel=-1;
    //if(!(flag&COMBOBOX_CONST_TEXT))
      {
         if(force){T.text=text   ; T.extra=extra  ;}
         else     {T.text.clear(); T.extra.clear();}
      }
      if(mode!=QUIET)call();
   }
   return T;
}
ComboBox& ComboBox::setAbsText(Int abs, C Str &text)
{
   StrEx extra;
   if(menu.list.absToVis(abs)<0)abs=-1; // disable selecting hidden and out of range elements
   if(abs!=_abs_sel || !Equal(T.text, text, true) || T.extra!=extra)
   {
     _abs_sel=abs;
      if(!(flag&COMBOBOX_CONST_TEXT)){T.text=text; T.extra=extra;}
      call();
   }
   return T;
}
/******************************************************************************/
ComboBox& ComboBox::rect(C Rect &rect)
{
   if(T.rect()!=rect)
   {
      super::rect(rect);
      menu.list.setRects();
   }
   return T;
}
/******************************************************************************/
ComboBox& ComboBox::skin(C GuiSkinPtr &skin, Bool sub_objects)
{
   super::skin=skin;
   if(sub_objects)menu.skin(skin, true);
   return T;
}
/******************************************************************************/
ComboBox& ComboBox::func(void (*func)(Ptr), Ptr user, Bool immediate)
{
   T._func          =func;
   T._func_user     =user;
   T._func_immediate=immediate;
   return T;
}
void ComboBox::call()
{
   if(_func)
   {
      if(_func_immediate)
      {
         DEBUG_BYTE_LOCK(_used); _func(_func_user);
      }else Gui.addFuncCall(_func, _func_user);
   }
}
/******************************************************************************/
GuiObj* ComboBox::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   if(GuiObj *go=super::test(gpc, pos, mouse_wheel))
   {
      if(flag&COMBOBOX_MOUSE_WHEEL)
         if(!mouse_wheel || mouse_wheel->type()!=GO_SLIDEBAR) // don't grab wheel focus if it already belongs to a SlideBar (for example ComboBox located in a Region with SlideBars)
            mouse_wheel=this;
      return go;
   }
   return null;
}
/******************************************************************************/
static void Next(ComboBox &cb, Int delta)
{
   if(Int sign=Sign(delta))for(Int i=cb.visSel()+sign; InRange(i, cb); i+=sign)
   {
      if(InRange(i, cb.menu.elms()))
      {
         MenuElm &elm=cb.menu.elm(i);
         if(elm.disabled || (elm.flag()&MENU_NOT_SELECTABLE) || elm.menu())continue; // if element is disabled, can't be selected or has children (normally we'd need to select one of the children, but for simplicity let's just skip it)
      }
      if(cb.menu._selectable_offset>=0)if(Ptr data=cb.menu.list.visToData(i))if(!*(Bool*)((Byte*)data+cb.menu._selectable_offset))continue;
      if(delta-=sign)continue; // want to keep going
      cb.set(cb.menu.list.visToAbs(i));
      break;
   }
}
void ComboBox::update(C GuiPC &gpc)
{
   GuiPC gpc_this(gpc, visible(), enabled());
   if(   gpc_this.enabled)
   {
      Bool menu_active=Button::operator()(); // remember state before button update
      super::update(gpc_this);
      if(gpc_this.visible)
      {
         if(Gui.kb()==this)
         {
            if(Kb.k(KB_UP  ))Next(T, -1);
            if(Kb.k(KB_DOWN))Next(T, +1);
         }
         if(Gui.wheel()==this)Next(T, -Ms.wheelI());
         if(menu_active && !menu.contains(Gui.menu()))super::set(false, QUIET);
         if(Gui.kb()==this && (Kb.k(KB_ENTER) || Kb.k(KB_NPENTER)) && Kb.k.first())
         {
            Kb.eatKey();
            super::set(!Button::operator()());
         }
      }
   }
}
/******************************************************************************/
Bool ComboBox::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      f.cmpUIntV(0); // version
      f<<flag<<_abs_sel<<menu_align;
      return f.ok();
   }
   return false;
}
Bool ComboBox::load(File &f, CChar *path)
{
   del();

   {Long f_pos=f.pos(); UInt ver=f.decUIntV(); f.pos(f_pos); if(ver<3)goto old;} // in the past the base class of ComboBox was GuiObj, so detect that and jump to old load if needed, '3' is the GuiObj save file version from the past

   if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>flag>>_abs_sel>>menu_align;
         if(f.ok()){setParams(); return true;}
      }break;
   }

   // old format
   if(0)
   {
   old:
      if(GuiObj::load(f, path))switch(f.decUIntV()) // version
      {
         case 1:
         {
            f>>_focusable>>flag>>_abs_sel;
            if(super::load(f, path))
               if(f.ok()){setParams(); return true;}
         }break;

         case 0:
         {
            f>>_focusable>>flag; f.skip(4); f>>_abs_sel;
            if(TextLine().load(f, path))
            if(super    ::load(f, path))
               if(f.ok()){setParams(); return true;}
         }break;
      }
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
