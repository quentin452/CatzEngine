/******************************************************************************/
#include "../../../stdafx.h"

#if SUPPORT_AVIF
#include "../../../../ThirdPartyLibs/begin.h"

#include "../../../../ThirdPartyLibs/AVIF/lib/include/avif/avif.h"

#include "../../../../ThirdPartyLibs/end.h"
#endif

namespace EE {
/******************************************************************************/
Bool _ImportAVIF(Image &image, File &f) {
    Bool ok = false;
#if SUPPORT_AVIF
    const Byte sig[] = {0, 0, 0, ' ', 'f', 't', 'y', 'p', 'a', 'v', 'i', 'f'};
    Byte data[Elms(sig)];
    if (f.getFast(data)) {
        data[3] = ' '; // ignore 4-th byte, because it can be different
        if (EqualMem(sig, data) && f.skip(-SIZEI(sig))) {
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
            if (avifDecoder *decoder = avifDecoderCreate()) {
                decoder->maxThreads = Cpu.threads();
                if (avifDecoderSetIOMemory(decoder, (uint8_t *)data, size) == AVIF_RESULT_OK)
                    if (avifDecoderParse(decoder) == AVIF_RESULT_OK) {
                        avifRGBImage rgb;
                        avifRGBImageSetDefaults(&rgb, decoder->image);
                        if (avifDecoderNextImage(decoder) == AVIF_RESULT_OK) {
                            Bool alpha = decoder->alphaPresent,
                                 hp = (decoder->image->depth > 8);
                            if (image.createSoft(decoder->image->width, decoder->image->height, 1, hp ? (alpha ? IMAGE_F16_4 : IMAGE_F16_3) : (alpha ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8_SRGB))) {
                                rgb.pixels = image.data();
                                rgb.rowBytes = image.pitch();
                                rgb.format = (alpha ? AVIF_RGB_FORMAT_RGBA : AVIF_RGB_FORMAT_RGB);
                                // rgb.isFloat =(decoder->image->depth==16); need to test gamma
                                Flt mul = 1.0f / ((1 << decoder->image->depth) - 1);

                                if (avifImageYUVToRGB(decoder->image, &rgb) == AVIF_RESULT_OK) {
                                    if (hp) // && !rgb.isFloat)
                                        REPD(y, image.h()) {
                                            Byte *dy = image.data() + y * image.pitch();
                                            REPD(x, image.w()) {
                                                Byte *d = dy + x * image.bytePP();
                                                ((Half *)d)[0] = SRGBToLinear(((U16 *)d)[0] * mul);
                                                ((Half *)d)[1] = SRGBToLinear(((U16 *)d)[1] * mul);
                                                ((Half *)d)[2] = SRGBToLinear(((U16 *)d)[2] * mul);
                                                if (alpha)
                                                    ((Half *)d)[3] = (((U16 *)d)[3] * mul);
                                            }
                                        }
                                    ok = true;
                                }
                            }
                        }
                    }
                avifDecoderDestroy(decoder);
            }
        error:;
        }
    }
#endif
    if (!ok)
        image.del();
    return ok;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
