/******************************************************************************

   Use 'MemStats' to access information about system memory usage.

   Use 'Cipher' to handle data encryption.

   Use memory functions to handle:
      -Allocate
      -Free
      -Reallocate
      -Zero
      -Set
      -Copy
      -Swap
      -Compare
      -New
      -Delete

/******************************************************************************/
// MEMORY STATUS
/******************************************************************************/
struct MemStats // System Memory Status
{
   Byte usage                 ; // percentage memory usage, 0..100, 0=no memory usage, 100=full memory usage
   Long avail_phys, total_phys, // available and total physical memory (in bytes)
        avail_page, total_page, // available and total paged    memory (in bytes)
        avail_virt, total_virt; // available and total virtual  memory (in bytes)

   Bool get(); // get values from system information, false on fail
};
/******************************************************************************/
// ALLOCATE RAW MEMORY
/******************************************************************************/
Ptr Alloc(ULong size); // allocate memory, Exit on fail, !! combine only with 'Free' !!, sample usage: "Alloc(100);" will allocate raw memory of 100 bytes
Ptr Alloc( Long size); // allocate memory, Exit on fail, !! combine only with 'Free' !!, sample usage: "Alloc(100);" will allocate raw memory of 100 bytes
Ptr Alloc(UInt  size); // allocate memory, Exit on fail, !! combine only with 'Free' !!, sample usage: "Alloc(100);" will allocate raw memory of 100 bytes
Ptr Alloc( Int  size); // allocate memory, Exit on fail, !! combine only with 'Free' !!, sample usage: "Alloc(100);" will allocate raw memory of 100 bytes

       Ptr AllocZero(ULong size);                                 // allocate and zero memory, Exit on fail, !! combine only with 'Free' !!, sample usage: "AllocZero(100);" will allocate raw memory of 100 zeroed bytes
       Ptr AllocZero( Long size);                                 // allocate and zero memory, Exit on fail, !! combine only with 'Free' !!, sample usage: "AllocZero(100);" will allocate raw memory of 100 zeroed bytes
inline Ptr AllocZero(UInt  size) {return AllocZero((ULong)size);} // allocate and zero memory, Exit on fail, !! combine only with 'Free' !!, sample usage: "AllocZero(100);" will allocate raw memory of 100 zeroed bytes
inline Ptr AllocZero( Int  size) {return AllocZero(( Long)size);} // allocate and zero memory, Exit on fail, !! combine only with 'Free' !!, sample usage: "AllocZero(100);" will allocate raw memory of 100 zeroed bytes

