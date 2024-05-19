/******************************************************************************/
#include "stdafx.h"
/******************************************************************************

   TODO: '_rws' could potentially by separated into '_rws_pak' and '_rws_downloaded', would it be better?

   Here 'link' means url without "https://www."

   All imports are considered verified (not expired),
      on 'changed' all imports are considered expired so they just get deleted.

/******************************************************************************/
#define LOG (DEBUG && 0)
#define COMPARE ComparePathCI
#define EQUAL !COMPARE
#define TIME Time.realTime() // use 'realTime' because sometimes it's mixed with 'curTime'
#define CC4_INCH CC4('I', 'N', 'C', 'H')
#define COPY_DOWNLOADED_MEM 1 // allows flush without waiting for import to finish, at the cost of extra memory copy
#if LOG
#pragma message("!! Warning: Use only for testing !!")
#endif
/******************************************************************************/
namespace EE {
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
Int InternetCache::LOD(C Image &image) {
    return image.is() ? BitHi(CeilPow2(image.size3().max())) : -1;
}
/******************************************************************************/
Str InternetCache::ImportImage::asText() C {
    Str s = image_ptr.name();
    if (lod >= 0)
        s.space() += S + "Lod:" + lod;
    return s;
}
void InternetCache::ImportImage::lockedRead() // this is called under 'ic._rws' write-lock only on the main thread
{
    // if(!done) not needed because PAK/DOWNLOADED are always converted to OTHER before 'done' (except case when 'fail' which is already checked below)
    switch (type) // this reads to 'T.temp'
    {
    case PAK:
        if (!fail) {
            File f;
            if (!f.read(*data.pak_file, *data.pak)) {
            read_fail:
                fail = true;
                // done=true; cannot set this, we can modify this only on the import thread
                return;
            }
            temp.setNumDiscard(f.size());
            if (!f.getFast(temp.data(), temp.elms()))
                goto read_fail;
            data.set(temp.data(), temp.elms());
            type = OTHER; // !! ADJUST AT THE END ONCE EVERYTHING IS READY !! important for import thread which first does fast check without locking
        }
        break;

    case DOWNLOADED:
        if (COPY_DOWNLOADED_MEM) {
            temp.setNumDiscard(data.memory_size);
            CopyFast(temp.data(), data.memory, temp.elms());
            data.set(temp.data(), temp.elms());
            type = OTHER; // !! ADJUST AT THE END ONCE EVERYTHING IS READY !! important for import thread which first does fast check without locking
        }
        break;
    }
}
// !! IF GETTING '__chkstk' ERRORS HERE THEN IT MEANS STACK USAGE IS TOO HIGH, REDUCE 'temp' SIZE !!
inline void InternetCache::ImportImage::import(InternetCache &ic) {
    Memt<Byte, 768 * 1024> temp; // 768 KB temp memory, stack is 1 MB, 840 KB still works for ZSTD compression (which has high stack usage)
    switch (type)                // this reads to local stack 'temp'
    {
    case PAK: // PAK imports are always loaded to memory first, so when doing 'flush' we can update the PAK quickly without having to wait for entire import to finish, but only until the file data is read to memory
    {
        ReadLock lock(ic._rws);
        if (isPak()) // check again under lock
        {
            if (fail) // if failed to read on the main thread
            {
                done = true;
                return;
            } // !! DON'T DO ANYTHING AFTER THIS STEP BECAUSE 'this' OBJECT CAN GET REMOVED !!

            File f;
            if (!f.read(*data.pak_file, *data.pak)) {
            read_fail:
                fail = true;
                done = true;
                return; // !! DON'T DO ANYTHING AFTER THIS STEP BECAUSE 'this' OBJECT CAN GET REMOVED !!
            }
            temp.setNumDiscard(f.size());
            if (!f.getFast(temp.data(), temp.elms()))
                goto read_fail;
            data.set(temp.data(), temp.elms());
            type = OTHER; // !! ADJUST AT THE END ONCE EVERYTHING IS READY !!
        }
    } break;

    case DOWNLOADED:
        if (COPY_DOWNLOADED_MEM) {
            ReadLock lock(ic._rws);
            if (isDownloaded()) // check again under lock
            {
                temp.setNumDiscard(data.memory_size);
                CopyFast(temp.data(), data.memory, temp.elms());
                data.set(temp.data(), temp.elms());
                type = OTHER; // !! ADJUST AT THE END ONCE EVERYTHING IS READY !!
            }
        }
        break;
    }

    DEBUG_ASSERT(data.type == DataSource::MEM, "data.type should be MEM"); // at this point 'data.type' is DataSource::MEM
    File f(data.memory, data.memory_size);
    ThreadMayUseGPUData(); // keep this covering even 'Import' in case the file is Engine 'Image' which can be IMAGE_2D
    image_temp.Import(f, -1, IMAGE_2D, ic._image_mip_maps);
    ThreadFinishedUsingGPUData();
    done = true; // !! DON'T DO ANYTHING AFTER THIS STEP BECAUSE 'this' OBJECT CAN GET REMOVED !!
}
NOINLINE static void ImportImageFunc(InternetCache::ImportImage &ii, InternetCache &ic, Int thread_index = 0) { ii.import(ic); } // don't inline so we don't use stack memory in calling function
/******************************************************************************/
InternetCache::InternetCache() : _missing(COMPARE), _downloaded(COMPARE) { zero(); }
/******************************************************************************/
struct SrcFile : PakFileData, InternetCache::FileTime {
    UInt compressed_size;
    InternetCache::Downloaded *downloaded;

    SrcFile &set(C Str &name, InternetCache::Downloaded &src) {
        type = FSTD_FILE;
        compress_mode = (Compressable(GetExt(name)) ? COMPRESS_ENABLE : COMPRESS_DISABLE);
        decompressed_size = compressed_size = src.file_data.elms();
        T.name = name;
        data.set(src.file_data.data(), src.file_data.elms());
        modify_time_utc = src.modify_time_utc;
        access_time = src.access_time;
        verify_time = src.verify_time;
        downloaded = &src;
        return T;
    }
    SrcFile &set(C Str &name, C Pak &pak, C PakFile &pf, C InternetCache::FileTime &time) {
        type = pf.type();
        compress_mode = COMPRESS_KEEP_ORIGINAL;
        compressed = pf.compression;
        decompressed_size = pf.data_size;
        compressed_size = pf.data_size_compressed;
        T.name = name;
        data.set(pf, pak);
        xxHash64_32 = pf.data_xxHash64_32;
        modify_time_utc = pf.modify_time_utc;
        access_time = time.access_time;
        verify_time = time.verify_time;
        downloaded = null;
        return T;
    }
};

static Int CompareName(C SrcFile &a, C SrcFile &b) { return COMPARE(a.name, b.name); }
static Int CompareName(C SrcFile &a, C Str &b) { return COMPARE(a.name, b); }
static Int CompareAccessTimeDesc(C SrcFile &a, C SrcFile &b) { return Compare(b.access_time, a.access_time); } // reverse order to list files with biggest access time first
static Int CompareAccessTimeAsc(InternetCache::FileTime *C &a, InternetCache::FileTime *C &b) { return Compare(a->access_time, b->access_time); }

#pragma pack(push, 4)
struct SavedFileTime {
    Flt access_time;  // relative to current time, it will be -Inf..0
    Long verify_time; // absolute DateTime.seconds

