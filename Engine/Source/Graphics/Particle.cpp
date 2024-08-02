/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
#define CC4_PRTC CC4('P', 'R', 'T', 'C')

#if 0
#define TEX_ZERO HalfZero
#define TEX_ONE HalfOne
#define SET_TEX(t, x, y) t.set(x, y)
#else
#define TEX_ZERO 0
#define TEX_ONE 255
#define SET_TEX(t, x, y) t.set(x, y, 0, 0)
#endif
/******************************************************************************/
Cache<Particles> ParticlesCache("Particles");

static Color (*ColorFunc)(C Color &color, Flt s) = ColorAlpha; // pointer to function

static Bool SoftParticles() {
    return D.particlesSoft() && Renderer.canReadDepth1S(); // we want soft and it's available
}
/******************************************************************************/
static Flt OpacityDefault(Flt s) { return Sqrt((s < 0.1f) ? s / 0.1f : 1 - (s - 0.1f) / 0.9f); }
static Flt OpacitySin(Flt s) { return Sin(s * PI); }
/******************************************************************************/
static Flt (*GetOpacityFunc(Bool smooth))(Flt s) // function which returns function, "GetOpacityFunc(Bool smooth)" -> "Flt f(Flt s)"
{
    return smooth ? OpacitySin : OpacityDefault;
}
static Flt (*GetOpacityFunc(C Particles &particles))(Flt s) // function which returns function, "GetOpacityFunc(C Particles &particles)" -> "Flt f(Flt s)"
{
    return particles.opacity_func ? particles.opacity_func : GetOpacityFunc(particles.smooth_fade);
}
/******************************************************************************/
// MAIN
/******************************************************************************/
Bool DrawParticleBegin(C Image &image, Byte glow, Bool motion_affects_alpha) {
    PROFILE_START("DrawParticleBegin(C Image &image, Byte glow, Bool motion_affects_alpha)")
    Renderer.wantDepthRead(); // !! call before 'SoftParticles' !!
    // Shader *shader;
    Bool soft = SoftParticles();
    switch (Renderer()) {
    default:
        PROFILE_STOP("DrawParticleBegin(C Image &image, Byte glow, Bool motion_affects_alpha)")
        return false;
    case RM_BLEND:
        // shader = Sh.Particle[false][soft][0][motion_affects_alpha];// disabled to fix particle distorsion bugs
        ColorFunc = ColorAlpha;
        D.alpha(Renderer.fastCombine() ? ALPHA_BLEND : ALPHA_RENDER_BLEND_FACTOR);
        D.alphaFactor(Color(0, 0, 0, glow));
        MaterialClear();
        if (glow)
            Renderer._has |= HAS_GLOW;
        break; // 'MaterialClear' must be called when changing 'D.alphaFactor'
    case RM_PALETTE:
    case RM_PALETTE1:
        // shader = Sh.Particle[true][soft][0][motion_affects_alpha];// disabled to fix particle distorsion bugs
        ColorFunc = ColorMul;
        D.alpha(ALPHA_ADD);
        break;
    }
    SetOneMatrix();
    D.depthOnWrite(true, false);
    VI.image(&image);
    // VI.shader(shader); // disabled to fix particle distorsion bugs
    VI.setType(VI_3D_BILB, VI_QUAD_IND);
#if GL // needed for iOS PVRTC Pow2 #ParticleImgPart
    Sh.ImgSize->setConditional(image._part.xy);
#endif
    PROFILE_STOP("DrawParticleBegin(C Image &image, Byte glow, Bool motion_affects_alpha)")
    return true;
}
void DrawParticleAdd(C Color &color, Flt opacity, Flt radius, Flt angle, C Vec &pos, C Vec &vel) {
    PROFILE_START("DrawParticleAdd(C Color &color, Flt opacity, Flt radius, Flt angle, C Vec &pos, C Vec &vel)")
    static Vtx3DBilb v[4]; // Use static array to avoid dynamic allocation
    v[0].pos = v[1].pos = v[2].pos = v[3].pos = pos;
    v[0].vel_angle = v[1].vel_angle = v[2].vel_angle = v[3].vel_angle.set(vel.x, vel.y, vel.z, GPU_HALF_SUPPORTED ? AngleFull(angle) : angle);
    v[0].color = v[1].color = v[2].color = v[3].color = ColorFunc(color, opacity);
    v[0].size = v[1].size = v[2].size = v[3].size = radius;
    SET_TEX(v[0].tex, TEX_ZERO, TEX_ONE);
    SET_TEX(v[1].tex, TEX_ONE, TEX_ONE);
    SET_TEX(v[2].tex, TEX_ONE, TEX_ZERO);
    SET_TEX(v[3].tex, TEX_ZERO, TEX_ZERO);
    if (Vtx3DBilb *vtx = (Vtx3DBilb *)VI.addVtx(4))
        memcpy(vtx, v, sizeof(v));
    PROFILE_STOP("DrawParticleAdd(C Color &color, Flt opacity, Flt radius, Flt angle, C Vec &pos, C Vec &vel)")
}

