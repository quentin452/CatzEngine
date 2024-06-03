/******************************************************************************/
#include "../../stdafx.h"

#define USE_FREE_TYPE LINUX

#define SPECIAL_CHARS 3 // Space, FullWidthSpace, Tab

#if USE_FREE_TYPE
#include "../../../ThirdPartyLibs/begin.h"

#include "../../../ThirdPartyLibs/FreeType/lib/include/ft2build.h"
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

#include "../../../ThirdPartyLibs/end.h"
#endif

#define TEST_FONT 0
#if TEST_FONT
#pragma message("!! Warning: Use this only for debugging !!")
#endif

#define FONT_SRGB 0           // 0=best
#define FONT_SRGB_SUB_PIXEL 0 // 0=best

ASSERT(!FONT_SRGB); // right now IMAGE_R8G8/IMAGE_BC5 is used, which don't have sRGB variants
// #FontImageLayout
#define FONT_IMAGE_TYPE (FONT_SRGB ? IMAGE_R8G8 /*_SRGB*/ : IMAGE_R8G8)
#define FONT_IMAGE_TYPE_SUB_PIXEL (FONT_SRGB_SUB_PIXEL ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8A8)

#define TOTAL_CREATE_FONTS 2

namespace EE {
/******************************************************************************/
#define CC4_FONT CC4('F', 'O', 'N', 'T')
/******************************************************************************/
DEFINE_CACHE_EX(Font, Fonts, FontPtr, "Font", 2); // use small number of elements per block because Font class size is big (above 128 KB)
/******************************************************************************/
// FONT MANAGE
/******************************************************************************/
void Font::zero() {
    _sub_pixel = false;
    _height = 0;
    _padd.zero();
    Zero(_char_to_font);
    Zero(_wide_to_font);
}
Font::Font() { zero(); }
Font &Font::del() {
    _chrs.del();
    _images.del();
    zero();
    return T;
}
/******************************************************************************/
UInt Font::memUsage() C {
    UInt size = 0;
    REPA(_images)
    size += _images[i].memUsage();
    return size;
}
Bool Font::hasChar(Char8 c) C {
    UInt index = _char_to_font[Unsigned(c)];
    return InRange(index, _chrs) && _chrs[index].chr == Char8To16(c);
} // check character in case it's remapped to '?'
Bool Font::hasChar(Char c) C {
    UInt index = _wide_to_font[Unsigned(c)];
    return InRange(index, _chrs) && _chrs[index].chr == c;
} // check character in case it's remapped to '?'
Int Font::charIndex(Char8 c) C {
    UInt index = _char_to_font[Unsigned(c)];
    return InRange(index, _chrs) ? index : -1;
}
Int Font::charIndex(Char c) C {
    UInt index = _wide_to_font[Unsigned(c)];
    return InRange(index, _chrs) ? index : -1;
}
Int Font::charWidth(Char c) C {
    UInt index = _wide_to_font[Unsigned(c)];
    return InRange(index, _chrs) ? _chrs[index].width : (SUPPORT_EMOJI && c) ? height()
                                                                             : 0;
}
Int Font::charWidth(Char8 c) C {
    UInt index = _char_to_font[Unsigned(c)];
    return InRange(index, _chrs) ? _chrs[index].width : (SUPPORT_EMOJI && c) ? height()
                                                                             : 0;
}
/******************************************************************************/
Int Font::charWidth(Char8 c0, Char8 c1, SPACING_MODE spacing) C {
    switch (spacing) {
    case SPACING_FAST:
        return charWidth(c0);
    case SPACING_NICE: {
        UShort index = _char_to_font[Unsigned(c0)];
        if (InRange(index, _chrs)) {
            C Chr &chr = _chrs[index];
            Int width = chr.width; // get width of the first character
            index = _char_to_font[Unsigned(c1)];
            if (InRange(index, _chrs)) // if the second character exists, then adjust spacing between them
            {
                C Byte *width_0 = chr.widths[1],       // we're taking the right side of the first  character
                    *width_1 = _chrs[index].widths[0]; // we're taking the left  side of the second character
                Int min = 255 * 2;                     // 255 (means no distance) * 2 (because of adding l+r)
                REP(FONT_WIDTH_TEST) {
                    Byte l = width_0[i],
                         r = width_1[i];
                    if (l != 255 && r != 255)
                        MIN(min, l + r);
                }
                if (min < 255 * 2) {
                    if (CharFlagFast(c0, c1) & CHARF_FONT_SPACE)
                        min /= 2; // if any of the 2 characters needs adjusting
                    width -= min;
                }
            }
            return width;
        }
        if (SUPPORT_EMOJI && c0)
            return height();
    } // here on purpose no break, to fall down and return 0
    default:
        return 0;
    }
}
Int Font::charWidth(Char c0, Char c1, SPACING_MODE spacing) C {
    switch (spacing) {
    case SPACING_FAST:
        return charWidth(c0);
    case SPACING_NICE: {
        UShort index = _wide_to_font[Unsigned(c0)];
        if (InRange(index, _chrs)) {
            C Chr &chr = _chrs[index];
            Int width = chr.width; // get width of the first character
            index = _wide_to_font[Unsigned(c1)];
            if (InRange(index, _chrs)) // if the second character exists, then adjust spacing between them
            {
                C Byte *width_0 = chr.widths[1],       // we're taking the right side of the first  character
                    *width_1 = _chrs[index].widths[0]; // we're taking the left  side of the second character
                Int min = 255 * 2;                     // 255 (means no distance) * 2 (because of adding l+r)
                REP(FONT_WIDTH_TEST) {
                    Byte l = width_0[i],
                         r = width_1[i];
                    if (l != 255 && r != 255)
                        MIN(min, l + r);
                }
                if (min < 255 * 2) {
                    if (CharFlagFast(c0, c1) & CHARF_FONT_SPACE)
                        min /= 2; // if any of the 2 characters needs adjusting
                    width -= min;
                }
            }
            return width;
        }
        if (SUPPORT_EMOJI && c0)
            return height();
    } // here on purpose no break, to fall down and return 0
    default:
        return 0;
    }
}
/******************************************************************************/
Int Font::textWidth(Int &spacings, SPACING_MODE spacing, CChar8 *text, Int max_length) C {
    Int width = 0, spcs;
    if (spacing != SPACING_CONST) {
        spcs = -1;          // for !SPACING_CONST we calculate spacings between characters, so start -1 and Max(0 later
        if (max_length < 0) // unlimited
        {
            Char8 c = *text;
        next:
            if (c) {
                spcs++;
            combining:
                Char8 next = *++text;
                if (CharFlagFast(next) & CHARF_SKIP)
                    goto combining;
                width += charWidth(c, next, spacing);
                c = next;
                goto next;
            }
        } else if (max_length) {
            Char8 c = *text;
        next1:
            if (c) {
                spcs++;
            combining1:
                if (!--max_length) { /*if(spacing!=SPACING_CONST)*/
                    width += charWidth(c);
                } else // for the last character we need to process only its width and ignore the spacing between the next one
                {
                    Char8 next = *++text;
                    if (CharFlagFast(next) & CHARF_SKIP)
                        goto combining1;
                    width += charWidth(c, next, spacing);
                    c = next;
                    goto next1;
                }
            }
        }
        MAX(spcs, 0);
    } else // for SPACING_CONST we don't calculate 'width' but just 'spacings'
    {
        spcs = 0;
        if (max_length < 0) // unlimited
        {
            Char8 c = *text;
        next2:
            if (c) {
                spcs++;
            combining2:
                Char8 next = *++text;
                if (CharFlagFast(next) & CHARF_SKIP)
                    goto combining2;
                c = next;
                goto next2;
            }
        } else if (max_length) {
            Char8 c = *text;
        next3:
            if (c) {
                spcs++;
            combining3:
                if (--max_length) {
                    Char8 next = *++text;
                    if (CharFlagFast(next) & CHARF_SKIP)
                        goto combining3;
                    c = next;
                    goto next3;
                }
            }
        }
    }
    spacings = spcs;
    return width;
}
Int Font::textWidth(Int &spacings, SPACING_MODE spacing, CChar *text, Int max_length) C {
    Int width = 0, spcs;
    if (spacing != SPACING_CONST) {
        spcs = -1;          // for !SPACING_CONST we calculate spacings between characters, so start -1 and Max(0 later
        if (max_length < 0) // unlimited
        {
            Char c = *text;
        next:
            if (c) {
                spcs++;
            combining:
                Char next = *++text;
                if (CharFlagFast(next) & CHARF_SKIP)
                    goto combining;
                width += charWidth(c, next, spacing);
                c = next;
                goto next;
            }
        } else if (max_length) {
            Char c = *text;
        next1:
            if (c) {
                spcs++;
            combining1:
                if (!--max_length) { /*if(spacing!=SPACING_CONST)*/
                    width += charWidth(c);
                } else // for the last character we need to process only its width and ignore the spacing between the next one
                {
                    Char next = *++text;
                    if (CharFlagFast(next) & CHARF_SKIP)
                        goto combining1;
                    width += charWidth(c, next, spacing);
                    c = next;
                    goto next1;
                }
            }
        }
        MAX(spcs, 0);
    } else // for SPACING_CONST we don't calculate 'width' but just 'spacings'
    {
        spcs = 0;
        if (max_length < 0) // unlimited
        {
            Char c = *text;
        next2:
            if (c) {
                spcs++;
            combining2:
                Char next = *++text;
                if (CharFlagFast(next) & CHARF_SKIP)
                    goto combining2;
                c = next;
                goto next2;
            }
        } else if (max_length) {
            Char c = *text;
        next3:
            if (c) {
                spcs++;
            combining3:
                if (--max_length) {
                    Char next = *++text;
                    if (CharFlagFast(next) & CHARF_SKIP)
                        goto combining3;
                    c = next;
                    goto next3;
                }
            }
        }
    }
    spacings = spcs;
    return width;
}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
Font &Font::freeOpenGLESData() {
    REPAO(_images).freeOpenGLESData();
    return T;
}
/******************************************************************************/
Bool Font::imageType(IMAGE_TYPE type) {
    if (!_sub_pixel)
        type = (FONT_SRGB ? ImageTypeIncludeSRGB : ImageTypeExcludeSRGB)(type);
    else {
        type = (FONT_SRGB_SUB_PIXEL ? ImageTypeIncludeSRGB : ImageTypeExcludeSRGB)(type);
#if FONT_SRGB_SUB_PIXEL
        if (type != IMAGE_R8G8B8A8_SRGB && type != IMAGE_B8G8R8A8_SRGB && type != IMAGE_BC2_SRGB && type != IMAGE_BC3_SRGB && type != IMAGE_BC7_SRGB && type != IMAGE_ETC2_RGBA_SRGB && type != IMAGE_ASTC_4x4_SRGB)
            return false; // sub pixel supports only these formats
#else
        if (type != IMAGE_R8G8B8A8 && type != IMAGE_B8G8R8A8 && type != IMAGE_BC2 && type != IMAGE_BC3 && type != IMAGE_BC7 && type != IMAGE_ETC2_RGBA && type != IMAGE_ASTC_4x4)
            return false; // sub pixel supports only these formats
#endif
    }
    Bool changed = false;
    REPA(_images) {
        Image &image = _images[i];

        Int w = image.w(),
            h = image.h();
        Int mip_maps = image.mipMaps();
        if (type == IMAGE_PVRTC1_2 || type == IMAGE_PVRTC1_4 || type == IMAGE_PVRTC1_2_SRGB || type == IMAGE_PVRTC1_4_SRGB) {
            w = h = CeilPow2(Max(w, h)); // PVRTC1 requires square power of 2 anyway, so resize image to max size to gain some quality
            mip_maps = 1;                // PVRTC has too low quality to enable mip-maps
        }
        if (image.type() != type || image.w() != w || image.h() != h || image.mipMaps() != mip_maps) // if change is needed
            if (image.copy(image, w, h, -1, type, -1, mip_maps))
                changed = true;
    }
    return changed;
}
void Font::toSoft() {
    REPA(_images) {
        Image &image = _images[i];
        image.copy(image, -1, -1, -1, ImageTypeUncompressed(image.type()), IMAGE_SOFT);
    }
}
Font &Font::replace(Char src, Char dest, Bool permanent) {
    if (src && src != dest) {
        Int src_i = 0xFFFF, // use 0xFFFF because it's set in 'setRemap' and it means out of range
            dest_i = 0xFFFF;
        REPA(_chrs) {
            Char c = _chrs[i].chr;
            if (c == src)
                src_i = i;
            else if (c == dest)
                dest_i = i;
        }
        if (permanent) {
            if (!dest || dest_i == 0xFFFF) // want to remove or dest not found
            {
                if (src_i != 0xFFFF) {
                    _chrs.remove(src_i, true);
                    setRemap();
                } // source exists
            } else // have a valid target
            {
                if (src_i == 0xFFFF)
                    src_i = _chrs.addNum(1); // source not found
                _chrs[src_i] = _chrs[dest_i];
                _chrs[src_i].chr = src;
                setRemap();
            }
        } else {
            Char8 c = Char16To8(src);
            if (Char8To16(c) != src)
                c = 0; // take 8-bit char, set zero if there's no 1:1 mapping
            if (c)
                _char_to_font[Unsigned(c)] = dest_i;
            if (src)
                _wide_to_font[Unsigned(src)] = dest_i;
        }
    }
    return T;
}
/******************************************************************************/
void Font::setRemap() {
    SetMem(_char_to_font, 0xFF, SIZE(_char_to_font));
    SetMem(_wide_to_font, 0xFF, SIZE(_wide_to_font));
    Int invalid[6] = {-1, -1, -1, -1, -1, -1}, space = -1;
    REPA(_chrs)
    if (Char w = _chrs[i].chr) {
        Char8 c = Char16To8(w);
        if (Char8To16(c) != w)
            c = 0; // take 8-bit char, set zero if there's no 1:1 mapping
        if (c)
            _char_to_font[Unsigned(c)] = i;
        if (w)
            _wide_to_font[Unsigned(w)] = i;
        switch (w) {
        case u'⍰':
            invalid[0] = i;
            break;
        case u'�':
            invalid[1] = i;
            break;
        case '?':
            invalid[2] = i;
            break;
        case '*':
            invalid[3] = i;
            break;
        case '#':
            invalid[4] = i;
            break;
        case '.':
            invalid[5] = i;
            break;
        case ' ':
            space = i;
            break;
        }
    }

    // set all characters which were not found, to be displayed as 'invalid'
    FREPA(invalid) {
        Int inv = invalid[i];
        if (inv >= 0) {
            Bool allow[ELMS(_wide_to_font)];
            const Bool test_allow = (SUPPORT_EMOJI && EmojiPak);
            if (test_allow) {
                SetMem(allow, true); // allow all by default
                REPA(*EmojiPak) {
                    C PakFile &pf = EmojiPak->file(i);
                    Char c = *pf.name; // get first character of emoji
                    allow[Unsigned(c)] = false;
                    ASSERT(Elms(allow) == USHORT_MAX + 1); // if we have emoji starting with that character, then disable replacing character to 'invalid' and just use that emoji
                }
            }

            // never replace '\0' (start from 1) and CHARF_MULTI0 (because that one needs to be processed in special way) (however when SUPPORT_EMOJI is disabled, then replace CHARF_MULTI0 as well, because without SUPPORT_EMOJI their width would be 0)
            UShort invalid = inv;
            Int multi0 = Min(SUPPORT_EMOJI ? 0xD800 : 0xDC00, Elms(_wide_to_font));
            for (Int i = 1; i < Elms(_char_to_font); i++)
                if (_char_to_font[i] == 0xFFFF)
                    _char_to_font[i] = invalid;
            for (Int i = 1; i < multi0; i++)
                if (_wide_to_font[i] == 0xFFFF && (!test_allow || allow[i]))
                    _wide_to_font[i] = invalid;
            for (Int i = 0xDC00; i < Elms(_wide_to_font); i++)
                if (_wide_to_font[i] == 0xFFFF && (!test_allow || allow[i]))
                    _wide_to_font[i] = invalid;

            break; // stop on first found
        }
    }
    if (space >= 0)
        _wide_to_font[Unsigned(Nbsp)] = space; // draw NBSP as space
}
/******************************************************************************/
// FONT IO
/******************************************************************************/
#pragma pack(push, 4)
struct FontChr3 {
    Char chr;
    Byte image, offset, width, height, widths[FONT_WIDTH_TEST][2];
    Rect tex;
};
struct FontChr0 {
    Char chr;
    Byte image, width, widths[FONT_WIDTH_TEST][2];
    Rect tex;
};
#pragma pack(pop)
static void FontLoadChr3(Font &font, File &f) {
    Mems<FontChr3> chrs;
    chrs.loadRaw(f);
    font._chrs.setNum(chrs.elms());
    REPA(font._chrs) {
        Font::Chr &dest = font._chrs[i];
        FontChr3 &src = chrs[i];
        dest.chr = src.chr;
        dest.image = src.image;
        dest.offset = src.offset;
        dest.width = src.width;
        dest.width_padd = Min(dest.width + font.paddingL() + font.paddingR(), 255);
        dest.height = src.height;
        dest.height_padd = Min(dest.height + font.paddingT() + font.paddingB(), 255);
        dest.tex = src.tex;
        REPD(s, 2)
        REPD(i, FONT_WIDTH_TEST)
        dest.widths[s][i] = src.widths[i][s];
    }
}
static void FontLoadChr0(Font &font, File &f) {
    Mems<FontChr0> chrs;
    chrs._loadRaw(f);
    font._chrs.setNum(chrs.elms());
    REPA(font._chrs) {
        Font::Chr &dest = font._chrs[i];
        FontChr0 &src = chrs[i];
        dest.chr = src.chr;
        dest.image = src.image;
        dest.tex = src.tex;
        dest.width = src.width;
        dest.width_padd = Min(dest.width + font.paddingL() + font.paddingR(), 255);
        dest.height = font.height();
        dest.height_padd = Min(dest.height + font.paddingT() + font.paddingB(), 255);
        dest.offset = 0;
        REPD(s, 2)
        REPD(i, FONT_WIDTH_TEST)
        dest.widths[s][i] = src.widths[i][s];
    }
}
static Bool Adjust(Font &font, Int layout) // #FontImageLayout
{
    REPA(font._images) {
        Image &image = font._images[i];
        if (font._sub_pixel) {
            // sub pixel have RGBA layout, just make sure we have correct sRGB
            if (!image.copy(image, -1, -1, -1, (FONT_SRGB_SUB_PIXEL ? ImageTypeIncludeSRGB : ImageTypeExcludeSRGB)(image.type())))
                return false;
        } else {
            Image dest;
            if (!dest.create(image.w(), image.h(), 1, FONT_IMAGE_TYPE, image.mode(), image.mipMaps()))
                return false;
            Image temp;
            C Image *src = &image;
            if (image.compressed())
                if (image.copy(temp, -1, -1, -1, ImageTypeUncompressed(image.type()), IMAGE_SOFT, 1))
                    src = &temp;
                else
                    return false;
            if (!src->lockRead())
                return false;
            if (!dest.lock(LOCK_WRITE)) {
                src->unlock();
                return false;
            }
            REPD(y, dest.h())
            REPD(x, dest.w()) {
                Color c = src->color(x, y);
                switch (layout) {
                case 0:
                    c.b = c.a = 0;
                    break; // source is RG
                case 1:
                    c.r = c.g;
                    c.g = c.a;
                    c.b = c.a = 0;
                    break; // source is GA
                }
                dest.color(x, y, c);
            }
            dest.unlock();
            dest.updateMipMaps(FILTER_LINEAR); // for speed use linear filter
            src->unlock();
            Swap(dest, image);
        }
    }
    return true;
}
Bool Font::save(File &f) C {
    f.putMulti(UInt(CC4_FONT), Byte(6), _sub_pixel, _height, _padd); // version
    f.cmpUIntV(_images.elms());
    FREPA(_images)
    if (!_images[i].saveData(f))
        return false;
    _chrs.saveRaw(f);
    return f.ok();
}
Bool Font::load(File &f) {
    del();
    if (f.getUInt() == CC4_FONT)
        switch (f.decUIntV()) // version
        {
        case 6: {
            f.getMulti(_sub_pixel, _height, _padd);
            _images.setNum(f.decUIntV());
            FREPA(_images)
            if (!_images[i].loadData(f))
                goto error;
            _chrs.loadRaw(f);
            if (f.ok()) {
                setRemap();
                return true;
            }
        } break;

        case 5: {
            f.getMulti(_sub_pixel, _height, _padd);
            _images.setNum(f.decUIntV());
            FREPA(_images)
            if (!_images[i]._loadData(f))
                goto error;
            _chrs.loadRaw(f);
            if (f.ok() && Adjust(T, 1)) {
                setRemap();
                return true;
            }
        } break;

        case 4: {
            f >> _sub_pixel >> _height >> _padd;
            _images.setNum(f.decUIntV());
            FontLoadChr3(T, f);
            FREPA(_images) {
                f.skip(4);
                if (!_images[i]._loadData(f))
                    goto error;
            } // skip 'GFX' CC4 and use '_loadData'
            if (f.ok() && Adjust(T, 1)) {
                setRemap();
                return true;
            }
        } break;

        case 3: {
            f >> _sub_pixel >> _height >> _padd.x >> _padd.z >> _padd.y >> _padd.w;
            _images.setNum(f.decUIntV());
            FontLoadChr3(T, f);
            FREPA(_images) {
                f.skip(4);
                if (!_images[i]._loadData(f))
                    goto error;
            } // skip 'GFX' CC4 and use '_loadData'
            if (f.ok() && Adjust(T, 0)) {
                setRemap();
                return true;
            }
        } break;

        case 2: {
            f >> _sub_pixel >> _height >> _padd.x >> _padd.z >> _padd.y >> _padd.w;
            _images.setNum(f.getInt());
            FontLoadChr0(T, f);
            FREPA(_images) {
                f.skip(4);
                if (!_images[i]._loadData(f))
                    goto error;
            } // skip 'GFX' CC4 and use '_loadData'
            if (f.ok() && Adjust(T, 0)) {
                setRemap();
                return true;
            }
        } break;

        case 1: {
            f >> _height >> _padd.x >> _padd.z >> _padd.y >> _padd.w;
            _images.setNum(f.getInt());
            FontLoadChr0(T, f);
            FREPA(_images) {
                f.skip(4);
                if (!_images[i]._loadData(f))
                    goto error;
            } // skip 'GFX' CC4 and use '_loadData'
            if (f.ok() && Adjust(T, 0)) {
                setRemap();
                return true;
            }
        } break;

        case 0: {
            f >> _height >> _padd.x >> _padd.z;
            _padd.y = _padd.x;
            _padd.w = _padd.z;
            _images.setNum(f.getInt());
            FontLoadChr0(T, f);
            FREPA(_images) {
                f.skip(4);
                if (!_images[i]._loadData(f))
                    goto error;
            } // skip 'GFX' CC4 and use '_loadData'
            if (f.ok() && Adjust(T, 0)) {
                setRemap();
                return true;
            }
        } break;
        }
error:
    del();
    return false;
}
Bool Font::save(C Str &name) C {
    File f;
    if (f.write(name)) {
        if (save(f) && f.flush())
            return true;
        f.del();
        FDelFile(name);
    }
    return false;
}
Bool Font::load(C Str &name) {
    File f;
    if (f.read(name))
        return load(f);
    del();
    return false;
}
void Font::operator=(C UID &id) { T = _EncodeFileName(id); }
void Font::operator=(C Str &name) {
    if (!load(name))
        Exit(MLT(S + "Can't load font \"" + name + "\"",
                 PL, S + u"Nie można wczytać czcionki \"" + name + "\""));
}
/******************************************************************************/
// FONT CREATE
/******************************************************************************/
struct SystemFont {
    Int size = 0,
        base_line = -1, // unknown
        base_line_offset = 0;
    Flt scale = 1;
#if WINDOWS_OLD
    HFONT font = null;
#endif
#if USE_FREE_TYPE
#if WINDOWS_OLD
    Mems<Byte> font_data;
#elif LINUX
    Str system_font;
#endif
#elif MAC
    NSFont *font = null;
    NSMutableParagraphStyle *style = null;
    NSMutableDictionary *attributes = null;
#endif
    Mems<VecUS2> supported_chars; // x=min, y=max, inclusive to allow supporting all 65536 characters in a single item x=0 y=65535, (if not inclusive then x=0 y=65536 would be out of U16 range)

