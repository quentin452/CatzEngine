/******************************************************************************/
#include "stdafx.h"
namespace EE {
namespace Game {
/******************************************************************************/
void WorldMap::Settings::del() {
    image_size = 0;
    areas_per_image = 1; // use 1 instead of 0 to avoid div by 0 which could cause crash on Web
    area_size = 1;       // use 1 instead of 0 to avoid div by 0 which could cause crash on Web
}
Bool WorldMap::Settings::save(File &f) C {
    f.cmpUIntV(0); // version
    f << areas_per_image << image_size << area_size;
    return f.ok();
}
Bool WorldMap::Settings::load(File &f) {
    switch (f.decUIntV()) // version
    {
    case 0: {
        f >> areas_per_image >> image_size >> area_size;
        if (f.ok())
            return true;
    } break;
    }
    del();
    return false;
}
Bool WorldMap::Settings::save(C Str &name) C {
    File f;
    if (f.write(name)) {
        if (save(f) && f.flush())
            return true;
        f.del();
        FDelFile(name);
    }
    return false;
}
Bool WorldMap::Settings::load(C Str &name) {
    File f;
    if (f.read(name))
        return load(f);
    del();
    return false;
}
/******************************************************************************/
Bool WorldMap::Create(Image &image, C VecI2 &key, Ptr user) {
    WorldMap &mm = *(WorldMap *)user;
    if (mm.name().is())
        image.load(mm.name() + '/' + key);
    return true;
}
/******************************************************************************/
WorldMap::WorldMap() : _map(Compare, WorldMap::Create, this) {
    _id.zero();
}
void WorldMap::del() {
    _map.del();
    _settings.del();
    _name.del();
    _id.zero();
}
/******************************************************************************/
Bool WorldMap::load(C Str &name) {
    del();
    if (name.is()) {
        if (!DecodeFileName(name, _id))
            _id.zero();
        _name = name;
        _name.tailSlash(false);
        if (!_settings.load(_name + "/Settings"))
            goto error;
    }
    return true;
error:
    del();
    return false;
}
void WorldMap::operator=(C Str &name) {
    if (!load(name))
        Exit(S + "Can't load World Map \"" + name + "\"");
}
Bool WorldMap::load(C UID &id) { return load(id.valid() ? _EncodeFileName(id) : null); }
void WorldMap::operator=(C UID &id) { T = (id.valid() ? _EncodeFileName(id) : null); }
/******************************************************************************/
void WorldMap::clear(C RectI *leave) {
    if (!leave)
        _map.del();
    else {
        REPA(_map)                      // check all elements (images), order is important
        if (!Cuts(_map.key(i), *leave)) // if it shouldn't be left (coordinates aren't in the 'leave' rectangle)
            _map.remove(i);             // remove this element
    }
}
/******************************************************************************/
} // namespace Game
} // namespace EE
/******************************************************************************/
