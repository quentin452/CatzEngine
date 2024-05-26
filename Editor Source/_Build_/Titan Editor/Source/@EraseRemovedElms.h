/******************************************************************************/
/******************************************************************************/
class EraseRemovedElms : ClosableWindow {
    static void OK(EraseRemovedElms &ere);
    static void Full(EraseRemovedElms &ere);

    class Elm {
        Str name;
        UID id;

        void create(C ::Elm &src);

      public:
        Elm();
    };

    TextNoTest text;
    Button ok, full, cancel;
    Region region;
    Memc<Elm> data;
    List<Elm> list;

    void create();
    virtual EraseRemovedElms &show() override;
    virtual EraseRemovedElms &hide() override;
    virtual void update(C GuiPC &gpc) override;
};
/******************************************************************************/
/******************************************************************************/
extern EraseRemovedElms EraseRemoved;
/******************************************************************************/
