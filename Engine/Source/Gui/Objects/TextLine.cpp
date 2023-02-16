/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define TEXTLINE_OFFSET 0.16f // set >=0.06 (at this value cursor is aligned with the TextLine rect left edge)
#define TEXTLINE_MARGIN 1.0f
#define ADJUST_OFFSET_ON_SEL 0 // if adjust offset when calling select methods
/******************************************************************************/
// MANAGE
/******************************************************************************/
static void Clear(TextLine &tl) {tl.clear();}
void TextLine::zero()
{
   kb_lit   =true;
   show_find=false;

  _can_select=false;
  _flag      = 0;
  _max_length=-1;
  _offset    = 0;

  _func_immediate=false;
  _func_user     =null;
  _func          =null;
  _edit.reset();
}
TextLine::TextLine() {zero();}
TextLine& TextLine::del()
{
   reset. del();
   hint.  del();
  _text.  del();
  _skin.clear();
   super::del(); zero(); return T;
}
void TextLine::createReset()
{
   reset.create().func(Clear, T).focusable(false).hide()._parent=this;
   reset._sub_type=BUTTON_TYPE_TEXTLINE_CLEAR;
   reset.skin=_skin;
}
TextLine& TextLine::create(C Str &text)
{
   del();

   createReset();
   T._type      =GO_TEXTLINE;
   T._visible   =true;
   T._rect.max.x= 0.40f;
   T._rect.min.y=-0.05f;
   return set(text, QUIET);
}
TextLine& TextLine::create(C TextLine &src)
{
   if(this!=&src)
   {
      if(!src.is())del();else
      {
         copyParams(src);
        _type          =GO_TEXTLINE;
         kb_lit        =src. kb_lit;
         show_find     =src. show_find;
         hint          =src. hint;
        _skin          =src._skin;
        _can_select    =src._can_select;
        _flag          =src._flag;
        _max_length    =src._max_length;
        _offset        =src._offset;
        _func_immediate=src._func_immediate;
        _func_user     =src._func_user;
        _func          =src._func;
        _text          =src._text;
        _edit          =src._edit;
         reset.create(src.reset)._parent=this;
         setTextInput();
         if(Gui.kb()==this)Gui.hideTextMenu();
      }
   }
   return T;
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
  Bool TextLine::showClear  ()C {return T().is();}
  Flt  TextLine::clientWidth()C {Flt w=rect().w(); if(showClear() && reset.visible())w-=reset.rect().w(); return w;}
C Str& TextLine::displayText()C {return password() ? Gui.passTemp(_text.length()) : _text;} // Warning: this is not thread-safe
/******************************************************************************/
void TextLine::setTextInput()C
{
   if(Gui.kb()==this)Kb.resetTextInput();
}
/******************************************************************************/
Bool      TextLine::password(       )C {return             _edit.password   ;                           }
TextLine& TextLine::password(Bool on)  {if(password()!=on){_edit.password=on; setTextInput();} return T;}

Bool      TextLine::number(       )C {return    FlagOn(_flag, NUMBER);}
TextLine& TextLine::number(Bool on)  {if(number()!=on){_flag^=NUMBER ; setTextInput();} return T;}

Bool      TextLine::email(       )C {return   FlagOn(_flag, EMAIL);}
TextLine& TextLine::email(Bool on)  {if(email()!=on){_flag^=EMAIL ; setTextInput();} return T;}

Bool      TextLine::url(       )C {return FlagOn(_flag, URL);}
TextLine& TextLine::url(Bool on)  {if(url()!=on){_flag^=URL ; setTextInput();} return T;}
/******************************************************************************/
TextLine& TextLine::maxLength(Int max_length)
{
   if(   max_length<0)max_length=-1;
   if(T._max_length!= max_length)
   {
      T._max_length=max_length;
      if(max_length>=0 && _text.length()>max_length)
      {
        _text.clip(     max_length);
         MIN(_edit.cur, max_length);
         MIN(_edit.sel, max_length);
         if (_edit.sel==_edit.cur)
         {
           _edit.sel=-1;
            if(Gui.kb()==this)Gui.hideTextMenu();
         }
         call();
         setTextInput();
      }
   }
   return T;
}
/******************************************************************************/
void TextLine::adjustOffset(Bool margin)
{
   if(GuiSkin *skin=getSkin())
      if(TextStyle *text_style=skin->textline.text_style())
   {
      TextStyleParams ts=*text_style; ts.size=rect().h()*skin->textline.text_size;
   #if DEFAULT_FONT_FROM_CUSTOM_SKIN
      if(!ts.font())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
   #endif

      Flt x=ts.textWidth(displayText(), _edit.cur) + _offset + ts.size.x*TEXTLINE_OFFSET, w=clientWidth(), margin_w=(margin ? ts.size.x*TEXTLINE_MARGIN : 0);
      if( x<  margin_w)_offset=Min(_offset-x+w*0.5f, 0);else
      if( x>w-margin_w)_offset=Min(_offset-x+w*0.5f, 0);
   }
}
Bool TextLine::cursorChanged(Int position, Bool margin)
{
   Clamp(position, 0, _text.length()); if(cursor()!=position)
   {
     _edit.cur=position;
      adjustOffset(margin);
      if(Gui.kb()==this)Gui.hideTextMenu();
      return true;
   }
   return false;
}
TextLine& TextLine::cursor(Int position, Bool margin)
{
   if(cursorChanged(position, margin))setTextInput();
   return T;
}
/******************************************************************************/
Bool TextLine::setChanged(C Str &text, SET_MODE mode)
{
   Str t=text; if(_max_length>=0)t.clip(_max_length);
   if(!Equal(T._text, t, true))
   {
      T._text    = t;
      T._edit.sel=-1;
      if(cursor()>t.length())cursorChanged(t.length());

      if(mode!=QUIET)call();
      if(Gui.kb()==this)Gui.hideTextMenu();
      return true;
   }
   return false;
}
TextLine& TextLine::set(C Str &text, SET_MODE mode)
{
   if(setChanged(text, mode))setTextInput();
   return T;
}
TextLine& TextLine::clear(SET_MODE mode) {return set(S, mode);}
/******************************************************************************/
TextLine& TextLine::func(void (*func)(Ptr), Ptr user, Bool immediate)
{
   T._func          =func;
   T._func_user     =user;
   T._func_immediate=immediate;
   return T;
}
void TextLine::call()
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
TextLine& TextLine::selectNone()
{
   if(_edit.sel>=0)
   {
     _edit.sel=-1;
      setTextInput();
      if(Gui.kb()==this)Gui.hideTextMenu();
   }
   return T;
}
TextLine& TextLine::selectAll()
{
   if(_text.is())
      if(_edit.sel!=0 || _edit.cur!=_text.length())
   {
     _edit.sel=0;
   #if ADJUST_OFFSET_ON_SEL
      cursorChanged(_text.length());
   #else
     _edit.cur=_text.length();
   #endif
      setTextInput();
      if(Gui.kb()==this)Gui.hideTextMenu();
   }
   return T;
}
TextLine& TextLine::selectExtNot()
{
         Int dot=TextPosI(_text, '.');
   if(dot<=0)dot=_text.length();
   if(dot> 0)
      if(_edit.sel!=0 || _edit.cur!=dot)
   {
     _edit.sel=0;
   #if ADJUST_OFFSET_ON_SEL
      cursorChanged(dot);
   #else
     _edit.cur=dot;
   #endif
      setTextInput();
      if(Gui.kb()==this)Gui.hideTextMenu();
   }
   return T;
}
/******************************************************************************/
TextLine& TextLine::cut()
{
   if(_edit.sel<0)
   {
      if(!_edit.password)ClipSet(T());
      clear();
   }else
   {
      Int min, max; MinMax(_edit.sel, _edit.cur, min, max);
      Int len=max-min;
      if(!_edit.password)ClipSet(Trim(T(), min, len));
     _text.remove(min, len);
     _edit.sel=-1;
     _edit.cur=min;
      adjustOffset();
      call();
      setTextInput();
      if(Gui.kb()==this)Gui.hideTextMenu();
   }
   return T;
}
TextLine& TextLine::copy()
{
   if(!_edit.password)
   if(_edit.sel<0)
   {
      ClipSet(T());
   }else
   {
      Int min, max; MinMax(_edit.sel, _edit.cur, min, max);
      ClipSet(Trim(T(), min, max-min));
   }
   return T;
}
TextLine& TextLine::paste()
{
   Bool changed=false;
   if(_edit.sel>=0) // del selection
   {
      Int min, max; MinMax(_edit.sel, _edit.cur, min, max);
      Int len=max-min;
     _text.remove(min, len);
     _edit.sel=-1;
     _edit.cur=min;
      changed=true;
   }
   Str paste=ClipGet(); if(paste.is())
   {
      if(_max_length>=0)
      {
         paste.clip(_max_length-_edit.cur); if(!paste.is())goto skip;
        _text .clip(_max_length-paste.length());
      }
     _text.insert(_edit.cur, paste);
     _edit.cur+=paste.length();
      changed=true;
   }
skip:
   if(changed)
   {
      adjustOffset();
      call();
      setTextInput();
      if(Gui.kb()==this)Gui.hideTextMenu();
   }
   return T;
}
/******************************************************************************/
Vec2 TextLine::overlayScreenPos ()C {return (Gui._overlay_textline==this) ? pos ()+Gui._overlay_textline_offset : screenPos ();}
Rect TextLine::overlayScreenRect()C {return (Gui._overlay_textline==this) ? rect()+Gui._overlay_textline_offset : screenRect();}
/******************************************************************************/
TextLine& TextLine::rect(C Rect &rect)
{
 //if(T.rect()!=rect) below looks fast so don't need this
   {
      super::rect(rect);
      reset .rect(Rect(rect.max.x-rect.h(), rect.min.y, rect.max.x, rect.max.y));
   }
   return T;
}
TextLine& TextLine::move(C Vec2 &delta)
{
 //if(delta.any()) looks fast so skip this check
   {
      super::move(delta);
      reset .move(delta);
   }
   return T;
}
/******************************************************************************/
TextLine& TextLine::skin(C GuiSkinPtr &skin, Bool sub_objects)
{
   T._skin=skin;
   if(sub_objects)reset.skin=skin;
   return T;
}
/******************************************************************************/
Flt TextLine::localTextPosX(Int index)C
{
   Flt pos=_offset;
   if(GuiSkin *skin=getSkin())
      if(TextStyle *text_style=skin->textline.text_style())
   {
      TextStyleParams ts=*text_style; ts.size=rect().h()*skin->textline.text_size;
   #if DEFAULT_FONT_FROM_CUSTOM_SKIN
      if(!ts.font())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
   #endif
      pos+=ts.size.x*TEXTLINE_OFFSET;
      if(_text.is() && index>0)
      {
       C Str &text=displayText();
         pos+=ts.textPos(text, index, 0, false).x;
      }
   }
   return pos;
}
Vec2 TextLine::localTextPosX(Int index0, Int index1)C
{
   Flt pos=_offset;
   if(GuiSkin *skin=getSkin())
      if(TextStyle *text_style=skin->textline.text_style())
   {
      TextStyleParams ts=*text_style; ts.size=rect().h()*skin->textline.text_size;
   #if DEFAULT_FONT_FROM_CUSTOM_SKIN
      if(!ts.font())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
   #endif
      pos+=ts.size.x*TEXTLINE_OFFSET;
      if(_text.is())
      {
       C Str &text=displayText();
         return Vec2(pos+ts.textPos(text, index0, 0, false).x,
                     pos+ts.textPos(text, index1, 0, false).x);
      }
   }
   return pos;
}
Rect TextLine::localTextRect(Int index)C
{
   return Rect().setX(localTextPosX(index)).setY(-rect().h(), 0);
}
Rect TextLine::localTextRect(Int index0, Int index1)C
{
   Vec2 lx=localTextPosX(index0, index1); if(lx.x>lx.y)lx.swap();
   return Rect().setX(lx.x, lx.y).setY(-rect().h(), 0);
}
Rect TextLine::localSelRect()C
{
   return (_edit.sel<0) ? localTextRect(           cursor())
                        : localTextRect(_edit.sel, cursor());
}
/******************************************************************************/
// MAIN
/******************************************************************************/
GuiObj* TextLine::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
   if(GuiObj *go=super::test(gpc, pos, mouse_wheel))
   {
      if(showClear())if(GuiObj *go=reset.test(gpc, pos, mouse_wheel))return go;
      return go;
   }
   return null;
}
void TextLine::update(C GuiPC &gpc)
{
   GuiPC gpc_this(gpc, visible() && showClear(), enabled());
   if(   gpc_this.enabled)
   {
      DEBUG_BYTE_LOCK(_used);

      reset.update(gpc_this);
      if(Gui.kb()==this)
      {
      #if !ADJUST_OFFSET_ON_SEL
         Int  sel    =_edit.sel;
      #endif
         Int  cur    =_edit.cur;
         Bool changed= EditText(_text, _edit);
         if(  changed)
         {
            if(_max_length>=0 && _text.length()>_max_length)
            {
              _text.clip(     _max_length);
               MIN(_edit.cur, _max_length);
               MIN(_edit.sel, _max_length);
               if (_edit.sel==_edit.cur)_edit.sel=-1;
            }
            call();
         }
         if(cur!=_edit.cur || changed
      #if !ADJUST_OFFSET_ON_SEL // when offset is not adjusted on selection, then it's possible that cursor is outside of visible space, so when changing selection (clearing it) we should focus on the cursor
         || sel!=_edit.sel
      #endif
         ){adjustOffset(); setTextInput(); Gui.hideTextMenu();}
      }
    C Vec2   *mt_pos=null;
      BS_FLAG mt_state;
      Touch  *touch;
      if(Gui.ms()==this && (Ms._button[0]&(BS_ON|BS_PUSHED))){mt_pos=&Ms.pos(); mt_state=Ms._button[0]; touch=null;}else
      if(Gui.kb()==this)REPA(Touches){Touch &t=Touches[i]; if(t.guiObj()==this && (t.state()&(BS_ON|BS_PUSHED|BS_RELEASED|BS_TAPPED))){mt_pos=&t.pos(); mt_state=t._state; t.disableScroll(); touch=&t; break;}} // check touches only if we already have keyboard focus, so without focus we don't select but instead can scroll
      if(mt_pos)
      {
         if(GuiSkin *skin=getSkin())
            if(TextStyle *text_style=skin->textline.text_style())
         {
            TextStyleParams ts=*text_style; ts.size=rect().h()*skin->textline.text_size;
         #if DEFAULT_FONT_FROM_CUSTOM_SKIN
            if(!ts.font())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
         #endif

          C Str &text=displayText();
            Flt gpc_offset=((Gui._overlay_textline==this) ? Gui._overlay_textline_offset.x : gpc.offset.x);
            Int pos=ts.textIndex(text, mt_pos->x - rect().min.x - gpc_offset - _offset - ts.size.x*TEXTLINE_OFFSET, (ButtonDb(mt_state) || _edit.overwrite) ? TEXT_INDEX_OVERWRITE : TEXT_INDEX_DEFAULT);

            if(ButtonDb(mt_state) && _text.is())
            {
               if(password())selectAll();else
               {
                 _edit.cur=
                 _edit.sel=pos;
                  CHAR_TYPE type=CharType(_text[Min(pos, _text.length()-1)]);
                  for(; _edit.sel                && CharType(_text[_edit.sel-1])==type; _edit.sel--);
                  for(; _edit.cur<_text.length() && CharType(_text[_edit.cur  ])==type; _edit.cur++);
                  if (  _edit.sel==_edit.cur)_edit.sel=-1;
                 _can_select=false;
                  setTextInput();
               }
               if(touch)Gui.showTextMenu();
            }else
            if(_can_select)
            {
               if(mt_state&(BS_PUSHED|BS_TAPPED|BS_LONG_PRESS)) // check BS_TAPPED|BS_LONG_PRESS too, because touches activate TextEdits only on BS_TAPPED|BS_LONG_PRESS (to allow for Touch-Scroll) and we want to set cursor in that case as well
               {
                  if(_edit.cur!=pos || _edit.sel>=0)
                  {
                    _edit.cur=pos;
                    _edit.sel=-1;
                     setTextInput();
                  }
                  if(touch && touch->longPress() && _edit.sel<0) // long press and no selection
                  {
                        DeviceVibrateShort(); // vibrate ASAP so user is notified quickly
                        Gui.showTextMenu();
                  }else Gui.hideTextMenu();
               }else
               if(mt_state&BS_ON)
               {
                  if(pos!=_edit.cur)
                  {
                     if(touch)DeviceVibrateShort(); // vibrate ASAP so user is notified quickly
                     if(_edit.sel<   0)_edit.sel=_edit.cur;else
                     if(_edit.sel==pos)_edit.sel=-1; // we're setting '_edit.cur' to 'pos' below, so if 'sel' is the same then clear it
                                       _edit.cur=pos;
                     setTextInput();
                     Gui.hideTextMenu();
                  }

                  // scroll
                  Flt w=clientWidth(), l=rect().min.x+gpc_offset, r=l+w; // text_rect
                  MAX(l, gpc.clip.min.x); MIN(r, gpc.clip.max.x); // clipped_text_rect
                  if(touch && touch->selecting()) // margin - touches may not reach screen border comfortably, so turn on scrolling with margin for them, but only after some movement to prevent instant scroll at start
                  {
                     Flt margin=ts.size.x;
                     MAX(l, D.rectUI().min.x+margin);
                     MIN(r, D.rectUI().max.x-margin);
                  }
                  // check <= instead of < in case we're at screen border
                  if(mt_pos->x<=l)_offset=Min(0,     Time.d()* 2+_offset                          );else
                  if(mt_pos->x>=r)_offset=Min(0, Max(Time.d()*-2+_offset, -ts.textWidth(text)+w/2));
               }else
               if(mt_state&BS_RELEASED) // released
               {
                  if(touch && _edit.sel>=0)Gui.showTextMenu(); // by touch and have some selection
               }
            }
         }
      }else _can_select=true;
   }
}
void TextLine::draw(C GuiPC &gpc)
{
   if(/*gpc.visible &&*/ visible())
      if(GuiSkin *skin=getSkin())
   {
      Bool        enabled=(T.enabled() && gpc.enabled);
      Rect        rect=T.rect()+gpc.offset, ext_rect;
    C PanelImage *panel_image             ; // may be null
    C Color      *panel_color, *text_color; // never  null
      if(enabled){panel_image=skin->textline.normal  (); panel_color=&skin->textline.  normal_panel_color; text_color=&skin->textline.  normal_text_color;}
      else       {panel_image=skin->textline.disabled(); panel_color=&skin->textline.disabled_panel_color; text_color=&skin->textline.disabled_text_color;}
      if(panel_image)panel_image->extendedRect(rect, ext_rect);else ext_rect=rect;
      if(Cuts(ext_rect, gpc.clip))
      {
         D.clip(gpc.clip);
         if(panel_image                )panel_image->draw(*panel_color, TRANSPARENT, rect);else
         if(panel_color->a             )rect        .draw(*panel_color             , true );
         if(skin->textline.rect_color.a)rect        .draw(skin->textline.rect_color, false);

         // draw text
         if(TextStyle *text_style=skin->textline.text_style())
         {
            Bool active=(Gui.kb()==this && enabled && ((Gui._overlay_textline==this) ? Equal(Gui._overlay_textline_offset, gpc.offset) : true)); // if this is the overlay textline, then draw cursor and editing only if it matches the overlay offset
            if(T().is() || active || show_find || hint.is())
            {
               Rect clip_rect=rect; clip_rect.max.x=clip_rect.min.x+clientWidth(); D.clip(clip_rect&gpc.clip);

               TextStyleParams ts=*text_style; ts.size=rect.h()*skin->textline.text_size; ts.align.set(1, 0); ts.color=ColorMul(ts.color, *text_color);
            #if DEFAULT_FONT_FROM_CUSTOM_SKIN
               if(!ts.font())ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
            #endif

               Flt x=rect.min.x + ts.size.x*TEXTLINE_OFFSET, y=rect.centerY();

               if(T().is() || active)
               {
                  if(active)ts.edit=&_edit;
                  D.text(ts, x+_offset, y, displayText());
               }else
               {
                  if(show_find)if(Image *image=skin->textline.find_image())
                  {
                     x=rect.min.x+image->aspect()*rect.h(); // set new x as the right side of image, so that potential 'hint' will be drawn to the right
                     image->draw(Rect(rect.min.x, rect.min.y, x, rect.max.y)); // draw on the left
                  }
                  if(hint.is())
                  {
                     ts.color.a=((ts.color.a*96)>>8); ts.size*=0.85f; D.text(ts, x, y, hint);
                  }
               }
            }
         }

         if(showClear())reset.draw(gpc);
         if(kb_lit && Gui.kb()==this){D.clip(gpc.clip); Gui.kbLit(this, rect, skin);}
      }
   }
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool TextLine::save(File &f, CChar *path)C
{
   if(super::save(f, path))
   {
      f.cmpUIntV(7); // version
      f<<kb_lit<<show_find<<_edit.password<<_max_length<<hint<<_text;
      f.putBool(reset.visible());
      f._putAsset(_skin.name(path));
      return f.ok();
   }
   return false;
}
Bool TextLine::load(File &f, CChar *path)
{
   del();
   createReset();
   if(super::load(f, path))switch(f.decUIntV()) // version
   {
      case 7:
      {
        _type=GO_TEXTLINE;

         f>>kb_lit>>show_find>>_edit.password>>_max_length>>hint>>_text;
         reset.visible(f.getBool());
        _skin.require(f._getAsset(), path);
         if(f.ok())return true;
      }break;

      case 6:
      {
        _type=GO_TEXTLINE;

         Bool kb_catch; f>>kb_catch>>kb_lit>>show_find>>_edit.password>>_max_length; f._getStr1(hint)._getStr1(_text);
         reset.visible(f.getBool());
        _skin.require(f._getAsset(), path);
         if(f.ok())return true;
      }break;

      case 5:
      {
        _type=GO_TEXTLINE;

         f>>kb_lit; f.skip(8); f>>_edit.password>>_max_length; f._getStr1(_text); f._getStr1();
         if(f.ok())return true;
      }break;

      case 4:
      {
        _type=GO_TEXTLINE;

         f>>kb_lit; f.skip(8); f>>_edit.password>>_max_length; f._getStr(_text);
         if(f.ok())return true;
      }break;

      case 3:
      {
        _type=GO_TEXTLINE;

         f>>kb_lit; f.skip(8); f>>_edit.password>>_max_length; f._getStr(_text);
         if(f.ok())return true;
      }break;

      case 2:
      {
        _type=GO_TEXTLINE;

         f>>kb_lit; f.skip(8); f>>_edit.password; f._getStr(_text);
         if(f.ok())return true;
      }break;

      case 1:
      {
        _type=GO_TEXTLINE;

         f>>kb_lit; f.skip(8); f>>_offset; _text=f._getStr16();
         if(f.ok())return true;
      }break;

      case 0:
      {
        _type=GO_TEXTLINE;

         f>>kb_lit; f.skip(8); f>>_offset; _text=f._getStr8();
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
