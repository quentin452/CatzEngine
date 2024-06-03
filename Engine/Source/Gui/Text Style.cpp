/******************************************************************************/
#include "../../stdafx.h"
namespace EE {
/******************************************************************************/
#define CC4_TXDS CC4('T', 'X', 'D', 'S') // TextStyle (formerly TextDrawSettings)
#define DefaultShade 230
static const Color DefaultSelectionColor(51, 153, 255, 64);
DEFINE_CACHE(TextStyle, TextStyles, TextStylePtr, "Text Style");
/******************************************************************************/
#if SUPPORT_EMOJI
struct EmojiKey {
    union {
        UInt u;
        Char c[2];
    };

    EmojiKey() {}
    EmojiKey(Char c) { u = c; }
    void append(Char c) { T.c[1] = c; }
};
struct Emoji {
    ImagePtr img;
};
static Int Compare(C EmojiKey &a, C EmojiKey &b) { return Compare(a.u, b.u); }
static Bool Create(Emoji &emoji, C EmojiKey &key, Ptr user) {
    Char name[3];
    name[0] = key.c[0];
    name[1] = key.c[1];
    name[2] = '\0';
    if (EmojiPak && EmojiPak->find(name, false))
        emoji.img = name; // load only if in 'EmojiPak', in that case require
    return true;          // always succeed to avoid loading again and again the same emoji on fail, in that case just have null image
}
static Map<EmojiKey, Emoji> Emojis(Compare, Create);
static inline C ImagePtr &FindEmoji(C EmojiKey &key) {
    C Emoji *emoji = Emojis.get(key);
    // if(emoji) always succeeds so can skip "if"
    return emoji->img;
    return ImageNull;
}
#endif
/******************************************************************************/
static Bool Separatable(Char c) {
    return Unsigned(c) >= 3585; // starting from Thai, Korean=0x1100, see 'LanguageSpecific', careful this range also includes German 'ẞ'
}
/******************************************************************************/
void DrawKeyboardCursor(C Vec2 &pos, Flt height) {
    ALPHA_MODE alpha = D.alpha(ALPHA_INVERT);
    Rect_U(pos, height / 11.0f, height).draw(WHITE);
    D.alpha(alpha);
}
void DrawKeyboardCursorOverwrite(C Vec2 &pos, Flt height, Flt width) {
    const Flt min_w = height / 5.0f; // use min width because some characters are just too thin
    ALPHA_MODE alpha = D.alpha(ALPHA_INVERT);
    Rect_LU r(pos, width, height);
    if (min_w > width)
        r.extendX((min_w - width) * 0.5f);
    r.draw(WHITE); // extend both left and right
    D.alpha(alpha);
}
/******************************************************************************/
// STR DATA
/******************************************************************************/
Bool StrData::visible() C {
    switch (type) {
    case TEXT:
        return text.is();
    case IMAGE:
        return image != null;
    case PANEL:
        return panel != null;
    }
    return false;
}
Bool StrData::operator==(C StrData &x) C {
    if (type != x.type)
        return false;
    switch (type) {
    case TEXT:
        return Equal(text, x.text, true);
    case IMAGE:
        return image == x.image;
    case COLOR:
        return color == x.color;
    case SHADOW:
        return shadow == x.shadow;
    case FONT:
        return font == x.font;
    case PANEL:
        return panel == x.panel;
    }
    return true;
}
void StrData::del() {
    switch (type) {
    case TEXT:
        DTOR(text);
        break;
    case IMAGE:
        DTOR(image);
        break;
    case FONT:
        DTOR(font);
        break;
    case PANEL:
        DTOR(panel);
        break;
    }
    type = NONE;
}
StrData &StrData::create(TYPE type) {
    if (T.type != type) {
        del();
        switch (T.type = type) {
        case TEXT:
            CTOR(text);
            break;
        case IMAGE:
            CTOR(image);
            break;
        case FONT:
            CTOR(font);
            break;
        case PANEL:
            CTOR(panel);
            break;
        }
    }
    return T;
}
void StrData::operator=(C StrData &src) {
    if (this != &src) {
        create(src.type);
        switch (type) {
        case TEXT:
            text = src.text;
            break;
        case IMAGE:
            image = src.image;
            break;
        case COLOR:
            color = src.color;
            break;
        case SHADOW:
            shadow = src.shadow;
            break;
        case FONT:
            font = src.font;
            break;
        case PANEL:
            panel = src.panel;
            break;
        }
    }
}
/******************************************************************************/
static Int Length(C StrData *data, Int datas) {
    Int l = 0;
    if (datas > 0) {
    next:
        switch (data->type) {
        case StrData::TEXT:
            l += data->text.length();
            break;
        case StrData::IMAGE:
            l++;
            break;
        }
        if (datas > 1) {
            datas--;
            data++;
            goto next;
        }
    }
    return l;
}
/******************************************************************************/
// STR EX
/******************************************************************************/
Int StrEx::length() C { return Length(data(), elms()); }
Int StrEx::strLength() C {
    Int l = 0;
    FREPA(T) {
        C StrData &d = T[i];
        if (d.type == StrData::TEXT)
            l += d.text.length();
    }
    return l;
}
Str StrEx::str() C {
    Str s;
    FREPA(T) {
        C StrData &d = T[i];
        if (d.type == StrData::TEXT)
            s += d.text;
    }
    return s;
}
void StrEx::operator=(C Str &text) {
    if (text.is())
        setNum(1).first().create(StrData::TEXT).text = text;
    else
        clear();
}

StrEx &StrEx::text(Char chr) {
    if (chr) {
        if (elms()) {
            StrData &last = T.last();
            if (last.type == StrData::TEXT) {
                last.text += chr;
                return T;
            }
        }
        New().create(StrData::TEXT).text = chr;
    }
    return T;
}
StrEx &StrEx::text(Char8 chr) {
    if (chr) {
        if (elms()) {
            StrData &last = T.last();
            if (last.type == StrData::TEXT) {
                last.text += chr;
                return T;
            }
        }
        New().create(StrData::TEXT).text = chr;
    }
    return T;
}
StrEx &StrEx::text(CChar *text) {
    if (Is(text)) {
        if (elms()) {
            StrData &last = T.last();
            if (last.type == StrData::TEXT) {
                last.text += text;
                return T;
            }
        }
        New().create(StrData::TEXT).text = text;
    }
    return T;
}
StrEx &StrEx::text(CChar8 *text) {
    if (Is(text)) {
        if (elms()) {
            StrData &last = T.last();
            if (last.type == StrData::TEXT) {
                last.text += text;
                return T;
            }
        }
        New().create(StrData::TEXT).text = text;
    }
    return T;
}
StrEx &StrEx::text(C Str &text) {
    if (text.is()) {
        if (elms()) {
            StrData &last = T.last();
            if (last.type == StrData::TEXT) {
                last.text += text;
                return T;
            }
        }
        New().create(StrData::TEXT).text = text;
    }
    return T;
}
StrEx &StrEx::text(C Str8 &text) {
    if (text.is()) {
        if (elms()) {
            StrData &last = T.last();
            if (last.type == StrData::TEXT) {
                last.text += text;
                return T;
            }
        }
        New().create(StrData::TEXT).text = text;
    }
    return T;
}
void StrEx::operator+=(C StrEx &str) { // Warning: 'str' might be 'this'
    Int elms = str.elms();             // copy first in case 'str' is 'this'
    FREP(elms) {
        StrData &d = New(); // allocate first, this might change memory address, important if 'str' is 'this'
        d = str[i];         // copy from 'str'
    }
}

StrEx &StrEx::image(C ImagePtr &image) {
    New().create(StrData::IMAGE).image = image;
    return T;
}
StrEx &StrEx::color(C Color &color) {
    New().create(StrData::COLOR).color = color;
    return T;
}
StrEx &StrEx::shadow(Byte shadow) {
    New().create(StrData::SHADOW).shadow = shadow;
    return T;
}
StrEx &StrEx::color(C Color *color) {
    if (!color)
        New().create(StrData::COLOR_OFF);
    else
        T.color(*color);
    return T;
}
StrEx &StrEx::shadow(C Byte *shadow) {
    if (!shadow)
        New().create(StrData::SHADOW_OFF);
    else
        T.shadow(*shadow);
    return T;
}
StrEx &StrEx::font(C FontPtr &font) {
    New().create(StrData::FONT).font = font;
    return T;
}
StrEx &StrEx::panel(C PanelImagePtr &panel) {
    New().create(StrData::PANEL).panel = panel;
    return T;
}

StrEx &StrEx::panelText(C PanelImagePtr &panel, C Str &text) {
    T.panel(panel);
    T += text;
    T.panel(null);
    return T;
}
StrEx &StrEx::panelImage(C PanelImagePtr &panel, C ImagePtr &image) {
    T.panel(panel);
    T += image;
    T.panel(null);
    return T;
}

StrEx &StrEx::space() {
    REPA(T) {
        StrData &d = T[i];
        switch (d.type) {
        case StrData::TEXT:
            if (d.text.is()) {
                if (!WhiteChar(d.text.last()))
                ins:
                    T += ' ';
                return T;
            }
            break; // process only if not empty
        case StrData::IMAGE:
            goto ins;
        }
    }
    return T;
}
StrEx &StrEx::nbsp() {
    REPA(T) {
        StrData &d = T[i];
        switch (d.type) {
        case StrData::TEXT:
            if (d.text.is()) {
                if (!WhiteChar(d.text.last()))
                ins:
                    T += Nbsp;
                return T;
            }
            break; // process only if not empty
        case StrData::IMAGE:
            goto ins;
        }
    }
    return T;
}
StrEx &StrEx::line() {
    REPA(T) {
        StrData &d = T[i];
        switch (d.type) {
        case StrData::TEXT:
            if (d.text.is()) {
                if (d.text.last() != '\n')
                ins:
                    T += '\n';
                return T;
            }
            break; // process only if not empty
        case StrData::IMAGE:
            goto ins;
        }
    }
    return T;
}
/******************************************************************************/
Bool StrEx::save(File &f, CChar *path) C {
    f.cmpUIntV(0); // version
    f.cmpUIntV(elms());
    FREPA(T) {
        C StrData &d = T[i];
        f << d.type;
        switch (d.type) {
        case StrData::COLOR:
            f << d.color;
            break;
        case StrData::SHADOW:
            f << d.shadow;
            break;
        case StrData::TEXT:
            f << d.text;
            break;
        case StrData::IMAGE:
            f.putAsset(d.image.id());
            break;
        case StrData::PANEL:
            f.putAsset(d.panel.id());
            break;
        case StrData::FONT:
            f.putAsset(d.font.id());
            break;
        }
    }
    return f.ok();
}
Bool StrEx::load(File &f, CChar *path) {
    switch (f.decUIntV()) // version
    {
    case 0: {
        setNumDiscard(f.decUIntV());
        FREPA(T) {
            StrData::TYPE type;
            f >> type;
            if (!InRange(type, StrData::NUM))
                goto error;
            StrData &d = T[i];
            d.create(type);
            switch (d.type) {
            case StrData::COLOR:
                f >> d.color;
                break;
            case StrData::SHADOW:
                f >> d.shadow;
                break;
            case StrData::TEXT:
                f >> d.text;
                break;
            case StrData::IMAGE:
                d.image.require(f.getAssetID(), path);
                break;
            case StrData::PANEL:
                d.panel.require(f.getAssetID(), path);
                break;
            case StrData::FONT:
                d.font.require(f.getAssetID(), path);
                break;
            }
        }
        if (f.ok())
            return true;
    } break;
    }
error:
    del();
    return false;
}
static Bool VisibleData(C StrData *data, Int datas) {
    REP(datas)
    if (data[i].visible())
        return true;
    return false;
}
static Bool PanelClosing(C StrData *data, Int datas) // if there are no visible elements (no text, images) until the panel (or data) ends
{
next:
    if (datas <= 0)
        return true; // reached the end = close
    switch (data->type) {
    case StrData::IMAGE:
        return false; // there's a visible element
    case StrData::TEXT:
        if (data->text.is())
            return false;
        break; // there's a visible element

    case StrData::PANEL:
        return true; // reached panel = close
    }
    data++;
    datas--;
    goto next;
}
/******************************************************************************/
struct TextSrc {
    CChar8 *t8;
    CChar *t16;

    Bool is() C { return Is(t8) || Is(t16); }

    void fix() {
        if (!t16 && !t8)
            t16 = u"";
    } // don't allow null so we can assume "t8 || t16" in 'c' 'n', prefer 't16' to avoid 'Char8To16Fast'

    Char c() C { return t8 ? Char8To16(*t8) : *t16; }   // assumes that "t8 || t16"
    Char n() { return t8 ? Char8To16(*++t8) : *++t16; } // assumes that "t8 || t16"

    void operator+=(Int i) {
        if (t8)
            t8 += i;
        else
            t16 += i;
    } // assumes that "t8 || t16"
    void operator-=(Int i) {
        if (t8)
            t8 -= i;
        else
            t16 -= i;
    } // assumes that "t8 || t16"

    Int length() C { return t8 ? Length(t8) : Length(t16); }

    Bool operator==(C TextSrc &ts) C { return t8 == ts.t8 && t16 == ts.t16; }
    Bool operator!=(C TextSrc &ts) C { return t8 != ts.t8 || t16 != ts.t16; }

    TextSrc() {}
    TextSrc(CChar8 *t) {
        t8 = t;
        t16 = null;
    }
    TextSrc(CChar *t) {
        t16 = t;
        t8 = null;
    }
};
/******************************************************************************/
struct TextSplit {
    Byte shadow;
    Color color;
    Int offset;
    Int length;
    Int datas;
    C StrData *data;
    C Font *font;
    C PanelImage *panel;
    TextSrc text;

    Int end() C { return offset + length; }
    void end(Int end) { length = end - offset; }

    void fixLength() {
        if (length < 0)
            length = text.length() + Length(data, datas);
    } // if unlimited, then recalc precisely

