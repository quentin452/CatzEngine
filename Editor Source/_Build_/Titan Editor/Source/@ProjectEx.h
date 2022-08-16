/******************************************************************************/
/******************************************************************************/
class ProjectEx : ProjectHierarchy
{
   class OuterRegion : Region
   {
      bool resize_on;

      void setRegionWidth(flt w);
      virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;
      virtual void update(C GuiPC &gpc)override;
      void draw();

      virtual GuiObj& show()override; // if there's no window active and no fullscreen editor then set keyboard focus
      virtual GuiObj& hide()override;

public:
   OuterRegion();
   };
   class ElmList : List<ListElm>
   {
      enum INCLUDE_CHILDREN_SIZE : byte
      {
         ICS_NEVER ,
         ICS_FOLDED,
         ICS_ALWAYS,
         ICS_NUM   ,
      };
      enum INCLUDE_TEXTURE_SIZE : byte
      {
         ITS_ELM    ,
         ITS_ELM_TEX,
         ITS_TEX    ,
         ITS_NUM    ,
      };
      class Warning : GuiImage
      {
         virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;

         void create(int data_abs_index, bool error, flt offset);
      };
      class SoundPlay : Button
      {
         UID lit_id, play_id;

public:
   SoundPlay();
      };
      class AppCheck : CheckBox
      {
         UID app_id;
         
         void setRect();

public:
   AppCheck();
      };

      INCLUDE_CHILDREN_SIZE ics;
      INCLUDE_TEXTURE_SIZE  its;
      bool                  show_all_elms, // if all elements are being listed
                            show_all_elm_types,
                            show_elm_type[ELM_NUM],
                            show_only_folder,
                            show_only_obj   ,
                            show_only_mtrl  ,
                            show_only_anim  ,
                            show_only_sound ,
                            file_size,
                            include_texture_size_in_object,
                            include_unpublished_elm_size,
                            flat_is,
                            flat_want,
                            list_all_children,
                            tapped_open;
      int                   tapped_vis, icon_col, name_col, size_col;
      flt                   tapped_time;
      UID                   lit_elm_id;
      Memc<ListElm>         data;
      Memx<Warning>         warnings;
      Memx<AppCheck>        app_checks;
      SoundPlay             sound_play;
      Menu                  menu;

      ListElm* curToListElm(              )C;
      ListElm* litToListElm(              )C;
      Elm*       operator()(              )C;
      EEItem*     visToItem(  int visible )C;
      EEItem*     absToItem(  int absolute)C;
      Elm*        visToElm (  int visible )C;
      Elm*        absToElm (  int absolute)C;
      int        itemToVis (C EEItem *data)C;
      int        itemToAbs (C EEItem *data)C;
      int         elmToVis (C Elm    *data)C;
      int         elmToAbs (C Elm    *data)C;

      bool identitySort()C; // if list is sorted in identity mode
      virtual void update(C GuiPC &gpc)override;
      virtual GuiObj& rect(C Rect &rect)override;

public:
   ElmList();
   };
   class ElmChange : Edit::_Undo::Change
   {
      enum TYPE
      {
         NONE      ,
         REMOVE    ,
         RESTORE   ,
         PUBLISH   ,
         SET_PARENT,
         SET_NAME  ,
      };

      TYPE                    type;
      uint                    test, set; // 'test'=mask for Elm.flags, 'set'=what to set
      Str                     name; // keep 'name' even though we could extract it from 'type', because 'type' will get changed in 'swap', while 'name' needs to remain constant
      Memc<UID>               elms;
      Memc<Edit::IDParam<UID>> elm_parents;
      Memc<Edit::IDParam<Str>> elm_names;

      virtual void swap(ptr user)override;

public:
   ElmChange();
   };

