/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************/
enum IMAGERT_TYPE : Byte // Image Render Target Type, this describes a group of Image Types
{                        // P=10bit, H=16bit, F=32bit, S=Signed
    IMAGERT_SRGBA,       // IMAGE_R8G8B8A8_SRGB                                                                   (         32-bit total       with Alpha)
    IMAGERT_SRGB,        // IMAGE_R8G8B8A8_SRGB                                                                   (         32-bit total       no   Alpha)
    IMAGERT_SRGB_P,      // IMAGE_F16_3, IMAGE_F16_4, IMAGE_R8G8B8A8_SRGB                                         (at least 10-bit per channel no   Alpha)
    IMAGERT_SRGBA_H,     // IMAGE_F16_4, IMAGE_R8G8B8A8_SRGB                                                      (at least 16-bit per channel with Alpha)
    IMAGERT_SRGB_H,      // IMAGE_F16_3, IMAGE_F16_4, IMAGE_R8G8B8A8_SRGB                                         (at least 16-bit per channel no   Alpha)
    IMAGERT_SRGBA_F,     // IMAGE_F32_4, IMAGE_F16_4, IMAGE_R8G8B8A8_SRGB                                         (at least 32-bit per channel with Alpha)
    IMAGERT_SRGB_F,      // IMAGE_F32_3, IMAGE_F32_4, IMAGE_F16_3, IMAGE_F16_4, IMAGE_R8G8B8A8_SRGB               (at least 32-bit per channel no   Alpha)

    IMAGERT_RGBA,   // IMAGE_R8G8B8A8                                                                        (         32-bit total       with Alpha)
    IMAGERT_RGB,    // IMAGE_R10G10B10A2, IMAGE_R8G8B8A8                                                     (         32-bit total       no   Alpha)
    IMAGERT_RGB_P,  // IMAGE_R10G10B10A2, IMAGE_F16_3, IMAGE_F16_4, IMAGE_R8G8B8A8                           (at least 10-bit per channel no   Alpha)
    IMAGERT_RGBA_H, // IMAGE_F16_4, IMAGE_R8G8B8A8                                                           (at least 16-bit per channel with Alpha)
    IMAGERT_RGB_H,  // IMAGE_F16_3, IMAGE_F16_4, IMAGE_R10G10B10A2, IMAGE_R8G8B8A8                           (at least 16-bit per channel no   Alpha)
    IMAGERT_RGBA_F, // IMAGE_F32_4, IMAGE_F16_4, IMAGE_R8G8B8A8                                              (at least 32-bit per channel with Alpha)
    IMAGERT_RGB_F,  // IMAGE_F32_3, IMAGE_F32_4, IMAGE_F16_3, IMAGE_F16_4, IMAGE_R10G10B10A2, IMAGE_R8G8B8A8 (at least 32-bit per channel no   Alpha)

    IMAGERT_RGBA_S, // IMAGE_R8G8B8A8_SIGN, IMAGE_F16_4                                                      (signed   32-bit total       with Alpha)
    IMAGERT_F32,    // IMAGE_F32, IMAGE_F16
    IMAGERT_F16,    // IMAGE_F16, IMAGE_F32
    IMAGERT_ONE,    // IMAGE_R8     , IMAGE_F16, IMAGE_R8G8     , IMAGE_F32  , IMAGE_R8G8B8A8
    IMAGERT_ONE_S,  // IMAGE_R8_SIGN, IMAGE_F16, IMAGE_R8G8_SIGN, IMAGE_F32  , IMAGE_R8G8B8A8_SIGN
    IMAGERT_TWO,    //                           IMAGE_R8G8     , IMAGE_F16_2, IMAGE_R8G8B8A8
    IMAGERT_TWO_S,  //                           IMAGE_R8G8_SIGN, IMAGE_F16_2, IMAGE_R8G8B8A8_SIGN
    IMAGERT_TWO_H,  // IMAGE_F16_2, IMAGE_F32_2
    IMAGERT_DS,     // IMAGE_D24S8, IMAGE_D24X8, IMAGE_D32, IMAGE_D32S8X24, IMAGE_D16
    IMAGERT_DS_S,   // IMAGE_D24S8, IMAGE_D32S8X24
    IMAGERT_NUM,    // number of Image render targets

    IMAGERT_RGBA_P = IMAGERT_RGBA_H, // (at least        10-bit per channel with Alpha)

