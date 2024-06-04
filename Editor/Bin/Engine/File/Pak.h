#pragma once
/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Pak' for archiving multiple data files into one "*.pak" file.

/******************************************************************************/
// PAK
/******************************************************************************/
enum PAK_FILE_FLAG {
    PF_REMOVED = 1 << 0,  // this file is marked as removed, this flag is useful if you're using multiple paks and you want to specify that a file which was in one Pak should now no longer exist (for example: having "Data.pak" - base Pak of data, "Patch.pak" - Pak storing updated files, "Data.pak" has "file.txt", in the "Patch.pak" we want to specify that "file.txt" should no longer exist, to do that, we include a dummy "file.txt" PakFile in the "Patch.pak" with PF_REMOVED enabled, now when loading "Data.pak" followed by "Patch.pak", "file.txt" from "Patch.pak" with PF_REMOVED flag will replace "file.txt" from "Data.pak" making it no longer accessible)
    PF_STD_DIR = 1 << 1,  // this file was originally created from a standard directory (not a file)
    PF_STD_LINK = 1 << 2, // this file was originally created from a symbolic link
};
struct PakFile // Single File stored in Pak
{
    CChar *name;               // file name  (this does not include the path, to get the full name please use 'Pak.fullName')
    Byte flag;                 // file flags (PAK_FILE_FLAG)
    COMPRESS_TYPE compression; // file compression algorithm
    Int parent,                // parent index            in 'Pak.file', -1=none
        children_offset,       // offset      of children in 'Pak.file' (this is the index of the first child in 'Pak.file' array)
        children_num;          // number      of children
    ULong data_offset;         // offset      of data     in Pak
    UInt data_size,            // size        of data
        data_size_compressed,  // size        of data after compression (if this file is not compressed, then this member is equal to 'data_size')
        data_xxHash64_32;      // xxHash64_32 of data, this member is set to 0 if PAK_SET_HASH was not enabled during Pak creation
    DateTime modify_time_utc;  // file modification time (UTC time zone)

    FSTD_TYPE type() C { return (flag & PF_STD_DIR) ? FSTD_DIR : (flag & PF_STD_LINK) ? FSTD_LINK
                                                                                      : FSTD_FILE; } // get type of the file
#if EE_PRIVATE
    PakFile &type(FSTD_TYPE type); // set type
    PakFile &reset();
#endif
};
/******************************************************************************/
enum PAK_FLAG // Pak Creation Flags
{
    PAK_SHORTEN = 1 << 0,  // when packing only one directory "xxx", files inside it won't be stored in "xxx\*.*" but in root "*.*"
    PAK_NO_DATA = 1 << 1,  // store only file names without their data
    PAK_NO_FILE = 1 << 2,  // don't create output Pak file, but only set Pak class members
    PAK_SET_HASH = 1 << 3, // calculate the hash member for each Pak file, if not enabled then the hash will be set to zero (hash calculation requires additional processing and slows down creation of Paks)
};
enum PAK_LOAD : Byte // Pak Load result
{
    PAK_LOAD_NOT_FOUND,           // source file was not found
    PAK_LOAD_NOT_PAK,             // source is not a Pak
    PAK_LOAD_UNSUPPORTED_VERSION, // source Pak version is not supported
    PAK_LOAD_INCOMPLETE_HEADER,   // source has incomplete header
    PAK_LOAD_INCOMPLETE_DATA,     // source has   complete header, however data is incomplete
    PAK_LOAD_OK,                  // Pak loaded OK
};
enum COMPRESS_MODE : Byte {
    COMPRESS_KEEP_ORIGINAL, // keep compression as from the original source file without making any changes
    COMPRESS_DISABLE,       // disable any compression (if source is compressed then it's decompressed and left uncompressed)
    COMPRESS_ENABLE,        // enable      compression (if source is compressed then it's decompressed first) file will be compressed as specified in the PAK create/update function
};
struct Pak // Set of Pak Files
{
    // load
    void operator=(C Str &name);                                                            // load Pak from file  , Exit  on fail
    Bool load(C Str &name, const_mem_addr Cipher *cipher = null);                           // load Pak from file  , false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
    Bool loadMem(const_mem_addr CPtr data, Int size, const_mem_addr Cipher *cipher = null); // load Pak from memory, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), 'data' must point to a constant memory address (only pointer is stored through which the data can be later accessed)

