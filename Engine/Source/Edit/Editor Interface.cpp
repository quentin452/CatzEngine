/******************************************************************************/
#include "stdafx.h"
namespace EE{
static Int Compare(C Edit::Elm &elm, C UID &id) {return Compare(elm.id, id);}
namespace Edit{
/******************************************************************************/
#define EI_VER 56 // this needs to be increased every time a new command is added, existing one is changed, or some of engine class file formats get updated
#define EI_STR (ENGINE_NAME " Editor Network Interface")

#define CLIENT_WAIT_TIME         (   60*1000) //    60 seconds
#define CLIENT_WAIT_TIME_LONG    (15*60*1000) // 15*60 seconds, some operations may take a long time to complete (reloading material textures with resizing, getting world objects, ..)
#define CLIENT_WAIT_TIME_PUBLISH (60*60*1000) // 60*60 seconds, publishing may take very long time (especially when creating PVRTC textures)

ASSERT( EI_NUM<=256); // because they're stored as bytes
ASSERT(ELM_NUM<=256); // because they're stored as bytes
/******************************************************************************/
static void Finalize(Elm &elm, C CMemPtr<Elm> &elms) // 'elms' must be sorted by their ID
{
    elm.full_name=elm.name;
   auto flags    =elm.flags;

   Memt<UID> processed; processed.add(elm.id);
 C Elm *cur=&elm;
parent:
 C UID &parent_id=cur->parent_id; if(parent_id.valid() && processed.binaryInclude(parent_id))if(cur=elms.binaryFind(parent_id, Compare))
   {
      elm.full_name=cur->name+'\\'+elm.full_name;
          flags   |=cur->flags; // !! can use OR because we have NO_PUBLISH, if it was PUBLISH then we need a SEPARATE AND !!
      goto parent;
   }

   FlagSet(elm.flags, Elm::REMOVED_FULL          , FlagOn(flags, Elm::REMOVED                                           )); // Removed
   FlagSet(elm.flags, Elm::NO_PUBLISH_FULL       , FlagOn(flags, Elm::REMOVED | Elm::NO_PUBLISH                         )); // Removed || NoPublish
   FlagSet(elm.flags, Elm::NO_PUBLISH_MOBILE_FULL, FlagOn(flags, Elm::REMOVED | Elm::NO_PUBLISH | Elm::NO_PUBLISH_MOBILE)); // Removed || NoPublish || NoPublishMobile
}
/******************************************************************************/
// MATERIAL MAP
/******************************************************************************/
void MaterialMap::create(Int resolution)
{
  _m .createSoftTry(resolution, resolution, 1, IMAGE_R8G8B8A8);
  _i .createSoftTry(resolution, resolution, 1, IMAGE_R8G8B8A8);
  _ip.clear();
}
void MaterialMap::del()
{
  _m .del();
  _i .del();
  _ip.del();
}
Int MaterialMap::resolution()C {return _m.w();}
/******************************************************************************/
void MaterialMap::set(Int x, Int y, C UID &m0, C UID &m1, C UID &m2, C UID &m3, C VecB4 &blend)
{
  _m.color(x, y, Color(_ip.getIDIndex0(m0), _ip.getIDIndex0(m1), _ip.getIDIndex0(m2), _ip.getIDIndex0(m3)));
  _i.color(x, y, Color( blend.x           ,  blend.y           ,  blend.z           ,  blend.w           ));
}
void MaterialMap::set(Int x, Int y, C UID &m0, C UID &m1, C UID &m2, C UID &m3, C Vec4 &blend)
{
   Vec4 b=blend; if(Flt sum=b.sum())b/=sum; // normalize blends
  _m.color (x, y, Color(_ip.getIDIndex0(m0), _ip.getIDIndex0(m1), _ip.getIDIndex0(m2), _ip.getIDIndex0(m3)));
  _i.colorF(x, y, b);
}
void MaterialMap::get(Int x, Int y, UID &m0, UID &m1, UID &m2, UID &m3, VecB4 &blend)
{
   Color m=_m.color(x, y),
         i=_i.color(x, y);
   m0=(InRange(m.r, _ip) ? _ip[m.r] : UIDZero);
   m1=(InRange(m.g, _ip) ? _ip[m.g] : UIDZero);
   m2=(InRange(m.b, _ip) ? _ip[m.b] : UIDZero);
   m3=(InRange(m.a, _ip) ? _ip[m.a] : UIDZero);
   blend.set(i.r, i.g, i.b, i.a);
}
void MaterialMap::get(Int x, Int y, UID &m0, UID &m1, UID &m2, UID &m3, Vec4 &blend)
{
   Color m=_m.color (x, y);
   Vec4  i=_i.colorF(x, y);
   m0=(InRange(m.r, _ip) ? _ip[m.r] : UIDZero);
   m1=(InRange(m.g, _ip) ? _ip[m.g] : UIDZero);
   m2=(InRange(m.b, _ip) ? _ip[m.b] : UIDZero);
   m3=(InRange(m.a, _ip) ? _ip[m.a] : UIDZero);
   blend.set(i.x, i.y, i.z, i.w);
}
/******************************************************************************/
void MaterialMap::resize(Int resolution)
{
   MAX(resolution, 0);
   if (resolution!=T.resolution())
   {
     _m.resize(resolution, resolution, FILTER_NONE, IC_CLAMP|IC_KEEP_EDGES);
     _i.resize(resolution, resolution, FILTER_NONE, IC_CLAMP|IC_KEEP_EDGES);
   }
}
/******************************************************************************/
Bool MaterialMap::save(File &f)C
{
   f.cmpUIntV(0); // version
   if(_m .save(f))
   if(_i .save(f))
   if(_ip.save(f))
      return f.ok();
   return false;
}
Bool MaterialMap::load(File &f)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         if(_m .load(f))
         if(_i .load(f))
         if(_ip.load(f))
            if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
// MATERIAL
/******************************************************************************/
static Str  Encode(       C Mems  <FileParams> &file_params) {return      FileParams::Encode(file_params);}
static void Decode(File &f, MemPtr<FileParams>  file_params) {file_params=FileParams::Decode(f.getStr() );}
/******************************************************************************/
Material& Material::reset()
{
   technique=MTECH_OPAQUE;
   tex_quality=MEDIUM;
   cull=true;
   detail_all_lod=false;
   flip_normal_y=false;
   smooth_is_rough=false;
   tex_downsize_mobile=tex_downsize_switch=0;
      color_s=1;
   emissive_s=0;
   emissive_glow=0;
   smooth=0;
   reflect_min=MATERIAL_REFLECT;
   reflect_max=1;
   glow=0;
   normal=0;
   bump=0;
   uv_scale=1;
      color_map   .clear();  alpha_map.clear();
       bump_map   .clear(); normal_map.clear();
     smooth_map   .clear();  metal_map.clear();
       glow_map   .clear();
     detail_color .clear();
     detail_bump  .clear();
     detail_normal.clear();
     detail_smooth.clear();
      macro_map   .clear();
   emissive_map   .clear();
   return T;
}
Bool Material::save(File &f)C
{
   f.cmpUIntV(0);
   f<<technique<<tex_quality<<cull<<detail_all_lod<<flip_normal_y<<smooth_is_rough<<tex_downsize_mobile<<tex_downsize_switch<<color_s<<emissive_s<<emissive_glow<<smooth<<reflect_min<<reflect_max<<glow<<normal<<bump<<uv_scale
    <<Encode(   color_map   )<<Encode( alpha_map)
    <<Encode(    bump_map   )<<Encode(normal_map)
    <<Encode(  smooth_map   )<<Encode( metal_map)
    <<Encode(    glow_map   )
    <<Encode(  detail_color )
    <<Encode(  detail_bump  )
    <<Encode(  detail_normal)
    <<Encode(  detail_smooth)
    <<Encode(   macro_map   )
    <<Encode(emissive_map   );
   return f.ok();
}
Bool Material::load(File &f)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f>>technique>>tex_quality>>cull>>detail_all_lod>>flip_normal_y>>smooth_is_rough>>tex_downsize_mobile>>tex_downsize_switch>>color_s>>emissive_s>>emissive_glow>>smooth>>reflect_min>>reflect_max>>glow>>normal>>bump>>uv_scale;
         Decode(f,    color_map   ); Decode(f,  alpha_map);
         Decode(f,     bump_map   ); Decode(f, normal_map);
         Decode(f,   smooth_map   ); Decode(f,  metal_map);
         Decode(f,     glow_map   );
         Decode(f,   detail_color );
         Decode(f,   detail_bump  );
         Decode(f,   detail_normal);
         Decode(f,   detail_smooth);
         Decode(f,    macro_map   );
         Decode(f, emissive_map   );
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
/******************************************************************************/
// SKELETON
/******************************************************************************/
SkeletonSlot::SkeletonSlot()
{
   dir .set (0, 0, 1);
   perp.set (0, 1, 0);
   pos .zero(       );
}
void SkeletonSlot::setFrom(C SkelSlot &src, C Skeleton &skeleton)
{
   SCAST(OrientP, T)=src;
   name=src.name;
   if(InRange(src.bone , skeleton.bones))bone_name =skeleton.bones[src.bone ].name;else bone_name .clear();
   if(InRange(src.bone1, skeleton.bones))bone_name1=skeleton.bones[src.bone1].name;else bone_name1.clear();
}
void SkeletonSlot::setTo(SkelSlot &dest, C Skeleton &skeleton)C
{
   SCAST(OrientP, dest)=T;
   Set(dest.name, name);
   dest.bone =skeleton.findBoneB((Str8)bone_name );
   dest.bone1=skeleton.findBoneB((Str8)bone_name1);
}
Bool SkeletonSlot::save(File &f)C
{
   f.cmpUIntV(0);
   f<<dir<<perp<<pos;
   f<<name<<bone_name<<bone_name1;
   return f.ok();
}
Bool SkeletonSlot::load(File &f)
{
   switch(f.decUIntV())
   {
      case 0:
      {
         f>>dir>>perp>>pos;
         f>>name>>bone_name>>bone_name1;
         if(f.ok())return true;
      }break;
   }
   /*del();*/ return false;
}
/******************************************************************************/
// OBJ DATA
/******************************************************************************/
ObjChange::ObjChange()
{
   cmd=EI_OBJ_NONE;
   type=PARAM_TYPE(0);
   id.zero();
}
void ObjChange:: removeParam(C UID &param_id                         ) {cmd=EI_OBJ_PARAM_REMOVE_ID   ; T.id  =param_id;}
void ObjChange:: removeParam(C Str &param_name, PARAM_TYPE param_type) {cmd=EI_OBJ_PARAM_REMOVE_NAME ; T.name=param_name; T.type=param_type;}
void ObjChange::restoreParam(C UID &param_id                         ) {cmd=EI_OBJ_PARAM_RESTORE_ID  ; T.id  =param_id;}
void ObjChange::restoreParam(C Str &param_name, PARAM_TYPE param_type) {cmd=EI_OBJ_PARAM_RESTORE_NAME; T.name=param_name; T.type=param_type;}

void ObjChange::renameParam(C UID &param_id      , C Str &param_new_name                       ) {cmd=EI_OBJ_PARAM_RENAME_ID  ; T.id  =param_id      ;                    T.param.name=param_new_name;}
void ObjChange::renameParam(C Str &param_old_name, C Str &param_new_name, PARAM_TYPE param_type) {cmd=EI_OBJ_PARAM_RENAME_NAME; T.name=param_old_name; T.type=param_type; T.param.name=param_new_name;}

void ObjChange::setParamTypeValue(C UID &param_id  , C Param &type_value                           ) {cmd=EI_OBJ_PARAM_SET_TYPE_VALUE_ID  ; T.id  =param_id  ;                        T.param=type_value;}
void ObjChange::setParamTypeValue(C Str &param_name, C Param &type_value, PARAM_TYPE param_old_type) {cmd=EI_OBJ_PARAM_SET_TYPE_VALUE_NAME; T.name=param_name; T.type=param_old_type; T.param=type_value;}

Bool ObjChange::save(File &f, CChar *path)C
{
   f<<cmd<<type<<id<<name; return param.save(f, path) && f.ok();
}
Bool ObjChange::load(File &f, CChar *path)
{
   f>>cmd>>type>>id>>name; if(param.load(f, path) && f.ok())return true;
   /*del();*/ return false;
}
/******************************************************************************/
ObjData& ObjData::reset()
{
   elm_obj_class_id.zero();
   access=OBJ_ACCESS_TERRAIN;
   path  =OBJ_PATH_CREATE;
   params.clear();
   return T;
}
ObjData::Param* ObjData::findParam(C Str &name, PARAM_TYPE type, Bool include_removed, Bool include_inherited)
{
   FREPA(params) // check in order, because Editor lists params from current object first (which we're the most interested in), then adds base/parent params later
   {
      Param &param=params[i];
      if( param.name==name // don't try to break when encountered a param with the same 'name', because there can be multiple params with the same name but different type/removed
      && (type==PARAM_NUM   || type==param.type)
      && (include_removed   || !param.  removed)
      && (include_inherited || !param.inherited)
      )return &param;
   }
   return null;
}
Bool ObjData::save(File &f)C {f<<elm_obj_class_id<<access<<path; return params.save(f) && f.ok();}
Bool ObjData::load(File &f)  {f>>elm_obj_class_id>>access>>path; return params.load(f) && f.ok();}
/******************************************************************************/
// CLIENT
/******************************************************************************/
Bool EditorInterface::   connected() {_conn.updateState(0); return _conn.state()==CONNECT_GREETED;} // try to receive data so we check if connection got disconnected
void EditorInterface::disconnect  ()
{
  _conn.del();
}
struct ConnectAttempt : Connection
{
   Bool sent, bad_ver;

