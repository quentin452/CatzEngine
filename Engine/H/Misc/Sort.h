/******************************************************************************

   Use 'Sort' functions to sort custom data.

   Use 'BinarySearch' functions to perform binary search on custom data.

/******************************************************************************/
// Basic Compare Functions
inline Int Compare(C SByte  &a, C SByte  &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Byte   &a, C Byte   &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Short  &a, C Short  &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C UShort &a, C UShort &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Int    &a, C Int    &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C UInt   &a, C UInt   &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Long   &a, C Long   &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C ULong  &a, C ULong  &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Flt    &a, C Flt    &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Dbl    &a, C Dbl    &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Ptr    &a, C Ptr    &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C CPtr   &a, C CPtr   &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1

T1(TYPE) ENABLE_IF_ENUM(TYPE, Int) Compare(C TYPE &a, C TYPE &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' enum values and return -1, 0, +1

inline Int ComparePtr(CPtr a, CPtr b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
/******************************************************************************/
// have to forward declare 'Compare' functions for Clang compiler so functions using default parameters "Int compare(C TYPE &a, C VALUE &b)=Compare" can use them
Int Compare(C Half   &a, C Half   &b);
Int Compare(C UID    &a, C UID    &b);
Int Compare(C Vec2   &a, C Vec2   &b);
Int Compare(C VecD2  &a, C VecD2  &b);
Int Compare(C Vec    &a, C Vec    &b);
Int Compare(C VecD   &a, C VecD   &b);
Int Compare(C Vec4   &a, C Vec4   &b);
Int Compare(C VecD4  &a, C VecD4  &b);
Int Compare(C VecI2  &a, C VecI2  &b);
Int Compare(C VecB2  &a, C VecB2  &b);
Int Compare(C VecSB2 &a, C VecSB2 &b);
Int Compare(C VecUS2 &a, C VecUS2 &b);
Int Compare(C VecI   &a, C VecI   &b);
Int Compare(C VecB   &a, C VecB   &b);
Int Compare(C VecSB  &a, C VecSB  &b);
Int Compare(C VecUS  &a, C VecUS  &b);
Int Compare(C VecI4  &a, C VecI4  &b);
Int Compare(C VecB4  &a, C VecB4  &b);
Int Compare(C VecSB4 &a, C VecSB4 &b);
Int Compare(C Color  &a, C Color  &b);
Int Compare(C VecH2  &a, C VecH2  &b);
Int Compare(C VecH   &a, C VecH   &b);
Int Compare(C VecH4  &a, C VecH4  &b);
Int Compare(C Rect   &a, C Rect   &b);
Int Compare(C RectI  &a, C RectI  &b);
Int Compare(C IndexWeight &a, C IndexWeight &b);
/******************************************************************************/
// Sort Data
         void Sort(Int  *data, Int elms                                           ); // sort Int    array
         void Sort(Flt  *data, Int elms                                           ); // sort Flt    array
         void Sort(Dbl  *data, Int elms                                           ); // sort Dbl    array
T1(TYPE) void Sort(TYPE *data, Int elms, Int compare(C TYPE &a, C TYPE &b)=Compare); // sort custom array using custom comparing function

// Binary Search, search sorted 'data' array for presence of 'value' and return if it was found in the array, 'index'=if the function returned true then this index points to the location where the 'value' is located in the array, if the function returned false then it means that 'value' was not found in the array however the 'index' points to the place where it should be added in the array while preserving sorted data, 'index' will always be in range (0..elms) inclusive
T2(DATA, VALUE)  Bool   BinarySearch     (C DATA *data, Int elms, C VALUE &value, Int &index, Int compare(C DATA &a, C VALUE &b)=Compare);
T2(DATA, VALUE)  Bool   BinarySearchFirst(C DATA *data, Int elms, C VALUE &value, Int &index, Int compare(C DATA &a, C VALUE &b)=Compare); // if there are multiple elements with same value, then the first one will be returned
T2(DATA, VALUE)  Bool   BinaryHas        (C DATA *data, Int elms, C VALUE &value,             Int compare(C DATA &a, C VALUE &b)=Compare) {Int i; return BinarySearch(data, elms, value, i, compare);                  } // check if 'value' is present in array
T2(DATA, VALUE)  DATA*  BinaryFind       (  DATA *data, Int elms, C VALUE &value,             Int compare(C DATA &a, C VALUE &b)=Compare) {Int i; return BinarySearch(data, elms, value, i, compare) ? &data[i] : null;} // check if 'value' is present in array and return it, null on fail
/******************************************************************************/
