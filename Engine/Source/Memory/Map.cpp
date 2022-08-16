/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
typedef Map<Int, Int> MapInt; ASSERT(OFFSET(MapInt::Elm, data)==0); // '_data_offset' is not used because it's assumed to be always 0
/******************************************************************************/
_Map::_Map(Int block_elms, Int compare(CPtr key_a, CPtr key_b), Bool create(Ptr data, CPtr key, Ptr user), Ptr user, void (&copy_key)(Ptr dest, CPtr src)) : _memx(block_elms)
{
  _elms=0;
  _key_offset=/*_data_offset=*/_desc_offset=_data_size=0;
  _mode=MAP_EXIT;
  _order=null;
  _user    =user;
  _compare =compare;
  _create  =create;
  _copy_key=copy_key;
}
_MapEx::_MapEx(Int block_elms, Int compare(CPtr key_a, CPtr key_b), Bool create(Ptr data, CPtr key, Ptr user), Ptr user, void (&copy_key)(Ptr dest, CPtr src)) : _Map(block_elms, compare, create, user, copy_key)
{
  _delay_remove_time=0; _delay_remove_check=0;
}
_MapTS::_MapTS(Int block_elms, Int compare(CPtr key_a, CPtr key_b), Bool create(Ptr data, CPtr key, Ptr user), Ptr user, void (&copy_key)(Ptr dest, CPtr src)) : _Map(block_elms, compare, create, user, copy_key)
{
}
void _Map::clear() {_memx.clear(); _elms=0;              } // here can't Free '_order' because it assumes that its size is '_memx.maxElms()'
void _Map::del  () {_memx.del  (); _elms=0; Free(_order);}

void _MapTS::clear() {SyncLocker locker(_lock); super::clear();}
void _MapTS::del  () {SyncLocker locker(_lock); super::del  ();}

Byte _Map::mode(Byte mode) {auto old_mode=T._mode; T._mode=mode; return old_mode;}

CPtr _Map::key       (Int i)C {return elmKey (*_order[i]);}
 Ptr _Map::operator[](Int i)  {return elmData(*_order[i]);}

CPtr _Map::absKey (Int abs_i)C {return elmKey (_memx.absElm(abs_i));}
CPtr _Map::absData(Int abs_i)C {return elmData(_memx.absElm(abs_i));}