   Memc<UID>            existing_enums      , // binary sorted container of existing (not removed) ELM_ENUM      elements
                        existing_obj_classes, // binary sorted container of existing (not removed) ELM_OBJ_CLASS elements
                         publish_fonts      , // binary sorted container of existing (not removed) ELM_FONT      elements that are publishable
                        existing_apps       ; // binary sorted container of existing (not removed) ELM_APP       elements
   ComboBox             list_options;
   Button               show_removed, theater;
   TextLine             filter;
   bool                 filter_is_id;
   UID                  filter_id;
   Str                  filter_path; // temp variable used during 'setFilter' which holds the full path of current element
   OuterRegion          outer_region;
   Region               region;
   UID                  lit_elm_id;
   Str                  lit_elm_name;
   ElmList              list;
   UID                  list_cur;
   EEItem              *list_cur_item;
   Memc<UID>            list_sel, menu_list_sel, drag_list_sel;
   Memc<EEItem*>        list_sel_item, menu_list_sel_item;
   ImagePtr             icon_folder, icon_file, icon_obj, icon_class, icon_mesh, icon_mtrl, icon_water_mtrl, icon_phys_mtrl, icon_anim, icon_env, icon_world, icon_mini_map, icon_image, icon_image_atlas, icon_icon_setts, icon_icon, icon_enum, icon_font, icon_panel_image, icon_ts, icon_panel, icon_gui, icon_gui_skin, icon_sound, icon_video, icon_lib, icon_app, icon_code, icon_play, icon_stop, arrow_right, arrow_down, warning, exclamation, icon_light, icon_particles;
   MenuBar              menu;
   Node<MenuElm>        obj_class_node, param_type_node, font_node;
   Sound                sound;
   Matrix                    mesh_matrix;
   Mesh                      mesh_mem; // use 'Mesh' so it can store 'BoneMap'
   Skeleton                  skel_mem;
   EditSkeleton         edit_skel_mem;
   Skeleton             slot_skel_mem; // skeleton used for copying slots only
   flt                  save_time;
   Edit::Undo<ElmChange> elm_undos;
   bool                 texture_changed;
   bool                 file_size_getter_step; // 0=game files, 1=textures
   FileSizeGetter       file_size_getter;

   // normally following functions should use 'menu_list_sel' however they can be also called through 'list_options' which doesn't use that
   static void NewFolder     (ProjectEx &proj);
   static void NewMtrl       (ProjectEx &proj);
   static void NewWaterMtrl  (ProjectEx &proj);
   static void NewPhysMtrl   (ProjectEx &proj);
   static void NewAnim       (ProjectEx &proj);
   static void NewObject     (ProjectEx &proj);
   static void NewObjectClass(ProjectEx &proj);
   static void NewEnum       (ProjectEx &proj);
   static void NewImage      (ProjectEx &proj);
   static void NewImageAtlas (ProjectEx &proj);
   static void NewIconSetts  (ProjectEx &proj);
   static void NewIcon       (ProjectEx &proj);
   static void NewFont       (ProjectEx &proj);
   static void NewPanelImage (ProjectEx &proj);
   static void NewTextStyle  (ProjectEx &proj);
   static void NewPanel      (ProjectEx &proj);
   static void NewGuiSkin    (ProjectEx &proj);
   static void NewGui        (ProjectEx &proj);
   static void NewSound      (ProjectEx &proj);
   static void NewVideo      (ProjectEx &proj);
   static void NewRawFile    (ProjectEx &proj);
   static void NewLib        (ProjectEx &proj);
   static void NewApp        (ProjectEx &proj);
   static void NewCode       (ProjectEx &proj);
   static void NewEnv        (ProjectEx &proj);
   static void NewMiniMap    (ProjectEx &proj);
   static void NewWorld      (ProjectEx &proj);

   static void ElmRename     (ProjectEx &proj);
   static void ReplaceName   (ProjectEx &proj);
   static void Remove        (ProjectEx &proj);
   static void Restore       (ProjectEx &proj);
   static void Reload        (ProjectEx &proj);
   static void CancelReload  (ProjectEx &proj);
   static void SplitAnim     (ProjectEx &proj);
   static void UndoElmChange (ProjectEx &proj);
   static void RedoElmChange (ProjectEx &proj);
   static void Duplicate     (ProjectEx &proj);
   static void CopyTo        (ProjectEx &proj);
   static void Expand        (ProjectEx &proj);
   static void ExpandAll     (ProjectEx &proj);
   static void Collapse      (ProjectEx &proj);
   static void ExploreElm    (ProjectEx &proj);
   static void Export        (ProjectEx &proj);
   static void Properties    (ProjectEx &proj);
   static void FilterChanged (ProjectEx &proj);
   static void ShowRemoved   (ProjectEx &proj);
   static void ShowTheater   (ProjectEx &proj);
   static void SoundPlay     (ProjectEx &proj);

   static void  EnablePublish      (ProjectEx &proj);
   static void DisablePublish      (ProjectEx &proj);
   static void  EnablePublishMobile(ProjectEx &proj);
   static void DisablePublishMobile(ProjectEx &proj);

   static void ImageMipMapOn (ProjectEx &proj);
   static void ImageMipMapOff(ProjectEx &proj);

   static void ImageResize0   (ProjectEx &proj);
   static void ImageResize16  (ProjectEx &proj);
   static void ImageResize32  (ProjectEx &proj);
   static void ImageResize64  (ProjectEx &proj);
   static void ImageResize128 (ProjectEx &proj);
   static void ImageResize256 (ProjectEx &proj);
   static void ImageResize512 (ProjectEx &proj);
   static void ImageResize1024(ProjectEx &proj);
   static void ImageResize2048(ProjectEx &proj);

