/******************************************************************************/
struct _Cache // Cache (base) - Do not use this class, use 'Cache' instead
{
   struct Desc
   {
      Str  file; // file name
      UInt flag, ptr_num;
   };
   struct Elm
   {
   };

   Int elms()C {return _elms;}

   void   lock()C;
   void unlock()C;

 ~_Cache() {del();}

private:
   struct DelayRemove
   {
      Flt  time;
      Elm *elm;
   };
mutable Byte         _d_lock;
   Bool              _case_sensitive;
   Byte              _mode;
   Int               _elms, /*_data_offset, */_desc_offset, _delay_remove_counter;
   Flt               _delay_remove_time;
   Dbl               _delay_remove_check;
   CChar8           *_debug_name;
   Elm             **_order;
   Memx<Elm>         _memx;
   Memc<DelayRemove> _delay_remove;
   SyncLock          _lock;
   Ptr               _user;
   Bool            (*_load          )( Ptr data, C Str &file);
   Bool            (*_load_user     )( Ptr data, C Str &file, Ptr user);
   Bool            (*_can_be_removed)(CPtr data);

   void clear         ();
   void del           ();
   Byte mode          (Byte mode);
   void caseSensitive (Bool sensitive);
   void delayRemove   (Flt  time);
   void delayRemoveNow();
   void delayRemoveInc();
   void delayRemoveDec();
   void reserve       (Int num);
   void update        ();

   void setLoadUser(Bool (*load_user)(Ptr data, C Str &file, Ptr user), Ptr user);

#if EE_PRIVATE
   void processDelayRemove(Bool always);

   Desc& elmDesc(  Elm &elm )  {return *(Desc*)((Byte*)&elm+_desc_offset);}
 C Desc& elmDesc(C Elm &elm )C {return *(Desc*)((Byte*)&elm+_desc_offset);}
   Ptr   elmData(  Elm &elm )  {return          (Byte*)&elm/*+_data_offset*/ ;} // assumes that '_data_offset' is zero
  CPtr   elmData(C Elm &elm )C {return          (Byte*)&elm/*+_data_offset*/ ;} // assumes that '_data_offset' is zero
   Elm*  dataElm( CPtr  data)C {return  (Elm *)((Byte*)data/*-_data_offset*/);} // assumes that '_data_offset' is zero

   Elm*        findExact(CChar *file, Int    &stop);
   Elm*        findElm  (CChar *file, CChar  *path);
   Int   findDelayRemove(  Elm &elm);
   Ptr      validElmData(  Elm &elm , Bool counted);
   void       addToOrder(  Elm &elm);
   void  removeFromOrder(  Elm &elm);
   Bool   lockedContains(C Elm *elm )C {return _memx.  contains(        elm  );} // this is NOT thread-safe
   Int          absIndex(C Elm *elm )C {return _memx.  absIndex(        elm  );} // this is NOT thread-safe
   Int        validIndex(C Elm *elm )C {return _memx.validIndex(        elm  );} // this is NOT thread-safe
   Int          absIndex( CPtr  data)C {return         absIndex(dataElm(data));} // this is NOT thread-safe, assumes that '_data_offset' is zero
   Int        validIndex( CPtr  data)C {return       validIndex(dataElm(data));} // this is NOT thread-safe, assumes that '_data_offset' is zero
   Bool              has( CPtr  data)C;
   void lockedIncRef    ( CPtr  data);
   void lockedRemoveData( CPtr  data, Bool counted);
   void       removeData( CPtr  data, Bool counted);
   void  incRefContained( CPtr  data);
#endif
   Ptr    find      (CChar *file, CChar *path, Bool counted);
   Ptr    find      (C UID &id  , CChar *path, Bool counted);
   Ptr    get       (CChar *file, CChar *path, Bool counted);
   Ptr    get       (C UID &id  , CChar *path, Bool counted);
   Ptr    require   (CChar *file, CChar *path, Bool counted);
   Ptr    require   (C UID &id  , CChar *path, Bool counted);
   Bool   contains  (CPtr   data                           )C;
   Int    ptrCount  (CPtr   data                           )C;
   Bool   dummy     (CPtr   data                           )C;
   void   dummy     (CPtr   data, Bool   dummy             );
   C Str& name      (CPtr   data                           )C;
   CChar* name      (CPtr   data, CChar *path              )C;
   UID    id        (CPtr   data                           )C;
   void   removeData(CPtr   data                           );

   void incDecRef(CPtr inc, CPtr dec);
   void incRef(CPtr data);
   void decRef(CPtr data);

   void lockedFrom(C _Cache &src);

 C Desc& lockedDesc(Int i)C;
   CPtr  lockedData(Int i)C;

   explicit _Cache(CChar8 *name, Int block_elms);

   NO_COPY_CONSTRUCTOR(_Cache);

   T1(TYPE)                                    friend struct Cache;
   template<typename TYPE, Cache<TYPE> &CACHE> friend struct CacheElmPtr;
};
/******************************************************************************/
#if EE_PRIVATE
inline void IncPtrNum(UInt &ptr_num)
{
   DEBUG_ASSERT(ptr_num<UINT_MAX, "'ptr_num' too big");
   ptr_num++;
}
#define DELAY_REMOVE_STEP (1.0f/8) // number of steps per 'delay_remove' time to check for element removal
#endif
/******************************************************************************/
