/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
static inline Bool EqualTex(C Vec2 &a, C Vec2 &b, Bool wrap=false) {return wrap ? EqualWrap(a, b) : Equal(a, b);}
/******************************************************************************/
Int SetVtxDup(MemPtr<VtxDup> vtxs, C Box &box, Flt pos_eps)
{
   Int unique=0;

   // link box->vtx
   Boxes boxes  (box        , vtxs.elms());
   Index box_vtx(boxes.num(), vtxs.elms());
   REPA(vtxs)box_vtx.elmGroup(boxes.index(vtxs[i].pos), i); box_vtx.set();

   // get duplicates
   Int xs=boxes.cells.x,
       ys=boxes.cells.y, xys=xs*ys;

   // iterate all boxes
   FREPD(z, boxes.cells.z)
   FREPD(y, boxes.cells.y)
   FREPD(x, boxes.cells.x)
   {
      Int         box_tests=0;
      IndexGroup *box_test[2+3+9],
                 *box_cur=&box_vtx.group[x + y*xs + z*xys]; // get current box

      // set which neighbor boxes to test
           box_test[box_tests++]=box_cur  ;
      if(x)box_test[box_tests++]=box_cur-1;
      if(y)
      {
         if(x<xs-1)box_test[box_tests++]=box_cur+1-xs;
                   box_test[box_tests++]=box_cur  -xs;
         if(     x)box_test[box_tests++]=box_cur-1-xs;
      }
      if(z)
      {
         if(y<ys-1)
         {
            if(x<xs-1)box_test[box_tests++]=box_cur+1+xs-xys;
                      box_test[box_tests++]=box_cur  +xs-xys;
            if(     x)box_test[box_tests++]=box_cur-1+xs-xys;
         }
            if(x<xs-1)box_test[box_tests++]=box_cur+1   -xys;
                      box_test[box_tests++]=box_cur     -xys;
            if(     x)box_test[box_tests++]=box_cur-1   -xys;
         if(y)
         {
            if(x<xs-1)box_test[box_tests++]=box_cur+1-xs-xys;
                      box_test[box_tests++]=box_cur  -xs-xys;
            if(     x)box_test[box_tests++]=box_cur-1-xs-xys;
         }
      }

      FREPA(*box_cur) // iterate all vertexes in this box
      {
         Int     vtx_cur_i=  (*box_cur)[i]; // this is i-th vtx in this box
         VtxDup &vtx_cur  =vtxs[vtx_cur_i];
       C Vec    &pos_cur  =vtx_cur.pos; // this is position of that vertex
         REPD(c, box_tests) // iterate all boxes to test
         {
            IndexGroup *bt=box_test[c];
            REPD(m, (box_cur==bt) ? i : bt->num) // iterate all vtxs in the test box (if the test box is the current box, then check only vertexes before the current one)
            {
               Int     vtx_test_i=(*bt)[m]; // this is m-th vtx in the test box
             C VtxDup &vtx_test  =vtxs[vtx_test_i];
               if(vtx_test.dup==vtx_test_i // if this vtx is unique (points to self), this is so that we assign mapping only to uniqe vtxs (and not to vtxs that point to other vtxs)
               && Equal(pos_cur, vtx_test.pos, pos_eps)) // if position is the same
               {
                  // found a duplicate
                  vtx_cur.dup=vtx_test_i; goto next; // set 'dup' index to point to unique 'vtx_test_i'
               }
            }
         }
         // haven't found a duplicate, so set as unique
         vtx_cur.dup=vtx_cur_i; // set 'dup' index to point to self
         unique++; // increase unique counter
      next:;
      }
   }
   return unique;
}
Int SetVtxDup(MemPtr<VtxDupNrm> vtxs, C Box &box, Flt pos_eps, Flt nrm_cos)
{
   Int unique=0;

   // link box->vtx
   Boxes boxes  (box        , vtxs.elms());
   Index box_vtx(boxes.num(), vtxs.elms());
   REPA(vtxs)box_vtx.elmGroup(boxes.index(vtxs[i].pos), i); box_vtx.set();

   // get duplicates
   Int xs=boxes.cells.x,
       ys=boxes.cells.y, xys=xs*ys;

   // iterate all boxes
   FREPD(z, boxes.cells.z)
   FREPD(y, boxes.cells.y)
   FREPD(x, boxes.cells.x)
   {
      Int         box_tests=0;
      IndexGroup *box_test[2+3+9],
                 *box_cur=&box_vtx.group[x + y*xs + z*xys]; // get current box

      // set which neighbor boxes to test
           box_test[box_tests++]=box_cur  ;
      if(x)box_test[box_tests++]=box_cur-1;
      if(y)
      {
         if(x<xs-1)box_test[box_tests++]=box_cur+1-xs;
                   box_test[box_tests++]=box_cur  -xs;
         if(     x)box_test[box_tests++]=box_cur-1-xs;
      }
      if(z)
      {
         if(y<ys-1)
         {
            if(x<xs-1)box_test[box_tests++]=box_cur+1+xs-xys;
                      box_test[box_tests++]=box_cur  +xs-xys;
            if(     x)box_test[box_tests++]=box_cur-1+xs-xys;
         }
            if(x<xs-1)box_test[box_tests++]=box_cur+1   -xys;
                      box_test[box_tests++]=box_cur     -xys;
            if(     x)box_test[box_tests++]=box_cur-1   -xys;
         if(y)
         {
            if(x<xs-1)box_test[box_tests++]=box_cur+1-xs-xys;
                      box_test[box_tests++]=box_cur  -xs-xys;
            if(     x)box_test[box_tests++]=box_cur-1-xs-xys;
         }
      }

      FREPA(*box_cur) // iterate all vertexes in this box
      {
         Int        vtx_cur_i=  (*box_cur)[i]; // this is i-th vtx in this box
         VtxDupNrm &vtx_cur  =vtxs[vtx_cur_i];
       C Vec       &pos_cur  =vtx_cur.pos; // this is position of that vertex
       C Vec       &nrm_cur  =vtx_cur.nrm; // this is normal   of that vertex
         REPD(c, box_tests) // iterate all boxes to test
         {
            IndexGroup *bt=box_test[c];
            REPD(m, (box_cur==bt) ? i : bt->num) // iterate all vtxs in the test box (if the test box is the current box, then check only vertexes before the current one)
            {
               Int        vtx_test_i=(*bt)[m]; // this is m-th vtx in the test box
             C VtxDupNrm &vtx_test  =vtxs[vtx_test_i];
               if(vtx_test.dup==vtx_test_i // if this vtx is unique (points to self), this is so that we assign mapping only to uniqe vtxs (and not to vtxs that point to other vtxs)
               && Equal(pos_cur, vtx_test.pos, pos_eps) // if position is the same
               && Dot  (nrm_cur, vtx_test.nrm)>=nrm_cos)
               {
                  // found a duplicate
                  vtx_cur.dup=vtx_test_i; goto next; // set 'dup' index to point to unique 'vtx_test_i'
               }
            }
         }
         // haven't found a duplicate, so set as unique
         vtx_cur.dup=vtx_cur_i; // set 'dup' index to point to self
         unique++; // increase unique counter
      next:;
      }
   }
   return unique;
}
/******************************************************************************/
MeshBase& MeshBase::setVtxDup2D(MESH_FLAG flag, Flt pos_eps, Flt nrm_cos)
{
   include(VTX_DUP); // vtx dup doesn't need to be initialized here, because the algorithm works in a way that only processed vertexes are tested
   flag&=(T.flag()&(VTX_NRM_TAN_BIN|VTX_HLP|VTX_TEX_ALL|VTX_SIZE|VTX_SKIN|VTX_MATERIAL|VTX_COLOR|VTX_FLAG)); // only these are tested
   if(nrm_cos<=-1)FlagDisable(flag, VTX_NRM); // disable vtx normal tests if we have tolerant 'nrm_cos'

   // link rect->vtx
   Rects rects   (Rect(T)    , vtxs());
   Index rect_vtx(rects.num(), vtxs());
   REPA(vtx)rect_vtx.elmGroup(rects.index(vtx.pos(i).xy), i); rect_vtx.set();

   // get duplicates
   Int xs=rects.cells.x,
       ys=rects.cells.y;

   FREPD(y, ys)
   FREPD(x, xs)
   {
      Int         rect_tests=0;
      IndexGroup *rect_test[2+3], *rect_cur=&rect_vtx.group[x+y*xs];
           rect_test[rect_tests++]=rect_cur  ;
      if(x)rect_test[rect_tests++]=rect_cur-1;
      if(y)
      {
         if(x<xs-1)rect_test[rect_tests++]=rect_cur+1-xs;
                   rect_test[rect_tests++]=rect_cur  -xs;
         if(x     )rect_test[rect_tests++]=rect_cur-1-xs;
      }

      FREPA(*rect_cur)
      {
         Int   ind_cur=(*rect_cur)[i];
         Vec2 &pos_cur=vtx.pos(ind_cur).xy;
         REPD(c, rect_tests)
         {
            IndexGroup *rt=rect_test[c];
            REPD(j, (rect_cur==rt) ? i : rt->num)
            {
               Int ind_test=(*rt)[j]; if(vtx.dup(ind_test)==ind_test && Equal(pos_cur, vtx.pos(ind_test).xy, pos_eps))
               {
                  if(flag)
                  {
                     if(flag&VTX_NRM      &&  Dot     (vtx.nrm     (ind_cur), vtx.nrm     (ind_test))<nrm_cos    )continue;
                     if(flag&VTX_TAN      &&  Dot     (vtx.tan     (ind_cur), vtx.tan     (ind_test))<EPS_TAN_COS)continue;
                     if(flag&VTX_BIN      &&  Dot     (vtx.bin     (ind_cur), vtx.bin     (ind_test))<EPS_BIN_COS)continue;
                     if(flag&VTX_HLP      && !Equal   (vtx.hlp     (ind_cur), vtx.hlp     (ind_test))            )continue;
                     if(flag&VTX_TEX0     && !EqualTex(vtx.tex0    (ind_cur), vtx.tex0    (ind_test))            )continue;
                     if(flag&VTX_TEX1     && !EqualTex(vtx.tex1    (ind_cur), vtx.tex1    (ind_test))            )continue;
                     if(flag&VTX_TEX2     && !EqualTex(vtx.tex2    (ind_cur), vtx.tex2    (ind_test))            )continue;
                     if(flag&VTX_TEX3     && !EqualTex(vtx.tex3    (ind_cur), vtx.tex3    (ind_test))            )continue;
                     if(flag&VTX_SIZE     && !Equal   (vtx.size    (ind_cur), vtx.size    (ind_test))            )continue;
                     if(flag&VTX_BLEND    &&           vtx.blend   (ind_cur)!=vtx.blend   (ind_test)             )continue;
                     if(flag&VTX_MATRIX   &&           vtx.matrix  (ind_cur)!=vtx.matrix  (ind_test)             )continue;
                     if(flag&VTX_MATERIAL &&           vtx.material(ind_cur)!=vtx.material(ind_test)             )continue;
                     if(flag&VTX_COLOR    &&           vtx.color   (ind_cur)!=vtx.color   (ind_test)             )continue;
                     if(flag&VTX_FLAG     &&           vtx.flag    (ind_cur)!=vtx.flag    (ind_test)             )continue;
                  }
                  vtx.dup(ind_cur)=ind_test; goto next;
               }
            }
         }
         vtx.dup(ind_cur)=ind_cur;
         next:;
      }
   }
   return T;
}
MeshBase& MeshBase::setVtxDup  (MESH_FLAG flag, Flt pos_eps, Flt nrm_cos                                                                             ) {return setVtxDupEx(flag, pos_eps, nrm_cos);}
MeshBase& MeshBase::setVtxDupEx(MESH_FLAG flag, Flt pos_eps, Flt nrm_cos, Flt tan_cos, Flt bin_cos, Bool tex_wrap, Bool smooth_groups_in_vtx_material)
{
   include(VTX_DUP); // vtx dup doesn't need to be initialized here, because the algorithm works in a way that only processed vertexes are tested
   MESH_FLAG t_flag=T.flag();
   flag                         &=(t_flag&(VTX_NRM_TAN_BIN|VTX_HLP|VTX_TEX_ALL|VTX_SIZE|VTX_SKIN|VTX_MATERIAL|VTX_COLOR|VTX_FLAG)); // only these are tested
   smooth_groups_in_vtx_material&=FlagOn(t_flag, VTX_MATERIAL);
   if(nrm_cos<=-1)FlagDisable(flag, VTX_NRM); // disable vtx normal   tests if we have tolerant 'nrm_cos'
   if(tan_cos<=-1)FlagDisable(flag, VTX_TAN); // disable vtx tangent  tests if we have tolerant 'tan_cos'
   if(bin_cos<=-1)FlagDisable(flag, VTX_BIN); // disable vtx binormal tests if we have tolerant 'bin_cos'

   // link box->vtx
   Boxes boxes  (Box(T)     , vtxs());
   Index box_vtx(boxes.num(), vtxs());
   REPA(vtx)box_vtx.elmGroup(boxes.index(vtx.pos(i)), i); box_vtx.set();

   // get duplicates
   Int xs=boxes.cells.x,
       ys=boxes.cells.y, xys=xs*ys;

   FREPD(z, boxes.cells.z)
   FREPD(y, boxes.cells.y)
   FREPD(x, boxes.cells.x)
   {
      Int         box_tests=0;
      IndexGroup *box_test[2+3+9], *box_cur=&box_vtx.group[x+y*xs+z*xys];
           box_test[box_tests++]=box_cur  ;
      if(x)box_test[box_tests++]=box_cur-1;
      if(y)
      {
         if(x<xs-1)box_test[box_tests++]=box_cur+1-xs;
                   box_test[box_tests++]=box_cur  -xs;
         if(     x)box_test[box_tests++]=box_cur-1-xs;
      }
      if(z)
      {
         if(y<ys-1)
         {
            if(x<xs-1)box_test[box_tests++]=box_cur+1+xs-xys;
                      box_test[box_tests++]=box_cur  +xs-xys;
            if(     x)box_test[box_tests++]=box_cur-1+xs-xys;
         }
            if(x<xs-1)box_test[box_tests++]=box_cur+1   -xys;
                      box_test[box_tests++]=box_cur     -xys;
            if(     x)box_test[box_tests++]=box_cur-1   -xys;
         if(y)
         {
            if(x<xs-1)box_test[box_tests++]=box_cur+1-xs-xys;
                      box_test[box_tests++]=box_cur  -xs-xys;
            if(     x)box_test[box_tests++]=box_cur-1-xs-xys;
         }
      }

      FREPA(*box_cur) // iterate all vertexes in current box
      {
         Int  ind_cur=(*box_cur)[i]; // vertex index
       C Vec &pos_cur=vtx.pos(ind_cur); // vertex position
         REPD(c, box_tests) // iterate all nearby boxes for testing
         {
            IndexGroup *bt=box_test[c]; // box for testing
            REPD(j, (box_cur==bt) ? i : bt->num) // iterate all vertexes in box for testing (but if it's current box then check only vertexes up to current vertex to make sure they were already processed before)
            {
               Int ind_test=(*bt)[j]; // test vertex index
               if(vtx.dup(ind_test)==ind_test // if its unique
               && Equal(pos_cur, vtx.pos(ind_test), pos_eps)) // same position
               {
                  if(flag)
                  {
                     if(flag&VTX_NRM      &&  Dot     (vtx.nrm     (ind_cur), vtx.nrm     (ind_test))<nrm_cos  )continue;
                     if(flag&VTX_TAN      &&  Dot     (vtx.tan     (ind_cur), vtx.tan     (ind_test))<tan_cos  )continue;
                     if(flag&VTX_BIN      &&  Dot     (vtx.bin     (ind_cur), vtx.bin     (ind_test))<bin_cos  )continue;
                     if(flag&VTX_HLP      && !Equal   (vtx.hlp     (ind_cur), vtx.hlp     (ind_test))          )continue;
                     if(flag&VTX_TEX0     && !EqualTex(vtx.tex0    (ind_cur), vtx.tex0    (ind_test), tex_wrap))continue;
                     if(flag&VTX_TEX1     && !EqualTex(vtx.tex1    (ind_cur), vtx.tex1    (ind_test), tex_wrap))continue;
                     if(flag&VTX_TEX2     && !EqualTex(vtx.tex2    (ind_cur), vtx.tex2    (ind_test), tex_wrap))continue;
                     if(flag&VTX_TEX3     && !EqualTex(vtx.tex3    (ind_cur), vtx.tex3    (ind_test), tex_wrap))continue;
                     if(flag&VTX_SIZE     && !Equal   (vtx.size    (ind_cur), vtx.size    (ind_test))          )continue;
                     if(flag&VTX_BLEND    &&           vtx.blend   (ind_cur)!=vtx.blend   (ind_test)           )continue;
                     if(flag&VTX_MATRIX   &&           vtx.matrix  (ind_cur)!=vtx.matrix  (ind_test)           )continue;
                     if(flag&VTX_MATERIAL &&           vtx.material(ind_cur)!=vtx.material(ind_test)           )continue;
                     if(flag&VTX_COLOR    &&           vtx.color   (ind_cur)!=vtx.color   (ind_test)           )continue;
                     if(flag&VTX_FLAG     &&           vtx.flag    (ind_cur)!=vtx.flag    (ind_test)           )continue;
                  }
                  if(smooth_groups_in_vtx_material)if(!(vtx.material(ind_cur).u&vtx.material(ind_test).u))continue; // if there are no shared smooth groups
                  vtx.dup(ind_cur)=ind_test; goto next; // set duplicate and continue
               }
            }
         }
         vtx.dup(ind_cur)=ind_cur; // set unique
         next:;
      }
   }
   return T;
}
/******************************************************************************/
void MeshBase::linkVtxVtxSamePos(Index &vtx_vtx, Flt pos_eps, Bool dual)C
{
   // prepare groups
   Memt<VecI2> temp_vtx_vtx;
   vtx_vtx.create(vtxs());

   // link box->vtx
   Boxes boxes  (Box(T)     , vtxs());
   Index box_vtx(boxes.num(), vtxs());
   REPA(vtx)box_vtx.elmGroup(boxes.index(vtx.pos(i)), i); box_vtx.set();

   // get duplicates
   Int xs=boxes.cells.x,
       ys=boxes.cells.y, xys=xs*ys;

   FREPD(z, boxes.cells.z)
   FREPD(y, boxes.cells.y)
   FREPD(x, boxes.cells.x)
   {
      Int         box_tests=0;
      IndexGroup *box_test[2+3+9], *box_cur=&box_vtx.group[x+y*xs+z*xys];
           box_test[box_tests++]=box_cur  ;
      if(x)box_test[box_tests++]=box_cur-1;
      if(y)
      {
         if(x<xs-1)box_test[box_tests++]=box_cur+1-xs;
                   box_test[box_tests++]=box_cur  -xs;
         if(     x)box_test[box_tests++]=box_cur-1-xs;
      }
      if(z)
      {
         if(y<ys-1)
         {
            if(x<xs-1)box_test[box_tests++]=box_cur+1+xs-xys;
                      box_test[box_tests++]=box_cur  +xs-xys;
            if(     x)box_test[box_tests++]=box_cur-1+xs-xys;
         }
            if(x<xs-1)box_test[box_tests++]=box_cur+1   -xys;
                      box_test[box_tests++]=box_cur     -xys;
            if(     x)box_test[box_tests++]=box_cur-1   -xys;
         if(y)
         {
            if(x<xs-1)box_test[box_tests++]=box_cur+1-xs-xys;
                      box_test[box_tests++]=box_cur  -xs-xys;
            if(     x)box_test[box_tests++]=box_cur-1-xs-xys;
         }
      }

      FREPA(*box_cur) // iterate all vertexes in current box
      {
         Int  ind_cur=(*box_cur)[i]; // vertex index
       C Vec &pos_cur=vtx.pos(ind_cur); // vertex position
         REPD(c, box_tests) // iterate all nearby boxes for testing
         {
            IndexGroup *bt=box_test[c]; // box for testing
            REPD(j, (box_cur==bt) ? i : bt->num) // iterate all vertexes in box for testing (but if it's current box then check only vertexes up to current vertex to make sure they were already processed before)
            {
               Int ind_test=(*bt)[j]; // test vertex index
               if(Equal(pos_cur, vtx.pos(ind_test), pos_eps)) // same position
               {
                           temp_vtx_vtx.New().set(ind_cur , ind_test); vtx_vtx.incGroup(ind_cur );
                  if(dual){temp_vtx_vtx.New().set(ind_test, ind_cur ); vtx_vtx.incGroup(ind_test);}
               }
            }
         }
      }
   }

   // add elms
   vtx_vtx.set();
   REPA(temp_vtx_vtx){C VecI2 &vv=temp_vtx_vtx[i]; vtx_vtx.addElm(vv.x, vv.y);}
}
/******************************************************************************/
void MeshBase::linkVtxVtxOnFace(Index &vtx_vtx)C
{
 C Int   *p;
 C VecI  *_tri =tri .ind();
 C VecI4 *_quad=quad.ind();

   // prepare groups
   vtx_vtx.create(vtxs());
   if(!vtx.dup())
   {
      REPA(tri ){p=(_tri ++)->c; REPD(j, 3)vtx_vtx.incGroup(*p++, 2);}
      REPA(quad){p=(_quad++)->c; REPD(j, 4)vtx_vtx.incGroup(*p++, 3);}
   }else
   {
      REPA(tri ){p=(_tri ++)->c; REPD(j, 3)vtx_vtx.incGroup(vtx.dup(*p++), 2);}
      REPA(quad){p=(_quad++)->c; REPD(j, 4)vtx_vtx.incGroup(vtx.dup(*p++), 3);}
   }
   vtx_vtx.set();

   // add elms
  _tri=tri.ind();
   REPA(tri)
   {
      VecI f=*_tri++; f.remap(vtx.dup());
      vtx_vtx.addElm(f.c[0], f.c[1]); vtx_vtx.addElm(f.c[0], f.c[2]);
      vtx_vtx.addElm(f.c[1], f.c[2]); vtx_vtx.addElm(f.c[1], f.c[0]);
      vtx_vtx.addElm(f.c[2], f.c[0]); vtx_vtx.addElm(f.c[2], f.c[1]);
   }
  _quad=quad.ind();
   REPA(quad)
   {
      VecI4 f=*_quad++; f.remap(vtx.dup());
      vtx_vtx.addElm(f.c[0], f.c[1]); vtx_vtx.addElm(f.c[0], f.c[2]); vtx_vtx.addElm(f.c[0], f.c[3]);
      vtx_vtx.addElm(f.c[1], f.c[2]); vtx_vtx.addElm(f.c[1], f.c[3]); vtx_vtx.addElm(f.c[1], f.c[0]);
      vtx_vtx.addElm(f.c[2], f.c[3]); vtx_vtx.addElm(f.c[2], f.c[0]); vtx_vtx.addElm(f.c[2], f.c[1]);
      vtx_vtx.addElm(f.c[3], f.c[0]); vtx_vtx.addElm(f.c[3], f.c[1]); vtx_vtx.addElm(f.c[3], f.c[2]);
   }

   // remove duplicates
   FREPA(vtx)
   {
      IndexGroup &ig=vtx_vtx.group[i];
      FREPAD(j, ig)
      {
         Int elm=ig[j];
         for(Int k=j+1; k<ig.num; )if(ig[k]==elm)ig.subElm(k);else k++;
      }
   }
}
void MeshBase::linkVtxVtxOnEdge(Index &vtx_vtx, Bool sort)C
{
 C Int *p;
   vtx_vtx.create(vtxs());
   FREPA(edge){p=edge.ind(i).c; vtx_vtx.incGroup(p[0]      ); vtx_vtx.incGroup(p[1]      );} vtx_vtx.set();
   FREPA(edge){p=edge.ind(i).c; vtx_vtx.addElm  (p[0], p[1]); vtx_vtx.addElm  (p[1], p[0]);}

   if(sort)
   {
    C Vec        *pos=vtx.pos();
      FloatIndex *fi =Alloc<FloatIndex>(vtx_vtx.group_elms_max);
      FREPA(vtx_vtx)
      {
         IndexGroup &ig    =vtx_vtx.group[i];
       C Vec2       &center=pos[i].xy;
         FREPA(ig)
         {
            Int vtx_index=ig[i];
            fi[i].f=AngleFast(pos[vtx_index].xy-center);
            fi[i].i=vtx_index;
         }
         Sort(fi, ig.num);
         FREPA(ig)ig[i]=fi[i].i;
      }
      Free(fi);
   }
}
/******************************************************************************/
void MeshBase::linkVtxEdge(Index &vtx_edge, Bool sort)C
{
 C Int *p;
   vtx_edge.create(vtxs());
   FREPA(edge){p=edge.ind(i).c; vtx_edge.incGroup(p[0]   ); vtx_edge.incGroup(p[1]   );} vtx_edge.set();
   FREPA(edge){p=edge.ind(i).c; vtx_edge.addElm  (p[0], i); vtx_edge.addElm  (p[1], i);}

   if(sort)
   {
    C Vec        *pos=vtx.pos();
      FloatIndex *fi =Alloc<FloatIndex>(vtx_edge.group_elms_max);
      FREPA(vtx_edge)
      {
         IndexGroup &ig    =vtx_edge.group[i];
       C Vec2       &center=pos[i].xy;
         FREPAD(e, ig)
         {
            Int edge_index=ig[e]; p=edge.ind(edge_index).c; Int vtx_index=p[0]; if(vtx_index==i)vtx_index=p[1];
            fi[e].f=AngleFast(pos[vtx_index].xy-center);
            fi[e].i=edge_index;
         }
         Sort(fi, ig.num);
         FREPA(ig)ig[i]=fi[i].i;
      }
      Free(fi);
   }
}
void MeshBase::linkVtxFace(Index &vtx_face)C
{
 C Int *p;
   vtx_face.create(vtxs());
   if(!vtx.dup())
   {
    C VecI  *_tri =tri .ind();  REPA(tri ){p=(_tri ++)->c; REPD(j, 3)vtx_face.incGroup(*p++            );}
    C VecI4 *_quad=quad.ind();  REPA(quad){p=(_quad++)->c; REPD(j, 4)vtx_face.incGroup(*p++            );} vtx_face.set();
             _tri =tri .ind(); FREPA(tri ){p=(_tri ++)->c; REPD(j, 3)vtx_face.addElm  (*p++, i         );}
             _quad=quad.ind(); FREPA(quad){p=(_quad++)->c; REPD(j, 4)vtx_face.addElm  (*p++, i^SIGN_BIT);}
   }else
   {
    C VecI  *_tri =tri .ind();  REPA(tri ){p=(_tri ++)->c; REPD(j, 3)vtx_face.incGroup(vtx.dup(*p++)            );}
    C VecI4 *_quad=quad.ind();  REPA(quad){p=(_quad++)->c; REPD(j, 4)vtx_face.incGroup(vtx.dup(*p++)            );} vtx_face.set();
             _tri =tri .ind(); FREPA(tri ){p=(_tri ++)->c; REPD(j, 3)vtx_face.addElm  (vtx.dup(*p++), i         );}
             _quad=quad.ind(); FREPA(quad){p=(_quad++)->c; REPD(j, 4)vtx_face.addElm  (vtx.dup(*p++), i^SIGN_BIT);}
   }
}
void MeshBase::linkFaceFace(Index &face_face)C
{
   face_face.create(faces());
   Index vtx_face; linkVtxFace(vtx_face);
   Memt<Int> face;

   REPAD(f, tri) // iterate all tris
   {
      VecI a=tri.ind(f); a.remap(vtx.dup()); REPA(a) // iterate all tri vtxs
      {
       C IndexGroup &ig=vtx_face.group[a.c[i]]; REPA(ig) // iterate all faces connected to the vtx
         {
            UInt test_face=ig[i];
            if(  test_face!=f)face.include(test_face); // face different than triangle 'f'
         }
      }
      face_face.incGroup(f, face.elms()); face.clear();
   }
   REPAD(f, quad) // iterate all quads
   {
      UInt fs=f^SIGN_BIT;
      VecI4 a=quad.ind(f); a.remap(vtx.dup()); REPA(a) // iterate all quad vtxs
      {
       C IndexGroup &ig=vtx_face.group[a.c[i]]; REPA(ig) // iterate all faces connected to the vtx
         {
            UInt test_face=ig[i];
            if(  test_face!=fs)face.include(test_face); // face different than quad 'f'
         }
      }
      face_face.incGroup(tris()+f, face.elms()); face.clear(); // add 'tris()' because we're processing quads and tris were listed first
   }

   face_face.set();

   REPAD(f, tri) // iterate all tris
   {
      VecI a=tri.ind(f); a.remap(vtx.dup()); REPA(a) // iterate all tri vtxs
      {
       C IndexGroup &ig=vtx_face.group[a.c[i]]; REPA(ig) // iterate all faces connected to the vtx
         {
            UInt test_face=ig[i];
            if(  test_face!=f)face.include(test_face); // face different than triangle 'f'
         }
      }
      IndexGroup &ig=face_face.group[f]; REPA(face)ig.add(face[i]); face.clear();
   }
   REPAD(f, quad) // iterate all quads
   {
      UInt fs=f^SIGN_BIT;
      VecI4 a=quad.ind(f); a.remap(vtx.dup()); REPA(a) // iterate all quad vtxs
      {
       C IndexGroup &ig=vtx_face.group[a.c[i]]; REPA(ig) // iterate all faces connected to the vtx
         {
            UInt test_face=ig[i];
            if(  test_face!=fs)face.include(test_face); // face different than quad 'f'
         }
      }
      IndexGroup &ig=face_face.group[tris()+f]; REPA(face)ig.add(face[i]); face.clear(); // add 'tris()' because we're processing quads and tris were listed first
   }
}
/******************************************************************************/
void MeshBase::linkEdgeFace()
{
   Index vtx_face; linkVtxFace(vtx_face);
   include(EDGE_ADJ_FACE);
   FREPAD(e, edge)
   {
      Int         r =-1, l=-1;
      VecI2       ep=edge.ind(e); ep.remap(vtx.dup());
      IndexGroup &vf=vtx_face.group[ep.c[0]];
      REPAD(f, vf)
      {
         Int tf=vf[f], side;
         if(tf&SIGN_BIT)
         {
            VecI4 fp=quad.ind(tf^SIGN_BIT); fp.remap(vtx.dup());
            side=GetSide(ep, fp);
         }else
         {
            VecI fp=tri.ind(tf); fp.remap(vtx.dup());
            side=GetSide(ep, fp);
         }
         if(side==SIDE_R)r=tf;else
         if(side==SIDE_L)l=tf;
      }
      edge.adjFace(e).set(r, l);
   }
}
/******************************************************************************/
void MeshBase::linkRectEdge(Index &rect_edge, C Rects &rects)C
{
   rect_edge.create(rects.num());
   FREPD(step, 2)
   {
      FREPA(edge)
      {
       C Int *p=edge.ind(i).c;
         for(PixelWalkerMask walker(rects.coords(vtx.pos(p[0]).xy), rects.coords(vtx.pos(p[1]).xy), RectI(0, 0, rects.cells.x-1, rects.cells.y-1)); walker.active(); walker.step())
         {
            Int group=rects.index(walker.pos());
            if(!step)rect_edge.incGroup(group);
            else     rect_edge.addElm  (group, i);
         }
      }
      if(!step)rect_edge.set();
   }
}
/******************************************************************************/
static void FillVtx(C Index &vtx_vtx, Memt<Bool> &is, Int vtx)
{
   Memt<Int> vtxs; if(!is[vtx]){is[vtx]=true; vtxs.add(vtx);}
   for(; vtxs.elms(); )
   {
      Int vtx=vtxs.pop();
      IndexGroup &ig=vtx_vtx.group[vtx]; REPA(ig){Int vtx=ig[i]; if(!is[vtx]){is[vtx]=true; vtxs.add(vtx);}}
   }
}
void MeshBase::getVtxNeighbors(Int vtx, MemPtr<Int> vtxs)C
{
   vtxs.clear(); if(InRange(vtx, T.vtx))
   {
      Index vtx_vtx; linkVtxVtxOnFace(vtx_vtx);
      Memt<Bool> is; is.setNumZero(T.vtxs());
      FillVtx(vtx_vtx, is, T.vtx.dup() ? T.vtx.dup(vtx) : vtx);
      FREPA(T.vtx)
      {
         Int v=i; if(T.vtx.dup())v=T.vtx.dup(v);
         if(is[v])vtxs.add(i);
      }
   }
}
void MeshBase::getFaceNeighbors(Int face, MemPtr<Int> faces)C
{
   faces.clear(); if((face&SIGN_BIT) ? InRange(face^SIGN_BIT, quad) : InRange(face, tri))
   {
      Index vtx_vtx; linkVtxVtxOnFace(vtx_vtx);
      Memt<Bool> is; is.setNumZero(T.vtxs());
      if(face&SIGN_BIT){VecI4 f=quad.ind(face^SIGN_BIT); f.remap(vtx.dup()); REPA(f)FillVtx(vtx_vtx, is, f.c[i]);}
      else             {VecI  f=tri .ind(face         ); f.remap(vtx.dup()); REPA(f)FillVtx(vtx_vtx, is, f.c[i]);}
      REPA(tri ){VecI  f=tri .ind(i); f.remap(vtx.dup()); if(is[f.x] || is[f.y] || is[f.z]           )faces.add(i         );}
      REPA(quad){VecI4 f=quad.ind(i); f.remap(vtx.dup()); if(is[f.x] || is[f.y] || is[f.z] || is[f.w])faces.add(i^SIGN_BIT);}
   }
}
/******************************************************************************/
struct EdgeAdj
{
   VecI2 ind, adj_face;

