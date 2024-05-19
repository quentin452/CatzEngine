/******************************************************************************

   TODO: add support for dual motion:
      first find biggest motion as done now,
      then find biggest motion along Perp(first biggest motion), using Dot(motion, perp)

/******************************************************************************/
#include "Motion Blur.h"
#include "!Header.h"
#include "Bloom.h"
#include "Temporal.h"
/******************************************************************************

   Motion Blur is blurred in both ways:
      -from current position to previous position
      -from current position to new      position (new is estimated as opposite of previous)
   This mode is not correct, future is not yet known, however it helps prevent "leaking" when rotating camera around an object,
      where one side doesn't get blurred because it wants to get samples behind the object however they're not avaialble since the object covers them,
      so when blurring in both ways, we can just use samples from the other side

   If this is ever changed then DUAL_DILATE_MOTION could be disabled and just 1 full dilated motion calculated.

/******************************************************************************/
#ifndef VIEW_FULL
#define VIEW_FULL 1
#endif

#ifndef JITTER
#define JITTER 0
#endif

#ifndef RANGE
#define RANGE 1
#endif

#ifndef SAMPLES
#define SAMPLES 1
#endif

#ifndef GATHER
#define GATHER 0
#endif

#ifndef TEMPORAL
#define TEMPORAL 0
#endif

#define EXPOSURE (GLOW == 2)

#define DUAL_MOTION 1 // 1=more precise but slower
#define TEX_CACHE 0   // 1=was much slower in tests, so don't use

#define MAX_BLUR_LENGTH (0.2 / 2) // #MaxMotionBlurLength

// disable PRECISE because for it to work best 'near' would have to be processed along 'base_uv_motion' line, however right now: PRECISE=1 improves moving background around moving object, however it has negative effect of unnatural blurring FPP weapons when rotating camera fast left/right constantly with a key, and with mouse up/down (while weapon rotates slightly based on up/down angle) in that case the weapons get blurred way too much
#define PRECISE 0 // if precisely (separately) calculate samples for far and near (base/center), this is to solve the problem of rotating camera in FPP view, with weapon attached to player/camera. in that case background is rotating, and on the background blur line it encounters an object (weapon) that is in focus. Blur algorithm counts the far samples that move over the base center, and then lerps to the near samples that move over the base center.

#define SHOW_BLUR_PIXELS 0 // show what pixels actually get blurred (they will be set to GREEN for fast blur and RED for slow blur) use only for debugging

#define MAX_UNROLL_RANGE 5 // only up to 7 is supported here because 'TexPointOfs' accepts offsets in -8..7 range, however limit to 5 because compilation on GL is very slow, and loading compiled shaders on DX GeForce 3080 is very slow too

#if 0 // use only for testing for fast compilation
#define MAX_UNROLL_RANGE 0
#define UNROLL LOOP
#endif
#define FAST_UNROLL [unroll] // always unroll because the operation is fast and won't slow down compilation
/******************************************************************************/
VecH2 UVToScreen(VecH2 uv) { return VecH2(uv.x * AspectRatio, uv.y); } // this is only to maintain XY proportions (it does not convert to screen coordinates)
VecH2 ScreenToUV(VecH2 uv) { return VecH2(uv.x / AspectRatio, uv.y); } // this is only to maintain XY proportions (it does not convert to screen coordinates)
Half ScreenLength2(VecH2 uv) { return Length2(UVToScreen(uv)); }
/******************************************************************************/
VecH2 GetMotionCameraOnly(Vec view_pos, Vec2 uv) {
    Vec view_pos_prev = Transform(view_pos, ViewToViewPrev); // view_pos/ViewMatrix*ViewMatrixPrev
    return uv - PosToUV(view_pos_prev);                      // cur-prev #MotionDir
}
void SetVel_VS(VtxInput vtx,
               NOPERSP out Vec2 uv
               : UV,
                 NOPERSP out Vec2 posXY
               : POS_XY,
                 NOPERSP out Vec4 vpos
               : POSITION) {
    uv = vtx.uv();
    posXY = UVToPosXY(uv);
    vpos = vtx.pos4();
}
VecH2 SetVel_PS(NOPERSP Vec2 uv
                : UV,
                  NOPERSP Vec2 posXY
                : POS_XY,
                  NOPERSP PIXEL) : TARGET {
    Vec pos = GetPosPix(pixel.xy, posXY);
    return GetMotionCameraOnly(pos, uv);
}
/******************************************************************************/
void DilateMaxMin(inout VecH4 motion, inout VecH2 length2, VecH2 sample_motion) // XY=biggest, ZW=smallest
{
    Half sample_len2 = ScreenLength2(sample_motion);
    if (sample_len2 > length2.x) {
        length2.x = sample_len2;
        motion.xy = sample_motion;
    } // biggest
    if (sample_len2 < length2.y) {
        length2.y = sample_len2;
        motion.zw = sample_motion;
    } // smallest
}
void DilateMax(inout VecH2 max_motion, inout Half max_length2, VecH2 sample_motion) {
    Half sample_len2 = ScreenLength2(sample_motion);
    if (sample_len2 > max_length2) {
        max_length2 = sample_len2;
        max_motion = sample_motion;
    }
}
void DilateMin(inout VecH2 min_motion, inout Half min_length2, VecH2 sample_motion) {
    Half sample_len2 = ScreenLength2(sample_motion);
    if (sample_len2 < min_length2) {
        min_length2 = sample_len2;
        min_motion = sample_motion;
    }
}
void DilateSecondary(inout VecH4 motion, inout Half max_dist2, VecH2 sample_motion) {
    Half sample_dist2 = ScreenLength2(sample_motion - motion.xy); // distance of sample_motion to biggest motion
    if (sample_dist2 > max_dist2) {
        max_dist2 = sample_dist2;
        motion.zw = sample_motion;
    }
}
/******************************************************************************/
void ToBlurMotion(inout VecH2 motion) {
    motion *= MotionScale_2;
    // limit max length - this prevents stretching objects to distances blur can't handle anyway, making only certain samples of it visible but not all, thus reducing quality
    Half screen_length2 = ScreenLength2(motion);
    if (screen_length2 > Sqr(MAX_BLUR_LENGTH))
        motion *= MAX_BLUR_LENGTH / Sqrt(screen_length2);
}
void ToBlurMotion(inout VecH2 motion, Half screen_length2) {
    motion *= MotionScale_2;
    screen_length2 *= Sqr(MotionScale_2); // since we've scaled 'motion' above, then we have to scale 'screen_length2' too
    // limit max length - this prevents stretching objects to distances blur can't handle anyway, making only certain samples of it visible but not all, thus reducing quality
    if (screen_length2 > Sqr(MAX_BLUR_LENGTH))
        motion *= MAX_BLUR_LENGTH / Sqrt(screen_length2);
}
/******************************************************************************

   Convert finds the Max and Min Motions (determined by their screen length, screen = UV corrected by aspect ratio)
       Input =         UV Motion
      Output = Max Min UV Motion

/******************************************************************************/
VecH4 Convert_PS(NOPERSP Vec2 uv
                 : UV) : TARGET {
    const Half min_pixel_motion = 0.5;                            // 1/2 of pixel (this value works well for TAA+TSR when rotating camera around trees they get refreshed sensibly and not too blurry)
    const Half small_length2 = Sqr(ImgSize.y * min_pixel_motion); // set to 'min_pixel_motion' to make sure we will ignore small motions and keep 0 #DilatedMotionZero, normally for motion blur we should do "/MotionScale_2" because later there's "*MotionScale_2", however we need to preserve small values for Temporal shader ignoring 'MotionScale_2' #DilatedMotionZero)
#if DUAL_MOTION
    const Int ofs = (RANGE - 1) / 2, min = 0 - ofs, max = RANGE - ofs; // correctness can be verified with this code: "Int RANGE=1,2,4,8; const Int ofs=(RANGE-1)/2, min=0-ofs, max=RANGE-ofs; Str s; for(Int x=min; x<max; x+=2)s.space()+=x; Exit(s);"
#if TEX_CACHE
    VecH2 tex_cache[(max - min + 1) / 2][(max - min + 1) / 2];
#endif

    // find max
    Half max_length2 = (min_pixel_motion > 0) ? small_length2 : -1; // -1 to always update
    VecH4 motion;
    if (min_pixel_motion > 0)
        motion.xy = 0;
#if RANGE <= (GL ? 16 : 256) // for GL limit to 16 because compilation is very slow
    UNROLL for (Int y = min; y < max; y += 2)
        UNROLL for (Int x = min; x < max; x += 2)
#else
    LOOP for (Int y = min; y < max; y += 2)
        LOOP for (Int x = min; x < max; x += 2)
#endif
    {
        VecH2 tex = TexLod(ImgXY, UVInView(uv + Vec2(x, y) * ImgSize.xy, VIEW_FULL));
#if TEX_CACHE
        tex_cache[uint(y - min) / 2][uint(x - min) / 2] = tex;
#endif
        DilateMax(motion.xy, max_length2, tex);
    }

    // find secondary
    Half dist2 = -1;         // -1 to always update
#if RANGE <= (GL ? 16 : 256) // for GL limit to 16 because compilation is very slow
    UNROLL for (Int y = min; y < max; y += 2)
        UNROLL for (Int x = min; x < max; x += 2)
#else
    LOOP for (Int y = min; y < max; y += 2)
        LOOP for (Int x = min; x < max; x += 2)
#endif
    {
#if TEX_CACHE
        VecH2 tex = tex_cache[uint(y - min) / 2][uint(x - min) / 2];
#else
        VecH2 tex = TexLod(ImgXY, UVInView(uv + Vec2(x, y) * ImgSize.xy, VIEW_FULL));
#endif
        DilateSecondary(motion, dist2, tex);
    }

    Half sec_length2 = ScreenLength2(motion.zw);
    if ((min_pixel_motion > 0) && sec_length2 < small_length2)
        motion.zw = 0; // #DilatedMotionZero

        /* Can't be done here because full motion is needed for Temporal shader #BlurMotion
        ToBlurMotion(motion.xy, max_length2);
        ToBlurMotion(motion.zw, sec_length2); */
#else
    VecH2 length2 = VecH2((min_pixel_motion > 0) ? small_length2 : -1, HALF_MAX); // x=biggest, y=smallest, initially set smallest to HALF_MAX so it always gets updated
    VecH4 motion;
    if (min_pixel_motion > 0)
        motion.xy = 0; // XY=biggest, ZW=smallest
#if 0                        // process samples individually
   // for RANGE=1 (no scale  ) uv should remain unmodified              , because it's already at the center of 1x1 texel
   // for RANGE>1 (downsample) uv should be moved to the center of texel, because it's         at the center of 2x2 texels
   if(RANGE>1)uv+=ImgSize.xy*0.5;
   const Int ofs=RANGE/2, min=0-ofs, max=RANGE-ofs;
   UNROLL for(Int y=min; y<max; y++)
   UNROLL for(Int x=min; x<max; x++)
      DilateMaxMin(motion, length2, TexPoint(ImgXY, UVInView(uv+Vec2(x, y)*ImgSize.xy, VIEW_FULL)));
#else                        // process samples in 2x2 blocks using linear filtering
    // for RANGE=1 (no scale          ) offset should be 0, because uv is already at the center of 1x1 texel
    // for RANGE=2 (2x downsample, 2x2) offset should be 0, because uv is already at the center of 2x2 texels (linear filtering is used)
    // for RANGE=4 (4x downsample, 4x4) offset should be 1, because uv is already at the center of 4x4 texels, however we want to process (2x2 2x2) so have to position at the center of top left 2x2
    // for RANGE=8 (8x downsample, 8x8) offset should be 3                                                                                   (2x2 2x2)
    const Int ofs = (RANGE - 1) / 2, min = 0 - ofs, max = RANGE - ofs; // correctness can be verified with this code: "Int RANGE=1,2,4,8; const Int ofs=(RANGE-1)/2, min=0-ofs, max=RANGE-ofs; Str s; for(Int x=min; x<max; x+=2)s.space()+=x; Exit(s);"
#if RANGE <= (GL ? 16 : 256) // for GL limit to 16 because compilation is very slow
    UNROLL for (Int y = min; y < max; y += 2)
        UNROLL for (Int x = min; x < max; x += 2)
#else
    LOOP for (Int y = min; y < max; y += 2)
        LOOP for (Int x = min; x < max; x += 2)
#endif
            DilateMaxMin(motion, length2, TexLod(ImgXY, UVInView(uv + Vec2(x, y) * ImgSize.xy, VIEW_FULL)));
#endif
    if ((min_pixel_motion > 0) && length2.y < small_length2)
        motion.zw = 0; // #DilatedMotionZero

        /* Can't be done here because full motion is needed for Temporal shader #BlurMotion
        ToBlurMotion(motion.xy, length2.x);
        ToBlurMotion(motion.zw, length2.y); */
#endif
    return motion;
}
/******************************************************************************/
#if 0
#define THREAD_SIZE_X RANGE
#define THREAD_SIZE_Y RANGE
#define THREAD_SIZE (THREAD_SIZE_X * THREAD_SIZE_Y)

  groupshared VecH4 SharedMotion [THREAD_SIZE];