T1(TYPE) TYPE*  Alloc    (             Long elms  ) {return (TYPE*)Alloc          (elms*SIZEL (TYPE));} // allocate          'elms' elements of 'type', Exit on fail, !! combine only with 'Free' !!, sample usage: "           Alloc    <Int> (100);" will allocate raw memory of 100        Int elements
T1(TYPE) TYPE*  Alloc    (             UInt elms=1) {return (TYPE*)Alloc          (elms*SIZEUL(TYPE));} // allocate          'elms' elements of 'type', Exit on fail, !! combine only with 'Free' !!, sample usage: "           Alloc    <Int> (100);" will allocate raw memory of 100        Int elements
T1(TYPE) TYPE*  Alloc    (              Int elms  ) {return (TYPE*)Alloc          (elms*SIZEL (TYPE));} // allocate          'elms' elements of 'type', Exit on fail, !! combine only with 'Free' !!, sample usage: "           Alloc    <Int> (100);" will allocate raw memory of 100        Int elements
T1(TYPE) TYPE*  AllocZero(             Long elms  ) {return (TYPE*)AllocZero      (elms*SIZEL (TYPE));} // allocate and zero 'elms' elements of 'type', Exit on fail, !! combine only with 'Free' !!, sample usage: "           AllocZero<Int> (100);" will allocate raw memory of 100 zeroed Int elements
T1(TYPE) TYPE*  AllocZero(             UInt elms=1) {return (TYPE*)AllocZero      (elms*SIZEUL(TYPE));} // allocate and zero 'elms' elements of 'type', Exit on fail, !! combine only with 'Free' !!, sample usage: "           AllocZero<Int> (100);" will allocate raw memory of 100 zeroed Int elements
T1(TYPE) TYPE*  AllocZero(              Int elms  ) {return (TYPE*)AllocZero      (elms*SIZEL (TYPE));} // allocate and zero 'elms' elements of 'type', Exit on fail, !! combine only with 'Free' !!, sample usage: "           AllocZero<Int> (100);" will allocate raw memory of 100 zeroed Int elements
T1(TYPE) TYPE*& Alloc    (TYPE* &data, Long elms  ) {         data=Alloc    <TYPE>(elms); return data;} // allocate          memory for 'elms' TYPE's , Exit on fail, !! combine only with 'Free' !!, sample usage: "Int *data; Alloc    (data, 100);" will allocate raw memory of 100        Int elements
T1(TYPE) TYPE*& Alloc    (TYPE* &data, UInt elms=1) {         data=Alloc    <TYPE>(elms); return data;} // allocate          memory for 'elms' TYPE's , Exit on fail, !! combine only with 'Free' !!, sample usage: "Int *data; Alloc    (data, 100);" will allocate raw memory of 100        Int elements
T1(TYPE) TYPE*& Alloc    (TYPE* &data,  Int elms  ) {         data=Alloc    <TYPE>(elms); return data;} // allocate          memory for 'elms' TYPE's , Exit on fail, !! combine only with 'Free' !!, sample usage: "Int *data; Alloc    (data, 100);" will allocate raw memory of 100        Int elements
T1(TYPE) TYPE*& AllocZero(TYPE* &data, Long elms  ) {         data=AllocZero<TYPE>(elms); return data;} // allocate and zero memory for 'elms' TYPE's , Exit on fail, !! combine only with 'Free' !!, sample usage: "Int *data; AllocZero(data, 100);" will allocate raw memory of 100 zeroed Int elements
T1(TYPE) TYPE*& AllocZero(TYPE* &data, UInt elms=1) {         data=AllocZero<TYPE>(elms); return data;} // allocate and zero memory for 'elms' TYPE's , Exit on fail, !! combine only with 'Free' !!, sample usage: "Int *data; AllocZero(data, 100);" will allocate raw memory of 100 zeroed Int elements
T1(TYPE) TYPE*& AllocZero(TYPE* &data,  Int elms  ) {         data=AllocZero<TYPE>(elms); return data;} // allocate and zero memory for 'elms' TYPE's , Exit on fail, !! combine only with 'Free' !!, sample usage: "Int *data; AllocZero(data, 100);" will allocate raw memory of 100 zeroed Int elements
/******************************************************************************/
// FREE RAW MEMORY
/******************************************************************************/
         void   Free(Ptr   &data);                                   // free memory and set pointer to null !! combine only with 'Alloc' !!
T1(TYPE) TYPE*& Free(TYPE* &data) {Free(*(Ptr*)&data); return data;} // free memory and set pointer to null !! combine only with 'Alloc' !!
/******************************************************************************/
// REALLOCATE MEMORY
/******************************************************************************/
T1(TYPE) void Realloc    (TYPE* &data, Int elms_new, Int elms_old); // reallocate memory without losing data with 'elms_new' and 'elms_old'                       , Exit on fail !! combine only with 'Alloc' !!
T1(TYPE) void ReallocZero(TYPE* &data, Int elms_new, Int elms_old); // reallocate memory without losing data with 'elms_new' and 'elms_old', and zero new elements, Exit on fail !! combine only with 'Alloc' !!
/******************************************************************************/
// ZERO MEMORY
/******************************************************************************/
         void Zero (Ptr   data, ULong size);                                   // zero                 memory
         void Zero (Ptr   data,  Long size);                                   // zero                 memory
         void Zero (Ptr   data, UInt  size);                                   // zero                 memory
         void Zero (Ptr   data,  Int  size);                                   // zero                 memory
