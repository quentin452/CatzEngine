/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
ProjectEx Proj;
/******************************************************************************/
ThreadSafeMap<UID, TextureInfo> TexInfos; // since Textures are generated from Hash, they will be the same for all projects, so we can keep it outside of 'Project' class
State StateProject(UpdateProject, DrawProject, InitProject, ShutProject);
/******************************************************************************/
int CompareProjPath(C UID &a, C UID &b) {return ComparePathNumber(Proj.elmFullName(a), Proj.elmFullName(b));}
/******************************************************************************/
void HideProject()
{
          Export.hide();
    ProjSettings.hide();
             MSM.hide();
             DST.hide();
     CreateMtrls.hide();
  ConvertToAtlas.hide();
ConvertToDeAtlas.hide();
       RenameElm.hide();
     ReplaceName.hide();
       ReloadElm.hide();
        NewWorld.hide(); WorldEdit.grid_plane_level_win.hide(); WorldEdit.goto_area.hide();
        CopyElms.hide(); CopyElms.replace_elms.hide();
       SizeStats.hide();
      PublishRes.hide();
    EraseRemoved.hide();
        ElmProps.hide();
       SplitAnim.hide();
      RenameSlot.hide();
      RenameBone.hide();
     RenameEvent.hide();
    SetMtrlColor.hide();
  MulSoundVolume.hide();
         Publish.export_data           .hide();
         ObjEdit.mesh_parts     .rename.hide();
         ObjEdit.mesh_variations.rename.hide();
      ImportCode.del ();
}
/******************************************************************************/
bool InitProject()
{
   SetKbExclusive();
   Mode.    show().disabled(false); // show this first before making 'Proj.visible' because first we need to show Code Editor and its menu, so when 'Proj.show' calls 'CodeEdit.resize' then it will set source code rect according to the menu rect
   Misc.    show().disabled(false);
   Proj.menu      .disabled(false);
   Proj.visible(!Misc.hide_proj());
   Theater.setVisibility(true);
   return true;
}
void ShutProject()
{
   Mode   .hide().disabled(true);
   Misc   .hide().disabled(true);
   Proj.menu     .disabled(true);
   Proj   .hide();
   Theater.setVisibility(false);
   TexDownsize.close(); // close to disable menu

   HideProject();
}
/******************************************************************************/
Elm* GuiObjToElm(GuiObj *go)
{
   if(go)
   {
      if(go==&      AnimEdit || go==&AnimEdit.preview)return AnimEdit.elm;
      if(go==&  PhysMtrlEdit)return   PhysMtrlEdit.elm;
      if(go==&   MiniMapEdit)return    MiniMapEdit.elm;
      if(go==&       EnvEdit)return        EnvEdit.elm;
      if(go==&      EnumEdit)return       EnumEdit.elm;
      if(go==&     ImageEdit)return      ImageEdit.elm;
      if(go==&ImageAtlasEdit)return ImageAtlasEdit.elm;
      if(go==&      IconEdit)return       IconEdit.elm;
      if(go==& IconSettsEdit)return  IconSettsEdit.elm;
      if(go==&      FontEdit)return       FontEdit.elm;
      if(go==& TextStyleEdit)return  TextStyleEdit.elm;
      if(go==&PanelImageEdit)return PanelImageEdit.elm;
      if(go==&     PanelEdit)return      PanelEdit.elm;
      if(go==&  ObjClassEdit)return   ObjClassEdit.elm;
      if(go==&     SoundEdit)return      SoundEdit.elm;
      if(go==&     VideoEdit)return      VideoEdit.elm;
      if(go==&  AppPropsEdit)return   AppPropsEdit.elm;
      if(go==&      MtrlEdit)return       MtrlEdit.elm;
      if(go==& WaterMtrlEdit)return  WaterMtrlEdit.elm;
      if(go==&       ObjEdit)return        ObjEdit.obj_elm;
      if(go==&     WorldEdit)return      WorldEdit.elm;
      if(go==&   GuiSkinEdit)return    GuiSkinEdit.elm;
      if(go==&       GuiEdit)return        GuiEdit.elm;
   }
   return null;
}
bool UpdateProject()
{
#if 0
   SPSet("CT", Kb.ctrlCmd());
   SPSet("SH", Kb.shift());
   SPSet("AL", Kb.alt());
   SPSet("WI", Kb.winCtrl());
   SPSet("SP", Kb.b(KB_SPACE));
   SPSet("TB", Kb.b(KB_TAB));
   SPSet("MX", Ms.pos().x);
   SPSet("MY", Ms.pos().y);
#endif
   CurTime.getUTC();
   Gui.update();

          Mode.resize();
       ObjEdit.resize();
      AnimEdit.resize();
     WorldEdit.resize();
   GuiSkinEdit.resize();
       GuiEdit.resize();
      CodeEdit.resize(false);
       Theater.resize();
   TexDownsize.resize();

   WorldEdit.updateCamCenter(); // update after gui so 'obj_pos' will work without delay
   Proj     .update();
   Server   .update(&Proj, false);
   Importer .update();
   Builder  .update(true);
   Water    .update(0.4f);
   D        .grassUpdate();
   Clouds   .layered.update();
   Physics  .stopSimulation().startSimulation(); // to visualize actors used in physical material editors

   // mouse maximize window
   if(Ms.bp(MS_MAXIMIZE))
      if(!ObjEdit.contains(Gui.ms()) && !AnimEdit.contains(Gui.ms()) && !WorldEdit.contains(Gui.ms()) && !TexDownsize.contains(Gui.ms()))App.window().toggle();

   // open element
   if(Kb.ctrlCmd() && Ms.bp(0) && Gui.ms() && Gui.ms()->type()==GO_TEXTLINE)
   {
      Memc<Str > splits; Split(splits, Gui.ms()->asTextLine()(), '\n');
      Memc<Elm*> elms  ; FREPA(splits)if(Elm *elm=Proj.findElm(splits[i]))elms.add(elm);
      switch(elms.elms())
      {
         case  0: break;
         case  1: Proj.elmToggle(elms[0]); break;
         default: // cycle between elements
         {
            int last=-1; // cycle is not perfect if both Window and Fullscreen editors are listed, because there's no way to detect which one was used the last time

            if(GuiObj *desktop=Gui.desktop())REP(desktop->childNum())if(GuiObj *go=desktop->child(i))if(go->type()==GO_WINDOW && go->visible())if(Elm *elm=GuiObjToElm(go)) // start from top most visible children
               REPA(elms)if(elms[i]==elm){last=i; goto found;}

            REPA(Mode.order)if(Mode.available[Mode.order[i]])if(Elm *elm=Mode.elm(Mode.order[i]))
               REPA(elms)if(elms[i]==elm){last=i; goto found;}

         found:
            Proj.elmActivate(elms[Mod(last+1, elms.elms())]);
         }break;
      }
   }

   D.viewRect(null); // restore full viewport because it could have been changed by 'Viewport4.update' or 'setViewportCamera'
   return true;
}
void DrawProject()
{
   IconEdit.makeIcon(); // call this first before any drawing

   // draw background
   if(Mode()<0             )D.clear(BackgroundColor());else
   if(Mode()==MODE_GUI     )GuiEdit.drawPre();else
   if(Mode()==MODE_GUI_SKIN)GuiSkinEdit.draw();else
   if(Mode()==MODE_CODE    )CodeEdit.CodeEditorInterface::draw();

   Gui.draw();
   Proj.draw();
   GuiEdit.draw();
   WorldEdit.higlight();
   //if(Importer.busy())D.text(0, D.h()-0.05, S+"Importing.. "+Importer.totalLeft());
   Draw();
}
/******************************************************************************/

/******************************************************************************/
   cchar8 *ProjectEx::explore_desc="Open folder containing the source file from which this element was created",
             *ProjectEx::dis_publish_desc="Disable publishing for this element.\nThis option can be useful if you want to disable element from being published without having to remove it.\nThis can be used for example on:\n-Images that are used to create an Image Atlas, however the Images themself are not used in the game.\n-Object which is used to create an Icon, however the Object itself is not used in the game.",
             * ProjectEx::en_publish_desc="Disable publishing for this element.\nThis option can be useful if you want to disable element from being published without having to remove it.\nThis can be used for example on:\n-Images that are used to create an Image Atlas, however the Images themself are not used in the game.\n-Object which is used to create an Icon, however the Object itself is not used in the game.",
             *     ProjectEx::reload_desc="Reload element from its original source";
