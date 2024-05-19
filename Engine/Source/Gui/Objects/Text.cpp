/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
void Text::zero() {
    auto_line = false;
}
Text::Text() { zero(); }
Text &Text::del() {
    text.del();
    extra.del();
    text_style.clear();
    skin.clear();
    super::del();
    zero();
    return T;
}
Text &Text::create(C Str &text, C TextStylePtr &text_style) {
    del();

    _type = GO_TEXT;
    _visible = true;
    T.text = text;
    T.text_style = text_style;

    return T;
}
Text &Text::create(C Text &src) {
    if (this != &src) {
        if (!src.is())
            del();
        else {
            copyParams(src);
            _type = GO_TEXT;
            auto_line = src.auto_line;
            text_style = src.text_style;
            skin = src.skin;
            text = src.text;
            extra = src.extra;
        }
    }
    return T;
}
/******************************************************************************/
Text &Text::clear() {
    text.del();
    extra.del();
    return T;
}
Text &Text::set(C Str &text) {
    if (is()) // only if created, this prevents modifying values after destructor was already called
    {
        T.text = text;
        extra.del();
    }
    return T;
}
/******************************************************************************/
TextStyle *Text::getTextStyle() C {
    if (text_style)
        return text_style();
    if (GuiSkin *skin = getSkin())
        return skin->text.text_style();
    return null;
}
Flt Text::textWidthLine() C {
    if (TextStyle *text_style = getTextStyle()) {
#if DEFAULT_FONT_FROM_CUSTOM_SKIN
        TextStyleParams ts = *text_style;
        if (!ts.font())
            if (GuiSkin *skin = getSkin())
                ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
#else
        C TextStyle &ts = *text_style;
#endif
        return ts.textWidth(text, extra.data(), extra.elms());
    }
    return 0;
}
Int Text::textLines(C Flt *width) C {
    if (TextStyle *text_style = getTextStyle()) {
#if DEFAULT_FONT_FROM_CUSTOM_SKIN
        TextStyleParams ts = *text_style;
        if (!ts.font())
            if (GuiSkin *skin = getSkin())
                ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
#else
        C TextStyle &ts = *text_style;
#endif

        return ts.textLines(text, extra.data(), extra.elms(), width ? *width : rect().w(), auto_line);
    }
    return 0;
}
Flt Text::textHeight(C Flt *width) C {
    if (TextStyle *text_style = getTextStyle())
        return textLines(width) * text_style->lineHeight();
    return 0;
}
Vec2 Text::textSize() C {
    if (TextStyle *text_style = getTextStyle()) {
#if DEFAULT_FONT_FROM_CUSTOM_SKIN
        TextStyleParams ts = *text_style;
        if (!ts.font())
            if (GuiSkin *skin = getSkin())
                ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
#else
        C TextStyle &ts = *text_style;
#endif

        Vec2 size;
        size.y = ts.textHeight(text, extra.data(), extra.elms(), rect().w(), auto_line, &size.x);
        return size;
    }
    return 0;
}
Int Text::textIndex(C Vec2 &screen_pos, TEXT_INDEX_MODE index_mode) C {
    if (TextStyle *text_style = getTextStyle()) {
#if DEFAULT_FONT_FROM_CUSTOM_SKIN
        TextStyleParams ts = *text_style;
        if (!ts.font())
            if (GuiSkin *skin = getSkin())
                ts.font(skin->font()); // adjust font in case it's empty and the custom skin has a different font than the 'Gui.skin'
#else
        C TextStyle &ts = *text_style;
#endif

        Vec2 local = screen_pos - screenPos();
        Bool eol;
        return ts.textIndexAlign(text, extra.data(), extra.elms(), local.x, local.y, index_mode, rect().size(), auto_line, false, eol);
    }
    return 0;
}
/******************************************************************************/
void Text::draw(C GuiPC &gpc) {
    if (/*gpc.visible &&*/ visible() && hasData())
        if (TextStyle *text_style = getTextStyle()) {
            Rect rect = T.rect() + gpc.offset;
            D.clip(gpc.clip);

#if DEFAULT_FONT_FROM_CUSTOM_SKIN
            GuiSkin *skin;
            if (!text_style->font() && (skin = getSkin())) // adjust font in case it's empty and the custom skin has a different font than the Gui.skin
            {
                TextStyleParams ts = *text_style;
                ts.font(skin->font());
                D.text(ts, rect, text, extra.data(), extra.elms(), auto_line);
            } else
#endif
                D.text(*text_style, rect, text, extra.data(), extra.elms(), auto_line, &NoTemp(T.rect().w()));
        }
}
/******************************************************************************/
struct TextCodeData {
    enum MODE : Byte {
        PREV,
        NEW,
        OLD,
        DEFAULT,
    };
    MODE shadow_mode, color_mode, nocode_mode;
    Byte shadow;
    Color color;
    CPtr pos;
};
static Str GetTextCode(C Str &text, C TextCodeData *code, Int codes) {
    Str out;
    Int cur_code = 0, colors = 0, shadows = 0, nocodes = 0;
    for (Int i = 0;; i++) {
        for (; InRange(cur_code, codes) && text() + i >= code[cur_code].pos;) {
            C TextCodeData &c = code[cur_code++];

            switch (c.color_mode) {
            case TextCodeData::NEW:
                out += "[color=";
                out += c.color.asHex();
                out += ']';
                colors++;
                break;

            case TextCodeData::DEFAULT:
                if (!colors)
                    break; // on purpose without break
            case TextCodeData::OLD:
                colors--;
                out += "[/color]";
                break;
            }

            switch (c.shadow_mode) {
            case TextCodeData::NEW:
                out += "[shadow=";
                out += TextHex(U32(c.shadow), 2);
                out += ']';
                shadows++;
                break;

            case TextCodeData::DEFAULT:
                if (!shadows)
                    break; // on purpose without break
            case TextCodeData::OLD:
                shadows--;
                out += "[/shadow]";
                break;
            }

            switch (c.nocode_mode) {
            case TextCodeData::NEW:
                out += "[nocode]";
                nocodes++;
                break;

            case TextCodeData::DEFAULT:
                if (!nocodes)
                    break; // on purpose without break
            case TextCodeData::OLD:
                nocodes--;
                out += "[/nocode]";
                break;
            }
        }
        if (i >= text.length())
            break;
        out += text[i];
    }
    return out;
}
/******************************************************************************/
Bool Text::save(File &f, CChar *path) C {
    if (super::save(f, path)) {
        f.putMulti(Byte(7), auto_line) << text; // version
        extra.save(f, path);
        f.putAsset(text_style.id());
        f.putAsset(skin.id());
        return f.ok();
    }
    return false;
}
Bool Text::load(File &f, CChar *path) {
    del();
    if (super::load(f, path))
        switch (f.decUIntV()) // version
        {
        case 7: {
            _type = GO_TEXT;

            f >> auto_line >> text;
            if (extra.load(f, path)) {
                text_style.require(f.getAssetID(), path);
                skin.require(f.getAssetID(), path);
                if (f.ok())
                    return true;
            }
        } break;

        case 6: {
            _type = GO_TEXT;

            f >> auto_line >> text;
            text_style.require(f.getAssetID(), path);
            skin.require(f.getAssetID(), path);
            Mems<TextCodeData> codes;
            codes.setNum(f.decUIntV());
            FREPA(codes) {
                auto &c = codes[i];
                UInt pos;
                f.getMulti(c.shadow_mode, c.color_mode, c.nocode_mode, c.shadow, c.color, pos);
                c.pos = Ptr(text() + pos);
            }
            if (f.ok())
                return true;
        } break;

        case 5: {
            _type = GO_TEXT;

            f >> auto_line;
            f._getStr1(text);
            text_style.require(f._getAsset(), path);
            skin.require(f._getAsset(), path);
            Mems<TextCodeData> codes;
            codes.setNum(f.decUIntV());
            FREPA(codes) {
                auto &c = codes[i];
                UInt pos;
                f.getMulti(c.shadow_mode, c.color_mode, c.nocode_mode, c.shadow, c.color, pos);
                c.pos = Ptr(text() + pos);
            }
            if (f.ok())
                return true;
        } break;

        case 4: {
            _type = GO_TEXT;

            f >> auto_line;
            f._getStr(text);
            text_style.require(f._getStr(), path);
            f._getStr();
            Mems<TextCodeData> codes;
            codes.setNum(f.getInt());
            FREPA(codes) {
                auto &c = codes[i];
                c.shadow_mode = TextCodeData::MODE(f.getByte());
                c.color_mode = TextCodeData::MODE(f.getByte());
                c.nocode_mode = TextCodeData::MODE(f.getByte());
                f >> c.shadow >> c.color;
                c.pos = Ptr(text() + f.getUInt());
            }
            if (f.ok())
                return true;
        } break;

        case 3: {
            _type = GO_TEXT;

            f >> auto_line;
            f._getStr(text);
            text_style.require(f._getStr(), path);
            f._getStr();
            Mems<TextCodeData> codes;
            codes.setNum(f.getInt());
            FREPA(codes) {
                auto &c = codes[i];
                c.shadow_mode = TextCodeData::MODE(f.getByte());
                f.skip(3);
                c.color_mode = TextCodeData::MODE(f.getByte());
                f.skip(3);
                c.nocode_mode = TextCodeData::DEFAULT;
                f >> c.shadow >> c.color;
                c.pos = Ptr(text() + f.getUInt());
            }
            if (f.ok())
                return true;
        } break;

        case 2: {
            _type = GO_TEXT;

            f >> auto_line;
            f._getStr(text);
            text_style.require(f._getStr(), path);
            f._getStr();
            if (f.ok())
                return true;
        } break;

        case 1: {
            _type = GO_TEXT;

            text = f._getStr16();
            text_style.require(f._getStr16(), path);
            f._getStr16();
            if (f.ok())
                return true;
        } break;

        case 0: {
            _type = GO_TEXT;

            text = f._getStr8();
            text_style.require(f._getStr8(), path);
            f._getStr8();
            if (f.ok())
                return true;
        } break;
        }
    del();
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
