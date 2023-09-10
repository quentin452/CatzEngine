/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
// FILE
/******************************************************************************/
bool SafeDel(C Str &name, ReadWriteSync &rws)
{
   if(name.is()){WriteLock wl(rws); return FDelFile(name);} return true;
}
bool SafeOverwrite(File &src, C Str &dest, ReadWriteSync &rws)
{
   return EE::SafeOverwrite(src, dest, null, null, S+'@'+TextHex(Random()), &rws);
}
bool SafeOverwriteChunk(File &src, C Str &dest, ReadWriteSync &rws)
{
   return (src.size()>4) ? SafeOverwrite(src, dest, rws) : SafeDel(dest, rws);
}
bool SafeCopy(C Str &src, C Str &dest)
{
   File f; return f.readStd(src) && EE::SafeOverwrite(f, dest, &NoTemp(FileInfoSystem(src).modify_time_utc), null, S+"@new"+Random());
}
/******************************************************************************/
void RemoveChunk(C Str &file, C Str &chunk, ReadWriteSync &rws)
{
   ReadLock rl(rws);
   File src; if(src.read(file))
   {
      File temp; ChunkWriter cw(temp.writeMem());
      for(ChunkReader cr(src); File *s=cr(); )
         if(!EqualPath(cr.name(), chunk))
            if(File *d=cw.beginChunk(cr.name(), cr.ver()))
               s->copy(*d);
      src.del();
      cw.endChunk();
      temp.pos(0); SafeOverwriteChunk(temp, file, rws);
   }
}
/******************************************************************************/
void SavedImage        (C Str &name) {if(ImagePtr       e=ImagePtr     ().find(name))if(!IsServer)e->load(name);} // on server the file may be compressed
void SavedImageAtlas   (C Str &name) {if(ImageAtlasPtr  e=ImageAtlasPtr().find(name))if(!IsServer)e->load(name);}
void SavedEditSkel     (C Str &name) {}
void SavedSkel         (C Str &name) {if(Skeleton      *e=Skeletons .find(name))if(!IsServer)e->load(name);}
void SavedAnim         (C Str &name) {if(Animation     *e=Animations.find(name))if(!IsServer)e->load(name);}
void SavedMesh         (C Str &name) {if(MeshPtr        e=MeshPtr ().find(name))if(!IsServer){CacheLock cl(Meshes); e->load(name);}}
void SavedEditMtrl     (C Str &name) {}
void SavedEditWaterMtrl(C Str &name) {}
void SavedEditPhysMtrl (C Str &name) {}
void SavedMtrl         (C Str &name) {if(MaterialPtr    e= MaterialPtr().find(name))if(!IsServer)e->load(name);}
void SavedWaterMtrl    (C Str &name) {if(WaterMtrlPtr   e=WaterMtrlPtr().find(name))if(!IsServer)e->load(name);}
void SavedPhysMtrl     (C Str &name) {if(PhysMtrl      *e=PhysMtrls     .find(name))if(!IsServer)e->load(name);}
void SavedEditPhys     (C Str &name) {}
void SavedPhys         (C Str &name) {if(PhysBodyPtr    e=PhysBodyPtr().find(name))if(!IsServer){CacheLock cl(PhysBodies); e->load(name);}}
void SavedEnum         (C Str &name) {if(Enum          *e=Enums        .find(name))if(!IsServer)e->load(name);}
void SavedEditEnum     (C Str &name) {}
void SavedEditObjPar   (C Str &name) {if(EditObjectPtr  e=EditObjectPtr().find(name))if(!IsServer)e->load(name);}
void SavedGameObjPar   (C Str &name) {if(ObjectPtr      e=    ObjectPtr().find(name))if(!IsServer){CacheLock cl(Objects); e->load(name);}}
void SavedGameWayp     (C Str &name) {if(Game::Waypoint *e=Game::Waypoints. find(name))if(!IsServer)e->load(name);}
void SavedFont         (C Str &name) {if(FontPtr        e=      FontPtr().find(name))if(!IsServer)e->load(name);}
void SavedTextStyle    (C Str &name) {if(TextStylePtr   e= TextStylePtr().find(name))if(!IsServer)e->load(name);}
void SavedPanelImage   (C Str &name) {if(PanelImagePtr  e=PanelImagePtr().find(name))if(!IsServer)e->load(name);}
void SavedPanel        (C Str &name) {if(PanelPtr       e=     PanelPtr().find(name))if(!IsServer)e->load(name);}
void SavedGuiSkin      (C Str &name) {if(GuiSkinPtr     e=   GuiSkinPtr().find(name))if(!IsServer)e->load(name);}
void SavedGui          (C Str &name) {}
void SavedEnv          (C Str &name) {if(EnvironmentPtr e=EnvironmentPtr().find(name))if(!IsServer)e->load(name);}

void Saved(C Image           &img , C Str &name) {if(ImagePtr e=ImagePtr().find(name))img.mustCopy(*e);}
void Saved(C ImageAtlas      &img , C Str &name) {SavedImageAtlas(name);}
void Saved(C IconSettings    &icon, C Str &name) {}
void Saved(C EditSkeleton    &skel, C Str &name) {}
void Saved(C     Skeleton    &skel, C Str &name) {if(Skeleton  *e=Skeletons .find(name))*e=skel;}
void Saved(C Animation       &anim, C Str &name) {if(Animation *e=Animations.find(name))*e=anim;}
void Saved(C Mesh            &mesh, C Str &name) {if(MeshPtr    e=MeshPtr ().find(name)){CacheLock cl(Meshes); e->create(mesh).setShader();}}
void Saved(C EditMaterial    &mtrl, C Str &name) {}
void Saved(C EditWaterMtrl   &mtrl, C Str &name) {}
void Saved(C EditPhysMtrl    &mtrl, C Str &name) {}
void Saved(C Material        &mtrl, C Str &name) {if( MaterialPtr e= MaterialPtr().find(name))*e=mtrl;}
void Saved(C WaterMtrl       &mtrl, C Str &name) {if(WaterMtrlPtr e=WaterMtrlPtr().find(name))*e=mtrl;}
void Saved(C PhysMtrl        &mtrl, C Str &name) {SavedPhysMtrl(name);}
void Saved(C PhysBody        &phys, C Str &name) {if(PhysBodyPtr  e=PhysBodyPtr().find(name)){CacheLock cl(PhysBodies); *e=phys;}}
void Saved(C Enum            &enm , C Str &name) {if(Enum        *e=Enums        .find(name))*e=enm;}
void Saved(C EditEnums       &enms, C Str &name) {}
void Saved(C EditObject      &obj , C Str &name) {if(EditObjectPtr e=EditObjectPtr().find(name))*e=obj;}
void Saved(C     Object      &obj , C Str &name) {if(    ObjectPtr e=    ObjectPtr().find(name)){CacheLock cl(Objects); *e=obj;}}
void Saved(C EditWaypoint    &wp  , C Str &name) {}
void Saved(C Game::Waypoint   &wp  , C Str &name) {if(Game::Waypoint *e=Game::Waypoints.find(name))*e=wp;}
void Saved(C EditFont        &font, C Str &name) {}
void Saved(C Font            &font, C Str &name) {SavedFont(name);}
void Saved(C EditTextStyle   &ts  , C Str &name) {}
void Saved(C EditPanelImage  &pi  , C Str &name) {}
void Saved(C PanelImage      &pi  , C Str &name) {SavedPanelImage(name);}
void Saved(C TextStyle       &ts  , C Str &name) {if(TextStylePtr e=TextStylePtr().find(name))*e=ts;}
void Saved(C EditPanel       &panl, C Str &name) {}
void Saved(C Panel           &panl, C Str &name) {if(PanelPtr     e=PanelPtr().find(name))*e=panl;}
void Saved(C EditGuiSkin     &skin, C Str &name) {}
void Saved(C     GuiSkin     &skin, C Str &name) {if(GuiSkinPtr   e=GuiSkinPtr().find(name))*e=skin;}
void Saved(C GuiObjs         &gui , C Str &name) {}
void Saved(C Lake            &lake, C Str &name) {}
void Saved(C River           &rivr, C Str &name) {}
void Saved(C EditEnv         &env , C Str &name) {}
void Saved(C Environment     &env , C Str &name) {if(EnvironmentPtr e=EnvironmentPtr().find(name))*e=env;}
void Saved(C Game::WorldSettings &s, C Str &name) {}

bool Save (C Image           &img , C Str &name                        ) {File f; img .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name)){Saved(img , name); return true;} return false;}
void Save (C ImageAtlas      &img , C Str &name                        ) {File f; img .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(img , name);}
void Save (C IconSettings    &icon, C Str &name                        ) {File f; icon.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(icon, name);}
void Save (C EditSkeleton    &skel, C Str &name                        ) {File f; skel.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(skel, name);}
void Save (C     Skeleton    &skel, C Str &name                        ) {File f; skel.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(skel, name);}
void Save (C Animation       &anim, C Str &name                        ) {File f; anim.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(anim, name);}
void Save (C Mesh            &mesh, C Str &name, C Str &resource_path) {File f; mesh.save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); CacheLock cl(Meshes); if(SafeOverwrite(f, name))Saved(mesh, name);}
void Save (C EditMaterial    &mtrl, C Str &name                        ) {File f; mtrl.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(mtrl, name);}
void Save (C EditWaterMtrl   &mtrl, C Str &name                        ) {File f; mtrl.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(mtrl, name);}
void Save (C EditPhysMtrl    &mtrl, C Str &name                        ) {File f; mtrl.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(mtrl, name);}
void Save (C Material        &mtrl, C Str &name, C Str &resource_path) {File f; mtrl.save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(mtrl, name);}
void Save (C WaterMtrl       &mtrl, C Str &name, C Str &resource_path) {File f; mtrl.save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(mtrl, name);}
void Save (C PhysMtrl        &mtrl, C Str &name                        ) {File f; mtrl.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(mtrl, name);}
void Save (C PhysBody        &phys, C Str &name, C Str &resource_path) {File f; phys.save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); CacheLock cl(PhysBodies); if(SafeOverwrite(f, name))Saved(phys, name);}
void Save (C Enum            &enm , C Str &name                        ) {File f; enm .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(enm , name);}
void Save (C EditEnums       &enms, C Str &name                        ) {File f; enms.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(enms, name);}
void Save (C EditObject      &obj , C Str &name, C Str &resource_path) {File f; obj .save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(obj , name);}
void Save (C     Object      &obj , C Str &name, C Str &resource_path) {File f; obj .save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); CacheLock cl(Objects); if(SafeOverwrite(f, name))Saved(obj, name);}
void Save (C EditWaypoint    &wp  , C Str &name                        ) {File f; wp  .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(wp  , name);}
void Save (C Game::Waypoint   &wp  , C Str &name                        ) {File f; wp  .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(wp  , name);}
void Save (C EditFont        &font, C Str &name                        ) {File f; font.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(font, name);}
void Save (C Font            &font, C Str &name                        ) {File f; font.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(font, name);}
void Save (C EditTextStyle   &ts  , C Str &name                        ) {File f; ts  .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(ts  , name);}
void Save (C TextStyle       &ts  , C Str &name, C Str &resource_path) {File f; ts  .save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(ts  , name);}
void Save (C EditPanelImage  &pi  , C Str &name                        ) {File f; pi  .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(pi  , name);}
void Save (C PanelImage      &pi  , C Str &name                        ) {File f; pi  .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(pi  , name);}
void Save (C EditPanel       &panl, C Str &name                        ) {File f; panl.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(panl, name);}
void Save (C Panel           &panl, C Str &name, C Str &resource_path) {File f; panl.save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(panl, name);}
void Save (C EditGuiSkin     &skin, C Str &name                        ) {File f; skin.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(skin, name);}
void Save (C     GuiSkin     &skin, C Str &name, C Str &resource_path) {File f; skin.save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(skin, name);}
void Save (C GuiObjs         &gui , C Str &name, C Str &resource_path) {File f; gui .save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(gui , name);}
void Save (C Lake            &lake, C Str &name                        ) {File f; lake.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name, WorldAreaSync))Saved(lake, name);}
void Save (C River           &rivr, C Str &name                        ) {File f; rivr.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name, WorldAreaSync))Saved(rivr, name);}
void Save (C EditEnv         &env , C Str &name                        ) {File f; env .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(env , name);}
void Save (C Environment     &env , C Str &name, C Str &resource_path) {File f; env .save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(env , name);}
void Save (C Game::WorldSettings &s, C Str &name, C Str &resource_path) {File f; s   .save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(s   , name);}

bool Load (Mesh       &mesh, C Str &name, C Str &resource_path) {File f; if(f.read(name))return mesh.load(f, resource_path.is() ? resource_path : GetPath(name)); mesh.del  (); return false;}
bool Load (Material   &mtrl, C Str &name, C Str &resource_path) {File f; if(f.read(name))return mtrl.load(f, resource_path.is() ? resource_path : GetPath(name)); mtrl.reset(); return false;}
bool Load (WaterMtrl  &mtrl, C Str &name, C Str &resource_path) {File f; if(f.read(name))return mtrl.load(f, resource_path.is() ? resource_path : GetPath(name)); mtrl.reset(); return false;}
bool Load (EditObject &obj , C Str &name, C Str &resource_path) {File f; if(f.read(name))return obj .load(f, resource_path.is() ? resource_path : GetPath(name)); obj .del  (); return false;}
// other assets either don't use sub-assets, or are stored in game path and don't require "edit->game" path change

bool SaveCode(C Str &code, C Str &name)
{
 //FileText f; f.writeMem(HasUnicode(code) ? UTF_16 : ANSI); // avoid UTF_8 because it's slower to write/read, and as there can be lot of codes, we don't want to sacrifice performance when opening big projects
   FileText f; f.writeMem(HasUnicode(code) ? UTF_8 : ANSI); // FIXME restore above UTF_16 once GitHub supports it, because now it can corrupt files
   f.putText(code);
   return EE::SafeOverwrite(f, name);
}
Edit::ERROR_TYPE LoadCode(Str &code, C Str &name)
{
   FileText f; if(f.read(name)){f.getAll(code); return f.ok() ? Edit::EE_ERR_NONE : Edit::EE_ERR_FILE_READ_ERROR;}
   code.clear(); return Edit::EE_ERR_FILE_NOT_FOUND;
}

void SavedBase(ELM_TYPE type, C Str &path) // called when saved the base version
{
   if(ElmEdit(type))SavedEdit(type, path);
   else             SavedGame(type, path);
}
void SavedCode(C Str &path) // called when saved code
{
   
}
void SavedEdit(ELM_TYPE type, C Str &path) // called when saved the edit version
{
   switch(type)
   {
      case ELM_SKEL      : SavedEditSkel     (path); break;
      case ELM_PHYS      : SavedEditPhys     (path); break;
      case ELM_ENUM      : SavedEditEnum     (path); break;
      case ELM_OBJ_CLASS : SavedEditObjPar   (path); break;
      case ELM_OBJ       : SavedEditObjPar   (path); break;
      case ELM_MESH      : SavedMesh         (path); break;
      case ELM_MTRL      : SavedEditMtrl     (path); break;
      case ELM_WATER_MTRL: SavedEditWaterMtrl(path); break;
      case ELM_PHYS_MTRL : SavedEditPhysMtrl (path); break;
   }
}
void SavedGame(ELM_TYPE type, C Str &path) // called when saved the game version
{
   switch(type)
   {
      case ELM_ENUM       : SavedEnum      (path); break;
      case ELM_OBJ_CLASS  : SavedGameObjPar(path); break;
      case ELM_OBJ        : SavedGameObjPar(path); break;
      case ELM_MESH       : SavedMesh      (path); break;
      case ELM_MTRL       : SavedMtrl      (path); break;
      case ELM_WATER_MTRL : SavedWaterMtrl (path); break;
      case ELM_PHYS_MTRL  : SavedPhysMtrl  (path); break;
      case ELM_SKEL       : SavedSkel      (path); break;
      case ELM_PHYS       : SavedPhys      (path); break;
      case ELM_ANIM       : SavedAnim      (path); break;
      case ELM_GUI_SKIN   : SavedGuiSkin   (path); break;
      case ELM_GUI        : SavedGui       (path); break;
      case ELM_FONT       : SavedFont      (path); break;
      case ELM_TEXT_STYLE : SavedTextStyle (path); break;
      case ELM_PANEL_IMAGE: SavedPanelImage(path); break;
      case ELM_PANEL      : SavedPanel     (path); break;
      case ELM_ENV        : SavedEnv       (path); break;
      case ELM_IMAGE      : SavedImage     (path); break;
      case ELM_IMAGE_ATLAS: SavedImageAtlas(path); break;
      case ELM_ICON       : SavedImage     (path); break;
   }
}
/******************************************************************************/
// SYNC / UNDO
/******************************************************************************/
void MAX1(TimeStamp &time, C TimeStamp &src_time) {if(src_time>time)time=src_time; time++;} // set as max from both and increase by one, "time=Max(time, src_time)+1"

bool Sync(TimeStamp &time, C TimeStamp &src_time) {if(src_time> time){time=src_time;        return true;} return false;}
bool Undo(TimeStamp &time, C TimeStamp &src_time) {if(src_time!=time){MAX1(time, src_time); return true;} return false;}

void SetUndo(C Edit::_Undo &undos, Button &undo, Button &redo)
{
   undo.enabled(undos.undosAvailable());
   redo.enabled(undos.redosAvailable());
}
/******************************************************************************/
// IMAGE
/******************************************************************************/
DIR_ENUM GetCubeDir(int face)
{
   switch(face)
   {
      default: return DIR_LEFT;
      case  1: return DIR_FORWARD;
      case  2: return DIR_RIGHT;
      case  3: return DIR_BACK;
      case  4: return DIR_DOWN;
      case  5: return DIR_UP;
   }
}
Str GetCubeFile(C Str &files, int face)
{
   Mems<FileParams> faces=FileParams::Decode(files);
   return (faces.elms()==1) ? files : InRange(face, faces) ? faces[face].encode() : S;
}
Str SetCubeFile(Str files, int face, C Str &file) // put 'file' into specified 'face' and return all files
{
   if(InRange(face, 6))
   {
      Mems<FileParams> faces=FileParams::Decode(files);
      if(faces.elms()==1){faces.setNum(6); REPAO(faces)=files;} // set all from original
      if(faces.elms()!=6)faces.clear(); // if invalid number then clear
      faces.setNum(6); // set 6 faces
      faces[face]=file; // set i-th face to target file
      files=FileParams::Encode(faces); // get all faces
   }
   return files;
}
/******************************************************************************/
bool HasAlpha(C Image &image) // if image has alpha channel
{
   return image.needsAlpha();
}
bool HasColor(C Image &image) // if image is not monochromatic
{
   return !image.monochromatic();
}
bool NeedFullAlpha(Image &image, int dest_type)
{
   return image.typeInfo().a && (!InRange(dest_type, IMAGE_TYPES) || ImageTI[dest_type].a); // have to change only if source and dest have alpha channel
}
bool SetFullAlpha(Image &image, IMAGE_TYPE dest_type) // returns if any change was made
{
   if(NeedFullAlpha(image, dest_type))
   {
      if(image.compressed())return image.copy(image, -1, -1, -1, ImageTypeExcludeAlpha(ImageTypeUncompressed(image.type())), IMAGE_SOFT, 1);
      if(image.lock())
      {
         REPD(y, image.h())
         REPD(x, image.w()){Color c=image.color(x, y); c.a=255; image.color(x, y, c);}
         image.unlock();
         return true;
      }
   }
   return false;
}

