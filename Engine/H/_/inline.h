/******************************************************************************/
inline Bool Any(C Half &x                                 ) {return FlagOn(x.data                           , 0x7FFF);} // faster version of "x!=0"
inline Bool Any(C Half &x, C Half &y                      ) {return FlagOn(x.data | y.data                  , 0x7FFF);} // faster version of "x!=0 || y!=0"
inline Bool Any(C Half &x, C Half &y, C Half &z           ) {return FlagOn(x.data | y.data | z.data         , 0x7FFF);} // faster version of "x!=0 || y!=0 || z!=0"
inline Bool Any(C Half &x, C Half &y, C Half &z, C Half &w) {return FlagOn(x.data | y.data | z.data | w.data, 0x7FFF);} // faster version of "x!=0 || y!=0 || z!=0 || w!=0"

inline Bool Any(C Flt &x                              ) {return FlagOn((U32&)x                              , ~SIGN_BIT);} // faster version of "x!=0"
inline Bool Any(C Flt &x, C Flt &y                    ) {return FlagOn((U32&)x | (U32&)y                    , ~SIGN_BIT);} // faster version of "x!=0 || y!=0"
inline Bool Any(C Flt &x, C Flt &y, C Flt &z          ) {return FlagOn((U32&)x | (U32&)y | (U32&)z          , ~SIGN_BIT);} // faster version of "x!=0 || y!=0 || z!=0"
inline Bool Any(C Flt &x, C Flt &y, C Flt &z, C Flt &w) {return FlagOn((U32&)x | (U32&)y | (U32&)z | (U32&)w, ~SIGN_BIT);} // faster version of "x!=0 || y!=0 || z!=0 || w!=0"

inline Bool Any(C Dbl &x                              ) {return FlagOn((U64&)x                              , (~0ull)>>1);} // faster version of "x!=0"
inline Bool Any(C Dbl &x, C Dbl &y                    ) {return FlagOn((U64&)x | (U64&)y                    , (~0ull)>>1);} // faster version of "x!=0 || y!=0"
inline Bool Any(C Dbl &x, C Dbl &y, C Dbl &z          ) {return FlagOn((U64&)x | (U64&)y | (U64&)z          , (~0ull)>>1);} // faster version of "x!=0 || y!=0 || z!=0"
inline Bool Any(C Dbl &x, C Dbl &y, C Dbl &z, C Dbl &w) {return FlagOn((U64&)x | (U64&)y | (U64&)z | (U64&)w, (~0ull)>>1);} // faster version of "x!=0 || y!=0 || z!=0 || w!=0"
/******************************************************************************/
inline Bool AtomicGet(C Bool &x        ) {return x;}
inline Byte AtomicGet(C Byte &x        ) {return x;}
inline Int  AtomicGet(C Int  &x        ) {return x;}
inline UInt AtomicGet(C UInt &x        ) {return x;}
inline Flt  AtomicGet(C Flt  &x        ) {return x;}
inline void AtomicSet(  Bool &x, Bool y) {x=y     ;}
inline void AtomicSet(  Byte &x, Byte y) {x=y     ;}
inline void AtomicSet(  Int  &x, Int  y) {x=y     ;}
inline void AtomicSet(  UInt &x, UInt y) {x=y     ;}
inline void AtomicSet(  Flt  &x, Flt  y) {x=y     ;}
#if X64
inline Long  AtomicGet(C Long  &x         ) {return x;}
inline ULong AtomicGet(C ULong &x         ) {return x;}
inline void  AtomicSet(  Long  &x, Long  y) {x=y     ;}
inline void  AtomicSet(  ULong &x, ULong y) {x=y     ;}
#endif
T1(TYPE) void AtomicSet(TYPE* &x, TYPE *y) {x=y;}
/******************************************************************************/
inline       Str8::Str8     (Str8 &&s) {_length=0; Swap(T, s);}
inline       Str ::Str      (Str  &&s) {_length=0; Swap(T, s);}
inline Str8& Str8::operator=(Str8 &&s) {           Swap(T, s); return T;}
inline Str & Str ::operator=(Str  &&s) {           Swap(T, s); return T;}

inline Bool Str8::save(File &f)C {f.putStr(T); return f.ok();}
inline Bool Str ::save(File &f)C {f.putStr(T); return f.ok();}
inline Bool Str8::load(File &f)  {f.getStr(T); return f.ok();}
inline Bool Str ::load(File &f)  {f.getStr(T); return f.ok();}
/******************************************************************************/
inline   TextParam*           FindParam(MemPtr<TextParam> params, C Str &name, Int i)  {return ConstCast(  CFindParam(params, name, i));}
inline   TextNode *           FindNode (MemPtr<TextNode >  nodes, C Str &name, Int i)  {return ConstCast(  CFindNode ( nodes, name, i));}
inline C XmlParam *  XmlNode::findParam(C Str &name, Int i                          )C {return ConstCast(T).findParam(        name, i) ;}
inline C TextNode * TextNode::findNode (C Str &name, Int i                          )C {return ConstCast(T).findNode (        name, i) ;}
inline C TextNode * TextData::findNode (C Str &name, Int i                          )C {return ConstCast(T).findNode (        name, i) ;}
/******************************************************************************/
inline C TextParam* FileParams::findParam(C Str &name)C {return ConstCast(T).findParam(name);}
/******************************************************************************/
inline C SkelBone * Skeleton ::findBone (              BONE_TYPE type, Int type_index, Int type_sub)C {return ConstCast(T).findBone (      type, type_index, type_sub);}
inline C SkelBone & Skeleton :: getBone (              BONE_TYPE type, Int type_index, Int type_sub)C {return ConstCast(T). getBone (      type, type_index, type_sub);}
inline C SkelBone * Skeleton ::findBone (CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub)C {return ConstCast(T).findBone (name, type, type_index, type_sub);}
inline C SkelBone * Skeleton ::findBone (CChar8 *name                                              )C {return ConstCast(T).findBone (name                            );}
inline C SkelSlot * Skeleton ::findSlot (CChar8 *name                                              )C {return ConstCast(T).findSlot (name                            );}
inline C SkelBone & Skeleton :: getBone (CChar8 *name                                              )C {return ConstCast(T). getBone (name                            );}
inline C SkelSlot & Skeleton :: getSlot (CChar8 *name                                              )C {return ConstCast(T). getSlot (name                            );}
inline C OrientM  * AnimSkel ::findSlot (CChar8 *name                                              )C {return ConstCast(T).findSlot (name                            );}
inline C AnimBone * Animation::findBone (CChar8 *name, BONE_TYPE type, Int type_index, Int type_sub)C {return ConstCast(T).findBone (name, type, type_index, type_sub);}
inline C AnimEvent* Animation::findEvent(CChar8 *name                                              )C {return ConstCast(T).findEvent(name                            );}
/******************************************************************************/
inline C Param* Object::findParam(CChar8 *name)C {return ConstCast(T).findParam(name  );}
inline   Param* Object::findParam(C Str8 &name)  {return ConstCast(T).findParam(name());}
inline C Param* Object::findParam(C Str8 &name)C {return ConstCast(T).findParam(name());}
inline C Param* Object::findParam(C Str  &name)C {return ConstCast(T).findParam(name  );}
inline C Param& Object:: getParam(C Str  &name)C {return ConstCast(T). getParam(name  );}
/******************************************************************************/
extern Bool  _ImportJXL (Image &image, File &f);
extern Bool (*ImportJXL)(Image &image, File &f);
inline void   SupportImportJXL() {ImportJXL=_ImportJXL;}

extern Bool  _ExportJXL (C Image &image, File &f, Flt quality, Flt compression_level);
extern Bool (*ExportJXL)(C Image &image, File &f, Flt quality, Flt compression_level);
inline void   SupportExportJXL() {ExportJXL=_ExportJXL;}

inline void   SupportJXL() {SupportImportJXL(); SupportExportJXL();}

extern Bool  _ImportAVIF (Image &image, File &f);
extern Bool (*ImportAVIF)(Image &image, File &f);
inline void   SupportImportAVIF() {ImportAVIF=_ImportAVIF;}

extern Bool  _ExportAVIF (C Image &image, File &f, Flt rgb_quality, Flt alpha_quality, Flt compression_level);
extern Bool (*ExportAVIF)(C Image &image, File &f, Flt rgb_quality, Flt alpha_quality, Flt compression_level);
inline void   SupportExportAVIF() {ExportAVIF=_ExportAVIF;}

inline void   SupportAVIF() {SupportImportAVIF(); SupportExportAVIF();}

extern Bool  _CompressBC67 (C Image &src, Image &dest);
extern Bool (*CompressBC67)(C Image &src, Image &dest);
inline void   SupportCompressBC() {CompressBC67=_CompressBC67;}

extern Bool  _CompressETC (C Image &src, Image &dest, Int quality=-1);
extern Bool (*CompressETC)(C Image &src, Image &dest, Int quality   );
inline void   SupportCompressETC() {CompressETC=_CompressETC;}

extern Bool  _CompressASTC (C Image &src, Image &dest);
extern Bool (*CompressASTC)(C Image &src, Image &dest);
inline void   SupportCompressASTC() {CompressASTC=_CompressASTC;}

extern Bool  _CompressPVRTC (C Image &src, Image &dest, Int quality=-1);
extern Bool (*CompressPVRTC)(C Image &src, Image &dest, Int quality   );
inline void   SupportCompressPVRTC() {if(WINDOWS_OLD || MAC || LINUX)CompressPVRTC=_CompressPVRTC;}

inline void SupportCompressAll() {SupportCompressBC(); SupportCompressETC(); SupportCompressASTC(); SupportCompressPVRTC(); SupportJXL(); SupportAVIF();}

extern Bool  _ResizeWaifu (C Image &src, Image &dest, UInt flags);
extern Bool (*ResizeWaifu)(C Image &src, Image &dest, UInt flags);
inline void   SupportFilterWaifu() {ResizeWaifu=_ResizeWaifu;}
/******************************************************************************/
// STRING / TEXT
/******************************************************************************/
inline Char * TextPos(Char  *src, Char  c) {return ConstCast(TextPos((CChar *)src, c));}
inline Char8* TextPos(Char8 *src, Char8 c) {return ConstCast(TextPos((CChar8*)src, c));}

inline void      Split(MemPtr<Str> splits, C Str &string, Char8 separator) {return Split(splits, string, Char8To16(separator));}
inline Memc<Str> Split(                    C Str &string, Char8 separator) {return Split(        string, Char8To16(separator));}
/******************************************************************************/
// MATH
/******************************************************************************/
inline Ptr Randomizer::pointer()
{
#if X64
   return Ptr(l());
#else
   return Ptr(T());
#endif
}
/******************************************************************************/
// VECTOR
/******************************************************************************/
inline VecB2 ::VecB2 (C VecI2  &v) {set(v.x, v.y          );}
inline VecSB2::VecSB2(C VecI2  &v) {set(v.x, v.y          );}
inline VecB  ::VecB  (C VecI   &v) {set(v.x, v.y, v.z     );}
inline VecSB ::VecSB (C VecI   &v) {set(v.x, v.y, v.z     );}
inline VecB4 ::VecB4 (C VecI4  &v) {set(v.x, v.y, v.z, v.w);}
inline VecB4 ::VecB4 (C VecUS4 &v) {set(v.x, v.y, v.z, v.w);}
inline VecSB4::VecSB4(C VecI4  &v) {set(v.x, v.y, v.z, v.w);}
inline VecUS2::VecUS2(C VecB2  &v) {set(v.x, v.y          );}
inline VecUS2::VecUS2(C VecI2  &v) {set(v.x, v.y          );}
inline VecUS ::VecUS (C VecB   &v) {set(v.x, v.y, v.z     );}
inline VecUS ::VecUS (C VecI   &v) {set(v.x, v.y, v.z     );}
inline VecUS4::VecUS4(C VecB4  &v) {set(v.x, v.y, v.z, v.w);}
inline VecUS4::VecUS4(C VecI4  &v) {set(v.x, v.y, v.z, v.w);}
inline Vec2  ::Vec2  (C VecH2  &v) {set(v.x, v.y          );}
inline Vec2  ::Vec2  (C VecD2  &v) {set(v.x, v.y          );}
inline Vec2  ::Vec2  (C VecI2  &v) {set(v.x, v.y          );}
inline Vec2  ::Vec2  (C VecB2  &v) {set(v.x, v.y          );}
inline Vec2  ::Vec2  (C VecSB2 &v) {set(v.x, v.y          );}
inline Vec2  ::Vec2  (C VecUS2 &v) {set(v.x, v.y          );}
inline VecD2 ::VecD2 (C VecH2  &v) {set(v.x, v.y          );}
inline VecD2 ::VecD2 (C Vec2   &v) {set(v.x, v.y          );}
inline VecD2 ::VecD2 (C VecI2  &v) {set(v.x, v.y          );}
inline VecD2 ::VecD2 (C VecB2  &v) {set(v.x, v.y          );}
inline VecD2 ::VecD2 (C VecSB2 &v) {set(v.x, v.y          );}
inline VecD2 ::VecD2 (C VecUS2 &v) {set(v.x, v.y          );}
inline Vec   ::Vec   (C VecH   &v) {set(v.x, v.y, v.z     );}
inline Vec   ::Vec   (C VecD   &v) {set(v.x, v.y, v.z     );}
inline Vec   ::Vec   (C VecI   &v) {set(v.x, v.y, v.z     );}
inline Vec   ::Vec   (C VecB   &v) {set(v.x, v.y, v.z     );}
inline Vec   ::Vec   (C VecSB  &v) {set(v.x, v.y, v.z     );}
inline VecD  ::VecD  (C VecH   &v) {set(v.x, v.y, v.z     );}
inline VecD  ::VecD  (C Vec    &v) {set(v.x, v.y, v.z     );}
inline VecD  ::VecD  (C VecI   &v) {set(v.x, v.y, v.z     );}
inline VecD  ::VecD  (C VecB   &v) {set(v.x, v.y, v.z     );}
inline VecD  ::VecD  (C VecSB  &v) {set(v.x, v.y, v.z     );}
inline Vec4  ::Vec4  (C VecH4  &v) {set(v.x, v.y, v.z, v.w);}
inline Vec4  ::Vec4  (C VecD4  &v) {set(v.x, v.y, v.z, v.w);}
inline Vec4  ::Vec4  (C VecI4  &v) {set(v.x, v.y, v.z, v.w);}
inline Vec4  ::Vec4  (C VecB4  &v) {set(v.x, v.y, v.z, v.w);}
inline Vec4  ::Vec4  (C VecSB4 &v) {set(v.x, v.y, v.z, v.w);}
inline Vec4  ::Vec4  (C VecUS4 &v) {set(v.x, v.y, v.z, v.w);}
inline VecD4 ::VecD4 (C VecH4  &v) {set(v.x, v.y, v.z, v.w);}
inline VecD4 ::VecD4 (C Vec4   &v) {set(v.x, v.y, v.z, v.w);}
inline VecD4 ::VecD4 (C VecI4  &v) {set(v.x, v.y, v.z, v.w);}
inline VecD4 ::VecD4 (C VecB4  &v) {set(v.x, v.y, v.z, v.w);}
inline VecD4 ::VecD4 (C VecSB4 &v) {set(v.x, v.y, v.z, v.w);}
inline VecD4 ::VecD4 (C VecUS4 &v) {set(v.x, v.y, v.z, v.w);}

inline Vec  Vec2 :: xy0()C {return Vec (x, y, 0);}
inline Vec  Vec2 :: x0y()C {return Vec (x, 0, y);}
inline Vec  Vec2 ::_0xy()C {return Vec (0, x, y);}
inline Vec  Vec2 ::_0yx()C {return Vec (0, y, x);}
inline VecD VecD2:: xy0()C {return VecD(x, y, 0);}
inline VecD VecD2:: x0y()C {return VecD(x, 0, y);}
inline VecD VecD2::_0xy()C {return VecD(0, x, y);}
inline VecD VecD2::_0yx()C {return VecD(0, y, x);}
inline VecI VecI2:: xy0()C {return VecI(x, y, 0);}
inline VecI VecI2:: x0y()C {return VecI(x, 0, y);}
inline VecI VecI2::_0xy()C {return VecI(0, x, y);}
inline VecI VecI2::_0yx()C {return VecI(0, y, x);}
/******************************************************************************/
// MATRIX
/******************************************************************************/
inline void Matrix3::mul(C RevMatrix3 &matrix, Matrix3 &dest)C {matrix.mul(T, dest);}
inline void Matrix ::mul(C RevMatrix  &matrix, Matrix  &dest)C {matrix.mul(T, dest);}

#if EE_PRIVATE
inline void SetFastMatrix(                 ) {Sh.ViewMatrix->set    (        CamMatrixInv);}
inline void SetFastMatrix(C Matrix  &matrix) {Sh.ViewMatrix->fromMul(matrix, CamMatrixInv);}
inline void SetFastMatrix(C MatrixM &matrix) {Sh.ViewMatrix->fromMul(matrix, CamMatrixInv);}

inline void SetFastMatrixPrev(                 ) {Sh.ViewMatrixPrev->set    (        CamMatrixInvPrev);}
inline void SetFastMatrixPrev(C Matrix  &matrix) {Sh.ViewMatrixPrev->fromMul(matrix, CamMatrixInvPrev);}
inline void SetFastMatrixPrev(C MatrixM &matrix) {Sh.ViewMatrixPrev->fromMul(matrix, CamMatrixInvPrev);}
#endif
/******************************************************************************/
// SHAPES
/******************************************************************************/
inline Ball::Ball(C BallM &ball) {set(ball.r, ball.pos);}
/******************************************************************************/
// TEMPLATES
/******************************************************************************/
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(SByte  i, C TYPE &container) {return InRange(i, Elms(container));} // template specialization for not enum's
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(Byte   i, C TYPE &container) {return InRange(i, Elms(container));} // template specialization for not enum's
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(Short  i, C TYPE &container) {return InRange(i, Elms(container));} // template specialization for not enum's
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(UShort i, C TYPE &container) {return InRange(i, Elms(container));} // template specialization for not enum's
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(Int    i, C TYPE &container) {return InRange(i, Elms(container));} // template specialization for not enum's
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(UInt   i, C TYPE &container) {return InRange(i, Elms(container));} // template specialization for not enum's
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(Long   i, C TYPE &container) {return InRange(i, Elms(container));} // template specialization for not enum's
T1(TYPE) DISABLE_IF_ENUM(TYPE, Bool) InRange(ULong  i, C TYPE &container) {return InRange(i, Elms(container));} // template specialization for not enum's
/******************************************************************************/
T1(TYPE) struct ClassFunc // various basic functions used by many classes
{
   static void New      (Ptr elm                        ) {    new(        elm )                     TYPE  ;}
   static void Del      (Ptr elm                        ) {       ( (TYPE*)elm )->PLATFORM(, TYPE::)~TYPE();} // for non-Windows (Clang) directly call specified destructor (ignoring any virtual) because this function is always paired with 'New' above, so we always operate on the same type of class, this improves performance and silences -Wdelete-non-abstract-non-virtual-dtor. Unsupported on Windows compiler
   static void Copy     (Ptr dest,  CPtr  src           ) {       (*(TYPE*)dest)=*(C TYPE*)src             ;}
   static Bool Load     (Ptr elm , C Str &file          ) {return ( (TYPE*)elm )->  load(file      )       ;}
   static Bool LoadUser (Ptr elm , C Str &file, Ptr user) {return ( (TYPE*)elm )->  load(file, user)       ;}
   static Bool LoadEmpty(Ptr elm                        ) {return ( (TYPE*)elm )->  load(          )       ;}
   static void Unload   (Ptr elm                        ) {return ( (TYPE*)elm )->unload(          )       ;}

   static inline Bool HasNew() {return !std::is_trivially_default_constructible<TYPE>::value && New!=ClassFunc<Int>::New;} // check also if the '<TYPE>.New' function address is different than '<Int>.New' because 'is_trivially_default_constructible' is not enough for cases when constructor exists but is empty
   static inline Bool HasDel() {return !std::is_trivially_destructible         <TYPE>::value && Del!=ClassFunc<Int>::Del;} // check also if the '<TYPE>.Del' function address is different than '<Int>.Del' because 'is_trivially_destructible'          is not enough for cases when  destructor exists but is empty

   static inline void (*GetNew())(Ptr elm) {return HasNew() ? New : null;}
   static inline void (*GetDel())(Ptr elm) {return HasDel() ? Del : null;}
};
/******************************************************************************/
// SORT
/******************************************************************************/
void _Sort(Ptr data, Int elms, Int elm_size,            Int compare(CPtr a, CPtr b           ));
void _Sort(Ptr data, Int elms, Int elm_size, CPtr user, Int compare(CPtr a, CPtr b, CPtr user));

T1(TYPE) void Sort(TYPE *data, Int elms,            Int compare(C TYPE &a, C TYPE &b           )) {_Sort(Ptr(data), elms, SIZE(TYPE),       (Int(*)(CPtr, CPtr      ))compare);}
T1(TYPE) void Sort(TYPE *data, Int elms, CPtr user, Int compare(C TYPE &a, C TYPE &b, CPtr user)) {_Sort(Ptr(data), elms, SIZE(TYPE), user, (Int(*)(CPtr, CPtr, CPtr))compare);}

Bool _BinarySearch     (CPtr data, Int elms, Int elm_size, CPtr value, Int &index, Int compare(CPtr a, CPtr b));
Bool _BinarySearchFirst(CPtr data, Int elms, Int elm_size, CPtr value, Int &index, Int compare(CPtr a, CPtr b));

T2(DATA, VALUE) Bool BinarySearch     (C DATA *data, Int elms, C VALUE &value, Int &index, Int compare(C DATA &a, C VALUE &b)) {return _BinarySearch     (data, elms, SIZE(DATA), &value, index, (Int(*)(CPtr, CPtr))compare);}
T2(DATA, VALUE) Bool BinarySearchFirst(C DATA *data, Int elms, C VALUE &value, Int &index, Int compare(C DATA &a, C VALUE &b)) {return _BinarySearchFirst(data, elms, SIZE(DATA), &value, index, (Int(*)(CPtr, CPtr))compare);}

#if EE_PRIVATE
struct FloatIndex // Float + Index
{
   Flt f; // float
   Int i; // index

   static Int Compare(C FloatIndex &a, C FloatIndex &b) {return EE::Compare(a.f, b.f);}
};
inline void Sort(FloatIndex *data, Int elms) {Sort(data, elms, FloatIndex::Compare);}
#endif
/******************************************************************************/
// MEMORY
/******************************************************************************/
#if X64
inline Ptr Alloc( Int size) {return Alloc(( IntPtr)size);}
inline Ptr Alloc(UInt size) {return Alloc((UIntPtr)size);}
#endif

void _Realloc    (Ptr &data, ULong size_new, ULong size_old); // reallocate memory without losing data                      , Exit on fail !! this function can be used only for memory allocated using 'Alloc', but not 'New' !!
void _Realloc    (Ptr &data,  Long size_new,  Long size_old); // reallocate memory without losing data                      , Exit on fail !! this function can be used only for memory allocated using 'Alloc', but not 'New' !!
void _ReallocZero(Ptr &data, ULong size_new, ULong size_old); // reallocate memory without losing data and zero new elements, Exit on fail !! this function can be used only for memory allocated using 'Alloc', but not 'New' !!
void _ReallocZero(Ptr &data,  Long size_new,  Long size_old); // reallocate memory without losing data and zero new elements, Exit on fail !! this function can be used only for memory allocated using 'Alloc', but not 'New' !!

T1(TYPE) void Realloc    (TYPE* &data, Int elms_new, Int elms_old) {_Realloc    (*(Ptr*)&data, elms_new*SIZEL(TYPE), elms_old*SIZEL(TYPE));}
T1(TYPE) void ReallocZero(TYPE* &data, Int elms_new, Int elms_old) {_ReallocZero(*(Ptr*)&data, elms_new*SIZEL(TYPE), elms_old*SIZEL(TYPE));}
#if EE_PRIVATE
T1(TYPE) void Realloc1    (TYPE* &data,              Int elms_old) {Realloc    (data, elms_old+1, elms_old);}
T1(TYPE) void ReallocZero1(TYPE* &data,              Int elms_old) {ReallocZero(data, elms_old+1, elms_old);}