   static void MtrlSetRGB1             (ProjectEx &proj);
   static void MtrlSetRGB              (ProjectEx &proj);
   static void MtrlMulRGB              (ProjectEx &proj);
   static void MtrlSetRGBCur           (ProjectEx &proj);
   static void MtrlSetTechCur          (ProjectEx &proj);
   static void MtrlSetBumpCur          (ProjectEx &proj);
   static void MtrlSetNormalCur        (ProjectEx &proj);
   static void MtrlSetSmoothCur        (ProjectEx &proj);
   static void MtrlSetReflectCur       (ProjectEx &proj);
   static void MtrlSetGlowCur          (ProjectEx &proj);
   static void MtrlResetAlpha          (ProjectEx &proj);
   static void MtrlCullOn              (ProjectEx &proj);
   static void MtrlCullOff             (ProjectEx &proj);
   static void MtrlFlipNrmYOn          (ProjectEx &proj);
   static void MtrlFlipNrmYOff         (ProjectEx &proj);
   static void MtrlReloadBaseTex       (ProjectEx &proj);

   static void MtrlSetColorTexCur      (ProjectEx &proj);
   static void MtrlSetBumpTexCur       (ProjectEx &proj);
   static void MtrlSetNormalTexCur     (ProjectEx &proj);
   static void MtrlSetSmoothTexCur     (ProjectEx &proj);
   static void MtrlSetMetalTexCur      (ProjectEx &proj);
   static void MtrlSetGlowTexCur       (ProjectEx &proj);

   static void MtrlMulTexCol           (ProjectEx &proj);
   static void MtrlMulTexNormal        (ProjectEx &proj);
   static void MtrlMulTexSmooth        (ProjectEx &proj);
   static void MtrlMulTexGlow          (ProjectEx &proj);
   static void MtrlMulTexEmissive      (ProjectEx &proj);
   static void MtrlMoveToObj           (ProjectEx &proj);
   static void MtrlMerge               (ProjectEx &proj);
   static void MtrlConvertToAtlas      (ProjectEx &proj);
   static void MtrlConvertToDeAtlas    (ProjectEx &proj);
   static void MtrlTexSizeMobileFull   (ProjectEx &proj);
   static void MtrlTexSizeMobileHalf   (ProjectEx &proj);
   static void MtrlTexSizeMobileQuarter(ProjectEx &proj);
   static void MtrlTexSizeMobileEighth (ProjectEx &proj);
   static void MtrlTexSizeSwitchFull   (ProjectEx &proj);
   static void MtrlTexSizeSwitchHalf   (ProjectEx &proj);
   static void MtrlTexSizeSwitchQuarter(ProjectEx &proj);
   static void MtrlTexSizeSwitchEighth (ProjectEx &proj);

   static void AnimClip   (ProjectEx &proj);
   static void AnimLinear (ProjectEx &proj);
   static void AnimCubic  (ProjectEx &proj);
   static void AnimLoopOn (ProjectEx &proj);
   static void AnimLoopOff(ProjectEx &proj);

   static void SoundImportAs(ptr mode);       
   static void SoundMulVolume(ProjectEx&proj);

   static void TransformSet       (ProjectEx &proj);
   static void TransformApply     (ProjectEx &proj);
   static void TransformBottom    (ProjectEx &proj);
   static void TransformBack      (ProjectEx &proj);
   static void TransformCenter    (ProjectEx &proj);
   static void TransformCenterXZ  (ProjectEx &proj);
   static void TransformRotYMinBox(ProjectEx &proj);

   static void MeshRemVtxTex1        (ProjectEx &proj);
   static void MeshRemVtxTex2        (ProjectEx &proj);
   static void MeshRemVtxTex3        (ProjectEx &proj);
   static void MeshRemVtxTex123      (ProjectEx &proj);
   static void MeshRemVtxCol         (ProjectEx &proj);
   static void MeshRemVtxSkin        (ProjectEx &proj);
   static void MeshDisableLQLODs     (ProjectEx &proj);
   static void MeshMergeCoplanarFaces(ProjectEx &proj);
   static void SetBody               (ProjectEx &proj);
                                    
   void columnVisible(int column, bool visible);

   static void ShowFileSize(ProjectEx &proj);
          void showFileSize();
   static void IncludeUnpublishedElmSize(ProjectEx &proj);
          void includeUnpublishedElmSize(bool on);
   static void IncludeTextureSizeInObject(ProjectEx &proj);
          void includeTextureSizeInObject(bool on);
   void checkIncludeChildrenSize();
   void checkIncludeTextureSize();
   static void IncludeChildrenSize(ptr                           ics); 
          void includeChildrenSize(ElmList::INCLUDE_CHILDREN_SIZE ics);
   static void IncludeTextureSize(ptr                          its); 
          void includeTextureSize(ElmList::INCLUDE_TEXTURE_SIZE its);
   static void ListOnlyFolder(ProjectEx &proj);                    
   static void ListOnlyObj   (ProjectEx &proj);                    
   static void ListOnlyMtrl  (ProjectEx &proj);                    
   static void ListOnlyAnim  (ProjectEx &proj);                    
   static void ListOnlySound (ProjectEx &proj);                    
   static void ListFlat      (ProjectEx &proj);                    

