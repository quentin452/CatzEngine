#pragma once
/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************/
#if EE_PRIVATE

#define CC4_SHDR CC4('S', 'H', 'D', 'R')

#if 1 // with new shader compilers, the generated shaders are small, so disable compression to get best performance
#define COMPRESS_GL_SHADER COMPRESS_NONE
#define COMPRESS_GL_SHADER_LEVEL 0
#elif 0
#define COMPRESS_GL_SHADER COMPRESS_LZ4
#define COMPRESS_GL_SHADER_LEVEL 99
#elif 0
#define COMPRESS_GL_SHADER COMPRESS_ZSTD
#define COMPRESS_GL_SHADER_LEVEL 99
#elif 1
#define COMPRESS_GL_SHADER COMPRESS_LZMA
#define COMPRESS_GL_SHADER_LEVEL 9 // shader files are small, so we can use high compression level and still get small dictionary size / memory usage
#else                              // shader size was slightly bigger than LZMA, and loading all shaders was bit slower
#define COMPRESS_GL_SHADER COMPRESS_LZHAM
#define COMPRESS_GL_SHADER_LEVEL 5
#endif

#define SUPPORT_FORWARD_DETAIL 0    // disable to reduce shader size
#define SUPPORT_FORWARD_TESSELATE 0 // disable to reduce shader size

Int ExpectedBufferSlot(C Str8 &name);
Int GetSamplerIndex(CChar8 *name);

enum API : Byte // !! These enums are saved !!
{
    API_DX,
    API_GL,
    API_VULKAN,
    API_METAL,
    API_NUM,
};

enum SC_FLAG : Byte {
    SC_NONE = 0,
    SC_SPIRV = 1 << 0,
};
SET_ENUM_FLAGS(SC_FLAG);

struct ShaderCompiler {
    struct Param {
        Str8 name;
        Int array_elms, cpu_data_size = 0, gpu_data_size;
        Memc<ShaderParam::Translation> translation;
        Mems<Byte> data;

        Bool operator==(C Param &p) C;
        Bool operator!=(C Param &p) C { return !(T == p); }

        Bool save(File &f) C;
#if WINDOWS
        void addTranslation(ID3D11ShaderReflectionType *type, C D3D11_SHADER_TYPE_DESC &type_desc, CChar8 *name, Int &offset, SByte &was_min16); // 'was_min16'=if last inserted parameter was of min16 type (-1=no last parameter)
        void addTranslation(ID3D12ShaderReflectionType *type, C D3D12_SHADER_TYPE_DESC &type_desc, CChar8 *name, Int &offset, Bool true_half);
#if SPIRV_CROSS
        void addTranslation(spvc_compiler compiler, spvc_type_id parent_id, spvc_type parent, spvc_type_id var_id, spvc_type var, Int var_i, Int offset, C Str8 &names = S8);
#endif
#endif
        void sortTranslation();
        void setDataFrom(C Param &src);
    };
    struct Bind {
        Str8 name;
        Int bind_slot;

        Bool operator==(C Bind &b) C { return bind_slot == b.bind_slot && Equal(name, b.name, true); }
        Bool operator!=(C Bind &b) C { return !(T == b); }
    };
    struct Buffer : Bind {
        Int size;
        Bool bind_explicit;
        Mems<Param> params;

        Int explicitBindSlot() C { return bind_explicit ? bind_slot : -1; }
        Bool operator==(C Buffer &b) C; // this checks 'bind_slot' only for 'bind_explicit'
        Bool operator!=(C Buffer &b) C { return !(T == b); }
        Param *findParam(C Str8 &name);
        Param &getParam(C Str8 &name);
    };
    struct Image : Bind {
    };
    struct IO {
        Byte index, reg, type, precision, mask;
        Str8 name;

#if WINDOWS
        void operator=(C D3D11_SIGNATURE_PARAMETER_DESC &desc) {
            name = desc.SemanticName;
            index = desc.SemanticIndex;
            reg = desc.Register;
            type = desc.ComponentType;
            precision = desc.MinPrecision;
            mask = desc.Mask;
        }
        void operator=(C D3D12_SIGNATURE_PARAMETER_DESC &desc) {
            name = desc.SemanticName;
            index = desc.SemanticIndex;
            reg = desc.Register;
            type = desc.ComponentType;
            precision = desc.MinPrecision;
            mask = desc.Mask;
        }
#endif
        Bool operator==(C IO &io) C { return index == io.index && reg == io.reg && type == io.type && precision == io.precision && mask == io.mask && Equal(name, io.name, true); }
        Bool operator!=(C IO &io) C { return !(T == io); }
    };

