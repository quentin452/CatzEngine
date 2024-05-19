/******************************************************************************/
#include "stdafx.h"

#include "../../../../ThirdPartyLibs/begin.h"

#define BC_LIB_DIRECTX 1
#define BC_LIB_TEXGENPACK 2 // faster than BC_LIB_DIRECTX
#define BC6_DEC BC_LIB_TEXGENPACK
#define BC7_DEC BC_LIB_TEXGENPACK

#if BC6_DEC == BC_LIB_DIRECTX || BC7_DEC == BC_LIB_DIRECTX
#include "../../../../ThirdPartyLibs/DirectXMath/include.h"
#include "../../../../ThirdPartyLibs/DirectXTex/BC.h"
// #include "../../../../ThirdPartyLibs/DirectXTex/BC.cpp"
// #include "../../../../ThirdPartyLibs/DirectXTex/BC4BC5.cpp"
#include "../../../../ThirdPartyLibs/DirectXTex/BC6HBC7.cpp"
#endif

#include "../../../../ThirdPartyLibs/end.h"

#if BC6_DEC == BC_LIB_TEXGENPACK || BC7_DEC == BC_LIB_TEXGENPACK
namespace TGP {
#pragma runtime_checks("", off)
#include "../../../../ThirdPartyLibs/TexGenPack/bptc.c"
#pragma runtime_checks("", restore)
} // namespace TGP
#endif

