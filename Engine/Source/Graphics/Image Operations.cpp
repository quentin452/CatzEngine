// TODO ADD PROFILE_START AND PROFILE_STOP PROFILERS
/******************************************************************************/
#include "stdafx.h"
namespace EE {
#include "Import/BC.h"
#include "Import/ETC.h"
/******************************************************************************/
ImageThreadsClass ImageThreads;
/******************************************************************************/
ImageThreadsClass &ImageThreadsClass::init() {
    if (!created())
        createIfEmpty(false, Cpu.threads() - 1, 0, "EE.Image #"); // -1 because we will do processing on the caller thread too
    return T;
}
/******************************************************************************/
static Bool Decompress(Image &image, IMAGE_TYPE &type, IMAGE_MODE &mode, Int &mip_maps) // returns if image exists
{
    type = image.type();
    mode = image.mode();
    mip_maps = image.mipMaps();
    if (image.is()) {
        if (image.compressed())
            return image.copy(image, -1, -1, -1, ImageTypeUncompressed(type), AsSoft(image.mode()), 1);
        return true;
    }
    return false;
}
static void Compress(Image &image, IMAGE_TYPE type, IMAGE_MODE mode, Int mip_maps) {
    image.copy(image, -1, -1, -1, type, mode, mip_maps);
}
/******************************************************************************/
Bool Image::extractNonCompressedMipMapNoStretch(Image &dest, Int w, Int h, Int d, Int mip_map, DIR_ENUM cube_face, Bool clamp) C // assumes &T!=&dest
{
    if (dest.create(w, h, d, T.hwType(), IMAGE_SOFT, 1, false)) {
        if (!T.lockRead(mip_map, cube_face))
            return false;
        // if(!dest.lock    (LOCK_WRITE        ))return false; not needed for SOFT

        CopyNoStretch(T, dest, clamp);

        // dest.unlock(); not needed for SOFT
        T.unlock();
        return true;
    }
    return false;
}
static Bool ExtractMipMap(C Image &src, Image &dest, Int mip_map, DIR_ENUM cube_face) // assumes &T!=&dest
{
    if (dest.create(Max(1, src.w() >> mip_map), Max(1, src.h() >> mip_map), Max(1, src.d() >> mip_map), src.hwType(), IMAGE_SOFT, 1, false)) {
        if (!src.lockRead(mip_map, cube_face))
            return false;
        // if(!dest.lock    (LOCK_WRITE        ))return false; not needed for SOFT

        CopyImgData(src.data(), dest.data(), src.pitch(), dest.pitch(), src.softBlocksY(mip_map), dest.softBlocksY(0), src.pitch2(), dest.pitch2(), src.ld(), dest.ld());

        // dest.unlock(); not needed for SOFT
        src.unlock();
        return true;
    }
    return false;
}
Bool Image::extractMipMap(Image &dest, Int type, Int mip_map, DIR_ENUM cube_face, FILTER_TYPE filter, UInt flags) C {
    if (InRange(mip_map, mipMaps())) {
        Image temp, &img = ((this == &dest) ? temp : dest);
        if (ExtractMipMap(T, img, mip_map, cube_face)) // extract
        {
            if (&img != &dest)
                Swap(dest, img); // swap if needed
            if (type > IMAGE_NONE && !dest.copy(dest, -1, -1, -1, type, -1, -1, filter, flags))
                return false; // apply conversion if needed
            return true;
        }
    }
    return false;
}
/******************************************************************************/
Bool Image::injectMipMap(C Image &src, Int mip_map, DIR_ENUM cube_face, FILTER_TYPE filter, UInt flags) {
    Bool ok = false;
    if (InRange(mip_map, mipMaps())) {
        C Image *s = &src;
        Image temp;
        Int w = Max(1, T.w() >> mip_map), h = Max(1, T.h() >> mip_map), d = Max(1, T.d() >> mip_map);
        if (s->w() != w || s->h() != h || s->d() != d || s->hwType() != hwType() || s == this)
            if (s->copy(temp, w, h, d, hwType(), IMAGE_SOFT, 1, filter, flags | IC_NO_ALT_TYPE))
                s = &temp;
            else
                return false; // resize to mip size, need IC_NO_ALT_TYPE to use target type
        if (lock(LOCK_WRITE, mip_map, cube_face)) {
            if (s->lockRead()) {
                ok = true;
                CopyImgData(s->data(), data(), s->pitch(), pitch(), s->softBlocksY(0), softBlocksY(mip_map), s->pitch2(), pitch2(), s->ld(), ld());
                s->unlock();
            }
            unlock();
        }
    }
    return ok;
}
/******************************************************************************/
Image &Image::clear() {
#if IMAGE_STREAM_FULL
    if (!waitForStream())
        return T; // in IMAGE_STREAM_FULL this image can be smaller, so the only thing we can do is wait to get full size
#else
    cancelStream();
    baseMip(0); // can just cancel entire stream because we will overwrite the whole thing
#endif
    if (soft())
        ZeroFast(softData(), memUsage());
    else {
        Int faces = T.faces();
        REPD(m, mipMaps())
        REPD(f, faces)
        if (lock(LOCK_WRITE, m, DIR_ENUM(f))) {
            ZeroFast(data(), ld() * pitch2());
            unlock();
        }
    }
    return T;
}
/******************************************************************************/
Image &Image::normalize(Bool red, Bool green, Bool blue, Bool alpha, C BoxI *box) {
    if (red || green || blue || alpha) {
        IMAGE_TYPE type;
        IMAGE_MODE mode;
        Int mip_maps;
        if (Decompress(T, type, mode, mip_maps)) {
            Vec4 min, max;
            if (stats(&min, &max, null, null, null, null, null, box)) {
                Flt d;
                Vec4 mul, add;
                if (red && (d = max.x - min.x)) {
                    mul.x = 1.0f / d;
                    add.x = -min.x * mul.x;
                } else {
                    mul.x = 1;
                    add.x = 0;
                }
                if (green && (d = max.y - min.y)) {
                    mul.y = 1.0f / d;
                    add.y = -min.y * mul.y;
                } else {
                    mul.y = 1;
                    add.y = 0;
                }
                if (blue && (d = max.z - min.z)) {
                    mul.z = 1.0f / d;
                    add.z = -min.z * mul.z;
                } else {
                    mul.z = 1;
                    add.z = 0;
                }
                if (alpha && (d = max.w - min.w)) {
                    mul.w = 1.0f / d;
                    add.w = -min.w * mul.w;
                } else {
                    mul.w = 1;
                    add.w = 0;
                }
                mulAdd(mul, add, box);
            }
            Compress(T, type, mode, mip_maps);
        }
    }
    return T;
}
/******************************************************************************/
Image &Image::mulAdd(C Vec4 &mul, C Vec4 &add, C BoxI *box) {
    if (mul != Vec4(1) || add != Vec4Zero) {
        IMAGE_TYPE type;
        IMAGE_MODE mode;
        Int mip_maps;
        if (Decompress(T, type, mode, mip_maps)) {
            REPD(f, faces())
            if (lock(LOCK_READ_WRITE, 0, (DIR_ENUM)f)) {
                BoxI b(0, size3());
                if (box)
                    b &= *box;
                for (Int z = b.min.z; z < b.max.z; z++)
                    for (Int y = b.min.y; y < b.max.y; y++)
                        for (Int x = b.min.x; x < b.max.x; x++)
                            color3DF(x, y, z, color3DF(x, y, z) * mul + add);
                unlock();
            }
            updateMipMaps();
            Compress(T, type, mode, mip_maps);
        }
    }
    return T;
}
/******************************************************************************/
void Image::bumpToNormal(Image &dest, Flt scale, Bool high_precision) C {
    C Image *src = this;
    Image temp;
    if (compressed())
        if (copy(temp, -1, -1, 1, ImageTypeUncompressed(type()), IMAGE_SOFT, 1))
            src = &temp;
        else
            return;
    if (src->lockRead()) {
        Image normal;
        if (normal.create(src->w(), src->h(), 1, high_precision ? IMAGE_F32_3 : IMAGE_R8G8B8, IMAGE_SOFT, 1) && normal.lock(LOCK_WRITE)) {
            high_precision = (normal.hwType() == IMAGE_F32_3); // verify in case it was created as different type
            Bool src_hp = src->highPrecision(),
                 src_1c = (src->typeChannels() == 1);
            if (!src_hp)
                scale /= (src_1c ? (1 << (8 * src->bytePP())) - 1 : 255);
            Flt z = 2 / scale;
            REPD(y, src->h())
            REPD(x, src->w()) {
                Vec nrm;
                // Flt bump;
                if (src_hp) {
                    // bump=src->pixelF( x                     ,  y                     );
                    Flt l = src->pixelF((x + src->w() - 1) % src->w(), y),
                        r = src->pixelF((x + 1) % src->w(), y),
                        u = src->pixelF(x, (y + src->h() - 1) % src->h()),
                        d = src->pixelF(x, (y + 1) % src->h());
                    nrm.x = l - r;
                    nrm.y = u - d;
                } else if (src_1c) {
                    // bump=src->pixel( x                     ,  y                     )*scale;
                    Int l = src->pixel((x + src->w() - 1) % src->w(), y),
                        r = src->pixel((x + 1) % src->w(), y),
                        u = src->pixel(x, (y + src->h() - 1) % src->h()),
                        d = src->pixel(x, (y + 1) % src->h());
                    nrm.x = l - r;
                    nrm.y = u - d;
                } else {
                    // bump=src->color( x                     ,  y                     ).r*scale;
                    Byte l = src->color((x + src->w() - 1) % src->w(), y).r,
                         r = src->color((x + 1) % src->w(), y).r,
                         u = src->color(x, (y + src->h() - 1) % src->h()).r,
                         d = src->color(x, (y + 1) % src->h()).r;
                    nrm.x = l - r;
                    nrm.y = u - d;
                }
#if 0
            nrm.x*=scale;
            nrm.y*=scale;
            nrm.z =2;
#else
                nrm.z = z;
#endif
                nrm.normalize();
                if (high_precision)
                    normal.pixF3(x, y) = nrm;
                else {
                    Color color;
                    color.r = SFltToUByte(nrm.x);
                    color.g = SFltToUByte(nrm.y);
                    color.b = SFltToUByte(nrm.z);
                    // color.a= FltToByte (bump);
                    normal.color(x, y, color);
                }
            }
            src->unlock();
            normal.unlock().updateMipMaps(FILTER_BEST, IC_WRAP); // normal maps are usually wrapped
            Swap(dest, normal);
        } else {
            src->unlock();
        }
    }
}
/******************************************************************************
Image& Image::dither(IMAGE_TYPE type)
{
   if(!compressed())switch(type)
   {
      case IMAGE_BC1:
      case IMAGE_BC2:
      case IMAGE_BC3: if(lock()) // "x<<1, y<<2" was tested as best combination (from x=0..3 and y=0..3, XOR was also tested instead of ADD but it made no difference)
      {
         if(hwType()==IMAGE_B8G8R8A8 || hwType()==IMAGE_R8G8B8A8)
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               VecB4 &v=pixB4(x, y);
               Int    d=(((x<<1)+(y<<2))&7)-4;
               v.x=Mid(v.x+ d    , 0, 255); // red   (8-bit original, 5-bit on DXT, 3-bit dither)
               v.y=Mid(v.y+(d>>1), 0, 255); // green (8-bit original, 6-bit on DXT, 2-bit dither)
               v.z=Mid(v.z+ d    , 0, 255); // blue  (8-bit original, 5-bit on DXT, 3-bit dither)
            }
         }else
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               Color c=color(x, y);
               Int   d=(((x<<1)+(y<<2))&7)-4;
               c.r=Mid(c.r+ d    , 0, 255); // red   (8-bit original, 5-bit on DXT, 3-bit dither)
               c.g=Mid(c.g+(d>>1), 0, 255); // green (8-bit original, 6-bit on DXT, 2-bit dither)
               c.b=Mid(c.b+ d    , 0, 255); // blue  (8-bit original, 5-bit on DXT, 3-bit dither)
               color(x, y, c);
            }
         }
         unlock();
      }break;

      case IMAGE_ETC1: if(lock()) // ETC has 4/4/4 bits or 5/5/5 bits per pixel (set 3-bit dither because 4 is visually too big, set smaller dither for green because it has bigger perceptual value in ETC encoder)
      {
         if(hwType()==IMAGE_B8G8R8A8 || hwType()==IMAGE_R8G8B8A8)
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               VecB4 &v=pixB4(x, y);
               Int    d=(((x<<1)+(y<<2))&7)-4;
               v.x=Mid(v.x+ d    , 0, 255);
               v.y=Mid(v.y+(d>>1), 0, 255);
               v.z=Mid(v.z+ d    , 0, 255);
            }
         }else
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               Color c=color(x, y);
               Int   d=(((x<<1)+(y<<2))&7)-4;
               c.r=Mid(c.r+ d    , 0, 255);
               c.g=Mid(c.g+(d>>1), 0, 255);
               c.b=Mid(c.b+ d    , 0, 255);
               color(x, y, c);
            }
         }
         unlock();
      }break;

      case IMAGE_PVRTC1_2:
      case IMAGE_PVRTC1_4: if(lock()) // 2-bit dithering gave best results
      {
         if(hwType()==IMAGE_B8G8R8A8 || hwType()==IMAGE_R8G8B8A8)
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               VecB4 &v=pixB4(x, y);
               Int    d=(((x<<0)+(y<<1))&3)-2;
               v.x=Mid(v.x+d, 0, 255);
               v.y=Mid(v.y+d, 0, 255);
               v.z=Mid(v.z+d, 0, 255);
            }
         }else
         {
            REPD(y, T.h())
            REPD(x, T.w())
            {
               Color c=color(x, y);
               Int   d=(((x<<0)+(y<<1))&3)-2;
               c.r=Mid(c.r+d, 0, 255);
               c.g=Mid(c.g+d, 0, 255);
               c.b=Mid(c.b+d, 0, 255);
               color(x, y, c);
            }
         }
         unlock();
      }break;
   }
   return T;
}
/******************************************************************************/
void Image::crop(Image &dest, Int x, Int y, Int w, Int h, C Vec4 *clear_color) C {
    crop3D(dest, x, y, 0, w, h, d(), clear_color);
}
void Image::crop3D(Image &dest, Int x, Int y, Int z, Int w, Int h, Int d, C Vec4 *clear_color) C {
    if (!is() || w <= 0 || h <= 0 || d <= 0) {
        dest.del();
        return;
    }
    if (&dest == this && x == 0 && y == 0 && z == 0 && w == T.w() && h == T.h() && d == T.d())
        return; // no change needed

    C Image *src = this;
    Image temp;
    if (compressed())
        if (copy(temp, -1, -1, -1, ImageTypeUncompressed(type()), IMAGE_SOFT, 1))
            src = &temp;
        else
            return;

    if (src->lockRead()) {
        Int mip_maps = ((mipMaps() > 1) ? 0 : 1);
        Image temp;
        if (temp.create(w, h, d, src->type(), src->mode(), mip_maps) && temp.lock(LOCK_WRITE)) {
            if (src->bytePP() <= 4) {
                REPD(dz, d) {
                    Int sz = z + dz;
                    Bool vz = InRange(sz, src->d());
                    REPD(dy, h) {
                        Int sy = y + dy;
                        Bool vzy = (vz && InRange(sy, src->h()));
                        REPD(dx, w) {
                            Int sx = x + dx;
                            Bool vzyx = (vzy && InRange(sx, src->w()));
                            if (vzyx)
                                temp.pixel3D(dx, dy, dz, src->pixel3D(sx, sy, sz));
                            else if (clear_color)
                                temp.color3DF(dx, dy, dz, *clear_color);
                        }
                    }
                }
            } else if (src->typeChannels() <= 1) {
                REPD(dz, d) {
                    Int sz = z + dz;
                    Bool vz = InRange(sz, src->d());
                    REPD(dy, h) {
                        Int sy = y + dy;
                        Bool vzy = (vz && InRange(sy, src->h()));
                        REPD(dx, w) {
                            Int sx = x + dx;
                            Bool vzyx = (vzy && InRange(sx, src->w()));
                            if (vzyx)
                                temp.pixel3DF(dx, dy, dz, src->pixel3DF(sx, sy, sz));
                            else if (clear_color)
                                temp.color3DF(dx, dy, dz, *clear_color);
                        }
                    }
                }
            } else {
                REPD(dz, d) {
                    Int sz = z + dz;
                    Bool vz = InRange(sz, src->d());
                    REPD(dy, h) {
                        Int sy = y + dy;
                        Bool vzy = (vz && InRange(sy, src->h()));
                        REPD(dx, w) {
                            Int sx = x + dx;
                            Bool vzyx = (vzy && InRange(sx, src->w()));
                            if (vzyx)
                                temp.color3DF(dx, dy, dz, src->color3DF(sx, sy, sz));
                            else if (clear_color)
                                temp.color3DF(dx, dy, dz, *clear_color);
                        }
                    }
                }
            }

            temp.unlock().updateMipMaps();
            temp.copy(temp, -1, -1, -1, T.type(), T.mode(), mip_maps);
            Swap(dest, temp);
        }
        src->unlock();
    }
}
/******************************************************************************/
static Bool TransparentX(C Image &image, Int x) {
    REPD(z, image.ld())
    REPD(y, image.lh())
    if (image.color3D(x, y, z).a)
        return false;
    return true;
}
static Bool TransparentY(C Image &image, Int y) {
    REPD(z, image.ld())
    REPD(x, image.lw())
    if (image.color3D(x, y, z).a)
        return false;
    return true;
}
static Bool TransparentZ(C Image &image, Int z) {
    REPD(y, image.lh())
    REPD(x, image.lw())
    if (image.color3D(x, y, z).a)
        return false;
    return true;
}
Image &Image::cropTransparent(Bool border) {
    C Image *src = this;
    Image temp;
    if (compressed())
        if (copy(temp, -1, -1, -1, ImageTypeUncompressed(type()), IMAGE_SOFT, 1))
            src = &temp;
        else
            return T;

    if (src->lockRead()) { // inclusive
        Int z = 0, z1 = src->d() - 1;
        Int y = 0, y1 = src->h() - 1;
        Int x = 0, x1 = src->w() - 1;

        // Z
        if (src->d() > 1) {
            for (; TransparentZ(*src, z);) {
                if (z >= z1) {
                    x = x1 = y = y1 = z = z1 = 0;
                    goto finish;
                } // fully transparent, return as 1x1x1 pixel
                z++;
            }
            for (; z1 > z && TransparentZ(*src, z1);)
                z1--;

            if (border) {
                MAX(--z, 0);
                MIN(++z1, src->d() - 1);
            }
        }

        // Y
        for (; TransparentY(*src, y);) {
            if (y >= y1) {
                x = x1 = y = y1 = z = z1 = 0;
                goto finish;
            } // fully transparent, return as 1x1x1 pixel
            y++;
        }
        for (; y1 > y && TransparentY(*src, y1);)
            y1--;

        // X
        for (; TransparentX(*src, x);) {
            if (x >= x1) {
                x = x1 = y = y1 = z = z1 = 0;
                goto finish;
            } // fully transparent, return as 1x1x1 pixel
            x++;
        }
        for (; x1 > x && TransparentX(*src, x1);)
            x1--;

        if (border) {
            MAX(--y, 0);
            MIN(++y1, src->h() - 1);
            MAX(--x, 0);
            MIN(++x1, src->w() - 1);
        }

    finish:
        src->unlock();
        src->crop3D(T, x, y, z, x1 - x + 1, y1 - y + 1, z1 - z + 1);
    }

    return T;
}
/******************************************************************************/
Image &Image::resize(Int w, Int h, FILTER_TYPE filter, UInt flags) {
    MAX(w, 1);
    MAX(h, 1);
    if (is() && (w != T.w() || h != T.h()))
        copy(T, w, h, -1, -1, -1, -1, filter, flags);
    return T;
}
/******************************************************************************/
Image &Image::resize3D(Int w, Int h, Int d, FILTER_TYPE filter, UInt flags) {
    MAX(w, 1);
    MAX(h, 1);
    MAX(d, 1);
    if (is() && (w != T.w() || h != T.h() || d != T.d()))
        copy(T, w, h, d, -1, -1, -1, filter, flags);
    return T;
}
/******************************************************************************/
// MIRROR
/******************************************************************************/
Image &Image::mirrorX() {
    if (is()) {
        C Image *src = this;
        Image temp;
        if (compressed())
            if (copy(temp, -1, -1, -1, ImageTypeUncompressed(type()), IMAGE_SOFT, 1))
                src = &temp;
            else
                return T;
        if (src->lockRead()) {
            Bool ok = false;
            Image mirror;
            if (mirror.create(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))
                if (mirror.lock(LOCK_WRITE)) {
                    if (mirror.highPrecision()) {
                        REPD(z, mirror.d())
                        REPD(y, mirror.h())
                        REPD(x, mirror.w())
                        mirror.color3DF(x, y, z, src->color3DF(mirror.w() - 1 - x, y, z));
                    } else {
                        REPD(z, mirror.d())
                        REPD(y, mirror.h())
                        REPD(x, mirror.w())
                        mirror.color3D(x, y, z, src->color3D(mirror.w() - 1 - x, y, z));
                    }
                    mirror.unlock().updateMipMaps();
                    ok = mirror.copy(mirror, w(), h(), d(), type(), mode(), mipMaps());
                }
            src->unlock();
            if (ok)
                Swap(T, mirror);
        }
    }
    return T;
}
/******************************************************************************/
Image &Image::mirrorY() {
    if (is()) {
        C Image *src = this;
        Image temp;
        if (compressed())
            if (copy(temp, -1, -1, -1, ImageTypeUncompressed(type()), IMAGE_SOFT, 1))
                src = &temp;
            else
                return T;
        if (src->lockRead()) {
            Bool ok = false;
            Image mirror;
            if (mirror.create(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))
                if (mirror.lock(LOCK_WRITE)) {
                    if (mirror.highPrecision()) {
                        REPD(z, mirror.d())
                        REPD(y, mirror.h())
                        REPD(x, mirror.w())
                        mirror.color3DF(x, y, z, src->color3DF(x, mirror.h() - 1 - y, z));
                    } else {
                        REPD(z, mirror.d())
                        REPD(y, mirror.h())
                        REPD(x, mirror.w())
                        mirror.color3D(x, y, z, src->color3D(x, mirror.h() - 1 - y, z));
                    }
                    mirror.unlock().updateMipMaps();
                    ok = mirror.copy(mirror, w(), h(), d(), type(), mode(), mipMaps());
                }
            src->unlock();
            if (ok)
                Swap(T, mirror);
        }
    }
    return T;
}
/******************************************************************************/
Image &Image::mirrorXY() {
    if (is()) {
        C Image *src = this;
        Image temp;
        if (compressed())
            if (copy(temp, -1, -1, -1, ImageTypeUncompressed(type()), IMAGE_SOFT, 1))
                src = &temp;
            else
                return T;
        if (src->lockRead()) {
            Bool ok = false;
            Image mirror;
            if (mirror.create(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))
                if (mirror.lock(LOCK_WRITE)) {
                    if (mirror.highPrecision()) {
                        REPD(z, mirror.d())
                        REPD(y, mirror.h())
                        REPD(x, mirror.w())
                        mirror.color3DF(x, y, z, src->color3DF(mirror.w() - 1 - x, mirror.h() - 1 - y, z));
                    } else {
                        REPD(z, mirror.d())
                        REPD(y, mirror.h())
                        REPD(x, mirror.w())
                        mirror.color3D(x, y, z, src->color3D(mirror.w() - 1 - x, mirror.h() - 1 - y, z));
                    }
                    mirror.unlock().updateMipMaps();
                    ok = mirror.copy(mirror, w(), h(), d(), type(), mode(), mipMaps());
                }
            src->unlock();
            if (ok)
                Swap(T, mirror);
        }
    }
    return T;
}
/******************************************************************************/
Image &Image::rotateL() {
    if (is()) {
        C Image *src = this;
        Image temp;
        if (compressed())
            if (copy(temp, -1, -1, -1, ImageTypeUncompressed(type()), IMAGE_SOFT, 1))
                src = &temp;
            else
                return T;
        if (src->lockRead()) {
            Bool ok = false;
            Image rot;
            if (rot.create(src->h(), src->w(), src->d(), src->type(), src->mode(), src->mipMaps()))
                if (rot.lock(LOCK_WRITE)) {
                    if (rot.highPrecision()) {
                        REPD(z, rot.d())
                        REPD(y, rot.h())
                        REPD(x, rot.w())
                        rot.color3DF(x, y, z, src->color3DF(src->w() - 1 - y, x, z));
                    } else {
                        REPD(z, rot.d())
                        REPD(y, rot.h())
                        REPD(x, rot.w())
                        rot.color3D(x, y, z, src->color3D(src->w() - 1 - y, x, z));
                    }
                    rot.unlock().updateMipMaps();
                    ok = rot.copy(rot, h(), w(), d(), type(), mode(), mipMaps());
                }
            src->unlock();
            if (ok)
                Swap(T, rot);
        }
    }
    return T;
}
Image &Image::rotateR() {
    if (is()) {
        C Image *src = this;
        Image temp;
        if (compressed())
            if (copy(temp, -1, -1, -1, ImageTypeUncompressed(type()), IMAGE_SOFT, 1))
                src = &temp;
            else
                return T;
        if (src->lockRead()) {
            Bool ok = false;
            Image rot;
            if (rot.create(src->h(), src->w(), src->d(), src->type(), src->mode(), src->mipMaps()))
                if (rot.lock(LOCK_WRITE)) {
                    if (rot.highPrecision()) {
                        REPD(z, rot.d())
                        REPD(y, rot.h())
                        REPD(x, rot.w())
                        rot.color3DF(x, y, z, src->color3DF(y, src->h() - 1 - x, z));
                    } else {
                        REPD(z, rot.d())
                        REPD(y, rot.h())
                        REPD(x, rot.w())
                        rot.color3D(x, y, z, src->color3D(y, src->h() - 1 - x, z));
                    }
                    rot.unlock().updateMipMaps();
                    ok = rot.copy(rot, h(), w(), d(), type(), mode(), mipMaps());
                }
            src->unlock();
            if (ok)
                Swap(T, rot);
        }
    }
    return T;
}
/******************************************************************************/
void Image::transform(Image &dest, C Matrix2 &matrix, FILTER_TYPE filter, UInt flags) C {
    if (is()) {
        C Image *src = this;
        Image temp;
        if (compressed())
            if (copy(temp, -1, -1, -1, ImageTypeUncompressed(type()), IMAGE_SOFT, 1))
                src = &temp;
            else
                return;
        if (src->lockRead()) {
            Vec2 corner[2 * 2];
            corner[0].set(0, 0);
            corner[1].set(src->w(), 0);
            corner[2].set(0, src->h());
            corner[3].set(src->w(), src->h());
            REPAO(corner) *= matrix;
            Rect rect;
            rect.from(corner, Elms(corner));

            Image &work = ((src == &dest) ? temp : dest);
            if (work.create(Round(rect.w()), Round(rect.h()), src->d(), src->type(), src->mode(), src->mipMaps()))
                if (work.lock(LOCK_WRITE)) {
                    Vec2 area_size = 1 / matrix.scale();
                    Bool clamp = IcClamp(flags), alpha_weight = FlagOn(flags, IC_ALPHA_WEIGHT),
                         downsize = (filter != FILTER_NONE && (area_size.x > 1 + EPS || area_size.y > 1 + EPS) && src->ld() == 1); // if we're downsampling (any scale is higher than 1) then we must use more complex 'areaColor*' methods
                    union {
                        PtrImageColor ptr_color;
                        struct
                        {
                            Bool linear_gamma;
                            PtrImageAreaColor ptr_area_color;
                        };
                    };
                    if (downsize) {
                        ptr_area_color = GetImageAreaColor(filter, linear_gamma);
                    } else
                        ptr_color = GetImageColorF(filter);
                    Matrix2P m;
                    matrix.inverse(m.orn());
                    // make sure that in 'corner[0]' (relative to 'rect') we get (0,0) UV
                    //(corner[0]-rect.min)*m=0;
                    //(corner[0]-rect.min)*m.orn+m.pos=0;
                    m.pos = -(corner[0] - rect.min) * m.orn();

                    // fix pos round (needed when scaling)
                    m.pos += 0.5f * (m.x + m.y);

                    // optimize to replace "work.colorF(x, work.h()-1-y, (src->*ptr_color)(coord.x, src->h()-1-coord.y, clamp, alpha_weight));"
                    //             with    "work.colorF(x,            y, (src->*ptr_color)(coord.x,            coord.y, clamp, alpha_weight));"
                    // this is because "Vec2(x, y) POS" refers to "Vec2(x, h-1-y) UV"

                    // convert "work.h()-1-y" -> "y"
                    // Vec2& Vec2::mul(C Matrix2P &m)
                    // x*m.x.x + (work.h()-1-y)*m.y.x + m.pos.x;
                    // x*m.x.y + (work.h()-1-y)*m.y.y + m.pos.y;
                    // ->
                    // x*m.x.x + y*(-m.y.x) + (work.h()-1)*m.y.x + m.pos.x;
                    // x*m.x.y + y*(-m.y.y) + (work.h()-1)*m.y.y + m.pos.y;

                    m.pos += (work.h() - 1) * m.y;
                    m.y.chs();

                    // fix pos round (needed when scaling)
                    m.pos -= 0.5f;

                    // convert "src->h()-1-coord.y" -> "coord.y"
                    // convert "src->h()-1-coord.y" -> "src->h()-1+coord.y"
                    CHS(m.x.y);
                    CHS(m.y.y);
                    CHS(m.pos.y);
                    // add "src->h()-1"
                    m.pos.y += src->h() - 1;

                    // REPD(z, work.d())
                    {
                        Vec coord; // coord.z=z;
                        REPD(y, work.h())
                        REPD(x, work.w()) {
                            coord.xy.set(x, y) *= m;
                            Vec4 color;
                            if (downsize) {
                                color = (src->*ptr_area_color)(coord.xy, area_size, clamp, alpha_weight);
                                if (linear_gamma)
                                    work.colorL(x, y, color);
                                else
                                    work.colorF(x, y, color);
                            } else {
                                color = (src->*ptr_color)(coord.x, coord.y, clamp, alpha_weight);
                                work.colorF(x, y, color);
                            }
                        }
                    }
                    work.unlock();
                    work.updateMipMaps();
                }
            src->unlock();
            if (&work != &dest)
                Swap(work, dest);
        }
    }
}
void Image::rotate(Image &dest, Flt angle, FILTER_TYPE filter, UInt flags) C { transform(dest, Matrix2().setRotate(angle), filter, flags); }
void Image::rotateScale(Image &dest, Flt angle, Flt scale, FILTER_TYPE filter, UInt flags) C { transform(dest, Matrix2().setRotate(angle).scale(scale), filter, flags); }
/******************************************************************************/
// ALPHA
/******************************************************************************/
Image &Image::alphaFromKey(C Color &key) {
    IMAGE_TYPE type;
    IMAGE_MODE mode;
    Int mip_maps;
    if (Decompress(T, type, mode, mip_maps)) {
        if (!typeInfo().a) {
            mustCopy(T, -1, -1, -1, ImageTypeIncludeAlpha(T.type()), T.mode(), T.mipMaps());
            type = T.type();
        } // if image doesn't have alpha channel then include it, set 'type' to value after conversion so it won't be converted back
        if (lock()) {
            REPD(z, T.d())
            REPD(y, T.h())
            REPD(x, T.w()) {
                Color c = color3D(x, y, z);
                color3D(x, y, z, (c == key) ? TRANSPARENT : Color(c.r, c.g, c.b));
            }
            unlock().updateMipMaps();
            Compress(T, type, mode, mip_maps);
        }
    }
    return T;
}
/******************************************************************************/
Image &Image::alphaFromBrightness() {
    IMAGE_TYPE type;
    IMAGE_MODE mode;
    Int mip_maps;
    if (Decompress(T, type, mode, mip_maps)) {
        if (!typeInfo().a) {
            mustCopy(T, -1, -1, -1, ImageTypeIncludeAlpha(T.type()), T.mode(), T.mipMaps());
            type = T.type();
        } // if image doesn't have alpha channel then include it, set 'type' to value after conversion so it won't be converted back
        if (lock()) {
            if (highPrecision()) {
                REPD(z, T.d())
                REPD(y, T.h())
                REPD(x, T.w()) {
                    Vec4 c = color3DF(x, y, z);
                    c.w = c.xyz.max();
                    color3DF(x, y, z, c);
                }
            } else {
                REPD(z, T.d())
                REPD(y, T.h())
                REPD(x, T.w()) {
                    Color c = color3D(x, y, z);
                    c.a = c.lum();
                    color3D(x, y, z, c);
                }
            }
            unlock().updateMipMaps();
            Compress(T, type, mode, mip_maps);
        }
    }
    return T;
}
/******************************************************************************/
Image &Image::divRgbByAlpha() {
    if (typeInfo().a) {
        IMAGE_TYPE type;
        IMAGE_MODE mode;
        Int mip_maps;
        if (Decompress(T, type, mode, mip_maps) && lock()) {
            if (highPrecision()) {
                REPD(z, T.d())
                REPD(y, T.h())
                REPD(x, T.w()) {
                    Vec4 c = color3DF(x, y, z);
                    if (c.w) {
                        c.xyz /= c.w;
                        color3DF(x, y, z, c);
                    }
                }
            } else {
                REPD(z, T.d())
                REPD(y, T.h())
                REPD(x, T.w()) {
                    Color c = color3D(x, y, z);
                    if (c.a) {
                        c.r = Min(c.r * 255 / c.a, 255);
                        c.g = Min(c.g * 255 / c.a, 255);
                        c.b = Min(c.b * 255 / c.a, 255);
                        color3D(x, y, z, c);
                    }
                }
            }
            unlock().updateMipMaps();
            Compress(T, type, mode, mip_maps);
        }
    }
    return T;
}
/******************************************************************************/
// DOWNSAMPLE
/******************************************************************************/
Image &Image::downSample(FILTER_TYPE filter, UInt flags) {
    if (w() > 1 || h() > 1 || d() > 1)
        copy(T, Max(1, w() >> 1), Max(1, h() >> 1), Max(1, d() >> 1), -1, -1, -1, filter, flags);
    return T;
}
/******************************************************************************
Image& Image::downSampleNormal()
{
   if((w()>1 || h()>1) && d()==1)
   {
      IMAGE_TYPE type;
      IMAGE_MODE mode;
      Int        mip_maps;
      if(Decompress(T, type, mode, mip_maps)) // try to preserve number of mip-maps
      {
         if(lockRead())
         {
            Image temp(Max(1, w()>>1), Max(1, h()>>1), Max(1, d()>>1), T.type(), T.mode(), ImageTI[type].compressed ? 1 : mip_maps);
            if(   temp.lock(LOCK_WRITE))
            {
               REPD(y, temp.h())
               REPD(x, temp.w())
               {
                  UInt  x2=x*2,
                        y2=y*2;
                  Color lu=color(x2+0, y2+0),
                        ru=color(x2+1, y2+0),
                        ld=color(x2+0, y2+1),
                        rd=color(x2+1, y2+1);

                  Vec nrm;
                  nrm.x = lu.r+ru.r+ld.r+rd.r - 4*128;
                  nrm.y = lu.g+ru.g+ld.g+rd.g - 4*128;
                  nrm.z = lu.b+ru.b+ld.b+rd.b - 4*128;
                  nrm.normalize();

                  Color color;
                  color.r=Round(nrm.x*127+128);
                  color.g=Round(nrm.y*127+128);
                  color.b=Round(nrm.z*127+128);
                  color.a=((lu.a+ru.a+ld.a+rd.a+2)>>2);

                  temp.color(x, y, color);
               }
               Swap(unlock(), temp.unlock().updateMipMaps());
               Compress(T, type, mode, mip_maps);
            }else
            {
               unlock();
            }
         }
      }
   }
   return T;
}
/******************************************************************************/
// BLUR
/******************************************************************************/
#define BLUR_CUBE_LINEAR_GAMMA 1 // this is for PBR rendering, so use linear to be more physically accurate
struct BlurCube {
    Bool linear, multi_channel;
    DIR_ENUM face;
    Int src_res, dest_res, src_face_size, src_pitch, src_mip;
    Flt diag_angle_cos_min, cos_min, angle, ball_r, ball_r2;
    union {
        struct // linear
        {
            Flt src_DirToCubeFacePixel_mul, src_DirToCubeFacePixel_add,
                src_CubeFacePixelToDir_mul, src_CubeFacePixelToDir_add,
                dest_CubeFacePixelToDir_mul, dest_CubeFacePixelToDir_add;
        };
        struct // spherical
        {
            Flt src_AngleToCubeFacePixel_mul, src_AngleToCubeFacePixel_add,
                src_CubeFacePixelToAngle_mul, src_CubeFacePixelToAngle_add,
                dest_CubeFacePixelToAngle_mul, dest_CubeFacePixelToAngle_add;
        };
    };
    Vec2 src_area_size;
    C Byte *src_data;
    C Image &src;
    Image &dest;
    SyncLock lock;

