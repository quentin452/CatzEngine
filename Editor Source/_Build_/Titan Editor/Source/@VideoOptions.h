/******************************************************************************/
/******************************************************************************/
class VideoOptions : PropWin {
    class Advanced : PropWin {
        static cchar8 *DiffuseMode_t[];
        ASSERT(DIFFUSE_LAMBERT == 0 && DIFFUSE_OREN_NAYAR == 1 && DIFFUSE_BURLEY == 2 && DIFFUSE_NUM == 3);
        static cchar8 *TexFilter_t[];
        static cchar8 *Density_t[];
        static cchar8 *DensityFilter_t[];
        static FILTER_TYPE DensityFilter_v[];
        static cchar8 *TexUse_t[];
        ASSERT(TEX_USE_DISABLE == 0 && TEX_USE_SINGLE == 1 && TEX_USE_MULTI == 2);
        static cchar8 *EdgeDetect_t[];
        ASSERT(EDGE_DETECT_NONE == 0 && EDGE_DETECT_SOFT == 1 && EDGE_DETECT_THIN == 2);
        static cchar8 *Precision_t[];
        ASSERT(IMAGE_PRECISION_8 == 0 && IMAGE_PRECISION_10 == 1);
        static cchar8 *Stage_t[];
        ASSERT(RS_DEFAULT == 0 && RS_DEPTH == 1 && RS_COLOR == 2 && RS_NORMAL == 3 && RS_SMOOTH == 4 && RS_REFLECT == 5 && RS_GLOW == 6 && RS_EMISSIVE == 7 && RS_MOTION == 8 && RS_LIGHT == 9 && RS_LIGHT_AO == 10 && RS_AO == 11 && RS_LIT_COLOR == 12 && RS_ALPHA == 13 && RS_REFLECTION == 14 && RS_WATER_COLOR == 15 && RS_WATER_NORMAL == 16 && RS_WATER_LIGHT == 17);
        /*static cchar8 *ToneMap_t[]=
      {
         "Off"     , // 0
         "Default" , // 1
         "ACES LDR", // 2
         "ACES HDR", // 3
      }; ASSERT(TONE_MAP_OFF==0 && TONE_MAP_DEFAULT==1 && TONE_MAP_ACES_LDR==2 && TONE_MAP_ACES_HDR==3 && TONE_MAP_NUM==4); */
        static cchar8 *ShadowReduceFlicker_t[];
        static cchar8 *ColorSpace_t[];
        ASSERT(COLOR_SPACE_NONE == 0 && COLOR_SPACE_SRGB == 1 && COLOR_SPACE_DISPLAY_P3 == 2 && COLOR_SPACE_NUM == 3);

