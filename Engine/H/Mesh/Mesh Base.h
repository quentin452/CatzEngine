/******************************************************************************

   'MeshBase' is the most low level software mesh.
      It contains vertexes, edges, triangles and quads.

/******************************************************************************/
enum VTX_FLAGS // flags of a single vertex 'MeshVtxs.flag'
{
   VTX_FLAG_CLOTH=0x1, // when converting MeshBase to ClothMesh then use the vertex in physical cloth creation
};
enum ETQ_FLAG // Edge-Triangle-Quad Flag
{
   ETQ_L      =0x01, // has solid left  side
   ETQ_R      =0x02, // has solid right side
   ETQ_NO_PHYS=0x04, // no  physical body will be created from this face

   ETQ_LR=ETQ_L|ETQ_R,
};
enum MESH_SIMPLIFY : Byte
{
   SIMPLIFY_QUADRIC, // vertex distances are calculated based on quadric matrixes
   SIMPLIFY_PLANES , // vertex distances are calculated based on planes (requires more memory)
};
enum MESH_AO_FUNC : Byte
{
   MAF_FULL      , // 0
   MAF_QUARTIC   , // Quart(x)
   MAF_SQUARE    , //   Sqr(x)
   MAF_LINEAR    , //       x
   MAF_LINEAR_REV, // 2-2/(x+1)
   MAF_SQUARE_REV, // 1-Sqr(1-x)
};
enum MESH_FLAG : ULong // Mesh Flag
{
   VTX_POS     =1ull<< 0, // vertex position
   VTX_NRM     =1ull<< 1, // vertex normal
   VTX_TAN     =1ull<< 2, // vertex tangent
   VTX_BIN     =1ull<< 3, // vertex binormal
   VTX_HLP     =1ull<< 4, // vertex helper
   VTX_TEX0    =1ull<< 5, // vertex texture UV 0
   VTX_TEX1    =1ull<< 6, // vertex texture UV 1
   VTX_TEX2    =1ull<< 7, // vertex texture UV 2
   VTX_TEX3    =1ull<< 8, // vertex texture UV 3
   VTX_COLOR   =1ull<< 9, // vertex color
   VTX_MATERIAL=1ull<<10, // vertex material blend
   VTX_MATRIX  =1ull<<11, // vertex matrix index
   VTX_BLEND   =1ull<<12, // vertex matrix blend
   VTX_SIZE    =1ull<<13, // vertex size
   VTX_FLAG    =1ull<<14, // vertex flag
   VTX_DUP     =1ull<<15, // vertex duplicate

   EDGE_IND     =1ull<<16, // edge vertex index
   EDGE_ADJ_FACE=1ull<<17, // edge adjacent face
   EDGE_NRM     =1ull<<18, // edge normal
   EDGE_FLAG    =1ull<<19, // edge flag
   EDGE_ID      =1ull<<20, // edge ID

   TRI_IND     =1ull<<21, // triangle vertex index
   TRI_ADJ_FACE=1ull<<22, // triangle adjacent face
   TRI_ADJ_EDGE=1ull<<23, // triangle adjacent edge
   TRI_NRM     =1ull<<24, // triangle normal
   TRI_FLAG    =1ull<<25, // triangle flag
   TRI_ID      =1ull<<26, // triangle ID

   QUAD_IND     =1ull<<27, // quad vertex index
   QUAD_ADJ_FACE=1ull<<28, // quad adjacent face
   QUAD_ADJ_EDGE=1ull<<29, // quad adjacent edge
   QUAD_NRM     =1ull<<30, // quad normal
   QUAD_FLAG    =1ull<<31, // quad flag
   QUAD_ID      =1ull<<32, // quad ID

   VTX_TEX_ALL    =VTX_TEX0  |VTX_TEX1|VTX_TEX2|VTX_TEX3,
   VTX_TAN_BIN    =VTX_TAN   |VTX_BIN                   ,
   VTX_NRM_TAN_BIN=VTX_NRM   |VTX_TAN_BIN               ,
   VTX_SKIN       =VTX_MATRIX|VTX_BLEND                 ,

   FACE_IND     =TRI_IND     |QUAD_IND     ,
   FACE_ADJ_FACE=TRI_ADJ_FACE|QUAD_ADJ_FACE,
   FACE_ADJ_EDGE=TRI_ADJ_EDGE|QUAD_ADJ_EDGE,
   FACE_NRM     =TRI_NRM     |QUAD_NRM     ,
   FACE_FLAG    =TRI_FLAG    |QUAD_FLAG    ,
   FACE_ID      =TRI_ID      |QUAD_ID      ,

    VTX_ALL= VTX_POS|VTX_NRM_TAN_BIN|VTX_HLP|VTX_TEX_ALL|VTX_COLOR|VTX_MATERIAL|VTX_MATRIX|VTX_BLEND|VTX_SIZE|VTX_DUP|VTX_FLAG,
   EDGE_ALL=EDGE_IND|EDGE_ADJ_FACE|              EDGE_NRM|EDGE_FLAG|EDGE_ID,
    TRI_ALL= TRI_IND| TRI_ADJ_FACE| TRI_ADJ_EDGE| TRI_NRM| TRI_FLAG| TRI_ID,
   QUAD_ALL=QUAD_IND|QUAD_ADJ_FACE|QUAD_ADJ_EDGE|QUAD_NRM|QUAD_FLAG|QUAD_ID,
   FACE_ALL=FACE_IND|FACE_ADJ_FACE|FACE_ADJ_EDGE|FACE_NRM|FACE_FLAG|FACE_ID,
    ADJ_ALL=EDGE_ADJ_FACE|TRI_ADJ_FACE|TRI_ADJ_EDGE|QUAD_ADJ_FACE|QUAD_ADJ_EDGE,
    NRM_ALL= VTX_NRM |EDGE_NRM |TRI_NRM |QUAD_NRM ,
   FLAG_ALL= VTX_FLAG|EDGE_FLAG|TRI_FLAG|QUAD_FLAG,
     ID_ALL=          EDGE_ID  |TRI_ID  |QUAD_ID  ,

   MESH_NONE= 0,
   MESH_ALL =~0ull,

#if EE_PRIVATE
   VTX_MSHR=VTX_POS|VTX_NRM_TAN_BIN|VTX_HLP|VTX_TEX_ALL|VTX_COLOR|VTX_MATERIAL|VTX_MATRIX|VTX_BLEND|VTX_SIZE, // vertex values which can be stored in MeshRender
#endif
};
SET_ENUM_FLAGS(MESH_FLAG);
/******************************************************************************/
void SetSkin(C MemPtrN<IndexWeight, 256> &skin, VtxBone &matrix, VecB4 &blend, C Skeleton *skeleton); // set 'matrix' and 'blend' skinning values from 'skin' and 'skeleton' (optional)
/******************************************************************************/
#if EE_PRIVATE
void FixMatrixWeight(VecB4  &matrix, VecB4 &blend);
void FixMatrixWeight(VecUS4 &matrix, VecB4 &blend);