    ~SystemFont() { del(); }

    void del() {
#if WINDOWS_OLD
        if (font) {
            DeleteObject(font);
            font = null;
        }
#endif
#if USE_FREE_TYPE
#if WINDOWS_OLD
        font_data.del();
#endif
#elif MAC
        [attributes release];
        attributes = null;
        [style release];
        style = null;
        /*[font       release];*/ font = null; // font is not manually allocated
#endif
    }
    Bool supportedChar(Char c) C {
        if (!supported_chars.elms())
            return true; // if unknown supported characters then assume all are supported
        REPA(supported_chars) {
            C auto &range = supported_chars[i];
            if (Unsigned(c) >= range.x && Unsigned(c) <= range.y)
                return true;
        }
        return false;
    }
    void includeChar(Char c) {
        supported_chars.add(c);
    }
    void setBaseLine(Int base_line) {
        T.base_line = base_line;
        base_line_offset = ((base_line >= 0) ? Round(
                                                   0.8f * size - base_line         // base line offset - 0.8 is the default baseline (was 80 when measured on Arial 100)
                                                   + base_line / scale - base_line // difference between unscaled baseline (base_line/scale) and scaled baseline
                                                   )
                                             : 0);
    }
    Bool create(C Str &system_font, Int size, Font::MODE mode, Flt weight, Flt scale) // here 'size' is already scaled
    {
        del();
#if WINDOWS_OLD
        if (font = CreateFont(size, 0, 0, 0, RoundU(Sat(weight) * 1000), 0, 0, 0, ANSI_CHARSET, 0, 0, (mode == Font::DEFAULT) ? ANTIALIASED_QUALITY : CLEARTYPE_QUALITY, 0, system_font))
            if (HDC dc = CreateCompatibleDC(null)) {
                T.size = size;
                T.scale = scale;
                SelectObject(dc, font);

                // base line
                TEXTMETRIC metric;
                if (GetTextMetrics(dc, &metric))
                    setBaseLine(metric.tmAscent);

                // supported chars
                // alternative: WCHAR c=CharReturn; WORD id; Bool exists=(GetGlyphIndicesW(dc, &c, 1, &id, GGI_MARK_NONEXISTING_GLYPHS)!=GDI_ERROR && id!=0xFFFF);
                auto glyphs_size = GetFontUnicodeRanges(dc, null);
                if (glyphs_size >= SIZE(GLYPHSET)) {
                    Memt<Byte> temp;
                    temp.setNum(glyphs_size);
                    GLYPHSET *glyphs = (GLYPHSET *)temp.data();
                    if (temp.elms() == GetFontUnicodeRanges(dc, glyphs)) {
                        supported_chars.setNum(glyphs->cRanges);
                        REPA(supported_chars) {
                            C auto &src = glyphs->ranges[i];
                            if (src.cGlyphs > 0) {
                                auto &dest = supported_chars[i];
                                dest.x = src.wcLow;
                                dest.y = src.wcLow + src.cGlyphs - 1; // -1 because y=inclusive
                            } else
                                supported_chars.remove(i, true);
                        }
                        if (system_font == "Segoe UI" || system_font == "Tahoma")
                            includeChar(CharReturn); // Even though "Segoe UI" and "Tahoma" by default report no support for CharReturn, in tests they do (at least on Win10), so force it, because it looks good
                    }
                }

#if USE_FREE_TYPE
                auto font_data_size = GetFontData(dc, 0, 0, null, 0);
                if (font_data_size > 0) {
                    font_data.setNum(font_data_size);
                    if (GetFontData(dc, 0, 0, font_data.data(), font_data.elms()) != font_data.elms())
                        font_data.del();
                }
#endif

                DeleteDC(dc);

#if USE_FREE_TYPE
                if (!font_data.elms())
                    goto error;
#endif

                return true;
            }
#elif USE_FREE_TYPE
        T.size = size;
        T.scale = scale;
#if LINUX
        T.system_font = system_font;
        if (GetBase(T.system_font) == T.system_font) // not path
        {
            ConsoleProcess cp;
            if (cp.create("fc-match", S + "-v \"" + T.system_font + '"')) // use FontConfig fc-match tool to get the font file name, "-v" needed to get full "file" path
            {
                cp.wait(1000);
                Str path = StrInside(cp.get(), "file: \"", "\"");
                if (path.is())
                    T.system_font = path;
            }
        }
#endif
        return true;
#elif MAC
        Flt font_size = size * (64.0f / 72); // to match Windows size
        if (NSStringAuto family = system_font) {
            font = [[NSFontManager sharedFontManager] fontWithFamily:family traits:0 weight:RoundU(Sat(weight) * 15) size:font_size];
            if (!font)
                font = [NSFont fontWithName:family size:font_size]; // does not accept weight
        }
        if (!font)
            font = [[NSFontManager sharedFontManager] fontWithFamily:@"Arial" traits:0 weight:RoundU(Sat(weight) * 15) size:font_size];

        if (font)
            if (style = [[NSMutableParagraphStyle alloc] init])
                if (attributes = [[NSMutableDictionary alloc] init]) {
                    [style setAlignment:NSCenterTextAlignment];
                    [attributes setObject:font forKey:NSFontAttributeName];
                    [attributes setObject:style forKey:NSParagraphStyleAttributeName];
                    [attributes setObject:[NSColor whiteColor] forKey:NSForegroundColorAttributeName];
                    T.size = size;
                    T.scale = scale;
                    setBaseLine(Round(font.ascender * size / (font.ascender - font.descender))); // 'descender' is negative
                    return true;
                }
#endif

#if WINDOWS_OLD && USE_FREE_TYPE
    error:
#endif
        del();
        return false;
    }
};
/******************************************************************************/
struct FontChar {
    Char chr = '\0';
    VecI2 ofs = 0, size = 0; // these are unaffected by shadow padding
    Byte widths[2][FONT_WIDTH_TEST];
    Image image;