INLINE Int _Map::dataInMapToAbsIndex(CPtr data)C {return data ? _memx.absIndexFastUnsafeValid(dataElm(data)) : -1;}
/******************************************************************************/
void _MapTS::  lock()C {_lock.on ();}
void _MapTS::unlock()C {_lock.off();}
/******************************************************************************/
_Map::Elm* _Map::findElm(CPtr key, Int &stop)C
{
   Int l=0, r=_elms; for(; l<r; )
   {
      Int mid    =UInt(l+r)/2,
          compare=T._compare(key, elmKey(*_order[mid]));
      if(!compare)
      {
         stop=mid;
         return _order[mid];
      }
      if(compare<0)r=mid;
      else         l=mid+1;
   }

   stop=l;
   return null;
}
/******************************************************************************/
void _Map::addToOrder(Elm &elm, Int index)
{
   MoveFastN(_order+index+1, _order+index, _elms-index); // faster version of: for(Int i=_elms; i>index; i--)_order[i]=_order[i-1];
  _order[index]=&elm; _elms++;
}
void _Map::removeFromOrder(Int index)
{
  _elms--;
   MoveFastN(_order+index, _order+index+1, _elms-index); // faster version of: for(Int i=index; i<_elms; i++)_order[i]=_order[i+1];
}
/******************************************************************************/
Int _Map::findAbsIndex(CPtr key)C {return dataInMapToAbsIndex(find(key));}
Ptr _Map::find        (CPtr key)C
{
   Int i; if(Elm *elm=findElm(key, i))if(!(elmDesc(*elm).flag&CACHE_ELM_LOADING))return elmData(*elm);
   return null;
}
Int _Map::findValidIndex(CPtr key)C
{
   Int i; if(Elm *elm=findElm(key, i))if(!(elmDesc(*elm).flag&CACHE_ELM_LOADING))return i;
   return -1;
}
Ptr _MapTS::find(CPtr key)C
{
   SyncLocker locker(_lock);
   return super::find(key);
}
Int _MapTS::findValidIndex(CPtr key)C
{
   SyncLocker locker(_lock);
   return super::findValidIndex(key);
}
Int _MapTS::findAbsIndex(CPtr key)C
{
   SyncLocker locker(_lock);
   return super::findAbsIndex(key);
}
/******************************************************************************/
//Int _Map::getValidIndex(CPtr key);
Int _Map::getAbsIndex  (CPtr key) {return dataInMapToAbsIndex(get(key));}
Ptr _Map::get          (CPtr key)
{
   Int stop;

   // find existing
   if(Elm *elm=findElm(key, stop))return (elmDesc(*elm).flag&CACHE_ELM_LOADING) ? null : elmData(*elm);

   // always null
   if(_mode==MAP_ALL_NULL)return null;

   // new element
   auto max_elms =_memx.maxElms(); Elm &elm=_memx.New(); Desc &desc=elmDesc(elm); Ptr data=elmData(elm), elm_key=elmKey(elm); desc.flag=0;
   if(  max_elms!=_memx.maxElms())Realloc(_order, _memx.maxElms(), max_elms);

   // always dummy
   if(_mode==MAP_ALL_DUMMY)
   {
   dummy:
      desc.flag|=CACHE_ELM_DUMMY;
     _copy_key(elm_key, key);
      addToOrder(elm, stop);
      return data;
   }

   // create
   {
      desc.flag|=CACHE_ELM_LOADING;
     _copy_key(elm_key, key); // copy key before creating
      addToOrder(elm, stop);

      // try to load
      if(_create ? _create(data, elm_key, _user) : true)
      {
         FlagDisable(desc.flag, CACHE_ELM_LOADING);
         return data;
      }

      if(findElm(key, stop))
      {
         removeFromOrder(stop); // don't try to optimize by using the same 'stop' calculated from 'addToOrder' as it may have changed, because inside 'create' there could be other objects created
         FlagDisable(desc.flag, CACHE_ELM_LOADING);
      }
   }

   // dummy on fail
   if(_mode==MAP_DUMMY)goto dummy;

   // null
  _memx.removeData(&elm);
   return null;
}
Ptr _MapTS::get(CPtr key)
{
   SyncLocker locker(_lock);
   return super::get(key);
}
/*Int _MapTS::getValidIndex(CPtr key)
{
   SyncLocker locker(_lock);
   return super::getValidIndex(key);
}*/
Int _MapTS::getAbsIndex(CPtr key)
{
   SyncLocker locker(_lock);
   return super::getAbsIndex(key);
}
/******************************************************************************/
void _Map::getFailed()C
{
   if(_mode==MAP_EXIT)Exit("Can't create object in 'Map' from key");
}
Ptr _Map  ::require          (CPtr key) {if(Ptr        data=get          (key)                   )return        data; getFailed(); return null;}
Ptr _MapTS::require          (CPtr key) {if(Ptr        data=get          (key)                   )return        data; getFailed(); return null;}
//Int _Map  ::requireValidIndex(CPtr key) {   Int valid_index=getValidIndex(key); if(valid_index>=0)return valid_index; getFailed(); return   -1;}
//Int _MapTS::requireValidIndex(CPtr key) {   Int valid_index=getValidIndex(key); if(valid_index>=0)return valid_index; getFailed(); return   -1;}
Int _Map  ::requireAbsIndex  (CPtr key) {   Int   abs_index=getAbsIndex  (key); if(  abs_index>=0)return   abs_index; getFailed(); return   -1;}
Int _MapTS::requireAbsIndex  (CPtr key) {   Int   abs_index=getAbsIndex  (key); if(  abs_index>=0)return   abs_index; getFailed(); return   -1;}
/******************************************************************************/
Ptr _Map::get    (CPtr key, Bool &just_created) {Int elms=T.elms(); Ptr data=get    (key); just_created=(elms!=T.elms()); return data;}
Ptr _Map::require(CPtr key, Bool &just_created) {Int elms=T.elms(); Ptr data=require(key); just_created=(elms!=T.elms()); return data;}
/******************************************************************************/
Bool _Map::dummy(CPtr data)C
{
 C Elm *elm=dataElm(data); if(containsElm(elm))if(elmDesc(*elm).flag&CACHE_ELM_DUMMY)return true;
   return false;
}
void _Map::dummy(CPtr data, Bool dummy)
{
   Elm *elm=dataElm(data); if(containsElm(elm))FlagSet(elmDesc(*elm).flag, CACHE_ELM_DUMMY, dummy);
}
/******************************************************************************/
Bool _Map  ::containsKey(CPtr key)C {return find(key)!=null;}
Bool _MapTS::containsKey(CPtr key)C {return find(key)!=null;}
/******************************************************************************/
Bool _Map::containsData(CPtr data)C
{
 C Elm *elm=dataElm(data); if(containsElm(elm))if(!(elmDesc(*elm).flag&CACHE_ELM_LOADING))return true;
   return false;
}
Bool _MapTS::containsData(CPtr data)C
{
   if(data)
   {
      SyncLocker locker(_lock);
      return super::containsData(data);
   }
   return false;
}
/******************************************************************************
Int         absIndex(C Elm *elm )C {return _memx.  absIndex(        elm  );} // this is NOT thread-safe
Int       validIndex(C Elm *elm )C {return _memx.validIndex(        elm  );} // this is NOT thread-safe
Int         absIndex( CPtr  data)C {return         absIndex(dataElm(data));} // this is NOT thread-safe, assumes that '_data_offset' is zero
Int       validIndex( CPtr  data)C {return       validIndex(dataElm(data));} // this is NOT thread-safe, assumes that '_data_offset' is zero

INLINE Int _Map::absIndex(CPtr data)C // this function assumes that 'data' is not null
{
#if 1 // fast, this can work because '_data_offset' is now zero
   DEBUG_ASSERT(_data_offset==0, "Map data offset should be zero but it isn't");
   return _memx.absIndex(dataElm(data));
#else
   #if WINDOWS
      __try                               {return _memx.absIndex(dataElm(data));} // use 'absIndex' only when exception control is possible
      __except(EXCEPTION_EXECUTE_HANDLER) {return -1;}
   #else
      return _memx._abs.index(data);
   #endif
#endif
}
/******************************************************************************/
CPtr _Map::dataToKey(CPtr data)C
{
 C Elm *elm=dataElm(data); if(containsElm(elm))return elmKey(*elm); // key does not change while loading, so ignore CACHE_ELM_LOADING
   return null;
}
CPtr _MapTS::dataToKey(CPtr data)C
{
   if(data)
   {
      SyncLocker locker(_lock);
      return super::dataToKey(data);
   }
   return null;
}
/******************************************************************************/
Int _Map::dataToIndex(CPtr data)C
{
 C Elm *elm=dataElm(data); if(containsElm(elm)) // only after we know that this element belongs to this container then we can access its key
   {
      Int i; if(findElm(elmKey(*elm), i))return i;
   }
   return -1;
}
Int _MapTS::dataToIndex(CPtr data)C
{
   if(data)
   {
      SyncLocker locker(_lock);
      return super::dataToIndex(data);
   }
   return -1;
}
/******************************************************************************/
void _Map::remove(Int i)
{
   if(InRange(i, _elms))
   {
      Elm *elm=_order[i];
      removeFromOrder(i);
     _memx.removeData(elm);
   }
}
void _MapTS::remove(Int i)
{
   if(InRange(i, _elms))
   {
      SyncLocker locker(_lock);
      super::remove(i);
   }
}
void _Map::removeKey(CPtr key)
{
   Int i; if(Elm *elm=findElm(key, i))
   {
      removeFromOrder(i);
     _memx.removeData(elm);
   }
}
void _MapTS::removeKey(CPtr key)
{
   SyncLocker locker(_lock);
   super::removeKey(key);
}
void _Map::removeData(CPtr data)
{
 C Elm *elm=dataElm(data); if(containsElm(elm)) // only after we know that this element belongs to this container then we can access its key
   {
      Int i; if(findElm(elmKey(*elm), i))
      {
         removeFromOrder(i);
        _memx.removeData(elm);
      }
   }
}
void _MapTS::removeData(CPtr data)
{
   if(data)
   {
      SyncLocker locker(_lock);
      super::removeData(data);
   }
}
/******************************************************************************/
Bool _Map::replaceKey(CPtr src_key, CPtr dest_key)
{
   Int src_i, dest_i;
   Elm * src_elm=findElm( src_key,  src_i),
       *dest_elm=findElm(dest_key, dest_i);
   if(src_elm || dest_elm) // if at least one element was found
   {
      if(src_elm!=dest_elm) // it's not the same element (this would mean that both keys are the same and we don't need to do anything)
      {
         if(src_elm && dest_elm)
         {
            // these assertions are to make sure of the following member order in class: data, key, desc, allowing to calculate the 'key_size'
            typedef Map<Long, Long>::Elm MapElmTest; // needed because macro can't support comma
            ASSERT(OFFSET(MapElmTest, data)==0 // 'data' is first
                && OFFSET(MapElmTest, key )> 0 // 'key' is after 'data'
                && OFFSET(MapElmTest, desc)>OFFSET(MapElmTest, key) // 'desc' is after 'key'
                && SIZE(MapElmTest)==MEMBER_SIZE(MapElmTest, data)+MEMBER_SIZE(MapElmTest, key)+MEMBER_SIZE(MapElmTest, desc)); // only data+key+desc in class
            Int key_size=_desc_offset-_key_offset; // !! Warning: this may include some padding between KEY and DESC, however we're just swapping between valid 'Map.Elm' KEY objects, so it's OK, alternative would be to store '_key_size' in '_Map' but that would increase class size

            Swap(elmKey(*src_elm), elmKey(*dest_elm), key_size); // swap in case 'src_key', 'dest_key' point to 'elmKey'
            Swap(_order[src_i], _order[dest_i]); // adjust order position
         }else
         {
            if(src_elm)
            {
              _copy_key(elmKey(*src_elm), dest_key); // adjust key "src_elm.key = dest_key"
               if(dest_i>src_i)dest_i--; // we're moving element from the order to the right, which means that one slot is already occupied by this element
               MoveElm(_order, _elms, src_i, dest_i); // adjust order position
            }else
            if(dest_elm)
            {
              _copy_key(elmKey(*dest_elm), src_key); // adjust key "dest_elm.key = src_key"
               if(src_i>dest_i)src_i--; // we're moving element from the order to the right, which means that one slot is already occupied by this element
               MoveElm(_order, _elms, dest_i, src_i); // adjust order position
            }
         }
      }
      return true;
   }
   return false;
}
Bool _MapTS::replaceKey(CPtr src_key, CPtr dest_key)
{
   SyncLocker locker(_lock);
   return super::replaceKey(src_key, dest_key);
}
/******************************************************************************/
void _Map::reserve(Int num)
{
   auto max_elms =_memx.maxElms(); _memx.reserve(num);
   if(  max_elms!=_memx.maxElms())Realloc(_order, _memx.maxElms(), max_elms);
}
void _MapTS::reserve(Int num)
{
   SyncLocker locker(_lock);
   return super::reserve(num);
}
/******************************************************************************/
static Int MapCompare(_Map::Elm *C&a, _Map::Elm *C&b, CPtr user)
{
  _Map &map=*(_Map*)user;
   return map._compare(map.elmKey(*a), map.elmKey(*b));
}
void _Map::compare(Int compare(CPtr key_a, CPtr key_b))
{
   if(T._compare!=compare)
   {
      T._compare=compare;
      Sort(_order, _elms, this, MapCompare);
   }
}
void _MapTS::compare(Int compare(CPtr key_a, CPtr key_b))
{
   SyncLocker locker(_lock);
   return super::compare(compare);
}
/******************************************************************************/
void _Map::from(C _Map &src)
{
   del();

  _mode=src._mode;
//_compare, _copy_key - keep

#if 0 // modify dest type to match src type (this is not needed)
   _key_offset=src. _key_offset;
//_data_offset=src._data_offset;
  _desc_offset=src._desc_offset;
  _data_size  =src._data_size;
  _user       =src._user;
  _create     =src._create;
  _memx._reset(src._memx.elmSize(), src._memx.blockElms(), src._memx._new, src._memx._del);
#endif

  _elms       =src._elms;
  _memx.setNum(src._memx.elms());  // pre allocate memory for elements
   Alloc(_order, _memx.maxElms()); // initialize order exactly the same way as source
   REPA(_memx)                     //      setup order exactly the same way as source
   {
    C Elm  & src_elm =   *src._order[i]; Int memx_index=src._memx.validIndex(&src_elm); RANGE_ASSERT_ERROR(memx_index, _memx, "Invalid element index in Map.operator=(C Map &src)"); // get valid index of i-th element in memx container
      Elm  &dest_elm =_memx[memx_index]; _order[i]=&dest_elm; // set valid index of i-th element
    C Desc & src_desc=src.elmDesc( src_elm);
      Desc &dest_desc=    elmDesc(dest_elm);
      dest_desc=src_desc; // copy desc
     _copy_key(elmKey(dest_elm), src.elmKey(src_elm)); // copy key
   }
}
/******************************************************************************/
// MAP EX
/******************************************************************************/
void _MapEx::delayRemove(Flt time)
{
   Bool adjust_existing=true;
   MAX(time, 0);
   if( time!=_delay_remove_time)
   {
    /*SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);*/

      Flt delta=time-_delay_remove_time; // how much are we increasing the delay
     _delay_remove_time  =time; // set new value
     _delay_remove_check+=delta*DELAY_REMOVE_STEP; // adjust check time
      if(adjust_existing)REPAO(_delay_remove).time+=delta; // adjust element removal times
      update();
   }
}
/******************************************************************************/
Int _MapEx::findDelayRemove(Elm &elm)
{
   REPA(_delay_remove)if(_delay_remove[i].elm==&elm)return i;
   return -1;
}
/******************************************************************************/
Ptr _MapEx::find(CPtr key, Bool counted)
{
   Int i; if(Elm *elm=findElm(key, i))
   {
      DescPtrNum &desc=elmDesc(*elm); if(!(desc.flag&CACHE_ELM_LOADING))
      {
         if(counted)IncPtrNum(desc.ptr_num);else FlagEnable(desc.flag, CACHE_ELM_STD_PTR);
         return elmData(*elm);
      }
   }
   return null;
}
Ptr _MapEx::get(CPtr key, Bool counted)
{
   Ptr data=super::get(key); if(Elm *elm=dataElm(data))
   {
      DescPtrNum &desc=elmDesc(*elm);
      if(counted)IncPtrNum(desc.ptr_num);else FlagEnable(desc.flag, CACHE_ELM_STD_PTR);
   }
   return data;
}
Ptr _MapEx::require(CPtr key, Bool counted)
{
   if(Ptr data=get(key, counted))return data; getFailed(); return null;
}
/******************************************************************************/
void _MapEx::incRef(CPtr data)
{
   if(Elm *elm=dataElm(data))
 /*#if !SYNC_LOCK_SAFE // if 'SyncLock' is not safe then crash may occur when trying to lock, to prevent that, check if we have any elements (this means map was already initialized)
      if(_elms)
   #endif*/
   {
    /*SyncUnlocker unlocker(D._lock); // must be used even though we're not using GPU
      SyncLocker     locker(  _lock);*/
      if(containsElm(elm))IncPtrNum(elmDesc(*elm).ptr_num);
   }
}
void _MapEx::decRef(CPtr data)
{
   if(Elm *elm=dataElm(data))
 /*#if !SYNC_LOCK_SAFE // if 'SyncLock' is not safe then crash may occur when trying to lock, to prevent that, check if we have any elements (this means map was already initialized)
      if(_elms)
   #endif*/
   {
    /*SyncUnlocker unlocker(D._lock); // this must be used also since later 'D._lock' can be locked when deleting the resource
      SyncLocker     locker(  _lock);*/
      if(containsElm(elm))
      {
         DescPtrNum &desc=elmDesc(*elm); DEBUG_ASSERT(desc.ptr_num>0, "'_MapEx.decRef' Decreasing 'ptr_num' when it's already zero");
         if(!--desc.ptr_num && !(desc.flag&CACHE_ELM_STD_PTR)) // if there are no more pointers accessing this element
         {
            Flt delay_remove_time=_delay_remove_time;
          //if( delay_remove_time   >0 && GetThreadID()==DelayRemoveThreadID)delay_remove_time-=DelayRemoveWaited; // if want to use delayed remove by time and we're unloading because of parent getting delay unloaded, then decrease the time which the parent already waited
            if( delay_remove_time   >0 // if want to use delayed remove by time
          //|| _delay_remove_counter>0 // if want to use delayed remove temporarily
          //|| (_can_be_removed && !_can_be_removed(data)) // or it can't be removed right now
            )
            {
               Flt time=Time.appTime()+delay_remove_time; // set removal time
               if(desc.flag&CACHE_ELM_DELAY_REMOVE) // if already listed
               {
                  Int i=findDelayRemove(*elm); DEBUG_ASSERT(i>=0, "'_MapEx.decRef' Element has CACHE_ELM_DELAY_REMOVE but isn't listed in '_delay_remove'");
                 _delay_remove[i].time=time; // adjust time
               }else // not yet listed
               {
                  FlagEnable(desc.flag, CACHE_ELM_DELAY_REMOVE);
                  DelayRemove &remove=_delay_remove.New();
                  remove.elm =elm;
                  remove.time=time;
               }
            }else // remove now
            {
               if(desc.flag&CACHE_ELM_DELAY_REMOVE)_delay_remove.remove(findDelayRemove(*elm)); // if was listed in the 'delay_remove' then remove it from it
               removeKey(elmKey(*elm)); // 'removeKey' faster than 'removeData'
            }
         }
      }
   }
}
/******************************************************************************/
void _MapEx::processDelayRemove(Bool always)
{
   if(_delay_remove.elms())
   {
    /*SyncUnlocker   unlocker(D._lock); // this must be used also since later 'D._lock' can be locked when deleting the resource
      SyncLocker delay_locker(DelayRemoveLock); DelayRemoveThreadID=GetThreadID(); // support only one 'processDelayRemove' at a time, because we use only one global 'DelayRemoveWaited' based on 'DelayRemoveThreadID' for all maps/threads
      SyncLocker       locker(  _lock);*/
     _delay_remove_check=Time.appTime()+_delay_remove_time*DELAY_REMOVE_STEP; // perform next check at this time
      REPA(_delay_remove)
      {
         DelayRemove &remove=_delay_remove[i];
         if(always || Time.appTime()>=remove.time) // if always remove or enough time has passed (use >= so when having zero delay time then it will be processed immediately)
         {
            Elm &elm=*remove.elm; DescPtrNum &desc=elmDesc(elm); _delay_remove.remove(i); // access before removal and remove afterwards
            if(desc.ptr_num || (desc.flag&CACHE_ELM_STD_PTR)) // if there is something accessing this element now
            {
               FlagDisable(desc.flag, CACHE_ELM_DELAY_REMOVE); // keep the element but disable the 'CACHE_ELM_DELAY_REMOVE' flag since we've removed it from the '_delay_remove' container
            }else // nothing accessing this element
            {
               // remove element from map
             //DelayRemoveWaited=Max(0, Time.appTime()-remove.time+_delay_remove_time); // get how much time this element was waiting to be removed, set this before removing this element, so its children will be able to access it in the destructor
               removeKey(elmKey(elm)); // 'removeKey' faster than 'removeData'
            }
         }
      }
    //DelayRemoveWaited  =0; // clear back to zero before clearing 'DelayRemoveThreadID', in case some other thread has ID=0, and thus would use 'DelayRemoveWaited' from this thread
    //DelayRemoveThreadID=0;
   }
}
void _MapEx::delayRemoveNow() {                                                                                               processDelayRemove(true );}
void _MapEx::update        () {if(_delay_remove.elms() && /*_delay_remove_counter==0 &&*/ Time.appTime()>=_delay_remove_check)processDelayRemove(false);}
/******************************************************************************/
}
/******************************************************************************/