    Bool operator==(C TextSplit &ts) C {
        return shadow == ts.shadow && color == ts.color && offset == ts.offset && length == ts.length && datas == ts.datas && data == ts.data && font == ts.font && panel == ts.panel && text == ts.text;
    }
    Bool operator!=(C TextSplit &ts) C { return !(T == ts); }
};
static Memc<TextSplit> TextSplits;
/******************************************************************************/
// TEXT PROCESSOR
/******************************************************************************/
struct TextProcessor {
    Byte shadow;
    TEXT_INDEX_MODE index_mode;
    SPACING_MODE spacing;
    Char prev_chr;
    Color color;
    Flt xsize, ysize, xsize_2, space, space_2, panel_r, panel_padd_r;
    Vec2 pos, size;
    C Font *font, *default_font;
    C PanelImage *panel;

    inline Bool spacingConst() C { return spacing == SPACING_CONST; }

    void setFontFast(C Font *font) {
        T.font = font;
        xsize = size.x / font->height(); // width of 1 font texel
    }

    Int charWidth(Char chr) C { return font->charWidth(chr); }
    Int nextCharWidth(Char next_chr) C { return font->charWidth(prev_chr, next_chr, spacing); }

    void advance(Char next_chr) { pos.x += space + xsize * nextCharWidth(next_chr); }
    void advanceSplit(Char next_chr) { pos.x -= space + xsize * nextCharWidth(next_chr); }
    Bool advanceFast(Char next_chr) {
        Flt w = xsize * nextCharWidth(next_chr);
        Flt test = (spacingConst() ? (index_mode == TEXT_INDEX_DEFAULT) ? space_2 : space : (index_mode == TEXT_INDEX_DEFAULT) ? w / 2
                                                                                        : (index_mode == TEXT_INDEX_OVERWRITE) ? w + space_2
                                                                                                                               : xsize * charWidth(prev_chr)); // for TEXT_INDEX_FIT make sure that the 'prev_chr' fully fits
        if (pos.x < test)
            return true;
        pos.x -= space + w;
        return false;
    }

    inline void panelPadd(C PanelImage &panel, Flt &left, Flt &right) C {
        left = panel.defaultInnerPadding().min.x,
        right = panel.defaultInnerPadding().max.x;
        Flt scale;
        if (panel.getSideScaleH(size.y, scale)) {
            left *= scale;
            right *= scale;
        }
    }

    Flt _width(TextSrc &text, C StrData *&data, Int &datas, Int &max_length, Bool stop_on_panel) // !! MAY CHANGE 'font' !! but doesn't change 'panel'
    {
        if (max_length) {
            // Char prev_chr='\0';
            Char chr = text.c();
            Int chr_pixels = 0, elements = (spacingConst() ? 0 : -1); // -1 because we will count only in between
            Flt width = 0;
        loop:
            if (!chr) {
            next_data:
                if (datas <= 0)
                    goto end;
                datas--;
                C StrData *d = data++;
                switch (d->type) {
                case StrData::TEXT:
                    if (d->text.is()) // process only if have anything, if not then proceed to 'next_data'
                    {
                        text = d->text();
                        chr = text.c();
                        goto have_char;
                    }
                    break;

                case StrData::IMAGE: {
                    C Image *image = d->image();
                    if (image && image->is()) {
                        Flt w = size.y * image->aspect();
                        if (spacingConst()) {
                            width += Max(w, space);
                        } else {
                            width += w;
                            elements++;
                        }
                    } else
                        elements++;
                    if (!--max_length)
                        goto end;
                    if (prev_chr) {
                        chr_pixels += nextCharWidth('\0');
                        prev_chr = '\0';
                    } // can check this after 'max_length' because if it goes to 'end', it will calculate the same thing
                } break;

                case StrData::PANEL: {
                    if (stop_on_panel) {
                        datas++;
                        data--;
                        goto end;
                    } // revert because we haven't processed this
                    if (prev_chr) {
                        chr_pixels += nextCharWidth('\0');
                        prev_chr = '\0';
                    }
                    if (C PanelImage *panel = d->panel()) {
                        width += chr_pixels * xsize;
                        chr_pixels = 0; // process what we have because 'xsize' might change
                        Flt w = _width(text, data, datas, max_length, true), panel_padd_l, panel_padd_r;
                        panelPadd(*panel, panel_padd_l, panel_padd_r);
                        w = Max(w + panel_padd_l + panel_padd_r, size.y);
                        if (spacingConst()) {
                            width += Max(w, space);
                        } else {
                            width += w;
                            elements++;
                        }
                        if (!max_length)
                            goto end;
                    }
                } break;

                case StrData::FONT: {
                    C Font *new_font = d->font();
                    if (!new_font)
                        new_font = default_font;
                    if (font != new_font) {
                        if (prev_chr) {
                            chr_pixels += nextCharWidth('\0');
                            prev_chr = '\0';
                        }
                        width += chr_pixels * xsize;
                        chr_pixels = 0;
                        setFontFast(new_font);
                    }
                } break;
                }
                goto next_data;
            }

        have_char:
            elements++;
            if (prev_chr)
                chr_pixels += nextCharWidth(chr);
            prev_chr = chr;

            {
            combining:
                Char n = text.n();
                if (!--max_length)
                    goto end; // check after advancing 'text' pointer
                if (CharFlagFast(n) & CHARF_SKIP)
                    goto combining;

                chr = n;
                goto loop;
            }

        end:
            if (prev_chr) {
                chr_pixels += nextCharWidth('\0');
                prev_chr = '\0';
            }

            if (elements > 0)
                width += elements * space; // to skip -1 and 0
            width += chr_pixels * xsize;
            return width;
        }
        return 0;
    }
    Flt panelWidth(TextSrc text, C StrData *data, Int datas, Int max_length = -1) {
        C Font *start_font = font;
        Flt width = _width(text, data, datas, max_length, true);
        if (font != start_font)
            setFontFast(start_font); // if font got changed, restore it
        return width;
    }
    Flt _width(TextSrc text, C StrData *data, Int datas, Int max_length) // !! MAY CHANGE 'font' !! but doesn't change 'panel'
    {
        Flt width = 0;
        if (panel) {
            width = _width(text, data, datas, max_length, true);
            Flt panel_padd_l, panel_padd_r;
            panelPadd(*panel, panel_padd_l, panel_padd_r);
            width = Max(width + panel_padd_l + panel_padd_r, size.y);
            if (spacingConst())
                MAX(width, space);
            else if (max_length && (text.c() || VisibleData(data, datas)))
                width += space; // have to add 'space' only if there's something after
        }
        width += _width(text, data, datas, max_length, false);
        return width;
    }
    Flt width(TextSrc text, C StrData *data, Int datas, Int max_length) {
        C Font *start_font = font;
        Flt width = _width(text, data, datas, max_length);
        if (font != start_font)
            setFontFast(start_font); // if font got changed, restore it
        return width;
    }
    Flt width(C TextSplit &split) {
        C Font *start_font = font;
        if (font != split.font)
            setFontFast(split.font);
        C PanelImage *start_panel = panel;
        panel = split.panel;
        Flt width = _width(split.text, split.data, split.datas, split.length);
        if (font != start_font)
            setFontFast(start_font); // if font got changed, restore it
        panel = start_panel;         // restore panel
        return width;
    }
    Flt widthNoRestore(C TextSplit &split, Int max_length) {
        if (font != split.font)
            setFontFast(split.font);
        panel = split.panel;
        return _width(split.text, split.data, split.datas, max_length);
    }

    void processPanelFast(C TextSrc &text, C StrData *data, Int datas) {
        Flt w = panelWidth(text, data, datas);
        Flt panel_padd_l /*, panel_padd_r*/;
        panelPadd(*panel, panel_padd_l, panel_padd_r);
        w += panel_padd_l + panel_padd_r;
        if (w < size.y) // require min square
        {
            Flt d = (size.y - w) / 2;
            w = size.y;
            panel_padd_l += d;
            panel_padd_r += d;
        }
        // here use "-" because we're decreasing
        if (spacingConst()) {
            Flt o = Max(w, space);
            panel_r = pos.x - o;
            pos.x -= (o - w) / 2;
        } else {
            panel_r = pos.x - w - space;
        } // calculate panel right side, it will be used to set position after finishing drawing it, this is needed to always apply 'space' after that panel, regardless if it's empty or not
        pos.x -= panel_padd_l;
        if (index_mode == TEXT_INDEX_FIT)
            pos.x -= panel_padd_r; // in FIT mode make sure we will have enough room to close the panel (display panel padding on the right side), this adjustment will be canceled out later when setting as 'panel_r'
    }

    Bool initFast(C TextStyleParams &style) {
        if (default_font = style.getFont()) {
            prev_chr = '\0';

            spacing = style.spacing;
            size = style.size;
            space = style.space.x * size.x;
            space_2 = space / 2;

            return true;
        }
        return false;
    }

    inline Bool insidePanelNearPos() C { return (index_mode == TEXT_INDEX_OVERWRITE) && panel && panel_r <= (spacingConst() ? 0 : -space_2); } // if we're inside a panel that's closing near the requested position, this is used to prevent from advancing to the next element if still haven't finished current panel

    Int textIndex(C TextStyleParams &style, Flt x, TEXT_INDEX_MODE index_mode, TextSrc text, C StrData *data, Int datas, C Font *start_font, C PanelImage *start_panel) {
        if (text.is() || VisibleData(data, datas))
            if (initFast(style)) {
                text.fix();
                pos.x = x;
                T.index_mode = index_mode;
                if (!start_font)
                    start_font = default_font;
                setFontFast(start_font);
                if (panel = start_panel)
                    processPanelFast(text, data, datas);

                Int offset = 0, chars = 0;
                Char chr = text.c();
            loop:
                if (!chr) {
                next_data:
                    if (datas <= 0)
                        goto end;
                    datas--;
                    C StrData *d = data++;
                    switch (d->type) {
                    case StrData::TEXT:
                        if (d->text.is()) // process only if have anything, if not then proceed to 'next_data'
                        {
                            text = d->text();
                            chr = text.c();
                            goto have_char;
                        }
                        break;

                    case StrData::IMAGE: {
                        if (prev_chr) {
                            if (advanceFast('\0'))
                                goto ret;
                            offset += chars;
                            chars = 0;
                            prev_chr = '\0';
                        }

                        Flt w;
                        C Image *image = d->image();
                        if (image && image->is())
                            w = size.y * image->aspect();
                        else
                            w = 0;
                        Flt test = (spacingConst() ? (index_mode == TEXT_INDEX_DEFAULT) ? Max(w, space) / 2 : Max(w, space) : (index_mode == TEXT_INDEX_DEFAULT) ? w / 2
                                                                                                                          : (index_mode == TEXT_INDEX_OVERWRITE) ? w + space_2
                                                                                                                                                                 : w);
                        if (pos.x < test)
                            goto ret;
                        const Bool FAST_CHECK = true;
                        if (!FAST_CHECK && insidePanelNearPos() && PanelClosing(data, datas))
                            goto ret;
                        pos.x -= (spacingConst() ? Max(w, space) : w + space);
                        if (FAST_CHECK && insidePanelNearPos() && pos.x /*-w-space : already applied this above*/ - panel_padd_r <= panel_r + EPS)
                            goto ret; // if this is the last element, "image_left_pos-image_w-panel_padd_r<=panel_r+space+EPS" 'panel_r' had 'space' applied so have to revert it
                        offset++;
                    } break;

                    case StrData::PANEL: {
                        if (prev_chr) {
                            if (insidePanelNearPos() || advanceFast('\0'))
                                goto ret;
                            offset += chars;
                            chars = 0;
                            prev_chr = '\0';
                        }
                        if (panel)
                            pos.x = panel_r;
                        if (panel = d->panel())
                            processPanelFast(text, data, datas);
                    } break;

                    case StrData::FONT: {
                        C Font *new_font = d->font();
                        if (!new_font)
                            new_font = default_font;
                        if (font != new_font) {
                            if (prev_chr) {
                                if (advanceFast('\0'))
                                    goto ret;
                                offset += chars;
                                chars = 0;
                                prev_chr = '\0';
                            }
                            setFontFast(new_font);
                        }
                    } break;
                    }
                    goto next_data;
                }

            have_char:

                if (prev_chr) {
                    if (advanceFast(chr))
                        goto ret;
                    offset += chars;
                    chars = 0;
                }
                prev_chr = chr;

                chars++;

                {
                combining:
                    Char n = text.n();
                    if (CharFlagFast(n) & CHARF_SKIP) {
                        chars++;
                        goto combining;
                    }

                    chr = n;
                    goto loop;
                }

            end:
                if (prev_chr) {
                    if (advanceFast('\0'))
                        goto ret;
                    offset += chars; /*chars=0; prev_chr='\0';*/
                }

            ret:
                /*prev_chr='\0';*/ return offset;
                // no need to clear because this is a standalone function to be called outside of others
            }
        return 0;
    }

    void separator(TextSplit &separator, C TextSrc &text, C StrData *data, Int datas, Int offset) {
        separator.shadow = shadow;
        separator.color = color;
        separator.datas = datas;
        // separator.length=-1; this may be called several times, don't set it here, instead call it only one time later
        separator.offset = offset;
        separator.text = text;
        separator.data = data;
        separator.font = font;
        separator.panel = panel;
    }