//groupshared VecH2 SharedLength2[THREAD_SIZE];

void Process(uint i, uint j)
{
   if(SharedMotion[i].x<SharedMotion[j].x)SharedMotion[i].xy=SharedMotion[j].xy; // XY=biggest
   if(SharedMotion[i].z>SharedMotion[j].z)SharedMotion[i].zw=SharedMotion[j].zw; // ZW=smallest

/* if(ScreenLength2(SharedMotion[i].xy)<ScreenLength2(SharedMotion[j].xy))SharedMotion[i].xy=SharedMotion[j].xy;
   if(ScreenLength2(SharedMotion[i].zw)>ScreenLength2(SharedMotion[j].zw))SharedMotion[i].zw=SharedMotion[j].zw;

   VecH4(ScreenLength2(a.xy)>ScreenLength2(b.xy) ? a.xy : b.xy,
         ScreenLength2(a.zw)<ScreenLength2(b.zw) ? a.zw : b.zw);

   if(SharedLength2[i].x<SharedLength2[j].x){SharedLength2[i].x=SharedLength2[j].x; SharedMotion[i].xy=SharedMotion[j].xy;}
   if(SharedLength2[i].y>SharedLength2[j].y){SharedLength2[i].y=SharedLength2[j].y; SharedMotion[i].zw=SharedMotion[j].zw;} */
}
[numthreads(THREAD_SIZE_X, THREAD_SIZE_Y, 1)]
void Convert_CS
(
   VecU  GroupPos  :SV_GroupID,
   VecU GlobalPos  :SV_DispatchThreadID,
   uint  LocalIndex:SV_GroupIndex
)
{
   needs to be updated
 //VecU2 PixelPos=GlobalPos+ViewportMin; bool inside=all(PixelPos<ViewportMax);
   VecH4 motion=ImgXY[GlobalPos.xy].xyxy; // XY=biggest, ZW=smallest

#if THREAD_SIZE > 1
 //SharedMotion [LocalIndex]=motion;
 //SharedLength2[LocalIndex]=length2;
   SharedMotion [LocalIndex]=ToLen2Angle1Fast(UVToScreen(motion.xy)).xyxy;
   GroupMemoryBarrierWithGroupSync();

   if(THREAD_SIZE>512){if(LocalIndex<512)Process(LocalIndex, LocalIndex+512); GroupMemoryBarrierWithGroupSync();}
   if(THREAD_SIZE>256){if(LocalIndex<256)Process(LocalIndex, LocalIndex+256); GroupMemoryBarrierWithGroupSync();}
   if(THREAD_SIZE>128){if(LocalIndex<128)Process(LocalIndex, LocalIndex+128); GroupMemoryBarrierWithGroupSync();}
   if(THREAD_SIZE> 64){if(LocalIndex< 64)Process(LocalIndex, LocalIndex+ 64); GroupMemoryBarrierWithGroupSync();}
   // no sync needed
   if(THREAD_SIZE> 32){if(LocalIndex< 32)Process(LocalIndex, LocalIndex+ 32);}
   if(THREAD_SIZE> 16){if(LocalIndex< 16)Process(LocalIndex, LocalIndex+ 16);}
   if(THREAD_SIZE>  8){if(LocalIndex<  8)Process(LocalIndex, LocalIndex+  8);}
   if(THREAD_SIZE>  4){if(LocalIndex<  4)Process(LocalIndex, LocalIndex+  4);}
   if(THREAD_SIZE>  2){if(LocalIndex<  2)Process(LocalIndex, LocalIndex+  2);}
   if(THREAD_SIZE>  1){if(LocalIndex<  1)Process(LocalIndex, LocalIndex+  1);}

   if(LocalIndex==0)
   {
    //motion =SharedMotion [0];
    //length2=SharedLength2[0];
    //motion =VecH4(ScreenToUV(FromLen2Angle1Fast(SharedMotion[0].xy)), ScreenToUV(FromLen2Angle1Fast(SharedMotion[0].zw)));
    //length2=VecH2(ScreenLength2(motion.xy), ScreenLength2(motion.zw));

            motion =VecH4(FromLen2Angle1Fast(SharedMotion[0].xy), FromLen2Angle1Fast(SharedMotion[0].zw));
      VecH2 length2=VecH2(Length2(motion.xy), Length2(motion.zw));
            motion =VecH4(ScreenToUV(motion.xy), ScreenToUV(motion.zw));
#else
      VecH2 length2=ScreenLength2(motion.xy);
#endif

      // limit max length - this prevents stretching objects to distances the blur can't handle anyway, making only certain samples of it visible but not all
      if(length2.x>Sqr(MAX_BLUR_LENGTH))motion.xy*=MAX_BLUR_LENGTH/Sqrt(length2.x);
    //if(length2.y>Sqr(MAX_BLUR_LENGTH))motion.zw*=MAX_BLUR_LENGTH/Sqrt(length2.y); don't have to scale Min motion, because it's only used to detect fast/simple blur

      if(length2.x<ScreenLength2(ImgSize.xy)*Sqr(min_motion))motion=0; // motions less than 0.5 pixel size force to 0 (ignore this code because this is done faster by just setting initial value of 'length2.x')

      RWImg[GroupPos.xy]=motion;

#if THREAD_SIZE > 1
   }
#endif
}
#endif
/******************************************************************************

    Input =         Max Min UV Motion
   Output = Dilated Max Min UV Motion

/******************************************************************************/
struct PixelMotion {
    VecH2 dir, // direction
        perp;  // direction perpendicular
    Half ext,  // size to check along directions
        len;   // size to check along directions
#if DUAL_DILATE_MOTION
    Half blur_len; // size to check along directions
#endif