    void setSpace(Int size) {
        T.chr = ' ';
        T.ofs = 0;
        T.size.set(DivRound(size, 4), 0);
        Zero(widths);
        image.del();
    }
    void setFullSpace(Int size) {
        T.chr = FullWidthSpace;
        T.ofs = 0;
        T.size.set(DivRound(size, 2), 0);
        Zero(widths);
        image.del();
    }
    void setTab(Int size) {
        T.chr = '\t';
        T.ofs = 0;
        T.size.set(DivRound(size, 4) * 3, 0); // make tab 3x wider than space
        Zero(widths);
        image.del();
    }

    static Int Compare(C FontChar &a, C FontChar &b) {
        Bool unicode = HasUnicode(a.chr);
        if (Int c = ::Compare(unicode, HasUnicode(b.chr)))
            return c; // keep ASCII (non-unicode) first, because they're most commonly used, to minimize image changes when drawing
        if (Int c = ::Compare(a.image.h(), b.image.h()))
            return unicode ? c : -c;                        // sort by height to pack as many characters as possible, reverse order for non-unicode, so when transitioning from non-unicode to unicode, heights will be similar (this makes order: big ASCII, small ASCII, small Unicode, big Unicode). Order was designed to keep small Unicode close to 1st image, as those are usually frequantly used symbols.
        return ::Compare(Unsigned(a.chr), Unsigned(b.chr)); // keep characters close together to minimize image changes when drawing
    }

