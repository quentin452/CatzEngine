/******************************************************************************/
#include "../../../stdafx.h"

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
Bool _ExportJXL(C Image &image, File &f, Flt quality, Flt compression_level) {
#if SUPPORT_JXL
    C Image *src = &image;
    Image temp;

    JxlPixelFormat format;
    Zero(format);
    format.endianness = JXL_LITTLE_ENDIAN;
    C auto &type_info = src->typeInfo();
    Bool real = (type_info.high_precision),
         alpha = (type_info.a > 0),
         color = (type_info.channels > 1 + alpha);
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

    if (!src->is())
        return false;
    if (src->cube())
        if (temp.fromCube(*src, type))
            src = &temp;
        else
            return false;

    if (!CanDoRawCopy(src->hwType(), type, IgnoreGamma(0, src->hwType(), type)))
        if (src->copy(temp, -1, -1, -1, type, IMAGE_SOFT, 1))
            src = &temp;
        else
            return false;

    if (src->lockRead()) {
        Bool ok = false;
        {
            auto enc = JxlEncoderMake(null);
            auto runner = JxlThreadParallelRunnerMake(null, JxlThreadParallelRunnerDefaultNumWorkerThreads());
            if (JXL_ENC_SUCCESS != JxlEncoderSetParallelRunner(enc.get(), JxlThreadParallelRunner, runner.get()))
                goto error;

            JxlBasicInfo basic_info;
            JxlEncoderInitBasicInfo(&basic_info);
            basic_info.xsize = src->w();
            basic_info.ysize = src->h();
            basic_info.bits_per_sample = (real ? 32 : 8);
            basic_info.exponent_bits_per_sample = (real ? 8 : 0);
            basic_info.num_color_channels = (color ? 3 : 1);
            if (alpha) {
                basic_info.num_extra_channels = 1;
                basic_info.alpha_bits = basic_info.bits_per_sample;
                basic_info.alpha_exponent_bits = basic_info.exponent_bits_per_sample;
            } else {
                basic_info.num_extra_channels = 0;
                basic_info.alpha_bits = 0;
                basic_info.alpha_exponent_bits = 0;
            }
            Bool lossless = (quality >= 1);
            basic_info.uses_original_profile = lossless; // headers say this should be false for lossy, this needs to be true for lossless because 'JxlEncoderSetFrameLossless' will fail
            if (JXL_ENC_SUCCESS != JxlEncoderSetBasicInfo(enc.get(), &basic_info))
                goto error;

            JxlColorEncoding color_encoding;
            JxlColorEncodingSetToSRGB(&color_encoding, !color);
            if (JXL_ENC_SUCCESS != JxlEncoderSetColorEncoding(enc.get(), &color_encoding))
                goto error;

            JxlEncoderFrameSettings *frame_settings = JxlEncoderFrameSettingsCreate(enc.get(), null);
            JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_KEEP_INVISIBLE, 1);
            if (compression_level >= 0)
                JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_EFFORT, Mid(RoundPos(Lerp(1, 9, compression_level)), 1, 9));
            if (quality >= 0) {
                JxlEncoderSetFrameDistance(frame_settings, Lerp(15, 0, Sat(quality)));
                JxlEncoderSetFrameLossless(frame_settings, lossless);
            }

            if (JXL_ENC_SUCCESS != JxlEncoderAddImageFrame(frame_settings, &format, src->data(), src->pitch2()))
                goto error;
            JxlEncoderCloseInput(enc.get());

        again:
            uint8_t temp[65536], *next_out = temp;
            size_t avail_out = SIZE(temp);
            JxlEncoderStatus process_result = JxlEncoderProcessOutput(enc.get(), &next_out, &avail_out);
            if (!f.put(temp, next_out - temp))
                goto error;
            if (JXL_ENC_NEED_MORE_OUTPUT == process_result)
                goto again;
            if (JXL_ENC_SUCCESS == process_result)
                ok = true;
        }

    error:
        src->unlock();
        return ok && f.ok();
    }
#endif
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