void DrawParticleEnd() {
    VI.end();
}
/******************************************************************************/
Bool DrawAnimatedParticleBegin(C Image &image, Byte glow, Bool motion_affects_alpha, Int x_frames, Int y_frames) {
    PROFILE_START("DrawAnimatedParticleBegin(C Image &image, Byte glow, Bool motion_affects_alpha, Int x_frames, Int y_frames)");

    // Setup initial rendering state
    Renderer.wantDepthRead();    // Ensure depth read is enabled
    Bool soft = SoftParticles(); // Determine if soft particles are used

    // Variable to hold shader (if required in the future)
    // Shader *shader = nullptr;

    // Configure based on renderer type
    switch (Renderer()) {
    case RM_BLEND:
        ColorFunc = ColorAlpha;
        D.alpha(Renderer.fastCombine() ? ALPHA_BLEND : ALPHA_RENDER_BLEND_FACTOR);
        D.alphaFactor(Color(0, 0, 0, glow));
        MaterialClear(); // Clear material properties if required
        if (glow)
            Renderer._has |= HAS_GLOW;
        break;

    case RM_PALETTE:
    case RM_PALETTE1:
        ColorFunc = ColorMul;
        D.alpha(ALPHA_ADD);
        break;

    default:
        PROFILE_STOP("DrawAnimatedParticleBegin(C Image &image, Byte glow, Bool motion_affects_alpha, Int x_frames, Int y_frames)");
        return false; // Return early if renderer type is unsupported
    }

    // Set common states
    SetOneMatrix();              // Setup the matrix for rendering
    D.depthOnWrite(true, false); // Enable depth writing
    VI.image(&image);            // Bind the image for rendering
    // VI.shader(shader); // Shader setup disabled due to fixes

    // Set vertex information
    VI.setType(VI_3D_BILB_ANIM, VI_QUAD_IND);
    Sh.ParticleFrames->set(VecI2(x_frames, y_frames));

#if GL
    Sh.ImgSize->setConditional(image._part.xy); // Set image size conditionally for certain platforms
#endif

    PROFILE_STOP("DrawAnimatedParticleBegin(C Image &image, Byte glow, Bool motion_affects_alpha, Int x_frames, Int y_frames)");
    return true;
}
void DrawAnimatedParticleAdd(C Color &color, Flt opacity, Flt radius, Flt angle, C Vec &pos, C Vec &vel, Flt frame) {
    PROFILE_START("DrawAnimatedParticleAdd(C Color &color, Flt opacity, Flt radius, Flt angle, C Vec &pos, C Vec &vel, Flt frame)")
    static Vtx3DBilbAnim v[4]; // Utilisation d'un tableau statique pour éviter l'allocation dynamique
    v[0].pos = v[1].pos = v[2].pos = v[3].pos = pos;
    v[0].vel_angle = v[1].vel_angle = v[2].vel_angle = v[3].vel_angle.set(vel.x, vel.y, vel.z, GPU_HALF_SUPPORTED ? AngleFull(angle) : angle);
    v[0].color = v[1].color = v[2].color = v[3].color = ColorFunc(color, opacity);
    v[0].size = v[1].size = v[2].size = v[3].size = radius;
    v[0].frame = v[1].frame = v[2].frame = v[3].frame = frame;
    SET_TEX(v[0].tex, TEX_ZERO, TEX_ONE);
    SET_TEX(v[1].tex, TEX_ONE, TEX_ONE);
    SET_TEX(v[2].tex, TEX_ONE, TEX_ZERO);
    SET_TEX(v[3].tex, TEX_ZERO, TEX_ZERO);
    if (Vtx3DBilbAnim *vtx = (Vtx3DBilbAnim *)VI.addVtx(4))
        memcpy(vtx, v, sizeof(v));
    PROFILE_STOP("DrawAnimatedParticleAdd(C Color &color, Flt opacity, Flt radius, Flt angle, C Vec &pos, C Vec &vel, Flt frame)")
}