    FontChar() { Zero(widths); } // Zero (for example required for spaces)
};
/******************************************************************************/
struct FontCreateBase : Font::Params {
    MemtN<SystemFont, TOTAL_CREATE_FONTS> fonts;

    FontCreateBase(C Params &src) : Params(src) {}
};
#if USE_FREE_TYPE
struct FreeTypeDrawContext {
    FT_Library library = null; // 'library' is not multi-thread safe so keep it in separate contexts
    FT_Face face = null;

    ~FreeTypeDrawContext() {
        if (face) {
            FT_Done_Face(face);
            face = null;
        }
        if (library) {
            FT_Done_FreeType(library);
            library = null;
        }
    }
    Bool create(SystemFont &font) {
        if (!FT_Init_FreeType(&library)) {
            FT_Library_SetLcdFilter(library, FT_LCD_FILTER_DEFAULT);
#if WINDOWS_OLD
            if (!FT_New_Memory_Face(library, font.font_data.data(), font.font_data.elms(), 0, &face))
#else
            if (!FT_New_Face(library, UnixPathUTF8(font.system_font), 0, &face))
#endif
#if 0 // in this version scale must be applied to match Windows Size, but prefer other version to have more precision
         if(!FT_Set_Pixel_Sizes(face, 0, font.size*64/72))
#else // this gives same result as Windows
                if (!FT_Set_Char_Size(face, 0, font.size * 64, 0, 64))
#endif
                {
                    // ok
                    font.setBaseLine(DivRound(face->ascender * font.size, face->height));
                    return true;
                }
        }
        return false;
    }
};
#endif
struct SystemFontDrawContext {
    FontCreateBase *base = null;
    Image image;
#if USE_FREE_TYPE
    MemtN<FreeTypeDrawContext, TOTAL_CREATE_FONTS> fonts;
#elif WINDOWS_OLD
    HDC dc = null;
    HBITMAP bitmap = null;
    VecB4 *data = null;
#elif MAC
    Image bitmap_image;
    NSBitmapImageRep *bitmap = null;
    NSGraphicsContext *context = null;
#endif

