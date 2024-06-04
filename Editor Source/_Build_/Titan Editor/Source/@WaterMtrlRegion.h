#pragma once
/******************************************************************************/
/******************************************************************************/
class WaterMtrlRegion : MaterialRegion {
    class Change : MaterialRegion::Change {
        EditWaterMtrl data;

        virtual void create(ptr user) override;
        virtual void apply(ptr user) override;
    };

    WaterMtrl temp;
    WaterMtrlPtr game;
    EditWaterMtrl edit;

    static void Render();
    void render();
    virtual void drawPreview() override;

    static void PreChanged(C Property &prop);
    static void Changed(C Property &prop);

    static Str Col(C WaterMtrlRegion &mr);
    static void Col(WaterMtrlRegion &mr, C Str &t);
    static Str Smooth(C WaterMtrlRegion &mr);
    static void Smooth(WaterMtrlRegion &mr, C Str &t);
    static Str Reflect(C WaterMtrlRegion &mr);
    static void Reflect(WaterMtrlRegion &mr, C Str &t);
    static Str NrmScale(C WaterMtrlRegion &mr);
    static void NrmScale(WaterMtrlRegion &mr, C Str &t);
    static Str FNY(C WaterMtrlRegion &mr);
    static void FNY(WaterMtrlRegion &mr, C Str &t);
    static Str SmtIsRgh(C WaterMtrlRegion &mr);
    static void SmtIsRgh(WaterMtrlRegion &mr, C Str &t);
    static Str WaveScale(C WaterMtrlRegion &mr);
    static void WaveScale(WaterMtrlRegion &mr, C Str &t);

    static Str ScaleColor(C WaterMtrlRegion &mr);
    static void ScaleColor(WaterMtrlRegion &mr, C Str &t);
    static Str ScaleNormal(C WaterMtrlRegion &mr);
    static void ScaleNormal(WaterMtrlRegion &mr, C Str &t);
    static Str ScaleBump(C WaterMtrlRegion &mr);
    static void ScaleBump(WaterMtrlRegion &mr, C Str &t);

    static Str Density(C WaterMtrlRegion &mr);
    static void Density(WaterMtrlRegion &mr, C Str &t);
    static Str DensityAdd(C WaterMtrlRegion &mr);
    static void DensityAdd(WaterMtrlRegion &mr, C Str &t);

    static Str Refract(C WaterMtrlRegion &mr);
    static void Refract(WaterMtrlRegion &mr, C Str &t);
    static Str RefractReflection(C WaterMtrlRegion &mr);
    static void RefractReflection(WaterMtrlRegion &mr, C Str &t);

    static Str ColorUnderwater0(C WaterMtrlRegion &mr);
    static void ColorUnderwater0(WaterMtrlRegion &mr, C Str &t);
    static Str ColorUnderwater1(C WaterMtrlRegion &mr);
    static void ColorUnderwater1(WaterMtrlRegion &mr, C Str &t);

    // #MaterialTextureLayoutWater
    virtual EditMaterial &getEditMtrl() override;
    virtual C ImagePtr &getBase0() override;
    virtual C ImagePtr &getBase1() override;
    virtual C ImagePtr &getBase2() override;
    //virtual C ImagePtr    & getDetail  ()override  {return game->  detail_map  ;}
    //virtual C ImagePtr    & getMacro   ()override  {return game->   macro_map  ;}
    //virtual C ImagePtr    & getEmissive()override  {return game->emissive_map  ;}
    virtual bool water() C override;

    void create();

    // operations
    virtual void flush() override;
    virtual void setChanged() override;
    virtual void set(Elm *elm) override;

    void set(C WaterMtrlPtr &mtrl);

    virtual void resizeBase(C VecI2 &size, bool relative = false) override;
    virtual void resizeBase0(C VecI2 &size, bool relative = false) override;
    virtual void resizeBase1(C VecI2 &size, bool relative = false) override;
    virtual void resizeBase2(C VecI2 &size, bool relative = false) override;

    virtual void rebuildBase(TEX_FLAG old_textures, uint changed_in_mtrl = 0, bool adjust_params = true, bool always = false) override;
    virtual void rebuildDetail() override;
    virtual void rebuildMacro() override;
    virtual void rebuildEmissive(TEX_FLAG old_textures, bool adjust_params = true) override;

    virtual void elmChanged(C UID &mtrl_id) override;

  public:
    WaterMtrlRegion();
};
/******************************************************************************/
/******************************************************************************/
extern WaterMtrlRegion WaterMtrlEdit;
/******************************************************************************/
