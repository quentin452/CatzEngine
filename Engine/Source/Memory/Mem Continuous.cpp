/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void _Memc::_reset(Int elm_size, void (*_new)(Ptr elm), void (*_del)(Ptr elm))
{
   T.~_Memc();
   new(this)_Memc(elm_size, _new, _del);
}
_Memc::_Memc(Int elm_size, void (*_new)(Ptr elm), void (*_del)(Ptr elm))
{
   T._elms    =0;
   T._elm_size=elm_size;
   T._max_elms=0;
   T._data    =null;
   T._new     =_new;
   T._del     =_del;
}
/******************************************************************************/
void _Memc::del()
{
   clear();
   Free (_data);
  _max_elms=0;
}
void _Memc::clear()
{
   if(_del)REPA(T)_del(T[i]); // destroy as the first step
  _elms=0;
}
/******************************************************************************/
inline void _Memc::_maxElms(UInt max_elms)
{
   if(!initialized())Exit("Attempting to create an object of zero size in 'Memc' container.\nThe container is not initialized or it is abstract and 'replaceClass' hasn't been called.");
  _max_elms=CeilPow2(max_elms);
}
void _Memc::reserve(Int num)
{
   if(Greater(num, maxElms())) // num>maxElms()
   {
     _maxElms(num);
     _Realloc(_data, requiredMem(), elmsMem());
   }
}
void _Memc::reserveAdd(Int num)
{
   Long new_elms=Long(elms())+num; if(new_elms>0)
   {
      if(new_elms>INT_MAX)Exit("'Memc.reserveAdd' size too big");
      reserve(new_elms);
   }
}
void _Memc::setNum(Int num)
{
   MAX(num, 0);
   if (num>elms()) // add elements
   {
      reserve(num);
      Int old_elms=elms(); _elms=num; // set '_elms' before accessing new elements to avoid range assert
      if(_new)for(Int i=old_elms; i<elms(); i++)_new(T[i]); // create as the last step
   }else
   if(num<elms()) // remove elements
   {
      if(_del)for(Int i=elms(); --i>=num; )_del(T[i]); // destroy as the first step
     _elms=num; // set '_elms' after accessing new elements to avoid range assert
   }
}
void _Memc::setNumZero(Int num)
{
   MAX(num, 0);
   if (num>elms()) // add elements
   {
      reserve(num);
      Int old_elms=elms(); _elms=num; // set '_elms' before accessing new elements to avoid range assert
      ZeroFast(T[old_elms], elmSize()*UIntPtr(elms()-old_elms));
      if(_new)for(Int i=old_elms; i<elms(); i++)_new(T[i]); // create as the last step
   }else
   if(num<elms()) // remove elements
   {
      if(_del)for(Int i=elms(); --i>=num; )_del(T[i]); // destroy as the first step
     _elms=num; // set '_elms' after accessing new elements to avoid range assert
   }
}
void _Memc::setNum(Int num, Int keep)
{
   MAX(num, 0);
   Clamp(keep, 0, Min(elms(), num));
   if(_del)for(Int i=elms(); --i>=keep; )_del(T[i]); // delete unkept elements, destroy as the first step
   if(Greater(num, maxElms())) // resize memory, num>maxElms()
   {
     _elms=keep; // set '_elms' before 'reserve' to copy only 'keep' elements
      reserve(num);
   }
  _elms=num; // set '_elms' before accessing new elements to avoid range assert
   if(_new)for(Int i=keep; i<elms(); i++)_new(T[i]); // create new elements, create as the last step
}
void _Memc::setNumZero(Int num, Int keep)
{
   MAX(num, 0);
   Clamp(keep, 0, Min(elms(), num));
   if(_del)for(Int i=elms(); --i>=keep; )_del(T[i]); // delete unkept elements, destroy as the first step
   if(Greater(num, maxElms())) // resize memory, num>maxElms()
   {
     _elms=keep; // set '_elms' before 'reserve' to copy only 'keep' elements
      reserve(num);
   }
  _elms=num; // set '_elms' before accessing new elements to avoid range assert
   ZeroFast(_element(keep), elmSize()*UIntPtr(elms()-keep)); // zero new elements, have to use '_element' to avoid out of range errors
   if(_new)for(Int i=keep; i<elms(); i++)_new(T[i]); // create new elements, create as the last step
}
void _Memc::setNumDiscard(Int num)
{
   MAX(num, 0);
   if( num!=elms())
   {
      if(Greater(num, maxElms())) // resize memory, num>maxElms()
      {
         clear(); // clear before 'reserve' to skip copying old elements
         reserve(num);
        _elms=num; // set '_elms' before accessing new elements to avoid range assert
         if(_new)FREPA(T)_new(T[i]); // create new elements, create as the last step
      }else
      if(num>elms()) // add elements in existing memory
      {
         Int old_elms=elms(); _elms=num; // set '_elms' before accessing new elements to avoid range assert
         if(_new)for(Int i=old_elms; i<elms(); i++)_new(T[i]); // create as the last step
      }else
    //if(num<elms()) // remove elements, "if" not needed because we already know that "num!=elms && !(num>elms())"
      {
         if(_del)for(Int i=elms(); --i>=num; )_del(T[i]); // destroy as the first step
        _elms=num; // set '_elms' after accessing new elements to avoid range assert
      }
   }
}
/******************************************************************************/
Int _Memc::addNum    (Int num) {Int index=elms(); Long new_elms=Long(index)+num; if(new_elms>INT_MAX)Exit("'Memc.addNum' size too big"    ); setNum    (new_elms); return index;}
Int _Memc::addNumZero(Int num) {Int index=elms(); Long new_elms=Long(index)+num; if(new_elms>INT_MAX)Exit("'Memc.addNumZero' size too big"); setNumZero(new_elms); return index;}
/******************************************************************************/
Ptr _Memc::NewAt(Int i)
{
   Clamp(i, 0, elms());
   Int old_elms=elms(); if(old_elms>=INT_MAX)Exit("'Memc.NewAt' size too big"); _elms++; // increase '_elms' before accessing new elements to avoid range assert
   if(Greater(elms(), maxElms())) // elms()>maxElms()
   {
     _maxElms(elms());
      Ptr temp=Alloc(requiredMem()); // copy everything to a new buffer
      CopyFast((Byte*)temp                       , data(), UIntPtr(         i)*elmSize());
      CopyFast((Byte*)temp+UIntPtr(i+1)*elmSize(), T[i]  , UIntPtr(old_elms-i)*elmSize());
      Free(_data); _data=temp;
   }else
   if(i<old_elms)
   {
      MoveFast(T[i+1], T[i], UIntPtr(old_elms-i)*elmSize());
   }
   Ptr elm=T[i]; if(_new)_new(elm); return elm; // create as the last step
}
void _Memc::removeLast()
{
   if(elms())
   {
      if(_del)_del(T[elms()-1]); // destroy as the first step
     _elms--;
   }
}
void _Memc::remove(Int i, Bool keep_order)
{
   if(InRange(i, T))
   {
      if(_del)_del(T[i]); // destroy as the first step
      if(i<elms()-1)
      {
         if(keep_order)MoveFast(T[i], T[     i+1], elmSize()*UIntPtr(elms()-1-i));
         else          CopyFast(T[i], T[elms()-1], elmSize());
      }
     _elms--;
   }
}
void _Memc::removeNum(Int i, Int n, Bool keep_order)
{
   if(i<0){n+=i; i=0;} // if 'i' is before the start, then move it to start and reduce number of elements to remove
   if(n>0 && InRange(i, T)) // if we want to remove elements and the index fits
   {
      MIN(n, elms()-i); // minimize what we can actually remove
      if(_del)REPD(j, n)_del(T[i+j]); // delete those elements, destroy as the first step
      if(i<elms()-n) // if there are any elements after those being removed
      {
                      if(keep_order)MoveFast(T[i], T[     i+n], elmSize()*UIntPtr(elms()-n-i));else // move all    elements after i(+n) to left
         {Int m=Min(n, elms()-i-n); CopyFast(T[i], T[elms()-m], elmSize()*UIntPtr(       m  ));}    // move last m elements to i-th
      }
     _elms-=n;
   }
}
void _Memc::removeData(CPtr elm, Bool keep_order)
{
   remove(index(elm), keep_order);
}
/******************************************************************************/
Ptr _Memc::operator()(Int i)
{
   if(i< 0     )Exit("i<0 inside '_Memc.operator()(Int i)'");
   if(i>=elms())setNumZero(i+1);
   return T[i];
}
/******************************************************************************/
Int _Memc::index(CPtr elm)C
{
   UIntPtr i=UIntPtr(elm)-UIntPtr(data());
   if(i<elmsMem())return i/elmSize(); // unsigned compare will already guarantee "i>=0 && "
   return -1;
}
/******************************************************************************/
Bool _Memc::binarySearch(CPtr value, Int &index, Int compare(CPtr a, CPtr b))C {return _BinarySearch(data(), elms(), elmSize(), value, index, compare);}

