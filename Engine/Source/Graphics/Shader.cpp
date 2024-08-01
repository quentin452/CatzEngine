// TODO ADD PROFILE_START AND PROFILE_STOP PROFILERS
/******************************************************************************

   Shader having 'SV_SampleIndex' will execute on a per-sample basis,
                           others will execute on per-pixel basis.

   Depth/Stencil tests however are always performed on a per-sample basis.

/******************************************************************************/
#include "stdafx.h"
namespace EE {
#if DEBUG
#define FORCE_TEX 0
#define FORCE_BUF 0
#define FORCE_SHADER 0
#else
#define FORCE_TEX 0
#define FORCE_BUF 0
#define FORCE_SHADER 0
#endif

#if DX11
#define ALLOW_PARTIAL_BUFFERS 0 // using partial buffers (1) actually made things slower, 100fps(1) vs 102fps(0), so use default value (0), TODO: check on newer hardware
#define BUFFER_DYNAMIC 0 // for ALLOW_PARTIAL_BUFFERS=0, using 1 made no difference in performance, so use 0 to reduce API calls. But for ALLOW_PARTIAL_BUFFERS=1 using 1 was slower. Probably it could improve performance if 'ShaderBuffer.data' was not allocated manually but obtained from D3D 'Map', however this would make things complicated, because 'data' always needs to be available for 'ShaderParam.set', we don't always change entire 'data', and 'Map'/'Unmap' most likely always return different 'data' memory address (allocates new memory underneath because of D3D11_MAP_WRITE_DISCARD), this would not work well with instanced rendering, which at the start we don't know how many matrixes (what CB) we need, so we can't map it at the start, because we still need to iterate all instances, count how many, during the process matrixes are already copied to 'data' memory.
#elif GL
#define GL_BUFFER_SUB 0
#define GL_BUFFER_SUB_RESET_PART 1
#define GL_BUFFER_SUB_RESET_FULL 2
#define GL_BUFFER_SUB_RESET_PART_FROM 3
#define GL_BUFFER_SUB_RESET_FULL_FROM 4
// #define GL_BUFFER_SUB_RING            5
// #define GL_BUFFER_SUB_RING_RESET      6
// #define GL_BUFFER_SUB_RING_RESET_FROM 7
#define GL_BUFFER_MAP 8
// #define GL_BUFFER_MAP_RING            9
#define GL_BUFFER_NUM 10

#define GL_DYNAMIC GL_STREAM_DRAW // same performance as GL_DYNAMIC_DRAW

#if WINDOWS
#define GL_UBO_MODE GL_BUFFER_SUB // GeForce 1050 Ti: GL_BUFFER_SUB. Intel UHD 630: GL_BUFFER_SUB_RESET_PART, GL_BUFFER_SUB_RESET_PART_FROM, GL_BUFFER_SUB_RESET_FULL_FROM (all same perf.)
#elif MAC
#define GL_UBO_MODE GL_BUFFER_SUB // FIXME 16 fps for GL_BUFFER_SUB, others show 30-40fps but really look like 8 fps - https://feedbackassistant.apple.com/feedback/7117741
#elif LINUX
#define GL_UBO_MODE GL_BUFFER_SUB // FIXME currently Linux has a bug on Intel GPU's in which only GL_BUFFER_SUB works, while others have flickering https://forums.intel.com/s/question/0D50P00004QebSqSAJ/graphics-driver-bug-linux-updating-ubos?language=en_US
#elif ANDROID
#define GL_UBO_MODE GL_BUFFER_SUB_RESET_FULL // GL_BUFFER_SUB_RESET_PART, GL_BUFFER_SUB_RESET_FULL, GL_BUFFER_SUB_RESET_PART_FROM, GL_BUFFER_SUB_RESET_FULL_FROM, GL_BUFFER_MAP (all same perf. Mali-G76 MP10)
#elif IOS
#define GL_UBO_MODE GL_BUFFER_SUB_RESET_PART_FROM // FIXME test on newer iOS Device. GL_BUFFER_SUB_RESET_PART, GL_BUFFER_SUB_RESET_PART_FROM (all same perf. iPad Mini 2)
#elif SWITCH
#define GL_UBO_MODE GL_BUFFER_SUB_RESET_FULL // GL_BUFFER_SUB, GL_BUFFER_SUB_RESET_FULL, GL_BUFFER_MAP (all same perf.) however GL_BUFFER_SUB_RESET_FULL had better performance at the start when switching to it
#elif WEB
#define GL_UBO_MODE GL_BUFFER_SUB // GL_BUFFER_SUB, GL_BUFFER_SUB_RESET_FULL, GL_BUFFER_SUB_RESET_FULL_FROM (all same perf. Chrome GeForce 1050 Ti Windows), for WEB we need buffer size >= what was defined in the shader because it will complain "GL_INVALID_OPERATION: It is undefined behaviour to use a uniform buffer that is too small." #WebUBO, so would have to use Ceil16(full_size), also WEB doesn't support Map - https://www.khronos.org/registry/webgl/specs/latest/2.0/#5.14
#else
#error
#endif

#if 0 // Test
#pragma message("!! Warning: Use this only for debugging !!")
      Int UBOMode=GL_UBO_MODE;
#undef GL_UBO_MODE
#define GL_UBO_MODE UBOMode
#endif
#define GL_READ_WRITE 0x88BA
#define GL_IMAGE_2D 0x904D
#define GL_COMPUTE_SHADER 0x91B9
#if !WINDOWS && !SWITCH
void (*glBindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
void (*glDispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
#endif
#endif
/******************************************************************************/
// SHADER CACHE
/******************************************************************************/
#include "Shader Hash.h" // this is generated after compiling shaders
#define COMPRESS_GL_SHADER_BINARY COMPRESS_ZSTD // in tests it was faster and had smaller size than LZ4
#define COMPRESS_GL_SHADER_BINARY_LEVEL ((App.flag & APP_SHADER_CACHE_MAX_COMPRESS) ? CompressionLevels(COMPRESS_GL_SHADER_BINARY).y : CompressionDefault(COMPRESS_GL_SHADER_BINARY))
static Bool ShaderCacheLoadHeader(File &f) {
    if (f.decUIntV() == 0) // ver
        if (f.getULong() == SHADER_HASH)
            if (f.getByte() == COMPRESS_GL_SHADER_BINARY)
                return true;
    return false;
}
Bool VerifyPrecompiledShaderCache(C Str &name) {
    Pak pak;
    if (pak.load(name)) {
        File f;
        if (f.read("Data", pak) && ShaderCacheLoadHeader(f))
            return true;
    }
    return false;
}
#if GL
static Bool ShaderCacheLoad(File &f) {
    if (ShaderCacheLoadHeader(f)) {
        Char8 temp[256];
#if GL
        f.getStr(temp);
        if (!Equal(temp, (CChar8 *)glGetString(GL_VERSION)))
            return false;
        f.getStr(temp);
        if (!Equal(temp, (CChar8 *)glGetString(GL_RENDERER)))
            return false;
        f.getStr(temp);
        if (!Equal(temp, (CChar8 *)glGetString(GL_VENDOR)))
            return false;
#endif
        return true;
    }
    return false;
}
static Bool ShaderCacheSave(File &f) {
    f.cmpUIntV(0); // ver
    f.putULong(SHADER_HASH);
    f.putByte(COMPRESS_GL_SHADER_BINARY);
#if GL
    f.putStr((CChar8 *)glGetString(GL_VERSION));
    f.putStr((CChar8 *)glGetString(GL_RENDERER));
    f.putStr((CChar8 *)glGetString(GL_VENDOR));
#endif
    return f.ok();
}

struct PrecompiledShaderCacheClass {
    Pak pak;

    Bool is() C { return pak.totalFiles(); }
    Bool create(C Str &name) {
        if (pak.load(name)) {
            File f;
            if (f.read("Data", pak) && ShaderCacheLoad(f))
                return true;
            LogN("Precompiled ShaderCache is outdated. Please regenerate it using \"Precompile Shaders\" tool, located inside \"Editor Source\\Tools\".");
            pak.del();
        }
        return false;
    }
} static PrecompiledShaderCache;

struct ShaderCacheClass {
    Str path;

    Bool is() C { return path.is(); }
    Str name() C { return path + "Data"; }

    Bool save() C {
        if (is()) {
            Str name = T.name();
            File f;
            if (f.write(name)) {
                if (ShaderCacheSave(f) && f.flush())
                    return true;
                f.del();
                FDelFile(name);
            }
        }
        return false;
    }
    Bool load() C {
        if (is()) {
            File f;
            if (f.readStd(name()))
                return ShaderCacheLoad(f);
        }
        return false;
    }

    void set(C Str &path) {
        if (path.is())
            T.path = NormalizePath(MakeFullPath(path)).tailSlash(true);
        else
            T.path.clear();
    }
    Bool create(C Str &path) {
        if (path.is()) {
            T.path = path;
            if (FCreateDirs(T.path)) {
                if (load())
                    return true;
                FDelInside(T.path); // if old data doesn't match what we want, then assume everything is outdated and delete everything
                if (save())
                    return true;
            }
        }
        T.path.clear();
        return false;
    }
} static ShaderCache;
#endif
DisplayClass &DisplayClass::shaderCache(C Str &path) {
#if GL
    if (!D.created())
        ShaderCache.set(path);
    else                                      // before display created, just store path
    {                                         // after created
        ShaderCache.create(ShaderCache.path); // initialize from stored path
#if SWITCH // on Nintendo Switch we might also have an already precompiled ShaderCache
        if (!(App.flag & APP_IGNORE_PRECOMPILED_SHADER_CACHE))
            PrecompiledShaderCache.create("rom:/ShaderCache.pak"); // specify full path in case user changed 'CurDir'
#endif
    }
#endif
    return T;
}
/******************************************************************************/
#if DX11
static ID3D11ShaderResourceView *VSTex[MAX_SHADER_IMAGES], *HSTex[MAX_SHADER_IMAGES], *DSTex[MAX_SHADER_IMAGES], *PSTex[MAX_SHADER_IMAGES], *CSTex[MAX_SHADER_IMAGES];
static ID3D11UnorderedAccessView *CSUAV[MAX_SHADER_IMAGES];
#elif GL
static UInt Tex[MAX_SHADER_IMAGES], TexSampler[MAX_SHADER_IMAGES], UAV[MAX_SHADER_IMAGES];
#endif
INLINE void DisplayState::texVS(Int index, GPU_API(ID3D11ShaderResourceView *, UInt) tex) {
#if DX11
    if (VSTex[index] != tex || FORCE_TEX)
        D3DC->VSSetShaderResources(index, 1, &(VSTex[index] = tex));
#endif
}
INLINE void DisplayState::texHS(Int index, GPU_API(ID3D11ShaderResourceView *, UInt) tex) {
#if DX11
    if (HSTex[index] != tex || FORCE_TEX)
        D3DC->HSSetShaderResources(index, 1, &(HSTex[index] = tex));
#endif
}
INLINE void DisplayState::texDS(Int index, GPU_API(ID3D11ShaderResourceView *, UInt) tex) {
#if DX11
    if (DSTex[index] != tex || FORCE_TEX)
        D3DC->DSSetShaderResources(index, 1, &(DSTex[index] = tex));
#endif
}
INLINE void DisplayState::texPS(Int index, GPU_API(ID3D11ShaderResourceView *, UInt) tex) {
#if DX11
    if (PSTex[index] != tex || FORCE_TEX)
        D3DC->PSSetShaderResources(index, 1, &(PSTex[index] = tex));
#endif
}
INLINE void DisplayState::texCS(Int index, GPU_API(ID3D11ShaderResourceView *, UInt) tex) {
#if DX11
    if (CSTex[index] != tex || FORCE_TEX)
        D3DC->CSSetShaderResources(index, 1, &(CSTex[index] = tex));
#endif
}
void DisplayState::texClear(GPU_API(ID3D11ShaderResourceView *, UInt) tex) {
#if DX11
    if (tex)
        REPA(PSTex)
    if (PSTex[i] == tex)
        PSTex[i] = null; // for performance reasons this clears only from Pixel Shader, to clear from all shaders use 'clearAll'
#elif GL
    if (tex)
        REPA(Tex)
    if (Tex[i] == tex)
        Tex[i] = ~0;
#endif
}
void DisplayState::uavClear(GPU_API(ID3D11UnorderedAccessView *, UInt) tex) {
#if DX11
    if (tex)
        REPA(CSUAV)
    if (CSUAV[i] == tex)
        CSUAV[i] = null;
#elif GL
    if (tex)
        REPA(UAV)
    if (UAV[i] == tex)
        UAV[i] = ~0;
#endif
}
void DisplayState::rtClear(C ImageRT &image) {
    REPA(Renderer._cur)
    if (Renderer._cur[i] == &image) {
        Renderer._cur[i] = null;
        Renderer._cur_id[i] = GPU_API(null, 0);
    }
    if (Renderer._cur_ds == &image) {
        Renderer._cur_ds = null;
        Renderer._cur_ds_id = GPU_API(null, 0);
    }
}
void DisplayState::texClearAll(GPU_API(ID3D11ShaderResourceView *, UInt) tex) {
#if DX11
    if (tex) {
        REPA(VSTex)
        if (VSTex[i] == tex)
            VSTex[i] = null;
        REPA(HSTex)
        if (HSTex[i] == tex)
            HSTex[i] = null;
        REPA(DSTex)
        if (DSTex[i] == tex)
            DSTex[i] = null;
        REPA(PSTex)
        if (PSTex[i] == tex)
            PSTex[i] = null;
        REPA(CSTex)
        if (CSTex[i] == tex)
            CSTex[i] = null;
    }
#elif GL
    if (tex)
        REPA(Tex)
    if (Tex[i] == tex)
        Tex[i] = ~0;
#endif
}
#if GL
static UInt ActiveTexture = 0;
INLINE static void ActivateTexture(Int index) {
    if (ActiveTexture != index || FORCE_TEX) {
        ActiveTexture = index;
        glActiveTexture(GL_TEXTURE0 + index);
    }
}
void DisplayState::texBind(UInt mode, UInt tex) // this should be called instead of 'glBindTexture'
{
    if (App.mainThread()) // textures are bound per-context, so remember them only on the main thread
    {
        if (Tex[ActiveTexture] == tex)
            return;
        Tex[ActiveTexture] = tex;
    }
    glBindTexture(mode, tex);
}
INLINE static void TexBind(UInt mode, UInt tex) {
    Tex[ActiveTexture] = tex;
    glBindTexture(mode, tex);
}
static UInt SamplerSlot[SSI_NUM]; // this contains the ShaderSampler.sampler for each sampler slot
#if GL_ES
static UInt SamplerSlotNoFilter[SSI_NUM];
#endif
void ShaderSampler::set(Int index) C {
    DEBUG_RANGE_ASSERT(index, SamplerSlot);
    SamplerSlot[index] = sampler; // set GL 'sampler' object to requested SSI slot
#if GL_ES
    SamplerSlotNoFilter[index] = sampler_no_filter;
#endif
}
static void SetTexture(Int index, Int sampler, C Image *image) // this is called only on the Main thread
{
    // texture
#if 0
   glBindMultiTextureEXT(GL_TEXTURE0+index, GL_TEXTURE_2D, txtr); // not supported on ATI (tested on Radeon 5850)
#else
    UInt txtr = (image ? image->_txtr : 0);
    DEBUG_RANGE_ASSERT(index, Tex);
    if (Tex[index] != txtr || FORCE_TEX) {
        ActivateTexture(index);
        if (!txtr) // clear all modes
        {
            Tex[index] = 0;
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindTexture(GL_TEXTURE_3D, 0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        } else
            switch (image->mode()) {
            case IMAGE_2D:
            case IMAGE_RT:
            case IMAGE_DS:
            case IMAGE_SHADOW_MAP:
                TexBind(GL_TEXTURE_2D, txtr);
                break;

            case IMAGE_3D:
                TexBind(GL_TEXTURE_3D, txtr);
                break;
            case IMAGE_CUBE:
                TexBind(GL_TEXTURE_CUBE_MAP, txtr);
                break;
            }
    }
#endif

    // sampler
#if GL_ES
    if (!txtr)
        return; // skip sampler if we don't have a texture, this is also needed for image=null case below
    DEBUG_RANGE_ASSERT(sampler, SamplerSlot);
    UInt gl_sampler = (image->filterable() ? SamplerSlot[sampler] : SamplerSlotNoFilter[sampler]); // here 'image'!=null because above we return if "!txtr"
#else
    DEBUG_RANGE_ASSERT(sampler, SamplerSlot);
    UInt gl_sampler = SamplerSlot[sampler];
#endif
    DEBUG_RANGE_ASSERT(index, TexSampler);
    if (TexSampler[index] != gl_sampler)
        glBindSampler(index, TexSampler[index] = gl_sampler);
}
static void SetRWImage(Int index, C Image *image) // this is called only on the Main thread
{
    UInt txtr = (image ? image->_txtr : 0);
    DEBUG_RANGE_ASSERT(index, UAV);
    if (UAV[index] != txtr || FORCE_TEX) {
        UAV[index] = txtr;
#if 0 // requires GL 4.4+
      glBindImageTextures(index, 1, &txtr);
#else
        glBindImageTexture(index, txtr, 0, true, 0, GL_READ_WRITE, image ? image->hwTypeInfo().format : GL_RGBA8);
#endif
    }
}
#endif
/******************************************************************************/
#if DX11
static ID3D11Buffer *VSBuf[MAX_SHADER_BUFFERS], *HSBuf[MAX_SHADER_BUFFERS], *DSBuf[MAX_SHADER_BUFFERS], *PSBuf[MAX_SHADER_BUFFERS], *CSBuf[MAX_SHADER_BUFFERS];

static INLINE void BufVS(Int index, ID3D11Buffer *buf) {
    if (VSBuf[index] != buf || FORCE_BUF)
        D3DC->VSSetConstantBuffers(index, 1, &(VSBuf[index] = buf));
}
static INLINE void BufHS(Int index, ID3D11Buffer *buf) {
    if (HSBuf[index] != buf || FORCE_BUF)
        D3DC->HSSetConstantBuffers(index, 1, &(HSBuf[index] = buf));
}
static INLINE void BufDS(Int index, ID3D11Buffer *buf) {
    if (DSBuf[index] != buf || FORCE_BUF)
        D3DC->DSSetConstantBuffers(index, 1, &(DSBuf[index] = buf));
}
static INLINE void BufPS(Int index, ID3D11Buffer *buf) {
    if (PSBuf[index] != buf || FORCE_BUF)
        D3DC->PSSetConstantBuffers(index, 1, &(PSBuf[index] = buf));
}
static INLINE void BufCS(Int index, ID3D11Buffer *buf) {
    if (CSBuf[index] != buf || FORCE_BUF)
        D3DC->CSSetConstantBuffers(index, 1, &(CSBuf[index] = buf));
}
#endif
/******************************************************************************/
Cache<ShaderFile> ShaderFiles("Shader");
/******************************************************************************/
// SHADER IMAGE
/******************************************************************************/
ThreadSafeMap<Str8, ShaderImage> ShaderImages(CompareCS);
ThreadSafeMap<Str8, ShaderRWImage> ShaderRWImages(CompareCS);
/******************************************************************************/
void ShaderSampler::del() {
#if DX11
    if (state) {
#if GPU_LOCK // lock not needed for 'Release'
        SyncLocker locker(D._lock);
        if (state)
#endif
        {
            if (D.created())
                state->Release();
            state = null;
        } // clear while in lock
    }
#elif GL
    if (sampler) {
#if GPU_LOCK
        SyncLocker locker(D._lock);
        if (sampler)
#endif
        {
            if (D.created())
                glDeleteSamplers(1, &sampler);
#if GL_ES
            if (sampler == sampler_no_filter)
                sampler_no_filter = 0; // if 'sampler_no_filter' is the same, then clear it too, so we don't delete the same sampler 2 times
#endif
            sampler = 0; // clear while in lock
        }
    }
#if GL_ES
    if (sampler_no_filter) {
#if GPU_LOCK
        SyncLocker locker(D._lock);
        if (sampler_no_filter)
#endif
        {
            if (D.created())
                glDeleteSamplers(1, &sampler_no_filter);
            sampler_no_filter = 0; // clear while in lock
        }
    }
#endif
#endif
}
#if DX11
Bool ShaderSampler::createTry(D3D11_SAMPLER_DESC &desc) {
#if GPU_LOCK // lock not needed for 'D3D'
    SyncLocker locker(D._lock);
#endif
    del();
    if (D3D)
        D3D->CreateSamplerState(&desc, &state);
    return state != null;
}
void ShaderSampler::create(D3D11_SAMPLER_DESC &desc) {
    if (!createTry(desc))
        Exit(S + "Can't create Sampler State\n"
                 "Filter: " +
             desc.Filter + "\n"
                           "Address: " +
             desc.AddressU + ',' + desc.AddressV + ',' + desc.AddressW + "\n"
                                                                         "MipLODBias: " +
             desc.MipLODBias + "\n"
                               "Anisotropy: " +
             desc.MaxAnisotropy + "\n"
                                  "ComparisonFunc: " +
             desc.ComparisonFunc + "\n"
                                   "MinMaxLOD: " +
             desc.MinLOD + ',' + desc.MaxLOD);
}
void ShaderSampler::setVS(Int index) C { D3DC->VSSetSamplers(index, 1, &state); }
void ShaderSampler::setHS(Int index) C { D3DC->HSSetSamplers(index, 1, &state); }
void ShaderSampler::setDS(Int index) C { D3DC->DSSetSamplers(index, 1, &state); }
void ShaderSampler::setPS(Int index) C { D3DC->PSSetSamplers(index, 1, &state); }
void ShaderSampler::setCS(Int index) C { D3DC->CSSetSamplers(index, 1, &state); }
void ShaderSampler::set(Int index) C {
    setVS(index);
    setHS(index);
    setDS(index);
    setPS(index);
    setCS(index);
}
#elif GL
#if GL_ES
UInt GLNoFilter(UInt filter) {
    switch (filter) {
    default:
        return filter; // GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST
    case GL_LINEAR:
        return GL_NEAREST;

    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
    case GL_LINEAR_MIPMAP_LINEAR:
        return GL_NEAREST_MIPMAP_NEAREST; // all must use this, because GL_NEAREST_MIPMAP_LINEAR didn't work
    }
}
#endif
void ShaderSampler::create() {
    if (!sampler) {
        glGenSamplers(1, &sampler);
        if (!sampler)
            Exit("Can't create OpenGL Sampler");
    }
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, filter_min);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, filter_mag);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, address[0]);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, address[1]);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, address[2]);
#if GL_ES
    DYNAMIC_ASSERT(!sampler_no_filter, "Sampler already created"); // assume it's not created yet, otherwise we may have to delete it first
    if (filter_min == GLNoFilter(filter_min) && filter_mag == GLNoFilter(filter_mag))
        sampler_no_filter = sampler;
    else // if parameters are the same, then just set as copy
    {
        if (!sampler_no_filter) {
            glGenSamplers(1, &sampler_no_filter);
            if (!sampler_no_filter)
                Exit("Can't create OpenGL Sampler");
        }
        glSamplerParameteri(sampler_no_filter, GL_TEXTURE_MIN_FILTER, GLNoFilter(filter_min));
        glSamplerParameteri(sampler_no_filter, GL_TEXTURE_MAG_FILTER, GLNoFilter(filter_mag));
        glSamplerParameteri(sampler_no_filter, GL_TEXTURE_WRAP_S, address[0]);
        glSamplerParameteri(sampler_no_filter, GL_TEXTURE_WRAP_T, address[1]);
        glSamplerParameteri(sampler_no_filter, GL_TEXTURE_WRAP_R, address[2]);
    }