T1(TYPE) void Zero (TYPE &elm             ) {memset(&elm , 0,   SIZE (TYPE));} // zero        element  memory
T1(TYPE) void ZeroN(TYPE *data,  Int  elms) {Zero  ( data, elms*SIZEL(TYPE));} // zero 'elms' elements memory
/******************************************************************************/
// SET MEMORY
/******************************************************************************/
         void SetMem (Ptr   data, Byte value, ULong size);                                          // set                 memory to Byte value
         void SetMem (Ptr   data, Byte value,  Long size);                                          // set                 memory to Byte value
         void SetMem (Ptr   data, Byte value, UInt  size);                                          // set                 memory to Byte value
         void SetMem (Ptr   data, Byte value,  Int  size);                                          // set                 memory to Byte value
T1(TYPE) void SetMem (TYPE &elm , Byte value            ) {memset(&elm , value,      SIZE (TYPE));} // set 'elm'           memory to Byte value
T1(TYPE) void SetMemN(TYPE *data, Byte value,  Int  elms) {SetMem( data, value, elms*SIZEL(TYPE));} // set 'elms' elements memory to Byte value
/******************************************************************************/
// COPY MEMORY
/******************************************************************************/
#if EE_PRIVATE
         void Copy (Ptr   dest,   CPtr  src, IntPtr dest_size, IntPtr src_size);                  // copy 'src'  memory  to 'dest', if 'src' is null then 'dest' will be set to zero, if 'src_size' is smaller than 'dest_size' then remaining bytes will be zeroed
#endif
         void Copy (Ptr   dest,   CPtr  src, ULong size);                                         // copy 'src'  memory  to 'dest', if 'src' is null then 'dest' will be set to zero
         void Copy (Ptr   dest,   CPtr  src,  Long size);                                         // copy 'src'  memory  to 'dest', if 'src' is null then 'dest' will be set to zero
         void Copy (Ptr   dest,   CPtr  src, UInt  size);                                         // copy 'src'  memory  to 'dest', if 'src' is null then 'dest' will be set to zero
         void Copy (Ptr   dest,   CPtr  src,  Int  size);                                         // copy 'src'  memory  to 'dest', if 'src' is null then 'dest' will be set to zero
T1(TYPE) void Copy (TYPE &dest, C TYPE &src            ) {memcpy(&dest, &src,      SIZE (TYPE));} // copy 'src'  element to 'dest'
T1(TYPE) void CopyN(TYPE *dest, C TYPE *src,  Int  elms) {Copy  ( dest,  src, elms*SIZEL(TYPE));} // copy 'elms' elements
/******************************************************************************/
// MEMORY ADVANCED
/******************************************************************************/
  inline void ZeroFast (Ptr   data, UIntPtr size) {memset( data, 0,      size      );} // !! this does not check for pointers!=null, size>=0
T1(TYPE) void ZeroFast (TYPE &elm               ) {memset(&elm , 0,      SIZE(TYPE));}
T1(TYPE) void ZeroFastN(TYPE *data, UInt    elms) {memset( data, 0, elms*SIZE(TYPE));} // !! this does not check for pointers!=null, elms>=0

  inline void MoveFast (Ptr   dest,   CPtr  src, UIntPtr size) {memmove(     dest,       src,      size      );} // !! this does not check for pointers!=null, size>=0                       , this is more safe than 'CopyFast'
T1(TYPE) void MoveFastN(TYPE *dest, C TYPE *src, UInt    elms) {memmove(     dest,       src, elms*SIZE(TYPE));} // !! this does not check for pointers!=null, elms>=0                       , this is more safe than 'CopyFast'
  inline void CopyFast (Ptr   dest,   CPtr  src, UIntPtr size) {memcpy (     dest,       src,      size      );} // !! this does not check for pointers!=null, size>=0 and if memory overlaps, this is less safe than 'MoveFast'
T1(TYPE) void CopyFast (TYPE &dest, C TYPE &src              ) {memcpy (    &dest,      &src,      SIZE(TYPE));} // !! this does not check                                 if memory overlaps, this is less safe than 'MoveFast'
T1(TYPE) void CopyFastN(TYPE *dest, C TYPE *src, UInt    elms) {memcpy ((Ptr)dest, (CPtr)src, elms*SIZE(TYPE));} // !! this does not check for pointers!=null, elms>=0 and if memory overlaps, this is less safe than 'MoveFast', cast to ptr to silence "-Wdynamic-class-memaccess" warning

