// TODO ADD PROFILE_START AND PROFILE_STOP PROFILERS
/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
#define CC4_ENV CC4('E', 'N', 'V', 0)
/******************************************************************************/
DEFINE_CACHE(Environment, Environments, EnvironmentPtr, "Environment");
/******************************************************************************/
// AMBIENT
/******************************************************************************/
void Environment::Ambient::set() C {
    D.ambientColorS(on ? color_s : VecZero);
    D.nightShadeColorS(on ? night_shade_color_s : VecZero);
}
void Environment::Ambient::get() {
    color_s = D.ambientColorS();
    night_shade_color_s = D.nightShadeColorS();
    on = (color_s.any() || night_shade_color_s.any());
}
void Environment::Ambient::reset() {
    on = true;
    color_s = 0.366f;
    night_shade_color_s.zero();
} // #DefaultAmbientValue

Bool Environment::Ambient::save(File &f, CChar *path) C {
    f.cmpUIntV(1); // version
    f << on << color_s << night_shade_color_s;
    return f.ok();
}
Bool Environment::Ambient::load(File &f, CChar *path) {
    switch (f.decUIntV()) {
    case 1: {
        f >> on >> color_s >> night_shade_color_s;
        if (f.ok())
            return true;
    } break;

    case 0: {
        f >> on >> color_s;
        night_shade_color_s.zero();
        if (f.ok())
            return true;
    } break;
    }
    reset();
    return false;
}
/******************************************************************************/
// BLOOM
/******************************************************************************/
void Environment::Bloom::set() C {
    D.bloomOriginal(on ? original : 1).bloomScale(on ? scale : 0).bloomCut(cut).bloomGlow(on ? glow : 0);
}
void Environment::Bloom::get() {
    original = D.bloomOriginal();
    scale = D.bloomScale();
    cut = D.bloomCut();
    glow = D.bloomGlow();
    on = !(Equal(original, 1) && Equal(scale, 0));
}
void Environment::Bloom::reset() {
    on = true;
    original = 1.0f;
    scale = 0.8f;
    cut = 0.3f;
    glow = 1.0f;
}

Bool Environment::Bloom::save(File &f, CChar *path) C {
    f.cmpUIntV(4); // version
    f << on << original << scale << cut << glow;
    return f.ok();
}
Bool Environment::Bloom::load(File &f, CChar *path) {
    Bool saturate, half, maximum;
    Byte blurs;
    Flt contrast;
    switch (f.decUIntV()) {
    case 4: {
        f >> on >> original >> scale >> cut >> glow;
        if (f.ok())
            return true;
    } break;

    case 3: {
        f >> on >> half >> maximum >> blurs >> original >> scale >> cut;
        glow = 1;
        scale *= 2;
        if (f.ok())
            return true;
    } break;

    case 2: {
        f >> on >> half >> saturate >> maximum >> blurs >> original >> scale >> cut;
        glow = 1;
        scale *= 2;
        if (f.ok())
            return true;
    } break;

    case 1: {
        f >> on >> half >> maximum >> blurs >> original >> scale >> cut >> contrast;
        glow = 1;
        scale *= 2;
        if (f.ok())
            return true;
    } break;

    case 0: {
        f >> on >> half >> maximum >> blurs >> original >> scale >> cut >> contrast;
        glow = 1;
        scale *= 4;
        if (f.ok())
            return true;
    } break;
    }
    reset();
    return false;
}
/******************************************************************************/
// CLOUDS
/******************************************************************************/
void Environment::Clouds::set() C {
    Int active = 0;
    FREPA(layers)
    if (layers[i].image) {
        C Layer &src = layers[i];
        LayeredClouds::Layer &dest = ::Clouds.layered.layer[active++];
        dest.scale = src.scale;
        dest.velocity = src.velocity;
        dest.colorS(src.color_s);
        dest.image = src.image;
    }
    ::Clouds.layered.set(on ? active : false);
    ::Clouds.layered.scaleY(vertical_scale);
}
void Environment::Clouds::get() {
    REPA(layers) {
        C LayeredClouds::Layer &src = ::Clouds.layered.layer[i];
        Layer &dest = layers[i];
        dest.scale = src.scale;
        dest.velocity = src.velocity;
        dest.color_s = src.colorS();
        dest.image = src.image;
    }
    on = (::Clouds.layered.layers() > 0);
    vertical_scale = ::Clouds.layered.scaleY();
}
void Environment::Clouds::reset() {
    on = true;
    vertical_scale = 1.05f;
    REPA(layers) {
        Layer &layer = layers[i];
        layer.color_s = 1;
        layer.image = null;
    }
    layers[0].scale = 1.0f / 2.8f;
    layers[0].velocity = 0.010f;
    layers[1].scale = 1.0f / 2.4f;
    layers[1].velocity = 0.008f;
    layers[2].scale = 1.0f / 2.0f;
    layers[2].velocity = 0.006f;
    layers[3].scale = 1.0f / 1.6f;
    layers[3].velocity = 0.004f;
}