    void set(VecH2 uv_motion, VecH2 uv_to_pixel) {
        dir = uv_motion * uv_to_pixel; // convert UV motion to pixel motion
        len = Length(dir);
        if (CanDiv(len))
            dir /= len;                      // normalize, NaN
        perp = Perp(dir);                    // perpendicular to movement
        ext = (Abs(dir.x) + Abs(dir.y)) * 2; // make smallest movements dilate neighbors, increase extent because later we use linear filtering, so because of blending, movement might get minimized, especially visible in cases such as moving object to the right (motion x=1, y=0) row up and down could have velocity at 0 and linear filtering could reduce motion at the object border (2 value was the smallest that fixed most problems)

#if DUAL_DILATE_MOTION
        blur_len = ext + len * MotionScale_2;
#endif
        len = ext + len;
    }
    VecH2 pos(VecH2 pos) // return pos along motion
    {
        return VecH2(Dot(pos, dir),   // position along motion dir
                     Dot(pos, perp)); // position along motion perp
    }
#if !DUAL_DILATE_MOTION
    Bool reaches(VecH2 pos) {
        pos = Abs(pos);
        return pos.x < len && pos.y < ext;
    } // reaches both sides
#else
    Bool reachesFull(VecH2 pos) { return pos.x > -ext && pos.x < len && Abs(pos.y) < ext; } // reaches front side #MotionDir
    Bool reachesBlur(VecH2 pos) { return Abs(pos.x) < blur_len && Abs(pos.y) < ext; }       // reaches both  sides
#endif
};
/******************************************************************************/
#if !DUAL_MOTION
#if !DUAL_DILATE_MOTION
void DilateMaxMin(inout VecH4 max_min_motion, inout VecH2 length2, VecH4 sample_motion, VecH2 pixel_delta, VecH2 uv_to_pixel, PixelMotion base_pixel_motion) // XY=biggest, ZW=smallest
{
    // if(all(Abs(sample_motion.xy)>=Abs(uv_delta))) this check slows down so don't use, also it would need some EPS +eps (ImgSize/RTSize *1 *0.99 *0.5) etc.
    {
        PixelMotion sample_pixel_motion;
        sample_pixel_motion.set(sample_motion.xy, uv_to_pixel);
        VecH2 base_pos = sample_pixel_motion.pos(pixel_delta); // base   position along sample motion
        VecH2 sample_pos = base_pixel_motion.pos(pixel_delta); // sample position along base   motion
        Bool sample_reaches_base = sample_pixel_motion.reaches(base_pos);
        Bool base_reaches_sample = base_pixel_motion.reaches(sample_pos);
        if (sample_reaches_base)
            DilateMax(max_min_motion.xy, length2.x, sample_motion.xy); // if sample reaches base                        -  biggest
        if (sample_reaches_base || base_reaches_sample)
            DilateMin(max_min_motion.zw, length2.y, sample_motion.zw); // if sample reaches base or base reaches sample - smallest
    }
}
#else
void DilateMaxMin(inout VecH4 max_min_full_motion, inout VecH2 full_length2,
                  inout VecH4 max_min_blur_motion, inout VecH2 blur_length2, VecH4 sample_motion, VecH2 pixel_delta, VecH2 uv_to_pixel, PixelMotion base_pixel_motion) // XY=biggest, ZW=smallest
{
    // if(all(Abs(sample_motion.xy)>=Abs(uv_delta))) this check slows down so don't use, also it would need some EPS +eps (ImgSize/RTSize *1 *0.99 *0.5) etc.
    {
        PixelMotion sample_pixel_motion;
        sample_pixel_motion.set(sample_motion.xy, uv_to_pixel);
        PixelMotion sample_pixel_motion1;
        sample_pixel_motion1.set(sample_motion.zw, uv_to_pixel);
        VecH2 base_pos = sample_pixel_motion.pos(pixel_delta);   // base   position along sample  motion
        VecH2 base_pos1 = sample_pixel_motion1.pos(pixel_delta); // base   position along sample1 motion
        VecH2 sample_pos = base_pixel_motion.pos(pixel_delta);   // sample position along base    motion
        Bool sample_full_reaches_base = sample_pixel_motion.reachesFull(base_pos);
        Bool sample_full_reaches_base1 = sample_pixel_motion1.reachesFull(base_pos1);
        Bool sample_blur_reaches_base = sample_pixel_motion.reachesBlur(base_pos);
        Bool base_blur_reaches_sample = base_pixel_motion.reachesBlur(sample_pos);

        if (sample_full_reaches_base)
            DilateMax(max_min_full_motion.xy, full_length2.x, sample_motion.xy); // if sample reaches base                        -  biggest
        if (sample_full_reaches_base1)
            DilateMin(max_min_full_motion.zw, full_length2.y, sample_motion.zw); // if sample reaches base                        - smallest
        if (sample_blur_reaches_base)
            DilateMax(max_min_blur_motion.xy, blur_length2.x, sample_motion.xy); // if sample reaches base                        -  biggest
        if (sample_blur_reaches_base || base_blur_reaches_sample)
            DilateMin(max_min_blur_motion.zw, blur_length2.y, sample_motion.zw); // if sample reaches base or base reaches sample - smallest, process this for blur because we need to know if along the blur dir there are any different motions in order to use slow/precise motion blur path
    }
}
#endif
/******************************************************************************/
#else
/******************************************************************************/
#if !DUAL_DILATE_MOTION
void DilateMax(inout VecH2 max_motion, inout Half max_length2, VecH4 sample_motion, out bool2 sample_reaches_base, VecH2 pixel_delta, VecH2 uv_to_pixel) {
    PixelMotion sample_pixel_motion;
    // if(all(Abs(sample_motion.xy)>=Abs(uv_delta))) this check slows down so don't use, also it would need some EPS +eps (ImgSize/RTSize *1 *0.99 *0.5) etc.
    {
        sample_pixel_motion.set(sample_motion.xy, uv_to_pixel);
        VecH2 base_pos = sample_pixel_motion.pos(pixel_delta); // base position along sample motion
        if (sample_reaches_base.x = sample_pixel_motion.reaches(base_pos))
            DilateMax(max_motion, max_length2, sample_motion.xy); // if sample reaches base
    }
    // if(all(Abs(sample_motion.zw)>=Abs(uv_delta))) this check slows down so don't use, also it would need some EPS +eps (ImgSize/RTSize *1 *0.99 *0.5) etc.
    {
        sample_pixel_motion.set(sample_motion.zw, uv_to_pixel);
        VecH2 base_pos = sample_pixel_motion.pos(pixel_delta); // base position along sample motion
        if (sample_reaches_base.y = sample_pixel_motion.reaches(base_pos))
            DilateMax(max_motion, max_length2, sample_motion.zw); // if sample reaches base
    }
}
void DilateSecondary(inout VecH4 motion, inout Half max_dist2, VecH4 sample_motion, bool2 sample_reaches_base, VecH2 pixel_delta, VecH2 uv_to_pixel, PixelMotion base_pixel_motion) // XY=biggest, ZW=secondary
{
    VecH2 sample_pos = base_pixel_motion.pos(pixel_delta); // sample position along base motion
    Bool base_reaches_sample = base_pixel_motion.reaches(sample_pos);
    if (base_reaches_sample || sample_reaches_base.x)
        DilateSecondary(motion, max_dist2, sample_motion.xy); // if base reaches sample, or sample.xy reaches base
    if (base_reaches_sample || sample_reaches_base.y)
        DilateSecondary(motion, max_dist2, sample_motion.zw); // if base reaches sample, or sample.zw reaches base
}
#else
void DilateMax(inout VecH2 max_full_motion, inout Half max_full_length2,
               inout VecH2 max_blur_motion, inout Half max_blur_length2, VecH4 sample_motion, out bool4 sample_reaches_base, VecH2 pixel_delta, VecH2 uv_to_pixel) {
    PixelMotion sample_pixel_motion;
    // if(all(Abs(sample_motion.xy)>=Abs(uv_delta))) this check slows down so don't use, also it would need some EPS +eps (ImgSize/RTSize *1 *0.99 *0.5) etc.
    {
        sample_pixel_motion.set(sample_motion.xy, uv_to_pixel);
        VecH2 base_pos = sample_pixel_motion.pos(pixel_delta); // base position along sample motion
        if (sample_reaches_base.x = sample_pixel_motion.reachesFull(base_pos))
            DilateMax(max_full_motion, max_full_length2, sample_motion.xy); // if sample.xy full reaches base
        if (sample_reaches_base.y = sample_pixel_motion.reachesBlur(base_pos))
            DilateMax(max_blur_motion, max_blur_length2, sample_motion.xy); // if sample.xy blur reaches base
    }
    // if(all(Abs(sample_motion.zw)>=Abs(uv_delta))) this check slows down so don't use, also it would need some EPS +eps (ImgSize/RTSize *1 *0.99 *0.5) etc.
    {
        sample_pixel_motion.set(sample_motion.zw, uv_to_pixel);
        VecH2 base_pos = sample_pixel_motion.pos(pixel_delta); // base position along sample motion
        if (sample_reaches_base.z = sample_pixel_motion.reachesFull(base_pos))
            DilateMax(max_full_motion, max_full_length2, sample_motion.zw); // if sample.zw full reaches base
        if (sample_reaches_base.w = sample_pixel_motion.reachesBlur(base_pos))
            DilateMax(max_blur_motion, max_blur_length2, sample_motion.zw); // if sample.zw blur reaches base
    }
}
void DilateSecondary(inout VecH4 full_motion, inout Half max_full_dist2,
                     inout VecH4 blur_motion, inout Half max_blur_dist2, VecH4 sample_motion, bool4 sample_reaches_base, VecH2 pixel_delta, VecH2 uv_to_pixel, PixelMotion base_pixel_blur_motion) // XY=biggest, ZW=secondary
{
    VecH2 sample_pos = base_pixel_blur_motion.pos(pixel_delta); // sample position along base motion
    Bool base_blur_reaches_sample = base_pixel_blur_motion.reachesBlur(sample_pos);
    if (base_blur_reaches_sample || sample_reaches_base.y)
        DilateSecondary(blur_motion, max_blur_dist2, sample_motion.xy); // if base blur reaches sample, or sample.xy blur reaches base
    if (base_blur_reaches_sample || sample_reaches_base.w)
        DilateSecondary(blur_motion, max_blur_dist2, sample_motion.zw); // if base blur reaches sample, or sample.zw blur reaches base
    if (sample_reaches_base.x)
        DilateSecondary(full_motion, max_full_dist2, sample_motion.xy); // if                              sample.xy full reaches base
    if (sample_reaches_base.z)
        DilateSecondary(full_motion, max_full_dist2, sample_motion.zw); // if                              sample.zw full reaches base
}
#endif
/******************************************************************************/
#endif
/******************************************************************************/
// can use 'RTSize' instead of 'ImgSize' since there's no scale
// Dilate doesn't use UV clamping, instead border around viewport is cleared
VecH4 Dilate_PS(
    NOPERSP Vec2 uv
    : UV,
      NOPERSP PIXEL
#if DUAL_DILATE_MOTION
      ,
      out VecH4 blur_motion
    : TARGET1
#endif
    ) : TARGET {
    const VecH2 uv_to_pixel = RTSize.zw;
    VecH4 motion = Img[pixel.xy];
#if DUAL_DILATE_MOTION
    blur_motion = motion;
#endif

#if DUAL_MOTION
#if TEX_CACHE
    VecH4 tex_cache[RANGE * 2 + 1][RANGE * 2 + 1];
#endif
#if !DUAL_DILATE_MOTION
    bool2 sample_reaches_base[RANGE * 2 + 1][RANGE * 2 + 1]; // X=using sample_motion.xy, Y=using sample_motion.zw
#else
    bool4 sample_reaches_base[RANGE * 2 + 1][RANGE * 2 + 1];
#endif

    // find new max
#if RANGE <= MAX_UNROLL_RANGE
    VecH2 motion_xy = motion.xy; // keep copy of 'motion.xy' because it might get overwritten
    Half max_length2 = ScreenLength2(motion.xy);
#if DUAL_DILATE_MOTION
    Half blur_max_length2 = max_length2;
#endif
    UNROLL for (Int y = -RANGE; y <= RANGE; y++)
        UNROLL for (Int x = -RANGE; x <= RANGE; x++) if (x || y) {
        VecH4 tex = TexPointOfs(Img, uv, VecI2(x, y)); // need UV clamp, so can't use Img[]
#if TEX_CACHE
        tex_cache[y + RANGE][x + RANGE] = tex;
#endif
#if !DUAL_DILATE_MOTION
        DilateMax(motion.xy, max_length2, tex, sample_reaches_base[y + RANGE][x + RANGE], VecH2(x, y), uv_to_pixel);
#else
        DilateMax(motion.xy, max_length2, blur_motion.xy, blur_max_length2, tex, sample_reaches_base[y + RANGE][x + RANGE], VecH2(x, y), uv_to_pixel);
#endif
    }

    // secondary
    Half sec_dist2 = ScreenLength2(motion.zw - motion.xy);
    DilateSecondary(motion, sec_dist2, motion_xy); // calculate from existing
#if DUAL_DILATE_MOTION
    Half blur_sec_dist2 = ScreenLength2(blur_motion.zw - blur_motion.xy);
    DilateSecondary(blur_motion, blur_sec_dist2, motion_xy); // calculate from existing
#endif
#else
    Half max_length2 = 0; // here just set 0 because we will process all samples, including (0,0) that's already set to 'motion'
#if DUAL_DILATE_MOTION
    Half blur_max_length2 = 0;
#endif
    LOOP for (Int y = -RANGE; y <= RANGE; y++)
        LOOP for (Int x = -RANGE; x <= RANGE; x++) // if(x || y) is ignored for LOOP to avoid extra overhead, instead codes were modified to need (0,0) samples too
    {
        VecH4 tex = TexPoint(Img, uv + Vec2(x, y) * RTSize.xy); // need UV clamp, so can't use Img[]
#if TEX_CACHE
        tex_cache[y + RANGE][x + RANGE] = tex;
#endif
#if !DUAL_DILATE_MOTION
        DilateMax(motion.xy, max_length2, tex, sample_reaches_base[y + RANGE][x + RANGE], VecH2(x, y), uv_to_pixel);
#else
        DilateMax(motion.xy, max_length2, blur_motion.xy, blur_max_length2, tex, sample_reaches_base[y + RANGE][x + RANGE], VecH2(x, y), uv_to_pixel);
#endif
    }
#endif

    // secondary
#if !DUAL_DILATE_MOTION
    PixelMotion base_pixel_motion;
    base_pixel_motion.set(motion.xy, uv_to_pixel);
#else
    PixelMotion base_pixel_blur_motion;
    base_pixel_blur_motion.set(blur_motion.xy, uv_to_pixel);
#endif

#if RANGE <= MAX_UNROLL_RANGE
    UNROLL for (Int y = -RANGE; y <= RANGE; y++)
        UNROLL for (Int x = -RANGE; x <= RANGE; x++) if (x || y) {
#if TEX_CACHE
        VecH4 tex = tex_cache[y + RANGE][x + RANGE];
#else
        VecH4 tex = TexPointOfs(Img, uv, VecI2(x, y)); // need UV clamp, so can't use Img[]
#endif
#if !DUAL_DILATE_MOTION
        DilateSecondary(motion, sec_dist2, tex, sample_reaches_base[y + RANGE][x + RANGE], VecH2(x, y), uv_to_pixel, base_pixel_motion);
#else
        DilateSecondary(motion, sec_dist2, blur_motion, blur_sec_dist2, tex, sample_reaches_base[y + RANGE][x + RANGE], VecH2(x, y), uv_to_pixel, base_pixel_blur_motion);
#endif
    }
#else
    Half sec_dist2 = 0; // here just set 0 because we will process all samples, including (0,0) that's already set to 'motion'
#if DUAL_DILATE_MOTION
    Half blur_sec_dist2 = 0;
#endif
    LOOP for (Int y = -RANGE; y <= RANGE; y++)
        LOOP for (Int x = -RANGE; x <= RANGE; x++) // if(x || y) is ignored for LOOP to avoid extra overhead, instead codes were modified to need (0,0) samples too
    {
#if TEX_CACHE
        VecH4 tex = tex_cache[y + RANGE][x + RANGE];
#else
        VecH4 tex = TexPoint(Img, uv + Vec2(x, y) * RTSize.xy); // need UV clamp, so can't use Img[]
#endif
#if !DUAL_DILATE_MOTION
        DilateSecondary(motion, sec_dist2, tex, sample_reaches_base[y + RANGE][x + RANGE], VecH2(x, y), uv_to_pixel, base_pixel_motion);
#else
        DilateSecondary(motion, sec_dist2, blur_motion, blur_sec_dist2, tex, sample_reaches_base[y + RANGE][x + RANGE], VecH2(x, y), uv_to_pixel, base_pixel_blur_motion);
#endif
    }
#endif

#if DUAL_DILATE_MOTION // #BlurMotion
    ToBlurMotion(blur_motion.xy, blur_max_length2);
    ToBlurMotion(blur_motion.zw); // here we have 'blur_sec_dist2' but we need 'blur_sec_length2'
#endif

#else // !DUAL_MOTION

    VecH2 length2 = VecH2(ScreenLength2(motion.xy), ScreenLength2(motion.zw));
#if DUAL_DILATE_MOTION
    VecH2 blur_length2 = length2;
#endif
    PixelMotion base_pixel_motion;
    base_pixel_motion.set(motion.xy, uv_to_pixel);

#if RANGE <= MAX_UNROLL_RANGE
    UNROLL for (Int y = -RANGE; y <= RANGE; y++)
        UNROLL for (Int x = -RANGE; x <= RANGE; x++) if (x || y)
#if !DUAL_DILATE_MOTION
            DilateMaxMin(motion, length2, TexPointOfs(Img, uv, VecI2(x, y)), VecH2(x, y), uv_to_pixel, base_pixel_motion); // need UV clamp, so can't use Img[]
#else
            DilateMaxMin(motion, length2, blur_motion, blur_length2, TexPointOfs(Img, uv, VecI2(x, y)), VecH2(x, y), uv_to_pixel, base_pixel_motion); // need UV clamp, so can't use Img[]
#endif
#else
    LOOP for (Int y = -RANGE; y <= RANGE; y++)
        LOOP for (Int x = -RANGE; x <= RANGE; x++)
#if !DUAL_DILATE_MOTION
            DilateMaxMin(motion, length2, TexPoint(Img, uv + Vec2(x, y) * RTSize.xy), VecH2(x, y), uv_to_pixel, base_pixel_motion); // need UV clamp, so can't use Img[]
#else
            DilateMaxMin(motion, length2, blur_motion, blur_length2, TexPoint(Img, uv + Vec2(x, y) * RTSize.xy), VecH2(x, y), uv_to_pixel, base_pixel_motion); // need UV clamp, so can't use Img[]
#endif
#endif

#if DUAL_DILATE_MOTION // #BlurMotion
    ToBlurMotion(blur_motion.xy, blur_length2.x);
    ToBlurMotion(blur_motion.zw, blur_length2.y);
#endif

#endif // DUAL_MOTION

    return motion;
}
/******************************************************************************/
// BLUR
/******************************************************************************/
VecH2 SampleWeight(Flt base_depth, Flt sample_depth, Half base_uv_motion_len, Half sample_uv_motion_len, Half uv_motion_len_to_step, Half step) // X=weight to be used for the specific sample, Y=if this sample is covered by base movement
{
    Half depth_delta = sample_depth - base_depth;
    VecH2 depth_weight = Sat(depth_delta * VecH2(1.0 / DEPTH_TOLERANCE, -1.0 / DEPTH_TOLERANCE) + 0.5);        // X=how much base is in front of sample, Y=how much sample is in front of base. 0.5 (middle) is returned if both have the same depth. Here it's always X=1-Y and Y=1-X
    VecH2 motion_weight = Sat(VecH2(base_uv_motion_len, sample_uv_motion_len) * uv_motion_len_to_step - step); // have to convert motion into step (instead of step into motion) to make sure that we reach 1.0 weights, this weight is about checking if one position motion is covering another position (smoothly). X=if base motion is covering sample, Y=if sample motion is covering base
                                                                                                               // depth_weight.x*motion_weight.x = this is needed for cases where base is the moving object     and sample is a static background (base  =object is in front, so depth_weight.x=1, base  =object is moving so motion_weight.x=1), we're returning weight=1 so background sample will be used on the object     position, which will make it appear semi-transparent
    // depth_weight.y*motion_weight.y = this is needed for cases where base is the static background and sample is a moving object     (sample=object is in front, so depth_weight.y=1, sample=object is moving so motion_weight.y=1), we're returning weight=1 so object     sample will be used on the background position, which will draw object on top of background
    Half weight = Dot(depth_weight, motion_weight);                                     // return sum of both cases, this will always be 0..1, because even if both base and sample have motion_weight, then depth weight is always X=1-Y
    return VecH2(weight, PRECISE ? motion_weight.x * depth_weight.x : motion_weight.x); // for PRECISE also mul by base depth_weight, because 'near' would have to be processed along 'base_uv_motion' line to don't do extra mul, but now we always do along biggest motion line, so treat as far only if depth is good, because without it, it just increases weight of fast moving objects when rotating camera around them
}
Half UVMotionLength(VecH2 uv_motion) {
    // return Abs(Dot(uv_motion, Normalize(dir.xy)))*MotionScale_2; don't use because it might lower blurring (if object is moving, but camera is rotating faster and in perpendicular way to the object movement, camera blur direction would take priority, then object motion would become 0, and it would become focused and not blurry, so better to keep the wrong blur object direction as long as it's blurry)
    return Length(uv_motion) * MotionScale_2; // here do not use AspectRatio/UVToScreen because we need UV's
}
/******************************************************************************

   Img   = color (W channel can be empty, alpha or glow)
   ImgX  = alpha
   Img1  = dilated = downsampled Max Min UV Motion
   ImgXY =              full-res         UV Motion

/******************************************************************************/
struct Pixel { // normally this should be done in full precision because we operate on many samples, however motion blur is slow, so use half
    VecH rgb;
#if ALPHA
    Half alpha;
#endif
#if GLOW
    VecH glow;
#endif