void ImageProps(C Image &image, UID *hash, IMAGE_TYPE *best_type, uint flags, Edit::Material::TEX_QUALITY quality) // calculate image hash and best type for image compression
{
   if(hash || best_type)
   {
      if(image.is())
      {
         bool       sign=false;
         IMAGE_TYPE type=IMAGE_NONE;
         if(flags&WATER_MTRL)
         {
            if(flags&MTRL_BASE_0){MAX(quality, MinMtrlTexQualityBase0); flags|=SRGB;} // #MaterialTextureLayoutWater
            if(flags&MTRL_BASE_1){MAX(quality, MinMtrlTexQualityBase1); sign=true; type=((quality>=Edit::Material::FULL) ? IMAGE_R8G8_SIGN : IMAGE_BC5_SIGN);} // normal tex always uses BC5_SIGN (RG HQ) #MaterialTextureLayoutWater
            if(flags&MTRL_BASE_2){MAX(quality, MinMtrlTexQualityBase2); sign=true; type=((quality>=Edit::Material::FULL) ? IMAGE_R8_SIGN   : IMAGE_BC4_SIGN);} // bump   tex always uses BC4_SIGN (R  HQ) #MaterialTextureLayoutWater
         }else
         {
            if(flags&MTRL_BASE_0){MAX(quality, MinMtrlTexQualityBase0); flags|=SRGB;} // #MaterialTextureLayout
            if(flags&MTRL_BASE_1){MAX(quality, MinMtrlTexQualityBase1); sign=true; type=((quality>=Edit::Material::FULL) ? IMAGE_R8G8_SIGN : IMAGE_BC5_SIGN);} // normal tex always uses BC5_SIGN (RG HQ) #MaterialTextureLayout
            if(flags&MTRL_BASE_2){MAX(quality, MinMtrlTexQualityBase2);} // #MaterialTextureLayout
         }
         if(flags&MTRL_DETAIL  ){MAX(quality, MinMtrlTexQualityDetail);} // #MaterialTextureLayoutDetail
         if(flags&MTRL_MACRO   ){MAX(quality, MinMtrlTexQualityMacro ); flags|=SRGB|IGNORE_ALPHA;}
         if(flags&MTRL_EMISSIVE){MAX(quality, MinMtrlTexQualityLight ); flags|=SRGB|IGNORE_ALPHA;}

         MD5  h;
         bool bc1=true, // BC1 4-bit uses 1-bit alpha (0 or 255) (R,G,B,a?255:0)
              bc2=true, // BC2 8-bit uses 4-bit alpha
              bc4=true, // BC4 4-bit is (R,0,0,1)
              bc5=true, // BC5 8-bit is (R,G,0,1)
              srgb=FlagOn(flags, SRGB),
              calc_type=(type==IMAGE_NONE), // if have to calculate type, always calculate type if unknown (even if 'best_type' is null), because it affects hash
              force_alpha=((flags&IGNORE_ALPHA) && image.typeInfo().a), // if we want to ignore alpha, and source had alpha, then we need to adjust as if it has full alpha, this is done because: ignoring alpha may save the image in format that doesn't support the alpha channel, however if the same image is later used for something else, and now wants to use that alpha channel, then it needs to be created as a different texture (with different hash)
              extract=((hash && (sign ? image.hwType()!=IMAGE_R8G8B8A8_SIGN : (image.hwType()!=IMAGE_R8G8B8A8 && image.hwType()!=IMAGE_R8G8B8A8_SRGB))) // hash is based on RGBA format
                    || (calc_type && image.compressed()) // checking 'type' requires color reads so copy to RGBA soft to make them faster
                    || force_alpha); // forcing alpha requires modifying the alpha channel, so copy to 'temp' which we can modify
         Image temp; C Image *src=(extract ? &temp : &image);
         Color min(255, 255), max(0, 0);
         FREPD(face, image.faces())
         {
            int src_face=face;
            if(extract)
               if(image.extractMipMap(temp, sign ? IMAGE_R8G8B8A8_SIGN : image.sRGB() ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8A8, 0, DIR_ENUM(face)))src_face=0; // !! RGBA is needed because below we use 'pixC' !!
               else goto error;
            if(src->lockRead(0, DIR_ENUM(src_face)))
            {
               if(force_alpha) // set before calculating hash
               {
                  byte alpha=(sign ? 127 : 255); // use 127 instead of 128, because this is for signed byte (which is in range -128..127)
                  REPD(z, temp.d())
                  REPD(y, temp.h())
                  REPD(x, temp.w())temp.pixC(x, y, z).a=alpha;
               }
               if(hash)
               {
                  int pitch =src->w()*src->bytePP(), // don't use 'src.pitch'                       to ignore any extra padding
                      pitch2=pitch  *src->h     (); // use "pitch*src.h()" instead of 'src.pitch2' to ignore any extra padding
                  FREPD(z, src->d()) // process in order
                  {
                     if(src->pitch()==pitch)h.update(src->data()                 + z*src->pitch2(), pitch2); // if don't have any extra padding then we can update hash for all Y's in one go
                     else FREPD(y, src->h())h.update(src->data() + y*src->pitch() + z*src->pitch2(), pitch ); // process in order
                  }
               }
               if(calc_type)REPD(z, src->d())
                            REPD(y, src->h())
                            REPD(x, src->w())
               {
                  Color c=src->color3D(x, y, z);
                  byte  bc2_a=((c.a*15+128)/255)*255/15;
                //if(c.a> 1 && c.a<254                    // BC1 supports only 0 and 255 alpha #BC1RGB
                //|| c.a<=1 && c.lum()>1      )bc1=false; // BC1 supports only black color at 0 alpha
                  if(Abs(c.a-bc2_a)>1         )bc2=false;
                //if(c.g>1 || c.b>1 || c.a<254)bc4=false;
                //if(         c.b>1 || c.a<254)bc5=false;
                  MIN(min.r, c.r); MIN(min.g, c.g); MIN(min.b, c.b); MIN(min.a, c.a);
                  MAX(max.r, c.r); MAX(max.g, c.g); MAX(max.b, c.b); MAX(max.a, c.a);
               }
               src->unlock();
            }else goto error;
         }

         if(calc_type)
         {
            if(                      min.a<254)bc1=false; // BC1 supports only           A=255 #BC1RGB
            if(max.g>1 || max.b>1 || min.a<254)bc4=false; // BC4 supports only G=0, B=0, A=255
            if(           max.b>1 || min.a<254)bc5=false; // BC5 supports only      B=0, A=255
            if((flags&MTRL_BASE_2) && !(flags&WATER_MTRL) && max.b-min.b>1)MAX(quality, Edit::Material::HIGH); // if this is Base2 Ext and have bump map #MaterialTextureLayout then disable MEDIUM quality BC1 and use HIGH quality BC7

            if(quality>=Edit::Material::FULL)
            {
               if(srgb || min.a<254 || max.b>1)type=IMAGE_R8G8B8A8_SRGB;else // sRGB or has Alpha or has Blue
               if(                     max.g>1)type=IMAGE_R8G8         ;else // has Green
                                               type=IMAGE_R8           ;
            }else
            {
               if(bc4 && !srgb                     )type=                         IMAGE_BC4 ;else // BC4 is 4-bit HQ so use it always if possible (doesn't support sRGB)
               if(bc1 && quality<Edit::Material::HIGH)type=(srgb ? IMAGE_BC1_SRGB : IMAGE_BC1);else // use BC1 only if we don't want HQ
               if(bc5 && !srgb                     )type=                         IMAGE_BC5 ;else // BC5 has better quality for RG than BC7 so check it first (doesn't support sRGB)
               if(SupportBC7                       )type=(srgb ? IMAGE_BC7_SRGB : IMAGE_BC7);else
               if(bc1                              )type=(srgb ? IMAGE_BC1_SRGB : IMAGE_BC1);else // check BC1 again, now without HQ
               if(bc2                              )type=(srgb ? IMAGE_BC2_SRGB : IMAGE_BC2);else
                                                    type=(srgb ? IMAGE_BC3_SRGB : IMAGE_BC3);
            }
         }
         if(best_type)*best_type=type;
         if(hash     )
         {
            h.update(&NoTemp(ImageHashHeader(image, type)), SIZE(ImageHashHeader)); // need to append hash with a header, to make sure different sized/cube/srgb/sign/quality images will always have different hash
           *hash=h();
         }
      }else
      {
      error:
         if(hash     )hash->zero();
         if(best_type)*best_type=IMAGE_NONE;
      }
   }
}
/******************************************************************************/
void LoadTexture(C Project &proj, C UID &tex_id, Image &image)
{
   ImagePtr src=proj.texPath(tex_id);
   if(src)src->copy(image, -1, -1, -1, ImageTypeUncompressed(src->type()), IMAGE_SOFT, 1);else image.del(); // always copy, because: src texture will always be compressed, also soft doesn't require locking
}
void ExtractBaseTextures(C Project &proj, C UID &base_0, C UID &base_1, C UID &base_2, Image *color, Image *alpha, Image *bump, Image *normal, Image *smooth, Image *metal, Image *glow)
{ // #MaterialTextureLayout
   TEX_FLAG tex=TEXF_NONE;
   if(base_0.valid() && (color || alpha                 )){Image b0; LoadTexture(proj, base_0, b0); tex|=ExtractBase0Texture(b0, color, alpha             );}
   if(base_1.valid() && (normal                         )){Image b1; LoadTexture(proj, base_1, b1); tex|=ExtractBase1Texture(b1, normal                   );}
   if(base_2.valid() && (bump || smooth || metal || glow)){Image b2; LoadTexture(proj, base_2, b2); tex|=ExtractBase2Texture(b2, bump, smooth, metal, glow);}
   if(color  && !(tex&TEXF_COLOR ))color ->del();
   if(alpha  && !(tex&TEXF_ALPHA ))alpha ->del();
   if(bump   && !(tex&TEXF_BUMP  ))bump  ->del();
   if(normal && !(tex&TEXF_NORMAL))normal->del();
   if(smooth && !(tex&TEXF_SMOOTH))smooth->del();
   if(metal  && !(tex&TEXF_METAL ))metal ->del();
   if(glow   && !(tex&TEXF_GLOW  ))glow  ->del();
}
void ExtractWaterBaseTextures(C Project &proj, C UID &base_0, C UID &base_1, C UID &base_2, Image *color, Image *alpha, Image *bump, Image *normal, Image *smooth, Image *reflect, Image *glow)
{ // #MaterialTextureLayoutWater
   TEX_FLAG tex=TEXF_NONE;
   if(base_0.valid() && color ){Image b0; LoadTexture(proj, base_0, b0); tex|=ExtractWaterBase0Texture(b0, color );}
   if(base_1.valid() && normal){Image b1; LoadTexture(proj, base_1, b1); tex|=ExtractWaterBase1Texture(b1, normal);}
   if(base_2.valid() && bump  ){Image b2; LoadTexture(proj, base_2, b2); tex|=ExtractWaterBase2Texture(b2, bump  );}
   if(color   && !(tex&TEXF_COLOR ))color  ->del();
   if(alpha   && !(tex&TEXF_ALPHA ))alpha  ->del();
   if(bump    && !(tex&TEXF_BUMP  ))bump   ->del();
   if(normal  && !(tex&TEXF_NORMAL))normal ->del();
   if(smooth  && !(tex&TEXF_SMOOTH))smooth ->del();
   if(reflect && !(tex&TEXF_METAL ))reflect->del();
   if(glow    && !(tex&TEXF_GLOW  ))glow   ->del();
}
void ExtractDetailTextures(C Project &proj, C UID &detail_tex, Image *color, Image *bump, Image *normal, Image *smooth)
{ // #MaterialTextureLayoutDetail
   TEX_FLAG tex=TEXF_NONE;
   if(detail_tex.valid())
      if(color || bump || normal || smooth)
   {
      Image detail; LoadTexture(proj, detail_tex, detail); tex=ExtractDetailTexture(detail, color, bump, normal, smooth);
   }
   if(color  && !(tex&TEXF_DET_COLOR ))color ->del();
   if(bump   && !(tex&TEXF_DET_BUMP  ))bump  ->del();
   if(normal && !(tex&TEXF_DET_NORMAL))normal->del();
   if(smooth && !(tex&TEXF_DET_SMOOTH))smooth->del();
}
UID MergedBaseTexturesID(C UID &base_0, C UID &base_1, C UID &base_2) // this function generates ID of a merged texture created from two base textures, formula for this function can be freely modified as in worst case merged textures will just get regenerated
{
   MD5 id;
   id.update(&base_0, SIZE(base_0));
   id.update(&base_1, SIZE(base_1));
   id.update(&base_2, SIZE(base_2));
   return id();
}
/******************************************************************************/
VecI ImageSize(C VecI &src, C VecI2 &custom, bool pow2)
{
   VecI size=src;
   if( custom.x>0)size.x=custom.x;
   if( custom.y>0)size.y=custom.y;
   if(!custom.x && custom.y>0 && src.y)size.x=Max(1, (src.x*custom.y+src.y/2)/src.y); // keep aspect ratio
   if(!custom.y && custom.x>0 && src.x)size.y=Max(1, (src.y*custom.x+src.x/2)/src.x); // keep aspect ratio
   if(pow2)size.set(NearestPow2(size.x), NearestPow2(size.y), NearestPow2(size.z));
   return size;
}
VecI2 GetSize(C Str &name, C Str &value, C VecI &src, C VecI &background_size)
{
   VecI2 size;
   if(value=="quarter")size.set(Max(1, src.x/4), Max(1, src.y/4));else
   if(value=="half"   )size.set(Max(1, src.x/2), Max(1, src.y/2));else
   if(value=="double" )size=src.xy*2;else
   if(value=="full"   )size=background_size.xy;else
   {
      Vec2 sf; if(Contains(value, ','))sf=TextVec2(value);else sf=TextFlt(value);
      UNIT_TYPE unit=GetUnitType(value);
      size.x=Round(ConvertUnitType(sf.x, src.x, unit));
      size.y=Round(ConvertUnitType(sf.y, src.y, unit));
   }
   size=ImageSize(src, size, false).xy;
   if(Starts(name, "maxSize")){MIN(size.x, src.x); MIN(size.y, src.y);}
   return size;
}
int GetFilterI(C Str &name)
{
   if(Contains(name, "nearest"   ) || Contains(name, "point") || Contains(name, "FilterNone"))return FILTER_NONE;
   if(Contains(name, "linear"    ))return FILTER_LINEAR;
   if(Contains(name, "cubic+"    ) || Contains(name, "cubicPlus"))return FILTER_CUBIC_PLUS; // !! check this before "cubic" !!
   if(Contains(name, "cubic"     ))return FILTER_CUBIC_FAST;
   if(Contains(name, "cubicMed"  ))return FILTER_CUBIC_FAST_MED;
   if(Contains(name, "cubicSharp"))return FILTER_CUBIC_FAST_SHARP;
   if(Contains(name, "Waifu"     ))return FILTER_WAIFU;
   if(Contains(name, "EASU"      ))return FILTER_EASU;
   if(Contains(name, "NoStretch" ))return FILTER_NO_STRETCH;
                                   return -1;
}
FILTER_TYPE GetFilter(C Str &name)
{
   int i=GetFilterI(name); return InRange(i, FILTER_NUM) ? FILTER_TYPE(i) : FILTER_BEST;
}
bool GetClampWrap(C Str &name, bool default_clamp)
{
   if(Contains(name, "clamp"))return true ; // clamp=1
   if(Contains(name, "wrap" ))return false; // clamp=0
                              return default_clamp; // default
}
bool GetAlphaWeight(C Str &name) {return Contains(name, "alphaWeight");}
bool GetKeepEdges  (C Str &name) {return Contains(name, "keepEdges"  );}
bool EditToGameImage(Image &edit, Image &game, bool pow2, bool srgb, bool alpha_lum, ElmImage::TYPE type, int mode, int mip_maps, bool has_color, bool has_alpha, bool ignore_alpha, bool env, C VecI2 &custom_size, C int *force_type)
{
   VecI size=edit.size3();
   if(!edit.cube() && IsCube((IMAGE_MODE)mode))switch(edit.cubeLayout())
   {
      case CUBE_LAYOUT_CROSS: size.x/=4; size.y/=3; break;
      case CUBE_LAYOUT_6x1  : size.x/=6;            break;
   }
   size=ImageSize(size, custom_size, pow2);

   Image temp, *src=&edit;

   if(force_type)
   {
      if(*force_type==IMAGE_NONE || *force_type<0)force_type=null;
   }
   if(alpha_lum)
   {
      if(&edit!=&game){src->copy(temp); src=&temp;}
      src->alphaFromBrightness().divRgbByAlpha();
   }
   if(ignore_alpha && src->typeInfo().a) // if want to ignore alpha then set it to full as some compressed texture formats will benefit from better quality (like BC7, ETC2, ASTC, PVRTC)
   {
      if(mip_maps<0)mip_maps=((src->mipMaps()==1) ? 1 : 0); // source will have now only one mip-map so we can't use "-1", auto-detect instead
      if(mode    <0)mode    =src->mode();                   // source will now be as IMAGE_SOFT      so we can't use "-1", auto-detect instead
      if(src->copy(temp, -1, -1, -1, IMAGE_R8G8B8_SRGB, IMAGE_SOFT, 1))src=&temp;
   }

   IMAGE_TYPE    dest_type;
   if(force_type)dest_type=IMAGE_TYPE(*force_type);else
   if(type==ElmImage::ALPHA)dest_type=IMAGE_A8;else
   if(type==ElmImage::FULL )dest_type=(has_color ? (srgb ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8A8) : has_alpha ? (srgb ? IMAGE_L8A8_SRGB : IMAGE_L8A8) : (srgb ? IMAGE_L8_SRGB : IMAGE_L8));else
                           ImageProps(*src, null, &dest_type, (srgb ? SRGB : 0) | (ignore_alpha ? IGNORE_ALPHA : 0), (type==ElmImage::FULL) ? Edit::Material::FULL : (type==ElmImage::COMPRESSED2) ? Edit::Material::HIGH : Edit::Material::MEDIUM);

   if((src->type()==IMAGE_L8 || src->type()==IMAGE_L8_SRGB) &&  dest_type==IMAGE_A8
   ||  src->type()==IMAGE_A8                               && (dest_type==IMAGE_L8 || dest_type==IMAGE_L8_SRGB))
   {
      Image temp2; if(temp2.createSoft(src->w(), src->h(), src->d(), dest_type) && src->lockRead())
      {
         REPD(z, temp2.d())
         REPD(y, temp2.h())
         REPD(x, temp2.w())temp2.pixel3D(x, y, z, src->pixel3D(x, y, z));
         src->unlock();
         Swap(temp, temp2); src=&temp;
      }
   }
   return src->copy(game, size.x, size.y, size.z, dest_type, mode, mip_maps, FILTER_BEST, IC_CLAMP|IC_ALPHA_WEIGHT|(env ? IC_ENV_CUBE : 0));
}
bool EditToGameImage(Image &edit, Image &game, C ElmImage &data, C int *force_type)
{
   return EditToGameImage(edit, game, data.pow2(), data.sRGB(), data.alphaLum(), data.type, data.mode, data.mipMapsActual() ? 0 : 1, data.hasColor(), data.hasAlpha3(), data.ignoreAlpha(), data.envActual(), data.size, force_type);
}
/******************************************************************************/
void DrawPanelImage(C PanelImage &pi, C Rect &rect, bool draw_lines)
{
   Vec2 size=rect.size()/5;
   Rect r=rect; r.extend(-size/3);
   Rect_LU lu(r.lu(), size            ); Rect_RU ru(r.ru(), size.x*3, size.y  );
   Rect_LD ld(r.ld(), size.x, size.y*3); Rect_RD rd(r.rd(), size.x*3, size.y*3);
   pi.draw(lu); pi.draw(ru);
   pi.draw(ld); pi.draw(rd);
   if(draw_lines)
   {
      pi.drawScaledLines(RED, lu); pi.drawScaledLines(RED, ru);
      pi.drawScaledLines(RED, ld); pi.drawScaledLines(RED, rd);
      lu.draw(AZURE, false); ru.draw(AZURE, false);
      ld.draw(AZURE, false); rd.draw(AZURE, false);
   }
}
/******************************************************************************/
bool UpdateMtrlBase1Tex(C Image &src, Image &dest)
{
   Image temp; if(src.copy(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))
   {
      // old: r=spec, g=NrmY, b=alpha, a=NrmX
      // new: r=NrmX, g=NrmY, b=spec , a=alpha
      REPD(y, temp.h())
      REPD(x, temp.w())
      {
         Color c=temp.color(x, y);
         c.set(c.a, c.g, c.r, c.b);
         temp.color(x, y, c);
      }
      return temp.copy(dest, -1, -1, -1, (src.type()==IMAGE_BC3 || src.type()==IMAGE_BC3_SRGB) ? IMAGE_BC7 : ImageTypeExcludeSRGB(src.type()), src.mode(), src.mipMaps(), FILTER_BEST, IC_WRAP);
   }
   return false;
}
/******************************************************************************/
bool ImportImage(Image &image, C Str &name, int type, int mode, int mip_maps, bool decompress)
{
   if(image.Import(name, type, mode, mip_maps))
   {
      if(image.compressed() && decompress && !image.copy(image, -1, -1, -1, ImageTypeUncompressed(image.type())))return false;
      return true;
   }
 /*if(name.is())
   {
      File f, dec; if(f.read(name+".cmpr"))if(Decompress(f, dec.writeMem()))
      {
         dec.pos(0); if(image.Import(dec, type, mode, mip_maps))return true;
      }
   }*/
   return false;
}
/******************************************************************************/
cchar8* IndexChannel(int i)
{
   switch(i)
   {
      case  0: return "r";
      case  1: return "g";
      case  2: return "b";
      case  3: return "a";
      default: return null;
   }
}
int ChannelIndex(char c)
{
   switch(c)
   {
      case 'r': case 'R': case 'x': case 'X': return 0;
      case 'g': case 'G': case 'y': case 'Y': return 1;
      case 'b': case 'B': case 'z': case 'Z': return 2;
      case 'a': case 'A': case 'w': case 'W': return 3;
   }
   return -1;
}
bool ChannelMonoTransform(C Str &value)
{
   return value.length()<=1 // up to 1 channels is treated as mono
   || ChannelIndex(value[0])==ChannelIndex(value[1]) && ChannelIndex(value[0])==ChannelIndex(value[2]); // check that RGB channels are the same
}
bool PartialTransform   (C TextParam &p   ) {return Contains(p.value, '@');} // if transform is partial (affects only part of the image and not full), '@' means transform at position
bool  LinearTransform   (C Str       &name) {return name=="mulRGB" || name=="addRGB" || name=="mulAddRGB";}
bool  ResizeTransformAny(C Str       &name) {return Starts(name, "resize") || Starts(name, "maxSize");}
bool  ResizeTransform   (C Str       &name) {return ResizeTransformAny(name) && !Contains(name, "NoStretch");} // skip "NoStretch" because it's more like "crop"
bool    MonoTransform   (C TextParam &p   ) {return p.name=="grey" || p.name=="greyPhoto" || p.name=="bump" || p.name=="bumpClamp" || (p.name=="channel" && ChannelMonoTransform(p.value)) || p.name=="getSat" || p.name=="getHue";} // if result is always mono
bool NonMonoTransform   (C TextParam &p   ) // if can change a mono image to non-mono, this is NOT the same as "!MonoTransform"
{
   int values=Occurrences(p.value, ',');
   return p.name=="inverseR"
       || p.name=="inverseG"
       || p.name=="inverseRG"
       || p.name=="lerpRGB" && values>2
       || p.name=="iLerpRGB" && values>2
       || p.name=="mulRGB" && TextVecEx(p.value).anyDifferent()
       || p.name=="mulRGBLin" && TextVecEx(p.value).anyDifferent()
       || p.name=="addRGB" && TextVecEx(p.value).anyDifferent()
       || p.name=="setRGB" && TextVecEx(p.value).anyDifferent()
       || p.name=="setRGBA" && p.asVec4().xyz.anyDifferent()
       || p.name=="mulAddRGB" && values>2
       || p.name=="addMulRGB" && values>2
       || p.name=="mulRGBIS" && TextVecEx(p.value).anyDifferent()
       || p.name=="mulRGBS" && TextVecEx(p.value).anyDifferent()
       || p.name=="mulRGBH" && values>1
       || p.name=="mulRGBHS" && values>1
       || p.name=="gamma" && TextVecEx(p.value).anyDifferent()
       || p.name=="brightness" && TextVecEx(p.value).anyDifferent()
       || p.name=="sigmoid" && TextVecEx(p.value).anyDifferent()
       || p.name=="contrast" && TextVecEx(p.value).anyDifferent()
       || p.name=="contrastAlphaWeight" && TextVecEx(p.value).anyDifferent()
       || p.name=="addSat"
       || p.name=="addSatPhoto"
       || p.name=="setSat"
       || p.name=="setSatPhoto"
       || p.name=="mulAddSat"
       || p.name=="mulAddSatPhoto"
       || p.name=="addHueSat"
       || p.name=="setHueSat"
       || p.name=="setHueSatPhoto"
       || p.name=="lerpHueSat"
       || p.name=="rollHueSat"
       || p.name=="rollHueSatPhoto"
       || p.name=="min" && TextVecEx(p.value).anyDifferent()
       || p.name=="max" && TextVecEx(p.value).anyDifferent()
       || p.name=="channel" && !ChannelMonoTransform(p.value)
       || p.name=="scaleXY" && TextVec2Ex(p.value).anyDifferent()
       || p.name=="bumpToNormal";
}
bool HighPrecTransform(C Str &name)
{
   return ResizeTransform(name)
       || name=="mulRGB" || name=="mulRGBLin" || name=="addRGB" || name=="setRGB" || name=="setRGBA" || name=="mulAddRGB" || name=="addMulRGB" || name=="mulA"
       || name=="mulRGBbyA"
       || name=="mulRGBS" || name=="mulRGBIS" || name=="mulRGBH" || name=="mulRGBHS"
       || name=="normalize"
       || name=="scale" || name=="scaleXY"
       || name=="lerpRGB" || name=="iLerpRGB"
       || name=="lerpLum" || name=="iLerpLum"
       || name=="blur" || name=="sharpen"
       || name=="bump" || name=="bumpClamp"
       || name=="contrast" || name=="contrastLum" || name=="contrastAlphaWeight" || name=="contrastLumAlphaWeight"
       || name=="brightness" || name=="brightnessLum" || name=="brightnessLumS"
       || name=="sigmoid" || name=="sigmoidLum"
       || name=="gamma" || name=="gammaA" || name=="gammaLum" || name=="gammaLumPhoto" || name=="gammaSat"
       || name=="SRGBToLinear" || name=="LinearToSRGB"
       || name=="greyPhoto"
       || name=="avgLum" || name=="medLum" || name=="avgContrastLum" || name=="medContrastLum"
       || name=="avgHue" || name=="avgHuePhoto" || name=="avgHueAlphaWeight" || name=="avgHuePhotoAlphaWeight" || name=="medHue" || name=="medHueAlphaWeight" || name=="medHuePhoto" || name=="medHuePhotoAlphaWeight" || name=="addHue" || name=="addHuePhoto" || name=="setHue" || name=="setHuePhoto" || name=="contrastHue" || name=="contrastHuePhoto" || name=="medContrastHue" || name=="medContrastHuePhoto" || name=="contrastHueAlphaWeight" || name=="contrastHuePhotoAlphaWeight" || name=="contrastHuePow"
       || name=="lerpHue" || name=="lerpHueSat" || name=="rollHue" || name=="rollHueSat" || name=="lerpHuePhoto" || name=="lerpHueSatPhoto" || name=="rollHuePhoto" || name=="rollHueSatPhoto"
       || name=="addSat" || name=="addSatPhoto" || name=="mulSat" || name=="mulSatPhoto" || name=="mulAddSat" || name=="mulAddSatPhoto" || name=="avgSat" || name=="avgSatPhoto" || name=="medSat" || name=="medSatPhoto" || name=="contrastSat" || name=="contrastSatPhoto" || name=="medContrastSat" || name=="contrastSatAlphaWeight" || name=="contrastSatPhotoAlphaWeight" || name=="minSat" || name=="minSatPhoto"
       || name=="setSat" || name=="setSatPhoto"
       || name=="addHueSat" || name=="setHueSat" || name=="setHueSatPhoto"
       || name=="mulSatH" || name=="mulSatHS" || name=="mulSatHPhoto" || name=="mulSatHSPhoto"
     //|| name=="metalToReflect"
       ;
}
bool SizeDependentTransform(C TextParam &p)
{
   return p.name=="blur" // range depends on size
       || p.name=="sharpen" // range depends on size
       || p.name=="bump" || p.name=="bumpClamp" // range depends on size
       || p.name=="size"   // coordinates/size depend on size
       || p.name=="crop"   // coordinates/size depend on size
       || p.name=="trim"   // coordinates/size depend on size
       || p.name=="extend" // coordinates/size depend on size
       || p.name=="resizeNoStretch"
       || p.name=="rotate"
       || p.name=="tile" // tile range depends on size
       || (p.name=="pos" || p.name=="position") && p.asVecI2().any() // coordinates depend on size
       || PartialTransform(p); // coordinates/size depend on size
}
bool ForcesMono(C Str &file)
{
   Mems<FileParams> files=FileParams::Decode(file);
   REPA(files) // go from end
   {
      FileParams &file=files[i];
      if(i && (file.name.is() || file.nodes.elms()))break; // stop on first file that has name (but allow the first which means there's only one file) so we don't process transforms for only 1 of multiple images
      REPA(file.params) // go from end
      {
       C TextParam &param=file.params[i];
         if(   MonoTransform(param) && !PartialTransform(param))return true; // if current transform generates mono (fully) and no non-mono were encountered up to now, then result is mono
         if(NonMonoTransform(param))return false; // if there's at least one transform that can generate color then result is not mono
      }
   }
   return false;
}
Str BumpFromColTransform(C Str &color_map, int blur) // 'blur'<0 = empty (default)
{
   Mems<FileParams> files=FileParams::Decode(color_map);
   REPA(files) // go from end
   {
      FileParams &file=files[i];
      if(i && (file.name.is() || file.nodes.elms()))break; // stop on first file that has name (but allow the first which means there's only one file) so we don't process transforms for only 1 of multiple images
      REPA(file.params) // go from end
      {
         TextParam &p=file.params[i]; if(p.name!="size" && p.name!="crop" && p.name!="trim" && p.name!="extend" && p.name!="resizeNoStretch" && p.name!="swapXY" && p.name!="mirrorXY" && p.name!="mirrorX" && p.name!="mirrorY" && p.name!="flipXY" && p.name!="flipX" && p.name!="flipY")file.params.remove(i, true); // allow only these transforms
      }
      if(!file.is())files.remove(i, true); // if nothing left then remove it
   }
   SetTransform(files, "bump", (blur<0) ? S : S+blur);
   return FileParams::Encode(files);
}
bool ExtractResize(MemPtr<FileParams> files, TextParam &resize)
{
   resize.del();
   REPA(files) // go from end
   {
      FileParams &file=files[i];
      if(i && (file.name.is() || file.nodes.elms()))break; // stop on first file that has name (but allow the first which means there's only one file) so we don't process transforms for only 1 of multiple images
      REPAD(pi, file.params) // go from end
      {
       C TextParam &p=file.params[pi];
         if(ResizeTransform(p.name))
         {
            resize=p; // extract resize
                    file.params.remove(pi, true); // remove it
            if(!file.is())files.remove( i, true); // if nothing left then remove it
            return true; // extracted
         }else
         if(SizeDependentTransform(p))return false; // if encountered a size dependent transform, it means we can't keep looking
      }
   }
   return false;
}
bool ExtractLinearTransform(MemPtr<FileParams> files, Vec &mul, Vec &add)
{
   REPA(files) // go from end
   {
      FileParams &file=files[i];
      if(i && (file.name.is() || file.nodes.elms()))break; // stop on first file that has name (but allow the first which means there's only one file) so we don't process transforms for only 1 of multiple images
      REPAD(pi, file.params) // go from end
      {
       C TextParam &p=file.params[pi];
         if(PartialTransform(p))goto none;
         if( LinearTransform(p.name))
         {
            if(p.name=="mulRGB"   ){mul=TextVecEx(p.value); add=0;}else
            if(p.name=="addRGB"   ){add=TextVecEx(p.value); mul=1;}else
            if(p.name=="mulAddRGB"){if(!TextVecVecEx(p.value, mul, add))goto none;}else
               goto none;
                    file.params.remove(pi, true); // remove it
            if(!file.is())files.remove( i, true); // if nothing left then remove it
            return true; // extracted
         }
         if(!ResizeTransform(p.name))goto none; // allow continue only on resize
      }
   }
none:
   mul=1; add=0;
   return false;
}
/******************************************************************************/
void AdjustImage(Image &image, bool rgb, bool alpha, bool high_prec)
{
   IMAGE_TYPE       type=image.type();
 C ImageTypeInfo &old_ti=ImageTI[type];
   if(rgb      )type=ImageTypeIncludeRGB  (type);
   if(alpha    )type=ImageTypeIncludeAlpha(type);
   if(high_prec)type=ImageTypeHighPrec    (type);
   if(type!=image.type())
   {
      image.copy(image, -1, -1, -1, type);
    C ImageTypeInfo &new_ti=image.typeInfo();
      if(old_ti.r && !old_ti.g && !old_ti.b && (new_ti.g || new_ti.b)) // expand single channel Red -> Green, Blue
      {
         REPD(z, image.d())
         REPD(y, image.h())
         REPD(x, image.w())
         {
            Vec4 c=image.color3DF(x, y, z);
            c.z=c.y=c.x;
            image.color3DF(x, y, z, c);
         }
      }
   }
}
void ContrastLum(Image &image, flt contrast, flt avg_lum, C BoxI &box)
{
   if(contrast!=1 && image.lock())
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         flt  c_lum=c.xyz.max(), want_lum=(c_lum-avg_lum)*contrast+avg_lum;
         if(c_lum>EPS)c.xyz*=want_lum/c_lum;else c.xyz=want_lum;
         image.color3DF(x, y, z, c);
      }
      image.unlock();
   }      
}
void AvgContrastLum(Image &image, flt contrast, dbl avg_lum, C BoxI &box)
{
   if(avg_lum && image.lock()) // lock for writing because we will use this lock for applying contrast too
   {
      dbl contrast_total=0, weight_total=0;
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z); dbl c_lum=c.xyz.max();
         if(dbl d=c_lum-avg_lum)
         {
            dbl contrast=Abs(d)/avg_lum, // div by 'avg_lum' so for bright values contrast will be proportionally the same
                weight=Sqr(d); // squared distance from avg_lum
            contrast_total+=weight*contrast;
              weight_total+=weight;
         }
      }
      if(weight_total)if(contrast_total/=weight_total)ContrastLum(image, contrast/contrast_total, avg_lum, box);
      image.unlock();
   }
}
void ContrastHue(Image &image, flt contrast, C Vec &avg_col, C BoxI &box, bool photo)
{
   if(contrast!=1 && image.lock())
   {
      flt avg_hue=RgbToHsb(avg_col).x;
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
       //flt  lin_lum; if(photo)lin_lum=LinearLumOfSRGBColor(c.xyz);
         flt      lum; if(photo)    lum=  SRGBLumOfSRGBColor(c.xyz);
         c.xyz=RgbToHsb(c.xyz);
         flt d_hue=HueDelta(avg_hue, c.x);
         d_hue*=contrast;
         Clamp(d_hue, -0.5f, 0.5f); // clamp so we don't go back
         c.x=d_hue+avg_hue;
         c.xyz=HsbToRgb(c.xyz);
         if(photo)
         {
          //c.xyz=SRGBToLinear(c.xyz); if(flt cur_lin_lum=LinearLumOfLinearColor(c.xyz))c.xyz*=lin_lum/cur_lin_lum; c.xyz=LinearToSRGB(c.xyz);
                                       if(flt cur_lum    =  SRGBLumOfSRGBColor  (c.xyz))c.xyz*=    lum/cur_lum    ; // prefer multiplications in sRGB space, as linear mul may change perceptual contrast and saturation
         }
         image.color3DF(x, y, z, c);
      }
      image.unlock();
   }
}
void AddHue(Image &image, flt hue, C BoxI &box, bool photo)
{
   hue=Frac(hue);
   if(hue && image.lock())
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
       //flt  lin_lum; if(photo)lin_lum=LinearLumOfSRGBColor(c.xyz);
         flt      lum; if(photo)    lum=  SRGBLumOfSRGBColor(c.xyz);
         c.xyz=RgbToHsb(c.xyz);
         c.x +=hue;
         c.xyz=HsbToRgb(c.xyz);
         if(photo)
         {
          //c.xyz=SRGBToLinear(c.xyz); if(flt cur_lin_lum=LinearLumOfLinearColor(c.xyz))c.xyz*=lin_lum/cur_lin_lum; c.xyz=LinearToSRGB(c.xyz);
                                       if(flt cur_lum    =  SRGBLumOfSRGBColor  (c.xyz))c.xyz*=    lum/cur_lum    ; // prefer multiplications in sRGB space, as linear mul may change perceptual contrast and saturation
         }
         image.color3DF(x, y, z, c);
      }
      image.unlock();
   }
}
void ContrastSat(Image &image, flt contrast, flt avg_sat, C BoxI &box, bool photo)
{
   if(contrast!=1 && image.lock())
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
       //flt  lin_lum; if(photo)lin_lum=LinearLumOfSRGBColor(c.xyz);
         flt      lum; if(photo)    lum=  SRGBLumOfSRGBColor(c.xyz);
         c.xyz=RgbToHsb(c.xyz);
         c.y=(c.y-avg_sat)*contrast+avg_sat;
         c.xyz=HsbToRgb(c.xyz);
         if(photo)
         {
          //c.xyz=SRGBToLinear(c.xyz); if(flt cur_lin_lum=LinearLumOfLinearColor(c.xyz))c.xyz*=lin_lum/cur_lin_lum; c.xyz=LinearToSRGB(c.xyz);
                                       if(flt cur_lum    =  SRGBLumOfSRGBColor  (c.xyz))c.xyz*=    lum/cur_lum    ; // prefer multiplications in sRGB space, as linear mul may change perceptual contrast and saturation
         }
         image.color3DF(x, y, z, c);
      }
      image.unlock();
   }
}
void MulRGBH(Image &image, flt red, flt yellow, flt green, flt cyan, flt blue, flt purple, C BoxI &box)
{
   flt mul[]={red, yellow, green, cyan, blue, purple, red, yellow}; // red and yellow are listed extra for wraparound
   REP(6)if(mul[i]!=1)goto mul; return; mul:
   for(int z=box.min.z; z<box.max.z; z++)
   for(int y=box.min.y; y<box.max.y; y++)
   for(int x=box.min.x; x<box.max.x; x++)
   {
      Vec4 c=image.color3DF(x, y, z);
      Vec  hsb=RgbToHsb(c.xyz);
      flt  hue=hsb.x*6; int hue_i=Trunc(hue); flt hue_frac=hue-hue_i;
      flt  hue_mul=Lerp(mul[hue_i], mul[hue_i+1], hue_frac);
      c.xyz*=hue_mul;
      image.color3DF(x, y, z, c);
   }
}
void MulRGBHS(Image &image, flt red, flt yellow, flt green, flt cyan, flt blue, flt purple, C BoxI &box)
{
   flt mul[]={red, yellow, green, cyan, blue, purple, red, yellow}; // red and yellow are listed extra for wraparound
   REP(6)if(mul[i]!=1)goto mul; return; mul:
   for(int z=box.min.z; z<box.max.z; z++)
   for(int y=box.min.y; y<box.max.y; y++)
   for(int x=box.min.x; x<box.max.x; x++)
   {
      Vec4 c=image.color3DF(x, y, z);
      Vec  hsb=RgbToHsb(c.xyz);
      flt  hue=hsb.x*6; int hue_i=Trunc(hue); flt hue_frac=hue-hue_i;
      flt  hue_mul=Lerp(mul[hue_i], mul[hue_i+1], hue_frac);
      c.xyz*=Lerp(1.0f, hue_mul, hsb.y);
      image.color3DF(x, y, z, c);
   }
}
void GammaSat(Image &image, flt gamma, C BoxI &box, bool photo)
{
   if(gamma!=1 && image.lock())
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
       //flt  lin_lum; if(photo)lin_lum=LinearLumOfSRGBColor(c.xyz);
         flt      lum; if(photo)    lum=  SRGBLumOfSRGBColor(c.xyz);

         c.xyz=RgbToHsb(c.xyz);
         c.y=Pow(c.y, gamma);
         c.xyz=HsbToRgb(c.xyz);
         if(photo)
         {
          //c.xyz=SRGBToLinear(c.xyz); if(flt cur_lin_lum=LinearLumOfLinearColor(c.xyz))c.xyz*=lin_lum/cur_lin_lum; c.xyz=LinearToSRGB(c.xyz);
                                       if(flt cur_lum    =  SRGBLumOfSRGBColor  (c.xyz))c.xyz*=    lum/cur_lum    ; // prefer multiplications in sRGB space, as linear mul may change perceptual contrast and saturation
         }
         image.color3DF(x, y, z, c);
      }
      image.unlock();
   }   
}
void MulAddSat(Image &image, flt mul, flt add, C BoxI &box, bool photo)
{
   if((mul!=1 || add!=0) && image.lock())
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
       //flt  lin_lum; if(photo)lin_lum=LinearLumOfSRGBColor(c.xyz);
         flt      lum; if(photo)    lum=  SRGBLumOfSRGBColor(c.xyz);

         c.xyz=RgbToHsb(c.xyz);
         c.y=c.y*mul+add;
         c.xyz=HsbToRgb(c.xyz);
         if(photo)
         {
          //c.xyz=SRGBToLinear(c.xyz); if(flt cur_lin_lum=LinearLumOfLinearColor(c.xyz))c.xyz*=lin_lum/cur_lin_lum; c.xyz=LinearToSRGB(c.xyz);
                                       if(flt cur_lum    =  SRGBLumOfSRGBColor  (c.xyz))c.xyz*=    lum/cur_lum    ; // prefer multiplications in sRGB space, as linear mul may change perceptual contrast and saturation
         }
         image.color3DF(x, y, z, c);
      }
      image.unlock();
   }   
}
void MinSat(Image &image, flt min, C BoxI &box, bool photo)
{
   if(min<1 && image.lock())
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
       //flt  lin_lum; if(photo)lin_lum=LinearLumOfSRGBColor(c.xyz);
         flt      lum; if(photo)    lum=  SRGBLumOfSRGBColor(c.xyz);

         c.xyz=RgbToHsb(c.xyz);
         MIN(c.y, min);
         c.xyz=HsbToRgb(c.xyz);
         if(photo)
         {
          //c.xyz=SRGBToLinear(c.xyz); if(flt cur_lin_lum=LinearLumOfLinearColor(c.xyz))c.xyz*=lin_lum/cur_lin_lum; c.xyz=LinearToSRGB(c.xyz);
                                       if(flt cur_lum    =  SRGBLumOfSRGBColor  (c.xyz))c.xyz*=    lum/cur_lum    ; // prefer multiplications in sRGB space, as linear mul may change perceptual contrast and saturation
         }
         image.color3DF(x, y, z, c);
      }
      image.unlock();
   }   
}
void MulSatH(Image &image, flt red, flt yellow, flt green, flt cyan, flt blue, flt purple, bool sat, bool photo, C BoxI &box)
{
   flt mul[]={red, yellow, green, cyan, blue, purple, red, yellow}; // red and yellow are listed extra for wraparound
   REP(6)if(mul[i]!=1)goto mul; return; mul:
   for(int z=box.min.z; z<box.max.z; z++)
   for(int y=box.min.y; y<box.max.y; y++)
   for(int x=box.min.x; x<box.max.x; x++)
   {
      Vec4 c=image.color3DF(x, y, z);
    //flt  lin_lum; if(photo)lin_lum=LinearLumOfSRGBColor(c.xyz);
      flt      lum; if(photo)    lum=  SRGBLumOfSRGBColor(c.xyz);
      Vec  hsb=RgbToHsb(c.xyz);
      flt  hue=hsb.x*6; int hue_i=Trunc(hue); flt hue_frac=hue-hue_i;
      flt  sat_mul=Lerp(mul[hue_i], mul[hue_i+1], hue_frac);
      if(sat)sat_mul=Lerp(1.0f, sat_mul, hsb.y);
      hsb.y*=sat_mul;
      c.xyz=HsbToRgb(hsb);
      if(photo)
      {
       //c.xyz=SRGBToLinear(c.xyz); if(flt cur_lin_lum=LinearLumOfLinearColor(c.xyz))c.xyz*=lin_lum/cur_lin_lum; c.xyz=LinearToSRGB(c.xyz);
                                    if(flt cur_lum    =  SRGBLumOfSRGBColor  (c.xyz))c.xyz*=    lum/cur_lum    ; // prefer multiplications in sRGB space, as linear mul may change perceptual contrast and saturation
      }
      image.color3DF(x, y, z, c);
   }
}
flt HueDelta(flt a, flt b) // returns -0.5 .. 0.5
{
   flt d=Frac(b-a); if(d>0.5f)d-=1; return d;
}
Vec2  LerpToMad(flt from, flt to) {return Vec2(to-from, from);}
Vec2 ILerpToMad(flt from, flt to) {return Vec2(1/(to-from), from/(from-to));}
flt   FloatSelf(flt x) {return x;}
flt   PowMax   (flt x, flt y) {return (x<=0) ? 0 : Pow(x, y);}