   ConnectAttempt() {sent=bad_ver=false;}

   void create(Int port) {clientConnectToServer(SockAddr().setLocalFast(port));}
   Bool update()
   {
      if(updateState(0))
      {
         if(!sent)
         {
            sent=true;
            data.reset().putByte(EI_VERSION_CHECK).putStr(EI_STR).cmpUIntV(EI_VER).pos(0);
            if(!send(data))del();
         }
         if(receive(0))
         {
            if(      data.getByte()==EI_VERSION_CHECK)
            if(Equal(data.getStr (), EI_STR, true))
            {
               bad_ver=(data.decUIntV()!=EI_VER);
               if(!bad_ver)
               {
                  tcpNoDelay(true);
                  return true;
               }
            }
            del();
         }
      }
      return false;
   }
};
Bool EditorInterface::connect(Str &message, Int timeout)
{
   disconnect();
   if(timeout<0)timeout=1000; UInt start_time=Time.curTimeMs();
   ConnectAttempt c[128];
   
   // if applications are run through the Editor then it passes its Editor Network Interface port as "EditorPort" param
   FREPA(App.cmd_line)
   {
    C Str &param=App.cmd_line[i]; if(param=="EditorPort")
      {
         if(InRange(i+1, App.cmd_line))
         {
            Int port=TextInt(App.cmd_line[i+1]); if(port>0)
            {
               c[0].create(port);
               goto created;
            }
         }
         break;
      }
   }
   FREPAO(c).create(65535-i);
created:

   for(;;)
   {
      Bool connecting=false;
      FREPA(c)if(c[i].update())
      {
         message.clear();
         Swap(_conn, SCAST(Connection, c[i]));
         return true;
      }else if(c[i].state()!=CONNECT_INVALID)connecting=true;
      if(!connecting)break;
      if((Time.curTimeMs()-start_time)>=timeout)break; // this code was tested OK for UInt overflow
      Time.wait(1);
   }
   REPA(c)if(c[i].bad_ver){message="This Application version is not compatible with the opened " ENGINE_NAME " Editor version.\nPlease upgrade your software."; return false;}
   message=ENGINE_NAME " Editor does not appears to be opened.\nPlease open " ENGINE_NAME " Editor and enable option \"Allow Incoming Connections\"."; return false;
}
/******************************************************************************/
// PROJECTS
/******************************************************************************/
Str EditorInterface::projectsPath()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_PROJECTS_PATH).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_PROJECTS_PATH)return f.getStr();
      disconnect();
   }
   return S;
}
Bool EditorInterface::projectsPath(C Str &path)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_PROJECTS_PATH).putStr(path).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_PROJECTS_PATH)return f.getBool();
      disconnect();
   }
   return false;
}