    ~SystemFontDrawContext() {
#if USE_FREE_TYPE
#elif WINDOWS_OLD
        if (bitmap) {
            DeleteObject(bitmap);
            bitmap = null;
        }
        if (dc) {
            DeleteDC(dc);
            dc = null;
        }
#elif MAC
        if (context) {
            SyncLocker locker(D._lock);
            [NSGraphicsContext restoreGraphicsState];
            //[context release]; this causes crashes on 10.9
            context = null;
        }
        [bitmap release];
        bitmap = null;
#endif
    }
    Bool create(Int w, Int h, FontCreateBase &base) {
        if (image.createSoft(w, h, 1, IMAGE_R8G8B8A8)) {
            T.base = &base;
#if USE_FREE_TYPE
            fonts.setNum(base.fonts.elms());
            FREPA(fonts)
            if (!fonts[i].create(base.fonts[i]))
                return false;
            return true;
#elif WINDOWS_OLD
            if (dc = CreateCompatibleDC(null)) {
                BITMAPV5HEADER bi;
                Zero(bi);
                bi.bV5Size = SIZE(bi);
                bi.bV5Width = w;
                bi.bV5Height = h;
                bi.bV5Planes = 1;
                bi.bV5BitCount = 32;
                bi.bV5Compression = BI_BITFIELDS;
                bi.bV5RedMask = 0x000000FF;
                bi.bV5GreenMask = 0x0000FF00;
                bi.bV5BlueMask = 0x00FF0000;
                bi.bV5AlphaMask = 0xFF000000;

                if (bitmap = CreateDIBSection(null, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (Ptr *)&data, null, 0)) {
                    SelectObject(dc, bitmap);
                    SetTextColor(dc, 0xFFFFFF); // RGB(255, 255, 255)    which was manually undefined in the headers
                    SetBkMode(dc, 1);           // 1 is macro for 'TRANSPARENT' which was manually undefined in the headers
                    SetTextAlign(dc, TA_CENTER | TA_TOP | TA_NOUPDATECP);

                    return true;
                }
            }
#elif MAC
            if (bitmap_image.createSoft(w, h, 1, IMAGE_R8G8B8A8)) {
                unsigned char *image_data = bitmap_image.data();
                bitmap = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&image_data
                                                                 pixelsWide:bitmap_image.w()
                                                                 pixelsHigh:bitmap_image.h()
                                                              bitsPerSample:8
                                                            samplesPerPixel:4
                                                                   hasAlpha:YES
                                                                   isPlanar:NO
                                                             colorSpaceName:NSCalibratedRGBColorSpace
                                                                bytesPerRow:bitmap_image.pitch()
                                                               bitsPerPixel:bitmap_image.hwTypeInfo().bit_pp];
                {
                    SyncLocker locker(D._lock);
                    if (context = [NSGraphicsContext graphicsContextWithBitmapImageRep:bitmap]) {
                        [NSGraphicsContext saveGraphicsState];
                        [NSGraphicsContext setCurrentContext:context];
                        return true;
                    }
                }
            }
#endif
        }
        return false;
    }
    C SystemFont *draw(Char chr, Int border) {
        if (chr == '\n')
            chr = CharReturn;
        C SystemFont *font = &base->fonts.first();
        if (!font->supportedChar(chr))
            for (Int i = 1; i < base->fonts.elms(); i++) {
                C SystemFont &test = base->fonts[i];
                if (test.supportedChar(chr)) {
                    font = &test;
                    break;
                }
            }
#if USE_FREE_TYPE
        image.clear();
        C auto face = T.fonts[base->fonts.index(font)].face;
        if (FT_UInt glyph_index = FT_Get_Char_Index(face, chr)) {
            if (!FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT))
                if (!FT_Render_Glyph(face->glyph, (base->mode == Font::SUB_PIXEL) ? FT_RENDER_MODE_LCD : FT_RENDER_MODE_NORMAL)) {
                    FT_Bitmap &bitmap = face->glyph->bitmap;
                    Int left = face->glyph->bitmap_left,
                        top = face->glyph->bitmap_top,
                        offset = border + font->base_line - top;
                    Int width = ((bitmap.pixel_mode == FT_PIXEL_MODE_LCD) ? bitmap.width / 3 : bitmap.width);
                    REPD(y, bitmap.rows)
                    REPD(x, width) {
                        Color col;
                        Byte *row = bitmap.buffer + y * bitmap.pitch;
                        switch (bitmap.pixel_mode) {
                        case FT_PIXEL_MODE_GRAY: {
                            Byte *data = row + x;
                            col.set(*data, *data, *data, 0xFF);
                        } break;

                        case FT_PIXEL_MODE_LCD: {
                            Byte *data = row + x * 3;
                            col.set(data[0], data[1], data[2], 0xFF);
                        } break;

                        default:
                            col = (((x ^ y) & 1) ? WHITE : BLACK);
                            break;
                        }
                        image.color(border + x, offset + y, col);
                    }
                }
        }
#elif WINDOWS_OLD
        // draw the character
        ZeroN(data, image.w() * image.h()); // RECT rect; rect.left=0; rect.top=0; rect.right=size_x; rect.bottom=size_y; FillRect(dc, &rect, HBRUSH(GetStockObject(BLACK_BRUSH)));
        SelectObject(dc, font->font);
        if (chr == u'ำ') // this character needs to be split in 2, because {Nbsp, u'ำ'} still draw a circle
        {
            wchar_t wc[] = {Nbsp, u'ํ', u'า'};
            TextOut(dc, image.w() / 2, border, wc, Elms(wc));
        } else if (CharFlagFast(chr) & CHARF_COMBINING) // Thai combining characters will draw with a circle ัิีึืฺุู็่้๊๋์ํ๎ but if we prepend them with NBSP they will draw without it
        {
            wchar_t wc[] = {Nbsp, chr};
            TextOut(dc, image.w() / 2, border, wc, Elms(wc));
        } else
            TextOut(dc, image.w() / 2, border, WChar(&chr), 1);

