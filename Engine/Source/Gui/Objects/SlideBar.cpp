/******************************************************************************

   Don't try to disable '_pixel_align' for buttons,
      because the middle button would flicker when moved.

/******************************************************************************/
#include "stdafx.h"
namespace EE {
static GuiSkin GuiSkinTemp;
#define CLIP_UNDER 1 // if adjust clipping so we don't draw underneath the SlideBar (looks better for semi-transparent SlideBars)
/******************************************************************************/
void SlideBar::zero() {
    sbc = SBC_STEP;
    _scroll_immediate = false;
    _scroll = false;
    _vertical = false;
    _usable = false; // keep it always as false when deleted
    _focusable = true;
    _scroll_to = 0;
    _offset = 0;
    _length = _length_total = 0;
    _scroll_mul = 0.5f;
    _scroll_add = 0;
    _scroll_button = 1.5f;
    _button_size = 0;
    _func_immediate = false;
    _func_user = null;
    _func = null;
}
SlideBar::SlideBar() { zero(); }
SlideBar &SlideBar::del() {
    _skin.clear();
    REPA(button)
    button[i].del();
    super::del();
    zero();
    return T;
}
/******************************************************************************/
void SlideBar::setButtonSubType() {
    button[SB_MIDDLE]._sub_type = BUTTON_TYPE_SLIDEBAR_CENTER;
    button[SB_LEFT_UP]._sub_type = (_vertical ? BUTTON_TYPE_SLIDEBAR_RIGHT : BUTTON_TYPE_SLIDEBAR_LEFT);    // this is the "min" button (left /up  ), because of how this button is positioned in the slidebar and how 'drawVertical' is performed, in vertical mode this actually needs to be drawn from the right skin
    button[SB_RIGHT_DOWN]._sub_type = (_vertical ? BUTTON_TYPE_SLIDEBAR_LEFT : BUTTON_TYPE_SLIDEBAR_RIGHT); // this is the "max" button (right/down), because of how this button is positioned in the slidebar and how 'drawVertical' is performed, in vertical mode this actually needs to be drawn from the left  skin
}
void SlideBar::setParams() {
    _type = GO_SLIDEBAR;
    REPA(button) {
        Button &b = button[i];
        b._parent = this;
        b._vertical = _vertical;
    }
    setButtonSubType();
}
SlideBar &SlideBar::create() {
    del();

    _visible = true;
    _usable = true;
    _length = 1;
    _length_total = 11;
    REPA(button) {
        Button &b = button[i];
        b.create();
        b.mode = BUTTON_CONTINUOUS;
        b._focusable = false;
    }
    setParams();
    return T;
}
SlideBar &SlideBar::create(C SlideBar &src) {
    if (this != &src) {
        if (!src.is())
            del();
        else {
            create().copyParams(src);
            _type = GO_SLIDEBAR;
            sbc = src.sbc;
            _scroll_immediate = src._scroll_immediate;
            _scroll = src._scroll;
            _vertical = src._vertical;
            _usable = src._usable;
            _focusable = src._focusable;
            _scroll_to = src._scroll_to;
            _offset = src._offset;
            _length = src._length;
            _length_total = src._length_total;
            _scroll_mul = src._scroll_mul;
            _scroll_add = src._scroll_add;
            _scroll_button = src._scroll_button;
            _button_size = src._button_size;
            _func_immediate = src._func_immediate;
            _func_user = src._func_user;
            _func = src._func;
            _skin = src._skin;
            REPAO(button).create(src.button[i])._parent = this;
        }
    }
    return T;
}
/******************************************************************************/
SlideBar &SlideBar::func(void (*func)(Ptr), Ptr user, Bool immediate) {
    T._func = func;
    T._func_user = user;
    T._func_immediate = immediate;
    return T;
}
void SlideBar::call() {
    if (_func) {
        if (_func_immediate) {
            DEBUG_BYTE_LOCK(_used);
            _func(_func_user);
        } else
            Gui.addFuncCall(_func, _func_user);
    }
}
/******************************************************************************/
SlideBar &SlideBar::focusable(Bool on) {
    if (_focusable != on) {
        _focusable = on;
        if (!on)
            kbClear();
    }
    return T;
}
/******************************************************************************/
SlideBar &SlideBar::desc(C Str &desc) {
    super ::desc(desc);
    button[0].desc(desc);
    button[1].desc(desc);
    button[2].desc(desc);
    return T;
}
/******************************************************************************/
void SlideBar::setButtonRect() {
    if (!App.closed()) // this may be called after app has closed, in which case the Gui Skin pointers are invalid which may cause crash
    {
        Rect r;
        Flt button_w = _button_size,
            button_h = _button_size;
        GuiSkin *skin1 = button[SB_LEFT_UP].getSkin();
        GuiSkin::ButtonImage *image1 = (skin1 ? _vertical ? &skin1->slidebar.right : &skin1->slidebar.left : null); // vertical mode has images swapped
        GuiSkin *skin2 = button[SB_RIGHT_DOWN].getSkin();
        GuiSkin::ButtonImage *image2 = (skin2 ? _vertical ? &skin2->slidebar.left : &skin2->slidebar.right : null); // vertical mode has images swapped
        button[SB_LEFT_UP].visible(image1 && (image1->normal || image1->image));
        button[SB_RIGHT_DOWN].visible(image2 && (image2->normal || image2->image));
        Int buttons = (button[SB_LEFT_UP].visible() + button[SB_RIGHT_DOWN].visible());
        if (_vertical) {
            if (buttons)
                MIN(button_h, rect().h() / buttons);
            else
                button_h = 0; // if buttons don't fit in space
            Flt space = rect().h() - buttons * button_h, size = (lengthTotal() ? space * Sat(length() / lengthTotal()) : 0), clamped_size = Max(size, Min(_button_size * 0.9f, space));
            r.min.x = rect().min.x;
            r.max.y = rect().max.y - (space - clamped_size) * T();
            if (button[SB_LEFT_UP].visible())
                r.max.y -= button_h;
            r.max.x = rect().max.x;
            r.min.y = r.max.y - clamped_size;
        } else {
            if (buttons)
                MIN(button_w, rect().w() / buttons);
            else
                button_w = 0; // if buttons don't fit in space
            Flt space = rect().w() - buttons * button_w, size = (lengthTotal() ? space * Sat(length() / lengthTotal()) : 0), clamped_size = Max(size, Min(_button_size * 0.9f, space));
            r.min.y = rect().min.y;
            r.min.x = rect().min.x + (space - clamped_size) * T();
            if (button[SB_LEFT_UP].visible())
                r.min.x += button_w;
            r.max.y = rect().max.y;
            r.max.x = r.min.x + clamped_size;
        }
        button[SB_MIDDLE].rect(r);
        button[SB_MIDDLE]._vertical = _vertical;
        button[SB_LEFT_UP].rect(Rect(rect().min.x, rect().max.y - button_h * button[SB_LEFT_UP].visible(), rect().min.x + button_w * button[SB_LEFT_UP].visible(), rect().max.y));
        button[SB_LEFT_UP]._vertical = _vertical;
        button[SB_RIGHT_DOWN].rect(Rect(rect().max.x - button_w * button[SB_RIGHT_DOWN].visible(), rect().min.y, rect().max.x, rect().min.y + button_h * button[SB_RIGHT_DOWN].visible()));
        button[SB_RIGHT_DOWN]._vertical = _vertical;
    }
}
SlideBar &SlideBar::rect(C Rect &rect) {
    if (T.rect() != rect) {
        super::rect(rect);
        if (_vertical != T.rect().vertical()) {
            _vertical ^= 1;
            setButtonSubType();
        }
        _button_size = (_vertical ? T.rect().w() : T.rect().h());
        setButtonRect();
    }
    return T;
}
SlideBar &SlideBar::move(C Vec2 &delta) {
    if (delta.any()) {
        super::move(delta);
        button[0].move(delta);
        button[1].move(delta);
        button[2].move(delta);
    }
    return T;
}
/******************************************************************************/
SlideBar &SlideBar::setLengths(Flt length, Flt length_total) {
    if (is()) // don't enable '_usable' when deleted
    {
        MAX(length, 0);
        MAX(length_total, 0);
        T._length = length;
        T._length_total = length_total;
        T._usable = (length + EPS < length_total);
        if (_scroll)
            _scroll_to = Max(0, Min(_scroll_to, maxOffset())); // adjust current scrolling if any
        return offset(_offset, SET_DEFAULT, SCROLL_ABS);
    }
    return T;
}
Flt SlideBar::operator()() C {
    Flt d = maxOffset();
    return (d > 0) ? _offset / d : 0;
}
Flt SlideBar::wantedOffset() C { return _scroll ? _scroll_to : _offset; }
Flt SlideBar::scrollDelta() C { return _scroll ? _scroll_to - _offset : 0; }

SlideBar &SlideBar::offset(Flt offset, SET_MODE mode, SCROLL scroll) {
    if (_usable) {
        if (_scroll)
            switch (scroll) {
            case SCROLL_STOP:
                _scroll = false;
                break;
            case SCROLL_REL:
                Flt delta = offset - _offset;
                _scroll_to = Max(0, Min(_scroll_to + delta, maxOffset()));
                break;
            }
        offset = Max(0, Min(offset, maxOffset()));
    } else {
        _scroll = false;
        offset = 0; // if not usable then scrolling will be disabled and small offsets below EPS may not get cleared to 0, in that case force precise 0 in case some codes do comparison against 0 value, and stop any scroll
    }
    if (T._offset != offset) {
        T._offset = offset;
        if (mode != QUIET)
            call();
    }
    setButtonRect();
    return T;
}
SlideBar &SlideBar::set(Flt frac, SET_MODE mode, SCROLL scroll) { return offset(Sat(frac) * maxOffset(), mode, scroll); }
SlideBar &SlideBar::scroll(Flt delta, Bool immediate) {
    if (immediate) {
        offset(_offset + delta, SET_DEFAULT, SCROLL_REL); // adjust current offset but without stopping
    } else {
        if (_scroll)
            _scroll_to += delta; // if currently scrolling
        else {
            _scroll_to = delta + _offset;
            _scroll = true;
        }
        _scroll_to = Max(0, Min(_scroll_to, maxOffset()));
    }
    return T;
}
SlideBar &SlideBar::scrollTo(Flt pos, Bool immediate) {
    Flt min = Max(0, Min(pos, maxOffset()));
    if (min < _offset) {
        _scroll = true;
        _scroll_to = min;
    } else {
        Flt max = Max(0, Min(pos, lengthTotal()) - length());
        if (max > _offset) {
            _scroll = true;
            _scroll_to = max;
        } else {
            _scroll = false;
            _scroll_to = _offset;
        }
    }
    return immediate ? offset(_scroll_to) : T;
}
SlideBar &SlideBar::scrollFit(Flt min, Flt max, Bool immediate) {
    min = Max(0, Min(min, maxOffset()));
    if (min < _offset) {
        _scroll = true;
        _scroll_to = min;
    } else {
        max = Max(0, Min(max - length(), min));
        if (max > _offset) {
            _scroll = true;
            _scroll_to = max;
        } else {
            _scroll = false;
            _scroll_to = _offset;
        }
    }
    return immediate ? offset(_scroll_to) : T;
}
SlideBar &SlideBar::scrollEnd(Bool immediate) { return scrollTo(lengthTotal(), immediate); }
SlideBar &SlideBar::scrollLeftUp() {
    if (button[SB_LEFT_UP].is())
        button[SB_LEFT_UP].push();
    else
        offset(_offset - Time.ad() * _scroll_button);
    return T;
}
SlideBar &SlideBar::scrollRightDown() {
    if (button[SB_RIGHT_DOWN].is())
        button[SB_RIGHT_DOWN].push();
    else
        offset(_offset + Time.ad() * _scroll_button);
    return T;
}
SlideBar &SlideBar::scrollOptions(Flt relative, Flt base, Bool immediate, Flt button_speed) {
    T._scroll_mul = Max(0, relative);
    T._scroll_add = Max(0, base);
    T._scroll_immediate = immediate;
    T._scroll_button = Max(0, button_speed);
    return T;
}
SlideBar &SlideBar::removeSideButtons() {
    button[SB_LEFT_UP].del();
    button[SB_RIGHT_DOWN].del();
    setButtonRect();
    return T;
}
SlideBar &SlideBar::skin(C GuiSkinPtr &skin) {
    if (_skin != skin) {
        _skin = skin;
        REPAO(button).skin = skin;
        setButtonRect();
    }
    return T;
}
/******************************************************************************/
GuiObj *SlideBar::test(C GuiPC &gpc, C Vec2 &pos, GuiObj *&mouse_wheel) {
    if (GuiObj *go = super::test(gpc, pos, mouse_wheel)) {
        if (_usable)
            mouse_wheel = this;

        if (GuiObj *go = button[2].test(gpc, pos, mouse_wheel))
            return go;
        if (GuiObj *go = button[1].test(gpc, pos, mouse_wheel))
            return go;
        if (GuiObj *go = button[0].test(gpc, pos, mouse_wheel))
            return go;

        return go;
    }
    return null;
}
/******************************************************************************/
void SlideBar::update(C GuiPC &gpc) {
    GuiPC gpc_this(gpc, visible(), enabled());
    if (gpc_this.enabled) {
        Int scroll_discrete = 0;
        Flt scroll_smooth = 0;

        // check for click on the back
        switch (sbc) {
        case SBC_STEP:
        case SBC_SMOOTH: {
            REPA(MT)
            if (MT.guiObj(i) == this && MT.bp(i)) {
                if (sbc == SBC_STEP) {
                    if (_vertical)
                        scroll_discrete -= Sign(MT.pos(i).y - (button[SB_MIDDLE].rect().centerY() + gpc.offset.y));
                    else
                        scroll_discrete += Sign(MT.pos(i).x - (button[SB_MIDDLE].rect().centerX() + gpc.offset.x));
                } else // change focus to buttons
                {
                    if (_vertical)
                        MT.guiObj(i, &button[(MT.pos(i).y > button[SB_MIDDLE].rect().centerY() + gpc.offset.y) ? 1 : 2]);
                    else
                        MT.guiObj(i, &button[(MT.pos(i).x < button[SB_MIDDLE].rect().centerX() + gpc.offset.x) ? 1 : 2]);
                }
            }
        } break;

        case SBC_SET_POS: {
            REPA(MT)
            if (MT.guiObj(i) == this && (MT.state(i) & (BS_ON | BS_PUSHED))) // check for 'bp' as well because single touchpad taps are not registered as 'b'
            {
                if (_vertical) {
                    Flt h_2 = button[SB_MIDDLE].rect().h() * 0.5f;
                    set(LerpR(button[SB_LEFT_UP].rect().min.y - h_2, button[SB_RIGHT_DOWN].rect().max.y + h_2, MT.pos(i).y - gpc.offset.y));
                } else {
                    Flt w_2 = button[SB_MIDDLE].rect().w() * 0.5f;
                    set(LerpR(button[SB_LEFT_UP].rect().max.x + w_2, button[SB_RIGHT_DOWN].rect().min.x - w_2, MT.pos(i).x - gpc.offset.x));
                }
            }
        } break;
        }

        if (Gui.wheel() == this) {
            scroll_smooth -= Ms.wheel();
        }
        if (Gui.kb() == this) {
            if (_vertical) {
                if (Kb.b(KB_UP))
                    scrollLeftUp();
                if (Kb.b(KB_DOWN))
                    scrollRightDown();
            } else {
                if (Kb.b(KB_LEFT))
                    scrollLeftUp();
                if (Kb.b(KB_RIGHT))
                    scrollRightDown();
            }
            scroll_discrete += Kb.k(KB_PGDN);
            scroll_discrete -= Kb.k(KB_PGUP);
            if (Kb.k(KB_HOME)) {
                scrollTo(0);
                scroll_discrete = 0;
            }
            if (Kb.k(KB_END)) {
                scrollTo(lengthTotal());
                scroll_discrete = 0;
            }
        }

        // buttons
        button[0].update(gpc_this);
        button[1].update(gpc_this);
        button[2].update(gpc_this);
        if ((button[0]() || button[1]() || button[2]()) && _usable) {
            Flt d = 0;
            if (button[SB_MIDDLE]())
                REPA(MT)
            if (MT.guiObj(i) == &button[SB_MIDDLE] && MT.b(i)) {
                if (_vertical)
                    d -= MT.dc(i).y * lengthTotal() / (button[SB_LEFT_UP].rect().min.y - button[SB_RIGHT_DOWN].rect().max.y);
                else
                    d += MT.dc(i).x * lengthTotal() / (button[SB_RIGHT_DOWN].rect().min.x - button[SB_LEFT_UP].rect().max.x);
                MT.disableScroll(i);
            }
            if (button[SB_LEFT_UP]())
                d -= Time.ad() * _scroll_button;
            if (button[SB_RIGHT_DOWN]())
                d += Time.ad() * _scroll_button;
            offset(_offset + d);
        }

        // scroll
        if (scroll_discrete || scroll_smooth)
            scroll((scroll_discrete + scroll_smooth) * (_scroll_mul * length() + _scroll_add), _scroll_immediate);
        if (_scroll) {
            if (_scroll_immediate || Equal(_scroll_to, _offset, 0.001f))
                offset(_scroll_to);
            else {
                Flt offset = T._offset;
                AdjustValTime(offset, _scroll_to, 0.0001f, Time.ad());
                T.offset(offset, SET_DEFAULT, SCROLL_ABS);
            }
        }
    }
}
/******************************************************************************/
void SlideBar::draw(C GuiPC &gpc) {
    if (/*gpc.visible &&*/ visible())
        if (GuiSkin *skin = getSkin()) {
            Rect r = rect() + gpc.offset, ext_rect;
            if (skin->slidebar.background)
                skin->slidebar.background->extendedRect(r, ext_rect);
            else
                ext_rect = r;
            if (Cuts(ext_rect, gpc.clip)) {
                D.clip(gpc.clip);

                // background
                Rect background_rect = r;
                if (_vertical) {
                    background_rect.min.y = button[SB_RIGHT_DOWN].rect().max.y + gpc.offset.y;
                    background_rect.max.y = button[SB_LEFT_UP].rect().min.y + gpc.offset.y;
                } else {
                    background_rect.min.x = button[SB_LEFT_UP].rect().max.x + gpc.offset.x;
                    background_rect.max.x = button[SB_RIGHT_DOWN].rect().min.x + gpc.offset.x;
                }
                if (skin->slidebar.background) {
                    if (_vertical)
                        skin->slidebar.background->drawVertical(skin->slidebar.background_color, TRANSPARENT, background_rect);
                    else
                        skin->slidebar.background->draw(skin->slidebar.background_color, TRANSPARENT, background_rect);
                } else if (skin->slidebar.background_color.a)
                    background_rect.draw(skin->slidebar.background_color);

                // scroll target
                if (_scroll && _usable)
                    if (GuiSkin *button_skin = button[SB_MIDDLE].getSkin())
                        if (Flt d = maxOffset()) {
                            d = (_scroll_to - _offset) / d;
                            Button button_temp;
                            button_temp.create(button[SB_MIDDLE]);

                            // !! do a fast copy that avoids expensive cache elm assignment, must be in sync with 'ZeroFast' below !!
                            CopyFast(GuiSkinTemp.slidebar.center, button_skin->slidebar.center);

                            GuiSkinTemp.slidebar.center.normal_color.a = ((GuiSkinTemp.slidebar.center.normal_color.a * 96) >> 8);
                            GuiSkinTemp.slidebar.center.pushed_color.a = ((GuiSkinTemp.slidebar.center.pushed_color.a * 96) >> 8);
                            button_temp.skin = &GuiSkinTemp;

#if CLIP_UNDER
                            GuiPC temp = gpc;
#endif
                            if (_vertical) {
                                d *= button[SB_LEFT_UP].rect().min.y - button[SB_RIGHT_DOWN].rect().max.y - button[SB_MIDDLE].rect().h();
                                button_temp._rect.min.y -= d;
                                button_temp._rect.max.y -= d;
#if CLIP_UNDER
                                if (d > 0)
                                    MIN(temp.clip.max.y, button[SB_MIDDLE].rect().min.y + gpc.offset.y); // sliding down
                                else
                                    MAX(temp.clip.min.y, button[SB_MIDDLE].rect().max.y + gpc.offset.y); // sliding up
#endif
                            } else {
                                d *= button[SB_RIGHT_DOWN].rect().min.x - button[SB_LEFT_UP].rect().max.x - button[SB_MIDDLE].rect().w();
                                button_temp._rect.min.x += d;
                                button_temp._rect.max.x += d;
#if CLIP_UNDER
                                if (d < 0)
                                    MIN(temp.clip.max.x, button[SB_MIDDLE].rect().min.x + gpc.offset.x); // sliding left
                                else
                                    MAX(temp.clip.min.x, button[SB_MIDDLE].rect().max.x + gpc.offset.x); // sliding right
#endif
                            }
#if CLIP_UNDER
                            button_temp.draw(temp);
#else
                            button_temp.draw(gpc);
#endif

                            // !! do a fast clear that avoids expensive cache elm release, must be in sync with 'CopyFast' above !!
                            ZeroFast(GuiSkinTemp.slidebar.center);
                        }
                button[SB_MIDDLE].draw(gpc);
                if (button[SB_LEFT_UP].is())
                    button[SB_LEFT_UP].draw(gpc);
                if (button[SB_RIGHT_DOWN].is())
                    button[SB_RIGHT_DOWN].draw(gpc);
                if (Gui.kb() == this)
                    Gui.kbLit(this, r, skin);
            }
        }
}
/******************************************************************************/
Bool SlideBar::save(File &f, CChar *path) C {
    if (super::save(f, path)) {
        f.putMulti(Byte(6), sbc, _scroll_immediate, _vertical, _usable, _focusable, _offset, _length, _length_total); // version
        f.putMulti(_scroll_mul, _scroll_add, _scroll_button, _button_size);
        f.putAsset(_skin.id());
        if (button[SB_MIDDLE].save(f, path))
            if (button[SB_LEFT_UP].save(f, path))
                if (button[SB_RIGHT_DOWN].save(f, path))
                    return f.ok();
    }
    return false;
}
Bool SlideBar::load(File &f, CChar *path) {
    del();
    if (super::load(f, path))
        switch (f.decUIntV()) // version
        {
        case 6: {
            f.getMulti(sbc, _scroll_immediate, _vertical, _usable, _focusable, _offset, _length, _length_total);
            f.getMulti(_scroll_mul, _scroll_add, _scroll_button, _button_size);
            _skin.require(f.getAssetID(), path);
            if (button[SB_MIDDLE].load(f, path))
                if (button[SB_LEFT_UP].load(f, path))
                    if (button[SB_RIGHT_DOWN].load(f, path))
                        if (f.ok()) {
                            setParams();
                            setButtonRect();
                            return true;
                        } // call 'setButtonRect' because it may be dependent on current 'Gui.skin'
        } break;

        case 5: {
            f >> sbc >> _scroll_immediate >> _vertical >> _usable >> _focusable >> _offset >> _length >> _length_total >> _scroll_mul >> _scroll_add >> _scroll_button >> _button_size;
            _skin.require(f._getAsset(), path);
            if (button[SB_MIDDLE].load(f, path))
                if (button[SB_LEFT_UP].load(f, path))
                    if (button[SB_RIGHT_DOWN].load(f, path))
                        if (f.ok()) {
                            setParams();
                            setButtonRect();
                            return true;
                        } // call 'setButtonRect' because it may be dependent on current 'Gui.skin'
        } break;

        case 4: {
            f >> _focusable;
            f.skip(4);
            f >> _scroll_immediate >> _vertical >> _usable >> _offset >> _length >> _length_total >> _scroll_mul >> _scroll_add >> _scroll_button >> _button_size >> sbc;
            f._getStr();
            if (button[SB_MIDDLE].load(f, path))
                if (button[SB_LEFT_UP].load(f, path))
                    if (button[SB_RIGHT_DOWN].load(f, path))
                        if (f.ok()) {
                            setParams();
                            setButtonRect();
                            return true;
                        } // call 'setButtonRect' because it may be dependent on current 'Gui.skin'
        } break;

        case 3: {
            f >> _focusable;
            f.skip(4);
            f >> _scroll_immediate >> _vertical >> _usable >> _offset >> _length >> _length_total >> _scroll_mul >> _scroll_add >> _scroll_button >> _button_size;
            sbc = SBC_STEP;
            f._getStr();
            if (button[SB_MIDDLE].load(f, path))
                if (button[SB_LEFT_UP].load(f, path))
                    if (button[SB_RIGHT_DOWN].load(f, path))
                        if (f.ok()) {
                            setParams();
                            setButtonRect();
                            return true;
                        } // call 'setButtonRect' because it may be dependent on current 'Gui.skin'
        } break;

        case 2: {
            f >> _focusable;
            f.skip(4);
            f >> _scroll_immediate >> _vertical >> _usable >> _offset >> _length >> _length_total >> _scroll_mul >> _scroll_add >> _button_size;
            sbc = SBC_STEP;
            f._getStr();
            if (button[SB_MIDDLE].load(f, path))
                if (button[SB_LEFT_UP].load(f, path))
                    if (button[SB_RIGHT_DOWN].load(f, path))
                        if (f.ok()) {
                            setParams();
                            setButtonRect();
                            return true;
                        } // call 'setButtonRect' because it may be dependent on current 'Gui.skin'
        } break;

        case 1: {
            f >> _focusable;
            f.skip(4);
            f >> _scroll_immediate >> _vertical >> _usable >> _offset >> _length >> _length_total >> _scroll_mul >> _scroll_add >> _button_size;
            sbc = SBC_STEP;
            f._getStr();
            if (button[SB_MIDDLE].load(f, path))
                if (button[SB_LEFT_UP].load(f, path))
                    if (button[SB_RIGHT_DOWN].load(f, path))
                        if (f.ok()) {
                            setParams();
                            setButtonRect();
                            return true;
                        } // call 'setButtonRect' because it may be dependent on current 'Gui.skin'
        } break;

        case 0: {
            f >> _focusable;
            f.skip(4);
            f >> _scroll_immediate >> _vertical >> _usable >> _offset >> _length >> _length_total >> _scroll_mul >> _scroll_add >> _button_size;
            sbc = SBC_STEP;
            if (button[SB_MIDDLE].load(f, path))
                if (button[SB_LEFT_UP].load(f, path))
                    if (button[SB_RIGHT_DOWN].load(f, path))
                        if (f.ok()) {
                            setParams();
                            setButtonRect();
                            return true;
                        } // call 'setButtonRect' because it may be dependent on current 'Gui.skin'
        } break;
        }
    del();
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
