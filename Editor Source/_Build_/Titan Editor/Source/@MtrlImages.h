/******************************************************************************/
/******************************************************************************/
class MtrlImages {
    class ImageResize : Image {
        VecI2 size; // if >0 then image should be resized
        int filter;
        bool clamp, alpha_weight, keep_edges;

        ImageResize &clearParams();
        ImageResize &del();
        bool create(C VecI2 &size, IMAGE_TYPE type);
        ImageResize &resize(C VecI2 &size);
        ImageResize &setFrom(C TextParam &param);
        void apply();
        operator ImageSource() C;

      public:
        ImageResize();
    };
    bool flip_normal_y, smooth_is_rough;
    int tex;
    ImageResize color, alpha, bump, normal, smooth, metal, glow;

    MtrlImages &del();
    /*bool create(C VecI2 &size)
   {
      del();
      return color .create(size, IMAGE_R8G8B8_SRGB)
          && alpha .create(size, IMAGE_I8)
          && bump  .create(size, IMAGE_I8)
          && normal.create(size, IMAGE_R8G8B8)
          && smooth.create(size, IMAGE_I8)
          && metal .create(size, IMAGE_I8)
          && glow  .create(size, IMAGE_I8);
   }
   void clear()
   {
      flip_normal_y=smooth_is_rough=false;
      tex=0;
      color .clear();
      alpha .clear();
      smooth.clear();
      metal .clear();
      glow  .clear();

      REPD(y, bump.h())
      REPD(x, bump.w())bump.pixB(x, y)=128;

      Color nrm(128, 128, 255);
      REPD(y, normal.h())
      REPD(x, normal.w())normal.color(x, y, nrm);
   }
   void compact()
   {
      if(!(tex&TEXF_COLOR ))color .del();
      if(!(tex&TEXF_ALPHA ))alpha .del();
      if(!(tex&TEXF_BUMP  ))bump  .del();
      if(!(tex&TEXF_NORMAL))normal.del();
      if(!(tex&TEXF_SMOOTH))smooth.del();
      if(!(tex&TEXF_METAL ))metal .del();
      if(!(tex&TEXF_GLOW  ))glow  .del();
   }
   void Export(C Str &name, C Str &ext)C
   {
      color .Export(name+"color." +ext);
      alpha .Export(name+"alpha." +ext);
      bump  .Export(name+"bump."  +ext);
      normal.Export(name+"normal."+ext);
      smooth.Export(name+"smooth."+ext);
      metal .Export(name+"metal." +ext);
      glow  .Export(name+"glow."  +ext);
   }
   static void Crop(ImageResize &image, C Rect &frac)
   {
      if(image.is())
      {
         RectI rect=Round(frac*(Vec2)image.size());
         Image temp; if(temp.createSoft(rect.w(), rect.h(), 1, ImageTypeUncompressed(image.type()))) // crop manually because we need to use Mod
         {
            if(image.lockRead())
            {
               REPD(y, temp.h())
               REPD(x, temp.w())temp.color(x, y, image.color(Mod(x+rect.min.x, image.w()), Mod(y+rect.min.y, image.h())));
               image.unlock();
               Swap(temp, SCAST(Image, image));
            }
         }
      }
   }
   void crop(C Rect &frac)
   {
      Crop(color , frac);
      Crop(alpha , frac);
      Crop(bump  , frac);
      Crop(normal, frac);
      Crop(smooth, frac);
      Crop(metal , frac);
      Crop(glow  , frac);
   }
   void resize(C VecI2 &size)
   {
      if(size.x>=0 || size.y>=0)
      {
         color .resize(size);
         alpha .resize(size);
         bump  .resize(size);
         normal.resize(size);
         smooth.resize(size);
         metal .resize(size);
         glow  .resize(size);
      }
   }
   void apply()
   {
      color .apply();
      alpha .apply();
      bump  .apply();
      normal.apply();
      smooth.apply();
      metal .apply();
      glow  .apply();
   }*/
    void fromMaterial(C EditMaterial &material, C Project &proj, bool changed_flip_normal_y = false, bool changed_smooth_is_rough = false);
    void fromMaterial(C EditWaterMtrl &material, C Project &proj, bool changed_flip_normal_y = false, bool changed_smooth_is_rough = false);
    TEX_FLAG createBaseTextures(Image &base_0, Image &base_1, Image &base_2) C;
    TEX_FLAG createWaterBaseTextures(Image &base_0, Image &base_1, Image &base_2) C;
    void baseTextureSizes(VecI2 *size0, VecI2 *size1, VecI2 *size2);
    void waterBaseTextureSizes(VecI2 *size0, VecI2 *size1, VecI2 *size2);
    /*void processAlpha()
   {
      if(!alpha.is() && color.typeInfo().a) // if we have no alpha map but it's possible it's in color
      { // set alpha from color
         color.copy(alpha, -1, -1, -1, IMAGE_A8, IMAGE_SOFT, 1);
         if(alpha.size.x<=0)alpha.size.x=color.size.x; // if alpha size not specified then use from color
         if(alpha.size.y<=0)alpha.size.y=color.size.y;
      }

      if(alpha.is() && alpha.typeChannels()>1 && alpha.typeInfo().a) // if alpha has both RGB and Alpha channels, then check which one to use
         if(alpha.lockRead())
      {
         byte min_alpha=255, min_lum=255;
         REPD(y, alpha.h())
         REPD(x, alpha.w())
         {
            Color c=alpha.color(x, y);
            MIN(min_alpha, c.a    );
            MIN(min_lum  , c.lum());
         }
         alpha.unlock();
         if(min_alpha>=254 && min_lum>=254)alpha.del();else
         alpha.copy(alpha, -1, -1, -1, (min_alpha>=254 && min_lum<254) ? IMAGE_L8 : IMAGE_A8, IMAGE_SOFT, 1); // alpha channel is almost fully white -> use luminance as alpha
      }
   }*/

  public:
    MtrlImages();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