void _Memc::sort(           Int compare(CPtr a, CPtr b           )) {_Sort(data(), elms(), elmSize(),       compare);}
void _Memc::sort(CPtr user, Int compare(CPtr a, CPtr b, CPtr user)) {_Sort(data(), elms(), elmSize(), user, compare);}

void _Memc::  reverseOrder(                      ) {  _ReverseOrder(data(), elms(), elmSize()                );}
void _Memc::randomizeOrder(                      ) {_RandomizeOrder(data(), elms(), elmSize()                );}
void _Memc::   rotateOrder(Int offset            ) {   _RotateOrder(data(), elms(), elmSize(), offset        );}
void _Memc::     moveElm  (Int elm, Int new_index) {     _MoveElm  (data(), elms(), elmSize(), elm, new_index);}
void _Memc::     swapOrder(Int i  , Int j        ) {if(InRange(i, T) && InRange(j, T))Swap(T[i], T[j], elmSize());}

void _Memc::moveElmLeftUnsafe(Int elm, Int new_index, Ptr temp) {_MoveElmLeftUnsafe(data(), elmSize(), elm, new_index, temp);}
/******************************************************************************/
void _Memc::copyTo  ( Ptr dest)C {if(dest)CopyFast(dest  , data(), elmsMem());}
void _Memc::copyFrom(CPtr src )  {        Copy    (data(), src   , elmsMem());} // use 'Copy' in case 'src' is null
/******************************************************************************/
Bool _Memc::saveRaw(File &f)C
{
   f.cmpUIntV(elms());
   f.put     (data(), elmsMem());
   return f.ok();
}
Bool _Memc::loadRaw(File &f)
{
   setNum(f.decUIntV());
   f.getFast(data(), elmsMem());
   if(f.ok())return  true;
   clear();  return false;
}
Bool _Memc::_saveRaw(File &f)C
{
   f.putInt(elms());
   f.put   (data(), elmsMem());
   return f.ok();
}
Bool _Memc::_loadRaw(File &f)
{
   setNum(f.getInt());
   f.getFast(data(), elmsMem());
   if(f.ok())return  true;
   clear();  return false;
}
/******************************************************************************/
// MEMC THREAD SAFE
/******************************************************************************/
_MemcThreadSafe::_MemcThreadSafe(Int elm_size, void (*_new)(Ptr elm), void (*_del)(Ptr elm)) : _memc(elm_size, _new, _del) {}

