/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
   EditMaterial::EditMaterial() : tech(MTECH_OPAQUE), tex_quality(Edit::Material::MEDIUM), flip_normal_y(false), smooth_is_rough(false), cull(true), detail_all_lod(false), color_s(1, 1, 1, 1), emissive_s(0, 0, 0), emissive_glow(0), smooth(0), reflect_min(MATERIAL_REFLECT), reflect_max(1), glow(0), normal(0), bump(0), uv_scale(1), det_uv_scale(4), det_power(0.3f), base_0_tex(UIDZero), base_1_tex(UIDZero), base_2_tex(UIDZero), detail_tex(UIDZero), macro_tex(UIDZero), emissive_tex(UIDZero) {REPAO(tex_downsize)=0;}
   flt EditMaterial::smoothMul()C
   {
      if(smooth_map.is())
      {
       //if(tweak)return 1-Min(Abs(smooth_tweak), 1); // 'smooth_tweak' is -1..1
         return smooth;
      }
      return 0;
   }
   flt EditMaterial::smoothAdd()C
   {
      if(smooth_map.is())
      {
       //if(tweak)return Sat(smooth_tweak); // 'smooth_tweak' is -1..1
         return 0;
      }
      return smooth;
   }
   flt EditMaterial::roughMul()C
   {
    //return smoothMul();
      if(smooth_map.is())
      {
       //if(tweak)return 1-Min(Abs(smooth_tweak), 1); // 'smooth_tweak' is -1..1
         return smooth;
      }
      return 0;
   }
   flt EditMaterial::roughAdd()C
   {
    //return 1-smoothMul()-smoothAdd();
      if(smooth_map.is())
      {
       //if(tweak)return Sat(-smooth_tweak); // 'smooth_tweak' is -1..1
         return 1-smooth;
      }
      return Sat(1-smooth);
   }
   void EditMaterial::setAbsRough(flt rough) // without texture
   {
      smooth=1-rough;
   }
   void EditMaterial::setRoughMulAdd(flt rough_mul, flt rough_add) // with texture
   {
      // it's best to calculate from average roughness: flt avg_rough=0.5*src.rough_mul+src.rough_add (0..1), rough_tweak=avg_rough*2-1 (-1..1), smooth_tweak=-rough_tweak;
      //smooth_tweak=1-src.rough_mul-src.rough_add*2; // optimized
      smooth=Avg(rough_mul, 1-rough_add);
   }
   bool     EditMaterial::hasBumpMap()C {return   bump_map.is() /*|| bump_from_color && color_map.is()*/;}
   bool     EditMaterial::hasNormalMap()C {return normal_map.is() || hasBumpMap();}
   bool     EditMaterial::hasDetailMap()C {return detail_color.is() || detail_bump.is() || detail_normal.is() || detail_smooth.is();}
   bool     EditMaterial::hasBase1Tex()C {return hasNormalMap();}
   bool     EditMaterial::hasBase2Tex()C {return smooth_map.is() || metal_map.is() || hasBumpMap() || glow_map.is();}
   TEX_FLAG EditMaterial::textures()C {TEX_FLAG tf=TEXF_NONE; if(color_map.is())tf|=TEXF_COLOR; if(alpha_map.is())tf|=TEXF_ALPHA; if(hasBumpMap ())tf|=TEXF_BUMP; if(hasNormalMap ())tf|=TEXF_NORMAL; if(smooth_map.is())tf|=TEXF_SMOOTH; if(metal_map.is())tf|=TEXF_METAL; if(glow_map.is())tf|=TEXF_GLOW; if(emissive_map.is())tf|=TEXF_EMISSIVE; return tf;}
   TEX_FLAG EditMaterial::texturesUsed()C {TEX_FLAG tf=TEXF_NONE; if(color_map.is())tf|=TEXF_COLOR; if(usesTexAlpha())tf|=TEXF_ALPHA; if(usesTexBump())tf|=TEXF_BUMP; if(usesTexNormal())tf|=TEXF_NORMAL; if(usesTexSmooth())tf|=TEXF_SMOOTH; if(usesTexMetal())tf|=TEXF_METAL; if(usesTexGlow())tf|=TEXF_GLOW; if(usesTexEmissive())tf|=TEXF_EMISSIVE; return tf;}
   bool     EditMaterial::usesTexColAlpha()C {return tech!=MTECH_OPAQUE                                   &&   (color_map.is() || alpha_map.is());}
   bool     EditMaterial::usesTexAlpha()C {return tech!=MTECH_OPAQUE                                   &&    alpha_map.is();}
   bool     EditMaterial::usesTexBump()C {return    (bump        >EPS_MATERIAL_BUMP || 1)             && hasBumpMap     ();}
   bool     EditMaterial::usesTexNormal()C {return     normal      >EPS_COL                             && hasNormalMap   ();}
   bool     EditMaterial::usesTexSmooth()C {return Abs(smoothMul())>EPS_COL                             &&   smooth_map.is();}
   bool     EditMaterial::usesTexMetal()C {return     reflect_max >EPS_COL                             &&    metal_map.is();}
   bool     EditMaterial::usesTexGlow()C {return     glow        >EPS_COL                             &&     glow_map.is();}
   bool     EditMaterial::usesTexDetail()C {return     det_power   >EPS_COL                             && hasDetailMap   ();}
   bool     EditMaterial::usesTexEmissive()C {return (emissive_s.max()>EPS_COL || emissive_glow>EPS_COL8) && emissive_map.is();}
   bool     EditMaterial::needTanBin()C
   {
      return usesTexBump  ()
          || usesTexNormal()
          || usesTexDetail();
   }
   bool EditMaterial::equal(C EditMaterial &src)C
   {
      return flip_normal_y_time==src.flip_normal_y_time && smooth_is_rough_time==src.smooth_is_rough_time && tex_quality_time==src.tex_quality_time
      && color_map_time==src.color_map_time && alpha_map_time==src.alpha_map_time && bump_map_time==src.bump_map_time && normal_map_time==src.normal_map_time && smooth_map_time==src.smooth_map_time && metal_map_time==src.metal_map_time && glow_map_time==src.glow_map_time
      && detail_map_time==src.detail_map_time && macro_map_time==src.macro_map_time && emissive_map_time==src.emissive_map_time
      && cull_time==src.cull_time && detail_all_lod_time==src.detail_all_lod_time && tech_time==src.tech_time && tex_downsize_time==src.tex_downsize_time
      && color_time==src.color_time && emissive_time==src.emissive_time && smooth_time==src.smooth_time && reflect_time==src.reflect_time && normal_time==src.normal_time && bump_time==src.bump_time
      && glow_time==src.glow_time && uv_scale_time==src.uv_scale_time && detail_time==src.detail_time;
   }
   bool EditMaterial::newer(C EditMaterial &src)C
   {
      return flip_normal_y_time>src.flip_normal_y_time || smooth_is_rough_time>src.smooth_is_rough_time || tex_quality_time>src.tex_quality_time
      || color_map_time>src.color_map_time || alpha_map_time>src.alpha_map_time || bump_map_time>src.bump_map_time || normal_map_time>src.normal_map_time || smooth_map_time>src.smooth_map_time || metal_map_time>src.metal_map_time || glow_map_time>src.glow_map_time
      || detail_map_time>src.detail_map_time || macro_map_time>src.macro_map_time || emissive_map_time>src.emissive_map_time
      || cull_time>src.cull_time || detail_all_lod_time>src.detail_all_lod_time || tech_time>src.tech_time || tex_downsize_time>src.tex_downsize_time
      || color_time>src.color_time || emissive_time>src.emissive_time || smooth_time>src.smooth_time || reflect_time>src.reflect_time || normal_time>src.normal_time || bump_time>src.bump_time
      || glow_time>src.glow_time || uv_scale_time>src.uv_scale_time || detail_time>src.detail_time;
   }
   void EditMaterial::reset() {T=EditMaterial();}
   void EditMaterial::resetAlpha() {color_s.w=((HasAlphaTest(tech) && !HasAlphaBlend(tech) && !HasAlphaTestDither(tech)) ? 0.5f : 1); color_time.getUTC();}
   void EditMaterial::separateNormalMap(C TimeStamp &time)
   {
      if(!normal_map.is() && hasNormalMap()) // if normal map is not specified, but is created from some other map
      {
                                               normal_map="|bump|"; // set normal map from bump map
         if(!ForcesMono(bump_map))SetTransform(normal_map, "grey"); // force grey scale, in case 'bump_map' may be RGB
                                               normal_map_time=time;
      }
   }
   void EditMaterial::cleanupMaps()
   { // no need to adjust time because this is called after maps have been changed
      if( alpha_map=="|color|") alpha_map.clear();
      if(normal_map=="|bump|" )normal_map.clear();
   }
   void EditMaterial::expandMap(Str &map, C MemPtr<FileParams> &color, C MemPtr<FileParams> &smooth, C MemPtr<FileParams> &bump)
   {
      bool normal=(&map==&normal_map);
      Mems<FileParams> files=FileParams::Decode(map);
      REPA(files)
      {
         FileParams &file=files[i];
       C MemPtr<FileParams> *src;
         if(file.name=="|color|" )src=&color ;else
         if(file.name=="|smooth|")src=&smooth;else
         if(file.name=="|bump|"  )src=&bump  ;else
            continue;
         if(src->elms()<=0)file.name.clear();else // if source is empty
         if(src->elms()==1) // if source has only one file
         {
          C FileParams &first=(*src)[0];
            file.name=first.name; // replace name with original
            FREPA(first.params)file.params.NewAt(i)=first.params[i]; // insert original parameters at the start
                    if(normal){file.params.NewAt(first.params.elms()).set("bumpToNormal"); flip_normal_y=false;} // need to force conversion to normal map
         }else
         if(i==0) // if source has multiple files, then we can add only if we're processing the first file (so all transforms from source will not affect anything already loaded)
         {
            file.name.clear(); // clear file name, but leave params/transforms to operate globally
            FREPA(*src)files.NewAt(i)=(*src)[i]; // add all files from source at the start
            if(normal){files.NewAt(src->elms()).params.New().set("bumpToNormal"); flip_normal_y=false;} // need to force conversion to normal map
            // !! here can't access 'file' anymore because its memory address could be invalid !!
         }
      }
      map=FileParams::Encode(files);
   }
   void EditMaterial::expandMaps()
   {
      Mems<FileParams> color =FileParams::Decode( color_map);
      Mems<FileParams> smooth=FileParams::Decode(smooth_map);
      Mems<FileParams> bump  =FileParams::Decode(  bump_map);
      expandMap( color_map, color, smooth, bump);
      expandMap( alpha_map, color, smooth, bump);
      expandMap(  bump_map, color, smooth, bump);
      expandMap(normal_map, color, smooth, bump);
      expandMap(smooth_map, color, smooth, bump);
      expandMap( metal_map, color, smooth, bump);
      expandMap(  glow_map, color, smooth, bump);
   }
   void EditMaterial::setDefault(C Project &proj)
   {
      Copy(tex_downsize, proj.tex_downsize);
   }
   void EditMaterial::newData(C Project &proj)
   {
      flip_normal_y_time++; smooth_is_rough_time++; tex_quality_time++;
      color_map_time++; alpha_map_time++; bump_map_time++; normal_map_time++; smooth_map_time++; metal_map_time++; glow_map_time++;
      detail_map_time++; macro_map_time++; emissive_map_time++;
      cull_time++; detail_all_lod_time++; tech_time++; tex_downsize_time++;
      color_time++; emissive_time++; smooth_time++; reflect_time++; normal_time++; bump_time++; glow_time++; uv_scale_time++; detail_time++;
      setDefault(proj);
   }
   void EditMaterial::create(C XMaterialEx &src, C TimeStamp &time) // used when importing models from 'XMaterial' and also when creating atlases from 'EditMaterial'
   {
      // !! Warning: !!
      detail_all_lod=false; detail_all_lod_time=time;
      smooth_is_rough=false; smooth_is_rough_time=time;
      tex_quality=Edit::Material::MEDIUM; tex_quality_time=time;
      // !! Warning: !!

      Copy(tex_downsize, src.tex_downsize); tex_downsize_time=time;
      flip_normal_y=src.flip_normal_y; flip_normal_y_time=time;
      cull=src.cull; cull_time=time;
      tech=src.technique; tech_time=time;
      color_s=src.color; color_time=time;
      emissive_s=src.emissive; emissive_glow=src.emissive_glow; emissive_time=time;
      reflect_min=src.reflect(); reflect_max=src.reflectMax(); reflect_time=time;
      glow=src.glow; glow_time=time;
      normal=src.normal; normal_time=time;
      bump=src.bump; bump_time=time;
       uv_scale=src. uv_scale; uv_scale_time=time;
      det_uv_scale=src.det_uv_scale; det_power=src.det_power; detail_time=time;

      if(src.smooth_map.is())setRoughMulAdd(src.rough_mul, src.rough_add);else setAbsRough(src.rough_add); smooth_time=time;

        base_0_tex=src.  base_0_id;
        base_1_tex=src.  base_1_id;
        base_2_tex=src.  base_2_id;
        detail_tex=src.  detail_id;
         macro_tex=src.   macro_id;
      emissive_tex=src.emissive_id;

         color_map   =src.        color_map;    color_map_time=time;
         alpha_map   =src.        alpha_map;    alpha_map_time=time;
          bump_map   =src.         bump_map;     bump_map_time=time;
        normal_map   =src.       normal_map;   normal_map_time=time;
        smooth_map   =src.       smooth_map;   smooth_map_time=time;
         metal_map   =src.        metal_map;    metal_map_time=time;
          glow_map   =src.         glow_map;     glow_map_time=time;
      emissive_map   =src.     emissive_map; emissive_map_time=time;
         macro_map   =                    S;    macro_map_time=time;
        detail_color =src. detail_color_map;   detail_map_time=time;
        detail_bump  =src.  detail_bump_map;
        detail_normal=src.detail_normal_map;
        detail_smooth=src.detail_smooth_map;

      if(src.adjust_params)adjustParams(~0, TEXF_NONE, src.has_textures, src.known_textures);
   }
   void EditMaterial::copyTo(Material &dest, C Project &proj)C
   {
      dest.cull=cull;
      dest.detail_all_lod=detail_all_lod;
      dest.technique=tech;
      dest.colorS(color_s);
      dest.emissiveS(emissive_s);
      dest.emissive_glow=emissive_glow;
      dest.rough_mul=roughMul();
      dest.rough_add=roughAdd();
      dest.reflect(reflect_min, reflect_max);
      dest.glow=glow;
      dest.normal=normal;
      dest.bump=bump;
      dest.    uv_scale=    uv_scale;
      dest.det_uv_scale=det_uv_scale;
      dest.det_power   =det_power;
      dest.base_0      =proj.texPath(  base_0_tex);
      dest.base_1      =proj.texPath(  base_1_tex);
      dest.base_2      =proj.texPath(  base_2_tex);
      dest.  detail_map=proj.texPath(  detail_tex);
      dest.   macro_map=proj.texPath(   macro_tex);
      dest.emissive_map=proj.texPath(emissive_tex);
      dest.validate();
   }
   void EditMaterial::copyTo(Edit::Material &dest)C
   {
      dest.technique=tech;
      dest.tex_downsize_mobile=tex_downsize[TSP_MOBILE];
      dest.tex_downsize_switch=tex_downsize[TSP_SWITCH];
      dest.cull=cull;
      dest.detail_all_lod=detail_all_lod;
      dest.flip_normal_y=flip_normal_y;
      dest.smooth_is_rough=smooth_is_rough;
      dest.tex_quality=tex_quality;
      dest.color_s=color_s;
      dest.emissive_s=emissive_s;
      dest.emissive_glow=emissive_glow;
      dest.smooth=smooth;
      dest.reflect_min=reflect_min;
      dest.reflect_max=reflect_max;
      dest.glow=glow;
      dest.normal=normal;
      dest.bump=bump;
      dest.uv_scale=uv_scale;
      dest.   color_map   =FileParams::Decode(color_map);
      dest.   alpha_map   =FileParams::Decode(alpha_map);
      dest.    bump_map   =FileParams::Decode(bump_map);
      dest.  normal_map   =FileParams::Decode(normal_map);
      dest.  smooth_map   =FileParams::Decode(smooth_map);
      dest.   metal_map   =FileParams::Decode(metal_map);
      dest.    glow_map   =FileParams::Decode(glow_map);
      dest.  detail_color =FileParams::Decode(detail_color);
      dest.  detail_bump  =FileParams::Decode(detail_bump);
      dest.  detail_normal=FileParams::Decode(detail_normal);
      dest.  detail_smooth=FileParams::Decode(detail_smooth);
      dest.   macro_map   =FileParams::Decode(macro_map);
      dest.emissive_map   =FileParams::Decode(emissive_map);
   }
   uint EditMaterial::sync(C Edit::Material &src)
   {
      TimeStamp time; time.getUTC();
      uint changed=0;

      changed|=SyncByValue(               tech_time, time, tech               , src.technique          )* CHANGED_PARAM;
      changed|=SyncByValue(               cull_time, time, cull               , src.cull               )* CHANGED_PARAM;
      changed|=SyncByValue(     detail_all_lod_time, time, detail_all_lod     , src.detail_all_lod     )* CHANGED_PARAM;
      changed|=SyncByValue(      flip_normal_y_time, time, flip_normal_y      , src.flip_normal_y      )*(CHANGED_PARAM|CHANGED_FLIP_NRM_Y);
      changed|=SyncByValue(    smooth_is_rough_time, time, smooth_is_rough    , src.smooth_is_rough    )*(CHANGED_PARAM|CHANGED_SMOOTH_IS_ROUGH);
      changed|=SyncByValue(        tex_quality_time, time, tex_quality        , src.tex_quality        )*(CHANGED_PARAM|CHANGED_TEX_QUALITY);

      changed|=SyncByValue(tex_downsize_time, time, tex_downsize[TSP_MOBILE], src.tex_downsize_mobile)*CHANGED_PARAM;
      changed|=SyncByValue(tex_downsize_time, time, tex_downsize[TSP_SWITCH], src.tex_downsize_switch)*CHANGED_PARAM;

      changed|=CHANGED_PARAM*SyncByValueEqual(   color_time, time,     color_s   , src.    color_s   );
      changed|=CHANGED_PARAM*SyncByValueEqual(emissive_time, time,  emissive_s   , src. emissive_s   );
      changed|=CHANGED_PARAM*SyncByValueEqual(emissive_time, time,  emissive_glow, src. emissive_glow);
      changed|=CHANGED_PARAM*SyncByValueEqual(  smooth_time, time,      smooth   , src.     smooth   );
      changed|=CHANGED_PARAM*SyncByValueEqual( reflect_time, time, reflect_min   , src.reflect_min   );
      changed|=CHANGED_PARAM*SyncByValueEqual( reflect_time, time, reflect_max   , src.reflect_max   );
      changed|=CHANGED_PARAM*SyncByValueEqual(    glow_time, time,        glow   , src.       glow   );
      changed|=CHANGED_PARAM*SyncByValueEqual(  normal_time, time,      normal   , src.     normal   );
      changed|=CHANGED_PARAM*SyncByValueEqual(    bump_time, time,        bump   , src.       bump   );
      changed|=CHANGED_PARAM*SyncByValueEqual(uv_scale_time, time,    uv_scale   , src.   uv_scale   );

      changed|=TEXF_COLOR     *SyncByValue(   color_map_time, time,    color_map   , FileParams::Encode(ConstCast(src.   color_map   )));
      changed|=TEXF_ALPHA     *SyncByValue(   alpha_map_time, time,    alpha_map   , FileParams::Encode(ConstCast(src.   alpha_map   )));
      changed|=TEXF_BUMP      *SyncByValue(    bump_map_time, time,     bump_map   , FileParams::Encode(ConstCast(src.    bump_map   )));
      changed|=TEXF_NORMAL    *SyncByValue(  normal_map_time, time,   normal_map   , FileParams::Encode(ConstCast(src.  normal_map   )));
      changed|=TEXF_SMOOTH    *SyncByValue(  smooth_map_time, time,   smooth_map   , FileParams::Encode(ConstCast(src.  smooth_map   )));
      changed|=TEXF_METAL     *SyncByValue(   metal_map_time, time,    metal_map   , FileParams::Encode(ConstCast(src.   metal_map   )));
      changed|=TEXF_GLOW      *SyncByValue(    glow_map_time, time,     glow_map   , FileParams::Encode(ConstCast(src.    glow_map   )));
      changed|=TEXF_DET_COLOR *SyncByValue(  detail_map_time, time,   detail_color , FileParams::Encode(ConstCast(src.  detail_color )));
      changed|=TEXF_DET_BUMP  *SyncByValue(  detail_map_time, time,   detail_bump  , FileParams::Encode(ConstCast(src.  detail_bump  )));
      changed|=TEXF_DET_NORMAL*SyncByValue(  detail_map_time, time,   detail_normal, FileParams::Encode(ConstCast(src.  detail_normal)));
      changed|=TEXF_DET_SMOOTH*SyncByValue(  detail_map_time, time,   detail_smooth, FileParams::Encode(ConstCast(src.  detail_smooth)));
      changed|=TEXF_MACRO     *SyncByValue(   macro_map_time, time,    macro_map   , FileParams::Encode(ConstCast(src.   macro_map   )));
      changed|=TEXF_EMISSIVE  *SyncByValue(emissive_map_time, time, emissive_map   , FileParams::Encode(ConstCast(src.emissive_map   )));

      return changed;
   }
   uint EditMaterial::sync(C EditMaterial &src)
   {
      uint changed=0;

      changed|=Sync(               tech_time, src.               tech_time, tech               , src.tech               )* CHANGED_PARAM;
      changed|=Sync(               cull_time, src.               cull_time, cull               , src.cull               )* CHANGED_PARAM;
      changed|=Sync(     detail_all_lod_time, src.     detail_all_lod_time, detail_all_lod     , src.detail_all_lod     )* CHANGED_PARAM;
      changed|=Sync(      flip_normal_y_time, src.      flip_normal_y_time, flip_normal_y      , src.flip_normal_y      )*(CHANGED_PARAM|CHANGED_FLIP_NRM_Y);
      changed|=Sync(    smooth_is_rough_time, src.    smooth_is_rough_time, smooth_is_rough    , src.smooth_is_rough    )*(CHANGED_PARAM|CHANGED_SMOOTH_IS_ROUGH);
      changed|=Sync(        tex_quality_time, src.        tex_quality_time, tex_quality        , src.tex_quality        )*(CHANGED_PARAM|CHANGED_TEX_QUALITY);
      changed|=SyncMem(    tex_downsize_time, src.       tex_downsize_time, tex_downsize       , src.tex_downsize       )* CHANGED_PARAM;

      changed|=Sync(   color_time, src.   color_time, color_s   , src.color_s   )*CHANGED_PARAM;
      changed|=Sync(  smooth_time, src.  smooth_time, smooth    , src.smooth    )*CHANGED_PARAM;
      changed|=Sync(    glow_time, src.    glow_time, glow      , src.glow      )*CHANGED_PARAM;
      changed|=Sync(  normal_time, src.  normal_time, normal    , src.normal    )*CHANGED_PARAM;
      changed|=Sync(    bump_time, src.    bump_time, bump      , src.bump      )*CHANGED_PARAM;
      changed|=Sync(uv_scale_time, src.uv_scale_time, uv_scale  , src.uv_scale  )*CHANGED_PARAM;
      if(Sync(emissive_time, src.emissive_time))
      {
         changed|=CHANGED_PARAM;
         emissive_s   =src.emissive_s;
         emissive_glow=src.emissive_glow;
      }
      if(Sync(reflect_time, src.reflect_time))
      {
         changed|=CHANGED_PARAM;
         reflect_min=src.reflect_min;
         reflect_max=src.reflect_max;
      }
      if(Sync(detail_time, src.detail_time))
      {
         changed|=CHANGED_PARAM;
         det_uv_scale=src.det_uv_scale;
         det_power   =src.det_power;
      }

      changed|=Sync( color_map_time, src. color_map_time,  color_map, src. color_map)*TEXF_COLOR;
      changed|=Sync( alpha_map_time, src. alpha_map_time,  alpha_map, src. alpha_map)*TEXF_ALPHA;
      changed|=Sync(  bump_map_time, src.  bump_map_time,   bump_map, src.  bump_map)*TEXF_BUMP;
      changed|=Sync(normal_map_time, src.normal_map_time, normal_map, src.normal_map)*TEXF_NORMAL;
      changed|=Sync(smooth_map_time, src.smooth_map_time, smooth_map, src.smooth_map)*TEXF_SMOOTH;
      changed|=Sync( metal_map_time, src. metal_map_time,  metal_map, src. metal_map)*TEXF_METAL;
      changed|=Sync(  glow_map_time, src.  glow_map_time,   glow_map, src.  glow_map)*TEXF_GLOW;

      if(Sync(detail_map_time, src.detail_map_time))
      {
         changed|=TEXF_DET;
         detail_color =src.detail_color;
         detail_bump  =src.detail_bump;
         detail_normal=src.detail_normal;
         detail_smooth=src.detail_smooth;
         detail_tex   =src.detail_tex;
      }
      if(Sync(macro_map_time, src.macro_map_time))
      {
         changed|=TEXF_MACRO;
         macro_map=src.macro_map;
         macro_tex=src.macro_tex;
      }
      if(Sync(emissive_map_time, src.emissive_map_time))
      {
         changed|=TEXF_EMISSIVE;
         emissive_map=src.emissive_map;
         emissive_tex=src.emissive_tex;
      }

      if(changed&CHANGED_BASE)
      {
         base_0_tex=src.base_0_tex;
         base_1_tex=src.base_1_tex;
         base_2_tex=src.base_2_tex;
      }
      return changed;
   }
   uint EditMaterial::undo(C EditMaterial &src)
   {
      uint changed=0;

      changed|=Undo(               tech_time, src.               tech_time, tech               , src.tech               )* CHANGED_PARAM;
      changed|=Undo(               cull_time, src.               cull_time, cull               , src.cull               )* CHANGED_PARAM;
      changed|=Undo(     detail_all_lod_time, src.     detail_all_lod_time, detail_all_lod     , src.detail_all_lod     )* CHANGED_PARAM;
      changed|=Undo(      flip_normal_y_time, src.      flip_normal_y_time, flip_normal_y      , src.flip_normal_y      )*(CHANGED_PARAM|CHANGED_FLIP_NRM_Y);
      changed|=Undo(    smooth_is_rough_time, src.    smooth_is_rough_time, smooth_is_rough    , src.smooth_is_rough    )*(CHANGED_PARAM|CHANGED_SMOOTH_IS_ROUGH);
      changed|=Undo(        tex_quality_time, src.        tex_quality_time, tex_quality        , src.tex_quality        )*(CHANGED_PARAM|CHANGED_TEX_QUALITY);
      changed|=UndoMem(    tex_downsize_time, src.       tex_downsize_time, tex_downsize       , src.tex_downsize       )* CHANGED_PARAM;

      changed|=Undo(   color_time, src.   color_time, color_s   , src.color_s   )*CHANGED_PARAM;
      changed|=Undo(  smooth_time, src.  smooth_time, smooth    , src.smooth    )*CHANGED_PARAM;
      changed|=Undo(    glow_time, src.    glow_time, glow      , src.glow      )*CHANGED_PARAM;
      changed|=Undo(  normal_time, src.  normal_time, normal    , src.normal    )*CHANGED_PARAM;
      changed|=Undo(    bump_time, src.    bump_time, bump      , src.bump      )*CHANGED_PARAM;
      changed|=Undo(uv_scale_time, src.uv_scale_time, uv_scale  , src.uv_scale  )*CHANGED_PARAM;
      if(Undo(emissive_time, src.emissive_time))
      {
         changed|=CHANGED_PARAM;
         emissive_s   =src.emissive_s;
         emissive_glow=src.emissive_glow;
      }
      if(Undo(reflect_time, src.reflect_time))
      {
         changed|=CHANGED_PARAM;
         reflect_min=src.reflect_min;
         reflect_max=src.reflect_max;
      }
      if(Undo(detail_time, src.detail_time))
      {
         changed|=CHANGED_PARAM;
         det_uv_scale=src.det_uv_scale;
         det_power   =src.det_power;
      }

      changed|=Undo( color_map_time, src. color_map_time,  color_map, src. color_map)*TEXF_COLOR;
      changed|=Undo( alpha_map_time, src. alpha_map_time,  alpha_map, src. alpha_map)*TEXF_ALPHA;
      changed|=Undo(  bump_map_time, src.  bump_map_time,   bump_map, src.  bump_map)*TEXF_BUMP;
      changed|=Undo(normal_map_time, src.normal_map_time, normal_map, src.normal_map)*TEXF_NORMAL;
      changed|=Undo(smooth_map_time, src.smooth_map_time, smooth_map, src.smooth_map)*TEXF_SMOOTH;
      changed|=Undo( metal_map_time, src. metal_map_time,  metal_map, src. metal_map)*TEXF_METAL;
      changed|=Undo(  glow_map_time, src.  glow_map_time,   glow_map, src.  glow_map)*TEXF_GLOW;

      if(Undo(detail_map_time, src.detail_map_time))
      {
         changed|=TEXF_DET;
         detail_color =src.detail_color;
         detail_bump  =src.detail_bump;
         detail_normal=src.detail_normal;
         detail_smooth=src.detail_smooth;
         detail_tex   =src.detail_tex;
      }
      if(Undo(macro_map_time, src.macro_map_time))
      {
         changed|=TEXF_MACRO;
         macro_map=src.macro_map;
         macro_tex=src.macro_tex;
      }
      if(Undo(emissive_map_time, src.emissive_map_time))
      {
         changed|=TEXF_EMISSIVE;
         emissive_map=src.emissive_map;
         emissive_tex=src.emissive_tex;
      }

      if(changed&CHANGED_BASE)
      {
         base_0_tex=src.base_0_tex;
         base_1_tex=src.base_1_tex;
         base_2_tex=src.base_2_tex;
      }
      return changed;
   }
   void EditMaterial::adjustParams(uint changed, TEX_FLAG old_textures, TEX_FLAG has_textures, TEX_FLAG known_textures) // 'old_textures'=textures() before making any change, 'has_textures'=used textures based on per-pixel data (if known), 'known_textures'=what textures in 'has_textures' are known
   {
      TimeStamp time; time.getUTC();
      TEX_FLAG  new_textures=textures(); // textures() after making any change
      TEX_FLAG  changed_presence=(old_textures^new_textures);

      // ALPHA
      if(!(new_textures&(TEXF_COLOR|TEXF_ALPHA))) // there are no color/alpha maps specified
      {
         disable_alpha: if(HasAlphaTest(tech) || HasAlphaBlend(tech) && color_s.w>=1-EPS_COL8){tech=MTECH_OPAQUE; tech_time=time;} // disable alpha technique if alpha-test, or alpha-blend with full alpha
      }else
      if(known_textures&TEXF_ALPHA)
      {
         if(has_textures&TEXF_ALPHA)
         {
            if(changed&(TEXF_COLOR|TEXF_ALPHA)) // enable alpha only if we've changed color/alpha textures (this is to allow having multiple materials with same textures with alpha, but some materials not using alpha)
            {
            enable_alpha:
               if(!HasAlphaBlend(tech) && !HasAlphaTestDither(tech) && color_s.w>=1-EPS_COL8){color_s.w=0.5f; color_time=time;}
               if(!HasAlpha     (tech)                                                      ){tech=MTECH_ALPHA_TEST; tech_time=time;}
            }
         }else goto disable_alpha; // alpha map not available
      }else
      if(!(old_textures&TEXF_ALPHA) && new_textures&TEXF_ALPHA)goto enable_alpha; // if there was no alpha, but now is specifically enabled

      // NORMAL
      if(!(new_textures&(TEXF_BUMP|TEXF_NORMAL))) // there are no bump/normal maps specified
      {
         disable_normal: normal=0; normal_time=time; // disable normal
      }else
      if(known_textures&(TEXF_BUMP|TEXF_NORMAL))
      {
         if(has_textures&(TEXF_BUMP|TEXF_NORMAL))
         {
            if(changed&(TEXF_BUMP|TEXF_NORMAL)) // enable normal only if we've changed bump/normal textures (this is to allow having multiple materials with same textures with normal, but some materials not using normal)
            {
               enable_normal: if(normal<=EPS_COL8){normal=1; normal_time=time;}
            }
         }else goto disable_normal; // normal map not available
      }else
      if(!(old_textures&(TEXF_BUMP|TEXF_NORMAL)) && new_textures&(TEXF_BUMP|TEXF_NORMAL))goto enable_normal; // if there was no bump/normal, but now they are

      // BUMP
      if(changed_presence&TEXF_BUMP)
         if(!(new_textures&TEXF_BUMP)){bump=0   ; bump_time=time;}else
         if(bump<=EPS_MATERIAL_BUMP  ){bump=0.03f; bump_time=time;}

      // SMOOTH
      if(changed_presence&TEXF_SMOOTH)
         if(!(new_textures&TEXF_SMOOTH)){smooth=0; smooth_time=time;} // no  texture -> smooth 0..1, 0=no smooth/fully rough
         else                           {smooth=1; smooth_time=time;} // has texture -> smooth 0..1, 1=use it

      // REFLECT/METAL
    /*Not needed because current setup will work well with or without texture
      if(changed_presence&TEXF_METAL)
         if(!(new_textures&TEXF_METAL)        ){reflect=MATERIAL_REFLECT; reflect_time=time;}else
         if(reflect<=MATERIAL_REFLECT+EPS_COL8){reflect=               1; reflect_time=time;}*/

      // GLOW
      if(changed_presence&TEXF_GLOW)
         if(!(new_textures&TEXF_GLOW)){glow=0; glow_time=time;}else
         if(glow<=EPS_COL8           ){glow=1; glow_time=time;}

      // EMISSIVE
      if(changed_presence&TEXF_EMISSIVE)
         if(!(new_textures&TEXF_EMISSIVE)){emissive_s=0; emissive_glow=0; emissive_time=time;}else
         if(emissive_s.min()<=EPS_COL8   ){emissive_s=1;                  emissive_time=time;}
   }
   void EditMaterial::FixOldFileParams(Str &name)
   {
      name=Replace(name, "<color>"  , "|color|" );
      name=Replace(name, "<smooth>" , "|smooth|");
      name=Replace(name, "<bump>"   , "|bump|"  );
   }
   void EditMaterial::ChangeMulToSet(Mems<FileParams> &fps)
   {
      FREPA(fps)
      {
         FileParams &fp=fps[i];
         if(!fp.name.is() && fp.params.elms()==1 && fp.params[0].name=="mulRGB" && PartialTransform(fp.params[0]))fp.params[0].name="setRGB";
         ChangeMulToSet(fp.nodes);
      }
   }
   void EditMaterial::ChangeMulToSet(Str &name)
   {
      Mems<FileParams> fps=FileParams::Decode(name);
      ChangeMulToSet(fps);
      name=FileParams::Encode(fps);
   }
   void EditMaterial::fixOldFileParams()
   {
      FixOldFileParams(color_map);
      FixOldFileParams(alpha_map);
      FixOldFileParams(bump_map);
      FixOldFileParams(normal_map);
      FixOldFileParams(smooth_map);
      FixOldFileParams(metal_map);
      FixOldFileParams(glow_map);
      FixOldFileParams(detail_color);
      FixOldFileParams(detail_bump);
      FixOldFileParams(detail_normal);
      FixOldFileParams(detail_smooth);
      FixOldFileParams(macro_map);
      FixOldFileParams(emissive_map);
      ChangeMulToSet(smooth_map);
      ChangeMulToSet( metal_map);
      ChangeMulToSet(  glow_map);
   }
   void EditMaterial::fixOldReflect(flt reflect)
   {
      if(metal_map.is())
      {
         reflect_min=MATERIAL_REFLECT;
         reflect_max=reflect;
      }else
      {
         reflect_min=reflect;
         reflect_max=1;
      }
   }
   bool EditMaterial::save(File &f)C
   {
      f.cmpUIntV(19);
      f<<flip_normal_y<<smooth_is_rough<<cull<<detail_all_lod<<tex_quality<<tech<<tex_downsize;
      f<<color_s<<emissive_s<<emissive_glow<<smooth<<reflect_min<<reflect_max<<glow<<normal<<bump<<uv_scale<<det_uv_scale<<det_power;
      f<<base_0_tex<<base_1_tex<<base_2_tex<<detail_tex<<macro_tex<<emissive_tex;

      f<<color_map<<alpha_map<<bump_map<<normal_map<<smooth_map<<metal_map<<glow_map
       <<detail_color<<detail_bump<<detail_normal<<detail_smooth
       <<macro_map
       <<emissive_map;

      f<<flip_normal_y_time<<smooth_is_rough_time<<tex_quality_time;
      f<<color_map_time<<alpha_map_time<<bump_map_time<<normal_map_time<<smooth_map_time<<metal_map_time<<glow_map_time;
      f<<detail_map_time<<macro_map_time<<emissive_map_time;
      f<<cull_time<<detail_all_lod_time<<tech_time<<tex_downsize_time;
      f<<color_time<<emissive_time<<smooth_time<<reflect_time<<normal_time<<bump_time<<glow_time<<uv_scale_time<<detail_time;
      return f.ok();
   }
   bool EditMaterial::load(File &f)
   {
      flt reflect, sss; bool bump_from_color=false; byte mip_map_blur, tex_downsize_mobile; UID old_reflection_tex; Str old_reflection_map; TimeStamp sss_time, mip_map_blur_time, bump_from_color_time, old_reflection_map_time;
      reset(); switch(f.decUIntV())
      {
         case 19:
         {
            f>>flip_normal_y>>smooth_is_rough>>cull>>detail_all_lod>>tex_quality>>tech>>tex_downsize; ASSERT(ELMS(tex_downsize)==2);
            f>>color_s>>emissive_s>>emissive_glow>>smooth>>reflect_min>>reflect_max>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power;
            f>>base_0_tex>>base_1_tex>>base_2_tex>>detail_tex>>macro_tex>>emissive_tex;

            f>>color_map>>alpha_map>>bump_map>>normal_map>>smooth_map>>metal_map>>glow_map
             >>detail_color>>detail_bump>>detail_normal>>detail_smooth
             >>macro_map
             >>emissive_map;

            f>>flip_normal_y_time>>smooth_is_rough_time>>tex_quality_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>metal_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>emissive_map_time;
            f>>cull_time>>detail_all_lod_time>>tech_time>>tex_downsize_time;
            f>>color_time>>emissive_time>>smooth_time>>reflect_time>>normal_time>>bump_time>>glow_time>>uv_scale_time>>detail_time;
         }break;

         case 18:
         {
            f>>flip_normal_y>>smooth_is_rough>>cull>>detail_all_lod>>tex_quality>>tech>>tex_downsize_mobile; REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>emissive_glow>>smooth>>reflect_min>>reflect_max>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power;
            f>>base_0_tex>>base_1_tex>>base_2_tex>>detail_tex>>macro_tex>>emissive_tex;

            f>>color_map>>alpha_map>>bump_map>>normal_map>>smooth_map>>metal_map>>glow_map
             >>detail_color>>detail_bump>>detail_normal>>detail_smooth
             >>macro_map
             >>emissive_map;

            f>>flip_normal_y_time>>smooth_is_rough_time>>tex_quality_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>metal_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>emissive_map_time;
            f>>cull_time>>detail_all_lod_time>>tech_time>>tex_downsize_time;
            f>>color_time>>emissive_time>>smooth_time>>reflect_time>>normal_time>>bump_time>>glow_time>>uv_scale_time>>detail_time;
         }break;

         case 17:
         {
            f>>flip_normal_y>>smooth_is_rough>>cull>>tex_quality>>tech>>tex_downsize_mobile; REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>emissive_glow>>smooth>>reflect_min>>reflect_max>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power;
            f>>base_0_tex>>base_1_tex>>base_2_tex>>detail_tex>>macro_tex>>emissive_tex;

            f>>color_map>>alpha_map>>bump_map>>normal_map>>smooth_map>>metal_map>>glow_map
             >>detail_color>>detail_bump>>detail_normal>>detail_smooth
             >>macro_map
             >>emissive_map;

            f>>flip_normal_y_time>>smooth_is_rough_time>>tex_quality_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>metal_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time;
            f>>color_time>>emissive_time>>smooth_time>>reflect_time>>normal_time>>bump_time>>glow_time>>uv_scale_time>>detail_time;
         }break;

         case 16:
         {
            f>>flip_normal_y>>smooth_is_rough>>cull>>tex_quality>>tech>>tex_downsize_mobile; REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>smooth>>reflect_min>>reflect_max>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power;
            f>>base_0_tex>>base_1_tex>>base_2_tex>>detail_tex>>macro_tex>>emissive_tex;

            f>>color_map>>alpha_map>>bump_map>>normal_map>>smooth_map>>metal_map>>glow_map
             >>detail_color>>detail_bump>>detail_normal>>detail_smooth
             >>macro_map
             >>emissive_map;

            f>>flip_normal_y_time>>smooth_is_rough_time>>tex_quality_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>metal_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time;
            f>>color_time>>emissive_time>>smooth_time>>reflect_time>>normal_time>>bump_time>>glow_time>>uv_scale_time>>detail_time;
         }break;

         case 15:
         {
            f>>flip_normal_y>>smooth_is_rough>>cull>>tex_quality>>tech>>tex_downsize_mobile; REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>smooth>>reflect_min>>reflect_max>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power;
            f>>base_0_tex>>base_1_tex>>base_2_tex>>detail_tex>>macro_tex>>emissive_tex;

            f>>color_map>>alpha_map>>bump_map>>normal_map>>smooth_map>>metal_map>>glow_map
             >>detail_color>>detail_bump>>detail_normal>>detail_smooth
             >>macro_map
             >>emissive_map;

            f>>flip_normal_y_time>>smooth_is_rough_time>>tex_quality_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>metal_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time;
            f>>color_time>>emissive_time>>smooth_time>>reflect_time>>normal_time>>bump_time>>glow_time>>uv_scale_time>>detail_time;
            if(smooth_map.is())smooth+=1; // here 'smooth' was stored as 'smooth_tweak' -1..1 when having 'smooth_map', and 'smooth_absolute' 0..1 without 'smooth_map'
         }break;

         case 14:
         {
            f>>flip_normal_y>>cull>>tex_quality>>tech>>tex_downsize_mobile; REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>smooth>>reflect_min>>reflect_max>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power;
            f>>base_0_tex>>base_1_tex>>base_2_tex>>detail_tex>>macro_tex>>emissive_tex;

            f>>color_map>>alpha_map>>bump_map>>normal_map>>smooth_map>>metal_map>>glow_map
             >>detail_color>>detail_bump>>detail_normal>>detail_smooth
             >>macro_map
             >>emissive_map;

            f>>flip_normal_y_time>>tex_quality_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>metal_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time;
            f>>color_time>>emissive_time>>smooth_time>>reflect_time>>normal_time>>bump_time>>glow_time>>uv_scale_time>>detail_time;
         }break;

         case 13:
         {
            f>>flip_normal_y>>cull>>tex_quality>>tech>>tex_downsize_mobile; REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>smooth>>reflect>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power;
            f>>base_0_tex>>base_1_tex>>base_2_tex>>detail_tex>>macro_tex>>emissive_tex;

            f>>color_map>>alpha_map>>bump_map>>normal_map>>smooth_map>>metal_map>>glow_map
             >>detail_color>>detail_bump>>detail_normal>>detail_smooth
             >>macro_map
             >>emissive_map;

            f>>flip_normal_y_time>>tex_quality_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>metal_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time;
            f>>color_time>>emissive_time>>smooth_time>>reflect_time>>normal_time>>bump_time>>glow_time>>uv_scale_time>>detail_time;
            fixOldFileParams(); fixOldReflect(reflect);
         }break;

         case 12:
         {
            f>>flip_normal_y>>cull>>tex_quality>>tech>>tex_downsize_mobile; REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>smooth>>reflect>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power;
            f>>base_0_tex>>base_1_tex>>base_2_tex>>detail_tex>>macro_tex>>emissive_tex;

            f>>color_map>>alpha_map>>bump_map>>normal_map>>smooth_map>>metal_map>>glow_map
             >>detail_color>>detail_bump>>detail_normal>>detail_smooth
             >>macro_map
             >>emissive_map;

            f>>flip_normal_y_time>>tex_quality_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>metal_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time;
            f>>color_time>>emissive_time>>smooth_time>>reflect_time>>normal_time>>bump_time>>glow_time>>uv_scale_time>>detail_time;
            fixOldFileParams(); fixOldReflect(reflect);
         }break;

         case 11:
         {
            f>>flip_normal_y>>cull>>tex_quality>>tech>>tex_downsize_mobile; REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>smooth>>reflect>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power;
            f>>base_0_tex>>base_1_tex>>base_2_tex>>detail_tex>>macro_tex>>emissive_tex;

            f>>color_map>>alpha_map>>bump_map>>normal_map>>smooth_map>>metal_map>>glow_map
             >>detail_color>>detail_bump>>detail_normal
             >>macro_map
             >>emissive_map;

            f>>flip_normal_y_time>>tex_quality_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>metal_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time;
            f>>color_time>>emissive_time>>smooth_time>>reflect_time>>normal_time>>bump_time>>glow_time>>uv_scale_time>>detail_time;
            fixOldFileParams(); fixOldReflect(reflect);
         }break;

         case 10:
         {
            f>>flip_normal_y>>cull>>tex_quality>>tech>>tex_downsize_mobile; REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>old_reflection_tex>>emissive_tex;

            f>>color_map>>alpha_map>>bump_map>>normal_map>>smooth_map>>glow_map
             >>detail_color>>detail_bump>>detail_normal
             >>macro_map
             >>old_reflection_map
             >>emissive_map;

            f>>flip_normal_y_time>>tex_quality_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>old_reflection_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time;
            f>>color_time>>emissive_time>>smooth_time>>sss_time>>normal_time>>glow_time>>uv_scale_time>>detail_time>>reflect_time; bump_time=normal_time; if(!old_reflection_map.is())reflect=MATERIAL_REFLECT;else metal_map=smooth_map;
            fixOldFileParams(); fixOldReflect(reflect);
         }break;

         case 9:
         {
            f>>flip_normal_y>>cull>>tex_quality>>tech>>tex_downsize_mobile; REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>old_reflection_tex>>emissive_tex;

            GetStr2(f, color_map); GetStr2(f, alpha_map); GetStr2(f, bump_map); GetStr2(f, normal_map); GetStr2(f, smooth_map); GetStr2(f, glow_map);
            GetStr2(f, detail_color); GetStr2(f, detail_bump); GetStr2(f, detail_normal);
            GetStr2(f, macro_map);
            GetStr2(f, old_reflection_map);
            GetStr2(f, emissive_map);

            f>>flip_normal_y_time>>tex_quality_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>old_reflection_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time;
            f>>color_time>>emissive_time>>smooth_time>>sss_time>>normal_time>>glow_time>>uv_scale_time>>detail_time>>reflect_time; bump_time=normal_time; if(!old_reflection_map.is())reflect=MATERIAL_REFLECT;else metal_map=smooth_map;
            fixOldFileParams(); fixOldReflect(reflect);
         }break;

         case 8:
         {
            f>>bump_from_color>>flip_normal_y>>cull>>tex_quality>>tech>>tex_downsize_mobile; REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>old_reflection_tex>>emissive_tex;

            GetStr2(f, color_map); GetStr2(f, alpha_map); GetStr2(f, bump_map); GetStr2(f, normal_map); GetStr2(f, smooth_map); GetStr2(f, glow_map);
            GetStr2(f, detail_color); GetStr2(f, detail_bump); GetStr2(f, detail_normal);
            GetStr2(f, macro_map);
            GetStr2(f, old_reflection_map);
            GetStr2(f, emissive_map);

            f>>bump_from_color_time>>flip_normal_y_time>>tex_quality_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>old_reflection_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time;
            f>>color_time>>emissive_time>>smooth_time>>sss_time>>normal_time>>glow_time>>uv_scale_time>>detail_time>>reflect_time; bump_time=normal_time; if(!old_reflection_map.is())reflect=MATERIAL_REFLECT;else metal_map=smooth_map;
            fixOldFileParams(); fixOldReflect(reflect);
         }break;

         case 7:
         {
            f>>bump_from_color>>flip_normal_y>>cull>>tex_quality>>tech>>tex_downsize_mobile>>mip_map_blur; REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>old_reflection_tex>>emissive_tex;

            GetStr2(f, color_map); GetStr2(f, alpha_map); GetStr2(f, bump_map); GetStr2(f, normal_map); GetStr2(f, smooth_map); GetStr2(f, glow_map);
            GetStr2(f, detail_color); GetStr2(f, detail_bump); GetStr2(f, detail_normal);
            GetStr2(f, macro_map);
            GetStr2(f, old_reflection_map);
            GetStr2(f, emissive_map);

            f>>bump_from_color_time>>flip_normal_y_time>>tex_quality_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>old_reflection_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time>>mip_map_blur_time;
            f>>color_time>>emissive_time>>smooth_time>>sss_time>>normal_time>>glow_time>>uv_scale_time>>detail_time>>reflect_time; bump_time=normal_time; if(!old_reflection_map.is())reflect=MATERIAL_REFLECT;else metal_map=smooth_map;
            fixOldFileParams(); fixOldReflect(reflect);
         }break;

         case 6:
         {
            f>>bump_from_color>>flip_normal_y>>cull>>tech>>tex_downsize_mobile>>mip_map_blur; REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>old_reflection_tex>>emissive_tex;

            GetStr2(f, color_map); GetStr2(f, alpha_map); GetStr2(f, bump_map); GetStr2(f, normal_map); GetStr2(f, smooth_map); GetStr2(f, glow_map);
            GetStr2(f, detail_color); GetStr2(f, detail_bump); GetStr2(f, detail_normal);
            GetStr2(f, macro_map);
            GetStr2(f, old_reflection_map);
            GetStr2(f, emissive_map);

            f>>bump_from_color_time>>flip_normal_y_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>old_reflection_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time>>mip_map_blur_time;
            f>>color_time>>emissive_time>>smooth_time>>sss_time>>normal_time>>glow_time>>uv_scale_time>>detail_time>>reflect_time; bump_time=normal_time; if(!old_reflection_map.is())reflect=MATERIAL_REFLECT;else metal_map=smooth_map;
            fixOldFileParams(); fixOldReflect(reflect);
         }break;

         case 5:
         {
            byte max_tex_size; f>>bump_from_color>>flip_normal_y>>cull>>tech>>max_tex_size>>mip_map_blur; tex_downsize_mobile=(max_tex_size>=1 && max_tex_size<=10); REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>old_reflection_tex>>emissive_tex;

            GetStr2(f, color_map); GetStr2(f, alpha_map); GetStr2(f, bump_map); GetStr2(f, normal_map); GetStr2(f, smooth_map); GetStr2(f, glow_map);
            GetStr2(f, detail_color); GetStr2(f, detail_bump); GetStr2(f, detail_normal);
            GetStr2(f, macro_map);
            GetStr2(f, old_reflection_map);
            GetStr2(f, emissive_map);

            f>>bump_from_color_time>>flip_normal_y_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>old_reflection_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time>>mip_map_blur_time;
            f>>color_time>>emissive_time>>smooth_time>>sss_time>>normal_time>>glow_time>>uv_scale_time>>detail_time>>reflect_time; bump_time=normal_time; if(!old_reflection_map.is())reflect=MATERIAL_REFLECT;else metal_map=smooth_map;
            fixOldFileParams(); fixOldReflect(reflect);
         }break;

         case 4:
         {
            byte max_tex_size; f>>bump_from_color>>flip_normal_y>>cull>>tech>>max_tex_size>>mip_map_blur; tex_downsize_mobile=(max_tex_size>=1 && max_tex_size<=10); REPAO(tex_downsize)=tex_downsize_mobile;
            f>>color_s>>emissive_s>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>old_reflection_tex>>emissive_tex;
            GetStr(f, color_map); GetStr(f, alpha_map); GetStr(f, bump_map); GetStr(f, normal_map); GetStr(f, smooth_map); GetStr(f, glow_map);
            GetStr(f, detail_color); GetStr(f, detail_bump); GetStr(f, detail_normal);
            GetStr(f, macro_map);
            GetStr(f, old_reflection_map);
            GetStr(f, emissive_map);

            f>>bump_from_color_time>>flip_normal_y_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>old_reflection_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>tex_downsize_time>>mip_map_blur_time;
            f>>color_time>>emissive_time>>smooth_time>>sss_time>>normal_time>>glow_time>>uv_scale_time>>detail_time>>reflect_time; bump_time=normal_time; if(!old_reflection_map.is())reflect=MATERIAL_REFLECT;else metal_map=smooth_map;
            fixOldFileParams(); fixOldReflect(reflect);
         }break;

         case 3:
         {
            f>>bump_from_color>>flip_normal_y>>cull>>tech>>mip_map_blur;
            f>>color_s>>emissive_s>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>old_reflection_tex>>emissive_tex;
            GetStr(f, color_map); GetStr(f, alpha_map); GetStr(f, bump_map); GetStr(f, normal_map); GetStr(f, smooth_map); GetStr(f, glow_map);
            GetStr(f, detail_color); GetStr(f, detail_bump); GetStr(f, detail_normal);
            GetStr(f, macro_map);
            GetStr(f, old_reflection_map);
            GetStr(f, emissive_map);

            f>>bump_from_color_time>>flip_normal_y_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>old_reflection_map_time>>emissive_map_time;
            f>>cull_time>>tech_time>>mip_map_blur_time;
            f>>color_time>>emissive_time>>smooth_time>>sss_time>>normal_time>>glow_time>>uv_scale_time>>detail_time>>reflect_time; bump_time=normal_time; if(!old_reflection_map.is())reflect=MATERIAL_REFLECT;else metal_map=smooth_map;
            fixOldFileParams(); fixOldReflect(reflect);
         }break;

         case 2:
         {
            f>>bump_from_color>>flip_normal_y>>cull>>tech;
            f>>color_s>>emissive_s>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>old_reflection_tex>>emissive_tex;
            GetStr(f, color_map); GetStr(f, alpha_map); GetStr(f, bump_map); GetStr(f, normal_map); GetStr(f, smooth_map); GetStr(f, glow_map);
            GetStr(f, detail_color); GetStr(f, detail_bump); GetStr(f, detail_normal);
            GetStr(f, macro_map);
            GetStr(f, old_reflection_map);
            GetStr(f, emissive_map);

            f>>bump_from_color_time>>flip_normal_y_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>old_reflection_map_time>>emissive_map_time;
            f>>cull_time>>tech_time;
            f>>color_time>>emissive_time>>smooth_time>>sss_time>>normal_time>>glow_time>>uv_scale_time>>detail_time>>reflect_time; bump_time=normal_time; if(!old_reflection_map.is())reflect=MATERIAL_REFLECT;else metal_map=smooth_map;
            fixOldFileParams(); fixOldReflect(reflect);
         }break;

         case 1:
         {
            f>>bump_from_color>>flip_normal_y>>cull>>tech;
            f>>color_s>>emissive_s>>smooth>>sss>>glow>>normal>>bump>>uv_scale>>det_uv_scale>>det_power>>reflect;
            f>>base_0_tex>>base_1_tex>>detail_tex>>macro_tex>>old_reflection_tex;
            GetStr(f, color_map); GetStr(f, alpha_map); GetStr(f, bump_map); GetStr(f, normal_map); GetStr(f, smooth_map); GetStr(f, glow_map);
            GetStr(f, detail_color); GetStr(f, detail_bump); GetStr(f, detail_normal);
            GetStr(f, macro_map);
            GetStr(f, old_reflection_map);

            f>>bump_from_color_time>>flip_normal_y_time;
            f>>color_map_time>>alpha_map_time>>bump_map_time>>normal_map_time>>smooth_map_time>>glow_map_time;
            f>>detail_map_time>>macro_map_time>>old_reflection_map_time;
            f>>cull_time>>tech_time;
            f>>color_time>>emissive_time>>smooth_time>>sss_time>>normal_time>>glow_time>>uv_scale_time>>detail_time>>reflect_time; bump_time=normal_time; if(!old_reflection_map.is())reflect=MATERIAL_REFLECT;else metal_map=smooth_map;
            fixOldFileParams(); fixOldReflect(reflect);
         }break;

         case 0: break; // empty, this requires 'reset' to be called before

         default: goto error;
      }
      if(f.ok())
      {
         if(bump_from_color && !bump_map.is() && color_map.is()){bump_map=BumpFromColTransform(color_map, 3); if(!bump_map_time.is())bump_map_time++;}
         return true;
      }
   error:
      reset(); return false;
   }
   bool EditMaterial::load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f);
      reset(); return false;
   }
/******************************************************************************/