inline Bool EqualMemFast(CPtr a, CPtr  b, UIntPtr size) {return !memcmp(a, b, size);} // !! this does not check for pointers!=null, size>=0 !!
/******************************************************************************/
// SWAP MEMORY
/******************************************************************************/
         void SwapFast(Ptr     a, Ptr     b, UIntPtr size);                                // swap memories (this function is slightly faster, because it doesn't check for pointers being not null and size>0, however it will crash if you specify null pointers or size<0)
         void Swap    (Ptr     a, Ptr     b,  IntPtr size);                                // swap memories
  inline void Swap    (Char   &a, Char   &b              ) {Char   temp=a;  a=b;  b=temp;} // swap elements
  inline void Swap    (Char8  &a, Char8  &b              ) {Char8  temp=a;  a=b;  b=temp;} // swap elements
  inline void Swap    (SByte  &a, SByte  &b              ) {SByte  temp=a;  a=b;  b=temp;} // swap elements
  inline void Swap    (Short  &a, Short  &b              ) {Short  temp=a;  a=b;  b=temp;} // swap elements
  inline void Swap    (Int    &a, Int    &b              ) {Int    temp=a;  a=b;  b=temp;} // swap elements
  inline void Swap    (Long   &a, Long   &b              ) {Long   temp=a;  a=b;  b=temp;} // swap elements
  inline void Swap    (Byte   &a, Byte   &b              ) {Byte   temp=a;  a=b;  b=temp;} // swap elements
  inline void Swap    (UShort &a, UShort &b              ) {UShort temp=a;  a=b;  b=temp;} // swap elements
  inline void Swap    (UInt   &a, UInt   &b              ) {UInt   temp=a;  a=b;  b=temp;} // swap elements
  inline void Swap    (ULong  &a, ULong  &b              ) {ULong  temp=a;  a=b;  b=temp;} // swap elements
  inline void Swap    (Flt    &a, Flt    &b              ) {Flt    temp=a;  a=b;  b=temp;} // swap elements
  inline void Swap    (Dbl    &a, Dbl    &b              ) {Dbl    temp=a;  a=b;  b=temp;} // swap elements
  inline void Swap    (Ptr    &a, Ptr    &b              ) {Ptr    temp=a;  a=b;  b=temp;} // swap elements
T1(TYPE) void Swap    (TYPE*  &a, TYPE*  &b              ) {TYPE  *temp=a;  a=b;  b=temp;} // swap elements
T1(TYPE) void Swap    (TYPE   &a, TYPE   &b              ) {SwapFast(&a, &b, SIZE(TYPE));} // swap elements
#if EE_PRIVATE
inline void SwapEndian( Short &i) {Byte *b=(Byte*)&i; Swap(b[0], b[1]);}
inline void SwapEndian(UShort &i) {Byte *b=(Byte*)&i; Swap(b[0], b[1]);}
inline void SwapEndian( Int   &i) {Byte *b=(Byte*)&i; Swap(b[0], b[3]); Swap(b[1], b[2]);}
inline void SwapEndian(UInt   &i) {Byte *b=(Byte*)&i; Swap(b[0], b[3]); Swap(b[1], b[2]);}
inline void SwapEndian( Long  &i) {Byte *b=(Byte*)&i; Swap(b[0], b[7]); Swap(b[1], b[6]); Swap(b[2], b[5]); Swap(b[3], b[4]);}
inline void SwapEndian(ULong  &i) {Byte *b=(Byte*)&i; Swap(b[0], b[7]); Swap(b[1], b[6]); Swap(b[2], b[5]); Swap(b[3], b[4]);}
#endif
/******************************************************************************/
// CHANGE ORDER OF ELEMENTS IN MEMORY
/******************************************************************************/
T1(TYPE) void   ReverseOrder(TYPE *data, Int elms                           ); // reverse   order of elements (first<->last)
T1(TYPE) void    RotateOrder(TYPE *data, Int elms, Int offset               ); // rotate    order of elements, changes the order of elements so "new_index=old_index+offset", 'offset'=offset of moving the original indexes into target indexes (-Inf..Inf)
T1(TYPE) void RandomizeOrder(TYPE *data, Int elms, Randomizer &random=Random); // randomize order of elements
/******************************************************************************/
// COMPARE MEMORY
/******************************************************************************/
         Bool EqualMem(  CPtr  a,   CPtr  b, IntPtr size);                                                   // if equal memories
