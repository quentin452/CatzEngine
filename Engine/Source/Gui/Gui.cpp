/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
struct QueuedMsgBox // queue these commands for thread-safety, this is so that 'Gui.msgBox' does not require 'Gui.cs' lock, this is important in case for example the main thread is deleting a thread inside a 'Gui.update' callback "Gui.update -> thread.del()" while that thread is "if(thread.wantStop())Gui.msgBox(failed)" calling 'Gui.msgBox' would require 'Gui.cs' lock which is already locked on the main thread
{
   Str          title, text;
   StrEx        extra;
   TextStylePtr text_style;

   void set(C Str &title, C Str &text, C TextStylePtr &text_style) {T.title=title; T.text=text; T.text_style=text_style;}
};
static Memc<QueuedMsgBox> QueuedMsgBoxs;
static SyncLock           QueuedMsgBoxLock;
/******************************************************************************/
struct MsgBox : Dialog
{
   static Memx<MsgBox> MsgBoxs;

   static void Del  (MsgBox &mb) {SyncLocker locker(Gui._lock); MsgBoxs.removeData(&mb);}
   static void Close(MsgBox &mb) {if(Gui.window_fade)mb.fadeOut();else Del(mb);}

   static MsgBox* Find(CPtr id) {REPA(MsgBoxs){MsgBox &mb=MsgBoxs[i]; if(mb.id==id)return &mb;} return null;}

   CPtr id;

   MsgBox& create(C Str &title, C Str &text, C TextStylePtr &text_style, CPtr id)
   {
      T.id=id;
      Gui+=super::create(title, text, Memt<Str>().add("OK"), text_style).level(65536);
      buttons[0].func(Close, T, false); // can't be immediate because we might delete this object inside callback
      if(Gui.window_fade){super::hide(); fadeIn();}
      activate();
      return T;
   }
   virtual MsgBox& hide() {Gui.addFuncCall(Del, T); return T;} // schedule deletion
};
Memx<MsgBox> MsgBox::MsgBoxs;
/******************************************************************************/
GUI Gui;
/******************************************************************************/
// GUI
/******************************************************************************/
GUI::GUI() : default_skin(3649875776, 1074192063, 580730756, 799774185), _desktops(4)
{
   allow_window_fade=true;
   window_fade=false;
   desc_delay=0.3f;
   resize_radius=0.022f;
   click_sound_id.zero();
   text_menu_height=0.08f;

   draw_keyboard_highlight=DrawKeyboardHighlight;
   draw_description       =DrawDescription;
   draw_imm               =DrawIMM;

   window_fade_in_speed =9;
   window_fade_out_speed=6;
   window_fade_scale    =0.85f;

   dialog_padd           =0.03f;
   dialog_button_height  =0.06f;
   dialog_button_padd    =dialog_button_height*2;
   dialog_button_margin  =dialog_button_height;
   dialog_button_margin_y=dialog_button_height*0.3f;

  _window_buttons_right=!OSMac(); // check for 'OSMac' instead of 'MAC' so it can be detected on WEB too, check for Mac instead of Apple, because iOS is also Apple, but since it's touchscreen based, prefer right side because most people are right-handed and it's easier to have the buttons on the right side
  _drag_want=_dragging=false;
  _drag_user  =null;
  _drag_start =null;
  _drag_cancel=null;
  _drag_finish=null;
  _pass_char='*';
  _kb=_ms=_ms_src=_ms_lit=_wheel=_desc=_touch_desc=null;
  _menu=null;
  _window=_window_lit=null;
  _desktop=null;
}
/******************************************************************************/
void GUI::screenChanged(Flt old_width, Flt old_height)
{
   FREPAO(_desktops).rect(D.rect());

   if(Menu *menu=Gui.menu())
      if(GuiObj *owner=menu->Owner())
         if(owner->isMenuBar() && owner->parent() && owner->parent()->isDesktop())
            menu->move(Vec2(old_width-D.w(), 0));
}
/******************************************************************************/
Bool GUI::Switch()
{
   if(Kb.alt())
   {
      // switch windows
   }else
   {
      // switch in window
      if(GuiObj *c=kb())
      if(GuiObj *p=c->parent())
      {
         switch(c->type())
         {
            case GO_MENU    :
            case GO_CUSTOM  :
            case GO_DESKTOP :
            case GO_VIEWPORT:
            case GO_REGION  :
            case GO_TEXTBOX :
            case GO_WINDOW  : return false;
            case GO_LIST    : if(p->isMenu    ())return false; c=p; p=c->parent(); if(!p)return false;  break;
            case GO_TEXTLINE: if(p->isComboBox()){             c=p; p=c->parent(); if(!p)return false;} break;
         }
         Bool next=!Kb.k.shift();
         switch(p->type())
         {
            case GO_DESKTOP: return p->asDesktop()._children.Switch(*c, next);
            case GO_REGION : return p->asRegion ()._children.Switch(*c, next);
            case GO_TAB    : return p->asTab    ()._children.Switch(*c, next);
            case GO_WINDOW : return p->asWindow ()._children.Switch(*c, next);
            case GO_TABS   : {Tabs &tabs=p->asTabs(); if(InRange(tabs(), tabs))return tabs.tab(tabs())._children.Switch(*c, next);} break;
         }
      }
   }
   return false;
}
/******************************************************************************/
void GUI::setText()
{
   REPAO(_desktops).setText();
}
/******************************************************************************/
struct TextMenuTab : Tab
{
   virtual TextMenuTab& activate()override {return T;} // ignore, because we cannot activate/change keyboard focus, because that would hide the screen keyboard
};
struct TextMenuTabs : Tabs
{
   virtual void draw(C GuiPC &gpc)override
   {
   #if 0 // shadow
      D.clip(gpc.clip);
      Rect r=rect()+gpc.offset;
      D.drawShadow(128, r, 0.01f);
   #endif
      ShaderParam *sp; Vec col; D.textBackgroundReset(sp, col); // text menu may be drawn with different style/colors/theme, so reset auto text background
      super::draw(gpc);
      D.textBackgroundSet(sp, col); // restore what was set originally
   }
};
static TextMenuTabs TextMenu;

