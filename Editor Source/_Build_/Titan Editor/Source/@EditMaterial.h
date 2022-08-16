/******************************************************************************/
class EditMaterial
{
   MATERIAL_TECHNIQUE        tech;
   Edit::Material::TEX_QUALITY tex_quality;
   bool                      flip_normal_y, smooth_is_rough, cull, detail_all_lod;
   byte                      tex_downsize[TSP_NUM];
   Vec4                      color_s;
   Vec                       emissive_s;
   flt                       emissive_glow,
                             smooth, // 0..1
                             reflect_min, reflect_max, // 0..1
                             glow, // 0..1
                             normal, // 0..1
                             bump, // 0..1
                             uv_scale, det_uv_scale, det_power;
   UID                       base_0_tex, base_1_tex, base_2_tex, detail_tex, macro_tex, emissive_tex;
   Str                       color_map, alpha_map, bump_map, normal_map, smooth_map, metal_map, glow_map,
                             detail_color, detail_bump, detail_normal, detail_smooth,
                             macro_map,
                             emissive_map;
   TimeStamp                 flip_normal_y_time, smooth_is_rough_time, tex_quality_time,
                             color_map_time, alpha_map_time, bump_map_time, normal_map_time, smooth_map_time, metal_map_time, glow_map_time,
                             detail_map_time, macro_map_time, emissive_map_time,
                             cull_time, detail_all_lod_time, tech_time, tex_downsize_time,
                             color_time, emissive_time, smooth_time, reflect_time, normal_time, bump_time, glow_time, uv_scale_time, detail_time;

   EditMaterial();

   // get
   // smooth_final = smooth_tex*smoothMul+smoothAdd
   //  rough_final =  rough_tex* roughMul+ roughAdd
   // rough_final = 1-smooth_final
   // rough_tex*roughMul+roughAdd = 1-((1-rough_tex)*smoothMul+smoothAdd)
   // (1-rough_tex)*-smoothMul-smoothAdd+1
   // rough_tex*smoothMul-smoothMul-smoothAdd+1
   // roughMul=smoothMul
   // roughAdd=1-smoothMul-smoothAdd
   flt smoothMul()C;
   flt smoothAdd()C;
   flt roughMul()C;
   flt roughAdd()C;
   void setAbsRough(flt rough); // without texture
   void setRoughMulAdd(flt rough_mul, flt rough_add); // with texture
   bool     hasBumpMap     ()C;                    
   bool     hasNormalMap   ()C;                    
   bool     hasDetailMap   ()C;                    
   bool     hasBase1Tex    ()C;                    // #MaterialTextureLayout
   bool     hasBase2Tex    ()C;                    // #MaterialTextureLayout
   TEX_FLAG textures       ()C;                    
   TEX_FLAG texturesUsed   ()C;                    
   bool     usesTexColAlpha()C;                    // alpha may come from color
   bool     usesTexAlpha   ()C;                    // check only alpha
   bool     usesTexBump    ()C;                    // always keep bump map because it can be used for multi-material per-pixel blending
   bool     usesTexNormal  ()C;                    
   bool     usesTexSmooth  ()C;                    
   bool     usesTexMetal   ()C;                    
   bool     usesTexGlow    ()C;                    
   bool     usesTexDetail  ()C;                    
   bool     usesTexEmissive()C;                    
   bool     needTanBin     ()C;

   bool equal(C EditMaterial &src)C;
   bool newer(C EditMaterial &src)C;

   // operations
   void reset();                   
   void resetAlpha();                                            
   void separateNormalMap(C TimeStamp&time=TimeStamp().getUTC());
   void cleanupMaps();
   void expandMap(Str &map, C MemPtr<FileParams> &color, C MemPtr<FileParams> &smooth, C MemPtr<FileParams> &bump);
   void expandMaps();

   void setDefault(C Project &proj);
   void newData(C Project &proj);
   void create(C XMaterialEx &src, C TimeStamp &time=TimeStamp().getUTC()); // used when importing models from 'XMaterial' and also when creating atlases from 'EditMaterial'
   void copyTo(Material &dest, C Project &proj)C;
   void copyTo(Edit::Material &dest)C;
   enum
   {
    //TEXF_COLOR.. this includes all TEXF_ enums
      CHANGED_PARAM          =1<<(TEX_NUM  ),
      CHANGED_TEX_QUALITY    =1<<(TEX_NUM+1),
      CHANGED_FLIP_NRM_Y     =1<<(TEX_NUM+2),
      CHANGED_SMOOTH_IS_ROUGH=1<<(TEX_NUM+3),

      CHANGED_BASE=TEXF_BASE|CHANGED_TEX_QUALITY|CHANGED_FLIP_NRM_Y|CHANGED_SMOOTH_IS_ROUGH, // any of these parameters should trigger rebuild base texture
   };
   uint sync(C Edit::Material &src);
   uint sync(C EditMaterial &src);
   uint undo(C EditMaterial &src);
   void adjustParams(uint changed, TEX_FLAG old_textures, TEX_FLAG has_textures, TEX_FLAG known_textures); // 'old_textures'=textures() before making any change, 'has_textures'=used textures based on per-pixel data (if known), 'known_textures'=what textures in 'has_textures' are known

   static void FixOldFileParams(Str &name);
   static void ChangeMulToSet(Mems<FileParams> &fps);
   static void ChangeMulToSet(Str &name);
   void fixOldFileParams();
   void fixOldReflect(flt reflect);

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
