/******************************************************************************

   Have to keep this as a separate file, so it won't be linked if unused.
   Because it's linked separately, its name can't include spaces (due to Android building toolchain).

/******************************************************************************/
#include "stdafx.h"

#include "../../../ThirdPartyLibs/begin.h"
#include "../../../ThirdPartyLibs/Waifu2x/lib/src/w2xconv.h"
#include "../../../ThirdPartyLibs/Waifu2x/lib/src/modelHandler.hpp"
#include "../../../ThirdPartyLibs/end.h"

namespace EE{
/******************************************************************************/
void WaifuMultiThreadedCall (Int elms, void func(IntPtr elm_index, Ptr user, Int thread_index), Ptr user) {ImageThreads.process(    elms                          , func, user);}
void WaifuMultiThreadedCall1(Int elms, void func(IntPtr elm_index, Ptr user, Int thread_index), Ptr user) {ImageThreads.process(Min(elms, ImageThreads.threads1()), func, user);}
/******************************************************************************/
static struct WaifuClass
{
   w2xc::W2XConv *waifu=null;
   Bool           tried=false;
   Mems<Byte>     data; // this keeps the data needed for waifu to work
   SyncLock       lock;

  ~WaifuClass()
   {
      if(waifu)
      {
         w2xc::w2xconv_fini(waifu);
         waifu=null;
      }
   }
   static void ConvertModel(C Str &json, C Str &bin)
   {
      TextData td;
      if(!td.loadJSON(json))Exit("Can't open model JSON");
      if( td.nodes.elms()!=1)Exit("invalid nodes");
      TextNode &models=td.nodes.first();
      File f; f.writeMem();
      f.putUInt(models.nodes.elms());
      Int last_output;
      FREPA(models.nodes)
      {
         TextNode &model=models.nodes[i];
         TextNode * nInputPlane=model.findNode( "nInputPlane"); if(! nInputPlane)Exit("fail"); Int  inputPlanes= nInputPlane->asInt(); f.putUInt( inputPlanes); if(i && inputPlanes!=last_output)Exit("input!=last_output");
         TextNode *nOutputPlane=model.findNode("nOutputPlane"); if(!nOutputPlane)Exit("fail"); Int outputPlanes=nOutputPlane->asInt(); f.putUInt(outputPlanes);
         TextNode *kW          =model.findNode("kW"); if(!kW)Exit("kW");
         TextNode *kH          =model.findNode("kH"); if(!kH)Exit("kH");
         int kernelSize=kW->asInt(); if(kernelSize!=kH->asInt())Exit("kernel not square"); if(kernelSize!=3)Exit("kernelSize not 3"); f.putUInt(kernelSize);

         TextNode *weight=model.findNode("weight"); if(!weight)Exit("weight");
         if(weight->nodes.elms()!=outputPlanes)Exit("outputPlanes");
         FREPA(weight->nodes)
         {
            TextNode &outputPlane=weight->nodes[i];
            if(outputPlane.nodes.elms()!=inputPlanes)Exit("inputPlanes");
            FREPA(outputPlane.nodes)
            {
               TextNode &weight=outputPlane.nodes[i];
               if(weight.nodes.elms()!=kernelSize)Exit("weight kernelSize");
               FREPA(weight.nodes)
               {
                  TextNode &weightRow=weight.nodes[i];
                  if(weightRow.nodes.elms()!=kernelSize)Exit("weight kernelSize");
                  FREPA(weightRow.nodes)
                  {
                     TextNode &w=weightRow.nodes[i];
                     f<<w.asFlt();
                  }
               }
            }
         }

         TextNode *bias=model.findNode("bias"); if(!bias || bias->nodes.elms()!=outputPlanes)Exit("bias");
         FREPA(bias->nodes)
         {
            TextNode &b=bias->nodes[i];
            f<<b.asFlt();
         }

         last_output=outputPlanes;
      }
      f.pos(0);
      Exit(SafeOverwrite(f, bin) ? "OK" : "save failed");
   }
   static void convert()
   {
      ConvertModel("C:/Esenthel/ThirdPartyLibs/Waifu2x/lib/models_rgb/scale2.0x_model.json", "C:/Esenthel/Data/Waifu/scale2.0x_model.dat");
   }
   Bool init()
   {
   #if 0
      #pragma message("!! Warning: Use this only one time !!")
      convert();
   #endif
      if(!tried)
      {
         SyncLocker locker(lock); if(!tried)
         {
            File f; if(f.readTry("Waifu/scale2.0x_model.dat"))
            {
               data.setNum(f.size());
               if(f.getFast(data.data(), data.elms()))
               {
                  auto temp=w2xc::w2xconv_init();
                  f.readMem(data.data(), data.elms());
                  temp->scale2_models.resize(f.getUInt());
                  for(Int i=0; i<temp->scale2_models.size(); i++)
                  {
                     auto &model=temp->scale2_models[i];
                     model. nInputPlanes=f.getUInt();
                     model.nOutputPlanes=f.getUInt();
                     model.kernelSize   =f.getUInt(); DYNAMIC_ASSERT(model.kernelSize==3, "kernel size!=3");
                     // just store pointers to 'f' data, which actually points to 'data'
                     model.weights=(w2xc::Model::Weight*)f.memFast(); f.skip(SIZE(w2xc::Model::Weight)*model.getNInputPlanes()*model.getNOutputPlanes());
                     model. biases=(float              *)f.memFast(); f.skip(SIZE(Flt                )*model.biases_size());
                  }
                  DYNAMIC_ASSERT(f.end() && f.ok(), "invalid waifu data");
                  waifu=temp; // set once fully initialized
               }
            }
            tried=true; // set at the end
         }
      }
      return waifu!=null;
   }
   Bool process(Image &image, Bool clamp) // !! 'image' must be IMAGE_F32_3 or IMAGE_F32_3_SRGB, should be resized x2 using FILTER_NONE !!
   {
      return waifu && w2xc::w2xconv_2x_rgb_f32_esenthel(waifu, image.data(), image.pitch(), image.w(), image.h(), clamp);
   }
}Waifu;
/******************************************************************************/
static Bool LockedMipHasAlpha(C Image &image) // assumes that image is not compressed and already locked, processes locked mip-map only
{
   if(image.typeInfo().a)
   {
      if(image.highPrecision())
      {
         REPD(y, image.lh())
         REPD(x, image.lw())if(!Equal(image.colorF(x, y).w, 1.0f, 0.5f/255))return true;
      }else
      {
         REPD(y, image.lh())
         REPD(x, image.lw())if(image.color(x, y).a!=255)return true;
      }
   }
   return false;
}
Bool _ResizeWaifu(C Image &src, Image &dest, UInt flags) // assumes that image HW types are not compressed and already locked, 'src' is smaller than 'dest', processes locked mip-map only
{
   if(Waifu.init())
   {
      Bool   clamp=IcClamp(flags);
      Image  temp[2], temp_alpha;
    C Image *s=&src;
      Int    i=0;

      // process RGB
      IMAGE_TYPE type=(s->sRGB() ? IMAGE_F32_3_SRGB : IMAGE_F32_3); // Waifu works best when operating on perceptual gamma (sRGB for colors and linear for non-colors (normap maps/alpha/etc.)) so just operate on image native gamma. When linear gamma is used for colors, then brightness can change significantly compared to original image.
      Bool first=true;
      do{
         Image &dest=temp[i]; i^=1;
         if(!dest.createSoftTry(s->lw()*2, s->lh()*2, 1, type))return false;
         if(first) // saturate because Waifu will not work well if values are outside of 0..1 range
         {
            first=false;
            REPD(y, dest.lh())
            REPD(x, dest.lw())dest.colorF(x, y, s->colorF(x/2, y/2).sat()); // always use native gamma
         }else
         {
            REPD(y, dest.lh())
            REPD(x, dest.lw())dest.colorF(x, y, s->colorF(x/2, y/2)); // always use native gamma
         }
         if(!Waifu.process(dest, clamp))return false;
         s=&dest;
      }while(s->lw()<dest.lw() || s->lh()<dest.lh());

      // process ALPHA
      if(dest.typeInfo().a && LockedMipHasAlpha(src)) // it's important to skip alpha when not needed because it requires a separate resize call (making resize 2x slower)
      {
         if(!s->copyTry(temp_alpha, -1, -1, -1, dest.hwType(), IMAGE_SOFT, 1, FILTER_BEST, flags))return false; // copy to 'temp_alpha' and include alpha channel format !! THIS MUST USE 'hwType' BECAUSE 'type' CAN BE COMPRESSED !!

         // copy alpha channel from 'src' into 'temp' as RGB
         s=&src;
         {
            Image &dest=temp[i]; i^=1;
            if(!dest.createSoftTry(s->lw()*2, s->lh()*2, 1, IMAGE_F32_3))return false;
            REPD(y, dest.lh())
            REPD(x, dest.lw())dest.colorF(x, y, Sat(s->colorF(x/2, y/2).w)); // saturate because Waifu will not work well if values are outside of 0..1 range
            if(!Waifu.process(dest, clamp))return false;
            s=&dest;
         }

         // upscale alpha
         while(s->lw()<dest.lw() || s->lh()<dest.lh())
         {
            Image &dest=temp[i]; i^=1;
            if(!dest.createSoftTry(s->lw()*2, s->lh()*2, 1, IMAGE_F32_3))return false;
            REPD(y, dest.lh())
            REPD(x, dest.lw())dest.colorF(x, y, s->colorF(x/2, y/2));
            if(!Waifu.process(dest, clamp))return false;
            s=&dest;
         }

         // put alpha from 's' RGB to 'temp_alpha' A
         REPD(y, temp_alpha.lh())
         REPD(x, temp_alpha.lw())
         {
            Vec4 c=temp_alpha.colorF(x, y);
            c.w=s->pixelF(x, y);
            temp_alpha.colorF(x, y, c);
         }

         s=&temp_alpha;
      }

      // store into 'dest'
      return dest.injectMipMap(*s, dest.lMipMap(), dest.lCubeFace(), FILTER_BEST, flags);
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