#endif
}
#endif
/******************************************************************************/
// SHADER BUFFER
/******************************************************************************/
ThreadSafeMap<Str8, ShaderBuffer> ShaderBuffers(CompareCS);
/******************************************************************************/
void ShaderBuffer::Buffer::del() {
    if (buffer) {
#if GPU_LOCK // lock not needed for 'Release'
        SafeSyncLocker lock(D._lock);
        if (buffer)
#endif
        {
            if (D.created()) {
#if DX11
                buffer->Release();
#elif GL
                glDeleteBuffers(1, &buffer);
#endif
            }

            buffer = GPU_API(null, 0);
        }
    }
    size = 0;
}
void ShaderBuffer::Buffer::create(Int size) {
    // if(T.size!=size) can't check for this, because buffers can be dynamically resized
    {
#if GPU_LOCK // lock not needed for 'D3D'
        SyncLocker locker(D._lock);
#endif

        del();
        if (D.created()) {
            T.size = size;

#if DX11
            D3D11_BUFFER_DESC desc;
            desc.ByteWidth = size;
            desc.Usage = (BUFFER_DYNAMIC ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT);
            desc.CPUAccessFlags = (BUFFER_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0);
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;

            D3D->CreateBuffer(&desc, null, &buffer);
#elif GL
#if WEB
            size = Ceil16(size); // needed for WebGL because it will complain "GL_INVALID_OPERATION: It is undefined behaviour to use a uniform buffer that is too small." #WebUBO
#endif
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_UNIFORM_BUFFER, buffer);
            glBufferData(GL_UNIFORM_BUFFER, size, null, GL_DYNAMIC);
#endif

            if (!buffer)
                Exit("Can't create Constant Buffer"); // Exit only if 'D.created' so we can still continue with APP_ALLOW_NO_GPU/APP_ALLOW_NO_XDISPLAY
        }
    }
}
/******************************************************************************/
// !! Warning: if we have any 'parts', then 'buffer' does not own the resources, but is just a raw copy !!
/******************************************************************************/
void ShaderBuffer::zero() {
    data = null;
    changed = false;
    explicit_bind_slot = -1;
    full_size = 0;
#if GL_MULTIPLE_UBOS
    part = 0;
#endif
}
void ShaderBuffer::del() {
#if DX11 || GL_MULTIPLE_UBOS
    if (parts.elms()) {
        buffer.zero(); // if we have any 'parts', then 'buffer' does not own the resources, so just zero it, and they will be released in the 'parts' container
        parts.del();
    }
#endif
    buffer.del();
    Free(data);
    zero();
}
void ShaderBuffer::create(Int size) // no locks needed because this is called only in shader loading, and there 'ShaderBuffers.lock' is called
{
    del();
#if GL_MULTIPLE_UBOS
    /* Test Results for viewing "Fantasy Demo" World in Editor on Mac
    1 - 8.0 fps
    2 - 12.0 fps
    4 - 14.0 fps
    8 - 17.0 fps
    16 - 20.0 fps
    32 - 20.0 fps
    128 - 20.0 fps
    256 - 22.0 fps
    512 - 25.0 fps
    1024 - 26.7 fps NOT SMOOTH
    2048 - 26.4 fps NOT SMOOTH
    Single UBO with GL_BUFFER_SUB_RESET_FULL - 27.2 fps NOT SMOOTH */
    parts.setNum(512);
    FREPAO(parts).create(size);
    buffer = parts[0];
#else
    buffer.create(size);
#endif
    full_size = size;
    AllocZero(data, size + MIN_SHADER_PARAM_DATA_SIZE); // add extra "Vec4 padd" at the end, because all 'ShaderParam.set' for performance reasons assume that there is at least MIN_SHADER_PARAM_DATA_SIZE size, use "+" instead of "Max" in case we have "Flt p[2]" and we call 'ShaderParam.set(Vec4)' for ShaderParam created from "p[1]" which would overwrite "p[1..4]"
    changed = true;
}
void ShaderBuffer::update() {
#if DX11
    if (BUFFER_DYNAMIC) {
        D3D11_MAPPED_SUBRESOURCE map;
        if (OK(D3DC->Map(buffer.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map))) {
            CopyFast(map.pData, data, buffer.size);
            D3DC->Unmap(buffer.buffer, 0);
        }
    } else
#if ALLOW_PARTIAL_BUFFERS // check for partial updates only if we may operate on partial buffers, because otherwise we always set entire buffers (which are smaller and separated into parts) and we can avoid the overhead of setting up 'D3D11_BOX'
        if (D3DC1) // use partial updates where available to reduce amount of memory
    {
        D3D11_BOX box;
        box.front = box.top = box.left = 0;
        box.right = Ceil16(buffer.size);
        box.back = box.bottom = 1; // must be 16-byte aligned or DX will fail
        D3DC1->UpdateSubresource1(buffer.buffer, 0, &box, data, 0, 0, D3D11_COPY_DISCARD);
    } else
#endif
        D3DC->UpdateSubresource(buffer.buffer, 0, null, data, 0, 0);
#elif GL
    glBindBuffer(GL_UNIFORM_BUFFER, buffer.buffer);
    switch (GL_UBO_MODE) {
    default /*GL_BUFFER_SUB*/:
        glBufferSubData(GL_UNIFORM_BUFFER, 0, buffer.size, data);
        break;
    case GL_BUFFER_SUB_RESET_PART:
        glBufferData(GL_UNIFORM_BUFFER, buffer.size, null, GL_DYNAMIC);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, buffer.size, data);
        break;
    case GL_BUFFER_SUB_RESET_FULL:
        glBufferData(GL_UNIFORM_BUFFER, full_size, null, GL_DYNAMIC);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, buffer.size, data);
        break;
    case GL_BUFFER_SUB_RESET_PART_FROM:
        glBufferData(GL_UNIFORM_BUFFER, buffer.size, data, GL_DYNAMIC);
        break;
    case GL_BUFFER_SUB_RESET_FULL_FROM:
        glBufferData(GL_UNIFORM_BUFFER, full_size, data, GL_DYNAMIC);
        break;
    case GL_BUFFER_MAP:
        if (Ptr dest = glMapBufferRange(GL_UNIFORM_BUFFER, 0, buffer.size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT)) {
            CopyFast(dest, data, buffer.size);
            glUnmapBuffer(GL_UNIFORM_BUFFER);
        }
        break;
    }
#endif
    changed = false;
}
void ShaderBuffer::bind(Int index) {
#if DX11
    BufVS(index, buffer.buffer);
    BufHS(index, buffer.buffer);
    BufDS(index, buffer.buffer);
    BufPS(index, buffer.buffer);
    BufCS(index, buffer.buffer);
#elif GL
    glBindBufferBase(GL_UNIFORM_BUFFER, index, buffer.buffer);
#endif
}
void ShaderBuffer::bindCheck(Int index) {
#if 1
    if (explicit_bind_slot == index)
        return;
#elif DX11
    if (index >= 0) {
        RANGE_ASSERT_ERROR(index, MAX_SHADER_BUFFERS, "Invalid ShaderBuffer bind index");
        ID3D11Buffer *buf = VSBuf[index];
        if (buffer.buffer == buf)
            return;
        REPA(parts)
        if (parts[i].buffer == buf)
            return;
    }
#endif
    Exit(S + "ShaderBuffer was expected to be bound at slot " + index);
}
#if DX11
void ShaderBuffer::setPart(Int part) {
    buffer = parts[part]; // perform a raw copy
    changed = true;
}
void ShaderBuffer::createParts(C Int *elms, Int elms_num) {
    Int elm_size = full_size / elms[0];
    parts.setNum(elms_num);
    parts[0] = buffer; // store a raw copy of the buffer that was already created in the first slot, so we can keep it as backup and use later
    for (Int i = 1; i < parts.elms(); i++)
        parts[i].create(elm_size * elms[i]);
}
#endif
/******************************************************************************/
// SHADER PARAM
/******************************************************************************/
ThreadSafeMap<Str8, ShaderParam> ShaderParams(CompareCS);
/******************************************************************************/
void ShaderParam::OptimizeTranslation(C CMemPtr<Translation> &src, Mems<Translation> &dest) {
    dest = src;
    REPA(dest)
    if (i) {
        Translation &prev = dest[i - 1],
                    &next = dest[i];
        if (prev.cpu_offset + prev.elm_size == next.cpu_offset && prev.gpu_offset + prev.elm_size == next.gpu_offset) {
            prev.elm_size += next.elm_size;
            dest.remove(i, true);
        }
    }
}
/******************************************************************************/
void ShaderParam::zero() {
    _data = null;
    _changed = null;
    _cpu_data_size = _gpu_data_size = _elements = 0;
}
/******************************************************************************/
Int ShaderParam::gpuArrayStride() C {
    if (_elements > 1) {
        if (_full_translation.elms() % _elements)
            Exit("ShaderParam.Translation mod");
        Int elm_translations = _full_translation.elms() / _elements; // calculate number of translations for a single element
        RANGE_ASSERT(elm_translations, _full_translation);
        return _full_translation[elm_translations].gpu_offset - _full_translation[0].gpu_offset;
    }
    return -1;
}
void ShaderParam::initAsElement(ShaderParam &parent, Int index) // this is called after 'parent' was already loaded, so 'gpu_offset' are relative to parameter (not cbuffer)
{
    DEBUG_ASSERT(this != &parent, "Can't init from self");
    RANGE_ASSERT(index, parent._elements);

    _data = parent._data;
    _changed = parent._changed;
    _cpu_data_size = parent._cpu_data_size / parent._elements; // set size of a single element
    _elements = 0;                                             // 0 means not an array

    /*if(parent._full_translation.elms()==1)
      {
        _full_translation=parent._full_translation;
         Translation &t=_full_translation[0];
         if(t.elm_size% parent._elements || parent._gpu_data_size%parent._elements)Exit("ShaderParam.Translation mod");
            t.elm_size/=parent._elements;
         DEBUG_ASSERT(t.cpu_offset==0 && t.gpu_offset==0, "Invalid translation offsets");
        _data+=t.elm_size*index;
        _optimized_translation=_full_translation;
        _gpu_data_size        =parent._gpu_data_size/parent._elements;
      }else*/
    {
        if (parent._full_translation.elms() % parent._elements)
            Exit("ShaderParam.Translation mod");
        Int elm_translations = parent._full_translation.elms() / parent._elements; // calculate number of translations for a single element
        _full_translation.clear();
        FREP(elm_translations)
        _full_translation.add(parent._full_translation[index * elm_translations + i]); // add translations for 'index-th' single element
        Int offset = _full_translation[0].gpu_offset;
        _data += offset;
        REPAO(_full_translation).gpu_offset -= offset; // apply offset
        offset = _full_translation[0].cpu_offset;
        REPAO(_full_translation).cpu_offset -= offset; // apply offset
        optimize();
        _gpu_data_size = 0;
        REPA(_optimized_translation)
        MAX(_gpu_data_size, _optimized_translation[i].gpu_offset + _optimized_translation[i].elm_size);
    }
}
/******************************************************************************/
ASSERT(MIN_SHADER_PARAM_DATA_SIZE >= SIZE(Vec4)); // can write small types without checking for 'canFit', because all 'ShaderBuffer's for 'ShaderParam' data are allocated with MIN_SHADER_PARAM_DATA_SIZE=SIZE(Vec4) padding