Bool EditorInterface::getProjects(MemPtr<Project> projects)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_PROJECTS).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_PROJECTS)
      if(projects.load(f))return true;
      disconnect();
   }
   projects.clear(); return false;
}

UID EditorInterface::curProject()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_PROJECT).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_PROJECT)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
Bool EditorInterface::openProject(C UID &proj_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_PROJECT).putUID(proj_id).pos(0);
      if(_conn.send(f))return curProject()==proj_id;
      disconnect();
   }
   return false;
}
/******************************************************************************/
// SETTINGS
/******************************************************************************/
Str EditorInterface::dataPath()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_DATA_PATH).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_DATA_PATH)return f.getStr();
      disconnect();
   }
   return S;
}
Str EditorInterface::editorPath()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_EDITOR_PATH).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_EDITOR_PATH)return f.getStr();
      disconnect();
   }
   return S;
}
/******************************************************************************/
// ELEMENTS
/******************************************************************************/
Bool EditorInterface::getElms(MemPtr<Elm> elms, Bool include_removed)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_ELMS).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_ELMS)
      {
         if(f.getBool()) // if success
         if(elms.load(f))
         {
            REPA(elms)Finalize(elms[i], elms); // setup full values for convenience
            if(!include_removed) // call this at the end, once all elements have been setup, so we won't remove an element that still has unprocessed children, do this on the client, to avoid overloading the server
               REPA(elms)if(elms[i].removedFull())elms.remove(i, true); // need to keep order because elements are sorted by ID
            return true;
         }
         goto fail;
      }
      disconnect();
   }
