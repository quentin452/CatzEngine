/******************************************************************************/
#include "!Set Prec Struct.h"
BUFFER(Fog)
VecH FogColor;
Flt FogDensity;
BUFFER_END

BUFFER(LocalFog)
VecH LocalFogColor;
Flt LocalFogDensity;
Vec LocalFogInside;
BUFFER_END
#include "!Set Prec Default.h"
/******************************************************************************/
