/******************************************************************************/
#include "stdafx.h"
/******************************************************************************

   TODO: '_rws' could potentially by separated into '_rws_pak' and '_rws_downloaded', would it be better?

/******************************************************************************/
#define COMPARE  ComparePathCI
#define EQUAL   !COMPARE
#define TIME     Time.realTime() // use 'realTime' because sometimes it's mixed with 'curTime'
#define CC4_INCH CC4('I','N','C','H')
#define COPY_DOWNLOADED_MEM 1 // allows flush without waiting for import to finish, at the cost of extra memory copy
/******************************************************************************/
namespace EE{
/******************************************************************************
static Str EatWWW(Str url) // convert "http://www.esenthel.com" -> "http://esenthel.com"
{
   Bool http =          StartsPath(url, "http://" ) ; if(http )url.remove(0, 7);
   Bool https=(!http && StartsPath(url, "https://")); if(https)url.remove(0, 8);
   if(Starts(url, "www."))url.remove(0, 4);
   if(https)url.insert(0, "https://");
   if(http )url.insert(0, "http://" );
   return url;
}
/******************************************************************************/
Int InternetCache::LOD(C Image &image)
{
   return image.is() ? BitHi(CeilPow2(image.size3().max())) : -1;
}
/******************************************************************************/
void InternetCache::ImportImage::lockedRead() // this is called under 'ic._rws' write-lock only on the main thread
{
 //if(!done) not needed because PAK/DOWNLOADED are always converted to OTHER before 'done' (except case when 'fail' which is already checked below)
   switch(type) // this reads to 'T.temp'
   {
      case PAK: if(!fail)
      {
         File f; if(!f.read(*data.pak_file, *data.pak))
         {
         read_fail:
            fail=true;
          //done=true; cannot set this, we can modify this only on the import thread
            return;
         }
         temp.setNumDiscard(f.size());
         if(!f.getFast(temp.data(), temp.elms()))goto read_fail;
         data.set(temp.data(), temp.elms());
         type=OTHER; // !! ADJUST AT THE END ONCE EVERYTHING IS READY !! important for importer thread which first does fast check without locking
      }break;

      case DOWNLOADED: if(COPY_DOWNLOADED_MEM)
      {
         temp.setNumDiscard(data.memory_size);
         CopyFast(temp.data(), data.memory, temp.elms());
         data.set(temp.data(), temp.elms());
         type=OTHER; // !! ADJUST AT THE END ONCE EVERYTHING IS READY !! important for importer thread which first does fast check without locking
      }break;
   }
}
// !! IF GETTING '__chkstk' ERRORS HERE THEN IT MEANS STACK USAGE IS TOO HIGH, REDUCE 'temp' SIZE !!
inline void InternetCache::ImportImage::import(InternetCache &ic)
{
   Memt<Byte, 768*1024> temp; // 768 KB temp memory, stack is 1 MB, 840 KB still works for ZSTD compression (which has high stack usage)
   switch(type) // this reads to local stack 'temp'
   {
      case PAK: // PAK imports are always loaded to memory first, so when doing 'flush' we can update the PAK quickly without having to wait for entire import to finish, but only until the file data is read to memory
      {
         ReadLock lock(ic._rws);
         if(isPak()) // check again under lock
         {
            if(fail) // if failed to read on the main thread
               {done=true; return;} // !! DON'T DO ANYTHING AFTER THIS STEP BECAUSE 'this' OBJECT CAN GET REMOVED !!

            File f; if(!f.read(*data.pak_file, *data.pak))
            {
            read_fail:
               fail=true;
               done=true; return; // !! DON'T DO ANYTHING AFTER THIS STEP BECAUSE 'this' OBJECT CAN GET REMOVED !!
            }
            temp.setNumDiscard(f.size());
            if(!f.getFast(temp.data(), temp.elms()))goto read_fail;
            data.set(temp.data(), temp.elms());
            type=OTHER; // !! ADJUST AT THE END ONCE EVERYTHING IS READY !!
         }
      }break;

      case DOWNLOADED: if(COPY_DOWNLOADED_MEM)
      {
         ReadLock lock(ic._rws);
         if(isDownloaded()) // check again under lock
         {
            temp.setNumDiscard(data.memory_size);
            CopyFast(temp.data(), data.memory, temp.elms());
            data.set(temp.data(), temp.elms());
            type=OTHER; // !! ADJUST AT THE END ONCE EVERYTHING IS READY !!
         }
      }break;
   }

   DEBUG_ASSERT(data.type==DataSource::MEM, "data.type should be MEM"); // at this point 'data.type' is DataSource::MEM
   File f(data.memory, data.memory_size);
   ThreadMayUseGPUData(); // keep this covering even 'Import' in case the file is Engine 'Image' which can be IMAGE_2D
   image_temp.Import(f, -1, IMAGE_2D, ic._image_mip_maps);
   ThreadFinishedUsingGPUData();
   done=true; // !! DON'T DO ANYTHING AFTER THIS STEP BECAUSE 'this' OBJECT CAN GET REMOVED !!
}
NOINLINE static void ImportImageFunc(InternetCache::ImportImage &ii, InternetCache &ic, Int thread_index=0) {ii.import(ic);} // don't inline so we don't use stack memory in calling function
/******************************************************************************/
InternetCache::InternetCache() : _missing(COMPARE), _downloaded(COMPARE) {zero();}
/******************************************************************************/
struct SrcFile : PakFileData, InternetCache::FileTime
{
   UInt                       compressed_size;
   InternetCache::Downloaded *downloaded;