Bool Environment::Clouds::save(File &f, CChar *path) C {
    f.cmpUIntV(0);                                             // version
    f << on << vertical_scale << Flt(4 /*ray_mask_contrast*/); // !! in the future don't save this !!
    FREPA(layers) {
        C Layer &layer = layers[i];
        f << layer.scale << layer.velocity << layer.color_s;
        f._putStr(layer.image.name(path));
    }
    return f.ok();
}
Bool Environment::Clouds::load(File &f, CChar *path) {
    switch (f.decUIntV()) {
    case 0: {
        Flt ray_mask_contrast;
        f >> on >> vertical_scale >> ray_mask_contrast;
        FREPA(layers) {
            Layer &layer = layers[i];
            f >> layer.scale >> layer.velocity >> layer.color_s;
            layer.image.require(f._getStr(), path);
        }
        if (f.ok())
            return true;
    } break;
    }
    reset();
    return false;
}
/******************************************************************************/
// FOG
/******************************************************************************/
void Environment::Fog::set() C {
    ::Fog.draw = on;
    ::Fog.affect_sky = affect_sky;
    ::Fog.density = density;
    ::Fog.colorS(color_s);
}
void Environment::Fog::get() {
    on = ::Fog.draw;
    affect_sky = ::Fog.affect_sky;
    density = ::Fog.density;
    color_s = ::Fog.colorS();
}
void Environment::Fog::reset() {
    on = false;
    affect_sky = false;
    density = 0.02f;
    color_s = 0.5f;
}

Bool Environment::Fog::save(File &f, CChar *path) C {
    f.cmpUIntV(0); // version
    f << on << affect_sky << density << color_s;
    return f.ok();
}
Bool Environment::Fog::load(File &f, CChar *path) {
    switch (f.decUIntV()) {
    case 0: {
        f >> on >> affect_sky >> density >> color_s;
        if (f.ok())
            return true;
    } break;
    }
    reset();
    return false;
}
/******************************************************************************/
// SKY
/******************************************************************************/
void Environment::Sky::set() C {
    ::Sky.frac(frac)
        .atmosphericDensityExponent(atmospheric_density_exponent)
        .atmosphericHorizonExponent(atmospheric_horizon_exponent)
        .atmosphericHorizonColorS(atmospheric_horizon_color_s)
        .atmosphericSkyColorS(atmospheric_sky_color_s)
        .atmosphericStars(atmospheric_stars)
        .atmosphericStarsOrientation(atmospheric_stars_orientation);
    if (!on)
        ::Sky.clear();
    else if (skybox)
        ::Sky.skybox(skybox);
    else
        ::Sky.atmospheric();
}
void Environment::Sky::get() {
    on = ::Sky._is;
    frac = ::Sky.frac();
    skybox = ::Sky.skybox();
    atmospheric_density_exponent = ::Sky.atmosphericDensityExponent();
    atmospheric_horizon_exponent = ::Sky.atmosphericHorizonExponent();
    atmospheric_horizon_color_s = ::Sky.atmosphericHorizonColorS();
    atmospheric_sky_color_s = ::Sky.atmosphericSkyColorS();
    atmospheric_stars = ::Sky.atmosphericStars();
    atmospheric_stars_orientation = ::Sky.atmosphericStarsOrientation();
}
void Environment::Sky::reset() {
    on = true;
    frac = 0.8f;
    skybox = null;
    atmospheric_density_exponent = 1;
    atmospheric_horizon_exponent = 3.5f;
    atmospheric_sky_color_s = LinearToSRGB(Vec4(0.032f, 0.113f, 0.240f, 1.0f));     // #DefaultSkyValue
    atmospheric_horizon_color_s = LinearToSRGB(Vec4(0.093f, 0.202f, 0.374f, 1.0f)); // #DefaultSkyValue
    atmospheric_stars = null;
    atmospheric_stars_orientation.identity();
}

