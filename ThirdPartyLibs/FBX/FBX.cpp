/******************************************************************************

   Node is a bone when:
      -node type is FbxNodeAttribute::eSkeleton
      -node has any skinning (vertexes attached to the node)
      -node has animation keyframes

/******************************************************************************/
#ifndef EE_PRIVATE
   #define EE_PRIVATE 1
#endif
#include "../../Engine/H/Engine.h"

#include "../begin.h"

//#define FBXSDK_SHARED
#include <fbxsdk.h>

#include "../end.h"

#define TRIANGULATE 0 // don't do this, because we want to preserve quads
#define KEY_SAFETY  0 // not needed since time's are represented using integer values, and they will always be filled out
/******************************************************************************/
namespace EE{
static SyncLock Lock; // FBX SDK is not thread-safe and crashes upon importing multiple files at the same time
/******************************************************************************/
struct FBX
{
   static Color   COLOR (C FbxColor   &c) {return Color(Vec4(c.mRed, c.mGreen, c.mBlue, c.mAlpha));}
   static Vec2    VEC   (C FbxVector2 &v) {return Vec2(v[0], v[1]            );}
   static Vec4    VEC   (C FbxVector4 &v) {return Vec4(v[0], v[1], v[2], v[3]);}
   static MatrixD MATRIX(C FbxAMatrix &f)
   {
      MatrixD m;
      m.x  .set(f.Get(0, 0), f.Get(0, 1), f.Get(0, 2));
      m.y  .set(f.Get(1, 0), f.Get(1, 1), f.Get(1, 2));
      m.z  .set(f.Get(2, 0), f.Get(2, 1), f.Get(2, 2));
      m.pos.set(f.Get(3, 0), f.Get(3, 1), f.Get(3, 2));
   #if DEBUG
      if(f.Get(0, 3)!=0 || f.Get(1, 3)!=0 || f.Get(2, 3)!=0 || f.Get(3, 3)!=1)Exit("FBX Matrix");
   #endif
      return m;
   }
   static Int CompareTime(C FbxTime &a, C FbxTime &b) {if(a<b)return -1; if(a>b)return +1; return 0;}

   struct Node
   {
      Node       *parent=null;
      Memc<Node*> children;
      FbxNode    *node=null;
      Bool        dummy=false, bone=false, mesh=false, has_skin=false,
                  has_bones=false; // if this or any children is a bone
      Str         full_name, ee_name;
      MatrixD     local, global;
      Int         bone_index=-1, // points to the bone index in the skeleton
          nearest_bone_index=-1, // this points to the nearest bone index in the skeleton (for example, if this node doesn't have a corresponding bone in the skeleton, then it will point to the bone index of one of its parents, but NOT children)
            xskel_node_index=-1; // points to 'XSkeleton.nodes'

      Node()
      {
         local .identity();
         global.identity();
      }

      Bool hasAnimFast(FbxAnimLayer *anim_layer)C
      {
         if(FbxAnimCurve *curve=node->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X)){Flt v=curve->EvaluateIndex(0); REP(curve->KeyGetCount())if(!Equal(curve->EvaluateIndex(i), v))return true;}
         if(FbxAnimCurve *curve=node->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y)){Flt v=curve->EvaluateIndex(0); REP(curve->KeyGetCount())if(!Equal(curve->EvaluateIndex(i), v))return true;}
         if(FbxAnimCurve *curve=node->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z)){Flt v=curve->EvaluateIndex(0); REP(curve->KeyGetCount())if(!Equal(curve->EvaluateIndex(i), v))return true;}

