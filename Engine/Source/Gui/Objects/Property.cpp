/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************

   Callbacks for 'Changed' and others can be called immediately because inside them we need 'prop' property anyway.

   If callbacks from '_pre_changed', 'fromGui' or '_changed' need to destroy objects,
      they should make a callback first instead of doing it immediately.

/******************************************************************************/
#pragma warning(disable : 4305) // truncation from 'Long' to 'Dbl'
/******************************************************************************/
static void Changed(Property &prop) {
    DEBUG_BYTE_LOCK(prop.button._used); // 'prop' doesn't have '_used' so just use any of its gui objects (such as button)
    if (prop._pre_changed)
        prop._pre_changed(prop);
    if (prop._auto_data)
        prop.fromGui(prop._auto_data);
    if (prop._changed)
        prop._changed(prop);
}
/******************************************************************************/
Property::~Property() {
    Delete(_cp);
    Delete(_win_io);
}
Property::Property() {
    min_use = max_use = mouse_edit_linked = false;
    min_value = max_value = 0;
    mouse_edit_mode = PROP_MOUSE_EDIT_LINEAR;
    mouse_edit_speed = 1.0f;
    real_precision = INT_MAX;
    _changed = _pre_changed = null;
    _auto_data = null;
    _cp = null;
    _win_io = null;
}
/******************************************************************************/
Int Property::actualPrecision() C {
    return md.integer() ? 0 : (real_precision == INT_MAX) ? md.dblPrecision() ? PRECISION_DBL : PRECISION_FLT
                                                          : real_precision;
}
/******************************************************************************/
Property &Property::create(C Str &name, C MemberDesc &md) {
    T.md = md;

    switch (md.type) {
    case DATA_NONE:
        _value_type = GO_NONE;
        break;

    case DATA_BOOL: {
        _value_type = GO_CHECKBOX;
        checkbox.create().func(Changed, T, true);
    } break;

    case DATA_INT:
    case DATA_UINT:
    case DATA_REAL:
    case DATA_VEC2:
    case DATA_VECI2: {
        _value_type = GO_TEXTLINE;
        textline.create().func(Changed, T, true);
        button.create().func(MouseEdit, T, true)._sub_type = BUTTON_TYPE_PROPERTY_VALUE;
        button.mode = BUTTON_CONTINUOUS;
        button._focusable = false;
    } break;

    case DATA_IMAGE_PTR:
    case DATA_IMAGEPTR: {
        setFile("img");
    } break;

    default: {
        _value_type = GO_TEXTLINE;
        textline.create().func(Changed, T, true);
    } break;
    }

    if (md.integer())
        mouse_edit_speed = 40;

    switch (md.type) {
    case DATA_INT: {
        switch (md.size) {
        case 1:
            min_use = true;
            max_use = true;
            min_value = -0x0080;
            max_value = 0x007F;
            break;
        case 2:
            min_use = true;
            max_use = true;
            min_value = -0x8000;
            max_value = 0x7FFF;
            break;
        case 4:
            min_use = true;
            max_use = true;
            min_value = INT_MIN;
            max_value = INT_MAX;
            break;
        case 8:
            min_use = false;
            max_use = true;
            min_value = LONG_MIN;
            max_value = LONG_MAX;
            break;
        }
    } break;

    case DATA_UINT: {
        switch (md.size) {
        case 1:
            min_use = true;
            max_use = true;
            min_value = 0;
            max_value = 0x00FF;
            break;
        case 2:
            min_use = true;
            max_use = true;
            min_value = 0;
            max_value = 0xFFFF;
            break;
        case 4:
            min_use = true;
            max_use = true;
            min_value = 0;
            max_value = UINT_MAX;
            break;
        case 8:
            min_use = true;
            max_use = false;
            min_value = 0;
            max_value = ULONG_MAX;
            break;
        }
    } break;
    }
    text.create(T.name = name);
    return T;
}
/******************************************************************************/
void Property::Color::update(C GuiPC &gpc) {
    if (gpc.visible && gpc.enabled && visible() && enabled()) {
        Bool pushed = (Gui.ms() == this && Ms.bp(0));
        if (!pushed)
            REPA(Touches) {
                Touch &touch = Touches[i];
                if (touch.guiObj() == this && touch.pd()) {
                    pushed = true;
                    break;
                }
            }
        if (pushed) {
            Property &prop = *(Property *)user;
            if (prop._cp) {
                if (prop._cp->hidden())
                    if (GuiObj *win = first(GO_WINDOW))
                        prop._cp->level(win->asWindow().level() + 1); // if the ColorPicker was not displayed yet, then set its level based on the first parent, do this before activating it, and do this here and not in 'setColor' because over there the properties aren't attached to Windows yet
                prop._cp->focusToggle();
            }
        }
    }
}
void Property::Color::draw(C GuiPC &gpc) {
    if (/*gpc.visible &&*/ visible()) {
        D.clip(gpc.clip);
        Property &prop = *(Property *)user;
        Rect rect = T.rect() + gpc.offset;
        if (prop._cp) {
            ::Color bottom = (*prop._cp)(), top = bottom;
            top.a = 255;
            rect.drawShadedY(top, bottom);
        }
        rect.draw(Gui.borderColor(), false);
    }
}
Property &Property::setColor() {
    if (!_cp) {
        _value_type = GO_CUSTOM;

        _color.create(this).desc(desc()); // try re-using existing description before deleting objects
        New(_cp)->create(display()).func(Changed, T, true).hide();

        Delete(_win_io);
        checkbox.del();
        textline.del();
        button.del();
        combobox.del();
        slider.del();
    }
    return T;
}
/******************************************************************************/
Property &Property::setFile(C Str &ext, C Str &desc) {
    if (!_win_io) {
        _value_type = GO_TEXTLINE;

        textline.create().func(Changed, T, true).desc(desc()); // try re-using existing description before deleting objects
        button.create("...").func(SelectFile, T, true).focusable(false).image = null;

        Delete(_cp);
        checkbox.del();
        combobox.del();
        slider.del();
        _color.del();
        _win_io_ext = ext;
        _win_io_ext_desc = desc;
    }
    return T;
}
/******************************************************************************/
Property &Property::setEnum() {
    if (!md.type)
        md.type = DATA_INT; // when using 'MemberDesc' then members of 'enum' type aren't detected but set to DATA_NONE
    _value_type = GO_COMBOBOX;

    combobox.create().func(Changed, T, true).desc(desc()); // try re-using existing description before deleting objects

    Delete(_cp);
    Delete(_win_io);
    checkbox.del();
    textline.del();
    button.del();
    slider.del();
    _color.del();
    return T;
}
Property &Property::setEnum(CChar8 *data[], Int elms) {
    setEnum();
    combobox.setData(data, elms);
    return T;
}
Property &Property::setEnum(CChar *data[], Int elms) {
    setEnum();
    combobox.setData(data, elms);
    return T;
}
/******************************************************************************/
Property &Property::setSlider() {
    _value_type = GO_SLIDER;

    slider.create().func(Changed, T, true).desc(desc()); // try re-using existing description before deleting objects

    Delete(_cp);
    Delete(_win_io);
    checkbox.del();
    textline.del();
    button.del();
    combobox.del();
    _color.del();
    return T;
}
/******************************************************************************/
Property &Property::mouseEditDel() {
    if (button._func == (Ptr)MouseEdit)
        button.del();
    return T;
}
/******************************************************************************/
Property &Property::display(C Str &name) {
    text.set(name);
    return T;
}
/******************************************************************************/
C Str &Property::desc() C {
    if (checkbox.desc().is())
        return checkbox.desc();
    if (textline.desc().is())
        return textline.desc();
    if (combobox.desc().is())
        return combobox.desc();
    if (slider.desc().is())
        return slider.desc();
    if (_color.desc().is())
        return _color.desc();
    return S;
}
Property &Property::desc(C Str &desc) {
    if (checkbox.is())
        checkbox.desc(desc);
    if (textline.is())
        textline.desc(desc);
    if (combobox.is())
        combobox.desc(desc);
    if (slider.is())
        slider.desc(desc);
    if (_color.is())
        _color.desc(desc);
    return T;
}
/******************************************************************************/
Bool Property::asBool() C {
    switch (_value_type) {
    case GO_CHECKBOX:
        return checkbox();
    case GO_TEXTLINE:
        return TextBool(textline());
    case GO_COMBOBOX:
        return combobox() > 0;
    case GO_SLIDER:
        return slider() >= 0.5f;
    default:
        return false;
    }
}
Int Property::asInt() C {
    switch (_value_type) {
    case GO_CHECKBOX:
        return checkbox();
    case GO_TEXTLINE:
        return TextInt(textline());
    case GO_COMBOBOX:
        return combobox();
    case GO_SLIDER:
        return Round(Lerp(min_use ? min_value : 0, max_use ? max_value : 1, (Dbl)slider()));
    default:
        return 0;
    }
}
Long Property::asLong() C {
    switch (_value_type) {
    case GO_CHECKBOX:
        return checkbox();
    case GO_TEXTLINE:
        return TextLong(textline());
    case GO_COMBOBOX:
        return combobox();
    case GO_SLIDER:
        return RoundL(Lerp(min_use ? min_value : 0, max_use ? max_value : 1, (Dbl)slider()));
    default:
        return 0;
    }
}
Flt Property::asFlt() C {
    switch (_value_type) {
    case GO_CHECKBOX:
        return checkbox();
    case GO_TEXTLINE:
        return TextFlt(textline());
    case GO_COMBOBOX:
        return combobox();
    case GO_SLIDER:
        return Lerp(min_use ? min_value : 0, max_use ? max_value : 1, (Dbl)slider());
    default:
        return 0;
    }
}
Dbl Property::asDbl() C {
    switch (_value_type) {
    case GO_CHECKBOX:
        return checkbox();
    case GO_TEXTLINE:
        return TextDbl(textline());
    case GO_COMBOBOX:
        return combobox();
    case GO_SLIDER:
        return Lerp(min_use ? min_value : 0, max_use ? max_value : 1, (Dbl)slider());
    default:
        return 0;
    }
}
Vec2 Property::asVec2() C {
    switch (_value_type) {
    case GO_CHECKBOX:
        return checkbox();
    case GO_TEXTLINE:
        return TextVec2(textline());
    case GO_COMBOBOX:
        return combobox();
    case GO_SLIDER:
        return Lerp(min_use ? min_value : 0, max_use ? max_value : 1, (Dbl)slider());
    default:
        return 0;
    }
}
VecD2 Property::asVecD2() C {
    switch (_value_type) {
    case GO_CHECKBOX:
        return checkbox();
    case GO_TEXTLINE:
        return TextVecD2(textline());
    case GO_COMBOBOX:
        return combobox();
    case GO_SLIDER:
        return Lerp(min_use ? min_value : 0, max_use ? max_value : 1, (Dbl)slider());
    default:
        return 0;
    }
}
Str Property::asText() C {
    switch (_value_type) {
    case GO_CHECKBOX:
        return checkbox();
    case GO_TEXTLINE:
        return textline();
    case GO_COMBOBOX:
        return combobox();
    case GO_SLIDER:
        return TextReal(Lerp(min_use ? min_value : 0, max_use ? max_value : 1, (Dbl)slider()), actualPrecision());
    case GO_CUSTOM:
        if (_cp)
            return (*_cp)();
        break;
    }
    return S;
}
/******************************************************************************/
Property &Property::set(Bool value, SET_MODE mode) {
    value = clamp(value);
    switch (_value_type) {
    case GO_CHECKBOX:
        checkbox.set(value, mode);
        break;
    case GO_TEXTLINE:
        textline.set(value, mode);
        break;
    case GO_COMBOBOX:
        combobox.set(value, mode);
        break;
    case GO_SLIDER:
        slider.set(value, mode);
        break;
    }
    return T;
}
Property &Property::set(Int value, SET_MODE mode) {
    value = clamp(value);
    switch (_value_type) {
    case GO_CHECKBOX:
        checkbox.set(value != 0, mode);
        break;
    case GO_TEXTLINE:
        textline.set(value, mode);
        break;
    case GO_COMBOBOX:
        combobox.set(value, mode);
        break;
    case GO_SLIDER:
        slider.set(LerpR(min_use ? min_value : 0, max_use ? max_value : 1, (Dbl)value), mode);
        break;
    }
    return T;
}
Property &Property::set(Long value, SET_MODE mode) {
    value = clamp(value);
    switch (_value_type) {
    case GO_CHECKBOX:
        checkbox.set(value != 0, mode);
        break;
    case GO_TEXTLINE:
        textline.set(value, mode);
        break;
    case GO_COMBOBOX:
        combobox.set(value, mode);
        break;
    case GO_SLIDER:
        slider.set(LerpR(min_use ? min_value : 0, max_use ? max_value : 1, (Dbl)value), mode);
        break;
    }
    return T;
}
Property &Property::set(Flt value, SET_MODE mode) {
    value = clamp(value);
    switch (_value_type) {
    case GO_CHECKBOX:
        checkbox.set(value != 0, mode);
        break;
    case GO_COMBOBOX:
        combobox.set(Round(value), mode);
        break;
    case GO_SLIDER:
        slider.set(LerpR(min_use ? min_value : 0, max_use ? max_value : 1, (Dbl)value), mode);
        break;

    case GO_TEXTLINE:
        switch (md.type) {
        case DATA_BOOL:
        case DATA_INT:
            textline.set(Round(value), mode);
            break;
        case DATA_UINT:
            textline.set(RoundU(value), mode);
            break;

        case DATA_REAL:
            textline.set(TextReal(value, actualPrecision()), mode);
            break;

        default:
            textline.set(value, mode);
            break;
        }
        break;
    }
    return T;
}
Property &Property::set(Dbl value, SET_MODE mode) {
    value = clamp(value);
    switch (_value_type) {
    case GO_CHECKBOX:
        checkbox.set(value != 0, mode);
        break;
    case GO_COMBOBOX:
        combobox.set(Round(value), mode);
        break;
    case GO_SLIDER:
        slider.set(LerpR(min_use ? min_value : 0, max_use ? max_value : 1, value), mode);
        break;

    case GO_TEXTLINE:
        switch (md.type) {
        case DATA_BOOL:
        case DATA_INT:
            textline.set(Round(value), mode);
            break;
        case DATA_UINT:
            textline.set(RoundU(value), mode);
            break;

        case DATA_REAL:
            textline.set(TextReal(value, actualPrecision()), mode);
            break;

        default:
            textline.set(value, mode);
            break;
        }
        break;
    }
    return T;
}
Property &Property::set(C Vec2 &value, SET_MODE mode) {
    Vec2 v(clamp(value.x), clamp(value.y));
    switch (_value_type) {
    case GO_CHECKBOX:
        checkbox.set(v.any(), mode);
        break;
    case GO_COMBOBOX:
        combobox.set(Round(v.x), mode);
        break;
    case GO_SLIDER:
        slider.set(LerpR(min_use ? min_value : 0, max_use ? max_value : 1, (Dbl)v.x), mode);
        break;

    case GO_TEXTLINE:
        switch (md.type) {
        case DATA_BOOL:
        case DATA_INT:
            textline.set(Round(v.x), mode);
            break;
        case DATA_UINT:
            textline.set(RoundU(v.x), mode);
            break;
        case DATA_VECI2:
            textline.set(Round(v), mode);
            break;

        case DATA_REAL:
            textline.set(TextReal(v.x, actualPrecision()), mode);
            break;

        default:
            textline.set(v.asText(actualPrecision()), mode);
            break;
        }
        break;
    }
    return T;
}
Property &Property::set(C VecD2 &value, SET_MODE mode) {
    VecD2 v(clamp(value.x), clamp(value.y));
    switch (_value_type) {
    case GO_CHECKBOX:
        checkbox.set(v.any(), mode);
        break;
    case GO_COMBOBOX:
        combobox.set(Round(v.x), mode);
        break;
    case GO_SLIDER:
        slider.set(LerpR(min_use ? min_value : 0, max_use ? max_value : 1, v.x), mode);
        break;

    case GO_TEXTLINE:
        switch (md.type) {
        case DATA_BOOL:
        case DATA_INT:
            textline.set(RoundL(v.x), mode);
            break;
        case DATA_UINT:
            textline.set(RoundUL(v.x), mode);
            break;
        case DATA_VECI2:
            textline.set(Round(v), mode);
            break;

        case DATA_REAL:
            textline.set(TextReal(v.x, actualPrecision()), mode);
            break;

        default:
            textline.set(v.asText(actualPrecision()), mode);
            break;
        }
        break;
    }
    return T;
}
Property &Property::set(C Str &value, SET_MODE mode) {
    switch (_value_type) {
    case GO_CHECKBOX:
        checkbox.set(clamp(TextBool(value)), mode);
        break;
    case GO_COMBOBOX:
        combobox.set(clamp(TextInt(value)), mode);
        break;
    case GO_SLIDER:
        slider.set(LerpR(min_use ? min_value : 0, max_use ? max_value : 1, TextDbl(value)), mode);
        break; // no need for 'clamp' because we already use min/max here and 'Slider.set' will saturate the value

    case GO_TEXTLINE:
        switch (md.type) {
        case DATA_BOOL:
            textline.set(clamp(TextBool(value)), mode);
            break;

        case DATA_INT:
            if (md.size == SIZE(Long))
                textline.set(clamp(TextLong(value)), mode);
            else
                textline.set(clamp(TextInt(value)), mode);
            break;
        case DATA_UINT:
            if (md.size == SIZE(ULong))
                textline.set(clamp(TextULong(value)), mode);
            else
                textline.set(clamp(TextUInt(value)), mode);
            break;
        case DATA_REAL:
            textline.set(TextReal(clamp(TextDbl(value)), actualPrecision()), mode);
            break;

        default:
            textline.set(value, mode);
            break;
        }
        break;

    case GO_CUSTOM:
        if (_cp)
            _cp->set(TextVec4(value), mode);
        break;
    }
    return T;
}
/******************************************************************************/
Bool Property::contains(C GuiObj *child) C {
    if (child)
        return text.contains(child) || checkbox.contains(child) || textline.contains(child) || button.contains(child) || combobox.contains(child) || slider.contains(child) || _color.contains(child);
    return false;
}
/******************************************************************************/
void Property::fromGui(Ptr object) {
    switch (_value_type) {
    case GO_CHECKBOX:
        md.fromBool(object, checkbox());
        break;
    case GO_COMBOBOX:
        md.fromInt(object, combobox());
        break;
    case GO_SLIDER:
        md.fromDbl(object, Lerp(min_use ? min_value : 0, max_use ? max_value : 1, (Dbl)slider()));
        break;
    case GO_CUSTOM:
        if (_cp)
            md.fromColor(object, (*_cp)());
        break;
    case GO_TEXTLINE:
        switch (md.type) {
        case DATA_INT:
            if (md.size == SIZE(Long))
                md.fromLong(object, clamp(TextLong(textline())));
            else
                md.fromInt(object, clamp(TextInt(textline())));
            break;
        case DATA_UINT:
            if (md.size == SIZE(ULong))
                md.fromULong(object, clamp(TextULong(textline())));
            else
                md.fromUInt(object, clamp(TextUInt(textline())));
            break;
        case DATA_REAL:
            md.fromDbl(object, clamp(TextDbl(textline())));
            break;
        default:
            md.fromText(object, textline());
            break;
        }
        break;
    }
}
void Property::toGui(CPtr object) {
    switch (_value_type) {
    case GO_CHECKBOX:
        checkbox.set(asBool(object), QUIET);
        break;
    case GO_TEXTLINE:
        textline.set(asText(object), QUIET);
        break;
    case GO_COMBOBOX:
        combobox.set(asEnum(object), QUIET);
        break;
    case GO_SLIDER:
        slider.set(LerpR(min_use ? min_value : 0, max_use ? max_value : 1, asDbl(object)), QUIET);
        break;
    case GO_CUSTOM:
        if (_cp)
            _cp->set(md.asColor(object), QUIET);
        break;
    }
}
void Property::toGui() {
    toGui(autoData());
}
Property &Property::hide() {
    if (visible()) {
        text.hide();
        checkbox.hide();
        textline.hide();
        button.hide();
        combobox.hide();
        slider.hide();
        _color.hide();
    }
    return T;
}
Property &Property::show() {
    if (!visible()) {
        text.show();
        checkbox.show();
        textline.show();
        button.show();
        combobox.show();
        slider.show();
        _color.show();
    }
    return T;
}
Property &Property::visible(Bool on) { return on ? show() : hide(); }
Bool Property::visible() C { return text.visible(); }
Property &Property::visibleToggle() { return text.visible() ? hide() : show(); }
/******************************************************************************/
Property &Property::moveValue(C Vec2 &delta) {
    if (delta.any()) {
        checkbox.move(delta);
        textline.move(delta);
        button.move(delta);
        combobox.move(delta);
        slider.move(delta);
        _color.move(delta);
    }
    return T;
}
Property &Property::move(C Vec2 &delta) {
    if (delta.any()) {
        text.move(delta);
        moveValue(delta);
    }
    return T;
}
Property &Property::pos(C Vec2 &pos) { return move(pos - text.pos()); }
Property &Property::close() {
    if (_cp)
        _cp->hide();
    return T;
}
Property &Property::enabled(Bool enabled) {
    text.enabled(enabled);
    checkbox.enabled(enabled);
    textline.enabled(enabled);
    button.enabled(enabled);
    combobox.enabled(enabled);
    slider.enabled(enabled);
    _color.enabled(enabled);
    if (!enabled)
        close();
    return T;
}
Property &Property::parent(GuiObj &parent) {
    parent += text;
    parent += checkbox;
    parent += textline;
    parent += button;
    parent += combobox;
    parent += slider;
    parent += _color;
    return T;
}
/******************************************************************************/
static void LoadFile(C Str &name, Property &prop) {
    prop.textline.set(SkipStartPath(name, DataPath()));
    if (prop._win_io)
        prop._win_io->del();
}
void Property::SelectFile(Property &prop) {
    if (!prop._win_io)
        New(prop._win_io);
    if (!prop._win_io->is()) {
        prop._win_io->create(S, S, DataPath(), LoadFile, LoadFile, prop).ext(prop._win_io_ext, prop._win_io_ext_desc);
        Str path = GetPath(prop.asText());
        if (FExistSystem(path))
            prop._win_io->path(S, path);
        else if (FExistSystem(DataPath() + path))
            prop._win_io->path(S, DataPath() + path);
    }
    if (GuiObj *win = prop.textline.first(GO_WINDOW))
        prop._win_io->level(win->asWindow().level() + 1);
    prop._win_io->load();
}
void Property::MouseEdit(Property &prop) {
    // get total delta movement (and number of start touches)
    Vec2 d = 0;
    Int on = 0, pd = 0;
    REPA(MT)
    if (MT.b(i) && MT.guiObj(i) == &prop.button) {
        d += MT.ad(i);
        if (Touch *touch = MT.touch(i))
            touch->disableScroll();
        else
            Ms.freeze();
        if (MT.bp(i))
            pd++;
        else
            on++;
    }

    if (pd && !on)
        switch (prop.md.type) // on start set initial value
        {
        case DATA_REAL:
        case DATA_INT:
        case DATA_UINT:
        case DATA_VEC2:
        case DATA_VECI2:
            prop._mouse_edit_value = (prop._auto_data ? prop.asVecD2(prop._auto_data) : prop.asVecD2());
            break; // use '_auto_data' if possible to get precise value (also this allows functionality if textline has "garbage" like "<multiple values>")
        }

    if (d.any()) {
        d *= prop.mouse_edit_speed;
        Int dim = ((prop.md.type == DATA_VEC2 || prop.md.type == DATA_VECI2) ? 2 : 1);
        Bool dim2 = (dim == 2);
        if (!dim2 || Kb.ctrlCmd() && !Kb.shift())
            d.set(d.sum(), 0);
        else if (!Kb.ctrlCmd() && Kb.shift())
            d.set(0, d.sum());
        else if ((Kb.ctrlCmd() && Kb.shift() || (Gui.ms() == &prop.button && Ms.b(1))) ^ prop.mouse_edit_linked)
            d.set(d.sum());
        switch (prop.md.type) {
        case DATA_INT:
        case DATA_UINT:
        case DATA_REAL:
        case DATA_VEC2:
        case DATA_VECI2: {
            switch (prop.mouse_edit_mode) {
            case PROP_MOUSE_EDIT_SCALAR: {
                Int precision = Abs(prop.actualPrecision());
                Dbl min = (precision ? Pow(0.1, precision) : 1.0 / 3); // 1/3 was evaluated to match mouse movement needed from 0->1 and 1->2
                if (Equal(prop._mouse_edit_value.x, 0, min))
                    prop._mouse_edit_value.x = min;
                if (Equal(prop._mouse_edit_value.y, 0, min))
                    prop._mouse_edit_value.y = min;
                prop._mouse_edit_value *= VecD2(ScaleFactor(d.x), ScaleFactor(d.y));
            } break;
            case PROP_MOUSE_EDIT_LINEAR: {
                prop._mouse_edit_value += d;
            } break;
            }
            prop._mouse_edit_value.x = prop.clamp(prop._mouse_edit_value.x);
            prop._mouse_edit_value.y = prop.clamp(prop._mouse_edit_value.y);
            prop.set(prop._mouse_edit_value);
        } break;
        }
    }
}
Bool Property::clamp(Bool value) {
    Int v = value;
    if (min_use)
        MAX(v, min_value);
    if (max_use)
        MIN(v, max_value);
    return v != 0;
}
Int Property::clamp(Int value) {
    if (min_use)
        MAX(value, min_value);
    if (max_use)
        MIN(value, max_value);
    return value;
}
UInt Property::clamp(UInt value) {
    if (min_use)
        MAX(value, min_value);
    if (max_use)
        MIN(value, max_value);
    return value;
}
Long Property::clamp(Long value) {
    if (min_use)
        MAX(value, min_value);
    if (max_use)
        MIN(value, max_value);
    return value;
}
ULong Property::clamp(ULong value) {
    if (min_use)
        MAX(value, min_value);
    if (max_use)
        MIN(value, max_value);
    return value;
}
Flt Property::clamp(Flt value) {
    if (min_use)
        MAX(value, min_value);
    if (max_use)
        MIN(value, max_value);
    return value;
}
Dbl Property::clamp(Dbl value) {
    if (min_use)
        MAX(value, min_value);
    if (max_use)
        MIN(value, max_value);
    return value;
}
/******************************************************************************/
Property &Property::autoData(Ptr object) {
    if (T._auto_data != object) {
        T._auto_data = object;
        if (object)
            toGui(object);
    }
    return T;
}
Property &Property::changed(void (*changed)(C Property &prop), void (*pre_changed)(C Property &prop)) {
    T._changed = changed;
    T._pre_changed = pre_changed;
    return T;
}
Property &Property::funcImmediate(Bool immediate) {
    checkbox.funcImmediate(immediate);
    textline.funcImmediate(immediate);
    button.funcImmediate(immediate);
    combobox.funcImmediate(immediate);
    slider.funcImmediate(immediate);
    if (_cp)
        _cp->funcImmediate(immediate);
    return T;
}
static const Flt NameValueSpace = 0.01f;
Rect Property::addTo(GuiObj &parent, C Vec2 &pos, Flt text_width, Flt height, Flt value_width) {
    Flt value_height = height * 0.92f;
    Vec2 p = pos;
    p.y -= 0.5f * height;

    // name
    text.rect(p + Vec2(Lerp(text_width * 0.5f, 0.0f, text.text_style ? text.text_style->align.x : 0), 0));
    parent += text;
    p.x += text_width + NameValueSpace;

    // checkbox
    if (checkbox.is()) {
        parent += checkbox.rect(Rect_L(p, value_height, value_height));
        p.x += checkbox.rect().w();
    }

    // textline
    if (textline.is()) {
        parent += textline.rect(Rect_L(p, value_width - value_height, value_height));
        p.x += textline.rect().w();
    }

    // button
    if (button.is()) {
        parent += button.rect(Rect_L(p, value_height, value_height));
        p.x += button.rect().w();
    }

    // combobox
    if (combobox.is()) {
        parent += combobox.rect(Rect_L(p, value_width, value_height));
        p.x += combobox.rect().w();
    }

    // slider
    if (slider.is()) {
        parent += slider.rect(Rect_L(p, value_width, value_height));
        p.x += slider.rect().w();
    }

    // color
    if (_color.is()) {
        parent += _color.rect(Rect_L(p, value_height, value_height));
        p.x += _color.rect().w();
        if (_cp)
            _cp->md = md;
    }
    return Rect(pos.x, pos.y - height, p.x, pos.y);
}
/******************************************************************************/
// MAIN
/******************************************************************************/
Rect AddProperties(Memx<Property> &properties, GuiObj &parent, C Vec2 &left_up, Flt property_height, Flt value_width, C TextStylePtr &text_style, C Flt *parent_width) {
    TextStylePtr ts = text_style;
    if (!ts && Gui.skin)
        ts = Gui.skin->text.text_style;
    Flt text_width = 0,
        right = left_up.x;
    FREPA(properties) {
        Property &prop = properties[i];
        prop.text.text_style = ts;
        MAX(text_width, prop.text.textWidthLine());
    }
    if (parent_width)
        MIN(value_width, *parent_width - left_up.x - text_width - NameValueSpace);
    FREPA(properties) {
        Property &prop = properties[i];
        Vec2 pos = left_up;
        pos.y -= i * property_height;
        MAX(right, prop.addTo(parent, pos, text_width, property_height, value_width).max.x);
    }
    return Rect(left_up.x, left_up.y - properties.elms() * property_height, right, left_up.y);
}
/******************************************************************************/
void SaveProperties(C Memx<Property> &properties, MemPtr<TextNode> nodes, Char space_replacement, Bool handle_same_names) {
    FREPA(properties) {
        C Property &prop = properties[i];
        if (prop._value_type) {
            C Str &name = prop.name;
            if (name.is()) {
                Int same_names = 0;
                if (handle_same_names)
                    REPD(j, i) {
                        C Property &p = properties[j];
                        if (p._value_type && Equal(p.name, name))
                            same_names++;
                    }
                Str temp = name;
                temp.replace(' ', space_replacement);
                if (same_names) {
                    temp += '@';
                    temp += same_names;
                }
                GetNode(nodes, temp).value = prop.asText();
            }
        }
    }
}
void LoadProperties(Memx<Property> &properties, C CMemPtr<TextNode> &nodes, Char space_replacement, Bool handle_same_names) {
    FREPA(properties) {
        Property &prop = properties[i];
        if (prop._value_type) {
            C Str &name = prop.name;
            if (name.is()) {
                Int same_names = 0;
                if (handle_same_names)
                    REPD(j, i) {
                        C Property &p = properties[j];
                        if (p._value_type && Equal(p.name, name))
                            same_names++;
                    }
                Str temp = name;
                temp.replace(' ', space_replacement);
                if (same_names) {
                    temp += '@';
                    temp += same_names;
                }
                if (C TextNode *node = CFindNode(nodes, temp))
                    prop.set(node->value, NO_SOUND);
            }
        }
    }
}
/******************************************************************************/
void SaveProperties(C Memx<Property> &properties, XmlNode &node, Char space_replacement) {
    if (space_replacement == ' ')
        space_replacement = '\0'; // can't allow spaces in XML
    FREPA(properties) {
        C Property &prop = properties[i];
        if (prop._value_type && prop.name.is()) {
            Str param_name = Replace(prop.name, ' ', space_replacement);
            if (param_name.is())
                node.getParam(param_name).value = prop.asText();
        }
    }
}
void LoadProperties(Memx<Property> &properties, C XmlNode &node, Char space_replacement) {
    if (space_replacement == ' ')
        space_replacement = '\0'; // can't allow spaces in XML
    FREPA(properties) {
        Property &prop = properties[i];
        if (prop._value_type && prop.name.is()) {
            Str param_name = Replace(prop.name, ' ', space_replacement);
            if (param_name.is())
                if (C XmlParam *param = node.findParam(param_name))
                    prop.set(param->value, NO_SOUND);
        }
    }
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
