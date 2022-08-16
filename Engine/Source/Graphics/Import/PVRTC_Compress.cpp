/******************************************************************************

   !! KEEP THIS AS SEPARATE CPP, SO WHEN UNUSED IT WON'T INCREASE BINARY SIZE !!

/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
#include "../../../../ThirdPartyLibs/begin.h"
#include "../../../../ThirdPartyLibs/PVRTC/PVRTex/PVRTextureDefines.h"
#include "../../../../ThirdPartyLibs/end.h"
/******************************************************************************/
namespace EE{
/******************************************************************************/
extern Bool _CompressPVRTC(Int w, Int h, CPtr data, Int type, Int quality, Ptr &compressed_data, Int &compressed_size);
/******************************************************************************/
Bool _CompressPVRTC(C Image &src, Image &dest, Int quality)
{
   Bool ok=false;
   if(dest.hwType()==IMAGE_PVRTC1_2 || dest.hwType()==IMAGE_PVRTC1_2_SRGB
   || dest.hwType()==IMAGE_PVRTC1_4 || dest.hwType()==IMAGE_PVRTC1_4_SRGB)
 //if(dest.size3()==src.size3()) this check is not needed because the code below supports different sizes
   {
      ok=true;
      if(quality<0)quality=((dest.hwType()==IMAGE_PVRTC1_2 || dest.hwType()==IMAGE_PVRTC1_2_SRGB) ? 3 : 4); // for PVRTC1_2 default to 3 (test=46s, because 4 is too slow test=100s), for PVRTC1_4 default to 4 (not slow, test=27s)
      switch(quality)
      {
         case  0: quality=pvrtexture::ePVRTCFastest; break;
         case  1: quality=pvrtexture::ePVRTCFast   ; break;
         case  2: quality=pvrtexture::ePVRTCNormal ; break;
         case  3: quality=pvrtexture::ePVRTCHigh   ; break;
         default: quality=pvrtexture::ePVRTCBest   ; break;
      }
      Int src_faces1=src.faces()-1;
      Image temp; // defined outside of loop to avoid overhead
      REPD(mip, Min(src.mipMaps(), dest.mipMaps()))
      {
         if(!temp.createTry(PaddedWidth(dest.w(), dest.h(), mip, dest.hwType()), PaddedHeight(dest.w(), dest.h(), mip, dest.hwType()), 1, dest.sRGB() ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8A8, IMAGE_SOFT, 1))return false; //  use R8G8B8A8 because PVRTEX operates on that format
         REPD(face, dest.faces())
         {
            if(! src.lockRead(            mip, (DIR_ENUM)Min(face, src_faces1)))               return false;
            if(!dest.lock    (LOCK_WRITE, mip, (DIR_ENUM)    face             )){src.unlock(); return false;}

            REPD(z, dest.ld())
            {
               Int sz=Min(z, src.ld()-1);
               // copy 'src' to 'temp'
               REPD(y, temp.lh())
               REPD(x, temp.lw())temp.color(x, y, src.color3D(Min(x, src.lw()-1), Min(y, src.lh()-1), sz)); // use clamping to avoid black borders

               // compress
               Ptr data=null;
               Int size=0;
               if(_CompressPVRTC(temp.lw(), temp.lh(), temp.data(), (dest.hwType()==IMAGE_PVRTC1_2 || dest.hwType()==IMAGE_PVRTC1_2_SRGB) ? ePVRTPF_PVRTCI_2bpp_RGBA : ePVRTPF_PVRTCI_4bpp_RGBA, quality, data, size))
               {
                  if(size==temp.lw()*temp.lh()*dest.hwTypeInfo().bit_pp/8)
                  {
                     Byte  *dest_data=dest.data() + z*dest.pitch2();
                     Int        pitch=ImagePitch  (temp.lw(), temp.lh(), 0, dest.hwType()), // compressed pitch of 'data'
                             blocks_y=ImageBlocksY(dest.lw(), dest.lh(), 0, dest.hwType());
                     REPD(y, blocks_y)Copy(dest_data + y*dest.pitch(), (Byte*)data + y*pitch, dest.pitch(), pitch); // zero remaining Pow2Padded data to avoid garbage
                     Int written=blocks_y*dest.pitch(); Zero(dest_data+written, dest.pitch2()-written); // zero remaining Pow2Padded data to avoid garbage
                  }else ok=false;
                  free(data); // was allocated using 'malloc'
               }else ok=false;
               if(!ok)break;
            }

            dest.unlock();
             src.unlock();
            if(!ok)break;
         }
      }
   }
   return ok;
}
/******************************************************************************/
}
/******************************************************************************/