fail:;
   elms.clear(); return false;
}
UID EditorInterface::litElm()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_ELM_LIT).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_ELM_LIT)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
Bool EditorInterface::selectedElms(MemPtr<UID> elms)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_ELMS_SELECTED).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_ELMS_SELECTED)
      {
         if(f.getBool()) // if success
         if(elms.loadRaw(f))return true;
         goto fail;
      }
      disconnect();
   }
fail:;
   elms.clear(); return false;
}
Bool EditorInterface::selectElms(C CMemPtr<UID> &elms)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ELMS_SELECTED); elms.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ELMS_SELECTED)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::reloadElms(C CMemPtr<UID> &elms, Bool remember_result)
{
   if(!elms.elms())return true;
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_RLD_ELMS)<<remember_result; elms.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_RLD_ELMS)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::cancelReloadElms(C CMemPtr<UID> &elms)
{
   if(!elms.elms())return true;
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_RLD_ELMS_CANCEL); elms.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_RLD_ELMS_CANCEL)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::reloadResult(C CMemPtr<UID> &elms, MemPtr< IDParam<RELOAD_RESULT> > results)
{
   if(elms.elms() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_RLD_ELMS_GET_RESULT); elms.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_RLD_ELMS_GET_RESULT)
      {
         if(f.getBool())
         if(results.load(f))return true;
         goto fail;
      }
      disconnect();
   }
fail:
   results.clear(); return !elms.elms(); // OK only if there were no elements
}
Bool EditorInterface::forgetReloadResult(C CMemPtr<UID> &elms)
{
   if(!elms.elms())return true;
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_RLD_ELMS_FORGET_RESULT); elms.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_RLD_ELMS_FORGET_RESULT)return f.getBool();
      disconnect();
   }
   return false;
}
UID EditorInterface::newElm(ELM_TYPE type, C Str &name, C UID &parent_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_NEW_ELM).putByte(type).putStr(name).putUID(parent_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_NEW_ELM)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
UID EditorInterface::newWorld(C Str &name, Int area_size, Int terrain_res, C UID &parent_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_NEW_WORLD).putStr(name).putInt(area_size).putInt(terrain_res).putUID(parent_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_NEW_WORLD)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
/******************************************************************************/
Bool EditorInterface::setElmCmd(Byte cmd, C CMemPtr< IDParam<Bool> > &elms)
{
   if(!elms.elms())return true;
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(cmd); elms.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==cmd)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::setElmName(C CMemPtr< IDParam<Str> > &elms)
{
   if(!elms.elms())return true;
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ELM_NAME); elms.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ELM_NAME)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::setElmRemoved      (C CMemPtr< IDParam<Bool> > &elms) {return setElmCmd(EI_SET_ELM_REMOVED       , elms);}
Bool EditorInterface::setElmPublish      (C CMemPtr< IDParam<Bool> > &elms) {return setElmCmd(EI_SET_ELM_PUBLISH       , elms);}
Bool EditorInterface::setElmPublishMobile(C CMemPtr< IDParam<Bool> > &elms) {return setElmCmd(EI_SET_ELM_PUBLISH_MOBILE, elms);}