    enum RESULT : Byte {
        NONE,
        FAIL,
        GOOD,
    };

    struct Shader;
    struct SubShader {
        C Shader *shader;
        SHADER_TYPE type;
        RESULT result = NONE;
        Str8 func_name;
        Str error;
        Mems<Buffer> buffers;
        Mems<Image> images, rw_images;
        Mems<IO> inputs, outputs;
        ShaderData shader_data;
        Int shader_data_index = -1,   // index of   'ShaderData' in 'ShaderFile'
            buffer_bind_index = -1,   // index of   buffer binds in 'ShaderFile'
            image_bind_index = -1,    // index of    image binds in 'ShaderFile'
            rw_image_bind_index = -1; // index of rw image binds in 'ShaderFile'

        Bool is() C { return func_name.is(); }
        void compile();
    };

    struct TextParam8 {
        Str8 name, value;
        void set(C Str8 &name, C Str8 &value) {
            T.name = name;
            T.value = value;
        }
    };

    struct Source;
    struct Shader {
        Str name;
        SHADER_MODEL model;
        Bool dummy = false; // if skip saving this shader, this is used only to access info about ConstantBuffers/ShaderParams
        Memc<TextParam8> params,
            extra_params; // params not affecting shader name
        SubShader sub[ST_NUM];
        C Source *source;

        API api() C;
        Bool compute() C { return sub[ST_CS].is(); } // if this is a compute shader

        Shader &multiSample() {
            MAX(model, SM_4_1);
            return T;
        } // SM_4_1 needed for 'SV_SampleIndex'
        Shader &multiSample(Bool on) {
            if (on)
                multiSample();
            return T;
        } // SM_4_1 needed for 'SV_SampleIndex'
        Shader &gather() {
            MAX(model, SM_4_1);
            return T;
        } // SM_4_1 needed for Texture Gather
        Shader &gather(Bool on) {
            if (on)
                gather();
            return T;
        } // SM_4_1 needed for Texture Gather
        Shader &gatherChannel() {
            MAX(model, SM_5);
            return T;
        } // SM_5   needed for Texture Gather per-channel
        Shader &gatherChannel(Bool on) {
            if (on)
                gatherChannel();
            return T;
        } // SM_5   needed for Texture Gather per-channel
        Shader &conservativeDepth() {
            MAX(model, SM_5);
            return T;
        } // SM_5   needed for Conservative Depth (DEPTH_GE/DEPTH_LE)
        Shader &conservativeDepth(Bool on) {
            if (on)
                conservativeDepth();
            return T;
        } // SM_5   needed for Conservative Depth (DEPTH_GE/DEPTH_LE)

        Shader &operator()(C Str &n0, C Str &v0) {
            params.New().set(n0, v0);
            return T;
        }
        Shader &operator()(C Str &n0, C Str &v0, C Str &n1, C Str &v1) {
            params.New().set(n0, v0);
            params.New().set(n1, v1);
            return T;
        }
        Shader &operator()(C Str &n0, C Str &v0, C Str &n1, C Str &v1, C Str &n2, C Str &v2) {
            params.New().set(n0, v0);
            params.New().set(n1, v1);
            params.New().set(n2, v2);
            return T;
        }
        Shader &operator()(C Str &n0, C Str &v0, C Str &n1, C Str &v1, C Str &n2, C Str &v2, C Str &n3, C Str &v3) {
            params.New().set(n0, v0);
            params.New().set(n1, v1);
            params.New().set(n2, v2);
            params.New().set(n3, v3);
            return T;
        }
        Shader &operator()(C Str &n0, C Str &v0, C Str &n1, C Str &v1, C Str &n2, C Str &v2, C Str &n3, C Str &v3, C Str &n4, C Str &v4) {
            params.New().set(n0, v0);
            params.New().set(n1, v1);
            params.New().set(n2, v2);
            params.New().set(n3, v3);
            params.New().set(n4, v4);
            return T;
        }

