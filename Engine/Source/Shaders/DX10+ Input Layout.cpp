/******************************************************************************

   This Shader is used only to create the most basic Vertex Shader which can be used for Input Layout creation.

/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
void VS(VtxInput vtx, out Vec4 vpos
        : POSITION) {
    vpos = vtx.pos4();
}
void PS() {
}
/******************************************************************************/