   void setListPadding();

   // get
 C ImagePtr& elmIcon(ELM_TYPE type);
   bool contains(C Elm &a, C Elm *b)C;      // if 'a' contains 'b'
   Enum* getEnumFromName(C Str &enum_name);
 C UID& curApp(); // warning: this relies on 'existing_apps' being setup properly
   GuiSkinPtr appGuiSkin()C;      
   bool includeTex(C UID &tex_id);

   Elm* getObjMeshElm(C UID &obj_id, bool send_to_server=true, bool set_gui=true);
   Elm* getObjSkelElm(C UID &obj_id, bool send_to_server=true, bool set_gui=true);
   Elm* getObjPhysElm(C UID &obj_id, bool send_to_server=true, bool set_gui=true);

   // create
   static cchar8 *explore_desc,
             *dis_publish_desc,
             * en_publish_desc,
             *     reload_desc;
   void setNewElm(Node<MenuElm> &n, C Str &prefix=S);
   void disableMenu(Menu &menu);
   void updateMenu();
   void create();

   // operations
   void explore(Elm &elm);
   Str newElmName(ELM_TYPE type, C UID &parent_id);
   void newElm(ELM_TYPE type);
   Elm* newElm(ELM_TYPE type, C UID &parent_id, C Str *name=null, bool refresh_elm_list=true);
   Elm* newWorld(C Str &name, int area_size, int hm_res, C UID &parent_id, bool set_cur);
   void setMtrl(Elm &mtrl, C XMaterialEx &src, C Str &src_file=S);
   Elm& newMtrl(C XMaterialEx &src, C UID parent_id=UIDZero, C Str &src_file=S); // create new material from 'src' !! this doesn't set elm list and doesn't send to the server !!

   void setElmName(Elm &elm, C Str &name, C TimeStamp &time=TimeStamp().getUTC());
   void setElmNames(Memc<Edit::IDParam<Str>> &elms, bool adjust_elms=false); // 'adjust_elms'=if this is performed because of undo, and in that case we need to remember current names, so we can undo this change later
   void renameElm(C UID &elm_id, C Str &name);
   void replaceName(C MemPtr<UID> &elm_ids, C Str &from, C Str &to);

   void closeElm(C UID &elm_id); // close elements that keep handles to files (sounds/vidoes)
   void paramEditObjChanged(C UID *obj_id=null);

   // remove
   void remove(ElmNode &node, Memc<UID> &sorted_ids, Memc<UID> &removed, C TimeStamp &time); // process recursively to remove only parents without their children
   void remove(Memc<UID> &ids, bool parents_only, bool set_undo=true); // !! WARNING: MIGHT SORT 'ids' !!
   void restore(Memc<UID> &ids, bool set_undo=true);

   // publish
   void _setElmPublish(ElmNode &node, Memc<UID> &sorted_ids, uint test, uint set, Memc<UID> &processed, C TimeStamp &time); // process recursively to disable only parents without their children
   void _setElmPublish(Memc<UID> &ids, uint test, uint set, bool parents_only, bool set_undo); // !! WARNING: MIGHT SORT 'ids' !!
   void setElmPublish(Memc<UID> &ids, sbyte all, sbyte mobile, bool parents_only, bool set_undo=true); // !! WARNING: MIGHT SORT 'ids' !!
   void disablePublish(Memc<UID> &ids, bool parents_only, bool set_undo=true); // !! WARNING: MIGHT SORT 'ids' !!