    PAK_LOAD loadEx(C Str &name, const_mem_addr Cipher *cipher = null, Long pak_offset = 0, Long *expected_size = null, Long *actual_size = null, MemPtr<DataRangeAbs> used_file_ranges = null);      // load Pak from file  , 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), 'pak_offset'=offset of PAK data inside the file                                                                     , 'expected_size'=expected size of the PAK file, 'actual_size'=actual size of the PAK file
    PAK_LOAD loadMemEx(const_mem_addr CPtr data, Int size, const_mem_addr Cipher *cipher = null, Long *expected_size = null, Long *actual_size = null, MemPtr<DataRangeAbs> used_file_ranges = null); // load Pak from memory, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), 'data' must point to a constant memory address (only pointer is stored through which the data can be later accessed), 'expected_size'=expected size of the PAK file, 'actual_size'=actual size of the PAK file

    // get
    Int rootFiles() C { return _root_files; }      // get number of files in root directory, they are stored first in the 'files' array
    Int totalFiles() C { return _files.elms(); }   // get number of all files
    C Mems<PakFile> &files() C { return _files; }  // get files array
    C PakFile &file(Int i) C { return _files[i]; } // get i-th file

    Long totalSize(C PakFile &file, Bool compressed = false) C;   // get      file          total size (size of the file and all of its children), 'compressed'=if return the compressed size 'PakFile.data_size_compressed' instead of 'PakFile.data_size'
    Long totalSize(Int i, Bool compressed = false) C;             // get i-th file          total size (size of the file and all of its children), 'compressed'=if return the compressed size 'PakFile.data_size_compressed' instead of 'PakFile.data_size'
    Str fullName(C PakFile &file) C;                              // get      file          full  name (path + name)
    Str fullName(Int i) C;                                        // get i-th file          full  name (path + name)
    C PakFile *find(CChar *name, Bool include_removed = true) C;  // find     file from its full  name (path + name), 'include_removed'=return also files which are marked as removed (have PF_REMOVED flag enabled), null on fail
    C PakFile *find(CChar8 *name, Bool include_removed = true) C; // find     file from its full  name (path + name), 'include_removed'=return also files which are marked as removed (have PF_REMOVED flag enabled), null on fail
    C PakFile *find(C UID &id, Bool include_removed = true) C;    // find     file from its ID                      , 'include_removed'=return also files which are marked as removed (have PF_REMOVED flag enabled), null on fail

    C Str &pakFileName() C { return _file_name; } // get name of the file where the Pak is located. If this Pak is stored inside another file, then this method will return the file name of the top most parent
    Pak &pakFileName(C Str &name);                // manually adjust the Pak file name in case the file was moved

