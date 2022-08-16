/******************************************************************************/
struct _Map // Map (base) - Do not use this class, use 'Map' instead
{
   struct Desc
   {
      UInt flag;
   };
   struct Elm
   {
   };

   Int elms    ()C {return _elms;}
   Int dataSize()C {return _data_size;} // get size of DATA element

   CPtr key       (Int i)C;
    Ptr operator[](Int i) ;
   CPtr operator[](Int i)C {return ConstCast(T)[i];}

 ~_Map() {del();}

#if !EE_PRIVATE
private:
#endif
   Byte       _mode;
   Int        _elms, _key_offset, /*_data_offset, */_desc_offset, _data_size;
   Elm      **_order;
   Memx<Elm>  _memx;
   Ptr        _user;
   Int      (*_compare )(CPtr key_a, CPtr key_b);
   Bool     (*_create  )( Ptr elm  , CPtr key  , Ptr user);
   void     (*_copy_key)( Ptr dest , CPtr src  );

#if EE_PRIVATE
   Desc& elmDesc(  Elm &elm )C {return *(Desc*)((Byte*)&elm+_desc_offset);}
 C Desc& elmDesc(C Elm &elm )C {return *(Desc*)((Byte*)&elm+_desc_offset);}
   Ptr   elmKey (  Elm &elm )C {return          (Byte*)&elm+ _key_offset ;}
  CPtr   elmKey (C Elm &elm )C {return          (Byte*)&elm+ _key_offset ;}
   Ptr   elmData(  Elm &elm )C {return          (Byte*)&elm/*+_data_offset*/ ;} // assumes that '_data_offset' is zero
  CPtr   elmData(C Elm &elm )C {return          (Byte*)&elm/*+_data_offset*/ ;} // assumes that '_data_offset' is zero
   Elm*  dataElm( CPtr  data)C {return  (Elm *)((Byte*)data/*-_data_offset*/);} // assumes that '_data_offset' is zero

   Elm*         findElm(CPtr  key, Int &stop )C;
   void      addToOrder( Elm &elm, Int  index);
   void removeFromOrder(           Int  index);
   void       getFailed()C;
   Bool     containsElm(C Elm *elm)C {return _memx.contains(elm);}
   Int  dataInMapToAbsIndex(CPtr data)C;

private:
#endif
   Byte mode(Byte mode);

   CPtr dataInMapToKeyRef(CPtr data)C {return (Byte*)data /*-_data_offset*/ + _key_offset;} // assumes that '_data_offset' is zero
   CPtr dataInMapToKeyPtr(CPtr data)C {return data ? dataInMapToKeyRef(data) : null;}

   CPtr absKey (Int abs_i)C;
   CPtr absData(Int abs_i)C;

   void from(C _Map &src);

   void clear();
   void del  ();

   Ptr find   (CPtr key)C;
   Ptr get    (CPtr key);
   Ptr require(CPtr key);

   Ptr get    (CPtr key, Bool &just_created);
   Ptr require(CPtr key, Bool &just_created);

   Int    findValidIndex(CPtr key)C;
 //Int     getValidIndex(CPtr key);
 //Int requireValidIndex(CPtr key);

   Int    findAbsIndex(CPtr key)C;
   Int     getAbsIndex(CPtr key);
   Int requireAbsIndex(CPtr key);

   Bool dummy(CPtr data            )C;
   void dummy(CPtr data, Bool dummy);

   Bool containsKey (CPtr key )C;
   Bool containsData(CPtr data)C;
   CPtr dataToKey   (CPtr data)C;
   Int  dataToIndex (CPtr data)C;
   
   void remove    (Int  i   );
   void removeKey (CPtr key );
   void removeData(CPtr data);
   Bool replaceKey(CPtr src, CPtr dest);

   void reserve(Int num);

   void compare(Int compare(CPtr key_a, CPtr key_b));

   explicit _Map(Int block_elms, Int compare(CPtr key_a, CPtr key_b), Bool create(Ptr data, CPtr key, Ptr user), Ptr user, void (&copy_key)(Ptr dest, CPtr src));

   NO_COPY_CONSTRUCTOR(_Map);

                friend struct _MapTS;
                friend struct _MapEx;
   T2(KEY,DATA) friend struct  Map;
   T2(KEY,DATA) friend struct  MapEx;
   T2(KEY,DATA) friend struct  ThreadSafeMap;
   template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP> friend struct MapElmPtr;
};
/******************************************************************************/
struct _MapEx : private _Map // Map Extended (base) - Do not use this class, use 'MapEx' instead
{
   struct DescPtrNum : Desc
   {
      UInt ptr_num=0;
   };

   Int elms()C {return super::elms();}

private:
   struct DelayRemove
   {
      Flt  time;
      Elm *elm;
   };
   Flt _delay_remove_time;
   Dbl _delay_remove_check;
   Memc<DelayRemove> _delay_remove;

   void delayRemove   (Flt time);
   void delayRemoveNow();
   void update();

#if EE_PRIVATE
   DescPtrNum& elmDesc(  Elm &elm)C {return *(DescPtrNum*)((Byte*)&elm+_desc_offset);}
 C DescPtrNum& elmDesc(C Elm &elm)C {return *(DescPtrNum*)((Byte*)&elm+_desc_offset);}

   void processDelayRemove(Bool always);
   Int  findDelayRemove(Elm &elm);
#endif

   Ptr find   (CPtr key, Bool counted);
   Ptr get    (CPtr key, Bool counted);
   Ptr require(CPtr key, Bool counted);

   void incRef(CPtr data);
   void decRef(CPtr data);

   explicit _MapEx(Int block_elms, Int compare(CPtr key_a, CPtr key_b), Bool create(Ptr data, CPtr key, Ptr user), Ptr user, void (&copy_key)(Ptr dest, CPtr src));

   T2(KEY, DATA)                                               friend struct MapEx;
   template<typename KEY, typename DATA, MapEx<KEY,DATA> &MAP> friend struct MapElmPtr;
};
/******************************************************************************/
struct _MapTS : private _Map // Map Thread Safe (base) - Do not use this class, use 'ThreadSafeMap' instead
{
   Int elms()C {return super::elms();}

   void   lock()C;
   void unlock()C;

 ~_MapTS() {del();}

private:
   SyncLock _lock;

   void clear();
   void del  ();

   Ptr find   (CPtr key)C;
   Ptr get    (CPtr key);
   Ptr require(CPtr key);

   Int    findValidIndex(CPtr key)C;
 //Int     getValidIndex(CPtr key);
 //Int requireValidIndex(CPtr key);

   Int    findAbsIndex(CPtr key)C;
   Int     getAbsIndex(CPtr key);
   Int requireAbsIndex(CPtr key);

   Bool containsKey (CPtr key )C;
   Bool containsData(CPtr data)C;
   CPtr dataToKey   (CPtr data)C;
   Int  dataToIndex (CPtr data)C;

   void lockedRemove    (Int  i   ) {super::remove(i);}
   void       remove    (Int  i   );
   void       removeKey (CPtr key );
   void       removeData(CPtr data);
   Bool       replaceKey(CPtr src, CPtr dest);

   void reserve(Int num);

   void compare(Int compare(CPtr key_a, CPtr key_b));

   explicit _MapTS(Int block_elms, Int compare(CPtr key_a, CPtr key_b), Bool create(Ptr data, CPtr key, Ptr user), Ptr user, void (&copy_key)(Ptr dest, CPtr src));

   T2(KEY,DATA) friend struct ThreadSafeMap;
};
/******************************************************************************/