void DrawAnimatedParticleEnd() {
    VI.end();
}
/******************************************************************************/
struct AnimatedMaterialParticle {
    Int frames_x, frames_all;
    Vec2 frame_size;
    Vec nrm;
    Vec4 tan;
    // Vec2 *part; can ignore because material based particles should have TexPow2 size in the image
} AMP;
Bool DrawAnimatedMaterialParticleBegin(C Material &material, Int x_frames, Int y_frames) {
    PROFILE_START("DrawAnimatedMaterialParticleBegin(C Material &material, Int x_frames, Int y_frames)")
    if (Shader *shader = DefaultShaders(&material, VTX_POS | VTX_NRM_TAN_BIN | VTX_COLOR | VTX_TEX0, 0, false).get(Renderer())) {
        DisableSkinning();
        material.setAuto();
        VI.shader(shader);
        // VI.cull   (material.cull); can ignore because particles are always faced toward the camera
        VI.setType(VI_3D_STANDARD, VI_QUAD_IND);
        MAX(x_frames, 1);
        MAX(y_frames, 1);
        AMP.frames_x = x_frames;
        AMP.frames_all = x_frames * y_frames;
        AMP.frame_size.set(1.0f / x_frames, 1.0f / y_frames);
        AMP.nrm = -ActiveCam.matrix.z;
        AMP.tan.set(ActiveCam.matrix.x, 1);
        // AMP.part=((material.base_0 && material.base_0->partial()) ? &material.base_0->_part.xy : null);
        PROFILE_STOP("DrawAnimatedMaterialParticleBegin(C Material &material, Int x_frames, Int y_frames)")
        return true;
    }
    PROFILE_STOP("DrawAnimatedMaterialParticleBegin(C Material &material, Int x_frames, Int y_frames)")
    return false;
}
void DrawAnimatedMaterialParticleAdd(C Color &color, Flt radius, Flt angle, C Vec &pos, Int frame) {
    PROFILE_START("DrawAnimatedMaterialParticleAdd(C Color &color, Flt radius, Flt angle, C Vec &pos, Int frame)")
    static Vtx3DStandard v[4]; // Utilisation d'un tableau statique pour éviter l'allocation dynamique
    Vec x, y;
    if (angle) {
        Flt cos, sin;
        CosSin(cos, sin, angle);
        cos *= radius;
        sin *= radius;
        x = ActiveCam.matrix.y * sin + ActiveCam.matrix.x * cos;
        y = ActiveCam.matrix.y * cos - ActiveCam.matrix.x * sin;
    } else {
        x = ActiveCam.matrix.x * radius;
        y = ActiveCam.matrix.y * radius;
    }
    frame = Mod(frame, AMP.frames_all);
    Int frame_x = frame % AMP.frames_x;
    Int frame_y = frame / AMP.frames_x;
    Vec2 uv(frame_x * AMP.frame_size.x, frame_y * AMP.frame_size.y), uv1 = uv + AMP.frame_size;
    v[0].pos = pos - x + y;
    v[1].pos = pos + x + y;
    v[2].pos = pos + x - y;
    v[3].pos = pos - x - y;
    v[0].tex.set(uv.x, uv.y);
    v[1].tex.set(uv1.x, uv.y);
    v[2].tex.set(uv1.x, uv1.y);
    v[3].tex.set(uv.x, uv1.y);
    v[0].nrm = v[1].nrm = v[2].nrm = v[3].nrm = AMP.nrm;
    v[0].tan = v[1].tan = v[2].tan = v[3].tan = AMP.tan;
    v[0].color = v[1].color = v[2].color = v[3].color = color;
    // if(AMP.part)REP(4)v[i].tex*=*AMP.part;
    if (Vtx3DStandard *vtx = (Vtx3DStandard *)VI.addVtx(4)) {
        memcpy(vtx, v, sizeof(v));
        PROFILE_STOP("DrawAnimatedMaterialParticleAdd(C Color &color, Flt radius, Flt angle, C Vec &pos, Int frame)")
    }
}
void DrawAnimatedMaterialParticleEnd() {
    VI.end();
}
/******************************************************************************/
Flt ParticleOpacity(Flt particle_life, Flt particle_life_max, Bool particles_smooth_fade) {
    if (particle_life_max > EPS)
        return GetOpacityFunc(particles_smooth_fade)(particle_life / particle_life_max);
    return 0;
}
/******************************************************************************/
// PARTICLES
/******************************************************************************/
void Particles::zero() {
    PROFILE_START("Particles::zero()")
    reborn = true;
    smooth_fade = false;
    motion_affects_alpha = true;
    glow = 0;
    color.set(255, 255, 255, 255);
    _palette = false;
    _palette_index = 0;
    _render_mode = RM_BLEND;

    radius = 0.05f;
    radius_random = 0;
    radius_growth = 1;
    offset_range = 0;
    offset_speed = 1;
    life = 1;
    life_random = 0;

    glue = 0;
    damping = 0;
    ang_vel = 0;
    vel_random = 0;
    vel_constant = 0;
    accel = 0;

    emitter_life_max = 0;
    emitter_life = 0;
    _fade = 1;
    fade_in = 1;
    fade_out = 1;
    radius_scale_base = 1;
    radius_scale_time = 0;

    matrix.identity();
    _matrix_prev.zero(); // zero so that the nearest 'update' will not use 'glue'
    inside_shape = true;
    shape = Ball(1);
    _src_type = PARTICLE_NONE;
    _src_ptr = null;
    _src_elms = 0;
    _src_help = null;

    image_x_frames = 1;
    image_y_frames = 1;
    image_speed = 1;

    hard_depth_offset = 0;
    opacity_func = null;
    PROFILE_STOP("Particles::zero()")
}
Particles &Particles::del() {
    p.del();
    palette_image = null;
    image = null;
    _src_mesh = null;
    Free(_src_help);
    zero();
    return T;
}
Particles::Particles() { zero(); }
Particles &Particles::create(C ImagePtr &image, C Color &color, Int elms, Flt radius, Flt life) {
    PROFILE_START("Particles::create(C ImagePtr &image, C Color &color, Int elms, Flt radius, Flt life)")
    del();
    p.setNum(elms);
    T.color = color;
    T.radius = radius;
    T.life = life;
    _src_type = PARTICLE_STATIC_SHAPE;
    T.image = image;
    setRenderMode();
    PROFILE_STOP("Particles::create(C ImagePtr &image, C Color &color, Int elms, Flt radius, Flt life)")
    return T;
}
Particles &Particles::create(C Particles &src) {
    PROFILE_START("Particles::create(C Particles &src)")
    if (this != &src) {
        del();

        reborn = src.reborn;
        smooth_fade = src.smooth_fade;
        motion_affects_alpha = src.motion_affects_alpha;
        glow = src.glow;
        color = src.color;

        radius = src.radius;
        radius_random = src.radius_random;
        radius_growth = src.radius_growth;
        offset_range = src.offset_range;
        offset_speed = src.offset_speed;
        life = src.life;
        life_random = src.life_random;

        glue = src.glue;
        damping = src.damping;
        ang_vel = src.ang_vel;
        vel_random = src.vel_random;
        vel_constant = src.vel_constant;
        accel = src.accel;

        emitter_life_max = src.emitter_life_max;
        emitter_life = src.emitter_life;
        fade_in = src.fade_in;
        fade_out = src.fade_out;
        radius_scale_base = src.radius_scale_base;
        radius_scale_time = src.radius_scale_time;

        matrix = src.matrix;
        shape = src.shape;
        inside_shape = src.inside_shape;

        image = src.image;
        image_x_frames = src.image_x_frames;
        image_y_frames = src.image_y_frames;
        image_speed = src.image_speed;

        palette_image = src.palette_image;

        _palette = src._palette;
        _palette_index = src._palette_index;
        _fade = src._fade;
        _render_mode = src._render_mode;
        _src_type = src._src_type;
        _src_ptr = src._src_ptr;
        _src_mesh = src._src_mesh;

        hard_depth_offset = src.hard_depth_offset;
        opacity_func = src.opacity_func;

        CopyN(Alloc(_src_help, src._src_elms), src._src_help, _src_elms = src._src_elms);
        p = src.p;
    }
    PROFILE_STOP("Particles::create(C Particles &src)")
    return T;
}
/******************************************************************************/
Particles &Particles::source(C Shape &static_shape) {
    T._src_type = PARTICLE_STATIC_SHAPE;
    T._src_ptr = null;
    T._src_mesh = null;
    T.shape = static_shape;
    return T;
}
Particles &Particles::source(C Shape *dynamic_shape) {
    T._src_type = PARTICLE_DYNAMIC_SHAPE;
    T._src_ptr = dynamic_shape;
    T._src_mesh = null;
    T._src_elms = 1;
    return T;
}
Particles &Particles::source(C Shape *dynamic_shapes, Int shapes) {
    T._src_type = PARTICLE_DYNAMIC_SHAPES;
    T._src_ptr = dynamic_shapes;
    T._src_mesh = null;
    T._src_elms = Max(shapes, 0);
    return T;
}
Particles &Particles::source(C OrientP *dynamic_point) {
    T._src_type = PARTICLE_DYNAMIC_ORIENTP;
    T._src_ptr = dynamic_point;
    T._src_mesh = null;
    T._src_elms = 1;
    return T;
}
Particles &Particles::source(C AnimatedSkeleton *dynamic_skeleton, Bool ragdoll_bones_only) {
    PROFILE_START("Particles::source(C AnimatedSkeleton *dynamic_skeleton, Bool ragdoll_bones_only)")
    T._src_type = PARTICLE_DYNAMIC_SKELETON;
    T._src_ptr = dynamic_skeleton;
    T._src_mesh = null;
    T._src_elms = 0;
    Free(_src_help);
    if (ragdoll_bones_only)
        if (C Skeleton *skel = dynamic_skeleton->skeleton()) {
            REPA(skel->bones)
            if (skel->bones[i].flag & BONE_RAGDOLL)
                _src_elms++;
            Alloc(_src_help, _src_elms);
            _src_elms = 0;
            REPA(skel->bones)
            if (skel->bones[i].flag & BONE_RAGDOLL)
                _src_help[_src_elms++] = i;
        }
    PROFILE_STOP("Particles::source(C AnimatedSkeleton *dynamic_skeleton, Bool ragdoll_bones_only)")
    return T;
}
Particles &Particles::source(C MeshPtr &dynamic_mesh) {
    T._src_type = PARTICLE_DYNAMIC_MESH;
    T._src_ptr = null;
    T._src_mesh = dynamic_mesh;
    T._src_elms = 1;
    return T;
}
Particles &Particles::source(C MeshPtr &dynamic_mesh, C AnimatedSkeleton *dynamic_skeleton) {
    T._src_type = PARTICLE_DYNAMIC_MESH_SKELETON;
    T._src_ptr = dynamic_skeleton;
    T._src_mesh = dynamic_mesh;
    T._src_elms = 1;
    return T;
}
/******************************************************************************/
Flt Particles::opacity(Vec *pos) C {
    PROFILE_START("Particles::opacity(Vec *pos)")
    Vec vec = 0;
    Flt opacity = 0;
    Flt (*func)(Flt s) = GetOpacityFunc(T);
    int particleCount = p.elms();
    for (int i = 0; i < particleCount; ++i) {
        C Particle &particle = T.p[i];
        if (particle.life_max > EPS) {
            Flt o = func(particle.life / particle.life_max);
            vec += o * particle.pos;
            opacity += o;
        }
    }
    if (opacity > 0)
        vec /= opacity;
    opacity /= particleCount;
    if (pos)
        *pos = vec;
    PROFILE_STOP("Particles::opacity(Vec *pos)")
    return opacity;
}