   SrcFile& set(C Str &name, InternetCache::Downloaded &src)
   {
      type=FSTD_FILE; compress_mode=(Compressable(GetExt(name)) ? COMPRESS_ENABLE : COMPRESS_DISABLE); decompressed_size=compressed_size=src.file_data.elms(); T.name=name; data.set(src.file_data.data(), src.file_data.elms()); modify_time_utc=src.modify_time_utc; access_time=src.access_time; verify_time=src.verify_time; downloaded=&src; return T;
   }
   SrcFile& set(C Str &name, C Pak &pak, C PakFile &pf, C InternetCache::FileTime &time)
   {
      type=pf.type(); compress_mode=COMPRESS_KEEP_ORIGINAL; compressed=pf.compression; decompressed_size=pf.data_size; compressed_size=pf.data_size_compressed; T.name=name; data.set(pf, pak); xxHash64_32=pf.data_xxHash64_32; modify_time_utc=pf.modify_time_utc; access_time=time.access_time; verify_time=time.verify_time; downloaded=null; return T;
   }
};

static Int CompareName          (C SrcFile &a, C SrcFile &b) {return COMPARE(a.name       , b.name       );}
static Int CompareName          (C SrcFile &a, C Str     &b) {return COMPARE(a.name       , b            );}
static Int CompareAccessTimeDesc(C SrcFile &a, C SrcFile &b) {return Compare(b.access_time, a.access_time);} // reverse order to list files with biggest access time first
static Int CompareAccessTimeAsc (InternetCache::FileTime *C &a, InternetCache::FileTime *C &b) {return Compare(a->access_time, b->access_time);}

#pragma pack(push, 4)
struct SavedFileTime
{
   Flt  access_time; // relative to current time, it will be -Inf..0
   Long verify_time; // absolute DateTime.seconds

   void zero() {access_time=-FLT_MAX; verify_time=0;}
   void set(C InternetCache::FileTime &ft, Dbl time, Long now)
   {
      access_time=       ft.access_time-time;
      verify_time=RoundL(ft.verify_time-time)+now;
   }
   void get(InternetCache::FileTime &ft, Dbl time, Long now)C
   {
      ft.access_time=Min(0, access_time    )     ; // prevent setting to the future in case of corrupt data
      ft.verify_time=Min(0, verify_time-now)+time; // prevent setting to the future in case of corrupt data
   }
   SavedFileTime() {}
   SavedFileTime(C InternetCache::FileTime &ft, Dbl time, Long now) {set(ft, time, now);}
};
#pragma pack(pop)

struct PostHeader : PakPostHeader
{
   Memc<SrcFile>  files;
   InternetCache &ic;

   PostHeader(InternetCache &ic) : ic(ic) {}