void ShaderParamBool::set(Bool b) {
    *(U32 *)_data = b;
    setChanged();
}
void ShaderParamBool::setConditional(Bool b) {
    U32 &dest = *(U32 *)_data;
    if (dest != (U32)b) {
        dest = b;
        setChanged();
    }
}

void ShaderParamInt::set(Int i) {
    *(Int *)_data = i;
    setChanged();
}
void ShaderParamInt::set(C VecI2 &v) {
    *(VecI2 *)_data = v;
    setChanged();
}
void ShaderParamInt::set(C VecI &v) {
    *(VecI *)_data = v;
    setChanged();
}
void ShaderParamInt::set(C VecI4 &v) {
    *(VecI4 *)_data = v;
    setChanged();
}
void ShaderParamInt::set(C RectI &v) {
    *(RectI *)_data = v;
    setChanged();
}
void ShaderParamInt::setConditional(Int i) {
    Int &dest = *(Int *)_data;
    if (dest != i) {
        dest = i;
        setChanged();
    }
}
void ShaderParamInt::setConditional(C VecI2 &v) {
    VecI2 &dest = *(VecI2 *)_data;
    if (dest != v) {
        dest = v;
        setChanged();
    }
}
void ShaderParamInt::setConditional(C VecI &v) {
    VecI &dest = *(VecI *)_data;
    if (dest != v) {
        dest = v;
        setChanged();
    }
}
void ShaderParamInt::setConditional(C VecI4 &v) {
    VecI4 &dest = *(VecI4 *)_data;
    if (dest != v) {
        dest = v;
        setChanged();
    }
}
void ShaderParamInt::setConditional(C RectI &v) {
    RectI &dest = *(RectI *)_data;
    if (dest != v) {
        dest = v;
        setChanged();
    }
}

void ShaderParam::set(Bool b) {
    *(Flt *)_data = b;
    setChanged();
}
void ShaderParam::set(Int i) {
    *(Flt *)_data = i;
    setChanged();
}
void ShaderParam::set(Flt f) {
    *(Flt *)_data = f;
    setChanged();
}
void ShaderParam::set(Dbl d) {
    *(Flt *)_data = d;
    setChanged();
}
void ShaderParam::set(C Vec2 &v) {
    *(Vec2 *)_data = v;
    setChanged();
}
void ShaderParam::set(C VecD2 &v) {
    *(Vec2 *)_data = v;
    setChanged();
}
void ShaderParam::set(C VecI2 &v) {
    *(Vec2 *)_data = v;
    setChanged();
}
void ShaderParam::set(C Vec &v) {
    *(Vec *)_data = v;
    setChanged();
}
void ShaderParam::set(C VecD &v) {
    *(Vec *)_data = v;
    setChanged();
}
void ShaderParam::set(C VecI &v) {
    *(Vec *)_data = v;
    setChanged();
}
void ShaderParam::set(C Vec4 &v) {
    *(Vec4 *)_data = v;
    setChanged();
}
void ShaderParam::set(C VecD4 &v) {
    *(Vec4 *)_data = v;
    setChanged();
}
void ShaderParam::set(C VecI4 &v) {
    *(Vec4 *)_data = v;
    setChanged();
}
void ShaderParam::set(C Rect &rect) {
    *(Rect *)_data = rect;
    setChanged();
}
void ShaderParam::set(C Color &color) {
    *(Vec4 *)_data = SRGBToDisplay(color);
    setChanged();
}

void ShaderParam::set(C Vec *v, Int elms) {
    Vec4 *gpu = (Vec4 *)_data;
    REP(Min(elms, Signed((_gpu_data_size + SIZEU(Flt)) / SIZEU(Vec4))))
    gpu[i].xyz = v[i]; // add SIZE(Flt) because '_gpu_data_size' may be SIZE(Vec) and div by SIZE(Vec4) would return 0 even though one Vec would fit (elements are aligned by 'Vec4' but we're writing only 'Vec')
    setChanged();
}
void ShaderParam::set(C Vec4 *v, Int elms) {
    CopyFast(_data, v, Min(Signed(_gpu_data_size), SIZEI(*v) * elms));
    setChanged();
}

void ShaderParam::set(C Matrix3 &matrix) {
    if (canFit(SIZE(Vec4) + SIZE(Vec4) + SIZE(Vec))) // do not test for 'SIZE(GpuMatrix)' !! because '_gpu_data_size' may be SIZE(GpuMatrix) minus last Flt, because it's not really used (this happens on DX10+)
    {
        Vec4 *gpu = (Vec4 *)_data;
        gpu[0].xyz.set(matrix.x.x, matrix.y.x, matrix.z.x); // SIZE(Vec4)
        gpu[1].xyz.set(matrix.x.y, matrix.y.y, matrix.z.y); // SIZE(Vec4)
        gpu[2].xyz.set(matrix.x.z, matrix.y.z, matrix.z.z); // SIZE(Vec )
        setChanged();
    }
}
void ShaderParam::set(C Matrix &matrix) {
    if (canFit(SIZE(GpuMatrix))) {
        Vec4 *gpu = (Vec4 *)_data;
        gpu[0].set(matrix.x.x, matrix.y.x, matrix.z.x, matrix.pos.x);
        gpu[1].set(matrix.x.y, matrix.y.y, matrix.z.y, matrix.pos.y);
        gpu[2].set(matrix.x.z, matrix.y.z, matrix.z.z, matrix.pos.z);
        setChanged();
    }
}
void ShaderParam::set(C MatrixM &matrix) {
    if (canFit(SIZE(GpuMatrix))) // we're setting as 'GpuMatrix' and not 'MatrixM'
    {
        Vec4 *gpu = (Vec4 *)_data;
        gpu[0].set(matrix.x.x, matrix.y.x, matrix.z.x, matrix.pos.x);
        gpu[1].set(matrix.x.y, matrix.y.y, matrix.z.y, matrix.pos.y);
        gpu[2].set(matrix.x.z, matrix.y.z, matrix.z.z, matrix.pos.z);
        setChanged();
    }
}
void ShaderParam::set(C Matrix4 &matrix) {
    if (canFit(SIZE(matrix))) {
        Vec4 *gpu = (Vec4 *)_data;
        gpu[0].set(matrix.x.x, matrix.y.x, matrix.z.x, matrix.pos.x);
        gpu[1].set(matrix.x.y, matrix.y.y, matrix.z.y, matrix.pos.y);
        gpu[2].set(matrix.x.z, matrix.y.z, matrix.z.z, matrix.pos.z);
        gpu[3].set(matrix.x.w, matrix.y.w, matrix.z.w, matrix.pos.w);
        setChanged();
    }
}
void ShaderParam::set(C Matrix *matrix, Int elms) {
    Vec4 *gpu = (Vec4 *)_data;
    REP(Min(elms, Signed(_gpu_data_size / SIZEU(GpuMatrix)))) {
        gpu[0].set(matrix->x.x, matrix->y.x, matrix->z.x, matrix->pos.x);
        gpu[1].set(matrix->x.y, matrix->y.y, matrix->z.y, matrix->pos.y);
        gpu[2].set(matrix->x.z, matrix->y.z, matrix->z.z, matrix->pos.z);
        gpu += 3;
        matrix++;
    }
    setChanged();
}
void ShaderParam::set(CPtr data, Int size) // !! Warning: 'size' is ignored here for performance reasons !!
{
    REPA(_optimized_translation) {
        C ShaderParam::Translation &trans = _optimized_translation[i];
        CopyFast(T._data + trans.gpu_offset, (Byte *)data + trans.cpu_offset, trans.elm_size);
    }
    setChanged();
}

void ShaderParam::set(C Vec &v, UInt elm) // use unsigned to ignore negative indexes
{
    UInt offset = SIZE(Vec4) * elm;    // elements are aligned by 'Vec4'
    if (canFitVar(offset + SIZE(Vec))) // we're writing only 'Vec'
    {
        Vec *gpu = (Vec *)(_data + offset);
        *gpu = v;
        setChanged();
    }
}
void ShaderParam::set(C Vec &a, C Vec &b) {
    if (canFit(SIZE(Vec4) + SIZE(Vec))) // elements are aligned by 'Vec4' but we're writing only 'Vec4'+'Vec'
    {
        Vec4 *gpu = (Vec4 *)_data; // elements are aligned by 'Vec4'
        gpu[0].xyz = a;
        gpu[1].xyz = b;
        setChanged();
    }
}
void ShaderParam::set(C Vec &a, C Vec &b, UInt elm) // use unsigned to ignore negative indexes
{
    UInt offset = (SIZE(Vec4) * 2) * elm;             // elements are aligned by 'Vec4'*2
    if (canFitVar(offset + (SIZE(Vec4) + SIZE(Vec)))) // we're writing only 'Vec4'+'Vec'
    {
        Vec4 *gpu = (Vec4 *)(_data + offset); // elements are aligned by 'Vec4'
        gpu[0].xyz = a;
        gpu[1].xyz = b;
        setChanged();
    }
}
void ShaderParam::set(C Vec4 &v, UInt elm) // use unsigned to ignore negative indexes
{
    UInt offset = SIZE(Vec4) * elm; // elements are aligned by 'Vec4'
    if (canFitVar(offset + SIZE(Vec4))) {
        Vec4 *gpu = (Vec4 *)(_data + offset);
        *gpu = v;
        setChanged();
    }
}
void ShaderParam::set(C Matrix &matrix, UInt elm) // use unsigned to ignore negative indexes
{
    UInt offset = SIZE(GpuMatrix) * elm;
    if (canFitVar(offset + SIZE(GpuMatrix))) {
        Vec4 *gpu = (Vec4 *)(_data + offset);
        gpu[0].set(matrix.x.x, matrix.y.x, matrix.z.x, matrix.pos.x);
        gpu[1].set(matrix.x.y, matrix.y.y, matrix.z.y, matrix.pos.y);
        gpu[2].set(matrix.x.z, matrix.y.z, matrix.z.z, matrix.pos.z);
        setChanged();
    }
}

void ShaderParam::fromMul(C Matrix &a, C Matrix &b) {
    if (canFit(SIZE(GpuMatrix))) {
        ((GpuMatrix *)_data)->fromMul(a, b);
        setChanged();
    }
}
void ShaderParam::fromMul(C Matrix &a, C MatrixM &b) {
    if (canFit(SIZE(GpuMatrix))) {
        ((GpuMatrix *)_data)->fromMul(a, b);
        setChanged();
    }
}
void ShaderParam::fromMul(C MatrixM &a, C MatrixM &b) {
    if (canFit(SIZE(GpuMatrix))) {
        ((GpuMatrix *)_data)->fromMul(a, b);
        setChanged();
    }
}
void ShaderParam::fromMul(C Matrix &a, C Matrix &b, UInt elm) // use unsigned to ignore negative indexes
{
    UInt offset = SIZE(GpuMatrix) * elm;
    if (canFitVar(offset + SIZE(GpuMatrix))) {
        GpuMatrix *gpu = (GpuMatrix *)(_data + offset);
        gpu->fromMul(a, b);
        setChanged();
    }
}
void ShaderParam::fromMul(C Matrix &a, C MatrixM &b, UInt elm) // use unsigned to ignore negative indexes
{
    UInt offset = SIZE(GpuMatrix) * elm;
    if (canFitVar(offset + SIZE(GpuMatrix))) {
        GpuMatrix *gpu = (GpuMatrix *)(_data + offset);
        gpu->fromMul(a, b);
        setChanged();
    }
}
void ShaderParam::fromMul(C MatrixM &a, C MatrixM &b, UInt elm) // use unsigned to ignore negative indexes
{
    UInt offset = SIZE(GpuMatrix) * elm;
    if (canFitVar(offset + SIZE(GpuMatrix))) {
        GpuMatrix *gpu = (GpuMatrix *)(_data + offset);
        gpu->fromMul(a, b);
        setChanged();
    }
}

void ShaderParam::set(C GpuMatrix &matrix) {
    if (canFit(SIZE(GpuMatrix))) {
        GpuMatrix &gpu = *(GpuMatrix *)_data;
        gpu = matrix;
        setChanged();
    }
}
void ShaderParam::set(C GpuMatrix &matrix, UInt elm) // use unsigned to ignore negative indexes
{
    UInt offset = SIZE(GpuMatrix) * elm;
    if (canFitVar(offset + SIZE(GpuMatrix))) {
        GpuMatrix *gpu = (GpuMatrix *)(_data + offset);
        *gpu = matrix;
        setChanged();
    }
}
void ShaderParam::set(C GpuMatrix *matrix, Int elms) {
    CopyFast(_data, matrix, Min(Signed(_gpu_data_size), SIZEI(GpuMatrix) * elms));
    setChanged();
}

void ShaderParam::setConditional(C Flt &f) {
    U32 &dest = *(U32 *)_data,
        &src = *(U32 *)&f;
    if (dest != src) {
        dest = src;
        setChanged();
    }
}
void ShaderParam::setConditional(C Vec2 &v) {
    Vec2 &dest = *(Vec2 *)_data;
    if (dest != v) {
        dest = v;
        setChanged();
    }
}
void ShaderParam::setConditional(C Vec &v) {
    Vec &dest = *(Vec *)_data;
    if (dest != v) {
        dest = v;
        setChanged();
    }
}
void ShaderParam::setConditional(C Vec4 &v) {
    Vec4 &dest = *(Vec4 *)_data;
    if (dest != v) {
        dest = v;
        setChanged();
    }
}
void ShaderParam::setConditional(C Color &color) {
    Vec4 v = SRGBToDisplay(color);
    Vec4 &dest = *(Vec4 *)_data;
    if (dest != v) {
        dest = v;
        setChanged();
    }
}
void ShaderParam::setConditional(C Rect &r) {
    Rect &dest = *(Rect *)_data;
    if (dest != r) {
        dest = r;
        setChanged();
    }
}

void ShaderParam::setConditional(C Vec &v, UInt elm) // use unsigned to ignore negative indexes
{
    UInt offset = SIZE(Vec4) * elm;    // elements are aligned by 'Vec4'
    if (canFitVar(offset + SIZE(Vec))) // we're writing only 'Vec'
    {
        Vec &dest = *(Vec *)(_data + offset);
        if (dest != v) {
            dest = v;
            setChanged();
        }
    }
}
void ShaderParam::setConditional(C Vec &a, C Vec &b) {
    if (canFit(SIZE(Vec4) + SIZE(Vec))) // elements are aligned by 'Vec4' but we're writing only 'Vec4'+'Vec'
    {
        Vec4 *gpu = (Vec4 *)_data;
        Vec &A = gpu[0].xyz,
            &B = gpu[1].xyz;
        if (A != a || B != b) {
            A = a;
            B = b;
            setChanged();
        }
    }
}
void ShaderParam::setConditional(C Vec &a, C Vec &b, UInt elm) // use unsigned to ignore negative indexes
{
    UInt offset = (SIZE(Vec4) * 2) * elm;             // elements are aligned by 'Vec4'*2
    if (canFitVar(offset + (SIZE(Vec4) + SIZE(Vec)))) // we're writing only 'Vec4'+'Vec'
    {
        Vec4 *gpu = (Vec4 *)(_data + offset);
        Vec &A = gpu[0].xyz,
            &B = gpu[1].xyz;
        if (A != a || B != b) {
            A = a;
            B = b;
            setChanged();
        }
    }
}
void ShaderParam::setInRangeConditional(C Vec &a, C Vec &b, UInt elm) {
    Vec4 *gpu = (Vec4 *)(_data + (SIZE(Vec4) * 2) * elm); // elements are aligned by 'Vec4'*2
    Vec &A = gpu[0].xyz,
        &B = gpu[1].xyz;
    if (A != a || B != b) {
        A = a;
        B = b;
        setChanged();
    }
}

