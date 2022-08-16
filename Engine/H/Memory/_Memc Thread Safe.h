/******************************************************************************/
struct _MemcThreadSafe // Thread-Safe Continuous Memory Based Container Base - Do not use this class, use 'MemcThreadSafe' instead
{
   void lockedClear() {_memc.clear();}
   void lockedDel  () {_memc.del  ();}
   void       clear();
   void       del  ();

   // get / set
   Int     elms    ()C {return _memc.elms    ();}
   UInt    elmSize ()C {return _memc.elmSize ();}
   UIntPtr memUsage()C {return _memc.memUsage();}
   UIntPtr elmsMem ()C {return _memc.elmsMem ();}

   Ptr lockedData (     )C {return _memc.data ( );}
   Ptr lockedAddr (Int i)C {return _memc.addr (i);}
   Ptr lockedElm  (Int i)C {return _memc      [i];}
   Ptr lockedFirst(     )C {return _memc.first( );}
   Ptr lockedLast (     )C {return _memc.last ( );}
   Ptr lockedNew  (     )  {return _memc.New  ( );}
   Ptr lockedNewAt(Int i)  {return _memc.NewAt(i);}

   Int  lockedIndex   (CPtr elm)C {return _memc.index   (elm);}
   Bool lockedContains(CPtr elm)C {return _memc.contains(elm);}
   Bool       contains(CPtr elm)C;

   void lockedRemoveLast(                               ) {_memc.removeLast();}
   void lockedRemove    (Int  i  , Bool keep_order=false) {_memc.remove    (i  , keep_order);}
   void lockedRemoveData(CPtr elm, Bool keep_order=false) {_memc.removeData(elm, keep_order);}
   void       removeData(CPtr elm, Bool keep_order=false);

   void setNum    (Int num);
   void setNumZero(Int num);
   Int  addNum    (Int num);

   Bool lockedBinarySearch(CPtr value, Int &index, Int compare(CPtr a, CPtr b))C {return _memc.binarySearch(value, index, compare);}

   void            sort(           Int compare(CPtr a, CPtr b           ));
   void            sort(CPtr user, Int compare(CPtr a, CPtr b, CPtr user));
   void    reverseOrder();
   void  randomizeOrder();
   void     rotateOrder(Int offset);
   void lockedSwapOrder(Int i  , Int j        ) {_memc.swapOrder(i  , j        );}
   void lockedMoveElm  (Int elm, Int new_index) {_memc.moveElm  (elm, new_index);}

   // operations
   void   lock()C {_lock.on ();}
   void unlock()C {_lock.off();}

   // io
   Bool saveRaw(File &f)C;
   Bool loadRaw(File &f) ;

private:
  _Memc     _memc;
   SyncLock _lock;

   explicit _MemcThreadSafe(Int elm_size, void (*_new)(Ptr elm), void (*_del)(Ptr elm));

   T1(TYPE) friend struct MemcThreadSafe;
};
/******************************************************************************/