        // copy BITMAP to Image
        REPD(y, image.h())
        CopyFast(image.data() + image.pitch() * y, (Byte *)data + image.pitch() * (image.h() - 1 - y), image.pitch()); // need to flip vertically
#elif MAC
        bitmap_image.clear();
        if (NSStringAuto str = chr) {
            NSPoint p;
            p.x = border;
            p.y = border;
            [str() drawAtPoint:p withAttributes:font->attributes];
            [context flushGraphics];
        }

        // copy Bitmap to Image
        REPD(y, image.h())
        CopyFast(image.data() + image.pitch() * y, bitmap_image.data() + bitmap_image.pitch() * y, image.pitch());
#endif
        return font;
    }
};
/******************************************************************************/
#if WINDOWS_OLD
static const Byte WinGDIGamma[] = {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 17, 18, 18, 19, 19, 20, 20, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 37, 38, 39, 40, 41, 41, 42, 43, 44, 45, 45, 46, 47, 48, 49, 50, 51, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 82, 84, 85, 86, 87, 88, 90, 91, 92, 93, 95, 96, 97, 99, 100, 101, 103, 104, 105, 107, 108, 109, 111, 112, 114, 115, 116, 118, 119, 121, 122, 124, 125, 127, 128, 130, 131, 133, 134, 136, 138, 139, 141, 142, 144, 146, 147, 149, 151, 152, 154, 156, 157, 159, 161, 163, 164, 166, 168, 170, 171, 173, 175, 177, 179, 181, 183, 184, 186, 188, 190, 192, 194, 196, 198, 200, 202, 204, 206, 208, 210, 212, 214, 216, 218, 220, 222, 224, 226, 229, 231, 233, 235, 237, 239, 242, 244, 246, 248, 250, 253, 255};
ASSERT(SIZE(WinGDIGamma) == 256);
#endif
#if 0
#pragma message("!! Warning: Use this only for debugging !!")
static struct WinGDIGammaCreate
{
   WinGDIGammaCreate()
   {
      Str s; FREP(256){if(i)s+=", "; s+=RoundPos(SRGBToLinear(i/255.0f)*255);} ClipSet(s); Exit(s);
   }
}WinGDIGammaCreator;
#endif

struct FontCreate : FontCreateBase {
    Int scaled_size, draw_border, char_border, padd_ul, padd_dr, shadow_blur, shadow_offset;
    VecI2 draw_size;      // size of the buffer used for drawing
    Memc<FontChar> chars; // chars to create
    Int processed;
    Int leftToProcess() C { return Max(0, chars.elms() - processed - SPECIAL_CHARS); } // skip special chars
    IMAGE_TYPE imageTypeTemp() C { return (mode == Font::SUB_PIXEL) ? FONT_IMAGE_TYPE_SUB_PIXEL : FONT_IMAGE_TYPE; }
    MemtN<SystemFontDrawContext, 16> dcs;

