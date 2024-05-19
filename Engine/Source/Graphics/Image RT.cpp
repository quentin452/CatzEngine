/******************************************************************************/
#include "../Platforms/iOS/iOS.h"
#include "stdafx.h"
namespace EE {
/******************************************************************************/
#define KNOWN_IMAGE_TYPE_USAGE (DX11)
/******************************************************************************/
struct ImageRTType {
    IMAGE_TYPE types[6]; // 6 is max IMAGE_TYPE elms per ImageRTType
};
static const ImageRTType ImageRTTypes[] =
    {
#if LINEAR_GAMMA
        {IMAGE_R8G8B8A8_SRGB},                                                     // 0 IMAGERT_SRGBA
        {IMAGE_R8G8B8A8_SRGB},                                                     // 1 IMAGERT_SRGB   (can't use IMAGE_R10G10B10A2 because it's not sRGB and will have low quality in dark colors, can't use IMAGE_R11G11B10F because precision for 0..1 is very bad, with a max error of 0.0153656006, can't use IMAGE_R9G9B9E5F because it's not supported for RT)
        {IMAGE_F16_3, IMAGE_F16_4, IMAGE_R8G8B8A8_SRGB},                           // 2 IMAGERT_SRGB_P (can't use IMAGE_R10G10B10A2 because it's not sRGB and will have low quality in dark colors, can't use IMAGE_R11G11B10F because precision for 0..1 is very bad, with a max error of 0.0153656006, can't use IMAGE_R9G9B9E5F because it's not supported for RT)
        {IMAGE_F16_4, IMAGE_R8G8B8A8_SRGB},                                        // 3 IMAGERT_SRGBA_H
        {IMAGE_F16_3, IMAGE_F16_4, IMAGE_R8G8B8A8_SRGB},                           // 4 IMAGERT_SRGB_H
        {IMAGE_F32_4, IMAGE_F16_4, IMAGE_R8G8B8A8_SRGB},                           // 5 IMAGERT_SRGBA_F
        {IMAGE_F32_3, IMAGE_F32_4, IMAGE_F16_3, IMAGE_F16_4, IMAGE_R8G8B8A8_SRGB}, // 6 IMAGERT_SRGB_F
#else
        {IMAGE_R8G8B8A8},                                                                        // 0 IMAGERT_SRGBA
        {IMAGE_R10G10B10A2, IMAGE_R8G8B8A8},                                                     // 1 IMAGERT_SRGB
        {IMAGE_R10G10B10A2, IMAGE_F16_3, IMAGE_F16_4, IMAGE_R8G8B8A8},                           // 2 IMAGERT_SRGB_P (can't use IMAGE_R11G11B10F because precision for 0..1 is very bad, with a max error of 0.0153656006)
        {IMAGE_F16_4, IMAGE_R8G8B8A8},                                                           // 3 IMAGERT_SRGBA_H
        {IMAGE_F16_3, IMAGE_F16_4, IMAGE_R10G10B10A2, IMAGE_R8G8B8A8},                           // 4 IMAGERT_SRGB_H
        {IMAGE_F32_4, IMAGE_F16_4, IMAGE_R8G8B8A8},                                              // 5 IMAGERT_SRGBA_F
        {IMAGE_F32_3, IMAGE_F32_4, IMAGE_F16_3, IMAGE_F16_4, IMAGE_R10G10B10A2, IMAGE_R8G8B8A8}, // 6 IMAGERT_SRGB_F
#endif

        {IMAGE_R8G8B8A8},                                                                        //  7 IMAGERT_RGBA
        {IMAGE_R10G10B10A2, IMAGE_R8G8B8A8},                                                     //  8 IMAGERT_RGB
        {IMAGE_R10G10B10A2, IMAGE_F16_3, IMAGE_F16_4, IMAGE_R8G8B8A8},                           //  9 IMAGERT_RGB_P (can't use IMAGE_R11G11B10F because precision for 0..1 is very bad, with a max error of 0.0153656006)
        {IMAGE_F16_4, IMAGE_R8G8B8A8},                                                           // 10 IMAGERT_RGBA_H
        {IMAGE_F16_3, IMAGE_F16_4, IMAGE_R10G10B10A2, IMAGE_R8G8B8A8},                           // 11 IMAGERT_RGB_H
        {IMAGE_F32_4, IMAGE_F16_4, IMAGE_R8G8B8A8},                                              // 12 IMAGERT_RGBA_F
        {IMAGE_F32_3, IMAGE_F32_4, IMAGE_F16_3, IMAGE_F16_4, IMAGE_R10G10B10A2, IMAGE_R8G8B8A8}, // 13 IMAGERT_RGB_F

        {IMAGE_R8G8B8A8_SIGN, IMAGE_F16_4}, // 14 IMAGERT_RGBA_S

        {IMAGE_F32, IMAGE_F16}, // 15 IMAGERT_F32
        {IMAGE_F16, IMAGE_F32}, // 16 IMAGERT_F16

        {IMAGE_R8, IMAGE_F16, IMAGE_R8G8, IMAGE_F32, IMAGE_R8G8B8A8},                // 17 IMAGERT_ONE
        {IMAGE_R8_SIGN, IMAGE_F16, IMAGE_R8G8_SIGN, IMAGE_F32, IMAGE_R8G8B8A8_SIGN}, // 18 IMAGERT_ONE_S
        {IMAGE_R8G8, IMAGE_F16_2, IMAGE_R8G8B8A8},                                   // 19 IMAGERT_TWO
        {IMAGE_R8G8_SIGN, IMAGE_F16_2, IMAGE_R8G8B8A8_SIGN},                         // 20 IMAGERT_TWO_S
        {IMAGE_F16_2, IMAGE_F32_2},                                                  // 21 IMAGERT_TWO_H

        {IMAGE_D24S8, IMAGE_D24X8, IMAGE_D32, IMAGE_D32S8X24, IMAGE_D16}, // 22 IMAGERT_DS
        {IMAGE_D24S8, IMAGE_D32S8X24},                                    // 23 IMAGERT_DS_S
};
ASSERT(IMAGERT_SRGBA == 0 && IMAGERT_SRGB == 1 && IMAGERT_SRGB_P == 2 && IMAGERT_SRGBA_H == 3 && IMAGERT_SRGB_H == 4 && IMAGERT_SRGBA_F == 5 && IMAGERT_SRGB_F == 6 && IMAGERT_RGBA == 7 && IMAGERT_RGB == 8 && IMAGERT_RGB_P == 9 && IMAGERT_RGBA_H == 10 && IMAGERT_RGB_H == 11 && IMAGERT_RGBA_F == 12 && IMAGERT_RGB_F == 13 && IMAGERT_RGBA_S == 14 && IMAGERT_F32 == 15 && IMAGERT_F16 == 16 && IMAGERT_ONE == 17 && IMAGERT_ONE_S == 18 && IMAGERT_TWO == 19 && IMAGERT_TWO_S == 20 && IMAGERT_TWO_H == 21 && IMAGERT_DS == 22 && IMAGERT_DS_S == 23 && IMAGERT_NUM == 24 && Elms(ImageRTTypes) == IMAGERT_NUM);

static CChar8 *ImageRTName[] =
    {
        "SRGBA",   // 0
        "SRGB",    // 1
        "SRGB_P",  // 2
        "SRGBA_H", // 3
        "SRGB_H",  // 4
        "SRGBA_F", // 5
        "SRGB_F",  // 6

        "RGBA",   // 7
        "RGB",    // 8
        "RGB_P",  // 9
        "RGBA_H", // 10
        "RGB_H",  // 11
        "RGBA_F", // 12
        "RGB_F",  // 13

        "RGBA_S", // 14
        "F32",    // 15
        "F16",    // 16
        "ONE",    // 17
        "ONE_S",  // 18
        "TWO",    // 19
        "TWO_S",  // 20
        "TWO_H",  // 21
        "DS",     // 22
        "DS_S",   // 23
};
ASSERT(IMAGERT_SRGBA == 0 && IMAGERT_SRGB == 1 && IMAGERT_SRGB_P == 2 && IMAGERT_SRGBA_H == 3 && IMAGERT_SRGB_H == 4 && IMAGERT_SRGBA_F == 5 && IMAGERT_SRGB_F == 6 && IMAGERT_RGBA == 7 && IMAGERT_RGB == 8 && IMAGERT_RGB_P == 9 && IMAGERT_RGBA_H == 10 && IMAGERT_RGB_H == 11 && IMAGERT_RGBA_F == 12 && IMAGERT_RGB_F == 13 && IMAGERT_RGBA_S == 14 && IMAGERT_F32 == 15 && IMAGERT_F16 == 16 && IMAGERT_ONE == 17 && IMAGERT_ONE_S == 18 && IMAGERT_TWO == 19 && IMAGERT_TWO_S == 20 && IMAGERT_TWO_H == 21 && IMAGERT_DS == 22 && IMAGERT_DS_S == 23 && IMAGERT_NUM == 24 && Elms(ImageRTName) == IMAGERT_NUM);

#if KNOWN_IMAGE_TYPE_USAGE
static IMAGE_TYPE ImageRTTypesOK[2][IMAGERT_NUM]; // [MultiSample][IMAGERT_NUM], this keeps info about result of creating different IMAGE_TYPE for 1-sample and multi-sample, this is because some formats may fail to create multi-sampled but succeed with 1-sample
#else
static ImageRTType ImageRTTypesOK[2][IMAGERT_NUM];
#endif
/******************************************************************************/
static const IMAGERT_TYPE GetImageRTTypeLookup[IMAGE_PRECISION_NUM][2] = // [precision][alpha]
    {
        // no alpha   ,     alpha
        {IMAGERT_SRGB, IMAGERT_SRGBA},     // 0 IMAGE_PRECISION_8
        {IMAGERT_SRGB_P, IMAGERT_SRGBA_P}, // 1 IMAGE_PRECISION_10
        {IMAGERT_SRGB_H, IMAGERT_SRGBA_H}, // 2 IMAGE_PRECISION_16
        {IMAGERT_SRGB_F, IMAGERT_SRGBA_F}, // 3 IMAGE_PRECISION_24
        {IMAGERT_SRGB_F, IMAGERT_SRGBA_F}, // 4 IMAGE_PRECISION_32
        {IMAGERT_SRGB_F, IMAGERT_SRGBA_F}, // 5 IMAGE_PRECISION_64
};
ASSERT(IMAGE_PRECISION_8 == 0 && IMAGE_PRECISION_10 == 1 && IMAGE_PRECISION_16 == 2 && IMAGE_PRECISION_24 == 3 && IMAGE_PRECISION_32 == 4 && IMAGE_PRECISION_64 == 5 && IMAGE_PRECISION_NUM == 6);

static const IMAGERT_TYPE GetImageRTTypeLookupLinear[IMAGE_PRECISION_NUM][2] = // [precision][alpha]
    {
        // no alpha   ,     alpha
        {IMAGERT_RGB, IMAGERT_RGBA},     // 0 IMAGE_PRECISION_8
        {IMAGERT_RGB_P, IMAGERT_RGBA_P}, // 1 IMAGE_PRECISION_10
        {IMAGERT_RGB_H, IMAGERT_RGBA_H}, // 2 IMAGE_PRECISION_16
        {IMAGERT_RGB_F, IMAGERT_RGBA_F}, // 3 IMAGE_PRECISION_24
        {IMAGERT_RGB_F, IMAGERT_RGBA_F}, // 4 IMAGE_PRECISION_32
        {IMAGERT_RGB_F, IMAGERT_RGBA_F}, // 5 IMAGE_PRECISION_64
};
ASSERT(IMAGE_PRECISION_8 == 0 && IMAGE_PRECISION_10 == 1 && IMAGE_PRECISION_16 == 2 && IMAGE_PRECISION_24 == 3 && IMAGE_PRECISION_32 == 4 && IMAGE_PRECISION_64 == 5 && IMAGE_PRECISION_NUM == 6);

IMAGERT_TYPE GetImageRTTypeLinear(Bool alpha, IMAGE_PRECISION precision) { return GetImageRTTypeLookupLinear[precision][alpha]; }
IMAGERT_TYPE GetImageRTType(Bool alpha, IMAGE_PRECISION precision) { return GetImageRTTypeLookup[precision][alpha]; }
IMAGERT_TYPE GetImageRTType(IMAGE_TYPE type, Bool allow_alpha, IMAGE_PRECISION max_precision) {
    C ImageTypeInfo &ti = ImageTI[type];
    return GetImageRTType(ti.a >= 8 && allow_alpha, Min(ti.precision, max_precision)); // compare alpha as >=8 instead of >0 to treat types such as IMAGE_R10G10B10A2 as without alpha (this type is chosen for 10-bit color and ignoring alpha), because we could actually increase precision when operating on 'IMAGE_R10G10B10A2' with 'allow_alpha'=true
}
IMAGERT_TYPE GetImageRTType(IMAGE_TYPE type, Bool allow_alpha) {
    C ImageTypeInfo &ti = ImageTI[type];
    return GetImageRTType(ti.a >= 8 && allow_alpha, ti.precision); // compare alpha as >=8 instead of >0 to treat types such as IMAGE_R10G10B10A2 as without alpha (this type is chosen for 10-bit color and ignoring alpha), because we could actually increase precision when operating on 'IMAGE_R10G10B10A2' with 'allow_alpha'=true
}
IMAGERT_TYPE GetImageRTType(IMAGE_TYPE type) {
    C ImageTypeInfo &ti = ImageTI[type];
    return GetImageRTType(ti.a >= 8, ti.precision); // compare alpha as >=8 instead of >0 to treat types such as IMAGE_R10G10B10A2 as without alpha (this type is chosen for 10-bit color and ignoring alpha), because we could actually increase precision when operating on 'IMAGE_R10G10B10A2' with 'allow_alpha'=true
}
/******************************************************************************/
void ResetImageTypeCreateResult() {
#if KNOWN_IMAGE_TYPE_USAGE
    REPD(rt_type, IMAGERT_NUM) // process all IMAGERT's
    {
        C ImageRTType &src = ImageRTTypes[rt_type];
        REPD(ms, 2) // have to separately for multi-sampled
        {
            IMAGE_TYPE &dest = ImageRTTypesOK[ms][rt_type];
            UInt flag = (ms ? ImageTypeInfo::USAGE_IMAGE_MS : (ImageTypeInfo::USAGE_IMAGE_RT | ImageTypeInfo::USAGE_IMAGE_DS)); // for multi-sampling 'ms' we need USAGE_IMAGE_MS (this implies multi-sampled RT or DS), otherwise RT or DS is enough
            FREPA(src.types) {
                IMAGE_TYPE type = src.types[i];
                if (ImageTI[type].usage() & flag) {
                    dest = type;
                    goto set;
                }
            } // find first matching
            dest = IMAGE_NONE;
        set:;
        }
    }
#else
    CopyFast(ImageRTTypesOK[0], ImageRTTypes);
    CopyFast(ImageRTTypesOK[1], ImageRTTypes);
#endif
}
Bool ImageRT::depthTexture() C {
#if GL
    return mode() == IMAGE_DS;
#else
    return _dsv && _srv;
#endif
}
/******************************************************************************/
#define DEBUG_DISCARD 0
#if DEBUG_DISCARD
#pragma message("!! Warning: Use this only for debugging !!")
#endif
void ImageRT::discard() {
#if DX11 && TILE_BASED_GPU // disable on non-tile-based GPU as it actually decreases performance (as tested on GeForce 1050 Ti TODO: check again in the future)
    if (DEBUG_DISCARD)
        clearHw(PURPLE);
    else if (D3DC1)
        D3DC1->DiscardView(_rtv ? &SCAST(ID3D11View, *_rtv) : &SCAST(ID3D11View, *_dsv)); // will not crash if parameter is null
#elif GL && !MAC && !LINUX                                                                // Mac doesn't have GL 4.3 'glInvalidateFramebuffer', Linux GeForce drivers have bugs (TODO: check again in the future)
    if (DEBUG_DISCARD) {
        if (Renderer._cur_ds == this && Renderer._cur_ds_id == _txtr)
            D.clearDS();
        else if (Renderer._cur[0] == this)
            D.clearCol(0, PURPLE);
        else if (Renderer._cur[1] == this)
            D.clearCol(1, PURPLE);
        else if (Renderer._cur[2] == this)
            D.clearCol(2, PURPLE);
        else if (Renderer._cur[3] == this)
            D.clearCol(3, PURPLE);
        else {
            _discard = true;
            return;
        }                 // discard at next opportunity when we want to attach it to FBO
        _discard = false; // discarded
    } else
#if WINDOWS
        if (glInvalidateFramebuffer) // requires GL 4.3, GL ES 3.0
#endif
    {
        // this should be called only if this image is already attached to current FBO - https://community.arm.com/graphics/b/blog/posts/mali-performance-2-how-to-correctly-handle-framebuffers
        // 'glInvalidateFramebuffer' can be called at the start of rendering (right after attaching to   FBO) to specify that we don't need previous     contents of this RT
        //                                     and at the end   of rendering (     BEFORE detaching from FBO) to specify that we don't need to store the contents of this RT
        // !! remember that images can have only '_txtr' or '_rb' or nothing at all (if they're provided by the system) but we still should discard them !!
        if (!IOS && D.mainFBO())          // iOS doesn't have main FBO
        {                                 // for main FBO we need to setup different values - https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glInvalidateFramebuffer.xhtml
            if (Renderer._cur_ds == this) // no need to check '_cur_ds_id' because main FBO always has texture 0
            {
                GLenum attachments[] = {GL_DEPTH, GL_STENCIL};
                glInvalidateFramebuffer(GL_FRAMEBUFFER, hwTypeInfo().s ? 2 : 1, attachments);
                _discard = false;
            } else if (Renderer._cur[0] == this) // check '_cur' because '_txtr' can be 0 for RenderBuffers
            {
                GLenum attachment = GL_COLOR;
                glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);
                _discard = false;
            } else
                _discard = true; // discard at next opportunity when we want to attach it to FBO
        } else {
            GLenum attachment;
            if (Renderer._cur_ds == this && Renderer._cur_ds_id == _txtr)
                attachment = (hwTypeInfo().s ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT);
            else // check both '_cur_ds' and '_cur_ds_id' because '_cur_ds_id' will be 0 when Image is a RenderBuffer or temporarily unbound Texture (only Textures can be temporarily unbound), this will work OK for RenderBuffers because both '_cur_ds_id' and '_txtr' will be zero
                if (Renderer._cur[0] == this)
                    attachment = GL_COLOR_ATTACHMENT0;
                else // check '_cur' because '_txtr' can be 0 for RenderBuffers
                    if (Renderer._cur[1] == this)
                        attachment = GL_COLOR_ATTACHMENT1;
                    else // check '_cur' because '_txtr' can be 0 for RenderBuffers
                        if (Renderer._cur[2] == this)
                            attachment = GL_COLOR_ATTACHMENT2;
                        else // check '_cur' because '_txtr' can be 0 for RenderBuffers
                            if (Renderer._cur[3] == this)
                                attachment = GL_COLOR_ATTACHMENT3;
                            else // check '_cur' because '_txtr' can be 0 for RenderBuffers
                            {
                                _discard = true;
                                return;
                            } // discard at next opportunity when we want to attach it to FBO
            glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);
            _discard = false; // discarded
        }
    }