        static Str Fov(C Advanced &adv);
        static void Fov(Advanced &adv, C Str &text);
        static Str TexFilter(C Advanced &adv);
        static void TexFilter(Advanced &adv, C Str &text);
        static Str TexMipFilter(C Advanced &adv);
        static void TexMipFilter(Advanced &adv, C Str &text);
        static Str DetailTexUse(C Advanced &adv);
        static void DetailTexUse(Advanced &adv, C Str &text);
        static Str Samples(C Advanced &adv);
        static void Samples(Advanced &adv, C Str &text);
        static Str Density(C Advanced &adv);
        static void Density(Advanced &adv, C Str &text);
        static Str DensityFilter(C Advanced &adv);
        static void DensityFilter(Advanced &adv, C Str &text);
        static Str GrassRange(C Advanced &adv);
        static void GrassRange(Advanced &adv, C Str &text); // grass objects will be hidden for 0
        static Str GrassDensity(C Advanced &adv);
        static void GrassDensity(Advanced &adv, C Str &text);
        static Str SoftParticle(C Advanced &adv);
        static void SoftParticle(Advanced &adv, C Str &text);
        // static Str  ColorPalette (C Advanced &adv             ) {return Renderer.color_palette.name();}
        // static void ColorPalette (  Advanced &adv, C Str &text) {Renderer.color_palette.get(ImageName(text));}
        // static Str  ColorPalette1(C Advanced &adv             ) {return Renderer.color_palette1.name();}
        // static void ColorPalette1(  Advanced &adv, C Str &text) {Renderer.color_palette1.get(ImageName(text));}
        static Str VolLight(C Advanced &adv);
        static void VolLight(Advanced &adv, C Str &text);
        static Str MaxLights(C Advanced &adv);
        static void MaxLights(Advanced &adv, C Str &text);
        static Str EdgeDetect(C Advanced &adv);
        static void EdgeDetect(Advanced &adv, C Str &text);
        static Str Stage(C Advanced &adv);
        static void Stage(Advanced &adv, C Str &text);
        static Str ToneMap(C Advanced &adv);
        static void ToneMap(Advanced &adv, C Str &text);
        static Str Contrast(C Advanced &adv);
        static void Contrast(Advanced &adv, C Str &text);
        static Str EyeAdaptBrigh(C Advanced &adv);
        static void EyeAdaptBrigh(Advanced &adv, C Str &text);
        static Str Exclusive(C Advanced &adv);
        static void Exclusive(Advanced &adv, C Str &text);
        static Str ColorSpace(C Advanced &adv);
        static void ColorSpace(Advanced &adv, C Str &text);
        static Str DiffuseMode(C Advanced &adv);
        static void DiffuseMode(Advanced &adv, C Str &text);
        static Str Hdr(C Advanced &adv);
        static void Hdr(Advanced &adv, C Str &text);
        static Str Sharpen(C Advanced &adv);
        static void Sharpen(Advanced &adv, C Str &text);
        static Str Dither(C Advanced &adv);
        static void Dither(Advanced &adv, C Str &text);
        static Str ColRTPrec(C Advanced &adv);
        static void ColRTPrec(Advanced &adv, C Str &text);
        static Str NrmRTPrec(C Advanced &adv);
        static void NrmRTPrec(Advanced &adv, C Str &text);
        static Str LumRTPrec(C Advanced &adv);
        static void LumRTPrec(Advanced &adv, C Str &text);
        static Str LitColRTPrec(C Advanced &adv);
        static void LitColRTPrec(Advanced &adv, C Str &text);
        static Str BloomScale(C Advanced &adv);
        static void BloomScale(Advanced &adv, C Str &text);
        static Str AmbLight(C Advanced &adv);
        static void AmbLight(Advanced &adv, C Str &text);
        static Str AOContrast(C Advanced &adv);
        static void AOContrast(Advanced &adv, C Str &text);
        static Str AORange(C Advanced &adv);
        static void AORange(Advanced &adv, C Str &text);
        static Str AORes(C Advanced &adv);
        static void AORes(Advanced &adv, C Str &text);
        static Str DOF(C Advanced &adv);
        static void DOF(Advanced &adv, C Str &text);
        static Str DOFIntensity(C Advanced &adv);
        static void DOFIntensity(Advanced &adv, C Str &text);
        static Str ShadowFlicker(C Advanced &adv);
        static void ShadowFlicker(Advanced &adv, C Str &text);
        static Str ShadowFrac(C Advanced &adv);
        static void ShadowFrac(Advanced &adv, C Str &text);
        static Str ShadowFade(C Advanced &adv);
        static void ShadowFade(Advanced &adv, C Str &text);
        static Str AllowGlow(C Advanced &adv);
        static void AllowGlow(Advanced &adv, C Str &text);
        static Str ForwardPrec(C Advanced &adv);
        static void ForwardPrec(Advanced &adv, C Str &text);
        static Str MaterialBlend(C Advanced &adv);
        static void MaterialBlend(Advanced &adv, C Str &text);
        static Str TexMipMin(C Advanced &adv);
        static void TexMipMin(Advanced &adv, C Str &text);
        static Str SkyNightLight(C Advanced &adv);
        static void SkyNightLight(Advanced &adv, C Str &text);
        static Str SkyNightLightIntensity(C Advanced &adv);
        static void SkyNightLightIntensity(Advanced &adv, C Str &text);
        static Str SkyNightLightSchedule(C Advanced &adv);
        static void SkyNightLightSchedule(Advanced &adv, C Str &text);

        enum MODE {
            AUTO,
            OFF,
            ON,
        };
        flt fov;
        bool sky_night_light_on;
        int sky_night_light_last_check, // -1/false/true
            sky_night_light_start;      // in minutes
        flt sky_night_light_intensity;
        Property *diffuse, *sky_night_light;

        void setFov(flt fov);