Bool EditorInterface::setElmParent(C CMemPtr< IDParam<UID> > &elms)
{
   if(!elms.elms())return true;
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ELM_PARENT); elms.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ELM_PARENT)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::setElmSrcFile(C CMemPtr< IDParam<Str> > &elms)
{
   if(!elms.elms())return true;
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ELM_SRC_FILE); elms.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ELM_SRC_FILE)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// WORLD
/******************************************************************************/
UID EditorInterface::curWorld()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
Bool EditorInterface::openWorld(C UID &world_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_WORLD).putUID(world_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_WORLD)return f.getBool();
      disconnect();
   }
   return false;
}
Int EditorInterface::worldAreaSize(C UID &world_id)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_AREA_SIZE).putUID(world_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_AREA_SIZE)return f.getInt();
      disconnect();
   }
   return 0;
}
/******************************************************************************/
// WORLD TERRAIN
/******************************************************************************/
Int EditorInterface::worldTerrainRes(C UID &world_id)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN_RES).putUID(world_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN_RES)return f.getInt();
      disconnect();
   }
   return 0;
}
Bool EditorInterface::worldTerrainAreas(C UID &world_id, RectI &areas)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN_AREAS).putUID(world_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN_AREAS)
      {
         if(f.getBool()){f>>areas; return true;}
         goto fail;
      }
      disconnect();
   }
fail:;
   areas.set(0, 0, -1, -1); return !world_id.valid();
}
/******************************************************************************/
Bool EditorInterface::worldTerrainDel(C UID &world_id, C VecI2 &area_xy)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_DEL_WORLD_TERRAIN).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_DEL_WORLD_TERRAIN)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::worldTerrainExists(C UID &world_id, C VecI2 &area_xy)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN_IS).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN_IS)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
Bool EditorInterface::worldTerrainGetHeight(C UID &world_id, C VecI2 &area_xy, Image &height)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN_HEIGHT).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN_HEIGHT)
      {
         if(f.getBool())return height.load(f);
         goto fail;
      }
      disconnect();
   }
fail:;
   height.del(); return !world_id.valid();
}
Bool EditorInterface::worldTerrainSetHeight(C UID &world_id, C VecI2 &area_xy, C Image &height, C UID &material_id)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_WORLD_TERRAIN_HEIGHT).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).putUID(material_id); height.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_WORLD_TERRAIN_HEIGHT)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
Bool EditorInterface::worldTerrainGetColor(C UID &world_id, C VecI2 &area_xy, Image &color)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN_COLOR).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN_COLOR)
      {
         if(f.getBool())return color.load(f);
         goto fail;
      }
      disconnect();
   }
fail:;
   color.del(); return !world_id.valid();
}
Bool EditorInterface::worldTerrainSetColor(C UID &world_id, C VecI2 &area_xy, C Image &color, C UID &material_id)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_WORLD_TERRAIN_COLOR).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).putUID(material_id); color.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_WORLD_TERRAIN_COLOR)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
Bool EditorInterface::worldTerrainGetMaterial(C UID &world_id, C VecI2 &area_xy, MaterialMap &material)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN_MATERIAL).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN_MATERIAL)
      {
         if(f.getBool())return material.load(f);
         goto fail;
      }
      disconnect();
   }
fail:;
   material.del(); return !world_id.valid();
}
Bool EditorInterface::worldTerrainSetMaterial(C UID &world_id, C VecI2 &area_xy, C MaterialMap &material)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_WORLD_TERRAIN_MATERIAL).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y); material.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_WORLD_TERRAIN_MATERIAL)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
Bool EditorInterface::worldTerrainGet(C UID &world_id, C VecI2 &area_xy, Image &height, MaterialMap &material, Image *color)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_TERRAIN).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).putBool(color!=null).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_TERRAIN)
      {
         if(f.getBool())if(height.load(f))if(material.load(f))if(color ? color->load(f) : true)return true;
         goto fail;
      }
      disconnect();
   }
fail:;
   height.del(); material.del(); if(color)color->del(); return !world_id.valid();
}
Bool EditorInterface::worldTerrainSet(C UID &world_id, C VecI2 &area_xy, C Image &height, C MaterialMap &material, C Image *color)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_WORLD_TERRAIN).putUID(world_id).cmpIntV(area_xy.x).cmpIntV(area_xy.y).putBool(color!=null); height.save(f); material.save(f); if(color)color->save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_WORLD_TERRAIN)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// WORLD OBJECTS
/******************************************************************************/
Bool EditorInterface::worldObjCreate(C UID &world_id, C CMemPtr<WorldObjParams> &objs)
{
   if(!objs.elms())return true;
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_NEW_WORLD_OBJ).putUID(world_id); objs.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_NEW_WORLD_OBJ)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::worldObjGetDesc(C UID &world_id, MemPtr<WorldObjDesc> objs, C CMemPtr<UID> &world_obj_instance_ids, C RectI *areas, Bool only_selected, Bool include_removed)
{
   if(world_id.valid() && world_obj_instance_ids.elms() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_OBJ_BASIC).putUID(world_id).putBool(areas!=null); if(areas)f<<*areas; f<<only_selected<<include_removed; world_obj_instance_ids.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_OBJ_BASIC)
      {
         if(f.getBool())return objs.load(f);
         goto fail;
      }
      disconnect();
   }
