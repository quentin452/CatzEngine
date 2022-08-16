/******************************************************************************/
/******************************************************************************/
class MaterialRegion : Region
{
   class Change : Edit::_Undo::Change
   {
      EditMaterial data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };
   class Texture : GuiCustom
   {
      TEX_TYPE        type;
      Str             text;
      Str             file;
      Color           rect_color;
      MemberDesc      md_file, md_time;
      Button          remove;
      MaterialRegion *mr;

      static void Load  (C Str &name, Texture &texture);
      static void Remove(             Texture &texture);

      ImagePtr getImage();
      static void ReplaceElmNames(Mems<FileParams> &files);
      void setDesc();
      static void FixPath(Mems<FileParams> &fps);
      void setFile(Str file, bool set_undo=true);
      void toGui();
      Texture& create(TEX_TYPE type, C MemberDesc &md_file, C MemberDesc &md_time, Rect rect, C Str &text, MaterialRegion &mr);

      static bool ExploreFiles(C Mems<FileParams> &fps);
      virtual void update(C GuiPC &gpc)override;
      bool draw(C Rect &rect);
      virtual void draw(C GuiPC &gpc)override;
      
      WindowIO win_io;

public:
   Texture();
   };

   static cchar8 *auto_reload_name;

   ELM_TYPE          elm_type;
   bool              auto_reload;
   Button            set_mtrl, undo, redo, locate, big, close, reload_base_textures;
   ComboBox          texture_options;
   Tabs              preview_mode;
   ViewportSkin      preview, preview_big;
   Mesh              preview_mesh[4];
   Camera            preview_cam;
   flt               min_zoom, max_zoom, mouse_edit_delta;
   Vec               mouse_edit_value;
   Vec2              light_angle;
   Region            sub;
   Button            brightness, rgb_1, emissive;
   Property         *red, *green, *blue, *alpha, *emit_red, *emit_green, *emit_blue, *smooth;
   Memx<Property>    props;
   Memx<Texture>     texs;
   TextBlack         ts;
   Material          temp;
   MaterialPtr       game;
   EditMaterial      edit, saved;
   Image             detail_bump;
   Str               detail_bump_file;
   UID               elm_id;
   Elm              *elm;
   bool              changed;
   Edit::Undo<Change> undos;   void undoVis();

   Vec previewLight()C;

   static void Render();
          void render();
   static  void DrawPreview(Viewport &viewport);
   virtual void drawPreview();

   static void PreChanged(C Property &prop); // set all RGB props to have the same change_type so they will not create too manu undos
   static void    Changed(C Property &prop);

   class MaterialTech
   {
      MATERIAL_TECHNIQUE tech;
      cchar8            *name;
      cchar             *desc;
   };
   static MaterialTech mtrl_techs[]
; ASSERT(MTECH_NUM==17);
   static Str  Tech(C MaterialRegion &mr          );
   static void Tech(  MaterialRegion &mr, C Str &t);

   static cchar8 *TexDownsizeText[]
; ASSERT(MaxMaterialDownsize==4);
   static Str  TexDownsizeMobile(C MaterialRegion &mr          );
   static void TexDownsizeMobile(  MaterialRegion &mr, C Str &t);
   static Str  TexDownsizeSwitch(C MaterialRegion &mr          );
   static void TexDownsizeSwitch(  MaterialRegion &mr, C Str &t);

   class TexQualityND : NameDesc
   {
      Edit::Material::TEX_QUALITY quality;
   };
   static TexQualityND TexQualities[]
;
   static Str  TexQuality(C MaterialRegion &mr          );
   static void TexQuality(  MaterialRegion &mr, C Str &t); // undo already called in 'PreChanged'

