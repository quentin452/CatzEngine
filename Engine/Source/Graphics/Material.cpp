/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   #MaterialTextureLayout
   base_0: RGB, Alpha
   base_1: NrmX, NrmY
   base_2: Metal, Rough, Bump, Glow
   detail: NrmX, NrmY, Rough, Col   #MaterialTextureLayoutDetail

   When changing the above to a different order, then look for "#MaterialTextureLayout" text in Engine/Editor to update the codes.

   In a lot of cases only Smooth is used without Metal, and if Smooth was stored in Red/X channel, then we could use BC4 texture with Metal in Green/Y channel as 0.
      However when Smooth and Metal are used together, then they need 2 channels, and to reduce texture sizes, BC1 RGB is used (with only 4-bits per pixel), instead of BC5 RG (8-bits).
      BC1 offers better quality for Green channel (6 bits) compared to Red (5 bits), and Smooth needs more precision, because Metal in most cases is 0.0 or 1.0, so it's better to put Smooth in Green and Metal to Red.

   https://seblagarde.wordpress.com/2011/08/17/feeding-a-physical-based-lighting-mode/
   https://google.github.io/filament/Filament.html#table_commonmatreflectance

   Water Reflectivity, Index of Refraction (IOR) of Water is 1.33:
      F(0)=Sqr(1.33-1)/Sqr(1.33+1)=0.020059312=~0.02 (2%)
   Reflectivity:
      Ice              0.018
      Water            0.02
      Eye              0.025
      Skin             0.028
      Plant,Leaf,Grass 0.035
      Rock, Bark       0.039
      Glass            0.04
      Plastic          0.04 .. 0.05
      Fabric           0.04 .. 0.056
      Sand             0.046
      Hair             0.047
      Teeth            0.058
      Asphalt          0.06
      Ruby             0.077271957
      Crystal          0.111111111
      Diamond          0.171968833

   Metals Linear Specular/Color:
                  R           G           B
      Iron        0.563       0.579       0.579
      Silver      0.971519    0.959915    0.915324
      Aluminium   0.913183    0.921494    0.924524
      Gold        1           0.765557    0.336057
      Copper      0.955008    0.637427    0.538163
      Chromium    0.549585    0.556114    0.554256
      Nickel      0.659777    0.608679    0.525649
      Titanium    0.541931    0.496791    0.449419
      Cobalt      0.662124    0.654864    0.633732
      Platinum    0.672411    0.637331    0.585456

/******************************************************************************/
#define CC4_MTRL CC4('M','T','R','L')

constexpr Byte TexSmooth(Byte tx) {return TEX_IS_ROUGH ? 255-tx : tx;} // convert between texture and smoothness

#define TEX_DEFAULT_SMOOTH 0 // 0..255
#define TEX_DEFAULT_METAL  0 // 0..255
#define TEX_DEFAULT_BUMP   0 // 0..255, normally this should be 128, but 0 will allow to use BC4/BC5 (for Mtrl.base_2 tex if there's no Glow) and always set Material.bump=0 when bump is not used #MaterialTextureLayout
/******************************************************************************/
static Int Compare(C UniqueMultiMaterialKey &a, C UniqueMultiMaterialKey &b)
{
   if(a.m[0]<b.m[0])return -1; if(a.m[0]>b.m[0])return +1;
   if(a.m[1]<b.m[1])return -1; if(a.m[1]>b.m[1])return +1;
   if(a.m[2]<b.m[2])return -1; if(a.m[2]>b.m[2])return +1;
   if(a.m[3]<b.m[3])return -1; if(a.m[3]>b.m[3])return +1;
                                                return  0;
}
/******************************************************************************/
DEFINE_CACHE(Material, Materials, MaterialPtr, "Material");
Material               MaterialDefault,
                       MaterialDefaultNoCull;
const Material        *MaterialLast,
                      *MaterialLast4[4]; // can't merge with 'MaterialLast' because that sets 'Sh.Material' but this sets 'Sh.MultiMaterial'
