/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
#include "!Set Prec Struct.h"
BUFFER(SMAA)
Flt SMAAThreshold = 0.05;
BUFFER_END

#define SMAA_HLSL_4 1 // TODO: using SMAA_HLSL_4_1 would be faster, however it's only used for predication and 'SMAADepthEdgeDetectionPS' which are not used
#define PointSampler SamplerPoint
#define LinearSampler SamplerLinearClamp

#define SMAA_AREATEX_SELECT(sample) sample.rg
#define SMAA_RT_METRICS RTSize       // can use 'RTSize' instead of 'ImgSize' since there's no scale
#define SMAA_THRESHOLD SMAAThreshold // best noticable on "iloyjp6kr6q56_jzjamo0z6#" /* Vehicles\Cartoon\Tank */
#define SMAA_MAX_SEARCH_STEPS 6
#define SMAA_MAX_SEARCH_STEPS_DIAG 0
#define SMAA_CORNER_ROUNDING 100
#if SMAA_MAX_SEARCH_STEPS_DIAG == 0
#define SMAA_DISABLE_DIAG_DETECTION
#endif
#define SMAA_COLOR_WEIGHT_USE 1                       // enabling slightly increases performance
#define SMAA_COLOR_WEIGHT float3(0.509, 1.000, 0.194) // ClipSet(ColorLumWeight2/ColorLumWeight2.max());

#include "!Set Prec Default.h"
#include "!Set Prec Image.h"
#include "SMAA.h"

void SMAAEdge_VS(VtxInput vtx,
                 NOPERSP out Vec2 uv
                 : UV,
                   NOPERSP out Vec4 offset[3]
                 : OFFSET,
                   NOPERSP out Vec4 vpos
                 : POSITION) {
    vpos = Vec4(vtx.pos2(), Z_BACK, 1); // set Z to be at the end of the viewport, this enables optimizations by processing only foreground pixels (no sky/background)
    uv = vtx.uv();
    SMAAEdgeDetectionVS(uv, offset);
}
VecH2 SMAAEdge_PS(NOPERSP Vec2 uv
                  : UV,
                    NOPERSP Vec4 offset[3]
                  : OFFSET) : TARGET // Input: GAMMA
{
    return SMAAColorEdgeDetectionPS(uv, offset, Img); // use instead of "SMAALumaEdgeDetectionPS" to differentiate between different colors
}

void SMAABlend_VS(VtxInput vtx,
                  NOPERSP out Vec2 uv
                  : UV,
                    NOPERSP out Vec2 pixcoord
                  : PIXCOORD,
                    NOPERSP out Vec4 offset[3]
                  : OFFSET,
                    NOPERSP out Vec4 vpos
                  : POSITION) {
    vpos = Vec4(vtx.pos2(), Z_BACK, 1); // set Z to be at the end of the viewport, this enables optimizations by processing only foreground pixels (no sky/background)
    uv = vtx.uv();
    SMAABlendingWeightCalculationVS(uv, pixcoord, offset);
}
VecH4 SMAABlend_PS(NOPERSP Vec2 uv
                   : UV,
                     NOPERSP Vec2 pixcoord
                   : PIXCOORD,
                     NOPERSP Vec4 offset[3]
                   : OFFSET) : TARGET {
    return SMAABlendingWeightCalculationPS(uv, pixcoord, offset, Img, Img1, Img2, 0);
}