    // create from files and save to disk
    Bool create(C Str &file, C Str &pak_name = S, UInt flag = PAK_SHORTEN, Cipher *dest_cipher = null, Cipher *src_cipher = null, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Bool (*Filter)(C Str &name) = null, COMPRESS_MODE (*CompressMode)(C Str &name) = null, Str *error_message = null, PakProgress *progress = null);           // create Pak, 'file '=single  file/directory      , 'pak_name'=Pak file name to save to, 'flag'=PAK_FLAG                                                                 , 'compression_level'=0..CompressionLevels(compress) (0=fastest/worst, ..=slowest/best), false on fail, 'Filter'=optional pointer to custom callback function which will receive information about encountered files and folders (their name) for which it should return true if the element should be included in the Pak and false if not included (if 'Filter' is null then all encountered files/folders will be included), 'error_message'=will contain a message what went wrong upon error, 'progress'=optional parameter allowing to control creation from secondary thread
    Bool create(C CMemPtr<Str> &files, C Str &pak_name = S, UInt flag = PAK_SHORTEN, Cipher *dest_cipher = null, Cipher *src_cipher = null, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Bool (*Filter)(C Str &name) = null, COMPRESS_MODE (*CompressMode)(C Str &name) = null, Str *error_message = null, PakProgress *progress = null); // create Pak, 'files'=list of file/directories    , 'pak_name'=Pak file name to save to, 'flag'=PAK_FLAG, all elements listed in 'files' must be located in the same path, 'compression_level'=0..CompressionLevels(compress) (0=fastest/worst, ..=slowest/best), false on fail, 'Filter'=optional pointer to custom callback function which will receive information about encountered files and folders (their name) for which it should return true if the element should be included in the Pak and false if not included (if 'Filter' is null then all encountered files/folders will be included), 'error_message'=will contain a message what went wrong upon error, 'progress'=optional parameter allowing to control creation from secondary thread
    Bool create(C CMemPtr<PakNode> &files, C Str &pak_name, UInt flag = 0, Cipher *dest_cipher = null, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Str *error_message = null, PakProgress *progress = null);                                                                                                                             // create Pak, 'files'=list of file/directory nodes, 'pak_name'=Pak file name to save to, 'flag'=PAK_FLAG                                                                 , 'compression_level'=0..CompressionLevels(compress) (0=fastest/worst, ..=slowest/best), false on fail                                                                                                                                                                                                                                                                                                                        , 'error_message'=will contain a message what went wrong upon error, 'progress'=optional parameter allowing to control creation from secondary thread
    Bool create(C CMemPtr<PakFileData> &files, C Str &pak_name, UInt flag = 0, Cipher *dest_cipher = null, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Str *error_message = null, PakProgress *progress = null);                                                                                                                         // create Pak, 'files'=list of file/directory data , 'pak_name'=Pak file name to save to, 'flag'=PAK_FLAG                                                                 , 'compression_level'=0..CompressionLevels(compress) (0=fastest/worst, ..=slowest/best), false on fail                                                                                                                                                                                                                                                                                                                        , 'error_message'=will contain a message what went wrong upon error, 'progress'=optional parameter allowing to control creation from secondary thread
#if EE_PRIVATE
    Bool create(C CMemPtr<PakNode> &files, C Str &pak_name, UInt flag, Cipher *dest_cipher, COMPRESS_TYPE compress, Int compression_level, Str *error_message, PakProgress *progress, PakInPlace *in_place, MemPtr<DataRangeAbs> used_file_ranges, PakPostHeader *post_header);
    Bool create(C CMemPtr<PakFileData> &files, C Str &pak_name, UInt flag, Cipher *dest_cipher, COMPRESS_TYPE compress, Int compression_level, Str *error_message, PakProgress *progress, PakInPlace *in_place, MemPtr<DataRangeAbs> used_file_ranges, PakPostHeader *post_header);
    Bool create(C Mems<C PakFileData *> &files, C Str &pak_name, UInt flag, Cipher *dest_cipher, COMPRESS_TYPE compress, Int compression_level, Str *error_message, PakProgress *progress, PakInPlace *in_place, MemPtr<DataRangeAbs> used_file_ranges, PakPostHeader *post_header);
    Bool savePreHeader(File &f, Long header_data_pos = LONG_MIN) C; // save Pak header     , false on fail
    Bool saveHeaderData(File &f) C;                                 // save Pak header data, false on fail
    Int headerDataSize() C;
    Bool update(C CMemPtr<PakFileData> &changes, C Str &pak_name, UInt flag = 0, Cipher *dest_cipher = null, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Str *error_message = null, PakProgress *progress = null, PakInPlace *in_place = null, PakPostHeader *post_header = null);
    Bool replace(Memb<DataRangeAbs> &used_file_ranges, C CMemPtr<PakFileData> &new_files, UInt flag = 0, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Str *error_message = null, PakProgress *progress = null, PakPostHeader *post_header = null);        // replace 'pak_name' Pak with        'new_files' in-place directly within the 'pak_name' file without creating any additional files                               , 'compression_level'=0..CompressionLevels(compress) (0=fastest/worst, ..=slowest/best), 'compress'=compression algorithm for the 'changes' (files from 'pak_name' will preserve their original compression), this operates on existing Pak, adjusting it without creating new files, for example updating 1GB Pak with 1MB file will perform 1MB+ operations, it   will perform as few     Disk operations as possible, however created Pak will not be fully optimized, and may contain some useless data, if update failed for some reason then original Pak will remain unmodified                                        , this function replaces  original files with 'new_files'
    Bool replaceInPlace(Memb<DataRangeAbs> &used_file_ranges, C CMemPtr<PakFileData> &new_files, UInt flag = 0, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Str *error_message = null, PakProgress *progress = null, PakPostHeader *post_header = null); // replace 'pak_name' Pak with        'new_files' in-place directly within the 'pak_name' file without creating any additional files                               , 'compression_level'=0..CompressionLevels(compress) (0=fastest/worst, ..=slowest/best), 'compress'=compression algorithm for the 'changes' (files from 'pak_name' will preserve their original compression), this operates on existing Pak, adjusting it without creating new files, for example updating 1GB Pak with 1MB file will perform 1MB+ operations, it   will perform as few     Disk operations as possible, however created Pak will not be fully optimized, and may contain some useless data, if update failed for some reason then original Pak will remain unmodified                                        , this function replaces  original files with 'new_files'
#endif