struct VtxDup
{
   Vec pos;
   Int dup;
};
struct VtxDupNrm : VtxDup
{
   Vec nrm;
};
Int SetVtxDup(MemPtr<VtxDup   > vtxs, C Box &box, Flt pos_eps=EPS                         ); // returns the number of unique positions
Int SetVtxDup(MemPtr<VtxDupNrm> vtxs, C Box &box, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS); // returns the number of unique positions
/******************************************************************************/
enum SIDE_TYPE
{
   SIDE_NONE=-1,
   SIDE_L   = 0,
   SIDE_R   = 1,
};
SIDE_TYPE GetSide(C VecI2 &edge, C VecI  &tri );
SIDE_TYPE GetSide(C VecI2 &edge, C VecI4 &quad);
/******************************************************************************/
UInt EtqFlagSwap(UInt flag); // swap sides of ETQ_FLAG
#endif
/******************************************************************************/
struct VtxFull // Vertex containing all possible data
{
   Vec     pos, nrm, tan, bin, hlp;
   Vec2    tex0, tex1, tex2, tex3;
   Color   color;
   VecB4   material;
   VtxBone matrix;
   VecB4   blend;
   Flt     size;

   void reset(); // reset vertex values, this sets all members to zero, except 'color' which is set to WHITE

   VtxFull& from(C MeshBase &mshb, Int i) ; // set VtxFull from i-th vertex in 'mshb'
   void     to  (  MeshBase &mshb, Int i)C; // set i-th 'mshb' vertex from VtxFull

   VtxFull& avg(C VtxFull &a, C VtxFull &b                            ); // set as average from vertexes, T=Avg(a, b)
   VtxFull& avg(C VtxFull &a, C VtxFull &b, C VtxFull &c              ); // set as average from vertexes, T=Avg(a, b, c)
   VtxFull& avg(C VtxFull &a, C VtxFull &b, C VtxFull &c, C VtxFull &d); // set as average from vertexes, T=Avg(a, b, c, d)

   VtxFull& lerp(C VtxFull &a, C VtxFull &b,                 Flt  step ); // set as linear interpolation from vertexes, T=Lerp(a, b, step)
   VtxFull& lerp(C VtxFull &a, C VtxFull &b, C VtxFull &c, C Vec &blend); // set as linear interpolation from vertexes, T=a*blend.x + b*blend.y + c*blend.z

   VtxFull& mul(C Matrix &matrix, C Matrix3 &matrix3); // transform vertex by matrixes ('pos hlp' by 'matrix', and 'nrm tan bin' by 'matrix3')
};
/******************************************************************************/
struct MeshVtxs // Mesh Vertexes
{
   Int      elms    ()C {return _elms    ;}                                                                                  // get number of vertexes
   Vec    * pos     ()  {return _pos     ;}     Vec    & pos     (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _pos     [i];} // get i-th vertex position
 C Vec    * pos     ()C {return _pos     ;}   C Vec    & pos     (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _pos     [i];} // get i-th vertex position
   Vec    * nrm     ()  {return _nrm     ;}     Vec    & nrm     (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _nrm     [i];} // get i-th vertex normal
 C Vec    * nrm     ()C {return _nrm     ;}   C Vec    & nrm     (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _nrm     [i];} // get i-th vertex normal
   Vec    * tan     ()  {return _tan     ;}     Vec    & tan     (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _tan     [i];} // get i-th vertex tangent
 C Vec    * tan     ()C {return _tan     ;}   C Vec    & tan     (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _tan     [i];} // get i-th vertex tangent
   Vec    * bin     ()  {return _bin     ;}     Vec    & bin     (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _bin     [i];} // get i-th vertex binormal
 C Vec    * bin     ()C {return _bin     ;}   C Vec    & bin     (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _bin     [i];} // get i-th vertex binormal
   Vec    * hlp     ()  {return _hlp     ;}     Vec    & hlp     (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _hlp     [i];} // get i-th vertex helper
 C Vec    * hlp     ()C {return _hlp     ;}   C Vec    & hlp     (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _hlp     [i];} // get i-th vertex helper
   Vec2   * tex0    ()  {return _tex0    ;}     Vec2   & tex0    (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _tex0    [i];} // get i-th vertex texture UV 0
 C Vec2   * tex0    ()C {return _tex0    ;}   C Vec2   & tex0    (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _tex0    [i];} // get i-th vertex texture UV 0
   Vec2   * tex1    ()  {return _tex1    ;}     Vec2   & tex1    (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _tex1    [i];} // get i-th vertex texture UV 1
 C Vec2   * tex1    ()C {return _tex1    ;}   C Vec2   & tex1    (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _tex1    [i];} // get i-th vertex texture UV 1
   Vec2   * tex2    ()  {return _tex2    ;}     Vec2   & tex2    (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _tex2    [i];} // get i-th vertex texture UV 2
 C Vec2   * tex2    ()C {return _tex2    ;}   C Vec2   & tex2    (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _tex2    [i];} // get i-th vertex texture UV 2
   Vec2   * tex3    ()  {return _tex3    ;}     Vec2   & tex3    (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _tex3    [i];} // get i-th vertex texture UV 3
 C Vec2   * tex3    ()C {return _tex3    ;}   C Vec2   & tex3    (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _tex3    [i];} // get i-th vertex texture UV 3
   Color  * color   ()  {return _color   ;}     Color  & color   (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _color   [i];} // get i-th vertex color
 C Color  * color   ()C {return _color   ;}   C Color  & color   (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _color   [i];} // get i-th vertex color
   VecB4  * material()  {return _material;}     VecB4  & material(Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _material[i];} // get i-th vertex material blend
 C VecB4  * material()C {return _material;}   C VecB4  & material(Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _material[i];} // get i-th vertex material blend
   VtxBone* matrix  ()  {return _matrix  ;}     VtxBone& matrix  (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _matrix  [i];} // get i-th vertex bone/matrix index , index maps to a specific bone of a skeleton, where 0=skeleton root bone, 1=skeleton bone #0, 2=skeleton bone #1, 3=skeleton bone #2, and so on, this can be summarized in the following : bone = (index ? skeleton.bone[index-1] : skeleton.root)
 C VtxBone* matrix  ()C {return _matrix  ;}   C VtxBone& matrix  (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _matrix  [i];} // get i-th vertex bone/matrix index , index maps to a specific bone of a skeleton, where 0=skeleton root bone, 1=skeleton bone #0, 2=skeleton bone #1, 3=skeleton bone #2, and so on, this can be summarized in the following : bone = (index ? skeleton.bone[index-1] : skeleton.root)
   VecB4  * blend   ()  {return _blend   ;}     VecB4  & blend   (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _blend   [i];} // get i-th vertex bone/matrix weight, weight factor between corresponding matrix.xyzw bones, their sum must be equal to 255 !!
 C VecB4  * blend   ()C {return _blend   ;}   C VecB4  & blend   (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _blend   [i];} // get i-th vertex bone/matrix weight, weight factor between corresponding matrix.xyzw bones, their sum must be equal to 255 !!
   Flt    * size    ()  {return _size    ;}     Flt    & size    (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _size    [i];} // get i-th vertex size
 C Flt    * size    ()C {return _size    ;}   C Flt    & size    (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _size    [i];} // get i-th vertex size
   Byte   * flag    ()  {return _flag    ;}     Byte   & flag    (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _flag    [i];} // get i-th vertex flag
 C Byte   * flag    ()C {return _flag    ;}   C Byte   & flag    (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _flag    [i];} // get i-th vertex flag
   Int    * dup     ()  {return _dup     ;}     Int    & dup     (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _dup     [i];} // get i-th vertex duplicate, index of identical vertex
 C Int    * dup     ()C {return _dup     ;}   C Int    & dup     (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _dup     [i];} // get i-th vertex duplicate, index of identical vertex