   EdgeAdj& set(Int p0, Int p1, Int f0, Int f1) {ind.set(p0, p1); adj_face.set(f0, f1); return T;}
};
struct Adj
{
   Int face,           // face index
       vtxi,           // index of vertex place in the face
       face_extra_vtx; // index of vertex which is loose

   void set(Int face, Int vtxi, Int face_extra_vtx) {T.face=face; T.vtxi=vtxi; T.face_extra_vtx=face_extra_vtx;}
};
MeshBase& MeshBase::setAdjacencies(Bool faces, Bool edges)
{
   if(faces || edges)
   {
      include((faces ? FACE_ADJ_FACE : MESH_NONE) | (edges ? FACE_ADJ_EDGE : MESH_NONE));
      if(faces)
      {
         SetMemN(tri .adjFace(), 0xFF, tris ());
         SetMemN(quad.adjFace(), 0xFF, quads());
      }
      if(edges)
      {
         SetMemN(tri .adjEdge(), 0xFF, tris ());
         SetMemN(quad.adjEdge(), 0xFF, quads());
      }

      // link vtx->face
      Index vtx_face; linkVtxFace(vtx_face);

      // add double sided edges
      Memb<EdgeAdj> _edge(1024);
   #if 0 // simple version, generates duplicates of edges, doesn't do any sorting if there are multiple face connections (for example 3 faces like "T")
      do not use, we need sorting, for example for 'removeDoubleSideFaces'
      REPAD(f0, tri) // for each triangle
      {
         VecI f0i=tri.ind(f0); f0i.remap(vtx.dup());
         REPD(f0vi, 3) // for each triangle vertex
         {
            Int f0v0=f0i.c[ f0vi     ],
                f0v1=f0i.c[(f0vi+1)%3];
            IndexGroup &ig=vtx_face.group[f0v0];
            REPAD(vfi, ig) // for each face that the vertex belongs to
            {
               Int f1=ig[vfi]; VecI4 f1i; Int f1vtxs;
               if( f1&SIGN_BIT) // quad
               {
                  f1i=quad.ind(f1^SIGN_BIT); f1i.remap(vtx.dup()); f1vtxs=4;
               }else
               {
                  if(f1>=f0)continue; // if the other triangle has a greater index (or is the same one) then skip, because we will check this again later for the other triangle
                  f1i.xyz=tri.ind(f1); f1i.xyz.remap(vtx.dup()); f1vtxs=3;
               }
               REPD(f1vi, f1vtxs) // for each face vertex
                  if(f0v1==f1i.c[ f1vi          ]  // here f0v1==f1v0 (order of vertexes is swapped)
                  && f0v0==f1i.c[(f1vi+1)%f1vtxs]) // here f0v0==f1v1
               {
                  if(faces){tri.adjFace(f0).c[f0vi]=f1          ; ((f1&SIGN_BIT) ? quad.adjFace(f1^SIGN_BIT).c[f1vi] : tri.adjFace(f1).c[f1vi])=f0;}
                  if(edges){tri.adjEdge(f0).c[f0vi]=_edge.elms(); ((f1&SIGN_BIT) ? quad.adjEdge(f1^SIGN_BIT).c[f1vi] : tri.adjEdge(f1).c[f1vi])=_edge.elms(); _edge.New().set(f0v0, f0v1, f0, f1);}
                  break;
               }
            }
         }
      }

      REPAD(f0, quad) // for each quad
      {
         VecI4 f0i=quad.ind(f0); f0i.remap(vtx.dup());
         REPD(f0vi, 4) // for each quad vertex
         {
            Int f0v0=f0i.c[ f0vi     ],
                f0v1=f0i.c[(f0vi+1)%4];
            IndexGroup &ig=vtx_face.group[f0v0];
            REPAD(vfi, ig) // for each face that the vertex belongs to
            {
               // in this case we will process only quads, as we've already processed triangle-quad pairs above
               Int f1=ig[vfi];
               if( f1&SIGN_BIT) // quad
               {
                  f1^=SIGN_BIT;
                  if(f1<f0) // here process only quads that have index smaller than 'f0', because we will check this again later for the other quad
                  {
                     VecI4 f1i=quad.ind(f1); f1i.remap(vtx.dup());
                     REPD(f1vi, 4) // for each quad vertex
                        if(f0v1==f1i.c[ f1vi     ]  // here f0v1==f1v0 (order of vertexes is swapped)
                        && f0v0==f1i.c[(f1vi+1)%4]) // here f0v0==f1v1
                     {
                        if(faces){quad.adjFace(f0).c[f0vi]=f1^SIGN_BIT ; quad.adjFace(f1).c[f1vi]=f0^SIGN_BIT;}
                        if(edges){quad.adjEdge(f0).c[f0vi]=_edge.elms(); quad.adjEdge(f1).c[f1vi]=_edge.elms(); _edge.New().set(f0v0, f0v1, f0^SIGN_BIT, f1^SIGN_BIT);}
                        break;
                     }
                  }
               }//else quad with tri pairs were already checked before
            }
         }
      }
   #else
      Memt<Adj> adj;
      REPAD(f0, tri) // for each triangle
      {
         VecI f0a=(faces ? tri.adjFace(f0) : tri.adjEdge(f0)); // get current f0 face adjacency (this can be either face or edge, because we're only checking if it's not set yet by comparing to -1)
         VecI f0i=tri.ind(f0); f0i.remap(vtx.dup());
         REPD(f0vi, 3) // for each triangle edge
            if(f0a.c[f0vi]==-1) // if not yet connected to something
         {
            Int f0v0=f0i.c[ f0vi     ], // edge first  vertex
                f0v1=f0i.c[(f0vi+1)%3]; // edge second vertex
            IndexGroup &ig=vtx_face.group[f0v0];
            REPAD(vfi, ig) // for each face that edge first vertex belongs to
            {
               Int f1=ig[vfi];
               if( f1&SIGN_BIT) // quad
               {
                  VecI4 f1a=(faces ? quad.adjFace(f1^SIGN_BIT) : quad.adjEdge(f1^SIGN_BIT)); // get current f1 face adjacency (this can be either face or edge, because we're only checking if it's not set yet by comparing to -1)
                  VecI4 f1i=quad.ind(f1^SIGN_BIT); f1i.remap(vtx.dup());
                  if(f1i.c[0]==f0v1 && f1i.c[1]==f0v0){if(f1a.c[0]==-1)adj.New().set(f1, 0, f1i.c[2]);}else // add adjacency to other face only if it's not yet connected to anything !! THIS IS IMPORTANT AS WITHOUT THIS DIRECTX MESH OPTIMIZER MAY CRASH !!
                  if(f1i.c[1]==f0v1 && f1i.c[2]==f0v0){if(f1a.c[1]==-1)adj.New().set(f1, 1, f1i.c[3]);}else
                  if(f1i.c[2]==f0v1 && f1i.c[3]==f0v0){if(f1a.c[2]==-1)adj.New().set(f1, 2, f1i.c[0]);}else
                  if(f1i.c[3]==f0v1 && f1i.c[0]==f0v0){if(f1a.c[3]==-1)adj.New().set(f1, 3, f1i.c[1]);}
               }else
               {
                  VecI f1a=(faces ? tri.adjFace(f1) : tri.adjEdge(f1)); // get current f1 face adjacency (this can be either face or edge, because we're only checking if it's not set yet by comparing to -1)
                  VecI f1i=tri.ind(f1); f1i.remap(vtx.dup());
                  if(f1i.c[0]==f0v1 && f1i.c[1]==f0v0){if(f1a.c[0]==-1)adj.New().set(f1, 0, f1i.c[2]);}else // add adjacency to other face only if it's not yet connected to anything !! THIS IS IMPORTANT AS WITHOUT THIS DIRECTX MESH OPTIMIZER MAY CRASH !!
                  if(f1i.c[1]==f0v1 && f1i.c[2]==f0v0){if(f1a.c[1]==-1)adj.New().set(f1, 1, f1i.c[0]);}else
                  if(f1i.c[2]==f0v1 && f1i.c[0]==f0v0){if(f1a.c[2]==-1)adj.New().set(f1, 2, f1i.c[1]);}
               }
            }
            if(adj.elms())
            {
               Int adj_i=0;
               if( adj.elms()>1 && vtx.pos()) // if the edge links many faces (for example 3 faces like "T")
               {
                  Matrix m; m.setPosDir(vtx.pos(f0v0), !(vtx.pos(f0v1)-vtx.pos(f0v0))); // construct matrix with pos on the edge first vertex and dir along the edge
                  Flt    angle=FLT_MAX, // set as max possible value so any first test will pass
                         a0=AngleFast(vtx.pos(f0i.c[(f0vi+2)%3]), m); // calculate angle of the loose vertex on the 'f0' triangle and set it as base/zero angle
                  REPA(adj) // iterate adjacent faces
                  {
                     Flt a=AngleFast(vtx.pos(adj[i].face_extra_vtx), m);
                     if( a<=a0   )a+=PI2; // use <= to include coplanar faces
                     if( a< angle){adj_i=i; angle=a;} // find face with smallest angle difference from 'a0'
                  }
               }
               Adj &a   =adj[adj_i];
               Int  f1  =a.face;
               Int  f1vi=a.vtxi;
               if(faces){tri.adjFace(f0).c[f0vi]=f1          ; ((f1&SIGN_BIT) ? quad.adjFace(f1^SIGN_BIT).c[f1vi] : tri.adjFace(f1).c[f1vi])=f0;}
               if(edges){tri.adjEdge(f0).c[f0vi]=_edge.elms(); ((f1&SIGN_BIT) ? quad.adjEdge(f1^SIGN_BIT).c[f1vi] : tri.adjEdge(f1).c[f1vi])=_edge.elms(); _edge.New().set(f0v0, f0v1, f0, f1);}
               adj.clear();
            }
         }
      }

      REPAD(f0, quad) // for each quad
      {
         VecI4 f0a=(faces ? quad.adjFace(f0) : quad.adjEdge(f0)); // get current f0 face adjacency (this can be either face or edge, because we're only checking if it's not set yet by comparing to -1)
         VecI4 f0i=quad.ind(f0); f0i.remap(vtx.dup());
         REPD(f0vi, 4) // for each quad edge
            if(f0a.c[f0vi]==-1) // if not yet connected to something
         {
            Int f0v0=f0i.c[ f0vi     ], // edge first  vertex
                f0v1=f0i.c[(f0vi+1)%4]; // edge second vertex
            IndexGroup &ig=vtx_face.group[f0v0];
            REPAD(vfi, ig) // for each face that edge first vertex belongs to
            {
               Int f1=ig[vfi];
               if( f1&SIGN_BIT) // quad
               {
                  VecI4 f1a=(faces ? quad.adjFace(f1^SIGN_BIT) : quad.adjEdge(f1^SIGN_BIT)); // get current f1 face adjacency (this can be either face or edge, because we're only checking if it's not set yet by comparing to -1)
                  VecI4 f1i=quad.ind(f1^SIGN_BIT); f1i.remap(vtx.dup());
                  if(f1i.c[0]==f0v1 && f1i.c[1]==f0v0){if(f1a.c[0]==-1)adj.New().set(f1, 0, f1i.c[2]);}else // add adjacency to other face only if it's not yet connected to anything !! THIS IS IMPORTANT AS WITHOUT THIS DIRECTX MESH OPTIMIZER MAY CRASH !!
                  if(f1i.c[1]==f0v1 && f1i.c[2]==f0v0){if(f1a.c[1]==-1)adj.New().set(f1, 1, f1i.c[3]);}else
                  if(f1i.c[2]==f0v1 && f1i.c[3]==f0v0){if(f1a.c[2]==-1)adj.New().set(f1, 2, f1i.c[0]);}else
                  if(f1i.c[3]==f0v1 && f1i.c[0]==f0v0){if(f1a.c[3]==-1)adj.New().set(f1, 3, f1i.c[1]);}
               }else
               {
                  VecI f1a=(faces ? tri.adjFace(f1) : tri.adjEdge(f1)); // get current f1 face adjacency (this can be either face or edge, because we're only checking if it's not set yet by comparing to -1)
                  VecI f1i=tri.ind(f1); f1i.remap(vtx.dup());
                  if(f1i.c[0]==f0v1 && f1i.c[1]==f0v0){if(f1a.c[0]==-1)adj.New().set(f1, 0, f1i.c[2]);}else // add adjacency to other face only if it's not yet connected to anything !! THIS IS IMPORTANT AS WITHOUT THIS DIRECTX MESH OPTIMIZER MAY CRASH !!
                  if(f1i.c[1]==f0v1 && f1i.c[2]==f0v0){if(f1a.c[1]==-1)adj.New().set(f1, 1, f1i.c[0]);}else
                  if(f1i.c[2]==f0v1 && f1i.c[0]==f0v0){if(f1a.c[2]==-1)adj.New().set(f1, 2, f1i.c[1]);}
               }
            }
            if(adj.elms())
            {
               Int adj_i=0;
               if( adj.elms()>1 && vtx.pos()) // if the edge links many faces (for example 3 faces like "T")
               {
                  Matrix m; m.setPosDir(vtx.pos(f0v0), !(vtx.pos(f0v1)-vtx.pos(f0v0))); // construct matrix with pos on the edge first vertex and dir along the edge
                  Flt    angle=FLT_MAX, // set as max possible value so any first test will pass
                         a0=AngleFast(vtx.pos(f0i.c[(f0vi+2)%4]), m); // calculate angle of the loose vertex on the 'f0' quad and set it as base/zero angle
                  REPA(adj) // iterate adjacent faces
                  {
                     Flt a=AngleFast(vtx.pos(adj[i].face_extra_vtx), m);
                     if( a<=a0   )a+=PI2; // use <= to include coplanar faces
                     if( a< angle){adj_i=i; angle=a;} // find face with smallest angle difference from 'a0'
                  }
               }
               Adj &a   =adj[adj_i];
               Int  f1  =a.face;
               Int  f1vi=a.vtxi;
               if(faces){quad.adjFace(f0).c[f0vi]=f1          ; ((f1&SIGN_BIT) ? quad.adjFace(f1^SIGN_BIT).c[f1vi] : tri.adjFace(f1).c[f1vi])=f0^SIGN_BIT;}
               if(edges){quad.adjEdge(f0).c[f0vi]=_edge.elms(); ((f1&SIGN_BIT) ? quad.adjEdge(f1^SIGN_BIT).c[f1vi] : tri.adjEdge(f1).c[f1vi])=_edge.elms(); _edge.New().set(f0v0, f0v1, f0^SIGN_BIT, f1);}
               adj.clear();
            }
         }
      }
   #endif

      if(edges)
      {
         // add one sided edges
         FREPA(tri)
         {
            Int *p=tri.adjEdge(i).c;
            REPD(j, 3)if(p[j]<0) // if any triangle edge didn't get linked to an edge, then it means it's empty (one-sided)
            {
               Int *v=tri.ind(i).c;
               p[j]=_edge.elms();
                    _edge.New().set(v[j], v[(j+1)%3], i, -1).ind.remap(vtx.dup());
            }
         }
         FREPA(quad)
         {
            Int *p=quad.adjEdge(i).c;
            REPD(j, 4)if(p[j]<0) // if any quad edge didn't get linked to an edge, then it means it's empty (one-sided)
            {
               Int *v=quad.ind(i).c;
               p[j]=_edge.elms();
                    _edge.New().set(v[j], v[(j+1)%4], i^SIGN_BIT, -1).ind.remap(vtx.dup());
            }
         }

         // set edges
         exclude(EDGE_ALL); edge._elms=_edge.elms();
         include(EDGE_IND|EDGE_ADJ_FACE);
         FREPA(edge)
         {
            edge.ind    (i)=_edge[i].ind;
            edge.adjFace(i)=_edge[i].adj_face;
         }
      }
   }
   return T;
}
/******************************************************************************/
}
/******************************************************************************/
