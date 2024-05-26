/******************************************************************************/
/******************************************************************************/
class ElmImageAtlas : ElmData {
    enum FLAG {
        MIP_MAPS = 1 << 0,
        COMPRESS = 1 << 1,
    };
    class Img {
        bool removed;
        UID id;
        TimeStamp removed_time;

        bool equal(C Img &src) C;
        bool newer(C Img &src) C;

        bool undo(C Img &src);

      public:
        Img();
    };
    static int CompareID(C Img &img, C UID &id);

    byte flag;
    Memc<Img> images;
    TimeStamp file_time, mip_maps_time, compress_time;

    bool mipMaps() C;
    void mipMaps(bool on);
    bool compress() C;
    void compress(bool on);

    C Img *find(C UID &id) C;
    Img *find(C UID &id);
    Img &get(C UID &id);

    // operations
    virtual void newData() override;

    bool equal(C ElmImageAtlas &src) C;
    bool newer(C ElmImageAtlas &src) C;

    virtual bool mayContain(C UID &id) C override;

    uint undo(C ElmImageAtlas &src);
    uint sync(C ElmImageAtlas &src);
    bool syncFile(C ElmImageAtlas &src);

    // io
    virtual bool save(File &f) C override;
    virtual bool load(File &f) override;
    virtual void save(MemPtr<TextNode> nodes) C override;
    virtual void load(C MemPtr<TextNode> &nodes) override;

  public:
    ElmImageAtlas();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
