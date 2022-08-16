/******************************************************************************/
/******************************************************************************/
class EditWaterMtrl : EditMaterial
{
   flt density, density_add,
       scale_color, scale_normal, scale_bump,
       refract, refract_reflection,
       wave_scale;
   Vec color_underwater0,
       color_underwater1;

   TimeStamp density_time,
             scale_color_time, scale_normal_time, scale_bump_time,
             refract_time, refract_reflection_time,
             wave_scale_time,
             color_underwater_time;

   EditWaterMtrl();

   // get
   bool usesTexBump()C;

   bool equal(C EditWaterMtrl &src)C;
   bool newer(C EditWaterMtrl &src)C;

   // operations
   void reset();                
   void newData(C Project&proj);
   /*void create(C WaterMtrl &src, C TimeStamp &time=TimeStamp().getUTC())
   {
      super.create(Material(), time); // call super to setup times for all values
      color_s               =Vec4(src.colorS(), 1); color_time=time;
      smooth                =src.smooth(); smooth_time=time;
      reflect_min           =src.reflect; reflect_time=time;
      normal                =src.normal ; normal_time=time;
      wave_scale            =src.wave_scale; wave_scale_time=time;
      scale_color           =src.scale_color; scale_color_time=time;
      scale_normal          =src.scale_normal; scale_normal_time=time;
      scale_bump            =src.scale_bump; scale_bump_time=time;
      density               =src.density;
      density_add           =src.density_add; density_time=time;
      refract               =src.refract; refract_time=time;
      refract_reflection    =src.refract_reflection; refract_reflection_time=time;
      color_underwater0     =src.colorUnderwater0S(); color_underwater_time=time;
      color_underwater1     =src.colorUnderwater1S();
      base_0_tex=src. colorMap().id();
      base_1_tex=src.normalMap().id();
      base_2_tex=src.  bumpMap().id();
   }*/
   void copyTo(WaterMtrl &dest, C Project&proj)C;
   uint sync(C EditMaterial  &src);           
   uint sync(C EditWaterMtrl &src);
   uint undo(C EditWaterMtrl &src);
   void adjustParams(uint changed, TEX_FLAG old_textures, TEX_FLAG has_textures, TEX_FLAG known_textures); // 'old_textures'=textures() before making any change, 'has_textures'=used textures based on per-pixel data (if known), 'known_textures'=what textures in 'has_textures' are known

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