/******************************************************************************/
RENDER_MODE Particles::renderMode() C { return D.colorPaletteAllow() ? _render_mode : RM_BLEND; }
Particles &Particles::setRenderMode() {
    _render_mode = (palette() ? (paletteIndex() ? RM_PALETTE1 : RM_PALETTE) : RM_BLEND);
    return T;
}
Particles &Particles::palette(Bool palette) {
    T._palette = palette;
    return setRenderMode();
}
Particles &Particles::paletteIndex(Byte palette_index) {
    T._palette_index = Sat(palette_index);
    return setRenderMode();
}
/******************************************************************************/
void Particles::reset(Int i) {
    PROFILE_START("Particles::reset(Int i)")
    if (InRange(i, p)) {
        Particle &p = T.p[i];
        if (reborn) {
            switch (_src_type) {
            case PARTICLE_STATIC_SHAPE:
                p.pos = (Random(shape, inside_shape) * matrix);
                break;
            case PARTICLE_DYNAMIC_SHAPE:
                p.pos = ((_src_ptr) ? Random(((Shape *)_src_ptr)[0], inside_shape) : matrix.pos);
                break;
            case PARTICLE_DYNAMIC_SHAPES:
                p.pos = ((_src_ptr && _src_elms) ? Random(((Shape *)_src_ptr)[Random(_src_elms)], inside_shape) : matrix.pos);
                break;
            case PARTICLE_DYNAMIC_ORIENTP:
                p.pos = ((_src_ptr) ? ((OrientP *)_src_ptr)->pos : matrix.pos);
                break;

            case PARTICLE_DYNAMIC_SKELETON: {
                if (C AnimatedSkeleton *anim_skel = (AnimatedSkeleton *)_src_ptr)
                    if (C Skeleton *skel = anim_skel->skeleton())
                        if (Int bones = anim_skel->minBones()) {
                            Int bone = ((_src_help && _src_elms) ? Min(bones, _src_help[Random(_src_elms)]) : Random(bones));
                            C SkelBone &skel_bone = skel->bones[bone];
                            p.pos = skel_bone.pos + skel_bone.dir * Random.f(skel_bone.length);
                            p.pos *= anim_skel->bones[bone]._matrix;
                            break;
                        }
                p.pos = matrix.pos;
            } break;

            case PARTICLE_DYNAMIC_MESH:
                p.pos = (_src_mesh ? Random(*_src_mesh) * matrix : matrix.pos);
                break;
            case PARTICLE_DYNAMIC_MESH_SKELETON:
                if (_src_mesh) {
                    if (_src_ptr)
                        p.pos = Random(*_src_mesh, *(AnimatedSkeleton *)_src_ptr);
                    else
                        p.pos = Random(*_src_mesh) * matrix;
                } else
                    p.pos = matrix.pos;
                break;

            default:
                p.pos = matrix.pos;
                break;
            }
            p.palette_y = (palette_image ? Random(palette_image->h()) : 0);
            p.image_index = Random(image_x_frames * image_y_frames);
            p.vel = Random(Ball(vel_random)) + vel_constant;
            p.ang_vel = Random.f(-ang_vel, ang_vel);
            p.radius = T.radius * ScaleFactor(Random.f(-radius_random, radius_random));
            Flt life = T.life * ScaleFactor(Random.f(-life_random, life_random));
            if (p.life_max > EPS && life)
                p.life = Frac(p.life - p.life_max, life); // if particle was created before, then set some initial life depending on what it has already
            else
                p.life = Random.f(life); // if we're creating particle for the first time, then set random initial life so it won't look like all particles created in the same time
            p.life_max = life;
        } else {
            p.life = p.life_max = 0; // if it shouldn't reborn then set life already as dead
        }
    }
    PROFILE_STOP("Particles::reset(Int i)")
}
Particles &Particles::reset() {
    REPA(p)
    reset(i);
    return T;
}
Particles &Particles::resetFull() {
    Bool temp = reborn;
    reborn = true;
    emitter_life = 0;
    reset();
    reborn = temp;
    return T;
}
/******************************************************************************/
Bool Particles::update(Flt dt) {
    PROFILE_START("Particles::update(Flt dt)")
    if (_src_type) {
        // life/death/fade
        if (emitter_life_max > 0) {
            emitter_life += dt;
            if (emitter_life >= emitter_life_max) {
                _fade = 0;
                return false;
            }
            Flt left = emitter_life_max - emitter_life;
            if (emitter_life < fade_in)
                _fade = emitter_life / fade_in;
            else // fade in
                if (left < fade_out)
                    _fade = left / fade_out;
                else           // fade out
                    _fade = 1; // middle
        } else {
            _fade = 1;
        }

        // matrix
        Matrix matrix;
        switch (_src_type) {
        default:
            matrix.identity();
            break;
        case PARTICLE_STATIC_SHAPE:
            matrix = shape.asMatrix() * T.matrix;
            break;
        case PARTICLE_DYNAMIC_MESH:
            matrix = T.matrix;
            break;
        case PARTICLE_DYNAMIC_SHAPE:
            if (_src_ptr)
                matrix = ((Shape *)_src_ptr)[0].asMatrix();
            else
                matrix.identity();
            break;
        case PARTICLE_DYNAMIC_SHAPES:
            if (_src_ptr && _src_elms)
                matrix = ((Shape *)_src_ptr)[Random(_src_elms)].asMatrix();
            else
                matrix.identity();
            break;
        case PARTICLE_DYNAMIC_ORIENTP:
            if (_src_ptr)
                matrix = *(OrientP *)_src_ptr;
            else
                matrix.identity();
            break;
        case PARTICLE_DYNAMIC_SKELETON:
            if (_src_ptr)
                matrix = ((AnimatedSkeleton *)_src_ptr)->matrix();
            else
                matrix.identity();
            break;
        case PARTICLE_DYNAMIC_MESH_SKELETON:
            if (_src_ptr)
                matrix = ((AnimatedSkeleton *)_src_ptr)->matrix();
            else
                matrix.identity();
            break;
        }

        // single particles
        Bool glue = (T.glue > EPS && _matrix_prev.x.any()),
             glue_full = (T.glue >= 1 - EPS);
        Matrix glue_transform;
        if (glue)
            GetTransform(glue_transform, _matrix_prev, matrix);
        Flt glue_mul, glue_add,
            damping = Pow(1 - T.damping, dt),
            growth = Pow(T.radius_growth, dt);
        Vec accel = T.accel * dt;
        if (glue && !glue_full) {
            glue_mul = Abs(T.glue - 0.5f) * -2 + 1; // 1-Abs(T.glue-0.5f)/0.5f
            glue_add = Sat(T.glue * 2 - 1);
        }
        REPA(p) {
            Particle &p = T.p[i];
            p.life += dt;
            if (p.life > p.life_max)
                reset(i);
            else {
                if (glue) {
                    if (glue_full)
                        p.pos *= glue_transform;
                    else
                        p.pos = Lerp(p.pos, p.pos * glue_transform, Sqr(1 - p.life / p.life_max) * glue_mul + glue_add);
                }
                p.vel *= damping;
                p.vel += accel;
                p.pos += p.vel * dt;
                p.radius *= growth;
            }
        }

        _matrix_prev = matrix;
        PROFILE_STOP("Particles::update(Flt dt)")
        return true;
    }
    PROFILE_STOP("Particles::update(Flt dt)")
    return false;
}
/******************************************************************************/
void Particles::drawSingleParticle(C Particle &p, bool animate, float opacity, float radius_scale, float offset_time, float offset_time2, bool offset, Randomizer &random, C ImagePtr &palette_image, int palette_image_w1, Image *render_color_palette, int render_color_palette_w1, float (*func)(Flt), bool &initialized) C {
    PROFILE_START("Particles::drawSingleParticle(C Particle &p, bool animate, float opacity, float radius_scale, float offset_time, float offset_time2, bool offset, Randomizer &random, C ImagePtr &palette_image, int palette_image_w1, Image *render_color_palette, int render_color_palette_w1, float (*func)(Flt), bool &initialized)");
    if (p.life_max <= EPS) {
        PROFILE_STOP("Particles::drawSingleParticle(C Particle &p, bool animate, float opacity, float radius_scale, float offset_time, float offset_time2, bool offset, Randomizer &random, C ImagePtr &palette_image, int palette_image_w1, Image *render_color_palette, int render_color_palette_w1, float (*func)(Flt), bool &initialized)");
        return;
    }
    float life = p.life / p.life_max;
    float radius = p.radius * radius_scale;
    Vec pos = p.pos;

    if (offset) {
        Vec offsetVec = random(Ball(offset_range), false);
        pos += offsetVec * Sin(random.f(PI2) + offset_time) +
               random(Ball(offset_range), false) * Sin(random.f(PI2) + offset_time2);
    }

    Color color = this->color;
    if (palette_image) {
        int x = RoundPos(life * palette_image_w1);
        color = ColorMul(color, palette_image->color(x, p.palette_y));
    }

    if (render_color_palette) {
        int x = render_color_palette_w1 - RoundPos(life * render_color_palette_w1);
        VecI4 p_color = {0, 0, 0, 0};

        REP(4) {
            Byte c = color.c[i];
            if (c) {
                C VecB4 &col = render_color_palette->pixB4(x, i);
                p_color.x += col.x * c;
                p_color.y += col.y * c;
                p_color.z += col.z * c;
                p_color.w += c;
            }
        }

        if (!p_color.w) {
            PROFILE_STOP("Particles::drawSingleParticle(C Particle &p, bool animate, float opacity, float radius_scale, float offset_time, float offset_time2, bool offset, Randomizer &random, C ImagePtr &palette_image, int palette_image_w1, Image *render_color_palette, int render_color_palette_w1, float (*func)(Flt), bool &initialized)");
            return;
        }
        color.r = p_color.x / p_color.w;
        color.g = p_color.y / p_color.w;
        color.b = p_color.z / p_color.w;
        color.a = 255;
    }

    if (!initialized) {
        initialized = true;
        if (animate)
            DrawAnimatedParticleBegin(*image, glow, motion_affects_alpha, image_x_frames, image_y_frames);
        else
            DrawParticleBegin(*image, glow, motion_affects_alpha);
    }

    if (animate)
        DrawAnimatedParticleAdd(color, opacity * func(life), radius, p.ang_vel * p.life, pos, p.vel, p.life * image_speed);
    else
        DrawParticleAdd(color, opacity * func(life), radius, p.ang_vel * p.life, pos, p.vel);
    PROFILE_STOP("Particles::drawSingleParticle(C Particle &p, bool animate, float opacity, float radius_scale, float offset_time, float offset_time2, bool offset, Randomizer &random, C ImagePtr &palette_image, int palette_image_w1, Image *render_color_palette, int render_color_palette_w1, float (*func)(Flt), bool &initialized)");
}