void SMAA_VS(VtxInput vtx,
             NOPERSP out Vec2 uv
             : UV,
               NOPERSP out Vec4 offset
             : OFFSET,
               NOPERSP out Vec4 vpos
             : POSITION) {
    vpos = Vec4(vtx.pos2(), Z_BACK, 1); // set Z to be at the end of the viewport, this enables optimizations by processing only foreground pixels (no sky/background)
    uv = vtx.uv();
    SMAANeighborhoodBlendingVS(uv, offset);
}
VecH4 SMAA_PS(NOPERSP Vec2 uv
              : UV,
                NOPERSP Vec4 offset
              : OFFSET) : TARGET {
    return SMAANeighborhoodBlendingPS(uv, offset, Img, Img1);
}
/******************************************************************************
// MLAA
Copyright (C) 2011 Jorge Jimenez (jorge@iryoku.com)
Copyright (C) 2011 Belen Masia (bmasia@unizar.es)
Copyright (C) 2011 Jose I. Echevarria (joseignacioechevarria@gmail.com)
Copyright (C) 2011 Fernando Navarro (fernandn@microsoft.com)
Copyright (C) 2011 Diego Gutierrez (diegog@unizar.es)
All rights reserved.
/******************************************************************************
// can use 'RTSize' instead of 'ImgSize' since there's no scale

#define MLAA_MAX_SEARCH_STEPS 6
#define MLAA_MAX_DISTANCE     32
#define MLAA_THRESHOLD        0.1

Vec2 MLAAArea(Vec2 distance, Flt e1, Flt e2)
{
   Flt  areaSize=MLAA_MAX_DISTANCE*5;
   Vec2 pixcoord=MLAA_MAX_DISTANCE*Round(4*Vec2(e1, e2))+distance;
   Vec2 texcoord=pixcoord/(areaSize-1);
   return TexLod(Img1, texcoord).rg; // AreaMap
}
void MLAA_VS(VtxInput vtx,
 NOPERSP out Vec2 outTex         :TEXCOORD0,
 NOPERSP out Vec4 outTexOffset[2]:TEXCOORD1,
 NOPERSP out Vec4 vpos           :POSITION )
{
   vpos           =Vec4(vtx.pos2(), Z_BACK, 1); // set Z to be at the end of the viewport, this enables optimizations by processing only foreground pixels (no sky/background)
   outTex         =vtx.uv();
   outTexOffset[0]=RTSize.xyxy*Vec4(-1, 0, 0,-1)+outTex.xyxy;
   outTexOffset[1]=RTSize.xyxy*Vec4( 1, 0, 0, 1)+outTex.xyxy;
}
Vec4 MLAAEdge_PS(NOPERSP Vec2 texcoord :TEXCOORD0,
                 NOPERSP Vec4 offset[2]:TEXCOORD1):TARGET
{
   Flt L      =Dot(TexPoint(Img, texcoord    ).rgb, ColorLumWeight2);
   Flt Lleft  =Dot(TexPoint(Img, offset[0].xy).rgb, ColorLumWeight2);
   Flt Ltop   =Dot(TexPoint(Img, offset[0].zw).rgb, ColorLumWeight2);
   Flt Lright =Dot(TexPoint(Img, offset[1].xy).rgb, ColorLumWeight2);
   Flt Lbottom=Dot(TexPoint(Img, offset[1].zw).rgb, ColorLumWeight2);

   Vec4 delta=Abs(L.xxxx-Vec4(Lleft, Ltop, Lright, Lbottom));
   Vec4 edges=step(Vec4(MLAA_THRESHOLD, MLAA_THRESHOLD, MLAA_THRESHOLD, MLAA_THRESHOLD), delta);

   if(Dot(edges, 1)==0)discard;

   return edges;
}

Flt MLAASearchXLeft (Vec2 texcoord) {Flt i, e=0; for(i=-1.5; i>-2*MLAA_MAX_SEARCH_STEPS; i-=2){e=TexLod(Img, texcoord+RTSize.xy*Vec2(i, 0)).g; FLATTEN if(e<0.9)break;} return Max(i+1.5-2*e, -2*MLAA_MAX_SEARCH_STEPS);}
Flt MLAASearchXRight(Vec2 texcoord) {Flt i, e=0; for(i= 1.5; i< 2*MLAA_MAX_SEARCH_STEPS; i+=2){e=TexLod(Img, texcoord+RTSize.xy*Vec2(i, 0)).g; FLATTEN if(e<0.9)break;} return Min(i-1.5+2*e,  2*MLAA_MAX_SEARCH_STEPS);}
Flt MLAASearchYUp   (Vec2 texcoord) {Flt i, e=0; for(i=-1.5; i>-2*MLAA_MAX_SEARCH_STEPS; i-=2){e=TexLod(Img, texcoord+RTSize.xy*Vec2(0, i)).r; FLATTEN if(e<0.9)break;} return Max(i+1.5-2*e, -2*MLAA_MAX_SEARCH_STEPS);}
Flt MLAASearchYDown (Vec2 texcoord) {Flt i, e=0; for(i= 1.5; i< 2*MLAA_MAX_SEARCH_STEPS; i+=2){e=TexLod(Img, texcoord+RTSize.xy*Vec2(0, i)).r; FLATTEN if(e<0.9)break;} return Min(i-1.5+2*e,  2*MLAA_MAX_SEARCH_STEPS);}

Vec4 MLAABlend_PS(NOPERSP Vec2 texcoord:TEXCOORD):TARGET
{
   Vec4 areas=0;
   Vec2 e=TexPoint(Img, texcoord).rg;

   BRANCH if(e.g) // Edge at north
   {
      Vec2 d     =Vec2(MLAASearchXLeft(texcoord), MLAASearchXRight(texcoord)); // Search distances to the left and to the right
      Vec4 coords=Vec4(d.x, -0.25, d.y+1, -0.25)*RTSize.xyxy+texcoord.xyxy; // Now fetch the crossing edges. Instead of sampling between edgels, we sample at -0.25, to be able to discern what value has each edge
      Flt  e1=TexLod(Img, coords.xy).r,
           e2=TexLod(Img, coords.zw).r;
      areas.rg=MLAAArea(Abs(d), e1, e2); // Ok, we know how this pattern looks like, now it is time for getting the actual area
   }

   BRANCH if(e.r) // Edge at west
   {
      Vec2 d     =Vec2(MLAASearchYUp(texcoord), MLAASearchYDown(texcoord)); // Search distances to the top and to the bottom
      Vec4 coords=Vec4(-0.25, d.x, -0.25, d.y+1)*RTSize.xyxy+texcoord.xyxy; // Now fetch the crossing edges (yet again)
      Flt  e1=TexLod(Img, coords.xy).g,
           e2=TexLod(Img, coords.zw).g;
      areas.ba=MLAAArea(Abs(d), e1, e2); // Get the area for this direction
   }

   return areas;
}
Vec4 MLAA_PS(NOPERSP Vec2 texcoord :TEXCOORD0,
             NOPERSP Vec4 offset[2]:TEXCOORD1):TARGET
{
   // Fetch the blending weights for current pixel:
   Vec4 topLeft=TexPoint(Img1, texcoord);
   Flt  bottom =TexPoint(Img1, offset[1].zw).g,
        right  =TexPoint(Img1, offset[1].xy).a;
   Vec4 a      =Vec4(topLeft.r, bottom, topLeft.b, right),
        w      =a*a*a; // Up to 4 lines can be crossing a pixel (one in each edge). So, we perform a weighted average, where the weight of each line is 'a' cubed, which favors blending and works well in practice.

   Flt sum=Dot(w, 1); // There is some blending weight with a value greater than 0?
   if( sum<1e-5)discard;

   // Add the contributions of the possible 4 lines that can cross this pixel:
#if 1 // use Bilinear Filtering to speedup calculations
   Vec4 color=TexLod(Img, texcoord-Vec2(0, a.r*RTSize.y))*w.r
             +TexLod(Img, texcoord+Vec2(0, a.g*RTSize.y))*w.g
             +TexLod(Img, texcoord-Vec2(a.b*RTSize.x, 0))*w.b
             +TexLod(Img, texcoord+Vec2(a.a*RTSize.x, 0))*w.a;
#else
   Vec4 C      =TexPoint(Img, texcoord    ),
        Cleft  =TexPoint(Img, offset[0].xy),
        Ctop   =TexPoint(Img, offset[0].zw),
        Cright =TexPoint(Img, offset[1].xy),
        Cbottom=TexPoint(Img, offset[1].zw),
        color  =Lerp(C, Ctop   , a.r)*w.r;
        color +=Lerp(C, Cbottom, a.g)*w.g;
        color +=Lerp(C, Cleft  , a.b)*w.b;
        color +=Lerp(C, Cright , a.a)*w.a;
#endif

   return color/sum; // Normalize the resulting color
}
/******************************************************************************/