    IMAGERT_SRGBA_P = IMAGERT_SRGBA_H, // (at least        10-bit per channel with Alpha)

    IMAGERT_RGBA_SP = IMAGERT_RGBA_H, // (at least signed 10-bit per channel with Alpha)
    IMAGERT_RGB_S = IMAGERT_RGBA_S,   // (         signed 32-bit total       no   Alpha)

    IMAGERT_RGB_A2_H = IMAGERT_RGBA_H,
    IMAGERT_RGB_A2_S = IMAGERT_RGBA_S,
    IMAGERT_RGB_A2 = IMAGERT_RGB,
};
/******************************************************************************/
#if EE_PRIVATE
struct ImageRTDesc // Render Target Description
{
    VecI2 size;
    Byte samples;
    IMAGERT_TYPE rt_type;

    ImageRTDesc &type(IMAGERT_TYPE rt_type) {
        T.rt_type = rt_type;
        return T;
    }

    ImageRTDesc() {}
    explicit ImageRTDesc(Int w, Int h, IMAGERT_TYPE rt_type, Byte samples = 1) {
        size.set(w, h);
        T.rt_type = rt_type;
        T.samples = samples;
    }

    // private:
    IMAGE_TYPE _type;
};
#endif
/******************************************************************************/
struct ImageRT : Image // Image Render Target
{
    Bool create(C VecI2 &size, IMAGE_TYPE type, IMAGE_MODE mode = IMAGE_RT, Byte samples = 1);         // create, false on fail
    ImageRT &mustCreate(C VecI2 &size, IMAGE_TYPE type, IMAGE_MODE mode = IMAGE_RT, Byte samples = 1); // create, Exit  on fail
#if EE_PRIVATE
    Bool depthTexture() C;
    constexpr INLINE Bool hasUAV() C {
#if DX11
        return _uav != null;
#elif GL
        return _txtr != 0;
#endif
        return false;
    }
    constexpr INLINE Bool canSwapSRV() C {
#if DX11
        return _srv_srgb != null;
#elif GL
        return _txtr_srgb != 0;
#endif
        return false;
    }
    constexpr INLINE Bool canSwapRTV() C {
#if DX11
        return _rtv_srgb != null;
#elif GL
        return _txtr_srgb != 0;
#endif
        return false;
    }
    constexpr INLINE Bool canSwapSRGB() C { return canSwapSRV() && canSwapRTV(); }

    void zero();
    void delThis();
    void del();
    Bool createEx() = delete;
    Bool createViews();
    Bool map();
    void unmap();
    void swapSRV();
    void swapRTV();
    void swapSRGB();

#if DX11
    void clearHw(C Vec4 &color = Vec4Zero); // hardware render target  clear
    void clearDS(Byte s = 0);               // hardware depth  stencil clear
#else
// there's no 'clearHw' on OpenGL
// there's no 'clearDS' on OpenGL
#endif
    void clearFull(C Vec4 &color = Vec4Zero, Bool restore_rt = false);     // clear full          area
    void clearViewport(C Vec4 &color = Vec4Zero, Bool restore_rt = false); // clear main viewport area

    void discard();
#endif

    ImageRT();
    ~ImageRT();
#if !EE_PRIVATE
  private:
#endif
    union {
        struct
        {
#if EE_PRIVATE && DX11
            ID3D11ShaderResourceView *_srv_srgb;
            ID3D11RenderTargetView *_rtv, *_rtv_srgb;
            ID3D11DepthStencilView *_dsv, *_rdsv;
            ID3D11UnorderedAccessView *_uav;
#else
            Ptr _ptr[6];
#endif
        };
        struct
        {
#if EE_PRIVATE && GL
            UInt _txtr_srgb;
#else
            UInt _uint;
#endif
        };
    };
    NO_COPY_CONSTRUCTOR(ImageRT);
};
/******************************************************************************/
struct ImageRTC : ImageRT // Image Render Target Counted
{
#if EE_PRIVATE
    Bool available() C { return _ptr_num == 0; } // if this image is not currently used
#endif
#if !EE_PRIVATE
  private:
#endif
    UInt _ptr_num = 0; // this shouldn't be modified in any 'del', 'create' method
};
/******************************************************************************/
struct ImageRTPtr // Render Target Pointer
{
#if EE_PRIVATE
    Bool find(C ImageRTDesc &desc);       // find Render Target, false on fail
    ImageRTPtr &get(C ImageRTDesc &desc); // find Render Target, Exit  on fail
    ImageRTPtr &getDS(Int w, Int h, Byte samples = 1, Bool reuse_main = true);
#endif
    Bool find(Int w, Int h, IMAGERT_TYPE rt_type, Byte samples = 1);       // find Render Target, false on fail, 'samples'=number of samples per-pixel (allows multi-sampling)
    ImageRTPtr &get(Int w, Int h, IMAGERT_TYPE rt_type, Byte samples = 1); // find Render Target, Exit  on fail, 'samples'=number of samples per-pixel (allows multi-sampling)