   MeshVtxs() {}
#if !EE_PRIVATE
private:
#endif
   Int _elms; Vec *_pos, *_nrm, *_tan, *_bin, *_hlp; Vec2 *_tex0, *_tex1, *_tex2, *_tex3; Color *_color; VecB4 *_material; VtxBone *_matrix; VecB4 *_blend; Flt *_size; Byte *_flag; Int *_dup;
   NO_COPY_CONSTRUCTOR(MeshVtxs);
};
/******************************************************************************/
struct MeshEdges // Mesh Edges
{
   Int    elms   ()C {return _elms    ;}                                                                                 // get number of edges
   VecI2* ind    ()  {return _ind     ;}     VecI2& ind    (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _ind     [i];} // get i-th edge vertex indexes
 C VecI2* ind    ()C {return _ind     ;}   C VecI2& ind    (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _ind     [i];} // get i-th edge vertex indexes
   VecI2* adjFace()  {return _adj_face;}     VecI2& adjFace(Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _adj_face[i];} // get i-th edge adjacent faces, index to faces adjacent to the edge encoded in following way : if(adj_face==-1) -> no face, else if(adj_face&SIGN_BIT)adj_quad_index=adj_face^SIGN_BIT, else adj_tri_index=adj_face
 C VecI2* adjFace()C {return _adj_face;}   C VecI2& adjFace(Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _adj_face[i];} // get i-th edge adjacent faces, index to faces adjacent to the edge encoded in following way : if(adj_face==-1) -> no face, else if(adj_face&SIGN_BIT)adj_quad_index=adj_face^SIGN_BIT, else adj_tri_index=adj_face
   Vec  * nrm    ()  {return _nrm     ;}     Vec  & nrm    (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _nrm     [i];} // get i-th edge normal
 C Vec  * nrm    ()C {return _nrm     ;}   C Vec  & nrm    (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _nrm     [i];} // get i-th edge normal
   Byte * flag   ()  {return _flag    ;}     Byte & flag   (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _flag    [i];} // get i-th edge flag
 C Byte * flag   ()C {return _flag    ;}   C Byte & flag   (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _flag    [i];} // get i-th edge flag
   Int  * id     ()  {return _id      ;}     Int  & id     (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _id      [i];} // get i-th edge ID
 C Int  * id     ()C {return _id      ;}   C Int  & id     (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _id      [i];} // get i-th edge ID

   MeshEdges() {}
#if !EE_PRIVATE
private:
#endif
   Int _elms; VecI2 *_ind, *_adj_face; Vec *_nrm; Byte *_flag; Int *_id;
   NO_COPY_CONSTRUCTOR(MeshEdges);
};
/******************************************************************************/
struct MeshTris // Mesh Triangles
{
   Int   elms   ()C {return _elms    ;}                                                                                // get number of triangles
   VecI* ind    ()  {return _ind     ;}     VecI& ind    (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _ind     [i];} // get i-th triangle vertex indexes
 C VecI* ind    ()C {return _ind     ;}   C VecI& ind    (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _ind     [i];} // get i-th triangle vertex indexes
   VecI* adjFace()  {return _adj_face;}     VecI& adjFace(Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _adj_face[i];} // get i-th triangle adjacent faces, index to faces adjacent to the triangle encoded in following way : if(adj_face==-1) -> no face, else if(adj_face&SIGN_BIT)adj_quad_index=adj_face^SIGN_BIT, else adj_tri_index=adj_face
 C VecI* adjFace()C {return _adj_face;}   C VecI& adjFace(Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _adj_face[i];} // get i-th triangle adjacent faces, index to faces adjacent to the triangle encoded in following way : if(adj_face==-1) -> no face, else if(adj_face&SIGN_BIT)adj_quad_index=adj_face^SIGN_BIT, else adj_tri_index=adj_face
   VecI* adjEdge()  {return _adj_edge;}     VecI& adjEdge(Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _adj_edge[i];} // get i-th triangle adjacent edges, index to edges adjacent to the triangle encoded in following way : if(adj_edge==-1) -> no edge, else adj_edge_index=adj_edge
 C VecI* adjEdge()C {return _adj_edge;}   C VecI& adjEdge(Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _adj_edge[i];} // get i-th triangle adjacent edges, index to edges adjacent to the triangle encoded in following way : if(adj_edge==-1) -> no edge, else adj_edge_index=adj_edge
   Vec * nrm    ()  {return _nrm     ;}     Vec & nrm    (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _nrm     [i];} // get i-th triangle normal
 C Vec * nrm    ()C {return _nrm     ;}   C Vec & nrm    (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _nrm     [i];} // get i-th triangle normal
   Byte* flag   ()  {return _flag    ;}     Byte& flag   (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _flag    [i];} // get i-th triangle flag
 C Byte* flag   ()C {return _flag    ;}   C Byte& flag   (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _flag    [i];} // get i-th triangle flag
   Int * id     ()  {return _id      ;}     Int & id     (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _id      [i];} // get i-th triangle ID
 C Int * id     ()C {return _id      ;}   C Int & id     (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _id      [i];} // get i-th triangle ID

   MeshTris() {}
#if !EE_PRIVATE
private:
#endif
   Int _elms; VecI *_ind, *_adj_face, *_adj_edge; Vec *_nrm; Byte *_flag; Int *_id;
   NO_COPY_CONSTRUCTOR(MeshTris);
};
/******************************************************************************/
struct MeshQuads // Mesh Quads
{
   Int    elms   ()C {return _elms    ;}                                                                                 // get number of quads
   VecI4* ind    ()  {return _ind     ;}     VecI4& ind    (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _ind     [i];} // get i-th quad vertex indexes
 C VecI4* ind    ()C {return _ind     ;}   C VecI4& ind    (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _ind     [i];} // get i-th quad vertex indexes
   VecI4* adjFace()  {return _adj_face;}     VecI4& adjFace(Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _adj_face[i];} // get i-th quad adjacent faces, index to faces adjacent to the quad encoded in following way : if(adj_face==-1) -> no face, else if(adj_face&SIGN_BIT)adj_quad_index=adj_face^SIGN_BIT, else adj_tri_index=adj_face
 C VecI4* adjFace()C {return _adj_face;}   C VecI4& adjFace(Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _adj_face[i];} // get i-th quad adjacent faces, index to faces adjacent to the quad encoded in following way : if(adj_face==-1) -> no face, else if(adj_face&SIGN_BIT)adj_quad_index=adj_face^SIGN_BIT, else adj_tri_index=adj_face
   VecI4* adjEdge()  {return _adj_edge;}     VecI4& adjEdge(Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _adj_edge[i];} // get i-th quad adjacent edges, index to edges adjacent to the quad encoded in following way : if(adj_edge==-1) -> no edge, else adj_edge_index=adj_edge
 C VecI4* adjEdge()C {return _adj_edge;}   C VecI4& adjEdge(Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _adj_edge[i];} // get i-th quad adjacent edges, index to edges adjacent to the quad encoded in following way : if(adj_edge==-1) -> no edge, else adj_edge_index=adj_edge
   Vec  * nrm    ()  {return _nrm     ;}     Vec  & nrm    (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _nrm     [i];} // get i-th quad normal
 C Vec  * nrm    ()C {return _nrm     ;}   C Vec  & nrm    (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _nrm     [i];} // get i-th quad normal
   Byte * flag   ()  {return _flag    ;}     Byte & flag   (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _flag    [i];} // get i-th quad flag
 C Byte * flag   ()C {return _flag    ;}   C Byte & flag   (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _flag    [i];} // get i-th quad flag
   Int  * id     ()  {return _id      ;}     Int  & id     (Int i)  {DEBUG_RANGE_ASSERT(i, _elms); return _id      [i];} // get i-th quad ID
 C Int  * id     ()C {return _id      ;}   C Int  & id     (Int i)C {DEBUG_RANGE_ASSERT(i, _elms); return _id      [i];} // get i-th quad ID

