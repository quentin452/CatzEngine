/******************************************************************************/
#include "stdafx.h"

#if SUPPORT_AVIF
#include "../../../../ThirdPartyLibs/begin.h"

#include "../../../../ThirdPartyLibs/AVIF/lib/include/avif/avif.h"

#include "../../../../ThirdPartyLibs/end.h"
#endif

namespace EE {
/******************************************************************************/
Bool _ExportAVIF(C Image &image, File &f, Flt rgb_quality, Flt alpha_quality, Flt compression_level) {
    Bool ok = false;
#if SUPPORT_AVIF
    C Image *src = &image;
    Image temp;

    C auto &type_info = src->typeInfo();
    Bool hp = (type_info.high_precision),
         alpha = (type_info.a > 0);
    IMAGE_TYPE type = (hp ? (alpha ? IMAGE_F16_4 : IMAGE_F16_3) : (alpha ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8_SRGB));

    if (src->cube())
        if (temp.fromCube(*src, type))
            src = &temp;
        else
            return false;

    if (hp // always copy for 'hp' because there we have to convert values
        || !CanDoRawCopy(src->hwType(), type, IgnoreGamma(0, src->hwType(), type)))
        if (src->copy(temp, -1, -1, -1, type, IMAGE_SOFT, 1))
            src = &temp;
        else
            return false;

    if (hp) {
        Image &img = temp;
        if (!img.lock())
            return false;
        REPD(y, img.h()) {
            Byte *dy = img.data() + y * img.pitch();
            REPD(x, img.w()) {
                Byte *d = dy + x * img.bytePP();
                ((U16 *)d)[0] = FltToU12(LinearToSRGB(((Half *)d)[0])); // 16-bits not supported in encoder
                ((U16 *)d)[1] = FltToU12(LinearToSRGB(((Half *)d)[1]));
                ((U16 *)d)[2] = FltToU12(LinearToSRGB(((Half *)d)[2]));
                if (alpha)
                    ((U16 *)d)[3] = FltToU12(((Half *)d)[3]);
            }
        }
        img.unlock();
    }
    if (!src->lockRead())
        return false;

    avifEncoder *encoder = null;
    avifImage *img = avifImageCreate(src->w(), src->h(), hp ? 12 : 8, AVIF_PIXEL_FORMAT_YUV444); // 16-bits not supported in encoder
    avifRWData output = AVIF_DATA_EMPTY;
    avifRGBImage rgb;
    avifRGBImageSetDefaults(&rgb, img);
    rgb.pixels = ConstCast(src->data());
    rgb.rowBytes = src->pitch();
    rgb.format = (alpha ? AVIF_RGB_FORMAT_RGBA : AVIF_RGB_FORMAT_RGB);
    if (avifImageRGBToYUV(img, &rgb) != AVIF_RESULT_OK)
        goto error;

    encoder = avifEncoderCreate();
    encoder->maxThreads = Cpu.threads();
    if (compression_level >= 0)
        encoder->speed = Mid(RoundPos(Lerp(AVIF_SPEED_FASTEST, AVIF_SPEED_SLOWEST, compression_level)), AVIF_SPEED_SLOWEST, AVIF_SPEED_FASTEST);
    if (rgb_quality >= 0)
        encoder->quality = Mid(RoundPos(Lerp(AVIF_QUALITY_WORST, AVIF_QUALITY_BEST, rgb_quality)), AVIF_QUALITY_WORST, AVIF_QUALITY_BEST);
    if (alpha_quality >= 0)
        encoder->qualityAlpha = Mid(RoundPos(Lerp(AVIF_QUALITY_WORST, AVIF_QUALITY_BEST, alpha_quality)), AVIF_QUALITY_WORST, AVIF_QUALITY_BEST);
    else
        encoder->qualityAlpha = encoder->quality;

    if (avifEncoderAddImage(encoder, img, 1, AVIF_ADD_IMAGE_FLAG_SINGLE) != AVIF_RESULT_OK)
        goto error;
    if (avifEncoderFinish(encoder, &output) != AVIF_RESULT_OK)
        goto error;

    ok = f.put(output.data, (Int)output.size);
error:
    if (img)
        avifImageDestroy(img);
    if (encoder)
        avifEncoderDestroy(encoder);
    avifRWDataFree(&output);

    src->unlock();
#endif
    return ok;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