   virtual void save(File &f, C Pak &pak)override
   {
      f.putMulti(UInt(CC4_INCH), Byte(0)); // version
      Dbl time=Time.curTime(); Long now=DateTime().getUTC().seconds(); // calculate times at same moment

      // _pak_files
      FREPA(pak)
      {
         SavedFileTime sft; if(C SrcFile *file=files.binaryFind(pak.fullName(i), CompareName))sft.set(*file, time, now);else sft.zero();
         f<<sft;
      }

      // _missing
      f.cmpUIntV(ic._missing.elms());
      FREPA(ic._missing)
      {
       C Str                     &name     =ic._missing.key(i); f<<name;
       C InternetCache::FileTime &file_time=ic._missing    [i];
         f<<SavedFileTime(file_time, time, now);
      }

      // custom
      if(ic._save)ic._save(f);
   }
};

static void ICUpdate(InternetCache &ic);
void InternetCache::enable() {App.includeFuncCall(ICUpdate, T);}

void InternetCache::zero()
{
   got=null;
   base_to_url =null;
    url_to_base=null;
  _compress=COMPRESS_NONE;
  _compress_level=255;
  _image_mip_maps=0;
  _verify_life=60*60;
  _max_missing=256;
  _max_file_size=512<<20;
  _max_mem_size=16<<20;
  _threads=null;
  _pak_size=-1;
  _pak_modify_time_utc.zero();
  _save=_load=null;
}
void InternetCache::del()
{
   REPAO(_downloading).stop(); // do this first
   if(_threads)_threads->cancelFuncUser(ImportImageFunc, T); // cancel importing
   flush();
   if(_threads)_threads->waitFuncUser(ImportImageFunc, T);
  _missing             .del();
  _downloaded          .del();
  _import_images       .del();
  _to_download         .del();
  _to_verify           .del();
  _pak                 .del();
  _pak_used_file_ranges.del();
  _pak_files           .del();
   App._callbacks.exclude(ICUpdate, T);
   zero();
   REPAO(_downloading).del(); // at the end because it might need time to delete
}
void InternetCache::create(C Str &name, Threads *threads, Cipher *cipher, COMPRESS_TYPE compress, void (*save)(File &f), void (*load)(File &f))
{
   del();

   if(D.canUseGPUDataOnSecondaryThread())_threads=threads; // setup worker threads only if we can operate on GPU on secondary threads
  _compress      =compress;
//_image_mip_maps=image_mip_maps;
  _save=save;
  _load=load;
   if(name.is())
   {
      File f; if(f.readStd(name, cipher))
      {
         if(_pak.loadHeader(f, null, null, _pak_used_file_ranges)==PAK_LOAD_OK)
         {
           _pak.pakFileName(name);
            if(_pak._cipher_per_file)f.cipherOffsetClear(); // post-header is right after header
            Dbl time=Time.curTime(); Long now=DateTime().getUTC().seconds(); // calculate times at same moment
            if(f.getUInt()==CC4_INCH)switch(f.decUIntV()) // version
            {
               case 0:
               {
                 _pak_files.setNumDiscard(_pak.totalFiles()); FREPA(_pak_files) // _pak_files
                  {
                     FileTime &file_time=_pak_files[i];
                     SavedFileTime sft; f>>sft; sft.get(file_time, time, now);
                  }
                  REP(f.decUIntV()) // _missing
                  {
                     Str name; if(!name.load(f))goto error;
                     FileTime &file_time=*_missing(name);
                     SavedFileTime sft; f>>sft; sft.get(file_time, time, now);
                  }
                  if(f.ok())
                  {
                     if(load)load(f); // custom
                     getPakFileInfo();
                     return;
                  }
               }break;
            }
         error:
           _missing.del();
         }
      //_pak.del(); _pak_used_file_ranges.clear(); _pak_files.clear(); ignore because we recreate in 'resetPak'
      }
     _pak.pakFileName(name);
     _pak._file_cipher=cipher;
      resetPak();
   }
}
/******************************************************************************/
static void GetFileInfo(C Str &name, Long &size, DateTime &modify_time_utc)
{
   FileInfo fi; if(fi.getSystem(name) && fi.type==FSTD_FILE)
   {
      size           =fi.size;
      modify_time_utc=fi.modify_time_utc;
   }else
   {
      size           =-1;
      modify_time_utc.zero();
   }
}
void InternetCache::getPakFileInfo()
{
   GetFileInfo(_pak.pakFileName(), _pak_size, _pak_modify_time_utc);
}
void InternetCache::checkPakFileInfo()
{
            Long size;                DateTime modify_time_utc; GetFileInfo(_pak.pakFileName(), size, modify_time_utc);
   if(_pak_size!=size || _pak_modify_time_utc!=modify_time_utc)resetPak();
}
/******************************************************************************/
NOINLINE void InternetCache::cleanMissing() // don't inline so we don't use stack memory in calling function
{
   if(_max_missing>=0 && _missing.elms()>_max_missing)
   {
      Memt<FileTime*> sorted; sorted.setNumDiscard(_missing.elms()); REPAO(sorted)=&_missing[i]; sorted.sort(CompareAccessTimeAsc);
      REP(_missing.elms()-_max_missing)_missing.removeData(sorted[i]); // elements are sorted here from oldest (index 0) to newest (index last), so remove those at the start (oldest)
   }
}
/******************************************************************************/
Bool InternetCache::flush() {return flush(null, null);}
Bool InternetCache::flush(Downloaded *keep, Mems<Byte> *keep_data) // if 'keep' is going to get removed then its data will be placed in 'keep_data'
{
 //if(_downloaded.elms()) always save because we need to save 'access_time' and 'verify_time' which can change without new '_downloaded'
   {
      if(_pak.pakFileName().is()) // we want to save data
      {
         // we're going to update PAK so make sure all imports have read PAK FILE data
         // need to do this at the start, because in case of failure, 'resetPak' will recreate the PAK
         {
            WriteLockEx lock(_rws);
            Bool fail=false; REPA(_import_images){auto &ii=_import_images[i]; ii.lockedRead(); fail|=ii.fail;} // read all
            if(  fail){resetPak(&lock); goto reset;} // if any failed then 'resetPak'
         }
         checkPakFileInfo();
      reset:
         // at this point there should be no PAK importers (for COPY_DOWNLOADED_MEM also no DOWNLOADED importers)

         PostHeader post_header(T); auto &files=post_header.files;
         Long file_size=0;
         Bool keep_got_removed=false;
         FREPA(_downloaded)     {C Str &name=_downloaded.key(i); if(                           !_missing.find(name))file_size+=files.New().set(name, _downloaded[i]                   ).compressed_size;}
         FREPA(_pak       )if(i){  Str  name=_pak  .fullName(i); if(!_downloaded.find(name) && !_missing.find(name))file_size+=files.New().set(name, _pak, _pak.file(i), _pak_files[i]).compressed_size;} // skip post-header #PostHeaderFileIndex and files already included from '_downloaded'

         cleanMissing();

         if(_max_file_size>=0 && file_size>_max_file_size) // limit file size
         {
            files.sort(CompareAccessTimeDesc);
            do{SrcFile &file=files.last(); if(file.downloaded==keep)keep_got_removed=true; file_size-=file.compressed_size; files.removeLast();}while(file_size>_max_file_size && files.elms());
         }
         files.sort(CompareName); // needed for 'binaryFind' in 'SavePostHeader' and below

         if(((_max_file_size<0 || _pak_size/2<_max_file_size) // if pak size is smaller than 2x limit "_pak_size<_max_file_size*2", allow some tolerance because 'replaceInPlace' does not generate compact paks but they may have holes
           && _pak.replaceInPlace(_pak_used_file_ranges, SCAST(Memc<PakFileData>, files), 0, _compress, _compress_level, null, null, &post_header)) // replace in place
         ||   _pak.replace       (_pak_used_file_ranges, SCAST(Memc<PakFileData>, files), 0, _compress, _compress_level, null, null, &post_header)) // recreate
         {
            getPakFileInfo();
           _pak_files.setNumDiscard(_pak.totalFiles());
            REPA(_pak_files)
            {
                   FileTime &file=_pak_files[i];
               if(C SrcFile *src =files.binaryFind(_pak.fullName(i), CompareName))
               {
                  file.access_time=src->access_time;
                  file.verify_time=src->verify_time;
               }else file.zero();
            }

            // delete '_downloaded' as its data was moved to PAK
            if(!COPY_DOWNLOADED_MEM)
            {
               if(_threads) // wait for Import Threads processing Downloaded files which we're about to delete
               {
                  Memt<Threads::Call> calls; REPA(_import_images){auto &ii=_import_images[i]; if(!ii.done && ii.isDownloaded())calls.New().set(ii, ImportImageFunc, T);} _threads->wait(calls);
               }else
               {
                                             REPA(_import_images){auto &ii=_import_images[i]; if(!ii.done && ii.isDownloaded())ImportImageFunc(ii, T);} // don't use 'ii.import' because that one is inline
               }
            }
            if(keep_got_removed && keep)Swap(keep->file_data, *keep_data); // swap before deleting
           _downloaded.del();
         }else return false;
      }else
      {
         const Bool remove_missing=true; // this will remove all missing from downloaded, even if we still have free memory
         if(_downloaded.elms())
            if(remove_missing || _max_mem_size>=0)
         {
            Long max_size=((_max_mem_size>=0) ? _max_mem_size : LONG_MAX);
            Mems<SrcFile> files(_downloaded.elms()); // don't use 'Memt' because we need a lot of stack memory for 'ImportImageFunc'
            Long size=0; FREPA(files)size+=files[i].set(_downloaded.key(i), _downloaded[i]).compressed_size;
            if(remove_missing || size>max_size) // limit mem size
            {
               if(size>max_size)files.sort(CompareAccessTimeDesc);
               if(COPY_DOWNLOADED_MEM)
               {
                  WriteLock lock(_rws);
                  REPAO(_import_images).lockedRead();
                  // at this point there should be no DOWNLOADED importers
               }
               REPA(files)
               {
                  SrcFile &file=files[i];
                  if(size>max_size // exceeds mem limit
                  || remove_missing && _missing.find(file.name)) // or this file has gone missing
                  {  // remove this 'file'
                     Downloaded &downloaded=*file.downloaded;
                     if(!COPY_DOWNLOADED_MEM) // we're going to remove 'downloaded'
                        REPA(_import_images)
                     {
                        auto &ii=_import_images[i]; if(!ii.done && ii.isDownloaded() && ii.data.memory==downloaded.file_data.data()) // find all importers using its data
                        {
                           if(_threads)_threads->wait(ii, ImportImageFunc   , T); // wait for thread to finish
                           else                           ImportImageFunc(ii, T); // don't use 'ii.import' because that one is inline
                        }
                     }
                     if(&downloaded==keep)Swap(keep->file_data, *keep_data); // swap before deleting
                    _downloaded.removeData(&downloaded);
                     size-=file.compressed_size;
                  }else
                  if(!remove_missing)break; // if released enough memory, and we don't need to check missing then we can stop
               }
            }
         }
         cleanMissing();
      }
   }
   return true;
}
/******************************************************************************/
Bool InternetCache::loading(C ImagePtr &image)C
{
   if(image)
   {
      REPA(_import_images)if(_import_images[i].image_ptr==image)return true; // importing
      Str url=image.name(); if(url.is())
      {
         REPA(_downloading)if(EQUAL(_downloading[i].url(), url))return true;
         if(_to_verify  .binaryHas(url, COMPARE)
         || _to_download.binaryHas(url, COMPARE))return true;
      }
   }
   return false;
}
/******************************************************************************/
Bool               InternetCache::getFile  (C Str &url, DataSource &file, CACHE_VERIFY verify) {return getFileEx(url, file, verify)==FILE;}
InternetCache::GET InternetCache::getFileEx(C Str &url, DataSource &file, CACHE_VERIFY verify, Bool access_download)
{
   file.set();
   if(!url.is())return FILE;
   Str name=SkipHttpWww(url); if(!name.is())return NONE;

   // check if it's known to be missing
   if(FileTime *missing=_missing.find(name))
   {
      if(access_download)missing->access_time=TIME;
      if(TIME-missing->verify_time<=_verify_life)return NONE; // verification still acceptable
      verify=CACHE_VERIFY_EXPIRED; // verification expired, however last known state is missing, so try to verify/download, but prevent from returning FILE
   }

   // find in cache
   Flt *verify_time;
   if(Downloaded *down=_downloaded.find(name))
   {
      if(access_download)down->access_time=TIME;
      verify_time=&down->verify_time;
      file.set(down->file_data.data(), down->file_data.elms());
   }else
   if(C PakFile *pf=_pak.find(name, false))
   {
      FileTime &time=pakFile(*pf);
      if(access_download)time.access_time=TIME;
      verify_time=&time.verify_time;
      file.set(*pf, _pak);
   }else // not found
   {
      if(access_download) // download
      {
         REPA(_downloading)if(EQUAL(_downloading[i].url(),  url         ))goto downloading;
                           if(   _to_download.binaryInclude(url, COMPARE)){enable(); _to_verify.binaryExclude(url, COMPARE);}
      downloading:
         return DOWNLOADING;
      }
      return NONE;
   }

   // found
   {
      if(verify==CACHE_VERIFY_SKIP                                      )return FILE; // verification not   needed
      if(verify!=CACHE_VERIFY_EXPIRED && TIME-*verify_time<=_verify_life)return FILE; // verification still acceptable
      if(access_download) // verify
      {
         REPA(_downloading)if(EQUAL       (_downloading[i].url(), url))goto verifying; // downloading now
                           if(_to_download.binaryHas    (url, COMPARE))goto verifying; // will download soon
                           if(_to_verify  .binaryInclude(url, COMPARE))enable();       // verify
      verifying:;
         return (verify==CACHE_VERIFY_DELAY) ? FILE : DOWNLOADING;
      }
      return (verify==CACHE_VERIFY_DELAY) ? FILE : NONE;
   }
}
ImagePtr InternetCache::getImage(C Str &url, CACHE_VERIFY verify)
{
   ImagePtr img; if(url.is())
   {
      DataSource file; Bool get_file=getFile(url, file, verify); // always call 'getFile' to adjust 'access_time' and request verification if needed
      if(!img.find(url))
      {
         CACHE_MODE mode=Images.mode(CACHE_DUMMY); img=url;
                         Images.mode(mode       );
         if(get_file)
         {
            ImportImage &ii=_import_images.New();
            Swap(ii.data, file); ii.type=(ii.data.type==DataSource::PAK_FILE ? ImportImage::PAK : ImportImage::DOWNLOADED);
            ii.image_ptr=img;
            import(ii);
            enable();
         }
      }
   }
   return img;
}
/******************************************************************************/
ImagePtr InternetCache::getImageLOD(C Str &base, Int lod, CACHE_VERIFY verify)
{
   ImagePtr img; if(base.is())
   {
      CACHE_MODE mode=Images.mode(CACHE_DUMMY); img=base;
                      Images.mode(mode       );

      Lod lods; if(base_to_url && url_to_base && url_to_base(base_to_url(base, lod), lods).is() && lod==lods.lod)
      {
         Clamp(lod, lods.min, lods.max);

         DataSource data;
         GET get=NONE;
         Int file_lod;
         // start download
         for(file_lod=lod;   file_lod<=lods.max; file_lod++)if(get=getFileEx(base_to_url(base, file_lod), data, verify))goto got;else break; // if FILE or DOWNLOADING (if NONE then break and stop looking)
         for(file_lod=lod; --file_lod>=lods.min;           )if(get=getFileEx(base_to_url(base, file_lod), data, verify))goto got;            // if FILE or DOWNLOADING
      got:
         // get any preview
         if(get!=FILE) // if haven't found any file
         {
            for(file_lod=lod;   file_lod<=lods.max; file_lod++)if(getFileEx(base_to_url(base, file_lod), data, verify, false))goto got_file; // if FILE
            for(file_lod=lod; --file_lod>=lods.min;           )if(getFileEx(base_to_url(base, file_lod), data, verify, false))goto got_file; // if FILE
         }else // got FILE
      got_file:
         if(file_lod>LOD(*img)) // import only if we might improve quality
         {
            REPA(_import_images)
            {
               ImportImage &ii=_import_images[i]; if(ii.image_ptr==img) // find import
               {
                  if(ii.lod<file_lod){cancel(ii); break;} // importing lower quality
                  goto importing; // importing same or better quality, no need to import anything
               }
            }
            // import
            ImportImage &ii=_import_images.New();
            Swap(ii.data, data); ii.type=(ii.data.type==DataSource::PAK_FILE ? ImportImage::PAK : ImportImage::DOWNLOADED);
            ii.image_ptr=img;
            ii.lod=file_lod;
            import(ii);
            enable();
         importing:;
         }
      }
   }
   return img;
}
/******************************************************************************/
void InternetCache::changed(C Str &url)
{
   if(url.is())
   {
      Str name=SkipHttpWww(url); if(name.is())
      {
         if(FileTime   *miss=_missing   .find(name      ))miss       ->verify_time=INT_MIN;
         if(Downloaded *down=_downloaded.find(name      ))down       ->verify_time=INT_MIN;
         if(C PakFile  *pf  =_pak       .find(name, true))pakFile(*pf).verify_time=INT_MIN;
         REPA(_downloading)
         {
            Download &down=_downloading[i]; if(EQUAL(down.url(), url))
            { // restart the download
            #if 1
               down.create(url);
            #else
               down.del(); _to_download.binaryInclude(url, COMPARE);
            #endif
               return;
            }
         }
         if(_to_verify  .binaryHas(url, COMPARE))return; // it will be checked
         if(_to_download.binaryHas(url, COMPARE))return; // it will be downloaded
         if(Images.has(url)) // download if currently referenced
         {
           _to_download.binaryInclude(url, COMPARE); enable();
         }
      }
   }
}
/******************************************************************************/
Bool InternetCache::busy()C
{
   if(_to_download.elms() || _to_verify.elms() || _import_images.elms())return true;
   REPA(_downloading)if(_downloading[i].state()!=DWNL_NONE)return true;
   return false;
}
void InternetCache::import(ImportImage &ii)
{
   if(_threads)_threads->queue(ii, ImportImageFunc, T);else ImportImageFunc(ii, T); // don't use 'ii.import' because that one is inline
}
void InternetCache::cancel(ImportImage &ii) // canceling is needed to make sure we won't replace newer data with older
{
   if(ii.done                                    // finished
   || !_threads                                  // no threads
   ||  _threads->cancel(ii, ImportImageFunc, T)) // canceled
       _import_images.removeData(&ii); // just remove
   else ii.image_ptr=null; // now processing, clear 'image_ptr' so we will ignore it
}
void InternetCache::cancel(C ImagePtr &image) // canceling is needed to make sure we won't replace newer data with older
{
   REPA(_import_images)
   {
      ImportImage &ii=_import_images[i]; if(ii.image_ptr==image)
      {
         cancel(ii);
         break; // can break because there can be only one importer for an image
      }
   }
}
void InternetCache::updating(Ptr data) // called when updating 'downloaded'
{
   if(data)
   REPA(_import_images)
   {
      ImportImage &ii=_import_images[i]; if(ii.isDownloaded() && ii.data.memory==data)
      {
         if(ii.done                                    // finished
         || !_threads                                  // no threads
         ||  _threads->cancel(ii, ImportImageFunc, T)) // canceled
     remove: _import_images.removeValid(i); // just remove
         else // now processing
         {
            if(COPY_DOWNLOADED_MEM)
            {
               {
                  WriteLock lock(_rws);
                  if(ii.isDownloaded()) // check again under lock
                  {
                     ii.data.set(null, 0);
                     ii.type=ImportImage::OTHER; // !! ADJUST AT THE END ONCE EVERYTHING IS READY !! important for importer thread which first does fast check without locking
                  }
               }
               ii.image_ptr=null; // clear 'image_ptr' so we will ignore it
            }else
            {
              _threads->wait(ii, ImportImageFunc, T); // wait for finish
               goto remove;
            }
         }
      }
   }
}
void InternetCache::resetPak(WriteLockEx *lock)
{
   // we're going to recreate the PAK file, as old one is considered invalid/missing/modified
   Memt<Threads::Call> calls;
   {
      WriteLock lock(_rws); // stop any further reads, this stops any conversions from 'isPak' until we release the lock, this is important as we need all isPak/fail to be included in 'calls' below, so we can cancel/wait for threads to finish, as they're going to be removed
      REPA(_import_images){auto &ii=_import_images[i]; if(ii.isPak())ii.fail=true; if(ii.fail)calls.New().set(ii, ImportImageFunc, T);} // force all PAK as fail, we assume that PAK is compromised, this will also force PAK importer to stop importing and return quickly
      if(_threads)_threads->cancel(calls);
   }
   if( lock   ) lock   ->off (); // unlock first
   if(_threads)_threads->wait(calls); // wait for all failed importers to return, have to wait with lock disabled

   Memt<Str> retry;
   REPA(_import_images)
   {
      auto &ii=_import_images[i]; if(ii.fail)
      {
         if(ii.image_ptr)retry.add(ii.image_ptr.name()); // not canceled, retry
        _import_images.removeValid(i);
      }
   }

   PostHeader post_header(T);
  _pak.create(CMemPtr<PakFileData>(), _pak.pakFileName(), 0, _pak._file_cipher, COMPRESS_NONE, 0, null, null, null, _pak_used_file_ranges, &post_header); // create an empty pak
  _pak_files.setNumDiscard(_pak.totalFiles()); REPAO(_pak_files).zero();
   getPakFileInfo();

   if(retry.elms())
   {
      Bool enable=false;
      REPA(retry)
      {
       C Str &url=retry[i], name=SkipHttpWww(url);
         // this file was from Pak that failed to load, and it wasn't canceled, it means it's not available locally anymore, try to download
         // no need to check 'missing' because once it's detected then all imports for that image are canceled
         {
            REPA(_downloading)if(EQUAL(_downloading[i].url(),  url         ))goto downloading;
                              if(   _to_download.binaryInclude(url, COMPARE)){enable=true; _to_verify.binaryExclude(url, COMPARE);}
         downloading:;
         }
      }
      if(enable)T.enable();
   }
}
inline void InternetCache::update()
{
   // update imported images
   REPA(_import_images)
   {
      ImportImage &ii=_import_images[i]; if(ii.done)
      {
         if(ii.fail){resetPak(); break;} // if failed to open file, then we have to reset, break because 'resetPak' will handle '_import_images'
         if(ii.image_ptr) // not canceled
         {
            Swap(*ii.image_ptr, ii.image_temp);
            if(got)got(ii.image_ptr);
         }
        _import_images.removeValid(i);
      }
   }

   // process downloaded data
   REPA(_downloading)
   {
      Download &down=_downloading[i]; switch(down.state())
      {
         case DWNL_NONE:
         {
         again:
            if(_to_download.elms()){down.create(_to_download.last()                   ); _to_download.removeLast();}else
            if(_to_verify  .elms()){down.create(_to_verify  .last(), null, null, -1, 0); _to_verify  .removeLast();} // use offset as -1 to encode special mode of verification
         }break;

         case DWNL_DONE: // finished downloading
         {
            Str name=SkipHttpWww(down.url());
           _missing.removeKey(name);
            if(down.offset()<0) // if this was verification
            {
               Flt        *verify_time=null;
               Downloaded * downloaded=_downloaded.find(name);
             C PakFile    *         pf=null;
               if(downloaded               ){if(downloaded->file_data.elms()==down.totalSize() && downloaded->modify_time_utc==down.modifyTimeUTC()){verify_time=& downloaded->verify_time;}}else
               if(pf=_pak.find(name, false)){if(        pf->data_size       ==down.totalSize() &&         pf->modify_time_utc==down.modifyTimeUTC()){verify_time=&pakFile(*pf).verify_time;}}

               if(verify_time)
               {
                 *verify_time=TIME;
                  // it's possible the image was not yet loaded due to CACHE_VERIFY_YES
                  ImagePtr img; if(img.find(down.url()))if(!img->is()) // if image empty
                  {
                     REPA(_import_images)if(_import_images[i].image_ptr==img)goto importing; // first check if it's importing already, but just not yet finished
                     // if not yet importing, then import
                     ImportImage &ii=_import_images.New();
                     if(downloaded){ii.data.set(downloaded->file_data.data(), downloaded->file_data.elms()); ii.type=ImportImage::DOWNLOADED;}
                     else          {ii.data.set(*pf, _pak);                                                  ii.type=ImportImage::PAK       ;}
                     Swap(ii.image_ptr, img);
                     import(ii);
                  importing:;
                  }
                  Lod lod; if(url_to_base && img.find(url_to_base(down.url(), lod)))
                  {
                  }
                  goto next;
               }
               down.create(Str(down.url())); // it's different so download it fully, copy the 'url' because it might get deleted in 'create'
            }else // move to 'downloaded'
            {
               Bool just_created;
               Downloaded *downloaded=_downloaded(name, just_created);
               updating(downloaded->file_data.data()); // we're going to modify 'downloaded->file_data' so we have to make sure no importers are using that data
               downloaded->file_data.setNumDiscard(down.done()).copyFrom((Byte*)down.data());
               downloaded->modify_time_utc=down.modifyTimeUTC();
               downloaded->verify_time=TIME;
               if(just_created)
               {
                  // set 'access_time'
                  if(C PakFile *pf=_pak.find(name, true))downloaded->access_time=pakFile(*pf).access_time;else // reuse from 'pak'
                                                         downloaded->access_time=downloaded ->verify_time;     // set as new
               }

               ImagePtr img; img.find(down.url());
               ImagePtr img_lod; Lod lod; if(url_to_base)
               {
                  Str base=url_to_base(down.url(), lod); if(img_lod.find(base))
                  {
                     Int lod_img=LOD(*img_lod);
                     if( lod.lod>=lod_img) // always import if just got the file and quality is same (most likely we got newer data) or higher
                     {
                     test_import_img_lod:;
                        check importers
                        cancel
                        goto import_img_lod;
                     }else // if got lower quality, then import only if we have received newer data
                     {
                      C DateTime *modify_time=null;
                        if(base_to_url)
                        {
                           Str name=SkipHttpWww(base_to_url(base, lod_img)); // name of data that's already loaded in the image
                           if(C Downloaded *down=_downloaded.find(name       ))modify_time=&down->modify_time_utc;else
                           if(C PakFile    *pf  =_pak       .find(name, false))modify_time=&pf  ->modify_time_utc;
                        }
                        if(!modify_time // haven't found existing data
                        ||  downloaded->modify_time_utc>*modify_time // or now received newer data
                        )goto test_import_img_lod;
                     }
                     img_lod.clear(); // don't import
                  import_img_lod:;
                  }
               }
               const Bool import=(img || img_lod);
               Mems<Byte> downloaded_data;

               if(_max_mem_size>=0)
               {
                  Long mem_size=0; REPA(_downloaded)mem_size+=_downloaded[i].file_data.elms();
                  if(  mem_size>_max_mem_size)
                  {
                     if(import)flush(downloaded, &downloaded_data); // if we're going to import, then make sure if 'downloaded' is going to get removed, then we will keep its memory
                     else      flush();
                     downloaded=null; // flush deletes '_downloaded' so clear this one, we'll find it again
                  }
               }

               if(import)
               {
                C PakFile *pf=null;
                  if(!downloaded)
                  {
                                    downloaded=_downloaded.find(name);
                     if(!downloaded)pf        =_pak       .find(name, false);
                  }
                //if(downloaded || pf) ignore because if it got removed and not found, then data was put in 'downloaded_data'
                  {
                     if(img)
                     {
                        cancel(img);
                        ImportImage &ii=_import_images.New();
                        if(downloaded){                                                                        ii.data.set(downloaded->file_data.data(), downloaded->file_data.elms()); ii.type=ImportImage::DOWNLOADED;}else
                        if(pf        ){                                                                        ii.data.set(*pf, _pak);                                                  ii.type=ImportImage::PAK       ;}else
                                      {if(img_lod)ii.temp=downloaded_data;else Swap(ii.temp, downloaded_data); ii.data.set(ii.temp.data(), ii.temp.elms());                             ii.type=ImportImage::OTHER     ;}
                        Swap(ii.image_ptr, img);
                        T.import(ii);
                     }
                     if(img_lod)
                     {
                        /*
                        ImportImage &ii=_import_images.New();
                        if(downloaded){                                ii.data.set(downloaded->file_data.data(), downloaded->file_data.elms()); ii.type=ImportImage::DOWNLOADED;}else
                        if(pf        ){                                ii.data.set(*pf, _pak);                                                  ii.type=ImportImage::PAK       ;}else
                                      {Swap(ii.temp, downloaded_data); ii.data.set(ii.temp.data(), ii.temp.elms());                             ii.type=ImportImage::OTHER     ;}
                        ii.lod=lod.lod;
                        Swap(ii.image_ptr, img);
                        T.import(ii);*/
                     }
                  }
               }
               goto next;
            }
         }break;

         case DWNL_ERROR: // failed
         {
            if(down.code()==404) // confirmed that file is missing
            {
               Str name=SkipHttpWww(down.url());
               Bool just_created;
               FileTime &missing=*_missing(name, just_created);
               missing.verify_time=TIME;
               if(just_created)
               {
                  // set 'access_time'
                  if(C Downloaded *downloaded=_downloaded.find(name      ))missing.access_time=downloaded ->access_time;else // reuse from 'downloaded'
                  if(C PakFile    *pf        =_pak       .find(name, true))missing.access_time=pakFile(*pf).access_time;else // reuse from 'pak'
                                                                           missing.access_time=missing     .verify_time;     // set as new
                  ImagePtr img; if(img.find(down.url())) // delete image
                  {
                     cancel(img);
                     if(img->is())
                     {
                        img->del();
                        if(got)got(img); // notify too because image got modified
                     }
                  }
               }
            }
            next: down.del(); goto again;
         }break;
      }
   }

   if(busy())enable();
}
static void ICUpdate(InternetCache &ic) {ic.update();}
/******************************************************************************/
}
/******************************************************************************/