    void zero() {
        access_time = -FLT_MAX;
        verify_time = 0;
    }
    void set(C InternetCache::FileTime &ft, Dbl time, Long now) {
        access_time = ft.access_time - time;
        verify_time = RoundL(ft.verify_time - time) + now;
    }
    void get(InternetCache::FileTime &ft, Dbl time, Long now) C {
        ft.access_time = Min(0, access_time);              // prevent setting to the future in case of corrupt data
        ft.verify_time = Min(0, verify_time - now) + time; // prevent setting to the future in case of corrupt data
    }
    SavedFileTime() {}
    SavedFileTime(C InternetCache::FileTime &ft, Dbl time, Long now) { set(ft, time, now); }
};
#pragma pack(pop)

struct PostHeader : PakPostHeader {
    Memc<SrcFile> files;
    InternetCache &ic;

    PostHeader(InternetCache &ic) : ic(ic) {}

    virtual void save(File &f, C Pak &pak) override {
        f.putMulti(UInt(CC4_INCH), Byte(0)); // version
        Dbl time = Time.curTime();
        Long now = DateTime().getUTC().seconds(); // calculate times at same moment

        if (LOG)
            LogN(S + "IC.save: " + pak.totalFiles() + " PakFiles, " + ic._missing.elms() + " Missing");

        // _pak_files
        FREPA(pak) {
            SavedFileTime sft;
            if (C SrcFile *file = files.binaryFind(pak.fullName(i), CompareName))
                sft.set(*file, time, now);
            else
                sft.zero();
            f << sft;
        }

        // _missing
        f.cmpUIntV(ic._missing.elms());
        FREPA(ic._missing) {
            C Str &link = ic._missing.key(i);
            f << link;
            C InternetCache::FileTime &file_time = ic._missing[i];
            f << SavedFileTime(file_time, time, now);
        }

        // custom
        if (ic._save)
            ic._save(f);
    }
};

static void ICUpdate(InternetCache &ic);
void InternetCache::enable() { App.includeFuncCall(ICUpdate, T); }

void InternetCache::zero() {
    got = null;
    image_lod_to_url = null;
    url_to_image_lod = null;
    is_image_lod = null;
    const_image_lod = null;
    _compress = COMPRESS_NONE;
    _compress_level = 255;
    _image_mip_maps = 0;
    _verify_life = 60 * 60;
    _max_missing = 256;
    _max_file_size = 512 << 20;
    _max_mem_size = 16 << 20;
    _threads = null;
    _pak_size = -1;
    _pak_modify_time_utc.zero();
    _save = _load = null;
}
void InternetCache::del() {
    REPAO(_downloading).stop(); // do this first
    if (_threads)
        _threads->cancelFuncUser(ImportImageFunc, T); // cancel importing
    flush();
    if (_threads)
        _threads->waitFuncUser(ImportImageFunc, T);
    _missing.del();
    _downloaded.del();
    _import_images.del();
    _to_download.del();
    _to_verify.del();
    _pak.del();
    _pak_used_file_ranges.del();
    _pak_files.del();
    App._callbacks.exclude(ICUpdate, T);
    zero();
    REPAO(_downloading).del(); // at the end because it might need time to delete
}
void InternetCache::create(C Str &name, Threads *threads, Cipher *cipher, COMPRESS_TYPE compress, void (*save)(File &f), void (*load)(File &f)) {
    del();

    if (D.canUseGPUDataOnSecondaryThread())
        _threads = threads; // setup worker threads only if we can operate on GPU on secondary threads
    _compress = compress;
    //_image_mip_maps=image_mip_maps;
    _save = save;
    _load = load;
    if (name.is()) {
        File f;
        if (f.readStd(name, cipher)) {
            if (_pak.loadHeader(f, null, null, _pak_used_file_ranges) == PAK_LOAD_OK) {
                _pak.pakFileName(name);
                if (_pak._cipher_per_file)
                    f.cipherOffsetClear(); // post-header is right after header
                Dbl time = Time.curTime();
                Long now = DateTime().getUTC().seconds(); // calculate times at same moment
                if (f.getUInt() == CC4_INCH)
                    switch (f.decUIntV()) // version
                    {
                    case 0: {
                        _pak_files.setNumDiscard(_pak.totalFiles());
                        FREPA(_pak_files) // _pak_files
                        {
                            FileTime &file_time = _pak_files[i];
                            SavedFileTime sft;
                            f >> sft;
                            sft.get(file_time, time, now);
                        }
                        REP(f.decUIntV()) // _missing
                        {
                            Str link;
                            if (!link.load(f))
                                goto error;
                            FileTime &file_time = *_missing(link);
                            SavedFileTime sft;
                            f >> sft;
                            sft.get(file_time, time, now);
                        }
                        if (f.ok()) {
                            if (load)
                                load(f); // custom
                            getPakFileInfo();
                            return;
                        }
                    } break;
                    }
            error:
                _missing.del();
            }
            //_pak.del(); _pak_used_file_ranges.clear(); _pak_files.clear(); ignore because we recreate in 'resetPak'
            if (LOG)
                LogN(S + "IC.load failed");
        }
        _pak.pakFileName(name);
        _pak._file_cipher = cipher;
        resetPak();
    }
}
/******************************************************************************/
static void GetFileInfo(C Str &name, Long &size, DateTime &modify_time_utc) {
    FileInfo fi;
    if (fi.getSystem(name) && fi.type == FSTD_FILE) {
        size = fi.size;
        modify_time_utc = fi.modify_time_utc;
    } else {
        size = -1;
        modify_time_utc.zero();
    }
}
void InternetCache::getPakFileInfo() {
    GetFileInfo(fileName(), _pak_size, _pak_modify_time_utc);
}
void InternetCache::checkPakFileInfo() {
    Long size;
    DateTime modify_time_utc;
    GetFileInfo(fileName(), size, modify_time_utc);
    if (_pak_size != size || _pak_modify_time_utc != modify_time_utc) {
        if (LOG)
            LogN(S + "IC.checkPakFileInfo FAILED");
        resetPak();
    }
}
/******************************************************************************/
NOINLINE void InternetCache::cleanMissing() // don't inline so we don't use stack memory in calling function
{                                           // this is called when all downloaded/pak files that have gone missing have been removed already
    if (_max_missing >= 0 && _missing.elms() > _max_missing) {
        Memt<FileTime *> sorted;
        sorted.setNumDiscard(_missing.elms());
        REPAO(sorted) = &_missing[i];
        sorted.sort(CompareAccessTimeAsc);
        REP(_missing.elms() - _max_missing)
        _missing.removeData(sorted[i]); // elements are sorted here from oldest (index 0) to newest (index last), so remove those at the start (oldest)
    }
}
/******************************************************************************/
void InternetCache::delFile() {
    Dbl time;
    if (LOG) {
        time = Time.curTime();
        LogN(S + "IC.delFile");
    }
    if (fileName().is()) // we have file
    {
        WriteLockEx lock(_rws);
        REPA(_import_images) {
            auto &ii = _import_images[i];
            ii.lockedRead();
        }                // read all
        resetPak(&lock); // 'resetPak', reset while still under lock, it will be unlocked inside
    }
    if (LOG)
        LogN(S + "IC.delFile Finish " + Flt(Time.curTime() - time));
}
/******************************************************************************/
Bool InternetCache::flush() { return flush(null, null); }
Bool InternetCache::flush(Downloaded *keep, Mems<Byte> *keep_data) // if 'keep' is going to get removed then its data will be placed in 'keep_data'
{
    Dbl time;
    if (LOG) {
        time = Time.curTime();
        LogN(S + "IC.flush");
    }

    // if(_downloaded.elms()) always save because we need to save 'access_time' and 'verify_time' which can change without new '_downloaded'
    {
        if (fileName().is()) // we want to save data
        {
            // we're going to update PAK so make sure all imports have read PAK FILE data
            // need to do this at the start, because in case of failure, 'resetPak' will recreate the PAK
            {
                WriteLockEx lock(_rws);
                Bool fail = false;
                REPA(_import_images) {
                    auto &ii = _import_images[i];
                    ii.lockedRead();
                    fail |= ii.fail;
                } // read all
                if (fail) {
                    if (LOG)
                        LogN(S + "IC.import FAILED");
                    resetPak(&lock);
                    goto reset;
                } // if any failed then 'resetPak', reset while still under lock, it will be unlocked inside
            }
            checkPakFileInfo();
        reset:
            // at this point there should be no PAK imports (for COPY_DOWNLOADED_MEM also no DOWNLOADED imports)

            PostHeader post_header(T);
            auto &files = post_header.files;
            Long file_size = 0;
            Bool keep_got_removed = false;
            FREPA(_downloaded) {
                C Str &link = _downloaded.key(i);
                if (!_missing.find(link))
                    file_size += files.New().set(link, _downloaded[i]).compressed_size;
            }
            FREPA(_pak)
            if (i) {
                Str link = _pak.fullName(i);
                if (!_downloaded.find(link) && !_missing.find(link))
                    file_size += files.New().set(link, _pak, _pak.file(i), _pak_files[i]).compressed_size;
            } // skip post-header #PostHeaderFileIndex and files already included from '_downloaded'

            cleanMissing();

            if (_max_file_size >= 0 && file_size > _max_file_size) // limit file size
            {
                files.sort(CompareAccessTimeDesc);
                do {
                    SrcFile &file = files.last();
                    if (file.downloaded == keep)
                        keep_got_removed = true;
                    file_size -= file.compressed_size;
                    files.removeLast();
                } while (file_size > _max_file_size && files.elms());
            }
            files.sort(CompareName); // needed for 'binaryFind' in 'SavePostHeader' and below

            if (((_max_file_size < 0 || _pak_size / 2 < _max_file_size)                                                                                   // if pak size is smaller than 2x limit "_pak_size<_max_file_size*2", allow some tolerance because 'replaceInPlace' does not generate compact paks but they may have holes
                 && _pak.replaceInPlace(_pak_used_file_ranges, SCAST(Memc<PakFileData>, files), 0, _compress, _compress_level, null, null, &post_header)) // replace in place
                || _pak.replace(_pak_used_file_ranges, SCAST(Memc<PakFileData>, files), 0, _compress, _compress_level, null, null, &post_header))         // recreate
            {
                getPakFileInfo();
                _pak_files.setNumDiscard(_pak.totalFiles());
                REPA(_pak_files) {
                    FileTime &file = _pak_files[i];
                    if (C SrcFile *src = files.binaryFind(_pak.fullName(i), CompareName)) {
                        file.access_time = src->access_time;
                        file.verify_time = src->verify_time;
                    } else
                        file.zero();
                }

                // delete '_downloaded' as its data was moved to PAK
                if (!COPY_DOWNLOADED_MEM) {
                    if (_threads) // wait for Import Threads processing Downloaded files which we're about to delete
                    {
                        Memt<Threads::Call> calls;
                        REPA(_import_images) {
                            auto &ii = _import_images[i];
                            if (!ii.done && ii.isDownloaded())
                                calls.New().set(ii, ImportImageFunc, T);
                        }
                        _threads->wait(calls);
                    } else {
                        REPA(_import_images) {
                            auto &ii = _import_images[i];
                            if (!ii.done && ii.isDownloaded())
                                ImportImageFunc(ii, T);
                        } // don't use 'ii.import' because that one is inline
                    }
                }
                if (keep_got_removed && keep)
                    Swap(keep->file_data, *keep_data); // swap before deleting
                _downloaded.del();
            } else {
                if (LOG)
                    LogN(S + "IC.flush !FAIL! " + Flt(Time.curTime() - time));
                return false;
            }
        } else {
            if (_downloaded.elms())
                if (_max_mem_size >= 0 || _missing.elms()) // limit mem size or remove missing
                {
                    Long max_size = ((_max_mem_size >= 0) ? _max_mem_size : LONG_MAX);
                    Mems<SrcFile> files(_downloaded.elms()); // don't use 'Memt' because we need a lot of stack memory for 'ImportImageFunc'
                    Long size = 0;
                    FREPA(files)
                    size += files[i].set(_downloaded.key(i), _downloaded[i]).compressed_size;
                    if (size > max_size || _missing.elms()) // limit mem size or remove missing
                    {
                        if (size > max_size)
                            files.sort(CompareAccessTimeDesc);
                        if (COPY_DOWNLOADED_MEM) {
                            WriteLock lock(_rws);
                            REPAO(_import_images).lockedRead();
                            // at this point there should be no DOWNLOADED imports
                        }
                        REPA(files) {
                            SrcFile &file = files[i];
                            if (size > max_size              // exceeds mem limit
                                || _missing.find(file.name)) // or this file has gone missing
                            {                                // remove this 'file'
                                Downloaded &downloaded = *file.downloaded;
                                if (!COPY_DOWNLOADED_MEM) // we're going to remove 'downloaded'
                                    REPA(_import_images) {
                                        auto &ii = _import_images[i];
                                        if (!ii.done && ii.isDownloaded() && ii.data.memory == downloaded.file_data.data()) // find all imports using its data
                                        {
                                            if (_threads)
                                                _threads->wait(ii, ImportImageFunc, T); // wait for thread to finish
                                            else
                                                ImportImageFunc(ii, T); // don't use 'ii.import' because that one is inline
                                        }
                                    }
                                if (&downloaded == keep)
                                    Swap(keep->file_data, *keep_data); // swap before deleting
                                _downloaded.removeData(&downloaded);
                                size -= file.compressed_size;
                            } else if (!_missing.elms())
                                break; // if released enough memory, and don't have any missing then we can stop
                        }
                    }
                }
            cleanMissing();
        }
    }
    if (LOG)
        LogN(S + "IC.flush Finish " + Flt(Time.curTime() - time));
    return true;
}
/******************************************************************************/
Bool InternetCache::verified(Flt time) C { return TIME - time <= _verify_life; }

Bool InternetCache::busy() C {
    if (_to_download.elms() || _to_verify.elms() || _import_images.elms())
        return true;
    REPA(_downloading)
    if (_downloading[i].state() != DWNL_NONE) return true;
    return false;
}
InternetCache::ImportImage *InternetCache::findImport(C Image &image) {
    REPA(_import_images) {
        ImportImage &ii = _import_images[i];
        if (ii.image_ptr == &image)
            return &ii;
    } // there can be only one import for an image at a time
    return null;
}
Bool InternetCache::_loading(C Str &url) C // assumes "url.is"
{
    REPA(_downloading)
    if (EQUAL(_downloading[i].url(), url)) return true;
    if (_to_download.binaryHas(url, COMPARE) || _to_verify.binaryHas(url, COMPARE))
        return true;
    return false;
}
Bool InternetCache::loading(C ImagePtr &image) C {
    if (image) {
        if (findImport(*image))
            return true; // importing
        Str url = image.name();
        if (url.is()) {
            if (_loading(url))
                return true;
#if 1
            if (is_image_lod && image_lod_to_url) {
                C Str &name = url; // for ImageLOD 'image.name' is name
                Lod lod;
                if (is_image_lod(name, lod)) // check if this is a ImageLOD
                    for (Int l = lod.min; l <= lod.max; l++)
                        if (_loading(image_lod_to_url(name, l)))
                            return true; // check all possible LODs
            }
#else
            if (is_image_lod && url_to_image_lod) {
                C Str &name = url; // for ImageLOD 'image.name' is name
                Lod lod;
                if (is_image_lod(name, lod)) // check if this is a ImageLOD
                {
                    Int l;
                    REPA(_downloading)
                    if (EQUAL(url_to_image_lod(_downloading[i].url(), l), name)) return true;
                    REPA(_to_download)
                    if (EQUAL(url_to_image_lod(_to_download[i], l), name)) return true;
                    REPA(_to_verify)
                    if (EQUAL(url_to_image_lod(_to_verify[i], l), name)) return true;
                }
            }
#endif
        }
    }
    return false;
}
/******************************************************************************/
Bool InternetCache::getFile(C Str &url, DataSourceTime &file, CACHE_VERIFY verify) { return _getFile(url, file, verify) == FILE; }
InternetCache::GET InternetCache::_getFile(C Str &url, DataSourceTime &file, CACHE_VERIFY verify, Bool access, Bool download) {
    file.zero();
    if (!url.is())
        return FILE;
    Str link = SkipHttpWww(url);
    if (!link.is())
        return NONE;

    // check if it's known to be missing
    if (FileTime *missing = _missing.find(link)) {
        if (access)
            missing->access_time = TIME;
        if (verified(missing->verify_time))
            return NONE;               // verification still acceptable
        verify = CACHE_VERIFY_EXPIRED; // verification expired, however last known state is missing, so try to verify/download, but prevent from returning FILE
    }

    // find in cache
    Flt *verify_time;
    if (Downloaded *down = _downloaded.find(link)) {
        if (access)
            down->access_time = TIME;
        verify_time = &down->verify_time;
        file.set(down->file_data.data(), down->file_data.elms());
        file.modify_time_utc = down->modify_time_utc;
    } else if (C PakFile *pf = _pak.find(link, false)) {
        FileTime &time = pakFile(*pf);
        if (access)
            time.access_time = TIME;
        verify_time = &time.verify_time;
        file.set(*pf, _pak);
        file.modify_time_utc = pf->modify_time_utc;
    } else // not found
    {
        if (download) // download
        {
            REPA(_downloading)
            if (EQUAL(_downloading[i].url(), url)) goto downloading;
            if (_to_download.binaryInclude(url, COMPARE)) {
                enable();
                _to_verify.binaryExclude(url, COMPARE);
            }
        downloading:
            return DOWNLOADING;
        }
        return NONE;
    }

    // found
    {
        if (verify == CACHE_VERIFY_SKIP)
            return FILE; // verification not   needed
        if (verify != CACHE_VERIFY_EXPIRED && verified(*verify_time))
            return FILE; // verification still acceptable
        if (download)    // verify
        {
            REPA(_downloading)
            if (EQUAL(_downloading[i].url(), url)) goto verifying; // downloading now
            if (_to_download.binaryHas(url, COMPARE))
                goto verifying; // will download soon
            if (_to_verify.binaryInclude(url, COMPARE))
                enable(); // verify
        verifying:;
            return (verify == CACHE_VERIFY_DELAY) ? FILE : DOWNLOADING;
        }
        return (verify == CACHE_VERIFY_DELAY) ? FILE : NONE;
    }
}
/******************************************************************************/
ImagePtr InternetCache::getImage(C Str &url, CACHE_VERIFY verify) {
    ImagePtr img;
    if (url.is()) {
        DataSourceTime file;
        Bool get_file = getFile(url, file, verify); // always call 'getFile' to adjust 'access_time' and request verification if needed
        if (!img.find(url)) {
            CACHE_MODE mode = Images.mode(CACHE_DUMMY);
            img = url;
            Images.mode(mode);
            if (get_file) {
                ImportImage &ii = _import_images.New();
                Swap(ii.data, file);
                ii.type = (ii.data.type == DataSource::PAK_FILE ? ImportImage::PAK : ImportImage::DOWNLOADED);
                ii.image_ptr = img;
                ii.lod = -1;
                import(ii);
                enable();
            }
        }
    }
    return img;
}
/******************************************************************************/
ImagePtr InternetCache::getImageLOD(C Str &name) {
    ImagePtr img;
    if (name.is()) {
        CACHE_MODE mode = Images.mode(CACHE_DUMMY);
        img = name;
        Images.mode(mode);
    }
    return img;
}
void InternetCache::_setImageLOD(Image &img, C Str &name, Int lod, CACHE_VERIFY verify) {
    Lod lods;
    if (is_image_lod && is_image_lod(name, lods) && image_lod_to_url) {
        Clamp(lod, lods.min, lods.max);

        DataSourceTime data;
        GET get = NONE;
        Int file_lod;
        // start download
        for (file_lod = lod; file_lod <= lods.max; file_lod++)
            if (get = _getFile(image_lod_to_url(name, file_lod), data, verify))
                goto got;
            else
                break; // if FILE or DOWNLOADING (if NONE then break and stop looking, this assumes that if one mip is missing, then there won't be any higher mip available than that)
        for (file_lod = lod; --file_lod >= lods.min;)
            if (get = _getFile(image_lod_to_url(name, file_lod), data, verify))
                goto got; // if FILE or DOWNLOADING
    got:
        // get any preview
        if (get != FILE) // if haven't found any file
        {
            for (file_lod = lod; file_lod <= lods.max; file_lod++)
                if (_getFile(image_lod_to_url(name, file_lod), data, verify, false, false))
                    goto got_file; // if FILE
            for (file_lod = lod; --file_lod >= lods.min;)
                if (_getFile(image_lod_to_url(name, file_lod), data, verify, false, false))
                    goto got_file; // if FILE
        } else                     // got FILE
        got_file:
            if (file_lod > LOD(img)) // import only if we might improve quality
            {
                if (auto import = findImport(img)) {
                    if (import->lod >= file_lod)
                        goto importing; // importing same or better quality, no need to import anything
                    cancel(*import);
                }
                // import
                ImportImage &ii = _import_images.New();
                Swap(ii.data, data);
                ii.type = (ii.data.type == DataSource::PAK_FILE ? ImportImage::PAK : ImportImage::DOWNLOADED);
                ii.image_ptr = &img;
                ii.lod = file_lod;
                import(ii);
                enable();
            }
    importing:;
    }
}
ImagePtr InternetCache::getImageLOD(C Str &name, Int lod, CACHE_VERIFY verify) {
    ImagePtr img;
    if (name.is()) {
        CACHE_MODE mode = Images.mode(CACHE_DUMMY);
        img = name;
        Images.mode(mode);
        _setImageLOD(*img, name, lod, verify);
    }
    return img;
}
void InternetCache::setImageLOD(C ImagePtr &img, Int lod, CACHE_VERIFY verify) {
    if (img)
        _setImageLOD(*img, img.name(), lod, verify);
}
/******************************************************************************/
Bool InternetCache::_changed(C Str &url, SByte download) // 'download' -1=never, 0=if referenced, 1=always, return if downloading
{
    Str link = SkipHttpWww(url);
    if (link.is()) {
        // set everything as expired
        Bool referenced = false;
        if (Downloaded *down = _downloaded.find(link)) {
            down->verify_time = INT_MIN;
            referenced = true;
        }
        if (C PakFile *pf = _pak.find(link, true)) {
            pakFile(*pf).verify_time = INT_MIN;
            referenced = true;
        }
        if (!referenced) {
            if (FileTime *miss = _missing.find(link)) {
                miss->verify_time = INT_MIN;
                referenced = true;
            }
        } else
        /*  referenced*/ {
            FileTime *miss = _missing(link);
            miss->verify_time = INT_MIN;
            miss->access_time = TIME;
        } // if image is 'referenced' (has some down/pf data), then disable that data from usage, always create missing to prevent from using that data with CACHE_VERIFY_DELAY/CACHE_VERIFY_SKIP, here "referenced=true" not needed because it's already true

        // check if downloading now
        REPA(_downloading) {
            Download &down = _downloading[i];
            if (EQUAL(down.url(), url)) { // restart the download
#if 1
                down.create(url);
#else
                down.del();
                _to_download.binaryInclude(url, COMPARE);
#endif
                return true;
            }
        }
        if (_to_verify.binaryHas(url, COMPARE))
            return true; // going to being verified
        if (download + referenced >= 1) {
            if (_to_download.binaryInclude(url, COMPARE))
                enable();
            return true;
        } // download
        else {
            if (_to_download.binaryHas(url, COMPARE))
                return true;
        } // going to being downloaded
    }
    return false;
}
void InternetCache::changed(C Str &url) {
    if (url.is()) {
        if (LOG)
            LogN(S + "IC.changed " + url);

        ImagePtr img;
        img.find(url); // check if currently in cache

        Lod lod;
        if (is_image_lod && is_image_lod(url, lod)) // this is ImageLOD
        {
            if (image_lod_to_url) {
                Int requested = -1;
                if (img) {
                    requested = LOD(*img);
                    if (auto import = findImport(*img))
                        MAX(requested, import->lod);    // find highest lod that was requested
                    Clamp(requested, lod.min, lod.max); // clamp min forces to load smallest lod (at least one) if nothing requested, clamp max is for safety
                }
                for (Int l = lod.max; l >= lod.min; l--)                             // start from max, to check highest lod that was requested, if yes, then can download all lower as well
                    if (_changed(image_lod_to_url(url, l), (l == requested)   ? 1    // if this is    what was requested, then always download
                                                           : (l <= requested) ? 0    // if lower than what was requested, then download if referenced. Don't download lods higher than requested, because if image was loaded only at small lod, but we would download bigger lod, then it would get loaded in download finish causing image to be loaded at higher lod/res. This line is optional, however better to do it, because if highest 'requested' lod is unavailable anymore, then we would have to wait for failed download to proceed to the lower one
                                                                              : -1)) // no need to download
                        if (img)
                            MAX(requested, l); // that lod was requested, so adjust value, but only if have 'img' (without 'img' we don't want to download anything)
            }
        } else
            _changed(url, img ? 1 : -1);

        cancel(img); // consider all data expired so cancel any imports, do this at the end because we still check '_import_images' earlier
    }
}
/******************************************************************************/
void InternetCache::import(ImportImage &ii) {
    if (LOG)
        LogN(S + "IC.import   Start  " + ii.asText());
    if (_threads)
        _threads->queue(ii, ImportImageFunc, T);
    else
        ImportImageFunc(ii, T); // don't use 'ii.import' because that one is inline
}
void InternetCache::cancel(ImportImage &ii) // canceling is needed to make sure we won't replace newer data with older
{
    if (ii.done                                      // finished
        || !_threads                                 // no threads
        || _threads->cancel(ii, ImportImageFunc, T)) // canceled
        _import_images.removeData(&ii);              // just remove
    else
        ii.image_ptr = null; // now processing, clear 'image_ptr' so we will ignore it
}
void InternetCache::cancel(C ImagePtr &image) // canceling is needed to make sure we won't replace newer data with older
{
    if (auto import = findImport(image))
        cancel(*import);
}
void InternetCache::updatingDownloaded(Ptr data) // called when updating 'downloaded'
{
    if (data)
        REPA(_import_images) {
            ImportImage &ii = _import_images[i];
            if (ii.isDownloaded() && ii.data.memory == data) {
                if (ii.done                                      // finished
                    || !_threads                                 // no threads
                    || _threads->cancel(ii, ImportImageFunc, T)) // canceled
                remove:
                    _import_images.removeValid(i); // just remove
                else                               // now processing
                {
                    if (COPY_DOWNLOADED_MEM) {
                        {
                            WriteLock lock(_rws);
                            if (ii.isDownloaded()) // check again under lock
                            {
                                ii.data.set(null, 0);
                                ii.type = ImportImage::OTHER; // !! ADJUST AT THE END ONCE EVERYTHING IS READY !! important for import thread which first does fast check without locking
                            }
                        }
                        ii.image_ptr = null; // clear 'image_ptr' so we will ignore it
                    } else {
                        _threads->wait(ii, ImportImageFunc, T); // wait for finish
                        goto remove;
                    }
                }
            }
        }
}
/******************************************************************************/
void InternetCache::resetPak(WriteLockEx *lock) {
    if (LOG)
        LogN(S + "IC.resetPak");

    // we're going to recreate the PAK file, as old one is considered invalid/missing/modified
    Memt<Threads::Call> calls;
    {
        WriteLock lock(_rws); // stop any further reads, this stops any conversions from 'isPak' until we release the lock, this is important as we need all isPak/fail to be included in 'calls' below, so we can cancel/wait for threads to finish, as they're going to be removed
        REPA(_import_images) {
            auto &ii = _import_images[i];
            if (ii.isPak())
                ii.fail = true;
            if (ii.fail)
                calls.New().set(ii, ImportImageFunc, T);
        } // force all PAK as fail, we assume that PAK is compromised, this will also force PAK import to stop importing and return quickly
        if (_threads)
            _threads->cancel(calls);
    }
    if (lock)
        lock->off(); // unlock first
    if (_threads)
        _threads->wait(calls); // wait for all failed imports to return, have to wait with lock disabled

    Memt<Str> retry;
    REPA(_import_images) {
        auto &ii = _import_images[i];
        if (ii.fail) {
            if (ii.image_ptr)
                retry.add(ii.image_ptr.name()); // not canceled, retry
            _import_images.removeValid(i);
        }
    }

    PostHeader post_header(T);
    _pak.create(CMemPtr<PakFileData>(), fileName(), 0, _pak._file_cipher, COMPRESS_NONE, 0, null, null, null, _pak_used_file_ranges, &post_header); // create an empty pak
    _pak_files.setNumDiscard(_pak.totalFiles());
    REPAO(_pak_files).zero();
    getPakFileInfo();

    if (retry.elms()) {
        Bool enable = false;
        REPA(retry) {
            C Str &url = retry[i];
            // this file was from Pak that failed to load, and it wasn't canceled, it means it's not available locally anymore, try to download
            // no need to check 'missing' because once it's detected then all imports for that image are canceled
            REPA(_downloading)
            if (EQUAL(_downloading[i].url(), url)) goto downloading;
            if (_to_download.binaryInclude(url, COMPARE)) {
                enable = true;
                _to_verify.binaryExclude(url, COMPARE);
            }
        downloading:;
        }
        if (enable)
            T.enable();
    }
}
/******************************************************************************/
void InternetCache::received(C Download &down, Int &down_lod, ImagePtr &image) {
    if (url_to_image_lod) {
        Str name = url_to_image_lod(down.url(), down_lod);
        if (image.find(name)) {
            C DateTime *modify_time = null;
            Int size, lod;
            Flt verify_time;
            auto import = findImport(*image);

            if (import) // #ImgLodPriority
            {
                modify_time = &import->data.modify_time_utc;
                size = import->data.size();
                lod = import->lod;
                verify_time = TIME;
            } else {
                lod = LOD(*image);
                if (down_lod > lod)
                    goto Import; // always import if received higher quality, to skip codes below
                if (image_lod_to_url && lod >= 0) {
                    Str link = SkipHttpWww(image_lod_to_url(name, lod)); // name of data that's already loaded in the image
                                                                         // if(                  !_missing   .find(link       )) ignore this, it will help when calling 'changed' and receiving new data that's actually the same
                    if (C Downloaded *down = _downloaded.find(link)) {
                        modify_time = &down->modify_time_utc;
                        size = down->file_data.elms();
                        verify_time = down->verify_time;
                    } else if (C PakFile *pf = _pak.find(link, false)) {
                        modify_time = &pf->modify_time_utc;
                        size = pf->data_size;
                        verify_time = pakFile(*pf).verify_time;
                    }
                }
            }

            if (down_lod > lod) // always import if received higher quality
            {
            Import:
                if (import)
                    cancel(*import);
                return;
            } else if (down_lod == lod) // received same quality
            {
                if (modify_time) // found existing data
                {
                    // here ignore 'const_image_lod' because we have precise data to compare, so in case it actually changed, then still import
                    if (down.totalSize() == size && down.modifyTimeUTC() == *modify_time)
                        goto dont_import; // received same      data (check 'totalSize' because 'size' is 0 for verification)
                    goto Import;          // received different data
                } else                    // haven't found existing data
                {
                    if (const_image_lod && const_image_lod(name))
                        goto dont_import; // ImageLOD is always constant, no need to import
                    goto Import;          // import
                }
            } else // received lower quality
            {
                // import only if existing is considered expired
                if (modify_time && verified(verify_time))
                    goto dont_import; // existing data is still considered verified, no need to import
                if (const_image_lod && const_image_lod(name))
                    goto dont_import; // ImageLOD is always constant, no need to import
                if (image_lod_to_url) // in case we requested high lod, and after that also smaller lod, in the meantime high lod expired (but we still want it), smaller lod got received, importing to smaller lod would lower quality, so instead of importing lower, first try to verify/download high lod, and on error downgrade quality there #ImgLodError
                {
                    DataSourceTime data;
                    if (_getFile(image_lod_to_url(name, lod), data, CACHE_VERIFY_YES, false, true) == DOWNLOADING)
                        goto dont_import; // normally this should be DOWNLOADING, because we already know that verification expired, the only other thing is missing, which gets set in download failed, but there image/import (its LOD) get adjusted to lower level
                }
                goto Import;
            }

        dont_import:
            image.clear();
        }
    }
}
/******************************************************************************/
inline void InternetCache::update() {
    // update imported images
    REPA(_import_images) {
        ImportImage &ii = _import_images[i];
        if (ii.done) {
            if (ii.fail) {
                if (LOG)
                    LogN(S + "IC.import   !FAIL! " + ii.asText());
                resetPak();
                break;
            }                 // if failed to open file, then we have to reset, break because 'resetPak' will handle '_import_images'
            if (ii.image_ptr) // not canceled
            {
                if (LOG)
                    LogN(S + "IC.import   Finish " + ii.asText());
                Swap(*ii.image_ptr, ii.image_temp); // put imported image into target
                if (got)                            // callback
                {
                    ImagePtr temp;
                    Swap(ii.image_ptr, temp);
                    _import_images.removeValid(i); // remove first
                    got(temp);                     // callback might modify '_import_images', so call after removing element
                    MIN(i, _import_images.elms()); // adjust index to prevent from being out of range
                    goto removed;
                }
            }
            _import_images.removeValid(i);
        }
    removed:;
    }

    // process downloaded data
    REPA(_downloading) {
        Download &down = _downloading[i];
        switch (down.state()) {
        case DWNL_NONE: {
        again:
            if (_to_download.elms()) {
                down.create(_to_download.last());
                _to_download.removeLast();
                if (LOG)
                    LogN(S + "IC.download Start  " + down.url());
            } else if (_to_verify.elms()) {
                down.create(_to_verify.last(), null, null, -1, 0);
                _to_verify.removeLast();
                if (LOG)
                    LogN(S + "IC.download Start  " + down.url());
            } // use offset as -1 to encode special mode of verification
        } break;

        case DWNL_DONE: // finished downloading
        {
            if (LOG)
                LogN(S + "IC.download Finish " + down.url());

            Str link = SkipHttpWww(down.url());
            if (down.offset() < 0) // if this was verification
            {
                Flt *verify_time = null;
                Downloaded *downloaded = _downloaded.find(link);
                C PakFile *pf = null;
                if (downloaded) {
                    if (downloaded->file_data.elms() == down.totalSize() && downloaded->modify_time_utc == down.modifyTimeUTC()) {
                        verify_time = &downloaded->verify_time;
                    }
                } else if (pf = _pak.find(link, false)) {
                    if (pf->data_size == down.totalSize() && pf->modify_time_utc == down.modifyTimeUTC()) {
                        verify_time = &pakFile(*pf).verify_time;
                    }
                }

                if (verify_time) {
                    ImagePtr img;
                    if (img.find(down.url()))
                        if (!img->is() && !findImport(*img)) // if image empty, it's possible the image was not yet loaded due to CACHE_VERIFY_YES, check if empty and not importing already
                        {
                            ImportImage &ii = _import_images.New();
                            if (downloaded) {
                                ii.data.set(downloaded->file_data.data(), downloaded->file_data.elms());
                                ii.data.modify_time_utc = downloaded->modify_time_utc;
                                ii.type = ImportImage::DOWNLOADED;
                            } else {
                                ii.data.set(*pf, _pak);
                                ii.data.modify_time_utc = pf->modify_time_utc;
                                ii.type = ImportImage::PAK;
                            }
                            Swap(ii.image_ptr, img);
                            ii.lod = -1;
                            import(ii);
                        }

                    ImagePtr img_lod;
                    Int down_lod;
                    received(down, down_lod, img_lod);
                    if (img_lod) {
                        // cancel(img_lod); already done in 'received'
                        ImportImage &ii = _import_images.New();
                        if (downloaded) {
                            ii.data.set(downloaded->file_data.data(), downloaded->file_data.elms());
                            ii.data.modify_time_utc = downloaded->modify_time_utc;
                            ii.type = ImportImage::DOWNLOADED;
                        } else {
                            ii.data.set(*pf, _pak);
                            ii.data.modify_time_utc = pf->modify_time_utc;
                            ii.type = ImportImage::PAK;
                        }
                        Swap(ii.image_ptr, img_lod);
                        ii.lod = down_lod;
                        import(ii);
                    }

                    // after 'received'
                    *verify_time = TIME;
                    _missing.removeKey(link);
                    goto next;
                }
                down.create(Str(down.url())); // it's different so download it fully, copy the 'url' because it might get deleted in 'create'
            } else                            // move to 'downloaded'
            {
                // this must be checked first, before modifying 'downloaded->modify_time_utc', 'downloaded->verify_time', '_missing', because 'received' will compare with those values
                ImagePtr img;
                img.find(down.url());
                ImagePtr img_lod;
                Int down_lod;
                received(down, down_lod, img_lod);

                // after 'received'
                Bool just_created;
                Downloaded *downloaded = _downloaded(link, just_created);
                updatingDownloaded(downloaded->file_data.data()); // we're going to modify 'downloaded->file_data' so we have to make sure no imports are using that data
                downloaded->file_data.setNumDiscard(down.size()).copyFrom((Byte *)down.data());
                downloaded->modify_time_utc = down.modifyTimeUTC();
                downloaded->verify_time = TIME;
                if (just_created) { // set 'access_time'
                    if (C PakFile *pf = _pak.find(link, true))
                        downloaded->access_time = pakFile(*pf).access_time;
                    else                                                   // reuse from 'pak'
                        downloaded->access_time = downloaded->verify_time; // set as new
                }
                _missing.removeKey(link);

                // flush
                const Bool import = (img || img_lod);
                Mems<Byte> downloaded_data;
                if (_max_mem_size >= 0) {
                    Long mem_size = 0;
                    REPA(_downloaded)
                    mem_size += _downloaded[i].file_data.elms();
                    if (mem_size > _max_mem_size) {
                        if (import)
                            flush(downloaded, &downloaded_data); // if we're going to import, then make sure if 'downloaded' is going to get removed, then we will keep its memory
                        else
                            flush();
                        downloaded = null; // flush deletes '_downloaded' so clear 'downloaded', we'll find it again
                    }
                }

                if (import) {
                    C PakFile *pf = null;
                    if (!downloaded) {
                        downloaded = _downloaded.find(link);
                        if (!downloaded)
                            pf = _pak.find(link, false);
                    }
                    // if(downloaded || pf) ignore because if it got removed and not found, then data was put in 'downloaded_data'
                    {
                        if (img) {
                            cancel(img);
                            ImportImage &ii = _import_images.New();
                            if (downloaded) {
                                ii.data.set(downloaded->file_data.data(), downloaded->file_data.elms());
                                ii.data.modify_time_utc = downloaded->modify_time_utc;
                                ii.type = ImportImage::DOWNLOADED;
                            } else if (pf) {
                                ii.data.set(*pf, _pak);
                                ii.data.modify_time_utc = pf->modify_time_utc;
                                ii.type = ImportImage::PAK;
                            } else {
                                if (img_lod)
                                    ii.temp = downloaded_data;
                                else
                                    Swap(ii.temp, downloaded_data);
                                ii.data.set(ii.temp.data(), ii.temp.elms());
                                ii.data.modify_time_utc = down.modifyTimeUTC();
                                ii.type = ImportImage::OTHER;
                            }
                            Swap(ii.image_ptr, img);
                            ii.lod = -1;
                            T.import(ii);
                        }
                        if (img_lod) {
                            // cancel(img_lod); already done in 'received'
                            ImportImage &ii = _import_images.New();
                            if (downloaded) {
                                ii.data.set(downloaded->file_data.data(), downloaded->file_data.elms());
                                ii.data.modify_time_utc = downloaded->modify_time_utc;
                                ii.type = ImportImage::DOWNLOADED;
                            } else if (pf) {
                                ii.data.set(*pf, _pak);
                                ii.data.modify_time_utc = pf->modify_time_utc;
                                ii.type = ImportImage::PAK;
                            } else {
                                Swap(ii.temp, downloaded_data);
                                ii.data.set(ii.temp.data(), ii.temp.elms());
                                ii.data.modify_time_utc = down.modifyTimeUTC();
                                ii.type = ImportImage::OTHER;
                            }
                            Swap(ii.image_ptr, img_lod);
                            ii.lod = down_lod;
                            T.import(ii);
                        }
                    }
                }
                goto next;
            }
        } break;

        case DWNL_ERROR: // failed
        {
            if (down.code() == 404) // confirmed that file is missing
            {
                if (LOG)
                    LogN(S + "IC.download !FAIL! " + down.url());

                Str link = SkipHttpWww(down.url());
                Bool just_created;
                FileTime &missing = *_missing(link, just_created);
                missing.verify_time = TIME;
                if (just_created) { // set 'access_time'
                    if (C Downloaded *downloaded = _downloaded.find(link))
                        missing.access_time = downloaded->access_time;
                    else // reuse from 'downloaded'
                        if (C PakFile *pf = _pak.find(link, true))
                            missing.access_time = pakFile(*pf).access_time;
                        else                                           // reuse from 'pak'
                            missing.access_time = missing.verify_time; // set as new
                }
                // codes below cannot be inside 'just_created' because expired (not verified) '_missing' can be created in 'changed'
                ImagePtr img;
                if (img.find(down.url())) // delete image
                {
                    cancel(img);
                    if (img->is()) {
                        img->del();
                        if (got)
                            got(img); // notify because image got modified
                    }
                }
                Int down_lod;
                if (is_image_lod && url_to_image_lod && image_lod_to_url) {
                    Str name = url_to_image_lod(down.url(), down_lod);
                    if (img.find(name)) {
                        // #ImgLodError
                        auto import = findImport(*img);
                        Int img_lod = (import ? import->lod : LOD(*img)); // need to check before deleting import, because 'import' overrides image 'lod' #ImgLodPriority
                        Bool adjust = (img_lod == down_lod);              // if what we have has gone missing, we'll need to adjust existing image (either via import of lower res lod, or deleting), cannot keep the same content, because when lower LOD gets received it could trigger download of this LOD again in an endless loop
                        if (import && (import->lod == down_lod || adjust)) {
                            cancel(*import);
                            import = null;
                        } // cancel import with this LOD, or if we're going to adjust image

                        Lod lod;
                        if (is_image_lod(name, lod))
                            for (Int file_lod = down_lod; --file_lod >= lod.min;) // make sure we have any lower mip
                            {
                                CACHE_VERIFY verify = CACHE_VERIFY_DELAY; // this is for temporary preview (it can get replaced with verified data soon, since we're requesting lower LOD download below), CACHE_VERIFY_DELAY might give previous expired results, while CACHE_VERIFY_YES might cause image to disapper (del)
                                DataSourceTime data;
                                if (GET get = _getFile(image_lod_to_url(name, file_lod), data, verify, false, true)) // if have FILE or DOWNLOADING we can stop
                                {
                                    if (adjust) // but if need to adjust existing image, then keep going until we find FILE so we can import it now
                                    {
                                        if (get != FILE)
                                            for (; --file_lod >= lod.min;) {
                                                get = _getFile(image_lod_to_url(name, file_lod), data, verify, false, false);
                                                if (get == FILE)
                                                    break;
                                            } // this time don't download
                                        if (get == FILE) {
                                            ImportImage &ii = _import_images.New();
                                            Swap(ii.data, data);
                                            ii.type = (ii.data.type == DataSource::PAK_FILE ? ImportImage::PAK : ImportImage::DOWNLOADED);
                                            Swap(ii.image_ptr, img);
                                            ii.lod = file_lod;
                                            T.import(ii);
                                            adjust = false; // started import, can clear 'adjust', no need to delete image
                                        }
                                    }
                                    break;
                                }
                            }

                        if (adjust && img->is()) // if failed to request import
                        {
                            img->del();
                            if (got)
                                got(img); // notify because image got modified
                        }
                    }
                }
            }
        next:
            down.del();
            goto again;
        } break;
        }
    }

    if (busy())
        enable();
}
static void ICUpdate(InternetCache &ic) { ic.update(); }
/******************************************************************************/
} // namespace EE
/******************************************************************************/