   MeshQuads() {}
#if !EE_PRIVATE
private:
#endif
   Int _elms; VecI4 *_ind, *_adj_face, *_adj_edge; Vec *_nrm; Byte *_flag; Int *_id;
   NO_COPY_CONSTRUCTOR(MeshQuads);
};
/******************************************************************************/
struct MeshBase // Mesh Base (the most low level software mesh, contains : Vertexes + Edges + Triangles + Quads)
{
   MeshVtxs  vtx ; // vertexes
   MeshEdges edge; // edges
   MeshTris  tri ; // triangles
   MeshQuads quad; // quads

   // manage
   MeshBase& del       (                                                                                                                                       ); // delete manually
   MeshBase& create    (  Int         vtxs, Int edges, Int tris, Int quads, MESH_FLAG flag=MESH_NONE                                                           ); // create, 'vtxs'=number of vertexes, 'edges'=number of edges, 'tris'=number of triangles, 'quads'=number of quads, 'flag'=type of data to allocate (here VTX_POS, EDGE_IND, TRI_IND, QUAD_IND are always created automatically so you don't have to specify them manually)
   MeshBase& create    (C MeshBase   &src ,           MESH_FLAG flag_and=MESH_ALL                                                                              ); // create from 'src'      , 'flag_and'=data to copy
   MeshBase& create    (C MeshRender &src ,           MESH_FLAG flag_and=MESH_ALL                                                                              ); // create from 'src'      , 'flag_and'=data to copy
   MeshBase& create    (C MeshPart   &src ,           MESH_FLAG flag_and=MESH_ALL                                                                              ); // create from 'src'      , 'flag_and'=data to copy
   MeshBase& create    (C MeshLod    &src ,           MESH_FLAG flag_and=MESH_ALL        , Bool set_face_id_from_part_index=false                              ); // create from 'src'      , 'flag_and'=data to copy, 'set_face_id_from_part_index'=if set face 'id' members to the index of the original MeshPart that they were created from
   MeshBase& create    (C MeshBase   *src , Int elms, MESH_FLAG flag_and=MESH_ALL        , Bool set_face_id_from_part_index=false                              ); // create from 'src' array, 'flag_and'=data to copy, 'set_face_id_from_part_index'=if set face 'id' members to the index of the original MeshPart that they were created from
   MeshBase& create    (C PhysPart   &src                                                                                                                      ); // create from 'src'
   MeshBase& createPhys(C MeshLod    &src ,           MESH_FLAG flag_and=VTX_POS|FACE_IND, Bool set_face_id_from_part_index=false, Bool skip_hidden_parts=false); // create from 'src'      , 'flag_and'=data to copy, 'set_face_id_from_part_index'=if set face 'id' members to the index of the original MeshPart that they were created from, this method ignores parts with MSHP_NO_PHYS_BODY flag and does not include them in the final mesh, 'skip_hidden_parts'=if ignore MeshPart's with MSHP_HIDDEN flag
#if EE_PRIVATE
   MeshBase& create    (C MeshBase  *src[], Int elms, MESH_FLAG flag_and=MESH_ALL        , Bool set_face_id_from_part_index=false                              ); // create from 'src' array, 'flag_and'=data to copy, 'set_face_id_from_part_index'=if set face 'id' members to the index of the original MeshPart that they were created from
   Bool      createVtx (C VtxBuf &vb, MESH_FLAG flag, UInt storage,                                                  MESH_FLAG flag_and=MESH_ALL) ; // create vertexes from vertex buffer
   Bool      createInd (C IndBuf &ib                                                                                                            ) ; // create indexed  from index  buffer
   MeshBase& copyFace  (MeshBase &dest, C CMemPtr<Bool> &edge_is, C CMemPtr<Bool> &tri_is, C CMemPtr<Bool> &quad_is, MESH_FLAG flag_and=MESH_ALL)C; // copy only selected elements                          , 'flag_and'=data to copy, 'edge_is tri_is quad_is' must point to an array of Bool's of size equal to number of eddges, triangles and quads respectively, i-th element will be copied only if "_is[i]==true", if the "is" parameter is set to null, then no elements are copied, !! method returns 'dest' !!
   void      copyID    (MeshBase &dest, Int id,                                                                      MESH_FLAG flag_and=MESH_ALL)C; // copy only elements with matching ID                  , 'flag_and'=data to copy, the 'id' parameter is compared to 'id' member of each edge, triangle and quad from the mesh, if any element matches the comparison, then it is copied into destination mesh
   void      copyID    (MeshLod  &dest,                                                                              MESH_FLAG flag_and=MESH_ALL)C; // copy according to ID's (each ID to separate MeshPart), 'flag_and'=data to copy, this method tests 'id' member of each edge, triangle and quad, and stores the element in different Mesh Parts depending on the 'id' value, elements of "id==0" go to 0-th MeshPart, elements of "id==1" go to 1-st MeshPart, ...
   void      copyID    (Mesh     &dest,                                                                              MESH_FLAG flag_and=MESH_ALL)C; // copy according to ID's (each ID to separate MeshPart), 'flag_and'=data to copy, this method tests 'id' member of each edge, triangle and quad, and stores the element in different Mesh Parts depending on the 'id' value, elements of "id==0" go to 0-th MeshPart, elements of "id==1" go to 1-st MeshPart, ...

   void copyVtxs (C MeshBase &src);   void copyVtxs (C MeshBase &src, C CMemPtr<Bool> &is);
   void copyEdges(C MeshBase &src);   void copyEdges(C MeshBase &src, C CMemPtr<Bool> &is);
   void copyTris (C MeshBase &src);   void copyTris (C MeshBase &src, C CMemPtr<Bool> &is);
   void copyQuads(C MeshBase &src);   void copyQuads(C MeshBase &src, C CMemPtr<Bool> &is);
#if PHYSX
   Bool create(PxConvexMesh   &convex);
   Bool create(PxTriangleMesh &mesh  );
#else
   Bool create(btConvexHullShape      &convex);
   Bool create(btBvhTriangleMeshShape &mesh  );
#endif
#endif

   MeshBase& include (MESH_FLAG flag); // include   elements specified with 'flag'
   MeshBase& exclude (MESH_FLAG flag); // exclude   elements specified with 'flag'
   MeshBase& keepOnly(MESH_FLAG flag); // keep only elements specified with 'flag'