// using sRGB->Sqr gives best results, preserves contrast the most, and looks most similar to Photoshop Brightness
flt _ApplyBrightness(flt x, flt brightness) // !! ASSUMES THAT "brightness!=0" !!
{
   x=SRGBToBrightness(x);
   if(brightness<0)x=SigmoidSqrInv(x*SigmoidSqr(brightness))/           brightness ;
   else            x=SigmoidSqr   (x*           brightness )/SigmoidSqr(brightness);
   return BrightnessToSRGB(x);
}
// contrast has to be done in some gamma space (not linear), because we need linear middle gray ~0.18 to be around 0.5 so that contrast won't change it. So either keep current sRGB gamma, or convert to Linear then Sqrt. sRGB looks most similar to Photoshop Brightness
flt _ApplyContrast(flt x, flt contrast) // !! ASSUMES THAT "contrast!=0" !!
{
   x=SRGBToContrast(x);
   x=x*2-1;
   if(contrast<0)x=SigmoidSqrInv(x*SigmoidSqr(contrast))/           contrast ;
   else          x=SigmoidSqr   (x*           contrast )/SigmoidSqr(contrast);
   x=x*0.5f+0.5f;
   return ContrastToSRGB(x);
}
void ApplyBrightness(flt &x, flt brightness)
{
   if(x>0 && x<1 && brightness)x=_ApplyBrightness(x, brightness);
}
void ApplyContrast(flt &x, flt contrast)
{
   if(x>0 && x<1 && contrast)x=_ApplyContrast(x, contrast);
}

void Crop(Image &image, int x, int y, int w, int h, C Color &background, bool hp)
{
   Vec4 clear_color=background;
   if(image.is())image.crop(image, x, y, w, h, &clear_color);else
   if(image.createSoft(w, h, 1, hp ? IMAGE_F32_4_SRGB : IMAGE_R8G8B8A8_SRGB))
      REPD(y, h)
      REPD(x, w)image.colorF(x, y, clear_color);
}