        Shader &extra(C Str &n0, C Str &v0) {
            extra_params.New().set(n0, v0);
            return T;
        }
        Shader &extra(C Str &n0, C Str &v0, C Str &n1, C Str &v1) {
            extra_params.New().set(n0, v0);
            extra_params.New().set(n1, v1);
            return T;
        }
        Shader &extra(C Str &n0, C Str &v0, C Str &n1, C Str &v1, C Str &n2, C Str &v2) {
            extra_params.New().set(n0, v0);
            extra_params.New().set(n1, v1);
            extra_params.New().set(n2, v2);
            return T;
        }
        Shader &extra(C Str &n0, C Str &v0, C Str &n1, C Str &v1, C Str &n2, C Str &v2, C Str &n3, C Str &v3) {
            extra_params.New().set(n0, v0);
            extra_params.New().set(n1, v1);
            extra_params.New().set(n2, v2);
            extra_params.New().set(n3, v3);
            return T;
        }

        Shader &tesselate(Bool tesselate) {
            if (tesselate) {
                sub[ST_HS].func_name = "HS";
                sub[ST_DS].func_name = "DS";
            }
            return T("TESSELATE", tesselate);
        }

        Shader &position(Int skin, Int color, Int alpha_test, Int test_blend, Int fx, Int tesselate) { return T("SKIN", skin, "COLORS", color, "ALPHA_TEST", alpha_test, "TEST_BLEND", test_blend, "FX", fx).tesselate(tesselate); }

        Shader &deferred(Int skin, Int materials, Int layout, Int bump_mode, Int alpha_test, Int detail, Int macro, Int color, Int mtrl_blend, Int uv_scale, Int heightmap, Int fx, Int tesselate) { return T("SKIN", skin, "MATERIALS", materials, "LAYOUT", layout, "BUMP_MODE", bump_mode)("ALPHA_TEST", alpha_test)("DETAIL", detail, "MACRO", macro)("COLORS", color, "MTRL_BLEND", mtrl_blend, "UV_SCALE", uv_scale, "HEIGHTMAP", heightmap, "FX", fx).tesselate(tesselate); }
        Shader &blendLight(Int skin, Int color, Int layout, Int bump_mode, Int alpha_test, Int alpha, Int reflect, Int emissive_map, Int uv_scale, Int fx, Int per_pixel, Int shadow_maps, Int tesselate = 0) { return T("SKIN", skin, "COLORS", color, "LAYOUT", layout, "BUMP_MODE", bump_mode)("ALPHA_TEST", alpha_test, "ALPHA", alpha)("REFLECT", reflect, "EMISSIVE_MAP", emissive_map)("UV_SCALE", uv_scale, "FX", fx, "PER_PIXEL", per_pixel, "SHADOW_MAPS", shadow_maps).tesselate(tesselate); }
        void finalizeName();
        Bool save(File &f, C ShaderCompiler &compiler) C;
    };

    struct Source {
        Str file_name;
        Mems<Byte> file_data;
#if DX_SHADER_COMPILER
        IDxcBlobEncoding *file_blob = null;
#else
        Ptr file_blob = null;
#endif
        Memc<Shader> shaders;
        SHADER_MODEL model;
        ShaderCompiler *compiler;

        API api() C { return compiler->api; }
        Bool newCompiler() C { return model >= SM_6 || api() != API_DX; }
        Bool load();

        Shader &New(C Str &name = S, C Str8 &vs_func_name = "VS", C Str8 &ps_func_name = "PS");
        Shader &computeNew(C Str &name = S, C Str8 &cs_func_name = "CS");

