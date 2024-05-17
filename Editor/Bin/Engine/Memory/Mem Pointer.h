/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'MemPtr' to operate on any kind of memory container.

   'MemPtr' is a pointer to one of all supported memory container types:
      'Mems', 'Memc', 'Memt', 'Memb', 'Memx', 'Meml' or regular continuous memory.

   'MemPtr' allows to write one function which can operate on all memory container types.

      For example, instead of writing multiple functions which accept different containers:
         Int Sum(C Memc<Int> &values) {Int sum=0; REPA(values)sum+=values[i]; return sum;}
         Int Sum(C Memb<Int> &values) {Int sum=0; REPA(values)sum+=values[i]; return sum;}
         ..

      You can write just one function:
         Int Sum(C CMemPtr<Int> &values) {Int sum=0; REPA(values)sum+=values[i]; return sum;}

         This function will accept all memory container types, so you can do the following:
            Memc<Int> memc_values; Sum(memc_values);
            Memb<Int> memb_values; Sum(memb_values);
            ..

/******************************************************************************/
template<typename TYPE, Int Memt_size> struct CMemPtr // Const Memory Container Pointer, 'Memt_size'=size of the 'Memt' helper stack memory
{
   // get / set
   Int     elms    ()C; // number of elements
   UInt    elmSize ()C; // size   of element
   UIntPtr memUsage()C; // memory usage

 C TYPE* data      (     )C; // get pointer to the start of the elements
 C TYPE* addr      (Int i)C; // get i-th  element address, null is returned if index is out of range
 C TYPE& operator[](Int i)C; // get i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
 C TYPE& first     (     )C; // get first element
 C TYPE& last      (     )C; // get last  element

   Int  index   (C TYPE *elm)C; // get index of element in container, -1 on fail      , testing is done by comparing elements memory address only
   Bool contains(C TYPE *elm)C; // check if memory container actually contains element, testing is done by comparing elements memory address only

   operator   Bool()C; // if points to something (not null)
   Bool  resizable()C; // if supports adding/removing elements
   Bool continuous()C; // if elements are stored in continuous memory

   // values
   T1(VALUE) Int  find(C VALUE &value)C {REPA(T)if(T[i]==value)return i; return -1;} // check if 'value' is present in container and return its index, -1 if not found
   T1(VALUE) Bool has (C VALUE &value)C {return find(value)>=0;                    } // check if 'value' is present in container

   T1(VALUE)   Bool  binarySearch(C VALUE &value, Int &index, Int compare(C TYPE &a, C VALUE &b)=Compare)C; // search sorted container for presence of 'value' and return if it was found in the container, 'index'=if the function returned true then this index points to the location where the 'value' is located in the container, if the function returned false then it means that 'value' was not found in the container however the 'index' points to the place where it should be added in the container while preserving sorted data, 'index' will always be in range (0..elms) inclusive
   T1(VALUE)   Bool  binaryHas   (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {Int i; return binarySearch(value, i, compare);               } // check if 'value' (using binary search) is present in container
   T1(VALUE) C TYPE* binaryFind  (C VALUE &value,             Int compare(C TYPE &a, C VALUE &b)=Compare)C {Int i; return binarySearch(value, i, compare) ? &T[i] : null;} // check if 'value' (using binary search) is present in container and return it, null on fail

#if EE_PRIVATE
   void copyTo(TYPE *dest)C; // copy raw memory of all elements to 'dest'
#endif

   // io
   Bool save   (File &f)C; // save elements with their own 'save' method, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail
   Bool saveRaw(File &f)C; // save raw memory of elements (number of elements + elements raw memory), false on fail

   // initialize 'CMemPtr' to point to source
                          CMemPtr& point(          null_t=null                         );
                          CMemPtr& point(C         TYPE             &src               );
                          CMemPtr& point(C         TYPE             *src, Int src_elms );
   template<Int src_elms> CMemPtr& point(C         TYPE            (&src)    [src_elms]);
                          CMemPtr& point(C  Mems  <TYPE           > &src               );
                          CMemPtr& point(C  Mems  <TYPE           > *src               );
                          CMemPtr& point(C  Memc  <TYPE           > &src               );
                          CMemPtr& point(C  Memc  <TYPE           > *src               );
   T1(EXTENDED)           CMemPtr& point(C  Memc  <EXTENDED       > *src               );
                          CMemPtr& point(C  Memt  <TYPE, Memt_size> &src               );
                          CMemPtr& point(C  Memt  <TYPE, Memt_size> *src               );
                          CMemPtr& point(C  Memb  <TYPE           > &src               );
                          CMemPtr& point(C  Memb  <TYPE           > *src               );
   T1(EXTENDED)           CMemPtr& point(C  Memb  <EXTENDED       > *src               );
                          CMemPtr& point(C  Memx  <TYPE           > &src               );
                          CMemPtr& point(C  Memx  <TYPE           > *src               );
   T1(EXTENDED)           CMemPtr& point(C  Memx  <EXTENDED       > *src               );
                          CMemPtr& point(C  Meml  <TYPE           > &src               );
                          CMemPtr& point(C  Meml  <TYPE           > *src               );
                          CMemPtr& point(C CMemPtr<TYPE, Memt_size> &src               );

                          CMemPtr(          null_t=null                         ) {point(null         );}
                          CMemPtr(C         TYPE             &src               ) {point(src          );}
   template<Int src_elms> CMemPtr(C         TYPE            (&src)    [src_elms]) {point(src          );}
                          CMemPtr(C         TYPE             *src, Int src_elms ) {point(src, src_elms);}
                          CMemPtr(C  Mems  <TYPE           > &src               ) {point(src          );}
                          CMemPtr(C  Mems  <TYPE           > *src               ) {point(src          );}
                          CMemPtr(C  Memc  <TYPE           > &src               ) {point(src          );}
                          CMemPtr(C  Memc  <TYPE           > *src               ) {point(src          );}
   T1(EXTENDED)           CMemPtr(C  Memc  <EXTENDED       > *src               ) {point(src          );}
                          CMemPtr(C  Memt  <TYPE, Memt_size> &src               ) {point(src          );}
                          CMemPtr(C  Memt  <TYPE, Memt_size> *src               ) {point(src          );}
                          CMemPtr(C  Memb  <TYPE           > &src               ) {point(src          );}
                          CMemPtr(C  Memb  <TYPE           > *src               ) {point(src          );}
   T1(EXTENDED)           CMemPtr(C  Memb  <EXTENDED       > *src               ) {point(src          );}
                          CMemPtr(C  Memx  <TYPE           > &src               ) {point(src          );}
                          CMemPtr(C  Memx  <TYPE           > *src               ) {point(src          );}
   T1(EXTENDED)           CMemPtr(C  Memx  <EXTENDED       > *src               ) {point(src          );}
                          CMemPtr(C  Meml  <TYPE           > &src               ) {point(src          );}
                          CMemPtr(C  Meml  <TYPE           > *src               ) {point(src          );}
                          CMemPtr(C CMemPtr<TYPE, Memt_size> &src               ) {point(src          );}

   enum MODE
   {
      PTR ,
      MEMS,
      MEMC,
      MEMT,
      MEMB,
      MEMX,
      MEML,
   };

   MODE mode()C {return _mode;} // get type of container from which 'CMemPtr' was created

 C Mems<TYPE           >* mems()C {return (_mode==MEMS) ? _mems : null;}
 C Memc<TYPE           >* memc()C {return (_mode==MEMC) ? _memc : null;}
 C Memt<TYPE, Memt_size>* memt()C {return (_mode==MEMT) ? _memt : null;}
 C Memb<TYPE           >* memb()C {return (_mode==MEMB) ? _memb : null;}
 C Memx<TYPE           >* memx()C {return (_mode==MEMX) ? _memx : null;}
 C Meml<TYPE           >* meml()C {return (_mode==MEML) ? _meml : null;}

private:
                          CMemPtr& operator=(   null_t                                 )=delete;
                          CMemPtr& operator=(C  TYPE                    &src           )=delete;
   template<Int src_elms> CMemPtr& operator=(C  TYPE                   (&src)[src_elms])=delete;
                          CMemPtr& operator=(C  Mems  <TYPE           > &src           )=delete;
                          CMemPtr& operator=(C  Memc  <TYPE           > &src           )=delete;
   template<Int src_size> CMemPtr& operator=(C  Memt  <TYPE,  src_size> &src           )=delete;
                          CMemPtr& operator=(C  Memb  <TYPE           > &src           )=delete;
                          CMemPtr& operator=(C  Memx  <TYPE           > &src           )=delete;
                          CMemPtr& operator=(C  Meml  <TYPE           > &src           )=delete;
   template<Int src_size> CMemPtr& operator=(C CMemPtr<TYPE,  src_size> &src           )=delete;
                          CMemPtr& operator=(C CMemPtr<TYPE, Memt_size> &src           )=delete; // this must be specified even though method above should do the same, because without it compiler will try to use the built-in 'operator=' which will just do raw memory copy

   union
   {
    C TYPE                  *_ptr ;
    C Mems<TYPE           > *_mems;
    C Memc<TYPE           > *_memc;
    C Memt<TYPE, Memt_size> *_memt;
    C Memb<TYPE           > *_memb;
    C Memx<TYPE           > *_memx;
    C Meml<TYPE           > *_meml;
   };
   MODE _mode;
   Int  _ptr_elms;

   friend struct Mems  <TYPE>;
   friend struct Memc  <TYPE>;
   friend struct Memt  <TYPE, Memt_size>;
   friend struct Memb  <TYPE>;
   friend struct Memx  <TYPE>;
   friend struct Meml  <TYPE>;
   friend struct MemPtr<TYPE, Memt_size>;
};
/******************************************************************************/
template<typename TYPE, Int Memt_size> struct MemPtr : CMemPtr<TYPE, Memt_size> // Memory Container Pointer, 'Memt_size'=size of the 'Memt' helper stack memory
{
   // manage
   MemPtr& clear(); // remove all elements
   MemPtr& del  (); // remove all elements and free helper memory

   // get / set
   TYPE* data      (     ) ; // get    pointer to the start of the elements
 C TYPE* data      (     )C; // get    pointer to the start of the elements
   TYPE* addr      (Int i) ; // get    i-th  element address, null is returned if index is out of range
 C TYPE* addr      (Int i)C; // get    i-th  element address, null is returned if index is out of range
   TYPE& operator[](Int i) ; // get    i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
 C TYPE& operator[](Int i)C; // get    i-th  element, accessing element out of range is an invalid operation and may cause undefined behavior
   TYPE& operator()(Int i) ; // get    i-th  element, accessing element out of range will cause creation of all elements before it, memory of those elements will be first zeroed before calling their constructor
   TYPE& first     (     ) ; // get    first element
 C TYPE& first     (     )C; // get    first element
   TYPE& last      (     ) ; // get    last  element
 C TYPE& last      (     )C; // get    last  element
   TYPE& New       (     ) ; // create new   element at the  end                                                                              , this method changes the memory address of all elements
   TYPE& NewAt     (Int i) ; // create new   element at i-th position, all old elements starting from i-th position will be moved to the right, this method changes the memory address of all elements

   // remove
   MemPtr& removeLast(                                  ); // remove last element                                                                                                                                                                        , this method does not change the memory address of any of the remaining elements
   MemPtr& remove    (  Int   i  , Bool keep_order=false); // remove i-th element                        , if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements
   MemPtr& removeData(C TYPE *elm, Bool keep_order=false); // remove element by giving its memory address, if 'keep_order'=false then moves the last element to i-th, if 'keep_order'=true then moves all elements after i-th to the left (keeping order), this method may      change the memory address of some elements

   MemPtr& setNum       (Int num); // set number of elements to 'num'                                                                              , this method may change the memory address of all elements
   MemPtr& setNumZero   (Int num); // set number of elements to 'num', memory of new elements will be first zeroed before calling their constructor, this method may change the memory address of all elements
   MemPtr& setNumDiscard(Int num); // set number of elements to 'num', if reallocating then discard previous elements                              , this method may change the memory address of all elements
   Int     addNum       (Int num); // add 'num' elements, return index of first added element                                                      , this method may change the memory address of all elements

   // values
   T1(VALUE) Int     find   (C VALUE &value                       )C {REPA(T)if(T[i]==value)return i; return -1;                                                 } // check if 'value' is present in container and return its index, -1 if not found
   T1(VALUE) Bool    has    (C VALUE &value                       )C {return find(value)>=0;                                                                     } // check if 'value' is present in container
   T1(VALUE) MemPtr& add    (C VALUE &value                       )  {New()=value; return T;                                                                     } // add      'value' to container                                                                                       , this method changes the memory address of all elements
   T1(VALUE) Bool    include(C VALUE &value                       )  {if(!has(value)){add(value); return true;} return false;                                    } // include  'value' if it's not already present in container, returns true if value wasn't present and has been added  , this method changes the memory address of all elements
   T1(VALUE) Bool    exclude(C VALUE &value, Bool keep_order=false)  {Int i=find(value); if(i>=0){remove(i, keep_order); return true ;}             return false;} // exclude  'value' if present  in container                , returns true if value was    present and has been removed, this method changes the memory address of all elements
   T1(VALUE) Bool    toggle (C VALUE &value, Bool keep_order=false)  {Int i=find(value); if(i>=0){remove(i, keep_order); return false;} add(value); return true ;} // toggle   'value'    presence in container                , returns true if value is now present in container        , this method changes the memory address of all elements

   T1(VALUE)   TYPE*   binaryFind   (C VALUE &value, Int compare(C TYPE &a, C VALUE &b)=Compare)  {return ConstCast(super::binaryFind  (value, compare));                                                                } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE) C TYPE*   binaryFind   (C VALUE &value, Int compare(C TYPE &a, C VALUE &b)=Compare)C {return           super::binaryFind  (value, compare) ;                                                                } // check if 'value' (using binary search) is present in container and return it, null on fail
   T1(VALUE)   MemPtr& binaryAdd    (C VALUE &value, Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i;           super::binarySearch(value, i, compare); NewAt (i)=value;                                return     T;} // add      'value' (using binary search)                                                                                                    , this method changes the memory address of all elements
   T1(VALUE)   Bool    binaryInclude(C VALUE &value, Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(      !super::binarySearch(value, i, compare)){NewAt (i)=value; return true;}                  return false;} // include  'value' (using binary search) if it's not already present in container, returns true if value wasn't present and has been added  , this method changes the memory address of all elements
   T1(VALUE)   Bool    binaryExclude(C VALUE &value, Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(       super::binarySearch(value, i, compare)){remove(i, true); return true;}                  return false;} // exclude  'value' (using binary search) if present  in container                , returns true if value was    present and has been removed, this method changes the memory address of all elements
   T1(VALUE)   Bool    binaryToggle (C VALUE &value, Int compare(C TYPE &a, C VALUE &b)=Compare)  {Int i; if(      !super::binarySearch(value, i, compare)){NewAt (i)=value; return true;} remove(i, true); return false;} // toggle   'value' (using binary search)    presence in container                , returns true if value is now present in container        , this method changes the memory address of all elements

   // order
   MemPtr& sort(           Int compare(C TYPE &a, C TYPE &b           )=Compare); // sort elements with custom comparing function
   MemPtr& sort(CPtr user, Int compare(C TYPE &a, C TYPE &b, CPtr user)        ); // sort elements with custom comparing function and 'user' parameter

   MemPtr& reverseOrder(            ); // reverse order of elements
   MemPtr&    swapOrder(Int i, Int j); // swap order of 'i' and 'j' valid elements

   // misc
                          MemPtr& operator=(C  TYPE                    &src           ); // copy elements using assignment operator
   template<Int src_elms> MemPtr& operator=(C  TYPE                   (&src)[src_elms]); // copy elements using assignment operator
                          MemPtr& operator=(C  Mems  <TYPE           > &src           ); // copy elements using assignment operator
                          MemPtr& operator=(C  Memc  <TYPE           > &src           ); // copy elements using assignment operator
   template<Int src_size> MemPtr& operator=(C  Memt  <TYPE,  src_size> &src           ); // copy elements using assignment operator
                          MemPtr& operator=(C  Memb  <TYPE           > &src           ); // copy elements using assignment operator
                          MemPtr& operator=(C  Memx  <TYPE           > &src           ); // copy elements using assignment operator
                          MemPtr& operator=(C  Meml  <TYPE           > &src           ); // copy elements using assignment operator
   template<Int src_size> MemPtr& operator=(C CMemPtr<TYPE,  src_size> &src           ); // copy elements using assignment operator (this will allow copying from 'CMemPtr' and 'MemPtr' with other sizes)
                          MemPtr& operator=(C  MemPtr<TYPE, Memt_size> &src           ); // copy elements using assignment operator (this must be specified even though method above should do the same, because without it compiler will try to use the built-in 'operator=' which will just do raw memory copy)

#if EE_PRIVATE
   MemPtr& copyFrom(C TYPE *src); // copy raw memory of all elements from 'src'
#endif

   // io
   Bool save(File &f);   Bool save(File &f)C; // save elements with their own 'save' method, this method first saves number of current elements, and then for each element calls its 'save' method, false on fail
   Bool load(File &f);                        // load elements with their own 'load' method, this method first loads number of saved   elements, and then for each element calls its 'load' method, false on fail

   Bool loadRaw(File &f); // load raw memory of elements (number of elements + elements raw memory), false on fail

   // initialize 'MemPtr' to point to source
                          MemPtr& point(       null_t=null                         );
                          MemPtr& point(       TYPE             &src               );
                          MemPtr& point(       TYPE             *src, Int src_elms );
   template<Int src_elms> MemPtr& point(       TYPE            (&src)    [src_elms]);
                          MemPtr& point(Mems  <TYPE           > &src               );
                          MemPtr& point(Mems  <TYPE           > *src               );
                          MemPtr& point(Memc  <TYPE           > &src               );
                          MemPtr& point(Memc  <TYPE           > *src               );
   T1(EXTENDED)           MemPtr& point(Memc  <EXTENDED       > *src               );
                          MemPtr& point(Memt  <TYPE, Memt_size> &src               );
                          MemPtr& point(Memt  <TYPE, Memt_size> *src               );
                          MemPtr& point(Memb  <TYPE           > &src               );
                          MemPtr& point(Memb  <TYPE           > *src               );
   T1(EXTENDED)           MemPtr& point(Memb  <EXTENDED       > *src               );
                          MemPtr& point(Memx  <TYPE           > &src               );
                          MemPtr& point(Memx  <TYPE           > *src               );
   T1(EXTENDED)           MemPtr& point(Memx  <EXTENDED       > *src               );
                          MemPtr& point(Meml  <TYPE           > &src               );
                          MemPtr& point(Meml  <TYPE           > *src               );
                          MemPtr& point(MemPtr<TYPE, Memt_size> &src               );

                          MemPtr(         null_t=null                          ) : CMemPtr<TYPE, Memt_size>(             ) {}
                          MemPtr(         TYPE              &src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
   template<Int src_elms> MemPtr(         TYPE             (&src)    [src_elms]) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(         TYPE              *src, Int src_elms ) : CMemPtr<TYPE, Memt_size>(src, src_elms) {}
                          MemPtr(  Mems  <TYPE           >  &src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(  Mems  <TYPE           >  *src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(  Memc  <TYPE           >  &src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(  Memc  <TYPE           >  *src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
   T1(EXTENDED)           MemPtr(  Memc  <EXTENDED       >  *src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(  Memt  <TYPE, Memt_size>  &src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(  Memt  <TYPE, Memt_size>  *src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(  Memb  <TYPE           >  &src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(  Memb  <TYPE           >  *src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
   T1(EXTENDED)           MemPtr(  Memb  <EXTENDED       >  *src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(  Memx  <TYPE           >  &src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(  Memx  <TYPE           >  *src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
   T1(EXTENDED)           MemPtr(  Memx  <EXTENDED       >  *src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(  Meml  <TYPE           >  &src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(  Meml  <TYPE           >  *src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(  MemPtr<TYPE, Memt_size>  &src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(  MemPtr<TYPE, Memt_size> &&src               ) : CMemPtr<TYPE, Memt_size>(src          ) {}
                          MemPtr(C MemPtr<TYPE, Memt_size>  &src               )=delete; // prevent from pointing to const pointers

   Mems<TYPE           >* mems()  {return (T._mode==T.MEMS) ? ConstCast(T._mems) : null;}
 C Mems<TYPE           >* mems()C {return (T._mode==T.MEMS) ?           T._mems  : null;}
   Memc<TYPE           >* memc()  {return (T._mode==T.MEMC) ? ConstCast(T._memc) : null;}
 C Memc<TYPE           >* memc()C {return (T._mode==T.MEMC) ?           T._memc  : null;}
   Memt<TYPE, Memt_size>* memt()  {return (T._mode==T.MEMT) ? ConstCast(T._memt) : null;}
 C Memt<TYPE, Memt_size>* memt()C {return (T._mode==T.MEMT) ?           T._memt  : null;}
   Memb<TYPE           >* memb()  {return (T._mode==T.MEMB) ? ConstCast(T._memb) : null;}
 C Memb<TYPE           >* memb()C {return (T._mode==T.MEMB) ?           T._memb  : null;}
   Memx<TYPE           >* memx()  {return (T._mode==T.MEMX) ? ConstCast(T._memx) : null;}
 C Memx<TYPE           >* memx()C {return (T._mode==T.MEMX) ?           T._memx  : null;}
   Meml<TYPE           >* meml()  {return (T._mode==T.MEML) ? ConstCast(T._meml) : null;}
 C Meml<TYPE           >* meml()C {return (T._mode==T.MEML) ?           T._meml  : null;}

   // prevent calling constructor through operator=
                          MemPtr& operator=(       null_t              )=delete;
                          MemPtr& operator=(C Mems<TYPE          > *src)=delete;
                          MemPtr& operator=(C Memc<TYPE          > *src)=delete;
   T1(EXTENDED)           MemPtr& operator=(C Memc<EXTENDED      > *src)=delete;
   template<Int src_size> MemPtr& operator=(C Memt<TYPE, src_size> *src)=delete;
                          MemPtr& operator=(C Memb<TYPE          > *src)=delete;
   T1(EXTENDED)           MemPtr& operator=(C Memb<EXTENDED      > *src)=delete;
                          MemPtr& operator=(C Memx<TYPE          > *src)=delete;
   T1(EXTENDED)           MemPtr& operator=(C Memx<EXTENDED      > *src)=delete;
                          MemPtr& operator=(C Meml<TYPE          > *src)=delete;
};
/******************************************************************************/
template<const_mem_addr typename TYPE, Int Memt_elms> struct MemPtrN : MemPtr<TYPE, SIZE(TYPE)*Memt_elms> // Memory Container Pointer, 'Memt_elms'=number of elements of the 'Memt'
{
   // copy elements using assignment operator
                          MemPtrN& operator=(C  TYPE                     &src           ); // copy elements using assignment operator
   template<Int src_elms> MemPtrN& operator=(C  TYPE                    (&src)[src_elms]); // copy elements using assignment operator
                          MemPtrN& operator=(C  Mems   <TYPE           > &src           ); // copy elements using assignment operator
                          MemPtrN& operator=(C  Memc   <TYPE           > &src           ); // copy elements using assignment operator
   template<Int src_size> MemPtrN& operator=(C  Memt   <TYPE,  src_size> &src           ); // copy elements using assignment operator
                          MemPtrN& operator=(C  Memb   <TYPE           > &src           ); // copy elements using assignment operator
                          MemPtrN& operator=(C  Memx   <TYPE           > &src           ); // copy elements using assignment operator
                          MemPtrN& operator=(C  Meml   <TYPE           > &src           ); // copy elements using assignment operator
   template<Int src_size> MemPtrN& operator=(C CMemPtr <TYPE,  src_size> &src           ); // copy elements using assignment operator (this will allow copying from 'MemPtr' with other sizes)
                          MemPtrN& operator=(C  MemPtrN<TYPE, Memt_elms> &src           ); // copy elements using assignment operator (this must be specified even though method above should do the same, because without it compiler will try to use the built-in 'operator=' which will just do raw memory copy)

   // initialize 'MemPtrN' to point to source
                          MemPtrN(        null_t=null                                    ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(             ) {}
                          MemPtrN(        TYPE                        &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
   template<Int src_elms> MemPtrN(        TYPE                       (&src)    [src_elms]) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(        TYPE                        *src, Int src_elms ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src, src_elms) {}
                          MemPtrN(Mems   <TYPE                      > &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Mems   <TYPE                      > *src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Memc   <TYPE                      > &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Memc   <TYPE                      > *src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
   T1(EXTENDED)           MemPtrN(Memc   <EXTENDED                  > *src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Memt   <TYPE, SIZE(TYPE)*Memt_elms> &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Memb   <TYPE                      > &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Memb   <TYPE                      > *src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
   T1(EXTENDED)           MemPtrN(Memb   <EXTENDED                  > *src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Memx   <TYPE                      > &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Memx   <TYPE                      > *src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
   T1(EXTENDED)           MemPtrN(Memx   <EXTENDED                  > *src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Meml   <TYPE                      > &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(Meml   <TYPE                      > *src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(MemPtr <TYPE, SIZE(TYPE)*Memt_elms> &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          MemPtrN(MemPtrN<TYPE,            Memt_elms> &src               ) : MemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}

   // prevent calling constructor through operator=
                          MemPtrN& operator=(       null_t                          )=delete;
                          MemPtrN& operator=(C Mems<TYPE                      > *src)=delete;
                          MemPtrN& operator=(C Memc<TYPE                      > *src)=delete;
   T1(EXTENDED)           MemPtrN& operator=(C Memc<EXTENDED                  > *src)=delete;
   template<Int src_elms> MemPtrN& operator=(C Memt<TYPE, SIZE(TYPE)*Memt_elms> *src)=delete;
                          MemPtrN& operator=(C Memb<TYPE                      > *src)=delete;
   T1(EXTENDED)           MemPtrN& operator=(C Memb<EXTENDED                  > *src)=delete;
                          MemPtrN& operator=(C Memx<TYPE                      > *src)=delete;
   T1(EXTENDED)           MemPtrN& operator=(C Memx<EXTENDED                  > *src)=delete;
                          MemPtrN& operator=(C Meml<TYPE                      > *src)=delete;
};
/******************************************************************************/
template<const_mem_addr typename TYPE, Int Memt_elms> struct CMemPtrN : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms> // Const Memory Container Pointer, 'Memt_elms'=number of elements of the 'Memt'
{
   // initialize 'CMemPtrN' to point to source
                          CMemPtrN(          null_t=null                                    ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(             ) {}
                          CMemPtrN(C         TYPE                        &src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
   template<Int src_elms> CMemPtrN(C         TYPE                       (&src)    [src_elms]) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          CMemPtrN(C         TYPE                        *src, Int src_elms ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src, src_elms) {}
                          CMemPtrN(C  Mems  <TYPE                      > &src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          CMemPtrN(C  Mems  <TYPE                      > *src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          CMemPtrN(C  Memc  <TYPE                      > &src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          CMemPtrN(C  Memc  <TYPE                      > *src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
   T1(EXTENDED)           CMemPtrN(C  Memc  <EXTENDED                  > *src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          CMemPtrN(C  Memt  <TYPE, SIZE(TYPE)*Memt_elms> &src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          CMemPtrN(C  Memb  <TYPE                      > &src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          CMemPtrN(C  Memb  <TYPE                      > *src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
   T1(EXTENDED)           CMemPtrN(C  Memb  <EXTENDED                  > *src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          CMemPtrN(C  Memx  <TYPE                      > &src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          CMemPtrN(C  Memx  <TYPE                      > *src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
   T1(EXTENDED)           CMemPtrN(C  Memx  <EXTENDED                  > *src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          CMemPtrN(C  Meml  <TYPE                      > &src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          CMemPtrN(C  Meml  <TYPE                      > *src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
                          CMemPtrN(C CMemPtr<TYPE, SIZE(TYPE)*Memt_elms> &src               ) : CMemPtr<TYPE, SIZE(TYPE)*Memt_elms>(src          ) {}
};
/******************************************************************************/
template<typename TYPE, Int size>   inline Int Elms(C CMemPtr<TYPE, size> &memp) {return memp.elms();}
/******************************************************************************/