void TransformImage(Image &image, TextParam param, bool clamp, C Color &background, C VecI &background_size)
{
   BoxI box(0, image.size3());
   {
      Str &s=(param.value.is() ? param.value : param.name);
      int at_pos=TextPosI(s, '@'); if(at_pos>=0)
      {
         VecI4 v=TextVecI4(s()+at_pos+1); // X,Y,W,H
         RectI r(v.xy, v.xy+v.zw);
         box&=BoxI(VecI(r.min, 0), VecI(r.max, box.max.z));
         s.clip(at_pos);
      }
   }

   if(HighPrecTransform(param.name))AdjustImage(image, false, false, true); // if transform might generate high precision values then make sure we can store them

   if(param.name=="size")
   {
      VecI2 v=TextVecI2Ex(param.value);
      Crop(image, 0, 0, v.x, v.y, background);
   }else
   if(param.name=="crop")
   {
      VecI4 v=TextVecI4(param.value);
      Crop(image, v.x, v.y, v.z, v.w, background);
   }else
   if(param.name=="trim")
   {
      VecI4 v;
      Memc<Str> c; Split(c, param.value, ','); switch(c.elms())
      {
         case  1: v=        TextInt  (param.value); break;
         case  2: v.xy=v.zw=TextVecI2(param.value); break;
         case  4: v=        TextVecI4(param.value); break;
         default: v=0; break;
      }
      Crop(image, v.x, v.y, image.w()-v.x-v.z, image.h()-v.y-v.w, background);
   }else
   if(param.name=="extend")
   {
      VecI4 v;
      Memc<Str> c; Split(c, param.value, ','); switch(c.elms())
      {
         case  1: v=        TextInt  (param.value); break;
         case  2: v.xy=v.zw=TextVecI2(param.value); break;
         case  4: v=        TextVecI4(param.value); break;
         default: v=0; break;
      }
      Crop(image, -v.x, -v.y, image.w()+v.x+v.z, image.h()+v.y+v.w, background);
   }else
   if(ResizeTransformAny(param.name))
   {
      VecI2       size        =GetSize       (param.name, param.value, image.size3(), background_size);
      FILTER_TYPE filter      =GetFilter     (param.name);
      bool        resize_clamp=GetClampWrap  (param.name, clamp);
      bool        alpha_weight=GetAlphaWeight(param.name);
      bool        keep_edges  =GetKeepEdges  (param.name);
      image.resize(size.x, size.y, filter, (resize_clamp?IC_CLAMP:IC_WRAP)|(alpha_weight?IC_ALPHA_WEIGHT:0)|(keep_edges?IC_KEEP_EDGES:0));
   }else
   if(Starts(param.name, "rotateScale")){Vec2 rs=param.asVec2(); image.rotateScale(image, DegToRad(rs.x         ), rs.y, GetFilter(param.name));}else
   if(Starts(param.name, "rotate"     ))                         image.rotate     (image, DegToRad(param.asFlt())      , GetFilter(param.name)); else
   if(param.name=="tile")image.tile(TextVecI2Ex(param.value));else
   if(param.name=="inverseRGB")
   {
      if(image.highPrecision())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); c.xyz=1-c.xyz; image.color3DF(x, y, z, c);}
      }else
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++){Color c=image.color3D(x, y, z); c.r=255-c.r; c.g=255-c.g; c.b=255-c.b; image.color3D(x, y, z, c);}
      }
   }else
   if(param.name=="inverseR")
   {
      if(image.highPrecision())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); c.x=1-c.x; image.color3DF(x, y, z, c);}
      }else
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++){Color c=image.color3D(x, y, z); c.r=255-c.r; image.color3D(x, y, z, c);}
      }
   }else
   if(param.name=="inverseG")
   {
      if(image.highPrecision())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); c.y=1-c.y; image.color3DF(x, y, z, c);}
      }else
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++){Color c=image.color3D(x, y, z); c.g=255-c.g; image.color3D(x, y, z, c);}
      }
   }else
   if(param.name=="inverseRG")
   {
      if(image.highPrecision())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); c.x=1-c.x; c.y=1-c.y; image.color3DF(x, y, z, c);}
      }else
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++){Color c=image.color3D(x, y, z); c.r=255-c.r; c.g=255-c.g; image.color3D(x, y, z, c);}
      }
   }else
   if(param.name=="swapRG")
   {
      if(image.highPrecision())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); Swap(c.x, c.y); image.color3DF(x, y, z, c);}
      }else
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++){Color c=image.color3D(x, y, z); Swap(c.r, c.g); image.color3D(x, y, z, c);}
      }
   }else
   if(param.name=="swapXY")
   {
      Image temp; temp.createSoft(image.h(), image.w(), image.d(), image.type());
      if(temp.highPrecision())
      {
         REPD(y, image.h())
         REPD(x, image.w())temp.colorF(y, x, image.colorF(x, y));
      }else
      {
         REPD(y, image.h())
         REPD(x, image.w())temp.color(y, x, image.color(x, y));
      }
      Swap(temp, image);
   }else
   if(param.name=="mirrorX" || param.name=="flipX" )image.mirrorX ();else
   if(param.name=="mirrorY" || param.name=="flipY" )image.mirrorY ();else
   if(param.name=="mirrorXY"|| param.name=="flipXY")image.mirrorXY();else
   if(param.name=="normalize")image.normalize(true, true, true, true, &box);else
   if(param.name=="absRGB")
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         c.xyz.abs();
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="abs")
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         c.abs();
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="satRGB")
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         c.xyz.sat();
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="sat")
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         c.sat();
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="satLum")
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         flt lum=c.xyz.max(); if(lum>1)c.xyz/=lum;
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="blur")
   {
      UNIT_TYPE unit=GetUnitType(param.value);
      Vec       r   =TextVecEx  (param.value);
      r.x=ConvertUnitType(r.x, image.w(), unit);
      r.y=ConvertUnitType(r.y, image.h(), unit);
      r.z=ConvertUnitType(r.z, image.d(), unit);
      if(box.min.allZero() && box.max==image.size3())image.blur(r, clamp);else
      {
         Image temp; image.blur(temp, r, clamp);
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)image.color3DF(x, y, z, temp.color3DF(x, y, z));
      }
   }else
   if(param.name=="sharpen")
   {
      Memc<Str> c; Split(c, param.value, ',');
      if(c.elms()>=1 && c.elms()<=2)
      {
         flt power=TextFlt(c[0]);
         flt range=((c.elms()>=2) ? TextFlt(c[1]) : 1);
         if(box.min.allZero() && box.max==image.size3())image.sharpen(power, range, clamp);else
         {
            Image temp; image.copy(temp); temp.sharpen(power, range, clamp);
            for(int z=box.min.z; z<box.max.z; z++)
            for(int y=box.min.y; y<box.max.y; y++)
            for(int x=box.min.x; x<box.max.x; x++)image.color3DF(x, y, z, temp.color3DF(x, y, z));
         }
      }
   }else
   if(param.name=="lerpRGB")
   {
      Vec from, to; if(TextVecVecEx(param.value, from, to))
      {
         Vec2 ma[3]={LerpToMad(from.x, to.x), LerpToMad(from.y, to.y), LerpToMad(from.z, to.z)};
         AdjustImage(image, true, false, false); image.mulAdd(Vec4(ma[0].x, ma[1].x, ma[2].x, 1), Vec4(ma[0].y, ma[1].y, ma[2].y, 0), &box);
      }
   }else
   if(param.name=="iLerpRGB")
   {
      Vec from, to; if(TextVecVecEx(param.value, from, to))
      {
         Vec2 ma[3]={ILerpToMad(from.x, to.x), ILerpToMad(from.y, to.y), ILerpToMad(from.z, to.z)};
         AdjustImage(image, true, false, false); image.mulAdd(Vec4(ma[0].x, ma[1].x, ma[2].x, 1), Vec4(ma[0].y, ma[1].y, ma[2].y, 0), &box);
      }
   }else
   if(param.name=="lerpLum")
   {
      Vec2 range=param.asVec2();
      Vec2 ma=LerpToMad(range.x, range.y);
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         flt lum=c.xyz.max()*ma.x+ma.y;
         c.xyz*=lum;
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="iLerpLum")
   {
      Vec2 range=param.asVec2();
      Vec2 ma=ILerpToMad(range.x, range.y);
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         flt lum=c.xyz.max()*ma.x+ma.y;
         c.xyz*=lum;
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="mulA"  ){flt alpha=param.asFlt(); if(alpha!=1){AdjustImage(image, false, true, false); image.mulAdd(Vec4(1, 1, 1, alpha), 0, &box);}}else
   if(param.name=="mulRGB")
   {
      Vec4 mul(TextVecEx(param.value), 1);
      if(mul.xyz.anyDifferent())AdjustImage(image, true, false, false);
      image.mulAdd(mul, Vec4Zero, &box);
   }else
   if(param.name=="addRGB")
   {
      Vec4 add(TextVecEx(param.value), 0);
      if(add.xyz.anyDifferent())AdjustImage(image, true, false, false);
      image.mulAdd(1, add, &box);
   }else
   if(param.name=="setRGB")
   {
      Vec4 add(TextVecEx(param.value), 0);
      if(add.xyz.anyDifferent())AdjustImage(image, true, false, false);
      image.mulAdd(Vec4(0, 0, 0, 1), add, &box);
   }else
   if(param.name=="setRGBA")
   {
      Vec4 add=param.asVec4();
      bool rgb=add.xyz.anyDifferent(), alpha=(add.w!=1);
      if(rgb || alpha)AdjustImage(image, rgb, alpha, false);
      image.mulAdd(Vec4(0, 0, 0, 1), add, &box);
   }else
   if(param.name=="mulAddRGB")
   {
      Vec mul, add; if(TextVecVecEx(param.value, mul, add)){AdjustImage(image, true, false, false); image.mulAdd(Vec4(mul, 1), Vec4(add, 0), &box);}
   }else
   if(param.name=="addMulRGB")
   {  // x=x*m+a, x=(x+A)*M
      Vec add, mul; if(TextVecVecEx(param.value, add, mul)){AdjustImage(image, true, false, false); image.mulAdd(Vec4(mul, 1), Vec4(add*mul, 0), &box);}
   }else
   if(param.name=="mulRGBLin")
   {
      Vec mul=TextVecEx(param.value);
      if( mul!=VecOne)
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         c.xyz=LinearToSRGB(SRGBToLinear(c.xyz)*mul);
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="mulRGBbyA")
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         c.xyz*=c.w;
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="mulRGBS")
   {
      Vec mul=TextVecEx(param.value);
      if( mul!=VecOne)
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         flt  sat=RgbToHsb(c.xyz).y;
         c.x=Lerp(c.x, c.x*mul.x, sat); // red
         c.y=Lerp(c.y, c.y*mul.y, sat); // green
         c.z=Lerp(c.z, c.z*mul.z, sat); // blue
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="mulRGBIS")
   {
      Vec mul=TextVecEx(param.value);
      if( mul!=VecOne)
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         flt  sat=RgbToHsb(c.xyz).y;
         c.x=Lerp(c.x*mul.x, c.x, sat); // red
         c.y=Lerp(c.y*mul.y, c.y, sat); // green
         c.z=Lerp(c.z*mul.z, c.z, sat); // blue
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="mulRGBH")
   {
      Mems<Str> vals; Split(vals, param.value, ',');
      switch(vals.elms())
      {
         case 1: {flt v=TextFlt(vals[0]); MulRGBH(image, v, v, v, v, v, v, box);} break;
         case 3: {Vec v(TextFlt(vals[0]), TextFlt(vals[1]), TextFlt(vals[2])); MulRGBH(image, v.x, Avg(v.x, v.y), v.y, Avg(v.y, v.z), v.z, Avg(v.z, v.x), box);} break;
         case 6: MulRGBH(image, TextFlt(vals[0]), TextFlt(vals[1]), TextFlt(vals[2]), TextFlt(vals[3]), TextFlt(vals[4]), TextFlt(vals[5]), box); break;
      }
   }else
   if(param.name=="mulRGBHS")
   {
      Mems<Str> vals; Split(vals, param.value, ',');
      switch(vals.elms())
      {
         case 1: {flt v=TextFlt(vals[0]); MulRGBHS(image, v, v, v, v, v, v, box);} break;
         case 3: {Vec v(TextFlt(vals[0]), TextFlt(vals[1]), TextFlt(vals[2])); MulRGBHS(image, v.x, Avg(v.x, v.y), v.y, Avg(v.y, v.z), v.z, Avg(v.z, v.x), box);} break;
         case 6: MulRGBHS(image, TextFlt(vals[0]), TextFlt(vals[1]), TextFlt(vals[2]), TextFlt(vals[3]), TextFlt(vals[4]), TextFlt(vals[5]), box); break;
      }
   }else
   if(param.name=="getSat")
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         flt  sat=RgbToHsb(c.xyz).y;
         c.xyz=sat; c.w=1;
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="getHue")
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         flt  hue=RgbToHsb(c.xyz).x;
         c.xyz=hue; c.w=1;
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="gamma")
   {
      Vec g=TextVecEx(param.value);
      if(g!=VecOne)
      {
         if(g.anyDifferent())AdjustImage(image, true, false, false);
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); c.xyz.set(PowMax(c.x, g.x), PowMax(c.y, g.y), PowMax(c.z, g.z)); image.color3DF(x, y, z, c);}
      }
   }else
   if(param.name=="gammaA")
   {
      flt g=param.asFlt();
      if(g!=1)
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); c.w=PowMax(c.w, g); image.color3DF(x, y, z, c);}
   }else
   if(param.name=="gammaLum")
   {
      flt g=param.asFlt();
      if(g!=1)
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); if(flt lum=c.xyz.max()){c.xyz*=PowMax(lum, g)/lum; image.color3DF(x, y, z, c);}}
   }else
   if(param.name=="gammaLumPhoto")
   {
      flt g=param.asFlt();
      if(g!=1)
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++){Vec4 c=image.color3DF(x, y, z); if(flt lum=SRGBLumOfSRGBColor(c.xyz)){c.xyz*=PowMax(lum, g)/lum; image.color3DF(x, y, z, c);}}
   }else
   if(param.name=="SRGBToLinear")
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)image.color3DF(x, y, z, SRGBToLinear(image.color3DF(x, y, z)));
   }else
   if(param.name=="LinearToSRGB")
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)image.color3DF(x, y, z, LinearToSRGB(image.color3DF(x, y, z)));
   }else
   if(param.name=="brightness")
   {
      Vec bright=TextVecEx(param.value), mul; if(bright.any())
      {
         if(bright.anyDifferent())AdjustImage(image, true, false, false);
         flt (*R)(flt);
         flt (*G)(flt);
         flt (*B)(flt);
         if(!bright.x){bright.x=1; mul.x=1; R=FloatSelf;}else if(bright.x<0){mul.x=1/bright.x; bright.x=SigmoidSqr(bright.x); R=SigmoidSqrInv;}else{mul.x=1/SigmoidSqr(bright.x); R=SigmoidSqr;}
         if(!bright.y){bright.y=1; mul.y=1; G=FloatSelf;}else if(bright.y<0){mul.y=1/bright.y; bright.y=SigmoidSqr(bright.y); G=SigmoidSqrInv;}else{mul.y=1/SigmoidSqr(bright.y); G=SigmoidSqr;}
         if(!bright.z){bright.z=1; mul.z=1; B=FloatSelf;}else if(bright.z<0){mul.z=1/bright.z; bright.z=SigmoidSqr(bright.z); B=SigmoidSqrInv;}else{mul.z=1/SigmoidSqr(bright.z); B=SigmoidSqr;}
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            if(c.x>0 && c.x<1)c.x=BrightnessToSRGB(R(SRGBToBrightness(c.x)*bright.x)*mul.x);
            if(c.y>0 && c.y<1)c.y=BrightnessToSRGB(G(SRGBToBrightness(c.y)*bright.y)*mul.y);
            if(c.z>0 && c.z<1)c.z=BrightnessToSRGB(B(SRGBToBrightness(c.z)*bright.z)*mul.z);
            image.color3DF(x, y, z, c);
         }
      }
   }else
   if(param.name=="brightnessLum")
   {
      flt bright=param.asFlt(), mul; flt (*f)(flt);
      if( bright)
      {
         if(bright<0){mul=1/           bright ; bright=SigmoidSqr(bright); f=SigmoidSqrInv;}
         else        {mul=1/SigmoidSqr(bright);                            f=SigmoidSqr   ;}
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z); flt old_lum=c.xyz.max();
            if(old_lum>0 && old_lum<1)
            {
               flt new_lum=SRGBToBrightness(old_lum);
               new_lum=f(new_lum*bright)*mul;
               new_lum=BrightnessToSRGB(new_lum);
               c.xyz*=new_lum/old_lum;
               image.color3DF(x, y, z, c);
            }
         }
      }
   }else
   if(param.name=="brightnessLumS")
   {
      if(flt bright=param.asFlt())
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         flt old_lum=c.xyz.max(); if(old_lum>0 && old_lum<1)
         {
            flt sat=RgbToHsb(c.xyz).y; if(flt b=bright*sat)
            {
               flt new_lum=_ApplyBrightness(old_lum, b);
               c.xyz*=new_lum/old_lum;
               image.color3DF(x, y, z, c);
            }
         }
      }
   }else
   if(param.name=="sigmoid")
   {
      Vec contrast=TextVecEx(param.value), mul; if(contrast.any())
      {
         if(contrast.anyDifferent())AdjustImage(image, true, false, false);
         flt (*R)(flt);
         flt (*G)(flt);
         flt (*B)(flt);
         if(!contrast.x){contrast.x=1; mul.x=1; R=FloatSelf;}else if(contrast.x<0){mul.x=1/contrast.x; contrast.x=SigmoidSqr(contrast.x); R=SigmoidSqrInv;}else{mul.x=1/SigmoidSqr(contrast.x); R=SigmoidSqr;}
         if(!contrast.y){contrast.y=1; mul.y=1; G=FloatSelf;}else if(contrast.y<0){mul.y=1/contrast.y; contrast.y=SigmoidSqr(contrast.y); G=SigmoidSqrInv;}else{mul.y=1/SigmoidSqr(contrast.y); G=SigmoidSqr;}
         if(!contrast.z){contrast.z=1; mul.z=1; B=FloatSelf;}else if(contrast.z<0){mul.z=1/contrast.z; contrast.z=SigmoidSqr(contrast.z); B=SigmoidSqrInv;}else{mul.z=1/SigmoidSqr(contrast.z); B=SigmoidSqr;}
         Vec ofs=contrast;
         contrast*=2;
         mul     *=0.5f;
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            if(c.x>0 && c.x<1)c.x=ContrastToSRGB(R(SRGBToContrast(c.x)*contrast.x-ofs.x)*mul.x+0.5f);
            if(c.y>0 && c.y<1)c.y=ContrastToSRGB(G(SRGBToContrast(c.y)*contrast.y-ofs.y)*mul.y+0.5f);
            if(c.z>0 && c.z<1)c.z=ContrastToSRGB(B(SRGBToContrast(c.z)*contrast.z-ofs.z)*mul.z+0.5f);
            image.color3DF(x, y, z, c);
         }
      }
   }else
   if(param.name=="sigmoidLum")
   {
      flt contrast=param.asFlt(), mul; flt (*f)(flt);
      if( contrast)
      {
         if(contrast<0){mul=1/           contrast ; contrast=SigmoidSqr(contrast); f=SigmoidSqrInv;}
         else          {mul=1/SigmoidSqr(contrast);                                f=SigmoidSqr   ;}
         flt ofs=contrast;
         contrast*=2;
         mul     *=0.5f;
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z); flt old_lum=c.xyz.max();
            if(old_lum>0 && old_lum<1)
            {
               flt new_lum=SRGBToContrast(old_lum);
               new_lum=f(new_lum*contrast-ofs)*mul+0.5f;
               new_lum=ContrastToSRGB(new_lum);
               c.xyz*=new_lum/old_lum;
               image.color3DF(x, y, z, c);
            }
         }
      }
   }else
   if(param.name=="contrast")
   {
      Vec contrast=TextVecEx(param.value); if(contrast!=VecOne)
      {
         Vec4 avg; if(image.stats(null, null, &avg, null, null, null, null, &box))
         {
            if(contrast.anyDifferent())AdjustImage(image, true, false, false);
            // col=(col-avg)*contrast+avg
            // col=col*contrast+avg*(1-contrast)
            image.mulAdd(Vec4(contrast, 1), Vec4(avg.xyz*(Vec(1)-contrast), 0), &box);
         }
      }
   }else
   if(param.name=="contrastAlphaWeight")
   {
      Vec contrast=TextVecEx(param.value); if(contrast!=VecOne)
      {
         Vec avg; if(image.stats(null, null, null, null, null, &avg, null, &box))
         {
            if(contrast.anyDifferent())AdjustImage(image, true, false, false);
            image.mulAdd(Vec4(contrast, 1), Vec4(avg*(Vec(1)-contrast), 0), &box);
         }
      }
   }else
   if(param.name=="contrastLum")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         Vec4 avg; if(image.stats(null, null, &avg, null, null, null, null, &box))ContrastLum(image, contrast, avg.xyz.max(), box);
      }
   }else
   if(param.name=="contrastLumAlphaWeight")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         Vec avg; if(image.stats(null, null, null, null, null, &avg, null, &box))ContrastLum(image, contrast, avg.max(), box);
      }
   }else
   if(param.name=="contrastHue")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         Vec4 avg; if(image.stats(null, null, &avg, null, null, null, null, &box))ContrastHue(image, contrast, avg.xyz, box);
      }
   }else
   if(param.name=="contrastHuePhoto")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         Vec4 avg; if(image.stats(null, null, &avg, null, null, null, null, &box))ContrastHue(image, contrast, avg.xyz, box, true);
      }
   }else
   if(param.name=="medContrastHue")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         Vec4 med; if(image.stats(null, null, null, &med, null, null, null, &box))ContrastHue(image, contrast, med.xyz, box);
      }
   }else
   if(param.name=="medContrastHuePhoto")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         Vec4 med; if(image.stats(null, null, null, &med, null, null, null, &box))ContrastHue(image, contrast, med.xyz, box, true);
      }
   }else
   if(param.name=="contrastHueAlphaWeight")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         Vec avg; if(image.stats(null, null, null, null, null, &avg, null, &box))ContrastHue(image, contrast, avg, box);
      }
   }else
   if(param.name=="contrastHuePhotoAlphaWeight")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         Vec avg; if(image.stats(null, null, null, null, null, &avg, null, &box))ContrastHue(image, contrast, avg, box, true);
      }
   }else
   if(param.name=="contrastHuePow")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         Vec4 avg; if(image.stats(null, null, &avg, null, null, null, null, &box) && image.lock())
         {
            flt avg_hue=RgbToHsb(avg.xyz).x;
            for(int z=box.min.z; z<box.max.z; z++)
            for(int y=box.min.y; y<box.max.y; y++)
            for(int x=box.min.x; x<box.max.x; x++)
            {
               Vec4 c=image.color3DF(x, y, z);
               c.xyz=RgbToHsb(c.xyz);
               flt d_hue=HueDelta(avg_hue, c.x);
               d_hue=Sign(d_hue)*Pow(Abs(d_hue)*2, contrast)/2; // *2 to get -1..1 range
               Clamp(d_hue, -0.5f, 0.5f); // clamp so we don't go back
               c.x=d_hue+avg_hue;
               c.xyz=HsbToRgb(c.xyz);
               image.color3DF(x, y, z, c);
            }
            image.unlock();
         }
      }
   }else
   if(param.name=="contrastSat")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         flt avg; if(image.statsSat(null, null, &avg, null, null, null, null, &box))ContrastSat(image, contrast, avg, box);
      }
   }else
   if(param.name=="contrastSatPhoto")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         flt avg; if(image.statsSat(null, null, &avg, null, null, null, null, &box))ContrastSat(image, contrast, avg, box, true);
      }
   }else
   if(param.name=="medContrastSat")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         flt med; if(image.statsSat(null, null, null, &med, null, null, null, &box))ContrastSat(image, contrast, med, box);
      }
   }else
   if(param.name=="contrastSatAlphaWeight")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         flt avg; if(image.statsSat(null, null, null, null, null, &avg, null, &box))ContrastSat(image, contrast, avg, box);
      }
   }else
   if(param.name=="contrastSatPhotoAlphaWeight")
   {
      flt contrast=param.asFlt(); if(contrast!=1)
      {
         flt avg; if(image.statsSat(null, null, null, null, null, &avg, null, &box))ContrastSat(image, contrast, avg, box, true);
      }
   }else
   if(param.name=="avgLum")
   {
      Vec4 avg; if(image.stats(null, null, &avg, null, null, null, null, &box))if(flt avg_l=avg.xyz.max())image.mulAdd(Vec4(Vec(param.asFlt()/avg_l), 1), 0, &box);
   }else
   if(param.name=="medLum")
   {
      Vec4 med; if(image.stats(null, null, null, &med, null, null, null, &box))if(flt med_l=med.xyz.max())image.mulAdd(Vec4(Vec(param.asFlt()/med_l), 1), 0, &box);
   }else
   if(param.name=="avgContrastLum")
   {
      Vec4 avg; if(image.stats(null, null, &avg, null, null, null, null, &box))AvgContrastLum(image, param.asFlt(), avg.xyz.max(), box);
   }else
   if(param.name=="medContrastLum")
   {
      Vec4 med; if(image.stats(null, null, null, &med, null, null, null, &box))AvgContrastLum(image, param.asFlt(), med.xyz.max(), box);
   }else
   if(param.name=="addHue"     )AddHue(image, param.asFlt(), box);else
   if(param.name=="addHuePhoto")AddHue(image, param.asFlt(), box, true);else
   if(param.name=="avgHue" || param.name=="avgHuePhoto")
   {
      if(image.lock()) // lock for writing because we will use this lock for applying hue too
      {
         Vec4 col; if(image.stats(null, null, &col, null, null, null, null, &box))AddHue(image, HueDelta(RgbToHsb(col.xyz).x, param.asFlt()), box, param.name=="avgHuePhoto");
         image.unlock();
      }
   }else
   if(param.name=="avgHueAlphaWeight" || param.name=="avgHuePhotoAlphaWeight")
   {
      if(image.lock()) // lock for writing because we will use this lock for applying hue too
      {
         Vec col; if(image.stats(null, null, null, null, null, &col, null, &box))AddHue(image, HueDelta(RgbToHsb(col).x, param.asFlt()), box, param.name=="avgHuePhotoAlphaWeight");
         image.unlock();
      }
   }else
   if(param.name=="medHue" || param.name=="medHuePhoto")
   {
      if(image.lock()) // lock for writing because we will use this lock for applying hue too
      {
         Vec4 col; if(image.stats(null, null, null, &col, null, null, null, &box))AddHue(image, HueDelta(RgbToHsb(col.xyz).x, param.asFlt()), box, param.name=="medHuePhoto");
         image.unlock();
      }
   }else
   if(param.name=="medHueAlphaWeight" || param.name=="medHuePhotoAlphaWeight")
   {
      if(image.lock()) // lock for writing because we will use this lock for applying hue too
      {
         Vec col; if(image.stats(null, null, null, null, null, null, &col, &box))AddHue(image, HueDelta(RgbToHsb(col).x, param.asFlt()), box, param.name=="medHuePhotoAlphaWeight");
         image.unlock();
      }
   }else
   if(param.name=="gammaSat"     )GammaSat(image, param.asFlt(), box);else
   if(param.name=="gammaSatPhoto")GammaSat(image, param.asFlt(), box, true);else
   if(param.name=="mulSat"        )MulAddSat(image, param.asFlt(), 0, box);else
   if(param.name=="mulSatPhoto"   )MulAddSat(image, param.asFlt(), 0, box, true);else
   if(param.name=="mulAddSat"     ){Vec2 ma=param.asVec2(); MulAddSat(image, ma.x, ma.y, box);}else
   if(param.name=="mulAddSatPhoto"){Vec2 ma=param.asVec2(); MulAddSat(image, ma.x, ma.y, box, true);}else
   if(param.name=="addSat"        )MulAddSat(image, 1, param.asFlt(), box);else
   if(param.name=="addSatPhoto"   )MulAddSat(image, 1, param.asFlt(), box, true);else
   if(param.name=="setSat"        )MulAddSat(image, 0, param.asFlt(), box);else
   if(param.name=="setSatPhoto"   )MulAddSat(image, 0, param.asFlt(), box, true);else
   if(param.name=="minSat"        )MinSat   (image, param.asFlt(), box);else
   if(param.name=="minSatPhoto"   )MinSat   (image, param.asFlt(), box, true);else
   if(param.name=="avgSat" || param.name=="avgSatPhoto")
   {
      flt avg; if(image.statsSat(null, null, &avg, null, null, null, null, &box))if(avg)MulAddSat(image, param.asFlt()/avg, 0, box, param.name=="avgSatPhoto");
   }else
   if(param.name=="medSat" || param.name=="medSatPhoto")
   {
      flt med; if(image.statsSat(null, null, null, &med, null, null, null, &box))if(med)MulAddSat(image, param.asFlt()/med, 0, box, param.name=="medSatPhoto");
   }else
   if(param.name=="mulSatH"
   || param.name=="mulSatHS")
   {
      bool sat=(param.name=="mulSatHS");
      Mems<Str> vals; Split(vals, param.value, ',');
      switch(vals.elms())
      {
         case 1: {flt v=TextFlt(vals[0]); MulSatH(image, v, v, v, v, v, v, sat, false, box);} break;
         case 3: {Vec v(TextFlt(vals[0]), TextFlt(vals[1]), TextFlt(vals[2])); MulSatH(image, v.x, Avg(v.x, v.y), v.y, Avg(v.y, v.z), v.z, Avg(v.z, v.x), sat, false, box);} break;
         case 6: MulSatH(image, TextFlt(vals[0]), TextFlt(vals[1]), TextFlt(vals[2]), TextFlt(vals[3]), TextFlt(vals[4]), TextFlt(vals[5]), sat, false, box); break;
      }
   }else
   if(param.name=="mulSatHPhoto"
   || param.name=="mulSatHSPhoto")
   {
      bool sat=(param.name=="mulSatHSPhoto");
      Mems<Str> vals; Split(vals, param.value, ',');
      switch(vals.elms())
      {
         case 1: {flt v=TextFlt(vals[0]); MulSatH(image, v, v, v, v, v, v, sat, true, box);} break;
         case 3: {Vec v(TextFlt(vals[0]), TextFlt(vals[1]), TextFlt(vals[2])); MulSatH(image, v.x, Avg(v.x, v.y), v.y, Avg(v.y, v.z), v.z, Avg(v.z, v.x), sat, true, box);} break;
         case 6: MulSatH(image, TextFlt(vals[0]), TextFlt(vals[1]), TextFlt(vals[2]), TextFlt(vals[3]), TextFlt(vals[4]), TextFlt(vals[5]), sat, true, box); break;
      }
   }else
   if(param.name=="addHueSat")
   {
      Vec2 hue_sat=param.asVec2(); if(hue_sat.any() && image.lock())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            c.xyz=RgbToHsb(c.xyz);
            c.xy+=hue_sat;
            c.xyz=HsbToRgb(c.xyz);
            image.color3DF(x, y, z, c);
         }
         image.unlock();
      }
   }else
   if(param.name=="setHue")
   {
      if(image.lock())
      {
         flt hue=param.asFlt();
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            c.xyz=RgbToHsb(c.xyz);
            c.x=hue;
            c.xyz=HsbToRgb(c.xyz);
            image.color3DF(x, y, z, c);
         }
         image.unlock();
      }
   }else
   if(param.name=="setHuePhoto")
   {
      if(image.lock())
      {
         flt hue=param.asFlt();
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            if(flt lum=SRGBLumOfSRGBColor(c.xyz))
            {
               Vec hsb=RgbToHsb(c.xyz);
               hsb.x=hue;
               c.xyz=HsbToRgb(hsb);
               c.xyz*=lum/SRGBLumOfSRGBColor(c.xyz);
               image.color3DF(x, y, z, c);
            }
         }
         image.unlock();
      }
   }else
   if(param.name=="setHueSat")
   {
      if(image.lock())
      {
         Vec2 hue_sat=param.asVec2();
         Vec  rgb=HsbToRgb(Vec(hue_sat, 1));
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            c.xyz=rgb*c.xyz.max();
            image.color3DF(x, y, z, c);
         }
         image.unlock();
      }
   }else
   if(param.name=="setHueSatPhoto") // photometric
   {
      if(image.lock())
      {
         Vec2 hue_sat=param.asVec2();
         Vec  rgb=HsbToRgb(Vec(hue_sat, 1)); rgb/=SRGBLumOfSRGBColor(rgb);
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            if(flt lum=SRGBLumOfSRGBColor(c.xyz))
            {
               c.xyz=rgb*lum;
               image.color3DF(x, y, z, c);
            }
         }
         image.unlock();
      }
   }else
   if(param.name=="lerpHue")
   {
      Vec2 hue_alpha=param.asVec2(); if(hue_alpha.y && image.lock())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            Vec hsb=RgbToHsb(c.xyz);
            hsb.x=hue_alpha.x;
            c.xyz=Lerp(c.xyz, HsbToRgb(hsb), hue_alpha.y);
            image.color3DF(x, y, z, c);
         }
         image.unlock();
      }
   }else
   if(param.name=="lerpHueSat")
   {
      Vec hue_sat_alpha=param.asVec(); if(hue_sat_alpha.z && image.lock())
      {
         Vec rgb=HsbToRgb(Vec(hue_sat_alpha.xy, 1));
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            c.xyz=Lerp(c.xyz, rgb*c.xyz.max(), hue_sat_alpha.z);
            image.color3DF(x, y, z, c);
         }
         image.unlock();
      }
   }else
   if(param.name=="rollHue")
   {
      Vec2 hue_alpha=param.asVec2(); if(hue_alpha.y && image.lock())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            Vec hsb=RgbToHsb(c.xyz);
            hsb.x+=HueDelta(hsb.x, hue_alpha.x)*hue_alpha.y;
            c.xyz=HsbToRgb(hsb);
            image.color3DF(x, y, z, c);
         }
         image.unlock();
      }
   }else
   if(param.name=="rollHueSat")
   {
      Vec hue_sat_alpha=param.asVec(); if(hue_sat_alpha.z && image.lock())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            Vec hsb=RgbToHsb(c.xyz);
            hsb.x+=HueDelta(hsb.x, hue_sat_alpha.x)*hue_sat_alpha.z;
            hsb.y =Lerp    (hsb.y, hue_sat_alpha.y, hue_sat_alpha.z);
            c.xyz=HsbToRgb(hsb);
            image.color3DF(x, y, z, c);
         }
         image.unlock();
      }
   }else
   if(param.name=="rollHuePhoto")
   {
      Vec2 hue_alpha=param.asVec2(); if(hue_alpha.y && image.lock())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            if(flt l=SRGBLumOfSRGBColor(c.xyz))
            {
               Vec hsb=RgbToHsb(c.xyz);
               hsb.x+=HueDelta(hsb.x, hue_alpha.x)*hue_alpha.y;
               c.xyz=HsbToRgb(hsb);
               c.xyz*=l/SRGBLumOfSRGBColor(c.xyz);
               image.color3DF(x, y, z, c);
            }
         }
         image.unlock();
      }
   }else
   if(param.name=="rollHueSatPhoto")
   {
      Vec hue_sat_alpha=param.asVec(); if(hue_sat_alpha.z && image.lock())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            if(flt l=SRGBLumOfSRGBColor(c.xyz))
            {
               Vec hsb=RgbToHsb(c.xyz);
               hsb.x+=HueDelta(hsb.x, hue_sat_alpha.x)*hue_sat_alpha.z;
               hsb.y =Lerp    (hsb.y, hue_sat_alpha.y, hue_sat_alpha.z);
               c.xyz=HsbToRgb(hsb);
               c.xyz*=l/SRGBLumOfSRGBColor(c.xyz);
               image.color3DF(x, y, z, c);
            }
         }
         image.unlock();
      }
   }else
   if(param.name=="grey")
   {
      if(image.lock())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            c.xyz=c.xyz.max();
            image.color3DF(x, y, z, c);
         }
         image.unlock();
      }
   }else
   if(param.name=="greyPhoto")
   {
      if(image.lock())
      {
         for(int z=box.min.z; z<box.max.z; z++)
         for(int y=box.min.y; y<box.max.y; y++)
         for(int x=box.min.x; x<box.max.x; x++)
         {
            Vec4 c=image.color3DF(x, y, z);
            c.xyz=SRGBLumOfSRGBColor(c.xyz);
            image.color3DF(x, y, z, c);
         }
         image.unlock();
      }
   }else
   if(param.name=="min")
   {
      Vec min=TextVecEx(param.value);
      if( min.anyDifferent())AdjustImage(image, true, false, false);
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         c.xyz=Min(c.xyz, min);
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="max")
   {
      Vec max=TextVecEx(param.value);
      if( max.anyDifferent())AdjustImage(image, true, false, false);
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         c.xyz=Max(c.xyz, max);
         image.color3DF(x, y, z, c);
      }
   }else
   if(param.name=="maxLum")
   {
      flt max=param.asFlt();
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         flt lum=c.xyz.max(); if(lum<max)
         {
            if(lum)c.xyz*=max/lum;
            else   c.xyz =max;
         }
         image.color3DF(x, y, z, c);
      }
   }else
   /*if(param.name=="metalToReflect")
   {
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         c.x=Lerp(0.04, 1, c.x);
         c.y=Lerp(0.04, 1, c.y);
         c.z=Lerp(0.04, 1, c.z);
         image.color3DF(x, y, z, c);
      }
   }else*/
   if(param.name=="channel") // Warning: this loses sRGB for 1..2 channels, because there are no IMAGE_R8_SRGB, IMAGE_R8G8_SRGB, IMAGE_F32_SRGB, IMAGE_F32_2_SRGB
   {
      int channels=param.value.length();
      if( channels>=1 && channels<=4)
      {
         int chn[4]; REPAO(chn)=ChannelIndex(param.value[i]);
         if(!(chn[0]==0 && chn[1]==1 && chn[2]==2 && (chn[3]==3 || chn[3]==-1 && !image.typeInfo().a))) // ignore identity RGBA and (RGB when image doesn't have alpha)
         {
            Image temp;
            bool  srgb=image.sRGB();
            if(image.highPrecision())
            {
               temp.createSoft(image.w(), image.h(), image.d(), channels==1 ? IMAGE_F32 : channels==2 ? IMAGE_F32_2 : channels==3 ? (srgb ? IMAGE_F32_3_SRGB : IMAGE_F32_3) : (srgb ? IMAGE_F32_4_SRGB : IMAGE_F32_4));
               Vec4 d(0, 0, 0, 1);
               REPD(z, image.d())
               REPD(y, image.h())
               REPD(x, image.w())
               {
                  Vec4 c=image.color3DF(x, y, z);
                  REPA(d.c){int ch=chn[i]; if(InRange(ch, c.c))d.c[i]=c.c[ch];}
                  temp.color3DF(x, y, z, d);
               }
            }else
            {
               temp.createSoft(image.w(), image.h(), image.d(), channels==1 ? IMAGE_R8 : channels==2 ? IMAGE_R8G8 : channels==3 ? (srgb ? IMAGE_R8G8B8_SRGB : IMAGE_R8G8B8) : (srgb ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8A8));
               Color d(0, 0, 0, 255);
               REPD(z, image.d())
               REPD(y, image.h())
               REPD(x, image.w())
               {
                  Color c=image.color3D(x, y, z);
                  REPA(d.c){int ch=chn[i]; if(InRange(ch, c.c))d.c[i]=c.c[ch];}
                  temp.color3D(x, y, z, d);
               }
            }
            Swap(temp, image);
         }
      }
   }else
   if(param.name=="alphaFromBrightness" || param.name=="alphaFromLum" || param.name=="alphaFromLuminance")
   {
      image.alphaFromBrightness();
   }else
   if(param.name=="bump" || param.name=="bumpClamp")
   {
      Vec2 blur=-1; // x=min, y=max, -1=auto
      if(param.value.is())
      {
         UNIT_TYPE unit=GetUnitType(param.value);
         flt       full=image.size().avgF();
         if(Contains(param.value, ','))
         {
            blur=param.asVec2(); // use 2 values if specified
            blur.x=ConvertUnitType(blur.x, full, unit);
            blur.y=ConvertUnitType(blur.y, full, unit);
         }else
         {
            blur.y=param.asFlt(); // if 1 value specified then use as max
            blur.y=ConvertUnitType(blur.y, full, unit);
         }
      }
      CreateBumpFromColor(image, image, blur.x, blur.y, param.name=="bumpClamp");
   }else
   if(param.name=="bumpToNormal")
   {
      image.bumpToNormal(image, param.value.is() ? param.asFlt() : image.size().avgF()*BUMP_TO_NORMAL_SCALE);
   } //else TODO: FIX FOR "fatal error C1061: compiler limit: blocks nested too deeply"
   if(param.name=="scale") // the formula is ok (for normal too), it works as if the bump was scaled vertically by 'scale' factor
   {
      flt scale=param.asFlt(); if(scale!=1)
      {
         if(image.typeChannels()<=1 || image.monochromatic())image.mulAdd(Vec4(Vec(scale), 1), Vec4(Vec(-0.5f*scale+0.5f), 0), &box);else // if image is 1-channel or monochromatic then we need to transform all RGB together
         if(!scale                                          )image.mulAdd(Vec4(Vec(    0), 1), Vec4(        0.5f, 0.5f, 1, 0), &box);else // if zero scale then set Vec(0.5, 0.5, 1)
         if(image.lock())
         {
            scale=1/scale;
            for(int z=box.min.z; z<box.max.z; z++)
            for(int y=box.min.y; y<box.max.y; y++)
            for(int x=box.min.x; x<box.max.x; x++)
            {
               Vec4 c=image.color3DF(x, y, z); Vec &n=c.xyz;
               n=n*2-1;
               n.normalize();
               n.z*=scale;
               n.normalize();
               n=n*0.5f+0.5f;
               image.color3DF(x, y, z, c);
            }
            image.unlock();
         }
      }
   }else
   if(param.name=="scaleXY")
   {
      Vec2 r=TextVec2Ex(param.value);
      // v2=(v2-0.5)*r+0.5
      // v2=v2*r-0.5*r+0.5
      if(image.typeChannels()<=1 || image.monochromatic()){flt a=r.avg(); image.mulAdd(Vec4(Vec(a), 1), Vec4(Vec(-0.5f*a+0.5f)  , 0), &box);} // if image is 1-channel or monochromatic then we need to transform all RGB together
      else                                                                image.mulAdd(Vec4(  r, 1, 1), Vec4(    -0.5f*r+0.5f, 0, 0), &box);
   }else
   if(param.name=="scaleAngle") // the formula is ok (for normal too), it works as if the bump was scaled vertically by 'scale' factor
   {
      flt scale=param.asFlt(); if(scale!=1)
      {
         if(image.typeChannels()<=1 || image.monochromatic())image.mulAdd(Vec4(Vec(scale), 1), Vec4(Vec(-0.5f*scale+0.5f), 0), &box);else // if image is 1-channel or monochromatic then we need to transform all RGB together
         if(!scale                                          )image.mulAdd(Vec4(Vec(    0), 1), Vec4(        0.5f, 0.5f, 1, 0), &box);else // if zero scale then set Vec(0.5, 0.5, 1)
         if(image.lock())
         {
            for(int z=box.min.z; z<box.max.z; z++)
            for(int y=box.min.y; y<box.max.y; y++)
            for(int x=box.min.x; x<box.max.x; x++)
            {
               Vec4 c=image.color3DF(x, y, z); Vec &n=c.xyz;
               n=n*2-1;
               n.normalize();
               Vec2 p(n.z, n.xy.length());
               flt angle=Angle(p);
               angle*=scale;
               Clamp(angle, -PI_2, PI_2);
               CosSin(p.x, p.y, angle);
               n.z=p.x;
               n.xy.setLength(p.y);
               n=n*0.5f+0.5f;
               image.color3DF(x, y, z, c);
            }
            image.unlock();
         }
      }
   }else
   if(param.name=="fixTransparent")
   {
      image.transparentToNeighbor(true, param.value.is() ? param.asFlt() : 1);
   }else
   if(param.name=="uniformTransparent")
   {
      Vec4 avg; avg.w=0; if(image.stats(null, null, null, null, null, &avg.xyz, null, &box))
      for(int z=box.min.z; z<box.max.z; z++)
      for(int y=box.min.y; y<box.max.y; y++)
      for(int x=box.min.x; x<box.max.x; x++)
      {
         Vec4 c=image.color3DF(x, y, z);
         c.xyz=FastBlend(avg, c).xyz;
         image.color3DF(x, y, z, c);
      }
   }
}
void TransformImage(Image &image, C MemPtr<TextParam> &params, bool clamp, C Color &background, C VecI &background_size)
{
   FREPA(params)TransformImage(image, params[i], clamp, background, background_size); // process in order
}
bool LoadImage(C Project *proj, Image &image, TextParam *image_resize, C FileParams &fp, bool srgb, bool clamp, C Color &background, C VecI &background_size, C Image *color, C TextParam *color_resize, C Image *smooth, C TextParam *smooth_resize, C Image *bump, C TextParam *bump_resize) // !! this ignores 'fp.nodes' !!
{
   if(image_resize)image_resize->del();
   if(!fp.name.is()){image.del(); return true;}
   Str  name=fp.name;
   bool lum_to_alpha=false;

   // check for special images
   if(name=="|color|" ){if(color ){color ->copy(image); if( color_resize){if(image_resize)*image_resize=* color_resize;else TransformImage(image, * color_resize, clamp, background, background_size);} goto imported;}}else // if have info about resize for source image, then if can store it in 'image_resize' then store and if not then resize now
   if(name=="|smooth|"){if(smooth){smooth->copy(image); if(smooth_resize){if(image_resize)*image_resize=*smooth_resize;else TransformImage(image, *smooth_resize, clamp, background, background_size);} goto imported;}}else // if have info about resize for source image, then if can store it in 'image_resize' then store and if not then resize now
   if(name=="|bump|"  ){if(bump  ){bump  ->copy(image); if(  bump_resize){if(image_resize)*image_resize=*  bump_resize;else TransformImage(image, *  bump_resize, clamp, background, background_size);} goto imported;}}else // if have info about resize for source image, then if can store it in 'image_resize' then store and if not then resize now
   {
      if(proj) // check for element ID
      {
         UID image_id; if(DecodeFileName(name, image_id))
         {
            name=proj->editPath(image_id); // if the filename is in UID format then it's ELM_IMAGE
            if(C Elm *image=proj->findElm(image_id))if(C ElmImage *data=image->imageData())lum_to_alpha=data->alphaLum();
         }
      }

      if(ImportImage(image, name, -1, IMAGE_SOFT, 1, true))
      {
      imported:
         image.copy(image, -1, -1, -1, srgb ? ImageTypeIncludeSRGB(image.type()) : ImageTypeExcludeSRGB(image.type())); // set desired sRGB
         if(lum_to_alpha)image.alphaFromBrightness().divRgbByAlpha();
         TransformImage(image, ConstCast(fp.params), clamp, background, background_size);
         return true;
      }
   }
   image.del(); return false;
}
bool HighPrecTransform(APPLY_MODE mode)
{
   switch(mode)
   {
      case APPLY_SET:
      case APPLY_MIN:
      case APPLY_MAX:
      case APPLY_MAX_RGB:
      case APPLY_MAX_A:
         return false;
      default: return true;
   }
}
const Palette PaletteFire[]=
{
   {0.00f, Vec(0xE8, 0x1F, 0x00)/255},
   {0.45f, Vec(0xFF, 0x8E, 0x0B)/255},
   {0.90f, Vec(0xFF, 0xD6, 0x13)/255},
   {1.00f, Vec(0xFF, 0xFF, 0xA0)/255},
};
bool LoadImages(C Project *proj, Image &image, TextParam *image_resize, C Str &src, bool srgb, bool clamp, C Color &background, C Image *color, C TextParam *color_resize, C Image *smooth, C TextParam *smooth_resize, C Image *bump, C TextParam *bump_resize)
{
   image.del(); if(image_resize)image_resize->del(); if(!src.is())return true;
   Mems<FileParams> files=FileParams::Decode(src);
   bool             ok=true, hp=false, has_data=false; // assume 'ok'=true
   Image            layer;
   TextParam        layer_resize, image_resize_final; ExtractResize(files, image_resize_final); // get what this image wants to be resized to at the end
   TextParam       *layer_resize_ptr=null; // if null then apply resize to source images (such as 'color_resize' for 'color' etc.)
   REPA(files) // go from end
   {
      FileParams &file=files[i];
      if(i && (file.name.is() || file.nodes.elms()))goto force_src_resize; // force resize if there's more than one file name specified
      REPA(file.params)if(SizeDependentTransform(file.params[i]))goto force_src_resize; // if there's at least one size dependent transform anywhere then always apply
   }
   layer_resize_ptr=&layer_resize; // if didn't meet any conditions above then store resize in 'layer_resize' to be processed later
force_src_resize:

    REPA(files)if(C TextParam *p=files[i].findParam("mode"))if(p->value!="set" && p->value!="skip" && p->value!="ignore"){hp=true; break;} // if there's at least one apply mode that's not "set" then use high precision
   FREPA(files) // process in order
   {
      FileParams &file=files[i];
      flt        alpha=1; {C TextParam *p=file.findParam("opacity"); if(!p)p=file.findParam("alpha"); if(p)alpha=p->asFlt();}
      if(alpha!=0)
      {
         APPLY_MODE mode=APPLY_SET; if(C TextParam *p=file.findParam("mode"))
         {
            if(p->value=="blend"                                                                  )mode=APPLY_BLEND;else
            if(p->value=="blendSimple"                                                            )mode=APPLY_BLEND_SIMPLE;else
            if(p->value=="merge" || p->value=="blendPremultiplied" || p->value=="premultipliedBlend")mode=APPLY_MERGE;else
            if(p->value=="mergeSimple"                                                            )mode=APPLY_MERGE_SIMPLE;else
            if(p->value=="mul"                                                                    )mode=APPLY_MUL;else
            if(p->value=="mulRGB"                                                                 )mode=APPLY_MUL_RGB;else
            if(p->value=="mulRGBblend"                                                            )mode=APPLY_MUL_RGB_BLEND;else
            if(p->value=="mulRGBS"                                                                )mode=APPLY_MUL_RGB_SAT;else
            if(p->value=="mulRGBIS"                                                               )mode=APPLY_MUL_RGB_INV_SAT;else
            if(p->value=="mulRGBLin"                                                              )mode=APPLY_MUL_RGB_LIN;else
            if(p->value=="mulA"                                                                   )mode=APPLY_MUL_A;else
            if(p->value=="mulLum"                                                                 )mode=APPLY_MUL_LUM;else
            if(p->value=="mulSat"                                                                 )mode=APPLY_MUL_SAT;else
            if(p->value=="mulSatPhoto"                                                            )mode=APPLY_MUL_SAT_PHOTO;else
            if(p->value=="div"                                                                    )mode=APPLY_DIV;else
            if(p->value=="divRGB"                                                                 )mode=APPLY_DIV_RGB;else
            if(p->value=="add"                                                                    )mode=APPLY_ADD;else
            if(p->value=="addRGB"                                                                 )mode=APPLY_ADD_RGB;else
            if(p->value=="addLum"                                                                 )mode=APPLY_ADD_LUM;else
            if(p->value=="mulInvLum"                                                              )mode=APPLY_MUL_INV_LUM;else
            if(p->value=="mulInvLumAddRGB"                                                        )mode=APPLY_MUL_INV_LUM_ADD_RGB;else
            if(p->value=="addSat"                                                                 )mode=APPLY_ADD_SAT;else
            if(p->value=="addSatPhoto"                                                            )mode=APPLY_ADD_SAT_PHOTO;else
            if(p->value=="addHue"                                                                 )mode=APPLY_ADD_HUE;else
            if(p->value=="addHuePhoto"                                                            )mode=APPLY_ADD_HUE_PHOTO;else
            if(p->value=="setAfromRGB"                                                            )mode=APPLY_SET_A_FROM_RGB;else
            if(p->value=="setHue"                                                                 )mode=APPLY_SET_HUE;else
            if(p->value=="setHuePhoto"                                                            )mode=APPLY_SET_HUE_PHOTO;else
            if(p->value=="setLum"                                                                 )mode=APPLY_SET_LUM;else
            if(p->value=="setLumPhoto"                                                            )mode=APPLY_SET_LUM_PHOTO;else
            if(p->value=="sub"                                                                    )mode=APPLY_SUB;else
            if(p->value=="subRGB"                                                                 )mode=APPLY_SUB_RGB;else
            if(p->value=="gamma"                                                                  )mode=APPLY_GAMMA;else
            if(p->value=="gammaLum"                                                               )mode=APPLY_GAMMA_LUM;else
            if(p->value=="brightness"                                                             )mode=APPLY_BRIGHTNESS;else
            if(p->value=="brightnessLum"                                                          )mode=APPLY_BRIGHTNESS_LUM;else
            if(p->value=="avg" || p->value=="average"                                              )mode=APPLY_AVG;else
            if(p->value=="min"                                                                    )mode=APPLY_MIN;else
            if(p->value=="max"                                                                    )mode=APPLY_MAX;else
            if(p->value=="maxRGB"                                                                 )mode=APPLY_MAX_RGB;else
            if(p->value=="maxA" || p->value=="maxAlpha"                                            )mode=APPLY_MAX_A;else
            if(p->value=="maxLum"                                                                 )mode=APPLY_MAX_LUM;else
            if(p->value=="maskMul"                                                                )mode=APPLY_MASK_MUL;else
            if(p->value=="maskAdd"                                                                )mode=APPLY_MASK_ADD;else
            if(p->value=="metal"                                                                  )mode=APPLY_METAL;else
            if(p->value=="scale"                                                                  )mode=APPLY_SCALE;else
            if(p->value=="scaleXY"                                                                )mode=APPLY_SCALE_XY;else
            if(p->value=="scaleAngle"                                                             )mode=APPLY_SCALE_ANGLE;else
            if(p->value=="fire"                                                                   )mode=APPLY_FIRE;else
            if(p->value=="skip" || p->value=="ignore"                                              )mode=APPLY_SKIP;
         }
         if(mode!=APPLY_SKIP)
         {
            bool layer_ok;
            if(file.nodes.elms())
            {
                  layer_ok=LoadImages(proj, layer, layer_resize_ptr, FileParams::Encode(file.nodes), srgb, clamp, background, color, color_resize, smooth, smooth_resize, bump, bump_resize);
               if(layer_ok)TransformImage(layer, file.params, clamp, background, image.size3());
            }else
            {
               layer_ok=LoadImage(proj, layer, layer_resize_ptr, file, srgb, clamp, background, image.size3(), color, color_resize, smooth, smooth_resize, bump, bump_resize);
            }
            if(layer_ok)
            {
               if(layer_resize.name.is() && !image_resize_final.name.is())Swap(layer_resize, image_resize_final); // if have info about source resize and final resize was not specified, then use source resize as final
               VecI2 pos=0; {C TextParam *p=file.findParam("position"); if(!p)p=file.findParam("pos"); if(p)pos=p->asVecI2();}
               bool  alpha_1=Equal(alpha, 1),
                 simple_set=(mode==APPLY_SET && alpha_1);
               if(!has_data && pos.allZero() && simple_set && layer.is())Swap(layer, image);else // if this is the first image, then just swap it as main
               {
                  VecI2 size=layer.size()+pos;
                  if(size.x>image.w() || size.y>image.h()) // make room for 'layer', do this even if 'layer' doesn't exist, because we may have 'pos' specified
                     Crop(image, 0, 0, Max(image.w(), size.x), Max(image.h(), size.y), background, hp || layer.highPrecision());
                  if(layer.is())
                  {
                     // put 'layer' onto image

                     Vec  mask[4];
                     bool mono;
                     switch(mode)
                     {
                        case APPLY_MASK_MUL:
                        case APPLY_MASK_ADD:
                        {
                           if(C TextParam *p=file.findParam("maskRed"  ))mask[0]=TextVecEx(p->asText());else mask[0]=1;
                           if(C TextParam *p=file.findParam("maskGreen"))mask[1]=TextVecEx(p->asText());else mask[1]=1;
                           if(C TextParam *p=file.findParam("maskBlue" ))mask[2]=TextVecEx(p->asText());else mask[2]=1;
                           if(C TextParam *p=file.findParam("maskAlpha"))mask[3]=TextVecEx(p->asText());else mask[3]=1;
                        }break;

                        case APPLY_SCALE      :
                        case APPLY_SCALE_XY   :
                        case APPLY_SCALE_ANGLE:
                           mono=(image.typeChannels()<=1 || image.monochromatic()); break;
                     }

                   C ImageTypeInfo &layer_ti=layer.typeInfo();
                     bool expand_r_gb=(layer_ti.r && !layer_ti.g && !layer_ti.b); // if have R but no GB then expand R into GB
                     AdjustImage(image, layer_ti.g>0 || layer_ti.b>0, layer_ti.a>0 || mode==APPLY_SET_A_FROM_RGB, layer.highPrecision() || HighPrecTransform(mode));
                     REPD(y, layer.h())
                     REPD(x, layer.w())
                     {
                        Vec4 l=layer.colorF(x, y);
                        if(expand_r_gb)l.z=l.y=l.x;
                        if(simple_set)
                        {
                           image.colorF(x+pos.x, y+pos.y, l);
                        }else
                        {
                           Vec4 base=image.colorF(x+pos.x, y+pos.y), c;
                           switch(mode)
                           {
                              default                       : c=l; break; // APPLY_SET
                              case APPLY_BLEND              : c=         Blend(base, l); break;
                              case APPLY_BLEND_SIMPLE       : c=     FastBlend(base, l); break;
                              case APPLY_MERGE              : c=    MergeBlend(base, l); break;
                              case APPLY_MERGE_SIMPLE       : c=FastMergeBlend(base, l); break;
                              case APPLY_MUL                : c=base*l; break;
                              case APPLY_MUL_RGB            : c.set(base.xyz*l.xyz, base.w); break;
                              case APPLY_MUL_RGB_BLEND      : c.set(base.xyz*Lerp(VecOne, l.xyz, l.w), base.w); break;
                              case APPLY_MUL_RGB_LIN        : c.set(LinearToSRGB(SRGBToLinear(base.xyz)*l.xyz), base.w); break; // this treats 'l' as already linear
                              case APPLY_MUL_A              : c.set(base.xyz, base.w*l.w); break;
                              case APPLY_SET_A_FROM_RGB     : c.set(base.xyz, l.xyz.max()); break;
                              case APPLY_MUL_LUM            : c.set(base.xyz*l.xyz.max(), base.w); break;
                              case APPLY_MUL_INV_LUM        : c.set(base.xyz*Sat(1-l.xyz.max())      , base.w); break;
                              case APPLY_MUL_INV_LUM_ADD_RGB: c.set(base.xyz*Sat(1-l.xyz.max())+l.xyz, base.w); break;
                              case APPLY_MUL_SAT            : c.xyz=RgbToHsb(base.xyz); c.y*=l.xyz.max(); c.set(HsbToRgb(c.xyz), base.w); break;
                              case APPLY_ADD_SAT            : c.xyz=RgbToHsb(base.xyz); c.y+=l.xyz.max(); c.set(HsbToRgb(c.xyz), base.w); break;
                              case APPLY_DIV                : c=base/l; break;
                              case APPLY_DIV_RGB            : c.set(base.xyz/l.xyz, base.w); break;
                              case APPLY_ADD                : c=base+l; break;
                              case APPLY_ADD_RGB            : c.set(base.xyz+l.xyz, base.w); break;
                              case APPLY_ADD_LUM            : {flt old_lum=base.xyz.max(), new_lum=old_lum+l.xyz.max(); if(old_lum>0)c.xyz=base.xyz*(new_lum/old_lum);else c.xyz=new_lum; c.w=base.w;} break;
                              case APPLY_SUB                : c=base-l; break;
                              case APPLY_SUB_RGB            : c.set(base.xyz-l.xyz, base.w); break;
                              case APPLY_GAMMA              : {flt gamma=l.xyz.max(); c.set(PowMax(base.x, gamma), PowMax(base.y, gamma), PowMax(base.z, gamma), base.w);} break;
                              case APPLY_GAMMA_LUM          : {flt gamma=l.xyz.max(); if(flt lum=base.xyz.max())c.xyz=base.xyz*(PowMax(lum, gamma)/lum);else c.xyz=base.xyz; c.w=base.w;} break;
                              case APPLY_AVG                : c=Avg(base, l); break;
                              case APPLY_MIN                : c=Min(base, l); break;
                              case APPLY_MAX                : c=Max(base, l); break;
                              case APPLY_MAX_RGB            : c.set(Max(base.xyz, l.xyz), base.w); break;
                              case APPLY_MAX_A              : c.set(base.xyz, Max(base.w, l.w)); break;
                              case APPLY_METAL              : {flt metal=l.xyz.max(); c.set(Lerp(base.xyz, l.xyz, metal), base.w);} break; // this applies metal map onto diffuse map (by lerping from diffuse to metal based on metal intensity)

                              case APPLY_ADD_HUE:
                              case APPLY_ADD_HUE_PHOTO:
                              {
                                 flt  hue  =l.xyz.max();
                                 bool photo=(mode==APPLY_ADD_HUE_PHOTO);
                                 c=base;
                               //flt  lin_lum; if(photo)lin_lum=LinearLumOfSRGBColor(c.xyz);
                                 flt      lum; if(photo)    lum=  SRGBLumOfSRGBColor(c.xyz);
                                 c.xyz=RgbToHsb(c.xyz);
                                 c.x +=hue;
                                 c.xyz=HsbToRgb(c.xyz);
                                 if(photo)
                                 {
                                  //c.xyz=SRGBToLinear(c.xyz); if(flt cur_lin_lum=LinearLumOfLinearColor(c.xyz))c.xyz*=lin_lum/cur_lin_lum; c.xyz=LinearToSRGB(c.xyz);
                                                               if(flt cur_lum    =  SRGBLumOfSRGBColor  (c.xyz))c.xyz*=    lum/cur_lum    ; // prefer multiplications in sRGB space, as linear mul may change perceptual contrast and saturation
                                 }
                              }break;

                              case APPLY_SET_HUE:
                              case APPLY_SET_HUE_PHOTO:
                              {
                                 flt  hue  =l.xyz.max();
                                 bool photo=(mode==APPLY_SET_HUE_PHOTO);
                                 c=base;
                               //flt  lin_lum; if(photo)lin_lum=LinearLumOfSRGBColor(c.xyz);
                                 flt      lum; if(photo)    lum=  SRGBLumOfSRGBColor(c.xyz);
                                 c.xyz=RgbToHsb(c.xyz);
                                 c.x  =hue;
                                 c.xyz=HsbToRgb(c.xyz);
                                 if(photo)
                                 {
                                  //c.xyz=SRGBToLinear(c.xyz); if(flt cur_lin_lum=LinearLumOfLinearColor(c.xyz))c.xyz*=lin_lum/cur_lin_lum; c.xyz=LinearToSRGB(c.xyz);
                                                               if(flt cur_lum    =  SRGBLumOfSRGBColor  (c.xyz))c.xyz*=    lum/cur_lum    ; // prefer multiplications in sRGB space, as linear mul may change perceptual contrast and saturation
                                 }
                              }break;

                              case APPLY_SET_LUM:
                              {
                               //c.xyz=SRGBToLinear(base.xyz); flt lum=SRGBToLinear(l.xyz).max(); if(flt cur_lin_lum=   c.xyz.max())c.xyz*=          lum/cur_lin_lum;else c.xyz=lum; c.xyz=LinearToSRGB(c.xyz);
                                                               flt lum=             l.xyz .max(); if(flt cur_lum    =base.xyz.max())c.xyz =base.xyz*(lum/cur_lum)   ;else c.xyz=lum; // prefer multiplications in sRGB space, as linear mul may change perceptual contrast and saturation
                                 c.w=base.w;
                              }break;

                              case APPLY_SET_LUM_PHOTO:
                              {
                               //c.xyz=SRGBToLinear(base.xyz); flt lum=LinearLumOfSRGBColor(l.xyz); if(flt cur_lin_lum=LinearLumOfLinearColor(   c.xyz))c.xyz*=          lum/cur_lin_lum ;else c.xyz=lum; c.xyz=LinearToSRGB(c.xyz);
                                                               flt lum=  SRGBLumOfSRGBColor(l.xyz); if(flt cur_lum    =  SRGBLumOfSRGBColor  (base.xyz))c.xyz =base.xyz*(lum/cur_lum    );else c.xyz=lum; // prefer multiplications in sRGB space, as linear mul may change perceptual contrast and saturation
                                 c.w=base.w;
                              }break;

                              case APPLY_MUL_RGB_SAT:
                              {
                                 flt sat=RgbToHsb(base.xyz).y;
                                 c.x=Lerp(base.x, base.x*l.x, sat); // red
                                 c.y=Lerp(base.y, base.y*l.y, sat); // green
                                 c.z=Lerp(base.z, base.z*l.z, sat); // blue
                                 c.w=base.w;
                              }break;

                              case APPLY_MUL_RGB_INV_SAT:
                              {
                                 flt sat=RgbToHsb(base.xyz).y;
                                 c.x=Lerp(base.x*l.x, base.x, sat); // red
                                 c.y=Lerp(base.y*l.y, base.y, sat); // green
                                 c.z=Lerp(base.z*l.z, base.z, sat); // blue
                                 c.w=base.w;
                              }break;

                              case APPLY_MAX_LUM:
                              {
                                 flt base_lum=base.xyz.max(), layer_lum=l.xyz.max();
                                 if(layer_lum>base_lum)
                                 {
                                    if(base_lum)c.xyz=base.xyz*(layer_lum/base_lum);
                                    else        c.xyz=layer_lum;
                                 }else c.xyz=base.xyz;
                                       c.w  =base.w;
                              }break;

                              case APPLY_MASK_ADD:
                              {
                                 c=base;
                                 c.xyz+=mask[0]*l.x;
                                 c.xyz+=mask[1]*l.y;
                                 c.xyz+=mask[2]*l.z;
                                 c.xyz+=mask[3]*l.w;
                              }break;

                              case APPLY_MASK_MUL:
                              {
                                 c=base;
                                 c.xyz*=Lerp(VecOne, mask[0], l.x);
                                 c.xyz*=Lerp(VecOne, mask[1], l.y);
                                 c.xyz*=Lerp(VecOne, mask[2], l.z);
                                 c.xyz*=Lerp(VecOne, mask[3], l.w);
                              }break;

                              case APPLY_MUL_SAT_PHOTO: 
                              {
                                 flt lum=SRGBLumOfSRGBColor(base.xyz);

                                 c.xyz=RgbToHsb(base.xyz);
                                 c.w=base.w;
                                 c.y*=l.xyz.max();
                                 c.xyz=HsbToRgb(c.xyz);

                                 if(flt cur_lum=SRGBLumOfSRGBColor(c.xyz))c.xyz*=lum/cur_lum; // prefer multiplications in sRGB space, as linear mul may change perceptual contrast and saturation
                              }break;

                              case APPLY_ADD_SAT_PHOTO: 
                              {
                                 flt lum=SRGBLumOfSRGBColor(base.xyz);

                                 c.xyz=RgbToHsb(base.xyz);
                                 c.w=base.w;
                                 c.y+=l.xyz.max();
                                 c.xyz=HsbToRgb(c.xyz);

                                 if(flt cur_lum=SRGBLumOfSRGBColor(c.xyz))c.xyz*=lum/cur_lum; // prefer multiplications in sRGB space, as linear mul may change perceptual contrast and saturation
                              }break;

                              case APPLY_BRIGHTNESS:
                              {
                                 c=base;
                                 ApplyBrightness(c.x, l.x);
                                 ApplyBrightness(c.y, l.y);
                                 ApplyBrightness(c.z, l.z);
                              }break;

                              case APPLY_BRIGHTNESS_LUM:
                              {
                                 c=base;
                                 if(flt bright=l.xyz.max())
                                 {
                                    flt old_lum=c.xyz.max(); if(old_lum>0 && old_lum<1)
                                    {
                                       flt new_lum=_ApplyBrightness(old_lum, bright);
                                       c.xyz*=new_lum/old_lum;
                                    }
                                 }
                              }break;
                              
                              case APPLY_SCALE:
                              {
                                 flt scale=l.xyz.max();
                                 c.w=base.w;
                                 if( mono )c.xyz=(base.xyz-0.5f)*scale+0.5f;else
                                 if(!scale)c.xyz.set(0.5f, 0.5f, 1);else
                                 {
                                    Vec &n=c.xyz;
                                    n=base.xyz*2-1;
                                    n.normalize();
                                    n.z/=scale;
                                    n.normalize();
                                    n=n*0.5f+0.5f;
                                 }
                              }break;

                              case APPLY_SCALE_XY:
                              {
                                 flt scale=l.xyz.max();
                                 c.w=base.w;
                                 if( mono )c.xyz=(base.xyz-0.5f)*scale+0.5f;else
                                 if(!scale)c.xyz.set(0.5f, 0.5f, 1);else
                                 {
                                    Vec &n=c.xyz;
                                    n=base.xyz*2-1;
                                    n.normalize();
                                    n.xy*=scale;
                                    n.normalize();
                                    n=n*0.5f+0.5f;
                                 }
                              }break;

                              case APPLY_SCALE_ANGLE:
                              {
                                 flt scale=l.xyz.max();
                                 c.w=base.w;
                                 if( mono )c.xyz=(base.xyz-0.5f)*scale+0.5f;else
                                 if(!scale)c.xyz.set(0.5f, 0.5f, 1);else
                                 {
                                    Vec &n=c.xyz;
                                    n=base.xyz*2-1;
                                    n.normalize();
                                    Vec2 p(n.z, n.xy.length());
                                    flt angle=Angle(p);
                                    angle*=scale;
                                    Clamp(angle, -PI_2, PI_2);
                                    CosSin(p.x, p.y, angle);
                                    n.z=p.x;
                                    n.xy.setLength(p.y);
                                    n=n*0.5f+0.5f;
                                 }
                              }break;

                              case APPLY_FIRE:
                              {
                                 c=base;
                                 if(flt alpha=l.xyz.max())
                                 {
                                    flt lum=base.xyz.max();
                                    Vec pal;
                                    for(int i=1; i<Elms(PaletteFire); i++)
                                    {
                                     C Palette &next=PaletteFire[i];
                                       if(lum<=next.lum)
                                       {
                                        C Palette &prev=PaletteFire[i-1];
                                          flt step=LerpRS(prev.lum, next.lum, lum );
                                               pal=Lerp  (prev.col, next.col, step);
                                          goto has_pal;
                                       }
                                    }
                                    pal=PaletteFire[Elms(PaletteFire)-1].col; // get last one
                                 has_pal:
                                    c.xyz=Lerp(c.xyz, pal, alpha);
                                 }
                              }break;
                           }
                           image.colorF(x+pos.x, y+pos.y, alpha_1 ? c : Lerp(base, c, alpha));
                        }
                     }
                  }else
                  if(!(file.name.is() || file.nodes.elms()))TransformImage(image, file.params, clamp, background, image.size3()); // if this 'layer' is empty and no file name was specified, then apply params to the entire 'image', so we can process entire atlas
               }
               has_data=true;
            }else ok=false;
         }
      }
   }

   // store information about image size, if can't store then just resize it
   if(image_resize)*image_resize=image_resize_final;else TransformImage(image, image_resize_final, clamp, background, image.size3());

   return ok;
}
/******************************************************************************/
// MATERIAL
/******************************************************************************/
// TEXT
/******************************************************************************/
bool ValidChar(char c) {return c>=32 && c<128;}
bool ValidText(C Str &text, int min, int max)
{
   if(text.length()>=min && (text.length()<=max || max<0))
   {
      REPA(text)if(!ValidChar(text[i]))return false;
      return true;
   }
   return false;
}
bool ValidFileName(C Str &name) {return name.length()>=1 && CleanFileName(name)==name;}
bool ValidFileNameForUpload(C Str &name, int max)
{
   if(ValidFileName(name) && ValidText(name, 1, max))
   {
      REPA(name)if(name[i]=='@' || name[i]=='~')return false; // because of "@new" and "~" is special char on unix ?
      return true;
   }
   return false;
}
bool ValidPass(C Str &pass) {return ValidText(pass, 4, 16) && !HasUnicode(pass);}
bool ValidEnum(C Str &name)
{
   if(!name.is())return false;
   FREPA(name)
   {
      uint flag=CharFlag(name[i]);
      bool ok=((flag&(CHARF_ALPHA|CHARF_UNDER)) || (i && (flag&CHARF_DIG))); // digit cannot be used as first character
      if(! ok)return false;
   }
   return true;
}
bool ValidSupport(C Str &support)
{
   return !support.is() || ValidEmail(support) || ValidURL(support);
}
bool ValidVideo(C Str &video)
{
   return !video.is() || (ValidURL(video) && (StartsPath(video, "https://www.youtube.com/embed")));
}
Str YouTubeEmbedToFull(C Str &video) {return Replace(video, "/embed/", "/watch?v=");}
Str YouTubeFullToEmbed(C Str &video) {return Replace(video, "/watch?v=", "/embed/");}
UID PassToMD5(C Str &pass) {return MD5Mem((Str8)CaseDown(pass), pass.length());}
Str NameToEnum(C Str &name)
{
   Str e; FREPA(name)
   {
      char c=name[i];
      if(c==' ')c='_';
      if(c>='0' && c<='9'
      || c>='a' && c<='z'
      || c>='A' && c<='Z'
      || c=='_')e+=c;
   }
   return e;
}
Str TimeAgo(C DateTime &date) {DateTime now; now.getUTC(); return TimeText(now-date, TIME_NAME_MED)+" ago";}