    static inline Flt Weight(Flt f) {
        switch (0) {
        case 0:
            return 1 - f; // Linear (similar to Cos but faster)
        case 1:
            return Cos(f * PI_2); // Cos (similar to Linear)
        case 2:
            return 1 - _SmoothCube(f); // SmoothCube (sharper than Linear/Cos)
        }
        return 1; // constant/average (unnatural)
    }

    static void ProcessLine(IntPtr y, BlurCube &bc, Int thread_index) { bc.processLine(y); }
    void processLine(Int y) {
        Vec dir_f;
        dir_f.z = 1;

        if (linear) {
            dir_f.y = -y * dest_CubeFacePixelToDir_mul - dest_CubeFacePixelToDir_add;
            // dir_angle.y=Atan(dir_f.y); // Angle(dir_f.z, dir_f.y); dir_f.z==1
        } else {
            Flt dir_angle_y = -y * dest_CubeFacePixelToAngle_mul - dest_CubeFacePixelToAngle_add;
            dir_f.y = Tan(dir_angle_y);
        }

        REPD(x, dest_res) {
            Bool check_other_faces = false;
            Vec dir_fn;
            RectI rect;
            Flt len2, sin2, cos;
            Vec zd, d, test;
            if (linear) {
                // dir_f=CubeFacePixelToDir(DIR_FORWARD, x, y, dest_res);
                dir_f.x = x * dest_CubeFacePixelToDir_mul + dest_CubeFacePixelToDir_add;
                dir_fn = dir_f;
                dir_fn.normalize();
                // dir_angle.x=Atan(dir_f.x); // Angle(dir_f.z, dir_f.x); dir_f.z==1

                zd.set(dir_fn.x, 0, dir_fn.z);
                len2 = zd.length2();
                if (ball_r2 >= len2) {
                    rect.setX(0, src_res - 1);
                    check_other_faces = true;
                } else {
                    sin2 = ball_r2 / len2;
                    cos = CosSin2(sin2);
                    d = CrossUp(zd);
                    d.setLength(cos * ball_r);
                    zd *= -sin2;
                    zd += dir_fn;
                    test = zd - d;
                    if (test.z > 0) {
                        rect.min.x = CeilSpecial(test.x / test.z * src_DirToCubeFacePixel_mul + src_DirToCubeFacePixel_add);
                        if (rect.min.x < 0) {
                        left_linear:
                            rect.min.x = 0;
                            check_other_faces = true;
                        }
                    } else
                        goto left_linear;
                    test = zd + d;
                    if (test.z > 0) {
                        rect.max.x = FloorSpecial(test.x / test.z * src_DirToCubeFacePixel_mul + src_DirToCubeFacePixel_add);
                        if (rect.max.x >= src_res) {
                        right_linear:
                            rect.max.x = src_res - 1;
                            check_other_faces = true;
                        }
                    } else
                        goto right_linear;
                }

                zd.set(0, dir_fn.y, dir_fn.z);
                len2 = zd.length2();
                if (ball_r2 >= len2) {
                    rect.setY(0, src_res - 1);
                    check_other_faces = true;
                } else {
                    sin2 = ball_r2 / len2;
                    cos = CosSin2(sin2);
                    d = CrossRight(zd);
                    d.setLength(cos * ball_r);
                    zd *= -sin2;
                    zd += dir_fn;
                    test = zd - d;
                    if (test.z > 0) {
                        rect.min.y = CeilSpecial(-test.y / test.z * src_DirToCubeFacePixel_mul + src_DirToCubeFacePixel_add);
                        if (rect.min.y < 0) {
                        down_linear:
                            rect.min.y = 0;
                            check_other_faces = true;
                        }
                    } else
                        goto down_linear;
                    test = zd + d;
                    if (test.z > 0) {
                        rect.max.y = FloorSpecial(-test.y / test.z * src_DirToCubeFacePixel_mul + src_DirToCubeFacePixel_add);
                        if (rect.max.y >= src_res) {
                        up_linear:
                            rect.max.y = src_res - 1;
                            check_other_faces = true;
                        }
                    } else
                        goto up_linear;
                }
            } else {
                Flt dir_angle_x = x * dest_CubeFacePixelToAngle_mul + dest_CubeFacePixelToAngle_add;
                dir_f.x = Tan(dir_angle_x);
                dir_fn = dir_f;
                dir_fn.normalize();

                zd.set(dir_fn.x, 0, dir_fn.z);
                len2 = zd.length2();
                if (ball_r2 >= len2) {
                    rect.setX(0, src_res - 1);
                    check_other_faces = true;
                } else {
                    sin2 = ball_r2 / len2;
                    cos = CosSin2(sin2);
                    d = CrossUp(zd);
                    d.setLength(cos * ball_r);
                    zd *= -sin2;
                    zd += dir_fn;
                    test = zd - d;
                    if (test.z > 0) {
                        rect.min.x = CeilSpecial(Atan(test.x / test.z) * src_AngleToCubeFacePixel_mul + src_AngleToCubeFacePixel_add);
                        if (rect.min.x < 0) {
                        left_sphere:
                            rect.min.x = 0;
                            check_other_faces = true;
                        }
                    } else
                        goto left_sphere;
                    test = zd + d;
                    if (test.z > 0) {
                        rect.max.x = FloorSpecial(Atan(test.x / test.z) * src_AngleToCubeFacePixel_mul + src_AngleToCubeFacePixel_add);
                        if (rect.max.x >= src_res) {
                        right_sphere:
                            rect.max.x = src_res - 1;
                            check_other_faces = true;
                        }
                    } else
                        goto right_sphere;
                }

                zd.set(0, dir_fn.y, dir_fn.z);
                len2 = zd.length2();
                if (ball_r2 >= len2) {
                    rect.setY(0, src_res - 1);
                    check_other_faces = true;
                } else {
                    sin2 = ball_r2 / len2;
                    cos = CosSin2(sin2);
                    d = CrossRight(zd);
                    d.setLength(cos * ball_r);
                    zd *= -sin2;
                    zd += dir_fn;
                    test = zd - d;
                    if (test.z > 0) {
                        rect.min.y = CeilSpecial(-Atan(test.y / test.z) * src_AngleToCubeFacePixel_mul + src_AngleToCubeFacePixel_add);
                        if (rect.min.y < 0) {
                        down_sphere:
                            rect.min.y = 0;
                            check_other_faces = true;
                        }
                    } else
                        goto down_sphere;
                    test = zd + d;
                    if (test.z > 0) {
                        rect.max.y = FloorSpecial(-Atan(test.y / test.z) * src_AngleToCubeFacePixel_mul + src_AngleToCubeFacePixel_add);
                        if (rect.max.y >= src_res) {
                        up_sphere:
                            rect.max.y = src_res - 1;
                            check_other_faces = true;
                        }
                    } else
                        goto up_sphere;
                }
            }

#if 0 // test rect coverage
#pragma message("!! Warning: Use this only for debugging !!")
         if(!face)
         {
            RectI test_rect=rect; test_rect.extend(1)&=RectI(0, src_res-1);
            Vec dir_test;
            // test top and bottom horizontal neighbor lines
            Flt dir_y0=(linear ?     -test_rect.min.y*src_CubeFacePixelToDir_mul  -src_CubeFacePixelToDir_add
                               : Tan(-test_rect.min.y*src_CubeFacePixelToAngle_mul-src_CubeFacePixelToAngle_add));
            Flt dir_y1=(linear ?     -test_rect.max.y*src_CubeFacePixelToDir_mul  -src_CubeFacePixelToDir_add
                               : Tan(-test_rect.max.y*src_CubeFacePixelToAngle_mul-src_CubeFacePixelToAngle_add));
            for(Int tx=test_rect.min.x; tx<=test_rect.max.x; tx++)
            {
               Flt dir_x=(linear ?     tx*src_CubeFacePixelToDir_mul  +src_CubeFacePixelToDir_add
                                 : Tan(tx*src_CubeFacePixelToAngle_mul+src_CubeFacePixelToAngle_add));
               dir_test.set(dir_x, dir_y0, 1); dir_test.normalize(); if(Dot(dir_fn, dir_test)>cos_min && !rect.includesY(test_rect.min.y))Exit("fail");
               dir_test.set(dir_x, dir_y1, 1); dir_test.normalize(); if(Dot(dir_fn, dir_test)>cos_min && !rect.includesY(test_rect.max.y))Exit("fail");
            }
            // test left and right vertical neighbor lines
            Flt dir_x0=(linear ?     test_rect.min.x*src_CubeFacePixelToDir_mul  +src_CubeFacePixelToDir_add
                               : Tan(test_rect.min.x*src_CubeFacePixelToAngle_mul+src_CubeFacePixelToAngle_add));
            Flt dir_x1=(linear ?     test_rect.max.x*src_CubeFacePixelToDir_mul  +src_CubeFacePixelToDir_add
                               : Tan(test_rect.max.x*src_CubeFacePixelToAngle_mul+src_CubeFacePixelToAngle_add));
            for(Int ty=test_rect.min.y; ty<=test_rect.max.y; ty++)
            {
               Flt dir_y=(linear ?     -ty*src_CubeFacePixelToDir_mul  -src_CubeFacePixelToDir_add
                                 : Tan(-ty*src_CubeFacePixelToAngle_mul-src_CubeFacePixelToAngle_add));
               dir_test.set(dir_x0, dir_y, 1); dir_test.normalize(); if(Dot(dir_fn, dir_test)>cos_min && !rect.includesX(test_rect.min.x))Exit("fail");
               dir_test.set(dir_x1, dir_y, 1); dir_test.normalize(); if(Dot(dir_fn, dir_test)>cos_min && !rect.includesX(test_rect.max.x))Exit("fail");
            }
         }
#endif

#if 0 // export coverage map
#pragma message("!! Warning: Use this only for debugging !!")
         //if(!face)
         if(i==1)
         if(x==Round(dest_res*0.7) && y==Round(dest_res*0.9))
         {
            Bool ok=true;
            Image img; img.createSoft(src_res, src_res, 1, IMAGE_R8G8B8);
            for(Int ty=0; ty<src_res; ty++)
            for(Int tx=0; tx<src_res; tx++)
            {
               Vec dir_test(tx*src_CubeFacePixelToDir_mul+src_CubeFacePixelToDir_add, -ty*src_CubeFacePixelToDir_mul-src_CubeFacePixelToDir_add, 1); dir_test.normalize();
             //Vec dir_test=CubeFacePixelToDir(DIR_FORWARD, tx, ty, src_res); dir_test.normalize();
               Bool cone=(Dot(dir_fn, dir_test)>=cos_min);
             //Bool cone=(AbsAngleBetweenN(dir_fn, dir_test)<=angle);
               Bool rect=rect.includes(VecI2(tx, ty));
               if(cone && !rect)ok=false;
               img.color(tx, ty, Color(cone ? 255 : 0, rect ? 255 : 0, 0));
            }
            img.Export(S+"C:/!/CubeFace "+face+" coverage.bmp"); Explore("C:/!"); //Exit(ok ? "ok" : "fail");
         }
#endif

            Vec4 col = 0;
            Flt weight = 0;
            C Byte *src_data = T.src_data + face * src_face_size + rect.min.y * src_pitch;
            for (Int y = rect.min.y; y <= rect.max.y; y++, src_data += src_pitch) {
                Flt dir_y = (linear ? -y * src_CubeFacePixelToDir_mul - src_CubeFacePixelToDir_add
                                    : Tan(-y * src_CubeFacePixelToAngle_mul - src_CubeFacePixelToAngle_add));
                for (Int x = rect.min.x; x <= rect.max.x; x++) {
                    Flt dir_x = (linear ? x * src_CubeFacePixelToDir_mul + src_CubeFacePixelToDir_add
                                        : Tan(x * src_CubeFacePixelToAngle_mul + src_CubeFacePixelToAngle_add));
                    Vec dir_test(dir_x, dir_y, 1);
                    dir_test.normalize();
                    Flt cos = Dot(dir_fn, dir_test);
                    if (cos > cos_min) {
                        Flt a = Acos(cos), w = Weight(a / angle);
                        // FIXME mul 'w' by texel area size
                        CPtr src_data_x = src_data + x * src.bytePP();
                        if (multi_channel)
                            col += w * (BLUR_CUBE_LINEAR_GAMMA ? ImageColorL : ImageColorF)(src_data_x, src.hwType());
                        else
                            col.x += w * (BLUR_CUBE_LINEAR_GAMMA ? ImagePixelL : ImagePixelF)(src_data_x, src.hwType());
                        weight += w;
                    }
                }
            }
            if (check_other_faces) {
                Vec dir;
                CubeFacePosToPos(face, dir, dir_fn); // convert local 'face' space to world space 'dir'
                FREPD(face1, 6)
                if (face1 != face)
                    if (Dot(VecDir[face1], dir) > diag_angle_cos_min) // do a fast check for potential overlap with cone and cube face
                    {
                        RectI rect1;
                        Vec dir_f1;
                        PosToCubeFacePos((DIR_ENUM)face1, dir_f1, dir); // convert world space 'dir' to local 'face1' space 'dir_f1'

#if 0 // full
#pragma message("!! Warning: Use this only for debugging !!")
               rect1.set(0, src_res-1); goto check;
#endif
                        // do a fast check for rotation along Y axis (between ->DIR_FORWARD->DIR_RIGHT->DIR_BACK->DIR_LEFT->) in this case, all Y's are the same as in test above, just have to rotate X's
                        Flt angle_delta;
                        switch (face) {
                        case DIR_FORWARD:
                            angle_delta = 0;
                            break;
                        case DIR_RIGHT:
                            angle_delta = PI_2;
                            break;
                        case DIR_BACK:
                            angle_delta = PI;
                            break;
                        case DIR_LEFT:
                            angle_delta = -PI_2;
                            break;
                        default:
                            goto full;
                        }
                        switch (face1) {
                        case DIR_FORWARD: /*angle_delta-=    0;*/
                            break;
                        case DIR_RIGHT:
                            angle_delta -= PI_2;
                            break;
                        case DIR_BACK:
                            angle_delta -= PI;
                            break;
                        case DIR_LEFT:
                            angle_delta -= -PI_2;
                            break;
                        default:
                            goto full;
                        }
                        if (linear) {
                            zd.set(dir_f1.x, 0, dir_f1.z);
                            len2 = zd.length2();
                            if (ball_r2 >= len2)
                                rect1.setX(0, src_res - 1);
                            else {
                                sin2 = ball_r2 / len2;
                                cos = CosSin2(sin2);
                                d = CrossUp(zd);
                                d.setLength(cos * ball_r);
                                zd *= -sin2;
                                zd += dir_f1;
                                test = zd - d;
                                if (test.z > 0) {
                                    rect1.min.x = CeilSpecial(test.x / test.z * src_DirToCubeFacePixel_mul + src_DirToCubeFacePixel_add);
                                    if (rect1.min.x < 0) {
                                    left_linear_x:
                                        rect1.min.x = 0;
                                    }
                                } else
                                    goto left_linear_x;
                                test = zd + d;
                                if (test.z > 0) {
                                    rect1.max.x = FloorSpecial(test.x / test.z * src_DirToCubeFacePixel_mul + src_DirToCubeFacePixel_add);
                                    if (rect1.max.x >= src_res) {
                                    right_linear_x:
                                        rect1.max.x = src_res - 1;
                                    }
                                } else
                                    goto right_linear_x;
                            }
                        } else {
                            zd.set(dir_f1.x, 0, dir_f1.z);
                            len2 = zd.length2();
                            if (ball_r2 >= len2)
                                rect1.setX(0, src_res - 1);
                            else {
                                sin2 = ball_r2 / len2;
                                cos = CosSin2(sin2);
                                d = CrossUp(zd);
                                d.setLength(cos * ball_r);
                                zd *= -sin2;
                                zd += dir_f1;
                                test = zd - d;
                                if (test.z > 0) {
                                    rect1.min.x = CeilSpecial(Atan(test.x / test.z) * src_AngleToCubeFacePixel_mul + src_AngleToCubeFacePixel_add);
                                    if (rect1.min.x < 0) {
                                    left_sphere_x:
                                        rect1.min.x = 0;
                                    }
                                } else
                                    goto left_sphere_x;
                                test = zd + d;
                                if (test.z > 0) {
                                    rect1.max.x = FloorSpecial(Atan(test.x / test.z) * src_AngleToCubeFacePixel_mul + src_AngleToCubeFacePixel_add);
                                    if (rect1.max.x >= src_res) {
                                    right_sphere_x:
                                        rect1.max.x = src_res - 1;
                                    }
                                } else
                                    goto right_sphere_x;
                            }
                        }
                        if (rect1.validX()) {
                            rect1.setY(rect.min.y, rect.max.y);
                        check:
#if 0 // test rect coverage
#pragma message("!! Warning: Use this only for debugging !!")
                  if(!face)
                  {
                     RectI test_rect=rect1; test_rect.extend(1)&=RectI(0, src_res-1);
                     Vec dir_test;
                     // test top and bottom horizontal neighbor lines
                     Flt dir_y0=(linear ?     -test_rect.min.y*src_CubeFacePixelToDir_mul  -src_CubeFacePixelToDir_add
                                        : Tan(-test_rect.min.y*src_CubeFacePixelToAngle_mul-src_CubeFacePixelToAngle_add));
                     Flt dir_y1=(linear ?     -test_rect.max.y*src_CubeFacePixelToDir_mul  -src_CubeFacePixelToDir_add
                                        : Tan(-test_rect.max.y*src_CubeFacePixelToAngle_mul-src_CubeFacePixelToAngle_add));
                     for(Int tx=test_rect.min.x; tx<=test_rect.max.x; tx++)
                     {
                        Flt dir_x=(linear ?     tx*src_CubeFacePixelToDir_mul  +src_CubeFacePixelToDir_add
                                          : Tan(tx*src_CubeFacePixelToAngle_mul+src_CubeFacePixelToAngle_add));
                        dir_test.set(dir_x, dir_y0, 1); dir_test.normalize(); if(Dot(dir_f1, dir_test)>cos_min && !rect1.includesY(test_rect.min.y))Exit("fail");
                        dir_test.set(dir_x, dir_y1, 1); dir_test.normalize(); if(Dot(dir_f1, dir_test)>cos_min && !rect1.includesY(test_rect.max.y))Exit("fail");
                     }
                     // test left and right vertical neighbor lines
                     Flt dir_x0=(linear ?     test_rect.min.x*src_CubeFacePixelToDir_mul  +src_CubeFacePixelToDir_add
                                        : Tan(test_rect.min.x*src_CubeFacePixelToAngle_mul+src_CubeFacePixelToAngle_add));
                     Flt dir_x1=(linear ?     test_rect.max.x*src_CubeFacePixelToDir_mul  +src_CubeFacePixelToDir_add
                                        : Tan(test_rect.max.x*src_CubeFacePixelToAngle_mul+src_CubeFacePixelToAngle_add));
                     for(Int ty=test_rect.min.y; ty<=test_rect.max.y; ty++)
                     {
                        Flt dir_y=(linear ?     -ty*src_CubeFacePixelToDir_mul  -src_CubeFacePixelToDir_add
                                          : Tan(-ty*src_CubeFacePixelToAngle_mul-src_CubeFacePixelToAngle_add));
                        dir_test.set(dir_x0, dir_y, 1); dir_test.normalize(); if(Dot(dir_f1, dir_test)>cos_min && !rect1.includesX(test_rect.min.x))Exit("fail");
                        dir_test.set(dir_x1, dir_y, 1); dir_test.normalize(); if(Dot(dir_f1, dir_test)>cos_min && !rect1.includesX(test_rect.max.x))Exit("fail");
                     }
                  }
#endif
                            C Byte *src_data = T.src_data + face1 * src_face_size + rect1.min.y * src_pitch;
                            for (Int y = rect1.min.y; y <= rect1.max.y; y++, src_data += src_pitch) {
                                Flt dir_y = (linear ? -y * src_CubeFacePixelToDir_mul - src_CubeFacePixelToDir_add
                                                    : Tan(-y * src_CubeFacePixelToAngle_mul - src_CubeFacePixelToAngle_add));
                                for (Int x = rect1.min.x; x <= rect1.max.x; x++) {
                                    Flt dir_x = (linear ? x * src_CubeFacePixelToDir_mul + src_CubeFacePixelToDir_add
                                                        : Tan(x * src_CubeFacePixelToAngle_mul + src_CubeFacePixelToAngle_add));
                                    Vec dir_test(dir_x, dir_y, 1);
                                    dir_test.normalize();
                                    Flt cos = Dot(dir_f1, dir_test);
                                    if (cos > cos_min) {
                                        Flt a = Acos(cos), w = Weight(a / angle);
                                        // FIXME mul 'w' by texel area size
                                        CPtr src_data_x = src_data + x * src.bytePP();
                                        if (multi_channel)
                                            col += w * (BLUR_CUBE_LINEAR_GAMMA ? ImageColorL : ImageColorF)(src_data_x, src.hwType());
                                        else
                                            col.x += w * (BLUR_CUBE_LINEAR_GAMMA ? ImagePixelL : ImagePixelF)(src_data_x, src.hwType());
                                        weight += w;
                                    }
                                }
                            }
                        }
                        continue;
                    full:
                        if (linear) {
                            zd.set(dir_f1.x, 0, dir_f1.z);
                            len2 = zd.length2();
                            if (ball_r2 >= len2)
                                rect1.setX(0, src_res - 1);
                            else {
                                sin2 = ball_r2 / len2;
                                cos = CosSin2(sin2);
                                d = CrossUp(zd);
                                d.setLength(cos * ball_r);
                                zd *= -sin2;
                                zd += dir_f1;
                                test = zd - d;
                                if (test.z > 0) {
                                    rect1.min.x = CeilSpecial(test.x / test.z * src_DirToCubeFacePixel_mul + src_DirToCubeFacePixel_add);
                                    if (rect1.min.x < 0) {
                                    left_linear_1:
                                        rect1.min.x = 0;
                                    }
                                } else
                                    goto left_linear_1;
                                test = zd + d;
                                if (test.z > 0) {
                                    rect1.max.x = FloorSpecial(test.x / test.z * src_DirToCubeFacePixel_mul + src_DirToCubeFacePixel_add);
                                    if (rect1.max.x >= src_res) {
                                    right_linear_1:
                                        rect1.max.x = src_res - 1;
                                    }
                                } else
                                    goto right_linear_1;
                            }
                            if (!rect1.validX())
                                continue;

                            zd.set(0, dir_f1.y, dir_f1.z);
                            len2 = zd.length2();
                            if (ball_r2 >= len2)
                                rect1.setY(0, src_res - 1);
                            else {
                                sin2 = ball_r2 / len2;
                                cos = CosSin2(sin2);
                                d = CrossRight(zd);
                                d.setLength(cos * ball_r);
                                zd *= -sin2;
                                zd += dir_f1;
                                test = zd - d;
                                if (test.z > 0) {
                                    rect1.min.y = CeilSpecial(-test.y / test.z * src_DirToCubeFacePixel_mul + src_DirToCubeFacePixel_add);
                                    if (rect1.min.y < 0) {
                                    down_linear_1:
                                        rect1.min.y = 0;
                                    }
                                } else
                                    goto down_linear_1;
                                test = zd + d;
                                if (test.z > 0) {
                                    rect1.max.y = FloorSpecial(-test.y / test.z * src_DirToCubeFacePixel_mul + src_DirToCubeFacePixel_add);
                                    if (rect1.max.y >= src_res) {
                                    up_linear_1:
                                        rect1.max.y = src_res - 1;
                                    }
                                } else
                                    goto up_linear_1;
                            }
                            if (!rect1.validY())
                                continue;
                        } else {
                            zd.set(dir_f1.x, 0, dir_f1.z);
                            len2 = zd.length2();
                            if (ball_r2 >= len2)
                                rect1.setX(0, src_res - 1);
                            else {
                                sin2 = ball_r2 / len2;
                                cos = CosSin2(sin2);
                                d = CrossUp(zd);
                                d.setLength(cos * ball_r);
                                zd *= -sin2;
                                zd += dir_f1;
                                test = zd - d;
                                if (test.z > 0) {
                                    rect1.min.x = CeilSpecial(Atan(test.x / test.z) * src_AngleToCubeFacePixel_mul + src_AngleToCubeFacePixel_add);
                                    if (rect1.min.x < 0) {
                                    left_sphere_1:
                                        rect1.min.x = 0;
                                    }
                                } else
                                    goto left_sphere_1;
                                test = zd + d;
                                if (test.z > 0) {
                                    rect1.max.x = FloorSpecial(Atan(test.x / test.z) * src_AngleToCubeFacePixel_mul + src_AngleToCubeFacePixel_add);
                                    if (rect1.max.x >= src_res) {
                                    right_sphere_1:
                                        rect1.max.x = src_res - 1;
                                    }
                                } else
                                    goto right_sphere_1;
                            }
                            if (!rect1.validX())
                                continue;

                            zd.set(0, dir_f1.y, dir_f1.z);
                            len2 = zd.length2();
                            if (ball_r2 >= len2)
                                rect1.setY(0, src_res - 1);
                            else {
                                sin2 = ball_r2 / len2;
                                cos = CosSin2(sin2);
                                d = CrossRight(zd);
                                d.setLength(cos * ball_r);
                                zd *= -sin2;
                                zd += dir_f1;
                                test = zd - d;
                                if (test.z > 0) {
                                    rect1.min.y = CeilSpecial(-Atan(test.y / test.z) * src_AngleToCubeFacePixel_mul + src_AngleToCubeFacePixel_add);
                                    if (rect1.min.y < 0) {
                                    down_sphere_1:
                                        rect1.min.y = 0;
                                    }
                                } else
                                    goto down_sphere_1;
                                test = zd + d;
                                if (test.z > 0) {
                                    rect1.max.y = FloorSpecial(-Atan(test.y / test.z) * src_AngleToCubeFacePixel_mul + src_AngleToCubeFacePixel_add);
                                    if (rect1.max.y >= src_res) {
                                    up_sphere_1:
                                        rect1.max.y = src_res - 1;
                                    }
                                } else
                                    goto up_sphere_1;
                            }
                            if (!rect1.validY())
                                continue;
                        }
                        goto check;
                    }
            }
            if (weight) {
                if (multi_channel)
                    col /= weight;
                else
                    col.x /= weight;
            } else {
                SyncLocker locker(lock);
                LOCK_MODE lock_mode;
                Int lock_mip_map;
                DIR_ENUM lock_cube_face;
                Int lock_count = src.lockCount();
                if (lock_count) // if 'src' is already locked ('src' can be 'dest') then we have to unlock it first (check 'lockCount' instead of 'lockMode', in case 'lockMode' is not set for SOFT)
                {               // remember which mip and face
                    lock_mode = src.lockMode();
                    lock_mip_map = src.lMipMap();
                    lock_cube_face = src.lCubeFace();
                    REP(lock_count)
                    src.unlock();
                }
                if (src.lockRead(src_mip, face)) {
                    Vec2 pix;
                    if (linear)
                        pix.set(dir_f.x * src_DirToCubeFacePixel_mul + src_DirToCubeFacePixel_add, -dir_f.y * src_DirToCubeFacePixel_mul + src_DirToCubeFacePixel_add);
                    // else      pix.set(     dir_angle.x *src_AngleToCubeFacePixel_mul+src_AngleToCubeFacePixel_add, -     dir_angle.y *src_AngleToCubeFacePixel_mul+src_AngleToCubeFacePixel_add);
                    else
                        pix.set(Atan(dir_f.x) * src_AngleToCubeFacePixel_mul + src_AngleToCubeFacePixel_add, -Atan(dir_f.y) * src_AngleToCubeFacePixel_mul + src_AngleToCubeFacePixel_add);

                    if (multi_channel) {
                        if (BLUR_CUBE_LINEAR_GAMMA)
                            col = src.areaColorLLinear(pix, src_area_size);
                        else
                            col = src.areaColorFLinear(pix, src_area_size);
                    } else {
                        if (BLUR_CUBE_LINEAR_GAMMA)
                            col.x = src.pixelLLinear(pix.x, pix.y);
                        else
                            col.x = src.pixelFLinear(pix.x, pix.y);
                    }
                    src.unlock();
                }
                REP(lock_count)
                ConstCast(src).lock(lock_mode, lock_mip_map, lock_cube_face); // restore lock
            }
            if (multi_channel) {
                if (BLUR_CUBE_LINEAR_GAMMA)
                    dest.colorL(x, y, col);
                else
                    dest.colorF(x, y, col);
            } else {
                if (BLUR_CUBE_LINEAR_GAMMA)
                    dest.pixelL(x, y, col.x);
                else
                    dest.pixelF(x, y, col.x);
            }
        }
    }

