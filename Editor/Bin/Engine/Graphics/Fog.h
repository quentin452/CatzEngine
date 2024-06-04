#pragma once
/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Fog' to set custom global fog.

/******************************************************************************/
struct FogClass {
    Bool draw,      // if draw the fog       ,    true/false   , default=false
        affect_sky; // if fog affects sky    ,    true/false   , default=false
    Flt density;    // fog density           ,       0..1      , default=0.02
    Vec color_l;    // fog color linear gamma, (0,0,0)..(1,1,1), default=colorS(0.5, 0.5, 0.5)

    C Vec &colorL() C { return color_l; }
    void colorL(C Vec &color_l) { T.color_l = color_l; } // get/set Linear Gamma color
    Vec colorS() C;
    void colorS(C Vec &color_s); // get/set sRGB   Gamma color

#if EE_PRIVATE
    void Draw(Bool after_sky);
#if LINEAR_GAMMA
    INLINE C Vec &colorD() C { return colorL(); }
#else
    INLINE Vec colorD() C { return colorS(); }
#endif

    FogClass();
#endif
} extern Fog; // Global Fog Control
/******************************************************************************/
void FogDraw(C OBox &obox, Flt density, C Vec &color_l);       // draw local 'obox' based        fog, with uniform  'density', this can be called only in RM_BLEND rendering mode
void FogDraw(C Ball &ball, Flt density, C Vec &color_l);       // draw local 'ball' based        fog, with uniform  'density', this can be called only in RM_BLEND rendering mode
void HeightFogDraw(C OBox &obox, Flt density, C Vec &color_l); // draw local 'obox' based height fog, with variable 'density', this can be called only in RM_BLEND rendering mode
/******************************************************************************/