/******************************************************************************/
      void ProjectEx::OuterRegion::setRegionWidth(flt w)
      {
         Clamp(w, Misc.size().x, Misc.size().x*3);
         if(!Equal(w, rect().w()))
         {
            size(Vec2(w, rect().h()));
            ScreenChanged();
         }
      }
      GuiObj* ProjectEx::OuterRegion::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)
{
         GuiObj *go=super::test(gpc, pos, mouse_wheel);
         if(visible())
         {
            Rect r=rect()+gpc.offset;
            r.min.x=r.max.x;
            r.max.x+=Min(0.011f, Gui.resize_radius); // make less than 'resize_radius' because many Object Editor window buttons are located 0.01 away from the border
            if(Cuts(pos, r&gpc.clip))go=this;
         }
         return go;
      }
      void ProjectEx::OuterRegion::update(C GuiPC &gpc)
{
         super::update(gpc);
         if(Gui.ms()==this)
         {
            if(!Ms.b(0)) // if not pressed then detect new resize, if pressed then keep previous
            {
               resize_on=false;
               Vec2 pos=Ms.pos()-gpc.offset;
               if(pos.x>=rect().max.x-D.pixelToScreenSize().x)resize_on=true;
            }else // button pressed
            if(resize_on) // resize
            {
               Vec2 pos=Ms.pos()-gpc.offset;
               setRegionWidth(pos.x-rect().min.x);
            }
         }else resize_on=false;
      }
      void ProjectEx::OuterRegion::draw()
      {
         if(resize_on && Gui.ms()==this)
            if(Image *image=Gui.image_resize_x())
         {
            Vec2 pos=Ms.pos(), size(MOUSE_IMAGE_SIZE*image->aspect(), MOUSE_IMAGE_SIZE); pos+=Vec2(-size.x, size.y)*0.5f;
            image->draw(Rect_LU(D.screenAlignedToPixel(pos), size));
         }
      }
      GuiObj& ProjectEx::OuterRegion::show(){if(hidden ()){super::show(); MtrlEdit.resize(); WaterMtrlEdit.resize(); CodeEdit.resize(); if(!Gui.window() && Mode()<0)Proj.list.kbSet();} return T;}
      GuiObj& ProjectEx::OuterRegion::hide(){if(visible()){super::hide(); MtrlEdit.resize(); WaterMtrlEdit.resize(); CodeEdit.resize(); Proj.updateMenu();                             } return T;}
         GuiObj* ProjectEx::ElmList::Warning::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel){return null;}
         void ProjectEx::ElmList::Warning::create(int data_abs_index, bool error, flt offset)
         {
            flt w=Proj.list.columnWidth(Proj.list.icon_col);
            super::create(Rect_U(offset, 0, w, w), error ? Proj.exclamation : Proj.warning);
            rect_color.zero();
            Proj.list.addChild(T, data_abs_index, Proj.list.icon_col); // add after everything is setup
         }
         void ProjectEx::ElmList::AppCheck::setRect()
         {
            flt x=Proj.region.rect().w()-Proj.region.slidebarSize()-D.pixelToScreenSize().x;
            rect(Rect_RU(x, 0, Proj.list.elmHeight(), Proj.list.elmHeight()));
         }
      ListElm* ProjectEx::ElmList::curToListElm(              )C {return List<ListElm>::operator()();}
      ListElm* ProjectEx::ElmList::litToListElm(              )C {return visToData(lit);}
      Elm*       ProjectEx::ElmList::operator()(              )C {if(ListElm *e=visToData(cur     ))return e->elm ; return null;}
      EEItem*     ProjectEx::ElmList::visToItem(  int visible )C {if(ListElm *e=visToData(visible ))return e->item; return null;}
      EEItem*     ProjectEx::ElmList::absToItem(  int absolute)C {if(ListElm *e=absToData(absolute))return e->item; return null;}
      Elm*        ProjectEx::ElmList::visToElm(  int visible )C {if(ListElm *e=visToData(visible ))return e->elm ; return null;}
      Elm*        ProjectEx::ElmList::absToElm(  int absolute)C {if(ListElm *e=absToData(absolute))return e->elm ; return null;}
      int        ProjectEx::ElmList::itemToVis(C EEItem *data)C {if(data)REP(visibleElms())if(visToItem(i)==data)return i; return -1;}
      int        ProjectEx::ElmList::itemToAbs(C EEItem *data)C {if(data)REP(  totalElms())if(absToItem(i)==data)return i; return -1;}
      int         ProjectEx::ElmList::elmToVis(C Elm    *data)C {if(data)REP(visibleElms())if(visToElm (i)==data)return i; return -1;}
      int         ProjectEx::ElmList::elmToAbs(C Elm    *data)C {if(data)REP(  totalElms())if(absToElm (i)==data)return i; return -1;}
      bool ProjectEx::ElmList::identitySort()C // if list is sorted in identity mode
      {
         FREPA(sort_column) // check in order
         {
            int col=sort_column[i];
            if( col<        0)continue; // unspecified = ignore
            if( col<=icon_col)return sort_swap[i]==false; // these columns sort in identity, so check if it's not swapped
                              return false; // other columns never sort in identity
         }
         return true; // if all are unspecified then it's OK
      }
      void ProjectEx::ElmList::update(C GuiPC &gpc)
{
         // process before update, because we're potentially eating a button
         if(tapped_vis>=0)
         {
            if(Time.appTime()>=tapped_time) // if enough time has passed to process it
            {
               if(tapped_open)
                  if(Elm *elm=visToElm(tapped_vis)){elm->opened(!elm->opened()); Proj.refresh(false, false);}
               tapped_vis=-1;
            }else
            if(!tapped_open)Ms.eat(0); // don't process any new presses
         }

         super::update(gpc);

         if(Gui.kb()==this || menu.contains(Gui.kb()))
         {
            KbSc rename(KB_F2), reload(KB_R, KBSC_CTRL_CMD), copyTo(KB_T, KBSC_CTRL_CMD), mulVol(KB_S, KBSC_CTRL_CMD|KBSC_ALT); // check manually, in case 'menu' is not created according to selection
            if(rename.pd())
            {
               rename.eat();
               if(Elm *elm=T())RenameElm.activate(elm);
            }else
            if(reload.pd())
            {
               reload.eat();
               Proj.setMenuListSel();
               Proj.reload(Proj.menu_list_sel);
            }else
            if(copyTo.pd())
            {
               copyTo.eat();
               CopyElms.display();
            }else
            if(mulVol.pd())
            {
               mulVol.eat();
               Proj.setMenuListSel();
               if(Proj.menu_list_sel.elms())MulSoundVolume.display(Proj.menu_list_sel);
            }else
            if(!Kb.k.ctrlCmd())
            {
               if(Kb.k(KB_RIGHT))
               {
                  if(ListElm *elm=curToListElm())
                  {
                     if(elm->hasVisibleChildren())
                     {
                        if(elm->item && !elm->item->opened  ){elm->item->opened=true ; goto right_done;}
                        if(elm->elm  && !elm->elm ->opened()){elm->elm ->opened(true); goto right_done;}
                        if(cur<elms()-1)setCur(cur+1);
                     right_done:
                        Proj.refresh(false, false);
                        scrollTo(cur+4); // scroll after refresh
                     }
                  }
               }else
               if(Kb.k(KB_LEFT))
               {
                  if(ListElm *elm=curToListElm())
                  {
                     if(!elm->close()) // try closing current element
                        for(int i=cur-1; i>=0; i--)if(ListElm *e=visToData(i))if(e->offset<elm->offset){setCur(i); break;} // find first parent
                     Proj.refresh(false, false);
                     scrollTo(cur-4); // scroll after refresh
                  }
               }else
               if(Kb.k(KB_ENTER))
               {
                  Kb.eatKey();
                  Proj.elmToggle(curToListElm());
               }/*else
               if(KbSc(KB_F4).pd()){if(Elm *elm=T())Proj.explore(*elm);}*/
            }
            menu.checkKeyboardShortcuts(); // check these before other menus are checked, but check after keys above, because menu shortcuts operate on 'menu_list_sel'
         }
         if(Gui.ms()==&sound_play)lit=elmToVis(Proj.findElm(sound_play.lit_id));
         UID lit_elm_id=UIDZero; if(ListElm *elm=litToListElm())if(elm->elm)lit_elm_id=elm->elm->id;
         if( lit_elm_id!=T.lit_elm_id)
         {
            T.lit_elm_id=lit_elm_id;
              ObjEdit.param_edit.setSkin();
            WorldEdit.param_edit.setSkin();
                    ObjClassEdit.setSkin();
         }
      }
      GuiObj& ProjectEx::ElmList::rect(C Rect &rect)
{
         super::rect(rect);
         REPAO(app_checks).setRect();
         return T;
      }
      void ProjectEx::ElmChange::swap(ptr user)
{
         // since we're swapping, then we need to adjust the 'type' so next change will be the opposite of this operation
         switch(type)
         {
            case PUBLISH   : set^=test   ; Proj._setElmPublish(elms, test , set  , false, false); break; // "set^=test" means toggle "disable/enable"
            case REMOVE    : type=RESTORE; Proj.       restore(elms,                      false); break;
            case RESTORE   : type=REMOVE ; Proj.       remove (elms,               false, false); break;
            case SET_PARENT:               Proj.  setElmParent(elm_parents, true); break;
            case SET_NAME  :               Proj.  setElmNames (elm_names  , true); break;
         }
      }
   void ProjectEx::NewFolder(ProjectEx &proj) {proj.newElm(ELM_FOLDER     );}
   void ProjectEx::NewMtrl(ProjectEx &proj) {proj.newElm(ELM_MTRL       );}
   void ProjectEx::NewWaterMtrl(ProjectEx &proj) {proj.newElm(ELM_WATER_MTRL );}
   void ProjectEx::NewPhysMtrl(ProjectEx &proj) {proj.newElm(ELM_PHYS_MTRL  );}
   void ProjectEx::NewAnim(ProjectEx &proj) {proj.newElm(ELM_ANIM       );}
   void ProjectEx::NewObject(ProjectEx &proj) {proj.newElm(ELM_OBJ        );}
   void ProjectEx::NewObjectClass(ProjectEx &proj) {proj.newElm(ELM_OBJ_CLASS  );}
   void ProjectEx::NewEnum(ProjectEx &proj) {proj.newElm(ELM_ENUM       );}
   void ProjectEx::NewImage(ProjectEx &proj) {proj.newElm(ELM_IMAGE      );}
   void ProjectEx::NewImageAtlas(ProjectEx &proj) {proj.newElm(ELM_IMAGE_ATLAS);}
   void ProjectEx::NewIconSetts(ProjectEx &proj) {proj.newElm(ELM_ICON_SETTS );}
   void ProjectEx::NewIcon(ProjectEx &proj) {proj.newElm(ELM_ICON       );}
   void ProjectEx::NewFont(ProjectEx &proj) {proj.newElm(ELM_FONT       );}
   void ProjectEx::NewPanelImage(ProjectEx &proj) {proj.newElm(ELM_PANEL_IMAGE);}
   void ProjectEx::NewTextStyle(ProjectEx &proj) {proj.newElm(ELM_TEXT_STYLE );}
   void ProjectEx::NewPanel(ProjectEx &proj) {proj.newElm(ELM_PANEL      );}
   void ProjectEx::NewGuiSkin(ProjectEx &proj) {proj.newElm(ELM_GUI_SKIN   );}
   void ProjectEx::NewGui(ProjectEx &proj) {proj.newElm(ELM_GUI        );}
   void ProjectEx::NewSound(ProjectEx &proj) {proj.newElm(ELM_SOUND      );}
   void ProjectEx::NewVideo(ProjectEx &proj) {proj.newElm(ELM_VIDEO      );}
   void ProjectEx::NewRawFile(ProjectEx &proj) {proj.newElm(ELM_FILE       );}
   void ProjectEx::NewLib(ProjectEx &proj) {proj.newElm(ELM_LIB        );}
   void ProjectEx::NewApp(ProjectEx &proj) {proj.newElm(ELM_APP        );}
   void ProjectEx::NewCode(ProjectEx &proj) {proj.newElm(ELM_CODE       );}
   void ProjectEx::NewEnv(ProjectEx &proj) {proj.newElm(ELM_ENV        );}
   void ProjectEx::NewMiniMap(ProjectEx &proj) {proj.newElm(ELM_MINI_MAP   );}
   void ProjectEx::NewWorld(ProjectEx &proj) {::NewWorld.display();}
   void ProjectEx::ElmRename(ProjectEx &proj) {if(proj.menu_list_sel.elms()==1)if(Elm *elm=proj.findElm(proj.menu_list_sel[0]))RenameElm.activate(elm->id, elm->name);}
   void ProjectEx::ReplaceName(ProjectEx &proj) {::ReplaceName.activate(proj.menu_list_sel);}
   void ProjectEx::Remove(ProjectEx &proj) {proj.list.kbSet(); proj.remove        (proj.menu_list_sel, true);}
   void ProjectEx::Restore(ProjectEx &proj) {proj.list.kbSet(); proj.restore       (proj.menu_list_sel);}
   void ProjectEx::Reload(ProjectEx &proj) {proj.reload      (proj.menu_list_sel);}
   void ProjectEx::CancelReload(ProjectEx &proj) {proj.cancelReload(proj.menu_list_sel);}
   void ProjectEx::SplitAnim(ProjectEx &proj) {if(proj.menu_list_sel.elms())::SplitAnim.activate(proj.menu_list_sel[0]);}
   void ProjectEx::UndoElmChange(ProjectEx &proj) {proj.elm_undos.undo();}
   void ProjectEx::RedoElmChange(ProjectEx &proj) {proj.elm_undos.redo();}
   void ProjectEx::Duplicate(ProjectEx &proj) {proj.list.kbSet(); proj.duplicate     (proj.menu_list_sel);}
   void ProjectEx::CopyTo(ProjectEx &proj) {CopyElms.display();}
   void ProjectEx::Expand(ProjectEx &proj) {proj.list.kbSet(); proj.expand        (proj.menu_list_sel, proj.menu_list_sel_item);}
   void ProjectEx::ExpandAll(ProjectEx &proj) {proj.list.kbSet(); proj.expandAll     (proj.menu_list_sel, proj.menu_list_sel_item);}
   void ProjectEx::Collapse(ProjectEx &proj) {proj.list.kbSet(); proj.collapse      (proj.menu_list_sel, proj.menu_list_sel_item);}
   void ProjectEx::ExploreElm(ProjectEx &proj) {proj.list.kbSet(); if(proj.menu_list_sel.elms()==1)if(Elm *elm=proj.findElm(proj.menu_list_sel[0]))proj.explore(*elm);}
   void ProjectEx::Export(ProjectEx &proj) {if(proj.menu_list_sel.elms())::Export.activate(proj.menu_list_sel[0]);}
   void ProjectEx::Properties(ProjectEx &proj) {if(proj.menu_list_sel.elms()==1)if(Elm *elm=proj.findElm(proj.menu_list_sel[0]))ElmProps.activate(*elm);}
   void ProjectEx::FilterChanged(ProjectEx &proj) {proj.refresh(false, false);}
   void ProjectEx::ShowRemoved(ProjectEx &proj) {proj.refresh(false, false);}
   void ProjectEx::ShowTheater(ProjectEx &proj) {Theater.setVisibility(true);}
   void ProjectEx::SoundPlay(ProjectEx &proj) {if(proj.sound.playing() && proj.list.sound_play.lit_id==proj.list.sound_play.play_id)proj.sound.close();else{proj.sound.close(); if(Elm *elm=proj.findElm(proj.list.sound_play.lit_id, ELM_SOUND)){proj.list.sound_play.play_id=elm->id; proj.sound.play(proj.gamePath(*elm));}}}
   void  ProjectEx::EnablePublish(ProjectEx &proj) {proj.list.kbSet(); proj.setElmPublish(proj.menu_list_sel, true , -1, false);}
   void ProjectEx::DisablePublish(ProjectEx &proj) {proj.list.kbSet(); proj.setElmPublish(proj.menu_list_sel, false, -1, true );}
   void  ProjectEx::EnablePublishMobile(ProjectEx &proj) {proj.list.kbSet(); proj.setElmPublish(proj.menu_list_sel, -1, true , false);}
   void ProjectEx::DisablePublishMobile(ProjectEx &proj) {proj.list.kbSet(); proj.setElmPublish(proj.menu_list_sel, -1, false, true );}
   void ProjectEx::ImageMipMapOn(ProjectEx &proj) {proj.imageMipMap(proj.menu_list_sel, true );}
   void ProjectEx::ImageMipMapOff(ProjectEx &proj) {proj.imageMipMap(proj.menu_list_sel, false);}
   void ProjectEx::ImageResize0(ProjectEx &proj) {proj.imageResize(proj.menu_list_sel, 0);}
   void ProjectEx::ImageResize16(ProjectEx &proj) {proj.imageResize(proj.menu_list_sel, 16);}
   void ProjectEx::ImageResize32(ProjectEx &proj) {proj.imageResize(proj.menu_list_sel, 32);}
   void ProjectEx::ImageResize64(ProjectEx &proj) {proj.imageResize(proj.menu_list_sel, 64);}
   void ProjectEx::ImageResize128(ProjectEx &proj) {proj.imageResize(proj.menu_list_sel, 128);}
   void ProjectEx::ImageResize256(ProjectEx &proj) {proj.imageResize(proj.menu_list_sel, 256);}
   void ProjectEx::ImageResize512(ProjectEx &proj) {proj.imageResize(proj.menu_list_sel, 512);}
   void ProjectEx::ImageResize1024(ProjectEx &proj) {proj.imageResize(proj.menu_list_sel, 1024);}
   void ProjectEx::ImageResize2048(ProjectEx &proj) {proj.imageResize(proj.menu_list_sel, 2048);}
   void ProjectEx::MtrlSetRGB1(ProjectEx &proj) {                proj.mtrlSetRGB         (proj.menu_list_sel, 1);}
   void ProjectEx::MtrlSetRGB(ProjectEx &proj) {                SetMtrlColor.display    (proj.menu_list_sel);}
   void ProjectEx::MtrlMulRGB(ProjectEx &proj) {                SetMtrlColor.display    (proj.menu_list_sel, true);}
   void ProjectEx::MtrlSetRGBCur(ProjectEx &proj) {if(MtrlEdit.elm)proj.mtrlSetRGB         (proj.menu_list_sel, MtrlEdit.edit.color_s.xyz                           );else Gui.msgBox(S, "There's no Material opened");}
   void ProjectEx::MtrlSetTechCur(ProjectEx &proj) {if(MtrlEdit.elm)proj.mtrlSetTech        (proj.menu_list_sel, MtrlEdit.edit.tech                                  );else Gui.msgBox(S, "There's no Material opened");}
   void ProjectEx::MtrlSetBumpCur(ProjectEx &proj) {if(MtrlEdit.elm)proj.mtrlSetBump        (proj.menu_list_sel, MtrlEdit.edit.bump                                  );else Gui.msgBox(S, "There's no Material opened");}
   void ProjectEx::MtrlSetNormalCur(ProjectEx &proj) {if(MtrlEdit.elm)proj.mtrlSetNormal      (proj.menu_list_sel, MtrlEdit.edit.normal                                );else Gui.msgBox(S, "There's no Material opened");}
   void ProjectEx::MtrlSetSmoothCur(ProjectEx &proj) {if(MtrlEdit.elm)proj.mtrlSetSmooth      (proj.menu_list_sel, MtrlEdit.edit.smooth                                );else Gui.msgBox(S, "There's no Material opened");}
   void ProjectEx::MtrlSetReflectCur(ProjectEx &proj) {if(MtrlEdit.elm)proj.mtrlSetReflect     (proj.menu_list_sel, MtrlEdit.edit.reflect_min, MtrlEdit.edit.reflect_max);else Gui.msgBox(S, "There's no Material opened");}
   void ProjectEx::MtrlSetGlowCur(ProjectEx &proj) {if(MtrlEdit.elm)proj.mtrlSetGlow        (proj.menu_list_sel, MtrlEdit.edit.glow                                  );else Gui.msgBox(S, "There's no Material opened");}
   void ProjectEx::MtrlResetAlpha(ProjectEx &proj) {                proj.mtrlResetAlpha     (proj.menu_list_sel);}
   void ProjectEx::MtrlCullOn(ProjectEx &proj) {                proj.mtrlCull           (proj.menu_list_sel, true );}
   void ProjectEx::MtrlCullOff(ProjectEx &proj) {                proj.mtrlCull           (proj.menu_list_sel, false);}
   void ProjectEx::MtrlFlipNrmYOn(ProjectEx &proj) {                proj.mtrlFlipNrmY       (proj.menu_list_sel, true );}
   void ProjectEx::MtrlFlipNrmYOff(ProjectEx &proj) {                proj.mtrlFlipNrmY       (proj.menu_list_sel, false);}
   void ProjectEx::MtrlReloadBaseTex(ProjectEx &proj) {                proj.mtrlReloadTextures (proj.menu_list_sel, true, false, false, false);}
   void ProjectEx::MtrlSetColorTexCur(ProjectEx &proj) {if(MtrlEdit.elm)proj.mtrlSetTexColor    (proj.menu_list_sel, MtrlEdit.edit. color_map                               );else Gui.msgBox(S, "There's no Material opened");}
   void ProjectEx::MtrlSetBumpTexCur(ProjectEx &proj) {if(MtrlEdit.elm)proj.mtrlSetTexBump     (proj.menu_list_sel, MtrlEdit.edit.  bump_map                               );else Gui.msgBox(S, "There's no Material opened");}
   void ProjectEx::MtrlSetNormalTexCur(ProjectEx &proj) {if(MtrlEdit.elm)proj.mtrlSetTexNormal   (proj.menu_list_sel, MtrlEdit.edit.normal_map, MtrlEdit.edit.flip_normal_y  );else Gui.msgBox(S, "There's no Material opened");}
   void ProjectEx::MtrlSetSmoothTexCur(ProjectEx &proj) {if(MtrlEdit.elm)proj.mtrlSetTexSmooth   (proj.menu_list_sel, MtrlEdit.edit.smooth_map, MtrlEdit.edit.smooth_is_rough);else Gui.msgBox(S, "There's no Material opened");}
   void ProjectEx::MtrlSetMetalTexCur(ProjectEx &proj) {if(MtrlEdit.elm)proj.mtrlSetTexMetal    (proj.menu_list_sel, MtrlEdit.edit. metal_map                               );else Gui.msgBox(S, "There's no Material opened");}
   void ProjectEx::MtrlSetGlowTexCur(ProjectEx &proj) {if(MtrlEdit.elm)proj.mtrlSetTexGlow     (proj.menu_list_sel, MtrlEdit.edit.  glow_map                               );else Gui.msgBox(S, "There's no Material opened");}
   void ProjectEx::MtrlMulTexCol(ProjectEx &proj) {                proj.mtrlMulTexCol     (proj.menu_list_sel);}
   void ProjectEx::MtrlMulTexNormal(ProjectEx &proj) {                proj.mtrlMulTexNormal  (proj.menu_list_sel);}
   void ProjectEx::MtrlMulTexSmooth(ProjectEx &proj) {                proj.mtrlMulTexSmooth  (proj.menu_list_sel);}
   void ProjectEx::MtrlMulTexGlow(ProjectEx &proj) {                proj.mtrlMulTexGlow    (proj.menu_list_sel);}
   void ProjectEx::MtrlMulTexEmissive(ProjectEx &proj) {                proj.mtrlMulTexEmissive(proj.menu_list_sel);}
   void ProjectEx::MtrlMoveToObj(ProjectEx &proj) {proj.mtrlMoveToObj      (proj.menu_list_sel);}
   void ProjectEx::MtrlMerge(ProjectEx &proj) {MSM             .display(proj.menu_list_sel);}
   void ProjectEx::MtrlConvertToAtlas(ProjectEx &proj) {ConvertToAtlas  .setElms(proj.menu_list_sel);}
   void ProjectEx::MtrlConvertToDeAtlas(ProjectEx &proj) {ConvertToDeAtlas.setElms(proj.menu_list_sel);}
   void ProjectEx::MtrlTexSizeMobileFull(ProjectEx &proj) {proj.mtrlTexDownsize(proj.menu_list_sel, TSP_MOBILE, 0);}
   void ProjectEx::MtrlTexSizeMobileHalf(ProjectEx &proj) {proj.mtrlTexDownsize(proj.menu_list_sel, TSP_MOBILE, 1);}
   void ProjectEx::MtrlTexSizeMobileQuarter(ProjectEx &proj) {proj.mtrlTexDownsize(proj.menu_list_sel, TSP_MOBILE, 2);}
   void ProjectEx::MtrlTexSizeMobileEighth(ProjectEx &proj) {proj.mtrlTexDownsize(proj.menu_list_sel, TSP_MOBILE, 3);}
   void ProjectEx::MtrlTexSizeSwitchFull(ProjectEx &proj) {proj.mtrlTexDownsize(proj.menu_list_sel, TSP_SWITCH, 0);}
   void ProjectEx::MtrlTexSizeSwitchHalf(ProjectEx &proj) {proj.mtrlTexDownsize(proj.menu_list_sel, TSP_SWITCH, 1);}
   void ProjectEx::MtrlTexSizeSwitchQuarter(ProjectEx &proj) {proj.mtrlTexDownsize(proj.menu_list_sel, TSP_SWITCH, 2);}
   void ProjectEx::MtrlTexSizeSwitchEighth(ProjectEx &proj) {proj.mtrlTexDownsize(proj.menu_list_sel, TSP_SWITCH, 3);}
   void ProjectEx::AnimClip(ProjectEx &proj) {proj.animClip  (proj.menu_list_sel);}
   void ProjectEx::AnimLinear(ProjectEx &proj) {proj.animLinear(proj.menu_list_sel, true );}
   void ProjectEx::AnimCubic(ProjectEx &proj) {proj.animLinear(proj.menu_list_sel, false);}
   void ProjectEx::AnimLoopOn(ProjectEx &proj) {proj.animLoop  (proj.menu_list_sel, true );}
   void ProjectEx::AnimLoopOff(ProjectEx &proj) {proj.animLoop  (proj.menu_list_sel, false);}
   void ProjectEx::SoundImportAs(ptr mode) {Proj.soundImportAs(Proj.menu_list_sel, SoundEditor::Import_as[intptr(mode)].codec, SoundEditor::Import_as[intptr(mode)].bit_rate);}
   void ProjectEx::SoundMulVolume(ProjectEx &proj) {MulSoundVolume.display(proj.menu_list_sel);}
   void ProjectEx::TransformSet(ProjectEx &proj) {proj.transformSet       (proj.menu_list_sel);}
   void ProjectEx::TransformApply(ProjectEx &proj) {proj.transformApply     (proj.menu_list_sel);}
   void ProjectEx::TransformBottom(ProjectEx &proj) {proj.transformBottom    (proj.menu_list_sel);}
   void ProjectEx::TransformBack(ProjectEx &proj) {proj.transformBack      (proj.menu_list_sel);}
   void ProjectEx::TransformCenter(ProjectEx &proj) {proj.transformCenter    (proj.menu_list_sel);}
   void ProjectEx::TransformCenterXZ(ProjectEx &proj) {proj.transformCenterXZ  (proj.menu_list_sel);}
   void ProjectEx::TransformRotYMinBox(ProjectEx &proj) {proj.transformRotYMinBox(proj.menu_list_sel);}
   void ProjectEx::MeshRemVtxTex1(ProjectEx &proj) {proj.removeMeshVtx         (proj.menu_list_sel, VTX_TEX1 );}
   void ProjectEx::MeshRemVtxTex2(ProjectEx &proj) {proj.removeMeshVtx         (proj.menu_list_sel, VTX_TEX2 );}
   void ProjectEx::MeshRemVtxTex3(ProjectEx &proj) {proj.removeMeshVtx         (proj.menu_list_sel, VTX_TEX3 );}
   void ProjectEx::MeshRemVtxTex123(ProjectEx &proj) {proj.removeMeshVtx         (proj.menu_list_sel, VTX_TEX1|VTX_TEX2|VTX_TEX3);}
   void ProjectEx::MeshRemVtxCol(ProjectEx &proj) {proj.removeMeshVtx         (proj.menu_list_sel, VTX_COLOR);}
   void ProjectEx::MeshRemVtxSkin(ProjectEx &proj) {proj.removeMeshVtx         (proj.menu_list_sel, VTX_SKIN );}
   void ProjectEx::MeshDisableLQLODs(ProjectEx &proj) {proj.disableLQLODs         (proj.menu_list_sel);}
   void ProjectEx::MeshMergeCoplanarFaces(ProjectEx &proj) {proj.meshMergeCoplanarFaces(proj.menu_list_sel);}
   void ProjectEx::SetBody(ProjectEx &proj) {proj.objSetBody            (proj.menu_list_sel, ObjEdit.mesh_elm ? ObjEdit.mesh_elm->id : UIDZero);}
   void ProjectEx::columnVisible(int column, bool visible)
   {
      flt col_width=list.columnWidth(column); // get column width before it's hidden, because after it's hidden, its width may be unavailable
      list.columnsHidden(!list.file_size);
      setListPadding();
      list.columnVisible(column, visible);
      flt region_width=rect().w();
      if(visible)outer_region.setRegionWidth(Max(region_width, list.columnOffset(column+1)+region.slidebarSize())); // make room to display entire column
      else       outer_region.setRegionWidth(    region_width- col_width                                         ); // decrease size by column width
      if(list.columnsHidden())list.sort(0, false); // if there are no columns visible then restore original sorting
   }
   void ProjectEx::ShowFileSize(ProjectEx &proj) {proj.showFileSize();}
          void ProjectEx::showFileSize()
   {
      list.file_size^=1;
      getFileSizes();
      columnVisible(list.size_col, list.file_size);
   }
   void ProjectEx::IncludeUnpublishedElmSize(ProjectEx &proj) {proj.includeUnpublishedElmSize(!proj.list.include_unpublished_elm_size);}
          void ProjectEx::includeUnpublishedElmSize(bool on)
   {
      if(list.include_unpublished_elm_size!=on)
      {
         list.include_unpublished_elm_size^=1;
         refresh(false, false);
         list_options.menu("Include Unpublished Element Size", on, QUIET);
      }
   }
   void ProjectEx::IncludeTextureSizeInObject(ProjectEx &proj) {proj.includeTextureSizeInObject(!proj.list.include_texture_size_in_object);}
          void ProjectEx::includeTextureSizeInObject(bool on)
   {
      if(list.include_texture_size_in_object!=on)
      {
         list.include_texture_size_in_object^=1;
         refresh(false, false);
         list_options.menu("Include Texture Size/In Objects", on, QUIET);
      }
   }
   void ProjectEx::checkIncludeChildrenSize()
   {
      list_options.menu("Include Children Size/Never"      , list.ics==ElmList::ICS_NEVER , QUIET);
      list_options.menu("Include Children Size/When Folded", list.ics==ElmList::ICS_FOLDED, QUIET);
      list_options.menu("Include Children Size/Always"     , list.ics==ElmList::ICS_ALWAYS, QUIET);
   }
   void ProjectEx::checkIncludeTextureSize()
   {
      list_options.menu("Include Texture Size/Element Only"   , list.its==ElmList::ITS_ELM    , QUIET);
      list_options.menu("Include Texture Size/Element+Texture", list.its==ElmList::ITS_ELM_TEX, QUIET);
      list_options.menu("Include Texture Size/Texture Only"   , list.its==ElmList::ITS_TEX    , QUIET);
   }
   void ProjectEx::IncludeChildrenSize(ptr                           ics) {Proj.includeChildrenSize(ElmList::INCLUDE_CHILDREN_SIZE(intptr(ics)));}
          void ProjectEx::includeChildrenSize(ElmList::INCLUDE_CHILDREN_SIZE ics)
   {
      if(InRange(ics, ElmList::ICS_NUM))
      {
         if(list.ics!=ics){list.ics=ics; refresh(false, false);}
         checkIncludeChildrenSize();
      }
   }
   void ProjectEx::IncludeTextureSize(ptr                          its) {Proj.includeTextureSize(ElmList::INCLUDE_TEXTURE_SIZE(intptr(its)));}
          void ProjectEx::includeTextureSize(ElmList::INCLUDE_TEXTURE_SIZE its)
   {
      if(InRange(its, ElmList::ITS_NUM))
      {
         if(list.its!=its){list.its=its; refresh(false, false);}
         checkIncludeTextureSize();
      }
   }
   void ProjectEx::ListOnlyFolder(ProjectEx &proj) {proj.list.show_only_folder^=1; proj.refresh(false, false);}
   void ProjectEx::ListOnlyObj(ProjectEx &proj) {proj.list.show_only_obj   ^=1; proj.refresh(false, false);}
   void ProjectEx::ListOnlyMtrl(ProjectEx &proj) {proj.list.show_only_mtrl  ^=1; proj.refresh(false, false);}
   void ProjectEx::ListOnlyAnim(ProjectEx &proj) {proj.list.show_only_anim  ^=1; proj.refresh(false, false);}
   void ProjectEx::ListOnlySound(ProjectEx &proj) {proj.list.show_only_sound ^=1; proj.refresh(false, false);}
   void ProjectEx::ListFlat(ProjectEx &proj) {proj.list.flat_want       ^=1; proj.refresh(false, false);}
   void ProjectEx::setListPadding()
   {
      list.padding.set(list.columnsHidden() ? list.elmHeight() : 0, region.rect().h()/2);
   }
 C ImagePtr& ProjectEx::elmIcon(ELM_TYPE type)
   {
      switch(type)
      {
         case ELM_NONE       : return ImageNull;
         case ELM_FOLDER     : return icon_folder;
         case ELM_OBJ        : return icon_obj;
         case ELM_OBJ_CLASS  : return icon_class;
         case ELM_MESH       : return icon_mesh;
         case ELM_MTRL       : return icon_mtrl;
         case ELM_WATER_MTRL : return icon_water_mtrl;
         case ELM_PHYS_MTRL  : return icon_phys_mtrl;
         case ELM_ANIM       : return icon_anim;
         case ELM_ENV        : return icon_env;
         case ELM_WORLD      : return icon_world;
         case ELM_MINI_MAP   : return icon_mini_map;
         case ELM_ENUM       : return icon_enum;
         case ELM_FONT       : return icon_font;
         case ELM_TEXT_STYLE : return icon_ts;
         case ELM_PANEL_IMAGE: return icon_panel_image;
         case ELM_PANEL      : return icon_panel;
         case ELM_GUI_SKIN   : return icon_gui_skin;
         case ELM_GUI        : return icon_gui;
         case ELM_IMAGE      : return icon_image;
         case ELM_IMAGE_ATLAS: return icon_image_atlas;
         case ELM_ICON_SETTS : return icon_icon_setts;
         case ELM_ICON       : return icon_icon;
         case ELM_SOUND      : return icon_sound;
         case ELM_VIDEO      : return icon_video;
         case ELM_LIB        : return icon_lib;
         case ELM_APP        : return icon_app;
         case ELM_CODE       : return icon_code;
         default             : return icon_file;
      }
   }
   bool ProjectEx::contains(C Elm &a, C Elm *b)C {return super::contains(a, b);}
   Enum* ProjectEx::getEnumFromName(C Str &enum_name)
   {
      REPA(existing_enums)if(Elm *elm=findElm(existing_enums[i]))if(elm->name==enum_name)return Enums(gamePath(elm->id));
      return null;
   }
 C UID& ProjectEx::curApp() // warning: this relies on 'existing_apps' being setup properly
   {
      if(existing_apps.binaryHas(app_id))return                    app_id; // if desired app exists
      if(list.app_checks.elms()         )return list.app_checks[0].app_id; // return first visible checkbox/app
      if(existing_apps  .elms()         )return existing_apps  [0];        // return any existing app
      return UIDZero; // no application
   }
   GuiSkinPtr ProjectEx::appGuiSkin()C {return gamePath(CodeEdit.appGuiSkin());}
   bool ProjectEx::includeTex(C UID &tex_id)
   {
      if(tex_id.valid())if(texs.binaryInclude(tex_id))
      {
         Importer.includeTex(tex_id);
         return true;
      }
      return false;
   }
   Elm* ProjectEx::getObjMeshElm(C UID &obj_id, bool send_to_server, bool set_gui)
   {
      Elm *mesh_elm=null;
      if(Elm *obj_elm=findElm(obj_id))
         if(ElmObj *obj_data=obj_elm->objData())
      {
         if(!obj_data->mesh_id.valid()) // it is not specified at the moment
         {
            if(set_gui)setListCurSel();
            UID mesh_id=obj_elm->id+ELM_OFFSET_MESH;
            mesh_elm=&getElm(mesh_id); if(mesh_elm->type && mesh_elm->type!=ELM_MESH)return null; // can't create because this ID is used by something else
            if(!mesh_elm->type) // new element
            {
               mesh_elm->setName(ElmNameMesh).setParent(obj_elm).type=ELM_MESH;
               if(ElmMesh *mesh_data=mesh_elm->meshData()){mesh_data->newData(); mesh_data->obj_id=obj_elm->id;}
               Mesh mesh; mesh.save(editPath(mesh_elm->id)); // save a dummy mesh
               makeGameVer(*mesh_elm); // save mesh first before the object
               if(send_to_server)Server.setElmFull(mesh_elm->id); // send to server
            }
            obj_data->newVer();
            obj_data->mesh_id=mesh_elm->id; // assign mesh to obj before calling 'setMeshPhys'
            makeGameVer(*obj_elm); // because we've assigned a mesh to this object, we need to resave this object too
            if(send_to_server)Server.setElmShort(obj_elm->id);
            if(set_gui)setList();

            // call at the end
            if(ObjEdit.obj_elm==obj_elm)ObjEdit.getMeshElm(); // make sure obj editor has assigned mesh element
            REPAO(WorldEdit.objs).setMeshPhys();
         }else mesh_elm=findElm(obj_data->mesh_id);
      }
      return mesh_elm;
   }
   Elm* ProjectEx::getObjSkelElm(C UID &obj_id, bool send_to_server, bool set_gui)
   {
      Elm *skel_elm=null;
      if(Elm *mesh_elm=getObjMeshElm(obj_id, send_to_server, set_gui)) // we need to have a mesh to have a skeleton
         if(ElmMesh *mesh_data=mesh_elm->meshData())
            if(Elm *obj_elm=findElm(obj_id, ELM_OBJ))
      {
         if(!mesh_data->skel_id.valid()) // it is not specified at the moment
         {
            if(set_gui)setListCurSel();
            UID skel_id=obj_elm->id+ELM_OFFSET_SKEL;
            skel_elm=&getElm(skel_id); if(skel_elm->type && skel_elm->type!=ELM_SKEL)return null; // can't create because this ID is used by something else
            if(!skel_elm->type) // new element
            {
               skel_elm->setName(ElmNameSkel).setParent(mesh_elm).type=ELM_SKEL;
               if(ElmSkel *skel_data=skel_elm->skelData()){skel_data->newData(); skel_data->mesh_id=mesh_elm->id; skel_data->transform=mesh_data->transform;}
               Skeleton skel; skel.save(gamePath(skel_elm->id)); // save a dummy skel
               if(send_to_server)Server.setElmFull(skel_elm->id);
            }
            mesh_data->newVer();
            mesh_data->skel_id=skel_elm->id; // assign skel to mesh
            makeGameVer(*mesh_elm); // because we've assigned a skel to this mesh, we need to resave this mesh too
            if(send_to_server)Server.setElmShort(mesh_elm->id);
            if(set_gui)setList();

            // call at the end
            if(ObjEdit.obj_elm==obj_elm)ObjEdit.getSkelElm(); // make sure obj editor has assigned skel element
         }else skel_elm=findElm(mesh_data->skel_id);
      }
      return skel_elm;
   }
   Elm* ProjectEx::getObjPhysElm(C UID &obj_id, bool send_to_server, bool set_gui)
   {
      Elm *phys_elm=null;
      if(Elm *mesh_elm=getObjMeshElm(obj_id, send_to_server, set_gui)) // we need to have a mesh to have a phys body
         if(ElmMesh *mesh_data=mesh_elm->meshData())
            if(Elm *obj_elm=findElm(obj_id, ELM_OBJ))
      {
         if(!mesh_data->phys_id.valid()) // it is not specified at the moment
         {
            if(set_gui)setListCurSel();
            UID phys_id=obj_elm->id+ELM_OFFSET_PHYS;
            phys_elm=&getElm(phys_id); if(phys_elm->type && phys_elm->type!=ELM_PHYS)return null; // can't create because this ID is used by something else
            if(!phys_elm->type) // new element
            {
               phys_elm->setName(ElmNamePhys).setParent(mesh_elm).type=ELM_PHYS;
               if(ElmPhys *phys_data=phys_elm->physData()){phys_data->newData(); phys_data->mesh_id=mesh_elm->id; phys_data->transform=mesh_data->transform;}
               PhysBody phys; phys.save(gamePath(phys_elm->id)); // save a dummy phys body
               if(send_to_server)Server.setElmFull(phys_elm->id);
            }
            mesh_data->newVer();
            mesh_data->phys_id=phys_elm->id; // assign phys to mesh
            makeGameVer(*obj_elm); // because we've assigned a phys to this object, we need to resave this object too
            if(send_to_server)Server.setElmShort(mesh_elm->id);
            if(set_gui)setList();

            // call at the end
            if(ObjEdit.obj_elm==obj_elm)ObjEdit.getPhysElm(); // make sure obj editor has assigned phys element
            REPAO(WorldEdit.objs).setMeshPhys();
         }else phys_elm=findElm(mesh_data->phys_id);
      }
      return phys_elm;
   }
   void ProjectEx::setNewElm(Node<MenuElm> &n, C Str &prefix)
   {
      n.New().create(prefix+ElmTypeName[ELM_FOLDER]     , NewFolder     , T).desc(ElmTypeDesc[ELM_FOLDER]);
      n.New().create(prefix+ElmTypeName[ELM_ENUM]       , NewEnum       , T).desc(ElmTypeDesc[ELM_ENUM]);
      n.New().create(prefix+ElmTypeName[ELM_IMAGE]      , NewImage      , T).desc(ElmTypeDesc[ELM_IMAGE]);
      n.New().create(prefix+ElmTypeName[ELM_IMAGE_ATLAS], NewImageAtlas , T).desc(ElmTypeDesc[ELM_IMAGE_ATLAS]);
      n.New().create(prefix+ElmTypeName[ELM_FONT]       , NewFont       , T).desc(ElmTypeDesc[ELM_FONT]);
      n.New().create(prefix+ElmTypeName[ELM_TEXT_STYLE] , NewTextStyle  , T).desc(ElmTypeDesc[ELM_TEXT_STYLE]);
      n.New().create(prefix+ElmTypeName[ELM_PANEL_IMAGE], NewPanelImage , T).desc(ElmTypeDesc[ELM_PANEL_IMAGE]);
      n.New().create(prefix+ElmTypeName[ELM_PANEL]      , NewPanel      , T).desc(ElmTypeDesc[ELM_PANEL]);
      n.New().create(prefix+ElmTypeName[ELM_GUI_SKIN]   , NewGuiSkin    , T).desc(ElmTypeDesc[ELM_GUI_SKIN]);
      n.New().create(prefix+ElmTypeName[ELM_GUI]        , NewGui        , T).desc(ElmTypeDesc[ELM_GUI]);
      n.New().create(prefix+ElmTypeName[ELM_MTRL]       , NewMtrl       , T).desc(ElmTypeDesc[ELM_MTRL]);
      n.New().create(prefix+ElmTypeName[ELM_WATER_MTRL] , NewWaterMtrl  , T).desc(ElmTypeDesc[ELM_WATER_MTRL]);
      n.New().create(prefix+ElmTypeName[ELM_PHYS_MTRL]  , NewPhysMtrl   , T).desc(ElmTypeDesc[ELM_PHYS_MTRL]);
      n.New().create(prefix+ElmTypeName[ELM_ANIM]       , NewAnim       , T).desc(ElmTypeDesc[ELM_ANIM]);
      n.New().create(prefix+ElmTypeName[ELM_OBJ_CLASS]  , NewObjectClass, T).desc(ElmTypeDesc[ELM_OBJ_CLASS]);
      n.New().create(prefix+ElmTypeName[ELM_OBJ]        , NewObject     , T).desc(ElmTypeDesc[ELM_OBJ]);
      n.New().create(prefix+ElmTypeName[ELM_ICON_SETTS] , NewIconSetts  , T).desc(ElmTypeDesc[ELM_ICON_SETTS]);
      n.New().create(prefix+ElmTypeName[ELM_ICON]       , NewIcon       , T).desc(ElmTypeDesc[ELM_ICON]);
      n.New().create(prefix+ElmTypeName[ELM_ENV]        , NewEnv        , T).desc(ElmTypeDesc[ELM_ENV]);
      n.New().create(prefix+ElmTypeName[ELM_WORLD]      , NewWorld      , T).desc(ElmTypeDesc[ELM_WORLD]);
      n.New().create(prefix+ElmTypeName[ELM_MINI_MAP]   , NewMiniMap    , T).desc(ElmTypeDesc[ELM_MINI_MAP]);
      n.New().create(prefix+ElmTypeName[ELM_SOUND]      , NewSound      , T).desc(ElmTypeDesc[ELM_SOUND]);
      n.New().create(prefix+ElmTypeName[ELM_VIDEO]      , NewVideo      , T).desc(ElmTypeDesc[ELM_VIDEO]);
      n.New().create(prefix+ElmTypeName[ELM_FILE]       , NewRawFile    , T).desc(ElmTypeDesc[ELM_FILE]);
      n.New().create(prefix+ElmTypeName[ELM_LIB]        , NewLib        , T).desc(ElmTypeDesc[ELM_LIB]);
      n.New().create(prefix+ElmTypeName[ELM_APP]        , NewApp        , T).desc(ElmTypeDesc[ELM_APP]);
      n.New().create(prefix+ElmTypeName[ELM_CODE]       , NewCode       , T).desc(ElmTypeDesc[ELM_CODE]);
   }
   void ProjectEx::disableMenu(Menu &menu)
   {
      if(menu.hidden())Gui-=menu; // if the menu got hidden then unlink it from Gui, so its keyboard shortcuts won't be used
   }
   void ProjectEx::updateMenu()
   {
      disableMenu(list.menu);
   }
   void ProjectEx::create()
   {
      Gui+=outer_region.create().hide();

      {
         Node<MenuElm> n;
         n.New().create("Show File Size", ShowFileSize, T).flag(MENU_TOGGLABLE);
         Node<MenuElm> &children=(n+="Include Children Size");
         children.New().create("Never"      , IncludeChildrenSize, ptr(ElmList::ICS_NEVER )).flag(MENU_TOGGLABLE);
         children.New().create("When Folded", IncludeChildrenSize, ptr(ElmList::ICS_FOLDED)).flag(MENU_TOGGLABLE);
         children.New().create("Always"     , IncludeChildrenSize, ptr(ElmList::ICS_ALWAYS)).flag(MENU_TOGGLABLE);
         Node<MenuElm> &texture=(n+="Include Texture Size");
         texture.New().create("Element Only"   , IncludeTextureSize, ptr(ElmList::ITS_ELM    )).flag(MENU_TOGGLABLE);
         texture.New().create("Element+Texture", IncludeTextureSize, ptr(ElmList::ITS_ELM_TEX)).flag(MENU_TOGGLABLE);
         texture.New().create("Texture Only"   , IncludeTextureSize, ptr(ElmList::ITS_TEX    )).flag(MENU_TOGGLABLE);
         texture++;
         texture.New().create("In Objects", IncludeTextureSizeInObject, T).flag(MENU_TOGGLABLE).desc("Normally Texture File Sizes are included only in Material Elements.\nThis option will include Texture File Sizes additionally in Objects that reference them.");
         n.New().create("Include Unpublished Element Size", IncludeUnpublishedElmSize, T).flag(MENU_TOGGLABLE).desc("If include file size of elements that have Disabled Publishing");
         n++;
         n.New().create("List Only Folders"   , ListOnlyFolder, T).flag(MENU_TOGGLABLE);
         n.New().create("List Only Objects"   , ListOnlyObj   , T).flag(MENU_TOGGLABLE);
         n.New().create("List Only Materials" , ListOnlyMtrl  , T).flag(MENU_TOGGLABLE);
         n.New().create("List Only Animations", ListOnlyAnim  , T).flag(MENU_TOGGLABLE);
         n.New().create("List Only Sounds"    , ListOnlySound , T).flag(MENU_TOGGLABLE);
         n.New().create("List Flat"           , ListFlat      , T).flag(MENU_TOGGLABLE);
         n++;
         Node<MenuElm> &New=(n+="New"); setNewElm(New);
         outer_region+=list_options.create().setData(n).focusable(false); list_options.flag|=COMBOBOX_CONST_TEXT;
         checkIncludeChildrenSize();
         checkIncludeTextureSize();
      }

      outer_region+=show_removed.create().func(ShowRemoved, T).focusable(false).desc("Show removed elements\nKeyboard Shortcut: Alt+R"); show_removed.image="Gui/Misc/trash.img"; show_removed.mode=BUTTON_TOGGLE;
      outer_region+=theater     .create().setImage("Gui/Misc/theater.img").func(ShowTheater, T).focusable(false).desc("Enable Theater mode allowing to preview multiple elements at the same time\nKeyboard Shortcut: Alt+4"); theater.mode=BUTTON_TOGGLE;
      outer_region+=filter.create().func(FilterChanged, T).desc(S+"Find element\nType element name or its ID to find it.\n\nKeyboard Shortcut: "+Kb.ctrlCmdName()+"+Shift+F"); filter.reset.show(); filter.show_find=true; filter.hint=S+"Find ("+KbSc(KB_F, KBSC_CTRL_CMD|KBSC_SHIFT).asText()+')';
      outer_region+=region.create();

      ListColumn lc[]=
      {
         ListColumn(MEMBER(ListElm, opened_icon), 0       , S     ), // 0 "Opened"
         ListColumn(MEMBER(ListElm, icon       ), 0       , S     ), // 1 "Icon"
         ListColumn(MEMBER(ListElm, name       ), LCW_DATA, "Name"), // 2 Name
         ListColumn(       ListElm:: Size        , 0.2f     , "Size"), // 3 File Size, don't use LCW_DATA because that could be slow because ListElm tex size calculation is slow
      }; list.icon_col=1; list.name_col=2; list.size_col=3;
      lc[0].md.setCompareFunc(ListElm::CompareIndex);
      lc[1].md.setCompareFunc(ListElm::CompareIndex);
      lc[2].md.setCompareFunc(ListElm::CompareName );
      lc[3].md.setCompareFunc(ListElm::CompareSize );
      region+=list.create(lc, Elms(lc), true);
      for(int i=list.name_col+1; i<list.columns(); i++)list.columnVisible(i, false);
      list.flag|=LIST_MULTI_SEL; list.draw_column=-1;
      list.setElmOffset(MEMBER(ListElm, offset)).setElmTextColor(MEMBER(ListElm, color)).setElmDesc(MEMBER(ListElm, desc));

      list.sound_play.create().func(SoundPlay, T).focusable(false).skin=&EmptyGuiSkin;

      icon_folder     ="Gui/Elm/folder.img";
      icon_file       ="Gui/Elm/file.img";
      icon_obj        ="Gui/Elm/mesh.img";
      icon_class      ="Gui/Elm/class.img";
      icon_mesh       ="Gui/Elm/mesh.img";
      icon_mtrl       ="Gui/Elm/material.img";
      icon_water_mtrl ="Gui/Elm/water_material.img";
      icon_phys_mtrl  ="Gui/Elm/physics_material.img";
      icon_anim       ="Gui/Elm/animation.img";
      icon_env        ="Gui/Elm/environment.img";
      icon_world      ="Gui/Elm/world.img";
      icon_mini_map   ="Gui/Elm/mini_map.img";
      icon_enum       ="Gui/Elm/enum.img";
      icon_image      ="Gui/Elm/image.img";
      icon_image_atlas="Gui/Elm/image_atlas.img";
      icon_icon_setts ="Gui/Elm/icon_settings.img";
      icon_icon       ="Gui/Elm/icon.img";
      icon_font       ="Gui/Elm/font.img";
      icon_ts         ="Gui/Elm/text.img";
      icon_panel_image="Gui/Elm/panel_image.img";
      icon_panel      ="Gui/Elm/panel.img";
      icon_gui_skin   ="Gui/Elm/gui_skin.img";
      icon_gui        ="Gui/Elm/gui.img";
      icon_sound      ="Gui/Elm/sound.img";
      icon_video      ="Gui/Elm/video.img";
      icon_lib        ="Gui/Elm/library.img";
      icon_app        ="Gui/Elm/application.img";
      icon_code       ="Gui/Elm/file.img";
      icon_play       ="Gui/arrow_right_big.img";
      icon_stop       ="Gui/Misc/stop.img";
      arrow_right     ="Gui/arrow_right_big.img";
      arrow_down      ="Gui/arrow_down_big.img";
      warning         ="Gui/Misc/warning.img";
      exclamation     ="Gui/Misc/exclamation.img";
      icon_light      ="Gui/Misc/light.img";
      icon_particles  ="Gui/Misc/particles.img";

      // menu
      Node<MenuElm> menu;
        ObjEdit.setMenu(menu, "OE ");
       AnimEdit.setMenu(menu, "AE ");
      WorldEdit.setMenu(menu, "WE ");
      Gui+=T.menu.create(menu).hide().disabled(true);
   }
   void ProjectEx::explore(Elm &elm)
   {
      Explore((elm.type==ELM_APP) ? CodeEdit.appPath(elm.name) : elmSrcFileFirst(&elm));
   }
   Str ProjectEx::newElmName(ELM_TYPE type, C UID &parent_id)
   {
      Elm     *parent  =findElm(parent_id);
      int      parent_i=elms.validIndex(parent);
      ElmNode &parent_n=(InRange(parent_i, hierarchy) ? hierarchy[parent_i] : root);
      FREP(1000)
      {
         Str  name  =ElmTypeName[type]; if(i)name.space()+=i;
         bool exists=false; FREPA(parent_n.children)if(name==elms[parent_n.children[i]].name){exists=true; break;}
         if( !exists)return name;
      }
      return S;
   }
   void ProjectEx::newElm(ELM_TYPE type)
   {
      setMenuListSel();
      newElm(type, menu_list_sel.elms() ? menu_list_sel[0] : UIDZero);
   }
   Elm* ProjectEx::newElm(ELM_TYPE type, C UID &parent_id, C Str *name, bool refresh_elm_list)
   {
      if(InRange(type, ELM_NUM) && type!=ELM_NONE && type!=ELM_MESH && type!=ELM_SKEL && type!=ELM_PHYS && type!=ELM_SHADER && type!=ELM_WORLD)
      {
         if(refresh_elm_list)setListCurSel();
         Str  elm_name=(name ? *name : newElmName(type, parent_id));
         Elm *parent=findElm(parent_id); if(parent)parent->opened(true);
         Elm &elm=super::newElm(elm_name, parent_id, type);
         switch(type)
         {
            case ELM_CODE:
            {
               Str code; SaveCode(code, codePath(elm));
               if(ElmCode *data=elm.codeData()){data->newData(); data->from(code);}
               Server.setElmFull(elm.id);
            }break;

            case ELM_MTRL:
            {
               EditMaterial edit; edit.newData(Proj); Save(edit, editPath(elm));
               if(ElmMaterial *data=elm.mtrlData()){data->newData(); data->from(edit);}
               makeGameVer(elm);
               Server.setElmFull(elm.id);
            }break;

            case ELM_WATER_MTRL:
            {
               EditWaterMtrl edit; edit.newData(Proj); Save(edit, editPath(elm));
               if(ElmWaterMtrl *data=elm.waterMtrlData()){data->newData(); data->from(edit);}
               makeGameVer(elm);
               Server.setElmFull(elm.id);
            }break;

            case ELM_PHYS_MTRL:
            {
               EditPhysMtrl edit; edit.newData(); Save(edit, editPath(elm));
               if(ElmPhysMtrl *data=elm.physMtrlData()){data->newData(); data->from(edit);}
               makeGameVer(elm);
               Server.setElmFull(elm.id);
            }break;

            case ELM_FONT:
            {
               EditFont edit; edit.newData(); Save(edit, editPath(elm));
               if(ElmFont *data=elm.fontData()){data->newData(); data->from(edit);}
               elmReload(elm.id, false, false);
               Server.setElmFull(elm.id);
            }break;

            case ELM_PANEL_IMAGE:
            {
               EditPanelImage edit; edit.newData(    ); Save(edit, editPath(elm));
                   PanelImage game; edit.make(game, T); Save(game, gamePath(elm)); savedGame(elm); // makeGameVer(elm);
               if(ElmPanelImage *data=elm.panelImageData()){data->newData(); data->from(edit);}
               Server.setElmFull(elm.id);
            }break;

            case ELM_PANEL:
            {
               EditPanel edit; edit.newData(); Save(edit, editPath(elm));
               if(ElmPanel *data=elm.panelData()){data->newData(); data->from(edit);}
               makeGameVer(elm);
               Server.setElmFull(elm.id);
            }break;

            case ELM_GUI_SKIN:
            {
               EditGuiSkin edit; edit.newData(); Save(edit, editPath(elm));
               if(ElmGuiSkin *data=elm.guiSkinData()){data->newData(); data->from(edit);}
               makeGameVer(elm);
               Server.setElmFull(elm.id);
            }break;

            case ELM_TEXT_STYLE:
            {
               EditTextStyle edit; edit.newData(); Save(edit, editPath(elm));
               if(ElmTextStyle *data=elm.textStyleData()){data->newData(); data->from(edit);}
               makeGameVer(elm);
               Server.setElmFull(elm.id);
            }break;

            case ELM_ENV:
            {
               EditEnv edit; edit.newData(); Save(edit, editPath(elm));
               if(ElmEnv *data=elm.envData()){data->newData(); data->from(edit);}
               makeGameVer(elm);
               Server.setElmFull(elm.id);
            }break;

            case ELM_OBJ_CLASS:
            {
               EditObject edit; edit.newData(); edit.setType(true, elm.id, edit_path).setAccess(true, OBJ_ACCESS_CUSTOM); Save(edit, editPath(elm.id)); // type must be set at the moment of creation, also since this is an object class, then make all objects based on this have OBJ_ACCESS_CUSTOM access
               if(ElmObjClass *data=elm.objClassData()){data->newData(); data->from(edit);}
               makeGameVer(elm);
               Server.setElmFull(elm.id);
            }break;

            case ELM_OBJ:
            {
               EditObject edit; edit.newData(); Save(edit, editPath(elm.id));
               if(ElmObj *data=elm.objData()){data->newData(); data->from(edit);}
               makeGameVer(elm);
               Server.setElmFull(elm.id);
            }break;

            case ELM_ANIM:
            {
               Animation anim; anim.length(1, false);
               if(ElmAnim *data=elm.animData())
               {
                  data->newData(); data->from(anim);
                  if(parent && parent->type==ELM_OBJ)data->skel_id=objToSkel(parent);
                  if(Elm *skel=findElm(data->skel_id))if(ElmSkel *skel_data=skel->skelData())data->transform=skel_data->transform;
                  anim.linear(data->linear()).loop(data->loop());
               }
               anim.save(gamePath(elm)); savedGame(elm);
               Server.setElmFull(elm.id);
            }break;

            case ELM_IMAGE:
            {
               if(ElmImage *data=elm.imageData())
               {
                  data->newData();
                  data->mipMaps(CodeEdit.importImageMipMaps());
               }
               makeGameVer(elm); Server.setElmFull(elm.id);
            }break;

            case ELM_ENUM: {if(ElmData *data=elm.Data())data->newData(); makeGameVer(elm); Server.setElmFull(elm.id);} break;

            case ELM_ICON:
            {
               if(ElmIcon *data=elm.iconData())
               {
                  data->newData();
                  if(parent && parent->type==ELM_OBJ)data->obj_id=parent->id;
               }
               Image image; image.save(gamePath(elm)); savedGame(elm); Server.setElmFull(elm.id); // save empty game image (edit can be left empty because it will be set to default)
            }break;

            case ELM_GUI        : {if(ElmGui        *data=elm.       guiData())data->newData(); GuiObjs       go; go   .save(gamePath(elm)); savedGame(elm); Server.setElmFull(elm.id);} break; // currently gui doesn't have edit version
            case ELM_IMAGE_ATLAS: {if(ElmImageAtlas *data=elm.imageAtlasData())data->newData(); ImageAtlas atlas; atlas.save(gamePath(elm)); savedGame(elm); Server.setElmFull(elm.id);} break; // save empty game atlas

            case ELM_SOUND: {if(ElmData *data=elm.Data())data->newData(); File f; f.writeTry(gamePath(elm)); f.del(); savedGame(elm); Server.setElmFull(elm.id);} break;
            case ELM_VIDEO: {if(ElmData *data=elm.Data())data->newData(); File f; f.writeTry(gamePath(elm)); f.del(); savedGame(elm); Server.setElmFull(elm.id);} break;
            case ELM_FILE : {if(ElmData *data=elm.Data())data->newData(); File f; f.writeTry(gamePath(elm)); f.del(); savedGame(elm); Server.setElmFull(elm.id);} break;

            case ELM_MINI_MAP:
            {
               if(MiniMapVer *ver=miniMapVerRequire(elm.id))
               {
                  ver->changed=true;
                  ver->time++;
                  if(ElmMiniMap *data=elm.miniMapData()){data->newData(); data->copyTo(ver->settings);}
                  createMiniMapPaths(elm.id); ver->settings.save(gamePath(elm.id).tailSlash(true)+"Settings");
                  Server.setElmFull(elm.id);
                  Server.setMiniMapSettings(elm.id, ver->settings, ver->time);
               }
            }break;

            default:
            {
               if(ElmData *data=elm.Data())data->newVer();
               Server.newElm(elm);
            }break;
         }
         if(refresh_elm_list)setList();
         if(type==ELM_CODE)activateSources(-1); // call after 'setList'
         if(!name) // name is only provided through Editor Network Interface or Drag and Drop file Importing
         {
            elmLocate(elm.id, true);
            RenameElm.activate(elm.id, elm_name);
            if(ElmChange *change=elm_undos.set(null, true))
            {
               change->type=ElmChange::RESTORE;
               change->name="New Element";
               change->elms.binaryInclude(elm.id);
            }
         }
         return &elm;
      }
      return null;
   }
   Elm* ProjectEx::newWorld(C Str &name, int area_size, int hm_res, C UID &parent_id, bool set_cur)
   {
      area_size=NearestPow2(area_size);
        hm_res =NearestPow2(  hm_res );
      if(area_size>=32 && area_size<=128
      &&   hm_res >=32 &&   hm_res <=128)
      {
         setListCurSel();
         Elm *parent=findElm(parent_id); if(parent)parent->opened(true);
         Elm &elm=super::newElm(name, parent_id, ELM_WORLD);
         if(ElmWorld *ew=elm.worldData())
         {
            ew->newData();
            ew->area_size=area_size;
            ew->  hm_res =  hm_res ;
         }
         makeGameVer(elm);
         Server.setElmFull(elm.id);
         setList();
         if(set_cur)list.setCur(list.elmToVis(&elm));
         return &elm;
      }
      return null;
   }
   void ProjectEx::setMtrl(Elm &mtrl, C XMaterialEx &src, C Str &src_file)
   {
      if(ElmMaterial *data=mtrl.mtrlData())
      {
         elmChanging(mtrl);

         if(src.base_0      .is() && includeTex(src.  base_0_id))saveTex(src.base_0      , src.  base_0_id); Server.setTex(src.  base_0_id);
         if(src.base_1      .is() && includeTex(src.  base_1_id))saveTex(src.base_1      , src.  base_1_id); Server.setTex(src.  base_1_id);
         if(src.base_2      .is() && includeTex(src.  base_2_id))saveTex(src.base_2      , src.  base_2_id); Server.setTex(src.  base_2_id);
         if(src.detail      .is() && includeTex(src.  detail_id))saveTex(src.detail      , src.  detail_id); Server.setTex(src.  detail_id);
         if(src.macro       .is() && includeTex(src.   macro_id))saveTex(src.macro       , src.   macro_id); Server.setTex(src.   macro_id);
         if(src.emissive_img.is() && includeTex(src.emissive_id))saveTex(src.emissive_img, src.emissive_id); Server.setTex(src.emissive_id);

         TimeStamp time; time.getUTC();
         EditMaterial edit_mtrl; edit_mtrl.create(src, time);
         Save(edit_mtrl, editPath(mtrl));
         makeGameVer(mtrl);
         data->newVer();
         data->setSrcFile(src_file, time);
         data->from(edit_mtrl);
         elmChanged(mtrl);
      }
   }
   Elm& ProjectEx::newMtrl(C XMaterialEx &src, C UID parent_id, C Str &src_file) // create new material from 'src' !! this doesn't set elm list and doesn't send to the server !!
   {
      Elm &mtrl=super::newElm(src.name, parent_id, ELM_MTRL); mtrl.mtrlData()->newData();
      setMtrl(mtrl, src, src_file);
      return mtrl;
   }
   void ProjectEx::setElmName(Elm &elm, C Str &name, C TimeStamp &time)
   {
      elm.setName(name, time);
      switch(elm.type)
      {
         case ELM_ENUM: makeGameVer(elm); break; // this relies on element name
      }
      switch(elm.type)
      {
         case ELM_ENUM     :
         case ELM_OBJ_CLASS: enumChanged(elm.id); break;
         case ELM_FONT     : fontChanged(); break;
         case ELM_CODE     : CodeEdit.sourceRename(elm.id); break;
         case ELM_APP      : CodeEdit.makeAuto(); break;
      }
   }
   void ProjectEx::setElmNames(Memc<Edit::IDParam<Str>> &elms, bool adjust_elms) // 'adjust_elms'=if this is performed because of undo, and in that case we need to remember current names, so we can undo this change later
   {
      if(elms.elms())
      {
         TimeStamp time; time.getUTC();
         Str name;
         FREPA(elms)if(Elm *elm=findElm(elms[i].id)) // get element
         {
            name=elms[i].value; // remember name to change to
            if(adjust_elms)elms[i].value=elm->name; // !! if we're adjusting, then set current name !!
            setElmName(*elm, name, time);
            Server.renameElm(*elm);
         }
         refresh(false, false);
      }
   }
   void ProjectEx::renameElm(C UID &elm_id, C Str &name)
   {
      if(Elm *elm=findElm(elm_id))
         if(!Equal(elm->name, name, true))
      {
         setElmName(*elm, name);
         Server.renameElm(*elm);
         refresh(false, false);
      }
   }
   void ProjectEx::replaceName(C MemPtr<UID> &elm_ids, C Str &from, C Str &to)
   {
      if(elm_ids.elms())
      {
         ElmChange *change=elm_undos.set(null, true); if(!change)return;
         change->type=ElmChange::SET_NAME;
         change->name="Replace Name";

         Str name;
         FREPA(elm_ids)if(Elm *elm=findElm(elm_ids[i]))
         {
            name=Replace(elm->name, from, to); if(!Equal(elm->name, name, true))
            {
               Edit::IDParam<Str> &elm_name=change->elm_names.New();
               elm_name.id   =elm->id;
               elm_name.value=name;
            }
         }
         setElmNames(change->elm_names, true);
      }
   }
   void ProjectEx::closeElm(C UID &elm_id) // close elements that keep handles to files (sounds/vidoes)
   {
      if(elm_id==list.sound_play.play_id)sound.close();
        Preview.closeElm(elm_id);
      SoundEdit.closeElm(elm_id);
      VideoEdit.closeElm(elm_id);
   }
   void ProjectEx::paramEditObjChanged(C UID *obj_id)
   {
        ObjEdit.param_edit.objChanged(obj_id);
      WorldEdit.param_edit.objChanged(obj_id);
              ObjClassEdit.objChanged(obj_id);
   }
   void ProjectEx::remove(ElmNode &node, Memc<UID> &sorted_ids, Memc<UID> &removed, C TimeStamp &time) // process recursively to remove only parents without their children
   {
      FREPA(node.children)
      {
         int  child_i=node.children[i];
         Elm &elm    =elms[child_i];
         if(  elm.exists())
         {
            if(sorted_ids.binaryHas(elm.id)) // if want to remove this element
            {
               if(time>elm.removed_time){elm.setRemoved(true, time); removed.add(elm.id);} // remove self, but skip the children
            }else // continue checking children
            {
               remove(hierarchy[child_i], sorted_ids, removed, time);
            }
         }
      }
   }
   void ProjectEx::remove(Memc<UID> &ids, bool parents_only, bool set_undo) // !! WARNING: MIGHT SORT 'ids' !!
   {
      if(ids.elms())
      {
         setListCurSel();
         TimeStamp time; time.getUTC(); Memc<UID> removed;

         if(parents_only)
         {
            ids.sort(); remove(root, ids, removed, time);
         }else
         REPA(ids)if(Elm *elm=findElm(ids[i]))if(elm->exists() && time>elm->removed_time){elm->setRemoved(true, time); removed.add(elm->id);}

         Server.removeElms(removed, true, time);
         if(set_undo)if(ElmChange *change=elm_undos.set(null, true)) // !! 'force_create' ALSO NEEDED BECAUSE OF "Swap(change.elms" BELOW !!
         {
            change->type=ElmChange::REMOVE;
            change->name="Remove";
            Swap(change->elms, removed);
         }
         setList(false);
         elmChangedParentRemovePublish();
      }
   }
   void ProjectEx::restore(Memc<UID> &ids, bool set_undo)
   {
      if(ids.elms())
      {
         setListCurSel();
         TimeStamp time; time.getUTC(); Memc<UID> restored;

         REPA(ids)if(Elm *elm=findElm(ids[i]))if(elm->removed() && time>elm->removed_time){elm->setRemoved(false, time); restored.add(elm->id);}

         Server.removeElms(restored, false, time);
         if(set_undo)if(ElmChange *change=elm_undos.set(null, true)) // !! 'force_create' ALSO NEEDED BECAUSE OF "Swap(change.elms" BELOW !!
         {
            change->type=ElmChange::RESTORE;
            change->name="Restore";
            Swap(change->elms, restored);
         }
         setList(false);
         Importer.investigate(ids); // !! CALL AFTER 'setList' BECAUSE MAY RELY ON HIERARCHY !!
         elmChangedParentRemovePublish();
      }
   }
   void ProjectEx::_setElmPublish(ElmNode &node, Memc<UID> &sorted_ids, uint test, uint set, Memc<UID> &processed, C TimeStamp &time) // process recursively to disable only parents without their children
   {
      REPA(node.children)
      {
         int  child_i=node.children[i];
         Elm &elm    =elms[child_i];
         if(!(elm.flag&test)) // if has publishing enabled ("!" because we have NO_PUBLISH* flags)
         {
            if(sorted_ids.binaryHas(elm.id)) // if want to set this element
            {
               if(time>elm.publish_time){FlagCopy(elm.flag, set, test); elm.publish_time=time; processed.add(elm.id);} // disable self, but skip children
            }else // continue checking children
            {
              _setElmPublish(hierarchy[child_i], sorted_ids, test, set, processed, time);
            }
         }
      }
   }
   void ProjectEx::_setElmPublish(Memc<UID> &ids, uint test, uint set, bool parents_only, bool set_undo) // !! WARNING: MIGHT SORT 'ids' !!
   {
      if(ids.elms())
      {
         TimeStamp time; time.getUTC(); Memc<UID> processed;

         if(parents_only && set==test) // supported only if all NO_PUBLISH* flags are enabled -> disabling publishing
         {
            ids.sort(); _setElmPublish(root, ids, test, set, processed, time);
         }else
         REPA(ids)if(Elm *elm=findElm(ids[i]))if(time>elm->publish_time && (elm->flag&test)!=set){FlagCopy(elm->flag, set, test); elm->publish_time=time; processed.add(elm->id);}

         Server.publishElms(processed, (test&Elm::NO_PUBLISH       ) ? FlagOff(set, Elm::NO_PUBLISH       ) : -1,
                                       (test&Elm::NO_PUBLISH_MOBILE) ? FlagOff(set, Elm::NO_PUBLISH_MOBILE) : -1, time);
         if(set_undo)if(ElmChange *change=elm_undos.set(null, true)) // !! 'force_create' ALSO NEEDED BECAUSE OF "Swap(change.elms" BELOW !!
         {
            change->type=ElmChange::PUBLISH;
            change->name=((set==test) ? "Disable Publishing" // if all NO_PUBLISH* flags are enabled -> disabling publishing
                       : (set==0   ) ?  "Enable Publishing" // if no  NO_PUBLISH* flags are enabled ->  enabling publishing
                       :                "Change Publishing"); // some enabled, some disabled        ->  changing publishing
            change->test=test;
            change->set =set;
            Swap(change->elms, processed);
         }
         refresh(false);
         elmChangedParentRemovePublish();
      }
   }
   void ProjectEx::setElmPublish(Memc<UID> &ids, sbyte all, sbyte mobile, bool parents_only, bool set_undo) // !! WARNING: MIGHT SORT 'ids' !!
   {
      if(ids.elms()) // any elements
         if(all>=0 || mobile>=0) // anything to change
      {
         uint test=0, set=0;
         if(all   >=0){test|=Elm::NO_PUBLISH       ; if(!all   )set|=Elm::NO_PUBLISH       ;}
         if(mobile>=0){test|=Elm::NO_PUBLISH_MOBILE; if(!mobile)set|=Elm::NO_PUBLISH_MOBILE;}
        _setElmPublish(ids, test, set, parents_only, set_undo);
      }
   }
   void ProjectEx::disablePublish(Memc<UID> &ids, bool parents_only, bool set_undo) // !! WARNING: MIGHT SORT 'ids' !!
   {
      setElmPublish(ids, false, -1, parents_only, set_undo);
   }
   void ProjectEx::reload(Memc<UID> &elm_ids)
   {
      if(elm_ids.elms()> 1)ReloadElm.activate(elm_ids);else
      if(elm_ids.elms()==1)ReloadElm.activate(elm_ids[0]);
   }
   void ProjectEx::cancelReload(C MemPtr<UID> &elm_ids)
   {
      REPA(elm_ids)if(Elm *elm=findElm(elm_ids[i]))elm->importing(false);
      Importer.cancel(elm_ids);
      refresh(false, false);
   }
   void ProjectEx::removeMeshVtx(C MemPtr<UID> &elm_ids, MESH_FLAG flag)
   {
      if(flag)REPA(elm_ids)if(Elm *obj_elm=findElm(elm_ids[i], ELM_OBJ))if(ElmObj *obj_data=obj_elm->objData())if(Elm *mesh_elm=findElm(obj_data->mesh_id, ELM_MESH))
      {
         if(elm_ids[i]==ObjEdit.obj_id) // if this is currently edited object
         {
            ObjEdit.remVtx(flag);
         }else
         {
            Mesh mesh; if(Load(mesh, editPath(mesh_elm->id), game_path))
            {
               bool changed=false;
               REPD(l, mesh.lods())
               {
                  MeshLod &lod=mesh.lod(l); REPA(lod)
                  {
                     MeshPart &part=lod.parts[i]; if(part.flag()&flag){changed=true; part.exclude(flag);}
                  }
               }
               if(changed)
               {
                  if(flag&VTX_SKIN)mesh.clearSkeleton();
                  Save(mesh, editPath(mesh_elm->id), game_path);
                  makeGameVer(*mesh_elm);
                  if(ElmMesh *mesh_data=mesh_elm->meshData())
                  {
                     mesh_data->newVer();
                     mesh_data->file_time.getUTC();
                  }
                  meshChanged(*mesh_elm);
                  Server.setElmLong(mesh_elm->id);
               }
            }
         }
      }
   }
   void ProjectEx::disableLQLODs(C MemPtr<UID> &elm_ids)
   {
      REPA(elm_ids)if(Elm *obj_elm=findElm(elm_ids[i], ELM_OBJ))if(ElmObj *obj_data=obj_elm->objData())if(Elm *mesh_elm=findElm(obj_data->mesh_id, ELM_MESH))
      {
         if(elm_ids[i]==ObjEdit.obj_id) // if this is currently edited object
         {
            ObjEdit.meshDisableLQLODs();
         }else
         {
            Mesh mesh; if(Load(mesh, editPath(mesh_elm->id), game_path) && DisableLQLODs(mesh))
            {
               Save(mesh, editPath(mesh_elm->id), game_path);
               makeGameVer(*mesh_elm);
               if(ElmMesh *mesh_data=mesh_elm->meshData())
               {
                  mesh_data->newVer();
                  mesh_data->file_time.getUTC();
               }
               meshChanged(*mesh_elm);
               Server.setElmLong(mesh_elm->id);
            }
         }
      }
   }
   void ProjectEx::meshMergeCoplanarFaces(C MemPtr<UID> &elm_ids)
   {
      REPA(elm_ids)if(Elm *obj_elm=findElm(elm_ids[i], ELM_OBJ))if(ElmObj *obj_data=obj_elm->objData())if(Elm *mesh_elm=findElm(obj_data->mesh_id, ELM_MESH))
      {
         if(elm_ids[i]==ObjEdit.obj_id) // if this is currently edited object
         {
            ObjEdit.meshMergeCoplanarFaces(true);
         }else
         {
            Mesh mesh; if(Load(mesh, editPath(mesh_elm->id), game_path) && mesh.weldCoplanarFaces())
            {
               Save(mesh, editPath(mesh_elm->id), game_path);
               makeGameVer(*mesh_elm);
               if(ElmMesh *mesh_data=mesh_elm->meshData())
               {
                  mesh_data->newVer();
                  mesh_data->file_time.getUTC();
               }
               meshChanged(*mesh_elm);
               Server.setElmLong(mesh_elm->id);
            }
         }
      }
   }
   bool ProjectEx::forceImageSize(Str &file, C VecI2 &size, bool relative)
   {
      Mems<FileParams> files=FileParams::Decode(file); if(files.elms())
      {
         Str resize_name;
       //if(relative) // for relative we need to remove any existing "resize" before calling 'loadImages', actually do this always, because there are now many "resize" commands and we want to remove all of them
            REPA(files) // go from end
         {
            FileParams &file=files[i];
            if(i && (file.name.is() || file.nodes.elms()))break; // stop on first file that has name (but allow the first which means there's only one file) so we don't process transforms for only 1 of multiple images
            REPA(file.params) // go from end
            {
               TextParam &p=file.params[i];
               if(ResizeTransform(p.name))
               {
                  if(!resize_name.is())resize_name=p.name; // get the name of resize at the end (check if not yet set because we're processing from the end), this is needed to preserve filtering choice
                  file.params.remove(i, true); // remove resize transforms
               }else
               if(SizeDependentTransform(p))goto skip; // if encountered a size dependent transform then it means we can't remove any other resize transforms
            }
            if(!file.is())files.remove(i, true); // if nothing left then remove it
         }
      skip:
         if(!resize_name.is())resize_name="resize"; // set default resize name
         bool specific_resize=(resize_name!="resize"); // if not default
         if(files.elms() && !(relative && !size.x && !size.y && !specific_resize)) // "relative && !size" means original size, for which we don't need to do anything, because "resize" was already removed
         {
            VecI2 s=size;
            if(relative) // for relative size, we need to get information about the source image size
            {
               Image temp; if(!loadImages(temp, null, FileParams::Encode(files)))return false; // if failed to load, then do nothing, can ignore sRGB
               s.set(Max(1, Shl(temp.w(), size.x)), Max(1, Shl(temp.h(), size.y)));
               s.set(NearestPow2(s.x), NearestPow2(s.y)); // textures are gonna be resized to pow2 anyway, so force pow2 size, to avoid double resize
            }

            // set "resize" param into 'files'
            if(specific_resize || s.any())SetResizeTransform(files, resize_name, s.any() ? TextVecI2Ex(s) : S); // only if any specified
         }
         file=FileParams::Encode(files);
         return true;
      }
      return false;
   }
   bool ProjectEx::forceImageSize(Str &file, C VecI2 &size, bool relative, TimeStamp &file_time, C TimeStamp &time)
   {
      if(forceImageSize(file, size, relative))
      {
         file_time=time;
         return true;
      }
      return false;
   }
   bool ProjectEx::imageMipMap(C MemPtr<UID> &elm_ids, bool on)
   {
      bool ok=true;
      REPA(elm_ids)if(Elm *image=findElm(elm_ids[i], ELM_IMAGE))
      {
         if(ElmImage *image_data=image->imageData())
         {
            if(image_data->mipMaps()!=on)
            if(ImageEdit.elm==image)ImageEdit.setMipMap(on);else
            {
               image_data->newVer();
               image_data->mipMaps(on); image_data->mip_maps_time.now();
               makeGameVer(*image);
               Server.setElmShort(image->id);
            }
         }else ok=false;
      }else ok=false;
      return ok;
   }
   void ProjectEx::imageResize(C MemPtr<UID> &elm_ids, C VecI2 &size)
   {
      REPA(elm_ids)
      if(Elm *image=findElm(elm_ids[i], ELM_IMAGE))
      if(ElmImage *image_data=image->imageData())
      if(image_data->size!=size)
      if(ImageEdit.elm==image)ImageEdit.setSize(size);else
      {
         image_data->newVer();
         image_data->size=size; image_data->size_time.now();
         makeGameVer(*image);
         Server.setElmShort(image->id);
      }
   }
   void ProjectEx::mtrlResetAlpha(C MemPtr<UID> &elm_ids)
   {
      REPA(elm_ids)
      if(Elm *mtrl=findElm(elm_ids[i], ELM_MTRL))
      if(ElmMaterial *mtrl_data=mtrl->mtrlData())
      if(MtrlEdit.elm==mtrl)MtrlEdit.resetAlpha();else
      {
         EditMaterial edit; if(edit.load(editPath(mtrl->id)))
         {
            mtrl_data->newVer();
            edit.resetAlpha();
            Save(edit, editPath(mtrl->id));
            makeGameVer(*mtrl);
            Server.setElmLong(mtrl->id);
         }
      }
   }
   void ProjectEx::mtrlSetRGB(C MemPtr<UID> &elm_ids, C Vec &srgb, bool mul)
   {
      if(!mul || srgb!=1)
      REPA(elm_ids)
      if(Elm *mtrl=findElm(elm_ids[i], ELM_MTRL))
      if(ElmMaterial *mtrl_data=mtrl->mtrlData())
      if(MtrlEdit.elm==mtrl)MtrlEdit.setRGB(mul ? MtrlEdit.edit.color_s.xyz*srgb : srgb);else
      {
         EditMaterial edit; if(edit.load(editPath(mtrl->id)))if(mul || edit.color_s.xyz!=srgb)
         {
            mtrl_data->newVer();
            if(mul)edit.color_s.xyz*=srgb;else edit.color_s.xyz=srgb; edit.color_time.now();
            Save(edit, editPath(mtrl->id));
            makeGameVer(*mtrl);
            Server.setElmLong(mtrl->id);
         }
      }
   }
   bool ProjectEx::mtrlSetTech(C MemPtr<UID> &elm_ids, MATERIAL_TECHNIQUE tech)
   {
      bool ok=true;
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(edit.tech!=tech)
         {
            edit.tech=tech; edit.tech_time.now();
            ok&=mtrlSync(elm_ids[i], edit, false, false, "setTech");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlSetBump(C MemPtr<UID> &elm_ids, flt bump, bool mul)
   {
      bool ok=true;
      if(!mul || bump!=1)
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(mul || edit.bump!=bump)
         {
            if(mul)edit.bump*=bump;else edit.bump=bump; edit.bump_time.now();
            ok&=mtrlSync(elm_ids[i], edit, false, false, "setBump");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlSetNormal(C MemPtr<UID> &elm_ids, flt normal, bool mul)
   {
      bool ok=true;
      if(!mul || normal!=1)
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(mul || edit.normal!=normal)
         {
            if(mul)edit.normal*=normal;else edit.normal=normal; edit.normal_time.now();
            ok&=mtrlSync(elm_ids[i], edit, false, false, "setNrm");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlSetSmooth(C MemPtr<UID> &elm_ids, flt smooth, bool mul)
   {
      bool ok=true;
      if(!mul || smooth!=1)
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(mul || edit.smooth!=smooth)
         {
            if(mul)edit.smooth*=smooth;else edit.smooth=smooth; edit.smooth_time.now();
            ok&=mtrlSync(elm_ids[i], edit, false, false, "setSmooth");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlSetReflect(C MemPtr<UID> &elm_ids, flt reflect_min, flt reflect_max)
   {
      bool ok=true;
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(edit.reflect_min!=reflect_min || edit.reflect_max!=reflect_max)
         {
            edit.reflect_min=reflect_min; edit.reflect_max=reflect_max; edit.reflect_time.now();
            ok&=mtrlSync(elm_ids[i], edit, false, false, "setReflect");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlSetGlow(C MemPtr<UID> &elm_ids, flt glow, bool mul)
   {
      bool ok=true;
      if(!mul || glow!=1)
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(mul || edit.glow!=glow)
         {
            if(mul)edit.glow*=glow;else edit.glow=glow; edit.glow_time.now();
            ok&=mtrlSync(elm_ids[i], edit, false, false, "setGlow");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlSetTexColor(C MemPtr<UID> &elm_ids, C Str &color_map)
   {
      bool ok=true;
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(!Equal(edit.color_map, color_map, true))
         {
            edit.color_map=color_map; edit.color_map_time.now();
            ok&=mtrlSync(elm_ids[i], edit, true, true, "setTexCol");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlSetTexBump(C MemPtr<UID> &elm_ids, C Str &bump_map)
   {
      bool ok=true;
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(!Equal(edit.bump_map, bump_map, true))
         {
            edit.bump_map=bump_map; edit.bump_map_time.now();
            ok&=mtrlSync(elm_ids[i], edit, true, true, "setTexBump");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlSetTexNormal(C MemPtr<UID> &elm_ids, C Str &normal_map, bool flip_normal_y)
   {
      bool ok=true;
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(!Equal(edit.normal_map, normal_map, true)
         ||        edit.flip_normal_y!=flip_normal_y)
         {
            edit.normal_map=normal_map; edit.normal_map_time.now();
            edit.flip_normal_y=flip_normal_y; edit.flip_normal_y_time=edit.normal_map_time;
            ok&=mtrlSync(elm_ids[i], edit, true, true, "setTexNrm");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlSetTexSmooth(C MemPtr<UID> &elm_ids, C Str &smooth_map, bool smooth_is_rough)
   {
      bool ok=true;
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(!Equal(edit.smooth_map, smooth_map, true)
         ||        edit.smooth_is_rough!=smooth_is_rough)
         {
            edit.smooth_map=smooth_map; edit.smooth_map_time.now();
            edit.smooth_is_rough=smooth_is_rough; edit.smooth_is_rough_time=edit.smooth_map_time;
            ok&=mtrlSync(elm_ids[i], edit, true, true, "setTexSmooth");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlSetTexMetal(C MemPtr<UID> &elm_ids, C Str &metal_map)
   {
      bool ok=true;
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(!Equal(edit.metal_map, metal_map, true))
         {
            edit.metal_map=metal_map; edit.metal_map_time.now();
            ok&=mtrlSync(elm_ids[i], edit, true, true, "setTexMetal");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlSetTexGlow(C MemPtr<UID> &elm_ids, C Str &glow_map)
   {
      bool ok=true;
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(!Equal(edit.glow_map, glow_map, true))
         {
            edit.glow_map=glow_map; edit.glow_map_time.now();
            ok&=mtrlSync(elm_ids[i], edit, true, true, "setTexGlow");
         }
      }
      return ok;
   }
   void ProjectEx::mtrlCull(C MemPtr<UID> &elm_ids, bool on)
   {
      REPA(elm_ids)
      if(Elm *mtrl=findElm(elm_ids[i], ELM_MTRL))
      if(ElmMaterial *mtrl_data=mtrl->mtrlData())
      if(MtrlEdit.elm==mtrl)MtrlEdit.cull(on);else
      {
         EditMaterial edit; if(edit.load(editPath(mtrl->id)))if(edit.cull!=on)
         {
            mtrl_data->newVer();
            edit.cull=on; edit.cull_time.now();
            Save(edit, editPath(mtrl->id));
            makeGameVer(*mtrl);
            Server.setElmLong(mtrl->id);
         }
      }
   }
   void ProjectEx::mtrlFlipNrmY(C MemPtr<UID> &elm_ids, bool on)
   {
      REPA(elm_ids)
      if(Elm *mtrl=findElm(elm_ids[i], ELM_MTRL))
      if(ElmMaterial *mtrl_data=mtrl->mtrlData())
      if(MtrlEdit.elm==mtrl)MtrlEdit.flipNrmY(on);else
      {
         EditMaterial edit; if(edit.load(editPath(mtrl->id)))if(edit.flip_normal_y!=on)
         {
            mtrl_data->newVer();
            edit.flip_normal_y=on; edit.flip_normal_y_time.now();
            Save(edit, editPath(mtrl->id));
          //makeGameVer(*mtrl); not needed because 'flip_normal_y' is not stored in game, however if tex ID is changed, then it's handled by 'mtrlReloadTextures' below
            if(!mtrlReloadTextures(mtrl->id, true, false, false, false))Server.setElmLong(mtrl->id); // 'Server.setElmLong' will be called by 'mtrlReloadTextures' unless it failed
         }
      }
   }
   void ProjectEx::mtrlSmoothIsRough(C MemPtr<UID> &elm_ids, bool on)
   {
      REPA(elm_ids)
      if(Elm *mtrl=findElm(elm_ids[i], ELM_MTRL))
      if(ElmMaterial *mtrl_data=mtrl->mtrlData())
      if(MtrlEdit.elm==mtrl)MtrlEdit.smoothIsRough(on);else
      {
         EditMaterial edit; if(edit.load(editPath(mtrl->id)))if(edit.smooth_is_rough!=on)
         {
            mtrl_data->newVer();
            edit.smooth_is_rough=on; edit.smooth_is_rough_time.now();
            Save(edit, editPath(mtrl->id));
          //makeGameVer(*mtrl); not needed because 'smooth_is_rough' is not stored in game, however if tex ID is changed, then it's handled by 'mtrlReloadTextures' below
            if(!mtrlReloadTextures(mtrl->id, true, false, false, false))Server.setElmLong(mtrl->id); // 'Server.setElmLong' will be called by 'mtrlReloadTextures' unless it failed
         }
      }
   }
   void ProjectEx::mtrlTexDownsize(C MemPtr<UID> &elm_ids, TEX_SIZE_PLATFORM tsp, byte downsize, C UID &base_0, C UID &base_1, C UID &base_2, bool precise) // 'precise'=if do a more precise but slower processing (if true then all materials from 'elm_ids' will have to be loaded from disk for checking, which might be slow)
   {
      Memt<UID> mtrls;
      REPA(elm_ids)if(C Elm *mtrl=findElm(elm_ids[i]))if(C ElmMaterial *mtrl_data=mtrl->mtrlData())
         if(precise // if always check (will correctly set other materials)
         || mtrl_data->tex_downsize[tsp]!=downsize) // only if different
            if(mtrls.binaryInclude(mtrl->id)) // if just added
      {
         // include other materials that have the same textures
         // can't check by just 'mtrl_data.base_*_tex' because textures for this material could have been already reloaded with different quality, which could generate a different ID for them, instead we will check materials by Tex ID and Tex Src File
         // can't just compare materials by Tex Src File because this is not stored in ElmMaterial and would require loading 'EditMaterial' for all materials which would be slow
         // so only materials which tex ID is included in 'tex_ids' will be loaded
         Memt<UID> tex_ids;
         if(mtrl_data->base_0_tex.valid())tex_ids.binaryInclude(mtrl_data->base_0_tex); // include searching for 'mtrl_data.base_0_tex'
         if(mtrl_data->base_1_tex.valid())tex_ids.binaryInclude(mtrl_data->base_1_tex); // include searching for 'mtrl_data.base_1_tex'
         if(mtrl_data->base_2_tex.valid())tex_ids.binaryInclude(mtrl_data->base_2_tex); // include searching for 'mtrl_data.base_2_tex'
         if(          base_0    .valid())tex_ids.binaryInclude(          base_0    ); // include searching for 'base_0' (Tex ID before reload)
         if(          base_1    .valid())tex_ids.binaryInclude(          base_1    ); // include searching for 'base_1' (Tex ID before reload)
         if(          base_2    .valid())tex_ids.binaryInclude(          base_2    ); // include searching for 'base_2' (Tex ID before reload)
         if(tex_ids.elms()) // if has any Tex ID's to compare
         {
            bool         edit_loaded=false;
            EditMaterial edit; // don't load 'edit' yet, because maybe we won't need it
            FREPA(elms) // check all other materials, process in order to make 'binaryInclude' faster
            {
             C Elm &elm=elms[i]; if(C ElmMaterial *test_data=elm.mtrlData())if(test_data->tex_downsize[tsp]!=downsize)if(mtrl!=&elm)
               if(tex_ids.binaryHas(test_data->base_0_tex) // if their textures match any in 'tex_ids'
               || tex_ids.binaryHas(test_data->base_1_tex)
               || tex_ids.binaryHas(test_data->base_2_tex))
               {
                  if(!edit_loaded){if(mtrlGet(mtrl->id, edit))edit_loaded=true;else break;} // make sure 'edit' is now loaded

                  EditMaterial test;
                  if(mtrlGet(elm.id, test)) // do extra checks if maps are the same
                  if(edit. color_map.is() && EqualPath(edit. color_map, test. color_map)
                  || edit.smooth_map.is() && EqualPath(edit.smooth_map, test.smooth_map)
                  || edit. metal_map.is() && EqualPath(edit. metal_map, test. metal_map)
                  || edit.  bump_map.is() && EqualPath(edit.  bump_map, test.  bump_map)
                  || edit.normal_map.is() && EqualPath(edit.normal_map, test.normal_map)
                  || edit.  glow_map.is() && EqualPath(edit.  glow_map, test.  glow_map))
                     mtrls.binaryInclude(elm.id); // process this material too
               }
            }
         }
      }
      REPA(mtrls)if(Elm *mtrl=findElm(mtrls[i]))if(ElmMaterial *mtrl_data=mtrl->mtrlData())if(mtrl_data->tex_downsize[tsp]!=downsize)
      {
         if(MtrlEdit.elm==mtrl)MtrlEdit.texDownsize(tsp, downsize);else
         {
            EditMaterial edit; if(edit.load(editPath(mtrl->id)))
            {
               mtrl_data->newVer();
               mtrl_data->tex_downsize[tsp]=edit.tex_downsize[tsp]=downsize; edit.tex_downsize_time.now();
               Save(edit, editPath(mtrl->id));
             //makeGameVer(*mtrl); this is not needed because 'tex_downsize' is not stored in the game version, instead textures are downsized during publishing
               Server.setElmLong(mtrl->id); // Long is needed because 'tex_downsize_time' is only in edit
            }
         }
      }
   }
   bool ProjectEx::mtrlTexQuality(C MemPtr<UID> &elm_ids, Edit::Material::TEX_QUALITY quality, C UID &base_0, C UID &base_1, C UID &base_2, bool precise) // 'precise'=if do a more precise but slower processing (if true then all materials from 'elm_ids' will have to be loaded from disk for checking, which might be slow) // !! this is not perfect !!
   {
      bool ok=true;
      Memt<UID> mtrls;
      REPA(elm_ids)if(C Elm *mtrl=findElm(elm_ids[i]))if(C ElmMaterial *mtrl_data=mtrl->mtrlData())
         if(precise // if always check (will correctly set other materials)
         || mtrl_data->tex_quality!=quality) // only if different
            if(mtrls.binaryInclude(mtrl->id)) // if just added
      {
         // include other materials that have the same textures
         // tex quality affects only 'base_0'
         // can't check by just 'mtrl_data.base_0_tex' because textures for this material could have been already reloaded with different quality, which could generate a different ID for them, instead we will check materials by Tex ID and Tex Src File
         // can't just compare materials by Tex Src File because this is not stored in ElmMaterial and would require loading 'EditMaterial' for all materials which would be slow
         // so only materials which tex ID is included in 'tex_ids' will be loaded
         Memt<UID> tex_ids;
         if(mtrl_data->base_0_tex.valid())tex_ids.binaryInclude(mtrl_data->base_0_tex); // include searching for 'mtrl_data.base_0_tex'
         if(mtrl_data->base_1_tex.valid())tex_ids.binaryInclude(mtrl_data->base_1_tex); // include searching for 'mtrl_data.base_1_tex'
         if(mtrl_data->base_2_tex.valid())tex_ids.binaryInclude(mtrl_data->base_2_tex); // include searching for 'mtrl_data.base_2_tex'
         if(          base_0    .valid())tex_ids.binaryInclude(          base_0    ); // include searching for 'base_0' (Tex ID before reload)
         if(          base_1    .valid())tex_ids.binaryInclude(          base_1    ); // include searching for 'base_1' (Tex ID before reload)
         if(          base_2    .valid())tex_ids.binaryInclude(          base_2    ); // include searching for 'base_2' (Tex ID before reload)
         if(tex_ids.elms()) // if has any Tex ID's to compare
         {
            bool         edit_loaded=false;
            EditMaterial edit; // don't load 'edit' yet, because maybe we won't need it
            FREPA(elms) // check all other materials, process in order to make 'binaryInclude' faster
            {
             C Elm &elm=elms[i]; if(C ElmMaterial *test_data=elm.mtrlData())if(test_data->tex_quality!=quality)if(mtrl!=&elm)
               if(tex_ids.binaryHas(test_data->base_0_tex) // if their textures match any in 'tex_ids'
               || tex_ids.binaryHas(test_data->base_1_tex)
               || tex_ids.binaryHas(test_data->base_2_tex))
               {
                  if(!edit_loaded){if(mtrlGet(mtrl->id, edit))edit_loaded=true;else break;} // make sure 'edit' is now loaded

                  EditMaterial test;
                  if(mtrlGet(elm.id, test)) // do extra checks if maps are the same
                  if(edit.color_map.is() && EqualPath(edit.color_map, test.color_map))
                     mtrls.binaryInclude(elm.id); // process this material too
               }
            }
         }
      }
      REPA(mtrls)if(Elm *mtrl=findElm(mtrls[i]))if(ElmMaterial *mtrl_data=mtrl->mtrlData())if(mtrl_data->tex_quality!=quality)
      {
         if(MtrlEdit.elm==mtrl)MtrlEdit.texQuality(quality);else
         {
            EditMaterial edit; if(!mtrlGet(mtrl->id, edit))ok=false;else
            {
               edit.tex_quality=quality; edit.tex_quality_time.now();
               ok&=mtrlSync(mtrl->id, edit, true, false, "texQuality");
            }
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlMulTexCol(C MemPtr<UID> &elm_ids)
   {
      bool ok=true;
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(!Equal(edit.color_s.xyz, Vec(1)) && edit.color_map.is())
         {
            Mems<FileParams> fps=FileParams::Decode(edit.color_map);
            Vec mul=edit.color_s.xyz; if(C TextParam *p=FindTransform(fps, "mulRGB"))mul*=TextVecEx(p->value);
            if(Equal(mul, Vec(1)))DelTransform(fps, "mulRGB");
            else                  SetTransform(fps, "mulRGB", TextVecEx(mul));
            edit.color_map=FileParams::Encode(fps); edit.color_map_time.now();
            edit.color_s.xyz=1; edit.color_time.now();
            ok&=mtrlSync(elm_ids[i], edit, true, false, "mulTexCol");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlMulTexNormal(C MemPtr<UID> &elm_ids)
   {
      bool ok=true;
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(!Equal(edit.normal, 1) && edit.hasNormalMap())
         {
            edit.separateNormalMap();
            Mems<FileParams> fps=FileParams::Decode(edit.normal_map);
            flt mul=edit.normal; if(C TextParam *p=FindTransform(fps, "scale"))mul*=p->asFlt();
            if(Equal(mul, 1))DelTransform(fps, "scale");
            else             SetTransform(fps, "scale", TextReal(mul, -3));
            edit.normal_map=FileParams::Encode(fps); edit.normal_map_time.now();
            edit.normal=1; edit.normal_time.now();
            ok&=mtrlSync(elm_ids[i], edit, true, false, "mulTexNrm");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlMulTexSmooth(C MemPtr<UID> &elm_ids)
   {
      bool ok=true;
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(edit.smooth_map.is())
            if(!Equal(edit.smoothMul(), 1) || !Equal(edit.smoothAdd(), 0))
         {
            Mems<FileParams> fps=FileParams::Decode(edit.smooth_map);

            Vec mul0, add0; ExtractLinearTransform(fps, mul0, add0);
            flt mul1=(edit.smooth_is_rough ? edit.roughMul() : edit.smoothMul()),
                add1=(edit.smooth_is_rough ? edit.roughAdd() : edit.smoothAdd());
            // new=(img*mul0+add0)*mul1+add1
            // new=img*mul0*mul1 + add0*mul1+add1
            Vec mul=mul0*mul1, add=add0*mul1+add1;

            bool need_mul=!Equal(mul, Vec(1)),
                 need_add=!Equal(add, Vec(0));
            if(need_mul && need_add)AddTransform(fps, "mulAddRGB", TextVecVecEx(mul, add));else
            if(need_mul            )AddTransform(fps, "mulRGB"   , TextVecEx   (mul     ));else
            if(            need_add)AddTransform(fps, "addRGB"   , TextVecEx   (     add));

            edit.smooth_map=FileParams::Encode(fps); edit.smooth_map_time.now();
            edit.smooth=1; edit.smooth_time.now();
            ok&=mtrlSync(elm_ids[i], edit, true, false, "mulTexSmooth");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlMulTexGlow(C MemPtr<UID> &elm_ids)
   {
      bool ok=true;
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(!Equal(edit.glow, 1) && edit.glow_map.is())
         {
            Mems<FileParams> fps=FileParams::Decode(edit.glow_map);
            flt mul=edit.glow; if(C TextParam *p=FindTransform(fps, "mulRGB"))mul*=p->asFlt();
            if(Equal(mul, 1))DelTransform(fps, "mulRGB");
            else             SetTransform(fps, "mulRGB", TextReal(mul, -3));
            edit.glow_map=FileParams::Encode(fps); edit.glow_map_time.now();
            edit.glow=1; edit.glow_time.now();
            ok&=mtrlSync(elm_ids[i], edit, true, false, "mulTexGlow");
         }
      }
      return ok;
   }
   bool ProjectEx::mtrlMulTexEmissive(C MemPtr<UID> &elm_ids)
   {
      bool ok=true;
      REPA(elm_ids)
      {
         EditMaterial edit; if(!mtrlGet(elm_ids[i], edit))ok=false;else
         if(!Equal(edit.emissive_s, Vec(1)) && edit.emissive_map.is())
         {
            Mems<FileParams> fps=FileParams::Decode(edit.emissive_map);
            Vec mul=edit.emissive_s; if(flt mul_f=mul.avg())edit.emissive_glow/=mul_f;
            if(C TextParam *p=FindTransform(fps, "mulRGB"))mul*=TextVecEx(p->value);
            if(Equal(mul, Vec(1)))DelTransform(fps, "mulRGB");
            else                  SetTransform(fps, "mulRGB", TextVecEx(mul));
            edit.emissive_map=FileParams::Encode(fps); edit.emissive_map_time.now();
            edit.emissive_s=1; edit.emissive_time.now();
            ok&=mtrlSync(elm_ids[i], edit, true, false, "mulTexEmissive");
         }
      }
      return ok;
   }
   void ProjectEx::mtrlMoveToObj(C MemPtr<UID> &elm_ids)
   {
      Memc<Edit::IDParam<UID>> moves;
      FREPA(elm_ids)
      {
       C UID &mtrl_id=elm_ids[i], obj_id=mtrlToObj(mtrl_id); if(obj_id.valid())moves.New()=Edit::IDParam<UID>(mtrl_id, obj_id);
      }
      setElmParent(moves, false, true);
   }
   bool ProjectEx::mtrlGet(C UID &elm_id, EditMaterial &mtrl)
   {
      if(C Elm *elm=findElm(elm_id))
      {
         if(     MtrlEdit.elm_id==elm_id){mtrl=     MtrlEdit.edit; return true;}
         if(WaterMtrlEdit.elm_id==elm_id){mtrl=WaterMtrlEdit.edit; return true;}
         switch(elm->type)
         {
            case ELM_MTRL      : return mtrl.load(editPath(elm_id));
            case ELM_WATER_MTRL:
            {
               EditWaterMtrl water_mtrl; if(!water_mtrl.load(editPath(elm_id)))break;
               mtrl=water_mtrl;
            }return true;
         }
      }
      return false;
   }
   bool ProjectEx::mtrlGet(C UID &elm_id, EditWaterMtrl &mtrl)
   {
      if(C Elm *elm=findElm(elm_id, ELM_WATER_MTRL))
      {
         if(WaterMtrlEdit.elm_id==elm_id){mtrl=WaterMtrlEdit.edit; return true;}
         return mtrl.load(editPath(elm_id));
      }
      return false;
   }
   bool ProjectEx::mtrlSync(C UID &elm_id, C EditMaterial &mtrl, bool reload_textures, bool adjust_params, cptr undo_change_type)
   {
      if(Elm *elm=findElm(elm_id))
      {
         if(MtrlEdit.elm_id==elm_id)
         {
            MtrlEdit.undos.set(undo_change_type);
               TEX_FLAG textures=MtrlEdit.edit.textures(); // get current state of textures before making any change
            if(uint     changed =MtrlEdit.edit.sync(mtrl))
            {
               if(reload_textures)
               {
                  if(changed&EditMaterial::CHANGED_BASE)MtrlEdit.rebuildBase    (textures, changed, adjust_params, true);
                  if(changed&TEXF_DET                 )MtrlEdit.rebuildDetail  ();
                  if(changed&TEXF_MACRO               )MtrlEdit.rebuildMacro   ();
                  if(changed&TEXF_EMISSIVE            )MtrlEdit.rebuildEmissive(textures, adjust_params);
               }else
               {
                  if(changed&(EditMaterial::CHANGED_BASE|TEXF_DET|TEXF_MACRO|TEXF_EMISSIVE))mtrlTexChanged();
               }
               MtrlEdit.toGui();
               MtrlEdit.setChanged();
               D.setShader(MtrlEdit.game());
            }
            return true;
         }
         if(WaterMtrlEdit.elm_id==elm_id)
         {
            WaterMtrlEdit.undos.set(undo_change_type);
               TEX_FLAG textures=WaterMtrlEdit.edit.textures(); // get current state of textures before making any change
            if(uint     changed =WaterMtrlEdit.edit.sync(mtrl))
            {
               if(reload_textures)
               {
                  if(changed&EditMaterial::CHANGED_BASE)WaterMtrlEdit.rebuildBase    (textures, changed, adjust_params, true);
                  if(changed&TEXF_DET                 )WaterMtrlEdit.rebuildDetail  ();
                  if(changed&TEXF_MACRO               )WaterMtrlEdit.rebuildMacro   ();
                  if(changed&TEXF_EMISSIVE            )WaterMtrlEdit.rebuildEmissive(textures, adjust_params);
               }else
               {
                  if(changed&(EditMaterial::CHANGED_BASE|TEXF_DET|TEXF_MACRO|TEXF_EMISSIVE))mtrlTexChanged();
               }
               WaterMtrlEdit.toGui();
               WaterMtrlEdit.setChanged();
             //D.setShader(WaterMtrlEdit.game());
            }
            return true;
         }
         switch(elm->type)
         {
            case ELM_MTRL:
            {
               EditMaterial edit; if(!mtrlGet(elm_id, edit))return false; // load
               TEX_FLAG     old_textures=edit.textures();
               if(uint changed=edit.sync(mtrl)) // if changed anything
               {
                  MaterialPtr game=gamePath(elm_id); if(!game)return false;
                  bool        want_tan_bin=game->needTanBin();
                  TEX_FLAG    has_textures=TEXF_NONE, known_textures=TEXF_NONE;

                  if(reload_textures)
                  {
                     if(changed&EditMaterial::CHANGED_BASE){known_textures|=TEXF_BASE    ; has_textures|=mtrlCreateBaseTextures   (edit, changed);}
                     if(changed&TEXF_DET                 ){known_textures|=TEXF_DET     ; has_textures|=mtrlCreateDetailTexture  (edit);}
                     if(changed&TEXF_MACRO               ){known_textures|=TEXF_MACRO   ; has_textures|=mtrlCreateMacroTexture   (edit);}
                     if(changed&TEXF_EMISSIVE            ){known_textures|=TEXF_EMISSIVE; has_textures|=mtrlCreateEmissiveTexture(edit);}
                  }

                  if(adjust_params)edit.adjustParams(changed, old_textures, has_textures, known_textures);
                  edit.copyTo(*game, T);

                  // save
                  if(ElmMaterial *data=elm->mtrlData()){data->newVer(); data->from(edit);}
                  Save( edit, editPath(elm_id));
                  Save(*game, gamePath(elm_id)); savedGame(*elm);
                  Server.setElmLong(elm_id);

                  // process dependencies
                  if(want_tan_bin!=game->needTanBin())mtrlSetAutoTanBin(elm_id);
                  if(changed&(EditMaterial::CHANGED_BASE|TEXF_DET|TEXF_MACRO|TEXF_EMISSIVE))mtrlTexChanged();
                  D.setShader(game());
               }
            }return true;

            case ELM_WATER_MTRL:
            {
               EditWaterMtrl edit; if(!mtrlGet(elm_id, edit))return false; // load
               TEX_FLAG      old_textures=edit.textures();
               if(uint changed=edit.sync(mtrl)) // if changed anything
               {
                  WaterMtrlPtr game=gamePath(elm_id); if(!game)return false;
                //bool         want_tan_bin=game->needTanBin();
                  TEX_FLAG     has_textures=TEXF_NONE, known_textures=TEXF_NONE;

                  if(reload_textures)
                  {
                     if(changed&EditMaterial::CHANGED_BASE){known_textures|=TEXF_BASE    ; has_textures|=mtrlCreateBaseTextures   (edit, changed);}
                     if(changed&TEXF_DET                 ){known_textures|=TEXF_DET     ; has_textures|=mtrlCreateDetailTexture  (edit);}
                     if(changed&TEXF_MACRO               ){known_textures|=TEXF_MACRO   ; has_textures|=mtrlCreateMacroTexture   (edit);}
                     if(changed&TEXF_EMISSIVE            ){known_textures|=TEXF_EMISSIVE; has_textures|=mtrlCreateEmissiveTexture(edit);}
                  }

                  if(adjust_params)edit.adjustParams(changed, old_textures, has_textures, known_textures);
                  edit.copyTo(*game, T);

                  // save
                  if(ElmWaterMtrl *data=elm->waterMtrlData()){data->newVer(); data->from(edit);}
                  Save( edit, editPath(elm_id));
                  Save(*game, gamePath(elm_id)); savedGame(*elm);
                  Server.setElmLong(elm_id);

                  // process dependencies
                //if(want_tan_bin!=game->needTanBin())mtrlSetAutoTanBin(elm_id);
                  if(changed&(EditMaterial::CHANGED_BASE|TEXF_DET|TEXF_MACRO|TEXF_EMISSIVE))mtrlTexChanged();
                //D.setShader(game());
               }
            }return true;
         }
      }
      return false;
   }
   bool ProjectEx::mtrlSync(C UID &elm_id, C EditWaterMtrl &mtrl, bool reload_textures, bool adjust_params, cptr undo_change_type)
   {
      if(Elm *elm=findElm(elm_id, ELM_WATER_MTRL))
      {
         if(WaterMtrlEdit.elm_id==elm_id)
         {
            WaterMtrlEdit.undos.set(undo_change_type);
               TEX_FLAG textures=WaterMtrlEdit.edit.textures(); // get current state of textures before making any change
            if(uint     changed =WaterMtrlEdit.edit.sync(mtrl))
            {
               if(reload_textures)
               {
                  if(changed&EditMaterial::CHANGED_BASE)WaterMtrlEdit.rebuildBase    (textures, changed, adjust_params, true);
                  if(changed&TEXF_DET                 )WaterMtrlEdit.rebuildDetail  ();
                  if(changed&TEXF_MACRO               )WaterMtrlEdit.rebuildMacro   ();
                  if(changed&TEXF_EMISSIVE            )WaterMtrlEdit.rebuildEmissive(textures, adjust_params);
               }else
               {
                  if(changed&(EditMaterial::CHANGED_BASE|TEXF_DET|TEXF_MACRO|TEXF_EMISSIVE))mtrlTexChanged();
               }
               WaterMtrlEdit.toGui();
               WaterMtrlEdit.setChanged();
             //D.setShader(WaterMtrlEdit.game());
            }
            return true;
         }
         // load
         EditWaterMtrl edit; if(!mtrlGet(elm_id, edit))return false;
         TEX_FLAG      old_textures=edit.textures();
         if(uint changed=edit.sync(mtrl)) // if changed anything
         {
            WaterMtrlPtr game=gamePath(elm_id); if(!game)return false;
          //bool         want_tan_bin=game->needTanBin();
            TEX_FLAG     has_textures=TEXF_NONE, known_textures=TEXF_NONE;

            if(reload_textures)
            {
               if(changed&EditMaterial::CHANGED_BASE){known_textures|=TEXF_BASE    ; has_textures|=mtrlCreateBaseTextures   (edit, changed);}
               if(changed&TEXF_DET                 ){known_textures|=TEXF_DET     ; has_textures|=mtrlCreateDetailTexture  (edit);}
               if(changed&TEXF_MACRO               ){known_textures|=TEXF_MACRO   ; has_textures|=mtrlCreateMacroTexture   (edit);}
               if(changed&TEXF_EMISSIVE            ){known_textures|=TEXF_EMISSIVE; has_textures|=mtrlCreateEmissiveTexture(edit);}
            }

            if(adjust_params)edit.adjustParams(changed, old_textures, has_textures, known_textures);
            edit.copyTo(*game, T);

            // save
            if(ElmWaterMtrl *data=elm->waterMtrlData()){data->newVer(); data->from(edit);}
            Save( edit, editPath(elm_id));
            Save(*game, gamePath(elm_id)); savedGame(*elm);
            Server.setElmLong(elm_id);

            // process dependencies
          //if(want_tan_bin!=game->needTanBin())mtrlSetAutoTanBin(elm_id);
            if(changed&(EditMaterial::CHANGED_BASE|TEXF_DET|TEXF_MACRO|TEXF_EMISSIVE))mtrlTexChanged();
          //D.setShader(game());
         }
         return true;
      }
      return false;
   }
   bool ProjectEx::mtrlSync(C UID &elm_id, C Edit::Material &mtrl, bool reload_textures, bool adjust_params)
   {
      if(Elm *elm=findElm(elm_id))
      {
         if(MtrlEdit.elm_id==elm_id)
         {
            MtrlEdit.undos.set("sync");
               TEX_FLAG textures=MtrlEdit.edit.textures(); // get current state of textures before making any change
            if(uint     changed =MtrlEdit.edit.sync(mtrl))
            {
               if(reload_textures)
               {
                  if(changed&EditMaterial::CHANGED_BASE)MtrlEdit.rebuildBase    (textures, changed, adjust_params, true);
                  if(changed&TEXF_DET                 )MtrlEdit.rebuildDetail  ();
                  if(changed&TEXF_MACRO               )MtrlEdit.rebuildMacro   ();
                  if(changed&TEXF_EMISSIVE            )MtrlEdit.rebuildEmissive(textures, adjust_params);
               }
               MtrlEdit.toGui();
               MtrlEdit.setChanged();
               D.setShader(MtrlEdit.game());
            }
            return true;
         }
         switch(elm->type)
         {
            case ELM_MTRL:
            {
               // load
               EditMaterial edit; if(!mtrlGet(elm_id, edit))return false;
               TEX_FLAG     old_textures=edit.textures();
               if(uint changed=edit.sync(mtrl)) // if changed anything
               {
                  MaterialPtr game=gamePath(elm_id); if(!game)return false;
                  bool        want_tan_bin=game->needTanBin();
                  TEX_FLAG    has_textures=TEXF_NONE, known_textures=TEXF_NONE;

                  if(reload_textures)
                  {
                     if(changed&EditMaterial::CHANGED_BASE){known_textures|=TEXF_BASE    ; has_textures|=mtrlCreateBaseTextures   (edit, changed);}
                     if(changed&TEXF_DET                 ){known_textures|=TEXF_DET     ; has_textures|=mtrlCreateDetailTexture  (edit);}
                     if(changed&TEXF_MACRO               ){known_textures|=TEXF_MACRO   ; has_textures|=mtrlCreateMacroTexture   (edit);}
                     if(changed&TEXF_EMISSIVE            ){known_textures|=TEXF_EMISSIVE; has_textures|=mtrlCreateEmissiveTexture(edit);}
                  }

                  if(adjust_params)edit.adjustParams(changed, old_textures, has_textures, known_textures);
                  edit.copyTo(*game, T);

                  // save
                  if(ElmMaterial *data=elm->mtrlData()){data->newVer(); data->from(edit);}
                  Save( edit, editPath(elm_id));
                  Save(*game, gamePath(elm_id)); savedGame(*elm);
                  Server.setElmLong(elm_id);

                  // process dependencies
                  if(want_tan_bin!=game->needTanBin())mtrlSetAutoTanBin(elm_id);
                  D.setShader(game());
               }
            }return true;
         }
      }
      return false;
   }
   TEX_FLAG ProjectEx::createBaseTextures(Image &base_0, Image &base_1, Image &base_2, C EditMaterial &material, bool changed_flip_normal_y, bool changed_smooth_is_rough)
   {
      MtrlImages mtrl_images;
             mtrl_images.fromMaterial(material, T, changed_flip_normal_y, changed_smooth_is_rough);
      return mtrl_images.createBaseTextures(base_0, base_1, base_2);
   }
   TEX_FLAG ProjectEx::createBaseTextures(Image &base_0, Image &base_1, Image &base_2, C EditWaterMtrl &material, bool changed_flip_normal_y, bool changed_smooth_is_rough)
   {
      MtrlImages mtrl_images;
             mtrl_images.fromMaterial(material, T, changed_flip_normal_y, changed_smooth_is_rough);
      return mtrl_images.createWaterBaseTextures(base_0, base_1, base_2);
   }
   TEX_FLAG ProjectEx::mtrlCreateBaseTextures(EditMaterial &material, uint changed) // 'changed'=only CHANGED_FLIP_NRM_Y or CHANGED_SMOOTH_IS_ROUGH are valid
   {
      // TODO: generating textures when the sources were not found, will reuse existing images, but due to compression, the quality will be lost, and new textures will be generated even though images are the same, this is because BC7->RGBA->BC7 is not the same
      Image      base_0, base_1, base_2;
      TEX_FLAG   textures=createBaseTextures(base_0, base_1, base_2, material, FlagOn(changed, EditMaterial::CHANGED_FLIP_NRM_Y), FlagOn(changed, EditMaterial::CHANGED_SMOOTH_IS_ROUGH));
      UID        old_tex_id;
      IMAGE_TYPE ct;

      // base 0
         old_tex_id =material.base_0_tex; ImageProps(base_0, &material.base_0_tex, &ct, MTRL_BASE_0, material.tex_quality);
      if(old_tex_id!=material.base_0_tex)material.color_map_time.getUTC(); // in order for 'base_0_tex' to sync, a base 0 texture time must be changed, but set it only if the new texture is different #MaterialTextureLayout
      if(base_0.is())
      {
         if(includeTex(material.base_0_tex))
         {
            base_0.copyTry(base_0, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP);
            saveTex(base_0, material.base_0_tex);
         }
         Server.setTex(material.base_0_tex);
      }

      // base 1
         old_tex_id =material.base_1_tex; ImageProps(base_1, &material.base_1_tex, &ct, MTRL_BASE_1);
      if(old_tex_id!=material.base_1_tex)material.normal_map_time.getUTC(); // in order for 'base_1_tex' to sync, a base 1 texture time must be changed, but set it only if the new texture is different #MaterialTextureLayout
      if(base_1.is())
      {
         if(includeTex(material.base_1_tex))
         {
            base_1.copyTry(base_1, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP);
            saveTex(base_1, material.base_1_tex);
         }
         Server.setTex(material.base_1_tex);
      }

      // base 2
         old_tex_id =material.base_2_tex; ImageProps(base_2, &material.base_2_tex, &ct, MTRL_BASE_2);
      if(old_tex_id!=material.base_2_tex)material.smooth_map_time.getUTC(); // in order for 'base_2_tex' to sync, a base 2 texture time must be changed, but set it only if the new texture is different #MaterialTextureLayout
      if(base_2.is())
      {
         if(includeTex(material.base_2_tex))
         {
            base_2.copyTry(base_2, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP);
            saveTex(base_2, material.base_2_tex);
         }
         Server.setTex(material.base_2_tex);
      }

      return textures;
   }
   TEX_FLAG ProjectEx::mtrlCreateBaseTextures(EditWaterMtrl &material, uint changed) // 'changed'=only CHANGED_FLIP_NRM_Y or CHANGED_SMOOTH_IS_ROUGH are valid
   {
      // TODO: generating textures when the sources were not found, will reuse existing images, but due to compression, the quality will be lost, and new textures will be generated even though images are the same, this is because BC7->RGBA->BC7 is not the same
      Image      base_0, base_1, base_2;
      TEX_FLAG   textures=createBaseTextures(base_0, base_1, base_2, material, FlagOn(changed, EditMaterial::CHANGED_FLIP_NRM_Y), FlagOn(changed, EditMaterial::CHANGED_SMOOTH_IS_ROUGH));
      UID        old_tex_id;
      IMAGE_TYPE ct;

      // base 0
         old_tex_id =material.base_0_tex; ImageProps(base_0, &material.base_0_tex, &ct, MTRL_BASE_0|WATER_MTRL, material.tex_quality);
      if(old_tex_id!=material.base_0_tex)material.color_map_time.getUTC(); // in order for 'base_0_tex' to sync, a base 0 texture time must be changed, but set it only if the new texture is different #MaterialTextureLayoutWater
      if(base_0.is())
      {
         if(includeTex(material.base_0_tex))
         {
            base_0.copyTry(base_0, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP);
            saveTex(base_0, material.base_0_tex);
         }
         Server.setTex(material.base_0_tex);
      }

      // base 1
         old_tex_id =material.base_1_tex; ImageProps(base_1, &material.base_1_tex, &ct, MTRL_BASE_1|WATER_MTRL);
      if(old_tex_id!=material.base_1_tex)material.normal_map_time.getUTC(); // in order for 'base_1_tex' to sync, a base 1 texture time must be changed, but set it only if the new texture is different #MaterialTextureLayoutWater
      if(base_1.is())
      {
         if(includeTex(material.base_1_tex))
         {
            base_1.copyTry(base_1, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP);
            saveTex(base_1, material.base_1_tex);
         }
         Server.setTex(material.base_1_tex);
      }

      // base 2
         old_tex_id =material.base_2_tex; ImageProps(base_2, &material.base_2_tex, &ct, MTRL_BASE_2|WATER_MTRL);
      if(old_tex_id!=material.base_2_tex)material.smooth_map_time.getUTC(); // in order for 'base_2_tex' to sync, a base 2 texture time must be changed, but set it only if the new texture is different #MaterialTextureLayoutWater
      if(base_2.is())
      {
         if(includeTex(material.base_2_tex))
         {
            base_2.copyTry(base_2, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP);
            saveTex(base_2, material.base_2_tex);
         }
         Server.setTex(material.base_2_tex);
      }

      return textures;
   }
   TEX_FLAG ProjectEx::mtrlCreateDetailTexture(EditMaterial &material)
   {
      TEX_FLAG textures=TEXF_NONE;
      // !! here order of loading images is important, because we pass pointers to those images in subsequent loads !!
      TextParam color_resize, smooth_resize, bump_resize, normal_resize;
      MtrlImages::ImageResize color, smooth, bump, normal; // #MaterialTextureLayoutDetail
      bool color_ok=loadImages( color, & color_resize, material.detail_color , true , false, GREY                                                                                     ), // load before 'smooth', here 'color' 'smooth' 'bump' are not yet available #MaterialTextureLayoutDetail
          smooth_ok=loadImages(smooth, &smooth_resize, material.detail_smooth, false, false, GREY                , &color, &color_resize, null   , null          , null , null        ), // load before 'bump'  , here         'smooth' 'bump' are not yet available #MaterialTextureLayoutDetail
            bump_ok=loadImages(  bump, &  bump_resize, material.detail_bump  , false, false, GREY                , &color, &color_resize, &smooth, &smooth_resize, null , null        ), // load before 'normal', here                  'bump' is  not yet available #MaterialTextureLayoutDetail
          normal_ok=loadImages(normal, &normal_resize, material.detail_normal, false, false, Color(128, 128, 255), &color, &color_resize, &smooth, &smooth_resize, &bump, &bump_resize);

      // process resize
      if( color_ok) color.setFrom( color_resize);
      if(smooth_ok)smooth.setFrom(smooth_resize);
      if(  bump_ok)  bump.setFrom(  bump_resize);
      if(normal_ok)normal.setFrom(normal_resize);

      if(!bump_ok && !material.detail_normal.is())normal_ok=false; // if bump map failed to load, and there is no dedicated normal map, and since it's possible that normal was created from the bump, which is not available, so normal needs to be marked as failed

      if(color_ok || smooth_ok || bump_ok || normal_ok) // proceed only if succeeded with setting anything, this is to avoid clearing existing texture when all failed to load, continue if at least one succeeded, in case the image is different while others will be extracted from old version
      {
                                ExtractDetailTextures(T, material.detail_tex, color_ok ? null : &color, bump_ok ? null : &bump, normal_ok ? null : &normal, smooth_ok ? null : &smooth);
         Image  detail; textures=CreateDetailTexture (detail, color, bump, normal, smooth);
         IMAGE_TYPE ct;                    ImageProps(detail, &material.detail_tex, &ct, MTRL_DETAIL); material.detail_map_time.getUTC(); // in order for 'detail_tex' to sync, 'detail_map_time' time must be changed
         if(detail.is())
         {
            if(includeTex(material.detail_tex))
            {
               detail.copyTry(detail, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP);
               saveTex(detail, material.detail_tex);
            }
            Server.setTex(material.detail_tex);
         }
      }
      return textures;
   }
   TEX_FLAG ProjectEx::mtrlCreateMacroTexture(EditMaterial &material)
   {
      Image macro; if(loadImages(macro, null, material.macro_map, true)) // proceed only if loaded ok
      {
         macro.resize(NearestPow2(macro.w()), NearestPow2(macro.h()), FILTER_BEST, IC_WRAP);
         IMAGE_TYPE ct; ImageProps(macro, &material.macro_tex, &ct, MTRL_MACRO); material.macro_map_time.getUTC(); // in order for 'macro_tex' to sync, 'macro_map_time' time must be changed
         if(macro.is())
         {
            if(includeTex(material.macro_tex))
            {
               SetFullAlpha(macro, ct);
               macro.copyTry(macro, -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, IC_WRAP);
               saveTex(macro, material.macro_tex);
            }
            Server.setTex(material.macro_tex);
            return TEXF_MACRO;
         }
      }
      return TEXF_NONE;
   }
   TEX_FLAG ProjectEx::mtrlCreateEmissiveTexture(EditMaterial &material)
   {
      Image emissive; if(loadImages(emissive, null, material.emissive_map, true)) // proceed only if loaded ok
      {
         emissive.resize(NearestPow2(emissive.w()), NearestPow2(emissive.h()));
         IMAGE_TYPE ct; ImageProps(emissive, &material.emissive_tex, &ct, MTRL_EMISSIVE); material.emissive_map_time.getUTC(); // in order for 'emissive_tex' to sync, 'emissive_map_time' time must be changed
         if(emissive.is())
         {
            if(includeTex(material.emissive_tex))
            {
               SetFullAlpha(emissive, ct);
               emissive.copyTry(emissive, -1, -1, -1, ct, IMAGE_2D, 0);
               saveTex(emissive, material.emissive_tex);
            }
            Server.setTex(material.emissive_tex);
            return TEXF_EMISSIVE;
         }
      }
      return TEXF_NONE;
   }
   bool ProjectEx::mtrlReloadTextures(C UID &elm_id, bool base, bool detail, bool macro, bool emissive)
   {
      if(!base && !detail && !macro && !emissive)return true; // nothing to reload
      if(Elm *elm=findElm(elm_id))
      {
         if(MtrlEdit.elm_id==elm_id)
         {
            MtrlEdit.undos.set("EI");
            if(base    )MtrlEdit.rebuildBase    (TEXF_NONE, 0, false, true);
            if(detail  )MtrlEdit.rebuildDetail  ();
            if(macro   )MtrlEdit.rebuildMacro   ();
            if(emissive)MtrlEdit.rebuildEmissive(TEXF_NONE, false);
            return true;
         }
         if(WaterMtrlEdit.elm_id==elm_id)
         {
            WaterMtrlEdit.undos.set("EI");
            if(base    )WaterMtrlEdit.rebuildBase    (TEXF_NONE, 0, false, true);
            if(detail  )WaterMtrlEdit.rebuildDetail  ();
            if(macro   )WaterMtrlEdit.rebuildMacro   ();
            if(emissive)WaterMtrlEdit.rebuildEmissive(TEXF_NONE, false);
            return true;
         }
         switch(elm->type)
         {
            case ELM_MTRL:
            {
               // load
               EditMaterial edit; if(!mtrlGet(elm_id, edit))return false;
               MaterialPtr  game=gamePath(elm_id); if(!game)return false;
               bool         want_tan_bin=game->needTanBin();

               // reload
               if(base    )mtrlCreateBaseTextures   (edit);
               if(detail  )mtrlCreateDetailTexture  (edit);
               if(macro   )mtrlCreateMacroTexture   (edit);
               if(emissive)mtrlCreateEmissiveTexture(edit);

               // save because texture ID's have been changed
               if(ElmMaterial *data=elm->mtrlData()){data->newVer(); data->from(edit);}
               Save( edit, editPath(elm_id)); edit.copyTo(*game, T);
               Save(*game, gamePath(elm_id)); savedGame(*elm);
               Server.setElmLong(elm_id);

               // process dependencies
               if(want_tan_bin!=game->needTanBin())mtrlSetAutoTanBin(elm_id);
               D.setShader(game());
            }return true;

            case ELM_WATER_MTRL:
            {
               // load
               EditWaterMtrl edit; if(!edit.load(editPath(elm_id)))return false;
               WaterMtrlPtr  game=gamePath(elm_id); if(!game)return false;

               // reload
               if(base    )mtrlCreateBaseTextures   (edit);
               if(detail  )mtrlCreateDetailTexture  (edit);
               if(macro   )mtrlCreateMacroTexture   (edit);
               if(emissive)mtrlCreateEmissiveTexture(edit);

               // save because texture ID's have been changed
               if(ElmWaterMtrl *data=elm->waterMtrlData()){data->newVer(); data->from(edit);}
               Save( edit, editPath(elm_id)); edit.copyTo(*game, T);
               Save(*game, gamePath(elm_id)); savedGame(*elm);
               Server.setElmLong(elm_id);
            }return true;
         }
      }
      return false;
   }
   void ProjectEx::mtrlReloadTextures(C MemPtr<UID> &elm_ids, bool base, bool detail, bool macro, bool emissive)
   {
      FREPA(elm_ids)mtrlReloadTextures(elm_ids[i], base, detail, macro, emissive);
   }
   bool ProjectEx::skelGet(C UID &elm_id, Skeleton &skel)C
   {
      if(C Elm *elm=findElm(elm_id, ELM_SKEL))
      {
         if(ObjEdit.skel_elm==elm)
         {
            if(ObjEdit.mesh_skel)skel=*ObjEdit.mesh_skel;else skel.del();
            return true;
         }
         Str file=gamePath(elm_id);
         if(C Skeleton *cached=Skeletons.find(file)){skel=*cached; return true;}
         return skel.load(file);
      }
      skel.del(); return !elm_id.valid();
   }
   Animation* ProjectEx::getAnim(C UID &elm_id, Animation &temp)C
   {
      Str file=gamePath(elm_id);
      Animation *anim=Animations.find(file);
      if(!anim){anim=&temp; anim->load(file);}
      return anim;
   }
   bool ProjectEx::animGet(C UID &elm_id, Animation &anim)C
   {
      if(C Elm *elm=findElm(elm_id, ELM_ANIM))return anim.load(gamePath(*elm));
      return false;
   }
   bool ProjectEx::animSet(C UID &elm_id, C Animation &anim)
   {
      if(Elm *elm=findElm(elm_id))
      if(ElmAnim *anim_data=elm->animData())
      {
         anim_data->newVer();
         anim_data->file_time.getUTC();
         anim_data->linear(anim.linear()).linear_time.getUTC();
         anim_data->loop  (anim.loop  ()).  loop_time.getUTC();
         Save(anim, gamePath(elm_id)); savedGame(*elm);
         Server.setElmLong(elm_id);
         if(AnimEdit.elm==elm)AnimEdit.toGui();
         return true;
      }
      return false;
   }
   void ProjectEx::setAnimParams(Elm &elm_anim)
   {
      if(ElmAnim *anim_data=elm_anim.animData())
         if(Animation *anim=Animations.get(gamePath(elm_anim.id)))
            if(anim->loop  ()!=anim_data->loop  ()
            || anim->linear()!=anim_data->linear())
      {
         anim->loop(anim_data->loop()).linear(anim_data->linear());
         Save(*anim, gamePath(elm_anim.id)); savedGame(elm_anim);
      }
   }
   void ProjectEx::animClip(C MemPtr<UID> &elm_ids)
   {
      REPA(elm_ids)
      if(Elm *anim=findElm(elm_ids[i], ELM_ANIM))
      if(ElmAnim *anim_data=anim->animData())
      {
         Animation temp, *animation=getAnim(anim->id, temp);
         anim_data->newVer();
         anim_data->file_time.getUTC();
         animation->clipAuto();
         Save(*animation, gamePath(anim->id)); savedGame(*anim);
         Server.setElmLong(anim->id);
      }
   }
   void ProjectEx::animLinear(C MemPtr<UID> &elm_ids, bool linear)
   {
      REPA(elm_ids)
      if(Elm *anim=findElm(elm_ids[i], ELM_ANIM))
      if(ElmAnim *anim_data=anim->animData())
      if(anim_data->linear()!=linear)
      {
         Animation temp, *animation=getAnim(anim->id, temp);
         anim_data->newVer();
         anim_data->file_time.getUTC();
         anim_data->linear(linear);
         animation->linear(linear);
         Save(*animation, gamePath(anim->id)); savedGame(*anim);
         Server.setElmLong(anim->id);
         if(AnimEdit.elm==anim)AnimEdit.toGui();
      }
   }
   void ProjectEx::animLoop(C MemPtr<UID> &elm_ids, bool loop)
   {
      REPA(elm_ids)
      if(Elm *anim=findElm(elm_ids[i], ELM_ANIM))
      if(ElmAnim *anim_data=anim->animData())
      if(anim_data->loop()!=loop)
      {
         Animation temp, *animation=getAnim(anim->id, temp);
         anim_data->newVer();
         anim_data->file_time.getUTC();
         anim_data->loop(loop);
         animation->loop(loop);
         Save(*animation, gamePath(anim->id)); savedGame(*anim);
         Server.setElmLong(anim->id);
         if(AnimEdit.elm==anim)AnimEdit.toGui();
      }
   }
   void ProjectEx::animSetTargetSkel(C MemPtr<UID> &anim_ids, C UID &skel_id)
   {
      bool changed=false;
      Elm      *skel_elm=findElm(skel_id, ELM_SKEL); UID sid=(skel_elm ? skel_elm->id : UIDZero);
      Skeleton *skel; if(RenameAnimBonesOnSkelChange){skel=Skeletons(gamePath(sid)); if(ObjEdit.skel_elm==skel_elm)ObjEdit.flushMeshSkel();} // !! if this skeleton is currently edited, then we have to flush it, for example, it could have been empty at the start and we've just added bones, so 'new_skel' is empty and bones are only in 'ObjEdit.mesh_skel_temp' memory, and before changing 'anim_data.skel_id' !!
      REPA(anim_ids)
      if(Elm *anim=findElm(anim_ids[i]))
      if(ElmAnim *anim_data=anim->animData())
      if(anim_data->skel_id!=sid)
      {
         changed=true;
         anim_data->newVer();
         anim_data->skel_id=sid;
         anim_data->skel_time.getUTC();
         if(skel_elm)anim_data->transform=skel_elm->skelData()->transform;
         bool file_changed=false;
         if(RenameAnimBonesOnSkelChange && skel)
         {
            Animation temp, *animation=getAnim(anim->id, temp);
            if(animation->setBoneNameTypeIndexesFromSkeleton(*skel))
            {
               file_changed=true;
               anim_data->file_time.getUTC();
               Save(*animation, gamePath(anim->id)); savedGame(*anim);
            }
         }
         if(file_changed)Server.setElmLong(anim->id);else Server.setElmShort(anim->id);
         if(AnimEdit.elm==anim)
         {
            AnimEdit.setMeshSkel();
            AnimEdit.toGui();
         }
      }
      if(changed)refresh(false, false);
   }
   void ProjectEx::transformSet(C MemPtr<UID> &elm_ids)
   {
      if(!ObjEdit.obj_elm)Gui.msgBox(S, "No object is currently opened");else
      {
         Pose pose=PoseIdentity; if(ObjEdit.mesh_elm)if(ElmMesh *mesh_data=ObjEdit.mesh_elm->meshData())pose=mesh_data->transform;
         REPA(elm_ids)
         if(Elm *obj =findElm(elm_ids[i]      ))if(ElmObj  * obj_data=obj -> objData())
         if(Elm *mesh=findElm(obj_data->mesh_id))if(ElmMesh *mesh_data=mesh->meshData())if(mesh_data->canHaveCustomTransform())
         if(ObjEdit.mesh_elm!=mesh)
         if(mesh_data->transform!=pose)
         {
            mesh_data->transform=pose; mesh_data->transform_time.getUTC(); mesh_data->newVer();
            makeGameVer(*mesh);
            meshTransformChanged(*mesh);
            Server.setElmShort(mesh->id);
         }
      }
   }
   void ProjectEx::transformApply(C MemPtr<UID> &elm_ids)
   {
      if(!ObjEdit.obj_elm                  )Gui.msgBox(S, "No object is currently opened");else
      if( ObjEdit.mode()!=ObjView::TRANSFORM)Gui.msgBox(S, "Transform mode is not enabled");else
      if( ObjEdit.trans.trans==PoseIdentity)Gui.msgBox(S, "Transformation is empty"      );else
      {
         Pose trans=ObjEdit.trans.trans;
         REPA(elm_ids)
         if(Elm *obj =findElm(elm_ids[i]      ))if(ElmObj  * obj_data=obj -> objData())
         if(Elm *mesh=findElm(obj_data->mesh_id))if(ElmMesh *mesh_data=mesh->meshData())if(mesh_data->canHaveCustomTransform())
         if(ObjEdit.mesh_elm==mesh)ObjEdit.trans.apply();else
         {
            mesh_data->transform*=trans; mesh_data->transform_time.getUTC(); mesh_data->newVer();
            makeGameVer(*mesh);
            meshTransformChanged(*mesh);
            Server.setElmShort(mesh->id);
         }
      }
   }
   void ProjectEx::transformApply(C MemPtr<UID> &elm_ids, C Matrix &matrix)
   {
      REPA(elm_ids)
      if(Elm *obj =findElm(elm_ids[i]      ))if(ElmObj  * obj_data=obj -> objData())
      if(Elm *mesh=findElm(obj_data->mesh_id))if(ElmMesh *mesh_data=mesh->meshData())if(mesh_data->canHaveCustomTransform())
      {
         mesh_data->transform*=matrix; mesh_data->transform_time.getUTC(); mesh_data->newVer();
         makeGameVer(*mesh);
         meshTransformChanged(*mesh);
         Server.setElmShort(mesh->id);
      }
   }
   void ProjectEx::transformBottom(C MemPtr<UID> &elm_ids)
   {
      REPA(elm_ids)
      if(Elm *obj =findElm(elm_ids[i]      ))if(ElmObj  * obj_data=obj -> objData())
      if(Elm *mesh=findElm(obj_data->mesh_id))if(ElmMesh *mesh_data=mesh->meshData())if(mesh_data->canHaveCustomTransform())
      if(mesh_data->box.valid())
      {
         Vec offset(0, -mesh_data->box.min.y, 0);
         offset/=mesh_data->transform().orn();
         mesh_data->transform+=offset; mesh_data->transform_time.getUTC(); mesh_data->newVer();
         makeGameVer(*mesh);
         meshTransformChanged(*mesh);
         Server.setElmShort(mesh->id);
      }
   }
   void ProjectEx::transformBack(C MemPtr<UID> &elm_ids)
   {
      REPA(elm_ids)
      if(Elm *obj =findElm(elm_ids[i]      ))if(ElmObj  * obj_data=obj -> objData())
      if(Elm *mesh=findElm(obj_data->mesh_id))if(ElmMesh *mesh_data=mesh->meshData())if(mesh_data->canHaveCustomTransform())
      if(mesh_data->box.valid())
      {
         Vec offset(0, 0, -mesh_data->box.min.z);
         offset/=mesh_data->transform().orn();
         mesh_data->transform+=offset; mesh_data->transform_time.getUTC(); mesh_data->newVer();
         makeGameVer(*mesh);
         meshTransformChanged(*mesh);
         Server.setElmShort(mesh->id);
      }
   }
   void ProjectEx::transformCenter(C MemPtr<UID> &elm_ids)
   {
      REPA(elm_ids)
      if(Elm *obj =findElm(elm_ids[i]      ))if(ElmObj  * obj_data= obj-> objData())
      if(Elm *mesh=findElm(obj_data->mesh_id))if(ElmMesh *mesh_data=mesh->meshData())if(mesh_data->canHaveCustomTransform())
      if(mesh_data->box.valid())
      {
         Vec offset=-mesh_data->box.center();
         offset/=mesh_data->transform().orn();
         mesh_data->transform+=offset; mesh_data->transform_time.getUTC(); mesh_data->newVer();
         makeGameVer(*mesh);
         meshTransformChanged(*mesh);
         Server.setElmShort(mesh->id);
      }
   }
   void ProjectEx::transformCenterXZ(C MemPtr<UID> &elm_ids)
   {
      REPA(elm_ids)
      if(Elm *obj =findElm(elm_ids[i]      ))if(ElmObj  * obj_data=obj -> objData())
      if(Elm *mesh=findElm(obj_data->mesh_id))if(ElmMesh *mesh_data=mesh->meshData())if(mesh_data->canHaveCustomTransform())
      if(mesh_data->box.valid())
      {
         Vec offset=-mesh_data->box.center().x0z();
         offset/=mesh_data->transform().orn();
         mesh_data->transform+=offset; mesh_data->transform_time.getUTC(); mesh_data->newVer();
         makeGameVer(*mesh);
         meshTransformChanged(*mesh);
         Server.setElmShort(mesh->id);
      }
   }
   void ProjectEx::transformRotYMinBox(C MemPtr<UID> &elm_ids)
   {
      Mesh edit; Memt<Vec2> vtxs;
      REPA(elm_ids)
      if(Elm *obj =findElm(elm_ids[i]      ))if(ElmObj  * obj_data=obj -> objData())
      if(Elm *mesh=findElm(obj_data->mesh_id))if(ElmMesh *mesh_data=mesh->meshData())if(mesh_data->canHaveCustomTransform() && mesh_data->box.valid() && Load(edit, editPath(*mesh), game_path))
      {
         Matrix matrix=mesh_data->transform();
       C MeshLod &lod=edit; vtxs.reserve(lod.vtxs()); REPA(lod)
         {
          C MeshPart &part=lod.parts[i]; if(!(part.part_flag&MSHP_HIDDEN))if(C Vec *src=part.base.vtx.pos())
            {
               int vs=part.base.vtxs();
               Vec2 *dest=&vtxs[vtxs.addNum(vs)];
               REP(vs)dest[i]=(src[i]*matrix.orn()).xz();
            }
         }
         Vec2 axis; if(BestFit(vtxs.data(), vtxs.elms(), axis))
         {
            flt a=Angle(axis); a=Frac(a, PI_2); if(a>EPS && a<PI_2-EPS)
            {
               if(a>PI_4)a-=PI_2;
               mesh_data->transform=matrix.rotateY(a);
               mesh_data->transform_time.getUTC(); mesh_data->newVer();
               makeGameVer(*mesh);
               meshTransformChanged(*mesh);
               Server.setElmShort(mesh->id);
            }
         }
         vtxs.clear();
      }
   }
   void ProjectEx::objSetBody(C MemPtr<UID> &elm_ids, C UID &body_id)
   {
      if(elm_ids.elms())
      {
         Elm *body=findElm(body_id, ELM_MESH);
         if( !body_id.valid() || body)
         {
            REPA(elm_ids)
            if(Elm *obj =findElm(elm_ids[i]      ))if(ElmObj  * obj_data=obj -> objData())
            if(Elm *mesh=findElm(obj_data->mesh_id))if(ElmMesh *mesh_data=mesh->meshData())
            {
             C UID &actual_body_id=((mesh->id==body_id) ? UIDZero : body_id); // if trying to set body to self, then clear it instead
               if(mesh_data->body_id!=actual_body_id)
               {
                  if(obj==ObjEdit.obj_elm)ObjEdit.setBody(actual_body_id);else
                  {
                     mesh_data->newVer();
                     mesh_data->body_id=actual_body_id;
                     mesh_data->body_time.getUTC();
                     meshTransformChanged(*mesh, true);
                     Server.setElmShort(mesh->id);
                  }
               }
            }
            refresh(false, false);
         }
      }
   }
   void ProjectEx::soundImportAs(C MemPtr<UID> &elm_ids, SOUND_CODEC codec, int rel_bit_rate)
   {
      Memt<UID> reloading;
      FREPA(elm_ids)if(Elm *elm=findElm(elm_ids[i], ELM_SOUND))
      {
         Mems<FileParams> files=FileParams::Decode(elm->srcFile()); if(files.elms())
         {
            FileParams &file=files[0];
            if(codec          )file.getParam("codec"     ).setValue((codec==SOUND_WAV) ? "raw" : CodecName(codec));else file.params.removeData(file.findParam("codec"     ), true);
            if(rel_bit_rate>=0)file.getParam("relBitRate").setValue(rel_bit_rate                                 );else file.params.removeData(file.findParam("relBitRate"), true); file.params.removeData(file.findParam("relativeBitRate"), true);
            elm->setSrcFile(FileParams::Encode(files)); Server.setElmShort(elm->id); reloading.add(elm->id);
         }
      }
      elmReload(reloading);
   }
   void ProjectEx::mulSoundVolume(C MemPtr<UID> &elm_ids, flt volume)
   {
      if(!Equal(volume, 1) && volume>=0)
      {
         Memt<UID> changed;
         FREPA(elm_ids)if(Elm *elm=findElm(elm_ids[i], ELM_SOUND))
         {
            Mems<FileParams> fps=FileParams::Decode(elm->srcFile());

            flt v=volume;
            if(C TextParam *p=FindTransform(fps, "volume"))v*=p->asFlt();
            if(Equal(v, 1)    )DelTransform(fps, "volume");
            else               SetTransform(fps, "volume", TextReal(v, -3));

            elm->setSrcFile(FileParams::Encode(fps)); Server.setElmShort(elm->id); changed.add(elm->id);
         }
         elmReload(changed);
      }
   }
   void ProjectEx::adjustAnimations(C UID &skel_id, C EditSkeleton &old_edit_skel, C Skeleton &old_skel, C Skeleton &new_skel, C MemPtr<Mems<IndexWeight>> &bone_weights, int old_bone_as_root)
   {
      int difference=0; // 0=none, 1=only name/types, 2=full
      MemtN<int, 256> old_to_new;
      if(old_bone_as_root>=0){full_difference: difference=2;}else
      {
         old_to_new.setNum(old_skel.bones.elms()); // no need to initialize to -1 because all will be set in the loop and used only if "difference==1"
         REPAD(ob, old_skel.bones) // check only old bones, because we have animations only for old bones
         {
          C SkelBone &old_bone=old_skel.bones[ob];
            int new_bone_i=-1; REPD(nb, Min(bone_weights.elms(), new_skel.bones.elms()))
            {
             C Mems<IndexWeight> &weights=bone_weights[nb];
               if(weights.elms()>1)goto full_difference; // if there's any new bone with more than 1 weight, then we have a full difference
               if(weights.elms() && weights[0].index==ob)
               {
                  if(new_bone_i>=0)goto full_difference; // if there was already another new bone referencing this old bone, then we have a full difference
                  new_bone_i=nb;
               }
            }
            if(new_bone_i<0)goto full_difference; // bone not found = full difference
          C SkelBone &new_bone=new_skel.bones[new_bone_i];

            if( Dot  (old_bone.dir , new_bone.dir )<0.9999f            // orientation was changed
            ||  Dot  (old_bone.perp, new_bone.perp)<0.9999f
            || !Equal(old_bone.pos , new_bone.pos )                   // position    was changed
            ||       (old_bone.parent==0xFF)!=(new_bone.parent==0xFF) // one has parent, but the other one doesn't
            )goto full_difference; // full difference

            if(old_bone.parent!=0xFF /*&& new_bone.parent!=0xFF - we've already checked if one has parent, but the other one doesn't*/) // if both have parents
            {
               if(!InRange(new_bone.parent, bone_weights))goto full_difference; // can't access new parent
             C Mems<IndexWeight> &new_parent=bone_weights[new_bone.parent];
               if(new_parent.elms()!=1 || new_parent[0].index!=old_bone.parent)goto full_difference; // if new parent doesn't point to old parent
            }

            if(!Equal(old_bone.name      , new_bone.name     ) // name       was changed
            ||        old_bone.type      !=new_bone.type       // type       was changed
            ||        old_bone.type_index!=new_bone.type_index // type_index was changed
            ||        old_bone.type_sub  !=new_bone.type_sub   // type_sub   was changed
            )difference=1; // set difference but keep checking if we find bigger

            old_to_new[ob]=new_bone_i;
         }
      }
      if(difference) // if there's any difference
         REPA(elms) // iterate all elements
      {
         Elm &anim_elm=elms[i]; if(ElmAnim *anim_data=anim_elm.animData())if(anim_data->skel_id==skel_id) // process animations using this skeleton
         {
            Animation temp, *anim=getAnim(anim_elm.id, temp);
            if(anim->is()) // process only if has any data (this also skips animations that haven't finished downloading from the server)
            {
               if(difference==1) // if difference is only in name/types
                  REPA(anim->bones)
               {
                  AnimBone &bone=anim->bones[i];
                  int   old_bone_i=(RenameAnimBonesOnSkelChange ? old_skel.findBoneI(bone.name)                                              // if we always adjust anim bone names then we can use this
                                                                : old_skel.findBoneI(bone.name, bone.type, bone.type_index, bone.type_sub)); // use types in case animation was from another skeleton and we haven't adjusted types, however avoid this because it can break in rare circumstances when for some reason 'old_skel' had 'setBoneTypes' called
                  if(InRange(old_bone_i, old_to_new))
                  {
                     int new_bone_i=old_to_new[old_bone_i]; if(InRange(new_bone_i, new_skel.bones)){bone.id()=new_skel.bones[new_bone_i]; continue;} // continue so we don't remove this bone
                  }
                  anim->bones.remove(i, true);
               }else // have to perform full adjustment
               {
                  anim->adjustForSameTransformWithDifferentSkeleton(old_skel, new_skel, old_bone_as_root, bone_weights, old_bone_as_root>=0 ? (anim_data->rootFlags()|(old_edit_skel.boneRootZero(old_bone_as_root) ? 0 : ROOT_BONE_POSITION|ROOT_START_IDENTITY)) : 0); // here we've modified skeleton and adjust animations based on skel changes, modify root only if we've potentially selected bone as root, in other cases preserve existing anim root keys, because for example if root was manually modified to start at non-identity (for reversed animations), and we would request ROOT_START_IDENTITY then root would get changed breaking custom setup
                  anim->optimize(); // 'optimize' after 'adjustForSameTransformWithDifferentSkeleton' because it may generate lot of keyframes
               }

               anim_data->newVer();
               anim_data->file_time.getUTC();
               Save(*anim, gamePath(anim_elm.id)); savedGame(anim_elm);
               Server.setElmLong(anim_elm.id);
            }
         }
      }
   }
   void ProjectEx::offsetAnimations(C Skeleton &old_skel, C Skeleton &new_skel, C UID &skel_id, C UID &ignore_anim_id)
   {
      REPA(elms) // iterate all project elements
      {
         Elm &anim_elm=elms[i]; if(ElmAnim *anim_data=anim_elm.animData())if(anim_data->skel_id==skel_id && anim_elm.id!=ignore_anim_id) // process animations using this skeleton
         {
            Animation temp, *anim=getAnim(anim_elm.id, temp);
            if(anim->is()) // process if has any data (this also skips animations that haven't finished downloading from the server)
            {
               anim->adjustForSameSkeletonWithDifferentPose(old_skel, new_skel);
               anim_data->newVer();
               anim_data->file_time.getUTC();
               Save(*anim, gamePath(anim_elm.id)); savedGame(anim_elm);
               Server.setElmLong(anim_elm.id);
            }
         }
      }
   }
   bool ProjectEx::validElm(Elm &elm)
   {
      if(elm.type)
         if(elm.type==ELM_FOLDER || elm.type==ELM_LIB || elm.initialized())
      {
         if(elm.type==ELM_WORLD)return elm.worldData()->valid();
         if(elm.type==ELM_OBJ)
         {
            ElmObj *data=elm.objData();
            if(Elm *mesh=findElm(data->mesh_id))if(!validElm(*mesh))return false;
         }
         if(elm.type==ELM_MESH)
         {
            ElmMesh *data=elm.meshData();
            if(Elm *skel=findElm(data->skel_id))if(!validElm(*skel))return false;
            if(Elm *phys=findElm(data->phys_id))if(!validElm(*phys))return false;
         }
         return true;
      }
      return false;
   }
   void ProjectEx::duplicate(Memc<UID> &ids, MemPtr<UID> duplicated, C Str &suffix)
   {
      clearListSel();
      bool manually_by_user=false;
      Memc<UID> temp; if(!duplicated){duplicated.point(temp); manually_by_user=true;} duplicated.clear();
      FREPA(ids) // !! process in order, this is important so 'duplicated' matches the 'ids' !!
         if(Elm *src=findElm(ids[i]))if(validElm(*src))
      {
         if(src->type!=ELM_CODE)flushElm(src->id);
         Elm &dup=super::newElm(); dup.type=src->type; // set 'type' before calling 'copyParams'
         dup.copyParams(*src).setName(src->name+suffix); // call 'setName' after 'copyParams'
         duplicated.add(dup.id);
         switch(src->type)
         {
            case ELM_OBJ_CLASS:
            {
               EditObject params; params.load(editPath(*src)); params.setType(true, dup.id, edit_path); Save(params, editPath(dup)); makeGameVer(dup);
            }break;

            case ELM_CODE:
            {
               Str code; codeGet(src->id, code); SaveCode(code, codePath(dup));
            }break;

            case ELM_OBJ:
               if(ElmObj *src_obj_data=src->objData())
               if(ElmObj *dup_obj_data=dup.objData())
            {
               File f; if(f.readTry(editPath(*src)))SafeOverwrite(f, editPath(dup)); // copy obj edit file
               if(Elm *src_mesh=findElm(src_obj_data->mesh_id)) // get source mesh
               {
                  if(Elm *dup_mesh=getObjMeshElm(dup.id, false, false)) // grab the duplicated mesh, but don't send to the server yet
                  {
                     flushElm(src_mesh->id); // flush it first
                     dup_obj_data->mesh_id=dup_mesh->id;
                     dup_mesh->copyParams(*src_mesh, false); // copy params
                     if(f.readTry(editPath(*src_mesh)) && SafeOverwrite(f, editPath(*dup_mesh)))SavedMesh(editPath(*dup_mesh)); // copy mesh edit file
                     if(ElmMesh *src_mesh_data=src_mesh->meshData())
                     if(ElmMesh *dup_mesh_data=dup_mesh->meshData())
                     {
                        dup_mesh_data->obj_id=dup.id; // this will get cleared in 'copyParams', so reassign again
                        if(Elm *src_skel=findElm(src_mesh_data->skel_id)) // get source skel
                        {
                           if(Elm *dup_skel=getObjSkelElm(dup.id, false, false)) // grab the duplicated skeleton, but don't send to the server yet
                           {
                              flushElm(src_skel->id); // flush it first
                              dup_skel->copyParams(*src_skel, false); // copy params
                              dup_skel->skelData()->mesh_id=dup_mesh->id; // this will get cleared in 'copyParams', so reassign again
                              if(f.readTry(editPath(*src_skel)) && SafeOverwrite(f, editPath(*dup_skel))) SavedEditSkel(editPath(*dup_skel)); // copy skel edit file
                              if(f.readTry(gamePath(*src_skel)) && SafeOverwrite(f, gamePath(*dup_skel))){SavedSkel    (gamePath(*dup_skel)); savedGame(*dup_skel);} // copy skel game file
                              Server.setElmFull(dup_skel->id); // send skel to server
                           }
                        }
                        if(Elm *src_phys=findElm(src_mesh_data->phys_id)) // get source phys
                        {
                           if(Elm *dup_phys=getObjPhysElm(dup.id, false, false)) // grab the duplicated phys body, but don't send to the server yet
                           {
                              flushElm(src_phys->id); // flush it first
                              dup_phys->copyParams(*src_phys, false); // copy params
                              dup_phys->physData()->mesh_id=dup_mesh->id; // this will get cleared in 'copyParams', so reassign again
                              if(f.readTry(editPath(*src_phys)) && SafeOverwrite(f, editPath(*dup_phys))) SavedEditPhys(editPath(*dup_phys)); // copy phys edit file
                              if(f.readTry(gamePath(*src_phys)) && SafeOverwrite(f, gamePath(*dup_phys))){SavedPhys    (gamePath(*dup_phys)); savedGame(*dup_phys);} // copy phys game file
                              Server.setElmFull(dup_phys->id); // send phys to server
                           }
                        }
                     }
                     makeGameVer(*dup_mesh); // make game mesh at end (after setting skeleton)
                     Server.setElmFull(dup_mesh->id); // send mesh to server
                  }else dup_obj_data->mesh_id.zero(); // couldn't create a mesh
               }
               makeGameVer(dup); // make game obj at end (after mesh and phys)
            }break;

            case ELM_MINI_MAP: break; // do nothing

            case ELM_WORLD: makeGameVer(dup); break; // save world settings

            case ELM_ENV        :
            case ELM_ENUM       :
            case ELM_PANEL      :
            case ELM_TEXT_STYLE :
            case ELM_MTRL       :
            case ELM_WATER_MTRL :
            case ELM_PHYS_MTRL  :
            case ELM_GUI_SKIN   :
            case ELM_GUI        :
            case ELM_ICON       :
            case ELM_ICON_SETTS :
            case ELM_IMAGE      :
            case ELM_IMAGE_ATLAS:
            case ELM_FONT       :
            case ELM_PANEL_IMAGE:
            case ELM_ANIM       :
            case ELM_SOUND      :
            case ELM_VIDEO      :
            case ELM_FILE       :
            {
               File f; if(f.readTry(editPath(*src)))   SafeOverwrite(f, editPath(dup));
                       if(f.readTry(gamePath(*src)))if(SafeOverwrite(f, gamePath(dup)))savedGame(dup);
            }break;
         }
         Server.setElmFull(dup.id);
      }
      if(duplicated.elms())list_cur=duplicated.last();
      list_sel=duplicated;
      setList();
      activateSources(); // rebuild sources if needed
      if(manually_by_user)
      {
         if(duplicated.elms()==1)RenameElm.activate(duplicated[0]);
         if(ElmChange *change=elm_undos.set(null, true))
         {
            change->type=ElmChange::RESTORE;
            change->name="Copy";
            REPA(duplicated)change->elms.binaryInclude(duplicated[i]);
         }
      }
   }
   void ProjectEx::eraseElm(C UID &elm_id)
{
           MtrlEdit.erasing(elm_id);
      WaterMtrlEdit.erasing(elm_id);
       PhysMtrlEdit.erasing(elm_id);
            ObjEdit.erasing(elm_id);
          WorldEdit.erasing(elm_id);
        MiniMapEdit.erasing(elm_id);
            EnvEdit.erasing(elm_id);
           EnumEdit.erasing(elm_id);
          ImageEdit.erasing(elm_id);
     ImageAtlasEdit.erasing(elm_id);
      IconSettsEdit.erasing(elm_id);
           IconEdit.erasing(elm_id);
           FontEdit.erasing(elm_id);
      TextStyleEdit.erasing(elm_id);
     PanelImageEdit.erasing(elm_id);
          PanelEdit.erasing(elm_id);
        GuiSkinEdit.erasing(elm_id);
            GuiEdit.erasing(elm_id);
       ObjClassEdit.erasing(elm_id);
          SoundEdit.erasing(elm_id);
          VideoEdit.erasing(elm_id);
           AnimEdit.erasing(elm_id);
           CodeEdit.erasing(elm_id);
       AppPropsEdit.erasing(elm_id);
      ImportTerrain.erasing(elm_id);
       Synchronizer.erasing(elm_id);
             super::eraseElm(elm_id);
   }
   bool ProjectEx::eraseTex(C UID &tex_id)
{
      Importer    .excludeTex(tex_id);
      Synchronizer.erasingTex(tex_id);
      return super::eraseTex  (tex_id);
   }
   void ProjectEx::eraseWorldAreaObjs(C UID &world_id, C VecI2 &area_xy)
{
      if(world_id==WorldEdit.elm_id)
         if(Area *area=WorldEdit.findAreaLoaded(area_xy))
      {
         bool erased=false; REPA(area->objs)if(Obj *obj=area->objs[i])if(obj->removed){erased=true; WorldEdit.objs.removeData(obj, true);}
         if(erased)area->setChangedObj();
         return;
      }
      super::eraseWorldAreaObjs(world_id, area_xy);
   }
   void ProjectEx::eraseRemoved(bool full)
{
      Builder .stop(); // stop the builder in case it processes removed elements
      Importer.stop();
      pauseServer();

      super::eraseRemoved(full);

      setList();
      Importer.investigate(root); // call after setting list because may rely on hierarchy
      resumeServer(); // call after setting list because may rely on hierarchy
   }
   void ProjectEx::elmChangedParentRemovePublish(bool network)
   {
      activateSources(); // rebuild sources if needed
      if( network)WorldEdit.delayedValidateRefs();else WorldEdit.validateRefs();
      if(!network)paramEditObjChanged(); // removed elements should display in red in param list, however disable from network because this might cause keyboard to lose focus
   }
   void ProjectEx::setElmParent(Memc<Edit::IDParam<UID>> &elms, bool adjust_elms, bool as_undo) // 'adjust_elms'=if this is performed because of undo, and in that case we need to remember current parents, so we can undo this change later
   {
      if(elms.elms())
      {
         ElmChange *change=null; if(as_undo)if(change=elm_undos.set(null, true))
         {
            change->type=ElmChange::SET_PARENT;
            change->name="Change Parent";
         }
         Memc<UID> changed;
         TimeStamp time; time.getUTC();
         FREPA(elms)
         {
               Elm *dest=findElm(elms[i].value); // !! first get desired parent before adjusting !! this can be null (no parent)
            if(Elm *elm =findElm(elms[i].id   )) //          get element
            {
               if(adjust_elms)elms[i].value=elm->parent_id; // !! IF WE'RE ADJUSTING, THEN REMEMBER CURRENT PARENT, BEFORE SETTING 'dest' !!
               if(!dest || ElmCanHaveChildren(dest->type))
               if(ElmMovable(elm->type))if(!contains(*elm, dest))
               {
                  if(change)
                  {
                     Edit::IDParam<UID> &elm_parent=change->elm_parents.New();
                     elm_parent.id   =elm->id;
                     elm_parent.value=elm->parent_id; // remember the old parent
                  }
                  elm->setParent(dest, time);
                  Server.setElmParent(*elm);
                  changed.add(elm->id);
               }
            }
         }
         refresh();
         Importer.investigate(changed); // !! CALL AFTER 'setList' BECAUSE MAY RELY ON HIERARCHY !!
         elmChangedParentRemovePublish();
      }
   }
   void ProjectEx::drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(focus_obj==&list && elms.elms()) // move elements to another location
      {
         Elm *dest=list.visToElm(list.screenToVis(screen_pos)); if(dest)dest->opened(true);
         if( !dest || ElmCanHaveChildren(dest->type))
         {
            ElmChange *change=elm_undos.set(null, true);
            if(change)
            {
               change->type=ElmChange::SET_PARENT;
               change->name="Change Parent";
            }
            Memc<UID> changed;
            TimeStamp time; time.getUTC();
            FREPA(elms)if(Elm *elm=findElm(elms[i]))if(ElmMovable(elm->type))if(!contains(*elm, dest))
            {
               if(change)
               {
                  Edit::IDParam<UID> &elm_parent=change->elm_parents.New();
                  elm_parent.id   =elm->id;
                  elm_parent.value=elm->parent_id; // remember the old parent
               }
               elm->setParent(dest, time);
               Server.setElmParent(*elm);
               changed.add(elm->id);
            }
            elms.clear(); // processed
            refresh();
            Importer.investigate(changed); // !! CALL AFTER 'refresh' BECAUSE MAY RELY ON HIERARCHY !!
            elmChangedParentRemovePublish();
         }
      }
   }
   void ProjectEx::collapse(Memc<UID> &ids, Memc<EEItem*> &items) {setListCurSel(); REPA(ids)if(Elm *elm=findElm(ids[i]))elm->opened(false); REPAO(items)->opened=false; setList(false, false);}
   void ProjectEx::expand(Memc<UID> &ids, Memc<EEItem*> &items) {setListCurSel(); REPA(ids)if(Elm *elm=findElm(ids[i]))elm->opened(true ); REPAO(items)->opened=true ; setList(false, false);}
   void ProjectEx::expandAll(Memc<UID> &ids, Memc<EEItem*> &items)
   {
      setListCurSel();
      REPA(ids)
      {
         int e=findElmI(ids[i]); if(InRange(e, hierarchy))
         {
            elms[e].opened(true); // open this element
            expandAll(hierarchy[e]); // open its children
         }
      }
      REPAO(items)->opened=true; // TODO: here children too
      setList(false, false);
   }
   void ProjectEx::expandAll(ElmNode &node)
   {
      REPA(node.children)
      {
         int  child_i=node.children[i];
         Elm &elm    =elms[child_i];
         if(  elm.exists() && ElmVisible(elm.type))
         {
            elm.opened(true);
            expandAll(hierarchy[child_i]);
         }
      }
   }
   void ProjectEx::floodExisting(ElmNode &node, uint publish_flag, bool parent_publish)
   {
      REPA(node.children)
      {
         int  child_i=node.children[i];
         Elm &elm    =elms[child_i];
         if(  elm.exists())
         {
            bool           elm_publish=(parent_publish && !(elm.flag&publish_flag)); // "!" because we have NO_PUBLISH* flags
                           elm.finalExists (true);
            if(elm_publish)elm.finalPublish(true);
            switch(elm.type)
            {
               case ELM_ENUM     :               existing_enums      .binaryInclude(elm.id); break;
               case ELM_OBJ_CLASS:               existing_obj_classes.binaryInclude(elm.id); break;
               case ELM_FONT     : if(elm_publish)publish_fonts      .binaryInclude(elm.id); break;
               case ELM_APP      :               existing_apps       .binaryInclude(elm.id); break;
            }
            floodExisting(hierarchy[child_i], publish_flag, elm_publish);
         }
      }
   }
   void ProjectEx::setExisting()
   {
      REPAO(elms).resetFinal();
                                      existing_apps .clear();
      Memc<UID> old_enums      ; Swap(existing_enums      , old_enums      ); // this also clears 'existing_enums'
      Memc<UID> old_obj_classes; Swap(existing_obj_classes, old_obj_classes); // this also clears 'existing_obj_classes'
      Memc<UID> old_fonts      ; Swap( publish_fonts      , old_fonts      ); // this also clears 'publish_fonts'
      floodExisting(root, PublishableFlag());
      if(!Same(existing_enums      , old_enums      )
      || !Same(existing_obj_classes, old_obj_classes))enumChanged();
      if(!Same( publish_fonts      , old_fonts      ))fontChanged();
   }
   int ProjectEx::CompareEnum(C Str &a, C Str &b) {return Compare(a, b);}
   void ProjectEx::enumChanged(C UID &enum_id) // 'enum_id' can also point to OBJ_CLASS
   {
      obj_class_node.children.clear();
      Memt<Str> obj_classes; FREPA(existing_obj_classes)if(Elm *obj_class=findElm(existing_obj_classes[i]))obj_classes.binaryInclude(obj_class->name, CompareEnum);
      if(InRange(OBJ_ACCESS_TERRAIN, ObjAccessNamesElms))obj_class_node+=ObjAccessNames[OBJ_ACCESS_TERRAIN];
      if(InRange(OBJ_ACCESS_GRASS  , ObjAccessNamesElms))obj_class_node+=ObjAccessNames[OBJ_ACCESS_GRASS  ];
      if(InRange(OBJ_ACCESS_OVERLAY, ObjAccessNamesElms))obj_class_node+=ObjAccessNames[OBJ_ACCESS_OVERLAY];
      if(obj_classes.elms())obj_class_node++;
      FREPA(obj_classes)obj_class_node+=obj_classes[i];

      param_type_node.children.clear();
      ASSERT(PARAM_BOOL ==0); (param_type_node+="Bool").desc("Boolean value\nFalse/True");
      ASSERT(PARAM_INT  ==1); (param_type_node+="Int").desc("Integer value");
      ASSERT(PARAM_FLT  ==2); (param_type_node+="Real").desc("Floating point value");
      ASSERT(PARAM_STR  ==3); (param_type_node+="String").desc("Text string");
      {
         ASSERT(PARAM_ENUM==4); Node<MenuElm> &e=param_type_node+="Enum"; e.flag(e.flag()|MENU_NOT_SELECTABLE).desc("Custom enum");
         Memt<Str> enums; FREPA(existing_enums)if(Elm *e=findElm(existing_enums[i]))enums.binaryInclude(e->name, CompareEnum);
         FREPA(enums)e+=enums[i];
      }
      ASSERT(PARAM_VEC2 ==5); (param_type_node+="Vec2").desc("2D vector");
      ASSERT(PARAM_VEC  ==6); (param_type_node+="Vec").desc("3D vector");
      ASSERT(PARAM_VEC4 ==7); (param_type_node+="Vec4").desc("4D vector");
      ASSERT(PARAM_COLOR==8); (param_type_node+="Color").desc("Color value");
      ASSERT(PARAM_ID   ==9); (param_type_node+="Element ID").desc("Project element or custom ID\nIn order to set its value please drag and drop a project element to the textline, or manually enter an ID \"UID(..)\"\nYou can drag and drop multiple elements at the same time."); // can't use slash in the name because ComboBox doesn't support it
      ObjClassEdit.enumChanged();
           ObjEdit.enumChanged(enum_id);
         WorldEdit.enumChanged();
          CodeEdit.makeAuto   (); // need to make auto header because it relies on enums
   }
   void ProjectEx::meshVariationChanged()
   {
        ObjEdit.meshVariationChanged();
      WorldEdit.meshVariationChanged();
       IconEdit.meshVariationChanged();
   }
   void ProjectEx::fontChanged()
   {
      font_node.children.clear();
      Memt<Str> names; FREPA(publish_fonts)if(Elm *font=findElm(publish_fonts[i]))names.binaryInclude(font->name, CompareEnum);
      font_node+=" "; // DefaultFont
      FREPA(names)font_node+=names[i];
      TextStyleEdit.fontChanged();
   }
   void ProjectEx::panelChanged(C UID &elm_id)
   {
      GuiSkinEdit.objs.setRect();
   }
   void ProjectEx::panelImageChanged(C UID &elm_id)
   {
      GuiSkinEdit.objs.setRect();
   }
   void ProjectEx::mtrlTexChanged()
   {
      if(list.file_size && list.its!=ElmList::ITS_ELM) // mtrl tex file size have changed
      {
      #if 1 // delayed
         AtomicSet(texture_changed, true);
      #else // immediate
         refresh(false, false);
      #endif
      }
   }
   void ProjectEx::DragElmsStart(ProjectEx &proj)
   {
      // get elements from project list
      proj.drag_list_sel.clear();
      FREPA(proj.list.sel)if(ListElm *list_elm=proj.list.absToData(proj.list.sel[i]))if(list_elm->elm)proj.drag_list_sel.add(list_elm->elm->id); // list in order

      // start drag
      proj.dragElmsStart();
   }
   void ProjectEx::dragElmsStart()
   {
      // get element types
      bool elm_types[ELM_NUM_ANY]; Zero(elm_types);
      REPA(drag_list_sel)if(Elm *elm=findElm(drag_list_sel[i]))elm_types[elm->type]=true;

      // enable compatible types
      REPAD(i, elm_types)if(!elm_types[i])
      REPAD(j, elm_types)if( elm_types[j] && ElmCompatible(ELM_TYPE(i), ELM_TYPE(j))){elm_types[i]=true; break;}

      REPAO(PropEx::props).dragStart(elm_types);
   }
   void ProjectEx::DragElmsCancel(ProjectEx &proj)
   {
      REPAO(PropEx::props).dragEnd();
   }
   void ProjectEx::DragElmsFinish(ProjectEx &proj, GuiObj *obj, C Vec2 &screen_pos)
   {
      Drag(proj.drag_list_sel, obj, screen_pos);
      REPAO(PropEx::props).dragEnd();
   }
   void ProjectEx::update()
   {
      // auto save
      if(Time.appTime()>=save_time)save(SAVE_AUTO);

      // elm sound play
      {
         Elm *elm=list.visToElm(list.lit);
         if(  elm && (elm->type==ELM_SOUND || sound.playing()))
         {
            list.sound_play.lit_id=elm->id;
            list.sound_play.image =((sound.playing() && list.sound_play.lit_id==list.sound_play.play_id || elm->type!=ELM_SOUND) ? icon_stop : icon_play);
            list.sound_play.posRU(Vec2(region.rect().w()-region.slidebarSize(), 0));
            list.addChild(list.sound_play, list.visToAbs(list.lit), 0);
         }else list-=list.sound_play;
      }

      // list updates
      if(!(Time.frame()&63)) // process approximately once per second
      {
         bool refresh=false;

         // file sizes
         if(file_size_getter.created())
         {
            bool busy=file_size_getter.busy(); // !! check this before calling 'get' !!
            if(       file_size_getter.get ())
            {
               if(file_size_getter_step==0) // game files
               {
                  REPA(file_size_getter.elms)
                  {
                     FileSizeGetter::Elm &file_size=file_size_getter.elms[i];
                     if(Elm *elm=findElm(file_size.id))if(elm->file_size<0)elm->file_size=file_size.file_size; // set file size only if unknown, because if it's known, then most likely it has more recent info than from 'FileSizeGetter'
                  }
               }else // textures
               {
                  REPA(file_size_getter.elms)
                  {
                     FileSizeGetter::Elm &file_size=file_size_getter.elms[i];
                     TexInfos(file_size.id)->file_size=file_size.file_size;
                  }
               }
               file_size_getter.clear();
               refresh=true;
            }
            if(!busy) // proceed to the next step
               if(file_size_getter_step==0)
            {
               file_size_getter_step=1;
               file_size_getter.get(tex_path);
            }
         }

         // texture info
         if(texture_changed)
         {
            AtomicSet(texture_changed, false);
            refresh=true;
         }

         if(refresh)T.refresh(false, false);
      }

      REPA(MT)
         if(MT.guiObj(i)==&list)
            if(ListElm *elm_gui=list.curToListElm())
      {
         if(EEItem *item=elm_gui->item) // Engine Library (headers+sources+folders, all const)
         {
            if((list.flag&LIST_MULTI_SEL) ? list.selMode()==LSM_SET : true)if(MT.bp(i) && !MT.bd(i))elmToggle(item);
         }else // use 'else' because after 'setList' the 'elm' pointer may be invalid
         if(Elm *elm=elm_gui->elm)
         {
            if(MT.bp(i))Gui.drag(DragElmsFinish, T, MT.touch(i), DragElmsStart, DragElmsCancel);
            if((list.flag&LIST_MULTI_SEL) ? list.selMode()==LSM_SET : true)
            {
               if(elm_gui->hasVisibleChildren() && MT.pos(i).x<=list.screenPos().x+elm_gui->offset+list.columnWidth(0)) // open arrow
               {
                  if(MT.bp(i) && !MT.bd(i))
                  {
                     elm->opened(!elm->opened());
                     refresh(false, false);
                  }
               }else
               switch(elm->type)
               {
                  case ELM_FOLDER:
                  case ELM_LIB   :
                  case ELM_CODE  :
                  {
                     if(MT.tappedFirst(i))
                     {
                        elmToggle(elm);
                        list.tapped_vis =list.cur;
                        list.tapped_time=Time.appTime()+Time.ad()+(MT.mouse(i) ? DoubleClickTime : TouchDoubleClickTime); // disable mouse buttons during this time
                        list.tapped_open=false; // disable
                     }
                  }break;

                  case ELM_APP:
                  {
                     if(MT.tappedFirst(i))
                     {
                        list.tapped_vis =list.cur;
                        list.tapped_time=Time.appTime()+Time.ad()+(MT.mouse(i) ? DoubleClickTime : TouchDoubleClickTime); // wait until potential double click, if it won't occur, then open element
                        list.tapped_open=true; // open
                     }else
                     if(MT.bd(i))
                     {
                        list.tapped_vis=-1; // cancel checking for tap open
                        elmToggle(elm);
                        MT.eat(i, 0); // eat so tap won't be called
                     }
                  }break;

                  default: if(MT.bd(i))elmToggle(elm); break;
               }
            }
         }
      }
      if(region.contains(Gui.ms()))
      {
         if(Ms.bp(1))
         {
            if(Kb.ctrlCmd()) // paste elm ID to codes
            {
               if(Mode()==MODE_CODE)if(Elm *elm=list.visToElm(list.lit))if(ElmCanAccessID(elm->type))
               {
                  CodeEdit.paste(Kb.shift() ? S+'"'+EncodeFileName(elm->id)+'"' : elm->id.asCString());
                  CodeEdit.kbSet();
               }
            }else // element conext menu
            {
               list.cur=list.lit; if(!list.sel.has(list.visToAbs(list.lit)))list.setCur(list.lit);
               elmMenu(Ms.pos());
            }
         }else
         if(Ms.bp(2))
         {
            if(Kb.ctrlCmd()) // copy elm ID to clipboard
            {
               if(Elm *elm=list.visToElm(list.lit))
               {
                  ClipSet(ElmCanAccessID(elm->type) ? (Kb.shift() ? S+'"'+EncodeFileName(elm->id)+'"' : elm->id.asCString())+" /* "+Replace(elmFullName(elm), "*/", "*\\")+" */" : "This element ID is not available");
               }
            }else Misc.hide_proj.push(); // hide project view
         }else
         if(Ms.bp(MS_BACK))
         {
            if(ListElm *elm=list.litToListElm())
            {
               if(!elm->close()) // try closing highlighted element
                  for(; --list.lit>=0; )if(ListElm *e=list.litToListElm())if(e->offset<elm->offset && e->close())break; // find first parent
               refresh(false, false);
               list.scrollTo(list.lit-4); // scroll after refresh
            }
         }
      }
      REPA(Touches)if(Touches[i].on() && Touches[i].guiObj()==&list && Touches[i].life()>=LongPressTime && !Touches[i].dragging() && !Gui.dragging() && !Gui.menu())
      {
         elmMenu(Touches[i].pos(), true);
         Gui.dragCancel();
      }
      KbSc esc(KB_ESC);
      if(  esc.pd())
      {
         esc.eat();
         if(Gui.kb()==&filter && filter().is())filter.clear();else Mode.kbToggle();
      }
      if(Gui.kb()==&filter && Kb.kf(KB_ENTER))
      {
         list.setCur(0);
         list.kbSet();
      }
      updateMenu();
   }
   int ProjectEx::CompareChildren(C int &a, C int &b)
   {
      Elm &ea=Proj.elms[a], &eb=Proj.elms[b];
      if(int c=Compare      (ElmOrder(ea.type), ElmOrder(eb.type)))return c;
      if(int c=CompareNumber(         ea.name ,          eb.name ))return c;
      return   Compare      (         ea.id   ,          eb.id    );
   }
   int ProjectEx::CompareChildren(C EEItem &a, C Elm &b)
   {
      if(int c=Compare(ElmOrder(a.type     ), ElmOrder(b.type)))return c;
      if(int c=Compare(         a.base_name ,          b.name ))return c;
      return 0;
   }
   bool ProjectEx::hasInvalid(ElmNode &node)
   {
      REPA(node.children)
      {
         int  child_i=node.children[i];
         Elm &elm    =elms[child_i];
         if(elm.finalPublish() && ElmVisible(elm.type)) // could use "elm.exists() && elm.publish()" here, however we need platform
            if(invalidRefs(elm) || hasInvalid(hierarchy[child_i]))return true;
      }
      return false;
   }
   uint ProjectEx::PublishableFlag(Edit::EXE_TYPE exe_type)
   {
      uint flag=Elm::REMOVED|Elm::NO_PUBLISH; // must exist and be publishable
      switch(exe_type)
      {
         case Edit::EXE_APK:
         case Edit::EXE_AAB:
         case Edit::EXE_IOS:
         case Edit::EXE_WEB:
       //case Edit.EXE_NS : Nintendo Switch excluded because its game sizes can be big
            flag|=Elm::NO_PUBLISH_MOBILE; break;
      }
      return flag;
   }
   void ProjectEx::getPublishElms(Memt<Elm*> &app_elms, ElmNode &node, uint publish_flag)
   {
      REPA(node.children)
      {
         int  child_i=node.children[i];
         Elm &elm    =elms[child_i];
         if(!(elm.flag&publish_flag)) // exists and publishable, "!" because we have NO_PUBLISH* flags
         {
            app_elms.add(&elm);
            getPublishElms(app_elms, hierarchy[child_i], publish_flag);
         }
      }
   }
   void ProjectEx::getActiveAppElms(Memt<Elm*> &app_elms, ElmNode &node, uint publish_flag, C UID &app_id, bool inside_valid)
   {
      REPA(node.children)
      {
         int  child_i=node.children[i];
         Elm &elm    =elms[child_i];
         if(!(elm.flag&publish_flag)) // exists and publishable, "!" because we have NO_PUBLISH* flags
         {
            if(inside_valid)app_elms.add(&elm);
            getActiveAppElms(app_elms, hierarchy[child_i], publish_flag, app_id, (elm.type==ELM_LIB) ? true : (elm.type==ELM_APP) ? (elm.id==app_id) : inside_valid); // include elements from all libraries and from active app only, in other case inherit valid from the parent
         }
      }
   }
   void ProjectEx::getPublishElms(Memt<Elm*> &app_elms, Edit::EXE_TYPE exe_type) {getPublishElms  (app_elms, root, PublishableFlag(exe_type));}
   void ProjectEx::getActiveAppElms(Memt<Elm*> &app_elms, Edit::EXE_TYPE exe_type) {getActiveAppElms(app_elms, root, PublishableFlag(exe_type), curApp(), false);}
   void ProjectEx::activateSources(int rebuild) // -1=never, 0=auto, 1=always
   {
      UID cur_app=curApp(); if(app_id!=cur_app){CodeEdit.makeAuto(); app_id=cur_app;} // set last app id to currently available, 'makeAuto' because 'APP_NAME' relies on active app
      CodeEdit.clearActiveSources();
      Memt<Elm*> app_elms; getActiveAppElms(app_elms, CodeEdit.configEXE()); FREPA(app_elms)if(app_elms[i]->type==ELM_CODE)CodeEdit.activateSource(app_elms[i]->id);
      if(rebuild>=0)CodeEdit.activateApp(rebuild>=1);else CodeEdit.makeAuto(); // if not activating app then call 'makeAuto' which will activate the auto header
   }
   void ProjectEx::ActivateApp(ElmList::AppCheck &app_check) {Proj.activateApp(app_check.app_id);}
          bool ProjectEx::activateApp(C UID            &elm_id   )
   {
      if(findElm(elm_id, ELM_APP))
      {
         UID old_app=curApp(); // get old app
         T.app_id=elm_id; // set new app
         REPAO(list.app_checks).set(list.app_checks[i].app_id==app_id, QUIET); // set all checkboxes states
         if(old_app!=curApp()) // if changed app
         {
            CodeEdit.makeAuto(); // we've changed app so we need to make auto here
            activateSources(1);
         }
         return true;
      }
      return false;
   }
   void ProjectEx::setList(EEItem &item, int depth, bool parent_contains_name)
   {
      bool this_contains_name=false, child_contains_name=false;
      if(filter().is())
      {
          this_contains_name=FlagOn(item.flag, ELM_CONTAINS_NAME);
         child_contains_name=FlagOn(item.flag, ELM_CONTAINS_NAME_CHILD);
         if(!(child_contains_name || this_contains_name || parent_contains_name))return;
      }
      bool     opened=(item.opened || child_contains_name || FlagOn(item.flag, ELM_EDITED_CHILD));
      ListElm &e=list.data.New().set(item, opened, depth);
      if(opened) // list children
         FREPA(item.children)setList(item.children[i], depth+1, parent_contains_name || this_contains_name);
   }
   void ProjectEx::includeElmFileSize(ListElm &e, ElmNode &node)
   {
      FREPA(node.children)
      {
         int  child_i=node.children[i];
         Elm &elm    =elms[child_i];
         if(  elm.      exists() || show_removed())
         if(  elm.finalPublish() || list.include_unpublished_elm_size) // could use 'publish' here, however we need platform
         {
            e.includeSize(elm);
            includeElmFileSize(e, hierarchy[child_i]);
         }
      }
   }
   void ProjectEx::includeElmNotVisibleFileSize(ListElm &e, ElmNode &node)
   {
      FREPA(node.children)
      {
         int  child_i=node.children[i];
         Elm &elm    =elms[child_i];
         if(!ElmVisible(elm.type))
            if(elm.      exists() || show_removed())
            if(elm.finalPublish() || list.include_unpublished_elm_size) // could use 'publish' here, however we need platform
         {
            e.includeSize(elm);
            includeElmFileSize(e, hierarchy[child_i]); // 'includeElmFileSize' is correct, not 'includeElmNotVisibleFileSize' (because for these elements we want all of their children)
         }
      }
   }
   bool ProjectEx::hasVisibleChildren(C ElmNode &node)C
   {
      REPA(node.children)
      {
         int  child_i=node.children[i];
       C Elm &elm    =elms[child_i];
         if(ElmVisible(elm.type))
         {
          C ElmNode &child=hierarchy[child_i];
            if(elm.exists() || show_removed() || FlagOn(child.flag, ELM_EDITED|ELM_EDITED_CHILD))
               if(list.show_elm_type[elm.type] || hasVisibleChildren(child))return true;
         }
      }
      return false;
   }
   void ProjectEx::setList(ElmNode &node, int depth, int vis_parent, bool parent_contains_name, bool parent_no_publish_all)
   {
      node.children.sort(CompareChildren);
      EEItem *ee=null; if(&node==&root && CodeEdit.items.elms() && list.show_all_elm_types)ee=&CodeEdit.items.first();
      FREPA(node.children)
      {
         int      child_i=node.children[i];
         ElmNode &child  =hierarchy[child_i];
         Elm     &elm    =elms     [child_i];
         if(ee && CompareChildren(*ee, elm)<0){setList(*ee, depth, parent_contains_name); ee=null;} // if "Engine" item should be included before this element
         if(elm.exists() || show_removed() || FlagOn(child.flag, ELM_EDITED|ELM_EDITED_CHILD))if(ElmVisible(elm.type))
         {
            bool this_contains_name=false, child_contains_name=false;
            if(filter().is())
            {
                this_contains_name=FlagOn(child.flag, ELM_CONTAINS_NAME);
               child_contains_name=FlagOn(child.flag, ELM_CONTAINS_NAME_CHILD);
               if(!(child_contains_name || this_contains_name || parent_contains_name))continue;
            }
            bool no_publish_all=(parent_no_publish_all || elm.noPublish()); // publish for all platforms
            if(list.show_elm_type[elm.type])
            {
               bool opened=(elm.opened() || child_contains_name || FlagOn(child.flag, ELM_EDITED_CHILD) || list.list_all_children),
                    exists=elm.finalExists (), // final
                   publish=elm.finalPublish(), // final (already includes exists)
                   invalid=(publish && invalidRefs(elm));

               int elm_list_index=list.data.elms();
               list.data.New().set(elm, child, depth, vis_parent);

               if(opened)setList(child, depth+1, list.flat_is ? -1 : elm_list_index, filter_is_id ? false : (parent_contains_name || this_contains_name), no_publish_all);else // list children, don't set 'parent_contains_name' when filter is by ID to make sure that desired element is listed last and its children are not listed at all
               if(publish && !invalid)invalid=hasInvalid(child); // if not listing children, then check if any of them are invalid and mark self as invalid

               ListElm &e=list.data[elm_list_index]; // !! get reference after calling 'setList' which may invalidate it if it was called after getting the ref !!
               e.hasVisibleChildren(list.flat_is ? false : (list.data.elms()>elm_list_index+1 || hasVisibleChildren(child)), opened); // first check if any new elements were listed after this one, because this check is faster than 'hasVisibleChildren'

               if(exists)
               {
                  if(list.file_size)
                     if(publish || list.include_unpublished_elm_size)
                  {
                     e.includeSize(elm);

                     bool included_all_children=false;
                     if(list.ics) // include children size
                        if(!opened || list.ics==ElmList::ICS_ALWAYS) // ICS_FOLDED or ICS_ALWAYS
                     {
                        if(opened && list.show_all_elms) // if element is opened (then it means we've listed its children), and if all elements are listed, then we can use an optimization by summing just the first children sizes from the list, instead of all children recursively, because those first children will already include their children sizes
                           for(int i=elm_list_index+1; i<list.data.elms(); i++) // iterate all elements added after this one (this will be its children and their children)
                        {
                           ListElm &child=list.data[i];
                           if(child.depth==depth+1)e.includeSize(child); // add only for direct children (and not children of children)
                        }else {includeElmFileSize(e, child); included_all_children=true;} // need to process all children recursively
                     }
                     if(!included_all_children) // if we haven't included all children yet
                        includeElmNotVisibleFileSize(e, child); // always include !ElmVisible without checking for 'ics', because they are never listed
                  }

                  if(invalid)
                  {
                     list.warnings.New().create(elm_list_index, true, e.offset);
                     e.desc="Element uses other elements which are not found, marked as removed or have publishing disabled";
                  }else
                  if(no_publish_all) // disabled for all platforms
                  {
                     list.warnings.New().create(elm_list_index, false, e.offset);
                     e.desc="Element will not be included in publishing";
                  }else
                  if(!publish) // disabled for this platform
                  {
                     ElmList::Warning &warning=list.warnings.New(); warning.create(elm_list_index, false, e.offset+list.columnWidth(list.icon_col)); warning.color.a=128; // half-transparent on right side
                     e.desc="Element will not be included in publishing for current Platform, but might be included for others";
                  }
                  if(elm.type==ELM_APP && existing_apps.elms()>1) // create checkboxes only if there are more than 1 apps
                  {
                     ElmList::AppCheck &app_check=list.app_checks.New();
                     app_check.create().func(ActivateApp, app_check).desc("Activate this application"); app_check.app_id=elm.id;
                     app_check.setRect();
                     list.addChild(app_check, elm_list_index);
                  }
               }
            }else // if this element is hidden, then don't list it, but proceed to children, regardless if this element is opened or not
            {
               setList(child, depth, vis_parent, filter_is_id ? false : (parent_contains_name || this_contains_name), no_publish_all); // list children, don't set 'parent_contains_name' when filter is by ID to make sure that desired element is listed last and its children are not listed at all
            }
         }
      }
      if(ee)setList(*ee, depth, parent_contains_name);
   }
   bool ProjectEx::setFilter(ElmNode &node)
   {
      if(node.children.elms())
      {
         byte flag=0;
         int  filter_path_length=filter_path.tailSlash(true).length();
         REPA(node.children)
         {
            int  child_i=node.children[i];
            Elm &elm    =elms[child_i];
            if(  elm.exists() || show_removed())
            {
               ElmNode &child=hierarchy[child_i];
               filter_path.clip(filter_path_length)+=elm.name;
               bool this_contains=(ElmVisible(elm.type) && (filter_is_id ? (elm.id==filter_id) : ContainsAny(elm.name, filter()) && ContainsAll(filter_path, filter()))), // !! check this first because 'setFilter' below will modify the 'filter_path' !!
                   child_contains=setFilter(child); // !! check this second !!
               FlagSet(child.flag, ELM_CONTAINS_NAME_CHILD, child_contains);
               FlagSet(child.flag, ELM_CONTAINS_NAME      ,  this_contains);
               flag|=child.flag;
            }
         }
         return FlagOn(flag, ELM_CONTAINS_NAME|ELM_CONTAINS_NAME_CHILD);
      }
      return false;
   }
   bool ProjectEx::setFilter(Memx<EEItem> &items)
   {
      if(items.elms())
      {
         byte flag=0;
         int  filter_path_length=filter_path.tailSlash(true).length();
         REPA(items)
         {
            EEItem &item=items[i];
            filter_path.clip(filter_path_length)+=item.base_name;
            bool this_contains=(filter_is_id ? false : ContainsAny(item.base_name, filter()) && ContainsAll(filter_path, filter())), // !! check this first because 'setFilter' below will modify the 'filter_path' !!
                child_contains=setFilter(item.children); // !! check this second !!
            FlagSet(item.flag, ELM_CONTAINS_NAME_CHILD, child_contains);
            FlagSet(item.flag, ELM_CONTAINS_NAME      ,  this_contains);
            flag|=item.flag;
         }
         return FlagOn(flag, ELM_CONTAINS_NAME|ELM_CONTAINS_NAME_CHILD);
      }
      return false;
   }
   ProjectEx& ProjectEx::editing(C UID &elm_id, bool force_open_parents)
   {
      int i=findElmI(elm_id); if(InRange(i, hierarchy))
      {
         hierarchy[i].flag|=ELM_EDITED;
         if(force_open_parents)for(i=hierarchy[i].parent; InRange(i, hierarchy); i=hierarchy[i].parent)hierarchy[i].flag|=ELM_EDITED_CHILD;
      }
      return T;
   }
   bool ProjectEx::editing(Memx<EEItem> &items, C Str &name)
   {
      byte flag=0;
      FREPA(items)
      {
         EEItem &item=items[i];
         FlagSet(item.flag, ELM_EDITED      , name.is() && EqualPath(item.full_name, name));
         FlagSet(item.flag, ELM_EDITED_CHILD, editing(item.children, name));
         flag|=item.flag;
      }
      return FlagOn(flag, ELM_EDITED|ELM_EDITED_CHILD);
   }
   ProjectEx& ProjectEx::editing(C Str &name) {editing(CodeEdit.items, name); return T;}
   void ProjectEx::setList(bool set_hierarchy, bool set_existing) // !! CALL 'setListCurSel' or 'clearListSel' before this !!
   {
      if(set_hierarchy)setHierarchy();else REPAO(hierarchy).flag=0; // if no need to set whole 'hierarchy' then reset flag manually
      if(set_existing )setExisting(); // after hierarchy
      editing(MtrlEdit.elm_id).editing(WaterMtrlEdit.elm_id).editing(PhysMtrlEdit.elm_id).editing(ObjEdit.obj_id).editing(WorldEdit.elm_id).editing(MiniMapEdit.elm_id).editing(EnvEdit.elm_id).editing(EnumEdit.elm_id).editing(ObjClassEdit.elm_id).editing(FontEdit.elm_id).editing(PanelImageEdit.elm_id).editing(TextStyleEdit.elm_id).editing(PanelEdit.elm_id).editing(GuiSkinEdit.elm_id).editing(GuiEdit.elm_id).editing(ImageEdit.elm_id).editing(ImageAtlasEdit.elm_id).editing(IconSettsEdit.elm_id).editing(IconEdit.elm_id).editing(SoundEdit.elm_id).editing(VideoEdit.elm_id).editing(AnimEdit.elm_id).editing(CodeEdit.sourceCurId(), true).editing(CodeEdit.sourceCurName()).editing(AppPropsEdit.elm_id); // after hierarchy
      if(filter().is())
      {
         filter_is_id=(filter_id.fromText(filter()) && hasElm(filter_id)); // set as ID only if there's an element with that ID, this is needed when entering a single name of 24 char length, which is also a valid FileNameID, which would force looking for ID but we want just the long name
         filter_path.clear(); setFilter(root);
         filter_path.clear(); setFilter(CodeEdit.items);
      }else filter_is_id=false; // if there's no filter then clear 'filter_is_id' because some codes may check it without checking for "filter().is()"
      list.show_all_elm_types=!(list.show_only_folder || list.show_only_obj || list.show_only_mtrl || list.show_only_anim || list.show_only_sound); REPAO(list.show_elm_type)=list.show_all_elm_types;
      list.flat_is=(list.flat_want && !list.show_all_elm_types);
      list.list_all_children=list.flat_is;
      if(list.show_only_folder)list.show_elm_type[ELM_FOLDER]=true;
      if(list.show_only_obj   )list.show_elm_type[ELM_OBJ   ]=true;
      if(list.show_only_mtrl  )list.show_elm_type[ELM_MTRL  ]=true;
      if(list.show_only_anim  )list.show_elm_type[ELM_ANIM  ]=true;
      if(list.show_only_sound )list.show_elm_type[ELM_SOUND ]=true;
      list.show_all_elms=(list.show_all_elm_types && !filter.is());
      list.warnings  .clear();
      list.app_checks.clear();
      list.data      .clear();
      setList(root);
      REPAO(list.data).vis_parent=list.data.addr(list.data[i].vis_parent_i); // now that all 'data' have been created, we can convert indexes to pointers
      list.setData(list.data);
      if(list.app_checks.elms()){UID cur_app=curApp(); REPAO(list.app_checks).set(list.app_checks[i].app_id==cur_app, QUIET);} // set checkboxes state
      if(lit_elm_id.valid() || lit_elm_name.is())elmHighlight(lit_elm_id, lit_elm_name, true);

      // set cur/sel
      list.cur=(list_cur.valid() ? list.elmToVis(findElm(list_cur)) : list.itemToVis(list_cur_item)); list_cur.zero(); list_cur_item=null;
      list.sel.clear();
      FREPA(list_sel     ){int abs=list. elmToAbs(findElm(list_sel     [i])); if(abs>=0)list.sel.add(abs);}
      FREPA(list_sel_item){int abs=list.itemToAbs(        list_sel_item[i] ); if(abs>=0)list.sel.add(abs);}
      clearListSel();

      Theater.refreshData();
   }
   void ProjectEx::clearListSel() // clear list selection
   {
      list_sel     .clear();
      list_sel_item.clear();
   }
   void ProjectEx::setListSel(Memc<UID> &list_sel, MemPtr<EEItem*> list_sel_item)
   {
      list_sel     .clear();
      list_sel_item.clear();
      FREPA(list.sel)if(ListElm *list_elm=list.absToData(list.sel[i]))
      {
         if(list_elm->elm                  )list_sel     .add(list_elm->elm->id);
         if(list_elm->item && list_sel_item)list_sel_item.add(list_elm->item  );
      }
   }
   void ProjectEx::setListCurSel() // needs to be called before adding/removing elements from 'elms'
   {
      list_cur.zero();
      list_cur_item=null;
      if(ListElm *list_elm=list.visToData(list.cur))
      {
         if(list_elm->elm)list_cur     =list_elm->elm->id;
                         list_cur_item=list_elm->item  ;
      }
      setListSel(list_sel, list_sel_item);
   }
   void ProjectEx::setMenuListSel()
   {
      setListSel(menu_list_sel, menu_list_sel_item);
   }
   ProjectEx& ProjectEx::refresh(bool set_hierarchy, bool set_existing)
   {
      setListCurSel(); setList(set_hierarchy, set_existing);
      return T;
   }
   void ProjectEx::elmOpenParents(C UID &id, bool set_list)
   {
      if(Elm *elm=findElm(id))for(; elm=findElm(elm->parent_id); )elm->opened(true);
      if(set_list)setList(false, false);
   }
   void ProjectEx::elmSelect(C MemPtr<UID> &elm_ids)
   {
      clearListSel();
      Memt<UID> added;
      FREPA(elm_ids) // add in order
         if(Elm *elm=firstVisibleParent(findElm(elm_ids[i]))) // add visible (not hidden)
            if(added.binaryInclude(elm->id)) // don't add the same element multiple times
      {
         elmOpenParents(elm->id, false);
         list_sel.add(elm->id);
      }
      setList(false, false);
   }
   bool ProjectEx::elmReload(C MemPtr<UID> &elm_ids, bool remember_result, bool refresh_elm_list)
   {
      bool ok=true; REPA(elm_ids)if(Elm *elm=findElm(elm_ids[i])){if(elm->type!=ELM_FOLDER && elm->type!=ELM_LIB && elm->type!=ELM_APP)elm->importing(true);}else ok=false;
      Importer.reload(elm_ids, remember_result);
      if(refresh_elm_list)refresh(false, false);
      return ok;
   }
   void ProjectEx::elmLocate(C UID &id, bool set_cur)
   {
      for(Elm *elm=findElm(id); elm; elm=findElm(elm->parent_id))
      {
         int vis=list.elmToVis(elm); if(vis>=0){if(set_cur)list.setCur(vis); list.scrollTo(vis, false, 1); return;}
      }
   }
   void ProjectEx::elmHighlight(C UID &id, C Str &name, bool force)
   {
      if(lit_elm_id!=id || !EqualPath(lit_elm_name, name) || force)
      {
         lit_elm_id  =id;
         lit_elm_name=name;
         REPAO(list.data).resetColor();
         for(Elm *elm=findElm(id); elm; elm=findElm(elm->parent_id))
         {
            int vis=list.elmToVis(elm); if(InRange(vis, list.data)){list.data[vis].highlight(); return;}
         }
         if(name.is())if(EEItem **item_ptr=CodeEdit.items_sorted.binaryFind(name, CodeView::CompareItem))for(EEItem *item=*item_ptr; item; item=item->parent)
         {
            int vis=list.itemToVis(item); if(InRange(vis, list.data)){list.data[vis].highlight(); return;}
         }
      }
   }
   void ProjectEx::elmToggle(EEItem *item)
   {
      if(item)switch(item->type)
      {
         case ELM_FOLDER:
         case ELM_LIB   :
         {
            item->opened^=1;
            refresh(false, false);
         }break;

         case ELM_CODE: CodeEdit.toggle(item); break;
      }
   }
   void ProjectEx::elmToggle(Elm *elm)
   {
      if(elm)switch(elm->type)
      {
         case ELM_FOLDER:
         case ELM_LIB   : elm->opened(!elm->opened()); refresh(false, false); break;

         case ELM_MTRL       :       MtrlEdit.toggle(elm); break;
         case ELM_WATER_MTRL :  WaterMtrlEdit.toggle(elm); break;
         case ELM_PHYS_MTRL  :   PhysMtrlEdit.toggle(elm); break;
         case ELM_OBJ        :        ObjEdit.toggle(elm); break;
         case ELM_WORLD      :      WorldEdit.toggle(elm); break;
         case ELM_MINI_MAP   :    MiniMapEdit.toggle(elm); break;
         case ELM_ENV        :        EnvEdit.toggle(elm); break;
         case ELM_ENUM       :       EnumEdit.toggle(elm); break;
         case ELM_IMAGE      :      ImageEdit.toggle(elm); break;
         case ELM_IMAGE_ATLAS: ImageAtlasEdit.toggle(elm); break;
         case ELM_ICON       :       IconEdit.toggle(elm); break;
         case ELM_ICON_SETTS :  IconSettsEdit.toggle(elm); break;
         case ELM_FONT       :       FontEdit.toggle(elm); break;
         case ELM_TEXT_STYLE :  TextStyleEdit.toggle(elm); break;
         case ELM_PANEL_IMAGE: PanelImageEdit.toggle(elm); break;
         case ELM_PANEL      :      PanelEdit.toggle(elm); break;
         case ELM_GUI_SKIN   :    GuiSkinEdit.toggle(elm); break;
         case ELM_GUI        :        GuiEdit.toggle(elm); break;
         case ELM_OBJ_CLASS  :   ObjClassEdit.toggle(elm); break;
         case ELM_SOUND      :      SoundEdit.toggle(elm); break;
         case ELM_VIDEO      :      VideoEdit.toggle(elm); break;
         case ELM_ANIM       :       AnimEdit.toggle(elm); break;
         case ELM_CODE       :       CodeEdit.toggle(elm); break;
         case ELM_APP        :   AppPropsEdit.toggle(elm); break;

         case ELM_FILE: elmLocate(elm->id, true); break; // TODO: in the future replace with some Hex Viewer
      }
   }
   void ProjectEx::elmToggle(ListElm *elm)
   {
      if(elm)
      {
         if(elm->elm)elmToggle(elm->elm );
         else       elmToggle(elm->item); // use else because after first toggle the pointer may be invalid
      }
   }
   void ProjectEx::elmToggle(C UID &id) {return elmToggle(findElm(id));}
   void ProjectEx::elmActivate(Elm *elm)
   {
      if(elm)switch(elm->type)
      {
         case ELM_MTRL       :       MtrlEdit.activate(elm); break;
         case ELM_WATER_MTRL :  WaterMtrlEdit.activate(elm); break;
         case ELM_PHYS_MTRL  :   PhysMtrlEdit.activate(elm); break;
         case ELM_OBJ        :        ObjEdit.activate(elm); break;
         case ELM_WORLD      :      WorldEdit.activate(elm); break;
         case ELM_MINI_MAP   :    MiniMapEdit.activate(elm); break;
         case ELM_ENV        :        EnvEdit.activate(elm); break;
         case ELM_ENUM       :       EnumEdit.activate(elm); break;
         case ELM_IMAGE      :      ImageEdit.activate(elm); break;
         case ELM_IMAGE_ATLAS: ImageAtlasEdit.activate(elm); break;
         case ELM_ICON       :       IconEdit.activate(elm); break;
         case ELM_ICON_SETTS :  IconSettsEdit.activate(elm); break;
         case ELM_FONT       :       FontEdit.activate(elm); break;
         case ELM_TEXT_STYLE :  TextStyleEdit.activate(elm); break;
         case ELM_PANEL_IMAGE: PanelImageEdit.activate(elm); break;
         case ELM_PANEL      :      PanelEdit.activate(elm); break;
         case ELM_GUI_SKIN   :    GuiSkinEdit.activate(elm); break;
         case ELM_GUI        :        GuiEdit.activate(elm); break;
         case ELM_OBJ_CLASS  :   ObjClassEdit.activate(elm); break;
         case ELM_SOUND      :      SoundEdit.activate(elm); break;
         case ELM_VIDEO      :      VideoEdit.activate(elm); break;
         case ELM_ANIM       :       AnimEdit.activate(elm); break;
         case ELM_CODE       :       CodeEdit.activate(elm); break;
         case ELM_APP        :   AppPropsEdit.activate(elm); break;
      }
   }
   void ProjectEx::elmNext(C UID &elm_id, int dir)
   {
      if(Elm *elm=findElm(elm_id))
      {
         ELM_TYPE type=elm->type;
         int vis=list.elmToVis(elm); if(vis>=0)REP(list.visibleElms()-1)
         {
            vis=Mod(vis+dir, list.visibleElms());
            if(Elm *elm=list.visToElm(vis))if(elm->type==type)
            {
               list.scrollTo(vis, false, 0.5f); elmActivate(elm); break;
            }
         }
      }
   }
   void ProjectEx::resize()
   {
      flt t=D.h(), b=-D.h(), w=Max(Misc.size().x, size().x); if(Misc.pos.x==0)
      {
         if(Misc.pos.y==0)MAX(b, Misc.rect().max.y);
         else             MIN(t, Misc.rect().min.y);
      }
      rect(Rect(-D.w(), b, -D.w()+w, t));
      flt text_size=0.038f, h=text_size*1.5f;
      theater     .rect(Rect_RU(             rect().w (), 0, h, h));
      show_removed.rect(Rect_RU(theater     .rect().lu(),    h, h));
      list_options.rect(Rect_RU(show_removed.rect().lu(), 0.85f*h, h));
      filter      .rect(Rect_LU(0, 0, list_options.rect().min.x, h));
      region      .rect(Rect   (0, -rect().h(), rect().w(), -h));
      list.   elmHeight(text_size).textSize(text_size);
      setListPadding();
      list.columnWidth (0, text_size*0.8f);
      list.columnWidth (1, text_size*0.8f);
    //list.columnWidth (2, region.rect().w()-region.slidebarSize()-list.columnWidth(1)-list.columnWidth(0)); don't set fixed column width
      list.sound_play.size(list.elmHeight());
   }
   void ProjectEx::elmMenu(C Vec2 &pos, bool touch)
   {
      int items=0, elms=0; Elm *elm=null; bool remove=false, restore=false, expand=false, expand_all=false, collapse=false, image=false, object=false, material=false, animation=false, sound=false, cancel_reload=false;
      REPA(list.sel)if(ListElm *list_elm=list.absToData(list.sel[i]))
      {
         if(       list_elm->item)items++;
         if(Elm *e=list_elm->elm )
         {
            elms++; elm=e;
            Elm *parent        =findElm(e->parent_id);
            bool parent_removed=(parent && parent->finalRemoved()); // if parent is removed (do extra check for 'findElm' in case 'parent_id' points to element which doesn't exist at all, for example it was copied from a different project)
            if( !parent_removed)if(e->removed())restore=true;else remove=true;
            if(e->importing())cancel_reload=true;
            if(list_elm->hasVisibleChildren())
            {
               if(e->opened())collapse=true;else expand=true;
               expand_all=true;
            }
            switch(e->type)
            {
               case ELM_IMAGE: image    =true; break;
               case ELM_OBJ  : object   =true; break;
               case ELM_MTRL : material =true; break;
               case ELM_ANIM : animation=true; break;
               case ELM_SOUND: sound    =true; break;
            }
         }
      }

      if(!elms && items)return; // don't do anything when just Items are selected

      // setup menu
      Node<MenuElm> n;
      if(!elms)
      {
         setNewElm(n, "New ");
      }else
      {
         if(elms==1)
         {
            expand=collapse=false; // hide expand/collapse if just 1 elm is selected
            Node<MenuElm> &New=(n+="New"); setNewElm(New);
            n++;
         }
         if(expand || expand_all || collapse)
         {
            if(expand    )n.New().create("Expand"    , Expand   , T);
            if(expand_all)n.New().create("Expand All", ExpandAll, T).desc("Expand all selected elements including their children");
            if(collapse  )n.New().create("Collapse"  , Collapse , T);
            n++;
         }
         if(elms==1)
         {
            n.New().create("Rename", ElmRename, T).kbsc(KbSc(KB_F2));
         }else
         {
            n.New().create("Replace Name Part", ReplaceName, T);
         }
         if(remove       )n.New().create("Remove"       , Remove      , T);
         if(restore      )n.New().create("Restore"      , Restore     , T);
                          n.New().create("Reload"       , Reload      , T).kbsc(KbSc(KB_R, KBSC_CTRL_CMD)).desc(reload_desc);
         if(cancel_reload)n.New().create("Cancel Reload", CancelReload, T);
         n++;
         if(elms==1)
         {
            if(elm->type==ELM_ANIM)
            {
               n.New().create("Split Animation", SplitAnim, T);
               n++;
            }
            n.New().create("Explore", ExploreElm, T)/*.kbsc(KbSc(KB_F4))*/.desc(explore_desc);
            switch(elm->type)
            {
               case ELM_ANIM:
               case ELM_IMAGE:
               case ELM_IMAGE_ATLAS:
               case ELM_FONT:
               case ELM_PANEL_IMAGE:
               case ELM_SKEL:
               case ELM_PHYS_MTRL:
               case ELM_ICON:
               case ELM_CODE:
               case ELM_MINI_MAP:
               case ELM_SOUND:
               case ELM_VIDEO:
               case ELM_FILE:
               case ELM_OBJ:
                  n.New().create("Export", Export, T);
               break;
            }
            n.New().create("Properties", Properties, T);
            n++;
         }
         if(image || object || material || animation || sound)
         {
            Node<MenuElm> &o=(n+="Operations");
            if(image)
            {
               Node<MenuElm> &m=(o+="Image");
               {
                  m.New().create("Enable Mip-Maps" , ImageMipMapOn , T).desc("This option will enable Mip-Maps for all selected images");
                  m.New().create("Disable Mip-Maps", ImageMipMapOff, T).desc("This option will disable Mip-Maps for all selected images");
                  Node<MenuElm> &r=(m+="Resize");
                  {
                     r.New().create("Original" , ImageResize0, T);
                     r.New().create("16x16"    , ImageResize16, T);
                     r.New().create("32x32"    , ImageResize32, T);
                     r.New().create("64x64"    , ImageResize64, T);
                     r.New().create("128x128"  , ImageResize128, T);
                     r.New().create("256x256"  , ImageResize256, T);
                     r.New().create("512x512"  , ImageResize512, T);
                     r.New().create("1024x1024", ImageResize1024, T);
                     r.New().create("2048x2048", ImageResize2048, T);
                  }
               }
            }
            if(material)
            {
               Node<MenuElm> &m=(o+="Material");
               {
                  m.New().create("Set RGB to (1, 1, 1)"      , MtrlSetRGB1   , T).desc("This option will set RGB color to (1, 1, 1) in all selected materials");
                  m.New().create("Set RGB to .."             , MtrlSetRGB    , T).desc("This option will set RGB color to manually specified value in all selected materials");
                  m.New().create("Set RGB to Edited Material", MtrlSetRGBCur , T).desc("This option will set RGB color to currently edited Material RGB value in all selected materials");
                  m.New().create("Multiply RGB by .."        , MtrlMulRGB    , T).desc("This option will multiply RGB color by manually specified value in all selected materials");
                  m.New().create("Set Alpha to 1 or 0.5"     , MtrlResetAlpha, T).desc("This option will set Alpha value to 0.5 for alpha-tested techniques and to 1.0 for other techniques in all selected materials");
                  m++;
                  m.New().create("Enable Cull" , MtrlCullOn , T).desc("This option will set Cull value to true for all selected materials");
                  m.New().create("Disable Cull", MtrlCullOff, T).desc("This option will set Cull value to false for all selected materials");
                  m++;
                  m.New().create("Enable Flip Normal Y" , MtrlFlipNrmYOn , T).desc("This option will set Flip Normal Y value to true for all selected materials");
                  m.New().create("Disable Flip Normal Y", MtrlFlipNrmYOff, T).desc("This option will set Flip Normal Y value to false for all selected materials");
                  m++;
                  m.New().create("Merge into one"  , MtrlMerge         , T).desc("This option will merge all selected materials into one\nMaterial textures are not combined in this mode");
                  m.New().create("Convert to Atlas", MtrlConvertToAtlas, T).desc("This option will merge all selected materials into one\nMaterial textures will be combined together");
                  m++;
                  m.New().create("Reload Base Textures", MtrlReloadBaseTex, T);
                  m++;
                  m.New().create("Set Technique to Edited Material", MtrlSetTechCur, T);
                  m++;
                  m.New().create("Set Bump Value to Edited Material"   , MtrlSetBumpCur   , T);
                  m.New().create("Set Normal Value to Edited Material" , MtrlSetNormalCur , T);
                  m.New().create("Set Smooth Value to Edited Material" , MtrlSetSmoothCur , T);
                  m.New().create("Set Reflect Value to Edited Material", MtrlSetReflectCur, T);
                  m.New().create("Set Glow Value to Edited Material"   , MtrlSetGlowCur   , T);
                  m++;
                  m.New().create("Set Color Texture to Edited Material"  , MtrlSetColorTexCur , T);
                  m.New().create("Set Bump Texture to Edited Material"   , MtrlSetBumpTexCur  , T);
                  m.New().create("Set Normal Texture to Edited Material" , MtrlSetNormalTexCur, T);
                  m.New().create("Set Smooth Texture to Edited Material" , MtrlSetSmoothTexCur, T);
                  m.New().create("Set Metal Texture to Edited Material"  , MtrlSetMetalTexCur , T);
                  m.New().create("Set Glow Texture to Edited Material"   , MtrlSetGlowTexCur  , T);
                  m++;
                  m.New().create("Multiply Color Texture by Color Value"      , MtrlMulTexCol     , T);
                  m.New().create("Multiply Normal Texture by Normal Value"    , MtrlMulTexNormal  , T);
                  m.New().create("Multiply Smooth Texture by Smooth Value"    , MtrlMulTexSmooth  , T);
                  m.New().create("Multiply Glow Texture by Glow Value"        , MtrlMulTexGlow    , T);
                  m.New().create("Multiply Emissive Texture by Emissive Value", MtrlMulTexEmissive, T);
                  m++;
                  m.New().create("Move to its Object", MtrlMoveToObj, T).desc("This option will move the Material Element to the Object it belongs to");
                  m++;
                  {
                     Node<MenuElm> &tsm=(m+="Texture Size Mobile");
                     tsm.New().create("Full"   , MtrlTexSizeMobileFull   , T);
                     tsm.New().create("Half"   , MtrlTexSizeMobileHalf   , T);
                     tsm.New().create("Quarter", MtrlTexSizeMobileQuarter, T);
                     tsm.New().create("Eighth" , MtrlTexSizeMobileEighth , T);

                     Node<MenuElm> &tss=(m+="Texture Size Switch");
                     tss.New().create("Full"   , MtrlTexSizeSwitchFull   , T);
                     tss.New().create("Half"   , MtrlTexSizeSwitchHalf   , T);
                     tss.New().create("Quarter", MtrlTexSizeSwitchQuarter, T);
                     tss.New().create("Eighth" , MtrlTexSizeSwitchEighth , T);

                     ASSERT(MaxMaterialDownsize==4);
                  }
               }
            }
            if(animation)
            {
               Node<MenuElm> &a=(o+="Animation");
               {
                  a.New().create("Clip to first/last keyframe"    , AnimClip   , T).desc("This option will clip the animation range starting with first keyframe and ending with last keyframe");
                  a.New().create("Enable Looping"                 , AnimLoopOn , T).desc("This option will set loop mode to enabled for selected animations");
                  a.New().create("Disable Looping"                , AnimLoopOff, T).desc("This option will set loop mode to disabled for selected animations");
                  a.New().create("Convert to Linear Interpolation", AnimLinear , T).desc("This option will set linear interpolation mode for selected animations, recommended for animations with high framerate");
                  a.New().create("Convert to Cubic Interpolation" , AnimCubic  , T).desc("This option will set cubic interpolation mode for selected animations, recommended for animations with low framerate");
               }
            }
            if(object)
            {
               Node<MenuElm> &t=(o+="Transform");
               {
                  t.New().create("Set"  , TransformSet  , T).desc("This will set the transformation of selected objects to the same one as currently opened object");
                  t.New().create("Apply", TransformApply, T).desc("This will apply the current transformation from the currently opened object \"Transform\" tab, to selected objects");
                  t++;
                  t.New().create("Move Bottom to Y=0"      , TransformBottom  , T).desc("This will transform the selected objects so that their Bottom Y is at 0 position");
                  t.New().create("Move Back to Z=0"        , TransformBack    , T).desc("This will transform the selected objects so that their Back Z is at 0 position");
                  t.New().create("Move Center to (0, 0, 0)", TransformCenter  , T).desc("This will transform the selected objects so that their Center is at (0, 0, 0) position");
                  t.New().create("Move Center XZ to (0, 0)", TransformCenterXZ, T).desc("This will transform the selected objects so that their Center.XZ is at (0, 0) position");
                  t++;
                  t.New().create("Rotate Y to Minimize Box", TransformRotYMinBox, T).desc("This will rotate selected objects along Y axis to minimize their bounding box");
               }
               Node<MenuElm> &m=(o+="Mesh");
               {
                  {
                     Node<MenuElm> &r=(m+="Remove");
                     r.New().create("Vertex TexCoord1"    , MeshRemVtxTex1  , T);
                     r.New().create("Vertex TexCoord2"    , MeshRemVtxTex2  , T);
                     r.New().create("Vertex TexCoord3"    , MeshRemVtxTex3  , T);
                     r.New().create("Vertex TexCoord1&2&3", MeshRemVtxTex123, T);
                     r++;
                     r.New().create("Vertex Color", MeshRemVtxCol, T);
                     r++;
                     r.New().create("Vertex Skin", MeshRemVtxSkin, T);
                  }
                  {
                     Node<MenuElm> &d=(m+="Disable");
                     d.New().create("Low Quality LODs", MeshDisableLQLODs, T);
                  }
                  {
                     Node<MenuElm> &o=(m+="Optimize");
                     o.New().create("Merge Coplanar Faces", MeshMergeCoplanarFaces, T);
                  }
               }
               Node<MenuElm> &obj=(o+="Object");
               {
                  obj.New().create("Set Body", SetBody, T).desc("This option will set the \"Body\" of all selected objects to the Object that is currently opened in the Object Editor");
               }
               m.New().create("Extract from Atlas", MtrlConvertToDeAtlas, T).desc("This option will extract the part of Material Texture that is used for selected Objects making the new texture smaller");
            }
            if(sound)
            {
               Node<MenuElm> &s=(o+="Sound");
               {
                  FREP(SoundEditor::ImportAsElms)s.New().create(SoundEditor::Import_as[i].name, SoundImportAs, ptr(i));
                  s++;
                  s.New().create("Multiply Volume by ..", SoundMulVolume, T).desc("This option will multiply volume by manually specified value in all selected sounds").kbsc(KbSc(KB_S, KBSC_CTRL_CMD|KBSC_ALT));
               }
            }
         }
         {
            Node<MenuElm> &c=(n+="Copy");
            c.New().create("Here"              , Duplicate, T).desc("This will duplicate selected elements in this Project");
            c.New().create("To Another Project", CopyTo   , T).desc("This will copy selected elements into another Project").kbsc(KbSc(KB_T, KBSC_CTRL_CMD));
         }
         {
            Node<MenuElm> &p=(n+="Publishing");
            p.New().create("Disable", DisablePublish, T).desc(dis_publish_desc);
            p.New().create( "Enable",  EnablePublish, T).desc( en_publish_desc);
            {
               p++;
               Node<MenuElm> &m=(p+="Mobile");
               m.New().create("Disable", DisablePublishMobile, T).desc(dis_publish_desc);
               m.New().create( "Enable",  EnablePublishMobile, T).desc( en_publish_desc);
            }
         }
      }
      if(elm_undos.changes())
      {
         if(n.children.elms())n++;
         if(ElmChange *undo=elm_undos.getNextUndo())n.New().create(S+"Undo "+undo->name, UndoElmChange, T);
         if(ElmChange *redo=elm_undos.getNextRedo())n.New().create(S+"Redo "+redo->name, RedoElmChange, T);
      }
      setMenuListSel();
      Gui+=list.menu.create(n); list.menu.setSize(touch).posRU(pos).moveToBottom().activate(); // move to bottom so it gets keyboard priority
   }
   bool ProjectEx::drop(C Str &name, C UID &parent_id, Memt<UID> &new_folders)
   {
      switch(FileInfoSystem(name).type)
      {
         case FSTD_FILE:
         {
            Str       ext=GetExt(name);
            ELM_TYPE type=ELM_NONE;
            switch(ExtType(ext))
            {
               case EXT_MESH : type=ELM_OBJ  ; break;
               case EXT_IMAGE: type=ELM_IMAGE; break;
            }
            if(!type)
            if(ext=="wav" || ext=="ogg" || ext=="flac" || ext=="opus" || ext=="mp3" || ext=="m4a" || ext=="weba")type=ELM_SOUND;else
            if(ext=="ogm" || ext=="ogv" || ext=="theora" || ext=="webm"                                         )type=ELM_VIDEO;else
            if(ext=="mtrl"                                                                                      )type=ELM_MTRL ;else
            if(ext=="anim"                                                                                      )type=ELM_ANIM ;else
            if(ext=="c" || ext=="cpp" || ext=="h" || ext=="cc" || ext=="cxx" || ext=="m" || ext=="mm"           )type=ELM_CODE ;else
            if(ext=="skel" || Ends(name, ".mesh.ascii") || Ends(name, ".xps.ascii")                             )type=ELM_OBJ  ;else
                                                                                                                 type=ELM_FILE ;
            if(Elm *elm=newElm(type, parent_id, &NoTemp(GetBaseNoExt(name)), false))
            {
               elm->setSrcFile(CodeEdit.importPaths(name)); elmReload(elm->id, false, false); list_sel.add(elm->id); // no need to call 'Server.setElmShort' because 'Server.setElmFull' was already requested in 'newElm'
               return true;
            }
         }break;

         case FSTD_DIR:
         {
            bool added_folder; Elm &folder=getFolder(GetBase(name), parent_id, added_folder); folder.opened(true);
            bool added_children=false; for(FileFind ff(name); ff(); )added_children|=drop(ff.pathName(), folder.id, new_folders);
            if(  added_folder && !added_children)elms.removeData(&folder, true);else
            {
               list_sel.add(folder.id); if(added_folder)new_folders.add(folder.id);
               return true;
            }
         }break;
      }
      return false;
   }
   void ProjectEx::drop(Memc<Str> &names, C UID &parent_id)
   {
      Memt<UID> new_folders;
      clearListSel();
      FREPA(names)drop(names[i], parent_id, new_folders);
       REPA(new_folders)if(Elm *elm=findElm(new_folders[i]))Server.newElm(*elm); // go from end because first are leafs
      setList();
   }
   void ProjectEx::drop(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos)
   {
      if(region.contains(focus_obj) && names.elms())
      {
         names.sort(ComparePathNumberCI).reverseOrder(); // sort by path so they will be imported in order
         Elm *parent=list.visToElm(list.screenToVis(screen_pos));
         UID  parent_id=(parent ? parent->id : UIDZero);
         if(parent)parent->opened(true);
         if(parent && parent->type==ELM_OBJ) // if we're dropping onto an object, then ask how we want to import stuff
         {
            Importer.import(*parent, names, screen_pos);
         }else
         {
            drop(names, parent_id);
         }
         names.clear();
      }
   }
   void ProjectEx::meshSetAutoTanBin(Elm &elm, C MaterialPtr &material)
{
      super::meshSetAutoTanBin(elm, material);
      if(ObjEdit.mesh_elm && ObjEdit.mesh_elm->id==elm.id)ObjEdit.setAutoTanBin(material);
   }
   void ProjectEx::animTransformChanged(Elm &elm_anim)
{
      super::animTransformChanged(elm_anim);
      if(&elm_anim==AnimEdit.elm)AnimEdit.toGui();
   }
   void ProjectEx::skelTransformChanged(C UID &skel_id)
{
      super::skelTransformChanged(skel_id);

      if(ObjEdit.skel_elm && ObjEdit.skel_elm->id==skel_id)ObjEdit.skelTransformChanged();
      if(AnimEdit.skel_id==skel_id)AnimEdit.setMeshSkel(true);
   }
   void ProjectEx::objChanged(Elm &obj)
{
      super::objChanged(obj);

              ObjEdit.param_edit.p->updateBase(edit_path);
      REPAO(WorldEdit.objs).params.updateBase(edit_path); // updateBase for all objects in case they use this object
      REPAO(WorldEdit.objs).setMeshPhys();
      paramEditObjChanged(&obj.id);
      ObjList.setChanged();
   }
   void ProjectEx::meshChanged(Elm &mesh)
{
      super::meshChanged(mesh);

      if(ObjEdit.mesh_elm && ObjEdit.mesh_elm->id==mesh.id)ObjEdit.meshChanged();
      if(AnimEdit.mesh_id==mesh.id)AnimEdit.setMeshSkel(true);
      if(Selection.elms() && Selection[0].mesh_proper.id()==mesh.id)WorldEdit.param_edit.meshVariationChanged();
   }
   void ProjectEx::elmChanging(Elm &elm)
   {
      switch(elm.type)
      {
         case ELM_ANIM: AnimEdit.elmChanging(elm.id); break;
      }
   }
   void ProjectEx::elmChanged(Elm &elm)
   {
      switch(elm.type)
      {
         case ELM_ANIM       :     setAnimTransform    (elm); setAnimParams(elm); AnimEdit.elmChanged(elm.id); break;
         case ELM_SKEL       :     setSkelTransform    (elm); skelChanged(elm); break;
         case ELM_PHYS       : if(!setPhysTransform    (elm))physChanged(elm); setPhysParams(elm); break;
         case ELM_MESH       :     meshTransformChanged(elm); break;
         case ELM_MTRL       : mtrlSetAutoTanBin(elm.id);                MtrlEdit.elmChanged(elm.id); if(MaterialPtr mtrl=MaterialPtr().find(gamePath(elm.id)))D.setShader(mtrl()); break;
         case ELM_WATER_MTRL :                                      WaterMtrlEdit.elmChanged(elm.id); break;
         case ELM_PHYS_MTRL  :                                       PhysMtrlEdit.elmChanged(elm.id); break;
         case ELM_ENUM       : enumChanged(elm.id);                      EnumEdit.elmChanged(elm.id); break;
         case ELM_IMAGE      :                                          ImageEdit.elmChanged(elm.id); break;
         case ELM_IMAGE_ATLAS:                                     ImageAtlasEdit.elmChanged(elm.id); break;
         case ELM_ICON_SETTS :                                      IconSettsEdit.elmChanged(elm.id); break;
         case ELM_FONT       : fontChanged();                            FontEdit.elmChanged(elm.id); break;
         case ELM_TEXT_STYLE :                                      TextStyleEdit.elmChanged(elm.id); break;
         case ELM_PANEL_IMAGE: panelImageChanged(elm.id);          PanelImageEdit.elmChanged(elm.id); break;
         case ELM_PANEL      : panelChanged(elm.id);                    PanelEdit.elmChanged(elm.id); break;
         case ELM_GUI_SKIN   :                                        GuiSkinEdit.elmChanged(elm.id); break;
         case ELM_GUI        :                                            GuiEdit.elmChanged(elm.id); break;
         case ELM_OBJ        :                      objChanged(elm);                                  break;
         case ELM_OBJ_CLASS  : enumChanged(elm.id); objChanged(elm); ObjClassEdit.elmChanged(elm.id); break;
         case ELM_SOUND      :                                          SoundEdit.elmChanged(elm.id); break;
         case ELM_VIDEO      :                                          VideoEdit.elmChanged(elm.id); break;
         case ELM_ENV        :                                            EnvEdit.elmChanged(elm.id); break;
         case ELM_WORLD      :                                          WorldEdit.elmChanged(elm.id); break;
         case ELM_MINI_MAP   :                                        MiniMapEdit.elmChanged(elm.id); break;
         case ELM_APP        :                                       AppPropsEdit.elmChanged(elm.id); break;
      }
      IconEdit.elmChanged(elm.id); // process at the end (after all stuff was done)
       ObjEdit.elmChanged(elm.id); // process at the end (after all stuff was done, like mesh transforms)
       Preview.elmChanged(elm.id); // process at the end (after all stuff was done)
   }
   void ProjectEx::receivedData(Elm &elm, File &data, File &extra)
   {
      elmChanging(elm);
      if(ElmHasFile(elm.type))
      {
         Str path;
         if(ElmSendBoth(elm.type))
         {
            path=editPath(elm); if(SafeOverwrite(data , path)) SavedEdit(elm.type, path);
            path=gamePath(elm); if(SafeOverwrite(extra, path)){SavedGame(elm.type, path); savedGame(elm, path);}
         }else
         {
            path=basePath(elm); if(SafeOverwrite(data , path))SavedBase(elm.type, path);
         }
      }
      data.pos(0); makeGameVer(elm, &data);
      elmChanged(elm);
   }
   void ProjectEx::syncElm(Elm &elm, Elm &src, File &src_data, File &src_extra, bool sync_long)
   {
      bool elm_newer_src, src_newer_elm;
      closeElm(elm.id);
      elmChanging(elm);
      UID       body_id;
      OBJ_PATH  path_mode;
      ObjectPtr obj; TerrainObj2 terrain; PhysPath phys;
      bool      world_valid=false; PathSettings path_settings;
      switch(elm.type)
      {
         case ELM_MESH     : body_id=elm.meshData()->body_id; break;
         case ELM_OBJ_CLASS: path_mode=elm.objClassData()->pathSelf(); break;
         case ELM_OBJ      : obj=gamePath(elm); terrain=*obj; phys=*obj; break;
         case ELM_WORLD    : world_valid=elm.worldData()->valid(); elm.worldData()->copyTo(path_settings); break;
      }
      if(super::syncElm(elm, src, src_data, src_extra, sync_long, elm_newer_src, src_newer_elm))
      {
         elmChanged(elm);
         switch(elm.type)
         {
            case ELM_MESH:
            {
               if(elm.meshData()->body_id!=body_id)meshTransformChanged(elm, true); // TODO: this is already called in 'elmChanged' above, however without checks for body, the right solution would be to call it only one time with body checks
            }break;

            case ELM_OBJ_CLASS:
            {
               if(elm.objClassData()->pathSelf()!=path_mode)verifyPathsForObjClass(elm.id); // if syncing changed the 'path_mode' then verify world paths
            }break;

            case ELM_OBJ:
            {
               if(TerrainObj2(*obj)!=terrain)rebuildEmbedForObj(elm.id      ); // if syncing changed the 'terrainObj' then rebuild world embedded objects
               if(PhysPath   (*obj)!=phys   )rebuildPathsForObj(elm.id, true); // if syncing changed the 'physPath'   then rebuild world paths, rebuild only for objects that don't override paths (if they override then it means that changing the base doesn't affect their path mode), we must rebuild this also for objects with final path mode set to ignore, in case we've just disabled paths
            }break;

            case ELM_WORLD:
            {
               PathSettings temp; elm.worldData()->copyTo(temp); if(path_settings!=temp)rebuildWorldAreas(elm); // if syncing changed 'path_settings' then paths need to be rebuilt
               if(!world_valid && elm.worldData()->valid()) // if world settings became valid after syncing, then rebuild some elements
                  if(WorldVer *world_ver=worldVerGet(elm.id))
               {
                  REPA(world_ver->lakes)
                  {
                   C UID      &water_id =world_ver->lakes.lockedKey (i);
                     WaterVer &water_ver=world_ver->lakes.lockedData(i); // can't check for 'water_ver.removed' because that depends on preious building which not yet occured
                     Lake lake; if(lake.load(editLakePath(elm.id, water_id)))if(!lake.removed)rebuildWater(&lake, null, water_id, *world_ver);
                  }
                  REPA(world_ver->rivers)
                  {
                   C UID      &water_id =world_ver->rivers.lockedKey (i);
                     WaterVer &water_ver=world_ver->rivers.lockedData(i); // can't check for 'water_ver.removed' because that depends on preious building which not yet occured
                     River river; if(river.load(editRiverPath(elm.id, water_id)))if(!river.removed)rebuildWater(null, &river, water_id, *world_ver);
                  }
               }
            }break;
         }
      }

      // request from server
      if(!sync_long && (src.newerFile(elm) || src_newer_elm))Server.getElmLong(elm.id); // if server has newer file version than local then request it

      // send to server
      if(elm.newerFile(src) || elm_newer_src)Server.setElmLong (elm.id);else // if local has newer file version than server then send it
      if(elm.newerData(src)                 )Server.setElmShort(elm.id);     // if local has any data newer     than server then send it
   }
   bool ProjectEx::syncWaypoint(C UID &world_id, C UID &waypoint_id, Version &src_ver, EditWaypoint &src)
{
      // if belongs to currently edited world
      if(world_id.valid() && WorldEdit.elm_id==world_id)
      {
         if(Version *waypoint_ver=WorldEdit.ver->waypoints.get(waypoint_id))
            if(src_ver!=*waypoint_ver)
               if(EditWaypoint *waypoint=WorldEdit.waypoints.get(waypoint_id))
         {
            bool changed=waypoint->sync(src);
            if(  changed){WorldEdit.setChangedWaypoint(waypoint); WorldEdit.setVisibleWaypoints(); WaypointList.setChanged();}
            if(waypoint->equal(src)){*waypoint_ver=src_ver; WorldEdit.ver->changed=true;}else if(changed){waypoint_ver->randomize(); WorldEdit.ver->changed=true;}
            return changed;
         }
         return false;
      }

      EditWaypoint server =src;
      bool         changed=super::syncWaypoint(world_id, waypoint_id, src_ver, src); // this modifies 'src_ver' and 'src'
      if(src.newer(server))Server.setWaypoint(world_id, waypoint_id, src_ver, src); // if after syncing local waypoint is newer than server, then send its data to the server
      return changed;
   }
   bool ProjectEx::syncLake(C UID &world_id, C UID &lake_id, Version &src_ver, Lake &src)
{
      // if belongs to currently edited world
      if(world_id.valid() && WorldEdit.elm_id==world_id)
      {
         if(WaterVer *lake_ver=WorldEdit.ver->lakes.get(lake_id))
            if(src_ver!=lake_ver->ver)
               if(Lake *lake=WorldEdit.lakes.get(lake_id))
         {
            bool changed=lake->sync(src);
            if(  changed){WorldEdit.setChangedLake(lake); WorldEdit.setVisibleWaters();}
            if(lake->equal(src)){lake_ver->ver=src_ver; WorldEdit.ver->changed=true;}else if(changed){lake_ver->ver.randomize(); WorldEdit.ver->changed=true;}
            return changed;
         }
         return false;
      }

      Lake         server =src;
      bool         changed=super::syncLake(world_id, lake_id, src_ver, src); // this modifies 'src_ver' and 'src'
      if(src.newer(server))Server.setLake(world_id, lake_id, src_ver, src); // if after syncing local lake is newer than server, then send its data to the server
      return changed;
   }
   bool ProjectEx::syncRiver(C UID &world_id, C UID &river_id, Version &src_ver, River &src)
{
      // if belongs to currently edited world
      if(world_id.valid() && WorldEdit.elm_id==world_id)
      {
         if(WaterVer *river_ver=WorldEdit.ver->rivers.get(river_id))
            if(src_ver!=river_ver->ver)
               if(River *river=WorldEdit.rivers.get(river_id))
         {
            bool changed=river->sync(src);
            if(  changed){WorldEdit.setChangedRiver(river); WorldEdit.setVisibleWaters();}
            if(river->equal(src)){river_ver->ver=src_ver; WorldEdit.ver->changed=true;}else if(changed){river_ver->ver.randomize(); WorldEdit.ver->changed=true;}
            return changed;
         }
         return false;
      }

      River        server =src;
      bool         changed=super::syncRiver(world_id, river_id, src_ver, src); // this modifies 'src_ver' and 'src'
      if(src.newer(server))Server.setRiver(world_id, river_id, src_ver, src); // if after syncing local lake is newer than server, then send its data to the server
      return changed;
   }
   void ProjectEx::hmDel(C UID &world_id, C VecI2 &area_xy, C TimeStamp *time)
{
      if(WorldEdit.elm_id==world_id)WorldEdit.hmDel(          area_xy, time);
      else                          super    ::hmDel(world_id, area_xy, time);
   }
   Heightmap* ProjectEx::hmGet(C UID &world_id, C VecI2 &area_xy, Heightmap &temp)
{
      // check if the world is currently being edited
      if(world_id.valid() && WorldEdit.elm_id==world_id)
         if(Area *area=WorldEdit.findAreaLoaded(area_xy))return area->hm;

      return super::hmGet(world_id, area_xy, temp);
   }
   uint ProjectEx::hmUpdate(C UID &world_id, C VecI2 &area_xy, uint area_sync_flag, C AreaVer &src_area, Heightmap &src_hm)
{
      if(area_sync_flag)
      {
         if(world_id.valid() && WorldEdit.elm_id==world_id && WorldEdit.ver)
            if(Cell<Area> *cell    =WorldEdit.     grid.find(area_xy))
            if(AreaVer    *area_ver=WorldEdit.ver->areas.get (area_xy))
         {
            Area &area=*cell->data(); if(area.loaded)
            {
               uint synced=0;
               bool loaded=area.loaded;
               area.load();
               if(!area.hm)New(area.hm);
               if(synced=area_ver->sync(src_area, *area.hm, src_hm, area_sync_flag))
               {
                  area.setChanged();
                  WorldEdit.ver->rebuildArea(area_xy, synced);
               }
               if(!loaded)area.unload();
               return synced;
            }
         }
         return super::hmUpdate(world_id, area_xy, area_sync_flag, src_area, src_hm);
      }
      return 0;
   }
   void ProjectEx::objGet(C UID &world_id, C VecI2 &area_xy, C Memc<UID> &obj_ids, Memc<ObjData> &objs)
{
      if(world_id.valid() && WorldEdit.elm_id==world_id)
         if(Area *area=WorldEdit.findAreaLoaded(area_xy))
      {
         REPA(area->objs)if(Obj *obj=area->objs[i]) // iterate all objects in the area
            if(obj_ids.binaryHas(obj->id)) // if this is wanted object
               objs.New()=*obj; // copy to output container
         return;
      }
      super::objGet(world_id, area_xy, obj_ids, objs);
   }
   bool ProjectEx::syncObj(C UID &world_id, C VecI2 &area_xy, Memc<ObjData> &objs, Map<VecI2, Memc<ObjData>> *obj_modified, Memc<UID> *local_newer)
{
      if(world_id.valid() && WorldEdit.elm_id==world_id && WorldEdit.ver)
      {
         bool        loaded_areas_changed=false; // if objects of loaded areas have changed
         Memt<VecI2> unload_areas; // areas to unload after processing
         Area       &target_area=*WorldEdit.getArea(area_xy); if(!target_area.loaded)unload_areas.binaryInclude(target_area.xy);
         REPA(objs)
         {
            ObjData &obj=objs[i];
            ObjVer  *obj_ver=WorldEdit.ver->obj.find(obj.id); // use 'find' to get null if not found
            if(!obj_ver) // not present in this world, then add to target area
            {
               bool area_loaded=!unload_areas.binaryHas(target_area.xy); // if area is loaded (and won't be unloaded)
               Obj &cur_obj=WorldEdit.objs.New(); cur_obj.id=obj.id;
               cur_obj.sync(obj, edit_path);
               cur_obj.attach(WorldEdit, &target_area); // call before 'setChangedEmbed'
               WorldEdit.ver->changedObj(cur_obj, target_area.xy); // call before 'setChangedEmbed'

               cur_obj.setChangedEmbed(); // call after 'attach', 'changedObj' and before 'setChangedPaths'
               if(cur_obj.physPath())cur_obj.setChangedPaths(); // call after 'setChangedEmbed'
               target_area.setChanged(); if(AreaVer *area_ver=target_area.getVer())area_ver->obj_ver.randomize();

               if(area_loaded){loaded_areas_changed=true; target_area.delayedValidateRefs();}
            }else
            {
               Area &cur_area=*WorldEdit.getArea(obj_ver->area_xy); if(!cur_area.loaded)unload_areas.binaryInclude(cur_area.xy);
               bool  area_loaded=(!unload_areas.binaryHas(target_area.xy) || !unload_areas.binaryHas(cur_area.xy)); // if area is loaded (and won't be unloaded)
               cur_area.load();
               REPA(cur_area.objs)if(cur_area.objs[i]->id==obj.id) // found that object
               {
                  Obj        &cur_obj              =*cur_area.objs[i];
                  uint        old_mesh_variation_id= cur_obj.params.mesh_variation_id;
                  TerrainObj  old_terrain          = cur_obj.terrainObj();
                  PhysPath    old_phys             = cur_obj.physPath();
                  TimeStamp   old_matrix_time      = cur_obj.matrix_time; // remember old matrix time before syncing
                  if(cur_obj.sync(obj, edit_path))
                  {
                     bool  changed_matrix   =(cur_obj.matrix_time>old_matrix_time),
                           changed_variation=(cur_obj.params.mesh_variation_id!=old_mesh_variation_id),
                           changed_embed    =(cur_obj.terrainObj()!=old_terrain ||  changed_matrix || changed_variation), // if changed terrain, matrix or mesh variation
                           changed_phys_path=(cur_obj.physPath  ()!=old_phys    || (changed_matrix && (old_phys || cur_obj.physPath()))); // if changed phys, or if changed matrix and have phys
                     Area &new_area=(changed_matrix ? target_area : cur_area); // check if received newer matrix
                     WorldEdit.ver->changedObj(cur_obj, new_area.xy); // call before 'setChangedEmbed'

                     if(changed_embed)cur_obj.setChangedEmbed(&new_area.xy); // call after 'changedObj' and before 'setChangedPaths' (use new_area coordinates)
                     cur_area.setChanged(); if(changed_phys_path)cur_obj.setChangedPaths(); if(AreaVer *area_ver=cur_area.getVer())area_ver->obj_ver.randomize(); // call 'setChangedPaths' after 'setChangedEmbed' for old area
                     cur_obj.attach(WorldEdit, &new_area); // attach to new area
                     new_area.setChanged(); if(changed_phys_path)cur_obj.setChangedPaths(); if(AreaVer *area_ver=new_area.getVer())area_ver->obj_ver.randomize(); // call 'setChangedPaths' after 'setChangedEmbed' for new area

                     if(area_loaded){loaded_areas_changed=true; cur_area.delayedValidateRefs(); new_area.delayedValidateRefs();}
                  }
                  break;
               }
            }
         }
         Memc<ObjData> target_objs; REPA(target_area.objs)target_objs.New()=*target_area.objs[i]; // copy before unloading
         if(local_newer)GetNewer(target_objs, objs, *local_newer);
         REPA(unload_areas)if(Area *area=WorldEdit.findArea(unload_areas[i]))if(!area->hasSelectedObj())area->unload(); // unload areas
         if(loaded_areas_changed)ObjList.setChanged();
         return Same(target_objs, objs);
      }
      return super::syncObj(world_id, area_xy, objs, obj_modified, local_newer);
   }
   void ProjectEx::syncCodes()
   {
      CodeEdit.overwriteChanges(); // first overwrite any code changes
      Memc<ElmTypeVer> elm_type_vers; FREPA(elms) // get list of project apps and codes
      {
         Elm &elm=elms[i];
         if(elm.type==ELM_APP || elm.type==ELM_CODE)elm_type_vers.New().set(elm);
      }
      Server.syncCodes(elm_type_vers);
   }
   void ProjectEx::syncCodes(C Memc<ElmCodeData> &elm_code_datas)
   {
      bool              new_source=false; // if downloaded a new source
      Memc<ElmCodeData> send;
      FREPA(elm_code_datas)
      {
       C ElmCodeData &ecd=elm_code_datas[i]; if(Elm *elm=findElm(ecd.id, ecd.type))switch(ecd.type)
         {
            case ELM_APP:
            {
               if(ElmApp *app_data=elm->appData())
               {
                  if(app_data->sync (ecd.app, true))AppPropsEdit.elmChanged(elm->id);
                  if(app_data->newer(ecd.app      ))send.New().set(*elm); // if after syncing, local copy has newer data than server, then include it in sending
               }
            }break;

            case ELM_CODE:
            {
               if(ElmCode *code_data=elm->codeData())
               {
                  Code code; LoadCode(code.current, codePath(*elm)); LoadCode(code.base, codeBasePath(*elm)); // load code from this computer
                  Str  latest; if(CodeEdit.sourceDataGet(elm->id, latest))Swap(code.current, latest); // update to latest data taken from Code Editor, in case user made a change between network data exchange
                  if(ecd.ver) // if server data is valid then merge
                  {
                     if(!code_data->ver)new_source=true; // if local version didn't exist then enable 'new_source'
                     Str merged=(Server.canWriteCode() ? Merge(code.base, code.current, ecd.code.current) : ecd.code.current); // if can't change code then just grab version from the server
                     if(Equal(merged, ecd.code.current, true))code_data->ver=ecd.ver;else if(!Equal(merged, code.current, true))code_data->newVer(); // if same as server then set ver from server, else if changed then set new ver
                     code.current=merged;
                     code.base   =ecd.code.current; // new base is latest server current
                     SaveCode(code.current, codePath(*elm)); SaveCode(code.base, codeBasePath(*elm)); // save code to this computer
                     CodeEdit.sourceDataSet(elm->id, code.current); // update Code Editor with latest source
                  }
                  if(Server.canWriteCode()
                  && !Equal(code.current, ecd.code.current, true))send.New().set(*elm, &code); // if after merging, local is different than server, then include it in sending
               }
            }break;
         }
      }
      if(send.elms())Server.syncCodes(send); // if we have different elements, then send them
      if(new_source)activateSources(); // if we've got a brand new source then reactivate sources in case it's used by the application
   }
   void ProjectEx::syncCodes(C Memc<ElmCodeBase> &elm_code_bases, bool resync)
   {
      FREPA(elm_code_bases)
      {
       C ElmCodeBase &ecb=elm_code_bases[i];
         SaveCode(ecb.data, codeBasePath(ecb.id)); // here we save to base, as it is on the server
      }
      if(resync)syncCodes();
   }
   bool ProjectEx::codeGet(C UID &elm_id, Str &data)
   {
      if(CodeEdit.sourceDataGet(elm_id, data))return true;
      if(Elm *elm=findElm(elm_id, ELM_CODE))return LoadCode(data, codePath(elm_id))==Edit::EE_ERR_NONE;
      data.clear(); return false;
   }
   bool ProjectEx::codeSet(C UID &elm_id, C Str &data)
   {
      if(Elm *elm=findElm(elm_id, ELM_CODE))
      {
         elmChanging(*elm);
         if(SaveCode(data, codePath(elm_id)))
         {
            elm->codeData()->newVer();
            CodeEdit.sourceDataSet(elm_id, data);
            elmChanged(*elm);
            // don't send to server as codes are synced manually
            return true;
         }
      }
      return false;
   }
   bool ProjectEx::fileRead(C UID &elm_id, File &f)
   {
      if(Elm *elm=findElm(elm_id))switch(elm->type)
      {
         case ELM_FILE :
         case ELM_SOUND:
         case ELM_VIDEO: return f.readTry(gamePath(elm->id));
      }
      return false;
   }
   bool ProjectEx::fileSet(C UID &elm_id, File &f)
   {
      if(Elm *elm=findElm(elm_id))switch(elm->type)
      {
         case ELM_FILE :
         case ELM_SOUND:
         case ELM_VIDEO:
         {
            closeElm(elm_id);
            elmChanging(*elm);
            if(SafeOverwrite(f, gamePath(elm->id)))
            {
               elm->Data()->newVer();
               if(ElmFile  *data=elm-> fileData())data->file_time.getUTC();
               if(ElmSound *data=elm->soundData())data->file_time.getUTC();
               if(ElmVideo *data=elm->videoData())data->file_time.getUTC();
               elmChanged(*elm);
               savedGame(*elm);
               Server.setElmLong(elm->id);
               return true;
            }
         }break;
      }
      return false;
   }
   bool ProjectEx::imageGet(C UID &elm_id, Image &image)
   {
      if(Elm *elm=findElm(elm_id, ELM_IMAGE))return image.ImportTry(editPath(elm_id));
      image.del(); return false;
   }
   bool ProjectEx::imageSet(C UID &elm_id, File &image, bool has_color, bool has_alpha)
   {
      if(Elm *elm=findElm(elm_id, ELM_IMAGE))if(ElmImage *image_data=elm->imageData())
      {
         elmChanging(*elm);
         if(SafeOverwrite(image, editPath(elm->id)))
         {
            image_data->newVer();
            image_data->file_time.getUTC();
            image_data->hasColor(has_color);
            image_data->hasAlpha(has_alpha);
            makeGameVer(*elm);
            elmChanged (*elm);
            Server.setElmLong(elm->id);
            return true;
         }
      }
      return false;
   }
   bool ProjectEx::imageSet(C UID &elm_id, C Image &image)
   {
      if(Elm *elm=findElm(elm_id, ELM_IMAGE))
      {
         File f; f.writeMem(); if((image.mode()==IMAGE_3D) ? image.save(f) : image.ExportWEBP(f, 1, 1))
         {
            f.pos(0); return imageSet(elm_id, f, HasColor(image), HasAlpha(image));
         }
      }
      return false;
   }
   bool ProjectEx::meshSet(C UID &elm_id, File &data)
   {
      Elm *elm=findElm(elm_id);
      if(elm)if(ElmObj  * obj_data=elm-> objData())elm=getObjMeshElm(elm->id); // if this is an object then get its mesh
      if(elm)if(ElmMesh *mesh_data=elm->meshData()) // mesh
      {
         Mesh mesh; if(mesh.load(data, game_path))
         {
          //mesh.setBase(); don't set base in case some mesh parts were deliberately deleted
            mesh.delRender().keepOnly(EditMeshFlagAnd).skeleton(null);
            CleanMesh(mesh);
            mesh_data->draw_group_id=findElmID(Enums.id(mesh.drawGroupEnum()), ELM_ENUM);
            mesh.drawGroupEnum(Enums(gamePath(mesh_data->draw_group_id))); // fix draw group enum
            // fix materials
            REP(mesh.lods())
            {
               MeshLod &lod=mesh.lod(i); REPA(lod)
               {
                  MeshPart &part=lod.parts[i];
                  REP(part.variations())if(i)part.variation(i, gamePath(findElmID(part.variation(i).id(), ELM_MTRL)));
                  Str mtrls[4]; REPAO(mtrls)=gamePath(findElmID(part.multiMaterial(i).id(), ELM_MTRL)); part.multiMaterial(mtrls[0], mtrls[1], mtrls[2], mtrls[3]);
               }
            }
            Save(mesh, editPath(elm->id), game_path);
            makeGameVer(*elm);
            mesh_data->file_time      .getUTC();
            mesh_data->draw_group_time.getUTC();
          //mesh_data.from(mesh); already called through 'makeGameVer'
            mesh_data->newVer();
            if(ObjEdit.mesh_elm==elm)ObjEdit.reloadMeshSkel();
            meshChanged(*elm);
            Server.setElmLong(elm->id);
            return true;
         }
      }
      return false;
   }
   bool ProjectEx::physSet(C UID &elm_id, File &data)
   {
      Elm *elm=findElm(elm_id);
      if(elm)if(ElmObj  * obj_data=elm-> objData())elm=getObjPhysElm(elm->id); // if this is an object then get its phys
      if(elm)if(ElmPhys *phys_data=elm->physData()) // phys
      {
         PhysBody phys; if(phys.load(data, game_path))
         {
            phys_data->mtrl_id=findElmID(PhysMtrls.id(phys.material), ELM_PHYS_MTRL);
            phys.material=PhysMtrls(gamePath(phys_data->mtrl_id)); // fix phys material
            Save(phys, gamePath(elm->id), game_path);
            makeGameVer(*elm);
            phys_data->   file_time.getUTC();
            phys_data->   mtrl_time.getUTC();
            phys_data->density_time.getUTC(); phys_data->density=phys.density;
            phys_data->from(phys);
            phys_data->newVer();
            if(ObjEdit.phys_elm==elm)ObjEdit.reloadPhys();
            physChanged(*elm);
            Server.setElmLong(elm->id);
            return true;
         }
      }
      return false;
   }
   bool ProjectEx::skelSetSlots(C UID &elm_id, C CMemPtr<Edit::SkeletonSlot> &skel_slots)
   {
      Elm *elm=findElm(elm_id);
      if(elm)if(ElmObj  * obj_data=elm-> objData())elm=getObjSkelElm(elm->id); // if this is an object then get its skeleton
      if(elm)if(ElmSkel *skel_data=elm->skelData()) // skel
      {
         if(ObjEdit.skel_elm==elm && ObjEdit.mesh_skel)
         {
            ObjEdit.mesh_undos.set();
            ObjEdit.mesh_skel->slots.setNum(skel_slots.elms()); REPAO(skel_slots).setTo(ObjEdit.mesh_skel->slots[i], *ObjEdit.mesh_skel);
            ObjEdit.setChangedSkel(false);
            return true;
         }else
         {
            Skeleton skel; if(skelGet(elm->id, skel))
            {
               skel.slots.setNum(skel_slots.elms()); REPAO(skel_slots).setTo(skel.slots[i], skel);
               Save(skel, gamePath(elm->id));
               skel_data->file_time.getUTC();
               skel_data->newVer();
               skelChanged(*elm);
               Server.setElmLong(elm->id);
               return true;
            }
         }
      }
      return false;
   }
   bool ProjectEx::modifyObj(C UID &obj_id, C MemPtr<Edit::ObjChange> &changes)
   {
      if(Elm *elm=findElm(obj_id))
      if(elm->type==ELM_OBJ || elm->type==ELM_OBJ_CLASS)
      if(EditObjectPtr params=editPath(obj_id))
      {
         elmChanging(*elm);
         if(params->modify(changes, T))
         {
            if(ElmObj      *obj_data      =elm->objData     ()){obj_data      ->newVer(); obj_data      ->from(*params);}
            if(ElmObjClass *obj_class_data=elm->objClassData()){obj_class_data->newVer(); obj_class_data->from(*params);}
            Save(*params, editPath(obj_id)); // save edit
            makeGameVer(*elm);
            elmChanged (*elm);
            Server.setElmLong(obj_id);
         }
         return true;
      }
      return false;
   }
   bool ProjectEx::codeCheck()
   {
      if(!valid()){Gui.msgBox(S, "Project is empty"); return false;}
      return true;
   }
   Str ProjectEx::codeSyncPath()
   {
      return ProjectsPath+ProjectsCodePath+EncodeFileName(id).tailSlash(true);
   }
   void ProjectEx::codeExplore() {if(codeCheck()){Str path=codeSyncPath(); FCreateDirs(path); Explore(path);}}
   bool ProjectEx::codeExport(bool gui)
   {
      if(codeCheck())
      {
         bool ok=true;
         Str path=codeSyncPath(); FCreateDirs(path);
         REPA(elms)
         {
            Elm &elm=elms[i]; if(elm.type==ELM_CODE && elm.finalExists())
            {
               // access code data
               Str data; codeGet(elm.id, data);
               // save to file
               Str name=path+EncodeFileName(elm.id)+CodeSyncExt;
               FileText f; if(f.write(name, HasUnicode(data) ? UTF_8 : ANSI))
               {
                  f.putText(data);
                  ok&=f.flushOK();
               }else
               {
                  if(gui)Gui.msgBox(S, S+"Can't write to \""+name+"\"");
                  ok=false;
                  break;
               }
            }
         }
         if(gui)Explore(path);
         return ok;
      }
      return false;
   }
   bool ProjectEx::codeImport(bool gui)
   {
      if(codeCheck())
      {
         bool ok=true;
         if(gui)ImportCode.updates.clear();

         Str path=codeSyncPath();
         REPA(elms)
         {
            Elm &elm=elms[i]; if(elm.type==ELM_CODE && elm.finalExists())
            {
               // load from file
               Str code, name=path+EncodeFileName(elm.id)+CodeSyncExt;
               FileText f; if(f.read(name))
               {
                  f.getAll(code);

                  // access code data
                  Str data; codeGet(elm.id, data);

                  // if different
                  if(!Equal(data, code, true))
                  {
                     if(gui)Swap(ImportCode.updates.New().set(elm.id).data, code); // remember for updating with gui
                     else   ok&=codeSet(elm.id, code); // update instantly
                  }
               }
            }
         }

         if(gui)ImportCode.import();
         return ok;
      }
      return false;
   }
   void ProjectEx::flush(SAVE_MODE save_mode)
{
      super         ::flush(save_mode);
      ImageEdit     .flush();
      ImageAtlasEdit.flush();
      IconSettsEdit .flush();
      IconEdit      .flush();
      EnumEdit      .flush();
      FontEdit      .flush();
      TextStyleEdit .flush();
      PanelImageEdit.flush();
      PanelEdit     .flush();
      GuiSkinEdit   .flush();
      GuiEdit       .flush();
      ObjClassEdit  .flush();
      SoundEdit     .flush();
      VideoEdit     .flush();
      AnimEdit      .flush();
      MtrlEdit      .flush();
      WaterMtrlEdit .flush();
      PhysMtrlEdit  .flush();
      ObjEdit       .flush(save_mode);
      WorldEdit     .flush();
      MiniMapEdit   .flush();
      EnvEdit       .flush();
      CodeEdit      .flush();
      AppPropsEdit  .flush();
   }
   void ProjectEx::flushElm(C UID &elm_id)
   {
      if(ImageEdit     .elm_id==elm_id)ImageEdit     .flush();
      if(ImageAtlasEdit.elm_id==elm_id)ImageAtlasEdit.flush();
      if(IconSettsEdit .elm_id==elm_id)IconSettsEdit .flush();
      if(IconEdit      .elm_id==elm_id)IconEdit      .flush();
      if(EnumEdit      .elm_id==elm_id)EnumEdit      .flush();
      if(FontEdit      .elm_id==elm_id)FontEdit      .flush();
      if(TextStyleEdit .elm_id==elm_id)TextStyleEdit .flush();
      if(PanelImageEdit.elm_id==elm_id)PanelImageEdit.flush();
      if(PanelEdit     .elm_id==elm_id)PanelEdit     .flush();
      if(GuiSkinEdit   .elm_id==elm_id)GuiSkinEdit   .flush();
      if(GuiEdit       .elm_id==elm_id)GuiEdit       .flush();
      if(ObjClassEdit  .elm_id==elm_id)ObjClassEdit  .flush();
      if(SoundEdit     .elm_id==elm_id)SoundEdit     .flush();
      if(VideoEdit     .elm_id==elm_id)VideoEdit     .flush();
      if(AnimEdit      .elm_id==elm_id)AnimEdit      .flush();
                                       MtrlEdit      .flush(elm_id);
      if(WaterMtrlEdit .elm_id==elm_id)WaterMtrlEdit .flush();
      if(PhysMtrlEdit  .elm_id==elm_id)PhysMtrlEdit  .flush();
      if(AppPropsEdit  .elm_id==elm_id)AppPropsEdit  .flush();
      if(EnvEdit       .elm_id==elm_id)EnvEdit       .flush();
      if(MiniMapEdit   .elm_id==elm_id)MiniMapEdit   .flush();
      if(ObjEdit. obj_elm && ObjEdit. obj_elm->id==elm_id)ObjEdit.flushObj ();
      if(ObjEdit.mesh_elm && ObjEdit.mesh_elm->id==elm_id
      || ObjEdit.skel_elm && ObjEdit.skel_elm->id==elm_id)ObjEdit.flushMeshSkel();
      if(ObjEdit.phys_elm && ObjEdit.phys_elm->id==elm_id)ObjEdit.flushPhys();
      CodeEdit.sourceOverwrite(elm_id);
   }
   void ProjectEx::getFileSizes()
   {
      if(list.file_size && !file_size_getter.created())file_size_getter.get(game_path);
   }
   void ProjectEx::savedTex(C UID &tex_id, int size) {TexInfos(tex_id)->file_size=size; /*TIG.savedTex(tex_id);*/}
   bool  ProjectEx::saveTex(C Image &img, C UID &tex_id)
   {
      File f; img.save(f.writeMem()); f.pos(0);
      Str name=texPath(tex_id); if(SafeOverwrite(f, name)){savedTex(tex_id, f.size()); Saved(img, name); return true;}
      return false;
   }
   void ProjectEx::pauseServer()
   {
      Server.clearProj();
      Server.projectOpen(UIDZero, S);
   }
   void ProjectEx::resumeServer()
   {
      if(!needUpdate() // can't connect to the server when the project needs to be updated first
      && synchronize)  // connect only if synchronization is enabled
      {
         Server.projectOpen(id, name);
         Server.projectDataRequest();
      }
   }
   LOAD_RESULT ProjectEx::open(C UID &id, C Str &name, C Str &path, Str &error, bool ignore_lock)
{
      save_time=Time.appTime()+AutoSaveTime; // when opening new project, make sure that auto save won't be triggered too quickly
      LOAD_RESULT result=super::open(id, name, path, error, ignore_lock);
      if(LoadOK(result))
      {
                             WorldEdit.setHmMtrl   (      hm_mtrl_id   );
                             WorldEdit.setWaterMtrl(   water_mtrl_id   );
         FREPA(mtrl_brush_id)MtrlBrush.setMaterial (i, mtrl_brush_id[i]);

         setList(); // this will also set 'Elm.finalRemoved' and 'Elm.finalPublish'
         enumChanged(); // before 'activateSources' because this generates C++ auto header
         fontChanged();
         activateSources(CodeEdit.initialized() ? 1 : -1); // set active sources but without rebuilding so code editor doesn't need to be loaded every time (unless it's already loaded), call after 'enumChanged'
         Importer.opened(T, root); // call after setting list because may rely on hierarchy
         resumeServer();
         // load all world versions so builder knows what to process
         FREPA(elms){Elm &elm=elms[i]; if(elm.type==ELM_WORLD)worldVerGet(elm.id);}
         getFileSizes();
         //if(list.tex_sharpness)TIG.getTexSharpnessFromProject();
      }
      ProjSettings.toGui();
      return result;
   }
   bool ProjectEx::save(SAVE_MODE save_mode)
{
      save_time=Time.appTime()+AutoSaveTime;
      return super::save(save_mode);
   }
   ProjectEx& ProjectEx::del()
{
      file_size_getter.stop();
      //TIG.stop();
      HideProject();

      elm_undos.del();
      mesh_mem.del(); mesh_matrix.identity();
      sound.close();
      lit_elm_id.zero(); lit_elm_name.clear();
      ObjEdit.back_meshes.clear();

         ImportTerrain.clearProj();
               Preview.clearProj();
              ObjPaint.clearProj();
              Importer.clearProj();
                   MSM.clearProj();
                   DST.clearProj();
           TexDownsize.clearProj();
           CreateMtrls.clearProj();
        ConvertToAtlas.clearProj();
      ConvertToDeAtlas.clearProj();

      Builder.stop();

      pauseServer();

      // 'TIG' can still do some processing because it doesn't operate on this project
      file_size_getter_step=false;
      file_size_getter.del();
      super::del(); return T;
   }
   void ProjectEx::close()
{
      // flush and close editors
           ObjEdit.set(null);
          MtrlEdit.set(null);
     WaterMtrlEdit.set(null);
      PhysMtrlEdit.set(null);
         WorldEdit.set(null);
       MiniMapEdit.set(null);
           EnvEdit.set(null);
           GuiEdit.set(null);
          CodeEdit.clear();
          CodeEdit.hideAll();
         ImageEdit.set(null);
    ImageAtlasEdit.set(null);
     IconSettsEdit.set(null);
          IconEdit.set(null);
          EnumEdit.set(null);
          FontEdit.set(null);
     TextStyleEdit.set(null);
    PanelImageEdit.set(null);
         PanelEdit.set(null);
       GuiSkinEdit.set(null);
      ObjClassEdit.set(null);
         SoundEdit.set(null);
         VideoEdit.set(null);
          AnimEdit.set(null);
      AppPropsEdit.set(null);

      super::close();

      setList();
      enumChanged();
      fontChanged();
   }
   void ProjectEx::pause()
   {
         ImageEdit.set(null);
    ImageAtlasEdit.set(null);
     IconSettsEdit.set(null);
          IconEdit.set(null);
          EnumEdit.set(null);
          FontEdit.set(null);
     TextStyleEdit.set(null);
    PanelImageEdit.set(null);
         PanelEdit.set(null);
       GuiSkinEdit.set(null);
      ObjClassEdit.set(null);
         SoundEdit.set(null);
         VideoEdit.set(null);
          AnimEdit.set(null);
          MtrlEdit.set(null);
     WaterMtrlEdit.set(null);
      PhysMtrlEdit.set(null);
      AppPropsEdit.set(null);
          CodeEdit.hideAll();
           EnvEdit.set(null);
       MiniMapEdit.set(null);

      save();

      ImportTerrain.clearProj();
      pauseServer();
      Importer.stop();
   }
   void ProjectEx::resume()
   {
      Importer.investigate(root); // call after setting list because may rely on hierarchy
      resumeServer();
   }
ProjectEx::ProjectEx() : filter_is_id(false), filter_id(UIDZero), lit_elm_id(UIDZero), list_cur(UIDZero), list_cur_item(null), mesh_matrix(1), save_time(0), elm_undos(false, this), texture_changed(false), file_size_getter_step(false) {}

ProjectEx::OuterRegion::OuterRegion() : resize_on(false) {}

ProjectEx::ElmList::ElmList() : ics(ICS_ALWAYS), its(ITS_ELM), show_all_elms(true), show_all_elm_types(true), show_only_folder(false), show_only_obj(false), show_only_mtrl(false), show_only_anim(false), show_only_sound(false), file_size(false), include_texture_size_in_object(false), include_unpublished_elm_size(false), flat_is(false), flat_want(false), list_all_children(false), tapped_open(false), tapped_vis(-1), icon_col(0), name_col(0), size_col(0), tapped_time(0), lit_elm_id(UIDZero) {}

ProjectEx::ElmList::SoundPlay::SoundPlay() : lit_id(UIDZero), play_id(UIDZero) {}

ProjectEx::ElmList::AppCheck::AppCheck() : app_id(UIDZero) {}

ProjectEx::ElmChange::ElmChange() : type(NONE), test(0), set(0) {}

/******************************************************************************/