    BlurCube(C Image &src, Int src_mip, Image &dest, Int dest_mip, Flt angle, Threads *threads, Bool linear) : src(src), dest(dest) {
        if (src.mode() == IMAGE_SOFT_CUBE && dest.cube() && InRange(src_mip, src.mipMaps()) && InRange(dest_mip, dest.mipMaps()) && !src.compressed() && !dest.compressed()) {
            T.linear = linear;
            T.src_mip = src_mip;
            src_data = src.softData(src_mip);
            src_face_size = src.softFaceSize(src_mip);
            src_pitch = src.softPitch(src_mip);
            src_res = Max(1, src.w() >> src_mip);
            dest_res = Max(1, dest.w() >> dest_mip);
            src_area_size = Flt(src_res) / dest_res;
            multi_channel = NeedMultiChannel(src.type(), dest.type());
            // calculate max angle between face direction vector and a point belonging to that face - this will be furthest point, so !Vec(1,1,1) is used and face direction Vec(0,0,1)
            const Flt diag_angle = AcosFast(SQRT3_3), // Flt a=AbsAngleBetween(!Vec(1,1,1), Vec(0,0,1)); AbsAngleBetween(Vec(SQRT3_3, SQRT3_3, SQRT3_3), Vec(0,0,1)); Acos(Dot(Vec(SQRT3_3, SQRT3_3, SQRT3_3), Vec(0,0,1)));
                diag_angle_ext = diag_angle + angle;
            T.angle = angle;
            cos_min = Cos(angle);
            diag_angle_cos_min = Cos(diag_angle_ext);

            // convert cone angle to ball radius
            if (angle >= PI_2 - EPS)
                ball_r = FLT_MAX;
            else {
                // Flt cone_r=Tan(angle), cone_h=1; ball_r=cone_r*cone_h/(Dist(cone_r, cone_h)+cone_r); formula to calculate ball radius inside a cone
                Flt cone_r = Tan(angle);
                ball_r = cone_r / (SqrtFast(Sqr(cone_r) + 1) + cone_r);
                // since ball is located inside the cone, its position will be ball.pos=cone_dir*(1-ball_r); with length=1-ball_r; to normalize ball position (make its length=1 so its radius can be used for normalized direction vectors) we can do ball/=1-ball_r;
                ball_r /= 1 - ball_r;
            }
            ball_r2 = Sqr(ball_r);

            if (linear) {
                src_DirToCubeFacePixel_mul = src_res * 0.5f;
                src_DirToCubeFacePixel_add = src_DirToCubeFacePixel_mul - 0.5f;
                Flt inv_res = 1.0f / src_res;
                src_CubeFacePixelToDir_mul = 2 * inv_res;
                src_CubeFacePixelToDir_add = inv_res - 1;
                inv_res = 1.0f / dest_res;
                dest_CubeFacePixelToDir_mul = 2 * inv_res;
                dest_CubeFacePixelToDir_add = inv_res - 1;
            } else {
                src_AngleToCubeFacePixel_mul = src_res / PI_2;
                src_AngleToCubeFacePixel_add = PI_4 * src_AngleToCubeFacePixel_mul - 0.5f;
                src_CubeFacePixelToAngle_mul = PI_2 / src_res;
                src_CubeFacePixelToAngle_add = -PI_4 + src_CubeFacePixelToAngle_mul / 2;
                dest_CubeFacePixelToAngle_mul = PI_2 / dest_res;
                dest_CubeFacePixelToAngle_add = -PI_4 + dest_CubeFacePixelToAngle_mul / 2;
            }
            for (face = DIR_ENUM(0); face < 6; face = DIR_ENUM(face + 1)) {
                if (dest.lock(LOCK_WRITE, dest_mip, face)) {
                    if (threads)
                        threads->process1(dest_res, ProcessLine, T);
                    else
                        REP(dest_res)
                    processLine(i);
                    dest.unlock();
                }
            }
        }
    }
};
/******************************************************************************/
struct BlurContext {
    typedef void (BlurContext::*Blur)(Int x, Int y, Int z) C; // pointer to BlurContext method

    Bool high_prec, clamp;
    Byte func;
    Int rangei, rangei_2_1;
    Flt range;
    VecI size;
    C Image *src;
    Image *dest;
    Threads *threads;
    MemtN<Byte, 256> weight_b;
    MemtN<Flt, 256> weight_f;
    Blur blur_ptr;

    BlurContext(C Image &src, Bool clamp) {
        if (T.high_prec = src.highPrecision()) {
            if (src.hwType() == IMAGE_F32)
                func = 0;
            else // HP 1F, here check 'hwType' because we will access memory directly using 'pixF' method
                if (src.typeChannels() == 1)
                func = 1;
            else          // HP 1C
                func = 2; // HP MC
        } else {
            C ImageTypeInfo &ti = src.hwTypeInfo(); // here check 'hwType' because we will access memory directly using 'pixB' method
            if (ti.bit_pp == 8 && ti.channels == 1)
                func = 3; // LP 1byte
            else
                func = 4; // LP MC
        }
        T.clamp = clamp;
        T.size = src.size3();
        T.rangei = 0;
        T.rangei_2_1 = 1;
        T.range = 0;
        T.threads = &ImageThreads.init();
        T.src = &src;
    }
    void setRange(Flt range) // this is used for blur
    {
        MAX(range, 0);
        if (T.range != range) {
            T.range = range;
            T.rangei = Ceil(range);
            T.rangei_2_1 = rangei * 2 + 1;
            Flt range_1 = range + 1;
            if (high_prec) {
                weight_f.setNum(rangei_2_1);
                REPAO(weight_f) = BlendSmoothCube(Flt(i - rangei) / range_1);
            } else {
                weight_b.setNum(rangei_2_1);
                REPAO(weight_b) = RoundU(255 * BlendSmoothCube(Flt(i - rangei) / range_1));
            }
        }
    }
    void setRange(Int range) // this is used for average
    {
        T.rangei = range;
        T.rangei_2_1 = rangei * 2 + 1;
    }