    void zero() {
        rgb = 0;
#if ALPHA
        alpha = 0;
#endif
#if GLOW
        glow = 0;
#endif
    }
    void set(Vec2 uv, Bool filter) {
        VecH4 tex = (filter ? TexLod(Img, uv) : TexPoint(Img, uv));
        rgb = tex.rgb;
// #AlphaGlow
#if GLOW // alpha channel=glow
        Half glow_a = tex.a;
        glow_a = SRGBToLinearFast(glow_a);                               // have to convert to linear because small glow of 1/255 would give 12.7/255 sRGB (Glow was sampled from non-sRGB texture and stored in RT alpha channel without any gamma conversions)
        glow = rgb * ((glow_a * BloomGlow()) / Max(Max(rgb), HALF_MIN)); // #Glow
#if ALPHA                                                                // alpha stored separately
        alpha = (filter ? TexLod(ImgX, uv) : TexPoint(ImgX, uv));
#endif
#elif ALPHA // alpha channel=alpha
        alpha = tex.a;
#endif
    }
    void mul(Half weight) // T*=weight
    {
        rgb *= weight;
#if ALPHA
        alpha *= weight;
#endif
#if GLOW
        glow *= weight;
#endif
    }
    void div(Half weight) // T/=weight
    {
        rgb /= weight;
#if ALPHA
        alpha /= weight;
#endif
#if GLOW
        glow /= weight;
#endif
    }
    void setDiv(Pixel p, Half weight) // T=p/weight
    {
        rgb = p.rgb / weight;
#if ALPHA
        alpha = p.alpha / weight;
#endif
#if GLOW
        glow = p.glow / weight;
#endif
    }
    void add(Pixel p) // T+=p
    {
        rgb += p.rgb;
#if ALPHA
        alpha += p.alpha;
#endif
#if GLOW
        glow += p.glow;
#endif
    }
    void add(Pixel p, Half weight) // T+=p*weight
    {
        rgb += p.rgb * weight;
#if ALPHA
        alpha += p.alpha * weight;
#endif
#if GLOW
        glow += p.glow * weight;
#endif
    }
    void mulAdd(Half mul, Pixel add) // T=T*mul+add
    {
        rgb = rgb * mul + add.rgb;
#if ALPHA
        alpha = alpha * mul + add.alpha;
#endif
#if GLOW
        glow = glow * mul + add.glow;
#endif
    }
    void setMulAdd(Pixel p, Half mul, Pixel add) // T=p*mul+add
    {
        rgb = p.rgb * mul + add.rgb;
#if ALPHA
        alpha = p.alpha * mul + add.alpha;
#endif
#if GLOW
        glow = p.glow * mul + add.glow;
#endif
    }
};
/******************************************************************************/
void Blur_VS(VtxInput vtx,
#if EXPOSURE
             NOINTERP out Half bloom_scale
             : BLOOM_SCALE,
#endif
               NOPERSP out Vec2 uv
             : UV,
               NOPERSP out Vec4 vpos
             : POSITION) {
#if EXPOSURE
    bloom_scale = BloomScale() * ImgX1[VecI2(0, 0)];
#endif
    uv = vtx.uv();
    vpos = vtx.pos4();
}