   /*static .MaxTexSize max_tex_sizes[]=
   {
      {Edit.MTS_128      ,       "128", "Textures will be resized to 128 if they are bigger"},
      {Edit.MTS_256      ,       "256", "Textures will be resized to 256 if they are bigger"},
      {Edit.MTS_512      ,       "512", "Textures will be resized to 512 if they are bigger"},
      {Edit.MTS_1024     ,      "1024", "Textures will be resized to 1024 if they are bigger"},
      {Edit.MTS_2048     ,      "2048", "Textures will be resized to 2048 if they are bigger"},
      {Edit.MTS_UNLIMITED, "Unlimited", "Textures won't be resized"},
      {Edit.MTS_PUBLISH_SETTINGS, "Use Publish Settings", "Texture size limit will be taken from the value specified in Project Publishing Settings"},
   };
   static Str  MaxTexSize(C MaterialRegion &mr          ) {REPA(max_tex_sizes)if(max_tex_sizes[i].mts==mr.edit.max_tex_size)return i; return S;}
   static void MaxTexSize(  MaterialRegion &mr, C Str &t) {int i=TextInt(t); if(InRange(i, max_tex_sizes)){mr.edit.max_tex_size=max_tex_sizes[i].mts; mr.edit.max_tex_size_time.now();}}*/

   static void RGB1(MaterialRegion &mr);
   static void RGB (MaterialRegion &mr);
   static void Emissive(MaterialRegion &mr);

   static Str  Red  (C MaterialRegion &mr          );
   static void Red  (  MaterialRegion &mr, C Str &t);
   static Str  Green(C MaterialRegion &mr          );
   static void Green(  MaterialRegion &mr, C Str &t);
   static Str  Blue (C MaterialRegion &mr          );
   static void Blue (  MaterialRegion &mr, C Str &t);
   static Str  Alpha(C MaterialRegion &mr          );
   static void Alpha(  MaterialRegion &mr, C Str &t);

   static const flt BumpScale;
   static Str  Bump      (C MaterialRegion &mr          );
   static void Bump      (  MaterialRegion &mr, C Str &t); // call 'setChanged' manually because it needs to be called before 'setShader'
   static Str  NrmScale  (C MaterialRegion &mr          );
   static void NrmScale  (  MaterialRegion &mr, C Str &t); // call 'setChanged' manually because it needs to be called before 'setShader'
   static Str  FNY       (C MaterialRegion &mr          );
   static void FNY       (  MaterialRegion &mr, C Str &t);
   static Str  RoughImage(C MaterialRegion &mr          );
   static void RoughImage(  MaterialRegion &mr, C Str &t);

   static Str  Smooth    (C MaterialRegion &mr          );
   static void Smooth    (  MaterialRegion &mr, C Str &t);
   static Str  ReflectMin(C MaterialRegion &mr          );
   static void ReflectMin(  MaterialRegion &mr, C Str &t);
   static Str  ReflectMax(C MaterialRegion &mr          );
   static void ReflectMax(  MaterialRegion &mr, C Str &t);
   static Str  Glow      (C MaterialRegion &mr          );
   static void Glow      (  MaterialRegion &mr, C Str &t);

   static Str  DetUVScale(C MaterialRegion &mr          );
   static void DetUVScale(  MaterialRegion &mr, C Str &t);
   static Str  DetPower  (C MaterialRegion &mr          );
   static void DetPower  (  MaterialRegion &mr, C Str &t); // call 'setChanged' manually because it needs to be called before 'setShader'
   static Str  DetLOD    (C MaterialRegion &mr          );
   static void DetLOD    (  MaterialRegion &mr, C Str &t); // call 'setChanged' manually because it needs to be called before 'setShader'

   static Str  Cull(C MaterialRegion &mr          );
   static void Cull(  MaterialRegion &mr, C Str &t);
 //static Str  SSS (C MaterialRegion &mr          ) {return mr.edit.sss;}
 //static void SSS (  MaterialRegion &mr, C Str &t) {       mr.edit.sss=TextFlt(t); mr.edit.sss_time.getUTC();}

