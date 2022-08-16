/******************************************************************************/
#include "stdafx.h"
namespace EE{
#define AUTO_INSTALLER_INFO 0 // if automatically download installer info when requesting installer
/******************************************************************************/
// STATICS
/******************************************************************************/
Patcher::LocalFile& Patcher::LocalFile::set(C Str &full_name, C FileInfo &fi)
{
   T.full_name      =full_name;
   T.type           =((fi.type==FSTD_FILE || fi.type==FSTD_LINK) ? Patcher::LocalFile::SYSTEM_FILE : Patcher::LocalFile::SYSTEM_DIR);
   T.file_size      =fi.size;
   T.modify_time_utc=fi.modify_time_utc;
   T.xxHash64_32    =0;
   return T;
}
static void AddFiles(MemPtr<Patcher::LocalFile> &local_files, FileFind &ff, Str parent)
{
   local_files.New().set(parent+ff.name, ff);
   if(ff.type==FSTD_DIR)
   {
      parent+=ff.name; parent+='\\';
      for(FileFind child(ff.pathName()); child(); )AddFiles(local_files, child, parent);
   }
}
void Patcher::SetList(MemPtr<LocalFile> local_files, C Str &dir)
{
   local_files.clear();
   for(FileFind ff(dir); ff(); )AddFiles(local_files, ff, S);
}
void Patcher::SetList(MemPtr<LocalFile> local_files, C PakSet &paks)
{
   local_files.clear();
   FREPA(paks)
   {
      LocalFile &lf= local_files.New();
      C PakFile &pf=*paks.file(i).file;
      lf.type           =LocalFile::PAK_FILE;
      lf.full_name      =paks.fullName(i);
      lf.file_size      =pf.data_size;
      lf.xxHash64_32    =pf.data_xxHash64_32;
      lf.modify_time_utc=pf.modify_time_utc;
   }
}
/******************************************************************************/
Str Patcher::Platform(OS_VER os) {return CaseDown(OSName(OSGroup(os)));}
/******************************************************************************/
// PATCHER DOWNLOADED
/******************************************************************************/
void Patcher::Downloaded::create(C Pak &pak, Int index, Download &download, Cipher *cipher)
{
 C PakFile &pf=pak.file(index);
   full_name=pak.fullName(pf);
 T.index    =index;
   if(download.size()==pf.data_size_compressed)
   {
      File temp; temp.readMem(download.data(), download.size(), cipher);
      xxHash64Calc _hasher, *hasher=(pf.data_xxHash64_32 ? &_hasher : null); // verify hash if available
      if(DecompressRaw(temp, data, pf.compression, pf.data_size_compressed, pf.data_size, true, hasher))
      if(                     data.size()==pf.data_size       )
      if(!hasher || hasher->hash.hash32()==pf.data_xxHash64_32)
      {
         data.pos(0);
         success        =true;
         type           =pf.type();
         xxHash64_32    =pf.data_xxHash64_32;
         modify_time_utc=pf.modify_time_utc;
         return;
      }
   }
   data.del();
}
void Patcher::Downloaded::createEmpty(C Pak &pak, Int index)
{
 C PakFile &pf=pak.file(index);
   success        =true;
   xxHash64_32    =pf.data_xxHash64_32;
   type           =pf.type();
   modify_time_utc=pf.modify_time_utc;
   full_name      =pak.fullName(pf);
 T.index          =index;
}
void Patcher::Downloaded::createFail(C Pak &pak, Int index)
{
 C PakFile &pf=pak.file(index);
   full_name=pak.fullName(pf);
 T.index    =index;
   type     =pf.type();
}
/******************************************************************************/
Patcher::InstallerInfo::InstallerInfo() {zero();}
/******************************************************************************/
// PATCHER
/******************************************************************************/
void Patcher::zero()
{
  _pak_available      =false;
  _inst_info_available=false;
  _inst_available     =false;
  _cipher             =null;
  _files_left         =0;
  _bytes_downloaded   =0;
   REPAO(_file_download).clearIndex();
}
inline void Patcher::delThread()
{
  _thread.stop(); _event.on(); _thread.del();
}
Patcher::Patcher() {zero();}
Patcher& Patcher::del()
{
   delThread(); // first delete thread
  _pak_download.del (); REPAO(_file_download).del(); _inst_info_download.del(); _inst_download.del(); // delete all downloaders
  _pak         .del ();
  _inst_info   .zero();
  _inst        .del ();
  _to_download .del ();
  _downloaded  .del ();
  _http        .del ();
  _name        .del ();
   zero(); return T;
}
Patcher& Patcher::create(C Str &http_dir, C Str &upload_name, Cipher *cipher)
{
   del();
   T._http  =http_dir; T._http.tailSlash(true);
   T._name  =upload_name;
   T._cipher=cipher;
   return T;
}
/******************************************************************************/
Patcher& Patcher::downloadInstallerInfo()
{
   if(is())
   {
     _inst_info_available=false;
     _inst_info.zero();
     _inst_info_download.create(_http+CaseDown(_name)+".installer."+Platform()+".txt");
   }
   return T;
}
DWNL_STATE Patcher::installerInfoState()
{
   if(_inst_info_available)return DWNL_DONE;
   DWNL_STATE state=_inst_info_download.state(); if(state==DWNL_DONE)
   {
      FileText f; f.readMem(_inst_info_download.data(), _inst_info_download.size()); TextData data; if(data.load(f))
      {
         if(TextNode *p=data.findNode("Size"         ))_inst_info.size       =p->asInt();
       //if(TextNode *p=data.findNode("xxHash32"     ))_inst_info.xxHash32   =TextUInt (S+"0x"+p->value);
         if(TextNode *p=data.findNode("xxHash64"     ))_inst_info.xxHash64_32=TextULong(S+"0x"+p->value)&UINT_MAX; // use 'TextULong' because 'TextUInt' may fail on large values
       //if(TextNode *p=data.findNode("xxHash64"     ))_inst_info.xxHash64   =TextULong(S+"0x"+p->value);
         if(TextNode *p=data.findNode("ModifyTimeUTC"))_inst_info.modify_time_utc.fromText(p->value);
         if(TextNode *p=data.findNode("Version"      ))_inst_info.version    =TextVer     (p->value);
        _inst_info_available=true; _inst_info_download.del(); return DWNL_DONE;
      }
     _inst_info_download.del().error(); state=DWNL_ERROR;
   }
   return state;
}
C Patcher::InstallerInfo* Patcher::installerInfo() {return (installerInfoState()==DWNL_DONE) ? &_inst_info : null;}
/******************************************************************************/
Patcher& Patcher::downloadInstaller()
{
   if(is())
   {
   #if AUTO_INSTALLER_INFO
      switch(installerInfoState())
      {
         case DWNL_NONE :
         case DWNL_ERROR:
            downloadInstallerInfo(); // for "Installer" we will also need "Installer Info"
         break;
      }
   #endif
     _inst_available=false;
     _inst.del();
     _inst_download.create(_http+CaseDown(_name)+".installer."+Platform());
   }
   return T;
}
DWNL_STATE Patcher::installerState()
{
   if(_inst_available)return DWNL_DONE;
   DWNL_STATE state=_inst_download.state(); if(state==DWNL_DONE)
   {
      xxHash64Calc *hasher=null;
   #if AUTO_INSTALLER_INFO
      DWNL_STATE inst_info_state=installerInfoState(); if(inst_info_state!=DWNL_DONE)return inst_info_state; // we need "Installer Info"
      xxHash64Calc _hasher; if(_inst_info.xxHash64_32)hasher=&_hasher; // if hash known, then test it
   #endif
      File compressed(_inst_download.data(), _inst_download.size(), _cipher);
      if(Decompress(compressed, _inst, true, hasher))
      {
      #if AUTO_INSTALLER_INFO
         if(                    _inst.size()==_inst_info.size       )
         if(!hasher || hasher->hash.hash32()==_inst_info.xxHash64_32)
      #endif
         {
           _inst.pos(0);
           _inst_available=true; _inst_download.del(); return DWNL_DONE;
         }
      #if AUTO_INSTALLER_INFO
         // if installer data doesn't match info, then one of possibilities is that data is OK but info outdated, so clear it
        _inst_info_available=false; //_inst_info.zero();
      #endif
      }
     _inst.del(); _inst_download.del().error(); state=DWNL_ERROR;
   }
   return state;
}
Int   Patcher::installerDownloadDone() {return _inst_download.done();}
Int   Patcher::installerDownloadSize() {return _inst_download.size();}
File* Patcher::installer            () {return (installerState()==DWNL_DONE) ? &_inst : null;}
/******************************************************************************/
Patcher& Patcher::downloadIndex()
{
   if(is())
   {
      delThread(); // first delete thread
     _pak_available=false;
     _pak.del();
     _pak_download.create(_http+CaseDown(_name)+".index.pak"); REPAO(_file_download).clearIndexDel();
     _to_download .del();
     _downloaded  .del();
     _files_left=0;
   }
   return T;
}
DWNL_STATE Patcher::indexState()
{
   if(_pak_available)return DWNL_DONE;
   DWNL_STATE state=_pak_download.state(); if(state==DWNL_DONE)
   {
      File compressed(_pak_download.data(), _pak_download.size(), _cipher), decompressed;
      if(Decompress(compressed, decompressed, true))
      {
         decompressed.pos(0); switch(_pak.loadHeader(decompressed))
         {
            case PAK_LOAD_OK             :
            case PAK_LOAD_INCOMPLETE_DATA: _pak_available=true; _pak_download.del(); return DWNL_DONE;
         }
      }
     _pak_download.del().error(); state=DWNL_ERROR;
   }
   return state;
}
C Pak* Patcher::index() {return (indexState()==DWNL_DONE) ? &_pak : null;}
/******************************************************************************/
static Str ServerPath(Str path, bool file=true) // !! this needs to be in sync with "Uploader" tool !!
{
   Str out; if(file){Str ext=GetExt(path); if(ext=="pl" || ext=="php" || ext=="old" || ext=="dat" || ext=="dll" || ext=="bat" || ext=="cmd")path+='_';} // these file formats can't be downloaded normally, because they're treated as scripts or some can be blocked by servers
   for(;;)
   {
      Str base=GetBase(path); if(!base.is())break;
      out =S + (file ? "f-" : "d-") + CaseDown(base) + (out.is() ? '/' : '\0') + out;
      path=GetPath(path);
      file=false;
   }
   return out;
}
static Bool PatcherUpdate  (Thread &thread) {((Patcher*)thread.user)->update(); return true;}
       void Patcher::update()
{
#if HAS_THREADS
  _event.wait();
#endif
   SyncLockerEx locker(_lock, false);
   REPA(_file_download)
   {
      FileDownload &download=_file_download[i];
      switch(       download.state())
      {
         case DWNL_DONE: // finished downloading
         {
            Downloaded downloaded; downloaded.create(_pak, download.index, download, _cipher);
            {locker.on(); Swap(downloaded, T._downloaded.New()); _bytes_downloaded+=download.done(); download.clearIndexDel(); goto next;} // set '_bytes_downloaded' and 'clearIndexDel' under lock to have correct knowledge of progress, don't delete 'download' outside of the lock, because 'Patcher.progress' operates on both '_bytes_downloaded' and 'Download.done', goto next without disabling lock because we will need it anyway
         }break;

         case DWNL_DOWNLOAD: // verify while downloading
         {
          C PakFile &pf=_pak.file(download.index);
            if((download.size()>=0) ? (download.size()!=pf.data_size_compressed)  // if file size is   known and it's    different than expected
                                    : (download.done()> pf.data_size_compressed)) // if file size is unknown but already exceeded  what expected
            {
               download.del(); // here del 'download' already, outside the 'lock' because it's still in progress and may take a while longer to stop it !! however do not clear 'index' because we need it below !!
               goto error; // set as failed download
            }
         }break;

         case DWNL_ERROR: error: // error encountered
         {
            Downloaded downloaded; downloaded.createFail(_pak, download.index); // create 'downloaded' as failed
            {locker.on(); Swap(downloaded, T._downloaded.New()); if(0)_bytes_downloaded+=download.done(); download.clearIndexDel(); goto next;} // 'clearIndexDel' under lock to have correct knowledge of progress (changing '_bytes_downloaded' is optional), goto next without disabling lock because we will need it anyway
         }break;

         case DWNL_NONE: if(_to_download.elms()) // not downloading anything
         {
            locker.on(); next: if(!_to_download.elms())locker.off();else // check for elements to download
            {
               download.index=_to_download.pop(); locker.off();
            #if 1 // direct download
               download.create(_http+CaseDown(_name)+'/'+ServerPath(_pak.fullName(download.index)), null, null, -1, 0, -1, false, false, &_event); // start downloading
            #else // through PHP (had problems when tried accessing 6 downloads at the same time)
               Memt<TextParam> params; params.New().set("file", ServerPath(_pak.fullName(download.index)));
               download.create(_http+CaseDown(_name)+".download.php", params, null, -1, 0, -1, false, false, &_event); // start downloading
            #endif
            }
         }break;
      }
   }
}
/******************************************************************************/
Patcher& Patcher::downloadFile(Int i)
{
   if(C Pak *pak=index())if(InRange(i, pak->files()))
   {
    C PakFile &pf=pak->file(i);
      if(pf.data_size) // has data size -> needs to be downloaded
      {
         {SyncLocker locker(_lock); _to_download.add(i); _files_left++;}
        _event.on();
         if(!_thread.created())_thread.create(PatcherUpdate, this);
      }else
      {
         SyncLocker locker(_lock); _downloaded.New().createEmpty(*pak, i); _files_left++;
      }
   }
   return T;
}
Bool Patcher::getNextDownload(Patcher::Downloaded &downloaded)
{
   SyncLocker locker(_lock);
   if(T._downloaded.elms())
   {
      Swap(downloaded, T._downloaded.last()); _downloaded.removeLast(); _files_left--;
      return true;
   }
   return false;
}
/******************************************************************************/
Long Patcher::progress()C
{
   SyncLocker locker(_lock);
   Long   size=_bytes_downloaded; REPA(_file_download)size+=_file_download[i].done();
   return size;
}
Long Patcher::filesSize()C
{
   SyncLocker locker(_lock);
   Long   size=_bytes_downloaded; REPA(_to_download)size+=_pak.file(_to_download[i]).data_size_compressed; REPA(_file_download){Int file_index=_file_download[i].index; if(InRange(file_index, _pak))size+=_pak.file(file_index).data_size_compressed;}
   return size;
}
Int Patcher::filesLeft()C {return _files_left;}
/******************************************************************************/
struct LocalFilePtr
{
 C Patcher::LocalFile *lf;
   Int                 org_index;