namespace EE {
/******************************************************************************/
Bool (*CompressBC67)(C Image &src, Image &dest);
/******************************************************************************/
static inline void Col565(Color &color, UShort u) { color.set(U5ToByte((u >> 11) & 0x1F), U6ToByte((u >> 5) & 0x3F), U5ToByte(u & 0x1F), 255); }

static inline U64 GetU48(CPtr data) { return Unaligned(*(U32 *)data) | (U64(Unaligned(((U16 *)data)[2])) << 32); } // need to read exactly 48 instead of 64 bits to avoid memory access exception
static inline U64 GetU64(CPtr data) { return Unaligned(*(U64 *)data); }
/******************************************************************************/
// BLOCK
/******************************************************************************/
static inline void _DecompressBlockBC1(C Byte *b, Color color[4], UInt &cis) {
    UShort c0 = *(UShort *)(b),
           c1 = *(UShort *)(b + 2);
    Col565(color[0], c0);
    Col565(color[1], c1);
    if (c0 <= c1) {
        color[2].r = (color[0].r + color[1].r + 1) / 2;
        color[2].g = (color[0].g + color[1].g + 1) / 2;
        color[2].b = (color[0].b + color[1].b + 1) / 2;
        color[2].a = 255;

        color[3].zero();
    } else {
        color[2].r = (color[0].r * 2 + color[1].r * 1 + 1) / 3;
        color[2].g = (color[0].g * 2 + color[1].g * 1 + 1) / 3;
        color[2].b = (color[0].b * 2 + color[1].b * 1 + 1) / 3;
        color[2].a = 255;

        color[3].r = (color[0].r * 1 + color[1].r * 2 + 1) / 3;
        color[3].g = (color[0].g * 1 + color[1].g * 2 + 1) / 3;
        color[3].b = (color[0].b * 1 + color[1].b * 2 + 1) / 3;
        color[3].a = 255;
    }
    cis = *(UInt *)(b + 4);
}
void DecompressBlockBC1(C Byte *b, Color (&block)[4][4]) {
    Color color[4];
    UInt cis;
    _DecompressBlockBC1(b, color, cis);
    REPD(y, 4)
    REPD(x, 4) {
        Int i = x + (y << 2),            // pixel index
            ci = ((cis >> (2 * i)) & 3); // color index
        block[y][x] = color[ci];
    }
}
void DecompressBlockBC1(C Byte *b, Color *dest, Int pitch) {
    Color color[4];
    UInt cis;
    _DecompressBlockBC1(b, color, cis);
    FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
    {
        REPD(x, 4) {
            Int i = x + (y << 2),            // pixel index
                ci = ((cis >> (2 * i)) & 3); // color index
            dest[x] = color[ci];
        }
        dest = (Color *)((Byte *)dest + pitch);
    }
}
/******************************************************************************/
static inline void _DecompressBlockBC2(C Byte *b, Color color[4], UInt &cis) {
    UShort c0 = *(UShort *)(b + 8),
           c1 = *(UShort *)(b + 10);
    Col565(color[0], c0);
    Col565(color[1], c1);

    color[2].r = (color[0].r * 2 + color[1].r * 1 + 1) / 3;
    color[2].g = (color[0].g * 2 + color[1].g * 1 + 1) / 3;
    color[2].b = (color[0].b * 2 + color[1].b * 1 + 1) / 3;

    color[3].r = (color[0].r * 1 + color[1].r * 2 + 1) / 3;
    color[3].g = (color[0].g * 1 + color[1].g * 2 + 1) / 3;
    color[3].b = (color[0].b * 1 + color[1].b * 2 + 1) / 3;

    cis = *(UInt *)(b + 12);
}
void DecompressBlockBC2(C Byte *b, Color (&block)[4][4]) {
    Color color[4];
    UInt cis;
    _DecompressBlockBC2(b, color, cis);
    ULong alpha = GetU64(b);
    REPD(y, 4)
    REPD(x, 4) {
        Int i = x + (y << 2),            // pixel index
            ci = ((cis >> (2 * i)) & 3); // color index
        Color &col = color[ci];
        block[y][x].set(col.r, col.g, col.b, U4ToByte((alpha >> (4 * i)) & 15));
    }
}
void DecompressBlockBC2(C Byte *b, Color *dest, Int pitch) {
    Color color[4];
    UInt cis;
    _DecompressBlockBC2(b, color, cis);
    ULong alpha = GetU64(b);
    FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
    {
        REPD(x, 4) {
            Int i = x + (y << 2),            // pixel index
                ci = ((cis >> (2 * i)) & 3); // color index
            Color &col = color[ci];
            dest[x].set(col.r, col.g, col.b, U4ToByte((alpha >> (4 * i)) & 15));
        }
        dest = (Color *)((Byte *)dest + pitch);
    }
}
/******************************************************************************/
static inline void _DecompressBlockBC4(C Byte *b, Byte value[8], U64 &is) {
    value[0] = b[0];
    value[1] = b[1];
    if (value[0] > value[1]) {
        value[2] = (value[0] * 6 + value[1] * 1 + 3) / 7;
        value[3] = (value[0] * 5 + value[1] * 2 + 3) / 7;
        value[4] = (value[0] * 4 + value[1] * 3 + 3) / 7;
        value[5] = (value[0] * 3 + value[1] * 4 + 3) / 7;
        value[6] = (value[0] * 2 + value[1] * 5 + 3) / 7;
        value[7] = (value[0] * 1 + value[1] * 6 + 3) / 7;
    } else {
        value[2] = (value[0] * 4 + value[1] * 1 + 2) / 5;
        value[3] = (value[0] * 3 + value[1] * 2 + 2) / 5;
        value[4] = (value[0] * 2 + value[1] * 3 + 2) / 5;
        value[5] = (value[0] * 1 + value[1] * 4 + 2) / 5;
        value[6] = 0;
        value[7] = 255;
    }
    is = GetU48(b + 2);
}
static inline void _DecompressBlockBC4(C Byte *b, SByte value[8], U64 &is) {
    value[0] = b[0];
    value[1] = b[1];
    if (value[0] > value[1]) {
        value[2] = (value[0] * 6 + value[1] * 1 + 3) / 7;
        value[3] = (value[0] * 5 + value[1] * 2 + 3) / 7;
        value[4] = (value[0] * 4 + value[1] * 3 + 3) / 7;
        value[5] = (value[0] * 3 + value[1] * 4 + 3) / 7;
        value[6] = (value[0] * 2 + value[1] * 5 + 3) / 7;
        value[7] = (value[0] * 1 + value[1] * 6 + 3) / 7;
    } else {
        value[2] = (value[0] * 4 + value[1] * 1 + 2) / 5;
        value[3] = (value[0] * 3 + value[1] * 2 + 2) / 5;
        value[4] = (value[0] * 2 + value[1] * 3 + 2) / 5;
        value[5] = (value[0] * 1 + value[1] * 4 + 2) / 5;
        value[6] = -127;
        value[7] = 127;
    }
    is = GetU48(b + 2);
}
/******************************************************************************/
static inline void _DecompressBlockBC3(C Byte *b, Color color[4], UInt &cis, Byte alpha[8], U64 &ais) {
    _DecompressBlockBC2(b, color, cis);
    _DecompressBlockBC4(b, alpha, ais);
}
void DecompressBlockBC3(C Byte *b, Color (&block)[4][4]) {
    Color color[4];
    UInt cis;
    Byte alpha[8];
    U64 ais;
    _DecompressBlockBC3(b, color, cis, alpha, ais);
    REPD(y, 4)
    REPD(x, 4) {
        Int i = x + (y << 2),            // pixel index
            ai = ((ais >> (3 * i)) & 7), // alpha index
            ci = ((cis >> (2 * i)) & 3); // color index
        Color &col = color[ci];
        block[y][x].set(col.r, col.g, col.b, alpha[ai]);
    }
}
void DecompressBlockBC3(C Byte *b, Color *dest, Int pitch) {
    Color color[4];
    UInt cis;
    Byte alpha[8];
    U64 ais;
    _DecompressBlockBC3(b, color, cis, alpha, ais);
    FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
    {
        REPD(x, 4) {
            Int i = x + (y << 2),            // pixel index
                ai = ((ais >> (3 * i)) & 7), // alpha index
                ci = ((cis >> (2 * i)) & 3); // color index
            Color &col = color[ci];
            dest[x].set(col.r, col.g, col.b, alpha[ai]);
        }
        dest = (Color *)((Byte *)dest + pitch);
    }
}
/******************************************************************************/
void DecompressBlockBC4(C Byte *b, Color (&block)[4][4]) {
    Byte red[8];
    U64 ris;
    _DecompressBlockBC4(b, red, ris);
    REPD(y, 4)
    REPD(x, 4) {
        Int i = x + (y << 2),            // pixel index
            ri = ((ris >> (3 * i)) & 7); // red   index
        block[y][x].set(red[ri], 0, 0, 255);
    }
}
void DecompressBlockBC4(C Byte *b, Color *dest, Int pitch) {
    Byte red[8];
    U64 ris;
    _DecompressBlockBC4(b, red, ris);
    FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
    {
        REPD(x, 4) {
            Int i = x + (y << 2),            // pixel index
                ri = ((ris >> (3 * i)) & 7); // red   index
            dest[x].set(red[ri], 0, 0, 255);
        }
        dest = (Color *)((Byte *)dest + pitch);
    }
}
/******************************************************************************/
void DecompressBlockBC4S(C Byte *b, SByte (&block)[4][4]) {
    SByte red[8];
    U64 ris;
    _DecompressBlockBC4(b, red, ris);
    REPD(y, 4)
    REPD(x, 4) {
        Int i = x + (y << 2),            // pixel index
            ri = ((ris >> (3 * i)) & 7); // red   index
        block[y][x] = red[ri];
    }
}
void DecompressBlockBC4S(C Byte *b, SByte *dest, Int pitch) {
    SByte red[8];
    U64 ris;
    _DecompressBlockBC4(b, red, ris);
    FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
    {
        REPD(x, 4) {
            Int i = x + (y << 2),            // pixel index
                ri = ((ris >> (3 * i)) & 7); // red   index
            dest[x] = red[ri];
        }
        dest = (SByte *)((Byte *)dest + pitch);
    }
}
/******************************************************************************/
void DecompressBlockBC5(C Byte *b, Color (&block)[4][4]) {
    Byte red[8];
    U64 ris;
    _DecompressBlockBC4(b, red, ris);
    Byte green[8];
    U64 gis;
    _DecompressBlockBC4(b + 8, green, gis);
    REPD(y, 4)
    REPD(x, 4) {
        Int i = x + (y << 2),            // pixel index
            ri = ((ris >> (3 * i)) & 7), // red   index
            gi = ((gis >> (3 * i)) & 7); // green index
        block[y][x].set(red[ri], green[gi], 0, 255);
    }
}
void DecompressBlockBC5(C Byte *b, Color *dest, Int pitch) {
    Byte red[8];
    U64 ris;
    _DecompressBlockBC4(b, red, ris);
    Byte green[8];
    U64 gis;
    _DecompressBlockBC4(b + 8, green, gis);
    FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
    {
        REPD(x, 4) {
            Int i = x + (y << 2),            // pixel index
                ri = ((ris >> (3 * i)) & 7), // red   index
                gi = ((gis >> (3 * i)) & 7); // green index
            dest[x].set(red[ri], green[gi], 0, 255);
        }
        dest = (Color *)((Byte *)dest + pitch);
    }
}
/******************************************************************************/
void DecompressBlockBC5S(C Byte *b, VecSB2 (&block)[4][4]) {
    SByte red[8];
    U64 ris;
    _DecompressBlockBC4(b, red, ris);
    SByte green[8];
    U64 gis;
    _DecompressBlockBC4(b + 8, green, gis);
    REPD(y, 4)
    REPD(x, 4) {
        Int i = x + (y << 2),            // pixel index
            ri = ((ris >> (3 * i)) & 7), // red   index
            gi = ((gis >> (3 * i)) & 7); // green index
        block[y][x].set(red[ri], green[gi]);
    }
}
void DecompressBlockBC5S(C Byte *b, VecSB2 *dest, Int pitch) {
    SByte red[8];
    U64 ris;
    _DecompressBlockBC4(b, red, ris);
    SByte green[8];
    U64 gis;
    _DecompressBlockBC4(b + 8, green, gis);
    FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
    {
        REPD(x, 4) {
            Int i = x + (y << 2),            // pixel index
                ri = ((ris >> (3 * i)) & 7), // red   index
                gi = ((gis >> (3 * i)) & 7); // green index
            dest[x].set(red[ri], green[gi]);
        }
        dest = (VecSB2 *)((Byte *)dest + pitch);
    }
}
/******************************************************************************/
void DecompressBlockBC6(C Byte *b, VecH (&block)[4][4]) {
#if BC6_DEC == BC_LIB_DIRECTX
    DirectX::D3DXDecodeBC6HU(block, b);
#elif BC6_DEC == BC_LIB_TEXGENPACK
    TGP::draw_block4x4_bptc_float(b, block);
#else
    Zero(block);
#endif
}
void DecompressBlockBC6(C Byte *b, VecH *dest, Int pitch) {
    VecH block[4][4];
    DecompressBlockBC6(b, block);
    FREPD(y, 4) {
        CopyFast(dest, block[y], SIZE(VecH) * 4);
        dest = (VecH *)((Byte *)dest + pitch);
    }
} // move in forward order so 'dest' can be increased by pitch
/******************************************************************************/
void DecompressBlockBC7(C Byte *b, Color (&block)[4][4]) {
#if BC7_DEC == BC_LIB_DIRECTX
    DirectX::D3DXDecodeBC7(block, b);
#elif BC7_DEC == BC_LIB_TEXGENPACK
    TGP::draw_block4x4_bptc(b, (UInt *)block);
#else
    Zero(block);
#endif
}
void DecompressBlockBC7(C Byte *b, Color *dest, Int pitch) {
    Color block[4][4];
#if BC7_DEC == BC_LIB_DIRECTX
    DirectX::D3DXDecodeBC7(block, b);
#elif BC7_DEC == BC_LIB_TEXGENPACK
    TGP::draw_block4x4_bptc(b, (UInt *)block);
#else
    Zero(block);
#endif
    FREPD(y, 4) // move in forward order so 'dest' can be increased by pitch
    {
        CopyFast(dest, block[y], SIZE(Color) * 4);
        dest = (Color *)((Byte *)dest + pitch);
    }
}
/******************************************************************************/
// PIXEL
/******************************************************************************/
Color DecompressPixelBC1(C Byte *b, Int x, Int y) {
    Int i = x + (y << 2),                        // pixel index
        ci = ((*(U32 *)(b + 4) >> (2 * i)) & 3); // color index

    // color
    Color color[4];
    UShort c0 = *(UShort *)(b),
           c1 = *(UShort *)(b + 2);
    Col565(color[0], c0);
    Col565(color[1], c1);

    color[ci].a = 255;
    if (c0 <= c1) {
        if (ci == 2) {
            color[2].r = (color[0].r + color[1].r + 1) / 2;
            color[2].g = (color[0].g + color[1].g + 1) / 2;
            color[2].b = (color[0].b + color[1].b + 1) / 2;
        } else if (ci == 3) {
            color[3].zero();
        }
    } else {
        if (ci == 2) {
            color[2].r = (color[0].r * 2 + color[1].r * 1 + 1) / 3;
            color[2].g = (color[0].g * 2 + color[1].g * 1 + 1) / 3;
            color[2].b = (color[0].b * 2 + color[1].b * 1 + 1) / 3;
        } else if (ci == 3) {
            color[3].r = (color[0].r * 1 + color[1].r * 2 + 1) / 3;
            color[3].g = (color[0].g * 1 + color[1].g * 2 + 1) / 3;
            color[3].b = (color[0].b * 1 + color[1].b * 2 + 1) / 3;
        }
    }
    return color[ci];
}
/******************************************************************************/
Color DecompressPixelBC2(C Byte *b, Int x, Int y) {
    Int i = x + (y << 2),                         // pixel index
        ci = ((*(U32 *)(b + 12) >> (2 * i)) & 3); // color index

    // color
    Color color[4];
    UShort c0 = *(UShort *)(b + 8),
           c1 = *(UShort *)(b + 10);
    Col565(color[0], c0);
    Col565(color[1], c1);
    if (ci == 2) {
        color[2].r = (color[0].r * 2 + color[1].r * 1 + 1) / 3;
        color[2].g = (color[0].g * 2 + color[1].g * 1 + 1) / 3;
        color[2].b = (color[0].b * 2 + color[1].b * 1 + 1) / 3;
    } else if (ci == 3) {
        color[3].r = (color[0].r * 1 + color[1].r * 2 + 1) / 3;
        color[3].g = (color[0].g * 1 + color[1].g * 2 + 1) / 3;
        color[3].b = (color[0].b * 1 + color[1].b * 2 + 1) / 3;
    }
    color[ci].a = U4ToByte((GetU64(b) >> (4 * i)) & 15);
    return color[ci];
}
/******************************************************************************/
Color DecompressPixelBC3(C Byte *b, Int x, Int y) {
    U64 ais = GetU48(b + 2);
    Int i = x + (y << 2),                         // pixel index
        ai = ((ais >> (3 * i)) & 7),              // alpha index
        ci = ((*(U32 *)(b + 12) >> (2 * i)) & 3); // color index

    // alpha
    Byte alpha[8];
    alpha[0] = b[0];
    alpha[1] = b[1];
    switch (ai) {
    case 2:
        alpha[2] = ((alpha[0] > alpha[1]) ? (alpha[0] * 6 + alpha[1] * 1 + 3) / 7 : (alpha[0] * 4 + alpha[1] * 1 + 2) / 5);
        break;
    case 3:
        alpha[3] = ((alpha[0] > alpha[1]) ? (alpha[0] * 5 + alpha[1] * 2 + 3) / 7 : (alpha[0] * 3 + alpha[1] * 2 + 2) / 5);
        break;
    case 4:
        alpha[4] = ((alpha[0] > alpha[1]) ? (alpha[0] * 4 + alpha[1] * 3 + 3) / 7 : (alpha[0] * 2 + alpha[1] * 3 + 2) / 5);
        break;
    case 5:
        alpha[5] = ((alpha[0] > alpha[1]) ? (alpha[0] * 3 + alpha[1] * 4 + 3) / 7 : (alpha[0] * 1 + alpha[1] * 4 + 2) / 5);
        break;
    case 6:
        alpha[6] = ((alpha[0] > alpha[1]) ? (alpha[0] * 2 + alpha[1] * 5 + 3) / 7 : 0);
        break;
    case 7:
        alpha[7] = ((alpha[0] > alpha[1]) ? (alpha[0] * 1 + alpha[1] * 6 + 3) / 7 : 255);
        break;
    }

    // color
    Color color[4];
    UShort c0 = *(UShort *)(b + 8),
           c1 = *(UShort *)(b + 10);
    Col565(color[0], c0);
    Col565(color[1], c1);
    if (ci == 2) {
        color[2].r = (color[0].r * 2 + color[1].r * 1 + 1) / 3;
        color[2].g = (color[0].g * 2 + color[1].g * 1 + 1) / 3;
        color[2].b = (color[0].b * 2 + color[1].b * 1 + 1) / 3;
    } else if (ci == 3) {
        color[3].r = (color[0].r * 1 + color[1].r * 2 + 1) / 3;
        color[3].g = (color[0].g * 1 + color[1].g * 2 + 1) / 3;
        color[3].b = (color[0].b * 1 + color[1].b * 2 + 1) / 3;
    }
    color[ci].a = alpha[ai];
    return color[ci];
}
/******************************************************************************/
Color DecompressPixelBC4(C Byte *b, Int x, Int y) {
    U64 ris = GetU48(b + 2);
    Int i = x + (y << 2),            // pixel index
        ri = ((ris >> (3 * i)) & 7); // red   index

    // red
    Byte red[8];
    red[0] = b[0];
    red[1] = b[1];
    switch (ri) {
    case 2:
        red[2] = ((red[0] > red[1]) ? (red[0] * 6 + red[1] * 1 + 3) / 7 : (red[0] * 4 + red[1] * 1 + 2) / 5);
        break;
    case 3:
        red[3] = ((red[0] > red[1]) ? (red[0] * 5 + red[1] * 2 + 3) / 7 : (red[0] * 3 + red[1] * 2 + 2) / 5);
        break;
    case 4:
        red[4] = ((red[0] > red[1]) ? (red[0] * 4 + red[1] * 3 + 3) / 7 : (red[0] * 2 + red[1] * 3 + 2) / 5);
        break;
    case 5:
        red[5] = ((red[0] > red[1]) ? (red[0] * 3 + red[1] * 4 + 3) / 7 : (red[0] * 1 + red[1] * 4 + 2) / 5);
        break;
    case 6:
        red[6] = ((red[0] > red[1]) ? (red[0] * 2 + red[1] * 5 + 3) / 7 : 0);
        break;
    case 7:
        red[7] = ((red[0] > red[1]) ? (red[0] * 1 + red[1] * 6 + 3) / 7 : 255);
        break;
    }

    return Color(red[ri], 0, 0, 255);
}
SByte DecompressPixelBC4S(C Byte *b, Int x, Int y) {
    U64 ris = GetU48(b + 2);
    Int i = x + (y << 2),            // pixel index
        ri = ((ris >> (3 * i)) & 7); // red   index

    // red
    SByte red[8];
    red[0] = b[0];
    red[1] = b[1];
    switch (ri) {
    case 2:
        red[2] = ((red[0] > red[1]) ? (red[0] * 6 + red[1] * 1 + 3) / 7 : (red[0] * 4 + red[1] * 1 + 2) / 5);
        break;
    case 3:
        red[3] = ((red[0] > red[1]) ? (red[0] * 5 + red[1] * 2 + 3) / 7 : (red[0] * 3 + red[1] * 2 + 2) / 5);
        break;
    case 4:
        red[4] = ((red[0] > red[1]) ? (red[0] * 4 + red[1] * 3 + 3) / 7 : (red[0] * 2 + red[1] * 3 + 2) / 5);
        break;
    case 5:
        red[5] = ((red[0] > red[1]) ? (red[0] * 3 + red[1] * 4 + 3) / 7 : (red[0] * 1 + red[1] * 4 + 2) / 5);
        break;
    case 6:
        red[6] = ((red[0] > red[1]) ? (red[0] * 2 + red[1] * 5 + 3) / 7 : -127);
        break;
    case 7:
        red[7] = ((red[0] > red[1]) ? (red[0] * 1 + red[1] * 6 + 3) / 7 : 127);
        break;
    }

    return red[ri];
}
Color DecompressPixelBC5(C Byte *b, Int x, Int y) {
    U64 ris = GetU48(b + 2), gis = GetU48(b + (8 + 2));
    Int i = x + (y << 2),            // pixel index
        ri = ((ris >> (3 * i)) & 7), // red   index
        gi = ((gis >> (3 * i)) & 7); // green index

    // red
    Byte red[8];
    red[0] = b[0];
    red[1] = b[1];
    switch (ri) {
    case 2:
        red[2] = ((red[0] > red[1]) ? (red[0] * 6 + red[1] * 1 + 3) / 7 : (red[0] * 4 + red[1] * 1 + 2) / 5);
        break;
    case 3:
        red[3] = ((red[0] > red[1]) ? (red[0] * 5 + red[1] * 2 + 3) / 7 : (red[0] * 3 + red[1] * 2 + 2) / 5);
        break;
    case 4:
        red[4] = ((red[0] > red[1]) ? (red[0] * 4 + red[1] * 3 + 3) / 7 : (red[0] * 2 + red[1] * 3 + 2) / 5);
        break;
    case 5:
        red[5] = ((red[0] > red[1]) ? (red[0] * 3 + red[1] * 4 + 3) / 7 : (red[0] * 1 + red[1] * 4 + 2) / 5);
        break;
    case 6:
        red[6] = ((red[0] > red[1]) ? (red[0] * 2 + red[1] * 5 + 3) / 7 : 0);
        break;
    case 7:
        red[7] = ((red[0] > red[1]) ? (red[0] * 1 + red[1] * 6 + 3) / 7 : 255);
        break;
    }

    // green
    Byte green[8];
    green[0] = b[8 + 0];
    green[1] = b[8 + 1];
    switch (gi) {
    case 2:
        green[2] = ((green[0] > green[1]) ? (green[0] * 6 + green[1] * 1 + 3) / 7 : (green[0] * 4 + green[1] * 1 + 2) / 5);
        break;
    case 3:
        green[3] = ((green[0] > green[1]) ? (green[0] * 5 + green[1] * 2 + 3) / 7 : (green[0] * 3 + green[1] * 2 + 2) / 5);
        break;
    case 4:
        green[4] = ((green[0] > green[1]) ? (green[0] * 4 + green[1] * 3 + 3) / 7 : (green[0] * 2 + green[1] * 3 + 2) / 5);
        break;
    case 5:
        green[5] = ((green[0] > green[1]) ? (green[0] * 3 + green[1] * 4 + 3) / 7 : (green[0] * 1 + green[1] * 4 + 2) / 5);
        break;
    case 6:
        green[6] = ((green[0] > green[1]) ? (green[0] * 2 + green[1] * 5 + 3) / 7 : 0);
        break;
    case 7:
        green[7] = ((green[0] > green[1]) ? (green[0] * 1 + green[1] * 6 + 3) / 7 : 255);
        break;
    }

    return Color(red[ri], green[gi], 0, 255);
}
VecSB2 DecompressPixelBC5S(C Byte *b, Int x, Int y) {
    U64 ris = GetU48(b + 2), gis = GetU48(b + (8 + 2));
    Int i = x + (y << 2),            // pixel index
        ri = ((ris >> (3 * i)) & 7), // red   index
        gi = ((gis >> (3 * i)) & 7); // green index

    // red
    SByte red[8];
    red[0] = b[0];
    red[1] = b[1];
    switch (ri) {
    case 2:
        red[2] = ((red[0] > red[1]) ? (red[0] * 6 + red[1] * 1 + 3) / 7 : (red[0] * 4 + red[1] * 1 + 2) / 5);
        break;
    case 3:
        red[3] = ((red[0] > red[1]) ? (red[0] * 5 + red[1] * 2 + 3) / 7 : (red[0] * 3 + red[1] * 2 + 2) / 5);
        break;
    case 4:
        red[4] = ((red[0] > red[1]) ? (red[0] * 4 + red[1] * 3 + 3) / 7 : (red[0] * 2 + red[1] * 3 + 2) / 5);
        break;
    case 5:
        red[5] = ((red[0] > red[1]) ? (red[0] * 3 + red[1] * 4 + 3) / 7 : (red[0] * 1 + red[1] * 4 + 2) / 5);
        break;
    case 6:
        red[6] = ((red[0] > red[1]) ? (red[0] * 2 + red[1] * 5 + 3) / 7 : -127);
        break;
    case 7:
        red[7] = ((red[0] > red[1]) ? (red[0] * 1 + red[1] * 6 + 3) / 7 : 127);
        break;
    }

    // green
    SByte green[8];
    green[0] = b[8 + 0];
    green[1] = b[8 + 1];
    switch (gi) {
    case 2:
        green[2] = ((green[0] > green[1]) ? (green[0] * 6 + green[1] * 1 + 3) / 7 : (green[0] * 4 + green[1] * 1 + 2) / 5);
        break;
    case 3:
        green[3] = ((green[0] > green[1]) ? (green[0] * 5 + green[1] * 2 + 3) / 7 : (green[0] * 3 + green[1] * 2 + 2) / 5);
        break;
    case 4:
        green[4] = ((green[0] > green[1]) ? (green[0] * 4 + green[1] * 3 + 3) / 7 : (green[0] * 2 + green[1] * 3 + 2) / 5);
        break;
    case 5:
        green[5] = ((green[0] > green[1]) ? (green[0] * 3 + green[1] * 4 + 3) / 7 : (green[0] * 1 + green[1] * 4 + 2) / 5);
        break;
    case 6:
        green[6] = ((green[0] > green[1]) ? (green[0] * 2 + green[1] * 5 + 3) / 7 : -127);
        break;
    case 7:
        green[7] = ((green[0] > green[1]) ? (green[0] * 1 + green[1] * 6 + 3) / 7 : 127);
        break;
    }

    return VecSB2(red[ri], green[gi]);
}
/******************************************************************************/
VecH DecompressPixelBC6(C Byte *b, Int x, Int y) {
    VecH block[4][4];
    DecompressBlockBC6(b, block);
    return block[y][x];
}
Color DecompressPixelBC7(C Byte *b, Int x, Int y) {
    Color block[4][4];
    DecompressBlockBC7(b, block);
    return block[y][x];
}
/******************************************************************************/
// COMPRESS
/******************************************************************************

   Based on Microsoft DirectXTex - https://github.com/Microsoft/DirectXTex/blob/master/DirectXTex/BC.cpp

/******************************************************************************/
struct BC1 {
    U16 rgb[2]; // 565 colors
    U32 bitmap; // 2bpp rgb bitmap
};
struct BC2 {
    UInt alpha[2]; // 4bpp alpha bitmap
    BC1 rgb;
};
struct BC4 {
    Byte value[2];  // values
    Byte bitmap[6]; // 3bpp value bitmap
};
struct BC3 {
    BC4 alpha;
    BC1 rgb;
};
struct BC4S {
    SByte value[2]; // values
    Byte bitmap[6]; // 3bpp value bitmap
};
struct BC5 {
    BC4 red, green;
};
struct BC5S {
    BC4S red, green;
};
static const Flt fEpsilon = Sqr(0.25f / 64);
static const Flt pC3[] = {2.0f / 2.0f, 1.0f / 2.0f, 0.0f / 2.0f};
static const Flt pD3[] = {0.0f / 2.0f, 1.0f / 2.0f, 2.0f / 2.0f};
static const Flt pC4[] = {3.0f / 3.0f, 2.0f / 3.0f, 1.0f / 3.0f, 0.0f / 3.0f};
static const Flt pD4[] = {0.0f / 3.0f, 1.0f / 3.0f, 2.0f / 3.0f, 3.0f / 3.0f};
static const Flt pC6[] = {5.0f / 5.0f, 4.0f / 5.0f, 3.0f / 5.0f, 2.0f / 5.0f, 1.0f / 5.0f, 0.0f / 5.0f};
static const Flt pD6[] = {0.0f / 5.0f, 1.0f / 5.0f, 2.0f / 5.0f, 3.0f / 5.0f, 4.0f / 5.0f, 5.0f / 5.0f};
static const Flt pC8[] = {7.0f / 7.0f, 6.0f / 7.0f, 5.0f / 7.0f, 4.0f / 7.0f, 3.0f / 7.0f, 2.0f / 7.0f, 1.0f / 7.0f, 0.0f / 7.0f};
static const Flt pD8[] = {0.0f / 7.0f, 1.0f / 7.0f, 2.0f / 7.0f, 3.0f / 7.0f, 4.0f / 7.0f, 5.0f / 7.0f, 6.0f / 7.0f, 7.0f / 7.0f};
static const Int pSteps3[] = {0, 2, 1};
static const Int pSteps4[] = {0, 2, 3, 1};
static const Int pSteps6[] = {0, 2, 3, 4, 5, 1};
static const Int pSteps8[] = {0, 2, 3, 4, 5, 6, 7, 1};

static void Decode565(Vec &col, UInt w565) {
    col.set(((w565 >> 11) & 31) / 31.0f,
            ((w565 >> 5) & 63) / 63.0f,
            ((w565 >> 0) & 31) / 31.0f);
}
static UInt Encode565(C Vec &col) {
    return ((col.x <= 0) ? 0 : (col.x >= 1) ? (31 << 11)
                                            : (RoundPos(col.x * 31) << 11)) |
           ((col.y <= 0) ? 0 : (col.y >= 1) ? (63 << 5)
                                            : (RoundPos(col.y * 63) << 5)) |
           ((col.z <= 0) ? 0 : (col.z >= 1) ? (31 << 0)
                                            : (RoundPos(col.z * 31) << 0));
}
static void OptimizeRGB(Vec &color_a, Vec &color_b, Vec4 (&color)[16], Int steps) {
    C Flt *pC, *pD;
    if (steps == 3) {
        pC = pC3;
        pD = pD3;
    } else {
        pC = pC4;
        pD = pD4;
    }

    Vec min = color[15].xyz, max = min;
    REP(15) {
        C Vec &c = color[i].xyz;
        MIN(min.x, c.x);
        MAX(max.x, c.x);
        MIN(min.y, c.y);
        MAX(max.y, c.y);
        MIN(min.z, c.z);
        MAX(max.z, c.z);
    }
    Vec dir = max - min;
    Flt l = dir.length2();
    if (l <= FLT_MIN) {
        color_a = min;
        color_b = max;
        return;
    } // Single color block.. no need to root-find
    dir /= l;
    Vec mid = Avg(min, max);
    Flt fDir[4] = {0, 0, 0, 0};
    FREPA(color) {
        Vec c = color[i].xyz - mid;
        c *= dir;
        fDir[0] += Sqr(c.x + c.y + c.z);
        fDir[1] += Sqr(c.x + c.y - c.z);
        fDir[2] += Sqr(c.x - c.y + c.z);
        fDir[3] += Sqr(c.x - c.y - c.z);
    }
    Int iDirMax = 0;
    Flt fDirMax = fDir[0];
    for (Int iDir = 1; iDir < 4; iDir++)
        if (fDir[iDir] > fDirMax)
            fDirMax = fDir[iDirMax = iDir];
    if (iDirMax & 2)
        Swap(min.y, max.y);
    if (iDirMax & 1)
        Swap(min.z, max.z);

    // Two color block.. no need to root-find
    if (l < 1.0f / 4096) {
        color_a = min;
        color_b = max;
        return;
    }

    // Use Newton's Method to find local minima of sum-of-squares error
    Flt fSteps = steps - 1;
    REP(16) // original version had 8 iterations, but 16 generates better results
    {
        // Calculate new steps
        Vec pSteps[4];
        FREP(steps)
        pSteps[i] = min * pC[i] + max * pD[i];

        // Calculate color direction
        dir = max - min;
        l = dir.length2();
        if (l < 1.0f / 4096)
            break;
        dir *= fSteps / l; // divide by 'l' and not 'Sqrt(l)' because we don't want to use it to calculate unit distances, but fraction distances

        // Evaluate function, and derivatives
        Flt start = Dot(min, dir), d2X = 0, d2Y = 0;
        Vec dX = 0, dY = 0;

        FREPA(color) {
            Flt dot = Dot(color[i].xyz, dir) - start;

            Int s;
            if (dot <= 0)
                s = 0;
            else if (dot >= fSteps)
                s = steps - 1;
            else
                s = RoundPos(dot);

            Vec diff = pSteps[s] - color[i].xyz;
            Flt fC = pC[s] / 8, fD = pD[s] / 8; // this should be 8 and not number of iterations (this was tested with 64 iters and /64 and results were worse)

            d2X += fC * pC[s];
            dX += fC * diff;

            d2Y += fD * pD[s];
            dY += fD * diff;
        }

        // Move endpoints
        if (d2X > 0)
            min -= dX / d2X;
        if (d2Y > 0)
            max -= dY / d2Y;

#if 0 // original
      if(Sqr(dX.x)<fEpsilon && Sqr(dX.y)<fEpsilon && Sqr(dX.z)<fEpsilon
      && Sqr(dY.x)<fEpsilon && Sqr(dY.y)<fEpsilon && Sqr(dY.z)<fEpsilon)break;
#else
        if (dX.length2() < fEpsilon && dY.length2() < fEpsilon)
            break;
#endif
    }

    color_a = min;
    color_b = max;
}
static void _CompressBC1(BC1 &bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a, Flt alpha_ref = 0.5f) // !! Warning: 'color' gets modified !! 'alpha_ref'=use zero to disable alpha encoding
{
    Int steps;
    if (alpha_ref > 0) {
        if (dither_a) {
            Flt error[16];
            Zero(error);
            FREPA(color) {
                Flt &dest = color[i].w, alpha = dest + error[i];
                dest = RoundPos(alpha);
                Flt diff = alpha - dest;

                if ((i & 3) != 3)
                    error[i + 1] += diff * (7.0f / 16);
                if (i < 12) {
                    if (i & 3)
                        error[i + 3] += diff * (3.0f / 16);
                    error[i + 4] += diff * (5.0f / 16);
                    if ((i & 3) != 3)
                        error[i + 5] += diff * (1.0f / 16);
                }
            }
        }

        Int transparent = 0;
        REPA(color)
        if (color[i].w < alpha_ref) transparent++;
        if (transparent == 16) {
            bc.rgb[0] = 0x0000;
            bc.rgb[1] = 0xFFFF;
            bc.bitmap = 0xFFFFFFFF;
            return;
        }
        steps = (transparent ? 3 : 4);
    } else
        steps = 4;

    Vec4 col[16];
    Vec error[16];
    if (dither_rgb)
        Zero(error);

    FREPA(col) {
        Vec clr = color[i].xyz;
        if (dither_rgb)
            clr += error[i];

        // align to actual value (needed for smooth gradients with dithering)
        col[i].x = RoundPos(clr.x * 31) / 31.0f;
        col[i].y = RoundPos(clr.y * 63) / 63.0f;
        col[i].z = RoundPos(clr.z * 31) / 31.0f;

        if (dither_rgb) {
            Vec diff = clr - col[i].xyz;

            if ((i & 3) != 3)
                error[i + 1] += diff * (7.0f / 16);
            if (i < 12) {
                if (i & 3)
                    error[i + 3] += diff * (3.0f / 16);
                error[i + 4] += diff * (5.0f / 16);
                if ((i & 3) != 3)
                    error[i + 5] += diff * (1.0f / 16);
            }
        }

        if (weight)
            col[i].xyz *= *weight;
    }

    Vec color_a, color_b;
    OptimizeRGB(color_a, color_b, col, steps);

    if (weight) {
        color_a /= *weight;
        color_b /= *weight;
    }
    UInt wColorA = Encode565(color_a), wColorB = Encode565(color_b);
    if (steps == 4 && wColorA == wColorB) {
        bc.rgb[0] = wColorA;
        bc.rgb[1] = wColorB;
        bc.bitmap = 0x00000000;
        return;
    }
    Decode565(color_a, wColorA);
    Decode565(color_b, wColorB);
    if (weight) {
        color_a *= *weight;
        color_b *= *weight;
    }

    // Calculate color steps
    Vec step[4];
    if ((steps == 3) == (wColorA <= wColorB)) {
        bc.rgb[0] = wColorA;
        bc.rgb[1] = wColorB;
        step[0] = color_a;
        step[1] = color_b;
    } else {
        bc.rgb[0] = wColorB;
        bc.rgb[1] = wColorA;
        step[0] = color_b;
        step[1] = color_a;
    }

    const Int *pSteps;
    if (steps == 3) {
        pSteps = pSteps3;
        step[2] = Avg(step[0], step[1]);
    } else {
        pSteps = pSteps4;
#if 0
      step[2]=Lerp(step[0], step[1], 1.0f/3);
      step[3]=Lerp(step[0], step[1], 2.0f/3);
#else
        Vec d = (step[1] - step[0]) / 3;
        step[2] = step[0] + d;
        step[3] = step[0] + d * 2;
#endif
    }

    // Calculate color direction
    Flt fSteps = steps - 1;
    Vec dir = step[1] - step[0];
    if (wColorA != wColorB)
        dir *= fSteps / dir.length2(); // divide by 'length2' and not 'length' because we don't want to use it to calculate unit distances, but fraction distances
    Flt start = Dot(step[0], dir);

    // Encode colors
    UInt bitmap = 0;
    if (dither_rgb)
        Zero(error);

    FREPA(color) {
        if (steps == 3 && color[i].w < alpha_ref)
            bitmap = (3 << 30) | (bitmap >> 2);
        else {
            Vec clr = color[i].xyz;
            if (weight)
                clr *= *weight;
            if (dither_rgb)
                clr += error[i];
            Flt dot = Dot(clr, dir) - start;

            Int s;
            if (dot <= 0.0f)
                s = 0;
            else if (dot >= fSteps)
                s = 1;
            else
                s = pSteps[RoundPos(dot)];

            bitmap = (s << 30) | (bitmap >> 2);

            if (dither_rgb) {
                Vec diff = clr - step[s];

                if ((i & 3) != 3)
                    error[i + 1] += diff * (7.0f / 16);
                if (i < 12) {
                    if (i & 3)
                        error[i + 3] += diff * (3.0f / 16);
                    error[i + 4] += diff * (5.0f / 16);
                    if ((i & 3) != 3)
                        error[i + 5] += diff * (1.0f / 16);
                }
            }
        }
    }
    bc.bitmap = bitmap;
}
static inline void _CompressBC1RGB(BC1 &bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb) { _CompressBC1(bc, color, weight, dither_rgb, false, 0); }

static void CompressBC1(Byte *bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a) { _CompressBC1RGB(*(BC1 *)bc, color, weight, dither_rgb); } // #BC1RGB no alpha
static void CompressBC2(Byte *bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a) {
    BC2 &bc2 = *(BC2 *)bc;
    _CompressBC1RGB(bc2.rgb, color, weight, dither_rgb);

    bc2.alpha[0] = 0;
    bc2.alpha[1] = 0;
    Flt error[16];
    if (dither_a)
        Zero(error);
    FREPA(color) {
        Flt alpha = color[i].w;
        if (dither_a)
            alpha += error[i];
        UInt u = RoundPos(alpha * 15);
        bc2.alpha[i >> 3] >>= 4;
        bc2.alpha[i >> 3] |= (u << 28);
        if (dither_a) {
            Flt diff = alpha - u / 15.0f;
            if ((i & 3) != 3)
                error[i + 1] += diff * (7.0f / 16);
            if (i < 12) {
                if (i & 3)
                    error[i + 3] += diff * (3.0f / 16);
                error[i + 4] += diff * (5.0f / 16);
                if ((i & 3) != 3)
                    error[i + 5] += diff * (1.0f / 16);
            }
        }
    }
}
static void OptimizeAlpha(Flt *pX, Flt *pY, const Flt *pPoints, Int steps, Bool sign) {
    const Flt *pC = (6 == steps) ? pC6 : pC8;
    const Flt *pD = (6 == steps) ? pD6 : pD8;

    const Flt MIN_VALUE = (sign ? -1 : 0);
    const Flt MAX_VALUE = 1;

    // Find Min and Max points, as starting point
    Flt fX = MAX_VALUE;
    Flt fY = MIN_VALUE;
    if (steps == 8) {
        FREP(16) {
            MIN(fX, pPoints[i]);
            MAX(fY, pPoints[i]);
        }
    } else {
        FREP(16) {
            if (pPoints[i] < fX && pPoints[i] > MIN_VALUE)
                fX = pPoints[i];
            if (pPoints[i] > fY && pPoints[i] < MAX_VALUE)
                fY = pPoints[i];
        }
        if (fX == fY)
            fY = MAX_VALUE;
    }

    // Use Newton's Method to find local minima of sum-of-squares error
    Flt fSteps = steps - 1;
    for (Int iIteration = 0; iIteration < 8; iIteration++) {
        if (fY - fX < 1.0f / 256)
            break;
        Flt fScale = fSteps / (fY - fX);

        // Calculate new steps
        Flt pSteps[8], low, high;
        for (Int iStep = 0; iStep < steps; iStep++)
            pSteps[iStep] = pC[iStep] * fX + pD[iStep] * fY;
        if (steps == 6) {
            pSteps[6] = MIN_VALUE;
            pSteps[7] = MAX_VALUE;

            low = Avg(fX, MIN_VALUE);
            high = Avg(fY, MAX_VALUE);
        } else { // never pass
            low = -FLT_MAX;
            high = FLT_MAX;
        }

        // Evaluate function, and derivatives
        Flt dX = 0, dY = 0, d2X = 0, d2Y = 0;
        FREP(16) {
            Flt value = pPoints[i],
                fDot = (value - fX) * fScale;
            Int step;
            if (fDot <= 0)
                step = (value <= low) ? 6 : 0;
            else if (fDot >= fSteps)
                step = (value >= high) ? 7 : steps - 1;
            else // do "steps-1" instead of "1" because this index is for the 'pSteps' array which returns "1" index for last element
                step = RoundPos(fDot);
            if (step < steps) {
                Flt diff = pSteps[step] - value;

                dX += pC[step] * diff;
                d2X += Sqr(pC[step]);

                dY += pD[step] * diff;
                d2Y += Sqr(pD[step]);
            }
        }

        // Move endpoints
        if (d2X > 0)
            fX -= dX / d2X;
        if (d2Y > 0)
            fY -= dY / d2Y;

        if (fX > fY)
            Swap(fX, fY);

        if (Sqr(dX) < 1.0f / 64 && Sqr(dY) < 1.0f / 64)
            break;
    }

    *pX = Mid(fX, MIN_VALUE, MAX_VALUE);
    *pY = Mid(fY, MIN_VALUE, MAX_VALUE);
}
static void _CompressBC4(BC4 &bc, Vec4 (&color)[16], Bool dither) {
    dither = false; // disable dithering for BC4 because it actually looks worse
    Flt fAlpha[16], error[16], fMinAlpha = color[0].x, fMaxAlpha = color[0].x;
    if (dither)
        Zero(error);
    FREPA(color) {
        Flt fAlph = color[i].x;
        if (dither)
            fAlph += error[i];
        fAlpha[i] = RoundPos(fAlph * 255) / 255.0f;
        if (fAlpha[i] < fMinAlpha)
            fMinAlpha = fAlpha[i];
        else if (fAlpha[i] > fMaxAlpha)
            fMaxAlpha = fAlpha[i];

        if (dither) {
            Flt diff = fAlph - fAlpha[i];
            if ((i & 3) != 3)
                error[i + 1] += diff * (7.0f / 16);
            if (i < 12) {
                if (i & 3)
                    error[i + 3] += diff * (3.0f / 16);
                error[i + 4] += diff * (5.0f / 16);
                if ((i & 3) != 3)
                    error[i + 5] += diff * (1.0f / 16);
            }
        }
    }
    if (fMinAlpha >= 1) {
        bc.value[0] = 0xFF;
        bc.value[1] = 0xFF;
        Zero(bc.bitmap);
        return;
    }

    // Optimize and Quantize Min and Max values
    Int steps = (fMinAlpha <= 0 || fMaxAlpha >= 1) ? 6 : 8;
    Flt fAlphaA, fAlphaB;
    OptimizeAlpha(&fAlphaA, &fAlphaB, fAlpha, steps, false);
    Byte bAlphaA = RoundPos(fAlphaA * 255);
    fAlphaA = bAlphaA / 255.0f;
    Byte bAlphaB = RoundPos(fAlphaB * 255);
    fAlphaB = bAlphaB / 255.0f;

    // Setup block
    if (steps == 8 && bAlphaA == bAlphaB) {
        bc.value[0] = bAlphaA;
        bc.value[1] = bAlphaB;
        Zero(bc.bitmap);
        return;
    }

    const Int *pSteps;
    Flt fStep[8], low, high;
    if (steps == 6) {
        bc.value[0] = bAlphaA;
        bc.value[1] = bAlphaB;

        fStep[0] = fAlphaA;
        fStep[1] = fAlphaB;

        for (Int i = 1; i < 5; i++)
            fStep[i + 1] = (fStep[0] * (5 - i) + fStep[1] * i) / 5;

        fStep[6] = 0;
        fStep[7] = 1;

        pSteps = pSteps6;

        low = Avg(fStep[0], 0);
        high = Avg(fStep[1], 1);
    } else {
        bc.value[0] = bAlphaB;
        bc.value[1] = bAlphaA;

        fStep[0] = fAlphaB;
        fStep[1] = fAlphaA;

        for (Int i = 1; i < 7; i++)
            fStep[i + 1] = (fStep[0] * (7 - i) + fStep[1] * i) / 7;

        pSteps = pSteps8;

        // never pass
        low = -FLT_MAX;
        high = FLT_MAX;
    }

    // Encode alpha bitmap
    Flt fSteps = steps - 1, fScale = (fStep[0] != fStep[1]) ? (fSteps / (fStep[1] - fStep[0])) : 0;
    if (dither)
        Zero(error);
    for (Int iSet = 0; iSet < 2; iSet++) {
        UInt dw = 0;
        Int iMin = iSet * 8;
        Int iMax = iMin + 8;
        for (Int i = iMin; i < iMax; i++) {
            Flt fAlph = color[i].x;
            if (dither)
                fAlph += error[i];
            Flt fDot = (fAlph - fStep[0]) * fScale;

            Int iStep;
            if (fDot <= 0)
                iStep = (fAlph <= low) ? 6 : 0;
            else if (fDot >= fSteps)
                iStep = (fAlph >= high) ? 7 : 1;
            else
                iStep = pSteps[RoundPos(fDot)];

            dw = (iStep << 21) | (dw >> 3);

            if (dither) {
                Flt diff = fAlph - fStep[iStep];
                if ((i & 3) != 3)
                    error[i + 1] += diff * (7.0f / 16);
                if (i < 12) {
                    if (i & 3)
                        error[i + 3] += diff * (3.0f / 16);
                    error[i + 4] += diff * (5.0f / 16);
                    if ((i & 3) != 3)
                        error[i + 5] += diff * (1.0f / 16);
                }
            }
        }

        bc.bitmap[0 + iSet * 3] = ((Byte *)&dw)[0];
        bc.bitmap[1 + iSet * 3] = ((Byte *)&dw)[1];
        bc.bitmap[2 + iSet * 3] = ((Byte *)&dw)[2];
    }
}
static void _CompressBC4(BC4S &bc, Vec4 (&color)[16], Bool dither) {
    dither = false; // disable dithering for BC4 because it actually looks worse
    Flt fAlpha[16], error[16], fMinAlpha = color[0].x, fMaxAlpha = color[0].x;
    if (dither)
        Zero(error);
    FREPA(color) {
        Flt fAlph = color[i].x;
        if (dither)
            fAlph += error[i];
        fAlpha[i] = SByteToSFlt(SFltToSByte(fAlph));
        if (fAlpha[i] < fMinAlpha)
            fMinAlpha = fAlpha[i];
        else if (fAlpha[i] > fMaxAlpha)
            fMaxAlpha = fAlpha[i];

        if (dither) {
            Flt diff = fAlph - fAlpha[i];
            if ((i & 3) != 3)
                error[i + 1] += diff * (7.0f / 16);
            if (i < 12) {
                if (i & 3)
                    error[i + 3] += diff * (3.0f / 16);
                error[i + 4] += diff * (5.0f / 16);
                if ((i & 3) != 3)
                    error[i + 5] += diff * (1.0f / 16);
            }
        }
    }
    if (fMinAlpha >= 1) {
        bc.value[0] = 127;
        bc.value[1] = 127;
        Zero(bc.bitmap);
        return;
    }

    // Optimize and Quantize Min and Max values
    Int steps = (fMinAlpha <= -1 || fMaxAlpha >= 1) ? 6 : 8;
    Flt fAlphaA, fAlphaB;
    OptimizeAlpha(&fAlphaA, &fAlphaB, fAlpha, steps, true);
    SByte bAlphaA = SFltToSByte(fAlphaA);
    fAlphaA = SByteToSFlt(bAlphaA);
    SByte bAlphaB = SFltToSByte(fAlphaB);
    fAlphaB = SByteToSFlt(bAlphaB);

    // Setup block
    if (steps == 8 && bAlphaA == bAlphaB) {
        bc.value[0] = bAlphaA;
        bc.value[1] = bAlphaB;
        Zero(bc.bitmap);
        return;
    }

    const Int *pSteps;
    Flt fStep[8], low, high;
    if (steps == 6) {
        bc.value[0] = bAlphaA;
        bc.value[1] = bAlphaB;

        fStep[0] = fAlphaA;
        fStep[1] = fAlphaB;

        for (Int i = 1; i < 5; i++)
            fStep[i + 1] = (fStep[0] * (5 - i) + fStep[1] * i) / 5;

        fStep[6] = -1;
        fStep[7] = 1;

        pSteps = pSteps6;

        low = Avg(fStep[0], -1);
        high = Avg(fStep[1], 1);
    } else {
        bc.value[0] = bAlphaB;
        bc.value[1] = bAlphaA;

        fStep[0] = fAlphaB;
        fStep[1] = fAlphaA;

        for (Int i = 1; i < 7; i++)
            fStep[i + 1] = (fStep[0] * (7 - i) + fStep[1] * i) / 7;

        pSteps = pSteps8;

        // never pass
        low = -FLT_MAX;
        high = FLT_MAX;
    }

    // Encode alpha bitmap
    Flt fSteps = steps - 1, fScale = (fStep[0] != fStep[1]) ? (fSteps / (fStep[1] - fStep[0])) : 0;
    if (dither)
        Zero(error);
    for (Int iSet = 0; iSet < 2; iSet++) {
        UInt dw = 0;
        Int iMin = iSet * 8;
        Int iMax = iMin + 8;
        for (Int i = iMin; i < iMax; i++) {
            Flt fAlph = color[i].x;
            if (dither)
                fAlph += error[i];
            Flt fDot = (fAlph - fStep[0]) * fScale;

            Int iStep;
            if (fDot <= 0)
                iStep = (fAlph <= low) ? 6 : 0;
            else if (fDot >= fSteps)
                iStep = (fAlph >= high) ? 7 : 1;
            else
                iStep = pSteps[RoundPos(fDot)];

            dw = (iStep << 21) | (dw >> 3);

            if (dither) {
                Flt diff = fAlph - fStep[iStep];
                if ((i & 3) != 3)
                    error[i + 1] += diff * (7.0f / 16);
                if (i < 12) {
                    if (i & 3)
                        error[i + 3] += diff * (3.0f / 16);
                    error[i + 4] += diff * (5.0f / 16);
                    if ((i & 3) != 3)
                        error[i + 5] += diff * (1.0f / 16);
                }
            }
        }

        bc.bitmap[0 + iSet * 3] = ((Byte *)&dw)[0];
        bc.bitmap[1 + iSet * 3] = ((Byte *)&dw)[1];
        bc.bitmap[2 + iSet * 3] = ((Byte *)&dw)[2];
    }
}
static void CompressBC3(Byte *bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a) {
    BC3 &bc3 = *(BC3 *)bc;
    _CompressBC1RGB(bc3.rgb, color, weight, dither_rgb);
    _CompressBC4(bc3.alpha, (Vec4(&)[16])color[0].w, dither_a);
}
static void CompressBC4(Byte *bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a) {
#if 1
    BC4 &bc4 = *(BC4 *)bc;
    _CompressBC4(bc4, color, dither_rgb);
#else
    DirectX::D3DXEncodeBC4U(bc, (DirectX::XMVECTOR *)color, 0);
#endif
}
static void CompressBC4S(Byte *bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a) {
#if 1
    BC4S &bc4 = *(BC4S *)bc;
    _CompressBC4(bc4, color, dither_rgb);
#else
    DirectX::D3DXEncodeBC4S(bc, (DirectX::XMVECTOR *)color, 0);
#endif
}
static void CompressBC5(Byte *bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a) {
#if 1
    BC5 &bc5 = *(BC5 *)bc;
    _CompressBC4(bc5.red, color, dither_rgb);
    _CompressBC4(bc5.green, (Vec4(&)[16])color[0].y, dither_rgb);
#else
    DirectX::D3DXEncodeBC5U(bc, (DirectX::XMVECTOR *)color, 0);
#endif
}
static void CompressBC5S(Byte *bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a) {
#if 1
    BC5S &bc5 = *(BC5S *)bc;
    _CompressBC4(bc5.red, color, dither_rgb);
    _CompressBC4(bc5.green, (Vec4(&)[16])color[0].y, dither_rgb);
#else
    DirectX::D3DXEncodeBC5S(bc, (DirectX::XMVECTOR *)color, 0);
#endif
}
/******************************************************************************/
static const Vec BCWeights = ColorLumWeight * 0.65f;
struct BCContext {
    Bool perceptual;
    Int x_mul, x_blocks, y_blocks, sz;
    Byte *dest_data_z;
    void (*compress_block)(Byte *bc, Vec4 (&color)[16], C Vec *weight, Bool dither_rgb, Bool dither_a);
    C Image &src;
    Image &dest;