fail:
   objs.clear(); return !(world_id.valid() && world_obj_instance_ids.elms());
}
Bool EditorInterface::worldObjGetData(C UID &world_id, MemPtr<WorldObjData> objs, C CMemPtr<UID> &world_obj_instance_ids, C RectI *areas, Bool only_selected, Bool include_removed, Bool include_removed_params)
{
   if(world_id.valid() && world_obj_instance_ids.elms() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_OBJ_FULL).putUID(world_id).putBool(areas!=null); if(areas)f<<*areas; f<<only_selected<<include_removed<<include_removed_params; world_obj_instance_ids.saveRaw(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(only_selected ? CLIENT_WAIT_TIME : CLIENT_WAIT_TIME_LONG)) // if world is big then it might take some time to get all objects, so wait for a long time
      if(f.getByte()==EI_GET_WORLD_OBJ_FULL)
      {
         if(f.getBool())return objs.load(f);
         goto fail;
      }
      disconnect();
   }
fail:
   objs.clear(); return !(world_id.valid() && world_obj_instance_ids.elms());
}
// !! when sending object data - HANDLE CORRECT REL PATH FOR OBJ PARAM ENUMS !!
/******************************************************************************/
// WORLD WAYPOINTS
/******************************************************************************/
Bool EditorInterface::worldWaypointGetList(C UID &world_id, MemPtr<UID> waypoint_ids)
{
   if(world_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_WAYPOINT_LIST).putUID(world_id); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_WAYPOINT_LIST)
      {
         if(f.getBool())return waypoint_ids.loadRaw(f);
         goto fail;
      }
      disconnect();
   }
fail:
   waypoint_ids.clear(); return !world_id.valid();
}
/******************************************************************************/
// WORLD CAMERA
/******************************************************************************/
Bool EditorInterface::worldCamGet(Camera &cam)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_WORLD_CAM).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_WORLD_CAM)return cam.load(f);
      disconnect();
   }
   return false;
}
Bool EditorInterface::worldCamSet(C Camera &cam)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_WORLD_CAM); cam.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_WORLD_CAM)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// WORLD DRAW
/******************************************************************************/
Bool EditorInterface::worldDrawLines(C CMemPtr<Line> &lines)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_DRAW_WORLD_LINES); lines.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_DRAW_WORLD_LINES)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// IMAGE
/******************************************************************************/
Bool EditorInterface::getImage(C UID &elm_id, Image &image)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_IMAGE).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_IMAGE)
      {
         if(f.getBool())return image.load(f);
         goto fail;
      }
      disconnect();
   }
fail:;
   image.del(); return !elm_id.valid();
}
Bool EditorInterface::setImage(C UID &elm_id, C Image &image)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_IMAGE).putUID(elm_id); image.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_IMAGE)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::setImageMipMaps(C UID &elm_id, Bool mip_maps)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_IMAGE_MIP_MAPS).putUID(elm_id).putBool(mip_maps); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_IMAGE_MIP_MAPS)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// CODE
/******************************************************************************/
Bool EditorInterface::getCode(C UID &elm_id, Str &code)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_CODE).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_CODE)
      {
         if(f.getBool())return code.load(f);
         goto fail;
      }
      disconnect();
   }
fail:;
   code.clear(); return !elm_id.valid();
}
Bool EditorInterface::setCode(C UID &elm_id, C Str &code)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_CODE).putUID(elm_id).putStr(code).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_CODE)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
Bool EditorInterface::codeSyncImport()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_CODE_SYNC_IMPORT).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_CODE_SYNC_IMPORT)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::codeSyncExport()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_CODE_SYNC_EXPORT).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_CODE_SYNC_EXPORT)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// FILE
/******************************************************************************/
Bool EditorInterface::getFile(C UID &elm_id, File &data)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_FILE).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_FILE)
      {
         if(f.getBool())return f.copy(data);
         return false;
      }
      disconnect();
   }
   return !elm_id.valid();
}
Bool EditorInterface::setFile(C UID &elm_id, File &data)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_FILE).putUID(elm_id); data.copy(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_FILE)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// MESH
/******************************************************************************/
Bool EditorInterface::getMesh(C UID &elm_id, Mesh &mesh, Matrix *matrix)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_MESH).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_MESH)
      {
         if(f.getBool())
         {
            if(matrix)f>>*matrix;else f.skip(SIZE(*matrix));
            if(!f.left()) // object may exist, but its mesh may not be set yet, in that case OK will be true, but no data available
            {
               mesh.del();
               return true;
            }
            if(mesh.load(f))
            {
               mesh.setAutoTanBin().setRender();
               return true;
            }
         }
         goto fail;
      }
      disconnect();
   }
