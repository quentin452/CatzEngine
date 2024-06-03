/******************************************************************************/
#include "../../stdafx.h"
namespace EE {
/******************************************************************************/
SkyClass Sky;
Flt Atmosphere::ViewRange = 10000;
VecD Atmosphere::SunPos;
Memc<Atmosphere> Atmospheres;
/******************************************************************************/
static inline Vec4 SkyNightLightColor(Vec4 srgb_col) {
    srgb_col.xyz *= NightLightFactor(Sky.nightLight());
    if (LINEAR_GAMMA)
        srgb_col.xyz = SRGBToLinear(srgb_col.xyz);
    return srgb_col;
}

static void SetHorCol() {
    if (!Sky.nightLight())
        Sh.SkyHorCol->set(Sky.atmosphericHorizonColorD());
    else
        Sh.SkyHorCol->set(SkyNightLightColor(Sky.atmosphericHorizonColorS()));
}
static void SetSkyCol() {
    if (!Sky.nightLight())
        Sh.SkySkyCol->set(Sky.atmosphericSkyColorD());
    else
        Sh.SkySkyCol->set(SkyNightLightColor(Sky.atmosphericSkyColorS()));
}
/******************************************************************************/
SkyClass::SkyClass() {
#if 0 // there's only one 'SkyClass' global 'Sky' and it doesn't need clearing members to zero
  _night_light=0;
#endif
    frac(0.8f); // !! if changing default value, then also change in 'Environment.Sky' !!
    _dns_exp = 1;
    _hor_exp = 3.5f;                              // !! if changing default value, then also change in 'Environment.Sky' !!
    _sky_col_l.set(0.032f, 0.113f, 0.240f, 1.0f); // #DefaultSkyValue
    _hor_col_l.set(0.093f, 0.202f, 0.374f, 1.0f); // #DefaultSkyValue
    _stars_m.identity();
    _box_blend = 0.5f;
    atmosphericPrecision(!MOBILE);
}
SkyClass &SkyClass::del() {
    _mshr.del();
    REPAO(_image).clear();
    _stars.clear();
    return T;
}
SkyClass &SkyClass::create() {
    SetHorCol();
    SetSkyCol();
    Sh.SkyBoxBlend->set(_box_blend);
    Sh.SkyStarOrn->set(_stars_m);
    Flt temp = _hor_exp;
    _hor_exp = -1;
    atmosphericHorizonExponent(temp); // set -1 to force reset
    temp = _dns_exp;
    _dns_exp = -1;
    atmosphericDensityExponent(temp); // set -1 to force reset

    // SKY_MESH_MIN_DIST
    MeshBase mshb;
    mshb.createIco(Ball(1), MESH_NONE, 3); // res 3 gives 'dist'=0.982246876
    _mshr.create(mshb.reverse());
#if DEBUG && 0 // calculate actual distance
    Flt dist = 1;
    C Vec *pos = mshb.vtx.pos();
    REPA(mshb.tri) {
        C VecI &t = mshb.tri.ind(i);
        MIN(dist, Dist(VecZero, Tri(pos[t.x], pos[t.y], pos[t.z])));
    }
    REPA(mshb.quad) {
        C VecI4 &q = mshb.quad.ind(i);
        MIN(dist, Dist(VecZero, Quad(pos[q.x], pos[q.y], pos[q.z], pos[q.w])));
    }
#endif
    return T;
}
/******************************************************************************/
Bool SkyClass::wantDepth() C { return frac() < 1; }
/******************************************************************************/
SkyClass &SkyClass::clear() {
    _is = false;
    return T;
}
SkyClass &SkyClass::atmospheric() {
    T._is = true;
    T._image[0] = null;
    T._image[1] = null;
    return T;
}
SkyClass &SkyClass::skybox(C ImagePtr &image) {
    T._is = (image != null);
    T._image[0] = image;
    T._image[1] = null;
    return T;
}
SkyClass &SkyClass::skybox(C ImagePtr &a, C ImagePtr &b) {
    T._is = (a || b);
    if (a && b && a != b) {
        T._image[0] = a;
        T._image[1] = b;
    } else if (a) {
        T._image[0] = a;
        T._image[1] = null;
    } else if (b) {
        T._image[0] = b;
        T._image[1] = null;
    } else {
        T._image[0] = null;
        T._image[1] = null;
    }
    return T;
}
/******************************************************************************/
Vec4 SkyClass::atmosphericHorizonColorS() C { return LinearToSRGB(atmosphericHorizonColorL()); }
Vec4 SkyClass::atmosphericSkyColorS() C { return LinearToSRGB(atmosphericSkyColorL()); }

SkyClass &SkyClass::atmosphericHorizonColorS(C Vec4 &color_s) { return atmosphericHorizonColorL(SRGBToLinear(color_s)); }
SkyClass &SkyClass::atmosphericSkyColorS(C Vec4 &color_s) { return atmosphericSkyColorL(SRGBToLinear(color_s)); }

SkyClass &SkyClass::frac(Flt frac) {
    SAT(frac);
    T._frac = frac;
    return T;
}
SkyClass &SkyClass::nightLight(Flt intensity) {
    SAT(intensity);
    if (intensity != T._night_light) {
        T._night_light = intensity;
        SetHorCol();
        SetSkyCol();
    }
    return T;
}
SkyClass &SkyClass::atmosphericHorizonExponent(Flt exp) {
    MAX(exp, 0);
    if (exp != T._hor_exp) {
        T._hor_exp = exp;
        Sh.SkyHorExp->set(Max(T._hor_exp, EPS_GPU));
    }
    return T;
} // avoid zero in case "Pow(1-Sat(inTex.y), SkyHorExp)" in shader would cause NaN or slow-downs
SkyClass &SkyClass::atmosphericHorizonColorL(C Vec4 &color_l) {
    Flt alpha = Sat(color_l.w);
    if (color_l.xyz != T._hor_col_l.xyz || alpha != T._hor_col_l.w) {
        T._hor_col_l.set(color_l.xyz, alpha);
        SetHorCol();
    }
    return T;
} // alpha must be saturated
SkyClass &SkyClass::atmosphericSkyColorL(C Vec4 &color_l) {
    Flt alpha = Sat(color_l.w);
    if (color_l.xyz != T._sky_col_l.xyz || alpha != T._sky_col_l.w) {
        T._sky_col_l.set(color_l.xyz, alpha);
        SetSkyCol();
    }
    return T;
} // alpha must be saturated
SkyClass &SkyClass::skyboxBlend(Flt blend) {
    SAT(blend);
    if (blend != T._box_blend) {
        T._box_blend = blend;
        Sh.SkyBoxBlend->set(T._box_blend);
    }
    return T;
}
SkyClass &SkyClass::atmosphericStars(C ImagePtr &cube) {
    T._stars = cube;
    return T;
}
SkyClass &SkyClass::atmosphericStarsOrientation(C Matrix3 &orn) {
    {
        T._stars_m = orn;
        Sh.SkyStarOrn->set(T._stars_m);
    }
    return T;
}
SkyClass &SkyClass::atmosphericPrecision(Bool per_pixel) {
    T._precision = per_pixel;
    return T;
}
SkyClass &SkyClass::atmosphericDensityExponent(Flt exp) {
    SAT(exp);
    if (exp != T._dns_exp) {
        T._dns_exp = exp;
        /* shader uses the formula based on "Flt AccumulatedDensity(Flt density, Flt range) {return 1-Pow(1-density, range);}"
           "1-Pow(SkyDnsExp, alpha)" but that gives the range 0..(1-SkyDnsExp), however we need it normalized, so:
           (1-Pow(SkyDnsExp, alpha)) / (1-SkyDnsExp) gives the range 0..1
             -Pow(SkyDnsExp, alpha) / (1-SkyDnsExp) + 1/(1-SkyDnsExp)
              Pow(SkyDnsExp, alpha) * -(1/(1-SkyDnsExp)) + 1/(1-SkyDnsExp)
              Pow(SkyDnsExp, alpha) *   mul              + add
        */
        Flt v = 1 - exp;
        if (v)
            v = 1 / v;
        Sh.SkyDnsExp->set(Max(T._dns_exp, EPS_GPU)); // avoid zero in case "Pow(0, alpha)" in shader would cause NaN or slow-downs
        Sh.SkyDnsMulAdd->set(Vec2(-v, v));
    }
    return T;
}
/******************************************************************************/
void SkyClass::setFracMulAdd() {
    // !! in this method we use 'SkyFracMulAdd' as Object Opacity, and not Sky Opacity, so we use "1-sky_opacity" (reversed compared to drawing the sky) !!
    if (isActual()) {
        Flt from, to;
        Bool can_read_depth = Renderer.safeCanReadDepth();                // use 'safe' version because 'Renderer._ds' can be null here (for example when using RS_REFLECTION)
        from = (can_read_depth ? D.viewRange() * frac() : D.viewRange()); // we're using fraction only if we have depth access
        to = D.viewRange();
        MIN(from, to - EPS_SKY_MIN_LERP_DIST); // make sure there's some distance between positions to avoid floating point issues, move 'from' instead of 'to' to make sure we always have zero opacity at the end

        // Flt obj_opacity=Length(O.pos)*SkyFracMulAdd.x+SkyFracMulAdd.y;
        //               1=       from  *SkyFracMulAdd.x+SkyFracMulAdd.y;
        //               0=       to    *SkyFracMulAdd.x+SkyFracMulAdd.y;
        Vec2 mul_add;
        mul_add.x = 1 / (from - to);
        mul_add.y = -to * mul_add.x;
        Sh.SkyFracMulAdd->set(mul_add);
    } else {
        Sh.SkyFracMulAdd->set(Vec2(0, 1));
    }
}
/******************************************************************************/
INLINE Shader *SkyTF(Int textures, Bool dither, Bool cloud) {
    Shader *&s = Sh.SkyTF[textures - 1][dither][cloud];
    if (SLOW_SHADER_LOAD && !s)
        s = Sh.getSkyTF(textures, dither, cloud);
    return s;
}
INLINE Shader *SkyT(Int multi_sample, Int textures, Bool dither, Bool cloud) {
    Shader *&s = Sh.SkyT[multi_sample][textures - 1][dither][cloud];
    if (SLOW_SHADER_LOAD && !s)
        s = Sh.getSkyT(multi_sample, textures, dither, cloud);
    return s;
}
INLINE Shader *SkyAF(Bool per_vertex, Bool stars, Bool dither, Bool cloud) {
    Shader *&s = Sh.SkyAF[per_vertex][stars][dither][cloud];
    if (SLOW_SHADER_LOAD && !s)
        s = Sh.getSkyAF(per_vertex, stars, dither, cloud);
    return s;
}
INLINE Shader *SkyA(Int multi_sample, Bool per_vertex, Bool density, Bool stars, Bool dither, Bool cloud) {
    Shader *&s = Sh.SkyA[multi_sample][per_vertex][density][stars][dither][cloud];
    if (SLOW_SHADER_LOAD && !s)
        s = Sh.getSkyA(multi_sample, per_vertex, density, stars, dither, cloud);
    return s;
}
/******************************************************************************/
// ATMOSPHERE
/******************************************************************************/
INLINE Shader *AtmosphereShader(Bool flat, Bool dither, Int multi_sample) {
    Shader *&s = Sh.Atmosphere[multi_sample][flat][dither];
    if (SLOW_SHADER_LOAD && !s)
        s = Sh.getAtmosphere(multi_sample, flat, dither);
    return s;
}
static inline Flt AtmosphereAltScaleRay(Flt height) { return (-50 / 8.0f) / height; } // - because of Exp, 50 was chosen to fill entire sphere with some color, 8.0 real physical value, 'height' proportional to atmosphere height
static inline Flt AtmosphereAltScaleMie(Flt height) { return (-50 / 1.2f) / height; } // - because of Exp, 50 was chosen to fill entire sphere with some color, 1.2 real physical value, 'height' proportional to atmosphere height
inline void Atmosphere::drawDo(Int multi_sample, Bool dither) C {
    Flt draw_r = r / SKY_MESH_MIN_DIST;
    Flt dist2 = Dist2(ActiveCam.matrix.pos, pos);                                            // use 'ActiveCam' instead of 'CamMatrix' because it's not affected by eyes
    Bool flat = (dist2 <= Sqr(draw_r + FrustumMain.view_quad_max_dist + D.eyeDistance_2())); // use flat if camera intersects with mesh

    MatrixM matrix;
    if (!flat) {
        matrix.setScalePos(-draw_r, pos); // reverse faces because 'Sky._mshr' is reversed
        Sky._mshr.set();
        D.depth(true);
        D.cull(true);
    } // else D.depth(false); already handled in "shader->draw" below

    Sh.AtmospherePlanetRadius->set(planet_radius);
    // Sh.AtmosphereHeight       ->set(height);
    Sh.AtmosphereRadius->set(r);
    Sh.AtmosphereLightPos->set(!(Vec(SunPos - pos) * CamMatrixInv.orn()));
    Sh.AtmosphereAltScaleRay->set(AtmosphereAltScaleRay(height));
    Sh.AtmosphereAltScaleMie->set(AtmosphereAltScaleMie(height));
    Sh.AtmosphereMieExtinction->set(mie_extinction);
    Sh.AtmosphereLightScale->set(light_scale);
    Sh.AtmosphereFogReduce->set(fog_reduce);
    Sh.AtmosphereFogReduceDist->set(fog_reduce_dist);
    Sh.AtmosphereDarken->set(darken);

    Shader *shader = AtmosphereShader(flat, dither, 0),
           *shader_multi = (multi_sample ? AtmosphereShader(flat, dither, multi_sample) : null);

    REPS(Renderer._eye, Renderer._eye_num) {
        Renderer.setEyeViewportCam();
        Sh.AtmospherePos->set(Vec(CamMatrix.pos - pos) * CamMatrixInv.orn());
        if (!flat)
            SetFastMatrix(matrix); // set these after 'setEyeViewportCam'. position that we set is for camera relative to ball. because shader assumes ball is at Vec(0,0,0) and the position that we specify is camera position relative to ball in view space
        if (shader_multi) {
            D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
            if (flat)
                shader_multi->draw();
            else {
                shader_multi->begin();
                Sky._mshr.draw();
            }
            D.stencilRef(0);
        } // call this first to set stencil, reset stencil ref for call below
        if (flat)
            shader->draw();
        else {
            shader->begin();
            Sky._mshr.draw();
        } // call this next
    }
}
/******************************************************************************/
static Int Compare(C Atmosphere &a, C Atmosphere &b) {
    return Compare(Dist2(a.pos, ActiveCam.matrix.pos), Dist2(b.pos, ActiveCam.matrix.pos));
}
void SkyClass::draw() {
    if (isActual()) {
        Shader *shader, *shader_multi = null;
        Bool blend,
            density = (atmosphericDensityExponent() < 1 - EPS_GPU),
            dither = (D.dither() && !Renderer._col->highPrecision()),
            vertex = !_precision,
            stars = ((_stars != null) && (_hor_col_l.w < 1 - EPS_COL_1 || _sky_col_l.w < 1 - EPS_COL_1)),
            cloud = (Clouds.draw && Clouds.layered.merge_with_sky && Clouds.layered.layers() && Clouds.layered.layer[0].image && Clouds.layered.layer[0].color_l.w && (Clouds.layered.draw_in_mirror || !Renderer.mirror()));
        Int tex = ((_image[0] != null) + (_image[1] != null)),
            multi = (Renderer._col->multiSample() ? ((Renderer._cur_type == RT_DEFERRED) ? 1 : 2) : 0);
        Flt from = (Renderer.canReadDepth() ? D.viewRange() * frac() : D.viewRange()), // we're using fraction only if we have depth access
            to = D.viewRange();
        blend = (from < to - EPS_SKY_MIN_LERP_DIST); // set blend mode if 'from' is far from 'to', and yes use < and not <= in case of precision issues for big values

        if (tex) {
            if (blend) {
                shader = SkyT(0, tex, dither, cloud);
                if (multi)
                    shader_multi = SkyT(multi, tex, dither, cloud);
            } else
                shader = SkyTF(tex, dither, cloud);
        } else {
            if (blend) {
                shader = SkyA(0, vertex, density, stars, dither, cloud);
                if (multi)
                    shader_multi = SkyA(multi, vertex, density, stars, dither, cloud);
            } else
                shader = SkyAF(vertex, stars, dither, cloud);
        }

        // set shader parameters
        if (tex) {
            Sh.Cub[0]->set(_image[0]());
            Sh.Cub[1]->set(_image[1]());
        } else if (stars) {
            Sh.Cub[0]->set(_stars());
        }

        if (AstrosDraw && Sun.is()) {
            Sh.SkySunHighlight->set(Vec2(Sun.highlight_front, Sun.highlight_back));
            Sh.SkySunPos->set(Sun.pos);
        } else {
            Sh.SkySunHighlight->set(Vec2Zero);
        }

        if (cloud)
            Clouds.layered.commit();

        Bool ds = true;
        Flt sky_ball_mesh_size;
        if (blend) {
            // Flt sky_opacity=Length(O.pos)*SkyFracMulAdd.x+SkyFracMulAdd.y;
            //               0=       from  *SkyFracMulAdd.x+SkyFracMulAdd.y;
            //               1=       to    *SkyFracMulAdd.x+SkyFracMulAdd.y;
            Vec2 mul_add;
            mul_add.x = 1 / (to - from);
            mul_add.y = -from * mul_add.x;
            Sh.SkyFracMulAdd->set(mul_add);

            sky_ball_mesh_size = from;
            // sky_ball_mesh_size-=DepthError(D.viewFrom(), D.viewRange(), sky_ball_mesh_size, FovPerspective(D.viewFovMode()), Renderer._ds->hwTypeInfo().d); // draw smaller by DepthError to avoid depth precision issues
            if (sky_ball_mesh_size * SKY_MESH_MIN_DIST <= Frustum.view_quad_max_dist) {
                sky_ball_mesh_size = to;
                ds = false;
            } // if the closest point on the mesh surface is in touch with the view quad, it means that the ball would not render fully, we have to render it with full size and with depth test disabled
        } else
            sky_ball_mesh_size = to;
#if !REVERSE_DEPTH                                            // for low precision depth we need to make sure that sky ball mesh is slightly smaller than view range, to avoid mesh clipping, this was observed on OpenGL with viewFrom=0.05, viewRange=1024, Cam.yaw=0, Cam.pitch=PI_2
        MIN(sky_ball_mesh_size, to * EPS_SKY_MIN_VIEW_RANGE); // alternatively we could try using 'D.depthClip'
#endif
        // !! THIS MUST NOT MODIFY 'Renderer._alpha' BECAUSE THAT WOULD DISABLE SUN RAYS !!
        Renderer.set(Renderer._col, Renderer._ds, true, blend ? NEED_DEPTH_READ : NO_DEPTH_READ); // specify correct mode because without it the sky may cover everything completely
        D.alpha(blend ? ALPHA_RENDER_BLEND : ALPHA_NONE);
        D.depthOnWriteFunc(ds, false, FUNC_LESS_EQUAL); // to make sure we draw at the end of viewRange
                                                        // D.cull            (true); ignore changing culling, because we're inside the sky ball, so we will always see its faces, we could potentially set false (to ignore overhead on the GPU for cull testing if any) however we choose to just ignore it to reduce GPU state changes on the CPU which are probably more costly
        _mshr.set();
        SetOneMatrix(MatrixM(sky_ball_mesh_size, CamMatrix.pos)); // normally we have to set matrixes after 'setEyeViewportCam', however since matrixes are always relative to the camera, and here we set exactly at the camera position, so the matrix will be the same for both eyes
        REPS(Renderer._eye, Renderer._eye_num) {
            Renderer.setEyeViewportCam();
            if (shader_multi) {
                D.depth((multi == 1) ? false : ds);
                D.stencil(STENCIL_MSAA_TEST, STENCIL_REF_MSAA);
                shader_multi->begin();
                _mshr.draw();
                D.stencilRef(0);
                D.depth(ds);
            } // call this first to set stencil, MS edges for deferred must not use depth testing, reset stencil ref and depth for call below
            shader->begin();
            _mshr.draw(); // call this next
        }
        D.depthOnWriteFunc(false, true, FUNC_DEFAULT);
        D.stencil(STENCIL_NONE);
    }

    if (Atmospheres.elms()) {
        Atmospheres.sort(Compare); // sort from closest (i=0 first), to farthest (last)
        Renderer.set(Renderer._col, Renderer._ds, true, NEED_DEPTH_READ);
        D.alpha(ALPHA_RENDER_MERGE);
        D.depthWriteFunc(false, FUNC_LESS);
        SetMatrixCount(); // needed for drawing mesh

        Flt to = D.viewRange(),
            from = Min(D.viewRange() * frac(), to - EPS_SKY_MIN_LERP_DIST);
        // Flt sky_opacity=Length(O.pos)*MulAdd.x+MulAdd.y;
        //               0=       from  *MulAdd.x+MulAdd.y;
        //               1=       to    *MulAdd.x+MulAdd.y;
        Vec2 mul_add;
        mul_add.x = 1 / (to - from);
        mul_add.y = -from * mul_add.x;
        Sh.SkyFracMulAdd->set(mul_add);
        Sh.AtmosphereViewRange->set(Atmosphere::ViewRange);

        Int multi_sample = (Renderer._col->multiSample() ? ((Renderer._cur_type == RT_DEFERRED) ? 1 : 2) : 0);
        Bool dither = (D.dither() && !Renderer._col->highPrecision());
        REPAO(Atmospheres).drawDo(multi_sample, dither);

        // restore default
        D.depthOnWriteFunc(false, true, FUNC_DEFAULT);
        D.stencil(STENCIL_NONE);
    }
}
/******************************************************************************/
void Atmosphere::draw() C {
    DEBUG_ASSERT(Renderer() == RM_PREPARE, "'Atmosphere.draw' called outside of RM_PREPARE");
    if (Frustum(T) && Renderer.firstPass())
        Atmospheres.add(T);
}
/******************************************************************************/
const Vec RayleighScattering = Vec(5.2091275314786692e-06, 1.2171661977460255e-05, 2.9715971624658822e-05); // Vec(5.802, 13.558, 33.1)*0.000001;
const Flt MieScattering = 3.996 * 0.000001;
/******************************************************************************/
Flt RayleighPhase(Flt angle_cos) {
    return 3 / (16 * PI) * (1 + angle_cos * angle_cos);
}
Flt MiePhase(Flt angle_cos) {
    const Flt g = 0.8;

    Flt num = 3 / (8 * PI) * (1 - g * g) * (1 + angle_cos * angle_cos);
    Flt denom = (2 + g * g) * Pow(1 + g * g - 2 * g * angle_cos, 1.5f);

    return num / denom;
}
/******************************************************************************/
void Atmosphere::scattering(Flt height, Vec &rayleigh_scattering, Flt &mie_scattering, Vec &extinction) C {
    Flt altitude = Max(0, height - planet_radius);
    Flt rayleigh_density = Exp(altitude * AtmosphereAltScaleRay(T.height));
    Flt mie_density = Exp(altitude * AtmosphereAltScaleMie(T.height));

    rayleigh_scattering = RayleighScattering * rayleigh_density;
    // Flt rayleigh_absorption=RayleighAbsorption*rayleigh_density;

    mie_scattering = MieScattering * mie_density;
    Flt mie_extinction = T.mie_extinction * mie_density;

    // extinction=rayleigh_scattering+(rayleigh_absorption+mie_scattering+mie_absorption);
    extinction = rayleigh_scattering + mie_extinction;

    // Vec ozone_absorption=OzoneAbsorption*Max(0, 1-Abs(altitude_km-25)/15); extinction+=ozone_absorption;
}
/******************************************************************************/
// !! THIS IGNORES 'light_scale', 'fog_reduce' AND MIE SCATTERING  !!
Vec Atmosphere::calcCol(C Vec &pos, C Vec &ray, C Vec &sun) C {
    Flt start, end;
    // Flt fog_factor; // fake factor that removes fog and mie highlights on objects (this is faster than shadow testing per sample)
    {
        Flt b = Dot(pos, ray);
        Flt c = pos.length2() - Sqr(T.r);
        Flt d = b * b - c;
        Flt atmos_start = -b - Sqrt(d),
            atmos_end = -b + Sqrt(d);
        start = Max(atmos_start, 0);
        end = atmos_end;
        if (d <= 0 || end <= start)
            return 0; // no atmosphere intersection
                      // Flt factor=1 ? (end-start)/(atmos_end-start) : end/atmos_end; // proportion of pixel_pos_cam_dist to atmos_end
        // fog_factor=1-fog_reduce*(1-factor)*Sat(1-end/fog_reduce_dist);
    }

    Flt angle_cos = Dot(ray, sun);
    Flt rayleigh_phase = RayleighPhase(angle_cos);
    Flt mie_phase = MiePhase(angle_cos);

    const Int steps = 16; // 16 is smallest value that looks as good as 256
    Vec lum = 0, transmittance = 1;
    Flt t0 = start, mul = end - start;
    for (Int i = 1; i <= steps; i++) {
        Flt t1 = Flt(i) / steps;
        if (1)
            t1 *= t1; // focus on samples closer to camera, this increases precision
        t1 = t1 * mul + start;
        Flt t = Avg(t0, t1), dt = t1 - t0;
        t0 = t1;

        Vec sample_pos = pos + t * ray;
        Flt height = sample_pos.length();

        Vec rayleigh_scattering, extinction;
        Flt mie_scattering;
        scattering(height, rayleigh_scattering, mie_scattering, extinction);

#if 1
        Flt sun_zenith_angle_cos = Dot(sun, sample_pos) / height; // Dot(sun, sample_pos/height) -1..1

        // !! IF CHANGING FORMULA THEN ALSO CHANGE #AtmosphereFormula !!
        // Flt x=sun_zenith_angle_cos*0.5+0.5;
        Flt mul = 0.6;
        Flt x = Max(0, sun_zenith_angle_cos * mul + (1 - mul)); // mul=0.5 (balance) .. 1.0 (more night). x=0..1
        Flt sun_transmittance = SmoothCube(x);

        Vec scattering = (rayleigh_scattering * rayleigh_phase /* + mie_scattering*mie_phase*/) * sun_transmittance;
#else
        Vec up = sample_pos / height;
        Flt sun_zenith_angle_cos = Dot(sun, up);
        Vec2 uv = Vec2(sun_zenith_angle_cos * 0.5 + 0.5, (height - AtmospherePlanetRadius) / AtmosphereHeight);

        Vec sun_transmittance = TexLod(SkyA, uv).rgb; // Lod needed for clamp
        Vec multi_scatter = TexLod(SkyB, uv).rgb;     // Lod needed for clamp

        Vec rayleigh_scattering_1 = rayleigh_scattering * (rayleigh_phase * sun_transmittance + multi_scatter);
        Vec mie_scattering_1 = mie_scattering * (mie_phase * sun_transmittance + multi_scatter);
        Vec scattering = rayleigh_scattering_1 + mie_scattering_1;
#endif

        Vec sample_transmittance = Exp(-dt * extinction);
#if 1 // faster but may have issues in certain cases with low 'steps', have to test if results are satisfactory
        lum += transmittance * scattering * dt;
#else
        Vec scattering_integral = (scattering - scattering * sample_transmittance) / extinction;
        lum += transmittance * scattering_integral;
#endif
        transmittance *= sample_transmittance;
    }
    // lum*=fog_factor*light_scale;
    return lum;
}
Vec Atmosphere::calcCol(Flt look_angle) C {
    return calcCol(Vec(0, planet_radius, 0), Vec(0, Cos(look_angle), Sin(look_angle)), Vec(0, 1, 0));
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