#endif
}
#if DX11
void ImageRT::clearHw(C Vec4 &color) {
    if (_rtv)
        D3DC->ClearRenderTargetView(_rtv, color.c);
}
void ImageRT::clearDS(Byte s) {
    if (_dsv)
        D3DC->ClearDepthStencilView(_dsv, D3D11_CLEAR_DEPTH | (hwTypeInfo().s ? D3D11_CLEAR_STENCIL : 0), 1, s);
}
#endif
/******************************************************************************/
void ImageRT::clearFull(C Vec4 &color, Bool restore_rt) {
#if DX11
    clearHw(color);
#else
    ImageRT *rt[Elms(Renderer._cur)], *ds;
    Bool restore_viewport;
    if (restore_rt) {
        REPAO(rt) = Renderer._cur[i];
        ds = Renderer._cur_ds;
        restore_viewport = !D._view_active.full;
    }

    discard();
    Renderer.set(this, null, false);
    D.clearCol(color);

    if (restore_rt)
        Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, restore_viewport);
#endif
}
void ImageRT::clearViewport(C Vec4 &color, Bool restore_rt) {
    if (D._view_main.full)
        clearFull(color, restore_rt);
    else {
        ImageRT *rt[Elms(Renderer._cur)], *ds;
        Bool restore_viewport;
        if (restore_rt) {
            REPAO(rt) = Renderer._cur[i];
            ds = Renderer._cur_ds;
            restore_viewport = !D._view_active.full;
        }

        Renderer.set(this, null, true);
        D.clearCol(color);

        if (restore_rt)
            Renderer.set(rt[0], rt[1], rt[2], rt[3], ds, restore_viewport);
    }
}
/******************************************************************************/
void ImageRT::zero() {
#if DX11
    _srv_srgb = null;
    _rtv = _rtv_srgb = null;
    _dsv = _rdsv = null;
    _uav = null;
#elif GL
    _txtr_srgb = 0;
#endif
}
ImageRT::ImageRT() { zero(); }
ImageRT::~ImageRT() { delThis(); } // delete children first, 'super.del' already called automatically in '~Image'
void ImageRT::delThis()            // delete only this class members without super
{
    D.rtClear(T);
#if DX11
    if (_srv_srgb || _rtv || _rtv_srgb || _dsv || _rdsv || _uav) {
        D.texClearAll(_srv_srgb);
        D.uavClear(_uav);
#if GPU_LOCK // lock not needed for 'Release'
        SyncLocker locker(D._lock);
#endif
        if (D.created()) {
            RELEASE(_srv_srgb);
            RELEASE(_rtv);
            RELEASE(_rtv_srgb);
            RELEASE(_dsv);
            RELEASE(_rdsv);
            RELEASE(_uav);
        }
    }
#elif GL
    D.uavClear(_txtr);
    if (_txtr_srgb) {
        D.uavClear(_txtr_srgb);
        D.texClearAll(_txtr_srgb);
#if GPU_LOCK
        SyncLocker locker(D._lock);
#endif
        if (D.created()) {
            glDeleteTextures(1, &_txtr_srgb);
        }
    }
#endif
    zero();
}
void ImageRT::del() {
    delThis(); // delete children first
    super::del();
}
Bool ImageRT::createViews() {
    switch (mode()) {
    case IMAGE_RT: // case IMAGE_RT_CUBE:
    {
#if DX11
        if (_rtv)
            return true; // if already has a view then keep it, this is important and needed for #SwapImageRT
        D3D11_RENDER_TARGET_VIEW_DESC rtvd;
        Zero(rtvd);
        rtvd.Format = hwTypeInfo().format;
        rtvd.ViewDimension = (multiSample() ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D);
#if GPU_LOCK // lock not needed for 'D3D'
        SyncLocker locker(D._lock);
#endif
        D3D->CreateRenderTargetView(_txtr, &rtvd, &_rtv);
        if (!_rtv)
            return false;

        if (D.computeAvailable()) {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
            Zero(uavd);
            uavd.Format = hwTypeInfo().format;
            uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            // uavd.Texture2D.MipSlice=0;
            D3D->CreateUnorderedAccessView(_txtr, &uavd, &_uav); // if(!_uav)return false; this might fail for map main
        }
#elif GL
        if (_txtr_srgb)
            return true; // if already has a view then keep it, this is important and needed for #SwapImageRT
#endif

        if (IMAGE_TYPE type_srgb = ImageTypeToggleSRGB(type()))
            if (type_srgb != type()) // try creating toggled sRGB Resource Views
            {
#if DX11
                D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
                Zero(srvd);
                if (multiSample()) {
                    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
                } else {
                    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                    srvd.Texture2D.MipLevels = mipMaps();
                }
                srvd.Format = rtvd.Format = ImageTI[type_srgb].format;
                D3D->CreateShaderResourceView(_txtr, &srvd, &_srv_srgb);
                D3D->CreateRenderTargetView(_txtr, &rtvd, &_rtv_srgb);
#elif GL
                if (D.canSwapSRGB()) {
#if GPU_LOCK
                    SyncLocker locker(D._lock);
#endif
                    glGenTextures(1, &_txtr_srgb);
                    if (_txtr_srgb) {
                        glGetError(); // clear any previous errors
                        glTextureView(_txtr_srgb, GL_TEXTURE_2D, _txtr, ImageTI[type_srgb].format, 0, mipMaps(), 0, faces());
                        if (glGetError() != GL_NO_ERROR) {
                            glDeleteTextures(1, &_txtr_srgb);
                            _txtr_srgb = 0;
                        } // delete on fail
                    }
                }
#endif
            }
    } break;

    case IMAGE_DS:
    case IMAGE_SHADOW_MAP: {
#if DX11
        if (_dsv)
            return true; // if already has a view then keep it, this is important and needed for #SwapImageRT
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
        Zero(dsvd);
        dsvd.Format = hwTypeInfo().format;
        dsvd.ViewDimension = (multiSample() ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D);
#if GPU_LOCK // lock not needed for 'D3D'
        SyncLocker locker(D._lock);
#endif
        D3D->CreateDepthStencilView(_txtr, &dsvd, &_dsv);
        if (!_dsv)
            return false;
        dsvd.Flags = D3D11_DSV_READ_ONLY_DEPTH;
        if (hwTypeInfo().s)
            dsvd.Flags |= D3D11_DSV_READ_ONLY_STENCIL;
        D3D->CreateDepthStencilView(_txtr, &dsvd, &_rdsv); // this will work only on DX11.0 but not 10.0, 10.1
#endif
    } break;
    }
    return true;
}
Bool ImageRT::create(C VecI2 &size, IMAGE_TYPE type, IMAGE_MODE mode, Byte samples) {
    if (T.size() == size && T.type() == type && T.mode() == mode && T.samples() == samples // do a fast check if we already have what we want, to avoid re-creating the same stuff
        && createViews())
        return true; // still have to check for views, in case this was swapped from 'Image' and views are null #SwapImageRT

    delThis(); // delete only this without super 'del', because that will already be done in 'createEx' below
    if (super::createEx(size.x, size.y, 1, type, mode, 1, samples)
#if GL
        || mode == IMAGE_DS && super::createEx(size.x, size.y, 1, type, IMAGE_GL_RB, 1, samples)
#endif
    )
        if (createViews())
            return true;

    del();
    return false;
}
ImageRT &ImageRT::mustCreate(C VecI2 &size, IMAGE_TYPE type, IMAGE_MODE mode, Byte samples) {
    if (!create(size, type, mode, samples))
        Exit(S + "Can't create ImageRT " + size.x + 'x' + size.y + ", type " + ImageTI[type].name + ", mode " + mode + ", samples " + samples + '.');
    return T;
}
/******************************************************************************/
Bool ImageRT::map() {
#if DX11
    del();
    if (OK(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (Ptr *)&_txtr))) {
        _mode = IMAGE_RT;
        if (setInfo()) {
            adjustInfo(hwW(), hwH(), hwD(), hwType());
            if (createViews()) {
                if (LINEAR_GAMMA && hwType() == IMAGE_R8G8B8A8) // #SwapFlipSRGB may fail to create sRGB, in that case create as linear and 'swapRTV' in 'ImageRT.map'
                {
                    if (!canSwapRTV())
                        return false;
                    swapSRGB();
                    if (!_srv)
                        Swap(_srv, _srv_srgb);
                }
                return true;
            }
        }
    }
