/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
ConvertToAtlasClass ConvertToAtlas;
/******************************************************************************/

/******************************************************************************/
   cchar8 *ConvertToAtlasClass::mode_t[]=
   {
      "Create as new objects (use for testing)",
      "Replace existing objects (and keep Original Elements)",
      "Replace existing objects (and Disable Publishing of Original Elements)",
      "Replace existing objects (and Remove Original Elements)",
   };
   const flt ConvertToAtlasClass::h=0.05f;
/******************************************************************************/
      void ConvertToAtlasClass::Mtrl::setScaleText(C VecI2 &size) {t_scaled_size.set(TexSize(size));}
      void ConvertToAtlasClass::Mtrl::setScale(             ) {scaled_size=Round(original_size*scale*ConvertToAtlas.scale); setScaleText(scaled_size);}
      void ConvertToAtlasClass::Mtrl::setScale(C Vec2 &scale) {T.scale=scale; setScale();}
      void ConvertToAtlasClass::Mtrl::posY(flt y        )
      {
         del .pos(Vec2(del .pos().x, y));
         name.pos(Vec2(name.pos().x, y));
         y-=del.rect().h()/2;
         prop           .pos(Vec2(prop.text      .pos().x, y));
         t_original_size.pos(Vec2(t_original_size.pos().x, y));
         t_scaled_size  .pos(Vec2(t_scaled_size  .pos().x, y));
      }
      void ConvertToAtlasClass::Preview::draw(C GuiPC &gpc)
{
         if(gpc.visible && visible())
         {
            D.clip(gpc.clip);
            Rect r=rect()+gpc.offset;
            if(ConvertToAtlas.tex_size.all())
            {
               r=Fit(flt(ConvertToAtlas.tex_size.x)/ConvertToAtlas.tex_size.y, r);
               ALPHA_MODE alpha=D.alpha(ALPHA_NONE);
               REPA(ConvertToAtlas.mtrls)
               {
                C Mtrl &mtrl=ConvertToAtlas.mtrls[i];
                  Rect  mr  =mtrl.packed_rect; mr/=Vec2(ConvertToAtlas.tex_size);
                  mr.setY(1-mr.max.y, 1-mr.min.y); // inverse Y because textures coordinates increasing down, and gui drawing increasing up
                  mr.min=r.lerp(mr.min);
                  mr.max=r.lerp(mr.max);
                  if(Image *image=mtrl.base_0())
                  {
                     if(mtrl.rotated)image->drawVertical(mr);
                     else            image->draw        (mr);
                  }
                  mr.draw(RED, false);
               }
               D.alpha(alpha);
            }else
            if(ConvertToAtlas.mtrls.elms())
            {
               D.text(r, "Textures too big\nDecrease scale");
            }
            r.draw(Gui.borderColor(), false);
         }
      }
      Str  ConvertToAtlasClass::Warning::MeshName(C Error &error) {return Proj.elmFullName(error.mesh_id);}
      Str  ConvertToAtlasClass::Warning::MtrlName(C Error &error) {return Proj.elmFullName(error.mtrl_id);}
      void ConvertToAtlasClass::Warning::Proceed(ptr) {ConvertToAtlas.convertPerform();}
      void ConvertToAtlasClass::Warning::create()
      {
         Gui+=super::create(Rect_C(0, 0, 1.7f, 1), "Convert To Atlas").hide(); button[2].show();
         T+=text.create(Vec2(clientWidth()/2, -0.06f), "Warning: Following meshes use wrapped texture coordinates.\nConverting to Atlas may introduce some artifacts.");
         T+=proceed.create(Rect_D(clientWidth()/2, -clientHeight()+0.03f, 0.34f, 0.055f), "Proceed Anyway").func(Proceed);
         T+=region.create(Rect(0, proceed.rect().max.y, clientWidth(), text.rect().min.y-0.04f).extend(-0.02f, -0.03f));
         ListColumn lc[]=
         {
            ListColumn(MeshName, LCW_DATA, "Mesh"),
            ListColumn(MtrlName, LCW_DATA, "Material"),
            ListColumn(MEMBER(Error, error), 0.18f, "UV Error"),
         };
         region+=list.create(lc, Elms(lc));
      }
      void ConvertToAtlasClass::Warning::display()
      {
         list.setData(ConvertToAtlas.errors);
         activate();
      }
   Str  ConvertToAtlasClass::TexSize(C VecI2 &size) {return S+size.x+'x'+size.y;}
   void ConvertToAtlasClass::Scale(Mtrl &mtrl, C Str &text) {mtrl.setScale(TextVec2Ex(text)); ConvertToAtlas.refresh();}
   Str  ConvertToAtlasClass::Scale(C Mtrl &mtrl) {return TextVec2Ex(mtrl.scale);}
   void ConvertToAtlasClass::Del(Mtrl &mtrl) {ConvertToAtlas.mtrls.removeData(&mtrl, true); ConvertToAtlas.refresh();}
   void ConvertToAtlasClass::Refresh(C Property &prop) {ConvertToAtlas.refresh();}
   void ConvertToAtlasClass::ChangedScale(C Property &prop) {REPAO(ConvertToAtlas.mtrls).setScale(); ConvertToAtlas.refresh();}
   int  ConvertToAtlasClass::CompareData(C Mtrl &a, C Mtrl &b) {return ComparePathNumber(a.name(), b.name());}
   void ConvertToAtlasClass::Convert(ConvertToAtlasClass &cta) {cta.convertDo();}
   void ConvertToAtlasClass::setErrors(Mesh &mesh, C Memc<UID> &mtrl_ids, C UID &mesh_id)
   {
      const Rect tex_rect(0, 1);
      REPD(l, mesh.lods())
      {
         MeshLod &lod=mesh.lod(l); REPA(lod)
         {
            MeshPart &part=lod.parts[i];
            MeshBase &base=part.base; if(base.vtx.tex0())REP(4)
            {
               UID mtrl_id=part.multiMaterial(i).id(); if(mtrl_id.valid() && mtrl_ids.binaryHas(mtrl_id))
               {
                  base.explodeVtxs().fixTexOffset();
                  flt error=0; REPA(base.vtx)MAX(error, Dist(base.vtx.tex0(i), tex_rect)); // verify that all tex uv's are within allowed tex rect
                  if( error>0.01f) // allow some tolerance
                  {
                     Error *e=null; REPA(errors){Error &err=errors[i]; if(err.mesh_id==mesh_id && err.mtrl_id==err.mtrl_id){e=&err; break;}}
                     if(!e){e=&errors.New(); e->mesh_id=mesh_id; e->mtrl_id=mtrl_id;}
                     MAX(e->error, error);
                  }
                  break;
               }
            }
         }
      }
   }
   bool ConvertToAtlasClass::Create(int &occurence, C UID &id, ptr user) {occurence=0; return true;}
   void ConvertToAtlasClass::convertMeshes(C UID &atlas_id)
   {
      MaterialPtr atlas_mtrl=Proj.gamePath(atlas_id);
      Memc<UID> mtrl_ids, obj_ids; REPA(mtrls)mtrl_ids.binaryInclude(mtrls[i].id);
      REPA(Proj.elms) // iterate all elements
      {
       C Elm &obj=Proj.elms[i]; if(C ElmObj *obj_data=obj.objData()) // if that's an object
         if(C Elm *mesh=Proj.findElm(obj_data->mesh_id))if(C ElmMesh *mesh_data=mesh->meshData())
         {
            REPA(mesh_data->mtrl_ids)if(mtrl_ids.binaryHas(mesh_data->mtrl_ids[i])) // if that mesh has one of the materials
            {
               obj_ids.binaryInclude(obj.id);
               break;
            }
         }
      }
      Memc<UID> duplicated; Proj.duplicate(obj_ids, duplicated, (mode==NEW) ? " (Atlas)" : " (Atlas Backup)");
      //       obj_ids duplicated
      // NEW              atlas
      // REPLACE         backup
      if(mode==NEW)Swap(duplicated, obj_ids);
      Memc<UID> &atlased=obj_ids, &non_atlased=duplicated;

      REPA(atlased)if(Elm *obj=Proj.findElm(atlased[i]))if(ElmObj *obj_data=obj->objData())
                   if(Elm *mesh_elm=Proj.findElm(obj_data->mesh_id, ELM_MESH))if(ElmMesh *mesh_data=mesh_elm->meshData())
      {
         if(ObjEdit.mesh_elm==mesh_elm)ObjEdit.flushMeshSkel();
         Mesh mesh; if(Load(mesh, Proj.editPath(mesh_elm->id), Proj.game_path))
         {
            REPD(l, mesh.lods())
            {
               MeshLod &lod=mesh.lod(l); REPA(lod)
               {
                  MeshPart   &part=lod.parts[i];
                  Mtrl       *mtrl=null;
                  MaterialPtr mtrls[4]; REPA(mtrls)
                  {
                     MaterialPtr &mtrl_ptr=mtrls[i]; if(mtrl_ptr=part.multiMaterial(i))
                     {
                        UID mtrl_id=mtrl_ptr.id(); REPA(T.mtrls)if(T.mtrls[i].id==mtrl_id){mtrl=&T.mtrls[i]; mtrl_ptr=atlas_mtrl; break;}
                     }
                  }
                  if(mtrl)
                  {
                     part.multiMaterial(mtrls[0], mtrls[1], mtrls[2], mtrls[3]);
                     MeshBase &base=part.base; if(base.vtx.tex0())
                     {
                        Vec2 mul=Vec2(mtrl->packed_rect.size())/tex_size,
                             add=Vec2(mtrl->packed_rect.min   )/tex_size;
                        base.explodeVtxs().fixTexOffset();
                        REPA(base.vtx)
                        {
                           Vec2 &t=base.vtx.tex0(i);
                           Clamp(t.x, 0, 1);
                           Clamp(t.y, 0, 1);
                           if(mtrl->rotated)Swap(t.x, t.y);
                           t*=mul; t+=add;
                        }
                        base.setTanBinDbl(); // need to set before 'weldVtx' so it doesn't weld too many vtx's. src mesh doesn't have them
                        base.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down, do not remove degenerate faces because they're not needed because we're doing this only because of 'explodeVtxs'
                     }
                  }
               }
            }
            mesh_data->newVer(); mesh_data->file_time.getUTC();
            mesh.keepOnly(EditMeshFlagAnd);
            Save(mesh, Proj.editPath(mesh_elm->id), Proj.game_path);
            Proj.makeGameVer(*mesh_elm);
            Server.setElmLong(mesh_elm->id);
            if(ObjEdit.mesh_elm==mesh_elm)ObjEdit.reloadMeshSkel();
         }
      }

      // adjust parents, publishing and removed status
      TimeStamp time; time.getUTC();

      int existing_atlased=0; C UID *obj_id=null; REPA(atlased)if(C Elm *elm=Proj.findElm(atlased[i]))if(elm->finalExists()){obj_id=&elm->id; existing_atlased++;}
      if( existing_atlased==1) // if there's only one object created, then put the atlas inside of it
         if(Elm *atlas=Proj.findElm(atlas_id))
      {
         atlas->setParent(*obj_id, time); Server.setElmParent(*atlas);
      }

      // process objects
      if(non_atlased.elms()==atlased.elms())
      {
         REPA(non_atlased)if(Elm *elm=Proj.findElm(non_atlased[i]))
         {
            if(mode==REPLACE_NO_PUBLISH || mode==REPLACE_REMOVE){elm->setParent(atlased[i], time); Server.setElmParent(*elm);} // move 'non_atlased' to 'atlased'
            if(mode==REPLACE_NO_PUBLISH)elm->setPublish(false, time);else
            if(mode==REPLACE_REMOVE    )elm->setRemoved(true , time);
         }
         if(mode==REPLACE_NO_PUBLISH)Server.publishElms(non_atlased, false, -1, time);else
         if(mode==REPLACE_REMOVE    )Server. removeElms(non_atlased, true ,     time);

         /*if(mode==NEW_NO_PUBLISH || mode==NEW_REMOVE) // move sub elements from 'non_atlased' -> 'atlased'
         {
            REPA(Proj.elms)
            {
               Elm &sub=Proj.elms[i]; int index;
               if(non_atlased.binarySearch(sub.parent_id, index) // if is located in one of 'non_atlased'
               && !mtrl_ids.binaryHas(sub.id) // this is not one of the original materials (let's keep original materials in objects)
               && ElmVisible(sub.type)) // this is not one of the special element of original object (mesh/skel/phys)
               {
                  sub.setParent(atlased[index], time); Server.setElmParent(sub);
               }
            }
         }*/
         if(mode!=NEW) // move materials to old ones, from 'atlased' -> 'non_atlased'
         {
            REPA(mtrl_ids)if(Elm *mtrl=Proj.findElm(mtrl_ids[i]))
            {
               int index; if(atlased.binarySearch(mtrl->parent_id, index)) // if is located in one of 'atlased'
               {
                  mtrl->setParent(non_atlased[index], time); Server.setElmParent(*mtrl);
               }
            }
         }
      }

      // process materials
      REPA(mtrl_ids)if(Elm *elm=Proj.findElm(mtrl_ids[i]))
      {
         if(mode==REPLACE_NO_PUBLISH)elm->setPublish(false, time);else
         if(mode==REPLACE_REMOVE    )elm->setRemoved(true , time);
      }
      if(mode==REPLACE_NO_PUBLISH)Server.publishElms(mtrl_ids, false, -1, time);else
      if(mode==REPLACE_REMOVE    )Server. removeElms(mtrl_ids, true ,     time);

      if(mode==REPLACE_NO_PUBLISH || mode==REPLACE_REMOVE)REPA(Proj.elms) // move material sub elements to atlas
      {
         Elm &sub=Proj.elms[i]; if(mtrl_ids.binaryHas(sub.parent_id))
         {
            sub.setParent(atlas_id, time); Server.setElmParent(sub);
         }
      }
   }
   bool ConvertToAtlasClass::AddMap(bool &forced, Str &dest, C Str &src, bool force, C Mtrl &mtrl, TEX_FLAG tex_flag,
                    C Vec &src_mul, C Vec &src_add, // transformation of the source map
                      flt dest_mul,   flt dest_add) // transformation of the dest   map
   {
      forced=false;

      if(force) // if need to write based on params (because there are other materials that need this texture to be generated, so we have to write something)
         if(tex_flag&(TEXF_BUMP|TEXF_NORMAL // don't set bump/normal because it should always be flat without src map
                     |TEXF_ALPHA)) // just leave alpha for now, because it's treated differently depending on alpha blend/test/opaque
            force=false;

      if(src.is() || force)
      {
         // src*src_mul+src_add = dest*dest_mul+dest_add
         // dest = (src*src_mul+src_add-dest_add)/dest_mul
         // dest = src*(src_mul/dest_mul) + (src_add-dest_add)/dest_mul
         Vec mul, add;
         if(Equal(dest_mul, 0)){mul=1; add.zero();}
         else                  {mul=src_mul/dest_mul; add=(src_add-dest_add)/dest_mul;}

         bool added=false;
         Mems<FileParams> fps_dest=FileParams::Decode(dest);
         Mems<FileParams> fps_src =FileParams::Decode(src );
         TextParam src_resize; for(; ExtractResize(fps_src, src_resize); ){} // remove any resizes if present, we replace it with a custome one below
         if(fps_src.elms()) // have source map
         {
            FileParams &fp=fps_dest.New();
            Swap(fps_src, fp.nodes);
            VecI2 size=mtrl.packed_rect.size();
            if(mtrl.edit.smooth_is_rough && tex_flag==TEXF_SMOOTH) fp.params.New().set("inverseRGB");
            if(mtrl.edit.flip_normal_y   && tex_flag==TEXF_NORMAL) fp.params.New().set("inverseG"); // !! this needs to be done before 'swapRG' !!
            if(mtrl.rotated                                      ){fp.params.New().set("swapXY"); if(tex_flag==TEXF_NORMAL)fp.params.New().set("swapRG");} // !! this needs to be done before 'resizeClamp' !!

            bool need_mul=!Equal(mul, Vec(1)),
                 need_add=!Equal(add, Vec(0));
            if(need_mul || need_add) // have to transform
            {
               if(tex_flag&(TEXF_BUMP|TEXF_NORMAL)) // bump and normal have to scale
               {
                  if(need_mul)fp.params.New().set("scale", TextVecEx(mul));
               }else
               {
                  if(need_mul && need_add)fp.params.New().set("mulAddRGB", TextVecVecEx(mul, add));else
                  if(need_mul            )fp.params.New().set("mulRGB"   , TextVecEx   (mul     ));else
                  if(            need_add)fp.params.New().set("addRGB"   , TextVecEx   (     add));
               }
            }
                                          fp.params.New().set("resizeClamp", TextVecI2Ex(size));
            if(mtrl.packed_rect.min.any())fp.params.New().set("position"   , S+mtrl.packed_rect.min.x+','+mtrl.packed_rect.min.y);
            added=true;
         }else // don't have source map
         if(force) // but we need to write based on params (because there are other materials that need this texture to be generated, so we have to write something)
         { // #MaterialTextureLayout
            flt src=((tex_flag&(TEXF_SMOOTH|TEXF_METAL)) ? 0 : 1); // for smooth/metal if there's no source map, then source is treated as no smooth/metal (0)
            Vec set=src*mul+add;
            FileParams &fp=fps_dest.New();
            fp.params.New().set("setRGB", TextVecEx(set)+'@'+mtrl.packed_rect.min.x+','+mtrl.packed_rect.min.y+','+mtrl.packed_rect.w()+','+mtrl.packed_rect.h());
            added=true; forced=true;
         }
         if(added)
         {
            dest=FileParams::Encode(fps_dest);
            return true;
         }
      }
      return false;
   }
   void ConvertToAtlasClass::checkSide(Str &dest, bool filled)
   {
      if(dest.is() && !filled) // if there's at least one texture, but it doesn't fill the target fully, then we need to force the size
      {
         Mems<FileParams> fps=FileParams::Decode(dest);
         FileParams &fp=fps.NewAt(0); // insert a dummy empty source !! this needs to be at the start, to pre-allocate entire size so any transforms can work (for example 'force'd transforms where source texture doesn't exist) !!
         fp.params.New().set("size", S+tex_size.x+','+tex_size.y); // with only position specified to force the entire texture size
         dest=FileParams::Encode(fps);
      }
   }
   void ConvertToAtlasClass::convertPerform()
   {
      // convert before hiding because that may release resources
      if(mtrls.elms())
      {
         TEX_FLAG tex_used=TEXF_NONE; // detect what textures are present and used in all materials, if at least one material uses a texture, then it means we will have to specify it for all materials
         flt      min_reflect=MATERIAL_REFLECT; // minimum reflectivity of all materials
         Map<UID, int> parent_occurence(Compare, Create);
         REPA(mtrls)
         {
            Mtrl &mtrl=mtrls[i]; MtrlEdit.flush(mtrl.id); mtrl.edit.load(Proj.editPath(mtrl.id)); // load after flushing
            if(Elm *elm=Proj.findElm(mtrl.id))(*parent_occurence(elm->parent_id))++; // calculate which parent is most frequently used for stored materials
            tex_used|=mtrl.edit.texturesUsed();
            MIN(min_reflect, mtrl.edit.reflect_min);
         }
         int occurences=0; UID parent_id=UIDZero; REPA(parent_occurence)if(parent_occurence[i]>occurences){occurences=parent_occurence[i]; parent_id=parent_occurence.key(i);} // calculate which parent is most frequently used for stored materials

      /* MtrlImages atlas_images; if(atlas_images.create(tex_size))
         atlas_images.clear();
         WorkerThreads.process1(mtrls, PackMaterial, atlas_images);
         atlas_images.compact();
         atlas_images.createBaseTextures(atlas.base_0, atlas.base_1, atlas.base_2);
         atlas_images.Export("d:/", "bmp"); */

         XMaterialEx atlas;
         atlas.adjust_params=false; // don't adjust params because EE Materials are OK
         atlas.name="Atlas";
         atlas.cull=true;
         atlas.color=0;
         atlas.normal=0;
         atlas.bump=0;
         atlas.  rough_mul=0; atlas.  rough_add=0;
         atlas.reflect_mul=0; atlas.reflect_add=0;
         atlas.glow=0;
         atlas.emissive=0;
         atlas.emissive_glow=0;
         REPAO(atlas.tex_downsize)=MaxMaterialDownsize-1; // set as biggest downsize (smallest size)
         flt      alpha=0; int alpha_num=0; MATERIAL_TECHNIQUE tech=MTECH_OPAQUE; // parameters for alpha materials
         flt      reflect_min=0, reflect_max=0;
         TEX_FLAG tex_wrote     =TEXF_NONE; // what textures we wrote to atlas
         TEX_FLAG tex_force_size=TEXF_NONE; // what textures need size forced
         TEX_FLAG tex_filled_x  =TEXF_NONE; // bit mask of which textures fill atlas image in X
         TEX_FLAG tex_filled_y  =TEXF_NONE; // bit mask of which textures fill atlas image in Y
         MaterialParams params; // temporary used for calculating reflect values
         FREPA(mtrls)
         {
            Mtrl &mtrl=mtrls[i];
            atlas.cull         &=mtrl.edit.cull; // if at least one material requires cull disabled, then disable for all
            atlas.color        +=mtrl.edit.color_s;
            atlas.normal       +=mtrl.edit.normal;
            atlas.bump         +=mtrl.edit.bump;
            atlas.rough_mul    +=mtrl.edit.roughMul(); atlas.rough_add+=mtrl.edit.roughAdd();
                reflect_min    +=mtrl.edit.reflect_min; reflect_max+=mtrl.edit.reflect_max;
            atlas.glow         +=mtrl.edit.glow;
            atlas.emissive     +=mtrl.edit.emissive_s;
            atlas.emissive_glow+=mtrl.edit.emissive_glow;

            REPA(atlas.tex_downsize)MIN(atlas.tex_downsize[i], mtrl.edit.tex_downsize[i]); // minimize downsize (maximize size)

            if(mtrl.edit.tech){alpha+=mtrl.edit.color_s.w; alpha_num++; tech=mtrl.edit.tech;}

            // check if normal map is greyscale and needs to have "bumpToNormal" forced, this can be done before "expandMaps" and before manually setting from bump map (because both already support "bumpToNormal", and actually it's better to load here, because normal map can be empty here and no need to load anything, faster)
            if(mtrl.edit.normal_map.is())
            {
               Image temp; TextParam resize; if(Proj.loadImages(temp, &resize, mtrl.edit.normal_map))if(temp.typeChannels()<=1 || temp.monochromaticRG())
               {
                  SetTransform(mtrl.edit.normal_map, "bumpToNormal"); // force "bumpToNormal"
                  mtrl.edit.flip_normal_y=false; // "bumpToNormal" always generates correct normal, so have to disable flip
               }
            }

            mtrl.edit.expandMaps(); // have to expand maps, because if normal map was referring to bump, then the new bump will be different (atlas of bumps)

            if(tex_used&TEXF_ALPHA) // if at least one material uses 'alpha_map', then we need to specify all of them, in case: alpha in one material comes from 'color_map', or it will in the future
               if(!mtrl.edit.alpha_map.is()) // if not yet specified
            {
               mtrl.edit.alpha_map=mtrl.edit.color_map; // set from 'color_map'
               AddTransform(mtrl.edit.alpha_map, "channel", "a"); // use alpha channel of 'color_map'
            }

            if(tex_used&TEXF_BUMP) // if at least one material uses 'bump_map', then we need to specify all normal maps
               if(!mtrl.edit.normal_map.is()) // if not yet specified
            {
               mtrl.edit.normal_map=mtrl.edit.bump_map; // set from 'bump_map'
               SetTransform(mtrl.edit.normal_map, "bumpToNormal"); // convert to normal
               mtrl.edit.flip_normal_y=false; // "bumpToNormal" always generates correct normal, so have to disable flip
            }

            TEX_FLAG tex_mtrl=TEXF_NONE; // what textures we've written to the atlas from this material, if there's at least one other texture of the same type in another material, then we have to force writing it for this material
            bool     forced;
            params.reflect(mtrl.edit.reflect_min, mtrl.edit.reflect_max); // calculate mul add
            if(AddMap(forced, atlas.   color_map, mtrl.edit.   color_map, FlagOn(tex_used, TEXF_COLOR   ), mtrl, TEXF_COLOR   , mtrl.edit.color_s.xyz                                             )){tex_mtrl|=TEXF_COLOR   ; if(forced)tex_force_size|=TEXF_COLOR   ;}
            if(AddMap(forced, atlas.   alpha_map, mtrl.edit.   alpha_map, FlagOn(tex_used, TEXF_ALPHA   ), mtrl, TEXF_ALPHA                                                                       )){tex_mtrl|=TEXF_ALPHA   ; if(forced)tex_force_size|=TEXF_ALPHA   ;}
            if(AddMap(forced, atlas.    bump_map, mtrl.edit.    bump_map, FlagOn(tex_used, TEXF_BUMP    ), mtrl, TEXF_BUMP                                                                        )){tex_mtrl|=TEXF_BUMP    ; if(forced)tex_force_size|=TEXF_BUMP    ;}
            if(AddMap(forced, atlas.  normal_map, mtrl.edit.  normal_map, FlagOn(tex_used, TEXF_NORMAL  ), mtrl, TEXF_NORMAL  , mtrl.edit.normal                                                  )){tex_mtrl|=TEXF_NORMAL  ; if(forced)tex_force_size|=TEXF_NORMAL  ;}
            if(AddMap(forced, atlas.  smooth_map, mtrl.edit.  smooth_map, FlagOn(tex_used, TEXF_SMOOTH  ), mtrl, TEXF_SMOOTH  , mtrl.edit.smoothMul(), mtrl.edit.smoothAdd()                      )){tex_mtrl|=TEXF_SMOOTH  ; if(forced)tex_force_size|=TEXF_SMOOTH  ;}
            if(AddMap(forced, atlas.   metal_map, mtrl.edit.   metal_map, FlagOn(tex_used, TEXF_METAL   ), mtrl, TEXF_METAL   , params.reflect_mul, params.reflect_add, 1-min_reflect, min_reflect)){tex_mtrl|=TEXF_METAL   ; if(forced)tex_force_size|=TEXF_METAL   ;} // 1-min_reflect because we will operate on min_reflect..1 range, so mul=1-min_reflect, add=min_reflect
            if(AddMap(forced, atlas.    glow_map, mtrl.edit.    glow_map, FlagOn(tex_used, TEXF_GLOW    ), mtrl, TEXF_GLOW    , mtrl.edit.glow                                                    )){tex_mtrl|=TEXF_GLOW    ; if(forced)tex_force_size|=TEXF_GLOW    ;}
            if(AddMap(forced, atlas.emissive_map, mtrl.edit.emissive_map, FlagOn(tex_used, TEXF_EMISSIVE), mtrl, TEXF_EMISSIVE, mtrl.edit.emissive_s                                              )){tex_mtrl|=TEXF_EMISSIVE; if(forced)tex_force_size|=TEXF_EMISSIVE;}

            tex_wrote|=tex_mtrl;

            if(mtrl.packed_rect.includesX(tex_size.x)) // if this packed rect includes the right side
               tex_filled_x|=tex_mtrl; // add filled textures from this material

            if(mtrl.packed_rect.includesY(tex_size.y)) // if this packed rect includes the bottom side
               tex_filled_y|=tex_mtrl; // add filled textures from this material
         }

         // if textures didn't fill entire needed space, then we need to place a dummy to force image size
         TEX_FLAG filled=tex_filled_x&tex_filled_y; // we need both sides to be filled
         filled&=~tex_force_size; // if a texture needs to have forced size, then we must disable filled so the size is specified
         checkSide(atlas.   color_map, FlagOn(filled, TEXF_COLOR   ));
         checkSide(atlas.   alpha_map, FlagOn(filled, TEXF_ALPHA   ));
         checkSide(atlas.    bump_map, FlagOn(filled, TEXF_BUMP    ));
         checkSide(atlas.  normal_map, FlagOn(filled, TEXF_NORMAL  ));
         checkSide(atlas.  smooth_map, FlagOn(filled, TEXF_SMOOTH  ));
         checkSide(atlas.   metal_map, FlagOn(filled, TEXF_METAL   ));
         checkSide(atlas.    glow_map, FlagOn(filled, TEXF_GLOW    ));
         checkSide(atlas.emissive_map, FlagOn(filled, TEXF_EMISSIVE));

         if(tex_wrote& TEXF_COLOR            ){atlas.color.xyz=                                1;                      }else atlas.color.xyz    /=mtrls.elms();                                       // if we ended up having color  map, then it means we've used the baked textures, for which we need to set the full color    multiplier
         if(tex_wrote&(TEXF_COLOR|TEXF_ALPHA)){atlas.color.w  =(alpha_num ? alpha/alpha_num : 1); atlas.technique=tech;}else atlas.color.w      /=mtrls.elms();                                       // if we ended up having alpha  map, then set parameters from alpha materials only (check color map too because alpha can come from it)
         if(tex_wrote& TEXF_BUMP             ){                                                                        }     atlas.bump         /=mtrls.elms();
         if(tex_wrote& TEXF_NORMAL           ){atlas.normal  =1;                                                       }else atlas.normal       /=mtrls.elms();                                       // if we ended up having normal map, then it means we've used the baked textures, for which we need to set the full normal   multiplier
         if(tex_wrote& TEXF_SMOOTH           ){atlas.rough_mul=1; atlas.rough_add=0;                                   }else{atlas.rough_mul    /=mtrls.elms(); atlas.rough_add/=mtrls.elms();}       // if we ended up having smooth map, then it means we've used the baked textures, for which we need to set the full smooth   multiplier
         if(tex_wrote& TEXF_METAL            ){atlas.reflect(min_reflect);                                             }else atlas.reflect      (reflect_min/mtrls.elms(), reflect_max/mtrls.elms()); // if we ended up having metal  map, then it means we've used the baked textures, for which we need to set the full reflect  multiplier
         if(tex_wrote& TEXF_GLOW             ){atlas.glow    =1;                                                       }else atlas.glow         /=mtrls.elms();                                       // if we ended up having glow   map, then it means we've used the baked textures, for which we need to set the full glow     multiplier
         if(tex_wrote& TEXF_EMISSIVE         ){atlas.emissive=1;                                                       }else atlas.emissive     /=mtrls.elms();                                       // if we ended up having light  map, then it means we've used the baked textures, for which we need to set the full emissive multiplier
                                                                                                                             atlas.emissive_glow/=mtrls.elms();

         EditMaterial edit; edit.create(atlas);
         Proj.createBaseTextures(atlas.base_0, atlas.base_1, atlas.base_2, edit);
         Proj.loadImages(atlas.emissive_img, null, edit.emissive_map, true);
         // copy images only if 'Importer.includeTex' which means this texture was encountered for the first time, don't check for 'Proj.includeTex' because with it we would have to also save the textures, which are done automatically in 'Proj.newMtrl'
         IMAGE_TYPE ct; ImageProps(atlas.base_0      , &atlas.  base_0_id, &ct, MTRL_BASE_0, edit.tex_quality); if(Importer.includeTex(atlas.  base_0_id))                                       atlas.      base_0.copyTry(atlas.      base_0, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP);
                        ImageProps(atlas.base_1      , &atlas.  base_1_id, &ct, MTRL_BASE_1                  ); if(Importer.includeTex(atlas.  base_1_id))                                       atlas.      base_1.copyTry(atlas.      base_1, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP);
                        ImageProps(atlas.base_2      , &atlas.  base_2_id, &ct, MTRL_BASE_2                  ); if(Importer.includeTex(atlas.  base_2_id))                                       atlas.      base_2.copyTry(atlas.      base_2, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP);
                        ImageProps(atlas.emissive_img, &atlas.emissive_id, &ct, MTRL_EMISSIVE                ); if(Importer.includeTex(atlas.emissive_id)){SetFullAlpha(atlas.emissive_img, ct); atlas.emissive_img.copyTry(atlas.emissive_img, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP);}

         UID atlas_id=Proj.newMtrl(atlas, parent_id).id;
         Server.setElmFull(atlas_id);
         convertMeshes(atlas_id);
         Proj.setList();
      }
      warning.hide();
      hide();
   }
   void ConvertToAtlasClass::convertDo()
   {
      warning.hide();
      Memc<UID> mtrl_ids; REPA(mtrls)mtrl_ids.binaryInclude(mtrls[i].id);

      // check if can (meshes need to have non-wrapped UV's)
      errors.clear();
      REPA(Proj.elms) // iterate all elements
      {
         Elm &elm=Proj.elms[i]; if(ElmMesh *mesh_data=elm.meshData()) // if that's a mesh
         {
            REPA(mesh_data->mtrl_ids)if(mtrl_ids.binaryHas(mesh_data->mtrl_ids[i])) // if that mesh has one of the materials
            {
               Mesh mesh; if(ObjEdit.mesh_elm==&elm)mesh.create(ObjEdit.mesh);else Load(mesh, Proj.editPath(elm.id), Proj.game_path); // load edit so we can have access to MeshBase
               setErrors(mesh, mtrl_ids, elm.id);
               break;
            }
         }
      }

      if(errors.elms())warning.display();else convertPerform();
   }
   void ConvertToAtlasClass::clearProj()
   {
      mtrls.clear();
      warning.hide();
   }
   void ConvertToAtlasClass::create()
   {
      warning.create();
               add("Force Square"   , MEMBER(ConvertToAtlasClass, force_square)).changed(Refresh);
               add("Auto Stretch"   , MEMBER(ConvertToAtlasClass, auto_stretch)).changed(Refresh);
               add("Allow Rotate"   , MEMBER(ConvertToAtlasClass, allow_rotate)).changed(Refresh);
Property &mode=add("Atlased Objects", MEMBER(ConvertToAtlasClass, mode)).setEnum(mode_t, Elms(mode_t)).desc("When creating material atlases, existing object meshes need to have their UV adjusted.\nWith this option you can control if the adjusted objects:\n-Replace the old ones (keeping their Element ID)\nor\n-They are created as new objects (with new Element ID)");
               add("Global Scale"   , MEMBER(ConvertToAtlasClass, scale)).range(1.0f/8, 8).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).changed(ChangedScale);
      Rect r=super::create("Convert To Atlas"); button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); mode.combobox.resize(Vec2(0.63f, 0));
      autoData(this);
      T+=region.create(Rect_LU(0.02f, r.min.y-0.02f, 1.8f, 0.7f));
      T+=preview.create(Rect_LU(region.rect().ru()+Vec2(0.02f, 0), 0.7f));
      Vec2 size(preview.rect().max.x, Max(-region.rect().min.y, -preview.rect().min.y));
      rect(Rect_C(0, size+0.02f+defaultInnerPaddingSize()));
      T+=columns.create(Vec2(1.12f, region.rect().max.y+ts.size.y*0.8f), "Scale           Original Size      Scaled Size", &ts);
      T+=t_tex_size.create(preview.rect().up()+Vec2(0, 0.04f));
      T+=convert.create(Rect_U(clientWidth()/2, -0.05f, 0.3f, 0.055f), "Convert").func(Convert, T);
   }
   void ConvertToAtlasClass::autoStretch()
   {
      if(mtrls.elms())
      {
         RectI r=mtrls.last().packed_rect; REPA(mtrls)r|=mtrls[i].packed_rect; if(r.size().all())
         {
            if(r.min.any() || r.max!=tex_size)
            {
               RectI tex_rect(0, tex_size);
               Vec2  scale=Vec2(tex_size)/r.size(), offset=-r.min*scale;
               REPA(mtrls)
               {
                  RectI &rect=mtrls[i].packed_rect;
                  rect=Round(Rect(rect)*scale+offset)&tex_rect;
               }
            }
         }
      }
      // convert to inclusive
      REPAO(mtrls).packed_rect.max--;
      REPA (mtrls)
      {
         RectI &r=mtrls[i].packed_rect;
         r.min.x=           0; REPAD(j, mtrls)if(j!=i){C RectI &t=mtrls[j].packed_rect; if(Cuts(r, t))r.min.x=t.max.x+1;}
         r.max.x=tex_size.x-1; REPAD(j, mtrls)if(j!=i){C RectI &t=mtrls[j].packed_rect; if(Cuts(r, t))r.max.x=t.min.x-1;}
         r.min.y=           0; REPAD(j, mtrls)if(j!=i){C RectI &t=mtrls[j].packed_rect; if(Cuts(r, t))r.min.y=t.max.y+1;}
         r.max.y=tex_size.y-1; REPAD(j, mtrls)if(j!=i){C RectI &t=mtrls[j].packed_rect; if(Cuts(r, t))r.max.y=t.min.y-1;}
      }
      // convert to exclusive
      REPAO(mtrls).packed_rect.max++;
   }
   void ConvertToAtlasClass::refresh()
   {
      flt y=0;
      FREPA(mtrls){mtrls[i].posY(y); y-=h;}
      Memc<RectSizeAnchor> mtrl_sizes; mtrl_sizes.setNum(mtrls.elms()); REPAO(mtrl_sizes).size=mtrls[i].scaled_size;
      Memc<RectI         > mtrl_rects; if(PackRectsUnknownLimit(mtrl_sizes, mtrl_rects, tex_size, allow_rotate, 0, false, true, ConvertToAtlas.force_square))
      {
         REPA(mtrls)
         {
            Mtrl &mtrl=mtrls[i];
            mtrl.packed_rect=mtrl_rects[i];
            mtrl.rotated=((mtrl.packed_rect.w()>mtrl.packed_rect.h())!=(mtrl.scaled_size.x>mtrl.scaled_size.y));
         }
         if(auto_stretch)autoStretch();
         t_tex_size.set(TexSize(tex_size));
      }else
      {
         REPAO(mtrls).packed_rect.zero();
         tex_size.zero();
       t_tex_size.clear();
      }
      REPA(mtrls){Mtrl &mtrl=mtrls[i]; mtrl.setScaleText(mtrl.packed_rect.size());}
      warning.hide();
   }
   void ConvertToAtlasClass::addElms(C MemPtr<UID> &elm_ids)
   {
      REPA(elm_ids)if(Elm *elm=Proj.findElm(elm_ids[i], ELM_MTRL))
      {
         REPA(mtrls)if(mtrls[i].id==elm->id)goto skip;
         {
            Mtrl &mtrl=mtrls.New();
            mtrl.id=elm->id;
            region+=mtrl.del .create(Rect_LU(0, 0, h, h)).setImage("Gui/close.img").func(Del, mtrl).desc("Remove this Material");
            region+=mtrl.name.create(Rect_LU(h, 0, 0.9f, h), Proj.elmFullName(elm->id)).disabled(true);
            if(ElmMaterial *mtrl_data=elm->mtrlData())
               if(mtrl.base_0=Proj.texPath(mtrl_data->base_0_tex))
                  mtrl.original_size=mtrl.base_0->size();
            mtrl.prop.create(S, MemberDesc(MEMBER(Mtrl, scale)).setFunc(Scale, Scale)).range(1.0f/8, 8).mouseEditLinked(true).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).autoData(&mtrl).text.text_style=&ts;
            region+=mtrl.t_original_size.create(Vec2(mtrl.name.rect().max.x+0.43f, 0-h*0.5f), TexSize(mtrl.original_size));
            region+=mtrl.t_scaled_size  .create(Vec2(mtrl.name.rect().max.x+0.66f, 0-h*0.5f));
            mtrl.prop.addTo(region,             Vec2(mtrl.name.rect().max.x+0.04f, 0), 0, h, 0.25f);
            mtrl.setScale();
         }
      skip:;
      }
      mtrls.sort(CompareData);
      refresh();
      activate();
   }
   void ConvertToAtlasClass::setElms(C MemPtr<UID> &elm_ids)
   {
      mtrls.clear();
      addElms(elm_ids);
   }
   void ConvertToAtlasClass::drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(contains(focus_obj))
      {
         addElms(elms);
         elms.clear();
      }
   }
   ConvertToAtlasClass& ConvertToAtlasClass::hide()
{
      super::hide();
      mtrls.clear(); warning.hide(); // release memory, since we're releasing then we also need to hide the warning to prevent from converting on empty materials
      return T;
   }
ConvertToAtlasClass::ConvertToAtlasClass() : force_square(false), auto_stretch(false), allow_rotate(true), scale(1), mode(NEW), tex_size(0) {}

ConvertToAtlasClass::Mtrl::Mtrl() : id(UIDZero), original_size(0), scaled_size(0), pos(0), packed_rect(0), rotated(false), scale(1) {}

ConvertToAtlasClass::Error::Error() : mesh_id(UIDZero), mtrl_id(UIDZero), error(0) {}

/******************************************************************************/