Bool GUI::visibleTextMenu()C {return TextMenu.visible();}
void GUI::   hideTextMenu()  {  Gui-=TextMenu.hide   ();} // also hide so we can do fast check in 'visibleTextMenu'
static void TextSelectAll(Ptr user)
{
   Gui.hideTextMenu();
   if(GuiObj *go=Gui.kb())switch(go->type())
   {
      case GO_TEXTLINE: if(go->asTextLine().selectAll().cursor()<=0)return; break; // if have no selection then return and don't show menu
      case GO_TEXTBOX : if(go->asTextBox ().selectAll().cursor()<=0)return; break; // if have no selection then return and don't show menu
   }
   Gui.showTextMenu();
}
static void TextCut(Ptr user)
{
   Gui.hideTextMenu();
   if(GuiObj *go=Gui.kb())switch(go->type())
   {
      case GO_TEXTLINE: go->asTextLine().cut(); break;
      case GO_TEXTBOX : go->asTextBox ().cut(); break;
   }
}
static void TextCopy(Ptr user)
{
   Gui.hideTextMenu();
   if(GuiObj *go=Gui.kb())switch(go->type())
   {
      case GO_TEXTLINE: go->asTextLine().copy(); break;
      case GO_TEXTBOX : go->asTextBox ().copy(); break;
   }
}
static void TextPaste(Ptr user)
{
   Gui.hideTextMenu();
   if(GuiObj *go=Gui.kb())switch(go->type())
   {
      case GO_TEXTLINE: go->asTextLine().paste(); break;
      case GO_TEXTBOX : go->asTextBox ().paste(); break;
   }
}