    // io
    Bool saveHeader(File &f) C; // save Pak header (information about files, without their data), false on fail

    Pak &del(); // delete manually
    ~Pak() { del(); }
    Pak();

#if !EE_PRIVATE
  private:
#endif
    UInt _root_files;
    ULong _data_offset;
    Mems<Byte> _data_decompressed;
    Mems<Char> _names;
    Mems<PakFile> _files;
    Bool _cipher_per_file;
    Byte _file_type;
    Int _file_cipher_offset;
    Cipher *_file_cipher;
    Str _file_name;
    CPtr _data;
#if EE_PRIVATE
    void zero();
    PAK_LOAD loadHeader(File &f, Long *expected_size = null, Long *actual_size = null, MemPtr<DataRangeAbs> used_file_ranges = null); // load just the header, access to data will not be available by using this method
#endif
    NO_COPY_CONSTRUCTOR(Pak);
};
/******************************************************************************/
// PAK SET
/******************************************************************************/
struct PaksFile // Single file stored in PakSet
{
    Pak *pak;            // Pak to which file belongs to
    C PakFile *file;     // pointer to Pak file
    Int children_offset, // offset of children in 'PakSet.file'
        children_num;    // number of children

    Str fullName() C; // get file full name (path + name)

    PaksFile() { Zero(T); }
};
/******************************************************************************/
struct PakSet // set of Pak's combined together with all their PakFile's combined in one database (PakFile's with PF_REMOVED aren't included in PakSet)
{
    // get
    Int rootFiles() C { return _root_files; }       // get number of files in root directory, they are stored first in the 'file' array
    Int totalFiles() C { return _files.elms(); }    // get number of all files
    C Memc<PaksFile> &files() C { return _files; }  // get files array
    C PaksFile &file(Int i) C { return _files[i]; } // get i-th file

    Long totalSize(C PaksFile &file, Bool compressed = false) C; // get      file          total size (size of the file and all of its children), 'compressed'=if return the compressed size 'PakFile.data_size_compressed' instead of 'PakFile.data_size'
    Long totalSize(Int i, Bool compressed = false) C;            // get i-th file          total size (size of the file and all of its children), 'compressed'=if return the compressed size 'PakFile.data_size_compressed' instead of 'PakFile.data_size'
    Str fullName(C PaksFile &file) C;                            // get      file          full  name (path + name)
    Str fullName(Int i) C;                                       // get i-th file          full  name (path + name)
    C PaksFile *find(CChar *name) C;                             // get      file from its full  name (path + name)
    C PaksFile *find(CChar8 *name) C;                            // get      file from its full  name (path + name)
    C PaksFile *find(C UID &id) C;                               // get      file from its ID