fail:
   mesh.del(); return !elm_id.valid();
}
Bool EditorInterface::setMesh(C UID &elm_id, C Mesh &mesh)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_MESH).putUID(elm_id); mesh.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_MESH)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// SKELETON
/******************************************************************************/
Bool EditorInterface::getSkeleton(C UID &elm_id, Skeleton &skel)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_SKEL).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_SKEL)
      {
         if(f.getBool())
         {
            if(!f.left()) // object may exist, but its skeleton may not be set yet, in that case OK will be true, but no data available
            {
               skel.del();
               return true;
            }
            if(skel.load(f))return true;
         }
         goto fail;
      }
      disconnect();
   }
fail:
   skel.del(); return !elm_id.valid();
}
Bool EditorInterface::getSkeletonSlots(C UID &elm_id, MemPtr<SkeletonSlot> skel_slots)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_SKEL_SLOTS).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_SKEL_SLOTS)
      {
         if(f.getBool())
         {
            if(!f.left()) // object may exist, but its skeleton may not be set yet, in that case OK will be true, but no data available
            {
               skel_slots.clear();
               return true;
            }
            if(skel_slots.load(f))return true;
         }
         goto fail;
      }
      disconnect();
   }
fail:
   skel_slots.clear(); return !elm_id.valid();
}
Bool EditorInterface::setSkeletonSlots(C UID &elm_id, C CMemPtr<SkeletonSlot> &skel_slots)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_SKEL_SLOTS).putUID(elm_id); skel_slots.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_SKEL_SLOTS)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// MATERIAL
/******************************************************************************/
UID EditorInterface::curMaterial()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_MTRL_CUR).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_MTRL_CUR)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
Bool EditorInterface::curMaterial(C UID &elm_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_MTRL_CUR).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_MTRL_CUR)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::getMaterial(C UID &elm_id, Material &mtrl)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_MTRL).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_MTRL)
      {
         if(f.getBool())return mtrl.load(f);
         goto fail;
      }
      disconnect();
   }
fail:
   mtrl.reset(); return !elm_id.valid();
}
Bool EditorInterface::setMaterial(C UID &elm_id, C Material &mtrl, Bool reload_textures, Bool adjust_params)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_MTRL).putUID(elm_id).putByte(reload_textures*1 | adjust_params*2); mtrl.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(reload_textures ? CLIENT_WAIT_TIME_LONG : CLIENT_WAIT_TIME)) // reloading textures may take a long time
      if(f.getByte()==EI_SET_MTRL)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::reloadMaterialTextures(C UID &elm_id, bool base, bool detail, bool macro, bool emissive)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_RLD_MTRL_TEX).putUID(elm_id).putByte(base*1 | detail*2 | macro*4 | emissive*8); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME_LONG)) // reloading textures may take a long time
      if(f.getByte()==EI_RLD_MTRL_TEX)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::mulMaterialTextureByColor(C UID &elm_id)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_MUL_MTRL_TEX_COL).putUID(elm_id); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME_LONG)) // reloading textures may take a long time
      if(f.getByte()==EI_MUL_MTRL_TEX_COL)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// ANIMATION
/******************************************************************************/
UID EditorInterface::curAnimation()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_ANIM_CUR).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_ANIM_CUR)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
Bool EditorInterface::curAnimation(C UID &anim_elm_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ANIM_CUR).putUID(anim_elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ANIM_CUR)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::getAnimation(C UID &anim_elm_id, Animation &anim)
{
   if(anim_elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_ANIM).putUID(anim_elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_ANIM)
      {
         if(f.getBool())return anim.load(f);
         goto fail;
      }
      disconnect();
   }
fail:
   anim.del(); return !anim_elm_id.valid();
}
Bool EditorInterface::setAnimation(C UID &anim_elm_id, C Animation &anim)
{
   if(anim_elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ANIM).putUID(anim_elm_id); anim.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ANIM)return f.getBool();
      disconnect();
   }
   return false;
}
UID EditorInterface::animationSkel(C UID &anim_elm_id)
{
   if(anim_elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_ANIM_SKEL).putUID(anim_elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_ANIM_SKEL)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
/******************************************************************************/
// PHYS
/******************************************************************************/
Bool EditorInterface::getPhys(C UID &elm_id, PhysBody &phys)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_PHYS).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_PHYS)
      {
         if(f.getBool())
         {
            if(!f.left()) // object may exist, but its phys may not be set yet, in that case OK will be true, but no data available
            {
               phys.del();
               return true;
            }
            if(phys.load(f))
            {
               return true;
            }
         }
         goto fail;
      }
      disconnect();
   }
fail:
   phys.del(); return !elm_id.valid();
}
Bool EditorInterface::setPhys(C UID &elm_id, C PhysBody &phys)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_PHYS).putUID(elm_id); phys.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_PHYS)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// OBJECT
/******************************************************************************/
UID EditorInterface::curObject()
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_OBJ_CUR).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_OBJ_CUR)return f.getUID();
      disconnect();
   }
   return UIDZero;
}
Bool EditorInterface::curObject(C UID &elm_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_OBJ_CUR).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_OBJ_CUR)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::getObject(C UID &elm_id, ObjData &obj, Bool include_removed_params)
{
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_GET_OBJ).putUID(elm_id).putBool(include_removed_params).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_GET_OBJ)
      {
         if(f.getBool())return obj.load(f);
         goto fail;
      }
      disconnect();
   }