cchar8* CountS(int n) {return (n==1) ? null : "s";}

Str Plural(Str name) // convert to plural name
{
   bool case_up=Equal(CaseUp(name), name, true);
   char last   =CaseDown(name.last());
   if(name=="child"                      )name+="ren"               ;else // child  -> children
   if(name=="potato"                     )name+="es"                ;else // potato -> potatoes
   if(name=="hero"                       )name+="es"                ;else // hero   -> heroes
   if(name=="mouse"                      )name.remove(1, 4)+="ice"  ;else // mouse  -> mice
   if(name=="man"                        )name.remove(1, 2)+="en"   ;else // man    -> men
   if(name=="woman"                      )name.remove(3, 2)+="en"   ;else // woman  -> women
   if(name=="goose"                      )name.remove(1, 4)+="eese" ;else // goose  -> geese
   if(name=="person"                     )name.remove(1, 5)+="eople";else // person -> people
   if(last=='y'                          )name.removeLast()+="ies"  ;else // body   -> bodies
   if(last=='x' || last=='h' || last=='s')name+="es"                ;else // box    -> boxes, mesh -> meshes, bus -> buses
   if(last=='f')
   {
      if(name!="dwarf" && name!="roof")name.removeLast()+="ves"; // leaf -> leaves, elf -> elves (dwarf -> dwarfs, roof -> roofs)
   }else
   if(Ends(name, "fe"))name.removeLast().removeLast()+="ves";else // life -> lives, knife -> knives
      name+='s';
   return case_up ? CaseUp(name) : name;
}
/******************************************************************************/
int Occurrences(C Str &s, char c)
{
   int o=0; REPA(s)if(s[i]==c)o++; return o;
}
/******************************************************************************/
                           // Contains(t, ',') ? TextVecI2(t) : VecI2(TextInt(t));
                           // Contains(t, ',') ? TextVec2 (t) : Vec2 (TextFlt(t));
                           // Contains(t, ',') ? TextVec  (t) : Vec  (TextFlt(t));