MaterialPtr            MaterialNull;
ThreadSafeMap<UniqueMultiMaterialKey, UniqueMultiMaterialData> UniqueMultiMaterialMap(Compare);
/******************************************************************************/
Bool HasAlpha(MATERIAL_TECHNIQUE technique)
{
   switch(technique)
   {
      case MTECH_ALPHA_TEST:
      case MTECH_ALPHA_TEST_DITHER:
      case MTECH_GRASS:
      case MTECH_GRASS_3D:
      case MTECH_LEAF_2D:
      case MTECH_LEAF:
      case MTECH_BLEND:
      case MTECH_BLEND_LIGHT:
      case MTECH_BLEND_LIGHT_GRASS:
      case MTECH_BLEND_LIGHT_LEAF:
      case MTECH_TEST_BLEND_LIGHT:
      case MTECH_TEST_BLEND_LIGHT_GRASS:
      case MTECH_TEST_BLEND_LIGHT_LEAF:
      case MTECH_DEPTH_BLEND:
         return true;

      default: return false;
   }
}
Bool HasAlphaTest(MATERIAL_TECHNIQUE technique)
{
   switch(technique)
   {
      case MTECH_ALPHA_TEST            :
      case MTECH_ALPHA_TEST_DITHER     :
      case MTECH_GRASS                 :
      case MTECH_GRASS_3D              :
      case MTECH_LEAF_2D               :
      case MTECH_LEAF                  :
      case MTECH_TEST_BLEND_LIGHT      :
      case MTECH_TEST_BLEND_LIGHT_GRASS:
      case MTECH_TEST_BLEND_LIGHT_LEAF :
         return true;

      default: return false;
   }
}
Bool HasAlphaTestDither(MATERIAL_TECHNIQUE technique)
{
   switch(technique)
   {
      case MTECH_ALPHA_TEST_DITHER:
         return true;

      default: return false;
   }
}
Bool HasAlphaTestNoBlend(MATERIAL_TECHNIQUE technique)
{
   switch(technique)
   {
      case MTECH_ALPHA_TEST       :
      case MTECH_ALPHA_TEST_DITHER:
      case MTECH_GRASS            :
      case MTECH_GRASS_3D         :
      case MTECH_LEAF_2D          :
      case MTECH_LEAF             :
         return true;

      default: return false;
   }
}
Bool HasAlphaBlend(MATERIAL_TECHNIQUE technique)
{
   switch(technique)
   {
      case MTECH_BLEND:
      case MTECH_BLEND_LIGHT:
      case MTECH_BLEND_LIGHT_GRASS:
      case MTECH_BLEND_LIGHT_LEAF:
      case MTECH_TEST_BLEND_LIGHT:
      case MTECH_TEST_BLEND_LIGHT_GRASS:
      case MTECH_TEST_BLEND_LIGHT_LEAF:
      case MTECH_DEPTH_BLEND:
         return true;

      default: return false;
   }
}
Bool HasAlphaBlendNoTest(MATERIAL_TECHNIQUE technique)
{
   switch(technique)
   {
      case MTECH_BLEND:
      case MTECH_BLEND_LIGHT:
      case MTECH_BLEND_LIGHT_GRASS:
      case MTECH_BLEND_LIGHT_LEAF:
      case MTECH_DEPTH_BLEND:
         return true;

      default: return false;
   }
}
Bool HasLeaf(MATERIAL_TECHNIQUE technique)
{
   switch(technique)
   {
      case MTECH_LEAF_2D:
      case MTECH_LEAF:
      case MTECH_BLEND_LIGHT_LEAF:
      case MTECH_TEST_BLEND_LIGHT_LEAF:
         return true;

      default: return false;
   }
}
Bool HasDepthWrite(MATERIAL_TECHNIQUE technique)
{
   switch(technique)
   {
      case MTECH_OPAQUE                :
      case MTECH_ALPHA_TEST            :
      case MTECH_ALPHA_TEST_DITHER     :
      case MTECH_FUR                   :
      case MTECH_GRASS                 :
      case MTECH_LEAF                  :
      case MTECH_TEST_BLEND_LIGHT      :
      case MTECH_TEST_BLEND_LIGHT_GRASS:
      case MTECH_TEST_BLEND_LIGHT_LEAF :
      case MTECH_GRASS_3D              :
      case MTECH_LEAF_2D               :
      case MTECH_DEPTH_BLEND           :
      case MTECH_CLEAR_COAT            :
         return true;
      default: return false;
   }
}
/******************************************************************************/
Vec4  MaterialParams::colorS    (                  )C {return           LinearToSRGB(   color_l) ;}
void  MaterialParams::colorS    (C Vec4 &color_s   )  {return    colorL(SRGBToLinear(   color_s));}
Vec   MaterialParams::emissiveS (                  )C {return           LinearToSRGB(emissive_l) ;}
void  MaterialParams::emissiveS (C Vec  &emissive_s)  {return emissiveL(SRGBToLinear(emissive_s));}
void  MaterialParams::reflect   (  Flt   reflect   )  {reflect_add=reflect; reflect_mul= 1-reflect     ;}
void XMaterial      ::reflect   (  Flt   reflect   )  {reflect_add=reflect; reflect_mul= 1-reflect     ;}
void  MaterialParams::reflect   (  Flt min, Flt max)  {reflect_add=    min; reflect_mul=(1-min    )*max;}
void XMaterial      ::reflect   (  Flt min, Flt max)  {reflect_add=    min; reflect_mul=(1-min    )*max;}
Flt   MaterialParams::reflectMax(                  )C {Flt div=(1-reflect_add); return Equal(div, 0) ? 1 : reflect_mul/div;}
Flt  XMaterial      ::reflectMax(                  )C {Flt div=(1-reflect_add); return Equal(div, 0) ? 1 : reflect_mul/div;}
/******************************************************************************/
Material::Material()
{
      color_l.set(1, 1, 1, 1);
   emissive_l.set(0, 0, 0);
   emissive_glow=0;
   rough_mul    =0; rough_add=1;
   reflect      (MATERIAL_REFLECT);
   glow         =0;
   normal       =0;
   bump         =0;
   det_power    =0.3f;
   det_uv_scale =4;
       uv_scale =1.0f;

   cull          =true;
   detail_all_lod=false;
   technique     =MTECH_OPAQUE;

   clear();
   validate();
}
Material::~Material()
{
#if !SYNC_LOCK_SAFE // if 'SyncLock' is not safe then crash may occur when trying to lock, to prevent that, check if we have any elements (this means cache was already initialized)
   if(UniqueMultiMaterialMap.elms())
#endif
   {
      UniqueMultiMaterialMap.lock  (); REPA(UniqueMultiMaterialMap){C UniqueMultiMaterialKey &key=UniqueMultiMaterialMap.lockedKey(i); if(key.m[0]==this || key.m[1]==this || key.m[2]==this || key.m[3]==this)UniqueMultiMaterialMap.lockedRemove(i);}
      UniqueMultiMaterialMap.unlock();
   }
}
/******************************************************************************/
Bool Material::hasAlphaBlendNoLight()C
{
   switch(technique)
   {
      case MTECH_BLEND:
      case MTECH_DEPTH_BLEND:
         return true;

      default: return false;
   }
}
Bool Material::hasAlphaBlendLight()C
{
   switch(technique)
   {
      case MTECH_BLEND_LIGHT:
      case MTECH_BLEND_LIGHT_GRASS:
      case MTECH_BLEND_LIGHT_LEAF:
      case MTECH_TEST_BLEND_LIGHT:
      case MTECH_TEST_BLEND_LIGHT_GRASS:
      case MTECH_TEST_BLEND_LIGHT_LEAF:
         return true;

      default: return false;
   }
}
Bool Material::hasGrass()C
{
   switch(technique)
   {
      case MTECH_GRASS:
      case MTECH_GRASS_3D:
      case MTECH_BLEND_LIGHT_GRASS:
      case MTECH_TEST_BLEND_LIGHT_GRASS:
         return true;

      default: return false;
   }
}
Bool Material::hasGrass2D()C
{
   switch(technique)
   {
      case MTECH_GRASS:
      case MTECH_BLEND_LIGHT_GRASS:
      case MTECH_TEST_BLEND_LIGHT_GRASS:
         return true;

      default: return false;
   }
}
Bool Material::hasGrass3D()C
{
   switch(technique)
   {
      case MTECH_GRASS_3D:
         return true;

      default: return false;
   }
}
Bool Material::hasLeaf2D()C
{
   switch(technique)
   {
      case MTECH_LEAF_2D:
         return true;

      default: return false;
   }
}
Bool Material::hasLeaf3D()C
{
   switch(technique)
   {
      case MTECH_LEAF:
      case MTECH_BLEND_LIGHT_LEAF:
      case MTECH_TEST_BLEND_LIGHT_LEAF:
         return true;

      default: return false;
   }
}
/******************************************************************************/
Bool Material::needTanBin()C
{
   // #MaterialTextureLayout
   return (base_1     && normal   >EPS_COL8         )  // normal        is in base_1
       || (base_2     && bump     >EPS_MATERIAL_BUMP)  // bump          is in base_2
       || (detail_map && det_power>EPS_COL8         ); // normal detail is in DetailMap #MaterialTextureLayoutDetail
}
/******************************************************************************/
Material& Material::reset   () {T=MaterialDefault; return validate();}
Material& Material::validate() // #MaterialTextureLayout
{
                      if(this==MaterialLast    )MaterialLast    =null;
   REPA(MaterialLast4)if(this==MaterialLast4[i])MaterialLast4[i]=null;

  _has_alpha_test=HasAlphaTest       (technique); // !! call 'HasAlphaTest'  instead of 'hasAlphaTest'  because that one just uses '_has_alpha_test' which we're setting here !!
  _depth_write   =HasDepthWrite      (technique); // !! call 'HasDepthWrite' instead of 'hasDepthWrite' because that one just uses '_depth_write'    which we're setting here !!
//_coverage      =HasAlphaTestNoBlend(technique);
  _alpha_factor.set(0, 0, 0, FltToByte(T.glow));

  _has=0;
   if(_alpha_factor.a || emissive_glow>EPS_COL8)_has|=HAS_GLOW;
   if(technique==MTECH_CLEAR_COAT              )_has|=HAS_CLEAR_COAT;
   if(technique==MTECH_FUR                     )_has|=HAS_FUR;

   // set multi
   {
     _multi.color       =    colorD();
     _multi.    uv_scale=    uv_scale;
     _multi.det_uv_scale=det_uv_scale;

      // normal map
      if(base_1)
      {
        _multi.normal=normal;
      }else
      {
        _multi.normal=0;
      }

      // base2
      if(base_2)
      {
        _multi.refl_rogh_glow_mul.x=reflect_mul;
        _multi.refl_rogh_glow_add.x=reflect_add;

        _multi.refl_rogh_glow_mul.y=rough_mul;
        _multi.refl_rogh_glow_add.y=rough_add;

        _multi.refl_rogh_glow_mul.z=glow;
        _multi.refl_rogh_glow_add.z=0;

        _multi.bump=bump;
      }else
      {
        _multi.refl_rogh_glow_mul=0;
        _multi.refl_rogh_glow_add.x=reflect_add;
        _multi.refl_rogh_glow_add.y=  rough_add;
        _multi.refl_rogh_glow_add.z=glow;

        _multi.bump=0;
      }

      // #MaterialTextureLayoutDetail
      // XY=nrm.xy -1..1 delta, Z=rough -1..1 delta, W=color 0..2 scale
      // SINGLE: det.xyz=det.xyz*(Material.det_power*2)+( -Material.det_power); -1..1
      // SINGLE: det.w  =det.w  *(Material.det_power*2)+(1-Material.det_power);  0..2
      // MULTI : det.xyz=det.xyz*MultiMaterial0.det_mul+MultiMaterial0.det_add; -1..1
      // MULTI : det.w  =det.w  *MultiMaterial0.det_mul+MultiMaterial0.det_inv;  0..2
      if(detail_map)
      {
        _multi.det_mul=  det_power*2;
        _multi.det_add= -det_power  ;
        _multi.det_inv=1-det_power  ;
      }else
      { // same as above with det_power=0
        _multi.det_mul=0;
        _multi.det_add=0;
        _multi.det_inv=1;
      }
     _multi.macro=(macro_map!=null);
   }
   return T;
}
static inline void WaitForStream(Bool &ok, C ImagePtr &image)
{
   if(image)ok&=image->waitForStream();
}
Bool Material::waitForStream()C
{
   Bool ok=true;
   WaitForStream(ok,   base_0    );
   WaitForStream(ok,   base_1    );
   WaitForStream(ok,   base_2    );
   WaitForStream(ok,   detail_map);
   WaitForStream(ok,    macro_map);
   WaitForStream(ok, emissive_map);
   return ok;
}
/******************************************************************************/
void Material::setOpaque()C
{
   if(MaterialLast!=this)
   {
      MaterialLast    =this;
      MaterialLast4[0]=null; // because they use the same shader images

      Renderer._has|=_has;
      Sh.Col[0]  ->set(      base_0());
      Sh.Nrm[0]  ->set(      base_1());
      Sh.Ext[0]  ->set(      base_2());
      Sh.Det[0]  ->set(  detail_map());
      Sh.Mac[0]  ->set(   macro_map());
      Sh.Lum     ->set(emissive_map());
      Sh.Material->set<MaterialParams>(T);
   #if !LINEAR_GAMMA
      Renderer.material_color_l->set(colorS());
   #endif
   }
}
void Material::setEmissive()C
{
   if(MaterialLast!=this)
   {
      MaterialLast    =this;
      MaterialLast4[0]=null; // because they use the same shader images

      // textures needed for alpha-test #MaterialTextureLayout
    //Renderer._has|=_has; already processed in 'setOpaque'
      Sh.Col[0]  ->set(base_0      ());
      Sh.Lum     ->set(emissive_map());
      Sh.Material->set<MaterialParams>(T); // params needed for alpha-test and emissive
   #if !LINEAR_GAMMA
      Renderer.material_color_l->set(colorS());
   #endif
   }
}
void Material::setBlend()C
{
   if(MaterialLast!=this)
   {
      MaterialLast=this;
    //MaterialLast4[0]=null; not needed since multi materials not rendered in blend mode

      D.alphaFactor(_alpha_factor); if(_alpha_factor.a)Renderer._has|=HAS_GLOW; // here operate only on '_alpha_factor.a' instead of '_has' because blend shaders use glow only from alpha factor but not 'emissive_glow'

      Sh.Col[0]  ->set(      base_0());
      Sh.Nrm[0]  ->set(      base_1());
      Sh.Ext[0]  ->set(      base_2());
      Sh.Det[0]  ->set(  detail_map());
      Sh.Mac[0]  ->set(   macro_map());
      Sh.Lum     ->set(emissive_map());
      Sh.Material->set<MaterialParams>(T);
   #if !LINEAR_GAMMA
      Renderer.material_color_l->set(colorS());
   #endif
   }
}
void Material::setBlendForce()C
{
   if(_alpha_factor.a // if has glow in material settings
   && base_2) // and on texture channel (glow is in Base2 #MaterialTextureLayout)
   { // then it means we need to disable it for forced blend, which operates on alpha instead of glow
      if(MaterialLast==this)MaterialLast=null;
      D.alphaFactor(TRANSPARENT);
   }else
   {
      if(MaterialLast==this)return;
         MaterialLast= this;

      D.alphaFactor(_alpha_factor); if(_alpha_factor.a)Renderer._has|=HAS_GLOW; // here operate only on '_alpha_factor.a' instead of '_has' because blend shaders use glow only from alpha factor but not 'emissive_glow'
   }

   Sh.Col[0]  ->set(      base_0());
   Sh.Nrm[0]  ->set(      base_1());
   Sh.Ext[0]  ->set(      base_2());
   Sh.Det[0]  ->set(  detail_map());
   Sh.Mac[0]  ->set(   macro_map());
   Sh.Lum     ->set(emissive_map());
   Sh.Material->set<MaterialParams>(T);
#if !LINEAR_GAMMA
   Renderer.material_color_l->set(colorS());
#endif
}
void Material::setOutline()C
{
   if(MaterialLast!=this)
   {
      MaterialLast=this;
    //MaterialLast4[0]=null; not needed since multi materials not rendered in outline mode
      // textures needed for alpha-test #MaterialTextureLayout
      Sh.Col[0]->set(base_0());
      Renderer.material_color_l->set(colorD()); // only Material Color is used for alpha-testing
   }
}
void Material::setBehind()C
{
   if(MaterialLast!=this)
   {
      MaterialLast=this;
    //MaterialLast4[0]=null; not needed since multi materials not rendered in behind mode
      // textures needed for alpha-test #MaterialTextureLayout
      Sh.Col[0]->set(base_0());
      Renderer.material_color_l->set(colorD()); // only Material Color is used for alpha-testing
   }
}
void Material::setShadow()C
{
   if(hasAlphaTest() && MaterialLast!=this) // this shader needs params/textures only for alpha test (if used)
   {
      MaterialLast=this;
    //MaterialLast4[0]=null; not needed since multi materials don't have alpha test and don't need to set values in shadow mode
      // textures needed for alpha-test #MaterialTextureLayout
      Sh.Col[0]->set(base_0());
      Renderer.material_color_l->set(colorD()); // only Material Color is used for alpha-testing
   }
}
void Material::setMulti(Int i)C
{
   DEBUG_RANGE_ASSERT(i, Sh.MultiMaterial);
   if(MaterialLast4[i]!=this)
   {
            MaterialLast4[i]=this;
      if(!i)MaterialLast    =null; // because they use the same shader images

      Renderer._has|=_has;

      Sh.Col          [i]->set(  base_0  ());
      Sh.Nrm          [i]->set(  base_1  ());
      Sh.Ext          [i]->set(  base_2  ());
      Sh.Det          [i]->set(detail_map());
      Sh.Mac          [i]->set( macro_map());
      Sh.MultiMaterial[i]->set(_multi      );
   }
}
void Material::setAuto()C
{
   switch(Renderer())
   {
      case RM_PREPARE :
      case RM_OPAQUE  :
      case RM_OPAQUE_M: setOpaque(); break;

      case RM_EMISSIVE: setEmissive(); break;

      case RM_FUR     :
      case RM_BLEND   :
      case RM_PALETTE :
      case RM_PALETTE1:
      case RM_OVERLAY : setBlend(); break;

      case RM_BEHIND  : setBehind(); break;

      case RM_OUTLINE : setOutline(); break;

      case RM_EARLY_Z :
      case RM_SHADOW  : setShadow(); break;
   }
}
/******************************************************************************/
Bool Material::saveData(File &f, CChar *path)C
{
   f.putMulti(Byte(14), cull, detail_all_lod, technique)<<SCAST(C MaterialParams, T); // version

   // textures
   f.putStr(      base_0.name(path)); // !! can't use 'id' because textures are stored in "Tex/" folder, so there's no point in using 'putAsset' !!
   f.putStr(      base_1.name(path)); // !! can't use 'id' because textures are stored in "Tex/" folder, so there's no point in using 'putAsset' !!
   f.putStr(      base_2.name(path)); // !! can't use 'id' because textures are stored in "Tex/" folder, so there's no point in using 'putAsset' !!
   f.putStr(  detail_map.name(path)); // !! can't use 'id' because textures are stored in "Tex/" folder, so there's no point in using 'putAsset' !!
   f.putStr(   macro_map.name(path)); // !! can't use 'id' because textures are stored in "Tex/" folder, so there's no point in using 'putAsset' !!
   f.putStr(emissive_map.name(path)); // !! can't use 'id' because textures are stored in "Tex/" folder, so there's no point in using 'putAsset' !!

   return f.ok();
}
Bool Material::loadData(File &f, CChar *path)
{
   MaterialParams &mp=T; Char temp[MAX_LONG_PATH]; Flt sss, smooth, reflect;
   switch(f.decUIntV())
   {
      case 14:
      {
         f.getMulti(cull, detail_all_lod, technique)>>mp;
         f.getStr(temp);       base_0.require(temp, path); // base_0 is RGBA
         f.getStr(temp);       base_1.require(temp, path); // base_1 is NormalXY
         f.getStr(temp);       base_2.require(temp, path); // base_2 is Metal, Rough, Bump, Glow
         f.getStr(temp);   detail_map.require(temp, path);
         f.getStr(temp);    macro_map.require(temp, path);
         f.getStr(temp); emissive_map.require(temp, path);
      }break;

      case 13:
      {
         f.getMulti(cull, technique)>>mp; detail_all_lod=false;
         f.getStr(temp);       base_0.require(temp, path); // base_0 is RGBA
         f.getStr(temp);       base_1.require(temp, path); // base_1 is NormalXY
         f.getStr(temp);       base_2.require(temp, path); // base_2 is Metal, Rough, Bump, Glow
         f.getStr(temp);   detail_map.require(temp, path);
         f.getStr(temp);    macro_map.require(temp, path);
         f.getStr(temp); emissive_map.require(temp, path);
      }break;

      case 12:
      {
         f.getMulti(cull, technique)>>color_l>>emissive_l>>rough_mul>>rough_add>>reflect_mul>>reflect_add>>glow>>normal>>bump>>det_power>>det_uv_scale>>uv_scale; emissive_glow=0; detail_all_lod=false;
         f.getStr(temp);       base_0.require(temp, path); // base_0 is RGBA
         f.getStr(temp);       base_1.require(temp, path); // base_1 is NormalXY
         f.getStr(temp);       base_2.require(temp, path); // base_2 is Metal, Rough, Bump, Glow
         f.getStr(temp);   detail_map.require(temp, path);
         f.getStr(temp);    macro_map.require(temp, path);
         f.getStr(temp); emissive_map.require(temp, path);
      }break;

      case 11:
      {
         f.getMulti(cull, technique)>>color_l>>emissive_l>>smooth>>reflect_mul>>reflect_add>>glow>>normal>>bump>>det_power>>det_uv_scale>>uv_scale; emissive_glow=0; detail_all_lod=false;
         f.getStr(temp);       base_0.require(temp, path); // base_0 is RGBA
         f.getStr(temp);       base_1.require(temp, path); // base_1 is NormalXY
         f.getStr(temp);       base_2.require(temp, path); if(Is(temp)){T.rough_add=1; T.rough_mul=-smooth;}else{T.rough_add=1-smooth; T.rough_mul=0;} // base_2 is Metal, Smooth, Bump, Glow
         f.getStr(temp);   detail_map.require(temp, path);
         f.getStr(temp);    macro_map.require(temp, path);
         f.getStr(temp); emissive_map.require(temp, path);
      }break;

      case 10:
      {
         f.getMulti(cull, technique)>>color_l>>emissive_l>>smooth>>reflect>>glow>>normal>>bump>>det_power>>det_uv_scale>>uv_scale; emissive_glow=0; detail_all_lod=false;
         f.getStr(temp);       base_0.require(temp, path); // base_0 is RGBA
         f.getStr(temp);       base_1.require(temp, path); // base_1 is NormalXY
         f.getStr(temp);       base_2.require(temp, path); if(Is(temp)){T.rough_add=1; T.rough_mul=0; T.reflect_add=MATERIAL_REFLECT; T.reflect_mul=0;}else{T.rough_add=1-smooth; T.rough_mul=0; T.reflect_add=reflect; T.reflect_mul=0;} // base_2 is Smooth, Reflectivity, Bump, Glow
         f.getStr(temp);   detail_map.require(temp, path);
         f.getStr(temp);    macro_map.require(temp, path);
         f.getStr(temp); emissive_map.require(temp, path);
      }break;

      case 9:
      {
         f.getMulti(cull, technique)>>color_l>>emissive_l>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect; colorS(color_l); emissive_glow=0; detail_all_lod=false;
         f.getStr(temp);       base_0.require(temp, path);
         f.getStr(temp);       base_1.require(temp, path); if(Is(temp)){T.rough_add=1; T.rough_mul=0;}else{T.rough_add=1-smooth; T.rough_mul=0;} // base_1 had normals and specular
                               base_2=null;
         f.getStr(temp);   detail_map.require(temp, path);
         f.getStr(temp);    macro_map.require(temp, path);
         f.getStr(temp);
         f.getStr(temp); emissive_map.require(temp, path);
         f.getStr(temp);
         f.getStr(temp);
         T.reflect_add=MATERIAL_REFLECT; T.reflect_mul=0; T.normal=0;
      }break;

      case 8:
      {
         f.getMulti(cull, technique)>>color_l>>emissive_l>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect; colorS(color_l); emissive_glow=0; detail_all_lod=false;
         f._getStr1(temp);       base_0.require(temp, path);
         f._getStr1(temp);       base_1.require(temp, path); if(Is(temp)){T.rough_add=1; T.rough_mul=0;}else{T.rough_add=1-smooth; T.rough_mul=0;} // base_1 had normals and specular
                                 base_2=null;
         f._getStr1(temp);   detail_map.require(temp, path);
         f._getStr1(temp);    macro_map.require(temp, path);
         f._getStr1(temp);
         f._getStr1(temp); emissive_map.require(temp, path);
         f._getStr1(temp);
         f._getStr1(temp);
         T.reflect_add=MATERIAL_REFLECT; T.reflect_mul=0; T.normal=0;
      }break;

      case 7:
      {
         f>>color_l>>emissive_l>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect>>cull>>technique; colorS(color_l); emissive_glow=0; detail_all_lod=false;
         f._getStr(temp);       base_0.require(temp, path);
         f._getStr(temp);       base_1.require(temp, path); if(Is(temp)){T.rough_add=1; T.rough_mul=0;}else{T.rough_add=1-smooth; T.rough_mul=0;} // base_1 had normals and specular
                                base_2=null;
         f._getStr(temp);   detail_map.require(temp, path);
         f._getStr(temp);    macro_map.require(temp, path);
         f._getStr(temp);
         f._getStr(temp); emissive_map.require(temp, path);
         f._getStr(temp);
         f._getStr(temp);
         T.reflect_add=MATERIAL_REFLECT; T.reflect_mul=0; T.normal=0;
      }break;

      case 6:
      {
         f>>color_l>>emissive_l>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect>>cull>>technique; colorS(color_l); emissive_glow=0; detail_all_lod=false;
         f._getStr(temp);       base_0.require(temp, path);
         f._getStr(temp);       base_1.require(temp, path); if(Is(temp)){T.rough_add=1; T.rough_mul=0;}else{T.rough_add=1-smooth; T.rough_mul=0;} // base_1 had normals and specular
                                base_2=null;
         f._getStr(temp);   detail_map.require(temp, path);
         f._getStr(temp);    macro_map.require(temp, path);
         f._getStr(temp);
         f._getStr(temp); emissive_map.require(temp, path);
         f._getStr8();
         T.reflect_add=MATERIAL_REFLECT; T.reflect_mul=0; T.normal=0;
      }break;

      case 5:
      {
         f>>color_l>>emissive_l>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect>>cull>>technique; colorS(color_l); emissive_glow=0; detail_all_lod=false;
         f._getStr(temp);       base_0.require(temp, path);
         f._getStr(temp);       base_1.require(temp, path); if(Is(temp)){T.rough_add=1; T.rough_mul=0;}else{T.rough_add=1-smooth; T.rough_mul=0;} // base_1 had normals and specular
                                base_2=null;
         f._getStr(temp);   detail_map.require(temp, path);
         f._getStr(temp);
         f._getStr(temp); emissive_map.require(temp, path);
                             macro_map=null;
         f._getStr8();
         T.reflect_add=MATERIAL_REFLECT; T.reflect_mul=0; T.normal=0;
      }break;

      case 4:
      {
         f>>color_l>>emissive_l>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect>>cull>>technique; colorS(color_l); emissive_glow=0; detail_all_lod=false;
         f._getStr(temp);       base_0.require(temp, path);
         f._getStr(temp);       base_1.require(temp, path); if(Is(temp)){T.rough_add=1; T.rough_mul=0;}else{T.rough_add=1-smooth; T.rough_mul=0;} // base_1 had normals and specular
                                base_2=null;
         f._getStr(temp);   detail_map.require(temp, path);
         f._getStr(temp);
         f._getStr(temp); emissive_map.require(temp, path);
                             macro_map=null;
         T.reflect_add=MATERIAL_REFLECT; T.reflect_mul=0; T.normal=0;
      }break;

      case 3:
      {
         f>>color_l>>emissive_l>>smooth>>sss>>glow>>normal>>bump>>det_uv_scale>>det_power>>reflect>>cull>>technique; uv_scale=1; colorS(color_l); emissive_glow=0; detail_all_lod=false;
                base_0.require(f._getStr8(), path);
                base_1.require(f._getStr8(), path); if(Is(temp)){T.rough_add=1; T.rough_mul=0;}else{T.rough_add=1-smooth; T.rough_mul=0;} // base_1 had normals and specular
                base_2=null;
            detail_map.require(f._getStr8(), path);
                     Str8 temp=f._getStr8();
          emissive_map.require(f._getStr8(), path);
             macro_map=null;
         T.reflect_add=MATERIAL_REFLECT; T.reflect_mul=0; T.normal=0;
      }break;

      case 2:
      {
         f.skip(1);
         f>>color_l>>smooth>>sss>>glow>>normal>>bump>>det_uv_scale>>det_power>>reflect>>cull>>technique; emissive_l=0; uv_scale=1; colorS(color_l); emissive_glow=0; detail_all_lod=false;
         if(technique==MTECH_FUR){det_power=color_l.w; color_l.w=1;}
             base_0.require(f._getStr8(), path);
             base_1.require(f._getStr8(), path); if(Is(temp)){T.rough_add=1; T.rough_mul=0;}else{T.rough_add=1-smooth; T.rough_mul=0;} // base_1 had normals and specular
             base_2=null;
         detail_map.require(f._getStr8(), path);
                  Str8 temp=f._getStr8();
          emissive_map=null;
             macro_map=null;
         T.reflect_add=MATERIAL_REFLECT; T.reflect_mul=0; T.normal=0;
      }break;

      case 1:
      {
         f.skip(1);
         f>>color_l>>smooth>>glow>>normal>>bump>>det_uv_scale>>det_power>>reflect>>cull>>technique; sss=0; emissive_l=0; uv_scale=1; colorS(color_l); emissive_glow=0; detail_all_lod=false;
         if(technique==MTECH_FUR){det_power=color_l.w; color_l.w=1;}
             base_0.require(f._getStr8(), path);
             base_1.require(f._getStr8(), path); if(Is(temp)){T.rough_add=1; T.rough_mul=0;}else{T.rough_add=1-smooth; T.rough_mul=0;} // base_1 had normals and specular
             base_2=null;
         detail_map.require(f._getStr8(), path);
                  Str8 temp=f._getStr8();
          emissive_map=null;
             macro_map=null;
         T.reflect_add=MATERIAL_REFLECT; T.reflect_mul=0; T.normal=0;
      }break;

      case 0:
      {
         f.skip(1);
         f>>color_l>>smooth>>glow>>normal>>bump>>det_uv_scale>>det_power>>reflect>>cull; sss=0; emissive_l=0; uv_scale=1; colorS(color_l); emissive_glow=0; detail_all_lod=false;
         switch(f.getByte())
         {
            default: technique=MTECH_OPAQUE    ; break;
            case 1 : technique=MTECH_ALPHA_TEST; break;
            case 4 : technique=MTECH_FUR       ; break;
            case 5 : technique=MTECH_GRASS     ; break;
         }
         if(technique==MTECH_FUR){det_power=color_l.w; color_l.w=1;}
         Char8 temp[80];
         f>>temp;     base_0.require(temp, path);
         f>>temp;     base_1.require(temp, path); if(Is(temp)){T.rough_add=1; T.rough_mul=0;}else{T.rough_add=1-smooth; T.rough_mul=0;} // base_1 had normals and specular
                      base_2=null;
         f>>temp; detail_map.require(temp, path);
         f>>temp;
                emissive_map=null;
                   macro_map=null;
         T.reflect_add=MATERIAL_REFLECT; T.reflect_mul=0; T.normal=0;
      }break;

      default: goto error;
   }
   if(f.ok()){validate(); return true;}
error:
   reset(); return false;
}
/******************************************************************************/
Bool Material::save(File &f, CChar *path)C
{
   f.putUInt(CC4_MTRL);
   return saveData(f, path);
}
Bool Material::load(File &f, CChar *path)
{
   if(f.getUInt()==CC4_MTRL)return loadData(f, path);
   reset(); return false;
}

