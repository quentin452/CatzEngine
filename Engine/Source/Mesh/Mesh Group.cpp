/******************************************************************************/
#include "stdafx.h"
namespace EE {
/******************************************************************************/
MeshGroup &MeshGroup::include(MESH_FLAG flag) {
    REPAO(meshes).include(flag);
    return T;
}
MeshGroup &MeshGroup::exclude(MESH_FLAG flag) {
    REPAO(meshes).exclude(flag);
    return T;
}
MeshGroup &MeshGroup::keepOnly(MESH_FLAG flag) {
    REPAO(meshes).keepOnly(flag);
    return T;
}
/******************************************************************************/
void MeshGroup::zero() { ext.zero(); }
MeshGroup::MeshGroup() { zero(); }

MeshGroup &MeshGroup::delBase() {
    REPAO(meshes).delBase();
    return T;
}
MeshGroup &MeshGroup::delRender() {
    REPAO(meshes).delRender();
    return T;
}
MeshGroup &MeshGroup::del() {
    meshes.del();
    zero();
    return T;
}
/******************************************************************************/
MeshGroup &MeshGroup::create(C Mesh &src, C Boxes &boxes) {
    struct MeshPartEx : MeshPart {
        Int lod_index, cell_index;
    };
    Memt<MeshPartEx> mesh_parts;
    Memt<MeshBaseIndex> mesh_splits;
    MemtN<Int, 256> cell_parts;

    // split 'src'
    FREPD(l, src.lods()) // add in order
    {
        C MeshLod &lod = src.lod(l);
        FREPAD(p, lod) // add in order
        {
            C MeshPart &part = lod.parts[p];
            part.base.split(mesh_splits, boxes);
            FREPAD(s, mesh_splits) // add in order
            {
                MeshBaseIndex &mesh_split = mesh_splits[s];
                MeshPartEx &mesh_part = mesh_parts.New();
                Swap(mesh_part.base, SCAST(MeshBase, mesh_split));
                mesh_part.copyParams(part, true);
                mesh_part.lod_index = l;
                cell_parts(mesh_part.cell_index = mesh_split.index) = 1; // set that this cell part has some data
            }
        }
    }

    // check which 'cell_parts' have anything
    Int cells = 0;
    FREPA(cell_parts) // process in order
    {
        Int &cell_part = cell_parts[i];
        if (cell_part)
            cell_part = cells++; // if cell part is valid, then set its target index as current number of cells, and inc that by 1
    }

    // create mesh group
    MeshGroup temp, &dest = (T.meshes.contains(&src) ? temp : T); // if this contains 'src' then operate on 'temp' first
    dest.meshes.setNum(cells);
    REPAD(s, dest.meshes) {
        Mesh &mesh = dest.meshes[s].setLods(src.lods());
        mesh.copyParams(src); // create split meshes to always have the same amount of LODs
        REPD(l, mesh.lods())  // process all LODs in this split mesh
        {
            MeshLod &lod = mesh.lod(l);
            lod.copyParams(src.lod(l));
            Int parts = 0; // how many parts in this LOD
            REPA(mesh_parts) {
                MeshPartEx &mesh_part = mesh_parts[i];
                if (mesh_part.lod_index == l && cell_parts[mesh_part.cell_index] == s)
                    parts++; // if mesh_part should be assigned to this lod
            }
            lod.parts.setNum(parts); // always set LOD parts in case we're operating on MeshGroup that already had some data, and this will clear it
            if (parts) {
                parts = 0;
                FREPA(mesh_parts) // add in order
                {
                    MeshPartEx &mesh_part = mesh_parts[i];
                    if (mesh_part.lod_index == l && cell_parts[mesh_part.cell_index] == s)
                        Swap(SCAST(MeshPart, mesh_part), lod.parts[parts++]);
                }
            }
        }
    }
    if (&dest == &temp)
        Swap(temp, T);
    setBox(true);
    return T;
}
MeshGroup &MeshGroup::create(C Mesh &src, C Plane *plane, Int planes) {
    struct MeshPartEx : MeshPart {
        Int lod_index, cell_index;
    };
    Memt<MeshPartEx> mesh_parts;
    Memt<MeshBaseIndex> mesh_splits;
    MemtN<Int, 256> cell_parts;

    // split 'src'
    FREPD(l, src.lods()) // add in order
    {
        C MeshLod &lod = src.lod(l);
        FREPAD(p, lod) // add in order
        {
            C MeshPart &part = lod.parts[p];
            part.base.split(mesh_splits, plane, planes);
            FREPAD(s, mesh_splits) // add in order
            {
                MeshBaseIndex &mesh_split = mesh_splits[s];
                MeshPartEx &mesh_part = mesh_parts.New();
                Swap(mesh_part.base, SCAST(MeshBase, mesh_split));
                mesh_part.copyParams(part, true);
                mesh_part.lod_index = l;
                cell_parts(mesh_part.cell_index = mesh_split.index) = 1; // set that this cell part has some data
            }
        }
    }

    // check which cell_parts have anything
    Int cells = 0;
    FREPA(cell_parts) // process in order
    {
        Int &cell_part = cell_parts[i];
        if (cell_part)
            cell_part = cells++; // if cell part is valid, then set its target index as current number of cells, and inc that by 1
    }

    // create mesh group
    MeshGroup temp, &dest = (T.meshes.contains(&src) ? temp : T); // if this contains 'src' then operate on 'temp' first
    dest.meshes.setNum(cells);
    REPAD(s, dest.meshes) {
        Mesh &mesh = dest.meshes[s].setLods(src.lods());
        mesh.copyParams(src); // create split meshes to always have the same amount of LODs
        REPD(l, mesh.lods())  // process all LODs in this split mesh
        {
            MeshLod &lod = mesh.lod(l);
            lod.copyParams(src.lod(l));
            Int parts = 0; // how many parts in this LOD
            REPA(mesh_parts) {
                MeshPartEx &mesh_part = mesh_parts[i];
                if (mesh_part.lod_index == l && cell_parts[mesh_part.cell_index] == s)
                    parts++; // if mesh_part should be assigned to this lod
            }
            lod.parts.setNum(parts); // always set LOD parts in case we're operating on MeshGroup that already had some data, and this will clear it
            if (parts) {
                parts = 0;
                FREPA(mesh_parts) // add in order
                {
                    MeshPartEx &mesh_part = mesh_parts[i];
                    if (mesh_part.lod_index == l && cell_parts[mesh_part.cell_index] == s)
                        Swap(SCAST(MeshPart, mesh_part), lod.parts[parts++]);
                }
            }
        }
    }
    if (&dest == &temp)
        Swap(temp, T);
    setBox(true);
    return T;
}
MeshGroup &MeshGroup::create(C Mesh &src, C VecI &cells) { return create(src, Boxes(src.ext, cells)); }
MeshGroup &MeshGroup::create(C MeshGroup &src, MESH_FLAG flag_and) {
    if (this == &src)
        keepOnly(flag_and);
    else {
        meshes.setNum(src.meshes.elms());
        FREPA(T)
        meshes[i].create(src.meshes[i], flag_and);
        copyParams(src);
    }
    return T;
}
void MeshGroup::copyParams(C MeshGroup &src) {
    if (this != &src) {
        ext = src.ext;
    }
}
/******************************************************************************/
// GET
/******************************************************************************/
MESH_FLAG MeshGroup::flag() C {
    MESH_FLAG flag = MESH_NONE;
    REPA(T)
    flag |= meshes[i].flag();
    return flag;
}
Int MeshGroup::vtxs() C {
    Int n = 0;
    REPA(T)
    n += meshes[i].vtxs();
    return n;
}
Int MeshGroup::baseVtxs() C {
    Int n = 0;
    REPA(T)
    n += meshes[i].baseVtxs();
    return n;
}
Int MeshGroup::edges() C {
    Int n = 0;
    REPA(T)
    n += meshes[i].edges();
    return n;
}
Int MeshGroup::tris() C {
    Int n = 0;
    REPA(T)
    n += meshes[i].tris();
    return n;
}
Int MeshGroup::quads() C {
    Int n = 0;
    REPA(T)
    n += meshes[i].quads();
    return n;
}
Int MeshGroup::faces() C {
    Int n = 0;
    REPA(T)
    n += meshes[i].faces();
    return n;
}
Int MeshGroup::trisTotal() C {
    Int n = 0;
    REPA(T)
    n += meshes[i].trisTotal();
    return n;
}
/******************************************************************************/
// JOIN
/******************************************************************************/
MeshGroup &MeshGroup::join(Int i0, Int i1) {
    if (i0 != i1 && InRange(i0, T) && InRange(i1, T)) {
        if (i1 < i0)
            Swap(i0, i1);
        meshes[i0] += meshes[i1];
        meshes.remove(i1, true);
    }
    return T;
}
MeshGroup &MeshGroup::joinAll(Bool test_material, Bool test_draw_group, Bool test_name, MESH_FLAG test_vtx_flag, Flt weld_pos_eps) {
    REPAO(meshes).joinAll(test_material, test_draw_group, test_name, test_vtx_flag, weld_pos_eps);
    return T;
}
/******************************************************************************/
// TEXTURIZE
/******************************************************************************/
MeshGroup &MeshGroup::texMap(Flt scale, Byte uv_index) {
    REPAO(meshes).texMap(scale, uv_index);
    return T;
}
MeshGroup &MeshGroup::texMap(C Matrix &matrix, Byte uv_index) {
    REPAO(meshes).texMap(matrix, uv_index);
    return T;
}
MeshGroup &MeshGroup::texMap(C Plane &plane, Byte uv_index) {
    REPAO(meshes).texMap(plane, uv_index);
    return T;
}
MeshGroup &MeshGroup::texMap(C Ball &ball, Byte uv_index) {
    REPAO(meshes).texMap(ball, uv_index);
    return T;
}
MeshGroup &MeshGroup::texMap(C Tube &tube, Byte uv_index) {
    REPAO(meshes).texMap(tube, uv_index);
    return T;
}
MeshGroup &MeshGroup::texMove(C Vec2 &move, Byte uv_index) {
    REPAO(meshes).texMove(move, uv_index);
    return T;
}
MeshGroup &MeshGroup::texScale(C Vec2 &scale, Byte uv_index) {
    REPAO(meshes).texScale(scale, uv_index);
    return T;
}
MeshGroup &MeshGroup::texRotate(Flt angle, Byte uv_index) {
    REPAO(meshes).texRotate(angle, uv_index);
    return T;
}
/******************************************************************************/
// TRANSFORM
/******************************************************************************/
MeshGroup &MeshGroup::move(C Vec &move) {
    ext += move;
    REPAO(meshes).move(move);
    return T;
}
MeshGroup &MeshGroup::scale(C Vec &scale) {
    ext *= scale;
    REPAO(meshes).scale(scale);
    return T;
}
MeshGroup &MeshGroup::scaleMove(C Vec &scale, C Vec &move) {
    ext *= scale;
    ext += move;
    REPAO(meshes).scaleMove(scale, move);
    return T;
}
/******************************************************************************/
// SET
/******************************************************************************/
MeshGroup &MeshGroup::setFaceNormals() {
    REPAO(meshes).setFaceNormals();
    return T;
}
MeshGroup &MeshGroup::setNormals() {
    REPAO(meshes).setNormals();
    return T;
}
MeshGroup &MeshGroup::setVtxDup(MESH_FLAG flag, Flt pos_eps, Flt nrm_cos) {
    REPAO(meshes).setVtxDup(flag, pos_eps, nrm_cos);
    return T;
}
MeshGroup &MeshGroup::setRender() {
    REPAO(meshes).setRender();
    return T;
}
MeshGroup &MeshGroup::setShader() {
    REPAO(meshes).setShader();
    return T;
}
MeshGroup &MeshGroup::material(C MaterialPtr &material) {
    REPAO(meshes).material(material);
    return T;
}
Bool MeshGroup::setBox(Bool set_mesh_boxes) {
    if (set_mesh_boxes)
        REPAO(meshes).setBox();
    Bool found = false;
    Box box;
    REPA(T) {
        C Mesh &mesh = T.meshes[i];
        if (mesh.vtxs()) {
            if (found)
                box |= mesh.ext;
            else {
                found = true;
                box = mesh.ext;
            }
        }
    }
    if (found)
        T.ext = box;
    else
        T.ext.zero();
    return found;
}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
MeshGroup &MeshGroup::weldVtx2D(MESH_FLAG flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps) {
    REPAO(meshes).weldVtx2D(flag, pos_eps, nrm_cos, remove_degenerate_faces_eps);
    return T;
}
MeshGroup &MeshGroup::weldVtx(MESH_FLAG flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps) {
    REPAO(meshes).weldVtx(flag, pos_eps, nrm_cos, remove_degenerate_faces_eps);
    return T;
}
MeshGroup &MeshGroup::weldVtxValues(MESH_FLAG flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps) {
    PROFILE_START("CatzEngine::MeshGroup::weldVtxValues(MESH_FLAG flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps)")
    flag &= T.flag();                                                                                                // can weld only values that we have
    if (flag & (VTX_POS | VTX_NRM_TAN_BIN | VTX_HLP | VTX_TEX_ALL | VTX_COLOR | VTX_MATERIAL | VTX_SKIN | VTX_SIZE)) // if have anything to weld
    {
        struct VtxDupIndex : VtxDupNrm {
            VecI index;
            Flt weight;
            Vec4 color;
        };

        // create vertex array
        Int vtx_num = baseVtxs();
        Memc<VtxDupIndex> vtxs;
        vtxs.setNum(vtx_num);
        vtx_num = 0;
        REPAD(m, T) {
            Mesh &mesh = T.meshes[m];
            REPAD(p, mesh.parts) {
                MeshBase &mshb = mesh.parts[p].base;
                C Vec *pos = mshb.vtx.pos();
                C Vec *nrm = mshb.vtx.nrm();
                Int part_vtx_ofs = vtx_num;

                FREPA(mshb.vtx) // add in order
                {
                    VtxDupIndex &vtx = vtxs[vtx_num++];
                    vtx.pos = (pos ? pos[i] : VecZero);
                    vtx.nrm = (nrm ? nrm[i] : VecZero);
                    vtx.index.set(i, p, m);
                    vtx.weight = 0;
                    vtx.color.zero();
                }

                // calculate weight for all vertexes based on their face area
                if (pos) {
                    if (C VecI *tri_ind = mshb.tri.ind())
                        REPA(mshb.tri) {
                            C VecI &ind = tri_ind[i];
                            Flt weight = TriArea2(pos[ind.x], pos[ind.y], pos[ind.z]);
                            vtxs[part_vtx_ofs + ind.x].weight += weight;
                            vtxs[part_vtx_ofs + ind.y].weight += weight;
                            vtxs[part_vtx_ofs + ind.z].weight += weight;
                        }
                    if (C VecI4 *quad_ind = mshb.quad.ind())
                        REPA(mshb.quad) {
                            C VecI4 &ind = quad_ind[i];
                            Flt weight = TriArea2(pos[ind.x], pos[ind.y], pos[ind.w]) + TriArea2(pos[ind.y], pos[ind.z], pos[ind.w]);
                            vtxs[part_vtx_ofs + ind.x].weight += weight;
                            vtxs[part_vtx_ofs + ind.y].weight += weight;
                            vtxs[part_vtx_ofs + ind.z].weight += weight;
                            vtxs[part_vtx_ofs + ind.w].weight += weight;
                        }
                }
            }
        }

        // get vtx dup
        SetVtxDup(SCAST(Memc<VtxDupNrm>, vtxs), ext, pos_eps, nrm_cos);

        // first scale unique vertex values by their weight
        REPA(vtxs) {
            VtxDupIndex &vn = vtxs[i];
            if (vn.dup == i) // unique
            {
                Flt weight = vn.weight;
                if (flag & VTX_POS)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.pos(vn.index.x) *= weight;
                // if(flag&VTX_MATERIAL)         meshes[vn.index.z].parts[vn.index.y].base.vtx.material(vn.index.x)*=weight; VecB4 !! sum must be equal to 255 !!
                // if(flag&VTX_MATRIX  )         meshes[vn.index.z].parts[vn.index.y].base.vtx.matrix  (vn.index.x)*=weight; VtxBone
                // if(flag&VTX_BLEND   )         meshes[vn.index.z].parts[vn.index.y].base.vtx.blend   (vn.index.x)*=weight; VecB4 !! sum must be equal to 255 !!
                if (flag & VTX_NRM)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.nrm(vn.index.x) *= weight;
                if (flag & VTX_TAN)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.tan(vn.index.x) *= weight;
                if (flag & VTX_BIN)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.bin(vn.index.x) *= weight;
                if (flag & VTX_HLP)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.hlp(vn.index.x) *= weight;
                if (flag & VTX_TEX0)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.tex0(vn.index.x) *= weight;
                if (flag & VTX_TEX1)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.tex1(vn.index.x) *= weight;
                if (flag & VTX_TEX2)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.tex2(vn.index.x) *= weight;
                if (flag & VTX_TEX3)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.tex3(vn.index.x) *= weight;
                if (flag & VTX_SIZE)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.size(vn.index.x) *= weight;
                if (flag & VTX_COLOR)
                    vn.color = meshes[vn.index.z].parts[vn.index.y].base.vtx.color(vn.index.x) * weight;
            }
        }

        // now add duplicate values to unique
        REPA(vtxs) {
            VtxDupIndex &vn = vtxs[i];
            if (vn.dup != i) // duplicate
            {
                VtxDupIndex &vd = vtxs[vn.dup];
                Flt weight = vn.weight;
                vd.weight += weight;
                if (flag & VTX_POS)
                    meshes[vd.index.z].parts[vd.index.y].base.vtx.pos(vd.index.x) += weight * meshes[vn.index.z].parts[vn.index.y].base.vtx.pos(vn.index.x);
                // if(flag&VTX_MATERIAL)meshes[vd.index.z].parts[vd.index.y].base.vtx.material(vd.index.x)+=weight*meshes[vn.index.z].parts[vn.index.y].base.vtx.material(vn.index.x); VecB4 !! sum must be equal to 255 !!
                // if(flag&VTX_MATRIX  )meshes[vd.index.z].parts[vd.index.y].base.vtx.matrix  (vd.index.x)+=weight*meshes[vn.index.z].parts[vn.index.y].base.vtx.matrix  (vn.index.x); VtxBone
                // if(flag&VTX_BLEND   )meshes[vd.index.z].parts[vd.index.y].base.vtx.blend   (vd.index.x)+=weight*meshes[vn.index.z].parts[vn.index.y].base.vtx.blend   (vn.index.x); VecB4 !! sum must be equal to 255 !!
                if (flag & VTX_NRM)
                    meshes[vd.index.z].parts[vd.index.y].base.vtx.nrm(vd.index.x) += weight * meshes[vn.index.z].parts[vn.index.y].base.vtx.nrm(vn.index.x);
                if (flag & VTX_TAN)
                    meshes[vd.index.z].parts[vd.index.y].base.vtx.tan(vd.index.x) += weight * meshes[vn.index.z].parts[vn.index.y].base.vtx.tan(vn.index.x);
                if (flag & VTX_BIN)
                    meshes[vd.index.z].parts[vd.index.y].base.vtx.bin(vd.index.x) += weight * meshes[vn.index.z].parts[vn.index.y].base.vtx.bin(vn.index.x);
                if (flag & VTX_HLP)
                    meshes[vd.index.z].parts[vd.index.y].base.vtx.hlp(vd.index.x) += weight * meshes[vn.index.z].parts[vn.index.y].base.vtx.hlp(vn.index.x);
                if (flag & VTX_TEX0)
                    meshes[vd.index.z].parts[vd.index.y].base.vtx.tex0(vd.index.x) += weight * meshes[vn.index.z].parts[vn.index.y].base.vtx.tex0(vn.index.x);
                if (flag & VTX_TEX1)
                    meshes[vd.index.z].parts[vd.index.y].base.vtx.tex1(vd.index.x) += weight * meshes[vn.index.z].parts[vn.index.y].base.vtx.tex1(vn.index.x);
                if (flag & VTX_TEX2)
                    meshes[vd.index.z].parts[vd.index.y].base.vtx.tex2(vd.index.x) += weight * meshes[vn.index.z].parts[vn.index.y].base.vtx.tex2(vn.index.x);
                if (flag & VTX_TEX3)
                    meshes[vd.index.z].parts[vd.index.y].base.vtx.tex3(vd.index.x) += weight * meshes[vn.index.z].parts[vn.index.y].base.vtx.tex3(vn.index.x);
                if (flag & VTX_SIZE)
                    meshes[vd.index.z].parts[vd.index.y].base.vtx.size(vd.index.x) += weight * meshes[vn.index.z].parts[vn.index.y].base.vtx.size(vn.index.x);
                if (flag & VTX_COLOR)
                    vd.color += weight * vn.color;
            }
        }

        // normalize unique vertex values
        REPA(vtxs) {
            VtxDupIndex &vn = vtxs[i];
            if (vn.dup == i) // unique
                if (Flt weight = vn.weight) {
                    weight = 1 / weight;
                    if (flag & VTX_POS)
                        meshes[vn.index.z].parts[vn.index.y].base.vtx.pos(vn.index.x) *= weight;
                    // if(flag&VTX_MATERIAL)meshes[vn.index.z].parts[vn.index.y].base.vtx.material(vn.index.x)*=weight; VecB4 !! sum must be equal to 255 !!
                    // if(flag&VTX_MATRIX  )meshes[vn.index.z].parts[vn.index.y].base.vtx.matrix  (vn.index.x)*=weight; VtxBone
                    // if(flag&VTX_BLEND   )meshes[vn.index.z].parts[vn.index.y].base.vtx.blend   (vn.index.x)*=weight; VecB4 !! sum must be equal to 255 !!
                    if (flag & VTX_NRM)
                        meshes[vn.index.z].parts[vn.index.y].base.vtx.nrm(vn.index.x).normalize();
                    if (flag & VTX_TAN)
                        meshes[vn.index.z].parts[vn.index.y].base.vtx.tan(vn.index.x).normalize();
                    if (flag & VTX_BIN)
                        meshes[vn.index.z].parts[vn.index.y].base.vtx.bin(vn.index.x).normalize();
                    if (flag & VTX_HLP)
                        meshes[vn.index.z].parts[vn.index.y].base.vtx.hlp(vn.index.x) *= weight;
                    if (flag & VTX_TEX0)
                        meshes[vn.index.z].parts[vn.index.y].base.vtx.tex0(vn.index.x) *= weight;
                    if (flag & VTX_TEX1)
                        meshes[vn.index.z].parts[vn.index.y].base.vtx.tex1(vn.index.x) *= weight;
                    if (flag & VTX_TEX2)
                        meshes[vn.index.z].parts[vn.index.y].base.vtx.tex2(vn.index.x) *= weight;
                    if (flag & VTX_TEX3)
                        meshes[vn.index.z].parts[vn.index.y].base.vtx.tex3(vn.index.x) *= weight;
                    if (flag & VTX_SIZE)
                        meshes[vn.index.z].parts[vn.index.y].base.vtx.size(vn.index.x) *= weight;
                    if (flag & VTX_COLOR)
                        meshes[vn.index.z].parts[vn.index.y].base.vtx.color(vn.index.x) = weight * vn.color;
                }
        }

        // set duplicate vertex values
        REPA(vtxs) {
            VtxDupIndex &vn = vtxs[i];
            if (vn.dup != i) // duplicate
            {
                VtxDupIndex &vd = vtxs[vn.dup];
                if (flag & VTX_POS)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.pos(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.pos(vd.index.x);
                if (flag & VTX_MATERIAL)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.material(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.material(vd.index.x);
                if (flag & VTX_MATRIX)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.matrix(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.matrix(vd.index.x);
                if (flag & VTX_BLEND)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.blend(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.blend(vd.index.x);
                if (flag & VTX_NRM)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.nrm(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.nrm(vd.index.x);
                if (flag & VTX_TAN)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.tan(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.tan(vd.index.x);
                if (flag & VTX_BIN)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.bin(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.bin(vd.index.x);
                if (flag & VTX_HLP)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.hlp(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.hlp(vd.index.x);
                if (flag & VTX_TEX0)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.tex0(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.tex0(vd.index.x);
                if (flag & VTX_TEX1)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.tex1(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.tex1(vd.index.x);
                if (flag & VTX_TEX2)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.tex2(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.tex2(vd.index.x);
                if (flag & VTX_TEX3)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.tex3(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.tex3(vd.index.x);
                if (flag & VTX_SIZE)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.size(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.size(vd.index.x);
                if (flag & VTX_COLOR)
                    meshes[vn.index.z].parts[vn.index.y].base.vtx.color(vn.index.x) = meshes[vd.index.z].parts[vd.index.y].base.vtx.color(vd.index.x);
            }
        }

        // remove degenerate faces
        if ((flag & VTX_POS) && remove_degenerate_faces_eps >= 0)
            removeDegenerateFaces(remove_degenerate_faces_eps);
    }
    PROFILE_STOP("CatzEngine::MeshGroup::weldVtxValues(MESH_FLAG flag, Flt pos_eps, Flt nrm_cos, Flt remove_degenerate_faces_eps)")
    return T;
}
MeshGroup &MeshGroup::freeOpenGLESData() {
    REPAO(meshes).freeOpenGLESData();
    return T;
}
/******************************************************************************/
// CONVERT
/******************************************************************************/
MeshGroup &MeshGroup::triToQuad(Flt cos) {
    REPAO(meshes).triToQuad(cos);
    return T;
}
MeshGroup &MeshGroup::quadToTri(Flt cos) {
    REPAO(meshes).quadToTri(cos);
    return T;
}
/******************************************************************************/
// FIX
/******************************************************************************/
MeshGroup &MeshGroup::fixTexWrapping(Byte uv_index) {
    REPAO(meshes).fixTexWrapping(uv_index);
    return T;
}
MeshGroup &MeshGroup::fixTexOffset(Byte uv_index) {
    REPAO(meshes).fixTexOffset(uv_index);
    return T;
}
/******************************************************************************/
// ADD/REMOVE
/******************************************************************************/
MeshGroup &MeshGroup::remove(Int i, Bool set_box) {
    if (InRange(i, T)) {
        meshes.remove(i, true);
        if (set_box)
            setBox(false);
    }
    return T;
}
/******************************************************************************/
// OPTIMIZE
/******************************************************************************/
MeshGroup &MeshGroup::removeDegenerateFaces(Flt eps) {
    REPAO(meshes).removeDegenerateFaces(eps);
    return T;
}
MeshGroup &MeshGroup::weldCoplanarFaces(Flt cos_face, Flt cos_vtx, Bool safe, Flt max_face_length) {
    REPAO(meshes).weldCoplanarFaces(cos_face, cos_vtx, safe, max_face_length);
    return T;
}
MeshGroup &MeshGroup::sortByMaterials() {
    REPAO(meshes).sortByMaterials();
    return T;
}

MeshGroup &MeshGroup::simplify(Flt intensity, Flt max_distance, Flt max_uv, Flt max_color, Flt max_material, Flt max_skin, Flt max_normal, Bool keep_border, MESH_SIMPLIFY mode, Flt pos_eps, MeshGroup *dest, Bool *stop) {
    if (!dest)
        dest = this;
    if (dest != this) {
        dest->copyParams(T);
        dest->meshes.setNum(meshes.elms());
    }
    REPAO(meshes).simplify(intensity, max_distance, max_uv, max_color, max_material, max_skin, max_normal, keep_border, mode, pos_eps, &dest->meshes[i], stop);
    return *dest;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
void MeshGroup::draw() C {
    switch (meshes.elms()) {
    case 0:
        return; // if           no  meshes then return
    case 1:
        break; // if only      one mesh   then continue but skip MeshGroup.ext frustum checking because it won't be needed (it's equal to meshes[0].ext which is checked later)
    default:
        if (!Frustum(ext))
            return; // if more than one meshes then check         the MeshGroup.ext
    }
    FREPA(meshes) {
        C Mesh &mesh = meshes[i];
        if (Frustum(mesh))
            mesh.draw();
    }
}
void MeshGroup::draw(C MatrixM &matrix) C {
    switch (meshes.elms()) {
    case 0:
        return; // if           no  meshes then return
    case 1:
        break; // if only      one mesh   then continue but skip MeshGroup.ext frustum checking because it won't be needed (it's equal to meshes[0].ext which is checked later)
    default:
        if (!Frustum(ext, matrix))
            return; // if more than one meshes then check         the MeshGroup.ext
    }
    FREPA(meshes) {
        C Mesh &mesh = meshes[i];
        if (Frustum(mesh, matrix))
            mesh.draw(matrix);
    }
}
/******************************************************************************/
void MeshGroup::drawShadow() C {
    switch (meshes.elms()) {
    case 0:
        return; // if           no  meshes then return
    case 1:
        break; // if only      one mesh   then continue but skip MeshGroup.ext frustum checking because it won't be needed (it's equal to meshes[0].ext which is checked later)
    default:
        if (!Frustum(ext))
            return; // if more than one meshes then check         the MeshGroup.ext
    }
    FREPA(meshes) {
        C Mesh &mesh = meshes[i];
        if (Frustum(mesh))
            mesh.drawShadow();
    }
}
void MeshGroup::drawShadow(C MatrixM &matrix) C {
    switch (meshes.elms()) {
    case 0:
        return; // if           no  meshes then return
    case 1:
        break; // if only      one mesh   then continue but skip MeshGroup.ext frustum checking because it won't be needed (it's equal to meshes[0].ext which is checked later)
    default:
        if (!Frustum(ext, matrix))
            return; // if more than one meshes then check         the MeshGroup.ext
    }
    FREPA(meshes) {
        C Mesh &mesh = meshes[i];
        if (Frustum(mesh, matrix))
            mesh.drawShadow(matrix);
    }
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
