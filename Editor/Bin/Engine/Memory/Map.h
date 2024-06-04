#pragma once
/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************

   Use 'Map' to quickly access custom data by creating it from specified key.
      Once 'Map' creates a resource, it will keep it in the memory for faster access.

   Objects in 'Map' containers are stored using 'Memx' container,
      which means that the memory address of the elements remains constant as long as the elements exist.

/******************************************************************************/
enum MAP_MODE : Byte // Map Mode
{
    MAP_EXIT,      //       load data, Exit  on fail
    MAP_NULL,      //       load data, null  on fail
    MAP_DUMMY,     //       load data, dummy on fail       (pointer to empty data, initialized with constructor but without the 'load' method)
    MAP_ALL_NULL,  // don't load data, always return null
    MAP_ALL_DUMMY, // don't load data, always return dummy (pointer to empty data, initialized with constructor but without the 'load' method)
};
/******************************************************************************/
T2(KEY, DATA)
struct Map : _Map // Map - container for dynamically created elements, consisting of unique keys and their corresponding data
{
    struct Elm : _Map::Elm {
        DATA data;
        KEY key;
        Desc desc;
    };

    // manage
    Map &clear(); // remove all elements
    Map &del();   // remove all elements and free helper memory

    // get
    Int elms() C;     // get number of elements in container
    Int dataSize() C; // get size of DATA element

    DATA *find(C KEY &key);       // find    element, don't create if not found, null on fail
    DATA *get(C KEY &key);        // get     element,       create if not found, null on fail
    DATA *operator()(C KEY &key); // require element,       create if not found, Exit on fail (unless different MAP_MODE selected)

    DATA *get(C KEY &key, Bool &just_created);        // get     element, create if not found, 'just_created'=will be set to true if just created new element, null on fail
    DATA *operator()(C KEY &key, Bool &just_created); // require element, create if not found, 'just_created'=will be set to true if just created new element, Exit on fail (unless different MAP_MODE selected)

    Int findValidIndex(C KEY &key) C; // find element valid index, don't create if not found, -1 on fail

    Int findAbsIndex(C KEY &key) C;  // find    element absolute index, don't create if not found,   -1 on fail
    Int getAbsIndex(C KEY &key);     // get     element absolute index,       create if not found,   -1 on fail
    Int requireAbsIndex(C KEY &key); // require element absolute index,       create if not found, Exit on fail (unless different MAP_MODE selected)

    Bool containsKey(C KEY &key) C;        // check if map contains an element with specified key
    Bool containsData(C DATA *data) C;     // check if map contains an element, testing is done by comparing elements memory address only
    C KEY *dataToKey(C DATA *data) C;      // get element key, this will return pointer to element's key if that element is      stored in this Map, null on fail
    C KEY *dataInMapToKey(C DATA *data) C; // get element key, this will return pointer to element's key,   that element must be stored in this Map or be null, this method is faster than 'key' because it does not check if element is stored in this Map
    C KEY &dataInMapToKey(C DATA &data) C; // get element key, this will return            element's key,   that element must be stored in this Map           , this method is faster than 'key' because it does not check if element is stored in this Map
    Int dataToIndex(C DATA *data) C;       // get element index in map, -1 on fail (if not stored in this Map)

    // operations
    C KEY &key(Int i) C;         // access i-th element key  from container
    DATA &operator[](Int i);     // access i-th element data from container
    C DATA &operator[](Int i) C; // access i-th element data from container

    C KEY &absKey(Int abs_i) C;   // access i-th absolute element key  from container, 'abs_i'=absolute index of the element
    DATA &absData(Int abs_i);     // access i-th absolute element data from container, 'abs_i'=absolute index of the element
    C DATA &absData(Int abs_i) C; // access i-th absolute element data from container, 'abs_i'=absolute index of the element

    MAP_MODE mode(MAP_MODE mode); // set map mode, returns previous mode

    void remove(Int i);                       // remove i-th element from container
    void removeKey(C KEY &key);               // remove      element from container
    void removeData(C DATA *data);            // remove      element from container
    Bool replaceKey(C KEY &src, C KEY &dest); // replace existing element 'src' key with 'dest', false on fail

    void reserve(Int num); // pre-allocate memory for storage of 'num' total elements

    void compare(Int compare(C KEY &a, C KEY &b)); // change 'compare' function, changing this function will result in all elements being re-sorted

    T1(EXTENDED)
    Map &replaceClass(); // replace the type of class stored in the container, all elements are automatically removed before changing the type of the class, the new type must be extended from the base 'DATA' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

    Map &operator=(C Map &src); // create from 'src'

    explicit Map(Int compare(C KEY &a, C KEY &b) = Compare, Bool create(DATA &data, C KEY &key, Ptr user) = null, Ptr user = null, Int block_elms = 64); // 'compare'=function which compares two keys, 'create'=function that creates 'data' on the base of the constant 'key'
};
/******************************************************************************/
T2(KEY, DATA)
struct MapEx : _MapEx // Map with reference count for elements
{
    struct Elm : _Map::Elm {
        DATA data;
        KEY key;
        DescPtrNum desc;
    };