    Bool initSplit(C TextStyleParams &style, Flt &width) {
        if (initFast(style)) {
            T.index_mode = TEXT_INDEX_FIT;
            setFontFast(default_font);
            panel = null;
            shadow = style.shadow;
            color = style.color;
            width += EPS; // increase a bit, to workaround numerical precision issues, example: calculating total width of multi-line text, then doing the same calculation but with space limited to previously calculated width. Have to make sure that new calculation will give same results, that text will fit in what was calculated.
            return true;
        }
        return false;
    }
    inline Flt fastWidth(Flt width, C TextSplit &split) {
        Flt w = width - pos.x; // 'width'=available space, 'pos.x' starts from width and goes down to 0, 'w'=distance travelled
        if (!spacingConst())
            w = Max(0, w - space); // revert last 'space' (if any)
#if 0                              // test if calculation is correct, should be same as width(split)
      DEBUG_ASSERT(Equal(w, T.width(split)), "fastWidth");
#endif
        return w;
    }

#define SINGLE 1
    Bool splitNext(TextSplit &split
#if !SINGLE
                   ,
                   TextSplit &next
#endif
                   ,
                   C TextStyleParams &style, Flt width, Bool auto_line, TextSrc &text, C StrData *&data, Int &datas, Int &offset, Flt *split_width = null) // Don't set null fonts.
    {
#if SINGLE
        separator(split, text, data, datas, offset);
#endif
        pos.x = width;
        if (panel)
            processPanelFast(text, data, datas);

        Char chr = text.c();
        if (!auto_line) {
        aln_loop:
            if (!chr) {
            aln_next_data:
                if (datas <= 0)
                    goto aln_end;
                datas--;
                C StrData *d = data++;
                switch (d->type) {
                case StrData::TEXT:
                    if (d->text.is()) {
                        text = d->text();
                        chr = text.c();
                        goto aln_have_char;
                    }
                    break; // process only if have anything, if not then proceed to 'next_data'
                case StrData::IMAGE:
                    offset++;
                    break;
                case StrData::PANEL:
                    panel = d->panel();
                    break;
                case StrData::FONT: {
                    C Font *new_font = d->font();
                    font = (new_font ? new_font : default_font);
                } break;
                case StrData::COLOR:
                    color = d->color;
                    break;
                case StrData::COLOR_OFF:
                    color = style.color;
                    break;
                case StrData::SHADOW:
                    shadow = d->shadow;
                    break;
                case StrData::SHADOW_OFF:
                    shadow = style.shadow;
                    break;
                }
                goto aln_next_data;
            }

        aln_have_char: {
            Char n = text.n();
            offset++;
            if (chr == '\n') {
                split.end(offset - 1);
                if (split_width)
                    *split_width = T.width(split);
                return true;
            }
            chr = n;
            goto aln_loop;
        }

        aln_end:
            split.length = -1; // unlimited, end(offset);
            if (split_width)
                *split_width = T.width(split);
        } else {
#if SINGLE
            TextSplit next;
#endif
            split.length = -1;
        loop:
            if (!chr) {
            next_data:
                if (datas <= 0)
                    goto end;
                datas--;
                C StrData *d = data++;
                switch (d->type) {
                case StrData::TEXT:
                    if (d->text.is()) // process only if have anything, if not then proceed to 'next_data'
                    {
                        text = d->text();
                        chr = text.c();
                        goto have_char;
                    }
                    break;

                case StrData::IMAGE: {
                    if (prev_chr) {
                        advanceSplit('\0');
                        prev_chr = '\0';
                    }

                    Flt w;
                    C Image *image = d->image();
                    if (image && image->is())
                        w = size.y * image->aspect();
                    else
                        w = 0;

                    // 'offset' is at image, but 'data', 'datas' after it
                    if (offset > split.offset                            // require at least length=1
                        && pos.x < (spacingConst() ? Max(w, space) : w)) // image doesn't fit
                    {
                        split.end(offset);
                        data--;
                        datas++;
                        if (split_width)
                            *split_width = fastWidth(width, split);
                        return true;
                    }

                    pos.x -= (spacingConst() ? Max(w, space) : w + space);
                    offset++;

                    split.end(offset);
                    separator(next, text, data, datas, offset);
                } break;

                case StrData::PANEL: {
                    if (prev_chr) {
                        advanceSplit('\0');
                        prev_chr = '\0';
                    }
                    if (panel)
                        pos.x = panel_r;
                    if (panel = d->panel())
                        processPanelFast(text, data, datas);
                } break;

                case StrData::FONT: {
                    C Font *new_font = d->font();
                    if (!new_font)
                        new_font = default_font;
                    if (font != new_font) {
                        if (prev_chr) {
                            advanceSplit('\0');
                            prev_chr = '\0';
                        }
                        setFontFast(new_font);
                    }
                } break;

                case StrData::COLOR:
                    color = d->color;
                    break;
                case StrData::COLOR_OFF:
                    color = style.color;
                    break;
                case StrData::SHADOW:
                    shadow = d->shadow;
                    break;
                case StrData::SHADOW_OFF:
                    shadow = style.shadow;
                    break;
                }
                goto next_data;
            }

        have_char: {
            Int offset_start = offset; // 'offset_start' is located at 'chr'

            if (prev_chr)
                advanceSplit(chr);

            // combining
            Int chars = 1;
        combining:
            Char n = text.n();
            if (CharFlagFast(n) & CHARF_SKIP) {
                chars++;
                goto combining;
            }
            offset += chars;
            // 'text' and 'offset' are now after ('chr' and combined)

            Bool min_length = (offset_start > split.offset); // require at least length=1

            Bool new_line = (chr == '\n'                                                         // manual new line
                             || (min_length                                                      // require at least length=1
                                 && pos.x < (spacingConst() ? space : xsize * charWidth(chr)))); // character doesn't fit

            Bool skippable = (chr == ' ' || chr == '\n');
            if (skippable                       // found separator
                || new_line && split.length < 0 // or going to create a new line, but separator wasn't found yet
                || min_length && (Separatable(prev_chr) || Separatable(chr))) {
                split.end(offset_start);
                if (skippable) {
                    if (new_line /*&& skippable already checked*/ && panel && !n && PanelClosing(data, datas))
                        panel = null; // if we've skipped 'chr', creating new line, inside panel, and there's no visible element until panel closes, then close it already, so we don't start the new line with this panel
                    separator(next, text, data, datas, offset);
                } else {
                    separator(next, text, data, datas, offset_start);
                    next.text -= chars;
                }
            }

            if (new_line) {
                if (offset_start > next.offset) // if went too far, and separator is behind this step, it means that more than text could have changed
                {                               // restart from last remembered position
                    shadow = next.shadow;
                    color = next.color;
                    offset = next.offset;
                    text = next.text;
                    data = next.data;
                    datas = next.datas;
                    panel = next.panel;
                    if (next.font != font)
                        setFontFast(next.font);
                    prev_chr = '\0'; // clear BEFORE 'width'
                    if (split_width)
                        *split_width = T.width(split);
                } else {
                    if (split_width) {
                        Int split_end = split.end();
                        // if( split_end==offset      ){prev_chr=chr; advanceSplit('\0');                      prev_chr='\0'; *split_width=fastWidth(width, split);}else // clear BEFORE 'width', this will never happen
                        if (split_end == offset_start) {
                            pos.x += xsize * (nextCharWidth(chr) - nextCharWidth('\0'));
                            prev_chr = '\0';
                            *split_width = fastWidth(width, split);
                        } else // clear BEFORE 'width'
                        {
                            prev_chr = '\0';
                            *split_width = T.width(split);
                        } // clear BEFORE 'width'
                    }
                    if (offset != next.offset) {
                        offset = next.offset;
                        text = next.text;
                    }
                    prev_chr = '\0';
                }
                return true;
            }

            prev_chr = chr;
            chr = n;
            goto loop;
        }

        end:
            if (prev_chr) {
                advanceSplit('\0');
                prev_chr = '\0';
            } // make sure to clear 'prev_chr', because this class can still be used after this function, BEFORE 'width'
            split.length = -1; // unlimited, end(offset);
            if (split_width)
                *split_width = fastWidth(width, split);
        }
        return false;
    }
    void splitParts(MemPtr<TextSplit> splits, C TextStyleParams &style, Flt width, Bool auto_line, TextSrc text, C StrData *data, Int datas) // 1. Have to set at least one line to support drawing cursor when text is empty. 2. Don't set null fonts.
    {
        splits.clear();
        if (initSplit(style, width)) {
            text.fix();
            Int offset = 0;
#if SINGLE
            for (; splitNext(splits.New(), style, width, auto_line, text, data, datas, offset);)
                ;
#else
            {
                TextSplit &split = splits.New();
                separator(split, text, data, datas, offset);
            }
        next:
            TextSplit &next = splits.New();
            if (splitNext(splits[splits.elms() - 2], next, style, width, auto_line, text, data, datas, offset))
                goto next;
            splits.removeLast();
#endif
        }
    }
    void split(MemPtr<TextSplit> splits, C TextStyleParams &style, Flt width, Bool auto_line, TextSrc text, C StrData *data, Int datas, Flt *actual_width = null) // 1. Have to set at least one line to support drawing cursor when text is empty. 2. Don't set null fonts.
    {
        splits.clear();
        if (actual_width)
            *actual_width = 0;
        if (initSplit(style, width)) {
            text.fix();
            Int offset = 0;

            TextSplit *split = &splits.New();
            separator(*split, text, data, datas, offset);
            pos.x = width;
            Char chr = text.c();
            if (!auto_line) {
            aln_loop:
                if (!chr) {
                aln_next_data:
                    if (datas <= 0)
                        goto aln_end;
                    datas--;
                    C StrData *d = data++;
                    switch (d->type) {
                    case StrData::TEXT:
                        if (d->text.is()) {
                            text = d->text();
                            chr = text.c();
                            goto aln_have_char;
                        }
                        break; // process only if have anything, if not then proceed to 'next_data'
                    case StrData::IMAGE:
                        offset++;
                        break;
                    case StrData::PANEL:
                        panel = d->panel();
                        break;
                    case StrData::FONT: {
                        C Font *new_font = d->font();
                        font = (new_font ? new_font : default_font);
                    } break;
                    case StrData::COLOR:
                        color = d->color;
                        break;
                    case StrData::COLOR_OFF:
                        color = style.color;
                        break;
                    case StrData::SHADOW:
                        shadow = d->shadow;
                        break;
                    case StrData::SHADOW_OFF:
                        shadow = style.shadow;
                        break;
                    }
                    goto aln_next_data;
                }

            aln_have_char: {
                Char n = text.n();
                offset++;
                if (chr == '\n') {
                    split->end(offset - 1);
                    if (actual_width)
                        MAX(*actual_width, T.width(*split));
                    split = &splits.New();
                    separator(*split, text, data, datas, offset);
                }
                chr = n;
                goto aln_loop;
            }

            aln_end:
                split->length = -1; // unlimited, end(offset);
                if (actual_width)
                    MAX(*actual_width, T.width(*split));
            } else {
                TextSplit *next = &splits.New();
                split = &splits[splits.elms() - 2];
                split->length = -1;
            loop:
                if (!chr) {
                next_data:
                    if (datas <= 0)
                        goto end;
                    datas--;
                    C StrData *d = data++;
                    switch (d->type) {
                    case StrData::TEXT:
                        if (d->text.is()) // process only if have anything, if not then proceed to 'next_data'
                        {
                            text = d->text();
                            chr = text.c();
                            goto have_char;
                        }
                        break;

                    case StrData::IMAGE: {
                        if (prev_chr) {
                            advanceSplit('\0');
                            prev_chr = '\0';
                        }

                        Flt w;
                        C Image *image = d->image();
                        if (image && image->is())
                            w = size.y * image->aspect();
                        else
                            w = 0;

                        // 'offset' is at image, but 'data', 'datas' after it
                        if (offset > split->offset                           // require at least length=1
                            && pos.x < (spacingConst() ? Max(w, space) : w)) // image doesn't fit
                        {
                            split->end(offset);
                            separator(*next, text, data - 1, datas + 1, offset);
                            if (actual_width)
                                MAX(*actual_width, fastWidth(width, *split));
                            next = &splits.New();
                            split = &splits[splits.elms() - 2];
                            split->length = -1;
                            pos.x = width;
                            if (panel)
                                processPanelFast(text, data, datas);
                        }

                        pos.x -= (spacingConst() ? Max(w, space) : w + space);
                        offset++;

                        split->end(offset);
                        separator(*next, text, data, datas, offset);
                    } break;

                    case StrData::PANEL: {
                        if (prev_chr) {
                            advanceSplit('\0');
                            prev_chr = '\0';
                        }
                        if (panel)
                            pos.x = panel_r;
                        if (panel = d->panel())
                            processPanelFast(text, data, datas);
                    } break;

                    case StrData::FONT: {
                        C Font *new_font = d->font();
                        if (!new_font)
                            new_font = default_font;
                        if (font != new_font) {
                            if (prev_chr) {
                                advanceSplit('\0');
                                prev_chr = '\0';
                            }
                            setFontFast(new_font);
                        }
                    } break;

                    case StrData::COLOR:
                        color = d->color;
                        break;
                    case StrData::COLOR_OFF:
                        color = style.color;
                        break;
                    case StrData::SHADOW:
                        shadow = d->shadow;
                        break;
                    case StrData::SHADOW_OFF:
                        shadow = style.shadow;
                        break;
                    }
                    goto next_data;
                }

            have_char: {
                Int offset_start = offset; // 'offset_start' is located at 'chr'

                if (prev_chr)
                    advanceSplit(chr);

                // combining
                Int chars = 1;
            combining:
                Char n = text.n();
                if (CharFlagFast(n) & CHARF_SKIP) {
                    chars++;
                    goto combining;
                }
                offset += chars;
                // 'text' and 'offset' are now after ('chr' and combined)

                Bool min_length = (offset_start > split->offset); // require at least length=1

                Bool new_line = (chr == '\n'                                                         // manual new line
                                 || (min_length                                                      // require at least length=1
                                     && pos.x < (spacingConst() ? space : xsize * charWidth(chr)))); // character doesn't fit

                Bool skippable = (chr == ' ' || chr == '\n');
                if (skippable                        // found separator
                    || new_line && split->length < 0 // or going to create a new line, but separator wasn't found yet
                    || min_length && (Separatable(prev_chr) || Separatable(chr))) {
                    split->end(offset_start);
                    if (skippable) {
                        if (new_line /*&& skippable already checked*/ && panel && !n && PanelClosing(data, datas))
                            panel = null; // if we've skipped 'chr', creating new line, inside panel, and there's no visible element until panel closes, then close it already, so we don't start the new line with this panel
                        separator(*next, text, data, datas, offset);
                    } else {
                        separator(*next, text, data, datas, offset_start);
                        next->text -= chars;
                    }
                }

                if (new_line) {
                    if (offset_start > next->offset) // if went too far, and separator is behind this step, it means that more than text could have changed
                    {                                // restart from last remembered position
                        shadow = next->shadow;
                        color = next->color;
                        offset = next->offset;
                        text = next->text;
                        data = next->data;
                        datas = next->datas;
                        panel = next->panel;
                        if (next->font != font)
                            setFontFast(next->font);
                        n = text.c();
                        prev_chr = '\0'; // clear BEFORE 'width' and 'processPanelFast'
                        if (actual_width)
                            MAX(*actual_width, T.width(*split));
                    } else {
                        if (actual_width) {
                            Int split_end = split->end();
                            // if( split_end==offset      ){prev_chr=chr; advanceSplit('\0');                      prev_chr='\0'; MAX(*actual_width, fastWidth(width, *split));}else // clear BEFORE 'width', this will never happen
                            if (split_end == offset_start) {
                                pos.x += xsize * (nextCharWidth(chr) - nextCharWidth('\0'));
                                prev_chr = '\0';
                                MAX(*actual_width, fastWidth(width, *split));
                            } else // clear BEFORE 'width'
                            {
                                prev_chr = '\0';
                                MAX(*actual_width, T.width(*split));
                            } // clear BEFORE 'width'
                        }
                        if (offset != next->offset) {
                            offset = next->offset;
                            text = next->text;
                            n = text.c();
                        }
                        prev_chr = '\0'; // clear before 'processPanelFast'
                    }
                    next = &splits.New();
                    split = &splits[splits.elms() - 2];
                    split->length = -1;
                    pos.x = width;
                    if (panel)
                        processPanelFast(text, data, datas);
                } else
                    prev_chr = chr;

                chr = n;
                goto loop;
            }

            end:
                if (prev_chr) {
                    advanceSplit('\0');
                    prev_chr = '\0';
                } // make sure to clear 'prev_chr', because this class can still be used after this function
                split->length = -1;
                if (actual_width)
                    MAX(*actual_width, fastWidth(width, *split));
                splits.removeLast(); // unlimited, end(offset); remove after adjusting 'split' because it might change memory address
            }
        }
    }
    Bool splitTest(C TextStyleParams &style, Flt width, Bool auto_line, TextSrc text, C StrData *data, Int datas) {
        Memt<TextSplit> full, parts;
        split(full, style, width, auto_line, text, data, datas);
        splitParts(parts, style, width, auto_line, text, data, datas);
        return full == parts;
    }
};
/******************************************************************************/
struct TextDrawer : TextProcessor {
    Bool sub_pixel;
    Int font_offset_i_y;
    Flt x_align_mul, y_align_mul;
    Vec2 font_offset;

