/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************/
#if EE_PRIVATE
struct IndexGroup
{
   Int num; Int *elm;

   void changeVal(Int from, Int to);
   void    remVal(Int value);
   void    subVal(Int value);
   void    subElm(Int i);

   void inc(Int num=1) {T.num+=num;}
   void add(Int elm  ) {T.elm[num++]=elm;}

   Int& operator[](Int i)  {return elm[i];}
   Int  operator[](Int i)C {return elm[i];}
};
/******************************************************************************/
struct IndexPtrGroup
{
   Int num; Ptr *elm;

   void changeVal(Ptr from, Ptr to);
   void    remVal(Ptr value);
   void    subVal(Ptr value);
   void    subElm(Int i);

   void inc(Int num=1) {T.num+=num;}
   void add(Ptr elm  ) {T.elm[num++]=elm;}

   Ptr& operator[](Int i)  {return elm[i];}
   Ptr  operator[](Int i)C {return elm[i];}
};
/******************************************************************************/
struct Index
{
   Int         groups, elms, group_elms_max, elms_max, *elm_group;
   Int        *groups_elm;
   IndexGroup *group;

   Index& del   ();
   Index& create(Int groups, Int elms_max=0);
   Index& reset ();
   Index& set   (Int *elm_group=null);

   void incGroup(Int group, Int num=1) {T.group[group].inc(num);}
   void addElm  (Int group, Int elm  ) {T.group[group].add(elm);}
   void elmGroup(Int group, Int elm  ) {T.group[elm_group[elm]=group].num++;}

           ~Index(                          ) {del ( );}
            Index(                          ) {Zero(T);}
   explicit Index(Int groups, Int elms_max=0) {Zero(T); create(groups, elms_max);}

   NO_COPY_CONSTRUCTOR(Index);
};
/******************************************************************************/
struct IndexPtr
{
   Int            groups, elms, group_elms_max;
   Ptr           *groups_elm;
   IndexPtrGroup *group;

   IndexPtr& del   ();
   IndexPtr& create(Int groups);
   IndexPtr& reset ();
   IndexPtr& set   ();

   void incGroup(Int group, Int num=1) {T.group[group].inc(num);}
   void addElm  (Int group, Ptr elm  ) {T.group[group].add(elm);}

           ~IndexPtr(          ) {del ( );}
            IndexPtr(          ) {Zero(T);}
   explicit IndexPtr(Int groups) {Zero(T); create(groups);}

   NO_COPY_CONSTRUCTOR(IndexPtr);
};
/******************************************************************************/
void CreateIs       (MemPtr<Bool> is, C CMemPtr<Int> &selection, Int elms);
void CreateIsNot    (MemPtr<Bool> is, C CMemPtr<Int> &selection, Int elms);
void CreateFaceIs   (MemPtr<Bool> tri_is, MemPtr<Bool> quad_is, C CMemPtr<Int> &faces, Int tris, Int quads);
void CreateFaceIsNot(MemPtr<Bool> tri_is, MemPtr<Bool> quad_is, C CMemPtr<Int> &faces, Int tris, Int quads);
/******************************************************************************/
void SetFaceIndex(Ptr data, C VecI *tri, Int tris, C VecI4 *quad, Int quads, Bool bit16);
/******************************************************************************/
#if EE_PRIVATE
       void SetRemap(   MemPtr<Int>  remap, C CMemPtr<Bool> &is, Int elms);
       void IndRemap(C CMemPtr<Int> &remap, Int   *ind, Int elms);
inline void IndRemap(C CMemPtr<Int> &remap, VecI2 *ind, Int elms) {IndRemap(remap, (Int*)ind, elms*2);}
inline void IndRemap(C CMemPtr<Int> &remap, VecI  *ind, Int elms) {IndRemap(remap, (Int*)ind, elms*3);}
inline void IndRemap(C CMemPtr<Int> &remap, VecI4 *ind, Int elms) {IndRemap(remap, (Int*)ind, elms*4);}

       Bool IndSave(File &f, C Int   *ind, Int inds, Int elms=-1);
       Bool IndLoad(File &f,   Int   *ind, Int inds             );
inline Bool IndSave(File &f, C VecI2 *ind, Int inds, Int elms=-1) {return IndSave(f, ind->c, inds*2, elms);}
inline Bool IndLoad(File &f,   VecI2 *ind, Int inds             ) {return IndLoad(f, ind->c, inds*2      );}
inline Bool IndSave(File &f, C VecI  *ind, Int inds, Int elms=-1) {return IndSave(f, ind->c, inds*3, elms);}
inline Bool IndLoad(File &f,   VecI  *ind, Int inds             ) {return IndLoad(f, ind->c, inds*3      );}
inline Bool IndSave(File &f, C VecI4 *ind, Int inds, Int elms=-1) {return IndSave(f, ind->c, inds*4, elms);}
inline Bool IndLoad(File &f,   VecI4 *ind, Int inds             ) {return IndLoad(f, ind->c, inds*4      );}
#endif
/******************************************************************************/
inline Int Elms(C IndexGroup    &index) {return index.num   ;}
inline Int Elms(C IndexPtrGroup &index) {return index.num   ;}
inline Int Elms(C Index         &index) {return index.groups;}
/******************************************************************************/
#endif
Bool  ElmIs(C CMemPtr<Bool> &is, Int i); // if i-th element is in range and set to true
Int CountIs(C CMemPtr<Bool> &is       ); // return the number of elements in the 'is' array which are true
/******************************************************************************/
