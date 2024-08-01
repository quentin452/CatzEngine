// TODO ADD PROFILE_START AND PROFILE_STOP PROFILERS
/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
#define BOM_UTF_8 0xBBEF
#define BOM_UTF_8_3 0xBF

#define BOM_UTF_16 0xFEFF
/******************************************************************************/
ENCODING LoadEncoding(File &f) // assumes f.pos() is 0
{
    if (f.size() >= 2) // encoding can be present only if there are at least 2 bytes
    {
        switch (f.getUShort()) {
        case BOM_UTF_16:
            return UTF_16;
        case BOM_UTF_8:
            if (f.getByte() == BOM_UTF_8_3)
                return UTF_8;
            break;
        }
        f.pos(0); // if there was no byte order mark found then reset position to the start
    }
    return ANSI; // default encoding
}
static void SaveEncoding(File &f, ENCODING encoding) {
    if (!f.size())
        switch (encoding) // we can save encoding only at the start of the file
        {
        case UTF_16:
            f.putUShort(BOM_UTF_16);
            break;
        case UTF_8:
            f.putMulti(U16(BOM_UTF_8), Byte(BOM_UTF_8_3));
            break;
        }
}
/******************************************************************************/
void FileText::zero() {
    indent = INDENT_TABS;
    depth = 0;
    _code = ANSI;
    _get = _put = '\0';
}
FileText &FileText::del() {
    _f.del();
    zero();
    return T;
}
FileText::FileText() { zero(); }

Bool FileText::end() C { return _f.end() && !_get; }

