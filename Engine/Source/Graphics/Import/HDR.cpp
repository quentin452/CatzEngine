// TODO ADD PROFILE_START AND PROFILE_STOP PROFILERS
/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
namespace EE {
/******************************************************************************/
static void GetLine(Str8 &line, File &f) {
    line.clear();
    for (;;) {
        Char8 c;
        f >> c;
        if (c == '\n' || !c)
            return;
        line += c;
    }
}
static void RGBEToRGB(C Byte (&rgbe)[4], Vec &rgb) {
    if (rgbe[3]) {
        Flt exp = ldexpf(1.0f, rgbe[3] - (128 + 8));
        rgb.set(rgbe[0] * exp,
                rgbe[1] * exp,
                rgbe[2] * exp);
    } else
        rgb.zero();
}
static Bool ReadPixels(File &f, Vec *data, Int w) {
    FREPD(x, w) {
        Byte rgbe[4];
        if (!f.getFast(rgbe))
            return false;
        RGBEToRGB(rgbe, data[x]);
    }
    return true;
}
static Bool ReadPixelsRLE(File &f, Vec *data, Int w, Memt<Byte> &buffer) {
    if ((w < 8) || (w > 0x7FFF))
        return ReadPixels(f, data, w); // RLE unavailable

    Byte rgbe[4];
    if (!f.getFast(rgbe))
        return false;
    if ((rgbe[0] != 2) || (rgbe[1] != 2) || (rgbe[2] & 0x80)) // not RLE
    {
        RGBEToRGB(rgbe, *data);
        return ReadPixels(f, data + 1, w - 1);
    }

    if (((rgbe[2] << 8) | rgbe[3]) != w)
        return false; // invalid width
    buffer.setNumDiscard(4 * w);
    Byte *ptr = buffer.data();
    REP(4) // read all channels
    {
        Byte *ptr_end = ptr + w;
        while (ptr < ptr_end) {
            Byte buf[2];
            if (!f.getFast(buf))
                return false;
            if (buf[0] > 128) // RLE
            {
                Int count = buf[0] - 128;
                if ((count == 0) || (count > ptr_end - ptr))
                    return false; // invalid data
                while (count-- > 0)
                    *ptr++ = buf[1];
            } else // no RLE
            {
                Int count = buf[0];
                if ((count == 0) || (count > ptr_end - ptr))
                    return false; // invalid data
                *ptr++ = buf[1];
                if (--count > 0) {
                    if (!f.getFast(ptr, count))
                        return false;
                    ptr += count;
                }
            }
        }
    }
    FREP(w) {
        rgbe[0] = buffer[i];
        rgbe[1] = buffer[i + w];
        rgbe[2] = buffer[i + 2 * w];
        rgbe[3] = buffer[i + 3 * w];
        RGBEToRGB(rgbe, *data++);
    }
    return true;
}
/******************************************************************************/
Bool Image::ImportHDR(File &f) {
    Char8 header[11];
    if (f.getFast(header) && EqualMem(header, "#?RADIANCE\n", SIZE(header))) {
        Str8 line;
        Bool ok = false;
        for (;;) {
            GetLine(line, f);
            if (!line.is())
                break;
            if (line == "FORMAT=32-bit_rle_rgbe")
                ok = true;
        }
        if (ok) {
            GetLine(line, f);
            VecI2 size = -1;
            if (Starts(line, "-Y ")) {
                CalcValue x, y;
                CChar8 *t = TextValue(line() + 3, y, false);
                if (Starts(t, " +X ")) {
                    TextValue(t + 4, x, false);
                    if (x.type && y.type)
                        size.set(x.asInt(), y.asInt());
                }
            }
            if (size.x >= 0 && size.y >= 0 && createSoft(size.x, size.y, 1, IMAGE_F32_3)) {
                Memt<Byte> buffer;
                FREPD(y, h())
                if (!ReadPixelsRLE(f, &pixF3(0, y), w(), buffer))
                    goto error;
                return true;
            }
        }
    }
error:
    del();
    return false;
}
Bool Image::ImportHDR(C Str &name) {
    File f;
    if (f.read(name))
        return ImportHDR(f);
    del();
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