    // get
    Int elms() C; // get number of elements in container

    MAP_MODE mode(MAP_MODE mode); // set map mode, returns previous mode
    MapEx &delayRemove(Flt time); // set amount of time (in seconds) after which unused elements are removed from map (<=0 value specifies immediate unloading), default=0
    MapEx &delayRemoveNow();      // immediately remove all elements that were marked for delay removal at a later time to free as much memory as possible

    void reserve(Int num); // pre-allocate memory for storage of 'num' total elements

    void update(); // update map to process all delay removed elements

    explicit MapEx(Int compare(C KEY &a, C KEY &b) = Compare, Bool create(DATA &data, C KEY &key, Ptr user) = null, Ptr user = null, Int block_elms = 64); // 'compare'=function which compares two keys, 'create'=function that creates 'data' on the base of the constant 'key'

  private:
    T1(EXTENDED)
    MapEx &replaceClass(); // replace the type of class stored in the container, all elements are automatically removed before changing the type of the class, the new type must be extended from the base 'DATA' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)
};
/******************************************************************************/
T2(KEY, DATA)
struct ThreadSafeMap : _MapTS // Thread Safe Map
{
    // manage
    ThreadSafeMap &clear(); // remove all elements
    ThreadSafeMap &del();   // remove all elements and free helper memory

    // get
    Int elms() C;     // get number of elements in container
    Int dataSize() C; // get size of DATA element

    DATA *find(C KEY &key);       // find    element, don't create if not found, null on fail
    DATA *get(C KEY &key);        // get     element,       create if not found, null on fail
    DATA *operator()(C KEY &key); // require element,       create if not found, Exit on fail (unless different MAP_MODE selected)

    Int findValidIndex(C KEY &key) C; // find element valid index, don't create if not found, -1 on fail

    Int findAbsIndex(C KEY &key) C;  // find    element absolute index, don't create if not found,   -1 on fail
    Int getAbsIndex(C KEY &key);     // get     element absolute index,       create if not found,   -1 on fail
    Int requireAbsIndex(C KEY &key); // require element absolute index,       create if not found, Exit on fail (unless different MAP_MODE selected)

    Bool containsKey(C KEY &key) C;        // check if map contains an element with specified key
    Bool containsData(C DATA *data) C;     // check if map contains an element, testing is done by comparing elements memory address only
    C KEY *dataToKey(C DATA *data) C;      // get element key, this will return pointer to element's key if that element is      stored in this Map, null on fail
    C KEY *dataInMapToKey(C DATA *data) C; // get element key, this will return pointer to element's key,   that element must be stored in this Map or be null, this method is faster than 'key' because it does not check if element is stored in this Map
    C KEY &dataInMapToKey(C DATA &data) C; // get element key, this will return            element's key,   that element must be stored in this Map           , this method is faster than 'key' because it does not check if element is stored in this Map
    Int dataToIndex(C DATA *data) C;       // get element index in map, -1 on fail (if not stored in this Map)

    // operations
    void lock() C;   //   lock elements container, unlock must be called after locking container
    void unlock() C; // unlock elements container, this   must be called after locking the container

    C KEY &lockedKey(Int i) C;   // access i-th element key  from container, this can be used after locking and before unlocking the container
    DATA &lockedData(Int i);     // access i-th element data from container, this can be used after locking and before unlocking the container
    C DATA &lockedData(Int i) C; // access i-th element data from container, this can be used after locking and before unlocking the container

    C KEY &lockedAbsKey(Int abs_i) C;   // access i-th absolute element key  from container, this can be used after locking and before unlocking the container, 'abs_i'=absolute index of the element
    DATA &lockedAbsData(Int abs_i);     // access i-th absolute element data from container, this can be used after locking and before unlocking the container, 'abs_i'=absolute index of the element
    C DATA &lockedAbsData(Int abs_i) C; // access i-th absolute element data from container, this can be used after locking and before unlocking the container, 'abs_i'=absolute index of the element

    MAP_MODE mode(MAP_MODE mode); // set map mode, returns previous mode

    void lockedRemove(Int i);                 // remove i-th element from container, assumes map is already locked
    void remove(Int i);                       // remove i-th element from container
    void removeKey(C KEY &key);               // remove      element from container
    void removeData(C DATA *data);            // remove      element from container
    Bool replaceKey(C KEY &src, C KEY &dest); // replace existing element 'src' key with 'dest', false on fail

    void reserve(Int num); // pre-allocate memory for storage of 'num' total elements

    void compare(Int compare(C KEY &a, C KEY &b)); // change 'compare' function, changing this function will result in all elements being re-sorted

