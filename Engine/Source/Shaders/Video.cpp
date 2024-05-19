/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
VecH4 YUV_PS(NOPERSP Vec2 tex
             : UV) : TARGET {
    /*Half y=TexLod(ImgX , tex).x,
           u=TexLod(ImgX1, tex).x,
           v=TexLod(ImgX2, tex).x;

      Half r=1.1643*(y-0.0625)                   + 1.5958*(v-0.5),
           g=1.1643*(y-0.0625) - 0.39173*(u-0.5) - 0.8129*(v-0.5),
           b=1.1643*(y-0.0625) + 2.017  *(u-0.5)                 ;*/

    Half y = TexLod(ImgX, tex).x * 1.1643 - 0.07276875;
#if UV_MERGED
    VecH2 uv = TexLod(ImgXY, tex).xy - 0.5;
    Half u = uv.x, v = uv.y;
#else
    Half u = TexLod(ImgX1, tex).x - 0.5,
         v = TexLod(ImgX2, tex).x - 0.5;
#endif

    VecH4 col;
    col.r = y + 1.5958 * v;
    col.g = y - 0.39173 * u - 0.8129 * v;
    col.b = y + 2.017 * u;
    col.a = (ALPHA ? TexLod(ImgX3, tex).x * 1.1643 - 0.07276875 : 1); // need to MulAdd because alpha image assumes to come from another YUV video

    if (GAMMA)
        col.rgb = SRGBToLinear(col.rgb);
    return col;
}
/******************************************************************************/