   void reload(Memc<UID> &elm_ids);
   void cancelReload(C MemPtr<UID> &elm_ids);
   void removeMeshVtx(C MemPtr<UID> &elm_ids, MESH_FLAG flag);
   void disableLQLODs(C MemPtr<UID> &elm_ids);
   void meshMergeCoplanarFaces(C MemPtr<UID> &elm_ids);
   bool forceImageSize(Str &file, C VecI2 &size, bool relative);
   bool forceImageSize(Str &file, C VecI2 &size, bool relative, TimeStamp &file_time, C TimeStamp &time);
   bool imageMipMap(C MemPtr<UID> &elm_ids, bool on);
   void imageResize(C MemPtr<UID> &elm_ids, C VecI2 &size);
   void mtrlResetAlpha(C MemPtr<UID> &elm_ids);
   void mtrlSetRGB(C MemPtr<UID> &elm_ids, C Vec &srgb, bool mul=false);
   bool mtrlSetTech(C MemPtr<UID> &elm_ids, MATERIAL_TECHNIQUE tech);
   bool mtrlSetBump(C MemPtr<UID> &elm_ids, flt bump, bool mul=false);
   bool mtrlSetNormal(C MemPtr<UID> &elm_ids, flt normal, bool mul=false);
   bool mtrlSetSmooth(C MemPtr<UID> &elm_ids, flt smooth, bool mul=false);
   bool mtrlSetReflect(C MemPtr<UID> &elm_ids, flt reflect_min, flt reflect_max);
   bool mtrlSetGlow(C MemPtr<UID> &elm_ids, flt glow, bool mul=false);
   bool mtrlSetTexColor(C MemPtr<UID> &elm_ids, C Str &color_map);
   bool mtrlSetTexBump(C MemPtr<UID> &elm_ids, C Str &bump_map);
   bool mtrlSetTexNormal(C MemPtr<UID> &elm_ids, C Str &normal_map, bool flip_normal_y);
   bool mtrlSetTexSmooth(C MemPtr<UID> &elm_ids, C Str &smooth_map, bool smooth_is_rough);
   bool mtrlSetTexMetal(C MemPtr<UID> &elm_ids, C Str &metal_map);
   bool mtrlSetTexGlow(C MemPtr<UID> &elm_ids, C Str &glow_map);
   void mtrlCull(C MemPtr<UID> &elm_ids, bool on);
   void mtrlFlipNrmY(C MemPtr<UID> &elm_ids, bool on);
   void mtrlSmoothIsRough(C MemPtr<UID> &elm_ids, bool on);
   void mtrlTexDownsize(C MemPtr<UID> &elm_ids, TEX_SIZE_PLATFORM tsp, byte downsize, C UID &base_0=UIDZero, C UID &base_1=UIDZero, C UID &base_2=UIDZero, bool precise=false); // 'precise'=if do a more precise but slower processing (if true then all materials from 'elm_ids' will have to be loaded from disk for checking, which might be slow)
   bool mtrlTexQuality(C MemPtr<UID> &elm_ids, Edit::Material::TEX_QUALITY quality, C UID &base_0=UIDZero, C UID &base_1=UIDZero, C UID &base_2=UIDZero, bool precise=false); // 'precise'=if do a more precise but slower processing (if true then all materials from 'elm_ids' will have to be loaded from disk for checking, which might be slow) // !! this is not perfect !!
   bool mtrlMulTexCol(C MemPtr<UID> &elm_ids);
   bool mtrlMulTexNormal(C MemPtr<UID> &elm_ids);
   bool mtrlMulTexSmooth(C MemPtr<UID> &elm_ids);
   bool mtrlMulTexGlow(C MemPtr<UID> &elm_ids);
   bool mtrlMulTexEmissive(C MemPtr<UID> &elm_ids);
   void mtrlMoveToObj(C MemPtr<UID> &elm_ids);
   bool mtrlGet(C UID &elm_id, EditMaterial &mtrl);
   bool mtrlGet(C UID &elm_id, EditWaterMtrl &mtrl);
   bool mtrlSync(C UID &elm_id, C EditMaterial &mtrl, bool reload_textures, bool adjust_params, cptr undo_change_type="sync");
   bool mtrlSync(C UID &elm_id, C EditWaterMtrl &mtrl, bool reload_textures, bool adjust_params, cptr undo_change_type="sync");
   bool mtrlSync(C UID &elm_id, C Edit::Material &mtrl, bool reload_textures, bool adjust_params);
   TEX_FLAG createBaseTextures(Image &base_0, Image &base_1, Image &base_2, C EditMaterial &material, bool changed_flip_normal_y=false, bool changed_smooth_is_rough=false);
   TEX_FLAG createBaseTextures(Image &base_0, Image &base_1, Image &base_2, C EditWaterMtrl &material, bool changed_flip_normal_y=false, bool changed_smooth_is_rough=false);
   TEX_FLAG mtrlCreateBaseTextures(EditMaterial &material, uint changed=0); // 'changed'=only CHANGED_FLIP_NRM_Y or CHANGED_SMOOTH_IS_ROUGH are valid
   TEX_FLAG mtrlCreateBaseTextures(EditWaterMtrl &material, uint changed=0); // 'changed'=only CHANGED_FLIP_NRM_Y or CHANGED_SMOOTH_IS_ROUGH are valid
   /*bool mtrlCreateReflectionTexture(Image &reflection, C EditMaterial &material)
   {
      bool loaded=false;
      reflection.del();
      Mems<FileParams> faces=FileParams.Decode(material.reflection_map);
      if(faces.elms()==6) // 6 images specified
      {
         Image images[6]; REPA(images)
         {
            Image &image=images[i];
            Str name=faces[i].name; UID image_id; if(DecodeFileName(name, image_id))name=editPath(image_id);else image_id.zero();
            if(ImportImage(image, name, -1, IMAGE_SOFT, 1, true))
            {
               loaded=true;
               if(Elm *elm=findElm(image_id))if(ElmImage *data=elm.imageData())if(IsCube(data.mode))image.crop(image, i*image.w()/6, 0, image.w()/6, image.h()); // crop to i-th face for cubes, we need to check ElmImage for cube, and not 'image.mode' because Cube ELM_IMAGE are stored in 6x1 Soft in editPath
            }
         }
         if(loaded) // create only if any of the images were loaded, otherwise we don't need it
         {
            ImagePtr cur_reflection;
            REPA(images) // check if any of the images didn't load
            {
               Image &image=images[i]; if(!image.is())
               {
                  if(!cur_reflection)cur_reflection=texPath(material.reflection_tex); // load existing reflection map
                  if( cur_reflection)cur_reflection->extractMipMap(image, -1, 0, GetCubeDir(i)); // extract from existing reflection map
               }
            }
            reflection.ImportCubeTry(images[2], images[0], images[5], images[4], images[1], images[3], -1, true, 1, true);
         }
      }else
      if(faces.elms()) // if at least one face is specified
      {
         Str name=material.reflection_map; UID image_id; if(DecodeFileName(name, image_id))name=editPath(image_id);
         if(ImportImage(reflection, name))
         {
            loaded=true;
            int res=NearestPow2(reflection.h());
            reflection.copyTry(reflection, res, res, 1, -1, IMAGE_SOFT_CUBE, 1);
         }
      }
      return !faces.elms() || loaded;
   }
   void mtrlCreateReflectionTexture(EditMaterial &material)
   {
      Image reflection; if(mtrlCreateReflectionTexture(reflection, material)) // proceed only if there's no source, or succeeded with importing, this is to avoid clearing existing texture when failed to load
      {
         ImageProps(reflection, &material.reflection_tex, null, SRGB|IGNORE_ALPHA); material.reflection_map_time.getUTC(); // in order for 'reflection_tex' to sync, 'reflection_map_time' time must be changed
         if(reflection.is())
         {
            if(includeTex(material.reflection_tex))
            {
               reflection.copyTry(reflection, -1, -1, -1, IMAGE_BC1_SRGB, IMAGE_CUBE, 1);
               saveTex(reflection, material.reflection_tex);
            }
            Server.setTex(material.reflection_tex);
         }
      }
   }*/
   TEX_FLAG mtrlCreateDetailTexture(EditMaterial &material);
   TEX_FLAG mtrlCreateMacroTexture(EditMaterial &material);
   TEX_FLAG mtrlCreateEmissiveTexture(EditMaterial &material);
   bool mtrlReloadTextures(C UID &elm_id, bool base, bool detail, bool macro, bool emissive);
   void mtrlReloadTextures(C MemPtr<UID> &elm_ids, bool base, bool detail, bool macro, bool emissive);
   bool skelGet(C UID &elm_id, Skeleton &skel)C;
   Animation* getAnim(C UID &elm_id, Animation &temp)C;
   bool animGet(C UID &elm_id, Animation &anim)C;
   bool animSet(C UID &elm_id, C Animation &anim);
   void setAnimParams(Elm &elm_anim);
   void animClip(C MemPtr<UID> &elm_ids);
   void animLinear(C MemPtr<UID> &elm_ids, bool linear);
   void animLoop(C MemPtr<UID> &elm_ids, bool loop);
   void animSetTargetSkel(C MemPtr<UID> &anim_ids, C UID &skel_id);
   void transformSet(C MemPtr<UID> &elm_ids);
   void transformApply(C MemPtr<UID> &elm_ids);
   void transformApply(C MemPtr<UID> &elm_ids, C Matrix &matrix);
   void transformBottom(C MemPtr<UID> &elm_ids);
   void transformBack(C MemPtr<UID> &elm_ids);
   void transformCenter(C MemPtr<UID> &elm_ids);
   void transformCenterXZ(C MemPtr<UID> &elm_ids);
   void transformRotYMinBox(C MemPtr<UID> &elm_ids);
   void objSetBody(C MemPtr<UID> &elm_ids, C UID &body_id);

