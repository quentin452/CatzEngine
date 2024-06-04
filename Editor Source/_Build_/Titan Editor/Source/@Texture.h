#pragma once
/******************************************************************************/
/******************************************************************************/
class Texture {
    enum {
        SRGB = 1 << 0,       // if sRGB gamma
        SIGN = 1 << 1,       // if signed
        DYNAMIC = 1 << 2,    // if texture is dynamically generated
        REGENERATE = 1 << 3, // if this texture needs to be regenerated
    };
    UID id; // texture id
    Edit::Material::TEX_QUALITY quality;
    byte downsize, // downsize
        channels,  // assume RGB by default (no alpha used)
        flags;

    Texture &downSize(int size);

    Texture &usesAlpha();
    Texture &normal();

    bool sRGB() C;
    Texture &sRGB(bool on);
    bool sign() C;
    Texture &sign(bool on);
    bool dynamic() C;
    Texture &dynamic(bool on);
    bool regenerate() C;
    Texture &regenerate(bool on);

    static int CompareTex(C Texture &tex, C UID &tex_id);

  public:
    Texture();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