void _MoveElmLeftUnsafe(Ptr data, UInt elm_size, Int elm, Int new_index, Ptr temp);
#endif

void   _ReverseOrder(Ptr data, Int elms, UInt elm_size                           ); // reverse   order of elements (first<->last)
void    _RotateOrder(Ptr data, Int elms, UInt elm_size, Int offset               ); // rotate    order of elements, changes the order of elements so "new_index=old_index+offset", 'offset'=offset of moving the original indexes into target indexes (-Inf..Inf)
void _RandomizeOrder(Ptr data, Int elms, UInt elm_size, Randomizer &random=Random); // randomize order of elements
void      _MoveElm  (Ptr data, Int elms, UInt elm_size, Int elm, Int new_index   ); // move 'elm' element to new position located at 'new_index'

T1(TYPE) void   ReverseOrder(TYPE *data, Int elms                        ) {  _ReverseOrder(data, elms, SIZE(TYPE)                );}
T1(TYPE) void    RotateOrder(TYPE *data, Int elms, Int offset            ) {   _RotateOrder(data, elms, SIZE(TYPE), offset        );}
T1(TYPE) void RandomizeOrder(TYPE *data, Int elms, Randomizer &random    ) {_RandomizeOrder(data, elms, SIZE(TYPE), random        );}
T1(TYPE) void      MoveElm  (TYPE *data, Int elms, Int elm, Int new_index) {     _MoveElm  (data, elms, SIZE(TYPE), elm, new_index);}
/******************************************************************************/
// MISC
/******************************************************************************/
inline Bool Application::mainThread()C {return GetThreadID()==_thread_id;}
/******************************************************************************/
// REFERENCE
/******************************************************************************/
T1(TYPE)           Bool Reference<TYPE>::save(File  &f    )C {          return f.put(_object_id);}
T1(TYPE)           Bool Reference<TYPE>::load(File  &f    )  {_object=null; if(f.get(_object_id))return true; _object_id.zero(); return false;}
T1(TYPE) T1(WORLD) void Reference<TYPE>::link(WORLD &world)  {if(!valid() && _object_id.valid())_object=CAST(TYPE, world.findObjById(_object_id));}
/******************************************************************************/
// FIXED ARRAY
/******************************************************************************/
template<typename TYPE, Int NUM>                                       FixedArray<TYPE, NUM>::FixedArray(                 )                {_elm_size=SIZE(TYPE); _data=null;}
template<typename TYPE, Int NUM>                                       FixedArray<TYPE, NUM>::FixedArray(C FixedArray &src) : FixedArray() {T=src;}
template<typename TYPE, Int NUM>               FixedArray<TYPE, NUM>&  FixedArray<TYPE, NUM>::operator= (C FixedArray &src)                {FREPAO(T)=src[i]; return T;}
template<typename TYPE, Int NUM>               FixedArray<TYPE, NUM>&  FixedArray<TYPE, NUM>::del         () {DeleteN(_data); _elm_size=SIZE(TYPE); return T;}
template<typename TYPE, Int NUM> T1(EXTENDED)  FixedArray<TYPE, NUM>&  FixedArray<TYPE, NUM>::replaceClass() {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>(); del(); _elm_size=SIZE(EXTENDED); _data=new EXTENDED[NUM]; return T;}

template<typename TYPE>                                FixedElm<TYPE>::FixedElm (               )              {_data=null;}
template<typename TYPE>                                FixedElm<TYPE>::FixedElm (C FixedElm &src) : FixedElm() {T=src;}
template<typename TYPE>               FixedElm<TYPE>&  FixedElm<TYPE>::operator=(C FixedElm &src)              {T()=src(); return T;}
template<typename TYPE>               FixedElm<TYPE>&  FixedElm<TYPE>::del         () {Delete(_data); return T;}
template<typename TYPE> T1(EXTENDED)  FixedElm<TYPE>&  FixedElm<TYPE>::replaceClass() {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>(); del(); _data=new EXTENDED; return T;}
/******************************************************************************/
// MEMS
/******************************************************************************/
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::clear()
{
   if(ClassFunc<TYPE>::HasDel())REPA(T)T[i].~TYPE(); // destroy as the first step
   Free(_data); _elms=0;
   return T;
}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::del() {return clear();}

T1(TYPE)  Int      Mems<TYPE>::elms    ()C {return _elms;}
T1(TYPE)  UInt     Mems<TYPE>::elmSize ()C {return SIZE(TYPE);}
T1(TYPE)  UIntPtr  Mems<TYPE>::memUsage()C {return UIntPtr(elms())*elmSize();}
T1(TYPE)  UIntPtr  Mems<TYPE>::elmsMem ()C {return UIntPtr(elms())*elmSize();}

T1(TYPE)  TYPE*  Mems<TYPE>::data      (     ) {                              return _data   ;}
T1(TYPE)  TYPE*  Mems<TYPE>::addr      (Int i) {return     InRange(i, _elms) ?      &_data[i] : null;}
T1(TYPE)  TYPE&  Mems<TYPE>::operator[](Int i) {DEBUG_RANGE_ASSERT(i, _elms); return _data[i];}
T1(TYPE)  TYPE&  Mems<TYPE>::first     (     ) {return T[       0];}
T1(TYPE)  TYPE&  Mems<TYPE>::last      (     ) {return T[elms()-1];}

T1(TYPE)  TYPE  Mems<TYPE>::popFirst(       Bool keep_order) {TYPE temp=first(); remove    (0, keep_order); return temp;}
T1(TYPE)  TYPE  Mems<TYPE>::pop     (Int i, Bool keep_order) {TYPE temp=   T[i]; remove    (i, keep_order); return temp;}
T1(TYPE)  TYPE  Mems<TYPE>::pop     (                      ) {TYPE temp= last(); removeLast(             ); return temp;}

T1(TYPE)  C TYPE*  Mems<TYPE>::data      (     )C {return ConstCast(T).data ( );}
T1(TYPE)  C TYPE*  Mems<TYPE>::addr      (Int i)C {return ConstCast(T).addr (i);}
T1(TYPE)  C TYPE&  Mems<TYPE>::operator[](Int i)C {return ConstCast(T)      [i];}
T1(TYPE)  C TYPE&  Mems<TYPE>::first     (     )C {return ConstCast(T).first( );}
T1(TYPE)  C TYPE&  Mems<TYPE>::last      (     )C {return ConstCast(T).last ( );}

T1(TYPE)  TYPE&  Mems<TYPE>::operator()(Int i)
{
   if(i< 0     )Exit("i<0 inside 'Mems.operator()(Int i)'");
   if(i>=elms())setNumZero(i+1);
   return T[i];
}
T1(TYPE)  TYPE&  Mems<TYPE>::New  (     ) {return T[addNum(1)];}
T1(TYPE)  TYPE&  Mems<TYPE>::NewAt(Int i)
{
   if(elms()>=INT_MAX)Exit("'Mems.NewAt' size too big");
   Clamp(i, 0, elms());
   TYPE *temp=Alloc<TYPE>(elms()+1);
   CopyFastN(temp    , data()  ,        i);
   CopyFastN(temp+i+1, data()+i, elms()-i);
   Free(_data); _data=temp; _elms++;
   TYPE &elm=T[i]; new(&elm)TYPE; return elm; // create as the last step
}

T1(TYPE)  Int  Mems<TYPE>::index(C TYPE *elm)C
{
   UIntPtr i=UIntPtr(elm)-UIntPtr(data());
   if(i<elmsMem())return Int(i/elmSize()); // unsigned compare will already guarantee "i>=0 && "
   return -1;
}
T1(TYPE)  Bool  Mems<TYPE>::contains(C TYPE *elm)C {return index(elm)>=0;}

T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::remove(Int i, Bool /*keep_order*/)
{
   if(InRange(i, T))
   {
      T[i].~TYPE(); // destroy as the first step
      TYPE *temp=Alloc<TYPE>(elms()-1);
      CopyFastN(temp  , data()    ,        i  );
      CopyFastN(temp+i, data()+i+1, elms()-i-1);
      Free(_data); _data=temp; _elms--;
   }
   return T;
}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::removeLast(                            ) {return remove(elms()-1              );}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::removeData(C TYPE *elm, Bool keep_order) {return remove(index(elm), keep_order);}

T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::setNum(Int num)
{
   MAX(num, 0);
   if (num>elms()) // add new elements
   {
      Int old_elms=elms();
      TYPE *temp=Alloc<TYPE>(num);
      CopyFastN(temp, data(), elms());
      Free(_data); _data=temp; _elms=num;
      if(ClassFunc<TYPE>::HasNew())for(Int i=old_elms; i<elms(); i++)new(&T[i])TYPE; // create as the last step
   }else
   if(num<elms()) // remove elements
   {
      if(ClassFunc<TYPE>::HasDel())for(Int i=elms(); --i>=num; )T[i].~TYPE(); // destroy as the first step
      TYPE *temp=Alloc<TYPE>(num);
      CopyFastN(temp, data(), num);
      Free(_data); _data=temp; _elms=num;
   }
   return T;
}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::setNumZero(Int num)
{
   MAX(num, 0);
   if (num>elms()) // add new elements
   {
      Int old_elms=elms();
      TYPE *temp=Alloc<TYPE>(num);
      CopyFastN(temp       , data(),     elms()); // copy old elements
      ZeroFastN(temp+elms(),         num-elms()); // zero new elements
      Free(_data); _data=temp; _elms=num;
      if(ClassFunc<TYPE>::HasNew())for(Int i=old_elms; i<elms(); i++)new(&T[i])TYPE; // create as the last step
   }else
   if(num<elms()) // remove elements
   {
      if(ClassFunc<TYPE>::HasDel())for(Int i=elms(); --i>=num; )T[i].~TYPE(); // destroy as the first step
      TYPE *temp=Alloc<TYPE>(num);
      CopyFastN(temp, data(), num);
      Free(_data); _data=temp; _elms=num;
   }
   return T;
}

T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::setNum(Int num, Int keep)
{
   MAX(num, 0);
   Clamp(keep, 0, Min(elms(), num));
   if(ClassFunc<TYPE>::HasDel())for(Int i=elms(); --i>=keep; )T[i].~TYPE(); // delete unkept elements, destroy as the first step
   if(num!=elms()) // resize memory
   {
      TYPE *temp=Alloc<TYPE>(num);
      CopyFastN(temp, data(), keep); // copy kept elements
      Free(_data); _data=temp; _elms=num;
   }
   if(ClassFunc<TYPE>::HasNew())for(Int i=keep; i<elms(); i++)new(&T[i])TYPE; // create new elements, create as the last step
   return T;
}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::setNumZero(Int num, Int keep)
{
   MAX(num, 0);
   Clamp(keep, 0, Min(elms(), num));
   if(ClassFunc<TYPE>::HasDel())for(Int i=elms(); --i>=keep; )T[i].~TYPE(); // delete unkept elements, destroy as the first step
   if(num!=elms()) // resize memory
   {
      TYPE *temp=Alloc<TYPE>(num);
      CopyFastN(temp, data(), keep); // copy kept elements
      Free(_data); _data=temp; _elms=num;
   }
   ZeroFastN(data()+keep, elms()-keep); // zero new elements
   if(ClassFunc<TYPE>::HasNew())for(Int i=keep; i<elms(); i++)new(&T[i])TYPE; // create new elements, create as the last step
   return T;
}

T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::setNumDiscard(Int num)
{
   MAX(num, 0);
   if( num!=elms())
   {
      if(ClassFunc<TYPE>::HasDel())REPA(T)T[i].~TYPE(); // delete all elements, destroy as the first step
      Alloc(Free(_data), _elms=num);
      if(ClassFunc<TYPE>::HasNew())FREPA(T)new(&T[i])TYPE; // create new elements, create as the last step
   }
   return T;
}
#if EE_PRIVATE
T1(TYPE)  void  Mems<TYPE>::minNumDiscard(Int num)
{
   if(Greater(num, elms())) // num>elms()
   {
      if(ClassFunc<TYPE>::HasDel())REPA(T)T[i].~TYPE(); // delete all elements, destroy as the first step
      Alloc(Free(_data), _elms=num);
      if(ClassFunc<TYPE>::HasNew())FREPA(T)new(&T[i])TYPE; // create new elements, create as the last step
   }
}
#endif

T1(TYPE)  Int  Mems<TYPE>::addNum(Int num) {Int index=elms(); Long new_elms=Long(index)+num; if(new_elms>INT_MAX)Exit("'Mems.addNum' size too big"); setNum((Int)new_elms); return index;}

T1(TYPE) T1(VALUE)  Bool  Mems<TYPE>::binarySearch     (C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return _BinarySearch     (data(), elms(), elmSize(), &value, index, (Int(*)(CPtr, CPtr))compare);}
T1(TYPE) T1(VALUE)  Bool  Mems<TYPE>::binarySearchFirst(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return _BinarySearchFirst(data(), elms(), elmSize(), &value, index, (Int(*)(CPtr, CPtr))compare);}

T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::sort(           Int compare(C TYPE &a, C TYPE &b           )) {_Sort(data(), elms(), elmSize(),       (Int(*)(CPtr, CPtr      ))compare); return T;}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::sort(CPtr user, Int compare(C TYPE &a, C TYPE &b, CPtr user)) {_Sort(data(), elms(), elmSize(), user, (Int(*)(CPtr, CPtr, CPtr))compare); return T;}

T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::  reverseOrder(                      ) {  _ReverseOrder(data(), elms(), elmSize()                ); return T;}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::randomizeOrder(                      ) {_RandomizeOrder(data(), elms(), elmSize()                ); return T;}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::   rotateOrder(Int offset            ) {   _RotateOrder(data(), elms(), elmSize(), offset        ); return T;}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::     moveElm  (Int elm, Int new_index) {     _MoveElm  (data(), elms(), elmSize(), elm, new_index); return T;}
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::     swapOrder(Int i  , Int j        ) {if(InRange(i, T) && InRange(j, T))Swap(_data[i], _data[j]); return T;}

T1(TYPE)                     Mems<TYPE>&  Mems<TYPE>::operator=(C  Mems  <TYPE      >  &src) {if(this!=&src     ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Mems<TYPE>&  Mems<TYPE>::operator=(C  Memc  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Mems<TYPE>&  Mems<TYPE>::operator=(C  Memt  <TYPE, size>  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Mems<TYPE>&  Mems<TYPE>::operator=(C  Memb  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Mems<TYPE>&  Mems<TYPE>::operator=(C  Memx  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Mems<TYPE>&  Mems<TYPE>::operator=(C  Meml  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Mems<TYPE>&  Mems<TYPE>::operator=(C CMemPtr<TYPE, size>  &src) {if(this!=src._mems){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Mems<TYPE>&  Mems<TYPE>::operator=(   Mems  <TYPE      > &&src) {Swap(T, src); return T;}

T1(TYPE)  Bool  Mems<TYPE>::operator==(C Mems<TYPE> &x)C {if(elms()!=x.elms())return false; REPA(T)if(T[i]!=x[i])return false; return true ;}
T1(TYPE)  Bool  Mems<TYPE>::operator!=(C Mems<TYPE> &x)C {if(elms()!=x.elms())return true ; REPA(T)if(T[i]!=x[i])return true ; return false;}

#if EE_PRIVATE
T1(TYPE)  void         Mems<TYPE>::copyTo  (  TYPE *dest)C {if(dest)CopyFast(dest  , data(), elmsMem());          }
T1(TYPE)  Mems<TYPE>&  Mems<TYPE>::copyFrom(C TYPE *src )  {        Copy    (data(), src   , elmsMem()); return T;} // use 'Copy' in case 'src' is null
T1(TYPE)  void         Mems<TYPE>:: setFrom(  TYPE* &data, Int elms) {if(data!=T._data){del(); T._data=data; T._elms=elms; data=null;}}
T1(TYPE)  void         Mems<TYPE>:: setTemp(  TYPE*  data, Int elms) {                         T._data=data; T._elms=elms;            }
#endif

T1(TYPE)  Bool  Mems<TYPE>::save(File &f)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Mems<TYPE>::save(File &f)  {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Mems<TYPE>::load(File &f)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE) T1(USER)  Bool  Mems<TYPE>::save(File &f, C USER &user)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f, user))return false; return f.ok();}
T1(TYPE) T1(USER)  Bool  Mems<TYPE>::load(File &f, C USER &user)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f, user))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE) T2(USER, USER1)  Bool  Mems<TYPE>::save(File &f, C USER &user, C USER1 &user1)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f, user, user1))return false; return f.ok();}
T1(TYPE) T2(USER, USER1)  Bool  Mems<TYPE>::load(File &f, C USER &user, C USER1 &user1)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f, user, user1))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE)  Bool  Mems<TYPE>::saveRawData(File &f)C {return f.putN(data(), elms());}
T1(TYPE)  Bool  Mems<TYPE>::loadRawData(File &f)  {return f.getN(data(), elms());}

T1(TYPE)  Bool  Mems<TYPE>::saveRaw(File &f)C {       f.cmpUIntV(elms()) ; saveRawData(f); return f.ok();}
T1(TYPE)  Bool  Mems<TYPE>::loadRaw(File &f)  {setNum(f.decUIntV(      )); loadRawData(f);     if(f.ok())return true; clear(); return false;}

#if EE_PRIVATE
T1(TYPE)  Int   Mems<TYPE>::saveRawSize()C {return CmpUIntVSize(elms())+elmsMem();}