FileText &FileText::writeMem(ENCODING encoding, Cipher *cipher) {
    del();
    _f.writeMem(65536, cipher);
    SaveEncoding(_f, T._code = encoding);
    return T;
}
Bool FileText::write(C Str &name, ENCODING encoding, Cipher *cipher) {
    del();
    if (_f.write(name, cipher)) {
        SaveEncoding(_f, T._code = encoding);
        return true;
    }
    return false;
}
Bool FileText::append(C Str &name, ENCODING encoding, Cipher *cipher) {
    del();
    if (_f.readStd(name, cipher))
        if (_f.size())
            encoding = LoadEncoding(_f); // take encoding from the file if it has some data
    if (_f.append(name, cipher)) {
        SaveEncoding(_f, T._code = encoding);
        return true;
    }
    return false;
}
Bool FileText::read(C Str &name, Cipher *cipher) {
    del();
    if (_f.read(name, cipher)) {
        _code = LoadEncoding(_f);
        return true;
    }
    return false;
}
Bool FileText::read(C UID &id, Cipher *cipher) {
    del();
    if (_f.read(id, cipher)) {
        _code = LoadEncoding(_f);
        return true;
    }
    return false;
}
Bool FileText::read(C Str &name, Pak &pak) {
    del();
    if (_f.read(name, pak)) {
        _code = LoadEncoding(_f);
        return true;
    }
    return false;
}
Bool FileText::read(C UID &id, Pak &pak) {
    del();
    if (_f.read(id, pak)) {
        _code = LoadEncoding(_f);
        return true;
    }
    return false;
}
FileText &FileText::readMem(CPtr data, Int size, Int encoding, Cipher *cipher) {
    del();
    _f.readMem(data, size, cipher);
    _code = LoadEncoding(_f); // load encoding always to skip potential BOM
    if (!_code && encoding >= 0)
        _code = (ENCODING)encoding; // override encoding if it was specified
    return T;
}
FileText &FileText::readMem(C Str &data) {
    del();
    _f.readMem(data(), data.length() * SIZE(Char));
    _code = UTF_16;
    return T;
}
/******************************************************************************/
Long FileText::charsLeft() C {
    return (_code == UTF_16) ? _f.left() / SIZEI(Char) : _f.left();
}
/******************************************************************************/
FileText &FileText::startLine() {
    if (indent)
        REP(depth)
    if (indent == INDENT_TABS)
        putChar('\t');
    else
        putText("   ");
    return T;
}
FileText &FileText::endLine() {
    switch (_code) {
    case ANSI:
        _f.putByte(0x0A);
        break;
    case UTF_16:
        _f.putUShort(0x000A);
        break;

    case UTF_8:
    case UTF_8_NAKED:
        _f.putByte(0x0A);
        _put = '\0';
        break; // MULTI1 must be in range 0xDC00..0xDFFF, but here we have '\n' so just clear any '_put'
    }
    return T;
}
FileText &FileText::putChar(Char8 c) {
    switch (_code) {
    case ANSI:
        _f.putByte(c);
        break;
    case UTF_16:
        _f.putUShort(Char8To16(c));
        break;

    case UTF_8:
    case UTF_8_NAKED: {
        _put = '\0'; // MULTI1 must be in range 0xDC00..0xDFFF, but here we have Char8 that will never convert to that range, so just clear any '_put'
        U8 u = c;
        if (u <= 0x7F)
            _f.putByte(u);
        else
            _f.putMulti(Byte(0xC0 | (u >> 6)), Byte(0x80 | (u & 0x3F)));
    } break;
    }
    return T;
}
FileText &FileText::putChar(Char c) {
    switch (_code) {
    case ANSI:
        _f.putByte(Char16To8(c));
        break;
    case UTF_16:
        _f.putUShort(c);
        break;

    case UTF_8:
    case UTF_8_NAKED: {
        if (_put) // check if we have MULTI0
        {
            if (c >= 0xDC00 && c <= 0xDFFF) // MULTI1 must be in range 0xDC00..0xDFFF
            {
                U32 u = (((Unsigned(_put) - 0xD800) << 10) | (Unsigned(c) - 0xDC00)) + 0x10000; // decode U16 c c1 -> U32 u
                _f.putMulti(Byte(0xF0 | (u >> 18)), Byte(0x80 | ((u >> 12) & 0x3F)), Byte(0x80 | ((u >> 6) & 0x3F)), Byte(0x80 | (u & 0x3F)));
            }
            _put = '\0';
            break;
        }
        if (c >= 0xD800 && c <= 0xDBFF) {
            _put = c;
            break;
        } // MULTI0, needs to have MULTI1 so put to buffer and writer later
        U16 u = c;
        if (u <= 0x07F)
            _f.putByte(u);
        else if (u <= 0x7FF)
            _f.putMulti(Byte(0xC0 | (u >> 6)), Byte(0x80 | (u & 0x3F)));
        else
            _f.putMulti(Byte(0xE0 | (u >> 12)), Byte(0x80 | ((u >> 6) & 0x3F)), Byte(0x80 | (u & 0x3F)));
    } break;
    }
    return T;
}
FileText &FileText::putText(C Str &text) {
    switch (_code) {
    case ANSI: {
        Str8 t = text;
        _f.putN(t(), t.length());
    } break;

    case UTF_16:
        _f.putN(text(), text.length());
        break;

    case UTF_8:
    case UTF_8_NAKED: {
#if 0 // slower
         FREPA(text)putChar(text[i]);
#else // faster
        Char8 temp[65536];
        const Int temp_elms = Elms(temp) - 3; // use size -3 because we may be writing 4 characters in one step
        Int i = 0, temp_pos = 0;
        if (_put && text.is())
            putChar(text()[i++]); // check if we have MULTI0, then save MULTI1 manually, () avoids range checks
        for (;;) {
            Bool end = (i >= text.length());
            if (end || !InRange(temp_pos, temp_elms)) // if finished, or there's no more room in the buffer
            {
                if (temp_pos) {
                    _f.putN(temp, temp_pos);
                    temp_pos = 0;
                } // flush
                if (end)
                    break;
            }
            U16 c = text()[i++]; // () avoids range checks
            if (c <= 0x07F)
                temp[temp_pos++] = c;
            else if (c <= 0x7FF) {
                temp[temp_pos++] = (0xC0 | (c >> 6));
                temp[temp_pos++] = (0x80 | (c & 0x3F));
            } else
#if 1 // since we operate on Char we must treat it as UTF-16, there 0xD800..0xDBFF are used to encode 2 Chars
                if (c >= 0xD800 && c <= 0xDBFF) // MULTI0
            {
                /* U32 -> 2x U16 formula:
                       u-=0x10000;
                       c0=0xD800+(u>>10  ); MULTI0
                       c1=0xDC00+(u&0x3FF); MULTI1 */
                U16 c1 = text[i++];
                U32 u = (((c - 0xD800) << 10) | (c1 - 0xDC00)) + 0x10000; // decode U16 c c1 -> U32 u
                temp[temp_pos++] = (0xF0 | (u >> 18));
                temp[temp_pos++] = (0x80 | ((u >> 12) & 0x3F));
                temp[temp_pos++] = (0x80 | ((u >> 6) & 0x3F));
                temp[temp_pos++] = (0x80 | (u & 0x3F));
            } else
#endif
            {
                temp[temp_pos++] = (0xE0 | (c >> 12));
                temp[temp_pos++] = (0x80 | ((c >> 6) & 0x3F));
                temp[temp_pos++] = (0x80 | (c & 0x3F));
            }
        }
#endif
    } break;
    }
    return T;
}
FileText &FileText::putLine(C Str &text) {
    return startLine().putText(text).endLine();
}
/******************************************************************************/
Char FileText::getChar() {
    switch (_code) {
    default:
    error:
        return 0;
    case UTF_16:
        return _f.getUShort();

    case ANSI: {
        Char8 c[2];
        c[0] = _f.getByte();
#if WINDOWS // Code Pages are used on Windows
        if ((c[0] & 0x80) && !_f.end()) // this may be a 2-character multi-byte wide char
        {
            c[1] = _f.getByte();
            wchar_t w[2];
            Int size = MultiByteToWideChar(CP_ACP, 0, c, Elms(c), w, Elms(w));
            if (size == 1)
                return w[0]; // if 2 bytes generated 1 wide char, then return it
            _f.skip(-1);     // otherwise, go back the extra byte that we've read, because we want to get only 1 char at this time
        }
#endif
        return Char8To16(c[0]);
    } break;

    case UTF_8:
    case UTF_8_NAKED: {
        if (_get) {
            Char c = _get;
            _get = '\0';
            return c;
        }

        Byte b0 = _f.getByte();
        if (!(b0 & (1 << 7)))
            return b0; // this handles NUL
        if (!(b0 & (1 << 6)))
            goto error; // bit 6 should be always on, here don't skip back, so we can always advance at least 1 char

        const Bool skip_back = false; // if character is invalid, then revert reading it, skip back, so other codes can process it
        Char c;
        Byte b1 = _f.getByte();
        if ((b1 & ((1 << 7) | (1 << 6))) != (1 << 7)) {
            if (skip_back)
                _f.skip(-1);
            goto error;
        }
        b1 &= 0x3F; // bit 7 should be always on, bit 6 always off, this handles NUL
        if (!(b0 & (1 << 5))) {
            b0 &= 0x1F;
            c = (b1 | (b0 << 6));
        } else {
            Byte b2 = _f.getByte();
            if ((b2 & ((1 << 7) | (1 << 6))) != (1 << 7)) {
                if (skip_back)
                    _f.skip(-1);
                goto error;
            }
            b2 &= 0x3F; // bit 7 should be always on, bit 6 always off, this handles NUL
            if (!(b0 & (1 << 4))) {
                b0 &= 0x0F;
                c = (b2 | (b1 << 6) | (b0 << 12));
            } else {
                Byte b3 = _f.getByte();
                if ((b3 & ((1 << 7) | (1 << 6))) != (1 << 7)) {
                    if (skip_back)
                        _f.skip(-1);
                    goto error;
                }
                b3 &= 0x3F; // bit 7 should be always on, bit 6 always off, this handles NUL
                if (!(b0 & (1 << 3))) {
                    b0 &= 0x07;
                    UInt u = (b3 | (b2 << 6) | (b1 << 12) | (b0 << 18));
                    // since we return Char, then we must convert 'u' to UTF-16 - https://en.wikipedia.org/wiki/UTF-16#Description
                    if (u <= 0xFFFF)
                        c = u;
                    else // if(u<=0xD7FF || u>=0xE000 && u<=0xFFFF)c=u; ranges U+0000 to U+D7FF and U+E000 to U+FFFF are represented natively
                        if (u <= 0x10FFFF) {
                        u -= 0x10000;
                        c = 0xD800 + (u >> 10);      // MULTI0
                        _get = 0xDC00 + (u & 0x3FF); // MULTI1
                    } else
                        c = '?'; // unsupported
                } else
                    c = '?'; // unsupported
            }
        }
        return c;
    } break;
    }
}
FileText &FileText::skipLine() {
    for (; !end();) {
        if (getChar() == '\n')
            break;
    }
    return T;
}
FileText &FileText::fullLine(Str &s) {
    s.clear();
    for (; !end();) {
        Char c = getChar();
        if (c == '\n')
            break;
        if (Safe(c))
            s += c;
    }
    return T;
}
FileText &FileText::getLine(Str &s) {
    s.clear();
    for (Bool start = true; !end();) {
        Char c = getChar();
        if (c == '\n')
            break;
        if (Safe(c)) {
            if (!WhiteChar(c))
                start = false;
            if (!start)
                s += c;
        }
    }
    return T;
}
FileText &FileText::getLine(Str8 &s) {
    s.clear();
    for (Bool start = true; !end();) {
        Char c = getChar();
        if (c == '\n')
            break;
        if (Safe(c)) {
            if (!WhiteChar(c))
                start = false;
            if (!start)
                s += c;
        }
    }
    return T;
}
FileText &FileText::getAll(Str &s) {
    s.clear();
    Int chars = charsLeft();
    if (chars > 0) {
        s.reserve(chars);
        switch (_code) {
        case UTF_16: {
            s._length = _f.getReturnSize(s._d.data(), chars * SIZEI(Char)) / SIZEI(Char);
            if (s.length() != chars)
                _f.error();
#if WINDOWS
        utf_16:
#endif
            Int length = s.length();
            CChar *t = s();
            FREP(length) {
                Char c = t[i];
                if (!Safe(c)) // found invalid char
                {
                    Str temp;
                    temp.reserve(length);
                    CopyFastN(temp._d.data(), t, i);
                    temp._length += i; // copy everything up to this point
#if 0                                  // simple iteration (slower)
                  for(; ++i<length; ){Char c=t[i]; if(Safe(c))temp._d[temp._length++]=c;}
#else // batched copying
                    Int last_ok = i + 1;
                    for (; ++i < length;) {
                        Char c = t[i];
                        if (!Safe(c)) {
                            Int copy = i - last_ok;
                            CopyFastN(temp._d.data() + temp._length, t + last_ok, copy);
                            temp._length += copy;
                            last_ok = i + 1;
                        }
                    }
                    Int copy = length - last_ok;
                    CopyFastN(temp._d.data() + temp._length, t + last_ok, copy);
                    temp._length += copy;
#endif
                    Swap(temp, s);
                    break;
                }
            }
        } break;

        case ANSI: {
            Str8 s8;
            s8.reserve(chars);
            Int length = s8._length = _f.getReturnSize(s8._d.data(), chars);
            if (length != chars)
                _f.error();
            Char *t = s._d.data();
            CChar8 *t8 = s8();
            FREP(length) {
                Char8 c = t8[i];
                if (Safe(c)) {
#if WINDOWS // Code Pages are used on Windows
                    if (c & 0x80) // this may be a 2-character multi-byte wide char
                    {
                        s._length = MultiByteToWideChar(CP_ACP, 0, t8, length, WChar(s._d.data()), chars); // let OS handle multi-byte conversion
                        goto utf_16;                                                                       // we've converted to 16-bit so process as UTF-16
                    }
#endif
                    *t++ = Char8To16(c);
                }
            }
            s._length = t - s._d.data();
        } break;

        default: {
            Char *t = s._d.data();
            for (; !end();) {
                Char c = getChar();
                if (Safe(c))
                    *t++ = c;
            }
            s._length = t - s._d.data();
        } break;
        }
        s._d[s._length] = '\0';
    }
    return T;
}
FileText &FileText::rewind() {
    depth = 0;
    _get = _put = '\0';
    _f.pos(0);
    LoadEncoding(_f); // we already know the encoding, but we need to skip the byte order mark at start, don't set it to '_code' because 'UTF_8_NAKED' may had been used
    return T;
}
Bool FileText::copy(File &dest) {
    depth = 0;
    _get = _put = '\0';
    if (!_f.pos(0))
        return false;
    return _f.copy(dest);
}
Char FileText::posInfo(Long pos, VecI2 &col_line) {
    rewind();
    Char last = '\0';
    VecI2 cl = 0;
    if (pos >= T.pos())
        for (MIN(pos, size());;) // this will allow reading right after the last character
        {
            last = getChar();
            if (T.pos() > pos || !ok())
                break;
            if (last == '\n') {
                cl.x = 0;
                cl.y++;
            } else if (last != '\r')
                cl.x++;
        }
    col_line = cl;
    return last;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
void FileTextEx::get(Int &i) { i = getInt(); }
void FileTextEx::get(Flt &f) { f = getFlt(); }
void FileTextEx::get(Dbl &d) { d = getDbl(); }
void FileTextEx::get(Vec2 &v) {
    v.x = getFlt();
    v.y = getFlt();
}
void FileTextEx::get(Vec &v) {
    v.x = getFlt();
    v.y = getFlt();
    v.z = getFlt();
}
void FileTextEx::get(Vec4 &v) {
    v.x = getFlt();
    v.y = getFlt();
    v.z = getFlt();
    v.w = getFlt();
}
void FileTextEx::get(VecI2 &v) {
    v.x = getInt();
    v.y = getInt();
}
void FileTextEx::get(VecI &v) {
    v.x = getInt();
    v.y = getInt();
    v.z = getInt();
}
void FileTextEx::get(VecI4 &v) {
    v.x = getInt();
    v.y = getInt();
    v.z = getInt();
    v.w = getInt();
}
void FileTextEx::get(VecB4 &v) {
    v.x = getInt();
    v.y = getInt();
    v.z = getInt();
    v.w = getInt();
}

Bool FileTextEx::getBool() { return TextBool(getWord()); }
Int FileTextEx::getInt() { return TextInt(getWord()); }
UInt FileTextEx::getUInt() { return TextUInt(getWord()); }
Flt FileTextEx::getFlt() { return TextFlt(getWord()); }
Dbl FileTextEx::getDbl() { return TextDbl(getWord()); }
Vec2 FileTextEx::getVec2() {
    Vec2 v;
    get(v);
    return v;
}
Vec FileTextEx::getVec() {
    Vec v;
    get(v);
    return v;
}
Vec4 FileTextEx::getVec4() {
    Vec4 v;
    get(v);
    return v;
}
VecI2 FileTextEx::getVecI2() {
    VecI2 v;
    get(v);
    return v;
}
VecI FileTextEx::getVecI() {
    VecI v;
    get(v);
    return v;
}
VecI4 FileTextEx::getVecI4() {
    VecI4 v;
    get(v);
    return v;
}
VecB4 FileTextEx::getVecB4() {
    VecB4 v;
    get(v);
    return v;
}

C Str &FileTextEx::getWord() {
    text.clear();
    for (Bool start = true; !end();) {
        Char c = getChar();
        if (!c)
            break;
        if (WhiteChar(c))
            if (start)
                continue;
            else
                break;
        if (Safe(c)) {
            start = false;
            text += c;
        }
    }
    return text;
}
C Str &FileTextEx::getName() {
    text.clear();
    for (; !end();) {
        Char c = getChar();
        if (!c || c == '\n')
            break;
        if (c == '"') {
            for (; !end();) {
                Char c = getChar();
                if (!c || c == '\n' || c == '"')
                    break;
                if (Safe(c))
                    text += c;
            }
            break;
        }
    }
    return text;
}
/******************************************************************************/
Bool FileTextEx::getIn() {
    for (; !end();) {
        getWord();
        if (text.first() == '{')
            return true;
        if (text.first() == '}')
            return false;
    }
    return false;
}
void FileTextEx::getOut() {
    for (Int depth = 0; !end();) {
        getWord();
        if (text.first() == '{')
            depth++;
        else if (text.first() == '}')
            if (--depth < 0)
                break;
    }
}
Bool FileTextEx::level() {
    for (; !end();) {
        getWord();
        if (text.first() == '}')
            break;
        if (text.first() == '{') {
            getOut();
            continue;
        }
        return true;
    }
    return false;
}
/******************************************************************************/