         if(FbxAnimCurve *curve=node->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X)){Flt v=curve->EvaluateIndex(0); REP(curve->KeyGetCount())if(!Equal(curve->EvaluateIndex(i), v))return true;}
         if(FbxAnimCurve *curve=node->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y)){Flt v=curve->EvaluateIndex(0); REP(curve->KeyGetCount())if(!Equal(curve->EvaluateIndex(i), v))return true;}
         if(FbxAnimCurve *curve=node->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z)){Flt v=curve->EvaluateIndex(0); REP(curve->KeyGetCount())if(!Equal(curve->EvaluateIndex(i), v))return true;}

         if(FbxAnimCurve *curve=node->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X)){Flt v=curve->EvaluateIndex(0); REP(curve->KeyGetCount())if(!Equal(curve->EvaluateIndex(i), v))return true;}
         if(FbxAnimCurve *curve=node->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y)){Flt v=curve->EvaluateIndex(0); REP(curve->KeyGetCount())if(!Equal(curve->EvaluateIndex(i), v))return true;}
         if(FbxAnimCurve *curve=node->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z)){Flt v=curve->EvaluateIndex(0); REP(curve->KeyGetCount())if(!Equal(curve->EvaluateIndex(i), v))return true;}

         return false;
      }
      Bool hasAnimFast()C // !! assumes that 'SetCurrentAnimationStack' was already called !!
      {
         return !Equal(local, MATRIX(node->EvaluateLocalTransform(0)), (Dbl)EPS_ANIM_ANGLE, EPSD); // use 'EPSD' instead of 'EPS_ANIM_POS' because mesh can be scaled
      }
      Bool hasAnim(FbxAnimLayer *anim_layer)C // !! assumes that 'SetCurrentAnimationStack' was already called !!
      {
         return hasAnimFast(anim_layer) || hasAnimFast();
      }
      Bool hasAnim(FBX &fbx)C
      {
         REP(fbx.scene->GetSrcObjectCount<FbxAnimStack>())
            if(FbxAnimStack *anim_stack=fbx.scene->GetSrcObject<FbxAnimStack>(i))
         {
            // first do a fast check if any of the keyframes are different
            Int anim_layers=anim_stack->GetMemberCount<FbxAnimLayer>();
            REPD(layer, anim_layers)if(FbxAnimLayer *anim_layer=anim_stack->GetMember<FbxAnimLayer>(layer))if(hasAnimFast(anim_layer))return true;

            // now check if the first keyframe is not as local transform
            fbx.scene->SetCurrentAnimationStack(anim_stack);
            if(hasAnimFast())return true;
         }
         return false;
      }

    /*Bool setNodesAsBoneIfAnimAffectsBoneOrMesh(FBX &fbx)
      {
         Bool child_is_bone_or_mesh=false; FREPA(children)child_is_bone_or_mesh|=children[i]->setNodesAsBoneIfAnimAffectsBoneOrMesh(fbx);
         child_is_bone_or_mesh|=mesh; // include from self
         if(!bone && child_is_bone_or_mesh && hasAnim(fbx))bone=true;
         return child_is_bone_or_mesh || bone;
      }*/
      void adjustBoneIndex(Int bone_index)
      {
         if(bone_index>=0xFF)bone_index=-1;
          T.bone_index=bone_index;
         if(bone_index<0)bone=false; // if we're clearing bone index, then it means we don't want this as a bone
      }
      void setBone(OrientP &bone)C
      {
         bone.pos =global.pos;
         bone.dir =global.x;
         bone.perp=global.y;
         bone.fix();
      }
   };

   FbxManager               *manager        =null;
   FbxScene                 *scene          =null;
   FbxImporter              *importer       =null;
   FbxGlobalSettings        *global_settings=null;
   Int                       sdk_major=0, sdk_minor=0, sdk_revision=0;
   Mems<Node>                nodes;
   Memc<FbxSurfaceMaterial*> engine_mtrl_to_fbx_mtrl; // this will point from Engine Material 'materials' container index to FBX Material pointer
   Flt                       scale=1.0f;
   Str                       app_name;

   Int   findNodeI(FbxNode *node)C {REPA(nodes)if(nodes[i].node==node)return i; return -1;}
   Node* findNode (FbxNode *node)  {return nodes.addr(findNodeI(node));}

   Int findMaterial(FbxSurfaceMaterial *mtrl)C {return engine_mtrl_to_fbx_mtrl.find(mtrl);}

  ~FBX()
   {
      SyncLocker locker(Lock);
      if(importer){importer->Destroy(); importer=null;}
      if(scene   ){scene   ->Destroy(); scene   =null;}
      if(manager ){manager ->Destroy(); manager =null;}
   }
   Bool init()
   {
      SyncLocker locker(Lock);
      FbxManager::GetFileFormatVersion(sdk_major, sdk_minor, sdk_revision); // get versions
      if(!manager )manager =FbxManager ::Create(           ); if(!manager )return false;
      if(!scene   )scene   =FbxScene   ::Create(manager, ""); if(!scene   )return false;
      if(!importer)importer=FbxImporter::Create(manager, ""); if(!importer)return false;
      return true;
   }
   Bool load(C Str &name, Bool all_nodes_as_bones, C Str8 &password)
   {
      SyncLocker locker(Lock);
      if(importer)
      {
         Bool status=importer->Initialize((WINDOWS ? UTF8 : UnixPathUTF8)(name), -1, manager->GetIOSettings());
         Int  file_major, file_minor, file_revision; importer->GetFileVersion(file_major, file_minor, file_revision);
         if(status)
         {
            status=importer->Import(scene);
            if(!status && importer->GetStatus()==FbxStatus::ePasswordError && password.is())
            {
               if(FbxIOSettings *io_settings=importer->GetIOSettings())
               {
                  io_settings->SetStringProp(IMP_FBX_PASSWORD       , password());
                  io_settings->SetBoolProp  (IMP_FBX_PASSWORD_ENABLE, true      );
                  status=importer->Import(scene);
                  if(!status && importer->GetStatus()==FbxStatus::ePasswordError){} // wrong password
               }
            }
            if(status)
            {
               global_settings=&scene->GetGlobalSettings(); if(!global_settings)return false;

               if(FbxDocumentInfo *doc_info=scene->GetDocumentInfo())
               {
                  app_name=doc_info->LastSaved_ApplicationName.Get();
               }

               // Convert Axis System
               FbxAxisSystem scene_axis_system=global_settings->GetAxisSystem(),
                         //desired_axis_system(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem:: eLeftHanded); should be this (the same as FbxAxisSystem::eDirectX), however it's broken (Y axis gets flipped somehow)
                           desired_axis_system(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded); // instead use 'eRightHanded' and call 'mirrorX'
               if(scene_axis_system!=desired_axis_system)desired_axis_system.ConvertScene(scene);

               // Convert Unit System
               FbxSystemUnit scene_system_unit=global_settings->GetSystemUnit(),
                           desired_system_unit=FbxSystemUnit::m; // meters
               if(scene_system_unit!=desired_system_unit)
               #if 0 // this is broken
                  desired_system_unit.ConvertScene(scene);
               #else
                  scale=scene_system_unit.GetScaleFactor()/desired_system_unit.GetScaleFactor();
               #endif

               // set nodes
               nodes.setNum(scene->GetNodeCount()); // create all nodes up-front !! this is important because we're using 'Mems' !!
                REPAO(nodes).node=scene->GetNode(i); // set node link
               FREPA (nodes)
               {
                  Node &node=nodes[i];
                  node.ee_name=node.full_name=FromUTF8(node.node->GetName());

                  // type
                  if(i || node.full_name!="RootNode") // all FBX have a dummy "RootNode" at 0 index, always ignore it because there's a limit of 255 bones
                  {
                     if(FbxNodeAttribute *attrib=node.node->GetNodeAttribute()) // this will be null for the identity "RootNode", but can also be null for some animations
                        switch(attrib->GetAttributeType())
                     {
                        case FbxNodeAttribute::eNull    : node.dummy=true; node.bone=all_nodes_as_bones; break;
                        case FbxNodeAttribute::eSkeleton: node.bone =true; break;
                        case FbxNodeAttribute::eMesh    : node.mesh =true; break;
                     }else
                     if(all_nodes_as_bones)node.bone=true; // this is needed because some animations can have all node attributes null
                  }

                  // matrix
                  node.local =MATRIX(node.node->EvaluateLocalTransform ());
                  node.global=MATRIX(node.node->EvaluateGlobalTransform());

                  // link with parent
                  if(node.parent=findNode(node.node->GetParent()))node.parent->children.add(&node);
               }

               return true;
            }
         }
      }
      return false;
   }
   void ignoreBones(C MemPtr<Str> &bone_names)
   {
      REPA(nodes)
      {
         Node &node=nodes[i]; if(node.bone)
         {
            REPA(bone_names)if(node.full_name==bone_names[i])
            {
               node.bone=false;
               break;
            }
         }
      }
   }
   void makeBoneNamesUnique()
   {
      FREPA(nodes)
      {
         Node &node=nodes[i]; if(node.bone)
         {
            if(!node.ee_name.is())node.ee_name="Root"; // we don't allow empty names because 'findBone' methods may skip searching if the name parameter is empty
         again:
            REPD(j, i)
            {
             C Node &test=nodes[j]; if(test.bone && node.ee_name==test.ee_name)
               {
                  Int  index=1;
                  Char separator='#';
                  REPA(node.ee_name)if(node.ee_name[i]==separator) // find if this name already has a separator
                     {index=TextInt(node.ee_name()+i+1)+1; node.ee_name.clip(i); break;} // get the index value, increase by one, and remove the separator
                  node.ee_name+=separator;
                  node.ee_name+=index;
                  goto again;
               }
            }
         }
      }
   }
   void shortenBoneNames(C MemPtr<Str> &start)
   {
      REPA(nodes)
      {
         Node &node=nodes[i]; if(node.bone)
         {
            Str &name=node.ee_name;
            REPA(start)name=SkipStart(name, start[i]);
         }
      }
   }
   void shortenBoneNames()
   {
      Int max_name_length=MEMBER_ELMS(SkelBone, name)-1, name_length=0;
      REPA(nodes)
      {
       C Node &node=nodes[i]; if(node.bone)MAX(name_length, node.ee_name.length());
      }
      if(name_length>max_name_length) // if name length exceeds allowed limit, then shorten names by removing the shared start
      {
         REPA(nodes)
         {
          C Node &node_i=nodes[i]; if(node_i.bone)
            {
             C Str &name_i=node_i.ee_name;
               REPD(j, i)
               {
                C Node &node_j=nodes[j]; if(node_j.bone)
                  {
                   C Str &name_j=node_j.ee_name;
                     FREPA(name_i)if(name_i[i]!=name_j[i]){MIN(name_length, i); break;}
                  }
               }
            }
         }
         REPA(nodes)
         {
            Str &name=nodes[i].ee_name;
            if(name_length)name.remove(0, name_length+(name[name_length]==' ')); // if this name has a space after shared start, then remove that space too
            // even after removing shared start, bone name can still be too long
            if(name.length()>max_name_length)name.clip(max_name_length-1-2); // leave room for 1 char separator + 2 char index
         }
         makeBoneNamesUnique(); // we've changed names, so we need to make sure that they are unique
      }
   }
   void setNodesAsBoneFromSkin()
   {
      FREPA(nodes)
         if(nodes[i].mesh)
            if(FbxMesh *mesh=nodes[i].node->GetMesh()) // access fbx mesh
      {
         Int polys=mesh->GetPolygonCount();
         if( polys>0)
            REPD(s, mesh->GetDeformerCount(FbxDeformer::eSkin))if(FbxSkin    *skin   =(FbxSkin*)mesh->GetDeformer(s, FbxDeformer::eSkin))
            REPD(b, skin-> GetClusterCount(                  ))if(FbxCluster *cluster=skin->GetCluster(b))
         {
            Int index_count=cluster->GetControlPointIndicesCount();
            if( index_count>0)
               if(Node *bone=findNode(cluster->GetLink()))
            {
               bone->bone    =true;
               bone->has_skin=true;
            }
         }
      }
   }
 /*void setNodeHasAnim()
   {
      FREP(scene->GetSrcObjectCount<FbxAnimStack>())
         if(FbxAnimStack *anim_stack=scene->GetSrcObject<FbxAnimStack>(i))
      {
         Int anim_layers=anim_stack->GetMemberCount<FbxAnimLayer>();
         REPD(layer, anim_layers)
            if(FbxAnimLayer *anim_layer=anim_stack->GetMember<FbxAnimLayer>(layer))
         {
            FREPAD(n, nodes)
            {
               Node &node=nodes[n];
               if(  !node.has_anim)
               {
                  Bool has_pos=false;
                  if(FbxAnimCurve *curve=node.node->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X))if(curve->KeyGetCount())has_pos=true;
                  if(FbxAnimCurve *curve=node.node->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y))if(curve->KeyGetCount())has_pos=true;
                  if(FbxAnimCurve *curve=node.node->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z))if(curve->KeyGetCount())has_pos=true;

                  Bool has_rot=false;
                  if(FbxAnimCurve *curve=node.node->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X))if(curve->KeyGetCount())has_rot=true;
                  if(FbxAnimCurve *curve=node.node->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y))if(curve->KeyGetCount())has_rot=true;
                  if(FbxAnimCurve *curve=node.node->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z))if(curve->KeyGetCount())has_rot=true;

                  Bool has_scale=false;
                  if(FbxAnimCurve *curve=node.node->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X))if(curve->KeyGetCount())has_scale=true;
                  if(FbxAnimCurve *curve=node.node->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y))if(curve->KeyGetCount())has_scale=true;
                  if(FbxAnimCurve *curve=node.node->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z))if(curve->KeyGetCount())has_scale=true;

                  if(has_pos || has_rot || has_scale)node.has_anim=true;
               }
            }
         }
      }
   }*/
   void setNodesAsBoneFromAnim()
   {
      FREPA(nodes)
      {
         Node &node=nodes[i];
         if(!node.bone && node.hasAnim(T))node.bone=true;
      }
      scene->SetCurrentAnimationStack(null); // reset context which may have been changed in 'hasAnim'
   }

   static Str TextureFile(C FbxTexture &texture)
   {
      if(C FbxFileTexture *file_texture=FbxCast<FbxFileTexture>(&texture))
      {
         Str file=FromUTF8(file_texture->GetFileName());
         if( file.is())return file;
      }else
      if(C FbxLayeredTexture *layered_texture=FbxCast<FbxLayeredTexture>(&texture))
      {
         Str file=TextureFile(SCAST(C FbxObject, *layered_texture));
         if( file.is())return file;
      }
      return S;
   }
   static Str TextureFile(C FbxObject &obj)
   {
      Int  textures=obj.GetSrcObjectCount<FbxTexture>();
      FREP(textures)
         if(FbxTexture *texture=obj.GetSrcObject<FbxTexture>(i))
      {
         Str file=TextureFile(*texture);
         if( file.is())return file;
      }
      return S;
   }
   static Str TextureFile(C FbxProperty &prop)
   {
      Int  textures=prop.GetSrcObjectCount<FbxTexture>();
      FREP(textures)
         if(FbxTexture *texture=prop.GetSrcObject<FbxTexture>(i))
      {
         Str file=TextureFile(*texture);
         if( file.is())return file;
      }
      return S;
   }
   static Str TextureFile(C FbxSurfaceMaterial &mtrl, char const *type)
   {
      return TextureFile(mtrl.FindProperty(type));
   }
   void set(MemPtr<XMaterial> materials)
   {
      if(materials)
      {
         Bool flip_normal_y=Contains(app_name, "Maya", false, WHOLE_WORD_STRICT);
         Int  mtrls=scene->GetMaterialCount();
         Memt<Bool> duplicate_name; // there may be multiple materials with the same name, we need to make them unique
         FREP(mtrls)if(FbxSurfaceMaterial *mtrl=scene->GetMaterial(i))
         {
            XMaterial *xm=null;

            // params
            if(FbxSurfacePhong *phong=FbxCast<FbxSurfacePhong>(mtrl))
            {
               xm=&materials.New(); engine_mtrl_to_fbx_mtrl.add(mtrl);
            /* Don't set anything because artists never set those parameters, and they usually should be tweaked in-engine anyway
               FbxDouble3 amb  =phong->Ambient           ; //xm->emissive  .set(amb[0], amb[1], amb[2]);
               FbxDouble3 dif  =phong->Diffuse           ; xm->color.xyz.set(dif[0], dif[1], dif[2]);
               FbxDouble  alpha=phong->TransparencyFactor; xm->color.w=1-alpha;
               FbxDouble3 spec =phong->Specular          ;
               FbxDouble  shin =phong->Shininess         ;
               FbxDouble3 glow =phong->Emissive          ; xm->emissive.set(glow[0], glow[1], glow[2]);
               FbxDouble  refl =phong->ReflectionFactor  ;
               FbxDouble  bump =phong->DisplacementFactor;
               FbxDouble  rough=phong->BumpFactor        ;*/
            }else
            if(FbxSurfaceLambert *lambert=FbxCast<FbxSurfaceLambert>(mtrl))
            {
               xm=&materials.New(); engine_mtrl_to_fbx_mtrl.add(mtrl);
            /* Don't set anything because artists never set those parameters, and they usually should be tweaked in-engine anyway
               FbxDouble3 amb  =lambert->Ambient           ; //xm->emissive  .set(amb[0], amb[1], amb[2]);
               FbxDouble3 dif  =lambert->Diffuse           ; xm->color.xyz.set(dif[0], dif[1], dif[2]);
               FbxDouble  alpha=lambert->TransparencyFactor; xm->color.w=1-alpha;
               FbxDouble3 glow =lambert->Emissive          ; xm->emissive.set(glow[0], glow[1], glow[2]);
               FbxDouble  bump =lambert->DisplacementFactor; //xm->bump=bump;
               FbxDouble  rough=lambert->BumpFactor        ; //xm->roughness=rough;*/
            }else continue;

            // name & props
            xm->name=FromUTF8(mtrl->GetName());
            REP(materials.elms()-1)if(materials[i].name==xm->name) // if another material has the same name
            { // mark them both as duplicates
               duplicate_name(                 i)=true; // mark other i-th material
               duplicate_name(materials.elms()-1)=true; // mark this 'xm'  material
               break; // stop as there's no need to look any further (other duplicates for this name should have already been detected)
            }
            xm->flip_normal_y=flip_normal_y;

            // textures
         #if 1
            Str tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sDiffuseFactor     ); if(tex.is())xm-> color_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sDiffuse           ); if(tex.is())xm-> color_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sTransparencyFactor); if(tex.is())xm-> alpha_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sTransparentColor  ); if(tex.is())xm-> alpha_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sShininess         ); if(tex.is())xm->smooth_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sSpecularFactor    ); if(tex.is())xm->smooth_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sSpecular          ); if(tex.is())xm->smooth_map=tex;
          //tex=TextureFile(*mtrl, FbxSurfaceMaterial::sAmbientFactor     ); if(tex.is())xm-> light_map=tex;
          //tex=TextureFile(*mtrl, FbxSurfaceMaterial::sAmbient           ); if(tex.is())xm-> light_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sReflectionFactor  ); if(tex.is())xm-> metal_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sReflection        ); if(tex.is())xm-> metal_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sDisplacementFactor); if(tex.is())xm->  bump_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sDisplacementColor ); if(tex.is())xm->  bump_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sBumpFactor        ); if(tex.is())xm->normal_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sBump              ); if(tex.is())xm->normal_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sNormalMap         ); if(tex.is())xm->normal_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sEmissive          ); if(tex.is())xm->  glow_map=tex;
            tex=TextureFile(*mtrl, FbxSurfaceMaterial::sEmissiveFactor    ); if(tex.is())xm->  glow_map=tex;
         #else
            Int TextureIndex;
            FOR_EACH_TEXTURE(TextureIndex)
            {
               FbxProperty Property=mtrl->FindProperty(FbxLayerElement::TEXTURE_CHANNEL_NAMES[TextureIndex]);
               if(Property.IsValid())
               {
                  Int TextureCount=Property.GetSrcObjectCount(FbxTexture::ClassId);
                  for(Int j=0; j<TextureCount; ++j)
                  {
                     /*if(FbxLayeredTexture *LayeredTexture=FbxCast<FbxLayeredTexture>(Property.GetSrcObject(FbxLayeredTexture::ClassId, j)))
                     {
                        // layered texture
                     }else*/
                     if(FbxTexture *Texture=FbxCast<FbxTexture>(Property.GetSrcObject(FbxTexture::ClassId, j)))
                        if(FbxFileTexture *FileTexture=FbxCast<FbxFileTexture>(Texture))
                     {
                        Str file=FromUTF8(FileTexture->GetFileName()),
                            type=FromUTF8(Property.GetName());
                        if( type=="DiffuseColor"  )xm->   color_map=file;else
                        if( type=="SpecularColor" )xm->specular_map=file;else
                        if( type=="SpecularFactor")xm->specular_map=file;else
                        if( type=="NormalMap"     )xm->  normal_map=file;
                     }
                  }
               }
            }
         #endif
         }
         REPA(duplicate_name)if(duplicate_name[i])materials[i].name+=S+'\\'+(ULong)engine_mtrl_to_fbx_mtrl[i]->GetUniqueID(); // append the name with materials Unique ID
      }
   }
   void boneRemap(C MemPtrN<Byte, 256> &old_to_new)
   {
      REPA(nodes)
      {
         Node &node=nodes[i]; node.adjustBoneIndex(InRange(node.bone_index, old_to_new) ? old_to_new[node.bone_index] : 0xFF);
      }
   }
   void set(Skeleton *skeleton, XSkeleton *xskeleton)
   {
      if(skeleton)
      {
         MemtN<Byte, 256> old_to_new;

         // create bones
         FREPA(nodes)
         {
            Node &node=nodes[i]; if(node.bone)
            {
               node.bone_index=skeleton->bones.elms();
               SkelBone  &sbon=skeleton->bones.New ();
               Set(sbon.name, node.ee_name);
               node.setBone(sbon);
            }
         }

         // set parents
         FREPA(nodes)
         {
            Node &node=nodes[i];
            if(InRange(node.bone_index, skeleton->bones)) // if this node is a bone
               for(Node *parent=node.parent; parent; parent=parent->parent) // iterate all parents
                  if(parent->bone) // if the parent is a bone
            {
               skeleton->bones[node.bone_index].parent=parent->bone_index; // set skel bone parent
               break; // stop searching
            }
         }

         // sort bones and set children
         skeleton->sortBones(old_to_new); // 'sortBones' needs to be called before any 'SetSkin'
         boneRemap(old_to_new);

         // set bone lengths, before removing nub-bones (so we can use the nubs for proper lengths of their parents)
         skeleton->setBoneLengths();

         // after bone lengths, remove nub-bones
         FREPA(nodes)
         {
            Node &node=nodes[i];
            if(node.bone && !node.children.elms() && !node.has_skin && Ends(node.full_name, "Nub"))
            {
               skeleton->removeBone(node.bone_index, old_to_new);
               node.adjustBoneIndex(-1); // !! adjust manually after remap, because remap will point to the parent of removed bone instead, this is important so that we won't apply this NUB bone animations to its parent !!
               boneRemap(old_to_new);
            }
         }

         // after skeleton bones are ready, link nearest bone and setup xskeleton
         FREPA(nodes)
         {
            Node &node=nodes[i];
            for(Node *parent=&node; parent; parent=parent->parent) // iterate all parents, starting from this node inclusive
            {
               if(parent->bone                 ){node.nearest_bone_index=parent->        bone_index; break;}
               if(parent->nearest_bone_index>=0){node.nearest_bone_index=parent->nearest_bone_index; break;}
            }
            if(node.bone)for(Node *parent=&node; parent && !parent->has_bones; parent=parent->parent)parent->has_bones=true; // if this node is a bone, then enable its and all of its parents 'has_bones'
         }
         if(xskeleton)
         {
            xskeleton->bones.setNum(skeleton->bones.elms());
            REPA(xskeleton->bones)
            {
               XSkeleton::Bone &xbone=xskeleton->bones[i];
              C SkeletonBone   &sbone= skeleton->bones[i];
               xbone.name=sbone.name;
               xbone.node=-1; // clear at start in case no nodes are linked with this bone (however shouldn't happen)
            }
            FREPA(nodes) // list in order
            {
               Node &node=nodes[i]; if(node.has_bones && (node.bone || node.dummy)) // in XSkeleton store only nodes that have any bones (or their children have), AND store only bones/dummies (without MESH, CAMERA, .., "RootNode" root node which would cause issues because it's MatrixIdentity however because of Bones having swapped XZ and 'mirrorX' creating transformation that should be an identity but isn't)
               {
                  node.xskel_node_index =xskeleton->nodes.elms();
                  XSkeleton::Node &xnode=xskeleton->nodes.New();
                   node.setBone(xnode.orient_pos);
                  xnode.name  =node.full_name;
                  xnode.parent=-1; // clear at start in case no parent was found to be stored in xskel nodes 
               }
            }
            // after 'xskel_node_index' was set for all nodes
            REPA(nodes)
            {
               Node &node=nodes[i]; if(InRange(node.xskel_node_index, xskeleton->nodes))
               {
                  XSkeleton::Node &xnode=xskeleton->nodes[node.xskel_node_index];
                  for(Node *parent=node.parent; parent; parent=parent->parent){Int parent_xskel_node_index=parent->xskel_node_index; if(parent_xskel_node_index>=0){xnode.parent=parent_xskel_node_index; break;}} // find first parent that is stored inside xskel nodes
                  if(InRange(node.bone_index, xskeleton->bones))xskeleton->bones[node.bone_index].node=node.xskel_node_index; // link skel bone with this node
               }
            }
         }
      }
   }
   static FbxAMatrix GetPoseMatrix(C FbxPose &pose, int node_index)
   {
      FbxMatrix   matrix=pose.GetMatrix(node_index); 
      FbxAMatrix amatrix; SCAST(FbxDouble4x4, amatrix)=matrix;
      return     amatrix;
   }
   static FbxAMatrix GetGeometryOffset(FbxNode *node)
   {
      return FbxAMatrix(node->GetGeometricTranslation(FbxNode::eSourcePivot),
                        node->GetGeometricRotation   (FbxNode::eSourcePivot),
                        node->GetGeometricScaling    (FbxNode::eSourcePivot));
   }
   static FbxAMatrix GetGlobalPosition(FbxNode *node, const FbxTime &time=FBXSDK_TIME_INFINITE, FbxPose *pose=null, FbxAMatrix *parent_global_position=null)
   {
      if(pose)
      {
         Int node_index=pose->Find(node); if(node_index>=0)
         {
            // The bind pose is always a global matrix. If we have a rest pose, we need to check if it is stored in global or local space.
            if(pose->IsBindPose() || !pose->IsLocalMatrix(node_index))return GetPoseMatrix(*pose, node_index);

            // We have a local matrix, we need to convert it to a global space matrix.
            FbxAMatrix parent;
            if(parent_global_position)parent=*parent_global_position;else
            if(node->GetParent()     )parent= GetGlobalPosition(node->GetParent(), time, pose);

            FbxAMatrix local_position=GetPoseMatrix(*pose, node_index);
            return parent*local_position;
         }
      }

      // There is no pose entry for that node, get the current global position instead.
      // Ideally this would use parent global position and local position to compute the global position.
      // Unfortunately the equation "parent_global_position*local_position"
      // does not hold when inheritance type is other than "Parent" (RSrs).
      // To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.
      return node->EvaluateGlobalTransform(time);
   }
   void set(Mesh *mesh, Skeleton *skeleton, MemPtr<Int> part_material_index)
   {
      if(mesh)
      {
         MemtN<Int, 128> poly_vtx;
         FREPA(nodes)if(nodes[i].mesh)
         {
            Node &node=nodes[i];

            if(TRIANGULATE)FbxGeometryConverter(manager).Triangulate(node.node->GetNodeAttribute(), true); // triangulate
            if(FbxMesh *fbx_mesh=node.node->GetMesh()) // access fbx mesh
            {
               Int         control_points=fbx_mesh->GetControlPointsCount();
               FbxVector4 *control_point =fbx_mesh->GetControlPoints     ();
               Int         polys         =fbx_mesh->GetPolygonCount      ();
               if(polys>0)
               {
                  MeshBase base;
                  Int      texs=Mid(fbx_mesh->GetElementUVCount(), 0, 4); // Engine supports only up to 4

                  Bool one_material=true;
                  REP(fbx_mesh->GetElementMaterialCount())
                     if(FbxGeometryElementMaterial *mtrl=fbx_mesh->GetElementMaterial(i))
                     if(mtrl->GetMappingMode()==FbxGeometryElement::eByPolygon){one_material=false; break;}

                  MatrixD matrix_d=MATRIX(GetGeometryOffset(node.node))*node.global;
                  Matrix  matrix=matrix_d;
                  // mesh needs to be transformed by "VertexTransformMatrix*matrix"

                  // skin
                  MemtN< Matrix, 256       > bone_matrix;
                  Memc < Memc<IndexWeight> > vtx_skin;
                  VecB4                      vtx_matrix=0, vtx_blend(255, 0, 0, 0); // !! sum must be equal to 255 !! defaults used for vertexes with no skin specified
                  Bool                       force_skin=false, animate=false;
                  if(skeleton && skeleton->bones.elms())
                  {
                     REP(skeleton->bones.elms()+VIRTUAL_ROOT_BONE)bone_matrix.add(matrix);

                     Int skin_count=fbx_mesh->GetDeformerCount(FbxDeformer::eSkin);
                     FREPD(s, skin_count)if(FbxSkin *skin=(FbxSkin*)fbx_mesh->GetDeformer(s, FbxDeformer::eSkin))
                     {
                        Int bones=skin->GetClusterCount();
                        FREPD(b, bones)if(FbxCluster *cluster=skin->GetCluster(b))
                        {
                           if(Node *bone=findNode(cluster->GetLink()))
                           {
                              if(InRange(bone->nearest_bone_index, skeleton->bones)) // use 'nearest_bone_index' in case this node was disabled as a bone
                              {
                                 if(Int *indices=cluster->GetControlPointIndices())
                                 if(Dbl *weights=cluster->GetControlPointWeights())
                                 {
                                    Int index_count=cluster->GetControlPointIndicesCount(); FREP(index_count)
                                    {
                                       Int point =indices[i];
                                       Flt weight=weights[i];
                                       if(InRange(point, control_points))vtx_skin(point).New().set(bone->nearest_bone_index+VIRTUAL_ROOT_BONE, weight);
                                    }
                                 }
                              }
                              if(InRange(bone->bone_index                  , skeleton->bones)
                              && InRange(bone->bone_index+VIRTUAL_ROOT_BONE, bone_matrix))
                              {
                               /*FbxAMatrix temp1, temp2;
                                 MatrixD transform_matrix     =MATRIX(cluster->GetTransformMatrix    (temp1)),
                                         transform_link_matrix=MATRIX(cluster->GetTransformLinkMatrix(temp2));*/

                                 // matrixes that calculate conversion from mesh to bind pose (code taken from FBX SDK tutorials)
                                 FbxTime time=FBXSDK_TIME_INFINITE; FbxPose *pose=null;
                                 FbxAMatrix GlobalPosition=GetGlobalPosition(node.node, time, pose);
                                 FbxAMatrix GeometryOffset=GetGeometryOffset(node.node);
                                 FbxAMatrix GlobalOffPosition=GlobalPosition*GeometryOffset;
                                 FbxAMatrix ReferenceGlobalInitPosition; cluster->GetTransformMatrix(ReferenceGlobalInitPosition);
                                 FbxAMatrix ReferenceGlobalCurrentPosition=GlobalOffPosition;
                                 FbxAMatrix ReferenceGeometry=GetGeometryOffset(node.node);
                                 ReferenceGlobalInitPosition*=ReferenceGeometry;
                                 FbxAMatrix ClusterGlobalInitPosition; cluster->GetTransformLinkMatrix(ClusterGlobalInitPosition);
                                 FbxAMatrix ClusterGlobalCurrentPosition=GetGlobalPosition(cluster->GetLink(), time, pose);
                                 FbxAMatrix ClusterRelativeInitPosition=ClusterGlobalInitPosition.Inverse()*ReferenceGlobalInitPosition;
                                 FbxAMatrix ClusterRelativeCurrentPositionInverse=ReferenceGlobalCurrentPosition.Inverse()*ClusterGlobalCurrentPosition;
                                 FbxAMatrix VertexTransformMatrix=ClusterRelativeCurrentPositionInverse*ClusterRelativeInitPosition;
                                 bone_matrix[bone->bone_index+VIRTUAL_ROOT_BONE]=MATRIX(VertexTransformMatrix)*matrix_d;
                                 animate=true;
                              }else
                              {
                              #if DEBUG
                                 Exit("Cluster Node is not a bone");
                              #endif
                              }
                           }else
                           {
                           #if DEBUG
                              Exit("Cluster Node not found");
                           #endif
                           }
                        }
                     }

                     if(node.nearest_bone_index>=0) // if this or any parent is a bone
                     {
                        force_skin=true; // if any parent is a bone, then always set skinning
                        vtx_matrix.x=node.nearest_bone_index+VIRTUAL_ROOT_BONE;
                     }
                  }

                  Int tris=0, quads=0;
                  REP(polys)
                  {
                     Int poly_vtxs=fbx_mesh->GetPolygonSize(i);
                     if( poly_vtxs==3) tris++;else
                     if( poly_vtxs==4)quads++;else
                     if( poly_vtxs>=5) tris+=poly_vtxs-2;
                  }

                  // set mesh
                  base.create(tris*3 + quads*4, 0, tris, quads, (fbx_mesh->GetElementVertexColorCount() ? VTX_COLOR : MESH_NONE) | ((texs>=1)?VTX_TEX0:MESH_NONE) | ((texs>=2)?VTX_TEX1:MESH_NONE) | ((texs>=3)?VTX_TEX2:MESH_NONE) | ((texs>=4)?VTX_TEX3:MESH_NONE) | (fbx_mesh->GetElementNormalCount()?VTX_NRM:MESH_NONE) | ((force_skin || vtx_skin.elms())?VTX_SKIN:MESH_NONE) | (one_material?MESH_NONE:FACE_ID));
                  Bool reverse=node.global.mirrored();

                  // polys
                  Int vtxs=0; tris=quads=0; FREPD(poly, polys) // polygons
                  {
                     Int poly_vtxs =fbx_mesh->GetPolygonSize(poly);
                     if( poly_vtxs>=3)
                     {
                        Int vtx_ofs=vtxs;

                        // vertexes
                        FREPD(vtx, poly_vtxs) // vertexes in poly
                        {
                           Int control_point_index=fbx_mesh->GetPolygonVertex(poly, vtx);

                           // pos
                           base.vtx.pos(vtxs)=(InRange(control_point_index, control_points) ? VEC(control_point[control_point_index]).xyz : VecZero);

                           // skin
                           if(base.vtx.matrix())
                           {
                              if(InRange(control_point_index, vtx_skin))
                              {
                                 Memc<IndexWeight> &vs=vtx_skin[control_point_index]; if(vs.elms())
                                 {
                                    SetSkin(vs, base.vtx.matrix(vtxs), base.vtx.blend(vtxs), skeleton);
                                    goto skin_set;
                                 }
                              }
                              // if vtx had no skinning set, then set default
                              base.vtx.matrix(vtxs)=vtx_matrix;
                              base.vtx.blend (vtxs)=vtx_blend ;
                           skin_set:;
                           }

                           // color
                           if(base.vtx.color())
                           {
                              Color color=WHITE;
                              if(FbxGeometryElementVertexColor *leVtxc=fbx_mesh->GetElementVertexColor(0))
                                 switch(leVtxc->GetMappingMode())
                              {
                                 case FbxGeometryElement::eByControlPoint: switch(leVtxc->GetReferenceMode())
                                 {
                                    case FbxGeometryElement::eDirect       : color=COLOR(leVtxc->GetDirectArray().GetAt(control_point_index)); break;
                                    case FbxGeometryElement::eIndexToDirect: color=COLOR(leVtxc->GetDirectArray().GetAt(leVtxc->GetIndexArray().GetAt(control_point_index))); break;
                                 }break;

                                 case FbxGeometryElement::eByPolygonVertex: switch(leVtxc->GetReferenceMode())
                                 {
                                    case FbxGeometryElement::eDirect       : color=COLOR(leVtxc->GetDirectArray().GetAt(vtxs)); break;
                                    case FbxGeometryElement::eIndexToDirect: color=COLOR(leVtxc->GetDirectArray().GetAt(leVtxc->GetIndexArray().GetAt(vtxs))); break;
                                 }break;
                              }
                              base.vtx.color(vtxs)=color;
                           }

                           // normal
                           if(base.vtx.nrm())
                           {
                              Vec nrm=0;
                              if(FbxGeometryElementNormal *leNormal=fbx_mesh->GetElementNormal(0))
                                 switch(leNormal->GetMappingMode())
                              {
                                 case FbxGeometryElement::eByControlPoint: switch(leNormal->GetReferenceMode())
                                 {
                                    case FbxGeometryElement::eDirect       : nrm=VEC(leNormal->GetDirectArray().GetAt(control_point_index)).xyz; break;
                                    case FbxGeometryElement::eIndexToDirect: nrm=VEC(leNormal->GetDirectArray().GetAt(leNormal->GetIndexArray().GetAt(control_point_index))).xyz; break;
                                 }break;

                                 case FbxGeometryElement::eByPolygonVertex: switch(leNormal->GetReferenceMode())
                                 {
                                    case FbxGeometryElement::eDirect       : nrm=VEC(leNormal->GetDirectArray().GetAt(vtxs)).xyz; break;
                                    case FbxGeometryElement::eIndexToDirect: nrm=VEC(leNormal->GetDirectArray().GetAt(leNormal->GetIndexArray().GetAt(vtxs))).xyz; break;
                                 }break;
                              }
                              base.vtx.nrm(vtxs)=nrm;
                           }

                           // tex
                           REPD(tex, texs)
                           {
                              Vec2 uv=0;
                              if(FbxGeometryElementUV *leUV=fbx_mesh->GetElementUV(tex))
                                 switch(leUV->GetMappingMode())
                              {
                                 case FbxGeometryElement::eByControlPoint: switch(leUV->GetReferenceMode())
                                 {
                                    case FbxGeometryElement::eDirect       : uv=VEC(leUV->GetDirectArray().GetAt(control_point_index)); break;
                                    case FbxGeometryElement::eIndexToDirect: uv=VEC(leUV->GetDirectArray().GetAt(leUV->GetIndexArray().GetAt(control_point_index))); break;
                                 }break;

                                 case FbxGeometryElement::eByPolygonVertex:
                                 {
                                    switch(leUV->GetReferenceMode())
                                    {
                                       case FbxGeometryElement::eDirect       : uv=VEC(leUV->GetDirectArray().GetAt(vtxs)); break;
                                       case FbxGeometryElement::eIndexToDirect: uv=VEC(leUV->GetDirectArray().GetAt(leUV->GetIndexArray().GetAt(vtxs))); break;
                                    }
                                 }break;
                              }
                              uv.y=1-uv.y; // FBX uses mirrored Y
                              if(tex==0)base.vtx.tex0(vtxs)=uv;else
                              if(tex==1)base.vtx.tex1(vtxs)=uv;else
                              if(tex==2)base.vtx.tex2(vtxs)=uv;else
                              if(tex==3)base.vtx.tex3(vtxs)=uv;
                           }

                           // counter
                           vtxs++;
                        }

                        // face
                        if(!one_material) // material
                        {
                           FbxSurfaceMaterial *mtrl=null;
                           FREPD(m, fbx_mesh->GetElementMaterialCount())
                              if(FbxGeometryElementMaterial *mtrl_elm=fbx_mesh->GetElementMaterial(m))
                                 if(mtrl=node.node->GetMaterial(mtrl_elm->GetIndexArray().GetAt(poly)))break;
                           Int mtrl_id=findMaterial(mtrl)+1; // +1 so not found material (-1) will have ID=0, materials with 0 index will have ID=1, ..

                           if (poly_vtxs==4)base.quad.id( quads)=mtrl_id;else
                           REP(poly_vtxs -2)base.tri .id(i+tris)=mtrl_id;
                        }

                        // vertex index
                        if(poly_vtxs==3) // triangle
                        {
                           base.tri.ind(tris).set(vtx_ofs, vtx_ofs+1, vtx_ofs+2); if(reverse)base.tri.ind(tris).reverse();
                           tris++;
                        }else
                        if(poly_vtxs==4) // quad
                        {
                           VecI4 &ind=base.quad.ind(quads++).set(vtx_ofs, vtx_ofs+1, vtx_ofs+2, vtx_ofs+3); if(reverse)ind.swapXZ(); // use 'swapXZ' to preserve the same triangles being generated, but flipped, this is important, because 'reverse' would cause different triangle combination
                           // there are 2 possibilities to choose a quad, default: 013+123, rotated: 012+230, to determine which one, we calculate normals for 2 tris in those 2 quads (4 tris total) and select the quad which tris are the most flat
                         C Vec &p0=base.vtx.pos(ind.x), &p1=base.vtx.pos(ind.y), &p2=base.vtx.pos(ind.z), &p3=base.vtx.pos(ind.w),
                               qn0=GetNormal(p0, p1, p3),  qn1=GetNormal(p1, p2, p3), //         quad normals
                              rqn0=GetNormal(p0, p1, p2), rqn1=GetNormal(p2, p3, p0); // rotated quad normals
                           if(Dot(rqn0, rqn1)>Dot(qn0, qn1))ind.rotateOrder();
                        }else // triangulate a polygon
                        {
                           // get average normal
                           Vec normal=0; REP(poly_vtxs)
                           {
                              VecI2 ind(i, (i+1)%poly_vtxs); ind+=vtx_ofs;
                              normal+=GetNormalEdge(base.vtx.pos(ind.x), base.vtx.pos(ind.y));
                           }
                           // add triangles
                           poly_vtx.setNum(poly_vtxs); REPAO(poly_vtx)=vtx_ofs+i;
                           for(; poly_vtx.elms()>=3; )
                           {
                              Bool added=false; // if added a face in this step
                              REP(poly_vtx.elms()-2)
                              {
                                 VecI &ind=base.tri.ind(tris); ind.set(poly_vtx[i], poly_vtx[i+1], poly_vtx[i+2]);
                                 Tri   tri(base.vtx.pos(ind.x), base.vtx.pos(ind.y), base.vtx.pos(ind.z)); Vec tri_nu=tri.getNormalU(); // can use 'getNormalU' because here we just need the sign
                                 if(Dot(normal, tri_nu)>0) // if normal of this triangle is correct
                                 {
                                    Vec cross[3]={Cross(tri_nu, tri.p[0]-tri.p[1]), Cross(tri_nu, tri.p[1]-tri.p[2]), Cross(tri_nu, tri.p[2]-tri.p[0])};
                                    REPAD(t, poly_vtx)if(t<i || t>i+2)if(Cuts(base.vtx.pos(poly_vtx[t]), tri, cross))goto cuts; // if any other vertex intersects with this triangle, then continue

                                    if(reverse)ind.reverse();
                                    tris++;
                                    poly_vtx.remove(i+1, true);
                                    added=true;
                                 cuts:;
                                 }
                              }
                              if(!added) // add all remaining
                              {
                                 REP(poly_vtx.elms()-2)
                                 {
                                    VecI &ind=base.tri.ind(tris); ind.set(poly_vtx[i], poly_vtx[i+1], poly_vtx[i+2]); if(reverse)ind.reverse();
                                    tris++;
                                    poly_vtx.remove(i+1, true);
                                 }
                                 break;
                              }
                           }
                        }
                     }
                  }

                  // finalize base
                  if(animate)base.animate(bone_matrix);else base.transform(matrix); // 'bone_matrix' was already transformed by 'matrix'
                  if(!base.vtx.nrm())base.setNormalsAuto(EPS_NRM_AUTO, EPSD); // use small pos epsilon in case mesh is scaled down, call before 'setTanBin'
                  if(!base.vtx.tan() || !base.vtx.bin())base.setTanBin(); //if(!base.vtx.tan())base.setTangents(); if(!base.vtx.bin())base.setBinormals(); // need to call before 'weldVtx' to don't remove too many vertexes
                  base.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small pos epsilon in case mesh is scaled down

                  // extract LOD index from name
                  Int lod_i=-1; // -1=unspecified
                  Str part_name=node.full_name;
                  {
                     Str temp_name=part_name;
                     for(; CharFlag(temp_name.last())&CHARF_DIG; )temp_name.removeLast();
                     if(Ends(temp_name, "_LOD", true))
                     {
                        lod_i=TextInt(part_name()+temp_name.length());
                        if(!InRange(lod_i, 16))lod_i=-1; // keep within sensible ranges
                        part_name=temp_name.removeLast(4); // remove "_LOD"
                     }
                  }
                  MAX(lod_i, 0);
                  mesh->setLods(Max(mesh->lods(), lod_i+1)); // make room for 'lod_i'
                  MIN(lod_i, mesh->lods()-1); // in case failed to create

                  Int pmi_pos=0; // position in 'part_material_index' at which to add new parts, it's sorted by LODs first, then by parts (L0P0 L0P1, L1P0 L1P1, ..), so we have to insert in between other elements
                  if(part_material_index)for(Int i=0; i<=lod_i; i++)pmi_pos+=mesh->lod(i).parts.elms(); // count how many parts are there for LODs up to 'lod_i' (inclusive)

                  // put to Mesh LOD
                  MeshLod &lod=mesh->lod(lod_i);
                  if(one_material)
                  {
                     MeshPart &part=lod.parts.New(); Set(part.name, part_name);
                     Swap(part.base, base);
                     if(part_material_index)
                     {
                        Int ee_mtrl=-1;
                        FREPD(m, fbx_mesh->GetElementMaterialCount())
                           if(FbxGeometryElementMaterial *mtrl_elm=fbx_mesh->GetElementMaterial(m))
                              if(mtrl_elm->GetMappingMode()==FbxGeometryElement::eAllSame)
                                 if(FbxSurfaceMaterial *mtrl=node.node->GetMaterial(mtrl_elm->GetIndexArray().GetAt(0)))
                                    {ee_mtrl=findMaterial(mtrl); break;}
                        part_material_index.NewAt(pmi_pos)=ee_mtrl;
                     }
                  }else
                  {
                     MeshLod temp;
                     base.copyID(temp, ~ID_ALL); // copy to 'temp', IDs are no longer needed
                     FREPA(temp)
                     {
                        MeshPart &src=temp.parts[i]; if(src.is())
                        {
                           MeshPart &dest=lod.parts.New(); Swap(src, dest); Set(dest.name, part_name);
                           if(part_material_index)part_material_index.NewAt(pmi_pos++)=i-1; // IDs were created with +1
                        }
                     }
                  }
               }
            }
         }
      }
   }
   static void AddKeyTimes(FbxAnimCurve *curve, Memc<FbxTime> &times, Dbl fps)
   {
      if(curve)
      {
         Int  keys=curve->KeyGetCount(); times.reserve(keys);
         FREP(keys) // process in order to avoid moving elements when inserting with 'binaryInclude'
         {
            FbxTime time=curve->KeyGetTime(i);
            times.binaryInclude(time, CompareTime);
            if(InRange(i+1, keys) && curve->KeyGetInterpolation(i)!=FbxAnimCurveDef::eInterpolationLinear) // have next key, and interpolation is not linear
            { // this is needed for keys that use non-linear interpolation (cubic/spline/tangets)
               Dbl  t =time.GetSecondDouble(), t1 =curve->KeyGetTime(i+1).GetSecondDouble(); // time in seconds for current and next key
               Long ti=RoundL(t*fps)         , ti1=RoundL(t1*fps); // frames
               if(ti1>ti)
               {
                  // add keys between
                  const Int max_steps=256; // some FBX can have invalid data, for example 1st keyframe time 0, and 2nd keyframe millions of seconds later which would result in heaps of keyframes, so limit up to 256 steps between keyframes
                //times.reserve(Min(ti1-ti, max_steps)); skip to avoid overhead because most likely we will already have enough room since frame containers are now allocated outside of the loop
                  if(ti1-ti<=max_steps)for(Long t=ti+1; t<ti1      ; t++){time.SetSecondDouble(t/fps                        ); times.binaryInclude(time, CompareTime);} // if within limit, then place keyframes on exact frame times according to FPS
                  else                 for(Int  i=   1; i<max_steps; i++){time.SetSecondDouble(Lerp(t, t1, i/Dbl(max_steps))); times.binaryInclude(time, CompareTime);} // place keyframes spread linearly between start end
               }
            }
         }
      }
   }
   void set(Skeleton *skeleton, MemPtr<XAnimation> animations)
   {
      if(skeleton && animations)
      {
         Memc<FbxTime> rot_times, pos_times, scale_times, times; // declare outside of loop to reuse memory allocations
         FREP(scene->GetSrcObjectCount<FbxAnimStack>())
            if(FbxAnimStack *anim_stack=scene->GetSrcObject<FbxAnimStack>(i))
         {
            scene->SetCurrentAnimationStack(anim_stack);

            Int anim_layers=anim_stack->GetMemberCount<FbxAnimLayer>();
            if( anim_layers>0)
            {
               FbxTimeSpan time_span=anim_stack->GetLocalTimeSpan(); Dbl time_start=time_span.GetStart().GetSecondDouble(), time_stop=time_span.GetStop().GetSecondDouble(), fps=FbxTime::GetFrameRate(global_settings->GetTimeMode());
               XAnimation &xanim    =animations.New();
               xanim.fps  =fps;
               xanim.start=time_start;
               xanim.name =FromUTF8(anim_stack->GetName());
               xanim.anim.length(time_stop-time_start, false);
               if(anim_layers>1)
               {
                  Bool ok=anim_stack->BakeLayers(null, 0, time_stop, 1.0/fps);
                  anim_layers=1;
               }
               REPD(layer, anim_layers)
                  if(FbxAnimLayer *anim_layer=anim_stack->GetMember<FbxAnimLayer>(layer))
               {
                  FREPAD(n, nodes)
                  {
                     Node &node=nodes[n]; if(InRange(node.bone_index, skeleton->bones))
                     {
                        rot_times.clear(); pos_times.clear(); scale_times.clear();

                        Node *animated_node_ancestor=&node; // there's following node hierarchy starting from root: null <-> root <-> .. <-> bone_node_parent (nearest node that is a bone) <-> .. (some nodes) <-> animated_node_ancestor (first node that has some animations, in most cases this is 'node' however if there were some parent bones that are ignored, then this could point to one of them) <-> .. <-> node (currently processed node), at start set to "&node" so we don't have to call 'hasAnim' below, this is never set to another node that is a bone (only 'node' can be set or another nodes that have animations but aren't bones)

                        for(Node *cur=&node; ; )
                        {
                           AddKeyTimes(cur->node->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X), rot_times, fps);
                           AddKeyTimes(cur->node->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y), rot_times, fps);
                           AddKeyTimes(cur->node->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z), rot_times, fps);

                           AddKeyTimes(cur->node->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X), pos_times, fps);
                           AddKeyTimes(cur->node->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y), pos_times, fps);
                           AddKeyTimes(cur->node->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z), pos_times, fps);

                           AddKeyTimes(cur->node->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X), scale_times, fps);
                           AddKeyTimes(cur->node->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y), scale_times, fps);
                           AddKeyTimes(cur->node->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z), scale_times, fps);

                           cur=cur->parent; if(!cur || cur->bone)break; // if there's no parent, or it's a bone (its animations are already stored in it so we can skip them), then break
                           if(cur->hasAnim(anim_layer))animated_node_ancestor=cur; // remember the last encountered node that has some animations !! 'hasAnim' assumes that 'SetCurrentAnimationStack' was already called !! check this after proceeding to the parent, so we don't call this for 'node' itself, since we've already set 'animated_node_ancestor' to it
                        }

                        if(rot_times.elms() || pos_times.elms() || scale_times.elms())
                        {
                           times.clear();
                        #if 0 // force keys for all frames, this can be used for testing
                           Int steps=Round(fps*xanim.anim.length())+1;
                           if(  rot_times.elms()){  rot_times.setNum(steps); FREP(steps)  rot_times[i].SetSecondDouble(time_start+i/fps);}
                           if(  pos_times.elms()){  pos_times.setNum(steps); FREP(steps)  pos_times[i].SetSecondDouble(time_start+i/fps);}
                           if(scale_times.elms()){scale_times.setNum(steps); FREP(steps)scale_times[i].SetSecondDouble(time_start+i/fps);}
                                                        times.setNum(steps); FREP(steps)      times[i].SetSecondDouble(time_start+i/fps);
                        #else
                           times.reserve(Max(rot_times.elms(), pos_times.elms(), scale_times.elms()));
                           FREPA(  rot_times)times.binaryInclude(  rot_times[i], CompareTime);
                           FREPA(  pos_times)times.binaryInclude(  pos_times[i], CompareTime);
                           FREPA(scale_times)times.binaryInclude(scale_times[i], CompareTime);
                        #endif

                        #if DEBUG
                         //if(!node.bone)Exit("Animated Node is not a Bone");
                        #endif
                           SkelBone &sbon=skeleton->bones[node.bone_index];
                           AnimBone &abon=xanim.anim.bones.New(); abon.set(sbon.name);
                           MatrixD3  parent_matrix_inv; if(sbon.parent!=0xFF)skeleton->bones[sbon.parent].inverse(parent_matrix_inv);

                           // orientation
                           MatrixD3 local_to_world; animated_node_ancestor->local.orn().inverseNonOrthogonal(local_to_world); local_to_world*=animated_node_ancestor->global.orn(); // GetTransform(animated_node_ancestor->local.orn(), animated_node_ancestor->global.orn());
                           MatrixD3 local_node_to_local_bone; if(sbon.parent!=0xFF)local_to_world.mul(parent_matrix_inv, local_node_to_local_bone);else local_node_to_local_bone=local_to_world; // after we convert to world space with 'local_to_world', we then convert it to bone in its parent space

                           // position
                           MatrixD pos_matrix;
                           if(Node *parent=animated_node_ancestor->parent)pos_matrix=parent->global;else pos_matrix.identity();
                                                                          pos_matrix.moveBack(sbon.pos);
                           if(sbon.parent!=0xFF                          )pos_matrix*=parent_matrix_inv;

                           abon.orns  .setNum(  rot_times.elms());
                           abon.poss  .setNum(  pos_times.elms());
                           abon.scales.setNum(scale_times.elms());

                        #if KEY_SAFETY
                           REPAO(abon.orns  ).time=NAN;
                           REPAO(abon.poss  ).time=NAN;
                           REPAO(abon.scales).time=NAN;
                        #endif

                           FREPA(times)
                           {
                              Int        index;
                            C FbxTime   &time      =times[i];
                              Flt        t         =time.GetSecondDouble()-time_start;
                              FbxAMatrix transform =node.node->EvaluateLocalTransform(time);
                              MatrixD3   node_local=node.local;
                              for(Node *n=&node; n!=animated_node_ancestor; ) // gather all transforms starting from this 'node' to 'animated_node_ancestor' inclusive
                              {
                                 n=n->parent;
                                 transform=n->node->EvaluateLocalTransform(time)*transform; // FBX multiply order is reversed compared to Engine
                                 node_local*=n->local;
                              }
                              MatrixD anim_matrix=MATRIX(transform);

                              // orientation
                              if(rot_times.binarySearch(time, index, CompareTime))
                              {
                                 MatrixD3 anim;
                                 anim.x=anim_matrix.z;
                                 anim.y=anim_matrix.y;
                                 anim.z=anim_matrix.x;
                              #if 0
                                 anim*=local_to_world; // convert to world space
                                 if(sbon.parent!=0xFF)anim*=parent_matrix_inv; // convert to local space (relative to skeleton parent bone)
                              #else // optimized
                                 anim*=local_node_to_local_bone; // convert from local node to local bone space
                              #endif

                                 OrientD o=anim; o.fix();
                                 AnimKeys::Orn &orn=abon.orns[index]; orn.time=t; orn.orn=o;
                              }

                              // position
                              if(pos_times.binarySearch(time, index, CompareTime))
                              {
                                 VecD p=anim_matrix.pos;
                              #if 0
                                 if(Node *parent=animated_node_ancestor->parent)p*=parent->global; // convert to world space (here we don't use 'local_to_world' because that includes this node orientation, but node position is independent on its orientation)
                                                                                p-=sbon.pos; // set as world space delta from bone
                                 if(sbon.parent!=0xFF                          )p*=parent_matrix_inv; // convert to local space (relative to skeleton parent bone)
                              #else // optimized
                                 p*=pos_matrix;
                              #endif
                                 AnimKeys::Pos &pos=abon.poss[index]; pos.time=t; pos.pos=p;
                              }

                              // scale
                              if(scale_times.binarySearch(time, index, CompareTime))
                              {
                                 AnimKeys::Scale &scale=abon.scales[index]; scale.time=t;
                                 scale.scale.x=ScaleFactorR(anim_matrix.z.length()/node_local.z.length());
                                 scale.scale.y=ScaleFactorR(anim_matrix.y.length()/node_local.y.length());
                                 scale.scale.z=ScaleFactorR(anim_matrix.x.length()/node_local.x.length());
                              }
                           }

                        #if KEY_SAFETY
                           REPA(abon.orns  )if(NaN(abon.orns  [i].time))abon.orns  .remove(i, true);
                           REPA(abon.poss  )if(NaN(abon.poss  [i].time))abon.poss  .remove(i, true);
                           REPA(abon.scales)if(NaN(abon.scales[i].time))abon.scales.remove(i, true);
                        #endif

                           abon.sortFrames(); // sort just in case FbxTime is weird somehow
                        }
                     }
                  }
               }
               xanim.anim.setTangents().setRootMatrix();
            }
         }
      }
   }
};
/******************************************************************************/
Bool _ImportFBX(C Str &name, Mesh *mesh, Skeleton *skeleton, MemPtr<XAnimation> animations, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index, XSkeleton *xskeleton, Bool all_nodes_as_bones, C Str8 &password)
{
   if( mesh    ) mesh    ->del();
   if( skeleton) skeleton->del();
   if(xskeleton)xskeleton->del();
   animations         .clear();
   materials          .clear();
   part_material_index.clear();

   FBX fbx; if(fbx.init() && fbx.load(name, all_nodes_as_bones, password))
   {
      Skeleton temp, *skel=(skeleton ? skeleton : (mesh || xskeleton || animations) ? &temp : null); // if skel not specified, but we want mesh, xskeleton or animations, then we have to process it (mesh requires it for skinning: bone names and types, animations require for skeleton bone transforms, parents, etc.)
      {
         SyncLocker locker(Lock);
         fbx.set(materials); // !! call before creating meshes to setup 'engine_mtrl_to_fbx_mtrl' !!
         if(skel)
         {
            fbx.setNodesAsBoneFromSkin();
            fbx.setNodesAsBoneFromAnim();
         #if 0
            Memc<Str> ignore;
         #if 0 // InfinityPBR - Medusa 256 bone limit
            ignore.add("Root");
            ignore.add("LeftWrist");
            ignore.add("RightWrist");
            ignore.add("Jaw_1");
            ignore.add("Tongue_1");
            ignore.add("Tongue_3");
            ignore.add("Tongue_5");
            ignore.add("L_weapon");

            ignore.add("Snakehead_8");
            ignore.add("Snakehead_13");
            ignore.add("Snakehead_23");
            ignore.add("Snakehead_33");
            ignore.add("Snakehead_18");
            ignore.add("Snakehead_28");

            ignore.add("L_Snakehead_68");
            ignore.add("L_Snakehead_73");
            ignore.add("L_Snakehead_78");
          //ignore.add("L_Snakehead_83");
            ignore.add("L_Snakehead_93");
            ignore.add("L_Snakehead_98");
            ignore.add("L_Snakehead_103");
            ignore.add("L_Snakehead_108");
            ignore.add("L_Snakehead_113");
            ignore.add("L_Snakehead_118");
            ignore.add("L_Snakehead_123");
            ignore.add("L_Snakehead_128");

            ignore.add("R_Snakehead_68");
            ignore.add("R_Snakehead_73");
            ignore.add("R_Snakehead_78");
          //ignore.add("R_Snakehead_83");
            ignore.add("R_Snakehead_93");
            ignore.add("R_Snakehead_98");
            ignore.add("R_Snakehead_103");
            ignore.add("R_Snakehead_108");
            ignore.add("R_Snakehead_113");
            ignore.add("R_Snakehead_118");
            ignore.add("R_Snakehead_123");
            ignore.add("R_Snakehead_128");

            ignore.add("Tail02");
            ignore.add("Tail04");
            ignore.add("Tail06");
            ignore.add("Tail08");
            ignore.add("Tail10");
            ignore.add("Tail12");
            ignore.add("Tail14");
            ignore.add("Tail16");
         #endif
            fbx.ignoreBones(ignore);
         #endif
          //Memc<Str> shorten; shorten.add(); fbx.shortenBoneNames(shorten);
            fbx.makeBoneNamesUnique();
            fbx.shortenBoneNames();
            fbx.set(skel, xskeleton);
            fbx.set(mesh, skel, part_material_index);
            fbx.set(skel, animations);
         }
      }

      REPAO(materials ).fixPath(GetPath(name));
         if(xskeleton )xskeleton->mirrorX();
         if( skel     ) skel    ->mirrorX().setBoneTypes(); // call 'setBoneTypes' after 'mirrorX' !!
         if(mesh      ){mesh    ->mirrorX().skeleton(skel).skeleton(null).setBox(); CleanMesh(*mesh);} // !! link with skeleton after calling 'setBoneTypes' !!
      REPAO(animations).anim     .mirrorX().setBoneTypeIndexesFromSkeleton(*skel); // !! call this after 'setBoneTypes' !!

      if(!Equal(fbx.scale, 1))
      {
            if( mesh     ) mesh    ->scale(fbx.scale);
            if( skeleton ) skeleton->scale(fbx.scale);
            if(xskeleton )xskeleton->scale(fbx.scale);
         REPAO(animations).anim     .scale(fbx.scale);
      }

      // after scale
      if(skeleton)skel->setBoneShapes(); // here process things that are needed only if we want to import skeleton (and not just operating on temporary local that will get discarded)
      if(mesh) // set LOD distances
      {
         Flt dist=2;
         for(Int i=1; i<mesh->lods(); i++, dist*=2)mesh->lod(i).dist(dist);
      }
      return true;
   }
   return false;
}
/******************************************************************************/
}
#include "FBX Shared.h"
#if FBX_LINK_TYPE==FBX_LINK_DLL
/******************************************************************************
   Memory allocated by the DLL MUST also be free'd by the DLL (otherwise errors happen), so:
      -allocate all FBX data on the DLL
      -return pointer to that data to the EXE
      -copy data on the EXE
      -free data in the DLL
/******************************************************************************/
extern "C" __declspec(dllexport) Int  __cdecl ImportFBXVer (                                 ) {return   0;}
extern "C" __declspec(dllexport) void __cdecl ImportFBXFree(CPtr   data                      ) {Free(data);}
extern "C" __declspec(dllexport) CPtr __cdecl ImportFBXData(CChar *name, Int &size, UInt flag)
{
   Mesh              mesh , * mesh_ptr=(flag&FBX_MESH ) ? & mesh : null;
   Skeleton          skel , * skel_ptr=(flag&FBX_SKEL ) ? & skel : null;
  XSkeleton         xskel , *xskel_ptr=(flag&FBX_XSKEL) ? &xskel : null;
   Memc<XAnimation>  anims; MemPtr<XAnimation> anims_ptr; if(flag&FBX_ANIM)anims_ptr.point(anims);
   Memc<XMaterial >  mtrls; MemPtr<XMaterial > mtrls_ptr; if(flag&FBX_MTRL)mtrls_ptr.point(mtrls);
   Memc<Int       >  pmi  ; MemPtr<Int       >   pmi_ptr; if(flag&FBX_PMI )  pmi_ptr.point(pmi  );

   if(_ImportFBX(name, mesh_ptr, skel_ptr, anims_ptr, mtrls_ptr, pmi_ptr, xskel_ptr, FlagOn(flag, FBX_ALL_NODES_AS_BONES), S8))
   {
      File f; f.writeMem(); SaveFBXData(f, mesh_ptr, skel_ptr, anims_ptr, mtrls_ptr, pmi_ptr, xskel_ptr); // save data to file
      Ptr    data=Alloc(f.size()); f.pos(0); f.get(data, size=f.size()); // copy file to memory
      return data; // return that memory
   }
   return null;
}
/******************************************************************************/
void InitPre() {}
Bool Init   () {return false;}
void Shut   () {}
Bool Update () {return false;}
void Draw   () {}
/******************************************************************************/
#endif
/******************************************************************************/