#elif DX12
https: // msdn.microsoft.com/en-us/library/windows/desktop/mt427784(v=vs.85).aspx
   In Direct3D 11, applications could call GetBuffer(0, .. ) only once. Every call to Present implicitly changed the resource identity of the returned interface. Direct3D 12 no longer supports that implicit resource identity change, due to the CPU overhead required and the flexible resource descriptor design. As a result, the application must manually call GetBuffer for every each buffer created with the swapchain. The application must manually render to the next buffer in the sequence after calling Present. Applications are encouraged to create a cache of descriptors for each buffer, instead of re-creating many objects each Present.
#elif IOS
    del();
    if (EAGLView *view = GetUIView()) {
        glGenRenderbuffers(1, &_rb);
        if (_rb) {
            glGetError(); // clear any previous errors
            glBindRenderbuffer(GL_RENDERBUFFER, _rb);
            [MainContext.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)view.layer];
            _mode = IMAGE_GL_RB;
            if (setInfo()) // this has a valid '_rb' so it can detect the size and type
            {
                adjustInfo(hwW(), hwH(), hwD(), hwType());
                D._res = size();
                D.densityUpdate();
                return true;
            }
        }
    }
#elif ANDROID || SWITCH || WEB
    // 'Renderer._main' has 'setInfo' called externally in the main loop
    return true;