    BCContext(C Image &src, Image &dest) : src(src), dest(dest) {}

    static void Process(IntPtr elm_index, BCContext &ctx, Int thread_index) { ctx.process(elm_index); }
    void process(Int by) {
        Int py = by * 4, yo[4];
        REPAO(yo) = Min(py + i, src.lh() - 1); // use clamping to avoid black borders
        Byte *dest_data = dest_data_z + by * dest.pitch();
        Vec4 rgba[16];
        REPD(bx, x_blocks) {
            Int px = bx * 4, xo[4];
            REPAO(xo) = Min(px + i, src.lw() - 1); // use clamping to avoid black borders
            src.gather(rgba, xo, Elms(xo), yo, Elms(yo), &sz, 1);
            if (perceptual) {
                Vec4 min, max;
                MinMax(rgba, 4 * 4, min, max);
#if 1                                                                 // this gave better results
                Vec weight = BCWeights + max.xyz + max.xyz - min.xyz; // max + delta = max + (max-min)
#else
                Vec weight = LinearToSRGB(ColorLumWeight2);
#endif
                compress_block(dest_data + bx * x_mul, rgba, &weight, true, true);
            } else
                compress_block(dest_data + bx * x_mul, rgba, null, true, true);
        }
    }

    Bool process() {
        ImageThreads.init();

        compress_block = ((dest.hwType() == IMAGE_BC1 || dest.hwType() == IMAGE_BC1_SRGB) ? CompressBC1 : (dest.hwType() == IMAGE_BC2 || dest.hwType() == IMAGE_BC2_SRGB) ? CompressBC2
                                                                                                      : (dest.hwType() == IMAGE_BC3 || dest.hwType() == IMAGE_BC3_SRGB)   ? CompressBC3
                                                                                                      : (dest.hwType() == IMAGE_BC4)                                      ? CompressBC4
                                                                                                      : (dest.hwType() == IMAGE_BC4_SIGN)                                 ? CompressBC4S
                                                                                                      : (dest.hwType() == IMAGE_BC5)                                      ? CompressBC5
                                                                                                      : (dest.hwType() == IMAGE_BC5_SIGN)                                 ? CompressBC5S
                                                                                                                                                                          : null);
        perceptual = dest.sRGB();
        x_mul = ((dest.hwType() == IMAGE_BC1 || dest.hwType() == IMAGE_BC1_SRGB || dest.hwType() == IMAGE_BC4 || dest.hwType() == IMAGE_BC4_SIGN) ? 8 : 16);
        Int src_faces1 = src.faces() - 1;
        REPD(mip, Min(src.mipMaps(), dest.mipMaps())) {
            x_blocks = PaddedWidth(dest.hwW(), dest.hwH(), mip, dest.hwType()) / 4; // operate on mip HW size to process partial and Pow2Padded blocks too
            y_blocks = PaddedHeight(dest.hwW(), dest.hwH(), mip, dest.hwType()) / 4;
            REPD(face, dest.faces()) {
                if (!src.lockRead(mip, (DIR_ENUM)Min(face, src_faces1)))
                    return false;
                if (!dest.lock(LOCK_WRITE, mip, (DIR_ENUM)face)) {
                    src.unlock();
                    return false;
                }

                REPD(z, dest.ld()) {
                    sz = Min(z, src.ld() - 1);
                    dest_data_z = dest.data() + z * dest.pitch2();
                    ImageThreads.process(y_blocks, Process, T);
                }
                dest.unlock();
                src.unlock();
            }
        }
        return true;
    }
};
Bool CompressBC(C Image &src, Image &dest) // no need to store this in a separate CPP file, because its code size is small
{
    if (dest.hwType() == IMAGE_BC1 || dest.hwType() == IMAGE_BC2 || dest.hwType() == IMAGE_BC3 || dest.hwType() == IMAGE_BC4 || dest.hwType() == IMAGE_BC5 || dest.hwType() == IMAGE_BC1_SRGB || dest.hwType() == IMAGE_BC2_SRGB || dest.hwType() == IMAGE_BC3_SRGB || dest.hwType() == IMAGE_BC4_SIGN || dest.hwType() == IMAGE_BC5_SIGN)
    // if(dest.size3()==src.size3()) this check is not needed because the code below supports different sizes
    {
#if 1
        return BCContext(src, dest).process();
#elif 1 // Intel ISPC
        only BC1 now supported
            need mips &cube
                Image temp;
        C Image *s = &src;
        if ((s->hwType() != IMAGE_R8G8B8A8 && s->hwType() != IMAGE_R8G8B8A8_SRGB) || s->w() != dest.hwW() || s->h() != dest.hwH()) {
            use extractNonCompressedMipMapNoStretch if (s->copyTry(temp, dest.hwW(), dest.hwH(), 1, dest.sRGB() ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8A8, (s->cube() && dest.cube()) ? IMAGE_SOFT_CUBE : IMAGE_SOFT, 1, FILTER_NO_STRETCH, true)) s = &temp;
            else return false; // we need to cover the area for entire HW size, to process partial and Pow2Padded blocks too
        }
        Bool ok = false;
        if (s->lockRead()) {
            if (dest.lock(LOCK_WRITE)) {
                ok = true;
                Int x_blocks = dest.hwW() / 4, // operate on HW size to process partial and Pow2Padded blocks too
                    y_blocks = dest.hwH() / 4,
                    x_mul = ((dest.hwType() == IMAGE_BC1 || dest.hwType() == IMAGE_BC1_SRGB || dest.hwType() == IMAGE_BC4 || dest.hwType() == IMAGE_BC4_SIGN) ? 8 : 16);
                rgba_surface surf;
                surf.width = s->w();
                surf.height = s->h();
                surf.stride = s->pitch();
                REPD(z, dest.d()) {
                    surf.ptr = ConstCast(s->data() + z * s->pitch2());
                    if (dest.pitch() == x_blocks * x_mul)
                        CompressBlocksBC1(&surf, dest.data() + z * dest.pitch2());
                    else {
                        split into multiple calls here
                    }
                }
                dest.unlock();
            }
            s->unlock();
        }
        return ok;
#endif
    }
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
