/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
#define TEXTBOX_OFFSET 0.16f // set >=0.06 (at this value cursor is aligned with the TextBox rect left edge) this value is applied to the left and right of the text to add some margin
#define TEXTBOX_MARGIN 2.5f

#define TEXTBOX_MARGIN_REL 0.0f
#define TEXTBOX_MARGIN_ABS 0.01f
/******************************************************************************/
void TextBox::zero() {
    kb_lit = true;
    min_height = 0.3f;
    _slidebar_size = 0.05f;
    _text_space = 0; // this parameter is used to make sure that all text functions use exactly the same text width. Important to keep in sync - multi-line drawing, clicking, editing, so that cursor position matches graphics.

    _can_select = false;
    _word_wrap = true;
    _auto_height = false;
    _max_length = -1;

    _func_immediate = false;
    _func_user = null;
    _func = null;

    _edit.reset();
    _crect.zero();
}
TextBox::TextBox() { zero(); }
TextBox &TextBox::del() {
    slidebar[0].del();
    slidebar[1].del();
    view.del();
    hint.del();
    _text.del();
    _skin.clear();
    super::del();
    zero();
    return T;
}
void TextBox::setParent() {
    slidebar[0]._parent = slidebar[1]._parent = view._parent = this;
}
void TextBox::setParams() {
    _type = GO_TEXTBOX;
    view._sub_type = BUTTON_TYPE_REGION_VIEW;
    setParent();
}
TextBox &TextBox::create(C Str &text) {
    del();

    _visible = true;
    _rect.max.x = 0.3f;
    _rect.min.y = -0.3f;

    view.create().hide().mode = BUTTON_CONTINUOUS;
    slidebar[0].create().setLengths(0, 0).hide();
    slidebar[1].create().setLengths(0, 0).hide();
    view._focusable = slidebar[0]._focusable = slidebar[1]._focusable = false;
    setParams();

    return set(text, QUIET);
}
TextBox &TextBox::create(C TextBox &src) {
    if (this != &src) {
        if (!src.is())
            del();
        else {
            copyParams(src);
            _type = GO_TEXTBOX;
            kb_lit = src.kb_lit;
            min_height = src.min_height;
            hint = src.hint;
            _slidebar_size = src._slidebar_size;
            _text_space = src._text_space;
            _can_select = src._can_select;
            _word_wrap = src._word_wrap;
            _auto_height = src._auto_height;
            _max_length = src._max_length;
            _func_immediate = src._func_immediate;
            _func_user = src._func_user;
            _func = src._func;
            _crect = src._crect;
            _skin = src._skin;
            _text = src._text;
            _edit = src._edit;
            view.create(src.view);
            slidebar[0].create(src.slidebar[0]);
            slidebar[1].create(src.slidebar[1]);
            setParent();
            setTextInput();
            if (Gui.kb() == this)
                Gui.hideTextMenu();
        }
    }
    return T;
}
/******************************************************************************/
void TextBox::setTextInput() C {
    if (Gui.kb() == this)
        Kb.resetTextInput();
}
TextBox &TextBox::slidebarSize(Flt size) {
    MAX(size, 0);
    if (_slidebar_size != size) {
        _slidebar_size = size;
        if (wordWrap())
            setVirtualSize();
        else
            setRectAndButtons(); // in 'wordWrap' mode, virtual size is dependent on the slidebar size
    }
    return T;
}
void TextBox::setVirtualSize(C Rect *set_rect) {
    Flt padd_h = 0;
    Vec2 size = 0;
    Rect rect = (set_rect ? *set_rect : T.rect()), crect = rect;
    GuiSkin *skin = getSkin();
    if (skin)
        if (Panel *panel = skin->region.normal()) {
            Rect padd;
            panel->innerPadding(crect, padd);
            crect.min += padd.min;
            crect.max -= padd.max;
            padd_h = padd.min.y + padd.max.y;
        }
    _text_space = crect.w();
    if (skin)
        if (TextStyle *text_style = skin->textline.text_style()) {
#if DEFAULT_FONT_FROM_CUSTOM_SKIN
            TextStyleParams ts = *text_style;
            if (!ts.font())
                ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
#else
            C TextStyle &ts = *text_style;
#endif

            Flt offset2 = ts.size.x * (TEXTBOX_OFFSET * 2);
            size.y = ts.textHeight(T(), _text_space -= offset2, wordWrap(), &size.x); // decrease available space for text by offset for both sides
            if (_auto_height) {
                Flt client_height = Max(size.y, min_height);
                rect.min.y = rect.max.y - client_height - padd_h;
            } else if (wordWrap()) // check if in word wrap we're exceeding lines beyond client rectangle, in that case slidebar has to be made visible, which reduces client width
            {
                Flt client_height = crect.h();    // here can't use 'clientHeight' because it may not be available yet
                if (size.y > client_height + EPS) // exceeds client height
                {                                 // recalculate using smaller width
                    size.y = ts.textHeight(T(), _text_space -= slidebarSize(), wordWrap(), &size.x);
                }
            }
            size.x += offset2; // we've calculated text widths with offset removed, but here we're calculating virtual size, and it needs to include it
        }
    if (_auto_height)
        super::rect(rect);
    slidebar[0]._length_total = size.x;
    slidebar[1]._length_total = size.y;
    setRectAndButtons();
}
/******************************************************************************/
TextBox &TextBox::maxLength(Int max_length) {
    if (max_length < 0)
        max_length = -1;
    if (T._max_length != max_length) {
        T._max_length = max_length;
        if (max_length >= 0 && _text.length() > max_length) {
            _text.clip(max_length);
            MIN(_edit.cur, max_length);
            MIN(_edit.sel, max_length);
            if (_edit.sel == _edit.cur) {
                _edit.sel = -1;
                if (Gui.kb() == this)
                    Gui.hideTextMenu();
            }
            setVirtualSize();
            call();
            setTextInput();
        }
    }
    return T;
}
/******************************************************************************/
TextBox &TextBox::scrollToCursor(Bool margin) {
    if (GuiSkin *skin = getSkin())
        if (TextStyle *text_style = skin->textline.text_style()) {
#if DEFAULT_FONT_FROM_CUSTOM_SKIN
            TextStyleParams ts = *text_style;
            if (!ts.font())
                ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
#else
            C TextStyle &ts = *text_style;
#endif

            Flt offset = ts.size.x * TEXTBOX_OFFSET;
            Vec2 pos = ts.textPos(T(), cursor(), _text_space, wordWrap()); // here Y is 0..Inf
            pos.x += offset;
            Flt pos_left = pos.x,
                pos_right = pos.x,
                pos_bottom = pos.y + ts.size.y; // bottom position of the cursor (add because Y is inverted)
            if (margin) {
                Flt margin = ts.size.x * TEXTBOX_MARGIN;
                pos_left -= margin;
                pos_right += margin;

                margin = ts.size.y * TEXTBOX_MARGIN_REL + TEXTBOX_MARGIN_ABS; // use vertical to see if we're at textbox end
                pos.y -= margin;
                pos_bottom += margin;
            }
            Flt bottom = pos_bottom;
            Rect kb_rect;
            if (Kb.rect(kb_rect)) { // TODO: this assumes screen keyboard is at the bottom
                Rect screen_rect = screenRect();
                Flt h = kb_rect.max.y - screen_rect.min.y;
                if (h > 0)
                    bottom += h;
            }
            if (pos_left < slidebar[0].offset() || pos_right > clientWidth() + slidebar[0].offset())
                scrollFitX(pos_left, pos_right, true);
            if (pos.y < slidebar[1].offset() || bottom > clientHeight() + slidebar[1].offset())
                scrollFitY(pos.y, bottom, true);

            if (GuiObj *parent = T.parent())
                if (parent->isRegion()) // scroll nearest parent too, in case this TextBox is located within a Region
                {
                    Region &region = parent->asRegion();
                    Vec2 ofs(-slidebar[0].wantedOffset(), -slidebar[1].wantedOffset());
                    ofs.x += clientRect().min.x;
                    ofs.y -= clientRect().max.y;
                    if (Kb.visible()) { // TODO: this assumes screen keyboard is at the bottom
                        Rect screen_rect = region.screenRect();
                        Flt h = kb_rect.max.y - screen_rect.min.y;
                        if (h > 0)
                            pos_bottom += h;
                    }
                    region.scrollFitX(pos_left + ofs.x, pos_right + ofs.x, true);
                    region.scrollFitY(pos.y + ofs.y, pos_bottom + ofs.y, true);
                }
        }
    return T;
}
Bool TextBox::cursorChanged(Int position, Bool margin) {
    Clamp(position, 0, _text.length());
    if (cursor() != position) {
        _edit.cur = position;
        scrollToCursor(margin);
        if (Gui.kb() == this)
            Gui.hideTextMenu();
        return true;
    }
    return false;
}
TextBox &TextBox::cursor(Int position, Bool margin) {
    if (cursorChanged(position, margin))
        setTextInput();
    return T;
}
/******************************************************************************/
Bool TextBox::setChanged(C Str &text, SET_MODE mode) {
    Str t = text;
    if (_max_length >= 0)
        t.clip(_max_length);
    if (!Equal(T._text, t, true)) {
        T._text = t;
        T._edit.sel = -1;
        setVirtualSize();
        if (cursor() > t.length())
            cursorChanged(t.length());

        if (mode != QUIET)
            call();
        if (Gui.kb() == this)
            Gui.hideTextMenu();
        return true;
    }
    return false;
}
TextBox &TextBox::set(C Str &text, SET_MODE mode) {
    if (setChanged(text, mode))
        setTextInput();
    return T;
}
TextBox &TextBox::clear(SET_MODE mode) { return set(S, mode); }
/******************************************************************************/
TextBox &TextBox::func(void (*func)(Ptr), Ptr user, Bool immediate) {
    T._func = func;
    T._func_user = user;
    T._func_immediate = immediate;
    return T;
}
void TextBox::call() {
    if (_func) {
        if (_func_immediate) {
            DEBUG_BYTE_LOCK(_used);
            _func(_func_user);
        } else
            Gui.addFuncCall(_func, _func_user);
    }
}
/******************************************************************************/
TextBox &TextBox::selectNone() {
    if (_edit.sel >= 0) {
        _edit.sel = -1;
        setTextInput();
        if (Gui.kb() == this)
            Gui.hideTextMenu();
    }
    return T;
}
TextBox &TextBox::selectAll() {
    if (_text.is())
        if (_edit.sel != 0 || _edit.cur != _text.length()) {
            _edit.sel = 0;
            _edit.cur = _text.length();
            setTextInput();
            if (Gui.kb() == this)
                Gui.hideTextMenu();
        }
    return T;
}
/******************************************************************************/
TextBox &TextBox::cut() {
    if (_edit.sel < 0) {
        if (!_edit.password)
            ClipSet(T());
        clear();
    } else {
        Int min, max;
        MinMax(_edit.sel, _edit.cur, min, max);
        Int len = max - min;
        if (!_edit.password)
            ClipSet(Trim(T(), min, len));
        _text.remove(min, len);
        _edit.sel = -1;
        _edit.cur = min;
        scrollToCursor();
        call();
        setTextInput();
        if (Gui.kb() == this)
            Gui.hideTextMenu();
    }
    return T;
}
TextBox &TextBox::copy() {
    if (!_edit.password)
        if (_edit.sel < 0) {
            ClipSet(T());
        } else {
            Int min, max;
            MinMax(_edit.sel, _edit.cur, min, max);
            ClipSet(Trim(T(), min, max - min));
        }
    return T;
}
TextBox &TextBox::paste() {
    Bool changed = false;
    if (_edit.sel >= 0) // del selection
    {
        Int min, max;
        MinMax(_edit.sel, _edit.cur, min, max);
        Int len = max - min;
        _text.remove(min, len);
        _edit.sel = -1;
        _edit.cur = min;
        changed = true;
    }
    Str paste = ClipGet();
    if (paste.is()) {
        if (_max_length >= 0) {
            paste.clip(_max_length - _edit.cur);
            if (!paste.is())
                goto skip;
            _text.clip(_max_length - paste.length());
        }
        _text.insert(_edit.cur, paste);
        _edit.cur += paste.length();
        changed = true;
    }
skip:
    if (changed) {
        scrollToCursor();
        call();
        setTextInput();
        if (Gui.kb() == this)
            Gui.hideTextMenu();
    }
    return T;
}
/******************************************************************************/
void TextBox::setRectAndButtons() {
    // first calculate client rectangle
    _crect = rect();
    if (GuiSkin *skin = getSkin())
        if (Panel *panel = skin->region.normal()) {
            Rect padd;
            panel->innerPadding(_crect, padd);
            _crect.min += padd.min;
            _crect.max -= padd.max;
        }

    Bool vertical, horizontal;
    Flt width = virtualWidth(),
        height = virtualHeight();

    Rect srect = _crect, mrect = _crect;
    if (vertical = (slidebar[1].is() && clientHeight() + EPS < height)) {
        srect.max.x -= slidebarSize();
        _crect.max.x -= slidebarSize();
    }
    if (horizontal = (slidebar[0].is() && clientWidth() + EPS < width)) {
        srect.min.y += slidebarSize();
        if (!wordWrap())
            _crect.min.y += slidebarSize();
        if (!vertical && slidebar[1].is() && clientHeight() + EPS < height) {
            srect.max.x -= slidebarSize();
            _crect.max.x -= slidebarSize();
            vertical = true;
        }
    }

    slidebar[0].setLengths(clientWidth(), width).rect(Rect(mrect.min.x, mrect.min.y, srect.max.x, mrect.min.y + slidebarSize()));
    slidebar[0].visible(slidebar[0]._usable && !wordWrap());
    slidebar[1].setLengths(clientHeight(), height).rect(Rect(mrect.max.x - slidebarSize(), srect.min.y, mrect.max.x, mrect.max.y));
    slidebar[1].visible(slidebar[1]._usable);
    view.rect(Rect(mrect.max.x - slidebarSize(), mrect.min.y, mrect.max.x, mrect.min.y + slidebarSize())).visible(slidebar[0]._usable && slidebar[1]._usable);
}
TextBox &TextBox::wordWrap(Bool wrap) {
    if (_word_wrap != wrap) {
        _word_wrap = wrap;
        setVirtualSize();
    }
    return T;
}
TextBox &TextBox::autoHeight(Bool auto_height) {
    if (_auto_height != auto_height)
        if (_auto_height = auto_height)
            setVirtualSize();
    return T;
}
/******************************************************************************/
TextBox &TextBox::rect(C Rect &rect) {
    if (T.rect() != rect) {
        if (_auto_height)
            setVirtualSize(&rect);
        else // for '_auto_height' need to adjust height in 'setVirtualSize', it will call 'super::rect'
        {
            super::rect(rect);
            if (wordWrap())
                setVirtualSize();
            else
                setRectAndButtons(); // in 'wordWrap' mode, virtual size is dependent on the rectangle
        }
    }
    return T;
}
TextBox &TextBox::move(C Vec2 &delta) {
    if (delta.any()) {
        super::move(delta);
        _crect += delta;
        view.move(delta);
        slidebar[0].move(delta);
        slidebar[1].move(delta);
    }
    return T;
}
/******************************************************************************/
TextBox &TextBox::skin(C GuiSkinPtr &skin, Bool sub_objects) {
    if (sub_objects) {
        view.skin = skin;
        REPAO(slidebar).skin(skin);
    }
    if (_skin != skin) {
        _skin = skin;
        setVirtualSize(); // new skin can have different text style and inner padding, so have to recalculate
    }
    return T;
}
/******************************************************************************/
Rect TextBox::localTextRect(Int index) C {
    Vec2 pos(-slidebar[0].offset(), slidebar[1].offset());
    if (GuiSkin *skin = getSkin())
        if (TextStyle *text_style = skin->textline.text_style()) {
#if DEFAULT_FONT_FROM_CUSTOM_SKIN
            TextStyleParams ts = *text_style;
            if (!ts.font())
                ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
#else
            C TextStyle &ts = *text_style;
#endif
            Vec2 p = ts.textPos(T(), index, _text_space, wordWrap()); // here Y is 0..Inf
            pos.x += p.x + ts.size.x * TEXTBOX_OFFSET;
            pos.y -= p.y;
            return Rect().setX(pos.x).setY(pos.y - ts.size.y, pos.y);
        }
    return pos;
}
Rect TextBox::localTextRect(Int index0, Int index1) C {
    Vec2 pos(-slidebar[0].offset(), slidebar[1].offset());
    if (GuiSkin *skin = getSkin())
        if (TextStyle *text_style = skin->textline.text_style()) {
#if DEFAULT_FONT_FROM_CUSTOM_SKIN
            TextStyleParams ts = *text_style;
            if (!ts.font())
                ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
#else
            C TextStyle &ts = *text_style;
#endif
            pos.x += ts.size.x * TEXTBOX_OFFSET;
            Vec2 p0 = ts.textPos(T(), index0, _text_space, wordWrap());
            p0.chsY(); // here Y is 0..Inf
            Vec2 p1 = ts.textPos(T(), index1, _text_space, wordWrap());
            p1.chsY(); // here Y is 0..Inf
            if (!Equal(p0.y, p1.y)) {
                p0.x = 0;
                p1.x = virtualWidth();
            } // if Y are different, then they're on multiple lines, so extend X to full space
            Rect r;
            r.from(p0, p1) += pos;
            r.min.y -= ts.size.y;
            return r;
        }
    return pos;
}
Rect TextBox::localSelRect() C {
    return (_edit.sel < 0) ? localTextRect(cursor())
                           : localTextRect(_edit.sel, cursor());
}
/******************************************************************************/
GuiObj *TextBox::test(C GuiPC &gpc, C Vec2 &pos, GuiObj *&mouse_wheel) {
    if (GuiObj *go = super::test(gpc, pos, mouse_wheel)) {
        Bool priority = !Kb.shift(); // when 'Kb.shift' disabled (default) then priority=1 (vertical), when enabled then priority=0 (horizontal)
        if (slidebar[priority]._usable)
            mouse_wheel = &slidebar[priority];
        else // check  priority slidebar first
            if (slidebar[!priority]._usable)
                mouse_wheel = &slidebar[!priority]; // check !priority slidebar next

        GuiPC gpc_this(gpc, visible(), enabled());
        if (GuiObj *go = slidebar[0].test(gpc_this, pos, mouse_wheel))
            return go;
        if (GuiObj *go = slidebar[1].test(gpc_this, pos, mouse_wheel))
            return go;
        if (GuiObj *go = view.test(gpc_this, pos, mouse_wheel))
            return go;

        return go;
    }
    return null;
}
/******************************************************************************/
void TextBox::moveCursor(Int lines, Int pages) {
    if (GuiSkin *skin = getSkin())
        if (TextStyle *text_style = skin->textline.text_style()) {
#if DEFAULT_FONT_FROM_CUSTOM_SKIN
            TextStyleParams ts = *text_style;
            if (!ts.font())
                ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
#else
            C TextStyle &ts = *text_style;
#endif

            Bool auto_line = wordWrap();
            if (!Kb.k.shift())
                _edit.sel = -1;
            else if (_edit.sel < 0)
                _edit.sel = _edit.cur;
            if (pages) {
                Int page_lines = Trunc(clientHeight() / ts.lineHeight());
                lines += pages * Max(1, page_lines); // always move at least one line
            }
            Vec2 pos = ts.textPos(T(), cursor(), _text_space, auto_line);
            pos.y += ts.lineHeight() * (lines + 0.5f); // add 0.5 to get rounding
            Bool eol;
            _edit.cur = ts.textIndex(T(), pos.x, pos.y, TEXT_INDEX_DEFAULT, _text_space, auto_line, eol); // TODO: this could be "_edit.overwrite ? TEXT_INDEX_OVERWRITE : TEXT_INDEX_DEFAULT" but for that case we would have to adjust 'pos.x' based on char width
        }
}
/******************************************************************************/
void ScrollMinus(SlideBar *sb, GuiObj *go, Bool vertical) {
    if (sb->offset() > 0) {
        sb->scrollLeftUp();
        return;
    }
    for (; go; go = go->parent())
        if (go->isRegion()) {
            sb = &go->asRegion().slidebar[vertical];
            if (sb->offset() > 0) {
                sb->scrollLeftUp();
                return;
            }
        }
}
void ScrollPlus(SlideBar *sb, GuiObj *go, Bool vertical) {
    if (!sb->atEnd()) {
        sb->scrollRightDown();
        return;
    }
    for (; go; go = go->parent())
        if (go->isRegion()) {
            sb = &go->asRegion().slidebar[vertical];
            if (!sb->atEnd()) {
                sb->scrollRightDown();
                return;
            }
        }
}
/******************************************************************************/
void TextBox::update(C GuiPC &gpc) {
    GuiPC gpc_this(gpc, visible(), enabled());
    if (gpc_this.enabled) {
        DEBUG_BYTE_LOCK(_used);

        view.update(gpc_this);
        if (view()) {
            if (Gui.ms() == &view) {
                Ms.freeze();
                slidebar[0].offset(slidebar[0]._offset + Ms.d().x * 2);
                slidebar[1].offset(slidebar[1]._offset - Ms.d().y * 2);
            }
            REPA(Touches) {
                Touch &t = Touches[i];
                if (t.guiObj() == &view && t.on()) {
                    slidebar[0].offset(slidebar[0]._offset + t.d().x * 2);
                    slidebar[1].offset(slidebar[1]._offset - t.d().y * 2);
                }
            }
        }

        // scroll horizontally
        if (Ms.wheelX() && (Gui.wheel() == &slidebar[0] || Gui.wheel() == &slidebar[1]) // if has focus on any of slidebars
            && slidebar[0]._usable)                                                     // we will scroll only horizontally, so check if that's possible
            slidebar[0].scroll(Ms.wheelX() * (slidebar[0]._scroll_mul * slidebar[0].length() + slidebar[0]._scroll_add), slidebar[0]._scroll_immediate);

        slidebar[0].update(gpc_this);
        slidebar[1].update(gpc_this);

        // update text here
        if (Gui.kb() == this) {
            Int cur = _edit.cur;
            Bool changed = EditText(_text, _edit, true);
            if (changed) {
                if (_max_length >= 0 && _text.length() > _max_length) {
                    _text.clip(_max_length);
                    MIN(_edit.cur, _max_length);
                    MIN(_edit.sel, _max_length);
                    if (_edit.sel == _edit.cur)
                        _edit.sel = -1;
                }
                setVirtualSize();
                call();
            }
            if (Kb.k(KB_UP)) {
                moveCursor(-1, 0);
                Kb.eatKey();
            }
            if (Kb.k(KB_DOWN)) {
                moveCursor(1, 0);
                Kb.eatKey();
            }
            if (Kb.k(KB_PGUP)) {
                moveCursor(0, -1);
                Kb.eatKey();
            }
            if (Kb.k(KB_PGDN)) {
                moveCursor(0, 1);
                Kb.eatKey();
            }
            if (cur != _edit.cur || changed) {
                cur = _edit.cur;
                _edit.cur = -1;
                cursor(cur);
            } // set -1 to force adjustment of offset and calling 'setTextInput', 'Gui.hideTextMenu'
        }
        C Vec2 *mt_pos = null;
        BS_FLAG mt_state;
        Touch *touch;
        if (Gui.ms() == this && (Ms._button[0] & (BS_ON | BS_PUSHED))) {
            mt_pos = &Ms.pos();
            mt_state = Ms._button[0];
            touch = null;
        } else if (Gui.kb() == this)
            REPA(Touches) {
                Touch &t = Touches[i];
                if (t.guiObj() == this && (t.state() & (BS_ON | BS_PUSHED | BS_RELEASED | BS_TAPPED))) {
                    mt_pos = &t.pos();
                    mt_state = t._state;
                    t.disableScroll();
                    touch = &t;
                    break;
                }
            } // check touches only if we already have keyboard focus, so without focus we don't select but instead can scroll
        if (mt_pos) {
            if (GuiSkin *skin = getSkin())
                if (TextStyle *text_style = skin->textline.text_style()) {
#if DEFAULT_FONT_FROM_CUSTOM_SKIN
                    TextStyleParams ts = *text_style;
                    if (!ts.font())
                        ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
#else
                    C TextStyle &ts = *text_style;
#endif

                    Flt offset = ts.size.x * TEXTBOX_OFFSET;
                    Rect clip = gpc.clip;
                    Rect kb_rect;
                    if (Kb.rect(kb_rect))
                        MAX(clip.min.y, kb_rect.max.y); // TODO: this assumes screen keyboard is at the bottom
                    Rect text_rect = clientRect() + gpc.offset, clipped_text_rect = text_rect & clip;
                    Vec2 clipped_pos = *mt_pos & clipped_text_rect; // have to clip so after we start selecting and move mouse outside the client rectangle, we don't set cursor to be outside, instead start smooth scrolling when mouse is outside towards cursor, but limit the cursor position within visible area
                    Bool eol;
                    Int pos = ts.textIndex(T(), clipped_pos.x - text_rect.min.x + slidebar[0].offset() - offset, text_rect.max.y - clipped_pos.y + slidebar[1].offset(), (ButtonDb(mt_state) || _edit.overwrite) ? TEXT_INDEX_OVERWRITE : TEXT_INDEX_DEFAULT, _text_space, wordWrap(), eol);

                    if (ButtonDb(mt_state) && _text.is()) {
                        if (eol && pos && _text[pos - 1] != '\n')
                            pos--; // if double-clicked at the end of the line and previous character isn't a new line (have to check for this because if we don't then we would go to previous line when clicking on empty lines), then go back, have to check 'eol' and not "_text[pos]=='\n'" because this line could be split because of too long text (in this case there will be 'eol' but no '\n')
                        Char c = _text[Min(pos, _text.length() - 1)];
                        if (c && c != '\n') // if not end and not new line
                        {
                            _edit.cur = _edit.sel = pos;
                            CHAR_TYPE type = CharType(c);
                            for (; _edit.sel && CharType(_text[_edit.sel - 1]) == type; _edit.sel--)
                                ;
                            for (; _edit.cur < _text.length() && CharType(_text[_edit.cur]) == type; _edit.cur++)
                                ;
                            if (_edit.sel == _edit.cur)
                                _edit.sel = -1;
                        } else {
                            _edit.cur = pos;
                            _edit.sel = -1; // when double-clicking on an empty line, just leave the cursor in its position
                        }
                        _can_select = false;
                        setTextInput();
                        if (touch)
                            Gui.showTextMenu();
                    } else if (_can_select) {
                        if (mt_state & (BS_PUSHED | BS_TAPPED | BS_LONG_PRESS)) // check BS_TAPPED|BS_LONG_PRESS too, because touches activate TextEdits only on BS_TAPPED|BS_LONG_PRESS (to allow for Touch-Scroll) and we want to set cursor in that case as well
                        {
                            if (_edit.cur != pos || _edit.sel >= 0) {
                                _edit.cur = _edit.sel = -1;
                                cursor(pos, false); // set -1 to force adjustment of offset and calling 'setTextInput'
                            }
                            if (touch && touch->longPress() && _edit.sel < 0) // long press and no selection
                            {
                                DeviceVibrateShort(); // vibrate ASAP so user is notified quickly
                                Gui.showTextMenu();
                            } else
                                Gui.hideTextMenu();
                        } else if (mt_state & BS_ON) {
                            if (pos != _edit.cur) {
                                if (touch)
                                    DeviceVibrateShort(); // vibrate ASAP so user is notified quickly
                                if (_edit.sel < 0)
                                    _edit.sel = _edit.cur;
                                else if (_edit.sel == pos)
                                    _edit.sel = -1; // we're setting '_edit.cur' to 'pos' below, so if 'sel' is the same then clear it
                                _edit.cur = pos;
                                setTextInput();
                                Gui.hideTextMenu();
                            }

                            // scroll
                            if (touch && touch->selecting()) // margin - touches may not reach screen border comfortably, so turn on scrolling with margin for them, but only after some movement to prevent instant scroll at start
                            {
                                Flt margin = ts.size.x;
                                MAX(clipped_text_rect.min.x, D.rectUI().min.x + margin);
                                MIN(clipped_text_rect.max.x, D.rectUI().max.x - margin);

                                margin = ts.size.y;
                                MAX(clipped_text_rect.min.y, D.rectUI().min.y + margin);
                                MIN(clipped_text_rect.max.y, D.rectUI().max.y - margin);
                            }
                            // if pos is outside of clipped text rect then scroll (check <= instead of < in case we're at screen border), scroll parents only if rect is actually clipped with some margin (so we can still scroll a little bit more so we can see visually the rect)
                            if (mt_pos->x <= clipped_text_rect.min.x)
                                ScrollMinus(&slidebar[0], (rect().min.x + gpc.offset.x < clip.min.x + (ts.size.x * TEXTBOX_MARGIN_REL + TEXTBOX_MARGIN_ABS)) ? parent() : null, false);
                            else if (mt_pos->x >= clipped_text_rect.max.x)
                                ScrollPlus(&slidebar[0], (rect().max.x + gpc.offset.x > clip.max.x - (ts.size.x * TEXTBOX_MARGIN_REL + TEXTBOX_MARGIN_ABS)) ? parent() : null, false);
                            if (mt_pos->y <= clipped_text_rect.min.y)
                                ScrollPlus(&slidebar[1], (rect().min.y + gpc.offset.y < clip.min.y + (ts.size.y * TEXTBOX_MARGIN_REL + TEXTBOX_MARGIN_ABS)) ? parent() : null, true);
                            else if (mt_pos->y >= clipped_text_rect.max.y)
                                ScrollMinus(&slidebar[1], (rect().max.y + gpc.offset.y > clip.max.y - (ts.size.y * TEXTBOX_MARGIN_REL + TEXTBOX_MARGIN_ABS)) ? parent() : null, true);
                        } else if (mt_state & BS_RELEASED) // released
                        {
                            if (touch && _edit.sel >= 0)
                                Gui.showTextMenu(); // by touch and have some selection
                        }
                    }
                }
        } else
            _can_select = true;
    }
}
void TextBox::draw(C GuiPC &gpc) {
    if (/*gpc.visible &&*/ visible()) {
        GuiSkin *skin = getSkin();
        Rect rect = T.rect() + gpc.offset, ext_rect;
        if (skin && skin->region.normal)
            skin->region.normal->extendedRect(rect, ext_rect);
        else
            ext_rect = rect;
        if (Cuts(ext_rect, gpc.clip)) {
            if (skin) {
                D.clip(gpc.clip);
                if (skin->region.normal)
                    skin->region.normal->draw(skin->region.normal_color, rect);
                else if (skin->region.normal_color.a)
                    rect.draw(skin->region.normal_color);

                // text
                if (TextStyle *text_style = skin->textline.text_style()) {
                    Bool enabled = (T.enabled() && gpc.enabled),
                         active = (Gui.kb() == this && enabled);
                    if (T().is() || active || hint.is()) {
                        TextStyleParams ts = *text_style;

                        C Color *text_color; // never null
                        if (enabled) {
                            text_color = &skin->textline.normal_text_color;
                            ts.image_color.a = 255;
                        } else {
                            text_color = &skin->textline.disabled_text_color;
                            ts.image_color.a = 128;
                        }

                        ts.align.set(1, -1);
                        ts.color = ColorMul(ts.color, *text_color);
#if DEFAULT_FONT_FROM_CUSTOM_SKIN
                        if (!ts.font())
                            ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
#endif

                        Rect text_rect = clientRect() + gpc.offset;
                        D.clip(text_rect & gpc.clip);
                        Flt offset = TEXTBOX_OFFSET * ts.size.x;
                        if (T().is() || active) {
                            text_rect.min.x += offset - slidebar[0].offset();
                            text_rect.max.x = text_rect.min.x + virtualWidth() - offset * 2;
                            text_rect.max.y += slidebar[1].offset();
                            text_rect.min.y = text_rect.max.y - virtualHeight();
                            if (active)
                                ts.edit = &_edit;
                            D.text(ts, text_rect, T(), null, 0, wordWrap(), &_text_space);
#if DEBUG && 0 // draw cursor position
                            Vec2 pos = ts.textPos(T(), cursor(), _text_space, wordWrap());
                            (pos * Vec2(1, -1) + text_rect.lu()).draw(RED);
#endif
                        } else if (hint.is()) {
                            text_rect.extendX(-offset); // we could've set virtualSize to full client size as a special case in 'setVirtualSize', however to just do this here, is faster (also because most of the time, hint is not displayed)
                            ts.color.a = ((ts.color.a * 96) >> 8);
                            ts.size *= 0.85f;
                            D.text(ts, text_rect, hint, null, 0, true);
                        }
                    }
                }
            }
            view.draw(gpc);
            slidebar[0].draw(gpc);
            slidebar[1].draw(gpc);

            if (kb_lit && Gui.kb() == this) {
                D.clip(gpc.clip);
                Gui.kbLit(this, rect, skin);
            }
        }
    }
}
/******************************************************************************/
Bool TextBox::save(File &f, CChar *path) C {
    if (super::save(f, path)) {
        // no need to save '_crect', '_text_space' because we always reset it after loading
        f.putMulti(Byte(1), kb_lit, _word_wrap, _max_length, _slidebar_size); // version
        f << hint << _text;
        f.putAsset(_skin.id());
        if (view.save(f, path))
            if (slidebar[0].save(f, path))
                if (slidebar[1].save(f, path))
                    return f.ok();
    }
    return false;
}
Bool TextBox::load(File &f, CChar *path) {
    del();
    if (super::load(f, path))
        switch (f.decUIntV()) // version
        {
        case 1: {
            f.getMulti(kb_lit, _word_wrap, _max_length, _slidebar_size);
            f >> hint >> _text;
            _skin.require(f.getAssetID(), path);
            if (view.load(f, path))
                if (slidebar[0].load(f, path))
                    if (slidebar[1].load(f, path))
                        if (f.ok()) {
                            setParams();
                            setVirtualSize();
                            return true;
                        } // have to reset virtual size in case text style has different values now, and because we're not saving '_crect'
        } break;

        case 0: {
            f.getMulti(kb_lit, _word_wrap, _max_length, _slidebar_size)._getStr1(hint)._getStr1(_text);
            _skin.require(f.getAssetID(), path);
            if (view.load(f, path))
                if (slidebar[0].load(f, path))
                    if (slidebar[1].load(f, path))
                        if (f.ok()) {
                            setParams();
                            setVirtualSize();
                            return true;
                        } // have to reset virtual size in case text style has different values now, and because we're not saving '_crect'
        } break;
        }
    del();
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
