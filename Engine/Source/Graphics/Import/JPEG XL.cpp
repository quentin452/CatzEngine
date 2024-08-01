// TODO ADD PROFILE_START AND PROFILE_STOP PROFILERS
/******************************************************************************/
#include "stdafx.h"

#if SUPPORT_JXL
#include "../../../../ThirdPartyLibs/begin.h"

#define JXL_STATIC_DEFINE
#include "../../../../ThirdPartyLibs/JpegXL/lib/lib/include/jxl/decode.h"
#include "../../../../ThirdPartyLibs/JpegXL/lib/lib/include/jxl/decode_cxx.h"
#include "../../../../ThirdPartyLibs/JpegXL/lib/lib/include/jxl/encode.h"
#include "../../../../ThirdPartyLibs/JpegXL/lib/lib/include/jxl/encode_cxx.h"
#include "../../../../ThirdPartyLibs/JpegXL/lib/lib/include/jxl/resizable_parallel_runner.h"
#include "../../../../ThirdPartyLibs/JpegXL/lib/lib/include/jxl/resizable_parallel_runner_cxx.h"
#include "../../../../ThirdPartyLibs/JpegXL/lib/lib/include/jxl/thread_parallel_runner.h"
#include "../../../../ThirdPartyLibs/JpegXL/lib/lib/include/jxl/thread_parallel_runner_cxx.h"

#include "../../../../ThirdPartyLibs/end.h"
#endif

namespace EE {
/******************************************************************************/
#if SUPPORT_JXL
static inline Bool JXLSigOK(const uint8_t *buf, size_t len) {
    switch (JxlSignatureCheck(buf, len)) {
    case JXL_SIG_CODESTREAM:
    case JXL_SIG_CONTAINER:
        return true;
    default:
        return false;
    }
}
#endif
Bool _ImportJXL(Image &image, File &f) {
#if SUPPORT_JXL
    uint8_t sig[12];
    if (f.getFast(sig) && JXLSigOK(sig, SIZE(sig)) && f.skip(-SIZEI(sig))) // based on 'JxlSignatureCheck' source code we need 12 bytes to make a full check
    {
        Memt<Byte> temp;
        Ptr data;
        Int size = Min(INT_MAX, f.left());
        if (f._type == FILE_MEM)
            data = f.memFast();
        else {
            data = temp.setNumDiscard(size).data();
            if (!f.getFast(data, size))
                goto error;
        }
        {
            auto runner = JxlResizableParallelRunnerMake(null);
            auto dec = JxlDecoderMake(null);
            if (JXL_DEC_SUCCESS != JxlDecoderSubscribeEvents(dec.get(), JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FULL_IMAGE))
                goto error;
            if (JXL_DEC_SUCCESS != JxlDecoderSetParallelRunner(dec.get(), JxlResizableParallelRunner, runner.get()))
                goto error;

            JxlBasicInfo info;
            JxlPixelFormat format;
            Zero(format);
            format.endianness = JXL_LITTLE_ENDIAN;

            JxlDecoderSetInput(dec.get(), (uint8_t *)data, size);
            JxlDecoderCloseInput(dec.get());

            for (;;)
                switch (JxlDecoderStatus status = JxlDecoderProcessInput(dec.get())) {
                // case JXL_DEC_ERROR:
                // case JXL_DEC_NEED_MORE_INPUT:
                default:
                    goto error;

                case JXL_DEC_BASIC_INFO: {
                    if (JXL_DEC_SUCCESS != JxlDecoderGetBasicInfo(dec.get(), &info))
                        goto error;
                    Bool real = (info.bits_per_sample > 8),
                         color = (info.num_color_channels > 1), // 'num_color_channels' can be 1 or 3
                        alpha = (info.alpha_bits != 0);
                    IMAGE_TYPE type;
                    if (real) {
                        format.data_type = JXL_TYPE_FLOAT;
                        if (alpha) {
                            type = IMAGE_F32_4_SRGB;
                            format.num_channels = 4;
                        } else {
                            type = IMAGE_F32_3_SRGB;
                            format.num_channels = 3;
                        }
                    } else {
                        format.data_type = JXL_TYPE_UINT8;
                        if (color) {
                            if (alpha) {
                                type = IMAGE_R8G8B8A8_SRGB;
                                format.num_channels = 4;
                            } else {
                                type = IMAGE_R8G8B8_SRGB;
                                format.num_channels = 3;
                            }
                        } else {
                            if (alpha) {
                                type = IMAGE_L8A8_SRGB;
                                format.num_channels = 2;
                            } else {
                                type = IMAGE_L8_SRGB;
                                format.num_channels = 1;
                            }
                        }
                    }

                    image.createSoft(info.xsize, info.ysize, 1, type);
                    JxlResizableParallelRunnerSetThreads(runner.get(), JxlResizableParallelRunnerSuggestThreads(info.xsize, info.ysize));
                } break;

                case JXL_DEC_COLOR_ENCODING: {
                    size_t icc_size;
                    if (JXL_DEC_SUCCESS != JxlDecoderGetICCProfileSize(dec.get(), &format, JXL_COLOR_PROFILE_TARGET_DATA, &icc_size))
                        goto error;
                    Mems<Char8> icc_profile((Int)icc_size);
                    if (JXL_DEC_SUCCESS != JxlDecoderGetColorAsICCProfile(dec.get(), &format, JXL_COLOR_PROFILE_TARGET_DATA, (uint8_t *)icc_profile.data(), icc_profile.elms()))
                        goto error;
                } break;

                    // case JXL_DEC_NEED_PREVIEW_OUT_BUFFER: break;
                    // case JXL_DEC_PREVIEW_IMAGE: break;

                case JXL_DEC_NEED_IMAGE_OUT_BUFFER: {
                    size_t buffer_size;
                    if (JXL_DEC_SUCCESS != JxlDecoderImageOutBufferSize(dec.get(), &format, &buffer_size))
                        goto error;
                    if (buffer_size != image.pitch2())
                        goto error;
                    if (JXL_DEC_SUCCESS != JxlDecoderSetImageOutBuffer(dec.get(), &format, image.data(), image.pitch2()))
                        goto error;
                } break;

                case JXL_DEC_FULL_IMAGE:
                    // info.orientation;
                    return true; // If the image is an animation, more full frames may be decoded.

                case JXL_DEC_SUCCESS:
                    return true; // All decoding successfully finished. It's not required to call JxlDecoderReleaseInput(dec.get()) here since the decoder will be destroyed.
                }
        }
    error:;
    }
#endif
    image.del();
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
