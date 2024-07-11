#pragma once
/******************************************************************************/
/******************************************************************************/
class WorldMapVer {
    bool changed;
    TimeStamp time; // this is time of both settings and images, images will be received only if their TimeStamp matches this value
    Game::WorldMap::Settings settings;
    Str path;
    UID world_map_id;
    Memc<VecI2> images;

    ~WorldMapVer();
    WorldMapVer &setChanged();
    void operator=(C WorldMapVer &src);

    // io
    bool save(File &f, bool network = false) C;
    bool load(File &f, bool network = false);
    bool save(C Str &name) C;
    bool load(C Str &name);

    void flush();

  public:
    WorldMapVer();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
