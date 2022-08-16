/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
enum TGA_FORMAT : Byte
{
   TGA_NULL    = 0,
   TGA_Map     = 1,
   TGA_RGB     = 2,
   TGA_Mono    = 3,
   TGA_RLEMap  = 9,
   TGA_RLERGB  =10,
   TGA_RLEMono =11,
   TGA_CompMap =32,
   TGA_CompMap4=33,
};
#pragma pack(push, 1)
struct TgaHeader
{
   Byte       IdLength;  // Image ID Field Length
   Byte       CmapType;  // Color Map Type
   TGA_FORMAT ImageType; // Image Type

   UShort CmapIndex;     // First Entry Index
   UShort CmapLength;    // Color Map Length
   Byte   CmapEntrySize; // Color Map Entry Size

   UShort X_Origin;    // X-origin of Image
   UShort Y_Origin;    // Y-origin of Image
   UShort ImageWidth;  // Image Width
   UShort ImageHeight; // Image Height
   Byte   PixelDepth;  // Pixel Depth
   Byte   ImagDesc;    // Image Descriptor
};
#pragma pack(pop)
struct TGA
{
   Bool              mono;
   UInt              bit_pp;
   MemtN<Color, 256> palette;

   void readUncompressed(Image &image, File &f, Int y, Int x_offset, Int width)
   {
	   switch(bit_pp)
	   {
	      case 8:
	      {
            if(palette.elms()   )FREPD(x, width){Byte pixel; f>>pixel; image.color(x+x_offset, y, InRange(pixel, palette) ? palette[pixel] : TRANSPARENT);}else
	         if(image.bytePP()==1)f.get(image.data() + x_offset + y*image.pitch(), width);else
                                 FREPD(x, width){Byte pixel; f>>pixel; image.color(x+x_offset, y, Color(pixel, pixel, pixel));}
	      }break;

	      case 15: case 16: FREPD(x, width){UShort pixel; f>>pixel; image.color(x+x_offset, y, Color((pixel>>7)&0xF8, (pixel>>2)&0xF8, (pixel&0x1F)<<3));} break;

	      case 24: switch(image.hwType())
	      {
            case IMAGE_B8G8R8: case IMAGE_B8G8R8_SRGB: {Byte *data=image.data() + x_offset*3 + y*image.pitch(); f.get(data, width*3);                                          } break;
	         case IMAGE_R8G8B8: case IMAGE_R8G8B8_SRGB: {Byte *data=image.data() + x_offset*3 + y*image.pitch(); f.get(data, width*3); REP(width)Swap(data[i*3+0], data[i*3+2]);} break; // swap Red with Blue

	         case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB:
	         {
	            VecB4 *data=(VecB4*)(image.data() + x_offset*4 + y*image.pitch()); C VecB *src=(VecB*)data; f.get(data, width*3); // read 3-byte to 4-byte memory
               // unpack from 3-byte to 4-byte
               data+=width; src+=width; // have to start from the end to don't overwrite source
	            REP(width){--data; --src; data->set(src->x, src->y, src->z, 255);}
	         }break;

	         case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB:
	         {
	            VecB4 *data=(VecB4*)(image.data() + x_offset*4 + y*image.pitch()); C VecB *src=(VecB*)data; f.get(data, width*3); // read 3-byte to 4-byte memory
               // unpack from 3-byte to 4-byte
               data+=width; src+=width; // have to start from the end to don't overwrite source
	            REP(width){--data; --src; data->set(src->z, src->y, src->x, 255);} // swap Red with Blue
	         }break;

	         default: FREPD(x, width){VecB pixel; f>>pixel; image.color(x+x_offset, y, Color(pixel.z, pixel.y, pixel.x, 255));} break;
		   }break;

	      case 32: switch(image.hwType())
         {
            case IMAGE_B8G8R8A8: case IMAGE_B8G8R8A8_SRGB: {Byte *data=image.data() + x_offset*4 + y*image.pitch(); f.get(data, width*4);                                          } break;
            case IMAGE_R8G8B8A8: case IMAGE_R8G8B8A8_SRGB: {Byte *data=image.data() + x_offset*4 + y*image.pitch(); f.get(data, width*4); REP(width)Swap(data[i*4+0], data[i*4+2]);} break; // swap Red with Blue
            default: FREPD(x, width){VecB4 pixel; f>>pixel; image.color(x+x_offset, y, Color(pixel.z, pixel.y, pixel.x, pixel.w));} break;
         }break;
	   }
   }
   Byte readCompressed(Image &image, File &f, Int y, Byte rleLeftover)
   {
      Byte rle;
      Int  filePos=0;
      for(int x=0; x<image.w(); )
      {
         if(rleLeftover==255)f>>rle;else
         {
            rle=rleLeftover;
            rleLeftover=255;
         }

         if(rle&128) // RLE-encoded packet
         {
            rle-=127; // calculate real repeat count
            if(x+rle>image.w())
            {
               rleLeftover=Byte(128+(rle-(image.w()-x)-1));
               filePos=f.pos();
               rle=Byte(image.w()-x);
            }
            switch(bit_pp)
            {
               case 32:
               {
                  VecB4 pixel; f>>pixel; Color color(pixel.z, pixel.y, pixel.x, pixel.w);
                  REP(rle)image.color(x+i, y, color);
               }break;

               case 24:
               {
	               Byte pixel[3]; f>>pixel; Color color(pixel[2], pixel[1], pixel[0], 255);
	               REP(rle)image.color(x+i, y, color);
               }break;

               case 15:
               case 16:
               {
	               UShort pixel; f>>pixel; Color color((pixel>>7)&0xF8, (pixel>>2)&0xF8, (pixel&0x1F)<<3);
	               REP(rle)image.color(x+i, y, color);
	            }break;

               case 8:
               {
                  Byte pixel; f>>pixel;
                  if(palette.elms()){Color c=(InRange(pixel, palette) ? palette[pixel] : TRANSPARENT); REP(rle)image.color(x+i, y, c);}else
                  if(image.bytePP()==1)REP(rle)image.pixel(x+i, y, pixel);else{Color c(pixel, pixel, pixel); REP(rle)image.color(x+i, y, c);}
               }break;
            }
            if(rleLeftover!=255)f.pos(filePos);
         }else // raw packet
         {
            rle++; // calculate real repeat count
            if(x+rle>image.w())
            {
               rleLeftover=Byte(rle-(image.w()-x)-1);
               rle=Byte(image.w()-x);
            }
            readUncompressed(image, f, y, x, rle);
         }
         x+=rle;
      }
      return rleLeftover;
   }
};
/******************************************************************************/
Bool Image::ImportTGA(File &f, Int type, Int mode, Int mip_maps)
{
   if(mode!=IMAGE_2D && mode!=IMAGE_SOFT)mode=IMAGE_SOFT;
   if(mip_maps<0)mip_maps=1;

	TGA tga;
   TgaHeader header; f>>header;

   Bool compressed, map;
   switch(Unaligned(header.ImageType))
   {
      case TGA_Map : map=true ; tga.mono=false; compressed=false; break;
      case TGA_RGB : map=false; tga.mono=false; compressed=false; break;
      case TGA_Mono: map=false; tga.mono=true ; compressed=false; break;

      case TGA_RLEMap : map=true ; tga.mono=false; compressed=true; break;
      case TGA_RLERGB : map=false; tga.mono=false; compressed=true; break;
      case TGA_RLEMono: map=false; tga.mono=true ; compressed=true; break;

      default: return false;
   }

   if( Unaligned(header.ImageWidth)==0  ||  Unaligned(header.ImageHeight  )==0)return false;
   if( Unaligned(header.PixelDepth)!=8  &&  Unaligned(header.PixelDepth   )!=15 && Unaligned(header.PixelDepth)!=16 && Unaligned(header.PixelDepth)!=24 && Unaligned(header.PixelDepth)!=32)return false;
   if( Unaligned(header.CmapType  )!=0  &&  Unaligned(header.CmapType     )!=1 )return false; // only 0 and 1 types are supported
   if( Unaligned(header.CmapType  )     &&  Unaligned(header.CmapEntrySize)!=24)return false; // if color map exists but entry is not 24-bit
   if((Unaligned(header.CmapType  )!=0) !=  map                                )return false; // if color map existence is different than map type
   if((Unaligned(header.CmapType  )!=0) != (Unaligned(header.CmapLength   )!=0))return false; // if color map existence is different than map length
   f.skip(Unaligned(header.IdLength)); // skip descriptor

   tga.bit_pp=Unaligned(header.PixelDepth);
   tga.palette.setNum(Unaligned(header.CmapLength));
   FREPA(tga.palette){Color &c=tga.palette[i]; f>>c.b>>c.g>>c.r; c.a=255;}

   Bool mirror_x=FlagOn(Unaligned(header.ImagDesc), 16),
        mirror_y=FlagOn(Unaligned(header.ImagDesc), 32);

   if(type<=0)if(map)type=IMAGE_R8G8B8_SRGB;else switch(Unaligned(header.PixelDepth))
   {
      case 15:
      case 16: type=(tga.mono ? IMAGE_I16 : IMAGE_R8G8B8_SRGB  ); break;
      case 24: type=(tga.mono ? IMAGE_I24 : IMAGE_R8G8B8_SRGB  ); break;
      case 32: type=(tga.mono ? IMAGE_I32 : IMAGE_R8G8B8A8_SRGB); break; // TGA uses BGRA order, we could use IMAGE_B8G8R8A8_SRGB, however it's an engine private type and can't be saved
      case 8 : type=            IMAGE_L8_SRGB                   ; break;
   }

   Bool convert=ImageTI[type].compressed;
   if(createTry(Unaligned(header.ImageWidth), Unaligned(header.ImageHeight), 1, convert ? IMAGE_B8G8R8A8_SRGB : IMAGE_TYPE(type), convert ? IMAGE_SOFT : IMAGE_MODE(mode), convert ? 1 : mip_maps)) // TGA uses BGRA order
   if(lock(LOCK_WRITE))
   {
      Byte rleLeftover=255;
      FREPD(y, T.h())
      {
         if(f.end())return false;
         Int dy=(mirror_y ? y : T.h()-y-1);
         if(compressed)rleLeftover=tga.  readCompressed(T, f, dy, rleLeftover);
         else                      tga.readUncompressed(T, f, dy, 0, T.w());
      }
      if(mirror_x)mirrorX();

      unlock();
      if(f.ok())
      {
         updateMipMaps();
         return copyTry(T, -1, -1, -1, type, mode, mip_maps);
      }
   }
   del(); return false;
}
/******************************************************************************/
Bool Image::ExportTGA(File &f)C
{
   Image  temp;
 C Image *src=this;
   if(src->cube      ())if(temp.fromCube(*src ,             IMAGE_B8G8R8A8_SRGB               ))src=&temp;else return false;
   if(src->compressed())if(src->copyTry ( temp, -1, -1, -1, IMAGE_B8G8R8A8_SRGB, IMAGE_SOFT, 1))src=&temp;else return false; // TGA uses BGRA

   Bool ok=false;
   if(src->lockRead())
   {
    C ImageTypeInfo &type_info=typeInfo(); // use 'T.type' to have precise information about source type
      Byte byte_pp=((type_info.channels==1) ? 1 : type_info.a ? 4 : 3);
      Bool ignore_gamma=IgnoreGamma(0, src->hwType(), IMAGE_B8G8R8A8_SRGB);

      TgaHeader header; Zero(header);
      Unaligned(header.ImageType  , (byte_pp<=1) ? TGA_Mono : TGA_RGB);
     _Unaligned(header.ImageWidth , src->w());
     _Unaligned(header.ImageHeight, src->h());
     _Unaligned(header.PixelDepth , byte_pp*8);
     _Unaligned(header.ImagDesc   , 32); // mirror_y
      f<<header;

      FREPD(y, src->h())switch(byte_pp)
      {
         case 1:
         {
	         if(src->bytePP()==1)f.put(src->data() + y*src->pitch(), src->w());else
	         FREPD(x, src->w())f.putByte(FltToByte(src->pixelF(x, y)));
         }break;

         case 3:
         {
	         if(src->hwType()==IMAGE_B8G8R8_SRGB || (src->hwType()==IMAGE_B8G8R8 && ignore_gamma))f.put(src->data() + y*src->pitch(), src->w()*3);else
	         if(ignore_gamma)FREPD(x, src->w()){Color c=src->color (x, y); Byte pixel[3]={c.b, c.g, c.r}; f<<pixel;}
	         else            FREPD(x, src->w()){Color c=src->colorS(x, y); Byte pixel[3]={c.b, c.g, c.r}; f<<pixel;}
         }break;

	      case 4:
	      {
	         if(src->hwType()==IMAGE_B8G8R8A8_SRGB || (src->hwType()==IMAGE_B8G8R8A8 && ignore_gamma))f.put(src->data() + y*src->pitch(), src->w()*4);else
	         if(ignore_gamma)FREPD(x, src->w()){Color c=src->color (x, y); Swap(c.r, c.b); f<<c;}
            else            FREPD(x, src->w()){Color c=src->colorS(x, y); Swap(c.r, c.b); f<<c;}
		   }break;
      }

      ok=f.ok();
      src->unlock();
   }
   return ok;
}
/******************************************************************************/
Bool Image::ImportTGA(  File &f   ) {return ImportTGA(f   , -1);}
Bool Image::ImportTGA(C Str  &name) {return ImportTGA(name, -1);}
Bool Image::ExportTGA(C Str  &name)C
{
   File f; if(f.writeTry(name)){if(ExportTGA(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool Image::ImportTGA(C Str &name, Int type, Int mode, Int mip_maps)
{
   File f; if(f.readTry(name))return ImportTGA(f, type, mode, mip_maps);
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