    FontCreate(C Params &src) : FontCreateBase(src) {
        char_border = 4;

        // adjust options
        MAX(size, 0);
        scaled_size = RoundU(size * scale);
        if (mode == Font::SUB_PIXEL) {
            image_type = FONT_IMAGE_TYPE_SUB_PIXEL;
            mip_maps = 1;
            shadow_opacity = 0; // disable shadows for sub pixels
        } else {
            image_type = (FONT_SRGB ? ImageTypeIncludeSRGB : ImageTypeExcludeSRGB)(image_type);
        }
        Clamp(max_image_size, 0, 65536);
        if (image_type == IMAGE_PVRTC1_2 || image_type == IMAGE_PVRTC1_4 || image_type == IMAGE_PVRTC1_2_SRGB || image_type == IMAGE_PVRTC1_4_SRGB)
            mip_maps = 1; // PVRTC has too low quality to enable mip-maps

        // drawing
        T.draw_border = 1;
        T.draw_size.set(scaled_size * 5 / 4 + draw_border * 2, scaled_size + draw_border * 2);

        // shadows
        if (shadow_opacity <= 0) {
            padd_ul = padd_dr = shadow_blur = shadow_offset = 0;
        } else {
            Int border = Max(0, RoundPos(size * src.shadow_blur)),
                offset = (shadow_diagonal ? Max(1, RoundPos(size * 0.045f)) : 0);

            T.padd_ul = Max(0, border - offset);
            T.padd_dr = Max(0, border + offset);

            T.shadow_blur = border;
            T.shadow_offset = offset;
        }

        // list of characters (remove those that repeat)
        Str chars;
        chars.reserve(src.characters.length());
        FREPA(src.characters) {
            Char c = src.characters[i];
            ASSERT(SPECIAL_CHARS == 3); // skip SPECIAL_CHARS, they will be manually inserted later
            if (c != ' '                // always added automatically
                && c != FullWidthSpace  // always added automatically
                && c != '\t'            // always added automatically
                && c != Nbsp            // always drawn as space
                && !Contains(chars, c)  // don't add same characters multiple times
            )
                chars += c;
        }
        T.chars.reserve(chars.length() + SPECIAL_CHARS);
        T.chars.setNum(chars.length());
        REPAO(T.chars).chr = chars[i];
        processed = 0;
    }
    Bool createFont() {
        if (fonts.New().create(system_font, scaled_size, mode, weight, scale)) { // create alternatives in case primary font doesn't have needed characters
            CChar8 *alternatives[] = {"Tahoma"};
            ASSERT(ELMS(alternatives) + 1 == TOTAL_CREATE_FONTS); // "Tahoma" was chosen because it has a nice version of '⏎'
            FREPA(alternatives)
            if (!fonts.New().create(alternatives[i], scaled_size, mode, weight, scale))
                fonts.removeLast();
            return true;
        }
        return false;
    }
    Bool createDrawContexts() {
        dcs.setNum(Cpu.threads());
        FREPA(dcs)
        if (!dcs[i].create(draw_size.x, draw_size.y, T))
            return false;
        return true;
    }
    Bool drawCharacters() {
        MultiThreadedCall(chars, FontCreate::DrawCharacter, T, dcs.elms());
        return true;
    }
    static void DrawCharacter(FontChar &font_char, FontCreate &font_create, Int thread_index) {
        font_create.drawCharacter(font_char, font_create.dcs[thread_index]);
    }
    static Bool TestCol(Image &image, Int x) {
        REPD(y, image.h()) {
            Color c = image.color(x, y);
            if (c.r || c.g || c.b)
                return true;
        }
        return false;
    }
    static Bool TestRow(Image &image, Int y) {
        REPD(x, image.w()) {
            Color c = image.color(x, y);
            if (c.r || c.g || c.b)
                return true;
        }
        return false;
    }