   void soundImportAs(C MemPtr<UID> &elm_ids, SOUND_CODEC codec=SOUND_NONE, int rel_bit_rate=-1);
   void mulSoundVolume(C MemPtr<UID> &elm_ids, flt volume);

   void adjustAnimations(C UID &skel_id, C EditSkeleton &old_edit_skel, C Skeleton &old_skel, C Skeleton &new_skel, C MemPtr<Mems<IndexWeight>> &bone_weights, int old_bone_as_root=-1);
   void offsetAnimations(C Skeleton &old_skel, C Skeleton &new_skel, C UID &skel_id, C UID &ignore_anim_id=UIDZero);
 /*void updateSkelBoneTypes()
   {
      REPA(elms)
      {
         Elm &elm=elms[i]; if(elm.type==ELM_SKEL)
         {
            Skeleton skel; if(skel.load(gamePath(elm)))
            {
               Skeleton temp=skel; temp.setBoneTypes();
               if(conditionalAdjustAnimations(elm.id, skel, temp, null))
               {
                  temp.save(gamePath(elm));
                  elm.skelData().newVer();
                  elm.skelData().file_time.getUTC();
               }
            }
         }
      }
   }*/

   bool validElm(Elm &elm);
   void duplicate(Memc<UID> &ids, MemPtr<UID> duplicated=null, C Str &suffix=" Copy");

