/******************************************************************************/
#include "stdafx.h"
ASSERT(DIFFUSE_LAMBERT   ==SDIFFUSE_LAMBERT
    && DIFFUSE_OREN_NAYAR==SDIFFUSE_OREN_NAYAR
    && DIFFUSE_BURLEY    ==SDIFFUSE_BURLEY
    && DIFFUSE_NUM       ==SDIFFUSE_NUM);
namespace EE{
/******************************************************************************

   !! Warning: Never return the same shader for Multi-Materials as Single-Materials !!
      Because crash/memory corruption may occur, since they need to operate on different containers 'MultiMaterialShaderDraws' and 'ShaderDraws'

/******************************************************************************/
ShaderImage  * FindShaderImage  (CChar8 *name) {return ShaderImages  .find(Str8Temp(name));}
ShaderRWImage* FindShaderRWImage(CChar8 *name) {return ShaderRWImages.find(Str8Temp(name));}
ShaderImage  *  GetShaderImage  (CChar8 *name) {ShaderImage   *image=FindShaderImage  (name); if(!image)Exit(S+"Shader Image \""   +name+"\" not found."); return image;}
ShaderRWImage*  GetShaderRWImage(CChar8 *name) {ShaderRWImage *image=FindShaderRWImage(name); if(!image)Exit(S+"Shader RW Image \""+name+"\" not found."); return image;}

ShaderParam* FindShaderParam(CChar8 *name)
{
   Str8Temp key(name); // use 'Str8Temp' to prevent memory allocation
   if(ShaderParam *sp=ShaderParams.find(key))return sp;

   // maybe we're looking for array member
   FREPA(key)
   {
      if(name[i]=='[') // check for existence of '['
      {
         Char8 parent_name[MAX_LONG_PATH]; if(InRange(i, parent_name))
         {
            i++; Set(parent_name, name, i); // copy all before '['
            if(ShaderParam *parent=ShaderParams.find(Str8Temp(parent_name)))
            {
               CalcValue val; CChar8 *end=TextValue(name+i, val); if(end && end[0]==']' && end[1]=='\0') // support only 1-dim arrays without further members "val[x]" but not "val[x][y].z"
               {
                  Int index=val.asInt(); if(InRange(index, parent->_elements))
                  {
                     ShaderParams.lock  (); ShaderParam &elm=*ShaderParams(key); if(!elm.is())elm.initAsElement(*parent, index); // init only if not initialized yet, in case it just got initialized on another thread before the lock
                     ShaderParams.unlock();
                     return &elm;
                  }
               }
            }
         }
         break;
      }
   }

   return null;
}
ShaderParam* GetShaderParam(CChar8 *name) {ShaderParam *param=FindShaderParam(name); if(!param)Exit(S+"Shader Param \""+name+"\" not found."); return param;}
/******************************************************************************/
ShaderBuffer* FindShaderBuffer(CChar8 *name) {return ShaderBuffers.find(Str8Temp(name));}
ShaderBuffer*  GetShaderBuffer(CChar8 *name) {ShaderBuffer *sb=FindShaderBuffer(name); if(!sb)Exit(S+"Shader Buffer \""+name+"\" not found."); return sb;}
/******************************************************************************/
static Int Layout(C Material &material) // Textures
{ // #MaterialTextureLayout
   if(material.base_0)
   {
      if(material.base_2)return 2; // layout 2 assumes we have both 'base_0' and 'base_2'
                         return 1;
   }                     return 0;
   // 'base_1' is normal map and doesn't affect texture layout
}
static Int BumpMode(C Material &material, MESH_FLAG mesh_flag)
{
   if(mesh_flag&VTX_NRM)
   {
      if((mesh_flag&VTX_TEX0) && (mesh_flag&VTX_TAN) && D.bumpMode()>=BUMP_NORMAL && material.base_1) // normal in 'base_1' #MaterialTextureLayout
      {
         if(D.bumpMode()>BUMP_NORMAL && material.bump>EPS_MATERIAL_BUMP && material.base_2) // bump in 'base_2' #MaterialTextureLayout
         {
            if(D.bumpMode()==BUMP_RELIEF)return SBUMP_RELIEF;
            return Mid(Ceil(material.bump/0.0075f)+SBUMP_PARALLAX0, SBUMP_PARALLAX_MIN, SBUMP_PARALLAX_MAX);
         }
         if(material.normal>EPS_COL8)return SBUMP_NORMAL;
      }
      return SBUMP_FLAT;
   }
   return SBUMP_ZERO;
}
static Int  EmissiveMode(C Material &material) {return (material.emissive_l.max()>EPS_COL8_LINEAR || material.emissive_glow>EPS_COL8) ? material.emissive_map ? 2 : 1 : 0;}
static Bool Detail      (C Material &material) {return  material.detail_map && material.det_power>EPS_COL8;} // #MaterialTextureLayoutDetail
static Bool Macro       (C Material &material) {return  material. macro_map;}
static Bool Reflect     (C Material &material) {return  material.reflect_add+((material.base_2 && material.reflect_mul>0) ? material.reflect_mul : 0)  // get maximum possible reflectivity                                                           , add 'reflect_mul' only if it increases reflectivity (>0) because we only want possible maximum. #MaterialTextureLayout 'reflect_mul' is multiplied with metal     texture which is stored in base_2
                                                    +1-(material.  rough_add+((material.base_2 && material.  rough_mul<0) ? material.  rough_mul : 0)) // get maximum possible smoothness, which is minimum possible roughness converted to smoothness, add   'rough_mul' only if it decreases roughness    (<0) because we only want possible minimum. #MaterialTextureLayout   'rough_mul' is multiplied with roughness texture which is stored in base_2
                                                       >EPS_COL8;}
static MESH_FLAG FlagHeightmap(MESH_FLAG mesh_flag, Bool heightmap)
{
   if(heightmap)
   {
      if(mesh_flag&VTX_POS)mesh_flag|=VTX_TEX0; // heightmap shaders generate tex from pos
      if(mesh_flag&VTX_NRM)mesh_flag|=VTX_TAN ; // heightmap shaders generate tan from nrm
   }
   return mesh_flag;
}
/******************************************************************************/
DefaultShaders::DefaultShaders(C Material *material, MESH_FLAG mesh_flag, Int lod_index, Bool heightmap)
{
 C Material *materials[4]=
   {
      material,
      null    ,
      null    ,
      null    ,
   };
   init(materials, mesh_flag, lod_index, heightmap);
}
void DefaultShaders::init(C Material *material[4], MESH_FLAG mesh_flag, Int lod_index, Bool heightmap)
{
   // !! Never return the same shader for Multi-Materials as Single-Materials !!
   if(!mesh_flag){set_empty: valid=false; return;}
 C Material *m=(material ? material[0] : null); if(!m){if(D.drawNullMaterials())m=&MaterialDefault;else goto set_empty;}
   mesh_flag=FlagHeightmap(mesh_flag, heightmap);
   valid    =true;
 T.heightmap=heightmap;
   materials=1;
   tex      =FlagOn(mesh_flag, VTX_TEX0 );
   normal   =FlagOn(mesh_flag, VTX_NRM  );
   color    =FlagOn(mesh_flag, VTX_COLOR);
   size     =FlagOn(mesh_flag, VTX_SIZE );

   layout=Layout(*m); bump=BumpMode(*m, mesh_flag); detail=Detail(*m); macro=Macro(*m); reflect=Reflect(*m); emissive=EmissiveMode(*m);
   if(material && material[1]) // && (mesh_flag&VTX_MATERIAL)) we must always return a different shader even when there's no VTX_MATERIAL component, because we need a different shader for multi-material parts that have 'umm', as they operate on 'MultiMaterialShaderDraws' and not 'ShaderDraws', otherwise crash or memory corruption may occur, because 'ShaderBase.shader_index' would point to wrong container
   {
      materials++; MAX(layout, Layout(*material[1])); MAX(bump, BumpMode(*material[1], mesh_flag)); detail|=Detail(*material[1]); macro|=Macro(*material[1]); reflect|=Reflect(*material[1]); MAX(emissive, EmissiveMode(*material[1]));
      if(material[2])
      {
         materials++; MAX(layout, Layout(*material[2])); MAX(bump, BumpMode(*material[2], mesh_flag)); detail|=Detail(*material[2]); macro|=Macro(*material[2]); reflect|=Reflect(*material[2]); MAX(emissive, EmissiveMode(*material[2]));
      #if MAX_MTRLS>=4
         if(material[3])
         {
            materials++; MAX(layout, Layout(*material[3])); MAX(bump, BumpMode(*material[3], mesh_flag)); detail|=Detail(*material[3]); macro|=Macro(*material[3]); reflect|=Reflect(*material[3]); MAX(emissive, EmissiveMode(*material[3]));
         }
      #endif
      }
   }
   switch(D.texDetail())
   {
      case TEX_USE_DISABLE:                detail=false; break;
      case TEX_USE_SINGLE : if(materials>1)detail=false; break;
   }
   if(!D.texMacro    ()                       || lod_index<=0 || skin || !heightmap)macro =false; // disable macro  for LODs=0, skin, !heightmaps
   if(!D.texDetailLOD() && !m->detail_all_lod && lod_index> 0                      )detail=false; // disable detail for LODs>0
   if(                                           lod_index> 0 || layout<2          )MIN(bump, SBUMP_NORMAL); // limit to normal mapping for LODs>0 and layout<2 (no bump channel)
   if(!tex                                                                         ){layout=0; detail=macro=false; MIN(emissive, 1);} // disable all textures if we don't have texcoords
   if(!normal || !D.envMap()                                                       )reflect=false; // reflection requires vtx normals
   if(materials>1                                                                  )MAX(layout, 1); // multi-materials currently don't support 0 textures
   if(materials>1 || heightmap                                                     )emissive=0; // multi-materials and heightmaps currently don't support emissive

   skin                =(FlagAll(mesh_flag, VTX_SKIN)           && materials==1 &&              !heightmap                             );
   fur                 =(normal && tex                          && materials==1 &&              !heightmap && m->technique==MTECH_FUR  ); // this requires tex coordinates, but not a material texture, we can do fur with just material color and 'FurCol'
   grass               =(normal                        && !skin && materials==1 && layout>=1 && !heightmap && m->hasGrass            ());
   leaf                =(normal && (mesh_flag&VTX_HLP) && !skin && materials==1 && layout>=1 && !heightmap && m->hasLeaf             () && D.bendLeafs());
   alpha               =(                                          materials==1 && layout>=1 && !heightmap && m->hasAlpha            ()); // this is about having alpha channel in material textures so we need a texture
   alpha_test          =(                                          materials==1 && layout>=1 && !heightmap && m->hasAlphaTest        ()) ? (m->technique==MTECH_ALPHA_TEST_DITHER ? ALPHA_TEST_DITHER : ALPHA_TEST_YES) : ALPHA_TEST_NO;
   alpha_blend         =(                                          materials==1 &&              !heightmap && m->hasAlphaBlend       ()); // this shouldn't require a texture, we can do alpha blending with just material color
   alpha_blend_no_light=(                                          materials==1 &&              !heightmap && m->hasAlphaBlendNoLight()); // this shouldn't require a texture, we can do alpha blending with just material color
   alpha_blend_light   =(                                          materials==1 &&              !heightmap && m->hasAlphaBlendLight  ()); // this shouldn't require a texture, we can do alpha blending with just material color
    mtrl_blend         =(                                          materials> 1 && layout>=2 && D.materialBlend()                      ); // this is per-pixel multi-material blending (blending between multiple materials)
   tesselate           =(normal && (lod_index<=0) && D.shaderModel()>=SM_5 && D.tesselation() && (!heightmap || D.tesselationHeightmap()));
   fx                  =(grass ? (m->hasGrass2D() ? FX_GRASS_2D : FX_GRASS_3D) : leaf ? (m->hasLeaf2D() ? (size ? FX_LEAFS_2D : FX_LEAF_2D) : (size ? FX_LEAFS_3D : FX_LEAF_3D)) : FX_NONE); // don't set FX_CLEAR_COAT here, because only Deferred shader supports it, and it's processed manually there
   clear_coat          =(m->technique==MTECH_CLEAR_COAT && normal && materials==1 && !heightmap);

   if(bump==SBUMP_ZERO){/*materials=1; can't return same shader for multi/single*/ layout=0; alpha_test=detail=macro=mtrl_blend=heightmap=false; fx=FX_NONE; MIN(emissive, 1);} // shaders with SBUMP_ZERO currently are very limited
   if(fx){detail=macro=tesselate=false; MIN(bump, SBUMP_NORMAL);} // shaders with effects currently don't support detail/macro/tesselate/fancy bump
   if(clear_coat){detail=macro=false; MIN(bump, SBUMP_NORMAL);} // shaders with clear coat currently don't support detail/macro/fancy bump
}
Shader* DefaultShaders::EarlyZ()C
{
#if SUPPORT_EARLY_Z
   if(valid && !alpha_blend && !alpha_test && !fx && !tesselate)return ShaderFiles("Early Z")->get(ShaderEarlyZ(skin));
#endif
   return null;
}
Shader* DefaultShaders::Opaque(Bool mirror)C
{
   if(valid && !alpha_blend && Renderer.anyDeferred())
   {
      // !! Never return the same shader for Multi-Materials as Single-Materials !!
      //if(fur)return ShaderFiles("Fur")->get(ShaderFurBase(skin, size, layout>0)); don't use FurBase because it's unfinished (doesn't use smooth/rough/normal maps) and the only difference is being affected by FurCol however difference is minimal
      Bool detail=T.detail, tesselate=T.tesselate; Byte bump=T.bump; if(mirror){detail=tesselate=false; MIN(bump, SBUMP_NORMAL);} // disable detail, tesselation and fancy bump for mirror
      return ShaderFiles("Deferred")->get(ShaderDeferred(skin, materials, layout, bump, alpha_test, detail, macro, color, mtrl_blend, heightmap, clear_coat ? FX_CLEAR_COAT : fx, tesselate));
   }
   return null;
}
Shader* DefaultShaders::Overlay()C
{
   if(valid && !fx)return ShaderFiles("Overlay")->get(ShaderOverlay(skin, tesselate));
   return null;
}
Shader* DefaultShaders::Emissive()C
{
#if SUPPORT_EMISSIVE
   if(valid && !alpha_blend && emissive)return ShaderFiles("Emissive")->get(ShaderEmissive(skin, color, alpha_test, emissive-1, fx, tesselate)); // for alpha_blend emissive is in the blend shader, emissive_map=(emissive==2) ? 1 : 0
#endif
   return null;
}
Shader* DefaultShaders::Outline()C
{
   if(valid && !alpha_blend && !fx)return ShaderFiles("Set Color")->get(ShaderSetColor(skin, color, alpha_test, tesselate));
   return null;
}
Shader* DefaultShaders::Behind()C
{
   if(valid && !fx)return ShaderFiles("Behind")->get(ShaderBehind(skin, color, alpha_test));
   return null;
}
Shader* DefaultShaders::Fur()C
{
   if(valid && fur)return ShaderFiles("Fur")->get(ShaderFurSoft(skin, size, layout>0));
   return null;
}
Shader* DefaultShaders::Shadow()C
{
   if(valid && (!alpha_blend || alpha_test))return ShaderFiles("Position")->get(ShaderPosition(skin, color, alpha_test, alpha_test && alpha_blend_light, fx, tesselate));
   return null;
}
Shader* DefaultShaders::Blend()C
{
   if(valid && alpha_blend_no_light) // "!alpha_blend_no_light" here will return null so BLST can be used in 'drawBlend'
      return ShaderFiles("Blend")->get(ShaderBlend(skin, color, layout, Min(bump, SBUMP_NORMAL), reflect, emissive>1)); // blend currently supports only up to normal mapping
   return null;
}
Shader* DefaultShaders::get(RENDER_MODE mode)C
{
   switch(mode)
   {
      default         : return null;
      case RM_EARLY_Z : return EarlyZ();
      case RM_OPAQUE  : return Opaque();
      case RM_OPAQUE_M: return Opaque(true);
      case RM_OVERLAY : return Overlay();
      case RM_EMISSIVE: return Emissive();
      case RM_OUTLINE : return Outline();
      case RM_BEHIND  : return Behind();
      case RM_FUR     : return Fur();
      case RM_SHADOW  : return Shadow();
      case RM_BLEND   : return Blend();
   }
}
FRST* DefaultShaders::Frst()C
{
   if(valid && !alpha_blend && Renderer.anyForward())
   {
      FRSTKey key;
      key.per_pixel   =(Renderer.forwardPrecision() && bump>SBUMP_ZERO);

      key.bump_mode   =Min(bump, key.per_pixel ? SBUMP_NORMAL : SBUMP_FLAT); // forward supports only up to normal mapping
      key.skin        =skin;
      key.materials   =materials;
      key.layout      =layout;
      key.alpha_test  =alpha_test;
      key.reflect     =reflect;
      key.emissive_map=(emissive>1);
      key.detail      =(detail && SUPPORT_FORWARD_DETAIL);
      key.color       =color;
      key.mtrl_blend  =mtrl_blend;
      key.fx          =fx;
      key.heightmap   =heightmap;
      key.tesselate   =(tesselate && SUPPORT_FORWARD_TESSELATE);
      return Frsts(key);
   }
   return null;
}
BLST* DefaultShaders::Blst()C
{
   if(valid
 //&& alpha_blend_light - always return because 'Mesh.drawBlend' may use it
   && normal // lighting requires vertex normals
   && materials==1
   )
   {
      BLSTKey key;
      key.per_pixel   =(((Renderer.type()==RT_FORWARD) ? Renderer.forwardPrecision() : true) && bump>SBUMP_ZERO);

      key.bump_mode   =Min(bump, key.per_pixel ? SBUMP_NORMAL : SBUMP_FLAT); // blend light currently supports only up to normal mapping
      key.color       =color;
      key.layout      =layout;
      key.alpha_test  =(alpha_test==ALPHA_TEST_YES); // BlendLight doesn't support ALPHA_TEST_DITHER
      key.alpha       =alpha;
      key.reflect     =reflect;
      key.emissive_map=(emissive>1);
      key.skin        =skin;
      key.fx          =fx;
      return Blsts(key);
   }
   return null;
}
void DefaultShaders::set(Shader *shader[RM_SHADER_NUM], FRST **frst, BLST **blst)
{
   if(shader)
   {
      shader[RM_EARLY_Z ]=EarlyZ();
      shader[RM_OPAQUE  ]=Opaque();
      shader[RM_OPAQUE_M]=Opaque(true);
      shader[RM_OVERLAY ]=Overlay();
      shader[RM_EMISSIVE]=Emissive();
      shader[RM_OUTLINE ]=Outline();
      shader[RM_BEHIND  ]=Behind();
      shader[RM_FUR     ]=Fur();
      shader[RM_SHADOW  ]=Shadow();
      shader[RM_BLEND   ]=Blend();
   }
   if(frst)*frst=Frst();
   if(blst)*blst=Blst();
}
/******************************************************************************/
}
/******************************************************************************/