   void set(C CMemPtr<Patcher::LocalFile> &local_files, Int index) {T.lf=&local_files[index]; T.org_index=index;}

   static Int Compare(C LocalFilePtr &a, C LocalFilePtr &b) {return ComparePath(a.lf->full_name, b.lf->full_name);}
   static Int Compare(C LocalFilePtr &a, C Str          &b) {return ComparePath(a.lf->full_name, b              );}
};
static Bool Equal(C PakFile *pf, C Patcher::LocalFile *lf)
{
   if(pf && (pf->flag&PF_REMOVED))pf=null; // treat as if doesn't exist

   if(!pf && !lf)return true; // both don't exist = they're the same
   if( pf &&  lf) // both exist
   {
      // verify File/Dir for local files of SYSTEM_* type
      if(lf->type!=Patcher::LocalFile::PAK_FILE) // not PAK_FILE -> it's a SYSTEM_* type
         if(FlagOn(pf->flag, PF_STD_DIR) != (lf->type==Patcher::LocalFile::SYSTEM_DIR))
            return false;

      return  pf->data_size==lf->file_size // files have same sizes
         && ((pf->data_size                          ) ? !CompareFile(pf->modify_time_utc , lf->modify_time_utc) : true)  // if they are files (have data)            then their modification time must match
         && ((pf->data_xxHash64_32 && lf->xxHash64_32) ?              pf->data_xxHash64_32==lf->xxHash64_32      : true); // if they both have information about hash then it                      must match
   }
   return false; // one exists and other doesn't = they're different
}
static Bool Equal(C Patcher::LocalFile *a, C Patcher::LocalFile *b)
{
   if(!a && !b)return true; // both don't exist = they're the same
   if( a &&  b) // both exist
   {
      if(a->type!=b->type) // if types are different
         if(a->type!=Patcher::LocalFile::PAK_FILE && b->type!=Patcher::LocalFile::PAK_FILE) // and both are not PAK_FILE -> they're SYSTEM_* type
            return false; // they must match

      return  a->file_size==b->file_size // files have same sizes
         && ((a->file_size                    ) ? !CompareFile(a->modify_time_utc, b->modify_time_utc) : true)  // if they are files (have data)            then their modification time must match
         && ((a->xxHash64_32 && b->xxHash64_32) ?              a->xxHash64_32    ==b->xxHash64_32      : true); // if they both have information about hash then it                      must match
   }
   return false; // one exists and other doesn't = they're different
}
/******************************************************************************/
Bool Patcher::compare(C CMemPtr<LocalFile> &local_files, MemPtr<Int> local_remove, MemPtr<Int> server_download)
{
    local_remove  .clear();
   server_download.clear();
   if(C Pak *pak=index())
   {
      // sort 'local_files', thanks to this we can use 'binarySearch' later
      Memt<LocalFilePtr> lfp; FREPA(local_files)lfp.New().set(local_files, i); lfp.sort(LocalFilePtr::Compare);

      // remove duplicate names
      REPA(lfp)if(i)
      {
         LocalFilePtr &l=lfp[i], &l_1=lfp[i-1];
         if(EqualPath(l.lf->full_name, l_1.lf->full_name)) // if 2 files on the list have the same full name
            lfp.remove((l.org_index<l_1.org_index) ? i : i-1, true); // remove the one with smaller index, greater index means it was added later, and replaces the "older version with smaller index"
      }

      // check which files should be removed
      FREPA(lfp) // 'lfp' is sorted by path, go from start to remove folders first
      {
       C LocalFilePtr &l =lfp[i];
       C LocalFile    &lf=*l.lf;
       C PakFile      *pf=pak->find(lf.full_name);
         if(!Equal(pf, &lf)) // add to list of files for deletion
            if(!(lf.type==LocalFile::PAK_FILE && pf)) // if the local file is stored in a Pak, and there exists server version of that file, then don't mark this as to be removed, because it will already be listed in 'server_download' list, and when downloaded, it will replace the existing file (having PAK_FILE listed in both lists - remove and download, could cause errors if in 'PakUpdate' we would update it first - because of download list, and remove it later - because of remove list), test this only for PAK_FILE and not for System, because for System we do need to remove it first, even if it will be updated later via download replace (for example it was originally a folder, but we're replacing it with a file, so we do need to include this in remove list to remove the folder, only after element was removed, the new file can be created)
               local_remove.add(l.org_index);
      }

      // check which files should be downloaded
      FREPA(*pak)
      {
       C PakFile   &pf=pak->file(i);
       C LocalFile *lf=null; if(LocalFilePtr *lfp_elm=lfp.binaryFind(pak->fullName(pf), LocalFilePtr::Compare))lf=lfp_elm->lf;
         if(!Equal(&pf, lf))server_download.add(i); // add to list of files for download
      }
      return true;
   }
   return false;
}
/******************************************************************************/
void Patcher::Compare(C CMemPtr<LocalFile> &src_files, C CMemPtr<LocalFile> &dest_files, MemPtr<Int> dest_remove, MemPtr<Int> src_copy)
{
  dest_remove.clear();
   src_copy  .clear();

   // sort files, thanks to this we can use 'binarySearch' later
   Memt<LocalFilePtr> dest_ptrs; FREPA(dest_files)dest_ptrs.New().set(dest_files, i); dest_ptrs.sort(LocalFilePtr::Compare);
   Memt<LocalFilePtr>  src_ptrs; FREPA( src_files) src_ptrs.New().set( src_files, i);  src_ptrs.sort(LocalFilePtr::Compare);

   // remove duplicate names
   REPA(dest_ptrs)if(i)
   {
      LocalFilePtr &l=dest_ptrs[i], &l_1=dest_ptrs[i-1];
      if(EqualPath(l.lf->full_name, l_1.lf->full_name)) // if 2 files on the list have the same full name
         dest_ptrs.remove((l.org_index<l_1.org_index) ? i : i-1, true); // remove the one with smaller index, greater index means it was added later, and replaces the "older version with smaller index"
   }
   REPA(src_ptrs)if(i)
   {
      LocalFilePtr &l=src_ptrs[i], &l_1=src_ptrs[i-1];
      if(EqualPath(l.lf->full_name, l_1.lf->full_name)) // if 2 files on the list have the same full name
         src_ptrs.remove((l.org_index<l_1.org_index) ? i : i-1, true); // remove the one with smaller index, greater index means it was added later, and replaces the "older version with smaller index"
   }

   // check which files should be removed
   FREPA(dest_ptrs) // 'dest_ptrs' is sorted by path, go from start to remove folders first
   {
    C LocalFilePtr &dest_ptr= dest_ptrs[i];
    C LocalFile    &dest    =*dest_ptr.lf;
    C LocalFile    *src     =null; if(LocalFilePtr *src_ptr=src_ptrs.binaryFind(dest.full_name, LocalFilePtr::Compare))src=src_ptr->lf;
      if(!Equal(src, &dest)) // add to list of files for deletion
         if(!(dest.type==LocalFile::PAK_FILE && src)) // if the dest file is stored in a Pak, and there exists src version of that file, then don't mark this as to be removed, because it will already be listed in 'src_copy' list, and when copied, it will replace the existing file (having PAK_FILE listed in both lists - remove and copy, could cause errors if in 'PakUpdate' we would update it first - because of copy list, and remove it later - because of remove list), test this only for PAK_FILE and not for System, because for System we do need to remove it first, even if it will be updated later via copy replace (for example it was originally a folder, but we're replacing it with a file, so we do need to include this in remove list to remove the folder, only after element was removed, the new file can be created)
            dest_remove.add(dest_ptr.org_index);
   }

   // check which files should be copied
   FREPA(src_ptrs)
   {
    C LocalFilePtr &src_ptr= src_ptrs[i];
    C LocalFile    &src    =*src_ptr.lf;
    C LocalFile    *dest   =null; if(LocalFilePtr *dest_ptr=dest_ptrs.binaryFind(src.full_name, LocalFilePtr::Compare))dest=dest_ptr->lf;
      if(!Equal(dest, &src))src_copy.add(src_ptr.org_index); // add to list of files for copy
   }
}
/******************************************************************************/
}
/******************************************************************************/
