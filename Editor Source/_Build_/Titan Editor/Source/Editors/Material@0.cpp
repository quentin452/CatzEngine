/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
MaterialRegion MtrlEdit;
/******************************************************************************/

/******************************************************************************/
   cchar8 *MaterialRegion::auto_reload_name="Reload on Change";
   ::MaterialRegion::MaterialTech MaterialRegion::mtrl_techs[]=
   {
      {MTECH_OPAQUE                , "Opaque"                , u"Standard rendering of opaque materials."},
      {MTECH_ALPHA_TEST            , "Alpha Test"            , u"Indicates that color texture alpha channel will be used as models transparency.\nThis is slightly slower than Default as alpha testing may disable some hardware-level optimizations."},
      {MTECH_ALPHA_TEST_DITHER     , "Alpha Test Dither"     , u"Indicates that color texture alpha channel will be used as models transparency with dithering.\nThis is slightly slower than Default as alpha testing may disable some hardware-level optimizations."},
      {MTECH_FUR                   , "Fur"                   , u"Mesh will be rendered with fur effect, the mesh will be wrapped with additional fur imitating textures.\nDetail Scale specifies fur intensity, Detail Power specifies fur length.\nSupported only in Deferred Renderer!"},
      {MTECH_GRASS                 , "Grass"                 , u"Mesh vertexes will bend on the wind like grass,\nbending intensity is determined by mesh vertex source Y position,\nwhich should be in the range from 0 to 1.\nGrass is made out of billboard instead of 3D geometric Mesh."},
      {MTECH_GRASS_3D              , "Grass 3D"              , u"Mesh vertexes will bend on the wind like grass,\nbending intensity is determined by mesh vertex source Y position,\nwhich should be in the range from 0 to 1.\nGrass is made out of 3D geometric Mesh instead of billboard."},
      {MTECH_LEAF                  , "Leaf"                  , u"Mesh vertexes will bend on the wind like tree leafs,\nto use this technique mesh must also contain leaf attachment positions,\nwhich can be generated in the Model Editor tool through menu options."},
      {MTECH_LEAF_2D               , "Leaf 2D"               , u"Mesh vertexes will bend on the wind like tree leafs,\nto use this technique mesh must also contain leaf attachment positions,\nwhich can be generated in the Model Editor tool through menu options.\nLeafs are made out of 2D billboards instead 3D geometric Mesh."},
      {MTECH_BLEND                 , "Blend"                 , u"Mesh will be smoothly blended on the screen using alpha values,\nmesh will not be affected by lighting or shadowing."},
      {MTECH_DEPTH_BLEND           , "Depth Blend"           , u"Works like Blend technique with additional Depth-Writing which enables correct Depth-Sorting."},
      {MTECH_BLEND_LIGHT           , "Blend Light"           , u"Works like Blend technique except that mesh will be affected by lighting or shadowing,\nhowever only the most significant directional light will be used (all other lights are ignored)\nDue to additional lighting calculations this is slower than Blend technique."},
      {MTECH_BLEND_LIGHT_GRASS     , "Blend Light Grass"     , u"Combination of Blend Light and Grass techniques."},
      {MTECH_BLEND_LIGHT_LEAF      , "Blend Light Leaf"      , u"Combination of Blend Light and Leaf techniques."},
      {MTECH_TEST_BLEND_LIGHT      , "Test Blend Light"      , u"Works like Blend Light technique with additional Alpha-Testing and Depth-Writing which enables correct Depth-Sorting."},
      {MTECH_TEST_BLEND_LIGHT_GRASS, "Test Blend Light Grass", u"Works like Blend Light Grass technique with additional Alpha-Testing and Depth-Writing which enables correct Depth-Sorting."},
      {MTECH_TEST_BLEND_LIGHT_LEAF , "Test Blend Light Leaf" , u"Works like Blend Light Leaf technique with additional Alpha-Testing and Depth-Writing which enables correct Depth-Sorting."},
      {MTECH_CLEAR_COAT            , "Clear Coat"            , u"Can be used for metallic car surfaces."},
   };
   cchar8 *MaterialRegion::TexDownsizeText[]=
   {
      "Full",
      "Half",
      "Quarter",
      "Eighth",
   };
   ::MaterialRegion::TexQualityND MaterialRegion::TexQualities[]=
   {
      {u"Low"             , u"same as Medium except uses PVRTC1_2 for iOS"          , Edit::Material::LOW   },
      {u"Medium (Default)", u"default mode, uses BC1 (or BC7 if have alpha channel)", Edit::Material::MEDIUM},
      {u"High"            , u"always uses BC7 even if don't have alpha channel"     , Edit::Material::HIGH  },
      {u"Full"            , u"uncompressed R8G8B8A8"                                , Edit::Material::FULL  },
   };
   const flt MaterialRegion::BumpScale=0.10f;