   static Str  EmissiveR(C MaterialRegion &mr          );   
   static void EmissiveR(  MaterialRegion &mr, C Str &t);    // call 'setChanged' manually because it needs to be called before 'setShader'
   static Str  EmissiveG(C MaterialRegion &mr          );   
   static void EmissiveG(  MaterialRegion &mr, C Str &t);    // call 'setChanged' manually because it needs to be called before 'setShader'
   static Str  EmissiveB(C MaterialRegion &mr          );   
   static void EmissiveB(  MaterialRegion &mr, C Str &t);    // call 'setChanged' manually because it needs to be called before 'setShader'
   static Str  EmissiveGlow(C MaterialRegion &mr          );
   static void EmissiveGlow(  MaterialRegion &mr, C Str &t); // call 'setChanged' manually because it needs to be called before 'setShader'

   static Str  UVScale(C MaterialRegion &mr          );
   static void UVScale(  MaterialRegion &mr, C Str &t);

   static void Undo  (MaterialRegion &editor);
   static void Redo  (MaterialRegion &editor);
   static void Locate(MaterialRegion &editor);

   static void Hide   (MaterialRegion &editor);
   static void SetMtrl(MaterialRegion &editor);

   static void AutoReload        (MaterialRegion &editor);
   static void ReloadBaseTextures(MaterialRegion &editor);

   static void ResizeBase128 (MaterialRegion &editor);
   static void ResizeBase256 (MaterialRegion &editor);
   static void ResizeBase512 (MaterialRegion &editor);
   static void ResizeBase1024(MaterialRegion &editor);
   static void ResizeBase2048(MaterialRegion &editor);
   static void ResizeBase4096(MaterialRegion &editor);

   static void ResizeBase128x64   (MaterialRegion &editor);
   static void ResizeBase256x128  (MaterialRegion &editor);
   static void ResizeBase512x256  (MaterialRegion &editor);
   static void ResizeBase1024x512 (MaterialRegion &editor);
   static void ResizeBase2048x1024(MaterialRegion &editor);

   static void ResizeBase64x128   (MaterialRegion &editor);
   static void ResizeBase128x256  (MaterialRegion &editor);
   static void ResizeBase256x512  (MaterialRegion &editor);
   static void ResizeBase512x1024 (MaterialRegion &editor);
   static void ResizeBase1024x2048(MaterialRegion &editor);

   static void ResizeBaseQuarter (MaterialRegion &editor);
   static void ResizeBaseHalf    (MaterialRegion &editor);
   static void ResizeBaseOriginal(MaterialRegion &editor);
   static void ResizeBaseDouble  (MaterialRegion &editor);

   static void ResizeBase0_128 (MaterialRegion &editor);
   static void ResizeBase0_256 (MaterialRegion &editor);
   static void ResizeBase0_512 (MaterialRegion &editor);
   static void ResizeBase0_1024(MaterialRegion &editor);
   static void ResizeBase0_2048(MaterialRegion &editor);
   static void ResizeBase0_4096(MaterialRegion &editor);

   static void ResizeBase0_128x64   (MaterialRegion &editor);
   static void ResizeBase0_256x128  (MaterialRegion &editor);
   static void ResizeBase0_512x256  (MaterialRegion &editor);
   static void ResizeBase0_1024x512 (MaterialRegion &editor);
   static void ResizeBase0_2048x1024(MaterialRegion &editor);

   static void ResizeBase0_64x128   (MaterialRegion &editor);
   static void ResizeBase0_128x256  (MaterialRegion &editor);
   static void ResizeBase0_256x512  (MaterialRegion &editor);
   static void ResizeBase0_512x1024 (MaterialRegion &editor);
   static void ResizeBase0_1024x2048(MaterialRegion &editor);

   static void ResizeBase0_Quarter (MaterialRegion &editor);
   static void ResizeBase0_Half    (MaterialRegion &editor);
   static void ResizeBase0_Original(MaterialRegion &editor);
   static void ResizeBase0_Double  (MaterialRegion &editor);

   static void ResizeBase1_128 (MaterialRegion &editor);
   static void ResizeBase1_256 (MaterialRegion &editor);
   static void ResizeBase1_512 (MaterialRegion &editor);
   static void ResizeBase1_1024(MaterialRegion &editor);
   static void ResizeBase1_2048(MaterialRegion &editor);
   static void ResizeBase1_4096(MaterialRegion &editor);

