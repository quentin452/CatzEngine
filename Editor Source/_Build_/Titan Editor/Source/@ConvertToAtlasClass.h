#pragma once
/******************************************************************************/
/******************************************************************************/
class ConvertToAtlasClass : PropWin {
    class Mtrl {
        Button del;
        UID id;
        TextLine name;
        Text t_original_size, t_scaled_size;
        VecI2 original_size, scaled_size, pos;
        RectI packed_rect; // not inclusive
        bool rotated;
        Vec2 scale;
        Property prop;
        ImagePtr base_0;
        EditMaterial edit;

        void setScaleText(C VecI2 &size);
        void setScale();
        void setScale(C Vec2 &scale);
        void posY(flt y);

      public:
        Mtrl();
    };
    class Error {
        UID mesh_id, mtrl_id;
        flt error;

      public:
        Error();
    };
    class Preview : GuiCustom {
        virtual void draw(C GuiPC &gpc) override;
    };
    class Warning : ClosableWindow {
        Text text;
        Region region;
        List<Error> list;
        Button proceed;

        static Str MeshName(C Error &error);
        static Str MtrlName(C Error &error);
        static void Proceed(ptr);

        void create();
        void display();
    };

    enum MODE {
        NEW,
        REPLACE_KEEP,
        REPLACE_NO_PUBLISH,
        REPLACE_REMOVE,
    };
    static cchar8 *mode_t[];
    static const flt h;

    Memx<Mtrl> mtrls;
    Memc<Error> errors;
    Region region;
    Preview preview;
    Warning warning;
    bool force_square, auto_stretch, allow_rotate;
    flt scale;
    MODE mode;
    VecI2 tex_size;
    Text columns, t_tex_size;
    Button convert;

    static Str TexSize(C VecI2 &size);
    static void Scale(Mtrl &mtrl, C Str &text);
    static Str Scale(C Mtrl &mtrl);
    static void Del(Mtrl &mtrl);
    static void Refresh(C Property &prop);
    static void ChangedScale(C Property &prop);
    static int CompareData(C Mtrl &a, C Mtrl &b);
    static void Convert(ConvertToAtlasClass &cta);
    //static void PackMaterial(Mtrl &mtrl, MtrlImages &atlas, int thread_index) {mtrl.pack(atlas);}

    void setErrors(Mesh &mesh, C Memc<UID> &mtrl_ids, C UID &mesh_id);
    static bool Create(int &occurence, C UID &id, ptr user);
    void convertMeshes(C UID &atlas_id);
    static bool AddMap(bool &forced, Str &dest, C Str &src, bool force, C Mtrl &mtrl, TEX_FLAG tex_flag,
                       C Vec &src_mul = 1, C Vec &src_add = 0, // transformation of the source map
                       flt dest_mul = 1, flt dest_add = 0);    // transformation of the dest   map
    void checkSide(Str &dest, bool filled);
    void convertPerform();
    void convertDo();
    void clearProj();
    void create();
    void autoStretch();
    void refresh();
    void addElms(C MemPtr<UID> &elm_ids);
    void setElms(C MemPtr<UID> &elm_ids);
    void drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos);
    virtual ConvertToAtlasClass &hide() override;
    /*virtual void update(C GuiPC &gpc)override
   {
      super.update(gpc);
      if(gpc.visible && visible())
      {
      }
   }*/

  public:
    ConvertToAtlasClass();
};
/******************************************************************************/
/******************************************************************************/
extern ConvertToAtlasClass ConvertToAtlas;
/******************************************************************************/