    void initDraw(C TextStyleParams &style) {
        sub_pixel = default_font->_sub_pixel; // 'sub_pixel' before 'setColor'

        x_align_mul = style.align.x / 2 - 0.5f;
        y_align_mul = style.align.y / 2 + 0.5f;
    }
    void setFont(C Font *font) {
        T.font = font;
        xsize = size.x / font->height(); // width  of 1 font texel
        ysize = size.y / font->height(); // height of 1 font texel
        xsize_2 = xsize / 2;

        font_offset_i_y = font->paddingT();
        font_offset.set(-xsize * font->paddingL(),
                        ysize * font_offset_i_y);
    }
};
/******************************************************************************/
struct TextDrawerHW : TextDrawer {
    ALPHA_MODE alpha;
    Int cur, sel;
    ShaderImage *shader_image;
    C Shader *shader;

    static inline Flt ByteToFontLum(Byte b) { return Min(b, 128) / 128.0f; } // calculate text brightness 0..1, multiply by "2" (/128.0f instead of /255.0f) will give better results for grey text color (reaches 1.0 already at grey 128 byte value)
    void setColor(C Color &color) {
        VI.color(T.color = color);
        if (!sub_pixel) {
#if LINEAR_GAMMA
            Sh.FontLum->set(ByteToFontLum(color.lum()));
#endif
        } else {
#if LINEAR_GAMMA
            Sh.FontLum->set(Vec(ByteToFontLum(color.r), ByteToFontLum(color.g), ByteToFontLum(color.b)));
#endif
            D.alphaFactor(color); // 'MaterialClear' called at start only one time instead of here which can be called multiple times
        }
    }
    void setShadow(Byte shadow) {
        Sh.FontShadow->set(ByteToFlt(T.shadow = shadow));
    }
    void changeColor(C Color &color) {
        if (T.color != color) {
            VI.flush();
            setColor(color);
        }
    }
    void changeShadow(Byte shadow) {
        if (T.shadow != shadow) {
            VI.flush();
            setShadow(shadow);
        }
    }

    void initDraw(C TextStyleParams &style) {
        super::initDraw(style);

        cur = -1;
        sel = -1;
        if (style.edit) {
            cur = style.edit->cur;
            sel = style.edit->sel;
        }

        // font params
        Flt contrast;
        Byte lum = style.color.lum();
        if (!lum)
            contrast = 1;
        else {
            Flt pixels = Renderer.screenToPixelSize(size).min();
            if (pixels >= 32)
                contrast = 1;
            else {
                contrast = 32 / pixels;
                contrast = Sqrt(contrast); // or alternative: contrast=Log2(contrast+1);
                contrast = Max(1, contrast);
                contrast = Lerp(1.0f, contrast, ByteToFlt(lum));
            }
        }
        Sh.FontContrast->set(contrast);
        Sh.FontShade->set(ByteSRGBToDisplay(style.shade));

        shader_image = (sub_pixel ? Sh.Img[0] : Sh.ImgXY[0]);

        // alpha
        if (sub_pixel) {
            alpha = D.alpha(Renderer.inside() ? ALPHA_FONT_DEC : ALPHA_FONT);
            VI.shader(Sh.FontCurSP);
            MaterialClear();
        } else // if drawing text while rendering, then decrease the alpha channel (glow), for sub-pixel we will be changing 'D.alphaFactor' for which we have to call 'MaterialClear'
            if (Renderer.inside())
                D.alpha(ALPHA_RENDER_BLEND); // if drawing text while rendering, set special alpha mode, but don't bother to restore it, as in Rendering, alpha blending is always set for each call

        // font depth
        if (D._text_depth) // apply new state
        {
            D.depthLock(true);
            D.depthWrite(false);
            Renderer.needDepthTest(); // !! 'needDepthTest' after 'depthWrite' !!
            VI.shader(Sh.Font[true][D._linear_gamma]);
        }

        shader = VI._shader;
        VI._image = null; // clear to make sure 'VI.imageConditional' below will work properly
    }
    Bool init(C TextStyleParams &style) {
        if (initFast(style)) {
            initDraw(style);
            return true;
        }
        return false;
    }
    void shut() {
        VI.end();

        // alpha
        if (sub_pixel)
            D.alpha(alpha); // restore alpha

        // font depth
        if (D._text_depth) // reset old state
        {
            D.depthUnlock();
            D.depthWrite(true);
        }
    }

    void processPanel(C TextSrc &text, C StrData *data, Int datas, Int max_length) {
        Flt w = panelWidth(text, data, datas, max_length);
        Flt panel_padd_l, panel_padd_r;
        panelPadd(*panel, panel_padd_l, panel_padd_r);
        w += panel_padd_l + panel_padd_r;
        if (w < size.y) // require min square
        {
            Flt d = (size.y - w) / 2;
            w = size.y;
            panel_padd_l += d;
            panel_padd_r += d;
        }
        if (spacingConst()) {
            Flt o = Max(w, space);
            panel_r = pos.x + o;
            pos.x += (o - w) / 2;
        } else {
            panel_r = pos.x + w + space;
        } // calculate panel right side, it will be used to set position after finishing drawing it, this is needed to always apply 'space' after that panel, regardless if it's empty or not
        panel->draw(Rect_LU(pos, w, size.y));
        pos.x += panel_padd_l;
    }

