/******************************************************************************/
#include "../../../stdafx.h"

#if SUPPORT_HEIF
#include "../../../../ThirdPartyLibs/Heif/libheif/heif.h"
#include "../../../../ThirdPartyLibs/begin.h"
#include "../../../../ThirdPartyLibs/end.h"
#endif

namespace EE {
/******************************************************************************/
#if SUPPORT_HEIF
static int64_t HEIFGetPos(Ptr userdata) {
    File &f = *(File *)userdata;
    return f.pos();
}
static int HEIFRead(Ptr data, size_t size, Ptr userdata) {
    File &f = *(File *)userdata;
    return !f.getFast(data, size);
}
static int HEIFSeek(int64_t position, Ptr userdata) {
    File &f = *(File *)userdata;
    return !f.pos(position);
}
static heif_reader_grow_status HEIFWaitForFileSize(int64_t target_size, Ptr userdata) {
    File &f = *(File *)userdata;
    return (target_size <= f.size()) ? heif_reader_grow_status_size_reached : heif_reader_grow_status_size_beyond_eof;
}
#endif
/******************************************************************************/
Bool Image::ImportHEIF(File &f) {
    Bool ok = false;
#if SUPPORT_HEIF
    if (heif_context *ctx = heif_context_alloc()) {
        heif_reader reader;
        reader.reader_api_version = 1;
        reader.get_position = HEIFGetPos;
        reader.read = HEIFRead;
        reader.seek = HEIFSeek;
        reader.wait_for_file_size = HEIFWaitForFileSize;
        heif_context_read_from_reader(ctx, &reader, &f, null);

        heif_image_handle *handle = null;
        heif_context_get_primary_image_handle(ctx, &handle);
        if (handle) {
            int w = heif_image_handle_get_width(handle);
            int h = heif_image_handle_get_height(handle);
            if (w >= 0 && h >= 0) {
                Bool alpha = (heif_image_handle_has_alpha_channel(handle) != 0);
                heif_image *image = null;
                heif_decode_image(handle, &image, heif_colorspace_RGB, alpha ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RGB, null);
                if (image) {
                    int stride;
                    if (const uint8_t *src = heif_image_get_plane_readonly(image, heif_channel_interleaved, &stride)) {
                        if (stride >= 0 && createSoftTry(w, h, 1, alpha ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8_SRGB)) {
                            ok = true;
                            Byte *dest = data();
                            if (stride == pitch()) {
                                CopyFast(dest, src, pitch() * h);
                            } else {
                                Int copy = Min(stride, pitch());
                                REP(h) {
                                    CopyFast(dest, src, copy);
                                    dest += pitch();
                                    src += stride;
                                }
                            }
                        }
                    }
                    heif_image_release(image);
                }
            }
            heif_image_handle_release(handle);
        }
        heif_context_free(ctx);
    }
#endif
    if (!ok)
        del();
    return ok;
}
Bool Image::ImportHEIF(C Str &name) {
#if SUPPORT_HEIF
    File f;
    if (f.readTry(name))
        return ImportHEIF(f);
#endif
    del();
    return false;
}
/******************************************************************************/
#if SUPPORT_HEIF
static heif_error HEIFWrite(struct heif_context *ctx, CPtr data, size_t size, Ptr userdata) {
    File &f = *(File *)userdata;
    heif_error error;
    if (f.put(data, size)) {
        error.code = heif_error_Ok;
        error.subcode = heif_suberror_Unspecified;
    } else {
        error.code = heif_error_Encoding_error;
        error.subcode = heif_suberror_Cannot_write_output_data;
    }
    error.message = null;
    return error;
}
#endif
Bool Image::ExportHEIF(File &f, Flt quality) C {
    Bool ok = false;
#if SUPPORT_HEIF
    C Image *src = this;
    Image temp;
    if (src->cube())
        if (temp.fromCube(*src, ImageTypeUncompressed(src->type())))
            src = &temp;
        else
            return false;
    if (src->compressed())
        if (src->copyTry(temp, -1, -1, 1, ImageTypeUncompressed(src->type()), IMAGE_SOFT, 1))
            src = &temp;
        else
            return false;

    if (src->lockRead()) {
        if (heif_context *ctx = heif_context_alloc()) {
            heif_encoder *encoder = null;
            heif_context_get_encoder_for_format(ctx, heif_compression_HEVC, &encoder);
            if (encoder) {
                Int q = RoundPos(quality * 100);
                if (q < 0)
                    q = 100;
                else // default to 100=lossless
                    if (q > 100)
                        q = 100;
                Bool lossless = (q >= 100);

                heif_encoder_set_lossy_quality(encoder, q);
                heif_encoder_set_lossless(encoder, lossless);

                enum TYPE {
                    MONO,
                    RGB,
                    RGBA,
                } type;
                C ImageTypeInfo &type_info = src->typeInfo();
                // if(type_info.channels<=1)type=MONO;else dunno how to configure libheif to make this work
                if (type_info.a)
                    type = RGBA;
                else
                    type = RGB;

                heif_image *image = null;
                heif_image_create(src->w(), src->h(), (type == MONO) ? heif_colorspace_monochrome : heif_colorspace_RGB, (type == MONO) ? heif_chroma_monochrome : (type == RGB) ? heif_chroma_interleaved_RGB
                                                                                                                                                                                 : heif_chroma_interleaved_RGBA,
                                  &image);
                if (image) {
                    if (lossless) {
                        heif_color_profile_nclx nclx;
                        nclx.version = 1;
                        nclx.matrix_coefficients = heif_matrix_coefficients_RGB_GBR;
                        nclx.transfer_characteristics = heif_transfer_characteristic_unspecified;
                        nclx.color_primaries = heif_color_primaries_unspecified;
                        nclx.full_range_flag = true;
                        heif_image_set_nclx_color_profile(image, &nclx);

                        heif_encoder_set_parameter_string(encoder, "chroma", "444");
                    }

                    heif_image_add_plane(image, heif_channel_interleaved, src->w(), src->h(), 8);
                    int stride;
                    if (uint8_t *dest = heif_image_get_plane(image, heif_channel_interleaved, &stride)) {
                        Bool direct_type;
                        switch (type) {
                        case MONO:
                            direct_type = (src->hwType() == IMAGE_R8 || src->hwType() == IMAGE_L8 || src->hwType() == IMAGE_A8 || src->hwType() == IMAGE_I8);
                            break;
                        case RGB:
                            direct_type = (src->hwType() == IMAGE_R8G8B8 || src->hwType() == IMAGE_R8G8B8_SRGB);
                            break;
                        case RGBA:
                            direct_type = (src->hwType() == IMAGE_R8G8B8A8 || src->hwType() == IMAGE_R8G8B8A8_SRGB);
                            break;
                        }
                        if (direct_type) {
                            C Byte *src_data = src->data();
                            if (stride == src->pitch()) {
                                CopyFast(dest, src_data, src->pitch() * src->h());
                            } else {
                                Int copy = Min(stride, src->pitch());
                                REP(src->h()) {
                                    CopyFast(dest, src_data, copy);
                                    dest += stride;
                                    src_data += src->pitch();
                                }
                            }
                        } else {
                            FREPD(y, src->h()) {
                                Byte *out = dest;
                                FREPD(x, src->w())
                                switch (type) {
                                case MONO:
                                    *(Byte *)out = FltToByte(src->pixelF(x, y));
                                    out += SIZE(Byte);
                                    break;
                                case RGB:
                                    *(VecB *)out = src->color(x, y).v3;
                                    out += SIZE(VecB);
                                    break;
                                case RGBA:
                                    *(Color *)out = src->color(x, y);
                                    out += SIZE(Color);
                                    break;
                                }
                                dest += stride;
                            }
                        }

                        if (heif_context_encode_image(ctx, image, encoder, null, null).code == heif_error_Ok) {
                            heif_writer writer;
                            writer.writer_api_version = 1;
                            writer.write = HEIFWrite;
                            if (heif_context_write(ctx, &writer, &f).code == heif_error_Ok)
                                ok = f.ok();
                        }
                    }
                    heif_image_release(image);
                }
                heif_encoder_release(encoder);
            }
            heif_context_free(ctx);
        }
        src->unlock();
    }
#endif
    return ok;
}
/******************************************************************************/
Bool Image::ExportHEIF(C Str &name, Flt quality) C {
#if SUPPORT_HEIF
    File f;
    if (f.writeTry(name)) {
        if (ExportHEIF(f, quality) && f.flush())
            return true;
        f.del();
        FDelFile(name);
    }
#endif
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