fail:
   obj.reset(); return !elm_id.valid();
}
Bool EditorInterface::modifyObject(C UID &elm_id, C CMemPtr<ObjChange> &changes)
{
   if(!changes.elms())return true;
   if(elm_id.valid() && connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_MODIFY_OBJ).putUID(elm_id); changes.save(f); f.pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_MODIFY_OBJ)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// APPLICATION
/******************************************************************************/
Bool EditorInterface::activeApp(C UID &elm_id)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_SET_ACTIVE_APP).putUID(elm_id).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_SET_ACTIVE_APP)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::exportApp(EXPORT_MODE mode, Bool data)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_EXPORT_APP).putByte(mode).putBool(data).pos(0);
      if(_conn.send(f))
      if(_conn.receive(data ? CLIENT_WAIT_TIME_PUBLISH : CLIENT_WAIT_TIME))
      if(f.getByte()==EI_EXPORT_APP)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// BUILD SETTINGS
/******************************************************************************/
Bool EditorInterface::buildDebug(Bool debug)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_BUILD_DEBUG).putBool(debug).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_BUILD_DEBUG)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::build32Bit(Bool bit32)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_BUILD_32BIT).putBool(bit32).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_BUILD_32BIT)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::buildAPI(Byte api)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_BUILD_API).putByte(api).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_BUILD_API)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::buildExe(EXE_TYPE type)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_BUILD_EXE).putByte(type).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_BUILD_EXE)return f.getBool();
      disconnect();
   }
   return false;
}
Bool EditorInterface::buildPaths(Bool relative)
{
   if(connected())
   {
      File &f=_conn.data.reset(); f.putByte(EI_BUILD_PATHS).putBool(relative).pos(0);
      if(_conn.send(f))
      if(_conn.receive(CLIENT_WAIT_TIME))
      if(f.getByte()==EI_BUILD_PATHS)return f.getBool();
      disconnect();
   }
   return false;
}
/******************************************************************************/
// SERVER
/******************************************************************************/
Bool EditorServer::Client::update()
{
   if(!super::update())return false;

   if(!connection_version_ok) // if not verified
      if(connection.receive(0)) // check for it
   {
      if(!connection.address().thisDevice())return false; // allow only from this device

      File &f=connection.data;
      Byte cmd =f.getByte();
      if(  cmd==EI_VERSION_CHECK)
      if(Equal(f.getStr(), EI_STR, true))
      {
         connection_version_ok=(f.decUIntV()==EI_VER);
         f.reset().putByte(EI_VERSION_CHECK).putStr(EI_STR).cmpUIntV(EI_VER).pos(0);
         connection.tcpNoDelay(true);
         connection.send(f);
      }
      if(!connection_version_ok)return false; // if received something that wasn't correct version match then remove
   }
   return true;
}
/******************************************************************************/
static Int CompareElm(C Elm &elm, C UID &id) {return Compare(elm.id, id);}

Elm* FindElm(MemPtr<Elm> elms, C UID &elm_id               ) {return elms.binaryFind(elm_id, CompareElm);}
Elm* FindElm(MemPtr<Elm> elms, C UID &elm_id, ELM_TYPE type) {if(Elm *elm=FindElm(elms, elm_id))if(elm->type==type)return elm; return null;}

static Elm* FindChild(MemPtr<Elm> elms, C UID &parent_id, ELM_TYPE type)
{
   REPA(elms)
   {
      Elm &child=elms[i]; if(child.parent_id==parent_id && child.type==type)return &child;
   }
   return null;
}

Elm* FindElm(MemPtr<Elm> elms, C Str &full_name, ELM_TYPE type)
{
   REPA(elms)
   {
      Elm &elm=elms[i]; if(EqualPath(elm.full_name, full_name))
      {
         if(elm.type==type)return &elm;
         switch(elm.type)
         {
            case ELM_MESH: if(                  type==ELM_SKEL || type==ELM_PHYS)if(Elm *child=FindChild(elms, elm.id, type))return child; break; // allow      SKEL and PHYS to be children of MESH
            case ELM_OBJ : if(type==ELM_MESH || type==ELM_SKEL || type==ELM_PHYS)REPA(elms)                                                       // allow MESH SKEL and PHYS to be children of OBJ
            {
               Elm &child=elms[i]; if(child.parent_id==elm.id)
               {
                  if(child.type==type    )return &child;
                  if(child.type==ELM_MESH)if(type==ELM_SKEL || type==ELM_PHYS)if(Elm *sub=FindChild(elms, child.id, type))return sub;
               }
            }break;
         }
      }
   }
   return null;
}

Bool ContainsElm(MemPtr<Elm> elms, C UID &parent_id, C UID &child_id)
{
   if(parent_id.valid()) // if parent valid
   {
      Memt<UID> ids;
      for(C UID *id=&child_id; id->valid() && ids.binaryInclude(*id); ) // if valid and not checked yet
      {
         if(*id==parent_id)return true;
         if(Elm *elm=FindElm(elms, *id))id=&elm->parent_id;else break;
      }
   }
   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
