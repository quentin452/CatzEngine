/******************************************************************************

   !! KEEP THIS AS SEPARATE CPP, SO WHEN UNUSED IT WON'T INCREASE BINARY SIZE !!

/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
#define ETC_LIB_ISPC     1 // only ETC1 compression, quality is good and comparable to ETC_LIB_ETCPACK with quality=0, but much faster
#define ETC_LIB_RG       2 // only ETC1
#define ETC_LIB_ETCPACK  3
#define ETC_LIB_ETC2COMP 4 // quality similar to ETCPACK however it's around 100x slower

#define ETC1_ENC ETC_LIB_ISPC
#define ETC2_ENC ETC_LIB_ETCPACK

#include "../../../../ThirdPartyLibs/begin.h"

#if ETC1_ENC==ETC_LIB_ISPC
   #ifndef ISPC_C__ESENTHEL_THIRDPARTYLIBS_BC_ISPC_TEXCOMP_KERNEL_ISPC_H // this helps on Android where this header could have been already included due to files packed to one CPP file
      #include "../../../../ThirdPartyLibs/BC/ispc_texcomp/ispc_texcomp.h"
   #endif
#endif

#if ETC1_ENC==ETC_LIB_RG
   #include "../../../../ThirdPartyLibs/RG-ETC1/rg_etc1.cpp"
#endif

#if ETC1_ENC==ETC_LIB_ETCPACK || ETC2_ENC==ETC_LIB_ETCPACK
   namespace ETCPACK
   {
      #define EXHAUSTIVE_CODE_ACTIVE 0 // disable to reduce code size and because this mode is too slow
      #define PGMOUT 0
      #define printf(x,...)
      #define   exit(x)
      #pragma warning(disable:4065) // switch statement contains 'default' but no 'case' labels
      #pragma warning(push)
      #pragma warning(disable:4390) // ';': empty controlled statement found; is this the intent?
      #pragma runtime_checks("", off)
      #include "../../../../ThirdPartyLibs/ETCPack/source/etcdec.cxx"
      #undef exit
      #include "../../../../ThirdPartyLibs/ETCPack/source/etcpack.cxx"
      #pragma runtime_checks("", restore)
      #pragma warning(pop)
      #undef R
      #undef G
      #undef B
      #undef RED
      #undef GREEN
      #undef BLUE
      #undef SHIFT
      #undef MASK

   #if ETC2_ENC==ETC_LIB_ETCPACK
      static struct ETCInit
      {
         ETCInit() {setupAlphaTableAndValtab();}
      }EI;
   #endif
   }
#endif

#if ETC1_ENC==ETC_LIB_ETC2COMP || ETC2_ENC==ETC_LIB_ETC2COMP
   #include "../../../../ThirdPartyLibs/etc2comp/lib/EtcImage.cpp"
   #include "../../../../ThirdPartyLibs/etc2comp/lib/EtcBlock4x4.cpp"
   #include "../../../../ThirdPartyLibs/etc2comp/lib/EtcBlock4x4Encoding.cpp"
   #include "../../../../ThirdPartyLibs/etc2comp/lib/EtcBlock4x4Encoding_ETC1.cpp"
   #include "../../../../ThirdPartyLibs/etc2comp/lib/EtcBlock4x4Encoding_R11.cpp"
   #include "../../../../ThirdPartyLibs/etc2comp/lib/EtcBlock4x4Encoding_RG11.cpp"
   #include "../../../../ThirdPartyLibs/etc2comp/lib/EtcBlock4x4Encoding_RGB8.cpp"
   #include "../../../../ThirdPartyLibs/etc2comp/lib/EtcBlock4x4Encoding_RGB8A1.cpp"
   #include "../../../../ThirdPartyLibs/etc2comp/lib/EtcBlock4x4Encoding_RGBA8.cpp"
   #include "../../../../ThirdPartyLibs/etc2comp/lib/EtcSortedBlockList.cpp"
   #include "../../../../ThirdPartyLibs/etc2comp/lib/EtcDifferentialTrys.cpp"
   #include "../../../../ThirdPartyLibs/etc2comp/lib/EtcIndividualTrys.cpp"
   #include "../../../../ThirdPartyLibs/etc2comp/lib/EtcMath.cpp"
#endif

#include "../../../../ThirdPartyLibs/end.h"
/******************************************************************************/
namespace EE{
/******************************************************************************/
struct ETCContext
{
   Bool   etc1;
   Int    quality, x_blocks, y_blocks, sz;
   Byte  *dest_data_z;
 C Image &src, *s;
   Image &dest;

#if ETC1_ENC==ETC_LIB_ISPC // ISPC
   Bool             fast;
   etc_enc_settings settings;
   rgba_surface     surf;
#endif

#if ETC1_ENC==ETC_LIB_RG // RG
   rg_etc1::etc1_pack_params pack;
#endif

#if ETC1_ENC==ETC_LIB_ETCPACK || ETC2_ENC==ETC_LIB_ETCPACK // ETCPACK
   Int block_size;
#endif
   