T1(TYPE) Bool EqualMem(C TYPE &a, C TYPE &b             ) {return EqualMem((CPtr)&a, (CPtr)&b, SIZE(TYPE));} // if equal elements (in memory)
/******************************************************************************/
} // close EE namespace to define following functions in global namespace
/******************************************************************************/
   // OVERRIDE NEW AND DELETE
   /******************************************************************************/
   Ptr  operator new   (size_t size)        ; // override default new    to use custom allocation with detection of memory leaks
   void operator delete(Ptr    data)noexcept; // override default delete to use custom freeing    with detection of memory leaks
/******************************************************************************/
namespace EE // restore EE namespace
{
/******************************************************************************/
// ALLOCATE MEMORY AND CALL CONSTRUCTOR
/******************************************************************************/
T1(TYPE) TYPE*& New(TYPE* &data          ) {data=            new TYPE              ; return data;} // create        object  !! combine only with 'Delete'  or 'delete'   !!
T1(TYPE) TYPE*& New(TYPE* &data, Int elms) {data=((elms>0) ? new TYPE[elms] : null); return data;} // create 'elms' objects !! combine only with 'DeleteN' or 'delete[]' !!
/******************************************************************************/
// CALL DESTRUCTOR AND FREE MEMORY
/******************************************************************************/
T1(TYPE) TYPE*& Delete (TYPE* &data) {delete   data; data=null; return data;} // delete object  and set pointer to null !! combine only with "New(data      )" or 'new'   !!
T1(TYPE) TYPE*& DeleteN(TYPE* &data) {delete[] data; data=null; return data;} // delete objects and set pointer to null !! combine only with "New(data, elms)" or 'new[]' !!
/******************************************************************************/
Str8 Base64(CPtr data, Int size); // calculate Base64 for given memory
/******************************************************************************/
#if EE_PRIVATE
UIntPtr   MemUsage    ();
UIntPtr   MemPeakUsage();
void ClearMemPeakUsage();

void ListMemLeaks();

Ptr  AllocAlign16(IntPtr size);
void  FreeAlign16(Ptr    &ptr);

Ptr  AllocAlign(IntPtr size, Int align);
void  FreeAlign(Ptr    &ptr);

         void _CopyIs  (Ptr   dest,   CPtr  src, C CMemPtr<Bool> &is  , UInt elm_size);                                                // copy only elements with "is[]=true"
         void _CopyList(Ptr   dest,   CPtr  src, C CMemPtr<Int > &list, UInt elm_size);                                                // copy only elements mentioned on the list
T1(TYPE) void  CopyIs  (TYPE *dest, C TYPE *src, C CMemPtr<Bool> &is                 ) {_CopyIs  (dest, (CPtr)src, is  , SIZE(TYPE));} // copy only elements with "is[]=true"
T1(TYPE) void  CopyList(TYPE *dest, C TYPE *src, C CMemPtr<Int > &list               ) {_CopyList(dest, (CPtr)src, list, SIZE(TYPE));} // copy only elements mentioned on the list

// copy different sized data
       void Copy8To16 (Ptr dest16, CPtr src8 , Int elms); // copy  8->16 bit
       void Copy8To32 (Ptr dest32, CPtr src8 , Int elms); // copy  8->32 bit
       void Copy16To8 (Ptr dest8 , CPtr src16, Int elms); // copy 16-> 8 bit
       void Copy16To32(Ptr dest32, CPtr src16, Int elms); // copy 16->32 bit
       void Copy24To32(Ptr dest32, CPtr src24, Int elms); // copy 24->32 bit
       void Copy32To8 (Ptr dest8 , CPtr src32, Int elms); // copy 32-> 8 bit
       void Copy32To16(Ptr dest16, CPtr src32, Int elms); // copy 32->16 bit
       void Copy32To24(Ptr dest24, CPtr src32, Int elms); // copy 32->24 bit
inline void Copy32To32(Ptr dest32, CPtr src32, Int elms) {return Copy(dest32, src32, elms*4);}
#endif
/******************************************************************************/