   static void ResizeBase1_128x64   (MaterialRegion &editor);
   static void ResizeBase1_256x128  (MaterialRegion &editor);
   static void ResizeBase1_512x256  (MaterialRegion &editor);
   static void ResizeBase1_1024x512 (MaterialRegion &editor);
   static void ResizeBase1_2048x1024(MaterialRegion &editor);

   static void ResizeBase1_64x128   (MaterialRegion &editor);
   static void ResizeBase1_128x256  (MaterialRegion &editor);
   static void ResizeBase1_256x512  (MaterialRegion &editor);
   static void ResizeBase1_512x1024 (MaterialRegion &editor);
   static void ResizeBase1_1024x2048(MaterialRegion &editor);

   static void ResizeBase1_Quarter (MaterialRegion &editor);
   static void ResizeBase1_Half    (MaterialRegion &editor);
   static void ResizeBase1_Original(MaterialRegion &editor);
   static void ResizeBase1_Double  (MaterialRegion &editor);
   
   static void ResizeBase2_128 (MaterialRegion &editor);
   static void ResizeBase2_256 (MaterialRegion &editor);
   static void ResizeBase2_512 (MaterialRegion &editor);
   static void ResizeBase2_1024(MaterialRegion &editor);
   static void ResizeBase2_2048(MaterialRegion &editor);
   static void ResizeBase2_4096(MaterialRegion &editor);

   static void ResizeBase2_128x64   (MaterialRegion &editor);
   static void ResizeBase2_256x128  (MaterialRegion &editor);
   static void ResizeBase2_512x256  (MaterialRegion &editor);
   static void ResizeBase2_1024x512 (MaterialRegion &editor);
   static void ResizeBase2_2048x1024(MaterialRegion &editor);

   static void ResizeBase2_64x128   (MaterialRegion &editor);
   static void ResizeBase2_128x256  (MaterialRegion &editor);
   static void ResizeBase2_256x512  (MaterialRegion &editor);
   static void ResizeBase2_512x1024 (MaterialRegion &editor);
   static void ResizeBase2_1024x2048(MaterialRegion &editor);

   static void ResizeBase2_Quarter (MaterialRegion &editor);
   static void ResizeBase2_Half    (MaterialRegion &editor);
   static void ResizeBase2_Original(MaterialRegion &editor);
   static void ResizeBase2_Double  (MaterialRegion &editor);
   
   static void BumpFromCol  (MaterialRegion &editor);
   static void BumpFromCol2 (MaterialRegion &editor);
   static void BumpFromCol3 (MaterialRegion &editor);
   static void BumpFromCol4 (MaterialRegion &editor);
   static void BumpFromCol5 (MaterialRegion &editor);
   static void BumpFromCol6 (MaterialRegion &editor);
   static void BumpFromCol8 (MaterialRegion &editor);
   static void BumpFromCol12(MaterialRegion &editor);
   static void BumpFromCol16(MaterialRegion &editor);
   static void BumpFromCol24(MaterialRegion &editor);
   static void BumpFromCol32(MaterialRegion &editor);

   static void MulTexCol     (MaterialRegion &editor);
   static void MulTexNormal  (MaterialRegion &editor);
   static void MulTexSmooth  (MaterialRegion &editor);
   static void MulTexGlow    (MaterialRegion &editor);
   static void MulTexEmissive(MaterialRegion &editor);

   bool bigVisible()C;