   ETCContext(C Image &src, Image &dest, Int quality) : quality(quality), src(src), dest(dest) {}

#if ETC1_ENC==ETC_LIB_ETCPACK || ETC2_ENC==ETC_LIB_ETCPACK // ETCPACK
   static void Process(IntPtr elm_index, ETCContext &ctx, Int thread_index) {ctx.process(elm_index);}
          void process(Int    by)
   {
      Int py=by*4, yo[4]; REPAO(yo)=Min(py+i, s->lh()-1); // use clamping to avoid black borders
      Byte *dest_data=dest_data_z + by*dest.pitch();
      REPD(bx, x_blocks)
      {
         Int   px=bx*4, xo[4]; REPAO(xo)=Min(px+i, s->lw()-1); // use clamping to avoid black borders
         Vec2  rg  [4][4];
         VecB  rgb [4][4];
         Color rgba[4][4];
         Byte     a[4][4];
         Color temp[4][4];
         UInt *d=(UInt*)(dest_data + bx*block_size);
         switch(dest.hwType())
         {
            case IMAGE_ETC1: s->gather(&rgb[0][0], xo, Elms(xo), yo, Elms(yo), &sz, 1); switch(quality)
            {
               default: ETCPACK::compressBlockDiffFlipFast  (rgb[0][0].c, temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
            #if EXHAUSTIVE_CODE_ACTIVE
               case  1: ETCPACK::compressBlockETC1Exhaustive(rgb[0][0].c, temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
            #endif
            }break;

            /*case IMAGE_ETC1_SRGB: s->gather(&rgb[0][0], xo, Elms(xo), yo, Elms(yo), &sz, 1); switch(quality)
            {
               default: ETCPACK::compressBlockDiffFlipFastPerceptual  (rgb[0][0].c, temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
            #if EXHAUSTIVE_CODE_ACTIVE
               case  1: ETCPACK::compressBlockETC1ExhaustivePerceptual(rgb[0][0].c, temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
            #endif
            }break;*/

            case IMAGE_ETC2_R:
            {
               s->gather(&rg[0][0], xo, Elms(xo), yo, Elms(yo), &sz, 1);
               U16 src[4*4]; REPAO(src)=FltToU16(rg[0][i].x); ETCPACK::compressBlockAlpha16(src, (Byte*)d, false);
            }goto no_swap;

            case IMAGE_ETC2_R_SIGN:
            {
               s->gather(&rg[0][0], xo, Elms(xo), yo, Elms(yo), &sz, 1);
               U16 src[4*4]; REPAO(src)=SFltToShort(rg[0][i].x)+32768; ETCPACK::compressBlockAlpha16(src, (Byte*)d, true);
            }goto no_swap;

            case IMAGE_ETC2_RG:
            {
               s->gather(&rg[0][0], xo, Elms(xo), yo, Elms(yo), &sz, 1);
               U16 src[4*4]; REPAO(src)=FltToU16(rg[0][i].x); ETCPACK::compressBlockAlpha16(src, (Byte*)d  , false);
                             REPAO(src)=FltToU16(rg[0][i].y); ETCPACK::compressBlockAlpha16(src, (Byte*)d+8, false);
            }goto no_swap;

            case IMAGE_ETC2_RG_SIGN:
            {
               s->gather(&rg[0][0], xo, Elms(xo), yo, Elms(yo), &sz, 1);
               U16 src[4*4]; REPAO(src)=SFltToShort(rg[0][i].x)+32768; ETCPACK::compressBlockAlpha16(src, (Byte*)d  , true);
                             REPAO(src)=SFltToShort(rg[0][i].y)+32768; ETCPACK::compressBlockAlpha16(src, (Byte*)d+8, true);
            }goto no_swap;

            case IMAGE_ETC2_RGB: s->gather(&rgb[0][0], xo, Elms(xo), yo, Elms(yo), &sz, 1); switch(quality)
            {
               default: ETCPACK::compressBlockETC2Fast      (rgb[0][0].c, null, temp[0][0].c, 4, 4, 0, 0, d[0], d[1], ETCPACK::ETC2PACKAGE_RGB_NO_MIPMAPS); break;
            #if EXHAUSTIVE_CODE_ACTIVE
               case  1: ETCPACK::compressBlockETC2Exhaustive(rgb[0][0].c,       temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
            #endif
            }break;

            case IMAGE_ETC2_RGB_SRGB: s->gather(&rgb[0][0], xo, Elms(xo), yo, Elms(yo), &sz, 1); switch(quality)
            {
               default: ETCPACK::compressBlockETC2FastPerceptual      (rgb[0][0].c, temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
            #if EXHAUSTIVE_CODE_ACTIVE
               case  1: ETCPACK::compressBlockETC2ExhaustivePerceptual(rgb[0][0].c, temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
            #endif
            }break;

            case IMAGE_ETC2_RGBA1: case IMAGE_ETC2_RGBA1_SRGB:
            {
               s->gather(&rgba[0][0], xo, Elms(xo), yo, Elms(yo), &sz, 1);
               REPD(y, 4)
               REPD(x, 4)
               {
                  rgb[y][x]=  rgba[y][x].v3;
                  a  [y][x]=((rgba[y][x].a>=128) ? 255 : 0); // manually adjust because compressor has issues
               }
               ETCPACK::compressBlockETC2Fast(rgb[0][0].c, &a[0][0], temp[0][0].c, 4, 4, 0, 0, d[0], d[1], ETCPACK::ETC2PACKAGE_RGBA1_NO_MIPMAPS);
            }break;

            case IMAGE_ETC2_RGBA: case IMAGE_ETC2_RGBA_SRGB:
            {
               s->gather(&rgba[0][0], xo, Elms(xo), yo, Elms(yo), &sz, 1);
               REPD(y, 4)
               REPD(x, 4)
               {
                  rgb[y][x]=rgba[y][x].v3;
                  a  [y][x]=rgba[y][x].a;
               }

               switch(quality)
               {
                  default: ETCPACK::compressBlockAlphaFast(&a[0][0], 0, 0, 4, 4, (Byte*)d); break;
                  case  1: ETCPACK::compressBlockAlphaSlow(&a[0][0], 0, 0, 4, 4, (Byte*)d); break;
               }

               d+=2;

               switch(dest.hwType())
               {
                  case IMAGE_ETC2_RGBA: switch(quality)
                  {
                     default: ETCPACK::compressBlockETC2Fast      (rgb[0][0].c, null, temp[0][0].c, 4, 4, 0, 0, d[0], d[1], ETCPACK::ETC2PACKAGE_RGB_NO_MIPMAPS); break;
                  #if EXHAUSTIVE_CODE_ACTIVE
                     case  1: ETCPACK::compressBlockETC2Exhaustive(rgb[0][0].c,       temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
                  #endif
                  }break;

                  case IMAGE_ETC2_RGBA_SRGB: switch(quality)
                  {
                     default: ETCPACK::compressBlockETC2FastPerceptual      (rgb[0][0].c, temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
                  #if EXHAUSTIVE_CODE_ACTIVE
                     case  1: ETCPACK::compressBlockETC2ExhaustivePerceptual(rgb[0][0].c, temp[0][0].c, 4, 4, 0, 0, d[0], d[1]); break;
                  #endif
                  }break;
               }
            }break;
         }
         SwapEndian(d[0]);
         SwapEndian(d[1]);
         no_swap:;
      }
   }
#endif

   Bool process()
   {
      etc1=(dest.hwType()==IMAGE_ETC1);
      if(etc1 || dest.hwType()==IMAGE_ETC2_R      || dest.hwType()==IMAGE_ETC2_RG      || dest.hwType()==IMAGE_ETC2_RGB      || dest.hwType()==IMAGE_ETC2_RGBA1      || dest.hwType()==IMAGE_ETC2_RGBA
              || dest.hwType()==IMAGE_ETC2_R_SIGN || dest.hwType()==IMAGE_ETC2_RG_SIGN || dest.hwType()==IMAGE_ETC2_RGB_SRGB || dest.hwType()==IMAGE_ETC2_RGBA1_SRGB || dest.hwType()==IMAGE_ETC2_RGBA_SRGB)
    //if(dest.size3()==src.size3()) this check is not needed because the code below supports different sizes
      {
         ImageThreads.init();

      #if ETC1_ENC==ETC_LIB_ISPC // ISPC
         if(etc1)GetProfile_etc_slow(&settings);
      #endif
      #if ETC1_ENC==ETC_LIB_RG // RG
         if(etc1)
         {
            rg_etc1::pack_etc1_block_init();
            switch(quality)
            {
               case 0: pack.m_quality=rg_etc1::cLowQuality   ; break; // makes gradients look bad
      default: case 1: pack.m_quality=rg_etc1::cMediumQuality; break; // much faster than high and almost as good
               case 2: pack.m_quality=rg_etc1::cHighQuality  ; break; // very slow
            }
            pack.m_dithering=false; // dithering here is actually very bad
         }
      #endif
      #if ETC1_ENC==ETC_LIB_ETCPACK || ETC2_ENC==ETC_LIB_ETCPACK // ETCPACK
         if(etc1 ? ETC1_ENC==ETC_LIB_ETCPACK : ETC2_ENC==ETC_LIB_ETCPACK)
         {
            if(quality<0)quality=0; // default to 0 as 1 is just too slow
            quality=Mid(quality, 0, EXHAUSTIVE_CODE_ACTIVE);
            block_size=dest.hwTypeInfo().block_bytes;
         }
      #endif

         Int src_faces1=src.faces()-1;
         Image temp; // defined outside of loop to avoid overhead
         REPD(mip, Min(src.mipMaps(), dest.mipMaps()))
         {
            s=&src;
            Bool read_from_src=true;
            Int  dest_mip_hwW=PaddedWidth (dest.hwW(), dest.hwH(), mip, dest.hwType()), // operate on mip HW size to process partial and Pow2Padded blocks too
                 dest_mip_hwH=PaddedHeight(dest.hwW(), dest.hwH(), mip, dest.hwType());
         #if ETC1_ENC==ETC_LIB_ISPC // ISPC
            if(etc1)
            {
               // to directly read from 'src', we need to match requirements for compressor, which needs:
               read_from_src=((src.hwType()==IMAGE_R8G8B8A8 || src.hwType()==IMAGE_R8G8B8A8_SRGB) // IMAGE_R8G8B8A8 hw type
                           && Max(1, src.w()>>mip)>=dest_mip_hwW   // src mip width  must be at least as dest mip width
                           && Max(1, src.h()>>mip)>=dest_mip_hwH); // src mip height must be at least as dest mip height
               if(!read_from_src)s=&temp;
            }
         #endif
            x_blocks=dest_mip_hwW/4; // operate on HW size to process partial and Pow2Padded blocks too
            y_blocks=dest_mip_hwH/4;
            REPD(face, dest.faces())
            {
               if(!read_from_src)
               {
                  if(!src.extractNonCompressedMipMapNoStretch(temp, dest_mip_hwW, dest_mip_hwH, 1, mip, (DIR_ENUM)Min(face, src_faces1), true))return false;
                  if(temp.hwType()!=IMAGE_R8G8B8A8 && temp.hwType()!=IMAGE_R8G8B8A8_SRGB)if(!temp.copyTry(temp, -1, -1, -1, dest.sRGB() ? IMAGE_R8G8B8A8_SRGB : IMAGE_R8G8B8A8))return false;
               }else
               if(! src.lockRead(            mip, (DIR_ENUM)Min(face, src_faces1)))                                return false; // we have to lock only for 'src' because 'temp' is 1mip-1face-SOFT and doesn't need locking
               if(!dest.lock    (LOCK_WRITE, mip, (DIR_ENUM)    face             )){if(read_from_src)src.unlock(); return false;}

            #if ETC1_ENC==ETC_LIB_ISPC // ISPC
               if(etc1)
               {
                  fast=(dest.pitch()==x_blocks*8);
                  surf.width =dest_mip_hwW;
                  surf.height=dest_mip_hwH;
                  surf.stride=s->pitch();
               }
            #endif

               REPD(z, dest.ld())
               {
                  sz=Min(z, s->ld()-1);
                  dest_data_z=dest.data() + z*dest.pitch2();
                  // compress
               #if ETC1_ENC==ETC_LIB_ISPC // ISPC
                  if(etc1)
                  {
                     surf.ptr=ConstCast(s->data() + sz*s->pitch2());
                     if(fast)CompressBlocksETC1(&surf, dest_data_z, &settings);else
                     {
                        surf.height=4;
                        REP(y_blocks)
                        {
                           CompressBlocksETC1(&surf, dest_data_z, &settings);
                           surf.ptr   +=s  ->pitch()*4;
                           dest_data_z+=dest.pitch();
                        }
                     }
                  }else
               #endif
               #if ETC1_ENC==ETC_LIB_RG // RG
                  if(etc1)
                  {
                     REPD(by, y_blocks)
                     {
                        Int py=by*4, yo[4]; REPAO(yo)=Min(py+i, s->lh()-1); // use clamping to avoid black borders
                        Byte *dest_data=dest_data_z + by*dest.pitch();
                        REPD(bx, x_blocks)
                        {
                           Int px=bx*4, xo[4]; REPAO(xo)=Min(px+i, s->lw()-1); // use clamping to avoid black borders
                           Color rgba[4][4]; s->gather(&rgba[0][0], xo, Elms(xo), yo, Elms(yo), &sz, 1);
                           rg_etc1::pack_etc1_block((UInt*)(dest_data + bx*8), (UInt*)rgba, pack);
                        }
                     }
                  }else
               #endif
               #if ETC1_ENC==ETC_LIB_ETCPACK || ETC2_ENC==ETC_LIB_ETCPACK // ETCPACK
                  if(etc1 ? ETC1_ENC==ETC_LIB_ETCPACK : ETC2_ENC==ETC_LIB_ETCPACK)
                  {
                     ImageThreads.process(y_blocks, Process, T);
                  }else
               #endif
                     {}
               }
                               dest.unlock();
               if(read_from_src)src.unlock();
            }
         }
         return true;
      }
      return false;
   }
};
/******************************************************************************/
Bool _CompressETC(C Image &src, Image &dest, Int quality)
{
#if ETC1_ENC==ETC_LIB_ETC2COMP || ETC2_ENC==ETC_LIB_ETC2COMP
   unfinished
   Etc::Image::Format format;
   switch(type)
   {
      case IMAGE_ETC1           : format=Etc::Image::Format::ETC1       ; break;
      case IMAGE_ETC2_R         : format=Etc::Image::Format::R11        ; break;
      case IMAGE_ETC2_R_SIGN    : format=Etc::Image::Format::SIGNED_R11 ; break;
      case IMAGE_ETC2_RG        : format=Etc::Image::Format::RG11       ; break;
      case IMAGE_ETC2_RG_SIGN   : format=Etc::Image::Format::SIGNED_RG11; break;
      case IMAGE_ETC2_RGB       : format=Etc::Image::Format::RGB8       ; break;
      case IMAGE_ETC2_RGB_SRGB  : format=Etc::Image::Format::SRGB8      ; break;
      case IMAGE_ETC2_RGBA1     : format=Etc::Image::Format::RGB8A1     ; break;
      case IMAGE_ETC2_RGBA1_SRGB: format=Etc::Image::Format::SRGB8A1    ; break;
      case IMAGE_ETC2_RGBA      : format=Etc::Image::Format::RGBA8      ; break;
      case IMAGE_ETC2_RGBA_SRGB : format=Etc::Image::Format::SRGBA8     ; break;
      default                   : format=Etc::Image::Format::UNKNOWN    ; break;
   }
   if(format!=Etc::Image::Format::UNKNOWN)
   {
      Image src; img.mustCopy(src, -1, -1, -1, img.sRGB() ? IMAGE_F32_4_SRGB : IMAGE_F32_4);

		auto error_metric=src.sRGB() ? Etc::ErrorMetric::REC709 : Etc::ErrorMetric::RGBX; // FIXME can use NORMALXYZ? careful maybe it needs Z which is not included in RG
      auto effort=70; // REC709(100 = 83.7s, 70=44.1s, 40=8.3s), RGBX(70=26.3s)
      Etc::Image image((Flt*)src.data(), src.w(), src.h(), error_metric);

		image.Encode(format, error_metric, effort, Cpu.threads(), Cpu.threads());
      Image out; out.mustCreateSoft(src.w(), src.h(), src.d(), type, 1);
      CopyFast(out.data(), image.GetEncodingBits(), out.memUsage());
   }
#endif
   return ETCContext(src, dest, quality).process();
}
/******************************************************************************/
}
/******************************************************************************/