void GUI::showTextMenu()
{
   if(GuiObj *go=Gui.kb())if(go->isTextEdit())
   {
      Rect rect;
      Bool sel, any, pass;
      switch(go->type())
      {
         case GO_TEXTLINE:
         {
            TextLine &tl=go->asTextLine();
            sel =(tl._edit.sel>=0);
            any = tl().is();
            pass= tl.password();
            rect= tl.localSelRect(); rect.clampFull(Rect_LU(0, 0, tl.clientWidth(), tl.clientHeight())); rect+=tl.overlayScreenPos();
         }break;

         case GO_TEXTBOX:
         {
            TextBox &tb=go->asTextBox();
            sel =(tb._edit.sel>=0);
            any = tb().is();
            pass=false;
            rect=tb.localSelRect(); rect.clampFull(Rect_LU(0, 0, tb.clientWidth(), tb.clientHeight())); rect+=tb.screenPos();
         }break;
      }
      Node<MenuElm> n;
      if(sel)
      {
         if(pass)n.New().create("Delete", TextCut);else
         {
            n.New().create("Cut" , TextCut);
            n.New().create("Copy", TextCopy);
         }
      }else
      if(any)n.New().create("Select All", TextSelectAll);
             n.New().create("Paste"     , TextPaste);

      Vec2 size(0, text_menu_height);
      rect.extendY(0.01f);

   #if 1
      TextMenu._tabs.replaceClass<TextMenuTab>();
      TextMenu.create((CChar**)null, n.children.elms(), false).skin(Gui.text_menu_skin).baseLevel(GBL_MENU); // for now disable 'auto_size' because we don't want changing tab text below calling resize
      FREPA(TextMenu)
      {
         Tab &tab=TextMenu.tab(i);
         MenuElm &m=n.children[i];
         tab.Button::setText(m.name); // don't call Tab.setText because that will resize Tabs
         tab.func(m._func1);
         size.x+=tab.textWidth(&size.y, true);
      }
      size.x+=(TextMenu.tabs()+1)*size.y/2; // spacing around elements
      // posAround
      Rect screen=D.rectUIKB();
      rect.clampFull(screen); // clip so we can detect position at screen center even if selection rectangle extends far away outside screen on one side
const Flt align=0;
      Flt pos_x=Lerp(rect.min.x, rect.max.x-size.x, LerpR(1.0f, -1.0f, align));
      Clamp(pos_x, screen.min.x, screen.max.x-size.x); // have to clip again

      Rect_LD above(pos_x, rect.max.y, size.x, size.y); // rect above 'rect'
      Rect_LU below(pos_x, rect.min.y, size.x, size.y); // rect below 'rect'
      Flt   h_above=(above&screen).h(), // visible menu height when using 'above'
            h_below=(below&screen).h(); // visible menu height when using 'below'
      const Bool prefer_up=true;
      if(h_above>h_below+(prefer_up ? -EPS : EPS))
      {
         rect=above;
         if(rect.max.y>screen.max.y)rect.moveY(screen.max.y-rect.max.y);
         if(TextMenu.tabs()==1)TextMenu.tab(0).subType(BUTTON_TYPE_TAB_TOP);else
         if(TextMenu.tabs()> 1)
         {
            TextMenu.tab(                0).subType(BUTTON_TYPE_TAB_TOP_LEFT );
            TextMenu.tab(TextMenu.tabs()-1).subType(BUTTON_TYPE_TAB_TOP_RIGHT);
         }
      }else
      {
         rect=below;
         if(rect.min.y<screen.min.y)rect.moveY(screen.min.y-rect.min.y);
         if(TextMenu.tabs()==1)TextMenu.tab(0).subType(BUTTON_TYPE_TAB_BOTTOM);else
         if(TextMenu.tabs()> 1)
         {
            TextMenu.tab(                0).subType(BUTTON_TYPE_TAB_BOTTOM_LEFT );
            TextMenu.tab(TextMenu.tabs()-1).subType(BUTTON_TYPE_TAB_BOTTOM_RIGHT);
         }
      }
      Gui+=TextMenu.rect(rect, 0, true);
   #else
      TextMenu.create(n).skin(Gui.text_menu_skin); TextMenu.list.cur_mode=LCM_MOUSE;
      if(GuiSkin *skin=TextMenu.getSkin())
         if(C PanelPtr &panel=skin->menu.normal)
      {
         Rect margin; panel->outerMargin(TextMenu.rect(), margin);
         rect.extendY(Max(margin.min.y, margin.max.y)); // use Max because we don't know yet if we'll place above or below
      }
      Gui+=TextMenu.posAround(rect, 0, true).show();
   #endif
   }
}
void GUI::updateTextMenu() {if(visibleTextMenu())showTextMenu();}
/******************************************************************************/
GuiObj* GUI::objAtPos(C Vec2 &pos)C
{
   GuiObj *mouse_wheel=null;
   return desktop() ? desktop()->test(pos, mouse_wheel) : null;
}
GuiObj* GUI::objNearest(C Vec2 &pos, C Vec2 &dir, Vec2 &out_pos)C
{
   if(desktop())
   {
      GuiObjNearest gon; gon.plane.normal=dir; if(gon.plane.normal.normalize())
      {
            gon.state    =0;
            gon.rect     =pos;
            gon.plane.pos=pos;
            gon.min_dist =D.pixelToScreenSize().max(); // use pixel size because this function may operate on mouse position which may be aligned to pixels
         if(gon.obj      =objAtPos(pos))switch(gon.obj->type())
         {
            case GO_NONE   : // ignore for GO_NONE too, which is used for 'ModalWindow._background'
            case GO_DESKTOP:
            case GO_WINDOW :
            case GO_REGION :
               break;
            default: gon.rect=gon.obj->screenRect().extend(-EPS); break;
         }

         desktop()->nearest(gon);

         if(gon.nearest.elms())
         {
            if(gon.state==2) // if start rectangle is covered
               REPA(gon.nearest)if(!gon.nearest[i].recalcDo(gon))gon.nearest.remove(i); // recalc all

         again:
            if(auto *nearest=gon.findNearest())
            {
               if(nearest->recalc) // if nearest has to be recalculated
               {
                  REPA(gon.nearest){auto &obj=gon.nearest[i]; if(obj.recalc)if(!obj.recalcDo(gon))gon.nearest.remove(i);} // recalc all
                  goto again; // find again
               }
               out_pos=nearest->rect.center();
               return  nearest->obj;
            }
         }
      }
   }
   out_pos=pos;
   return null;
}
void GUI::moveMouse(C Vec2 &dir)C
{
   if(Gui.msLit())
   {
     _List *list;
      if(Gui.msLit()->type()==GO_LIST)
      {
         list=&Gui.msLit()->asList();
      list:
         Vec2 pos=Ms.pos()+list->scrollDelta().chsY();
         if(list->flag&LIST_NEAREST_COLUMN && list->drawMode()==LDM_LIST)
         {
            VecI2 col_vis=list->nearest2(pos, dir); if(col_vis.y>=0)
            {
               list->scrollToCol(col_vis.x).scrollTo(col_vis.y); if(list->scrolling())
               {
                  Ms.pos(list->visToScreenRect(col_vis).center()-list->scrollDelta().chsY());
                  return;
               }
            }
         }else
         {
            int vis=list->nearest(pos, dir); if(vis>=0)
            {
               list->scrollTo(vis); if(list->scrollingMain())
               {
                  Ms.pos(list->visToScreenRect(vis).center()-list->scrollDelta().chsY());
                  return;
               }
            }
         }
      }else
      if(Region *region=Gui.msLit()->firstScrollableRegion())
         if(GuiObj *nearest=region->nearest(Ms.pos()+region->scrollDelta().chsY(), dir))
      {
         if(nearest->type()==GO_LIST)
         {
            list=&nearest->asList();
            goto list;
         }
         region->scrollTo(*nearest); if(region->scrolling())
         {
            Ms.pos(nearest->screenRect().center()-region->scrollDelta().chsY());
            return;
         }
      }
   }
   Vec2 pos; if(GuiObj *nearest=Gui.objNearest(Ms.pos(), dir, pos))Ms.pos(pos);
}
/******************************************************************************/
Color GUI::backgroundColor()C {if(GuiSkin *skin=Gui.skin())return skin->background_color; return         WHITE;}
Color GUI::    borderColor()C {if(GuiSkin *skin=Gui.skin())return skin->    border_color; return Color(0, 112);}
/******************************************************************************/
TextLine* GUI::overlayTextLine(Vec2 &offset)
{
   Rect kb_rect; if(Kb.rect(kb_rect) && kb() && kb()->isTextLine())
   {
      TextLine &tl=kb()->asTextLine();
      Rect_LU   tl_rect(tl.screenPos(), tl.size());
      if(Cuts(tl_rect, kb_rect) || tl_rect.min.y<D.rectUI().min.y)
      {
         // try to move above kb rect first (because when typing with fingers the hands are usually downwards, so they would occlude what's below them)
         if(kb_rect.max.y+tl_rect.h()<=D.rectUI().max.y) // if it fits in the visible screen area
         {
            offset=kb_rect.up()+tl_rect.size()*Vec2(-0.5f, 1.0f)-tl.pos();
         }else // move at the bottom of the screen
         {
            offset=Vec2(0, D.rectUI().min.y)+tl_rect.size()*Vec2(-0.5f, 1.0f)-tl.pos();
         }
         return &tl;
      }
   }
   return null;
}
/******************************************************************************/
void GUI::msgBox(C Str &title, C Str &text, C TextStylePtr &text_style)
{
   SyncLocker locker(QueuedMsgBoxLock);
   REPA(QueuedMsgBoxs)
   {
      QueuedMsgBox &qmb=QueuedMsgBoxs[i];
      if(Equal(qmb.title, title, true) && Equal(qmb.text, text, true) && qmb.text_style==text_style)return; // if already exists then do nothing
   }
   QueuedMsgBoxs.New().set(title, text, text_style);
}
Dialog& GUI::getMsgBox(CPtr id)
{
   SyncLocker locker(_lock);
   if(id)if(MsgBox *mb=MsgBox::Find(id))return *mb;
   return MsgBox::MsgBoxs.New().create(S, S, null, id); // always create to set the 'id' and 'Window.level'
}
// if 'SyncLock' is not safe then crash may occur when trying to lock, to prevent that, check if we have any elements (this means cache was already initialized)
Dialog* GUI::   findMsgBox(CPtr id) {if(id && (SYNC_LOCK_SAFE || MsgBox::MsgBoxs.elms())){SyncLocker locker(_lock); return MsgBox::Find(id);} return null;}
void    GUI::    delMsgBox(CPtr id) {if(id && (SYNC_LOCK_SAFE || MsgBox::MsgBoxs.elms())){SyncLocker locker(_lock); if(MsgBox *mb=MsgBox::Find(id))MsgBox::MsgBoxs.removeData(mb);}}
void    GUI::fadeOutMsgBox(CPtr id) {if(id && (SYNC_LOCK_SAFE || MsgBox::MsgBoxs.elms())){SyncLocker locker(_lock); if(MsgBox *mb=MsgBox::Find(id))mb->fadeOut();}}
void    GUI::  closeMsgBox(CPtr id) {if(id && (SYNC_LOCK_SAFE || MsgBox::MsgBoxs.elms())){SyncLocker locker(_lock); if(MsgBox *mb=MsgBox::Find(id))MsgBox::Close(*mb);}}
/******************************************************************************/
GUI& GUI::passwordChar(Char c) // Warning: this is not thread-safe
{
   if(_pass_char!=c)
   {
     _pass_char=c;
     _pass_temp.clear();
   }
   return T;
}
C Str& GUI::passTemp(Int length) // Warning: this is not thread-safe
{
   if(   length<_pass_temp.length()  )_pass_temp.clip(length);else
   for(; length>_pass_temp.length(); )_pass_temp+=_pass_char;
   return _pass_temp;
}
/******************************************************************************/
static void SetWindowButtons(GuiObj &go)
{
   if (go.isWindow())go.asWindow().setButtons();
   REP(go.childNum())if(GuiObj *child=go.child(i))SetWindowButtons(*child);
}
GUI& GUI::windowButtonsRight(Bool right)
{
   if(_window_buttons_right!=right)
   {
     _window_buttons_right=right;
      REPA(_desktops)SetWindowButtons(_desktops[i]);
   }
   return T;
}
/******************************************************************************/
void GUI::playClickSound()C
{
   SoundPlay(click_sound_id, 1, VOLUME_UI);
}
/******************************************************************************/
void GUI::ms(GuiObj *obj)
{
  _ms=_ms_src=obj;
}
/******************************************************************************/
Vec2 GUI::dragPos()C
{
   if(dragging())
   {
      Touch *touch=FindTouch(_drag_touch_id);
      return touch ? touch->pos() : Ms.pos();
   }
   return 0;
}
void GUI::dragCancel()
{
   if(_drag_cancel){if(dragging())_drag_cancel(_drag_user); _drag_cancel=null;} // call cancel only if actually started dragging
  _drag_want=_dragging=false;
}
void GUI::drag(C Str &name, Touch *touch)
{
   dragCancel();
   if(_drag_want=true) // previously this checked for "if(_drag_want=name.is())" however empty names are now allowed
   {
      if(touch)touch->disableScroll(); // when dragging is initiated, then disable scrolling
     _drag_touch_id=(touch ? touch->id () :        0);
     _drag_pos     =(touch ? touch->pos() : Ms.pos());
     _drag_name    =name;
     _drag_user    =null;
     _drag_start   =null;
     _drag_cancel  =null;
     _drag_finish  =null;
   }
}
void GUI::drag(void finish(Ptr user, GuiObj *obj, C Vec2 &screen_pos), Ptr user, Touch *touch, void start(Ptr user), void cancel(Ptr user))
{
   dragCancel();
   if(_drag_want=true) // previously this checked for "if(_drag_want=(finish!=null))" however empty functions are now allowed
   {
      if(touch)touch->disableScroll(); // when dragging is initiated, then disable scrolling
     _drag_touch_id=(touch ? touch->id () :        0);
     _drag_pos     =(touch ? touch->pos() : Ms.pos());
     _drag_name    .clear();
     _drag_user    =user;
     _drag_start   =start;
     _drag_cancel  =cancel;
     _drag_finish  =finish;
   }
}
void GUI::dragDraw()
{
   if(dragging())if(Image *image=image_drag())image->draw(Rect_D(dragPos(), Vec2(0.05f*image->aspect(), 0.05f)));
}
/******************************************************************************/
void GUI::update()
{
   Dbl t=Time.curTime();
//_time_d_fade_in =Time.ad()*60;
  _time_d_fade_out=Time.ad()*14;
   SyncLocker locker(_lock);

   // test
   auto ovr_tl=overlayTextLine(_overlay_textline_offset); if(ovr_tl!=_overlay_textline) // TODO: we could also check offset "|| (ovr_tl && ovr_tl_ofs!=_overlay_textline_offset)"
   {
     _overlay_textline=ovr_tl;
      updateTextMenu();
   }

  _wheel =null;
  _ms_lit=((Ms.detected() && Ms.visible() && Ms._on_client && desktop()) ? desktop()->test(Ms.pos(), _wheel) : null); // don't set 'msLit' if mouse is not detected or hidden or on another window (do checks if mouse was focused on other window but now moves onto our window, and with buttons pressed in case for drag and drop detection and we would want to highlight the target gui object at which we're gonna drop the files)
   BS_FLAG ms_button=BS_NONE; REPA(Ms._button)ms_button|=Ms._button[i];
   if(!(ms_button&(BS_ON|BS_RELEASED)))_ms=_ms_src=msLit();
   if(App.active())
   {
   #if 1
      if((ms_button&BS_PUSHED) && msLit())msLit()->activate();
   #else
      if(Ms.bp(0) && msLit())msLit()->activate();
   #endif
      REPA(Touches)
      {
       C Touch &touch=Touches[i]; BS_FLAG state=touch._state;
         if(touch.scrolling()) // we might want to scroll
         {
            if(state&(BS_RELEASED|BS_TAPPED|BS_LONG_PRESS)) // potential activation
               if(GuiObj *go=touch.guiObj())
            {
               BS_FLAG mask;
               switch(go->type())
               {
                  case GO_TEXTLINE:
                  case GO_TEXTBOX :
                           mask=BS_TAPPED|BS_LONG_PRESS; break; // activate TextEdits only on Tap/LongPress, in case we just want to Touch-Scroll, because their activation will show soft keyboard
                  default: mask=BS_RELEASED            ; break; // others activate on release. This is important for Buttons to don't show keyboard focus, and especially for Comboboxes so when they're pushed, they don't get unpushed when touch BS_PUSHED (because if Combobox is pushed, and we activate Combobox on BS_PUSHED then any visible Menu disappears, which causes Combobox to get unpushed since its Menu is now hidden)
               }
               if(state&mask)go->activate();
            }
         }else
         {
            if(state&BS_PUSHED)
               if(GuiObj *go=touch.guiObj())
                  go->activate();
         }
      }
      if(Kb.k(KB_TAB))if(Switch())Kb.eatKey();
   }
  _window_lit=&msLit()->first(GO_WINDOW)->asWindow();

   // update
#if 1 // update all
   FREPAO(_desktops).update();
#else // update active
   if(desktop())desktop()->update();
#endif

   // callbacks
  _callbacks.update();

   // add message boxes after update and callbacks, so they're immediately displayed when created
   if(QueuedMsgBoxs.elms())
   {
      SyncLocker locker(QueuedMsgBoxLock);
      FREPA(QueuedMsgBoxs) // process in order
      {
       C QueuedMsgBox &qmb=QueuedMsgBoxs[i];
         REPA(MsgBox::MsgBoxs)
         {
          C MsgBox &mb=MsgBox::MsgBoxs[i];
            if(Equal(mb.title          , qmb.title, true)
            && Equal(mb.text.text      , qmb.text , true)
            &&       mb.text.extra     ==qmb.extra
            &&       mb.text.text_style==qmb.text_style)goto skip; // if already exists then do nothing
         }
         // create new one
         MsgBox::MsgBoxs.New().create(qmb.title, qmb.text, qmb.text_style, null);
      skip:;
      }
      QueuedMsgBoxs.clear();
   }

   // mouse description
   if(_desc!=ms()) // if there is a new object under mouse than previous description
   {
      if(Ms.pixelDelta().allZero())_desc=null;else // set it only if we've moved the mouse (to eliminate showing description by elements activated with keyboard, touch or programatically)
      {
        _desc     =ms();
        _desc_time=Time.appTime();
      }
   }else
   if(Ms.pixelDelta().any() || (ms_button&(BS_ON|BS_PUSHED|BS_RELEASED)))_desc_time=Time.appTime(); // if there was a mouse action then reset the timer

   // touch description
   Touch  *stylus=null; REPA(Touches){Touch &touch=Touches[i]; if(touch.stylus() && !touch.on() && !touch.rs()){stylus=&touch; break;}}
   GuiObj *stylus_obj=(stylus ? stylus->guiObj() : null);
   if(_touch_desc!=stylus_obj) // if there is a new object under stylus than previous description
   {
     _touch_desc     =stylus_obj;
     _touch_desc_time=Time.appTime();
   }

   // drag !! process after objects update, because '_List.update' requires that 'Gui.dragging()' is still active at the moment of dragging release !!
   if(_drag_want)
   {
      Touch *touch=FindTouch(_drag_touch_id);
      if(!_drag_touch_id && Ms.bp(1))dragCancel();else // if mouse dragging and RMB pressed then cancel dragging
      if( _drag_touch_id ? (!touch || touch->rs()) : (!Ms.b(0) || Ms.br(0))) // finish dragging
      {
         if(dragging())
            if(GuiObj *lit=(touch ? objAtPos(touch->pos()) : msLit()))
         {
            if(_drag_name.is() && App.drop){Memc<Str> names; names.add(_drag_name); App.drop( names    , lit, touch ? touch->pos() : Ms.pos());} // don't use 'Memt' here, because that would reduce the stack memory, and in this method all gui object updates are called, so it's best to give them as much as possible, and also this is called only on drag finish, so once in a long time
            if(_drag_finish               )                                     _drag_finish(_drag_user, lit, touch ? touch->pos() : Ms.pos());
           _drag_cancel=null; // clear cancel so we won't call it, because we've already called finish

            if(!menu()) // if menu was not activated during custom function calls
               if(lit=(touch ? objAtPos(touch->pos()) : msLit())) // detect lit again because previous may got deleted during custom function calls
                  lit->activate(); // activate object at which we've drag and dropped
         }
         dragCancel(); // clear drag related members
      }else // start dragging
      if(!dragging() && (touch ? touch->dragging() : Ms.dragging()))
      {
        _dragging=true;
         if(_drag_start)_drag_start(_drag_user);
      }
   }

  _update_time=Time.curTime()-t;
}
/******************************************************************************/
static void DrawDescriptionObj(GuiObj &obj, C Vec2 &pos, Flt start_time, Bool mouse)
{
   Bool   immediate=false;
   CChar *text=obj.desc();
   switch(obj.type())
   {
      case GO_LIST:
      {
        _List &list=obj.asList();
         immediate=FlagOn(list.flag, LIST_IMMEDIATE_DESC);
         if(list._desc_offset>=0)
            if(Ptr data=list.screenToData(pos))text=*(Char**)((Byte*)data+list._desc_offset);
      }break;
   }
   if(Is(text) && (immediate || Time.appTime()-start_time>=Gui.desc_delay) && Gui.draw_description)Gui.draw_description(&obj, pos, text, mouse);
}
/******************************************************************************/
void DrawKeyboardHighlight(GuiObj *obj, C Rect &rect, C GuiSkin *skin)
{
   if(skin && skin->keyboard_highlight_color.a)
      rect.draw(skin->keyboard_highlight_color, false);
}
void DrawDescription(GuiObj *obj, C Vec2 &pos, CChar *text, Bool mouse)
{
   if(Gui.skin)
      if(TextStyle *text_style=Gui.skin->desc.text_style())
         DrawPanelText(Gui.skin->desc.normal(), Gui.skin->desc.normal_color, Gui.skin->desc.padding, *text_style, pos, text, mouse);
}
void DrawIMM(GuiObj *obj)
{
   if((Kb.immBuffer().is() || Kb.immCandidate().elms()) && obj && obj->isTextLine())
      if(Gui.skin)
         if(TextStyle *text_style=Gui.skin->imm.text_style())
   {
      TextLine      &textline=obj->asTextLine();
      TextEdit       edit; edit.cur=Kb.immCursor(); if(Kb.immCandidate().elms())edit.sel=Kb.immSelection().y;
      TextStyleParams ts=*text_style; if(Kb.immBuffer().is())ts.edit=&edit;
      Str              t=Kb.immBuffer(); if(Kb.immCandidate().elms()){if(t.is())t+='\n'; FREPA(Kb.immCandidate())t.space()+=S+(i+1)%10+Kb.immCandidate()[i];} // use %10 so "10" will be displayed as "0" (because pressing 0 button activates that candidate)
      DrawPanelText(Gui.skin->imm.normal(), Gui.skin->imm.normal_color, Gui.skin->imm.padding, ts, textline.screenPos()+Vec2(0, -textline.rect().h()), t, false);
   }
}
/******************************************************************************/
void GUI::draw()
{
   SyncLocker locker(_lock);

   if(desktop())
   {
      desktop()->draw();
      D.clip(); // reset clip after drawing all gui objects

      // show description
      if(draw_description)
      {
         if(      _desc)                                                                                         DrawDescriptionObj(*      _desc, Ms   .pos(),       _desc_time, true );
         if(_touch_desc)REPA(Touches){Touch &touch=Touches[i]; if(touch.stylus() && touch.guiObj()==_touch_desc){DrawDescriptionObj(*_touch_desc, touch.pos(), _touch_desc_time, false); break;}}
      }

      // show imm
      if(draw_imm && kb() && kb()->isTextLine())draw_imm(kb());
   }
}
/******************************************************************************/
void GUI::del()
{
#if 0
   image_shadow   .clear();
   image_drag     .clear();
   image_resize_x .clear();
   image_resize_y .clear();
   image_resize_ld.clear();
   image_resize_lu.clear();
   image_resize_ru.clear();
   image_resize_rd.clear();

   skin.clear();
#endif

   MsgBox::MsgBoxs.del();
  _desktops       .del();
  _callbacks      .del();
   GuiSkins       .del();
   Panels         .del();
   PanelImages    .del();
}
void GUI::create()
{
   if(LogInit)LogN("GUI.create");

   AddEmojiPak(DESKTOP ? "Emoji/BC7.pak" : "Emoji/ETC2.pak"); // !! NEED TO LOAD BEFORE LOADING ANY FONTS !!

   if(D.created()) // needed for APP_ALLOW_NO_GPU/APP_ALLOW_NO_XDISPLAY
      if(skin=default_skin)
   {
      EmptyGuiSkin.list.   cursor_color=skin->list.   cursor_color;
      EmptyGuiSkin.list.highlight_color=skin->list.highlight_color;
      EmptyGuiSkin.list.selection_color=skin->list.selection_color;
   }
   EmptyGuiSkin.region.normal_color.a=0;

   image_shadow   .get("Gui/shadow.img");
   image_drag     .get("Gui/drag.img");
   image_resize_x .get("Gui/resize_x.img");
   image_resize_y .get("Gui/resize_y.img");
   image_resize_ld.get("Gui/resize_ru.img");
   image_resize_lu.get("Gui/resize_rd.img");
   image_resize_ru.get("Gui/resize_ru.img");
   image_resize_rd.get("Gui/resize_rd.img");

  _desktops.New().activate();
}
/******************************************************************************/
void EngineLogo::load()
{
   img="Img/Engine Logo.img";
}
void EngineLogo::draw(Flt alpha)
{
   img.drawFs(ColorAlpha(alpha));
}
/******************************************************************************/
}
/******************************************************************************/