Str TextVecI2Ex(C VecI2&v) // try to keep as one value if XY are the same
{
   Str s; s=v.x; if(v.y!=v.x)s+=S+","+v.y; 
   return s;
}
Str TextVec2Ex(C Vec2 &v) // try to keep as one value if XY are the same
{
   Str s; s=v.x; if(v.y!=v.x)s+=S+","+v.y; 
   return s;
}
Str TextVecEx(C Vec &v, int precision) // try to keep as one value if XYZ are the same
{
   return (Equal(v.x, v.y) && Equal(v.x, v.z)) ? TextReal(v.x, precision) : v.asText(precision);
}
bool TextVecVecEx(cchar *t, Vec &a, Vec &b)
{
   Memc<Str> c; Split(c, t, ','); switch(c.elms())
   {
      case 2: a=TextFlt(c[0]); b=TextFlt(c[1]); return true;
      case 6: a.set(TextFlt(c[0]), TextFlt(c[1]), TextFlt(c[2])); b.set(TextFlt(c[3]), TextFlt(c[4]), TextFlt(c[5])); return true;
   }
   return false;
}
Str TextVecVecEx(C Vec &a, C Vec &b, int precision)
{
   return (Equal(a.x, a.y) && Equal(a.x, a.z)
        && Equal(b.x, b.y) && Equal(b.x, b.z)) ? TextReal(a.x, precision)+", "+TextReal(b.x, precision)
                                               : a.asText(     precision)+", "+b.asText(     precision);
}
/******************************************************************************/
Str RelativePath  (C Str &path) {return SkipStartPath(path, GetPath(App.exe()));}
Str EditToGamePath(  Str  path)
{
   Str out;
   for(path.tailSlash(false); ; )
   {
      Str base=GetBase(path); path=GetPath(path);
      if(!base.is()   )return path.tailSlash(true)+out; // needed for "/xxx" unix style paths
      if( base=="Edit")return path.tailSlash(true)+"Game\\"+out; // replace "edit" with "game"
      out=(out.is() ? base.tailSlash(true)+out : base);
   }
}
/******************************************************************************/
cchar8 *FormatSuffixes[]=
{
   "_BC1"      , // for DX11 UWP, Web
   "_BC3"      , // for DX11 UWP, Web
   "_ETC2_R"   , // for Android, iOS
   "_ETC2_RG"  , // for Android, iOS
   "_ETC2_RGB" , // for Android, iOS
   "_ETC2_RGBA", // for Android, iOS
   "_PVRTC1_2" , // for iOS
   "_PVRTC1_4" , // for iOS
   "_SIMPLE"   , // used for simplified Materials
};  int FormatSuffixElms=Elms(FormatSuffixes);
cchar8* FormatSuffixSimple() {return "_SIMPLE";}
cchar8* FormatSuffix(IMAGE_TYPE type)
{
   switch(type)
   {
      default: return null;

      // we can return the same suffix for non-sRGB and sRGB, unsigned and signed, because depending on sRGB/signed they will already have different hash
      case IMAGE_BC1: case IMAGE_BC1_SRGB: return "_BC1";
      case IMAGE_BC3: case IMAGE_BC3_SRGB: return "_BC3";

      case IMAGE_ETC2_R   : case IMAGE_ETC2_R_SIGN   : return "_ETC2_R";
      case IMAGE_ETC2_RG  : case IMAGE_ETC2_RG_SIGN  : return "_ETC2_RG";
      case IMAGE_ETC2_RGB : case IMAGE_ETC2_RGB_SRGB : return "_ETC2_RGB";
      case IMAGE_ETC2_RGBA: case IMAGE_ETC2_RGBA_SRGB: return "_ETC2_RGBA";

      case IMAGE_PVRTC1_2: case IMAGE_PVRTC1_2_SRGB: return "_PVRTC1_2";
      case IMAGE_PVRTC1_4: case IMAGE_PVRTC1_4_SRGB: return "_PVRTC1_4";
   }
}
Str8 ImageDownSizeSuffix(int size)
{
   if(size>0 && size<INT_MAX)return S+"_"+size;
   return S;
}
/******************************************************************************/
TextParam* FindTransform(MemPtr<FileParams> files, C Str &name) // this ignores partial(non full size) transforms
{
   REPA(files) // go from end
   {
      FileParams &file=files[i];
      if(i && (file.name.is() || file.nodes.elms()))break; // stop on first file that has name (but allow the first which means there's only one file) so we don't process transforms for only 1 of multiple images
      REPA(file.params) // go from end
      {
         TextParam &p=file.params[i]; if(p.name==name && !PartialTransform(p))return &p;
      }
   }
   return null;
}
void DelTransform(MemPtr<FileParams> files, C Str &name) // this ignores partial(non full size) transforms 
{
   REPA(files) // go from end
   {
      FileParams &file=files[i];
      if(i && (file.name.is() || file.nodes.elms()))break; // stop on first file that has name (but allow the first which means there's only one file) so we don't process transforms for only 1 of multiple images
      REPAD(pi, file.params) // go from end
      {
         TextParam &p=file.params[pi]; if(p.name==name && !PartialTransform(p))
         {
                    file.params.remove(pi, true);
            if(!file.is())files.remove( i, true); // if nothing left then remove it
            return;
         }
      }
   }
}
void SetTransform(MemPtr<FileParams> files, C Str &name, C Str &value) // this ignores partial(non full size) transforms 
{
   if(files.elms()) // set only if we have something (to ignore setting for completely empty)
   {
      if(files.elms()>1 && (files.last().name.is() || files.last().nodes.elms()))files.New(); // if we have more than one image, then we need to make sure that we add the parameter not to one of the images, but as last file that has no name specified to set transform for everything
      TextParam *p;
      REPA(files) // go from end
      {
         FileParams &file=files[i];
         if(i && (file.name.is() || file.nodes.elms()))break; // stop on first file that has name (but allow the first which means there's only one file) so we don't process transforms for only 1 of multiple images
         REPA(file.params) // go from end
         {
            p=&file.params[i]; if(p->name==name && !PartialTransform(*p))goto found;
         }
      }
      p=&files.last().params.New().setName(name); // if didn't found then create a new one
   found:
      p->setValue(value);
   }
}
void AddTransform(MemPtr<FileParams> files, C Str &name, C Str &value)
{
   if(files.elms()) // set only if we have something (to ignore setting for completely empty)
   {
      if(files.elms()>1 && (files.last().name.is() || files.last().nodes.elms()))files.New(); // if we have more than one image, then we need to make sure that we add the parameter not to one of the images, but as last file that has no name specified to set transform for everything
      files.last().params.New().set(name, value);
   }
}
void SetResizeTransform(MemPtr<FileParams> files, C Str &name, C Str &value) // this ignores partial(non full size) transforms 
{
   if(files.elms()) // set only if we have something (to ignore setting for completely empty)
   {
      if(files.elms()>1 && (files.last().name.is() || files.last().nodes.elms()))files.New(); // if we have more than one image, then we need to make sure that we add the parameter not to one of the images, but as last file that has no name specified to set transform for everything
      TextParam *p;
      REPA(files) // go from end
      {
         FileParams &file=files[i];
         if(i && (file.name.is() || file.nodes.elms()))break; // stop on first file that has name (but allow the first which means there's only one file) so we don't process transforms for only 1 of multiple images
         REPA(file.params) // go from end
         {
            p=&file.params[i];
            if(p->name==name && !PartialTransform(*p))goto found;
            if(SizeDependentTransform(*p))goto need_new; // if encountered a size-dependent transform then it means we can't change any resize transforms before that, but need to create a new one
         }
      }
   need_new:
      p=&files.last().params.New().setName(name); // if didn't found then create a new one
   found:
      p->setValue(value);
   }
}
void SetTransform(Str &file, C Str &name, C Str &value) // this tries first to adjust existing transform
{
   Mems<FileParams> files=FileParams::Decode(file);
   SetTransform(files, name, value);
   file=FileParams::Encode(files);
}
void AddTransform(Str &file, C Str &name, C Str &value)
{
   Mems<FileParams> files=FileParams::Decode(file);
   AddTransform(files, name, value);
   file=FileParams::Encode(files);
}
/******************************************************************************/
SOUND_CODEC TextSoundCodec(C Str &t)
{
   if(t=="raw"
   || t=="wav"
   || t=="uncompressed")return SOUND_WAV;
   if(t=="vorbis"      )return SOUND_SND_VORBIS;
   if(t=="opus"        )return SOUND_SND_OPUS;
                        return SOUND_NONE;
}
/******************************************************************************/
// MESH
/******************************************************************************/
int       VisibleVtxs      (C MeshLod &mesh) {int       num =        0; REPA(mesh)if(!(mesh.parts[i].part_flag&MSHP_HIDDEN))num +=mesh.parts[i].vtxs           (); return num ;}
int       VisibleTris      (C MeshLod &mesh) {int       num =        0; REPA(mesh)if(!(mesh.parts[i].part_flag&MSHP_HIDDEN))num +=mesh.parts[i].tris           (); return num ;}
int       VisibleTrisTotal (C MeshLod &mesh) {int       num =        0; REPA(mesh)if(!(mesh.parts[i].part_flag&MSHP_HIDDEN))num +=mesh.parts[i].trisTotal      (); return num ;}
int       VisibleQuads     (C MeshLod &mesh) {int       num =        0; REPA(mesh)if(!(mesh.parts[i].part_flag&MSHP_HIDDEN))num +=mesh.parts[i].quads          (); return num ;}
int       VisibleSize      (C MeshLod &mesh) {int       size=        0; REPA(mesh)if(!(mesh.parts[i].part_flag&MSHP_HIDDEN))size+=mesh.parts[i].render.memUsage(); return size;}
MESH_FLAG VisibleFlag      (C MeshLod &mesh) {MESH_FLAG flag=MESH_NONE; REPA(mesh)if(!(mesh.parts[i].part_flag&MSHP_HIDDEN))flag|=mesh.parts[i].flag           (); return flag;}
MESH_FLAG VisibleFlag      (C Mesh    &mesh) {MESH_FLAG flag=MESH_NONE; REP (mesh.lods()                                   )flag|=VisibleFlag       (mesh.lod(i)); return flag;}
flt       VisibleLodQuality(C Mesh    &mesh, int lod_index)
{
   Clamp(lod_index, 0, mesh.lods());
 C MeshLod &base=mesh,
           &lod =mesh.lod(lod_index);
   Int      v   =VisibleVtxs     (base),
            f   =VisibleTrisTotal(base);
   return Avg(v ? flt(VisibleVtxs     (lod))/v : 1,
              f ? flt(VisibleTrisTotal(lod))/f : 1);
}
/******************************************************************************/
void KeepParams(C Mesh &src, Mesh &dest)
{
   REPAD(d, dest)
   {
      MeshPart &dest_part=dest.parts[d];
      int        src_part_i=-1;
      REPAD(s, src)
      {
       C MeshPart &src_part=src.parts[s];
         if(Equal(src_part.name, dest_part.name)) // if same name
            if(src_part_i<0 || Abs(s-d)<Abs(src_part_i-d))src_part_i=s; // if new index is closer to original index
      }
      if(InRange(src_part_i, src))
      {
       C MeshPart &src_part=src.parts[src_part_i];
         dest_part.part_flag=src_part.part_flag;
         dest_part.drawGroup(src_part.drawGroup(), src.drawGroupEnum());
         dest_part.variations(src_part.variations());
         REP(dest_part.variations())if(i)dest_part.variation(i, src_part.variation(i));
      }
   }
   dest.drawGroupEnum(src.drawGroupEnum()); // keep the same draw group enum
}
void RemovePartsAndLods(Mesh &mesh)
{
   REPD(l, mesh.lods()) // have to go from end because we're removing LODs
   {
      MeshLod &lod=mesh.lod(l);
      // remove LODs
      if(NegativeSB(lod.dist2) // negative distance (marked as disabled)
      || InRange(l+1, mesh.lods()) && (l ? lod.dist2 : 0)>=mesh.lod(l+1).dist2) // distance is higher than the next one (have to check next one and not previous one, because we need to delete those with negative distance first. Force 0 dist for #0 LOD because currently it's uneditable and assumed to be 0 however it may not be)
      {
      remove_lod:
         mesh.removeLod(l);
      }else
      {
         // remove hidden mesh parts
         REPA(lod)if(lod.parts[i].part_flag&MSHP_HIDDEN)lod.parts.remove(i);
         if(!lod.parts.elms())goto remove_lod;
      }
   }
}
void EditToGameMesh(C Mesh &edit, Mesh &game, Skeleton *skel, Enum *draw_group, C Matrix *matrix)
{
   game.create(edit, GameMeshFlagAnd);
   RemovePartsAndLods(game);
   game.joinAll(true, true, false, MeshJoinAllTestVtxFlag, -1); // disable vtx weld, because: 1) mesh isn't scaled/transformed yet 2) it would be performed only if some parts were merged 3) there are no tangents yet. Instead let's always do it manually
   if(matrix)game.transform(*matrix); // transform before welding
   game.setAutoTanBin() // calculate tangents before welding
       .weldVtx(VTX_ALL, EPS, EPS_COL8_COS, -1).skeleton(skel).drawGroupEnum(draw_group).setBox(); // use 8-bit for vtx normals because they can't handle more anyway, always recalculate box because of transform and welding
   game.setRender().delBase();
}
/******************************************************************************/
bool HasMaterial(C MeshPart &part, C MaterialPtr &material)
{
   REP(part.variations())if(part.variation(i)==material)return true;
   return false;
}
/******************************************************************************/
void CleanMesh(Mesh &mesh)
{
   REPD(l, mesh.lods())
   {
      MeshLod &lod=mesh.lod(l);
      REPA(lod)if(!lod.parts[i].is())lod.parts.remove(i, true);
   }
}
bool FixVtxNrm(MeshBase &base)
{
   bool ok=false; if(base.vtx.nrm())
   {
      MeshBase temp(base, VTX_POS|FACE_IND); temp.setNormals(); // copy to a temp mesh and set its vtx normals
      if(base.vtxs()==temp.vtxs() && temp.vtx.nrm()) // safety checks
      {
         ok=true;
         REPA(base.vtx)
         {
            Vec &nrm=base.vtx.nrm(i); if(nrm.allZero()) // if any 'base' vtx normal is zero
            {
               nrm=temp.vtx.nrm(i); if(nrm.allZero())ok=false; // copy from 'temp'
            }
         }
      }
   }
   return ok;
}
void FixMesh(Mesh &mesh)
{
   mesh.setBase(true).delRender() // create base if empty
       .material(null).variations(0) // clear any existing materials, they will be set later according to 'mtrls'
       .skeleton(null).drawGroupEnum(null) // clear 'skeleton', clear 'drawGroupEnum'
       .removeUnusedVtxs(); // remove unused vertexes

   // check if some vtx normals are wrong
   REP(mesh.lods())
   {
      MeshLod &lod=mesh.lod(i); REPA(lod)
      {
         MeshPart &part=lod.parts[i];
         MeshBase &base=part.base;
         if(base.vtx.nrm())REPA(base.vtx)if(!base.vtx.nrm(i).any()) // all zero
         {
            if(!FixVtxNrm(base)) // if didn't fix yet, then it's possible that vtx shares only co-planar faces
            {
               base.explodeVtxs();
               FixVtxNrm(base);
               if(!base.vtx.tan() || !base.vtx.bin())base.setTanBinDbl(); //if(!base.vtx.tan())base.setTangents(); if(!base.vtx.bin())base.setBinormals(); // set in case mesh doesn't have them yet, need to call before 'weldVtx'
               base.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down, do not remove degenerate faces because they're not needed because we're doing this only because of 'explodeVtxs'
            }
            break;
         }
      }
   }
}
bool SamePartInAllLods(C Mesh &mesh, int part)
{
#if 0 // checks only 1 part
   if(InRange(part, mesh.parts))
   {
      cchar8 *name=mesh.parts[part].name;
      for(Int i=mesh.lods(); --i>=1; )
      {
       C MeshLod &lod=mesh.lod(i);
         if(!InRange(part, lod) || !Equal(name, lod.parts[part].name))return false;
      }
      return true;
   }
   return false;
#else // checks all parts
   for(Int i=mesh.lods(); --i>=1; )
   {
    C MeshLod &lod=mesh.lod(i); if(lod.parts.elms()!=mesh.parts.elms())return false;
      REPA(lod.parts)if(!Equal(lod.parts[i].name, mesh.parts[i].name))return false;
   }
   return true;
#endif
}
void SetDrawGroup(Mesh &mesh, MeshLod &lod, int part, int group, Enum *draw_group_enum)
{
   if(SamePartInAllLods(mesh, part))
   {
      REP(mesh.lods())
      {
         MeshLod &lod=mesh.lod(i);
         if(InRange(part, lod))lod.parts[part].drawGroup(group, draw_group_enum);
      }
   }else
   {
      if(InRange(part, lod))lod.parts[part].drawGroup(group, draw_group_enum);
   }
}
bool DisableLQLODs(Mesh &mesh)
{
   bool changed=false, disable=false;
   int  last_tris=INT_MAX;
   FREP(mesh.lods())
   {
      MeshLod &lod=mesh.lod(i);
      if(!NegativeSB(lod.dist2)) // not disabled
      {
         int tris=VisibleTrisTotal(lod);
         if(disable || tris+128>=last_tris) // reached low tris, or there's only 128 tri difference between this and previous LOD
         {
            CHSSB(lod.dist2); // disable
            changed=true;
         }else
         {
            last_tris=tris;
            if(tris<=256)disable=true; // disable next LODs
         }
      }
   }
   return changed;
}
/******************************************************************************/
// SKELETON
/******************************************************************************/
Str BoneNeutralName(C Str &name)
{
   Str n=Replace(name, "right", CharAlpha);
       n=Replace(n   , "left" , CharAlpha);
       n.replace('r', CharBeta).replace('l', CharBeta).replace('R', CharBeta).replace('L', CharBeta);
   return n;
}
/******************************************************************************/
// OBJECT
/******************************************************************************/
// following functions are used to determine whether object should override mesh/phys
bool OverrideMeshSkel(C Mesh *mesh, C Skeleton *skel) {return (mesh && mesh->is()) || (skel && skel->is());}
bool OverridePhys    (C PhysBody *body              ) {return (body && body->is());}

