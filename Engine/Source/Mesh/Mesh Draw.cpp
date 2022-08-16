/******************************************************************************/
// TODO: reusing another MeshPart variation for drawing shadows when shadow shader doesn't use material, the best way would be to use Int MeshPart.last_shadow_instance, last_shadow_instance_no_cull and process that instead of MeshPart.Variation.last_instance when 'ReuseDefaultMaterialForNonSkinnedShadowShader' is detected, however it would require additional memory usage in MeshPart and extra overhead in 'drawShadow'
// TODO: consider drawing alpha tested materials after opaque, to implement: add new AlphaShaderDraws, process them in Mesh.draw if(material.hasAlphaTest - may need Bool member for fast access), this could potentially improve performance, probably more on mobile tile-based renderers
#include "stdafx.h"
namespace EE{
/******************************************************************************/
static INLINE void SetViewMatrix(Matrix &view_matrix                   ) {view_matrix=CamMatrixInv;}
static INLINE void SetViewMatrix(Matrix &view_matrix, C Matrix  &matrix) {matrix.mul(CamMatrixInv, view_matrix);} // view_matrix=matrix*CamMatrixInv
static INLINE void SetViewMatrix(Matrix &view_matrix, C MatrixM &matrix) {matrix.mul(CamMatrixInv, view_matrix);} // view_matrix=matrix*CamMatrixInv

static INLINE void SetViewMatrixPrev(Matrix &view_matrix                   ) {view_matrix=CamMatrixInvPrev;}
static INLINE void SetViewMatrixPrev(Matrix &view_matrix, C Matrix  &matrix) {matrix.mul(CamMatrixInvPrev, view_matrix);} // view_matrix=matrix*CamMatrixInvPrev
static INLINE void SetViewMatrixPrev(Matrix &view_matrix, C MatrixM &matrix) {matrix.mul(CamMatrixInvPrev, view_matrix);} // view_matrix=matrix*CamMatrixInvPrev

static INLINE void SetViewMatrix(MatrixPair &matrix, C Matrix &cur, C Matrix &prev)
{
   SetViewMatrix    (matrix.cur , cur );
   SetViewMatrixPrev(matrix.prev, prev);
}
static INLINE void SetViewMatrix(MatrixPair &matrix, C MatrixM &cur, C MatrixM &prev)
{
   SetViewMatrix    (matrix.cur , cur );
   SetViewMatrixPrev(matrix.prev, prev);
}

static INLINE void SetSkinning() {Sh.VtxSkinning->setConditional(Matrixes>1);}
/******************************************************************************/
void MeshBase::draw2D(C Color &vtx_color, C Color &edge_color, C Color &face_color, Flt vtx_r, Flt side_width)C
{
 C Int *p;

   if(face_color.a && quad.ind())
   {
      VI.color(face_color); REPA(quad){p=quad.ind(i).c; VI.quad(vtx.pos(p[0]).xy, vtx.pos(p[1]).xy, vtx.pos(p[2]).xy, vtx.pos(p[3]).xy);}
      VI.end  ();
   }
   if(face_color.a && tri.ind())
   {
      VI.color(face_color); REPA(tri){p=tri.ind(i).c; VI.tri(vtx.pos(p[0]).xy, vtx.pos(p[1]).xy, vtx.pos(p[2]).xy);}
      VI.end  ();
   }
   if(edge_color.a && edge.flag() && side_width)
   {
      Color C0=edge_color,
            C1=ColorAlpha(edge_color, 0.25f);
      REPA(edge)
      {
         Byte f=edge.flag(i); if(f&ETQ_LR)
         {
            p=edge.ind(i).c;
          C Vec2 &p0=vtx.pos(p[0]).xy,
                 &p1=vtx.pos(p[1]).xy;
            Color c0=((f&ETQ_NO_PHYS) ? WHITE                 : C0),
                  c1=((f&ETQ_NO_PHYS) ? Color(255,255,255,64) : C1);
            Vec2  r =Perp(p1-p0); r.setLength((f&ETQ_NO_PHYS) ? side_width*2 : side_width);
            if(f&ETQ_R)VI.quad(c0, c1, p0, p0+r, p1+r, p1);
            if(f&ETQ_L)VI.quad(c0, c1, p0, p0-r, p1-r, p1);
         }
      }
      VI.end();
   }
   if(edge_color.a && edge.ind())
   {
      VI.color(edge_color);
      REPA(edge)
      {
         p=edge.ind(i).c;
         VI.line(vtx.pos(p[0]).xy, vtx.pos(p[1]).xy);
      }
      VI.end();
   }
   if(vtx_color.a && vtx.pos())
   {
      VI.color(vtx_color); REPA(vtx)VI.dot(vtx.pos(i).xy, vtx_r);
      VI.end  ();
   }
}
void MeshLod  ::draw2D(C Color &vtx_color, C Color &edge_color, C Color &face_color, Flt vtx_r, Flt side_width)C {FREPA(T)parts [i].base.draw2D(vtx_color, edge_color, face_color, vtx_r, side_width);}
void MeshGroup::draw2D(C Color &vtx_color, C Color &edge_color, C Color &face_color, Flt vtx_r, Flt side_width)C {FREPA(T)meshes[i].     draw2D(vtx_color, edge_color, face_color, vtx_r, side_width);}
/******************************************************************************/
void MeshBase::drawNormals2D(Flt length, C Color &edge_color, C Color &vtx_color)C
{
 C Vec *pos=vtx.pos(),
       *nrm;
   if(vtx_color.a && (nrm=vtx.nrm()))
   {
      VI.color(vtx_color); REPA(vtx)VI.line(pos[i].xy, pos[i].xy+length*nrm[i].xy);
      VI.end  ();
   }
   if(edge_color.a && (nrm=edge.nrm()))
   {
      VI.color(edge_color);
    C VecI2 *_edge=edge.ind();
      REPA(edge)
      {
       C Int *p=(_edge++)->c;
         Vec2 v=Avg(pos[p[0]].xy, pos[p[1]].xy);
         VI.line(v, v+length*(nrm++)->xy);
      }
      VI.end();
   }
}
void MeshBase::drawNormals(Flt length, C Color &face_color, C Color &vtx_color, C Color &tangent_color, C Color &binormal_color)C
{
 C Int *p;
 C Vec *pos=vtx.pos();
   Int  p0, p1, p2, p3;

   if(     vtx_color.a)if(C Vec *nrm=vtx.nrm()){VI.color(     vtx_color); REPA(vtx)VI.line(pos[i], pos[i]+length*nrm[i]); VI.end();}
   if( tangent_color.a)if(C Vec *tan=vtx.tan()){VI.color( tangent_color); REPA(vtx)VI.line(pos[i], pos[i]+length*tan[i]); VI.end();}
   if(binormal_color.a)if(C Vec *bin=vtx.bin()){VI.color(binormal_color); REPA(vtx)VI.line(pos[i], pos[i]+length*bin[i]); VI.end();}

   if(face_color.a)
   {
      if(C Vec *nrm=edge.nrm())
      {
         VI.color(face_color);
       C VecI2 *_edge=edge.ind();
         REPA(edge)
         {
            p=(_edge++)->c;
            Vec v=Avg(pos[p[0]], pos[p[1]]);
            VI.line(v, v+length*(*nrm++));
         }
         VI.end();
      }
      {
         VI.color(face_color);
       C VecI *_tri=tri.ind();
       C Vec  * nrm=tri.nrm();
         REPA(tri)
         {
            p=(_tri++)->c; p0=p[0]; p1=p[1]; p2=p[2];
            Vec v=Avg(pos[p0], pos[p1], pos[p2]),
                n=(nrm ? *nrm++ : GetNormal(pos[p0], pos[p1], pos[p2]));
            VI.line(v, v+length*n);
         }
         VI.end();
      }
      {
         VI.color(face_color);
       C VecI4 *_quad=quad.ind();
       C Vec   *  nrm=quad.nrm();
         REPA(quad)
         {
            p=(_quad++)->c; p0=p[0]; p1=p[1]; p2=p[2]; p3=p[3];
            Vec v=Avg(pos[p0], pos[p1], pos[p2], pos[p3]),
                n=(nrm ? *nrm++ : GetNormal(pos[p0], pos[p1], pos[p3]));
            VI.line(v, v+length*n);
         }
         VI.end();
      }
   }
}
void MeshLod  ::drawNormals2D(Flt length, C Color &edge_color, C Color &vtx_color                                                 )C {REPA(T)parts [i].base.drawNormals2D(length, edge_color, vtx_color);}
void MeshGroup::drawNormals2D(Flt length, C Color &edge_color, C Color &vtx_color                                                 )C {REPA(T)meshes[i]     .drawNormals2D(length, edge_color, vtx_color);}
void MeshLod  ::drawNormals  (Flt length, C Color &face_color, C Color &vtx_color, C Color &tangent_color, C Color &binormal_color)C {REPA(T)parts [i].base.drawNormals  (length, face_color, vtx_color, tangent_color, binormal_color);}
void MeshGroup::drawNormals  (Flt length, C Color &face_color, C Color &vtx_color, C Color &tangent_color, C Color &binormal_color)C {REPA(T)meshes[i]     .drawNormals  (length, face_color, vtx_color, tangent_color, binormal_color);}
/******************************************************************************
Code for testing Grass/Leaf bending

#define GrassBendFreq  1.0
#define GrassBendScale 0.033

#define LeafBendFreq   2.0
#define LeafBendScale  0.039
#define LeafsBendScale (LeafBendScale/2)

Vec2 GetGrassBend(Vec world_pos, Bool prev=false)
{
   Vec4 bend_factor=Sh.BendFactor->getVec4();
   Flt  offset=Dot(world_pos.xz(), Vec2(0.7, 0.9)*GrassBendFreq);
   return Vec2((1.0*GrassBendScale)*Sin(offset+bend_factor.x) + (1.0*GrassBendScale)*Sin(offset+bend_factor.y),
               (1.0*GrassBendScale)*Sin(offset+bend_factor.z) + (1.0*GrassBendScale)*Sin(offset+bend_factor.w));
}
Vec2 Rotate(Vec2 vec, Vec2 cos_sin) // rotate vector by cos and sin values obtained from a custom angle
{
   return Vec2(vec.x*cos_sin.x - vec.y*cos_sin.y,
               vec.x*cos_sin.y + vec.y*cos_sin.x);
}
Vec2 GetLeafsBend(Vec center, Bool prev=false)
{
   Vec4 bend_factor=Sh.BendFactor->getVec4();
   Flt  offset=Dot(center.xy, Vec2(0.7, 0.8)*LeafBendFreq);
   return Vec2((1.0*LeafsBendScale)*(Flt)Sin(offset+bend_factor.x) + (1.0*LeafsBendScale)*(Flt)Sin(offset+bend_factor.y),
               (1.0*LeafsBendScale)*(Flt)Sin(offset+bend_factor.z) + (1.0*LeafsBendScale)*(Flt)Sin(offset+bend_factor.w));
}
void BendLeafs(Vec center, Flt offset, Vec &pos, Bool prev=false)
{
   if(center.allZero())return;
   Vec    delta=pos-center;
   Vec2   cos_sin, bend=GetLeafsBend(center+offset, prev);
   if(Kb.ctrl())bend*=delta.length()/2;
   CosSin(cos_sin.x, cos_sin.y, bend.x); delta.xy=Rotate(delta.xy, cos_sin);
   CosSin(cos_sin.x, cos_sin.y, bend.y); Vec2 zy=Rotate(delta.zy(), cos_sin); delta.z=zy.x; delta.y=zy.y;
   pos=center+delta;
}
void ProcessVtx(Vec &local_pos, C Vec &hlp, Flt size)
{
   if(!Kb.alt())
   {
      ULong id=xxHash64Mem(&ObjMatrix, SIZE(ObjMatrix));
      Randomizer rnd(id, 0);
      local_pos*=Matrix3().setRotateY(rnd.f(PI2));
   }
   if(0)
   {
      BendLeafs(hlp, size, local_pos);
   }else
   if(Kb.shift())
   {
      if(local_pos.y>0)
      {
         Flt len2=local_pos.length2();
         Flt blend=Min(len2, Sqr(local_pos.y*2));
         Vec2 bend=GetGrassBend(ObjMatrix.pos)*blend;
         local_pos.x+=bend.x;
         local_pos.z+=bend.y;
         if(Flt l2=local_pos.length2())local_pos*=SqrtFast(len2/l2); // local_pos.setLength2(len2);
      }
   }else
   if(Kb.ctrl())
   {
      if(local_pos.y>0)
      {
         Flt dist=local_pos.length();
         Flt blend=Sqr(Min(dist, local_pos.y*2));
         Vec2 bend=GetGrassBend(ObjMatrix.pos)*blend;
         local_pos.x+=bend.x;
         local_pos.z+=bend.y;
         local_pos.setLength(dist);
      }
      /*if(Kb.win())
      {
         Flt b=local_pos.length();
         if(local_pos.y<=0)b=0;
         if(Kb.alt())b*=b;
         if(Kb.shift())MIN(b, local_pos.y);else
         if(Kb.b(KB_Z))MIN(b, Sqr(local_pos.y*2));
         Vec2 bend=GetGrassBend1(ObjMatrix.pos)*(b*1);
         if(b>0)
         {
            Vec n=local_pos;
            n.x+=bend.x;
            n.z+=bend.y;
            n.setLength(local_pos.length());
            local_pos=n;
         }
      }else
      {
         Flt b=local_pos.y;
         Vec2 bend=GetGrassBend1(ObjMatrix.pos)*(b*1);
         if(b>0)
         {
            Flt dist=local_pos.y;
            Vec n(bend.x, dist, bend.y);
            if(Kb.shift())
            {
            Flt f=2;
               if(Kb.alt()){n.x*=f; n.z*=f;}
               n.setLength(dist);
               if(Kb.alt()){n.x/=f; n.z/=f;}
            }
            local_pos.x+=n.x;
            local_pos.z+=n.z;
            local_pos.y =n.y;
         }
      }*
   }else
   {
      if(local_pos.y>0)
      {
         Flt b=Sqr(local_pos.y);
         Vec2 bend=GetGrassBend(ObjMatrix.pos)*(b*1);
         local_pos.x+=bend.x;
         local_pos.z+=bend.y;
      }
   }
}
void ProcessVtx(Vtx3DFull &vtx) {ProcessVtx(vtx.pos, vtx.hlp, vtx.size);}
/******************************************************************************/
void MeshBase::drawAuto(C Material *material)C
{
    C Material &mtrl  =GetMaterial(material);
   if(Shader   *shader=DefaultShaders(&mtrl, flag()&~VTX_MATERIAL, 0, false).get(Renderer()))
   {
      SetSkinning();
      Vtx3DFull v[4]; Zero(v);
    C Vec      *nrm ;

      ALPHA_MODE alpha=D.alpha(ALPHA_NONE);
      mtrl.setAuto();

      // tris
      VI.shader(shader);
      VI.cull  (mtrl.cull);
      nrm=tri.nrm(); if(C VecI *_tri=tri.ind())REPA(tri)
      {
         VecI p=*_tri++;
         v[0].pos=vtx.pos(p.x);
         v[1].pos=vtx.pos(p.y);
         v[2].pos=vtx.pos(p.z);
         if(vtx.nrm())
         {
            if(vtx.tan())
            {
               if(vtx.bin())
               {
                  v[0].setNrmTan(vtx.nrm(p.x), vtx.tan(p.x), vtx.bin(p.x));
                  v[1].setNrmTan(vtx.nrm(p.y), vtx.tan(p.y), vtx.bin(p.y));
                  v[2].setNrmTan(vtx.nrm(p.z), vtx.tan(p.z), vtx.bin(p.z));
               }else
               {
                  v[0].setNrmTan(vtx.nrm(p.x), vtx.tan(p.x));
                  v[1].setNrmTan(vtx.nrm(p.y), vtx.tan(p.y));
                  v[2].setNrmTan(vtx.nrm(p.z), vtx.tan(p.z));
               }
            }else
            {
               v[0].nrm=vtx.nrm(p.x);
               v[1].nrm=vtx.nrm(p.y);
               v[2].nrm=vtx.nrm(p.z);
            }
         }else
         if(nrm)
         {
            v[0].nrm=v[1].nrm=v[2].nrm=*nrm++;
         }
         if(vtx.hlp())
         {
            v[0].hlp=vtx.hlp(p.x);
            v[1].hlp=vtx.hlp(p.y);
            v[2].hlp=vtx.hlp(p.z);
         }
         if(vtx.tex0())
         {
            v[0].tex0=vtx.tex0(p.x);
            v[1].tex0=vtx.tex0(p.y);
            v[2].tex0=vtx.tex0(p.z);
         }
         if(vtx.tex1())
         {
            v[0].tex1=vtx.tex1(p.x);
            v[1].tex1=vtx.tex1(p.y);
            v[2].tex1=vtx.tex1(p.z);
         }
         if(vtx.tex2())
         {
            v[0].tex2=vtx.tex2(p.x);
            v[1].tex2=vtx.tex2(p.y);
            v[2].tex2=vtx.tex2(p.z);
         }
         if(vtx.tex3())
         {
            v[0].tex3=vtx.tex3(p.x);
            v[1].tex3=vtx.tex3(p.y);
            v[2].tex3=vtx.tex3(p.z);
         }
         if(vtx.blend() && vtx.matrix())
         {
            v[0].blend =vtx.blend (p.x);
            v[1].blend =vtx.blend (p.y);
            v[2].blend =vtx.blend (p.z);
            v[0].matrix=vtx.matrix(p.x);
            v[1].matrix=vtx.matrix(p.y);
            v[2].matrix=vtx.matrix(p.z);
         }
         if(vtx.size())
         {
            v[0].size=vtx.size(p.x);
            v[1].size=vtx.size(p.y);
            v[2].size=vtx.size(p.z);
         }
         if(vtx.material())
         {
            v[0].material=vtx.material(p.x);
            v[1].material=vtx.material(p.y);
            v[2].material=vtx.material(p.z);
         }
         if(vtx.color())
         {
            v[0].color=vtx.color(p.x);
            v[1].color=vtx.color(p.y);
            v[2].color=vtx.color(p.z);
         }
         //REP(3)ProcessVtx(v[i]);
         VI.face(v[0], v[1], v[2]);
      }
      VI.end();

      // quads
      VI.shader(shader);
      VI.cull  (mtrl.cull);
      Zero(v); nrm=quad.nrm(); if(C VecI4 *_quad=quad.ind())REPA(quad)
      {
         VecI4 p=*_quad++;
         v[0].pos=vtx.pos(p.x);
         v[1].pos=vtx.pos(p.y);
         v[2].pos=vtx.pos(p.z);
         v[3].pos=vtx.pos(p.w);
         if(vtx.nrm())
         {
            if(vtx.tan())
            {
               if(vtx.bin())
               {
                  v[0].setNrmTan(vtx.nrm(p.x), vtx.tan(p.x), vtx.bin(p.x));
                  v[1].setNrmTan(vtx.nrm(p.y), vtx.tan(p.y), vtx.bin(p.y));
                  v[2].setNrmTan(vtx.nrm(p.z), vtx.tan(p.z), vtx.bin(p.z));
                  v[3].setNrmTan(vtx.nrm(p.w), vtx.tan(p.w), vtx.bin(p.w));
               }else
               {
                  v[0].setNrmTan(vtx.nrm(p.x), vtx.tan(p.x));
                  v[1].setNrmTan(vtx.nrm(p.y), vtx.tan(p.y));
                  v[2].setNrmTan(vtx.nrm(p.z), vtx.tan(p.z));
                  v[3].setNrmTan(vtx.nrm(p.w), vtx.tan(p.w));
               }
            }else
            {
               v[0].nrm=vtx.nrm(p.x);
               v[1].nrm=vtx.nrm(p.y);
               v[2].nrm=vtx.nrm(p.z);
               v[3].nrm=vtx.nrm(p.w);
            }
         }else
         if(nrm)
         {
            v[0].nrm=v[1].nrm=v[2].nrm=v[3].nrm=*nrm++;
         }
         if(vtx.hlp())
         {
            v[0].hlp=vtx.hlp(p.x);
            v[1].hlp=vtx.hlp(p.y);
            v[2].hlp=vtx.hlp(p.z);
            v[3].hlp=vtx.hlp(p.w);
         }
         if(vtx.tex0())
         {
            v[0].tex0=vtx.tex0(p.x);
            v[1].tex0=vtx.tex0(p.y);
            v[2].tex0=vtx.tex0(p.z);
            v[3].tex0=vtx.tex0(p.w);
         }
         if(vtx.tex1())
         {
            v[0].tex1=vtx.tex1(p.x);
            v[1].tex1=vtx.tex1(p.y);
            v[2].tex1=vtx.tex1(p.z);
            v[3].tex1=vtx.tex1(p.w);
         }
         if(vtx.tex2())
         {
            v[0].tex2=vtx.tex2(p.x);
            v[1].tex2=vtx.tex2(p.y);
            v[2].tex2=vtx.tex2(p.z);
            v[3].tex2=vtx.tex2(p.w);
         }
         if(vtx.tex3())
         {
            v[0].tex3=vtx.tex3(p.x);
            v[1].tex3=vtx.tex3(p.y);
            v[2].tex3=vtx.tex3(p.z);
            v[3].tex3=vtx.tex3(p.w);
         }
         if(vtx.blend() && vtx.matrix())
         {
            v[0].blend =vtx.blend (p.x);
            v[1].blend =vtx.blend (p.y);
            v[2].blend =vtx.blend (p.z);
            v[3].blend =vtx.blend (p.w);
            v[0].matrix=vtx.matrix(p.x);
            v[1].matrix=vtx.matrix(p.y);
            v[2].matrix=vtx.matrix(p.z);
            v[3].matrix=vtx.matrix(p.w);
         }
         if(vtx.size())
         {
            v[0].size=vtx.size(p.x);
            v[1].size=vtx.size(p.y);
            v[2].size=vtx.size(p.z);
            v[3].size=vtx.size(p.w);
         }
         if(vtx.material())
         {
            v[0].material=vtx.material(p.x);
            v[1].material=vtx.material(p.y);
            v[2].material=vtx.material(p.z);
            v[3].material=vtx.material(p.w);
         }
         if(vtx.color())
         {
            v[0].color=vtx.color(p.x);
            v[1].color=vtx.color(p.y);
            v[2].color=vtx.color(p.z);
            v[3].color=vtx.color(p.w);
         }
         //REP(4)ProcessVtx(v[i]);
         VI.face(v[0], v[1], v[2], v[3]);
      }
      VI.end  ();
      D .alpha(alpha);
   }
}
/******************************************************************************/
void MeshRender::drawBoneHighlight(Int bone, Shader *shader)C
{
   SPSet("BoneHighlight", bone+1); shader->commit();
   draw();
/* Bone Splits
   Int ind_offset=0;
   FREP(_bone_splits)
   {
      MeshRender::BoneSplit &bs=_bone_split[i];
      SPSet("BoneHighlight", bs.realToSplit(bone+1));
      if(Matrixes>1)SetMatrixVelSplit(bs.split_to_real, bs.bones); shader->commit(); // commit matrix changes to the shader
      drawRange(bs.tris, ind_offset);
      ind_offset+=bs.tris*3;
   }
   if(Matrixes>1)SetMatrixVelRestore(); shader->commit(); // restore default matrix set and commit matrix changes to the shader*/
}
/******************************************************************************/
// INSTANCED DRAWING
/******************************************************************************/
static void NewInstance(Memc<ShaderDraw> &shader_draws, Int instance_index, ShaderBase &shader, Material::MaterialShader *cur, C MeshPart &mesh, C MeshPart::Variation &variation)
{
   ShaderMaterial *shader_material;
again:
   if(cur->shader!=&shader) // check if we already have a match
   {
      if(cur->shader) // if this shader is specified
      {
         if(cur->next_material_shader>=0){cur=&MaterialShaders[cur->next_material_shader]; goto again;} // !! we can access 'next_material_shader' only when knowing that "shader!=null", because only 'shader' is cleared in 'MaterialShader.unlink' !! check if there's another one, we could check this before "if(cur->shader)", however we prioritize cases where Materials have only one Shader (the most common scenario)
         cur->next_material_shader=MaterialShaders.elms();
         cur=&MaterialShaders.New();
      }
      cur->shader=&shader;
      cur->next_material_shader=-1;
      cur->shader_material=ShaderMaterials.elms();

      // link Shader<->ShaderDraw
      ShaderDraw *shader_draw;
      if(  shader.shader_draw<0)
      {
         shader.shader_draw=shader_draws.elms();
         shader_draw=&shader_draws.New();
         shader_draw->shader               =&shader;
         shader_draw->first_shader_material= ShaderMaterials.elms();
         goto shader_draw_initialized; // we've created a new 'ShaderDraw', which means it doesn't have any 'ShaderMaterial's yet, so we can skip 'next_shader_material' linking, also skip setting 'shader_draw' address because we already have it
      }
      shader_draw=&shader_draws[shader.shader_draw];
      ShaderMaterials[shader_draw->last_shader_material].next_shader_material=ShaderMaterials.elms();
   shader_draw_initialized:
      shader_draw->last_shader_material=ShaderMaterials.elms();

      // we've linked Material->Shader, so let's create Shader->Material
      shader_material=&ShaderMaterials.New();
      shader_material-> next_shader_material     =-1;
      shader_material->first_shader_material_mesh= ShaderMaterialMeshes.elms();
   #if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
      if(..)shader_material->material=&material; // have to pass as parameter and can't use variation.material because this can be variation.getMaterial or variation.getShadowMaterial, also can't check for mesh._umm because this can be called for shadows which ignores it
      else  shader_material->umm     =&     umm;
   #endif
      goto shader_material_initialized; // we've created a new 'ShaderMaterial', which means it doesn't have any 'ShaderMaterialMesh'es yet, so we can skip 'next_shader_material_mesh' linking, also skip setting 'shader_material' address because we already have it
   }
   shader_material=&ShaderMaterials[cur->shader_material];
   ShaderMaterialMeshes[shader_material->last_shader_material_mesh].next_shader_material_mesh=ShaderMaterialMeshes.elms();
shader_material_initialized:
   shader_material->last_shader_material_mesh=ShaderMaterialMeshes.elms();
   ShaderMaterialMesh   &shader_material_mesh=ShaderMaterialMeshes.New();
   shader_material_mesh.next_shader_material_mesh=-1;
   shader_material_mesh.first_instance           =instance_index;
   shader_material_mesh.mesh                     =&mesh;
#if !SUPPORT_MATERIAL_CHANGE_IN_RENDERING
   #if INSTANCE_PTR
      shader_material_mesh.variation  =&variation;
   #else
      shader_material_mesh.variation_1=Renderer._mesh_variation_1;
   #endif
#endif
}
/******************************************************************************/
INLINE void SkeletonInstance::set(C AnimatedSkeleton &anim_skel)
{
   T.anim_skel=&anim_skel;
   T.skel_shader.shader=null;
   T.skel_shader.next_skeleton_shader=-1;
#if COUNT_MATERIAL_USAGE
   T.skel_shader.material.material=null;
#endif
}
INLINE void SkeletonBlendInstance::set(C AnimatedSkeleton &anim_skel)
{
   T.anim_skel=&anim_skel;
   T.skel_shader.shader=null;
   T.skel_shader.next_skeleton_shader=-1;
#if COUNT_MATERIAL_USAGE
   T.skel_shader.material.material=null;
#endif
}

SkeletonInstance& SkeletonInstances::getSkeletonInstance(C AnimatedSkeleton &anim_skel, Int &instance_index)
{
   // already exists
   if(instance_index>=0)return T[instance_index];

   // add new
   instance_index=elms();
   SkeletonInstance &instance=New();
   instance.set(anim_skel);
   return instance;
}
SkeletonBlendInstance& BlendInstancesClass::getSkeletonInstance(C AnimatedSkeleton &anim_skel)
{
   // already exists
   if(anim_skel._instance.blend>=0)return T[anim_skel._instance.blend].skeleton;

   // add new
   anim_skel._instance.blend=elms();
   BlendInstance &instance=New();
   instance.type=BlendInstance::SKELETON;
   instance.setZ(anim_skel.pos());
   instance.skeleton.set(anim_skel);
   return instance.skeleton;
}

void SkeletonInstance::newInstance(ShaderBase &shader, C Material &material, Memc<SkeletonShaderMaterialMeshInstance> &instances)
{
   SkeletonShader         *cur_shader=&skel_shader;
   SkeletonShaderMaterial *cur_material;
again_shader:
   if(cur_shader->shader!=&shader) // check if we already have a match
   {
      if(cur_shader->shader) // if this shader is specified
      {
         if(cur_shader->next_skeleton_shader>=0){cur_shader=&SkeletonShaders[cur_shader->next_skeleton_shader]; goto again_shader;}
            cur_shader->next_skeleton_shader =SkeletonShaders.elms();
            cur_shader=&SkeletonShaders.New();
      }
      cur_shader->shader=&shader;
      cur_shader->next_skeleton_shader=-1;
      cur_material=&cur_shader->material;
      goto set_material;
   }else // found an existing shader
   {  // iterate materials
      cur_material=&cur_shader->material;
   again_material:
      if(cur_material->material!=&material) // check if we already have a match
      {
         if(cur_material->material) // if this material is specified
         {
            if(cur_material->next_skeleton_shader_material>=0){cur_material=&SkeletonShaderMaterials[cur_material->next_skeleton_shader_material]; goto again_material;}
               cur_material->next_skeleton_shader_material=SkeletonShaderMaterials.elms();
               cur_material=&SkeletonShaderMaterials.New();
         }
      set_material:
         cur_material->material=&material; material.incUsage();
         cur_material->next_skeleton_shader_material=-1;
         cur_material->first_mesh_instance=
         cur_material-> last_mesh_instance=instances.elms();
      }else // found an existing material
      {
         instances[cur_material->last_mesh_instance].next_instance=instances.elms();
                   cur_material->last_mesh_instance               =instances.elms();
      }
   }
}
void SkeletonBlendInstance::newInstance(ShaderBase &shader, C Material &material, UInt type)
{
   SkeletonBlendShader    *cur_shader=&skel_shader;
   SkeletonShaderMaterial *cur_material;
again_shader:
   if(cur_shader->shader!=&shader) // check if we already have a match
   {
      if(cur_shader->shader) // if this shader is specified
      {
         if(cur_shader->next_skeleton_shader>=0){cur_shader=&SkeletonBlendShaders[cur_shader->next_skeleton_shader]; goto again_shader;}
            cur_shader->next_skeleton_shader =SkeletonBlendShaders.elms();
            cur_shader=&SkeletonBlendShaders.New();
      }
      cur_shader->shader=&shader;
      cur_shader->next_skeleton_shader=-1;
      cur_shader->type=type;
      cur_material=&cur_shader->material;
      goto set_material;
   }else // found an existing shader
   {  // iterate materials
      cur_material=&cur_shader->material;
   again_material:
      if(cur_material->material!=&material) // check if we already have a match
      {
         if(cur_material->material) // if this material is specified
         {
            if(cur_material->next_skeleton_shader_material>=0){cur_material=&SkeletonBlendShaderMaterials[cur_material->next_skeleton_shader_material]; goto again_material;}
               cur_material->next_skeleton_shader_material=SkeletonBlendShaderMaterials.elms();
               cur_material=&SkeletonBlendShaderMaterials.New();
         }
      set_material:
         cur_material->material=&material; material.incUsage();
         cur_material->next_skeleton_shader_material=-1;
         cur_material->first_mesh_instance=
         cur_material-> last_mesh_instance=SkeletonBlendShaderMaterialMeshInstances.elms();
      }else // found an existing material
      {
         SkeletonBlendShaderMaterialMeshInstances[cur_material->last_mesh_instance].next_instance=SkeletonBlendShaderMaterialMeshInstances.elms();
                                                  cur_material->last_mesh_instance               =SkeletonBlendShaderMaterialMeshInstances.elms();
      }
   }
}

INLINE void SkeletonBlendInstance::addBlend(Shader &shader, C Material &material, C MeshPart &mesh)
{
   newInstance(shader, material, BlendInstance::STATIC);
   SkeletonBlendShaderMaterialMeshInstances.New().set(mesh);
}
INLINE void SkeletonBlendInstance::addBlend(BLST &blst, C Material &material, C MeshPart &mesh)
{
   newInstance((ShaderBase&)blst, material, BlendInstance::STATIC_BLST);
   SkeletonBlendShaderMaterialMeshInstances.New().set(mesh);
}
INLINE void SkeletonBlendInstance::addFur(Shader &shader, C Material &material, C MeshPart &mesh)
{
   newInstance(shader, material, BlendInstance::STATIC_FUR);
   SkeletonBlendShaderMaterialMeshInstances.New().set(mesh);
}
/******************************************************************************/
INLINE EarlyZInstance& EarlyZInstance::set(C MeshRender &mesh)
{
   T.mesh=&mesh;
   return T;
}
INLINE void SkeletonShaderMaterialMeshInstance::set(C MeshRender &mesh)
{
   T.next_instance       =-1;
   T.mesh                =&mesh;
   T.shader_param_changes= Renderer._shader_param_changes;
}
INLINE void SkeletonOpaqueShaderMaterialMeshInstance::set(C MeshRender &mesh)
{
   super::set(mesh);
   T.highlight=Renderer._mesh_highlight;
}
INLINE void SkeletonBlendShaderMaterialMeshInstance::set(C MeshRender &mesh)
{
   super::set(mesh);
   T.stencil_mode=(STENCIL_MODE)Renderer._mesh_stencil_mode;
}
INLINE EmissiveInstance& EmissiveInstance::set(C MeshPart &mesh, C MeshPart::Variation &variation)
{
   T.mesh=&mesh;
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
   T.shader  =variation.shader[RM_EMISSIVE];
   T.material=variation.material();
#else
   T.variation=&variation;
#endif
   T.shader_param_changes=Renderer._shader_param_changes;
   variation.material->incUsage(); // for ambient, material will always be != null, because only materials with ambient value can create ambient instances
   return T;
}
INLINE void SkeletonEmissiveInstance::set(C MeshPart &mesh, C MeshPart::Variation &variation, C AnimatedSkeleton &anim_skel)
{
   T.mesh=&mesh;
#if SUPPORT_MATERIAL_CHANGE_IN_RENDERING
   T.shader  =variation.shader[RM_EMISSIVE];
   T.material=variation.material();
#else
   T.variation=&variation;
#endif
   T.anim_skel=&anim_skel;
   T.shader_param_changes=Renderer._shader_param_changes;
   variation.material->incUsage(); // for ambient, material will always be != null, because only materials with ambient value can create ambient instances
}
INLINE OpaqueShaderMaterialMeshInstance& OpaqueShaderMaterialMeshInstance::set()
{
   T.next_instance       =-1;
   T.highlight           =Renderer._mesh_highlight;
   T.stencil_value       =Renderer._mesh_stencil_value;
   T.shader_param_changes=Renderer._shader_param_changes;
   return T;
}
INLINE ShadowShaderMaterialMeshInstance& ShadowShaderMaterialMeshInstance::set()
{
   T.next_instance=-1;
   T.shader_param_changes=Renderer._shader_param_changes;
   return T;
}
/******************************************************************************/
BlendInstance::~BlendInstance()
{
#if COUNT_MATERIAL_USAGE
   switch(type)
   {
      case STATIC     :
      case STATIC_BLST:
      case STATIC_FUR : s.material->decUsage(); break;

      case SKELETON: DTOR(skeleton); break;
   }
#endif
}
void BlendInstance::unlink() {if(type==SKELETON)skeleton.unlinkBlend();}

INLINE void BlendInstance::setZ     (C VecD    &pos                           ) {                                                          setViewZ(DistPointActiveCamPlaneZ(pos));}
INLINE void BlendInstance::setMatrix(C MatrixM &matrix                        ) {SetViewMatrix(stat.view_matrix.cur, matrix             ); viewMatrixChanged();} // here 'matrix_prev' is ignored because it's assumed to be unused
INLINE void BlendInstance::setMatrix(C MatrixM &matrix, C MatrixM &matrix_prev) {SetViewMatrix(stat.view_matrix    , matrix, matrix_prev); viewMatrixChanged();}

INLINE BlendInstance& BlendInstancesClass::add(Shader &shader, C Material &material, C MeshPart &mesh, C MeshPart::Variation &variation)
{
   BlendInstance &obj=New(); obj.type=BlendInstance::STATIC;
#if 1
   obj.stat.shader   =&shader;
   obj.stat.material =&material;
#else
   obj.stat.variation=&variation;
#endif
   obj.stat.mesh     =&mesh;
   obj.stat.highlight           =              Renderer._mesh_highlight;
   obj.stat.stencil_mode        =(STENCIL_MODE)Renderer._mesh_stencil_mode;
   obj.stat.shader_param_changes=              Renderer._shader_param_changes;
   material.incUsage();
   return obj;
}
INLINE BlendInstance& BlendInstancesClass::add(BLST &blst, C Material &material, C MeshPart &mesh, C MeshPart::Variation &variation)
{
   BlendInstance &obj=New(); obj.type=BlendInstance::STATIC_BLST;
#if 1
   obj.stat.blst     =&blst;
   obj.stat.material =&material;
#else
   obj.stat.variation=&variation;
#endif
   obj.stat.mesh                =&mesh;
   obj.stat.highlight           =              Renderer._mesh_highlight;
   obj.stat.stencil_mode        =(STENCIL_MODE)Renderer._mesh_stencil_mode;
   obj.stat.shader_param_changes=              Renderer._shader_param_changes;
   material.incUsage();
   return obj;
}
INLINE BlendInstance& BlendInstancesClass::addFur(Shader &shader, C Material &material, C MeshPart &mesh, C MeshPart::Variation &variation)
{
   BlendInstance &obj=New(); obj.type=BlendInstance::STATIC_FUR;
#if 1
   obj.stat.shader   =&shader;
   obj.stat.material =&material;
#else
   obj.stat.variation=&variation;
#endif
   obj.stat.mesh                =&mesh;
   obj.stat.highlight           =Renderer._mesh_highlight;
 //obj.stat.stencil_mode        =Renderer._mesh_stencil_mode; ignored for fur
   obj.stat.shader_param_changes=Renderer._shader_param_changes;
   material.incUsage();
   return obj;
}
INLINE void BlendInstancesClass::add(BlendObject        &blend_obj, C VecD &pos) {BlendInstance &obj=New(); obj.type=BlendInstance::BLEND_OBJ ; obj.blend_obj =&blend_obj ; obj.setZ(pos);}
       void BlendInstancesClass::add(  Game::Obj        & game_obj             ) {BlendInstance &obj=New(); obj.type=BlendInstance:: GAME_OBJ ; obj. game_obj =& game_obj ; obj.setZ(game_obj.pos());}
       void BlendInstancesClass::add(  Game::Area::Data & game_area            ) {BlendInstance &obj=New(); obj.type=BlendInstance:: GAME_AREA; obj. game_area=& game_area;
   Game::Area &area=game_area.area(); Flt area_size=area.world()->areaSize();
   obj.setZ(VecD((area.xz().x+0.5f)*area_size, ActiveCam.matrix.pos.y, (area.xz().y+0.5f)*area_size));
}
void BlendObject::scheduleDrawBlend(C VecD &pos) {if(Renderer.firstPass())BlendInstances.add(T, pos);}
/******************************************************************************/
void MeshPart::draw(C MatrixM &matrix, C MatrixM &matrix_prev)C
{
   DEBUG_ASSERT(Renderer()==RM_PREPARE, "'MeshPart.draw' called outside of RM_PREPARE");
   if(_draw_mask&Renderer._mesh_draw_mask)
   {
    C Variation &variation=getVariation();
      switch(Renderer._cur_type)
      {
         case RT_DEFERRED:
         {
            if(Shader *shader=variation.shader[Renderer._opaque_mode_index])
            {
            #if SUPPORT_EARLY_Z
               if(Renderer._mesh_early_z)if(Shader *shader=variation.shader[RM_EARLY_Z])
               {
                  if(shader==Renderer._shader_early_z)SetViewMatrix(EarlyZInstances[variation.getMaterial().cull].New().set(render).view_matrix, matrix);
               }
            #endif

               if(variation.last_opaque_instance<0)
               {
                  if(_umm)NewInstance(MultiMaterialShaderDraws, OpaqueShaderMaterialMeshInstances.elms(), *shader, &          _umm         ->       material_shader, T, variation);
                  else    NewInstance(             ShaderDraws, OpaqueShaderMaterialMeshInstances.elms(), *shader, &variation.getMaterial()._opaque_material_shader, T, variation);
               }else                                            OpaqueShaderMaterialMeshInstances[variation.last_opaque_instance].next_instance=OpaqueShaderMaterialMeshInstances.elms();
                                                                                                  variation.last_opaque_instance               =OpaqueShaderMaterialMeshInstances.elms();
               SetViewMatrix(OpaqueShaderMaterialMeshInstances.New().set().view_matrix, matrix, matrix_prev);

            #if SUPPORT_EMISSIVE
               if(Shader *shader=variation.shader[RM_EMISSIVE])SetViewMatrix(EmissiveInstances.New().set(T, variation).view_matrix, matrix);
            #endif
            }else
            if(Shader *shader=variation.shader[RM_BLEND])BlendInstances.add(*shader        , variation.getMaterial(), T, variation).setMatrix(matrix, matrix_prev);else
            if(               variation.blst            )BlendInstances.add(*variation.blst, variation.getMaterial(), T, variation).setMatrix(matrix, matrix_prev);

            if(Shader *shader=variation.shader[RM_FUR])BlendInstances.addFur(*shader, variation.getMaterial(), T, variation).setMatrix(matrix, matrix_prev);
         }break;
         
         case RT_FORWARD: // velocities not needed for RT_FORWARD
         {
         #if SUPPORT_EARLY_Z
            if(Renderer._mesh_early_z && Renderer.firstPass())if(Shader *shader=variation.shader[RM_EARLY_Z])
            {
               if(shader==Renderer._shader_early_z)SetViewMatrix(EarlyZInstances[variation.getMaterial().cull].New().set(render).view_matrix, matrix);
            }
         #endif

            if(variation.frst)
            {
               if(Renderer.firstPass() || variation.frst->all_passes)//if(Shader *shader=_frst->getShader())
               {
                  if(variation.last_opaque_instance<0)
                  {
                     if(_umm)NewInstance(MultiMaterialShaderDraws, OpaqueShaderMaterialMeshInstances.elms(), *variation.frst, &          _umm         ->       material_shader, T, variation);
                     else    NewInstance(             ShaderDraws, OpaqueShaderMaterialMeshInstances.elms(), *variation.frst, &variation.getMaterial()._opaque_material_shader, T, variation);
                  }else                                            OpaqueShaderMaterialMeshInstances[variation.last_opaque_instance].next_instance=OpaqueShaderMaterialMeshInstances.elms();
                                                                                                     variation.last_opaque_instance               =OpaqueShaderMaterialMeshInstances.elms();
                  SetViewMatrix(OpaqueShaderMaterialMeshInstances.New().set().view_matrix.cur, matrix);
               }
            }else
            if(Renderer.firstPass())
            {
               if(Shader *shader=variation.shader[RM_BLEND])BlendInstances.add(*shader        , variation.getMaterial(), T, variation).setMatrix(matrix);else
               if(               variation.blst            )BlendInstances.add(*variation.blst, variation.getMaterial(), T, variation).setMatrix(matrix);
            }
         }break;
      }
   }
}
/******************************************************************************/
void MeshPart::draw(C AnimatedSkeleton &anim_skel)C
{
   DEBUG_ASSERT(Renderer()==RM_PREPARE, "'MeshPart.draw' called outside of RM_PREPARE");
   if(_draw_mask&Renderer._mesh_draw_mask)
   {
    C Variation &variation=          getVariation();
    C Material  &material =variation.getMaterial ();
      switch(Renderer._cur_type)
      {
         case RT_DEFERRED:
         {
            if(Shader *shader=variation.shader[Renderer._opaque_mode_index])
            {
               SkeletonOpaqueInstances.getSkeletonInstanceOpaque(anim_skel)
                                      .newInstance(*shader, material, SkeletonOpaqueShaderMaterialMeshInstances);
               SkeletonOpaqueShaderMaterialMeshInstances.New().set(T);

            #if SUPPORT_EMISSIVE
               if(Shader *shader=variation.shader[RM_EMISSIVE])SkeletonEmissiveInstances.New().set(T, variation, anim_skel);
            #endif
            }else
            if(Shader *shader=variation.shader[RM_BLEND])BlendInstances.getSkeletonInstance(anim_skel).addBlend(*        shader, material, T);else
            if(               variation.blst            )BlendInstances.getSkeletonInstance(anim_skel).addBlend(*variation.blst, material, T);

            if(Shader *shader=variation.shader[RM_FUR])BlendInstances.getSkeletonInstance(anim_skel).addFur(*shader, material, T);
         }break;
         
         case RT_FORWARD:
         {
            if(variation.frst)
            {
               if(Renderer.firstPass() || variation.frst->all_passes)//if(Shader *shader=_frst->getShader())
               {
                  SkeletonOpaqueInstances.getSkeletonInstanceOpaque(anim_skel)
                                         .newInstance(*variation.frst, material, SkeletonOpaqueShaderMaterialMeshInstances);
                  SkeletonOpaqueShaderMaterialMeshInstances.New().set(T);
               }
            }else
            if(Renderer.firstPass())
            {
               if(Shader *shader=variation.shader[RM_BLEND])BlendInstances.getSkeletonInstance(anim_skel).addBlend(*        shader, material, T);else
               if(               variation.blst            )BlendInstances.getSkeletonInstance(anim_skel).addBlend(*variation.blst, material, T);
            }
         }break;
      }
   }
}
/******************************************************************************/
void MeshPart::draw(C AnimatedSkeleton &anim_skel, C Material &material)C
{
   DEBUG_ASSERT(Renderer()==RM_PREPARE, "'MeshPart.draw' called outside of RM_PREPARE");
   if(_draw_mask&Renderer._mesh_draw_mask)
   {
    C Variation &variation=          getVariation();
  //C Material  &material =variation.getMaterial (); use custom material instead
      switch(Renderer._cur_type)
      {
         case RT_DEFERRED:
         {
            if(Shader *shader=variation.shader[Renderer._opaque_mode_index])
            {
               SkeletonOpaqueInstances.getSkeletonInstanceOpaque(anim_skel)
                                      .newInstance(*shader, material, SkeletonOpaqueShaderMaterialMeshInstances);
               SkeletonOpaqueShaderMaterialMeshInstances.New().set(T);

            #if SUPPORT_EMISSIVE
               if(Shader *shader=variation.shader[RM_EMISSIVE])SkeletonEmissiveInstances.New().set(T, variation, anim_skel);
            #endif
            }else
            if(Shader *shader=variation.shader[RM_BLEND])BlendInstances.getSkeletonInstance(anim_skel).addBlend(*        shader, material, T);else
            if(               variation.blst            )BlendInstances.getSkeletonInstance(anim_skel).addBlend(*variation.blst, material, T);

            if(Shader *shader=variation.shader[RM_FUR])BlendInstances.getSkeletonInstance(anim_skel).addFur(*shader, material, T);
         }break;
         
         case RT_FORWARD:
         {
            if(variation.frst)
            {
               if(Renderer.firstPass() || variation.frst->all_passes)//if(Shader *shader=_frst->getShader())
               {
                  SkeletonOpaqueInstances.getSkeletonInstanceOpaque(anim_skel)
                                         .newInstance(*variation.frst, material, SkeletonOpaqueShaderMaterialMeshInstances);
                  SkeletonOpaqueShaderMaterialMeshInstances.New().set(T);
               }
            }else
            if(Renderer.firstPass())
            {
               if(Shader *shader=variation.shader[RM_BLEND])BlendInstances.getSkeletonInstance(anim_skel).addBlend(*        shader, material, T);else
               if(               variation.blst            )BlendInstances.getSkeletonInstance(anim_skel).addBlend(*variation.blst, material, T);
            }
         }break;
      }
   }
}
/******************************************************************************/
void MeshPart::drawShadow(C MatrixM &matrix)C
{
   DEBUG_ASSERT(Renderer()==RM_SHADOW, "'MeshPart.drawShadow' called outside of RM_SHADOW");
   if(_draw_mask&Renderer._mesh_draw_mask)
   {
    C Variation &variation=getVariation(); if(Shader *shader=variation.shader[RM_SHADOW])
      {
         if(variation.last_shadow_instance<0)
         {
          C Material &material=variation.getShadowMaterial(ReuseDefaultMaterialForNonSkinnedShadowShader(shader));
            NewInstance(ShaderDraws, ShadowShaderMaterialMeshInstances.elms(), *shader, &material._shadow_material_shader, T, variation);
         }else ShadowShaderMaterialMeshInstances[variation.last_shadow_instance].next_instance=ShadowShaderMaterialMeshInstances.elms();
                                                 variation.last_shadow_instance               =ShadowShaderMaterialMeshInstances.elms();
         SetViewMatrix(ShadowShaderMaterialMeshInstances.New().set().view_matrix, matrix);
      }
   }
}
/******************************************************************************/
void MeshPart::drawShadow(C AnimatedSkeleton &anim_skel)C
{
   DEBUG_ASSERT(Renderer()==RM_SHADOW, "'MeshPart.drawShadow' called outside of RM_SHADOW");
   if(_draw_mask&Renderer._mesh_draw_mask)
   {
    C Variation &variation=getVariation(); if(Shader *shader=variation.shader[RM_SHADOW])
      {
         SkeletonShadowInstances.getSkeletonInstanceShadow(anim_skel)
                                .newInstance(*shader, variation.getShadowMaterial(ReuseDefaultMaterialForSkinnedShadowShader(shader)), SkeletonShadowShaderMaterialMeshInstances);
         SkeletonShadowShaderMaterialMeshInstances.New().set(T);
      }
   }
}
void MeshPart::drawShadow(C AnimatedSkeleton &anim_skel, C Material &material)C
{
   DEBUG_ASSERT(Renderer()==RM_SHADOW, "'MeshPart.drawShadow' called outside of RM_SHADOW");
   if(_draw_mask&Renderer._mesh_draw_mask)
   {
    C Variation &variation=getVariation(); if(Shader *shader=variation.shader[RM_SHADOW])
      {
         SkeletonShadowInstances.getSkeletonInstanceShadow(anim_skel)
                                .newInstance(*shader, GetShadowMaterial(&material, ReuseDefaultMaterialForSkinnedShadowShader(shader)), SkeletonShadowShaderMaterialMeshInstances);
         SkeletonShadowShaderMaterialMeshInstances.New().set(T);
      }
   }
}
/******************************************************************************/
void MeshLod::draw()C
{
   DEBUG_ASSERT(Renderer()==RM_PREPARE, "'MeshLod.draw' called outside of RM_PREPARE");
   Matrix view_matrix; SetViewMatrix(view_matrix);

   switch(Renderer._cur_type)
   {
      case RT_DEFERRED:
      {
         Matrix view_matrix_prev; SetViewMatrixPrev(view_matrix_prev);
         FREPA(T) // process in order
         {
          C MeshPart &part=parts[i]; if(part._draw_mask&Renderer._mesh_draw_mask)
            {
             C MeshPart::Variation &variation=part.getVariation();

               if(Shader *shader=variation.shader[Renderer._opaque_mode_index])
               {
               #if SUPPORT_EARLY_Z
                  if(Renderer._mesh_early_z)if(Shader *shader=variation.shader[RM_EARLY_Z])
                  {
                     if(shader==Renderer._shader_early_z)EarlyZInstances[variation.getMaterial().cull].New().set(part.render).view_matrix=view_matrix;
                  }
               #endif

                  if(variation.last_opaque_instance<0)
                  {
                     if(part._umm)NewInstance(MultiMaterialShaderDraws, OpaqueShaderMaterialMeshInstances.elms(), *shader, &     part._umm         ->       material_shader, part, variation);
                     else         NewInstance(             ShaderDraws, OpaqueShaderMaterialMeshInstances.elms(), *shader, &variation.getMaterial()._opaque_material_shader, part, variation);
                  }else                                                 OpaqueShaderMaterialMeshInstances[variation.last_opaque_instance].next_instance=OpaqueShaderMaterialMeshInstances.elms();
                                                                                                          variation.last_opaque_instance               =OpaqueShaderMaterialMeshInstances.elms();
                  OpaqueShaderMaterialMeshInstances.New().set().view_matrix.set(view_matrix, view_matrix_prev);

               #if SUPPORT_EMISSIVE
                  if(Shader *shader=variation.shader[RM_EMISSIVE])EmissiveInstances.New().set(part, variation).view_matrix=view_matrix;
               #endif
               }else
               if(Shader *shader=variation.shader[RM_BLEND])BlendInstances.add(*shader        , variation.getMaterial(), part, variation).setViewMatrix(view_matrix, view_matrix_prev);else
               if(               variation.blst            )BlendInstances.add(*variation.blst, variation.getMaterial(), part, variation).setViewMatrix(view_matrix, view_matrix_prev);

               if(Shader *shader=variation.shader[RM_FUR])BlendInstances.addFur(*shader, variation.getMaterial(), part, variation).setViewMatrix(view_matrix, view_matrix_prev);
            }
         }
      }break;
      
      case RT_FORWARD: // velocities not needed for RT_FORWARD
      {
         FREPA(T) // process in order
         {
          C MeshPart &part=parts[i]; if(part._draw_mask&Renderer._mesh_draw_mask)
            {
             C MeshPart::Variation &variation=part.getVariation();

            #if SUPPORT_EARLY_Z
               if(Renderer._mesh_early_z && Renderer.firstPass())if(Shader *shader=variation.shader[RM_EARLY_Z])
               {
                  if(shader==Renderer._shader_early_z)EarlyZInstances[variation.getMaterial().cull].New().set(part.render).view_matrix=view_matrix;
               }
            #endif

               if(variation.frst)
               {
                  if(Renderer.firstPass() || variation.frst->all_passes)//if(Shader *shader=variation.frst->getShader())
                  {
                     if(variation.last_opaque_instance<0)
                     {
                        if(part._umm)NewInstance(MultiMaterialShaderDraws, OpaqueShaderMaterialMeshInstances.elms(), *variation.frst, &     part._umm         ->       material_shader, part, variation);
                        else         NewInstance(             ShaderDraws, OpaqueShaderMaterialMeshInstances.elms(), *variation.frst, &variation.getMaterial()._opaque_material_shader, part, variation);
                     }else                                                 OpaqueShaderMaterialMeshInstances[variation.last_opaque_instance].next_instance=OpaqueShaderMaterialMeshInstances.elms();
                                                                                                             variation.last_opaque_instance               =OpaqueShaderMaterialMeshInstances.elms();
                     OpaqueShaderMaterialMeshInstances.New().set().view_matrix.cur=view_matrix;
                  }
               }else
               if(Renderer.firstPass())
               {
                  if(Shader *shader=variation.shader[RM_BLEND])BlendInstances.add(*shader        , variation.getMaterial(), part, variation).setViewMatrix(view_matrix);else
                  if(               variation.blst            )BlendInstances.add(*variation.blst, variation.getMaterial(), part, variation).setViewMatrix(view_matrix);
               }
            }
         }
      }break;
   }
}
/******************************************************************************/
void MeshLod::draw(C MatrixM &matrix, C MatrixM &matrix_prev)C
{
   DEBUG_ASSERT(Renderer()==RM_PREPARE, "'MeshLod.draw' called outside of RM_PREPARE");
   Matrix view_matrix; SetViewMatrix(view_matrix, matrix);

   switch(Renderer._cur_type)
   {
      case RT_DEFERRED:
      {
         Matrix view_matrix_prev; SetViewMatrixPrev(view_matrix_prev, matrix_prev);
         FREPA(T) // process in order
         {
          C MeshPart &part=parts[i]; if(part._draw_mask&Renderer._mesh_draw_mask)
            {
             C MeshPart::Variation &variation=part.getVariation();

               if(Shader *shader=variation.shader[Renderer._opaque_mode_index])
               {
               #if SUPPORT_EARLY_Z
                  if(Renderer._mesh_early_z)if(Shader *shader=variation.shader[RM_EARLY_Z])
                  {
                     if(shader==Renderer._shader_early_z)EarlyZInstances[variation.getMaterial().cull].New().set(part.render).view_matrix=view_matrix;
                  }
               #endif

                  if(variation.last_opaque_instance<0)
                  {
                     if(part._umm)NewInstance(MultiMaterialShaderDraws, OpaqueShaderMaterialMeshInstances.elms(), *shader, &     part._umm         ->       material_shader, part, variation);
                     else         NewInstance(             ShaderDraws, OpaqueShaderMaterialMeshInstances.elms(), *shader, &variation.getMaterial()._opaque_material_shader, part, variation);
                  }else                                                 OpaqueShaderMaterialMeshInstances[variation.last_opaque_instance].next_instance=OpaqueShaderMaterialMeshInstances.elms();
                                                                                                          variation.last_opaque_instance               =OpaqueShaderMaterialMeshInstances.elms();
                  OpaqueShaderMaterialMeshInstances.New().set().view_matrix.set(view_matrix, view_matrix_prev);

               #if SUPPORT_EMISSIVE
                  if(Shader *shader=variation.shader[RM_EMISSIVE])EmissiveInstances.New().set(part, variation).view_matrix=view_matrix;
               #endif
               }else
               if(Shader *shader=variation.shader[RM_BLEND])BlendInstances.add(*shader        , variation.getMaterial(), part, variation).setViewMatrix(view_matrix, view_matrix_prev);else
               if(               variation.blst            )BlendInstances.add(*variation.blst, variation.getMaterial(), part, variation).setViewMatrix(view_matrix, view_matrix_prev);

               if(Shader *shader=variation.shader[RM_FUR])BlendInstances.addFur(*shader, variation.getMaterial(), part, variation).setViewMatrix(view_matrix, view_matrix_prev);
            }
         }
      }break;
      
      case RT_FORWARD: // velocities not needed for RT_FORWARD
      {
         FREPA(T) // process in order
         {
          C MeshPart &part=parts[i]; if(part._draw_mask&Renderer._mesh_draw_mask)
            {
             C MeshPart::Variation &variation=part.getVariation();

            #if SUPPORT_EARLY_Z
               if(Renderer._mesh_early_z && Renderer.firstPass())if(Shader *shader=variation.shader[RM_EARLY_Z])
               {
                  if(shader==Renderer._shader_early_z)EarlyZInstances[variation.getMaterial().cull].New().set(part.render).view_matrix=view_matrix;
               }
            #endif

               if(variation.frst)
               {
                  if(Renderer.firstPass() || variation.frst->all_passes)//if(Shader *shader=variation.frst->getShader())
                  {
                     if(variation.last_opaque_instance<0)
                     {
                        if(part._umm)NewInstance(MultiMaterialShaderDraws, OpaqueShaderMaterialMeshInstances.elms(), *variation.frst, &     part._umm         ->       material_shader, part, variation);
                        else         NewInstance(             ShaderDraws, OpaqueShaderMaterialMeshInstances.elms(), *variation.frst, &variation.getMaterial()._opaque_material_shader, part, variation);
                     }else                                                 OpaqueShaderMaterialMeshInstances[variation.last_opaque_instance].next_instance=OpaqueShaderMaterialMeshInstances.elms();
                                                                                                             variation.last_opaque_instance               =OpaqueShaderMaterialMeshInstances.elms();
                     OpaqueShaderMaterialMeshInstances.New().set().view_matrix.cur=view_matrix;
                  }
               }else
               if(Renderer.firstPass())
               {
                  if(Shader *shader=variation.shader[RM_BLEND])BlendInstances.add(*shader        , variation.getMaterial(), part, variation).setViewMatrix(view_matrix);else
                  if(               variation.blst            )BlendInstances.add(*variation.blst, variation.getMaterial(), part, variation).setViewMatrix(view_matrix);
               }
            }
         }
      }break;
   }
}
/******************************************************************************/
void MeshLod::draw(C AnimatedSkeleton &anim_skel)C
{
   DEBUG_ASSERT(Renderer()==RM_PREPARE, "'MeshLod.draw' called outside of RM_PREPARE");
   SkeletonInstance      *opaque_skeleton=null;
   SkeletonBlendInstance * blend_skeleton=null;
   switch(Renderer._cur_type)
   {
      case RT_DEFERRED:
      {
         FREPA(T) // process in order
         {
          C MeshPart &part=parts[i]; if(part._draw_mask&Renderer._mesh_draw_mask)
            {
             C MeshPart::Variation &variation=part     .getVariation();
             C Material            &material =variation.getMaterial ();

               if(Shader *shader=variation.shader[Renderer._opaque_mode_index])
               {
                  if(!opaque_skeleton)opaque_skeleton=&SkeletonOpaqueInstances.getSkeletonInstanceOpaque(anim_skel);
                                      opaque_skeleton->newInstance(*shader, material, SkeletonOpaqueShaderMaterialMeshInstances);
                  SkeletonOpaqueShaderMaterialMeshInstances.New().set(part);

               #if SUPPORT_EMISSIVE
                  if(Shader *shader=variation.shader[RM_EMISSIVE])SkeletonEmissiveInstances.New().set(part, variation, anim_skel);
               #endif
               }else
               if(Shader *shader=variation.shader[RM_BLEND]){if(!blend_skeleton)blend_skeleton=&BlendInstances.getSkeletonInstance(anim_skel); blend_skeleton->addBlend(*        shader, material, part);}else
               if(               variation.blst            ){if(!blend_skeleton)blend_skeleton=&BlendInstances.getSkeletonInstance(anim_skel); blend_skeleton->addBlend(*variation.blst, material, part);}

               if(Shader *shader=variation.shader[RM_FUR]){if(!blend_skeleton)blend_skeleton=&BlendInstances.getSkeletonInstance(anim_skel); blend_skeleton->addFur(*shader, material, part);}
            }
         }
      }break;
      
      case RT_FORWARD:
      {
         FREPA(T) // process in order
         {
          C MeshPart &part=parts[i]; if(part._draw_mask&Renderer._mesh_draw_mask)
            {
             C MeshPart::Variation &variation=part     .getVariation();
             C Material            &material =variation.getMaterial ();

               if(variation.frst)
               {
                  if(Renderer.firstPass() || variation.frst->all_passes)//if(Shader *shader=variation.frst->getShader())
                  {
                     if(!opaque_skeleton)opaque_skeleton=&SkeletonOpaqueInstances.getSkeletonInstanceOpaque(anim_skel);
                                         opaque_skeleton->newInstance(*variation.frst, material, SkeletonOpaqueShaderMaterialMeshInstances);
                     SkeletonOpaqueShaderMaterialMeshInstances.New().set(part);
                  }
               }else
               if(Renderer.firstPass())
               {
                  if(Shader *shader=variation.shader[RM_BLEND]){if(!blend_skeleton)blend_skeleton=&BlendInstances.getSkeletonInstance(anim_skel); blend_skeleton->addBlend(*        shader, material, part);}else
                  if(               variation.blst            ){if(!blend_skeleton)blend_skeleton=&BlendInstances.getSkeletonInstance(anim_skel); blend_skeleton->addBlend(*variation.blst, material, part);}
               }
            }
         }
      }break;
   }
}
/******************************************************************************/
void MeshLod::draw(C AnimatedSkeleton &anim_skel, C Material &material)C
{
   DEBUG_ASSERT(Renderer()==RM_PREPARE, "'MeshLod.draw' called outside of RM_PREPARE");
   SkeletonInstance      *opaque_skeleton=null;
   SkeletonBlendInstance * blend_skeleton=null;
   switch(Renderer._cur_type)
   {
      case RT_DEFERRED:
      {
         FREPA(T) // process in order
         {
          C MeshPart &part=parts[i]; if(part._draw_mask&Renderer._mesh_draw_mask)
            {
             C MeshPart::Variation &variation=part     .getVariation();
           //C Material            &material =variation.getMaterial (); use custom material instead

               if(Shader *shader=variation.shader[Renderer._opaque_mode_index])
               {
                  if(!opaque_skeleton)opaque_skeleton=&SkeletonOpaqueInstances.getSkeletonInstanceOpaque(anim_skel);
                                      opaque_skeleton->newInstance(*shader, material, SkeletonOpaqueShaderMaterialMeshInstances);
                  SkeletonOpaqueShaderMaterialMeshInstances.New().set(part);

               #if SUPPORT_EMISSIVE
                  if(Shader *shader=variation.shader[RM_EMISSIVE])SkeletonEmissiveInstances.New().set(part, variation, anim_skel);
               #endif
               }else
               if(Shader *shader=variation.shader[RM_BLEND]){if(!blend_skeleton)blend_skeleton=&BlendInstances.getSkeletonInstance(anim_skel); blend_skeleton->addBlend(*        shader, material, part);}else
               if(               variation.blst            ){if(!blend_skeleton)blend_skeleton=&BlendInstances.getSkeletonInstance(anim_skel); blend_skeleton->addBlend(*variation.blst, material, part);}

               if(Shader *shader=variation.shader[RM_FUR]){if(!blend_skeleton)blend_skeleton=&BlendInstances.getSkeletonInstance(anim_skel); blend_skeleton->addFur(*shader, material, part);}
            }
         }
      }break;
      
      case RT_FORWARD:
      {
         FREPA(T) // process in order
         {
          C MeshPart &part=parts[i]; if(part._draw_mask&Renderer._mesh_draw_mask)
            {
             C MeshPart::Variation &variation=part     .getVariation();
           //C Material            &material =variation.getMaterial (); use custom material instead

               if(variation.frst)
               {
                  if(Renderer.firstPass() || variation.frst->all_passes)//if(Shader *shader=variation.frst->getShader())
                  {
                     if(!opaque_skeleton)opaque_skeleton=&SkeletonOpaqueInstances.getSkeletonInstanceOpaque(anim_skel);
                                         opaque_skeleton->newInstance(*variation.frst, material, SkeletonOpaqueShaderMaterialMeshInstances);
                     SkeletonOpaqueShaderMaterialMeshInstances.New().set(part);
                  }
               }else
               if(Renderer.firstPass())
               {
                  if(Shader *shader=variation.shader[RM_BLEND]){if(!blend_skeleton)blend_skeleton=&BlendInstances.getSkeletonInstance(anim_skel); blend_skeleton->addBlend(*        shader, material, part);}else
                  if(               variation.blst            ){if(!blend_skeleton)blend_skeleton=&BlendInstances.getSkeletonInstance(anim_skel); blend_skeleton->addBlend(*variation.blst, material, part);}
               }
            }
         }
      }break;
   }
}
/******************************************************************************/
void MeshLod::drawShadow()C
{
   DEBUG_ASSERT(Renderer()==RM_SHADOW, "'MeshLod.drawShadow' called outside of RM_SHADOW");
   REPA(T)
   {
    C MeshPart &part=parts[i]; if(part._draw_mask&Renderer._mesh_draw_mask)
      {
       C MeshPart::Variation &variation=part.getVariation(); if(Shader *shader=variation.shader[RM_SHADOW])
         {
            if(variation.last_shadow_instance<0)
            {
             C Material &material=variation.getShadowMaterial(ReuseDefaultMaterialForNonSkinnedShadowShader(shader));
               NewInstance(ShaderDraws, ShadowShaderMaterialMeshInstances.elms(), *shader, &material._shadow_material_shader, part, variation);
            }else ShadowShaderMaterialMeshInstances[variation.last_shadow_instance].next_instance=ShadowShaderMaterialMeshInstances.elms();
                                                    variation.last_shadow_instance               =ShadowShaderMaterialMeshInstances.elms();
            SetViewMatrix(ShadowShaderMaterialMeshInstances.New().set().view_matrix);
         }
      }
   }
}
void MeshLod::drawShadow(C MatrixM &matrix)C
{
   DEBUG_ASSERT(Renderer()==RM_SHADOW, "'MeshLod.drawShadow' called outside of RM_SHADOW");
   Matrix view_matrix; Bool matrix_set=false; // set matrix only when needed, because some meshes may not have shadows
   REPA(T)
   {
    C MeshPart &part=parts[i]; if(part._draw_mask&Renderer._mesh_draw_mask)
      {
       C MeshPart::Variation &variation=part.getVariation(); if(Shader *shader=variation.shader[RM_SHADOW])
         {
            if(!matrix_set){matrix_set=true; SetViewMatrix(view_matrix, matrix);}
            if(variation.last_shadow_instance<0)
            {
             C Material &material=variation.getShadowMaterial(ReuseDefaultMaterialForNonSkinnedShadowShader(shader));
               NewInstance(ShaderDraws, ShadowShaderMaterialMeshInstances.elms(), *shader, &material._shadow_material_shader, part, variation);
            }else ShadowShaderMaterialMeshInstances[variation.last_shadow_instance].next_instance=ShadowShaderMaterialMeshInstances.elms();
                                                    variation.last_shadow_instance               =ShadowShaderMaterialMeshInstances.elms();
            ShadowShaderMaterialMeshInstances.New().set().view_matrix=view_matrix;
         }
      }
   }
}
/******************************************************************************/
void MeshLod::drawShadow(C AnimatedSkeleton &anim_skel)C
{
   DEBUG_ASSERT(Renderer()==RM_SHADOW, "'MeshLod.drawShadow' called outside of RM_SHADOW");
   SkeletonInstance *shadow_skeleton=null; // set skeleton only when needed, because some meshes may not have shadows
   REPA(T)
   {
    C MeshPart &part=parts[i]; if(part._draw_mask&Renderer._mesh_draw_mask)
      {
       C MeshPart::Variation &variation=part.getVariation(); if(Shader *shader=variation.shader[RM_SHADOW])
         {
          C Material &material=variation.getShadowMaterial(ReuseDefaultMaterialForSkinnedShadowShader(shader));
            if(!shadow_skeleton)shadow_skeleton=&SkeletonShadowInstances.getSkeletonInstanceShadow(anim_skel);
                                shadow_skeleton->newInstance(*shader, material, SkeletonShadowShaderMaterialMeshInstances);
            SkeletonShadowShaderMaterialMeshInstances.New().set(part);
         }
      }
   }
}
void MeshLod::drawShadow(C AnimatedSkeleton &anim_skel, C Material &material)C
{
   DEBUG_ASSERT(Renderer()==RM_SHADOW, "'MeshLod.drawShadow' called outside of RM_SHADOW");
   SkeletonInstance *shadow_skeleton=null; // set skeleton only when needed, because some meshes may not have shadows
   REPA(T)
   {
    C MeshPart &part=parts[i]; if(part._draw_mask&Renderer._mesh_draw_mask)
      {
       C MeshPart::Variation &variation=part.getVariation(); if(Shader *shader=variation.shader[RM_SHADOW])
         {
          C Material &shadow_material=GetShadowMaterial(&material, ReuseDefaultMaterialForSkinnedShadowShader(shader));
            if(!shadow_skeleton)shadow_skeleton=&SkeletonShadowInstances.getSkeletonInstanceShadow(anim_skel);
                                shadow_skeleton->newInstance(*shader, shadow_material, SkeletonShadowShaderMaterialMeshInstances);
            SkeletonShadowShaderMaterialMeshInstances.New().set(part);
         }
      }
   }
}
/******************************************************************************
void Cloth::drawSkinned(C AnimatedSkeleton &anim_skel)C
{
   if(_cloth_mesh)
   {
    C Material &material=_cloth_mesh->getMaterial();
      switch(Renderer._cur_type)
      {
         case RT_DEFERRED:
         {
            if(Shader *shader=_cloth_mesh->_skin_shader[Renderer._opaque_mode_index])
            {
               SkeletonOpaqueInstances.getSkeletonInstanceOpaque(anim_skel)
                                      .newInstance(*shader, material, SkeletonOpaqueShaderMaterialMeshInstances);
               SkeletonOpaqueShaderMaterialMeshInstances.New().set(_cloth_mesh->_skin);
            }
         }break;

         case RT_FORWARD:
         {
            if(FRST *frst=_cloth_mesh->_phys_frst)if(Renderer.firstPass() || frst->all_passes)//if(Shader *shader=frst->getShader())
            {
               SkeletonOpaqueInstances.getSkeletonInstanceOpaque(anim_skel)
                                      .newInstance(*frst, material, SkeletonOpaqueShaderMaterialMeshInstances);
               SkeletonOpaqueShaderMaterialMeshInstances.New().set(_cloth_mesh->_skin);
            }
         }break;
      }
   }
}
void Cloth::drawSkinnedShadow(C AnimatedSkeleton &anim_skel)C
{
   if(_cloth_mesh)if(Shader *shader=_cloth_mesh->_skin_shader[RM_SHADOW])
   {
    C Material &material=_cloth_mesh->getShadowMaterial(ReuseDefaultMaterialForSkinnedShadowShader(shader));
      SkeletonShadowInstances.getSkeletonInstanceShadow(anim_skel)
                             .newInstance(*shader, material, SkeletonShadowShaderMaterialMeshInstances);
      SkeletonShadowShaderMaterialMeshInstances.New().set(_cloth_mesh->_skin);
   }
}
/******************************************************************************/
void Mesh::draw      (C Matrix           &matrix, C Matrix  &matrix_prev )C {getDrawLod(          matrix  ).draw      (matrix, matrix_prev);}
void Mesh::draw      (C MatrixM          &matrix, C MatrixM &matrix_prev )C {getDrawLod(          matrix  ).draw      (matrix, matrix_prev);}
void Mesh::draw      (C Matrix           &matrix                         )C {getDrawLod(          matrix  ).draw      (matrix             );}
void Mesh::draw      (C MatrixM          &matrix                         )C {getDrawLod(          matrix  ).draw      (matrix             );}
void Mesh::draw      (                                                   )C {getDrawLod(                  ).draw      (                   );}
void Mesh::drawShadow(C Matrix           &matrix                         )C {getDrawLod(          matrix  ).drawShadow(matrix             );}
void Mesh::drawShadow(C MatrixM          &matrix                         )C {getDrawLod(          matrix  ).drawShadow(matrix             );}
void Mesh::drawShadow(                                                   )C {getDrawLod(                  ).drawShadow(                   );}
void Mesh::draw      (C AnimatedSkeleton &anim_skel                      )C {getDrawLod(anim_skel.matrix()).draw      (anim_skel          );}
void Mesh::draw      (C AnimatedSkeleton &anim_skel, C Material &material)C {getDrawLod(anim_skel.matrix()).draw      (anim_skel, material);}
void Mesh::drawShadow(C AnimatedSkeleton &anim_skel                      )C {getDrawLod(anim_skel.matrix()).drawShadow(anim_skel          );}
void Mesh::drawShadow(C AnimatedSkeleton &anim_skel, C Material &material)C {getDrawLod(anim_skel.matrix()).drawShadow(anim_skel, material);}
/******************************************************************************/
// BLEND
/******************************************************************************/
void MeshPart::drawBlend(C Vec4 *color)C
{
   DEBUG_ASSERT(Renderer()==RM_BLEND || !Renderer._render, "'MeshPart.drawBlend' called outside of RM_BLEND");
   if(_draw_mask&Renderer._mesh_draw_mask)
   {
    C Variation &variation=getVariation();
      Shader    *s=variation.shader[RM_BLEND];                                                        // first try using Blend if specified
      if(    !s && variation.blst)s=*(Shader**)(((Byte*)variation.blst)+Renderer._blst_light_offset); // then  try using Blend Light
      if(s)
      {
       C Material &material=variation.getMaterial();
         SetSkinning();
         D.cull        (material.cull);
         D.depthOnWrite(true, material.hasDepthWrite()); Renderer.needDepthTest(); // !! 'needDepthTest' after 'depthWrite' !!
         D.alpha       (              Renderer._mesh_blend_alpha );
         D.stencil     ((STENCIL_MODE)Renderer._mesh_stencil_mode);
         material.setBlendForce();
         Bool no_blend=!material.hasAlphaBlend(); // if there is no blending then it means it's ALPHA_NONE or ALPHA_TEST, in both cases we should ignore the material.color.a, and use 1.0
         if(color || no_blend)
         {
            Vec4 col=material.colorD();
            if(no_blend)col.w=1; // force full alpha for non-blending techniques
            if(color   )col *=*color;
            Renderer.material_color_l->set(col);
            MaterialClear(); // we've changed material properties
         }
         s->begin(); render.set().draw();
         D.stencil(STENCIL_NONE);
      }
   }
}
void MeshPart::drawBlend(C Material &material, C Vec4 *color)C
{
   DEBUG_ASSERT(Renderer()==RM_BLEND || !Renderer._render, "'MeshPart.drawBlend' called outside of RM_BLEND");
   if(_draw_mask&Renderer._mesh_draw_mask)
   {
    C Variation &variation=getVariation();
      Shader    *s=variation.shader[RM_BLEND];                                                        // first try using Blend if specified
      if(    !s && variation.blst)s=*(Shader**)(((Byte*)variation.blst)+Renderer._blst_light_offset); // then  try using Blend Light
      if(s)
      {
         SetSkinning();
         D.cull        (material.cull);
         D.depthOnWrite(true, material.hasDepthWrite()); Renderer.needDepthTest(); // !! 'needDepthTest' after 'depthWrite' !!
         D.alpha       (              Renderer._mesh_blend_alpha );
         D.stencil     ((STENCIL_MODE)Renderer._mesh_stencil_mode);
         material.setBlendForce();
         Bool no_blend=!material.hasAlphaBlend(); // if there is no blending then it means it's ALPHA_NONE or ALPHA_TEST, in both cases we should ignore the material.color.a, and use 1.0
         if(color || no_blend)
         {
            Vec4 col=material.colorD();
            if(no_blend)col.w=1; // force full alpha for non-blending techniques
            if(color   )col *=*color;
            Renderer.material_color_l->set(col);
            MaterialClear(); // we've changed material properties
         }
         s->begin(); render.set().draw();
         D.stencil(STENCIL_NONE);
      }
   }
}
void MeshLod::drawBlend(                                                    C Vec4 *color)C {                                                    FREPAO(parts).drawBlend(          color);}
void MeshLod::drawBlend(C Material &material                              , C Vec4 *color)C {                                                    FREPAO(parts).drawBlend(material, color);}
void Mesh   ::drawBlend(                                                    C Vec4 *color)C {                                   getDrawLod(       ObjMatrix  ).drawBlend(          color);}
void Mesh   ::drawBlend(C MatrixM          &matrix,                         C Vec4 *color)C {SetOneMatrix(matrix, matrix     ); getDrawLod(          matrix  ).drawBlend(          color);} // blend might use velocity so set both matrixes
void Mesh   ::drawBlend(C MatrixM          &matrix, C MatrixM &matrix_prev, C Vec4 *color)C {SetOneMatrix(matrix, matrix_prev); getDrawLod(          matrix  ).drawBlend(          color);}
void Mesh   ::drawBlend(C AnimatedSkeleton &anim_skel,                      C Vec4 *color)C {anim_skel.setMatrix();             getDrawLod(anim_skel.matrix()).drawBlend(          color);}
/******************************************************************************/
void MeshPart::drawBoneHighlight(Int bone)C
{
   if(_draw_mask&Renderer._mesh_draw_mask)
      if(Shader *s=ShaderFiles("Bone Highlight")->get(S+FlagAll(render.flag(), VTX_SKIN)+(FlagOn(render.flag(), VTX_NRM) ? SBUMP_FLAT : SBUMP_ZERO)))
   {
    C Variation &variation=          getVariation();
    C Material  &material =variation.getMaterial ();
      SetSkinning();
      D.cull        (material.cull);
      D.depthOnWrite(true, true); Renderer.needDepthTest(); // !! 'needDepthTest' after 'depthWrite' !!
      D.alpha       (ALPHA_NONE);
      material.setBlendForce();
      s->begin(); render.set().drawBoneHighlight(bone, s);
   }
}
/******************************************************************************/
// OVERLAY
/******************************************************************************/
void MeshPart::drawOverlay(C Image &image, C Color &color)C
{
   DEBUG_ASSERT(Renderer()==RM_OVERLAY, "'MeshPart.drawOverlay' called outside of RM_OVERLAY");
   if(_draw_mask&Renderer._mesh_draw_mask)
   {
    C Variation &variation=getVariation(); if(Shader *shader=variation.shader[RM_OVERLAY])
      {
         SetSkinning();
       //D .alpha      (ALPHA_OVERLAY); not needed because ALPHA_OVERLAY is used everywhere in RM_OVERLAY
         D .alphaFactor(TRANSPARENT); // 'MaterialClear' called below
         D .depth      (true);
         D .cull       (true);
       /*D .depthWrite (false); not needed because false is used everywhere in RM_OVERLAY*/ Renderer.needDepthTest(); // !! 'needDepthTest' after 'depthWrite' !!
         Sh.Color[0]->set(color);
         Sh.Col  [0]->set(image);
         shader->begin (); render.set().draw();
         MaterialClear (); // we've changed texture and 'D.alphaFactor'
      }
   }
}
/******************************************************************************/
// OUTLINE
/******************************************************************************/
void MeshPart::drawOutline(C Color &color)C
{
   DEBUG_ASSERT(Renderer()==RM_OUTLINE, "'MeshPart.drawOutline' called outside of RM_OUTLINE");
   if(_draw_mask&Renderer._mesh_draw_mask)
   {
    C Variation &variation=getVariation(); if(Shader *shader=variation.shader[RM_OUTLINE])
      {
       C Material &material=variation.getMaterial();
         Renderer.setOutline(color);
         SetSkinning();
         D.depth(true);
         D.cull (material.cull); material.setOutline();
         shader->begin(); render.set().draw();
      }
   }
}
void MeshLod::drawOutline(C Color &color                               )C {                                         REPAO(parts).drawOutline(color);}
void Mesh   ::drawOutline(C Color &color                               )C {                       getDrawLod(       ObjMatrix  ).drawOutline(color);}
void Mesh   ::drawOutline(C Color &color, C MatrixM          &matrix   )C { SetOneMatrix(matrix); getDrawLod(          matrix  ).drawOutline(color);}
void Mesh   ::drawOutline(C Color &color, C AnimatedSkeleton &anim_skel)C {anim_skel.setMatrix(); getDrawLod(anim_skel.matrix()).drawOutline(color);}
/******************************************************************************/
// OUTLINE
/******************************************************************************/
void MeshPart::drawBehind(C Color &color_perp, C Color &color_parallel)C
{
   DEBUG_ASSERT(Renderer()==RM_BEHIND, "'MeshPart.drawBehind' called outside of RM_BEHIND");
   if(_draw_mask&Renderer._mesh_draw_mask)
   {
    C Variation &variation=getVariation(); if(Shader *shader=variation.shader[RM_BEHIND])
      {
       C Material &material=variation.getMaterial();
         SetSkinning();
         D.cull(material.cull); material.setBehind();
         Sh.Color[0]->set(color_perp    );
         Sh.Color[1]->set(color_parallel);
         shader->begin(); render.set().draw();
      }
   }
}
void MeshLod::drawBehind(C Color &color_perp, C Color &color_parallel                               )C {                                        FREPAO(parts).drawBehind(color_perp, color_parallel);}
void Mesh   ::drawBehind(C Color &color_perp, C Color &color_parallel                               )C {                       getDrawLod(       ObjMatrix  ).drawBehind(color_perp, color_parallel);}
void Mesh   ::drawBehind(C Color &color_perp, C Color &color_parallel, C MatrixM          &matrix   )C { SetOneMatrix(matrix); getDrawLod(          matrix  ).drawBehind(color_perp, color_parallel);}
void Mesh   ::drawBehind(C Color &color_perp, C Color &color_parallel, C AnimatedSkeleton &anim_skel)C {anim_skel.setMatrix(); getDrawLod(anim_skel.matrix()).drawBehind(color_perp, color_parallel);}
/******************************************************************************/
}
/******************************************************************************/