        // SKY NIGHT LIGHT
        void skyNightLight(bool on);
        flt skyNightLightIntensity() C;
        void skyNightLightIntensity(flt intensity);
        Str skyNightLightSchedule() C;
        void skyNightLightSchedule(Str text);
        bool skyNightLightOn(int time) C; // if should be enabled for specified time
        void skyNightLightUpdate();

        void ctor();
        void create();
        virtual Advanced &hide() override;

      public:
        Advanced();
    };

    static cchar8 *Render_t[];
    ASSERT(RT_DEFERRED == 0 && RT_FORWARD == 1 && RT_NUM == 2);
    static cchar8 *EdgeSoften_t[];
    ASSERT(EDGE_SOFTEN_NONE == 0 && EDGE_SOFTEN_FXAA == 1 && !SUPPORT_MLAA && EDGE_SOFTEN_SMAA == 2 + SUPPORT_MLAA);
    static cchar8 *ShadowSize_t[];
    static cchar8 *ShadowNum_t[];
    static cchar8 *ShadowSoft_t[];
    ASSERT(SHADOW_SOFT_NUM == 6);
    static cchar8 *BumpMode_t[];
    ASSERT(BUMP_FLAT == 0 && BUMP_NORMAL == 1 && BUMP_PARALLAX == 2 && BUMP_RELIEF == 3 && BUMP_NUM == 4);
    static cchar8 *MotionMode_t[];
    static cchar8 *AO_t[];

    class Skin {
        cchar8 *name;
        UID id;
    };
    static Skin skins[];

    static void Mode(VideoOptions &vo, C Str &t);
    static Str Full(C VideoOptions &vo);
    static void Full(VideoOptions &vo, C Str &t);
    static Str Sync(C VideoOptions &vo);
    static void Sync(VideoOptions &vo, C Str &t);
    static Str Render(C VideoOptions &vo);
    static void Render(VideoOptions &vo, C Str &t);
    static Str TAA(C VideoOptions &vo);
    static void TAA(VideoOptions &vo, C Str &t);
    static Str TempSuperRes(C VideoOptions &vo);
    static void TempSuperRes(VideoOptions &vo, C Str &t);
    static Str EdgeSoft(C VideoOptions &vo);
    static void EdgeSoft(VideoOptions &vo, C Str &t);
    static Str Shadow(C VideoOptions &vo);
    static void Shadow(VideoOptions &vo, C Str &t);
    static Str ShadowSize(C VideoOptions &vo); // go from end to check biggest first
    static void ShadowSize(VideoOptions &vo, C Str &t);
    static Str ShadowNum(C VideoOptions &vo);
    static void ShadowNum(VideoOptions &vo, C Str &t);
    static Str ShadowSoft(C VideoOptions &vo);
    static void ShadowSoft(VideoOptions &vo, C Str &t);
    static Str ShadowJit(C VideoOptions &vo);
    static void ShadowJit(VideoOptions &vo, C Str &t);
    static Str BumpMode(C VideoOptions &vo);
    static void BumpMode(VideoOptions &vo, C Str &t);
    static Str MotionMode(C VideoOptions &vo);
    static void MotionMode(VideoOptions &vo, C Str &t);
    static Str AO(C VideoOptions &vo);
    static void AO(VideoOptions &vo, C Str &t);
    static Str EyeAdapt(C VideoOptions &vo);
    static void EyeAdapt(VideoOptions &vo, C Str &t);
    static Str Scale(C VideoOptions &vo);
    static void Scale(VideoOptions &vo, C Str &t);
    static Str ScaleWin(C VideoOptions &vo);
    static void ScaleWin(VideoOptions &vo, C Str &t);
    static void SkinChanged(VideoOptions &vo);

    Property *full, *mode, *shd_siz, *shd_num, *shd_sft, *shd_jit, *skin;
    Button advanced_show;
    Advanced advanced;
    flt scale;
    bool scale_win;

    static void ShowAdvanced(VideoOptions &vo);

    void setScale();
    void setScale(flt scale);
    void setScaleWin(bool scale);

    UID skinID(C Str &name) C;
    int skinIndex(C UID &id) C;
    Str skinName() C;

    void resize();
    void ctor();
    void create();
    void setVis();
    void hideAll();
    virtual Window &show() override;
    virtual VideoOptions &hide() override;
    virtual void update(C GuiPC &gpc) override;

  public:
    VideoOptions();
};
/******************************************************************************/
/******************************************************************************/
extern VideoOptions VidOpt;
/******************************************************************************/