    // operations
#if EE_PRIVATE
    Bool addTry(C Str &name, const_mem_addr Cipher *cipher, Bool auto_rebuild, Long pak_offset); // add Pak from file, 'auto_rebuild'=if automatically call the 'rebuild' method, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), 'pak_offset'=offset of PAK data inside the file
#endif
    Bool addTry(C Str &name, const_mem_addr Cipher *cipher = null, Bool auto_rebuild = true);                           // add Pak from file  , 'auto_rebuild'=if automatically call the 'rebuild' method, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
    PakSet &add(C Str &name, const_mem_addr Cipher *cipher = null, Bool auto_rebuild = true);                           // add Pak from file  , 'auto_rebuild'=if automatically call the 'rebuild' method, Exit  on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed)
    Bool addMemTry(const_mem_addr CPtr data, Int size, const_mem_addr Cipher *cipher = null, Bool auto_rebuild = true); // add Pak from memory, 'auto_rebuild'=if automatically call the 'rebuild' method, false on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), 'data' must point to a constant memory address (only pointer is stored through which the data can be later accessed)
    PakSet &addMem(const_mem_addr CPtr data, Int size, const_mem_addr Cipher *cipher = null, Bool auto_rebuild = true); // add Pak from memory, 'auto_rebuild'=if automatically call the 'rebuild' method, Exit  on fail, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), 'data' must point to a constant memory address (only pointer is stored through which the data can be later accessed)
    Bool remove(C Str &name);                                                                                           // remove previously added "Pak from file"  , true is returned if Pak was found and removed, false if it was not found, this method always calls 'rebuild' upon success
    Bool removeMem(CPtr data);                                                                                          // remove previously added "Pak from memory", true is returned if Pak was found and removed, false if it was not found, this method always calls 'rebuild' upon success
    void rebuild();                                                                                                     // rebuild Pak files database from loaded Pak's, this needs to be called once after adding new Pak's

    Int paks() C { return _paks.elms(); } // get number of loaded Paks
    Str pakName(Int i) C;                 // get name   of i-th   Pak

    PakSet &del(); // delete manually
    PakSet();

#if !EE_PRIVATE
  private:
#endif
    struct Src : Pak {
        Str name;
        Long pak_offset;
    };
    UInt _root_files;
    SyncLock _lock;
    Meml<Src> _paks;
    Memc<PaksFile> _files;
} extern Paks; // this is the global Pak set which is automatically used when opening files
/******************************************************************************/
// PAK CREATE
/******************************************************************************/
struct PakProgress // class that can be optionally passed to Pak creation functions to get extra control over that process (for example on one thread you can call Pak create function with this class object as the parameter, then on secondary thread you can access that object and read the 'progress' or enable 'stop' to break Pak creation)
{
    Bool stop;    // set this to 'true' on secondary thread to break Pak creation, default=false
    Flt progress; // current progress of Pak creation, 0..1, Pak creation functions will modify this value according to the progress, default=0

    PakProgress &reset() {
        stop = false;
        progress = 0;
        return T;
    } // reset members to initial values
    PakProgress() { reset(); }
#if EE_PRIVATE
    Bool wantStop(Str *error_message = null) C;
#endif
};
/******************************************************************************/
struct DataSource {
    enum TYPE : Byte {
        NONE,     // data is empty
        NAME,     // data comes from a file named 'name' which can be accessed using standard IO functions, such as 'File.read'    and 'FileInfo'
        STD,      // data comes from a file named 'name' which can be accessed using standard IO functions, such as 'File.readStd' and 'FileInfoSystem'
        FILE,     // data comes from a pointer to 'File' object, which should be already opened for reading
        PAK_FILE, // data comes from a pointer to 'PakFile' and 'Pak' objects
        MEM,      // data comes from specified memory address and its size
    };

    TYPE type;
    Str name; // used when "type==NAME || type==STD"
    union {
        struct {
            File *file;
        }; // used when "type==FILE"
        struct {
            C PakFile *pak_file;
            C Pak *pak;
        }; // used when "type==PAK_FILE"
        struct {
            CPtr memory;
            Long memory_size;
        };
    }; // used when "type==MEM"