T1(TYPE)  Bool  Mems<TYPE>::_saveRaw(File &f)C {       f.putInt(elms()) ; saveRawData(f); return f.ok();}
T1(TYPE)  Bool  Mems<TYPE>::_loadRaw(File &f)  {setNum(f.getInt(      )); loadRawData(f);     if(f.ok())return true; clear(); return false;}
T1(TYPE)  Bool  Mems<TYPE>::_save   (File &f)C {       f.putInt(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Mems<TYPE>::_load   (File &f)  {setNum(f.getInt(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}
#endif

T1(TYPE)  Mems<TYPE>::~Mems(            )          {del();}
T1(TYPE)  Mems<TYPE>:: Mems(            )          {_data=null; _elms=0;}
T1(TYPE)  Mems<TYPE>:: Mems(  Int   elms)          {Alloc(_data, _elms=Max(elms, 0)); if(ClassFunc<TYPE>::HasNew())FREPA(T)new(&T[i])TYPE;}
T1(TYPE)  Mems<TYPE>:: Mems(C Mems  &src) : Mems() {T=src;}
T1(TYPE)  Mems<TYPE>:: Mems(  Mems &&src) : Mems() {Swap(T, src);}
/******************************************************************************/
// MEMC
/******************************************************************************/
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::clear() {super::clear(); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::del  () {super::del  (); return T;}

T1(TYPE)  Int      Memc<TYPE>::elms    ()C {return super::elms    ();}
T1(TYPE)  UInt     Memc<TYPE>::elmSize ()C {return super::elmSize ();}
T1(TYPE)  UIntPtr  Memc<TYPE>::memUsage()C {return super::memUsage();}
T1(TYPE)  UIntPtr  Memc<TYPE>::elmsMem ()C {return super::elmsMem ();}

T1(TYPE)  TYPE*  Memc<TYPE>::data      (     ) {DEBUG_ASSERT(elmSize()==SIZE(TYPE) || elms()<=1, "'Memc.data' Can't cast to C++ pointer after using 'replaceClass'."); return (TYPE*)super::data();}
T1(TYPE)  TYPE*  Memc<TYPE>::addr      (Int i) {return  (TYPE*)super::addr      (i);}
T1(TYPE)  TYPE*  Memc<TYPE>::addrFirst (     ) {return  (TYPE*)super::addrFirst ( );}
T1(TYPE)  TYPE*  Memc<TYPE>::addrLast  (     ) {return  (TYPE*)super::addrLast  ( );}
T1(TYPE)  TYPE&  Memc<TYPE>::operator[](Int i) {return *(TYPE*)super::operator[](i);}
T1(TYPE)  TYPE&  Memc<TYPE>::operator()(Int i) {return *(TYPE*)super::operator()(i);}
T1(TYPE)  TYPE&  Memc<TYPE>::first     (     ) {return *(TYPE*)super::first     ( );}
T1(TYPE)  TYPE&  Memc<TYPE>::last      (     ) {return *(TYPE*)super::last      ( );}
T1(TYPE)  TYPE&  Memc<TYPE>::New       (     ) {return *(TYPE*)super::New       ( );}
T1(TYPE)  TYPE&  Memc<TYPE>::NewAt     (Int i) {return *(TYPE*)super::NewAt     (i);}

T1(TYPE)  TYPE  Memc<TYPE>::popFirst(       Bool keep_order) {TYPE temp=first(); remove    (0, keep_order); return temp;}
T1(TYPE)  TYPE  Memc<TYPE>::pop     (Int i, Bool keep_order) {TYPE temp=   T[i]; remove    (i, keep_order); return temp;}
T1(TYPE)  TYPE  Memc<TYPE>::pop     (                      ) {TYPE temp= last(); removeLast(             ); return temp;}

T1(TYPE)  C TYPE*  Memc<TYPE>::data      (     )C {return ConstCast(T).data     ( );}
T1(TYPE)  C TYPE*  Memc<TYPE>::addr      (Int i)C {return ConstCast(T).addr     (i);}
T1(TYPE)  C TYPE*  Memc<TYPE>::addrFirst (     )C {return ConstCast(T).addrFirst( );}
T1(TYPE)  C TYPE*  Memc<TYPE>::addrLast  (     )C {return ConstCast(T).addrLast ( );}
T1(TYPE)  C TYPE&  Memc<TYPE>::operator[](Int i)C {return ConstCast(T)          [i];}
T1(TYPE)  C TYPE&  Memc<TYPE>::first     (     )C {return ConstCast(T).first    ( );}
T1(TYPE)  C TYPE&  Memc<TYPE>::last      (     )C {return ConstCast(T).last     ( );}

T1(TYPE)  Int   Memc<TYPE>::index   (C TYPE *elm)C {return super::index   (elm);}
T1(TYPE)  Bool  Memc<TYPE>::contains(C TYPE *elm)C {return super::contains(elm);}

T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::removeLast(                                   ) {super::removeLast(                ); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::remove    (  Int   i  ,        Bool keep_order) {super::remove    (i,    keep_order); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::removeNum (  Int   i  , Int n, Bool keep_order) {super::removeNum (i, n, keep_order); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::removeData(C TYPE *elm,        Bool keep_order) {super::removeData(elm,  keep_order); return T;}

T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::setNum       (Int num          ) {       super::setNum       (num      ); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::setNum       (Int num, Int keep) {       super::setNum       (num, keep); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::setNumZero   (Int num          ) {       super::setNumZero   (num      ); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::setNumZero   (Int num, Int keep) {       super::setNumZero   (num, keep); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::setNumDiscard(Int num          ) {       super::setNumDiscard(num      ); return T;}
T1(TYPE)  Int          Memc<TYPE>::addNum       (Int num          ) {return super::addNum       (num      );          }

T1(TYPE) T1(VALUE)  Bool  Memc<TYPE>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return super::binarySearch(&value, index, (Int(*)(CPtr, CPtr))compare);}

T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::sort(           Int compare(C TYPE &a, C TYPE &b           )) {super::sort(      (Int(*)(CPtr, CPtr      ))compare); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::sort(CPtr user, Int compare(C TYPE &a, C TYPE &b, CPtr user)) {super::sort(user, (Int(*)(CPtr, CPtr, CPtr))compare); return T;}

T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::  reverseOrder(                      ) {super::  reverseOrder(              ); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::randomizeOrder(                      ) {super::randomizeOrder(              ); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::   rotateOrder(Int offset            ) {super::   rotateOrder(offset        ); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::     swapOrder(Int i  , Int j        ) {super::     swapOrder(i, j          ); return T;}
T1(TYPE)  Memc<TYPE>&  Memc<TYPE>::     moveElm  (Int elm, Int new_index) {super::     moveElm  (elm, new_index); return T;}

T1(TYPE)                     Memc<TYPE>&  Memc<TYPE>::operator=(C  Mems  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memc<TYPE>&  Memc<TYPE>::operator=(C  Memc  <TYPE      >  &src) {if(this!=&src     ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE) template<Int size>  Memc<TYPE>&  Memc<TYPE>::operator=(C  Memt  <TYPE, size>  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memc<TYPE>&  Memc<TYPE>::operator=(C  Memb  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memc<TYPE>&  Memc<TYPE>::operator=(C  Memx  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memc<TYPE>&  Memc<TYPE>::operator=(C  Meml  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Memc<TYPE>&  Memc<TYPE>::operator=(C CMemPtr<TYPE, size>  &src) {if(this!=src._memc){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Memc<TYPE>&  Memc<TYPE>::operator=(   Memc  <TYPE      > &&src) {Swap(T, src); return T;}

T1(TYPE)  Bool  Memc<TYPE>::operator==(C Memc<TYPE> &x)C {if(elms()!=x.elms())return false; REPA(T)if(T[i]!=x[i])return false; return true ;}
T1(TYPE)  Bool  Memc<TYPE>::operator!=(C Memc<TYPE> &x)C {if(elms()!=x.elms())return true ; REPA(T)if(T[i]!=x[i])return true ; return false;}

T1(TYPE) T1(EXTENDED)  Memc<TYPE>&  Memc<TYPE>::replaceClass          ()  {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>(); super::_reset(SIZE(EXTENDED), ClassFunc<EXTENDED>::GetNew(), ClassFunc<EXTENDED>::GetDel()); return T;}
T1(TYPE) T1(BASE    )               Memc<TYPE>::operator   Memc<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(  Memc<BASE>*)this;}
T1(TYPE) T1(BASE    )               Memc<TYPE>::operator C Memc<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(C Memc<BASE>*)this;}

T1(TYPE)  Bool  Memc<TYPE>::save(File &f)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memc<TYPE>::save(File &f)  {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memc<TYPE>::load(File &f)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE) T1(USER)  Bool  Memc<TYPE>::save(File &f, C USER &user)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f, user))return false; return f.ok();}
T1(TYPE) T1(USER)  Bool  Memc<TYPE>::load(File &f, C USER &user)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f, user))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE) T2(USER, USER1)  Bool  Memc<TYPE>::save(File &f, C USER &user, C USER1 &user1)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f, user, user1))return false; return f.ok();}
T1(TYPE) T2(USER, USER1)  Bool  Memc<TYPE>::load(File &f, C USER &user, C USER1 &user1)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f, user, user1))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE)  Bool  Memc<TYPE>::saveRaw(File &f)C {return super::saveRaw(f);}
T1(TYPE)  Bool  Memc<TYPE>::loadRaw(File &f)  {return super::loadRaw(f);}

#if EE_PRIVATE
T1(TYPE)  Bool  Memc<TYPE>::_saveRaw(File &f)C {return super::_saveRaw(f);}
T1(TYPE)  Bool  Memc<TYPE>::_loadRaw(File &f)  {return super::_loadRaw(f);}
T1(TYPE)  Bool  Memc<TYPE>::_save   (File &f)C {       f.putInt(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memc<TYPE>::_load   (File &f)  {setNum(f.getInt(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}
#endif

T1(TYPE)  Memc<TYPE>::Memc(            ) : _Memc(SIZE(TYPE)   , ClassFunc<TYPE>::GetNew(), ClassFunc<TYPE>::GetDel()) {}
T1(TYPE)  Memc<TYPE>::Memc(C Memc  &src) : _Memc(src.elmSize(),                src._new  ,                src._del  ) {T=src;}
T1(TYPE)  Memc<TYPE>::Memc(  Memc &&src) : _Memc(            0,                     null ,                     null ) {Swap(T, src);}
/******************************************************************************/
// MEMC ABSTRACT
/******************************************************************************/
T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::clear() {super::clear(); return T;}
T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::del  () {super::del  (); return T;}

T1(TYPE)  Int      MemcAbstract<TYPE>::elms    ()C {return super::elms    ();}
T1(TYPE)  UInt     MemcAbstract<TYPE>::elmSize ()C {return super::elmSize ();}
T1(TYPE)  UIntPtr  MemcAbstract<TYPE>::memUsage()C {return super::memUsage();}
T1(TYPE)  UIntPtr  MemcAbstract<TYPE>::elmsMem ()C {return super::elmsMem ();}

T1(TYPE)  TYPE*  MemcAbstract<TYPE>::data      (     ) {DEBUG_ASSERT(elmSize()==SIZE(TYPE) || elms()<=1, "'MemcAbstract.data' Can't cast to C++ pointer after using 'replaceClass'."); return (TYPE*)super::data();}
T1(TYPE)  TYPE*  MemcAbstract<TYPE>::addr      (Int i) {return  (TYPE*)super::addr      (i);}
T1(TYPE)  TYPE&  MemcAbstract<TYPE>::operator[](Int i) {return *(TYPE*)super::operator[](i);}
T1(TYPE)  TYPE&  MemcAbstract<TYPE>::operator()(Int i) {return *(TYPE*)super::operator()(i);}
T1(TYPE)  TYPE&  MemcAbstract<TYPE>::first     (     ) {return *(TYPE*)super::first     ( );}
T1(TYPE)  TYPE&  MemcAbstract<TYPE>::last      (     ) {return *(TYPE*)super::last      ( );}
T1(TYPE)  TYPE&  MemcAbstract<TYPE>::New       (     ) {return *(TYPE*)super::New       ( );}
T1(TYPE)  TYPE&  MemcAbstract<TYPE>::NewAt     (Int i) {return *(TYPE*)super::NewAt     (i);}

T1(TYPE)  C TYPE*  MemcAbstract<TYPE>::data      (     )C {return ConstCast(T).data ( );}
T1(TYPE)  C TYPE*  MemcAbstract<TYPE>::addr      (Int i)C {return ConstCast(T).addr (i);}
T1(TYPE)  C TYPE&  MemcAbstract<TYPE>::operator[](Int i)C {return ConstCast(T)      [i];}
T1(TYPE)  C TYPE&  MemcAbstract<TYPE>::first     (     )C {return ConstCast(T).first( );}
T1(TYPE)  C TYPE&  MemcAbstract<TYPE>::last      (     )C {return ConstCast(T).last ( );}

T1(TYPE)  Int   MemcAbstract<TYPE>::index   (C TYPE *elm)C {return super::index   (elm);}
T1(TYPE)  Bool  MemcAbstract<TYPE>::contains(C TYPE *elm)C {return super::contains(elm);}

T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::removeLast(                            ) {super::removeLast(               ); return T;}
T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::remove    (  Int   i  , Bool keep_order) {super::remove    (i  , keep_order); return T;}
T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::removeData(C TYPE *elm, Bool keep_order) {super::removeData(elm, keep_order); return T;}

T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::setNum    (Int num) {       super::setNum    (num); return T;}
T1(TYPE)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::setNumZero(Int num) {       super::setNumZero(num); return T;}
T1(TYPE)  Int                  MemcAbstract<TYPE>::addNum    (Int num) {return super::addNum    (num);          }

T1(TYPE) T1(EXTENDED)  MemcAbstract<TYPE>&  MemcAbstract<TYPE>::replaceClass          ()  {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>(); super::_reset(SIZE(EXTENDED), ClassFunc<EXTENDED>::GetNew(), ClassFunc<EXTENDED>::GetDel()); return T;}
T1(TYPE) T1(BASE    )                       MemcAbstract<TYPE>::operator   Memc<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(  Memc<BASE>*)this;}
T1(TYPE) T1(BASE    )                       MemcAbstract<TYPE>::operator C Memc<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(C Memc<BASE>*)this;}

T1(TYPE)  MemcAbstract<TYPE>::MemcAbstract() : _Memc(0, null, null) {}
/******************************************************************************/
// MEMC THREAD SAFE
/******************************************************************************/
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::lockedClear() {super::lockedClear(); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::lockedDel  () {super::lockedDel  (); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::      clear() {super::      clear(); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::      del  () {super::      del  (); return T;}

T1(TYPE)  Int      MemcThreadSafe<TYPE>::elms    ()C {return super::elms    ();}
T1(TYPE)  UInt     MemcThreadSafe<TYPE>::elmSize ()C {return super::elmSize ();}
T1(TYPE)  UIntPtr  MemcThreadSafe<TYPE>::memUsage()C {return super::memUsage();}
T1(TYPE)  UIntPtr  MemcThreadSafe<TYPE>::elmsMem ()C {return super::elmsMem ();}

T1(TYPE)  TYPE*  MemcThreadSafe<TYPE>::lockedData (     ) {DEBUG_ASSERT(elmSize()==SIZE(TYPE) || elms()<=1, "'MemcThreadSafe.data' Can't cast to C++ pointer after using 'replaceClass'."); return (TYPE*)super::lockedData();}
T1(TYPE)  TYPE*  MemcThreadSafe<TYPE>::lockedAddr (Int i) {return  (TYPE*)super::lockedAddr (i);}
T1(TYPE)  TYPE&  MemcThreadSafe<TYPE>::lockedElm  (Int i) {return *(TYPE*)super::lockedElm  (i);}
T1(TYPE)  TYPE&  MemcThreadSafe<TYPE>::lockedFirst(     ) {return *(TYPE*)super::lockedFirst( );}
T1(TYPE)  TYPE&  MemcThreadSafe<TYPE>::lockedLast (     ) {return *(TYPE*)super::lockedLast ( );}
T1(TYPE)  TYPE&  MemcThreadSafe<TYPE>::lockedNew  (     ) {return *(TYPE*)super::lockedNew  ( );}
T1(TYPE)  TYPE&  MemcThreadSafe<TYPE>::lockedNewAt(Int i) {return *(TYPE*)super::lockedNewAt(i);}

T1(TYPE)  TYPE  MemcThreadSafe<TYPE>::lockedPopFirst(       Bool keep_order) {TYPE temp=lockedFirst( ); lockedRemove    (0, keep_order); return temp;}
T1(TYPE)  TYPE  MemcThreadSafe<TYPE>::lockedPop     (Int i, Bool keep_order) {TYPE temp=lockedElm  (i); lockedRemove    (i, keep_order); return temp;}
T1(TYPE)  TYPE  MemcThreadSafe<TYPE>::lockedPop     (                      ) {TYPE temp=lockedLast ( ); lockedRemoveLast(             ); return temp;}

T1(TYPE)  void  MemcThreadSafe<TYPE>::lockedSwapPopFirst(TYPE &dest,        Bool keep_order) {Swap(dest, lockedFirst( )); lockedRemove    (0, keep_order);}
T1(TYPE)  void  MemcThreadSafe<TYPE>::lockedSwapPop     (TYPE &dest, Int i, Bool keep_order) {Swap(dest, lockedElm  (i)); lockedRemove    (i, keep_order);}
T1(TYPE)  void  MemcThreadSafe<TYPE>::lockedSwapPop     (TYPE &dest                        ) {Swap(dest, lockedLast ( )); lockedRemoveLast(             );}
T1(TYPE)  void  MemcThreadSafe<TYPE>::      swapPop     (TYPE &dest                        ) {SyncLocker locker(_lock);   lockedSwapPop   (dest         );}

T1(TYPE)  C TYPE*  MemcThreadSafe<TYPE>::lockedData (     )C {return ConstCast(T).lockedData ( );}
T1(TYPE)  C TYPE*  MemcThreadSafe<TYPE>::lockedAddr (Int i)C {return ConstCast(T).lockedAddr (i);}
T1(TYPE)  C TYPE&  MemcThreadSafe<TYPE>::lockedElm  (Int i)C {return ConstCast(T).lockedElm  (i);}
T1(TYPE)  C TYPE&  MemcThreadSafe<TYPE>::lockedFirst(     )C {return ConstCast(T).lockedFirst( );}
T1(TYPE)  C TYPE&  MemcThreadSafe<TYPE>::lockedLast (     )C {return ConstCast(T).lockedLast ( );}

T1(TYPE)  Int   MemcThreadSafe<TYPE>::lockedIndex   (C TYPE *elm)C {return super::lockedIndex   (elm);}
T1(TYPE)  Bool  MemcThreadSafe<TYPE>::lockedContains(C TYPE *elm)C {return super::lockedContains(elm);}
T1(TYPE)  Bool  MemcThreadSafe<TYPE>::      contains(C TYPE *elm)C {return super::      contains(elm);}

T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::lockedRemoveLast(                            ) {super::lockedRemoveLast(               ); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::lockedRemove    (  Int   i  , Bool keep_order) {super::lockedRemove    (i  , keep_order); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::lockedRemoveData(C TYPE *elm, Bool keep_order) {super::lockedRemoveData(elm, keep_order); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::      removeData(C TYPE *elm, Bool keep_order) {super::      removeData(elm, keep_order); return T;}

T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::setNum    (Int num) {       super::setNum    (num); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::setNumZero(Int num) {       super::setNumZero(num); return T;}
T1(TYPE)  Int                    MemcThreadSafe<TYPE>::addNum    (Int num) {return super::addNum    (num);          }

T1(TYPE) T1(VALUE)  Bool  MemcThreadSafe<TYPE>::lockedBinarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return super::lockedBinarySearch(&value, index, (Int(*)(CPtr, CPtr))compare);}

T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::sort(           Int compare(C TYPE &a, C TYPE &b           )) {super::sort(      (Int(*)(CPtr, CPtr      ))compare); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::sort(CPtr user, Int compare(C TYPE &a, C TYPE &b, CPtr user)) {super::sort(user, (Int(*)(CPtr, CPtr, CPtr))compare); return T;}

T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::   reverseOrder(                      ) {super::   reverseOrder(              ); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>:: randomizeOrder(                      ) {super:: randomizeOrder(              ); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::    rotateOrder(Int offset            ) {super::    rotateOrder(offset        ); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::lockedSwapOrder(Int i  , Int j        ) {super::lockedSwapOrder(i  , j        ); return T;}
T1(TYPE)  MemcThreadSafe<TYPE>&  MemcThreadSafe<TYPE>::lockedMoveElm  (Int elm, Int new_index) {super::lockedMoveElm  (elm, new_index); return T;}

T1(TYPE)  void  MemcThreadSafe<TYPE>::  lock()C {super::  lock();}
T1(TYPE)  void  MemcThreadSafe<TYPE>::unlock()C {super::unlock();}

T1(TYPE)  Bool  MemcThreadSafe<TYPE>::save(File &f)C {SyncLocker locker(_lock);        f.cmpUIntV(elms()) ; FREPA(T)if(!lockedElm(i).save(f))return false; return f.ok();}
T1(TYPE)  Bool  MemcThreadSafe<TYPE>::save(File &f)  {SyncLocker locker(_lock);        f.cmpUIntV(elms()) ; FREPA(T)if(!lockedElm(i).save(f))return false; return f.ok();}
T1(TYPE)  Bool  MemcThreadSafe<TYPE>::load(File &f)  {SyncLocker locker(_lock); setNum(f.decUIntV(      )); FREPA(T)if(!lockedElm(i).load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE)  Bool  MemcThreadSafe<TYPE>::saveRaw(File &f)C {return super::saveRaw(f);}
T1(TYPE)  Bool  MemcThreadSafe<TYPE>::loadRaw(File &f)  {return super::loadRaw(f);}

T1(TYPE)  MemcThreadSafe<TYPE>::MemcThreadSafe() : _MemcThreadSafe(SIZE(TYPE), ClassFunc<TYPE>::GetNew(), ClassFunc<TYPE>::GetDel()) {}
/******************************************************************************/
// MEMT
/******************************************************************************/
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::clear()
{
   if(ClassFunc<TYPE>::HasDel())REPA(T)T[i].~TYPE(); // destroy as the first step
  _elms=0;
   return T;
}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::del()
{
   clear();
   if(_data){Free(_data); _max_elms=SIZE(_temp)/elmSize();}
   return T;
}

template<typename TYPE, Int size>  Int      Memt<TYPE, size>:: elms    ()C {return _elms;}
template<typename TYPE, Int size>  UInt     Memt<TYPE, size>:: elmSize ()C {return SIZE(TYPE);}
template<typename TYPE, Int size>  UIntPtr  Memt<TYPE, size>:: memUsage()C {return SIZE(T) + (_data ? UIntPtr(maxElms())*elmSize() : 0);}
template<typename TYPE, Int size>  UIntPtr  Memt<TYPE, size>::elmsMem  ()C {return                    UIntPtr(   elms())*elmSize()     ;}

template<typename TYPE, Int size>  TYPE*  Memt<TYPE, size>::data      (     ) {                              return _data ? _data    :  (TYPE*)_temp    ;}
template<typename TYPE, Int size>  TYPE&  Memt<TYPE, size>::_element  (Int i) {                              return _data ? _data[i] : ((TYPE*)_temp)[i];}
template<typename TYPE, Int size>  TYPE&  Memt<TYPE, size>::operator[](Int i) {DEBUG_RANGE_ASSERT(i, _elms); return _element(i);}
template<typename TYPE, Int size>  TYPE*  Memt<TYPE, size>::addr      (Int i) {return     InRange(i, _elms)   ?    &_element(i) : null;}
template<typename TYPE, Int size>  TYPE&  Memt<TYPE, size>::first     (     ) {return T[       0];}
template<typename TYPE, Int size>  TYPE&  Memt<TYPE, size>::last      (     ) {return T[elms()-1];}

template<typename TYPE, Int size>  TYPE  Memt<TYPE, size>::popFirst(       Bool keep_order) {TYPE temp=first(); remove    (0, keep_order); return temp;}
template<typename TYPE, Int size>  TYPE  Memt<TYPE, size>::pop     (Int i, Bool keep_order) {TYPE temp=   T[i]; remove    (i, keep_order); return temp;}
template<typename TYPE, Int size>  TYPE  Memt<TYPE, size>::pop     (                      ) {TYPE temp= last(); removeLast(             ); return temp;}

template<typename TYPE, Int size>  C TYPE*  Memt<TYPE, size>::data      (     )C {return ConstCast(T).data ( );}
template<typename TYPE, Int size>  C TYPE*  Memt<TYPE, size>::addr      (Int i)C {return ConstCast(T).addr (i);}
template<typename TYPE, Int size>  C TYPE&  Memt<TYPE, size>::operator[](Int i)C {return ConstCast(T)      [i];}
template<typename TYPE, Int size>  C TYPE&  Memt<TYPE, size>::first     (     )C {return ConstCast(T).first( );}
template<typename TYPE, Int size>  C TYPE&  Memt<TYPE, size>::last      (     )C {return ConstCast(T).last ( );}

template<typename TYPE, Int size>  TYPE&  Memt<TYPE, size>::operator()(Int i)
{
   if(i< 0     )Exit("i<0 inside 'Memt.operator()(Int i)'");
   if(i>=elms())setNumZero(i+1);
   return T[i];
}
template<typename TYPE, Int size>  TYPE&  Memt<TYPE, size>::New  (     ) {return T[addNum(1)];}
template<typename TYPE, Int size>  TYPE&  Memt<TYPE, size>::NewAt(Int i)
{
   Clamp(i, 0, elms());
   Int old_elms=elms(); if(old_elms>=INT_MAX)Exit("'Memt.NewAt' size too big"); _elms++;
   if(Greater(elms(), maxElms())) // elms()>maxElms()
   {
     _max_elms=CeilPow2(elms());
      TYPE *temp=Alloc<TYPE>(maxElms());
      CopyFastN(temp      , &T[0],          i);
      CopyFastN(temp+(i+1), &T[i], old_elms-i);
      Free(_data); _data=temp;
   }else
   if(i<old_elms)
   {
      MoveFastN(&T[i+1], &T[i], old_elms-i);
   }
   TYPE &elm=T[i]; new(&elm)TYPE; return elm; // create as the last step
}

template<typename TYPE, Int size>  Int  Memt<TYPE, size>::index(C TYPE *elm)C
{
   UIntPtr i=UIntPtr(elm)-UIntPtr(data());
   if(i<elmsMem())return Int(i/elmSize()); // unsigned compare will already guarantee "i>=0 && "
   return -1;
}
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::contains(C TYPE *elm)C {return index(elm)>=0;}

template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::remove(Int i, Bool keep_order)
{
   if(InRange(i, T))
   {
      T[i].~TYPE(); // destroy as the first step
      if(i<elms()-1) // if this is not the last element
      {
         if(keep_order)MoveFastN(&T[i], &T[     i+1], elms()-i-1);
         else          CopyFast ( T[i],  T[elms()-1]);
      }
     _elms--;
   }
   return T;
}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::removeLast(                            ) {return remove(elms()-1              );}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::removeData(C TYPE *elm, Bool keep_order) {return remove(index(elm), keep_order);}

template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::reserve(Int num)
{
   if(Greater(num, maxElms())) // num>maxElms()
   {
     _max_elms=CeilPow2(num);
      TYPE *temp=Alloc<TYPE>(maxElms());
      CopyFastN(temp, data(), elms());
      Free(_data); _data=temp;
   }
   return T;
}

template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::setNum(Int num)
{
   MAX(num, 0);
   if( num>elms()) // add elements
   {
      reserve(num);
      Int old_elms=elms(); _elms=num;
      if(ClassFunc<TYPE>::HasNew())for(Int i=old_elms; i<elms(); i++)new(&T[i])TYPE; // create as the last step
   }else
   if(num<elms()) // remove elements
   {
      if(ClassFunc<TYPE>::HasDel())for(Int i=elms(); --i>=num; )T[i].~TYPE(); // destroy as the first step
     _elms=num;
   }
   return T;
}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::setNumZero(Int num)
{
   MAX(num, 0);
   if( num>elms()) // add elements
   {
      reserve(num);
      Int old_elms=elms(); _elms=num;
      ZeroFastN(&T[old_elms], elms()-old_elms);
      if(ClassFunc<TYPE>::HasNew())for(Int i=old_elms; i<elms(); i++)new(&T[i])TYPE; // create as the last step
   }else
   if(num<elms()) // remove elements
   {
      if(ClassFunc<TYPE>::HasDel())for(Int i=elms(); --i>=num; )T[i].~TYPE(); // destroy as the first step
     _elms=num;
   }
   return T;
}

template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::setNum(Int num, Int keep)
{
   MAX(num, 0);
   Clamp(keep, 0, Min(elms(), num));
   if(ClassFunc<TYPE>::HasDel())for(Int i=elms(); --i>=keep; )T[i].~TYPE(); // delete unkept elements, destroy as the first step
   if(Greater(num, maxElms())) // resize memory, num>maxElms()
   {
     _elms=keep; // set '_elms' before 'reserve' to copy only 'keep' elements
      reserve(num);
   }
  _elms=num; // set '_elms' before accessing new elements to avoid range assert
   if(ClassFunc<TYPE>::HasNew())for(Int i=keep; i<elms(); i++)new(&T[i])TYPE; // create new elements, create as the last step
   return T;
}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::setNumZero(Int num, Int keep)
{
   MAX(num, 0);
   Clamp(keep, 0, Min(elms(), num));
   if(ClassFunc<TYPE>::HasDel())for(Int i=elms(); --i>=keep; )T[i].~TYPE(); // delete unkept elements, destroy as the first step
   if(Greater(num, maxElms())) // resize memory, num>maxElms()
   {
     _elms=keep; // set '_elms' before 'reserve' to copy only 'keep' elements
      reserve(num);
   }
  _elms=num; // set '_elms' before accessing new elements to avoid range assert
   ZeroFastN(&_element(keep), elms()-keep); // zero new elements, have to use '_element' to avoid out of range errors
   if(ClassFunc<TYPE>::HasNew())for(Int i=keep; i<elms(); i++)new(&T[i])TYPE; // create new elements, create as the last step
   return T;
}

template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::setNumDiscard(Int num)
{
   MAX(num, 0);
   if( num!=elms())
   {
      if(Greater(num, maxElms())) // resize memory, num>maxElms()
      {
         clear(); // clear before 'reserve' to skip copying old elements
         reserve(num);
        _elms=num; // set '_elms' before accessing new elements to avoid range assert
         if(ClassFunc<TYPE>::HasNew())FREPA(T)new(&T[i])TYPE; // create new elements, create as the last step
      }else
      if(num>elms()) // add elements in existing memory
      {
         Int old_elms=elms(); _elms=num; // set '_elms' before accessing new elements to avoid range assert
         if(ClassFunc<TYPE>::HasNew())for(Int i=old_elms; i<elms(); i++)new(&T[i])TYPE; // create as the last step
      }else
    //if(num<elms()) // remove elements, "if" not needed because we already know that "num!=elms && !(num>elms())"
      {
         if(ClassFunc<TYPE>::HasDel())for(Int i=elms(); --i>=num; )T[i].~TYPE(); // destroy as the first step
        _elms=num;
      }
   }
   return T;
}
#if EE_PRIVATE
template<typename TYPE, Int size>  void  Memt<TYPE, size>::minNumDiscard(Int num)
{
   if(Greater(num, elms())) // num>elms()
   {
      if(Greater(num, maxElms())) // resize memory, num>maxElms()
      {
         clear(); // clear before 'reserve' to skip copying old elements
         reserve(num);
        _elms=num; // set '_elms' before accessing new elements to avoid range assert
         if(ClassFunc<TYPE>::HasNew())FREPA(T)new(&T[i])TYPE; // create new elements, create as the last step
      }else // add elements in existing memory
      {
         Int old_elms=elms(); _elms=num; // set '_elms' before accessing new elements to avoid range assert
         if(ClassFunc<TYPE>::HasNew())for(Int i=old_elms; i<elms(); i++)new(&T[i])TYPE; // create as the last step
      }
   }
}
#endif

template<typename TYPE, Int size>  Int  Memt<TYPE, size>::addNum(Int num) {Int index=elms(); Long new_elms=Long(index)+num; if(new_elms>INT_MAX)Exit("'Memt.addNum' size too big"); setNum((Int)new_elms); return index;}

template<typename TYPE, Int size> T1(VALUE)  Bool  Memt<TYPE, size>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return _BinarySearch(data(), elms(), elmSize(), &value, index, (Int(*)(CPtr, CPtr))compare);}

template<typename TYPE, Int size>  Memt<TYPE, size>&   Memt<TYPE, size>::sort(           Int compare(C TYPE &a, C TYPE &b           )) {_Sort(data(), elms(), elmSize(),       (Int(*)(CPtr, CPtr      ))compare); return T;}
template<typename TYPE, Int size>  Memt<TYPE, size>&   Memt<TYPE, size>::sort(CPtr user, Int compare(C TYPE &a, C TYPE &b, CPtr user)) {_Sort(data(), elms(), elmSize(), user, (Int(*)(CPtr, CPtr, CPtr))compare); return T;}

template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::  reverseOrder(                      ) {  _ReverseOrder(data(), elms(), elmSize()                ); return T;}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::randomizeOrder(                      ) {_RandomizeOrder(data(), elms(), elmSize()                ); return T;}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::   rotateOrder(Int offset            ) {   _RotateOrder(data(), elms(), elmSize(), offset        ); return T;}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::     moveElm  (Int elm, Int new_index) {     _MoveElm  (data(), elms(), elmSize(), elm, new_index); return T;}
template<typename TYPE, Int size>  Memt<TYPE, size>&  Memt<TYPE, size>::     swapOrder(Int i  , Int j        ) {if(InRange(i, T) && InRange(j, T))        Swap(T[i], T[j]); return T;}

template<typename TYPE, Int size>                         Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C  Mems  <TYPE          > &src) {                         setNum(src.elms()); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int size>                         Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C  Memc  <TYPE          > &src) {                         setNum(src.elms()); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int size>                         Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C  Memt  <TYPE,     size> &src) {if(this!=Ptr(&src     )){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int size> template<Int src_size>  Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C  Memt  <TYPE, src_size> &src) {if(this!=Ptr(&src     )){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int size>                         Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C  Memb  <TYPE          > &src) {                         setNum(src.elms()); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int size>                         Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C  Memx  <TYPE          > &src) {                         setNum(src.elms()); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int size>                         Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C  Meml  <TYPE          > &src) {                         setNum(src.elms()); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int size> template<Int src_size>  Memt<TYPE, size>&  Memt<TYPE, size>::operator=(C CMemPtr<TYPE, src_size> &src) {if(this!=Ptr(src._memt)){setNum(src.elms()); FREPAO(T)=src[i];} return T;}

template<typename TYPE, Int Memt_elms>                         MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C  Mems  <TYPE           > &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C  Memc  <TYPE           > &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms> template<Int src_size>  MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C  Memt  <TYPE,  src_size> &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C  MemtN <TYPE, Memt_elms> &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C  Memb  <TYPE           > &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C  Memx  <TYPE           > &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C  Meml  <TYPE           > &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}
template<typename TYPE, Int Memt_elms> template<Int src_size>  MemtN<TYPE, Memt_elms>&  MemtN<TYPE, Memt_elms>::operator=(C CMemPtr<TYPE,  src_size> &src) {Memt<TYPE, SIZE(TYPE)*Memt_elms>::operator=(src); return T;}

template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::operator==(C Memt<TYPE, size> &x)C {if(elms()!=x.elms())return false; REPA(T)if(T[i]!=x[i])return false; return true ;}
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::operator!=(C Memt<TYPE, size> &x)C {if(elms()!=x.elms())return true ; REPA(T)if(T[i]!=x[i])return true ; return false;}

template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::save(File &f)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::save(File &f)  {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::load(File &f)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}

template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::saveRawData(File &f)C {return f.putN(data(), elms());}
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::loadRawData(File &f)  {return f.getN(data(), elms());}

template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::saveRaw(File &f)C {       f.cmpUIntV(elms()) ; saveRawData(f); return f.ok();}
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::loadRaw(File &f)  {setNum(f.decUIntV(      )); loadRawData(f);     if(f.ok())return true; clear(); return false;}

#if EE_PRIVATE
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::loadRawDataFast(File &f) {return f.getFastN(data(), elms());}

template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::_saveRaw(File &f)C {       f.putInt  (elms()) ; saveRawData(f); return f.ok();}
template<typename TYPE, Int size>  Bool  Memt<TYPE, size>::_loadRaw(File &f)  {setNum(f.getInt  (      )); loadRawData(f);     if(f.ok())return true; clear(); return false;}
#endif

template<typename TYPE, Int size>  Memt<TYPE, size>::~Memt(           )          {del();}
template<typename TYPE, Int size>  Memt<TYPE, size>:: Memt(           )          {_data=null; _elms=0; _max_elms=SIZE(_temp)/elmSize();} // '_data' being set to 'null' instead of '_temp' allows for moving 'Memt' into another memory address
template<typename TYPE, Int size>  Memt<TYPE, size>:: Memt(C Memt &src) : Memt() {T=src;}
/******************************************************************************/
// MEMB
/******************************************************************************/
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::clear() {super::clear(); return T;}
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::del  () {super::del  (); return T;}

T1(TYPE)  Int      Memb<TYPE>::  elms    ()C {return super::  elms    ();}
T1(TYPE)  UInt     Memb<TYPE>::  elmSize ()C {return super::  elmSize ();}
T1(TYPE)  UInt     Memb<TYPE>::blockElms ()C {return super::blockElms ();}
T1(TYPE)  UIntPtr  Memb<TYPE>::  memUsage()C {return super::  memUsage();}

T1(TYPE)  TYPE*  Memb<TYPE>::addr      (Int i) {return  (TYPE*)super::addr      (i);}
T1(TYPE)  TYPE&  Memb<TYPE>::operator[](Int i) {return *(TYPE*)super::operator[](i);}
T1(TYPE)  TYPE&  Memb<TYPE>::operator()(Int i) {return *(TYPE*)super::operator()(i);}
T1(TYPE)  TYPE&  Memb<TYPE>::first     (     ) {return *(TYPE*)super::first     ( );}
T1(TYPE)  TYPE&  Memb<TYPE>::last      (     ) {return *(TYPE*)super::last      ( );}
T1(TYPE)  TYPE&  Memb<TYPE>::New       (     ) {return *(TYPE*)super::New       ( );}
T1(TYPE)  TYPE&  Memb<TYPE>::NewAt     (Int i) {return *(TYPE*)super::NewAt     (i);}

T1(TYPE)  TYPE  Memb<TYPE>::popFirst(       Bool keep_order) {TYPE temp=first(); remove    (0, keep_order); return temp;}
T1(TYPE)  TYPE  Memb<TYPE>::pop     (Int i, Bool keep_order) {TYPE temp=   T[i]; remove    (i, keep_order); return temp;}
T1(TYPE)  TYPE  Memb<TYPE>::pop     (                      ) {TYPE temp= last(); removeLast(             ); return temp;}

T1(TYPE)  C TYPE*  Memb<TYPE>::addr      (Int i)C {return ConstCast(T).addr (i);}
T1(TYPE)  C TYPE&  Memb<TYPE>::operator[](Int i)C {return ConstCast(T)      [i];}
T1(TYPE)  C TYPE&  Memb<TYPE>::first     (     )C {return ConstCast(T).first( );}
T1(TYPE)  C TYPE&  Memb<TYPE>::last      (     )C {return ConstCast(T).last ( );}

T1(TYPE)  Int   Memb<TYPE>::index   (C TYPE *elm)C {return super::index   (elm);}
T1(TYPE)  Bool  Memb<TYPE>::contains(C TYPE *elm)C {return super::contains(elm);}

T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::removeLast(                            ) {super::removeLast(               ); return T;}
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::remove    (  Int   i  , Bool keep_order) {super::remove    (i  , keep_order); return T;}
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::removeData(C TYPE *elm, Bool keep_order) {super::removeData(elm, keep_order); return T;}

T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::setNum    (Int num) {       super::setNum    (num); return T;}
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::setNumZero(Int num) {       super::setNumZero(num); return T;}
T1(TYPE)  Int          Memb<TYPE>::addNum    (Int num) {return super::addNum    (num);          }

T1(TYPE) T1(VALUE)  Bool  Memb<TYPE>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return super::binarySearch(&value, index, (Int(*)(CPtr, CPtr))compare);}

T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::sort(           Int compare(C TYPE &a, C TYPE &b           )) {super::sort(      (Int(*)(CPtr, CPtr      ))compare); return T;}
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::sort(CPtr user, Int compare(C TYPE &a, C TYPE &b, CPtr user)) {super::sort(user, (Int(*)(CPtr, CPtr, CPtr))compare); return T;}

T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::reverseOrder(                      ) {super::reverseOrder(              ); return T;}
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::   swapOrder(Int i  , Int j        ) {super::   swapOrder(i  , j        ); return T;}
T1(TYPE)  Memb<TYPE>&  Memb<TYPE>::     moveElm(Int elm, Int new_index) {super::     moveElm(elm, new_index); return T;}

T1(TYPE)                     Memb<TYPE>&  Memb<TYPE>::operator=(C  Mems  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memb<TYPE>&  Memb<TYPE>::operator=(C  Memc  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Memb<TYPE>&  Memb<TYPE>::operator=(C  Memt  <TYPE, size>  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memb<TYPE>&  Memb<TYPE>::operator=(C  Memb  <TYPE      >  &src) {if(this!=&src     ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Memb<TYPE>&  Memb<TYPE>::operator=(C  Memx  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memb<TYPE>&  Memb<TYPE>::operator=(C  Meml  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Memb<TYPE>&  Memb<TYPE>::operator=(C CMemPtr<TYPE, size>  &src) {if(this!=src._memb){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Memb<TYPE>&  Memb<TYPE>::operator=(   Memb  <TYPE      > &&src) {Swap(T, src); return T;}

T1(TYPE) T1(EXTENDED)  Memb<TYPE>&  Memb<TYPE>::replaceClass          ()  {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>(); super::_reset(SIZE(EXTENDED), _block_elms, ClassFunc<EXTENDED>::GetNew(), ClassFunc<EXTENDED>::GetDel()); return T;}
T1(TYPE) T1(BASE    )               Memb<TYPE>::operator   Memb<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(  Memb<BASE>*)this;}
T1(TYPE) T1(BASE    )               Memb<TYPE>::operator C Memb<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(C Memb<BASE>*)this;}

T1(TYPE)  Bool  Memb<TYPE>::save(File &f)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memb<TYPE>::save(File &f)  {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memb<TYPE>::load(File &f)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE)  Bool  Memb<TYPE>::saveRaw(File &f)C {return super::saveRaw(f);}
T1(TYPE)  Bool  Memb<TYPE>::loadRaw(File &f)  {return super::loadRaw(f);}

T1(TYPE)  Memb<TYPE>::Memb(Int block_elms) : _Memb(SIZE(TYPE)   ,     block_elms , ClassFunc<TYPE>::GetNew(), ClassFunc<TYPE>::GetDel()) {}
T1(TYPE)  Memb<TYPE>::Memb(C Memb  &src  ) : _Memb(src.elmSize(), src.blockElms(),                src._new  ,                src._del  ) {T=src;}
T1(TYPE)  Memb<TYPE>::Memb(  Memb &&src  ) : _Memb(            0,               0,                     null ,                     null ) {Swap(T, src);}
/******************************************************************************/
// MEMB ABSTRACT
/******************************************************************************/
T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::clear() {super::clear(); return T;}
T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::del  () {super::del  (); return T;}

T1(TYPE)  Int   MembAbstract<TYPE>::  elms   ()C {return super::  elms   ();}
T1(TYPE)  UInt  MembAbstract<TYPE>::  elmSize()C {return super::  elmSize();}
T1(TYPE)  UInt  MembAbstract<TYPE>::blockElms()C {return super::blockElms();}

T1(TYPE)  TYPE*  MembAbstract<TYPE>::addr      (Int i) {return  (TYPE*)super::addr      (i);}
T1(TYPE)  TYPE&  MembAbstract<TYPE>::operator[](Int i) {return *(TYPE*)super::operator[](i);}
T1(TYPE)  TYPE&  MembAbstract<TYPE>::operator()(Int i) {return *(TYPE*)super::operator()(i);}
T1(TYPE)  TYPE&  MembAbstract<TYPE>::first     (     ) {return *(TYPE*)super::first     ( );}
T1(TYPE)  TYPE&  MembAbstract<TYPE>::last      (     ) {return *(TYPE*)super::last      ( );}
T1(TYPE)  TYPE&  MembAbstract<TYPE>::New       (     ) {return *(TYPE*)super::New       ( );}
T1(TYPE)  TYPE&  MembAbstract<TYPE>::NewAt     (Int i) {return *(TYPE*)super::NewAt     (i);}

T1(TYPE)  C TYPE*  MembAbstract<TYPE>::addr      (Int i)C {return ConstCast(T).addr (i);}
T1(TYPE)  C TYPE&  MembAbstract<TYPE>::operator[](Int i)C {return ConstCast(T)      [i];}
T1(TYPE)  C TYPE&  MembAbstract<TYPE>::first     (     )C {return ConstCast(T).first( );}
T1(TYPE)  C TYPE&  MembAbstract<TYPE>::last      (     )C {return ConstCast(T).last ( );}

T1(TYPE)  Int   MembAbstract<TYPE>::index   (C TYPE *elm)C {return super::index   (elm);}
T1(TYPE)  Bool  MembAbstract<TYPE>::contains(C TYPE *elm)C {return super::contains(elm);}

T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::removeLast(                            ) {super::removeLast(               ); return T;}
T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::remove    (  Int   i  , Bool keep_order) {super::remove    (i  , keep_order); return T;}
T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::removeData(C TYPE *elm, Bool keep_order) {super::removeData(elm, keep_order); return T;}

T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::setNum    (Int num) {       super::setNum    (num); return T;}
T1(TYPE)  MembAbstract<TYPE>&  MembAbstract<TYPE>::setNumZero(Int num) {       super::setNumZero(num); return T;}
T1(TYPE)  Int                  MembAbstract<TYPE>::addNum    (Int num) {return super::addNum    (num);          }

T1(TYPE) T1(EXTENDED)  MembAbstract<TYPE>&  MembAbstract<TYPE>::replaceClass          ()  {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>(); super::_reset(SIZE(EXTENDED), _block_elms, ClassFunc<EXTENDED>::GetNew(), ClassFunc<EXTENDED>::GetDel()); return T;}
T1(TYPE) T1(BASE    )                       MembAbstract<TYPE>::operator   Memb<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(  Memb<BASE>*)this;}
T1(TYPE) T1(BASE    )                       MembAbstract<TYPE>::operator C Memb<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(C Memb<BASE>*)this;}

T1(TYPE)  MembAbstract<TYPE>::MembAbstract(Int block_elms) : _Memb(0, block_elms, null, null) {}
/******************************************************************************/
// MEMB CONST
/******************************************************************************/
T1(TYPE)  TYPE*  MembConst<TYPE>::addr      (Int i)C {return ConstCast(T).Memb<TYPE>::addr      (i);}
T1(TYPE)  TYPE&  MembConst<TYPE>::operator[](Int i)C {return ConstCast(T).Memb<TYPE>::operator[](i);}
T1(TYPE)  TYPE&  MembConst<TYPE>::first     (     )C {return ConstCast(T).Memb<TYPE>::first     ( );}
T1(TYPE)  TYPE&  MembConst<TYPE>::last      (     )C {return ConstCast(T).Memb<TYPE>::last      ( );}

T1(TYPE) T1(BASE)  MembConst<TYPE>::operator   MembConst<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE>(); return *(  MembConst<BASE>*)this;}
T1(TYPE) T1(BASE)  MembConst<TYPE>::operator C MembConst<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE>(); return *(C MembConst<BASE>*)this;}
/******************************************************************************/
// MEMX
/******************************************************************************/
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::clear() {super::clear(); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::del  () {super::del  (); return T;}

T1(TYPE)  Int      Memx<TYPE>::  absElms ()C {return super::  absElms ();}
T1(TYPE)  Int      Memx<TYPE>::validElms ()C {return super::validElms ();}
T1(TYPE)  Int      Memx<TYPE>::     elms ()C {return super::     elms ();}
T1(TYPE)  UInt     Memx<TYPE>::  elmSize ()C {return super::  elmSize ();}
T1(TYPE)  UIntPtr  Memx<TYPE>::  memUsage()C {return super::  memUsage();}

T1(TYPE)  TYPE&  Memx<TYPE>::    absElm(Int i) {return *(TYPE*)super::    absElm(i);}
T1(TYPE)  TYPE&  Memx<TYPE>::  validElm(Int i) {return *(TYPE*)super::  validElm(i);}
T1(TYPE)  TYPE*  Memx<TYPE>:: addr     (Int i) {return  (TYPE*)super:: addr     (i);}
T1(TYPE)  TYPE*  Memx<TYPE>:: addrFirst(     ) {return  (TYPE*)super:: addrFirst( );}
T1(TYPE)  TYPE*  Memx<TYPE>:: addrLast (     ) {return  (TYPE*)super:: addrLast ( );}
T1(TYPE)  TYPE&  Memx<TYPE>::operator[](Int i) {return *(TYPE*)super::operator[](i);}
T1(TYPE)  TYPE&  Memx<TYPE>::     first(     ) {return *(TYPE*)super::     first( );}
T1(TYPE)  TYPE&  Memx<TYPE>::      last(     ) {return *(TYPE*)super::      last( );}
T1(TYPE)  TYPE&  Memx<TYPE>::     New  (     ) {return *(TYPE*)super::     New  ( );}
T1(TYPE)  TYPE&  Memx<TYPE>::     NewAt(Int i) {return *(TYPE*)super::     NewAt(i);}

T1(TYPE)  C TYPE&  Memx<TYPE>::    absElm(Int i)C {return ConstCast(T).   absElm(i);}
T1(TYPE)  C TYPE&  Memx<TYPE>::  validElm(Int i)C {return ConstCast(T). validElm(i);}
T1(TYPE)  C TYPE*  Memx<TYPE>:: addr     (Int i)C {return ConstCast(T).addr     (i);}
T1(TYPE)  C TYPE*  Memx<TYPE>:: addrFirst(     )C {return ConstCast(T).addrFirst( );}
T1(TYPE)  C TYPE*  Memx<TYPE>:: addrLast (     )C {return ConstCast(T).addrLast ( );}
T1(TYPE)  C TYPE&  Memx<TYPE>::operator[](Int i)C {return ConstCast(T)          [i];}
T1(TYPE)  C TYPE&  Memx<TYPE>::     first(     )C {return ConstCast(T).    first( );}
T1(TYPE)  C TYPE&  Memx<TYPE>::      last(     )C {return ConstCast(T).     last( );}

T1(TYPE)  Int   Memx<TYPE>::validToAbsIndex(  Int valid)C {return super::validToAbsIndex(valid);}
T1(TYPE)  Int   Memx<TYPE>::absToValidIndex(  Int   abs)C {return super::absToValidIndex(abs  );}
T1(TYPE)  Bool  Memx<TYPE>::absIndexIsValid(  Int   abs)C {return super::absIndexIsValid(abs  );}
T1(TYPE)  Int   Memx<TYPE>::validIndex     (C TYPE *elm)C {return super::validIndex     (elm  );}
T1(TYPE)  Int   Memx<TYPE>::  absIndex     (C TYPE *elm)C {return super::  absIndex     (elm  );}
T1(TYPE)  Bool  Memx<TYPE>::  contains     (C TYPE *elm)C {return super::  contains     (elm  );}

T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::removeAbs  (  Int   i  , Bool keep_order) {super::removeAbs  (i  , keep_order); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::removeValid(  Int   i  , Bool keep_order) {super::removeValid(i  , keep_order); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::removeData (C TYPE *elm, Bool keep_order) {super::removeData (elm, keep_order); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::removeLast (                            ) {super::removeLast (               ); return T;}

T1(TYPE) T1(VALUE)  Bool  Memx<TYPE>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return super::binarySearch(&value, index, (Int(*)(CPtr, CPtr))compare);}

T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::sort(           Int compare(C TYPE &a, C TYPE &b           )) {super::sort(      (Int(*)(CPtr, CPtr      ))compare); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::sort(CPtr user, Int compare(C TYPE &a, C TYPE &b, CPtr user)) {super::sort(user, (Int(*)(CPtr, CPtr, CPtr))compare); return T;}

T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::reverseOrder(                      ) {super::reverseOrder(              ); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::   swapOrder(Int i  , Int j        ) {super::   swapOrder(i  , j        ); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::moveElm     (Int elm, Int new_index) {super::moveElm     (elm, new_index); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::moveToStart (Int elm               ) {super::moveToStart (elm           ); return T;}
T1(TYPE)  Memx<TYPE>&  Memx<TYPE>::moveToEnd   (Int elm               ) {super::moveToEnd   (elm           ); return T;}

T1(TYPE)                     Memx<TYPE>&  Memx<TYPE>::operator=(C  Mems  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memx<TYPE>&  Memx<TYPE>::operator=(C  Memc  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Memx<TYPE>&  Memx<TYPE>::operator=(C  Memt  <TYPE, size>  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memx<TYPE>&  Memx<TYPE>::operator=(C  Memb  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Memx<TYPE>&  Memx<TYPE>::operator=(C  Memx  <TYPE      >  &src) {if(this!=&src     ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Memx<TYPE>&  Memx<TYPE>::operator=(C  Meml  <TYPE      >  &src) {                    setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Memx<TYPE>&  Memx<TYPE>::operator=(C CMemPtr<TYPE, size>  &src) {if(this!=src._memx){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Memx<TYPE>&  Memx<TYPE>::operator=(   Memx  <TYPE      > &&src) {Swap(T, src); return T;}

T1(TYPE) T1(EXTENDED)  Memx<TYPE>&  Memx<TYPE>::replaceClass          ()  {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>(); super::_reset(SIZE(EXTENDED), _abs.blockElms(), ClassFunc<EXTENDED>::GetNew(), ClassFunc<EXTENDED>::GetDel()); return T;}
T1(TYPE) T1(BASE    )               Memx<TYPE>::operator   Memx<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(  Memx<BASE>*)this;}
T1(TYPE) T1(BASE    )               Memx<TYPE>::operator C Memx<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(C Memx<BASE>*)this;}

T1(TYPE)  Bool  Memx<TYPE>::save(File &f)C {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memx<TYPE>::save(File &f)  {       f.cmpUIntV(elms()) ; FREPA(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Memx<TYPE>::load(File &f)  {setNum(f.decUIntV(      )); FREPA(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE)  Memx<TYPE>::Memx(Int block_elms) : _Memx(SIZE(TYPE)   , block_elms          , ClassFunc<TYPE>::GetNew(), ClassFunc<TYPE>::GetDel()) {}
T1(TYPE)  Memx<TYPE>::Memx(C Memx  &src  ) : _Memx(src.elmSize(), src._abs.blockElms(),                src._new  ,                src._del  ) {T=src;}
T1(TYPE)  Memx<TYPE>::Memx(  Memx &&src  ) : _Memx(            0,                    0,                     null ,                     null ) {Swap(T, src);}
/******************************************************************************/
// MEMX ABSTRACT
/******************************************************************************/
T1(TYPE)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::clear() {super::clear(); return T;}
T1(TYPE)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::del  () {super::del  (); return T;}

T1(TYPE)  Int      MemxAbstract<TYPE>::  absElms ()C {return super::  absElms ();}
T1(TYPE)  Int      MemxAbstract<TYPE>::validElms ()C {return super::validElms ();}
T1(TYPE)  Int      MemxAbstract<TYPE>::     elms ()C {return super::     elms ();}
T1(TYPE)  UInt     MemxAbstract<TYPE>::  elmSize ()C {return super::  elmSize ();}
T1(TYPE)  UIntPtr  MemxAbstract<TYPE>::  memUsage()C {return super::  memUsage();}

T1(TYPE)  TYPE&  MemxAbstract<TYPE>::    absElm(Int i) {return *(TYPE*)super::    absElm(i);}
T1(TYPE)  TYPE&  MemxAbstract<TYPE>::  validElm(Int i) {return *(TYPE*)super::  validElm(i);}
T1(TYPE)  TYPE*  MemxAbstract<TYPE>::      addr(Int i) {return  (TYPE*)super::      addr(i);}
T1(TYPE)  TYPE&  MemxAbstract<TYPE>::operator[](Int i) {return *(TYPE*)super::operator[](i);}
T1(TYPE)  TYPE&  MemxAbstract<TYPE>::     first(     ) {return *(TYPE*)super::     first( );}
T1(TYPE)  TYPE&  MemxAbstract<TYPE>::      last(     ) {return *(TYPE*)super::      last( );}
T1(TYPE)  TYPE&  MemxAbstract<TYPE>::     New  (     ) {return *(TYPE*)super::     New  ( );}
T1(TYPE)  TYPE&  MemxAbstract<TYPE>::     NewAt(Int i) {return *(TYPE*)super::     NewAt(i);}

T1(TYPE)  C TYPE&  MemxAbstract<TYPE>::    absElm(Int i)C {return ConstCast(T).  absElm(i);}
T1(TYPE)  C TYPE&  MemxAbstract<TYPE>::  validElm(Int i)C {return ConstCast(T).validElm(i);}
T1(TYPE)  C TYPE*  MemxAbstract<TYPE>::      addr(Int i)C {return ConstCast(T).    addr(i);}
T1(TYPE)  C TYPE&  MemxAbstract<TYPE>::operator[](Int i)C {return ConstCast(T)         [i];}
T1(TYPE)  C TYPE&  MemxAbstract<TYPE>::     first(     )C {return ConstCast(T).   first( );}
T1(TYPE)  C TYPE&  MemxAbstract<TYPE>::      last(     )C {return ConstCast(T).    last( );}

T1(TYPE)  Int   MemxAbstract<TYPE>::validToAbsIndex(  Int valid)C {return super::validToAbsIndex(valid);}
T1(TYPE)  Int   MemxAbstract<TYPE>::absToValidIndex(  Int   abs)C {return super::absToValidIndex(abs  );}
T1(TYPE)  Int   MemxAbstract<TYPE>::validIndex     (C TYPE *elm)C {return super::validIndex     (elm  );}
T1(TYPE)  Int   MemxAbstract<TYPE>::  absIndex     (C TYPE *elm)C {return super::  absIndex     (elm  );}
T1(TYPE)  Bool  MemxAbstract<TYPE>::  contains     (C TYPE *elm)C {return super::  contains     (elm  );}

T1(TYPE)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::removeAbs  (  Int   i  , Bool keep_order) {super::removeAbs  (i  , keep_order); return T;}
T1(TYPE)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::removeValid(  Int   i  , Bool keep_order) {super::removeValid(i  , keep_order); return T;}
T1(TYPE)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::removeData (C TYPE *elm, Bool keep_order) {super::removeData (elm, keep_order); return T;}
T1(TYPE)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::removeLast (                            ) {super::removeLast (               ); return T;}

T1(TYPE) T1(EXTENDED)  MemxAbstract<TYPE>&  MemxAbstract<TYPE>::replaceClass          ()  {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>(); super::_reset(SIZE(EXTENDED), _abs.blockElms(), ClassFunc<EXTENDED>::GetNew(), ClassFunc<EXTENDED>::GetDel()); return T;}
T1(TYPE) T1(BASE    )                       MemxAbstract<TYPE>::operator   Memx<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(  Memx<BASE>*)this;}
T1(TYPE) T1(BASE    )                       MemxAbstract<TYPE>::operator C Memx<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE    >(); return *(C Memx<BASE>*)this;}

T1(TYPE)  MemxAbstract<TYPE>::MemxAbstract(Int block_elms) : _Memx(0, block_elms, null, null) {}
/******************************************************************************/
// MEML
/******************************************************************************/
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::del  () {super::del  (); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::clear() {super::clear(); return T;}

T1(TYPE)  Int      Meml<TYPE>::elms    ()C {return super::elms    ();}
T1(TYPE)  UInt     Meml<TYPE>::elmSize ()C {return super::elmSize ();}
T1(TYPE)  UIntPtr  Meml<TYPE>::memUsage()C {return super::memUsage();}

T1(TYPE)  TYPE*  Meml<TYPE>::addr      (Int       i   ) {return  (TYPE*)super::addr      (i);}
T1(TYPE)  TYPE&  Meml<TYPE>::operator[](Int       i   ) {return *(TYPE*)super::operator[](i);}
T1(TYPE)  TYPE&  Meml<TYPE>::operator()(Int       i   ) {return *(TYPE*)super::operator()(i);}
T1(TYPE)  TYPE&  Meml<TYPE>::operator[](MemlNode *node) {return *(TYPE*) node->data      ( );}
T1(TYPE)  TYPE&  Meml<TYPE>::New       (              ) {return *(TYPE*)super::New       ( );}
T1(TYPE)  TYPE&  Meml<TYPE>::NewAt     (Int       i   ) {return *(TYPE*)super::NewAt     (i);}

T1(TYPE)  C TYPE*  Meml<TYPE>::addr      (Int       i   )C {return ConstCast(T).addr(i   );}
T1(TYPE)  C TYPE&  Meml<TYPE>::operator[](Int       i   )C {return ConstCast(T)     [i   ];}
T1(TYPE)  C TYPE&  Meml<TYPE>::operator[](MemlNode *node)C {return ConstCast(T)     [node];}

T1(TYPE)  MemlNode*  Meml<TYPE>::add      (              ) {return super::add      (    );}
T1(TYPE)  MemlNode*  Meml<TYPE>::addBefore(MemlNode *node) {return super::addBefore(node);}
T1(TYPE)  MemlNode*  Meml<TYPE>::addAfter (MemlNode *node) {return super::addAfter (node);}

T1(TYPE)  MemlNode*  Meml<TYPE>::first()C {return super::first();}
T1(TYPE)  MemlNode*  Meml<TYPE>::last ()C {return super::last ();}

T1(TYPE)  Int   Meml<TYPE>::index   (C TYPE *elm)C {return super::index   (elm);}
T1(TYPE)  Bool  Meml<TYPE>::contains(C TYPE *elm)C {return super::contains(elm);}

T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::removeFirst(                Bool keep_order) {super::removeFirst(      keep_order); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::removeLast (                               ) {super::removeLast (                ); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::remove     (MemlNode *node, Bool keep_order) {super::remove     (node, keep_order); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::removeData (C TYPE   *elm , Bool keep_order) {super::removeData (elm , keep_order); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::removeIndex(Int       i   , Bool keep_order) {super::removeIndex(i   , keep_order); return T;}

T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::setNum    (Int num) {super::setNum    (num); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::setNumZero(Int num) {super::setNumZero(num); return T;}

T1(TYPE) T1(VALUE)  Bool  Meml<TYPE>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C {return super::binarySearch(&value, index, (Int(*)(CPtr, CPtr))compare);}

T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::sort(           Int compare(C TYPE &a, C TYPE &b           )) {super::sort(      (Int(*)(CPtr, CPtr      ))compare); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::sort(CPtr user, Int compare(C TYPE &a, C TYPE &b, CPtr user)) {super::sort(user, (Int(*)(CPtr, CPtr, CPtr))compare); return T;}

T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::reverseOrder(            ) {super::reverseOrder(    ); return T;}
T1(TYPE)  Meml<TYPE>&  Meml<TYPE>::   swapOrder(Int i, Int j) {super::   swapOrder(i, j); return T;}

T1(TYPE)                     Meml<TYPE>&  Meml<TYPE>::operator=(C  Mems  <TYPE      >  &src) {                     setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Meml<TYPE>&  Meml<TYPE>::operator=(C  Memc  <TYPE      >  &src) {                     setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE) template<Int size>  Meml<TYPE>&  Meml<TYPE>::operator=(C  Memt  <TYPE, size>  &src) {                     setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Meml<TYPE>&  Meml<TYPE>::operator=(C  Memb  <TYPE      >  &src) {                     setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Meml<TYPE>&  Meml<TYPE>::operator=(C  Memx  <TYPE      >  &src) {                     setNum(src.elms()); FREPAO(T)=src[i];  return T;}
T1(TYPE)                     Meml<TYPE>&  Meml<TYPE>::operator=(C  Meml  <TYPE      >  &src) {if(this!=&src      ){setNum(src.elms()); for(MemlNode *d=first(), *s=src.first(); d && s; d=d->next(), s=s->next())T[d]=src[s];} return T;}
T1(TYPE) template<Int size>  Meml<TYPE>&  Meml<TYPE>::operator=(C CMemPtr<TYPE, size>  &src) {if(this!= src._meml){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
T1(TYPE)                     Meml<TYPE>&  Meml<TYPE>::operator=(   Meml  <TYPE      > &&src) {Swap(T, src); return T;}

T1(TYPE)  Bool  Meml<TYPE>::save(File &f)C {       f.cmpUIntV(elms()) ; MFREP(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Meml<TYPE>::save(File &f)  {       f.cmpUIntV(elms()) ; MFREP(T)if(!T[i].save(f))return false; return f.ok();}
T1(TYPE)  Bool  Meml<TYPE>::load(File &f)  {setNum(f.decUIntV(      )); MFREP(T)if(!T[i].load(f))goto   error;     if(f.ok())return true; error: clear(); return false;}

T1(TYPE)  Meml<TYPE>::Meml(            ) : _Meml(SIZE(TYPE)   , ClassFunc<TYPE>::GetNew(), ClassFunc<TYPE>::GetDel()) {}
T1(TYPE)  Meml<TYPE>::Meml(C Meml  &src) : _Meml(src.elmSize(),                src._new  ,                src._del  ) {T=src;}
T1(TYPE)  Meml<TYPE>::Meml(  Meml &&src) : _Meml(            0,                     null ,                     null ) {Swap(T, src);}
/******************************************************************************/
// CONST MEM PTR
/******************************************************************************/
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(          null_t                              ) {_mode=       PTR        ; _ptr =null                      ; _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C         TYPE             &src               ) {_mode=       PTR        ; _ptr =&src                      ; _ptr_elms=            1; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C         TYPE             *src, Int src_elms ) {_mode=       PTR        ; _ptr = src                      ; _ptr_elms=     src_elms; return T;}
template<typename TYPE, Int Memt_size> template<Int src_elms>  CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C         TYPE            (&src)    [src_elms]) {_mode=       PTR        ; _ptr = src                      ; _ptr_elms=     src_elms; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Mems  <TYPE           > &src               ) {_mode=       MEMS       ; _mems=&src                      ; _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Mems  <TYPE           > *src               ) {_mode=(src ? MEMS : PTR); _mems= src                      ; _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Memc  <TYPE           > &src               ) {_mode=       MEMC       ; _memc=&src                      ; _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Memc  <TYPE           > *src               ) {_mode=(src ? MEMC : PTR); _memc= src                      ; _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size> T1(EXTENDED)            CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Memc  <EXTENDED       > *src               ) {_mode=(src ? MEMC : PTR); _memc=&SCAST(C Memc<TYPE>, *src); _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Memt  <TYPE, Memt_size> &src               ) {_mode=       MEMT       ; _memt=&src                      ; _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Memt  <TYPE, Memt_size> *src               ) {_mode=(src ? MEMT : PTR); _memt= src                      ; _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Memb  <TYPE           > &src               ) {_mode=       MEMB       ; _memb=&src                      ; _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Memb  <TYPE           > *src               ) {_mode=(src ? MEMB : PTR); _memb= src                      ; _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size> T1(EXTENDED)            CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Memb  <EXTENDED       > *src               ) {_mode=(src ? MEMB : PTR); _memb=&SCAST(C Memb<TYPE>, *src); _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Memx  <TYPE           > &src               ) {_mode=       MEMX       ; _memx=&src                      ; _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Memx  <TYPE           > *src               ) {_mode=(src ? MEMX : PTR); _memx= src                      ; _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size> T1(EXTENDED)            CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Memx  <EXTENDED       > *src               ) {_mode=(src ? MEMX : PTR); _memx=&SCAST(C Memx<TYPE>, *src); _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Meml  <TYPE           > &src               ) {_mode=       MEML       ; _meml=&src                      ; _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C  Meml  <TYPE           > *src               ) {_mode=(src ? MEML : PTR); _meml= src                      ; _ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         CMemPtr<TYPE, Memt_size>&  CMemPtr<TYPE, Memt_size>::point(C CMemPtr<TYPE, Memt_size> &src               ) {_mode=  src._mode       ; _ptr = src._ptr                 ; _ptr_elms=src._ptr_elms; return T;}

template<typename TYPE, Int Memt_size>  Int  CMemPtr<TYPE, Memt_size>::elms()C
{
   switch(_mode)
   {
      default  : return _ptr_elms; // PTR
      case MEMS: return _mems->elms();
      case MEMC: return _memc->elms();
      case MEMT: return _memt->elms();
      case MEMB: return _memb->elms();
      case MEMX: return _memx->elms();
      case MEML: return _meml->elms();
   }
}
template<typename TYPE, Int Memt_size>  UInt  CMemPtr<TYPE, Memt_size>::elmSize()C
{
   switch(_mode)
   {
      default  : return  SIZE(TYPE); // PTR
      case MEMS: return _mems->elmSize();
      case MEMC: return _memc->elmSize();
      case MEMT: return _memt->elmSize();
      case MEMB: return _memb->elmSize();
      case MEMX: return _memx->elmSize();
      case MEML: return _meml->elmSize();
   }
}
template<typename TYPE, Int Memt_size>  UIntPtr  CMemPtr<TYPE, Memt_size>::memUsage()C
{
   switch(_mode)
   {
      default  : return  UIntPtr(elms())*elmSize(); // PTR
      case MEMS: return _mems->memUsage();
      case MEMC: return _memc->memUsage();
      case MEMT: return _memt->memUsage();
      case MEMB: return _memb->memUsage();
      case MEMX: return _memx->memUsage();
      case MEML: return _meml->memUsage();
   }
}
template<typename TYPE, Int Memt_size>  C TYPE*  CMemPtr<TYPE, Memt_size>::data()C
{
   switch(_mode)
   {
      default  : return _ptr; // PTR
      case MEMS: return _mems->data();
      case MEMC: return _memc->data();
      case MEMT: return _memt->data();
      case MEMB: Exit("'CMemPtr.data' does not support MEMB mode"); return null;
      case MEMX: Exit("'CMemPtr.data' does not support MEMX mode"); return null;
      case MEML: Exit("'CMemPtr.data' does not support MEML mode"); return null;
   }
}
template<typename TYPE, Int Memt_size>  C TYPE*  CMemPtr<TYPE, Memt_size>::addr(Int i)C
{
   switch(_mode)
   {
      default  : return  InRange(i, _ptr_elms) ? &_ptr[i] : null; // PTR
      case MEMS: return _mems->addr(i);
      case MEMC: return _memc->addr(i);
      case MEMT: return _memt->addr(i);
      case MEMB: return _memb->addr(i);
      case MEMX: return _memx->addr(i);
      case MEML: return _meml->addr(i);
   }
}
template<typename TYPE, Int Memt_size>  C TYPE&  CMemPtr<TYPE, Memt_size>::operator[](Int i)C
{
   switch(_mode)
   {
      default  : DEBUG_RANGE_ASSERT(i, _ptr_elms); return _ptr[i]; // PTR
      case MEMS: return (*_mems)[i];
      case MEMC: return (*_memc)[i];
      case MEMT: return (*_memt)[i];
      case MEMB: return (*_memb)[i];
      case MEMX: return (*_memx)[i];
      case MEML: return (*_meml)[i];
   }
}
template<typename TYPE, Int Memt_size>  C TYPE&  CMemPtr<TYPE, Memt_size>::first()C {return T[0       ];}
template<typename TYPE, Int Memt_size>  C TYPE&  CMemPtr<TYPE, Memt_size>::last ()C {return T[elms()-1];}

template<typename TYPE, Int Memt_size>  Int  CMemPtr<TYPE, Memt_size>::index(C TYPE *elm)C
{
   switch(_mode)
   {
      case PTR : {Int i=elm-_ptr; if(InRange(i, _ptr_elms))return i;} break;
      case MEMS: return _mems->     index(elm);
      case MEMC: return _memc->     index(elm);
      case MEMT: return _memt->     index(elm);
      case MEMB: return _memb->     index(elm);
      case MEMX: return _memx->validIndex(elm);
      case MEML: return _meml->     index(elm);
   }
   return -1;
}
template<typename TYPE, Int Memt_size>  Bool  CMemPtr<TYPE, Memt_size>::contains(C TYPE *elm)C {return index(elm)>=0;}

template<typename TYPE, Int Memt_size> T1(VALUE)  Bool  CMemPtr<TYPE, Memt_size>::binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b))C
{
   switch(_mode)
   {
      default  : return        BinarySearch(_ptr, _ptr_elms, value, index, compare); // PTR
      case MEMS: return _mems->binarySearch(                 value, index, compare);
      case MEMC: return _memc->binarySearch(                 value, index, compare);
      case MEMT: return _memt->binarySearch(                 value, index, compare);
      case MEMB: return _memb->binarySearch(                 value, index, compare);
      case MEMX: return _memx->binarySearch(                 value, index, compare);
      case MEML: return _meml->binarySearch(                 value, index, compare);
   }
}

#if EE_PRIVATE
template<typename TYPE, Int Memt_size>  void  CMemPtr<TYPE, Memt_size>::copyTo(TYPE *dest)C
{
   switch(_mode)
   {
      case PTR :  if(dest)CopyFastN(dest, _ptr, _ptr_elms); break;
      case MEMS: _mems->copyTo(dest); break;
      case MEMC: _memc->copyTo(dest); break;
      case MEMT: _memt->copyTo(dest); break;
      case MEMB: _memb->copyTo(dest); break;
      case MEMX: _memx->copyTo(dest); break;
      case MEML: _meml->copyTo(dest); break;
   }
}
#endif
template<typename TYPE, Int Memt_size>        CMemPtr<TYPE, Memt_size>::operator Bool()C {return _mode!=PTR || elms();}
template<typename TYPE, Int Memt_size>  Bool  CMemPtr<TYPE, Memt_size>::resizable    ()C {return _mode!=PTR;}
template<typename TYPE, Int Memt_size>  Bool  CMemPtr<TYPE, Memt_size>::continuous   ()C
{
   switch(_mode)
   {
      default  : return  true; // PTR
      case MEMS: return  Mems<TYPE>           ::Continuous;
      case MEMC: return _Memc                 ::Continuous;
      case MEMT: return  Memt<TYPE, Memt_size>::Continuous;
      case MEMB: return _Memb                 ::Continuous;
      case MEMX: return _Memx                 ::Continuous;
      case MEML: return _Meml                 ::Continuous;
   }
}

template<typename TYPE, Int Memt_size>  Bool  CMemPtr<TYPE, Memt_size>::save(File &f)C
{
   switch(_mode)
   {
      default  : f.cmpUIntV(elms()); FREPA(T)if(!T[i].save(f))return false; return f.ok(); // PTR
      case MEMS: return _mems->save(f);
      case MEMC: return _memc->save(f);
      case MEMT: return _memt->save(f);
      case MEMB: return _memb->save(f);
      case MEMX: return _memx->save(f);
      case MEML: return _meml->save(f);
   }
}
template<typename TYPE, Int Memt_size>  Bool  CMemPtr<TYPE, Memt_size>::saveRaw(File &f)C
{
   switch(_mode)
   {
      default  : f.cmpUIntV(elms()); f.putN(_ptr, elms()); return f.ok(); // PTR
      case MEMS: return _mems->saveRaw(f);
      case MEMC: return _memc->saveRaw(f);
      case MEMT: return _memt->saveRaw(f);
      case MEMB: return _memb->saveRaw(f);
      case MEMX: return _memx->saveRaw(f);
      case MEML: return _meml->saveRaw(f);
   }
}
/******************************************************************************/
// MEM PTR
/******************************************************************************/
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(       null_t                              ) {T._mode=       super::PTR               ; T._ptr =null                    ; T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(       TYPE             &src               ) {T._mode=       super::PTR               ; T._ptr =&src                    ; T._ptr_elms=            1; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(       TYPE             *src, Int src_elms ) {T._mode=       super::PTR               ; T._ptr = src                    ; T._ptr_elms=     src_elms; return T;}
template<typename TYPE, Int Memt_size> template<Int src_elms>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(       TYPE            (&src)    [src_elms]) {T._mode=       super::PTR               ; T._ptr = src                    ; T._ptr_elms=     src_elms; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Mems  <TYPE           > &src               ) {T._mode=       super::MEMS              ; T._mems=&src                    ; T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Mems  <TYPE           > *src               ) {T._mode=(src ? super::MEMS : super::PTR); T._mems= src                    ; T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memc  <TYPE           > &src               ) {T._mode=       super::MEMC              ; T._memc=&src                    ; T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memc  <TYPE           > *src               ) {T._mode=(src ? super::MEMC : super::PTR); T._memc= src                    ; T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size> T1(EXTENDED)            MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memc  <EXTENDED       > *src               ) {T._mode=(src ? super::MEMC : super::PTR); T._memc=&SCAST(Memc<TYPE>, *src); T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memt  <TYPE, Memt_size> &src               ) {T._mode=       super::MEMT              ; T._memt=&src                    ; T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memt  <TYPE, Memt_size> *src               ) {T._mode=(src ? super::MEMT : super::PTR); T._memt= src                    ; T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memb  <TYPE           > &src               ) {T._mode=       super::MEMB              ; T._memb=&src                    ; T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memb  <TYPE           > *src               ) {T._mode=(src ? super::MEMB : super::PTR); T._memb= src                    ; T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size> T1(EXTENDED)            MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memb  <EXTENDED       > *src               ) {T._mode=(src ? super::MEMB : super::PTR); T._memb=&SCAST(Memb<TYPE>, *src); T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memx  <TYPE           > &src               ) {T._mode=       super::MEMX              ; T._memx=&src                    ; T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memx  <TYPE           > *src               ) {T._mode=(src ? super::MEMX : super::PTR); T._memx= src                    ; T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size> T1(EXTENDED)            MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Memx  <EXTENDED       > *src               ) {T._mode=(src ? super::MEMX : super::PTR); T._memx=&SCAST(Memx<TYPE>, *src); T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Meml  <TYPE           > &src               ) {T._mode=       super::MEML              ; T._meml=&src                    ; T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(Meml  <TYPE           > *src               ) {T._mode=(src ? super::MEML : super::PTR); T._meml= src                    ; T._ptr_elms=            0; return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::point(MemPtr<TYPE, Memt_size> &src               ) {T._mode=       src._mode                ; T._ptr = src._ptr               ; T._ptr_elms=src._ptr_elms; return T;}

template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::clear()
{
   switch(T._mode)
   {
      case super::PTR : if(T._ptr_elms)Exit("'MemPtr.clear' does not support PTR mode"); break;
      case super::MEMS: ConstCast(T._mems)->clear(); break;
      case super::MEMC: ConstCast(T._memc)->clear(); break;
      case super::MEMT: ConstCast(T._memt)->clear(); break;
      case super::MEMB: ConstCast(T._memb)->clear(); break;
      case super::MEMX: ConstCast(T._memx)->clear(); break;
      case super::MEML: ConstCast(T._meml)->clear(); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::del()
{
   switch(T._mode)
   {
      case super::PTR : if(T._ptr_elms)Exit("'MemPtr.del' does not support PTR mode"); break;
      case super::MEMS: ConstCast(T._mems)->del(); break;
      case super::MEMC: ConstCast(T._memc)->del(); break;
      case super::MEMT: ConstCast(T._memt)->del(); break;
      case super::MEMB: ConstCast(T._memb)->del(); break;
      case super::MEMX: ConstCast(T._memx)->del(); break;
      case super::MEML: ConstCast(T._meml)->del(); break;
   }
   return T;
}

template<typename TYPE, Int Memt_size>    TYPE*  MemPtr<TYPE, Memt_size>::data()  {return ConstCast(super::data());}
template<typename TYPE, Int Memt_size>  C TYPE*  MemPtr<TYPE, Memt_size>::data()C {return           super::data() ;}

template<typename TYPE, Int Memt_size>    TYPE*  MemPtr<TYPE, Memt_size>::addr(Int i)  {return ConstCast(super::addr(i));}
template<typename TYPE, Int Memt_size>  C TYPE*  MemPtr<TYPE, Memt_size>::addr(Int i)C {return           super::addr(i) ;}

template<typename TYPE, Int Memt_size>    TYPE&  MemPtr<TYPE, Memt_size>::operator[](Int i)  {return ConstCast(super::operator[](i));}
template<typename TYPE, Int Memt_size>  C TYPE&  MemPtr<TYPE, Memt_size>::operator[](Int i)C {return           super::operator[](i) ;}

template<typename TYPE, Int Memt_size>  TYPE&  MemPtr<TYPE, Memt_size>::operator()(Int i)
{
   switch(T._mode)
   {
      default         : RANGE_ASSERT(i, T._ptr_elms); return ConstCast(T._ptr[i]); // PTR
      case super::MEMS: return ConstCast(*T._mems)(i);
      case super::MEMC: return ConstCast(*T._memc)(i);
      case super::MEMT: return ConstCast(*T._memt)(i);
      case super::MEMB: return ConstCast(*T._memb)(i);
      case super::MEMX: Exit("'MemPtr.operator(Int)' does not support MEMX mode"); return *(TYPE*)null;
      case super::MEML: return ConstCast(*T._meml)(i);
   }
}
template<typename TYPE, Int Memt_size>    TYPE&  MemPtr<TYPE, Memt_size>::first()  {return ConstCast(super::first());}
template<typename TYPE, Int Memt_size>  C TYPE&  MemPtr<TYPE, Memt_size>::first()C {return           super::first() ;}
template<typename TYPE, Int Memt_size>    TYPE&  MemPtr<TYPE, Memt_size>::last ()  {return ConstCast(super::last ());}
template<typename TYPE, Int Memt_size>  C TYPE&  MemPtr<TYPE, Memt_size>::last ()C {return           super::last () ;}

template<typename TYPE, Int Memt_size>  TYPE&  MemPtr<TYPE, Memt_size>::New()
{
   switch(T._mode)
   {
      default         : Exit("'MemPtr.New' does not support PTR mode"); return *(TYPE*)null; // PTR
      case super::MEMS: return ConstCast(T._mems)->New();
      case super::MEMC: return ConstCast(T._memc)->New();
      case super::MEMT: return ConstCast(T._memt)->New();
      case super::MEMB: return ConstCast(T._memb)->New();
      case super::MEMX: return ConstCast(T._memx)->New();
      case super::MEML: return ConstCast(T._meml)->New();
   }
}
template<typename TYPE, Int Memt_size>  TYPE&  MemPtr<TYPE, Memt_size>::NewAt(Int i)
{
   switch(T._mode)
   {
      default         : Exit("'MemPtr.NewAt' does not support PTR mode"); return *(TYPE*)null; // PTR
      case super::MEMS: return ConstCast(T._mems)->NewAt(i);
      case super::MEMC: return ConstCast(T._memc)->NewAt(i);
      case super::MEMT: return ConstCast(T._memt)->NewAt(i);
      case super::MEMB: return ConstCast(T._memb)->NewAt(i);
      case super::MEMX: return ConstCast(T._memx)->NewAt(i);
      case super::MEML: return ConstCast(T._meml)->NewAt(i);
   }
}

template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::removeLast()
{
   switch(T._mode)
   {
      case super::PTR : if(T._ptr_elms)Exit("'MemPtr.removeLast' does not support PTR mode"); break;
      case super::MEMS: ConstCast(T._mems)->removeLast(); break;
      case super::MEMC: ConstCast(T._memc)->removeLast(); break;
      case super::MEMT: ConstCast(T._memt)->removeLast(); break;
      case super::MEMB: ConstCast(T._memb)->removeLast(); break;
      case super::MEMX: ConstCast(T._memx)->removeLast(); break;
      case super::MEML: ConstCast(T._meml)->removeLast(); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::remove(Int i, Bool keep_order)
{
   switch(T._mode)
   {
      case super::PTR : if(InRange(i, T._ptr_elms))Exit("'MemPtr.remove' does not support PTR mode"); break;
      case super::MEMS: ConstCast(T._mems)->remove     (i, keep_order); break;
      case super::MEMC: ConstCast(T._memc)->remove     (i, keep_order); break;
      case super::MEMT: ConstCast(T._memt)->remove     (i, keep_order); break;
      case super::MEMB: ConstCast(T._memb)->remove     (i, keep_order); break;
      case super::MEMX: ConstCast(T._memx)->removeValid(i, keep_order); break;
      case super::MEML: ConstCast(T._meml)->removeIndex(i, keep_order); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::removeData(C TYPE *elm, Bool keep_order)
{
   switch(T._mode)
   {
      case super::PTR : if(InRange(index(elm), T._ptr_elms))Exit("'MemPtr.removeData' does not support PTR mode"); break;
      case super::MEMS: ConstCast(T._mems)->removeData(elm, keep_order); break;
      case super::MEMC: ConstCast(T._memc)->removeData(elm, keep_order); break;
      case super::MEMT: ConstCast(T._memt)->removeData(elm, keep_order); break;
      case super::MEMB: ConstCast(T._memb)->removeData(elm, keep_order); break;
      case super::MEMX: ConstCast(T._memx)->removeData(elm, keep_order); break;
      case super::MEML: ConstCast(T._meml)->removeData(elm, keep_order); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::setNum(Int num)
{
   switch(T._mode)
   {
      case super::PTR : if(T._ptr_elms!=num)Exit("'MemPtr.setNum' does not support PTR mode"); break;
      case super::MEMS: ConstCast(T._mems)->setNum(num); break;
      case super::MEMC: ConstCast(T._memc)->setNum(num); break;
      case super::MEMT: ConstCast(T._memt)->setNum(num); break;
      case super::MEMB: ConstCast(T._memb)->setNum(num); break;
      case super::MEMX: ConstCast(T._memx)->setNum(num); break;
      case super::MEML: ConstCast(T._meml)->setNum(num); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::setNumZero(Int num)
{
   switch(T._mode)
   {
      case super::PTR : if(T._ptr_elms!=num)Exit("'MemPtr.setNumZero' does not support PTR mode"); break;
      case super::MEMS: ConstCast(T._mems)->setNumZero(num); break;
      case super::MEMC: ConstCast(T._memc)->setNumZero(num); break;
      case super::MEMT: ConstCast(T._memt)->setNumZero(num); break;
      case super::MEMB: ConstCast(T._memb)->setNumZero(num); break;
      case super::MEMX: Exit("'MemPtr.setNumZero' does not support MEMX mode"); break;
      case super::MEML: ConstCast(T._meml)->setNumZero(num); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::setNumDiscard(Int num)
{
   switch(T._mode)
   {
      case super::PTR : if(T._ptr_elms!=num)Exit("'MemPtr.setNumDiscard' does not support PTR mode"); break;
      case super::MEMS: ConstCast(T._mems)->setNumDiscard(num); break;
      case super::MEMC: ConstCast(T._memc)->setNumDiscard(num); break;
      case super::MEMT: ConstCast(T._memt)->setNumDiscard(num); break;
      case super::MEMB: ConstCast(T._memb)->setNum       (num); break;
      case super::MEMX: ConstCast(T._memx)->setNum       (num); break;
      case super::MEML: ConstCast(T._meml)->setNum       (num); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  Int  MemPtr<TYPE, Memt_size>::addNum(Int num)
{
   switch(T._mode)
   {
      default         : if(num)Exit("'MemPtr.addNum' does not support PTR mode"); return T._ptr_elms; // PTR
      case super::MEMS: return ConstCast(T._mems)->addNum(num);
      case super::MEMC: return ConstCast(T._memc)->addNum(num);
      case super::MEMT: return ConstCast(T._memt)->addNum(num);
      case super::MEMB: return ConstCast(T._memb)->addNum(num);
      case super::MEMX: return ConstCast(T._memx)->addNum(num);
      case super::MEML: return ConstCast(T._meml)->addNum(num);
   }
}

template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::sort(Int compare(C TYPE &a, C TYPE &b))
{
   switch(T._mode)
   {
      case super::PTR :                     Sort(ConstCast(T._ptr), T._ptr_elms, compare); break;
      case super::MEMS: ConstCast(T._mems)->sort(                                compare); break;
      case super::MEMC: ConstCast(T._memc)->sort(                                compare); break;
      case super::MEMT: ConstCast(T._memt)->sort(                                compare); break;
      case super::MEMB: ConstCast(T._memb)->sort(                                compare); break;
      case super::MEMX: ConstCast(T._memx)->sort(                                compare); break;
      case super::MEML: ConstCast(T._meml)->sort(                                compare); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::sort(CPtr user, Int compare(C TYPE &a, C TYPE &b, CPtr user))
{
   switch(T._mode)
   {
      case super::PTR :                     Sort(ConstCast(T._ptr), T._ptr_elms, user, compare); break;
      case super::MEMS: ConstCast(T._mems)->sort(                                user, compare); break;
      case super::MEMC: ConstCast(T._memc)->sort(                                user, compare); break;
      case super::MEMT: ConstCast(T._memt)->sort(                                user, compare); break;
      case super::MEMB: ConstCast(T._memb)->sort(                                user, compare); break;
      case super::MEMX: ConstCast(T._memx)->sort(                                user, compare); break;
      case super::MEML: ConstCast(T._meml)->sort(                                user, compare); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::reverseOrder()
{
   switch(T._mode)
   {
      case super::PTR :                     ReverseOrder(ConstCast(T._ptr), T._ptr_elms); break;
      case super::MEMS: ConstCast(T._mems)->reverseOrder(                              ); break;
      case super::MEMC: ConstCast(T._memc)->reverseOrder(                              ); break;
      case super::MEMT: ConstCast(T._memt)->reverseOrder(                              ); break;
      case super::MEMB: ConstCast(T._memb)->reverseOrder(                              ); break;
      case super::MEMX: ConstCast(T._memx)->reverseOrder(                              ); break;
      case super::MEML: ConstCast(T._meml)->reverseOrder(                              ); break;
   }
   return T;
}
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::swapOrder(Int i, Int j)
{
   switch(T._mode)
   {
      case super::PTR : if(InRange(i, T._ptr_elms) && InRange(j, T._ptr_elms))Swap(&T[i], &T[j], T.elmSize()); break;
      case super::MEMS: ConstCast(T._mems)->swapOrder(i, j); break;
      case super::MEMC: ConstCast(T._memc)->swapOrder(i, j); break;
      case super::MEMT: ConstCast(T._memt)->swapOrder(i, j); break;
      case super::MEMB: ConstCast(T._memb)->swapOrder(i, j); break;
      case super::MEMX: ConstCast(T._memx)->swapOrder(i, j); break;
      case super::MEML: ConstCast(T._meml)->swapOrder(i, j); break;
   }
   return T;
}

template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C  TYPE                    &src           ) {                       setNum(         1);      T[0]=src   ;  return T;}
template<typename TYPE, Int Memt_size> template<Int src_elms>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C  TYPE                   (&src)[src_elms]) {                       setNum(src_elms  ); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C  Mems  <TYPE           > &src           ) {if(T._mems!=    &src ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C  Memc  <TYPE           > &src           ) {if(T._memc!=    &src ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int Memt_size> template<Int src_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C  Memt  <TYPE,  src_size> &src           ) {if(T._memt!=Ptr(&src)){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C  Memb  <TYPE           > &src           ) {if(T._memb!=    &src ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C  Memx  <TYPE           > &src           ) {if(T._memx!=    &src ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C  Meml  <TYPE           > &src           ) {if(T._meml!=    &src ){setNum(src.elms()); FREPAO(T)=src[i];} return T;}
template<typename TYPE, Int Memt_size> template<Int src_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C CMemPtr<TYPE,  src_size> &src           ) {                       setNum(src.elms()); FREPAO(T)=src[i];  return T;}
template<typename TYPE, Int Memt_size>                         MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::operator=(C  MemPtr<TYPE, Memt_size> &src           ) {                       setNum(src.elms()); FREPAO(T)=src[i];  return T;}

template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C  TYPE                     &src           ) {super::operator=(src); return T;}
template<typename TYPE, Int Memt_elms> template<Int src_elms>  MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C  TYPE                    (&src)[src_elms]) {super::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C  Mems   <TYPE           > &src           ) {super::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C  Memc   <TYPE           > &src           ) {super::operator=(src); return T;}
template<typename TYPE, Int Memt_elms> template<Int src_size>  MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C  Memt   <TYPE,  src_size> &src           ) {super::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C  Memb   <TYPE           > &src           ) {super::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C  Memx   <TYPE           > &src           ) {super::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C  Meml   <TYPE           > &src           ) {super::operator=(src); return T;}
template<typename TYPE, Int Memt_elms> template<Int src_size>  MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C CMemPtr <TYPE,  src_size> &src           ) {super::operator=(src); return T;}
template<typename TYPE, Int Memt_elms>                         MemPtrN<TYPE, Memt_elms>&  MemPtrN<TYPE, Memt_elms>::operator=(C  MemPtrN<TYPE, Memt_elms> &src           ) {super::operator=(src); return T;}

#if EE_PRIVATE
template<typename TYPE, Int Memt_size>  MemPtr<TYPE, Memt_size>&  MemPtr<TYPE, Memt_size>::copyFrom(C TYPE *src)
{
   switch(T._mode)
   {
      case super::PTR : CopyN(ConstCast(T._ptr), src, T._ptr_elms); break; // use 'CopyN' in case 'src' is null
      case super::MEMS: ConstCast(T._mems)->copyFrom(src); break;
      case super::MEMC: ConstCast(T._memc)->copyFrom(src); break;
      case super::MEMT: ConstCast(T._memt)->copyFrom(src); break;
      case super::MEMB: ConstCast(T._memb)->copyFrom(src); break;
      case super::MEMX: ConstCast(T._memx)->copyFrom(src); break;
      case super::MEML: ConstCast(T._meml)->copyFrom(src); break;
   }
   return T;
}
#endif
template<typename TYPE, Int Memt_size>  Bool  MemPtr<TYPE, Memt_size>::save(File &f)C {return super::save(f);}
template<typename TYPE, Int Memt_size>  Bool  MemPtr<TYPE, Memt_size>::save(File &f)
{
   switch(T._mode)
   {
      default         : f.cmpUIntV(T.elms()); FREPA(T)if(!T[i].save(f))return false; return f.ok(); // PTR
      case super::MEMS: return ConstCast(T._mems)->save(f);
      case super::MEMC: return ConstCast(T._memc)->save(f);
      case super::MEMT: return ConstCast(T._memt)->save(f);
      case super::MEMB: return ConstCast(T._memb)->save(f);
      case super::MEMX: return ConstCast(T._memx)->save(f);
      case super::MEML: return ConstCast(T._meml)->save(f);
   }
}
template<typename TYPE, Int Memt_size>  Bool  MemPtr<TYPE, Memt_size>::load(File &f)
{
   switch(T._mode)
   {
      default         : if(f.decUIntV()!=T.elms())return false; FREPA(T)if(!T[i].load(f))return false; return f.ok(); // PTR
      case super::MEMS: return ConstCast(T._mems)->load(f);
      case super::MEMC: return ConstCast(T._memc)->load(f);
      case super::MEMT: return ConstCast(T._memt)->load(f);
      case super::MEMB: return ConstCast(T._memb)->load(f);
      case super::MEMX: return ConstCast(T._memx)->load(f);
      case super::MEML: return ConstCast(T._meml)->load(f);
   }
}

template<typename TYPE, Int Memt_size>  Bool  MemPtr<TYPE, Memt_size>::loadRaw(File &f)
{
   switch(T._mode)
   {
      default         : if(f.decUIntV()!=T.elms())return false; f.getN(ConstCast(T._ptr), T.elms()); return f.ok(); // PTR
      case super::MEMS: return ConstCast(T._mems)->loadRaw(f);
      case super::MEMC: return ConstCast(T._memc)->loadRaw(f);
      case super::MEMT: return ConstCast(T._memt)->loadRaw(f);
      case super::MEMB: return ConstCast(T._memb)->loadRaw(f);
      case super::MEMX: return ConstCast(T._memx)->loadRaw(f);
      case super::MEML: return ConstCast(T._meml)->loadRaw(f);
   }
}
/******************************************************************************/
// COUNTED POINTER
/******************************************************************************/
T1(TYPE)  void  CountedPtr<TYPE>::DecRef(TYPE *data) {ASSERT_BASE_EXTENDED<PtrCounter, TYPE>(); SCAST(PtrCounter, *data).decRef(ClassFunc<TYPE>::Unload   );}
T1(TYPE)  void  CountedPtr<TYPE>::IncRef(TYPE *data) {ASSERT_BASE_EXTENDED<PtrCounter, TYPE>(); SCAST(PtrCounter, *data).incRef(ClassFunc<TYPE>::LoadEmpty);}

T1(TYPE)  CountedPtr<TYPE>&  CountedPtr<TYPE>::clear    (                   ) {                                DecRef(T._data); T._data=       null ;  return T;}
T1(TYPE)  CountedPtr<TYPE>&  CountedPtr<TYPE>::operator=(  TYPE       * data) {if(T!=data){IncRef(      data); DecRef(T._data); T._data=       data ;} return T;} // have to inc ref first, in case new pointer is a child of current that would get released if dec ref was first
T1(TYPE)  CountedPtr<TYPE>&  CountedPtr<TYPE>::operator=(C CountedPtr & eptr) {if(T!=eptr){IncRef(eptr._data); DecRef(T._data); T._data= eptr._data ;} return T;} // have to inc ref first, in case new pointer is a child of current that would get released if dec ref was first
T1(TYPE)  CountedPtr<TYPE>&  CountedPtr<TYPE>::operator=(  CountedPtr &&eptr) {                                              Swap(_data, eptr._data);  return T;}
T1(TYPE)  CountedPtr<TYPE>&  CountedPtr<TYPE>::operator=(  null_t           ) {clear();                                                                return T;}

T1(TYPE)  CountedPtr<TYPE>:: CountedPtr(  null_t           ) {       T._data=      null ;}
T1(TYPE)  CountedPtr<TYPE>:: CountedPtr(  TYPE       * data) {IncRef(T._data=      data);}
T1(TYPE)  CountedPtr<TYPE>:: CountedPtr(C CountedPtr & eptr) {IncRef(T._data=eptr._data);}
T1(TYPE)  CountedPtr<TYPE>:: CountedPtr(  CountedPtr &&eptr) {       T._data=eptr._data ; eptr._data=null;}
T1(TYPE)  CountedPtr<TYPE>::~CountedPtr(                   ) {clear();}
/******************************************************************************/
// CACHE
/******************************************************************************/
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::clear         (               ) {                   super::clear         (         ); return T;}
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::del           (               ) {                   super::del           (         ); return T;}
T1(TYPE)  CACHE_MODE    Cache<TYPE>::mode          (CACHE_MODE mode) {return (CACHE_MODE)super::mode          (mode     );          }
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::caseSensitive (Bool  sensitive) {                   super::caseSensitive (sensitive); return T;}
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::delayRemove   (Flt   time     ) {                   super::delayRemove   (time     ); return T;}
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::delayRemoveNow(               ) {                   super::delayRemoveNow(         ); return T;}
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::delayRemoveInc(               ) {                   super::delayRemoveInc(         ); return T;}
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::delayRemoveDec(               ) {                   super::delayRemoveDec(         ); return T;}
T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::reserve       (Int   num      ) {                   super::reserve       (num      ); return T;}

T1(TYPE)  TYPE*  Cache<TYPE>::find      (C Str &file, CChar *path) {return (TYPE*)super::find   (file, path, false);}
T1(TYPE)  TYPE*  Cache<TYPE>::find      (C UID &id  , CChar *path) {return (TYPE*)super::find   (id  , path, false);}
T1(TYPE)  TYPE*  Cache<TYPE>::get       (C Str &file, CChar *path) {return (TYPE*)super::get    (file, path, false);}
T1(TYPE)  TYPE*  Cache<TYPE>::get       (C UID &id  , CChar *path) {return (TYPE*)super::get    (id  , path, false);}
T1(TYPE)  TYPE*  Cache<TYPE>::operator()(C Str &file, CChar *path) {return (TYPE*)super::require(file, path, false);}
T1(TYPE)  TYPE*  Cache<TYPE>::operator()(C UID &id  , CChar *path) {return (TYPE*)super::require(id  , path, false);}

T1(TYPE)  C Str&  Cache<TYPE>::name    (C TYPE *data             )C {return super::name    (data       );}
T1(TYPE)  CChar*  Cache<TYPE>::name    (C TYPE *data, CChar *path)C {return super::name    (data,  path);}
T1(TYPE)  UID     Cache<TYPE>::id      (C TYPE *data             )C {return super::id      (data       );}
T1(TYPE)  Int     Cache<TYPE>::ptrCount(C TYPE *data             )C {return super::ptrCount(data       );}
#if EE_PRIVATE
T1(TYPE)  Bool    Cache<TYPE>::has     (C TYPE *data             )C {return super::has     (data       );}
T1(TYPE)  Bool    Cache<TYPE>::has     (C Str  &file, CChar *path)C {return super::has     (file, path );}
#endif
T1(TYPE)  Bool    Cache<TYPE>::contains(C TYPE *data             )C {return super::contains(data       );}
T1(TYPE)  Bool    Cache<TYPE>::dummy   (C TYPE *data             )C {return super::dummy   (data       );}
T1(TYPE)  void    Cache<TYPE>::dummy   (C TYPE *data, Bool  dummy)  {       super::dummy   (data, dummy);}

T1(TYPE)    Int           Cache<TYPE>::  elms      (     )C {return         super::  elms      ( );}
T1(TYPE)    void          Cache<TYPE>::  lock      (     )C {               super::  lock      ( );}
T1(TYPE) C _Cache::Desc&  Cache<TYPE>::  lockedDesc(Int i)C {return         super::  lockedDesc(i);}
T1(TYPE)    TYPE       &  Cache<TYPE>::  lockedData(Int i)  {return *(TYPE*)super::  lockedData(i);}
T1(TYPE) C  TYPE       &  Cache<TYPE>::  lockedData(Int i)C {return *(TYPE*)super::  lockedData(i);}
T1(TYPE)    void          Cache<TYPE>::unlock      (     )C {               super::unlock      ( );}

T1(TYPE)  void  Cache<TYPE>::removeData(C TYPE *data) {return super::removeData(data);}

T1(TYPE)  void  Cache<TYPE>::update() {return super::update();}

T1(TYPE)  void  Cache<TYPE>::setLoadUser(Ptr user) {super::setLoadUser(ClassFunc<TYPE>::LoadUser, user);}

T1(TYPE) T1(EXTENDED)  Cache<TYPE>&  Cache<TYPE>::replaceClass() {ASSERT_BASE_EXTENDED<TYPE, EXTENDED>(); lock(); del(); /*_data_offset=OFFSET(typename Cache<EXTENDED>::Elm, data);*/ _desc_offset=OFFSET(typename Cache<EXTENDED>::Elm, desc); _memx.replaceClass<typename Cache<EXTENDED>::Elm>(); _load=ClassFunc<EXTENDED>::Load; unlock(); return T;}

T1(TYPE)  Cache<TYPE>&  Cache<TYPE>::operator=(C Cache<TYPE> &src) {if(this!=&src){lock(); src.lock(); lockedFrom(src); FREPA(T)lockedData(i)=src.lockedData(i); src.unlock(); unlock();} return T;}

T1(TYPE)  Cache<TYPE>::Cache(CChar8 *name, Int block_elms) : _Cache(name, block_elms) {replaceClass<TYPE>();}

         inline Int Elms(C _Cache       &cache) {return cache.elms();}
T1(TYPE) inline Int Elms(C  Cache<TYPE> &cache) {return cache.elms();}
/******************************************************************************/
// CACHE ELEMENT POINTER
/******************************************************************************/
template<typename TYPE, Cache<TYPE> &CACHE>  C Str&  CacheElmPtr<TYPE,CACHE>::name (            )C {return CACHE.name (_data       );}
template<typename TYPE, Cache<TYPE> &CACHE>  CChar*  CacheElmPtr<TYPE,CACHE>::name (CChar *path )C {return CACHE.name (_data, path );}
template<typename TYPE, Cache<TYPE> &CACHE>  UID     CacheElmPtr<TYPE,CACHE>::id   (            )C {return CACHE.id   (_data       );}
template<typename TYPE, Cache<TYPE> &CACHE>  Bool    CacheElmPtr<TYPE,CACHE>::dummy(            )C {return CACHE.dummy(_data       );}
template<typename TYPE, Cache<TYPE> &CACHE>  void    CacheElmPtr<TYPE,CACHE>::dummy(Bool   dummy)  {       CACHE.dummy(_data, dummy);}

template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::clear    (                    ) {            CACHE.   decRef(            T._data); T._data=       null ;  return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(  TYPE        * data) {if(T!=data){CACHE.incDecRef(      data, T._data); T._data=       data ;} return T;} // have to inc ref first, in case new pointer is a child of current that would get released if dec ref was first
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(C CacheElmPtr & eptr) {if(T!=eptr){CACHE.incDecRef(eptr._data, T._data); T._data= eptr._data ;} return T;} // have to inc ref first, in case new pointer is a child of current that would get released if dec ref was first
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(  CacheElmPtr &&eptr) {                                               Swap(_data, eptr._data);  return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(  null_t            ) {clear();                                                                 return T;}
#if EE_PRIVATE
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::setContained(TYPE       * data) {if(T!=data){CACHE.incRefContained(data); CACHE.decRef(T._data); T._data=data;} return T;} // have to inc ref first, in case new pointer is a child of current that would get released if dec ref was first
#endif

template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::find     (CChar  *file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::find   (    file , path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::find     (CChar8 *file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::find   (Str(file), path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::find     (C Str  &file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::find   (    file , path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::find     (C Str8 &file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::find   (Str(file), path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::find     (C UID  &id  , CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::find   (    id   , path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::get      (CChar  *file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::get    (    file , path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::get      (CChar8 *file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::get    (Str(file), path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::get      (C Str  &file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::get    (    file , path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::get      (C Str8 &file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::get    (Str(file), path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::get      (C UID  &id  , CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::get    (    id   , path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::require  (CChar  *file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::require(    file , path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::require  (CChar8 *file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::require(Str(file), path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::require  (C Str  &file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::require(    file , path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::require  (C Str8 &file, CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::require(Str(file), path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::require  (C UID  &id  , CChar *path) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::require(    id   , path, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(CChar  *file             ) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::require(    file , null, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(CChar8 *file             ) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::require(Str(file), null, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(C Str  &file             ) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::require(    file , null, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(C Str8 &file             ) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::require(Str(file), null, true); CACHE.decRef(old); return T;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>&  CacheElmPtr<TYPE,CACHE>::operator=(C UID  &id               ) {TYPE *old=T._data; T._data=(TYPE*)CACHE._Cache::require(    id   , null, true); CACHE.decRef(old); return T;}

template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(  null_t            ) {             T._data=      null ;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(  TYPE        * data) {CACHE.incRef(T._data=      data);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(C CacheElmPtr & eptr) {CACHE.incRef(T._data=eptr._data);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(  CacheElmPtr &&eptr) {             T._data=eptr._data ; eptr._data=null;}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(  CChar       * file) {             T._data=(TYPE*)CACHE._Cache::require(    file , null, true);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(  CChar8      * file) {             T._data=(TYPE*)CACHE._Cache::require(Str(file), null, true);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(C Str         & file) {             T._data=(TYPE*)CACHE._Cache::require(    file , null, true);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(C Str8        & file) {             T._data=(TYPE*)CACHE._Cache::require(Str(file), null, true);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>:: CacheElmPtr(C UID         & id  ) {             T._data=(TYPE*)CACHE._Cache::require(    id   , null, true);}
template<typename TYPE, Cache<TYPE> &CACHE>  CacheElmPtr<TYPE,CACHE>::~CacheElmPtr(                    ) {clear();}
/******************************************************************************/
inline void CachesDelayRemove(Flt time) // set amount of time (in seconds) after which unused elements are removed from all Engine Caches (<=0 value specifies immediate unloading), default=0
{
   Objects     .delayRemove(time);
   Meshes      .delayRemove(time);
   PhysBodies  .delayRemove(time);
   WaterMtrls  .delayRemove(time);
   Materials   .delayRemove(time);
   Fonts       .delayRemove(time);
   ImageAtlases.delayRemove(time);
   Images      .delayRemove(time);
   PanelImages .delayRemove(time);
   Panels      .delayRemove(time);
   TextStyles  .delayRemove(time);
   GuiSkins    .delayRemove(time);
   Environments.delayRemove(time);
}
/******************************************************************************/
// MAP
/******************************************************************************/
T2(KEY, DATA)  Map<KEY, DATA>&  Map<KEY, DATA>::del  () {super::del  (); return T;}
T2(KEY, DATA)  Map<KEY, DATA>&  Map<KEY, DATA>::clear() {super::clear(); return T;}

T2(KEY, DATA)  ThreadSafeMap<KEY, DATA>&  ThreadSafeMap<KEY, DATA>::del  () {super::del  (); return T;}
T2(KEY, DATA)  ThreadSafeMap<KEY, DATA>&  ThreadSafeMap<KEY, DATA>::clear() {super::clear(); return T;}

T2(KEY, DATA)  Int  Map<KEY, DATA>::elms    ()C {return super::elms    ();}
T2(KEY, DATA)  Int  Map<KEY, DATA>::dataSize()C {return super::dataSize();}

T2(KEY, DATA)  Int  MapEx<KEY, DATA>::elms()C {return super::elms();}

T2(KEY, DATA)  Int  ThreadSafeMap<KEY, DATA>::elms    ()C {return super::elms    ();}
T2(KEY, DATA)  Int  ThreadSafeMap<KEY, DATA>::dataSize()C {return super::dataSize();}

T2(KEY, DATA)  DATA*  Map<KEY, DATA>::find      (C KEY &key) {return (DATA*)super::find   (&key);}
T2(KEY, DATA)  DATA*  Map<KEY, DATA>::get       (C KEY &key) {return (DATA*)super::get    (&key);}
T2(KEY, DATA)  DATA*  Map<KEY, DATA>::operator()(C KEY &key) {return (DATA*)super::require(&key);}

T2(KEY, DATA)  DATA*  Map<KEY, DATA>::get       (C KEY &key, Bool &just_created) {return (DATA*)super::get    (&key, just_created);}
T2(KEY, DATA)  DATA*  Map<KEY, DATA>::operator()(C KEY &key, Bool &just_created) {return (DATA*)super::require(&key, just_created);}

T2(KEY, DATA)  DATA*  ThreadSafeMap<KEY, DATA>::find      (C KEY &key) {return (DATA*)super::find   (&key);}
T2(KEY, DATA)  DATA*  ThreadSafeMap<KEY, DATA>::get       (C KEY &key) {return (DATA*)super::get    (&key);}
T2(KEY, DATA)  DATA*  ThreadSafeMap<KEY, DATA>::operator()(C KEY &key) {return (DATA*)super::require(&key);}

T2(KEY, DATA)  Int  Map<KEY, DATA>::   findValidIndex(C KEY &key)C {return super::   findValidIndex(&key);}
//T2(KEY, DATA)  Int  Map<KEY, DATA>::    getValidIndex(C KEY &key)  {return super::    getValidIndex(&key);}
//T2(KEY, DATA)  Int  Map<KEY, DATA>::requireValidIndex(C KEY &key)  {return super::requireValidIndex(&key);}

T2(KEY, DATA)  Int  Map<KEY, DATA>::   findAbsIndex(C KEY &key)C {return super::   findAbsIndex(&key);}
T2(KEY, DATA)  Int  Map<KEY, DATA>::    getAbsIndex(C KEY &key)  {return super::    getAbsIndex(&key);}
T2(KEY, DATA)  Int  Map<KEY, DATA>::requireAbsIndex(C KEY &key)  {return super::requireAbsIndex(&key);}

T2(KEY, DATA)  Int  ThreadSafeMap<KEY, DATA>::   findValidIndex(C KEY &key)C {return super::   findValidIndex(&key);}
//T2(KEY, DATA)  Int  ThreadSafeMap<KEY, DATA>::    getValidIndex(C KEY &key)  {return super::    getValidIndex(&key);}
//T2(KEY, DATA)  Int  ThreadSafeMap<KEY, DATA>::requireValidIndex(C KEY &key)  {return super::requireValidIndex(&key);}

T2(KEY, DATA)  Int  ThreadSafeMap<KEY, DATA>::   findAbsIndex(C KEY &key)C {return super::   findAbsIndex(&key);}
T2(KEY, DATA)  Int  ThreadSafeMap<KEY, DATA>::    getAbsIndex(C KEY &key)  {return super::    getAbsIndex(&key);}
T2(KEY, DATA)  Int  ThreadSafeMap<KEY, DATA>::requireAbsIndex(C KEY &key)  {return super::requireAbsIndex(&key);}

T2(KEY, DATA)    Bool  Map<KEY, DATA>::containsKey   (C KEY  &key )C {return        super::containsKey      (&key );}
T2(KEY, DATA)    Bool  Map<KEY, DATA>::containsData  (C DATA *data)C {return        super::containsData     ( data);}
T2(KEY, DATA)  C KEY*  Map<KEY, DATA>::dataToKey     (C DATA *data)C {return  (KEY*)super::dataToKey        ( data);}
T2(KEY, DATA)  C KEY*  Map<KEY, DATA>::dataInMapToKey(C DATA *data)C {return  (KEY*)super::dataInMapToKeyPtr( data);}
T2(KEY, DATA)  C KEY&  Map<KEY, DATA>::dataInMapToKey(C DATA &data)C {return *(KEY*)super::dataInMapToKeyRef(&data);}
T2(KEY, DATA)    Int   Map<KEY, DATA>::dataToIndex   (C DATA *data)C {return        super::dataToIndex      ( data);}

T2(KEY, DATA)    Bool  ThreadSafeMap<KEY, DATA>::containsKey   (C KEY  &key )C {return        super::containsKey      (&key );}
T2(KEY, DATA)    Bool  ThreadSafeMap<KEY, DATA>::containsData  (C DATA *data)C {return        super::containsData     ( data);}
T2(KEY, DATA)  C KEY*  ThreadSafeMap<KEY, DATA>::dataToKey     (C DATA *data)C {return  (KEY*)super::dataToKey        ( data);}
T2(KEY, DATA)  C KEY*  ThreadSafeMap<KEY, DATA>::dataInMapToKey(C DATA *data)C {return  (KEY*)super::dataInMapToKeyPtr( data);}
T2(KEY, DATA)  C KEY&  ThreadSafeMap<KEY, DATA>::dataInMapToKey(C DATA &data)C {return *(KEY*)super::dataInMapToKeyRef(&data);}
T2(KEY, DATA)    Int   ThreadSafeMap<KEY, DATA>::dataToIndex   (C DATA *data)C {return        super::dataToIndex      ( data);}

T2(KEY, DATA)  C KEY &  Map<KEY, DATA>::key       (Int i)C {return *(KEY *)super::key       (i);}
T2(KEY, DATA)    DATA&  Map<KEY, DATA>::operator[](Int i)  {return *(DATA*)super::operator[](i);}
T2(KEY, DATA)  C DATA&  Map<KEY, DATA>::operator[](Int i)C {return *(DATA*)super::operator[](i);}

T2(KEY, DATA)  C KEY &  Map<KEY, DATA>::absKey (Int abs_i)C {return *(KEY *)super::absKey (abs_i);}
T2(KEY, DATA)    DATA&  Map<KEY, DATA>::absData(Int abs_i)  {return *(DATA*)super::absData(abs_i);}
T2(KEY, DATA)  C DATA&  Map<KEY, DATA>::absData(Int abs_i)C {return *(DATA*)super::absData(abs_i);}

T2(KEY, DATA)  C KEY &  ThreadSafeMap<KEY, DATA>::lockedKey (Int i)C {return *(KEY *)super::key       (i);}
T2(KEY, DATA)    DATA&  ThreadSafeMap<KEY, DATA>::lockedData(Int i)  {return *(DATA*)super::operator[](i);}
T2(KEY, DATA)  C DATA&  ThreadSafeMap<KEY, DATA>::lockedData(Int i)C {return *(DATA*)super::operator[](i);}

T2(KEY, DATA)  C KEY &  ThreadSafeMap<KEY, DATA>::lockedAbsKey (Int abs_i)C {return *(KEY *)super::absKey (abs_i);}
T2(KEY, DATA)    DATA&  ThreadSafeMap<KEY, DATA>::lockedAbsData(Int abs_i)  {return *(DATA*)super::absData(abs_i);}
T2(KEY, DATA)  C DATA&  ThreadSafeMap<KEY, DATA>::lockedAbsData(Int abs_i)C {return *(DATA*)super::absData(abs_i);}

T2(KEY, DATA)  void  ThreadSafeMap<KEY, DATA>::  lock()C {super::  lock();}
T2(KEY, DATA)  void  ThreadSafeMap<KEY, DATA>::unlock()C {super::unlock();}

T2(KEY, DATA)  MAP_MODE            Map  <KEY, DATA>::mode(MAP_MODE mode) {return (MAP_MODE)super::mode(mode);}
T2(KEY, DATA)  MAP_MODE            MapEx<KEY, DATA>::mode(MAP_MODE mode) {return (MAP_MODE)super::mode(mode);}
T2(KEY, DATA)  MAP_MODE  ThreadSafeMap  <KEY, DATA>::mode(MAP_MODE mode) {return (MAP_MODE)super::mode(mode);}

T2(KEY, DATA)  MapEx<KEY,DATA>&  MapEx<KEY,DATA>::delayRemove   (Flt time) {super::delayRemove   (time); return T;}
T2(KEY, DATA)  MapEx<KEY,DATA>&  MapEx<KEY,DATA>::delayRemoveNow(        ) {super::delayRemoveNow(    ); return T;}

T2(KEY, DATA)  void  MapEx<KEY,DATA>::update() {super::update();}

T2(KEY, DATA)  void  Map<KEY, DATA>::remove    (  Int   i               ) {       super::remove    ( i   );}
T2(KEY, DATA)  void  Map<KEY, DATA>::removeKey (C KEY  &key             ) {       super::removeKey (&key );}
T2(KEY, DATA)  void  Map<KEY, DATA>::removeData(C DATA *data            ) {       super::removeData( data);}
T2(KEY, DATA)  Bool  Map<KEY, DATA>::replaceKey(C KEY  &src, C KEY &dest) {return super::replaceKey(&src, &dest);}

T2(KEY, DATA)  void            Map  <KEY, DATA>::reserve(Int num) {super::reserve(num);}
T2(KEY, DATA)  void            MapEx<KEY, DATA>::reserve(Int num) {super::reserve(num);}
T2(KEY, DATA)  void  ThreadSafeMap  <KEY, DATA>::reserve(Int num) {super::reserve(num);}

T2(KEY, DATA)  void            Map<KEY, DATA>::compare(Int compare(C KEY &a, C KEY &b)) {super::compare((Int(*)(CPtr, CPtr))compare);}
T2(KEY, DATA)  void  ThreadSafeMap<KEY, DATA>::compare(Int compare(C KEY &a, C KEY &b)) {super::compare((Int(*)(CPtr, CPtr))compare);}

T2(KEY, DATA)  void  ThreadSafeMap<KEY, DATA>::lockedRemove    (  Int   i               ) {       super::lockedRemove    ( i   );}
T2(KEY, DATA)  void  ThreadSafeMap<KEY, DATA>::      remove    (  Int   i               ) {       super::      remove    ( i   );}
T2(KEY, DATA)  void  ThreadSafeMap<KEY, DATA>::      removeKey (C KEY  &key             ) {       super::      removeKey (&key );}
T2(KEY, DATA)  void  ThreadSafeMap<KEY, DATA>::      removeData(C DATA *data            ) {       super::      removeData( data);}
T2(KEY, DATA)  Bool  ThreadSafeMap<KEY, DATA>::      replaceKey(C KEY  &src, C KEY &dest) {return super::      replaceKey(&src, &dest);}

T2(KEY, DATA) T1(EXTENDED)            Map  <KEY, DATA>&            Map  <KEY, DATA>::replaceClass() {ASSERT_BASE_EXTENDED<DATA, EXTENDED>();         del(); _key_offset=UIntPtr(&((typename Map  <KEY, EXTENDED>::Elm*)null)->key); /*_data_offset=UIntPtr(&((typename Map  <KEY, EXTENDED>::Elm*)null)->data);*/ _desc_offset=UIntPtr(&((typename Map  <KEY, EXTENDED>::Elm*)null)->desc); _data_size=SIZE(EXTENDED); _memx.replaceClass<typename Map  <KEY, EXTENDED>::Elm>();           return T;}
T2(KEY, DATA) T1(EXTENDED)            MapEx<KEY, DATA>&            MapEx<KEY, DATA>::replaceClass() {ASSERT_BASE_EXTENDED<DATA, EXTENDED>();         del(); _key_offset=UIntPtr(&((typename MapEx<KEY, EXTENDED>::Elm*)null)->key); /*_data_offset=UIntPtr(&((typename MapEx<KEY, EXTENDED>::Elm*)null)->data);*/ _desc_offset=UIntPtr(&((typename MapEx<KEY, EXTENDED>::Elm*)null)->desc); _data_size=SIZE(EXTENDED); _memx.replaceClass<typename MapEx<KEY, EXTENDED>::Elm>();           return T;}
T2(KEY, DATA) T1(EXTENDED)  ThreadSafeMap  <KEY, DATA>&  ThreadSafeMap  <KEY, DATA>::replaceClass() {ASSERT_BASE_EXTENDED<DATA, EXTENDED>(); lock(); del(); _key_offset=UIntPtr(&((typename Map  <KEY, EXTENDED>::Elm*)null)->key); /*_data_offset=UIntPtr(&((typename Map  <KEY, EXTENDED>::Elm*)null)->data);*/ _desc_offset=UIntPtr(&((typename Map  <KEY, EXTENDED>::Elm*)null)->desc); _data_size=SIZE(EXTENDED); _memx.replaceClass<typename Map  <KEY, EXTENDED>::Elm>(); unlock(); return T;}

T2(KEY, DATA)            Map<KEY, DATA>&            Map<KEY, DATA>::operator=(C           Map<KEY, DATA> &src) {if(this!=&src){                    from(src); FREPA(T)         T[i]=src           [i];                        } return T;}
T2(KEY, DATA)  ThreadSafeMap<KEY, DATA>&  ThreadSafeMap<KEY, DATA>::operator=(C ThreadSafeMap<KEY, DATA> &src) {if(this!=&src){lock(); src.lock(); from(src); FREPA(T)lockedData(i)=src.lockedData(i); src.unlock(); unlock();} return T;}

T2(KEY, DATA)            Map  <KEY, DATA>::          Map  (Int compare(C KEY &a, C KEY &b), Bool create(DATA &data, C KEY &key, Ptr user), Ptr user, Int block_elms) : _Map  (block_elms, (Int(*)(CPtr, CPtr))compare, (Bool(*)(Ptr, CPtr, Ptr))create, user, ClassFunc<KEY>::Copy) {replaceClass<DATA>();}
T2(KEY, DATA)            MapEx<KEY, DATA>::          MapEx(Int compare(C KEY &a, C KEY &b), Bool create(DATA &data, C KEY &key, Ptr user), Ptr user, Int block_elms) : _MapEx(block_elms, (Int(*)(CPtr, CPtr))compare, (Bool(*)(Ptr, CPtr, Ptr))create, user, ClassFunc<KEY>::Copy) {replaceClass<DATA>();}
T2(KEY, DATA)  ThreadSafeMap  <KEY, DATA>::ThreadSafeMap  (Int compare(C KEY &a, C KEY &b), Bool create(DATA &data, C KEY &key, Ptr user), Ptr user, Int block_elms) : _MapTS(block_elms, (Int(*)(CPtr, CPtr))compare, (Bool(*)(Ptr, CPtr, Ptr))create, user, ClassFunc<KEY>::Copy) {replaceClass<DATA>();}

inline Int Elms(C _Map   &map) {return map.elms();}
inline Int Elms(C _MapEx &map) {return map.elms();}
inline Int Elms(C _MapTS &map) {return map.elms();}
/******************************************************************************/
// MAP ELEMENT POINTER
/******************************************************************************/
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  Bool    MapElmPtr<KEY,DATA,MAP>::dummy(          )C {return MAP.dummy(_data       );}
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  void    MapElmPtr<KEY,DATA,MAP>::dummy(Bool dummy)  {       MAP.dummy(_data, dummy);}

template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>&  MapElmPtr<KEY,DATA,MAP>::clear    (                  ) {                                    MAP.decRef(T._data); T._data=       null ;  return T;}
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>&  MapElmPtr<KEY,DATA,MAP>::operator=(  DATA      * data) {if(T!=data){MAP.incRef(      data); MAP.decRef(T._data); T._data=       data ;} return T;} // have to inc ref first, in case new pointer is a child of current that would get released if dec ref was first
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>&  MapElmPtr<KEY,DATA,MAP>::operator=(C MapElmPtr & eptr) {if(T!=eptr){MAP.incRef(eptr._data); MAP.decRef(T._data); T._data= eptr._data ;} return T;} // have to inc ref first, in case new pointer is a child of current that would get released if dec ref was first
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>&  MapElmPtr<KEY,DATA,MAP>::operator=(  MapElmPtr &&eptr) {                                                      Swap(_data, eptr._data);  return T;}
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>&  MapElmPtr<KEY,DATA,MAP>::operator=(  null_t          ) {clear();                                                                        return T;}

template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>&  MapElmPtr<KEY,DATA,MAP>::find     (C KEY &key) {DATA *old=T._data; T._data=(DATA*)MAP.find   (&key, true); MAP.decRef(old); return T;}
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>&  MapElmPtr<KEY,DATA,MAP>::get      (C KEY &key) {DATA *old=T._data; T._data=(DATA*)MAP.get    (&key, true); MAP.decRef(old); return T;}
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>&  MapElmPtr<KEY,DATA,MAP>::require  (C KEY &key) {DATA *old=T._data; T._data=(DATA*)MAP.require(&key, true); MAP.decRef(old); return T;}
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>&  MapElmPtr<KEY,DATA,MAP>::operator=(C KEY &key) {DATA *old=T._data; T._data=(DATA*)MAP.require(&key, true); MAP.decRef(old); return T;}

template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>:: MapElmPtr(  null_t          ) {           T._data=      null ;}
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>:: MapElmPtr(  DATA      * data) {MAP.incRef(T._data=      data);}
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>:: MapElmPtr(C MapElmPtr & eptr) {MAP.incRef(T._data=eptr._data);}
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>:: MapElmPtr(  MapElmPtr &&eptr) {           T._data=eptr._data ; eptr._data=null;}
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>:: MapElmPtr(C KEY       & key ) {           T._data=(DATA*)MAP.require(&key, true);}
template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP>  MapElmPtr<KEY,DATA,MAP>::~MapElmPtr(                  ) {clear();}
/******************************************************************************/
// GRID
/******************************************************************************/
T1(TYPE)  void         Grid<TYPE>:: del(                )  {                    super::del (            );} // delete all      cells
T1(TYPE)  void         Grid<TYPE>:: del(Cell<TYPE> *cell)  {                    super::del ((_Cell*)cell);} // delete selected cell
T1(TYPE)  Cell<TYPE>&  Grid<TYPE>:: get(C VecI2 &xy     )  {return (Cell<TYPE>&)super::get (xy          );} // get 'xy' cell, create it   if not found
T1(TYPE)  Cell<TYPE>*  Grid<TYPE>::find(C VecI2 &xy     )C {return (Cell<TYPE>*)super::find(xy          );} // get 'xy' cell, return null if not found
T1(TYPE)  Bool         Grid<TYPE>::size(  RectI &rect   )C {return              super::size(rect        );} // get rectangle covering all grid cells, false on fail (if no grid cells are present)

T1(TYPE)               Grid<TYPE>&  Grid<TYPE>::fastAccess  (C RectI *rect) {super::fastAccess                  (rect); return T;} // optimize accessing cells (via 'find/get' methods) within 'rect' rectangle, normally cells are accessed recursively, however after calling this method all cells within the rectangle will be available instantly, if null is provided then the optimization is disabled
T1(TYPE) T1(EXTENDED)  Grid<TYPE>&  Grid<TYPE>::replaceClass(             ) {_Grid::replaceClass<TYPE, EXTENDED>(    ); return T;} // replace the type of class stored in the grid, all grid cells are automatically removed before changing the type of the class, the new type must be extended from the base 'TYPE' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

// call custom function on grid cells
T1(TYPE)                void  Grid<TYPE>::func      (               void func(Cell<TYPE> &cell, Ptr        user)                 ) {super::func      (      (void (*)(_Cell &cell, Ptr user))func,  null);} // call 'func' on all existing grid cells
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::func      (               void func(Cell<TYPE> &cell, USER_DATA *user), USER_DATA *user) {super::func      (      (void (*)(_Cell &cell, Ptr user))func,  user);} // call 'func' on all existing grid cells
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::func      (               void func(Cell<TYPE> &cell, USER_DATA &user), USER_DATA &user) {super::func      (      (void (*)(_Cell &cell, Ptr user))func, &user);} // call 'func' on all existing grid cells
T1(TYPE)                void  Grid<TYPE>::func      (C RectI &rect, void func(Cell<TYPE> &cell, Ptr        user)                 ) {super::func      (rect, (void (*)(_Cell &cell, Ptr user))func,  null);} // call 'func' on all existing grid cells in specified rectangle
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::func      (C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA *user), USER_DATA *user) {super::func      (rect, (void (*)(_Cell &cell, Ptr user))func,  user);} // call 'func' on all existing grid cells in specified rectangle
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::func      (C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA &user), USER_DATA &user) {super::func      (rect, (void (*)(_Cell &cell, Ptr user))func, &user);} // call 'func' on all existing grid cells in specified rectangle
T1(TYPE)                void  Grid<TYPE>::funcCreate(C RectI &rect, void func(Cell<TYPE> &cell, Ptr        user)                 ) {super::funcCreate(rect, (void (*)(_Cell &cell, Ptr user))func,  null);} // call 'func' on all          grid cells in specified rectangle (this method creates grid cells if they don't exist yet)
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::funcCreate(C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA *user), USER_DATA *user) {super::funcCreate(rect, (void (*)(_Cell &cell, Ptr user))func,  user);} // call 'func' on all          grid cells in specified rectangle (this method creates grid cells if they don't exist yet)
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::funcCreate(C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA &user), USER_DATA &user) {super::funcCreate(rect, (void (*)(_Cell &cell, Ptr user))func, &user);} // call 'func' on all          grid cells in specified rectangle (this method creates grid cells if they don't exist yet)

// call custom function on grid cells (multi-threaded version)
T1(TYPE)                void  Grid<TYPE>::mtFunc(Threads &threads,                void func(Cell<TYPE> &cell, Ptr        user, Int thread_index)                 ) {super::mtFunc(threads,       (void (*)(_Cell &cell, Ptr user, Int thread_index))func,  null);} // call 'func' on all existing grid cells
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::mtFunc(Threads &threads,                void func(Cell<TYPE> &cell, USER_DATA *user, Int thread_index), USER_DATA *user) {super::mtFunc(threads,       (void (*)(_Cell &cell, Ptr user, Int thread_index))func,  user);} // call 'func' on all existing grid cells
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::mtFunc(Threads &threads,                void func(Cell<TYPE> &cell, USER_DATA &user, Int thread_index), USER_DATA &user) {super::mtFunc(threads,       (void (*)(_Cell &cell, Ptr user, Int thread_index))func, &user);} // call 'func' on all existing grid cells
T1(TYPE)                void  Grid<TYPE>::mtFunc(Threads &threads, C RectI &rect, void func(Cell<TYPE> &cell, Ptr        user, Int thread_index)                 ) {super::mtFunc(threads, rect, (void (*)(_Cell &cell, Ptr user, Int thread_index))func,  null);} // call 'func' on all existing grid cells in specified rectangle
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::mtFunc(Threads &threads, C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA *user, Int thread_index), USER_DATA *user) {super::mtFunc(threads, rect, (void (*)(_Cell &cell, Ptr user, Int thread_index))func,  user);} // call 'func' on all existing grid cells in specified rectangle
T1(TYPE) T1(USER_DATA)  void  Grid<TYPE>::mtFunc(Threads &threads, C RectI &rect, void func(Cell<TYPE> &cell, USER_DATA &user, Int thread_index), USER_DATA &user) {super::mtFunc(threads, rect, (void (*)(_Cell &cell, Ptr user, Int thread_index))func, &user);} // call 'func' on all existing grid cells in specified rectangle
/******************************************************************************/
// GAME::OBJ_MEMX
/******************************************************************************/
namespace Game
{
   T1(TYPE)  ObjMap<TYPE>&  ObjMap<TYPE>::clear() {_map.clear(); return T;}

   T1(TYPE)  Int  ObjMap<TYPE>::elms   ()C {return _map.elms    ();}
   T1(TYPE)  Int  ObjMap<TYPE>::elmSize()C {return _map.dataSize();}

   T1(TYPE)    TYPE&  ObjMap<TYPE>::operator[](Int i)  {return _map[i];}
   T1(TYPE)  C TYPE&  ObjMap<TYPE>::operator[](Int i)C {return _map[i];}

   T1(TYPE)  Bool  ObjMap<TYPE>::containsId (C UID  &obj_id)C {return obj_id.valid() ? _map.containsKey (obj_id) : false;}
   T1(TYPE)  Bool  ObjMap<TYPE>::containsObj(C TYPE *obj   )C {return                  _map.containsData(obj   )        ;}

   T1(TYPE)  TYPE*  ObjMap<TYPE>::find(C UID &obj_id) {return obj_id.valid() ? _map.find(obj_id) : null;}

   T1(TYPE)  ObjMap<TYPE>&  ObjMap<TYPE>::remove   (  Int   i     ) {                  _map.remove    (i     ); return T;}
   T1(TYPE)  ObjMap<TYPE>&  ObjMap<TYPE>::removeId (C UID  &obj_id) {if(obj_id.valid())_map.removeKey (obj_id); return T;}
   T1(TYPE)  ObjMap<TYPE>&  ObjMap<TYPE>::removeObj(C TYPE *data  ) {                  _map.removeData(data  ); return T;}

   T1(TYPE) T1(BASE)  ObjMap<TYPE>::operator    ObjMap<BASE>&()  {ASSERT_BASE_EXTENDED<BASE, TYPE>(); return *(  ObjMap<BASE>*)this;}
   T1(TYPE) T1(BASE)  ObjMap<TYPE>::operator  C ObjMap<BASE>&()C {ASSERT_BASE_EXTENDED<BASE, TYPE>(); return *(C ObjMap<BASE>*)this;}

   T1(TYPE)  ObjMap<TYPE>::ObjMap(Int block_elms) : _map(Compare, null, null, block_elms) {}
}
T1(TYPE)  Int  Elms(C Game::ObjMap<TYPE> &obj_map) {return obj_map.elms();}
/******************************************************************************/
// LIST
/******************************************************************************/
T1(TYPE)  _List&  _List::setData    (TYPE       *data, Int elms, C CMemPtr<Bool> &visible, Bool keep_cur) {return _setData(     data  ,      elms  ,     SIZE(TYPE), visible, keep_cur);}
T1(TYPE)  _List&  _List::setData    (Mems<TYPE> &mems,           C CMemPtr<Bool> &visible, Bool keep_cur) {return _setData(mems.data(), mems.elms(), mems.elmSize(), visible, keep_cur);}
T1(TYPE)  _List&  _List::setDataNode(Memx<TYPE> &memx,           C CMemPtr<Bool> &visible, Bool keep_cur) {return _setData(memx       ,      OFFSET(TYPE, children), visible, keep_cur); SCAST(Memx<TYPE>, MEMBER(TYPE, children));} // cast verifies that 'children' member can be casted to 'Memx<TYPE>' which is a requirement
/******************************************************************************/
// INTERPOLATOR
/******************************************************************************/
T1(TYPE)  AngularInterpolator<TYPE>::AngularInterpolator() {_value=_prev=_cur=_next=0;}
T1(TYPE)   LinearInterpolator<TYPE>:: LinearInterpolator() {_value=_prev=_cur=_next=0;}
T1(TYPE)   SplineInterpolator<TYPE>:: SplineInterpolator() {_value=_prev2=_prev=_cur=_next=0;}

T1(TYPE)  void AngularInterpolator<TYPE>::add(C TYPE &value, C InterpolatorTemp &temp)
{
   switch(temp.op)
   {
      case 0: _prev=_value=value; break; // initialize '_value' already so we can access it ASAP
      case 1:               _cur =_prev+AngleDelta(_prev, value); break;
      case 2: _prev=_value; _cur =_prev+AngleDelta(_prev, value); break; // start interpolating from current value
      case 3:               _next=_cur +AngleDelta(_cur , value); break;
   }
}
T1(TYPE)  void LinearInterpolator<TYPE>::add(C TYPE &value, C InterpolatorTemp &temp)
{
   switch(temp.op)
   {
      case 0: _prev=_value=value; break; // initialize '_value' already so we can access it ASAP
      case 1:               _cur =value; break;
      case 2: _prev=_value; _cur =value; break; // start interpolating from current value
      case 3:               _next=value; break;
   }
}
T1(TYPE)  void SplineInterpolator<TYPE>::add(C TYPE &value, C InterpolatorTemp &temp)
{
   switch(temp.op)
   {
      case 0: _prev=_prev2=_value=value; break; // initialize '_value' already so we can access it ASAP
      case 1:               _cur =value; break;
      case 2: _prev=_value; _cur =value; break; // start interpolating from current value
      case 3:               _next=value; break;
   }
}

T1(TYPE)  void AngularInterpolator<TYPE>::update(C InterpolatorTemp &temp) {if(temp.op){              _prev=_cur; _cur=_next;} _value=Lerp (        _prev, _cur,                  temp.frac);}
T1(TYPE)  void  LinearInterpolator<TYPE>::update(C InterpolatorTemp &temp) {if(temp.op){              _prev=_cur; _cur=_next;} _value=Lerp (        _prev, _cur,                  temp.frac);}
T1(TYPE)  void  SplineInterpolator<TYPE>::update(C InterpolatorTemp &temp) {if(temp.op){_prev2=_prev; _prev=_cur; _cur=_next;} _value=Lerp4(_prev2, _prev, _cur, _cur-_prev+_cur, temp.frac);} // predict next instead of using '_next' because we may not know it in all cases
/******************************************************************************/
// IO
/******************************************************************************/
T1(TYPE)  void  FList(C Str &path, FILE_LIST_MODE func(C FileFind &ff, TYPE *user), TYPE *user) {FList(path, (FILE_LIST_MODE (*)(C FileFind &ff, Ptr user))func,  user);}
T1(TYPE)  void  FList(C Str &path, FILE_LIST_MODE func(C FileFind &ff, TYPE &user), TYPE &user) {FList(path, (FILE_LIST_MODE (*)(C FileFind &ff, Ptr user))func, &user);}
/******************************************************************************/
// GUI
/******************************************************************************/
T1(TYPE)  ListColumn::ListColumn(TYPE &member                     , Flt width, C Str &name) : md(member) {create(null                           , width, name);}
T1(TYPE)  ListColumn::ListColumn(Str (*data_to_text)(C TYPE &data), Flt width, C Str &name)              {create((Str(*)(CPtr data))data_to_text, width, name);}
/******************************************************************************/
// GRAPHICS
/******************************************************************************/
#if EE_PRIVATE
INLINE void DisplayState::primType(UInt prim_type)
{
   if(D._prim_type!=prim_type)
   {
      D._prim_type=prim_type;
   #if DX11
      D3DC->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)prim_type);
   #endif
   }
}
#endif
/******************************************************************************/
// MESH
/******************************************************************************/
inline void MeshPart::draw(C MatrixM &matrix)C {draw(matrix, matrix);}
inline void MeshLod ::draw(C MatrixM &matrix)C {draw(matrix, matrix);}
/******************************************************************************/
// SOUND
/******************************************************************************/
inline Int SndOpusEncoder::frequency   ()C {return _frequency;}
inline Int SndOpusEncoder::bytes       ()C {return _encoder.bytes();}
inline Int SndOpusEncoder::bits        ()C {return _encoder.bits();}
inline Int SndOpusEncoder::channels    ()C {return _encoder.channels();}
inline Int SndOpusEncoder::block       ()C {return _encoder.block();}
inline Int SndOpusEncoder::frameSamples()C {return _encoder.frameSamples();}
/******************************************************************************/
// EDIT
/******************************************************************************/
namespace Edit
{
   inline Int _Undo::changes()C {return _changes.elms();}

   T1(TYPE)  Undo<TYPE>::Undo(Bool full, Ptr user, Flt time) : _Undo(full, user, time) {replaceClass<TYPE>();}

   T1(TYPE)  TYPE*  Undo<TYPE>::getNextUndo() {return (TYPE*)super::getNextUndo();}
   T1(TYPE)  TYPE*  Undo<TYPE>::getNextRedo() {return (TYPE*)super::getNextRedo();}

   T1(TYPE)  TYPE&  Undo<TYPE>::operator[](Int i) {return (TYPE&)super::operator[](i);}
   T1(TYPE)  TYPE*  Undo<TYPE>::addr      (Int i) {return (TYPE*)super::addr      (i);}

   T1(TYPE)  TYPE*  Undo<TYPE>::set(CPtr change_type, Bool force_create, Flt extra_time) {return (TYPE*)super::set(change_type, force_create, extra_time);}
   T1(TYPE)  TYPE*  Undo<TYPE>::set( Int change_type, Bool force_create, Flt extra_time) {return (TYPE*)super::set(change_type, force_create, extra_time);}

   T1(TYPE) T1(CHANGE)  Undo<TYPE>&  Undo<TYPE>::replaceClass() {ASSERT_BASE_EXTENDED<TYPE, CHANGE>(); _changes.replaceClass<CHANGE>(); return T;}

   inline C ObjData::Param* ObjData::findParam(C Str &name, PARAM_TYPE type, Bool include_removed, Bool include_inherited)C {return ConstCast(T).findParam(name, type, include_removed, include_inherited);}
}
/******************************************************************************/
#if EE_PRIVATE

#if X86 || (ARM && X64) || WEB // x86 32/64 and ARM 64 can do unaligned reads. When using WebAssembly (WASM) for WEB platform, unaligned access is supported, however when executed on platforms without native unaligned access support (Arm32) it will be extremely slow, however since Arm32 is in extinction then it's better to enable unaligned access to get better performance on majority of platforms that support it.
   T1(TYPE) C TYPE& Unaligned(              C TYPE &src) {return src;}
   T1(TYPE)   void  Unaligned(TYPE   &dest, C TYPE &src) {  dest=src;}
   T1(TYPE)   void _Unaligned(Byte   &dest, C TYPE &src) {  dest=src;}
   T1(TYPE)   void _Unaligned(Short  &dest, C TYPE &src) {  dest=src;}
   T1(TYPE)   void _Unaligned(UShort &dest, C TYPE &src) {  dest=src;}
   T1(TYPE)   void _Unaligned(Int    &dest, C TYPE &src) {  dest=src;}
   T1(TYPE)   void _Unaligned(UInt   &dest, C TYPE &src) {  dest=src;}
#else
   T1(TYPE)   TYPE  Unaligned(              C TYPE &src) {if(SIZE(TYPE)==1)return src;else{TYPE temp; CopyFast(Ptr(&temp), CPtr(&src), SIZE(TYPE)); return temp;}} // !! these functions must casted to 'Ptr', because without it, compiler may try to inline the 'memcpy' when it detects that both params are of the same type and in that case it will assume that they are memory aligned and crash will occur !!
   T1(TYPE)   void  Unaligned(TYPE   &dest, C TYPE &src) {if(SIZE(TYPE)==1)  dest=src;else{           CopyFast(Ptr(&dest), CPtr(&src), SIZE(TYPE));             }} // !! these functions must casted to 'Ptr', because without it, compiler may try to inline the 'memcpy' when it detects that both params are of the same type and in that case it will assume that they are memory aligned and crash will occur !!
   T1(TYPE)   void _Unaligned(Byte   &dest, C TYPE &src) {                   dest=Unaligned(src) ;                                                               }
   T1(TYPE)   void _Unaligned(Short  &dest, C TYPE &src) {Unaligned(dest, (Short )Unaligned(src));                                                               }
   T1(TYPE)   void _Unaligned(UShort &dest, C TYPE &src) {Unaligned(dest, (UShort)Unaligned(src));                                                               }
   T1(TYPE)   void _Unaligned(Int    &dest, C TYPE &src) {Unaligned(dest, (Int   )Unaligned(src));                                                               }
   T1(TYPE)   void _Unaligned(UInt   &dest, C TYPE &src) {Unaligned(dest, (UInt  )Unaligned(src));                                                               }
#endif

T2(TA, TB)  File&  File::putMulti(C TA &a, C TB &b)
{
 C UInt size=SIZE(a)+SIZE(b);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   T<<buf;
   return T;
}
T2(TA, TB)  File&  File::getMulti(TA &a, TB &b)
{
 C UInt size=SIZE(a)+SIZE(b);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   return T;
}

T3(TA, TB, TC)  File&  File::putMulti(C TA &a, C TB &b, C TC &c)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   T<<buf;
   return T;
}
T3(TA, TB, TC)  File&  File::getMulti(TA &a, TB &b, TC &c)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   return T;
}

T4(TA, TB, TC, TD)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   T<<buf;
   return T;
}
T4(TA, TB, TC, TD)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   return T;
}

T5(TA, TB, TC, TD, TE)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   Unaligned((TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]), e);
   T<<buf;
   return T;
}
T5(TA, TB, TC, TD, TE)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d, TE &e)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   Unaligned(e, (TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]));
   return T;
}

T6(TA, TB, TC, TD, TE, TF)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   Unaligned((TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]), e);
   Unaligned((TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]), f);
   T<<buf;
   return T;
}
T6(TA, TB, TC, TD, TE, TF)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d, TE &e, TF &f)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   Unaligned(e, (TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]));
   Unaligned(f, (TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]));
   return T;
}

T7(TA, TB, TC, TD, TE, TF, TG)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f, C TG &g)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   Unaligned((TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]), e);
   Unaligned((TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]), f);
   Unaligned((TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]), g);
   T<<buf;
   return T;
}
T7(TA, TB, TC, TD, TE, TF, TG)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d, TE &e, TF &f, TG &g)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   Unaligned(e, (TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]));
   Unaligned(f, (TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]));
   Unaligned(g, (TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]));
   return T;
}

T8(TA, TB, TC, TD, TE, TF, TG, TH)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f, C TG &g, C TH &h)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   Unaligned((TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]), e);
   Unaligned((TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]), f);
   Unaligned((TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]), g);
   Unaligned((TH&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)]), h);
   T<<buf;
   return T;
}
T8(TA, TB, TC, TD, TE, TF, TG, TH)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d, TE &e, TF &f, TG &g, TH &h)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   Unaligned(e, (TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]));
   Unaligned(f, (TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]));
   Unaligned(g, (TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]));
   Unaligned(h, (TH&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)]));
   return T;
}

T9(TA, TB, TC, TD, TE, TF, TG, TH, TI)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f, C TG &g, C TH &h, C TI &i)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   Unaligned((TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]), e);
   Unaligned((TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]), f);
   Unaligned((TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]), g);
   Unaligned((TH&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)]), h);
   Unaligned((TI&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)]), i);
   T<<buf;
   return T;
}
T9(TA, TB, TC, TD, TE, TF, TG, TH, TI)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d, TE &e, TF &f, TG &g, TH &h, TI &i)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   Unaligned(e, (TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]));
   Unaligned(f, (TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]));
   Unaligned(g, (TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]));
   Unaligned(h, (TH&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)]));
   Unaligned(i, (TI&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)]));
   return T;
}

T10(TA, TB, TC, TD, TE, TF, TG, TH, TI, TJ)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f, C TG &g, C TH &h, C TI &i, C TJ &j)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i)+SIZE(j);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   Unaligned((TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]), e);
   Unaligned((TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]), f);
   Unaligned((TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]), g);
   Unaligned((TH&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)]), h);
   Unaligned((TI&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)]), i);
   Unaligned((TJ&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i)]), j);
   T<<buf;
   return T;
}
T10(TA, TB, TC, TD, TE, TF, TG, TH, TI, TJ)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d, TE &e, TF &f, TG &g, TH &h, TI &i, TJ &j)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i)+SIZE(j);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   Unaligned(e, (TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]));
   Unaligned(f, (TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]));
   Unaligned(g, (TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]));
   Unaligned(h, (TH&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)]));
   Unaligned(i, (TI&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)]));
   Unaligned(j, (TJ&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i)]));
   return T;
}

T11(TA, TB, TC, TD, TE, TF, TG, TH, TI, TJ, TK)  File&  File::putMulti(C TA &a, C TB &b, C TC &c, C TD &d, C TE &e, C TF &f, C TG &g, C TH &h, C TI &i, C TJ &j, C TK &k)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i)+SIZE(j)+SIZE(k);
   Byte buf[size];
   Unaligned((TA&)(buf[0]), a);
   Unaligned((TB&)(buf[SIZE(a)]), b);
   Unaligned((TC&)(buf[SIZE(a)+SIZE(b)]), c);
   Unaligned((TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]), d);
   Unaligned((TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]), e);
   Unaligned((TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]), f);
   Unaligned((TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]), g);
   Unaligned((TH&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)]), h);
   Unaligned((TI&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)]), i);
   Unaligned((TJ&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i)]), j);
   Unaligned((TK&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i)+SIZE(j)]), k);
   T<<buf;
   return T;
}
T11(TA, TB, TC, TD, TE, TF, TG, TH, TI, TJ, TK)  File&  File::getMulti(TA &a, TB &b, TC &c, TD &d, TE &e, TF &f, TG &g, TH &h, TI &i, TJ &j, TK &k)
{
 C UInt size=SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i)+SIZE(j)+SIZE(k);
   Byte buf[size]; T>>buf;
   Unaligned(a, (TA&)(buf[0]));
   Unaligned(b, (TB&)(buf[SIZE(a)]));
   Unaligned(c, (TC&)(buf[SIZE(a)+SIZE(b)]));
   Unaligned(d, (TD&)(buf[SIZE(a)+SIZE(b)+SIZE(c)]));
   Unaligned(e, (TE&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)]));
   Unaligned(f, (TF&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)]));
   Unaligned(g, (TG&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)]));
   Unaligned(h, (TH&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)]));
   Unaligned(i, (TI&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)]));
   Unaligned(j, (TJ&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i)]));
   Unaligned(k, (TK&)(buf[SIZE(a)+SIZE(b)+SIZE(c)+SIZE(d)+SIZE(e)+SIZE(f)+SIZE(g)+SIZE(h)+SIZE(i)+SIZE(j)]));
   return T;
}
#endif
/******************************************************************************/