Bool Environment::Sky::save(File &f, CChar *path) C {
    f.cmpUIntV(0); // version
    f << on << frac << atmospheric_density_exponent << atmospheric_horizon_exponent << atmospheric_horizon_color_s << atmospheric_sky_color_s << atmospheric_stars_orientation;
    f._putStr(atmospheric_stars.name(path))._putStr(skybox.name(path));
    return f.ok();
}
Bool Environment::Sky::load(File &f, CChar *path) {
    switch (f.decUIntV()) {
    case 0: {
        f >> on >> frac >> atmospheric_density_exponent >> atmospheric_horizon_exponent >> atmospheric_horizon_color_s >> atmospheric_sky_color_s >> atmospheric_stars_orientation;
        atmospheric_stars.require(f._getStr(), path);
        skybox.require(f._getStr(), path);
        if (f.ok())
            return true;
    } break;
    }
    reset();
    return false;
}
/******************************************************************************/
// SUN
/******************************************************************************/
void Environment::Sun::set() C {
    ::Sun.draw = on;
    ::Sun.glow = glow;
    ::Sun.size = size;
    ::Sun.highlight_front = highlight_front;
    ::Sun.highlight_back = highlight_back;
    ::Sun.pos = pos;
    ::Sun.lightColorS(light_color_s);
    ::Sun.rays_color = rays_color;
    ::Sun.image_color = image_color;
    ::Sun.image = image;
}
void Environment::Sun::get() {
    on = ::Sun.draw;
    glow = ::Sun.glow;
    size = ::Sun.size;
    highlight_front = ::Sun.highlight_front;
    highlight_back = ::Sun.highlight_back;
    pos = ::Sun.pos;
    light_color_s = ::Sun.lightColorS();
    rays_color = ::Sun.rays_color;
    image_color = ::Sun.image_color;
    image = ::Sun.image;
}
void Environment::Sun::reset() {
    on = true;
    glow = 128;
    size = 0.15;
    highlight_front = 0.20f;
    highlight_back = 0.15f;
    pos.set(-SQRT3_3, SQRT3_3, -SQRT3_3);
    rays_color = 0.12f;
    image_color = 1;
    image = null;
    light_color_s = 0.950f; // #DefaultAmbientValue
}

Bool Environment::Sun::save(File &f, CChar *path) C {
    f.cmpUIntV(1); // version
    f << on << glow << size << highlight_front << highlight_back << pos << light_color_s << rays_color << image_color;
    f.putAsset(image.id());
    return f.ok();
}
Bool Environment::Sun::load(File &f, CChar *path) {
    Bool blend;
    switch (f.decUIntV()) {
    case 1: {
        f >> on >> glow >> size >> highlight_front >> highlight_back >> pos >> light_color_s >> rays_color >> image_color;
        image.require(f.getAssetID(), path);
        if (f.ok())
            return true;
    } break;

    case 0: {
        f >> on >> blend >> glow >> size >> highlight_front >> highlight_back >> pos >> light_color_s >> rays_color >> image_color;
        image.require(f._getStr(), path);
        if (f.ok())
            return true;
    } break;
    }
    reset();
    return false;
}
/******************************************************************************/
// ENVIRONMENT
/******************************************************************************/
void Environment::set() C {
    ambient.set();
    bloom.set();
    clouds.set();
    fog.set();
    sky.set();
    sun.set();
}
void Environment::get() {
    ambient.get();
    bloom.get();
    clouds.get();
    fog.get();
    sky.get();
    sun.get();
}
void Environment::reset() {
    ambient.reset();
    bloom.reset();
    clouds.reset();
    fog.reset();
    sky.reset();
    sun.reset();
}

Bool Environment::save(File &f, CChar *path) C {
    f.putUInt(CC4_ENV).cmpUIntV(0); // version
    if (ambient.save(f, path))
        if (bloom.save(f, path))
            if (clouds.save(f, path))
                if (fog.save(f, path))
                    if (sky.save(f, path))
                        if (sun.save(f, path))
                            return f.ok();
    return false;
}
Bool Environment::load(File &f, CChar *path) {
    if (f.getUInt() == CC4_ENV)
        switch (f.decUIntV()) {
        case 0: {
            if (ambient.load(f, path))
                if (bloom.load(f, path))
                    if (clouds.load(f, path))
                        if (fog.load(f, path))
                            if (sky.load(f, path))
                                if (sun.load(f, path))
                                    if (f.ok())
                                        return true;
        } break;
        }
    reset();
    return false;
}

Bool Environment::save(C Str &name) C {
    File f;
    if (f.write(name)) {
        if (save(f, _GetPath(name)) && f.flush())
            return true;
        f.del();
        FDelFile(name);
    }
    return false;
}
Bool Environment::load(C Str &name) {
    File f;
    if (f.read(name))
        return load(f, _GetPath(name));
    reset();
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