void ShaderParam::setSafe(C Vec4 &v) {
    CopyFast(_data, &v, Min(_gpu_data_size, SIZEU(v)));
    setChanged();
} // !! ALSO CHECK '#setSafeConditional' !!
/******************************************************************************/
ShaderParamChange &ShaderParamChange::set(C Color &color) {
    value = SRGBToDisplay(color);
    return T;
}
/******************************************************************************/
// SHADERS
/******************************************************************************/
#if DX11
#if GPU_LOCK
ShaderVS11::~ShaderVS11() {
    if (vs) {
        SyncLocker locker(D._lock);
        if (vs) {
            if (D.created())
                vs->Release();
            vs = null;
        }
    }
} // clear while in lock
ShaderHS11::~ShaderHS11() {
    if (hs) {
        SyncLocker locker(D._lock);
        if (hs) {
            if (D.created())
                hs->Release();
            hs = null;
        }
    }
} // clear while in lock
ShaderDS11::~ShaderDS11() {
    if (ds) {
        SyncLocker locker(D._lock);
        if (ds) {
            if (D.created())
                ds->Release();
            ds = null;
        }
    }
} // clear while in lock
ShaderPS11::~ShaderPS11() {
    if (ps) {
        SyncLocker locker(D._lock);
        if (ps) {
            if (D.created())
                ps->Release();
            ps = null;
        }
    }
} // clear while in lock
ShaderCS11::~ShaderCS11() {
    if (cs) {
        SyncLocker locker(D._lock);
        if (cs) {
            if (D.created())
                cs->Release();
            cs = null;
        }
    }
} // clear while in lock
#else // lock not needed for 'Release'
ShaderVS11::~ShaderVS11() {
    if (vs) {
        if (D.created())
            vs->Release();
        vs = null;
    }
}
ShaderHS11::~ShaderHS11() {
    if (hs) {
        if (D.created())
            hs->Release();
        hs = null;
    }
}
ShaderDS11::~ShaderDS11() {
    if (ds) {
        if (D.created())
            ds->Release();
        ds = null;
    }
}
ShaderPS11::~ShaderPS11() {
    if (ps) {
        if (D.created())
            ps->Release();
        ps = null;
    }
}
ShaderCS11::~ShaderCS11() {
    if (cs) {
        if (D.created())
            cs->Release();
        cs = null;
    }
}
#endif
#elif GL
#if GPU_LOCK
ShaderSubGL::~ShaderSubGL() {
    if (shader) {
        SyncLocker locker(D._lock);
        if (D.created())
            glDeleteShader(shader);
        shader = 0;
    }
} // clear while in lock
#else
ShaderSubGL::~ShaderSubGL() {
    if (shader) {
        if (D.created())
            glDeleteShader(shader);
        shader = 0;
    }
} // clear while in lock
#endif
#endif

#if DX11
// lock not needed for 'D3D', however we need a lock because this may get called from multiple threads at the same time, but we can use another lock to allow processing during rendering (when D._lock is locked)
static SyncLock ShaderLock; // use custom lock instead of 'D._lock' to allow shader creation while rendering
ID3D11VertexShader *ShaderVS11::create() {
    if (!vs && elms()) {
        SyncLocker locker(GPU_LOCK ? D._lock : ShaderLock);
        if (!vs && elms() && D3D) {
            D3D->CreateVertexShader(data(), elms(), null, &vs);
            clean();
        }
    }
    return vs;
}
ID3D11HullShader *ShaderHS11::create() {
    if (!hs && elms()) {
        SyncLocker locker(GPU_LOCK ? D._lock : ShaderLock);
        if (!hs && elms() && D3D) {
            D3D->CreateHullShader(data(), elms(), null, &hs);
            clean();
        }
    }
    return hs;
}
ID3D11DomainShader *ShaderDS11::create() {
    if (!ds && elms()) {
        SyncLocker locker(GPU_LOCK ? D._lock : ShaderLock);
        if (!ds && elms() && D3D) {
            D3D->CreateDomainShader(data(), elms(), null, &ds);
            clean();
        }
    }
    return ds;
}
ID3D11PixelShader *ShaderPS11::create() {
    if (!ps && elms()) {
        SyncLocker locker(GPU_LOCK ? D._lock : ShaderLock);
        if (!ps && elms() && D3D) {
            D3D->CreatePixelShader(data(), elms(), null, &ps);
            clean();
        }
    }
    return ps;
}
ID3D11ComputeShader *ShaderCS11::create() {
    if (!cs && elms()) {
        SyncLocker locker(GPU_LOCK ? D._lock : ShaderLock);
        if (!cs && elms() && D3D) {
            D3D->CreateComputeShader(data(), elms(), null, &cs);
            clean();
        }
    }
    return cs;
}
#elif GL
CChar8 *GLSLVersion() {
    switch (D.shaderModel()) {
    // https://en.wikipedia.org/wiki/OpenGL_Shading_Language
    default:
        return ""; // avoid null in case some drivers will crash
    case SM_GL_3:
        return "#version 330\n";
    case SM_GL_4:
        return "#version 400\n";
    case SM_GL_4_2:
        return "#version 420\n";
    case SM_GL_4_3:
        return "#version 430\n";
    case SM_GL_ES_3:
        return "#version 300 es\n";
    case SM_GL_ES_3_1:
        return "#version 310 es\n";
    case SM_GL_ES_3_2:
        return "#version 320 es\n";
    }
}
static SyncLock ShaderLock; // use custom lock instead of 'D._lock' to allow shader creation while rendering
UInt ShaderSubGL::create(UInt gl_type, Str *messages) {
    if (!shader && elms()) {
        SyncLocker locker(GPU_LOCK ? D._lock : ShaderLock);
        if (!shader && elms()) {
            CPtr data;
            Int size;
#if COMPRESS_GL_SHADER // compressed
            File src, temp;
            src.readMem(T.data(), T.elms());
            if (!Decompress(src, temp, true))
                return 0;
            temp.pos(0);
            data = temp.mem();
            size = temp.size(); // decompress shader
#else // uncompressed
            data = T.data();
            size = T.elms();
#endif
            UInt shader = glCreateShader(gl_type);
            if (!shader)
                Exit("Can't create GL SHADER"); // create into temp var first and set to this only after fully initialized

            CChar8 *srcs[] =
            {
                GLSLVersion(), // version must be first
#if GL_ES
                "#define noperspective\n"                 // 'noperspective'   not available on GL ES
                "#define gl_ClipDistance ClipDistance\n", // 'gl_ClipDistance' not available on GL ES
#endif
#if LINUX // FIXME - https://forums.intel.com/s/question/0D50P00004QfQyQSAV/graphics-driver-bug-linux-glsl-cant-handle-precisions
                "#define mediump\n#define highp\n#define precision\n", // Linux drivers fail to process constants VS "mediump float v;" PS "precision mediump float; float v;"
#endif
                (CChar8 *)data
            };

#ifdef GL_SHADER_BINARY_FORMAT_SPIR_V_ARB
            if (D.SpirVAvailable()) {
                glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, data, size);
                glSpecializeShader(shader, "main", 0, null, null);
            } else
#endif
            {
                glShaderSource(shader, Elms(srcs), srcs, null);
                glCompileShader(shader); // compile
            }

            GLint ok;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
            if (ok)
                T.shader = shader;
            else // set to this only after all finished, so if another thread runs this method, it will detect 'shader' presence only after it was fully initialized
            {
                if (messages) {
                    Char8 error[64 * 1024];
                    error[0] = 0;
                    glGetShaderInfoLog(shader, Elms(error), null, error);
                    messages->line() += (S + "Shader compilation failed:\n" + error).line() + "Shader code:\n";
                    if (!D.SpirVAvailable())
                        FREPA(srcs) *messages += srcs[i];
                    messages->line();
                }
                glDeleteShader(shader); // shader=0; not needed since it's a temporary
            }

            clean();
        }
    }
    return shader;
}
static Str ShaderSource(UInt shader) {
    Char8 source[64 * 1024];
    if (shader)
        glGetShaderSource(shader, SIZE(source), null, source);
    else
        source[0] = 0;
    return source;
}
Str ShaderSubGL::source() {
    return ShaderSource(shader);
}
#endif
/******************************************************************************/
// SHADER
/******************************************************************************/
#if DX11
// these members must have native alignment because we use them in atomic operations for set on multiple threads
ALIGN_ASSERT(Shader11, vs);
ALIGN_ASSERT(Shader11, hs);
ALIGN_ASSERT(Shader11, ds);
ALIGN_ASSERT(Shader11, ps);
ALIGN_ASSERT(ComputeShader11, cs);
/*Shader11::~Shader11()
{
   /* can't release 'vs,hs,ds,ps' shaders since they're just copies from 'Shader*11'
   if(D.created())
   {
   #if GPU_LOCK // lock not needed for 'Release'
      SyncLocker locker(D._lock);
   #endif
      if(vs)vs->Release();
      if(hs)hs->Release();
      if(ds)ds->Release();
      if(ps)ps->Release();
   }*
}
ComputeShader11::~ComputeShader11()
{
   /* can't release 'cs' shaders since they're just copies from 'Shader*11'
   if(D.created())
   {
   #if GPU_LOCK // lock not needed for 'Release'
      SyncLocker locker(D._lock);
   #endif
      if(cs)cs->Release();
   }*
}*/
Bool Shader11::validate(ShaderFile &shader, Str *messages) // this function should be multi-threaded safe
{
    if (!vs && InRange(data_index[ST_VS], shader._vs))
        AtomicSet(vs, shader._vs[data_index[ST_VS]].create());
    if (!hs && InRange(data_index[ST_HS], shader._hs))
        AtomicSet(hs, shader._hs[data_index[ST_HS]].create());
    if (!ds && InRange(data_index[ST_DS], shader._ds))
        AtomicSet(ds, shader._ds[data_index[ST_DS]].create());
    if (!ps && InRange(data_index[ST_PS], shader._ps))
        AtomicSet(ps, shader._ps[data_index[ST_PS]].create());
    return vs && ps;
}
Bool ComputeShader11::validate(ShaderFile &shader, Str *messages) // this function should be multi-threaded safe
{
    if (!cs && InRange(data_index, shader._cs))
        AtomicSet(cs, shader._cs[data_index].create());
    return cs;
}
#if 1
static ID3D11VertexShader *VShader;
static INLINE void SetVS(ID3D11VertexShader *shader) {
    if (VShader != shader || FORCE_SHADER)
        D3DC->VSSetShader(VShader = shader, null, 0);
}
static ID3D11HullShader *HShader;
static INLINE void SetHS(ID3D11HullShader *shader) {
    if (HShader != shader || FORCE_SHADER)
        D3DC->HSSetShader(HShader = shader, null, 0);
}
static ID3D11DomainShader *DShader;
static INLINE void SetDS(ID3D11DomainShader *shader) {
    if (DShader != shader || FORCE_SHADER)
        D3DC->DSSetShader(DShader = shader, null, 0);
}
static ID3D11PixelShader *PShader;
static INLINE void SetPS(ID3D11PixelShader *shader) {
    if (PShader != shader || FORCE_SHADER)
        D3DC->PSSetShader(PShader = shader, null, 0);
}
static ID3D11ComputeShader *CShader;
static INLINE void SetCS(ID3D11ComputeShader *shader) {
    if (CShader != shader || FORCE_SHADER)
        D3DC->CSSetShader(CShader = shader, null, 0);
}
#else
static INLINE void SetVS(ID3D11VertexShader *shader) { D3DC->VSSetShader(shader, null, 0); }
static INLINE void SetHS(ID3D11HullShader *shader) { D3DC->HSSetShader(shader, null, 0); }
static INLINE void SetDS(ID3D11DomainShader *shader) { D3DC->DSSetShader(shader, null, 0); }
static INLINE void SetPS(ID3D11PixelShader *shader) { D3DC->PSSetShader(shader, null, 0); }
static INLINE void SetCS(ID3D11ComputeShader *shader) { D3DC->CSSetShader(shader, null, 0); }
#endif

#if 1 // set multiple in 1 API call
// !! 'links' are assumed to be sorted by 'index' and all consecutive elements have 'index+1' !!
static INLINE void SetBuffers(C BufferLinkPtr &links, ID3D11Buffer *buf[MAX_SHADER_BUFFERS], void (STDMETHODCALLTYPE ID3D11DeviceContext::*SetConstantBuffers)(UINT StartSlot, UINT NumBuffers, ID3D11Buffer *C *ppConstantBuffers)) // use INLINE to allow directly using virtual func calls
{
    REPA(links) // go from the end
    {
        C BufferLink &link = links[i];
        ID3D11Buffer *buffer = link.buffer->buffer.buffer;
        Int last_index = link.index;
        if (buf[last_index] != buffer || FORCE_BUF) // find first that's different
        {
            buf[last_index] = buffer;
            Int first_index = last_index; // initially this is also the first index
            for (; --i >= 0;)             // check all previous
            {
                C BufferLink &link = links[i];
                ID3D11Buffer *buffer = link.buffer->buffer.buffer;
                Int index = link.index;
                if (buf[index] != buffer || FORCE_BUF) // if another is different too
                    buf[first_index = index] = buffer; // set this buffer and change first index
            }
            (D3DC->*SetConstantBuffers)(first_index, last_index - first_index + 1, buf + first_index); // set all from 'first_index' until 'last_index' (inclusive) in 1 API call
            break;                                                                                     // finished
        }
    }
}
static INLINE void SetImages(C ImageLinkPtr &links, ID3D11ShaderResourceView *tex[MAX_SHADER_IMAGES], void (STDMETHODCALLTYPE ID3D11DeviceContext::*SetShaderResources)(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView *C *ppShaderResourceViews), Bool always = false) // use INLINE to allow directly using virtual func calls
{
    REPA(links) // go from the end
    {
        C ImageLink &link = links[i];
        ID3D11ShaderResourceView *srv = link.image->getSRV();
        Int last_index = link.index;
        if (tex[last_index] != srv || always || FORCE_TEX) // find first that's different
        {
            tex[last_index] = srv;
            Int first_index = last_index; // initially this is also the first index
            for (; --i >= 0;)             // check all previous
            {
                C ImageLink &link = links[i];
                ID3D11ShaderResourceView *srv = link.image->getSRV();
                Int index = link.index;
                if (tex[index] != srv || always || FORCE_TEX) // if another is different too
                    tex[first_index = index] = srv;           // set this image and change first index
            }
            (D3DC->*SetShaderResources)(first_index, last_index - first_index + 1, tex + first_index); // set all from 'first_index' until 'last_index' (inclusive) in 1 API call
            break;                                                                                     // finished
        }
    }
}
static INLINE void SetUAV(C RWImageLink &link) {
    if (C ImageRT *rt = link.image->_image) {
        D.texClear(rt->_srv); // on DX11 when setting UAV, it automatically unbinds it from SRVs, so have to clear it manually
        D.rtClear(*rt);       // on DX11 when setting UAV, it automatically unbinds it from  RTs, so have to clear it manually
    }
}
static INLINE void SetImages(C RWImageLinkPtr &links, ID3D11UnorderedAccessView *tex[MAX_SHADER_IMAGES], void (STDMETHODCALLTYPE ID3D11DeviceContext::*SetUnorderedAccessView)(UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView *C *ppUnorderedAccessViews, const UINT *pUAVInitialCounts)) // use INLINE to allow directly using virtual func calls
{
    REPA(links) // go from the end
    {
        C RWImageLink &link = links[i];
        ID3D11UnorderedAccessView *uav = link.image->getUAV();
        Int last_index = link.index;
        if (tex[last_index] != uav || FORCE_TEX) // find first that's different
        {
            tex[last_index] = uav;
            Int first_index = last_index; // initially this is also the first index
            SetUAV(link);
            for (; --i >= 0;) // check all previous
            {
                C RWImageLink &link = links[i];
                ID3D11UnorderedAccessView *uav = link.image->getUAV();
                Int index = link.index;
                if (tex[index] != uav || FORCE_TEX) // if another is different too
                {
                    tex[first_index = index] = uav; // set this image and change first index
                    SetUAV(link);
                }
            }
            (D3DC->*SetUnorderedAccessView)(first_index, last_index - first_index + 1, tex + first_index, null); // set all from 'first_index' until 'last_index' (inclusive) in 1 API call
            break;                                                                                               // finished
        }
    }
}
static INLINE void ClearImages(C RWImageLinkPtr &links, ID3D11UnorderedAccessView *tex[MAX_SHADER_IMAGES], void (STDMETHODCALLTYPE ID3D11DeviceContext::*SetUnorderedAccessView)(UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView *C *ppUnorderedAccessViews, const UINT *pUAVInitialCounts)) // use INLINE to allow directly using virtual func calls
{
    REPA(links) // go from the end
    {
        C RWImageLink &link = links[i];
        Int last_index = link.index;
        if (tex[last_index] != null || FORCE_TEX) // find first that's different
        {
            tex[last_index] = null;
            Int first_index = last_index; // initially this is also the first index
            for (; --i >= 0;)             // check all previous
            {
                C RWImageLink &link = links[i];
                Int index = link.index;
                if (tex[index] != null || FORCE_TEX) // if another is different too
                    tex[first_index = index] = null; // set this image and change first index
            }
            (D3DC->*SetUnorderedAccessView)(first_index, last_index - first_index + 1, tex + first_index, null); // set all from 'first_index' until 'last_index' (inclusive) in 1 API call
            break;                                                                                               // finished
        }
    }
}

INLINE void Shader11::setVSBuffers() C { SetBuffers(buffers[ST_VS], VSBuf, &ID3D11DeviceContext::VSSetConstantBuffers); }
INLINE void Shader11::setHSBuffers() C { SetBuffers(buffers[ST_HS], HSBuf, &ID3D11DeviceContext::HSSetConstantBuffers); }
INLINE void Shader11::setDSBuffers() C { SetBuffers(buffers[ST_DS], DSBuf, &ID3D11DeviceContext::DSSetConstantBuffers); }
INLINE void Shader11::setPSBuffers() C { SetBuffers(buffers[ST_PS], PSBuf, &ID3D11DeviceContext::PSSetConstantBuffers); }
INLINE void ComputeShader11::setBuffers() C { SetBuffers(buffers, CSBuf, &ID3D11DeviceContext::CSSetConstantBuffers); }