   // create
   MeshBase& createPlane (  Int x=2, Int y=2, MESH_FLAG flag=MESH_NONE                                       ); // create mesh as 3D plane from (0,0,0) to (1,1,0) vertex positions using quads, 'x, y'=vertex resolution, 'flag'=data to set in the mesh
   MeshBase& create      (C Box     &box    , MESH_FLAG flag=MESH_NONE, Int resolution=-1                    ); // create mesh as 3D box                                  , 'flag'=data to set in the mesh
   MeshBase& create      (C OBox    &obox   , MESH_FLAG flag=MESH_NONE, Int resolution=-1                    ); // create mesh as 3D oriented box                         , 'flag'=data to set in the mesh
   MeshBase& create      (C Ball    &ball   , MESH_FLAG flag=MESH_NONE, Int resolution=-1                    ); // create mesh as 3D ball in cube      mode and UV mapping, 'flag'=data to set in the mesh
   MeshBase& create2     (C Ball    &ball   , MESH_FLAG flag=MESH_NONE, Int resolution=-1, Int resolution2=-1); // create mesh as 3D ball in spherical mode and UV mapping, 'flag'=data to set in the mesh
   MeshBase& createIco   (C Ball    &ball   , MESH_FLAG flag=MESH_NONE, Int resolution=-1                    ); // create mesh as 3D ball in icosphere mode and UV mapping, 'flag'=data to set in the mesh
   MeshBase& create      (C Capsule &capsule, MESH_FLAG flag=MESH_NONE, Int resolution=-1, Int resolution2=-1); // create mesh as 3D capsule                              , 'flag'=data to set in the mesh
   MeshBase& create      (C Tube    &tube   , MESH_FLAG flag=MESH_NONE, Int resolution=-1                    ); // create mesh as 3D tube                                 , 'flag'=data to set in the mesh
   MeshBase& create      (C Cone    &cone   , MESH_FLAG flag=MESH_NONE, Int resolution=-1                    ); // create mesh as 3D cone                                 , 'flag'=data to set in the mesh
   MeshBase& create      (C Torus   &torus  , MESH_FLAG flag=MESH_NONE, Int resolution=-1, Int resolution2=-1); // create mesh as 3D torus                                , 'flag'=data to set in the mesh
   MeshBase& create      (C Shape   &shape  , MESH_FLAG flag=MESH_NONE, Int resolution=-1, Int resolution2=-1); // create mesh as 3D shape                                , 'flag'=data to set in the mesh
   MeshBase& createConvex(C Vec     *point  , Int       points        , Int max_points=-1                    ); // create mesh as 3D convex mesh created from 'point' array of 'points' elements, you can optionally limit the maximum number of generated points to 'max_points' by setting it to a value different than -1
#if EE_PRIVATE
   MeshBase& createGrid    (  Int x=2, Int y=2, Bool fast=false                           ); // create mesh as 2D grid from (0,0) to (1,1) vertex positions using edges, 'x, y'=vertex resolution
   MeshBase& createFast    (C Box    &box                                                 ); // create mesh as 3D box (fast version - only vertex position and quad indexes are set)
   MeshBase& createBox     (C Matrix &matrix , MESH_FLAG flag=MESH_NONE, Int resolution=-1); // create mesh as 3D box              , 'flag'=data to set in the mesh
   MeshBase& createHalf    (C Ball   &ball   , MESH_FLAG flag=MESH_NONE, Int resolution=-1); // create mesh as 3D ball's upper half, 'flag'=data to set in the mesh
   MeshBase& createIcoHalf (C Ball   &ball   , MESH_FLAG flag=MESH_NONE, Int resolution=-1); // create mesh as 3D ball's upper half, 'flag'=data to set in the mesh
   MeshBase& createFast    (C Tube   &tube   ,                           Int resolution=-1); // create mesh as 3D tube (fast)
   MeshBase& createEdge    (C Rect   &rect   , Bool solid=false                           ); // create mesh as 2D edged-rectangle
   MeshBase& createEdge    (C Circle &circle , Bool solid=false,         Int resolution=-1); // create mesh as 2D edged-circle
   MeshBase& createEdgeStar(  Flt r1, Flt r2 , Bool solid=false,         Int resolution=-1); // create mesh as 2D edged-star, 'r1,r2'=star radius
#endif

   // get
   Bool is       ()C {return vtx.elms() || edge.elms() || tri.elms() || quad.elms()  ;} // if  has any data
   Int  vtxs     ()C {return vtx.elms()                                              ;} // get number of vertexes
   Int  edges    ()C {return               edge.elms()                               ;} // get number of edges
   Int  tris     ()C {return                              tri.elms()                 ;} // get number of triangles
   Int  quads    ()C {return                                            quad.elms()  ;} // get number of quads
   Int  faces    ()C {return                              tri.elms()  + quad.elms()  ;} // get number of faces
   Int  trisTotal()C {return                              tri.elms()  + quad.elms()*2;} // get number of triangles including quads (each quad can be represented by 2 triangles)

   MESH_FLAG flag        (                            )C; // get available data
   UInt      memUsage    (                            )C; // get memory usage
   Bool      getBox      (Box  &box                   )C; // get box  encapsulating the mesh, this method iterates through all vertexes, false on fail (if no vertexes are present)
   Bool      getBox      (Box  &box, C Matrix3 &matrix)C; // get box  encapsulating the mesh, this method iterates through all vertexes, 'matrix'=matrix affecting vertex positions, false on fail (if no vertexes are present)
   Bool      getBox      (Box  &box, C Matrix  &matrix)C; // get box  encapsulating the mesh, this method iterates through all vertexes, 'matrix'=matrix affecting vertex positions, false on fail (if no vertexes are present)
   Bool      getBall     (Ball &ball                  )C; // get ball encapsulating the mesh, this method iterates through all vertexes, false on fail (if no vertexes are present)
   Bool      getRect     (Rect &rect                  )C; // get 2D rectangle encapsulating the mesh on XY plane, false on fail (if no vertexes are present)
   Bool      getRectXZ   (Rect &rect                  )C; // get 2D rectangle encapsulating the mesh on XZ plane, false on fail (if no vertexes are present)
   Flt       area        (Vec  *center=null           )C; // get surface area of all mesh faces, 'center'=if specified then it will be calculated as the average surface center
   Flt       convexVolume(                            )C; // calculate volume of this mesh assuming that it's convex
   Int       maxID       (                            )C; // get maximum value of ID in edges/faces, -1 if doesn't have any
   Bool      hasID       (Int   id                    )C; // if  any of the edges/faces have their ID set to 'id'

   // set
#if EE_PRIVATE
   MeshBase& setEdgeNormals(Bool flag=false); // recalculate edge            2D normals, 'flag'=if include ETQ_FLAG behavior in normal calculation
   MeshBase& setNormals2D  (Bool flag=false); // recalculate edge and vertex 2D normals, 'flag'=if include ETQ_FLAG behavior in normal calculation
   MeshBase& setVtxDupEx   (MESH_FLAG flag=MESH_NONE, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS, Flt tan_cos=EPS_TAN_COS, Flt bin_cos=EPS_BIN_COS, Bool tex_wrap=false, Bool smooth_groups_in_vtx_material=false);
#endif
   MeshBase& setNormals    (                                                                  ); // recalculate vertex            3D normals
   MeshBase& setNormalsAuto(Flt angle=EPS_NRM_AUTO, Flt pos_eps=EPS                           ); // recalculate vertex            3D normals using an automatic smart algorithm, 'angle'=angle limit for merging vertex normals, this method will generate a lot of vertexes, it's recommended to call 'weldVtx' afterwards
   MeshBase& setFaceNormals(                                                                  ); // recalculate triangle and quad 3D normals
   MeshBase& setTanBin     (                                                                  ); // recalculate vertex            3D tangents and binormals
   MeshBase& setTanBinDbl  (                                                                  ); // recalculate vertex            3D tangents and binormals using Dbl precision, which is slower but correctly handles huge meshes
   MeshBase& setAutoTanBin (                                                                  ); // automatically calculate vertex tangents and binormals if needed, if they're not needed then they will be removed
   MeshBase& setVtxDup2D   (MESH_FLAG flag=MESH_NONE, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS); // set vertex 2D duplicates (vtx.dup)
   MeshBase& setVtxDup     (MESH_FLAG flag=MESH_NONE, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS); // set vertex 3D duplicates (vtx.dup)
   MeshBase& setAdjacencies(Bool faces=true, Bool edges=false                                 ); // set adjacencies, 'faces'=if set face adjacencies ('tri.adjFace', 'quad.adjFace'), 'edges'=if set edges ('edge') and edge adjacencies ('tri.adjEdge', 'quad.adjEdge', 'edge.adjFace')

