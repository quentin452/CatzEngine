#pragma once
/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************/
struct _Memc // Continuous Memory Based Container Base - Do not use this class, use 'Memc' instead
{
    static constexpr Bool Continuous = true; // Memc memory is continuous

    void del();
    void clear();

    Int elms() C { return _elms; }
    UInt elmSize() C { return _elm_size; }
    Ptr data() C { return _data; }
    UIntPtr memUsage() C { return UIntPtr(_max_elms) * _elm_size; }
    UIntPtr elmsMem() C { return UIntPtr(_elms) * _elm_size; }
#if EE_PRIVATE
    UInt maxElms() C { return _max_elms; }
    Bool initialized() C { return _elm_size > 0; }
    ULong requiredMem() C { return ULong(_max_elms) * _elm_size; } // this needs to use biggest possible type to check if we can support required size
#endif

    Ptr addr(Int i) C { return InRange(i, _elms) ? _element(i) : null; }
    Ptr addrFirst() C { return _elms ? _data : null; }
    Ptr addrLast() C { return _elms ? _element(_elms - 1) : null; }
    Ptr operator[](Int i) C {
        DEBUG_RANGE_ASSERT(i, _elms);
        return _element(i);
    }
    Ptr operator()(Int i);
    Ptr first() C { return T[0]; }
    Ptr last() C { return T[elms() - 1]; }
    Ptr New() { return T[addNum(1)]; }
    Ptr NewAt(Int i);

    Int index(CPtr elm) C;
    Bool contains(CPtr elm) C { return index(elm) >= 0; }

    void removeLast();
    void remove(Int i, Bool keep_order = false);
    void removeNum(Int i, Int n, Bool keep_order = false);
    void removeData(CPtr elm, Bool keep_order = false);

    void setNum(Int num);
    void setNum(Int num, Int keep);
    void setNumZero(Int num);
    void setNumZero(Int num, Int keep);
    void setNumDiscard(Int num);
    Int addNum(Int num);
    Int addNumZero(Int num);
    void reserve(Int num);
    void reserveAdd(Int num);

    Bool binarySearch(CPtr value, Int &index, Int compare(CPtr a, CPtr b)) C;

    void sort(Int compare(CPtr a, CPtr b));
    void sort(CPtr user, Int compare(CPtr a, CPtr b, CPtr user));
    void reverseOrder();
    void randomizeOrder();
    void rotateOrder(Int offset);
    void swapOrder(Int i, Int j);
    void moveElm(Int elm, Int new_index);

    Bool saveRaw(File &f) C;
    Bool loadRaw(File &f);

#if EE_PRIVATE
    void moveElmLeftUnsafe(Int elm, Int new_index, Ptr temp);

    void copyTo(Ptr dest) C;
    void copyFrom(CPtr src);

    Bool _saveRaw(File &f) C;
    Bool _loadRaw(File &f);
#endif

    ~_Memc() { del(); }

  private:
    Int _elms;
    UInt _elm_size, _max_elms;
    Ptr _data;
    void (*_new)(Ptr elm),
        (*_del)(Ptr elm);

    explicit _Memc(Int elm_size, void (*_new)(Ptr elm), void (*_del)(Ptr elm));
    void _reset(Int elm_size, void (*_new)(Ptr elm), void (*_del)(Ptr elm));
#if EE_PRIVATE
    void _maxElms(UInt max_elms);
#endif

    inline Ptr _element(Int i) C { return (Byte *)_data + UIntPtr(i) * _elm_size; }

    NO_COPY_CONSTRUCTOR(_Memc);

    T1(TYPE)
    friend struct Memc;
    T1(TYPE)
    friend struct MemcAbstract;
    friend struct _MemcThreadSafe;
};
/******************************************************************************/