    void _draw(C TextStyleParams &style, Flt x, Flt y, TextSrc text, C StrData *data, Int datas, Int max_length, C PanelImage *start_panel, Int offset = 0, Int next_offset = -1) // assumes: 'text.fix'
    {
        panel = start_panel;

        Flt total_width = (Equal(x_align_mul, 0) ? 0 : width(text, data, datas, max_length)); // don't calculate 'width' when not needed, AFTER 'setFont' and 'panel'
        x += total_width * x_align_mul;
        y += size.y * y_align_mul;

        pos.set(x, y);
        if (style.pixel_align)
            D.alignScreenToPixel(pos);

        if (panel) {
            VI.flush();
            VI.shader(null);
            processPanel(text, data, datas, max_length);
            VI.shader(shader);
            VI.color(color);
        }

        Int start_offset = offset;
        Flt cur_w = -1, cur_x, sel_x;
#if DEBUG
        cur_x = sel_x = 0; // to prevent run-time check exceptions in debug mode
#endif

        if (max_length) {
            Char n, chr = text.c();
        loop:
            if (!chr) {
            next_data:
                if (datas <= 0)
                    goto end;
                datas--;
                C StrData *d = data++;
                switch (d->type) {
                case StrData::COLOR:
                    changeColor(d->color);
                    break;
                case StrData::COLOR_OFF:
                    changeColor(style.color);
                    break;
                case StrData::SHADOW:
                    changeShadow(d->shadow);
                    break;
                case StrData::SHADOW_OFF:
                    changeShadow(style.shadow);
                    break;
                case StrData::TEXT:
                    if (d->text.is()) // process only if have anything, if not then proceed to 'next_data'
                    {
                        text = d->text();
                        chr = text.c();
                        goto have_char;
                    }
                    break;

                case StrData::IMAGE: {
                    if (prev_chr) {
                        advance('\0');
                        prev_chr = '\0';
                    }
                    if (sel == offset) {
                        sel_x = pos.x;
                    }

                    C Image *image = d->image();
                    if (image && image->is()) {
                        VI.flush();
                        VI.shader(null);
                        // TODO: this should handle adjusting/restoring D.alpha for sub_pixel, however that would introduce overhead

                        Flt w = size.y * image->aspect();
                        if (cur == offset) {
                            cur_x = pos.x;
                            cur_w = w;
                        }
                        Flt x = pos.x;
                        if (spacingConst()) {
                            Flt o = Max(w, space);
                            pos.x += o;
                            x += (o - w) / 2;
                        } else {
                            pos.x += w + space;
                        }
                        image->draw(style.image_color, TRANSPARENT, Rect_LU(x, pos.y, w, size.y));

                        VI.shader(shader);
                        VI.color(color);
                    } else {
                        if (cur == offset) {
                            cur_x = pos.x;
                            cur_w = size.y / 2;
                        }
                        pos.x += space;
                    }
                    offset++;
                    if (!--max_length)
                        goto end;
                } break;

                case StrData::PANEL: {
                    if (prev_chr) {
                        advance('\0');
                        prev_chr = '\0';
                    }
                    if (panel)
                        pos.x = panel_r;
                    if (panel = d->panel()) {
                        VI.flush();
                        VI.shader(null);
                        // TODO: this should handle adjusting/restoring D.alpha for sub_pixel, however that would introduce overhead

                        processPanel(text, data, datas, max_length);

                        VI.shader(shader);
                        VI.color(color);

                        if (!max_length)
                            goto end;
                    }
                } break;

                case StrData::FONT: {
                    C Font *new_font = d->font();
                    if (!new_font)
                        new_font = default_font;
                    if (font != new_font) {
                        if (prev_chr) {
                            advance('\0');
                            prev_chr = '\0';
                        }
                        // no need to 'VI.flush' because this will be done in 'VI.imageConditional'
                        setFont(new_font);
                    }
                } break;
                }
                goto next_data;
            }
        have_char:

            if (prev_chr)
                advance(chr);
            prev_chr = chr;

            // update positions
            if (cur == offset) {
                cur_x = pos.x;
                cur_w = xsize * charWidth(chr);
            }
            if (sel == offset) {
                sel_x = pos.x;
            }
            offset++;

            // draw character
            if (chr != ' ') // don't draw space sign, potentially we could check it below using "fc.height" instead, however this is faster
            {
                auto font_index = font->_wide_to_font[U16(chr)];
                if (InRange(font_index, font->_chrs)) {
                    C Font::Chr &fc = font->_chrs[font_index];
                    // if(fc.height) // potentially we could check for empty characters (full width space, half width space, nbsp, however in most cases we will have something to draw, so this check will slow down things
                    {
                        Vec2 chr_pos = pos;
                        if (spacingConst())
                            chr_pos.x += space_2 - xsize_2 * fc.width; // move back by half of the character width
                        chr_pos.x += font_offset.x;
                        chr_pos.y += ysize * (font_offset_i_y - fc.offset);
                        if (style.pixel_align)
                            D.alignScreenXToPixel(chr_pos.x);

                        Rect_LU rect(chr_pos, xsize * fc.width_padd, ysize * fc.height_padd);
                        VI.imageConditional(&font->_images[fc.image], *shader_image);
                        if (sub_pixel)
                            VI.imagePart(rect, fc.tex);
                        else
                            VI.font(rect, fc.tex);

                        // combining
                        if (!--max_length)
                            goto end; // !! THIS CAN CHECK BEFORE ADVANCING 'text' AS LONG AS WE'RE NOT GOING TO USE IT AFTERWARD !!
                        n = text.n();
                        UInt flag = CharFlagFast(n);
                        if (flag & (CHARF_COMBINING | (SUPPORT_EMOJI ? 0 : CHARF_MULTI1))) // without SUPPORT_EMOJI also check CHARF_MULTI1
                        {
#if !SUPPORT_EMOJI
                            if (flag & CHARF_MULTI1)
                                goto skip_multi1;
#endif
                            chr_pos.x += xsize_2 * fc.width; // after 'chr_pos' was pixel aligned, move by half of the character width to put it at centerX of 'chr' character
                            chr_pos.y += ysize * fc.offset;  // reset Y pos
                            Bool skipped_bottom_shadow_padding = false;

                        draw_combining:
                            // update positions
                            if (cur == offset) {
                                cur_x = pos.x;
                                cur_w = xsize * charWidth(chr);
                            }
                            if (sel == offset) {
                                sel_x = pos.x;
                            }
                            offset++;

                            auto font_index = font->_wide_to_font[U16(n)];
                            if (InRange(font_index, font->_chrs)) {
                                C Font::Chr &fc = font->_chrs[font_index];

                                if ((flag & CHARF_STACK) && !skipped_bottom_shadow_padding) // if found a first stacking character, then skip shadow padding at the bottom, because next character is drawn after base character and on top, so its bottom shadow must not overlap the base
                                {
                                    skipped_bottom_shadow_padding = true; // do this only once
                                    chr_pos.y += ysize * font->paddingB();
                                }

                                Vec2 n_pos = chr_pos;
                                n_pos.x -= xsize_2 * fc.width; // move back by half of the character width
                                n_pos.y -= ysize * fc.offset;
                                // if(style.pixel_align)D.alignScreenXToPixel(n_pos.x); 'n_pos' based on 'chr_pos' which was already aligned

                                Flt n_height_padd = ysize * fc.height_padd;
                                Rect_LU rect(n_pos, xsize * fc.width_padd, n_height_padd);
                                VI.imageConditional(&font->_images[fc.image], *shader_image);
                                if (sub_pixel)
                                    VI.imagePart(rect, fc.tex);
                                else
                                    VI.font(rect, fc.tex);
                                if (flag & CHARF_STACK)
                                    chr_pos.y += n_height_padd; // if the drawn character is stacking, then move position higher for next character, to stack combining characters on top of each other (needed for THAI)
                            }

                            if (!--max_length)
                                goto end; // !! THIS CAN CHECK BEFORE ADVANCING 'text' AS LONG AS WE'RE NOT GOING TO USE IT AFTERWARD !!
                            n = text.n();
                            flag = CharFlagFast(n);
                            if (flag & CHARF_COMBINING)
                                goto draw_combining; // if next character is combining too
                        }
                        chr = n;
                        goto loop;
                    }
                }
#if SUPPORT_EMOJI
                // if character not present in font, try to find emoji
                EmojiKey key(chr);
                if (CharFlagFast(chr) & CHARF_MULTI0) // first check if it's multi-char
                {
                    if (!--max_length)
                        goto end; // !! THIS CAN CHECK BEFORE ADVANCING 'text' AS LONG AS WE'RE NOT GOING TO USE IT AFTERWARD !!
                    n = text.n(); // this should be CHARF_MULTI1

                    // update positions
                    if (cur == offset) {
                        cur_x = pos.x;
                        cur_w = xsize * charWidth(chr);
                    }
                    if (sel == offset) {
                        sel_x = pos.x;
                    }
                    offset++;

                    if (!n)
                        goto end; // safety check
                    key.append(n);
                }
                Flt w = xsize * charWidth(chr);
                if (C ImagePtr &image = FindEmoji(key)) {
                    VI.flush();
                    VI.shader(null);
                    // TODO: this should handle adjusting/restoring D.alpha for sub_pixel, however that would introduce overhead

                    image->draw(style.image_color, TRANSPARENT, Rect_LU(spacingConst() ? pos.x + space_2 - w / 2 : pos.x, pos.y, w, size.y));

                    VI.shader(shader);
                    VI.color(color);
                } else {
                    font_index = font->_wide_to_font[Unsigned('\1')];
                    if (InRange(font_index, font->_chrs)) // if haven't found emoji, try drawing as invalid/unknown
                    {
                        C Font::Chr &fc = font->_chrs[font_index];
                        Vec2 chr_pos = pos;
                        if (spacingConst())
                            chr_pos.x += space_2 - xsize_2 * fc.width; // move back by half of the character width
                        else
                            chr_pos.x += w / 2 - xsize_2 * fc.width; // adjust here because we have spacing based on square size
                        chr_pos.x += font_offset.x;
                        chr_pos.y += ysize * (font_offset_i_y - fc.offset);
                        if (style.pixel_align)
                            D.alignScreenXToPixel(chr_pos.x);

                        Rect_LU rect(chr_pos, xsize * fc.width_padd, ysize * fc.height_padd);
                        VI.imageConditional(&font->_images[fc.image], *shader_image);
                        if (sub_pixel)
                            VI.imagePart(rect, fc.tex);
                        else
                            VI.font(rect, fc.tex);
                    }
                }
#endif
            }

        skip_combining:
            if (!--max_length)
                goto end; // !! THIS CAN CHECK BEFORE ADVANCING 'text' AS LONG AS WE'RE NOT GOING TO USE IT AFTERWARD !!
            n = text.n();
            if (CharFlagFast(n) & CHARF_SKIP) {
#if !SUPPORT_EMOJI
            skip_multi1:
#endif
                // update positions
                if (cur == offset) {
                    cur_x = pos.x;
                    cur_w = xsize * charWidth(chr);
                }
                if (sel == offset) {
                    sel_x = pos.x;
                }
                offset++;

                goto skip_combining;
            }
            chr = n;
            goto loop;
        }
    end:
        if (prev_chr) {
            advance('\0');
            prev_chr = '\0';
        }

        // selection
        if (sel >= 0 && cur >= 0) // have some selection
        {
            Int min, max;
            Rect rect;
            if (sel < cur) {
                min = sel;
                rect.min.x = sel_x;
                max = cur;
                rect.max.x = cur_x;
            } else {
                min = cur;
                rect.min.x = cur_x;
                max = sel;
                rect.max.x = sel_x;
            }
            if (max > start_offset && min < offset) // if selection intersects with this text
            {
                VI.flush();
                VI.shader(null);

                if (min < start_offset)
                    rect.min.x = x; // if minimum is before the start of this text, then use starting position
                if (max >= offset)
                    rect.max.x = pos.x; // if maximum is after  the end   of this text, then use current  position
                Flt h = style.lineHeight();
                rect.setY(y - h, pos.y);
                D.alignScreenYToPixel(rect.min.y); // use "y-h" instead of "pos.y-h" to get exact value of the next line
                rect.moveY((h - size.y) / 2);      // adjust rectangle so text is at selection center
                rect.draw(style.selection);

                VI.shader(shader);
                VI.color(color);
            }
        }

        // cursor
        if (cur == offset && cur != next_offset) {
            cur_x = pos.x;
            cur_w = size.y / 2;
        } // allow drawing cursor at the end only if it's not going to be drawn in the next line
        if (cur_w >= 0 && !Kb._cur_hidden && App.active()) // it's faster to check 'App.active' here instead of adjusting 'Kb._cur_hidden' all the time based on 'App.active'
        {
            VI.flush();
            VI.shader(null);

            if (style.edit->overwrite && sel < 0)
                DrawKeyboardCursorOverwrite(Vec2(cur_x, pos.y), size.y, spacingConst() ? Max(space, cur_w) : cur_w); // draw overwrite only without selection
            else
                DrawKeyboardCursor(Vec2(cur_x, pos.y), size.y);

            VI.shader(shader);
            VI.color(color);
        }
    }
    void draw(C TextStyleParams &style, Flt x, Flt y, TextSrc text, C StrData *data, Int datas) {
        if (text.is() || VisibleData(data, datas) || (style.edit && (style.edit->cur >= 0 || style.edit->sel >= 0)))
            if (init(style)) {
                text.fix();
                setColor(style.color); // after 'sub_pixel'
                setShadow(style.shadow);
                setFont(default_font); // before 'width'
                _draw(style, x, y, text, data, datas, -1, null);
                shut();
            }
    }
    void _draw(C TextStyleParams &style, C Rect &rect, Memc<TextSplit> &splits) {
        Flt h = style.lineHeight();
        Vec2 p(rect.lerpX(style.align.x * -0.5f + 0.5f),
               Lerp(rect.min.y + (splits.elms() - 1) * h, rect.max.y, style.align.y * -0.5f + 0.5f));
        if (style.pixel_align)
            D.alignScreenToPixel(p); // align here to prevent jittering between lines when moving the whole text

        C Rect &clip = D._clip ? D._clip_rect : D.viewRect();
        Vec2 range;
        style.posY(p.y, range);
        Int start = Max(0, Trunc((range.y - clip.max.y) / h)), // can use 'Trunc' instead of 'Floor' because we use "Max(0"
            end = Min(splits.elms() - 1, Ceil((range.x - clip.min.y) / h));

#if DEBUG && 0
        D.clip(null);
        D.lineX(RED, range.y, -D.w(), D.w());                           // max
        D.lineX(RED, range.x - (splits.elms() - 1) * h, -D.w(), D.w()); // min
        D.text(0, D.h() * 0.9f, S + start + ' ' + end);
#endif

        if (start <= end) {
            initDraw(style);
            TextSplit *split = &splits[start];
            setColor(split->color); // after 'sub_pixel'
            setShadow(split->shadow);
            setFont(split->font); // before 'width'
        next:
            TextSplit *next = splits.addr(start + 1);
            _draw(style, p.x, p.y - start * h, split->text, split->data, split->datas, split->length, split->panel, split->offset, next ? next->offset : -1);
            if (++start <= end) {
                split = next;
                changeColor(split->color);
                changeShadow(split->shadow);
                if (split->font != font)
                    setFont(split->font);
                goto next;
            }
            shut();
        }
    }
    void draw(C TextStyleParams &style, C Rect &rect, TextSrc text, C StrData *data, Int datas, Bool auto_line, C Flt *width = null) {
        if (text.is() || VisibleData(data, datas) || (style.edit && (style.edit->cur >= 0 || style.edit->sel >= 0))) {
            auto &splits = TextSplits; // HW drawing functions can be called only on the main thread under 'D._lock' so we can use just 1 global
            split(splits, style, width ? *width : rect.w(), auto_line, text, data, datas);
            if (splits.elms())
                _draw(style, rect, splits);
        }
    }
};
/******************************************************************************/
struct TextDrawerSoft : TextDrawer {
    struct Rect2 {
        Rect src, dest;
    };
    Int rects, mip_map;
    Flt contrast, shadow;
    Vec4 color;
    RectI clip;
    Rect2 rect[65536 / SIZE(Rect2)];
    C Image *src;
    Image *dest;

    void setFont(C Font *font) {
        super::setFont(font);

        Flt max_size = Max(Abs(xsize), Abs(ysize)), mip_map = 0.5f * Log2(Sqr(1 / max_size));
        T.mip_map = Max(0, Round(mip_map));

        CHS(ysize);
        CHS(font_offset.y); // #TextSoft
    }
    void setColor(C Color &color) { T.color = super::color = color; }
    void setShadow(Byte shadow) { T.shadow = ByteToFlt(super::shadow = shadow); }

    void changeColor(C Color &color) {
        if (super::color != color) {
            flush();
            setColor(color);
        }
    }
    void changeShadow(Byte shadow) {
        if (super::shadow != shadow) {
            flush();
            setShadow(shadow);
        }
    }

    void drawImage(C Image &image, C Rect &rect) C {
        Rect uv(0, 0, 1, 1);
        Vec2 s = image.size() / rect.size() * uv.size();
        Flt mip_map = 0.5f * Log2(Sqr(s).max());
        if (image.lockRead(Mid(Round(mip_map), 0, image.mipMaps() - 1))) // for higher precision, this could use some bigger mip (maybe something like Floor(mip_map), Floor(mip_map)-1, Round(mip_map)-1) and 'areaColorF*' instead of 'colorF*'
        {
            RectI dest(Trunc(rect.min.x), Trunc(rect.min.y), Ceil(rect.max.x), Ceil(rect.max.y));
            dest &= clip;
            Vec2 mul_add_x((uv.max.x - uv.min.x) / (rect.max.x - rect.min.x) * image.lw(),
                           (uv.min.x + (uv.max.x - uv.min.x) * (0.5f - rect.min.x) / (rect.max.x - rect.min.x)) * image.lw() - 0.5f);
            Vec2 mul_add_y((uv.max.y - uv.min.y) / (rect.max.y - rect.min.y) * image.lh(),
                           (uv.min.y + (uv.max.y - uv.min.y) * (0.5f - rect.min.y) / (rect.max.y - rect.min.y)) * image.lh() - 0.5f);

            for (Int y = dest.min.y; y < dest.max.y; y++) {
                Flt ty = y * mul_add_y.x + mul_add_y.y;
                for (Int x = dest.min.x; x < dest.max.x; x++) {
                    Flt tx = x * mul_add_x.x + mul_add_x.y;
                    Vec4 c = image.colorFCubicFast(tx, ty, true, true); // 'colorFCubicFastSharp' was too sharp, alternatively 'colorFLinear' can be used
                    T.dest->mergeF(x, y, c);
                }
            }
            image.unlock();
        }
    }
    void flush() {
        FREP(rects) {
            C Rect2 &r = rect[i];
            RectI dest;
            dest.set(Trunc(r.dest.min.x), Trunc(r.dest.max.y), Ceil(r.dest.max.x), Ceil(r.dest.min.y));
            dest &= clip;
            Vec2 mul_add_x((r.src.max.x - r.src.min.x) / (r.dest.max.x - r.dest.min.x) * src->lw(),
                           (r.src.min.x + (r.src.max.x - r.src.min.x) * (0.5f - r.dest.min.x) / (r.dest.max.x - r.dest.min.x)) * src->lw() - 0.5f);
            Vec2 mul_add_y((r.src.min.y - r.src.max.y) / (r.dest.max.y - r.dest.min.y) * src->lh(),
                           (r.src.max.y + (r.src.min.y - r.src.max.y) * (0.5f - r.dest.min.y) / (r.dest.max.y - r.dest.min.y)) * src->lh() - 0.5f);

            for (Int y = dest.min.y; y < dest.max.y; y++) {
                // Flt fy=LerpR(r.dest.min.y, r.dest.max.y, y+0.5f); // fy=(y+0.5f - r.dest.min.y)/(r.dest.max.y - r.dest.min.y)
                // Flt ty=Lerp (r.src .max.y, r.src .min.y, fy)*src->lh()-0.5f; // ty=(r.src.max.y + (r.src.min.y-r.src.max.y) * fy)*src->lh()-0.5f
                //  ty=(r.src.max.y + (r.src.min.y-r.src.max.y) * (y+0.5f - r.dest.min.y)/(r.dest.max.y - r.dest.min.y))*src->lh()-0.5f
                //  ty=y *                (r.src.min.y-r.src.max.y)                         / (r.dest.max.y - r.dest.min.y)  * src->lh()
                //       + (r.src.max.y + (r.src.min.y-r.src.max.y) * (0.5f - r.dest.min.y) / (r.dest.max.y - r.dest.min.y)) * src->lh() - 0.5f
                Flt ty = y * mul_add_y.x + mul_add_y.y;
                for (Int x = dest.min.x; x < dest.max.x; x++) {
                    // Flt fx=LerpR(r.dest.min.x, r.dest.max.x, x+0.5f);
                    // Flt tx=Lerp (r.src .min.x, r.src .max.x, fx)*src->lw()-0.5f;
                    Flt tx = x * mul_add_x.x + mul_add_x.y;

                    Vec4 c = src->colorFLinear(tx, ty);

                    if (sub_pixel) {
                        c *= color.w;
                        Vec4 d = T.dest->colorF(x, y), out;
                        out.xyz = c.xyz * color.xyz + d.xyz * (1 - c.xyz);
                        out.w = c.w + d.w * (1 - c.w);
                        T.dest->colorF(x, y, out);
                    } else {
                        // #FontImageLayout
                        Flt a = Min(c.x * contrast, 1), // font opacity, scale up by 'contrast' to improve quality when font is very small
                            s = c.y * shadow;           // font shadow

                        // Flt final_alpha=1-(1-s)*(1-a);
                        // 1-(1-s)*(1-a)
                        // 1-(1-a-s+sa)
                        // 1-1+a+s-sa
                        // a + s - s*a
                        Flt final_alpha = a + s - s * a;

                        // ALPHA_MERGE:
                        Vec4 out;
                        out.xyz = color.xyz * (a * color.w);
                        out.w = color.w * final_alpha;

                        T.dest->mergeF(x, y, out);
                    }
                }
            }
        }
        rects = 0;
    }
    void unlock() {
        if (src) {
            flush();
            src->unlock();
        }
    }
    Bool drawChr(C Image &image, C Rect &rect_src, C Rect &rect_dest) {
        if (src != &image) {
            unlock();
            if (image.lockRead(Min(mip_map, image.mipMaps() - 1))) {
                src = &image;
            } else {
                src = null;
                return false;
            }
        }
        if (!InRange(rects, rect))
            flush();
        Rect2 &r = rect[rects++];
        r.src = rect_src;
        r.dest = rect_dest;
        return true;
    }