   // transform
   MeshBase& move     (              C Vec &move                               ); //           move mesh
   MeshBase& scale    (C Vec &scale                                            ); // scale          mesh
   MeshBase& scaleMove(C Vec &scale, C Vec &move                               ); // scale and move mesh
   MeshBase& setSize  (C Box &box                                              ); // scale and move mesh to fit box
   MeshBase& transform(C Matrix3               &matrix, Bool scale_normals=true); // transform by matrix, 'scale_normals'=if scale normals (if false then normals are not scaled but only rotated)
   MeshBase& transform(C Matrix                &matrix, Bool scale_normals=true); // transform by matrix, 'scale_normals'=if scale normals (if false then normals are not scaled but only rotated)
   MeshBase& animate  (C MemPtrN<Matrix , 256> &matrixes                       ); // animate   by matrixes
   MeshBase& animate  (C MemPtrN<MatrixM, 256> &matrixes                       ); // animate   by matrixes
   MeshBase& animate  (C AnimatedSkeleton      &anim_skel                      ); // animate   by skeleton
   MeshBase& mirrorX  (                                                        ); // mirror in X axis
   MeshBase& mirrorY  (                                                        ); // mirror in Y axis
   MeshBase& mirrorZ  (                                                        ); // mirror in Z axis
   MeshBase& reverse  (                                                        ); // reverse all      faces
   MeshBase& reverse  (  Int           face                                    ); // reverse selected face , here the 'face'  index can point to both triangles and quads, if face is a triangle then "face=triangle_index", if face is a quad then "face=quad_index^SIGN_BIT"
   MeshBase& reverse  (C CMemPtr<Int> &faces                                   ); // reverse selected faces, here the 'faces' index can point to both triangles and quads, if face is a triangle then "face=triangle_index", if face is a quad then "face=quad_index^SIGN_BIT"
#if EE_PRIVATE
   MeshBase& rightToLeft(                                                      ); // convert from right hand to left hand coordinate system
#endif

   // texture transform
   MeshBase& texMove  (C Vec2 &move , Byte uv_index=0); // move   texture UV's
   MeshBase& texScale (C Vec2 &scale, Byte uv_index=0); // scale  texture UV's
   MeshBase& texRotate(  Flt   angle, Byte uv_index=0); // rotate texture UV's

   // texturize
   MeshBase& texMap  (  Flt     scale=1, Byte uv_index=0); // map texture UV's according to vertex XY position and scale
   MeshBase& texMapXZ(  Flt     scale=1, Byte uv_index=0); // map texture UV's according to vertex XZ position and scale
   MeshBase& texMap  (C Matrix &matrix , Byte uv_index=0); // map texture UV's according to matrix
   MeshBase& texMap  (C Plane  &plane  , Byte uv_index=0); // map texture UV's according to plane
   MeshBase& texMap  (C Ball   &ball   , Byte uv_index=0); // map texture UV's according to ball
   MeshBase& texMap  (C Tube   &tube   , Byte uv_index=0); // map texture UV's according to tube

   // operations
#if EE_PRIVATE
   MeshBase& weldEdge     (); // weld edges
   MeshBase& weldVtx2D    (MESH_FLAG flag=MESH_NONE, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS, Flt remove_degenerate_faces_eps=EPS); // weld 2D vertexes     , this function will weld vertexes together if they share the same position (ignoring Z), 'flag'=if selected elements aren't equal then don't weld, 'remove_degenerate_faces_eps'=epsilon used for removing degenerate faces which may occur after welding vertexes (use <0 to disable removal)
#endif
   MeshBase& weldVtx      (MESH_FLAG flag=MESH_NONE, Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS, Flt remove_degenerate_faces_eps=EPS); // weld 3D vertexes     , this function will weld vertexes together if they share the same position, 'flag'=if selected elements aren't equal then don't weld, 'remove_degenerate_faces_eps'=epsilon used for removing degenerate faces which may occur after welding vertexes (use <0 to disable removal)
   MeshBase& weldVtxValues(MESH_FLAG flag          , Flt pos_eps=EPS, Flt nrm_cos=EPS_COL_COS, Flt remove_degenerate_faces_eps=EPS); // weld    vertex values, this function will weld values of vertexes which  share the same position, 'flag'=                                 elements to weld, 'remove_degenerate_faces_eps'=epsilon used for removing degenerate faces which may occur after welding vertexes (use <0 to disable removal)

   MeshBase& explodeVtxs(); // separate vertexes so that each edge/face has its own unique vertexes

   MeshBase& tesselate    (                          Flt weld_pos_eps=EPS); // smooth subdivide faces, preserving original vertexes, 'weld_pos_eps'=epsilon used for final vertex position welding
   MeshBase& tesselate    (C CMemPtr<Bool> &vtx_sel, Flt weld_pos_eps=EPS); // smooth subdivide faces, preserving original vertexes, 'weld_pos_eps'=epsilon used for final vertex position welding, only selected vertexes will be tesselated, 'vtx_sel'=array of all vertexes in mesh specifying if each vertex is selected (true) or not (false)
   MeshBase& tesselate    (C CMemPtr<Int > &vtx_sel, Flt weld_pos_eps=EPS); // smooth subdivide faces, preserving original vertexes, 'weld_pos_eps'=epsilon used for final vertex position welding, only selected vertexes will be tesselated, 'vtx_sel'=list of selected vertexes
   MeshBase& subdivide    (                                              ); // smooth subdivide faces,  smoothing original vertexes
   MeshBase& subdivideEdge(Bool freeze_z=false, C CMemPtr<Bool> &is=null ); // smooth subdivide edges, 'is'=only selected edges

   MeshBase&   boneRemap(C CMemPtrN<BoneType, 256> &old_to_new); // remap vertex bone/matrix indexes according to bone 'old_to_new' remap
   void     setUsedBones(   MemPtrN<Bool    , 256>  bones)C;
   void includeUsedBones(   MemPtrN<Bool    , 256>  bones)C;

   MeshBase& setVtxAO(Flt strength, Flt bias, Flt max, Flt ray_length, Flt pos_eps=EPS, Int rays=1024, MESH_AO_FUNC func=MAF_FULL, Threads *threads=null); // calculate per-vertex ambient occlusion in vertex colors, 'strength'=0..1 AO strength, 'bias'=0..1, 'max'=AO limit 0..1, 'ray_length'=max ray distance to test, 'rays'=number of rays to use for AO calculation, 'func'=falloff function

#if EE_PRIVATE
   void splitVtxs (MeshBase &dest, C CMemPtr<Bool> & vtx_is,                                                    MESH_FLAG flag_and=MESH_ALL); // split by moving selected vertexes to 'dest' leaving the rest only, copy only elements included in 'flag_and'
   void splitFaces(MeshBase &dest, C CMemPtr<Bool> &edge_is, C CMemPtr<Bool> &tri_is, C CMemPtr<Bool> &quad_is, MESH_FLAG flag_and=MESH_ALL); // split by moving selected faces    to 'dest' leaving the rest only, copy only elements included in 'flag_and'