   void   setRGB     (C Vec                   &srgb              );
   void   setNormal  (flt                    normal              );
   void   setSmooth  (flt                    smooth              );
   void   setReflect (flt reflect_min, flt reflect_max           );
   void resetAlpha   (                                           );
   void cull         (bool                      on               );
   void flipNrmY     (bool                      on               ); // 'rebuildBase' already calls 'setChanged' and 'toGui'
   void smoothIsRough(bool                      on               ); // 'rebuildBase' already calls 'setChanged' and 'toGui'
 //void maxTexSize   (Edit.MAX_TEX_SIZE         mts              ) {if(edit.max_tex_size       !=mts                                 ){        undos.set("mts"       ); edit.max_tex_size       =mts                              ; edit.   max_tex_size_time.getUTC(); setChanged(); toGui();}}
   void texDownsize  (TEX_SIZE_PLATFORM tsp, byte downsize       );  
   void texQuality   (Edit::Material::TEX_QUALITY q, bool undo=true); // 'rebuildBase' already calls 'setChanged' and 'toGui'

   virtual void resizeBase(C VecI2 &size, bool relative=false);
   virtual void resizeBase0(C VecI2 &size, bool relative=false);
   virtual void resizeBase1(C VecI2 &size, bool relative=false);
   virtual void resizeBase2(C VecI2 &size, bool relative=false);
   void bumpFromCol(int blur);

   virtual   EditMaterial& getEditMtrl(); 
   virtual C ImagePtr    & getBase0   (); 
   virtual C ImagePtr    & getBase1   (); 
   virtual C ImagePtr    & getBase2   (); 
   virtual C ImagePtr    & getDetail  (); 
   virtual C ImagePtr    & getMacro   (); 
   virtual C ImagePtr    & getEmissive(); 
   virtual   bool          water      ()C;

   void setBottom(C Rect &prop_rect);
   void setSmoothParam();
   void create();

   // operations
   Image* getDetailBump(C Str &file);
   void resize();
   void toGui();
           void flush(C UID &elm_id);
   virtual void flush();
   virtual void setChanged();
   virtual void set(Elm *elm);
   virtual void activate(Elm *elm);
   virtual void toggle(Elm *elm);
           void            hideBig();        
   virtual MaterialRegion& show   ()override;
   virtual MaterialRegion& hide   ()override;

   void set(C MaterialPtr &mtrl);    
   void set(Memt<MaterialPtr> mtrls);
   void drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos);
   enum TEX_CHANNEL_TYPE : byte
   {
      TC_COLOR ,
      TC_ALPHA ,
      TC_NORMAL,
      TC_ROUGH ,
      TC_SMOOTH,
      TC_METAL ,
      TC_SPEC  ,
      TC_AO    ,
      TC_BUMP  ,
      TC_GLOW  ,
      TC_NUM   ,
   };
   class ImageSource : FileParams
   {
      class TexChannel
      {
         TEX_CHANNEL_TYPE type;
         int              pos;

         TexChannel&set (TEX_CHANNEL_TYPE type);                                  
         TexChannel&find(C Str &name, C Str &text, bool case_sensitive=false, WHOLE_WORD whole_word=WHOLE_WORD_NO);
         void fix();                      

         static int Compare(C TexChannel &a, C TexChannel &b);

public:
   TexChannel();
      };

      bool multi_channel, need_metal_channel;
      int  index, order, detected_channels, channel[TC_NUM];

      void set(C Str &name, int index);
      
      void process();
      void process(TEX_TYPE tex_type, EditMaterial &mtrl, bool append=false, bool multi_images=false);

public:
   ImageSource();
   };
   static int Compare(C ImageSource &a, C ImageSource &b);
   void drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos);

   virtual void rebuildBase(TEX_FLAG old_textures, uint changed_in_mtrl=0, bool adjust_params=true, bool always=false);
   virtual void rebuildDetail();
   virtual void rebuildMacro();
   virtual void rebuildEmissive(TEX_FLAG old_textures, bool adjust_params=true);

   virtual void elmChanged(C UID&mtrl_id);
   void erasing(C UID &elm_id);         

   bool winIOContains(GuiObj *go)C;

   // update
   virtual void update(C GuiPC &gpc)override;

public:
   MaterialRegion();
};
/******************************************************************************/
/******************************************************************************/
extern MaterialRegion MtrlEdit;
/******************************************************************************/
