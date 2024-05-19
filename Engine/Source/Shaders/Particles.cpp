/******************************************************************************/
#include "!Header.h"
#include "Sky.h"
/******************************************************************************/
#define ANIM_NONE 0
#define ANIM_YES 1
#define ANIM_SMOOTH 2

#include "!Set Prec Struct.h"
BUFFER(Particle)
Vec2 ParticleFrames = Vec2(1, 1);
BUFFER_END
#include "!Set Prec Default.h"
/******************************************************************************/
void Particle_VS(VtxInput vtx,
                 out Vec4 vpos
                 : POSITION,
                   out VecH4 col
                 : COLOR,
                   out Vec2 uv
                 : UV
#if SOFT
                   ,
                   out Vec2 depth_size
                 : DEPTH_SIZE
#endif
#if ANIM == ANIM_SMOOTH
                   ,
                   out Vec anim
                 : UV1
#endif
) {
    uv = vtx.uv();
    col = (PALETTE ? vtx.colorF() : vtx.colorFast()); // use linear color for palette

    Half size = vtx.size(),
         angle = vtx._tan.w;
    Vec pos = TransformPos(vtx.pos());
    VecH2 offset = VecH2(uv) * VecH2(2, -2) + VecH2(-1, 1);
    VecH2 cos_sin;
    CosSin(cos_sin.x, cos_sin.y, angle);
    offset = Rotate(offset, cos_sin);

    if (MOTION_STRETCH)
        if (pos.z > 0) {
#define PARTICLE_PROJECT 100
            VecH vel = TransformDir(vtx.tan());
            if (vel.z < 0)
                vel = -vel; // view space velocity, always make it along the camera direction, so we won't have a situation where the 'pos1' is behind the camera
            Vec pos1 = pos + vel / PARTICLE_PROJECT;
            VecH2 vel2 = (pos1.xy / pos1.z - pos.xy / pos.z) * PARTICLE_PROJECT; // 2D velocity
            Half len = Length2(vel2);
            if (len > 0) {
                len = Sqrt(len);
                // Flt max_stretch=5; if(len>max_stretch){vel2*=max_stretch/len; len=max_stretch;} // NaN
                VecH2 x = vel2 * (vel2.x / len),
                      y = vel2 * (vel2.y / len);
                offset = VecH2(offset.x * (x.x + 1) + offset.y * y.x, offset.x * x.y + offset.y * (y.y + 1));
                if (MOTION_AFFECTS_ALPHA) {
                    if (PALETTE)
                        col /= 1 + len; // in RM_PALETTE each component
                    else
                        col.a /= 1 + len; // in RM_BLEND   only alpha
                }
            }
        }
    pos.xy += offset * size;

    // sky
    Flt d = Length(pos);
    Half opacity = Sat(d * SkyFracMulAdd.x + SkyFracMulAdd.y);
    if (PALETTE)
        col *= opacity; // in RM_PALETTE each component
    else
        col.a *= opacity; // in RM_BLEND   only alpha

#if SOFT
    {
        depth_size.x = pos.z;
        depth_size.y = size;

        if (pos.z > -size) {
            Flt wanted_z = Max(Viewport.from + EPS, pos.z - size);
            if (pos.z >= wanted_z)
                pos *= wanted_z / pos.z;
            else {
                pos.xy *= (wanted_z + size) / (pos.z + size); // this is optional (scales UV tex when behind camera)
                pos.z = wanted_z;
            }
        }
    }
#endif
    if (ANIM != ANIM_NONE) {
        Flt frame = vtx.uv1().x;
#if 0                   // integer version
      UInt frames=UInt(Round(ParticleFrames.x*ParticleFrames.y));
      UInt f     =UInt(Trunc(frame))%frames;
#if ANIM == ANIM_SMOOTH // frame blending
      {
         UInt f1=(f+1)%frames;
         anim.xy =uv;
         anim.z  =Frac(frame);
         anim.x +=f1%UInt(Round(ParticleFrames.x));
         anim.y +=f1/UInt(Round(ParticleFrames.x));
         anim.xy/=              ParticleFrames    ;
      }
#endif
      uv.x+=f%UInt(Round(ParticleFrames.x));
      uv.y+=f/UInt(Round(ParticleFrames.x));
      uv  /=             ParticleFrames    ;
#else // float version
        Flt frames = ParticleFrames.x * ParticleFrames.y;
        frame = Frac(frame / frames) * frames; // frame=[0..frames)
        Flt f;
        frame = modf(frame, f);
#if ANIM == ANIM_SMOOTH // frame blending
        {
            Flt f1 = f + 1;
            if (f1 + 0.5 >= frames)
                f1 = 0; // f1=(f+1)%frames;
            anim.xy = uv;
            anim.z = frame; // frame step [0..1)
            Flt y;
            anim.x += ParticleFrames.x * modf(f1 / ParticleFrames.x, y); // anim.x+=f1%Round(ParticleFrames.x);
            anim.y += y;                                                 // anim.y+=f1/Round(ParticleFrames.x);
            anim.xy /= ParticleFrames;
        }
#endif
        Flt y;
        uv.x += ParticleFrames.x * modf(f / ParticleFrames.x, y); // uv.x+=f%Round(ParticleFrames.x);
        uv.y += y;                                                // uv.y+=f/Round(ParticleFrames.x);
        uv /= ParticleFrames;
#endif
    }
#if GL // needed for iOS PVRTC Pow2 #ParticleImgPart
    uv.xy *= ImgSize.xy;
#endif
    vpos = Project(pos);
}
/******************************************************************************/
VecH4 Particle_PS(PIXEL,
                  VecH4 col
                  : COLOR,
                    Vec2 uv
                  : UV
#if SOFT
                    ,
                    Vec2 depth_size
                  : DEPTH_SIZE
#endif
#if ANIM == ANIM_SMOOTH
                    ,
                    Vec anim
                  : UV1
#endif
#if !PALETTE
                    ,
                    out Half alpha
                  : TARGET1 // #RTOutput.Blend
#endif
                  ) : TARGET {
    VecH4 tex = RTex(Img, uv);
#if ANIM == ANIM_SMOOTH
    tex = Lerp(tex, RTex(Img, anim.xy), anim.z);
#endif
#if SOFT
    {
        Flt z0 = depth_size.x - tex.a * depth_size.y,
            z1 = depth_size.x + tex.a * depth_size.y;
        tex.a *= Sat((TexDepthPix(pixel.xy) - z0) / depth_size.y); // fade out at occluder
        tex.a *= Sat(z1 / (z1 - z0 + HALF_MIN));                   // smooth visibility fraction when particle near (or behind) camera, NaN
    }
#endif
    if (PALETTE)
        col *= tex.a;
    else
        col *= tex;
#if !PALETTE
    alpha = col.a;
#endif
    return col;
}
/******************************************************************************/