   void      copyVtx   (Int i, MeshBase &dest, Int dest_i)C; // copy i-th vertex to 'dest' dest_i-th vertex
   MeshBase& splitEdges(Flt length=1      , Bool *is=null) ; // split edges, 'length'=maximum length of edge, 'is'=only selected edges
   MeshBase& splitEdges(Flt length, Flt d , Bool *is=null) ; // split edges, 'length'=minimum length of edge to part, 'd'=length of added edges on left & right side, 'is'=only selected edges

   // join / split
   void split    (MemPtr<MeshBaseIndex> meshes, C Boxes &boxes,             MESH_FLAG flag_and=MESH_ALL)C; // split to container of meshes by boxes                                        , copy only elements included in 'flag_and'
   void split    (MemPtr<MeshBaseIndex> meshes, C VecI  &cells,             MESH_FLAG flag_and=MESH_ALL)C; // split to container of meshes by number of splits                             , copy only elements included in 'flag_and'
   void split    (MemPtr<MeshBaseIndex> meshes, C Plane *plane, Int planes, MESH_FLAG flag_and=MESH_ALL)C; // split to container of meshes by planes                                       , copy only elements included in 'flag_and'
   void splitBone(MeshBase &dest, Int bone                    ,             MESH_FLAG flag_and=MESH_ALL) ; // split by moving elements influenced by 'bone' to 'dest' leaving the rest only, copy only elements included in 'flag_and'
#endif

   // fix
   MeshBase& fixTexOffset  (Byte uv_index=0); // fix texture offset  , this reduces big texture coordinates to small ones increasing texturing quality on low precision video cards
   MeshBase& fixTexWrapping(Byte uv_index=0); // fix texture wrapping, fixes texture coordinates created by spherical/tube mapping (this can add new vertexes to the mesh)

   // link
#if EE_PRIVATE
   void linkVtxVtxSamePos(Index &vtx_vtx , Flt pos_eps=EPS, Bool dual=true)C; // link vertexes   with neighbor vertexes that share the same position, 'dual'=if link both A->B and B->A connections, if false then only A->B
   void linkVtxVtxOnFace (Index &vtx_vtx                                  )C; // link vertexes   with neighbor vertexes on faces
   void linkVtxVtxOnEdge (Index &vtx_vtx , Bool sort=true                 )C; // link vertexes   with neighbor vertexes on edges, 'sort'=if sort vertexes in angle order
   void linkVtxEdge      (Index &vtx_edge, Bool sort=true                 )C; // link vertexes   with edges, 'sort'=if sort edges in angle order
   void linkVtxFace      (Index &vtx_face                                 )C; // link vertexes   with faces
   void linkFaceFace     (Index &face_face                                )C; // link faces      with faces, first are listed triangles followed by quads
   void linkEdgeFace     (                                                ) ; // link edges      with faces
   void linkRectEdge     (Index &rect_edge, C Rects &rects                )C; // link rectangles with edges
#endif
   void getVtxNeighbors (Int vtx , MemPtr<Int> vtxs )C; // get 'vtxs'  neighbors of 'vtx'  (including itself), this will return an array of vertexes that are connected to each other
   void getFaceNeighbors(Int face, MemPtr<Int> faces)C; // get 'faces' neighbors of 'face' (including itself), this will return an array of faces    that are connected to each other, here the 'face' and 'faces' indexes can point to both triangles and quads, if face is a triangle then "face=triangle_index", if face is a quad then "face=quad_index^SIGN_BIT"

   // convert
#if EE_PRIVATE
   MeshBase& edgeToPoly (MemPtr<Poly> polys        ); // edges     to polys
#endif
   MeshBase& edgeToDepth(Bool tex_align=true       ); // edges     to depth     (extrude 2D edges to 3D faces)
   MeshBase& edgeToTri  (Bool set_id   =false      ); // edges     to triangles (triangulation)
   MeshBase& triToQuad  (Flt  cos      =EPS_COL_COS); // triangles to quads    , 'cos'=minimum cosine between 2      triangle normals to weld  them into 1 quad (0..1)
   MeshBase& quadToTri  (Flt  cos      =2          ); // quads     to triangles, 'cos'=minimum cosine between 2 quad triangle normals to leave them as   1 quad (0..1, value >1 converts all quads into triangles)
   MeshBase& quadToTri  (C CMemPtr<Int> &quads     ); // quads     to triangles, 'quads'=indexes of quads to convert

   enum TEX_MODE
   {
      TEX_NONE   , // don't create   texture coordinates
      TEX_KEEP   , // keep  existing texture coordinates
      TEX_UNIFORM, // set   uniform  texture coordinates in range 0..1
      TEX_SCALED , // set   scaled   texture coordinates (X coords are scaled by vertex size, Y coords are scaled by distances between vertexes)
   };
   MeshBase& inflateEdges(TEX_MODE x_tex_coords=TEX_SCALED, TEX_MODE y_tex_coords=TEX_SCALED, Bool to_edges=false); // inflate mesh edges, 'x_tex_coords'=mode for settings X texture coordinates, 'y_tex_coords'=mode for settings Y texture coordinates, 'to_edges'=if inflate into edges (if false then faces will be created), this function operates on XY plane

#if EE_PRIVATE
   // csg
   MeshBase& cut2D (                                                  ); // cut 2D mesh by self  !! beta !!
   MeshBase& cut2D (C MeshBase &mshb                                  ); // cut 2D mesh by mshb  !! beta !!
   MeshBase& grid2D(C Vec2     &scale=Vec2(1), C Vec2 &offset=Vec2Zero); // cut 2D mesh by grid  !! beta !!
   MeshBase& csg2D (C MeshBase &mshb         ,   UInt sel             ); // csg 2D, sel=SEL_FLAG !! beta !!
#endif

   // add / remove
   MeshBase& addVtx (C Vec        &pos                             ); // add empty vertex at 'pos' position
   MeshBase& addEdge(C VecI2      &ind                             ); // add edge     by giving vertex indexes
   MeshBase& addTri (C VecI       &ind                             ); // add triangle by giving vertex indexes
   MeshBase& addQuad(C VecI4      &ind                             ); // add quad     by giving vertex indexes
   MeshBase& add    (C MeshBase   &src, MESH_FLAG flag_and=MESH_ALL); // add MeshBase
   MeshBase& add    (C MeshRender &src, MESH_FLAG flag_and=MESH_ALL); // add MeshRender
   MeshBase& add    (C MeshPart   &src, MESH_FLAG flag_and=MESH_ALL); // add MeshPart

#if EE_PRIVATE
   MeshBase& keepVtxs (C CMemPtr<Bool> &is); // keep only vertexes  which "is[i]==true"
   MeshBase& keepEdges(C CMemPtr<Bool> &is); // keep only edges     which "is[i]==true"
   MeshBase& keepTris (C CMemPtr<Bool> &is); // keep only triangles which "is[i]==true"
   MeshBase& keepQuads(C CMemPtr<Bool> &is); // keep only quads     which "is[i]==true"
#endif
   MeshBase& removeVtx (Int vtx );
   MeshBase& removeEdge(Int edge);
   MeshBase& removeTri (Int tri );
   MeshBase& removeQuad(Int quad);
   MeshBase& removeFace(Int face); // remove face, here the 'face' index can point to both triangles and quads, if face is a triangle then "face=triangle_index", if face is a quad then "face=quad_index^SIGN_BIT"