INLINE void Shader11::setVSImages() C { SetImages(images[ST_VS], VSTex, &ID3D11DeviceContext::VSSetShaderResources); }
INLINE void Shader11::setHSImages() C { SetImages(images[ST_HS], HSTex, &ID3D11DeviceContext::HSSetShaderResources); }
INLINE void Shader11::setDSImages() C { SetImages(images[ST_DS], DSTex, &ID3D11DeviceContext::DSSetShaderResources); }
INLINE void Shader11::setPSImages() C { SetImages(images[ST_PS], PSTex, &ID3D11DeviceContext::PSSetShaderResources); }
INLINE void ComputeShader11::setImages() C {
    SetImages(images, CSTex, &ID3D11DeviceContext::CSSetShaderResources, true); // TODO: since 'D.texClear' doesn't clear 'CSTex' for performance reasons, then always set 'CSTex'
    SetImages(rw_images, CSUAV, &ID3D11DeviceContext::CSSetUnorderedAccessViews);
}
INLINE void ComputeShader11::clearImages() C { ClearImages(rw_images, CSUAV, &ID3D11DeviceContext::CSSetUnorderedAccessViews); }
#else // set separately
INLINE void Shader11::setVSBuffers() C {
    REPA(buffers[ST_VS]) {
        C BufferLink &link = buffers[ST_VS][i];
        BufVS(link.index, link.buffer->buffer.buffer);
    }
}
INLINE void Shader11::setHSBuffers() C {
    REPA(buffers[ST_HS]) {
        C BufferLink &link = buffers[ST_HS][i];
        BufHS(link.index, link.buffer->buffer.buffer);
    }
}
INLINE void Shader11::setDSBuffers() C {
    REPA(buffers[ST_DS]) {
        C BufferLink &link = buffers[ST_DS][i];
        BufDS(link.index, link.buffer->buffer.buffer);
    }
}
INLINE void Shader11::setPSBuffers() C {
    REPA(buffers[ST_PS]) {
        C BufferLink &link = buffers[ST_PS][i];
        BufPS(link.index, link.buffer->buffer.buffer);
    }
}
INLINE void ComputeShader11::setBuffers() C {
    REPA(buffers) {
        C BufferLink &link = buffers[i];
        BufCS(link.index, link.buffer->buffer.buffer);
    }
}

INLINE void Shader11::setVSImages() C {
    REPA(images[ST_VS]) {
        C ImageLink &link = images[ST_VS][i];
        D.texVS(link.index, link.image->getSRV());
    }
}
INLINE void Shader11::setHSImages() C {
    REPA(images[ST_HS]) {
        C ImageLink &link = images[ST_HS][i];
        D.texHS(link.index, link.image->getSRV());
    }
}
INLINE void Shader11::setDSImages() C {
    REPA(images[ST_DS]) {
        C ImageLink &link = images[ST_DS][i];
        D.texDS(link.index, link.image->getSRV());
    }
}
INLINE void Shader11::setPSImages() C {
    REPA(images[ST_PS]) {
        C ImageLink &link = images[ST_PS][i];
        D.texPS(link.index, link.image->getSRV());
    }
}
#endif

INLINE void Shader11::updateBuffers() C {
    REPA(all_buffers) {
        ShaderBuffer &b = *all_buffers[i];
        if (b.changed)
            b.update();
    }
}
INLINE void ComputeShader11::updateBuffers() C {
    REPA(all_buffers) {
        ShaderBuffer &b = *all_buffers[i];
        if (b.changed)
            b.update();
    }
}

void Shader11::commit() C { updateBuffers(); }

void Shader11::commitTex() C {
    if (hs) {
        setHSImages();
        setDSImages();
    }
    setVSImages();
    setPSImages();
}