    void drawCharacter(FontChar &fc, SystemFontDrawContext &dc) {
        C SystemFont *font = dc.draw(fc.chr, draw_border);
#if TEST_FONT
        if (fc.chr >= 'a' && fc.chr <= 'z' || fc.chr >= 'A' && fc.chr <= 'Z' || fc.chr >= '0' && fc.chr <= '9')
            dc.image.Export(S + "d:/" + fc.chr + ".bmp");
#endif

        // calculate width
        for (fc.ofs.x = 0; fc.ofs.x < dc.image.w(); fc.ofs.x++)
            if (TestCol(dc.image, fc.ofs.x))
                break; // start from left
        fc.size.x = 0;
        REPD(x, dc.image.w())
        if (TestCol(dc.image, x)) {
            fc.size.x = x - fc.ofs.x + 1;
            break;
        } // start from right

        // calculate height
        for (fc.ofs.y = 0; fc.ofs.y < dc.image.h(); fc.ofs.y++)
            if (TestRow(dc.image, fc.ofs.y))
                break; // start from top
        fc.size.y = 0;
        REPD(y, dc.image.h())
        if (TestRow(dc.image, y)) {
            fc.size.y = y - fc.ofs.y + 1;
            break;
        } // start from bottom

        // set char image
        Image &img = fc.image;
        if (img.createSoft(fc.size.x, fc.size.y, 1, imageTypeTemp())) {
            // copy
            Bool clear_shadow = (shadow_opacity <= 0);

            REPD(y, img.h())
            REPD(x, img.w()) {
                Color c = dc.image.color(fc.ofs.x + x, fc.ofs.y + y);
                if (mode == Font::SUB_PIXEL) {
                    c.a = AvgI(c.r, c.g, c.b);
                } else {
                    Byte intensity = AvgI(c.r, c.g, c.b);
#if USE_FREE_TYPE
                    // FreeType doesn't have gamma applied
#elif WINDOWS_OLD
                    intensity = WinGDIGamma[intensity]; // remove gamma applied by WinGDI
#endif
                    c.set(intensity, clear_shadow ? 0 : intensity, 0, 0); // R=font, G=shadow #FontImageLayout
                }
                img.color(x, y, c);
            }

            // calculate widths precisely
            Int w = img.w(), offset_scale = DivRound(scaled_size - size, 2), offset = draw_border - fc.ofs.y; // map from 'fc.image' to 'dc.image'
            FREP(FONT_WIDTH_TEST) {
                Int y1 = ((i == 0) ? 0 : offset_scale + i * size / FONT_WIDTH_TEST) + offset,                                   // for first step, ignore 'offset_scale' and always go from the start
                    y2 = ((i == FONT_WIDTH_TEST - 1) ? scaled_size : offset_scale + (i + 1) * size / FONT_WIDTH_TEST) + offset; // for  last step, ignore 'offset_scale' and always go to   the end

                if (y2 <= y1)
                    y2 = y1 + 1; // make sure that at least one iteration is performed, because search is exclusive "y<y2"
                Int x, y, found;
                for (x = 0, found = 255; x < w; x++)
                    for (y = y1; y < y2; y++)
                        if (img.pixel(x, y)) {
                            found = Min(254, x);
                            goto found_l;
                        }
            found_l:
                if (fc.chr == u'ำ' && found != 255)
                    found = Min(found + scaled_size / 9, 254);
                fc.widths[0][i] = found; // use "Min(254" because 255 means nothing was found, 'ำ' needs to be placed closer to other characters
                for (x = 0, found = 255; x < w; x++)
                    for (y = y1; y < y2; y++)
                        if (img.pixel(w - 1 - x, y)) {
                            found = Min(254, x);
                            goto found_r;
                        }
            found_r:
                fc.widths[1][i] = found; // use "Min(254" because 255 means nothing was found
#if TEST_FONT && 0
                img.color(0, y1, GREEN);
                img.color(0, y2 - 1, RED); // -1 because search is exclusive "y<y2"
#endif
            }

#if TEST_FONT
            if (fc.chr >= 'a' && fc.chr <= 'z' || fc.chr >= 'A' && fc.chr <= 'Z' || fc.chr >= '0' && fc.chr <= '9')
                img.Export(S + "d:/" + fc.chr + " clip.bmp");
#endif

            // apply shadow
            if (shadow_opacity > 0) {
                ASSERT(FONT_IMAGE_TYPE == IMAGE_R8G8); // #FontImageLayout
                img._type = img._hw_type = IMAGE_L8A8; // convert R8G8 -> L8A8 needed for shadows
                img.setShadow(shadow_blur, shadow_opacity, shadow_spread, false, shadow_offset, 0, false);
                img._type = img._hw_type = FONT_IMAGE_TYPE; // convert L8A8 -> R8G8
            }
        }
        if (font)
            fc.ofs.y += font->base_line_offset;
    }
    Bool clean() {
        // remove those that don't exist
        REPA(chars)
        if (!chars[i].size.x)
            chars.remove(i, true); // check only width, because SPECIAL_CHARS have height=0

        // sort by height
        chars.sort(FontChar::Compare);

        // insert SPECIAL_CHARS as last
        ASSERT(SPECIAL_CHARS == 3);
        chars.New().setSpace(size);
        chars.New().setFullSpace(size);
        chars.New().setTab(size);

        return true;
    }
    Bool prepareFont(Font &font) {
        // set font data
        font._sub_pixel = (T.mode == Font::SUB_PIXEL);
        font._height = T.size;
        font._padd.x = font._padd.y = T.padd_ul;
        font._padd.z = font._padd.w = T.padd_dr;
        font._chrs.setNumZero(T.chars.elms());

        // set characters
        REPA(font._chrs) {
            Font::Chr &dest = font._chrs[i];
            FontChar &src = T.chars[i];
            dest.image = 0;
            dest.tex.zero();
            dest.chr = src.chr;
            dest.width = src.size.x;
            dest.width_padd = Min(dest.width + font.paddingL() + font.paddingR(), 255);
            dest.height = src.size.y;
            dest.height_padd = Min(dest.height + font.paddingT() + font.paddingB(), 255);
            dest.offset = Mid(src.ofs.y - draw_border, 0, 255); // use 'Mid' because we're storing as Byte
            Copy(dest.widths, src.widths);
        }
        return true;
    }
    Bool setImages(Font &font) {
        for (; leftToProcess();) // until there are no chars left
        {
            Int w = 0, h = 0, fits = 0;

            for (Int size = 16; size <= max_image_size; size <<= 1)
                for (Int aspect = 0; aspect < 3; aspect++) {
                    w = h = size;
                    switch (aspect) {
                    case 0:
                        w >>= 1;
                        break; // at 1st attempt try making (size/2, size  ), as textures should preferrably be taller than wider
                    case 1:
                        h >>= 1;
                        break; // at 2nd attempt try making (size  , size/2), in case this fits the characters but above method doesn't
                               // at 3rd attempt try making (size  , size  )
                    }
                    fits = countFitsInSize(w, h);
                    if (fits == leftToProcess())
                        goto all_fit;
                }
            if (fits == 0)
                return false; // we still have characters to process but none of them fits

        all_fit:;
            applyFonts(font, w, h, fits);
        }
        return true;
    }
    Int countFitsInSize(Int w, Int h) {
        Int x = 1, y = 1, max_h = 0; // start from 1, 1 because of texture filtering
        FREP(leftToProcess())        // this skips SPECIAL_CHARS
        {
            VecI2 size = chars[i + processed].image.size();
            if (y + size.y + 1 > h)
                return i;           // exceeds height
            if (x + size.x + 1 > w) // exceeds width
            {                       // go to next line
                x = 1;
                y += max_h + char_border;
                max_h = 0;
                if (x + size.x + 1 > w || y + size.y + 1 > h)
                    return i; // doesn't fit
            }

            // write
            x += size.x + char_border;
            MAX(max_h, size.y);
        }
        return leftToProcess(); // all of them fit
    }
    void applyFonts(Font &font, Int w, Int h, Int fits) {
        // set font image
        Int image_index = font._images.elms();
        Image &image = font._images.New().mustCreateSoft(w, h, 1, imageTypeTemp()).clear(); // create as soft

        Int x = 1, y = 1, max_h = 0;
        const Flt eps = 0.5f * 0; // don't add this epsilon because it disables per pixel font, and when enabled it's rarely even visible (only when font is very small but magnified)

        FREP(fits) {
            FontChar &src = T.chars[i + processed];
            Font::Chr &dest = font._chrs[i + processed];
            VecI2 size = src.image.size();
            if (x + size.x + 1 > w) // exceeds width
            {                       // go to next line
                x = 1;
                y += max_h + char_border;
                max_h = 0;
            }
            dest.image = image_index;
            dest.tex.set(Flt(x - eps) / w, Flt(y - eps) / h, Flt(x + size.x + eps) / w, Flt(y + size.y + eps) / h);

            REPD(sy, size.y)
            REPD(sx, size.x) {
                Color c = src.image.color(sx, sy);
                // #FontImageLayout we could do some advanced remapping here
                image.color(x + sx, y + sy, c);
            }

            x += size.x + char_border;
            MAX(max_h, size.y);
        }
        processed += fits;
    }
};
static void CompressTextures(Image &image, FontCreate &fc, Int thread_index) {
    ThreadMayUseGPUData();
    image.mustCopy(image, -1, -1, -1, fc.image_type, fc.software ? IMAGE_SOFT : IMAGE_2D, fc.mip_maps);
}
Bool Font::create(C Params &params) {
    del();

    SyncUnlocker locker(D._lock); // if this is called in 'Draw' then 'D._lock' is already locked, however multi-threaded font creation may want to lock 'D._lock' resulting in deadlock, therefore we need to first unlock it
    FontCreate fc(params);
    if (fc.createFont())
        if (fc.createDrawContexts())
            if (fc.drawCharacters())
                if (fc.clean())
                    if (fc.prepareFont(T))
                        if (fc.setImages(T)) {
#if USE_FREE_TYPE
#elif WINDOWS_OLD
                            if (fc.mode == Font::SMOOTHED) // font with extra smoothing uses anti-aliasing, 2 pixels with half transparency are used instead of 1, this means that widths are 1 extra pixel bigger
                            {
                                _padd.z++; // since we'll decrease the width by 1, then we need to increase the right padding by 1
                                REPA(_chrs) {
                                    Font::Chr &c = _chrs[i];
                                    c.width = Max(c.width - 1, 0);
                                    // c.width_padd remains the same, because width was decreased but padding increased
                                    REP(FONT_WIDTH_TEST)
                                    if (c.widths[1][i] != 0xFF)
                                        c.widths[1][i] = Mid(c.widths[1][i] + 1, 0, 254); // here increase by 1 and not decrease, because 'widths[1]' means amount of distance from the right side
                                                                                          // REP(FONT_WIDTH_TEST)if(c.widths[0][i]!=0xFF)c.widths[0][i]=Mid(c.widths[0][i]+1, 0, 254); // don't use this as that's too much
                                }
                            }
#endif

                            MultiThreadedCall(_images, CompressTextures, fc);
                            setRemap();
                            return true;
                        }
    del();
    return false;
}
/******************************************************************************/
void DisplayDraw::textDepth(Bool use, Flt depth) {
    if (D._text_depth = use)
        Sh.FontDepth->set(depth);
}
void DisplayDraw::textBackgroundAuto() { Sh.FontLum = GetShaderParam("FontLum"); }
void DisplayDraw::textBackgroundReset(ShaderParam *&sp, Vec &col) {
    sp = Sh.FontLum;
    textBackgroundAuto();
    col = Sh.FontLum->getVec();
}
void DisplayDraw::textBackgroundSet(ShaderParam *sp, C Vec &col) {
    SPSet("FontLum", col);
    Sh.FontLum = sp;
}
void DisplayDraw::textBackgroundBlack() {
    SPSet("FontLum", Vec(1));
    Sh.FontLum = &Sh.Dummy;
} // set dummy to disable changing
void DisplayDraw::textBackgroundWhite() {
    SPSet("FontLum", Vec(0));
    Sh.FontLum = &Sh.Dummy;
} // set dummy to disable changing
/******************************************************************************/
} // namespace EE
/******************************************************************************/