   MeshBase& removeVtxs (C CMemPtr<Int> &vtxs                               ); // remove selected vertexes
   MeshBase& removeEdges(C CMemPtr<Int> &edges, Bool remove_unused_vtxs=true); // remove selected edges
   MeshBase& removeTris (C CMemPtr<Int> &tris , Bool remove_unused_vtxs=true); // remove selected triangles
   MeshBase& removeQuads(C CMemPtr<Int> &quads, Bool remove_unused_vtxs=true); // remove selected quads
   MeshBase& removeFaces(C CMemPtr<Int> &faces, Bool remove_unused_vtxs=true); // remove selected faces, here the 'faces' indexes can point to both triangles and quads, if face is a triangle then "face=triangle_index", if face is a quad then "face=quad_index^SIGN_BIT"

   MeshBase& removeDoubleSideFaces(Bool remove_unused_vtxs=true);

   Int mergeFaces(Int a, Int b); // this method will merge 2 faces together, but only if they share exactly 2 vertexes, upon success index of merged face is returned, -1 on fail

   // optimize
#if EE_PRIVATE
   MeshBase& removeSingleFaces    (Flt  fraction                                              ); // remove fraction (0..1) of single faces (single triangles or quads not linked to any other face)
   MeshBase& removeDoubleEdges    ();
   MeshBase& weldInlineEdges      (Flt  cos_edge=EPS_COL_COS, Flt cos_vtx=-1, Bool z_test=true); // weld inline edge vertexes, 'cos_edge'=minimum cosine between edge normals, 'cos_vtx'=minimum cosine between vertex normals, 'z_test'=if perform tests for inline 'z' vertex component
#endif
   MeshBase& optimizeCache        (Bool faces=true, Bool vertexes=true); // this method will re-order elements for best rendering performance, 'faces'=if re-order faces, 'vertexes'=if re-order vertexes
   MeshBase& removeDegenerateFaces(Flt  eps=EPS);
   Bool      removeUnusedVtxs     (Bool include_edge_references=true); // remove vertexes which aren't used by any face or edge, if 'include_edge_references' is set to false then only face references are tested (without the edges), returns true if any vertex was removed

   MeshBase& simplify(Flt intensity, Flt max_distance=1.0f, Flt max_uv=1.0f, Flt max_color=0.02f, Flt max_material=0.02f, Flt max_skin=1, Flt max_normal=PI, Bool keep_border=false, MESH_SIMPLIFY mode=SIMPLIFY_QUADRIC, Flt pos_eps=EPS, MeshBase *dest=null); // simplify mesh by removing vertexes/faces, 'intensity'=how much to simplify (0..1, 0=no simplification, 1=full simplification), 'max_distance'=max distance between elements to merge them (0..Inf), 'max_uv'=max allowed vertex texture UV deviations (0..1), 'max_color'=max allowed vertex color deviations (0..1), 'max_material'=max allowed vertex material deviations (0..1), 'max_skin'=max allowed vertex skin deviations (0..1), 'max_normal'=max allowed vertex normal angle deviations (0..PI), 'keep_border'=if always keep border edges (edges that have faces only on one side), 'pos_eps'=vertex position epsilon, 'dest'=destination MeshBase (if set to null then the mesh will simplify itself), returns dest

   Bool weldCoplanarFaces(Flt cos_face=EPS_COL8_COS, Flt cos_vtx=EPS_COL8_COS, Bool safe=false, Flt max_face_length=-1); // weld coplanar faces, 'cos_face'=minimum cosine between face normals, 'cos_vtx'=minimum cosine between vertex normals, 'safe'=if process only faces without neighbors, 'max_face_length'=max allowed face length (-1=no limit), returns true if any change was made

   // draw
#if EE_PRIVATE
   void draw2D       (C Color &vtx_color, C Color &edge_color, C Color &face_color, Flt vtx_r=0.04f, Flt side_width=0.1f                                              )C; // draw 2D
   void drawNormals2D(  Flt    length   , C Color &edge_color, C Color &vtx_color=TRANSPARENT                                                                         )C; // draw 2D normals
   void drawNormals  (  Flt    length   , C Color &face_color, C Color &vtx_color=TRANSPARENT, C Color &tangent_color=TRANSPARENT, C Color &binormal_color=TRANSPARENT)C; // draw 3D normals
#endif
   void drawAuto(C Material *material)C; // draw 3D, doesn't use automatic Frustum culling, optionally call this outside Render or inside Render function in any desired RENDER_MODE

   // io
   void operator=(C Str  &name) ; // load, Exit  on fail
   Bool save     (C Str  &name)C; // save, false on fail
   Bool load     (C Str  &name) ; // load, false on fail
   Bool save     (  File &f                  )C; // save, false on fail
   Bool load     (  File &f, CChar *path=null) ; // load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
#if EE_PRIVATE
   Bool saveData(File     &f)C; // save, false on fail
   Bool loadData(File     &f) ; // load, false on fail
   Bool saveTxt (FileText &f)C; // save text, false on fail
   Bool loadTxt (FileText &f) ; // load text, false on fail
   Bool saveTxt (C Str &name)C; // save text, false on fail
   Bool loadTxt (C Str &name) ; // load text, false on fail
#endif

   MeshBase& operator+=(C Vec        &v  ) {return move     ( v );} // move      by vector
   MeshBase& operator-=(C Vec        &v  ) {return move     (-v );} // move      by vector
   MeshBase& operator*=(C Matrix3    &m  ) {return transform( m );} // transform by matrix
   MeshBase& operator*=(C Matrix     &m  ) {return transform( m );} // transform by matrix
   MeshBase& operator+=(C MeshBase   &src) {return add      (src);} // add 'src'
   MeshBase& operator+=(C MeshRender &src) {return add      (src);} // add 'src'
   MeshBase& operator+=(C MeshPart   &src) {return add      (src);} // add 'src'
   MeshBase& operator =(C MeshBase   &src) {return create   (src);} // create from 'src'

           ~MeshBase() {del ( );}
            MeshBase() {Zero(T);}
   explicit MeshBase(  Int         vtxs, Int edges, Int tris, Int quads, MESH_FLAG flag=MESH_NONE);
   explicit MeshBase(C MeshBase   &src , MESH_FLAG flag_and=MESH_ALL                             );
   explicit MeshBase(C MeshRender &src , MESH_FLAG flag_and=MESH_ALL                             );
   explicit MeshBase(C MeshPart   &src , MESH_FLAG flag_and=MESH_ALL                             );
   explicit MeshBase(C MeshLod    &src , MESH_FLAG flag_and=MESH_ALL                             );
   explicit MeshBase(C PhysPart   &src                                                           );
};
/******************************************************************************/
#if EE_PRIVATE
struct MeshBaseIndex : MeshBase
{
   Int index;
};

#define DEFAULT_VTX_NRM 0, 1,  0 // value was chosen to point up, so if object is viewed from all ground sides (left/right/front/back) it will have same relative normal
#define DEFAULT_VTX_TAN 1, 0,  0
#define DEFAULT_VTX_BIN 0, 0, -1 // Cross(Vec(DEFAULT_VTX_NRM), Vec(DEFAULT_VTX_TAN))
#endif
/******************************************************************************/
inline Int Elms(C MeshVtxs  &vtx ) {return vtx .elms();}
inline Int Elms(C MeshEdges &edge) {return edge.elms();}
inline Int Elms(C MeshTris  &tri ) {return tri .elms();}
inline Int Elms(C MeshQuads &quad) {return quad.elms();}
/******************************************************************************/