    DataSource &zero() {
        type = NONE;
        return T;
    } // set NONE     type
    DataSource &set(C Str &name) {
        type = NAME;
        T.name = name;
        return T;
    } // set NAME     type
    DataSource &setStd(C Str &name) {
        type = STD;
        T.name = name;
        return T;
    } // set STD      type
    DataSource &set(File &file) {
        type = FILE;
        T.file = &file;
        return T;
    } // set FILE     type
    DataSource &set(C PakFile &pak_file, C Pak &pak) {
        type = PAK_FILE;
        T.pak_file = &pak_file;
        T.pak = &pak;
        return T;
    } // set PAK_FILE type
    DataSource &set(CPtr memory, Long size) {
        type = MEM;
        T.memory = memory;
        T.memory_size = size;
        return T;
    } // set MEM      type

    File *open(File &temp) C;
#if EE_PRIVATE
    File *openRaw(File &temp) C;
#endif
    Long size() C;
    Long sizeCompressed() C;
    Str srcName() C;
    FSTD_TYPE fstdType() C;

    DataSource() { zero(); }
};
/******************************************************************************/
struct PakFileData // Pak File Data, used for creating or updating Pak's from custom files
{
    enum MODE : Byte {
        REPLACE,      // remove any previous information about this file in Pak,       include     new information about this file specifying that it    now exists, with new data taken from 'file' member    , use this mode for creating new files or updating previous files
        REMOVE,       // remove any previous information about this file in Pak, don't include any new information about this file                                                                             , use this mode for completely removing any information about previous file
        MARK_REMOVED, // remove any previous information about this file in Pak,       include     new information about this file specifying that it is now removed, by enabling PF_REMOVED flag for this file, use this mode for removing previous file and leaving information that it now is removed (see comments for PF_REMOVED flag to get more information)
    };

    FSTD_TYPE type;              // type of the file, can be set to FSTD_FILE, FSTD_DIR or FSTD_LINK, used only if "mode==REPLACE" or "mode==MARK_REMOVED"
    MODE mode;                   // mode specifying how this file should be treated (whether it should be replaced, removed completely, or marked as removed)
    COMPRESS_MODE compress_mode; // specify compression mode for this file
    COMPRESS_TYPE compressed;    // if source file is already compressed then specify here its compression algorithm, used only if "mode==REPLACE"
    Long decompressed_size;      // if source file is already compressed then specify here its original decompressed size (otherwise leave to -1), used only if "mode==REPLACE"
    Str name;                    // target name in the destination Pak (can include folders), for example "Folder/file.ext"
    DataSource data;             // file data, used only if "mode==REPLACE"
    UInt xxHash64_32;            // file hash, if you know it already then you can set it manually to save processing time, in other case you can leave it to 0, used only if "mode==REPLACE"
    DateTime modify_time_utc;    // file modification time (UTC time zone)

    PakFileData() {
        type = FSTD_NONE;
        mode = REPLACE;
        compress_mode = COMPRESS_ENABLE;
        compressed = COMPRESS_NONE;
        decompressed_size = -1;
        xxHash64_32 = 0;
        modify_time_utc.zero();
    }
};
/******************************************************************************/
struct PakNode // Pak File Node, used for creating Pak's from custom files
{
    FSTD_TYPE type;              // type of the file, can be set to FSTD_FILE, FSTD_DIR or FSTD_LINK
    Bool exists;                 // if the file exists, if it's set to false then file will be created with PF_REMOVED flag enabled (see comments for PF_REMOVED flag to get more information)
    COMPRESS_MODE compress_mode; // specify compression mode for this file
    COMPRESS_TYPE compressed;    // if source file is already compressed then specify here its compression algorithm
    Long decompressed_size;      // if source file is already compressed then specify here its original decompressed size (otherwise leave to -1)
    Str name;                    // target name in the destination Pak, for example "file.ext" (can't include folders, for example can't be "Folder/file.ext")
    DataSource data;             // file data
    UInt xxHash64_32;            // file hash, if you know it already then you can set it manually to save processing time, in other case you can leave it to 0
    DateTime modify_time_utc;    // file modification time (UTC time zone)
    Memb<PakNode> children;      // sub-elements

#if EE_PRIVATE
    PakNode &newSetFolder(C Str &name, C DateTime &modify_time_utc) // call this only for objects that were just created (right after constructor called)
    {
        T.type = FSTD_DIR;
        T.name = name;
        T.modify_time_utc = modify_time_utc;
        return T;
    }
    PakNode &setRemoved(C Str &name, C DateTime &modify_time_utc, FSTD_TYPE type) {
        T.type = type;
        T.exists = false;
        T.compress_mode = COMPRESS_ENABLE;
        T.compressed = COMPRESS_NONE;
        T.decompressed_size = -1;
        T.name = name;
        T.data.zero();
        T.xxHash64_32 = 0;
        T.modify_time_utc = modify_time_utc;
        children.del(); // remove all existing children (they are not needed if this node is marked as removed)
        return T;
    }
    PakNode &set(C Str &name, C PakFileData &pfd) {
        T.type = pfd.type;
        T.exists = true;
        T.compress_mode = pfd.compress_mode;
        T.compressed = pfd.compressed;
        T.decompressed_size = pfd.decompressed_size;
        T.name = name;
        T.data = pfd.data;
        T.xxHash64_32 = pfd.xxHash64_32;
        T.modify_time_utc = pfd.modify_time_utc;
        return T;
    }
#endif