void _MemcThreadSafe::clear() {SyncLocker locker(_lock); _memc.clear();}
void _MemcThreadSafe::del  () {SyncLocker locker(_lock); _memc.del  ();}

Bool _MemcThreadSafe::contains(CPtr elm)C {SyncLocker locker(_lock); return _memc.contains(elm);}

void _MemcThreadSafe::removeData(CPtr elm, Bool keep_order) {SyncLocker locker(_lock); _memc.removeData(elm, keep_order);}

void _MemcThreadSafe::setNum    (Int num) {SyncLocker locker(_lock);        _memc.setNum    (num);}
void _MemcThreadSafe::setNumZero(Int num) {SyncLocker locker(_lock);        _memc.setNumZero(num);}
Int  _MemcThreadSafe::addNum    (Int num) {SyncLocker locker(_lock); return _memc.addNum    (num);}

void _MemcThreadSafe::sort(           Int compare(CPtr a, CPtr b           )) {SyncLocker locker(_lock); _memc.sort(      compare);}
void _MemcThreadSafe::sort(CPtr user, Int compare(CPtr a, CPtr b, CPtr user)) {SyncLocker locker(_lock); _memc.sort(user, compare);}

void _MemcThreadSafe::  reverseOrder(          ) {SyncLocker locker(_lock); _memc.  reverseOrder(      );}
void _MemcThreadSafe::randomizeOrder(          ) {SyncLocker locker(_lock); _memc.randomizeOrder(      );}
void _MemcThreadSafe::   rotateOrder(Int offset) {SyncLocker locker(_lock); _memc.   rotateOrder(offset);}

Bool _MemcThreadSafe::saveRaw(File &f)C {SyncLocker locker(_lock); return _memc.saveRaw(f);}
Bool _MemcThreadSafe::loadRaw(File &f)  {SyncLocker locker(_lock); return _memc.loadRaw(f);}
/******************************************************************************/
}
/******************************************************************************/