int CompareObj(C Game::Area::Data::AreaObj &a, C Game::Area::Data::AreaObj &b) // this function is used for sorting object before they're saved into game area
{
   if(int c=Compare(a.mesh().id()      , b.mesh().id()      ))return c; // first compare by mesh
   if(int c=Compare(a.meshVariationID(), b.meshVariationID()))return c; // then  compare by mesh variation
   if(int c=Compare(a.matrix.pos       , b.matrix.pos       ))return c; // last  compare by position
   return 0;
}
/******************************************************************************/
// ANIMATION
/******************************************************************************/
void SetRootMoveRot(Animation &anim, C Vec *root_move, C Vec *root_rot)
{
   if(root_rot)
   {
      const int precision=4; // number of keyframes per 90 deg, can be modified, this is needed because rotation interpolation is done by interpolating axis vectors, and few samples are needed to get smooth results
      int num=(Equal(*root_rot, VecZero) ? Equal(anim.rootStart().angle(), 0) ? 0 : 1 : 1+Max(1, Round(root_rot->length()*(precision/PI_2))));
      anim.keys.orns.setNum(num);
      if(num)
      {
         OrientD orn=anim.rootStart();
         anim.keys.orns[0].time=0;
         anim.keys.orns[0].orn =orn;
         if(num>=2)
         {
            num--;
            VecD axis=*root_rot; dbl angle=axis.normalize(); MatrixD3 rot; rot.setRotate(axis, angle/num);
            for(int i=1; i<=num; i++)
            {
               orn.mul(rot, true);
               anim.keys.orns[i].time=((i==num) ? anim.length() : flt(i)/num*anim.length()); // !! have to set precise time for last keyframe to make sure root movement is calculated properly !!
               anim.keys.orns[i].orn =orn;
            }
         }
      }
   }
   if(root_move)
   {
      const int precision=10; // number of keyframes per meter, can be modified
      bool no_rot=(!root_rot || anim.keys.orns.elms()<=1);
      int  num=(Equal(*root_move, VecZero) ? Equal(anim.rootStart().pos, VecZero) ? 0 : 1 : no_rot ? 2 : 1+Max(1, Round(root_move->length()*precision)));
      anim.keys.poss.setNum(num);
      if(num)
      {
         anim.keys.poss[0].time=0;
         anim.keys.poss[0].pos =anim.rootStart().pos;
         if(num>=2)
         {
            if(no_rot)
            {
               anim.keys.poss[1].time=anim.length();
               anim.keys.poss[1].pos =anim.rootStart().pos+*root_move;
            }else
            {
               num--;
               const bool simple=false; // these are not perfect, but not bad
               VecD pos=anim.rootStart().pos, dir=*root_move/(simple ? num : num*2), // for advanced mode, make 'dir' 2x smaller, because we will process it 2 times per step
                   axis=*root_rot; dbl angle=axis.normalize(); MatrixD3 rot; rot.setRotate(axis, angle/num);
               for(int i=1; i<=num; i++)
               {
                  if(simple){dir*=rot; pos+=dir;}
                  else      {pos+=dir; dir*=rot; pos+=dir;} // much more precise
                  anim.keys.poss[i].time=((i==num) ? anim.length() : flt(i)/num*anim.length()); // !! have to set precise time for last keyframe to make sure root movement is calculated properly !!
                  anim.keys.poss[i].pos =pos;
               }
            }
         }
      }
   }
   if(root_move || root_rot)
   {
      anim.keys.setTangents(anim.loop(), anim.length());
      anim.setRootMatrix();
   }
}
bool DelEndKeys(Animation &anim) // return if any change was made
{
   bool changed=false;
   flt time=anim.length()-EPS;
   REPA(anim.bones)
   {
      bool bone_changed=false;
      AnimBone &bone=anim.bones[i];
      if(bone.poss  .elms()>=2 && bone.poss  .last().time>=time && bone.poss  .first().time<=EPS){bone.poss  .removeLast(); bone_changed=true;}
      if(bone.orns  .elms()>=2 && bone.orns  .last().time>=time && bone.orns  .first().time<=EPS){bone.orns  .removeLast(); bone_changed=true;}
      if(bone.scales.elms()>=2 && bone.scales.last().time>=time && bone.scales.first().time<=EPS){bone.scales.removeLast(); bone_changed=true;}
      if(bone_changed){bone.setTangents(anim.loop(), anim.length()); changed=true;}
   }
   //anim.setRootMatrix(); we don't change root, only bones
   return changed;
}
/******************************************************************************/
// MATH
/******************************************************************************/
 // have to work with SIGN_BIT for special case of -0
 // have to work with SIGN_BIT for special case of -0