void Shader11::start() C // same as 'begin' but without committing buffers and textures
{
    if (hs /* && D.tesselationAllow()*/) // currently disabled to avoid extra overhead as tesselation isn't generally used, TODO:
    {
        D.primType(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
        SetHS(hs);
        SetDS(ds);
        setHSBuffers();
        setDSBuffers();
    } else {
        D.primType(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        SetHS(null);
        SetDS(null);
    }
    SetVS(vs);
    SetPS(ps);
    setVSBuffers();
    setPSBuffers();
}
void Shader11::startTex() C // same as 'begin' but without committing buffers
{
    if (hs /* && D.tesselationAllow()*/) // currently disabled to avoid extra overhead as tesselation isn't generally used, TODO:
    {
        D.primType(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
        SetHS(hs);
        SetDS(ds);
        setHSImages();
        setDSImages();
        setHSBuffers();
        setDSBuffers();
    } else {
        D.primType(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        SetHS(null);
        SetDS(null);
    }
    SetVS(vs);
    SetPS(ps);
    setVSImages();
    setPSImages();
    setVSBuffers();
    setPSBuffers();
}
void Shader11::begin() C {
    if (hs /* && D.tesselationAllow()*/) // currently disabled to avoid extra overhead as tesselation isn't generally used, TODO:
    {
        D.primType(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
        SetHS(hs);
        SetDS(ds);
        setHSImages();
        setDSImages();
        setHSBuffers();
        setDSBuffers();
    } else {
        D.primType(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        SetHS(null);
        SetDS(null);
    }
    SetVS(vs);
    SetPS(ps);
    setVSImages();
    setPSImages();
    setVSBuffers();
    setPSBuffers();
    updateBuffers();
}
void ComputeShader11::begin() C {
#if DX11 // on DX11 trying to bind image SRV's while they're bound as RT's will fail
    Renderer.set(null, null, false); // clear RT's
#endif
    SetCS(cs);
    setImages();
    setBuffers();
    updateBuffers();
}
void ComputeShader11::end() C {
    clearImages(); // have to unlink UAVs after computing, because DX will fail to bind their SRVs while UAVs are still bound
}
void ComputeShader11::compute(C VecI2 &groups) C {
    begin();
    D3DC->Dispatch(groups.x, groups.y, 1);
    end();
}
void ComputeShader11::compute(C VecI &groups) C {
    begin();
    D3DC->Dispatch(groups.x, groups.y, groups.z);
    end();
}
#elif GL
/******************************************************************************/
ShaderGL::~ShaderGL() {
    if (prog) {
#if GPU_LOCK
        SyncLocker locker(D._lock);
#endif
        if (D.created())
            glDeleteProgram(prog);
        prog = 0; // clear while in lock
    }
    // no need to release 'vs,ps' shaders since they're just copies from 'ShaderSubGL'
}
ComputeShaderGL::~ComputeShaderGL() {
    if (prog) {
#if GPU_LOCK
        SyncLocker locker(D._lock);
#endif
        if (D.created())
            glDeleteProgram(prog);
        prog = 0; // clear while in lock
    }
    // no need to release 'cs' shaders since they're just copies from 'ShaderSubGL'
}
Str ShaderGL::source() C {
    return S + "Vertex Shader:\n" + ShaderSource(vs) + "\nPixel Shader:\n" + ShaderSource(ps);
}
Str ComputeShaderGL::source() C {
    return ShaderSource(cs);
}
/******************************************************************************/
static const Int ProgramBinaryHeader = 4;
ASSERT(SIZE(GLenum) == ProgramBinaryHeader); // make room for 'format'
static void SaveProgramBinary(UInt prog, C Str &name) {
    GLint size = 0;
    glGetProgramiv(prog, GL_PROGRAM_BINARY_LENGTH, &size);
    if (size) {
        Memt<Byte> shader_data;
        shader_data.setNumDiscard(size + ProgramBinaryHeader);
        GLenum format = 0;
        GLsizei size = 0;
        glGetProgramBinary(prog, shader_data.elms() - ProgramBinaryHeader, &size, &format, shader_data.data() + ProgramBinaryHeader);
        if (size == shader_data.elms() - ProgramBinaryHeader && format) {
            *(GLenum *)shader_data.data() = format;
            File f(shader_data.data(), shader_data.elms());
            if (COMPRESS_GL_SHADER_BINARY) {
                File temp;
                temp.writeMem();
                temp.cmpUIntV(f.size());
                CompressRaw(f, temp, COMPRESS_GL_SHADER_BINARY, COMPRESS_GL_SHADER_BINARY_LEVEL);
                temp.pos(0);
                Swap(f, temp);
            }
            SafeOverwrite(f, name);
        }
    }
}
static UInt CreateProgramFromBinary(CPtr data, Int size) {
    if (size > ProgramBinaryHeader) {
        UInt prog = glCreateProgram();
        if (!prog)
            Exit("Can't create GL Shader Program");
        glProgramBinary(prog, *(GLenum *)data, (Byte *)data + ProgramBinaryHeader, size - ProgramBinaryHeader);
        GLint ok = 0;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (ok)
            return prog;
        glDeleteProgram(prog);
    }
    return 0;
}
static UInt CreateProgramFromBinary(File &f) {
    if (COMPRESS_GL_SHADER_BINARY) {
        Int size = f.decUIntV();
        File temp;
        if (!DecompressRaw(f, temp, COMPRESS_GL_SHADER_BINARY, f.left(), size, true))
            return 0;
        temp.pos(0);
        return CreateProgramFromBinary(temp.memFast(), temp.size());
    } else {
        Memt<Byte> shader_data;
        shader_data.setNumDiscard(f.left());
        if (!f.getFast(shader_data.data(), shader_data.elms()))
            return 0; // load everything into temp memory, to avoid using File buffer
        return CreateProgramFromBinary(shader_data.data(), shader_data.elms());
    }
}
/******************************************************************************/
static const Char8 ShaderSeparator = '@';
static const Char8 ComputeShaderSeparator = '#'; // must be different so it doesn't conflict with regular shaders

UInt ShaderGL::compile(MemPtr<ShaderSubGL> vs_array, MemPtr<ShaderSubGL> ps_array, ShaderFile *shader, Str *messages) // this function doesn't need to be multi-threaded safe, it's called by 'validate' where it's already surrounded by a lock, GL thread-safety should be handled outside of this function
{
    if (messages)
        messages->clear();
    UInt prog = 0; // have to operate on temp variable, so we can return it to 'validate' which still has to do some things before setting it into 'this'

    // load from cache
    if (PrecompiledShaderCache.is()) {
        File f;
        if (f.read(ShaderFiles.name(shader) + ShaderSeparator + T.name, PrecompiledShaderCache.pak)) {
            if (prog = CreateProgramFromBinary(f))
                return prog;
#if SWITCH // if there's Precompiled ShaderCache, but failed to create, then report this, it could happen due to updated Nintendo SDK
            static Bool msg = true;
            if (msg) {
                msg = false;
                LogN("Precompiled ShaderCache is outdated. Please regenerate it using \"Precompile Shaders\" tool, located inside \"Editor Source\\Tools\".");
            }
#endif
        }
    }
    Str shader_cache_name; // this name will be used for loading from cache, and if failed to load, then save to cache
    if (ShaderCache.is()) {
        shader_cache_name = ShaderCache.path + ShaderFiles.name(shader) + ShaderSeparator + T.name;
        File f;
        if (f.readStd(shader_cache_name)) {
            if (prog = CreateProgramFromBinary(f))
                return prog;
            f.del();
            FDelFile(shader_cache_name); // if failed to create, then assume file data is outdated and delete it
        }
    }

    // prepare shaders
    if (!vs && InRange(vs_index, vs_array)) {
        if (LogInit)
            LogN(S + "Compiling vertex shader in technique \"" + name + "\" of shader \"" + ShaderFiles.name(shader) + "\"");
        vs = vs_array[vs_index].create(GL_VERTEX_SHADER, messages);
    } // no need for 'AtomicSet' because we don't need to be multi-thread safe here
    if (!ps && InRange(ps_index, ps_array)) {
        if (LogInit)
            LogN(S + "Compiling pixel shader in technique \"" + name + "\" of shader \"" + ShaderFiles.name(shader) + "\"");
        ps = ps_array[ps_index].create(GL_FRAGMENT_SHADER, messages);
    } // no need for 'AtomicSet' because we don't need to be multi-thread safe here

    // prepare program
    if (vs && ps) {
        if (LogInit)
            Log(S + "Linking vertex+pixel shader in technique \"" + name + "\" of shader \"" + ShaderFiles.name(shader) + "\": ");
        prog = glCreateProgram();
        if (!prog)
            Exit("Can't create GL Shader Program");
        FREP(GL_VTX_NUM) // this is for GL_VTX_SEMANTIC, keep just in case we don't want to store "layout(location=I)" inside shaders
        {
            Char8 name[16], temp[256];
            Set(name, "ATTR");
            Append(name, TextInt(i, temp));
            glBindAttribLocation(prog, VtxSemanticToIndex(i), name);
        }
        glAttachShader(prog, vs);
        glAttachShader(prog, ps);
        glLinkProgram(prog);
        GLint ok;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint max_length;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &max_length);
            Mems<char> error;
            error.setNumZero(max_length + 1);
            glGetProgramInfoLog(prog, max_length, null, error.data());
            if (messages)
                messages->line() += (S + "Error linking vertex+pixel shader in technique \"" + name + "\" of shader \"" + ShaderFiles.name(shader) + "\"\n" + error.data()).line() + source().line();
            glDeleteProgram(prog);
            prog = 0;
        }
        if (LogInit)
            LogN("Success");

        // save to cache
        if (prog && shader_cache_name.is())
            SaveProgramBinary(prog, shader_cache_name);
    }
    return prog;
}
UInt ComputeShaderGL::compile(MemPtr<ShaderSubGL> cs_array, ShaderFile *shader, Str *messages) // this function doesn't need to be multi-threaded safe, it's called by 'validate' where it's already surrounded by a lock, GL thread-safety should be handled outside of this function
{
    if (messages)
        messages->clear();
    UInt prog = 0; // have to operate on temp variable, so we can return it to 'validate' which still has to do some things before setting it into 'this'

    // load from cache
    if (PrecompiledShaderCache.is()) {
        File f;
        if (f.read(ShaderFiles.name(shader) + ComputeShaderSeparator + T.name, PrecompiledShaderCache.pak))
            if (prog = CreateProgramFromBinary(f))
                return prog;
    }
    Str shader_cache_name; // this name will be used for loading from cache, and if failed to load, then save to cache
    if (ShaderCache.is()) {
        shader_cache_name = ShaderCache.path + ShaderFiles.name(shader) + ComputeShaderSeparator + T.name;
        File f;
        if (f.readStd(shader_cache_name)) {
            if (prog = CreateProgramFromBinary(f))
                return prog;
            f.del();
            FDelFile(shader_cache_name); // if failed to create, then assume file data is outdated and delete it
        }
    }

    // prepare shaders
    if (!cs && InRange(cs_index, cs_array)) {
        if (LogInit)
            LogN(S + "Compiling compute shader in technique \"" + name + "\" of shader \"" + ShaderFiles.name(shader) + "\"");
        cs = cs_array[cs_index].create(GL_COMPUTE_SHADER, messages);
    } // no need for 'AtomicSet' because we don't need to be multi-thread safe here

    // prepare program
    if (cs) {
        if (LogInit)
            Log(S + "Linking compute shader in technique \"" + name + "\" of shader \"" + ShaderFiles.name(shader) + "\": ");
        prog = glCreateProgram();
        if (!prog)
            Exit("Can't create GL Shader Program");
        glAttachShader(prog, cs);
        glLinkProgram(prog);
        GLint ok;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint max_length;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &max_length);
            Mems<char> error;
            error.setNumZero(max_length + 1);
            glGetProgramInfoLog(prog, max_length, null, error.data());
            if (messages)
                messages->line() += (S + "Error linking compute shader in technique \"" + name + "\" of shader \"" + ShaderFiles.name(shader) + "\"\n" + error.data()).line() + source().line();
            glDeleteProgram(prog);
            prog = 0;
        }
        if (LogInit)
            LogN("Success");

        // save to cache
        if (prog && shader_cache_name.is())
            SaveProgramBinary(prog, shader_cache_name);
    }
    return prog;
}
/******************************************************************************/
static void GetProgramUniforms(UInt prog, MemtN<SamplerImageLink, 256> &images, MemtN<RWImageLink, 256> &rw_images, MemtN<BufferLink, 256> &buffers, Mems<ShaderBuffer *> &all_buffers) {
    Int params = 0;
    glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &params);
    FREP(params) {
        Char8 name[1024];
        name[0] = 0;
        Int elms = 0;
        GLenum type = 0;
        glGetActiveUniform(prog, i, Elms(name), null, &elms, &type, name);
        switch (type) {
            /*case GL_SAMPLER:
              {
                 Int location=glGetUniformLocation(prog, name);
                 LogN(S+"SAMPLER:"+location+" "+name);
              }break;*/

        case GL_SAMPLER_2D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_2D_SHADOW:
#if defined GL_SAMPLER_2D_SHADOW_EXT && GL_SAMPLER_2D_SHADOW_EXT != GL_SAMPLER_2D_SHADOW
        case GL_SAMPLER_2D_SHADOW_EXT:
#endif
        {
            if (name[0] != 'S' || name[2] != '_')
                Exit("Invalid Sampler name"); // all GL buffers assume to start with 'S' this is adjusted in 'ShaderCompiler' #SamplerName
            Int tex_unit = images.elms();
            if (!InRange(tex_unit, Tex))
                Exit(S + "Texture index: " + tex_unit + ", is too big");
            Int location = glGetUniformLocation(prog, name);
            if (location < 0)
                Exit(S + "Invalid Uniform Location (" + location + ") of GLSL Parameter \"" + name + "\"");
            Int sampler_index = TextInt(name + 1);
            images.New().set(tex_unit, sampler_index, *GetShaderImage(name + 3));
            // LogN(S+"IMAGE: "+name+", location:"+location+", tex_unit:"+tex_unit);

            glUseProgram(prog);
            glUniform1i(location, tex_unit); // set 'location' sampler to use 'tex_unit' texture unit
        } break;

        case GL_IMAGE_2D: {
            Int uav_unit = rw_images.elms();
            if (!InRange(uav_unit, UAV))
                Exit(S + "Texture index: " + uav_unit + ", is too big");
            Int location = glGetUniformLocation(prog, name);
            if (location < 0)
                Exit(S + "Invalid Uniform Location (" + location + ") of GLSL Parameter \"" + name + "\"");
            rw_images.New().set(uav_unit, *GetShaderRWImage(name));

            glUseProgram(prog);
            glUniform1i(location, uav_unit); // set 'location' image to use 'uav_unit' UAV unit
        } break;
        }
    }

    Int variable_slot_index = SBI_NUM;
    params = 0;
    glGetProgramiv(prog, GL_ACTIVE_UNIFORM_BLOCKS, &params);
    all_buffers.setNum(params);
    FREP(params) {
        Char8 _name[256];
        _name[0] = '\0';
        Int length = 0;
        glGetActiveUniformBlockName(prog, i, Elms(_name), &length, _name);
        CChar8 *name;
        if (D.SpirVAvailable()) {
            name = TextPos(_name, '.');
            if (!name)
                Exit("Invalid buffer name"); // SPIR-V generates buffer names as "type_NAME.NAME", so just get after '.'
            name++;
        } else {
            if (_name[0] != '_')
                Exit("Invalid buffer name"); // all GL buffers assume to start with '_' this is adjusted in 'ShaderCompiler' #UBOName
            name = _name + 1;                // skip '_'
        }
        ShaderBuffer *buffer = all_buffers[i] = GetShaderBuffer(name);
#if GL_MULTIPLE_UBOS
        glUniformBlockBinding(prog, i, i);
#else
        if (buffer->explicit_bind_slot >= 0)
            glUniformBlockBinding(prog, i, buffer->explicit_bind_slot);
        else                                                               // explicit bind slot buffers are always bound to the same slot, and linked with the GL program
        {                                                                  // non-explicit buffers will be assigned to slots starting from SBI_NUM (to avoid conflict with explicits)
            glUniformBlockBinding(prog, i, variable_slot_index);           // link with 'variable_slot_index' slot
            buffers.New().set(variable_slot_index, buffer->buffer.buffer); // request linking buffer with 'variable_slot_index' slot
            variable_slot_index++;
        }
#endif
#if DEBUG // verify sizes and offsets
        GLint size = 0;
        glGetActiveUniformBlockiv(prog, i, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
        DYNAMIC_ASSERT(Ceil16(size) == Ceil16(buffer->full_size), S + "UBO \"" + name + "\" has different size: " + size + ", than expected: " + buffer->full_size /*+"\n"+source()*/);
        GLint uniforms = 0;
        glGetActiveUniformBlockiv(prog, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uniforms);
        MemtN<GLint, 256> uniform;
        uniform.setNum(uniforms);
        glGetActiveUniformBlockiv(prog, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uniform.data());
        REPA(uniform) {
            Char8 name[1024];
            name[0] = 0;
            Int elms = 0;
            GLenum type = 0;
            glGetActiveUniform(prog, uniform[i], Elms(name), null, &elms, &type, name);
            GLuint uni = uniform[i];
            GLint offset = -1;
            glGetActiveUniformsiv(prog, 1, &uni, GL_UNIFORM_OFFSET, &offset);
            GLint array_stride = -1;
            glGetActiveUniformsiv(prog, 1, &uni, GL_UNIFORM_ARRAY_STRIDE, &array_stride);
            GLint matrix_stride = -1;
            glGetActiveUniformsiv(prog, 1, &uni, GL_UNIFORM_MATRIX_STRIDE, &matrix_stride);
            if (elms > 1)
                if (ShaderParam *param = FindShaderParam((Str8)SkipEnd(name, "[0]"))) // GL may add [0] to name when using arrays
                {
                    DYNAMIC_ASSERT(param->_elements == elms, "Invalid ShaderParam array elements");
                    DYNAMIC_ASSERT(param->gpuArrayStride() == array_stride, "Invalid ShaderParam array stride");
                }
            if (ShaderParam *param = FindShaderParam(name)) {
                DYNAMIC_ASSERT(param->_changed == &buffer->changed, "ShaderParam does not belong to ShaderBuffer");
                DYNAMIC_ASSERT(param->_full_translation.elms(), "ShaderParam has no translation");
                Int gpu_offset = param->_full_translation[0].gpu_offset + (param->_data - buffer->data);
                DYNAMIC_ASSERT(offset == gpu_offset, "Invalid ShaderParam gpu_offset");
                UInt size;
                switch (type) {
                case GL_INT:
                    size = SIZE(Int);
                    break;
                case GL_UNSIGNED_INT:
                    size = SIZE(UInt);
                    break;
                case GL_INT_VEC2:
                    size = SIZE(VecI2);
                    break;
                case GL_INT_VEC3:
                    size = SIZE(VecI);
                    break;
                case GL_INT_VEC4:
                    size = SIZE(VecI4);
                    break;
                case GL_UNSIGNED_INT_VEC2:
                    size = SIZE(VecI2);
                    break;
                case GL_UNSIGNED_INT_VEC3:
                    size = SIZE(VecI);
                    break;
                case GL_UNSIGNED_INT_VEC4:
                    size = SIZE(VecI4);
                    break;
                case GL_FLOAT:
                    size = SIZE(Flt);
                    break;
                case GL_FLOAT_VEC2:
                    size = SIZE(Vec2);
                    break;
                case GL_FLOAT_VEC3:
                    size = SIZE(Vec);
                    break;
                case GL_FLOAT_VEC4:
                    size = SIZE(Vec4);
                    break;
                case GL_FLOAT_MAT3:
                    size = SIZE(Matrix3);
                    DYNAMIC_ASSERT(matrix_stride == SIZE(Vec4), S + "Invalid ShaderParam \"" + name + "\" matrix stride: " + matrix_stride);
                    break;
                case GL_FLOAT_MAT4:
                    size = SIZE(Matrix4);
                    DYNAMIC_ASSERT(matrix_stride == SIZE(Vec4), S + "Invalid ShaderParam \"" + name + "\" matrix stride: " + matrix_stride);
                    break;
                case GL_FLOAT_MAT4x3:
                    size = SIZE(Matrix);
                    DYNAMIC_ASSERT(matrix_stride == SIZE(Vec4), S + "Invalid ShaderParam \"" + name + "\" matrix stride: " + matrix_stride);
                    break;
                default:
                    Exit("Invalid ShaderParam type");
                    break;
                }
                DYNAMIC_ASSERT(size == param->_cpu_data_size, "Invalid ShaderParam size");
            } // else Exit(S+"ShaderParam \""+name+"\" not found"); disable because currently 'FindShaderParam' does not support finding members, such as "Viewport.size_fov_tan" etc.
        }
#endif
    }
}
Bool ShaderGL::validate(ShaderFile &shader, Str *messages) // this function should be multi-threaded safe
{
    if (prog || !D.created())
        return true; // needed for APP_ALLOW_NO_GPU/APP_ALLOW_NO_XDISPLAY, skip shader compilation if we don't need it (this is because compiling shaders on Linux with no GPU can exit the app with a message like "Xlib:  extension "XFree86-VidModeExtension" missing on display ":99".")
    SyncLocker locker(GPU_LOCK ? D._lock : ShaderLock);
    if (!prog)
        if (UInt prog = compile(shader._vs, shader._ps, &shader, messages)) // create into temp var first and set to this only after fully initialized
        {
            MemtN<SamplerImageLink, 256> images;
            MemtN<RWImageLink, 256> rw_images;
            MemtN<BufferLink, 256> buffers;
            GetProgramUniforms(prog, images, rw_images, buffers, all_buffers);
            T.images = images;
            T.buffers = buffers;

            // !! at the end !!
            T.prog = prog; // set to this only after all finished, so if another thread runs this method, it will detect 'prog' presence only after it was fully initialized
        }
    return prog != 0;
}
Bool ComputeShaderGL::validate(ShaderFile &shader, Str *messages) // this function should be multi-threaded safe
{
    if (prog || !D.created())
        return true; // needed for APP_ALLOW_NO_GPU/APP_ALLOW_NO_XDISPLAY, skip shader compilation if we don't need it (this is because compiling shaders on Linux with no GPU can exit the app with a message like "Xlib:  extension "XFree86-VidModeExtension" missing on display ":99".")
    SyncLocker locker(GPU_LOCK ? D._lock : ShaderLock);
    if (!prog)
        if (UInt prog = compile(shader._cs, &shader, messages)) // create into temp var first and set to this only after fully initialized
        {
            MemtN<SamplerImageLink, 256> images;
            MemtN<RWImageLink, 256> rw_images;
            MemtN<BufferLink, 256> buffers;
            GetProgramUniforms(prog, images, rw_images, buffers, all_buffers);
            T.images = images;
            T.rw_images = rw_images;
            T.buffers = buffers;

            // !! at the end !!
            T.prog = prog; // set to this only after all finished, so if another thread runs this method, it will detect 'prog' presence only after it was fully initialized
        }
    return prog != 0;
}
/******************************************************************************/
INLINE void ShaderGL::bindImages() C {
    REPA(images) {
        C SamplerImageLink &t = images[i];
        SetTexture(t.index, t.sampler, t.image->get());
    }
}
INLINE void ComputeShaderGL::bindImages() C {
    REPA(images) {
        C SamplerImageLink &t = images[i];
        SetTexture(t.index, t.sampler, t.image->get());
    }
    REPA(rw_images) {
        C RWImageLink &t = rw_images[i];
        SetRWImage(t.index, t.image->get());
    }
}
void ShaderGL::commitTex() C {
    bindImages();
}

#if GL_MULTIPLE_UBOS
void ShaderGL::commit() C {
    REPA(all_buffers) {
        ShaderBuffer &b = *all_buffers[i];
        if (b.changed) {
            b.buffer = b.parts[b.part];
            b.part = (b.part + 1) % b.parts.elms();
            b.update();
        }
        glBindBufferBase(GL_UNIFORM_BUFFER, i, b.buffer.buffer);
    }
}
void ShaderGL::start() C // same as 'begin' but without committing buffers and textures
{
    glUseProgram(prog);
}
void ShaderGL::startTex() C // same as 'begin' but without committing buffers
{
    glUseProgram(prog);
    commitTex();
}
void ShaderGL::begin() C {
    glUseProgram(prog);
    commitTex();
    commit();
}
#else
INLINE void ShaderGL::bindBuffers() C {
    REPA(buffers) {
        C BufferLink &b = buffers[i];
        glBindBufferBase(GL_UNIFORM_BUFFER, b.index, b.buffer);
    }
}
INLINE void ComputeShaderGL::bindBuffers() C {
    REPA(buffers) {
        C BufferLink &b = buffers[i];
        glBindBufferBase(GL_UNIFORM_BUFFER, b.index, b.buffer);
    }
}

INLINE void ShaderGL::updateBuffers() C {
    REPA(all_buffers) {
        ShaderBuffer &b = *all_buffers[i];
        if (b.changed)
            b.update();
    }
}
INLINE void ComputeShaderGL::updateBuffers() C {
    REPA(all_buffers) {
        ShaderBuffer &b = *all_buffers[i];
        if (b.changed)
            b.update();
    }
}

void ShaderGL::commit() C {
    updateBuffers();
}
void ShaderGL::start() C // same as 'begin' but without committing buffers and textures
{
    glUseProgram(prog);
    bindBuffers();
}
void ShaderGL::startTex() C // same as 'begin' but without committing buffers
{
    glUseProgram(prog);
    bindImages(); // 'commitTex'
    bindBuffers();
}
void ShaderGL::begin() C {
    glUseProgram(prog);
    updateBuffers(); // 'commit'
    bindImages();    // 'commitTex'
    bindBuffers();
}
void ComputeShaderGL::begin() C {
    glUseProgram(prog);
    updateBuffers(); // 'commit'
    bindImages();    // 'commitTex'
    bindBuffers();
}
void ComputeShaderGL::end() C {
    REPA(rw_images) {
        C RWImageLink &t = rw_images[i];
        SetRWImage(t.index, null);
    } // clear same like on DX11 because drivers are buggy
}
#endif
void ComputeShaderGL::compute(C VecI2 &groups) C {
    begin();
    glDispatchCompute(groups.x, groups.y, 1);
    end();
}
void ComputeShaderGL::compute(C VecI &groups) C {
    begin();
    glDispatchCompute(groups.x, groups.y, groups.z);
    end();
}
#endif
/******************************************************************************/
// MANAGE
/******************************************************************************/
ShaderFile::ShaderFile() {
    // !! keep constructor here to properly initialize containers, because type sizes and constructors are hidden !!
}
void ShaderFile::del() {
    // !! keep this to properly delete '_shaders', because type sizes and constructors are hidden !!
    _shaders.del();         // first delete this, then individual shaders
    _compute_shaders.del(); // first delete this, then individual shaders

    _vs.del();
    _hs.del();
    _ds.del();
    _ps.del();
    _cs.del();

    _buffer_links.del();
    _image_links.del();
    _rw_image_links.del();
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Shader *ShaderFile::shader(Int i) {
    if (InRange(i, _shaders)) {
        Shader &shader = _shaders[i];
        if (shader.validate(T))
            return &shader;
    }
    return null;
}
ComputeShader *ShaderFile::computeShader(Int i) {
    if (InRange(i, _compute_shaders)) {
        ComputeShader &shader = _compute_shaders[i];
        if (shader.validate(T))
            return &shader;
    }
    return null;
}

Shader *ShaderFile::find(C Str8 &name, Str *messages) {
    if (name.is())
        for (Int l = 0, r = _shaders.elms(); l < r;) {
            Int mid = UInt(l + r) / 2,
                compare = Compare(name, _shaders[mid].name, true);
            if (!compare) {
                Shader &shader = _shaders[mid];
                return shader.validate(T, messages) ? &shader : null;
            }
            if (compare < 0)
                r = mid;
            else
                l = mid + 1;
        }
    if (messages)
        *messages = "Technique not found in shader.";
    return null;
}
ComputeShader *ShaderFile::computeFind(C Str8 &name, Str *messages) {
    if (name.is())
        for (Int l = 0, r = _compute_shaders.elms(); l < r;) {
            Int mid = UInt(l + r) / 2,
                compare = Compare(name, _compute_shaders[mid].name, true);
            if (!compare) {
                ComputeShader &shader = _compute_shaders[mid];
                return shader.validate(T, messages) ? &shader : null;
            }
            if (compare < 0)
                r = mid;
            else
                l = mid + 1;
        }
    if (messages)
        *messages = "Technique not found in shader.";
    return null;
}

Shader *ShaderFile::find(C Str8 &name) { return find(name, null); }
ComputeShader *ShaderFile::computeFind(C Str8 &name) { return computeFind(name, null); }

Shader *ShaderFile::get(C Str8 &name) {
    if (name.is()) {
        Str messages;
        if (Shader *shader = find(name, &messages))
            return shader;
        Exit(S + "Error accessing Shader \"" + name + "\" in ShaderFile \"" + ShaderFiles.name(this) + "\"." + (messages.is() ? S + "\n" + messages : S));
    }
    return null;
}
ComputeShader *ShaderFile::computeGet(C Str8 &name) {
    if (name.is()) {
        Str messages;
        if (ComputeShader *shader = computeFind(name, &messages))
            return shader;
        Exit(S + "Error accessing ComputeShader \"" + name + "\" in ShaderFile \"" + ShaderFiles.name(this) + "\"." + (messages.is() ? S + "\n" + messages : S));
    }
    return null;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
#if GL
static INLINE void FlipY(Vtx2DTex *v, Int n) {
    if (!D.mainFBO())
        FREP(n)
    CHS(v[i].pos.y); // in OpenGL when drawing to RenderTarget the 'dest.pos.y' must be flipped
}
#endif
void Shader::draw(C Image *image, C Rect *rect) C {
    Sh.Img[0]->set(image);
    draw(rect);
}
void Shader::draw(C Rect *rect) C {
    if (!rect) // full screen/viewport
    {
    full:
        VI.shader(this);
        VI.setType(VI_2D_TEX, VI_STRIP);
        if (Vtx2DTex *v = (Vtx2DTex *)VI.addVtx(3)) // this draws 1 tri that's stretched to cover entire screen
        {
            v[0].pos.set(-1, 1);
            v[1].pos.set(3, 1);
            v[2].pos.set(-1, -3);
            if (D._view_active.full) {
                v[0].tex.set(0, 0);
                v[1].tex.set(2, 0);
                v[2].tex.set(0, 2);
            } else {
                C RectI &viewport = D._view_active.recti;
                Rect tex(Flt(viewport.min.x) / Renderer.resW(), Flt(viewport.min.y) / Renderer.resH(),
                         Flt(viewport.max.x + viewport.w()) / Renderer.resW(), Flt(viewport.max.y + viewport.h()) / Renderer.resH());
                v[0].tex.set(tex.min.x, tex.min.y);
                v[1].tex.set(tex.max.x, tex.min.y);
                v[2].tex.set(tex.min.x, tex.max.y);
            }
#if GL
            FlipY(v, 3);
#endif
        }
    } else {
        C RectI &viewport = D._view_active.recti;
        RectI recti = Renderer.screenToPixelI(*rect);
        Bool flip_x = (recti.max.x < recti.min.x),
             flip_y = (recti.max.y < recti.min.y);
        if (flip_x)
            recti.swapX();
        if (flip_y)
            recti.swapY();
        if (!Cuts(recti, viewport))
            return;
        if (!flip_x && !flip_y && Inside(viewport, recti))
            goto full;

        VI.shader(this);
        VI.setType(VI_2D_TEX, VI_STRIP);
        if (Vtx2DTex *v = (Vtx2DTex *)VI.addVtx(4)) {
            Flt xm = 2.0f / viewport.w(),
                ym = 2.0f / viewport.h();
            Rect frac((recti.min.x - viewport.min.x) * xm - 1, (viewport.max.y - recti.max.y) * ym - 1,
                      (recti.max.x - viewport.min.x) * xm - 1, (viewport.max.y - recti.min.y) * ym - 1);
            if (flip_x)
                frac.swapX();
            if (flip_y)
                frac.swapY();
            v[0].pos.set(frac.min.x, frac.max.y);
            v[1].pos.set(frac.max.x, frac.max.y);
            v[2].pos.set(frac.min.x, frac.min.y);
            v[3].pos.set(frac.max.x, frac.min.y);

            Rect tex(Flt(recti.min.x) / Renderer.resW(), Flt(recti.min.y) / Renderer.resH(),
                     Flt(recti.max.x) / Renderer.resW(), Flt(recti.max.y) / Renderer.resH());
            v[0].tex.set(tex.min.x, tex.min.y);
            v[1].tex.set(tex.max.x, tex.min.y);
            v[2].tex.set(tex.min.x, tex.max.y);
            v[3].tex.set(tex.max.x, tex.max.y);
#if GL
            FlipY(v, 4);
#endif
        }
    }
    VI.end();
}
void Shader::draw(C Image *image, C Rect *rect, C Rect &tex) C {
    Sh.Img[0]->set(image);
    draw(rect, tex);
}
void Shader::draw(C Rect *rect, C Rect &tex) C {
    VI.shader(this);
    VI.setType(VI_2D_TEX, VI_STRIP);
    if (Vtx2DTex *v = (Vtx2DTex *)VI.addVtx(4)) {
        if (!D._view_active.full || rect) {
            C RectI &viewport = D._view_active.recti;
            RectI recti;

            if (!rect) {
                recti = viewport;
                v[0].pos.set(-1, 1);
                v[1].pos.set(1, 1);
                v[2].pos.set(-1, -1);
                v[3].pos.set(1, -1);
            } else {
                recti = Renderer.screenToPixelI(*rect);
                Bool flip_x = (recti.max.x < recti.min.x),
                     flip_y = (recti.max.y < recti.min.y);
                if (flip_x)
                    recti.swapX();
                if (flip_y)
                    recti.swapY();
                if (!Cuts(recti, viewport)) {
                    VI.clear();
                    return;
                }
                Flt xm = 2.0f / viewport.w(),
                    ym = 2.0f / viewport.h();
                Rect frac((recti.min.x - viewport.min.x) * xm - 1, (viewport.max.y - recti.max.y) * ym - 1,
                          (recti.max.x - viewport.min.x) * xm - 1, (viewport.max.y - recti.min.y) * ym - 1);
                if (flip_x)
                    frac.swapX();
                if (flip_y)
                    frac.swapY();
                v[0].pos.set(frac.min.x, frac.max.y);
                v[1].pos.set(frac.max.x, frac.max.y);
                v[2].pos.set(frac.min.x, frac.min.y);
                v[3].pos.set(frac.max.x, frac.min.y);
            }
        } else {
            v[0].pos.set(-1, 1);
            v[1].pos.set(1, 1);
            v[2].pos.set(-1, -1);
            v[3].pos.set(1, -1);
        }
        v[0].tex.set(tex.min.x, tex.min.y);
        v[1].tex.set(tex.max.x, tex.min.y);
        v[2].tex.set(tex.min.x, tex.max.y);
        v[3].tex.set(tex.max.x, tex.max.y);
#if GL
        FlipY(v, 4);
#endif
    }
    VI.end();
}
/******************************************************************************/
// IO
/******************************************************************************/
static ShaderBuffer *Get(Int i, C MemtN<ShaderBuffer *, 256> &buffers) {
    RANGE_ASSERT_ERROR(i, buffers, "Invalid ShaderBuffer index");
    return buffers[i];
}
static ShaderImage *Get(Int i, C MemtN<ShaderImage *, 256> &images) {
    RANGE_ASSERT_ERROR(i, images, "Invalid ShaderImage index");
    return images[i];
}
static ShaderRWImage *Get(Int i, C MemtN<ShaderRWImage *, 256> &images) {
    RANGE_ASSERT_ERROR(i, images, "Invalid ShaderRWImage index");
    return images[i];
}
/******************************************************************************/
Int ExpectedBufferSlot(C Str8 &name) {
    if (name == "Frame")
        return SBI_FRAME;
    if (name == "Camera")
        return SBI_CAMERA;
    if (name == "ObjMatrix")
        return SBI_OBJ_MATRIX;
    if (name == "ObjMatrixPrev")
        return SBI_OBJ_MATRIX_PREV;
    if (name == "Mesh")
        return SBI_MESH;
    if (name == "Material")
        return SBI_MATERIAL;
    if (name == "Viewport")
        return SBI_VIEWPORT;
    if (name == "Color")
        return SBI_COLOR;
    ASSERT(SBI_NUM == 8);
    return -1;
}
Int GetSamplerIndex(CChar8 *name) {
    if (Equal(name, "SamplerRender"))
        return SSI_RENDER;
    if (Equal(name, "SamplerPoint"))
        return SSI_POINT;
    if (Equal(name, "SamplerLinearClamp"))
        return SSI_LINEAR_CLAMP;
    if (Equal(name, "SamplerLinearWrap"))
        return SSI_LINEAR_WRAP;
    if (Equal(name, "SamplerLinearCWW"))
        return SSI_LINEAR_CWW;
    if (Equal(name, "SamplerShadowMap"))
        return SSI_SHADOW;
    if (Equal(name, "SamplerFont"))
        return SSI_FONT;
    if (Equal(name, "SamplerMinimum"))
        return SSI_MINIMUM;
    if (Equal(name, "SamplerMaximum"))
        return SSI_MAXIMUM;
    ASSERT(SSI_NUM == 9);
    return -1;
}
static void TestBuffer(C Str8 &name, Int bind_slot) {
    Int expected = ExpectedBufferSlot(name);
    if (expected == bind_slot || expected < 0 && (bind_slot < 0 || bind_slot >= SBI_NUM))
        return;
    Exit(S + "Shader Buffer \"" + name + "\" was expected to be at slot: " + expected + ", but got: " + bind_slot);
}
static void TestBuffer(C ShaderBuffer *buffer, Int bind_slot) {
    C Str8 *name = ShaderBuffers.dataToKey(buffer);
    if (!name)
        Exit("Can't find ShaderBuffer name");
    return TestBuffer(*name, bind_slot);
}
static Bool Test(C CMemPtr<Mems<BufferLink>> &links) {
    REPA(links) {
        C Mems<BufferLink> &link = links[i];
        if (link.elms() > 1) {
            Int first = link[0].index;
            for (Int i = 1; i < link.elms(); i++)
                if (link[i].index != first + i)
                    Exit("Invalid Buffer index");
        }
    }
    return true;
}
static Bool Test(C CMemPtr<Mems<ImageLink>> &links) {
    REPA(links) {
        C Mems<ImageLink> &link = links[i];
        if (link.elms() > 1) {
            Int first = link[0].index;
            for (Int i = 1; i < link.elms(); i++)
                if (link[i].index != first + i)
                    Exit("Invalid Image index");
        }
    }
    return true;
}
static Bool Test(C CMemPtr<Mems<RWImageLink>> &links) {
    REPA(links) {
        C Mems<RWImageLink> &link = links[i];
        if (link.elms() > 1) {
            Int first = link[0].index;
            for (Int i = 1; i < link.elms(); i++)
                if (link[i].index != first + i)
                    Exit("Invalid RW Image index");
        }
    }
    return true;
}
static void LoadTranslation(MemPtr<ShaderParam::Translation> translation, File &f, Int elms) {
    if (elms <= 1)
        translation.loadRaw(f);
    else {
        UShort single_translations, gpu_offset, cpu_offset;
        f.getMulti(gpu_offset, cpu_offset, single_translations);
        translation.setNum(single_translations * elms);
        Int t = 0;
        FREPS(t, single_translations)
        f >> translation[t];                           // load 1st element translation
        for (Int e = 1, co = 0, go = 0; e < elms; e++) // add rest of the elements
        {
            co += cpu_offset; // offset between elements
            go += gpu_offset; // offset between elements
            FREP(single_translations) {
                ShaderParam::Translation &trans = translation[t++];
                trans = translation[i];
                trans.cpu_offset += co;
                trans.gpu_offset += go;
            }
        }
    }
}
void ConstantIndex::set(Int bind_index, Int src_index) {
    _Unaligned(T.bind_index, bind_index);
    _Unaligned(T.src_index, src_index);
    DYNAMIC_ASSERT(T.bind_index == bind_index && T.src_index == src_index, "Constant index out of range");
}
/******************************************************************************/
#if !GL
Bool BufferLink::load(File &f, C MemtN<ShaderBuffer *, 256> &buffers) {
    ConstantIndex ci;
    f >> ci;
    index = ci.bind_index;
    RANGE_ASSERT_ERROR(index, MAX_SHADER_BUFFERS, S + "Buffer index: " + index + ", is too big");
    buffer = Get(ci.src_index, buffers);
    if (DEBUG) {
        TestBuffer(buffer, index);
        DYNAMIC_ASSERT(index >= SBI_NUM, "This buffer should be always bound at startup and not while setting shader");
    }
    return f.ok();
}
Bool ImageLink::load(File &f, C MemtN<ShaderImage *, 256> &images) {
    ConstantIndex ci;
    f >> ci;
    index = ci.bind_index;
    RANGE_ASSERT_ERROR(index, MAX_SHADER_IMAGES, S + "Image index: " + index + ", is too big");
    image = Get(ci.src_index, images);
    return f.ok();
}
Bool RWImageLink::load(File &f, C MemtN<ShaderRWImage *, 256> &images) {
    ConstantIndex ci;
    f >> ci;
    index = ci.bind_index;
    RANGE_ASSERT_ERROR(index, MAX_SHADER_IMAGES, S + "RW Image index: " + index + ", is too big");
    image = Get(ci.src_index, images);
    return f.ok();
}
#endif
/******************************************************************************/
#if WINDOWS
Bool Shader11::load(File &f, C ShaderFile &shader_file, C MemtN<ShaderBuffer *, 256> &file_buffers) {
    ShaderIndex indexes[ST_BASE];
    f.getStr(name) >> indexes;
    FREPA(data_index) {
        C ShaderIndex &index = indexes[i];
        data_index[i] = index.shader_data_index;
        RANGE_ASSERT_ERROR(index.buffer_bind_index, shader_file._buffer_links, "Buffer Bind Index out of range");
        buffers[i] = shader_file._buffer_links[index.buffer_bind_index];
        RANGE_ASSERT_ERROR(index.image_bind_index, shader_file._image_links, "Image Bind Index out of range");
        images[i] = shader_file._image_links[index.image_bind_index];
    }
    all_buffers.setNum(f.decUIntV());
    FREPAO(all_buffers) = Get(f.getUShort(), file_buffers);
    if (f.ok())
        return true;
    /*del();*/ return false;
}
Bool ComputeShader11::load(File &f, C ShaderFile &shader_file, C MemtN<ShaderBuffer *, 256> &file_buffers) {
    ComputeShaderIndex index;
    f.getStr(name) >> index;
    data_index = index.shader_data_index;
    RANGE_ASSERT_ERROR(index.buffer_bind_index, shader_file._buffer_links, "Buffer Bind Index out of range");
    buffers = shader_file._buffer_links[index.buffer_bind_index];
    RANGE_ASSERT_ERROR(index.image_bind_index, shader_file._image_links, "Image Bind Index out of range");
    images = shader_file._image_links[index.image_bind_index];
    RANGE_ASSERT_ERROR(index.rw_image_bind_index, shader_file._rw_image_links, "RW Image Bind Index out of range");
    rw_images = shader_file._rw_image_links[index.rw_image_bind_index];
    all_buffers.setNum(f.decUIntV());
    FREPAO(all_buffers) = Get(f.getUShort(), file_buffers);
    if (f.ok())
        return true;
    /*del();*/ return false;
}
#endif
/******************************************************************************/
#if GL
Bool ShaderGL::load(File &f, C ShaderFile &shader_file, C MemtN<ShaderBuffer *, 256> &buffers) {
    // name + indexes
    f.getStr(name).getMulti(vs_index, ps_index);
    if (f.ok())
        return true;
    /*del();*/ return false;
}
Bool ComputeShaderGL::load(File &f, C ShaderFile &shader_file, C MemtN<ShaderBuffer *, 256> &buffers) {
    // name + indexes
    f.getStr(name) >> cs_index;
    if (f.ok())
        return true;
    /*del();*/ return false;
}
#endif
/******************************************************************************/
static void ExitParam(C Str &param_name, C Str &shader_name) {
    Exit(S + "Shader Param \"" + param_name + "\"\nfrom Shader File \"" + shader_name + "\"\nAlready exists in Shader Constants Map but with different parameters.\nThis means that some of your shaders were compiled with different headers.\nPlease recompile your shaders.");
}
Bool ShaderFile::load(C Str &name) {
    del();

    Str8 temp_str;
    File f;
    if (f.read(Sh.path + name) && f.getUInt() == CC4_SHDR && f.getByte() == GPU_API(API_DX, API_GL))
        switch (f.decUIntV()) // version
        {
        case 0: {
            // buffers
            MemtN<ShaderBuffer *, 256> buffers;
            buffers.setNum(f.decUIntV());
            ShaderBuffers.lock();
            ShaderParams.lock();
            FREPA(buffers) {
                // buffer
                f.getStr(temp_str);
                ShaderBuffer &sb = *ShaderBuffers(temp_str);
                buffers[i] = &sb;
                if (!sb.is()) // wasn't yet created
                {
                    sb.create(f.decUIntV());
                    f >> sb.explicit_bind_slot;
                    if (sb.explicit_bind_slot >= 0) {
                        SyncLocker lock(D._lock);
                        sb.bind(sb.explicit_bind_slot);
                    }
                    if (DEBUG)
                        TestBuffer(temp_str, sb.explicit_bind_slot);
                } else // verify if it's identical to previously created
                {
                    if (sb.full_size != f.decUIntV())
                        ExitParam(temp_str, name);
                    sb.bindCheck(f.getSByte());
                }

                // params
                REP(f.decUIntV()) {
                    f.getStr(temp_str);
                    ShaderParam &sp = *ShaderParams(temp_str);
                    if (!sp.is()) // wasn't yet created
                    {
                        sp._data = sb.data;
                        sp._changed = &sb.changed;
                        f.getMulti(sp._cpu_data_size, sp._gpu_data_size, sp._elements); // info
                        LoadTranslation(sp._full_translation, f, sp._elements);         // translation
                        Int offset = sp._full_translation[0].gpu_offset;
                        sp._data += offset;
                        REPAO(sp._full_translation).gpu_offset -= offset; // apply offset. 'gpu_offset' is stored relative to start of cbuffer, however when loading we want to adjust 'sp.data' to point to the start of the param, so since we're adjusting it we have to adjust 'gpu_offset' too.
                        if (f.getBool())
                            f.get(sp._data, sp._gpu_data_size); // load default value, no need to zero in other case, because data is stored in ShaderBuffer's, and they're always zeroed at start
                        sp.optimize();                          // optimize
                    } else                                      // verify if it's identical to previously created
                    {
                        UInt cpu_data_size, gpu_data_size, elements;
                        f.getMulti(cpu_data_size, gpu_data_size, elements);
                        Memt<ShaderParam::Translation> translation;
                        if (sp._changed != &sb.changed            // check matching Constant Buffer
                            || sp._cpu_data_size != cpu_data_size // check cpu size
                            || sp._gpu_data_size != gpu_data_size // check gpu size
                            || sp._elements != elements)
                            ExitParam(temp_str, name);                 // check number of elements
                        LoadTranslation(translation, f, sp._elements); // translation
                        Int offset = translation[0].gpu_offset;
                        REPAO(translation).gpu_offset -= offset; // apply offset
                        if (f.getBool())
                            f.skip(gpu_data_size); // ignore default value

                        // check translation
                        if (translation.elms() != sp._full_translation.elms())
                            ExitParam(temp_str, name);
                        FREPA(translation)
                        if (translation[i] != sp._full_translation[i])
                            ExitParam(temp_str, name);
                    }
                }
            }
            ShaderParams.unlock();
            ShaderBuffers.unlock();

            // images
            MemtN<ShaderImage *, 256> images;
            images.setNum(f.decUIntV());
            FREPA(images) {
                f.getStr(temp_str);
                images[i] = ShaderImages(temp_str);
            }
            MemtN<ShaderRWImage *, 256> rw_images;
            rw_images.setNum(f.decUIntV());
            FREPA(rw_images) {
                f.getStr(temp_str);
                rw_images[i] = ShaderRWImages(temp_str);
            }

            // shaders
#if !GL
            if (_buffer_links.load(f, buffers))             //   buffer link map
                if (_image_links.load(f, images))           //    image link map
                    if (_rw_image_links.load(f, rw_images)) // rw image link map
#if DEBUG
                        if (Test(_buffer_links))
                            if (Test(_image_links))
                                if (Test(_rw_image_links))
#endif
#endif
                                    if (_vs.load(f))
                                        if (_hs.load(f))
                                            if (_ds.load(f))
                                                if (_ps.load(f))
                                                    if (_cs.load(f))
                                                        if (_shaders.load(f, T, buffers))
                                                            if (_compute_shaders.load(f, T, buffers))
                                                                if (f.ok())
                                                                    return true;
        } break;
        }
    // error:
    del();
    return false;
}
/******************************************************************************/
void DisplayState::clearShader() {
    // set ~0 for pointers because that's the most unlikely value that they would have
#if DX11
    SetMem(VSTex, ~0);
    SetMem(HSTex, ~0);
    SetMem(DSTex, ~0);
    SetMem(PSTex, ~0);
    SetMem(CSTex, ~0);
    SetMem(CSUAV, ~0);
    SetMem(VSBuf, ~0);
    SetMem(HSBuf, ~0);
    SetMem(DSBuf, ~0);
    SetMem(PSBuf, ~0);
    SetMem(VShader, ~0);
    SetMem(HShader, ~0);
    SetMem(DShader, ~0);
    SetMem(PShader, ~0);
#elif GL
    SetMem(Tex, ~0);
    SetMem(UAV, ~0);
#endif
}
/******************************************************************************/
// FORWARD RENDERER SHADER
/******************************************************************************/
static Int Compare(C FRSTKey &a, C FRSTKey &b) {
    if (Int c = Compare(a.skin, b.skin))
        return c;
    if (Int c = Compare(a.materials, b.materials))
        return c;
    if (Int c = Compare(a.layout, b.layout))
        return c;
    if (Int c = Compare(a.bump_mode, b.bump_mode))
        return c;
    if (Int c = Compare(a.alpha_test, b.alpha_test))
        return c;
    if (Int c = Compare(a.reflect, b.reflect))
        return c;
    if (Int c = Compare(a.emissive_map, b.emissive_map))
        return c;
    if (Int c = Compare(a.detail, b.detail))
        return c;
    if (Int c = Compare(a.color, b.color))
        return c;
    if (Int c = Compare(a.mtrl_blend, b.mtrl_blend))
        return c;
    if (Int c = Compare(a.uv_scale, b.uv_scale))
        return c;
    if (Int c = Compare(a.heightmap, b.heightmap))
        return c;
    if (Int c = Compare(a.fx, b.fx))
        return c;
    if (Int c = Compare(a.per_pixel, b.per_pixel))
        return c;
    if (Int c = Compare(a.tesselate, b.tesselate))
        return c;
    return 0;
}
static Bool Create(FRST &frst, C FRSTKey &key, Ptr) {
    ShaderFile *shader_file = ShaderFiles("Forward");
    if (key.bump_mode == SBUMP_ZERO) {
        Shader *shader = shader_file->get(ShaderForward(key.skin, key.materials, key.layout, key.bump_mode, key.alpha_test, key.reflect, key.emissive_map, key.detail, key.color, key.mtrl_blend, key.uv_scale, key.heightmap, key.fx, key.per_pixel, false, 0, 0, false, 0, false, 0, false, 0, false));
        frst.all_passes = false;
        frst.none = shader;
        frst.dir = shader;
        frst.point = shader;
        frst.linear = shader;
        frst.cone = shader;
        REPAO(frst.dir_shd) = shader;
        frst.point_shd = shader;
        frst.linear_shd = shader;
        frst.cone_shd = shader;
    } else {
        frst.all_passes = true;
        frst.none = shader_file->get(ShaderForward(key.skin, key.materials, key.layout, key.bump_mode, key.alpha_test, key.reflect, key.emissive_map, key.detail, key.color, key.mtrl_blend, key.uv_scale, key.heightmap, key.fx, key.per_pixel, false, false, 0, false, false, false, false, false, false, key.tesselate));
        frst.dir = shader_file->get(ShaderForward(key.skin, key.materials, key.layout, key.bump_mode, key.alpha_test, key.reflect, key.emissive_map, key.detail, key.color, key.mtrl_blend, key.uv_scale, key.heightmap, key.fx, key.per_pixel, true, false, 0, false, false, false, false, false, false, key.tesselate));
        frst.point = shader_file->get(ShaderForward(key.skin, key.materials, key.layout, key.bump_mode, key.alpha_test, key.reflect, key.emissive_map, key.detail, key.color, key.mtrl_blend, key.uv_scale, key.heightmap, key.fx, key.per_pixel, false, false, 0, true, false, false, false, false, false, key.tesselate));
        frst.linear = shader_file->get(ShaderForward(key.skin, key.materials, key.layout, key.bump_mode, key.alpha_test, key.reflect, key.emissive_map, key.detail, key.color, key.mtrl_blend, key.uv_scale, key.heightmap, key.fx, key.per_pixel, false, false, 0, false, false, true, false, false, false, key.tesselate));
        frst.cone = shader_file->get(ShaderForward(key.skin, key.materials, key.layout, key.bump_mode, key.alpha_test, key.reflect, key.emissive_map, key.detail, key.color, key.mtrl_blend, key.uv_scale, key.heightmap, key.fx, key.per_pixel, false, false, 0, false, false, false, false, true, false, key.tesselate));

        if (D.shadowSupported()) {
            REPAO(frst.dir_shd) = shader_file->get(ShaderForward(key.skin, key.materials, key.layout, key.bump_mode, key.alpha_test, key.reflect, key.emissive_map, key.detail, key.color, key.mtrl_blend, key.uv_scale, key.heightmap, key.fx, key.per_pixel, true, true, Ceil2(i + 1), false, false, false, false, false, false, key.tesselate));
            frst.point_shd = shader_file->get(ShaderForward(key.skin, key.materials, key.layout, key.bump_mode, key.alpha_test, key.reflect, key.emissive_map, key.detail, key.color, key.mtrl_blend, key.uv_scale, key.heightmap, key.fx, key.per_pixel, false, false, 0, true, true, false, false, false, false, key.tesselate));
            frst.linear_shd = shader_file->get(ShaderForward(key.skin, key.materials, key.layout, key.bump_mode, key.alpha_test, key.reflect, key.emissive_map, key.detail, key.color, key.mtrl_blend, key.uv_scale, key.heightmap, key.fx, key.per_pixel, false, false, 0, false, false, true, true, false, false, key.tesselate));
            frst.cone_shd = shader_file->get(ShaderForward(key.skin, key.materials, key.layout, key.bump_mode, key.alpha_test, key.reflect, key.emissive_map, key.detail, key.color, key.mtrl_blend, key.uv_scale, key.heightmap, key.fx, key.per_pixel, false, false, 0, false, false, false, false, true, true, key.tesselate));
        } else {
            REPAO(frst.dir_shd) = null;
            frst.point_shd = null;
            frst.linear_shd = null;
            frst.cone_shd = null;
        }
    }
    return true;
}
ThreadSafeMap<FRSTKey, FRST> Frsts(Compare, Create);
/******************************************************************************/
// BLEND LIGHT SHADER
/******************************************************************************/
static Int Compare(C BLSTKey &a, C BLSTKey &b) {
    if (Int c = Compare(a.skin, b.skin))
        return c;
    if (Int c = Compare(a.color, b.color))
        return c;
    if (Int c = Compare(a.layout, b.layout))
        return c;
    if (Int c = Compare(a.bump_mode, b.bump_mode))
        return c;
    if (Int c = Compare(a.alpha_test, b.alpha_test))
        return c;
    if (Int c = Compare(a.alpha, b.alpha))
        return c;
    if (Int c = Compare(a.reflect, b.reflect))
        return c;
    if (Int c = Compare(a.emissive_map, b.emissive_map))
        return c;
    if (Int c = Compare(a.uv_scale, b.uv_scale))
        return c;
    if (Int c = Compare(a.fx, b.fx))
        return c;
    if (Int c = Compare(a.per_pixel, b.per_pixel))
        return c;
    return 0;
}
static Bool Create(BLST &blst, C BLSTKey &key, Ptr) {
    ShaderFile *shader = ShaderFiles("Blend Light");
    blst.dir[0] = shader->get(ShaderBlendLight(key.skin, key.color, key.layout, key.bump_mode, key.alpha_test, key.alpha, key.reflect, key.emissive_map, key.uv_scale, key.fx, key.per_pixel, 0, 0));
    if (D.shadowSupported()) {
        REP(6)
        blst.dir[i + 1] = shader->get(ShaderBlendLight(key.skin, key.color, key.layout, key.bump_mode, key.alpha_test, key.alpha, key.reflect, key.emissive_map, key.uv_scale, key.fx, key.per_pixel, i + 1, 0));
    } else {
        REP(6)
        blst.dir[i + 1] = blst.dir[0];
    }
    return true;
}
ThreadSafeMap<BLSTKey, BLST> Blsts(Compare, Create);
/******************************************************************************
can't be used because in RM_PREPARE we add models to the list and lights simultaneously
Shader* FRST::getShader()
{
   return *(Shader**)(((Byte*)this)+Renderer._frst_light_offset);
}
/******************************************************************************/
Int Matrixes, FurVels;
#if DX11
static Int MatrixesPart, FurVelPart;
static Byte BoneNumToPart[256 + 1];
#endif
static ShaderBuffer *SBObjMatrix, *SBObjMatrixPrev, *SBFurVel;
void SetMatrixCount(Int num) {
    if (Matrixes != num) {
        Matrixes = num;
        // !! Warning: for performance reasons this doesn't adjust 'ShaderParam.translation', so using 'ShaderParam.set*' based on translation will use full size, so make sure that method isn't called for 'ObjMatrix' and 'ObjMatrixPrev' !!
#if DX11
#if ALLOW_PARTIAL_BUFFERS
        if (D3DC1) {
            SBObjMatrix->buffer.size = SIZE(GpuMatrix) * Matrixes;
            SBObjMatrixPrev->buffer.size = SIZE(GpuMatrix) * Matrixes;
            Int m16 = Ceil16(Matrixes * 3);
            if (MatrixesPart != m16) {
                MatrixesPart = m16;
                // Warning: code below does not set the cached buffers as 'bind' does, as it's not needed, because those buffers have constant bind index
                ASSERT(SBI_OBJ_MATRIX_PREV == SBI_OBJ_MATRIX + 1); // can do this only if they're next to each other
                UInt first[] = {0, 0},                             // must be provided or DX will fail
                    num[] = {Ceil16(Matrixes * 3), Ceil16(Matrixes * 3)};
                ID3D11Buffer *buf[] = {SBObjMatrix->buffer.buffer, SBObjMatrixPrev->buffer.buffer};
                D3DC1->VSSetConstantBuffers1(SBI_OBJ_MATRIX, 2, buf, first, num);
                D3DC1->HSSetConstantBuffers1(SBI_OBJ_MATRIX, 2, buf, first, num);
                D3DC1->DSSetConstantBuffers1(SBI_OBJ_MATRIX, 2, buf, first, num);
                D3DC1->PSSetConstantBuffers1(SBI_OBJ_MATRIX, 2, buf, first, num);
                D3DC1->CSSetConstantBuffers1(SBI_OBJ_MATRIX, 2, buf, first, num);
            }
        } else
#endif
        {
            Int part = BoneNumToPart[num];
            if (MatrixesPart != part) {
                MatrixesPart = part;
                SBObjMatrix->setPart(part);
                SBObjMatrixPrev->setPart(part);
#if 0
            SBObjMatrix    ->bind(SBI_OBJ_MATRIX     );
            SBObjMatrixPrev->bind(SBI_OBJ_MATRIX_PREV);
#else // bind 2 at the same time \
      // Warning: code below does not set the cached buffers as 'bind' does, as it's not needed, because those buffers have constant bind index
                ASSERT(SBI_OBJ_MATRIX_PREV == SBI_OBJ_MATRIX + 1); // can do this only if they're next to each other
                ID3D11Buffer *buf[] = {SBObjMatrix->buffer.buffer, SBObjMatrixPrev->buffer.buffer};
                D3DC->VSSetConstantBuffers(SBI_OBJ_MATRIX, 2, buf);
                D3DC->HSSetConstantBuffers(SBI_OBJ_MATRIX, 2, buf);
                D3DC->DSSetConstantBuffers(SBI_OBJ_MATRIX, 2, buf);
                D3DC->PSSetConstantBuffers(SBI_OBJ_MATRIX, 2, buf);
                D3DC->CSSetConstantBuffers(SBI_OBJ_MATRIX, 2, buf);
#endif
            }
        }
#elif GL
        // will affect 'ShaderBuffer::update()'
        SBObjMatrix->buffer.size = SIZE(GpuMatrix) * Matrixes;
        SBObjMatrixPrev->buffer.size = SIZE(GpuMatrix) * Matrixes;
#endif
    }
}
void SetFurVelCount(Int num) // !! unlike 'SetMatrixCount' this needs to be called before Shader start/begin, because it doesn't bind the new buffer !!
{
    if (FurVels != num) {
        FurVels = num;

#if DX11
        Int part = BoneNumToPart[num];
        if (FurVelPart != part)
            SBFurVel->setPart(FurVelPart = part);
#elif GL
        SBFurVel->buffer.size = SIZE(Vec4) * num;
#endif
    }
}
/******************************************************************************/
void InitMatrix() {
    ViewMatrix = Sh.ViewMatrix->asGpuMatrix();
    ViewMatrixPrev = Sh.ViewMatrixPrev->asGpuMatrix();

    DYNAMIC_ASSERT(Sh.ViewMatrix->_cpu_data_size == SIZE(GpuMatrix) * MAX_MATRIX, "Unexpected size of 'ViewMatrix'");
    DYNAMIC_ASSERT(Sh.ViewMatrixPrev->_cpu_data_size == SIZE(GpuMatrix) * MAX_MATRIX, "Unexpected size of 'ViewMatrixPrev'");
    DYNAMIC_ASSERT(Sh.FurVel->_cpu_data_size == SIZE(Vec) * MAX_MATRIX, "Unexpected size of 'FurVel'");

    SBObjMatrix = GetShaderBuffer("ObjMatrix");
    DYNAMIC_ASSERT(SBObjMatrix->full_size == SIZE(GpuMatrix) * MAX_MATRIX, "Unexpected size of 'ObjMatrix'");
    SBObjMatrixPrev = GetShaderBuffer("ObjMatrixPrev");
    DYNAMIC_ASSERT(SBObjMatrixPrev->full_size == SIZE(GpuMatrix) * MAX_MATRIX, "Unexpected size of 'ObjMatrixPrev'");
    SBFurVel = GetShaderBuffer("FurVel");
    DYNAMIC_ASSERT(SBFurVel->full_size == SIZE(Vec4) * MAX_MATRIX, "Unexpected size of 'FurVel'");

#if DX11
    const Int parts[] = {MAX_MATRIX, 192, 160, 128, 96, 80, 64, 56, 48, 32, 16, 8, 1}; // start from the biggest, because 'ShaderBuffer.size' uses it as the total size
    if (!ALLOW_PARTIAL_BUFFERS || !D3DC1)                                              // have to create parts only if we won't use partial buffers
    {
        SBObjMatrix->createParts(parts, Elms(parts));
        SBObjMatrixPrev->createParts(parts, Elms(parts));
    }
    SBFurVel->createParts(parts, Elms(parts));
    Int end = Elms(BoneNumToPart);
    for (Int i = 0; i < Elms(parts) - 1; i++) {
        Int start = parts[i + 1] + 1;
        SetMem(&BoneNumToPart[start], i, end - start);
        end = start;
    }
    REP(end)
    BoneNumToPart[i] = Elms(parts) - 1;
#elif GL
#if !WINDOWS && !SWITCH
    glBindImageTexture = (decltype(glBindImageTexture))D.glGetProcAddress("glBindImageTexture");
    glDispatchCompute = (decltype(glDispatchCompute))D.glGetProcAddress("glDispatchCompute");
#endif
#endif
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