    void initDraw(C TextStyleParams &style, Image &dest) {
        super::initDraw(style);

        rects = 0;
        T.src = null;
        T.dest = &dest;
        clip.set(0, 0, dest.w(), dest.h());

        // font params
        Byte lum = style.color.lum();
        if (!lum)
            contrast = 1;
        else {
            Flt pixels = size.min();
            if (pixels >= 32)
                contrast = 1;
            else {
                contrast = 32 / pixels;
                contrast = Sqrt(contrast); // or alternative: contrast=Log2(contrast+1);
                contrast = Max(1, contrast);
                contrast = Lerp(1.0f, contrast, ByteToFlt(lum));
            }
        }
    }
    Bool init(C TextStyleParams &style, Image &dest) {
        if (initFast(style)) {
            initDraw(style, dest);
            return true;
        }
        return false;
    }
    void shut() {
        unlock();
    }

    void processPanel(C TextSrc &text, C StrData *data, Int datas, Int max_length) {
        Flt w = panelWidth(text, data, datas, max_length);
        Flt panel_padd_l, panel_padd_r;
        panelPadd(*panel, panel_padd_l, panel_padd_r);
        w += panel_padd_l + panel_padd_r;
        if (w < size.y) // require min square
        {
            Flt d = (size.y - w) / 2;
            w = size.y;
            panel_padd_l += d;
            panel_padd_r += d;
        }
        if (spacingConst()) {
            Flt o = Max(w, space);
            panel_r = pos.x + o;
            pos.x += (o - w) / 2;
        } else {
            panel_r = pos.x + w + space;
        } // calculate panel right side, it will be used to set position after finishing drawing it, this is needed to always apply 'space' after that panel, regardless if it's empty or not
          // panel->draw(Rect_LU(pos, w, size.y)); FIXME #TextSoft
        pos.x += panel_padd_l;
    }