    PakNode() {
        type = FSTD_NONE;
        exists = true;
        compress_mode = COMPRESS_ENABLE;
        compressed = COMPRESS_NONE;
        decompressed_size = -1;
        xxHash64_32 = 0;
        modify_time_utc.zero();
    }
};
/******************************************************************************/
// MAIN
/******************************************************************************/
// Create Pak
inline Bool PakCreate(C Str &file, C Str &pak_name = S, UInt flag = PAK_SHORTEN, Cipher *dest_cipher = null, Cipher *src_cipher = null, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Bool (*Filter)(C Str &name) = null, COMPRESS_MODE (*CompressMode)(C Str &name) = null, Str *error_message = null, PakProgress *progress = null) { return Pak().create(file, pak_name, flag, dest_cipher, src_cipher, compress, compression_level, Filter, CompressMode, error_message, progress); }
inline Bool PakCreate(C CMemPtr<Str> &files, C Str &pak_name = S, UInt flag = PAK_SHORTEN, Cipher *dest_cipher = null, Cipher *src_cipher = null, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Bool (*Filter)(C Str &name) = null, COMPRESS_MODE (*CompressMode)(C Str &name) = null, Str *error_message = null, PakProgress *progress = null) { return Pak().create(files, pak_name, flag, dest_cipher, src_cipher, compress, compression_level, Filter, CompressMode, error_message, progress); }
inline Bool PakCreate(C CMemPtr<PakNode> &files, C Str &pak_name, UInt flag = 0, Cipher *dest_cipher = null, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Str *error_message = null, PakProgress *progress = null) { return Pak().create(files, pak_name, flag, dest_cipher, compress, compression_level, error_message, progress); }
inline Bool PakCreate(C CMemPtr<PakFileData> &files, C Str &pak_name, UInt flag = 0, Cipher *dest_cipher = null, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Str *error_message = null, PakProgress *progress = null) { return Pak().create(files, pak_name, flag, dest_cipher, compress, compression_level, error_message, progress); }