VecH4 Blur_PS(
#if EXPOSURE
    NOINTERP Half bloom_scale
    : BLOOM_SCALE,
#endif
      NOPERSP Vec2 uv0
    : UV,
      NOPERSP PIXEL
#if GLOW
      ,
      out VecH bloom
    : TARGET1
#endif
    ) : TARGET {
    Pixel base;
    base.set(uv0, true);               // here use linear filtering in case 'Img' is super-sampled and we're just down-sampling without any motion blur
    VecH4 dilated = TexLod(Img1, uv0); // dilated motion (XY=biggest, ZW=smallest), use linear filtering because 'Img1' may be smaller
    // return VecH4(SRGBToLinear(Vec(Length(dilated.xy)*10,Length(dilated.zw)*10,0)),1);

    BRANCH if (any(dilated.xy)) // XY=biggest, can use 'any' because small motions were already forced to 0 in 'Convert' #DilatedMotionZero
    {
        VecH2 blur_dir = dilated.xy;
        if (!DUAL_DILATE_MOTION)
            ToBlurMotion(blur_dir); // #BlurMotion

        Vec4 dir = Vec4(blur_dir, -blur_dir);
        Int steps = SAMPLES;
        dir /= steps;
        Half jitter;
        if (JITTER)
            jitter = Noise1D_4(pixel.xy); // use only 4 step dither because others might be too distracting (individual pixels visible)
        Vec2 uv1 = uv0;

#if DUAL_MOTION
        BRANCH if (Dist2(dilated.xy, dilated.zw) <= Length2(dilated.xy) * Sqr(0.5)) // if secondary motion is close to biggest motion then just do a fast and simple blur, ignoring depths and precise motions
#else
        BRANCH if (Length2(dilated.zw) >= Length2(dilated.xy) * Sqr(0.5)) // if smallest motion is close to biggest motion then just do a fast and simple blur, ignoring depths and precise motions
#endif
        {
            Half weight = JITTER ? 0 : 1; // when using JITTER the base sample has to be jittered too so 'base' can't be used
            Pixel blur;
            if (JITTER)
                blur.zero();
            else
                blur = base; // when using JITTER the base sample has to be jittered too so 'base' can't be used
            if (JITTER)      // for JITTER we have to process steps starting from 0.5 because we're not leaving extra weight for the base sample (since it has to be jittered too), so move the starting UV's by 0.5 back and apply jitter offset
            {
                Half step0 = (/*i=0*/ -0.5) + jitter;
                uv0 += step0 * dir.xy;
                Half step1 = (/*i=0*/ -0.5) - jitter;
                uv1 += step1 * dir.zw;
            }
            UNROLL for (Int i = 1; i <= steps; i++) // start from 1 because we've already got #0 before
            {
                Half w0 = UVInsideView(uv0 += dir.xy);
                Pixel p0;
                p0.set(uv0, true);
                blur.add(p0, w0); // can use linear filtering because this is fast mode ignoring depths/motions
                Half w1 = UVInsideView(uv1 += dir.zw);
                Pixel p1;
                p1.set(uv1, true);
                blur.add(p1, w1); // can use linear filtering because this is fast mode ignoring depths/motions
                weight += w0 + w1;
            }
            if (!JITTER || weight > 0.5)
                base.setDiv(blur, weight); // update only if have any samples, base=blur/weight

            if (SHOW_BLUR_PIXELS)
                base.rgb.g += 0.1;
        }
        else {
            VecH2 weight = 0; // X=near, Y=far
            Pixel near, far;
            near.zero();
            far.zero();

            // GET DEPTH + MOTION
            Flt base_depth;
            VecH2 base_uv_motion;
            Half base_uv_motion_len;
            if (TEMPORAL) // in Temporal the color RT is already adjusted by UV (to be always the same each frame), but depth and motion RT's are jittered every frame, which gives inconsistency between color and DepthMotion.
            {             // to workaround this problem, DepthMotion are taken from the pixel that's closest to camera and has highest motion
                // TODO: Warning: these ignore UVClamp/UVInView
                Half base_uv_motion_len2;
                if (GATHER) {
                    Vec2 test_uv = uv0 + TemporalOffset;
                    Vec4 d = TexDepthRawGather(test_uv);
                    VecH4 r = TexGatherR(ImgXY, test_uv); // motion.x
                    VecH4 g = TexGatherG(ImgXY, test_uv); // motion.y

                    VecH2 test_uv_motion;
                    Half test_len2;
                    if (1) // slower, higher quality. This improves blur on pixels around object (border). To verify improvement, set very low 'D.density', and rotate player+camera in TPP very fast left or right, object has to be fixed to the camera, and background rotating/blurry, you will see that this mode improves smoothness of object border pixels.
                    {
                        // set initial values with 'TemporalOffsetGatherIndex' as if we were using 'uv0' without 'TemporalOffset'
                        base_depth = d[TemporalOffsetGatherIndex];
                        base_uv_motion = VecH2(r[TemporalOffsetGatherIndex], g[TemporalOffsetGatherIndex]);
                        base_uv_motion_len2 = Length2(base_uv_motion);

                        test_uv_motion = VecH2(r.x, g.x);
                        test_len2 = Length2(test_uv_motion);
                        if (DEPTH_SMALLER(d.x, base_depth) && test_len2 > base_uv_motion_len2) {
                            base_depth = d.x;
                            base_uv_motion = test_uv_motion;
                            base_uv_motion_len2 = test_len2;
                        }
                    } else {
                        base_depth = d.x;
                        base_uv_motion = VecH2(r.x, g.x);
                        base_uv_motion_len2 = Length2(base_uv_motion);
                    } // faster, lower quality, just set from first one
                    test_uv_motion = VecH2(r.y, g.y);
                    test_len2 = Length2(test_uv_motion);
                    if (DEPTH_SMALLER(d.y, base_depth) && test_len2 > base_uv_motion_len2) {
                        base_depth = d.y;
                        base_uv_motion = test_uv_motion;
                        base_uv_motion_len2 = test_len2;
                    }
                    test_uv_motion = VecH2(r.z, g.z);
                    test_len2 = Length2(test_uv_motion);
                    if (DEPTH_SMALLER(d.z, base_depth) && test_len2 > base_uv_motion_len2) {
                        base_depth = d.z;
                        base_uv_motion = test_uv_motion;
                        base_uv_motion_len2 = test_len2;
                    }
                    test_uv_motion = VecH2(r.w, g.w);
                    test_len2 = Length2(test_uv_motion);
                    if (DEPTH_SMALLER(d.w, base_depth) && test_len2 > base_uv_motion_len2) {
                        base_depth = d.w;
                        base_uv_motion = test_uv_motion;
                        base_uv_motion_len2 = test_len2;
                    }
                } else { // Warning: this ignores 'TemporalOffsetGatherIndex'
                    Vec2 test_uv = uv0 + TemporalOffsetStart;
                    FAST_UNROLL for (Int y = 0; y <= 1; y++)
                        FAST_UNROLL for (Int x = 0; x <= 1; x++) {
                        Flt test_depth = TexDepthRawPointOfs(test_uv, VecI2(x, y));      // need UV clamp
                        VecH2 test_uv_motion = TexPointOfs(ImgXY, test_uv, VecI2(x, y)); // need UV clamp
                        Half test_len2 = Length2(test_uv_motion);
                        if ((x == 0 && y == 0) // first sample
                            || (DEPTH_SMALLER(test_depth, base_depth) && test_len2 > base_uv_motion_len2)) {
                            base_depth = test_depth;
                            base_uv_motion = test_uv_motion;
                            base_uv_motion_len2 = test_len2;
                        }
                    }
                }
                base_uv_motion_len = Sqrt(base_uv_motion_len2) * MotionScale_2;
            } else {
                base_depth = TexDepthRawPoint(uv0);    // Tex in case src is super sampled
                base_uv_motion = TexPoint(ImgXY, uv0); // Tex in case src is super sampled
                base_uv_motion_len = UVMotionLength(base_uv_motion);
            }
            base_depth = LinearizeDepth(base_depth);

            Half uv_motion_len_to_step0 = 1 / Length(dir.xy); // NaN, allows to convert travelled UV distance into how many steps (travelled_uv*uv_motion_len_to_step=step)
            Half uv_motion_len_to_step1 = 1 / Length(dir.zw); // NaN
            Half step_add = -1.5;                             // this value allows the last step to still has some weight, use -1.5 instead of -1 because on a 3D ball moving right, pixels in the center have higher movement due to perspective correction (pixels closer to camera move faster than those far), so when calculating biggest movement from neighbors, then the pixels at the border will get biggest/dilated movement (coming from ball center) that's slightly bigger than border movement. So the search vector that's set from biggest/dilated motion will be bigger than the sample movement, and for example its motion might cover only 9/10 steps instead of 10/10. To workaround this, make step offset slightly smaller.
            if (JITTER)                                       // for JITTER we have to process steps starting from 0.5 because we're not leaving extra weight for the base sample (since it has to be jittered too), so move the starting UV's by 0.5 back and apply jitter offset
            {
                Half step0 = (/*i=0*/ -0.5) + jitter;
                uv0 += step0 * dir.xy;
                Half step1 = (/*i=0*/ -0.5) - jitter;
                uv1 += step1 * dir.zw;
            }
            UNROLL for (Int i = 1; i <= steps; i++) // start from 1 because #0 is processed at the end
            {
                Half step0, step1;
                if (JITTER) {
                    /*step0=(i-0.5)+jitter;
                      step1=(i-0.5)-jitter;
                      uv0  =start+step0*dir.xy;
                      uv1  =start+step1*dir.zw;
                      step0=Max(0, step0+step_add);
                      step1=Max(0, step1+step_add);*/
                    step0 = Max(0, Half(i - 0.5 + step_add) + jitter);
                    step1 = Max(0, Half(i - 0.5 + step_add) - jitter);
                } else {
                    step0 = step1 = Max(0, i + step_add);
                }
                uv0 += dir.xy;
                uv1 += dir.zw;

                // need to disable filtering to avoid ghosting on borders
                Flt sample0_depth, sample1_depth;
                VecH2 sample0_uv_motion, sample1_uv_motion;
                Pixel p0, p1;
                if (TEMPORAL)                                                        // in Temporal the color RT is already adjusted by UV (to be always the same each frame), but depth and motion RT's are jittered every frame, which gives inconsistency between color and DepthMotion.
                {                                                                    // to workaround this problem, DepthMotion are taken from the pixel that's closest to camera
                    Vec2 aligned_uv0 = (Floor(uv0 * ImgSize.zw) + 0.5) * ImgSize.xy; // have to align to exact center of the texel
                    Vec2 aligned_uv1 = (Floor(uv1 * ImgSize.zw) + 0.5) * ImgSize.xy; // have to align to exact center of the texel
                    // TODO: this should use 'aligned_uv0' 'aligned_uv1', however that would break SUPER_RES which has 2x2 color pixels inside 1x1 DepthMotion
                    p0.set(uv0, false); // here can't use filtering because we need precise per-pixel data to avoid leaking
                    p1.set(uv1, false); // here can't use filtering because we need precise per-pixel data to avoid leaking
#if GATHER
                    Vec4 d0 = TexDepthRawGather(aligned_uv0 + TemporalOffset);
                    Vec4 d1 = TexDepthRawGather(aligned_uv1 + TemporalOffset);
#endif
                    Vec2 test_uv0 = aligned_uv0 + TemporalOffsetStart; // this will be used for TexPoint
                    Vec2 test_uv1 = aligned_uv1 + TemporalOffsetStart; // this will be used for TexPoint
                    VecI2 ofs0 = 0, ofs1 = 0;
#if GATHER
                    /* TEXTURE ACCESSING                 (Y^)
                       GATHER returns in following order: V1 X  Y
                                                          V0 W  Z
                                                           + U0 U1 (X>) */
                    sample0_depth = d0.w; // VecI2(0, 0)
                    TestDepth(sample0_depth, d0.x, ofs0, VecI2(0, 1));
                    TestDepth(sample0_depth, d0.y, ofs0, VecI2(1, 1));
                    TestDepth(sample0_depth, d0.z, ofs0, VecI2(1, 0));

                    sample1_depth = d1.w; // VecI2(0, 0)
                    TestDepth(sample1_depth, d1.x, ofs1, VecI2(0, 1));
                    TestDepth(sample1_depth, d1.y, ofs1, VecI2(1, 1));
                    TestDepth(sample1_depth, d1.z, ofs1, VecI2(1, 0));
#else
                    sample0_depth = TexDepthRawPoint(test_uv0);
                    sample1_depth = TexDepthRawPoint(test_uv1);
                    FAST_UNROLL for (Int y = 0; y <= 1; y++)
                        FAST_UNROLL for (Int x = 0; x <= 1; x++) if (x || y) // skip (0,0) we already have it
                    {
                        VecI2 ofs = VecI2(x, y);
                        Flt d = TexDepthRawPointOfs(test_uv0, ofs);
                        if (DEPTH_SMALLER(d, sample0_depth)) {
                            sample0_depth = d;
                            ofs0 = ofs;
                        }
                        d = TexDepthRawPointOfs(test_uv1, ofs);
                        if (DEPTH_SMALLER(d, sample1_depth)) {
                            sample1_depth = d;
                            ofs1 = ofs;
                        }
                    }
#endif
                    sample0_depth = LinearizeDepth(sample0_depth);
                    sample0_uv_motion = TexPoint(ImgXY, test_uv0 + ofs0 * ImgSize.xy);
                    sample1_depth = LinearizeDepth(sample1_depth);
                    sample1_uv_motion = TexPoint(ImgXY, test_uv1 + ofs1 * ImgSize.xy);
                } else {
                    p0.set(uv0, false); // here can't use filtering because we need precise per-pixel data to avoid leaking
                    p1.set(uv1, false); // here can't use filtering because we need precise per-pixel data to avoid leaking
                    sample0_uv_motion = TexPoint(ImgXY, uv0).xy;
                    sample0_depth = TexDepthPoint(uv0); // Tex in case src is super sampled
                    sample1_uv_motion = TexPoint(ImgXY, uv1).xy;
                    sample1_depth = TexDepthPoint(uv1); // Tex in case src is super sampled
                }

                Half sample0_uv_motion_len = UVMotionLength(sample0_uv_motion);
                Half sample1_uv_motion_len = UVMotionLength(sample1_uv_motion);

                VecH2 w0 = SampleWeight(base_depth, sample0_depth, base_uv_motion_len, sample0_uv_motion_len, uv_motion_len_to_step0, step0);
                w0.x *= UVInsideView(uv0);
                VecH2 w1 = SampleWeight(base_depth, sample1_depth, base_uv_motion_len, sample1_uv_motion_len, uv_motion_len_to_step1, step1);
                w1.x *= UVInsideView(uv1);

                if (PRECISE) {
                    w0 = w0.x * VecH2(w0.y, 1 - w0.y); // adjust X weight to be sample weight * near weight, Y weight to be sample weight * far weight (1-near)
                    w1 = w1.x * VecH2(w1.y, 1 - w1.y); // adjust X weight to be sample weight * near weight, Y weight to be sample weight * far weight (1-near)
                    near.add(p0, w0.x);
                    near.add(p1, w1.x); // near+=p0*w0.x + p1*w1.x;
                    far.add(p0, w0.y);
                    far.add(p1, w1.y); // far +=p0*w0.y + p1*w1.y;
                } else {
                    w0.y = Max(w0.y, w0.x); // adjust Y weight to be maximum of (near weight (Y) and sample weight (X)), it works as if sample weight but if it's near then it's always 1 (to ignore depth: anything covering the base sample), this weight will be used to blend with the final color and the base color
                    w1.y = Max(w1.y, w1.x);
                    near.add(p0, w0.x);
                    near.add(p1, w1.x); // near+=p0*w0.x + p1*w1.x; // in !PRECISE we just need one data (doesn't matter if we operate on 'far' or 'near', so just choose any)
                }
                weight += w0 + w1;
            }
            Half mul = 1.0 / (steps * 2 + (JITTER ? 0 : 1)); // in every step we have 2 samples + 1 to make room for base sample (can't do that for JITTER because when base is moving then it has to be jittered too, so it has to be processed by codes in the loop)
            if (PRECISE) {
                // Warning: these codes don't adjust 'weight.x'
                if (JITTER) {
                    if (weight.x < 1)
                        near.add(base, 1 - weight.x); // if gathered near colors are less than 1 full sample, then increase to 'base' (use it only if necessary because we want 'base' to be jittered)
                    else
                        near.div(weight.x); // otherwise normalize
                } else {
                    near.add(base);         // here we can always add base sample because we don't need it to be jittered
                    near.div(weight.x + 1); // normalize including base sample
                }
#if 0 // original
            if(CanDiv(weight.y))far/=weight.y; // normalize, NaN
            weight.y*=mul;
            base=Lerp(near, far, weight.y);
#else // optimized
                far.mul(mul);
                weight.y *= mul;
                base.setMulAdd(near, 1 - weight.y, far); // base=near*(1-weight.y) + far;
#endif
            } else if (1) // process using near/far weights
            {
                if (CanDiv(weight.x)) // process if we've got any samples (if not then just keep 'base' as is)
                {
                    weight.y *= mul;
#if 0 // original
               near/=weight.x; // normalize, NaN
               base=Lerp(base, near, weight.y);
#else // optimized
                    near.mul(weight.y / weight.x);
                    base.mulAdd(1 - weight.y, near); // base=base*(1-weight.y) + near*(weight.y/weight.x);
#endif
                }
            } else // simple mode ignoring near/far
            {
                near.mul(mul);
                weight.x *= mul;
                base.mulAdd(1 - weight.x, near); // base=base*(1-weight.x) + near;
            }

            if (SHOW_BLUR_PIXELS)
                base.rgb.r += 0.1;
        }

#if DITHER == 1 // only in blur
        ApplyDither(base.rgb, pixel.xy);
#endif

#if 0 // test how many samples were used for blurring
      base.rgb=steps/Half(SAMPLES);
#endif
    }

#if DITHER == 2 // always
    ApplyDither(base.rgb, pixel.xy);
#endif

#if GLOW
#if !EXPOSURE
    Half bloom_scale = BloomScale();
#endif
    bloom = BloomColor(base.rgb, bloom_scale) + base.glow;
#endif

#if ALPHA
    return VecH4(base.rgb, base.alpha);
#else
    return VecH4(base.rgb, 1); // force full alpha so back buffer effects can work ok
#endif
}
/******************************************************************************
void Explosion_VS(VtxInput vtx,
              out Vec  outPos:TEXCOORD0,
              out Vec  outVel:TEXCOORD1,
              out Vec4 vpos  :POSITION )
{
   outVel=TransformTP (Normalize(vtx.pos())*Step, (Matrix3)CamMatrix);
   outPos=TransformPos(vtx.pos());
   vpos  =Project     ( outPos  );
}
void Explosion_PS(Vec   inPos:TEXCOORD0,
                  Vec   inVel:TEXCOORD1,
              out VecH outVel:TARGET1  ) // #RTOutput
{
   need to update
   inVel/=inPos.z;
   outVel=inVel;
}
/******************************************************************************/