   virtual void eraseElm(C UID &elm_id)override;
   virtual bool eraseTex(C UID &tex_id)override;
   virtual void eraseWorldAreaObjs(C UID &world_id, C VecI2 &area_xy)override;
   virtual void eraseRemoved(bool full)override;

   void elmChangedParentRemovePublish(bool network=false);

   void setElmParent(Memc<Edit::IDParam<UID>> &elms, bool adjust_elms=false, bool as_undo=false); // 'adjust_elms'=if this is performed because of undo, and in that case we need to remember current parents, so we can undo this change later
   void drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos);
   void collapse (Memc<UID> &ids, Memc<EEItem*> &items);           
   void expand   (Memc<UID> &ids, Memc<EEItem*> &items);           
   void expandAll(Memc<UID> &ids, Memc<EEItem*> &items);
   void expandAll(ElmNode &node);

   void floodExisting(ElmNode &node, uint publish_flag, bool parent_publish=true);
   void setExisting();

   static int CompareEnum(C Str &a, C Str &b);
   void enumChanged(C UID &enum_id=UIDZero); // 'enum_id' can also point to OBJ_CLASS
   void meshVariationChanged();
   void fontChanged();
   void panelChanged(C UID &elm_id);
   void panelImageChanged(C UID &elm_id);
   void mtrlTexChanged();

   // update
   static void DragElmsStart(ProjectEx &proj);
   void dragElmsStart();
   static void DragElmsCancel(ProjectEx &proj);
   static void DragElmsFinish(ProjectEx &proj, GuiObj *obj, C Vec2 &screen_pos);
   void update();

   // gui
   static int CompareChildren(C int &a, C int &b);
   static int CompareChildren(C EEItem &a, C Elm &b);
   bool hasInvalid(ElmNode &node);
   static uint PublishableFlag(Edit::EXE_TYPE exe_type=CodeEdit.configEXE());
   void getPublishElms(Memt<Elm*> &app_elms, ElmNode &node, uint publish_flag);
   void getActiveAppElms(Memt<Elm*> &app_elms, ElmNode &node, uint publish_flag, C UID&app_id, bool inside_valid);
   void getPublishElms  (Memt<Elm*> &app_elms, Edit::EXE_TYPE exe_type);                                        
   void getActiveAppElms(Memt<Elm*> &app_elms, Edit::EXE_TYPE exe_type);                                         // set 'false' to ignore sources placed in root

   void activateSources(int rebuild=0); // -1=never, 0=auto, 1=always
   static void ActivateApp(ElmList::AppCheck &app_check);
          bool activateApp(C UID            &elm_id   );
   void setList(EEItem &item, int depth, bool parent_contains_name);
   void includeElmFileSize(ListElm &e, ElmNode &node);
   void includeElmNotVisibleFileSize(ListElm &e, ElmNode &node);
   bool hasVisibleChildren(C ElmNode &node)C;
   void setList(ElmNode &node, int depth=0, int vis_parent=-1, bool parent_contains_name=false, bool parent_no_publish_all=false);
   bool setFilter(ElmNode &node);
   bool setFilter(Memx<EEItem> &items);
   ProjectEx& editing(C UID &elm_id, bool force_open_parents=true);
   bool editing(Memx<EEItem> &items, C Str &name);
   ProjectEx& editing(C Str &name);                              
   void setList(bool set_hierarchy=true, bool set_existing=true); // !! CALL 'setListCurSel' or 'clearListSel' before this !!
   void clearListSel(); // clear list selection
   void setListSel(Memc<UID> &list_sel, MemPtr<EEItem*> list_sel_item=null);
   void setListCurSel(); // needs to be called before adding/removing elements from 'elms'
   void setMenuListSel();
   ProjectEx& refresh(bool set_hierarchy=true, bool set_existing=true);
   void elmOpenParents(C UID &id, bool set_list=true);
   void elmSelect(C MemPtr<UID> &elm_ids);
   bool elmReload(C MemPtr<UID> &elm_ids, bool remember_result=false, bool refresh_elm_list=true);
   void elmLocate(C UID &id, bool set_cur=false);
   void elmHighlight(C UID &id, C Str &name, bool force=false);
   void elmToggle(EEItem *item);
   void elmToggle(Elm *elm);
   void elmToggle(ListElm *elm);
   void elmToggle(C UID &id); 
   void elmActivate(Elm *elm);
   void elmNext(C UID &elm_id, int dir=1);

   void resize();
   void elmMenu(C Vec2 &pos, bool touch=false);
   bool drop(C Str &name, C UID &parent_id, Memt<UID> &new_folders);
   void drop(Memc<Str> &names, C UID &parent_id);
   void drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos);

   // process dependencies
   virtual void meshSetAutoTanBin(Elm &elm, C MaterialPtr &material)override;
   virtual void animTransformChanged(Elm &elm_anim)override;
   virtual void skelTransformChanged(C UID &skel_id)override;
   virtual void objChanged(Elm &obj)override;
   virtual void meshChanged(Elm &mesh)override;
   void elmChanging(Elm &elm);
   void elmChanged(Elm &elm);
   
   // sync
   void receivedData(Elm &elm, File &data, File &extra);
   void syncElm(Elm &elm, Elm &src, File &src_data, File &src_extra, bool sync_long);
   virtual bool syncWaypoint(C UID &world_id, C UID &waypoint_id, Version &src_ver, EditWaypoint &src)override;
   virtual bool syncLake(C UID &world_id, C UID &lake_id, Version &src_ver, Lake &src)override;
   virtual bool syncRiver(C UID &world_id, C UID &river_id, Version &src_ver, River &src)override;

   virtual void hmDel(C UID &world_id, C VecI2 &area_xy, C TimeStamp *time=null)override;
   virtual Heightmap* hmGet(C UID &world_id, C VecI2 &area_xy, Heightmap &temp)override;
   virtual uint hmUpdate(C UID &world_id, C VecI2 &area_xy, uint area_sync_flag, C AreaVer &src_area, Heightmap &src_hm)override;
   virtual void objGet(C UID &world_id, C VecI2 &area_xy, C Memc<UID> &obj_ids, Memc<ObjData> &objs)override; // assumes that 'obj_ids' is sorted
   virtual bool syncObj(C UID &world_id, C VecI2 &area_xy, Memc<ObjData> &objs, Map<VecI2, Memc<ObjData>> *obj_modified=null, Memc<UID> *local_newer=null)override;

   void syncCodes();
   void syncCodes(C Memc<ElmCodeData> &elm_code_datas);
   void syncCodes(C Memc<ElmCodeBase> &elm_code_bases, bool resync);

   // Editor Network Interface
   bool codeGet(C UID &elm_id, Str &data);
   bool codeSet(C UID &elm_id, C Str &data);

   bool fileRead(C UID &elm_id, File &f);
   bool fileSet(C UID &elm_id, File &f);

   bool imageGet(C UID &elm_id, Image &image);
   bool imageSet(C UID &elm_id, File &image, bool has_color, bool has_alpha);
   bool imageSet(C UID &elm_id, C Image &image);
   
   bool meshSet(C UID &elm_id, File &data);
   bool physSet(C UID &elm_id, File &data);
   bool skelSetSlots(C UID &elm_id, C CMemPtr<Edit::SkeletonSlot> &skel_slots);

   bool modifyObj(C UID &obj_id, C MemPtr<Edit::ObjChange> &changes);

   // External Code Sync
   bool codeCheck();
   Str codeSyncPath();
   void codeExplore();        
   bool codeExport (bool gui);
   bool codeImport(bool gui);

   // io
   virtual void flush(SAVE_MODE save_mode=SAVE_DEFAULT)override;
   void flushElm(C UID &elm_id);

   void getFileSizes();
   void savedTex(C UID &tex_id, int size);    
   bool  saveTex(C Image &img, C UID &tex_id);

   void pauseServer();
   void resumeServer();
   virtual LOAD_RESULT open(C UID &id, C Str &name, C Str &path, Str &error, bool ignore_lock=false)override;
   virtual bool save(SAVE_MODE save_mode=SAVE_DEFAULT)override;
   virtual ProjectEx& del()override;
   virtual void close()override;
   void pause();
   void resume();

   // outer region
   inline   void  hide   (            ); 
   inline   bool  visible(            )C;
   inline   void  visible(  bool  on  ); 
   inline   void  draw   (            ); 
   inline C Rect& rect   (            ); 
   inline   void  rect   (C Rect &rect); 
   inline   Vec2  size   (            ); 

public:
   ProjectEx();
};
/******************************************************************************/
/******************************************************************************/
extern ProjectEx Proj;
/******************************************************************************/