    T1(EXTENDED)
    ThreadSafeMap &replaceClass(); // replace the type of class stored in the container, all elements are automatically removed before changing the type of the class, the new type must be extended from the base 'DATA' (if you're receiving a compilation error pointing to this method this means that the new class isn't extended from the base class)

    ThreadSafeMap &operator=(C ThreadSafeMap &src); // create from 'src'

    explicit ThreadSafeMap(Int compare(C KEY &a, C KEY &b) = Compare, Bool create(DATA &data, C KEY &key, Ptr user) = null, Ptr user = null, Int block_elms = 64); // 'compare'=function which compares two keys, 'create'=function that creates 'data' on the base of the constant 'key'
};
/******************************************************************************/
// MAP ELEMENT POINTER
/******************************************************************************/
template <typename KEY, typename DATA, MapEx<KEY, DATA> &MAP>
struct MapElmPtr // Map Element Pointer - can hold a reference to a DATA based object in the MAP map, number of active references for a given object is stored in the map
{
    // operators
    DATA *operator()() C { return T._data; }                               // access the data, you can use the returned pointer   as long as this 'MapElmPtr' object exists and not modified
    DATA *operator->() C { return T._data; }                               // access the data, you can use the returned pointer   as long as this 'MapElmPtr' object exists and not modified
    DATA &operator*() C { return *T._data; }                               // access the data, you can use the returned reference as long as this 'MapElmPtr' object exists and not modified
    Bool operator==(null_t) C { return T._data == null; }                  // if pointers are equal
    Bool operator!=(null_t) C { return T._data != null; }                  // if pointers are different
    Bool operator==(C DATA *data) C { return T._data == data; }            // if pointers are equal
    Bool operator!=(C DATA *data) C { return T._data != data; }            // if pointers are different
    Bool operator==(C MapElmPtr &eptr) C { return T._data == eptr._data; } // if pointers are equal
    Bool operator!=(C MapElmPtr &eptr) C { return T._data != eptr._data; } // if pointers are different
    operator Bool() C { return T._data != null; }                          // if pointer  is  valid

    // get
    Bool dummy() C;         // check if this object is a dummy (it was not loaded but created as empty)
    void dummy(Bool dummy); // set dummy state for this object (this can be used for example if object was first loaded as a dummy, but then you've downloaded/generated/saved its data, and now need to update the dummy state)

    // operations
    MapElmPtr &clear();                      // clear the pointer to  null , this automatically decreases the reference count of current data
    MapElmPtr &operator=(null_t);            // clear the pointer to  null , this automatically decreases the reference count of current data
    MapElmPtr &operator=(DATA *data);        // set       pointer to 'data', this automatically decreases the reference count of current data and increases the reference count of the new data
    MapElmPtr &operator=(C MapElmPtr &eptr); // set       pointer to 'eptr', this automatically decreases the reference count of current data and increases the reference count of the new data
    MapElmPtr &operator=(MapElmPtr &&eptr);  // set       pointer to 'eptr', this automatically decreases the reference count of current data and increases the reference count of the new data

    // get object and store it temporarily (as long as it is referenced by at least one 'MapElmPtr')
    MapElmPtr &find(C KEY &key);      // find    object by its file name ID, don't load if not found, null on fail
    MapElmPtr &get(C KEY &key);       // get     object by its file name ID,       load if not found, null on fail
    MapElmPtr &require(C KEY &key);   // require object by its file name ID,       load if not found, Exit on fail (unless different MAP_MODE selected)
    MapElmPtr &operator=(C KEY &key); // require object by its file name ID,       load if not found, Exit on fail (unless different MAP_MODE selected)

    // constructors / destructors
    MapElmPtr(null_t = null);     // initialize the pointer with  null
    MapElmPtr(DATA *data);        // initialize the pointer with 'data', this automatically increases the reference count of the    'data'
    MapElmPtr(C MapElmPtr &eptr); // initialize the pointer with 'eptr', this automatically increases the reference count of the    'eptr'
    MapElmPtr(MapElmPtr &&eptr);  // initialize the pointer with 'eptr', this automatically increases the reference count of the    'eptr'
    MapElmPtr(C KEY &key);        // initialize the pointer with 'key' , this automatically increases the reference count of the    'file', works exactly the same as 'operator=(C KEY &key)', require object, load if not found, Exit on fail (unless different MAP_MODE selected)
    ~MapElmPtr();                 // release    the pointer            , this automatically decreases the reference count of current data

  private:
    DATA *_data;
};
/******************************************************************************/
struct MapLock // Map Lock (automatically locks and unlocks the map at object creation and destruction)
{
    explicit MapLock(_MapTS &map) : _map(map) { _map.lock(); }
    ~MapLock() { _map.unlock(); }

  private:
    _MapTS &_map;
    NO_COPY_CONSTRUCTOR(MapLock);
};
/******************************************************************************/