// Update Pak
Bool PakUpdate(Pak &pak, C CMemPtr<PakFileData> &changes, C Str &pak_name, UInt flag = 0, Cipher *dest_cipher = null, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Str *error_message = null, PakProgress *progress = null);                               // update  'pak'      Pak by applying 'changes'   and saving the new Pak into  'pak_name' (if dest file is same as source file, then a temporary file will be used), 'compression_level'=0..CompressionLevels(compress) (0=fastest/worst, ..=slowest/best), 'compress'=compression algorithm for the 'changes' (files from 'pak'      will preserve their original compression), this completely recreates the Pak fully rewriting its data            , for example updating 1GB Pak with 1MB file will perform 1GB+ operations, this will result in a lot of Disk operations            , however created Pak will     be fully optimized, and as small as possible         , if update failed for some reason then original Pak will remain unmodified and any temporary files will be deleted, this function preserves original files and only modifies those listed in 'changes'
Bool PakUpdateInPlace(C CMemPtr<PakFileData> &changes, C Str &pak_name, UInt flag = 0, Cipher *cipher = null, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Str *error_message = null, PakProgress *progress = null);                                       // update  'pak_name' Pak by applying 'changes'   in-place directly within the 'pak_name' file without creating any additional files                               , 'compression_level'=0..CompressionLevels(compress) (0=fastest/worst, ..=slowest/best), 'compress'=compression algorithm for the 'changes' (files from 'pak_name' will preserve their original compression), this operates on existing Pak, adjusting it without creating new files, for example updating 1GB Pak with 1MB file will perform 1MB+ operations, it   will perform as few     Disk operations as possible, however created Pak will not be fully optimized, and may contain some useless data, if update failed for some reason then original Pak will remain unmodified                                        , this function preserves original files and only modifies those listed in 'changes'
Bool PakReplaceInPlace(C CMemPtr<PakFileData> &new_files, C Str &pak_name, UInt flag = 0, Cipher *cipher = null, COMPRESS_TYPE compress = COMPRESS_NONE, Int compression_level = 9, Str *error_message = null, PakProgress *progress = null, PakPostHeader *post_header = null); // replace 'pak_name' Pak with        'new_files' in-place directly within the 'pak_name' file without creating any additional files                               , 'compression_level'=0..CompressionLevels(compress) (0=fastest/worst, ..=slowest/best), 'compress'=compression algorithm for the 'changes' (files from 'pak_name' will preserve their original compression), this operates on existing Pak, adjusting it without creating new files, for example updating 1GB Pak with 1MB file will perform 1MB+ operations, it   will perform as few     Disk operations as possible, however created Pak will not be fully optimized, and may contain some useless data, if update failed for some reason then original Pak will remain unmodified                                        , this function replaces  original files with 'new_files'

// Compare Pak
Bool PakEqual(MemPtr<PakFileData> files, C Pak &pak);                                      // if 'files' are the same as the ones in 'pak'          (this will verify if all files from 'files' are of the same size, modification time and hash (if available) as those from 'pak'         , Warning: folders may get ignored)
Bool CPakEqual(C CMemPtr<PakFileData> &files, C Pak &pak);                                 // if 'files' are the same as the ones in 'pak'          (this will verify if all files from 'files' are of the same size, modification time and hash (if available) as those from 'pak'         , Warning: folders may get ignored)
Bool PakEqual(MemPtr<PakFileData> files, C Str &pak_name, Cipher *pak_cipher = null);      // if 'files' are the same as the ones in 'pak_name' Pak (this will verify if all files from 'files' are of the same size, modification time and hash (if available) as those from 'pak_name' Pak, Warning: folders may get ignored)
Bool CPakEqual(C CMemPtr<PakFileData> &files, C Str &pak_name, Cipher *pak_cipher = null); // if 'files' are the same as the ones in 'pak_name' Pak (this will verify if all files from 'files' are of the same size, modification time and hash (if available) as those from 'pak_name' Pak, Warning: folders may get ignored)

inline Int Elms(C Pak &pak) { return pak.totalFiles(); }
inline Int Elms(C PakSet &paks) { return paks.totalFiles(); }

#if EE_PRIVATE
// Emoji Pak
extern Pak *EmojiPak;
Bool AddEmojiPak(C Str &name, Bool auto_rebuild = true); // add pak containing emojis, false on fail

Bool Equal(C PakFile *a, C PakFile *b);        // test if both PakFile's are of the same version
Bool Equal(C PakFileData *pfd, C PakFile *pf); // test if both PakFile's are of the same version
Bool Equal(PakFileData *pfd, C PakFile *pf);   // test if both PakFile's are of the same version

struct PakInPlace {
    Memb<DataRangeAbs> used_file_ranges;
    Pak &src_pak;

    PakInPlace(Pak &src_pak) : src_pak(src_pak) {}
};
struct PakPostHeader {
    virtual void save(File &f, C Pak &pak) {}
};
#endif
/******************************************************************************/