#elif GL
    forceInfo(D.resW(), D.resH(), 1, type() ? type() : LINEAR_GAMMA ? IMAGE_R8G8B8A8_SRGB
                                                                    : IMAGE_R8G8B8A8,
              IMAGE_GL_RB, samples());
    return true;
#endif
    return false;
}
/******************************************************************************/
void ImageRT::unmap() {
#if DX11
    del();
#elif IOS
   if (_rb) {
       glBindRenderbuffer(GL_RENDERBUFFER, _rb);
       [MainContext.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:nil]; // detach existing renderbuffer from the drawable object
       del();
   }
#else
    // on other platforms we're not responsible for the 'Renderer._main' as the system creates it and deletes it, don't delete it here, to preserve info about IMAGE_TYPE and samples
#endif
}
/******************************************************************************/
void ImageRT::swapSRV() {
#if DX11
    Swap(_srv, _srv_srgb);
    D.texClear(_srv_srgb); // we have to remove from tex cache, because if we're going to try to bind this as Render Target later, then DX automatically unbinds its SRVs, engine already clears cache in that case, however only for current '_srv' and not the secondary '_srv_srgb'
#elif GL
   Swap(_txtr, _txtr_srgb);
#endif
}
void ImageRT::swapRTV() {
#if DX11
    Swap(_rtv, _rtv_srgb);
#elif GL
   Swap(_txtr, _txtr_srgb);
#endif
}
void ImageRT::swapSRGB() {
#if DX11
    swapSRV();
    swapRTV();
#elif GL
   Swap(_txtr, _txtr_srgb);
#endif
    _hw_type = ImageTypeToggleSRGB(hwType()); // !! have to toggle 'hwType' and not 'type' because 'CompareDesc' and 'Set' expect that !!
}
static Int CompareDesc(C ImageRTC &image, C ImageRTDesc &desc) {
    if (Int c = Compare(image.w(), desc.size.x))
        return c;
    if (Int c = Compare(image.h(), desc.size.y))
        return c;
    if (Int c = Compare(image.type(), desc._type))
        return c; // !! have to compare 'type' instead of 'hwType', because 'type' is always non-sRGB (same as 'desc._type') while 'hwType' can be sRGB-toggled in 'swapSRGB' !!
    if (Int c = Compare(image.samples(), desc.samples))
        return c;
    return 0;
}
static void Set(ImageRTPtr &p, ImageRTC &rt, IMAGE_TYPE want_type) // this is called only when "_ptr_num==0"
{
    if (want_type != rt.hwType()) // !! have to compare 'hwType' instead of 'type', because 'type' is always non-sRGB (same as 'desc._type') while 'hwType' can be sRGB-toggled in 'swapSRGB' !!
    {
        DYNAMIC_ASSERT(rt.canSwapSRGB(), "Can't swap RT sRGB");
        rt.swapSRGB();
    }
    rt._ptr_num++;
    p._data = &rt;
    rt.discard();
}
/******************************************************************************/
ImageRTPtr &ImageRTPtr::clearNoDiscard() {
    if (_data) {
        DEBUG_ASSERT(_data->_ptr_num, "ImageRTC._ptr_num should be >0");
        _data->_ptr_num--;
        _data = null;
    }
    return T;
}
ImageRTPtr &ImageRTPtr::clear() {
    if (_data) {
        DEBUG_ASSERT(_data->_ptr_num, "ImageRTC._ptr_num should be >0");
        _data->_ptr_num--;
        if (!_data->_ptr_num)
            _data->discard();
        _data = null;
    }
    return T;
}
ImageRTPtr &ImageRTPtr::operator=(C ImageRTPtr &p) {
    if (T != p) {
        clear();
        if (T._data = p._data) {
            T._data->_ptr_num++;
            T._last_index = p._last_index;
        } // assign to new
    }
    return T;
}
ImageRTPtr &ImageRTPtr::operator=(ImageRTC *p) {
    if (T != p) {
        clear();
        if (T._data = p)
            T._data->_ptr_num++; // assign to new
    }
    return T;
}
ImageRTPtr::ImageRTPtr(C ImageRTPtr &p) {
    if (T._data = p._data) {
        T._data->_ptr_num++;
        T._last_index = p._last_index;
    } else
        T._last_index = -1;
}
ImageRTPtr::ImageRTPtr(ImageRTC *p) {
    if (T._data = p)
        T._data->_ptr_num++;
    else
        T._last_index = -1;
}
/******************************************************************************/
Bool ImageRTPtr::find(C ImageRTDesc &desc) {
    clear(); // clear first so we can find the same Image if possible

    Bool multi_sample = (desc.samples > 1);
#if KNOWN_IMAGE_TYPE_USAGE
    ConstCast(desc._type) = ImageRTTypesOK[multi_sample][desc.rt_type];
#else
   ImageRTType &types = ImageRTTypesOK[multi_sample][desc.rt_type];
again:
   ConstCast(desc._type) = types.types[0];
#endif

    IMAGE_TYPE want_type = desc._type;
    if (D.canSwapSRGB()) // if can swap sRGB, then try to create non-sRGB, and later swap sRGB views
    {                    // this allows to potentially reduce amount of needed Render Targets, since they can reuse each other memory (instead of using 1 RGB and 1 sRGB, we can just allocate 1 RGB, and swap its views if needed)
        IMAGE_TYPE non_srgb = ImageTypeExcludeSRGB(desc._type);
        if (desc._type != non_srgb)
            ConstCast(desc._type) = non_srgb; // if we're requesting sRGB, then request non-sRGB
    }

    Bool found;
    if (InRange(_last_index, Renderer._rts) && !CompareDesc(Renderer._rts[_last_index], desc))
        found = true; // in range and matches ("!CompareDesc" -> match)
    else
        found = Renderer._rts.binarySearch(desc, _last_index, CompareDesc);

    if (found) {
        // check '_last_index' first
        ImageRTC &rt = Renderer._rts[_last_index];
        if (rt.available()) {
            Set(T, rt, want_type);
            return true;
        }

        // check all neighbors with the same desc
        for (Int i = _last_index - 1; i >= 0; i--) {
            ImageRTC &rt = Renderer._rts[i];
            if (CompareDesc(rt, desc))
                break;
            if (rt.available()) {
                T._last_index = i;
                Set(T, rt, want_type);
                return true;
            }
        }
        for (Int i = _last_index + 1; InRange(i, Renderer._rts); i++) {
            ImageRTC &rt = Renderer._rts[i];
            if (CompareDesc(rt, desc))
                break;
            if (rt.available()) {
                T._last_index = i;
                Set(T, rt, want_type);
                return true;
            }
        }
    }
#if KNOWN_IMAGE_TYPE_USAGE
    if (desc._type) // check this after 'found' because in most cases we will already return from codes above
    {               // since we have KNOWN_IMAGE_TYPE_USAGE, and a valid type, then we assume that this should always succeed
        ImageRTC &rt = Renderer._rts.NewAt(_last_index);
        if (rt.create(desc.size, desc._type, ImageTI[desc._type].d ? IMAGE_DS : IMAGE_RT, desc.samples)) {
            Set(T, rt, want_type);
            return true;
        }
        Exit(S + "Can't create Render Target " + desc.size.x + 'x' + desc.size.y + ' ' + ImageRTName[desc.rt_type] + ", samples:" + desc.samples);
    }
#else
   ImageRTC temp; // try to create first as a standalone variable (not in 'Renderer._rts') in case it fails so we don't have to remove it
   if (temp.create(desc.size, desc._type, ImageTI[desc._type].d ? IMAGE_DS : IMAGE_RT, desc.samples)) {
       ImageRTC &rt = Renderer._rts.NewAt(_last_index);
       Swap(rt, temp);
       Set(T, rt, want_type);
       return true;
   }
   // fail
   if (desc._type != IMAGE_NONE) // try another type, and don't try this again
   {
       MoveFastN(&types.types[0], &types.types[1], ELMS(types.types) - 1); // move all elements from index 1 and right, to the left by 1, to index 0..
       types.types[ELMS(types.types) - 1] = IMAGE_NONE;                    // set last type as none
       if (types.types[0] != IMAGE_NONE)
           goto again; // try the new type
   }
#endif
    return false;
}
/******************************************************************************/
ImageRTPtr &ImageRTPtr::get(C ImageRTDesc &desc) {
    if (!find(desc))
        Exit(S + "Can't create Render Target " + desc.size.x + 'x' + desc.size.y + ' ' + ImageRTName[desc.rt_type] + ", samples:" + desc.samples);
    return T;
}
ImageRTPtr &ImageRTPtr::getDS(Int w, Int h, Byte samples, Bool reuse_main) {
    clear(); // clear first so we can find the same Image if possible
    if (reuse_main) {
        ImageRTC &ds = Renderer._main_ds, *cur_ds = Renderer._cur_main_ds;
#if !GL // can't reuse '_main_ds' on GL because it would trigger 'D.mainFBO' and different orientations
        if (/*ds .available() && */ ds.accessible() && ds.w() == w && ds.h() == h && ds.samples() == samples) {
            T = &ds;
            return T;
        } // if ds is not used (actually don't check this because an 'ImageRTPtr ds' handle can be set to it at the start of each frame), accessible and compatible then we can use it
#endif
        if (cur_ds && cur_ds != &ds && /*cur_ds->available() && */ cur_ds->accessible() && cur_ds->w() == w && cur_ds->h() == h && cur_ds->samples() == samples) {
            T = cur_ds;
            return T;
        } // if ds is not used (actually don't check this because an 'ImageRTPtr ds' handle can be set to it at the start of each frame), accessible and compatible then we can use it
    }
    get(ImageRTDesc(w, h, IMAGERT_DS, samples));
    return T;
}
/******************************************************************************/
Bool ImageRTPtr::find(Int w, Int h, IMAGERT_TYPE rt_type, Byte samples) { return find(ImageRTDesc(w, h, rt_type, samples)); }
ImageRTPtr &ImageRTPtr::get(Int w, Int h, IMAGERT_TYPE rt_type, Byte samples) { return get(ImageRTDesc(w, h, rt_type, samples)); }
/******************************************************************************/
} // namespace EE
/******************************************************************************/