        Shader &forward(Int skin, Int materials, Int layout, Int bump_mode, Int alpha_test, Int reflect, Int emissive_map, Int detail, Int color, Int mtrl_blend, Int uv_scale, Int heightmap, Int fx, Int per_pixel, Int light_dir, Int light_dir_shd, Int light_dir_shd_num, Int light_point, Int light_point_shd, Int light_linear, Int light_linear_shd, Int light_cone, Int light_cone_shd, Int tesselate) { return New()("SKIN", skin)("MATERIALS", materials, "LAYOUT", layout)("BUMP_MODE", bump_mode)("ALPHA_TEST", alpha_test)("REFLECT", reflect, "EMISSIVE_MAP", emissive_map, "DETAIL", detail)("COLORS", color, "MTRL_BLEND", mtrl_blend, "UV_SCALE", uv_scale, "HEIGHTMAP", heightmap)("FX", fx)("PER_PIXEL", per_pixel)("LIGHT_DIR", light_dir, "LIGHT_DIR_SHD", light_dir_shd, "LIGHT_DIR_SHD_NUM", light_dir_shd_num)("LIGHT_POINT", light_point, "LIGHT_POINT_SHD", light_point_shd)("LIGHT_LINEAR", light_linear, "LIGHT_LINEAR_SHD", light_linear_shd)("LIGHT_CONE", light_cone, "LIGHT_CONE_SHD", light_cone_shd).tesselate(tesselate); }
        void forwardLight(Int skin, Int materials, Int layout, Int bump_mode, Int alpha_test, Int reflect, Int emissive_map, Int detail, Int color, Int mtrl_blend, Int uv_scale, Int heightmap, Int fx, Int per_pixel, Int tesselate) {
            REPD(shd, 2) {
                if (shd)
                    for (Int maps = 2; maps <= 6; maps += 2) // 2, 4, 6 maps
                        forward(skin, materials, layout, bump_mode, alpha_test, reflect, emissive_map, detail, color, mtrl_blend, uv_scale, heightmap, fx, per_pixel, true, shd, maps, false, false, false, false, false, false, tesselate);
                else                                                                                                                                                                                                                    // light dir with shadow maps
                    forward(skin, materials, layout, bump_mode, alpha_test, reflect, emissive_map, detail, color, mtrl_blend, uv_scale, heightmap, fx, per_pixel, true, false, 0, false, false, false, false, false, false, tesselate); // light dir no   shadow
                forward(skin, materials, layout, bump_mode, alpha_test, reflect, emissive_map, detail, color, mtrl_blend, uv_scale, heightmap, fx, per_pixel, false, false, 0, true, shd, false, false, false, false, tesselate);       // light point
                forward(skin, materials, layout, bump_mode, alpha_test, reflect, emissive_map, detail, color, mtrl_blend, uv_scale, heightmap, fx, per_pixel, false, false, 0, false, false, true, shd, false, false, tesselate);       // light linear
                forward(skin, materials, layout, bump_mode, alpha_test, reflect, emissive_map, detail, color, mtrl_blend, uv_scale, heightmap, fx, per_pixel, false, false, 0, false, false, false, false, true, shd, tesselate);       // light cone
            }
            forward(skin, materials, layout, bump_mode, alpha_test, reflect, emissive_map, detail, color, mtrl_blend, uv_scale, heightmap, fx, per_pixel, false, false, 0, false, false, false, false, false, false, tesselate); // no light
        }

        ~Source();
    };
    Str dest, messages;
    SHADER_MODEL model;
    API api;
    SC_FLAG flag;
    Memc<Source> sources;
    Map<Str8, Buffer> buffers;
    Memc<Str8> images, rw_images;
    SyncLock lock;

    void message(C Str &t) { messages.line() += t; }
    Bool error(C Str &t) {
        message(t);
        return false;
    }

    ShaderCompiler &set(C Str &dest, SHADER_MODEL model, API api, SC_FLAG flag);
    Source &New(C Str &file_name);

    Bool compileTry(Threads &threads);
    void compile(Threads &threads);

    ShaderCompiler();
};

#pragma pack(push, 1)
struct ConstantIndex {
    Byte bind_index;
    UShort src_index;

    void set(Int bind_index, Int src_index);
    ConstantIndex(Int bind_index, Int src_index) { set(bind_index, src_index); }
    ConstantIndex() {}
};
struct ShaderIndex {
    Int shader_data_index;
    UShort buffer_bind_index, image_bind_index;

    ShaderIndex() {}
    ShaderIndex(C ShaderCompiler::SubShader &shader);
};
struct ComputeShaderIndex : ShaderIndex {
    UShort rw_image_bind_index;

    ComputeShaderIndex() {}
    ComputeShaderIndex(C ShaderCompiler::SubShader &shader);
};
#pragma pack(pop)

#endif
/******************************************************************************/