    void setX() {
        switch (func) {
        case 0:
            blur_ptr = clamp ? &BlurContext::blur_X_HP_1F_CLAMP : &BlurContext::blur_X_HP_1F_WRAP;
            break;
        case 1:
            blur_ptr = clamp ? &BlurContext::blur_X_HP_1C_CLAMP : &BlurContext::blur_X_HP_1C_WRAP;
            break;
        case 2:
            blur_ptr = clamp ? &BlurContext::blur_X_HP_MC_CLAMP : &BlurContext::blur_X_HP_MC_WRAP;
            break;
        case 3:
            blur_ptr = clamp ? &BlurContext::blur_X_LP_1B_CLAMP : &BlurContext::blur_X_LP_1B_WRAP;
            break;
        case 4:
            blur_ptr = clamp ? &BlurContext::blur_X_LP_MC_CLAMP : &BlurContext::blur_X_LP_MC_WRAP;
            break;
        }
    }
    void setY() {
        switch (func) {
        case 0:
            blur_ptr = clamp ? &BlurContext::blur_Y_HP_1F_CLAMP : &BlurContext::blur_Y_HP_1F_WRAP;
            break;
        case 1:
            blur_ptr = clamp ? &BlurContext::blur_Y_HP_1C_CLAMP : &BlurContext::blur_Y_HP_1C_WRAP;
            break;
        case 2:
            blur_ptr = clamp ? &BlurContext::blur_Y_HP_MC_CLAMP : &BlurContext::blur_Y_HP_MC_WRAP;
            break;
        case 3:
            blur_ptr = clamp ? &BlurContext::blur_Y_LP_1B_CLAMP : &BlurContext::blur_Y_LP_1B_WRAP;
            break;
        case 4:
            blur_ptr = clamp ? &BlurContext::blur_Y_LP_MC_CLAMP : &BlurContext::blur_Y_LP_MC_WRAP;
            break;
        }
    }
    void setZ() {
        switch (func) {
        case 0:
            blur_ptr = clamp ? &BlurContext::blur_Z_HP_1F_CLAMP : &BlurContext::blur_Z_HP_1F_WRAP;
            break;
        case 1:
            blur_ptr = clamp ? &BlurContext::blur_Z_HP_1C_CLAMP : &BlurContext::blur_Z_HP_1C_WRAP;
            break;
        case 2:
            blur_ptr = clamp ? &BlurContext::blur_Z_HP_MC_CLAMP : &BlurContext::blur_Z_HP_MC_WRAP;
            break;
        case 3:
            blur_ptr = clamp ? &BlurContext::blur_Z_LP_1B_CLAMP : &BlurContext::blur_Z_LP_1B_WRAP;
            break;
        case 4:
            blur_ptr = clamp ? &BlurContext::blur_Z_LP_MC_CLAMP : &BlurContext::blur_Z_LP_MC_WRAP;
            break;
        }
    }

    void setAvgX() {
        switch (func) {
        case 0:
            blur_ptr = clamp ? &BlurContext::avg_X_HP_1F_CLAMP : &BlurContext::avg_X_HP_1F_WRAP;
            break;
        case 1:
            blur_ptr = clamp ? &BlurContext::avg_X_HP_1C_CLAMP : &BlurContext::avg_X_HP_1C_WRAP;
            break;
        case 2:
            blur_ptr = clamp ? &BlurContext::avg_X_HP_MC_CLAMP : &BlurContext::avg_X_HP_MC_WRAP;
            break;
        case 3:
            blur_ptr = clamp ? &BlurContext::avg_X_LP_1B_CLAMP : &BlurContext::avg_X_LP_1B_WRAP;
            break;
        case 4:
            blur_ptr = clamp ? &BlurContext::avg_X_LP_MC_CLAMP : &BlurContext::avg_X_LP_MC_WRAP;
            break;
        }
    }
    void setAvgY() {
        switch (func) {
        case 0:
            blur_ptr = clamp ? &BlurContext::avg_Y_HP_1F_CLAMP : &BlurContext::avg_Y_HP_1F_WRAP;
            break;
        case 1:
            blur_ptr = clamp ? &BlurContext::avg_Y_HP_1C_CLAMP : &BlurContext::avg_Y_HP_1C_WRAP;
            break;
        case 2:
            blur_ptr = clamp ? &BlurContext::avg_Y_HP_MC_CLAMP : &BlurContext::avg_Y_HP_MC_WRAP;
            break;
        case 3:
            blur_ptr = clamp ? &BlurContext::avg_Y_LP_1B_CLAMP : &BlurContext::avg_Y_LP_1B_WRAP;
            break;
        case 4:
            blur_ptr = clamp ? &BlurContext::avg_Y_LP_MC_CLAMP : &BlurContext::avg_Y_LP_MC_WRAP;
            break;
        }
    }
    void setAvgZ() {
        switch (func) {
        case 0:
            blur_ptr = clamp ? &BlurContext::avg_Z_HP_1F_CLAMP : &BlurContext::avg_Z_HP_1F_WRAP;
            break;
        case 1:
            blur_ptr = clamp ? &BlurContext::avg_Z_HP_1C_CLAMP : &BlurContext::avg_Z_HP_1C_WRAP;
            break;
        case 2:
            blur_ptr = clamp ? &BlurContext::avg_Z_HP_MC_CLAMP : &BlurContext::avg_Z_HP_MC_WRAP;
            break;
        case 3:
            blur_ptr = clamp ? &BlurContext::avg_Z_LP_1B_CLAMP : &BlurContext::avg_Z_LP_1B_WRAP;
            break;
        case 4:
            blur_ptr = clamp ? &BlurContext::avg_Z_LP_MC_CLAMP : &BlurContext::avg_Z_LP_MC_WRAP;
            break;
        }
    }

    inline void blur(Int x, Int y, Int z) C { (T.*blur_ptr)(x, y, z); }

    static void BlurFunc(IntPtr elm_index, BlurContext &bc, Int thread_index) {
        Int z = Unsigned(elm_index) / Unsigned(bc.size.y),
            y = Unsigned(elm_index) % Unsigned(bc.size.y);
        REPD(x, bc.size.x)
        bc.blur(x, y, z);
    }

    void blur() {
        if (threads)
            threads->process1(size.z * size.y, BlurFunc, T);
        else
            REPD(z, size.z)
        REPD(y, size.y)
        REPD(x, size.x)
        blur(x, y, z);
    }

