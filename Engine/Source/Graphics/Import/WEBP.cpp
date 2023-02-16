/******************************************************************************/
#include "stdafx.h"

#if SUPPORT_WEBP
   #include "../../../../ThirdPartyLibs/begin.h"

   #include "../../../../ThirdPartyLibs/Webp/src/webp/decode.h"
   #include "../../../../ThirdPartyLibs/Webp/src/webp/encode.h"

   #include "../../../../ThirdPartyLibs/end.h"
#endif

namespace EE{
/******************************************************************************/
Bool Image::ImportWEBP(File &f)
{
   Bool ok=false;
#if SUPPORT_WEBP
   if(f.getUInt()==CC4('R','I','F','F'))
   {
      UInt size;
      if(f.getFast(size)
      && f.left()>=size
      && f.getUInt()==CC4('W','E','B','P')
      && f.skip(-12))
      {
         size+=8; // we have to add 8 for the RIFF+size data
         Memt<Byte> temp;
         Ptr        data;
         if(f._type==FILE_MEM)data=f.memFast();else
         {
            data=temp.setNumDiscard(size).data();
            if(!f.getFast(data, size))goto error;
         }
         WebPDecoderConfig         config;
         if(WebPInitDecoderConfig(&config))
         {
          //config.options.use_threads=1; made no difference in performance
            if(WebPGetFeatures((uint8_t*)data, size, &config.input)==VP8_STATUS_OK)
            {
               IMAGE_TYPE type;
               if(config.input.has_alpha)
               {
                  type                    =IMAGE_R8G8B8A8_SRGB;
                  config.output.colorspace=MODE_RGBA;
               }else
               {
                  type                    =IMAGE_R8G8B8_SRGB;
                  config.output.colorspace=MODE_RGB;
               }
               if(createSoft(config.input.width, config.input.height, 1, type))
               {
                  config.output.is_external_memory=true;
                  config.output.u.RGBA.stride=T.pitch ();
                  config.output.u.RGBA.size  =T.pitch2();
                  config.output.u.RGBA.rgba  =T.data  ();
                  if(WebPDecode((uint8_t*)data, size, &config)==VP8_STATUS_OK)ok=true;
               }
            }
            WebPFreeDecBuffer(&config.output);
         }
      error:;
      }
   }
#endif
   if(!ok)del(); return ok;
}
Bool Image::ImportWEBP(C Str &name)
{
#if SUPPORT_WEBP
   File f; if(f.read(name))return ImportWEBP(f);
#endif
   del(); return false;
}
/******************************************************************************/
#if SUPPORT_WEBP
static int WEBPWriter(const uint8_t* data, size_t data_size, const WebPPicture* const pic)
{
   File  &f=*(File*)pic->custom_ptr;
   return f.put(data, (Int)data_size);
}
#endif
Bool Image::ExportWEBP(File &f, Flt rgb_quality, Flt alpha_quality)C
{
   Bool ok=false;
#if SUPPORT_WEBP
 C Image *src=this;
   Image  temp;
   if(src->cube  ()                                                      )if(temp.fromCube(*src ,             IMAGE_B8G8R8A8_SRGB               ))src=&temp;else return false;
   if(src->hwType()!=IMAGE_B8G8R8A8 && src->hwType()!=IMAGE_B8G8R8A8_SRGB)if(src->copy    ( temp, -1, -1, -1, IMAGE_B8G8R8A8_SRGB, IMAGE_SOFT, 1))src=&temp;else return false; // WEBP uses BGRA

   if(src->w()<=WEBP_MAX_DIMENSION
   && src->h()<=WEBP_MAX_DIMENSION)
      if(src->lockRead())
   {
      WebPPicture picture;
      WebPConfig  config;
      if(WebPPictureInit(&picture)
      && WebPConfigInit (&config))
      {
         Flt q=rgb_quality*100;
         if( q<  0)q=100;else // default to 100=lossless
         if( q>100)q=100;

         Flt aq=alpha_quality*100;
         if( aq<  0)aq=  q;else // default to 'q'=rgb_quality
         if( aq>100)aq=100;

       //config .pass         =10; // very little difference, not sure if better or worse
       //config .method       =6; // don't turn this on, because for lossless it can be too slow for camera photographs (91s for 4K x 2K photo, default method=4 is 7.5s), for lossy it's not as slow, it makes file smaller but at lower quality so just skip too
         config .thread_level =1; // 0=disabled, 1=no performance difference, >1 makes the compression to fail
       //config .near_lossless=; // made things much worse
         config .lossless     =(q>=100-EPS);
         config .      quality= q;
         config .alpha_quality=aq;
         config .exact        =true;
         config .use_sharp_yuv=true; // if enabled then introduces plenty of blurry artifacts for pixel-art rescaled x4 NearestNeighbor, however using 'autofilter=1' fixes the problem
         config .autofilter   =1; // enable because of 'use_sharp_yuv'
         picture.width        =src->w();
         picture.height       =src->h();
      #if 1 // RGBA
         picture.use_argb     =true;
         picture.argb         =(uint32_t*)src->data();
         picture.argb_stride  =src->pitch()/src->bytePP(); // in pixel units
      #else // YUVA, this didn't improve the quality, we could use it mainly if we had the source already in YUV format
         Image Y, U, V, A;
         Y.createSoft(src->w(), src->h(), 1, IMAGE_L8);
         U.createSoft(src->w(), src->h(), 1, IMAGE_L8);
         V.createSoft(src->w(), src->h(), 1, IMAGE_L8);
         if(src->typeInfo().a)
            if(src->copy(A, -1, -1, -1, IMAGE_A8, IMAGE_SOFT, 1))
         {
            picture.a       =(uint8_t*)A.data();
            picture.a_stride=A.pitch()/A.bytePP(); // in pixel units
         }
         REPD(y, src->h())
         REPD(x, src->w())
         {
            Vec yuv=RgbToYuv(src->colorF(x, y).xyz);
            Y.pixelF(x, y, yuv.x);
            U.pixelF(x, y, yuv.y);
            V.pixelF(x, y, yuv.z);
         }
         U.downSample();
         V.downSample();
         picture.use_argb =false;
         picture.y        =(uint8_t*)Y.data();
         picture.u        =(uint8_t*)U.data();
         picture.v        =(uint8_t*)V.data();
         picture.y_stride =Y.pitch()/Y.bytePP(); // in pixel units
         picture.uv_stride=U.pitch()/U.bytePP(); // in pixel units
      #endif
         picture.custom_ptr   =&f;
         picture.writer       =WEBPWriter;
         if(WebPEncode(&config, &picture))ok=f.ok();
         WebPPictureFree(&picture);
      }
      src->unlock();
   }
#endif
   return ok;
}
Bool Image::ExportWEBP(C Str &name, Flt rgb_quality, Flt alpha_quality)C
{
#if SUPPORT_WEBP
   File f; if(f.write(name)){if(ExportWEBP(f, rgb_quality, alpha_quality) && f.flush())return true; f.del(); FDelFile(name);}
#endif
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