/******************************************************************************/
      void MaterialRegion::Change::create(ptr user)
{
         data=MtrlEdit.edit;
         MtrlEdit.undoVis();
      }
      void MaterialRegion::Change::apply(ptr user)
{
         uint changed=MtrlEdit.edit.undo(data);
         MtrlEdit.setChanged();
         MtrlEdit.toGui();
         D.setShader(MtrlEdit.game());
         MtrlEdit.undoVis();
         if(changed&(EditMaterial::CHANGED_BASE|TEXF_DET|TEXF_MACRO|TEXF_EMISSIVE))Proj.mtrlTexChanged();
      }
      void MaterialRegion::Texture::Load(C Str &name, Texture &texture) {texture.setFile(name);}
      void MaterialRegion::Texture::Remove(             Texture &texture) {texture.setFile(S);}
      ImagePtr MaterialRegion::Texture::getImage()
      {
         if(mr)
         {
            EditMaterial &em      =mr->getEditMtrl();
          C ImagePtr     &base_0  =mr->getBase0   (),
                         &base_1  =mr->getBase1   (),
                         &base_2  =mr->getBase2   (),
                         &macro   =mr->getMacro   (),
                         &detail  =mr->getDetail  (),
                         &emissive=mr->getEmissive();
            if(!mr->water())switch(type) // #MaterialTextureLayout
            {
               case TEX_COLOR     : if(em.   color_map   .is()                       )return base_0  ; break;
               case TEX_ALPHA     : if(em.   color_map   .is() || em.alpha_map.is()  )return base_0  ; break;
               case TEX_BUMP      : if(em.  hasBumpMap    ()                         )return base_2  ; break;
               case TEX_NORMAL    : if(em.hasNormalMap    ()                         )return base_1  ; break;
               case TEX_SMOOTH    : if(em.  smooth_map   .is()                       )return base_2  ; break;
               case TEX_METAL     : if(em.   metal_map   .is()                       )return base_2  ; break;
               case TEX_GLOW      : if(em.    glow_map   .is()                       )return base_2  ; break;
               case TEX_EMISSIVE  : if(em.emissive_map   .is()                       )return emissive; break;
               case TEX_MACRO     : if(em.   macro_map   .is()                       )return macro   ; break;
               case TEX_DET_COLOR : if(em.  detail_color .is()                       )return detail  ; break; // #MaterialTextureLayoutDetail
               case TEX_DET_BUMP  : if(em.  detail_bump  .is()                       )return detail  ; break;
               case TEX_DET_NORMAL: if(em.  detail_normal.is() || em.detail_bump.is())return detail  ; break;
               case TEX_DET_SMOOTH: if(em.  detail_smooth.is()                       )return detail  ; break;
            }else switch(type) // #MaterialTextureLayoutWater
            {
               case TEX_COLOR     : if(em.color_map.is())return base_0; break;
               case TEX_BUMP      : if(em.  hasBumpMap())return base_2; break;
               case TEX_NORMAL    : if(em.hasNormalMap())return base_1; break;
            }
         }
         return null;
      }
      void MaterialRegion::Texture::ReplaceElmNames(Mems<FileParams> &files)
      {
         UID image_id; REPA(files)
         {
            FileParams &fp=files[i];
            if(DecodeFileName(fp.name, image_id))fp.name=Proj.elmFullName(image_id);
            ReplaceElmNames(fp.nodes);
         }
      }
      void MaterialRegion::Texture::setDesc()
      {
         Mems<FileParams> files=FileParams::Decode(file); ReplaceElmNames(files);
         Str desc=Replace(text, '\n', ' '); if(C ImagePtr &image=getImage())desc+=S+", "+image->w()+'x'+image->h();
         if(type==TEX_MACRO)desc.line()+="Can be set for heightmap materials to decrease repetitiveness of textures.\nBecomes visible at distance of around 100 meters.";
         FREPA(files){desc+='\n'; desc+=files[i].encode();}
         desc+="\nUse Ctrl+Click to Explore";
         desc+="\nIf you have multiple data packed per-channel in one image, then you can hold Shift key while drag and drop to auto-detect,\nor specify manually which channel to use by appending \"?channel=X\" to file name where X is r, g, b, a.";
         T.desc(desc);
      }
      void MaterialRegion::Texture::FixPath(Mems<FileParams> &fps)
      {
         REPA(fps)
         {
            FileParams &fp=fps[i];
            if(Contains(fp.name, '|'))fp.name=GetBaseNoExt(fp.name); // we want to support things like "|bump|" but when entering that into 'WindowIO', it may append the path, so when this command is detected, remove the path (and possible extension too)
            FixPath(fp.nodes);
         }
      }
      void MaterialRegion::Texture::setFile(Str file, bool set_undo)
      {
         // fix |..| paths, such as "|color|" etc.
         Mems<FileParams> fps=FileParams::Decode(file);
         FixPath(fps);
         file=FileParams::Encode(fps);

         T.file=file; setDesc();
         if(mr)
         {
            if(set_undo)mr->undos.set(null, true);
            EditMaterial &mtrl=mr->getEditMtrl();
            TEX_FLAG  textures=mtrl.textures(); // get current state of textures before making any change
          //if(type>=TEX_RFL_L && type<=TEX_RFL_U)file=SetCubeFile(md_file.asText(&mtrl), type-TEX_RFL_L, file);
            md_time.as<TimeStamp>(&mtrl).now();
            md_file.fromText     (&mtrl, file);

            if(!file.is()) // if removing texture
            {
               Str name; switch(type)
               {
                  case TEX_COLOR : name="|color|" ; break;
                  case TEX_SMOOTH: name="|smooth|"; break;
                  case TEX_BUMP  : name="|bump|"  ; break;
               }
               if(name.is())REPA(mr->texs)
               {
                  Texture &tex=mr->texs[i]; if(tex.type>=TEX_BASE_BEGIN && tex.type<=TEX_BASE_END) // iterate all base textures
                  {
                     Mems<FileParams> fps=FileParams::Decode(tex.md_file.asText(&mtrl)); // get file name of that texture
                     if(fps.elms()==1 && fps[0].name==name) // if that file is made from removed one
                     {
                        tex.md_time.as<TimeStamp>(&mtrl).now();
                        tex.md_file.fromText     (&mtrl, S); // remove too
                        tex.toGui();
                     }
                  }
               }
            }

            if(type==TEX_SMOOTH)mr->setSmoothParam();

            // rebuild methods already call 'setChanged'
            if(type>=TEX_BASE_BEGIN && type<=TEX_BASE_END)mr->rebuildBase    (textures, 1<<type);else
            if(type>=TEX_DET_BEGIN  && type<=TEX_DET_END )mr->rebuildDetail  ();else
            if(type==TEX_MACRO                           )mr->rebuildMacro   ();else
            if(type==TEX_EMISSIVE                        )mr->rebuildEmissive(textures);else
                                                          mr->setChanged     ();

            // if adjusting reflection texture, then adjust parameters of all other relfection slots, because they are connected
            //if(type>=TEX_RFL_L && type<=TEX_RFL_ALL)REPA(mr.texs)if(mr.texs[i].type>=TEX_RFL_L && mr.texs[i].type<=TEX_RFL_ALL)mr.texs[i].toGui();
         }
      }
      void MaterialRegion::Texture::toGui()
      {
         if(mr)
         {
            EditMaterial &mtrl=mr->getEditMtrl();
            file=md_file.asText(&mtrl); //if(type>=TEX_RFL_L && type<=TEX_RFL_U)file=GetCubeFile(file, type-TEX_RFL_L);
            if(type==TEX_SMOOTH)text=(mtrl.smooth_is_rough ? "Rough" : "Smooth");
            setDesc();
         }
      }
      ::MaterialRegion::Texture& MaterialRegion::Texture::create(TEX_TYPE type, C MemberDesc &md_file, C MemberDesc &md_time, Rect rect, C Str &text, MaterialRegion &mr)
      {
         T.mr=&mr;
         T.type=type;
         T.md_file=md_file;
         T.md_time=md_time;
         super::create().rect(rect.extend(-0.003f)); T.text=text;
         win_io.create().ext(SUPPORTED_IMAGE_EXT, Replace(text, '\n', ' ')).io(Load, Load, T);
         remove.create(Rect_RU(rect.ru(), 0.035f, 0.035f)).func(Remove, T); remove.image="Gui/close.img";
         return T;
      }
      bool MaterialRegion::Texture::ExploreFiles(C Mems<FileParams> &fps)
      {
         FREPA(fps)
         {
          C FileParams &fp=fps[i];
            if(fp.name.is())
            {
               UID id; if(id.fromFileName(fp.name))Proj.elmLocate(id, true);else Explore(FFirstUp(fp.name));
               return true;
            }
            if(ExploreFiles(fp.nodes))return true;
         }
         return false;
      }
      void MaterialRegion::Texture::update(C GuiPC &gpc)
{
         if(gpc.visible && visible())
         {
            super::update(gpc);
            rect_color=((Gui.msLit()==this && Gui.skin) ? Gui.skin->keyboard_highlight_color : Gui.borderColor());
            REPA(MT)if(MT.bp(i) && MT.guiObj(i)==this)
            {
               if(file.is()) // texture path has anything
               {
                  if(Kb.ctrlCmd()) // Ctrl+Click -> explore file path
                  {
                     ExploreFiles(FileParams::Decode(file));
                     goto skip_win_io;
                  }else
                  {
                     SetPath(win_io, file);
                  }
               }else
               if(mr)
               {
                  Mems<FileParams> fps=FileParams::Decode(mr->getEditMtrl().color_map);
                  Str  mtrl_path=Proj.elmSrcFileFirst(mr->elm); if(FileInfoSystem(mtrl_path).type==FSTD_FILE)mtrl_path=GetPath(mtrl_path);
                  Str color_path; if(fps.elms())color_path=FFirstUp(GetPath(fps[0].name));
                  SetPath(win_io, (mtrl_path.length()>color_path.length()) ? mtrl_path : color_path);
               }
               win_io.activate();
            skip_win_io:
               break;
            }
         }
         remove.visible((Gui.ms()==this || Gui.ms()==&remove) && file.is());
      }
      bool MaterialRegion::Texture::draw(C Rect &rect)
      {
         bool tex=false;
         if(mr)
         {
            EditMaterial &em      =mr->getEditMtrl();
          C ImagePtr     &base_0  =mr->getBase0   (),
                         &base_1  =mr->getBase1   (),
                         &base_2  =mr->getBase2   (),
                         &macro   =mr->getMacro   (),
                         &detail  =mr->getDetail  (),
                         &emissive=mr->getEmissive();
            ALPHA_MODE alpha=D.alpha(ALPHA_NONE);
            if(!mr->water())switch(type) // #MaterialTextureLayout
            {
               case TEX_COLOR     : if(em.   color_map   .is()                       )if(base_0  ){                                                                                      base_0  ->drawFit(rect); tex=true;} break;
               case TEX_ALPHA     : if(em.   color_map   .is() || em.alpha_map.is()  )if(base_0  ){VI.shader(ShaderFiles("Main")->get(                     "DrawTexWG"               )); base_0  ->drawFit(rect); tex=true;} break;
               case TEX_BUMP      : if(em.  hasBumpMap      ()                       )if(base_2  ){VI.shader(ShaderFiles("Main")->get(                     "DrawTexZG"               )); base_2  ->drawFit(rect); tex=true;} break;
               case TEX_NORMAL    : if(em.hasNormalMap      ()                       )if(base_1  ){VI.shader(ShaderFiles("Main")->get(                     "DrawTexNrm"              )); base_1  ->drawFit(rect); tex=true;} break;
               case TEX_SMOOTH    : if(em.  smooth_map   .is()                       )if(base_2  ){VI.shader(ShaderFiles("Main")->get(em.smooth_is_rough ? "DrawTexYG" : "DrawTexYIG")); base_2  ->drawFit(rect); tex=true;} break;
               case TEX_METAL     : if(em.   metal_map   .is()                       )if(base_2  ){VI.shader(ShaderFiles("Main")->get(                     "DrawTexXG"               )); base_2  ->drawFit(rect); tex=true;} break;
               case TEX_GLOW      : if(em.    glow_map   .is()                       )if(base_2  ){VI.shader(ShaderFiles("Main")->get(                     "DrawTexWG"               )); base_2  ->drawFit(rect); tex=true;} break;
               case TEX_EMISSIVE  : if(em.emissive_map   .is()                       )if(emissive){                                                                                      emissive->drawFit(rect); tex=true;} break;
               case TEX_MACRO     : if(em.   macro_map   .is()                       )if(macro   ){                                                                                      macro   ->drawFit(rect); tex=true;} break;
               case TEX_DET_COLOR : if(em.  detail_color .is()                       )if(detail  ){VI.shader(ShaderFiles("Main")->get(                     "DrawTexWG"               )); detail  ->drawFit(rect); tex=true;} break; // #MaterialTextureLayoutDetail
               case TEX_DET_BUMP  : if(em.  detail_bump  .is()                       )if(detail  ){                                      if(Image *bump=mr->getDetailBump(em.detail_bump))bump    ->drawFit(rect); tex=true;} break; // Detail Bump is not stored in texture
               case TEX_DET_NORMAL: if(em.  detail_normal.is() || em.detail_bump.is())if(detail  ){VI.shader(ShaderFiles("Main")->get(                     "DrawTexDetNrm"           )); detail  ->drawFit(rect); tex=true;} break;
               case TEX_DET_SMOOTH: if(em.  detail_smooth.is()                       )if(detail  ){VI.shader(ShaderFiles("Main")->get(                     "DrawTexZIG"              )); detail  ->drawFit(rect); tex=true;} break; // inverse because it's roughness
             /*case TEX_RFL_L     : if(em. reflection_map.is()                       )if(reflection){reflection->drawCubeFace(WHITE, TRANSPARENT, rect, DIR_LEFT   ); tex=true;} break;
               case TEX_RFL_F     : if(em. reflection_map.is()                       )if(reflection){reflection->drawCubeFace(WHITE, TRANSPARENT, rect, DIR_FORWARD); tex=true;} break;
               case TEX_RFL_R     : if(em. reflection_map.is()                       )if(reflection){reflection->drawCubeFace(WHITE, TRANSPARENT, rect, DIR_RIGHT  ); tex=true;} break;
               case TEX_RFL_B     : if(em. reflection_map.is()                       )if(reflection){reflection->drawCubeFace(WHITE, TRANSPARENT, rect, DIR_BACK   ); tex=true;} break;
               case TEX_RFL_D     : if(em. reflection_map.is()                       )if(reflection){reflection->drawCubeFace(WHITE, TRANSPARENT, rect, DIR_DOWN   ); tex=true;} break;
               case TEX_RFL_U     : if(em. reflection_map.is()                       )if(reflection){reflection->drawCubeFace(WHITE, TRANSPARENT, rect, DIR_UP     ); tex=true;} break;
               case TEX_RFL_ALL   : if(em. reflection_map.is()                       )if(reflection)
               {
                  Image &i=*reflection; flt x[5]={rect.min.x, rect.lerpX(1.0/4), rect.lerpX(2.0/4), rect.lerpX(3.0/4), rect.max.x},
                                            y[4]={rect.min.y, rect.lerpY(1.0/3), rect.lerpY(2.0/3), rect.max.y};
                  i.drawCubeFace(WHITE, TRANSPARENT, Rect(x[0], y[1], x[1], y[2]), DIR_LEFT   );
                  i.drawCubeFace(WHITE, TRANSPARENT, Rect(x[1], y[1], x[2], y[2]), DIR_FORWARD);
                  i.drawCubeFace(WHITE, TRANSPARENT, Rect(x[2], y[1], x[3], y[2]), DIR_RIGHT  );
                  i.drawCubeFace(WHITE, TRANSPARENT, Rect(x[3], y[1], x[4], y[2]), DIR_BACK   );
                  i.drawCubeFace(WHITE, TRANSPARENT, Rect(x[1], y[0], x[2], y[1]), DIR_DOWN   );
                  i.drawCubeFace(WHITE, TRANSPARENT, Rect(x[1], y[2], x[2], y[3]), DIR_UP     );
                  tex=true;
               }break;*/
            }else switch(type) // #MaterialTextureLayoutWater
            {
               case TEX_COLOR : if(em.color_map.is())if(base_0){                                                   base_0->drawFit(rect); tex=true;} break;
               case TEX_BUMP  : if(em.  hasBumpMap())if(base_2){VI.shader(ShaderFiles("Main")->get("DrawTexXSG")); base_2->drawFit(rect); tex=true;} break;
               case TEX_NORMAL: if(em.hasNormalMap())if(base_1){VI.shader(ShaderFiles("Main")->get("DrawTexNrm")); base_1->drawFit(rect); tex=true;} break;
            }
            D.alpha(alpha);
         }
         return tex;
      }
      void MaterialRegion::Texture::draw(C GuiPC &gpc)
{
         if(gpc.visible && visible())
         {
            D.clip(gpc.clip);
            Rect   r=rect()+gpc.offset;
            TextStyleParams ts; if(draw(r))ts.reset(false).size=0.040f;else ts.reset(true).size=0.036f;
            D.text(ts, r, text);

            // if mouse cursor is over reload button, and source file is not found, then draw exclamation as a warning
            if((Gui.ms()==&mr->reload_base_textures || Gui.ms()==&mr->texture_options)
            && Proj.invalidTexSrc(file))
               Proj.exclamation->drawFit(Rect_C(r.center(), 0.08f, 0.08f));

            r.draw(rect_color, false);
         }
      }
   void MaterialRegion::undoVis() {SetUndo(undos, undo, redo);}
   Vec MaterialRegion::previewLight()C {return Matrix3().setRotateXY(light_angle.y-ActiveCam.pitch, light_angle.x-ActiveCam.yaw).z;}
   void MaterialRegion::Render() {MtrlEdit.render();}
          void MaterialRegion::render()
   {
      switch(Renderer())
      {
         case RM_PREPARE:
         {
            if(InRange(preview_mode(), preview_mesh))
            {
               Matrix m; m.setScale((game && game->technique==MTECH_FUR) ? 0.75f : 1.0f);
               preview_mesh[preview_mode()].draw(m);
            }

            LightDir(previewLight(), 1-D.ambientColorL()).add(false);
         }break;
      }
   }
   void MaterialRegion::DrawPreview(Viewport &viewport) {((MaterialRegion*)viewport.user)->drawPreview();}
   void MaterialRegion::drawPreview()
   {
      preview_cam.set();

      REPAO(preview_mesh).material(game).setShader();

      CurrentEnvironment().set();
      MOTION_MODE  motion   =D.   motionMode(); D.   motionMode( MOTION_NONE);
      AMBIENT_MODE ambient  =D.  ambientMode(); D.  ambientMode(AMBIENT_FLAT);
      DOF_MODE     dof      =D.      dofMode(); D.      dofMode(    DOF_NONE);
      bool         eye_adapt=D.eyeAdaptation(); D.eyeAdaptation(       false);
      bool         astros   =AstrosDraw       ; AstrosDraw     =false;
      bool         ocean    =Water.draw       ; Water.draw     =false;

      Renderer(Render);

      D.      dofMode(dof      );
      D.   motionMode(motion   );
      D.  ambientMode(ambient  );
      D.eyeAdaptation(eye_adapt);
      AstrosDraw     =astros;
      Water.draw     =ocean;
   }
   void MaterialRegion::PreChanged(C Property &prop) {cptr change_type=&prop; if(change_type==MtrlEdit.green || change_type==MtrlEdit.blue)change_type=MtrlEdit.red; MtrlEdit.undos.set(change_type);}
   void    MaterialRegion::Changed(C Property &prop) {MtrlEdit.setChanged();}
   Str  MaterialRegion::Tech(C MaterialRegion &mr          ) {REPA(mtrl_techs)if(mtrl_techs[i].tech==mr.edit.tech)return i; return S;}
   void MaterialRegion::Tech(  MaterialRegion &mr, C Str &t) {int i=TextInt(t); if(InRange(i, mtrl_techs)){mr.edit.tech=mtrl_techs[i].tech; mr.edit.tech_time.now(); mr.setChanged(); D.setShader(mr.game());}}
   Str  MaterialRegion::TexDownsizeMobile(C MaterialRegion &mr          ) {return mr.edit.tex_downsize[TSP_MOBILE];}
   void MaterialRegion::TexDownsizeMobile(  MaterialRegion &mr, C Str &t) {       mr.edit.tex_downsize[TSP_MOBILE]=TextInt(t); mr.edit.tex_downsize_time.getUTC();}
   Str  MaterialRegion::TexDownsizeSwitch(C MaterialRegion &mr          ) {return mr.edit.tex_downsize[TSP_SWITCH];}
   void MaterialRegion::TexDownsizeSwitch(  MaterialRegion &mr, C Str &t) {       mr.edit.tex_downsize[TSP_SWITCH]=TextInt(t); mr.edit.tex_downsize_time.getUTC();}
   Str  MaterialRegion::TexQuality(C MaterialRegion &mr          ) {REPA(TexQualities)if(TexQualities[i].quality==mr.edit.tex_quality)return i; return S;}
   void MaterialRegion::TexQuality(  MaterialRegion &mr, C Str &t) {int i=TextInt(t); if(InRange(i, TexQualities))mr.texQuality(TexQualities[i].quality, false);}
   void MaterialRegion::RGB1(MaterialRegion &mr) {mr.undos.set("rgb1"); mr.edit.color_s.xyz=1; mr.edit.color_time.getUTC(); mr.setChanged(); mr.toGui();}
   void MaterialRegion::RGB(MaterialRegion &mr)
   {
      mr.undos.set("brightness");
      Vec2 d=0; int on=0, pd=0; REPA(MT)if(MT.b(i) && MT.guiObj(i)==&mr.brightness){d+=MT.ad(i); MT.freeze(i); if(MT.bp(i))pd++;else on++;}
      Vec &rgb=mr.edit.color_s.xyz; if(pd && !on){mr.mouse_edit_value=rgb; mr.mouse_edit_delta=0;} flt d_sum=d.sum(); if(mr.red)d_sum*=mr.red->mouse_edit_speed; mr.mouse_edit_delta+=d_sum;
      flt  max=mr.mouse_edit_value.max(), lum=max+mr.mouse_edit_delta;
      if(mr.red)
      {
         if(mr.red->min_use){flt min=mr.red->min_value; if(lum<min){mr.mouse_edit_delta-=lum-min; lum=min;}}
         if(mr.red->max_use){flt max=mr.red->max_value; if(lum>max){mr.mouse_edit_delta-=lum-max; lum=max;}}
      }
      Vec v=mr.mouse_edit_value; if(max)v/=max;else v=1; v*=lum;
      if(mr.red  ){mr.red  ->set(v.x, QUIET); rgb.x=mr.red  ->asFlt();}
      if(mr.green){mr.green->set(v.y, QUIET); rgb.y=mr.green->asFlt();}
      if(mr.blue ){mr.blue ->set(v.z, QUIET); rgb.z=mr.blue ->asFlt();}
      mr.edit.color_time.getUTC(); mr.setChanged();
   }
   void MaterialRegion::Emissive(MaterialRegion &mr)
   {
      mr.undos.set("Emissive");
      Vec2 d=0; int on=0, pd=0; REPA(MT)if(MT.b(i) && MT.guiObj(i)==&mr.emissive){d+=MT.ad(i); MT.freeze(i); if(MT.bp(i))pd++;else on++;}
      Vec &rgb=mr.edit.emissive_s; if(pd && !on){mr.mouse_edit_value=rgb; mr.mouse_edit_delta=0;} flt d_sum=d.sum(); if(mr.emit_red)d_sum*=mr.emit_red->mouse_edit_speed; mr.mouse_edit_delta+=d_sum;
      flt  max=mr.mouse_edit_value.max(), lum=max+mr.mouse_edit_delta;
      if(mr.emit_red)
      {
         if(mr.emit_red->min_use){flt min=mr.emit_red->min_value; if(lum<min){mr.mouse_edit_delta-=lum-min; lum=min;}}
         if(mr.emit_red->max_use){flt max=mr.emit_red->max_value; if(lum>max){mr.mouse_edit_delta-=lum-max; lum=max;}}
      }
      Vec  v  =mr.mouse_edit_value; if(max)v/=max;else v=1; v*=lum;
      if(mr.emit_red  ){mr.emit_red  ->set(v.x, QUIET); rgb.x=mr.emit_red  ->asFlt();}
      if(mr.emit_green){mr.emit_green->set(v.y, QUIET); rgb.y=mr.emit_green->asFlt();}
      if(mr.emit_blue ){mr.emit_blue ->set(v.z, QUIET); rgb.z=mr.emit_blue ->asFlt();}
      mr.edit.emissive_time.getUTC(); mr.setChanged(); D.setShader(mr.game());
   }
   Str  MaterialRegion::Red(C MaterialRegion &mr          ) {return mr.edit.color_s.x;}
   void MaterialRegion::Red(  MaterialRegion &mr, C Str &t) {       mr.edit.color_s.x=TextFlt(t); mr.edit.color_time.getUTC();}
   Str  MaterialRegion::Green(C MaterialRegion &mr          ) {return mr.edit.color_s.y;}
   void MaterialRegion::Green(  MaterialRegion &mr, C Str &t) {       mr.edit.color_s.y=TextFlt(t); mr.edit.color_time.getUTC();}
   Str  MaterialRegion::Blue(C MaterialRegion &mr          ) {return mr.edit.color_s.z;}
   void MaterialRegion::Blue(  MaterialRegion &mr, C Str &t) {       mr.edit.color_s.z=TextFlt(t); mr.edit.color_time.getUTC();}
   Str  MaterialRegion::Alpha(C MaterialRegion &mr          ) {return mr.edit.color_s.w;}
   void MaterialRegion::Alpha(  MaterialRegion &mr, C Str &t) {       mr.edit.color_s.w=TextFlt(t); mr.edit.color_time.getUTC();}
   Str  MaterialRegion::Bump(C MaterialRegion &mr          ) {return mr.edit.bump/BumpScale;}
   void MaterialRegion::Bump(  MaterialRegion &mr, C Str &t) {       mr.edit.bump=TextFlt(t)*BumpScale; mr.edit.bump_time.getUTC(); mr.setChanged(); D.setShader(mr.game());}
   Str  MaterialRegion::NrmScale(C MaterialRegion &mr          ) {return mr.edit.normal;}
   void MaterialRegion::NrmScale(  MaterialRegion &mr, C Str &t) {       mr.edit.normal=TextFlt(t); mr.edit.normal_time.getUTC(); mr.setChanged(); D.setShader(mr.game());}
   Str  MaterialRegion::FNY(C MaterialRegion &mr          ) {return mr.edit.flip_normal_y;}
   void MaterialRegion::FNY(  MaterialRegion &mr, C Str &t) {TEX_FLAG textures=mr.edit.textures(); mr.edit.flip_normal_y=TextBool(t); mr.edit.flip_normal_y_time.getUTC(); mr.rebuildBase(textures, EditMaterial::CHANGED_FLIP_NRM_Y, false);}
   Str  MaterialRegion::RoughImage(C MaterialRegion &mr          ) {return mr.edit.smooth_is_rough;}
   void MaterialRegion::RoughImage(  MaterialRegion &mr, C Str &t) {TEX_FLAG textures=mr.edit.textures(); mr.edit.smooth_is_rough=TextBool(t); mr.edit.smooth_is_rough_time.getUTC(); mr.rebuildBase(textures, EditMaterial::CHANGED_SMOOTH_IS_ROUGH, false);}
   Str  MaterialRegion::Smooth(C MaterialRegion &mr          ) {return mr.edit.smooth;}
   void MaterialRegion::Smooth(  MaterialRegion &mr, C Str &t) {       mr.edit.smooth=TextFlt(t); mr.edit.smooth_time.getUTC();}
   Str  MaterialRegion::ReflectMin(C MaterialRegion &mr          ) {return mr.edit.reflect_min;}
   void MaterialRegion::ReflectMin(  MaterialRegion &mr, C Str &t) {       mr.edit.reflect_min=TextFlt(t); mr.edit.reflect_time.getUTC();}
   Str  MaterialRegion::ReflectMax(C MaterialRegion &mr          ) {return mr.edit.reflect_max;}
   void MaterialRegion::ReflectMax(  MaterialRegion &mr, C Str &t) {       mr.edit.reflect_max=TextFlt(t); mr.edit.reflect_time.getUTC();}
   Str  MaterialRegion::Glow(C MaterialRegion &mr          ) {return mr.edit.glow;}
   void MaterialRegion::Glow(  MaterialRegion &mr, C Str &t) {       mr.edit.glow=TextFlt(t); mr.edit.glow_time.getUTC();}
   Str  MaterialRegion::DetUVScale(C MaterialRegion &mr          ) {return mr.edit.det_uv_scale;}
   void MaterialRegion::DetUVScale(  MaterialRegion &mr, C Str &t) {       mr.edit.det_uv_scale=TextFlt(t); mr.edit.detail_time.getUTC();}
   Str  MaterialRegion::DetPower(C MaterialRegion &mr          ) {return mr.edit.det_power;}
   void MaterialRegion::DetPower(  MaterialRegion &mr, C Str &t) {       mr.edit.det_power=TextFlt(t); mr.edit.detail_time.getUTC(); mr.setChanged(); D.setShader(mr.game());}
   Str  MaterialRegion::DetLOD(C MaterialRegion &mr          ) {return mr.edit.detail_all_lod;}
   void MaterialRegion::DetLOD(  MaterialRegion &mr, C Str &t) {       mr.edit.detail_all_lod=TextBool(t); mr.edit.detail_all_lod_time.now(); mr.setChanged(); D.setShader(mr.game());}
   Str  MaterialRegion::Cull(C MaterialRegion &mr          ) {return mr.edit.cull;}
   void MaterialRegion::Cull(  MaterialRegion &mr, C Str &t) {       mr.edit.cull=TextBool(t); mr.edit.cull_time.now();}
   Str  MaterialRegion::EmissiveR(C MaterialRegion &mr          ) {return mr.edit.emissive_s.x;}
   void MaterialRegion::EmissiveR(  MaterialRegion &mr, C Str &t) {       mr.edit.emissive_s.x=TextFlt(t); mr.edit.emissive_time.getUTC(); mr.setChanged(); D.setShader(mr.game());}
   Str  MaterialRegion::EmissiveG(C MaterialRegion &mr          ) {return mr.edit.emissive_s.y;}
   void MaterialRegion::EmissiveG(  MaterialRegion &mr, C Str &t) {       mr.edit.emissive_s.y=TextFlt(t); mr.edit.emissive_time.getUTC(); mr.setChanged(); D.setShader(mr.game());}
   Str  MaterialRegion::EmissiveB(C MaterialRegion &mr          ) {return mr.edit.emissive_s.z;}
   void MaterialRegion::EmissiveB(  MaterialRegion &mr, C Str &t) {       mr.edit.emissive_s.z=TextFlt(t); mr.edit.emissive_time.getUTC(); mr.setChanged(); D.setShader(mr.game());}
   Str  MaterialRegion::EmissiveGlow(C MaterialRegion &mr          ) {return mr.edit.emissive_glow;}
   void MaterialRegion::EmissiveGlow(  MaterialRegion &mr, C Str &t) {       mr.edit.emissive_glow=TextFlt(t); mr.edit.emissive_time.getUTC(); mr.setChanged(); D.setShader(mr.game());}
   Str  MaterialRegion::UVScale(C MaterialRegion &mr          ) {return mr.edit.uv_scale;}
   void MaterialRegion::UVScale(  MaterialRegion &mr, C Str &t) {       mr.edit.uv_scale=TextFlt(t); mr.edit.uv_scale_time.getUTC();}
   void MaterialRegion::Undo(MaterialRegion &editor) {editor.undos.undo();}
   void MaterialRegion::Redo(MaterialRegion &editor) {editor.undos.redo();}
   void MaterialRegion::Locate(MaterialRegion &editor) {Proj.elmLocate(editor.elm_id);}
   void MaterialRegion::Hide(MaterialRegion &editor) {editor.set(null);}
   void MaterialRegion::SetMtrl(MaterialRegion &editor) {SetObjOp(editor.set_mtrl() ? OP_OBJ_SET_MTRL : OP_OBJ_NONE);}
   void MaterialRegion::AutoReload(MaterialRegion &editor) {editor.auto_reload=editor.texture_options.menu(auto_reload_name);}
   void MaterialRegion::ReloadBaseTextures(MaterialRegion &editor) {editor.undos.set("rebuildBase"); editor.rebuildBase(editor.getEditMtrl().textures(), 0, false, true);}
   void MaterialRegion::ResizeBase128(MaterialRegion &editor) {editor.resizeBase(128);}
   void MaterialRegion::ResizeBase256(MaterialRegion &editor) {editor.resizeBase(256);}
   void MaterialRegion::ResizeBase512(MaterialRegion &editor) {editor.resizeBase(512);}
   void MaterialRegion::ResizeBase1024(MaterialRegion &editor) {editor.resizeBase(1024);}
   void MaterialRegion::ResizeBase2048(MaterialRegion &editor) {editor.resizeBase(2048);}
   void MaterialRegion::ResizeBase4096(MaterialRegion &editor) {editor.resizeBase(4096);}
   void MaterialRegion::ResizeBase128x64(MaterialRegion &editor) {editor.resizeBase(VecI2(128, 64));}
   void MaterialRegion::ResizeBase256x128(MaterialRegion &editor) {editor.resizeBase(VecI2(256, 128));}
   void MaterialRegion::ResizeBase512x256(MaterialRegion &editor) {editor.resizeBase(VecI2(512, 256));}
   void MaterialRegion::ResizeBase1024x512(MaterialRegion &editor) {editor.resizeBase(VecI2(1024, 512));}
   void MaterialRegion::ResizeBase2048x1024(MaterialRegion &editor) {editor.resizeBase(VecI2(2048, 1024));}
   void MaterialRegion::ResizeBase64x128(MaterialRegion &editor) {editor.resizeBase(VecI2(64, 128));}
   void MaterialRegion::ResizeBase128x256(MaterialRegion &editor) {editor.resizeBase(VecI2(128, 256));}
   void MaterialRegion::ResizeBase256x512(MaterialRegion &editor) {editor.resizeBase(VecI2(256, 512));}
   void MaterialRegion::ResizeBase512x1024(MaterialRegion &editor) {editor.resizeBase(VecI2(512, 1024));}
   void MaterialRegion::ResizeBase1024x2048(MaterialRegion &editor) {editor.resizeBase(VecI2(1024, 2048));}
   void MaterialRegion::ResizeBaseQuarter(MaterialRegion &editor) {editor.resizeBase(-2, true);}
   void MaterialRegion::ResizeBaseHalf(MaterialRegion &editor) {editor.resizeBase(-1, true);}
   void MaterialRegion::ResizeBaseOriginal(MaterialRegion &editor) {editor.resizeBase( 0, true);}
   void MaterialRegion::ResizeBaseDouble(MaterialRegion &editor) {editor.resizeBase( 1, true);}
   void MaterialRegion::ResizeBase0_128(MaterialRegion &editor) {editor.resizeBase0(128);}
   void MaterialRegion::ResizeBase0_256(MaterialRegion &editor) {editor.resizeBase0(256);}
   void MaterialRegion::ResizeBase0_512(MaterialRegion &editor) {editor.resizeBase0(512);}
   void MaterialRegion::ResizeBase0_1024(MaterialRegion &editor) {editor.resizeBase0(1024);}
   void MaterialRegion::ResizeBase0_2048(MaterialRegion &editor) {editor.resizeBase0(2048);}
   void MaterialRegion::ResizeBase0_4096(MaterialRegion &editor) {editor.resizeBase0(4096);}
   void MaterialRegion::ResizeBase0_128x64(MaterialRegion &editor) {editor.resizeBase0(VecI2(128, 64));}
   void MaterialRegion::ResizeBase0_256x128(MaterialRegion &editor) {editor.resizeBase0(VecI2(256, 128));}
   void MaterialRegion::ResizeBase0_512x256(MaterialRegion &editor) {editor.resizeBase0(VecI2(512, 256));}
   void MaterialRegion::ResizeBase0_1024x512(MaterialRegion &editor) {editor.resizeBase0(VecI2(1024, 512));}
   void MaterialRegion::ResizeBase0_2048x1024(MaterialRegion &editor) {editor.resizeBase0(VecI2(2048, 1024));}
   void MaterialRegion::ResizeBase0_64x128(MaterialRegion &editor) {editor.resizeBase0(VecI2(64, 128));}
   void MaterialRegion::ResizeBase0_128x256(MaterialRegion &editor) {editor.resizeBase0(VecI2(128, 256));}
   void MaterialRegion::ResizeBase0_256x512(MaterialRegion &editor) {editor.resizeBase0(VecI2(256, 512));}
   void MaterialRegion::ResizeBase0_512x1024(MaterialRegion &editor) {editor.resizeBase0(VecI2(512, 1024));}
   void MaterialRegion::ResizeBase0_1024x2048(MaterialRegion &editor) {editor.resizeBase0(VecI2(1024, 2048));}
   void MaterialRegion::ResizeBase0_Quarter(MaterialRegion &editor) {editor.resizeBase0(-2, true);}
   void MaterialRegion::ResizeBase0_Half(MaterialRegion &editor) {editor.resizeBase0(-1, true);}
   void MaterialRegion::ResizeBase0_Original(MaterialRegion &editor) {editor.resizeBase0( 0, true);}
   void MaterialRegion::ResizeBase0_Double(MaterialRegion &editor) {editor.resizeBase0( 1, true);}
   void MaterialRegion::ResizeBase1_128(MaterialRegion &editor) {editor.resizeBase1(128);}
   void MaterialRegion::ResizeBase1_256(MaterialRegion &editor) {editor.resizeBase1(256);}
   void MaterialRegion::ResizeBase1_512(MaterialRegion &editor) {editor.resizeBase1(512);}
   void MaterialRegion::ResizeBase1_1024(MaterialRegion &editor) {editor.resizeBase1(1024);}
   void MaterialRegion::ResizeBase1_2048(MaterialRegion &editor) {editor.resizeBase1(2048);}
   void MaterialRegion::ResizeBase1_4096(MaterialRegion &editor) {editor.resizeBase1(4096);}
   void MaterialRegion::ResizeBase1_128x64(MaterialRegion &editor) {editor.resizeBase1(VecI2(128, 64));}
   void MaterialRegion::ResizeBase1_256x128(MaterialRegion &editor) {editor.resizeBase1(VecI2(256, 128));}
   void MaterialRegion::ResizeBase1_512x256(MaterialRegion &editor) {editor.resizeBase1(VecI2(512, 256));}
   void MaterialRegion::ResizeBase1_1024x512(MaterialRegion &editor) {editor.resizeBase1(VecI2(1024, 512));}
   void MaterialRegion::ResizeBase1_2048x1024(MaterialRegion &editor) {editor.resizeBase1(VecI2(2048, 1024));}
   void MaterialRegion::ResizeBase1_64x128(MaterialRegion &editor) {editor.resizeBase1(VecI2(64, 128));}
   void MaterialRegion::ResizeBase1_128x256(MaterialRegion &editor) {editor.resizeBase1(VecI2(128, 256));}
   void MaterialRegion::ResizeBase1_256x512(MaterialRegion &editor) {editor.resizeBase1(VecI2(256, 512));}
   void MaterialRegion::ResizeBase1_512x1024(MaterialRegion &editor) {editor.resizeBase1(VecI2(512, 1024));}
   void MaterialRegion::ResizeBase1_1024x2048(MaterialRegion &editor) {editor.resizeBase1(VecI2(1024, 2048));}
   void MaterialRegion::ResizeBase1_Quarter(MaterialRegion &editor) {editor.resizeBase1(-2, true);}
   void MaterialRegion::ResizeBase1_Half(MaterialRegion &editor) {editor.resizeBase1(-1, true);}
   void MaterialRegion::ResizeBase1_Original(MaterialRegion &editor) {editor.resizeBase1( 0, true);}
   void MaterialRegion::ResizeBase1_Double(MaterialRegion &editor) {editor.resizeBase1( 1, true);}
   void MaterialRegion::ResizeBase2_128(MaterialRegion &editor) {editor.resizeBase2(128);}
   void MaterialRegion::ResizeBase2_256(MaterialRegion &editor) {editor.resizeBase2(256);}
   void MaterialRegion::ResizeBase2_512(MaterialRegion &editor) {editor.resizeBase2(512);}
   void MaterialRegion::ResizeBase2_1024(MaterialRegion &editor) {editor.resizeBase2(1024);}
   void MaterialRegion::ResizeBase2_2048(MaterialRegion &editor) {editor.resizeBase2(2048);}
   void MaterialRegion::ResizeBase2_4096(MaterialRegion &editor) {editor.resizeBase2(4096);}
   void MaterialRegion::ResizeBase2_128x64(MaterialRegion &editor) {editor.resizeBase2(VecI2(128, 64));}
   void MaterialRegion::ResizeBase2_256x128(MaterialRegion &editor) {editor.resizeBase2(VecI2(256, 128));}
   void MaterialRegion::ResizeBase2_512x256(MaterialRegion &editor) {editor.resizeBase2(VecI2(512, 256));}
   void MaterialRegion::ResizeBase2_1024x512(MaterialRegion &editor) {editor.resizeBase2(VecI2(1024, 512));}
   void MaterialRegion::ResizeBase2_2048x1024(MaterialRegion &editor) {editor.resizeBase2(VecI2(2048, 1024));}
   void MaterialRegion::ResizeBase2_64x128(MaterialRegion &editor) {editor.resizeBase2(VecI2(64, 128));}
   void MaterialRegion::ResizeBase2_128x256(MaterialRegion &editor) {editor.resizeBase2(VecI2(128, 256));}
   void MaterialRegion::ResizeBase2_256x512(MaterialRegion &editor) {editor.resizeBase2(VecI2(256, 512));}
   void MaterialRegion::ResizeBase2_512x1024(MaterialRegion &editor) {editor.resizeBase2(VecI2(512, 1024));}
   void MaterialRegion::ResizeBase2_1024x2048(MaterialRegion &editor) {editor.resizeBase2(VecI2(1024, 2048));}
   void MaterialRegion::ResizeBase2_Quarter(MaterialRegion &editor) {editor.resizeBase2(-2, true);}
   void MaterialRegion::ResizeBase2_Half(MaterialRegion &editor) {editor.resizeBase2(-1, true);}
   void MaterialRegion::ResizeBase2_Original(MaterialRegion &editor) {editor.resizeBase2( 0, true);}
   void MaterialRegion::ResizeBase2_Double(MaterialRegion &editor) {editor.resizeBase2( 1, true);}
   void MaterialRegion::BumpFromCol(MaterialRegion &editor) {editor.bumpFromCol(-1);}
   void MaterialRegion::BumpFromCol2(MaterialRegion &editor) {editor.bumpFromCol( 2);}
   void MaterialRegion::BumpFromCol3(MaterialRegion &editor) {editor.bumpFromCol( 3);}
   void MaterialRegion::BumpFromCol4(MaterialRegion &editor) {editor.bumpFromCol( 4);}
   void MaterialRegion::BumpFromCol5(MaterialRegion &editor) {editor.bumpFromCol( 5);}
   void MaterialRegion::BumpFromCol6(MaterialRegion &editor) {editor.bumpFromCol( 6);}
   void MaterialRegion::BumpFromCol8(MaterialRegion &editor) {editor.bumpFromCol( 8);}
   void MaterialRegion::BumpFromCol12(MaterialRegion &editor) {editor.bumpFromCol(12);}
   void MaterialRegion::BumpFromCol16(MaterialRegion &editor) {editor.bumpFromCol(16);}
   void MaterialRegion::BumpFromCol24(MaterialRegion &editor) {editor.bumpFromCol(24);}
   void MaterialRegion::BumpFromCol32(MaterialRegion &editor) {editor.bumpFromCol(32);}
   void MaterialRegion::MulTexCol(MaterialRegion &editor) {Proj.mtrlMulTexCol     (editor.elm_id);}
   void MaterialRegion::MulTexNormal(MaterialRegion &editor) {Proj.mtrlMulTexNormal  (editor.elm_id);}
   void MaterialRegion::MulTexSmooth(MaterialRegion &editor) {Proj.mtrlMulTexSmooth  (editor.elm_id);}
   void MaterialRegion::MulTexGlow(MaterialRegion &editor) {Proj.mtrlMulTexGlow    (editor.elm_id);}
   void MaterialRegion::MulTexEmissive(MaterialRegion &editor) {Proj.mtrlMulTexEmissive(editor.elm_id);}
   bool MaterialRegion::bigVisible()C {return visible() && big();}
   void   MaterialRegion::setRGB(C Vec                   &srgb              ) {if(edit.color_s.xyz        !=srgb                                ){        undos.set("rgb"       ); edit.color_s.xyz        =srgb                             ; edit.          color_time.getUTC(); setChanged(); toGui();}}
   void   MaterialRegion::setNormal(flt                    normal              ) {if(edit.normal             !=normal                              ){        undos.set("normal"    ); edit.normal             =normal                           ; edit.         normal_time.getUTC(); setChanged(); toGui();}}
   void   MaterialRegion::setSmooth(flt                    smooth              ) {if(edit.smooth             !=smooth                              ){        undos.set("smooth"    ); edit.smooth             =smooth                           ; edit.         smooth_time.getUTC(); setChanged(); toGui();}}
   void   MaterialRegion::setReflect(flt reflect_min, flt reflect_max           ) {if(edit.reflect_min!=reflect_min || edit.reflect_max!=reflect_max){        undos.set("reflect"   ); edit.reflect_min=reflect_min; edit.reflect_max=reflect_max; edit.        reflect_time.getUTC(); setChanged(); toGui();}}
   void MaterialRegion::resetAlpha(                                           ) {                                                                           undos.set("alpha"     ); edit.resetAlpha()                                         ;                                     setChanged(); toGui(); }
   void MaterialRegion::cull(bool                      on               ) {if(edit.cull               !=on                                  ){        undos.set("cull"      ); edit.cull               =on                               ; edit.           cull_time.getUTC(); setChanged(); toGui();}}
   void MaterialRegion::flipNrmY(bool                      on               ) {if(edit.flip_normal_y      !=on                                  ){        undos.set("fny"       ); edit.flip_normal_y      =on                               ; edit.  flip_normal_y_time.getUTC(); rebuildBase(edit.textures(), EditMaterial::CHANGED_FLIP_NRM_Y     , false);}}
   void MaterialRegion::smoothIsRough(bool                      on               ) {if(edit.smooth_is_rough    !=on                                  ){        undos.set("sir"       ); edit.smooth_is_rough    =on                               ; edit.smooth_is_rough_time.getUTC(); rebuildBase(edit.textures(), EditMaterial::CHANGED_SMOOTH_IS_ROUGH, false);}}
   void MaterialRegion::texDownsize(TEX_SIZE_PLATFORM tsp, byte downsize       ) {if(edit.tex_downsize[tsp]  !=downsize                            ){        undos.set("texSize"   ); edit.tex_downsize[tsp]  =downsize                         ; edit.   tex_downsize_time.getUTC(); setChanged(); toGui();}}
   void MaterialRegion::texQuality(Edit::Material::TEX_QUALITY q, bool undo) {if(edit.tex_quality        !=q                                   ){if(undo)undos.set("texQuality"); edit.tex_quality        =q                                ; edit.    tex_quality_time.getUTC(); rebuildBase(edit.textures(), 0, false);}}
   void MaterialRegion::resizeBase(C VecI2 &size, bool relative)
   {
      undos.set("resizeBase");
      TimeStamp time; time.getUTC();
      VecI2 sizes[3]={size, size, size};

      if(relative && size.any()) // if we want to have relative size and not original, then first revert to original size
         if(Proj.forceImageSize(edit. color_map, 0, relative, edit. color_map_time, time) // !! use '|' because all need to be processed !!
         |  Proj.forceImageSize(edit. alpha_map, 0, relative, edit. alpha_map_time, time)
         |  Proj.forceImageSize(edit.  bump_map, 0, relative, edit.  bump_map_time, time)
         |  Proj.forceImageSize(edit.normal_map, 0, relative, edit.normal_map_time, time)
         |  Proj.forceImageSize(edit.smooth_map, 0, relative, edit.smooth_map_time, time)
         |  Proj.forceImageSize(edit. metal_map, 0, relative, edit. metal_map_time, time)
         |  Proj.forceImageSize(edit.  glow_map, 0, relative, edit.  glow_map_time, time))
      {
         MtrlImages mi; mi.fromMaterial(edit, Proj); mi.baseTextureSizes(&sizes[0], &sizes[1], &sizes[2]); // calculate actual sizes
         REPA(sizes)
         {
            sizes[i].set(Max(1, Shl(sizes[i].x, size.x)), Max(1, Shl(sizes[i].y, size.y)));
            sizes[i].set(NearestPow2(sizes[i].x), NearestPow2(sizes[i].y)); // textures are gonna be resized to pow2 anyway, so force pow2 size, to avoid double resize
         }
         // #MaterialTextureLayout
         if(sizes[1]!=sizes[2])edit.separateNormalMap(time); // normal can be from bump
         relative=false; // we now have the sizes known, so disable relative mode
      }

      // #MaterialTextureLayout
      if(Proj.forceImageSize(edit. color_map, sizes[0], relative, edit. color_map_time, time) // !! use '|' because all need to be processed !!
      |  Proj.forceImageSize(edit. alpha_map, sizes[0], relative, edit. alpha_map_time, time)
      |  Proj.forceImageSize(edit.  bump_map, sizes[2], relative, edit.  bump_map_time, time)
      |  Proj.forceImageSize(edit.normal_map, sizes[1], relative, edit.normal_map_time, time)
      |  Proj.forceImageSize(edit.smooth_map, sizes[2], relative, edit.smooth_map_time, time)
      |  Proj.forceImageSize(edit. metal_map, sizes[2], relative, edit. metal_map_time, time)
      |  Proj.forceImageSize(edit.  glow_map, sizes[2], relative, edit.  glow_map_time, time))
      {
         edit.cleanupMaps();
         rebuildBase(edit.textures());
      }
   }
   void MaterialRegion::resizeBase0(C VecI2 &size, bool relative)
   {
      // #MaterialTextureLayout
      undos.set("resizeBase");
      TimeStamp time; time.getUTC();
      VecI2 size0=size;

      if(relative && size.any()) // if we want to have relative size and not original, then first revert to original size
         if(Proj.forceImageSize(edit.color_map, 0, relative, edit.color_map_time, time) // !! use '|' because all need to be processed !!
         |  Proj.forceImageSize(edit.alpha_map, 0, relative, edit.alpha_map_time, time))
      {
         MtrlImages mi; mi.fromMaterial(edit, Proj); mi.baseTextureSizes(&size0, null, null); // calculate actual sizes
         size0.set(Max(1, Shl(size0.x, size.x)), Max(1, Shl(size0.y, size.y)));
         size0.set(NearestPow2(size0.x), NearestPow2(size0.y)); // textures are gonna be resized to pow2 anyway, so force pow2 size, to avoid double resize
         relative=false; // we now have the sizes known, so disable relative mode
      }

      if(Proj.forceImageSize(edit.color_map, size0, relative, edit.color_map_time, time) // !! use '|' because all need to be processed !!
      |  Proj.forceImageSize(edit.alpha_map, size0, relative, edit.alpha_map_time, time))
      {
         edit.cleanupMaps();
         rebuildBase(edit.textures());
      }
   }
   void MaterialRegion::resizeBase1(C VecI2 &size, bool relative)
   {
      // #MaterialTextureLayout
      undos.set("resizeBase");
      TimeStamp time; time.getUTC();
      VecI2 size1=size;

      if(relative || game && game->base_2 && game->base_2->size()!=size1)edit.separateNormalMap(time); // separate if needed (normal can be from bump), and before reverting

      if(relative && size.any()) // if we want to have relative size and not original, then first revert to original size
         if(Proj.forceImageSize(edit.normal_map, 0, relative, edit.normal_map_time, time))
      {
         MtrlImages mi; mi.fromMaterial(edit, Proj); mi.baseTextureSizes(null, &size1, null); // calculate actual sizes
         size1.set(Max(1, Shl(size1.x, size.x)), Max(1, Shl(size1.y, size.y)));
         size1.set(NearestPow2(size1.x), NearestPow2(size1.y)); // textures are gonna be resized to pow2 anyway, so force pow2 size, to avoid double resize
         relative=false; // we now have the sizes known, so disable relative mode
      }

      if(Proj.forceImageSize(edit.normal_map, size1, relative, edit.normal_map_time, time))
      {
         edit.cleanupMaps();
         rebuildBase(edit.textures());
      }
   }
   void MaterialRegion::resizeBase2(C VecI2 &size, bool relative)
   {
      // #MaterialTextureLayout
      undos.set("resizeBase");
      TimeStamp time; time.getUTC();
      VecI2 size2=size;

    //if(relative || game && game->base_1 && game->base_1->size()!=size2)edit.separateNormalMap(time); // separate if needed (normal can be from bump), and before reverting

      if(relative && size.any()) // if we want to have relative size and not original, then first revert to original size
         if(Proj.forceImageSize(edit.smooth_map, 0, relative, edit.smooth_map_time, time)  // !! use '|' because all need to be processed !!
         |  Proj.forceImageSize(edit. metal_map, 0, relative, edit. metal_map_time, time)
         |  Proj.forceImageSize(edit.  bump_map, 0, relative, edit.  bump_map_time, time)
         |  Proj.forceImageSize(edit.  glow_map, 0, relative, edit.  glow_map_time, time))
      {
         MtrlImages mi; mi.fromMaterial(edit, Proj); mi.baseTextureSizes(null, null, &size2); // calculate actual sizes
         size2.set(Max(1, Shl(size2.x, size.x)), Max(1, Shl(size2.y, size.y)));
         size2.set(NearestPow2(size2.x), NearestPow2(size2.y)); // textures are gonna be resized to pow2 anyway, so force pow2 size, to avoid double resize
         relative=false; // we now have the sizes known, so disable relative mode
      }

      if(Proj.forceImageSize(edit.smooth_map, size2, relative, edit.smooth_map_time, time)  // !! use '|' because all need to be processed !!
      |  Proj.forceImageSize(edit. metal_map, size2, relative, edit. metal_map_time, time)
      |  Proj.forceImageSize(edit.  bump_map, size2, relative, edit.  bump_map_time, time)
      |  Proj.forceImageSize(edit.  glow_map, size2, relative, edit.  glow_map_time, time))
      {
         edit.cleanupMaps();
         rebuildBase(edit.textures());
      }
   }
   void MaterialRegion::bumpFromCol(int blur)
   {
      undos.set("bumpFromCol");
      EditMaterial &edit=getEditMtrl();
      TEX_FLAG textures=edit.textures(); // get current state of textures before making any change
      edit.bump_map=BumpFromColTransform(edit.color_map, blur); edit.bump_map_time.now();
      rebuildBase(textures, TEXF_BUMP);
   }
   EditMaterial& MaterialRegion::getEditMtrl() {return edit;}
   C ImagePtr    & MaterialRegion::getBase0() {return game->base_0;}
   C ImagePtr    & MaterialRegion::getBase1() {return game->base_1;}
   C ImagePtr    & MaterialRegion::getBase2() {return game->base_2;}
   C ImagePtr    & MaterialRegion::getDetail() {return game->  detail_map;}
   C ImagePtr    & MaterialRegion::getMacro() {return game->   macro_map;}
   C ImagePtr    & MaterialRegion::getEmissive() {return game->emissive_map;}
   bool          MaterialRegion::water()C{return false;}
   void MaterialRegion::setBottom(C Rect &prop_rect)
   {
      reload_base_textures.rect(Rect_LU(0.10f, prop_rect.min.y-0.05f, 0.42f, 0.053f));
           texture_options.rect(Rect_LU(reload_base_textures.rect().ru(), reload_base_textures.rect().h()));
   }
   void MaterialRegion::setSmoothParam()
   {
      if(smooth)
      {
         if(edit.smooth_map.is())
         {
          //if(tweak){smooth.display("Smooth Tweak"); smooth.range(-1, 1);}else
                     {smooth->display("Smoothness"  ); smooth->range( 0, 4);}
         }else       {smooth->display("Smoothness"  ); smooth->range( 0, 1);}
      }
   }
   void MaterialRegion::create()
   {
      Gui+=super::create(Rect_LU(0, 0, 0.73f, 1)).skin(&LightSkin, false).hide();
      flt w=rect().w()-slidebarSize(), e=0.01f, we=w-e*2, p=0.007f, h=0.05f, prop_height=0.044f;
        T+=big         .create(Rect_LU(e, 0, h*1.6f, h), "<<").focusable(false); big.mode=BUTTON_TOGGLE;
        T+=set_mtrl    .create(Rect_LU(big.rect().max.x+p, big.rect().max.y, h, h)).func(SetMtrl, T).focusable(false).desc("Enable this and click on the screen to set material at that location.\nOptionally hold Shift key to completely replace target material on all parts that use it."); set_mtrl.mode=BUTTON_TOGGLE; set_mtrl.image="Gui/Misc/set.img";
        T+=undo        .create(Rect_LU(set_mtrl.rect().ru()+Vec2(p, 0), 0.05f, 0.05f )).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
        T+=redo        .create(Rect_LU(undo.rect().ru()               , 0.05f, 0.05f )).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
        T+=locate      .create(Rect_L (redo.rect().right() +Vec2(p, 0), 0.11f, 0.043f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
        T+=close       .create(Rect_RU(w-e, 0, h, h)).func(Hide, T); close.image="Gui/close.img"; close.skin=&EmptyGuiSkin;
        T+=preview_mode.create(Rect   (locate.rect().max.x+p, redo.rect().min.y, close.rect().min.x-p, close.rect().max.y), 0, (cchar**)null, Elms(preview_mesh)).valid(true).set(0);
        T+=preview     .create(Rect_LU(big.rect().ld()-Vec2(0, 0.007f), we, we), DrawPreview); preview.fov=PreviewFOV; preview.user=this;
      Gui+=preview_big .create(DrawPreview).hide(); preview_big.fov=preview.fov; preview_big.user=preview.user;
        T+=sub         .create(Rect_LU(0, 0, rect().w(), 1)).skin(&EmptyGuiSkin, false); sub.kb_lit=false;
      preview_mode.tab(0).setImage("Gui/Misc/circle.img");
      preview_mode.tab(1).setImage("Gui/Misc/box.img");
      preview_mode.tab(2).setImage("Gui/Misc/tube.img");
      preview_mode.tab(3).setImage("Gui/Misc/grid_small.img");

      Property &tech=props.New().create("Technique", MemberDesc(DATA_INT).setFunc(Tech, Tech)).setEnum();
      ListColumn tech_lc[]=
      {
         ListColumn(MEMBER(MaterialTech, name), LCW_DATA, "name"),
      };
      tech.combobox.setColumns(tech_lc, Elms(tech_lc)).setData(mtrl_techs, Elms(mtrl_techs)); tech.combobox.menu.list.setElmDesc(MEMBER(MaterialTech, desc));

      /*Property &max_tex_size=props.New().create("Max Image Size", MemberDesc(DATA_INT).setFunc(MaxTexSize, MaxTexSize)).setEnum();
      ListColumn mts_lc[]=
      {
         ListColumn(MEMBER(.MaxTexSize, name), LCW_DATA, "name"),
      };
      max_tex_size.combobox.setColumns(mts_lc, Elms(mts_lc)).setData(max_tex_sizes, Elms(max_tex_sizes)); max_tex_size.combobox.menu.list.setElmDesc(MEMBER(.MaxTexSize, desc)); */

  red=&props.New().create("Red"  , MemberDesc(DATA_REAL).setFunc(Red  , Red  )).range(0, 4).mouseEditSpeed(0.4f);
green=&props.New().create("Green", MemberDesc(DATA_REAL).setFunc(Green, Green)).range(0, 4).mouseEditSpeed(0.4f);
 blue=&props.New().create("Blue" , MemberDesc(DATA_REAL).setFunc(Blue , Blue )).range(0, 4).mouseEditSpeed(0.4f);
alpha=&props.New().create("Alpha", MemberDesc(DATA_REAL).setFunc(Alpha, Alpha)).range(0, 1);
    //props.New();
      props.New().create("Bump"           , MemberDesc(DATA_REAL).setFunc(Bump    , Bump    )).range(0, 1);
      props.New().create("Normal"         , MemberDesc(DATA_REAL).setFunc(NrmScale, NrmScale)).range(0, 2);
      props.New().create("Flip Normal Y"  , MemberDesc(DATA_BOOL).setFunc(FNY     , FNY     ));
    //props.New();

        props.New().create("Use Roughness"  , MemberDesc(DATA_BOOL).setFunc(RoughImage, RoughImage)).desc("If source Image is a Roughness Texture instead of Smoothness");
smooth=&props.New().create("Smoothness"     , MemberDesc(DATA_REAL).setFunc(Smooth    , Smooth    )); // range depends on smooth texture presence
        props.New().create("Reflectivity"   , MemberDesc(DATA_REAL).setFunc(ReflectMin, ReflectMin)).range(0, 1).desc(S+"Base Reflectivity\nDefault="+MATERIAL_REFLECT);
        props.New().create("ReflectivityMax", MemberDesc(DATA_REAL).setFunc(ReflectMax, ReflectMax)).range(0, 1).desc("This value specifies the amount of Reflectivity that can be obtained from the Metal texture.\nIn most cases this value should be left at 1.");
        props.New().create("Glow"           , MemberDesc(DATA_REAL).setFunc(Glow      , Glow      )).range(0, 4);

emit_red  =&props.New().create("Emit Red"  , MemberDesc(DATA_REAL).setFunc(EmissiveR   , EmissiveR   )).range(0, 4).mouseEditSpeed(0.4f);
emit_green=&props.New().create("Emit Green", MemberDesc(DATA_REAL).setFunc(EmissiveG   , EmissiveG   )).range(0, 4).mouseEditSpeed(0.4f);
emit_blue =&props.New().create("Emit Blue" , MemberDesc(DATA_REAL).setFunc(EmissiveB   , EmissiveB   )).range(0, 4).mouseEditSpeed(0.4f);
            props.New().create("Emit Glow" , MemberDesc(DATA_REAL).setFunc(EmissiveGlow, EmissiveGlow)).range(0, 4).mouseEditSpeed(0.4f);
    //props.New();
    //props.New().create("Subsurf Scatter", MemberDesc(DATA_REAL).setFunc(SSS , SSS )).range(0, 1);
      props.New().create("Detail UV Scale", MemberDesc(DATA_REAL).setFunc(DetUVScale, DetUVScale)).range(0.01f, 1024).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);
      props.New().create("Detail Power"   , MemberDesc(DATA_REAL).setFunc(DetPower  , DetPower  )).range(0, 1);
      props.New().create("Detail All LODs", MemberDesc(DATA_BOOL).setFunc(DetLOD    , DetLOD    )).desc(S+"If use Detail for all Mesh LODs.\nIf disabled then only 1st Mesh LOD will use Detail.");
    //props.New();

      props.New().create("Cull"         , MemberDesc(DATA_BOOL).setFunc(Cull   , Cull   ));
      props.New().create("UV Scale"     , MemberDesc(DATA_REAL).setFunc(UVScale, UVScale)).range(0.01f, 1024).mouseEditMode(PROP_MOUSE_EDIT_SCALAR);

Property &tq =props.New().create("Tex Quality"    , MemberDesc(DATA_INT).setFunc(TexQuality       , TexQuality       )).setEnum().desc("Select Texture Quality"); tq.combobox.setColumns(NameDescListColumn, Elms(NameDescListColumn)).setData(TexQualities, Elms(TexQualities)); tq.combobox.menu.list.setElmDesc(MEMBER(NameDesc, desc));
Property &tsm=props.New().create("Tex Size Mobile", MemberDesc(DATA_INT).setFunc(TexDownsizeMobile, TexDownsizeMobile)).setEnum(TexDownsizeText, Elms(TexDownsizeText)).desc("If Downsize Textures when making Applications for Android, iOS, Web");
Property &tss=props.New().create("Tex Size Switch", MemberDesc(DATA_INT).setFunc(TexDownsizeSwitch, TexDownsizeSwitch)).setEnum(TexDownsizeText, Elms(TexDownsizeText)).desc("If Downsize Textures when making Applications for Nintendo Switch");

      ts.reset().size=0.038f; ts.align.set(1, 0);
      Rect prop_rect=AddProperties(props, sub, 0, prop_height, 0.16f, &ts); REPAO(props).autoData(this).changed(Changed, PreChanged);
      sub+=emissive  .create(Rect_RU(emit_red->textline.rect().left(), emit_red->button.rect().w(), prop_height*2)).func(Emissive, T).focusable(false).subType(BUTTON_TYPE_PROPERTY_VALUE); emissive  .mode=BUTTON_CONTINUOUS;
      sub+=brightness.create(Rect_RU(     red->textline.rect().left(),      red->button.rect().w(), prop_height*2)).func(RGB     , T).focusable(false).subType(BUTTON_TYPE_PROPERTY_VALUE); brightness.mode=BUTTON_CONTINUOUS;
      sub+=rgb_1.create(Rect_R(brightness.rect().left()-Vec2(0.01f, 0), prop_height, prop_height*2), "1").func(RGB1, T).focusable(false).desc("Set RGB to 1"); rgb_1.text_size/=2;
      tech.combobox.resize(Vec2(0.27f, 0)); // increase size
      tq  .combobox.resize(Vec2(0.12f, 0)); // increase size
      tsm .combobox.resize(Vec2(0.12f, 0)); // increase size
      tss .combobox.resize(Vec2(0.12f, 0)); // increase size

      flt tex_size=prop_height*3; int i=-1; flt s=e-0.003f;
      sub+=texs.New().create(TEX_COLOR     , MEMBER(EditMaterial,      color_map), MEMBER(EditMaterial,      color_map_time), Rect_LU(prop_rect.ru()+Vec2(s           , i*prop_height), tex_size, tex_size), "Color"         , T);
      sub+=texs.New().create(TEX_ALPHA     , MEMBER(EditMaterial,      alpha_map), MEMBER(EditMaterial,      alpha_map_time), Rect_LU(prop_rect.ru()+Vec2(s+tex_size*1, i*prop_height), tex_size, tex_size), "Alpha"         , T); i-=3;
      sub+=texs.New().create(TEX_BUMP      , MEMBER(EditMaterial,       bump_map), MEMBER(EditMaterial,       bump_map_time), Rect_LU(prop_rect.ru()+Vec2(s           , i*prop_height), tex_size, tex_size), "Bump"          , T);
      sub+=texs.New().create(TEX_NORMAL    , MEMBER(EditMaterial,     normal_map), MEMBER(EditMaterial,     normal_map_time), Rect_LU(prop_rect.ru()+Vec2(s+tex_size*1, i*prop_height), tex_size, tex_size), "Normal"        , T); i-=3;
      sub+=texs.New().create(TEX_SMOOTH    , MEMBER(EditMaterial,     smooth_map), MEMBER(EditMaterial,     smooth_map_time), Rect_LU(prop_rect.ru()+Vec2(s           , i*prop_height), tex_size, tex_size), "Smooth"        , T);
      sub+=texs.New().create(TEX_METAL     , MEMBER(EditMaterial,      metal_map), MEMBER(EditMaterial,      metal_map_time), Rect_LU(prop_rect.ru()+Vec2(s+tex_size*1, i*prop_height), tex_size, tex_size), "Metal"         , T); i-=3;
      sub+=texs.New().create(TEX_GLOW      , MEMBER(EditMaterial,       glow_map), MEMBER(EditMaterial,       glow_map_time), Rect_LU(prop_rect.ru()+Vec2(s           , i*prop_height), tex_size, tex_size), "Glow"          , T);
      sub+=texs.New().create(TEX_EMISSIVE  , MEMBER(EditMaterial,   emissive_map), MEMBER(EditMaterial,   emissive_map_time), Rect_LU(prop_rect.ru()+Vec2(s+tex_size*1, i*prop_height), tex_size, tex_size), "Emit\nLight"   , T); i-=3;
      sub+=texs.New().create(TEX_DET_COLOR , MEMBER(EditMaterial,   detail_color), MEMBER(EditMaterial,     detail_map_time), Rect_LU(prop_rect.ru()+Vec2(s           , i*prop_height), tex_size, tex_size), "Detail\nColor" , T);
      sub+=texs.New().create(TEX_DET_SMOOTH, MEMBER(EditMaterial,  detail_smooth), MEMBER(EditMaterial,     detail_map_time), Rect_LU(prop_rect.ru()+Vec2(s+tex_size*1, i*prop_height), tex_size, tex_size), "Detail\nSmooth", T); i-=3;
      sub+=texs.New().create(TEX_DET_BUMP  , MEMBER(EditMaterial,    detail_bump), MEMBER(EditMaterial,     detail_map_time), Rect_LU(prop_rect.ru()+Vec2(s           , i*prop_height), tex_size, tex_size), "Detail\nBump"  , T);
      sub+=texs.New().create(TEX_DET_NORMAL, MEMBER(EditMaterial,  detail_normal), MEMBER(EditMaterial,     detail_map_time), Rect_LU(prop_rect.ru()+Vec2(s+tex_size*1, i*prop_height), tex_size, tex_size), "Detail\nNormal", T); i-=3;
      sub+=texs.New().create(TEX_MACRO     , MEMBER(EditMaterial,      macro_map), MEMBER(EditMaterial,      macro_map_time), Rect_LU(prop_rect.ru()+Vec2(s+tex_size*1, i*prop_height), tex_size, tex_size), "Macro"         , T);
    /*sub+=texs.New().create(TEX_RFL_ALL   , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(s-tex_size*3, i*prop_height), tex_size, tex_size), "Reflect\nAll"  , T);
      sub+=texs.New().create(TEX_RFL_L     , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(s-tex_size*2, i*prop_height), tex_size, tex_size), "Reflect\nLeft" , T);
      sub+=texs.New().create(TEX_RFL_F     , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(s-tex_size*1, i*prop_height), tex_size, tex_size), "Reflect\nFront", T);
      sub+=texs.New().create(TEX_RFL_R     , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(s+tex_size*0, i*prop_height), tex_size, tex_size), "Reflect\nRight", T);
      sub+=texs.New().create(TEX_RFL_B     , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(s+tex_size*1, i*prop_height), tex_size, tex_size), "Reflect\nBack" , T); i-=3;
      sub+=texs.New().create(TEX_RFL_D     , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(s+tex_size*0, i*prop_height), tex_size, tex_size), "Reflect\nDown" , T);
      sub+=texs.New().create(TEX_RFL_U     , MEMBER(EditMaterial, reflection_map), MEMBER(EditMaterial, reflection_map_time), Rect_LU(prop_rect.ru()+Vec2(s+tex_size*1, i*prop_height), tex_size, tex_size), "Reflect\nUp"   , T); i-=3; */
      REPA(texs)sub+=texs[i].remove;

      sub+=reload_base_textures.create("Reload Base Textures").func(ReloadBaseTextures, T).desc("Reload base textures, such as Color, Alpha, Bump, Normal, Smooth, Metal and Glow, from their original source files."); // #MaterialTextureLayout
      Node<MenuElm> n;
      n.New().create(auto_reload_name, AutoReload, T).flag(MENU_TOGGLABLE).setOn(auto_reload).desc("If this is enabled then base textures will be instantly reloaded when changing them.\nIf you only want to change the source file paths, without actually reloading the textures, then you can disable this option first.");
      {
         Node<MenuElm> &resize=(n+="Resize Base Textures"); resize.desc("This allows to resize the base textures, such as Color, Alpha, Bump, Normal, Smooth, Metal and Glow to a custom size."); // #MaterialTextureLayout
         resize.New().create( "128x128" , ResizeBase128 , T);
         resize.New().create( "256x256" , ResizeBase256 , T);
         resize.New().create( "512x512" , ResizeBase512 , T);
         resize.New().create("1024x1024", ResizeBase1024, T);
         resize.New().create("2048x2048", ResizeBase2048, T);
         resize.New().create("4096x4096", ResizeBase4096, T);
         Node<MenuElm> &other=(resize+="Other");
         
         other.New().create("128x64"   , ResizeBase128x64, T);
         other.New().create("256x128"  , ResizeBase256x128, T);
         other.New().create("512x256"  , ResizeBase512x256, T);
         other.New().create("1024x512" , ResizeBase1024x512, T);
         other.New().create("2048x1024", ResizeBase2048x1024, T);
         other++;
         other.New().create("64x128"   , ResizeBase64x128, T);
         other.New().create("128x256"  , ResizeBase128x256, T);
         other.New().create("256x512"  , ResizeBase256x512, T);
         other.New().create("512x1024" , ResizeBase512x1024, T);
         other.New().create("1024x2048", ResizeBase1024x2048, T);
         
         resize++;
         resize.New().create("Quarter" , ResizeBaseQuarter , T);
         resize.New().create("Half"    , ResizeBaseHalf    , T);
         resize.New().create("Original", ResizeBaseOriginal, T);
         resize.New().create("Double"  , ResizeBaseDouble  , T);
      }
      {
         Node<MenuElm> &resize=(n+=(water() ? "Resize Color Texture" : "Resize Color+Alpha Textures")); if(!water())resize.desc("This allows to resize the Base 0 textures, such as Color and Alpha to a custom size."); // #MaterialTextureLayout #MaterialTextureLayoutWater
         resize.New().create( "128x128" , ResizeBase0_128 , T);
         resize.New().create( "256x256" , ResizeBase0_256 , T);
         resize.New().create( "512x512" , ResizeBase0_512 , T);
         resize.New().create("1024x1024", ResizeBase0_1024, T);
         resize.New().create("2048x2048", ResizeBase0_2048, T);
         resize.New().create("4096x4096", ResizeBase0_4096, T);
         Node<MenuElm> &other=(resize+="Other");
         
         other.New().create("128x64"   , ResizeBase0_128x64, T);
         other.New().create("256x128"  , ResizeBase0_256x128, T);
         other.New().create("512x256"  , ResizeBase0_512x256, T);
         other.New().create("1024x512" , ResizeBase0_1024x512, T);
         other.New().create("2048x1024", ResizeBase0_2048x1024, T);
         other++;
         other.New().create("64x128"   , ResizeBase0_64x128, T);
         other.New().create("128x256"  , ResizeBase0_128x256, T);
         other.New().create("256x512"  , ResizeBase0_256x512, T);
         other.New().create("512x1024" , ResizeBase0_512x1024, T);
         other.New().create("1024x2048", ResizeBase0_1024x2048, T);
         
         resize++;
         resize.New().create("Quarter" , ResizeBase0_Quarter , T);
         resize.New().create("Half"    , ResizeBase0_Half    , T);
         resize.New().create("Original", ResizeBase0_Original, T);
         resize.New().create("Double"  , ResizeBase0_Double  , T);
      }
      {
         Node<MenuElm> &resize=(n+="Resize Normal Texture"); resize.desc("This allows to resize the Base 1 textures, such as Normal to a custom size."); // #MaterialTextureLayout #MaterialTextureLayoutWater
         resize.New().create( "128x128" , ResizeBase1_128 , T);
         resize.New().create( "256x256" , ResizeBase1_256 , T);
         resize.New().create( "512x512" , ResizeBase1_512 , T);
         resize.New().create("1024x1024", ResizeBase1_1024, T);
         resize.New().create("2048x2048", ResizeBase1_2048, T);
         resize.New().create("4096x4096", ResizeBase1_4096, T);
         Node<MenuElm> &other=(resize+="Other");
         
         other.New().create("128x64"   , ResizeBase1_128x64, T);
         other.New().create("256x128"  , ResizeBase1_256x128, T);
         other.New().create("512x256"  , ResizeBase1_512x256, T);
         other.New().create("1024x512" , ResizeBase1_1024x512, T);
         other.New().create("2048x1024", ResizeBase1_2048x1024, T);
         other++;
         other.New().create("64x128"   , ResizeBase1_64x128, T);
         other.New().create("128x256"  , ResizeBase1_128x256, T);
         other.New().create("256x512"  , ResizeBase1_256x512, T);
         other.New().create("512x1024" , ResizeBase1_512x1024, T);
         other.New().create("1024x2048", ResizeBase1_1024x2048, T);
         
         resize++;
         resize.New().create("Quarter" , ResizeBase1_Quarter , T);
         resize.New().create("Half"    , ResizeBase1_Half    , T);
         resize.New().create("Original", ResizeBase1_Original, T);
         resize.New().create("Double"  , ResizeBase1_Double  , T);
      }
      {
         Node<MenuElm> &resize=(n+=(water() ? "Resize Bump Texture" : "Resize Smooth+Metal+Bump+Glow Textures")); if(!water())resize.desc("This allows to resize the Base 2 textures, such as Smooth, Metal, Bump and Glow to a custom size."); // #MaterialTextureLayout #MaterialTextureLayoutWater
         resize.New().create( "128x128" , ResizeBase2_128 , T);
         resize.New().create( "256x256" , ResizeBase2_256 , T);
         resize.New().create( "512x512" , ResizeBase2_512 , T);
         resize.New().create("1024x1024", ResizeBase2_1024, T);
         resize.New().create("2048x2048", ResizeBase2_2048, T);
         resize.New().create("4096x4096", ResizeBase2_4096, T);
         Node<MenuElm> &other=(resize+="Other");
         
         other.New().create("128x64"   , ResizeBase2_128x64, T);
         other.New().create("256x128"  , ResizeBase2_256x128, T);
         other.New().create("512x256"  , ResizeBase2_512x256, T);
         other.New().create("1024x512" , ResizeBase2_1024x512, T);
         other.New().create("2048x1024", ResizeBase2_2048x1024, T);
         other++;
         other.New().create("64x128"   , ResizeBase2_64x128, T);
         other.New().create("128x256"  , ResizeBase2_128x256, T);
         other.New().create("256x512"  , ResizeBase2_256x512, T);
         other.New().create("512x1024" , ResizeBase2_512x1024, T);
         other.New().create("1024x2048", ResizeBase2_1024x2048, T);
         
         resize++;
         resize.New().create("Quarter" , ResizeBase2_Quarter , T);
         resize.New().create("Half"    , ResizeBase2_Half    , T);
         resize.New().create("Original", ResizeBase2_Original, T);
         resize.New().create("Double"  , ResizeBase2_Double  , T);
      }
      {
         Node<MenuElm> &bump=(n+="Set Bump from Color");
       //bump.New().create("Blur Auto", BumpFromCol  , T);
         bump.New().create("Blur 2"   , BumpFromCol2 , T);
         bump.New().create("Blur 3"   , BumpFromCol3 , T);
         bump.New().create("Blur 4"   , BumpFromCol4 , T);
         bump.New().create("Blur 5"   , BumpFromCol5 , T);
         bump.New().create("Blur 6"   , BumpFromCol6 , T);
         bump.New().create("Blur 8"   , BumpFromCol8 , T);
         bump.New().create("Blur 12"  , BumpFromCol12, T);
         bump.New().create("Blur 16"  , BumpFromCol16, T);
         bump.New().create("Blur 24"  , BumpFromCol24, T);
         bump.New().create("Blur 32"  , BumpFromCol32, T);
      }
      {
         Node<MenuElm> &extra=(n+="Extra");
         extra.New().create("Multiply Color Texture by Color Value"      , MulTexCol     , T);
         extra.New().create("Multiply Normal Texture by Normal Value"    , MulTexNormal  , T);
         extra.New().create("Multiply Smooth Texture by Smooth Value"    , MulTexSmooth  , T);
         extra.New().create("Multiply Glow Texture by Glow Value"        , MulTexGlow    , T);
         extra.New().create("Multiply Emissive Texture by Emissive Value", MulTexEmissive, T);
      }
      sub+=texture_options.create().setData(n); texture_options.flag|=COMBOBOX_CONST_TEXT;

      setBottom(prop_rect);

      preview_mesh[0].create(1).parts[0].base.create(Ball(0.42f      ), VTX_NRM|VTX_TAN|VTX_TEX0, 16);
      preview_mesh[1].create(1).parts[0].base.create(Box (0.42f/SQRT3), VTX_NRM|VTX_TAN|VTX_TEX0    );
      preview_mesh[2].create(1).parts[0].base.create(Tube(0.12f, 0.75f), VTX_NRM|VTX_TAN|VTX_TEX0, 16);
      preview_mesh[3].create(1).parts[0].base.createPlane(2, 2       , VTX_NRM|VTX_TAN|VTX_TEX0    ).transform(Matrix().setRotateX(PI_2).move(-0.5f, 0, -0.5f).scale(16)).texScale(16);
      REPAO(preview_mesh).setRender().setBox();
      preview_cam.setFromAt(Vec(0, 0, -1), VecZero);
   }
   Image* MaterialRegion::getDetailBump(C Str &file)
   {
      if(!EqualPath(file, detail_bump_file))
      {
         detail_bump_file=file;
         Image temp; Proj.loadImages(temp, null, detail_bump_file, true); // use sRGB because this is for preview
         temp.copyTry(detail_bump, Min(temp.w(), 128), Min(temp.h(), 128), 1, IMAGE_L8_SRGB, IMAGE_2D, 1, FILTER_LINEAR, IC_WRAP|IC_IGNORE_GAMMA); // we only need a preview, so make it small, no mip maps, and fast filtering, need to IC_IGNORE_GAMMA because 'loadImages' may lose it due to "channel" transform and here we need sRGB for preview
      }
      return detail_bump.is() ? &detail_bump : null;
   }
   void MaterialRegion::resize()
   {
          rect(Rect_RU(D.w(), D.h(), rect().w(), D.h()*2));
      sub.rect(Rect(0.01f, -rect().h(), rect().w(), preview.rect().min.y-0.01f));
      sub.virtualSize(&NoTemp(sub.childrenSize()+Vec2(0, 0.02f))); // add a bit of padding
      if(!sub.slidebar[1].visible())move(Vec2(slidebarSize(), 0));
      preview_big.rect(EditRect(false));
   }
   void MaterialRegion::toGui()
   {
      setSmoothParam(); // call this first, in case it affects 'toGui'
      REPAO(props).toGui();
      REPAO(texs ).toGui();
   }
           void MaterialRegion::flush(C UID &elm_id) {if(T.elm_id==elm_id)flush();}
   void MaterialRegion::flush()
   {
      if(elm && game && changed)
      {
         if(ElmMaterial *data=elm->mtrlData()){data->newVer(); data->from(edit);} // modify just before saving/sending in case we've received data from server after edit
         Save( edit, Proj.editPath(elm_id)); edit.copyTo(*game, Proj);
         Save(*game, Proj.gamePath(elm_id)); Proj.savedGame(*elm);
         Proj.mtrlSetAutoTanBin(elm->id);
         Server.setElmLong(elm->id);
         REPA(edit.tex_downsize)if(saved.tex_downsize[i]!=edit.tex_downsize[i])Proj.mtrlTexDownsize(elm_id, (TEX_SIZE_PLATFORM)i, edit.tex_downsize[i], saved.base_0_tex, saved.base_1_tex, saved.base_2_tex, true); // upon flushing set all materials with same textures to the same 'tex_downsize'
                                if(saved.tex_quality    !=edit.tex_quality    )Proj.mtrlTexQuality (elm_id,                       edit.tex_quality    , saved.base_0_tex, saved.base_1_tex, saved.base_2_tex, true); // upon flushing set all materials with same textures to the same 'tex_quality'
         saved=edit;
      }
      changed=false;
   }
   void MaterialRegion::setChanged()
   {
      if(elm && game)
      {
         changed=true;
         if(ElmMaterial *data=elm->mtrlData()){data->newVer(); data->from(edit);}
         edit.copyTo(*game, Proj);
      }
   }
   void MaterialRegion::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_MTRL)elm=null;
      if(T.elm!=elm)
      {
         flush();
         undos.del(); undoVis();
         if(elm)game=     Proj.gamePath( elm->id) ;else game=&temp;
         if(elm)edit.load(Proj.editPath(*elm   ));else edit.reset(); saved=edit;
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         toGui();
         Proj.refresh(false, false);
         if(!elm)
         {
            MaterialRegion &other=((this==&MtrlEdit) ? WaterMtrlEdit : MtrlEdit);
            hide(); if(other.elm)other.show();
         }
         ObjEdit.leaf.clear(); // clear attachments when changing material
      }
   }
   void MaterialRegion::activate(Elm *elm)
   {
      set(elm); if(T.elm)
      {
         MaterialRegion &other=((this==&MtrlEdit) ? WaterMtrlEdit : MtrlEdit);
         show(); other.hide();
      }
   }
   void MaterialRegion::toggle(Elm *elm)
   {
      if(elm==T.elm && visible())elm=null; activate(elm);
   }
           void            MaterialRegion::hideBig()         {if(bigVisible())big.push();}
   MaterialRegion& MaterialRegion::show(){if(hidden ()){super::show();                                           resize(); CodeEdit.resize();} return T;}
   MaterialRegion& MaterialRegion::hide(){if(visible()){super::hide(); preview_big.hide(); REPAO(props).close(); resize(); CodeEdit.resize();} return T;}
   void MaterialRegion::set(C MaterialPtr &mtrl) {activate(Proj.findElm(mtrl.id()));}
   void MaterialRegion::set(Memt<MaterialPtr> mtrls)
   {
      REPD(i, mtrls.elms())
      {
         if(!mtrls[i])mtrls.remove(i, true); // remove if null
         else         REPD(j, i)if(mtrls[i]==mtrls[j]){mtrls.remove(i, true); break;} // remove duplicates
      }
      FREPA(mtrls)if(mtrls[i]==game){set(mtrls[(i+1)%mtrls.elms()]); return;} // activate next
                     if(mtrls.elms())set(mtrls[0]); // activate first
   }
   void MaterialRegion::drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(contains(focus_obj))
      {
         FREPA(elms)if(Elm *elm=Proj.findElm(elms[i]))if(elm->type==elm_type)
         {
            activate(elm);
            break;
         }else
         if(elm->type==ELM_IMAGE)
         {
            REPA(texs)if(texs[i].contains(focus_obj))
            {
               texs[i].setFile(EncodeFileName(elm->id));
               break;
            }
            break;
         }
         elms.clear(); // processed
      }
   }
         ::MaterialRegion::ImageSource::TexChannel& MaterialRegion::ImageSource::TexChannel::set(TEX_CHANNEL_TYPE type) {T.type=type; T.pos=-1; return T;}
         ::MaterialRegion::ImageSource::TexChannel& MaterialRegion::ImageSource::TexChannel::find(C Str &name, C Str &text, bool case_sensitive, WHOLE_WORD whole_word) {if(pos<0)pos=TextPosI(name, text, case_sensitive, whole_word); return T;}
         void MaterialRegion::ImageSource::TexChannel::fix() {if(pos<0)pos=INT_MAX;}
         int MaterialRegion::ImageSource::TexChannel::Compare(C TexChannel &a, C TexChannel &b) {return ::Compare(a.pos, b.pos);}
      void MaterialRegion::ImageSource::set(C Str &name, int index) {T.name=name; T.index=index; REPAO(channel)=-1;}
      void MaterialRegion::ImageSource::process()
      {
         Str base=GetBaseNoExt(name);

         if(Contains(base, "color") || Contains(base, "albedo") || Contains(base, "diffuse") || Contains(base, "col", false, WHOLE_WORD_ALPHA) || Contains(base, "BC", true, WHOLE_WORD_ALPHA) || Contains(base, "alb", false, WHOLE_WORD_ALPHA) || Contains(base, "diff", false, WHOLE_WORD_ALPHA))channel[TC_COLOR]=0;else
         if(Contains(base, "alpha"))channel[TC_ALPHA]=0;else
         if(Contains(base, "normal") || Contains(base, "nrm", false, WHOLE_WORD_ALPHA) || Contains(base, "N", true, WHOLE_WORD_ALPHA))channel[TC_NORMAL]=0;else
         if(Contains(base, "emissive") || Contains(base, "emission") || Contains(base, "illum") || Contains(base, "glow") || Contains(base, "EMM", true, WHOLE_WORD_ALPHA) || Contains(base, "E", true, WHOLE_WORD_ALPHA))channel[TC_GLOW]=0;else
         if(Contains(base, "RMA", true, WHOLE_WORD_ALPHA))
         {
            channel[TC_ROUGH]=0;
            channel[TC_METAL]=1;
            channel[TC_AO   ]=2;
         }else
         if(Contains(base, "ORM", true, WHOLE_WORD_ALPHA) || Contains(base, "ARM", true, WHOLE_WORD_ALPHA))
         {
            channel[TC_AO   ]=0;
            channel[TC_ROUGH]=1;
            channel[TC_METAL]=2;
         }else
         if(Contains(base, "MRG", true, WHOLE_WORD_ALPHA))
         {
            channel[TC_METAL]=0;
            channel[TC_ROUGH]=1;
            channel[TC_GLOW ]=2;
         }else
         if(Contains(base, "mask", false, WHOLE_WORD_ALPHA) && Contains(base, "unity")) // Unity HDRP Mask R=metal, G=AO, B=Detail mask, A=smoothness
         {
            channel[TC_METAL ]=0;
            channel[TC_AO    ]=1;
            channel[TC_SMOOTH]=3;
         }else
         if(Contains(base, "metal") && ContainsAny(base, "smooth gloss")) // Unity RGB=metal, A=smoothness
         {
            channel[TC_METAL ]=0;
            channel[TC_SMOOTH]=3;
            need_metal_channel=false; // no need to specify channel for metal because it's in RGB slots
         }else
         if((Contains(base, "specular") || Contains(base, "spec", false, WHOLE_WORD_ALPHA)) && ContainsAny(base, "smooth gloss")) // Unity RGB=specular, A=smoothness
         {
            channel[TC_SPEC  ]=0;
            channel[TC_SMOOTH]=3;
         }else // auto-detect
         {
            TexChannel tc[TC_NUM];
            tc[0].set(TC_ROUGH ).find(base, "roughness" ).find(base, "rough").find(base, "R", true, WHOLE_WORD_ALPHA);
            tc[1].set(TC_SMOOTH).find(base, "smoothness").find(base, "smooth").find(base, "gloss");
            tc[2].set(TC_METAL ).find(base, "metalness" ).find(base, "metallic").find(base, "metal").find(base, "MT", true).find(base, "M", true, WHOLE_WORD_ALPHA);
            tc[3].set(TC_SPEC  ).find(base, "specular"  ).find(base, "spec", false, WHOLE_WORD_ALPHA);
            tc[4].set(TC_AO    ).find(base, "occlusion" ).find(base, "occl").find(base, "AO", true).find(base, "O", true, WHOLE_WORD_ALPHA).find(base, "ao", false, WHOLE_WORD_ALPHA).find(base, "cavity");
            tc[5].set(TC_BUMP  ).find(base, "height"    ).find(base, "bump");
            REPAO(tc).fix(); // fix for sorting, so unspecified channels are at the end
            Sort(tc, Elms(tc), TexChannel::Compare);
            REPA(tc)if(InRange(tc[i].pos, INT_MAX))channel[tc[i].type]=i;
         }

         if(channel[TC_AO   ]>=0)order=2;else // check AO first in case it's multi-channel because it's most popular
         if(channel[TC_GLOW ]>=0)order=3;else // then glow
         if(channel[TC_METAL]>=0)order=1;     // 

         REPA(channel)if(channel[i]>=0)detected_channels++; multi_channel=(detected_channels>1);
      }
      void MaterialRegion::ImageSource::process(TEX_TYPE tex_type, EditMaterial &mtrl, bool append, bool multi_images)
      {
         if(tex_type==TEX_COLOR)
         {
            if(InRange(channel[TC_AO], 4))
            {
               if(multi_channel)params.New().set("channel", IndexChannel(channel[TC_AO]));
               params.New().set("mode", "mulRGB");
               params.New().set("alpha", "0.5");
            }else
            if(InRange(channel[TC_GLOW], 4))
            {
               if(multi_channel)params.New().set("channel", IndexChannel(channel[TC_GLOW]));
               params.New().set("mode", "addRGB");
            }else
            if(InRange(channel[TC_METAL], 4))
            {
               if(multi_channel && need_metal_channel)params.New().set("channel", IndexChannel(channel[TC_METAL]));
               params.New().set("mode", "metal");
            }else
            if(Kb.shift())params.New().set("channel", "rgb"); // make shift to ignore alpha channel
         }else
         if(tex_type==TEX_SMOOTH)
         {
            if(InRange(channel[TC_AO], 4) // has AO
            && (append || (channel[TC_SMOOTH]<0 && channel[TC_ROUGH]<0))) // appending or don't have smooth/rough
            {
               if(multi_channel)params.New().set("channel", IndexChannel(channel[TC_AO]));
               params.New().set("mode", "mulRGB");
            }else // check Smooth/Rough
            {
               if(multi_channel)
               {
                  if(InRange(channel[TC_SMOOTH], 4))params.New().set("channel", IndexChannel(channel[TC_SMOOTH]));else
                  if(InRange(channel[TC_ROUGH ], 4))params.New().set("channel", IndexChannel(channel[TC_ROUGH ]));
               }
               if(multi_images)
               {
                  if(InRange(channel[TC_SMOOTH], 4) &&  mtrl.smooth_is_rough
                  || InRange(channel[TC_ROUGH ], 4) && !mtrl.smooth_is_rough)params.New().set("inverseRGB");
               }else // single image - adjust material 'smooth_is_rough'
               {
                  if(InRange(channel[TC_SMOOTH], 4)){mtrl.smooth_is_rough=false; mtrl.smooth_is_rough_time.getUTC();}else
                  if(InRange(channel[TC_ROUGH ], 4)){mtrl.smooth_is_rough=true ; mtrl.smooth_is_rough_time.getUTC();}else
                  if(Kb.shift()
                //&& (channel[TC_METAL]>=0 || channel[TC_SPEC]>=0)  // and has metal/specular, this check is optional
                  )params.New().set("channel", "a"); // make shift to use alpha channel (Unity RGB=metal/specular, A=smoothness)
               }
            }
         }else
         if(tex_type==TEX_METAL)
         {
            if(InRange(channel[TC_AO], 4) // has AO
            && (append || channel[TC_METAL]<0)) // appending or don't have metal
            {
               if(multi_channel)params.New().set("channel", IndexChannel(channel[TC_AO]));
               params.New().set("mode", "mulRGB");
            }else // check Metal
            if(multi_channel && InRange(channel[TC_METAL], 4) && need_metal_channel)params.New().set("channel", IndexChannel(channel[TC_METAL]));
         }else
         if(tex_type==TEX_GLOW)
         {
            if(multi_channel && InRange(channel[TC_GLOW], 4))params.New().set("channel", IndexChannel(channel[TC_GLOW]));
         }
      }
   int MaterialRegion::Compare(C ImageSource &a, C ImageSource &b)
   {
      if(int c=::Compare(a.order, b.order))return c;
      return   ::Compare(a.index, b.index);
   }
   void MaterialRegion::drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(contains(focus_obj))
      {
         undos.set(null, true); // set undo manually because we might change some parameters

         Texture *tex=null;
         REPA(texs)if(texs[i].contains(focus_obj)){tex=&texs[i]; break;}

         Memc<ImageSource> images; FREPA(names)if(ExtType(GetExt(names[i]))==EXT_IMAGE)images.New().set(CodeEdit.importPaths(names[i]), i);
         bool append=(Kb.ctrlCmd() && tex && tex->file.is()),
              multi_images=((images.elms()>1 || append) && tex); // multiple images

         if(multi_images
         || Kb.shift()
         || !tex
         ) // auto detect
         {
            REPAO(images).process();
            images.sort(Compare); // sort by order
            if(tex)
            {
               REPAO(images).process(tex->type, edit, append, multi_images);
            }else
            {
               Memc<ImageSource> tex_images[TEX_NUM];
               TEX_FLAG old_textures=edit.textures();
               bool has_color=false;
               FREPA(images) // process in order
               {
                C ImageSource &image=images[i];
                  if(image.channel[TC_COLOR ]>=0                               ){tex_images[TEX_COLOR ].add(image); has_color=true;}
                  if(image.channel[TC_ALPHA ]>=0                               ) tex_images[TEX_ALPHA ].add(image);
                  if(image.channel[TC_AO    ]>=0                               ) tex_images[TEX_COLOR ].add(image);
                  if(image.channel[TC_NORMAL]>=0                               ) tex_images[TEX_NORMAL].add(image);
                  if(image.channel[TC_ROUGH ]>=0 || image.channel[TC_SMOOTH]>=0) tex_images[TEX_SMOOTH].add(image);
                  if(image.channel[TC_METAL ]>=0                               ) tex_images[TEX_METAL ].add(image);
                  if(image.channel[TC_BUMP  ]>=0                               ) tex_images[TEX_BUMP  ].add(image);
                  if(image.channel[TC_GLOW  ]>=0                               ) tex_images[TEX_GLOW  ].add(image);
               }
               bool auto_reload=T.auto_reload; T.auto_reload=false; // disable auto-reload, so we can reload textures all at one time at the end
               TEX_FLAG changed=TEXF_NONE;
               REPA(tex_images)
               {
                  TEX_TYPE tex_type=(TEX_TYPE)i;
                  Memc<ImageSource> &tex_image=tex_images[tex_type]; if(tex_image.elms())
                  {
                     REPAO(tex_image).process(tex_type, edit);
                     REPA(texs)
                     {
                        Texture &tex=texs[i];
                        if(tex.type==tex_type)
                        {
                           bool append=false;
                           if(tex_type==TEX_COLOR && !has_color)append=true; // if we're applying to Color Slot (AO or Emissive) without Color Texture, then force append to apply on existing
                           Str file=FileParams::Encode(SCAST(Memc<FileParams>, tex_image));
                           tex.setFile(append ? FileParams::Merge(tex.file, file) : file, false); // we've already set undo
                           changed|=TEX_FLAG(1<<tex_type);
                           break;
                        }
                     }
                  }
               }
               T.auto_reload=auto_reload; // restore auto-reload
               if(changed)
               {
                  if(changed&TEXF_BASE    )rebuildBase    (old_textures, changed);
                  if(changed&TEXF_EMISSIVE)rebuildEmissive(old_textures);
               }
            }
         }
         if(tex)
         {
            Str file=FileParams::Encode(SCAST(Memc<FileParams>, images));
            tex->setFile(append ? FileParams::Merge(tex->file, file) : file, false); // we've already set undo
         }
      }
   }
   void MaterialRegion::rebuildBase(TEX_FLAG old_textures, uint changed_in_mtrl, bool adjust_params, bool always)
   {
      if(elm && game)
      {
         bool want_tan_bin=game->needTanBin();

         TEX_FLAG has_textures=TEXF_NONE, known_textures=TEXF_NONE;
         if(auto_reload || always)
         {
            known_textures|=TEXF_BASE; has_textures|=Proj.mtrlCreateBaseTextures(edit, changed_in_mtrl);
            Time.skipUpdate(); // compressing textures can be slow
         }
         if(adjust_params)edit.adjustParams(changed_in_mtrl, old_textures, has_textures, known_textures);

         setChanged();
         Proj.mtrlTexChanged();
         D.setShader(game());
         toGui();

         if(want_tan_bin!=game->needTanBin())Proj.mtrlSetAutoTanBin(elm->id);
      }
   }
   void MaterialRegion::rebuildDetail()
   {
      if(elm && game)
      {
         bool want_tan_bin=game->needTanBin();

         Proj.mtrlCreateDetailTexture(edit);
         setChanged();
         Proj.mtrlTexChanged();
         Time.skipUpdate(); // compressing textures can be slow

         D.setShader(game());
         toGui();

         if(want_tan_bin!=game->needTanBin())Proj.mtrlSetAutoTanBin(elm->id);
      }
   }
   void MaterialRegion::rebuildMacro()
   {
      if(elm && game)
      {
         Proj.mtrlCreateMacroTexture(edit);
         setChanged();
         Proj.mtrlTexChanged();
         Time.skipUpdate(); // compressing textures can be slow

         D.setShader(game());
         toGui();
      }
   }
   void MaterialRegion::rebuildEmissive(TEX_FLAG old_textures, bool adjust_params)
   {
      if(elm && game)
      {
         TEX_FLAG has_textures=TEXF_NONE, known_textures=TEXF_NONE;
         known_textures|=TEXF_EMISSIVE; has_textures|=Proj.mtrlCreateEmissiveTexture(edit);
         if(adjust_params)edit.adjustParams(0, old_textures, has_textures, known_textures);
         setChanged();
         Proj.mtrlTexChanged();
         Time.skipUpdate(); // compressing textures can be slow

         D.setShader(game());
         toGui();
      }
   }
   void MaterialRegion::elmChanged(C UID &mtrl_id)
   {
      if(elm && elm->id==mtrl_id)
      {
         undos.set(null, true);
         EditMaterial temp; if(temp.load(Proj.editPath(*elm)))if(edit.sync(temp)){edit.copyTo(*game, Proj); toGui();}
      }
   }
   void MaterialRegion::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
   bool MaterialRegion::winIOContains(GuiObj *go)C {REPA(texs)if(texs[i].win_io.contains(go))return true; return false;}
   void MaterialRegion::update(C GuiPC &gpc)
{
      super::update(gpc);
      preview_big.visible(bigVisible());
      if(Gui.ms()==&preview || Gui.ms()==&preview_big)
      {
         bool rot_cam=(Ms.b(0) || Ms.b(MS_BACK)), rot_light=Ms.b(1);
         preview_cam.transformByMouse(min_zoom, max_zoom, (rot_cam ? CAMH_ROT : 0)|CAMH_ZOOM);
         if(rot_light)light_angle+=Ms.d()*Vec2(-1,  1);
         if(rot_cam || rot_light)Ms.freeze();
         if(Ms.bd(0))big.push();
      }
      if(Ms.bp(2))
      {
         if(contains(Gui.ms()))set(null);else
         if(Gui.ms()==&preview_big)big.push();
      }
      REPA(Touches)if(Touches[i].guiObj()==&preview || Touches[i].guiObj()==&preview_big)if(Touches[i].on()){preview_cam.yaw-=Touches[i].ad().x*2.0f; preview_cam.pitch+=Touches[i].ad().y*2.0f; preview_cam.setSpherical();}
      if(visible() && Kb.k.k && (contains(Gui.kb()) || contains(Gui.ms()) || preview_big.contains(Gui.ms()) || winIOContains(Gui.ms())))
      {
         KbSc prev(KB_PGUP, KBSC_CTRL_CMD|KBSC_REPEAT),
              next(KB_PGDN, KBSC_CTRL_CMD|KBSC_REPEAT);
         if(prev.pd()){prev.eat(); Proj.elmNext(elm_id, -1);}else
         if(next.pd()){next.eat(); Proj.elmNext(elm_id    );}
      }
   }
MaterialRegion::MaterialRegion() : elm_type(ELM_MTRL), auto_reload(true), min_zoom(0.48f), max_zoom(3), mouse_edit_delta(0), mouse_edit_value(0), light_angle(PI_4), red(null), green(null), blue(null), alpha(null), emit_red(null), emit_green(null), emit_blue(null), smooth(null), game(&temp), elm_id(UIDZero), elm(null), changed(false), undos(true) {}

MaterialRegion::Texture::Texture() : mr(null) {}

MaterialRegion::ImageSource::ImageSource() : multi_channel(false), need_metal_channel(true), index(0), order(0), detected_channels(0) {}

MaterialRegion::ImageSource::TexChannel::TexChannel() : pos(-1) {}

/******************************************************************************/