/******************************************************************************/
int UniquePairs(int elms) {return elms*(elms-1)/2;}
/******************************************************************************/
bool Distance2D(C Vec2 &point, C Edge &edge, flt &dist, flt min_length) // calculate 2D distance between 'point' and 'edge' projected to screen, true is returned if 'edge' is at least partially visible (not behind camera), 'dist' will contain distance between point and edge
{
   Edge e=edge;
   if(Clip(e, Plane(ActiveCam.matrix.pos + D.viewFrom()*ActiveCam.matrix.z, -ActiveCam.matrix.z)))
   {
      Edge2 e2(PosToScreen(e.p[0]), PosToScreen(e.p[1]));
      if(e2.length()<min_length)return false; // if edge is too short then skip it
      dist=Dist(point, e2);
      return true;
   }
   return false;
}
int MatrixAxis(C Vec2 &screen_pos, C Matrix &matrix)
{
   int axis=-1;
   flt d, dist=0;
   if(Distance2D(screen_pos, Edge(matrix.pos, matrix.pos+matrix.x), d))if(axis<0 || d<dist){dist=d; axis=0;}
   if(Distance2D(screen_pos, Edge(matrix.pos, matrix.pos+matrix.y), d))if(axis<0 || d<dist){dist=d; axis=1;}
   if(Distance2D(screen_pos, Edge(matrix.pos, matrix.pos+matrix.z), d))if(axis<0 || d<dist){dist=d; axis=2;}
   if(dist>0.05f)axis=-1;
   return axis;
}
void MatrixAxis(Edit::Viewport4 &v4, C Matrix &matrix, int &axis, Vec *axis_vec)
{
   int editing=-1; REPA(MT)if(MT.b(i, MT.mouse(i)) && v4.getView(MT.guiObj(i))){editing=i; break;}
   if( editing<0)
   {
      axis=-1;
      if(editing< 0)REPA(MT)if(MT.mouse(i) && v4.getView(MT.guiObj(i))){editing=i; break;} // get mouse
      if(editing>=0)if(Edit::Viewport4::View *view=v4.getView(MT.guiObj(editing)))
      {
         view->setViewportCamera();
         axis=MatrixAxis(MT.pos(editing), matrix);
      }
      if(axis_vec)switch(axis)
      {
         case  0: *axis_vec=!matrix.x; break;
         case  1: *axis_vec=!matrix.y; break;
         case  2: *axis_vec=!matrix.z; break;
         default:  axis_vec->zero()   ; break;
      }
   }
}
int GetNearestAxis(C Matrix &matrix, C Vec &dir)
{
   flt dx=Abs(Dot(!matrix.x, dir)),
       dy=Abs(Dot(!matrix.y, dir)),
       dz=Abs(Dot(!matrix.z, dir));
   return MaxI(dx, dy, dz);
}
bool UniformScale(C Matrix3 &m) {return UniformScale(m.scale());}
bool UniformScale(C Vec     &s)
{
   return Equal(s.x/s.y, 1)
       && Equal(s.x/s.z, 1);
}
/******************************************************************************/
flt CamMoveScale(bool perspective    ) {return perspective ? ActiveCam.dist*Tan(D.viewFov()/2) : D.viewFov();}
Vec2   MoveScale(Edit::Viewport4::View &view) {return Vec2(D.w()*2, D.h()*2)/view.viewport.size();}

flt AlignDirToCamEx(C Vec &dir, C Vec2 &delta, C Vec &cam_x, C Vec &cam_y) {return (!Vec2(Dot(cam_x, dir), Dot(cam_y, dir)) * delta).sum();}
Vec AlignDirToCam  (C Vec &dir, C Vec2 &delta, C Vec &cam_x, C Vec &cam_y) {return !dir * AlignDirToCamEx(dir, delta, cam_x, cam_y);}
/******************************************************************************/
flt MatrixLength(C Vec &x, C Vec &y, C Vec &z, C Vec &dir) // matrix length along direction
{
   return Abs(Dot(x, dir))
         +Abs(Dot(y, dir))
         +Abs(Dot(z, dir));
}
/******************************************************************************/
void Include(RectI &rect,           C VecI2 &x) {if(rect.valid())rect|=x;else          rect=x; }
void Include(RectI &rect,           C RectI &x) {if(rect.valid())rect|=x;else          rect=x; }
void Include(Rect  &rect,           C Rect  &x) {if(rect.valid())rect|=x;else          rect=x; }
void Include(Rect  &rect, bool &is, C Vec2  &x) {if(is          )rect|=x;else{is=true; rect=x;}}
void Include(Rect  &rect, bool &is, C Rect  &x) {if(is          )rect|=x;else{is=true; rect=x;}}
void Include(Box   &box , bool &is, C Vec   &v) {if(is          )box |=v;else{is=true; box =v;}}
void Include(Box   &box , bool &is, C Box   &b) {if(is          )box |=b;else{is=true; box =b;}}
/******************************************************************************/
void DrawMatrix(C Matrix &matrix, int bold_axis)
{
   matrix.draw(); switch(bold_axis)
   {
      case 0: DrawArrow2(RED  , matrix.pos, matrix.pos+matrix.x, 0.005f); break;
      case 1: DrawArrow2(GREEN, matrix.pos, matrix.pos+matrix.y, 0.005f); break;
      case 2: DrawArrow2(BLUE , matrix.pos, matrix.pos+matrix.z, 0.005f); break;
   }
}
/******************************************************************************/
// MISC
/******************************************************************************/
void Hide(GuiObj &go) {go.hide();}
/******************************************************************************/
Rect GetRect(C Rect &rect, Memt<Rect> &rects) // !! this will modify 'rects' !!
{
   for(flt x=D.w(); ; ) // start with right side
   {
      for(flt y=D.h(); ; ) // start from top
      {
         Rect temp=rect; temp+=Vec2(x, y)-temp.ru(); // move to test position

         if(!rects.elms())return temp;

         if(temp.min.y<-D.h())break; // we're outside the screen

         bool cuts=false;
         REPA(rects)if(Cuts(temp, rects[i]))
         {
            MIN(y, rects[i].min.y-EPS);
            cuts=true;
         }
         if(!cuts)return temp;
      }

      // find the maximum out of rectangles min.x and remove those rectangles
      int found=-1; REPA(rects)if(found==-1 || rects[i].min.x>x){found=i; x=rects[i].min.x;}
                    REPA(rects)if(rects[i].min.x>=x-EPS)rects.remove(i);
   }
}
/******************************************************************************/
void Include(MemPtr<UID> ids, C UID &id)
{
   if(id.valid())ids.binaryInclude(id);
}
/******************************************************************************/
bool Same(C Memc<UID> &a, C Memc<UID> &b)
{
   if(a.elms()!=b.elms())return false;
   REPA(a)if(a[i]!=b[i])return false;
   return true;
}
bool Same(C Memc<ObjData> &a, C Memc<ObjData> &b)
{
   if(a.elms()!=b.elms())return false;
   REPA(a)
   {
    C ObjData &oa=a[i];
      REPA(b)
      {
       C ObjData &ob=b[i];
         if(oa.id==ob.id)if(oa.equal(ob))goto oa_equal;else break;
      }
      return false; // not found or not equal
      oa_equal:;
   }
   return true;
}
void GetNewer(C Memc<ObjData> &a, C Memc<ObjData> &b, Memc<UID> &newer) // get id's of 'a' objects that are newer than 'b'
{
   REPA(a)
   {
    C ObjData &oa=a[i], *ob=null;
      REPA(b)if(b[i].id==oa.id){ob=&b[i]; break;}
      if(!ob || oa.newer(*ob))newer.add(oa.id);
   }
}
/******************************************************************************/
bool EmbedObject(C Box &obj_box, C VecI2 &area_xy, flt area_size)
{
   return obj_box.min.x/area_size<area_xy.x-0.5f || obj_box.max.x/area_size>area_xy.x+1.5f
       || obj_box.min.z/area_size<area_xy.y-0.5f || obj_box.max.z/area_size>area_xy.y+1.5f;
}
/******************************************************************************/
bool SameOS(OS_VER a, OS_VER b)
{
   return OSGroup(a)==OSGroup(b);
}
/******************************************************************************/
UID GetFileNameID(Str name)
{
   for(; name.is(); name=GetPath(name)){UID id=FileNameID(name); if(id.valid())return id;}
   return UIDZero;
}
UID AsID(C Elm *elm) {return elm ? elm->id : UIDZero;}
/******************************************************************************/
void SetPath(WindowIO &win_io, C Str &path, bool clear)
{
   Mems<FileParams> fps=FileParams::Decode(path); if(fps.elms()==1 && !fps[0].nodes.elms())
   {
      Str first=FFirstUp(fps[0].name);
      if(FileInfoSystem(first).type==FSTD_FILE)first=GetPath(first);
      fps[0].name=SkipStartPath(fps[0].name, first);
      win_io.path(S, first).textline.set(fps[0].encode()); return;
   }else
   if(fps.elms())
   {
      win_io.path(S).textline.set(path); return;
   }
   if(clear)win_io.path(S).textline.clear();
}
/******************************************************************************/
bool ParamTypeID        (PARAM_TYPE type           ) {return type==PARAM_ID || type==PARAM_ID_ARRAY;}
bool ParamTypeCompatible(PARAM_TYPE a, PARAM_TYPE b) {return a==b || (ParamTypeID(a) && ParamTypeID(b));}
bool ParamCompatible    (C Param   &a, C Param   &b) {return a.name==b.name && ParamTypeCompatible(a.type, b.type);}
/******************************************************************************/
/*void Add(MemPtr<Rename> diff, C Rename &rename, bool optimize=true)
{
   if(optimize)REPA(diff)switch(diff[i].check(rename)) // need to go from the end
   {
      case Rename.REQUIRED: goto add; // we already know that this is required, so skip checking
      case Rename.SAME    :                       return; // no need to apply the same change twice
      case Rename.REVERSE : diff.remove(i, true); return;
    //case Rename.UNKNOWN : break; // keep on checking
   }
add:
   diff.add(rename);
}
void AddReverse(MemPtr<Rename> diff, C Rename &rename, bool optimize=true)
{
   if(optimize)REPA(diff)switch(diff[i].check(rename)) // need to go from the end
   {
      case Rename.REQUIRED: goto add; // we already know that this is required, so skip checking
      case Rename.SAME    : diff.remove(i, true); return; //                                        !! HERE SAME AND REVERSE ARE SWITCHED !!
      case Rename.REVERSE :                       return; // no need to apply the same change twice !! HERE SAME AND REVERSE ARE SWITCHED !!
    //case Rename.UNKNOWN : break; // keep on checking
   }
add:
   diff.New().set(rename.dest, rename.src); // we're reversing so we need to replace dest with src
}
void Diff(MemPtr<Rename> diff, C MemPtr<Rename> &current, C MemPtr<Rename> &desired, bool optimize=true)
{
   diff.clear();
   int min=Min(current.elms(), desired.elms()), equal=0; for(; equal<min; equal++)if(current[equal]!=desired[equal])break; // calculate amount of equal changes

   // reverse 'current'
   for(int i=current.elms(); --i>=equal; )AddReverse(diff, current[i], optimize); // need to go from back

   // apply 'desired'
   for(int i=equal; i<desired.elms(); i++)Add(diff, desired[i], optimize); // need to go from start
}
/******************************************************************************/
UNIT_TYPE UnitType(C Str &s)
{
   if(s=="px"                 )return UNIT_PIXEL;
   if(s=="x"                  )return UNIT_REAL;
   if(s=="pc" || s=='%'       )return UNIT_PERCENT;
   if(s=="pm" || s==CharPermil)return UNIT_PERMIL;
   return UNIT_DEFAULT;
}
UNIT_TYPE GetUnitType(C Str &s)
{
   if(s.is())
   {
      const uint flag_and=CHARF_DIG10|CHARF_SIGN|CHARF_ALPHA|CHARF_UNDER|CHARF_SPACE;
      uint flag=CharFlag(s.last())&flag_and;
      int  pos =s.length()-1; for(; pos>0 && (CharFlag(s[pos-1])&flag_and)==flag; pos--);
      return UnitType(s()+pos);
   }
   return UNIT_DEFAULT;
}
flt ConvertUnitType(flt value, flt full, UNIT_TYPE unit)
{
   switch(unit)
   {
      default          : return value;
      case UNIT_REAL   : return value     *full;
      case UNIT_PERCENT: return value/ 100*full;
      case UNIT_PERMIL : return value/1000*full;
   }
}
/******************************************************************************/
// GUI
/******************************************************************************/
Color BackgroundColor()
{
   return Gui.backgroundColor();
}
Color BackgroundColorLight()
{
   if(Gui.skin)
      if(Gui.skin->window.active)
         return ColorMul(Gui.skin->window.active->center_color, Gui.skin->window.active_color);
   return Gui.backgroundColor();
}
Color GuiListTextColor()
{
   if(Gui.skin && Gui.skin->list.text_style)return Gui.skin->list.text_style->color;
   return BLACK;
}
const Color LitSelColor=RED, SelColor=YELLOW, LitColor=CYAN, DefColor=WHITE, InvalidColor=PURPLE;
Color GetLitSelCol(bool lit, bool sel, C Color &none)
{
   if(lit && sel)return LitSelColor;
   if(       sel)return SelColor;
   if(lit       )return LitColor;
                 return none;
}
bool ErrorCopy(C Str &src, C Str &dest)
{
   Gui.msgBox(S, S+"Error copying\n\""+src+"\"\nto\n\""+dest+'"');
   return false;
}
bool ErrorRecycle(C Str &name)
{
   Gui.msgBox(S, S+"Error recycling\n\""+name+"\"");
   return false;
}
bool ErrorCreateDir(C Str &name)
{
   Gui.msgBox(S, S+"Error creating folder\n\""+name+"\"");
   return false;
}
bool RecycleLoud  (C Str &name            ) {return FRecycle   (name     ) ? true : ErrorRecycle  (name);}
bool CreateDirLoud(C Str &name            ) {return FCreateDirs(name     ) ? true : ErrorCreateDir(name);}
bool SafeCopyLoud (C Str &src, C Str &dest) {return SafeCopy   (src, dest) ? true : ErrorCopy     (src, dest);}
/******************************************************************************/
// SOUND
/******************************************************************************/
const BitRateQuality BitRateQualities[]=
{
   {-0.2f,  32*1000}, // aoTuV only
   {-0.1f,  45*1000},
   { 0.0f,  64*1000},
   { 0.1f,  80*1000},
   { 0.2f,  96*1000},
   { 0.3f, 112*1000},
   { 0.4f, 128*1000},
   { 0.5f, 160*1000},
   { 0.6f, 192*1000},
   { 0.7f, 224*1000},
   { 0.8f, 256*1000},
   { 0.9f, 320*1000},
   { 1.0f, 500*1000},
};
flt VorbisBitRateToQuality(int rel_bit_rate) // relative bit rate in bits per second (bit rate for 44.1kHz stereo)
{
   for(int i=1; i<Elms(BitRateQualities); i++)if(rel_bit_rate<=BitRateQualities[i].bit_rate)
   {
    C BitRateQuality &p=BitRateQualities[i-1],
                     &n=BitRateQualities[i  ];
      flt step=LerpR(p.bit_rate, n.bit_rate, rel_bit_rate);
      return   Lerp (p.quality , n.quality , step);
   }
   return 1;
}
/******************************************************************************/
// DEPRECATED
/******************************************************************************/
int DecIntV(File &f)
{
   byte v; f>>v;
   Bool positive=((v>>6)&1);
   UInt u=(v&63);
   if(v&128)
   {
      f>>v; u|=((v&127)<<6);
      if(v&128)
      {
         f>>v; u|=((v&127)<<(6+7));
         if(v&128)
         {
            f>>v; u|=((v&127)<<(6+7+7));
            if(v&128)
            {
               f>>v; u|=(v<<(6+7+7+7));
            }
         }
      }
   }
   return positive ? u+1 : -Int(u);
}

void GetStr2(File &f, Str &s) {s=GetStr2(f);}
Str  GetStr2(File &f)
{
   Int length=DecIntV(f);
   if( length<0) // unicode
   {
      CHS(length); MIN(length, f.left()/2);
      Str s; s.reserve(length); REP(length){char c; f>>c; s+=c;} return s;
   }else
   if(length)
   {
      MIN(length, f.left());
      Str8 s; s.reserve(length); REP(length){char8 c; f>>c; s+=c;} return s;
   }
   return S;
}

void PutStr(File &f, C Str &s)
{
   uint length =s.length();
   bool unicode=HasUnicode(s);

   f.putUInt(unicode ? length^SIGN_BIT : length);
   if(length)
   {
      if(unicode){          f.putN(s(), length);}
      else       {Str8 t=s; f.putN(t(), length);}
   }
}
Str GetStr(File &f)
{
   uint length=f.getUInt();
   if(  length&SIGN_BIT) // unicode
   {
         length^=SIGN_BIT; MIN(length, f.left()/2);
      if(length){Str s; s.reserve(length); REP(length){char c; f>>c; s+=c;} return s;}
   }else
   {
      MIN(length, f.left());
      if (length){Str8 s; s.reserve(length); REP(length){char8 c; f>>c; s+=c;} return s;}
   }
   return S;
}
void GetStr(File &f, Str &s) {s=GetStr(f);}

Mems<FileParams> _DecodeFileParams(C Str &str)
{
   Mems<FileParams> files; if(str.is())
   {
      Memc<Str> strs=Split(str, '|'); // get list of all files
      files.setNum(strs.elms()); FREPA(files)
      {
         FileParams &file=files[i];
         Memc<Str> fp=Split(strs[i], '?'); // file_name?params
         file.name=(fp.elms() ? fp[0] : S);
         if(fp.elms()>=2)
         {
            Memc<Str> name_vals=Split(fp[1], '&'); FREPA(name_vals)
            {
               Memc<Str> name_val=Split(name_vals[i], '=');
               if(name_val.elms()==2)file.params.New().set(name_val[0], name_val[1]);
            }
         }
      }
   }
   return files;
}
/******************************************************************************/

/******************************************************************************/
   bool FileSizeGetter::created()C {           return path.is();}
   bool FileSizeGetter::busy()  {cleanup(); return thread.active();}
   bool FileSizeGetter::get()
   {
      cleanup();
      if(elms_thread.elms())
      {
         SyncLocker locker(lock);
         if(!elms.elms())Swap(elms_thread, elms);else
         {
            FREPA(elms_thread)elms.add(elms_thread[i]);
            elms_thread.clear();
         }
         return true;
      }
      return false;
   }
   void FileSizeGetter::clear() {elms.clear();}
   void FileSizeGetter::stop() {thread.stop();}
   void FileSizeGetter::del()
   {
      thread.del(); // del the thread first
      elms_thread.clear();
      elms       .clear();
      path       .clear();
   }
   void FileSizeGetter::get(C Str &path)
   {
      del();
      if(path.is())
      {
         T.path=path;
         thread.create(Func, this);
      }
   }
  FileSizeGetter::~FileSizeGetter() {del();}
   void FileSizeGetter::cleanup()
   {
      if(!thread.active())thread.del(); // delete to free resources
   }
   bool FileSizeGetter::Func(Thread &thread) {return ((FileSizeGetter*)thread.user)->func();}
          bool FileSizeGetter::func()
   {
      for(FileFind ff(path); !thread.wantStop() && ff(); )
      {
         if(ff.type==FSTD_FILE)
         {
            UID id; if(DecodeFileName(ff.name, id))
            {
               SyncLocker locker(lock);
               Elm &elm=elms_thread.New();
               elm.id=id;
               elm.file_size=ff.size;
            }
         }
      }
      return false;
   }
   ImageHashHeader::ImageHashHeader(C Image &image, IMAGE_TYPE type)
   {
      ASSERT(SIZE(ImageHashHeader)==3*4+4); // make sure all compilers generate the same size
      Zero(T); // it's very important to zero entire data at the start, in case there's any extra padding, to make sure hash is always the same
      T.size=image.size3();
      T.type=type;
      if(image.cube())T.flags|=1;
   }
   XMaterialEx::XMaterialEx(C Project *proj) : base_0_id(UIDZero), base_1_id(UIDZero), base_2_id(UIDZero), detail_id(UIDZero), macro_id(UIDZero), emissive_id(UIDZero), adjust_params(true), has_textures(TEXF_NONE), known_textures(TEXF_NONE)
   {
      if(proj)
      {
         Copy(tex_downsize, proj->tex_downsize);
      }else
      {
         REPAO(tex_downsize)=0;
      }
   }
   void XMaterialEx::create(C Material &src)
   {
      super::create(src);
      adjust_params=false; // don't adjust params because EE Materials are OK
   }
   SrcDest& SrcDest::set(C Str &src, C Str &dest) {T.src=src; T.dest=dest; return T;}
   SrcDest& SrcDest::set(C Str &src, C Str &dest, C Str &name) {T.src=src+name; T.dest=dest+name; return T;}
   bool Rename::operator==(C Rename &rename)C {return Equal(src, rename.src, true) && Equal(dest, rename.dest, true);}
   bool Rename::operator!=(C Rename &rename)C {return !(T==rename);}
/******************************************************************************/