    void _draw(C TextStyleParams &style, Flt x, Flt y, TextSrc text, C StrData *data, Int datas, Int max_length, C PanelImage *start_panel) // assumes: 'text.fix'
    {
        panel = start_panel;

        Flt total_width = (Equal(x_align_mul, 0) ? 0 : width(text, data, datas, max_length)); // don't calculate 'width' when not needed, AFTER 'setFont' and 'panel'
        x += total_width * x_align_mul;
        y -= size.y * y_align_mul; // #TextSoft

        pos.set(x, y);
        if (style.pixel_align)
            pos = Round(pos); // #TextSoft

        if (panel)
            processPanel(text, data, datas, max_length);

        if (max_length) {
            Char n, chr = text.c();
        loop:
            if (!chr) {
            next_data:
                if (datas <= 0)
                    goto end;
                datas--;
                C StrData *d = data++;
                switch (d->type) {
                case StrData::COLOR:
                    changeColor(d->color);
                    break;
                case StrData::COLOR_OFF:
                    changeColor(style.color);
                    break;
                case StrData::SHADOW:
                    changeShadow(d->shadow);
                    break;
                case StrData::SHADOW_OFF:
                    changeShadow(style.shadow);
                    break;
                case StrData::TEXT:
                    if (d->text.is()) // process only if have anything, if not then proceed to 'next_data'
                    {
                        text = d->text();
                        chr = text.c();
                        goto have_char;
                    }
                    break;

                case StrData::IMAGE: {
                    if (prev_chr) {
                        advance('\0');
                        prev_chr = '\0';
                    }

                    C Image *image = d->image();
                    if (image && image->is()) {
                        flush();

                        Flt w = size.y * image->aspect();
                        Flt x = pos.x;
                        if (spacingConst()) {
                            Flt o = Max(w, space);
                            pos.x += o;
                            x += (o - w) / 2;
                        } else {
                            pos.x += w + space;
                        }
                        drawImage(*image, Rect_LD(x, pos.y, w, size.y)); // #TextSoft
                    } else {
                        pos.x += space;
                    }
                    if (!--max_length)
                        goto end;
                } break;

                case StrData::PANEL: {
                    if (prev_chr) {
                        advance('\0');
                        prev_chr = '\0';
                    }
                    if (panel)
                        pos.x = panel_r;
                    if (panel = d->panel()) {
                        processPanel(text, data, datas, max_length);
                        if (!max_length)
                            goto end;
                    }
                } break;

                case StrData::FONT: {
                    C Font *new_font = d->font();
                    if (!new_font)
                        new_font = default_font;
                    if (font != new_font) {
                        if (prev_chr) {
                            advance('\0');
                            prev_chr = '\0';
                        }
                        setFont(new_font);
                    }
                } break;
                }
                goto next_data;
            }
        have_char:

            if (prev_chr)
                advance(chr);
            prev_chr = chr;

            // draw character
            if (chr != ' ') // don't draw space sign, potentially we could check it below using "fc.height" instead, however this is faster
            {
                auto font_index = font->_wide_to_font[U16(chr)];
                if (InRange(font_index, font->_chrs)) {
                    C Font::Chr &fc = font->_chrs[font_index];
                    // if(fc.height) // potentially we could check for empty characters (full width space, half width space, nbsp, however in most cases we will have something to draw, so this check will slow down things
                    {
                        Vec2 chr_pos = pos;
                        if (spacingConst())
                            chr_pos.x += space_2 - xsize_2 * fc.width; // move back by half of the character width
                        chr_pos.x += font_offset.x;
                        chr_pos.y += ysize * (font_offset_i_y - fc.offset);
                        if (style.pixel_align)
                            chr_pos.x = Round(chr_pos.x); // #TextSoft

                        Rect_LU rect(chr_pos, xsize * fc.width_padd, ysize * fc.height_padd);
                        drawChr(font->_images[fc.image], fc.tex, rect); // #TextSoft

                        // combining
                        if (!--max_length)
                            goto end; // !! THIS CAN CHECK BEFORE ADVANCING 'text' AS LONG AS WE'RE NOT GOING TO USE IT AFTERWARD !!
                        n = text.n();
                        UInt flag = CharFlagFast(n);
                        if (flag & (CHARF_COMBINING | (SUPPORT_EMOJI ? 0 : CHARF_MULTI1))) // without SUPPORT_EMOJI also check CHARF_MULTI1
                        {
#if !SUPPORT_EMOJI
                            if (flag & CHARF_MULTI1)
                                goto skip_combining;
#endif
                            chr_pos.x += xsize_2 * fc.width; // after 'chr_pos' was pixel aligned, move by half of the character width to put it at centerX of 'chr' character
                            chr_pos.y += ysize * fc.offset;  // reset Y pos
                            Bool skipped_bottom_shadow_padding = false;

                        draw_combining:
                            auto font_index = font->_wide_to_font[U16(n)];
                            if (InRange(font_index, font->_chrs)) {
                                C Font::Chr &fc = font->_chrs[font_index];

                                if ((flag & CHARF_STACK) && !skipped_bottom_shadow_padding) // if found a first stacking character, then skip shadow padding at the bottom, because next character is drawn after base character and on top, so its bottom shadow must not overlap the base
                                {
                                    skipped_bottom_shadow_padding = true; // do this only once
                                    chr_pos.y += ysize * font->paddingB();
                                }

                                Vec2 n_pos = chr_pos;
                                n_pos.x -= xsize_2 * fc.width; // move back by half of the character width
                                n_pos.y -= ysize * fc.offset;
                                // if(style.pixel_align)n_pos.x=Round(n_pos.x); 'n_pos' based on 'chr_pos' which was already aligned #TextSoft

                                Flt n_height_padd = ysize * fc.height_padd;
                                Rect_LU rect(n_pos, xsize * fc.width_padd, n_height_padd);
                                drawChr(font->_images[fc.image], fc.tex, rect); // #TextSoft
                                if (flag & CHARF_STACK)
                                    chr_pos.y += n_height_padd; // if the drawn character is stacking, then move position higher for next character, to stack combining characters on top of each other (needed for THAI)
                            }

                            if (!--max_length)
                                goto end; // !! THIS CAN CHECK BEFORE ADVANCING 'text' AS LONG AS WE'RE NOT GOING TO USE IT AFTERWARD !!
                            n = text.n();
                            flag = CharFlagFast(n);
                            if (flag & CHARF_COMBINING)
                                goto draw_combining; // if next character is combining too
                        }
                        chr = n;
                        goto loop;
                    }
                }
#if SUPPORT_EMOJI
                // FIXME #TextSoft
#endif
            }

        skip_combining:
            if (!--max_length)
                goto end; // !! THIS CAN CHECK BEFORE ADVANCING 'text' AS LONG AS WE'RE NOT GOING TO USE IT AFTERWARD !!
            n = text.n();
            if (CharFlagFast(n) & CHARF_SKIP)
                goto skip_combining;
            chr = n;
            goto loop;
        }
    end:
        if (prev_chr) {
            advance('\0');
            prev_chr = '\0';
        }
    }
    void draw(C TextStyleParams &style, Image &image, Flt x, Flt y, TextSrc text, C StrData *data, Int datas) {
        if (text.is() || VisibleData(data, datas))
            if (init(style, image)) {
                text.fix();
                setColor(style.color); // after 'sub_pixel'
                setShadow(style.shadow);
                setFont(default_font); // before 'width'
                _draw(style, x, y, text, data, datas, -1, null);
                shut();
            }
    }
    void draw(C TextStyleParams &style, Image &image, C Rect &rect, TextSrc text, C StrData *data, Int datas, Bool auto_line) {
        if (text.is() || VisibleData(data, datas)) {
            Memt<TextSplit> splits; // use temp because this can be called outside of Draw #TextSoft
            split(splits, style, rect.w(), auto_line, text, data, datas);
            if (splits.elms()) {
                Flt h = style.lineHeight();
                Vec2 p(rect.lerpX(style.align.x * -0.5f + 0.5f),
                       Lerp(rect.max.y - (splits.elms() - 1) * h, rect.min.y, style.align.y * -0.5f + 0.5f)); // #TextSoft
                if (style.pixel_align)
                    p = Round(p); // align here to prevent jittering between lines when moving the whole text

                // #TextSoft
                Rect clip(0, image.size());
                Vec2 range;
                style.posYI(p.y, range);
                Int start = Max(0, Trunc((clip.min.y - range.x) / h)), // can use 'Trunc' instead of 'Floor' because we use "Max(0"
                    end = Min(splits.elms() - 1, Ceil((clip.max.y - range.y) / h));

                if (start <= end) {
                    initDraw(style, image);
                    TextSplit *split = &splits[start];
                    setColor(split->color); // after 'sub_pixel'
                    setShadow(split->shadow);
                    setFont(split->font); // before 'width'
                next:
                    TextSplit *next = splits.addr(start + 1);
                    _draw(style, p.x, p.y + start * h, split->text, split->data, split->datas, split->length, split->panel); // #TextSoft
                    if (++start <= end) {
                        split = next;
                        changeColor(split->color);
                        changeShadow(split->shadow);
                        if (split->font != font)
                            setFont(split->font);
                        goto next;
                    }
                    shut();
                }
            }
        }
    }
};
/******************************************************************************/
Font *TextStyleParams::getFont() C {
    if (_font)
        return _font;
    if (GuiSkin *skin = Gui.skin())
        return skin->font(); // copy to temp 'skin' in case of multi-thread issues
    return null;
}
Flt TextStyleParams::posY(Flt y) C {
    if (C Font *font = getFont()) {
        Flt ysize = size.y / font->height(), // height of 1 font texel
            y_align_mul = align.y * 0.5f + 0.5f;
        y += size.y * y_align_mul + ysize * font->paddingT();
    }
    return y;
}
void TextStyleParams::posY(Flt y, Vec2 &range) C {
    if (C Font *font = getFont()) {
        Flt ysize = size.y / font->height(), // height of 1 font texel
            y_align_mul = align.y * 0.5f + 0.5f;
        y += size.y * y_align_mul;
        range.x = y - size.y - ysize * font->paddingB(); // min
        range.y = y + ysize * font->paddingT();          // max
    }
}
void TextStyleParams::posYI(Flt y, Vec2 &range) C {
    if (C Font *font = getFont()) {
        Flt ysize = size.y / font->height(), // height of 1 font texel
            y_align_mul = align.y * 0.5f + 0.5f;
        y -= size.y * y_align_mul;
        range.x = y - ysize * font->paddingT();          // min
        range.y = y + size.y + ysize * font->paddingB(); // max
    }
}
void TextStyleParams::setPerPixelSize() {
    if (C Font *font = getFont())
        size = D.pixelToScreenSize(font->height());
}
/******************************************************************************/
Flt TextStyleParams::textWidth(C Str &str, Int max_length) C {
    if (max_length && str.is())
        if (C Font *font = getFont()) {
            if (max_length >= str.length())
                max_length = -1; // if 'max_length' specified and >= than what we have, then change to unlimited mode because that will be faster

            Flt xsize = size.x / font->height(),
                space = size.x * T.space.x;
            Int spacings, width = font->textWidth(spacings, spacing, str, max_length);
            return width * xsize + space * spacings;
        }
    return 0;
}
Flt TextStyleParams::textWidth(C Str8 &str, Int max_length) C {
    if (max_length && str.is())
        if (C Font *font = getFont()) {
            if (max_length >= str.length())
                max_length = -1; // if 'max_length' specified and >= than what we have, then change to unlimited mode because that will be faster

            Flt xsize = size.x / font->height(),
                space = size.x * T.space.x;
            Int spacings, width = font->textWidth(spacings, spacing, str, max_length);
            return width * xsize + space * spacings;
        }
    return 0;
}
Flt TextStyleParams::textWidth(CChar *text, Int max_length) C {
    if (max_length && Is(text))
        if (C Font *font = getFont()) {
            Flt xsize = size.x / font->height(),
                space = size.x * T.space.x;
            Int spacings, width = font->textWidth(spacings, spacing, text, max_length);
            return width * xsize + space * spacings;
        }
    return 0;
}
Flt TextStyleParams::textWidth(CChar8 *text, Int max_length) C {
    if (max_length && Is(text))
        if (C Font *font = getFont()) {
            Flt xsize = size.x / font->height(),
                space = size.x * T.space.x;
            Int spacings, width = font->textWidth(spacings, spacing, text, max_length);
            return width * xsize + space * spacings;
        }
    return 0;
}
Flt TextStyleParams::textWidth(CChar *text, C StrData *data, Int datas) C {
    Int max_length = -1;
    if (max_length) {
        TextProcessor tp;
        if (tp.initFast(T)) {
            TextSrc ts = (text ? text : u""); // same as ts.fix();
            tp.setFontFast(tp.default_font);  // before 'width'
            tp.panel = null;
            return tp._width(ts, data, datas, max_length, false);
        }
    }
    return 0;
}
/******************************************************************************/
Int TextStyleParams::textIndex(CChar8 *text, Flt x, TEXT_INDEX_MODE index_mode) C {
    Int pos = 0;
    if (Is(text))
        if (C Font *font = getFont()) {
            Flt space = size.x * T.space.x;
            if (spacingConst()) {
                x /= space;
                if (index_mode == TEXT_INDEX_DEFAULT)
                    x += 0.5f;
                pos = Trunc(x);
#if 0 // fast
         Clamp(pos, 0, Length(text));
#else // CHARF_SKIP
                CChar8 *start = text;
                for (Int base_chars = 0;; base_chars++) {
                    Char8 c = *text;
                    if (!c || base_chars >= pos) {
                        pos = text - start;
                        break;
                    }
                skip:
                    Char8 next = *++text;
                    if (CharFlagFast(next) & CHARF_SKIP)
                        goto skip;
                }
#endif
            } else
                for (Flt xsize = size.x / font->height(), space_2 = space / 2;;) {
                    Char8 c = *text;
                    if (!c)
                        break;
                    CChar8 *start = text;
                skip1:
                    Char8 next = *++text;
                    if (CharFlagFast(next) & CHARF_SKIP)
                        goto skip1;
                    Flt w = font->charWidth(c, next, spacing) * xsize;
                    if (x < ((index_mode == TEXT_INDEX_DEFAULT) ? w / 2 : (index_mode == TEXT_INDEX_OVERWRITE) ? w + space_2
                                                                                                               : xsize * font->charWidth(c)))
                        break; // for TEXT_INDEX_FIT make sure that the 'c' fully fits
                    x -= w + space;
                    pos += text - start; // advance by how many characters were processed
                }
        }
    return pos;
}
Int TextStyleParams::textIndex(CChar *text, Flt x, TEXT_INDEX_MODE index_mode) C {
    Int pos = 0;
    if (Is(text))
        if (C Font *font = getFont()) {
            Flt space = size.x * T.space.x;
            if (spacingConst()) {
                x /= space;
                if (index_mode == TEXT_INDEX_DEFAULT)
                    x += 0.5f;
                pos = Trunc(x);
#if 0 // fast
         Clamp(pos, 0, Length(text));
#else // CHARF_SKIP
                CChar *start = text;
                for (Int base_chars = 0;; base_chars++) {
                    Char c = *text;
                    if (!c || base_chars >= pos) {
                        pos = text - start;
                        break;
                    }
                skip:
                    Char next = *++text;
                    if (CharFlagFast(next) & CHARF_SKIP)
                        goto skip;
                }
#endif
            } else
                for (Flt xsize = size.x / font->height(), space_2 = space / 2;;) {
                    Char c = *text;
                    if (!c)
                        break;
                    CChar *start = text;
                skip1:
                    Char next = *++text;
                    if (CharFlagFast(next) & CHARF_SKIP)
                        goto skip1;
                    Flt w = font->charWidth(c, next, spacing) * xsize;
                    if (x < ((index_mode == TEXT_INDEX_DEFAULT) ? w / 2 : (index_mode == TEXT_INDEX_OVERWRITE) ? w + space_2
                                                                                                               : xsize * font->charWidth(c)))
                        break; // for TEXT_INDEX_FIT make sure that the 'c' fully fits
                    x -= w + space;
                    pos += text - start; // advance by how many characters were processed
                }
        }
    return pos;
}
/******************************************************************************/
Int TextStyleParams::textIndex(CChar *text, Flt x, Flt y, TEXT_INDEX_MODE index_mode, Flt width, Bool auto_line, Bool &eol) C // 'eol' calculation is needed for 2 lines, where cursor could be set at end of 1st or start of 2nd, for double-click we need to know which one
{
#if 1
    Int line = Max(0, Trunc(y / lineHeight()));
#else
    Int line = Floor(y / lineHeight());
    if (line < 0)
        goto end;
#endif
    TextProcessor tp;
    if (tp.initSplit(T, width)) {
        TextSrc ts = (text ? text : u""); // same as ts.fix();
        C StrData *data = null;
        Int datas = 0;
        Int offset = 0, lines = 0;
        TextSplit split;
    next:
        if (!tp.splitNext(split, T, width, auto_line, ts, data, datas, offset) || line == lines) {
            Int i = textIndex(split.text.t16, x, index_mode);
            split.fixLength();
            if (eol = (i >= split.length))
                i = split.length; // yes this must check ">=" and not ">"
            return i + split.offset;
        }
        lines++;
        goto next;
        // eol=true; split.fixLength(); return split.end();
    }
    // end:
    eol = false;
    return 0;
}
Int TextStyleParams::textIndex(CChar *text, C StrData *data, Int datas, Flt x, Flt y, TEXT_INDEX_MODE index_mode, Flt width, Bool auto_line, Bool &eol) C // 'eol' calculation is needed for 2 lines, where cursor could be set at end of 1st or start of 2nd, for double-click we need to know which one
{
#if 1
    Int line = Max(0, Trunc(y / lineHeight()));
#else
    Int line = Floor(y / lineHeight());
    if (line < 0)
        goto end;
#endif
    TextProcessor tp;
    if (tp.initSplit(T, width)) {
        TextSrc ts = (text ? text : u""); // same as ts.fix();
        Int offset = 0, lines = 0;
        TextSplit split;
    next:
        if (!tp.splitNext(split, T, width, auto_line, ts, data, datas, offset) || line == lines) {
            Int i = tp.textIndex(T, x, index_mode, split.text, split.data, split.datas, split.font, split.panel);
            split.fixLength();
            if (eol = (i >= split.length))
                i = split.length; // yes this must check ">=" and not ">"
            return i + split.offset;
        }
        lines++;
        goto next;
        // eol=true; split.fixLength(); return split.end();
    }
    // end:
    eol = false;
    return 0;
}
Int TextStyleParams::textIndexAlign(CChar *text, C StrData *data, Int datas, Flt x, Flt y, TEXT_INDEX_MODE index_mode, C Vec2 &size, Bool auto_line, Bool clamp, Bool &eol) C {
    Flt width = size.x;
    TextDrawer tp;
    if (tp.initSplit(T, width)) {
        tp.initDraw(T);

        Bool align_x = !Equal(align.x, 1);
        if (align_x) {
            Flt p_x = size.x * (align.x * -0.5f + 0.5f);
            x -= p_x;
        }
        Flt h = lineHeight();
        Int lines = -1; // unknown
        if (!Equal(align.y, -1)) {
            lines = textLines(text, data, datas, size.x, auto_line); // here use 'size.x' instead of 'width' because 'width' got adjusted
            Flt p_y = Lerp(-size.y + (lines - 1) * h, 0, align.y * -0.5f + 0.5f) + T.size.y * tp.y_align_mul;
            y -= p_y;
        }
        Int line;
        if (clamp)
            line = Max(Trunc(-y / h), 0);
        else {
            line = Floor(-y / h);
            if (line < 0 || (lines >= 0 && line >= lines))
                goto end;
        } // if line is out of range, >= check only if 'lines' are known, yes this must check "line>=" and not "line>"
        lines = 0;
        Int offset = 0;
        TextSrc ts = (text ? text : u""); // same as ts.fix();
        TextSplit split;
    next:
        Bool next = tp.splitNext(split, T, width, auto_line, ts, data, datas, offset);
        if (!next && clamp // last line
            || line == lines) {
            Flt ax = x;
            if (align_x) {
                Flt total_width = tp.width(split);
                ax -= total_width * tp.x_align_mul;
                if (!clamp && ax > total_width)
                    goto end;
            }
            if (!clamp && ax < 0)
                goto end;
            Int i = tp.textIndex(T, ax, index_mode, split.text, split.data, split.datas, split.font, split.panel);
            split.fixLength();
            if (eol = (i >= split.length)) // yes this must check ">=" and not ">"
            {
                if (!clamp)
                    goto end;
                i = split.length;
            }
            return i + split.offset;
        }
        if (next) {
            lines++;
            goto next;
        }
    }
end:
    eol = false;
    return -1;
}
/******************************************************************************/
Vec2 TextStyleParams::textPos(CChar *text, Int index, Flt width, Bool auto_line) C {
    if (index > 0 && Is(text)) {
        TextProcessor tp;
        if (tp.initSplit(T, width)) {
            TextSrc ts = text; // ts.fix(); NOT NEEDED, ALREADY CHECKED ABOVE
            C StrData *data = null;
            Int datas = 0;
            Int offset = 0, line = 0;
            TextSplit splits[2], *split = &splits[0], *next = &splits[1];
            if (!tp.splitNext(*split, T, width, auto_line, ts, data, datas, offset)) // reached end
            {
            ret:
                Vec2 pos(0, line * lineHeight());
                if (index > split->offset) // skip <0 and 0
                {
                    index -= split->offset;
                    pos.x = textWidth(split->text.t16, index);
                    if (pos.x && !spacingConst())
                        pos.x += tp.space;
                }
                return pos;
            }
        next:
            Bool ok = tp.splitNext(*next, T, width, auto_line, ts, data, datas, offset);
            if (index < next->offset)
                goto ret; // allow drawing cursor at the end only if it's not going to be drawn in the next line
            line++;
            Swap(split, next);
            if (!ok)
                goto ret;
            goto next;
        }
    }
    return 0;
}
Vec2 TextStyleParams::textPos(CChar *text, C StrData *data, Int datas, Int index, Flt width, Bool auto_line) C {
    if (index > 0 && (Is(text) || VisibleData(data, datas))) {
        TextProcessor tp;
        if (tp.initSplit(T, width)) {
            TextSrc ts = (text ? text : u""); // same as ts.fix();
            Int offset = 0, line = 0;
            TextSplit splits[2], *split = &splits[0], *next = &splits[1];
            if (!tp.splitNext(*split, T, width, auto_line, ts, data, datas, offset)) // reached end
            {
            ret:
                Vec2 pos(0, line * lineHeight());
                if (index > split->offset) // skip <0 and 0
                {
                    index -= split->offset;
                    pos.x = tp.widthNoRestore(*split, index);
                    if (pos.x && !spacingConst())
                        pos.x += tp.space;
                }
                return pos;
            }
        next:
            Bool ok = tp.splitNext(*next, T, width, auto_line, ts, data, datas, offset);
            if (index < next->offset)
                goto ret; // allow drawing cursor at the end only if it's not going to be drawn in the next line
            line++;
            Swap(split, next);
            if (!ok)
                goto ret;
            goto next;
        }
    }
    return 0;
}
/******************************************************************************/
Int TextStyleParams::textLines(CChar8 *text, C StrData *data, Int datas, Flt width, Bool auto_line, Flt *actual_width) C {
    Int lines = 0;
    Flt w, *width_ptr = null;
    if (actual_width) {
        *actual_width = 0;
        width_ptr = &w;
    }
    TextProcessor tp;
    if (tp.initSplit(T, width)) {
        TextSrc ts = (text ? text : ""); // same as ts.fix();
        Int offset = 0;
        TextSplit split;
    next:
        Bool next = tp.splitNext(split, T, width, auto_line, ts, data, datas, offset, width_ptr);
        lines++;
        if (actual_width)
            MAX(*actual_width, w);
        if (next)
            goto next;
    }
    return lines;
}
Int TextStyleParams::textLines(CChar *text, C StrData *data, Int datas, Flt width, Bool auto_line, Flt *actual_width) C {
    Int lines = 0;
    Flt w, *width_ptr = null;
    if (actual_width) {
        *actual_width = 0;
        width_ptr = &w;
    }
    TextProcessor tp;
    if (tp.initSplit(T, width)) {
        TextSrc ts = (text ? text : u""); // same as ts.fix();
        Int offset = 0;
        TextSplit split;
    next:
        Bool next = tp.splitNext(split, T, width, auto_line, ts, data, datas, offset, width_ptr);
        lines++;
        if (actual_width)
            MAX(*actual_width, w);
        if (next)
            goto next;
    }
    return lines;
}
Int TextStyleParams::textLines(CChar *text, Flt width, Bool auto_line, Flt *actual_width) C { return textLines(text, null, 0, width, auto_line, actual_width); }
Flt TextStyleParams::textHeight(CChar *text, Flt width, Bool auto_line, Flt *actual_width) C { return lineHeight() * textLines(text, width, auto_line, actual_width); }
Flt TextStyleParams::textHeight(CChar *text, C StrData *data, Int datas, Flt width, Bool auto_line, Flt *actual_width) C { return lineHeight() * textLines(text, data, datas, width, auto_line, actual_width); }
Flt TextStyleParams::textHeight(CChar8 *text, C StrData *data, Int datas, Flt width, Bool auto_line, Flt *actual_width) C { return lineHeight() * textLines(text, data, datas, width, auto_line, actual_width); }
/******************************************************************************/
TextStyleParams &TextStyleParams::resetColors(Bool gui) {
    // use 'Gui.skin.text.text_style', 'Gui.skin.text_style' if available
    if (GuiSkin *skin = Gui.skin())
        if (TextStyle *text_style = (gui ? skin->text.text_style() : skin->text_style())) {
            shadow = text_style->shadow;
            shade = text_style->shade;
            color = text_style->color;
            image_color = text_style->image_color;
            selection = text_style->selection;
            return T;
        }

    // otherwise set defaults
    shadow = 255;
    shade = DefaultShade;
    color = WHITE;
    image_color = WHITE;
    selection = DefaultSelectionColor;
    return T;
}
TextStyleParams &TextStyleParams::reset(Bool gui) {
    // use 'Gui.skin.text.text_style', 'Gui.skin.text_style' if available
    if (GuiSkin *skin = Gui.skin())
        if (TextStyle *text_style = (gui ? skin->text.text_style() : skin->text_style())) {
            T = *text_style;
            return T;
        }

    // otherwise set defaults
    pixel_align = true;
    align.set(0, 0);
    size.set(0.08f, 0.08f);
    space.set(0.06f, 1);
    spacing = SPACING_NICE;
    shadow = 255;
    shade = DefaultShade;
    color = WHITE;
    image_color = WHITE;
    selection = DefaultSelectionColor;
    _font = null; // keep as null to always use the current value of 'Gui.skin.font'
    edit = null;
    return T;
}
/******************************************************************************/
TextStyle &TextStyle::font(C FontPtr &font) {
    _font = font;
    super::font(_font());
    return T;
}
TextStyle &TextStyle::resetColors(Bool gui) {
    super::resetColors(gui);
    return T;
}
TextStyle &TextStyle::reset(Bool gui) {
    super::reset(gui);
    if (GuiSkin *skin = Gui.skin())
        if (TextStyle *text_style = (gui ? skin->text.text_style() : skin->text_style())) {
            font(text_style->font());
            return T;
        }
    font(null);
    return T;
} // copy to temp 'skin' and 'text_style' in case of multi-thread issues, call 'font' to always make sure that '_font' from 'TextStyle' and 'TextStyleParams' is synchronized
TextStyle::TextStyle() {
    if (GuiSkin *skin = Gui.skin())
        if (TextStyle *text_style = skin->text_style()) {
            font(text_style->font());
            return;
        }
    font(null);
} // copy to temp 'skin' and 'text_style' in case of multi-thread issues, call 'font' to always make sure that '_font' from 'TextStyle' and 'TextStyleParams' is synchronized
/******************************************************************************/
// DRAW
/******************************************************************************/
void DisplayDraw::text(C TextStyleParams &text_style, Flt x, Flt y, CChar8 *text, C StrData *data, Int datas) { TextDrawerHW().draw(text_style, x, y, text, data, datas); }
void DisplayDraw::text(C TextStyleParams &text_style, Flt x, Flt y, CChar *text, C StrData *data, Int datas) { TextDrawerHW().draw(text_style, x, y, text, data, datas); }

void DisplayDraw::text(C TextStyleParams &text_style, C Rect &rect, CChar8 *text, C StrData *data, Int datas, Bool auto_line, C Flt *width) { TextDrawerHW().draw(text_style, rect, text, data, datas, auto_line, width); }
void DisplayDraw::text(C TextStyleParams &text_style, C Rect &rect, CChar *text, C StrData *data, Int datas, Bool auto_line, C Flt *width) { TextDrawerHW().draw(text_style, rect, text, data, datas, auto_line, width); }

void DisplayDraw::text(Flt x, Flt y, CChar8 *text, C StrData *data, Int datas) {
    if (Gui.skin)
        if (C TextStyle *text_style = Gui.skin->text_style())
            TextDrawerHW().draw(*text_style, x, y, text, data, datas);
}
void DisplayDraw::text(Flt x, Flt y, CChar *text, C StrData *data, Int datas) {
    if (Gui.skin)
        if (C TextStyle *text_style = Gui.skin->text_style())
            TextDrawerHW().draw(*text_style, x, y, text, data, datas);
}

void DisplayDraw::text(C Rect &rect, CChar8 *text, C StrData *data, Int datas, Bool auto_line) {
    if (Gui.skin)
        if (C TextStyle *text_style = Gui.skin->text_style())
            TextDrawerHW().draw(*text_style, rect, text, data, datas, auto_line);
}
void DisplayDraw::text(C Rect &rect, CChar *text, C StrData *data, Int datas, Bool auto_line) {
    if (Gui.skin)
        if (C TextStyle *text_style = Gui.skin->text_style())
            TextDrawerHW().draw(*text_style, rect, text, data, datas, auto_line);
}
/******************************************************************************/
void TextStyleParams::drawSoft(Image &image, Flt x, Flt y, CChar *text, C StrData *data, Int datas) C { TextDrawerSoft().draw(T, image, x, y, text, data, datas); }
void TextStyleParams::drawSoft(Image &image, Flt x, Flt y, CChar8 *text, C StrData *data, Int datas) C { TextDrawerSoft().draw(T, image, x, y, text, data, datas); }

void TextStyleParams::drawSoft(Image &image, C Rect &rect, CChar *text, C StrData *data, Int datas, Bool auto_line) C { TextDrawerSoft().draw(T, image, rect, text, data, datas, auto_line); }
void TextStyleParams::drawSoft(Image &image, C Rect &rect, CChar8 *text, C StrData *data, Int datas, Bool auto_line) C { TextDrawerSoft().draw(T, image, rect, text, data, datas, auto_line); }
/******************************************************************************/
void DrawPanelText(C Panel *panel, C Color &panel_color, Flt padding, C TextStyleParams &style, C Vec2 &pos, CChar *text, Bool mouse) {
    if (Is(text)) {
        TextDrawerHW td;
        Flt width;
        auto &splits = TextSplits; // HW drawing functions can be called only on the main thread under 'D._lock' so we can use just 1 global
        td.split(splits, style, D.rectUI().w() - padding * 2, true, text, null, 0, &width);
        if (splits.elms()) {
            Flt height = splits.elms() * style.lineHeight();

            Rect_LU r(pos, width, height);
            r.extend(padding);
            if (mouse) {
                Int height = 32; // default mouse cursor height
                if (Ms._cursor && Ms._cursor->_image)
                    height = Ms._cursor->_image->h() - Ms._cursor->_hot_spot.y;
                Flt y = D.pixelToScreenSize().y * height;
                r.min.y -= y;
                r.max.y -= y;
            }
            if (r.max.x > D.rectUI().max.x) {
                r.min.x -= r.max.x - D.rectUI().max.x;
                r.max.x = D.rectUI().max.x;
            }
            if (r.min.x < D.rectUI().min.x) {
                r.max.x += D.rectUI().min.x - r.min.x;
                r.min.x = D.rectUI().min.x;
            }
            if (r.min.y < D.rectUI().min.y) {
                r += Vec2(0, pos.y - r.min.y + padding);
            }
            if (r.max.y > D.rectUI().max.y) {
                r.min.y -= r.max.y - D.rectUI().max.y;
                r.max.y = D.rectUI().max.y;
            }

            if (panel)
                panel->draw(panel_color, r);
            else if (panel_color.a)
                r.draw(panel_color);

            td._draw(style, r.extend(-padding), splits);
        }
    }
}
/******************************************************************************/
// IO
/******************************************************************************/
#pragma pack(push, 1)
struct TextStyleDesc {
    SPACING_MODE spacing;
    Byte shadow, shade;
    Color color, selection;
    Vec2 align, size, space;
};
#pragma pack(pop)

Bool TextStyle::save(File &f, CChar *path) C {
    f.putMulti(UInt(CC4_TXDS), Byte(4)); // version

    TextStyleDesc desc;
    Unaligned(desc.spacing, spacing);
    Unaligned(desc.shadow, shadow);
    Unaligned(desc.shade, shade);
    Unaligned(desc.color, color);
    Unaligned(desc.selection, selection);
    Unaligned(desc.align, align);
    Unaligned(desc.size, size);
    Unaligned(desc.space, space);
    f << desc;
    f._putAsset(font().name(path));
    return f.ok();
}
Bool TextStyle::load(File &f, CChar *path) {
    pixel_align = true;
    edit = null;
    if (f.getUInt() == CC4_TXDS)
        switch (f.decUIntV()) // version
        {
        case 4: {
            TextStyleDesc desc;
            if (f.getFast(desc)) {
                Unaligned(spacing, desc.spacing);
                Unaligned(shadow, desc.shadow);
                Unaligned(shade, desc.shade);
                Unaligned(color, desc.color);
                Unaligned(selection, desc.selection);
                Unaligned(align, desc.align);
                Unaligned(size, desc.size);
                Unaligned(space, desc.space);
                _font.require(f._getAsset(), path);
                super::font(_font());
                if (f.ok())
                    return true;
            }
        } break;

        case 3: {
#pragma pack(push, 1)
            struct TextStyleDesc {
                SPACING_MODE spacing;
                Byte shadow, shade;
                Color color;
                Vec2 align, size, space;
            };
#pragma pack(pop)
            TextStyleDesc desc;
            if (f.get(desc)) {
                Unaligned(spacing, desc.spacing);
                Unaligned(shadow, desc.shadow);
                Unaligned(shade, desc.shade);
                Unaligned(color, desc.color);
                Unaligned(align, desc.align);
                Unaligned(size, desc.size);
                Unaligned(space, desc.space);
                _font.require(f._getStr(), path);
                super::font(_font());
                selection = DefaultSelectionColor;
                if (f.ok())
                    return true;
            }
        } break;

        case 2: {
#pragma pack(push, 4)
            struct TextStyleDesc {
                SPACING_MODE spacing;
                Byte shadow, shade;
                Color color;
                Vec2 align, size, space;
            };
#pragma pack(pop)
            TextStyleDesc desc;
            if (f.get(desc)) {
                Unaligned(spacing, desc.spacing);
                Unaligned(shadow, desc.shadow);
                Unaligned(shade, desc.shade);
                Unaligned(color, desc.color);
                Unaligned(align, desc.align);
                Unaligned(size, desc.size);
                Unaligned(space, desc.space);
                _font.require(f._getStr(), path);
                super::font(_font());
                selection = DefaultSelectionColor;
                if (f.ok())
                    return true;
            }
        } break;

        case 1: {
#pragma pack(push, 4)
            struct TextStyleDesc {
                SPACING_MODE spacing;
                Byte shadow, shade;
                VecB4 color;
                Vec2 align, size, space;
            };
#pragma pack(pop)
            TextStyleDesc desc;
            if (f.get(desc)) {
                Unaligned(spacing, desc.spacing);
                Unaligned(shadow, desc.shadow);
                Unaligned(shade, desc.shade);
                color.set(Unaligned(desc.color.z), Unaligned(desc.color.y), Unaligned(desc.color.x), Unaligned(desc.color.w));
                Unaligned(align, desc.align);
                Unaligned(size, desc.size);
                Unaligned(space, desc.space);
                _font.require(f._getStr(), path);
                super::font(_font());
                selection = DefaultSelectionColor;
                if (f.ok())
                    return true;
            }
        } break;

        case 0: {
#pragma pack(push, 4)
            struct TextStyleDesc {
                SPACING_MODE spacing;
                Byte shadow, shade;
                VecB4 color;
                Vec2 align, size, space;
            };
#pragma pack(pop)
            TextStyleDesc desc;
            if (f.get(desc)) {
                Unaligned(spacing, desc.spacing);
                Unaligned(shadow, desc.shadow);
                shade = DefaultShade;
                color.set(Unaligned(desc.color.z), Unaligned(desc.color.y), Unaligned(desc.color.x), Unaligned(desc.color.w));
                Unaligned(align, desc.align);
                Unaligned(size, desc.size);
                Unaligned(space, desc.space);
                _font.require(f._getStr8(), path);
                super::font(_font());
                selection = DefaultSelectionColor;
                if (f.ok())
                    return true;
            }
        } break;
        }
    reset();
    return false;
}

Bool TextStyle::save(C Str &name) C {
    File f;
    if (f.write(name)) {
        if (save(f, _GetPath(name)) && f.flush())
            return true;
        f.del();
        FDelFile(name);
    }
    return false;
}
Bool TextStyle::load(C Str &name) {
    File f;
    if (f.read(name))
        return load(f, _GetPath(name));
    reset();
    return false;
}
void TextStyle::operator=(C Str &name) {
    if (!load(name))
        Exit(MLT(S + "Can't load Text Style \"" + name + "\"",
                 PL, S + u"Nie można wczytać stylu tekstu \"" + name + "\""));
}
/******************************************************************************/
// MAIN
/******************************************************************************/
void ShutFont() {
#if SUPPORT_EMOJI
    EmojiPak = null;
    Emojis.del();
#endif
    TextStyles.del();
    Fonts.del();
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