void Particles::draw(Flt opacity) C {
    PROFILE_START("Particles::draw(Flt opacity)");

    // Initial checks and setup
    if (!_src_type || !image || Renderer() != renderMode()) {
        PROFILE_STOP("Particles::draw(Flt opacity)");
        return;
    }

    opacity *= _fade;
    if (opacity <= 0) {
        PROFILE_STOP("Particles::draw(Flt opacity)");
        return;
    }

    bool initialized = false, offset = (offset_range > EPS);
    float offset_time = Time.time() * offset_speed, offset_time2 = offset_time * 0.7f, radius_scale = radiusScale(), (*func)(Flt s) = GetOpacityFunc(T);
    Randomizer random(UIDZero);
    Image *render_color_palette = nullptr;
    int render_color_palette_w1 = 0, palette_image_w1 = 0;

    // Set up the color palette if necessary
    if (Renderer() != _render_mode) {
        render_color_palette = &D._color_palette_soft[paletteIndex()];
        if (render_color_palette->h() < 4) {
            PROFILE_STOP("Particles::draw(Flt opacity)");
            return;
        }
        render_color_palette_w1 = render_color_palette->w() - 1;
    }

    if (palette_image)
        palette_image_w1 = palette_image->w() - 1;

    bool animate = (image_x_frames > 1 || image_y_frames > 1) && (image_speed > 0);
    REPA(p) {
        Vec2 screen_pos;
        if (PosToScreen(T.p[i].pos, screen_pos)) // Check if the particle is within the camera view
            drawSingleParticle(T.p[i], animate, opacity, radius_scale, offset_time, offset_time2, offset, random, palette_image, palette_image_w1, render_color_palette, render_color_palette_w1, func, initialized);
    }
    if (initialized)
        DrawParticleEnd();
    PROFILE_STOP("Particles::draw(Flt opacity)");
}
/******************************************************************************/
Bool Particles::save(File &f, Bool include_particles, CChar *path) C {
    f.putUInt(CC4_PRTC);
    f.cmpUIntV(is() ? 1 : 0); // version (0 is reserved for empty)
    if (is()) {
        f << reborn << motion_affects_alpha << _palette << _palette_index << inside_shape << glow << color << smooth_fade << _src_type
          << radius << radius_random << radius_growth << offset_range << offset_speed << life << life_random
          << glue << damping << ang_vel << vel_random << vel_constant << accel << matrix
          << emitter_life_max << emitter_life << _fade << fade_in << fade_out
          << radius_scale_base << radius_scale_time << image_x_frames << image_y_frames << image_speed << hard_depth_offset;
        f.putBool(include_particles);
        if (include_particles) {
            if (!p.saveRaw(f))
                return false;
        } else
            f.cmpUIntV(p.elms());                            // particles
        f.putAsset(image.id()).putAsset(palette_image.id()); // image names
        if (!shape.save(f))
            return false; // shape
    }
    return f.ok();
}
/******************************************************************************/
Bool Particles::load(File &f, CChar *path) {
    del();
    if (f.getUInt() == CC4_PRTC)
        switch (f.decUIntV()) {
        case 1: {
            f >> reborn >> motion_affects_alpha >> _palette >> _palette_index >> inside_shape >> glow >> color >> smooth_fade >> _src_type >> radius >> radius_random >> radius_growth >> offset_range >> offset_speed >> life >> life_random >> glue >> damping >> ang_vel >> vel_random >> vel_constant >> accel >> matrix >> emitter_life_max >> emitter_life >> _fade >> fade_in >> fade_out >> radius_scale_base >> radius_scale_time >> image_x_frames >> image_y_frames >> image_speed >> hard_depth_offset;
            if (f.getBool()) {
                if (!p.loadRaw(f))
                    goto error;
            } else
                p.setNumZero(f.decUIntV());              // particles
            image.require(f.getAssetID(), path);         // image name
            palette_image.require(f.getAssetID(), path); // image name
            if (!shape.load(f))
                goto error; // shape
            setRenderMode();
            if (f.ok())
                return true;
        } break;

        case 0: {
            if (f.ok())
                return true; // 0 is empty
        } break;
        }
error:
    del();
    return false;
}
/******************************************************************************/
Bool Particles::save(C Str &name, Bool include_particles) C {
    File f;
    if (f.write(name)) {
        if (save(f, include_particles, _GetPath(name)) && f.flush())
            return true;
        f.del();
        FDelFile(name);
    }
    return false;
}
/******************************************************************************/
Bool Particles::load(C Str &name) {
    File f;
    if (f.read(name))
        return load(f, _GetPath(name));
    del();
    return false;
}
/******************************************************************************/
// RAW PARTICLES
/******************************************************************************/
void RawParticles::zero() {
    motion_affects_alpha = true;
    glow = 0;
    _palette = false;
    _palette_index = 0;
    _particles = _max_particles = 0;
    _render_mode = RM_BLEND;
}
RawParticles::RawParticles() { zero(); }
/******************************************************************************/
RawParticles &RawParticles::del() {
    image.clear();
    _vb.del();
    _ib.del();
    zero();
    return T;
}
RawParticles &RawParticles::create(C ImagePtr &image) {
    del();

    T.image = image;

    return setRenderMode();
}
/******************************************************************************/
RawParticles &RawParticles::set(C Particle *particle, Int particles) {
    PROFILE_START("RawParticles::set(C Particle *particle, Int particles)")
    MAX(particles, 0);
    if (particles > _max_particles) // re-create buffers
    {
        _max_particles = particles;
        _vb.createNum(SIZE(Vtx3DBilb), particles * 4, false); // non-dynamic was always faster (DX10+: 96/15 fps, GL: 96/84 fps, GL ES: ?)

        // indexes
        if (_vb.vtxs() <= 0x10000)
            _ib.del();
        else // use 'IndBuf16384Quads'
        {    // custom Index Buffer needed
            if (!_ib.create(particles * (2 * 3), _vb.vtxs() <= 0x10000, false))
                return del();
            if (Ptr index = _ib.lock(LOCK_WRITE)) {
                Int p = 0;
                if (_ib.bit16()) {
                    U16 *ind = (U16 *)index;
                    REP(particles) {
                        ind[0] = p;
                        ind[1] = p + 1;
                        ind[2] = p + 3;
                        ind[3] = p + 3;
                        ind[4] = p + 1;
                        ind[5] = p + 2;
                        p += 4;
                        ind += 6;
                    }
                } else {
                    U32 *ind = (U32 *)index;
                    REP(particles) {
                        ind[0] = p;
                        ind[1] = p + 1;
                        ind[2] = p + 3;
                        ind[3] = p + 3;
                        ind[4] = p + 1;
                        ind[5] = p + 2;
                        p += 4;
                        ind += 6;
                    }
                }
                _ib.unlock();
            } else
                return del();
        }
    }

    // vertexes
    _particles = particles;
    if (Vtx3DBilb *v = (Vtx3DBilb *)_vb.lock(LOCK_WRITE)) {
        FREP(particles) {
            C Particle &p = particle[i];
            v[0].pos = v[1].pos = v[2].pos = v[3].pos = p.pos;
            v[0].vel_angle = v[1].vel_angle = v[2].vel_angle = v[3].vel_angle.set(p.vel.x, p.vel.y, p.vel.z, GPU_HALF_SUPPORTED ? AngleFull(p.angle) : p.angle);
            v[0].color = v[1].color = v[2].color = v[3].color = p.color;
            v[0].size = v[1].size = v[2].size = v[3].size = p.radius;
            SET_TEX(v[0].tex, TEX_ZERO, TEX_ONE);
            SET_TEX(v[1].tex, TEX_ONE, TEX_ONE);
            SET_TEX(v[2].tex, TEX_ONE, TEX_ZERO);
            SET_TEX(v[3].tex, TEX_ZERO, TEX_ZERO);
            v += 4;
        }
        _vb.unlock();
    } else {
        PROFILE_STOP("RawParticles::set(C Particle *particle, Int particles)")
        return del();
    }
    PROFILE_STOP("RawParticles::set(C Particle *particle, Int particles)")
    return T;
}
/******************************************************************************/
RENDER_MODE RawParticles::renderMode() C { return D.colorPaletteAllow() ? _render_mode : RM_BLEND; }
RawParticles &RawParticles::setRenderMode() {
    _render_mode = (palette() ? (paletteIndex() ? RM_PALETTE1 : RM_PALETTE) : RM_BLEND);
    return T;
}
/******************************************************************************/
RawParticles &RawParticles::palette(Bool palette) {
    T._palette = palette;
    return setRenderMode();
}
RawParticles &RawParticles::paletteIndex(Byte index) {
    T._palette_index = index;
    return setRenderMode();
}
/******************************************************************************/
void RawParticles::draw() C {
    PROFILE_START("RawParticles::draw()")
    if (image && _particles && Renderer() == renderMode()) {
        Renderer.wantDepthRead(); // !! call before 'SoftParticles' !!
        Shader *shader;
        Bool soft = SoftParticles();
        switch (Renderer()) {
        default:
            return;
        case RM_BLEND:
            shader = Sh.Particle[false][soft][0][motion_affects_alpha];
            D.alpha(Renderer.fastCombine() ? ALPHA_BLEND : ALPHA_RENDER_BLEND_FACTOR);
            D.alphaFactor(Color(0, 0, 0, glow));
            MaterialClear();
            if (glow)
                Renderer._has |= HAS_GLOW;
            break; // 'MaterialClear' must be called when changing 'D.alphaFactor'
        case RM_PALETTE:
        case RM_PALETTE1:
            shader = Sh.Particle[true][soft][0][motion_affects_alpha];
            D.alpha(ALPHA_ADD);
            break;
        }

        SetOneMatrix();
        D.depthOnWrite(true, false);
        D.cull(false);
        Sh.Img[0]->set(image());
#if GL // needed for iOS PVRTC Pow2 #ParticleImgPart
        Sh.ImgSize->setConditional(image->_part.xy);
#endif

        // set
        C IndBuf &ib = (T._ib.is() ? T._ib : IndBuf16384Quads);
        SetDefaultVAO();
        _vb.set();
        ib.set();
        D.vf(VI._vf3D_bilb.vf); // OpenGL requires setting 1)VAO 2)VB+IB 3)VF

        // draw
        shader->begin();
#if DX11
        D3DC->DrawIndexed(_particles * (2 * 3), 0, 0);
#elif GL
        glDrawElements(GL_TRIANGLES, _particles * (2 * 3), ib.bit16() ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, null);
#endif
    }
    PROFILE_STOP("RawParticles::draw()")
}
/******************************************************************************/
// MAIN
/******************************************************************************/
void ShutParticles() {
    ParticlesCache.del();
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