    ImageRTC *operator()() C { return T._data; }                      // access the data, you can use the returned reference as long as this 'ImageRTPtr' object exists and not modified
    ImageRTC *operator->() C { return T._data; }                      // access the data, you can use the returned reference as long as this 'ImageRTPtr' object exists and not modified
    ImageRTC &operator*() C { return *T._data; }                      // access the data, you can use the returned reference as long as this 'ImageRTPtr' object exists and not modified
    Bool operator==(null_t) C { return T._data == null; }             // if pointers are equal
    Bool operator!=(null_t) C { return T._data != null; }             // if pointers are different
    Bool operator==(ImageRTC *p) C { return T._data == p; }           // if pointers are equal
    Bool operator!=(ImageRTC *p) C { return T._data != p; }           // if pointers are different
    Bool operator==(C ImageRTC *p) C { return T._data == p; }         // if pointers are equal
    Bool operator!=(C ImageRTC *p) C { return T._data != p; }         // if pointers are different
    Bool operator==(C ImageRTPtr &p) C { return T._data == p._data; } // if pointers are equal
    Bool operator!=(C ImageRTPtr &p) C { return T._data != p._data; } // if pointers are different
    operator Bool() C { return T._data != null; }                     // if pointer  is  valid
    operator ImageRTC *() C { return T._data; }

    ImageRTPtr &clear();                              // clear the pointer to null, this automatically decreases the reference count of current data
    ImageRTPtr &operator=(null_t) { return clear(); } // clear the pointer to null, this automatically decreases the reference count of current data
    ImageRTPtr &operator=(C ImageRTPtr &p);           // set       pointer to 'p' , this automatically decreases the reference count of current data and increases the reference count of the new data
    ImageRTPtr &operator=(ImageRTC *p);               // set       pointer to 'p' , this automatically decreases the reference count of current data and increases the reference count of the new data

    ImageRTPtr(null_t = null) {
        _data = null;
        _last_index = -1;
    }
    ImageRTPtr(C ImageRTPtr &p);
    ImageRTPtr(ImageRTC *p);
#if EE_PRIVATE
    ImageRTPtr &clearNoDiscard(); // clear the pointer to null, this automatically decreases the reference count of current data, without discarding
    explicit ImageRTPtr(C ImageRTDesc &desc) {
        _data = null;
        _last_index = -1;
        get(desc);
    }
#endif
    ~ImageRTPtr() { clear(); }

#if !EE_PRIVATE
  private:
#endif
    ImageRTC *_data;
    Int _last_index;
};
/******************************************************************************/
#if EE_PRIVATE
struct ImageRTPtrRef {
    ImageRTPtr &ref;

    ImageRTPtr &get(C ImageRTDesc &desc) { return ref.get(desc); }

    ImageRTC *operator()() C { return ref; }
    ImageRTC *operator->() C { return ref; }
    ImageRTC &operator*() C { return *ref; }
    operator ImageRTPtr &() C { return ref; }
    operator ImageRTC *() C { return ref; }

    void clear() { ref.clear(); }

    explicit ImageRTPtrRef(ImageRTPtr &image_rt_ptr) : ref(image_rt_ptr) {}
    ~ImageRTPtrRef() { ref.clear(); }

    NO_COPY_CONSTRUCTOR(ImageRTPtrRef);
};

IMAGERT_TYPE GetImageRTTypeLinear(Bool alpha, IMAGE_PRECISION precision);
IMAGERT_TYPE GetImageRTType(Bool alpha, IMAGE_PRECISION precision);
IMAGERT_TYPE GetImageRTType(IMAGE_TYPE type);
IMAGERT_TYPE GetImageRTType(IMAGE_TYPE type, Bool allow_alpha);
IMAGERT_TYPE GetImageRTType(IMAGE_TYPE type, Bool allow_alpha, IMAGE_PRECISION max_precision);
void ResetImageTypeCreateResult();
#endif
/******************************************************************************/