    // X
    void blur_X_HP_1F_CLAMP(Int x, Int y, Int z) C {
        Flt color = 0, power = 0;
        Int i = 0, min = x - rangei, max = Min(x + rangei, size.x - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        C Flt *data = &src->pixF(0, y, z);
        for (; min <= max; min++, i++) {
            Flt c = data[min], w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->pixF(x, y, z) = color;
    }
    void blur_X_HP_1F_WRAP(Int x, Int y, Int z) C {
        Flt color = 0, power = 0;
        C Flt *data = &src->pixF(0, y, z);
        REP(rangei_2_1) {
            Flt c = data[Mod(x + i - rangei, size.x)], w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->pixF(x, y, z) = color;
    }
    void blur_X_HP_1C_CLAMP(Int x, Int y, Int z) C {
        Flt color = 0, power = 0;
        Int i = 0, min = x - rangei, max = Min(x + rangei, size.x - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        for (; min <= max; min++, i++) {
            Flt c = src->pixel3DF(min, y, z), w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->pixel3DF(x, y, z, color);
    }
    void blur_X_HP_1C_WRAP(Int x, Int y, Int z) C {
        Flt color = 0, power = 0;
        REP(rangei_2_1) {
            Flt c = src->pixel3DF(Mod(x + i - rangei, size.x), y, z), w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->pixel3DF(x, y, z, color);
    }
    void blur_X_HP_MC_CLAMP(Int x, Int y, Int z) C {
        Vec4 color = 0;
        Flt power = 0;
        Int i = 0, min = x - rangei, max = Min(x + rangei, size.x - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        for (; min <= max; min++, i++) {
            Vec4 c = src->color3DF(min, y, z);
            Flt w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->color3DF(x, y, z, color);
    }
    void blur_X_HP_MC_WRAP(Int x, Int y, Int z) C {
        Vec4 color = 0;
        Flt power = 0;
        REP(rangei_2_1) {
            Vec4 c = src->color3DF(Mod(x + i - rangei, size.x), y, z);
            Flt w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->color3DF(x, y, z, color);
    }
    void blur_X_LP_1B_CLAMP(Int x, Int y, Int z) C {
        UInt value = 0, power = 0;
        Int i = 0, min = x - rangei, max = Min(x + rangei, size.x - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        C Byte *data = &src->pixB(0, y, z);
        for (; min <= max; min++, i++) {
            Byte c = data[min], w = weight_b[i];
            value += w * c;
            power += w;
        }
        if (power) {
            UInt p_2 = (power >> 1);
            value = (value + p_2) / power;
        }
        dest->pixB(x, y, z) = value;
    }
    void blur_X_LP_1B_WRAP(Int x, Int y, Int z) C {
        UInt value = 0, power = 0;
        C Byte *data = &src->pixB(0, y, z);
        REP(rangei_2_1) {
            Byte c = data[Mod(x + i - rangei, size.x)], w = weight_b[i];
            value += w * c;
            power += w;
        }
        if (power) {
            UInt p_2 = (power >> 1);
            value = (value + p_2) / power;
        }
        dest->pixB(x, y, z) = value;
    }
    void blur_X_LP_MC_CLAMP(Int x, Int y, Int z) C {
        UInt r = 0, g = 0, b = 0, a = 0, power = 0;
        Int i = 0, min = x - rangei, max = Min(x + rangei, size.x - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        for (; min <= max; min++, i++) {
            Color c = src->color3D(min, y, z);
            Byte w = weight_b[i];
            r += c.r * w;
            g += c.g * w;
            b += c.b * w;
            a += c.a * w;
            power += w;
        }
        if (power) {
            UInt p_2 = (power >> 1);
            r = (r + p_2) / power;
            g = (g + p_2) / power;
            b = (b + p_2) / power;
            a = (a + p_2) / power;
        }
        dest->color3D(x, y, z, Color(r, g, b, a));
    }
    void blur_X_LP_MC_WRAP(Int x, Int y, Int z) C {
        UInt r = 0, g = 0, b = 0, a = 0, power = 0;
        REP(rangei_2_1) {
            Color c = src->color3D(Mod(x + i - rangei, size.x), y, z);
            Byte w = weight_b[i];
            r += c.r * w;
            g += c.g * w;
            b += c.b * w;
            a += c.a * w;
            power += w;
        }
        if (power) {
            UInt p_2 = (power >> 1);
            r = (r + p_2) / power;
            g = (g + p_2) / power;
            b = (b + p_2) / power;
            a = (a + p_2) / power;
        }
        dest->color3D(x, y, z, Color(r, g, b, a));
    }

    // Y
    void blur_Y_HP_1F_CLAMP(Int x, Int y, Int z) C {
        Flt color = 0, power = 0;
        Int i = 0, min = y - rangei, max = Min(y + rangei, size.y - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt pitch = src->pitch();
        C Flt *data = &src->pixF(x, 0, z);
        for (; min <= max; min++, i++) {
            Flt c = *(Flt *)((Byte *)data + min * pitch), w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->pixF(x, y, z) = color;
    }
    void blur_Y_HP_1F_WRAP(Int x, Int y, Int z) C {
        Flt color = 0, power = 0;
        UInt pitch = src->pitch();
        C Flt *data = &src->pixF(x, 0, z);
        REP(rangei_2_1) {
            Flt c = *(Flt *)((Byte *)data + Mod(y + i - rangei, size.y) * pitch), w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->pixF(x, y, z) = color;
    }
    void blur_Y_HP_1C_CLAMP(Int x, Int y, Int z) C {
        Flt color = 0, power = 0;
        Int i = 0, min = y - rangei, max = Min(y + rangei, size.y - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        for (; min <= max; min++, i++) {
            Flt c = src->pixel3DF(x, min, z), w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->pixel3DF(x, y, z, color);
    }
    void blur_Y_HP_1C_WRAP(Int x, Int y, Int z) C {
        Flt color = 0, power = 0;
        REP(rangei_2_1) {
            Flt c = src->pixel3DF(x, Mod(y + i - rangei, size.y), z), w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->pixel3DF(x, y, z, color);
    }
    void blur_Y_HP_MC_CLAMP(Int x, Int y, Int z) C {
        Vec4 color = 0;
        Flt power = 0;
        Int i = 0, min = y - rangei, max = Min(y + rangei, size.y - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        for (; min <= max; min++, i++) {
            Vec4 c = src->color3DF(x, min, z);
            Flt w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->color3DF(x, y, z, color);
    }
    void blur_Y_HP_MC_WRAP(Int x, Int y, Int z) C {
        Vec4 color = 0;
        Flt power = 0;
        REP(rangei_2_1) {
            Vec4 c = src->color3DF(x, Mod(y + i - rangei, size.y), z);
            Flt w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->color3DF(x, y, z, color);
    }
    void blur_Y_LP_1B_CLAMP(Int x, Int y, Int z) C {
        UInt value = 0, power = 0, pitch = src->pitch();
        Int i = 0, min = y - rangei, max = Min(y + rangei, size.y - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        C Byte *data = &src->pixB(x, 0, z);
        for (; min <= max; min++, i++) {
            Byte c = data[min * pitch], w = weight_b[i];
            value += w * c;
            power += w;
        }
        if (power) {
            UInt p_2 = (power >> 1);
            value = (value + p_2) / power;
        }
        dest->pixB(x, y, z) = value;
    }
    void blur_Y_LP_1B_WRAP(Int x, Int y, Int z) C {
        UInt value = 0, power = 0, pitch = src->pitch();
        C Byte *data = &src->pixB(x, 0, z);
        REP(rangei_2_1) {
            Byte c = data[Mod(y + i - rangei, size.y) * pitch], w = weight_b[i];
            value += w * c;
            power += w;
        }
        if (power) {
            UInt p_2 = (power >> 1);
            value = (value + p_2) / power;
        }
        dest->pixB(x, y, z) = value;
    }
    void blur_Y_LP_MC_CLAMP(Int x, Int y, Int z) C {
        UInt r = 0, g = 0, b = 0, a = 0, power = 0;
        Int i = 0, min = y - rangei, max = Min(y + rangei, size.y - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        for (; min <= max; min++, i++) {
            Color c = src->color3D(x, min, z);
            Byte w = weight_b[i];
            r += c.r * w;
            g += c.g * w;
            b += c.b * w;
            a += c.a * w;
            power += w;
        }
        if (power) {
            UInt p_2 = (power >> 1);
            r = (r + p_2) / power;
            g = (g + p_2) / power;
            b = (b + p_2) / power;
            a = (a + p_2) / power;
        }
        dest->color3D(x, y, z, Color(r, g, b, a));
    }
    void blur_Y_LP_MC_WRAP(Int x, Int y, Int z) C {
        UInt r = 0, g = 0, b = 0, a = 0, power = 0;
        REP(rangei_2_1) {
            Color c = src->color3D(x, Mod(y + i - rangei, size.y), z);
            Byte w = weight_b[i];
            r += c.r * w;
            g += c.g * w;
            b += c.b * w;
            a += c.a * w;
            power += w;
        }
        if (power) {
            UInt p_2 = (power >> 1);
            r = (r + p_2) / power;
            g = (g + p_2) / power;
            b = (b + p_2) / power;
            a = (a + p_2) / power;
        }
        dest->color3D(x, y, z, Color(r, g, b, a));
    }

    // Z
    void blur_Z_HP_1F_CLAMP(Int x, Int y, Int z) C {
        Flt color = 0, power = 0;
        Int i = 0, min = z - rangei, max = Min(z + rangei, size.z - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt pitch = src->pitch2();
        C Flt *data = &src->pixF(x, y, 0);
        for (; min <= max; min++, i++) {
            Flt c = *(Flt *)((Byte *)data + min * pitch), w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->pixF(x, y, z) = color;
    }
    void blur_Z_HP_1F_WRAP(Int x, Int y, Int z) C {
        Flt color = 0, power = 0;
        UInt pitch = src->pitch2();
        C Flt *data = &src->pixF(x, y, 0);
        REP(rangei_2_1) {
            Flt c = *(Flt *)((Byte *)data + Mod(z + i - rangei, size.z) * pitch), w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->pixF(x, y, z) = color;
    }
    void blur_Z_HP_1C_CLAMP(Int x, Int y, Int z) C {
        Flt color = 0, power = 0;
        Int i = 0, min = z - rangei, max = Min(z + rangei, size.z - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        for (; min <= max; min++, i++) {
            Flt c = src->pixel3DF(x, y, min), w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->pixel3DF(x, y, z, color);
    }
    void blur_Z_HP_1C_WRAP(Int x, Int y, Int z) C {
        Flt color = 0, power = 0;
        REP(rangei_2_1) {
            Flt c = src->pixel3DF(x, y, Mod(z + i - rangei, size.z)), w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->pixel3DF(x, y, z, color);
    }
    void blur_Z_HP_MC_CLAMP(Int x, Int y, Int z) C {
        Vec4 color = 0;
        Flt power = 0;
        Int i = 0, min = z - rangei, max = Min(z + rangei, size.z - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        for (; min <= max; min++, i++) {
            Vec4 c = src->color3DF(x, y, min);
            Flt w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->color3DF(x, y, z, color);
    }
    void blur_Z_HP_MC_WRAP(Int x, Int y, Int z) C {
        Vec4 color = 0;
        Flt power = 0;
        REP(rangei_2_1) {
            Vec4 c = src->color3DF(x, y, Mod(z + i - rangei, size.z));
            Flt w = weight_f[i];
            color += w * c;
            power += w;
        }
        if (power)
            color /= power;
        dest->color3DF(x, y, z, color);
    }
    void blur_Z_LP_1B_CLAMP(Int x, Int y, Int z) C {
        UInt value = 0, power = 0, pitch = src->pitch2();
        Int i = 0, min = z - rangei, max = Min(z + rangei, size.z - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        C Byte *data = &src->pixB(x, y, 0);
        for (; min <= max; min++, i++) {
            Byte c = data[min * pitch], w = weight_b[i];
            value += w * c;
            power += w;
        }
        if (power) {
            UInt p_2 = (power >> 1);
            value = (value + p_2) / power;
        }
        dest->pixB(x, y, z) = value;
    }
    void blur_Z_LP_1B_WRAP(Int x, Int y, Int z) C {
        UInt value = 0, power = 0, pitch = src->pitch2();
        C Byte *data = &src->pixB(x, y, 0);
        REP(rangei_2_1) {
            Byte c = data[Mod(z + i - rangei, size.z) * pitch], w = weight_b[i];
            value += w * c;
            power += w;
        }
        if (power) {
            UInt p_2 = (power >> 1);
            value = (value + p_2) / power;
        }
        dest->pixB(x, y, z) = value;
    }
    void blur_Z_LP_MC_CLAMP(Int x, Int y, Int z) C {
        UInt r = 0, g = 0, b = 0, a = 0, power = 0;
        Int i = 0, min = z - rangei, max = Min(z + rangei, size.z - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        for (; min <= max; min++, i++) {
            Color c = src->color3D(x, y, min);
            Byte w = weight_b[i];
            r += c.r * w;
            g += c.g * w;
            b += c.b * w;
            a += c.a * w;
            power += w;
        }
        if (power) {
            UInt p_2 = (power >> 1);
            r = (r + p_2) / power;
            g = (g + p_2) / power;
            b = (b + p_2) / power;
            a = (a + p_2) / power;
        }
        dest->color3D(x, y, z, Color(r, g, b, a));
    }
    void blur_Z_LP_MC_WRAP(Int x, Int y, Int z) C {
        UInt r = 0, g = 0, b = 0, a = 0, power = 0;
        REP(rangei_2_1) {
            Color c = src->color3D(x, y, Mod(z + i - rangei, size.z));
            Byte w = weight_b[i];
            r += c.r * w;
            g += c.g * w;
            b += c.b * w;
            a += c.a * w;
            power += w;
        }
        if (power) {
            UInt p_2 = (power >> 1);
            r = (r + p_2) / power;
            g = (g + p_2) / power;
            b = (b + p_2) / power;
            a = (a + p_2) / power;
        }
        dest->color3D(x, y, z, Color(r, g, b, a));
    }

    // AVERAGE

    // X
    void avg_X_HP_1F_CLAMP(Int x, Int y, Int z) C {
        Flt color = 0;
        Int i = 0, min = x - rangei, max = Min(x + rangei, size.x - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1;
        C Flt *data = &src->pixF(0, y, z);
        for (; min <= max; min++, i++)
            color += data[min];
        color /= power; // always non-zero
        dest->pixF(x, y, z) = color;
    }
    void avg_X_HP_1F_WRAP(Int x, Int y, Int z) C {
        Flt color = 0;
        C Flt *data = &src->pixF(0, y, z);
        REP(rangei_2_1)
        color += data[Mod(x + i - rangei, size.x)];
        color /= rangei_2_1; // always non-zero
        dest->pixF(x, y, z) = color;
    }
    void avg_X_HP_1C_CLAMP(Int x, Int y, Int z) C {
        Flt color = 0;
        Int i = 0, min = x - rangei, max = Min(x + rangei, size.x - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1;
        for (; min <= max; min++, i++)
            color += src->pixel3DF(min, y, z);
        color /= power; // always non-zero
        dest->pixel3DF(x, y, z, color);
    }
    void avg_X_HP_1C_WRAP(Int x, Int y, Int z) C {
        Flt color = 0;
        REP(rangei_2_1)
        color += src->pixel3DF(Mod(x + i - rangei, size.x), y, z);
        color /= rangei_2_1; // always non-zero
        dest->pixel3DF(x, y, z, color);
    }
    void avg_X_HP_MC_CLAMP(Int x, Int y, Int z) C {
        Vec4 color = 0;
        Int i = 0, min = x - rangei, max = Min(x + rangei, size.x - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1;
        for (; min <= max; min++, i++)
            color += src->color3DF(min, y, z);
        color /= power; // always non-zero
        dest->color3DF(x, y, z, color);
    }
    void avg_X_HP_MC_WRAP(Int x, Int y, Int z) C {
        Vec4 color = 0;
        REP(rangei_2_1)
        color += src->color3DF(Mod(x + i - rangei, size.x), y, z);
        color /= rangei_2_1; // always non-zero
        dest->color3DF(x, y, z, color);
    }
    void avg_X_LP_1B_CLAMP(Int x, Int y, Int z) C {
        UInt value = 0;
        Int i = 0, min = x - rangei, max = Min(x + rangei, size.x - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1;
        C Byte *data = &src->pixB(0, y, z);
        for (; min <= max; min++, i++)
            value += data[min];
        UInt p_2 = (power >> 1);
        value = (value + p_2) / power; // always non-zero
        dest->pixB(x, y, z) = value;
    }
    void avg_X_LP_1B_WRAP(Int x, Int y, Int z) C {
        UInt value = 0;
        C Byte *data = &src->pixB(0, y, z);
        REP(rangei_2_1)
        value += data[Mod(x + i - rangei, size.x)];
        UInt p_2 = (rangei_2_1 >> 1);
        value = (value + p_2) / rangei_2_1; // always non-zero
        dest->pixB(x, y, z) = value;
    }
    void avg_X_LP_MC_CLAMP(Int x, Int y, Int z) C {
        UInt r = 0, g = 0, b = 0, a = 0;
        Int i = 0, min = x - rangei, max = Min(x + rangei, size.x - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1;
        for (; min <= max; min++, i++) {
            Color c = src->color3D(min, y, z);
            r += c.r;
            g += c.g;
            b += c.b;
            a += c.a;
        }
        UInt p_2 = (power >> 1);
        r = (r + p_2) / power;
        g = (g + p_2) / power;
        b = (b + p_2) / power;
        a = (a + p_2) / power; // always non-zero
        dest->color3D(x, y, z, Color(r, g, b, a));
    }
    void avg_X_LP_MC_WRAP(Int x, Int y, Int z) C {
        UInt r = 0, g = 0, b = 0, a = 0;
        REP(rangei_2_1) {
            Color c = src->color3D(Mod(x + i - rangei, size.x), y, z);
            r += c.r;
            g += c.g;
            b += c.b;
            a += c.a;
        }
        UInt p_2 = (rangei_2_1 >> 1);
        r = (r + p_2) / rangei_2_1;
        g = (g + p_2) / rangei_2_1;
        b = (b + p_2) / rangei_2_1;
        a = (a + p_2) / rangei_2_1; // always non-zero
        dest->color3D(x, y, z, Color(r, g, b, a));
    }

    // Y
    void avg_Y_HP_1F_CLAMP(Int x, Int y, Int z) C {
        Flt color = 0;
        Int i = 0, min = y - rangei, max = Min(y + rangei, size.y - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1, pitch = src->pitch();
        C Flt *data = &src->pixF(x, 0, z);
        for (; min <= max; min++, i++)
            color += *(Flt *)((Byte *)data + min * pitch);
        color /= power; // always non-zero
        dest->pixF(x, y, z) = color;
    }
    void avg_Y_HP_1F_WRAP(Int x, Int y, Int z) C {
        Flt color = 0;
        UInt pitch = src->pitch();
        C Flt *data = &src->pixF(x, 0, z);
        REP(rangei_2_1)
        color += *(Flt *)((Byte *)data + Mod(y + i - rangei, size.y) * pitch);
        color /= rangei_2_1; // always non-zero
        dest->pixF(x, y, z) = color;
    }
    void avg_Y_HP_1C_CLAMP(Int x, Int y, Int z) C {
        Flt color = 0;
        Int i = 0, min = y - rangei, max = Min(y + rangei, size.y - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1;
        for (; min <= max; min++, i++)
            color += src->pixel3DF(x, min, z);
        color /= power; // always non-zero
        dest->pixel3DF(x, y, z, color);
    }
    void avg_Y_HP_1C_WRAP(Int x, Int y, Int z) C {
        Flt color = 0;
        REP(rangei_2_1)
        color += src->pixel3DF(x, Mod(y + i - rangei, size.y), z);
        color /= rangei_2_1; // always non-zero
        dest->pixel3DF(x, y, z, color);
    }
    void avg_Y_HP_MC_CLAMP(Int x, Int y, Int z) C {
        Vec4 color = 0;
        Int i = 0, min = y - rangei, max = Min(y + rangei, size.y - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1;
        for (; min <= max; min++, i++)
            color += src->color3DF(x, min, z);
        color /= power; // always non-zero
        dest->color3DF(x, y, z, color);
    }
    void avg_Y_HP_MC_WRAP(Int x, Int y, Int z) C {
        Vec4 color = 0;
        REP(rangei_2_1)
        color += src->color3DF(x, Mod(y + i - rangei, size.y), z);
        color /= rangei_2_1; // always non-zero
        dest->color3DF(x, y, z, color);
    }
    void avg_Y_LP_1B_CLAMP(Int x, Int y, Int z) C {
        UInt value = 0, pitch = src->pitch();
        Int i = 0, min = y - rangei, max = Min(y + rangei, size.y - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1;
        C Byte *data = &src->pixB(x, 0, z);
        for (; min <= max; min++, i++)
            value += data[min * pitch];
        UInt p_2 = (power >> 1);
        value = (value + p_2) / power; // always non-zero
        dest->pixB(x, y, z) = value;
    }
    void avg_Y_LP_1B_WRAP(Int x, Int y, Int z) C {
        UInt value = 0, pitch = src->pitch();
        C Byte *data = &src->pixB(x, 0, z);
        REP(rangei_2_1)
        value += data[Mod(y + i - rangei, size.y) * pitch];
        UInt p_2 = (rangei_2_1 >> 1);
        value = (value + p_2) / rangei_2_1; // always non-zero
        dest->pixB(x, y, z) = value;
    }
    void avg_Y_LP_MC_CLAMP(Int x, Int y, Int z) C {
        UInt r = 0, g = 0, b = 0, a = 0;
        Int i = 0, min = y - rangei, max = Min(y + rangei, size.y - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1;
        for (; min <= max; min++, i++) {
            Color c = src->color3D(x, min, z);
            r += c.r;
            g += c.g;
            b += c.b;
            a += c.a;
        }
        UInt p_2 = (power >> 1);
        r = (r + p_2) / power;
        g = (g + p_2) / power;
        b = (b + p_2) / power;
        a = (a + p_2) / power; // always non-zero
        dest->color3D(x, y, z, Color(r, g, b, a));
    }
    void avg_Y_LP_MC_WRAP(Int x, Int y, Int z) C {
        UInt r = 0, g = 0, b = 0, a = 0;
        REP(rangei_2_1) {
            Color c = src->color3D(x, Mod(y + i - rangei, size.y), z);
            r += c.r;
            g += c.g;
            b += c.b;
            a += c.a;
        }
        UInt p_2 = (rangei_2_1 >> 1);
        r = (r + p_2) / rangei_2_1;
        g = (g + p_2) / rangei_2_1;
        b = (b + p_2) / rangei_2_1;
        a = (a + p_2) / rangei_2_1; // always non-zero
        dest->color3D(x, y, z, Color(r, g, b, a));
    }

    // Z
    void avg_Z_HP_1F_CLAMP(Int x, Int y, Int z) C {
        Flt color = 0;
        Int i = 0, min = z - rangei, max = Min(z + rangei, size.z - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1, pitch = src->pitch2();
        C Flt *data = &src->pixF(x, y, 0);
        for (; min <= max; min++, i++)
            color += *(Flt *)((Byte *)data + min * pitch);
        color /= power; // always non-zero
        dest->pixF(x, y, z) = color;
    }
    void avg_Z_HP_1F_WRAP(Int x, Int y, Int z) C {
        Flt color = 0;
        UInt pitch = src->pitch2();
        C Flt *data = &src->pixF(x, y, 0);
        REP(rangei_2_1)
        color += *(Flt *)((Byte *)data + Mod(z + i - rangei, size.z) * pitch);
        color /= rangei_2_1; // always non-zero
        dest->pixF(x, y, z) = color;
    }
    void avg_Z_HP_1C_CLAMP(Int x, Int y, Int z) C {
        Flt color = 0;
        Int i = 0, min = z - rangei, max = Min(z + rangei, size.z - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1;
        for (; min <= max; min++, i++)
            color += src->pixel3DF(x, y, min);
        color /= power; // always non-zero
        dest->pixel3DF(x, y, z, color);
    }
    void avg_Z_HP_1C_WRAP(Int x, Int y, Int z) C {
        Flt color = 0;
        REP(rangei_2_1)
        color += src->pixel3DF(x, y, Mod(z + i - rangei, size.z));
        color /= rangei_2_1; // always non-zero
        dest->pixel3DF(x, y, z, color);
    }
    void avg_Z_HP_MC_CLAMP(Int x, Int y, Int z) C {
        Vec4 color = 0;
        Int i = 0, min = z - rangei, max = Min(z + rangei, size.z - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1;
        for (; min <= max; min++, i++)
            color += src->color3DF(x, y, min);
        color /= power; // always non-zero
        dest->color3DF(x, y, z, color);
    }
    void avg_Z_HP_MC_WRAP(Int x, Int y, Int z) C {
        Vec4 color = 0;
        REP(rangei_2_1)
        color += src->color3DF(x, y, Mod(z + i - rangei, size.z));
        color /= rangei_2_1; // always non-zero
        dest->color3DF(x, y, z, color);
    }
    void avg_Z_LP_1B_CLAMP(Int x, Int y, Int z) C {
        UInt value = 0, pitch = src->pitch2();
        Int i = 0, min = z - rangei, max = Min(z + rangei, size.z - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1;
        C Byte *data = &src->pixB(x, y, 0);
        for (; min <= max; min++, i++)
            value += data[min * pitch];
        UInt p_2 = (power >> 1);
        value = (value + p_2) / power; // always non-zero
        dest->pixB(x, y, z) = value;
    }
    void avg_Z_LP_1B_WRAP(Int x, Int y, Int z) C {
        UInt value = 0, pitch = src->pitch2();
        C Byte *data = &src->pixB(x, y, 0);
        REP(rangei_2_1)
        value += data[Mod(z + i - rangei, size.z) * pitch];
        UInt p_2 = (rangei_2_1 >> 1);
        value = (value + p_2) / rangei_2_1; // always non-zero
        dest->pixB(x, y, z) = value;
    }
    void avg_Z_LP_MC_CLAMP(Int x, Int y, Int z) C {
        UInt r = 0, g = 0, b = 0, a = 0;
        Int i = 0, min = z - rangei, max = Min(z + rangei, size.z - 1);
        if (min < 0) {
            i = -min;
            min = 0;
        }
        UInt power = max - min + 1;
        for (; min <= max; min++, i++) {
            Color c = src->color3D(x, y, min);
            r += c.r;
            g += c.g;
            b += c.b;
            a += c.a;
        }
        UInt p_2 = (power >> 1);
        r = (r + p_2) / power;
        g = (g + p_2) / power;
        b = (b + p_2) / power;
        a = (a + p_2) / power; // always non-zero
        dest->color3D(x, y, z, Color(r, g, b, a));
    }
    void avg_Z_LP_MC_WRAP(Int x, Int y, Int z) C {
        UInt r = 0, g = 0, b = 0, a = 0;
        REP(rangei_2_1) {
            Color c = src->color3D(x, y, Mod(z + i - rangei, size.z));
            r += c.r;
            g += c.g;
            b += c.b;
            a += c.a;
        }
        UInt p_2 = (rangei_2_1 >> 1);
        r = (r + p_2) / rangei_2_1;
        g = (g + p_2) / rangei_2_1;
        b = (b + p_2) / rangei_2_1;
        a = (a + p_2) / rangei_2_1; // always non-zero
        dest->color3D(x, y, z, Color(r, g, b, a));
    }
};
/******************************************************************************/
Bool Image::averageX(Image &dest, Int range, Bool clamp) C {
    if (range <= 0 || w() <= 1)
        return copy(dest);
    IMAGE_TYPE type = T.type();
    IMAGE_MODE mode = T.mode();
    Int mip_maps = T.mipMaps();
    C Image *src = this;
    Image temp;
    if (src->compressed())
        if (src->copy(temp, -1, -1, -1, ImageTypeUncompressed(type), IMAGE_SOFT, 1))
            src = &temp;
        else
            return false;
    Image &work = ((src == &dest) ? temp : dest); // this makes &work!=src
    if (!work.create(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))
        return false; // use 'src.mode' and 'src.mipMaps' because if(src.compressed) then we will get IMAGE_SOFT 1 (what we want, because we will compress below in 'copy'), and if not compressed then we will create the target as what we want the dest to be
    if (!src->lockRead())
        return false;
    if (!work.lock(LOCK_WRITE)) {
        src->unlock();
        return false;
    }

    BlurContext bc(*src, clamp);
    bc.setRange(range);
    bc.setAvgX();
    bc.dest = &work;
    bc.blur();

    UInt flags = (clamp ? IC_CLAMP : IC_WRAP);
    work.unlock().updateMipMaps(FILTER_BEST, flags);
    src->unlock();
    if (work.type() == type && work.mode() == mode && work.mipMaps() == mip_maps) {
        if (&work != &dest)
            Swap(work, dest);
        return true;
    } // if we have desired type mode and mip maps, then all we need is to Swap if needed, remember that after Swap we should operate on 'dest' and not 'work'
    return work.copy(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, flags);
}
Bool Image::averageY(Image &dest, Int range, Bool clamp) C {
    if (range <= 0 || h() <= 1)
        return copy(dest);
    IMAGE_TYPE type = T.type();
    IMAGE_MODE mode = T.mode();
    Int mip_maps = T.mipMaps();
    C Image *src = this;
    Image temp;
    if (src->compressed())
        if (src->copy(temp, -1, -1, -1, ImageTypeUncompressed(type), IMAGE_SOFT, 1))
            src = &temp;
        else
            return false;
    Image &work = ((src == &dest) ? temp : dest); // this makes &work!=src
    if (!work.create(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))
        return false; // use 'src.mode' and 'src.mipMaps' because if(src.compressed) then we will get IMAGE_SOFT 1 (what we want, because we will compress below in 'copy'), and if not compressed then we will create the target as what we want the dest to be
    if (!src->lockRead())
        return false;
    if (!work.lock(LOCK_WRITE)) {
        src->unlock();
        return false;
    }

    BlurContext bc(*src, clamp);
    bc.setRange(range);
    bc.setAvgY();
    bc.dest = &work;
    bc.blur();

    UInt flags = (clamp ? IC_CLAMP : IC_WRAP);
    work.unlock().updateMipMaps(FILTER_BEST, flags);
    src->unlock();
    if (work.type() == type && work.mode() == mode && work.mipMaps() == mip_maps) {
        if (&work != &dest)
            Swap(work, dest);
        return true;
    } // if we have desired type mode and mip maps, then all we need is to Swap if needed, remember that after Swap we should operate on 'dest' and not 'work'
    return work.copy(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, flags);
}
Bool Image::averageZ(Image &dest, Int range, Bool clamp) C {
    if (range <= 0 || d() <= 1)
        return copy(dest);
    IMAGE_TYPE type = T.type();
    IMAGE_MODE mode = T.mode();
    Int mip_maps = T.mipMaps();
    C Image *src = this;
    Image temp;
    if (src->compressed())
        if (src->copy(temp, -1, -1, -1, ImageTypeUncompressed(type), IMAGE_SOFT, 1))
            src = &temp;
        else
            return false;
    Image &work = ((src == &dest) ? temp : dest); // this makes &work!=src
    if (!work.create(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))
        return false; // use 'src.mode' and 'src.mipMaps' because if(src.compressed) then we will get IMAGE_SOFT 1 (what we want, because we will compress below in 'copy'), and if not compressed then we will create the target as what we want the dest to be
    if (!src->lockRead())
        return false;
    if (!work.lock(LOCK_WRITE)) {
        src->unlock();
        return false;
    }

    BlurContext bc(*src, clamp);
    bc.setRange(range);
    bc.setAvgZ();
    bc.dest = &work;
    bc.blur();

    UInt flags = (clamp ? IC_CLAMP : IC_WRAP);
    work.unlock().updateMipMaps(FILTER_BEST, flags);
    src->unlock();
    if (work.type() == type && work.mode() == mode && work.mipMaps() == mip_maps) {
        if (&work != &dest)
            Swap(work, dest);
        return true;
    } // if we have desired type mode and mip maps, then all we need is to Swap if needed, remember that after Swap we should operate on 'dest' and not 'work'
    return work.copy(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, flags);
}
/******************************************************************************/
Bool Image::blurX(Image &dest, Flt range, Bool clamp) C {
    if (range <= 0 || w() <= 1)
        return copy(dest);
    IMAGE_TYPE type = T.type();
    IMAGE_MODE mode = T.mode();
    Int mip_maps = T.mipMaps();
    C Image *src = this;
    Image temp;
    if (src->compressed())
        if (src->copy(temp, -1, -1, -1, ImageTypeUncompressed(type), IMAGE_SOFT, 1))
            src = &temp;
        else
            return false;
    Image &work = ((src == &dest) ? temp : dest); // this makes &work!=src
    if (!work.create(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))
        return false; // use 'src.mode' and 'src.mipMaps' because if(src.compressed) then we will get IMAGE_SOFT 1 (what we want, because we will compress below in 'copy'), and if not compressed then we will create the target as what we want the dest to be
    if (!src->lockRead())
        return false;
    if (!work.lock(LOCK_WRITE)) {
        src->unlock();
        return false;
    }

    BlurContext bc(*src, clamp);
    bc.setRange(range);
    bc.setX();
    bc.dest = &work;
    bc.blur();

    UInt flags = (clamp ? IC_CLAMP : IC_WRAP);
    work.unlock().updateMipMaps(FILTER_BEST, flags);
    src->unlock();
    if (work.type() == type && work.mode() == mode && work.mipMaps() == mip_maps) {
        if (&work != &dest)
            Swap(work, dest);
        return true;
    } // if we have desired type mode and mip maps, then all we need is to Swap if needed, remember that after Swap we should operate on 'dest' and not 'work'
    return work.copy(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, flags);
}
Bool Image::blurY(Image &dest, Flt range, Bool clamp) C {
    if (range <= 0 || h() <= 1)
        return copy(dest);
    IMAGE_TYPE type = T.type();
    IMAGE_MODE mode = T.mode();
    Int mip_maps = T.mipMaps();
    C Image *src = this;
    Image temp;
    if (src->compressed())
        if (src->copy(temp, -1, -1, -1, ImageTypeUncompressed(type), IMAGE_SOFT, 1))
            src = &temp;
        else
            return false;
    Image &work = ((src == &dest) ? temp : dest); // this makes &work!=src
    if (!work.create(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))
        return false; // use 'src.mode' and 'src.mipMaps' because if(src.compressed) then we will get IMAGE_SOFT 1 (what we want, because we will compress below in 'copy'), and if not compressed then we will create the target as what we want the dest to be
    if (!src->lockRead())
        return false;
    if (!work.lock(LOCK_WRITE)) {
        src->unlock();
        return false;
    }

    BlurContext bc(*src, clamp);
    bc.setRange(range);
    bc.setY();
    bc.dest = &work;
    bc.blur();

    UInt flags = (clamp ? IC_CLAMP : IC_WRAP);
    work.unlock().updateMipMaps(FILTER_BEST, flags);
    src->unlock();
    if (work.type() == type && work.mode() == mode && work.mipMaps() == mip_maps) {
        if (&work != &dest)
            Swap(work, dest);
        return true;
    } // if we have desired type mode and mip maps, then all we need is to Swap if needed, remember that after Swap we should operate on 'dest' and not 'work'
    return work.copy(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, flags);
}
Bool Image::blurZ(Image &dest, Flt range, Bool clamp) C {
    if (range <= 0 || d() <= 1)
        return copy(dest);
    IMAGE_TYPE type = T.type();
    IMAGE_MODE mode = T.mode();
    Int mip_maps = T.mipMaps();
    C Image *src = this;
    Image temp;
    if (src->compressed())
        if (src->copy(temp, -1, -1, -1, ImageTypeUncompressed(type), IMAGE_SOFT, 1))
            src = &temp;
        else
            return false;
    Image &work = ((src == &dest) ? temp : dest); // this makes &work!=src
    if (!work.create(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()))
        return false; // use 'src.mode' and 'src.mipMaps' because if(src.compressed) then we will get IMAGE_SOFT 1 (what we want, because we will compress below in 'copy'), and if not compressed then we will create the target as what we want the dest to be
    if (!src->lockRead())
        return false;
    if (!work.lock(LOCK_WRITE)) {
        src->unlock();
        return false;
    }

    BlurContext bc(*src, clamp);
    bc.setRange(range);
    bc.setZ();
    bc.dest = &work;
    bc.blur();

    UInt flags = (clamp ? IC_CLAMP : IC_WRAP);
    work.unlock().updateMipMaps(FILTER_BEST, flags);
    src->unlock();
    if (work.type() == type && work.mode() == mode && work.mipMaps() == mip_maps) {
        if (&work != &dest)
            Swap(work, dest);
        return true;
    } // if we have desired type mode and mip maps, then all we need is to Swap if needed, remember that after Swap we should operate on 'dest' and not 'work'
    return work.copy(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, flags);
}
/******************************************************************************/
Image &Image::average(C VecI &range, Bool clamp) {
    average(T, range, clamp);
    return T;
}
Bool Image::average(Image &dest, C VecI &range, Bool clamp) C {
    Bool blur[] = {range.x > 0 && w() > 1, range.y > 0 && h() > 1, range.z > 0 && d() > 1};
    Int blurs = blur[0] + blur[1] + blur[2];
    if (!blurs)
        return copy(dest);
    IMAGE_TYPE type = T.type();
    IMAGE_MODE mode = T.mode();
    Int mip_maps = T.mipMaps();
    C Image *src = this;
    Image temp;
    if (src->compressed())
        if (src->copy(temp, -1, -1, -1, ImageTypeUncompressed(type), IMAGE_SOFT, 1))
            src = &temp;
        else
            return false;
    BlurContext bc(*src, clamp);
    if (src == &dest) // this also means !T.compressed && 'temp' is empty
    {
        if (blurs & 1)
            temp.create(w(), h(), d(), type, mode, mip_maps); // we will end up in 'temp' so create 'temp' as target mode and mip maps
        else
            temp.createSoft(w(), h(), d(), type, 1); // we will end up in 'dest' so create 'temp' as SOFT
        if (!temp.is())
            return false;
        bc.dest = &temp;
        if (!temp.lock(LOCK_WRITE))
            return false;
        if (!dest.lock(LOCK_READ_WRITE))
            return false;    // we read from src and write to dest, and src==dest
                             // we already locked 'src' by locking 'dest'
    } else if (src == &temp) // this also means T.compressed && 'temp' is created
    {
        if (!dest.createSoft(w(), h(), d(), ImageTypeUncompressed(type), 1))
            return false; // always create as soft, because T.compressed so we will have to compress it anyway
        bc.dest = &dest;
        // no need to lock, because src==temp (which is SOFT) and dest is SOFT
    } else // src!=&dest && src!=&temp && !T.compressed
    {
        if (!dest.create(w(), h(), d(), type, mode, mip_maps))
            return false; // we will end up in 'dest' so create it as target mode and mip maps
        if (blurs > 1 && !temp.createSoft(w(), h(), d(), type, 1))
            return false; // create temp as SOFT
        bc.dest = ((blurs & 1) ? &dest : &temp);
        if (!src->lockRead())
            return false;
        if (!dest.lock(LOCK_WRITE))
            return false;
        // 'temp' is SOFT so doesn't need lock
    }

    if (blur[0]) {
        bc.setRange(range.x);
        bc.setAvgX();
        bc.blur();
        bc.src = bc.dest;
        bc.dest = ((bc.src == &dest) ? &temp : &dest);
    }
    if (blur[1]) {
        bc.setRange(range.y);
        bc.setAvgY();
        bc.blur();
        bc.src = bc.dest;
        bc.dest = ((bc.src == &dest) ? &temp : &dest);
    }
    if (blur[2]) {
        bc.setRange(range.z);
        bc.setAvgZ();
        bc.blur();
        bc.src = bc.dest;
        bc.dest = ((bc.src == &dest) ? &temp : &dest);
    }

    if (src == &dest) {
        temp.unlock(); // unlock in case we will Swap it
        dest.unlock();
    } else if (src == &temp) {
    } else {
        src->unlock();
        dest.unlock();
    }

    Image &img = ConstCast(*bc.src); // at this point 'bc.src' points to non const image ('temp' or 'dest'), so we can use 'ConstCast'
    UInt flags = (clamp ? IC_CLAMP : IC_WRAP);
    img.updateMipMaps(FILTER_BEST, flags);
    if (img.type() == type && img.mode() == mode && img.mipMaps() == mip_maps) {
        if (&img != &dest)
            Swap(img, dest);
        return true;
    } // if we have desired type mode and mip maps, then all we need is to Swap if needed, remember that after Swap we should operate on 'dest' and not 'img'
    return img.copy(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, flags);
}
Image &Image::blur(C Vec &range, Bool clamp) {
    blur(T, range, clamp);
    return T;
}
Bool Image::blur(Image &dest, C Vec &range, Bool clamp) C {
    Bool blur[] = {range.x > 0 && w() > 1, range.y > 0 && h() > 1, range.z > 0 && d() > 1};
    Int blurs = blur[0] + blur[1] + blur[2];
    if (!blurs)
        return copy(dest);
    IMAGE_TYPE type = T.type();
    IMAGE_MODE mode = T.mode();
    Int mip_maps = T.mipMaps();
    C Image *src = this;
    Image temp;
    if (src->compressed())
        if (src->copy(temp, -1, -1, -1, ImageTypeUncompressed(type), IMAGE_SOFT, 1))
            src = &temp;
        else
            return false;
    BlurContext bc(*src, clamp);
    if (src == &dest) // this also means !T.compressed && 'temp' is empty
    {
        if (blurs & 1)
            temp.create(w(), h(), d(), type, mode, mip_maps); // we will end up in 'temp' so create 'temp' as target mode and mip maps
        else
            temp.createSoft(w(), h(), d(), type, 1); // we will end up in 'dest' so create 'temp' as SOFT
        if (!temp.is())
            return false;
        bc.dest = &temp;
        if (!temp.lock(LOCK_WRITE))
            return false;
        if (!dest.lock(LOCK_READ_WRITE))
            return false;    // we read from src and write to dest, and src==dest
                             // we already locked 'src' by locking 'dest'
    } else if (src == &temp) // this also means T.compressed && 'temp' is created
    {
        if (!dest.createSoft(w(), h(), d(), ImageTypeUncompressed(type), 1))
            return false; // always create as soft, because T.compressed so we will have to compress it anyway
        bc.dest = &dest;
        // no need to lock, because src==temp (which is SOFT) and dest is SOFT
    } else // src!=&dest && src!=&temp && !T.compressed
    {
        if (!dest.create(w(), h(), d(), type, mode, mip_maps))
            return false; // we will end up in 'dest' so create it as target mode and mip maps
        if (blurs > 1 && !temp.createSoft(w(), h(), d(), type, 1))
            return false; // create temp as SOFT
        bc.dest = ((blurs & 1) ? &dest : &temp);
        if (!src->lockRead())
            return false;
        if (!dest.lock(LOCK_WRITE))
            return false;
        // 'temp' is SOFT so doesn't need lock
    }

    if (blur[0]) {
        bc.setRange(range.x);
        bc.setX();
        bc.blur();
        bc.src = bc.dest;
        bc.dest = ((bc.src == &dest) ? &temp : &dest);
    }
    if (blur[1]) {
        bc.setRange(range.y);
        bc.setY();
        bc.blur();
        bc.src = bc.dest;
        bc.dest = ((bc.src == &dest) ? &temp : &dest);
    }
    if (blur[2]) {
        bc.setRange(range.z);
        bc.setZ();
        bc.blur();
        bc.src = bc.dest;
        bc.dest = ((bc.src == &dest) ? &temp : &dest);
    }

    if (src == &dest) {
        temp.unlock(); // unlock in case we will Swap it
        dest.unlock();
    } else if (src == &temp) {
    } else {
        src->unlock();
        dest.unlock();
    }

    Image &img = ConstCast(*bc.src); // at this point 'bc.src' points to non const image ('temp' or 'dest'), so we can use 'ConstCast'
    UInt flags = (clamp ? IC_CLAMP : IC_WRAP);
    img.updateMipMaps(FILTER_BEST, flags);
    if (img.type() == type && img.mode() == mode && img.mipMaps() == mip_maps) {
        if (&img != &dest)
            Swap(img, dest);
        return true;
    } // if we have desired type mode and mip maps, then all we need is to Swap if needed, remember that after Swap we should operate on 'dest' and not 'img'
    return img.copy(dest, -1, -1, -1, type, mode, mip_maps, FILTER_BEST, flags);
}
/******************************************************************************/
Bool Image::blurCubeAngle(Flt angle, Bool linear) {
    if (cube() && angle > 0) {
        if (!waitForStream())
            return false; // since we'll access 'softData' without locking, make sure stream is finished
        Threads *threads = &ImageThreads.init();
        Image *img = this, temp;
        Bool convert = false;
        if (img->mode() != IMAGE_SOFT_CUBE || img->compressed())
            if (img->copy(temp, -1, -1, -1, ImageTypeUncompressed(img->type()), IMAGE_SOFT_CUBE, 1)) {
                img = &temp;
                convert = true;
            } else
                return false;
        Image dest;
        if (!dest.create(img->w(), img->h(), img->d(), img->type(), img->mode(), convert ? 1 : img->mipMaps()))
            return false; // if we'll convert later, then create with 1 mip map only
        BlurCube(*img, 0, dest, 0, angle, threads, linear);
        if (convert)
            return dest.copy(T, -1, -1, -1, type(), mode(), mipMaps()); // convert to original type and mode
        else
            Swap(dest.updateMipMaps(), T);
    }
    return true;
}
Bool Image::blurCubePixel(Flt pixel_range, Bool linear) {
    Flt angle = Atan(pixel_range / (h() / 2.0f)); // Angle(h()/2.0f, pixel_range);
    return blurCubeAngle(angle, linear);
}
Bool Image::blurCubeMipMaps() {
    if (cube() && mipMaps() > 1) {
        if (!waitForStream())
            return false; // since we'll access 'softData' without locking, make sure stream is finished
        Threads *threads = &ImageThreads.init();
        Image *img = this, temp;
        if (img->mode() != IMAGE_SOFT_CUBE || img->compressed())
            if (img->copy(temp, -1, -1, -1, ImageTypeUncompressed(img->type()), IMAGE_SOFT_CUBE))
                img = &temp;
            else
                return false;
        Flt last = 0;
        for (Int i = 1; i < img->mipMaps(); i++) {
            Flt angle = Sqr(i / Flt(img->mipMaps() - 1)) * PI;                  // !! this angle must match specular highlights in the shader depending on smoothness !!
            BlurCube(*img, i - 1, *img, i, angle - last * 0.5f, threads, true); // make angle range smaller by a factor of angle from last step to approximately match results if 0th mip was always used as the source (which would be very slow)
            last = angle;
        }
        if (img != this && !img->copy(T, -1, -1, -1, type(), mode()))
            return false; // convert to original type and mode
    }
    return true;
}
/******************************************************************************/
Image &Image::sharpen(Flt power, C Vec &range, Bool clamp, Bool blur) {
    IMAGE_TYPE type;
    IMAGE_MODE mode;
    Int mip_maps;
    if (range.x > 0 || range.y > 0 || range.z > 0)
        if (Decompress(T, type, mode, mip_maps) && lock()) {
            Image temp;
            if (blur)
                T.blur(temp, range, clamp);
            else
                T.average(temp, Round(range), clamp);

            Bool hp = T.highPrecision();
            REPD(z, T.d())
            REPD(y, T.h())
            REPD(x, T.w()) {
                if (hp) {
                    Vec4 col_src = color3DF(x, y, z),
                         col_blur = temp.color3DF(x, y, z);
                    color3DF(x, y, z, col_src + (col_src - col_blur) * power);
                } else {
                    Color col_src = color3D(x, y, z),
                          col_blur = temp.color3D(x, y, z), c;
                    c.a = Mid(col_src.a + Round((col_src.a - col_blur.a) * power), 0, 255);
                    c.r = Mid(col_src.r + Round((col_src.r - col_blur.r) * power), 0, 255);
                    c.g = Mid(col_src.g + Round((col_src.g - col_blur.g) * power), 0, 255);
                    c.b = Mid(col_src.b + Round((col_src.b - col_blur.b) * power), 0, 255);
                    color3D(x, y, z, c);
                }
            }
            unlock().updateMipMaps(FILTER_BEST, (clamp ? IC_CLAMP : IC_WRAP));
            Compress(T, type, mode, mip_maps);
        }
    return T;
}
/******************************************************************************/
Image &Image::noise(Byte red, Byte green, Byte blue, Byte alpha) {
    IMAGE_TYPE type;
    IMAGE_MODE mode;
    Int mip_maps;
    if (red || green || blue || alpha)
        if (Decompress(T, type, mode, mip_maps) && lock()) {
            switch (T.type()) {
            case IMAGE_L8:
            case IMAGE_L8_SRGB:
            case IMAGE_L8A8:
            case IMAGE_L8A8_SRGB: {
                Byte noise = Max(red, green, blue);
                REPD(z, T.d())
                REPD(y, T.h())
                REPD(x, T.w()) {
                    Color c = color3D(x, y, z);
                    c.r = c.g = c.b = Mid(c.r + Random(-noise, noise), 0, 255);
                    c.a = Mid(c.a + Random(-alpha, alpha), 0, 255);
                    color3D(x, y, z, c);
                }
            } break;

            default:
                REPD(z, T.d())
                REPD(y, T.h())
                REPD(x, T.w()) {
                    Color c = color3D(x, y, z);
                    c.r = Mid(c.r + Random(-red, red), 0, 255);
                    c.g = Mid(c.g + Random(-green, green), 0, 255);
                    c.b = Mid(c.b + Random(-blue, blue), 0, 255);
                    c.a = Mid(c.a + Random(-alpha, alpha), 0, 255);
                    color3D(x, y, z, c);
                }
                break;
            }
            unlock().updateMipMaps();
            Compress(T, type, mode, mip_maps);
        }
    return T;
}
/******************************************************************************/
Image &Image::RGBToHSB() {
    IMAGE_TYPE type;
    IMAGE_MODE mode;
    Int mip_maps;
    if (Decompress(T, type, mode, mip_maps) && lock()) {
        REPD(z, T.d())
        REPD(y, T.h())
        REPD(x, T.w()) {
            Vec4 c = color3DF(x, y, z);
            c.xyz = RgbToHsb(c.xyz);
            color3DF(x, y, z, c);
        }
        unlock().updateMipMaps();
        Compress(T, type, mode, mip_maps);
    }
    return T;
}
Image &Image::HSBToRGB() {
    IMAGE_TYPE type;
    IMAGE_MODE mode;
    Int mip_maps;
    if (Decompress(T, type, mode, mip_maps) && lock()) {
        REPD(z, T.d())
        REPD(y, T.h())
        REPD(x, T.w()) {
            Vec4 c = color3DF(x, y, z);
            c.xyz = HsbToRgb(c.xyz);
            color3DF(x, y, z, c);
        }
        unlock().updateMipMaps();
        Compress(T, type, mode, mip_maps);
    }
    return T;
}
/******************************************************************************/
Image &Image::tile(C VecI2 &range) {
    IMAGE_TYPE type;
    IMAGE_MODE mode;
    Int mip_maps;

    VecI2 r(Min(range.x, w()), Min(range.y, h()));
    if ((r.x > 0 || r.y > 0) && Decompress(T, type, mode, mip_maps) && lock()) {
#if 1
        if (highPrecision()) {
            if (r.x > 0) {
                const Flt mul = 1.0f / (r.x + 1);
                FREPD(y, T.h())
                FREPD(x, r.x) {
                    Vec4 c0 = colorF(x, y),
                         c1 = colorF(T.w() - x - 1, y);
                    colorF(T.w() - x - 1, y, Lerp(c0, c1, (x + 1) * mul));
                }
            }

            if (r.y > 0) {
                const Flt mul = 1.0f / (r.y + 1);
                FREPD(x, T.w())
                FREPD(y, r.y) {
                    Vec4 c0 = colorF(x, y),
                         c1 = colorF(x, T.h() - y - 1);
                    colorF(x, T.h() - y - 1, Lerp(c0, c1, (y + 1) * mul));
                }
            }
        } else {
            if (r.x > 0) {
                const UInt div = r.x + 1, div_2 = (div + 1) / 2;
                FREPD(y, T.h())
                FREPD(x, r.x) {
                    UInt s0 = r.x - x,
                         s1 = x + 1;
                    Color c0 = color(x, y),
                          c1 = color(T.w() - x - 1, y), c;
                    c.r = (c0.r * s0 + c1.r * s1 + div_2) / div;
                    c.g = (c0.g * s0 + c1.g * s1 + div_2) / div;
                    c.b = (c0.b * s0 + c1.b * s1 + div_2) / div;
                    c.a = (c0.a * s0 + c1.a * s1 + div_2) / div;
                    color(T.w() - x - 1, y, c);
                }
            }

            if (r.y > 0) {
                const UInt div = r.y + 1, div_2 = (div + 1) / 2;
                FREPD(x, T.w())
                FREPD(y, r.y) {
                    UInt s0 = r.y - y,
                         s1 = y + 1;
                    Color c0 = color(x, y),
                          c1 = color(x, T.h() - y - 1), c;
                    c.r = (c0.r * s0 + c1.r * s1 + div_2) / div;
                    c.g = (c0.g * s0 + c1.g * s1 + div_2) / div;
                    c.b = (c0.b * s0 + c1.b * s1 + div_2) / div;
                    c.a = (c0.a * s0 + c1.a * s1 + div_2) / div;
                    color(x, T.h() - y - 1, c);
                }
            }
        }
#else
        if (r.x > 0)
            FREPD(y, T.h())
        FREPD(x, r.x) {
            UInt s0 = x + 1,
                 s1 = r.x - x,
                 sum = s0 + s1,
                 sum_2 = (sum + 1) / 2;
            Color c0 = color(x, y),
                  c1 = color(T.w() - r.x + x, y), c;
            c.a = (c0.a * s0 + c1.a * s1 + sum_2) / sum;
            c.r = (c0.r * s0 + c1.r * s1 + sum_2) / sum;
            c.g = (c0.g * s0 + c1.g * s1 + sum_2) / sum;
            c.b = (c0.b * s0 + c1.b * s1 + sum_2) / sum;
            color(T.w() - r.x + x, y, c);
        }

        if (r.y > 0)
            FREPD(x, T.w())
        FREPD(y, r.y) {
            UInt s0 = y + 1,
                 s1 = r.y - y,
                 sum = s0 + s1,
                 sum_2 = (sum + 1) / 2;
            Color c0 = color(x, y),
                  c1 = color(x, T.h() - r.y + y), c;
            c.a = (c0.a * s0 + c1.a * s1 + sum_2) / sum;
            c.r = (c0.r * s0 + c1.r * s1 + sum_2) / sum;
            c.g = (c0.g * s0 + c1.g * s1 + sum_2) / sum;
            c.b = (c0.b * s0 + c1.b * s1 + sum_2) / sum;
            color(x, T.h() - r.y + y, c);
        }
#endif
        unlock().updateMipMaps(FILTER_BEST, IC_WRAP);
        Compress(T, type, mode, mip_maps);
    }
    return T;
}
/******************************************************************************/
// MIN / MAX
/******************************************************************************/
static Flt Median(Long (&v)[256], Long p, Bool exact) {
    Long n = 0;
    FREPA(v) {
        n += v[i];
        if (n > p) {
            if (exact || n > p + 1)
                return i / 255.0f; // if exact, or the next value is still the same value (we use histogram, so we can just check if from "v[i]" we've accumulated 1 more than needed, this is OK)
            // find next value
            Int j = i + 1;
            for (; j < 255 && !v[j]; j++)
                ;                          // 255 instead of 256, because we always want to stop on the last element
            return (i + j) / (255.0f * 2); // return average of 'i' and 'j'
        }
    }
    return 1;
}
static Int CompareX(C Vec2 &a, C Vec2 &b) { return Compare(a.x, b.x); } // ignore 'y' alpha weight
static Flt Median(Vec2 *val_alpha, Long pixels, Dbl alpha_stop) {
    Dbl alpha = 0;
    for (Long i = 0; i < pixels; i++) {
        alpha += val_alpha[i].y;
        if (Equal(alpha, alpha_stop) && InRange(i + 1, pixels))
            return Avg(val_alpha[i].x, val_alpha[i + 1].x); // if falls down exactly at the end, then take average of this and next value
        if (alpha >= alpha_stop)
            return val_alpha[i].x;
    }
    return pixels ? val_alpha[pixels - 1].x : 0; // if didn't found inside the loop, then return the last value
}
Bool Image::stats(Vec4 *min, Vec4 *max, Vec4 *avg, Vec4 *med, Vec4 *mod, Vec *avg_alpha_weight, Vec *med_alpha_weight, C BoxI *box_ptr) C {
    if (!min && !max && !avg && !med && !mod && !avg_alpha_weight && !med_alpha_weight)
        return true; // nothing to calculate
    if (is()) {
        C Image *src = this;
        Image temp;
        if (compressed())
            if (copy(temp, -1, -1, -1, ImageTypeUncompressed(type()), IMAGE_SOFT, 1))
                src = &temp;
            else
                goto error;
        if (src->lockRead()) {
            VecD4 sum = 0, sum_alpha_weight = 0;
            Vec4 _avg, _med;
            Long r[256], g[256], b[256], a[256]; // channel 8-bit histogram
            Flt *rf, *gf, *bf, *af;
            Vec2 *raf, *gaf, *baf;
            Memt<Flt> hist;               // channels are listed separately, all reds, followed by all greens, followed by all blues, followed by all alphas
            Memt<Vec2> hist_alpha_weight; // channels are listed separately, all reds (with alpha), followed by all greens (with alpha), followed by all blues (with alpha)
            if (mod)                      // to calculate mode, we need to have average and median
            {                             // if not specified then operate on temp
                if (!avg)
                    avg = &_avg;
                if (!med)
                    med = &_med;
            }
            if (min)
                *min = FLT_MAX;
            if (max)
                *max = -FLT_MAX;
            Bool med_fast = !src->highPrecision(); // calculate median using fast method (8-bit histogram)
            BoxI box(0, src->size3());
            if (box_ptr)
                box &= *box_ptr;
            Long pixels = box.volumeL();
            if (med) {
                if (med_fast) {
                    Zero(r);
                    Zero(g);
                    Zero(b);
                    Zero(a);
                } else {
                    hist.setNum(pixels * 4); // 4 channels
                    rf = hist.data();
                    gf = rf + pixels;
                    bf = gf + pixels;
                    af = bf + pixels;
                }
            }
            if (med_alpha_weight) {
                hist_alpha_weight.setNum(pixels * 3); // 3 channels
                raf = hist_alpha_weight.data();
                gaf = raf + pixels;
                baf = gaf + pixels;
                // to calculate 'med_alpha_weight', we need to have 'sum' which will be calculated if we want 'avg'
                if (!avg)
                    avg = &_avg; // if not specified then operate on temp
            }

            for (Int z = box.min.z; z < box.max.z; z++)
                for (Int y = box.min.y; y < box.max.y; y++)
                    for (Int x = box.min.x; x < box.max.x; x++) {
                        Vec4 color = src->color3DF(x, y, z);
                        if (min)
                            *min = Min(*min, color);
                        if (max)
                            *max = Max(*max, color);
                        if (avg)
                            sum += color;
                        if (avg_alpha_weight) {
                            sum_alpha_weight.xyz += color.xyz * color.w;
                            sum_alpha_weight.w += color.w;
                        }
                        if (med) {
                            if (med_fast) {
                                r[FltToByte(color.x)]++;
                                g[FltToByte(color.y)]++;
                                b[FltToByte(color.z)]++;
                                a[FltToByte(color.w)]++;
                            } else {
                                *rf++ = color.x;
                                *gf++ = color.y;
                                *bf++ = color.z;
                                *af++ = color.w;
                            }
                        }
                        if (med_alpha_weight) {
                            (raf++)->set(color.x, color.w);
                            (gaf++)->set(color.y, color.w);
                            (baf++)->set(color.z, color.w);
                        }
                    }

            src->unlock();

            if (avg)
                *avg = sum / Dbl(pixels);
            if (avg_alpha_weight) {
                if (sum_alpha_weight.w)
                    sum_alpha_weight.xyz /= sum_alpha_weight.w;
                *avg_alpha_weight = sum_alpha_weight.xyz;
            }
            if (med) {
                Long p = Unsigned(pixels) / 2;
                Bool exact = pixels & 1;
                if (med_fast) {
                    p -= !exact; // for not exact (average) we need to stop at 1 before, and average it with the next one, alternatively we could change 'Median' to find previous and not next value, however that would require 1 more iteration, so it's faster to just do this 1 time, instead of 1 iteration in each call
                    med->set(Median(r, p, exact), Median(g, p, exact), Median(b, p, exact), Median(a, p, exact));
                } else {
                    // go back at the start of the array, and sort
                    rf -= pixels;
                    Sort(rf, pixels);
                    gf -= pixels;
                    Sort(gf, pixels);
                    bf -= pixels;
                    Sort(bf, pixels);
                    af -= pixels;
                    Sort(af, pixels);
                    if (exact)
                        med->set(rf[p], gf[p], bf[p], af[p]);
                    else {
                        Long q = p - 1;
                        med->set(Avg(rf[q], rf[p]), Avg(gf[q], gf[p]), Avg(bf[q], bf[p]), Avg(af[q], af[p]));
                    }
                }
            }
            if (med_alpha_weight) {
                // go back at the start of the array, and sort
                raf -= pixels;
                Sort(raf, pixels, CompareX);
                gaf -= pixels;
                Sort(gaf, pixels, CompareX);
                baf -= pixels;
                Sort(baf, pixels, CompareX);
                Dbl alpha_stop = sum.w / 2;
                med_alpha_weight->set(Median(raf, pixels, alpha_stop), Median(gaf, pixels, alpha_stop), Median(baf, pixels, alpha_stop));
            }
            if (mod)
                *mod = 3 * (*med) - 2 * (*avg); // Mode = 3*Median - 2*Mean

            return true;
        }
    }

error:
    if (min)
        min->zero();
    if (max)
        max->zero();
    if (avg)
        avg->zero();
    if (med)
        med->zero();
    if (mod)
        mod->zero();
    if (avg_alpha_weight)
        avg_alpha_weight->zero();
    if (med_alpha_weight)
        med_alpha_weight->zero();
    return false;
}
Bool Image::statsSat(Flt *min, Flt *max, Flt *avg, Flt *med, Flt *mod, Flt *avg_alpha_weight, Flt *med_alpha_weight, C BoxI *box_ptr) C {
    if (!min && !max && !avg && !med && !mod && !avg_alpha_weight && !med_alpha_weight)
        return true; // nothing to calculate
    if (is()) {
        C Image *src = this;
        Image temp;
        if (compressed())
            if (copy(temp, -1, -1, -1, ImageTypeUncompressed(type()), IMAGE_SOFT, 1))
                src = &temp;
            else
                goto error;
        if (src->lockRead()) {
            Dbl sum = 0, sum_alpha = 0;
            VecD2 sum_alpha_weight = 0;
            Flt _avg, _med;
            Long v[256]; // 8-bit histogram
            Flt *vf;
            Vec2 *vaf;
            Memt<Flt> hist;
            Memt<Vec2> hist_alpha_weight;
            if (mod) // to calculate mode, we need to have average and median
            {        // if not specified then operate on temp
                if (!avg)
                    avg = &_avg;
                if (!med)
                    med = &_med;
            }
            if (min)
                *min = FLT_MAX;
            if (max)
                *max = -FLT_MAX;
            Bool med_fast = !src->highPrecision(); // calculate median using fast method (8-bit histogram)
            BoxI box(0, src->size3());
            if (box_ptr)
                box &= *box_ptr;
            Long pixels = box.volumeL();
            if (med) {
                if (med_fast)
                    Zero(v);
                else
                    vf = hist.setNum(pixels).data();
            }
            if (med_alpha_weight) {
                vaf = hist_alpha_weight.setNum(pixels).data();
            }

            for (Int z = box.min.z; z < box.max.z; z++)
                for (Int y = box.min.y; y < box.max.y; y++)
                    for (Int x = box.min.x; x < box.max.x; x++) {
                        Vec4 c = src->color3DF(x, y, z);
                        Flt s = RgbToHsb(c.xyz).y;
                        if (min)
                            MIN(*min, s);
                        if (max)
                            MAX(*max, s);
                        if (avg)
                            sum += s;
                        if (avg_alpha_weight) {
                            sum_alpha_weight.x += s * c.w;
                            sum_alpha_weight.y += c.w;
                        }
                        if (med) {
                            if (med_fast)
                                v[FltToByte(s)]++;
                            else
                                *vf++ = s;
                        }
                        if (med_alpha_weight) {
                            (vaf++)->set(s, c.w);
                            sum_alpha += c.w;
                        }
                    }

            src->unlock();

            if (avg)
                *avg = sum / Dbl(pixels);
            if (avg_alpha_weight) {
                if (sum_alpha_weight.y)
                    sum_alpha_weight.x /= sum_alpha_weight.y;
                *avg_alpha_weight = sum_alpha_weight.x;
            }
            if (med) {
                Long p = Unsigned(pixels) / 2;
                Bool exact = pixels & 1;
                if (med_fast) {
                    p -= !exact; // for not exact (average) we need to stop at 1 before, and average it with the next one, alternatively we could change 'Median' to find previous and not next value, however that would require 1 more iteration, so it's faster to just do this 1 time, instead of 1 iteration in each call
                    *med = Median(v, p, exact);
                } else {
                    // go back at the start of the array, and sort
                    vf -= pixels;
                    Sort(vf, pixels);
                    *med = (exact ? vf[p] : Avg(vf[p - 1], vf[p]));
                }
            }
            if (med_alpha_weight) {
                // go back at the start of the array, and sort
                vaf -= pixels;
                Sort(vaf, pixels, CompareX);
                Dbl alpha_stop = sum_alpha / 2;
                *med_alpha_weight = Median(vaf, pixels, alpha_stop);
            }
            if (mod)
                *mod = 3 * (*med) - 2 * (*avg); // Mode = 3*Median - 2*Mean

            return true;
        }
    }

error:
    if (min)
        *min = 0;
    if (max)
        *max = 0;
    if (avg)
        *avg = 0;
    if (med)
        *med = 0;
    if (mod)
        *mod = 0;
    if (avg_alpha_weight)
        *avg_alpha_weight = 0;
    if (med_alpha_weight)
        *med_alpha_weight = 0;
    return false;
}
/******************************************************************************/
Bool Image::needsAlpha() C {
    if (!typeInfo().a)
        return false;
    Bool extract = compressed();
    IMAGE_TYPE type = ImageTypeUncompressed(T.type());
    Image temp;
    C Image *src = (extract ? &temp : this);
    REPD(face, faces()) // 'faces' and not 'src.faces' because that could be empty
    {
        int src_face = face;
        if (extract)
            if (extractMipMap(temp, type, 0, DIR_ENUM(face)))
                src_face = 0;
            else
                return true; // error
        if (src->lockRead(0, DIR_ENUM(src_face))) {
            REPD(z, src->ld())
            REPD(y, src->lh())
            REPD(x, src->lw()) {
                Vec4 c = src->color3DF(x, y, z);
                if (!Equal(c.w, 1, 1.5f / 255)) {
                    src->unlock();
                    return true;
                } // 1.5 instead of 0.5 because compressed texture formats may be imprecise (BC7..)
            }
            src->unlock();
        }
    }
    return false;
}
Bool Image::monochromatic() C {
    C ImageTypeInfo &type_info = typeInfo();
    if (!type_info.r && !type_info.g && !type_info.b || type() == IMAGE_L8 || type() == IMAGE_L8_SRGB || type() == IMAGE_L8A8 || type() == IMAGE_L8A8_SRGB)
        return true;

    Bool extract = compressed();
    IMAGE_TYPE type = ImageTypeUncompressed(T.type());
    Image temp;
    C Image *src = (extract ? &temp : this);
    REPD(face, faces()) // 'faces' and not 'src.faces' because that could be empty
    {
        int src_face = face;
        if (extract)
            if (extractMipMap(temp, type, 0, DIR_ENUM(face)))
                src_face = 0;
            else
                return false; // error
        if (src->lockRead(0, DIR_ENUM(src_face))) {
            REPD(z, src->ld())
            REPD(y, src->lh())
            REPD(x, src->lw()) {
                Color c = src->color3D(x, y, z);
                if (!c.mono()) {
                    src->unlock();
                    return false;
                }
            }
            src->unlock();
        }
    }
    return true;
}
Bool Image::monochromaticRG() C {
    C ImageTypeInfo &type_info = typeInfo();
    if (!type_info.r && !type_info.g || type() == IMAGE_L8 || type() == IMAGE_L8_SRGB || type() == IMAGE_L8A8 || type() == IMAGE_L8A8_SRGB)
        return true;

    Bool extract = compressed();
    IMAGE_TYPE type = ImageTypeUncompressed(T.type());
    Image temp;
    C Image *src = (extract ? &temp : this);
    REPD(face, faces()) // 'faces' and not 'src.faces' because that could be empty
    {
        int src_face = face;
        if (extract)
            if (extractMipMap(temp, type, 0, DIR_ENUM(face)))
                src_face = 0;
            else
                return false; // error
        if (src->lockRead(0, DIR_ENUM(src_face))) {
            REPD(z, src->ld())
            REPD(y, src->lh())
            REPD(x, src->lw()) {
                Color c = src->color3D(x, y, z);
                if (c.r != c.g) {
                    src->unlock();
                    return false;
                }
            }
            src->unlock();
        }
    }
    return true;
}
/******************************************************************************/
Image &Image::minimum(Flt distance) {
    IMAGE_TYPE type;
    IMAGE_MODE mode;
    Int mip_maps;

    SAT(distance);
    if (distance > EPS && Decompress(T, type, mode, mip_maps)) {
        Image temp(w(), h(), 1, T.type(), T.mode(), (T.type() != type || T.mode() != mode) ? 1 : mip_maps);
        if (temp.lock(LOCK_WRITE)) {
            if (lockRead()) {
                REPD(y, T.h())
                REPD(x, T.w()) {
                    Vec4 c = T.colorF(x, y);
                    for (Int sy = -1; sy <= 1; sy++)
                        if (InRange(y + sy, T.h()))
                            for (Int sx = -1; sx <= 1; sx++)
                                if (InRange(x + sx, T.w()) && (sx || sy)) {
                                    Vec2 o(sx, sy);
                                    o.setLength(distance);
                                    Vec4 t = colorFCubicFast(x + o.x, y + o.y, true);
                                    MIN(c.x, t.x);
                                    MIN(c.y, t.y);
                                    MIN(c.z, t.z);
                                    MIN(c.w, t.w);
                                }
                    temp.colorF(x, y, c);
                }
                unlock();
            }
            temp.unlock().updateMipMaps();
            Swap(temp, T);
        }
        Compress(T, type, mode, mip_maps);
    }
    return T;
}
/******************************************************************************/
Image &Image::maximum(Flt distance) {
    IMAGE_TYPE type;
    IMAGE_MODE mode;
    Int mip_maps;

    SAT(distance);
    if (distance > EPS && Decompress(T, type, mode, mip_maps)) {
        Image temp(w(), h(), 1, T.type(), T.mode(), (T.type() != type || T.mode() != mode) ? 1 : mip_maps);
        if (temp.lock(LOCK_WRITE)) {
            if (lockRead()) {
                REPD(y, T.h())
                REPD(x, T.w()) {
                    Vec4 c = T.colorF(x, y);
                    for (Int sy = -1; sy <= 1; sy++)
                        if (InRange(y + sy, T.h()))
                            for (Int sx = -1; sx <= 1; sx++)
                                if (InRange(x + sx, T.w()) && (sx || sy)) {
                                    Vec2 o(sx, sy);
                                    o.setLength(distance);
                                    Vec4 t = T.colorFLinear(x + o.x, y + o.y, true);
                                    MAX(c.x, t.x);
                                    MAX(c.y, t.y);
                                    MAX(c.z, t.z);
                                    MAX(c.w, t.w);
                                }
                    temp.colorF(x, y, c);
                }
                unlock();
            }
            temp.unlock().updateMipMaps();
            Swap(temp, T);
        }
        Compress(T, type, mode, mip_maps);
    }
    return T;
}
/******************************************************************************/
Image &Image::transparentToNeighbor(Bool clamp, Flt step) {
#if 1 // new method
    if (!typeInfo().a)
        return T; // true;
    Int mips = TotalMipMaps(w(), h(), d());
    if (mips <= 1)
        return T; // true;
    Image *src = this, temp;
    if (!src->highPrecision() || src->compressed())
        if (src->copy(temp, -1, -1, -1, IMAGE_F32_4, IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA))
            src = &temp;
        else
            return T; // false; // first we have to copy to IMAGE_F32_4 to make sure we have floating point, so that downsizing will not use ALPHA_LIMIT, this is absolutely critical
    Bool ok = false;
    if (src->lock()) {
        Memt<Image> mip;
        mip.setNum(mips - 1);
        Image *s = src;
        VecI size = s->size3();
        SAT(step);
        Bool lerp = (step != 1);
        FREPA(mip) {
            size >>= 1;
            if (!s->copy(mip[i], size.x, size.y, size.z, IMAGE_F32_4, IMAGE_SOFT, 1, FILTER_CUBIC_FAST_SMOOTH, (clamp ? IC_CLAMP : IC_WRAP) | IC_ALPHA_WEIGHT | IC_NO_ALPHA_LIMIT | IC_IGNORE_GAMMA))
                goto error; // we need a non-sharpening filter and one that spreads in all directions (more than 2x2 samples), need to use IC_NO_ALPHA_LIMIT or else it won't work well
            s = &mip[i];
        }

        REPD(y, h())
        REPD(x, w()) {
            Vec4 s = src->colorF(x, y);
            if (!s.w)
                FREPA(mip) // if transparent, then iterate all mip maps starting from the biggest
                {
                    Image &m = mip[i];
                    Vec2 x_mul_add;
                    x_mul_add.x = Flt(m.w()) / w();
                    x_mul_add.y = x_mul_add.x * 0.5f - 0.5f;
                    Vec2 y_mul_add;
                    y_mul_add.x = Flt(m.h()) / h();
                    y_mul_add.y = y_mul_add.x * 0.5f - 0.5f;
                    // Vec2 z_mul_add; z_mul_add.x=Flt(m.d())/d(); z_mul_add.y=z_mul_add.x*0.5f-0.5f;

                    Vec4 c = m.colorFLinearTTNF32_4(x * x_mul_add.x + x_mul_add.y, y * y_mul_add.x + y_mul_add.y, clamp); // !! need to use 'colorFLinearTTNF32_4' which uses ALPHA_LIMIT_NONE !!
                                                                                                                          // Vec4 c=m.colorFCubicFastSmooth(x*x_mul_add.x + x_mul_add.y, y*y_mul_add.x + y_mul_add.y, clamp);
                    if (c.w)                                                                                              // if we've found some valid color value
                    {
                        c.w = 0; // remember to clear alpha to zero to preserve original transparency, but we keep the new RGB values
                        if (lerp)
                            c.xyz = Lerp(s.xyz, c.xyz, step);
                        src->colorF(x, y, c);
                        break; // finished looking for a sample
                    }
                }
        }
        ok = true;
    error:;
        src->unlock();
        if (ok) {
            src->updateMipMaps(FILTER_BEST, (clamp ? IC_CLAMP : IC_WRAP) | IC_ALPHA_WEIGHT);
            ok = src->copy(T, -1, -1, -1, type(), mode(), mipMaps(), FILTER_BEST, (clamp ? IC_CLAMP : IC_WRAP) | IC_ALPHA_WEIGHT | IC_IGNORE_GAMMA);
        }
    }
    return T; // ok;
#else // old method
    IMAGE_TYPE type;
    IMAGE_MODE mode;
    Int mip_maps;

    if (Decompress(T, type, mode, mip_maps)) {
        if (lock()) {
            Image used(w(), h(), 1, IMAGE_I8, IMAGE_SOFT, 1);
            used.clear();
            Memt<VecI2> opn, opn_next; // coordinates of transparent pixel which has opaque neighbor
            // iterate all pixels
            REPD(y, T.h())
            REPD(x, T.w())
            if (color(x, y).a)
                used.pixB(x, y) = 1;
            else {
                // iterate all neighbors
                for (Int sy = y - 1; sy <= y + 1; sy++)
                    if (InRange(sy, T.h()))
                        for (Int sx = x - 1; sx <= x + 1; sx++)
                            if (InRange(sx, T.w()))
                                if (color(sx, sy).a) // if at least one neighbor has alpha
                                {
                                    used.pixB(x, y) = 2;
                                    opn.New().set(x, y);
                                    goto added;
                                }
            added:;
            }
            for (Byte prev_step = 1, cur_step = 2; opn.elms();) {
                Byte next_step = (cur_step + 1) & 0xFF;
                if (!next_step)
                    next_step = 1; // set next step, and make sure to skip '0' which is used for "not yet set"
                REPA(opn) {
                    VecI2 p = opn[i];
                    opn.removeLast();
                    VecI4 sum = 0;
                    for (Int sy = p.y - 1; sy <= p.y + 1; sy++)
                        if (InRange(sy, T.h()))
                            for (Int sx = p.x - 1; sx <= p.x + 1; sx++)
                                if (InRange(sx, T.w())) {
                                    Byte &u = used.pixB(sx, sy);
                                    if (u == prev_step) // was set in previous frame
                                    {
                                        Color c = T.color(sx, sy);
                                        Int a = c.a + 1;
                                        if (sy == p.y || sx == p.x)
                                            a *= 2; // make horizontal/vertical neighbors 2x more significant, and thus making corner neighbors 2x less
                                        sum.x += c.r * a;
                                        sum.y += c.g * a;
                                        sum.z += c.b * a;
                                        sum.w += a;
                                    } else if (!u) // if not yet added
                                    {
                                        u = next_step;              // set as already added
                                        opn_next.New().set(sx, sy); // add to next list of pixels
                                    }
                                }
                    Int h = sum.w / 2; // half
                    T.color(p.x, p.y, sum.w ? Color((sum.x + h) / sum.w, (sum.y + h) / sum.w, (sum.z + h) / sum.w, 0) : TRANSPARENT);
                }
                prev_step = cur_step;
                cur_step = next_step;
                Swap(opn, opn_next);
            }
            unlock().updateMipMaps();
        }
        Compress(T, type, mode, mip_maps);
    }
    return T;
#endif
}
/******************************************************************************/
Image &Image::transparentForFiltering(Bool clamp, Flt intensity) {
    C Image *src = this;
    Image temp;
    if (compressed())
        if (copy(temp, -1, -1, -1, ImageTypeUncompressed(type()), IMAGE_SOFT, 1))
            src = &temp;
        else
            return T;
    if (src->lockRead()) {
        Bool ok = false;
        Image dest;
        if (dest.create(src->w(), src->h(), src->d(), src->type(), src->mode(), src->mipMaps()) && dest.lock(LOCK_WRITE)) {
            intensity /= 9 * 2; // 9 neighbors, and half
            const Flt scale = 2;
            RectI mask(0, 0, src->w() - 1, src->h() - 1);
            REPD(z, dest.d())
            REPD(y, dest.h())
            REPD(x, dest.w()) {
                Vec4 base = src->color3DF(x, y, z);
                Vec4 out;
                out.xyz = 0;
                out.w = base.w;
                Flt weight = 0;

                // Example #1: center A=  1, neighbor A=  5, center weight=  1, neighbor weight=  4
                // Example #2: center A=128, neighbor A=255, center weight=128, neighbor weight=127
                // Example #3: center A=255, neighbor A=255, center weight=255, neighbor weight=  0
                // Example #4: center A=  1, neighbor A=255, center weight=  1, neighbor weight=255

                // add starting color
                Flt w = base.w; // with weight according to its alpha
                out.xyz += w * base.xyz;
                weight += w;

                RectI rect(x - 1, y - 1, x + 1, y + 1);
                if (clamp)
                    rect &= mask;
                for (Int sy = rect.min.y; sy <= rect.max.y; sy++)
                    for (Int sx = rect.min.x; sx <= rect.max.x; sx++)
                        if (sx != x || sy != y) // skip center
                        {
                            Vec4 s = src->color3DF(clamp ? sx : Mod(sx, src->w()), clamp ? sy : Mod(sy, src->h()), z);
                            // add neighbor color
                            Flt w = s.w - base.w * scale;
                            if (w > 0) // with weight as difference between neighbor and center alpha
                            {
                                w *= intensity;
                                out.xyz += w * s.xyz;
                                weight += w;
                            }
                        }

                if (weight)
                    out.xyz /= weight;
                else
                    out.xyz = base.xyz; // if have no weight, then just copy original
                dest.color3DF(x, y, z, out);
            }
            ok = true;
            dest.unlock().updateMipMaps(FILTER_BEST, clamp ? IC_CLAMP : IC_WRAP);
        }
        src->unlock();
        if (ok)
            Swap(dest, T);
    }
    return T;
}
/******************************************************************************/
static VecI2 move[8] =
    {
        VecI2(0, 1),
        VecI2(0, -1),
        VecI2(1, 0),
        VecI2(-1, 0),

        VecI2(1, 1),
        VecI2(1, -1),
        VecI2(-1, 1),
        VecI2(-1, -1),
};
Bool Image::getSameColorNeighbors(Int x, Int y, MemPtr<VecI2> pixels, Bool diagonal) C {
    pixels.clear();
    if (InRange(x, w()) && InRange(y, h())) {
        C Image *src = this;
        Image temp;
        if (compressed()) {
            if (!copy(temp, -1, -1, -1, ImageTypeUncompressed(type()), IMAGE_SOFT, 1))
                return false;
            src = &temp;
        }
        if (src->lockRead()) {
            const Int moves = (diagonal ? 8 : 4);
            Image used(w(), h(), 1, IMAGE_I8, IMAGE_SOFT, 1);
            used.clear().pixel(x, y, 1);
            Color color = src->color(x, y);
            pixels.add(VecI2(x, y));
            FREPAD(processed, pixels) {
                VecI2 pos = pixels[processed]; // don't use reference because 'pixels' may get modified later
                REP(moves) {
                    VecI2 next = pos + move[i];
                    if (InRange(next.x, w()) && InRange(next.y, h())) {
                        Byte &b = used.pixB(next.x, next.y);
                        if (!b) {
                            b = 1;
                            if (src->color(next.x, next.y) == color)
                                pixels.add(next);
                        }
                    }
                }
            }
            src->unlock();
            return true;
        }
    }
    return false;
}
Image &Image::fill(Int x, Int y, C Color &color, Bool diagonal) {
    if (InRange(x, w()) && InRange(y, h())) {
        IMAGE_TYPE type;
        IMAGE_MODE mode;
        Int mip_maps;

        if (Decompress(T, type, mode, mip_maps)) {
            if (lock()) {
                const Int moves = (diagonal ? 8 : 4);
                Image used(w(), h(), 1, IMAGE_I8, IMAGE_SOFT, 1);
                used.clear().pixel(x, y, 1);
                Color src = T.color(x, y);
                Memt<VecI2> pixels;
                for (pixels.add(VecI2(x, y)); pixels.elms();) {
                    VecI2 pos = pixels.pop();
                    T.color(pos.x, pos.y, color);
                    REP(moves) {
                        VecI2 next = pos + move[i];
                        if (InRange(next.x, w()) && InRange(next.y, h())) {
                            Byte &b = used.pixB(next.x, next.y);
                            if (!b) {
                                b = 1;
                                if (T.color(next.x, next.y) == src)
                                    pixels.add(next);
                            }
                        }
                    }
                }
                unlock().updateMipMaps();
            }
            Compress(T, type, mode, mip_maps);
        }
    }
    return T;
}
/******************************************************************************/
// SHADOWS
/******************************************************************************/
Image &Image::createShadow(C Image &src, Int blur, Flt shadow_opacity, Flt shadow_spread, Bool border_padd) {
    MAX(blur, 0);
    Clamp(shadow_opacity, 0, 1);
    Clamp(shadow_spread, 0, 1);
    shadow_spread = 1 - shadow_spread;
    if (shadow_spread)
        shadow_spread = 1 / shadow_spread;
    Int padd = blur + border_padd;

    C Image *s = &src;
    Image decompressed;
    if (s->compressed())
        if (s->copy(decompressed, -1, -1, -1, ImageTypeUncompressed(s->type()), IMAGE_SOFT, 1))
            s = &decompressed;
        else
            return T;
    Image temp(s->w() + padd * 2, s->h() + padd * 2, 1, IMAGE_A8, IMAGE_SOFT, 1);

    // copy
    if (s->lockRead()) {
        FREPD(y, temp.h())
        FREPD(x, temp.w())
        temp.pixB(x, y) = s->color(x - padd, y - padd).a;
        s->unlock();
    }

    // blur
    temp.blur(blur, true);

    // normalize
    Int max = 0;
    REPD(y, temp.h())
    REPD(x, temp.w())
    MAX(max, temp.pixB(x, y));
    if (max)
        REPD(y, temp.h())
    REPD(x, temp.w()) {
        Flt s = Flt(temp.pixB(x, y)) / max * shadow_opacity;
        if (shadow_spread)
            s *= shadow_spread;
        else if (s)
            s = 1;
        MIN(s, shadow_opacity);
        temp.pixB(x, y) = RoundPos(s * 255);
    }

    Swap(T, temp);
    return T;
}
Image &Image::applyShadow(C Image &shadow, C Color &shadow_color, C VecI2 &offset, Int image_type, Bool combine) {
    IMAGE_TYPE type;
    IMAGE_MODE mode;
    Int mip_maps;

    if (shadow_color.a && Decompress(T, type, mode, mip_maps)) {
        Int l = Max(0, -offset.x),
            u = Max(0, -offset.y);
        if (image_type < 0)
            image_type = type;      // get original
        if (!ImageTI[image_type].a) // if no alpha available, auto-detect
        {
            if ((type == IMAGE_L8 || type == IMAGE_L8A8 || type == IMAGE_A8) && shadow_color.mono())
                image_type = IMAGE_L8A8;
            else if ((type == IMAGE_L8_SRGB || type == IMAGE_L8A8_SRGB) && shadow_color.mono())
                image_type = IMAGE_L8A8_SRGB;
            else
                image_type = (IsSRGB(type) ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8A8);
        }

        Image temp(Max(w(), shadow.w() + offset.x) - Min(0, offset.x), Max(h(), shadow.h() + offset.y) - Min(0, offset.y), 1, ImageTypeUncompressed((IMAGE_TYPE)image_type), IMAGE_SOFT, 1);
        if (shadow.lockRead()) {
            if (lockRead()) {
                Color sc = shadow_color;
                FREPD(y, temp.h())
                FREPD(x, temp.w()) {
                    Color col = color(x - l, y - u);
                    Byte shd = shadow.pixel(x - l - offset.x, y - u - offset.y);
                    sc.a = (shadow_color.a * shd + 128) / 255;
                    if (combine)
                        temp.color(x, y, Blend(sc, col));
                    else {
                        col.a = sc.a;
                        temp.color(x, y, col);
                    }
                }
                unlock();
            }
            shadow.unlock();
        }
        temp.copy(temp, -1, -1, -1, IMAGE_TYPE(image_type), mode, mip_maps);
        Swap(T, temp);
    }
    return T;
}
Image &Image::setShadow(Int blur, Flt shadow_opacity, Flt shadow_spread, Bool border_padd, C VecI2 &offset, Int image_type, Bool combine) {
    if (shadow_opacity > 0)
        applyShadow(Image().createShadow(T, blur, shadow_opacity, shadow_spread, border_padd), BLACK, offset - blur - border_padd, image_type, combine);
    return T;
}
/******************************************************************************/
Bool Image::raycast(C Vec &start, C Vec &move, C Matrix *image_matrix, Flt *hit_frac, Vec *hit_pos, Flt precision) C {
    Bool hit = false;
    Vec s = start, m = move;
    if (image_matrix) {
        s /= *image_matrix;
        m /= image_matrix->orn();
    }
    Rect rect(0, 0, 1, 1);
    Flt frac_start, frac_end;
    if (SweepPointRect(s.xy, m.xy, rect, &frac_start) && SweepPointRect((s.xy + m.xy), -m.xy, rect, &frac_end)) // 'frac_end' is frac from end position towards start, "1-frac_end" is frac from start towards end
        if (lockRead()) {
            Int w1 = w() - 1, h1 = h() - 1;
            s.x *= w1;
            m.x *= w1;
            s.y *= h1;
            m.y *= h1;

            Vec pos = s + frac_start * m,                 // start clipped
                m_clip = m * (1 - frac_end - frac_start); // move  clipped
            Int steps = Max(1, RoundPos(m_clip.length() * precision));
            m_clip /= steps;
            Flt image_pos = pixelFLinear(pos.x, pos.y);
            REP(steps) {
                Vec next = pos + m_clip;
                Flt image_next = pixelFLinear(next.x, next.y);
                if (pos.z >= image_pos && next.z <= image_next     // current is above and next is below
                    || pos.z <= image_pos && next.z >= image_next) // current is below and next is above
                {
                    hit = true;
                    if (hit_frac || hit_pos) {
                        // X    Y1        Y2
                        // 0  pos .z  image_pos
                        // 1  next.z  image_next

                        // Y1(x) = pos.z     +  m_clip.z             *x
                        // Y2(x) = image_pos + (image_next-image_pos)*x

                        // Y1(x) = Y2(x)
                        // pos.z + m_clip.z*x = image_pos + (image_next-image_pos)*x
                        // (m_clip.z-(image_next-image_pos))*x = image_pos - pos.z
                        // (m_clip.z+image_pos-image_next)*x = image_pos - pos.z
                        // x = image_pos - pos.z / (m_clip.z+image_pos-image_next)
                        if (Flt div = m_clip.z + image_pos - image_next) {
                            Flt x = (image_pos - pos.z) / div;
                            pos += m_clip * x;
                        }

                        Int maxi = Abs(m).maxI();
                        Flt frac = m.c[maxi];
                        if (frac)
                            frac = (pos.c[maxi] - s.c[maxi]) / frac;
                        if (hit_frac)
                            *hit_frac = frac;
                        if (hit_pos)
                            *hit_pos = start + move * frac;
                    }
                    break;
                }
                pos = next;
                image_pos = image_next;
            }
            unlock();
        }
    return hit;
}
/******************************************************************************
void normalToBump(Image &dest, Bool high_quality); // convert normal map to bump map

static inline Flt DX(Vec nrm, Bool rescale)
{
   if(rescale)nrm=nrm*(255.0f/127)-128.0f/127.0f; // (nrm*255-128)/127
   else       nrm=nrm*2-1;
   Vec2 n2(nrm.z, -nrm.x);
   n2.y/=Max(0.1f, Abs(n2.x));
   return n2.y;
}
static inline Flt DY(Vec nrm, Bool rescale)
{
   if(rescale)nrm=nrm*(255.0f/127)-128.0f/127.0f; // (nrm*255-128)/127
   else       nrm=nrm*2-1;
   Vec2 n2(nrm.z, -nrm.y);
   n2.y/=Max(0.1f, Abs(n2.x));
   return n2.y;
}
void Image::normalToBump(Image &dest, Bool high_quality)
{
   Bool high=true;
   // obliczaj lokalnie w blokach (przykladowo dla kazdego 8x8 obliczaj lokalna heightmape)
   // roznice w poziomach pomiedzy blokami obliczaj na podstawie sredniej roznicy wspolnych pikseli (krawedzi)
   // roznice pomiedzy blokami nalezy obliczac od srodka obrazka, promieniscie na zewnatrz (jaka rozchodzaca sie fala)
   if(high_quality)
   {
      if(lockRead())
      {
         Image delta, bump, temp;
         if(delta.create(_x, _y, 1, IMAGE_F32_2, IMAGE_SOFT, 1))
         if(bump .create(_x, _y, 1, IMAGE_F32_2, IMAGE_SOFT, 1))
         if(temp .create(_x, _y, 1, IMAGE_F32  , IMAGE_SOFT, 1))
         {
            // set bump delta's from normals
            Bool rescale=(bytePP()<=4); // rescale imprecisions of integer types resulting in 128/255 != 0.5
            REPD(y, T._y)
            REPD(x, T._x)
            {
               Vec nrm=colorF(x, y);
               delta.pixF2(x, y).set(DX(nrm, rescale), DY(nrm, rescale));
            }

            // create blended temp
            temp.clear();
            Int res  =Max(1, Min(64, T._x, T._y)),
                blend=(res>>1);
            for(Int my=0; my<T._y; my+=res)
            for(Int mx=0; mx<T._x; mx+=res)
            {
               bump.pixF2(mx, my)=0; // set starting heights to zero

               // fill horizontal middle line
               for(Int x=mx-1; x>=0; x--)bump.pixF2(x, my).x=bump.pixF2(x+1, my).x-delta.pixF2(x, my).x;
               for(Int x=mx+1; x<_x; x++)bump.pixF2(x, my).x=bump.pixF2(x-1, my).x+delta.pixF2(x, my).x;

               // fill vertical lines from source horizontal middle line
               REPD(x, T._x)
               {
                  for(Int y=my-1; y>=0; y--)bump.pixF2(x, y).x=bump.pixF2(x, y+1).x-delta.pixF2(x, y).y;
                  for(Int y=my+1; y<_y; y++)bump.pixF2(x, y).x=bump.pixF2(x, y-1).x+delta.pixF2(x, y).y;
               }

               // copy vertical middle line to Y channel
               REPD(y, T._y){Vec2 &b=bump.pixF2(mx, y); b.y=b.x;}

               // fill horizontal lines from source vertical middle line
               REPD(y, T._y)
               {
                  for(Int x=mx-1; x>=0; x--)bump.pixF2(x, y).y=bump.pixF2(x+1, y).y-delta.pixF2(x, y).x;
                  for(Int x=mx+1; x<_x; x++)bump.pixF2(x, y).y=bump.pixF2(x-1, y).y+delta.pixF2(x, y).x;
               }

               // merge X and Y channels
               REPD(y, T._y)
               REPD(x, T._x){Vec2 &b=bump.pixF2(x, y); b.x+=b.y;}

               Vec4 min, max; bump.getMinMax(min, max);
               Flt  mul, add; if(min.x==max.x){mul=1; add=0;}else{mul=1.0f/(max.x-min.x); add=-min.x*mul;}

               Int y_from=Max(0, my-blend), y_to=Min(my+res+blend, T._y), y_min_blend=(my ? (my+blend) : 0), y_max_blend=((my+res<T._y) ? (my+res-blend-1) : _y),
                   x_from=Max(0, mx-blend), x_to=Min(mx+res+blend, T._x), x_min_blend=(mx ? (mx+blend) : 0), x_max_blend=((mx+res<T._x) ? (mx+res-blend-1) : _x);
               for(Int y=y_from; y<y_to; y++)
               for(Int x=x_from; x<x_to; x++)
               {
                  Flt b=1;
                  if(blend)
                  {
                     if(x<x_min_blend)b*=Flt(blend*2+1 + x-x_min_blend)/(blend*2+1);else
                     if(x>x_max_blend)b*=Flt(blend*2+1 + x_max_blend-x)/(blend*2+1);

                     if(y<y_min_blend)b*=Flt(blend*2+1 + y-y_min_blend)/(blend*2+1);else
                     if(y>y_max_blend)b*=Flt(blend*2+1 + y_max_blend-y)/(blend*2+1);
                  }
                  temp.pixF(x, y)+=(bump.pixF2(x, y).x*mul + add)*b;
               }
            }

            // create destination
            if(dest.create(_x, _y, 1, IMAGE_L8, mode()))
            if(dest.lock())
            {
               REPD(y, _y)
               REPD(x, _x)dest.pixB(x, y)=Round(Sat(temp.pixF(x, y))*255);
               dest.unlock();
            }
         }
         unlock();
      }
   }else
   {
      if(lockRead())
      {
         Image bump;
         if(   bump.create(_x, _y, 1, IMAGE_F32_2, IMAGE_SOFT, 1))
         {
            Bool rescale=(bytePP()<=4); // rescale imprecisions of integer types resulting in 128/255 != 0.5
            Int  mx     =(T._x>>1), // middle x
                 my     =(T._y>>1); // middle y
            bump.pixF2(mx, my)=0;    // set starting heights to zero

            // fill horizontal middle line
            for(Int x=mx-1; x>=0; x--)bump.pixF2(x, my).x=bump.pixF2(x+1, my).x-DX(colorF(x, my), rescale);
            for(Int x=mx+1; x<_x; x++)bump.pixF2(x, my).x=bump.pixF2(x-1, my).x+DX(colorF(x, my), rescale);

            // fill vertical lines from source horizontal middle line
            REPD(x, T._x)
            {
               for(Int y=my-1; y>=0; y--)bump.pixF2(x, y).x=bump.pixF2(x, y+1).x-DY(colorF(x, y), rescale);
               for(Int y=my+1; y<_y; y++)bump.pixF2(x, y).x=bump.pixF2(x, y-1).x+DY(colorF(x, y), rescale);
            }

            // copy vertical middle line to Y channel
            REPD(y, T._y){Vec2 &b=bump.pixF2(mx, y); b.y=b.x;}

            // fill horizontal lines from source vertical middle line
            REPD(y, T._y)
            {
               for(Int x=mx-1; x>=0; x--)bump.pixF2(x, y).y=bump.pixF2(x+1, y).y-DX(colorF(x, y), rescale);
               for(Int x=mx+1; x<_x; x++)bump.pixF2(x, y).y=bump.pixF2(x-1, y).y+DX(colorF(x, y), rescale);
            }

            // merge X and Y channels
            REPD(y, T._y)
            REPD(x, T._x){Vec2 &b=bump.pixF2(x, y); b.x+=b.y;}

            bump.normalize();

            // create dest
            dest.create(T._x, T._y, 1, IMAGE_L8, mode(), 1);
            if(dest.lock())
            {
               REPD(y, T._y)
               REPD(x, T._x)dest.pixB(x, y)=Round(bump.pixF2(x, y).x*255);
               dest.unlock();
            }
         }
         unlock();
      }
   }
}
/******************************************************************************/
void (*DecompressBlock(IMAGE_TYPE type))(C Byte *b, Color (&block)[4][4]) {
    switch (type) {
    default:
        return null;
    case IMAGE_BC1:
    case IMAGE_BC1_SRGB:
        return DecompressBlockBC1;
    case IMAGE_BC2:
    case IMAGE_BC2_SRGB:
        return DecompressBlockBC2;
    case IMAGE_BC3:
    case IMAGE_BC3_SRGB:
        return DecompressBlockBC3;
    case IMAGE_BC4:
        return DecompressBlockBC4;
        // case IMAGE_BC4_SIGN    :                             return DecompressBlockBC4S     ; needs high precision
    case IMAGE_BC5:
        return DecompressBlockBC5;
        // case IMAGE_BC5_SIGN    :                             return DecompressBlockBC5S     ; needs high precision
        // case IMAGE_BC6         :                             return DecompressBlockBC6      ; needs high precision
    case IMAGE_BC7:
    case IMAGE_BC7_SRGB:
        return DecompressBlockBC7;
    case IMAGE_ETC1:
        return DecompressBlockETC1;
    case IMAGE_ETC2_R:
        return DecompressBlockETC2R;
    case IMAGE_ETC2_R_SIGN:
        return DecompressBlockETC2RS;
    case IMAGE_ETC2_RG:
        return DecompressBlockETC2RG;
    case IMAGE_ETC2_RG_SIGN:
        return DecompressBlockETC2RGS;
    case IMAGE_ETC2_RGB:
    case IMAGE_ETC2_RGB_SRGB:
        return DecompressBlockETC2RGB;
    case IMAGE_ETC2_RGBA1:
    case IMAGE_ETC2_RGBA1_SRGB:
        return DecompressBlockETC2RGBA1;
    case IMAGE_ETC2_RGBA:
    case IMAGE_ETC2_RGBA_SRGB:
        return DecompressBlockETC2RGBA;
    }
}
Bool ImageCompare::compare(C Image &a, C Image &b, Flt similar_dif, Bool alpha_weight, Int a_mip, Flt skip_dif) // !! Warning: this always ignores gamma !!
{
    // clear
    skipped = false;
    max_dif = 0;
    avg_dif = 0;
    avg_dif2 = 0;
    similar = 0;
    psnr = 0;

    Bool ok = false;
    if (InRange(a_mip, a.mipMaps())) // if 'a' has requested mip map
    {
        // get dimensions of mip map
        Int aw = Max(1, a.w() >> a_mip),
            ah = Max(1, a.h() >> a_mip);
        // if 'b' has the same size
        FREPD(b_mip, b.mipMaps()) {
            // get dimensions of mip map
            Int bw = Max(1, b.w() >> b_mip),
                bh = Max(1, b.h() >> b_mip);
            if (aw == bw && ah == bh) // if match
            {
                Image temp_a, temp_b;
                C Image *sa = &a, *sb = &b;
                void (*decompress_a)(C Byte * b, Color(&block)[4][4]) = DecompressBlock(sa->hwType());
                void (*decompress_b)(C Byte * b, Color(&block)[4][4]) = DecompressBlock(sb->hwType());
                if (!decompress_a && sa->compressed()) {
                    if (!sa->extractMipMap(temp_a, ImageTypeUncompressed(sa->type()), a_mip))
                        return false;
                    sa = &temp_a;
                    a_mip = 0;
                }
                if (!decompress_b && sb->compressed()) {
                    if (!sb->extractMipMap(temp_b, ImageTypeUncompressed(sb->type()), b_mip))
                        return false;
                    sb = &temp_b;
                    b_mip = 0;
                }
                if (sa->lockRead(a_mip)) {
                    if (sb->lockRead(b_mip)) {
                        if (sa->lockSize() == sb->lockSize()) {
                            ok = true;

                            const Bool per_channel = false, // false=faster
                                high_prec = (sa->highPrecision() || sb->highPrecision());
                            const Int a_x_mul = sa->hwTypeInfo().block_bytes,
                                      b_x_mul = sb->hwTypeInfo().block_bytes;
                            const UInt channels = 4,
                                       max_value = (high_prec ? 1 : 255),
                                       scale = channels * max_value;
                            ULong similar_pixels = 0,
                                  processed_pixels = 0;

                            // high_prec
                            Flt skip_fdif,
                                similar_fdif,
                                max_fdif;
                            Dbl total_fdif,
                                total_fdif2;

                            // !high_prec
                            UInt skip_idif,
                                similar_idif,
                                max_idif;
                            ULong total_idif,
                                total_idif2;

                            if (high_prec) {
                                skip_fdif = Max(0, skip_dif * scale);
                                similar_fdif = Max(0, similar_dif * scale);
                                max_fdif = 0;
                                total_fdif = 0;
                                total_fdif2 = 0;
                            } else {
                                skip_idif = Max(0, RoundPos(skip_dif * scale));
                                similar_idif = Max(0, RoundPos(similar_dif * scale));
                                max_idif = 0;
                                total_idif = 0;
                                total_idif2 = 0;
                            }

                            REPD(by, DivCeil4(sa->lh())) {
                                const Int py = by * 4, ys = Min(4, sa->lh() - py);
                                Int yo[4];
                                REP(ys)
                                yo[i] = py + i;
                                REPD(bx, DivCeil4(sa->lw())) {
                                    const Int px = bx * 4, xs = Min(4, sa->lw() - px);
                                    Int xo[4];
                                    REPAO(xo) = Min(px + i, sa->lw() - 1); // set all 'xo' and clamp, because we may read more, read below why

                                    Color a_col[4][4], b_col[4][4];  // [y][x]
                                    Vec4 a_colf[4][4], b_colf[4][4]; // [y][x]

                                    if (high_prec) {
                                        // it's okay to call decompress on partial blocks, because if source is compressed then its size will always fit the entire block, and we're decompressing to temporary memory
                                        if (decompress_a) {
                                            decompress_a(sa->data() + bx * a_x_mul + by * sa->pitch(), a_col);
                                            REPD(y, ys)
                                            REPD(x, xs)
                                            a_colf[y][x] = a_col[y][x];
                                        } else
                                            sa->gather(a_colf[0], xo, 4, yo, ys); // always read 4 xs because we need correct alignment for y's (for example if we would read only 2, then the next row would not be set for block[1][0] but for block[0][2])
                                        if (decompress_b) {
                                            decompress_b(sb->data() + bx * b_x_mul + by * sb->pitch(), b_col);
                                            REPD(y, ys)
                                            REPD(x, xs)
                                            b_colf[y][x] = b_col[y][x];
                                        } else
                                            sb->gather(b_colf[0], xo, 4, yo, ys); // always read 4 xs because we need correct alignment for y's (for example if we would read only 2, then the next row would not be set for block[1][0] but for block[0][2])

                                        REPD(y, ys)
                                        REPD(x, xs) {
                                            C Vec4 &ca = a_colf[y][x],
                                                   &cb = b_colf[y][x];
                                            Flt avg_a = Avg(ca.w, cb.w),
                                                dif_r = Abs(ca.x - cb.x),
                                                dif_g = Abs(ca.y - cb.y),
                                                dif_b = Abs(ca.z - cb.z),
                                                dif_a = Abs(ca.w - cb.w),
                                                dif_rgb = dif_r + dif_g + dif_b,
                                                dif = (alpha_weight ? dif_rgb * avg_a : dif_rgb) + dif_a;

                                            MAX(max_fdif, dif);
                                            total_fdif += dif;

                                            if (per_channel)
                                                total_fdif2 += (alpha_weight ? Sqr(dif_r * avg_a) + Sqr(dif_g * avg_a) + Sqr(dif_b * avg_a)
                                                                             : Sqr(dif_r) + Sqr(dif_g) + Sqr(dif_b)) +
                                                               Sqr(dif_a);
                                            else
                                                total_fdif2 += dif * dif;

                                            if (dif <= similar_fdif)
                                                similar_pixels++;
                                            processed_pixels++;

                                            if (dif > skip_fdif) {
                                                skipped = true;
                                                goto finish;
                                            } // skip after setting all other parameters, especially 'max_fdif'
                                        }
                                    } else {
                                        // it's okay to call decompress on partial blocks, because if source is compressed then its size will always fit the entire block, and we're decompressing to temporary memory
                                        if (decompress_a)
                                            decompress_a(sa->data() + bx * a_x_mul + by * sa->pitch(), a_col);
                                        else
                                            sa->gather(a_col[0], xo, 4, yo, ys); // always read 4 xs because we need correct alignment for y's (for example if we would read only 2, then the next row would not be set for block[1][0] but for block[0][2])
                                        if (decompress_b)
                                            decompress_b(sb->data() + bx * b_x_mul + by * sb->pitch(), b_col);
                                        else
                                            sb->gather(b_col[0], xo, 4, yo, ys); // always read 4 xs because we need correct alignment for y's (for example if we would read only 2, then the next row would not be set for block[1][0] but for block[0][2])

                                        REPD(y, ys)
                                        REPD(x, xs) {
                                            C Color &ca = a_col[y][x],
                                                    &cb = b_col[y][x];
                                            UInt avg_a = AvgI(UInt(ca.a), UInt(cb.a)),
                                                 dif_r = Abs(ca.r - cb.r),
                                                 dif_g = Abs(ca.g - cb.g),
                                                 dif_b = Abs(ca.b - cb.b),
                                                 dif_a = Abs(ca.a - cb.a),
                                                 dif_rgb = dif_r + dif_g + dif_b,
                                                 dif = (alpha_weight ? DivRound(dif_rgb * avg_a, 255u) : dif_rgb) + dif_a;

                                            MAX(max_idif, dif);
                                            total_idif += dif;

                                            if (per_channel)
                                                total_idif2 += (alpha_weight ? Sqr(DivRound(dif_r * avg_a, 255u)) + Sqr(DivRound(dif_g * avg_a, 255u)) + Sqr(DivRound(dif_b * avg_a, 255u))
                                                                             : Sqr(dif_r) + Sqr(dif_g) + Sqr(dif_b)) +
                                                               Sqr(dif_a);
                                            else
                                                total_idif2 += dif * dif;

                                            if (dif <= similar_idif)
                                                similar_pixels++;
                                            processed_pixels++;

                                            if (dif > skip_idif) {
                                                skipped = true;
                                                goto finish;
                                            } // skip after setting all other parameters, especially 'max_idif'
                                        }
                                    }
                                }
                            }

                        finish:
                            Dbl MSE;
                            if (high_prec) {
                                MSE = total_fdif2 / Dbl(processed_pixels * (per_channel ? channels * max_value * max_value : scale * scale));
                                T.avg_dif = total_fdif / Dbl(processed_pixels * scale);
                                T.max_dif = max_fdif / Flt(scale);
                            } else {
                                MSE = total_idif2 / Dbl(processed_pixels * (per_channel ? channels * max_value * max_value : scale * scale));
                                T.avg_dif = total_idif / Dbl(processed_pixels * scale);
                                T.max_dif = max_idif / Flt(scale);
                            }
                            T.avg_dif2 = Sqrt(MSE);
                            T.psnr = 10 * log10(1.0 / MSE);
                            T.similar = similar_pixels / Dbl(processed_pixels);
                        }
                        sb->unlock();
                    }
                    sa->unlock();
                }
                break;
            }
        }
    }
    return ok;
}
/******************************************************************************/
#if WINDOWS_OLD
HICON CreateIcon(C Image &image, C VecI2 *cursor_hot_spot) {
    HICON icon = null;
    Image temp;
    C Image *src = &image;
    if (src->compressed())
        if (src->copy(temp, -1, -1, 1, IMAGE_R8G8B8A8_SRGB, IMAGE_SOFT, 1))
            src = &temp;
        else
            src = null;
    if (src && src->lockRead()) {
        BITMAPV5HEADER bi;
        Zero(bi);
        bi.bV5Size = SIZE(bi);
        bi.bV5Width = src->w();
        bi.bV5Height = src->h();
        bi.bV5Planes = 1;
        bi.bV5BitCount = 32;
        bi.bV5Compression = BI_BITFIELDS;
        // must be BGRA, trying to use RGBA results in no transparency
        bi.bV5RedMask = 0x00FF0000;
        bi.bV5GreenMask = 0x0000FF00;
        bi.bV5BlueMask = 0x000000FF;
        bi.bV5AlphaMask = 0xFF000000;

        VecB4 *data = null;
        HBITMAP hBitmap = CreateDIBSection(null, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (Ptr *)&data, null, 0),
                hMonoBitmap = CreateBitmap(src->w(), src->h(), 1, 1, null);

        if (data) {
            REPD(y, src->h())
            FREPD(x, src->w()) {
                Color c = src->color(x, y);
                (data++)->set(c.b, c.g, c.r, c.a);
            }

            ICONINFO ii;
            if (cursor_hot_spot) {
                ii.fIcon = false;
                ii.xHotspot = cursor_hot_spot->x;
                ii.yHotspot = cursor_hot_spot->y;
            } else {
                ii.fIcon = true;
                ii.xHotspot = 0; // this is ignored for icons
                ii.yHotspot = 0; // this is ignored for icons
            }
            ii.hbmMask = hMonoBitmap;
            ii.hbmColor = hBitmap;

            icon = CreateIconIndirect(&ii);
        }

        DeleteObject(hBitmap);
        DeleteObject(hMonoBitmap);

        src->unlock();
    }
    return icon;
}
#endif
/******************************************************************************/
} // namespace EE
/******************************************************************************/
