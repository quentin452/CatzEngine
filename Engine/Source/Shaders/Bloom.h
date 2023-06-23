/******************************************************************************/
#include "!Set Prec Struct.h"
BUFFER(Bloom)
   VecH4 BloomParams; // x=original, y=scale, z=cut, w=glow/(res*res)
   VecH  Contrast;
BUFFER_END
#include "!Set Prec Default.h"
/******************************************************************************/
Half BloomOriginal() {return BloomParams.x;}
Half BloomScale   () {return BloomParams.y;}
Half BloomGlow    () {return BloomParams.w;}
/******************************************************************************/
VecH BloomColor(VecH color, Half bloom_scale)
{
   Half col_lum=Max(color), lum=col_lum*bloom_scale+BloomParams.z;
   return (lum>0) ? color*(Sqr(lum)/col_lum) : VecH(0, 0, 0);
}
VecH BloomColor(VecH color) {return BloomColor(color, BloomScale());}
/******************************************************************************/