Bool Material::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool Material::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   reset(); return false;
}
/******************************************************************************/
void MaterialClear() // must be called: after changing 'Renderer.mode', after changing 'D.alphaFactor', or material parameters/textures
{
         MaterialLast  =null;
   REPAO(MaterialLast4)=null;
}
/******************************************************************************/
static Int ImgW(C ImageSource &src, C Image *img) {return (!img->is()) ? 0 : (src.size.x>0) ? src.size.x : img->w();}
static Int ImgH(C ImageSource &src, C Image *img) {return (!img->is()) ? 0 : (src.size.y>0) ? src.size.y : img->h();}

static FILTER_TYPE Filter(Int filter) {return InRange(filter, FILTER_NUM) ? FILTER_TYPE(filter) : FILTER_BEST;}

TEX_FLAG CreateBaseTextures(Image &base_0, Image &base_1, Image &base_2, C ImageSource &color, C ImageSource &alpha, C ImageSource &bump, C ImageSource &normal, C ImageSource &smooth, C ImageSource &metal, C ImageSource &glow, Bool resize_to_pow2, Bool flip_normal_y, Bool smooth_is_rough)
{ // #MaterialTextureLayout
   TEX_FLAG texf=TEXF_NONE;
   Image dest_0, dest_1, dest_2;
   {
      Image  color_temp; C Image * color_src=& color.image; if( color_src->compressed())if( color_src->copyTry( color_temp, -1, -1, -1,  color_src->typeInfo().a    ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8_SRGB, IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA)) color_src=& color_temp;else goto error; // keep alpha because we might use it for alpha and IC_ALPHA_WEIGHT
      Image  alpha_temp; C Image * alpha_src=& alpha.image; if( alpha_src->compressed())if( alpha_src->copyTry( alpha_temp, -1, -1, -1,  alpha_src->typeInfo().a    ? IMAGE_L8A8          : IMAGE_L8         , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA)) alpha_src=& alpha_temp;else goto error;
      Image   bump_temp; C Image *  bump_src=&  bump.image; if(  bump_src->compressed())if(  bump_src->copyTry(  bump_temp, -1, -1, -1,   bump_src->highPrecision() ? IMAGE_F32           : IMAGE_L8         , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA))  bump_src=&  bump_temp;else goto error; // use high precision because we might use it to create normal map or stretch
      Image normal_temp; C Image *normal_src=&normal.image; if(normal_src->compressed())if(normal_src->copyTry(normal_temp, -1, -1, -1, normal_src->highPrecision() ? IMAGE_F32_2         : IMAGE_R8G8       , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA))normal_src=&normal_temp;else goto error; // use high precision because we still do math operations so higher precision could be useful
      Image smooth_temp; C Image *smooth_src=&smooth.image; if(smooth_src->compressed())if(smooth_src->copyTry(smooth_temp, -1, -1, -1,                                                     IMAGE_L8         , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA))smooth_src=&smooth_temp;else goto error;
      Image  metal_temp; C Image * metal_src=& metal.image; if( metal_src->compressed())if( metal_src->copyTry( metal_temp, -1, -1, -1,                                                     IMAGE_L8         , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA)) metal_src=& metal_temp;else goto error;
      Image   glow_temp; C Image *  glow_src=&  glow.image; if(  glow_src->compressed())if(  glow_src->copyTry(  glow_temp, -1, -1, -1,                                                     IMAGE_L8         , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA))  glow_src=&  glow_temp;else goto error;

      // set alpha
      Bool alpha_from_col=false;
      if(  alpha_src->is()) // there's alpha map specified
      {
         if(alpha_src->typeChannels()>1 && alpha_src->typeInfo().a // if alpha has both RGB and Alpha channels, then check which one to use
         && alpha_src->lockRead())
         {
            Byte min_alpha=255, min_lum=255;
            REPD(y, alpha_src->h())
            REPD(x, alpha_src->w())
            {
               Color c=alpha_src->color(x, y);
               MIN(min_alpha, c.a    );
               MIN(min_lum  , c.lum());
            }
            alpha_src->unlock();
            if(min_alpha>=254 && min_lum<254)if(alpha_src->copyTry(alpha_temp, -1, -1, -1, IMAGE_L8, IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA))alpha_src=&alpha_temp;else goto error; // alpha channel is fully white -> use luminance as alpha
         }
      }else // if there's no alpha map
      if(color.image.typeInfo().a) // but there is alpha channel in color map
      {
         Byte min_alpha=255;
         alpha_src=&alpha_temp.mustCreate(color_src->w(), color_src->h(), 1, IMAGE_A8, IMAGE_SOFT, 1);
         if(color_src->lockRead())
         {
            REPD(y, color_src->h())
            REPD(x, color_src->w())
            {
               Byte a=color_src->color(x, y).a;
                      alpha_temp.pixel(x, y, a);
               MIN(min_alpha, a);
            }
            color_src->unlock();
         }
         if(min_alpha>=254)alpha_temp.del(); // alpha channel in color map is fully white
         else              alpha_from_col=true;
      }
      FILTER_TYPE alpha_filter=Filter((alpha_from_col && alpha.filter<0) ? color.filter : alpha.filter);
      VecI2       alpha_size; if(!alpha_src->is())alpha_size.zero();else alpha_size.set((alpha.size.x>0) ? alpha.size.x : (alpha_from_col && color.size.x>0) ? color.size.x : alpha_src->w(),
                                                                                        (alpha.size.y>0) ? alpha.size.y : (alpha_from_col && color.size.y>0) ? color.size.y : alpha_src->h());

      // set what textures we have (set this before 'normal' is generated from 'bump')
      if( color_src->is())texf|=TEXF_COLOR ;
      if( alpha_src->is())texf|=TEXF_ALPHA ;
      if(  bump_src->is())texf|=TEXF_BUMP  ;
      if(normal_src->is())texf|=TEXF_NORMAL;
      if(smooth_src->is())texf|=TEXF_SMOOTH;
      if( metal_src->is())texf|=TEXF_METAL ;
      if(  glow_src->is())texf|=TEXF_GLOW  ;

      // base_0 RGBA
      if(texf&(TEXF_COLOR|TEXF_ALPHA|TEXF_SMOOTH|TEXF_METAL|TEXF_BUMP|TEXF_GLOW)) // if want any for Base0 or Base2, because shaders support base_2 only if base_0 is also present, so if we want base_2 then also need base_0
      {
         Int w=Max(1, ImgW(color, color_src), alpha_size.x), // Max 1 in case all images are empty, but we still need it because of Base2
             h=Max(1, ImgH(color, color_src), alpha_size.y); if(resize_to_pow2){w=NearestPow2(w); h=NearestPow2(h);}
         if( color_src->is() && (color_src->w()!=w || color_src->h()!=h))if(color_src->copyTry(color_temp, w, h, -1,                           IMAGE_R8G8B8_SRGB  , IMAGE_SOFT, 1, Filter(color.filter), (color.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA|IC_ALPHA_WEIGHT))color_src=&color_temp;else goto error;
         if( alpha_src->is() && (alpha_src->w()!=w || alpha_src->h()!=h))if(alpha_src->copyTry(alpha_temp, w, h, -1, alpha_src->typeInfo().a ? IMAGE_A8 : IMAGE_L8, IMAGE_SOFT, 1,        alpha_filter , (alpha.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA                ))alpha_src=&alpha_temp;else goto error;
         if(!color_src->is() ||  color_src->lockRead())
         {
            if(!alpha_src->is() || alpha_src->lockRead())
            {
               dest_0.createSoftTry(w, h, 1, IMAGE_R8G8B8A8_SRGB);
               Int   alpha_component=(alpha_src->typeInfo().a ? 3 : 0); // use Alpha or Red in case src is R8/L8
               Color c=WHITE;
               REPD(y, dest_0.h())
               REPD(x, dest_0.w())
               {
                  if(color_src->is())c.rgb=color_src->color(x, y).rgb;
                  if(alpha_src->is())c.a  =alpha_src->color(x, y).c[alpha_component];
                  dest_0.color(x, y, c);
               }
               alpha_src->unlock();
            }
            color_src->unlock();
         }
      }

      // base_1 NRM !! do this first before base_2 SRBG which resizes bump !!
    C Image *bump_to_normal=null;
      if(  bump_src->is() && !normal_src->is()                                                )bump_to_normal=  bump_src;else // if bump available and normal not, then create normal from bump
      if(normal_src->is() && (normal.image.typeChannels()==1 || normal_src->monochromaticRG()))bump_to_normal=normal_src;     // if normal is provided as monochromatic, then treat it as bump and convert to normal
      if(bump_to_normal) // create normal from bump
      {
         // it's best to resize bump instead of normal
         Int w=((normal.size.x>0) ? normal.size.x : (bump_to_normal==bump_src && bump.size.x>0) ? bump.size.x : bump_to_normal->w()),
             h=((normal.size.y>0) ? normal.size.y : (bump_to_normal==bump_src && bump.size.y>0) ? bump.size.y : bump_to_normal->h()); if(resize_to_pow2){w=NearestPow2(w); h=NearestPow2(h);}
       C ImageSource &src=((bump_to_normal==bump_src) ? bump : normal);
         if(bump_to_normal->w()!=w || bump_to_normal->h()!=h)if(bump_to_normal->copyTry(normal_temp, w, h, -1, IMAGE_F32, IMAGE_SOFT, 1, Filter(src.filter), (src.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA))bump_to_normal=&normal_temp;else goto error; // !! convert to 'normal_temp' instead of 'bump_temp' because we still need original bump later !!
         bump_to_normal->bumpToNormal(normal_temp, AvgF(w, h)*BUMP_TO_NORMAL_SCALE); normal_src=&normal_temp;
         flip_normal_y=false; // no need to flip since normal map generated from bump is always correct
      }
      if(normal_src->is())
      {
         Int w=ImgW(normal, normal_src),
             h=ImgH(normal, normal_src); if(resize_to_pow2){w=NearestPow2(w); h=NearestPow2(h);}
         if( normal_src->is() && (normal_src->w()!=w || normal_src->h()!=h))if(normal_src->copyTry(normal_temp, w, h, -1, IMAGE_F32_2, IMAGE_SOFT, 1, Filter(normal.filter), (normal.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA))normal_src=&normal_temp;else goto error; // use high precision because we still do math operations so higher precision could be useful
         if(!normal_src->is() ||  normal_src->lockRead())
         {
            dest_1.createSoftTry(w, h, 1, IMAGE_R8G8_SIGN, 1);
            Vec4 c=0;
            REPD(y, dest_1.h())
            REPD(x, dest_1.w())
            {
               if(normal_src->is()){c.xy=normal_src->colorF(x, y).xy*2-1; if(flip_normal_y)CHS(c.y);}
               dest_1.colorF(x, y, c);
            }
            normal_src->unlock();
         }
      }

      // base_2 SRBG
      if(texf&(TEXF_SMOOTH|TEXF_METAL|TEXF_BUMP|TEXF_GLOW))
      {
         Int w=Max(ImgW(smooth, smooth_src), ImgW(metal, metal_src), ImgW(bump, bump_src), ImgW(glow, glow_src)),
             h=Max(ImgH(smooth, smooth_src), ImgH(metal, metal_src), ImgH(bump, bump_src), ImgH(glow, glow_src)); if(resize_to_pow2){w=NearestPow2(w); h=NearestPow2(h);}

         if(smooth_src->is() && (smooth_src->w()!=w || smooth_src->h()!=h))if(smooth_src->copyTry(smooth_temp, w, h, -1, IMAGE_L8, IMAGE_SOFT, 1, Filter(smooth.filter), (smooth.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA))smooth_src=&smooth_temp;else goto error;
         if( metal_src->is() && ( metal_src->w()!=w ||  metal_src->h()!=h))if( metal_src->copyTry( metal_temp, w, h, -1, IMAGE_L8, IMAGE_SOFT, 1, Filter( metal.filter), ( metal.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA)) metal_src=& metal_temp;else goto error;
         if(  bump_src->is() && (  bump_src->w()!=w ||   bump_src->h()!=h))if(  bump_src->copyTry(  bump_temp, w, h, -1, IMAGE_L8, IMAGE_SOFT, 1, Filter(  bump.filter), (  bump.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA))  bump_src=&  bump_temp;else goto error;
         if(  glow_src->is() && (  glow_src->w()!=w ||   glow_src->h()!=h))if(  glow_src->copyTry(  glow_temp, w, h, -1, IMAGE_L8, IMAGE_SOFT, 1, Filter(  glow.filter), (  glow.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA))  glow_src=&  glow_temp;else goto error;

         if(!smooth_src->is() || smooth_src->lockRead())
         {
            if(!metal_src->is() || metal_src->lockRead())
            {
               if(!bump_src->is() || bump_src->lockRead())
               {
                  if(!glow_src->is() || glow_src->lockRead())
                  {
                     dest_2.createSoftTry(w, h, 1, IMAGE_R8G8B8A8);
                     Color c;
                     c.BASE_CHANNEL_ROUGH=TexSmooth(TEX_DEFAULT_SMOOTH);
                     c.BASE_CHANNEL_METAL=          TEX_DEFAULT_METAL  ;
                     c.BASE_CHANNEL_BUMP =          TEX_DEFAULT_BUMP   ;
                     c.BASE_CHANNEL_GLOW =255;
                     REPD(y, dest_2.h())
                     REPD(x, dest_2.w())
                     {
                        if(smooth_src->is()){c.BASE_CHANNEL_ROUGH=smooth_src->color(x, y).lum(); if(smooth_is_rough!=TEX_IS_ROUGH)c.BASE_CHANNEL_ROUGH=255-c.BASE_CHANNEL_ROUGH;}
                        if( metal_src->is()) c.BASE_CHANNEL_METAL= metal_src->color(x, y).lum();
                        if(  bump_src->is()) c.BASE_CHANNEL_BUMP =  bump_src->color(x, y).lum();
                        if(  glow_src->is()) c.BASE_CHANNEL_GLOW =  glow_src->color(x, y).lum();
                        dest_2.color(x, y, c);
                     }
                     glow_src->unlock();
                  }
                  bump_src->unlock();
               }
               metal_src->unlock();
            }
            smooth_src->unlock();
         }
      }
   }

error:
   Swap(dest_0, base_0);
   Swap(dest_1, base_1);
   Swap(dest_2, base_2);
   return texf;
}
TEX_FLAG ExtractBase0Texture(Image &base_0, Image *color, Image *alpha)
{
   TEX_FLAG tex=TEXF_NONE;
   if(color)color->createSoftTry(base_0.w(), base_0.h(), 1, IMAGE_R8G8B8_SRGB);
   if(alpha)alpha->createSoftTry(base_0.w(), base_0.h(), 1, IMAGE_L8);
   REPD(y, base_0.h())
   REPD(x, base_0.w())
   {
      Color c=base_0.color(x, y); // #MaterialTextureLayout
      if(color){color->color(x, y, c  ); if(c.r<254 || c.g<254 || c.b<254)tex|=TEXF_COLOR;}
      if(alpha){alpha->pixel(x, y, c.a); if(c.a<254                      )tex|=TEXF_ALPHA;}
   }
   return tex;
}
TEX_FLAG ExtractBase1Texture(Image &base_1, Image *normal)
{
   TEX_FLAG tex=TEXF_NONE;
   if(normal)
   {
      normal->createSoftTry(base_1.w(), base_1.h(), 1, IMAGE_R8G8B8);
      REPD(y, base_1.h())
      REPD(x, base_1.w())
      {
         Vec4 n; n.xy=base_1.colorF(x, y).xy; // #MaterialTextureLayout
         if(Abs(n.x)>1.5/127
         || Abs(n.y)>1.5/127)tex|=TEXF_NORMAL;
         n.z=CalcZ(n.xy);
         n.xyz=n.xyz*0.5+0.5;
         n.w=1;
         normal->colorF(x, y, n);
      }
   }
   return tex;
}
TEX_FLAG ExtractBase2Texture(Image &base_2, Image *bump, Image *smooth, Image *metal, Image *glow)
{
   TEX_FLAG tex=TEXF_NONE;
   if(smooth)smooth->createSoftTry(base_2.w(), base_2.h(), 1, IMAGE_L8);
   if(metal )metal ->createSoftTry(base_2.w(), base_2.h(), 1, IMAGE_L8);
   if(bump  )bump  ->createSoftTry(base_2.w(), base_2.h(), 1, IMAGE_L8);
   if(glow  )glow  ->createSoftTry(base_2.w(), base_2.h(), 1, IMAGE_L8);
   REPD(y, base_2.h())
   REPD(x, base_2.w())
   {
      Color c=base_2.color(x, y); // #MaterialTextureLayout
      if(smooth){smooth->pixel(x, y, TexSmooth(c.BASE_CHANNEL_ROUGH)); if(TexSmooth(c.BASE_CHANNEL_ROUGH)>1                )tex|=TEXF_SMOOTH;} ASSERT(TEX_DEFAULT_SMOOTH==0);
      if(metal ){metal ->pixel(x, y,           c.BASE_CHANNEL_METAL ); if(          c.BASE_CHANNEL_METAL >1                )tex|=TEXF_METAL ;} ASSERT(TEX_DEFAULT_METAL ==0);
      if(bump  ){bump  ->pixel(x, y,           c.BASE_CHANNEL_BUMP  ); if(      Abs(c.BASE_CHANNEL_BUMP-TEX_DEFAULT_BUMP)>1)tex|=TEXF_BUMP  ;}
      if(glow  ){glow  ->pixel(x, y,           c.BASE_CHANNEL_GLOW  ); if(          c.BASE_CHANNEL_GLOW  <254              )tex|=TEXF_GLOW  ;}
   }
   return tex;
}
/******************************************************************************/
TEX_FLAG CreateDetailTexture(Image &detail, C ImageSource &color, C ImageSource &bump, C ImageSource &normal, C ImageSource &smooth, Bool resize_to_pow2, Bool flip_normal_y, Bool smooth_is_rough)
{
   TEX_FLAG texf=TEXF_NONE;
   Image dest;
   {
      Image  color_temp; C Image * color_src=& color.image; if( color_src->compressed())if( color_src->copyTry( color_temp, -1, -1, -1,                                         IMAGE_L8_SRGB, IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA)) color_src=& color_temp;else goto error;
      Image   bump_temp; C Image *  bump_src=&  bump.image; if(  bump_src->compressed())if(  bump_src->copyTry(  bump_temp, -1, -1, -1, bump_src->highPrecision() ? IMAGE_F32 : IMAGE_L8     , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA))  bump_src=&  bump_temp;else goto error; // use high precision because we might use it to create normal map or stretch
      Image normal_temp; C Image *normal_src=&normal.image; if(normal_src->compressed())if(normal_src->copyTry(normal_temp, -1, -1, -1,                                         IMAGE_R8G8   , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA))normal_src=&normal_temp;else goto error;
      Image smooth_temp; C Image *smooth_src=&smooth.image; if(smooth_src->compressed())if(smooth_src->copyTry(smooth_temp, -1, -1, -1,                                         IMAGE_L8     , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA))smooth_src=&smooth_temp;else goto error;

      // set what textures we have (set this before 'normal' is generated from 'bump')
      if( color_src->is())texf|=TEXF_DET_COLOR ;
    //if(  bump_src->is())texf|=TEXF_DET_BUMP  ;
      if(normal_src->is())texf|=TEXF_DET_NORMAL;
      if(smooth_src->is())texf|=TEXF_DET_SMOOTH;

      Int w=Max(ImgW(color, color_src), ImgW(smooth, smooth_src)),
          h=Max(ImgH(color, color_src), ImgH(smooth, smooth_src)); if(resize_to_pow2){w=NearestPow2(w); h=NearestPow2(h);}

      // normal
    C Image *bump_to_normal=null;
      if(  bump_src->is() && !normal_src->is()                                                )bump_to_normal=  bump_src;else // if bump available and normal not, then create normal from bump
      if(normal_src->is() && (normal.image.typeChannels()==1 || normal_src->monochromaticRG()))bump_to_normal=normal_src;     // if normal is provided as monochromatic, then treat it as bump and convert to normal
      if(bump_to_normal) // create normal from bump
      {
         // it's best to resize bump instead of normal
         MAX(w, (normal.size.x>0) ? normal.size.x : (bump_to_normal==bump_src && bump.size.x>0) ? bump.size.x : bump_to_normal->w());
         MAX(h, (normal.size.y>0) ? normal.size.y : (bump_to_normal==bump_src && bump.size.y>0) ? bump.size.y : bump_to_normal->h()); if(resize_to_pow2){w=NearestPow2(w); h=NearestPow2(h);}
       C ImageSource &src=((bump_to_normal==bump_src) ? bump : normal);
         if(bump_to_normal->w()!=w || bump_to_normal->h()!=h)if(bump_to_normal->copyTry(normal_temp, w, h, -1, IMAGE_F32, IMAGE_SOFT, 1, Filter(src.filter), (src.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA))bump_to_normal=&normal_temp;else goto error; // !! convert to 'normal_temp' instead of 'bump_temp' because we still need original bump later !!
         bump_to_normal->bumpToNormal(normal_temp, AvgF(w, h)*BUMP_TO_NORMAL_SCALE); normal_src=&normal_temp;
         flip_normal_y=false; // no need to flip since normal map generated from bump is always correct
      }else
      if(normal_src->is())
      {
         MAX(w, ImgW(normal, normal_src));
         MAX(h, ImgH(normal, normal_src)); if(resize_to_pow2){w=NearestPow2(w); h=NearestPow2(h);}
      }

      if( color_src->is() && ( color_src->w()!=w ||  color_src->h()!=h))if( color_src->copyTry( color_temp, w, h, -1, IMAGE_L8_SRGB, IMAGE_SOFT, 1, Filter( color.filter), ( color.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA|IC_ALPHA_WEIGHT)) color_src=& color_temp;else goto error;
    //if(  bump_src->is() && (  bump_src->w()!=w ||   bump_src->h()!=h))if(  bump_src->copyTry(  bump_temp, w, h, -1, IMAGE_L8     , IMAGE_SOFT, 1, Filter(  bump.filter), (  bump.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA                ))  bump_src=&  bump_temp;else goto error;
      if(normal_src->is() && (normal_src->w()!=w || normal_src->h()!=h))if(normal_src->copyTry(normal_temp, w, h, -1, IMAGE_R8G8   , IMAGE_SOFT, 1, Filter(normal.filter), (normal.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA                ))normal_src=&normal_temp;else goto error;
      if(smooth_src->is() && (smooth_src->w()!=w || smooth_src->h()!=h))if(smooth_src->copyTry(smooth_temp, w, h, -1, IMAGE_L8     , IMAGE_SOFT, 1, Filter(smooth.filter), (smooth.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA                ))smooth_src=&smooth_temp;else goto error;

      dest.createSoftTry(w, h, 1, IMAGE_R8G8B8A8);

      if(!color_src->is() || color_src->lockRead())
      {
         if(!bump_src->is() || bump_src->lockRead())
         {
            if(!normal_src->is() || normal_src->lockRead())
            {
               if(!smooth_src->is() || smooth_src->lockRead())
               {
                  Color detail; // #MaterialTextureLayoutDetail
                  detail.DETAIL_CHANNEL_NORMAL=128;
                  detail.DETAIL_CHANNEL_ROUGH =128;
                  detail.DETAIL_CHANNEL_COLOR =128;

                  REPD(y, dest.h())
                  REPD(x, dest.w())
                  {
                     if( color_src->is()) detail.DETAIL_CHANNEL_COLOR = color_src->color(x, y).lum();
                   //if(  bump_src->is()) bump                        =  bump_src->color(x, y).lum();
                     if(normal_src->is()){detail.DETAIL_CHANNEL_NORMAL=normal_src->color(x, y).rg   ; if(flip_normal_y                )detail.DETAIL_CHANNEL_NORMAL_Y=255-detail.DETAIL_CHANNEL_NORMAL_Y;}
                     if(smooth_src->is()){detail.DETAIL_CHANNEL_ROUGH =smooth_src->color(x, y).lum(); if(smooth_is_rough!=TEX_IS_ROUGH)detail.DETAIL_CHANNEL_ROUGH   =255-detail.DETAIL_CHANNEL_ROUGH   ;}
                     dest.color(x, y, detail);
                  }
                  smooth_src->unlock();
               }
               normal_src->unlock();
            }
            bump_src->unlock();
         }
         color_src->unlock();
      }
   }

error:
   Swap(dest, detail);
   return texf;
}
TEX_FLAG ExtractDetailTexture(C Image &detail, Image *color, Image *bump, Image *normal, Image *smooth)
{
   TEX_FLAG tex=TEXF_NONE;
   if(color )color ->createSoftTry(detail.w(), detail.h(), 1, IMAGE_L8);
   if(bump  )bump  ->del();
   if(normal)normal->createSoftTry(detail.w(), detail.h(), 1, IMAGE_R8G8B8);
   if(smooth)smooth->createSoftTry(detail.w(), detail.h(), 1, IMAGE_L8);
   REPD(y, detail.h())
   REPD(x, detail.w())
   {
      Color c=detail.color(x, y); // #MaterialTextureLayoutDetail
      if(color ){color ->pixel(x, y,           c.DETAIL_CHANNEL_COLOR ); if(Abs(c.DETAIL_CHANNEL_COLOR-128)>1)tex|=TEXF_DET_COLOR ;}
      if(smooth){smooth->pixel(x, y, TexSmooth(c.DETAIL_CHANNEL_ROUGH)); if(Abs(c.DETAIL_CHANNEL_ROUGH-128)>1)tex|=TEXF_DET_SMOOTH;}
      if(normal)
      {
         Vec n; n.xy.set((c.DETAIL_CHANNEL_NORMAL_X-128)/127.0, (c.DETAIL_CHANNEL_NORMAL_Y-128)/127.0); n.z=CalcZ(n.xy);
         normal->color(x, y, Color(c.DETAIL_CHANNEL_NORMAL_X, c.DETAIL_CHANNEL_NORMAL_Y, Mid(Round(n.z*127+128), 0, 255))); if(Abs(c.DETAIL_CHANNEL_NORMAL_X-128)>1 || Abs(c.DETAIL_CHANNEL_NORMAL_Y-128)>1)tex|=TEXF_DET_NORMAL;
      }
   }
   return tex;
}
/******************************************************************************/
TEX_FLAG CreateWaterBaseTextures(Image &base_0, Image &base_1, Image &base_2, C ImageSource &color, C ImageSource &alpha, C ImageSource &bump, C ImageSource &normal, C ImageSource &smooth, C ImageSource &metal, C ImageSource &glow, Bool resize_to_pow2, Bool flip_normal_y, Bool smooth_is_rough)
{ // #MaterialTextureLayoutWater
   TEX_FLAG texf=TEXF_NONE;
   Image dest_0, dest_1, dest_2;
   {
      Image  color_temp; C Image * color_src=& color.image; if( color_src->compressed())if( color_src->copyTry( color_temp, -1, -1, -1,  color_src->typeInfo().a    ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8_SRGB, IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA)) color_src=& color_temp;else goto error; // keep alpha because we might use it for alpha and IC_ALPHA_WEIGHT
    //Image  alpha_temp; C Image * alpha_src=& alpha.image; if( alpha_src->compressed())if( alpha_src->copyTry( alpha_temp, -1, -1, -1,  alpha_src->typeInfo().a    ? IMAGE_L8A8          : IMAGE_L8         , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA)) alpha_src=& alpha_temp;else goto error;
      Image   bump_temp; C Image *  bump_src=&  bump.image; if(  bump_src->compressed())if(  bump_src->copyTry(  bump_temp, -1, -1, -1,   bump_src->highPrecision() ? IMAGE_F32           : IMAGE_L8         , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA))  bump_src=&  bump_temp;else goto error; // use high precision because we still do math operations so higher precision could be useful
      Image normal_temp; C Image *normal_src=&normal.image; if(normal_src->compressed())if(normal_src->copyTry(normal_temp, -1, -1, -1, normal_src->highPrecision() ? IMAGE_F32_2         : IMAGE_R8G8       , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA))normal_src=&normal_temp;else goto error; // use high precision because we still do math operations so higher precision could be useful
    //Image smooth_temp; C Image *smooth_src=&smooth.image; if(smooth_src->compressed())if(smooth_src->copyTry(smooth_temp, -1, -1, -1,                                                     IMAGE_L8         , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA))smooth_src=&smooth_temp;else goto error;
    //Image  metal_temp; C Image * metal_src=& metal.image; if( metal_src->compressed())if( metal_src->copyTry( metal_temp, -1, -1, -1,                                                     IMAGE_L8         , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA)) metal_src=& metal_temp;else goto error;
    //Image   glow_temp; C Image *  glow_src=&  glow.image; if(  glow_src->compressed())if(  glow_src->copyTry(  glow_temp, -1, -1, -1,                                                     IMAGE_L8         , IMAGE_SOFT, 1, FILTER_BEST, IC_IGNORE_GAMMA))  glow_src=&  glow_temp;else goto error;

      // set what textures we have (set this before 'normal' is generated from 'bump')
      if( color_src->is())texf|=TEXF_COLOR ;
    //if( alpha_src->is())texf|=TEXF_ALPHA ;
      if(  bump_src->is())texf|=TEXF_BUMP  ;
      if(normal_src->is())texf|=TEXF_NORMAL;
    //if(smooth_src->is())texf|=TEXF_SMOOTH;
    //if( metal_src->is())texf|=TEXF_METAL ;
    //if(  glow_src->is())texf|=TEXF_GLOW  ;

      // base_0
      {
         Int w=ImgW(color, color_src),
             h=ImgH(color, color_src); if(resize_to_pow2){w=NearestPow2(w); h=NearestPow2(h);}
         if( color_src->is() && (color_src->w()!=w || color_src->h()!=h))if(color_src->copyTry(color_temp, w, h, -1, IMAGE_R8G8B8_SRGB, IMAGE_SOFT, 1, Filter(color.filter), (color.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA|IC_ALPHA_WEIGHT))color_src=&color_temp;else goto error;
         if(!color_src->is() ||  color_src->lockRead())
         {
            dest_0.createSoftTry(w, h, 1, IMAGE_R8G8B8_SRGB);
            Color c=WHITE;
            REPD(y, dest_0.h())
            REPD(x, dest_0.w())
            {
               if(color_src->is())c.rgb=color_src->color(x, y).rgb;
               dest_0.color(x, y, c);
            }
            color_src->unlock();
         }
      }

      // base_1 NRM !! do this first before base_2 Bump which resizes bump !!
    C Image *bump_to_normal=null;
      if(  bump_src->is() && !normal_src->is()                                                )bump_to_normal=  bump_src;else // if bump available and normal not, then create normal from bump
      if(normal_src->is() && (normal.image.typeChannels()==1 || normal_src->monochromaticRG()))bump_to_normal=normal_src;     // if normal is provided as monochromatic, then treat it as bump and convert to normal
      if(bump_to_normal) // create normal from bump
      {
         // it's best to resize bump instead of normal
         Int w=((normal.size.x>0) ? normal.size.x : (bump_to_normal==bump_src && bump.size.x>0) ? bump.size.x : bump_to_normal->w()),
             h=((normal.size.y>0) ? normal.size.y : (bump_to_normal==bump_src && bump.size.y>0) ? bump.size.y : bump_to_normal->h()); if(resize_to_pow2){w=NearestPow2(w); h=NearestPow2(h);}
       C ImageSource &src=((bump_to_normal==bump_src) ? bump : normal);
         if(bump_to_normal->w()!=w || bump_to_normal->h()!=h)if(bump_to_normal->copyTry(normal_temp, w, h, -1, IMAGE_F32, IMAGE_SOFT, 1, Filter(src.filter), (src.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA))bump_to_normal=&normal_temp;else goto error; // !! convert to 'normal_temp' instead of 'bump_temp' because we still need original bump later !!
         bump_to_normal->bumpToNormal(normal_temp, AvgF(w, h)*BUMP_TO_NORMAL_SCALE); normal_src=&normal_temp;
         flip_normal_y=false; // no need to flip since normal map generated from bump is always correct
      }
      if(normal_src->is())
      {
         Int w=ImgW(normal, normal_src),
             h=ImgH(normal, normal_src); if(resize_to_pow2){w=NearestPow2(w); h=NearestPow2(h);}
         if( normal_src->is() && (normal_src->w()!=w || normal_src->h()!=h))if(normal_src->copyTry(normal_temp, w, h, -1, IMAGE_F32_2, IMAGE_SOFT, 1, Filter(normal.filter), (normal.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA))normal_src=&normal_temp;else goto error;
         if(!normal_src->is() ||  normal_src->lockRead())
         {
            dest_1.createSoftTry(w, h, 1, IMAGE_R8G8_SIGN, 1);
            Vec4 c=0;
            REPD(y, dest_1.h())
            REPD(x, dest_1.w())
            {
               if(normal_src->is()){c.xy=normal_src->colorF(x, y).xy*2-1; if(flip_normal_y)CHS(c.y);}
               dest_1.colorF(x, y, c);
            }
            normal_src->unlock();
         }
      }

      // base_2 BUMP
      if(bump_src->is())
      {
         Int w=ImgW(bump, bump_src),
             h=ImgH(bump, bump_src); if(resize_to_pow2){w=NearestPow2(w); h=NearestPow2(h);}
         if( bump_src->is() && (bump_src->w()!=w || bump_src->h()!=h))if(bump_src->copyTry(bump_temp, w, h, -1, IMAGE_F32, IMAGE_SOFT, 1, Filter(bump.filter), (bump.clamp?IC_CLAMP:IC_WRAP)|IC_IGNORE_GAMMA))bump_src=&bump_temp;else goto error;
         if(!bump_src->is() ||  bump_src->lockRead())
         {
            dest_2.createSoftTry(w, h, 1, IMAGE_R8_SIGN);
            Vec4 c=0;
            REPD(y, dest_2.h())
            REPD(x, dest_2.w())
            {
               c.x=(bump_src->is() ? bump_src->colorF(x, y).xyz.max()*2-1 : 0);
               dest_2.colorF(x, y, c);
            }
            bump_src->unlock();
         }
      }
   }

error:
   Swap(dest_0, base_0);
   Swap(dest_1, base_1);
   Swap(dest_2, base_2);
   return texf;
}
TEX_FLAG ExtractWaterBase0Texture(C Image &base_0, Image *color)
{ // #MaterialTextureLayoutWater
   TEX_FLAG tex=TEXF_NONE;
   if(color)
   {
      color->createSoftTry(base_0.w(), base_0.h(), 1, IMAGE_R8G8B8_SRGB);
      REPD(y, base_0.h())
      REPD(x, base_0.w())
      {
         Color c=base_0.color(x, y);
         if(c.r<254 || c.g<254 || c.b<254)tex|=TEXF_COLOR;
         color->color(x, y, c);
      }
   }
   return tex;
}
TEX_FLAG ExtractWaterBase1Texture(C Image &base_1, Image *normal)
{ // #MaterialTextureLayoutWater
   TEX_FLAG tex=TEXF_NONE;
   if(normal)
   {
      normal->createSoftTry(base_1.w(), base_1.h(), 1, IMAGE_R8G8B8);
      REPD(y, base_1.h())
      REPD(x, base_1.w())
      {
         Vec4 n; n.xy=base_1.colorF(x, y).xy;
         if(Abs(n.x)>1.5/127
         || Abs(n.y)>1.5/127)tex|=TEXF_NORMAL;
         n.z=CalcZ(n.xy);
         n.xyz=n.xyz*0.5+0.5;
         n.w=1;
         normal->colorF(x, y, n);
      }
   }
   return tex;
}
TEX_FLAG ExtractWaterBase2Texture(C Image &base_2, Image *bump)
{ // #MaterialTextureLayoutWater
   TEX_FLAG tex=TEXF_NONE;
   if(bump)
   {
      bump->createSoftTry(base_2.w(), base_2.h(), 1, IMAGE_L8);
      REPD(y, base_2.h())
      REPD(x, base_2.w())
      {
         Flt c=base_2.pixelF(x, y);
         if(Abs(c)>1.5/127)tex|=TEXF_BUMP;
         bump->pixelF(x, y, c*0.5+0.5);
      }
   }
   return tex;
}
/******************************************************************************/
Bool CreateBumpFromColor(Image &bump, C Image &color, Flt min_blur_range, Flt max_blur_range, Bool clamp)
{
   Image color_temp; C Image *color_src=&color; if(color_src->compressed())if(color_src->copyTry(color_temp, -1, -1, -1, ImageTypeUncompressed(color_src->type()), IMAGE_SOFT, 1))color_src=&color_temp;else goto error;
   {
      Image bump_temp; if(bump_temp.createSoftTry(color.w(), color.h(), 1, IMAGE_F32)) // operate on temporary in case "&bump==&color", create as high precision to get good quality for blur/normalize
      {
         if(color_src->lockRead())
         {
            REPD(y, bump_temp.h())
            REPD(x, bump_temp.w())bump_temp.pixF(x, y)=color_src->colorF(x, y).xyz.max();
            color_src->unlock();

            if(min_blur_range<0)min_blur_range=0; // auto
            if(max_blur_range<0)max_blur_range=3; // auto
            Bool  first=true;
            Flt   power=1;
            Image bump_step;
            for(Flt blur=max_blur_range; ; ) // start with max range, because it's the most important, so we want it to be precise, and then we will go with half steps down
            {
               bump_temp.blur(first ? bump : bump_step, blur, clamp); // always set the first blur into 'bump' to set it as base, or in case we finish after one step
               if(!first)
               {
                  REPD(y, bump.h())
                  REPD(x, bump.w())bump.pixF(x, y)+=bump_step.pixF(x, y)*power;
               }
               if(blur<=min_blur_range)break;
               first =false;
               blur *=0.5f; if(blur<1)blur=0; // if we reach below 1 blur, then go straight to 0 to avoid doing 0.5, 0.25, 0.125, ..
               power*=0.5f;
            }
            bump.normalize();
            return true;
         }
      }
   }
error:
   bump.del(); return false;
}
/******************************************************************************/
static inline Flt LightSpecular(C Vec &normal, C Vec &light_dir, C Vec &eye_dir, Flt power=64)
{
#if 1 // blinn
   return Pow(Sat(Dot(normal, !(light_dir+eye_dir))), power);
#else // phong
   Vec reflection=!(normal*(2*Dot(normal, light_dir)) - light_dir);
   return Pow(Sat(Dot(reflection, eye_dir)), power);
#endif
}
Bool MergeBaseTextures(Image &base_0, C Material &material, Int image_type, Int max_image_size, C Vec *light_dir, Flt light_power, Flt spec_mul, FILTER_TYPE filter)
{ // #MaterialTextureLayout

   // dimensions
   VecI2 size=0;
   if(material.base_0)size=Max(size, material.base_0->size());
   if(material.base_1)size=Max(size, material.base_1->size());
   if(material.base_2)size=Max(size, material.base_2->size());
   if(max_image_size>0)
   {
      MIN(size.x, max_image_size);
      MIN(size.y, max_image_size);
   }

   Image color; // operate on temp variable in case 'base_0' argument is set to other images used in this func
   if(size.any()) // this implies we have at least one texture
   {
      if(material.base_0)
      {
         if(!material.base_0->copyTry(color, size.x, size.y, 1, IMAGE_R8G8B8A8_SRGB, IMAGE_SOFT, 1, filter, IC_WRAP))return false; // create new color map, use IMAGE_R8G8B8A8_SRGB to always include Alpha
      }else
      {
         if(!color.createSoftTry(size.x, size.y, 1, IMAGE_R8G8B8A8_SRGB))return false;
         REPD(y, color.h())
         REPD(x, color.w())color.color(x, y, WHITE);
      }

      // TODO: this has to be updated based on PBR
      MAX(light_power, 0);
           spec_mul*=(1-material.rough_add)*light_power/255.0f;
      Flt  glow_mul =   material.glow      *(2*1.75f), // *2 because shaders use this multiplier, *1.75 because shaders iterate over few pixels and take the max out of them (this is just approximation)
           glow_blur=0.07f;
      Bool has_normal=(light_dir && material.base_1 &&    material.normal    *light_power>0.01f),
           has_spec  =(light_dir && material.base_2 && (1-material.rough_add)*light_power>0.01f),
           has_glow  =(             material.base_2 &&    material.glow                  >0.01f);

      Image normal; // 'base_1' resized to 'color' resolution
      if(has_normal)if(!material.base_1->copyTry(normal, color.w(), color.h(), 1, ImageTypeUncompressed(material.base_1->type()), IMAGE_SOFT, 1, filter, IC_WRAP))return false;

      Image b2; // 'base_2' resized to 'color' resolution
      if(has_spec || has_glow)if(!material.base_2->copyTry(b2, color.w(), color.h(), 1, ImageTypeUncompressed(material.base_2->type()), IMAGE_SOFT, 1, filter, IC_WRAP))return false;

      // setup glow (before baking normals)
      Image glow; if(has_glow && glow.createSoftTry(color.w(), color.h(), 1, IMAGE_F32_3)) // use Vec because we're storing glow with multiplier
      {
         REPD(y, glow.h())
         REPD(x, glow.w())
         {
            Vec4 c=color.colorF(x, y); // RGB
            c.xyz*=b2.colorF(x, y).BASE_CHANNEL_GLOW*glow_mul; // Glow
            glow.colorF(x, y, c);
         }
         glow.blur(glow.size3()*glow_blur, false);
      }

      // setup alpha
      Bool has_alpha; // if has any alpha in the texture channel
      if(image_type<=0) // if need to set 'has_alpha'
      {
         has_alpha=false;
         REPD(y, color.h())
         REPD(x, color.w())
         {
            Color c=color.color(x, y);
            if(c.a<254)has_alpha=true;
            color.color(x, y, c);
         }
      }

      // bake normal map
      if(has_normal || has_spec)
      {
         Flt light=Sat(light_dir->z)*light_power, // light intensity at flat normal without ambient
           ambient=1-light; // setup ambient so light intensity at flat normal is 1
         REPD(y, color.h())
         REPD(x, color.w())
         {
            // I'm assuming that the texture plane is XY plane with Z=0, and facing us (towards -Z) just like browsing image in some viewer
            Color col=color.color(x, y);
            Vec n;
            n.xy=normal.colorF(x, y).xy*material.normal; CHS(n.y);
            n.z=-CalcZ(n.xy);
            n.normalize();

            if(has_normal)
            {
               Flt d=Sat(-Dot(n, *light_dir)), l=ambient + light_power*d;
               col=ColorBrightness(col, l);
            }
            if(has_spec)if(Byte s=TexSmooth(b2.color(x, y).BASE_CHANNEL_ROUGH)) // #MaterialTextureLayout
            {
               Flt spec=LightSpecular(-n, *light_dir, Vec(0, 0, 1))*spec_mul;
               Color cs=ColorBrightness(s*spec); cs.a=0;
               col=ColorAdd(col, cs);
            }
            color.color(x, y, col);
         }
      }

      // apply glow map (after baking normal)
      if(glow.is())
         REPD(y, color.h())
         REPD(x, color.w())
      {
         Color c=color.color(x, y),
               g=glow .color(x, y); g.a=0;
         color.color(x, y, ColorAdd(c, g));
      }

      // image type
      if(image_type<=0)
      {
         if(material.base_0)
         {
            image_type=material.base_0->type();
            if(has_alpha)image_type=ImageTypeIncludeAlpha(IMAGE_TYPE(image_type)); // convert image type to one with    alpha channel
            else         image_type=ImageTypeExcludeAlpha(IMAGE_TYPE(image_type)); // convert image type to one without alpha channel
         }else           image_type=(has_alpha ? IMAGE_BC7_SRGB : IMAGE_BC1_SRGB);
      }
      if(image_type==IMAGE_PVRTC1_2 || image_type==IMAGE_PVRTC1_4 || image_type==IMAGE_PVRTC1_2_SRGB || image_type==IMAGE_PVRTC1_4_SRGB)size=NearestPow2(size.avgI()); // PVRTC1 must be square and pow2

      // final copy
    C ImagePtr &base=(material.base_0 ? material.base_0 : material.base_1 ? material.base_1 : material.base_2);
      if(!color.copyTry(color, size.x, size.y, 1, image_type, base->mode(), (base->mipMaps()>1) ? 0 : 1, filter, IC_WRAP))return false;
   }

   Swap(base_0, color);
   return true;
}
/******************************************************************************/
static Bool CanBeRemoved(C Material &material) {return material.canBeRemoved();} // Renderer Instancing doesn't use incRef/decRef for more performance, so we need to do additional checking for materials if they can be removed from cache, by checking if they're not assigned to any instance

void ShutMaterial() {Materials.del();}
void InitMaterial()
{
   MaterialDefault.cull=true;
   MaterialDefault.validate();

   MaterialDefaultNoCull=MaterialDefault;
   MaterialDefaultNoCull.cull=false;

   Materials.canBeRemoved(CanBeRemoved);
}
/******************************************************************************/
}
/******************************************************************************/
